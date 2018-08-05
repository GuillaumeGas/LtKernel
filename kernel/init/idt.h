#pragma once

#define IDT_SIZE 255
#define IDT_ADDR 0x800

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

struct idt
{
    u16 limit;
    u32 base;
} __attribute__ ((packed));

void init_idt (void);
void init_pic (void);

#ifdef __IDT__
struct idt g_idt;
struct idt_descriptor g_idt_descriptor[IDT_SIZE];
#else
extern struct idt g_idt;
extern struct idt_descriptor g_idt_descriptor[];
#endif
