/* BasicBus.c
    Copyright (c) 2017 by Timothy J. Weber, tw@timothyweber.org.

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
*/

#define IN_BASICBUS

#include <system.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "BasicBus.h"

#include "byteBuffer.h"

// Define this for concise logging about commands received and processed.
// Define it to "2" to also see every character received.
#define LOGGING  1
// #undef LOGGING

#define SERIAL_BUFLEN  48

#define MAX_VARIABLES  10

byte slaveID;
byte isSelectedSlave = false;

byte serialOutputBuffer[SERIAL_BUFLEN];
byte serialInputBuffer[SERIAL_BUFLEN];

ByteBuf serialInput;
ByteBuf serialOutput;

#ifdef LOGGING
// Holds a character to represent the last error on the serial input port
byte lastSerialError = '\0';
#endif

// Commands must be preceded by \n~ to be parsed, and should be followed by \n to ensure quick processing.
// But that makes a lot of extra lines while logging.
// So, only send them when necessary.
byte justSentNewline = false;

// Pointer to the block of parametrs, with size.
// This is maintained here, sending and receiving, and the client gets a callback when things change.
byte bbParamCount = 0;
unsigned short* bbParams = 0;

// When these are in order or equal, send the specified parameters from firmwareParams to serial.
byte firstParamToReport = 1;  // the 0-relative index of the first param that needs reporting
byte lastParamToReport = 0;  // the 0-relative index of the last param that needs reporting

// When this null-terminated string is nonempty, re-request the specified variables to be sent.
// That is - it's just the symbols for each variable that we tried to send but couldn't,
// held until they've been sent.
byte queuedVariablesBuffer[MAX_VARIABLES] = "";
ByteBuf queuedVariables;

byte wildcardUnderway = false;

typedef enum {
	IN_GARBAGE,  // startup or in a line that we should ignore
	AT_LINE_START,  // just saw a newline
    BETWEEN_COMMANDS,  // waiting for a command or whitespace
    AT_COMMAND,  // the first thing in the buffer is a non-space potential command
	IN_COMMAND_TAIL,  // just processed some prefix of a command; skip the rest.
} SerInState;
SerInState serInState;

// Forward declarations
byte ProcessBBCommands(void);
void putc(char c);

#ifdef _PIC18F45K22
 #define RX_TRIS  trisc
 #define RX_ANSEL  anselc
 #define RX_PIN  7

 #define TX_TRIS  trisc
 #define TX_PIN  6
#else
 #error Need to define the RX port pin for this chip.
#endif


//============================================================================
// Serial port usage and initialization

// If isSelected, take control of the MISO line.
// Otherwise, tri-state it.
void beSelectedSlave(byte isSelected)
{
    if (isSelected) {
        txsta.TXEN = 1;
        TX_TRIS.TX_PIN = 0;
    } else {
        txsta.TXEN = 0;
        TX_TRIS.TX_PIN = 1;
        clear(serialOutput);
    }

    isSelectedSlave = isSelected;
}

void InitializeBasicBus(byte id, byte paramCount, unsigned short* params)
{
	// Set for 9600 baud.
	txsta.BRGH = 1;  // Set baud rate to...
	spbrg = 25;  // ... 9600 baud
	rcsta.SPEN = 1;  // Enable serial port.
	rcsta.CREN = 1;  // Enable serial reception.

	pie1.RCIE = 1;  // Interrupts on reception.

    RX_TRIS.RX_PIN = 1;
    RX_ANSEL.RX_PIN = 0;

	serInState = AT_LINE_START;

    bbMasterStatus = ' ';  // A convenient default status.
	
	init(serialInput, serialInputBuffer);
	init(serialOutput, serialOutputBuffer);
    init(queuedVariables, queuedVariablesBuffer);

    beSelectedSlave(false);
	
    slaveID = id;
	bbParamCount = paramCount;
	bbParams = params;
}

