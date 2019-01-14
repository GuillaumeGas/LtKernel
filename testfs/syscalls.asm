[BITS 32]

%define SYSCALL_INTERRUPT 0x30

;; Voir liste des num�ro de syscall dans kernel/user/syscalls.c
%define SYSCALL_PRINT 1

global _print

_print:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; on r�cup�re le param�tre : pointeur sur la cha�ne
    mov eax, SYSCALL_PRINT

    int SYSCALL_INTERRUPT

    leave
    ret