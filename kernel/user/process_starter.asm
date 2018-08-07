[BITS 32]

extern g_tss
global _start_process

    ;; on désactive les interruptions pendant la commutation de tâche (commutation software)
    ;; on utilise le mécanisme utilisé par le proc quand il termine l'exécution d'une interruption
    ;; il va dépiler tous ces éléments pour restaurer les registres et retourner sur la précédente
    ;; tâche.
    ;; On push donc les éléments qui nous intéressent, afin qu'il saute sur la tâche utilisateur
    ;; On prend garde de changer les droits des segments en ring 3 car ce sont les droits donnés
    ;; dans les descripteurs de segments utilisateur dans la gdt
    ;; On modifie également l'EFLAGS afin de désactiver le bit NT (Nested Task) et donc la commutation hardware
    ;; ainsi que le bit IF afin d'autoriser les interrupts une fois dans la tâche utilisateur.
	
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
