#pragma once

#include <kernel/init/vmm.h>

#include <kernel/lib/list.h>

enum ProcessState
{
    PROCESS_STATE_ALIVE,
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

    List * children;
    Process * parent;

} typedef Process;

Process * GetCurrentProcess();
Process * GetProcessFromPid(int pid);