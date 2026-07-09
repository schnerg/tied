#ifndef variables_h
#define variables_h

#include <stdbool.h>


typedef enum
{
	NORMAL,
	INSERT,
	FLTREE	
}MODE;

extern bool file_tree_toggle;
extern bool syntax;
extern bool toggle_line_nums;



#endif
