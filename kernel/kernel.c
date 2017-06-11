#include "screen.h"
#include "types.h"
#include "memory.h"
#include "gdt.h"

void kmain (void);

void _start (void) {
    clear();

    setColor (WHITE);
    println ("< ## LtKernel ## >");
    print (">> Loading GDT...");

    init_gdt ();
    println ("OK");

    kmain ();
}

void kmain (void) {
    setColor (RED);
    println ("Hello from LtKernel !");
    setColor (WHITE);

    while (1) {}
}
