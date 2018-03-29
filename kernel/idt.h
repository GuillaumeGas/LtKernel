#ifndef __DEF_IDT__
#define __DEF_IDT__

#include "types.h"

struct idt_descriptor
{
    u16 offset0_15;
    u16 selector;
    u16 type;
    u16 offset31_16;
} __attribute__ ((packed));

void init_idt ();
void init_pic ();

#endif
