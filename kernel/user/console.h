#pragma once

#include <kernel/user/process.h>

struct Console
{
    Process * ownerProcess;
} typedef Console;

void CnslSetOwnerProcess(Process * process);
Process * CnslGetOwnerProcess();
void CnslFreeOwnerProcess();
void CnslHandleKey(const char key);
int CnslIsAvailable();

#ifdef __CONSOLE__
Console gConsole;
#else
extern Console gConsole;
#endif