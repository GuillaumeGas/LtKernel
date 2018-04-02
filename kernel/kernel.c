#include "screen.h"
#include "types.h"
#include "memory.h"
#include "gdt.h"
#include "idt.h"
#include "proc_io.h"

void kmain (void);

void _asm_gdt_init (void);

void _start (void)
{
    clear();
    
    setColorEx (BLACK, WHITE, 0, 1);
    print ("< ## LtKernel ## >\n");

    setColorEx (BLACK, CYAN, 0, 1);
    println ("[Boot] IDT loaded");

    init_pic ();
    println ("[Boot] PIC loaded");

    init_idt ();
    println ("[Boot] IDT loaded");
    
    init_gdt ();
    
    asm("movw $0x18, %ax \n \
         movw %ax, %ss \n \
         movl $0x20000, %esp");
    
    kmain ();
}

void kmain (void)
{        
    println ("[Boot] GDT loaded\n");

    sti ();
    setColorEx (BLACK, BLUE, 0, 1);
    println ("[Kernel] Interrupts enabled\n");

    setColorEx (BLACK, RED, 0, 1);
    println ("Hello from LtKernel !");
    setColor (WHITE);

    while (1);
}
