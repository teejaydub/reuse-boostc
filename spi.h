/* spi.h

	SPI interface, supporting bit-bang serial on any combination of GPIO pins.
	(Useful if your MSSP peripheral is already in use for other things, but you
	can find three other free pins.)

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

// Selects the SPI slave on the given (single) Slave Select line.
void spi_select(void);

// Deselects the SPI slave on SS.
void spi_deselect(void);

// Sends the given byte.
void spi_write(byte d);

// Receives a byte and returns it.
byte spi_read(void);

#endif
// __SPI_TJW_H