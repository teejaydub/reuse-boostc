/* BasicBus.h
    Copyright (c) 2017, 2018 by Timothy J. Weber, tw@timothyweber.org.
*/

#ifndef __BASICBUS_H
#define __BASICBUS_H

#ifdef IN_BASICBUS
 #define BB_EXTERN  
#else
 #define BB_EXTERN  extern
#endif

#include "fixed16.h"
#include "types-tjw.h"

// Holds the last status returned by the master.
// The caller can set this to an initial value if desired.
BB_EXTERN byte bbMasterStatus;

// Call this right away after startup.
// Uses the hardware serial port.
// Sets the slave ID to respond to on the bus.
// Sends and receives parameter values from the given buffer.
void InitializeBasicBus(byte id, byte paramCount, unsigned short* params);

// Call this in your low-priority interrupt routine.
// It returns true if it handled an interrupt.
byte BasicBusISR(void);

// Call this frequently to keep the queue flowing.
// It will call the handler functions below.
// Returns true if any commands were received from the master.
byte PollBasicBus(void);


// Implement this in the calling code.
// It's called when a new value is received for the specified parameter.
// The new value is already in the params buffer sent to InitializeBasicBus().
void BBParameter(byte index);

// Implement this in the calling code.
// It's called when the master has requested that all parameters be reset.
// After it returns, all parameters are scheduled to send out.
void ResetBBParams(void);

// Call this when the given parameter has changed, to schedule it for sending out.
void ChangedBBParameter(byte index);

// Or call this, if you don't know which parameter has changed, to schedule all of them for sending out.
void EnqueueBBParameters(void);

// Returns true if any parameters are (still) queued to be sent.
byte AnyBBParamsQueued(void);


// This is called to request a specific variable to be sent.
// Handle it by sending that variables value with SendBB* below.
// If the code is '*', send all variables.
// If the code is 'S', send the slave's status.
void OnBBRequest(byte code);

// Call this to send a variable reading, when requested.
void SendBBReading(byte code, unsigned short value);

// Same for fixed-point variables.
void SendBBFixedReading(byte code, fixed16 value);

// Clears the output buffer immediately,
// e.g. to make room for an urgent next message.
void ClearBBOutput(void);


// Put a string out on the bus, outside of commands, for diagnostics.
// Note that this takes over as the selected slave, so that you can see the output;
// it'll cause havoc if called in a multi-slave environment.
void putsBB(const char* s);
void putcBB(char c);

#endif
// __BASICBUS_H