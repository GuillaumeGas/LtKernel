#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>

#define __IDT__
#include "idt.h"

void _asm_divided_by_zero_isr(void);
void _asm_debug_isr(void);
void _asm_non_maskable_int_isr(void);
void _asm_breakpoint_isr(void);
void _asm_overflow_isr(void);
void _asm_bound_range_exceeded_isr(void);
void _asm_invalid_opcode_isr(void);
void _asm_device_not_available_isr(void);
void _asm_double_fault_isr(void);
void _asm_invalid_tss_isr(void);
void _asm_segment_not_present_isr(void);
void _asm_stack_segment_fault_isr(void);
void _asm_general_protection_fault_isr(void);
void _asm_page_fault_isr(void);
void _asm_x87_floating_point_isr(void);
void _asm_alignment_check_isr(void);
void _asm_machine_check_isr(void);
void _asm_simd_floating_point_isr(void);
void _asm_virtualization_isr(void);
void _asm_security_isr(void);

void _asm_default_isr(void);
void _asm_clock_isr(void);
void _asm_keyboard_isr(void);
void _asm_com1_isr(void);

void _asm_syscall_isr(void);

void load_idt(struct idt * idt_ptr);

static void init_idt_descriptor(u32 offset_irq, u16 selector, u16 type, struct idt_descriptor * desc);

void init_idt()
{
	int i = 0;

	g_idt.limit = IDT_SIZE * sizeof(struct idt_descriptor);
	g_idt.base = IDT_ADDR;

	mmset((u8*)IDT_ADDR, 0, g_idt.limit);

	for (; i < IDT_SIZE; i++)
		init_idt_descriptor((u32)_asm_default_isr, 0x8, CPU_GATE, &g_idt_descriptor[i]);

	init_idt_descriptor((u32)_asm_divided_by_zero_isr, 0x8, CPU_GATE, &g_idt_descriptor[0]);
	init_idt_descriptor((u32)_asm_debug_isr, 0x8, CPU_GATE, &g_idt_descriptor[1]);
	init_idt_descriptor((u32)_asm_non_maskable_int_isr, 0x8, CPU_GATE, &g_idt_descriptor[2]);
	init_idt_descriptor((u32)_asm_breakpoint_isr, 0x8, CPU_GATE, &g_idt_descriptor[3]);
	init_idt_descriptor((u32)_asm_overflow_isr, 0x8, CPU_GATE, &g_idt_descriptor[4]);
	init_idt_descriptor((u32)_asm_bound_range_exceeded_isr, 0x8, CPU_GATE, &g_idt_descriptor[5]);
	init_idt_descriptor((u32)_asm_invalid_opcode_isr, 0x8, CPU_GATE, &g_idt_descriptor[6]);
	init_idt_descriptor((u32)_asm_device_not_available_isr, 0x8, CPU_GATE, &g_idt_descriptor[7]);
	init_idt_descriptor((u32)_asm_double_fault_isr, 0x8, CPU_GATE, &g_idt_descriptor[8]);
	init_idt_descriptor((u32)_asm_invalid_tss_isr, 0x8, CPU_GATE, &g_idt_descriptor[10]);
	init_idt_descriptor((u32)_asm_segment_not_present_isr, 0x8, CPU_GATE, &g_idt_descriptor[11]);
	init_idt_descriptor((u32)_asm_stack_segment_fault_isr, 0x8, CPU_GATE, &g_idt_descriptor[12]);
	init_idt_descriptor((u32)_asm_general_protection_fault_isr, 0x8, CPU_GATE, &g_idt_descriptor[13]);
	init_idt_descriptor((u32)_asm_page_fault_isr, 0x8, CPU_GATE, &g_idt_descriptor[14]);
	init_idt_descriptor((u32)_asm_x87_floating_point_isr, 0x8, CPU_GATE, &g_idt_descriptor[16]);
	init_idt_descriptor((u32)_asm_alignment_check_isr, 0x8, CPU_GATE, &g_idt_descriptor[17]);
	init_idt_descriptor((u32)_asm_machine_check_isr, 0x8, CPU_GATE, &g_idt_descriptor[18]);
	init_idt_descriptor((u32)_asm_simd_floating_point_isr, 0x8, CPU_GATE, &g_idt_descriptor[19]);
	init_idt_descriptor((u32)_asm_virtualization_isr, 0x8, CPU_GATE, &g_idt_descriptor[20]);
	init_idt_descriptor((u32)_asm_security_isr, 0x8, CPU_GATE, &g_idt_descriptor[30]);

	// premier segment (code), offset 0x8 dans la gdt
	// 0x9C00, segment en mémoire, privilège super utilisateur
	// 33 ème entrée dans l'idt car les 32 premiers sont réservés au exceptions
	// type : CPU_GATE
	//        1 0 0 0 . 1 1 1 0 . 0 0 0 0 . 0 0 0 0
	init_idt_descriptor((u32)_asm_clock_isr, 0x8, CPU_GATE, &g_idt_descriptor[32]);
	init_idt_descriptor((u32)_asm_keyboard_isr, 0x8, CPU_GATE, &g_idt_descriptor[33]);
	init_idt_descriptor((u32)_asm_com1_isr, 0x8, CPU_GATE, &g_idt_descriptor[36]);
	init_idt_descriptor((u32)_asm_syscall_isr, 0x8, SYSCALL_GATE, &g_idt_descriptor[48]); // Trag Gate, appels systeme int 0x30

	mmcopy((u8*)g_idt_descriptor, (u8*)g_idt.base, g_idt.limit);

	load_idt(&g_idt);
}

void init_idt_descriptor(u32 offset_irq, u16 selector, GATE_ATTR type, struct idt_descriptor * desc)
{
	desc->offset0_15 = (offset_irq & 0xFFFF);
	desc->selector = selector;
	desc->type = type << 8;
	desc->offset16_31 = (offset_irq & 0xFFFF0000) >> 16;
}
