#ifndef common_h
#define common_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#ifdef _WIN32
	#include <windows.h>
#elif __linux__
	#include <sys/ioctl.h>
	#include <termios.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#include "py_list.h"



typedef struct
{
	char * contents;
	bool line_deleted;
	bool line_added;
	bool has_changed;
	int index;
	int  count;
	int  copacity;
	int num_lines_changed;
}Buff;


typedef struct
{
	bool line_added;
	bool line_deleted;
	int line_num;
	int num_lines_changed;
	int index;
	Buff * data;
}Change;


typedef struct Line_data
{
	char * data;
	int copacity;
	int count;
	struct Line_data * next;
	struct Line_data * prev;
}Line_data;


typedef struct
{
	Line_data * head;
	Line_data ** list_of_lienes;
	int count;
	int copacity;
}Lines_data;


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


typedef struct
{
	int rows,cols;
	struct termios orig_termios;
}Window;


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
