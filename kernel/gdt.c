#include "memory.h"
#define __GDT__
#include "gdt.h"

void load_gdt (struct gdt * gdt_ptr);

void init_gdt_descriptor (u32 base, u32 limit, u8 access, u8 flags, struct gdt_descriptor * entry)
{
    entry->limit0_15 = limit & 0xFFFF;
    entry->base0_15 = base & 0xFFFF;
    entry->base16_23 = (base & 0xFF0000) >> 16;
    entry->access = access;
    entry->limit16_19 = (limit & 0xF00) >> 16;
    entry->flags = flags & 0xF;
    entry->base24_31 = (base & 0xFF000000) >> 24;
}

void init_gdt ()
{    
    init_gdt_descriptor (0, 0, 0, 0, &g_gdt_desc[0]);

    // segments du noyau, il a accès à toute la mémoire
    init_gdt_descriptor (0, 0xFFFFF, 0x9B, 0x0C, &g_gdt_desc[1]); // code
    init_gdt_descriptor (0, 0xFFFFF, 0x93, 0x0C, &g_gdt_desc[2]); // data

    // segments utilisateur
    init_gdt_descriptor (0, 0xFFFF, 0xFF, 0x0C, &g_gdt_desc[3]); // ucode
    init_gdt_descriptor (0, 0xFFFF, 0xF3, 0x0C, &g_gdt_desc[4]); // udata

    // tss
    init_gdt_descriptor ((u32)&g_tss, (u32) sizeof (struct tss), 0xE9, 0x0, &g_gdt_desc[5]);

    g_tss.debug_flag = 0x00;
    g_tss.io_map = 0x00;
    g_tss.esp0 = 0x20000;
    g_tss.ss0 = 0x10;
    
    g_gdt.limit = GDT_SIZE * sizeof (struct gdt_descriptor);
    g_gdt.base = GDT_ADDR;

    memcopy ((u8*)g_gdt_desc, (u8*)g_gdt.base, g_gdt.limit);

    load_gdt (&g_gdt);

    asm ("movw $0x28, %ax \n \
          ltr %ax");
    
    // we can't init the esp register because the leave instructure at the end
    // of this function is going to erase esp with ebp value
}
