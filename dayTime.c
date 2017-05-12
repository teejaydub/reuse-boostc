/* dayTime.c
    Copyright (c) 2007 by Timothy J. Weber, tw@timothyweber.org.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
