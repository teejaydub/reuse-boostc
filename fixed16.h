/* fixed16.h

	Defines a type for 16-bit fixed point numbers with 8 integral and 8 fractional bits.

	Only bare-bones support so far - just what's needed and timely for CoolBot.
*/

#ifndef __FIXED16_H
#define __FIXED16_H

typedef signed short fixed16;

// Returns b converted to fixed-point, with no fractional part.
inline fixed16 fixedFromByte(char b)
{
	return ((fixed16) b) << 8;
}

inline fixed16 makeFixed(char integral, char fractional)
{
	return (((fixed16) integral) << 8) | ((fixed16) fractional);
}

// Returns the integral part of f in i.
inline void fixedIntegralTo(fixed16 f, signed char& i)
{
	HIBYTE(i, f);
}

// Returns the fractional part of f times 256.
inline void fixedFracTo(fixed16 f, unsigned char& frac)
{
	LOBYTE(frac, f);
}

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
inline char fixedRoundToByte(fixed16 f)
{
	return (char)((f + 0x0080) >> 8);
}

#endif
