/* log.h
    Copyright (c) 2009 by Timothy J. Weber, tw@timothyweber.org.

    Provides various ways to compute logarithms.
*/

#ifndef _LOG_H_
#define _LOG_H_

#include "fixed16.h"

// Returns the log base 2 of x.
// If x is 0, returns 0.
fixed16 log2_us(unsigned short x);

// Returns the log base 2 of x.
// If X is nonpositive, returns 0.
fixed16 log2_f(fixed16 x);


#endif //_LOG_H_
