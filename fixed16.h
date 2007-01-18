/* fixed16.h

	Defines a type for 16-bit fixed point numbers with 8 integral and 8 fractional bits.

	Only bare-bones support so far - just what's needed and timely for CoolBot.
*/

#ifndef __FIXED16_H
#define __FIXED16_H

typedef short fixed16;

// Returns b converted to fixed-point, with no fractional part.
inline fixed16 fixedFromByte(char b)
{
	return ((fixed16) b) << 8;
}

inline fixed16 makeFixed(char integral, char fractional)
{
	return (((fixed16) integral) << 8) | ((fixed16) fractional);
}

// Returns the integral part of f.
inline char fixedIntegral(fixed16 f)
{
	return f >> 8;
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
