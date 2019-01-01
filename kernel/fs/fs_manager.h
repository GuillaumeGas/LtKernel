#pragma once

#include <kernel/lib/stdlib.h>
#include <kernel/drivers/ata.h>

#include "ext2.h"

KeStatus FsInit(AtaDevice * device);
void FsCleanCallback();

KeStatus ReadFileFromInode(int inodeNumber, Ext2File ** file);
void FreeFile(Ext2File * file);

#ifdef __FS__
Ext2Disk * gExt2Disk = NULL;
#else
extern Ext2Disk * gExt2Disk;
#endif