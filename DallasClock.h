// Driver for DS1307 real-time clock communication.

#ifndef _DALLAS_CLOCK
#define _DALLAS_CLOCK

#include "types-tjw.h"

// Call this once, before other functions.
// Assumes complete control over the I2C bus.
void InitDallasClock(void);

// Returns the current value of the seconds register, as a number from 0-59.
byte GetClockSeconds(void);

// Reads 8 bytes into buffer.
void GetClockMemory(byte* buffer);

#endif