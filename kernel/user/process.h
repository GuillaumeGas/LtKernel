#pragma once

#include <kernel/init/vmm.h>
#include <kernel/lib/status.h>
#include <kernel/lib/list.h>
#include <kernel/lib/handle.h>
#include <kernel/fs/file.h>

#include "thread.h"

enum ProcessState
{
	PROCESS_STATE_INIT,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_PAUSE,
    PROCESS_STATE_DEAD
} typedef ProcessState;

typedef struct Process Process;
typedef struct Thread Thread;

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

	File * currentDirectory;
	List * fileHandleList;
	List * dirHandleList;

} typedef Process;

Process * GetCurrentProcess();
Process * GetProcessFromPid(int pid);

KeStatus CreateProcess(PageDirectory pageDirectory, u32 entryAddr, Process * parent, File * location, Process ** newProcess);

void SwitchToMemoryMappingOfProcess(Process * process);