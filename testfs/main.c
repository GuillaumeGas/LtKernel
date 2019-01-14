#include "stdio.h"

void print(const char * str)
{
    _print(str);
}

void test()
{
    char str[] = "Hello World !";
    print(str);
}

void main()
{
	test();
	while (1);
}
