#include "../drivers/screen.h"
/* #include "gdt.h" */

#include "../utils/types.h"
#include "../utils/memory.h"
#include "gdt.h"

void kmain (void);

void _start (void) {
    clear();

    setColor (WHITE);
    print ("< ## LtKernel ## >\n");

    print (">> Loading GDT...");
    init_gdt ();

    print ("OK\n");

    kmain ();
}

void kmain (void) {
    setColor (RED);
    print ("Hello from LtKernel !\n");

    while (1) {}
}
