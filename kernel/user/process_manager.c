#include <kernel/kernel.h>
#include <kernel/scheduler.h>

#include <kernel/init/vmm.h>
#include <kernel/init/gdt.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("USER", LOG_LEVEL, format, ##__VA_ARGS__)

#define __PROCESS_MANAGER__
#include "process_manager.h"

static void ProcessInit(Process * process, int pid, PageDirectory * pageDirectory, u32 vEntryAddr, u32 esp0, Process * parent);

void PmInit()
{
	gProcessList = ListCreate();
}

/*
	Permet de créer une tâche utilisateur :
	 - réserve une page pour le répertoire de pages de la tâche
	 -         une page pour la table de pages de la tâche
	 - on utilise la table de pages déjà existant du noyau afin que ce dernier ait accès
	   à toute la mémoire en cas d'interruption/appel système
	 - réserve une page pour y stocker le code de la tâche
	 - initialise les répertoire et table de pages
*/
KeStatus PmCreateProcess(void * taskAddr, unsigned int size, Process * parent, int * pid)
{
	PageDirectory userPd = { 0 };
	Process * newProcess = NULL;
	u8 * vUserCodePtr = NULL;
	Page kernelStackPage = { 0 };
	u8 * pUserStack = NULL;
    u8 * vUserStack = NULL;
	unsigned int count = 0;
	KeStatus status = STATUS_FAILURE;

	if (taskAddr == NULL)
	{
		KLOG(LOG_ERROR, "Invalid taskAddr parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (size == 0)
	{
		KLOG(LOG_ERROR, "Invalid size parameter");
		return STATUS_INVALID_PARAMETER;
	}

	if (pid == NULL)
	{
		KLOG(LOG_ERROR, "Invalid pid parameter");
		return STATUS_NULL_PARAMETER;
	}

	userPd = CreateProcessPageDirectory();
	vUserCodePtr = (u8 *)USER_TASK_V_ADDR;

	// On utilise maintenant le répertoire de pages de la tâche utilisateur pour le mettre correctement à jour
    PageDirectoryEntry * currentKernelPd = _getCurrentPagesDirectory();
	_setCurrentPagesDirectory(userPd.pdEntry);

	// Tant qu'on a du code à copier en mémoire...
	while (count < size)
	{
		// On récupère une page physique libre dans laquelle on va y copier le code
		u8 * pNewCodePage = (u8 *)GetFreePage();

		if (pNewCodePage == NULL)
		{
			KLOG(LOG_ERROR, "Couldn't find a free page");
			status = STATUS_PHYSICAL_MEMORY_FULL;
			goto clean;
		}

		// On ajoute la page physique dans l'espace d'adressage de la tâche utilisateur
		AddPageToPageDirectory(vUserCodePtr, pNewCodePage, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, userPd);

		MmSet((u8*)vUserCodePtr, 0, PAGE_SIZE);

		// Si on a de quoi copier sur une page entière, on fait ça sinon on copie seulement le reste de code à copier
		if ((size - count) < PAGE_SIZE)
			MmCopy((u8 *)taskAddr + count, vUserCodePtr, size - count);
		else
			MmCopy((u8 *)taskAddr + count, vUserCodePtr, PAGE_SIZE);

		vUserCodePtr = (u8 *)((unsigned int)vUserCodePtr + PAGE_SIZE);
		count += PAGE_SIZE;
	}

	kernelStackPage = PageAlloc();

	pUserStack = (u8 *)GetFreePage();
	if (pUserStack == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't find a free page");
		goto clean;
	}

    vUserStack = (u8 *)(USER_STACK_V_ADDR - PAGE_SIZE);
	AddPageToPageDirectory(vUserStack, pUserStack, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, userPd);

    // Pour l'exemple : on réserve une page exprès pour les données de la tâche
    u8 * pUserData = GetFreePage();
    u8 * vUserData = (u8 *)0x50000000;
    AddPageToPageDirectory(vUserData, pUserData, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, userPd);

	newProcess = (Process *)kmalloc(sizeof(Process));
	if (newProcess == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Process));
		status = STATUS_ALLOC_FAILED;
		goto clean;
	}

	ProcessInit(newProcess, gNbProcess, &userPd, (u32)USER_TASK_V_ADDR, (u32)kernelStackPage.vAddr + PAGE_SIZE, parent);
	ListPush(gProcessList, (void*)newProcess);

    gNbProcess++;

	// On revient sur le répertoire de pages initial du noyau
	_setCurrentPagesDirectory(currentKernelPd);

	*pid = newProcess->pid;
	pUserStack = NULL;
	pUserData = NULL;

	status = STATUS_SUCCESS;

clean:
	if (pUserStack != NULL)
	{
		ReleasePage(pUserStack);
		pUserStack = NULL;
	}

	if (pUserData != NULL)
	{
		ReleasePage(pUserData);
		pUserData = NULL;
	}

	return status;
}

