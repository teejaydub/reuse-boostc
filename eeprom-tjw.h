/* eeprom-tjw.h
    Copyright (c) 2006, 2018 by Timothy J. Weber, tw@timothyweber.org.

    Portable EEPROM read and write routines.
    Addresses start at 0 for the first byte of EEPROM.
*/

char read_eeprom(char addr);
void write_eeprom(char addr, char data);

void read_eeprom_block(char addr, char* buf, unsigned char len);
void write_eeprom_block(char addr, char* buf, unsigned char len);

// Adds one to an unsigned long counter at the given EEPROM address.
// Avoids overflow by bracketing the count at its max value.
void inc_eeprom_counter_long(char addr);

// Accumulates the maximum unsigned value in the given EEPROM address.
void accum_max_eeprom_byte(char addr, unsigned char value);

// EE_PIR is the register where EEIF is.
#if defined(_PIC16F688)
	#define EE_PIR pir1
#else
	#define EE_PIR pir2
#endif

// Wait for a previous EEPROM write to finish.
// Must have been preceded by an EEPROM write call.
inline void wait_eeprom_write(void)
{	
	// Wait for the write to complete.
	while (!EE_PIR.EEIF)
		clear_wdt();
	EE_PIR.EEIF = 0;
}