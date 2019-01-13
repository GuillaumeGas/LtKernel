[BITS 32]

%define KERNEL_MODE 0
%define USER_MODE 1

extern g_tss
global _start_process

; Commentaires � revoir !

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
	DISABLE_IRQ

	push ebp
	mov esi, esp

	mov eax, [esi+8]
	mov cr3, eax

	mov eax, [esi+76]
	cmp eax, KERNEL_MODE
	jne user_mode

kernel_mode:
	; eflags
	push dword [esi+20]
	; cs
	push dword [esi+24]
	; eip
	push dword [esi+28]
	jmp next

user_mode:
	; ss
	push dword [esi+12]
	; esp
	push dword [esi+16]
	; eflags
	push dword [esi+20]
	; cs
	push dword [esi+24]
	; eip
	push dword [esi+28]
	
next:
	push dword [esi+32]
	push dword [esi+36]
	push dword [esi+40]
	push dword [esi+44]
	push dword [esi+48]
	push dword [esi+52]
	push dword [esi+56]
	push dword [esi+60]
	push dword [esi+64]
	push dword [esi+68]
	push dword [esi+72]

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
	
	; enleve le mask du PIC
	mov al, 0x20
	out 0x20, al

	int 3

	iret