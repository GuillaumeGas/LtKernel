[BITS 32]
global _asm_idt_init

extern _asm_default_irq
extern _asm_clock_irq	

idt_ptr:
	dw 0x108
	dd 0x800
	
_asm_idt_init:
	mov eax, _asm_default_irq
	mov ebx, idt_desc_default ; on commence par remplir le descripteur
	mov word [ebx], ax
	shr eax, 16
	mov [ebx+6], ax
	
	mov ebx, 0x800 		; start ptr
	mov ecx, 0
	mov eax, 0x10A
	;; add ecx, 0x7F8		; end ptr

loop:	
	mov edx, [idt_desc_default]
	mov [ebx], edx
	mov edx, [idt_desc_default+4]
	mov [ebx+4], edx
	add ebx, 8
	add ecx, 1
	cmp eax, ecx
	jge loop
	
init_clock:
	mov eax, _asm_clock_irq
	mov ebx, idt_desc_clock	; on commence par remplir le descripteur
	mov word [ebx], ax
	mov [ebx+6], eax

	mov eax, 0x900
	;; add eax, 0x100
	mov ebx, [idt_desc_clock]
	mov ecx, [idt_desc_clock+4]
	mov [eax], ebx
	mov [ebx+4], ecx

	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	
	lidt [idt_ptr]
	ret
	
idt_desc_default:
	db 0x0, 0x0, 0x8, 0x0, 0x0, 10001110b, 0x0, 0x0
	db 0, 0, 0, 0, 0, 0, 0, 0
	
idt_desc_clock:
	db 0x0, 0x0, 0x8, 0x0, 0x0, 10001110b, 0x0, 0x0
	db 0, 0, 0, 0, 0, 0, 0, 0	

;; Gdt
;; gdt_begin:
;; 	db 0, 0, 0, 0, 0, 0, 0, 0
;; gdt_cs:				; code segment
;; 	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10011011b, 11011111b, 0x0
;; gdt_ds:				; data segment
;; 	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10010011b, 11011111b, 0x0
;; gdt_ss:				; stack segment
;; 	db 0x00, 0x00, 0xFF, 0xFF, 0xFF, 10010111b, 11010000b, 0xFF
;; gdt_end:

;; gdt_ptr:
;; 	dw 0x20                 ; limit : 16 bits (word)
;; 	dd 0x0			; base : 32 bits (dword)
