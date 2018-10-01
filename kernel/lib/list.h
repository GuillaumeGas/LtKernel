#pragma once

struct list_elem
{
	struct list_elem * prev;
	struct list_elem * next;
	void * data;
};
typedef struct list_elem List;

List * list_create();
void list_destroy(List * list);
void list_push(List * list, void * data);
void * list_get(List * list, unsigned int index);
void * list_top(List * list);