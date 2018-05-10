#include "lib/types.h"
#include "lib/memory.h"
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
    clear();
    
    setColorEx (BLACK, WHITE, 0, 1);
    println ("< ## LtKernel ## >\n");

    setColorEx (BLACK, CYAN, 0, 1);

    init_pic ();
    println ("[Boot] PIC loaded");

    init_idt ();
    println ("[Boot] IDT loaded");
    
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
    println ("[Boot] GDT loaded\n");

    init_serial ();
    println ("[Boot] Serial port COM1 initialized");
    
    sti ();
    setColorEx (BLACK, BLUE, 0, 1);
    println ("[Kernel] Interrupts enabled\n");
    
    setColorEx (BLACK, RED, 0, 1);
    println ("Hello from LtKernel !");
    setColor (WHITE);

    /* println ("Starting new task..."); */
    /* memcopy ((u8*)test_task, (u8*)0x30000, 100); */

    /* task_switch (); */
    /* asm ("int $3"); */
    while (1);
}
