#include "../drivers/screen.h"
#include "../drivers/serial.h"
#include "../drivers/proc_io.h"
#include "../drivers/kbmap.h"
#include "../lib/types.h"
#include "../lib/stdio.h"

#include "gdt.h"

void divided_by_zero_isr (void) { sc_setColor (RED); kprint ("[Fault] Divided by zero"); sc_setColor (WHITE); }
void debug_isr (void) { sc_setColor (RED); kprint ("[Fault/Trap] Debug"); sc_setColor (WHITE); }
void non_maskable_int_isr (void) { sc_setColor (RED); kprint ("[Interrupt] Non-maskable Interrupt"); sc_setColor (WHITE); }
void breakpoint_isr (void)
{
    sc_clear ();
    
    sc_setColor (RED); kprint ("[Trap] Breakpoint\n"); sc_setColor (WHITE);
    write_serial ('a');

    print_gdt ();
    kprint ("\n");
    print_tss ();
    
    while (1) {}
}	
void overflow_isr (void) { sc_setColor (RED); kprint ("[Trap] Overflow"); sc_setColor (WHITE); }
void bound_range_exceeded_isr (void) { sc_setColor (RED); kprint ("[Fault] Bound Range Exceeded"); sc_setColor (WHITE); }
void invalid_opcode_isr (void) { sc_setColor (RED); kprint ("[Fault] Invalid Opcode"); sc_setColor (WHITE); }
void device_not_available_isr (void) { sc_setColor (RED); kprint ("[Fault] Device not available"); sc_setColor (WHITE); }
void double_fault_isr (void) { sc_setColor (RED); kprint ("[Abort] Double fault"); sc_setColor (WHITE); }
void invalid_tss_isr (void) { sc_setColor (RED); kprint ("[Fault] Invalid TSS"); sc_setColor (WHITE); }
void segment_not_present_isr (void) { sc_setColor (RED); kprint ("[Fault] Segment not present"); sc_setColor (WHITE); }
void stack_segment_fault_isr (void) { sc_setColor (RED); kprint ("[Fault] Stack-Segment fault"); sc_setColor (WHITE); while (1) {} }
void general_protection_fault_isr (void) { sc_setColor (RED); kprint ("[Fault] General protection fault"); sc_setColor (WHITE); }

void page_fault_isr (void)
{
    sc_setColor (RED);
    /* register void * cr2 asm ("cr2"); */
    /* printInt (254); */
    kprint ("[Fault] Page fault ! Caused by the virtual address 0x\n");
    /* printInt (cr2); */
    sc_setColor (WHITE);
}

void x87_floating_point_isr (void) { sc_setColor (RED); kprint ("[Fault] x87 Floating-point exception"); sc_setColor (WHITE); }
void alignment_check_isr (void) { sc_setColor (RED); kprint ("[Fault] Alignment check"); sc_setColor (WHITE); }
void machine_check_isr (void) { sc_setColor (RED); kprint ("[Abort] Machine check"); sc_setColor (WHITE); }
void simd_floating_point_isr (void) { sc_setColor (RED); kprint ("[Fault] SIMD Floating-point exception"); sc_setColor (WHITE); }
void virtualization_isr (void) { sc_setColor (RED); kprint ("[Fault] Virtualization exception"); sc_setColor (WHITE); }
void security_isr (void) { sc_setColor (RED); kprint ("[!] Security exception"); sc_setColor (WHITE); }
void triple_fault_isr (void) { sc_setColor (RED); kprint ("[!] Triple fault"); sc_setColor (WHITE); }


void default_isr (void)
{
    kprint ("Unhandled interrupt\n");
}

void clock_isr (void)
{
    static int tic = 0;
    static int sec = 0;
    tic++;
    if (tic % 100 == 0){
	sec++;
	tic = 0;
	if (sec % 2 == 0)
	    kprint (".");
	/* else */
	/*     kprint ("tac"); */
    }
}

void keyboard_isr (void)
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
	/* default: */
	    kprint ("%c", kbdmap
		   [i * 4 + (lshift_enable || rshift_enable)]);
	}
    } else {                /* touche relachee */
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

void com1_isr ()
{
    kprint ("One byte received on COM port : %c\n", read_serial ());
}
