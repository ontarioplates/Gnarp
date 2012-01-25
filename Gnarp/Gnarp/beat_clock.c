// Copyright (c) 2012, David Tuzman, All Right Reserved

#include "beat_clock.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "arpeggiator.h"

ISR(TCC0_CCA_vect){
    //reset beat clock
    TCC0.CNT = 0;
}

ISR(TCC0_CCD_vect){
    //midi_send_clock(serial_midi_device());  //send clock tick
    //calculate time for next clock tick
}

static uint16_t BPM;

void configure_beat_clock_timer(){
    //system clock = 24MHz; cyc/beat = 1.44Trillion/BPM
    const uint32_t numerator = 1440000000;
    
    //corresponds to division value for TCxx.CTRLA
    const uint32_t clock_divide[8] = {0, 1, 2, 4, 8, 64, 256, 1024};
            
    volatile uint8_t current_clock_divide_select = (TCC0.CTRLA & 0x0F);
    volatile uint8_t new_clock_divide_select = 1;
    volatile uint32_t adjusted_count = 0;
    
    //compare value for no divider
    volatile uint32_t cycle_per_beat = numerator/BPM;
    
    volatile uint32_t compare_value = cycle_per_beat/clock_divide[new_clock_divide_select];
    
    //run loop until compare_value is a 16 bit number
    while (compare_value > 0xFFFF){
        //try the next highest divider
        new_clock_divide_select++;
        
        //unless you've explored all of them
        if (new_clock_divide_select > 7)
            return;
        
        compare_value = cycle_per_beat/clock_divide[new_clock_divide_select];
    }
    
    //stop and reset the counter
    TCC0.CTRLA = 0;
    TCC0.CNT = 0;
 /*   
    This is a draft of code to allow the beat clock to
    continue without restarting if the BPM changes
    
    if (TCC0.CTRLA){
        if (!(current_clock_divide_select == new_clock_divide_select)){
            //stop and scale the timer count if the divider must change
            TCC0.CTRLA = 0x00;
            adjusted_count = TCC0.CNT * clock_divide[new_clock_divide_select];
            adjusted_count = adjusted_count / clock_divide[current_clock_divide_select];
            while (adjusted_count > compare_value)
                adjusted_count = adjusted_count - compare_value;
            TCC0.CNT = (uint16_t) adjusted_count;
        }
        else{
            //otherwise, just stop the timer 
            TCC0.CTRLA = 0x00;
        }            
    }            
*/    
    
    //set the new compare value for beat
    TCC0.CCA = (uint16_t) compare_value;
    //set the new compare value for midi-clock ticks
    TCC0.CCD = (uint16_t) compare_value/24;
    
    //enable CCA (beat count) and CCD (midi tick) interrupt
    TCC0.CTRLB |= 0x10;
    TCC0.CTRLB |= 0x80;
    
    //set the new clock divider and start the clock
    TCC0.CTRLA = new_clock_divide_select;

    return;
}

void initialize_beat_clock(uint16_t new_BPM){
    BPM = new_BPM;
    configure_beat_clock_timer();
    
    //configure CCA and CCD as mid-level interrupts
    TCC0.INTCTRLB &= ~0xC0;
    TCC0.INTCTRLB |= 0x80;
    TCC0.INTCTRLB &= ~0x03;
    TCC0.INTCTRLB |= 0x02;
    
    //enable CCA and CCD interrupts
    TCC0.CTRLB |= 0x80;
    TCC0.CTRLB |= 0x10;
}

uint16_t get_BPM(){
    return BPM;
}

void increment_BPM(){
    BPM += 1;
    configure_beat_clock_timer();
}

void decrement_BPM(){
    BPM -= 1;
    configure_beat_clock_timer();
}



