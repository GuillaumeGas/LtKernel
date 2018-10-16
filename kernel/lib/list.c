#include "list.h"

#include <kernel/lib/kmalloc.h>
#include <kernel/lib/stdlib.h>

List * ListCreate()
{
	ListElem * list = (ListElem *)kmalloc(sizeof (ListElem));
	
	if (list == NULL)
		return NULL;
	
	list->prev = NULL;
	list->next = NULL;
	list->data = NULL;
	
	return list;
}

void ListDestroy(List * list)
{
	ListDestroyEx(list, NULL);
}

void ListDestroyEx(List * list, CleanFunPtr cleaner)
{
	if (list == NULL)
		return;

	ListElem * elem = list;
	ListElem * next = list->next;

	while (elem != NULL)
	{
		if (cleaner != NULL)
			cleaner(elem->data);
		kfree(elem);
		elem = next;
		next = next->next;
	}
}

void ListPush(List * list, void * data)
{
	if (list == NULL)
		return;

	if (list->data == NULL)
	{
		list->data = data;
	}
	else
	{
		ListElem * elem = list;

		while (elem->next != NULL);

		elem->next = (ListElem *)kmalloc(sizeof(ListElem));
		
		if (elem->next != NULL)
		{
			elem->next->prev = elem;
			elem = elem->next;
			elem->data = data;
			elem->next = NULL;
		}
	}
}

void * ListGet(List * list, unsigned int index)
{
	if (list == NULL)
		return NULL;

	while (index > 0)
	{
		if (list->next == NULL)
			return NULL;
		list = list->next;
		index--;
	}

	return list->data;
}

void * ListTop(List * list)
{
	if (list == NULL)
		return NULL;

	return list->data;
}