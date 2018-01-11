
#ifndef _PIC16F1789_ADDS_H_
#define _PIC16F1789_ADDS_H_

////////////////////////////////////////////////////////////////////////////
//
//       Register Definitions
//
////////////////////////////////////////////////////////////////////////////

#define PORTA                   0x000C
#define PORTB                   0x000D
#define PORTC                   0x000E
#define PORTD                   0x000F
#define PORTE                   0x0010
#define PIR1					0x0011
#define PIR2					0x0012

#define TMR0                    0x0015 
#define TMR1L                   0x0016 
#define TMR1H                   0x0017 
#define T1CON                   0x0018
#define T1GCON                  0x0019
#define TMR2                    0x001A 
#define PR2                     0x001B
#define T2CON                   0x001C 

#define TRISA                   0x008C
#define TRISB                   0x008D
#define TRISC                   0x008E
#define TRISD                   0x008F
#define TRISE                   0x0090
#define PIE1                    0x0091

#define OPTION_REG              0x0095 

#define WDTCON                  0x0097 

#define OSCCON                  0x0099

#define ADRESL                  0x009B
#define ADRESH                  0x009C
#define ADCON0                  0x009D
#define ADCON1                  0x009E

#define LATA                    0x010C
#define LATB                    0x010D
#define LATC                    0x010E
#define LATD                    0x010F
#define LATE                    0x0110
#define CM1CON0                 0x0111 
#define CM1CON1                 0x0112 
#define CM2CON0                 0x0113 
#define CM2CON1                 0x0114 

#define ANSELA                  0x018C
#define ANSELB                  0x018D
#define ANSELC                  0x018E
#define ANSELD                  0x018F
#define ANSELE                  0x0190
#define EEADR                   0x0191 
#define EEADRL                  0x0191 
#define EEADRH                  0x0192
#define EEDATL                  0x0193 
#define EEDATH                  0x0194 
#define EECON1                  0x0195 
#define EECON2                  0x0196 

#define RCREG                   0x0199 
#define TXREG                   0x019A 
#define SPBRG                   0x019B 
#define SPBRGH                  0x019C 
#define RCSTA                   0x019D 
#define TXSTA                   0x019E 


/////// ADCON0 Bits ////////////////////////////////////////////////////////
#define ADRMD                 0x0007 
#define CHS4                  0x0006 
#define CHS3                  0x0005 
#define CHS2                  0x0004 
#define CHS1                  0x0003 
#define CHS0                  0x0002 
#define GO                    0x0001 
#define NOT_DONE              0x0001 
#define GO_DONE               0x0001 
#define ADON                  0x0000 

/////// ADCON1 Bits ////////////////////////////////////////////////////////
#define ADFM                  0x0007 

/////// CM1CON0 Bits ///////////////////////////////////////////////////////
#define C1ON                  0x0007 
#define C1OUT                 0x0006 
#define C1OE                  0x0005 
#define C1POL                 0x0004 

/////// CM2CON0 Bits ///////////////////////////////////////////////////////
#define C2ON                  0x0007 
#define C2OUT                 0x0006 
#define C2OE                  0x0005 
#define C2POL                 0x0004 

/////// CM2CON1 Bits ///////////////////////////////////////////////////////

/////// EECON1 Bits ////////////////////////////////////////////////////////
#define EEPGD                 0x0007 
#define WRERR                 0x0003 
#define WREN                  0x0002 
#define WR                    0x0001 
#define RD                    0x0000 

/////// INTCON Bits ////////////////////////////////////////////////////////
#define GIE                   0x0007 
#define PEIE                  0x0006 
#define T0IE                  0x0005 
#define TMR0IE                0x0005 
#define INTE                  0x0004 
#define RBIE                  0x0003 
#define IOCIE                 0x0003 
#define T0IF                  0x0002 
#define TMR0IF                0x0002 
#define INTF                  0x0001 
#define RBIF                  0x0000 
#define IOCIF                 0x0000 

/////// OPTION_REG Bits /////////////////////////////////////////////////////
#define NOT_RBPU              0x0007 
#define NOT_WPUEN             0x0007 
#define INTEDG                0x0006 
#define T0CS                  0x0005 
#define T0SE                  0x0004 
#define PSA                   0x0003 
#define PS2                   0x0002 
#define PS1                   0x0001 
#define PS0                   0x0000 

