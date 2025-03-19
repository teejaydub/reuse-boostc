/* longPress.c
    Copyright (c) 2014 by Timothy J. Weber, tw@timothyweber.org.
*/

#include <system.h>

#define IN_LONG_PRESS
#include "longPress.h"

#include "math-tjw.h"


// Global for the last tick count when all buttons were up.
byte ticksAllButtonsUp;
byte ticksPerLongPress = TICKS_PER_SEC;

// Button bit masks.
byte lastDowns;
byte shortPresses;
byte longPresses;
byte waitingForRelease;  // after a long press

void InitLongPress(byte newTicksPerLongPress)
{
	ticksPerLongPress = newTicksPerLongPress;
	ticksAllButtonsUp = ticks;
	lastDowns = 0;
	shortPresses = 0;
	longPresses = 0;
	waitingForRelease = false;
}

void CheckButtonReleases(void)
{
	CheckButtons();
	
	if (IS_ANY_BTN_DOWN) {
		lastDowns |= buttonsPressed;
		
		// If we've been holding it down long enough, note that it's a long press.
		if (!waitingForRelease && ticks - ticksAllButtonsUp >= ticksPerLongPress) {
			longPresses |= lastDowns;
			waitingForRelease = true;
		}
	} else {
		// No buttons are down.  Was one just released?
		if (lastDowns) {
			// Yes.  Note the timing.
			if (ticks - ticksAllButtonsUp < ticksPerLongPress)
				shortPresses |= lastDowns;
				
			lastDowns = 0;
		}
		waitingForRelease = false;
		ticksAllButtonsUp = ticks;
	}
}

byte GetShortRelease(void)
{
	return clearLowestSetBit<byte>(shortPresses, LAST_BTN);
}

byte GetLongPress(void)
{
	return clearLowestSetBit<byte>(longPresses, LAST_BTN);
}