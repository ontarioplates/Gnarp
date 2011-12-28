#ifndef SERIAL_MIDI_H
#define SERIAL_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif 


#include "stdlib.h"
#include "./xnorMIDI/midi_device.h"
#include "arpeggiator.h"

#define MIDI_CHAN 0

void initialize_serial_midi(MidiDevice* midi_device, Sequencer* sequencer);
MidiDevice* get_midi_device();

#ifdef __cplusplus
}
#endif 

#endif
