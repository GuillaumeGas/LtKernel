#include "../drivers/screen.h"

void _start (void) {
    clear();

    setColor (WHITE);
    print ("< ## LtKernel ## >\n");
    setColor (RED);
    print ("Hello from LtKernel !\n");

    while (1) {}
}
