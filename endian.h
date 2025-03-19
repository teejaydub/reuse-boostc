/* endian.h
    Copyright (c) 2009 by Timothy J. Weber, tw@timothyweber.org.

	Helps work out endian issues with long ints.
	That is, when you access individual bytes, is the most-significant stored first, or last?
*/

#ifndef __ENDIAN_H
#define __ENDIAN_H

// BoostC is little-endian - that is, it stores the LSB first in multi-byte integers.
#define LITTLE_ENDIAN

#endif