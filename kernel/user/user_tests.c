#define PRINTF(str)   asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (str));
#define SCANF(buffer) asm("mov %0, %%ebx; mov $0x02, %%eax; int $0x30" :: "m" (buffer));
#define PAUSE()       while(1);

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

    begin[0] = '>';
    begin[1] = ' ';
    begin[2] = '\0';

    PRINTF(begin);
    SCANF(buffer);

    PAUSE();
}