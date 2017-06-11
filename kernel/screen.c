#include "screen.h"

int _line = 1;
int _column = 1;
u8 _color = 0x0F;

void printChar (char c) {
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

    if (_column > COLUMNS) {
	_column = 0;
	_line++;
    }

    if (_line > LINES) {
    	scrollUp ();
    	_line--;
    }
}

void print (char * str) {
    while ((*str) != 0)
    	printChar (*(str++));
}

void println (char * str) {
    print (str);
    printChar ('\n');
}

void clear () {
    int i = 0;
    u8 * screen_ptr = (u8*)SCREEN_PTR;
    for (; i < (LINES*COLUMNS)*2; i++)
	*(screen_ptr+i) = 0;
    _line = 0;
    _column = 0;
}

void setColor (u8 value) {
    _color = value;
}

void scrollUp () {
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
