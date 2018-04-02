[BITS 32]

extern memcopy
global init_gdt

init_gdt:
	mov eax, 0		; adresse de destination
	mov ebx, gdt_begin	; adresse de la gdt locale
	mov ecx, 0x20		; taille en octets

	;; Stocke a l'adresse 0x0 la gdt
loop:	
	mov edx, [ebx]
	mov [eax], edx
	mov edx, [ebx+4]
	mov [eax+4], edx
	add ebx, 8
	add eax, 8
	cmp ecx, eax
	jge loop
			
	;; Load Gdt
	lgdt [gdt_ptr]

	;; on initialise les segments de donnees (0x10 car troisieme entree dans la GDT)
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;; un jump pour mettre à jour le registre segment de code
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
