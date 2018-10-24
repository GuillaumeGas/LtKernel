#pragma once

#include <kernel/lib/types.h>
#include <kernel/init/isr.h>

void SyscallHandler(int syscallNumber, InterruptContext * context);