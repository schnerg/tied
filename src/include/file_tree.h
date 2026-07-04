#ifndef file_view_h
#define file_view_h


#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
	#include <direct.h>
	#define getcwd _getcwd
#else
	#include <unistd.h>
#endif

#include "data.h"
#include "global.h"


typedef struct 
{
	char working_directory[1024];
	
	Lines_data lines; 
	
	i32 count;
	i32 copacity;
	i32 y_offset;
}File_tree;


void toggle_file_tree();
void init_file_tree( File_tree * tree );
#endif
