#ifndef cursor_h
#define cursor_h

#define TAB_STOP 4

#include <string.h>
#include <stdio.h>

#include "buffer.h"
#include "screen.h"

typedef struct
{
	int rx;
	int last_index;
	int index;
	int y_index;
	int last_x_offset;
	int last_y_offset;
	int x_offset;
	int y_offset;
}Cursor;

void adjust_yx_offsets( Cursor * c, Window * window, int line_nums, Buff * cbuff  );
void print_cursor( Cursor * c, int mode );
void update_cursor( Cursor * c, Buff * line_buff );
void index_to_rx( Cursor * c, Buff * cbuff, int line_nums );

#endif
