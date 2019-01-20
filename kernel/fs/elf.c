#include "elf.h"
#include "file.h"

#include <kernel/lib/stdlib.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/lib/stdio.h>

#include <kernel/init/vmm.h>
#include <kernel/user/process_manager.h>

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

KeStatus ElfInit(File * file, ElfFile * elf)
{
    KeStatus status = STATUS_FAILURE;

    if (file == NULL)
    {
        KLOG(LOG_ERROR, "Invalid extFile parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (elf == NULL)
    {
        KLOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    elf->header = (ElfHeader *)file->content;

    if (elf->header->phoff == 0)
    {
        KLOG(LOG_ERROR, "Elf program header table pointer is NULL");
        status = STATUS_UNEXPECTED;
    }

    elf->prgHeaderTable = (ElfProgramHeaderTable *)((u8 *)elf->header + elf->header->phoff);

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

void ElfHeaderDump(ElfFile * elf)
{
	if (elf == NULL)
	{
		KLOG(LOG_ERROR, "Invalid elf parameter");
		return;
	}

	kprint("\n");
	kprint("ident     : %s\n", elf->header->ident);
	kprint("type      : %d\n", elf->header->type);
	kprint("machine   : %d\n", elf->header->machine);
	kprint("version   : %d\n", elf->header->version);
	kprint("entry     : %x\n", elf->header->entry);
	kprint("phoff     : %d\n", elf->header->phoff);
	kprint("shoff     : %d\n", elf->header->shoff);
	kprint("flags     : %d\n", elf->header->flags);
	kprint("ehsize    : %d\n", elf->header->ehsize);
	kprint("phentsize : %d\n", elf->header->phentsize);
	kprint("phnum     : %d\n", elf->header->phnum);
	kprint("shentsize : %d\n", elf->header->shentsize);
	kprint("shnum     : %d\n", elf->header->shnum);
	kprint("shstrndx  : %d\n", elf->header->shstrndx);
	kprint("\n");
}

void ElfProgramHeaderEntryDump(ElfProgramHeaderTable * entry)
{
	if (entry == NULL)
	{
		KLOG(LOG_ERROR, "Invalid entry parameter");
		return;
	}

	kprint("\n");
	kprint("type     : %d\n", entry->type);
	kprint("offset   : %d\n", entry->offset);
	kprint("vaddr    : %x\n", entry->vaddr);
	kprint("paddr    : %x\n", entry->paddr);
	kprint("fileSize : %d\n", entry->fileSize);
	kprint("memSize  : %d\n", entry->memSize);
	kprint("flags    : %d\n", entry->flags);
	kprint("align    : %d\n", entry->align);
	kprint("\n");
}

/*
	TODO : vérifier si tous les cas d'erreurs sont gérés (et surtout si la mémoire est bien libérée...)
*/
KeStatus LoadElf(File * file, ElfFile * elf)
{
	KeStatus status = STATUS_FAILURE;

	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
		return STATUS_INVALID_PARAMETER;
	}

	if (elf == NULL)
	{
		KLOG(LOG_ERROR, "Invalid elf parameter");
		return STATUS_INVALID_PARAMETER;
	}

	if (!ElfCheckIdent(file->content))
	{
	    KLOG(LOG_ERROR, "Not a Elf file !");
		status = STATUS_NOT_ELF_FILE;
		goto clean;
	}
	else
	{
		status = ElfInit(file, elf);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "ElfInit() failed with status %d", status);
			goto clean;
		}
		else
		{
			Process * process = NULL;
			
			status = PmCreateProcess(elf->header->entry, &process, NULL, file->parent);
			if (FAILED(status))
			{
				KLOG(LOG_ERROR, "PmCreateProcess() failed with code %d", status);
				goto clean;
			}

			SwitchToMemoryMappingOfProcess(process);

			int i = 0;
			for (; i < elf->header->phnum; i++)
			{
				u8 * vUserCodePtr = (u8 *)elf->prgHeaderTable[i].vaddr;
				u32 size = elf->prgHeaderTable[i].memSize;
				u32 count = 0;
				u8 * codeAddr = (u8 *)((u32)elf->header + elf->prgHeaderTable[i].offset);

				if (vUserCodePtr == NULL)
					break;

				//KLOG(LOG_DEBUG, "header addr : %x, offset code : %x", elf->header, elf->prgHeaderTable[i].offset);
				//KLOG(LOG_DEBUG, "code addr : %x", codeAddr);

				//KLOG(LOG_DEBUG, "[%d] Mapping code at 0x%x...", i, elf->prgHeaderTable[i].vaddr);

				while (count < size)
				{
                    // pas top, il faudrait prendre en compte la taille du code, et la pile utilisateur
                    if (!CheckUserVirtualAddressValidity((u32)vUserCodePtr))
                    {
                        KLOG(LOG_ERROR, "Invalid user virtual address (%x), can't map code in memory", vUserCodePtr);
                        status = STATUS_INVALID_VIRTUAL_USER_ADDRESS;
                        goto onError;
                    }

					// On récupère une page physique libre dans laquelle on va y copier le code
					u8 * pNewCodePage = (u8 *)GetFreePage();

					if (pNewCodePage == NULL)
					{
						KLOG(LOG_ERROR, "Couldn't find a free page");
						status = STATUS_PHYSICAL_MEMORY_FULL;
						goto onError;
					}

					// On ajoute la page physique dans l'espace d'adressage de la tâche utilisateur
					AddPageToPageDirectory(vUserCodePtr, pNewCodePage, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, process->pageDirectory);

					MmSet((u8*)vUserCodePtr, 0, PAGE_SIZE);

					// Si on a de quoi copier sur une page entière, on fait ça sinon on copie seulement le reste de code à copier
					if ((size - count) < PAGE_SIZE)
						MmCopy(codeAddr + count, vUserCodePtr, size - count);
					else
						MmCopy(codeAddr + count, vUserCodePtr, PAGE_SIZE);

					vUserCodePtr = (u8 *)((unsigned int)vUserCodePtr + (unsigned int)PAGE_SIZE);
					count += (u32)PAGE_SIZE;
				}
			}

		onError:
			// On revient sur le répertoire de pages initial du noyau
			RestoreMemoryMapping();
		}
	}

clean:
	return status;
}