/* Sound.c
    Copyright (c) 2007-2012 by Timothy J. Weber, tw@timothyweber.org.

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

#include <system.h>

#include "Sound.h"
#include "SoundConsts.h"

#include "shadowRegs.h"
#include "types-tjw.h"

byte soundTimeoutHigh;
byte soundTimeoutLow;
unsigned short remainingDuration = 0;  // of the current sound, in milliseconds
unsigned short followingSilence = 0;  // duration of silence to follow the current sound, or 0 if none

const char* currentSong = 0;
byte millisElapsed = 0;

inline void TurnSoundOff(void)
{
	t1con = 0;  // turn Timer 1 off.
	pie1.TMR1IE = 0;
	pir1.TMR1IF = 0;
	tmr1h = 0;
	tmr1l = 0;
	SET_SHADOW_BIT(SOUND_PORT, SOUND_SHADOW, SOUND_PIN, 0);
	remainingDuration = 0;
	followingSilence = 0;
	currentSong = 0;
	millisElapsed = 0;
}

void StartSound(unsigned short periodUs)
{
	// Prepare the port.
	SOUND_TRIS.SOUND_PIN = 0;
  
	if (periodUs) {
		// Set up the timer
		t1con = 0x01;  // prescale 1:1 on 16-bit counter @ 1 MHz.
		intcon.PEIE = 1;
		pie1.TMR1IE = 1;
		
		// Think in terms of half-period, not periods.
		periodUs >>= 1;
		periodUs = 0xFFFF - periodUs;
		
		soundTimeoutHigh = periodUs >> 8;
		soundTimeoutLow = periodUs & 0xFF;
	} else
		TurnSoundOff();
}
// Duplicate version, for calling from the interrupt!
inline void StartSoundInternal(unsigned short periodUs)
{
	// Prepare the port.
	SOUND_TRIS.SOUND_PIN = 0;
  
	if (periodUs) {
		// Set up the timer
		t1con = 0x01;  // prescale 1:1 on 16-bit counter @ 1 MHz.
		intcon.PEIE = 1;
		pie1.TMR1IE = 1;
		
		// Think in terms of half-period, not periods.
		periodUs >>= 1;
		periodUs = 0xFFFF - periodUs;
		
		soundTimeoutHigh = periodUs >> 8;
		soundTimeoutLow = periodUs & 0xFF;
	} else
		TurnSoundOff();
}

void PlaySound(unsigned short periodUs, unsigned short durationMs)
{
	StartSound(periodUs);
	remainingDuration = durationMs;
	followingSilence = 0;
}
// Duplicate version, for calling from the interrupt!
inline void PlaySoundInternal(unsigned short periodUs, unsigned short durationMs)
{
	StartSoundInternal(periodUs);
	remainingDuration = durationMs;
	followingSilence = 0;
}

void PlayClick(void)
{
	PlaySound(1000, 2);
}

#define NUM_NOTES  24
unsigned short notePeriods[NUM_NOTES] = {
	3823,
	3608,
	3405,
	3214,
	3034,
	2864,
	2703,
	2551,
	2408,
	2273,
	2241,
	2025,
	1911,
	1804,
	1703,
	1607,
	1517,
	1432,
	1351,
	1276,
	1204,
	1136,
	1073,
	1066
};

// Given one of the note length codes,
// return the total time alloted to that note length,
// at a standard tempo (150 bpm), in milliseconds.
inline unsigned short TimeFor(char noteLengthCode)
{
	switch (noteLengthCode) {
	case '1':
		return 1600;
	case '2':
		return 800;
	case '4':
		return 400;
	case '8':
		return 200;
	case '6':
		return 100;
	case '3':

  return 50;
	default:
		return 0;
	}
}

// Starts playing the next note of the current song, if the current song is nonzero.
// Bump the current song forward to the next note.
// If we reach the end of the song, stop making sound and set the current song to zero.
inline void PlayNextNote(void)
{
	if (currentSong) {
		if (*currentSong) {
			unsigned short noteTimeMs = TimeFor('8');
			unsigned short noteDurationMs = noteTimeMs * 4 / 5;
			
			// This is a modifier added to the next note.
			char note = 0;
			byte playNote = false;
			
			// Step through modifiers until we have enough information to play the next note.
			for (; *currentSong && !playNote; ++currentSong) {
				switch (*currentSong) {
				case '1':
				case '2':
				case '3':
				case '4':
				case '8':
				case '6':
					noteTimeMs = TimeFor(*currentSong);
					noteDurationMs = noteTimeMs * 4 / 5;
					break;
				
				case '+':
					++note;
					break;
				
				case '-':
					--note;
					break;
				
				case 'c':
					playNote = true;
					break;
				case 'd':
					note += 2;
					playNote = true;
					break;
				case 'e':
					note += 4;
					playNote = true;
					break;
				case 'f':
					note += 5;
					playNote = true;
					break;
				case 'g':
					note += 7;
					playNote = true;
					break;
				case 'a':
					note += 9;
					playNote = true;
					break;
				case 'b':
					note += 11;
					playNote = true;
					break;
				case 'C':
					note += 12;
					playNote = true;
					break;
				case 'D':
					note += 14;
					playNote = true;
					break;
				case 'E':
					note += 16;
					playNote = true;
					break;
				case 'F':
					note += 17;
					playNote = true;
					break;
				case 'G':
					note += 19;
					playNote = true;
					break;
				case 'A':
					note += 21;
					playNote = true;
					break;
				case 'B':
					note += 23;
					playNote = true;
					break;
				
				case ' ':
					note = NUM_NOTES;
					playNote = true;
					break;
				}
		  
				if (playNote) {
					unsigned short notePeriod;
					if (note < NUM_NOTES)
						notePeriod = notePeriods[note];
					else
						notePeriod = 0;
			  
					PlaySoundInternal(notePeriod, noteDurationMs);
					followingSilence = noteTimeMs - noteDurationMs;
				}
			}
		} else {
			// Done playing this song; forget about it.
			TurnSoundOff();
		}
	} else {
		// There is no song.  Ensure we're not playing anything.
		TurnSoundOff();
	}
}

void PlaySong(const char* song)
{
	currentSong = song;
}

void UpdateSoundMs(void)
{
	++millisElapsed;
}

void UpdateSong(void)
{
	if (millisElapsed) {
		// If we've just started a song, start the first note playing.
		if (currentSong && (remainingDuration == 0)) {
			millisElapsed = 0;
			PlayNextNote();
		// Are we playing something now?
		} else if (remainingDuration > 0) {
			// Yes. Let's see if it's over yet.
			if (remainingDuration > millisElapsed)
				remainingDuration -= millisElapsed;
			else
				remainingDuration = 0;
			millisElapsed = 0;
				
			if (remainingDuration == 0)
				// We just  finished playing a note or silence. What comes next?
				if (followingSilence)
					// The silence to make that note staccato (or just not legato).
					PlaySoundInternal(0, followingSilence);  // this clears followingSilence automatically
				else
					// Look for the next note in the song, if there is any.
					PlayNextNote();
		}
	}
}

void SoundInterrupt(void)
{
	if (pir1.TMR1IF) {
		// Clear the interrupt.
		pir1.TMR1IF = 0;
    
		// Toggle the sound pin.
		#ifdef SOUND_LATCH
		SOUND_LATCH ^= SOUND_MASK;
		#else
		TOGGLE_SHADOW_BIT(SOUND_PORT, SOUND_SHADOW, SOUND_PIN);
		#endif
    
		// Reload the timer period.
		tmr1l = 0;  // so it doesn't roll over just after we set the high byte.
		tmr1h = soundTimeoutHigh;
		tmr1l = soundTimeoutLow;
	}
}
