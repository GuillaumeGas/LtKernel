#pragma once

/*
  Driver port COM

  Source : https://wiki.osdev.org/Serial_Ports

  Concernant les adresses des ports, on pourrait aller les chercher via
  le BDA (BIOS Data Area)
 */

#include <kernel/lib/types.h>

// Port COM1
#define COM1_PORT 0x3F8
#define COM2_PORT 0x2F8

void SerialInit ();
char SerialRead (u16 port);
void SerialWrite (u16 port, char c);
