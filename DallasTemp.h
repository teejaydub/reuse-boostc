/* Dallas/Maxim DS18B20 1-Wire temperature sensor interface.

	Requires onewire.c, which requires onewire-const.h.
	Requires crc_8bit.c (and .h) to do CRC checking.
	
	Be sure to use an internal or external pullup on the 1-Wire bus pin.
	
	And, if you want to support parasite power, don't call DT_ReadDone - 
	either use ReadTemp*, or time the conversion yourself.
	
	Supports both busses supported by the onewire module.
	Bus number can be either 0 or 1.
	
	Only a single sensor is supported on each bus at the moment -
	that is, reading is done in "SKIP ROM" (global reply) mode.
*/

#include "types-tjw.h"
#include "fixed16.h"


#define DT_MIN_TEMP  -55
#define DT_MAX_TEMP  125

// This is returned for the first temperature reading.
// It's in degrees C.
#define DT_POWERUP_TEMP  85

// This is returned when there was an error reading the temperature.
// It's not a value the sensor is supposed to ever return.
// It's in fixed-point degrees C.
#define DT_BAD_TEMPERATURE  ((short) 0x6400)


// Returns the number of sensors connected to the specified bus.
unsigned char DT_CountSensors(byte bus);

// Synchronous reading:

// Reads the temperature to the nearest degree,
// and returns it in a signed byte.
// Returned range is -55 to 125 degrees C.
// If the specified bus has no slave devices on it, the minimum temperature is returned.
// Takes about 96 ms.
signed char DT_ReadTempRough(byte bus);

// Reads the temperature to the highest resolution possible,
// and returns it as 16-bit fixed point (8 bits integer, 8 bits fractional).
// Returned range is -55 to 125 degrees C.
// If the specified bus has no slave devices on it, the minimum temperature is returned.
// Takes about 750 ms.
signed short DT_ReadTempFine(byte bus);

// Asynchronous reading:

// Starts temperature conversion on the given bus.
// No other 1-Wire commands should be done on the bus until DT_ReadDone returns true.
unsigned char DT_StartReadFine(byte bus);

// Returns true if the conversion has finished.
unsigned char DT_ReadDone(byte bus);

// Returns the result of the last temperature conversion on the given bus.
fixed16 DT_GetLastTemp(byte bus);
