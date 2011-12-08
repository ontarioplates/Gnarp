#include "main.h"

#include "hardware.h"
#include "serial_midi.h"

#include <avr/interrupt.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

/*static MidiDevice midi_device;

bool beat_overflow = 0;
uint8_t next_velocity;
note_division next_division;
uint16_t next_duration;
	
const uint8_t pitch_array[7] = {50, 55, 53, 60, 59, 65, 40};
uint8_t pitch_array_select = 0;
*/
ISR(USARTD1_RXC_vect){
//	static uint8_t new_byte[1];
//	new_byte[0] = USARTD1.DATA;
	midi_device_input(serial_midi_device(),1,&(USARTD1.DATA));
	midi_device_process(serial_midi_device());
}

void test_pots(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	
	initialize_hardware();
	
	uint8_t selPOT = 0;
	
	while(1){
		read_hardware();
	
		set_seven_segment_LEDs(100*(selPOT+1) + get_pot_value(selPOT, 1, 99));
			
		if(get_encoder_switch_edge()==EDGE_RISE){
			selPOT++;
			if (selPOT>4)
				selPOT = 0;
		}
	
	}	
}
/*
void test_encoder(){
	initLED();
	initSW();
	initPOT();
	initENC();
	
	o_LED7SEG = 100;
	
	while(1){
		runSW();
		runPOT();
		runENC();
		
		if (i_ENCcw)
			o_LED7SEG++;
		else if (i_ENCccw)
			o_LED7SEG += -1;
			
		o_LEDSTAT = i_SWENCstate;

		
		runLED();
	}
}

void test_midiTX(){
	initCLOCK();
	initLED();
	initMIDI();
	initPOT();
	initSW();
	initENC();
	
	uint8_t val = 0;
	uint8_t send = 0;
	
	while(1){
		runSW();
		runPOT();
		runENC();
		
		o_LEDDP2 = i_SWENCstate;
		o_LEDDP1 = 0;
		o_LEDDP0 = 0;
		o_LEDSTAT = 0;
		
		send = 0;
		
		if (i_ENCccw){
			send = 1;
			val = (val - 1) % 256;
		}
		
		if (i_ENCcw){
			send = 1;
			val = (val + 1) % 256;
		}
		
		if (send)
			initMIDIvar(val);
		
		if (i_SWENCon){
			send = 2;
		}
			
		if (send==1){
			while (!(USARTD1.STATUS & 0x20)){}//Data Register Empty Flag
			
			o_LEDDP1 = 1;
			USARTD1.DATA = 0xF8;		//sync
		}
		
		if (send == 2){
			while (!(USARTD1.STATUS & 0x20)){}//Data Register Empty Flag
			
			o_LEDDP1 = 1;
			USARTD1.DATA = 0xB0;		//control change
			
			while (!(USARTD1.STATUS & 0x20)){}//Data Register Empty Flag
			
			o_LEDDP0 = 1;
			USARTD1.DATA = 0x01;		//mod wheel
			
			while (!(USARTD1.STATUS & 0x20)){}//Data Register Empty Flag
			
			o_LEDSTAT = 1;
			USARTD1.DATA = 0x37;		//value = 55
		}
		
		o_LED7SEG = val;
		runLED();	
			
		
//		if (USARTD1.STATUS & 0x80)
//			i_MIDIRX = USARTD1.DATA;
		
		
//		runLED();
	}
}

void test_midiTX2(){
	initCLOCK();
	initENC();
	initLED();
	initSW();
	initPOT();
	initMIDI();
	
	uint8_t val = 0;
	
	while(1){
		runPOT();
		runENC();
		runSW();
		
		val = scalePOT(0,248,253);
		
		if (i_SWPUSHon){
			while (!(USARTD1.STATUS & 0x20)){}//Data Register Empty Flag
			USARTD1.DATA = val;		//sync
		}
		
		o_LEDSTAT =	i_SWPUSHstate;
		
		o_LED7SEG = val;
		
		runLED();
	}
	
}

void test_midiRX(){
	initCLOCK();
	initENC();
	initLED();
	initSW();
	initPOT();
	initMIDI();
	
	uint8_t val = 0;
	
	while(1){
		runPOT();
		runENC();
		runSW();
		
		o_LED7SEG = i_MIDIRX;
		
		runLED();
	}		
}

void test_OSCOUT(){
	initCLOCK();
	
	PORTC.DIRSET = 0xF8;
	PORTCFG.CLKEVOUT = 0x01;  //Peripheral Clock output to port C pin7
	
	while(1){}
	
	
}
*/

