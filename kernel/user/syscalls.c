#include "syscalls.h"

#include <kernel/lib/stdio.h>
#include <kernel/drivers/clock.h>

#include <kernel/debug/debug.h>
#include <kernel/lib/stdlib.h>

static void SysPrint(const char * str);

enum SyscallId
{
    SYSCALL_PRINT = 1
};

void SyscallHandler(int syscallNumber, InterruptContext * context)
{
    switch (syscallNumber)
    {
        case SYSCALL_PRINT:
            SysPrint((char *)context->ebx);
            break;
        default:
            kprint("SyscallHandler() : unknown system call !\n");
    }
}

static void SysPrint(const char * str)
{
    kprint(str);
}