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
	Permet de créer une tâche utilisateur :
	 - réserve une page pour le répertoire de pages de la tâche
	 -         une page pour la table de pages de la tâche
	 - on utilise la table de pages déjà existant du noyau afin que ce dernier ait accès
	   à toute la mémoire en cas d'interruption/appel système
	 - réserve une page pour y stocker le code de la tâche
	 - initialise les répertoire et table de pages
*/
void create_process(void * task_addr, unsigned int size)
{
	PageDirectory user_pd = { 0 };
	Process * new_process = NULL;
	u8 * v_user_code_ptr = NULL;
	Page kernel_stack_page = { 0 };
	u8 * p_user_stack = NULL;
	unsigned int count = 0;

	user_pd = create_process_pd();
	v_user_code_ptr = (u8 *)USER_TASK_V_ADDR;

	// Tant qu'on a du code à copier en mémoire...
	while (count < size)
	{
		// On récupère une page physique libre dans laquelle on va y copier le code
		u8 * p_new_code_page = (u8 *)get_free_page();

		// On ajoute la page dans l'espace d'adressage du noyau
		pd0_add_page(v_user_code_ptr, p_new_code_page, IN_MEMORY | WRITEABLE);
		// On ajoute la page physique dans l'espace d'adressage de la tâche utilisateur
		pd_add_page(v_user_code_ptr, p_new_code_page, IN_MEMORY | WRITEABLE, user_pd);

		mmset((u8*)v_user_code_ptr, 0, PAGE_SIZE);

		// Si on a de quoi copier sur une page entière, on fait ça sinon on copie seulement le reste de code à copier
		if ((size - count) < PAGE_SIZE)
			mmcopy((u8 *)task_addr + count, v_user_code_ptr, size - count);
		else
			mmcopy((u8 *)task_addr + count, v_user_code_ptr, PAGE_SIZE);

		v_user_code_ptr += PAGE_SIZE;
		count += PAGE_SIZE;
	}

	kernel_stack_page = PageAlloc();

	p_user_stack = (u8 *)get_free_page();
	pd_add_page((u8 *)USER_STACK_V_ADDR, p_user_stack, IN_MEMORY | WRITEABLE, user_pd);

	new_process = (struct process *)kmalloc(sizeof(struct process));
	new_process->pid = g_nb_process;
	new_process->page_directory = user_pd;
	new_process->regs.ss = USER_STACK_SEG_SELECTOR;
	new_process->regs.esp = USER_STACK_V_ADDR;
	new_process->regs.cs = USER_CODE_SEG_SELECTOR;
	new_process->regs.eip = USER_TASK_V_ADDR;
	new_process->regs.eflags = 0x200 & 0xFFFFBFFF;
	new_process->kstack_esp0 = kernel_stack_page.v_addr + PAGE_SIZE;

	ListPush(g_process_list, (void*)new_process);

	g_nb_process++;
}

void start_process(int pid)
{
	/*if (pid == -1)
	{
		kprint("[Process Manager] : start_process() failed, pid = -1\n");
		return;
	}
	else
	{
		g_current_process = list_get(g_process_list, pid);
		g_current_process->start_execution_time = g_clock;

		g_tss.esp0 = (u32)g_current_process->kstack_esp0;

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
	}*/
}

static void _cleanProcess(void * param)
{
	if (param == NULL)
		return;

	Process * process = (Process *)param;
	// todo clean process content

	kfree(process);
}

void ProcessManagerCleanCallback()
{
	//ListDestroyEx(g_process_list, _cleanProcess);
}