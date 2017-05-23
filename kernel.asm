[BITS 16]
[ORG 0x0]

%define DATA_ADDR 0x1000	; data segment
%define STACK_ADDR 0x8000 	; stack segment, same as bootsect
%define STACK_PTR_OFFSET 0xF000	; stack pointer, same as bootsect

	jmp start

%include "utils.asm"

start:
	;; Init segments
	mov ax, DATA_ADDR
	mov ds, ax
	mov es, ax
	mov ax, STACK_ADDR
	mov ss, ax
	mov sp, STACK_PTR_OFFSET

	mov si, helloMsg
	call print

end:
	jmp end

	;; Variables
	helloMsg db "Kernel loaded !", 13, 10, 0