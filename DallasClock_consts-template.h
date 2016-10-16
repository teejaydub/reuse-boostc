// Constants for Dallas real-time clock module.
// Assumes this module will set up and control the I2C bus, for now.

////////////////////////////////////////////////////////////////////////////
// i2c master hardware / software mode definition
//
// For i2c hardware support comment out the #define use_ic2_SW line
////////////////////////////////////////////////////////////////////////////
//#define use_i2c_SW

#if defined use_i2c_SW
////////////////////////////////////////////////////////////////////////////
// i2c software implementation template arguments and variables
////////////////////////////////////////////////////////////////////////////
#define i2c_ARGS	3, PORTC, TRISC, 4, PORTC, TRISC, e_SSPCON1, e_SSPCON2, \
					e_SSPSTAT, e_SSPBUF, e_SSPIF_BIT, e_SSPIF_PIR,			\
					e_BCLIF_BIT, e_BCLIF_PIR, 7, e_SSPADD, (i2c_reset_wdt | i2c_SMP)

// RAM used by the software i2c driver to emulate the equivalent i2c hardware registers					
unsigned short swi2c_SSPCON1@0x40;	// define location for the emulated SSPCON1
unsigned short swi2c_SSPCON2@0x41;	// define location for the emulated SSPCON2
unsigned short swi2c_SSPSTAT@0x42;	// define location for the emulated SSPSTAT
unsigned short swi2c_SSPBUF@0x43;	// define location for the emulated SSPBUF
unsigned short swi2c_SSPIF_PIR@0x44;// define location for the emulated SSPIF_PIR
unsigned short swi2c_BCLIF_PIR@0x45;// define location for the emulated BCLIF_PIR
unsigned short swi2c_SSPADD@0x46;	// define location for the emulated SSPADD

// variables cannot be passed as template arguments. The following constants map to
// the PIC registers and software emalated i2c RAM locations. These constants are
// then used by the templated functions. When changing the address of an emulated
// register the corresponding constant mapping must also be changed.
#define e_SSPCON1	0x40
#define e_SSPCON2	0x41
#define e_SSPSTAT	0x42
#define e_SSPADD	0x43
#define e_SSPBUF	0x44
#define e_SSPIF_PIR	0x45
#define e_BCLIF_PIR	0x46
#define e_SSPIF_BIT	3
#define e_BCLIF_BIT	3					

#else
////////////////////////////////////////////////////////////////////////////
// i2c hardwareware implementation template arguments
////////////////////////////////////////////////////////////////////////////
#define i2c_ARGS	3, PORTC, TRISC, 4, PORTC, TRISC, e_SSPCON1, e_SSPCON2, \
					e_SSPSTAT, e_SSPBUF, e_SSPIF_BIT, e_SSPIF_PIR,			\
					e_BCLIF_BIT, e_BCLIF_PIR, 7, e_SSPADD, (i2c_reset_wdt | i2c_SMP |i2c_HW)

// variables cannot be passed as template arguments. The following constants map to
// the PIC registers and PIC's i2c register locations. These constants are
// then used by the template1d functions. 
#define e_SSPCON1	0xfc6
#define e_SSPCON2	0xfc5
#define e_SSPSTAT	0xfc7
#define e_SSPADD	0xfc8
#define e_SSPBUF	0xfc9
#define e_SSPIF_PIR	0xf9e
#define e_BCLIF_PIR	0xfa1
#define e_SSPIF_BIT	3
#define e_BCLIF_BIT	3

#endif