byte BasicBusISR(void)
{
    if (pir1.RCIF) {
        byte c;
        if (rcsta.FERR) {
            c = rcreg;
            #ifdef LOGGING
            lastSerialError = '#';
            #endif
        } else {
            c = rcreg;
            if (rcsta.OERR) {
                // Overrun error, meaning we missed some characters - restart reception.
                rcsta.CREN = 0;
                clear(serialInput);
                rcsta.CREN = 1;
                #ifdef LOGGING
                lastSerialError = '!';
                #endif
            } else {
                push<SERIAL_BUFLEN>(serialInput, c);
                #if defined(LOGGING) && (LOGGING >= 2)
                putc('>');
                putc(c);
                putNewline();
                #endif
            }
        }
        return true;
    } else
        return false;
}


//============================================================================
// Serial I/O

// Enqueues this character for sending.
// Drops it if the queue is full.
void putc(char c)
{
    if (isSelectedSlave) {
        push<SERIAL_BUFLEN>(serialOutput, c);
        justSentNewline = (c == '\n');
    }
}

// Standardize so we can easily change to "\r\n" if necessary.
inline void putNewline(void)
{
    putc('\n');
}

inline void ensureNewline(void)
{
    if (!justSentNewline)
        putNewline();
}

void puts(const char* s)
{
	while (*s)
		putc(*s++);
}

void putsBB(const char* s) {
    if (!isSelectedSlave)
        beSelectedSlave(true);
    puts(s);
}

void putnibble(byte n)
{
	if (n <= 9)
		putc('0' + n);
	else
		putc('A' + n - 0xA);
}

void puthex(byte b)
{
	putnibble(b >> 4);
	putnibble(b & 0x0F);
}

void putDecimal(int value)
{
	char buf[7];
	itoa(value, buf, 10);
	puts(buf);
}

void putSignedByteDecimal(signed char value, bool isNegative)
{
	// Leading negative sign.
	if (value < 0 || isNegative) {
		// Normalize it so the second digit is easier to show.
		value = -value;
		putc('-');
	}

	putDecimal(value);
}

// Outputs a fixed-point value to serial, with as much precision as is meaningful.
// For now, I'm taking that to be two decimal places.
void putFixed(fixed16 f)
{
	// First, round to the nearest two decimals.
	// If this cascades to the upper digits, you want to know ahead of time.
	f += FIXED_ONE_HALF / 100;

	// Integer part.
	putSignedByteDecimal(fixedTruncToByte(f), f < 0);
	
	// Fractional part.
	putc('.');
	
	f = fixedFrac(f);  // Just the fractional part, positive.
	f *= 10;  // Move tenths into the units place.
	putnibble(FIXED_INTEGRAL(f));
	
	f -= fixedFloor(f);  // Remove the integral portion.
	f *= 10;  // Move hundredths into the units place.
	
	putnibble(FIXED_INTEGRAL(f));
}

// Returns the next character.
// Assumes there's something there!
byte getc(void)
{
	return pop(serialInput);
}

byte peekc(void)
{
	if (isEmpty(serialInput))
		return '\0';
	else
		return peek(serialInput);
}

// Reads a decimal number of the given type, and returns it.
// When it runs out of numeric digits, stops and returns what it's accumulated so far.
// If the input digits exceed the size of T, it just overflows.
// The following non-digit character is left in the input.
template <class T>
T readDecimal(void)
{
	T result = 0;
	
	while (isdigit(peekc()))
		result = result * 10 + (getc() - '0');
	
	return result;
}


//============================================================================
// Parameter and variable I/O

#define ONE_PARAM_LEN  12  // like "~P255=32767\n"

// Notes that we should re-send this variable later.
void EnqueueVariable(byte code)
{
    if (!contains(queuedVariables, code))
        push<MAX_VARIABLES>(queuedVariables, code);
}

