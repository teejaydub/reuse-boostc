/* serial.h */

#ifndef __SERIAL_H
#define __SERIAL_H

#ifdef IN_SERIAL
#define SERIAL_EXTERN
#else
#define SERIAL_EXTERN extern
#endif

// If this is set, ser_data is the next incoming byte.
// Cleared automatically by ReadSerial().
SERIAL_EXTERN bit ser_hasData;

// If this is set, there has been some kind of error.
// When you've recovered, call InitializeSerial().
SERIAL_EXTERN bit ser_error;

// Set to a character representing the error type.
// Errors detected:
//   F: Framing error - no stop bit received (actually, when we expected a stop bit, line was low)
//   C: Collision (new data finished before old data read)
SERIAL_EXTERN char ser_errorType;

// After calling this, set GIE to start processing.
void InitializeSerial();

// Must be called in an ISR.
void SerialInterrupt();

// Returns the next available character.
// If this isn't called often enough, and incoming bytes collide, the Collision error is reported.
unsigned char ReadSerial();

#endif