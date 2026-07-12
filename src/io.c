#include "include/io.h"


//I hate windows! >:(
char getch( Window * window ) 
{
	char c = -1;
	#ifdef _WIN32	
		DWORD numread;
		INPUT_RECORD input_record_buffer[2];
		ReadConsoleInput( window->hstdin, input_record_buffer, 2, &numread );
		for( long unsigned int i = 0; i < numread; i++ )
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

char * get_input( const char * message, Window * window )
{
	//write prompt
	i32 size = 2;
	char * temp = calloc( size, sizeof( char ) );
	
	i32 new_size = int_to_str_size( window->rows ) + int_to_str_size( 13 ) + strlen("\x1b[;H") + 1; 
	char * temp_data = calloc( new_size, sizeof( char ) );	
	snprintf( temp_data, new_size, "\x1b[%d;%dH", window->rows, 13 );
	write( 0, temp_data, strlen( temp_data ) );
	free( temp_data );
	
	write( 0, "\33[J", 3 );
	
	new_size = strlen( message ) + 1;
	temp_data = calloc( new_size, sizeof( char ) );	
	snprintf( temp_data, new_size ,"%s", message);
	write( 0, temp_data, strlen( temp_data) );
	free( temp_data );
	
	int i = 0;	
	char ch; 
	char back = '\b';
	while( ( ch = getch( window ) ) != 13 )	
	{
		if( ch == 27 ) // escape 
		{
			free( temp );
			return NULL;
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
			if( i >= size - 1 )
			{
				size++;
				temp = realloc( temp, size * sizeof( char ) );
			}
			temp[i] = ch;
			i++;
		}
	}
	temp[i] = '\0';
	return temp;
}


