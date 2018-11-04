#pragma once

#include <kernel/lib/types.h>

struct ElfHeader
{
	unsigned char ident[16];      /* ELF identification */
	u16 type;             /* 2 (exec file) */
	u16 machine;          /* 3 (intel architecture) */
	u32 version;          /* 1 */
	u32 entry;            /* starting point */
	u32 phoff;            /* program header table offset */
	u32 shoff;            /* section header table offset */
	u32 flags;            /* various flags */
	u16 ehsize;           /* ELF header (this) size */

	u16 phentsize;        /* program header table entry size */
	u16 phnum;            /* number of entries */

	u16 shentsize;        /* section header table entry size */
	u16 shnum;            /* number of entries */

	u16 shstrndx;         /* index of the section name string table */
} typedef ElfHeader;

BOOL ElfCheckIdent(ElfHeader * header);