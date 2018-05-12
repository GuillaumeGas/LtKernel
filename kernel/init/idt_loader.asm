[BITS 32]

global load_idt

;;; Transmet l'adresse de l'idt au processeur
;;; Param : adresse de l'idt
load_idt:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	lidt [eax]

	leave
	ret