KeStatus PmCreateProcessFromElf(PageDirectory * pageDirectory, u32 entryAddr, int * pid, Process * parent)
{
	Process * newProcess = NULL;
	Page kernelStackPage = { 0 };
	u8 * pUserStack = NULL;
	u8 * vUserStack = NULL;
	KeStatus status = STATUS_FAILURE;

	if (pageDirectory == NULL)
	{
		KLOG(LOG_ERROR, "Invalid pageDirectory parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (pid == NULL)
	{
		KLOG(LOG_ERROR, "Invalid pid parameter");
		return STATUS_NULL_PARAMETER;
	}

	// On utilise maintenant le répertoire de pages de la tâche utilisateur pour le mettre correctement à jour
	PageDirectoryEntry * currentKernelPd = _getCurrentPagesDirectory();
	_setCurrentPagesDirectory(pageDirectory->pdEntry);

	kernelStackPage = PageAlloc();

	pUserStack = (u8 *)GetFreePage();
	if (pUserStack == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't find a free page");
		goto clean;
	}

	vUserStack = (u8 *)(USER_STACK_V_ADDR - PAGE_SIZE);
	AddPageToPageDirectory(vUserStack, pUserStack, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, *pageDirectory);

	// Pour l'exemple : on réserve une page exprès pour les données de la tâche
	u8 * pUserData = GetFreePage();
	u8 * vUserData = (u8 *)0x50000000;
	AddPageToPageDirectory(vUserData, pUserData, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, *pageDirectory);

	newProcess = (Process *)kmalloc(sizeof(Process));
	if (newProcess == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Process));
		status = STATUS_ALLOC_FAILED;
		goto clean;
	}

	ProcessInit(newProcess, gNbProcess, pageDirectory, entryAddr, (u32)kernelStackPage.vAddr + PAGE_SIZE, parent);
	ListPush(gProcessList, (void*)newProcess);

	gNbProcess++;

	*pid = newProcess->pid;
	pUserStack = NULL;

	status = STATUS_SUCCESS;

clean:
	// On revient sur le répertoire de pages initial du noyau
	_setCurrentPagesDirectory(currentKernelPd);

	if (pUserStack != NULL)
	{
		ReleasePage(pUserStack);
		pUserStack = NULL;
	}

	return status;
}

