/* buttons.h
    Copyright (c) 2007-2008 by Timothy J. Weber, tw@timothyweber.org.

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

/*	
	Generic module for handling momentary pushbuttons connected each to a port pin.
	Handles debouncing.
	Works well with uiTime, but can also function without it - requires no timers.
*/

#ifndef __BUTTONS_H
#define __BUTTONS_H

#ifdef IN_BUTTONS
 #define BUTTONS_EXTERN
#else
 #define BUTTONS_EXTERN  extern
#endif


#include "types-tjw.h"

#define NO_BTN  0xFF

#include "buttons-consts.h"
	

// Global to indicate button presses.
// Buttons are represented by their bit masks.
// Clear the appropriate bit when the button press has been handled.
BUTTONS_EXTERN byte buttonsPressed;


// Call this to set up the port pins.
// If you are using built-in weak pull-ups, you have to take care of those yourself (for now).
void InitButtons(void);

// Call this every so often to update the button status, often in an interrupt routine.
// (We define "so often" as a "button press period.")
// About every ms is usually good, and dovetails well with uiTime.c.
// Sets a bit in buttonsPressed whenever a button is pressed and stable long enough.
void CheckButtons(void);

// Returns the *_BTN index (from buttons-consts.h) corresponding to the next key pressed, 
// or NO_BTN if none was pressed.
// Basically pulls out the bits of buttonsPressed and zeroes them out.
// Prefers low-numbered buttons first, if multiple buttons are pressed.
byte GetButton(void);


#endif