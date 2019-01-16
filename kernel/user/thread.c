#include "thread.h"

#include <kernel/user/process_manager.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("USER", LOG_LEVEL, format, ##__VA_ARGS__)

static int s_ThreadId = 0;

Thread * GetCurrentThread()
{
    return gCurrentThread;
}

Thread * GetThreadFromTid(int tid)
{
    if (tid < 0)
    {
        KLOG(LOG_ERROR, "Invalid tid : %d", tid);
        return NULL;
    }

    return (Thread *)ListGet(gThreadsList, tid);
}

static KeStatus InitThread(Thread * thread, u32 entryAddr);

/*
| - CreateMainThread (p)
|  - Init du pointeur sur process
|  - Réservation d'une page pour la pile, on garde son adresse physique en mémoire
|  - Init des registres
*/
KeStatus CreateMainThread(Process * process, u32 entryAddr, Thread ** mainThread)
{
    KeStatus status = STATUS_FAILURE;
    Thread * thread = NULL;

    if (process == NULL)
    {
        KLOG(LOG_ERROR, "Invalid process parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (mainThread == NULL)
    {
        KLOG(LOG_ERROR, "Invalid mainThread parameter");
        return STATUS_NULL_PARAMETER;
    }

    thread = (Thread *)kmalloc(sizeof(Thread));
    if (thread == NULL)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Thread));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    thread->tid = s_ThreadId++;
    thread->startExecutionTime = 0;
    thread->state = THREAD_STATE_INIT;
    thread->process = process;

    status = InitThread(thread, entryAddr);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "InitThread() failed with code %d", status);
		goto clean;
	}

	*mainThread = thread;
	thread = NULL;

	gNbThreads++;

    status = STATUS_SUCCESS;

clean:
	if (thread != NULL)
	{
		kfree(thread);
		thread = NULL;
	}

    return status;
}

void SwitchToMemoryMappingOfThread(Thread * thread)
{
	if (thread == NULL)
	{
		KLOG(LOG_ERROR, "Invalid thread parameter");
		return;
	}

	SaveCurrentMemoryMapping();

	PageDirectory * pageDirectory = &(thread->process->pageDirectory);
	_setCurrentPagesDirectory(pageDirectory->pdEntry);
}

void ThreadPrepare(Thread * thread)
{
	SwitchToMemoryMappingOfThread(thread);

	AddPageToPageDirectory((u8 *)thread->stackPage.vAddr, (u8 *)thread->stackPage.vAddr, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, thread->process->pageDirectory);

	RestoreMemoryMapping();
}

static KeStatus InitThread(Thread * thread, u32 entryAddr)
{
	KeStatus status = STATUS_FAILURE;
    Page kernelStackPage = { 0 };
	u32 * pUserStack = 0;
	u32 * vUserStack = 0;

    if (thread == NULL)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return STATUS_NULL_PARAMETER;
    }

    kernelStackPage = PageAlloc();

    pUserStack = (u32 *)GetFreePage();
    if (pUserStack == NULL)
    {
        KLOG(LOG_ERROR, "Couldn't find a free page");
		status = STATUS_PHYSICAL_MEMORY_FULL;
        goto clean;
    }

    vUserStack = (u32 *)(USER_STACK_V_ADDR);
 
	thread->stackPage.vAddr = vUserStack;
	thread->stackPage.pAddr = pUserStack;

	thread->regs.ss = USER_STACK_SEG_SELECTOR;
	thread->regs.cs = USER_CODE_SEG_SELECTOR;
	thread->regs.ds = USER_DATA_SEG_SELECTOR;
	thread->regs.es = USER_DATA_SEG_SELECTOR;
	thread->regs.fs = USER_DATA_SEG_SELECTOR;
	thread->regs.gs = USER_DATA_SEG_SELECTOR;

	thread->regs.eax = 0;
	thread->regs.ebp = 0;
	thread->regs.ecx = 0;
	thread->regs.edx = 0;
	thread->regs.edi = 0;
	thread->regs.esi = 0;

	thread->regs.esp = (u32)vUserStack - (u32)(sizeof(void*));
	thread->regs.eip = entryAddr;

	thread->regs.eflags = 0x200 & 0xFFFFBFFF;
	thread->kstack.esp0 = (u32)kernelStackPage.vAddr + PAGE_SIZE;
	thread->kstack.ss0 = 0x10;

	MmSet((u8 *)thread->console.consoleBuffer, 0, THREAD_CONSOLE_BUFFER_SIZE);
	thread->console.bufferIndex = 0;
	thread->console.readyToBeFlushed = FALSE;

	status = STATUS_SUCCESS;

clean:
	return status;
}