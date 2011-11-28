#include "serial_midi.h"
#include <avr/interrupt.h>
#include "stdlib.h"

static MidiDevice midi_device;

void serial_midi_send(MidiDevice * device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
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

MidiDevice * serial_midi_device(void) {
   return &midi_device;
}

MidiDevice* serial_midi_init(){
   //send up the device
   midi_device_init(&midi_device);
   midi_device_set_send_func(&midi_device, serial_midi_send);

   return serial_midi_device();
}

