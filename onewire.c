
#include <system.h>

#include "onewire.h"
#include "onewire-const.h"


#define OUTPUT_LOW  { clear_bit(ow_port, OW_PIN); clear_bit(ow_tris, OW_PIN); }

#define OUTPUT_HIZ  set_bit(ow_tris, OW_PIN)


char OW_Reset()
{
	// Disable interrupts.
	intcon.GIE = 0;

	// Low for T_RSTL >= 480 us.
	OUTPUT_LOW;
	
	delay_10us(25);
	delay_10us(25);
	
	// Hi-Z.
	OUTPUT_HIZ;
	
	// Wait until halfway between the end of the earliest, shortest possible presence detect pulse (75 us)
	// and the beginning of the latest possible one (60 us).
	// That's 67.5, so wait 60 + a few cycles.
	// But we'll go with 70, since that's what the Dallas docs use.
	delay_10us(7);
	
	// Test the result.
	status.RP0 = 0;  // seems to be needed?!
	char result = ow_port.OW_PIN == 0;
	
	// Interrupts are OK now.
	intcon.GIE = 1;
	
	// Allow it to complete for the remainder of T_RSTH = 480 us since Hi-Z.
	// 480 - 60 = 420.
	// The docs say 430, so use that.
	delay_10us(20);
	delay_10us(23);
	
	return result;
}

void OW_SendByte(unsigned char b)
{
	unsigned char bitCount = 8;
	
	// Disable interrupts.
	intcon.GIE = 0;

	sendLoop:
		// Low for 4 us (docs say 5 us).
		nop();
		nop();
		OUTPUT_LOW;
		nop();
		nop();
		nop();
		nop();
		
		// Output the next bit.
		asm { 
			rrf _b, F 
		}
		if (status.C)
			set_bit(ow_port, OW_PIN);
			
		// Wait for 60 us.
		delay_10us(6);
		
		// Recovery time >= 1 us.
		OUTPUT_HIZ;
		
		nop();
		nop();
		
		asm {
			decfsz _bitCount, F
			goto sendLoop
		}
	
	// Restore interrupts.
	intcon.GIE = 1;
}

unsigned char OW_ReadByte()
{
	unsigned char bitCount = 8;
	unsigned char result = 0;
	
	// Disable interrupts.
	intcon.GIE = 0;

	recLoop:
		// Low for 6 us.
		nop();
		nop();
		OUTPUT_LOW;
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();

		// HIZ for 4 us.
		OUTPUT_HIZ;
		nop();
		nop();
		nop();
		nop();
		nop();
		
		// Get the next bit.

#if OW_MASK != 0x02
 #error Have to hard-code the pin constant here, unfortunately.
#endif
		asm { 
			movf _ow_port, W
			andlw 0x02  // OW_MASK
			addlw 255  // C = 1 iff the next input bit = 1
			rrf _result, F
		}
//		if (OW_PORT.OW_PIN)  // this should happen <= 15 ms from when the output goes low, but close to it.
//			set_bit(result, 7);
//		else
//			clear_bit(result, 7);
		
		// Total time must be > 61 us.
		delay_10us(5);
	
		asm {
			decfsz _bitCount, F
			goto recLoop
		}
	
	// Restore interrupts.
	intcon.GIE = 1;
	
	return result;
}

