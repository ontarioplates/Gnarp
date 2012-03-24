// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef BEAT_CLOCK_H_
#define BEAT_CLOCK_H_

#include <stdint.h>
#include <stdbool.h>

#define BPM_MAX 400
#define BPM_MIN 20

void initialize_beat_clock(uint16_t BPM);
bool increment_BPM(int add_me);
bool decrement_BPM(int subtract_me);
uint16_t get_BPM();

#endif /* BEAT_CLOCK_H_ */