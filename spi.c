/* spi.c
    Copyright (c) 2017 by Timothy J. Weber, tw@timothyweber.org.
*/

#define USE_SHADOW_REGS

#define IN_SPI_TJW_H

#include <system.h>

#include "shadowRegs.h"
#include "spi.h"

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

	spiLastSelectCount = tmr1l;
	
	// Set up an interrupt when the CK pin changes, and read MOSI that time.
	SPI_SDI_TRIS.SPI_SDI_PIN = 1;
	clearSpiReceive();
	option_reg.INTEDG = SPI_CLOCK_EDGE;
	intcon.INTE = 1;
	
	spiLenUsed = 0;
	clearSpiReceive();
	
	// Also write the sent data out the MISO pin in sync with INT.
	SPI_SDO_TRIS.SPI_SDO_PIN = 0;
	SPI_SDO_PORT.SPI_SDO_PIN = 0;

	spiSend = 0;
	spiSendLen = 0;
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
		
		#ifdef SPI_DELAY_US
		for (byte j = SPI_DELAY_US / 6; j; --j)
			nop();
		#endif
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

		#ifdef SPI_DELAY_US
		for (byte j = SPI_DELAY_US / 6; j; --j)
			nop();
		#endif
		
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
