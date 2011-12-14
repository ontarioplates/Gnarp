#include "play_list.h"

static PlayList global_play_list;
/*
ISR(TCC0_CCA_vect){
	TCC0.CNT = 0;      //reset beat clock
}

ISR(TCC0_CCB_vect){
	TCC0.CTRLB &= ~0x20;  //disable CCB and CCC interrupts
	TCC0.CTRLB &= ~0x40;
	
	midi_send_noteon(serial_midi_device(),MIDI_CHAN, get_next_pitch(), get_next_velocity());
	
	configure_note_timing(get_pot_value(1,0,7), get_pot_value(0, 10, 0xFFFF));
	
	TCC0.CTRLB |= 0x20;   //enable CCB (note on) interrupt
	TCC0.CTRLB |= 0x40;   //enable CCC (note off) interrupt
}

ISR(TCC0_CCC_vect){
	if sad
	midi_send_noteoff(serial_midi_device(), MIDI_CHAN, get_current_pitch(), get_current_velocity());
	TCC0.CTRLB &= ~0x40;  //disable CCC (note off)
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

static void configure_note_timing(note_division division, uint16_t duration){
	const uint32_t division_numerator[8]   = {1, 2, 3, 1, 1, 3, 1, 1};
	const uint32_t division_denominator[8] = {1, 3, 4, 2, 3, 8, 4, 6};
	volatile uint16_t current_time;
	volatile uint32_t next_note_on_time;
	volatile uint32_t next_cutoff_time;
		
//	Q: 1/1 2/3 3/2
//	E: 1/2 1/3 3/4
//	S: 1/4 1/6 3/8
//  a= q (1) /e (1/2) /s (1/4)
//  b= d (3/2) / x (1) / t (2/3)
//  note length = a^-2 * 2/3^(b-1)   ... too complicated, just make a lookup table/array
	

	
	current_time = TCC0.CNT;    //log current time

	next_note_on_time = TCC0.CCA * division_numerator[division];       //calculate the new length for Compare B (interrupt for new note)
	next_note_on_time = next_note_on_time / division_denominator[division];
	
	next_cutoff_time = next_note_on_time * duration;                  //calculate the new length for Compare C (interrupt for note off)
	next_cutoff_time = next_cutoff_time / 0xFFFF;
	
	if (next_cutoff_time > next_note_on_time)                         //ensure the note cuts off before the next note
		next_cutoff_time = next_note_on_time - 10;
	
	next_cutoff_time += current_time;                                 //add current time to Compare C for final value, check for overflow
	if (next_cutoff_time > TCC0.CCA)
		next_cutoff_time = next_cutoff_time - TCC0.CCA;
	
	next_note_on_time += current_time;                                //add current time to Compare C for final value, check for overflow
	if (next_note_on_time > TCC0.CCA)    //the counter will reset at CCA, so check for overflow
		next_note_on_time = next_note_on_time - TCC0.CCA;
		
	TCC0.CCB = (uint16_t) next_note_on_time;    //set compare B to new value
	TCC0.CCC = (uint16_t) next_cutoff_time;     //set compare C to new value

}

uint8_t get_current_pitch(){
	
}

uint8_t get_next_pitch(){
	
}

uint8_t get_current_velocity(){
	
}

uint8_t get_next_velocity(){
	
}
*/
PlayList* get_play_list(){
	//return global play list pointer for use with all functions
	
	return &global_play_list;
}

Note* get_note_from_play_list(PlayList* play_list){
	//set play status
	//return note to play
	
	if (play_list->count >= MAX_PLAY_NOTES)
	    play_list->count = 0;
	
	play_list->play_status = 1;
	
	return play_list->notes[play_list->play_index];
}

void initialize_play_list(PlayList* play_list){
	//reset the play list note pointers, counters, and flags
	
	uint8_t i;
	for (i = 0; i < MAX_PLAY_NOTES; i++)
	    play_list->notes[i] = NULL;
	play_list->count = 0;
	play_list->play_index = 0;
	play_list->play_status = 0;
}

void initialize_arpeggiator(){
	//initialize note list in linkedlist.c
	//initialize play list
	
    initialize_note_list();
	initialize_play_list(get_play_list());
}

