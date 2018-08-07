#pragma once

#ifdef __SCHEDULER__
unsigned int g_clock = 0;
#else
extern unsigned int g_clock;
#endif

void schedule();