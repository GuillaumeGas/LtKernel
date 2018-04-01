#define __IDT__
#include "idt.h"
#include "proc_io.h"
#include "memory.h"
#include "screen.h"

void _asm_default_irq (void);
void _asm_clock_irq (void);
void _asm_idt_init(void);

void init_idt (void)
{
    _asm_idt_init ();
    
    /* int i = 0; */
    
    /* /\* cli (); // on desactive les interruptions *\/ */

    /* u16 select = 0x08; */
    
    /* for (i = 0; i < IDT_SIZE; i++) */
    /* 	init_idt_descriptor ((u32)_asm_default_irq, select, 0x8E00, &idt_desc[i]); */
    
    /* // premier segment (code), offset 0x8 dans la gdt */
    /* // 0x9C00, segment en mémoire, privilège super utilisateur */
    /* // 33 ème entrée dans l'idt car les 32 premiers sont réservés au exceptions */
    /* // type : 0x8E00 */
    /* //        1 0 0 0 . 1 1 1 0 . 0 0 0 0 . 0 0 0 0 */
    /* init_idt_descriptor ((u32) _asm_clock_irq, select, 0x8E00, &idt_desc[32]); */

    /* idt.limit = IDT_SIZE * 8; */
    /* idt.base = IDT_ADDR; */

    /* memcopy ((u8*)idt_desc, (u8*)idt.base, idt.limit); */
    
    /* asm ("lidtl (idt)"); */
}

void init_idt_descriptor (u32 offset_irq, u16 selector, u16 type, struct Idt_descriptor * desc)
{
    desc->offset0_15 = (offset_irq && 0xFFFF);
    desc->selector = selector;
    desc->type = type;
    desc->offset16_31 = (offset_irq & 0xFFFF0000) >> 16;
    return;
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
    outb (0x20, 0x11);
    outb (0xA0, 0x11);
    
    // ICW2, port 0x21 (et 0xA1 pour esclave)
    // x x x x x 0 0 0
    // Les bits de poids fort servent à stocker l'adresse de base du vecteur d'interruption
    // Il correspondent en fait à un offset dans l'idt
    // Ici, on ne configure que le controleur maître, donc les IRQs 0-7, pour qu'il utilise
    // les interruptions à partir de l'offset 0x20 dans l'idt.
    // Sous archi type x86, les 32 premiers vecteurs sont réservés à la gestion des exceptions.
    outb (0x21, 0x20);
    outb (0xA1, 0x70);

    // On utilise pas de controlleur esclave pour le moment, donc pas besoin de renseigner les
    // registres ICW3 et ICW4

    // finalement si
    outb (0x21, 0x04);
    outb (0xA1, 0x01);

    outb (0x21, 0x01);
    outb (0xA1, 0x01);
    
    // OCW1, port 0x21 (et 0xA1 pour esclave)
    // x x x x x x x x
    // Chaque bit permet de masquer une interruption (1) ou non (0)
    // 1 1 1 1 1 1 1 0 // on masque tout sauf l'horloge système
    outb (0x21, 0xFC);
    outb (0xA1, 0xFF);
}
