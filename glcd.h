/* Graphic LCD code.
   For SparkFun's LCD-00710, a 128 x 64 module with an 8-bit parellel interface.
   Based on http://jormungand.net/projects/devices/avrlcd/
*/

#ifndef __GLCD_H
#define __GLCD_H  1

#include "types-tjw.h"


/* glcd_init(): initialize lcd port directions
 *             enable both display chips
 *             set display line to zero */
void glcd_init(void);

/* glcd_clear(): clear the entire lcd to the given value */
void glcd_clear(byte data);

/* glcd_setbit(): sets the bit at (x,y) to value v
 *               may not appear until lcd_flush() is called. */
void glcd_setbit(byte x, byte y, byte v);

/* glcd_flush(): write pending bits out to display */
void glcd_flush(void);

/* glcd_set_cursor(): set the cursor position to the given 6x8 cell */
void glcd_set_cursor(byte row, byte col);

/* glcd_putch(): write an ascii character at the cursor and advance */
void glcd_putch(byte ch);

/* glcd_puts(): write a string at the cursor, and advance
	Supports \n for newline.
 */ 
void glcd_puts(const char* str);

#endif