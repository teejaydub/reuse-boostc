/* dayTime.h
	Copyright (c) 2007 by Timothy J. Weber.
	
	Stores and operates on the time of day.
	Stored in an unsigned short as minutes since midnight.
	
	One master time value is presented for use as the current time.
	You can also convert values to time types, and do math on them.
	
	Gets time from uiSeconds and uiTime.  Takes over uiSeconds's
	global 'seconds' for use as the current seconds.
*/

#ifndef __DAYTIME_H
#define __DAYTIME_H


#include "types-tjw.h"


#ifdef IN_DAYTIME
 #define DAYTIME_EXTERN
#else
 #define DAYTIME_EXTERN  extern
#endif


typedef unsigned short  dayTime_t;


// Don't touch this outside of this module - it's here just for access by the inline functions.
DAYTIME_EXTERN unsigned short currentTime;


inline dayTime_t MakeDayTime(byte hours, byte minutes)
{
	return (unsigned short)(hours) * 60 + (unsigned short) minutes;
}

// Sets the current time, in a 24-hour clock.
void SetDayTime(byte hours, byte minutes);

// This must be called frequently to update the time - at least once per minute.
// It returns true if we've just crossed to a new day.
byte UpdateDayTime(void);




#endif