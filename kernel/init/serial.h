#pragma once

// Source : https://wiki.osdev.org/Serial_Ports

// On utilise une adresse en dur pour le moment, mais on doit pouvoir trouver les adresses
// le BDA (Bios Data Area) : 0x0400 (COM1-COM4, chaque adresse codée sur 2 octets)
#define COM1_PORT 0x3F8

void init_serial ();
char read_serial ();
void write_serial (char c);
