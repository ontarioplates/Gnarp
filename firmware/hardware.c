// Copyright (c) 2012, David Tuzman, All Rights Reserved

#include "hardware.h"

static HardwareManager manager;

HardwareManager* get_hardware_manager_ptr(){
    return &manager;
}    
    
void initialize_HardwareManager(){
    manager.encoder_and_switch_info = 0x00;
    
    for (uint8_t i = 0; i < NUM_POTS; i++)
        manager.pot_values[i] = 0;
}    
    
static void initialize_clock(){
    //CLOCK AND PLL SETUP
    while (CLK.CTRL != 0x04){
        CLK.PSCTRL = 0x01;               //Set Prescaler to 1.
        CLK.RTCCTRL = 0x04;              //Set Real Time Clock Control to internal RCOSC but do not enable.      
        OSC.XOSCCTRL = 0x8B;            //prepare for external clock (8-12 MHz)
        OSC.CTRL = 0x08;                //enable external clock 
        while (!(OSC.STATUS & 0x08)){}  //wait for External Oscillator to become stable and ready
        OSC.PLLCTRL = 0xC2;             //Set the PLL to use the external crystal and set multiplication factor to 2.
        OSC.CTRL = 0x18;                //Enable the PLL, disable the External Clock.
        while (!(OSC.STATUS & 0x10)){}  //wait for PLL to become stable and ready
        CCP = 0xD8;                     //Configuration Change Protection, write signature to change Clock to PLL.
        CLK.CTRL = 0x04;                //Set the Clock to PLL
    }        
}

static void initialize_MIDI(){
    cli();                     //disable global interrupts
    PORTD.OUTSET = 0x80;       //set TxD high for initialization
    PORTD.DIRCLR = 0x40;       //USARTRX as input
    PORTD.DIRSET = 0x80;       //USARTTX as output
    USARTD1.CTRLB = 0x18;      //set RXEN and TXEN in CTRLB Register to enable USART receiver and transmitter
    USARTD1.CTRLA = 0x20;      //enable RX interrupt as Mid Level
    USARTD1.CTRLC = 0x03;      //Asynchronous, Parity disabled, Single stop bit, 8 bit character size
    USARTD1.BAUDCTRLA = 0x2F;  //BSEL = 47
    USARTD1.BAUDCTRLB = 0x00;  //BSCALE = 0
    PMIC.CTRL |= 0x87;         //enable all levels on interrupts
    sei();                     //enable global interrupts
}

static void initialize_encoder(){
    PORTB.DIRCLR = 0x03;       //Encoder A and B input
}

static void initialize_realtime_utility(){
	TCD0.CTRLA = 0x00;  //stop the counter
	TCD0.CTRLB = 0x00;  //disable compares
	TCD0.CNT = 0x0000;
}

void realtime_pause(uint16_t pause_ms){
	const uint8_t count_per_ms = 23;
	const uint8_t CTRLA_val = 0x07; // 1/1024 system clock (24M)
	
	uint16_t pause_count;
	
	if (pause_ms > 2796)
	    pause_ms = 2796;
		
	pause_count = pause_ms * count_per_ms;
	
	bool was_running = TCD0.CTRLA;	
	uint16_t count_at_start = 0;
	
	TCD0.CTRLA = 0x00;  //stop the counter
	
	if (was_running)        //log the current count
	    count_at_start = TCD0.CNT;
	
	TCD0.CNT = 0;       //reset the count
	
	TCD0.CTRLA = CTRLA_val; //start the counter
	
	while (TCD0.CNT < pause_count) {};  //wait until the counter trips
		
	if (was_running){    //continue the counter from its last position
		if (count_at_start + pause_count < count_at_start)
		    TCD0.CNT = count_at_start;
		else
		    TCD0.CNT = count_at_start + pause_count;
	}		
	else                //stop the counter
	    TCD0.CTRLA = 0x00;
}

void realtime_count_start(){
	const uint8_t CTRLA_val = 0x07; // 1/1024 system clock (24M)
	TCD0.CTRLA = CTRLA_val;
	TCD0.CNT = 0;
}

void realtime_count_stop(){
    TCD0.CTRLA = 0x00;
	TCD0.CNT = 0;
}

