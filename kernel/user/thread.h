#pragma once

#include <kernel/lib/status.h>
#include <kernel/lib/types.h>
#include <kernel/init/vmm.h>

#include "common.h"

#define THREAD_CONSOLE_BUFFER_SIZE 512

enum ThreadState
{
	THREAD_STATE_INIT,
    THREAD_STATE_RUNNING,
    THREAD_STATE_PAUSE,
    THREAD_STATE_DEAD
} typedef ThreadState;

struct Process;
typedef struct Process Process;

struct Thread
{
    int tid;
    unsigned int startExecutionTime;
    ThreadState state;
    Process * process;
	Page stackPage;
	ExecMode privilegeLevel;

    struct
    {
        u32 esp0;
        u16 ss0;
    } kstack;

    struct
    {
        u32 eax, ebx, ecx, edx;
        u32 ebp, esp, esi, edi;
        u32 eip, eflags;
        u32 cs : 16, ss : 16, ds : 16, es : 16, fs : 16, gs : 16;
        u32 cr3;
    } regs;

    struct
    {
        char consoleBuffer[THREAD_CONSOLE_BUFFER_SIZE];
        unsigned int bufferIndex;
        BOOL readyToBeFlushed;
    } console;

} typedef Thread;

Thread * GetCurrentThread();
Thread * GetThreadFromTid(int tid);
KeStatus CreateMainThread(Process * process, u32 entryAddr, Thread ** mainThread);
void ThreadPrepare(Thread * thread);

void SwitchToMemoryMappingOfThread(Thread * thread);