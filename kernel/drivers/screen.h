#ifndef __DEF_SCREEN__
#define __DEF_SCREEN__

#include <kernel/lib/types.h>

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
#define BLUE           0x1
#define GREEN          0x2
#define CYAN           0x3
#define RED            0x4
#define MAGENTA        0x5
#define YELLOW         0x6
#define WHITE          0x7

void ScPrintChar(char c);
void ScClear();
void ScSetColor (u8 color);
void ScSetColorEx (u8, u8, u8, u8);
void ScScrollUp ();
void ScSetBackground (u8 color);
void ScMoveCursor(u8 x, u8 y);
void ScEnableCursor();
void ScDisableCursor();
int ScIsCursorEnabled();
void ScShowCursor(void);
void ScHideCursor(void);

#endif
