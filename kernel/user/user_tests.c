#define PRINTF(str)      asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (str));
#define SCANF(buffer)    asm("mov %0, %%ebx; mov $0x02, %%eax; int $0x30" :: "m" (buffer));
#define EXEC(addr)       asm("mov %0, %%ebx; mov $0x03, %%eax; int $0x30" :: "m" (addr));
#define EXIT()           asm("mov $0x04, %%eax; int $0x30" ::);
#define LIST_PROCESS()   asm("mov $0x0A, %%eax; int $0x30" ::);

#define PAUSE() while(1);

#define EXEC_CMD               'e'
#define DEBUG_LIST_PROCESS_CMD 'p'
#define DEBUG_REGISTERS_CMD    'r'

#define TMP_DATA_ADDR 0x50000000

// Test utilisateur (CPL 3)
void TestTask1()
{
	char * str = (char*)0x50000000; // test avec la page réservée pour les données

	str[0] = 'T';
	str[1] = 'a';
	str[2] = 's';
	str[3] = 'k';
	str[4] = '1';
	str[5] = '\n';
	str[6] = '\0';

    PRINTF(str);

    EXIT();

	while (1);
}

void TestTask2()
{
	char * str = (char*)0x40000100;

	str[0] = 'T';
	str[1] = 'a';
	str[2] = 's';
	str[3] = 'k';
	str[4] = '2';
	str[5] = '\n';
	str[6] = '\0';

	asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (str));

    while (1);
}

void TestConsole()
{
    char * begin = (char *)TMP_DATA_ADDR;
    char * buffer = (char *)TMP_DATA_ADDR + 4;
    void * addr = TestTask1;

	while (1)
	{
		begin[0] = '>';
		begin[1] = ' ';
		begin[2] = '\0';

		PRINTF(begin);
		SCANF(buffer);

        switch (buffer[0])
        {
            case EXEC_CMD:
                EXEC(addr);
                break;
            case DEBUG_LIST_PROCESS_CMD:
                LIST_PROCESS();
                break;
        }

		PRINTF(buffer);
	}

    PAUSE();
}