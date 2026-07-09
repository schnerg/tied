#ifndef data_h
#define data_h

#include <stdlib.h>
#include <stdbool.h>
#include "global.h"


typedef struct Line_data
{
	char * data;
	char * to_display;
	i32 dcount;
	i32 dcopacity;
	i32 copacity;
	i32 count;
	bool deleted;
	bool is_dir;
	bool expanded;
	struct Line_data * head;
	struct Line_data * next;
	struct Line_data * prev;
}Line_data;


typedef struct
{
	Line_data * head;
	Line_data ** list_of_lines;
	i32 expanded_count;
	i32 count;
	i32 copacity;
}Lines_data;


void update_line( Line_data * line );
void realloc_data( Line_data * temp );
void update_list_of_lines( Lines_data * lines );
void init_line( Line_data * temp );
void resize_list( Lines_data * lines );

#endif
