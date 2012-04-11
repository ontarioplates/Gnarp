// Copyright (c) 2012, David Tuzman, All Rights Reserved

#include "eeprom_comm.h"

#include <stdint.h>
#include <stdbool.h>

#include <avr/eeprom.h>

#include "hardware.h"
#include "beat_clock.h"

#define LOG_GROUP_SIZE 40

#define STORE_ERROR_NO_LOGS 101
#define STORE_ERROR_INSUFFICIENT_SPACE 102

const size_t size_of_log_entry = sizeof(LogEntry);

static uint8_t log_id = 0;
static uint8_t log_group_id;
static uint16_t log_array_index = 0;
static LogEntry log_array[LOG_GROUP_SIZE];
static bool all_logs_valid = false;
static uint16_t log_eeprom_index;

uint8_t get_eeprom_restart_delay() {
    return eeprom_read_byte(EEPROM_ADDR_RESTART_DELAY);
}

void set_eeprom_restart_delay(uint8_t new_value) {
    eeprom_update_byte(EEPROM_ADDR_RESTART_DELAY, new_value);
}

void initialize_logs() {
    //load the eeprom log index from the eeprom
    log_eeprom_index = eeprom_read_word((uint16_t*)EEPROM_ADDR_LOG_INDEX_UNUSED);
    
    //check if the eeprom index is valid
    //if the eeprom index has been reset, the eeprom index will read as all ones
    if (log_eeprom_index == 0xFFFF){
		//reset the eeprom index
        log_eeprom_index = EEPROM_ADDR_LOGS_BEGIN;
		eeprom_update_word(EEPROM_ADDR_LOG_INDEX_UNUSED, log_eeprom_index);
		
		//reset the maximum log group id
		log_group_id = 0;
		eeprom_write_byte(EEPROM_ADDR_LOG_GROUP_ID_UNUSED, log_group_id);
	}
	else
	    log_group_id = eeprom_read_byte(EEPROM_ADDR_LOG_GROUP_ID_UNUSED);
		

	TCD1.CTRLB = 0x00;  //disable compares
	TCD1.CNT = 0x0000;
	TCD1.INTFLAGS |= 0x01;  //reset the overflow flag
	TCD1.CTRLA = 0x07;  //start the counter with 1/1024 clock speed
}    

void create_log_entry(bool midi_in_flag, uint8_t byte0, uint8_t byte1, uint8_t byte2){
	LogEntry* new_log = &log_array[log_array_index];
	
    uint16_t time = TCD1.CNT;  //capture relative timer
    TCD1.CNT = 0;                //reset relative timer
	
	if (TCD1.INTFLAGS & 0x01){   //check if the counter overflowed
	    time = 0xFFFF;  //set time to max if so
	    TCD1.INTFLAGS |= 0x01;  //and reset the overflow flag
	}		
    
	new_log->log_id__bpm = (log_id++ << 10) | (get_BPM() & 0x03FF);

	new_log->timestamp = time;  //set timestamp to relative timer

    new_log->midi_message[0] = byte0;
    new_log->midi_message[1] = byte1;
    new_log->midi_message[2] = byte2;
	
	new_log->midi_in_flag__hardware_seven_segment = (midi_in_flag << 15) | get_seven_segment_LED_state();
	new_log->hardware_LEDs__hardware_pot_value_0 = (get_LEDs_four_bits() << 4) | (get_pot_value(0,0,0xF) & 0x0F);
	
	//log the pots 1-4 positions scaled 0-15
	new_log->hardware_pot_values_1thru4 = 0x0000;
	for (int i = 1; i < 5; i++)
	    new_log->hardware_pot_values_1thru4 |= ( (get_pot_value(i, 0, 0xF) & 0x0F) << (4 * (4 - i)) );

    new_log->hardware_encoder_and_switches = get_raw_encoder_and_switch_info();
    
	//increment array counter and check for overflow
    log_array_index++;
    if (log_array_index >= LOG_GROUP_SIZE){
        all_logs_valid = true;
        log_array_index = 0;
    }
}

