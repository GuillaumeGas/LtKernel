[BITS 32]

extern default_isr
extern clock_isr
extern keyboard_isr	

global _asm_default_isr
global _asm_clock_isr
global _asm_keyboard_isr

%macro  SAVE_REGS 0
        pushad
        push ds
        push es
        push fs
        push gs
        push ebx
        mov bx,0x10
        mov ds,bx
        pop ebx
%endmacro

%macro  RESTORE_REGS 0
        pop gs
        pop fs
        pop es
        pop ds
        popad
%endmacro
	
_asm_default_isr:
	SAVE_REGS
	call default_isr
	mov al, 0x20
	out 0x20, al
	RESTORE_REGS
	iret
	
_asm_clock_isr:
	SAVE_REGS
	call clock_isr
	mov al, 0x20 		; EOI (End Of Interrupt)
	out 0x20, al
	RESTORE_REGS
	iret

_asm_keyboard_isr:
	SAVE_REGS
	call keyboard_isr
	mov al, 0x20
	out 0x20, al
	RESTORE_REGS
	iret
