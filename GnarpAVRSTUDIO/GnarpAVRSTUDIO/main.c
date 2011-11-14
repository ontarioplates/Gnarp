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


	// Set baud rate
/*	UBRRH = (uint8_t)(clockScale >> 8);
	UBRRL = (uint8_t)(clockScale & 0xFF);
*/
/*
	// Enable transmitter
	if(out)
        printf("MIDI OUT enabled\n");
//		UCSRB |= _BV(TXEN);
	if(in) {
		//Enable receiver
		//RX Complete Interrupt Enable  (user must provide routine)
        printf("MIDI IN enabled\n");
		UCSRB |= _BV(RXEN) | _BV(RXCIE);
	}
	*/
	//Set frame format: Async, 8data, 1 stop bit, 1 start bit, no parity
	//needs to have URSEL set in order to write into this reg
//	UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);



void mcuStartup(){

    //CLOCK AND PLL SETUP
	unsigned char XOSCTEST = 0;
	unsigned char PLLMULTFACTOR;
	PLLMULTFACTOR = 0x02;		//Set the PLL Multiplication Factor to 2x.
	CLK.PSCTRL = 0x01;			//Set Prescaler to 1.
	CLK.RTCCTRL = 0x04;			//Set Real Time Clock Control to internal RCOSC but do not enable.
	OSC.XOSCCTRL = 0x8B;
	OSC.CTRL = 0x08;			//Once XOOSCTEST equals 1, it will exit the do loop and enable the external oscillator.
	for (XOSCTEST = 0; XOSCTEST < 1; )
		XOSCTEST = OSC.STATUS >> 3 &1;
	OSC.PLLCTRL = 0xC0 + PLLMULTFACTOR;	//Set the PLL to use the external crystal and set multiplication factor.
	OSC.CTRL = 0x18;			//Enable the PLL, disable the External Clock.
	XOSCTEST = 0;
	for (XOSCTEST = 0; XOSCTEST < 1; )
		XOSCTEST = OSC.STATUS >> 4 &1;
	CCP = 0xD8;					//Configuration Change Protection, write signature to change Clock to PLL.
	CLK.CTRL = 0x04;			//Set the Clock to PLL

/*	Rev 1 Board I/O:
	INPUT
		VREF			-	PORTA.AREF
		Switch/Pot0:4	-	ADCA.CH3:7
		Encoder A		-	PORTB.0
		Encoder B		-	PORTB.1
		Beat Reset		-	PORTB.2 (ASYNC Interrupt)
		Encoder P		-	PORTB.3
		MIDI In			-	USARTD1.RX
		Sync In			-	PORTE.2 (ASYNC Interrupt / OC0C?)
		Bypass			-	PE3
	OUTPUT
		~LT				-	PORTA.1
		~BL				-	PORTA.2
		Status LED		-	PORTC.3
		LED0:3			-	POTRTC.4:7
		DP0:2			-	PORTD0:2
		DSEL0:2			-	PORTD3:6
		MIDI Out		-	USARTD1.TX
		Sync Out		-	PORTE.0 (ASYNC Interrupt / OC0C?)

*/

    // I/O DIRECTION SETUP
    PORTA.DIR = 0x06;        //1 and 2 output
    PORTB.DIR = 0x00;        //No output
    PORTC.DIR = 0xF8;        //4-7 output
    PORTD.DIR = 0xBF;        //0-6, 8 output
    PORTE.DIR = 0x01;


    // USART SETUP
	cli();						//disable global interrupts
	USARTD1.CTRLA = 0x27;		//enable RX interrupt as Medium Level, TX interrupt as Low Level, DRE as Hi Level
	USARTD1.CTRLB = 0x18;		//set RXEN and TXEN in CTRLB Register to enable USART reciever and transmitter
	USARTD1.CTRLC = 0x03;		//Asynchronous, Parity disabled, Single stop bit, 8 bit character size
	USARTD1.BAUDCTRLA = 0x2F;	//BSEL = 47
	USARTD1.BAUDCTRLB = 0x00;	//BSCALE = 0
	PMIC.CTRL |= 0x07;			//enable all levels on interrupts
	sei();						//enable global interrupts

}

void testLED() {
	PORTC.DIR = 0x08;

	uint32_t n = 0x0FF0;
	uint32_t i = 0;

	while (1){
		if (i > n/2)
			PORTC.OUTSET = 0x08;
		else
			PORTC.OUTCLR = 0x08;

		i++;

		if (i > n){
			i = 0;
			n = 3*n/4;
		}

		if (n < 0x000F)
			n = 0x0FF0;
	}
}

void testLED_TOGGLESW(){
	PORTC.DIRSET = 0x08;
	PORTE.DIRCLR = 0x08;

	uint32_t n;
	uint32_t i;

	while (1){
		n = 0xFFF0;
		i = 0;
		PORTC.OUTSET = 0x08;

		while (PORTE.IN & 0x08){
			if (i > n/2)
				PORTC.OUTSET = 0x08;
			else
				PORTC.OUTCLR = 0x08;

			i++;

			if (i > n){
				i = 0;
				n = 3*n/4;
			}

			if (n < 0x000F)
				n = 0xFFF0;
		}
	}
}

