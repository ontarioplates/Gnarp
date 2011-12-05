#ifndef SERIAL_MIDI_H
#define SERIAL_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <inttypes.h>
#include "./xnorMIDI/midi.h"

typedef enum {QUARTER, QUARTER_TRIPLET, EIGTH_DOTTED, EIGTH, EIGTH_TRIPLET, SIXTEENTH_DOTTED, SIXTEENTH, SIXTEENTH_TRIPLET}
note_division;

static void configure_note_timing(note_division division, uint16_t duration);
static uint8_t get_next_pitch();
static uint8_t get_next_velocity();
static uint8_t get_current_pitch();
static uint8_t get_current_velocity();
void generic_callback(MidiDevice * device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2);

//initialize serial midi and return the device pointer
MidiDevice* serial_midi_init();
MidiDevice* serial_midi_device();

#ifdef __cplusplus
}
#endif 

#endif
