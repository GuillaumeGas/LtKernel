[BITS 32]
global init_idt

extern _asm_divided_by_zero_isr
extern _asm_debug_isr
extern _asm_non_maskable_int_isr
extern _asm_breakpoint_isr	
extern _asm_overflow_isr
extern _asm_bound_range_exceeded_isr
extern _asm_invalid_opcode_isr
extern _asm_device_not_available_isr
extern _asm_double_fault_isr
extern _asm_invalid_tss_isr
extern _asm_segment_not_present_isr
extern _asm_stack_segment_fault_isr
extern _asm_general_protection_fault_isr
extern _asm_page_fault_isr
extern _asm_x87_floating_point_isr
extern _asm_alignment_check_isr
extern _asm_machine_check_isr
extern _asm_simd_floating_point_isr
extern _asm_virtualization_isr
extern _asm_security_isr
extern _asm_triple_fault_isr	
	
extern _asm_default_isr
extern _asm_clock_isr	
extern _asm_keyboard_isr
extern _asm_com1_isr
	
;;; On remplie d'abord l'idt avec une isr par défaut
init_idt:
	push ebp
	mov ebp, esp
	
	mov eax, _asm_default_isr
	mov ebx, idt_empty_descriptor ; on commence par remplir le descripteur
	mov word [ebx], ax
	shr eax, 16		; obligé de faire la copie de l'adresse en deux fois car 16 bits copiés à un endroit et le reste copié à un autre
	mov [ebx+6], ax
	
	mov ebx, 0x800 		; adresse où on va copier l'idt
	mov eax, 0xB20		; adresse de fin (0xFF entrées)
	;; donc 32 pour les exceptions et une pour l'IRQ 0 (horloge)

	;; On remplie tous les descripteurs par une routine par défaut
loop:	
	mov edx, [idt_empty_descriptor]
	mov [ebx], edx
	mov edx, [idt_empty_descriptor+4]
	mov [ebx+4], edx
	add ebx, 8
	cmp eax, ebx
	jge loop

;;; Addresse de départ
	mov eax, 0x800
	
;;; Ajout des exceptions/fault du proc (34 premières entrées)
	push _asm_divided_by_zero_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_debug_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_non_maskable_int_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_breakpoint_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_overflow_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_bound_range_exceeded_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_invalid_opcode_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_device_not_available_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_double_fault_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_invalid_tss_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_segment_not_present_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_stack_segment_fault_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_general_protection_fault_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_page_fault_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_x87_floating_point_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_alignment_check_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_machine_check_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_simd_floating_point_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8
	
	push _asm_virtualization_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_security_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8

	push _asm_triple_fault_isr
	push idt_empty_descriptor
	push eax
	call add_entry
	add esp, 12
	add eax, 8
	
;;; Gestion des interruptions hardware (IRQs)

	;; push _asm_clock_isr
	;; push idt_empty_descriptor
	;; push eax
	;; call add_entry
	;; add esp, 12
	;; add eax, 8

	;; push _asm_keyboard_isr
	;; push idt_empty_descriptor
	;; push eax
	;; call add_entry
	;; add esp, 12
	;; add eax, 8
	
	;; IRQ 0 (horloge)
	push _asm_clock_isr
	push idt_empty_descriptor
	push 0x900
	call add_entry
	add esp, 12
	;; add eax, 8

	;; IRQ 1 (clavier)
	push _asm_keyboard_isr
	push idt_empty_descriptor
	push 0x908
	call add_entry
	add esp, 12
	;; add eax, 8

	;; IRQ 4 (port COM 1)
	push _asm_com1_isr
	push idt_empty_descriptor
	push 0x920
	call add_entry
	add esp, 12
	
	lidt [idt_ptr]

	leave
	ret

add_entry:
	push ebp
	mov ebp, esp

	push eax 		; because we saved the current descriptor addr in it
	
	mov ecx, [ebp + 16]
	mov ebx, [ebp + 12]
	mov word [ebx], cx
	shr ecx, 16
	mov [ebx+6], cx

	mov eax, [ebp + 8]
	mov ecx, [ebx + 4]
	mov ebx, [ebx]
	mov [eax], ebx
	mov [eax + 4], ecx

	pop eax
	
	mov esp, ebp
	pop ebp
	
	ret
	
idt_empty_descriptor:
	db 0x0, 0x0, 0x8, 0x0, 0x0, 10001110b, 0x0, 0x0
	db 0, 0, 0, 0, 0, 0, 0, 0
	
idt_ptr:
	dw 0x320
	dd 0x800
