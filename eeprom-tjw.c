/* eeprom-tjw.c
    Copyright (c) 2007 by Timothy J. Weber, tw@timothyweber.org.

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

#include <system.h>
#include "eeprom-tjw.h"

// Chips that can read and write their program ROM call it 'EEARDL' instead of 'EEADR'.
// Compensate.
#ifdef EEADRL
 // (If the header doesn't compensate already.)
 #ifndef EEADR
  volatile char eeadr@EEADRL;
  volatile char eedata@EEDATL;
 #endif
#endif

char read_eeprom(char addr)
{
	#ifdef _PIC18F2620
	clear_bit(eecon1, EEPGD);
	clear_bit(eecon1, CFGS);
	eeadrh = 0;
	#endif
	eeadr = addr;
	set_bit(eecon1, RD);
	return eedata;
}

void write_eeprom(char addr, char data)
{        //---- Write eeprom -----
	#ifdef _PIC18F2620
	clear_bit(eecon1, EEPGD);
	clear_bit(eecon1, CFGS);
	eeadrh = 0;
	#endif

	while (eecon1.WR);                // check completion of last write

	eeadr = addr;                        // set address register
	eedata = data;                        // set data register
	set_bit(eecon1, WREN);

	// Disable interrupts while writing.
	intcon.GIE = 0;
	
	eecon2 = 0x55;
	eecon2 = 0xAA;
	set_bit(eecon1, WR);                // write command
	intcon.GIE = 1;

	clear_bit(eecon1, WREN);        // inhibit further writing
}

void read_eeprom_block(char addr, char* buf, unsigned char len)
{
	while (len--)
		*buf++ = read_eeprom(addr++);
}

void write_eeprom_block(char addr, char* buf, unsigned char len)
{
	while (len--)
		write_eeprom(addr++, *buf++);
}

