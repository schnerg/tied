#include "include/io.h"


//I hate windows! >:(
char getch( Window * window ) 
{
	char c = -1;
	int nread;
	#ifdef _WIN32	
		DWORD numread;
		INPUT_RECORD input_record_buffer[2];
		ReadConsoleInput( e->window.hstdin, input_record_buffer, 2, &numread );
		for( int i = 0; i < numread; i++ )
			if( input_record_buffer[i].Event.KeyEvent.bKeyDown )
			{
				c = input_record_buffer[i].Event.KeyEvent.uChar.AsciiChar; 
				if( c == 0 && input_record_buffer[i].Event.KeyEvent.wVirtualKeyCode != 17 )
					c = input_record_buffer[i].Event.KeyEvent.wVirtualKeyCode; 
			}
	#elif __linux__
		nread = read( STDIN_FILENO, &c, 1);
	#endif
  return c;
}
