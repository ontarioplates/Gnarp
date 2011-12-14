#include "serial_midi.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "stdlib.h"

#define MIDI_CHAN 0

static MidiDevice midi_device;

void serial_midi_send(MidiDevice* device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
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

MidiDevice * serial_midi_device() {
   return &midi_device;
}

void serial_midi_init(){
   //send up the device
   midi_device_init(&midi_device);
   midi_register_noteon_callback(&midi_device, generic_noteon_callback);
   midi_register_catchall_callback(&midi_device, generic_catchall_callback);
   
 //  initialize_note_timer();
 //  send_all_notes_off();
}

void generic_noteon_callback(MidiDevice * device, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
    volatile uint8_t bytes[3] = {inByte0, inByte1, inByte2};
        
    volatile static uint8_t j = 0;
    
    if (j < 0xFF)
        j++;
    else
        j = 0;
    

}

void generic_catchall_callback(MidiDevice * device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
    volatile uint8_t bytes[3] = {inByte0, inByte1, inByte2};
        
    volatile static uint8_t j = 0;
    
    if (j < 0xFF)
        j++;
    else
        j = 0;
    

}