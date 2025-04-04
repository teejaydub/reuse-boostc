/* dayTime.h
    Copyright (c) 2007, 2017 by Timothy J. Weber, tw@timothyweber.org.

	Stores and operates on the time of day.
	Stored in an unsigned short as minutes since midnight.
	
	One master time value is presented for use as the current time.
	You can also convert values to time types, and do math on them.
	
	Gets time from uiSeconds and uiTime.  Takes over uiSeconds's
	global 'seconds' for use as the current seconds, 0-59.
*/

#ifndef __DAYTIME_H
#define __DAYTIME_H


#include "types-tjw.h"


#define SECONDS_PER_MINUTE  60
#define MINUTES_PER_HOUR  60
#define HOURS_PER_DAY  24
#define MINUTES_PER_DAY  (MINUTES_PER_HOUR * HOURS_PER_DAY)


#ifdef IN_DAYTIME
 #define DAYTIME_EXTERN
#else
 #define DAYTIME_EXTERN  extern
#endif


typedef unsigned short  dayTime_t;


// This value is guaranteed never to be used for a valid dayTime.
#define INVALID_DAYTIME  0x05DC  // AKA 25 hours, 0 minutes.


// Don't change this outside of this module.
// Format is minutes since midnight.
DAYTIME_EXTERN dayTime_t currentDayTime;


inline dayTime_t MakeDayTime(byte hours, byte minutes)
{
	return (unsigned short)(hours) * MINUTES_PER_HOUR + (unsigned short) minutes;
}

#define MAKE_DAY_TIME(hours, minutes)  (hours * MINUTES_PER_HOUR + minutes)

// Returns the given time, divided up into hours and minutes and returned in the two specified variables.
void DecodeDayTime(dayTime_t time, byte& hours, byte& minutes);

// Sets the current time, in a 24-hour clock.
void SetDayTime(byte hours, byte minutes);

// This must be called frequently to update the time - at least once per minute.
// It returns true if we've just crossed to a new day.
byte UpdateDayTime(void);

// Or, use this one if you're updating seconds using the default Timer 0 implementation of uiTime,
// with 1.024/1.000 and 256/250 due to timers.
// (not yet reliable)
byte UpdateDayTimeAdjusted(void);

// Gets the current time, in a 24-hour clock, into the specified variables.
#define GetDayTime(hours, minutes)  DecodeDayTime(currentTime, hours, minutes)


#endif