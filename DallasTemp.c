#include <system.h>

#include "onewire.h"
#include "types-tjw.h"

#include "DallasTemp.h"


// Defines for 1-Wire commands.
#define DT_ConvertT  0x44
#define DT_ReadScratchPad  0xBE
#define DT_WriteScratchPad  0x4E
#define DT_CopyScratchPad  0x48
#define DT_ReadPowerSupply  0xB4
	
// offset of the resolution bits in the config register
#define DT_ConfigResOffset  5

// The number of significant bits for low-res and high-res sensing.
#define BitsToSense_LowRes  9
#define BitsToSense_HighRes  12

// Conversion time in ms, for low-res sensing.
#define ConversionTime_LowRes  95
#define ConversionTime_HighRes  751


unsigned char DT_CountSensors(byte bus)
{
	// For now, assume we have at most one sensor connected.
	if (OWB_Reset(bus))
		return 1;
	else
		return 0;
}

byte DT_IsParasite(byte bus)
{
	if (bus) {
		OW_SendByte_2(OW_SkipROM);
		OW_SendByte_2(DT_ReadPowerSupply);
		return !OW_ReadBit_2();  // parasite-powered devices pull the bus low.
	} else {
		OW_SendByte(OW_SkipROM);
		OW_SendByte(DT_ReadPowerSupply);
		return !OW_ReadBit();  // parasite-powered devices pull the bus low.
	}
}

void StartRead(byte bus, unsigned char configReg)
{
	byte useParasite = DT_IsParasite(bus);
	
	if (bus) {
		// Talk to whoever's attached (assuming one).
		OW_SendByte_2(OW_SkipROM);
		
		// Configure for low resolution.
		OW_SendByte_2(DT_WriteScratchPad);
		OW_SendByte_2(DT_MAX_TEMP);  // Alarm THigh (disable).
		OW_SendByte_2(DT_MIN_TEMP - 1);  // Alarm TLow (disable).
		OW_SendByte_2(configReg);  // Resolution.
		
		OW_Reset_2();  // Don't need to write anything else.
		
		// Start temperature conversion.
		OW_SendByte_2(OW_SkipROM);
		OW_SendByte_2(DT_ConvertT);
		
		// Support parasite power.
		if (useParasite) 
			OW_PowerOn_2();
	} else {
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
		
		// Support parasite power.
		if (useParasite) 
			OW_PowerOn();
	}
}

signed short DoRead(byte bus, unsigned char configReg, unsigned short conversionTime)
{
	StartRead(bus, configReg);
	
	// Wait for it to finish.
	while (conversionTime > 255) {
		delay_ms(255);
		conversionTime -= 255;
	}
	delay_ms((unsigned char)(conversionTime));
	
	return DT_GetLastTemp(0);
}

signed char DT_ReadTempRough(byte bus)
{
	// The bus can only be 0 or 1.
	if (bus > 1)
		return DT_MIN_TEMP;
		
	// Check for the sensor one more time.
	if (!OWB_Reset(bus))
		return 0;
		
	short value = DoRead(bus, 
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

signed short DT_ReadTempFine(byte bus)
{
	// The bus can only be 0 or 1, and it must have something on it.
	if (bus > 1 || !OWB_Reset(bus))
		return 0;
	else 
		return DoRead(bus, 
			(BitsToSense_HighRes - 9) << DT_ConfigResOffset, 
			ConversionTime_HighRes);
}

unsigned char DT_StartReadFine(byte bus)
{
	// The bus can only be 0 or 1, and it must have something on it.
	if (bus > 1 || !OWB_Reset(bus))
		return 0;
	else { 
		StartRead(bus, (BitsToSense_HighRes - 9) << DT_ConfigResOffset);
		return 1;
	}
}

unsigned char DT_ReadDone(byte bus)
{
	return OWB_ReadByte(bus);
}

signed short DT_GetLastTemp(byte bus)
{
	// Read the temperature value.
	if (!OWB_Reset(bus))
		return 0;

	unsigned char lsb;
	signed char msb;

	if (bus) {	
		OW_SendByte_2(OW_SkipROM);
		OW_SendByte_2(DT_ReadScratchPad);
		
		lsb = OW_ReadByte_2();
		msb = OW_ReadByte_2();
	} else {
		OW_SendByte(OW_SkipROM);
		OW_SendByte(DT_ReadScratchPad);
		
		lsb = OW_ReadByte();
		msb = OW_ReadByte();
	}
		
	// Adjust to a sane representation.
	signed short result = (msb << 8) | lsb;
	result <<= 4;
	
	// That's all we need, but it's going to keep sending the rest of its memory.
	// We'd be bored by that, so reset the bus.
	OWB_Reset(bus);
	
	return result;
}

