#ifndef common_h
#define common_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "py_list.h"
#include "screen.h"
#include "buffer.h"
#include "cursor.h"
#include "data.h"
#include "fileio.h"
#include "search.h"
#include "io.h"


typedef struct
{
	bool line_added;
	bool line_deleted;
	int line_num;
	int num_lines_changed;
	int index;
	Buff * data;
}Change;



typedef struct
{
	
	//undo_redo_stuffs
	Buff * line_buff;
	py_list_t * undo_stack;
	py_list_t * redo_stack;
	bool can_undo;	
	bool can_redo;	
	
	//int mode;
	enum modes
	{
		NORMAL,
		INSERT
	}mode;

	Cursor cursor;
//line stuff
	Lines_data lines;
	int tabs;
	int tabs_space;
	int line_nums;
	

	//editor settings
	bool saved;
	char debug_message[40];
	char file_name[255];
//window stuffs
	bool done;
	Window window;
}Editor;


#endif
