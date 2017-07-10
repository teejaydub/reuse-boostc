// spi-consts.h

// Define these pin assignments and protocol properties to suit your application.

// Currently only supports "bit-bang" mode.

// For bit-bang mode, SPI_MASTER must be true.
#define SPI_MASTER  true  // false = slave

// Port pin assignments.
#define SPI_SCK_PORT  portb
#define SPI_SCK_SHADOW  portb_
#define SPI_SCK_PIN  6
#define SPI_SCK_TRIS  trisb

#define SPI_SDI_PORT  porte
#define SPI_SDI_PIN  3
#define SPI_SDI_TRIS  trise

#define SPI_SDO_PORT  portb
#define SPI_SDO_SHADOW  portb_
#define SPI_SDO_PIN  7
#define SPI_SDO_TRIS  trisb

#define SPI_SS_PORT  portb
#define SPI_SS_SHADOW  portb_
#define SPI_SS_PIN  5
#define SPI_SS_TRIS  trisb

// Protocol parameters.
#define SPI_CLOCK_IDLE  0  // clock pin value when idle.

#define SPI_RISING  1
#define SPI_FALLING  0
#define SPI_CLOCK_EDGE  SPI_FALLING  // output data on this clock edge

