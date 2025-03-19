/* mem-tjw.h
    Copyright (c) 2006 by Timothy J. Weber, tw@timothyweber.org.

    Similar to std C lib mem*, but more efficient if you don't need the exact behavior.
*/

#ifndef __MEMTJW_H
#define __MEMTJW_H

// Like memcpy, but doesn't return anything.
void copyBytes(char* dst, char* src, unsigned char len);

// Like memcmp, but only returns true or false.
unsigned char bytesEqual(char* a, char* b, unsigned char len);

#endif
// __MEMTJW_H
