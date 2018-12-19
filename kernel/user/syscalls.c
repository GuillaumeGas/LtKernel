#include "syscalls.h"

#include <kernel/drivers/clock.h>
#include <kernel/drivers/screen.h>

#include <kernel/user/process.h>
#include <kernel/user/console.h>
#include <kernel/user/process_manager.h>

#include <kernel/debug/debug.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

static KeStatus SysPrint(const char * str);
static KeStatus SysScanf(char * buffer);
static KeStatus SysExec(void * funAddr);
static KeStatus SysWait(int pid);
static KeStatus SysExit();
static KeStatus SysDebugListProcess();

enum SyscallId
{
    SYSCALL_PRINT = 1,
    SYSCALL_SCANF = 2,
    SYSCALL_EXEC  = 3,
	SYSCALL_WAIT  = 4,
    SYSCALL_EXIT  = 5,

    SYSCALL_DEBUG_LIST_PROCESS   = 10,
    SYSCALL_DEBUG_DUMP_REGISTERS = 11
};

void SyscallHandler(int syscallNumber, InterruptContext * context)
{
	int ret = 0;

    switch (syscallNumber)
    {
        case SYSCALL_PRINT:
            ret = SysPrint((char *)context->ebx);
            break;
        case SYSCALL_SCANF:
            ret = SysScanf((char *)context->ebx);
            break;
        case SYSCALL_EXEC:
            ret = SysExec((void *)context->ebx);
            break;
		case SYSCALL_WAIT:
			ret = SysWait((int)context->ebx);
			break;
        case SYSCALL_EXIT:
            ret = SysExit();
            break;
        case SYSCALL_DEBUG_LIST_PROCESS:
            ret = SysDebugListProcess();
            break;
        default:
            kprint("SyscallHandler() : unknown system call !\n");
    }
	context->eax = ret;
}

static KeStatus SysPrint(const char * str)
{
    kprint(str);
	return STATUS_SUCCESS;
}

static KeStatus SysScanf(char * buffer)
{
    Process * currentProcess = GetCurrentProcess();

	ScEnableCursor();

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

	ScDisableCursor();

	MmCopy((u8 *)currentProcess->console.consoleBuffer, (u8 *)buffer, currentProcess->console.bufferIndex + 1);
	buffer[currentProcess->console.bufferIndex] = '\0';

	CnslFreeOwnerProcess();

	currentProcess->console.bufferIndex = 0;
	currentProcess->console.readyToBeFlushed = FALSE;

	return STATUS_SUCCESS;
}

static KeStatus SysExec(void * funAddr)
{
    if (funAddr == NULL)
    {
        kprint("Error in syscall.c!SysExec(), invalid funAddr parameter (value : %x)\n", (u8 *)funAddr);
        return STATUS_INVALID_PARAMETER;
    }

    Process * currentProcess = GetCurrentProcess();

    if (currentProcess == NULL)
    {
        kprint("Error in syscall.c!SysExec(), GetCurrentProcess() returned NULL !\n");
        return STATUS_PROCESS_NOT_FOUND;
    }

    int newProcPid = PmCreateProcess(funAddr, 500 /*tmp !*/, currentProcess);
	/*Process * newProcess = GetProcessFromPid(newProcPid);*/
	
	// TODO, implémenter syscall_wait !
	//while (newProcess->state != PROCESS_STATE_DEAD);

	return newProcPid;
}

static KeStatus SysWait(int pid)
{
	// On empêche d'attendre sur le processus system pour l'instant...
	if (pid == 0)
	{
		// TODO : ajouter des logs de debug qui s'affichent que si nécessaire...
		kprint("Can't wait on system process !\n");
		return STATUS_INVALID_PARAMETER;
	}

	Process * currentProc = GetCurrentProcess();
	Process * process = GetProcessFromPid(pid);

	if (currentProc == NULL)
	{
		kprint("SysWait() : GetCurrentProcess() returned NULL !\n");
		return STATUS_FAILURE;
	}

	if (currentProc->pid == pid)
	{
		kprint("SysWait() : a process can't wait its own death !\n");
		return STATUS_INVALID_PARAMETER;
	}

	if (process == NULL)
	{
		kprint("SysWait() : can't find process with pid : %d\n", pid);
		return STATUS_PROCESS_NOT_FOUND;
	}

	while (process->state == PROCESS_STATE_ALIVE);

	return STATUS_SUCCESS;
}

static KeStatus SysExit()
{
    Process * currentProcess = GetCurrentProcess();

    if (currentProcess == NULL)
    {
        kprint("Error in syscall.c!SysExit(), GetCurrentProcess() returned NULL !\n");
		return STATUS_FAILURE;
    }

    currentProcess->state = PROCESS_STATE_DEAD;
	gNbProcess--;

	return STATUS_SUCCESS;
}

static KeStatus SysDebugListProcess()
{
    PmPrintProcessList();
	return STATUS_SUCCESS;
}