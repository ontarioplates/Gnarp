#include <avr/interrupt.h>
#include <avr/io.h>
#include "stdlib.h"
#include "./xnorMIDI/midi.h"
#include "main.h"


#define MIDI_CLOCK_RATE 11250

static MidiDevice midi_device;

void serial_midi_recieve(MidiDevice * device){
	while(USARTD1.STATUS & _BV(7))					//RXCIF is set
	{
		uint8_t temp[3] = {USARTD1.DATA,0,0};
		midi_device_input(&midi_device, 1, temp);
	}
}

void serial_midi_send(MidiDevice * device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
   //we always send the first byte
	while(!(USARTD1.STATUS & _BV(5)));		//wait for empty transmit buffer
	USARTD1.DATA = inByte0;
   //if cnt == 2 or 3 we send the send byte
   if(cnt > 1) {
	  while(!(USARTD1.STATUS & _BV(5)));		//wait for empty transmit buffer
      USARTD1.DATA = inByte1;
   }
   //if cnt == 3 we send the third byte
   if(cnt == 3) {
	  while(!(USARTD1.STATUS & _BV(5)));		//wait for empty transmit buffer
      USARTD1.DATA = inByte2;
   }
}

void arp_fallthrough_callback(MidiDevice * device, uint8_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2){
    int i;
    uint8_t bytes[3] = {inByte0, inByte1, inByte2};
    for (i=0; i<cnt; i++);
  //      printf("%d ", bytes[i]);
  //  printf("\n");
}


MidiDevice * serial_midi_device(void) {
   return &midi_device;
}

MidiDevice* serial_midi_init(uint16_t clockScale, bool out, bool in){
   //send up the device
   midi_device_init(&midi_device);
   midi_device_set_send_func(&midi_device, serial_midi_send);
   midi_device_set_pre_input_process_func(&midi_device, serial_midi_recieve);
   midi_register_fallthrough_callback(&midi_device, arp_fallthrough_callback);

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

   return serial_midi_device();
}



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

int main(void) {

	//testLED();
	//testLED_TOGGLESW();
	testOUTTGL();

	return 0;
}

