#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/init/gdt.h>
#include <kernel/init/idt.h>
#include <kernel/init/memory.h>
#include <kernel/drivers/proc_io.h>
#include <kernel/drivers/screen.h>
#include <kernel/drivers/serial.h>
#include <kernel/logger.h>
#include <kernel/user/process_manager.h>
#include <kernel/multiboot.h>

void kinit(MultibootPartialInfo * mbi, u32 multibootMagicNumber);
void CheckMultibootPartialInfo(MultibootPartialInfo * mbi, u32 multibootMagicNumber);

void kmain(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
	cli();

	init_gdt();

	asm("movw $0x10, %ax \n \
         movw %ax, %ss \n \
         movl $0x300000, %esp");

	kinit(mbi, multibootMagicNumber);
}

// Test utilisateur (CPL 3)
void test_task()
{
	char * str = (char*)0x400B00;
	int i = 0;

	str[0] = 'T';
	str[1] = 'a';
	str[2] = 's';
	str[3] = 'k';
	str[4] = '1';
	str[5] = '\n';
	str[6] = '\0';

	while (1)
	{
		for (i = 0; i < 1000000; i++);
		asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (str));
	}
}

void test_task2()
{
	char * str = (char*)0x400B00;
	int i = 0;

	str[0] = 'T';
	str[1] = 'a';
	str[2] = 's';
	str[3] = 'k';
	str[4] = '2';
	str[5] = '\n';
	str[6] = '\0';

	while (1)
	{
		for (i = 0; i < 1000000; i++);
		asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (str));
	}
}

void kinit(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
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

	//init_vmm();
	//kprint("[Kernel] Paging enabled\n");

	//init_process_manager();
	//kprint("[Kernel] Process manager initialized\n\n");

	//{
	//	sc_setColor(WHITE);
	//	kprint("> Starting new task 0...\n\n");
	//	sc_setColor(RED);
	//	
	//	/*create_process(test_task, 500);
	//	create_process(test_task2, 500);*/
	//}

	//sti();

	while (1);
}

/*
	V�rifie la pr�cense, et affiche si possible les infos du multiboot.
	https://www.gnu.org/software/grub/manual/multiboot/html_node/Boot-information-format.html#Boot-information-format
*/
void CheckMultibootPartialInfo(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
	kprint("\n");

	if (multibootMagicNumber != MULTIBOOT_HEADER_MAGIC)
	{
		kprint("Invalid magic number : %x\n", multibootMagicNumber);
		asm("hlt");
	}

	if (FlagOn(mbi->flags, MEM_INFO))
	{
		kprint("[MULTIBOOT] RAM detected : %x (lower), %x (upper)\n\n", mbi->low_mem, mbi->high_mem);
	}
}