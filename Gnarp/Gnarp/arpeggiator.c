#include "arpeggiator.h"

#include "serial_midi.h"
#include "./xnorMIDI/midi.h"
#include "hardware.h"

#include <avr/interrupt.h>
#include <avr/io.h>

static void calculate_start_time_increment(Sequencer* sequencer){
    //0 - qtr note (no division)
    //1 - dotted 8th (3/4)
    //2 - triplet qtr (2/3)
    //3 - 8th (1/2)
    //4 - dotted 16th (3/8)
    //5 - triplet 8th (1/3)
    //6 - 16th (1/4)
    
    //start with the time for a single beat
    volatile uint32_t new_start_time_increment = (uint32_t) TCC0.CCA;
    
    //based on the division selection, scale the time
    switch(sequencer->division){
        case 0:    break;
        
        case 1:    new_start_time_increment *= 3;
                new_start_time_increment /= 4;
                break;
                
        case 2: new_start_time_increment *= 2;
                new_start_time_increment /= 3;
                break;
                
        case 3: new_start_time_increment /= 2;
                break;
                
        case 4: new_start_time_increment *= 3;
                new_start_time_increment /= 8;
                break;
        
        case 5: new_start_time_increment /= 3;
                break;
                
        case 6: new_start_time_increment /= 4;
                break;
    }
    
    //divide by repeat parameter to fit in all the repeats
    new_start_time_increment /= (sequencer->repeat_max + 1);
    
    //load the start time increment into the sequences
    sequencer->start_time_increment = (uint16_t) new_start_time_increment;
}

static void calculate_stop_time_increment(Sequencer* sequencer){
    volatile uint32_t new_stop_time_increment = (uint32_t) (sequencer->start_time_increment) * sequencer->duration;
    new_stop_time_increment = new_stop_time_increment / MAX_NOTE_DURATION;
    
    sequencer->stop_time_increment = (uint16_t) new_stop_time_increment;
}

//Reset all data in the sequencer
void initialize_sequencer(Sequencer* sequencer){  
    uint8_t i;
    
    //disable CCB (note on) and CCC (note off) interrupts
    TCC0.CTRLB &= ~0x20; 
    TCC0.CTRLB &= ~0x40;
    
    //configure CCB and CCC as mid-level interrupts
    TCC0.INTCTRLB &= ~0x30;
    TCC0.INTCTRLB |= 0x20;
    TCC0.INTCTRLB &= ~0x0C;
    TCC0.INTCTRLB |= 0x08;
    
	//initialize the note list
	initialize_note_list(&(sequencer->note_list));
	
    //empty the play list
    for (i = 0; i < MAX_PLAY_NOTES; i++)
        sequencer->play_list[i] = NULL;
    
    //reset all parameters and indeces
    sequencer->note_index = 0;
    sequencer->octave_index = 0;
    sequencer->repeat_index = 0;
    sequencer->note_max = 0;
    sequencer->repeat_max = 0;
    sequencer->octave_max = 0;
    sequencer->start_time_increment = 0;
    sequencer->stop_time_increment = 0;
    sequencer->pattern = 0;
    sequencer->duration = 0;
    sequencer->division = 0;
    sequencer->play_status = 0;
    
    //link the note list to the player and flag to rebuild the play list
    sequencer->rebuild_play_list = 1;
    
    //calculate the time increments
    calculate_start_time_increment(sequencer);
    calculate_stop_time_increment(sequencer);
}

void set_rebuild_play_list(Sequencer* sequencer, bool new_flag){
    sequencer->rebuild_play_list = new_flag;
}

static void reset_play_list_indeces(Sequencer* sequencer){
    sequencer->octave_index = 0;
    sequencer->note_index = 0;
    sequencer->repeat_index = 0;
}

static void increment_play_list_indeces(Sequencer* sequencer){
    //increment repeat count
    sequencer->repeat_index += 1;
    
    //if note has repeated enough times, reset the repeat index and increment the note index to get the next note to play
    if (sequencer->repeat_index > sequencer->repeat_max){
        sequencer->repeat_index = 0;
        sequencer->note_index += 1;
    }
    
    //if the play list is at the end, reset the note index and increment the octave index
    if (sequencer->note_index > sequencer->note_max){
        sequencer->note_index = 0;
        sequencer->octave_index += 1;
    }
    
    //if the last octave is reached, reset the octave index
    if (sequencer->octave_index > sequencer->octave_max){
        sequencer->octave_index = 0;
    }    
}


