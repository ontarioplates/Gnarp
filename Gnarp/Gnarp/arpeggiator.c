#include "arpeggiator.h"

//Play list to be used for all function
static NotePlayer global_note_player;

//Return pointer to the global note player
NotePlayer* get_note_player(){
    return &global_note_player;
}

ISR(TCC0_CCA_vect){
    //reset beat clock
    TCC0.CNT = 0;
}

ISR(TCC0_CCB_vect){
    //MAYBE DISABLE USARTRX INTERRUPT?
    
    //disable CCB (note on) and CCC (note off) interrupts
    TCC0.CTRLB &= ~0x20; 
    TCC0.CTRLB &= ~0x40;
    
    NotePlayer* note_player = get_note_player();
    
    //capture the time at interrupt start (aka the current compare B value)
    uint32_t current_time = (uint32_t) TCC0.CCB;
    uint32_t next_stop_time;
    uint32_t next_start_time;
    
    //if a note is still playing, stop it
    if (note_player->play_status)
        stop_current_note(note_player);
    
	//rebuild the playlist if necessary
	if (note_player->rebuild_playlist)
		build_play_list(note_player);
	
    //start the next note
    start_next_note(note_player);
    
    //compute next compare values
    next_start_time = current_time + note_player->start_time_increment;
    next_stop_time = current_time + note_player->stop_time_increment;
    
    //check for overflow
    if (next_start_time > TCC0.CCA)
        next_start_time = next_start_time - TCC0.CCA;
    if (next_stop_time > TCC0.CCA)
        next_stop_time = next_stop_time - TCC0.CCA;
    
    //assign values to compare registers
    TCC0.CCB = (uint16_t) next_start_time;
    TCC0.CCC = (uint16_t) next_stop_time;
    
    //enable CCB (note on) and CCC (note off) interrupts
    TCC0.CTRLB |= 0x20;
    TCC0.CTRLB |= 0x40;
}

ISR(TCC0_CCC_vect){
    //MAYBE DISABLE USARTRX INTERRUPT?
    //disable CCB (note on) and CCC (note off) interrupts
    TCC0.CTRLB &= ~0x20; 
    TCC0.CTRLB &= ~0x40;
    
    //stop the current note
    stop_current_note(get_note_player());
    
    //enable CCB (note on) interrupt
    TCC0.CTRLB |= 0x40;
}

ISR(TCC0_CCD_vect){
    //midi_send_clock(serial_midi_device());  //send clock tick
    //calculate time for next clock tick
}

static void send_all_notes_off(){
    uint8_t i;
    for (i = 0; i < 128; i++)
        midi_send_noteoff(serial_midi_device(),MIDI_CHAN,i, 0);
}

static void initialize_note_timer(){
    TCC0.CTRLA = 0x00;  //disable timer
    TCC0.CTRLB = 0x00;  //disable all compares
    TCC0.CTRLC = 0x00;
    TCC0.CTRLD = 0x00;
    TCC0.INTFLAGS = 0x00;  //clear interrupt flags
    TCC0.INTCTRLA = 0x00;
    TCC0.INTCTRLB = 0x5B;  //enable CCA interrupt Hi-Level, CCB low, CCC mid, CCD low
    TCC0.CNT = 0;          //reset counter
}


static void calculate_start_time_increment(NotePlayer* note_player){
    uint8_t i;
    
    //start with value for a quarter note
    uint32_t new_start_time_increment = TCC0.CCA;
    
    //according to time division setting, divide by x^2
    for (i = 0; i < note_player->time_division; i++)
        new_start_time_increment /= 2;
        
    switch(note_player->time_variation){
        case NONE:      break;
            
        case DOTTED:    new_start_time_increment *= 3;
                        new_start_time_increment /= 2;
                        break;
                
        case TRIPLET:   new_start_time_increment *= 2;
                        new_start_time_increment /= 3;
                        break;
    }    
    
    note_player->start_time_increment = new_start_time_increment;
}

