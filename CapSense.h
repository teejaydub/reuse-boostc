/* CapSense.h
	Using capacitive touch sensing to simulate up to 4 pushbuttons.
	Follows the guidelines set out in Microchip's AN1101 and AN1103.
	
	Takes over the following resources:
		comparators 1 and 2
		the S-R latch
		the voltage reference
		timers 0 and 1
*/

#ifndef _CAPSENSE_H
#define _CAPSENSE_H


#ifdef IN_CAPSENSE
#define CAPSENSE_EXTERN
#else
#define CAPSENSE_EXTERN  extern
#endif
CAPSENSE_EXTERN unsigned char currentCapSenseChannel;


// Initializes.
// The pin assignments are dictated by the peripherals used in this approach.
void InitCapSense(void);

// Call this when sensing should start.
// Global interrupts must be enabled before this call.
void StartCapSense(void);

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
#if 0
	currentCapSenseChannel++;
	
	if (currentCapSenseChannel >= NUM_CAPSENSE_CHANNELS)
		currentCapSenseChannel = 0;
#endif
		
	SetCapSenseChannel();
}

// Call this first in the main ISR.
// It returns true if there was a Timer0 interrupt.
inline unsigned char CapSenseISR(void)
{
	if (intcon.T0IF) {
		// Read TMR1.
		
		// Is it a button press?
	
		// Average the new value.
	
		// Move to the next sensor.
		BumpCapSenseChannel();
	
		// Start the next reading.
		RestartCapSenseTimer();
		
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