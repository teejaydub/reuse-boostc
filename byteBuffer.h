/* byteBuffer.h

	A fast buffer that can have bytes written to it at the end and read from the beginning.
	When empty, it's reset to the beginning.
	So, it's like a circular buffer, except without the overhead,
	for situations where you can be sure of the maximum length of messages
	but not the precise length of an individual message.
	
    Copyright (c) 2017 by Timothy J. Weber, tw@timothyweber.org.

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

#ifndef __BYTE_BUFFER_H
#define __BYTE_BUFFER_H

#include <types-tjw.h>

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

inline void clear(ByteBuf& bb)
{
	bb.lenUsed = 0;
	bb.readIndex = 0;
}

// Pushes a character onto the end of the buffer.
// Does nothing if the buffer has already grown to maxLen.
template <int maxLen>
inline void push(ByteBuf& bb, byte b)
{
	if (bb.lenUsed <= maxLen) {
		bb.buffer[bb.lenUsed++] = b;
	}
}

inline byte peek(ByteBuf& bb)
{
	return bb.buffer[bb.readIndex];
}

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
		if (*bufp++ == c)
			return true;
	return false;
}

#endif
// __BYTE_BUFFER_H