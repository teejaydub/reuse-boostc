/* chipinit.h
    Copyright (c) 2007, 2018 by Timothy J. Weber, tw@timothyweber.org.

	Standard code to initialize PICs.
	
	Packaged in inline functions, because it plays easier with the compilers
	(can't specify source files in other directories!)
	and because this code is typically only used once, so inline doesn't cost anything.
	
	As I use new chips, I'll add chip-specific sections.
	
	Chips supported:
	
	PIC12F629
	PIC12F675
	PIC12F683
	PIC16F1789
	PIC16F688
	PIC16F690
	PIC16F916
	PIC18F2320
	PIC18F2550
	PIC18F2620
	PIC18F45K22
*/

#if defined(_PIC12F675) || defined(_PIC16F1789) || defined(_PIC16F916) || defined(_PIC16F688) || defined(_PIC12F683) 
#elif defined(_PIC18F2320) || defined(_PIC18F2620) || defined(_PIC16F690) || defined(_PIC16F883) || defined(_PIC16F886) || defined(_PIC16F887)
#elif defined(_PIC18F1320) || defined(_PIC18F2550) || defined(_PIC18F45K22)
	// New chips supported must be listed here.
#else
	#error "chipinit.h: Chip not recognized; check code and add this chip's required settings"
#endif

// Disables all optional peripherals, including:
//  Comparators
//  A/D converters
//  LCD driver
inline void DisablePeripherals(void)
{
	// Disable comparators.
	#if defined(_PIC12F629) || defined(_PIC12F675) || defined(_PIC18F2620) || defined(_PIC18F2550)
		cmcon = 7;
	#endif
	
	#if defined(_PIC16F916) || defined(_PIC16F688) || defined(_PIC12F683)
		cmcon0 = 7;
	#endif
	
	// Disable A/D.
	#if defined(_PIC12F675) || defined(_PIC16F916) || defined(_PIC16F688) || defined(_PIC12F683) || defined(_PIC16F690) || defined(_PIC16F883) || defined(_PIC16F886) || defined(_PIC16F887)
		ansel = 0;
	#endif
	
	#if defined(_PIC18F45K22) || defined(_PIC16F1789)
		ansela = 0;
		anselb = 0;
		#ifdef ANSELC
		anselc = 0;
		#endif
		#ifdef ANSELD
		anseld = 0;
		#endif
		#ifdef ANSELE
		ansele = 0;
		#endif
	#endif
	
	#if defined(_PIC16F690) || defined(_PIC16F883) || defined(_PIC16F886) || defined(_PIC16F887)
		anselh = 0;
	#endif

	#if defined(_PIC16F1789)
		// A/D is disabled on startup, but disconnect all channels just to be sure.
		adcon0 = 0x50;  // CHS = 10100
	#endif
	
	#if defined(_PIC18F2320) || defined(_PIC18F2620) || defined(_PIC18F1320) || defined(_PIC18F2550)
		adcon1 = 0x0F;  // all digital inputs	
	#endif
	
	// Disable LCD driver.
	#if defined(_PIC16F916)
		lcdcon.VLCDEN = 0;
	#endif
}
