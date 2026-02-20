#ifndef editor_h
#define editor_h



#include "common.h"
#include "syntax.h"

void init( Editor * e );
void events( Editor * e );
void render( Editor * e );
void quit( Editor * e );


#endif
