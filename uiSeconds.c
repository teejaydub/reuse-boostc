/* uiSeconds.c
    Copyright (c) 2007, 2017 by Timothy J. Weber, tw@timothyweber.org.
*/

#define IN_UISECONDS

#include <system.h>

#include "types-tjw.h"

#include "uiSeconds.h"



// Tick value when we last updated.
static byte secondTicks = 0;

void ClearUiSeconds(void)
{
	seconds = 0;
	secondTicks = ticks;
}

byte UpdateUiSeconds(void)
{
	byte result = false;
	
	byte elapsedTicks = ticks - secondTicks;
	while (elapsedTicks >= TICKS_PER_SEC) {
		++seconds;

		elapsedTicks -= TICKS_PER_SEC;
		secondTicks += TICKS_PER_SEC;
		
		result = true;
	}
}

void UpdateUiSecondsTimer2(void)
{
	// Timer 2, rolling over with a 4,000 ms period.
	if (pir1.TMR2IF) {
		// Clear the interrupt.
		pir1.TMR2IF = 0;

		if (++secondTicks >= 250) {
			++seconds;
			secondTicks = 0;
		}
	} 
}
