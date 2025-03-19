/* shadowRegs18.h
    Copyright (c) 2006-2011 by Timothy J. Weber, tw@timothyweber.org.

    Like shadowRegs.h, but for use on the PIC18F architecture,
    which has LAT registers and so doesn't need shadow registers.
    Can be used to keep code portable across both the PIC16F and PIC18F environments.
    (It does suffer a bit in readability, though - only use it if that portability is required.)
*/

#ifndef _TJW_SHADOWREGS18_H
#define _TJW_SHADOWREGS18_H

#include "types-tjw.h"

// Sets the specified latch register, changing only the masked bits.
#define SET_SHADOW(regName, latchReg, newValue, mask)  { latchReg = (latchReg & (~(mask))) | (newValue & mask);  }
#define SET_SHADOW_A
#define SET_SHADOW_B
#define SET_SHADOW_C
	
// Sets the given bit in a latched port.
// The bit number must be constant.
// NOTE: When newValue depends on shadowReg, BoostC may generate bad code!
#define SET_SHADOW_BIT(regName, latchReg, bit, newValue)  { latchReg.bit = newValue; }
#define SET_SHADOW_A_BIT(bit, newValue)  { lata.bit = newValue; }
#define SET_SHADOW_B_BIT(bit, newValue)  { latb.bit = newValue; }
#define SET_SHADOW_C_BIT(bit, newValue)  { latc.bit = newValue; }
	
// Toggles the given bit in a latched port.
// The bit number must be constant.
#define TOGGLE_SHADOW_BIT(regName, latchReg, bit)  { latchReg ^= (1 << bit); }

#endif
//_TJW_SHADOWREGS18_H
