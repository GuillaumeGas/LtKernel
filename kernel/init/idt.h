#pragma once

#include <kernel/lib/types.h>

#define IDT_SIZE 255
#define IDT_ADDR 0x800

#define TASK_GATE 0x5
#define INT_GATE 0xE
#define TRAP_GATE 0xF

#define USER_DPL (3 << 5)
#define KERNEL_DPL 0
#define PRESENT (1 << 7)

#define CPU_GATE     INT_GATE | PRESENT | KERNEL_DPL
#define SYSCALL_GATE TRAP_GATE | PRESENT | USER_DPL

typedef u16 GATE_ATTR;

/*
  -- IDT --
  On retrouve sur les 32 premières entrées les exceptions du processeur.
  Puis les 16 entrees reservees au materiel.
  Puis une entree pour les appels systeme (0x30)
 */

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

// Structures utilisées

// Concernant le champ type :
//  P D P L . 0 1 1 1 . 0 0 0 x . x x x x
//  P : indique si le segment est présent en mémoire (1) ou non (0)
//  Dpl : niveau de privilège du segment. (super utilisateur : 0)
//  Les x, c'est le reste du champ de 16 bits qui n'est pas pris en compte
struct idt_descriptor
{
    u16 offset0_15;
    u16 selector;
    u16 type;
    u16 offset16_31;
} __attribute__ ((packed));
typedef struct idt_descriptor IdtDescriptor;

struct idt
{
    u16 limit;
    u32 base;
} __attribute__ ((packed));
typedef struct idt Idt;

void IdtInit (void);
void PicInit (void);
void IdtInitDescriptor(u32 isrAddr, u16 type, unsigned int index);
void IdtReload();

#ifdef __IDT__
Idt gIdt;
IdtDescriptor gIdtDescriptor[IDT_SIZE];
#else
extern Idt gIdt;
extern IdtDescriptor gIdtDescriptor[];
#endif
