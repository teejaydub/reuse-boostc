/* spi.c

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

#define USE_SHADOW_REGS

#define IN_SPI_TJW_H

#include <system.h>

#include "shadowRegs.h"
#include "spi.h"

byte lastSelectCount;

#if SPI_MASTER
inline void set_spi_data(byte d)
{
	#ifdef USE_SHADOW_REGS
	SET_SHADOW_BIT(SPI_SDO_PORT, SPI_SDO_SHADOW, SPI_SDO_PIN, d != 0);
	#else
	SPI_SDO_PORT.SPI_SDO_PIN = (d != 0);
	#endif
}
#endif

#if SPI_MASTER
inline void set_spi_clock(byte c)
{
	#ifdef USE_SHADOW_REGS
	SET_SHADOW_BIT(SPI_SCK_PORT, SPI_SCK_SHADOW, SPI_SCK_PIN, c);
	#else
	SPI_SCK_PORT.SPI_SCK_PIN = c;
	#endif
}
#endif

#ifdef USE_SHADOW_REGS
 #define SET_SPI_CLOCK(c)  SET_SHADOW_BIT(SPI_SCK_PORT, SPI_SCK_SHADOW, SPI_SCK_PIN, c)
#else
 #define SET_SPI_CLOCK(c)  SPI_SCK_PORT.SPI_SCK_PIN = c
#endif

#if SPI_MASTER
void spi_select(void)
{
	// Assumes active-low slave select.
	SET_SHADOW_BIT(SPI_SS_PORT, SPI_SS_SHADOW, SPI_SS_PIN, 0);
}

void spi_deselect(void)
{
	// Assumes active-low slave select.
	SET_SHADOW_BIT(SPI_SS_PORT, SPI_SS_SHADOW, SPI_SS_PIN, 1);
}
#endif

void spi_init(void)
{
	#if SPI_MASTER
	SET_SPI_CLOCK(SPI_CLOCK_IDLE);
	set_spi_data(0);

	SPI_SCK_TRIS.SPI_SCK_PIN = 0;  // set SCK for output.
	SPI_SDO_TRIS.SPI_SDO_PIN = 0;  // set SDO for output
	SPI_SDI_TRIS.SPI_SDI_PIN = 1;  // set SDI for input
	SPI_SS_TRIS.SPI_SS_PIN = 0;  // set SS for output
	#else
	// Assign Timer 1 to count cycles on the CS pin.
	// If it's different, we were selected, so reset.
	t1con.TMR1CS = 1;
	t1con.NOT_T1SYNC = 1;  // unsynchronized is probably better.
	t1con.TMR1ON = 1;  // Now counting cycles on CS.
	lastSelectCount = tmr1l;
	#endif
}

// Bit-bang serial blocking write of d, as fast as possible (which won't be very fast by SPI standards).
void spi_write(byte d)
{
	#if SPI_MASTER
	for (byte i = 0; i != 8; i++) {
		SET_SPI_CLOCK(~SPI_CLOCK_EDGE);
		
		set_spi_data(d & 0x80);
		
		SET_SPI_CLOCK(SPI_CLOCK_EDGE);
		d <<= 1;
	}
	SET_SPI_CLOCK(SPI_CLOCK_IDLE);
	#else
	
	#endif
}

byte spi_read(void)
{
	#if SPI_MASTER
	byte result = 0;
	for (byte i = 0; i != 8; i++) {
		SET_SPI_CLOCK(SPI_CLOCK_EDGE);
		
		// Shift the new data in through the low bit.
		result <<= 1;
		if (SPI_SDI_PORT.SPI_SDI_PIN)
			result |= 1;
			
		SET_SPI_CLOCK(~SPI_CLOCK_EDGE);
	}
	SET_SPI_CLOCK(SPI_CLOCK_IDLE);
	return result;
	#else
	
	#endif
}

#if !SPI_MASTER
bool messageRestarted(void)
{
	byte nextCount = tmr1l;
	bool result = (lastSelectCount != nextCount);
	lastSelectCount = nextCount;
	return result;
}
#endif