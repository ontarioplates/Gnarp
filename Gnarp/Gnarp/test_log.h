// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef TEST_LOG_H_
#define TEST_LOG_H_

#include <stdint.h>
#include <stdbool.h>

#include "hardware.h"

typedef struct LogEntry LogEntry;

struct LogEntry {
	uint16_t timestamp;
	bool midi_in_flag;
	uint8_t midi_message[3];
    HardwareManager hardware_snapshot;
};





#endif /* TEST_LOG_H_ */