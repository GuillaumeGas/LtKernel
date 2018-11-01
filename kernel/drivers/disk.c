//#include "disk.h"
//
//#include <kernel/lib/stdlib.h>
//#include <kernel/drivers/proc_io.h>
//#include <kernel/lib/stdio.h>
//
//// Base : https://github.com/AlgorithMan-de/wyoos/commit/fa076f542ab5f16a32bd141e4b44c62ab0bac9b8
////        https://github.com/levex/osdev/blob/master/drivers/ata.c
//
//#define ATA_MASTER_VALUE     0xA0
//#define ATA_SLAVE_VALUE      0xB0
//#define ATA_IDENTIFY_COMMAND 0xEC
//
//#define ATA_STATUS_BUSY          0x80
//#define ATA_STATUS_DRIVE_READY   0x40
//#define ATA_STATUS_WRITE_FAULT   0x20
//#define ATA_STATUS_SEEK_COMPLETE 0x10
//#define ATA_STATUS_REQ_READY     0x08
//#define ATA_STATUS_CORRECTED     0x04
//#define ATA_STATUS_INDEX         0x02
//#define ATA_STATUS_ERROR         0x01
//
//#define SECTOR_SIZE 512
//
//AtaInfo AtaCreate(u16 portBase, AtaType type)
//{
//	AtaInfo info = { 0 };
//
//	info.type = ATA_MASTER_VALUE;
//	info.dataPort = portBase;
//	info.errorPort = portBase + 0x1;
//	info.sectorCountPort = portBase + 0x2;
//	info.lbaLowPort = portBase + 0x3;
//	info.lbaMidPort = portBase + 0x4;
//	info.lbaHiPort = portBase + 0x5;
//	info.devicePort = portBase + 0x6;
//	info.commandPort = portBase + 0x7;
//	info.controlPort = portBase + 0x206;
//
//	return info;
//}
//
//
//// COPY
//
//#define ATA_REG_ALTSTATUS 0x0C
//#define ATA_REG_STATUS 0x07
//
//#define ATA_SR_BSY     0x80
//#define ATA_SR_DRDY    0x40
//#define ATA_SR_DF      0x20
//#define ATA_SR_DSC     0x10
//#define ATA_SR_DRQ     0x08
//#define ATA_SR_CORR    0x04
//#define ATA_SR_IDX     0x02
//#define ATA_SR_ERR 0x01
//
//#define ATA_REG_DATA       0x00
//#define ATA_REG_ERROR      0x01
//#define ATA_REG_FEATURES   0x01
//#define ATA_REG_SECCOUNT0  0x02
//#define ATA_REG_LBA0       0x03
//#define ATA_REG_LBA1       0x04
//#define ATA_REG_LBA2       0x05
//#define ATA_REG_HDDEVSEL   0x06
//#define ATA_REG_COMMAND    0x07
//#define ATA_REG_STATUS     0x07
//#define ATA_REG_SECCOUNT1  0x08
//#define ATA_REG_LBA3       0x09
//#define ATA_REG_LBA4       0x0A
//#define ATA_REG_LBA5       0x0B
//#define ATA_REG_CONTROL    0x0C
//#define ATA_REG_ALTSTATUS  0x0C
//#define ATA_REG_DEVADDRESS 0x0D
//
//#define ATA_CMD_READ_PIO          0x20
//#define ATA_CMD_READ_PIO_EXT      0x24
//#define ATA_CMD_READ_DMA          0xC8
//#define ATA_CMD_READ_DMA_EXT      0x25
//#define ATA_CMD_WRITE_PIO         0x30
//#define ATA_CMD_WRITE_PIO_EXT     0x34
//#define ATA_CMD_WRITE_DMA         0xCA
//#define ATA_CMD_WRITE_DMA_EXT     0x35
//#define ATA_CMD_CACHE_FLUSH       0xE7
//#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
//#define ATA_CMD_PACKET            0xA0
//#define ATA_CMD_IDENTIFY_PACKET   0xA1
//#define ATA_CMD_IDENTIFY 0xEC
//
//void
//ata_io_wait(int io_base) {
//	inb(io_base + ATA_REG_ALTSTATUS);
//	inb(io_base + ATA_REG_ALTSTATUS);
//	inb(io_base + ATA_REG_ALTSTATUS);
//	inb(io_base + ATA_REG_ALTSTATUS);
//}
//
//int
//ata_status_wait(int io_base, int timeout) {
//	int status;
//
//	if (timeout > 0) {
//		int i = 0;
//		while ((status = inb(io_base + ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout)) i++;
//	}
//	else {
//		while ((status = inb(io_base + ATA_REG_STATUS)) & ATA_SR_BSY);
//	}
//	return status;
//}
//
//int
//ata_wait(int io, int adv)
//{
//	u8 status = 0;
//
//	ata_io_wait(io);
//
//	status = ata_status_wait(io, -1);
//
//	if (adv) {
//		status = inb(io + ATA_REG_STATUS);
//		if (status & ATA_SR_ERR) return 1;
//		if (status & ATA_SR_DF)  return 1;
//		if (!(status & ATA_SR_DRQ)) return 1;
//	}
//
//	return 0;
//}
//// END COPY
//
//void AtaIdentify(AtaInfo * info)
//{
//	u8 status = 0;
//
//    // Selecting drive...
//    outb(info->devicePort, info->type);
//    kprint("Drive selected\n");
//
//    // We send '0' on the following ports before sending the IDENTIFY command, according to the ATA specs
//	outb(info->sectorCountPort, 0);
//	outb(info->lbaLowPort, 0);
//	outb(info->lbaMidPort, 0);
//	outb(info->lbaHiPort, 0);
//
//	outb(info->commandPort, ATA_IDENTIFY_COMMAND);
//
//	status = inb(info->commandPort);
//	if (status == 0x00)
//	{
//		kprint("[ATA][INFO] : AtaIdentify() retuned 0 for identify command\n");
//		return;
//	}
//
//    kprint("%s %s is connected.\n", info->devicePort == ATA_PRIMARY ? "Primary" : "Secondary", info->type == ATA_MASTER ? "Master" : "Slave");
//
//	//int i = 0;
//	//for (; i < 256; i++)
//	//{
//	//	inw(info->dataPort);
//	//}
//
//	// TODO : 'register' l'ATA device et le mettre à disposition
//}
//
//static void AtaWriteSector(AtaInfo * info, u32 sectorNum, u8 * buffer, unsigned long pos)
//{
//	u8 cmd = 0xE0;
//
//	outb(info->dataPort + ATA_REG_CONTROL, 0x02);
//
//	ata_wait(info->dataPort, 0);
//
//	outb(info->dataPort + ATA_REG_HDDEVSEL, (cmd | (u8)((pos >> 24 & 0x0F))));
//	ata_wait(info->dataPort, 0);
//	outb(info->dataPort + ATA_REG_FEATURES, 0x00);
//	outb(info->dataPort + ATA_REG_SECCOUNT0, 1);
//	outb(info->lbaLowPort, (u8)(pos & 0x000000ff));
//	outb(info->lbaMidPort, (u8)((pos & 0x0000ff00) >> 8));
//	outb(info->lbaHiPort, (u8)((pos & 0x00ff0000) >> 16));
//	outb(info->commandPort, ATA_CMD_WRITE_PIO);
//	ata_wait(info->dataPort, 0);
//
//	//for (int i = 0; i < 16; i++) {
//		outw(info->dataPort, buffer[0]);
//		asm("nop; nop; nop");
//	//}
//	//outb(info->commandPort, ATA_CMD_CACHE_FLUSH);
//
//	//ata_wait(info->dataPort, 0);
//}
//
//void AtaWrite(AtaInfo * info, u32 sectorNum, int count, u8 * buffer) 
//{
//	if (buffer == NULL)
//		return;
//
//	unsigned long pos = info->pos;
//
//	DISABLE_IRQ();
//
//	unsigned int i = 0;
//	for (; i < count; i++)
//	{
//		AtaWriteSector(info, sectorNum, buffer, pos);
//		buffer += SECTOR_SIZE;
//		for (int j = 0; j < 1000; j++);
//	}
//	info->pos += count;
//
//	ENABLE_IRQ();
//}
//
//void AtaRead(AtaInfo * info, u32 sectorNum, int count, u8 * buffer) {}
//
////void IdeCommon(int drive, int blockNumber, int count)
////{
////    outb(0x171, 0x0);                                // 0 sur le port 0x171
////    outb(0x172, count);                              // Numéro du secteur
////    outb(0x173, (unsigned char) blockNumber);        // 8 bits de poids faible du bloque addresse
////    outb(0x174, (unsigned char)(blockNumber >> 8));  // 8 bits suivant
////    outb(0x175, (unsigned char)(blockNumber >> 16)); // --
////
////    // "Drive indicator, magic bits, highest 4 bits of the block address"
////    outb(0x176, 0xE0 | (drive << 4) | ((blockNumber >> 24) & 0x0F));
////}
////
////void IdeRead(int drive, int blockNumber, int count, char * buffer)
////{
////    u16 tmp = 0;
////    int idx = 0;
////
////    IdeCommon(drive, blockNumber, count);
////    outb(0x177, 0x20);
////
////    // On attend que le drive soit ready
////    while (!(inb(0x177) & 0x08));
////
////    for (; idx < 256 * count; idx++)
////    {
////        tmp = inw(0x170);
////        buffer[idx * 2] = (unsigned char)tmp;
////        buffer[idx * 2 + 1] = (unsigned char)(tmp >> 8);
////    }
////}
////
////void IdeWrite(int drive, int blockNumber, int count, char * buffer)
////{
////    u16 tmp = 0;
////    int idx = 0;
////
////    IdeCommon(drive, blockNumber, count);
////    outb(0x177, 0x30);
////
////    // On que le drive dise que c'est bon
////    while (!(inb(0x177) & 0x08));
////
////    for (; idx < 255 * count; idx++)
////    {
////        tmp = (buffer[idx * 2 + 1] << 8) | buffer[idx * 2];
////        outw(0x170, tmp);
////    }
////}