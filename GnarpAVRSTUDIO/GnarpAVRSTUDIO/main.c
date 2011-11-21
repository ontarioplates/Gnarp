#include <avr/interrupt.h>
#include <avr/io.h>
#include "stdbool.h"
#include "stdlib.h"
#include "main.h"


#define MIDI_CLOCK_RATE 11250
#define DEBOUNCE 8
#define POTMIN 0x00C0
#define POTMAX 0x0FFF
#define POTRANGE 0x0F3F 

bool i_SWPUSHstate	= 0;
bool i_SWPUSHon		= 0;
bool i_SWPUSHoff	= 0;

bool i_SWENCstate	= 0;
bool i_SWENCon		= 0;
bool i_SWENCoff		= 0;

bool i_SWTOGstate	= 0;
bool i_SWTOGon		= 0;
bool i_SWTOGoff		= 0;

bool o_LEDDP0		= 0;
bool o_LEDDP1		= 0;
bool o_LEDDP2		= 0;
bool o_LEDSTAT		= 0;

uint8_t _i_SW0		= 0x00;
uint8_t _i_SW1		= 0x00;
uint8_t _i_SW		= 0x00;
uint8_t _i_SWc[3]	= {0,0,0};

uint16_t o_LED7SEG	= 0x0000;

uint16_t i_POT[5]	= {0,0,0,0,0};

bool _i_ENCA0		= 0;
bool i_ENCcw		= 0;
bool i_ENCccw		= 0;

uint8_t i_MIDIRX	= 0x00;

ISR(USARTD1_RXC_vect){
	i_MIDIRX = USARTD1.DATA & 0xFF;
//	USARTD1.STATUS &= ~0x80;
}


void initCLOCK(){
    //CLOCK AND PLL SETUP
	while (CLK.CTRL != 0x04){
		CLK.PSCTRL = 0x01;			//Set Prescaler to 1.
		CLK.RTCCTRL = 0x04;			//Set Real Time Clock Control to internal RCOSC but do not enable.
		
		OSC.XOSCCTRL = 0x8B;		//prepare for external clock (8-12 MHz)
		OSC.CTRL = 0x08;			//enable external clock 
		while (!(OSC.STATUS & 0x08)){}	//wait for External Oscillator to become stable and ready
		
		OSC.PLLCTRL = 0xC2;	//Set the PLL to use the external crystal and set multiplication factor to 2.
		OSC.CTRL = 0x18;				//Enable the PLL, disable the External Clock.
	
		while (!(OSC.STATUS & 0x10)){}	//wait for PLL to become stable and ready

		CCP = 0xD8;					//Configuration Change Protection, write signature to change Clock to PLL.
		CLK.CTRL = 0x04;			//Set the Clock to PLL
	}		
}

void initMIDI(){

	cli();							//disable global interrupts
	PORTD.DIRCLR		= 0x40;		//USARTRX as input
	PORTD.DIRSET		= 0x80;		//USARTTX as output
	PORTD.OUTSET		= 0x80;		//set TxD high for initialization
	USARTD1.CTRLA		= 0x20;		//enable RX interrupt as Medium Level, TX interrupt as Low Level, DRE as Hi Level
	USARTD1.CTRLC		= 0x03;		//Asynchronous, Parity disabled, Single stop bit, 8 bit character size
	USARTD1.BAUDCTRLA	= 0x2F;		//BSEL = 47
	USARTD1.BAUDCTRLB	= 0x00;		//BSCALE = 0
	USARTD1.CTRLB		= 0x18;		//set RXEN and TXEN in CTRLB Register to enable USART receiver and transmitter
	PMIC.CTRL			|= 0x02;	//enable all levels on interrupts
	sei();							//enable global interrupts

}

void initMIDIvar(uint8_t bcA){
	cli();							//disable global interrupts
	PORTD.DIRCLR		= 0x40;		//USARTRX as input
	PORTD.DIRSET		= 0x80;		//USARTTX as output
	PORTD.OUTSET		= 0x80;		//set TxD high for initialization
	USARTD1.CTRLA		= 0x20;		//enable RX interrupt as Medium Level, TX interrupt as Low Level, DRE as Hi Level
	USARTD1.CTRLC		= 0x03;		//Asynchronous, Parity disabled, Single stop bit, 8 bit character size
	USARTD1.BAUDCTRLA	= bcA;		//BSEL = 47
	USARTD1.BAUDCTRLB	= 0x00;		//BSCALE = 0
	USARTD1.CTRLB		= 0x18;		//set RXEN and TXEN in CTRLB Register to enable USART receiver and transmitter
	PMIC.CTRL			|= 0x02;	//enable all levels on interrupts
	sei();							//enable global interrupts
	
	
	//WORKS at bcA = 3

}



