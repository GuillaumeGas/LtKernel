#define __SCHEDULER__
#include "scheduler.h"

#include <kernel/user/process_manager.h>
#include <kernel/lib/stdlib.h>

void schedule()
{
	if (g_current_process == NULL && g_process_list[0].pid != -1)
	{
		start_process(g_process_list[0].pid);
	}
	else
	{
		//if (g_clock % 100 == 0)
		//{
		//	u32 * stack_ptr = NULL;

		//	// Tous les registres ont �t� push sur la pile, soit par le proc via l'interruption, soit par les push et le pushad dans int.asm
		//	asm("mov (%%ebp), %%eax; mov %%eax, %0" : "=m" (stack_ptr) :);

		//	g_current_process->regs.ss = stack_ptr[18];
		//	g_current_process->regs.esp = stack_ptr[17];
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

		//	kprint("[Scheduler] switching to process %d\n", g_process_list[(g_current_process->pid + 1) % g_nb_process]);

		//	start_process((g_current_process->pid + 1) % g_nb_process);
		//}
	}

	g_clock++;
}