bool store_log_block_into_eeprom(){
    uint8_t first_log_array_index;
    uint8_t last_log_array_index;
    uint8_t logs_to_write;
    size_t bytes_to_write;
    uint16_t bytes_available;
    
    if (all_logs_valid){
        first_log_array_index = log_array_index;
        
		if (first_log_array_index == 0)
		    last_log_array_index = LOG_GROUP_SIZE - 1;
		else
		    last_log_array_index = first_log_array_index - 1;
        
        logs_to_write = LOG_GROUP_SIZE;
    }        
    else{
        //if there are no log entries, exit the routine
        if (log_array_index == 0){
			set_seven_segment_LEDs(STORE_ERROR_NO_LOGS);
            return false;
		}			
        
        first_log_array_index = 0;
        last_log_array_index = log_array_index - 1;
        logs_to_write = log_array_index;
    }
    
	
	//calculate how many bytes are available (minus the space to be used for the log_group_id)
    bytes_available = EEPROM_ADDR_LOGS_END - log_eeprom_index;
	if (bytes_available > 0)
	    bytes_available -= 1;
    
    //if there is not enough memory available to store all of the logs,
    //store as many as possible of the most recent longs
    if (logs_to_write * size_of_log_entry > bytes_available){
        logs_to_write = bytes_available / size_of_log_entry;
        
        //if there isn't even enough space to write a single log, exit the routine
        if (logs_to_write == 0){
            set_seven_segment_LEDs(STORE_ERROR_INSUFFICIENT_SPACE);
            return false;
        }
        
        //adjust the first index to fit exactly the number of logs to write
        first_log_array_index = last_log_array_index + LOG_GROUP_SIZE - logs_to_write + 1;
        
        //check for overflow
        if (first_log_array_index >= LOG_GROUP_SIZE)
            first_log_array_index -= LOG_GROUP_SIZE;        
    }
    
    //now there is guaranteed to be enough space in the eeprom to store the prepared logs
	
	//write the log_group_id to eeprom
	eeprom_write_byte(log_eeprom_index, log_group_id);
	
	//routine will succeed, so set indication to eeprom address of the new log block
	set_seven_segment_LEDs(log_eeprom_index);
	
	//increment the eeprom index
	log_eeprom_index++;
	
	if (first_log_array_index < last_log_array_index) {
        //no circular buffer consideration, just one block to write
		bytes_to_write = size_of_log_entry * logs_to_write;
	    eeprom_write_block(&log_array[first_log_array_index], (uint8_t*) log_eeprom_index, bytes_to_write);    
        log_eeprom_index += bytes_to_write;	
	}
	else {
        //calculate the number of bytes for the first chunk
        //write the block to memory
        //increment the eeprom index
        bytes_to_write = size_of_log_entry * (LOG_GROUP_SIZE - first_log_array_index);    
        eeprom_write_block(&log_array[first_log_array_index], (uint8_t*) log_eeprom_index, bytes_to_write);    
        log_eeprom_index += bytes_to_write;
    
        //calculate the number of bytes for the second chunk
        //write the block to memory
        //increment the eeprom index
        bytes_to_write = size_of_log_entry * (last_log_array_index + 1);
        eeprom_write_block(&log_array[0], (uint8_t*) log_eeprom_index, bytes_to_write);    
        log_eeprom_index += bytes_to_write;	    
	}
	
//	log_eeprom_index++;   
    //store the new eeprom index into the eeprom
    eeprom_update_word((uint16_t*)EEPROM_ADDR_LOG_INDEX_UNUSED, log_eeprom_index);
	
	log_group_id++;	
	//write the log_group_id to the unused_id space
	eeprom_update_byte((uint8_t*)EEPROM_ADDR_LOG_GROUP_ID_UNUSED, log_group_id);

    //reset the log array to avoid duplicate logs
	log_array_index = 0;
	log_id = 0;
	all_logs_valid = false;
	
	return true;
}

void initialize_eeprom(uint8_t version){
	eeprom_update_byte(EEPROM_ADDR_VERSION, version);
	
	initialize_logs();
}