#ifndef common_h
#define common_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>


typedef struct
{
	char * contents;
	int  count;
	int  copacity;
}Buff;







typedef struct Line_data
{
	char * data;
	int copacity;
	int count;
	struct Line_data * next;
	struct Line_data * prev;
}Line_data;


typedef struct
{
	Line_data * head;
	Line_data ** list_of_lienes;
	int count;
	int copacity;
}Lines_data;





typedef struct
{
	int rx;
	int last_index;
	int index;
	int y_index;
	int last_x_offset;
	int last_y_offset;
	int x_offset;
	int y_offset;
}Cursor;


typedef struct
{
	int rows,cols;
	struct termios orig_termios;
}Window;

typedef struct
{


	Cursor cursor;
	


//line stuff
	Lines_data lines;
	int tabs;
	int tabs_space;
	int line_nums;
	//editor settings
	int mode;
	bool saved;
	char debug_message[40];
	char file_name[255];
//window stuffs
	bool done;
	Window window;
}Editor;

#endif
