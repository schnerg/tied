#ifndef fileio_h
#define fileio_h

#include <stdio.h>

#ifdef _WIN32
	#include <io.h>
	#include <windows.h>
	#define access _access
#elif __linux__
	#include <unistd.h>
#endif

#include "data.h"
#include "screen.h"
#include "io.h"


typedef struct
{
	char * file_name;
	Lines_data * files;
	int count;
}Files;


//void save_file( char * file_name, Lines_data * lines );
//int save_file( char * file_name, Lines_data * lines );
i32 file_permissions( const char * file_name );
bool is_directory( const char * file_name );
void free_file( Lines_data * lines);
int save_file( Files * file_name, Lines_data * lines, File_tree * tree, Window * window, char * debug_message );
int load_file( Lines_data * lines, FILE * file, char * file_name, char * debug_message );
#endif
