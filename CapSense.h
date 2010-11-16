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


//==================================================================
// Internal-use routines.

#ifdef IN_CAPSENSE
#define CAPSENSE_EXTERN
#else
#define CAPSENSE_EXTERN  extern
#endif
CAPSENSE_EXTERN byte currentCapSenseChannel;
CAPSENSE_EXTERN CapSenseReading csReadings[MAX_CAPSENSE_CHANNELS];
CAPSENSE_EXTERN byte csButton;

// The lowest value read on the given channel in the last two seconds.
CAPSENSE_EXTERN CapSenseReading csGlobalMin[MAX_CAPSENSE_CHANNELS];

#define NUM_CAPSENSE_MIN_BINS  4
CAPSENSE_EXTERN CapSenseReading csMinBin[MAX_CAPSENSE_CHANNELS][NUM_CAPSENSE_MIN_BINS];
CAPSENSE_EXTERN byte csCurrentMinBin;

#define TICKS_PER_BIN_CHANGE  2
CAPSENSE_EXTERN byte csLastBinTicks;

CAPSENSE_EXTERN byte csLastButtonTicks;

// Set to one of the channels when that button is down, or NO_CAPSENSE_BUTTONS if none are down.
// Will often still be down after GetCapSenseButton() has cleared csButton.
CAPSENSE_EXTERN byte csHoldingButton;


// Returns the last reading from the given sensor (0-3).
// (Readings are filtered before they're accessed here.)
inline CapSenseReading GetLastCapSenseReading(byte index)
{
	return csReadings[index];
}

// For internal use.
// It's here so it can be inlined, for speed.
inline void RestartCapSenseTimer(void)
{
	// Clear out and reset both timers.
	// Timer 1 will start counting oscillations afresh,
	// and Timer 0 will restart its count to 256 cycles till the interrupt.
	tmr0 = 0;
	t1con.TMR1ON = 0;
	tmr1l = 0;
	tmr1h = 0;
	t1con.TMR1ON = 1;
	intcon.T0IF = 0;
}

inline void SetCapSenseChannel(void)
{
	// In addition to selecting the channel,
	// these vaules connect the comparators to the right voltage references,
	// and set their outputs to the inputs of the SR latch.
	cm1con0 = 0x94 + currentCapSenseChannel;
	
	// This also sets comparator's 2 output to appear on the C2OUT pin, which
	// is routed to charge and discharge all of the sensors in parallel.
	cm2con0 = 0xA0 + currentCapSenseChannel;
}

inline void BumpCapSenseChannel(void)
{
	currentCapSenseChannel++;
	
	// Skip unused channels.
	#if !(CAPSENSE_CHANNELS & CAPSENSE_CHANNEL0)
	if (currentCapSenseChannel == 0)
		currentCapSenseChannel++;
	#endif
	#if !(CAPSENSE_CHANNELS & CAPSENSE_CHANNEL1)
	if (currentCapSenseChannel == 1)
		currentCapSenseChannel++;
	#endif
	#if !(CAPSENSE_CHANNELS & CAPSENSE_CHANNEL2)
	if (currentCapSenseChannel == 2)
		currentCapSenseChannel++;
	#endif
	#if !(CAPSENSE_CHANNELS & CAPSENSE_CHANNEL3)
	if (currentCapSenseChannel == 3)
		currentCapSenseChannel++;
	#endif
	
	if (currentCapSenseChannel >= MAX_CAPSENSE_CHANNELS)
		currentCapSenseChannel = 0;
		
	SetCapSenseChannel();

	// Start the next reading.
	RestartCapSenseTimer();
}

void BumpCapSenseMinBin(void);

//==================================================================
// Interrupt routines.

// Call this first in the main ISR.
// It returns true if there was a Timer0 interrupt.
// It's inline (and therefore so is much of this module) to save the call/return overhead in the ISR,
// since it happens so often.
inline byte CapSenseISR(void)
{
	if (UiTimeInterrupt()) {
		// Read TMR1.
		CapSenseReading reading = (tmr1h << 8) | tmr1l;
		
		// Do some of the indexing once.
		CapSenseReading* currentGlobalMin = &csGlobalMin[currentCapSenseChannel];
		CapSenseReading* currentReading = &csReadings[currentCapSenseChannel];
		
		// Compute the "pressed" threshold.
		CapSenseReading threshold = *currentGlobalMin;
		if (threshold > CS_SENSE_THRESHOLD) {
			threshold -= CS_SENSE_THRESHOLD;
			threshold = max(threshold, CS_MIN_THRESHOLD);
		} else
			threshold = CS_MIN_THRESHOLD;
		
		// Filter the new value.
		// Running average, over 16 samples.
		reading = *currentReading + (reading - *currentReading) / 16;
		*currentReading = reading;

		// Is it a button press?
		bit isBelowMin = reading < threshold;
		if (isBelowMin) {
			if (csButton == NO_CAPSENSE_BUTTONS && csHoldingButton == NO_CAPSENSE_BUTTONS && isBelowMin) 
			{
				// Yes: note it.
				csButton = currentCapSenseChannel;
				csLastButtonTicks = ticks;
				csHoldingButton = currentCapSenseChannel;
			}
		} else {
			// If this is the button we were holding, we're not holding it anymore.
			if (csHoldingButton == currentCapSenseChannel)
				csHoldingButton = NO_CAPSENSE_BUTTONS;
		}
	
		// Update the minima.
		// But not if a button is currently down... until it's been down for a second.
		if (!csHoldingButton == NO_CAPSENSE_BUTTONS || (ticks - csLastButtonTicks) > TICKS_PER_SEC) {
			CapSenseReading* currentMinBin = &csMinBin[currentCapSenseChannel][csCurrentMinBin];
			if (reading < *currentMinBin) {
				*currentMinBin = reading;
				
				if (reading < *currentGlobalMin)
					*currentGlobalMin = reading;
			}
		}
		
		// Move to the next min bin, every other tick (about twice a second).
		if (ticks - csLastBinTicks >= TICKS_PER_BIN_CHANGE)
			BumpCapSenseMinBin();
	
		// Move to the next sensor.
		BumpCapSenseChannel();
	
		return true;
	} else
		return false;
}

// Call this at the end of the ISR, after other processing.
inline void CapSenseISRDone(void)
{
	if (intcon.T0IF)
		// See the app notes: This means another interrupt has occurred before we returned from the last one.
		// That means the current count is inaccurate, so it must be discarded.
		RestartCapSenseTimer();
}

#endif
// _CAPSENSE_H