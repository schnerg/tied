#ifndef undo_redo_h
#define undo_redo_h


#include "py_list.h"
#include "screen.h"
#include "buffer.h"
#include "cursor.h"
#include "data.h"



typedef struct
{
	bool line_added;
	bool line_deleted;
	int line_num;
	int num_lines_changed;
	int index;
	Buff * data;
}Change;


void free_change( Change * change );
void push_insert_to_undo_stack( py_list_t * undo_stack, Buff * line_buff, Line_data * line, Cursor * c );
void push_new_line_to_undo_stack( py_list_t * undo_stack, Buff * line_buff, Cursor * c, Line_data * line );
void push_del_line_to_undo_stack( py_list_t * undo_stack, Buff * line_buff, Cursor * c, Line_data * line  );
void push_change_to_undo_stack( py_list_t * undo_stack, py_list_t * redo_stack, Buff * line_buff, bool * can_undo );
void redo_change( py_list_t * redo_stack, py_list_t * undo_stack, Cursor * c, Lines_data * lines, Buff * line_buff, Window * window, bool * can_redo, bool * can_undo );
void undo_change( py_list_t * redo_stack, py_list_t * undo_stack, Cursor * c, Lines_data * lines, Buff * line_buff, Window * window, bool * can_redo, bool * can_undo );


#endif
