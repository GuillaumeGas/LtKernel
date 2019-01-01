#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/lib/list.h>

#include <kernel/init/gdt.h>
#include <kernel/init/idt.h>
#include <kernel/init/vmm.h>

#include <kernel/drivers/proc_io.h>
#include <kernel/drivers/screen.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/ata.h>

#include <kernel/fs/fs_manager.h>

#include <kernel/user/process_manager.h>
#include <kernel/user/user_tests.h>

#include <kernel/debug/debug.h>
#include <kernel/debug/ltdbg.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("KERNEL", LOG_LEVEL, format, ##__VA_ARGS__)

#define __KERNEL__
#include <kernel/kernel.h>

#define __MULTIBOOT__
#include <kernel/multiboot.h>

#define KERNEL_DEBUG

static void KernelInit(MultibootPartialInfo * mbi, u32 multibootMagicNumber);
static void CheckMultibootPartialInfo(MultibootPartialInfo * mbi, u32 multibootMagicNumber);
static void InitKernelInfo();
static void InitCleanCallbacksList();
static void CleanKernel();

static List * CleanCallbacksList = NULL;

void kmain(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
	DISABLE_IRQ();

	GdtInit();

	asm("movw $0x10, %ax \n \
         movw %ax, %ss \n \
         movl $0xA0000, %esp");

	KernelInit(mbi, multibootMagicNumber);
}

static void KernelInit(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
	LoggerInit(LOG_SCREEN);

	ScClear();

	ScSetColor(RED);
	kprint("< ## LtKernel ## >\n\n");
    ScSetColor(WHITE);

	PicInit();
	KLOG(LOG_INFO, "PIC loaded");

	IdtInit();
    KLOG(LOG_INFO, "IDT loaded");

	SerialInit();
	LoggerInit(LOG_SCREEN | LOG_SERIAL);

    KLOG(LOG_INFO, "GDT loaded");
    KLOG(LOG_INFO, "Serial port COM1 initialized");

	CheckMultibootPartialInfo(mbi, multibootMagicNumber);

	InitKernelInfo();
    KLOG(LOG_INFO, "Kernel info structure initialized");

	InitVmm();
    KLOG(LOG_INFO, "Paging enabled");

	PmInit();
	KLOG(LOG_INFO, "Process manager initialized");

	ENABLE_IRQ();

	// On part du principe que le driver du port COM est opé
	if (gKernelInfo.debug == TRUE)
	{
		LoggerInit(LOG_SCREEN); // on désactive la sortie de log du le port COM1 utilisé par le debugger
		DbgInit();
		__debugbreak();
	}

	/* Pour commencer... */
	// Pour le test, on fait simple : vu qu'on file à bochs une image de cd avec le noyau dessus sur le channel 0, on place le disque en channel 1 en master
	// TODO : détecter les autres devices possible, permettre d'y accéder simplement, etc...
	AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
	if (!AtaInit(&device))
	{
        KLOG(LOG_ERROR, "Ata driver initialization failed !");
	}
	else
	{
        KLOG(LOG_INFO, "Ata %s %s using PIO mode initialized", device.dataPort == ATA_PRIMARY ? "Primary" : "Secondary", device.type == ATA_MASTER ? "Master" : "Slave");
	}

	KeStatus status = FsInit(&device);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "File system initialization failed !");
    }
    else
    {
        KLOG(LOG_INFO, "File system initialized");
    }
	/* */

	InitCleanCallbacksList();

	{
		int pid = -1;
		KeStatus status = PmCreateProcess(TestConsole, 500, NULL, &pid);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "PmCreateProcess() failed with status : %d", status);
		}
	}

    // Fonction de nettoyage pour vérifier qu'on garde bien une trace de tout ce qu'on alloue, et qu'on est capable de tout libérer
    //CleanKernel();
    //CheckHeap();

	while (1);
}

/*
	Vérifie la précense, et affiche si possible les infos du multiboot.
	https://www.gnu.org/software/grub/manual/multiboot/html_node/Boot-information-format.html#Boot-information-format
*/
static void CheckMultibootPartialInfo(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
	kprint("\n");

	gMbi = *mbi;

	if (multibootMagicNumber != MULTIBOOT_HEADER_MAGIC)
	{ 
		KLOG(LOG_ERROR, "Invalid magic number : %x", multibootMagicNumber);
		asm("hlt");
	}

	if (FlagOn(mbi->flags, MEM_INFO))
	{
		KLOG(LOG_INFO, "[MULTIBOOT] RAM detected : %x (lower), %x (upper)", mbi->low_mem, mbi->high_mem);
	}
	else
	{
		KLOG(LOG_WARNING, "Missing MEM_INFO flag !");
		asm("hlt");
	}
}

static void InitKernelInfo()
{
	gKernelInfo.pPageDirectory.pdEntry = (PageDirectoryEntry *)KERNEL_PAGE_DIR_P_ADDR;
	gKernelInfo.pPageTables = (PageTableEntry *)KERNEL_PAGES_TABLE_P_ADDR;
	gKernelInfo.pKernelLimit = KERNEL_LIMIT_P_ADDR;
	gKernelInfo.pStackAddr = KERNEL_STACK_P_ADDR;
	gKernelInfo.vPagesHeapBase = KERNEL_PAGES_HEAP_V_BASE_ADDR;
	gKernelInfo.vPagesHeapLimit = KERNEL_PAGES_HEAP_V_LIMIT_ADDR;
	gKernelInfo.vHeapBase = KERNEL_HEAP_V_BASE_ADDR;
	gKernelInfo.vHeapLimit = KERNEL_HEAP_V_LIMIT_ADDR;

#ifdef KERNEL_DEBUG
	gKernelInfo.debug = TRUE;
#else
	gKernelInfo.debug = FALSE;
#endif
}

static void InitCleanCallbacksList()
{
    CleanCallbacksList = ListCreate();

    ListPush(CleanCallbacksList, (CleanCallbackFun)VmmCleanCallback);
	ListPush(CleanCallbacksList, (CleanCallbackFun)PmCleanCallback);
	ListPush(CleanCallbacksList, (CleanCallbackFun)FsCleanCallback);
}

static void CleanKernel()
{
    while (CleanCallbacksList != NULL)
    {
        CleanCallbackFun callback = ListPop(&CleanCallbacksList);

        if (callback != NULL)
            callback();
    }
}