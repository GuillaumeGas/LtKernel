#include "screen.h"
#include "types.h"
#include "memory.h"
#include "gdt.h"

void kmain (void);

void _start (void) {
    init_gdt ();
    clear();
    setColor (WHITE);
    print ("< ## LtKernel ## >\n");
    print (">> Loading GDT...");
    print ("OK\n");
    kmain ();
}

void kmain (void) {
    setColor (RED);
    print ("Hello from LtKernel !");

    while (1) {}
}
