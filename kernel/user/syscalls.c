#include "syscalls.h"

#include <kernel/drivers/clock.h>
#include <kernel/user/process.h>
#include <kernel/user/console.h>
#include <kernel/debug/debug.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

static void SysPrint(const char * str);
static void SysScanf(char * buffer);

enum SyscallId
{
    SYSCALL_PRINT = 1,
    SYSCALL_SCANF = 2
};

void SyscallHandler(int syscallNumber, InterruptContext * context)
{
    switch (syscallNumber)
    {
        case SYSCALL_PRINT:
            SysPrint((char *)context->ebx);
            break;
        case SYSCALL_SCANF:
            SysScanf((char *)context->ebx);
            break;
        default:
            kprint("SyscallHandler() : unknown system call !\n");
    }
}

static void SysPrint(const char * str)
{
    kprint(str);
}

static void SysScanf(char * buffer)
{
    Process * currentProcess = GetCurrentProcess();

    if (CnslIsAvailable())
    {
        CnslSetOwnerProcess(currentProcess);
    }
	else
	{
		Process * currentCnslOwner = CnslGetOwnerProcess();

		if (currentCnslOwner != currentProcess)
		{
			while (!CnslIsAvailable());

			// TODO : Add lock/mutex whatever ?
			CnslSetOwnerProcess(currentProcess);
		}
	}

	while (!currentProcess->console.readyToBeFlushed);

	MmCopy((u8 *)currentProcess->console.consoleBuffer, (u8 *)buffer, currentProcess->console.bufferIndex + 1);
	buffer[currentProcess->console.bufferIndex] = '\0';

	CnslFreeOwnerProcess();

	currentProcess->console.bufferIndex = 0;
	currentProcess->console.readyToBeFlushed = FALSE;
}