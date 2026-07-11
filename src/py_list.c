#include "include/py_list.h"
#include <stdio.h>


// probably not the best solution!
// void pointers are probably not best practice.
py_list_t * init_py_list( int item_size )
{
	py_list_t * list = calloc( 1, sizeof( py_list_t ) );
	if( list == NULL )
		return NULL;
	list->item_size = item_size;
	list->copacity = 5;
	list->count = 0;
	list->items = calloc( list->copacity, item_size );
	if( list->items == NULL )
	{
		free( list );
		return NULL;
	}
	for( int i = 0; i < list->copacity; i++ )
		list->items[i] = NULL;
	return list;
}


int resize_py_list( py_list_t * list )
{
	i32 temp_copacity = list->copacity + 5;	
	void ** temp_items = realloc( list->items, temp_copacity * list->item_size );	
	if( temp_items != NULL )
	{
		list->copacity = temp_copacity;
		list->items = temp_items;
		for( int i = list->count; i < list->copacity; i++ )
			list->items[i] = NULL;
	}
	return 0;
}


int append_to_py_list( py_list_t * list, void * item )
{
	list->items[list->count++] = item; 
	while( list->count >= list->copacity )
		resize_py_list( list );
	return 0;
}
