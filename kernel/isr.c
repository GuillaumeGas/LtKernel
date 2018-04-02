#include "screen.h"
#include "types.h"

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
	if (sec % 2 == 0)
	    println ("tic");
	else
	    println ("tac");
    }
}
