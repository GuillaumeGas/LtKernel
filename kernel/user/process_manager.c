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
	gProcessList = ListCreate();
	if (gProcessList == NULL)
	{
		KLOG(LOG_ERROR, "ListCreate() failed");
	}

	gThreadsList = ListCreate();
	if (gThreadsList == NULL)
	{
		KLOG(LOG_ERROR, "ListCreate() failed");
	}
}

KeStatus PmCreateProcess(u32 entryAddr, Process ** newProcess, Process * parent, File * location)
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

	status = CreateProcess(processPd, entryAddr, parent, location, &process);
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

// TODO : revoir la création de threads noyau
void PmCreateKernelThread()
{
	Thread * thread = (Thread *)kmalloc(sizeof(Thread));
	if (thread == NULL)
	{
		// TODO : va aussi falloir crash le systeme ici
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Thread));
		return;
	}

	thread->tid = gThreadId++;
	thread->startExecutionTime = 0;
	thread->state = THREAD_STATE_RUNNING;
	thread->process = NULL;
	thread->privilegeLevel = KERNEL;

	gNbThreads++;

	gCurrentThread = thread;
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

		PageDirectoryEntry * pd = NULL;

		if (gCurrentThread->privilegeLevel == USER)
		{
			gCurrentProcess = gCurrentThread->process;
			gCurrentProcess->state = PROCESS_STATE_RUNNING;
			pd = gCurrentProcess->pageDirectory.pdEntry;

			ThreadPrepare(gCurrentThread);
		}
		else
		{
			pd = gKernelInfo.pPageDirectory.pdEntry;
		}

		gTss.esp0 = gCurrentThread->kstack.esp0;
        gTss.ss0 = gCurrentThread->kstack.ss0;


		// Si on passe sur un thread noyau, on utilise le pd du process qui s'exécutait,
		// au pire ce sera le process system quand il sera créé
		_start_process(
			pd,
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

static void PrintThreadCallback(void * _thread, void * _context)
{
	UNREFERENCED_PARAMETER(_context);

	if (_thread == NULL)
	{
		KLOG(LOG_ERROR, "Invalid _thread parameter");
		return;
	}

	Thread * thread = (Thread *)_thread;

	kprint(" - Thread %d, state : %s, start exec time : %d\n",
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

    kprint("\nProcess %d, state : %s, start exec time : %d\n",
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