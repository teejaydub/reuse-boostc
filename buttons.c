/* buttons.c
    Copyright (c) 2007-2014 by Timothy J. Weber, tw@timothyweber.org.
*/

#include <system.h>
#include <memory.h>

#define IN_BUTTONS
#include "buttons.h"

#include "math-tjw.h"


#if NUM_BTNS > 1
static byte downs[NUM_BTNS];
#else
static byte downs;
#endif


void CheckButtons(void)
{
	// We only care about button presses, not releases.
	// So, just keep track of how many consecutive "down" events we've seen for each button.

	#if NUM_BTNS > 1
	byte* down = &downs[0];
	byte mask = 1 << FIRST_BTN;
	
	byte i;
	for (i = FIRST_BTN; i <= LAST_BTN; i++) {
		if (IS_BTN_DOWN_MASK(mask)) {
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
	#else
	if (IS_THE_BTN_DOWN) {
		if (downs == MIN_DOWNS)
			// We have a press.
			buttonsPressed.FIRST_BTN = 1;
		
		if (downs <= MIN_DOWNS)
			++(downs);
	} else
		downs = 0;
	#endif
}

void InitButtons(void)
{
	BUTTON_TRIS |= ALL_BTNS_MASK;
	buttonsPressed = 0;
	
	#if NUM_BTNS > 1
	memset(downs, 0, sizeof(downs));
	#else
	downs = 0;
	#endif
}

byte GetButton(void)
{
	#if NUM_BTNS > 1
	return clearLowestSetBit<byte>(buttonsPressed, LAST_BTN);
	#else
	if (buttonsPressed.FIRST_BTN) {
		buttonsPressed = 0;
		return FIRST_BTN;
	}
	#endif
	
	return NO_BTN;
}