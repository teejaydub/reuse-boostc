/* serial.h 
    Copyright (c) 2006, 2007 by Timothy J. Weber, tw@timothyweber.org.

	Define SOFTWARE_RECEIVE to provide reception in software (not debugged or polished yet).
*/

#ifndef __SERIAL_H
#define __SERIAL_H

#ifdef IN_SERIAL
#define SERIAL_EXTERN
#else
#define SERIAL_EXTERN extern
#endif

// If this is set, there's a new byte to be read with ReadSerial().
// (Internal: dataQueue is the next incoming byte.)
// Cleared automatically by ReadSerial().
SERIAL_EXTERN bit ser_hasData;

// If this is set, there has been some kind of error.
// When you've recovered, call InitializeSerial().
SERIAL_EXTERN bit ser_error;

// Set to a character representing the error type.
// Errors detected:
//   F: Framing error - no stop bit received (actually, when we expected a stop bit, line was low)
//   C: Collision (new data finished before old data read)
//   c: Collision, soft (the buffer in this module has overflowed)
SERIAL_EXTERN char ser_errorType;

// After calling this, set GIE to start processing.
void InitializeSerial();  // equivalent to receive, no transmit, for legacy reasons.
void InitializeSerial2(bool useReceive, bool useTransmit);

// Must be called in an ISR.
void SerialInterrupt();

// Returns the next available character.
// If this isn't called often enough, and incoming bytes collide, the Collision error is reported.
unsigned char ReadSerial();

// Sends the specified character out the serial port.
inline void WriteSerial(char c)
{
	while (!pir1.TXIF)
		;
	txreg = c;
}

// Sends the specified null-terminated string out the serial port.
inline void WriteSerialString(char* s)
{
	while (*s != 0)
		WriteSerial(*s++);
}

// Sends the specified string out the serial port.
inline void WriteSerialBuf(unsigned char* buf, unsigned char len)
{
	while (len--)
		WriteSerial(*buf++);
}

#endif
