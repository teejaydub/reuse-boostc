// LCDMenu.c

#include <system.h>

#include "lcd.h"
#include "LCDMenu.h"
#include "LCDMenuConsts.h"

rom char* okChar = {0x1c,0x14,0x1c,0x0,0x5,0x6,0x5};
rom char* ellipsisChar = {0x0,0x0,0x0,0x0,0x0,0x0,0x15};


void DefineLMCustomChars(void)
{
	lcd_set_char(OK_CHAR, okChar);
	lcd_set_char(ELLIPSIS_CHAR, ellipsisChar);
}

/* Displays menus like this:

	Given: "New game...|Statistics...|Change sides|Game...|Sound...|Auto-off..."
	Shows:
		_N_SGSA?
		New game...
	
	where _ shows the position of the cursor.
	
	As the directional buttons are pressed, a different initial is highlighted
	on the first row, and that item's full description is shown 
	on the second row.
*/
byte GetMenuChoice(rom char* menuItems)
{	
	// Analyze the input.
	byte i;
	byte titleLength = 0;  // Nonzero iff there's a title.
	byte maxItemIndex = 0;
	for (i = 0; menuItems[i]; i++) {
		switch (menuItems[i]) {
		case ':':
			// Note the length of the title.
			titleLength = i + 1;
			maxItemIndex = 0;
			break;
			
		case '|':
			// Count the items.
			++maxItemIndex;
			break;
		}
	}
	
	// Abort if there were no items.
	if (maxItemIndex == 0)
		return 0;
	
	// Clear the screen.
	lcd_clear();
	
	// Show the title, if it's given.
	lcd_datamode();
	for (i = 0; i < titleLength; i++)
		lcd_write(menuItems[i]);
	lcd_write(' ');
	
	// Display the list of choice initials.
	byte choiceListX;
	if (titleLength)
		choiceListX = titleLength + 1;
	else
		choiceListX = 0;
	
	for (; menuItems[i]; i++)
		if (i == 0 || menuItems[i - 1] == '|' || menuItems[i - 1] == ':')
			lcd_write(menuItems[i]);
	
	
	// Start selecting.
	byte currentChoice = 0;
	while (1) {
		// Index to the selected item's text.
		byte itemIndex = 0;
		for (i = titleLength; itemIndex < currentChoice; i++)
			if (menuItems[i] == '|')
				++itemIndex;
			
		// Display the current item's text on the second line.
		lcd_gotoxy(0, 1);
		lcd_datamode();
		byte x;
		for (x = 0; menuItems[i] && menuItems[i] != '|'; i++, x++)
			lcd_write(menuItems[i]);
			
		// Fill to the end of the line.
		for (; x < DISPLAY_WIDTH; x++)
			lcd_write(' ');

		// Move the cursor to the current choice initial.		
		lcd_gotoxy(choiceListX + currentChoice, 0);
	
		// Get and process the next key.
		switch (kb_getc()) {
		case LEFT_KEY:
			if (currentChoice == 0)
				currentChoice = maxItemIndex;
			else
				--currentChoice;
			break;
		
		case RIGHT_KEY:
			if (currentChoice == maxItemIndex)
				currentChoice = 0;
			else
				++currentChoice;
			break;
		
		case ENTER_KEY:
			return currentChoice;
			break;
		}
	}
}
