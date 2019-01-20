#pragma once

#include <kernel/fs/file.h>

typedef void* Handle;

struct FileHandle
{
	Handle handle;
	File * file;
	int cursor;
} typedef FileHandle;

struct DirHandle
{
	Handle handle;
	File * dir;

	/*
		Pour le moment on laisse un pointeur sur le fichier
		Le problème c'est si on utilise ce même pointeur ailleurs,
		Si ça arrive il ne faut pas free celui-ci dans le FreeDirHandle()...
		Une solution serait de garder un handle sur l'élément courant.. mais ça pourrait aussi bien
		etre un handle de fichier de que rep, qui sont conservés dans deux listes différentes...
	*/ 
	File * cursor;
} typedef DirHandle;

KeStatus CreateFileHandle(File * file, FileHandle ** fileHandle);
KeStatus CreateDirHandle(File * dir, DirHandle ** DirHandle);

void FreeFileHandle(FileHandle * fileHandle);
// TODO : attention à ne pas free le gRootFile...
void FreeDirHandle(DirHandle * dirHandle);