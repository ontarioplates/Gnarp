
#ifndef HWSETUP_H_
#define HWSETUP_H_

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define DEBOUNCE 8
#define POTMIN 0x00E0
#define POTMAX 0x0FFF

typedef enum {TURN_NONE, TURN_CW, TURN_CCW}
turn_state;

typedef enum {EDGE_NONE, EDGE_RISE, EDGE_FALL}
switch_edge;
    
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


#endif /* HWSETUP_H_ */