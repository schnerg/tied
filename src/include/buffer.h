#ifndef buffer_h
#define buffer_h

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

typedef struct
{
	char * contents;
	char * to_display;
	bool line_deleted;
	bool line_added;
	bool has_changed;
	int index;
	int  dcount;
	int  count;
	int  copacity;
	int num_lines_changed;
}Buff;


void resize_buffer( Buff * buff );
void append_to_buffer( Buff  * buff, char * str, int size );
void reset_buffer( Buff * buff );
Buff * init_buffer();
void write_line_buffer_to_line( Line_data * line, Buff * buff );

#endif