void initENC(){
	PORTB.DIRCLR = 0x03;		//Encoder A and B input
}

void runENC(){
	bool A1;	//current A
	bool B1;	//current B
	
	A1 = !(PORTB.IN & 0x01);
	B1 = !((PORTB.IN >> 1) & 0x01);
	
	if (!_i_ENCA0 & A1)		//movement
	{
		if (B1)
			i_ENCcw = 1;	//CW		
		else
			i_ENCcw = 0;	//CCW
			
		i_ENCccw = !i_ENCcw;
	}
	else{
		i_ENCcw = 0;
		i_ENCccw = 0;
	}		
	_i_ENCA0 = A1;
}

void initPOT(){
	PORTA.DIRCLR	= 0xF9;		//ADC3:7 and VREF input

	ADCA.CTRLA 		= 0x00;		//disable ADC
	ADCA.CTRLB 		= 0x00;
	ADCA.REFCTRL	= 0x20;		//set PORTA reference voltage
	ADCA.EVCTRL		= 0x00;
	ADCA.PRESCALER	= 0x00;
	ADCA.INTFLAGS	= 0x00;
	ADCA.CTRLA		|= 0x01;	//enable ADC

	ADCA.CH0.CTRL	= 0x01;		//select external single-ended input
	ADCA.CH0.MUXCTRL= 0x00;
	ADCA.CH0.INTCTRL= 0x00;
}

void runPOT(){
	uint8_t i;
	
	for(i = 0; i < 5; i++){
		ADCA.CH0.INTFLAGS	|= 0x01;			//clear interrupt flag
		ADCA.CH0.MUXCTRL	&= ~(0x07 << 3);	//clear pin select
		ADCA.CH0.MUXCTRL	|= ((i+3) << 3);	//set pin select to current input
		ADCA.CH0.CTRL		|=	0x80;			//start conversion
		
		while(!(ADCA.CH0.INTFLAGS & 0x01)){}
		
		i_POT[i] = ADCA.CH0.RESL;
		i_POT[i] |= ADCA.CH0.RESH << 8;
		
		if (i_POT[i] < POTMIN)
			i_POT[i] = 0;
		else
			i_POT[i] = i_POT[i] - POTMIN;
	}
	
}

uint16_t scalePOT(uint8_t pot, uint16_t outmin, uint16_t outmax){
	//pot: 0-4 to select input pot
	//outmin: minimum value to output
	//outmax: maximum value to output
	
	double temp;
	
	temp = 1.0*i_POT[pot]/POTRANGE;
	temp = temp*(outmax - outmin) + outmin;
	
	return (uint16_t) temp;
}

void initLED(){
	//initialize all LED outputs, set all as blank
	
	PORTA.DIRSET = 0x06;	//~LT and ~BL output
	PORTA.OUTSET = 0x06;	//~LT and ~BL high
	
	PORTC.DIRSET = 0xF8;	//STATLED and LED0:3 output
	PORTD.DIRSET = 0x3F;	//DSEL0:2 and DP0:2 output
	
	
	//All LEDs off
	PORTD.OUTSET = 0x38;	//DSEL0:2 high (arm all 7 segments)
	PORTC.OUTSET = 0xF0;	//LED0:3 high (blank all 7 segments)
	PORTD.OUTCLR = 0x38;	//DSEL0: low (disarm all 7 segments)
	
	PORTD.OUTCLR = 0x07;	//DP0:2 low (blank all dps)
	PORTC.OUTSET = 0x08;	//STATLED high (blank statled)
}

void runLED(){
	//booleans and such convert to LED out
	bool DP[3] = {o_LEDDP0, o_LEDDP1, o_LEDDP2};
	
	uint8_t i;
	uint8_t digit;
	uint16_t threeDigits;
	
	threeDigits = o_LED7SEG;					//copy 7seg number
	
	for (i=0 ; i<3 ; i++){
		digit = threeDigits%10;					//extract lowest current digit of 7seg
		if (threeDigits==0 && (i>0))			//if the rest of the 7seg is zero, blank LEDS (except for 1st digit)
			digit = 10;
			
		PORTD.OUTCLR = 0x08 << (i+2)%3;			//arm appropriate 7 segment		(CHANGE INDEX SCALING FOR NEXT REVISION)
		PORTC.OUTCLR = 0xF0;					//clear digit select
		PORTC.OUTSET = digit << 4;				//set digit select #
		PORTD.OUTSET = 0x38;					//disarm all 7 segments
		
		threeDigits = threeDigits/10;			//shift 7seg number down to next digit
		
		if (DP[i])							//light appropriate decimal points  (CHANGE INDEX SCALING FOR NEXT REVISION)
			PORTD.OUTSET = 1 << (i+2)%3;
		else
			PORTD.OUTCLR = 1 << (i+2)%3;
	}
	
	if (o_LEDSTAT)								//light STATLED if necessary
		PORTC.OUTCLR = 0x08;
	else
		PORTC.OUTSET = 0x08;
	
}

