#ifndef editor_h
#define editor_h


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>


#ifdef _WIN32
	#include <direct.h>
	#define make_directory( path ) _mkdir(path)
#elif __linux__
	#include <sys/stat.h>
	#include <sys/types.h>
	#define make_directory( path ) mkdir( path, S_IRWXU )// might need to change file permissions! :(
#endif



#include "global.h"
#include "py_list.h"
#include "screen.h"
#include "buffer.h"
#include "cursor.h"
#include "data.h"
#include "fileio.h"
#include "search.h"
#include "io.h"
#include "undo_redo.h"
#include "syntax.h"
#include "file_tree.h"




typedef struct
{
	File_tree tree;
	
	//undo_redo_stuffs
	Buff * line_buff;
	py_list_t * undo_stack;
	py_list_t * redo_stack;
	bool can_undo;	
	bool can_redo;	
	//int mode;
	MODE mode;
	
	Cursor cursor;
//line stuff
	Lines_data lines;	

	Files files;
	//char * file_name;

	int tabs;
	int tabs_space;
	int line_nums;
	//editor settings
	bool saved;
	char debug_message[40];
//window stuffs
	bool done;
	Window window;
}Editor;




void init( Editor * e );
void events( Editor * e );
void render( Editor * e );
void quit( Editor * e );


#endif
