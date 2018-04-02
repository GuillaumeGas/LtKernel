[BITS 32]

extern default_isr
extern clock_isr

global _asm_default_isr
global _asm_clock_isr
	
_asm_default_isr:
	call default_isr
	mov al, 0x20
	out 0x20, al
	iret
	
_asm_clock_isr:
	call clock_isr
	mov al, 0x20 		; EOI (End Of Interrupt)
	out 0x20, al
	iret
