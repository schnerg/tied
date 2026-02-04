#include "include/py_list.h"
#include <stdio.h>


// probably not the best solution!
// void pointers are probably not best practice.
py_list_t * init_py_list( int item_size )
{
	py_list_t * list = calloc( 1, sizeof( py_list_t ) );
	list->item_size = item_size;
	list->copacity = 5;
	list->count = 0;
	list->items = calloc( list->copacity, item_size );
	return list;
}


void resize_py_list( py_list_t * list )
{
	list->copacity += 5;
	list->items = realloc( list->items, list->copacity * list->item_size );
	for( int i = list->count; i < list->copacity; i++ )
		list->items[i] = NULL;
	return;
}


void append_to_py_list( py_list_t * list, void * item )
{
	list->items[list->count] = item; 
	list->count++;
	if( list->count >= list->copacity )
		resize_py_list( list );
	return;
}
