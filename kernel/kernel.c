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

    sti ();
    setColorEx (BLACK, BLUE, 0, 1);
    println ("[Kernel] Interrupts enabled\n");

    setColorEx (BLACK, RED, 0, 1);
    println ("Hello from LtKernel !");
    setColor (WHITE);

    /* println ("Starting new task..."); */
    memcopy ((u8*)test_task, (u8*)0x30000, 100);

    // on désactive les interruptions pendant la commutation de tâche (commutation software)
    // on utilise le mécanisme utilisé par le proc quand il termine l'exécution d'une interruption
    // il va dépiler tous ces éléments pour restaurer les registres et retourner sur la précédente
    // tâche.
    // On push donc les éléments qui nous intéressent, afin qu'il saute sur la tâche utilisateur
    // On prend garde de changer les droits des segments en ring 3 car ce sont les droits donnés
    // dans les descripteurs de segments utilisateur dans la gdt
    // On modifie également l'EFLAGS afin de désactiver le bit NT (Nested Task) et donc la commutation hardware
    // ainsi que le bit IF afin d'autoriser les interrupts une fois dans la tâche utilisateur.
    cli ();
    asm ("push $0x23");
    asm ("push $0x30000 ");
    asm ("pushfl");
    asm ("popl %eax ");
    asm ("orl $0x200, %eax ");
    asm ("and $0xffffbfff, %eax ");
    asm ("push %eax ");
    asm ("push $0x1B ");
    asm ("push $0x30000 ");
    // on garde en mémoire le pointeur sur la pile noyau qui sera utilisé pour de futur interruptions
    asm ("movl $0x20000, %0" : "=m" (g_tss.esp0) : );
    asm ("movw $0x23, %ax ");
    asm ("movw %ax, %ds ");
    asm ("iret");
    
    while (1);
}
