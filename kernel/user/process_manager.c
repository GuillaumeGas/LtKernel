#include <kernel/kernel.h>
#include <kernel/scheduler.h>

#include <kernel/init/vmm.h>
#include <kernel/init/gdt.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/debug/debug.h>

#include "process.h"
#include "thread.h"

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("USER", LOG_LEVEL, format, ##__VA_ARGS__)

#define __PROCESS_MANAGER__
#include "process_manager.h"

void PmInit()
{
	// TODO : check list creation
	gProcessList = ListCreate();
	gThreadsList = ListCreate();
}

KeStatus PmCreateProcess(u32 entryAddr, Process ** newProcess, Process * parent)
{
	PageDirectory processPd = { 0 };
	Process * process = NULL;
	KeStatus status = STATUS_FAILURE;

	if (entryAddr == 0)
	{
		KLOG(LOG_ERROR, "Invalid entryAddr parameter");
		return STATUS_INVALID_PARAMETER;
	}

	if (newProcess == NULL)
	{
		KLOG(LOG_ERROR, "Invalid newProcess parameter");
		return STATUS_NULL_PARAMETER;
	}

	processPd = CreateProcessPageDirectory();

	KLOG(LOG_DEBUG, "pd : %x", processPd.pdEntry);

	status = CreateProcess(processPd, entryAddr, parent, &process);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "CreateProcess() failed with code %d", status);
		goto clean;
	}

	ListPush(gProcessList, process);
	gNbProcess++;
	*newProcess = process;

clean:
	return status;
}

void PmStartThread(int tid)
{
	if (tid == -1)
	{
		KLOG(LOG_ERROR, "Invalid tid parameter");
		return;
	}
	else
	{
		gCurrentThread = ListGet(gThreadsList, tid);
		if (gCurrentThread == NULL)
		{
			KLOG(LOG_ERROR, "Thread %d not found", tid);
			return;
		}

		gCurrentThread->startExecutionTime = g_clock;
		gCurrentThread->state = THREAD_STATE_RUNNING;
		
		gCurrentProcess = gCurrentThread->process;
		gCurrentProcess->state = PROCESS_STATE_RUNNING;

		ThreadPrepare(gCurrentThread);

		gTss.esp0 = gCurrentThread->kstack.esp0;
        gTss.ss0 = gCurrentThread->kstack.ss0;

		_start_process(
			gCurrentThread->process->pageDirectory.pdEntry,
			gCurrentThread->regs.ss,
			gCurrentThread->regs.esp,
			gCurrentThread->regs.eflags,
			gCurrentThread->regs.cs,
			gCurrentThread->regs.eip,
			gCurrentThread->regs.eax,
			gCurrentThread->regs.ecx,
			gCurrentThread->regs.edx,
			gCurrentThread->regs.ebx,
			gCurrentThread->regs.ebp,
			gCurrentThread->regs.esi,
			gCurrentThread->regs.edi,
			gCurrentThread->regs.ds,
			gCurrentThread->regs.es,
			gCurrentThread->regs.fs,
			gCurrentThread->regs.gs,
			gCurrentThread->regs.cs == K_CODE_SEG_SELECTOR ? KERNEL : USER
		);
	}
}

static void CleanProcessCallback(void * param)
{
	if (param == NULL)
	{
		KLOG(LOG_ERROR, "Invalid parameter");
		return;
	}

	//Process * process = (Process *)param;
 //   ListDestroy(process->pageDirectory.pageTableList);
	//kfree(process);
}

void PmCleanCallback()
{
	asm("int $0x3"::);
	ListDestroyEx(gProcessList, CleanProcessCallback);
}

void PmDumpProcess(Process * process)
{
	if (process == NULL)
	{
		KLOG(LOG_ERROR, "Invalid process parameter");
		return;
	}

    //kprint("== Process %d ==\n", process->pid);
    //kprint(" - kstack_esp0 : %x\n", process->kstack.esp0);
    //kprint(" - page_directory : %x\n", process->pageDirectory.pdEntry);
    //kprint(" - start_execution_time : %x\n", process->startExcecutionTime);
    //kprint(" - ss : %x\n", process->regs.ss);
    //kprint(" - esp : %x\n", process->regs.esp);
    //kprint(" - eflags : %x\n", process->regs.eflags);
    //kprint(" - cs : %x\n", process->regs.cs);
    //kprint(" - eip : %x\n", process->regs.eip);
    //kprint(" - eax : %x\n", process->regs.eax);
    //kprint(" - ecx : %x\n", process->regs.ecx);
    //kprint(" - edx : %x\n", process->regs.edx);
    //kprint(" - ebx : %x\n", process->regs.ebx);
    //kprint(" - ebp : %x\n", process->regs.ebp);
    //kprint(" - esi : %x\n", process->regs.esi);
    //kprint(" - edi : %x\n", process->regs.edi);
    //kprint(" - ds : %x\n", process->regs.ds);
    //kprint(" - es : %x\n", process->regs.es);
    //kprint(" - fs : %x\n", process->regs.fs);
    //kprint(" - gs : %x\n", process->regs.gs);
    //kprint(" - cs : %x\n", process->regs.cs);
}

static void PrintThreadCallback(void * _thread, void * _context)
{
	UNREFERENCED_PARAMETER(_context);

	if (_thread == NULL)
	{
		KLOG(LOG_ERROR, "Invalid _thread parameter");
		return;
	}

	Thread * thread = (Thread *)_thread;

	kprint(" > Thread %d, state : %s, start exec time : %d\n",
		thread->tid,
		thread->state == THREAD_STATE_RUNNING ? "ALIVE" : (thread->state == THREAD_STATE_PAUSE ? "PAUSE" : (thread->state == THREAD_STATE_INIT ? "INIT" : "DEAD")),
		thread->startExecutionTime
	);
}

void PmPrintThreadsList(Process * process)
{
	if (process == NULL)
	{
		KLOG(LOG_ERROR, "Invalid process parameter");
		return;
	}

	List * threadList = process->threads;

	if (threadList == NULL)
	{
		KLOG(LOG_ERROR, "threadList is NULL");
		return;
	}

	ListEnumerate(threadList, PrintThreadCallback, NULL);
}

static void PrintProcessCallback(void * _process, void * _context)
{
    UNREFERENCED_PARAMETER(_context);

	if (_process == NULL)
	{
		KLOG(LOG_ERROR, "Invalid _process parameter");
		return;
	}

    Process * process = (Process *)_process;

    kprint("Process %d, state : %s, start exec time : %d\n",
        process->pid,
        process->state == PROCESS_STATE_RUNNING ? "ALIVE" : (process->state == PROCESS_STATE_PAUSE ? "PAUSE" : (process->state == PROCESS_STATE_INIT ? "INIT" : "DEAD")),
        process->startExcecutionTime
    );

	PmPrintThreadsList(_process);
}

void PmPrintProcessList()
{
    List * processList = gProcessList;

	if (processList == NULL)
	{
		KLOG(LOG_ERROR, "processList is NULL");
		return;
	}

    ListEnumerate(processList, PrintProcessCallback, NULL);
}