#ifndef fileio_h
#define fileio_h

#include <stdio.h>
#include <unistd.h>
#include "data.h"
#include "screen.h"
#include "io.h"


typedef struct
{
	Lines_data * files;
	int count;
}Files;


//void save_file( char * file_name, Lines_data * lines );
//int save_file( char * file_name, Lines_data * lines );
bool is_directory( char * file_name );
void free_file( Lines_data * lines);
int save_file( char * file_name, Lines_data * lines, File_tree * tree, Window * window, char * debug_message );
int load_file( Lines_data * lines, char * file_name, char * debug_message );
#endif
