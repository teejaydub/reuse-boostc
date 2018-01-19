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
// #define LOGGING  1

#define SERIAL_BUFLEN  48

byte serialOutputBuffer[SERIAL_BUFLEN];
byte serialInputBuffer[SERIAL_BUFLEN];

ByteBuf serialInput;
ByteBuf serialOutput;

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

typedef enum {
	IN_GARBAGE,  // startup or in a line that we should ignore
	IN_LINE_START,  // just saw a newline
	IN_COMMAND,  // just saw a command initializer
} SerInState;
SerInState serInState;

// Forward declarations
void ProcessBBCommands(void);

#ifdef _PIC18F45K22
 #define RX_TRIS  trisc
 #define RX_ANSEL  anselc
 #define RX_PIN  7
#else
 #error Need to define the RX port pin for this chip.
#endif


void InitializeBasicBus(byte paramCount, unsigned short* params)
{
	// Set for 9600 baud.
	txsta.BRGH = 1;  // Set baud rate to...
	spbrg = 25;  // ... 9600 baud
	rcsta.SPEN = 1;  // Enable serial port.
	txsta.TXEN = 1;  // Enable transmission.  (When BB selection is implemented, don't start until we've been selected.)
	rcsta.CREN = 1;  // Enable serial reception.

	pie1.RCIE = 1;  // Interrupts on reception.

    RX_TRIS.RX_PIN = 1;
    RX_ANSEL.RX_PIN = 0;

	serInState = IN_LINE_START;

    bbMasterStatus = ' ';  // A convenient default status.
	
	init(serialInput, serialInputBuffer);
	init(serialOutput, serialOutputBuffer);
	
	bbParamCount = paramCount;
	bbParams = params;
}

// Enqueues this character for sending.
// Drops it if the queue is full.
void putc(char c)
{
	push<SERIAL_BUFLEN>(serialOutput, c);
    justSentNewline = (c == '\n');
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

// Returns the next character, or \0 if there's none available.
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

template <class T>
T readDecimal(void)
{
	T result = 0;
	
	while (isdigit(peekc()))
		result = result * 10 + (getc() - '0');
	
	return result;
}

#define ONE_PARAM_LEN  12  // like "~P255=32767\n"

// Sends the given parameter, and returns true if there was room.
// If there isn't room, do nothing and return flase.
byte SendBBParameter(byte index)
{
	if (serialOutput.lenUsed + ONE_PARAM_LEN < SERIAL_BUFLEN) {
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

byte SendBBReading(byte code, unsigned short value)
{
	if (serialOutput.lenUsed + ONE_PARAM_LEN < SERIAL_BUFLEN) {
        ensureNewline();
		putc('~');
		putc(code);
		putc('=');
		putDecimal(value);
        putNewline();
		
		return true;
	} else
		return false;
}

byte SendBBFixedReading(byte code, fixed16 value)
{
	if (serialOutput.lenUsed + ONE_PARAM_LEN < SERIAL_BUFLEN) {
        ensureNewline();
		putc('~');
		putc(code);
		putc('=');
		putFixed(value);
        putNewline();
		
		return true;
	} else
		return false;
}

void ChangedBBParameter(byte index)
{
	if (index < firstParamToReport)
		firstParamToReport = index;
	if (index > lastParamToReport)
        if (index >= bbParamCount)
          lastParamToReport = bbParamCount - 1;
        else
		  lastParamToReport = index;
}

void EnqueueBBParameters(void)
{
	firstParamToReport = 0;
	lastParamToReport = bbParamCount - 1;
}

byte AnyBBParamsQueued(void)
{
    return firstParamToReport <= lastParamToReport;
}

void ClearBBOutput(void)
{
    clear(serialOutput);
    justSentNewline = false;
}

byte BasicBusISR(void)
{
    if (pir1.RCIF) {
        byte c;
        if (rcsta.FERR) {
            c = rcreg;
            #ifdef LOGGING
            putc('#');
            putNewline();
            #endif
        } else {
            c = rcreg;
            if (rcsta.OERR) {
                // Overrun error, meaning we missed some characters - restart reception.
                rcsta.CREN = 0;
                clear(serialInput);
                rcsta.CREN = 1;
                #ifdef LOGGING
                putc('!');
                putc(c);
                putNewline();
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

void PollBasicBus(void)
{
	// Push characters to transmit.
	if (pir1.TXIF && !isEmpty(serialOutput))
		txreg = pop(serialOutput);
		
	// If we're waiting to report some parameters, and there's room, send 'em out.
	if (firstParamToReport <= lastParamToReport && firstParamToReport < bbParamCount)
		if (SendBBParameter(firstParamToReport))
			// They'll fail often due to insufficient room in the output buffer, 
			// so just try again until there's enough room, then note that it's been sent.
			++firstParamToReport;

    // Process pending commands.
    ProcessBBCommands();
}

void ProcessBBCommands(void) {
	while (!isEmpty(serialInput)) {
		switch(serInState) {
		case IN_GARBAGE:
			if (getc() == '\n') {
				serInState = IN_LINE_START;
			}
			break;
			
		case IN_LINE_START:
			switch(getc()) {
			case '~':
				serInState = IN_COMMAND;
				break;
            case '\r':
			case '\n':
				break;
			default:
				serInState = IN_GARBAGE;
				break;
			}
			break;
		
		case IN_COMMAND:
			if (length(serialInput) >= 3 && contains(serialInput, '\n')) {  // wait till we have the whole line.
				if (length(serialInput) >= 3) {
					switch(getc()) {
					case 'S':  // radio status, S=<decimal byte>
						if (getc() == '=') {
							bbMasterStatus = readDecimal<byte>();
							#ifdef LOGGING
                                ensureNewline();
								puts("S=");
								putDecimal(bbMasterStatus);
                                putNewline();
							#endif
						} else
							// Not S=, so skip to the next line.
							serInState = IN_GARBAGE;
						break;
						
					case 'P':  // Short parameter, P<n>=<unsigned decimal short>
						if (isdigit(peekc())) {
							byte offset = readDecimal<byte>();
							if (offset < bbParamCount && getc() == '=') {
								unsigned short data = readDecimal<unsigned short>();
								bbParams[offset] = data;
								BBParameter(offset);

								#ifdef LOGGING
                                    ensureNewline();
									putc('P');
									putDecimal(offset);
									putc('=');
									putDecimal(data);
									putNewline();
								#endif
							} else
								// Not P=x=, so skip.
								serInState = IN_GARBAGE;
						} else if (peekc() == '?') {
							// Output all parameters.
                            #ifdef LOGGING
                                ensureNewline();
                                putc('!');
                                putNewline();
                            #endif
							EnqueueBBParameters();
						} else
							// Not P=, so skip.
							serInState = IN_GARBAGE;
						break;				
						
					case ' ':
						// Skip it and come around again.
						break;
						
					default:
						// Unrecognized command.
						// Ignore it and move on.
						serInState = IN_GARBAGE;
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
				// It's not full, so just wait for more.
				return;
			}
			break;
			
		default:
			// Unknown state somehow, so recover.
			serInState = IN_GARBAGE;
			break;
		}
	}
}
