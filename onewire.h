/*
	Software support for Dallas 1-Wire protocol, as master.
	
	To use, define constants in onewire-const.h.
	
	And, be sure to activate either internal or external pullup on the pin.
*/

#include "types-tjw.h"
	
// Defines for 1-Wire commands

#define OW_SearchROM  0x0F0
#define OW_ReadROM  0x33
#define OW_MatchROM  0x55
#define OW_SkipROM  0xCC


// Resets the bus.
// Returns true if there's a presence on the bus.
char OW_Reset();

// Sends a byte to the bus.
void OW_SendByte(unsigned char b);

// Reads a byte from the bus.
unsigned char OW_ReadByte();

// Reads a single bit from the bus.
// Returns nonzero if it's one, zero if it's zero.
byte OW_ReadBit();

// Drives power to the bus until the next operation.
void OW_PowerOn();
