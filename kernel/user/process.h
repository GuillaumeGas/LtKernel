#pragma once

#include <kernel/init/vmm.h>
#include <kernel/lib/status.h>
#include <kernel/lib/list.h>

#include "thread.h"

enum ProcessState
{
	PROCESS_STATE_INIT,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_PAUSE,
    PROCESS_STATE_DEAD
} typedef ProcessState;

struct Process;
typedef struct Process Process;

struct Process
{
    int pid;
    unsigned int startExcecutionTime;
    PageDirectory pageDirectory;
    ProcessState state;
	Thread * mainThread;

    List * children;
    Process * parent;
	List * threads;

} typedef Process;

Process * GetCurrentProcess();
Process * GetProcessFromPid(int pid);

KeStatus CreateProcess(PageDirectory pageDirectory, u32 entryAddr, Process * parent, Process ** newProcess);

void SwitchToMemoryMappingOfProcess(Process * process);