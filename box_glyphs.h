
#ifndef BOX_GLYPHS_H
#define BOX_GLYPHS_H

#pragma once

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

#endif // !BOX_GLYPHS_H