void initSW(){
	PORTB.DIRCLR = 0x0C;				//SW8(push) and Encoder pushbutton input
	PORTE.DIRCLR = 0x08;				//SW7(toggle) input
}

void runSW(){
	//_i_SW1 = current [x, x, x, x, x, encoder, pushmom, toggle]
	//_i_SW0 = last ["]
	//_i_SW	 = final ["]
	//_i_SWc[] = count for ["]
	
	uint8_t i;
	
	_i_SW1 = 0x00;									//capture current physical switch positions
	_i_SW1 |= !(PORTE.IN >> 3) & 0x01;
	_i_SW1 |= !((PORTB.IN >> 2) & 0x01) << 1;
	_i_SW1 |= !((PORTB.IN >> 3) & 0x01) << 2;
	
	for (i = 0; i < 3; i++){
		if (_i_SW1 >> i == _i_SW0 >> i)			//if switch didn't change
			_i_SWc[i]++;							//increment count		
		else
			_i_SWc[i] = 0;							//else reset count			
		if (_i_SWc[i] > DEBOUNCE){					//if count is over debounce value
			_i_SW &= ~(1 << i);
			_i_SW |= _i_SW1 & (1 << i);				//set final switch to current position
			_i_SWc[i] = 0;							//and reset count
		}
	}	
	
	_i_SW0 = _i_SW1;								//set last switch position to current switch position
	
	//set booleans
	
	if (_i_SW & 0x01){			//if toggle IS on
		i_SWTOGoff = 0;				//not a new off
		if (i_SWTOGstate)			//if toggle WAS on
			i_SWTOGon = 0;				//not a new on
		else							//else (toggle WAS off)
			i_SWTOGon = 1;				//new on
		i_SWTOGstate = 1;		//set current value
	}		
	else{						//if toggle IS off
		i_SWTOGon = 0;				//not a new on
		if (i_SWTOGstate)			//if toggle WAS on
			i_SWTOGoff = 1;				//new off
		else
			i_SWTOGoff = 0;
		i_SWTOGstate = 0;
	}		
	
	if (_i_SW & 0x02){		
		i_SWPUSHoff = 0;		
		if (i_SWPUSHstate)		
			i_SWPUSHon = 0;		
		else						
			i_SWPUSHon = 1;				
		i_SWPUSHstate = 1;		
	}		
	else{						
		i_SWPUSHon = 0;				
		if (i_SWPUSHstate)			
			i_SWPUSHoff = 1;		
		else
			i_SWPUSHoff = 0;
		i_SWPUSHstate = 0;
	}
	
	if (_i_SW & 0x04){		
		i_SWENCoff = 0;		
		if (i_SWENCstate)		
			i_SWENCon = 0;		
		else						
			i_SWENCon = 1;				
		i_SWENCstate = 1;		
	}		
	else{						
		i_SWENCon = 0;				
		if (i_SWENCstate)			
			i_SWENCoff = 1;		
		else
			i_SWENCoff = 0;
		i_SWENCstate = 0;
	}		
	
	
}

void test_switches_and_LEDs(){
	initLED();
	initSW();
	
	o_LED7SEG = 500;
	
	while(1){
		runSW();
		
		o_LEDDP0 = i_SWENCstate;
		o_LEDDP1 = i_SWPUSHstate;
		o_LEDDP2 = i_SWTOGstate;
		
		if (i_SWTOGstate){
			if (i_SWPUSHon)
				o_LED7SEG += 10;
			if (i_SWENCon)
				o_LED7SEG += 1;
		}
		else{
			if (i_SWPUSHoff)
				o_LED7SEG += -10;
			if (i_SWENCoff)
				o_LED7SEG += -1;
		}
		
		if (o_LED7SEG > 999)
			o_LED7SEG = 0;
		
		if (o_LED7SEG < 1)
			o_LED7SEG = 999; 
		
		runLED();
	}
}

void test_pots(){
	initLED();
	initSW();
	initPOT();
	
	uint8_t selPOT = 0;
	
	while(1){
		runSW();
		runPOT();
	
	
		o_LED7SEG = 0*(selPOT+1) + scalePOT(selPOT, 1, 990);
			
		if(i_SWENCon){
			selPOT++;
			if (selPOT>4)
				selPOT = 0;
		}
	
		runLED();
	
	}	
}

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

int main(void) {

//	test_midiTX();
	test_midiTX2();
//	test_midiRX();

	return 0;
}

