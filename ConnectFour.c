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
	HANDLE consoleOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (consoleOutputHandle == INVALID_HANDLE_VALUE) {
		printf("Getting the console's output handle failed.");
		exit(-1);
	}
	
	CONSOLE_FONT_INFOEX currentFont = { 0 };
	currentFont.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	success = (UINT)GetCurrentConsoleFontEx(consoleOutputHandle, TRUE, &currentFont);
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


	HANDLE consoleInputHandle = GetStdHandle(STD_INPUT_HANDLE);
	if (consoleInputHandle == INVALID_HANDLE_VALUE) {
		printf("Getting the console's input handle failed.\n");
		exit(-1);
	}

	// Note: In POSIX systems, use tcgetattr and tcsetattr.
	DWORD oldConsoleMode = 0;
	bool consoleModeSuccess = (bool)GetConsoleMode(consoleInputHandle, &oldConsoleMode);
	if (consoleModeSuccess) {
		DWORD newConsoleMode = oldConsoleMode & ~ENABLE_LINE_INPUT;
		consoleModeSuccess = (bool)SetConsoleMode(consoleInputHandle, newConsoleMode);
		if (!consoleModeSuccess) {
			// SetConsoleMode sometimes returns error 87 (invalid parameter) even when
			// the call is successful. Check if there REALLY was an error by comparing
			// against the expected flags.
			DWORD currentMode = 0;
			GetConsoleMode(consoleInputHandle, &currentMode);
			if (currentMode == newConsoleMode) {
				consoleModeSuccess = true;
			}
		}
	}
	if (!consoleModeSuccess) {
		printf("Warning: Unable to enable raw input mode. Please use the enter key to input options.\n");
	}
	

	Board* board = newBoard();
	while (getWinner(board) == BOARD_EMPTY) {
		printf("\n\n\n\n\n\n\n\n");
		drawBoard(board);
		printf("Player %c, select a column number. Press q to quit.\n", currentTurn(board));
		char input = '\0';
		while (!(isdigit(input) || input == 'q' || input == 'Q')) {
			scanf_s("%c", &input, 1);
		}
		if (input == 'q' || input == 'Q') {
			break;
		}
		int column = (int)(input - '0');
		printf("%d\n", column);
		if (column >= BOARD_WIDTH) {
			printf("Invalid column number.\n");
			continue;
		}

		if (!placeInColumn(board, column)) {
			printf("Column %d is already full.\n", column);
		}
	}
	
	char winner = getWinner(board);
	if (winner != BOARD_EMPTY) {
		drawBoard(board);
		printf("Player %c won! (%d)", winner, (int)winner);
	}
	else {
		printf("Player %c conceded.", currentTurn(board));
	}

	freeBoard(board);
	if (consoleModeSuccess) {
		bool returnToNormalSuccess = (bool)SetConsoleMode(consoleInputHandle, oldConsoleMode);
		if (!returnToNormalSuccess) {
			printf("Error: Unable to disable raw input mode. Your terminal may behave strangely. I recommend\n");
			printf("you close and re-open your terminal.");
		}
	}
}
