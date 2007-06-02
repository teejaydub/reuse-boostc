// User interface system for a small alphanumeric LCD panel.
// Designed for a 2x16 panel, but should scale.
// Requires the lcd module, which also requires lcd_consts.h,
// and LCDUIConsts.h.

#ifndef _LCDUI_H
#define _LCDUI_H

#include "types-tjw.h"


// Some custom characters defined here.
// The code points, defined as character constants:
#define OK_CHAR  '\001'
#define ELLIPSIS_CHAR  '\002'


// Initializes the module.
// Initializes the LCD module.
// Defines custom characters.
void InitLCDUI(void);


// Puts "OK" in the upper right corner, puts the cursor on it,
// and waits for the user to press Enter.
void ConfirmMessage(void);

// Prints the given message centered in a field of the given width,
// starting at the current cursor position.
// Erases the rest of the field.
void PrintCentered(const char* msg, byte field);

// Shows the given two-line message, then prompts for confirmation.
void DialogBox(const char* msg1, const char* msg2);

// Clears the display and presents a menu of options.
// menuItems looks like:
//   [title:]item1[|item2[|item3...]]]
// Examples:
//   No title: "New game...|Statistics...|Change sides|Game...|Sound...|Auto-off..."
//     (uses a convention of "..." to mean "more options," not an atomic action.
//   With title: "New game:Random|Computer first|Player first|Cancel"
//   Simple confirmation: "Reset Statistics:OK|Cancel"
// Returns the 0-relative index of the chosen item.
byte GetMenuChoice(rom char* menuItems);


#endif
