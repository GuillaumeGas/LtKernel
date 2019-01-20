#pragma once

#include <kernel/init/vmm.h>

#include <kernel/lib/stdlib.h>
#include <kernel/lib/types.h>
#include <kernel/lib/list.h>

#include <kernel/fs/file.h>

#include "common.h"
#include "thread.h"
#include "process.h"

#define USER_TASK_V_ADDR  0x40000000
#define USER_STACK_V_ADDR 0xE0000000

// Sélecteurs de segment de code et de pile d'une tâche utilisateur
// Les bits RPL (niveau de privilège) sont à '11' afin de permettre le passage en niveau de privilèges utilisateur
// lors de l'exécution de l'instruction iret (voir process_starter.asm et le fonctionnement de iret)
#define USER_CODE_SEG_SELECTOR 0x1B
#define USER_STACK_SEG_SELECTOR 0x23
#define USER_DATA_SEG_SELECTOR 0x23

#ifdef __PROCESS_MANAGER__
Process * gCurrentProcess = NULL;
List * gProcessList = NULL;
unsigned int gNbProcess = 0;

Thread * gCurrentThread = NULL;
List * gThreadsList = NULL;
unsigned int gNbThreads = 0;

int gThreadId = 0;
#else
extern Process * gCurrentProcess;
extern List * gProcessList;
extern unsigned int gNbProcess;

extern Thread * gCurrentThread;
extern List * gThreadsList;
extern unsigned int gNbThreads;

extern int gThreadId;
#endif

void PmInit();

KeStatus PmCreateProcess(u32 entryPoint, Process ** newProcess, Process * parent, File * location);

void PmStartThread(int tid);
void PmCleanCallback();
void PmPrintProcessList();

void PmCreateKernelThread();

void _start_process(PageDirectoryEntry * pd, u32 ss, u32 esp, u32 eflags, u32 cs, u32 eip,
	u32 eax, u32 ecx, u32 edx, u32 ebx, u32 ebp, u32 esi, u32 edi, u32 ds, u32 es, u32 fs, u32 gs,
	ExecMode execMode);