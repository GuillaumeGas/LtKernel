[BITS 32]

global _idtLoad

;;; Transmet l'adresse de l'idt au processeur
;;; Param : adresse de l'idt
_idtLoad:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	lidt [eax]

	leave
	ret
