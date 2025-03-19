/* shadowRegs.h
    Copyright (c) 2006-2018 by Timothy J. Weber, tw@timothyweber.org.

    Defines shadow registers for GPIO regs shared among multiple modules.
    This lets one module change GPIO bits without affecting bits it doesn't control,
    and without causing read-modify-write problems.
*/

#ifndef _TJW_SHADOWREGS_H
#define _TJW_SHADOWREGS_H

#include "types-tjw.h"

#ifdef IN_SHADOWREGS
 #define SHADOW_REGS_EXTERN
#else
 #define SHADOW_REGS_EXTERN  extern
#endif

#if defined(_PIC16F1789) || defined(_PIC16F887) || defined(_PIC18F45K22)
 #define NUM_PORTS  5
#else
 #define NUM_PORTS  3
#endif

// Later chips have LATx registers that make all of this unnecessary.
#if defined(_PIC16F1789) || defined(_PIC18F45K22)
 #define HAS_LATCH
#endif

#ifdef HAS_LATCH
	// Just use the latch registers as the shadow registers.
	#define porta_  lata
	#define portb_  latb
	#define portc_  latc
	#if NUM_PORTS >= 5
	#define portd_  latd
	#define porte_  late
	#endif
#else
	SHADOW_REGS_EXTERN byte porta_;
	SHADOW_REGS_EXTERN byte portb_;
	SHADOW_REGS_EXTERN byte portc_;
	#if NUM_PORTS >= 5
	SHADOW_REGS_EXTERN byte portd_;
	SHADOW_REGS_EXTERN byte porte_;
	#endif
#endif

// Sets the specified shadowed register, changing only the masked bits.
#ifdef HAS_LATCH
	#define SET_SHADOW(regName, shadowReg, newValue, mask)  { shadowReg = (shadowReg & (~(mask))) | (newValue & mask); }
	#define SET_SHADOW_A(newValue, mask)  { porta_ = (porta_ & (~(mask))) | (newValue & mask); }
	#define SET_SHADOW_B(newValue, mask)  { portb_ = (portb_ & (~(mask))) | (newValue & mask); }
	#define SET_SHADOW_C(newValue, mask)  { portc_ = (portc_ & (~(mask))) | (newValue & mask); }
	#if NUM_PORTS >= 5
	 #define SET_SHADOW_D(newValue, mask)  { portd_ = (portd_ & (~(mask))) | (newValue & mask); }
	 #define SET_SHADOW_E(newValue, mask)  { porte_ = (porte_ & (~(mask))) | (newValue & mask); }
	#endif
#else
	#define SET_SHADOW(regName, shadowReg, newValue, mask)  { shadowReg = (shadowReg & (~(mask))) | (newValue & mask); regName = shadowReg; }
	#define SET_SHADOW_A(newValue, mask)  { porta_ = (porta_ & (~(mask))) | (newValue & mask); porta = porta_; }
	#define SET_SHADOW_B(newValue, mask)  { portb_ = (portb_ & (~(mask))) | (newValue & mask); portb = portb_; }
	#define SET_SHADOW_C(newValue, mask)  { portc_ = (portc_ & (~(mask))) | (newValue & mask); portc = portc_; }
	#if NUM_PORTS >= 5
	 #define SET_SHADOW_D(newValue, mask)  { portd_ = (portd_ & (~(mask))) | (newValue & mask); portd = portd_; }
	 #define SET_SHADOW_E(newValue, mask)  { porte_ = (porte_ & (~(mask))) | (newValue & mask); porte = porte_; }
	#endif
#endif
	
// Sets the given bit in a shadowed port.
// The bit number must be constant.
// NOTE: When newValue depends on shadowReg, BoostC seems to generate bad code!
#ifdef HAS_LATCH
	#define SET_SHADOW_BIT(regName, shadowReg, bit, newValue)  { shadowReg.bit = newValue; }
	#define SET_SHADOW_A_BIT(bit, newValue)  { porta_.bit = newValue; }
	#define SET_SHADOW_B_BIT(bit, newValue)  { portb_.bit = newValue; }
	#define SET_SHADOW_C_BIT(bit, newValue)  { portc_.bit = newValue; }
	#if NUM_PORTS >= 5
	 #define SET_SHADOW_D_BIT(bit, newValue)  { portd_.bit = newValue; }
	 #define SET_SHADOW_E_BIT(bit, newValue)  { porte_.bit = newValue; }
	#endif
#else
	#define SET_SHADOW_BIT(regName, shadowReg, bit, newValue)  { shadowReg.bit = newValue; regName = shadowReg; }
	#define SET_SHADOW_A_BIT(bit, newValue)  { porta_.bit = newValue; porta = porta_; }
	#define SET_SHADOW_B_BIT(bit, newValue)  { portb_.bit = newValue; portb = portb_; }
	#define SET_SHADOW_C_BIT(bit, newValue)  { portc_.bit = newValue; portc = portc_; }
	#if NUM_PORTS >= 5
	 #define SET_SHADOW_D_BIT(bit, newValue)  { portd_.bit = newValue; portd = portd_; }
	 #define SET_SHADOW_E_BIT(bit, newValue)  { porte_.bit = newValue; porte = porte_; }
	#endif
#endif
	
// Toggles the given bit in a shadowed port.
// The bit number must be constant.
#ifdef HAS_LATCH
	#define TOGGLE_SHADOW_BIT(regName, shadowReg, bit)  { shadowReg ^= (1 << bit); }
#else
	#define TOGGLE_SHADOW_BIT(regName, shadowReg, bit)  { shadowReg ^= (1 << bit); regName = shadowReg; }
#endif

#endif
//_TJW_SHADOWREGS_H
