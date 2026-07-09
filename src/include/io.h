#ifndef io_h
#define io_h

#include <ctype.h>
#include "screen.h"


int get_input( char * input, const char * message, i32 size, File_tree * tree, i32 rows, Window * window);
char getch( Window * Window );

#endif
