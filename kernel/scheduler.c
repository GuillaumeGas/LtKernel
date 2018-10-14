#define __SCHEDULER__
#include "scheduler.h"

#include <kernel/user/process_manager.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/init/gdt.h>

void schedule()
{
	//if (g_current_process == NULL && list_top(g_process_list) != NULL)
	//{
	//	g_current_process = (Process *)list_top(g_process_list);
	//	start_process(g_current_process->pid);
	//}
	//else if (g_current_process != NULL)
	//{
	//	u32 * stack_ptr = NULL;

	//	// Tous les registres ont été push sur la pile, par le proc via l'interruption et par les push et le pushad dans int.asm
	//	// On récupère le pointeur de pile stocké dans ebp afin de récupérer les registres qui nous intéressent.
	//	asm("mov (%%ebp), %%eax; mov %%eax, %0" : "=m" (stack_ptr) : );

	//	
	//	g_current_process->regs.eflags = stack_ptr[16];
	//	g_current_process->regs.cs = stack_ptr[15];
	//	g_current_process->regs.eip = stack_ptr[14];
	//	g_current_process->regs.eax = stack_ptr[13];
	//	g_current_process->regs.ecx = stack_ptr[12];
	//	g_current_process->regs.edx = stack_ptr[11];
	//	g_current_process->regs.ebx = stack_ptr[10];
	//	g_current_process->regs.ebp = stack_ptr[8];
	//	g_current_process->regs.esi = stack_ptr[7];
	//	g_current_process->regs.edi = stack_ptr[6];
	//	g_current_process->regs.ds = stack_ptr[5];
	//	g_current_process->regs.es = stack_ptr[4];
	//	g_current_process->regs.fs = stack_ptr[3];
	//	g_current_process->regs.gs = stack_ptr[2];

	//	if (g_current_process->regs.cs != K_CODE_SEG_SELECTOR)
	//	{
	//		g_current_process->regs.ss = stack_ptr[18];
	//		g_current_process->regs.esp = stack_ptr[17];
	//	}
	//	else
	//	{
	//		g_current_process->regs.ss = g_tss.ss0;
	//		g_current_process->regs.esp = (u32)(&stack_ptr[17]);
	//	}

	//	g_current_process->kstack_esp0 = (u32 *)(g_tss.esp0);

	//	start_process((g_current_process->pid + 1) % g_nb_process);
	//}
}