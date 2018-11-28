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

#include <kernel/debug/ltdbg.h>

#include <kernel/logger.h>

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

	ScSetColor(WHITE);
	kprint("< ## LtKernel ## >\n\n");

	PicInit();
	kprint("[Kernel] PIC loaded\n");

	IdtInit();
	kprint("[Kernel] IDT loaded\n");

	SerialInit();
	LoggerInit(LOG_SCREEN | LOG_SERIAL);

	kprint("[Kernel] GDT loaded\n");
	kprint("[Kernel] Serial port COM1 initialized\n");

	CheckMultibootPartialInfo(mbi, multibootMagicNumber);

	InitKernelInfo();
	kprint("[Kernel] Kernel info structure initialized\n");

	InitVmm();
	kprint("[Kernel] Paging enabled\n");

	/* Pour commencer... */
	// Pour le test, on fait simple : vu qu'on file à bochs une image de cd avec le noyau dessus sur le channel 0, on place le disque en channel 1 en master
	// TODO : détecter les autres devices possible, permettre d'y accéder simplement, etc...
	//AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
	//if (!AtaInit(&device))
	//{
	//	kprint("[Kernel] Ata driver initialization failed !\n");
	//}
	//else
	//{
	//	kprint("[Kernel] Ata %s %s using PIO mode initialized\n", device.dataPort == ATA_PRIMARY ? "Primary" : "Secondary", device.type == ATA_MASTER ? "Master" : "Slave");
	//}

	//FsInit(&device);
	/* */


	PmInit();
	kprint("[Kernel] Process manager initialized\n\n");

	InitCleanCallbacksList();

	//PmCreateProcess(TestConsole, 500, NULL); 

	ENABLE_IRQ();

	// On part du principe que le driver du port COM est opé
	if (gKernelInfo.debug == TRUE)
	{
		LoggerInit(LOG_SCREEN); // on désactive la sortie de log du le port COM1 utilisé par le debugger
		DbgInit();
		asm("int $3");
	}

    // Fonction de nettoyage pour vérifier qu'on garde bien une trace de tout ce qu'on alloue, et qu'on est capable de tout libérer
    CleanKernel();
    CheckHeap();

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
		kprint("Invalid magic number : %x\n", multibootMagicNumber);
		asm("hlt");
	}

	if (FlagOn(mbi->flags, MEM_INFO))
	{
		kprint("[MULTIBOOT] RAM detected : %x (lower), %x (upper)\n\n", mbi->low_mem, mbi->high_mem);
	}
	else
	{
		kprint("Missing MEM_INFO flag !\n");
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