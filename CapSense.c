/* CapSense.c
	Using capacitive touch sensing to simulate up to 4 pushbuttons.
	Follows the guidelines set out in Microchip's AN1101 and AN1103.
*/

#define IN_CAPSENSE

#include <system.h>

#include "CapSense.h"
#include "CapSense-consts.h"

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
	#elif CAPSENSE_CHANNELS & CAPSENSE_CHANNEL2
	currentCapSenseChannel = 1;
	#elif CAPSENSE_CHANNELS & CAPSENSE_CHANNEL3
	currentCapSenseChannel = 2;
	#elif CAPSENSE_CHANNELS & CAPSENSE_CHANNEL4
	currentCapSenseChannel = 3;
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
	intcon.T0IE = 1;
	
currentCapSenseChannel = 1;
	SetCapSenseChannel();
}

void StartCapSense(void)
{
	RestartCapSenseTimer();
}

