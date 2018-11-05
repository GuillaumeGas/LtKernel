#include <stdio.h>
#include <stdlib.h>

void main(int argc, char ** argv)
{
	FILE * diskFile = NULL;
	FILE * binFile = NULL;
	long int fileSize = 0;
	long int diskFileSize = 0;
	char * buffer = NULL;
	int ret = 0;

	if (argc <= 2)
		return;

	printf("Working...\n");

	fopen_s(&diskFile, argv[1], "wb");
	if (diskFile == NULL)
	{
		printf("Failed to open disk file !\n");
		return;
	}

	fopen_s(&binFile, argv[2], "rb");
	if (binFile == NULL)
	{
		printf("Failed to open bin file !\n");
		return;
	}

	fseek(binFile, 0L, SEEK_END);
	fileSize = ftell(binFile);
	rewind(binFile);

	fseek(diskFile, 0L, SEEK_END);
	diskFileSize = ftell(diskFile);
	rewind(diskFile);

	printf("FileSize : %d\n", fileSize);
	printf("DiskFileSize : %d\n", diskFileSize);

	buffer = (char *)malloc(fileSize + sizeof(long int));
	if (buffer == NULL)
	{
		printf("Failed to allocate buffer\n");
		goto clean;
	}

	*((long int*)buffer) = fileSize;

	ret = fread(buffer + sizeof(long int), 1, fileSize, binFile);
	if (ret <= 0)
	{
		printf("fread() returned %d\n", ret);
		goto clean;
	}

	ret = fwrite(buffer, 1, fileSize + sizeof(long int), diskFile);
	if (ret <= 0)
	{
		printf("fwrite() returned %d\n", ret);
		goto clean;
	}

	fseek(diskFile, 0L, SEEK_END);
	diskFileSize = ftell(diskFile);
	rewind(diskFile);

	printf("DiskFileSize : %d\n", diskFileSize);

	if (diskFileSize % 512 != 0)
	{
		int i = 0;
		diskFileSize += 512 - (diskFileSize % 512);

		for (; i < diskFileSize; i++)
			fwrite(buffer, 1, 1, diskFile);
	}

	fseek(diskFile, 0L, SEEK_END);
	diskFileSize = ftell(diskFile);
	rewind(diskFile);

	printf("DiskFileSize : %d\n", diskFileSize);

	printf("Done !\n");

clean:
	if (buffer != NULL)
		free(buffer);

	fclose(diskFile);
	fclose(binFile);
}

