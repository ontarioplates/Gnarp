#include "main.h"


ISR(USARTD1_RXC_vect){
//    static uint8_t new_byte[1];
//    new_byte[0] = USARTD1.DATA;
    midi_device_input(serial_midi_device(),1,&(USARTD1.DATA));
    midi_device_process(serial_midi_device());
}


void fake_midi_noteon_input(uint8_t pitch, uint8_t velocity){
    const uint8_t noteon_byte1 = 144;
    
    midi_device_input(serial_midi_device(), 1, &noteon_byte1);
    midi_device_input(serial_midi_device(), 1, &pitch);
    midi_device_input(serial_midi_device(), 1, &velocity);
    midi_device_process(serial_midi_device());
}

void fake_midi_noteff_input(uint8_t pitch, uint8_t velocity){
    const uint8_t noteoff_byte1 = 128;
    
    midi_device_input(serial_midi_device(), 1, &noteoff_byte1);
    midi_device_input(serial_midi_device(), 1, &pitch);
    midi_device_input(serial_midi_device(), 1, &velocity);
    midi_device_process(serial_midi_device());
}

int main(void) {
    const initial_BPM = 120;
    volatile Sequencer* sequencer = get_sequencer();

    uint8_t pitch = 100;
    
    initialize_hardware();
    initialize_note_list();
    initialize_sequencer();
    serial_midi_init();
    initialize_beat_clock(initial_BPM);
    set_seven_segment_LEDs(get_BPM());

    while(1){
        read_hardware();
        
        if (get_encoder() == TURN_CW)
            pitch++;
        else if (get_encoder() == TURN_CCW)
            pitch -= 1;    
        
        set_seven_segment_LEDs((uint16_t) pitch);
        
        if (get_encoder_switch_edge() == EDGE_RISE)
            fake_midi_noteon_input(pitch,100);
        
        if (get_pushbutton_switch_edge() == EDGE_RISE)
            fake_midi_noteff_input(pitch,100);
        
 //       if (get_toggle_switch_state())
   //         stop_sequencer(sequencer, 1);

    }
  /*  
    while(1){
        read_hardware();
        
        if (get_encoder() == TURN_CW){
            increment_BPM();
            set_seven_segment_LEDs(get_BPM());
        }
        else if (get_encoder() == TURN_CCW){
            decrement_BPM();
            set_seven_segment_LEDs(get_BPM());
        }
        
        if (get_pushbutton_switch_edge() == EDGE_RISE)
            continue_sequencer(sequencer, 1);
    };
   */            
    return 0;
}