#pragma once

#include <kernel/lib/types.h>

struct ExceptionContext
{
	u32 cr3, cr2, cr0;
	u32 gs, fs, es, ds;
	u32 eflags;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 eip;
} typedef ExceptionContext;

struct ExceptionContextWithCode
{
	u32 cr3, cr2, cr0;
	u32 gs, fs, es, ds;
	u32 eflags;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 code;
	u32 eip;
} typedef ExceptionContextWithCode;

void kdump ();
void panic (const char * str);
void PrintExceptionContext(ExceptionContext * context);
void PrintExceptionContextWithCode(ExceptionContextWithCode * context);
