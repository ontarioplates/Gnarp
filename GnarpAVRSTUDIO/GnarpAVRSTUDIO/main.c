#include <avr/interrupt.h>
#include <avr/io.h>
#include "stdlib.h"
#include "main.h"


#define MIDI_CLOCK_RATE 11250


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

int main(void) {

	//testLED();
	//testLED_TOGGLESW();
	//testOUTTGL();
	//testLEDfade();
	test7Seg();
	//testADC();


	return 0;
}

