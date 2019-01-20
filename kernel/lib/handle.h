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
		Le probl�me c'est si on utilise ce m�me pointeur ailleurs,
		Si �a arrive il ne faut pas free celui-ci dans le FreeDirHandle()...
		Une solution serait de garder un handle sur l'�l�ment courant.. mais �a pourrait aussi bien
		etre un handle de fichier de que rep, qui sont conserv�s dans deux listes diff�rentes...
	*/ 
	File * cursor;
} typedef DirHandle;

KeStatus CreateFileHandle(File * file, FileHandle ** fileHandle);
KeStatus CreateDirHandle(File * dir, DirHandle ** DirHandle);

void FreeFileHandle(FileHandle * fileHandle);
// TODO : attention � ne pas free le gRootFile...
void FreeDirHandle(DirHandle * dirHandle);