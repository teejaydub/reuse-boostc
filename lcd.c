/* lcd.c
    Copyright (c) 2007, 2009 by Timothy J. Weber, tw@timothyweber.org.

    Stores data for the BoostC lcd_driver module, to let it be used from multiple modules.
    Requires constants to be set in lcd_consts.h.
*/

#include <system.h>

#define IN_LCD

#include "lcd.h"
#include "types-tjw.h"


void lcd_set_char(unsigned char c, rom char* s)
{
	// Point to the right character position.
	lcd_function(set_cg_ram + (c << 3));
	
	// Write out the characters.
	lcd_datamode();
	byte i;
	for (i = 0; i < 8; i++)
		lcd_write(s[i]);
}