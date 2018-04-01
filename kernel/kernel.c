#include "screen.h"
#include "types.h"
#include "memory.h"
/* #include "gdt.h" */
#include "idt.h"
#include "proc_io.h"

void kmain (void);

void _asm_gdt_init (void);
void _asm_idt_init (void);

void _start (void)
{
    clear();
    
    setColor (WHITE);
    print ("< ## LtKernel ## >\n");
    
    println ("[Boot] IDT loaded");

    init_pic ();
    /* println ("[Boot] PIC loaded"); */

    /* init_gdt (); */
    _asm_gdt_init ();
    
    asm("movw $0x18, %ax \n \
         movw %ax, %ss \n \
         movl $0x20000, %esp");
    
    kmain ();
}

void kmain (void)
{    
    /* init_idt (); */
    _asm_idt_init ();
    
    sti ();
    println ("[Boot] GDT loaded\n");
    println ("[Kernel] Interrupts enabled\n");

    setColor (RED);
    println ("Hello from LtKernel !");
    setColor (WHITE);

    while (1);
}
