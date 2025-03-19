/* mem-tjw.c
    Copyright (c) 2006 by Timothy J. Weber, tw@timothyweber.org.
*/

void copyBytes(char* dst, char* src, unsigned char len)
{
	while (len--)
		*dst++ = *src++;
}

unsigned char bytesEqual(char* a, char* b, unsigned char len)
{
	while (len--)
		if (*a++ != *b++)
			return 0;
			
	return 1;
}
