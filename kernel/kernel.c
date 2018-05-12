#include "lib/types.h"
#include "lib/memory.h"
#include "lib/stdio.h"
#include "init/gdt.h"
#include "init/idt.h"
#include "drivers/proc_io.h"
#include "drivers/screen.h"
#include "drivers/serial.h"

void kmain (void);

void _asm_gdt_init (void);
void task_switch (void);

void _start (void)
{
    cli ();
    
    sc_clear();
    
    sc_setColor (WHITE);
    kprint ("< ## LtKernel ## >\n\n");

    sc_setColor (CYAN);

    init_pic ();
    kprint ("[Boot] PIC loaded\n");

    init_idt ();
    kprint ("[Boot] IDT loaded\n");
    
    init_gdt ();
    
    asm("movw $0x10, %ax \n \
         movw %ax, %ss \n \
         movl $0x20000, %esp");
    
    kmain ();
}

void test_task ()
{
    while (1) {}
}

void kmain (void)
{
    kprint ("[Boot] GDT loaded\n");

    init_serial ();
    kprint ("[Boot] Serial port COM1 initialized\n");
    
    sti ();
    sc_setColor (BLUE);
    kprint ("[Kernel] Interrupts enabled\n\n");
    
    sc_setColor (RED);
    kprint ("Hello from LtKernel !\n");
    sc_setColor (WHITE);

    /* kprint ("Starting new task...\n"); */
    /* mmcopy ((u8*)test_task, (u8*)0x30000, 100); */

    /* task_switch (); */
    asm ("int $3");
    while (1);
}