/////// OSCCON Bits ////////////////////////////////////////////////////////
#define SPLLEN                0x0007
#define IRCF3                 0x0006 
#define IRCF2                 0x0005 
#define IRCF1                 0x0004 
#define IRCF0                 0x0003 
#define SCS1                  0x0001 
#define SCS0                  0x0000 

/////// PIE1 Bits //////////////////////////////////////////////////////////
#define TMR1GIE               0x0007
#define ADIE                  0x0006 
#define RCIE                  0x0005 
#define TXIE                  0x0004 
#define SSPIE                 0x0003 
#define CCP1IE                0x0002 
#define TMR2IE                0x0001 
#define TMR1IE                0x0000 

/////// PIR1 Bits //////////////////////////////////////////////////////////
#define TMR1GIF               0x0007
#define ADIF                  0x0006 
#define RCIF                  0x0005 
#define TXIF                  0x0004 
#define SSPIF                 0x0003 
#define CCP1IF                0x0002 
#define TMR2IF                0x0001 
#define TMR1IF                0x0000 

/////// PIR2 Bits //////////////////////////////////////////////////////////
#define OSFIF                 0x0007 
#define C2IF                  0x0006 
#define C1IF                  0x0005 
#define EEIF                  0x0004 
#define BCLIF                 0x0003 
#define C4IF                  0x0002 
#define C3IF                  0x0001 
#define CCP2IF                0x0000 

/////// RCSTA Bits /////////////////////////////////////////////////////////
#define SPEN                  0x0007 
#define RX9                   0x0006 
#define RC9                   0x0006 // Backward compatibility only
#define NOT_RC8               0x0006 // Backward compatibility only
#define RC8_9                 0x0006 // Backward compatibility only
#define SREN                  0x0005 
#define CREN                  0x0004 
#define ADDEN                 0x0003 
#define FERR                  0x0002 
#define OERR                  0x0001 
#define RX9D                  0x0000 
#define RCD8                  0x0000 // Backward compatibility only

/////// T1CON Bits /////////////////////////////////////////////////////////
#define T1CKPS1               0x0005 
#define T1CKPS0               0x0004 
#define T1OSCEN               0x0003 
#define NOT_T1SYNC            0x0002 
#define T1INSYNC              0x0002 // Backward compatibility only
#define T1SYNC                0x0002 
#define TMR1ON                0x0000 

/////// T2CON Bits /////////////////////////////////////////////////////////
#define TOUTPS3               0x0006 
#define TOUTPS2               0x0005 
#define TOUTPS1               0x0004 
#define TOUTPS0               0x0003 
#define TMR2ON                0x0002 
#define T2CKPS1               0x0001 
#define T2CKPS0               0x0000 

/////// TXSTA Bits /////////////////////////////////////////////////////////
#define CSRC                  0x0007 
#define TX9                   0x0006 
#define NOT_TX8               0x0006 // Backward compatibility only
#define TX8_9                 0x0006 // Backward compatibility only
#define TXEN                  0x0005 
#define SYNC                  0x0004 
#define SENDB                 0x0003 
#define BRGH                  0x0002 
#define TRMT                  0x0001 
#define TX9D                  0x0000 
#define TXD8                  0x0000 // Backward compatibility only

/////// WDTCON Bits ////////////////////////////////////////////////////////
#define WDTPS3                0x0004 
#define WDTPS2                0x0003 
#define WDTPS1                0x0002 
#define WDTPS0                0x0001 
#define SWDTEN                0x0000 

////////////////////////////////////////////////////////////////////////////
//
//       Configuration Bits
//
////////////////////////////////////////////////////////////////////////////

#define _CONFIG               0x8007

#define _CONFIG1              0x8007 
#define _CONFIG2              0x8008 

