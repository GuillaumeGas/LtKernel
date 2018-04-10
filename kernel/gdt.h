#ifndef __DEF_GDT__
#define __DEF_GDT__

#define GDT_ADDR 0x0
#define GDT_SIZE 6

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

struct tss
{
    u16    previous_task, __previous_task_unused;
    u32    esp0;
    u16    ss0, __ss0_unused;
    u32    esp1;
    u16    ss1, __ss1_unused;
    u32    esp2;
    u16    ss2, __ss2_unused;
    u32    cr3;
    u32    eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    u16    es, __es_unused;
    u16    cs, __cs_unused;
    u16    ss, __ss_unused;
    u16    ds, __ds_unused;
    u16    fs, __fs_unused;
    u16    gs, __gs_unused;
    u16    ldt_selector, __ldt_sel_unused;
    u16    debug_flag, io_map;
} __attribute__ ((packed));

void init_gdt ();
/* void init_gdt_descriptor (u32 base, u32 limit, u8 access, u8 flags, struct gdt_descriptor * entry); */

#ifdef __GDT__
struct tss g_tss;
#else
extern struct tss g_tss;
#endif

/* #ifdef __GDT__ */
/* struct gdt g_gdt; */
/* struct gdt_descriptor g_gdt_desc[GDT_SIZE]; */
/* #else */
/* extern struct gdt g_gdt; */
/* extern struct gdt_descriptor g_gdt_desc[]; */
/* /\* extern struct tss g_tss; *\/ */
/* #endif */

#endif
