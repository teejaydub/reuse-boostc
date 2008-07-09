/* chipinit.h

	Standard code to initialize PICs.
	
	Packaged in inline functions, because it plays easier with the compilers
	(can't specify source files in other directories!)
	and because this code is typically only used once, so inline doesn't cost anything.
	
	As I use new chips, I'll add chip-specific sections.
	
	Chips supported:
	
	PIC12F629
	PIC12F675
	PIC12F683
	PIC16F688
	PIC16F916
	PIC18F2620
*/


// Disables all optional peripherals, including:
//  Comparators
//  A/D converters
//  LCD driver
inline void DisablePeripherals(void)
{
	// Disable comparators.
	#if defined(_PIC12F629) || defined(_PIC12F675) || defined(_PIC18F2620)
		cmcon = 7;
	#endif
	
	#if defined(_PIC16F916) || defined(_PIC16F688) || defined(_PIC12F683)
		cmcon0 = 7;
	#endif
	
	// Disable A/D.
	#if defined(_PIC12F675) || defined(_PIC16F916) || defined(_PIC16F688) || defined(_PIC12F683)
		ansel = 0;
	#endif
	
	#if defined(_PIC18F2620)
		adcon1 = 0x0F;  // all digital inputs
	#endif
	
	// Disable LCD driver.
	#if defined(_PIC16F916)
		lcdcon.VLCDEN = 0;
	#endif
}
