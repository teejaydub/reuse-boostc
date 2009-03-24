/* fixed16.h
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
/*
	Defines a type for 16-bit fixed point numbers with 8 integral and 8 fractional bits.

	Only bare-bones support so far - just what's needed and timely for CoolBot.
	
	Rules for using fixed-point variables:
	
	- Convert integers and fractions to fixed-point variables using the various 
		conversion routines.
	
	- You can add and subtract two fixed-point values directly.
	
	- You can multiply two fixed-point values together by right-shifting 8 bits.
		The shift can take place on ONE of the two values beforehand,
		or on the result afterwards.  This produces a fixed-point result.
		
	- Similarly, you can divide two fixed-point values and get a fixed result
		by left-shifting the dividend (top) 8 bits, or by left-shifting 
		the result 8 bits, or by right-shifting the divisor (bottom) 8 bits.
		
	- These shifts can also take place partly before and partly after,
		to preserve resolution when you know ahead of time the maximum magnitude
		of one of the operands.
		
	- You can multiply a fixed-point with a plain integer (8-bit or 16-bit)
		directly - it's like you've already done the right-shift.
		The result is fixed-point.
		
	- Similarly, you can divide a fixed-point by a plain integer directly
		to get a fixed-point result.
*/

#ifndef __FIXED16_H
#define __FIXED16_H

typedef signed short fixed16;

// Returns b converted to fixed-point, with no fractional part.
inline fixed16 fixedFromByte(char b)
{
	return ((fixed16) b) << 8;
}
// This version is more efficient.
#define FIXED_FROM_BYTE(b)  (b << 8)

// Returns b as a fixed-point fraction, with no integral part.
inline fixed16 fixedFracFromByte(char b)
{
	return (fixed16) b;
}
// This version is more efficient.
#define FIXED_FRAC_FROM_BYTE(b)  (b)

inline fixed16 makeFixed(char integral, unsigned char fractional)
{
	return (((fixed16) integral) << 8) | ((fixed16) fractional);
}
// This version is more efficient.
#define MAKE_FIXED(result, integral, fractional)  MAKESHORT(result, fractional, integral)
#define MAKE_FIXED_CONST(integral, fractional)  (integral * 256 + fractional)

// Returns the integral part of f in i.
inline void fixedIntegralTo(fixed16 f, signed char& i)
{
	HIBYTE(i, f);
}
// This version is more efficient.
#define FIXED_INTEGRAL_TO(f, i)  HIBYTE(i, f)

// Returns the fractional part of f times 256.
inline void fixedFracTo(fixed16 f, unsigned char& frac)
{
	LOBYTE(frac, f);
}
// This version is more efficient.
#define FIXED_FRAC_TO(f, frac)  LOBYTE(frac, f)

// Return f truncated to an integer.
inline signed char fixedIntegral(fixed16 f)
{
	return f >> 8;
}
// More efficient.
#define FIXED_INTEGRAL(f)  (f >> 8)

// Returns the tenths digit of the fractional part of f.
// I.e., if f = 1.2, returns 2.
inline unsigned char fixedTenths(fixed16 f)
{
	// Start out with 1/256ths.
	unsigned char tempFixed;
	fixedFracTo(f, tempFixed);
	
	// Convert to 1/16ths.
	tempFixed >>= 4;
	
	// Convert to 1/160ths.
	tempFixed *= 10;
	
	// Add 0.05 = 1/20 = 8/160 so we round to the nearest tenth.
	tempFixed += 8;
	
	// Convert to 1/10ths.
	tempFixed >>= 4;
	
	return tempFixed;
}

// Returns f truncated to an integer.
inline fixed16 fixedTrunc(fixed16 f)
{
	return f & 0xFF00;
}

// Returns true if f has a nonzero fractional part.
inline char fixedHasFrac(fixed16 f)
{
	return (f & 0x00FF) != 0;
}

// Return f rounded to the nearest integer.
inline signed char fixedRoundToByte(fixed16 f)
{
	return (signed char)((f + 0x0080) >> 8);
}

// Return 1/f.
inline fixed16 fixedReciprocal(fixed16 f)
{
	return ((1 << 8) / f) << 8;
}

#endif
