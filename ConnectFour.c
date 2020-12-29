// ConnectFour.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// Project configuration notes: I've disabled Language Extensions, which should cause VS to
// use ANSI-C like syntax. I'm unclear on this, though.

// Cross-platform Compat: Currently, this is designed to work using the windows API. It will later be
// modified to work on unix as well.

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Base for all the box glyph codes.
#define UNICODE_BOX_LINES_START 0x2500

// Corner names describe where the corner is pointing
#define GLYPH_TOP_LEFT		(0xC + UNICODE_BOX_LINES_START)
#define GLYPH_TOP_RIGHT		(0x10 + UNICODE_BOX_LINES_START)
#define GLYPH_BOTTOM_LEFT	(0x14 + UNICODE_BOX_LINES_START)
#define GLYPH_BOTTOM_RIGHT	(0x18 + UNICODE_BOX_LINES_START)

// T-junction names describe the side opposite of the only missing side. So the character T would be a
// "Bottom" T, because the top line is missing and bottom is opposite of top. This is the conventional
// name for the unicode characters.
//
// An intuition for it is thinking about which direction is being ended.
#define GLYPH_T_RIGHT	(0x1C + UNICODE_BOX_LINES_START)
#define GLYPH_T_LEFT	(0x24 + UNICODE_BOX_LINES_START)
#define GLYPH_T_DOWN	(0x2C + UNICODE_BOX_LINES_START)
#define GLYPH_T_UP		(0x34 + UNICODE_BOX_LINES_START)

// Others
#define GLYPH_CROSS			(0x3C + UNICODE_BOX_LINES_START)
#define GLYPH_LINE_HORIZ	(0x0 + UNICODE_BOX_LINES_START)
#define GLYPH_LINE_VERT		(0x2 + UNICODE_BOX_LINES_START)

#define UTF8_CODEPOINT_WIDTH 4

#define BOARD_HEIGHT 6
#define BOARD_WIDTH 7

#define BOARD_PLAYER1 'O'
#define BOARD_PLAYER2 'X'
#define BOARD_EMPTY ' '

static char* boardTopLine;
static char* boardMiddleLine;
static char* boardBottomLine;

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
	} else {
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

// Returns number of characters copied.
size_t shrinkChar(IN wchar_t* original, IN size_t buffLen, OUT char* buffer) {
	for (size_t i = 0; i < buffLen; i++) {
		char c = (char)original[i];
		buffer[i] = c;
		if (c == '\0') {
			return i + 1;
		}
	}
	return buffLen;
}


size_t drawLine(IN int start, IN int tee, IN int end, IN size_t buffLen, OUT char* buffer) {
	int utfCodepoints[BOARD_WIDTH * 2 + 2] = { 0 };
	utfCodepoints[0] = start;
	for (int i = 0; i < BOARD_WIDTH; i++) {
		utfCodepoints[i * 2 + 1] = GLYPH_LINE_HORIZ;
		utfCodepoints[i * 2 + 2] = tee;
	}
	utfCodepoints[BOARD_WIDTH * 2] = end;
	utfCodepoints[BOARD_WIDTH * 2 + 1] = 0;

	return utfEncodeCodepoints(utfCodepoints, buffLen, NULL, buffer);
}

char* drawLineM(IN int start, IN int tee, IN int end) {
	size_t len = drawLine(start, tee, end, 0, NULL);
	if (len == SIZE_MAX) {
		return NULL;
	}
	char* ret = malloc(len * sizeof(char));
	if (ret == NULL) {
		printf("Out of memory!");
		exit(-1);
	}
	drawLine(start, tee, end, len, ret);
	return ret;
}

size_t drawRow(IN char* const rowData, IN size_t buffLen, OUT char* buffer) {
	int utfCodepoints[BOARD_WIDTH * 2 + 2] = { 0 };
	utfCodepoints[0] = GLYPH_LINE_VERT;
	for (int i = 0; i < BOARD_WIDTH; i++) {
		utfCodepoints[i * 2 + 1] = rowData[i];
		utfCodepoints[i * 2 + 2] = GLYPH_LINE_VERT;
	}
	utfCodepoints[BOARD_WIDTH * 2 + 1] = 0;

	return utfEncodeCodepoints(utfCodepoints, buffLen, NULL, buffer);
}

int main(void)
{
	UINT success = SetConsoleCP(CP_UTF8);
	if (success == 0) {
		printf("Setting console input to UTF-8 failed.");
		exit(-1);
	}
	success = SetConsoleOutputCP(CP_UTF8);
	if (success == 0) {
		printf("Setting console output to UTF-8 failed.");
		exit(-1);
	}
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (consoleHandle == INVALID_HANDLE_VALUE) {
		printf("Getting the console's output handle failed.");
		exit(-1);
	}
	
	CONSOLE_FONT_INFOEX currentFont = { 0 };
	currentFont.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	success = (UINT)GetCurrentConsoleFontEx(consoleHandle, TRUE, &currentFont);
	if (success == (UINT)FALSE) {
		DWORD errorCode = GetLastError();
		printf("Getting the current console font failed. (Error %lu)", errorCode);
		exit(-1);
	}

	char convertedFontName[ARRAYSIZE(currentFont.FaceName)];
	shrinkChar(currentFont.FaceName, ARRAYSIZE(convertedFontName), convertedFontName);
	if (strncmp(convertedFontName, "Consolas", 8) != 0) {
		printf("Warning: This program was written for Windows's default console font, Consolas. It uses unicode characters for drawing grid lines.\n");
		printf("Using a different font without unicode support may produce invalid characters. You are currently using the font '%ls'.\n", currentFont.FaceName);
		system("pause");
	}
	
	char board[BOARD_HEIGHT * BOARD_WIDTH];
	memset(board, (int)BOARD_EMPTY, sizeof(board));
	
	
	boardTopLine = drawLineM(GLYPH_TOP_LEFT, GLYPH_T_DOWN, GLYPH_TOP_RIGHT);
	boardMiddleLine = drawLineM(GLYPH_T_RIGHT, GLYPH_CROSS, GLYPH_T_LEFT);
	boardBottomLine = drawLineM(GLYPH_BOTTOM_LEFT, GLYPH_T_UP, GLYPH_BOTTOM_RIGHT);

	printf("%s\n", boardTopLine);
	char scratchBuffer[100] = { 0 };
	for (int i = 0; i < BOARD_HEIGHT - 1; i++) {
		drawRow(&board[i * BOARD_WIDTH], ARRAYSIZE(scratchBuffer), scratchBuffer);
		printf("%s\n%s\n", scratchBuffer, boardMiddleLine);
	}
	drawRow(&board[(BOARD_HEIGHT - 1) * BOARD_WIDTH], ARRAYSIZE(scratchBuffer), scratchBuffer);
	printf("%s\n%s\n\n\n", scratchBuffer, boardBottomLine);

	free(boardTopLine);
	free(boardMiddleLine);
	free(boardBottomLine);
}
