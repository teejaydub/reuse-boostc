// dayTime.c

#define IN_DAYTIME

#include <system.h>

#include "dayTime.h"
#include "uiSeconds.h"


#define SECONDS_PER_MINUTE  60
#define MINUTES_PER_HOUR  60
#define HOURS_PER_DAY  24
#define MINUTES_PER_DAY  (MINUTES_PER_HOUR * HOURS_PER_DAY)


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
		
		if (currentTime >= MINUTES_PER_DAY) {
			// We'll assume less than a minute has passed since we noticed.
			currentTime = 0;
			result = true;
		}
	}
	
	return result;
}