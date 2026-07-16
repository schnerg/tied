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
#include "buffer.h"
#include "cursor.h"
#include "data.h"
#include "fileio.h"
#include "search.h"
#include "io.h"
#include "screen.h"
//#include "syntax.h"
#include "file_tree.h"




typedef struct
{
	//syntax
	Syntax syntax;

	File_tree tree;
	//int mode;
	MODE mode;
	char * clipboard;
	Cursor cursor;
//line stuff
	Buff * line_buff;
	Lines_data lines;	
	Files files;
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
