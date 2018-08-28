#include <kernel/init/vmm.h>
#include <kernel/lib/memory.h>
#include <kernel/scheduler.h>
#include <kernel/lib/stdio.h>

#define __PROCESS_MANAGER__
#include "process_manager.h"

void init_process_manager()
{
	unsigned int index = 0;

	for (; index < NB_MAX_PROCESS; index++)
	{
		g_process_list[index].pid = -1;
		g_process_list[index].pd = NULL;
		g_process_list[index].start_execution_time = 0;
	}
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
void create_process(u8 * task_addr, unsigned int size)
{
	if (g_nb_process >= NB_MAX_PROCESS)
	{
		kprint("[Process Manager] : create_process() failed, too many processes !\n");
		return;
	}
	else
	{
		struct page_directory_entry * pd = (struct page_directory_entry*)get_free_page();
		struct page_table_entry * pt = (struct page_table_entry*)get_free_page();
		struct process * new_process = NULL;
		unsigned int index = 0;

		// récupération d'une page pour y stocker le code à exécuter
		u8 * new_task_addr = (u8*)get_free_page();

		kprint("[Process Manager] : create_process() !\n");

		// copie de la tâche dans la zone réservée à la tâche utilisateur
		mmcopy(task_addr, new_task_addr, size);

		init_pages_directory(pd);
		set_page_directory_entry(pd, (u32)kernel_pt, IN_MEMORY | WRITEABLE);

		for (; index < NB_PAGES_PER_TABLE; index++)
			set_page_table_entry(&(pt[index]), 0, EMPTY);

		set_page_directory_entry(&pd[USER_TASK_V_ADDR >> 22], (u32)pt, IN_MEMORY | WRITEABLE | NON_PRIVILEGED_ACCESS);
		set_page_table_entry(pt, (u32)new_task_addr, IN_MEMORY | WRITEABLE | NON_PRIVILEGED_ACCESS);

		new_process = &g_process_list[g_nb_process];
		new_process->pid = g_nb_process;
		new_process->pd = pd;
		new_process->regs.ss = USER_STACK_SEG_SELECTOR;
		new_process->regs.esp = USER_STACK_START_ADDR;
		new_process->regs.cs = USER_CODE_SEG_SELECTOR;
		new_process->regs.eip = USER_TASK_V_ADDR;
		new_process->regs.eflags = 0x200 & 0xFFFFBFFF;

		g_nb_process++;
	}
}

void start_process(int pid)
{
	if (pid == -1)
	{
		kprint("[Process Manager] : start_process() failed, pid = -1\n");
		return;
	}
	else
	{
		g_current_process = &g_process_list[pid];
		g_current_process->start_execution_time = g_clock;

		_start_process(
			g_current_process->pd, 
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
			g_current_process->regs.gs
		);
	}
}
