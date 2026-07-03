#ifndef file_view_h
#define file_view_h


#include <stdbool.h>

#ifdef _WIN32
	#include <direct.h>
	#define getcwd _getcwd
#else
	#include <unistd.h>
#endif

#include "global.h"




typedef struct 
{
	char working_directory[1024];
}File_tree;


void toggle_file_tree();
#endif
