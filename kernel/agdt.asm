[BITS 32]
extern memcopy
global _asm_gdt_init

_asm_gdt_init:
	;; push 0x20
	;; push 0x0
	;; push gdt_begin	
	;; call memcopy

	mov eax, 0x0

	mov ebx, [gdt_begin]
	mov ecx, [gdt_begin+4]
	mov [eax], ebx
	mov [eax+4], ecx
	
	add eax, 8
	mov ebx, [gdt_cs]
	mov ecx, [gdt_cs+4]
	mov [eax], ebx
	mov [eax+4], ecx
	
	add eax, 8
	mov ebx, [gdt_ds]
	mov ecx, [gdt_ds+4]
	mov [eax], ebx
	mov [eax+4], ecx
	
	add eax, 8
	mov ebx, [gdt_ss]
	mov ecx, [gdt_ss+4]
	mov [eax], ebx
	mov [eax+4], ecx
	
	;; mov ecx, [ebx]
	;; mov [eax], ecx
	;; mov ecx, [ebx+4]
	;; mov [eax+4], ecx
	;; add eax, 8
	;; add eax, 8
	;; cmp eax, 0xFF
	;; jg loop
	
	;; Load Gdt
	lgdt [gdt_ptr]

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	jmp 0x08:next
next:	
	ret

;; Gdt
gdt_begin:
	db 0, 0, 0, 0, 0, 0, 0, 0
gdt_cs:				; code segment
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10011011b, 11011111b, 0x0
gdt_ds:				; data segment
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10010011b, 11011111b, 0x0
gdt_ss:				; stack segment
	db 0x00, 0x00, 0x00, 0x00, 0x00, 10010111b, 11010000b, 0x00
gdt_end:

gdt_ptr:
	dw 0x20                 ; limit : 16 bits (word)
	dd 0x0			; base : 32 bits (dword)
