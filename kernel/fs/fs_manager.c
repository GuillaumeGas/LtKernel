#define __FS__
#include "fs_manager.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)
#include <kernel/debug/debug.h>

#include <kernel/fs/elf.h>

static void TestFs();

KeStatus FsInit(AtaDevice * device)
{
    KeStatus status = Ext2ReadDiskOnDevice(device, &gExt2Disk);
	if (FAILED(status) || gExt2Disk == NULL)
	{
		KLOG(LOG_ERROR, "File system intialization failed with status %d", status);
        return status;
	}

    TestFs();

    return STATUS_SUCCESS;
}

void FsCleanCallback()
{
	Ext2FreeDisk(gExt2Disk);
}

static void TestFs()
{
    Ext2Inode * inode = NULL;
    KeStatus status = STATUS_FAILURE;
    int inodeNumber = 12;

    status = Ext2ReadInode(gExt2Disk, inodeNumber, &inode);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Failed to retrieve inode %d !", inodeNumber);
    }
    else
    {
        Ext2File * file = NULL;
        status = Ext2ReadFile(gExt2Disk, inode, (Ext2File *)&file);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "Failed to read file !");
        }
        else
        {
            if (!ElfCheckIdent(file))
            {
                KLOG(LOG_ERROR, "Not a Elf file !");
                kfree(file);
            }
            else
            {
                ElfFile elf = { 0 };

                status = ElfInit(file, &elf);
                if (FAILED(status))
                {
                    KLOG(LOG_ERROR, "ElfInit() failed with status %d", status);
                }
                else
                {
                    int i = 0;
                    for (; i < elf.header->phnum; i++)
                    {
                        KLOG(LOG_DEBUG, "[%d] 0x%x", i, elf.prgHeaderTable[i].vaddr);
                    }

                    ElfHeaderDump(&elf);

                    ElfFree(&elf);
                }
            }
        }
        kfree(inode);
    }
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
    kprint("entry     : %d\n", elf->header->entry);
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