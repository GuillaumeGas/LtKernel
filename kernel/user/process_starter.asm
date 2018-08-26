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
