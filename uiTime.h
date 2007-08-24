// uiTime.h
//
// Simple, human-level timekeeping for user interface purposes.
// Resolution is milliseconds and seconds; accuracy is low.

#ifndef __UITIME_H
#define __UITIME_H

#ifdef IN_UITIME
 #define UITIME_EXTERN
#else
 #define UITIME_EXTERN  extern
#endif


// Count of ticks since startup.
// A "tick" happens roughly four times per second.
// Rolls over about every 67 seconds.
// Seems to be a convenient time unit for many UI-related short delays.
//
// Always use the difference between the current value of ticks and some previously-stored value;
// that way, you never have to worry about when it rolls over.
//
// Only change this value if you're sure it won't be used elsewhere in your app.
// If you only need one UI timer, you can save one byte of RAM that way (FWTW).
UITIME_EXTERN unsigned char ticks;

// Express your desired timeouts and delay factors in terms of this.
#define TICKS_PER_SEC  4


// Initializes, and dedicates Timer 0 for use and maintenance by this module.
// Requires that GIE is enabled elsewhere, and that UiTimeInterrupt is called.
void InitUiTime_Timer0(void);

// Call this in your interrupt handler.
// Returns true about once a millisecond, which can be used for other tasks.
unsigned char UiTimeInterrupt(void);


#endif