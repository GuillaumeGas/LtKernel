#define __CONSOLE__
#include "console.h"

#include <kernel/lib/stdio.h>

void CnslSetOwnerThread(Thread * thread)
{
    gConsole.ownerThread = thread;
}

Thread * CnslGetOwnerThread()
{
    return gConsole.ownerThread;
}

void CnslFreeOwnerThread()
{
    gConsole.ownerThread = NULL;
}

void CnslHandleKey(const char key)
{
    if (gConsole.ownerThread != NULL)
    {
		Thread * t = gConsole.ownerThread;
		t->console.consoleBuffer[t->console.bufferIndex++] = key;
        kprint("%c", key);

		if (key == '\n')
		{
			t->console.readyToBeFlushed = TRUE;
		}
    }
}

int CnslIsAvailable()
{
    return gConsole.ownerThread == NULL;
}