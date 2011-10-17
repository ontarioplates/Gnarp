#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>


//initialize serial midi and return the device pointer
MidiDevice* serial_midi_init(uint16_t clockScale, bool out, bool in);

#ifdef __cplusplus
}
#endif

#endif
