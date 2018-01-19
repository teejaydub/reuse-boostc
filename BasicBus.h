/* BasicBus.h
    Copyright (c) 2017, 2018 by Timothy J. Weber, tw@timothyweber.org.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


	Simple serial protocol, based loosely on the original BasicBus design from 2004.
	Geared toward multi-slave capability, but currently only supports one master and one slave.
	
	This implements the slave side, processing these commands from the master:
	S=b
		Reports the master status, byte b.
	P?
		Requests a report of all parameters.

    Sends these responses back to the master:
    Pn=s
        Reports that parameter n (0-relative byte offset)'s new value is s (unsigned short).
    S=n
        Reports a change in the slave's status, as a byte value.
	A=x
		Sends a reading for variable code A with value x.
		A is any ASCII code (but traditionally capital or lowercase letters), except P.
		x is a decimal value, unsigned byte or smaller.

    Commands must be on a line (\n-delimited) that starts with '~'.
    Commands may be grouped together on a line, separated by spaces.
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

// Holds the current status returned by the master.
BB_EXTERN byte bbMasterStatus;

// Call this right away after startup.
// Uses the hardware serial port.
// Sends and receives parameter values from the given buffer.
void InitializeBasicBus(byte paramCount, unsigned short* params);

// Call this in your low-priority interrupt routine.
// It returns true if it handled an interrupt.
byte BasicBusISR(void);

// Call this frequently to keep the queue flowing.
// It will call the handler functions below.
void PollBasicBus(void);

// Implement this in the calling code.
// It's called when a new value is received for the specified parameter.
// The new value is already in the params buffer sent to InitializeBasicBus().
void BBParameter(byte index);

// Call this when the given parameter has changed, to schedule it for sending out.
void ChangedBBParameter(byte index);

// Or call this, if you don't know which parameter has changed, to schedule all of them for sending out.
void EnqueueBBParameters(void);

// Call this to send a variable reading.
// Returns true if there was enough room in the outgoing buffer, false if not (and does nothing).
// If you don't have time to wait for this to return true, you may need to call ClearBBOutput() first.
byte SendBBReading(byte code, unsigned short value);

// Same for fixed-point variables.
byte SendBBFixedReading(byte code, fixed16 value);

// Clears the output buffer immediately,
// e.g. to make room for an urgent next message.
void ClearBBOutput(void);

#endif
// __BASICBUS_H