// Copyright (c) 2012, David Tuzman, All Right Reserved

#include "main.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>

#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "hardware.h"
#include "serial_midi.h"
#include "arpeggiator.h"
#include "beat_clock.h"
#include "eeprom_comm.h"

#define SOFTWARE_VERSION 0
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

void fake_midi_noteoff_input(MidiDevice* midi_device, uint8_t pitch, uint8_t velocity){
    const uint8_t noteoff_byte1 = 128;
    
    midi_device_input(midi_device, 1, &noteoff_byte1);
    midi_device_input(midi_device, 1, &pitch);
    midi_device_input(midi_device, 1, &velocity);
    midi_device_process(midi_device);
}

bool aux_restart_delay(){
	bool LED_on;
	realtime_count_start();
	
    //read the current value
    uint8_t delay_value_in_ms = get_eeprom_restart_delay();
    
    while(1){
		if (realtime_count_compare(50)){
    		if (LED_on){
    		    set_LED_off(LED_DECIMAL_POINT_0);
				set_LED_off(LED_DECIMAL_POINT_1);
			}				
    		else{
    		    set_LED_on(LED_DECIMAL_POINT_0);
				set_LED_on(LED_DECIMAL_POINT_1);
			}				
			LED_on = !LED_on;
    	}	
						
        read_hardware();
        
        //display the delayed_restart value
        set_seven_segment_LEDs(delay_value_in_ms);
        
        //adjust the value in RAM (this should affect the note sounds
        if (get_encoder() == TURN_CW && delay_value_in_ms < 255)
            change_restart_delay(++delay_value_in_ms);
        if (get_encoder() == TURN_CCW && delay_value_in_ms > 0)
            change_restart_delay(--delay_value_in_ms);
        
        
        //press the encoder to save the value into EEPROM and exit edit mode                    
        if (get_encoder_switch_edge() == EDGE_RISE){
            set_eeprom_restart_delay(delay_value_in_ms);
			set_seven_segment_LEDs(delay_value_in_ms);
            return true;
        }
	
		//press pushbutton to cancel and exit
		if (get_pushbutton_switch_edge() == EDGE_RISE){
		    initialize_restart_delay();
			delay_value_in_ms = get_eeprom_restart_delay();
            set_seven_segment_LEDs(delay_value_in_ms);
			return false;
		}			        
    }
}

void log_storage_test(uint8_t group_size){
	int logs_made = 0;
	
	while (logs_made<group_size){
		create_log_entry(logs_made,logs_made,logs_made,logs_made);
	    set_seven_segment_LEDs(++logs_made);
	}
	
	read_hardware();
	while(get_encoder_switch_edge() != EDGE_RISE){
		read_hardware();
	}
}

bool aux_logs() {
	return store_log_block_into_eeprom();
}

void aux_exit(bool success){
	realtime_count_stop();
	
	set_LED_off(LED_DECIMAL_POINT_0);
	set_LED_off(LED_DECIMAL_POINT_1);
	set_LED_off(LED_DECIMAL_POINT_2);
	
	if (success){
	    for (LED_choose i = 0; i < 3; i++){
		    for (int j = 0; j < 3; j++){
		        set_LED_on(3-i);
		        realtime_pause(40);
		        set_LED_off(3-i);
		        realtime_pause(40);
		    }			
	    }
	}
	else {
		for (int j = 0; j <3; j++){
		    set_LED_on(LED_DECIMAL_POINT_0);
	        set_LED_on(LED_DECIMAL_POINT_1);
	        set_LED_on(LED_DECIMAL_POINT_2);
			realtime_pause(100);
	        set_LED_off(LED_DECIMAL_POINT_0);
        	set_LED_off(LED_DECIMAL_POINT_1);
        	set_LED_off(LED_DECIMAL_POINT_2);
			realtime_pause(100);
		}			
	}
}

void aux_menu(){
	const uint8_t mode_max = 1;
	uint8_t mode = 0;
	bool exit_mode = false;
	bool LED_on = false;
	
	set_seven_segment_LEDs(mode);
	
	realtime_count_start();
	
	while(1){
		if (realtime_count_compare(80)){
    		if (LED_on)
    		    set_LED_off(LED_DECIMAL_POINT_0);
    		else
    		    set_LED_on(LED_DECIMAL_POINT_0);
			LED_on = !LED_on;
    	}
		
		read_hardware();
		
		if (get_encoder() == TURN_CW){
		    mode++;
		    if (mode > mode_max)
			    mode = 0;
			set_seven_segment_LEDs(mode);
		}
		else if (get_encoder() == TURN_CCW){
			if (mode == 0)
			    mode = mode_max;
			else
			    mode--;
			set_seven_segment_LEDs(mode);
		}
		else if (get_encoder_switch_edge() == EDGE_RISE) {
		    switch(mode) {
				case 0: //exit_mode = aux_logs();
				        break;
				case 1: exit_mode = aux_restart_delay();
				        break;
				
				default: continue;
			}
			aux_exit(exit_mode);
			return;
		}
		else if (get_pushbutton_switch_edge() == EDGE_RISE) {
			aux_exit(exit_mode);
		    return;
		}			
		    
		
	}
}


int main(void) {
    const uint16_t initial_BPM = 120;
    
    uint16_t BPM_add;
    
    manager_ptr = initialize_hardware();
	
	initialize_eeprom(SOFTWARE_VERSION);
	
/*	uint16_t val = 0;
	
	set_seven_segment_LEDs(val);
	realtime_count_start();
	while(1){
		read_hardware();
		
		if (get_encoder_switch_edge() == EDGE_RISE || val > 999)
		    val = 0;
		
		if (realtime_count_compare(10))
		    set_seven_segment_LEDs(++val);
		
		
	}
	
	while(1){
		read_hardware();
		if (get_encoder_switch_state() && get_pushbutton_switch_state()){
            aux_menu();
            set_seven_segment_LEDs(222);
        }
	}
	
	uint8_t log_groups_made = 0;
	
	while(1){
		read_hardware();
		
		set_seven_segment_LEDs(log_groups_made);
		
		if (get_encoder_switch_edge() == EDGE_RISE){
		    log_storage_test(1);
			log_groups_made++;
		}
		
		if (get_pushbutton_switch_edge() == EDGE_RISE)
		    store_log_block_into_eeprom();
	}		
    */
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
            aux_menu();
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

/*        if (get_pushbutton_switch_edge() == EDGE_RISE)
		    fake_midi_noteon_input(&midi_device, get_BPM(), 100);
        if (get_pushbutton_switch_edge() == EDGE_FALL)
		    fake_midi_noteoff_input(&midi_device, get_BPM(), 100);
 */           
        if (get_toggle_switch_edge() == EDGE_FALL)
            disable_sequencer(&sequencer);
            
        if (get_toggle_switch_edge() == EDGE_RISE)
            enable_sequencer(&sequencer);
    }
    

               
    return 0;
}