#include "screen.h"

#include <kernel/drivers/proc_io.h>

static int _line = 1;
static int _column = 1;
static u8 _color = 0x0F;
static int _isCursorEnabled = 0;

void ScPrintChar(char c)
{
	if (c == LF) 
	{
		_line++;
		_column = 0;
	}
	else if (c == CR) 
	{
		_column = 0;
	}
	else 
	{
		if (_column > COLUMNS - 1) 
		{
			_column = 0;
			_line++;
		}

		if (_line > LINES) 
		{
			ScScrollUp();
			_line--;
		}

		u8 * screen_ptr = (u8*)SCREEN_PTR + ((_column + (_line * COLUMNS)) * 2);
		*screen_ptr = c;
		*(screen_ptr + 1) = _color;
		_column++;
	}

	if (_column > COLUMNS - 1) {
		_column = 0;
		_line++;
	}

	if (_line > LINES) {
		ScScrollUp();
		_line--;
	}
}

void ScClear()
{
	int i = 0;
	u8 * screen_ptr = (u8*)SCREEN_PTR;
	for (; i < (LINES*COLUMNS) * 2; i += 2)
		screen_ptr[i] = ' ';
	_line = 0;
	_column = 0;
}

void ScSetColor(u8 value)
{
	if (value == WHITE)
		ScSetColorEx(BLACK, value, 0, 0);
	else
		ScSetColorEx(BLACK, value, 0, 1);
}

void ScSetColorEx(u8 background, u8 foreground, u8 blink, u8 intensity)
{
	_color = 0x0;
	u8 mask = (0x1 << 7);
	_color = (blink & mask);
	mask = (0x7 << 4);
	_color |= (mask & (background << 4));
	mask = (0x1 << 3);
	_color |= (mask & (intensity << 3));
	mask = 0x7;
	_color |= (mask & foreground);
}

void ScScrollUp()
{
	u8 * screen_ptr = (u8*)SCREEN_PTR;
	u8 * screen_end_ptr = (u8*)SCREEN_END_PTR;
	u8 * last_line_ptr = screen_ptr + ((1 + (LINES * COLUMNS)) * 2);

	while (screen_ptr <= screen_end_ptr) {
		if (screen_ptr < last_line_ptr) {
			*screen_ptr = *(screen_ptr + (COLUMNS * 2));
		}
		else {
			*screen_ptr = 0;
		}
		screen_ptr++;
	}
}

void ScSetBackground(u8 color)
{
	u8 * screen_ptr = (u8*)SCREEN_PTR + 1;
	u8 * screen_end_ptr = (u8*)SCREEN_END_PTR;

	for (; screen_ptr <= screen_end_ptr; screen_ptr += 2)
		*screen_ptr = (*screen_ptr & 0x8F) | ((color & 0x7) << 4);
}

void ScEnableCursor()
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | 13);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);

	_isCursorEnabled = 1;

	ScShowCursor();
}

void ScDisableCursor()
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);

	_isCursorEnabled = 0;
}

void ScMoveCursor(u8 x, u8 y)
{
	u16 c_pos;

	c_pos = y * COLUMNS + x;

	outb(0x3d4, 0x0f);
	outb(0x3d5, (u8)c_pos & 0xFF);
	outb(0x3d4, 0x0e);
	outb(0x3d5, (u8)(c_pos >> 8) & 0xFF);
}

void ScShowCursor(void)
{
	ScMoveCursor(_column, _line);
}

void ScHideCursor(void)
{
	ScMoveCursor(-1, -1);
}

int ScIsCursorEnabled()
{
	return _isCursorEnabled;
}