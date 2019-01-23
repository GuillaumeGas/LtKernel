#include <kernel/lib/types.h>
#include <kernel/lib/stdlib.h>

void Halt()
{
	asm("hlt");
}

void Pause ()
{
    while (1) {}
}

void MmCopy(void * src, void * dst, unsigned int size)
{
	if (src == NULL || size == 0)
		return;

	u8 * _dst = (u8 *)dst;
	u8 * _src = (u8 *)src;

	while ((size--) > 0)
		*(_dst++) = *(_src++);
}

void MmSet(u8 * src, u8 byte, unsigned int size)
{
	if (src == NULL || size == 0)
		return;

	while ((size--) > 0)
		*(src++) = byte;
}

void StrCpy(const char * src, char * dst)
{
	if (src == NULL || dst == NULL)
		return;

	while (*src != '\0')
		*(dst++) = *(src++);
	*dst = '\0';
}

unsigned long StrLen(const char * str)
{
	if (str == NULL)
		return 0;

	unsigned long length = 0;
	while (*(str++) != '\0')
		length++;
	return length;
}

int StrCmp(const char * str1, const char * str2)
{
	if (str1 == NULL)
		return -2;

	if (str2 == NULL)
		return -2;

	unsigned int index = 0;

	while (str1[index] != '\0' && str2[index] != '\0')
	{
		if (str1[index] < str2[index])
			return -1;

		if (str1[index] > str2[index])
			return 1;

		if (str1[index + 1] == '\0' && str2[index + 1] != '\0')
			return -1;

		if (str2[index + 1] == '\0' && str1[index + 1] != '\0')
			return 1;

		index++;
	}

	return 0;
}

int FirstIndexOf(const char * str, char const c)
{
	if (str == NULL)
		return -2;

	int index = 0;

	while (str[index] != '\0')
	{
		if (str[index] == c)
			return index;
		index++;
	}

	return -1;
}