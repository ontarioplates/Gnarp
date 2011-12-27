#include "serial_midi.h"

static MidiDevice midi_device;

MidiDevice * serial_midi_device() {
   return &midi_device;
}

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

void noteon_to_arpeggiator(MidiDevice * device, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
    //try to add the note to the note list.
    //if successful, flag to rebuild the play list
    //if it's the first note, restart the sequencer
    
    if (insert_note(get_note_list(), inByte1, inByte2)){
        set_rebuild_play_list(get_sequencer(), 1);
        
        if (get_note_list_length(get_note_list()) == 1)
            continue_sequencer(get_sequencer(), 1);
    }
    
}

void noteoff_to_arpeggiator(MidiDevice * device, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
    //try to remove the note from the list
    //if successful, set the rebuild flag
    //if the note list is now empty, fully stop the sequencer
    
    if (remove_note_by_pitch(get_note_list(), inByte1)){
        set_rebuild_play_list(get_sequencer(), 1);
        
        if (get_note_list_length(get_note_list()) == 0)
            stop_sequencer(get_sequencer(), 1);
    }
}

void serial_midi_init(){
   //set up the device
   midi_device_init(&midi_device);
   
   midi_device_set_send_func(&midi_device, serial_midi_send);
   midi_register_noteon_callback(&midi_device, noteon_to_arpeggiator);
   midi_register_noteoff_callback(&midi_device, noteoff_to_arpeggiator);
   
   //all midi messages that are not expected will be send through to midi out
   midi_register_fallthrough_callback(&midi_device, serial_midi_send);

}

