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

#include <kernel/user/process_manager.h>
#include <kernel/user/user_tests.h>

#include <kernel/logger.h>

#define __KERNEL__
#include <kernel/kernel.h>

#define __MULTIBOOT__
#include <kernel/multiboot.h>

static void kinit(MultibootPartialInfo * mbi, u32 multibootMagicNumber);
static void CheckMultibootPartialInfo(MultibootPartialInfo * mbi, u32 multibootMagicNumber);
static void InitKernelInfo();
static void InitCleanCallbacksList();
static void CleanKernel();

static List * CleanCallbacksList = NULL;

void kmain(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
	cli();

	init_gdt();

	asm("movw $0x10, %ax \n \
         movw %ax, %ss \n \
         movl $0xA0000, %esp");

	kinit(mbi, multibootMagicNumber);
}

void cleanStr(void * str)
{
	kfree(str);
}

static void kinit(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
	init_logger(LOG_SCREEN);

	sc_clear();

	sc_setColor(WHITE);
	kprint("< ## LtKernel ## >\n\n");

	init_pic();
	kprint("[Kernel] PIC loaded\n");

	init_idt();
	kprint("[Kernel] IDT loaded\n");

	init_serial();
	init_logger(LOG_SCREEN | LOG_SERIAL);

	kprint("[Kernel] GDT loaded\n");
	kprint("[Kernel] Serial port COM1 initialized\n");

	CheckMultibootPartialInfo(mbi, multibootMagicNumber);

	InitKernelInfo();
	kprint("[Kernel] Kernel info structure initialized\n");

	InitVmm();
	kprint("[Kernel] Paging enabled\n");

	init_process_manager();
	kprint("[Kernel] Process manager initialized\n\n");

	InitCleanCallbacksList();

	sti();

	create_process(test_task, 500);

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

	g_mbi = *mbi;

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
	g_kernelInfo.pageDirectory_p.pd_entry = (PageDirectoryEntry *)KERNEL_PAGE_DIR_P_ADDR;
	g_kernelInfo.pageTables_p = (PageTableEntry *)KERNEL_PAGES_TABLE_P_ADDR;
	g_kernelInfo.kernelLimit_p = KERNEL_LIMIT_P_ADDR;
	g_kernelInfo.stackAddr_p = KERNEL_STACK_P_ADDR;
	g_kernelInfo.pagesHeapBase_v = KERNEL_PAGES_HEAP_V_BASE_ADDR;
	g_kernelInfo.pagesHeapLimit_v = KERNEL_PAGES_HEAP_V_LIMIT_ADDR;
	g_kernelInfo.heapBase_v = KERNEL_HEAP_V_BASE_ADDR;
	g_kernelInfo.heapLimit_v = KERNEL_HEAP_V_LIMIT_ADDR;
}

static void InitCleanCallbacksList()
{
    CleanCallbacksList = ListCreate();

    ListPush(CleanCallbacksList, (CleanCallbackFun)VmmCleanCallback);
	ListPush(CleanCallbacksList, (CleanCallbackFun)ProcessManagerCleanCallback);
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