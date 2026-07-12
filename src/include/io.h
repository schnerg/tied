#ifndef io_h
#define io_h

#include <ctype.h>
#include "screen.h"
#include "cursor.h"


char * get_input( char * input, const char * message, Window * window );
char * get_input2( char * input, const char * message, Window * window );
//char * get_input(  const char * message, Window * window );
char getch( Window * Window );

#endif
