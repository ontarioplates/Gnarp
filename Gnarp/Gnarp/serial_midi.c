#include "serial_midi.h"

#include <avr/io.h>

static Sequencer* stored_sequencer;
static MidiDevice* stored_midi_device;

MidiDevice* get_midi_device() {
   return stored_midi_device;
}

void serial_midi_send(MidiDevice* midi_device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
   //we always send the first byte
    while (!(USARTD1.STATUS & 0x20)){}; // Wait for empty transmit buffer
    USARTD1.DATA = inByte0;
   //if cnt == 2 or 3 we send the send byte
   if(cnt > 1) {
      while (!(USARTD1.STATUS & 0x20)){}; // Wait for empty transmit buffer
      USARTD1.DATA = inByte1;
   }
   //if cnt == 3 we send the third byte
   if(cnt == 3) {
      while (!(USARTD1.STATUS & 0x20)){}; // Wait for empty transmit buffer
      USARTD1.DATA = inByte2;
   }
}

void noteon_to_arpeggiator(MidiDevice * midi_device, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
    add_note_to_arpeggiator(stored_sequencer, inByte1, inByte2);
}

void noteoff_to_arpeggiator(MidiDevice * midi_device, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
	remove_note_from_arpeggiator(stored_sequencer, inByte1);
}

void initialize_serial_midi(MidiDevice* midi_device, Sequencer* sequencer){
   //set up the device
   midi_device_init(midi_device);
   
   midi_device_set_send_func(midi_device, serial_midi_send);
   midi_register_noteon_callback(midi_device, noteon_to_arpeggiator);
   midi_register_noteoff_callback(midi_device, noteoff_to_arpeggiator);
   
   //all midi messages that are not expected will be sent through to midi out
   midi_register_fallthrough_callback(midi_device, serial_midi_send);
   
   //store the device and sequencer pointers
   stored_sequencer = sequencer;
   stored_midi_device = midi_device;
}

