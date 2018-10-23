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

void _idtLoad(struct idt * idt_ptr);

static void IdtInitDescriptor(u32 offset_irq, u16 selector, u16 type, IdtDescriptor * desc);

void IdtInit()
{
	int i = 0;

	gIdt.limit = IDT_SIZE * sizeof(struct idt_descriptor);
	gIdt.base = IDT_ADDR;

	MmSet((u8*)IDT_ADDR, 0, gIdt.limit);

	for (; i < IDT_SIZE; i++)
		IdtInitDescriptor((u32)_asm_default_isr, 0x8, CPU_GATE, &gIdtDescriptor[i]);

	IdtInitDescriptor((u32)_asm_divided_by_zero_isr, 0x8, CPU_GATE, &gIdtDescriptor[0]);
	IdtInitDescriptor((u32)_asm_debug_isr, 0x8, CPU_GATE, &gIdtDescriptor[1]);
	IdtInitDescriptor((u32)_asm_non_maskable_int_isr, 0x8, CPU_GATE, &gIdtDescriptor[2]);
	IdtInitDescriptor((u32)_asm_breakpoint_isr, 0x8, CPU_GATE, &gIdtDescriptor[3]);
	IdtInitDescriptor((u32)_asm_overflow_isr, 0x8, CPU_GATE, &gIdtDescriptor[4]);
	IdtInitDescriptor((u32)_asm_bound_range_exceeded_isr, 0x8, CPU_GATE, &gIdtDescriptor[5]);
	IdtInitDescriptor((u32)_asm_invalid_opcode_isr, 0x8, CPU_GATE, &gIdtDescriptor[6]);
	IdtInitDescriptor((u32)_asm_device_not_available_isr, 0x8, CPU_GATE, &gIdtDescriptor[7]);
	IdtInitDescriptor((u32)_asm_double_fault_isr, 0x8, CPU_GATE, &gIdtDescriptor[8]);
	IdtInitDescriptor((u32)_asm_invalid_tss_isr, 0x8, CPU_GATE, &gIdtDescriptor[10]);
	IdtInitDescriptor((u32)_asm_segment_not_present_isr, 0x8, CPU_GATE, &gIdtDescriptor[11]);
	IdtInitDescriptor((u32)_asm_stack_segment_fault_isr, 0x8, CPU_GATE, &gIdtDescriptor[12]);
	IdtInitDescriptor((u32)_asm_general_protection_fault_isr, 0x8, CPU_GATE, &gIdtDescriptor[13]);
	IdtInitDescriptor((u32)_asm_page_fault_isr, 0x8, CPU_GATE, &gIdtDescriptor[14]);
	IdtInitDescriptor((u32)_asm_x87_floating_point_isr, 0x8, CPU_GATE, &gIdtDescriptor[16]);
	IdtInitDescriptor((u32)_asm_alignment_check_isr, 0x8, CPU_GATE, &gIdtDescriptor[17]);
	IdtInitDescriptor((u32)_asm_machine_check_isr, 0x8, CPU_GATE, &gIdtDescriptor[18]);
	IdtInitDescriptor((u32)_asm_simd_floating_point_isr, 0x8, CPU_GATE, &gIdtDescriptor[19]);
	IdtInitDescriptor((u32)_asm_virtualization_isr, 0x8, CPU_GATE, &gIdtDescriptor[20]);
	IdtInitDescriptor((u32)_asm_security_isr, 0x8, CPU_GATE, &gIdtDescriptor[30]);

	// premier segment (code), offset 0x8 dans la gdt
	// 0x9C00, segment en mémoire, privilège super utilisateur
	// 33 ème entrée dans l'idt car les 32 premiers sont réservés au exceptions
	// type : CPU_GATE
	//        1 0 0 0 . 1 1 1 0 . 0 0 0 0 . 0 0 0 0
	IdtInitDescriptor((u32)_asm_clock_isr, 0x8, CPU_GATE, &gIdtDescriptor[32]);
	IdtInitDescriptor((u32)_asm_keyboard_isr, 0x8, CPU_GATE, &gIdtDescriptor[33]);
	IdtInitDescriptor((u32)_asm_com1_isr, 0x8, CPU_GATE, &gIdtDescriptor[36]);
	IdtInitDescriptor((u32)_asm_syscall_isr, 0x8, SYSCALL_GATE, &gIdtDescriptor[48]); // Trag Gate, appels systeme int 0x30

	MmCopy((u8*)gIdtDescriptor, (u8*)gIdt.base, gIdt.limit);

	_idtLoad(&gIdt);
}

void IdtInitDescriptor(u32 offset_irq, u16 selector, GATE_ATTR type, IdtDescriptor * desc)
{
	desc->offset0_15 = (offset_irq & 0xFFFF);
	desc->selector = selector;
	desc->type = type << 8;
	desc->offset16_31 = (offset_irq & 0xFFFF0000) >> 16;
}
