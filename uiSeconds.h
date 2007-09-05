// uiSeconds.h
//
// Simple timekeeping for user interface purposes.
// Resolution is from one second to ; accuracy is low.
// Requires uiTime.

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
void UpdateUiSeconds(void);


#endif