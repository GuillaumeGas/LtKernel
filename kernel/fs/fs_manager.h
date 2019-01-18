#pragma once

#include <kernel/lib/stdlib.h>
#include <kernel/drivers/ata.h>

#include "ext2.h"

KeStatus FsInit(AtaDevice * device);
void FsCleanCallback();

KeStatus ReadFileFromInode(int inodeNumber, File ** file);
void FreeFile(File * file);

#ifdef __FS__
Ext2Disk * gExt2Disk = NULL;
File * gRootFile = NULL;
#else
extern Ext2Disk * gExt2Disk;
extern File * gRootFile;
#endif