// Returns true if there's room in the output buffer for one more parameter or variable.
inline bool CanWriteParam(void)
{
    return serialOutput.lenUsed + ONE_PARAM_LEN < SERIAL_BUFLEN;
}

// Sends the given parameter, and returns true if there was room.
// If there isn't room, do nothing and return false.
byte SendBBParameter(byte index)
{
	if (CanWriteParam()) {
        ensureNewline();
		puts("~P");
		putDecimal(index);
		putc('=');
		putDecimal(bbParams[index]);
        putNewline();
		
		return true;
	} else
		return false;
}

// Sends the given variable value, or queues it for later.
void SendBBReading(byte code, unsigned short value)
{
    if (CanWriteParam()) {
        ensureNewline();
		putc('~');
		putc(code);
		putc('=');
		putDecimal(value);
        putNewline();
	} else
        // If there's no room in the output buffer, queue this variable for sending later.
        EnqueueVariable(code);
}

void SendBBFixedReading(byte code, fixed16 value)
{
    if (CanWriteParam()) {
        ensureNewline();
		putc('~');
		putc(code);
		putc('=');
		putFixed(value);
        putNewline();
    } else
        // If there's no room in the output buffer, queue this variable for sending later.
        EnqueueVariable(code);
}

void ChangedBBParameter(byte index)
{
    if (AnyBBParamsQueued()) {
        // Some are already queued up, so expand the set we're sending to include this one.
        if (index < firstParamToReport)
            firstParamToReport = index;
        if (index > lastParamToReport)
            if (index >= bbParamCount)
              lastParamToReport = bbParamCount - 1;
            else
              lastParamToReport = index;
    } else {
        // Nothing's queued, so send only this one.
        firstParamToReport = lastParamToReport = index;
    }
}

void EnqueueBBParameters(void)
{
	firstParamToReport = 0;
	lastParamToReport = bbParamCount - 1;
}

byte AnyBBParamsQueued(void)
{
    return firstParamToReport <= lastParamToReport && firstParamToReport < bbParamCount;
}

void ClearBBOutput(void)
{
    clear(serialOutput);
    justSentNewline = false;
}


//============================================================================
// High-level processing

byte PollBasicBus(void)
{
    #ifdef LOGGING
    if (lastSerialError) {
        putc(lastSerialError);
        lastSerialError = '\0';
        putc('\n');
    }
    #endif

    // Process pending input commands from the master.
    byte result = ProcessBBCommands();

	// Push characters to transmit.
	if (pir1.TXIF && !isEmpty(serialOutput))
		txreg = pop(serialOutput);
		
	// If we're waiting to report some parameters, and there's room, send 'em out.
	if (AnyBBParamsQueued()) {
		if (SendBBParameter(firstParamToReport))
			// They'll fail often due to insufficient room in the output buffer, 
			// so just try again until there's enough room, then note that it's been sent.
			++firstParamToReport;
    } else if (!isEmpty(queuedVariables)) {
        if (CanWriteParam())
        // If we're waiting to send some variables, and there's room, send the next one.
            OnBBRequest(pop(queuedVariables));
    } else if (wildcardUnderway) {
        // We were responding to a wildcard request, but everything's now been sent.
        ensureNewline();
        puts("~.\n");  // "all done"
        wildcardUnderway = false;
    }

    return result;
}

