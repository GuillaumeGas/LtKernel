[BITS 32]

%define SYSCALL_INTERRUPT 0x30

;; Voir liste des numéro de syscall dans kernel/user/syscalls.c
%define SYSCALL_PRINT         0x1
%define SYSCALL_SCAN          0x2
%define SYSCALL_EXIT          0x5
%define SYSCALL_LIST_PROCESS  0xA

global _print
global _scan
global _exit
global _listProcess

_print:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; on récupère le paramètre : pointeur sur la chaîne
    mov eax, SYSCALL_PRINT

    int SYSCALL_INTERRUPT

    leave
    ret

_scan:
	push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; on récupère le paramètre : pointeur sur la chaîne
    mov eax, SYSCALL_SCAN

    int SYSCALL_INTERRUPT

    leave
    ret

_exit:
	push ebp
    mov ebp, esp

    mov eax, SYSCALL_EXIT

    int SYSCALL_INTERRUPT

    leave
    ret

_listProcess:
	push ebp
    mov ebp, esp

    mov eax, SYSCALL_LIST_PROCESS

    int SYSCALL_INTERRUPT

    leave
    ret