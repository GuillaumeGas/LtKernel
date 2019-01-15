#pragma once

#include <kernel/lib/status.h>

#include "process.h"

#define THREAD_CONSOLE_BUFFER_SIZE 512

enum ThreadState
{
    THREAD_STATE_ALIVE,
    THREAD_STATE_PAUSE,
    THREAD_STATE_DEAD
} typedef ThreadState;

struct Thread
{
    int tid;
    unsigned int startExecutionTime;
    ThreadState state;
    Process * process;

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
Thread * GetCurrentThreadFromTid(int tid);
KeStatus CreateThread(PageDirectory * pageDirectory, u32 entryAddr, Process * process);