#include "debug.h"

#include <kernel/drivers/screen.h>
#include <kernel/init/gdt.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

void kdump()
{
	ScClear();
	ScSetBackground(BLUE);

	ScSetColorEx(BLUE, RED, 0, 1);
	kprint(">> Debug breakpoint\n\n");

	ScSetColorEx(BLUE, WHITE, 0, 1);

	GdtPrint();
	kprint("\n");
	TssPrint();

	Pause();
}

void panicStr(const char * str)
{
	ScClear();
	ScSetBackground(BLUE);

	ScSetColorEx(BLUE, RED, 0, 1);
	kprint(">> %s\n\n", str);

	ScSetColorEx(BLUE, WHITE, 0, 1);

	Pause ();
}

void PrintInterruptContext(InterruptContext * context)
{
	u16 cs = 0;

	asm("mov %%cs, %%ax; mov %%ax, %0" : "=m" (cs) : );

	kprint("eax = %x, ebx = %x, ecx = %x, edx = %x\n", context->eax, context->ebx, context->ecx, context->edx);
	kprint("esp = %x, ebp = %x, esi = %x, edi = %x\n", context->esp, context->ebp, context->esi, context->edi);
	kprint("eip = %x", context->eip);
	kprint("gs = %x, fs = %x, es = %x, ds = %x\n", context->gs, context->fs, context->es, context->ds, cs);
}

void PrintExceptionContext(ExceptionContext * context)
{
    u16 cs = 0;

    asm("mov %%cs, %%ax; mov %%ax, %0" : "=m" (cs) : );

	kprint("eax = %x, ebx = %x, ecx = %x, edx = %x\n", context->eax, context->ebx, context->ecx, context->edx);
	kprint("esp = %x, ebp = %x, esi = %x, edi = %x\n", context->esp, context->ebp, context->esi, context->edi);
	kprint("eip = %x", context->eip);
	kprint("gs = %x, fs = %x, es = %x, ds = %x\n", context->gs, context->fs, context->es, context->ds, cs);
	kprint("eflags = %x (%b)\n", context->eflags, context->eflags);
	kprint("cr0 = %x (%b)\n", context->cr0, context->cr0);
	kprint("cr2 = %x (%b)\n", context->cr2, context->cr2);
	kprint("cr3 = %x (%b)\n", context->cr3, context->cr3);
}

void PrintExceptionContextWithCode(ExceptionContextWithCode * context)
{
    u16 cs = 0;

    asm("mov %%cs, %%ax; mov %%ax, %0" : "=m" (cs) : );

	kprint("eax = %x, ebx = %x, ecx = %x, edx = %x\n", context->eax, context->ebx, context->ecx, context->edx);
	kprint("esp = %x, ebp = %x, esi = %x, edi = %x\n", context->esp, context->ebp, context->esi, context->edi);
	kprint("eip = %x\n", context->eip);
	kprint("gs = %x, fs = %x, es = %x, ds = %x, cs = %x\n", context->gs, context->fs, context->es, context->ds, cs);
	kprint("eflags = %x (%b)\n", context->eflags, context->eflags);
	kprint("cr0 = %x (%b)\n", context->cr0, context->cr0);
	kprint("cr2 = %x (%b)\n", context->cr2, context->cr2);
	kprint("cr3 = %x (%b)\n", context->cr3, context->cr3);
	kprint("Error code : %x (%b)\n", context->code, context->code);
}
