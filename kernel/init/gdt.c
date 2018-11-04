#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/types.h>
#include <kernel/drivers/screen.h>

#define __GDT__
#include "gdt.h"

/*

	TODO : 
	 - décrire quel segment sert à quoi
	 - ou utiliser des macro parce qu'on comprend rien à ces valeurs

*/

void _gdtLoad (const Gdt * gdtAddr);
void _tssLoad (const u16 tssSelector);

static void GdtInitDescriptor (u32 base, u32 limit, u8 access, u8 flags, GdtDescriptor * desc);
static void TssInit ();

void GdtInit ()
{
    MmSet ((u8*)GDT_ADDR, 0, (u32) (GDT_SIZE * sizeof (GdtDescriptor)));
    MmSet ((u8*)&gTss, 0, (u32) (sizeof (Tss)));

    TssInit ();
    
    GdtInitDescriptor (0, 0, 0, 0, &gGdtDescriptor[0]);
    GdtInitDescriptor (0, 0xFFFFF, 0x9B, 0x0D, &gGdtDescriptor[1]);
    GdtInitDescriptor (0, 0xFFFFF, 0x93, 0x0D, &gGdtDescriptor[2]);
    GdtInitDescriptor (0, 0xFFFFF, 0xFF, 0x0F, &gGdtDescriptor[3]);
    GdtInitDescriptor (0, 0xFFFFF, 0xF3, 0x0F, &gGdtDescriptor[4]);
    GdtInitDescriptor ((u32)&gTss, (u32) (sizeof (Tss)), 0xE9, 0, &gGdtDescriptor[5]);

    gGdt.limit = GDT_SIZE * sizeof (GdtDescriptor);
    gGdt.base = GDT_ADDR;

    MmCopy ((u8*) gGdtDescriptor, (u8*) gGdt.base, gGdt.limit);

	_gdtLoad(&gGdt);

	_tssLoad(TSS_SEG_SELECTOR);
}

static void GdtInitDescriptor (u32 base, u32 limit, u8 access, u8 flags, GdtDescriptor * desc)
{
    desc->limit0_15 = (limit & 0xFFFF);
    desc->base0_15 = (base & 0xFFFF);
    desc->base16_23 = (base >> 16) & 0xFF;
    desc->access = access;
    desc->limit16_19 = (limit >> 16) & 0xF;
    desc->flags = flags & 0xF;
    desc->base24_31 = (base >> 24) & 0xFF;
}

static void TssInit ()
{
    gTss.debug_flag = 0x00;
    gTss.io_map = 0x00;
}

GdtDescriptor * GdtGetDescriptor(u8 selector)
{
	if (selector <= ((GDT_SIZE - 1) * sizeof(GdtDescriptor)))
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
		return (GdtDescriptor *)selector;
#pragma GCC diagnostic pop
	}
	return (GdtDescriptor *) 0;
}

u32 GdtGetBaseAddrFromDescriptor (GdtDescriptor * desc)
{
    u32 addr = desc->base0_15;
    addr |= (desc->base16_23 << 16);
    addr |= (desc->base24_31 << 24);
    return addr;
}

void GdtPrint ()
{
    int i = 1;
    GdtDescriptor * gdt_desc = (GdtDescriptor *)0;

    kprint (">> GDT (base : %x, limit : %x)\n\n", GDT_ADDR, GDT_ADDR + (GDT_SIZE * sizeof (GdtDescriptor)));

    // On n'affiche pas la ligne vide
    for (; i < GDT_SIZE; i++)
		GdtPrintDescriptor (&gdt_desc[i]);
}

void GdtPrintDescriptor (GdtDescriptor * gdt_desc)
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

void TssPrint ()
{
    kprint (">> TSS (base : %x, limit : %x)\n\n", (int)&gTss, (int)(&gTss) + sizeof (Tss));

    kprint (" esp0 : %x | ss0 : %x\n", gTss.esp0, gTss.ss0);
    kprint (" esp1 : %x | ss1 : %x\n", gTss.esp1, gTss.ss1);
    kprint (" esp2 : %x | ss2 : %x\n", gTss.esp2, gTss.ss2);
    kprint (" cr3 : %x (%b)\n", gTss.cr3, gTss.cr3);
    kprint (" eip (%x), eflags (%x), eax (%x), ecx (%x), edx (%x), esp (%x), ebp (%x), esi (%x), edi (%x)\n",
	    gTss.eip, gTss.eflags, gTss.eax, gTss.ecx, gTss.edx, gTss.ebx, gTss.esp, gTss.ebp, gTss.esi, gTss.edi);
    kprint (" es (%x), cs (%x), ss (%x), ds (%x), fs (%x), gs (%x)\n",
	    gTss.es, gTss.cs, gTss.ss, gTss.ds, gTss.fs, gTss.gs);
}
