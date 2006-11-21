// Defines shadow registers for GPIO regs shared among multiple modules.
// This lets one module change GPIO bits without affecting bits it doesn't control,
// and without causing read-modify-write problems.

#ifndef _TJW_SHADOWREGS_H
#define _TJW_SHADOWREGS_H

#include "types-tjw.h"

#ifdef IN_SHADOW_REGS
 #define SHADOW_REGS_EXTERN
#else
 #define SHADOW_REGS_EXTERN  extern
#endif

SHADOW_REGS_EXTERN byte porta_;
SHADOW_REGS_EXTERN byte portb_;
SHADOW_REGS_EXTERN byte portc_;

// Sets the specified shadowed register, changing only the masked bits.
#define SET_SHADOW(regName, shadowReg, newValue, mask)  { shadowReg = (shadowReg & (~(mask))) | newValue; regName = shadowReg; }
	
// Sets the given shadowed pin.
#define SET_SHADOW_PIN(regName, shadowReg, newValue, pin)  { shadowReg = (shadowReg & (~(BITMASK(pin))) | (newValue? BITMASK(pin): 0); regName = shadowReg; }
	
#endif
//_TJW_SHADOWREGS_H
