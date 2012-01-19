#ifndef ARPEGGIATOR_H_
#define ARPEGGIATOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "note_list.h"

#define MAX_PLAY_NOTES 48
#define MAX_NOTE_DURATION 0xFF
#define MIDI_OCTAVE 12
#define RAND_BUFF 10

typedef enum {QUARTER, EIGHTH, SIXTEENTH}
note_time_division;

typedef enum {NONE, DOTTED, TRIPLET}
note_time_variation;

struct Sequencer
{
    bool    run_status;
    bool    play_status;
    bool    rebuild_play_list;
    
    uint8_t note_index;
    uint8_t repeat_index;
    uint8_t octave_index;
    
    uint8_t note_max;
    uint8_t repeat_max;
    uint8_t octave_max;
    
    uint16_t start_time_increment;
    uint16_t stop_time_increment;
    
    uint8_t pattern;
    uint8_t duration;
    uint8_t division;
    
    Note*   play_list[MAX_PLAY_NOTES];
    NoteList note_list;
};

typedef struct Sequencer Sequencer;

void adjust_sequencer_to_bpm(Sequencer* sequencer);
void initialize_sequencer(Sequencer* sequencer);
void add_note_to_arpeggiator(Sequencer* sequencer, uint8_t pitch, uint8_t velocity);
void remove_note_from_arpeggiator(Sequencer*, uint8_t pitch);

void continue_sequencer(Sequencer* sequencer, bool restart);
void stop_sequencer(Sequencer* sequencer, bool full_stop);

#endif /* ARPEGGIATOR_H_ */