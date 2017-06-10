#include "gdt.h"

struct gdt_descriptor _gdt_desc[4];
struct gdt _gdt;

void init_gdt_descriptor (u32 limit, u32 base, u8 access, u8 flags, struct gdt_descriptor * entry) {
    u32 mask = 0xFFFF;
    entry->limit0_15 = limit & mask;
    entry->base0_15 = base & mask;
    entry->base16_23 = (base & 0xFF0000) >> 16;
    entry->access = access;
    entry->limit16_19 = (limit & 0xF0000) >> 16;
    entry->flags = flags & 0xF;
    entry->base24_31 = (base & 0xFF000000) >> 24;
}

void init_gdt () {
    init_gdt_descriptor (0, 0, 0, 0, &_gdt_desc);
    init_gdt_descriptor (0xFFFFF, 0, 0x9B, 0x0D, &_gdt_desc[1]); // code
    init_gdt_descriptor (0xFFFFF, 0, 0x93, 0x0D, &_gdt_desc[2]); // data
    init_gdt_descriptor (0, 0, 0x97, 0x0D, &_gdt_desc[3]);      // stack

    _gdt.limit = GDT_SIZE * 8;
    _gdt.base = GDT_ADDR;

    memcopy ((u8*)&_gdt_desc, (u8*)_gdt.base, _gdt.limit);
    /* int i = 0; */
    /* u8 * src = (u8*)&_gdt_desc; */
    /* u8 * dst = (u8*)_gdt.base; */
    /* for (; i < _gdt.limit; i++) */
    /* 	*(src+i) = *(src+i); */

    /* chargement du registre GDTR */
    asm("lgdtl (_gdt)");

    /* initialisation des segments */
    asm (" movw $0x10, %ax \n");
    asm (" movw %ax, %ds \n");
    asm (" movw %ax, %es \n");
    asm (" movw %ax, %fs \n");
    asm (" movw %ax, %gs \n");
    asm (" ljmp $0x08, $next \n");
    asm (" next: \n");

    // we can't init the esp register because the leave instructure at the end
    // of this function is going to erase esp with ebp value
}
