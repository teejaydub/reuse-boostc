/* Dallas/Maxim DS18B20 1-Wire temperature sensor interface.

	Requires onewire.c, which requires onewire-const.h.
	
	Be sure to use an internal or external pullup on the 1-Wire bus pin.
	
	And, if you want to support parasite power, don't call DT_ReadDone - 
	either use ReadTemp*, or time the conversion yourself.	
*/

#define DT_MIN_TEMP  -55
#define DT_MAX_TEMP  125

// This is returned for the first temperature reading.
#define DT_POWERUP_TEMP  85

// Returns the number of sensors connected.
unsigned char DT_CountSensors();

// Synchronous reading:

// Reads the temperature to the nearest degree,
// and returns it in a signed byte.
// Returned range is -55 to 125 degrees C.
// If the specified sensor doesn't exist, the minimum temperature is returned.
// Takes about 96 ms.
signed char DT_ReadTempRough(unsigned char sensor);

// Reads the temperature to the highest resolution possible,
// and returns it as 16-bit fixed point (8 bits integer, 8 bits fractional).
// Returned range is -55 to 125 degrees C.
// If the specified sensor doesn't exist, the minimum temperature is returned.
// Takes about 750 ms.
signed short DT_ReadTempFine(unsigned char sensor);

// Asynchronous reading:

// Starts temperature conversion on the given sensor.
// No other 1-Wire commands should be done on the bus until DT_ReadDone returns true.
unsigned char DT_StartReadFine(unsigned char sensor);

// Returns true if the conversion has finished.
unsigned char DT_ReadDone(unsigned char sensor);

// Returns the result of the last temperature conversion on the given sensor.
signed short DT_GetLastTemp(unsigned char sensor);
