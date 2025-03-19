/* Define the port, tri-state reg, and pin. */

#define OW_PIN  1

// Second bus - irrelevant unless you call one of the *_2 functions.
#define OW_PIN2  5

volatile char ow_port@TRISA;  // PORTA
volatile char ow_tris@TRISB;  // TRISA
#define ow_port_  portb_

// Don't modify this.
#define OW_MASK  (1 << OW_PIN)
#define OW_MASK_2  (1 << OW_PIN_2)
