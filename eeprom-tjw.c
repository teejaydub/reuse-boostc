#include <system.h>
#include "eeprom-tjw.h"

void write_eeprom(char addr, char data)
{        //---- Write eeprom -----
	while (eecon1.WR);                // check completion of last write

	eeadr = addr;                        // set address register
	eedata = data;                        // set data register
	set_bit(eecon1, WREN );

	eecon2 = 0x55;
	eecon2 = 0xAA;
	set_bit(eecon1, WR);                // write command
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