static void set_sequencer_parameters(Sequencer* sequencer){
    //correlate pots to control each parameter of the arpeggiator
    const uint8_t octave_pot_sel= 0;
    const uint8_t octave_pot_min = 0;
    const uint8_t octave_pot_max = 3;
    
    const uint8_t repeat_pot_sel= 1;
    const uint8_t repeat_pot_min = 0;
    const uint8_t repeat_pot_max = 4;
    
    const uint8_t division_pot_sel= 2;
    const uint8_t division_pot_min = 0;
    const uint8_t division_pot_max = 6;
    
    const uint8_t duration_pot_sel= 3;
    const uint16_t duration_pot_min = 0;
    const uint16_t duration_pot_max = 0xFFFF;
    
    const uint8_t pattern_pot_sel= 4;
    const uint8_t pattern_pot_min = 0;
    const uint8_t pattern_pot_max = 6;
    
    //read the new values from the pots
    uint8_t octave_max_new = get_pot_value(octave_pot_sel, octave_pot_min, octave_pot_max);
    uint8_t repeat_max_new = get_pot_value(repeat_pot_sel, repeat_pot_min, repeat_pot_max);
    uint8_t division_new = get_pot_value(division_pot_sel,division_pot_min,division_pot_max);
    uint16_t duration_new = get_pot_value(duration_pot_sel, duration_pot_min, duration_pot_max);
    uint8_t pattern_new = get_pot_value(pattern_pot_sel, pattern_pot_min, pattern_pot_max);
    
    bool update_start_time_increment = 0;
    bool update_stop_time_increment = 0;
    
    //flag to calculate new interrupt times if necessary
    if (sequencer->repeat_max != repeat_max_new){
        update_start_time_increment = 1;
        update_stop_time_increment = 1;
    }
    
    if (sequencer->division != division_new){
        update_start_time_increment = 1;
        update_stop_time_increment = 1;
    }
    
    if (sequencer->duration != duration_new){
        update_stop_time_increment = 1;
    }
    
    if (sequencer->pattern != pattern_new){
        sequencer->rebuild_play_list = 1;
    }
    
    //load the new parameters into the arpeggiator
    sequencer->octave_max = octave_max_new;
    sequencer->repeat_max = repeat_max_new;
    sequencer->division = division_new;
    sequencer->duration = duration_new;
    sequencer->pattern = pattern_new;
    
    if (update_start_time_increment)
        calculate_start_time_increment(sequencer);
    if (update_stop_time_increment)
        calculate_stop_time_increment(sequencer);
}

static void build_play_list(Sequencer* sequencer){
    
    //builds the play list according to pattern selection
    
    NoteList* note_list = &(sequencer->note_list);
    uint8_t pattern = sequencer->pattern;
    
    uint8_t play_list_index = 0;
    Note* current_note;
    
    uint8_t note_list_size = note_list->length;
    uint8_t random_list_depth;      //index for random pattern
    
    uint8_t i;
    uint8_t mirror = 0;

    pattern = 0;

    switch(pattern){
        //Asc pitch
        case 0:
            for(current_note = note_list->head_pitch; current_note; current_note=current_note->next_note_by_pitch)
                sequencer->play_list[play_list_index++] = current_note;
            break;

        //Desc pitch
        case 1:
            for(current_note = note_list->tail_pitch; current_note; current_note=current_note->previous_note_by_pitch)
                sequencer->play_list[play_list_index++] = current_note;
            break;

        //Asc trigger
        case 2:
            for(current_note = note_list->head_trigger; current_note; current_note=current_note->next_note_by_trigger)
                sequencer->play_list[play_list_index++] = current_note;
            break;

        //Desc trigger
        case 3:
            for(current_note = note_list->tail_trigger; current_note; current_note=current_note->previous_note_by_trigger)
                sequencer->play_list[play_list_index++] = current_note;
            break;

        //random
        case 4:
            for(; play_list_index < RAND_BUFF; play_list_index++){
                random_list_depth = rand() % note_list_size;
                current_note = note_list->head_pitch;
                for(i = 0; i < random_list_depth; i++)
                    current_note = current_note->next_note_by_pitch;
                sequencer->play_list[play_list_index++] = current_note;
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
                sequencer->play_list[play_list_index + k] = sequencer->play_list[play_list_index - k + edge_scale];
            }
            play_list_index = mirrored_length;
        }
    }
    //set play list note_max appropriately
    sequencer->note_max = play_list_index;
    
    //check if the list is now shorter than the current note_index
    while (sequencer->note_index > sequencer->note_max)
        sequencer->note_index -= sequencer->note_max;
    
    sequencer->rebuild_play_list = 0;
    
    return;
}

