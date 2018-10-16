#include "debug.h"

#include <kernel/drivers/screen.h>
#include <kernel/init/gdt.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

void kdump()
{
	sc_clear();
	sc_setBackground(BLUE);

	sc_setColorEx(BLUE, RED, 0, 1);
	kprint(">> Debug breakpoint\n\n");

	sc_setColorEx(BLUE, WHITE, 0, 1);

	print_gdt();
	kprint("\n");
	print_tss();

	pause();
}

void panic(const char * str)
{
	sc_clear();
	sc_setBackground(BLUE);

	sc_setColorEx(BLUE, RED, 0, 1);
	kprint(">> %s\n\n", str);

	sc_setColorEx(BLUE, WHITE, 0, 1);

	pause ();
}

void PrintExceptionContext(ExceptionContext * context)
{
	kprint("eax = %x, ebx = %x, ecx = %x, edx = %x\n", context->eax, context->ebx, context->ecx, context->edx);
	kprint("esp = %x, ebp = %x, esi = %x, edi = %x\n", context->esp, context->ebp, context->esi, context->edi);
	kprint("eip = %x", context->eip);
	kprint("gs = %x, fs = %x, es = %x, ds = %x\n", context->gs, context->fs, context->es, context->ds);
	kprint("eflags = %x (%b)\n", context->eflags, context->eflags);
	kprint("cr0 = %x (%b)\n", context->cr0, context->cr0);
	kprint("cr2 = %x (%b)\n", context->cr2, context->cr2);
	kprint("cr3 = %x (%b)\n", context->cr3, context->cr3);
}

void PrintExceptionContextWithCode(ExceptionContextWithCode * context)
{
	kprint("eax = %x, ebx = %x, ecx = %x, edx = %x\n", context->eax, context->ebx, context->ecx, context->edx);
	kprint("esp = %x, ebp = %x, esi = %x, edi = %x\n", context->esp, context->ebp, context->esi, context->edi);
	kprint("eip = %x\n", context->eip);
	kprint("gs = %x, fs = %x, es = %x, ds = %x\n", context->gs, context->fs, context->es, context->ds);
	kprint("eflags = %x (%b)\n", context->eflags, context->eflags);
	kprint("cr0 = %x (%b)\n", context->cr0, context->cr0);
	kprint("cr2 = %x (%b)\n", context->cr2, context->cr2);
	kprint("cr3 = %x (%b)\n", context->cr3, context->cr3);
	kprint("Error code : %x (%b)\n", context->code, context->code);
}
