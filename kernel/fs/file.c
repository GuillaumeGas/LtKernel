#include "file.h"
#include "fs_manager.h"

#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)

#include <kernel/debug/debug.h>

KeStatus CreateFile(Ext2Disk * disk, Ext2Inode * inode, int inum, File ** file)
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

	localFile->content = NULL;
	localFile->disk = disk;
	localFile->inode = inode;
	localFile->inum = inum;
	localFile->name = NULL;
	localFile->opened = FALSE;
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

KeStatus OpenFile(File * file)
{
	KeStatus status = STATUS_FAILURE;

	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (file->inode == NULL)
	{
		status = Ext2ReadInode(gExt2Disk, file->inum, &file->inode);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
			goto clean;
		}
	}

	status = Ext2ReadFile(file->disk, file->inode, file->inum, (char **)&file->content);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "Ext2ReadFile() failed with code %d", status);
		goto clean;
	}

	file->opened = TRUE;

	status = STATUS_SUCCESS;

clean:
	return status;
}

KeStatus OpenFileFromName(const char * filePath, File ** file)
{
	KeStatus status = STATUS_FAILURE;
	File * localFile = NULL;
	File * directory = NULL;
	char * path = (char *)filePath;

	if (filePath == NULL)
	{
		KLOG(LOG_ERROR, "Invalid filePath parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
		return STATUS_NULL_PARAMETER;
	}

	// TODO : ajouter un pointeur sur le répertoire courant du thread
	// Dans le cas d'un chemin relatif, on va utiliser celui du thread et non le root
	if (path[0] == '/')
	{
		directory = gRootFile;
		path++; // on passe le '/'
	}
	else
	{
		// TMP
		directory = gRootFile;
		//directory = gCurrentThread->currentDirectory;
	}

	while (*path != '\0')
	{
		// Si on se trouve à la fin du chemin, ce soit être le nom du fichier
		int indexSlash = 0;
		if ((indexSlash = FirstIndexOf(path, '/')) < 0)
		{
			if (IsCached(directory, path, &localFile) == TRUE)
			{
				if (localFile->opened == FALSE)
				{
					status = OpenFile(localFile);
					if (FAILED(status))
					{
						KLOG(LOG_ERROR, "OpenFile() failed with code %d", status);
						goto clean;
					}
				}

				*file = localFile;
				localFile = NULL;

				status = STATUS_SUCCESS;
				break;
			}
			else
			{
				status = STATUS_FILE_NOT_FOUND;
				goto clean;
			}
		}
		else
		{
			// Sinon, on remplace la prochaine occurence de '/' par '\0' pour déterminer quoi faire
			// et on oublie pas de rétablir le '/' ensuite
			path[indexSlash] = '\0';

			KLOG(LOG_DEBUG, "dir : %s", path);

			if (StrCmp(path, "..") == 0)
			{
				directory = directory->parent;
			}
			else if (StrCmp(path, ".") == 0)
			{
				// on ne fait rien
			}
			else
			{
				if (IsCached(directory, path, &directory) == FALSE)
				{
					status = STATUS_FILE_NOT_FOUND;
					goto clean;
				}

				if (directory->opened == FALSE)
				{
					status = OpenFile(directory);
					if (FAILED(status))
					{
						// TODO : il faudra sans doute mieux gérer cette erreur... (accès interdit par exemple ?)
						KLOG(LOG_ERROR, "OpenFile() failed with code %d", status);
						goto clean;
					}
				}
				BrowseAndCacheDirectory(directory);
			}

			path[indexSlash] = '/';
			path = &path[indexSlash + 1];
		}
	}

clean:
	return status;
}

KeStatus ReadFileFromInode(int inodeNumber, File ** file)
{
	Ext2Inode * inode = NULL;
	KeStatus status = STATUS_FAILURE;

	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
		return STATUS_INVALID_PARAMETER;
	}

	status = Ext2ReadInode(gExt2Disk, inodeNumber, &inode);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "Failed to retrieve inode %d !", inodeNumber);
		goto clean;
	}

	status = CreateFile(gExt2Disk, inode, inodeNumber, file);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "CreateFile() failed with code %d", status);
		goto clean;
	}

	status = OpenFile(*file);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "OpenFile() failed with code %d", status);
		goto clean;
	}

	status = STATUS_SUCCESS;

clean:
	if (inode != NULL)
	{
		kfree(inode);
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

	// Shouldn't happen
	if (directory->inode == NULL)
	{
		KLOG(LOG_WARNING, "directory->inode == NULL");

		status = Ext2ReadInode(gExt2Disk, directory->inum, &directory->inode);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
			goto clean;
		}
	}

	if (directory->opened == FALSE)
	{
		status = OpenFile(directory);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "OpenFile() failed with code %d", status);
			goto clean;
		}
	}

	if (IsDirectory(directory) == FALSE)
	{
		KLOG(LOG_ERROR, "Not a directory !");
		status = STATUS_NOT_A_DIRECTORY;
		goto clean;
	}

	Ext2DirectoryEntry * currentEntry = (Ext2DirectoryEntry *)directory->content;
	u32 dirContentSize = directory->inode->size;

	while (dirContentSize > 0 && currentEntry->recLen > 0)
	{
		char * fileName = (char *)kmalloc(currentEntry->nameLen + 1);
		if (fileName == NULL)
		{
			KLOG(LOG_ERROR, "Couldn't allocate %d bytes", currentEntry->nameLen + 1);
			goto clean;
		}
		MmCopy(&currentEntry->name, fileName, currentEntry->nameLen + 1);
		fileName[currentEntry->nameLen] = '\0';

		if (StrCmp(fileName, ".") != 0 && StrCmp(fileName, "..") != 0)
		{
			if (IsCached(directory, fileName, NULL) == FALSE)
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


		dirContentSize -= currentEntry->recLen;
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

	file->parent = file;

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

BOOL IsCached(File * dir, const char * fileName, File ** file)
{
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
				if (file != NULL)
				{
					*file = leaf;
				}

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
	int nb = 0;

	while (f != NULL)
	{
		if (f->name != NULL)
		{
			int i = 0;
			for (; i < nb; i++)
				kprint(" ");

			if (f->leaf != NULL)
				kprint("[%s]\n", f->name);
			else
				kprint("%s\n", f->name);
		}

		if (f->leaf != NULL)
		{
			f = f->leaf;
			nb += 4;
		}
		else if (f->next != NULL)
		{
			f = f->next;
		}
		else
		{
			f = f->parent->next;
			nb -= 4;

			if (f == dir)
				f = NULL;
		}
	}
}