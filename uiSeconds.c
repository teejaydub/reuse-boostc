// uiSeconds.c

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
