/* longPress.h
    Copyright (c) 2014 by Timothy J. Weber, tw@timothyweber.org.

	Uses the uiTime and buttons modules to return "long press" events,
	as well as "short release" events.
*/

#ifndef __LONG_PRESS_H
#define __LONG_PRESS_H

#ifdef IN_LONG_PRESS
 #define LONG_PRESS_EXTERN
#else
 #define LONG_PRESS_EXTERN  extern
#endif


#include "buttons.h"
#include "uiTime.h"


// Call this to initialize.
// A long press is one that lasts at least the given number of UI ticks;
// others are short presses.
void InitLongPress(byte newTicksPerLongPress);

// Call this instead of CheckButtons(), usually about every ms.
void CheckButtonReleases(void);

// Returns the button code of a button that has just been released
// if it was held down for a short press.
// Clears the button event - it's only returned once.
// Undefined if multiple buttons have been held down.
// Returns NO_BTN if there was no short press.
byte GetShortRelease(void);

// Returns the button code of a button that has been held down for a long press.
// Clears the button event - it's only returned once, and then not again until all buttons have been released.
// Undefined if multiple buttons have been held down.
// Returns NO_BTN if there was no long press.
byte GetLongPress(void);

#endif