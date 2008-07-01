// uiTime.h
//
// Simple, human-level timekeeping for user interface purposes.
// Resolution is milliseconds and seconds under a minute; 
// accuracy is dependent on the driving method.
//
// Can be driven off of Timer 0 or Timer 1 with most of the overhead handled here,
// or from a 60 Hz external signal.

#ifndef __UITIME_H
#define __UITIME_H

#ifdef IN_UITIME
 #define UITIME_EXTERN
#else
 #define UITIME_EXTERN  extern
#endif


// Count of ticks since startup.
// A "tick" happens roughly four times per second.
// Rolls over about every 64 - 67 seconds.
// Seems to be a convenient time unit for many UI-related short delays.
//
// Always use the difference between the current value of ticks and some previously-stored value;
// that way, you never have to worry about when it rolls over.
//
// Only change this value if it's only used to mean one thing in your app.
// If you only need one UI timer, you can save one byte of RAM that way (FWTW).
//
// ASSUMES A CLOCK RATE OF 4 MHz at the moment!
UITIME_EXTERN unsigned char ticks;

// Resets the timer to 0.
void ResetUITimer(void);

// Express your desired timeouts and delay factors in terms of this.
#define TICKS_PER_SEC  4
#define LOG2_TICKS_PER_SEC  2


//====================================================================
// Routines for using Timer 0 or 1 as the time source

// Initializes, and dedicates Timer 0 for use and maintenance by this module.
// Requires that GIE is enabled elsewhere, and that UiTimeInterrupt is called.
void InitUiTime_Timer0(void);

// Initializes, and dedicates Timer 1 for use and maintenance by this module.
// Requires that GIE is enabled elsewhere, and that UiTimeInterrupt1 is called.
void InitUiTime_Timer1(void);

// Call this in your interrupt handler if using Timer 0.
// Returns true about once a millisecond, which can be used for other tasks.
// E.g.:
//
//	void interrupt(void)
//	{
//		if (UiTimeInterrupt())
//			CheckButtons();
//	}
unsigned char UiTimeInterrupt(void);

// Call this in your interrupt handler if using Timer 1.
void UiTimeInterrupt1(void);


//====================================================================
// Routines for using an external 60 Hz time source

// Call this once to initialize.
void InitUiTime_60Hz(void);

// Call this at 60 Hz to update.
void UiTimeUpdate60(void);


#endif