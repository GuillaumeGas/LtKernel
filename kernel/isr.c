#include "screen.h"
#include "types.h"

void isr_default_int (void)
{
    println ("Unhandled interrupt");
}

void isr_clock_int (void)
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
