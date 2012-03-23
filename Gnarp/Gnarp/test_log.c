// Copyright (c) 2012, David Tuzman, All Right Reserved

#include "test_log.h"

#include <stdint.h>
#include <stdbool.h>

#include <avr/eeprom.h>

#include "hardware.h"

#define NUM_LOGS 50

static uint16_t log_array_index = 0;
static LogEntry log_array[NUM_LOGS];
static bool all_logs_valid = false;

void create_log_entry(bool midi_in_flag, uint8_t byte0, uint8_t byte1, uint8_t byte2){
	//uint16_t time = TCD0.CNT;  //capture relative timer
	//TCD0.CNT = 0;				//reset relative timer
	
	
	//increment log index and check for overflow
	log_array_index++;
	if (log_array_index >= NUM_LOGS){
		all_logs_valid = true;
		log_array_index = 0;
	}		
	
	//log_array[log_array_index].timestamp = time;	//set timestamp to relative timer
		
	log_array[log_array_index].midi_in_flag = midi_in_flag;
	log_array[log_array_index].midi_message[0] = byte0;
	log_array[log_array_index].midi_message[1] = byte1;
	log_array[log_array_index].midi_message[2] = byte2;
	log_array[log_array_index].hardware_snapshot = *get_hardware_manager_ptr();
}

void store_logs_into_eeprom(){
	uint8_t first_index;
	uint8_t last_index;
	
	last_index = log_array_index;
	
	if (all_logs_valid)
		first_index = log_array_index + 1;
	else
		first_index = 0;
		
	if (first_index >= NUM_LOGS)
		first_index -= NUM_LOGS;
	
	uint8_t block_size_1 = sizeof(LogEntry) * (NUM_LOGS - first_index);
	uint8_t block_size_2 = sizeof(LogEntry) * (last_index + 1);
	
	eeprom_write_block(&log_array[first_index], , block_size_1);
	eeprom_write_block(&log_array, , block_size_2);	
	
}