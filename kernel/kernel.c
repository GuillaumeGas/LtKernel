#include "lib/types.h"
#include "lib/memory.h"
#include "lib/stdio.h"
#include "init/gdt.h"
#include "init/idt.h"
#include "drivers/proc_io.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "logger.h"

void kmain (void);
void task_switch (void);

void _start (void)
{
    cli ();

    init_logger (LOG_SCREEN);
    
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

// Test utilisateur (CPL 3)
void test_task ()
{
    char * str = (char*) 0x30100;
    str[0] = 'H';
    str[1] = 'e';
    str[2] = 'l';
    str[3] = 'l';
    str[4] = 'o';
    str[5] = 'w';
    asm ("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (str));
    while (1) {}
}

void kmain (void)
{
    init_serial ();
    init_logger (LOG_SCREEN | LOG_SERIAL);
    
    kprint ("[Kernel] GDT loaded\n");
    kprint ("[Kernel] Serial port COM1 initialized\n");
    
    sti ();
    sc_setColor (BLUE);
    kprint ("[Kernel] Interrupts enabled\n\n");
    
    sc_setColor (RED);
    kprint ("Hello from LtKernel !\n");
    sc_setColor (WHITE);

    kprint ("Starting new task...\n");
    mmcopy ((u8*)test_task, (u8*)0x30000, 100);

    /* asm ("int $3"); */
    
    task_switch ();
   
    while (1);
}
