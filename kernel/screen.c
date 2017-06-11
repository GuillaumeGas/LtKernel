#include "screen.h"

int _line = 0;
int _column = 0;
char _color = 0x0F;

void printChar (char c) {
    if (c == LF) {
	_line++;
	_column = 0;
    } else if (c == CR) {
	_column = 0;
    } else {
	u8 * screen_ptr = (u8*) (SCREEN_PTR + 2 * _column + (COLUMNS*2) * _line);
	*screen_ptr = c;
	if (_color == 0x0F) {
	    *(screen_ptr+1) = WHITE;
	} else {
	    *(screen_ptr+1) = RED;
	}
	_column++;
    }

    if (_column > COLUMNS) {
	_column = 0;
	_line++;
    }
}

void print (char * str) {
    while ((*str) != 0)
    	printChar (*(str++));
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
