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

	mov eax, [ebp+8]
	mov cr3, eax
	xor eax, eax

	cli
	push 0x23
	push 0x400A00
	pushf
	pop eax
	or eax, 0x200
	and eax, 0xFFFFBFFF
	push eax
	push 0x1B
	push 0x400000

	mov eax, g_tss
	mov dword [eax+4], 0x20000
	mov word [eax+8], 0x10

	mov ax, 0x23
	mov ds, ax
	
	iret
