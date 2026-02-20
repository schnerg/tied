#ifndef fileio_h
#define fileio_h


#include <stdio.h>
#include "data.h"


void save_file( char * file_name, Lines_data * lines );
int read_file( Lines_data * lines, char * file_name );
#endif
