/* BlockingSound.h

	Sound generation through an arbitrary port pin, using no timers.
	
	Blocks on each call, and turns off interrupts,
	so only useful when there's no multitasking going on.
	
	Requires BlockingSoundConsts.h defined.
*/

#ifndef __BLOCKING_SOUND_H
#define __BLOCKING_SOUND_H

// Plays a square wave with the given period (in microseconds) 
// for the given duration (in milliseconds).
// Duration can range up to 32767 ms.
void PlaySound(unsigned short periodUs, unsigned short durationMs);

// Makes a short click.
void PlayClick(void);

#endif
