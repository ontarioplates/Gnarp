#ifndef SERIAL_MIDI_H
#define SERIAL_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <inttypes.h>
#include "./xnorMIDI/midi.h"

void generic_callback(MidiDevice * device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2);
void generic_noteon_callback(MidiDevice * device, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2);
void generic_catchall_callback(MidiDevice * device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2);

void pre_input_process();

//initialize serial midi and return the device pointer
void serial_midi_init();
MidiDevice* serial_midi_device();

#ifdef __cplusplus
}
#endif 

#endif
