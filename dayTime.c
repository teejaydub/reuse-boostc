// dayTime.c

#define IN_DAYTIME

#include <system.h>

#include "dayTime.h"
#include "uiSeconds.h"


void SetDayTime(byte hours, byte minutes)
{
	seconds = 0;
	currentTime = MakeDayTime(hours, minutes);
}

byte UpdateDayTime(void)
{
	byte result = false;
	
	while (seconds >= SECONDS_PER_MINUTE) {
		intcon.GIE = 0;
		seconds -= 60;
		intcon.GIE = 1;
		
		++currentTime;
		
		if (currentTime >= MINUTES_PER_DAY) {
			// We'll assume less than a minute has passed since we noticed.
			currentTime = 0;
			result = true;
		}
	}
	
	return result;
}

void DecodeDayTime(dayTime_t time, byte& hours, byte& minutes)
{
	hours = time / MINUTES_PER_HOUR;
	minutes = time % MINUTES_PER_HOUR;
}
