// Copyright (c) 2012, David Tuzman, All Right Reserved

#include "test_log.h"

#include <stdint.h>
#include <stdbool.h>

#include <avr/eeprom.h>

#include "hardware.h"

#define EEPROM_MAX_ADDR 0x03FF

#define LOG_BLOCK_SIZE 50

#define LOG_EEPROM_INDEX_ADDR 1
#define LOG_EEPROM_INITAL_ADDR 3

const size_t size_of_log_entry = sizeof(LogEntry);
//
static uint16_t log_array_index = 0;
static LogEntry log_array[LOG_BLOCK_SIZE];
static bool all_logs_valid = false;

static uint16_t log_eeprom_index;

void initialize_test_log() {
    //load the eeprom log index from the eeprom
    log_eeprom_index = eeprom_read_word((uint16_t*)LOG_EEPROM_INDEX_ADDR);
    
    //check if the eeprom index is valid
    //if the eeprom index has been reset, the eeprom index will read as all ones
    if (log_eeprom_index == 0xFFFF)
        log_eeprom_index = LOG_EEPROM_INITAL_ADDR;
}    

void create_log_entry(bool midi_in_flag, uint8_t byte0, uint8_t byte1, uint8_t byte2){
    //uint16_t time = TCD0.CNT;  //capture relative timer
    //TCD0.CNT = 0;                //reset relative timer
       
    //increment log index and check for overflow
    log_array_index++;
    if (log_array_index >= LOG_BLOCK_SIZE){
        all_logs_valid = true;
        log_array_index = 0;
    }        
    
    //log_array[log_array_index].timestamp = time;    //set timestamp to relative timer
        
    log_array[log_array_index].midi_in_flag = midi_in_flag;
    log_array[log_array_index].midi_message[0] = byte0;
    log_array[log_array_index].midi_message[1] = byte1;
    log_array[log_array_index].midi_message[2] = byte2;
    log_array[log_array_index].hardware_snapshot = *get_hardware_manager_ptr();
}

void store_log_block_into_eeprom(){
    uint8_t first_log_array_index;
    uint8_t last_log_array_index;
    uint8_t logs_to_write;
    size_t bytes_to_write;
    uint8_t bytes_available;
    
    if (all_logs_valid){
        first_log_array_index = log_array_index + 1;
        if (first_log_array_index >= LOG_BLOCK_SIZE)
            first_log_array_index = 0;
        
        last_log_array_index = log_array_index;
        
        logs_to_write = LOG_BLOCK_SIZE;
    }        
    else{
        //if there are no log entries, exit the routine
        if (log_array_index == 0)
            return;
        
        first_log_array_index = 0;
        last_log_array_index = log_array_index;
        logs_to_write = log_array_index + 1;
    }
    
    bytes_available = EEPROM_MAX_ADDR - log_eeprom_index;
    
    //if there is not enough memory available to store all of the logs,
    //store as many as possible of the most recent longs
    if (logs_to_write * size_of_log_entry > bytes_available){
        logs_to_write = bytes_available / size_of_log_entry;
        
        //if there isn't even enough space to write a single log, exit the routine
        if (logs_to_write == 0){
            //write some routine for LED indication
            return;
        }
        
        //adjust the first index to fit exactly the number of logs to write
        first_log_array_index = last_log_array_index + LOG_BLOCK_SIZE - logs_to_write + 1;
        
        //check for overflow
        if (first_log_array_index >= LOG_BLOCK_SIZE)
            first_log_array_index -= LOG_BLOCK_SIZE;        
    }
    
    //now there is guaranteed to be enough space in the eeprom to store the prepared logs
    
    //calculate the number of bytes for the first chunk
    //write the block to memory
    //increment the eeprom index
    bytes_to_write = size_of_log_entry * (LOG_BLOCK_SIZE - first_log_array_index);    
    eeprom_write_block(&log_array[first_log_array_index], (uint8_t*) log_eeprom_index, bytes_to_write);    
    log_eeprom_index += bytes_to_write;
    
    //if there is no circular buffer consideration, exit the routine
    //write some LED indication routine
    if (first_log_array_index < last_log_array_index)
        return;
    
    //calculate the number of bytes for the second chunk
    //write the block to memory
    //increment the eeprom index
    bytes_to_write = size_of_log_entry * (last_log_array_index + 1);   
    eeprom_write_block(&log_array[first_log_array_index], (uint8_t*) log_eeprom_index, bytes_to_write);    
    log_eeprom_index += bytes_to_write;
    
    //store the new eeprom index into the eeprom
    eeprom_write_word((uint16_t*)LOG_EEPROM_INDEX_ADDR, log_eeprom_index);
    
    //write some LED indication routine

}