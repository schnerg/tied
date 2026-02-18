#include "include/screen.h"


void enter_alt_screen()
{
	write( STDOUT_FILENO, "\x1b[?1049h", 8 );
	return;
}


void leave_alt_screen()
{
	write( STDOUT_FILENO, "\x1b[?1049l", 8 );
	return;
}


void disable_raw_mode( Window * window )
{
	leave_alt_screen();
	#ifdef _WIN32
		SetConsoleMode( window->hstdin, window->original_mode );
		HANDLE out = GetStdHandle( STD_OUTPUT_HANDLE );
		SetConsoleMode( out, window->original_mode );
	#elif __linux__
		tcsetattr( STDIN_FILENO, TCSAFLUSH, &window->orig_termios );
	#endif
	return;
}


void enable_Raw_mode( Window * window )
{
	enter_alt_screen();
	#ifdef _WIN32
		HANDLE in = GetStdHandle( STD_INPUT_HANDLE );
		HANDLE out = GetStdHandle( STD_OUTPUT_HANDLE );
		window->hstdin = in;
		DWORD vertual_terminal, raw; 
		GetConsoleMode( out, &vertual_terminal );
		GetConsoleMode( in, &window->original_mode );
		raw = window->original_mode & ~( ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS );
		vertual_terminal |=( ENABLE_VIRTUAL_TERMINAL_PROCESSING |  ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | DISABLE_NEWLINE_AUTO_RETURN) ;
		SetConsoleMode( out, vertual_terminal );
		SetConsoleMode( in, raw );
	#elif __linux__
		tcgetattr( STDIN_FILENO, &window->orig_termios );
		struct termios raw = window->orig_termios;
		raw.c_iflag &= ~( BRKINT | ICRNL | INPCK | ISTRIP | IXON );
		raw.c_oflag &= ~( OPOST );
		raw.c_cflag |= ( CS8 );
		raw.c_lflag &= ~( ECHO | ICANON | IEXTEN | ISIG );
		raw.c_cc[VMIN] = 0;
		raw.c_cc[VTIME] = 1;
		tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw);
	#endif
	return;
}


bool get_window_size( Window * window )
{
	int rows, cols;
	#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
	 	GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );
		cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	#elif __linux__
		struct winsize w;
		ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
		rows =  w.ws_row;
		cols = w.ws_col;
	#endif
		if( window->rows != rows || window->cols != cols )
		{
			window->rows =  rows;
			window->cols = cols;
			return true;  
		}
	return false;  
} 

