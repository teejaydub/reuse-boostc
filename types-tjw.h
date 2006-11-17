
#ifndef _TJW_TYPES
#define _TJW_TYPES

// Unsigned chars are used all the time -- cut down on typing.
typedef unsigned char byte;

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

#endif
