#pragma once

#include <kernel/lib/types.h>

#define ATA_0 0x1F0
#define ATA_1 0x170

enum AtaType
{
	ATA_MASTER,
	ATA_SLAVE
};
typedef enum AtaType AtaType;

struct AtaInfo
{
	u8 type; // master or slave
	u16 dataPort;
	u8 errorPort;
	u8 sectorCountPort;
	u8 lbaLowPort;
	u8 lbaMidPort;
	u8 lbaHiPort;
	u8 devicePort;
	u8 commandPort;
	u8 controlPort;
} typedef AtaInfo;

AtaInfo AtaCreate(u16 portBase, AtaType type);
void AtaIdentify(AtaInfo * info);
void AtaRead(AtaInfo * info, u32 sectorNum, int count, u8 * buffer);
void AtaWrite(AtaInfo * info, u32 sectorNum, int count, u8 * buffer);

//void IdeCommon(int drive, int blockNumber, int count);
//void IdeRead(int drive, int blockNumber, int count, char * buffer);
//void IdeWrite(int drive, int blockNumber, int count, char * buffer);