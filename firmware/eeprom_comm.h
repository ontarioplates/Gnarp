// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef EEPROM_COMM_H_
#define EEPROM_COMM_H_

#include <stdint.h>
#include <stdbool.h>

#include "hardware.h"

#define EEPROM_ADDR_VERSION 0
#define EEPROM_ADDR_LOG_INDEX 2
#define EEPROM_ADDR_RESTART_DELAY 1
#define EEPROM_ADDR_LOGS 10
#define EEPROM_ADDR_MAX 0x03FF

typedef struct LogEntry LogEntry;

struct LogEntry {
    uint16_t timestamp;
    bool midi_in_flag;
    uint8_t midi_message[3];
    HardwareManager hardware_snapshot;
};


uint8_t get_eeprom_restart_delay();
void set_eeprom_restart_delay(uint8_t new_value);
void create_log_entry(bool midi_in_flag, uint8_t byte0, uint8_t byte1, uint8_t byte2);
void store_log_block_into_eeprom();
void initialize_eeprom();


#endif /* EEPROM_COMM_H_ */