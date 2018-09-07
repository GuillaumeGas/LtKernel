#include "panic.h"

void panic(PanicType type)
{
	kprint("Kernel Panic !");

	switch (type)
	{
	case HEAP_LIMIT:
		kprint("HEAP_LIMIT\n");
		break;
	case MEMORY_FULL:
		kprint("MEMORY_FULL\n");
		break;
	default:
		kprint("Unknown Type\n");
	}

	asm("hlt");
}