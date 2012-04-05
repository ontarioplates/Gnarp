// Copyright (c) 2012, David Tuzman, All Rights Reserved

#include "arpeggiator.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include "./xnorMIDI/midi.h"

#include "serial_midi.h"
#include "hardware.h"
#include "eeprom_comm.h"

void change_restart_delay(uint8_t new_value){
    //(1 tick / 1024 cycles)(24*10^6 cycles / 1 second)(1 second / 1000 milliseconds)(x milliseconds / 1 compare)
    const float compare_factor = 23.4375;
	
    //multiply for counter compare value
    int16_t compare_value = compare_factor * new_value;
    
    //stop and reset the counter
    TCC1.CTRLA = 0;
    TCC1.CNT = 0;
    
    //disable all compares and clear all interrupt flags
    TCC1.CTRLB &= ~0xF0;
    TCC1.INTFLAGS |= 0xF0;
    
    //assign the value to trip the interrupt
    TCC1.CCA = compare_value;
       
    //configure CCA as low-level interrupt
    TCC1.INTCTRLB &= ~0x03;
    TCC1.INTCTRLB |= 0x01;
}

void initialize_restart_delay() {
	uint8_t stored_delay = get_eeprom_restart_delay();
	
	if (stored_delay == 0xFF)
	    stored_delay = 20;
		
	set_eeprom_restart_delay(stored_delay);
	
    change_restart_delay(stored_delay);
}

static void restart_delay(){    
    //stop and reset the counter
    TCC1.CTRLA = 0;
    TCC1.CNT = 0;
    
    //clear the restart flag
    TCC1.INTFLAGS |= 0x10;
    
    //enable restart timer compare
    TCC1.CTRLB |= 0x10;
    
    //start timer with clock divide @ 1024
    TCC1.CTRLA = 0x07;
}

static uint8_t final_pitch(Sequencer* sequencer){
    uint16_t final_pitch;
    final_pitch = sequencer->play_list[sequencer->note_index]->pitch + MIDI_OCTAVE*(sequencer->octave_index);
    while (final_pitch > 255)
            final_pitch -= 12;
    return (uint8_t) final_pitch;
}

static uint8_t final_velocity(Sequencer* sequencer){
    uint16_t final_velocity;
    final_velocity = sequencer->play_list[sequencer->note_index]->velocity;
    return (uint8_t) final_velocity;
}

static uint8_t final_channel(Sequencer* sequencer){
    uint16_t final_channel;
    final_channel = sequencer->play_list[sequencer->note_index]->channel;
    return (uint8_t) final_channel;
}

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
        //quarter note
        case 0:    break;
        
        //dotted eighth
        case 3: new_start_time_increment *= 3;
                new_start_time_increment /= 4;
                break;
        
        //quarter triplet
        case 5: new_start_time_increment *= 2;
                new_start_time_increment /= 3;
                break;
        
        //eighth
        case 1: new_start_time_increment /= 2;
                break;
                
        //dotted 16th
        case 4: new_start_time_increment *= 3;
                new_start_time_increment /= 8;
                break;
        
        //eighth triplet
        case 6: new_start_time_increment /= 3;
                break;
        
        //sixteenth                
        case 2: new_start_time_increment /= 4;
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
    
    //disable CCB (note-on) and CCC (note-off) interrupts
    TCC0.INTCTRLB &= ~0x30;
    TCC0.INTCTRLB &= ~0x0C;
    
    //clear CCB (note-on) and CCC (note-off) interrupt flags
    TCC0.INTFLAGS |= 0x20;
    TCC0.INTFLAGS |= 0x40;
    
    //initialize the note list
    initialize_note_list(&(sequencer->note_list));
    
    //initialize the delayed restart timer, set to value>256 to disable update
    initialize_restart_delay();
    
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
    sequencer->enable = 0;
    sequencer->rebuild_play_list = 1;
    
    //calculate the time increments
    calculate_start_time_increment(sequencer);
    calculate_stop_time_increment(sequencer);
}

