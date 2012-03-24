// Copyright (c) 2012, David Tuzman, All Right Reserved

#include "hardware.h"

static HardwareManager manager;

HardwareManager* get_hardware_manager_ptr(){
    return &manager;
}    
    
void initialize_HardwareManager(){
    manager.encoder_state = TURN_NONE;
    manager.pushbutton_switch_edge = EDGE_NONE;
    manager.toggle_switch_edge = EDGE_NONE;
    manager.encoder_switch_edge = EDGE_NONE;
    manager.pushbutton_switch_state = 0;
    manager.toggle_switch_state = 0;
    manager.encoder_switch_state = 0;
    
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
        if (current_b)
            manager.encoder_state = TURN_CW;    //CW        
        else
            manager.encoder_state = TURN_CCW;    //CCW
    }
    else
        manager.encoder_state = TURN_NONE;   
    last_a = current_a;
}

turn_state get_encoder(){
    return manager.encoder_state;
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
    
    for (i=0 ; i<3 ; i++){
        digit = seven_segment_value%10;                 //extract lowest current digit of 7seg
        if (seven_segment_value==0 && (i>0))            //if the rest of the 7seg is zero, blank LEDS (except for 1st digit)
            digit = 10;
            
        PORTD.OUTCLR = 0x08 << (i+2)%3;         //arm appropriate 7 segment        (CHANGE INDEX SCALING FOR NEXT REVISION)
        PORTC.OUTCLR = 0xF0;                    //clear digit select
        PORTC.OUTSET = digit << 4;              //set digit select #
        PORTD.OUTSET = 0x38;                    //disarm all 7 segments
        
        seven_segment_value = seven_segment_value/10;           //shift 7seg number down to next digit 
    }
}

void set_LED_on(LED_choose choice){
    //booleans and such convert to LED out
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
    //booleans and such convert to LED out
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

static void initialize_switches(){
    PORTB.DIRCLR = 0x0C;                //SW8(push) and Encoder pushbutton input
    PORTE.DIRCLR = 0x08;                //SW7(toggle) input
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
    
    //detect rising and falling edges
    //set switch booleans for state and edges appropriately
    
    if (final_switch_states & 0x01){            //if toggle IS on
        if (manager.toggle_switch_state)                //if toggle WAS on
            manager.toggle_switch_edge = EDGE_NONE;     //no edge
        else                                    //else (toggle WAS off)
            manager.toggle_switch_edge = EDGE_RISE;     //new on
        manager.toggle_switch_state = 1;                //set current value
    }        
    else{                                       //if toggle IS off
        if (manager.toggle_switch_state)                //if toggle WAS on
            manager.toggle_switch_edge = EDGE_FALL;     //new off
        else                                    //else (toggle WAS off)
            manager.toggle_switch_edge = EDGE_NONE;     //no edge
        manager.toggle_switch_state = 0;                //set current value
    }        
    
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
  
}

bool get_encoder_switch_state(){
    return manager.encoder_switch_state;
}

switch_edge get_encoder_switch_edge(){
    return manager.encoder_switch_edge;
}

bool get_pushbutton_switch_state(){
    return manager.pushbutton_switch_state;
}

switch_edge get_pushbutton_switch_edge(){
    return manager.pushbutton_switch_edge;
}

bool get_toggle_switch_state(){
    return manager.toggle_switch_state;
}

switch_edge get_toggle_switch_edge(){
    return manager.toggle_switch_edge;
}

HardwareManager* initialize_hardware(){
    initialize_HardwareManager();
    initialize_clock();
    initialize_MIDI();
    initialize_pots();
    initialize_switches();
    initialize_encoder();
    initialize_LEDs();
    return &manager;
}

void read_hardware(){
    read_switches();
    read_pots();
    read_encoder();
}

/*void postloop_functions(bool status_LED, bool decimal_point_0, bool decimal_point_1, bool decimal_point_2, uint16_t seven_segment_value){
    
    
    //set_LEDs(status_LED, decimal_point_0, decimal_point_1, decimal_point_2, seven_segment_value);
}*/