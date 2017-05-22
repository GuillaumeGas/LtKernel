[BITS 16]
[ORG 0x0]

%define DATA_ADDR 0x07C0	; data segment
%define STACK_ADDR 0x8000	; stack segment
%define STACK_PTR_OFFSET 0xF000	; stack pointer
%define KERNEL_ADDR 0x100	; kernel selector 0100:0000 -> 0x1000 (physical mem)
	
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
	mov al, [sectorsCount]
	mov ch, 0
	mov cl, [sectorNum]
	mov dh, 0
	mov dl, BOOT_DRIVE
	int 0x13
	pop es

	;; go to the kernel !
	jmp dword KERNEL_ADDR:0
	
end:
	jmp end
	
	;; Variables
	msgBoot db "Loading LtKernel...", 13, 10, 0
	sectorsCount db 1
	sectorNum db 2
	
times 510-($-$$) db 144
dw 0xAA55