#ifndef io_h
#define io_h

#include <ctype.h>
#include "screen.h"
#include "cursor.h"


char * get_input(  const char * message, Window * window );
char getch( Window * Window );

#endif
