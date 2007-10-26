// uiSeconds.c

#define IN_UISECONDS

#include <system.h>

#include "types-tjw.h"

#include "uiSeconds.h"



// Number of ticks since we last updated.  Don't modify outside this module.
unsigned char secondTicks = 0;

// Extra ticks that we haven't yet factored in.
unsigned char spareSecondTicks = 0;


void ClearUiSeconds(void)
{
	seconds = 0;
	secondTicks = ticks;
	spareSecondTicks = 0;
}

void UpdateUiSeconds(void)
{
	#if 0
	unsigned startTicks = ticks;
	unsigned char elapsedSecs = startTicks - secondTicks;
	
	if (elapsedSecs) {
		elapsedSecs += spareSecondTicks;
		spareSecondTicks = elapsedSecs & (TICKS_PER_SEC - 1);
		elapsedSecs = elapsedSecs >> LOG2_TICKS_PER_SEC;
		
		seconds += elapsedSecs;
	
		secondTicks = startTicks;
	}
	#else
	
	byte startTicks = ticks;
	byte elapsedTicks = startTicks - secondTicks;
	while (elapsedTicks >= TICKS_PER_SEC) {
		++seconds;
		elapsedTicks -= TICKS_PER_SEC;
		secondTicks += TICKS_PER_SEC;
	}
	#endif
}
