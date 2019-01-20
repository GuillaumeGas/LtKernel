#pragma once

#include <kernel/lib/types.h>
#include <kernel/init/isr.h>
#include <kernel/lib/handle.h>

struct DirEntry
{
	Handle handle;
	int isDirectory;
	char name[256];
} typedef DirEntry;

void SyscallHandler(int syscallNumber, InterruptContext * context);