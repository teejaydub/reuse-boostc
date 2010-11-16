/* CapSense.c
	Using capacitive touch sensing to simulate up to 4 pushbuttons.
	Follows the guidelines set out in Microchip's AN1101 and AN1103.
*/

#define IN_CAPSENSE

#include <system.h>
#include <memory.h>
#include <stdlib.h>

#include "CapSense.h"
#include "CapSense-consts.h"


byte csButton;

CapSenseReading csReadings[MAX_CAPSENSE_CHANNELS];

byte currentCapSenseChannel;

// The lowest value read on the given channel in the last two seconds.
CapSenseReading csGlobalMin[MAX_CAPSENSE_CHANNELS];

#define NUM_CAPSENSE_MIN_BINS  4
CapSenseReading csMinBin[MAX_CAPSENSE_CHANNELS][NUM_CAPSENSE_MIN_BINS];
byte csCurrentMinBin;

#define TICKS_PER_BIN_CHANGE  2
byte csLastBinTicks;

byte csLastButtonTicks;

// Set to one of the channels when that button is down, or NO_CAPSENSE_BUTTONS if none are down.
// Will often still be down after GetCapSenseButton() has cleared csButton.
byte csHoldingButton;


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

void InitCapSense(void)
{
	#ifndef _PIC16F886
	 #error "Need to determine register usage for this algorithm used with this chip."
	#endif
	
	// Set up the relaxation oscillator.
	// Values taken from Appendix A of Microchip AN1101.
	cm2con1 = 0x32;
	srcon = 0xF0;
	vrcon = 0x8D;  // Enable the voltage reference, in the low range, as 21/32 of Vdd.
	
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL0
	ansel.0 = 1;  // on RA0, AN0
	trisa.0 = 1;
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL1
	ansel.1 = 1;  // on RA1, AN1
	trisa.1 = 1;
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL2
	anselh.1 = 1;  // on RB3, AN9
	trisb.3 = 1;
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL3
	anselh.2 = 1;  // on RB1, AN10
	trisb.1 = 1;
	#endif

	// Set current to the first one we're using.
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL0
	currentCapSenseChannel = 0;
	csReadings[0] = 0;
	csGlobalMin[0] = 0;
	#elif CAPSENSE_CHANNELS & CAPSENSE_CHANNEL1
	currentCapSenseChannel = 1;
	csReadings[1] = 0;
	csGlobalMin[1] = 0;
	#elif CAPSENSE_CHANNELS & CAPSENSE_CHANNEL2
	currentCapSenseChannel = 2;
	csReadings[2] = 0;
	csGlobalMin[2] = 0;
	#elif CAPSENSE_CHANNELS & CAPSENSE_CHANNEL3
	currentCapSenseChannel = 3;
	csReadings[3] = 0;
	csGlobalMin[3] = 0;
	#endif

	// The low voltage reference is always used, and is on RA2, AN2.
	ansel.2 = 1;
	trisa.2 = 1;
	
	// The SR latch outputs on RA5, which is also C2OUT.
	trisa.5 = 0;
	
	// RC0 is T1CKI, which must be hard-wired to C2OUT externally.
	trisc.0 = 1;
	
	// Timer 1 takes its input from the T1CKI pin.
	t1con.TMR1CS = 1;
	
	// Set up the interrupt on TMR0 overflow.
	// It runs free, and we check TMR1's value on each TMR0 overflow interrupt.
	InitUiTime_Timer0();
	
	currentCapSenseChannel = 0;
	csCurrentMinBin = 0;
	
	// Clear all bins.
	memset(csMinBin, 0, MAX_CAPSENSE_CHANNELS * NUM_CAPSENSE_MIN_BINS * sizeof(CapSenseReading)); 
	memset(csGlobalMin, 0, MAX_CAPSENSE_CHANNELS * sizeof(CapSenseReading));
	csLastBinTicks = ticks;
	csHoldingButton = NO_CAPSENSE_BUTTONS;

	SetCapSenseChannel();
	RestartCapSenseTimer();
	
	csButton = NO_CAPSENSE_BUTTONS;
}

CapSenseReading GetLastCapSenseReading(byte index)
{
	return csReadings[index];
}

byte GetCapSenseButton(void)
{
	byte result = csButton;
	csButton = NO_CAPSENSE_BUTTONS;
	return result;
}

inline void BumpCapSenseMinBin(void)
{
	if (++csCurrentMinBin >= NUM_CAPSENSE_MIN_BINS)
		csCurrentMinBin = 0;
	
	// Reset each channel's newly-current bin to contain just that channel's most-recent reading.
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL0
	csMinBin[0][csCurrentMinBin] = csReadings[0];
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL1
	csMinBin[1][csCurrentMinBin] = csReadings[1];
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL2
	csMinBin[2][csCurrentMinBin] = csReadings[2];
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL3
	csMinBin[3][csCurrentMinBin] = csReadings[3];
	#endif
	
	// Find the global min again, over all bins, for all channels.
	CapSenseReading min1, min2;
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL0
	min1 = min(csMinBin[0][0], csMinBin[0][1]);
	min2 = min(csMinBin[0][2], csMinBin[0][3]);
	csGlobalMin[0] = min(min1, min2);
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL1
	min1 = min(csMinBin[1][0], csMinBin[1][1]);
	min2 = min(csMinBin[1][2], csMinBin[1][3]);
	csGlobalMin[1] = min(min1, min2);
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL2
	min1 = min(csMinBin[2][0], csMinBin[2][1]);
	min2 = min(csMinBin[2][2], csMinBin[2][3]);
	csGlobalMin[2] = min(min1, min2);
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL3
	min1 = min(csMinBin[3][0], csMinBin[3][1]);
	min2 = min(csMinBin[3][2], csMinBin[3][3]);
	csGlobalMin[3] = min(min1, min2);
	#endif

	csLastBinTicks = ticks;
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

byte CapSenseISR(void)
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

void CapSenseISRDone(void)
{
	if (intcon.T0IF)
		// See the app notes: This means another interrupt has occurred before we returned from the last one.
		// That means the current count is inaccurate, so it must be discarded.
		RestartCapSenseTimer();
}
