#ifndef screen_h
#define screen_h

#include <unistd.h>

#ifdef _WIN32
	#include <windows.h>
#elif __linux__
	#include <sys/ioctl.h>
	#include <termios.h>
#endif

#include <stdbool.h>


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




#endif
