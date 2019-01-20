#pragma once

/*
Signature des appels syst�mes d'i/o dont les d�finitions sont pr�sentes dans
syscalls.asm
*/

void _print(const char * str);
void _scan(char * str);
void _exit();
int _openDir(const char * dirPath, void * handle);
int _readDir(const void * handle, void * dirEntry);
void _listProcess();