#include "screen.h"

int _line = 1;
int _column = 1;
u8 _color = 0x0F;

void sc_printChar (char c)
{
    if (c == LF) {
	_line++;
	_column = 0;
    } else if (c == CR) {
	_column = 0;
    } else {
	u8 * screen_ptr = (u8*) SCREEN_PTR + ((_column + (_line * COLUMNS))*2);
	*screen_ptr = c;
	*(screen_ptr+1) = _color;
	_column++;
    }

    if (_column > COLUMNS - 1) {
	_column = 0;
	_line++;
    }

    if (_line > LINES) {
    	sc_scrollUp ();
    	_line--;
    }
}

void sc_clear ()
{
    int i = 0;
    u8 * screen_ptr = (u8*)SCREEN_PTR;
    for (; i < (LINES*COLUMNS)*2; i++)
	*(screen_ptr+i) = 0;
    _line = 0;
    _column = 0;
}

void sc_setColor (u8 value)
{
    sc_setColorEx (BLACK, value, 0, 0);
}

void sc_setColorEx (u8 background, u8 foreground, u8 blink, u8 intensity)
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

void sc_scrollUp ()
{
    u8 * screen_ptr = (u8*) SCREEN_PTR;
    u8 * screen_end_ptr = (u8*) SCREEN_END_PTR;
    u8 * last_line_ptr = screen_ptr + ((1 + (LINES * COLUMNS)) * 2);
    
    while (screen_ptr <= screen_end_ptr) {
	if (screen_ptr < last_line_ptr) {
	    *screen_ptr = *(screen_ptr + (COLUMNS * 2));
	} else {
	    *screen_ptr = 0;
	}
	screen_ptr++;
    }
}
