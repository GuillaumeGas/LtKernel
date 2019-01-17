#define __CLOCK__
#include "clock.h"

#include <kernel/lib/stdio.h>

void DrvClock()
{
    gClockTic++;
    if (gClockTic % 100 == 0)
    {
        gClockSec++;
        gClockTic = 0;
#ifdef CLOCK_DEBUG
        if (gClockSec % 2 == 0)
            kprint(".");
#endif
    }
}