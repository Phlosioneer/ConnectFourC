
#ifndef UTF8_H
#define UTF8_H

#pragma once

#include "common.h"

// Takes a unicode codepoint, and writes it to the given buffer.
// Returns the number of bytes written, or SIZE_MAX number if the buffer is too small.
size_t utfEncodeChar(IN int codepoint, IN size_t buffLen, OUT char* bytes);

// Returns the size of the codepoint's representation, in bytes.
// If the codepoint is invalid, returns SIZE_MAX.
size_t utfCodepointSize(int codepoint);

// Encodes the null-terminated codepoints in the given buffer, including the null-terminator. On success, the
// size of the string is stored in strLenPtr, and 0 is returned.
//
// If an error occurs, all characters up to that error are written to the buffer, and strLenPtr is set accordingly.
// Then utfEncodeCodepoints returns SIZE_MAX if any code point is invalid, and it returns the minimum buffer size
// if it runs out of space. If both errors occur, SIZE_MAX is returned.
//
// strLenPtr may be NULL.
size_t utfEncodeCodepoints(IN int const* codepoints, IN size_t buffLen, OUT size_t* strLenPtr, OUT char* buffer);


#endif // !UTF8_H