byte ProcessBBCommands(void) {
    byte result = false;

	while (!isEmpty(serialInput)) {
		switch(serInState) {
		case IN_GARBAGE:
			if (getc() == '\n') {
				serInState = AT_LINE_START;
			}
			break;
			
		case AT_LINE_START:
			switch(getc()) {
			case '~':
				serInState = BETWEEN_COMMANDS;
				break;
            case '\r':
			case '\n':
				break;
			default:
				serInState = IN_GARBAGE;
				break;
			}
			break;

        case BETWEEN_COMMANDS:
            // Remove leading whitespace.
            // This is important to ensure the buffer gets cleared after the last command on a line.
            while (peekc() == ' ')
                getc();
            if (peekc() == '\n') {
                getc();
                serInState = AT_LINE_START;
            } else
                serInState = AT_COMMAND;
            break;

		case AT_COMMAND:
            // Wait till we have the whole line.
			if (contains(serialInput, '\n')) {
				if (length(serialInput) >= 3) {
					switch(getc()) {

					case 'S':  // Master status, S=<decimal byte>
						if (getc() == '=') {
							bbMasterStatus = readDecimal<byte>();
                            result = true;

							#ifdef LOGGING
                                ensureNewline();
								puts("S=");
								putDecimal(bbMasterStatus);
                                putNewline();
							#endif
						}
						serInState = IN_COMMAND_TAIL;
						break;
						
					case 'P':  // Short parameter, P<n>=<unsigned decimal short>
						if (isdigit(peekc())) {
							byte offset = readDecimal<byte>();
							if (offset < bbParamCount && getc() == '=') {
								unsigned short data = readDecimal<unsigned short>();
								bbParams[offset] = data;
								BBParameter(offset);
                                result = true;

								#ifdef LOGGING
                                    ensureNewline();
									putc('P');
									putDecimal(offset);
									putc('=');
									putDecimal(data);
									putNewline();
								#endif
							}
						} else if (peekc() == '!') {
                            // "P!": Reset all parameters.
                            #ifdef LOGGING
                            ensureNewline();
                            puts("P!");
                            putNewline();
                            #endif

                            ResetBBParams();
                            EnqueueBBParameters();
                        }
						serInState = IN_COMMAND_TAIL;
						break;

                    case '?':  // requests
                        switch (peekc()) {

                        case '=':  // Select slave, ?=<decimal byte> or ?=*
                            getc();
                            if (peekc() == '*') {
                                beSelectedSlave(true);
                                result = true;
                            } else if (isdigit(peekc())) {
                                byte targetID = readDecimal<byte>();
                                if (slaveID == targetID) {
                                    beSelectedSlave(true);
                                    result = true;
                                }
                            }
                            break;

                        case 'P':  // Request parameters
                            #ifdef LOGGING
                                ensureNewline();
                                puts("?P");
                                putNewline();
                            #endif

                            EnqueueBBParameters();
                            wildcardUnderway = true;
                            result = true;
                            break;

                        default:  // Request status or variable
                            #ifdef LOGGING
                                ensureNewline();
                                putc('?');
                                putc(peekc());
                                putNewline();
                            #endif

                            if (peekc() == '*')
                                wildcardUnderway = true;
                            OnBBRequest(peekc());
                            result = true;
                            break;
                        }
                        serInState = IN_COMMAND_TAIL;
                        break;
						
					case ' ':
						// Skip it and come around again.
						break;

                    case '\n':
                        serInState = AT_LINE_START;
                        break;
						
					default:
						// Unrecognized command.
						// Skip to the next space or newline.
                        while (!isEmpty(serialInput) && !isspace(peekc()))
                            getc();
						break;
					}
				} else {
					// We have a whole line, but it's not long enough to be any recognized command.
					// Skip it.
					serInState = IN_GARBAGE;
				}
			} else {
				// Don't have a newline yet.
				// If the buffer's full, there's nothing we can do, so discard and wait longer.
				if (isFull<SERIAL_BUFLEN>(serialInput))
					serInState = IN_GARBAGE;
				// Even if it's not full, we need to wait for more.
				return result;
			}
			break;
			
        case IN_COMMAND_TAIL:
            // Skip any unrecognized junk in a command we don't recognize,
            // or at the end of a command we have processed.
            while (!isEmpty(serialInput) && !isspace(peekc()))
                getc();
            serInState = BETWEEN_COMMANDS;
            break;
        
		default:
			// Unknown state somehow, so recover.
			serInState = IN_GARBAGE;
			break;
		}
	}

    return result;
}
