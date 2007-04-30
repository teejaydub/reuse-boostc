/* chipinit.h

	Standard code to initialize PICs.
	
	Packaged in inline functions, because it plays easier with the compilers
	(can't specify source files in other directories!)
	and because this code is typically only used once, so inline doesn't cost anything.
	
	As I use new chips, I'll add chip-specific sections.
*/

#include <system.h>

// Disables all optional peripherals, including:
//  Comparators
//  A/D converters
inline void DisablePeripherals(void)
{
	#if defined(_PIC12F629) || defined(_PIC12F675)
		// Disable comparators.
		cmcon = 7;
	#endif
	
	#if defined(_PIC12F675)
		// Disable A/D.
		ansel = 0;
	#endif
	
	#if defined(_PIC18F2620)
	#endif
}
