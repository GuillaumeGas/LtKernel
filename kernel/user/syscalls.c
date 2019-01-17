#include "syscalls.h"

#include <kernel/drivers/clock.h>
#include <kernel/drivers/screen.h>

#include <kernel/user/process.h>
#include <kernel/user/thread.h>
#include <kernel/user/console.h>
#include <kernel/user/process_manager.h>

#include <kernel/debug/debug.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("USER", LOG_LEVEL, format, ##__VA_ARGS__)

static KeStatus SysPrint(const char * str, int * ret);
static KeStatus SysScanf(char * buffer, int * ret);
static KeStatus SysExec(void * funAddr, int * ret);
static KeStatus SysWait(int pid, int * ret);
static KeStatus SysExit(int * ret);
static KeStatus SysDebugListProcess(int * ret);

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
	KeStatus status = STATUS_FAILURE;

    switch (syscallNumber)
    {
        case SYSCALL_PRINT:
			status = SysPrint((char *)context->ebx, &ret);
            break;
        case SYSCALL_SCANF:
			status = SysScanf((char *)context->ebx, &ret);
            break;
        case SYSCALL_EXEC:
			status = SysExec((void *)context->ebx, &ret);
            break;
		case SYSCALL_WAIT:
			status = SysWait((int)context->ebx, &ret);
			break;
        case SYSCALL_EXIT:
			status = SysExit(&ret);
            break;
        case SYSCALL_DEBUG_LIST_PROCESS:
			status = SysDebugListProcess(&ret);
            break;
        default:
            KLOG(LOG_ERROR, "Unknown system call !");
    }
	context->eax = ret;

	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "An error occured while executing syscall");
	}
}

static KeStatus SysPrint(const char * str, int * ret)
{
	if (str == NULL)
	{
		KLOG(LOG_ERROR, "Invalid str parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

    kprint(str);
    *ret = 0;

	return STATUS_SUCCESS;
}

static KeStatus SysScanf(char * buffer, int * ret)
{
	if (buffer == NULL)
	{
		KLOG(LOG_ERROR, "Invalid buffer parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

    Thread * currentThread = GetCurrentThread();
	if (currentThread == NULL)
	{
		KLOG(LOG_ERROR, "GetCurrentThread() returned NULL");
		*ret = -1; // TODO : definir quelques codes d'erreur de retour, types qui seront utilisés dans les libs userland
		return STATUS_PROCESS_NOT_FOUND;
	}

	ScEnableCursor();

    if (CnslIsAvailable())
    {
        CnslSetOwnerThread(currentThread);
    }
	else
	{
		Thread * currentCnslOwner = CnslGetOwnerThread();

		if (currentCnslOwner != currentThread)
		{
			while (!CnslIsAvailable());

			// TODO : Add lock/mutex whatever ?
			CnslSetOwnerThread(currentThread);
		}
	}

	while (!currentThread->console.readyToBeFlushed);

	ScDisableCursor();

	MmCopy((u8 *)currentThread->console.consoleBuffer, (u8 *)buffer, currentThread->console.bufferIndex + 1);
	buffer[currentThread->console.bufferIndex] = '\0';

	CnslFreeOwnerThread();

	currentThread->console.bufferIndex = 0;
	currentThread->console.readyToBeFlushed = FALSE;

	*ret = 0;

	return STATUS_SUCCESS;
}

static KeStatus SysExec(void * funAddr, int * ret)
{
	KeStatus status = STATUS_FAILURE;

    if (funAddr == NULL)
    {
        KLOG(LOG_ERROR, "Invalid funAddr parameter");
        return STATUS_NULL_PARAMETER;
    }

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

 //   Process * currentProcess = GetCurrentProcess();
 //   if (currentProcess == NULL)
 //   {
	//	KLOG(LOG_ERROR, "GetCurrentProcess() returned NULL");
	//	status = STATUS_PROCESS_NOT_FOUND;
	//	*ret = -1;
	//	goto clean;
 //   }

	//int newProcPid = -1;
	//status = PmCreateProcess(funAddr, 500 /*tmp !*/, currentProcess, &newProcPid);
	//if (FAILED(status) || newProcPid == -1)
	//{
	//	KLOG(LOG_ERROR, "PmCreateProcess() failed with status : %d", status);
	//	*ret = -1;
	//	goto clean;
	//}
	/*Process * newProcess = GetProcessFromPid(newProcPid);*/
	
	// TODO, implémenter syscall_wait !
	//while (newProcess->state != PROCESS_STATE_DEAD);

	//*ret = newProcPid;

clean:
	return status;
}

static KeStatus SysWait(int pid, int * ret)
{
	KeStatus status = STATUS_FAILURE;

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

	// On empêche d'attendre sur le processus system pour l'instant...
	if (pid == 0)
	{
		KLOG(LOG_DEBUG, "Can't wait on system process !");
		status = STATUS_INVALID_PARAMETER;
		goto clean;
	}

	Process * currentProc = GetCurrentProcess();
	Process * process = GetProcessFromPid(pid);

	if (currentProc == NULL)
	{
		KLOG(LOG_ERROR, "GetCurrentProcess() returned NULL");
		status = STATUS_PROCESS_NOT_FOUND;
		goto clean;
	}

	if (currentProc->pid == pid)
	{
		KLOG(LOG_ERROR, "A process can't wait its own death !");
		status = STATUS_FAILURE;
		goto clean;
	}

	if (process == NULL)
	{
		KLOG(LOG_ERROR, "Can't find process with pid : %d", pid);
		status = STATUS_PROCESS_NOT_FOUND;
		goto clean;
	}

	while (process->state == PROCESS_STATE_ALIVE);

	status = STATUS_SUCCESS;
	*ret = 0;

clean:
	return status;
}

static KeStatus SysExit(int * ret)
{
	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

    Process * currentProcess = GetCurrentProcess();
    if (currentProcess == NULL)
    {
        KLOG(LOG_ERROR, "GetCurrentProcess() returned NULL !");
		return STATUS_PROCESS_NOT_FOUND;
    }

    currentProcess->state = PROCESS_STATE_DEAD;
	gNbProcess--;

	*ret = 0;
	return STATUS_SUCCESS;
}

static KeStatus SysDebugListProcess(int * ret)
{
	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

    PmPrintProcessList();
	*ret = 0;
	return STATUS_SUCCESS;
}