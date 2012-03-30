// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef EEPROM_COMM_H_
#define EEPROM_COMM_H_

#include <stdint.h>
#include <stdbool.h>

#include "hardware.h"

#define EEPROM_ADDR_VERSION 1000   //1 byte
#define EEPROM_ADDR_LOG_INDEX_UNUSED 1002 //2 bytes - the lowest available eeprom address
#define EEPROM_ADDR_LOG_GROUP_ID_UNUSED 1004 //1 byte - the lowest available log group ID
#define EEPROM_ADDR_RESTART_DELAY 1001 //1 bytes
#define EEPROM_ADDR_LOGS_BEGIN 0
#define EEPROM_ADDR_LOGS_END 999
#define EEPROM_ADDR_MAX 0x03FF

typedef struct LogEntry LogEntry;
typedef struct HardwareSnapshot HardwareSnapshot;

struct LogEntry {
	uint8_t log_id; /**< Identification number.  MSB indicates a MIDI IN*/
    uint16_t timestamp; /**< Amount of time since the last log*/
    uint8_t midi_message[3]; /**< Midi Message at moment of log*/
	
	uint8_t hardware_LEDs; /**< Each bit represents one LED: MSB(status, dp0, dp1, dp2)LSB*/
	uint16_t hardware_seven_segment; /**< The seven segment display at moment of log*/
	
	uint8_t hardware_pot_values[5]; /**< Positions of the pots at moment of log (scaled to 8 bits)*/
	
	//turn state:
	//0 - static
	//1 - CW
	//2 - CCW
	
	//switch state:
	//0 - off
	//1 - on
	//2 - edge off
	//3 - edge on
	
	uint8_t hardware_encoder; /**< Bits 0:1 indicate the turn state.  Bits 2:3 indicate the switch state*/
    uint8_t hardware_push_and_toggle; /**< Bits 0:1 indicate pushbutton state.  Bits 2:3 indicate toggle state*/
};



uint8_t get_eeprom_restart_delay();
void set_eeprom_restart_delay(uint8_t new_value);
void create_log_entry(bool midi_in_flag, uint8_t byte0, uint8_t byte1, uint8_t byte2);
bool store_log_block_into_eeprom();
void initialize_eeprom(uint8_t version);


#endif /* EEPROM_COMM_H_ */