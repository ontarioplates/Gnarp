// Copyright (c) 2012, David Tuzman, All Rights Reserved

#include "main.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "hardware.h"
#include "serial_midi.h"
#include "arpeggiator.h"
#include "beat_clock.h"
#include "aux_menu.h"

#define SOFTWARE_VERSION 0

static Sequencer sequencer;
static MidiDevice midi_device;
static HardwareManager* manager_ptr;

ISR(USARTD1_RXC_vect){
    midi_device_input(&midi_device,1,&(USARTD1.DATA));
    midi_device_process(&midi_device);
}

//interrupt to start the next note
ISR(TCC0_CCB_vect){
    //continue to the next note without restarting
    continue_sequencer(&sequencer, 0);
}

//interrupt to stop the current note
ISR(TCC0_CCC_vect){
    //stop the sequencer note without a full stop
    stop_sequencer(&sequencer, 0);
}

//interrupt for sequencer restart delay
ISR(TCC1_CCA_vect){
    //stop and reset the counter
    TCC1.CTRLA = 0;
    TCC1.CNT = 0;
    
    //disable CCA compare
    TCC1.CTRLB &= ~0x10;
    
    //restart the sequencer
    continue_sequencer(&sequencer, 1);
}


void fake_midi_noteon_input(MidiDevice* midi_device, uint8_t pitch, uint8_t velocity){
    const uint8_t noteon_byte1 = 144;
    
    midi_device_input(midi_device, 1, &noteon_byte1);
    midi_device_input(midi_device, 1, &pitch);
    midi_device_input(midi_device, 1, &velocity);
    midi_device_process(midi_device);
}

void fake_midi_noteoff_input(MidiDevice* midi_device, uint8_t pitch, uint8_t velocity){
    const uint8_t noteoff_byte1 = 128;
    
    midi_device_input(midi_device, 1, &noteoff_byte1);
    midi_device_input(midi_device, 1, &pitch);
    midi_device_input(midi_device, 1, &velocity);
    midi_device_process(midi_device);
}


int main(void) {
    const uint16_t initial_BPM = 120;
    
    uint16_t BPM_add;
    
    manager_ptr = initialize_hardware();

	initialize_eeprom(SOFTWARE_VERSION);
	
    initialize_sequencer(&sequencer);
    
    initialize_serial_midi(&midi_device, &sequencer);
    
    initialize_beat_clock(initial_BPM);
    
    set_seven_segment_LEDs(get_BPM());
    
    read_hardware();
    
    if (get_switch_state(SWITCH_TOGGLE))
        enable_sequencer(&sequencer);

    while(1){
        read_hardware();
        
        //press the encoder and pushbutton to enter edit mode for the delayed restart
        if (get_switch_state(SWITCH_ENCODER) && get_switch_state(SWITCH_PUSHBUTTON)){
            aux_enter();
            set_seven_segment_LEDs(get_BPM());
        }            
        
        if (get_switch_state(SWITCH_ENCODER))
            BPM_add = 5;
        else
            BPM_add = 1;
        
        if (get_encoder() == TURN_CW){
            if (increment_BPM(BPM_add)){
                bpm_change_postprocess(&sequencer);
                set_seven_segment_LEDs(get_BPM());
            }                
        }
        else if (get_encoder() == TURN_CCW){
            if (decrement_BPM(BPM_add)){
                bpm_change_postprocess(&sequencer);
                set_seven_segment_LEDs(get_BPM());
            }                
        }
        
        if (get_switch_edge(SWITCH_PUSHBUTTON) == EDGE_RISE)
            continue_sequencer(&sequencer, 1);

/*        if (get_switch_edge(SWITCH_PUSHBUTTON) == EDGE_RISE)
		    fake_midi_noteon_input(&midi_device, get_BPM(), 100);
        if (get_switch_edge(SWITCH_PUSHBUTTON) == EDGE_FALL)
		    fake_midi_noteoff_input(&midi_device, get_BPM(), 100);
 */           
        if (get_switch_edge(SWITCH_TOGGLE) == EDGE_FALL)
            disable_sequencer(&sequencer);
            
        if (get_switch_edge(SWITCH_TOGGLE) == EDGE_RISE)
            enable_sequencer(&sequencer);
    }
    

               
    return 0;
}