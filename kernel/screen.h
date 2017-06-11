#ifndef __DEF_SCREEN__
#define __DEF_SCREEN__

#include "types.h"

#define SCREEN_PTR     0xB8000
#define SCREEN_END_PTR 0xB8FA0
#define LINES          25
#define COLUMNS        80
#define LF             10 // new line, y++
#define CR             13 // x = 0

#define BLACK          0x0
#define WHITE          0x0F
#define RED            0x4E

void print (char * str);
void printChar (char c);
void clear ();
void setColor (u8 color);

#endif