void continue_sequencer(Sequencer* sequencer, bool restart){

    //disable noteon and noteoff interrupts
    TCC0.CTRLB &= ~0x20; 
    TCC0.CTRLB &= ~0x40;
    
    //clear noteon and noteoff interrupt flags
    TCC0.INTFLAGS |= 0x20;
    TCC0.INTFLAGS |= 0x40;
    
    volatile uint32_t current_time;
    volatile uint32_t next_start_time;
    volatile uint32_t next_stop_time;
    
	//if there are no notes in the list, don't do anything
    if (sequencer->note_list.length == 0)
        return;
    
    
    current_time = (uint32_t) TCC0.CNT;
    /*
    //if this is a continuation, log the count time
    //otherwise restart the counter
    if (!restart)
        current_time = (uint32_t) TCC0.CNT;
    else{
        current_time = 0;
        TCC0.CNT = 0;
    }
    */
    
    
    //turn off the current note if it is still playing
    if (sequencer->play_status){
        midi_send_noteoff(get_midi_device(),MIDI_CHAN,sequencer->play_list[sequencer->note_index]->pitch,sequencer->play_list[sequencer->note_index]->velocity);
        sequencer->play_status = 0;
    }
        
    //load the new hardware settings from the user
    set_sequencer_parameters(sequencer);
    
    //compute next compare values
    next_start_time = current_time + sequencer->start_time_increment;
    next_stop_time = current_time + sequencer->stop_time_increment;
    
    //check for overflow
    if (next_start_time > TCC0.CCA)
        next_start_time = next_start_time - TCC0.CCA;
    if (next_stop_time > TCC0.CCA)
        next_stop_time = next_stop_time - TCC0.CCA;
    
    //assign values to compare registers
    TCC0.CCB = (uint16_t) next_start_time;
    TCC0.CCC = (uint16_t) next_stop_time;
    
    //rebuild the pattern if necessary
    if (sequencer->rebuild_play_list)
        build_play_list(sequencer);
        
    //if this is a continuation, increment the play list indeces
    //otherwise, reset them all
    if (!restart)
        increment_play_list_indeces(sequencer);
    else{
        reset_play_list_indeces(sequencer);
    }
    
    //send midi message to start the note
    midi_send_noteon(get_midi_device(),MIDI_CHAN,sequencer->play_list[sequencer->note_index]->pitch,sequencer->play_list[sequencer->note_index]->velocity);
    
    set_LEDs_on(0,0,0,1);
    
    //set play flag
    sequencer->play_status = 1;
    
    //set run flag
    sequencer->run_status = 1;
    
    //enable note on and note off interrupts
    TCC0.CTRLB |= 0x20; 
    TCC0.CTRLB |= 0x40;
}

void stop_sequencer(Sequencer* sequencer, bool full_stop){
    //disable CCB (note on) and CCC (note off) interrupts
    TCC0.CTRLB &= ~0x20; 
    TCC0.CTRLB &= ~0x40;
    
    //clear note off interrupt flag
    TCC0.INTFLAGS |= 0x40;
    
    //stop the current note if it's playing
    if (sequencer->play_status){
        midi_send_noteoff(get_midi_device(),MIDI_CHAN,sequencer->play_list[sequencer->note_index]->pitch,sequencer->play_list[sequencer->note_index]->velocity);
        set_LEDs_off(0,0,0,1);
        sequencer->play_status = 0;
    }
    
    //if this is a full stop clear the run status of the sequencer
    //if this is not a full stop, renable the note on interrupt
    if (full_stop)
        sequencer->run_status = 0;
    else    
        TCC0.CTRLB |= 0x20;
    
}

void add_note_to_arpeggiator(Sequencer* sequencer, uint8_t pitch, uint8_t velocity){
    //try to add the note to the note list.
    //if successful, flag to rebuild the play list
    //if it's the first note, restart the sequencer
    
    if (insert_note(&(sequencer->note_list), pitch, velocity)){
		sequencer->rebuild_play_list = 1;
        
		if (sequencer->note_list.length == 1)
            continue_sequencer(sequencer, 1);
    }
}

void remove_note_from_arpeggiator(Sequencer* sequencer, uint8_t pitch){
    //try to remove the note from the list
    //if successful, set the rebuild flag
    //if the note list is now empty, fully stop the sequencer
    
    if (remove_note_by_pitch(&(sequencer->note_list), pitch)){
        sequencer->rebuild_play_list = 1;
        
        if (sequencer->note_list.length == 0)
            stop_sequencer(sequencer, 1);
    }	
	
}

void adjust_sequencer_to_bpm(Sequencer* sequencer){
    //if the sequencer is running while the bpm changes, restart the sequencer
    if (sequencer->run_status)
        continue_sequencer(sequencer, 1);
}

//interrupt to start the next note
ISR(TCC0_CCB_vect){
    //continue to the next note without restarting
    continue_sequencer(get_sequencer(), 0);
}

//interrupt to stop the current note
ISR(TCC0_CCC_vect){
    //stop the sequencer note without a full stop
    stop_sequencer(get_sequencer(), 0);
}

