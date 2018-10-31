#pragma once

#include <kernel/init/vmm.h>

#include <kernel/lib/list.h>

#define PROCESS_CONSOLE_BUFFER_SIZE 512

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
		char consoleBuffer[PROCESS_CONSOLE_BUFFER_SIZE];
		unsigned int bufferIndex;
		BOOL readyToBeFlushed;
	} console;

    List * children;
    Process * parent;

} typedef Process;

Process * GetCurrentProcess();