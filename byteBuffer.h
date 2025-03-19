/* byteBuffer.h
    Copyright (c) 2017 by Timothy J. Weber, tw@timothyweber.org.

	A fast buffer that can have bytes written to it at the end and read from the beginning.
	When empty, it's reset to the beginning.
	So, it's like a circular buffer, except without the overhead,
	for situations where you can be sure of the maximum length of messages
	but not the precise length of an individual message.
    You also must be sure that a previous message will be completely read (so the buffer is cleared)
    before the next message is received.
	
    Copyright (c) 2017, 2018 by Timothy J. Weber, tw@timothyweber.org.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Define REF_BUG if there's a bug with references that affects this module.
// Was showing up with SourceBoost 7.42 and the PIC18F45K22.
// References will be mocked up via pointers instead.

#ifndef __BYTE_BUFFER_H
#define __BYTE_BUFFER_H

#include <ctype.h>
#include "types-tjw.h"

struct ByteBuf_s {
	byte* buffer;
	byte lenUsed;
	byte readIndex;
};

typedef struct ByteBuf_s ByteBuf;

inline void init(ByteBuf& bb, byte* buffer)
{
	bb.buffer = buffer;
	bb.lenUsed = 0;
	bb.readIndex = 0;
}

#ifndef REF_BUG
inline void clear(ByteBuf& bb)
{
	bb.lenUsed = 0;
	bb.readIndex = 0;
}
#else
#define clear(b)  clearP(&b)
inline void clearP(ByteBuf* bb)
{
	bb->lenUsed = 0;
	bb->readIndex = 0;
}
#endif

// Pushes a character onto the end of the buffer.
// Does nothing if the buffer has already grown to maxLen.
template <int maxLen>
inline void push(ByteBuf& bb, byte b)
{
	if (bb.lenUsed < maxLen) {
		bb.buffer[bb.lenUsed++] = b;
	}
}

#ifndef REF_BUG
inline byte peek(ByteBuf& bb)
{
	return bb.buffer[bb.readIndex];
}
#else
#define peek(b)  peekP(&b)
inline byte peekP(ByteBuf* bb)
{
	return bb->buffer[bb->readIndex];
}
#endif

#ifndef REF_BUG
inline byte pop(ByteBuf& bb)
{
	byte result = peek(bb);
	if (++bb.readIndex > bb.lenUsed)
		// Constrain to the used length of the buffer.
		bb.readIndex = bb.lenUsed;
	if (bb.readIndex == bb.lenUsed)
		// Reset back to the beginning when it's empty.
		clear(bb);
		
	return result;
}
#else
#define pop(b)  popP(&b)
inline byte popP(ByteBuf* bb)
{
	byte result = peekP(bb);
	if (++bb->readIndex > bb->lenUsed)
		// Constrain to the used length of the buffer.
		bb->readIndex = bb->lenUsed;
	if (bb->readIndex == bb->lenUsed)
		// Reset back to the beginning when it's empty.
		clearP(bb);
		
	return result;
}
#endif

inline bool isEmpty(ByteBuf& bb)
{
	return bb.readIndex == bb.lenUsed;
}

template <int maxLen>
inline bool isFull(ByteBuf& bb)
{
	return bb.lenUsed >= maxLen;
}

inline byte length(ByteBuf& bb)
{
	return bb.lenUsed - bb.readIndex;
}

// Discards the given number of characters from the input queue.
inline void skip(ByteBuf& bb, byte count)
{
	bb.readIndex += count;
	if (++bb.readIndex > bb.lenUsed)
		bb.readIndex = bb.lenUsed;
	if (bb.readIndex == bb.lenUsed)
		// Reset back to the beginning when it's empty.
		clear(bb);
}

inline void read(ByteBuf& bb, byte* data, byte count)
{
	strncpy(data, bb.buffer + bb.readIndex, count);
	skip(bb, count);
}

inline bool contains(ByteBuf& bb, char c)
{
	byte* bufp = bb.buffer + bb.readIndex;
	for (byte i = bb.readIndex; i < bb.lenUsed; i++)
		if (*(bufp++) == c)
			return true;
	return false;
}

inline bool containsWhitespace(ByteBuf& bb)
{
    byte* bufp = bb.buffer + bb.readIndex;
    for (byte i = bb.readIndex; i < bb.lenUsed; i++)
        if (isspace(*bufp))
            return true;
        else
            ++bufp;
    return false;
}

#endif
// __BYTE_BUFFER_H