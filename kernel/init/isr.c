#include <kernel/drivers/screen.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/proc_io.h>
#include <kernel/drivers/clock.h>
#include <kernel/drivers/keyboard.h>

#include <kernel/lib/types.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

#include <kernel/debug/debug.h>
#include <kernel/scheduler.h>

#include <kernel/init/gdt.h>

#include <kernel/user/syscalls.h>

#include "isr.h"

#define EXCEPTION_SCREEN \
	ScClear(); \
	ScSetBackground(BLUE); \

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

	ScSetColorEx(BLUE, RED, 0, 1);
	kprint(">> [Fault] General protection fault ! \n\n");

	ScSetColorEx(BLUE, WHITE, 0, 1);

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

	// coming from kernel
	if (context->cr3 == 0x1000)
	{
		PrintExceptionContextWithCode(context);
	}
	else
	{
		PrintExceptionUserContextWithCode((ExceptionContextUserWithCode *)context);
	}

	Pause();
}

void page_fault_isr(ExceptionContextWithCode * context)
{
	u32 code = context->code;

	EXCEPTION_SCREEN

		ScSetColorEx(BLUE, RED, 0, 1);
	kprint(">> [Fault] Page fault ! \n\n");

	ScSetColorEx(BLUE, WHITE, 0, 1);

	u8 p = (code & 1);
	u8 wr = (code & 2);
	u8 us = (code & 4);
	u8 rsvd = (code & 8);
	u8 id = (code & 16);

	kprint("Error code : %x (%b)\n", code, code);
	kprint(" - P : %d (%s)\n", p ? 1 : 0, p ? "protection violation" : "non-present page");
	kprint(" - W/R : %d (%s)\n", wr ? 1 : 0, wr ? "write access" : "read access");
	kprint(" - U/S : %d (%s)\n", us ? 1 : 0, us ? "user mode" : "supervisor mode");
	kprint(" - RSVD : %d (%s)\n", rsvd ? 1 : 0, rsvd ? "one or more page directory entries contain reserved bits which are set to 1" : "PSE or PAE flags in CR4 are set to 1");
	kprint(" - I/D : %d (%s)\n\n", id ? 1 : 0, id ? "instruction fetch (applies when the No-Execute bit is supported and enabled" : "-");
	kprint("Linear address : %x, %b*\n\n", context->cr2, context->cr2, 32);

	if (us == 1)
	{
		PrintExceptionContextWithCode(context);
	}
	else
	{
		PrintExceptionUserContextWithCode((ExceptionContextUserWithCode *)context);
	}

	Pause();
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
	//DefaultExceptionHandler(context, "[!] Unhandled interrupt !");
}

void clock_isr(void)
{
    DrvClock();
	Schedules();
}

void keyboard_isr(void)
{
    DrvKeyboard();
}

void com1_isr()
{
	//kprint("One byte received on COM port : %c\n", SerialRead());
}

void syscall_isr(int syscallNumber, InterruptContext * context)
{
    SyscallHandler(syscallNumber, context);
}

static void DefaultExceptionHandler(ExceptionContext * context, const char * str)
{
	EXCEPTION_SCREEN

	ScSetColorEx(BLUE, RED, 0, 1);
	kprint(str);
	kprint("\n\n");
	ScSetColorEx(BLUE, WHITE, 0, 1);

	PrintExceptionContext(context);

	asm("hlt");
}