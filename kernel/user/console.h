#pragma once

#include <kernel/user/thread.h>

struct Console
{
    Thread * ownerThread;
} typedef Console;

void CnslSetOwnerThread(Thread * thread);
Thread * CnslGetOwnerThread();
void CnslFreeOwnerThread();
void CnslHandleKey(const char key);
int CnslIsAvailable();

#ifdef __CONSOLE__
Console gConsole;
#else
extern Console gConsole;
#endif