#include <kernel/drivers/screen.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/proc_io.h>
#include <kernel/drivers/kbmap.h>

#include <kernel/lib/types.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

#include <kernel/debug/debug.h>
#include <kernel/scheduler.h>

#include <kernel/drivers/proc_io.h>

#include "gdt.h"

#define EXCEPTION_SCREEN \
	sc_clear(); \
	sc_setBackground(BLUE); \

//#define CLOCK_DEBUG

static void DefaultExceptionHandler(ExceptionContext * context, const char * str);

void divided_by_zero_isr(ExceptionContext * context) 
{ 
	DefaultExceptionHandler(context, "[Fault] Divided by zero");
}

void debug_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault/Trap] Debug");
}

void non_maskable_int_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Interrupt] Non-maskable Interrupt");
}

void breakpoint_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Trap] Breakpoint");
}

void overflow_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Trap] Overflow");
}

void bound_range_exceeded_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] Bound Range Exceeded");
}

void invalid_opcode_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] Invalid Opcode");
}

void device_not_available_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] Device not available");
}

void double_fault_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Abort] Double fault");
}

void invalid_tss_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] Invalid TSS");
}

void segment_not_present_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] Segment not present");
}

void stack_segment_fault_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] Stack-Segment fault");
}

void general_protection_fault_isr(ExceptionContextWithCode * context)
{
	u32 code = context->code;
	// https://wiki.osdev.org/Exceptions#Selector_Error_Code
	u8 E = (code & 1);
	u8 Tlb = ((code >> 1) & 3);
	u8 Index = (code >> 3);

	EXCEPTION_SCREEN

	sc_setColorEx(BLUE, RED, 0, 1);
	kprint(">> [Fault] General protection fault ! \n\n");

	sc_setColorEx(BLUE, WHITE, 0, 1);

	kprint("Selector Error Code : %x (%b)\n", code, code);
	
	kprint("E = %d, ", E);
	if (E == 1)
		kprint("The exception originated externally to the processor.\n");
	else
		kprint("\n");

	kprint("Tlb : %b, ", Tlb);
	if (Tlb == 0)
		kprint("The Selector Index references a descriptor in the GDT.\n");
	if (Tlb == 1)
		kprint("The Selector Index references a descriptor in the IDT.\n");
	if (Tlb == 2)
		kprint("The Selected Index references a descriptor in the LDT.\n");
	if (Tlb == 3)
		kprint("The Selected Index references a descriptor in the IDT.\n");

	kprint("Selector index : %x\n\n", Index);

	PrintExceptionContextWithCode(context);

	hlt();
}

void page_fault_isr(ExceptionContextWithCode * context)
{
	u32 code = context->code;

	EXCEPTION_SCREEN

	sc_setColorEx(BLUE, RED, 0, 1);
	kprint(">> [Fault] Page fault ! \n\n");

	sc_setColorEx(BLUE, WHITE, 0, 1);

	u8 p = (code & 1);
	u8 wr = (code & 2);
	u8 us = (code & 4);
	u8 rsvd = (code & 8);
	u8 id = (code & 16);

	kprint ("Error code : %x (%b)\n", code, code);
	kprint (" - P : %d (%s)\n", p ? 1 : 0, p ? "protection violation" : "non-present page");
	kprint (" - W/R : %d (%s)\n", wr ? 1 : 0, wr ? "write access" : "read access");
	kprint (" - U/S : %d (%s)\n", us ? 1 : 0, us ? "user mode" : "supervisor mode");
	kprint (" - RSVD : %d (%s)\n", rsvd ? 1 : 0, rsvd ? "one or more page directory entries contain reserved bits which are set to 1" : "PSE or PAE flags in CR4 are set to 1");
	kprint (" - I/D : %d (%s)\n\n", id ? 1 : 0, id ? "instruction fetch (applies when the No-Execute bit is supported and enabled" : "-");
	kprint("Linear address : %x\n\n", context->cr2);

	PrintExceptionContextWithCode(context);

	hlt();
}

void x87_floating_point_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] x87 Floating-point exception");
}

void alignment_check_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] Alignment check");
}

void machine_check_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Abort] Machine check");
}

void simd_floating_point_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] SIMD Floating-point exception");
}

void virtualization_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[Fault] Virtualization exception");
}

void security_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[!] Security exception");
}

void default_isr(ExceptionContext * context)
{
	DefaultExceptionHandler(context, "[!] Unhandled interrupt !");
}

void clock_isr(void)
{
	static int tic = 0;
	static int sec = 0;
	tic++;
	if (tic % 100 == 0) {
		sec++;
		tic = 0;
#ifdef CLOCK_DEBUG
		if (sec % 2 == 0)
			kprint(".");
#endif
	}
	schedule();
}

void keyboard_isr(void)
{
	uchar i;
	static int lshift_enable;
	static int rshift_enable;
	/* static int alt_enable; */
	/* static int ctrl_enable; */

	do {
		i = inb(0x64);
	} while ((i & 0x01) == 0);

	i = inb(0x60);
	i--;

	if (i < 0x80) {         /* touche enfoncee */
		switch (i) {
		case 0x29:
			lshift_enable = 1;
			break;
		case 0x35:
			rshift_enable = 1;
			break;
			/* case 0x1C: */
			/*     ctrl_enable = 1; */
			/*     break; */
			/* case 0x37: */
			/*     alt_enable = 1; */
			/*     break; */
		default:
			kprint("%c", kbdmap
				[i * 4 + (lshift_enable || rshift_enable)]);
		}
	}
	else {                /* touche relachee */
		i -= 0x80;
		switch (i) {
		case 0x29:
			lshift_enable = 0;
			break;
		case 0x35:
			rshift_enable = 0;
			break;
			/* case 0x1C: */
			/*     ctrl_enable = 0; */
			/*     break; */
			/* case 0x37: */
			/*     alt_enable = 0; */
			/*     break; */
		}
	}
}

void com1_isr()
{
	kprint("One byte received on COM port : %c\n", read_serial());
}

void syscall_isr(int syscall_number)
{
	char * message = NULL;

	switch (syscall_number) {
	case 1:
		// On va chercher le param qu'on a passe dans ebx, sauvegarde sur la pile
		asm("mov 44(%%ebp), %%eax; mov %%eax, %0" : "=m" (message));
		kprint(message);
		break;
	default:
		kprint("Unhandled syscall number !");
	}
}

static void DefaultExceptionHandler(ExceptionContext * context, const char * str)
{
	EXCEPTION_SCREEN

	sc_setColorEx(BLUE, RED, 0, 1);
	kprint(str);
	kprint("\n\n");
	sc_setColorEx(BLUE, WHITE, 0, 1);

	PrintExceptionContext(context);

	asm("hlt");
}