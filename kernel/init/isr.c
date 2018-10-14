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

#define CLOCK_DEBUG

u32 get_cr2();

void divided_by_zero_isr(void) { sc_setColor(RED); panic("[Fault] Divided by zero"); sc_setColor(WHITE); }
void debug_isr(void) { sc_setColor(RED); panic("[Fault/Trap] Debug"); sc_setColor(WHITE); }
void non_maskable_int_isr(void) { sc_setColor(RED); panic("[Interrupt] Non-maskable Interrupt"); sc_setColor(WHITE); }
void breakpoint_isr(void)
{
	kdump();

	while (1) {}
}
void overflow_isr(void) { sc_setColor(RED); panic("[Trap] Overflow"); sc_setColor(WHITE); }
void bound_range_exceeded_isr(void) { sc_setColor(RED); panic("[Fault] Bound Range Exceeded"); sc_setColor(WHITE); }
void invalid_opcode_isr(void) { sc_setColor(RED); panic("[Fault] Invalid Opcode"); sc_setColor(WHITE); }
void device_not_available_isr(void) { sc_setColor(RED); panic("[Fault] Device not available"); sc_setColor(WHITE); }
void double_fault_isr(void) { sc_setColor(RED); panic("[Abort] Double fault"); sc_setColor(WHITE); }
void invalid_tss_isr(void) { sc_setColor(RED); panic("[Fault] Invalid TSS"); sc_setColor(WHITE); }
void segment_not_present_isr(void) { sc_setColor(RED); panic("[Fault] Segment not present"); sc_setColor(WHITE); }
void stack_segment_fault_isr(void) { sc_setColor(RED); panic("[Fault] Stack-Segment fault"); sc_setColor(WHITE); while (1) {} }
void general_protection_fault_isr(void) { sc_setColor(RED); panic("[Fault] General protection fault"); sc_setColor(WHITE); }

void page_fault_isr(u32 code)
{
	u32 * eip = 0;
	u32 cr2 = 0;
	asm("mov 56(%%ebp), %%eax; mov %%eax, %0" : "=m" (eip));
	//asm("mov %%cr2, %%eax; mov %%eax, %0" : "=m" (cr2) : );

	//cr2 = get_cr2();

	sc_clear();
	sc_setBackground(BLUE);

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
	kprint("Linear address : %x\n", cr2);

	/*kprint("EIP : %x, CR2 : %x\n\n", eip, cr2);
	print_gdt();
	kprint("\n");
	print_tss();*/

	pause();

	/* sc_setColor (RED); */
	/* register void * cr2 asm ("cr2"); */
	/* printInt (254); */
	/* panic ("[Fault] Page fault !"); */
	/* printInt (cr2); */
	/* sc_setColor (WHITE); */
}

void x87_floating_point_isr(void) { sc_setColor(RED); panic("[Fault] x87 Floating-point exception"); sc_setColor(WHITE); }
void alignment_check_isr(void) { sc_setColor(RED); panic("[Fault] Alignment check"); sc_setColor(WHITE); }
void machine_check_isr(void) { sc_setColor(RED); panic("[Abort] Machine check"); sc_setColor(WHITE); }
void simd_floating_point_isr(void) { sc_setColor(RED); panic("[Fault] SIMD Floating-point exception"); sc_setColor(WHITE); }
void virtualization_isr(void) { sc_setColor(RED); panic("[Fault] Virtualization exception"); sc_setColor(WHITE); }
void security_isr(void) { sc_setColor(RED); panic("[!] Security exception"); sc_setColor(WHITE); }
void triple_fault_isr(void) { sc_setColor(RED); panic("[!] Triple fault"); sc_setColor(WHITE); }


void default_isr(void)
{
	kprint("Unhandled interrupt\n");
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
