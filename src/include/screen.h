#ifndef screen_h
#define screen_h


#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#ifdef _WIN32
	#include <windows.h>
#elif __linux__
	#include <sys/ioctl.h>
	#include <termios.h>
#endif

#include "cursor.h"

typedef struct
{
	int rows,cols;
	#ifdef _WIN32
		HANDLE hstdin;
		DWORD original_mode;
	#elif __linux__
		struct termios orig_termios;
	#endif
}Window;

void disable_raw_mode( Window * window );
void enable_Raw_mode( Window * window );
bool get_window_size( Window * window );
void print_chars_to_screen( Buff * line_buff, Lines_data * lines, Cursor * c, Window * window, int line_nums );
void print_mode( Window * window, int mode, char * debug_message );
void adjust_yx_offsets( Cursor * c, Window * window, int line_nums, Buff * cbuff  );

#endif
