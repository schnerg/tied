#include "include/py_list.h"

// probably not the best solution!
py_list_t * init_py_list( int item_size )
{
	py_list_t * list = calloc( 1, sizeof( py_list_t ) );
	list->item_size = item_size;
	list->copacity = 5;
	list->items = calloc( list->copacity, sizeof( item_size ) );
	return list;
}


void resize_py_list( py_list_t * list )
{
	list->copacity +=5;
	list->items = realloc( list->items, list->copacity * list->item_size );
	return;
}


void append_to_py_list( py_list_t * list, void * item )
{
	while( list->count >= list->copacity )
		resize_py_list( list );
	list->items[list->count] = item; 
	list->count++;
	return;
}
