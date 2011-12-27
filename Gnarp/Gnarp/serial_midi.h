#ifndef SERIAL_MIDI_H
#define SERIAL_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "stdlib.h"
#include "./xnorMIDI/midi.h"
#include "arpeggiator.h"

#define MIDI_CHAN 0

void serial_midi_init();
MidiDevice* serial_midi_device();

#ifdef __cplusplus
}
#endif 

#endif
