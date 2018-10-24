#pragma once

#include <kernel/lib/types.h>

//#define CLOCK_DEBUG   

#ifdef __CLOCK__
u32 gClockTic = 0;
u32 gClockSec = 0;
#else
extern u32 gClockTic;
extern u32 gClockSec;
#endif

void DrvClock();