// Portable EEPROM read and write routines.


char read_eeprom(char addr);
void write_eeprom(char addr, char data);

void read_eeprom_block(char addr, char* buf, unsigned char len);
void write_eeprom_block(char addr, char* buf, unsigned char len);