static void calculate_stop_time_increment(NotePlayer* note_player){
    uint32_t new_stop_time_increment = (uint32_t) (note_player->start_time_increment) * note_player->note_duration;
    new_stop_time_increment = new_stop_time_increment / MAX_NOTE_DURATION;
    
    note_player->stop_time_increment = (uint16_t) new_stop_time_increment;
}

//Reset all data in play list
void initialize_note_player(NotePlayer* note_player){
    
    uint8_t i;
    for (i = 0; i < MAX_PLAY_NOTES; i++)
        note_player->play_list[i] = NULL;
    note_player->note_max = 0;
    note_player->note_index = 0;
    note_player->play_status = 0;
}

void initialize_arpeggiator(){
    //initialize note list in linkedlist.c
    //initialize play list
    
    initialize_note_list();
//    initialize_play_list(get_note_player());
}

void build_play_list(NotePlayer* note_player){
    
    //builds the play list according to pattern selection (pot0)
	
	NoteList* note_list = note_player->note_list;
    
    uint8_t play_list_index = 0;
    Note* current_note;
    
    uint8_t note_list_size = note_list->length;
    uint8_t random_list_depth;      //index for random pattern
    
    uint8_t i;
    uint8_t mirror = 0;


    switch(0){
        //Asc pitch
        case 0:
            for(current_note = note_list->head_pitch; current_note; current_note=current_note->next_note_by_pitch)
                note_player->play_list[play_list_index++] = current_note;
            break;

        //Desc pitch
        case 1:
            for(current_note = note_list->tail_pitch; current_note; current_note=current_note->previous_note_by_pitch)
                note_player->play_list[play_list_index++] = current_note;
            break;

        //Asc trigger
        case 2:
            for(current_note = note_list->head_trigger; current_note; current_note=current_note->next_note_by_trigger)
                note_player->play_list[play_list_index++] = current_note;
            break;

        //Desc trigger
        case 3:
            for(current_note = note_list->tail_trigger; current_note; current_note=current_note->previous_note_by_trigger)
                note_player->play_list[play_list_index++] = current_note;
            break;

        //random
        case 4:
            for(; play_list_index < RAND_BUFF; play_list_index++){
                random_list_depth = rand() % note_list_size;
                current_note = note_list->head_pitch;
                for(i = 0; i < random_list_depth; i++)
                    current_note = current_note->next_note_by_pitch;
                note_player->play_list[play_list_index++] = current_note;
            }
            break;
    }

    //option to mirror the pattern
    if (mirror){
        uint8_t mirrored_length;
        uint8_t edge_scale;
        uint8_t k;

        if (mirror == 2){
			//double edge
            mirrored_length = play_list_index*2;
            edge_scale = 1;
        }
        if (mirror == 1){
			//single edge
            if (play_list_index < 3)
                mirrored_length = 0;
            else{
                mirrored_length = play_list_index*2 - 2;
                edge_scale = 0;
            }
        }
        if (mirrored_length){
            play_list_index += -1;
            for (k = 1; play_list_index + k < mirrored_length; k++){
                note_player->play_list[play_list_index + k] = note_player->play_list[play_list_index - k + edge_scale];
            }
            play_list_index = mirrored_length;
        }
    }

    note_player->note_max = play_list_index;     //set play list note_max appropriately
	
	while (note_player->note_index > note_player->note_max)
		note_player->note_index -= note_player->note_max;
	
	note_player->rebuild_playlist = 0;
	
    return;
}

