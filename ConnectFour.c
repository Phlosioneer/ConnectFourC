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

#include "common.h"
#include "grid.h"

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

	Board* board = newBoard();
	drawBoard(board);

	freeBoard(board);
}
