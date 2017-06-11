#ifndef __DEF_SCREEN__
#define __DEF_SCREEN__

#include "types.h"

/**
   The text mode video screen is mapped to the physical address 0xB8000.
    - 80 columns
    - 25 rows
   A character is coded on 2 bytes :
    - 1st : the ASCII value
    - 2nd : its attributes :
       -> 1 bit  : blink
       -> 3 bits : background color
       -> 1 bit  : over intensity
       -> 3 bits : character color
 **/

#define SCREEN_PTR     0xB8000
#define SCREEN_END_PTR 0xB8FA0 // 80*25*2 = 4000 bytes
#define LINES          25
#define COLUMNS        80
#define LF             10 // new line, y++
#define CR             13 // x = 0

#define BLACK          0x0
#define WHITE          0x0F
#define RED            0x4E

void print (char * str);
void println (char * str);
void printChar (char c);
void clear ();
void setColor (u8 color);
void scrollUp ();

#endif
