/* spi.h

	SPI interface, supporting bit-bang serial on any combination of GPIO pins.
	(Useful if your MSSP peripheral is already in use for other things, but you
	can find three or four other free pins.)
	
	Master mode works pretty fully.
	
	Slave mode requires Timer 1 (without interrupt), and exclusive use of the SPI bus.
	That is, there can't be any other slaves.

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
bool messageRestarted(void);


#endif

#endif
// __SPI_TJW_H