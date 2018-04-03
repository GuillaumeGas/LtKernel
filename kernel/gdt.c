#include "memory.h"
#define __GDT__
#include "gdt.h"

void gdt_loader (struct gdt * gdt_ptr);

void init_gdt_descriptor (u32 limit, u32 base, u8 access, u8 flags, struct gdt_descriptor * entry)
{
    u32 mask = 0xFF;
    entry->limit0_15 = limit & mask;
    entry->base0_15 = base & mask;
    entry->base16_23 = (base & 0xFF0000) >> 16;
    entry->access = access;
    entry->limit16_19 = (limit & 0xF0000) >> 16;
    entry->flags = flags & 0xF;
    entry->base24_31 = (base & 0xFF000000) >> 24;
}

void init_gdt ()
{    
    init_gdt_descriptor (0, 0, 0, 0, &g_gdt_desc[0]);
    init_gdt_descriptor (0xFFFFF, 0, 0x9B, 0x0D, &g_gdt_desc[1]); // code
    init_gdt_descriptor (0xFFFFF, 0, 0x93, 0x0D, &g_gdt_desc[2]); // data
    init_gdt_descriptor (0, 0, 0x97, 0x0D, &g_gdt_desc[3]);      // stack

    g_gdt.limit = GDT_SIZE * 8;
    g_gdt.base = GDT_ADDR;

    memcopy ((u8*)g_gdt_desc, (u8*)g_gdt.base, g_gdt.limit);

    gdt_loader (&g_gdt);
    
    // we can't init the esp register because the leave instructure at the end
    // of this function is going to erase esp with ebp value
}
