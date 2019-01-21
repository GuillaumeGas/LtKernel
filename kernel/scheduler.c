#define __SCHEDULER__
#include "scheduler.h"

#include <kernel/user/process_manager.h>
#include <kernel/user/process.h>
#include <kernel/user/thread.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/init/gdt.h>
#include <kernel/debug/debug.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("SCHEDULER", LOG_LEVEL, format, ##__VA_ARGS__)

static int GetNextThreadTid()
{
    int nextTid = (gCurrentThread->tid + 1) % gNbThreads;
	return nextTid;
    /*do
    {
        Thread * t = ListGet(gThreadsList, nextTid);
        if (t->state == THREAD_STATE_RUNNING)
            return nextTid;
        nextTid = (nextTid + 1) % gNbThreads;
		if (gCurrentThread->tid == nextTid)
		{
			return nextTid;
		}
    } while (1);*/
}

void Schedules()
{
	if (gCurrentThread == NULL && ListTop(gThreadsList) != NULL)
	{
		gCurrentThread = (Thread *)ListTop(gThreadsList);
		
		PmStartThread(gCurrentThread->tid);
	}
	else if (gCurrentThread != NULL && (gNbThreads > 1 || gCurrentThread->state != THREAD_STATE_RUNNING))
	{
		u32 * stack_ptr = NULL;

		// Tous les registres ont été push sur la pile, par le proc via l'interruption et par les push et le pushad dans int.asm
		// On récupère le pointeur de pile stocké dans ebp afin de récupérer les registres qui nous intéressent.
		asm("mov (%%ebp), %%eax; mov %%eax, %0" : "=m" (stack_ptr) : );
		
		gCurrentThread->regs.eflags = stack_ptr[16];
		gCurrentThread->regs.cs = stack_ptr[15];
		gCurrentThread->regs.eip = stack_ptr[14];
		gCurrentThread->regs.eax = stack_ptr[13];
		gCurrentThread->regs.ecx = stack_ptr[12];
		gCurrentThread->regs.edx = stack_ptr[11];
		gCurrentThread->regs.ebx = stack_ptr[10];
		gCurrentThread->regs.ebp = stack_ptr[8];
		gCurrentThread->regs.esi = stack_ptr[7];
		gCurrentThread->regs.edi = stack_ptr[6];
		gCurrentThread->regs.ds = stack_ptr[5];
		gCurrentThread->regs.es = stack_ptr[4];
		gCurrentThread->regs.fs = stack_ptr[3];
		gCurrentThread->regs.gs = stack_ptr[2];

		if (gCurrentThread->regs.cs != K_CODE_SEG_SELECTOR)
		{
			gCurrentThread->regs.ss = stack_ptr[18];
			gCurrentThread->regs.esp = stack_ptr[17];

            gCurrentThread->kstack.esp0 = gTss.esp0;
		}
		else
		{
			gCurrentThread->regs.ss = gTss.ss0;
			gCurrentThread->regs.esp = (u32)(&stack_ptr[17]);

            gCurrentThread->kstack.esp0 = gCurrentThread->regs.esp;
		}
		
		gCurrentThread->kstack.ss0 = gTss.ss0;

		PmStartThread(GetNextThreadTid());
	}
}