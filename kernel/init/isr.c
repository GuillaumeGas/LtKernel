#include "../drivers/screen.h"
#include "../drivers/serial.h"
#include "../drivers/proc_io.h"
#include "../drivers/kbmap.h"
#include "../lib/types.h"

#include "../init/gdt.h"

int next_instruction = 0;

void divided_by_zero_isr (void) { setColor (RED); println ("[Fault] Divided by zero"); setColor (WHITE); }
void debug_isr (void) { setColor (RED); println ("[Fault/Trap] Debug"); setColor (WHITE); }
void non_maskable_int_isr (void) { setColor (RED); println ("[Interrupt] Non-maskable Interrupt"); setColor (WHITE); }
void breakpoint_isr (void)
{
    setColor (RED); println ("[Trap] Breakpoint"); setColor (WHITE);
    write_serial ('a');

    /* print_gdt (); */
    /* print_gdt_in_memory (); */
    
    while (1) {}
}	
void overflow_isr (void) { setColor (RED); println ("[Trap] Overflow"); setColor (WHITE); }
void bound_range_exceeded_isr (void) { setColor (RED); println ("[Fault] Bound Range Exceeded"); setColor (WHITE); }
void invalid_opcode_isr (void) { setColor (RED); println ("[Fault] Invalid Opcode"); setColor (WHITE); }
void device_not_available_isr (void) { setColor (RED); println ("[Fault] Device not available"); setColor (WHITE); }
void double_fault_isr (void) { setColor (RED); println ("[Abort] Double fault"); setColor (WHITE); }
void invalid_tss_isr (void) { setColor (RED); println ("[Fault] Invalid TSS"); setColor (WHITE); }
void segment_not_present_isr (void) { setColor (RED); println ("[Fault] Segment not present"); setColor (WHITE); }
void stack_segment_fault_isr (void) { setColor (RED); println ("[Fault] Stack-Segment fault"); setColor (WHITE); while (1) {} }
void general_protection_fault_isr (void) { setColor (RED); println ("[Fault] General protection fault"); setColor (WHITE); }

void page_fault_isr (void)
{
    setColor (RED);
    /* register void * cr2 asm ("cr2"); */
    /* printInt (254); */
    print ("[Fault] Page fault ! Caused by the virtual address 0x");
    /* printInt (cr2); */
    println ("");
    setColor (WHITE);
}

void x87_floating_point_isr (void) { setColor (RED); println ("[Fault] x87 Floating-point exception"); setColor (WHITE); }
void alignment_check_isr (void) { setColor (RED); println ("[Fault] Alignment check"); setColor (WHITE); }
void machine_check_isr (void) { setColor (RED); println ("[Abort] Machine check"); setColor (WHITE); }
void simd_floating_point_isr (void) { setColor (RED); println ("[Fault] SIMD Floating-point exception"); setColor (WHITE); }
void virtualization_isr (void) { setColor (RED); println ("[Fault] Virtualization exception"); setColor (WHITE); }
void security_isr (void) { setColor (RED); println ("[!] Security exception"); setColor (WHITE); }
void triple_fault_isr (void) { setColor (RED); println ("[!] Triple fault"); setColor (WHITE); }


void default_isr (void)
{
    println ("Unhandled interrupt");
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
	    println (".");
	/* else */
	/*     println ("tac"); */
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
	    printChar(kbdmap
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
    println ("COM1 event !");
}
