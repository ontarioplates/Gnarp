// Copyright (c) 2012, David Tuzman, All Right Reserved

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

#define TEST_EEPROM_ADDR (uint16_t*)0

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

void fake_midi_noteff_input(MidiDevice* midi_device, uint8_t pitch, uint8_t velocity){
    const uint8_t noteoff_byte1 = 128;
    
    midi_device_input(midi_device, 1, &noteoff_byte1);
    midi_device_input(midi_device, 1, &pitch);
    midi_device_input(midi_device, 1, &velocity);
    midi_device_process(midi_device);
}

void edit_delayed_restart(){
    //read the current value
    uint8_t delay_value_in_ms = initialize_delayed_restart(257,0);
    
    while(1){
        read_hardware();
        
        //display the delayed_restart value
        set_seven_segment_LEDs(delay_value_in_ms);
        
        //adjust the value in RAM (this should affect the note sounds
        if (get_encoder() == TURN_CW && delay_value_in_ms < 255)
            initialize_delayed_restart(++delay_value_in_ms, 0);
        if (get_encoder() == TURN_CCW && delay_value_in_ms > 0)
            initialize_delayed_restart(--delay_value_in_ms, 0);
        
        
        //press the encoder to save the value into EEPROM and exit edit mode                    
        if (get_encoder_switch_edge() == EDGE_RISE){
            initialize_delayed_restart(257, 1);
            return;
        }            
    }
}

int main(void) {
    const uint16_t initial_BPM = 120;
    
    uint16_t BPM_add;
    
    manager_ptr = initialize_hardware();
    
    initialize_sequencer(&sequencer);
    
    initialize_serial_midi(&midi_device, &sequencer);
    
    initialize_beat_clock(initial_BPM);
    
    set_seven_segment_LEDs(get_BPM());
    
    read_hardware();
    
    if (get_toggle_switch_state())
        enable_sequencer(&sequencer);
    
/*    uint16_t eeprom_data = eeprom_read_byte(TEST_EEPROM_ADDR);
    
    while(1){
        read_hardware();
        
        set_seven_segment_LEDs(eeprom_data);
        
        if (get_encoder() == TURN_CW && eeprom_data < 0xff)
            eeprom_data++;
        if (get_encoder() == TURN_CCW && eeprom_data > 0)
            eeprom_data--;
        
        if (get_encoder_switch_edge() == EDGE_RISE){
            eeprom_write_byte(TEST_EEPROM_ADDR, eeprom_data);
            set_LED_on(LED_STATUS);
        }            
        if (get_encoder_switch_edge() == EDGE_FALL){
            set_LED_off(LED_STATUS);
        }
    }
*/    
    while(1){
        read_hardware();
        
        //press the encoder and pushbutton to enter edit mode for the delayed restart
        if (get_encoder_switch_state() && get_pushbutton_switch_state()){
            edit_delayed_restart();
            set_seven_segment_LEDs(get_BPM());
        }            
        
        if (get_encoder_switch_state())
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
        
        if (get_pushbutton_switch_edge() == EDGE_RISE)
            continue_sequencer(&sequencer, 1);
            
        if (get_toggle_switch_edge() == EDGE_FALL)
            disable_sequencer(&sequencer);
            
        if (get_toggle_switch_edge() == EDGE_RISE)
            enable_sequencer(&sequencer);
    }
    

               
    return 0;
}