#include "elf.h"

#include <kernel/lib/stdlib.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)

#include <kernel/debug/debug.h>

static const char ElfIdent[] = { 0x7f, 'E', 'L', 'F' };

BOOL ElfCheckIdent(ElfHeader * header)
{
	for (int i = 0; i < 4; i++)
		if (header->ident[i] != ElfIdent[i])
			return FALSE;
	return TRUE;
}

KeStatus ElfInit(Ext2File * extFile, ElfFile * file)
{
    KeStatus status = STATUS_FAILURE;

    if (extFile == NULL)
    {
        KLOG(LOG_ERROR, "Invalid extFile parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (file == NULL)
    {
        KLOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    file->header = (ElfHeader *)extFile;

    if (file->header->phoff == 0)
    {
        KLOG(LOG_ERROR, "Elf program header table pointer is NULL");
        status = STATUS_UNEXPECTED;
    }

    file->prgHeaderTable = (ElfProgramHeaderTable *)(file->header + file->header->phoff);

    status = STATUS_SUCCESS;

    return status;
}

void ElfFree(const ElfFile * elf)
{
    if (elf == NULL)
    {
        KLOG(LOG_ERROR, "Invalid elf parameter");
        return;
    }

    kfree(elf->header);
}