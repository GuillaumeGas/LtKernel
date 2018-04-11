[BITS 32]

extern g_tss
global task_switch

task_switch:
	push 0x23
	push 0x30000
	pushf
	pop eax
	or eax, 0x200
	and eax, 0xFFFFBFFF
	push eax
	push 0x1B
	push 0x30000
	mov eax, g_tss
	mov dword [eax+4], 0x20000
	mov ax, 0x23
	mov ds, ax
	iret
	
	leave
	ret
