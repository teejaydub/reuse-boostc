/* CapSense.h
	Using capacitive touch sensing to simulate up to 4 pushbuttons.
	Follows the guidelines set out in Microchip's AN1101 and AN1103.
	
	Takes over the following resources:
		comparators 1 and 2
		the S-R latch
		the voltage reference
		timers 0 and 1
		
	Requires:
		types-tjw
		uiTime (which it initializes and updates)
		
	Call InitCapSense(), and CapSenseISR() and CapSenseISRDone() as described below.
	Then call GetCapSenseButton() to process buttons.
*/

#ifndef _CAPSENSE_H
#define _CAPSENSE_H

#include <stdlib.h>

#include "CapSense-consts.h"
#include "types-tjw.h"
#include "uiTime.h"

#define MAX_CAPSENSE_CHANNELS  4


typedef signed short CapSenseReading;


// Initializes the module.
// The pin assignments are dictated by the peripherals used in this approach.
// Global interrupts must be enabled soon after this call.
void InitCapSense(void);

// Returns NO_CAPSENSE_BUTTONS if no buttons have been pressed,
// or 0-3 if a corresponding button has been pressed.
// Channels correspond to the various negative input pins to Comparators 1 & 2.
// On the PIC16F886, e.g., there are four negative input pins, shared with both comparators,
// on RA0, RA1, RB3, and RB1, in that order (channels 0-3 respectively).
// Calling this indicates that the button has been processed, and clears the way for the next button
// to be reported.
#define NO_CAPSENSE_BUTTONS  0xFF
byte GetCapSenseButton(void);


// Returns the last reading from the given sensor (0-3).
// (Readings are filtered before they're accessed here.)
CapSenseReading GetLastCapSenseReading(byte index);

//==================================================================
// Interrupt routines.

// Call this first in the main ISR.
// It returns true if there was a Timer0 interrupt.
// It's inline (and therefore so is much of this module) to save the call/return overhead in the ISR,
// since it happens so often.
byte CapSenseISR(void);

// Call this at the end of the ISR, after other processing.
void CapSenseISRDone(void);

#endif
// _CAPSENSE_H