// uiSeconds.c

#define IN_UISECONDS

#include <system.h>

#include "uiSeconds.h"



// Number of ticks since we last updated.  Don't modify outside this module.
unsigned char secondTicks;

// Extra ticks that we haven't yet factored in.
unsigned char spareSecondTicks;


void ClearUiSeconds(void)
{
	seconds = 0;
	secondTicks = ticks;
	spareSecondTicks = 0;
}

void UpdateUiSeconds(void)
{
	unsigned char elapsedSecs = ticks - secondTicks;
	
	if (elapsedSecs) {
		elapsedSecs += spareSecondTicks;
		spareSecondTicks = elapsedSecs & (TICKS_PER_SEC - 1);
		elapsedSecs = elapsedSecs >> LOG2_TICKS_PER_SEC;
		
		seconds += elapsedSecs;
	
		secondTicks = ticks;
	}
}