static void build_play_list(Sequencer* sequencer){
    
    //builds the play list according to pattern selection
    
    NoteList* note_list = &(sequencer->note_list);
    
    uint8_t play_list_index = 0;
    Note* current_note;
    
    uint8_t note_list_size = note_list->length;
    
    uint8_t random_order[note_list_size];
    uint8_t j;
    uint8_t temp;
    uint8_t random_list_depth;      //index for random pattern
    
    uint8_t i;
    
    bool mirror = 0;
    uint8_t pattern = 0;
    
    
    switch(sequencer->pattern){
        case 0:
            pattern = 0;
            mirror = 0;
            break;
        case 1:
            pattern = 1;
            mirror = 0;
            break;
        case 2:
            pattern = 0;
            mirror = 1;
            break;
        case 3:
            pattern = 2;
            mirror =  0;
            break;
        case 4:
            pattern = 3;
            mirror = 0;
            break;
        case 5:
            pattern = 2;
            mirror = 1;
            break;
        case 6:
            pattern = 4;
            mirror = 0;
            break;
        case 7:
            pattern = 5;
            mirror = 0;
            break;
    }

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
            for (i = 0; i<note_list_size; i++)
                random_order[i] = i;
            for (i = 0; i<note_list_size; i++){
                j = rand() % note_list_size;
                temp = random_order[i];
                random_order[i] = random_order[j];
                random_order[j] = temp;
            }            
            for (i=0; i<note_list_size; i++){
                current_note = note_list->head_pitch;
                for (j = 0; j < random_order[i]; j++)
                    current_note = current_note->next_note_by_pitch;
                sequencer->play_list[play_list_index++] = current_note;
            }

            break;
            
        //case 5:
            
    }

    //option to mirror the pattern
    if (mirror){
        uint8_t mirrored_length;
        uint8_t edge_scale;
        uint8_t k;

        if (MIRROR_EDGE_DOUBLE == true){
            //double edge
            mirrored_length = play_list_index*2;
            edge_scale = 1;
        }
        if (MIRROR_EDGE_DOUBLE == false){
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
    sequencer->note_max = play_list_index - 1;
    
    //check if the list is now shorter than the current note_index
//    while (sequencer->note_index > sequencer->note_max)
//        sequencer->note_index -= sequencer->note_max;
    
    sequencer->rebuild_play_list = 0;
    
    return;
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
        
        //build a new random playlist if necessary
        if (sequencer->pattern == 4)
            build_play_list(sequencer);
    }    
}


void set_sequencer_parameters(Sequencer* sequencer, bool restart){
    //read the new values from the pots
    volatile uint8_t octave_max_new = get_pot_value(POT_SEL_OCTAVE, POT_MIN_OCTAVE, POT_MAX_OCTAVE);
    volatile uint8_t repeat_max_new = get_pot_value(POT_SEL_REPEAT, POT_MIN_REPEAT, POT_MAX_REPEAT);
    volatile uint8_t division_new = get_pot_value(POT_SEL_DIVISION, POT_MIN_DIVISION, POT_MAX_DIVISION);
    volatile uint8_t duration_new = get_pot_value(POT_SEL_DURATION, POT_MIN_DURATION, POT_MAX_DURATION);
    volatile uint8_t pattern_new = get_pot_value(POT_SEL_PATTERN, POT_MIN_PATTERN, POT_MAX_PATTERN);
    
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
    
    if (update_start_time_increment || restart)
        calculate_start_time_increment(sequencer);
    if (update_stop_time_increment || restart)
        calculate_stop_time_increment(sequencer);
}

void continue_sequencer(Sequencer* sequencer, bool restart){
    //disable CCB (note-on) and CCC (note-off) timer compares
    TCC0.CTRLB &= ~0x20; 
    TCC0.CTRLB &= ~0x40;
    
    //disable CCB (note-on) and CCC (note-off) interrupts
    TCC0.INTCTRLB &= ~0x30;
    TCC0.INTCTRLB &= ~0x0C;
    
    //clear CCB (note-on) and CCC (note-off) interrupt flags
    TCC0.INTFLAGS |= 0x20;
    TCC0.INTFLAGS |= 0x40;
    
    volatile uint32_t current_time;
    volatile uint32_t next_start_time;
    volatile uint32_t next_stop_time;
    
    current_time = (uint32_t) TCC0.CNT;
    
    
    if (!(sequencer->enable) || (sequencer->note_list.length == 0)){
        sequencer->run_status = 0;
        return;
    }        
    
    
    //turn off the current note if it is still playing
    if (sequencer->play_status){
        midi_send_noteoff(get_midi_device(),final_channel(sequencer),final_pitch(sequencer),final_velocity(sequencer));
        set_LED_off(LED_STATUS);
        sequencer->play_status = 0;
    }
        
    //load the new hardware settings from the user
    set_sequencer_parameters(sequencer, restart);
    
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
    
    //enable CCB (note-on) and CCC (note-off) compares
    TCC0.CTRLB |= 0x20; 
    TCC0.CTRLB |= 0x40;    
    
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
    midi_send_noteon(get_midi_device(), final_channel(sequencer), final_pitch(sequencer), final_velocity(sequencer));
    set_LED_on(LED_STATUS);
    
    //set play flag
    sequencer->play_status = 1;
    
    //set run flag
    sequencer->run_status = 1;
    
    //enable CCB (note-on) and CCC (note-off) interrupts (mid-level)
    TCC0.INTCTRLB |= 0x20;
    TCC0.INTCTRLB |= 0x08;
}

