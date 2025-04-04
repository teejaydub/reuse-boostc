/* uiSeconds.h
    Copyright (c) 2007, 2017 by Timothy J. Weber, tw@timothyweber.org.

    Simple timekeeping; accumulates uiTime's clock ticks into seconds.
    Range is from one second to 2^16 seconds (~18 hours); accuracy is dependent on uiTime.
    Requires uiTime.
    Or, call InitUiSeconds_Timer2() first, then UpdateUiSecondsTimer2() in the ISR,
    for more accurate timekeeping (precise to the accuracy of the instruction clock).
*/

#ifndef __UISECONDS_H
#define __UISECONDS_H

#ifdef IN_UISECONDS
 #define UISECONDS_EXTERN
#else
 #define UISECONDS_EXTERN  extern
#endif

#include "uiTime.h"


// Count of tick-based seconds since startup, or since the counter was last cleared.
// 
// Rolls over about every 18 hours.
//
// Either use the difference between this and a previously-stored value to avoid 
// rollover problems, or clear it and use it directly (but be sure it's only used
// to mean one thing in your app).
UISECONDS_EXTERN unsigned short seconds;


// Call this to initialize the module or to reset the counter.
void ClearUiSeconds(void);

// Call this periodically to update the count.
// Just needs to be called with as much resolution as is needed for timekeeping.
// And, must be called at least once a minute.
// Returns true if we've just rolled over to a new second.
byte UpdateUiSeconds(void);

// Alternate, more accurate form that uses Timer 2:
// call this once to initialize.
inline void InitUiSeconds_Timer2(void)
{
	// Set up Timer 2 for 1:16 prescaler, with a period of 250.
	pie1.TMR2IE = 1;
	pr2 = 249;
	t2con = 0x07;  // turn it on with 1:16 prescaler and 1:1 postscaler
	seconds = 0;
}

// Then call this in the ISR to keep the seconds updated.
void UpdateUiSecondsTimer2(void);

#endif