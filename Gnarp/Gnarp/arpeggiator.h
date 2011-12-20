#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "hardware.h"
#include "note_list.h"
#include "serial_midi.h"

#ifndef ARPEGGIATOR_H_
#define ARPEGGIATOR_H_

#define MAX_PLAY_NOTES 48
#define MAX_NOTE_DURATION 0xFFFF

typedef enum {QUARTER, EIGHTH, SIXTEENTH}
note_time_division;

typedef enum {NONE, DOTTED, TRIPLET}
note_time_variation;

struct NotePlayer
{
    bool    play_status;
    
    uint8_t note_index;
    uint8_t repeat_index;
    uint8_t octave_index;
    
    uint8_t note_max;
    uint8_t repeat_max;
    uint8_t octave_max;
    
    uint16_t start_time_increment;
    uint16_t stop_time_increment;
    
    uint16_t beats_per_minute;
    note_time_division time_division;
    note_time_variation time_variation;
    
    Note*   play_list[MAX_PLAY_NOTES];
};



#endif /* ARPEGGIATOR_H_ */