//called whenever a new MIDI noteon message is recieved
//adds the new note to the note list (if it's not a duplicate)
//rebuilds play list and starts playing if it's the first note in the list
void input_note_on(NotePlayer* note_player, uint8_t pitch, uint8_t velocity){
    //MAYBE DISABLE USARTrx AND/OR TIMER INTERRUPTS
	
	//try to insert the note
    bool insert_success = insert_note(note_player->note_list, pitch, velocity);
	
	//if the note list is full, quit the routine
	if (!insert_success)
		return;
	
	//if this is the first note, build the playlist and start playing immediately
	if (note_player->note_list->length == 1){
		//capture the current time
		uint32_t current_time = TCC0.CNT;
		
		build_play_list(note_player);
		
		//play the first (only) note
		start_first_note(note_player);
    
		//compute next compare values
		uint32_t next_start_time = current_time + note_player->start_time_increment;
		uint32_t next_stop_time = current_time + note_player->stop_time_increment;
    
		//check for overflow
		if (next_start_time > TCC0.CCA)
			next_start_time = next_start_time - TCC0.CCA;
		if (next_stop_time > TCC0.CCA)
			next_stop_time = next_stop_time - TCC0.CCA;
    
		//assign values to compare registers
		TCC0.CCB = (uint16_t) next_start_time;
		TCC0.CCC = (uint16_t) next_stop_time;
    
		//enable CCB (note on) and CCC (note off) interrupts
		TCC0.CTRLB |= 0x20;
		TCC0.CTRLB |= 0x40;
		
		return;
	}
	
	//otherwise set the playlist to be rebuilt on the next note on
	note_player->rebuild_playlist = 1;
}

void input_note_off(NotePlayer* note_player, uint8_t pitch){
	//MAYBE DISABLE USARTrx AND/OR TIMER INTERRUPTS

	
	bool remove_success = remove_note_by_pitch(note_player->note_list,pitch);
	
	if (!remove_success)
		return;
		
	if (note_player->note_list->length == 0){
		//disable CCB (note on) and CCC (note off) interrupts
		TCC0.CTRLB &= ~0x20; 
		TCC0.CTRLB &= ~0x40;
		
		//stop playing
		stop_current_note(note_player);
		
		//reset the note player
		initialize_note_player(note_player);
	}
}

void stop_current_note(NotePlayer* note_player){
    //set midi message to stop the current note
    midi_send_noteoff(serial_midi_device(),MIDI_CHAN,note_player->play_list[note_player->note_index]->pitch,note_player->play_list[note_player->note_index]->velocity);
    
    //clear play flag
    note_player->play_status = 0;
}

void start_first_note(NotePlayer* note_player){
	note_player->repeat_index = 0;
	note_player->octave_index = 0;
	note_player->note_index = 0;
	note_player->play_status = 1;
    midi_send_noteon(serial_midi_device(),MIDI_CHAN,note_player->play_list[note_player->note_index]->pitch,note_player->play_list[note_player->note_index]->velocity);
}

void start_next_note(NotePlayer* note_player){
    //increment repeat count
    note_player->repeat_index += 1;
    
    //if note has repeated enough times, reset the repeat index and increment the note index to get the next note to play
    if (note_player->repeat_index > note_player->repeat_max){
        note_player->repeat_index = 0;
        note_player->note_index += 1;
    }
    
    //if the play list is at the end, reset the note index and increment the octave index
    if (note_player->note_index > note_player->note_max){
        note_player->note_index = 0;
        note_player->octave_index += 1;
    }
    
    //if the last octave is reached, reset the octave index
    if (note_player->octave_index > note_player->octave_max){
        note_player->octave_index = 0;
        
        //ADD CODE TO CHECK FOR RANDOM SETTING
    }

    //send midi message to start the note
    midi_send_noteon(serial_midi_device(),MIDI_CHAN,note_player->play_list[note_player->note_index]->pitch,note_player->play_list[note_player->note_index]->velocity);
    
    //set play flag
    note_player->play_status = 1;
}


