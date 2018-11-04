#include "elf.h"

#include <kernel/lib/stdlib.h>

static const char ElfIdent[] = { 0x7f, 'E', 'L', 'F' };

BOOL ElfCheckIdent(ElfHeader * header)
{
	for (int i = 0; i < 4; i++)
		if (header->ident[i] != ElfIdent[i])
			return FALSE;
	return TRUE;
}