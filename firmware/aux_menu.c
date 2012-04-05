// Copyright (c) 2012, David Tuzman, All Right Reserved

#include "aux_menu.h"

#include <stdbool.h>
#include <stdint.h>

#include "hardware.h"
#include "arpeggiator.h"

bool aux_restart_delay(){
	bool LED_on = 0;
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
        if (get_switch_edge(SWITCH_ENCODER) == EDGE_RISE){
            set_eeprom_restart_delay(delay_value_in_ms);
			set_seven_segment_LEDs(delay_value_in_ms);
            return true;
        }
	
		//press pushbutton to cancel and exit
		if (get_switch_edge(SWITCH_PUSHBUTTON) == EDGE_RISE){
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
	while(get_switch_edge(SWITCH_ENCODER) != EDGE_RISE){
		read_hardware();
	}
}

bool aux_logs() {
	set_LED_on(LED_DECIMAL_POINT_0);
	set_LED_on(LED_DECIMAL_POINT_1);
	
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

void aux_enter(){
	const uint8_t mode_max = 10;
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
		else if (get_switch_edge(SWITCH_ENCODER) == EDGE_RISE) {
		    switch(mode) {
				case 0: exit_mode = aux_logs();
				        break;
				case 1: exit_mode = aux_restart_delay();
				        break;
				
				default: break;
			}
			aux_exit(exit_mode);
			return;
		}
		else if (get_switch_edge(SWITCH_PUSHBUTTON) == EDGE_RISE) {
			aux_exit(exit_mode);
		    return;
		}			
		    
		
	}
}