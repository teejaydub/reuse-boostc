/* math-tjw.h
    Copyright (c) 2010, 2017 by Timothy J. Weber, tw@timothyweber.org.

    General-purpose math routines.
*/

#ifndef __MATH_TJW_H
#define __MATH_TJW_H

#include <stdlib.h>

#include <types-tjw.h>


// Accumulate minimum and maximum values.
// Mainly useful when you have a complex expression as the accumulator,
// to save evaluating it twice.
template <class T>
void accumulateMin(T* accumulator, T newValue)
{
	*accumulator = min(*accumulator, newValue);
}

template <class T>
void accumulateMax(T* accumulator, T newValue)
{
	*accumulator = max(*accumulator, newValue);
}

// Average two unsigned values efficiently, without overflow.
template <class T>
T averageUnsigned(T a, T b)
{
	T result = a >> 1;
	result += b >> 1;
	
	if ((a & 1) != 0 || (b & 1) != 0)
		++result;
		
	return result;
}

// Return the value of the lowest bit set in x, and clear it in x.
// The highest bit that we will look at is highestBit.
// Returns 0 if no bit at or below highestBit is set.
template <class T>
inline
T clearLowestSetBit(T& x, byte highestBit)
{
	// Save time: check for zero first.
	if (x == 0)
		return 0;
		
	byte i;
	T mask = 1;
	for (i = 0; i <= highestBit; i++) {
		if (x & mask) {
			x ^= mask;
			return i;
		}
		mask <<= 1;
	}
	return 0;
}

#endif
// __MATH_TJW_H