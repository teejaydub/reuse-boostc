/* spi.h
    Copyright (c) 2010, 2017 by Timothy J. Weber, tw@timothyweber.org.

	SPI interface, supporting bit-bang serial on any combination of GPIO pins.
	(Useful if your MSSP peripheral is already in use for other things, but you
	can find three or four other free pins.)
	
	Master mode works pretty fully.
	
	Slave mode requires Timer 1 (without interrupt), and exclusive use of the SPI bus.
	That is, there can't be any other slaves.
	It also assumes that the queue is statically sized to the maximum length
	of a whole conversation between Chip Select events.
*/

#ifndef __SPI_TJW_H
#define __SPI_TJW_H

#include <types-tjw.h>
#include <string.h>

// Pin assignments, etc.
#include "spi-consts.h"

#ifdef IN_SPI_TJW_H
 #define SPI_EXTERN
#else
 #define SPI_EXTERN  extern
#endif

#if !SPI_MASTER
  SPI_EXTERN byte spiQueue[SPI_QUEUE_LEN];
  SPI_EXTERN byte spiLastSelectCount;
  SPI_EXTERN byte spiReceive;
  SPI_EXTERN byte spiSend;
  SPI_EXTERN byte spiBitsToGo;
  SPI_EXTERN byte spiLenUsed;  // tail of the queue
  SPI_EXTERN byte spiRead;  // head of the queue
  
  SPI_EXTERN byte* spiSendBuf;
  SPI_EXTERN byte spiSendLen;  // if zero, nothing left to send - so send zeros.
#endif

// Initializes the SPI interface on the specified pins.
void spi_init(void);

// Sends the given byte, in Master mode.
// In Slave mode, sets this as the next byte to be sent.
void spi_write(byte d);

#if SPI_MASTER

// Master interface: bit-banged synchronously.

// Selects the SPI slave on the given (single) Slave Select line.
void spi_select(void);

// Deselects the SPI slave on SS.
void spi_deselect(void);

// Receives a byte and returns it.
byte spi_read(void);

#else

// Slave interface: requires an interrupt.

// Returns true if the slave as been reselected since the last read.
// Clears the condition if it was set.
inline bool messageRestarted(void)
{
	byte nextCount = tmr1l;
	bool result = (spiLastSelectCount != nextCount);
	spiLastSelectCount = nextCount;
	return result;
}

inline void clearSpiReceive(void)
{
	// spiReceive = 0;  // you'd think you'd need this,
	// but this next line takes care of it - because after 8 shift-ins,
	// the original value is irrelevant:
	spiBitsToGo = 8;
}

// Call this in the interrupt handler.
inline void spiInterrupt(void)
{
	if (intcon.INTF) {
		intcon.INTF = 0;
		
		// Receive a bit
		spiReceive = (spiReceive << 1) | SPI_SDI_PORT.SPI_SDI_PIN;
		
		// Send a bit
		SPI_SDO_PORT.SPI_SDO_PIN = (spiSend & 0x80)? 1: 0;
		spiSend <<= 1;
		
		// Handle end of bytes.
		if (--spiBitsToGo == 0) {
			if (spiLenUsed <= SPI_QUEUE_LEN) {
				spiQueue[spiLenUsed++] = spiReceive;
			}
			if (spiSendLen) {
				spiSend = *(spiSendBuf++);
				--spiSendLen;				
			}
			spiBitsToGo = 8;
		}
	}
}

// Also call this often.
// Returns the number of characters waiting.
inline byte spiPoll(void)
{
	if (messageRestarted()) {
		clearSpiReceive();
		spiLenUsed = 0;
		spiRead = 0;
		spiSend = 0;
		spiSendLen = 0;
	}

	return spiLenUsed - spiRead;
}

// Returns the next character from the queue.
inline byte spiPeek(void)
{
	return spiQueue[spiRead];
}

// Returns the character after the next one from the queue.
inline byte spiPeekNext(void)
{
	return spiQueue[spiRead + 1];
}

// Returns and discards the next character from the input queue.
inline byte spiRead(void)
{
	byte result = spiPeek();
	if (++spiRead > spiLenUsed)
		spiRead = spiLenUsed;
	return result;
}

// Discards the given number of characters from the input queue.
inline void spiSkip(byte count)
{
	spiRead += count;
	if (spiRead > spiLenUsed)
		spiRead = spiLenUsed;
}

// Writes this byte immediately.
inline void spiWrite(byte data)
{
	spiSendLen = 0;
	spiSend = data;
}

// Queues up the given data buffer for writing out.
inline void spiWrite(byte* data, byte count)
{
	if (spiBitsToGo == 8 && count) {
		// This is the usual case, if this message is processed while the bus is quiet.
		// Just set the next byte and continue on later with the bytes after that.
		spiSend = *data;
		spiSendBuf = data + 1;
		spiSendLen = count - 1;
	} else {
		// This would be unusual, and might be a sign of other problems,
		// but if we're in the middle of outputting a byte,
		// save the whole buffer for later.
		// (Or, count is zero, which means this has no effect.)
		spiSendBuf = data;
		spiSendLen = count;
	}
}

inline void spiReadBuf(byte* data, byte count)
{
	strncpy(data, spiQueue + spiRead, count);
	spiSkip(count);
}

#endif

#endif
// __SPI_TJW_H