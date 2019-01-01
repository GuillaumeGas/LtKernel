#define __SCHEDULER__
#include "scheduler.h"

#include <kernel/user/process_manager.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/init/gdt.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("SCHEDULER", LOG_LEVEL, format, ##__VA_ARGS__)

static int GetNextProcessPid()
{
    int nextPid = (gCurrentProcess->pid + 1) % gNbProcess;

    do
    {
        Process * p = ListGet(gProcessList, nextPid);
        if (p->state == PROCESS_STATE_ALIVE)
            return nextPid;
        nextPid = (nextPid + 1) % gNbProcess;
		if (gCurrentProcess->pid == nextPid)
		{
			return nextPid;
		}
    } while (1);
}

void Schedules()
{
	if (gCurrentProcess == NULL && ListTop(gProcessList) != NULL)
	{
		gCurrentProcess = (Process *)ListTop(gProcessList);
		//KLOG(LOG_DEBUG, "Switching to process %d", gCurrentProcess->pid);
		PmStartProcess(gCurrentProcess->pid);
	}
	else if (gCurrentProcess != NULL && (gNbProcess > 1 || gCurrentProcess->state != PROCESS_STATE_ALIVE))
	{
		u32 * stack_ptr = NULL;

		// Tous les registres ont été push sur la pile, par le proc via l'interruption et par les push et le pushad dans int.asm
		// On récupère le pointeur de pile stocké dans ebp afin de récupérer les registres qui nous intéressent.
		asm("mov (%%ebp), %%eax; mov %%eax, %0" : "=m" (stack_ptr) : );
		
		gCurrentProcess->regs.eflags = stack_ptr[16];
		gCurrentProcess->regs.cs = stack_ptr[15];
		gCurrentProcess->regs.eip = stack_ptr[14];
		gCurrentProcess->regs.eax = stack_ptr[13];
		gCurrentProcess->regs.ecx = stack_ptr[12];
		gCurrentProcess->regs.edx = stack_ptr[11];
		gCurrentProcess->regs.ebx = stack_ptr[10];
		gCurrentProcess->regs.ebp = stack_ptr[8];
		gCurrentProcess->regs.esi = stack_ptr[7];
		gCurrentProcess->regs.edi = stack_ptr[6];
		gCurrentProcess->regs.ds = stack_ptr[5];
		gCurrentProcess->regs.es = stack_ptr[4];
		gCurrentProcess->regs.fs = stack_ptr[3];
		gCurrentProcess->regs.gs = stack_ptr[2];

		if (gCurrentProcess->regs.cs != K_CODE_SEG_SELECTOR)
		{
			gCurrentProcess->regs.ss = stack_ptr[18];
			gCurrentProcess->regs.esp = stack_ptr[17];
		}
		else
		{
			gCurrentProcess->regs.ss = gTss.ss0;
			gCurrentProcess->regs.esp = (u32)(&stack_ptr[17]);
		}

		gCurrentProcess->kstack.esp0 = gTss.esp0;
		gCurrentProcess->kstack.ss0 = gTss.ss0;

		PmStartProcess(GetNextProcessPid());
	}
}