static void ProcessInit(Process * process, int pid, PageDirectory * pageDirectory, u32 vEntryAddr, u32 esp0, Process * parent)
{
	if (process == NULL)
	{
		KLOG(LOG_ERROR, "Invalid process parameter");
		return;
	}

	if (pageDirectory == NULL)
	{
		KLOG(LOG_ERROR, "Invalid pageDirectory parameter");
		return;
	}

	process->pid = pid;
	process->pageDirectory = *pageDirectory;
	process->state = PROCESS_STATE_ALIVE;

	process->regs.ss = USER_STACK_SEG_SELECTOR;
	process->regs.cs = USER_CODE_SEG_SELECTOR;
	process->regs.ds = USER_DATA_SEG_SELECTOR;
	process->regs.es = USER_DATA_SEG_SELECTOR;
	process->regs.fs = USER_DATA_SEG_SELECTOR;
	process->regs.gs = USER_DATA_SEG_SELECTOR;

	process->regs.eax = 0;
	process->regs.ebp = 0;
	process->regs.ecx = 0;
	process->regs.edx = 0;
	process->regs.edi = 0;
	process->regs.esi = 0;

	process->regs.esp = USER_STACK_V_ADDR - 4;
	process->regs.eip = vEntryAddr;

	process->regs.eflags = 0x200 & 0xFFFFBFFF;
	process->kstack.esp0 = esp0;
	process->kstack.ss0 = 0x10;

	MmSet((u8 *)process->console.consoleBuffer, 0, PROCESS_CONSOLE_BUFFER_SIZE);
	process->console.bufferIndex = 0;
	process->console.readyToBeFlushed = FALSE;

	process->children = ListCreate();
	if (process->children == NULL)
	{
		KLOG(LOG_ERROR, "ListCreate() returned NULL");
	}

	process->parent = parent;
}

void PmStartProcess(int pid)
{
	if (pid == -1)
	{
		KLOG(LOG_ERROR, "Invalid pid parameter");
		return;
	}
	else
	{
		gCurrentProcess = ListGet(gProcessList, pid);
		if (gCurrentProcess == NULL)
		{
			KLOG(LOG_ERROR, "Process %d not found", pid);
			return;
		}

		gCurrentProcess->startExcecutionTime = g_clock;
	
		gTss.esp0 = gCurrentProcess->kstack.esp0;
        gTss.ss0 = gCurrentProcess->kstack.ss0;

		_start_process(
			gCurrentProcess->pageDirectory.pdEntry,
			gCurrentProcess->regs.ss,
			gCurrentProcess->regs.esp,
			gCurrentProcess->regs.eflags,
			gCurrentProcess->regs.cs,
			gCurrentProcess->regs.eip,
			gCurrentProcess->regs.eax,
			gCurrentProcess->regs.ecx,
			gCurrentProcess->regs.edx,
			gCurrentProcess->regs.ebx,
			gCurrentProcess->regs.ebp,
			gCurrentProcess->regs.esi,
			gCurrentProcess->regs.edi,
			gCurrentProcess->regs.ds,
			gCurrentProcess->regs.es,
			gCurrentProcess->regs.fs,
			gCurrentProcess->regs.gs,
			gCurrentProcess->regs.cs == K_CODE_SEG_SELECTOR ? KERNEL : USER
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

	Process * process = (Process *)param;
    ListDestroy(process->pageDirectory.pageTableList);
	kfree(process);
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

    kprint("== Process %d ==\n", process->pid);
    kprint(" - kstack_esp0 : %x\n", process->kstack.esp0);
    kprint(" - page_directory : %x\n", process->pageDirectory.pdEntry);
    kprint(" - start_execution_time : %x\n", process->startExcecutionTime);
    kprint(" - ss : %x\n", process->regs.ss);
    kprint(" - esp : %x\n", process->regs.esp);
    kprint(" - eflags : %x\n", process->regs.eflags);
    kprint(" - cs : %x\n", process->regs.cs);
    kprint(" - eip : %x\n", process->regs.eip);
    kprint(" - eax : %x\n", process->regs.eax);
    kprint(" - ecx : %x\n", process->regs.ecx);
    kprint(" - edx : %x\n", process->regs.edx);
    kprint(" - ebx : %x\n", process->regs.ebx);
    kprint(" - ebp : %x\n", process->regs.ebp);
    kprint(" - esi : %x\n", process->regs.esi);
    kprint(" - edi : %x\n", process->regs.edi);
    kprint(" - ds : %x\n", process->regs.ds);
    kprint(" - es : %x\n", process->regs.es);
    kprint(" - fs : %x\n", process->regs.fs);
    kprint(" - gs : %x\n", process->regs.gs);
    kprint(" - cs : %x\n", process->regs.cs);
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
        process->state == PROCESS_STATE_ALIVE ? "ALIVE" : (process->state == PROCESS_STATE_PAUSE ? "PAUSE" : "DEAD"),
        process->startExcecutionTime
    );
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