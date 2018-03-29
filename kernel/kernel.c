#include "screen.h"
#include "types.h"
#include "memory.h"
#include "gdt.h"
#include "idt.h"

void kmain (void);

void _start (void)
{
    clear();

    setColor (WHITE);
    println ("< ## LtKernel ## >");
    print (">> Loading GDT...");

    init_gdt ();
    
    asm("movw $0x18, %ax \n \
         movw %ax, %ss \n \
         movl $0x20000, %esp");
    
    println ("OK");

    print (">> Loading IDT...");

    init_idt ();

    println ("O...");
    
    kmain ();
}

void kmain (void)
{
    setColor (RED);
    println ("Hello from LtKernel !");
    setColor (WHITE);

    while (1) {}
}
