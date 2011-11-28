#ifndef SERIAL_MIDI_H
#define SERIAL_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <inttypes.h>
#include "./xnorMIDI/midi.h"

//initialize serial midi and return the device pointer
MidiDevice* serial_midi_init();

#ifdef __cplusplus
}
#endif 

#endif
