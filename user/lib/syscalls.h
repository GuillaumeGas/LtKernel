#pragma once

/*
Signature des appels systèmes d'i/o dont les définitions sont présentes dans
syscalls.asm
*/

void _print(const char * str);
void _scan(char * str);
void _exit();
void _listProcess();