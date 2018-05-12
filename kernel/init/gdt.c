#include "../lib/memory.h"
#include "../lib/stdio.h"
#include "../drivers/screen.h"
#define __GDT__
#include "gdt.h"

extern void asm_init_gdt (void);

void init_gdt ()
{
    mmset ((u8*)&g_tss, 0, (u32) (sizeof (struct tss)));
    
    g_tss.debug_flag = 0x00;
    g_tss.io_map = 0x00;
    g_tss.esp0 = 0x20000;
    g_tss.ss0 = 0x10;

    asm_init_gdt ();
}

void print_gdt_in_memory ()
{
    int i = 0;
    struct gdt_descriptor * gdt_desc = (struct gdt_descriptor*) (0 + sizeof (struct gdt_descriptor));

    kprint (">> GDT (base : %x, limit : %x)\n\n", GDT_ADDR, GDT_ADDR + (GDT_SIZE * sizeof (struct gdt_descriptor)));

    // On n'affiche pas la ligne vide
    for (; i < (GDT_SIZE - 1); i++)
	print_gdt_descriptor (&gdt_desc[i]);
}

void print_gdt_descriptor (struct gdt_descriptor * gdt_desc)
{
    u32 base = 0x0;
    u32 limit = 0x0;
    
    base = (0xFF & gdt_desc->base0_15);
    base |= ((0xF & gdt_desc->base16_23) << 16);
    base |= ((0xF & gdt_desc->base24_31) << 24);

    limit = (0xFF & gdt_desc->limit0_15);
    limit |= (0xF0 & gdt_desc->limit16_19) << 16;
    
    kprint (" base : %x, limit : %x\n", base, limit);
}
