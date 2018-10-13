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

void kmain(void);

struct mb_partial_info {
	unsigned long flags;
	unsigned long low_mem;
	unsigned long high_mem;
	unsigned long boot_device;
	unsigned long cmdline;
};

void start_kmain(struct mb_partial_info * mbi)
{
	kprint("Grub example kernel is loaded...\n");
	kprint("RAM detected : %x (lower), %x (upper)\n", mbi->low_mem, mbi->high_mem);
	kprint("Done.\n");

	while (1);
	/*cli();

	init_gdt();

	asm("movw $0x10, %ax \n \
         movw %ax, %ss \n \
         movl $0x300000, %esp");

	kmain();*/
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

void kmain(void)
{
	init_logger(LOG_SCREEN);

	sc_clear();

	sc_setColor(WHITE);
	kprint("< ## LtKernel ## >\n\n");

	sc_setColor(BLUE);

	init_pic();
	kprint("[Kernel] PIC loaded\n");

	init_idt();
	kprint("[Kernel] IDT loaded\n");

	init_serial();
	init_logger(LOG_SCREEN | LOG_SERIAL);

	kprint("[Kernel] GDT loaded\n");
	kprint("[Kernel] Serial port COM1 initialized\n");

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