/////// Configuration Word1 ////////////////////////////////////////////////
#define _BOR_ON               0x3FFF 
#define _BOR_NSLEEP           0x3DFF 
#define _BOR_SBODEN           0x3BFF 
#define _BOR_OFF              0x39FF 
#define _CPD_ON               0x3FFF 
#define _CPD_OFF              0x3EFF 
#define _CP_ON                0x3FFF 
#define _CP_OFF               0x3F7F 
#define _MCLRE_ON             0x3FFF 
#define _MCLRE_OFF            0x3FBF 
#define _PWRTE_ON             0x3FFF 
#define _PWRTE_OFF            0x3FDF 
#define _WDT_ON               0x3FFF 
#define _WDT_RUN              0x3FF7 
#define _WDT_SOFT             0x3FEF 
#define _WDT_OFF              0x3FE7 

#define _ECH                  0x3FFF // External clock, high power 
#define _ECM                  0x3FFE //  medium power
#define _ECL                  0x3FFD //  low power
#define _INTOSC               0x3FFC // Internal oscillator, CLKIN is I/O
#define _EXTRC                0x3FFB // External RC circuit on CLKIN
#define _HS                   0x3FFA // High-speed oscillator on OSC1 and OSC2
#define _XT                   0x3FF9 // Crystal/resonator
#define _LP                   0x3FF8 // Low-power crystal

/////// Configuration Word2 ////////////////////////////////////////////////
#define _LVP_ON               0x3FFF 
#define _LVP_OFF              0x2FFF 
#define _BOR21V               0x3FFF 
#define _BOR40V               0x3BFF 
#define _PLLEN                0x3FFF 
#define _PLLEN_OFF            0x3EFF 

#define _WRT_OFF              0x3FFF // No prog memory write protection
#define _WRT_LOW              0x3FFE // Low prog memory write protected
#define _WRT_HALF             0x3FFD // First half prog memory write protected
#define _WRT_ALL              0x3FFC // All prog memory write protected

/////////////////////////////////////////////////
// EEPROM Base Address when programing
/////////////////////////////////////////////////

// To initialise EEPROM when a device is programmed
// use #pragma DATA _EEPROM, 12, 34, 56 
#define _EEPROM               0xF000

/////////////////////////////////////////////////
// Register mapped variables
/////////////////////////////////////////////////

volatile char adcon0                 @ADCON0;
volatile char adcon1                 @ADCON1;
volatile char adresh                 @ADRESH;
volatile char adresl                 @ADRESL;
volatile char ansela                 @ANSELA;
volatile char anselb                 @ANSELB;
volatile char anselc                 @ANSELC;
volatile char anseld                 @ANSELD;
volatile char ansele                 @ANSELE;

volatile char cm1con0                @CM1CON0;
volatile char cm1con1                @CM1CON1;
volatile char cm2con0                @CM2CON0;
volatile char cm2con1                @CM2CON1;

volatile char eeadr                  @EEADRL;
volatile char eeadrl                 @EEADRL;
volatile char eeadrh                 @EEADRH;
volatile char eecon1                 @EECON1;
volatile char eecon2                 @EECON2;
volatile char eedata                 @EEDATL;
volatile char eedatl                 @EEDATL;
volatile char eedath                 @EEDATH;

volatile char lata                   @LATA;
volatile char latb                   @LATB;
volatile char latc                   @LATC;
volatile char latd                   @LATD;
volatile char late                   @LATE;

volatile char option_reg             @OPTION_REG;
volatile char osccon                 @OSCCON;

volatile char pie1                   @PIE1;
volatile char pir1                   @PIR1;
volatile char pir2                   @PIR2;
volatile char pr2                    @PR2;

volatile char porta                  @PORTA;
volatile char portb                  @PORTB;
volatile char portc                  @PORTC;
volatile char portd                  @PORTD;
volatile char porte                  @PORTE;

volatile char rcreg                  @RCREG;
volatile char rcsta                  @RCSTA;

volatile char spbrg                  @SPBRG;
volatile char spbrgh                 @SPBRGH;

volatile char trisa                  @TRISA;
volatile char trisb                  @TRISB;
volatile char trisc                  @TRISC;
volatile char trisd                  @TRISD;
volatile char trise                  @TRISE;

volatile char t1con                  @T1CON;
volatile char t2con                  @T2CON;
volatile char tmr0                   @TMR0;
volatile char tmr1l                  @TMR1L;
volatile char tmr1h                  @TMR1H;
volatile char txreg                  @TXREG;
volatile char txsta                  @TXSTA;

volatile char wdtcon                 @WDTCON;

#endif
// _PIC16F1789_ADDS_H_
