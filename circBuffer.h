/* byteBuffer.h

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

// Define BYTEBUF_CIRCULAR if you want a circular buffer.
// This is mainly useful to get access to the history.


#ifndef __BYTE_BUFFER_H
#define __BYTE_BUFFER_H

#include <ctype.h>
#include "types-tjw.h"

struct ByteBuf_s {
	byte* buffer;  // storage
	byte readIndex;  // where the stored string starts (< maxLen)
    byte writeIndex;  // where the next byte should be written (< maxLen)
	byte len;  // length of the content; may wrap around.
    /* Examples:
        with maxLen=16
        Buffer after clearing:
        [................]
         0             maxLen-1
         ^readIndex=0
         ^writeIndex=0
         len=0

        Buffer with "one two" written to it:
        [one two.........]
         ^readIndex=0
                ^writeIndex=7
                len=7

        Buffer after reading "one ":
        [one two.........]
             ^readIndex=4
                ^writeIndex=7
                len=3

        Then add " three":
        [one two three...]
             ^readIndex=4
                      ^writeIndex=13
                      len=9

        Then add " fo":
        [ure two three fo]
             ^readIndex=4
         ^writeIndex=0
          len=12

        Then add "ur":
        [ure two three fo]
             ^readIndex=4
           ^writeIndex=2
           len=14

        Then add "!!":
        [ur!!two three fo]
             ^readIndex=4
             ^writeIndex=4
             len=16

        Then read "two three ":
        [ur!!two three fo]
                       ^readIndex=14
             ^writeIndex=4
              len=6

        Then read "fo":
        [ur!!two three fo]
         ^readIndex=0
             ^writeIndex=4
              len=4

        Then read "ur!!":
        [ur!!two three fo]
             ^readIndex=0
             ^writeIndex=4
              len=0
    */
};

typedef struct ByteBuf_s ByteBuf;

inline void init(ByteBuf& bb, byte* buffer)
{
	bb.buffer = buffer;
	bb.readIndex = 0;
    bb.writeIndex = 0;
	bb.len = 0;
}

#ifndef REF_BUG
inline void clear(ByteBuf& bb)
{
	bb.readIndex = bb.writeIndex;
    bb.writeIndex = 0;
	bb.len = 0;
}
#else
#define clear(b)  clearP(&b)
inline void clearP(ByteBuf* bb)
{
	bb->readIndex = bb->writeIndex;
    bb->writeIndex = 0;
	bb->len = 0;
}
#endif

// Increments the given index variable with respect to the maximum length.
#ifndef REF_BUG
#define incIndex(maxLen, b)  (incIndexR<maxLen>(b))
template<int maxLen>
inline void incIndexR(byte& b)
{
    ++b;
    if (b >= maxLen)
        b = 0;
}
#else
#define incIndex(maxLen, b)  (incIndexP<maxLen>(&b))
template<int maxLen>
inline void incIndexP(byte* b)
{
    ++(*b);
    if (*b >= maxLen)
        *b = 0;
}
#endif

// Pushes a character onto the end of the buffer.
// Does nothing if the buffer has already grown to maxLen.
template <int maxLen>
inline void push(ByteBuf& bb, byte b)
{
	if (bb.len < maxLen) {
		bb.buffer[bb.writeIndex] = b;
        incIndex(maxLen, bb.writeIndex);
        bb.len++;
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

// Discards one character from the input queue.
// Does nothing if the queue is empty.
#ifndef REF_BUG
template <int maxLen>
inline void skip(ByteBuf& bb)
{
    if (bb.len > 0) {
        incIndex(maxLen, bb.readIndex);
        --bb.len;
    }
}
#else
#define skip(b)  skipP(&b)
template <int maxLen>
inline void skipP(ByteBuf* bb)
{
    if (bb->len > 0) {
        incIndex(maxLen, bb->readIndex);
        --bb->len;
    }
}
#endif

#ifndef REF_BUG
#define pop(maxLen, b)  (popR<maxLen>(&b))
template <int maxLen>
inline byte popR(ByteBuf& bb)
{
	byte result = peek(bb);
    skip(maxLen, bb);
	return result;
}
#else
#define pop(maxLen, b)  (popP<maxLen>(&b))
template <int maxLen>
inline byte popP(ByteBuf* bb)
{
	byte result = peekP(bb);
    skipP<maxLen>(bb);
	return result;
}
#endif

inline bool isEmpty(ByteBuf& bb)
{
	return bb.len == 0;
}

template <int maxLen>
inline bool isFull(ByteBuf& bb)
{
	return bb.len >= maxLen;
}

inline byte length(ByteBuf& bb)
{
	return bb.len;
}

// Copy bytes from bb into data.
// Could be more efficient, but going for correct here because I don't think this is used.
template <int maxLen>
inline void read(ByteBuf& bb, byte* data, byte count)
{
	// strncpy(data, bb.buffer + bb.readIndex, count);
	// skip(bb, count);
    while ((count > 0) && (len > 0))
        *(data++) = pop(maxLen, bb);
}

template <int maxLen>
inline bool contains(ByteBuf& bb, char c)
{
	byte bufIndex = bb.readIndex;
	for (byte i = 0; i < bb.len; i++)
		if (bb.buffer[bufIndex] == c)
			return true;
        else
            incIndex(maxLen, bufIndex);
	return false;
}

template <int maxLen>
inline bool containsWhitespace(ByteBuf& bb)
{
    byte bufIndex = bb.readIndex;
    for (byte i = 0; i < bb.len; i++)
        if (isspace(bb.buffer[bufIndex]))
            return true;
        else
            incIndex(maxLen, bufIndex);
    return false;
}

#endif
// __BYTE_BUFFER_H