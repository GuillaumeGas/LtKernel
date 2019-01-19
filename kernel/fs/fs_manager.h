#pragma once

#include <kernel/lib/stdlib.h>
#include <kernel/drivers/ata.h>

#include "ext2.h"
#include "file.h"

KeStatus FsInit(AtaDevice * device);
void FsCleanCallback();

void FreeFile(File * file);

#ifdef __FS__
Ext2Disk * gExt2Disk = NULL;
File * gRootFile = NULL;
#else
extern Ext2Disk * gExt2Disk;
extern File * gRootFile;
#endif