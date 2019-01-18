#include "file.h"
#include "fs_manager.h"

#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)

#include <kernel/debug/debug.h>

KeStatus CreateFile(Ext2Disk * disk, Ext2Inode * inode, int inum, void * fileContent, File ** file)
{
	KeStatus status = STATUS_FAILURE;
	File * localFile = NULL;

	if (disk == NULL)
	{
		KLOG(LOG_ERROR, "Invalid disk parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (inode == NULL)
	{
		KLOG(LOG_ERROR, "Invalid inode parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (fileContent == NULL)
	{
		KLOG(LOG_ERROR, "Invalid fileContent parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
		return STATUS_NULL_PARAMETER;
	}

	localFile = (File *)kmalloc(sizeof(File));
	if (localFile == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(File));
		status = STATUS_ALLOC_FAILED;
		goto clean;
	}

	localFile->content = fileContent;
	localFile->disk = disk;
	localFile->inode = inode;
	localFile->inum = inum;
	localFile->name = NULL;
	localFile->next = NULL;
	localFile->parent = NULL;
	localFile->prev = NULL;

	*file = localFile;
	localFile = NULL;

	status = STATUS_SUCCESS;

clean:
	if (localFile != NULL)
	{
		kfree(localFile);
		localFile = NULL;
	}

	return status;
}

/*
	
*/
KeStatus BrowseAndCacheDirectory(File * directory)
{
	KeStatus status = STATUS_FAILURE;

	if (directory == NULL)
	{
		KLOG(LOG_ERROR, "Invalid directory parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (directory->inode == NULL)
	{
		// Pour le moment, forcément le cas car quand on créé un fichier, inode et content sont renseignés
		status = Ext2ReadInode(gExt2Disk, directory->inum, &directory->inode);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
			goto clean;
		}
	}

	// TODO : quand Ext2ReadFile aura été mis à jour, décommenter...
	if (directory->content == NULL)
	{
		KLOG(LOG_WARNING, "content is null !");
		status = STATUS_UNEXPECTED;
		goto clean;

		//status = Ext2ReadFile(gExt2Disk, directory->inode, directory->inode, &directory->content);
		//if (FAILED(status))
		//{
		//	KLOG(LOG_ERROR, "Ext2ReadFile() failed with code %d", result);
		//	goto clean;
		//}
	}

	if (IsDirectory(directory) == FALSE)
	{
		KLOG(LOG_WARNING, "Not a directory");

		//KLOG(LOG_ERROR, "Not a directory !");
		//status = STATUS_NOT_A_DIRECTORY;
		//goto clean;
	}

	Ext2DirectoryEntry * currentEntry = (Ext2DirectoryEntry *)directory->content;
	Ext2DirectoryEntry * previousEntry = NULL;
	u32 dirContentSize = directory->inode->size;

	KLOG(LOG_DEBUG, "dirContentSize : %d", dirContentSize);
	KLOG(LOG_DEBUG, "currentEntry->recLen : %d", currentEntry->recLen);

	while (/*dirContentSize > 0 && */currentEntry->recLen > 0)
	{
		char * fileName = (char *)kmalloc(currentEntry->nameLen + 1);
		if (fileName == NULL)
		{
			KLOG(LOG_ERROR, "Couldn't alloocate %d bytes", currentEntry->nameLen + 1);
			goto clean;
		}
		MmCopy(&currentEntry->name, fileName, currentEntry->nameLen + 1);
		fileName[currentEntry->nameLen] = '\0';

		if (StrCmp(fileName, ".") != 0 && StrCmp(fileName, "..") != 0)
		{
			if (IsCached(directory, fileName) == FALSE)
			{
				File * newCachedFile = (File *)kmalloc(sizeof(File));
				if (newCachedFile == NULL)
				{
					KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(File));
					goto clean;
				}

				newCachedFile->content = NULL;
				newCachedFile->disk = gExt2Disk;
				newCachedFile->inode = NULL;
				newCachedFile->inum = currentEntry->inode;
				newCachedFile->leaf = NULL;
				newCachedFile->name = fileName;
				fileName = NULL;
				newCachedFile->next = NULL;
				newCachedFile->parent = directory;
				newCachedFile->prev = NULL;

				if (directory->leaf == NULL)
				{
					directory->leaf = newCachedFile;
				}
				else
				{
					newCachedFile->next = directory->leaf;
					directory->leaf->prev = newCachedFile;
					directory->leaf = newCachedFile;
				}
			}
			else
			{
				kfree(fileName);
				fileName = NULL;
			}
		}
		else
		{
			kfree(fileName);
			fileName = NULL;
		}


		previousEntry = currentEntry;
		currentEntry = (Ext2DirectoryEntry *)((char *)currentEntry + (currentEntry->recLen));

		if (currentEntry == NULL)
		{
			KLOG(LOG_ERROR, "NULL irectory entry");
			status = STATUS_UNEXPECTED;
			goto clean;
		}
	}

	status = STATUS_SUCCESS;

clean:
	return status;
}

KeStatus InitRootFile(File * file)
{
	KeStatus status = STATUS_FAILURE;
	const char name[] = "/";

	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
		return STATUS_NULL_PARAMETER;
	}

	file->name = (char *)kmalloc(StrLen(name) + 1);
	if (file->name == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", StrLen(name));
		status = STATUS_ALLOC_FAILED;
		goto clean;
	}

	StrCpy(name, file->name);

	status = STATUS_SUCCESS;

clean:
	return status;
}

BOOL IsDirectory(File * file)
{
	if (file->inode == NULL)
	{
		KeStatus status = Ext2ReadInode(gExt2Disk, file->inum, &file->inode);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
			return FALSE;
		}
	}

	return (file->inode->mode & EXT2_S_IFDIR) ? TRUE : FALSE;
}

BOOL IsCached(File * dir, const char * fileName)
{
	return FALSE;
	if (dir == NULL)
	{
		KLOG(LOG_ERROR, "Invalid dir parameter");
		return FALSE;
	}

	if (fileName == NULL)
	{
		KLOG(LOG_ERROR, "Invalid fileName parameter");
		return FALSE;
	}

	File * leaf = dir->leaf;
	while (leaf != NULL)
	{
		if (leaf->name != NULL)
		{
			if (StrCmp(leaf->name, fileName) == 0)
			{
				return TRUE;
			}
		}
		leaf = leaf->next;
	}
	return FALSE;
}

void PrintDirectory(File * dir)
{
	if (dir == NULL)
	{
		KLOG(LOG_ERROR, "Invalid dir parameter");
		return;
	}

	File * f = dir;
	BOOL canEnter = TRUE;

	while (f != NULL)
	{
		if (f->name != NULL)
		{
			if (f->leaf != NULL)
				kprint(" %s >\n", f->name);
			else
				kprint("- %s\n", f->name);
		}

		if (f->leaf != NULL && canEnter == TRUE)
		{
			f = f->leaf;
		}
		else if (f->next != NULL)
		{
			f = f->next;
			canEnter = TRUE;
		}
		else
		{
			f = f->parent;
			canEnter = FALSE;

			if (f == dir)
				f = NULL;
		}
	}
}