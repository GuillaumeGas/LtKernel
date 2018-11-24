#pragma once

#include <kernel/lib/types.h>

struct KeDebugContext
{
	u32 cr3, cr2, cr0;
	u32 gs, fs, es, ds, ss;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 eip;
	u16 cs;
	u32 eflags;
} typedef KeDebugContext;

void DbgInit();