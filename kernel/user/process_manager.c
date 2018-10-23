#include <kernel/kernel.h>
#include <kernel/init/vmm.h>
#include <kernel/init/gdt.h>
#include <kernel/scheduler.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/kmalloc.h>

#define __PROCESS_MANAGER__
#include "process_manager.h"

void init_process_manager()
{
	g_process_list = ListCreate();
}

/*
	Permet de cr�er une t�che utilisateur :
	 - r�serve une page pour le r�pertoire de pages de la t�che
	 -         une page pour la table de pages de la t�che
	 - on utilise la table de pages d�j� existant du noyau afin que ce dernier ait acc�s
	   � toute la m�moire en cas d'interruption/appel syst�me
	 - r�serve une page pour y stocker le code de la t�che
	 - initialise les r�pertoire et table de pages
*/
void create_process(void * task_addr, unsigned int size)
{
	PageDirectory user_pd = { 0 };
	Process * new_process = NULL;
	u8 * v_user_code_ptr = NULL;
	Page kernel_stack_page = { 0 };
	u8 * p_user_stack = NULL;
    u8 * v_user_stack = NULL;
	unsigned int count = 0;

	user_pd = CreateProcessPageDirectory();
	v_user_code_ptr = (u8 *)USER_TASK_V_ADDR;

	// On utilise maintenant le r�pertoire de pages de la t�che utilisateur pour le mettre correctement � jour
	_setCurrentPagesDirectory(user_pd.pd_entry);

	// Tant qu'on a du code � copier en m�moire...
	while (count < size)
	{
		// On r�cup�re une page physique libre dans laquelle on va y copier le code
		u8 * p_new_code_page = (u8 *)GetFreePage();

		//kprint("code page : %x\n", p_new_code_page);

		// On ajoute la page physique dans l'espace d'adressage de la t�che utilisateur
		AddPageToPageDirectory(v_user_code_ptr, p_new_code_page, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, user_pd);

		mmset((u8*)v_user_code_ptr, 0, PAGE_SIZE);

		// Si on a de quoi copier sur une page enti�re, on fait �a sinon on copie seulement le reste de code � copier
		if ((size - count) < PAGE_SIZE)
			mmcopy((u8 *)task_addr + count, v_user_code_ptr, size - count);
		else
			mmcopy((u8 *)task_addr + count, v_user_code_ptr, PAGE_SIZE);

		v_user_code_ptr = (u8 *)((unsigned int)v_user_code_ptr + PAGE_SIZE);
		count += PAGE_SIZE;
	}

	kernel_stack_page = PageAlloc();

	p_user_stack = (u8 *)GetFreePage();
    v_user_stack = (u8 *)(USER_STACK_V_ADDR - PAGE_SIZE);
	AddPageToPageDirectory(v_user_stack, p_user_stack, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, user_pd);

    // Pour l'exemple : on r�serve une page expr�s pour les donn�es de la t�che
    u8 * p_user_data = GetFreePage();
    u8 * v_user_data = (u8 *)0x50000000;
    AddPageToPageDirectory(v_user_data, p_user_data, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, user_pd);

	new_process = (struct process *)kmalloc(sizeof(struct process));
	new_process->pid = g_nb_process;
	new_process->page_directory = user_pd;
    
    new_process->regs.ss = USER_STACK_SEG_SELECTOR;
    new_process->regs.cs = USER_CODE_SEG_SELECTOR;
    new_process->regs.ds = USER_DATA_SEG_SELECTOR;
    new_process->regs.es = USER_DATA_SEG_SELECTOR;
    new_process->regs.fs = USER_DATA_SEG_SELECTOR;
    new_process->regs.gs = USER_DATA_SEG_SELECTOR;
    
    new_process->regs.eax = 0;
    new_process->regs.ebp = 0;
    new_process->regs.ecx = 0;
    new_process->regs.edx = 0;
    new_process->regs.edi = 0;
    new_process->regs.esi = 0;

	new_process->regs.esp = USER_STACK_V_ADDR;
	new_process->regs.eip = USER_TASK_V_ADDR;

	new_process->regs.eflags = 0x200 & 0xFFFFBFFF;
	new_process->kstack.esp0 = (u32)kernel_stack_page.v_addr + PAGE_SIZE;
	new_process->kstack.ss0 = 0x10;

	ListPush(g_process_list, (void*)new_process);

	g_nb_process++;

	// On revient sur le r�pertoire de pages initial du noyau
	_setCurrentPagesDirectory(g_kernelInfo.pageDirectory_p.pd_entry);
}

static int testValue2 = 0;
void start_process(int pid)
{
	if (pid == -1)
	{
		kprint("[Process Manager] : start_process() failed, pid = -1\n");
		return;
	}
	else
	{
		g_current_process = ListGet(g_process_list, pid);
		g_current_process->start_execution_time = g_clock;

		//if (testValue2++ < 2)
		//{
		//	kprint("Starting task %d with pd : %x\n", g_current_process->pid, g_current_process->page_directory.pd_entry);

		//	// si on passe sur le rep de la t�che on a une entr�e bizarre du r�pertoire de page
		//	_setCurrentPagesDirectory(g_current_process->page_directory.pd_entry);

		//	u32 * pde = (u32 *)(0xFFFFF000 | PD_OFFSET(0x40000000));
		//	kprint("*pde : %x (%b*)\n", *pde, *pde, 32);

		//	_setCurrentPagesDirectory(g_kernelInfo.pageDirectory_p.pd_entry);
		//}
		//else
		//	pause();

		/*if (testValue2++ < 10)
			kprint("switching to %d\n", g_current_process->pid);*/

		g_tss.esp0 = g_current_process->kstack.esp0;
        g_tss.ss0 = g_current_process->kstack.ss0;

		_start_process(
			g_current_process->page_directory.pd_entry, 
			g_current_process->regs.ss, 
			g_current_process->regs.esp, 
			g_current_process->regs.eflags,
			g_current_process->regs.cs, 
			g_current_process->regs.eip,
			g_current_process->regs.eax,
			g_current_process->regs.ecx,
			g_current_process->regs.edx,
			g_current_process->regs.ebx,
			g_current_process->regs.ebp,
			g_current_process->regs.esi,
			g_current_process->regs.edi,
			g_current_process->regs.ds,
			g_current_process->regs.es,
			g_current_process->regs.fs,
			g_current_process->regs.gs,
			g_current_process->regs.cs == K_CODE_SEG_SELECTOR ? KERNEL : USER
		);
	}
}

static void _cleanProcess(void * param)
{
	if (param == NULL)
		return;

	Process * process = (Process *)param;
    ListDestroy(process->page_directory.page_table_list);
	kfree(process);
}

void ProcessManagerCleanCallback()
{
	ListDestroyEx(g_process_list, _cleanProcess);
}

void DumpProcess(Process * process)
{
    kprint("== Process %d ==\n", process->pid);
    kprint(" - kstack_esp0 : %x\n", process->kstack.esp0);
    kprint(" - page_directory : %x\n", process->page_directory.pd_entry);
    kprint(" - start_execution_time : %x\n", process->start_execution_time);
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