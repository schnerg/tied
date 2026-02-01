#ifndef py_list_h
#define py_list_h

#include <stdlib.h>

typedef struct
{
	void ** items;
	int item_size;
	int count;
	int copacity;
}py_list_t;


py_list_t * init_py_list( int item_size );
void resize_py_list( py_list_t * list );
void append_to_py_list( py_list_t * list, void * item );


#endif
