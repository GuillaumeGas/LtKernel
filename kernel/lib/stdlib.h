#pragma once

#include <kernel/lib/types.h>
#include <kernel/lib/status.h>

#define FlagOn(a, b) ((a & b) != 0)
#define NULL 0
#define TRUE 1
#define FALSE 0

#define UNREFERENCED_PARAMETER(param) param=param

void Pause ();
void Halt();

void MmCopy(u8 * src, u8 * dst, unsigned int size);
void MmSet(u8 * src, u8 byte, unsigned int size);

void StrCpy(const char * src, char * dst);
unsigned long StrLen(const char * str);