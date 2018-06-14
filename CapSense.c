/* CapSense.c
	Using capacitive touch sensing to simulate up to 4 pushbuttons.
	Follows the guidelines set out in Microchip's AN1101 and AN1103.

    Readings are made of the time it takes to charge and discharge the touch pad, acting as a capacitor.
    Read by using Timer 1 as a free-running counter of pulses, and Timer 0 as a periodic interrupt
    to sample that counter.

    Timer 0 rolls over every 1.024 ms, and the timer 1 value at that time becomes the next reading.

    Readings are filtered using an exponential weighted moving average, to reduce the effects of noise.

    Generally, readings are about the same when the system is in a steady state.
    When the user touches a pad, capacitance increases, cycle time lengthens, and Timer 1 counts drop.
    So, if readings drop sharply, it's a button press.

    We choose a value for the "normal" steady-state reading and call this the "baseline."

    The baseline is not consistent across devices, though, or even placement of a device
    (relative to ferrous materials, water, whether the user is holding it, etc.).  So dynamic
    calibration is required.

    For each channel, we have several "bins", that we switch among round-robin,
    every TICKS_PER_BIN_CHANGE (for "ticks" occurring at roughly four times a second).

    We track the maximum (filtered) reading for each bin, and the maximum of all previous bins
    becomes the baseline until we next change bins.

    So, if all readings go down for a given channel, we'll adjust that channel's baseline down
    after TICKS_PER_BIN_CHANGE / TICKS_PER_SEC * NUM_CAPSENSE_BINS seconds.
    When I wrote this, those settings worked out to a little over 2 minutes (in theory).

    We also have a "threshold," that determines how far below the baseline a reading can go
    before it's recognized as a button press.  The threshold is calibrated once and hard-coded,
    per channel.  The smaller the threshold, the easier it is to get a button press.
    
    If thresholds are smaller than the overall noise level in the system, false-positive
    button presses can occur - buttons being pressed when the user doesn't intend to.
*/

#define IN_CAPSENSE

#include <system.h>
#include <memory.h>
#include <stdlib.h>

#include "eeprom-tjw.h"
#include "math-tjw.h"

#include "CapSense.h"
#include "CapSense-consts.h"

// Some equivalent registers that are just named differently.
#ifdef _PIC18F45K22
    #define ansel  ansela
    #define anselh  anselb
    #define tmr0  tmr0l
#endif
 
CapSenseReading csReadings[MAX_CAPSENSE_CHANNELS];

// The channel currently being timed by the hardware.
byte currentCapSenseChannel;

// The baseline value from which we expect low-going excursions when a finger approaches.
CapSenseReading csBaseline[MAX_CAPSENSE_CHANNELS];

#ifdef CS_AUTO_CALIBRATE
// These are kept here during calibration; cleared by the calibration code.

// The min reading for each channel seen during the current state.
CapSenseReading csMin[MAX_CAPSENSE_CHANNELS];
#endif

// This is set if a button was pressed during the current bin - includes held down.
byte csDownInBin[MAX_CAPSENSE_CHANNELS];

#define TICKS_PER_BIN_CHANGE  (60 * TICKS_PER_SEC)
byte csLastBinTicks;

// Do the first few bin switches more quickly.
// This helps get good baseline values in place.
#define TICKS_PER_BIN_CHANGE_INITIAL  1
byte csBinSwitches;

byte csLastButtonTicks;
byte csPollsSinceDown;  // Set to zero when any button is down, incremented when no button is down, up to 255.

// Set to one of the channels when that button is down, or NO_CAPSENSE_BUTTONS if none are down.
// Will often still be down after GetCapSenseButton() has cleared csButton.
byte csHoldingButton;

// Holds the index of the most-recently-pressed button, or NO_CAPSENSE_BUTTONS
// if none have been pressed since the last call to GetCapSenseButton().
byte csButton;


//==================================================================
// Main code

