[BITS 32]

global _init_vmm
global _setCurrentPagesDirectory

;;; Place l'adresse du page directory passé en paramètre dans cr3
;;; Met le bit de pagination (31) à 1 dans cr0 pour activer la pagination
_init_vmm:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	mov cr3, eax

	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	
	leave
	ret

_setCurrentPagesDirectory:
	push ebp
	mov ebp, esp

	push eax
	mov eax, [ebp+8]
	mov cr3, eax
	pop eax

	leave
	ret