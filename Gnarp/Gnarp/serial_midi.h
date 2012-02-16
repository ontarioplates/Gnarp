// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef SERIAL_MIDI_H
#define SERIAL_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdlib.h>

#include "./xnorMIDI/midi_device.h"

#include "arpeggiator.h"

#define MIDI_CHAN 0

void initialize_serial_midi(MidiDevice* midi_device, Sequencer* sequencer);
MidiDevice* get_midi_device();
void serial_midi_config_active(MidiDevice* midi_device);
void serial_midi_config_bypass(MidiDevice* midi_device);

#ifdef __cplusplus
}
#endif 

#endif
