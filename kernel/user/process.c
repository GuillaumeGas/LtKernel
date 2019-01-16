#include "process.h"

#include <kernel/user/process_manager.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("USER", LOG_LEVEL, format, ##__VA_ARGS__)

static int s_CurrentProcessId = 0;

Process * GetCurrentProcess()
{
    return gCurrentProcess;
}

Process * GetProcessFromPid(int pid)
{
	if (pid < 0)
	{
		KLOG(LOG_ERROR, "Invalid pid : %d", pid);
		return NULL;
	}

	return (Process *)ListGet(gProcessList, pid);
}

/*
- Création du processus :
\  - Répertoire de pages
| - CreateProcess(pageDir, entryAddr, parent, process*) // entryAddr = adresse virtuelle fournie par ElfLoader
\  - Création du process p
| - Création de la liste d'enfants
| - Init du parent


- Vmm doit mettre à disposition une fonction de changer de contexte d'adressage
en lui filant un thread, un processus ou juste un pageDir, et de restaurer le context précédent
- Le chargeur d'elf doit :
- Créer le processus
- Switcher de contexte grâce à la fonction précédente pour mapper le binaire
*/
KeStatus CreateProcess(PageDirectory pageDirectory, u32 entryAddr, Process * parent, Process ** newProcess)
{
	KeStatus status = STATUS_FAILURE;
	Process * process = NULL;
	List * childrenList = NULL;
	List * threadsList = NULL;
	Thread * mainThread = NULL;

	if (entryAddr == 0)
	{
		KLOG(LOG_ERROR, "Invalid entryAddr parameter");
		return STATUS_INVALID_PARAMETER;
	}

	if (newProcess == NULL)
	{
		KLOG(LOG_ERROR, "Invalid newProcess parameter");
		return STATUS_NULL_PARAMETER;
	}

	process = (Process *)kmalloc(sizeof(Process));
	if (process == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Process));
		status = STATUS_ALLOC_FAILED;
		goto clean;
	}

	childrenList = ListCreate();
	if (childrenList == NULL)
	{
		KLOG(LOG_ERROR, "ListCreate() returned NULL");
		status = STATUS_FAILURE;
		goto clean;
	}

	threadsList = ListCreate();
	if (threadsList == NULL)
	{
		KLOG(LOG_ERROR, "ListCreate() returned NULL");
		status = STATUS_FAILURE;
		goto clean;
	}

	process->pageDirectory = pageDirectory;

	status = CreateMainThread(process, entryAddr, &mainThread);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "CreateMainThread() failed with code %d", status);
		goto clean;
	}

	ListPush(threadsList, mainThread);
	ListPush(gThreadsList, mainThread);
	process->mainThread = mainThread;

	process->parent = parent;
	process->children = childrenList;
	process->threads = threadsList;
	process->pid = s_CurrentProcessId++;
	process->startExcecutionTime = 0;
	process->state = PROCESS_STATE_INIT;

	childrenList = NULL;
	threadsList = NULL;

	*newProcess = process;
	process = NULL;

	status = STATUS_SUCCESS;

clean:
	if (process != NULL)
	{
		kfree(process);
		process = NULL;
	}

	if (childrenList != NULL)
	{
		ListDestroy(childrenList);
		childrenList = NULL;
	}

	if (threadsList != NULL)
	{
		ListDestroy(threadsList);
		threadsList = NULL;
	}

	return status;
}

// Don't forget to restore the memory mapping after calling this function !
void SwitchToMemoryMappingOfProcess(Process * process)
{
	if (process == NULL)
	{
		KLOG(LOG_ERROR, "Invalid process parameter");
		return;
	}

	SaveCurrentMemoryMapping();

	PageDirectory * pageDirectory = &(process->pageDirectory);
	_setCurrentPagesDirectory(pageDirectory->pdEntry);
}