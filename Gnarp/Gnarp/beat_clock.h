// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef BEAT_CLOCK_H_
#define BEAT_CLOCK_H_

#include <stdint.h>

void initialize_beat_clock(uint16_t BPM);
void increment_BPM();
void decrement_BPM();
uint16_t get_BPM();

#endif /* BEAT_CLOCK_H_ */