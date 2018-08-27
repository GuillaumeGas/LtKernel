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
	cli

	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	mov cr3, eax

	; ss
	push dword [ebp+12]
	; esp
	push dword [ebp+16]
	; eflags
	push dword [ebp+20]
	; cs
	push dword [ebp+24]
	; eip
	push dword [ebp+28]

	push dword [ebp+32]
	push dword [ebp+36]
	push dword [ebp+40]
	push dword [ebp+44]
	push dword [ebp+48]
	push dword [ebp+52]
	push dword [ebp+56]
	push dword [ebp+60]
	push dword [ebp+64]
	push dword [ebp+68]
	push dword [ebp+72]

	pop gs
	pop fs
	pop es
	pop ds
	pop edi
	pop esi
	pop ebp
	pop ebx
	pop edx
	pop ecx
	pop eax

	mov ax, 0x23
	mov ds, ax
	
	iret
