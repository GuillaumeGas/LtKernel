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
		if (g_nb_process > 1 && g_current_process->start_execution_time > 100)
			switch_process((g_current_process->pid + 1) % g_nb_process);
	}

	g_clock++;
}