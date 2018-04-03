#ifndef __DEF_GDT__
#define __DEF_GDT__

#define GDT_ADDR 0x0
#define GDT_SIZE 4

// Structures utilisées

struct gdt_descriptor
{
    u16 limit0_15;
    u16 base0_15;
    u8 base16_23;
    u8 access;
    u8 limit16_19 : 4; // bit fields : allows us to use an 8bits field
    u8 flags : 4;      //  to store two differents informations
    u8 base24_31;
} __attribute__ ((packed)); // means we don't want data alignment

struct gdt
{
    u16 limit; // gdt size
    u32 base;  // gdt location
} __attribute__ ((packed));

void init_gdt ();
void init_gdt_descriptor (u32 base, u32 limit, u8 access, u8 flags, struct gdt_descriptor * entry);

#ifdef __GDT__
struct gdt g_gdt;
struct gdt_descriptor g_gdt_desc[4];
#endif

#endif