inline void SetCapSenseChannel(void)
{
	// In addition to selecting the channel,
	// these values connect the comparators to the right voltage references,
	// and set their outputs to the inputs of the SR latch.
	cm1con0 = BITMASK(C1ON)  // enabled
        // non-inverted output, not output to pin
        | BITMASK(C1POL)  // inverted output polarity
        #ifdef _PIC18F45K22
        | BITMASK(C1SP)  // high speed
        #endif
        | BITMASK(C1R)  // C1Vin+ connects to C1Vref
        | currentCapSenseChannel;
	
	// This also sets comparator 2's output to appear on the C2OUT pin, which
	// is routed to charge and discharge all of the sensors in parallel.
    cm2con0 = 
        BITMASK(C2ON)  // enabled
        // non-inverted logic, not output to pin, non-inverted output polarity
        #ifdef _PIC18F45K22
        | BITMASK(C2SP)  // high speed
        #endif
        #ifdef INTERNAL_LOW_REF
        | BITMASK(C2R)  // C2Vin+ connects to C2Vref instead of external C12IN+
        #endif
        | currentCapSenseChannel;
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

// Returns true if the channel with the specified index is used, as defined by the constants in CapSense-consts.h.
byte IsChannelUsed(byte channel)
{
	return (1 << channel) & CAPSENSE_CHANNELS;
}

void InitCapSense(void)
{
#if defined(_PIC16F886) || defined(_PIC16F887) || defined(_PIC18F45K22)
    // Set up the relaxation oscillator.
    // It's driven by C1, which detects the upper voltage threshold,
    // and C2, which detects the lower.
    #if defined(_PIC18F45K22)
        #ifdef INTERNAL_LOW_REF
            // Use the FVR for both high and low refs.  This gives us some immunity from line noise?
            // It also omits the external voltage reference, saving a few (cheap) parts.
            vrefcon0 = BITMASK(FVREN)  // Turn the FVR on
                | BITMASK(FVRS1) | BITMASK(FVRS0);  // set to 4.096 V
            vrefcon1 = BITMASK(DACEN)  // Turn DAC on
                | BITMASK(DACPSS1);  // ratiometric from Vss to FVRBUF1.
            vrefcon2 = 16;  // 16/32 of Vdd = 2.048 V of swing, from 2.048 V (from the DAC) to 4.096 V (from the FVR).
            cm2con1 = BITMASK(C1RSEL)  // Use the FVR as C1Vref
                // use DAC for C2Vref
                | BITMASK(C1HYS) | BITMASK(C2HYS);  // use hysteresis on both
                // outputs are asynchronous.
        #else
            vrefcon1 = BITMASK(DACEN);  // Turn DAC on, ratiometric from Vss to Vdd.
            vrefcon2 = 21;  // 21/32 of Vdd = 2.0 V of swing, from 5/4 = 1.25 V (from the ext ref) to 21/32 * 5 V = 3.28 V (from the DAC).
            cm2con1 =  // Use the DAC for C1Vref.  Don't care about C2Vref; it's unused.
                BITMASK(C1HYS) | BITMASK(C2HYS);  // use hysteresis on both
                // outputs are asynchronous.
        #endif
        srcon0 = BITMASK(SRLEN)  // enable the S-R latch
            | BITMASK(SRNQEN);  // enable its negated output on SRNQ
        srcon1 = BITMASK(SRSC1E)  // C1 output sets the S-R latch
            | BITMASK(SRRC2E);  // C2 output resets the S-R latch
    #else
        // Values taken from Appendix A of Microchip AN1101.
        cm2con1 = 0x32;
        srcon = 0xF0;  // C1 output sets the S-R latch; C2 output resets.
        vrcon = 0x8D;  // Enable the voltage reference, in the low range, as 21/32 of Vdd.
        // The external voltage divider must be attached to comparator 2's + input, often RA2.
    #endif

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

	// The low voltage reference is always used, and is on RA2, AN2.
	ansel.2 = 1;
	trisa.2 = 1;
	
	// The SR latch outputs on RA5, which is also C2OUT.
	trisa.5 = 0;
	
	// RC0 is T1CKI, which must be hard-wired to C2OUT externally.
	trisc.0 = 1;
	
	// Timer 1 takes its input from the T1CKI pin.
	#if defined(_PIC18F45K22)
        t1con.TMR1CS1 = 1;
        t1con.TMR1CS0 = 0;
	#else
        t1con.TMR1CS = 1;
    #endif
#elif defined(_PIC16F1789)
	// These won't work, but they should do no harm for now.
	cm2con1 = 0x32;
#else
	#error "Need to determine register usage for this algorithm used with this chip."
#endif
	
	// Set current to the first one we're using.
	currentCapSenseChannel = FIRST_CAPSENSE_CHANNEL;

	// Set up the interrupt on TMR0 overflow.
	// It runs free, and we check TMR1's value on each TMR0 overflow interrupt.
	InitUiTime_Timer0();
	
	// Clear all bins.
	csCurrentBin = 0;
    csBinSwitches = 0;
	memset(csBinMax, 0, sizeof(csBinMax)); 
	memset(csBaseline, 0, sizeof(csBaseline));  // Set to zero to prevent any presses until we've had time to stabilize.
	memset(csReadings, 0, sizeof(csReadings));
	csLastBinTicks = ticks;
	csPollsSinceDown = 255;
	csHoldingButton = NO_CAPSENSE_BUTTONS;
	memset(csDownInBin, 0, sizeof(csDownInBin));
	
	read_eeprom_block(CAPSENSE_EEPROM_ADDR, (char*) csThresholds, CAPSENSE_EEPROM_LEN);

	SetCapSenseChannel();
	RestartCapSenseTimer();
	
	csButton = NO_CAPSENSE_BUTTONS;
}

CapSenseReading GetLastCapSenseReading(byte index)
{
	return csReadings[index];
}

CapSenseReading GetBaseline(byte index)
{
	return csBaseline[index];
}

byte GetCapSenseButton(void)
{
	byte result = csButton;
	csButton = NO_CAPSENSE_BUTTONS;
	
	return result;
}

inline void BumpCapSenseBin(void)
{
	if (++csCurrentBin >= NUM_CAPSENSE_BINS)
		csCurrentBin = 0;
	
	// Find the global max again, over all bins, for all channels.
    #if NUM_CAPSENSE_BINS == 2
    	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL0
    	csBaseline[0] = max(csBinMax[0][0], csBinMax[0][1]);
    	#endif
    	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL1
    	csBaseline[1] = max(csBinMax[1][0], csBinMax[1][1]);
    	#endif
    	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL2
    	csBaseline[2] = max(csBinMax[2][0], csBinMax[2][1]);
    	#endif
    	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL3
    	csBaseline[3] = max(csBinMax[3][0], csBinMax[3][1]);
    	#endif
    #else
        #error "Recode the loop unrolling in BumpCapSenseBin() for this value of NUM_CAPSENSE_BINS."
    #endif

	// Reset each channel's newly-current bin to contain just that channel's most-recent reading.
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL0
	csBinMax[0][csCurrentBin] = csReadings[0];
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL1
	csBinMax[1][csCurrentBin] = csReadings[1];
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL2
	csBinMax[2][csCurrentBin] = csReadings[2];
	#endif
	#if CAPSENSE_CHANNELS & CAPSENSE_CHANNEL3
	csBinMax[3][csCurrentBin] = csReadings[3];
	#endif
	
	csLastBinTicks = ticks;
	csDownInBin[csCurrentBin] = false;

    if (csBinSwitches < NUM_CAPSENSE_BINS)
        ++csBinSwitches;
}

inline void BumpCapSenseChannel(void)
{
	currentCapSenseChannel++;
	
	// Skip unused channels.
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
	
	if (currentCapSenseChannel > LAST_CAPSENSE_CHANNEL)
		currentCapSenseChannel = FIRST_CAPSENSE_CHANNEL;
		
	SetCapSenseChannel();

	// Start the next reading.
	RestartCapSenseTimer();
}

byte CapSenseISR(void)
{
	if (UiTimeInterrupt()) {
        // Timer 0 has rolled over recently.
		// Read TMR1: it's the number of times the oscillator has cycled since the last rollover.
		CapSenseReading reading = (tmr1h << 8) | tmr1l;
		
		// Do some of the indexing once.
		CapSenseReading* currentBaseline = &csBaseline[currentCapSenseChannel];
		CapSenseReading* currentReading = &csReadings[currentCapSenseChannel];
		
		// Compute the "pressed" threshold.
		// threshold = the threshold constant - sensitivity, but bracketed at the minimum threshold.
		CapSenseReading threshold = *currentBaseline;
		byte sensitivity = csThresholds[currentCapSenseChannel];
		if (threshold > sensitivity) {
			threshold -= sensitivity;
			threshold = max(threshold, CS_MIN_THRESHOLD);
		} else
			threshold = CS_MIN_THRESHOLD;
		
		// Filter the new value.
		// Exponential weighted moving average, over FILTER_LENGTH samples.
		reading = *currentReading + (reading - *currentReading) / FILTER_LENGTH;
		*currentReading = reading;

		// Is it a button press?
		if (reading < threshold) {
			// Yes, it's "down."
			if (csButton == NO_CAPSENSE_BUTTONS && csHoldingButton == NO_CAPSENSE_BUTTONS
				&& csPollsSinceDown > DEBOUNCE_POLLS  // debounce by number of polls - ~<= 1000/sec.
			) {
				// And this is the falling edge: note it.
				csButton = currentCapSenseChannel;
				csLastButtonTicks = ticks;
				csHoldingButton = currentCapSenseChannel;
				csPollsSinceDown = 0;
				csDownInBin[csCurrentBin] = true;
			}
		} else {
			// No, it's not "down".
			// If this is the button we were holding, we're not holding it anymore.
			if (csHoldingButton == currentCapSenseChannel)
				csHoldingButton = NO_CAPSENSE_BUTTONS;
				
			if (csHoldingButton == NO_CAPSENSE_BUTTONS && csPollsSinceDown < 255)
				++csPollsSinceDown;
		}
	
		// Update the current bin's maximum.
		//accumulateMax<CapSenseReading>(&csBinMax[currentCapSenseChannel][csCurrentBin], reading);
		// That works, but the resulting function call uses one too many stack levels.
        CapSenseReading* currentMax = &csBinMax[currentCapSenseChannel][csCurrentBin];
		if (reading > *currentMax)
            // But use an exponential moving average instead of accumulating directly.
			*currentMax = *currentMax + (reading - *currentMax) / FILTER_LENGTH;
            // *currentMax = reading;

#ifdef CS_AUTO_CALIBRATE
		// During calibration, keep track of the minima as well.
		if (csAutoCalibrateState == acPressAndReleaseButton)
			accumulateMin<CapSenseReading>(&csMin[currentCapSenseChannel], reading);
#endif		
		
		// Move to the next min bin, every once in a while.
        // This allows us to adapt to changes in baseline capacitance (because it's gotten colder or wetter, e.g.)
        // without adjusting too soon simply because the user has held a button down for a while.
		if (ticks - csLastBinTicks >= TICKS_PER_BIN_CHANGE
            || (csBinSwitches < NUM_CAPSENSE_BINS && ticks - csLastBinTicks >= TICKS_PER_BIN_CHANGE_INITIAL)
            )
			BumpCapSenseBin();
	
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


//==================================================================
// Calibration

#ifdef CS_AUTO_CALIBRATE

#define SETTLE_TICKS  (TICKS_PER_BIN_CHANGE * NUM_CAPSENSE_BINS + 1)  // Allow time for all bins to be overwritten.
byte ticksStateStart;

#define TIMES_THRU_BUTTONS  3
byte timesThruButtons;

#ifdef CS_AUTO_CALIBRATE
// The maximum depression from the baseline seen while the button is considered to be released and pressed, respectively.
// Positive values represent negative deviations from the baseline.
CapSenseReading csMaxWaiting[MAX_CAPSENSE_CHANNELS];
CapSenseReading csMaxHolding[MAX_CAPSENSE_CHANNELS][TIMES_THRU_BUTTONS];

// Ditto, but while another button is being pressed.
CapSenseReading csMaxOthers[MAX_CAPSENSE_CHANNELS];
#endif


CapSenseReading csMinOtherButtons[MAX_CAPSENSE_CHANNELS];  // min of each button while other buttons are being pressed

// Sets all elements in the array of length count to value.
void InitReadingArray(CapSenseReading* array, byte count, CapSenseReading value)
{
	while (count--)
		*array++ = value;
}

inline void EnterState(CSAutoCalibrateState newState)
{
	csAutoCalibrateState = newState;
	ticksStateStart = ticks;
}

void CapSenseStartCalibrate(void)
{
	EnterState(acStart);
}

byte CapSenseContinueCalibrate(void)
{
	byte channel;
	
	// Call this to ensure buttons are taken out of the one-element "queue",
	// simulating normal processing.
	GetCapSenseButton();
	
	// State actions and transitions.
	
	switch (csAutoCalibrateState) {
	
	case acStart:
		csCalButton = FIRST_CAPSENSE_CHANNEL;
		timesThruButtons = 0;
		InitReadingArray(csMaxWaiting, MAX_CAPSENSE_CHANNELS, 0);
		InitReadingArray(csMaxHolding, MAX_CAPSENSE_CHANNELS * TIMES_THRU_BUTTONS, 0);
		InitReadingArray(csMaxOthers, MAX_CAPSENSE_CHANNELS, 0);
		EnterState(acPressNothing);
		break;
		
	case acPressNothing:
		if (ticks - ticksStateStart > SETTLE_TICKS) {
			// Done waiting.
			// Move mins into csMinWaiting.
			for (channel = FIRST_CAPSENSE_CHANNEL; channel <= LAST_CAPSENSE_CHANNEL; ++channel)
				accumulateMax<CapSenseReading>(&csMaxWaiting[channel], csBaseline[channel] - csMin[channel]);
			
			// Move to the first button, if this is the first "nothing down" period.
			if (timesThruButtons == 0) {
				EnterState(acPressAndReleaseButton);
			} else {
				// Otherwise, we're done.
				EnterState(acDone);
			}
		}
		break;
		
	case acPressAndReleaseButton:
		if (ticks - ticksStateStart > SETTLE_TICKS) {
			// Done waiting.
			// Move the from csMin into csMinHolding and csMinOthers. 
			for (channel = FIRST_CAPSENSE_CHANNEL; channel <= LAST_CAPSENSE_CHANNEL; ++channel) {
				CapSenseReading* accumulator;
				if (channel == csCalButton)
					// Accumulate the min for the current button into the "min while holding" accumulator.
					accumulator = &csMaxHolding[channel][timesThruButtons];
				else
					// Accumulate the min for other channels while this button is being pressed, over all trials.
					accumulator = &csMaxOthers[channel];
	
				// Get it from the min bin, which already contains a min of many recent readings.
				accumulateMax<CapSenseReading>(accumulator, csBaseline[channel] - csMin[channel]);
			}
		
			// Move to the next button.
			EnterState(acPressAndReleaseButton);
			++csCalButton;
			
			// Skip over inactive channels.
			while (!IsChannelUsed(csCalButton) && csCalButton <= LAST_CAPSENSE_CHANNEL)
				++csCalButton;

			// Reset the csMin for this button and pass.
			InitReadingArray(csMin, MAX_CAPSENSE_CHANNELS, MAX_CS_READING);

			// Start a new round, if we've been through all channels.
			if (csCalButton > LAST_CAPSENSE_CHANNEL) {
				csCalButton = FIRST_CAPSENSE_CHANNEL;
				++timesThruButtons;
				
				if (timesThruButtons >= TIMES_THRU_BUTTONS)
					EnterState(acPressNothing);
			} 
		}
		break;
	}
	
	if (csAutoCalibrateState == acDone) {
		// csMaxWaiting, csMaxHolding, and csMaxOthers have been collected.
		// Finish computing the target thresholds and rate the results.
		for (csCalButton = FIRST_CAPSENSE_CHANNEL; csCalButton <= LAST_CAPSENSE_CHANNEL; ++csCalButton) {
			if (IsChannelUsed(csCalButton)) {
				// Find the maximum excursions (lowest deviation from baseline) for this channel.
				CapSenseReading maxWaiting = csMaxWaiting[csCalButton];
				CapSenseReading maxOthers = csMaxOthers[csCalButton];
							
				// Find the minimum and maximum readings during *this* button's presses.
				CapSenseReading minMe = MAX_CS_READING;  // the smallest maximum excursion for the weakest press
				CapSenseReading maxMe = 0;  // the largest excursion for the strongest press
				byte i;
				for (i = 0; i < TIMES_THRU_BUTTONS; ++i) {
					minMe = min(minMe, csMaxHolding[csCalButton][i]);
					maxMe = max(maxMe, csMaxHolding[csCalButton][i]);
				}
				
				// Report a Fail status if we can't tell the difference between a neighboring button and our own,
				if (maxOthers + 2 * CS_MIN_THRESHOLD >= minMe 
					// or between our lightest press and our noisiest non-press.
					|| maxWaiting + 2 * CS_MIN_THRESHOLD >= minMe)
				{ 
					csResults[csCalButton] = acrFail;
					
					// But, still calculate a threshold based on telling this button's hardest press apart from its release.
					// The buttons may fight, but at least we can tell when one of them is hit.
					csThresholds[csCalButton] = (maxMe / 2) + (maxWaiting / 2) - CS_MIN_THRESHOLD;
				} else {
					// Otherwise, report the excursion distance from the steady-state "waiting" reading
					// that will recognize the weakest button press, with a slight margin to ensure it's read as a press.
					csThresholds[csCalButton] = minMe - CS_MIN_THRESHOLD;
				
					// If the weakest button press isn't distinguishable from noise or other buttons, note that
					// this button is just "OK" - you have to press it hard.
					if (maxOthers > minMe
						|| maxOthers > minMe - 2 * CS_MIN_THRESHOLD
						|| maxWaiting > minMe
						|| maxWaiting > minMe - CS_MIN_THRESHOLD)
					{
						csResults[csCalButton] = acrOK;
					} else
						csResults[csCalButton] = acrGreat;
				}
			} else {
				// Blank out the results for unused buttons, just for completeness.
				csResults[csCalButton] = acrFail;
				csThresholds[csCalButton] = 0;
			}
		}
	
		// Copy results back to EEPROM.
		write_eeprom_block(CAPSENSE_EEPROM_ADDR, (char*) csThresholds, CAPSENSE_EEPROM_LEN);
		
		// Copy intermediate results to the start of EEPROM, so they can be conveniently read out.
		// Overwrites whatever's there (ASCII table for the display at the moment).
		#if 0
		write_eeprom_block(0, (char*) csMaxWaiting, sizeof(csMaxWaiting));
		write_eeprom_block(sizeof(csMaxWaiting), (char*) csMaxOthers, sizeof(csMaxOthers));
		write_eeprom_block(sizeof(csMaxWaiting) + sizeof(csMaxOthers), (char*) csMaxHolding, sizeof(csMaxHolding));
		#endif
	
		// Signal when done.
		return false;
	} else
		return true;
}

#endif
// CS_AUTO_CALIBRATE