/*
//Play queue variables

uint8_t octScale = 100;
uint8_t MCU_USART_pitch;
uint8_t MCU_USART_velocity;
NoteList *testList;

uint8_t curPitch = 0;
uint8_t rptCNT = 0;
uint8_t ptrnCNT = 0;
uint8_t octCNT = 0;
uint8_t UIoctMAX;
uint8_t ptrnMAX;
uint8_t UIrptMAX;
uint8_t UIpattern;

uint8_t MCUgateTMR;
uint8_t MCUplayTMR;
uint8_t UIgateMAX;
uint8_t UIplayMAX;
Note playQ[MAX_NOTES];

void pruint8_tQ(){
    uint8_t i;

    for(i = 0; i < MAX_NOTES; i++){
        if (playQ[i].pitch == -1)
            break;

        pruint8_tf("%3i: %3i / %3i\n",i, playQ[i].pitch, playQ[i].velocity);
    }
    return;
}

void createPattern(NoteList *note_list){
 //   outNote playQ[MAX_NOTES];

    uint8_t pIndex = 0;
    Note* curNode;
    uint8_t cnt;
    uint8_t i;
    uint8_t r;
    uint8_t mirror = 0;
    uint8_t caseVar = UIpattern;
    while(caseVar > 9){
        mirror += 1;
        caseVar += -10;
    }
    switch(caseVar){
        //Asc pitch
        case 0:
            for(curNode = note_list->head_pitch; curNode; curNode=curNode->next_note_by_pitch){
                playQ[pIndex].pitch = curNode->pitch;
                note_player->play_list[play_list_index++]->velocity = curNode->velocity;
            }
            break;

        //Desc pitch
        case 1:
            for(curNode = note_list->tail_pitch; curNode; curNode=curNode->previous_note_by_pitch){
                playQ[pIndex].pitch = curNode->pitch;
                note_player->play_list[play_list_index++]->velocity = curNode->velocity;
            }
            break;

        //Asc trigger
        case 2:
            for(curNode = note_list->head_trigger; curNode; curNode=curNode->next_note_by_trigger){
                playQ[pIndex].pitch = curNode->pitch;
                note_player->play_list[play_list_index++]->velocity = curNode->velocity;
            }
            break;

        //Desc trigger
        case 3:
            for(curNode = note_list->tail_trigger; curNode; curNode=curNode->previous_note_by_trigger){
                playQ[pIndex].pitch = curNode->pitch;
                note_player->play_list[play_list_index++]->velocity = curNode->velocity;
            }
            break;

        //random
        case 4:
            cnt = note_list->note_max;
            for(; pIndex < RAND_BUFF; pIndex++){
                r = rand() % cnt;
                curNode = note_list->head_pitch;
                for(i = 0; i < r; i++)
                    curNode = curNode->next_note_by_pitch;
                playQ[pIndex].pitch = curNode->pitch;
                playQ[pIndex].velocity = curNode->velocity;
            }
            break;
    }


    if (mirror){
        uint8_t mirrored_length;
        uint8_t m;
        uint8_t k;

        if (mirror == 2){                         //double edge

            mirrored_length = pIndex*2;
            m = 1;
        }
        if (mirror == 1){                         //single edge

            if (pIndex < 3)
                mirrored_length = 0;
            else{
                mirrored_length = pIndex*2 - 2;
                m = 0;
            }
        }


        if (mirrored_length){
            pIndex += -1;
            for (k = 1; pIndex + k < mirrored_length; k++){
                        playQ[pIndex + k].pitch = playQ[pIndex - k + m].pitch;
                        playQ[pIndex + k].velocity = playQ[pIndex - k + m].velocity;
                    }
            pIndex = mirrored_length;
        }
    }

    ptrnMAX = pIndex;

    for(;pIndex<MAX_NOTES;pIndex++){
        playQ[pIndex].pitch = -1;
        playQ[pIndex].velocity = -1;
    }

    pruint8_tf("Play Notes:\n");
    for(pIndex = 0; pIndex < ptrnMAX; pIndex++)
        pruint8_tf("#%2i || P: %2i // V: %2i\n", pIndex, playQ[pIndex].pitch, playQ[pIndex].velocity);
    pruint8_tf("-----------\n");
    for(pIndex = 0; pIndex < ptrnMAX; pIndex++)
        pruint8_tf("#%2i || P: %2i // V: %2i\n", pIndex, playQ[pIndex].pitch, playQ[pIndex].velocity);
    pruint8_tf("\n");

    return;
}



void MIDI_note_off(uint8_t pitch){
    pruint8_tf("OFF: %3i |", pitch);
    
    USART = 0x80;               //note off msg
    USART = pitch;              //note pitch msg
    USART = 0x00;               //don't care velocity msg
    
}

void MIDI_note_on(uint8_t pitch, uint8_t velocity){
    pruint8_tf("ON:  %3i |", pitch);
    
    USART = 0x90;               //note on msg
    USART = pitch;              //note pitch msg
    USART = velocity;           //note velocity msg
    
}

void killNote(){
    MCUgateTMR = -1;               //reset and stop gate timer
    MIDI_note_off(curPitch);    //turn off note
    curPitch = 0;               //reset note flag
}

void playNote(){
    MCUplayTMR = 0;                                     //reset and start play timer

    if (curPitch)
        killNote();                                 //stop current note

    if (rptCNT > UIrptMAX){
        rptCNT = 0;
        ptrnCNT += 1;
//        pruint8_tf("\nrepeat OF - ptrnCNT = %i\n", ptrnCNT);
        if (ptrnCNT > ptrnMAX - 1){
            ptrnCNT = 0;
            octCNT += 1;
      //      pruint8_tf("\ptrn OF\n");
                if (octCNT > UIoctMAX){
                    octCNT = 0;
          //          pruint8_tf("\oct OF\n");
                    if (UIpattern == 8) //random
                        createPattern(testList);
                }
        }
    }

    ptrnCNT = ptrnCNT % ptrnMAX;

    curPitch = playQ[ptrnCNT].pitch + octCNT*octScale;

    MIDI_note_on(curPitch, playQ[ptrnCNT].velocity);

    MCUgateTMR = 0;                                    //reset and start gate timer

    rptCNT += 1;                                    //increment indecies


    return;
}

void uint8_tERRUPT_playTMR(){
    playNote();
}

void uint8_tERRUPT_gateTMR(){
    MCUgateTMR = -1;                   //reset and stop gate timer
    if (curPitch){                   //If a note is on...
        MIDI_note_off(curPitch);    //Turn off note
        curPitch = 0;               //Reset note flag
    }
}

void resetBeat(){
    //reset and stop counters
    MCUgateTMR = -1;
    MCUplayTMR = -1;

    //reset indecies
    rptCNT = 0;
    ptrnCNT = 0;
    octCNT = 0;

    //play note
    playNote();
}

void keyPress(){
    add_note_in_full_order(testList, MCU_USART_pitch, MCU_USART_velocity);   //add note to list
    createPattern(testList);                    //rebuild pattern
    if (testList->note_max == 1)                   //if it's the first note, reset the beat
        resetBeat();
}

void keyRelease(){
    listRMV(testList, MCU_USART_pitch);                //remove note from list
    createPattern(testList);                    //rebuild pattern
    if (testList->note_max < 1){                    //if list is empty, stop all timers
        MCUplayTMR = -1;
        MCUgateTMR = -1;
    }
}



uint8_t main(){
    pruint8_tf("Start...!\n");

    pruint8_tf("Initialize List\n");
    testList = listINIT();

    uint8_t cycle = 0;
    uint8_t go = 1;
    uint8_t event = 0;

    UIoctMAX = 2;
    ptrnMAX = 0;
    UIrptMAX = 3;
    UIpattern = 11;

    MCUgateTMR = -1;
    MCUplayTMR = -1;

    UIgateMAX = 1;
    UIplayMAX = 3;
    srand ( time(NULL) );

    MCU_USART_pitch = 10;
    MCU_USART_velocity = 2;
    keyPress();

    MCU_USART_pitch = 5;
    MCU_USART_velocity = 2;
    keyPress();

    MCU_USART_pitch = 16;
    MCU_USART_velocity = 2;
    keyPress();

    MCU_USART_pitch = 18;
    MCU_USART_velocity = 2;
    keyPress();


    pruint8_tf("\n");
 //   pruint8_tf("ptrnMAX = %i\n", ptrnMAX);

    while(cycle<70){
        pruint8_tf("#%3i| ", cycle);
        event = 0;
        if (MCUgateTMR >= UIgateMAX)
            killNote();
        if (MCUplayTMR >= UIplayMAX)
            playNote();

        if (MCUgateTMR != -1)
            MCUgateTMR++;
        if (MCUplayTMR != -1)
            MCUplayTMR++;

        pruint8_tf("\n");
        cycle++;
    }



 //   pruint8_tListT(testList,0);

    return 0;

}
*/