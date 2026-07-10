#include "include/io.h"


//I hate windows! >:(
char getch( Window * window ) 
{
	char c = -1;
	#ifdef _WIN32	
		DWORD numread;
		INPUT_RECORD input_record_buffer[2];
		ReadConsoleInput( window->hstdin, input_record_buffer, 2, &numread );
		for( int i = 0; i < numread; i++ )
			if( input_record_buffer[i].Event.KeyEvent.bKeyDown )
			{
				c = input_record_buffer[i].Event.KeyEvent.uChar.AsciiChar; 
				if( c == 0 && input_record_buffer[i].Event.KeyEvent.wVirtualKeyCode != 17 )
					c = input_record_buffer[i].Event.KeyEvent.wVirtualKeyCode; 
			}
	#elif __linux__
		window->no_warnings_int++;
		read( STDIN_FILENO, &c, 1);
	#endif
  return c;
}

int get_input( char * input, const char * message, i32 size, Window * window)
{
	//write prompt
	char buff[1024];
	sprintf( buff, "\x1b[%d;%dH", window->rows, 13 );
	write( 0, buff, strlen( buff ) );
	sprintf( buff, "\33[J" );
	write( 0, buff, strlen( buff ) );
	sprintf( buff, "%s", message);
	write( 0, buff, strlen( buff ) );

//read input
	int i = 0;	
	char ch; 
	char back = '\b';
	while( ( ch = getch( window ) ) != 13 && i < size )	
	{
		if( ch == 27 ) // escape 
		{
			return 1;
		}
		if ( ch == 127 )// backspace
		{
			if(i > 0)
			{
				i--;
				write(STDOUT_FILENO, &back, 1);
				write(STDOUT_FILENO, " ", 1);
				write(STDOUT_FILENO, &back, 1);
			}
		}
		else if( isprint( ch ) && !strchr( "<>:\"/\\|?", ch ) )
		{
			write(STDOUT_FILENO, &ch, 1);
			buff[i] = ch;
			i++;
		}
	}
	buff[i] = '\0';
	strcpy( input, buff );
	return 0;
}


