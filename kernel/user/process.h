#pragma once

#include <kernel/init/vmm.h>

struct Process
{
    int pid;
    unsigned int startExcecutionTime;
    PageDirectory pageDirectory;

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
} typedef Process;

Process * GetCurrentProcess();