void stop_sequencer(Sequencer* sequencer, bool full_stop){
    //disable CCC (note-off) timer compares
    TCC0.CTRLB &= ~0x20;
    
    //disable CCB (note-on) and CCC (note-off) interrupts
    TCC0.INTCTRLB &= ~0x30;
    TCC0.INTCTRLB &= ~0x0C;
    
    //clear CCC (note-off) interrupt flags
    TCC0.INTFLAGS |= 0x40;
    
    if (!(sequencer->enable) || (sequencer->note_list.length == 0)){
        sequencer->run_status = 0;
        return;
    }        
    
    //stop the current note if it's playing
    if (sequencer->play_status){
        midi_send_noteoff(get_midi_device(), final_channel(sequencer), final_pitch(sequencer), final_velocity(sequencer));
        set_LED_off(LED_STATUS);
        sequencer->play_status = 0;
    }
    
    //if this is a full stop clear the run status of the sequencer
    //if this is not a full stop, reenable the note on interrupt
    if (full_stop)
        sequencer->run_status = 0;
    else    
        TCC0.INTCTRLB |= 0x08;
}

void add_note_to_arpeggiator(Sequencer* sequencer, uint8_t pitch, uint8_t velocity, uint8_t channel){
    //try to add the note to the note list.
    //if successful, flag to rebuild the play list
    //if the sequencer isn't currently running, (re)start it with a delay
    
    if (insert_note(&(sequencer->note_list), pitch, velocity, channel)){
        sequencer->rebuild_play_list = 1;   

        if (!sequencer->run_status)
            restart_delay();                  
    }
}

void remove_note_from_arpeggiator(Sequencer* sequencer, uint8_t pitch){
    //try to remove the note from the list
    //if successful, set the rebuild flag
    //if the note list is now empty, fully stop the sequencer
    
    //if note is playing, stop it
    if (sequencer->play_list[sequencer->note_index]->pitch == pitch)
        stop_sequencer(sequencer,0);
    
    //if the note was successfully removed, flag to rebuild the playlist
    if (remove_note_by_pitch(&(sequencer->note_list), pitch)){
        sequencer->rebuild_play_list = 1;
        
        //if there are no notes in the list, completely stop the sequencer
        if (sequencer->note_list.length == 0)
            stop_sequencer(sequencer, 1);
    }    
    
}

void bpm_change_postprocess(Sequencer* sequencer){
    //if the sequencer is running while the bpm changes, calculate the new start/stop increments and reset the 
    if (!(sequencer->enable) || !(sequencer->run_status))
        return;
    
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
    
    //turn off the current note if it is still playing
 /*   if (sequencer->play_status){
        midi_send_noteoff(get_midi_device(),MIDI_CHAN,final_pitch(sequencer),final_velocity(sequencer));
        set_LEDs_off(0,0,0,1);
        sequencer->play_status = 0;
    }
        
    //load the new hardware settings from the user
    set_sequencer_parameters(sequencer, restart);
    */
    
    //CALCULATE NEW TIME INCREMENTS
    calculate_start_time_increment(sequencer);
    calculate_stop_time_increment(sequencer);
 
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
    
  /*  //rebuild the pattern if necessary
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
    midi_send_noteon(get_midi_device(),MIDI_CHAN,final_pitch(sequencer),final_velocity(sequencer));
    set_LEDs_on(1,0,0,0);
    
    //set play flag
    sequencer->play_status = 1;
    
    //set run flag
    sequencer->run_status = 1;
    */
    //enable note on and note off interrupts
    TCC0.CTRLB |= 0x20; 
    TCC0.CTRLB |= 0x40;    
            
        
}

void disable_sequencer(Sequencer* sequencer){
    stop_sequencer(sequencer, 1);
    sequencer->enable = 0;
    
    //start all currently held notes (enter THRU)
    for (Note* i = sequencer->note_list.head_trigger; i != NULL; i = i->next_note_by_trigger){
        midi_send_noteon(get_midi_device(), i->channel, i->pitch, i->velocity);
    }
    
    serial_midi_config_bypass(get_midi_device());
    
}

void enable_sequencer(Sequencer* sequencer){
    serial_midi_config_active(get_midi_device());
    
    //stop all currently held notes (exit THRU)
    for (Note* i = sequencer->note_list.head_trigger; i != NULL; i = i->next_note_by_trigger){
        midi_send_noteoff(get_midi_device(), i->channel, i->pitch, i->velocity);
    }
    
    sequencer->enable = 1;
    continue_sequencer(sequencer, 1);    
}