// uiTime.c

#define IN_UITIME

#include <system.h>

#include "uiTime.h"


// Rolls over every "tick".
// A tick is 0.262 seconds; there are about 3.8 per second.
// Driven by Timer 0.
unsigned char tickScaler;


void InitUiTime_Timer0(void)
{
	option_reg = 0x01;  // 1:4 prescaler on Timer 0: rolls over with a period of 1.024 ms.
	intcon.T0IE = 1;
	tickScaler = 0;
	ticks = 0;
}

unsigned char UiTimeInterrupt(void)
{
	// Timer 0, rolling over with a 1.024 ms period.
	if (intcon.T0IF) {
		// Clear the interrupt.
		intcon.T0IF = 0;

		if (++tickScaler == 0) {
			ticks++;
			return true;
		}
	}
	
	return false;
}
