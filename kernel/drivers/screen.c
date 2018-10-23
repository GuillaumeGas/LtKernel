#include "screen.h"

static int _line = 1;
static int _column = 1;
static u8 _color = 0x0F;

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
	for (; i < (LINES*COLUMNS) * 2; i++)
		*(screen_ptr + i) = 0;
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
