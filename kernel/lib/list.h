#pragma once

struct ListElem
{
	struct ListElem * prev;
	struct ListElem * next;
	void * data;
} typedef ListElem;
typedef ListElem List;

typedef void(*CleanFunPtr)(void*);

List * ListCreate();
void ListDestroy(List * list);
void ListDestroyEx(List * list, CleanFunPtr cleaner);
void ListPush(List * list, void * data);
void * ListGet(List * list, unsigned int index);
void * ListTop(List * list);
void * ListPop(List ** list);