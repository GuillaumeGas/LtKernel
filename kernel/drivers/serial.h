#pragma once

/*
  Driver port COM

  Source : https://wiki.osdev.org/Serial_Ports

  Concernant les adresses des ports, on pourrait aller les chercher via
  le BDA (BIOS Data Area)
 */

// Port COM1
#define COM1_PORT 0x3F8

void SerialInit ();
char SerialRead ();
void SerialWrite (char c);
