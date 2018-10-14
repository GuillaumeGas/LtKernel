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