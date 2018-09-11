#include "list.h"

#include <kernel/lib/kmalloc.h>
#include <kernel/lib/stdlib.h>

void list_create(List * list)
{
	if (list == NULL)
		return;
	list->data = NULL;
	list->next = NULL;
	list->prev = NULL;
}

void list_destroy(List * list)
{
	if (list == NULL)
		return;

	struct list_elem * elem = list;
	struct list_elem * next = list->next;

	while (elem != NULL)
	{
		kfree(elem);
		elem = next;
		next = next->next;
	}
}

void list_push(List * list, void * data)
{
	if (list == NULL)
		return;

	if (list->data == NULL)
	{
		list->data = data;
	}
	else
	{
		struct list_elem * elem = list;

		while (elem->next != NULL);

		elem->next = (struct list_elem *)kmalloc(sizeof(struct list_elem));
		elem->next->prev = elem;
		elem = elem->next;
		elem->data = data;
		elem->next = NULL;
	}
}

void * list_get(List * list, unsigned int index)
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