void testOUTTGL() {
	PORTC.DIRSET = 0x08;
	PORTE.DIRCLR = 0x08;

	uint32_t n;
	uint32_t i = 0;

	while (1){
		if (PORTE.IN & 0x08)
			n = 0x0EF0;
		else
			n = 0x0900;
		i++;
		if (i > n){
			PORTC.OUTTGL = 0x08;
			i = 0;
		}
	}

	while (1){
		if (i > n/2)
			PORTC.OUTTGL = 0x08;
//		else
	//		PORTC.OUTTGL = 0x08;

		i++;

		if (i > n){
			i = 0;
			n = 3*n/4;
		}

		if (n < 0x000F)
			n = 0x0FF0;
	}
}

void testLEDfade(){
	PORTC.DIRSET = 0x08;
	PORTE.DIRCLR = 0x08;

	uint32_t n = 100;
	uint32_t x = 1;
	uint32_t i = 0;

	while(1){
		if (PORTE.IN & 0x08)
			x = n;
		else
			x = 10;

		if (i <= x)
			PORTC.OUTCLR = 0x08;
		else
			PORTC.OUTSET = 0x08;

		i++;
		if (i>n)
			i=0;

	}



}

void test7Seg(){
	PORTA.DIRSET = 0x06;
	PORTA.OUTSET = 0x06;
	
	PORTC.DIRSET = 0xF8;
	
	PORTD.DIRSET = 0xBF;
	PORTD.OUTSET = 0x38;
	PORTC.OUTSET = 0xF0;
	PORTD.OUTCLR = 0x3F;
	
	PORTE.DIRCLR = 0x08;

	uint32_t tick = 0;
	uint32_t tickM = 0x008F;
	uint32_t i = 0;

	uint32_t LED1=0;
	uint32_t LED10=0;
	uint32_t LED100=0;

	while(1){
		if (i >= (LED10*1+LED100*10))
		{
			PORTC.OUTCLR = 0x08;
			PORTD.OUTCLR = 0x07;
		}			
		else
		{
			PORTC.OUTSET = 0x08;
			PORTD.OUTSET = 0x07;
		}			
			
		PORTD.OUTCLR = 0x10;
		PORTC.OUTCLR = 0xF0;
		PORTC.OUTSET = LED100 << 4;
		PORTD.OUTSET = 0x10;
		
		PORTD.OUTCLR = 0x08;
		PORTC.OUTCLR = 0xF0;
		PORTC.OUTSET = LED10 << 4;
		PORTD.OUTSET = 0x08;
		
		PORTD.OUTCLR = 0x20;
		PORTC.OUTCLR = 0xF0;
		PORTC.OUTSET = LED1 << 4;
		PORTD.OUTSET = 0x20;

		i++;
		if (i>99)
			i = 0;
		
		tick++;
		if (tick>tickM)
		{
			tick = 0;
			LED1++;
			if (LED1>9)
			{
				LED1 = 0;
				LED10++;
				if (LED10>9)
				{
					LED10 = 0;
					LED100++;
					if (LED100 > 9)
						LED100 = 0;
				}
			}				
				
		}		

	}
}

void testADC(){
	PORTA.DIRSET = 0x06;
	PORTA.DIRCLR = 0xF9;
	PORTA.OUTSET = 0x06;

	PORTC.DIRSET = 0xF8;
	PORTD.DIRSET = 0xBF;
	PORTE.DIRCLR = 0x08;
	
	PORTD.OUTCLR = 0xF0;
	PORTC.OUTCLR = 0xF0;
	PORTC.OUTSET = 7 << 4;
	PORTD.OUTSET = 0xF0;
	
	uint16_t i = 0;
	uint16_t N = 0x09FF;
	uint16_t result = 0;
	double	temp = 0;
	uint16_t LEDout = 0;

	ADCA.CTRLA 		= 0x00;	//disable ADC
	ADCA.CTRLB 		= 0x00;
	ADCA.REFCTRL	= 0x20; //set PORTA reference voltage
	ADCA.EVCTRL		= 0x00;
	ADCA.PRESCALER	= 0x00;
	ADCA.INTFLAGS	= 0x00;
	ADCA.CTRLA		|= 0x01;	//enable ADC

	ADCA.CH0.CTRL	= 0x01;	//select external single-ended input
	ADCA.CH0.MUXCTRL= 0x38;	//select pin 7
	ADCA.CH0.INTCTRL= 0x00;
	
	while(1){
		ADCA.CH0.INTFLAGS |= 0x01;	//clear interrupt flag
		ADCA.CH0.CTRL	|=	0x80;	//start conversion
		
		PORTC.OUTSET = 0x08;
		i=0;
		while(!(ADCA.CH0.INTFLAGS & 0x01)){
			i++;
			if (i>N){
				i = 0;
				PORTC.OUTTGL = 0x08;
			}
		}
		PORTC.OUTSET = 0x08;
		
		result = ADCA.CH0.RESL;
		result |= ADCA.CH0.RESH << 8;
		
		temp = result*9.0;
		temp = temp/0xFFF;
		
		LEDout = (uint16_t)temp % 9;
		
		PORTD.OUTCLR = 0xF0;
		PORTC.OUTCLR = 0xF0;
		PORTC.OUTSET = LEDout << 4;
		PORTD.OUTSET = 0xF0;
		
		
		
	}
	
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
	
	
		o_LED7SEG = 100*(selPOT+1) + scalePOT(selPOT, 0, 7);
			
		if(i_SWENCon){
			selPOT++;
			if (selPOT>4)
				selPOT = 0;
		}
	
		runLED();
	
	}	
}

int main(void) {

	test_pots();


	return 0;
}

