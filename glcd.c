/* glcd.c -- (c) chris kern , modified by Timothy Weber, http://timothyweber.org  */
// Ported to BoostC.

#include <system.h>
#include "glcd.h"

// Hard-coding the pin assignments here.
#define LCD_D_PORT  porta
#define LCD_D_TRIS  trisa

#define LCD_CTL_PORT  latc
#define LCD_CTL_TRIS  trisc
#define LCD_E_PIN  7
#define LCD_DI_PIN  6
#define LCD_RW_PIN  5
#define LCD_RST_PIN  2
#define LCD_CS1_PIN  0
#define LCD_CS2_PIN  1

#define LCD_E_MASK  (1 << LCD_E_PIN)
#define LCD_DI_MASK  (1 << LCD_DI_PIN)
#define LCD_RW_MASK  (1 << LCD_RW_PIN)
#define LCD_RST_MASK  (1 << LCD_RST_PIN)
#define LCD_CS1_MASK  (1 << LCD_CS1_PIN)
#define LCD_CS2_MASK  (1 << LCD_CS2_PIN)

#define LCD_CTL_MASK  (LCD_E_MASK | LCD_DI_MASK | LCD_RW_MASK | LCD_RST_MASK | LCD_CS1_MASK | LCD_CS2_MASK)

#define LCD_INST 0
#define LCD_DATA 1

#define LCD_YADDR(Y) (((Y) & 0x3F) | 0x40)
#define LCD_XADDR(X) (((X) & 0x07) | 0xB8)
#define LCD_POWERON(P) (((P)?1:0) | 0x3E)
#define LCD_STARTLINE(L) (((L) & 0x3F) | 0xC0)

// Instruction bits.
#define LCD_BUSY (1<<7)
#define LCD_OFF (1<<5)
#define LCD_RESET (1<<4)

#define CHARACTER_STUFF
#ifdef CHARACTER_STUFF
#include "glcd_font.h"
#endif

// These might need to be adjusted.  If they're too low you'll get garbled
// data; too high and updates will be slow.
// (That was from the inherited code; neither the datasheet nor experimentation
// suggest any problem when running at a 4 MHz clock rate.)
inline void delayShort(void)
{
	asm {
//		nop;
	}
}
inline void delayLong(void)
{
	asm {
//		nop;
	}
}

void glcd_write(byte chip, byte reg, byte data) {
	LCD_D_TRIS = 0;
	LCD_CTL_PORT.LCD_RW_PIN = 0;
	
	// Chip select is active low.
	LCD_CTL_PORT.LCD_CS1_PIN = chip;
	LCD_CTL_PORT.LCD_CS2_PIN = !chip;
	
	LCD_CTL_PORT.LCD_DI_PIN = reg;
	
	LCD_D_PORT = data;
	delayLong();
	LCD_CTL_PORT.LCD_E_PIN = 1;
	delayShort();
	LCD_CTL_PORT.LCD_E_PIN = 0;
}

byte glcd_read(byte chip, byte reg) {
	byte d;
  
	LCD_D_TRIS = 0xFF;
	LCD_CTL_PORT.LCD_RW_PIN = 1;

	// Chip select is active low.
	LCD_CTL_PORT.LCD_CS1_PIN = chip;
	LCD_CTL_PORT.LCD_CS2_PIN = !chip;
	
	LCD_CTL_PORT.LCD_DI_PIN = reg;
	
	delayLong();
	LCD_CTL_PORT.LCD_E_PIN = 1;
	delayShort();
  
	d = LCD_D_PORT;
	LCD_CTL_PORT.LCD_E_PIN = 0;
  
	return d;
}

void glcd_wait(byte chip) {
	while (glcd_read(chip, LCD_INST) & (LCD_BUSY | LCD_RESET))
		;
}

void glcd_write_wait(byte chip, byte reg, byte data) {
	glcd_write(chip, reg, data);
	glcd_wait(chip);
}

void glcd_init(void) {
	
	LCD_D_TRIS = 0xFF;  // Set the data lines to inputs for now.
	LCD_CTL_TRIS &= (~LCD_CTL_MASK);  // All the control lines are outputs.
	LCD_CTL_PORT &= ~(LCD_RW_MASK | LCD_DI_MASK | LCD_E_MASK | LCD_RST_MASK);  // R/W, D/I, E, RST low
	LCD_CTL_PORT |= (LCD_CS1_MASK | LCD_CS2_MASK);  // CS1, CS2 high
	
	delay_ms(10);
	LCD_CTL_PORT.LCD_RST_PIN = 1;
	delay_ms(50);	
	
	glcd_wait(0);
	glcd_wait(1);
	glcd_write_wait(0, LCD_INST, LCD_POWERON(1));
	glcd_write_wait(1, LCD_INST, LCD_POWERON(1));
	glcd_write_wait(0, LCD_INST, LCD_STARTLINE(0));
	glcd_write_wait(1, LCD_INST, LCD_STARTLINE(0));
	
	glcd_clear(0xAA);
}


