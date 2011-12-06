#include "arpeggiator.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"

ISR(TCC0_CCA_vect){
	TCC0.CNT = 0;      //reset beat clock
}

ISR(TCC0_CCB_vect){
	TCC0.CTRLB &= ~0x20;  //disable CCB and CCC interrupts
	TCC0.CTRLB &= ~0x40;
	
	midi_send_noteon(serial_midi_device(),MIDI_CHAN, get_next_pitch(), get_next_velocity());
	
	configure_note_timing(get_pot_value(1,0,7), get_pot_value(0, 10, 0xFFFF));
	
	TCC0.CTRLB |= 0x20;   //enable CCB (note on) interrupt
	TCC0.CTRLB |= 0x40;   //enable CCC (note off) interrupt
}

ISR(TCC0_CCC_vect){
	midi_send_noteoff(serial_midi_device(), MIDI_CHAN, get_current_pitch(), get_current_velocity());
	TCC0.CTRLB &= ~0x40;  //disable CCC (note off)
}

ISR(TCC0_CCD_vect){
	//midi_send_clock(serial_midi_device());  //send clock tick
	//calculate time for next clock tick
}

static void send_all_notes_off(){
	uint8_t i;
	for (i = 0; i < 128; i++)
		midi_send_noteoff(serial_midi_device(),MIDI_CHAN,i, 0);
}

static void initialize_note_timer(){
	TCC0.CTRLA = 0x00;  //disable timer
	TCC0.CTRLB = 0x00;  //disable all compares
	TCC0.CTRLC = 0x00;
	TCC0.CTRLD = 0x00;
	TCC0.INTFLAGS = 0x00;  //clear interrupt flags
	TCC0.INTCTRLA = 0x00;
	TCC0.INTCTRLB = 0x5B;  //enable CCA interrupt Hi-Level, CCB low, CCC mid, CCD low
    TCC0.CNT = 0;          //reset counter
}

static void configure_note_timing(note_division division, uint16_t duration){
	const uint32_t division_numerator[8]   = {1, 2, 3, 1, 1, 3, 1, 1};
	const uint32_t division_denominator[8] = {1, 3, 4, 2, 3, 8, 4, 6};
	volatile uint16_t current_time;
	volatile uint32_t next_note_on_time;
	volatile uint32_t next_cutoff_time;
		
//	Q: 1/1 2/3 3/2
//	E: 1/2 1/3 3/4
//	S: 1/4 1/6 3/8
//  a= q (1) /e (1/2) /s (1/4)
//  b= d (3/2) / x (1) / t (2/3)
//  note length = a^-2 * 2/3^(b-1)   ... too complicated, just make a lookup table/array
	

	
	current_time = TCC0.CNT;    //log current time

	next_note_on_time = TCC0.CCA * division_numerator[division];       //calculate the new length for Compare B (interrupt for new note)
	next_note_on_time = next_note_on_time / division_denominator[division];
	
	next_cutoff_time = next_note_on_time * duration;                  //calculate the new length for Compare C (interrupt for note off)
	next_cutoff_time = next_cutoff_time / 0xFFFF;
	
	if (next_cutoff_time > next_note_on_time)                         //ensure the note cuts off before the next note
		next_cutoff_time = next_note_on_time - 10;
	
	next_cutoff_time += current_time;                                 //add current time to Compare C for final value, check for overflow
	if (next_cutoff_time > TCC0.CCA)
		next_cutoff_time = next_cutoff_time - TCC0.CCA;
	
	next_note_on_time += current_time;                                //add current time to Compare C for final value, check for overflow
	if (next_note_on_time > TCC0.CCA)    //the counter will reset at CCA, so check for overflow
		next_note_on_time = next_note_on_time - TCC0.CCA;
		
	TCC0.CCB = (uint16_t) next_note_on_time;    //set compare B to new value
	TCC0.CCC = (uint16_t) next_cutoff_time;     //set compare C to new value

}

uint8_t get_current_pitch(){
	
}

uint8_t get_next_pitch(){
	
}

uint8_t get_current_velocity(){
	
}

uint8_t get_next_velocity(){
	
}