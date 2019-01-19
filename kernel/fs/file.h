#pragma once

#include "ext2.h"

typedef struct File File;
struct File
{
	Ext2Disk * disk;
	int inum;
	Ext2Inode * inode;
	char * name;
	void * content;
	BOOL opened;
	File * leaf;   // premier répertoire fils
	File * parent;
	File * prev;
	File * next;
};

KeStatus CreateFile(Ext2Disk * disk, Ext2Inode * inode, int inum, File ** file);
KeStatus OpenFile(File * file);
KeStatus OpenFileFromName(const char * fileName, File ** file);
KeStatus ReadFileFromInode(int inodeNumber, File ** file);
KeStatus BrowseAndCacheDirectory(File * directory);
KeStatus InitRootFile(File * file);
BOOL IsDirectory(File * file);
BOOL IsCached(File * dir, const char * fileName, File ** file);

void PrintDirectory(File * dir);