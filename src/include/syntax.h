#ifndef syntax_h
#define syntax_h

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include "buffer.h"
#include "colors.h"

typedef struct
{
	int i;	
	char c;
	char * data;
	int len;
	bool update;
	bool is_string;
	bool is_comment;
}lexer_t;


typedef struct
{
	lexer_t * lexer;
}Syntax;



void syntax_highlighting(  Buff * buffer, Syntax * syntax,char * str, const int len);
#endif
