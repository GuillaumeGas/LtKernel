#include "syscalls.h"

#include <kernel/lib/stdio.h>
#include <kernel/drivers/clock.h>
#include <kernel/init/isr.h>

static void SysPrint(const char * str);
static void SysTime(u32 * retPtr);

enum SyscallId
{
    SYSCALL_PRINT = 1,
    SYSCALL_TIME  = 2
} typedef;

void SyscallHandler(int syscallNumber, Context context)
{
    switch (syscallNumber)
    {
        case SYSCALL_PRINT:
            SysPrint((char *)context->eax);
            break;
        case SYSCALL_TIME:
            SysTime(&context->eax);
            break;
        default:
            kprint("SyscallHandler() : unknown system call !\n");
    }
}

static void SysPrint(const char * str)
{
    kprint(str);
}

static void SysTime(u32 * retPtr)
{
    *retPtr = gClockSec;
}