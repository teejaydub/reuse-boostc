// BlockingSound.c

#include <system.h>

#include "BlockingSound.h"
#include "BlockingSoundConsts.h"
#include "types-tjw.h"

void PlaySound(unsigned short periodUs, unsigned short durationMs)
{
	// Prepare the port.
	SOUND_TRIS.SOUND_PIN = 0;
	
	// Think in terms of half-period, not periods.
	periodUs >>= 1;
	durationMs <<= 1;
	
	// Cut the period into two byte values we can pass to the standard delay functions.
	byte periodm = periodUs / 1000;
	byte period10u = (periodUs % 1000) / 10;
	
	intcon.GIE = 0;
	
	unsigned short usPlayed = 0;
	unsigned short msPlayed = 0;
	while (msPlayed < durationMs) {
		// Play for one period.
		
		// Toggle the sound pin.
		SOUND_LATCH ^= SOUND_MASK;
		
		// Delay.
		if (periodm)
			delay_ms(periodm);
		if (period10u)
			delay_10us(period10u);
		
		// Accumulate the total time.
		usPlayed += periodUs;
		
		if (usPlayed > 1000) {
			usPlayed -= 1000;
			++msPlayed;
		}
	}		
	
	intcon.GIE = 1;
}

void PlayClick(void)
{
	// Prepare the port.
	SOUND_TRIS.SOUND_PIN = 0;
	
	// Just toggle the pin state once.
	//SOUND_LATCH ^= SOUND_MASK;
	
	PlaySound(1000, 2);
}
