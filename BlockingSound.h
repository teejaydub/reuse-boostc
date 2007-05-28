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

// Plays a song written out in a string of note names.
// Plain letters signify diatonic (C major) notes over two octaves,
// lowercase for the lower octave (c-b) and uppercase for the upper octave (C-B).
// E.g., an ascending scale is "cdefgabC".
// The default duration is an eighth note at 120 bpm.
// Duration is changed for subsequent notes using '8', '4', '2', and '1'
// for eighth, quarter, half, and whole notes (4/4), or '6' for sixteenths or '3' for thirty-seconds.
// Accidentals can be prefixed as '+' for sharp and '-' for flat,
// and affect only the following note.
void PlaySong(const char* song);

#endif
