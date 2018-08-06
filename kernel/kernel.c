#include <kernel/lib/types.h>
#include <kernel/lib/memory.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/init/gdt.h>
#include <kernel/init/idt.h>
#include <kernel/init/vmm.h>
#include <kernel/drivers/proc_io.h>
#include <kernel/drivers/screen.h>
#include <kernel/drivers/serial.h>
#include <kernel/logger.h>
#include <kernel/user/task_manager.h>

void kmain(void);

void _start(void)
{
	cli();

	init_gdt();

	asm("movw $0x10, %ax \n \
         movw %ax, %ss \n \
         movl $0x20000, %esp");

	kmain();
}

// Test utilisateur (CPL 3)
void test_task()
{
	char * str = (char*)0x400B00;
	str[0] = 'H';
	str[1] = 'e';
	str[2] = 'l';
	str[3] = 'l';
	str[4] = 'o';
	str[5] = ' ';
	str[6] = '!';
	//str[7] = 'o';
	//str[8] = 'r';
	//str[9] = 'l';
	//str[10] = 'd';
	asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (str));
	while (1) {}
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

	sti();
	kprint("[Kernel] Interrupts enabled\n");

	init_vmm();
	kprint("[Kernel] Paging enabled\n\n");

	{
		struct page_directory_entry * pd = NULL;

		// On copie la tâche utilisateur en 0x100000 
		//  (rappel : on est en noyau, l'adresse virtuelle 0x1000000 équivaut à la même en physique étant donné le mapping effectué)
		kprint("> Starting new task...\n\n");
		sc_setColor(RED);
		
		mmcopy((u8*)test_task, (u8*)USER_TASK_P_ADDR, 100);

		pd = create_task();
		switch_task(pd);
	}

	while (1);
}
