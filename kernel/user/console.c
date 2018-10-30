#define __CONSOLE__
#include "console.h"

#include <kernel/lib/stdio.h>

void CnslSetOwnerProcess(Process * process)
{
    gConsole.ownerProcess = process;
}

Process * CnslGetOwnerProcess()
{
    return gConsole.ownerProcess;
}

void CnslFreeOwnerProcess()
{
    gConsole.ownerProcess = NULL;
}

void CnslHandleKey(const char key)
{
    if (gConsole.ownerProcess != NULL)
    {
        kprint("%c", key);
        //ProcessHandleInput(key);
    }
}

int CnslIsAvailable()
{
    return gConsole.ownerProcess == NULL;
}