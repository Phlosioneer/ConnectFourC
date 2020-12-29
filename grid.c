

#include <stdio.h>
#include <limits.h>
#include "grid.h"

#include "box_glyphs.h"
#include "utf8.h"

static char* boardTopLine;
static char* boardMiddleLine;
static char* boardBottomLine;

struct Board {
	char cells[BOARD_WIDTH * BOARD_HEIGHT];
};

Board* newBoard(void) {
	Board* ret = malloc(sizeof(Board));
	if (ret == NULL) {
		printf("Out of memory!");
		exit(-1);
	}

	memset(&ret->cells, (int)BOARD_EMPTY, sizeof(Board));
	return ret;
}

void freeBoard(Board* board) {
	free(board);
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

void drawBoard(Board* board) {
	if (boardTopLine == NULL) {
		boardTopLine = drawLineM(GLYPH_TOP_LEFT, GLYPH_T_DOWN, GLYPH_TOP_RIGHT);
		boardMiddleLine = drawLineM(GLYPH_T_RIGHT, GLYPH_CROSS, GLYPH_T_LEFT);
		boardBottomLine = drawLineM(GLYPH_BOTTOM_LEFT, GLYPH_T_UP, GLYPH_BOTTOM_RIGHT);
	}

	printf("%s\n", boardTopLine);
	char scratchBuffer[100] = { 0 };
	for (int i = 0; i < BOARD_HEIGHT - 1; i++) {
		drawRow(&board->cells[i * BOARD_WIDTH], ARRAYSIZE(scratchBuffer), scratchBuffer);
		printf("%s\n%s\n", scratchBuffer, boardMiddleLine);
	}
	drawRow(&board->cells[(BOARD_HEIGHT - 1) * BOARD_WIDTH], ARRAYSIZE(scratchBuffer), scratchBuffer);
	printf("%s\n%s\n\n\n", scratchBuffer, boardBottomLine);
}