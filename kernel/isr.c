#include "screen.h"
#include "types.h"
#include "proc_io.h"
#include "kbmap.h"

void default_isr (void)
{
    println ("Unhandled interrupt");
}

void clock_isr (void)
{
    static int tic = 0;
    static int sec = 0;
    tic++;
    if (tic % 100 == 0){
	sec++;
	tic = 0;
	/* if (sec % 2 == 0) */
	/*     println ("tic"); */
	/* else */
	/*     println ("tac"); */
    }
}

void keyboard_isr (void)
{
    uchar i;
    static int lshift_enable;
    static int rshift_enable;
    static int alt_enable;
    static int ctrl_enable;

    do {
	i = inb(0x64);
    } while ((i & 0x01) == 0);

    i = inb(0x60);
    i--;

    if (i < 0x80) {         /* touche enfoncee */
	switch (i) {
	case 0x29:
	    lshift_enable = 1;
	    break;
	case 0x35:
	    rshift_enable = 1;
	    break;
	case 0x1C:
	    ctrl_enable = 1;
	    break;
	case 0x37:
	    alt_enable = 1;
	    break;
	default:
	    printChar(kbdmap
		   [i * 4 + (lshift_enable || rshift_enable)]);
	}
    } else {                /* touche relachee */
	i -= 0x80;
	switch (i) {
	case 0x29:
	    lshift_enable = 0;
	    break;
	case 0x35:
	    rshift_enable = 0;
	    break;
	case 0x1C:
	    ctrl_enable = 0;
	    break;
	case 0x37:
	    alt_enable = 0;
	    break;
	}
    }
}
