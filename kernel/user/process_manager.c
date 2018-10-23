#include <kernel/kernel.h>
#include <kernel/scheduler.h>

#include <kernel/init/vmm.h>
#include <kernel/init/gdt.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/kmalloc.h>

#define __PROCESS_MANAGER__
#include "process_manager.h"

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
void PmCreateProcess(void * taskAddr, unsigned int size)
{
	PageDirectory userPd = { 0 };
	Process * newProcess = NULL;
	u8 * vUserCodePtr = NULL;
	Page kernelStackPage = { 0 };
	u8 * pUserStack = NULL;
    u8 * vUserStack = NULL;
	unsigned int count = 0;

	userPd = CreateProcessPageDirectory();
	vUserCodePtr = (u8 *)USER_TASK_V_ADDR;

	// On utilise maintenant le répertoire de pages de la tâche utilisateur pour le mettre correctement à jour
	_setCurrentPagesDirectory(userPd.pdEntry);

	// Tant qu'on a du code à copier en mémoire...
	while (count < size)
	{
		// On récupère une page physique libre dans laquelle on va y copier le code
		u8 * pNewCodePage = (u8 *)GetFreePage();

		//kprint("code page : %x\n", p_new_code_page);

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
    vUserStack = (u8 *)(USER_STACK_V_ADDR - PAGE_SIZE);
	AddPageToPageDirectory(vUserStack, pUserStack, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, userPd);

    // Pour l'exemple : on réserve une page exprès pour les données de la tâche
    u8 * pUserData = GetFreePage();
    u8 * vUserData = (u8 *)0x50000000;
    AddPageToPageDirectory(vUserData, pUserData, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, userPd);

	newProcess = (Process *)kmalloc(sizeof(Process));
	newProcess->pid = gNbProcess;
	newProcess->pageDirectory = userPd;
    
    newProcess->regs.ss = USER_STACK_SEG_SELECTOR;
    newProcess->regs.cs = USER_CODE_SEG_SELECTOR;
    newProcess->regs.ds = USER_DATA_SEG_SELECTOR;
    newProcess->regs.es = USER_DATA_SEG_SELECTOR;
    newProcess->regs.fs = USER_DATA_SEG_SELECTOR;
    newProcess->regs.gs = USER_DATA_SEG_SELECTOR;
    
    newProcess->regs.eax = 0;
    newProcess->regs.ebp = 0;
    newProcess->regs.ecx = 0;
    newProcess->regs.edx = 0;
    newProcess->regs.edi = 0;
    newProcess->regs.esi = 0;

	newProcess->regs.esp = USER_STACK_V_ADDR;
	newProcess->regs.eip = USER_TASK_V_ADDR;

	newProcess->regs.eflags = 0x200 & 0xFFFFBFFF;
	newProcess->kstack.esp0 = (u32)kernelStackPage.vAddr + PAGE_SIZE;
	newProcess->kstack.ss0 = 0x10;

	ListPush(gProcessList, (void*)newProcess);

	gNbProcess++;

	// On revient sur le répertoire de pages initial du noyau
	_setCurrentPagesDirectory(gKernelInfo.pPageDirectory.pdEntry);
}

void PmStartProcess(int pid)
{
	if (pid == -1)
	{
		kprint("[Process Manager] : start_process() failed, pid = -1\n");
		return;
	}
	else
	{
		gCurrentProcess = ListGet(gProcessList, pid);
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
		return;

	Process * process = (Process *)param;
    ListDestroy(process->pageDirectory.pageTableList);
	kfree(process);
}

void PmCleanCallback()
{
	ListDestroyEx(gProcessList, CleanProcessCallback);
}

void DumpProcess(Process * process)
{
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