#include <kernel/init/memory.h>
#include <kernel/init/gdt.h>
#include <kernel/scheduler.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/kmalloc.h>

#define __PROCESS_MANAGER__
#include "process_manager.h"

void init_process_manager()
{
	list_create(&g_process_list);
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
		struct page_directory_entry * user_pd = NULL;
		struct process * new_process = NULL;
		u8 * v_user_code_ptr = NULL;
		u32 * kernel_stack = NULL;
		u32 * user_stack = NULL;
		unsigned int count = 0;
		unsigned int index = 0;

		user_pd = create_process_pd();

		v_user_code_ptr = (u8 *)USER_TASK_V_ADDR;

		while (count < size)
		{
			u8 * p_new_code_page = (u8 *)get_free_page();

			if ((size - count) < PAGE_SIZE)
				mmcopy(task_addr + count, p_new_code_page, size - count);
			else
				mmcopy(task_addr + count, p_new_code_page, PAGE_SIZE);

			add_page(user_pd, p_new_code_page, v_user_code_ptr);
			v_user_code_ptr += PAGE_SIZE;
			count += PAGE_SIZE;
		}

		kernel_stack = (u32*)page_alloc();
		
		user_stack = (u32*)get_free_page();
		add_page(user_pd, user_stack, USER_STACK_V_ADDR);

		new_process = (struct process *)kmalloc(sizeof(struct process));
		new_process->pid = g_nb_process;
		new_process->pd = user_pd;
		new_process->regs.ss = USER_STACK_SEG_SELECTOR;
		new_process->regs.esp = USER_STACK_V_ADDR;
		new_process->regs.cs = USER_CODE_SEG_SELECTOR;
		new_process->regs.eip = USER_TASK_V_ADDR;
		new_process->regs.eflags = 0x200 & 0xFFFFBFFF;
		new_process->kstack_esp0 = kernel_stack + PAGE_SIZE;

		list_push(&g_process_list, (void*)new_process);

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
		g_current_process = list_get(&g_process_list, pid);
		g_current_process->start_execution_time = g_clock;

		g_tss.esp0 = (u32)g_current_process->kstack_esp0;

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
			g_current_process->regs.gs,
			g_current_process->regs.cs == K_CODE_SEG_SELECTOR ? KERNEL : USER
		);
	}
}
