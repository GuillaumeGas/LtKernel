#include "idt.h"
#include "proc_io.h"

void init_idt ()
{
    cli (); // on desactive les interruptions
    init_pic ();

    //sti (); // on reactive les interruptions
}

void init_pic ()
{
    // ICW1, port 0x20 (et 0xA0 pour esclave)
    // 0 0 0 1 . 0 0 0 0
    // Les 4 derniers bits sont pour :
    //  - le déclenchement par niveau (1) ou front (0)
    //  - /
    //  - un seul controleur (1) ou cascade (0)
    //     (?) J'ai essaye le mode simple, mais il ne semble pas supporte !
    //  - avec ICW4 (1) ou sans (0)
    outbp (0x20, 0x10);
    
    
}
