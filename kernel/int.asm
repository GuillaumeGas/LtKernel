[BITS 32]
extern isr_default_int
extern isr_clock_int

global _asm_default_irq
global _asm_clock_irq
	
_asm_default_irq:
	call isr_default_int
	mov al, 0x20
	out 0x20, al
	iret
	
_asm_clock_irq:
	call isr_clock_int
	mov al, 0x20 		; EOI (End Of Interrupt)
	out 0x20, al
	iret