//#include <stdio.h>
//#include <stdlib.h>
//
//#define PAGE_SIZE 100
//#define HEAP_SIZE 4096
//
//#define BLOCK_HEADER_SIZE sizeof(int)
//#define DEFAULT_BLOCK_SIZE PAGE_SIZE
//#define DEFAULT_BLOCK_SIZE_WITH_HEADER DEFAULT_BLOCK_SIZE + BLOCK_HEADER_SIZE
//
//#define BLOCK_FREE 0
//#define BLOCK_USED 1
//
//// Taille minimale d'un bloc sans compter son header
//#define MINIMAL_BLOCK_SIZE 1
//
//struct mem_block
//{
//	int size : 31; // 31 bits pour la taille, 1 bit pour indiquer si le bloc est libre (0) ou non (1)
//	int state : 1;
//	void * data;
//};
//
//void * memory = NULL;
//struct mem_block * g_heap = NULL;
//struct mem_block * g_last_heap_block = NULL;
//int HEAP_LIMIT_ADDR = 0;
//
//static void * _kmalloc(struct mem_block * block, int size);
//static void _splitBlock(struct mem_block * block, int size);
//static void _kdefrag();
//
//void panic(const char * str)
//{
//	printf(str);
//	exit(-1);
//}
//
//void mmset(unsigned char * src, unsigned char byte, int size)
//{
//	while ((size--) > 0)
//		*(src++) = byte;
//}
//
//struct mem_block * ksbrk(int n)
//{
//	// +1 car on prend en compte la taille du bloque pointé par g_heap
//	if ((g_last_heap_block + ((n + 1) * DEFAULT_BLOCK_SIZE) + (BLOCK_HEADER_SIZE * 2)) >= HEAP_LIMIT_ADDR)
//	{
//		panic("HEAP_LIMIT\n");
//	}
//	else
//	{
//		struct mem_block * new_block_v_addr = g_heap;
//		int i = 0;
//
//		if (new_block_v_addr->size != 0)
//			new_block_v_addr = g_last_heap_block + g_last_heap_block->size + BLOCK_HEADER_SIZE;
//
//		g_last_heap_block = new_block_v_addr;
//
//		/*for (; i < n; i++)
//		{
//			u32 * new_page = (u32*)get_free_page();
//
//			if (new_page == NULL)
//				panic(MEMORY_FULL);
//
//			pd_add_page(new_page, (u32)(new_block_v_addr + ((n - 1) * PAGE_SIZE)));
//		}*/
//
//		new_block_v_addr->size = n * DEFAULT_BLOCK_SIZE;
//		new_block_v_addr->state = BLOCK_FREE;
//		mmset(&(new_block_v_addr->data), new_block_v_addr->size, 0);
//
//		return new_block_v_addr;
//	}
//}
//
//void * kmalloc(int size)
//{
//	// size est la taille en octets désirée
//	// On travaille sur la taille totale d'un bloque, donc en ajoutant la taille du header
//	return _kmalloc(g_heap, size);
//}
//
//void kfree(void * ptr)
//{
//	struct mem_block * block = (struct mem_block*)((int)ptr - sizeof(int));
//	block->state = BLOCK_FREE;
//	mmset(&(block->data), block->size, 0);
//}
//
//static void * _kmalloc(struct mem_block * block, int size)
//{
//	void * res_ptr = NULL;
//
//	while (block <= g_last_heap_block && res_ptr == NULL)
//	{
//		if (block->state == BLOCK_USED || size > block->size)
//		{
//			block = block + block->size + BLOCK_HEADER_SIZE;
//			continue;
//		}
//
//		if ((block->size - (size + (int)(BLOCK_HEADER_SIZE))) >= (int)(BLOCK_HEADER_SIZE + MINIMAL_BLOCK_SIZE))
//			_splitBlock(block, size);
//
//		res_ptr = &(block->data);
//	}
//
//	if (res_ptr == NULL)
//	{
//		block = ksbrk(size / PAGE_SIZE);
//
//		res_ptr = _kmalloc(block, size);
//	}
//
//	block->state = BLOCK_USED;
//
//	return res_ptr;
//}
//
//static void _splitBlock(struct mem_block * block, int size)
//{
//	struct mem_block * second_block = block + size + BLOCK_HEADER_SIZE;
//	second_block->size = block->size - size - BLOCK_HEADER_SIZE;
//	second_block->state = BLOCK_FREE;
//
//	if (block == g_last_heap_block)
//		g_last_heap_block = second_block;
//
//	block->size = size;
//}
//
//static void _kdefrag()
//{
//	struct mem_block * block = g_heap;
//
//	while (block < g_last_heap_block)
//	{
//		struct mem_block * next = block + block->size + BLOCK_HEADER_SIZE;
//
//		if (block->state == BLOCK_FREE && next->state == BLOCK_FREE)
//		{
//			block->size += next->size + BLOCK_HEADER_SIZE;
//			mmset(&(block->data), 0, block->size);
//			if (next == g_last_heap_block)
//				g_last_heap_block = block;
//		}
//		else
//		{
//			block += block->size + BLOCK_HEADER_SIZE;
//		}
//	}
//}
//
//void init_heap()
//{
//	g_heap = (struct mem_block*)memory;
//	g_last_heap_block = g_heap;
//
//	HEAP_LIMIT_ADDR = g_heap + HEAP_SIZE;
//
//	mmset(g_heap, 0, HEAP_SIZE);
//
//	ksbrk(1);
//}
//
//void dumpHeap()
//{
//	struct mem_block * block = g_heap;
//	int i = 0;
//	printf("== Heap Dump ==\n\n");
//	while (block <= g_last_heap_block)
//	{
//		printf("[%d] Size : %d, Addr : %x, ", i++, block->size, block);
//		if (block->state == BLOCK_FREE)
//			printf("FREE\n");
//		else
//			printf("\n");
//		block = block + block->size + BLOCK_HEADER_SIZE;
//	}
//	printf("\n\n");
//}
//
//int main()
//{
//	/*char * test = NULL;
//	char * test2 = NULL;
//	memory = malloc(HEAP_SIZE);
//
//	init_heap();
//
//	dumpHeap();
//
//	test = kmalloc(1);
//	dumpHeap();
//
//	test2 = kmalloc(200);
//	dumpHeap();
//
//	kfree(test2);
//	dumpHeap();
//
//	kfree(test);
//	dumpHeap();
//
//	kdefrag();
//
//	dumpHeap();*/
//
//	unsigned int base = 0x800000;
//	unsigned int limit = 0x1000000;
//	int i = 0;
//
//	while (base < limit)
//	{
//		i++;
//		base += 0x1000;
//	}
//
//	printf("i : %d, base : %x\n", i, base);
//
//	getchar();
//
//	return 0;
//}