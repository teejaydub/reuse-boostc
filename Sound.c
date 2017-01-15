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

#include <string.h>

#include "Sound.h"
#include "SoundConsts.h"

#include "shadowRegs.h"
#include "types-tjw.h"

byte soundTimeoutHigh;
byte soundTimeoutLow;
unsigned short remainingDuration = 0;  // of the current sound, in milliseconds
unsigned short followingSilence = 0;  // duration of silence to follow the current sound, or 0 if none

char currentSong[MAX_SONG_LENGTH + 1] = {'\0'};
byte currentSongIndex = 0;
byte millisElapsed = 0;

// For interpreting song strings.
unsigned short noteTimeMs;
byte detachment;  // out of 5, where 5/5 = legato, and usually 4/5 = staccato.
byte swinging = false;
byte swingLong = false;
byte transpose = 0;


inline void TurnSoundOff(void)
{
	t1con = 0;  // turn Timer 1 off.
	pie1.TMR1IE = 0;
	pir1.TMR1IF = 0;
	tmr1h = 0;
	tmr1l = 0;
	SOUND_TRIS.SOUND_PIN = 1;
	SET_SHADOW_BIT(SOUND_PORT, SOUND_SHADOW, SOUND_PIN, 0);
}

void StartSound(unsigned short periodUs)
{
	// Prepare the port.
	SOUND_TRIS.SOUND_PIN = 0;
  
	if (periodUs) {
		// Set up the timer
		t1con = 0x01;  // prescale 1:1 on 16-bit counter @ 1 MHz.
		intcon.PEIE = 1;  // enable peripheral interrupts
		pie1.TMR1IE = 1;  // enable timer 1 interrupt
		
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

void PlayClick(void)
{
	SOUND_TRIS.SOUND_PIN = 0;
	SET_SHADOW_BIT(SOUND_PORT, SOUND_SHADOW, SOUND_PIN, 1);
	PlaySound(4000, 1);
}

#define NUM_NOTES  36
unsigned short notePeriods[NUM_NOTES] = {
    // Just temperament, asymmetric, five-limit
	7645,  // c
	7167,  // 
	6796,  // d
	6371,  // 
	6116,  // e
	5734,  // f
	5437,  // 
	5297,  // g (adjusted from 5097)
	4778,  // 
	4587,  // a
	4247,  // 
	4177,  // b (adjusted from 4077)
	3823,  // C
	3584,  // 
	3398,  // D
	3186,  // 
	3058,  // E
	2867,  // F
	2718,  // 
	2548,  // G
	2389,  // 
	2294,  // A
	2124,  // 
	2039,  // B
	1911,  // Transpose C
	1792,  // 
	1699,  // D
	1592,  // 
	1529,  // E
	1433,  // F
	1359,  // 
	1274,  // G
	1194,  // 
	1147,  // A
	1062,  // 
	1019,  // B
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
	char nextSongChar = currentSong[currentSongIndex];
	if (nextSongChar) {
		// This is the note number to play next.  It's modified by the letters later.
		char note = 0;
		byte playNote = false;
		byte isDotted = false;
		
		// Step through modifiers until we have enough information to play the next note.
		for (; nextSongChar && !playNote; nextSongChar = currentSong[++currentSongIndex]) {
			switch (nextSongChar) {
			case '1':
			case '2':
			case '3':
			case '4':
			case '8':
			case '6':
				noteTimeMs = TimeFor(nextSongChar);
				swinging = false;
				break;
				
			case '.':
				isDotted = true;
				break;
				
			case 'T':
				transpose = 12;
				break;
				
			case 't':
				transpose = 0;
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
			
			case '_':
				detachment = 5;
				break;
				
			case '^':
				detachment = 4;
				break;
			
			case '~':
				swinging = true;
				swingLong = true;
				break;
			}
	  
			if (playNote) {
				unsigned short notePeriod;
				if (note < NUM_NOTES)
					notePeriod = notePeriods[note + transpose];
				else
					notePeriod = 0;
		  
				// thisNoteTime is the time taken by the note plus its following silence.
				unsigned thisNoteTime;
				if (isDotted)
					thisNoteTime = noteTimeMs + (noteTimeMs >> 1);
				else
					thisNoteTime = noteTimeMs;
					
				if (swinging) {
					unsigned swingBy = thisNoteTime / 3;
					if (swingLong)
						thisNoteTime = thisNoteTime + swingBy;
					else
						thisNoteTime = thisNoteTime - swingBy;
					swingLong = !swingLong;
				}
					
				// noteDurationMs is the duration of the note's sound.
				unsigned short noteDurationMs = thisNoteTime;
				if (detachment == 4) {
					noteDurationMs *= 4;
					noteDurationMs /= 5;
				}
				PlaySound(notePeriod, noteDurationMs);
				followingSilence = thisNoteTime - noteDurationMs;
			}
		}
	} else {
		// Done playing this song; forget about it.
		TurnSoundOff();
		currentSong[0] = 0;
		currentSongIndex = 0;
		remainingDuration = 0;
	}
}

void PlaySong(const char* song)
{
	// Go back to defaults for interpreting the song string.
	noteTimeMs = TimeFor('8');
	detachment = 4;
	swinging = false;
	
	strncpy(currentSong, song, MAX_SONG_LENGTH);
	currentSongIndex = 0;
}

void TurnOffAllSound(void)
{
    currentSongIndex = 0;
    currentSong[0] = '\0';
    PlaySound(0, 0);
}

unsigned char IsSoundPlaying(void)
{
    return t1con != 0 || currentSong[currentSongIndex] != '\0';
}

void UpdateSoundMs(void)
{
	++millisElapsed;
}

void UpdateSong(void)
{
	if (millisElapsed) {
		// Are we playing something now?
		if (remainingDuration > 0) {
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
					PlaySound(0, followingSilence);  // this clears followingSilence automatically
				else
					// Look for the next note in the song, if there is any.
					PlayNextNote();
		}
		// Or if we've just started a song, start the first note playing.
		else if (currentSongIndex == 0 && currentSong[0] != '\0') {
			millisElapsed = 0;
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
