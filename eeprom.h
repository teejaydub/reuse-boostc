
void write_eeprom(char addr, char data);

// Chips that can read and write their program ROM call it 'EEARDL' instead of 'EEADR'.
// Compensate.
#ifdef EEADRL
volatile char eeadr@EEADRL;
volatile char eedata@EEDATL;
#endif

inline char read_eeprom(char addr)
{
	eeadr = addr;
	set_bit(eecon1, RD);
	return eedata;
}
