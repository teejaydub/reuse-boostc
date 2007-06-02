// Menu system for a small alphanumeric LCD panel.
// Designed for a 2x16 panel, but should scale.
// Requires the lcd module, which also requires lcd_consts.h,
// and LCDMenuConsts.h.

#ifndef _LCDMENU_H
#define _LCDMENU_H

#include "types-tjw.h"

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
