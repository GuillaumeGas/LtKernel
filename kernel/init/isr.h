#pragma once

#include <kernel/lib/types.h>

struct InterruptContext
{
	u32 gs, fs, es, ds;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 eip;
} typedef InterruptContext;

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

struct ExceptionContextUserWithCode
{
	u32 cr3, cr2, cr0;
	u32 gs, fs, es, ds;
	u32 eflags;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 code;
	u32 eip;

	u32 csU;
	u32 eflagsU;
	u32 espU;
	u32 ssU;
} typedef ExceptionContextUserWithCode;