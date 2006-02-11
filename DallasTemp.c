#include <system.h>

#include "onewire.h"

#include "DallasTemp.h"


// Defines for 1-Wire commands.
#define DT_ConvertT  0x44
#define DT_ReadScratchPad  0xBE
#define DT_WriteScratchPad  0x4E
#define DT_CopyScratchPad  0x48
	
// offset of the resolution bits in the config register
#define DT_ConfigResOffset  5


// The number of significant bits for low-res sensing.
#define BitsToSense_LowRes  9

// Conversion time in ms, for low-res sensing.
#define ConversionTime_LowRes  ((94 << (BitsToSense_LowRes - 9)) + 1)

unsigned char DT_CountSensors()
{
	// For now, assume we have at most one sensor connected.
	if (OW_Reset())
		return 1;
	else
		return 0;
}

signed char DT_ReadTempRough(unsigned char sensor)
{
	// Assume there's at most one sensor.
	if (sensor > 0)
		return DT_MIN_TEMP;

	// Talk to whoever's attached (assuming one).
	OW_SendByte(OW_SkipROM);
	
	// Configure for low resolution.
	OW_SendByte(DT_WriteScratchPad);
	OW_SendByte(DT_MAX_TEMP);  // Alarm THigh (disable).
	OW_SendByte(DT_MIN_TEMP - 1);  // Alarm TLow (disable).
	OW_SendByte((BitsToSense_LowRes - 9) << DT_ConfigResOffset);  // Resolution.
	
	OW_Reset();  // Don't need to write anything else.

	// Start temperature conversion.
	OW_SendByte(OW_SkipROM);
	OW_SendByte(DT_ConvertT);
	
	// Wait for it to finish.
	delay_ms(ConversionTime_LowRes);

	// Read the temperature value.
	OW_Reset();
	OW_SendByte(OW_SkipROM);
	OW_SendByte(DT_ReadScratchPad);
	
	unsigned char lsb = OW_ReadByte();
	unsigned char msb = OW_ReadByte();
	
	// That's all we need, but it's going to keep sending the rest of its memory.
	// We'd be bored by that, so reset the bus.
	OW_Reset();
	
	// Construct the return value from the two value bytes.
	signed char result = (lsb >> 4) | (msb << 4);
	if (test_bit(lsb, 3))  // 2^-1 bit
		// round up
		return result + 1;
	else
		// round down
		return result;
}
