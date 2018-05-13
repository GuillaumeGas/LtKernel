#include "debug.h"

#include "../drivers/screen.h"
#include "../init/gdt.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"

void kdump ()
{
    sc_clear ();
    sc_setBackground (BLUE);

    sc_setColorEx (BLUE, RED, 0, 1);
    kprint (">> Debug breakpoint\n\n");
    
    sc_setColorEx (BLUE, WHITE, 0, 1);

    print_gdt ();
    kprint ("\n");
    print_tss ();
    
    pause ();
}

void panic (const char * str)
{
    sc_clear ();
    sc_setBackground (BLUE);

    sc_setColorEx (BLUE, RED, 0, 1);
    kprint (">> %s\n\n", str);
    
    sc_setColorEx (BLUE, WHITE, 0, 1);

    print_gdt ();
    kprint ("\n");
    print_tss ();
    
    /* pause (); */
}
