#include "panic.h"
#include <kernel/lib/stdio.h>

void panic(PanicType type)
{
	kprint("Kernel Panic ! ");

	switch (type)
	{
	case HEAP_LIMIT:
		kprint("HEAP_LIMIT\n");
		break;
	case MEMORY_FULL:
		kprint("MEMORY_FULL\n");
		break;
	case VIRTUAL_MEMORY_FULL:
		kprint("VIRTUAL_MEMORY_FULL\n");
		break;
	case PAGE_TABLE_NOTE_FOUND:
		kprint("PAGE_TABLE_NOTE_FOUND\n");
		break;
	default:
		kprint("Unknown Type\n");
	}

	while (1);
}