// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#define POT_FILTER_COEFF 4.0
#define NUM_POTS 5
#define DEBOUNCE 8
#define POT_MIN 0x00E0
#define POT_MAX 0x0FFF

typedef enum {TURN_NONE, TURN_CW, TURN_CCW}
turn_state;

typedef enum {EDGE_NONE, EDGE_RISE, EDGE_FALL}
switch_edge;

typedef struct Hardware_Manager Hardware_Manager;

struct Hardware_Manager{
    turn_state encoder_state;
	
    switch_edge pushbutton_switch_edge;
    switch_edge toggle_switch_edge;
    switch_edge encoder_switch_edge;
	
    bool pushbutton_switch_state;
    bool toggle_switch_state;
    bool encoder_switch_state;
	
    uint16_t pot_values[5];
};
    
void initialize_hardware();
void read_hardware();

void set_LEDs_on(bool status_LED, bool decimal_point_0, bool decimal_point_1, bool decimal_point_2);
void set_LEDs_off(bool status_LED, bool decimal_point_0, bool decimal_point_1, bool decimal_point_2);
void set_seven_segment_LEDs(uint16_t seven_segment_value);
void postloop_functions(bool status_LED, bool decimal_point_0, bool decimal_point_1, bool decimal_point_2, uint16_t seven_segment_value);

turn_state get_encoder();
uint16_t get_pot_value(uint8_t pot, uint16_t outmin, uint16_t outmax);
bool get_encoder_switch_state();
bool get_pushbutton_switch_state();
bool get_toggle_switch_state();
switch_edge get_encoder_switch_edge();
switch_edge get_pushbutton_switch_edge();
switch_edge get_toggle_switch_edge();


#endif /* HARDWARE_H_ */