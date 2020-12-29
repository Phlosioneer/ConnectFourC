
#include <limits.h>

#include "utf8.h"


// Takes a unicode codepoint, and writes it to the given buffer.
// Returns the number of bytes written, or SIZE_MAX number if the buffer is too small.
size_t utfEncodeChar(IN int codepoint, IN size_t buffLen, OUT char* bytes) {
	if (codepoint < 0x80) {
		if (buffLen < 1) {
			return SIZE_MAX;
		}
		bytes[0] = (char)codepoint;
		return 1;
	}
	else if (codepoint < 0x800) {
		if (buffLen < 2) {
			return SIZE_MAX;
		}
		bytes[0] = 0b11000000 + (char)(codepoint >> 6);
		bytes[1] = 0x80 + (char)(codepoint & 0x3F);
		return 2;
	}
	else if (codepoint < 0x10000) {
		if (buffLen < 3) {
			return SIZE_MAX;
		}
		bytes[0] = 0b11100000 + (char)(codepoint >> 12);
		bytes[1] = 0x80 + (char)((codepoint >> 6) & 0x3F);
		bytes[2] = 0x80 + (char)(codepoint & 0x3F);
		return 3;
	}
	else if (codepoint < 0x110000) {
		if (buffLen < 4) {
			return SIZE_MAX;
		}
		bytes[0] = 0b11110 + (char)(codepoint >> 18);
		bytes[1] = 0x80 + (char)((codepoint >> 12) & 0x3F);
		bytes[2] = 0x80 + (char)((codepoint >> 6) & 0x3F);
		bytes[3] = 0x80 + (char)(codepoint & 0x3F);
		return 4;
	}
	else {
		return SIZE_MAX;
	}
}

// Returns the size of the codepoint's representation, in bytes.
// If the codepoint is invalid, returns SIZE_MAX.
size_t utfCodepointSize(int codepoint) {
	if (codepoint < 0x80) { return 1; }
	if (codepoint < 0x800) { return 2; }
	if (codepoint < 0x10000) { return 3; }
	if (codepoint < 0x11000) { return 4; }
	return SIZE_MAX;
}

// Encodes the null-terminated codepoints in the given buffer, including the null-terminator. On success, the
// size of the string is stored in strLenPtr, and 0 is returned.
//
// If an error occurs, all characters up to that error are written to the buffer, and strLenPtr is set accordingly.
// Then utfEncodeCodepoints returns SIZE_MAX if any code point is invalid, and it returns the minimum buffer size
// if it runs out of space. If both errors occur, SIZE_MAX is returned.
//
// strLenPtr may be NULL.
size_t utfEncodeCodepoints(IN int const* codepoints, IN size_t buffLen, OUT size_t* strLenPtr, OUT char* buffer) {
	size_t cursor = 0;
	for (int i = 0; codepoints[i] != 0; i++) {
		size_t written = utfEncodeChar(codepoints[i], buffLen - cursor, &buffer[cursor]);
		if (written == SIZE_MAX) {
			if (strLenPtr != NULL) {
				*strLenPtr = cursor;
			}
			// Calculate the extra space.
			for (int j = 0; codepoints[j] != 0; j++) {
				size_t size = utfCodepointSize(codepoints[j]);
				if (size == SIZE_MAX) {
					return SIZE_MAX;
				}
				cursor += size;
			}
			// +1 for null terminator.
			return cursor + 1;
		}
		cursor += written;
	}

	if (cursor == buffLen) {
		// Not enough space for the null terminator.
		return 1;
	}
	buffer[cursor] = '\0';
	if (strLenPtr != NULL) {
		*strLenPtr = cursor + 1;
	}
	return 0;
}