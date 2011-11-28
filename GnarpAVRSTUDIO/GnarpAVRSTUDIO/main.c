#include "main.h"

#include <stdbool.h>
#include <stdlib.h>

#include "hardware.h"
#include "serial_midi.h"

#define MIDI_CHAN 0


/*
uint8_t i_MIDIRX	= 0x00;

ISR(USARTD1_RXC_vect){
	i_MIDIRX = USARTD1.DATA & 0xFF;
//	USARTD1.STATUS &= ~0x80;
}
*/
void test_pots(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	
	startup_functions();
	
	uint8_t selPOT = 0;
	
	while(1){
		preloop_functions();
	
		seven_segment_value = 100*(selPOT+1) + get_pot_value(selPOT, 1, 99);
			
		if(get_encoder_switch_edge()==EDGE_RISE){
			selPOT++;
			if (selPOT>4)
				selPOT = 0;
		}
	
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	
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
	
	startup_functions();
	
	while(1){
		preloop_functions();
		
		decimal_point0 = get_encoder_switch_state();
		decimal_point1 = get_pushbutton_switch_state();
		decimal_point2 = get_toggle_switch_state();
		status_LED = 0;
		seven_segment_value = 0;
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
}


void test_seven_segment(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	uint16_t i = 0;
	
	startup_functions();
	
	while(1){
		preloop_functions();
		
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
	
	startup_functions();
	
	while(1){
		preloop_functions();
		
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

void test_xnor(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	uint16_t note = 100;
	
	startup_functions();
	MidiDevice * midi_device = serial_midi_init();
	
	while(1){
		preloop_functions();
		
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
		    midi_send_noteon(midi_device,MIDI_CHAN,note,120);
		else if (get_encoder_switch_edge() == EDGE_FALL)
			midi_send_noteoff(midi_device,MIDI_CHAN,note,120);
			
		status_LED = get_encoder_switch_state();
		decimal_point0 = (get_encoder() == TURN_CW);
		decimal_point1 = (get_encoder() == TURN_CCW);
		seven_segment_value = note;
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
	
}



int main(void) {

	test_xnor();

	return 0;
}

