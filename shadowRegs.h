// Defines shadow registers for GPIO regs shared among multiple modules.
// This lets one module change GPIO bits without affecting bits it doesn't control,
// and without causing read-modify-write problems.

#ifndef _TJW_SHADOWREGS_H
#define _TJW_SHADOWREGS_H

#include "types-tjw.h"

#ifdef IN_SHADOWREGS
 #define SHADOW_REGS_EXTERN
#else
 #define SHADOW_REGS_EXTERN  extern
#endif

SHADOW_REGS_EXTERN byte porta_;
SHADOW_REGS_EXTERN byte portb_;
SHADOW_REGS_EXTERN byte portc_;

// Sets the specified shadowed register, changing only the masked bits.
#define SET_SHADOW(regName, shadowReg, newValue, mask)  { shadowReg = (shadowReg & (~(mask))) | (newValue & mask); regName = shadowReg; }
#define SET_SHADOW_A  { porta_ = (porta_ & (~(mask))) | (newValue & mask); porta = porta_; }
#define SET_SHADOW_B  { portb_ = (portb_ & (~(mask))) | (newValue & mask); portb = portb_; }
#define SET_SHADOW_C  { portc_ = (portc_ & (~(mask))) | (newValue & mask); portc = portc_; }
	
// Sets the given bit in a shadowed port.
// The bit number must be constant.
#define SET_SHADOW_BIT(regName, shadowReg, bit, newValue)  { shadowReg.bit = newValue; regName = shadowReg; }
#define SET_SHADOW_A_BIT(pin, newValue)  { porta_.bit = newValue; porta = porta_; }
#define SET_SHADOW_B_BIT(pin, newValue)  { portb_.bit = newValue; portb = portb_; }
#define SET_SHADOW_C_BIT(pin, newValue)  { portc_.bit = newValue; portc = portc_; }

#endif
//_TJW_SHADOWREGS_H
