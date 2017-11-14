/* spi.h

	SPI interface, supporting bit-bang serial on any combination of GPIO pins.
	(Useful if your MSSP peripheral is already in use for other things, but you
	can find three or four other free pins.)
	
	Master mode works pretty fully.
	
	Slave mode requires Timer 1 (without interrupt), and exclusive use of the SPI bus.
	That is, there can't be any other slaves.
	It also uses the queue module for a circular input buffer.
	If the queue fills up, older bytes received will be lost.

    Copyright (c) 2010, 2017 by Timothy J. Weber, tw@timothyweber.org.

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

#ifndef __SPI_TJW_H
#define __SPI_TJW_H

#include <types-tjw.h>

// Pin assignments, etc.
#include "spi-consts.h"

#ifdef IN_SPI_TJW_H
 #define SPI_EXTERN
#else
 #define SPI_EXTERN  extern
#endif

#if !SPI_MASTER
 #include "queue.h"
#endif

#if !MASTER
  SPI_EXTERN byte spiLastSelectCount;
  SPI_EXTERN byte spiReceive;
  SPI_EXTERN byte spiBitsToGo;
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
	spiReceive = 0;
	spiBitsToGo = 8;
}

// Call this in the interrupt handler.
inline void spiInterrupt(void)
{
	if (intcon.INTF) {
		intcon.INTF = 0;
		spiReceive = (spiReceive << 1) | SPI_SDI_PORT.SPI_SDI_PIN;
		if (messageRestarted())
			spiBitsToGo = 7;
		else if (--spiBitsToGo == 0) {
			PrePushQueue();
			*QueueTail() = spiReceive;
			PushQueue();
			
			spiBitsToGo = 8;
		}
	}
}

// Also call this often to catch unusual resets.
// Returns the number of characters waiting.
inline byte spiPoll(void)
{
	return queueCount;
}

// Returns the next character from the queue.
inline byte spiPeek(void)
{
	return *QueueHead();
}

// Returns the character after the next one from the queue.
inline byte spiPeekNext(void)
{
	return *QueueNextHead();
}

// Returns and discards the next character from the input queue.
inline byte spiRead(void)
{
	byte result = *QueueHead();
	
	PopQueue();
	return result;
}

// Discards the given number of characters from the input queue.
inline void spiSkip(byte count)
{
	while (count--)
		PopQueue();
}

#endif

#endif
// __SPI_TJW_H