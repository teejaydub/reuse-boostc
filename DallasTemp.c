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

// The number of significant bits for low-res and high-res sensing.
#define BitsToSense_LowRes  9
#define BitsToSense_HighRes  12

// Conversion time in ms, for low-res sensing.
#define ConversionTime_LowRes  95
#define ConversionTime_HighRes  751


unsigned char DT_CountSensors()
{
	// For now, assume we have at most one sensor connected.
	if (OW_Reset())
		return 1;
	else
		return 0;
}

signed short DoRead(unsigned char configReg, unsigned short conversionTime)
{
	// Talk to whoever's attached (assuming one).
	OW_SendByte(OW_SkipROM);
	
	// Configure for low resolution.
	OW_SendByte(DT_WriteScratchPad);
	OW_SendByte(DT_MAX_TEMP);  // Alarm THigh (disable).
	OW_SendByte(DT_MIN_TEMP - 1);  // Alarm TLow (disable).
	OW_SendByte(configReg);  // Resolution.
	
	OW_Reset();  // Don't need to write anything else.

	// Start temperature conversion.
	OW_SendByte(OW_SkipROM);
	OW_SendByte(DT_ConvertT);
	
	// Wait for it to finish.
	while (conversionTime > 255) {
		delay_ms(255);
		conversionTime -= 255;
	}
	delay_ms((unsigned char)(conversionTime));

	// Read the temperature value.
	OW_Reset();
	OW_SendByte(OW_SkipROM);
	OW_SendByte(DT_ReadScratchPad);
	
	unsigned char lsb = OW_ReadByte();
	signed char msb = OW_ReadByte();
	
	// Adjust to a sane representation.
	signed short result = (msb << 8) | lsb;
	result <<= 4;
	
	// That's all we need, but it's going to keep sending the rest of its memory.
	// We'd be bored by that, so reset the bus.
	OW_Reset();
	
	return result;
}

signed char DT_ReadTempRough(unsigned char sensor)
{
	// Assume there's at most one sensor.
	if (sensor > 0)
		return DT_MIN_TEMP;
		
	// Check for the sensor one more time.
	if (!OW_Reset())
		return 0;
		
	short value = DoRead(
		(BitsToSense_LowRes - 9) << DT_ConfigResOffset, 
		ConversionTime_LowRes);

	// Construct the return value from the two value bytes.
	signed char result = value >> 8;
	
	if (test_bit(value[1], 7))  // 2^-1 bit
		// round up
		return result + 1;
	else
		// round down
		return result;
}

signed short DT_ReadTempFine(unsigned char sensor)
{
	// Assume there's at most one sensor.
	if (sensor > 0 || !OW_Reset())
		return 0;
	else 
		return DoRead(
			(BitsToSense_HighRes - 9) << DT_ConfigResOffset, 
			ConversionTime_HighRes);
}
