#include "list.h"

#include <kernel/lib/kmalloc.h>
#include <kernel/lib/stdlib.h>

#include <kernel/lib/stdio.h>

#include <kernel/debug/debug.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("LIB", LOG_LEVEL, format, ##__VA_ARGS__)

List * ListCreate()
{
	ListElem * list = (ListElem *)kmalloc(sizeof (ListElem));
	
	if (list == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(ListElem));
		return NULL;
	}
	
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
	{
		KLOG(LOG_ERROR, "Invalid list parameter");
		return;
	}

	ListElem * elem = list;
	ListElem * next = list->next;

	while (elem != NULL)
	{
		if (cleaner != NULL && elem->data != NULL)
			cleaner(elem->data);
		kfree(elem);
		elem = next;

        if (next != NULL)
            next = next->next;
	}
}

void ListPush(List * list, void * data)
{
	if (list == NULL)
	{
		KLOG(LOG_ERROR, "Invalid list parameter");
		return;
	}

	if (list->data == NULL)
	{
		list->data = data;
	}
	else
	{
		ListElem * elem = list;

        while (elem->next != NULL)
        {
            elem = elem->next;
        }

		elem->next = (ListElem *)kmalloc(sizeof(ListElem));
		
		if (elem->next == NULL)
		{
			KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(ListElem));
			return;
		}

		elem->next->prev = elem;
		elem = elem->next;
		elem->data = data;
		elem->next = NULL;
	}
}

void * ListGet(List * list, unsigned int index)
{
	if (list == NULL)
	{
		KLOG(LOG_ERROR, "Invalid list parameter");
		return NULL;
	}

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
	{
		KLOG(LOG_ERROR, "Invalid list parameter");
		return NULL;
	}

	return list->data;
}

void * ListPop(List ** list)
{
	if (list == NULL)
	{
		KLOG(LOG_ERROR, "Invalid list parameter");
		return NULL;
	}

	if ((*list) == NULL)
	{
		KLOG(LOG_ERROR, "Invalid list parameter");
		return NULL;
	}

    void * data = (*list)->data;
    ListElem * next = (*list)->next;
    kfree(*list);
    *list = (List *)next;
    return data;
}

void ListEnumerate(List * list, EnumerateFunPtr callback, void * Context)
{
	if (list == NULL)
	{
		KLOG(LOG_ERROR, "Invalid list parameter");
		return;
	}
	if (callback == NULL)
	{
		KLOG(LOG_ERROR, "Invalid callback parameter");
		return;
	}

    ListElem * elem = list;
    ListElem * next = list->next;

    while (elem != NULL)
    {
        callback(elem->data, Context);
        elem = next;

        if (next != NULL)
            next = next->next;
    }
}