#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>

#define __IDT__
#include "idt.h"

void _asm_divided_by_zero_isr(void);
void _asm_non_maskable_int_isr(void);
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

static void _IdtInitDescriptor(u32 isrAddr, u16 selector, GATE_ATTR type, IdtDescriptor * desc);

void IdtInit()
{
	int i = 0;

	gIdt.limit = IDT_SIZE * sizeof(struct idt_descriptor);
	gIdt.base = IDT_ADDR;

	MmSet((u8*)IDT_ADDR, 0, gIdt.limit);

	for (; i < IDT_SIZE; i++)
		IdtInitDescriptor((u32)_asm_default_isr, CPU_GATE, i);

	IdtInitDescriptor((u32)_asm_divided_by_zero_isr, CPU_GATE, 0);
	IdtInitDescriptor((u32)_asm_non_maskable_int_isr, CPU_GATE, 2);
	IdtInitDescriptor((u32)_asm_overflow_isr, CPU_GATE, 4);
	IdtInitDescriptor((u32)_asm_bound_range_exceeded_isr, CPU_GATE, 5);
	IdtInitDescriptor((u32)_asm_invalid_opcode_isr, CPU_GATE, 6);
	IdtInitDescriptor((u32)_asm_device_not_available_isr, CPU_GATE, 7);
	IdtInitDescriptor((u32)_asm_double_fault_isr, CPU_GATE, 8);
	IdtInitDescriptor((u32)_asm_invalid_tss_isr, CPU_GATE, 10);
	IdtInitDescriptor((u32)_asm_segment_not_present_isr, CPU_GATE, 11);
	IdtInitDescriptor((u32)_asm_stack_segment_fault_isr, CPU_GATE, 12);
	IdtInitDescriptor((u32)_asm_general_protection_fault_isr, CPU_GATE, 13);
	IdtInitDescriptor((u32)_asm_page_fault_isr, CPU_GATE, 14);
	IdtInitDescriptor((u32)_asm_x87_floating_point_isr, CPU_GATE, 16);
	IdtInitDescriptor((u32)_asm_alignment_check_isr, CPU_GATE, 17);
	IdtInitDescriptor((u32)_asm_machine_check_isr, CPU_GATE, 18);
	IdtInitDescriptor((u32)_asm_simd_floating_point_isr, CPU_GATE, 19);
	IdtInitDescriptor((u32)_asm_virtualization_isr, CPU_GATE, 20);
	IdtInitDescriptor((u32)_asm_security_isr, CPU_GATE, 30);

	// premier segment (code), offset 0x8 dans la gdt
	// 0x9C00, segment en mémoire, privilège super utilisateur
	// 33 ème entrée dans l'idt car les 32 premiers sont réservés au exceptions
	// type : CPU_GATE
	//        1 0 0 0 . 1 1 1 0 . 0 0 0 0 . 0 0 0 0
	IdtInitDescriptor((u32)_asm_clock_isr, CPU_GATE, 32);
	IdtInitDescriptor((u32)_asm_keyboard_isr, CPU_GATE, 33);
	IdtInitDescriptor((u32)_asm_com1_isr, CPU_GATE, 36);
	IdtInitDescriptor((u32)_asm_syscall_isr, SYSCALL_GATE, 48); // Trag Gate, appels systeme int 0x30

	IdtReload();
}

// Default idt descriptor initialization
void IdtInitDescriptor(u32 isrAddr, u16 type, unsigned int index)
{
	_IdtInitDescriptor(isrAddr, 0x8, type, &gIdtDescriptor[index]);
}

static void _IdtInitDescriptor(u32 isrAddr, u16 selector, GATE_ATTR type, IdtDescriptor * desc)
{
	desc->offset0_15 = (isrAddr & 0xFFFF);
	desc->selector = selector;
	desc->type = type << 8;
	desc->offset16_31 = (isrAddr & 0xFFFF0000) >> 16;
}

void IdtReload()
{
	MmCopy((u8*)gIdtDescriptor, (u8*)gIdt.base, gIdt.limit);

	_idtLoad(&gIdt);
}