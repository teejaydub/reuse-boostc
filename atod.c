/* atod.c
    Copyright (c) 2009-2012 by Timothy J. Weber, tw@timothyweber.org.

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

// Encapsulates setting up and reading an A-to-D channel.
// Requires types-tjw.

#include <system.h>
#include <stdlib.h>

#include "atod.h"


#if defined(_PIC16F1789) || defined(_PIC18F45K22)
 // In this new paradigm, you no longer refer to the A/D channel, you just refer to the port and pin.
 #define CHANNELS_BY_PORT
#elif defined(_PIC16F886) || defined(_PIC16F887)
 #define MORE_THAN_8_CHANNELS
#elif defined(_PIC16F688) || defined(_PIC16F916) || defined(_PIC12F683)
#else
 #error "Need to define how many channels this PIC has."
#endif

void SetADChannel(byte channel)
{
#if defined(_PIC18F45K22)
    ansela = 0;
    anselb = 0;
    anselc = 0;
    anseld = 0;
    ansele = 0;
	// Seemingly-random assignment of channels to port pins.
	if (channel <= 3) {
		ansela = BITMASK(channel);
	} else if (channel == 4) {
		ansela = BITMASK(5);
	} else if (channel >= 5 && channel <= 7) {
		ansele = BITMASK(channel - 5);
	} else if (channel == 8) {
		anselb = BITMASK(2);
	} else if (channel == 9) {
		anselb = BITMASK(3);
	} else if (channel == 10) {
		anselb = BITMASK(1);
	} else if (channel == 11) {
		anselb = BITMASK(4);
	} else if (channel == 12) {
		anselb = BITMASK(0);
	} else if (channel == 13) {
		anselb = BITMASK(5);
	} else if (channel >= 14 && channel <= 19) {
		anselc = BITMASK(channel - 12);
	} else if (channel >= 20 && channel <= 27) {
		anseld = BITMASK(channel - 20);
	}
#elif defined(CHANNELS_BY_PORT)
	if (channel < 8) {
		ansela = BITMASK(channel);
		anselb = 0;
	} else {
		ansela = 0;
		anselb = BITMASK(channel - 8);
	}
#else
	#if MORE_THAN_8_CHANNELS
		if (channel > 7) {
			anselh = BITMASK(channel - 8);
		} else {
	#endif
	#ifdef _PIC12F683
			ansel = (ansel & 0x01110000) | BITMASK(channel);  // preserve the clock select bits, which are stored here
	#else
			ansel = BITMASK(channel);
	#endif
	#ifdef MORE_THAN_8_CHANNELS
		}
	#endif
#endif
	
	pir1.ADIF = 0;
	
#if defined(_PIC16F1789)
	adcon0 = (adcon0 & 0b10000011) | (channel << 2);
#elif defined(_PIC16F1789)
	adcon0 = (adcon0 & 0b10000000) | (channel << 2);
#else
	adcon0 = (adcon0 & 0b11000000) | (channel << 2);  // keep current justification and reference (or clock for 16F886), !ADON, !GO.
#endif
#ifdef _PIC16F886
	adcon1 = (adcon1 & 0b10000000);  // Ref = Vss to Vdd, preserve justification.
#endif
}

void TurnOffADChannel(byte channel)
{
#if defined(CHANNELS_BY_PORT)
	ansela = 0;
	anselb = 0;
	#ifdef ANSELC
	anselc = 0;
	#endif
	#ifdef ANSELD
	anseld = 0;
	#endif
	#ifdef ANSELE
	ansele = 0;
	#endif
#else
	#ifdef MORE_THAN_8_CHANNELS
		if (channel > 7) {
			anselh &= ~(BITMASK(channel - 8));
		} else {
	#endif
			ansel &= ~(BITMASK(channel));
	#ifdef MORE_THAN_8_CHANNELS
		}
	#endif
#endif
		
#ifdef _PIC16F1789
	adcon0 = (adcon0 & 0b10000000) | (channel << 2);
#else
	adcon0 = (adcon0 & 0b11000000) | (channel << 2);  // and right-justified, !ADON, !GO, keep the existing clock.
		// Shouldn't this clear all the address bits, not leave it selected?
#endif
}

// Blocks the required settling time, starts a conversion, and waits for it to finish.
// Assumes the A/D channel has already been set up and selected.
// The result will be in adresh/adresl.
void AcquireAndConvertAD(void)
{
	// Turn on the A/D.
	adcon0.ADON = 1;

	// Wait for settling and acquisition.
	delay_us(40);
	
	// Start conversion.
	adcon0.GO_DONE = 1;
	
	// Wait for conversion to finish.
	while (adcon0.GO_DONE)
		clear_wdt();
	
	// Turn off A/D.
	adcon0.ADON = 0;
}

#ifndef ATOD_INLINE
unsigned short GetADValueShort(byte channel)
{
#if 0
    return 0;
#else
	ReadADChannel(channel);
	
	unsigned short result;
	MAKESHORT(result, adresl, adresh);
	return result;
#endif
}
#endif
