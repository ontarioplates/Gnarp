#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "hardware.h"
#include "note_list.h"

#ifndef PLAY_LIST_H_
#define PLAY_LIST_H_

#define MAX_PLAY_NOTES 48

struct PlayList
{
    uint8_t length;
	bool    play_status;
	uint8_t current_index;
	uint8_t next_index;
    Note*   notes[MAX_PLAY_NOTES];
};



#endif /* PLAY_LIST_H_ */