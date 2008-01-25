// buttons.c - see buttons.h

#include <system.h>

#define IN_BUTTONS
#include "buttons.h"


#define NUM_BTNS  (LAST_BTN - FIRST_BTN + 1)


void CheckButtons(void)
{
	// We only care about button presses, not releases.
	// So, just keep track of how many consecutive "down" events we've seen for each button.

	static char downs[NUM_BTNS];

	byte i;
	byte* down = &downs[0];
	byte mask = 1 << FIRST_BTN;
	
	for (i = FIRST_BTN; i <= LAST_BTN; i++) {
		if (!(BUTTON_PORT & mask)) {
			if (*down == MIN_DOWNS)
				// We have a press.
				buttonsPressed |= mask;
			
			if (*down <= MIN_DOWNS)
				++(*down);
		} else
			*down = 0;
	
		// Move to the next array element.
		mask <<= 1;
		++down;
	}
}

void InitButtons(void)
{
	BUTTON_TRIS |= ALL_BTNS_MASK;
	buttonsPressed = 0;
}

byte GetButton(void)
{
	byte i;
	byte mask = (1 << FIRST_BTN);
	for (i = FIRST_BTN; i <= LAST_BTN; i++) {
		if (buttonsPressed & mask) {
			clear_bit(buttonsPressed, i);
			return i;
		}
		mask <<= 1;
	}
	
	return NO_BTN;
}