bool realtime_count_compare(uint16_t compare_ms){
	const uint8_t count_per_ms = 23;
	if (TCD0.INTFLAGS & 0x01){  //if there was an overflow, return true and reset the count
		TCD0.CNT = 0;
	    TCD0.INTFLAGS |= 0x01;  //clear the overflow flag
	    return true;
	}		
	else if (TCD0.CNT > count_per_ms * compare_ms){ //if the counter is larger than the compare, reset the count and return true
	    TCD0.CNT = 0;
		return true;
	}
	else
	    return false;	    
}

static void read_encoder(){
    static bool last_a = 0;
    bool current_a;
    bool current_b;
    
    //read current pin states
    current_a = !(PORTB.IN & 0x01);
    current_b = !((PORTB.IN >> 1) & 0x01);
    
    //determine movement of encoder from edge of A and state of B
    //and set encoder_state appropriately
    if (!last_a & current_a)
    {
		manager.encoder_and_switch_info |= (1 << 7);    //set turn flag
        if (current_b)
		    manager.encoder_and_switch_info |= (1 << 6); //set direction flag
        else
            manager.encoder_and_switch_info &= ~(1 << 6);    //clear direction flag
    }
    else
        manager.encoder_and_switch_info &= ~(3 << 6);  //clear turn and direction flags  
    last_a = current_a;
}

turn_state get_encoder(){
	if (manager.encoder_and_switch_info & (1 << 7)){
	    if (manager.encoder_and_switch_info & (1 << 6))
		    return TURN_CW;
		else
		    return TURN_CCW;
	}			
	else
        return TURN_NONE;
}

static void initialize_pots(){
    PORTA.DIRCLR = 0xF9;        //ADC3:7 and VREF input
    ADCA.CTRLA = 0x00;          //disable ADC
    ADCA.CTRLB = 0x10;          //ADC to signed mode
    ADCA.REFCTRL = 0x20;        //set PORTA reference voltage
    ADCA.EVCTRL = 0x00;
    ADCA.PRESCALER = 0x02;     //set prescaler to clk/16 for accuracy
    ADCA.INTFLAGS = 0x00;
    ADCA.CTRLA |= 0x01;         //enable ADC
    ADCA.CH0.CTRL = 0x01;       //select external single-ended input
    ADCA.CH0.MUXCTRL = 0x00;
    ADCA.CH0.INTCTRL = 0x00;
}

static void read_pots(){
    volatile uint8_t i;
    volatile int16_t new_reading;
    
    //cycle through each ADC input and read the values
    //and set the variables appropriately
    for(i = 0; i < 5; i++){
        ADCA.CH0.INTFLAGS |= 0x01;           //clear interrupt flag
        ADCA.CH0.MUXCTRL &= ~(0x07 << 3);    //clear pin select
        ADCA.CH0.MUXCTRL |= ((i+3) << 3);    //set pin select to current input
        ADCA.CH0.CTRL |=    0x80;            //start conversion
        
        while(!(ADCA.CH0.INTFLAGS & 0x01)){} //wait for read to complete

        //load ADC value into the new variable
        new_reading = ADCA.CH0.RESL;
        new_reading |= ADCA.CH0.RESH << 8;
        
        if (new_reading < POT_MIN)
            new_reading = POT_MIN;
        else
            new_reading = new_reading - POT_MIN;
        
        //LPF on new value to reduce noise
        manager.pot_values[i] = manager.pot_values[i] + (new_reading - (int16_t) manager.pot_values[i])/POT_FILTER_COEFF;
    }
    
}

uint16_t get_pot_value(uint8_t pot_select, uint16_t output_min, uint16_t output_max){
    //pot_select: 0-4 to select input pot
    //output_min: minimum value to output
    //output_max: maximum value to output
    
    const uint16_t pot_range = POT_MAX - POT_MIN + 1; 
    volatile float temp;
    
    temp = 1.0*manager.pot_values[pot_select]/pot_range;
    
    if (ALL_EIGHT_POSITION_SWITCHES && output_max <= 7)
        temp = temp*(7 - output_min + 1) + output_min;
    else
        temp = temp*(output_max - output_min + 1) + output_min;
        
    if (temp > output_max)
        temp = output_max;
        
    if (temp < output_min)
        temp = output_min;
    
    return (uint16_t) temp;
}

