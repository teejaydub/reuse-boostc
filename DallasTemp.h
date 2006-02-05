/* Dallas/Maxim DS18B20 1-Wire temperature sensor interface.
*/

#define DT_MIN_TEMP  -55
#define DT_MAX_TEMP  125

// Returns the number of sensors connected.
unsigned char DT_CountSensors();

// Reads the temperature to the nearest degree,
// and returns it in a signed byte.
// Returned range is -55 to 125 degrees C.
// If the specified sensor doesn't exist, the minimum temperature is returned.
signed char DT_ReadTempRough(unsigned char sensor);
