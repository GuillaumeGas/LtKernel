#define __CLOCK__
#include "clock.h"

#include <kernel/lib/stdio.h>

//#define CLOCK_DEBUG

void DrvClock()
{
    gClockTic++;
    if (gClockTic % 500 == 0)
    {
        gClockSec++;
#ifdef CLOCK_DEBUG
        if (gClockSec % 2 == 0)
            kprint(".");
#endif
    }
}