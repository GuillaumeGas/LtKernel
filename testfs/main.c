//#define PRINTF(str)      asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (str));

void test()
{
	char * str = (char*)0x50000000;
	str[0] = 'H';
	str[1] = 'e';
	str[2] = 'l';
	str[3] = 'l';
	str[4] = 'o';
	str[5] = ' ';
	str[6] = '!';
	str[7] = '\0';

	asm("mov $0x50000000, %ebx; mov $0x01, %eax; int $0x30");
}

void main()
{

	//asm("mov %0, %%ebx" :: "m" (str));
	//asm("mov $0x01, %eax");
	//asm("int $0x30");

	test();
	while (1);
}
