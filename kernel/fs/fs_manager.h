#pragma once

#include <kernel/lib/stdlib.h>
#include <kernel/drivers/ata.h>

#include "ext2.h"

void FsInit(AtaDevice * device);
void FsCleanCallback();

#ifdef __FS__
Ext2Disk * gExt2Disk = NULL;
#else
extern Ext2Disk * gExt2Disk;
#endif