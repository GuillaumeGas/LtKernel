#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/types.h>
#include <kernel/drivers/screen.h>

#define __GDT__
#include "gdt.h"

void load_gdt (const Gdt * gdt_ptr);
void load_tss (const u16 tss_selector);

static void init_gdt_descriptor (u32 base, u32 limit, u8 access, u8 flags, GdtDescriptor * desc);
static void init_tss ();

void init_gdt ()
{
    mmset ((u8*)GDT_ADDR, 0, (u32) (GDT_SIZE * sizeof (GdtDescriptor)));
    mmset ((u8*)&g_tss, 0, (u32) (sizeof (Tss)));

    init_tss ();
    
    init_gdt_descriptor (0, 0, 0, 0, &g_gdt_descriptor[0]);
    init_gdt_descriptor (0, 0xFFFFF, 0x9B, 0x0D, &g_gdt_descriptor[1]);
    init_gdt_descriptor (0, 0xFFFFF, 0x93, 0x0D, &g_gdt_descriptor[2]);
    init_gdt_descriptor (0, 0xFFFFF, 0xFF, 0x0F, &g_gdt_descriptor[3]);
    init_gdt_descriptor (0, 0xFFFFF, 0xF3, 0x0F, &g_gdt_descriptor[4]);
    init_gdt_descriptor ((u32)&g_tss, (u32) (sizeof (Tss)), 0xE9, 0, &g_gdt_descriptor[5]);

    g_gdt.limit = GDT_SIZE * sizeof (GdtDescriptor);
    g_gdt.base = GDT_ADDR;

    mmcopy ((u8*) g_gdt_descriptor, (u8*) g_gdt.base, g_gdt.limit);

    load_gdt (&g_gdt);

    load_tss (TSS_SEG_SELECTOR);
}

static void init_gdt_descriptor (u32 base, u32 limit, u8 access, u8 flags, GdtDescriptor * desc)
{
    desc->limit0_15 = (limit & 0xFFFF);
    desc->base0_15 = (base & 0xFFFF);
    desc->base16_23 = (base >> 16) & 0xFF;
    desc->access = access;
    desc->limit16_19 = (limit >> 16) & 0xF;
    desc->flags = flags & 0xF;
    desc->base24_31 = (base >> 24) & 0xFF;
}

static void init_tss ()
{
    g_tss.debug_flag = 0x00;
    g_tss.io_map = 0x00;
}

GdtDescriptor * get_gdt_descriptor(u8 selector)
{
	if (selector <= ((GDT_SIZE - 1) * sizeof(GdtDescriptor)))
		return (GdtDescriptor *) selector;
	return (GdtDescriptor *) 0;
}

u32 get_base_addr (GdtDescriptor * desc)
{
    u32 addr = desc->base0_15;
    addr |= (desc->base16_23 << 16);
    addr |= (desc->base24_31 << 24);
    return addr;
}

void print_gdt ()
{
    int i = 1;
    GdtDescriptor * gdt_desc = (GdtDescriptor *)0;

    kprint (">> GDT (base : %x, limit : %x)\n\n", GDT_ADDR, GDT_ADDR + (GDT_SIZE * sizeof (GdtDescriptor)));

    // On n'affiche pas la ligne vide
    for (; i < GDT_SIZE; i++)
	print_gdt_descriptor (&gdt_desc[i]);
}

void print_gdt_descriptor (GdtDescriptor * gdt_desc)
{
    u32 base = 0x0;
    u32 limit = 0x0;

    base |= gdt_desc->base24_31 << 24;
    base |= gdt_desc->base16_23 << 16;
    base |= gdt_desc->base0_15;

    limit |= gdt_desc->limit16_19 << 16;
    limit |= gdt_desc->limit0_15;
    
    kprint (" [%x] base : %x, limit : %x\n", gdt_desc, base, (base + limit));
}

void print_tss ()
{
    kprint (">> TSS (base : %x, limit : %x)\n\n", (int)&g_tss, (int)(&g_tss) + sizeof (Tss));

    kprint (" esp0 : %x | ss0 : %x\n", g_tss.esp0, g_tss.ss0);
    kprint (" esp1 : %x | ss1 : %x\n", g_tss.esp1, g_tss.ss1);
    kprint (" esp2 : %x | ss2 : %x\n", g_tss.esp2, g_tss.ss2);
    kprint (" cr3 : %x (%b)\n", g_tss.cr3, g_tss.cr3);
    kprint (" eip (%x), eflags (%x), eax (%x), ecx (%x), edx (%x), esp (%x), ebp (%x), esi (%x), edi (%x)\n",
	    g_tss.eip, g_tss.eflags, g_tss.eax, g_tss.ecx, g_tss.edx, g_tss.ebx, g_tss.esp, g_tss.ebp, g_tss.esi, g_tss.edi);
    kprint (" es (%x), cs (%x), ss (%x), ds (%x), fs (%x), gs (%x)\n",
	    g_tss.es, g_tss.cs, g_tss.ss, g_tss.ds, g_tss.fs, g_tss.gs);
}
