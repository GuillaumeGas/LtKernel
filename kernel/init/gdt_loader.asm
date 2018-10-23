[BITS 32]

global _gdtLoad
global _tssLoad

extern gTss
	
;;; Transmet l'adresse de la gdt au processeur
;;; Param : adresse de la gdt
_gdtLoad:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	lgdt [eax]

	;; on initialise les segments de donnees (0x10 car troisieme entree dans la GDT)
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;; un jump pour mettre à jour le registre segment de code
	jmp 0x08:next
next:
	leave
	ret

;;; Renseigne le selecteur correspondant au descripteur tss dans la gdt
;;;  On en profite pour le segment et pointeur de pile du noyau
;;; Param : selecteur de segment (u16)
_tssLoad:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov eax, [ebp+8]
	ltr ax

	mov eax, gTss
	mov ebx, 0x300000
	mov dword [eax+4], ebx
	mov word [eax+8], ss

	leave
	ret
