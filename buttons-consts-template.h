// buttons-consts.h
// Customize this to fit your application.


#ifndef __BUTTONS_CONSTS
#define __BUTTONS_CONSTS


// The port used for buttons - all buttons must be on the same port.
#define BUTTON_PORT  portb
#define BUTTON_TRIS  trisb

// The button constants, sequential.
// Replace these with the buttons needed for your app.
#define PREV_BTN  0
#define NEXT_BTN  1

// The individual button masks.
#define PREV_BTN_MASK  (1 << PREV_BTN)
#define NEXT_BTN_MASK  (1 << NEXT_BTN)

#define FIRST_BTN  PREV_BTN
#define LAST_BTN  NEXT_BTN

// Minimum down events before recognizing a button press, in button check periods (often, ms).
#define MIN_DOWNS  40  // max 254


#endif