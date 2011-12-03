#ifndef SERIAL_MIDI_H
#define SERIAL_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <inttypes.h>
#include "./xnorMIDI/midi.h"

//initialize serial midi and return the device pointer
MidiDevice* serial_midi_init();
MidiDevice* serial_midi_device();
//void serial_midi_send(MidiDevice * device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2);

#ifdef __cplusplus
}
#endif 

#endif
