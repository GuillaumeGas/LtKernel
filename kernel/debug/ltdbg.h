#pragma once

#include <kernel/lib/types.h>

#define DEFAULT_ASM_BUFFER_SIZE 20

enum BpState 
{
	BP_ENABLED,
	BP_DISABLED
} typedef BpState;

struct KeDebugContext
{
	u32 cr3, cr2, cr0;
	u32 gs, fs, es, ds, ss;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 eip;
	u16 cs;
	u32 eflags;
} typedef KeDebugContext;

struct KeBreakpoint
{
	u32 addr;
	BpState state;
	u8 savedInstByte;
	int id;
} typedef KeBreakpoint;

void DbgInit();