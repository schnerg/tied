#ifndef data_h
#define data_h

#define TAB_STOP 4
#include <stdlib.h>

typedef struct Line_data
{
	char * data;
	char * to_display;
	int dcount;
	int copacity;
	int count;
	struct Line_data * next;
	struct Line_data * prev;
}Line_data;


typedef struct
{
	Line_data * head;
	Line_data ** list_of_lines;
	int count;
	int copacity;
}Lines_data;

void update_line( Line_data * line );
void realloc_data( Line_data * temp );
void update_list_of_lines( Lines_data * lines );
void init_line( Line_data * temp );
void resize_list( Lines_data * lines );

#endif
