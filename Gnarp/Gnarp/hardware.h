// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#define POT_FILTER_COEFF 20.0 /**< LPF coefficient used while reading pots and rotary switches.  As this increases, value changes will be slower and noise is reduced.*/
#define NUM_POTS 5
#define DEBOUNCE 8
#define POT_MIN 0
#define POT_MAX 2047

#define ALL_EIGHT_POSITION_SWITCHES true

typedef enum {TURN_NONE, TURN_CW, TURN_CCW}
turn_state;

typedef enum {EDGE_NONE, EDGE_RISE, EDGE_FALL}
switch_edge;

typedef enum {LED_STATUS, LED_DECIMAL_POINT_0, LED_DECIMAL_POINT_1, LED_DECIMAL_POINT_2}
LED_choose;

typedef struct HardwareManager HardwareManager;

/**
 * @brief This structure stores the information for all the input hardware
 */
struct HardwareManager{
    turn_state encoder_state; /**< Tracks movement of the rotary encoder*/
	
    switch_edge pushbutton_switch_edge; /**< Tracks movement of the pushbutton*/
    switch_edge toggle_switch_edge; /**< Tracks movement of the toggle switch*/
    switch_edge encoder_switch_edge; /**< Tracks movement of the encoder pushbutton*/
	
    bool pushbutton_switch_state; /**< Current state of the pushbutton*/
    bool toggle_switch_state; /**< Current state of the toggle switch*/
    bool encoder_switch_state; /**< Current state of the encoder pushbutton*/
	
    uint16_t pot_values[5]; /**< Array of the raw values of each pot/switch (bounded by #POT_MIN and #POT_MAX)*/
};

/**
 * @defgroup harware_functions I/O Hardware Functions
 *
 * Functions used when reading and writing to the user interface hardware.
 *
 * Use these functions to gather information from the unit's electromechanical input hardware (switches, pots, buttons) and
 * to send information to the output hardware (LEDs).
 *
 * All functions are hard-coded to use data inside the statically declared #HardwareManager "manager".
 *
 * @{
 */

/**
 * @brief Initialize all necessary MCU registers for Hardware I/O
 * 
 * This function must be called once at the beginning of the code in order for the hardware to respond properly.
 * Initializes all Inputs (switches, pots, MIDI, encoder, sync) and all Outputs (LEDs, MIDI, sync).
 * 
 * @return HardwareManager pointer to store in the main source.  This can be used while debugging to view the hardware values.
 */    
HardwareManager* initialize_hardware();

/**
 * @brief Read the instantaneous hardware values and load them into the HardwareManager
 *
 * This function must be called frequently in order to update the HardwareManger values, maintaining accurate
 * software values to represent the hardware state.
 */
void read_hardware();

/**
 * @brief Turn on the specified LED
 *
 * @param choice LED to turn on
 */
void set_LED_on(LED_choose choice);

/**
 * @brief Turn off the specified LED
 *
 * @param choice LED to turn off
 */
void set_LED_off(LED_choose choice);

/**
 * @brief Illuminate the 7-segment display to the specified number
 *
 * @param seven_segment_value 1-3 digit number to write on the 7-segment display
 */
void set_seven_segment_LEDs(uint16_t seven_segment_value);


//void postloop_functions(bool status_LED, bool decimal_point_0, bool decimal_point_1, bool decimal_point_2, uint16_t seven_segment_value);

/**
 * @brief Request the turn state of the rotary encoder
 *
 * @return TURN_CW if the encoder rotates clockwise
 * @return TURN_CCW if the encoder rotates counterclockwise
 * @return TURN_NONE otherwise (no movement)
 */
turn_state get_encoder();

/**
 * @brief Request a linearly scaled value of a specified pot (or rotary switch)
 *
 * @param pot index of the pot from which to scale the value
 * @param outmin minimum output value
 * @param outmax maximum output value
 * @return 16 bit integer of the scaled value of the pot position
 */
uint16_t get_pot_value(uint8_t pot, uint16_t outmin, uint16_t outmax);

/**
 * @brief Request the state of the encoder pushbutton
 *
 * @return 1 if the switch is currently pressed
 * @return 0 otherwise
 */
bool get_encoder_switch_state();

/**
 * @brief Request the state of the pushbutton
 *
 * @return 1 if the switch is currently pressed
 * @return 0 otherwise
 */
bool get_pushbutton_switch_state();

/**
 * @brief Request the state of the toggle switch
 *
 * @return 1 if the switch is currently in the on position
 * @return 0 otherwise
 */
bool get_toggle_switch_state();

/**
 * @brief Request the transient state of the encoder pushbutton
 *
 * @return EDGE_RISE if the switch is turning on
 * @return EDGE_FALL if the switch is turning off
 * @return EDGE_NONE otherwise (no change)
 */
switch_edge get_encoder_switch_edge();

/**
 * @brief Request the transient state of the pushbutton
 *
 * @return EDGE_RISE if the switch is turning on
 * @return EDGE_FALL if the switch is turning off
 * @return EDGE_NONE otherwise (no change)
 */
switch_edge get_pushbutton_switch_edge();

/**
 * @brief Request the transient state of the toggle switch
 *
 * @return EDGE_RISE if the switch is turning on
 * @return EDGE_FALL if the switch is turning off
 * @return EDGE_NONE otherwise (no change)
 */
switch_edge get_toggle_switch_edge();

/**@}*/


#endif /* HARDWARE_H_ */