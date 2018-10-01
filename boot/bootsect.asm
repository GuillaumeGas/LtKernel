[BITS 16]
[ORG 0x0]

%define DATA_ADDR 0x07C0	; data segment
%define STACK_ADDR 0x8000	; stack segment
%define STACK_PTR_OFFSET 0xF000	; stack pointer
%define KERNEL_ADDR 0x100 	; kernel selector 010000:0000 -> 0x100000 (physical mem)
%define KSIZE 30
	
%define BOOT_DRIVE 0x0 		; drive (for int 0x13)
	
	jmp start
	
%include "utils.asm"

start:
	;; Segments init
	mov ax, DATA_ADDR
	mov ds, ax
	mov es, ax
	mov ax, STACK_ADDR
	mov ss, ax
	mov sp, STACK_PTR_OFFSET

	mov si, msgBoot
	call print

	;; Load kernel
	mov ah, 0
	mov dl, BOOT_DRIVE
	int 0x13

	push es
	mov ax, KERNEL_ADDR
	mov es, ax
	mov bx, 0
	
	mov ah, 0x02		; we read sectors from drive
	mov al, KSIZE
	mov ch, 0
	mov cl, [sectorNum]
	mov dh, 0
	mov dl, BOOT_DRIVE
	int 0x13
	pop es

	mov si, okMsg
	call print
	mov si, loadGdtMsg
	call print

	;; Loading gdt
	;; Gdt size
	mov ax, gdt_end
	mov bx, gdt_begin
	sub ax, bx
	mov word [gdt_ptr], ax

	;; Gdt physical address
	xor eax, eax
	xor ebx, ebx
	mov ax, ds
	shl eax, 4
	mov bx, gdt_begin
	add eax, ebx
	mov dword [gdt_ptr+2], eax

	;; Clear interrupt flag
	cli

	;; Load Gdt
	lgdt [gdt_ptr]

	mov si, okMsg
	call print
	
	;; Enable protected mode
	;; xor eax, eax
	;; mov eax, 1
	mov eax, cr0
	or ax, 1
	mov cr0, eax

	;; because of the documentation...
	jmp next
next:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, 0x9F000

	jmp dword 0x8:0x1000
	
	;; Variables
	msgBoot db "Loading LtKernel...", 0
	loadGdtMsg db "Loading GDT...", 0
	okMsg db "OK", 13, 10, 0
	sectorsCount db 30
	sectorNum db 2

	;; Gdt
gdt_begin:
	db 0, 0, 0, 0, 0, 0, 0, 0
gdt_cs:				; code segment
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10011011b, 11011111b, 0x0
gdt_ds:				; data segment
	db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10010011b, 11011111b, 0x0
gdt_end:

gdt_ptr:
	dw 0x0000               ; limit : 16 bits (word)
	dd 0			; base : 32 bits (dword)
	
times 510-($-$$) db 144
dw 0xAA55
