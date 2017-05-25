// Screen properties
#define SCREEN_PTR 0xB8000
#define LINES 25
#define COLUMNS 80
#define LF 10

#define BLACK 0x0
#define WHITE 0x0F
#define RED   0x4E

typedef unsigned char u8;

int _line = 0;
int _column = 0;
char _color = 0x0F;

void printChar (char c) {
    if (_line < LINES) {
	if (_column >= COLUMNS) {
	    _column = 0;
	    _line++;
	}
	if (c == LF) {
	    _line++;
	    _column = 0;
	} else {
	    u8 * screen_ptr = SCREEN_PTR;
	    u8 pos = (_column + (_line * COLUMNS)) * 2;
	    screen_ptr[pos] = c;
	    if (_color == 0x0F) {
		screen_ptr[pos+1] = WHITE;
	    } else {
		screen_ptr[pos+1] = RED;
	    }
	    _column++;
	}
    }
}

void print (u8 * str) {
    while ((*str) != 0)
    	printChar (*(str++));
}

void clear () {
    int i = 0;
    u8 * screen_ptr = SCREEN_PTR;
    for (; i < (LINES*COLUMNS)*2; i++)
	*(screen_ptr+i) = 0;
    _line = 0;
    _column = 0;
}

void setColor (u8 value) {
    _color = value;
}