void test_switches(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	
	initialize_hardware();
	
	while(1){
		read_hardware();
		
		decimal_point0 = get_encoder_switch_state();
		decimal_point1 = get_pushbutton_switch_state();
		decimal_point2 = get_toggle_switch_state();
		status_LED = 0;
		seven_segment_value = 0;
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
}


volatile void test_seven_segment(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	uint16_t i = 0;
	
	initialize_hardware();
	
	while(1){
		read_hardware();
		
		decimal_point0 = get_encoder_switch_state();
		decimal_point1 = get_pushbutton_switch_state();
		decimal_point2 = get_toggle_switch_state();
		status_LED = 0;
		
		if (get_encoder() == TURN_CW){
			if (i == 999)
				i = 0;
			else
				i++;
		}			
		else if (get_encoder()==TURN_CCW){
			if (i == 0)
				i = 999;
			else
				i += -1;
		}
		
		seven_segment_value = i;
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
}

void test_ADC(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	uint16_t pot_select = 0;
	
	initialize_hardware();
	
	while(1){
		read_hardware();
		
		decimal_point0 = get_encoder_switch_state();
		decimal_point1 = get_pushbutton_switch_state();
		decimal_point2 = get_toggle_switch_state();
		status_LED = 0;
		
		if (get_encoder() == TURN_CW){
			if (pot_select == 4)
				pot_select = 0;
			else
				pot_select++;
		}			
		else if (get_encoder()==TURN_CCW){
			if (pot_select == 0)
				pot_select = 4;
			else
				pot_select += -1;
		}
		
		seven_segment_value = pot_select*100 + get_pot_value(pot_select,0,99);
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
}
/*
void test_xnor_out(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	uint16_t note = 100;
	
	initialize_hardware();
//	serial_midi_init();
	
	while(1){
		read_hardware();
		
		if (get_encoder() == TURN_CCW){
			if (note <= 64)
				note = 152;
			else
				note += -1;
		}
		else if (get_encoder() == TURN_CW){
			if (note >= 152)
				note = 64;
			else
				note++;
		}				
					
		if (get_encoder_switch_edge() == EDGE_RISE)
		    midi_send_noteon(serial_midi_device(),MIDI_CHAN,note,120);
		else if (get_encoder_switch_edge() == EDGE_FALL)
			midi_send_noteoff(serial_midi_device(),MIDI_CHAN,note,120);
			
		status_LED = get_encoder_switch_state();
		decimal_point0 = (get_encoder() == TURN_CW);
		decimal_point1 = (get_encoder() == TURN_CCW);
		seven_segment_value = note;
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
	
}
*/
void test_xnor_in(){
	static uint8_t new_byte;

	initialize_hardware();
	serial_midi_init();
	
	while(1){
		read_hardware();
		
		if (get_encoder_switch_edge()){
			new_byte = 0x90;
			midi_device_input(serial_midi_device(),1,&new_byte);
			midi_device_process(serial_midi_device());
			
			new_byte = 0x3D;
			midi_device_input(serial_midi_device(),1,&new_byte);
			midi_device_process(serial_midi_device());
			
			new_byte = 0x64;
			midi_device_input(serial_midi_device(),1,&new_byte);
			midi_device_process(serial_midi_device());
		}
		
	}
	
}

void test_blank(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;

	
	initialize_hardware();
	
	while(1){
		read_hardware();

		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
	
}

/*void test_timer(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;

	
	initialize_hardware();
	TCC0.CTRLA = 0x00;  //disable timer
	TCC0.CTRLB = 0x10;  //enable compare/capture A
	TCC0.CTRLC = 0x00;
	TCC0.CTRLD = 0x00;
	TCC0.INTCTRLA = 0x00;
	TCC0.INTCTRLB = 0x03;  //enable CCA interrupt Hi-Level
	TCC0.CCA = 23437;	//compare to 46875 (12MHz / 256)
	TCC0.CNT = 0x0000;	//reset counter
	TCC0.CTRLA = 0x07;  //enable timer = clk/1024
	
	
	
	while(1){
		read_hardware();
		
		if (get_encoder() == TURN_CW){
			if (LED_count == 999)
				LED_count = 0;
			else
				LED_count++;
		}		
		else if (get_encoder() == TURN_CCW){
			if (LED_count == 0)
				LED_count = 999;
			else
				LED_count+= -1;
		}				
		
		if (get_encoder_switch_edge() == EDGE_RISE){
			LED_count = 0;
			TCC0_CNT = 0;
		}			
			
		seven_segment_value = LED_count;
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
	
}*/

void BPM_to_TMR(uint16_t BPM){
    const uint32_t numerator = 60000000;                                 //clk = 12MHz, cyc/MIDItick = 30M/BPM
    const uint32_t clock_divide[8] = {0, 1, 2, 4, 8, 64, 256, 1024};     //corresponds to scaler value for TCxx.CTRLA
	
	volatile uint8_t current_clock_divide_select = (TCC0.CTRLA & 0x0F);
	volatile uint8_t new_clock_divide_select = 1;
	volatile uint32_t adjusted_count = 0;
	
	volatile uint32_t cycle_per_MIDItick = numerator/BPM;   //compare value for no divider
	
	volatile uint32_t compare_value = cycle_per_MIDItick/clock_divide[new_clock_divide_select];
	
	while (compare_value > 0xFFFF){        //run loop until compare_value is a 16 bit number
		new_clock_divide_select++;             //try the next highest divider
		
		if (new_clock_divide_select > 7)       //unless you've explored all of them
			return;
		
		compare_value = cycle_per_MIDItick/clock_divide[new_clock_divide_select];
	}
	
	if (TCC0.CTRLA){
		if (!(current_clock_divide_select == new_clock_divide_select)){           //stop and scale the timer count if the divider must change
			TCC0.CTRLA = 0x00;
			adjusted_count = TCC0.CNT * clock_divide[new_clock_divide_select];
			adjusted_count = adjusted_count / clock_divide[current_clock_divide_select];
			while (adjusted_count > compare_value)
				adjusted_count = adjusted_count - compare_value;
			TCC0.CNT = (uint16_t) adjusted_count;
		}
		else
			TCC0.CTRLA = 0x00;  //otherwise, just stop the timer 
	}			
	
	
	TCC0.CCA = (uint16_t) compare_value;    //set the new compare value
	TCC0.CTRLA = new_clock_divide_select;   //set the new clock divider and start the clock

	return;
}

void BPM_to_TMR2(uint16_t BPM){
    const uint32_t numerator = 1440000000;                                 //clk = 24MHz, cyc/beat = 1.44Trillion/BPM
    const uint32_t clock_divide[8] = {0, 1, 2, 4, 8, 64, 256, 1024};     //corresponds to division value for TCxx.CTRLA
	
	volatile uint8_t current_clock_divide_select = (TCC0.CTRLA & 0x0F);
	volatile uint8_t new_clock_divide_select = 1;
	volatile uint32_t adjusted_count = 0;
	
	volatile uint32_t cycle_per_beat = numerator/BPM;   //compare value for no divider
	
	volatile uint32_t compare_value = cycle_per_beat/clock_divide[new_clock_divide_select];
	
	while (compare_value > 0xFFFF){        //run loop until compare_value is a 16 bit number
		new_clock_divide_select++;             //try the next highest divider
		
		if (new_clock_divide_select > 7)       //unless you've explored all of them
			return;
		
		compare_value = cycle_per_beat/clock_divide[new_clock_divide_select];
	}
	
	if (TCC0.CTRLA){
		if (!(current_clock_divide_select == new_clock_divide_select)){           //stop and scale the timer count if the divider must change
			TCC0.CTRLA = 0x00;
			adjusted_count = TCC0.CNT * clock_divide[new_clock_divide_select];
			adjusted_count = adjusted_count / clock_divide[current_clock_divide_select];
			while (adjusted_count > compare_value)
				adjusted_count = adjusted_count - compare_value;
			TCC0.CNT = (uint16_t) adjusted_count;
		}
		else
			TCC0.CTRLA = 0x00;  //otherwise, just stop the timer 
	}			
	
	
	TCC0.CCA = (uint16_t) compare_value;    //set the new compare value for beat
	TCC0.CCD = (uint16_t) compare_value/24; //set the new compare value for midi-clock ticks
	
	TCC0.CTRLB |= 0x90;   //enable CCA (beat count) and CCD (tick count)
	
	TCC0.CTRLA = new_clock_divide_select;   //set the new clock divider and start the clock

	return;
}


/*

void test_notes(){
	volatile uint16_t BPM = 60;
	volatile uint8_t beat_count = 0;
	volatile bool play_notes = 0;
	volatile uint32_t temp_duration;

		
	initialize_hardware();
	serial_midi_init();
	
	TCC0.CTRLA = 0x00;  //disable timer
	TCC0.CTRLB = 0x00;  //disable all compares
	TCC0.CTRLC = 0x00;
	TCC0.CTRLD = 0x00;
	TCC0.INTFLAGS = 0x00;  //clear interrupt flags
	TCC0.INTCTRLA = 0x00;
	TCC0.INTCTRLB = 0x5B;  //enable CCA interrupt Hi-Level, CCB low, CCC mid, CCD low
    TCC0.CNT = 0;     //reset counter
	BPM_to_TMR2(BPM);
	
	next_velocity = 100;
	next_division = QUARTER;
	next_duration = 0xEFFF;
	
	note_on(pitch_array[pitch_array_select],next_velocity,next_division,next_duration);
	
	while(1){
		read_hardware();

		if (beat_overflow){
			beat_overflow = 0;
			beat_count++;
			if (beat_count > 3)
				beat_count = 0;
		}
		
		if (TCC0.CNT < TCC0.CCA/4)
			set_LEDs_on(0,1,0,0);
		else
			set_LEDs_off(0,1,0,0);
		
		next_duration = get_pot_value(0, 50, 0xFFFF);
		next_division = get_pot_value(1, 0, 7);
		
		if (get_encoder() == TURN_CW && BPM < 300)
			BPM_to_TMR2(++BPM);
		if (get_encoder() == TURN_CCW && BPM > 30){
			BPM += -1;
			BPM_to_TMR2(BPM);
		}		
		
			
		if (get_encoder_switch_state())
			set_seven_segment_LEDs(BPM);
		else
			set_seven_segment_LEDs(beat_count);
			
		if (get_toggle_switch_edge() == EDGE_RISE){
			note_off(pitch_array[pitch_array_select],next_velocity);
			beat_count = 0;
			TCC0.CNT = 0;
			pitch_array_select = 0;
			note_on(pitch_array[pitch_array_select],next_velocity,next_division,next_duration);
		}			
	}
	
}

void test_notes2(){
	volatile uint16_t BPM = 60;
	volatile uint8_t beat_count = 0;
	volatile bool play_notes = 0;
	volatile uint32_t temp_duration;

		
	initialize_hardware();
	serial_midi_init();
	
	next_velocity = 100;
	next_division = QUARTER;
	next_duration = 0xEFFF;
	
	note_on(pitch_array[pitch_array_select],next_velocity,next_division,next_duration);
	
	while(1){
		read_hardware();

		if (beat_overflow){
			beat_overflow = 0;
			beat_count++;
			if (beat_count > 3)
				beat_count = 0;
		}
		
		if (TCC0.CNT < TCC0.CCA/4)
			set_LEDs_on(0,1,0,0);
		else
			set_LEDs_off(0,1,0,0);
		
		next_duration = get_pot_value(0, 50, 0xFFFF);
		next_division = get_pot_value(1, 0, 7);
		
		if (get_encoder() == TURN_CW && BPM < 300)
			BPM_to_TMR2(++BPM);
		if (get_encoder() == TURN_CCW && BPM > 30){
			BPM += -1;
			BPM_to_TMR2(BPM);
		}		
		
			
		if (get_encoder_switch_state())
			set_seven_segment_LEDs(BPM);
		else
			set_seven_segment_LEDs(beat_count);
			
		if (get_toggle_switch_edge() == EDGE_RISE){
			note_off(pitch_array[pitch_array_select],next_velocity);
			beat_count = 0;
			TCC0.CNT = 0;
			pitch_array_select = 0;
			note_on(pitch_array[pitch_array_select],next_velocity,next_division,next_duration);
		}			
	}
	
}
*/

void test_pot_banks(){
	initialize_hardware();
	
	uint16_t pot_out_max = 10;
	uint16_t pot_out_min = 0;
	uint16_t pot_out_value = 0;
	
	while(1){
		read_hardware();
		
		if (get_encoder() == TURN_CW){
			if (get_encoder_switch_state()){
				if (pot_out_min < (pot_out_max - 1))
					pot_out_min++;
			}					
			else if (pot_out_max < 999)
				pot_out_max++;
		}
		else if (get_encoder() == TURN_CCW){
			if (get_encoder_switch_state()){
				if (pot_out_min > 0)
					pot_out_min += -1;
			}					
			else if (pot_out_max > (pot_out_min + 1))
				pot_out_max += -1;
		}
		
		pot_out_value = get_pot_value(2, pot_out_min, pot_out_max);
		
		if (get_toggle_switch_state())
			set_seven_segment_LEDs(pot_out_value);
		else if (get_pushbutton_switch_state())
			set_seven_segment_LEDs(pot_out_min);
		else
			set_seven_segment_LEDs(pot_out_max);
		
	}
}

int main(void) {

	test_xnor_in();
   
	return 0;
}