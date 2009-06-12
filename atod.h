/* atod.h
    Copyright (c) 2009 by Timothy J. Weber, tw@timothyweber.org.

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

// Encapsulates selecting and reading an A-to-D channel.
// Requires types-tjw.


#ifndef __ATOD_H
#define __ATOD_H

#include "fixed16.h"
#include "types-tjw.h"


// Initializes the conversion clock to the correct value for the given device frequency, passed as a constant.
#define ADC_CLOCK_1MHZ  0b00000000
#define ADC_CLOCK_4MHZ  0b01000000
#define ADC_CLOCK_8MHZ  0b10000000
#define ADC_CLOCK_RC  0b11000000
inline void InitADClock(byte adcClock)
{
	adcon0 |= (adcClock & 0b11000000);
}

// Select the given A/D channel.
// Also requires that the corresponding TRIS register bit be set.
void SetADChannel(byte channel);

// Waits the required settling time, starts a conversion, and waits for it to finish.
// Assumes the A/D channel has already been set up and selected.
// The result will be in adresh/adresl.
void AcquireAndConvertAD(void);

// Reads the given A/D channel.
inline void ReadADChannel(byte channel)
{
	SetADChannel(channel);
	AcquireAndConvertAD();
}

// Sets and acquires the given A/D channel and returns its most-significant 8 bits.
inline byte GetADValue8(byte channel)
{
	ReadADChannel(channel);
	return adresh;
}

// Same, for unsigned short.
inline unsigned short GetADValueShort(byte channel)
{
	ReadADChannel(channel);
	
	unsigned short result;
	MAKESHORT(result, adresl, adresh);
	return result;
}

// Same, for fixed16.
inline fixed16 GetADValueFixed(byte channel)
{
	ReadADChannel(channel);
	
	fixed16 result;
	MAKE_FIXED(result, adresh, adresl);
	return result;
}

#endif