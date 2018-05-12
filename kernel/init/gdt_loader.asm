[BITS 32]

global load_gdt

;;; Transmet l'adresse de la gdt au processeur
;;; Param : adresse de la gdt
load_gdt:
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