static void initialize_LEDs(){
    //initialize all LED outputs, set all as blank
    
    PORTA.DIRSET = 0x06;    //~LT and ~BL output
    PORTA.OUTSET = 0x06;    //~LT and ~BL high
    
    PORTC.DIRSET = 0xF8;    //STATLED and LED0:3 output
    PORTD.DIRSET = 0x3F;    //DSEL0:2 and decimal_points0:2 output
    
    
    //All LEDs off
    PORTD.OUTSET = 0x38;    //DSEL0:2 high (arm all 7 segments)
    PORTC.OUTSET = 0xF0;    //LED0:3 high (blank all 7 segments)
    PORTD.OUTCLR = 0x38;    //DSEL0: low (disarm all 7 segments)
    
    PORTD.OUTCLR = 0x07;    //decimal_points0:2 low (blank all decimal_pointss)
    PORTC.OUTSET = 0x08;    //STATLED high (blank statled)
}

void set_seven_segment_LEDs(uint16_t seven_segment_value){
	uint8_t i;
    uint8_t digit;
	bool all_blank = seven_segment_value > 999;
	
	manager.seven_segment_LEDs_state = seven_segment_value;
    
    for (i=0 ; i<3 ; i++){
        digit = seven_segment_value%10;                 //extract lowest current digit of 7seg
        if (all_blank || (seven_segment_value==0 && (i>0)))            //if the rest of the 7seg is zero, blank LEDS (except for 1st digit)
            digit = 10;
            
        PORTD.OUTCLR = 0x08 << (i+2)%3;         //arm appropriate 7 segment        (CHANGE INDEX SCALING FOR NEXT REVISION)
        PORTC.OUTCLR = 0xF0;                    //clear digit select
        PORTC.OUTSET = digit << 4;              //set digit select #
        PORTD.OUTSET = 0x38;                    //disarm all 7 segments
        
        seven_segment_value = seven_segment_value/10;           //shift 7seg number down to next digit 
    }
	

}		

void set_LED_on(LED_choose choice){
    manager.LEDs_states |= (1 << choice);
    switch(choice){
        case LED_STATUS:            PORTC.OUTCLR = 0x08;
                                    break;
        case LED_DECIMAL_POINT_0:   PORTD.OUTSET = 0x04;
                                    break;
        case LED_DECIMAL_POINT_1:   PORTD.OUTSET = 0x01;
                                    break;
        case LED_DECIMAL_POINT_2:   PORTD.OUTSET = 0x02;
                                    break;
    }
}

void set_LED_off(LED_choose choice){
    manager.LEDs_states &= ~(1 << choice);
    switch(choice){
        case LED_STATUS:            PORTC.OUTSET = 0x08;
									break;
        case LED_DECIMAL_POINT_0:   PORTD.OUTCLR = 0x04;
									break;
        case LED_DECIMAL_POINT_1:   PORTD.OUTCLR = 0x01;
									break;
        case LED_DECIMAL_POINT_2:   PORTD.OUTCLR = 0x02;
									break;
    }
}

void set_LEDs_nibble(uint8_t nibble){
	manager.LEDs_states = 0x0F & nibble;
}

bool get_LED(LED_choose choice){
	return manager.LEDs_states & (1 << choice);
}

uint8_t get_LEDs_nibble(){
	return manager.LEDs_states;
}

static void initialize_switches(){
    PORTB.DIRCLR = 0x0C;                //SW8(push) and Encoder pushbutton input
    PORTE.DIRCLR = 0x08;                //SW7(toggle) input
	
    manager.encoder_and_switch_info = 0x00;
}

