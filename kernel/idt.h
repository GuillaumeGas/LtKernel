#ifndef __DEF_IDT__
#define __DEF_IDT__

#include "types.h"

#define IDT_SIZE 0xFF // 32 (exceptions) + 16 irqs
#define IDT_ADDR 0x800 // bon, pk pas

// Liste des IRQs
/*

    IRQ 0 : Horloge Système
    IRQ 1 : Clavier
    IRQ 2 : N/A (cascade du second contrôleur)
    IRQ 3 : Port série (COM2/COM4)
    IRQ 4 : Port série (COM1/COM3)
    IRQ 5 : LPT2 (carte de son)
    IRQ 6 : Lecteur de disquettes
    IRQ 7 : Port parallèle (LPT1)
    IRQ 8 : Horloge temps réel
    IRQ 9 : N/A (PCI)
    IRQ 10 : N/A
    IRQ 11 : N/A (USB)
    IRQ 12 : N/A (PS/2)
    IRQ 13 : Coprocesseur math.
    IRQ 14 : Disque dur primaire
    IRQ 15 : Disque dur secondaire

 */

// Concernant le champ type :
//  P D P L . 0 1 1 1 . 0 0 0 x . x x x x
//  P : indique si le segment est présent en mémoire (1) ou non (0)
//  Dpl : niveau de privilège du segment. (super utilisateur : 0)
//  Les x, c'est le reste du champ de 16 bits qui n'est pas pris en compte
struct Idt_descriptor
{
    u16 offset0_15;
    u16 selector;
    u16 type;
    u16 offset16_31;
} __attribute__ ((packed));

struct Idt
{
    u16 limit;
    u32 base;
} __attribute__ ((packed));

void init_idt (void);
void init_idt_descriptor (u32 offset_irq, u16 selector, u16 type, struct Idt_descriptor * desc);
void init_pic ();

/* #ifdef __IDT__ */
/* struct Idt idt; */
/* struct Idt_descriptor idt_desc[IDT_SIZE]; */
/* #else */
/* extern struct Idt idt; */
/* extern struct Idt_descriptor idt_desc[]; */
/* #endif */

#endif
