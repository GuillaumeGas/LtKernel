#pragma once

#include <kernel/lib/types.h>

//#define CLOCK_DEBUG   

static u32 gClockTic = 0;
static u32 gClockSec = 0;

void DrvClock();