static void read_switches(){
    
    //switch values = {(5b) unused, encoder, pushbutton, toggle}
    static uint8_t current_switch_states = 0x00;
    static uint8_t last_switch_states = 0x00;
    static uint8_t final_switch_states = 0x00;
    
    static uint8_t switch_history_counts[3] = {0,0,0};

    uint8_t i;
    
    //read current physical switch states
    current_switch_states = 0x00;
    current_switch_states |= !(PORTE.IN >> 3) & 0x01;
    current_switch_states |= !((PORTB.IN >> 2) & 0x01) << 1;
    current_switch_states |= !((PORTB.IN >> 3) & 0x01) << 2;
    
    //compare current switch states with the history
    //set the final switch value appropriately if history is large enough
    for (i = 0; i < 3; i++){
        if (current_switch_states >> i == last_switch_states >> i) //if switch didn't change
            switch_history_counts[i]++;                            //increment count        
        else
            switch_history_counts[i] = 0;                          //else reset count            
        if (switch_history_counts[i] > DEBOUNCE){                  //if count is over debounce value
            final_switch_states &= ~(1 << i);
            final_switch_states |= current_switch_states & (1 << i);             //set final switch to current position
            switch_history_counts[i] = 0;                          //and reset count
        }
    }    
    
    last_switch_states = current_switch_states;                    //set last switch position to current switch position
    
	final_switch_states = current_switch_states; //DELETE THIS.. JUST FOR DEBUGGING!!!!
    //detect rising and falling edges
    //set switch booleans for state and edges appropriately
    
	for (int i = 0; i < 3; i++){
        if (final_switch_states & (1 << i)){                        //if switch IS on
            if (manager.encoder_and_switch_info & (1 << 2*i))         //if switch WAS on
			    manager.encoder_and_switch_info &= ~(1 << (2*i+1));   //maintain state (set), clear the edge
            else                                                    //else (switch WAS off)
                manager.encoder_and_switch_info |= (0x03 << 2*i);     //set edge and set state
        }        
        else{                                                       //if switch IS off
            if (manager.encoder_and_switch_info & (1 << 2*i)){         //if switch WAS on
                manager.encoder_and_switch_info |= (1 << (2*i+1));     //set edge
				manager.encoder_and_switch_info &= ~(1 << 2*i);        //and clear state
			}				
            else                                                    //else (switch WAS off)
                 manager.encoder_and_switch_info &= ~(1 << (2*i+1));  //maintain state (clear), clear the edge
        }        
	}
	/*  
    if (final_switch_states & 0x02){                //if pushbutton IS on
        if (manager.pushbutton_switch_state)                //if pushbutton WAS on
            manager.pushbutton_switch_edge = EDGE_NONE;     //no edge
        else                                        //else (pushbutton WAS off)
            manager.pushbutton_switch_edge = EDGE_RISE;     //new on
        manager.pushbutton_switch_state = 1;                //set current value
    }        
    else{                                           //if pushbutton IS off
        if (manager.pushbutton_switch_state)                //if pushbutton WAS on
            manager.pushbutton_switch_edge = EDGE_FALL;     //new off
        else                                        //else (pushbutton WAS off)
            manager.pushbutton_switch_edge = EDGE_NONE;     //no edge
        manager.pushbutton_switch_state = 0;                //set current value
    }        
    
    if (final_switch_states & 0x04){             //if encoder IS on
        if (manager.encoder_switch_state)                //if encoder WAS on
            manager.encoder_switch_edge = EDGE_NONE;     //no edge
        else                                     //else (encoder WAS off)
            manager.encoder_switch_edge = EDGE_RISE;     //new on
        manager.encoder_switch_state = 1;                //set current value
    }        
    else{                                        //if encoder IS off
        if (manager.encoder_switch_state)                //if encoder WAS on
            manager.encoder_switch_edge = EDGE_FALL;     //new off
        else                                     //else (encoder WAS off)
            manager.encoder_switch_edge = EDGE_NONE;     //no edge
        manager.encoder_switch_state = 0;                //set current value
    }        
  */
}

switch_edge get_switch_edge(switch_select s){
	if (manager.encoder_and_switch_info & (1 << (2*s+1))){
		if (manager.encoder_and_switch_info & (1 << 2*s))
		    return EDGE_RISE;
		else
		    return EDGE_FALL;
	}
	else
	    return EDGE_NONE;    
}

bool get_switch_state(switch_select s){
	return manager.encoder_and_switch_info & (1 << 2*s);
}

uint8_t get_raw_encoder_and_switch_info(){
	return manager.encoder_and_switch_info;
}

uint16_t get_seven_segment_LED_state(){
	return manager.seven_segment_LEDs_state;
}

HardwareManager* initialize_hardware(){
    initialize_HardwareManager();
    initialize_clock();
    initialize_MIDI();
    initialize_pots();
    initialize_switches();
    initialize_encoder();
    initialize_LEDs();
	initialize_realtime_utility();
    return &manager;
}

void read_hardware(){
    read_switches();
    read_pots();
    read_encoder();
}