#include "main.h"

#include "hardware.h"
#include "serial_midi.h"

#include <avr/interrupt.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#define MIDI_CHAN 0

uint16_t tick_count = 0;
uint16_t LED_count = 0;

volatile uint16_t note_out;
volatile uint16_t velocity_out;

ISR(TCC0_CCA_vect){
	TCC0.CNT = 0x0000;	//reset counter
	tick_count++;
	if (note_out)
		midi_send_noteon(serial_midi_device(),MIDI_CHAN,(uint8_t)note_out,(uint8_t)velocity_out);
}

ISR(USARTD1_RXC_vect){
	midi_device_input(serial_midi_device(),1,USARTD1.DATA);
}

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


volatile void test_seven_segment(){
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

void test_xnor_out(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	uint16_t note = 100;
	
	startup_functions();
//	serial_midi_init();
	
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

void test_xnor_in(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;
	uint16_t note = 100;
	
	startup_functions();
	serial_midi_init();
	
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
					
/*		if (get_encoder_switch_edge() == EDGE_RISE)
		    midi_send_noteon(serial_midi_device(),MIDI_CHAN,note,120);
		else if (get_encoder_switch_edge() == EDGE_FALL)
			midi_send_noteoff(serial_midi_device(),MIDI_CHAN,note,120);
	*/		
		status_LED = get_encoder_switch_state();
		decimal_point0 = (get_encoder() == TURN_CW);
		decimal_point1 = (get_encoder() == TURN_CCW);
		seven_segment_value = note;
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
	
}

void test_blank(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;

	
	startup_functions();
	
	while(1){
		preloop_functions();

		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
	
}

void test_timer(){
	bool decimal_point0 = 0;
	bool decimal_point1 = 0;
	bool decimal_point2 = 0;
	bool status_LED = 0;
	uint16_t seven_segment_value = 0;

	
	startup_functions();
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
		preloop_functions();
		
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
	
}

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
	TCC0.CTRLA = new_clock_divide_select;   //set the new clock divider and start the clock

	return;
}

void test_BPM(){
	volatile bool decimal_point0 = 0;
	volatile bool decimal_point1 = 0;
	volatile bool decimal_point2 = 0;
	volatile bool status_LED = 0;
	volatile uint16_t seven_segment_value = 0;
	volatile uint16_t BPM = 120;
	volatile uint8_t beat_count = 0;
	volatile uint8_t measure_count = 0;
	
	volatile bool off_sent = 0;
	
	
	startup_functions();
	serial_midi_init();
	
	TCC0.CTRLA = 0x00;  //disable timer
	TCC0.CTRLB = 0x10;  //enable compare/capture A
	TCC0.CTRLC = 0x00;
	TCC0.CTRLD = 0x00;
	TCC0.INTCTRLA = 0x00;
	TCC0.INTCTRLB = 0x03;  //enable CCA interrupt Hi-Level
	BPM_to_TMR(BPM);	//compare to 46875 (12MHz / 256)
//	TCC0.CNT = 0x0000;	//reset counter
	//TCC0.CTRLA = 0x01;  //enable timer = clk
	
	while(1){
		preloop_functions();

		if (get_encoder() == TURN_CW){
			if (BPM < 400)
				BPM++;
				BPM_to_TMR(BPM);
		}
		else if (get_encoder() == TURN_CCW){
			if (BPM > 50){
				BPM += -1;
				BPM_to_TMR(BPM);
			}				
		}
		

		if (tick_count >= 24){
			midi_send_noteon(serial_midi_device(),MIDI_CHAN,48,100);
			off_sent = 0;
			tick_count = tick_count - 24;
			beat_count++;
				if (beat_count > 3){
					midi_send_noteon(serial_midi_device(),MIDI_CHAN,52,100);
					beat_count = 0;
					measure_count++;
					if (measure_count > 99){
						measure_count = 0;
					}						
				}
		}
		
		if (tick_count > 2 && !off_sent){
			off_sent = 1;
			midi_send_noteoff(serial_midi_device(),MIDI_CHAN,48,100);
				if (beat_count == 0)
					midi_send_noteoff(serial_midi_device(),MIDI_CHAN,52,100);
		}
		
		decimal_point0 = 0;
		decimal_point1 = 0;
		if (tick_count < 12)
			decimal_point0 = 1;
		if (beat_count < 2)
			decimal_point1 = 1;
		
		if (get_encoder_switch_edge() == EDGE_RISE){
			beat_count = 0;
			measure_count = 0;
			tick_count = 0;
		}			
		
		if (get_toggle_switch_state())
			seven_segment_value = beat_count + 10*measure_count;
		else
			seven_segment_value = BPM;
		
		postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
	}
	
}

void test_tick_accuracy(){
	volatile bool decimal_point0 = 0;
	volatile bool decimal_point1 = 0;
	volatile bool decimal_point2 = 0;
	volatile bool status_LED = 0;
	volatile uint16_t seven_segment_value = 0;
	
	volatile uint16_t BPM;

	
	volatile uint32_t beat_count = 0;
	volatile uint32_t beat_count_max;
	
	volatile bool finished = 0;
	
	
	startup_functions();
	serial_midi_init();

	
	TCC0.CTRLA = 0x00;  //disable timer
	TCC0.CTRLB = 0x10;  //enable compare/capture A
	TCC0.CTRLC = 0x00;
	TCC0.CTRLD = 0x00;
	TCC0.INTCTRLA = 0x00;
	TCC0.INTCTRLB = 0x03;  //enable CCA interrupt Hi-Level

	while(1){
		for(BPM = 30; BPM < 34; BPM++){
			preloop_functions();
			
			if (!get_toggle_switch_state())
				break;
		
			seven_segment_value = BPM;
			velocity_out = BPM/127;		//prepare signifying MIDI outputs
			note_out = BPM - velocity_out*127;
		
			decimal_point0 = 0;
			decimal_point1 = 0;
			decimal_point2 = 0;
			status_LED = 0;
		
			beat_count = 0;
			
			//4 beats or 4 seconds, whichever is more samples
			if (BPM/20 > 4)
				beat_count_max = BPM/15;
			else
				beat_count_max = 4;
		
			midi_send_clock(serial_midi_device());    //clock tick to separate different BPMs
			midi_send_clock(serial_midi_device());
			
			tick_count = 0;
		
			BPM_to_TMR(BPM);  //start ticks
		
		
	
			while(beat_count < beat_count_max && get_toggle_switch_state()){
				preloop_functions();
				if (tick_count >= 24){
					tick_count = tick_count - 24;
					beat_count++;
				}
				if (tick_count < 12)
					status_LED = 1;
				else
					status_LED = 0;
				postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
			}
		
			finished = 1;

		}
	
		while(!get_toggle_switch_state() || finished){
			note_out = 0;
		
			preloop_functions();
			BPM_to_TMR(200);
			if (tick_count >= 24)
				tick_count = 0;
		
			if (tick_count < 12){
				decimal_point0 = 1;
				decimal_point1 = 1;
				decimal_point2 = 1;
				status_LED = 1;
				seven_segment_value = 111;
			}
			else{
				decimal_point0 = 0;
				decimal_point1 = 0;
				decimal_point2 = 0;
				status_LED = 0;
				seven_segment_value = 888;			
			}	
			postloop_functions(status_LED,decimal_point0,decimal_point1,decimal_point2,seven_segment_value);
		}
	
	
	}	
}

int main(void) {

//	test_xnor_out();
//    test_BPM();
   test_tick_accuracy();
   
	return 0;
}