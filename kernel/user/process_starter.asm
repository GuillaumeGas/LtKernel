[BITS 32]

extern g_tss
global _start_process

    ;; on d�sactive les interruptions pendant la commutation de t�che (commutation software)
    ;; on utilise le m�canisme utilis� par le proc quand il termine l'ex�cution d'une interruption
    ;; il va d�piler tous ces �l�ments pour restaurer les registres et retourner sur la pr�c�dente
    ;; t�che.
    ;; On push donc les �l�ments qui nous int�ressent, afin qu'il saute sur la t�che utilisateur
    ;; On prend garde de changer les droits des segments en ring 3 car ce sont les droits donn�s
    ;; dans les descripteurs de segments utilisateur dans la gdt
    ;; On modifie �galement l'EFLAGS afin de d�sactiver le bit NT (Nested Task) et donc la commutation hardware
    ;; ainsi que le bit IF afin d'autoriser les interrupts une fois dans la t�che utilisateur.
	
_start_process:
	push ebp
	mov ebp, esp
	
	cli

	mov eax, [ebp+8]
	mov cr3, eax

	; ss
	mov eax, [ebp+12]
	push eax
	; esp
	mov eax, [ebp+16]
	push eax
	; eflags
	mov eax, [ebp+20]
	push eax
	; cs
	mov eax, [ebp+24]
	push eax
	; eip
	mov eax, [ebp+28]
	push eax

	mov ax, 0x23
	mov ds, ax
	
	iret
