#include "syscalls.h"

#include <kernel/drivers/clock.h>

#include <kernel/user/process.h>
#include <kernel/user/console.h>
#include <kernel/user/process_manager.h>

#include <kernel/debug/debug.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

static void SysPrint(const char * str);
static void SysScanf(char * buffer);
static void SysExec(void * funAddr);
static void SysExit();
static void SysDebugListProcess();

enum SyscallId
{
    SYSCALL_PRINT = 1,
    SYSCALL_SCANF = 2,
    SYSCALL_EXEC  = 3,
    SYSCALL_EXIT  = 4,

    SYSCALL_DEBUG_LIST_PROCESS   = 10,
    SYSCALL_DEBUG_DUMP_REGISTERS = 11
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
        case SYSCALL_EXEC:
            SysExec((void *)context->ebx);
            break;
        case SYSCALL_EXIT:
            SysExit();
            break;
        case SYSCALL_DEBUG_LIST_PROCESS:
            SysDebugListProcess();
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

static void SysExec(void * funAddr)
{
    if (funAddr == NULL)
    {
        kprint("Error in syscall.c!SysExec(), invalid funAddr parameter (value : %x)\n", (u8 *)funAddr);
        return;
    }

    Process * currentProcess = GetCurrentProcess();

    if (currentProcess == NULL)
    {
        kprint("Error in syscall.c!SysExec(), GetCurrentProcess() returned NULL !\n");
        return;
    }

    int newProcPid = PmCreateProcess(funAddr, 500 /*tmp !*/, currentProcess);
	Process * newProcess = GetProcessFromPid(newProcPid);
	
	// TODO, implémenter syscall_wait !
	while (newProcess->state != PROCESS_STATE_DEAD);
}

static void SysExit()
{
    Process * currentProcess = GetCurrentProcess();

    if (currentProcess == NULL)
    {
        kprint("Error in syscall.c!SysExit(), GetCurrentProcess() returned NULL !\n");
        return;
    }

    currentProcess->state = PROCESS_STATE_DEAD;
	gNbProcess--;
}

static void SysDebugListProcess()
{
    PmPrintProcessList();
}