void build_play_list(PlayList* play_list, NoteList* note_list){
	
	//builds the play list according to pattern selection (pot0)
	
	uint8_t play_list_index = 0;
    Note* current_note;
	
	uint8_t note_list_size = note_list->count;
	uint8_t random_list_depth;      //index for random pattern
	
    uint8_t i;
    uint8_t mirror = 0;


    switch(get_pot_value(0, 0, 4)){
        //Asc pitch
        case 0:
            for(current_note = note_list->head_pitch; current_note; current_note=current_note->next_note_by_pitch)
				play_list->notes[play_list_index++] = current_note;
            break;

        //Desc pitch
        case 1:
            for(current_note = note_list->tail_pitch; current_note; current_note=current_note->previous_note_by_pitch)
                play_list->notes[play_list_index++] = current_note;
            break;

        //Asc trigger
        case 2:
            for(current_note = note_list->head_trigger; current_note; current_note=current_note->next_note_by_trigger)
                play_list->notes[play_list_index++] = current_note;
            break;

        //Desc trigger
        case 3:
            for(current_note = note_list->tail_trigger; current_note; current_note=current_note->previous_note_by_trigger)
                play_list->notes[play_list_index++] = current_note;
            break;

        //random
        case 4:
            for(; play_list_index < RAND_BUFF; play_list_index++){
                random_list_depth = rand() % note_list_size;
                current_note = note_list->head_pitch;
                for(i = 0; i < random_list_depth; i++)
                    current_note = current_note->next_note_by_pitch;
                play_list->notes[play_list_index++] = current_note;
            }
            break;
    }


    //option to mirror the pattern
    if (mirror){
        uint8_t mirrored_length;
        uint8_t edge_scale;
        uint8_t k;

        if (mirror == 2){                         //double edge
            mirrored_length = play_list_index*2;
            edge_scale = 1;
        }
        if (mirror == 1){                         //single edge
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
                play_list->notes[play_list_index + k] = play_list->notes[play_list_index - k + edge_scale];
            }
            play_list_index = mirrored_length;
        }
    }

    play_list->count = play_list_index;     //set play list count appropriately

    return;
}


void input_note_on(PlayList* play_list, uint8_t pitch, uint8_t velocity){
	
	//called whenever a new MIDI noteon message is recieved
	
	//adds the new note to the note list (if it's not a duplicate)
	//rebuilds play list and starts playing if it's the first note in the list
	
	bool first_note = 0;
	
	if (get_note_list()->count == 0)        //check for empty note list
	    first_note = 1;
	
	add_note_in_full_order(get_note_list(),pitch,velocity);     //add note into note list
	
	if (first_note){         //if it's the first note in the note list, build the play list and start playing by setting the play interrupt flag
	    build_play_list(get_play_list(),get_note_list());
		TCC0.INTFLAGS &= 0x20;
	}		
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
                play_list->notes[play_list_index++]->velocity = curNode->velocity;
            }
            break;

        //Desc pitch
        case 1:
            for(curNode = note_list->tail_pitch; curNode; curNode=curNode->previous_note_by_pitch){
                playQ[pIndex].pitch = curNode->pitch;
                play_list->notes[play_list_index++]->velocity = curNode->velocity;
            }
            break;

        //Asc trigger
        case 2:
            for(curNode = note_list->head_trigger; curNode; curNode=curNode->next_note_by_trigger){
                playQ[pIndex].pitch = curNode->pitch;
                play_list->notes[play_list_index++]->velocity = curNode->velocity;
            }
            break;

        //Desc trigger
        case 3:
            for(curNode = note_list->tail_trigger; curNode; curNode=curNode->previous_note_by_trigger){
                playQ[pIndex].pitch = curNode->pitch;
                play_list->notes[play_list_index++]->velocity = curNode->velocity;
            }
            break;

        //random
        case 4:
            cnt = note_list->count;
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
    if (testList->count == 1)                   //if it's the first note, reset the beat
        resetBeat();
}

void keyRelease(){
    listRMV(testList, MCU_USART_pitch);                //remove note from list
    createPattern(testList);                    //rebuild pattern
    if (testList->count < 1){                    //if list is empty, stop all timers
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