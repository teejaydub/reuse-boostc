/* types-tjw.h
    Copyright (c) 2006, 2017 by Timothy J. Weber, tw@timothyweber.org.
*/

#ifndef _TJW_TYPES
#define _TJW_TYPES

// Unsigned chars are used all the time -- cut down on typing.
typedef unsigned char byte;

// Defines for normal and active-low logic.
#define TRUE_ACTIVE_LOW  0
#define FALSE_ACTIVE_LOW  1

// Returns a bitmask for the specified bit number.
#define BITMASK(bitNum)  (1 << bitNum)

// Sets the specified bit in the specified register based on the given expression.
inline void setBit(char& reg, byte bitNum, byte testValue)
{
	if (testValue) 
		reg |= BITMASK(bitNum); 
	else 
		reg &= (~BITMASK(bitNum)); 
}

// Toggles the specified bit in the specified register.
#define toggleBit(reg, bitNum)  (reg ^= BITMASK(bitNum))

// Swaps the bytes of short 's', and puts them in dest.
#define SWAPBYTES(dest, s)  dest = ((s & 0xFF) << 8) | ((s & 0xFF00) >> 8);

// Return the low (first) and high (second) byte of a numeric constant.
#define LOWBYTE(x)  (x & 0xFF)
#define HIGHBYTE(x)  ((x & 0xFF00) >> 8)

#endif
