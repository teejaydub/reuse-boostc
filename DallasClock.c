// Dallas real-time clock implementation.

#include <system.h>

#define IN_DALLAS_CLOCK
#include "DallasClock.h"

#include "DallasClock_consts.h"
#include "i2c_driver.h"

#define DIRECTION_WRITE  0
#define DIRECTION_READ  1

#define ACKNOWLEDGE  0
#define NO_ACKNOWLEDGE  1

// Set this from the table in the PIC data sheet - in 18F series it's labeled "I2C� CLOCK RATE W/BRG"
#define I2C_BAUD_VALUE  0x12  // = 100 kHz at 8 MHz clock rate

#define I2C_SLAVE_ADDRESS  0xD0  // For the DS1307 RTC module.


//=============================================================================
// Intermediate I2C support

void InitI2CBus(void)
{
	volatile bit scl_tris@TRISC.SCL, sda_tris@TRISC.SDA;
	volatile bit scl@PORTC.SCL, sda@PORTC.SDA;

	// Flush the bus "manually" until we've read 10 consecutive ones.
	// Others have had success with this method for recovering if there are some slave
	// devices that are still waiting for data to get clocked out from previous reads,
	// e.g. if the PIC is getting restarted after programming and the previous execution
	// was in the middle of a read when programming started.
	sspcon1.SSPEN = 0;
	sda_tris = 1;
	scl = 1;
	scl_tris = 0;
	scl = 1;
		
	byte ones = 0;
	while (ones < 10) {
		// One clock pulse.
		scl = 0;
		delay_us(10);
		if (sda)
			ones++;
		else
			ones = 0;
		scl = 1;
	}
	
	// Now, do the library initialization.
	i2c_init(I2C_BAUD_VALUE);
}

// Writes the specified bytes starting at the specified register; returns nonzero on success.
byte WriteToI2C(byte address, byte* values, byte len)
{
	i2c_start();
	if (i2c_write(I2C_SLAVE_ADDRESS | DIRECTION_WRITE))
		return false;
	if (i2c_write(address))
		return false;
	while (len--)
		if (i2c_write(*values++))
			return false;
		
	i2c_stop();
	return true;
}

// Writes the specified byte to the specified register; returns nonzero on success.
byte WriteByteToI2C(byte address, byte value)
{
	i2c_start();
	if (i2c_write(I2C_SLAVE_ADDRESS | DIRECTION_WRITE))
		return false;
	if (i2c_write(address))
		return false;
	if (i2c_write(value))
		return false;
		
	i2c_stop();
	return true;
}

// Reads the specified number of bytes from the device into retValues, starting at the specified address.
// Returns true on success.
byte ReadFromI2C(byte address, byte* retValues, byte len)
{
	// Set the register pointer that we want to read from.
	i2c_start();
	if (i2c_write(I2C_SLAVE_ADDRESS | DIRECTION_WRITE))
		return false;
	if (i2c_write(address))
		return false;
	
	// Now do the read.
	i2c_restart();
	if (i2c_write(I2C_SLAVE_ADDRESS | DIRECTION_READ))
		return false;
	while (len--)
		*retValues++ = i2c_read(len == 0? NO_ACKNOWLEDGE: ACKNOWLEDGE);
	i2c_stop();
		
	return true;
}


//=============================================================================
// Clock-specific routines.

// Initialize to a known time.
void WriteEpoch(void)
{
	#ifdef ZERO_EPOCH
	byte currentTime[] = { 0, 0x00, 0x00, 7, 0x01, 0x01, 0x00 };
	#else
	byte currentTime[] = { 0x0, 0x13, 0x09, 1, 0x16, 0x10, 0x16 };
	#endif
	WriteToI2C(0, currentTime, 7);
}

byte InitDallasClock(void)
{
	InitI2CBus();
	return ReadClock() != 0;
}

byte GetClockSeconds(void)
{
	byte secondsBCD;
	if (ReadFromI2C(0, &secondsBCD, 1))
		return secondsBCD;
	else
		return 0xFF;
}

void GetClockMemory(byte* buffer)
{
	ReadFromI2C(0, buffer, 8);
}

byte ReadClock(void)
{
	ReadFromI2C(0, currentTimeBuf, 7);
	return currentTime.seconds < 0x60;
}

void WriteClock(void)
{
	// We apparently need to stop the clock in order to set it.
	// This is undocumented, but kinda makes a little sense, sort of, I guess.
	byte stopByte = 0x80;
	WriteToI2C(0, &stopByte, 1);

	WriteToI2C(0, currentTimeBuf, 7);
}

void SetClockRegister(byte index, byte newValue)
{
	if (index == 0) {
		// This is an easy case: just write it.
		WriteToI2C(0, &newValue, 1);
		return;
	}

	// Save the old seconds value.
	byte oldSeconds = currentTime.seconds;

	// Stop the clock by setting the seconds to 0x80.
	byte stopByte = 0x80;
	WriteToI2C(0, &stopByte, 1);
	
	// Read the whole time.
	ReadFromI2C(0, currentTimeBuf, 7);
	
	// Modify the whole time.
	currentTimeBuf[index] = newValue;
	
	// Restore the seconds.
	if (index == 0)
		currentTime.seconds = newValue;
	else
		currentTime.seconds = oldSeconds;
	
	// Write the whole time back out.
	WriteToI2C(0, currentTimeBuf, 7);
}
