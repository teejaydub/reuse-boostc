/* dayTime.c
    Copyright (c) 2007, 2017 by Timothy J. Weber, tw@timothyweber.org.
*/

#define IN_DAYTIME

#include <system.h>

#include "dayTime.h"
#include "uiSeconds.h"


// This is the number of seconds we count per minute if we're using
// 1024 microseconds per millisecond and 256 ms * 4 ticks = 1 second.
#define SECONDS_PER_MINUTE_ADJUSTED  57

// Every so many minutes, wait an additional second.
// This was 12 based on the spreadsheet.
#define ADJUST_MINUTES  4
byte minutesSinceAdjusted = 0;


void SetDayTime(byte hours, byte minutes)
{
	seconds = 0;
	currentDayTime = MakeDayTime(hours, minutes);
}

byte UpdateDayTime(void)
{
	byte result = false;
	
	while (seconds >= SECONDS_PER_MINUTE) {
		intcon.GIE = 0;
		seconds -= SECONDS_PER_MINUTE;
		intcon.GIE = 1;
		
		// Add a minute.
		++currentDayTime;
		
		if (currentDayTime >= MINUTES_PER_DAY) {
			// We'll assume less than a minute has passed since we noticed.
			currentDayTime = 0;
			result = true;
		}
	}
	
	return result;
}

byte UpdateDayTimeAdjusted(void)
{
	byte result = false;
	
	while (seconds >= SECONDS_PER_MINUTE_ADJUSTED) {
		intcon.GIE = 0;
		seconds -= SECONDS_PER_MINUTE_ADJUSTED;
		intcon.GIE = 1;
		
		// Add a minute.
		++currentDayTime;
		++minutesSinceAdjusted;
				
		if (currentDayTime >= MINUTES_PER_DAY) {
			// We'll assume less than a minute has passed since we noticed.
			currentDayTime = 0;
			result = true;
		}
	}
	
	if (minutesSinceAdjusted >= ADJUST_MINUTES && seconds > 0) {
		// Several times an hour, delay one second.
		intcon.GIE = 0;
		--seconds;
		intcon.GIE = 1;
		minutesSinceAdjusted = 0;
	}
	
	return result;
}

void DecodeDayTime(dayTime_t time, byte& hours, byte& minutes)
{
	hours = time / MINUTES_PER_HOUR;
	minutes = time % MINUTES_PER_HOUR;
}