#define CACHE_EMPTY 255 
static byte cache_chip = CACHE_EMPTY;
static byte cache_x;
static byte cache_y;
static byte cache_d;

void glcd_flush(void) {
	if (cache_chip == CACHE_EMPTY) 
		return;
		
	glcd_write_wait(cache_chip, LCD_INST, LCD_YADDR(cache_y));
	glcd_write_wait(cache_chip, LCD_INST, LCD_XADDR(cache_x));
	glcd_write_wait(cache_chip, LCD_DATA, cache_d);
	
	cache_chip = CACHE_EMPTY;
}

void glcd_load(byte chip, byte x, byte y) {
	if (cache_chip == chip &&
		cache_x == x &&
		cache_y == y) 
	{
		// Already have it.
		return;
	}
	
	// Nope, don't have it, so write out the last one.
	glcd_flush();
	
	// And load the new one.
	cache_x = x;
	cache_y = y;
	cache_chip = chip;
	
	glcd_write_wait(cache_chip, LCD_INST, LCD_YADDR(cache_y));
	glcd_write_wait(cache_chip, LCD_INST, LCD_XADDR(cache_x));
	
	// the lcd has a read pipeline; each read gets you the last's result.
	glcd_read(cache_chip, LCD_DATA);
	cache_d = glcd_read(cache_chip, LCD_DATA);
}

void glcd_clear(byte data) {
	byte x, y;  
	
	for (x = 0; x < 8; ++x) {
		glcd_write_wait(0, LCD_INST, LCD_YADDR(0));
		glcd_write_wait(0, LCD_INST, LCD_XADDR(x));
		for (y = 0; y < 64; ++y) { 
			glcd_write_wait(0, LCD_DATA, data);
		}
	}
	
	for (x = 0; x < 8; ++x) {
		glcd_write_wait(1, LCD_INST, LCD_YADDR(0));
		glcd_write_wait(1, LCD_INST, LCD_XADDR(x));
		for (y = 0; y < 64; ++y) { 
			glcd_write_wait(1, LCD_DATA, data);
		}
	}
	
	cache_chip = CACHE_EMPTY;
}

void glcd_setbit(byte x, byte y, byte v) {
	byte lcd_chip = (x & 0x40) ? 1 : 0;
	byte lcd_x = (y & 0x3F) >> 3;
	byte lcd_y = (x & 0x3F);
	byte lcd_bit = y & 0x07;
	glcd_load(lcd_chip, lcd_x, lcd_y);
	if (v) {
		cache_d |= 1 << lcd_bit;
	} else {
		cache_d &= ~(1 << lcd_bit);
	}
}

#ifdef CHARACTER_STUFF
static byte cursor_x;
static byte cursor_y;

void glcd_set_cursor(byte row, byte col) {
  cursor_x = 6 * col;
  cursor_y = 8 * row;
}

void glcd_putch(byte ch) {
  byte x, y;
  byte offset;
  byte b;
  if (ch < 32) ch = 32;
  if (ch > 128) ch = 128;
  
  // Get the offset in the character page.
  // Each page contains 32 characters.
  // Because of ROM character constant constraints, no page can be more than 256 characters,
  // and they have to be addressed by constant reference.
  offset = 5 * (ch % 32);
  
  for(x = 0; x < 6; ++x) {
	if (ch >= 96)
		b = font_5x7_data96[offset + x];
	else if (ch >= 64)
		b = font_5x7_data64[offset + x];
	else if (ch >= 32)
		b = font_5x7_data32[offset + x];
    
    for(y = 0; y < 8; ++y) {
      if (x < 5 && y < 7) {
        glcd_setbit(cursor_x + x, cursor_y +y, b & (1<<y));
      } else {
        glcd_setbit(cursor_x + x, cursor_y +y, 0);
      }
    }
  }
  cursor_x += 6;
  glcd_flush();
}

void glcd_puts(const char* str) {
  while (*str)
    glcd_putch(*str++);
}

#endif