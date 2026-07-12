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

char * get_input( char * input, const char * message, Window * window )
{
	i32 size = 0;
	char * temp = NULL;
	//write prompt
	if( input == NULL )	
	{
		size = 2;
		temp = calloc( size, sizeof( char ) );
	}
	else
		temp = input;
	//move cursor to bottom of screen
	i32 new_size = int_to_str_size( window->rows ) + int_to_str_size( 13 ) + strlen("\x1b[;H") + 1; 
	char * temp_data = calloc( new_size, sizeof( char ) );	
	snprintf( temp_data, new_size, "\x1b[%d;%dH", window->rows, 13 );
	write( 0, temp_data, strlen( temp_data ) );
	free( temp_data );
	
	write( 0, "\33[K", 3 );// clear row after cursor
	
	new_size = strlen( message ) + 1;
	temp_data = calloc( new_size, sizeof( char ) );	
	snprintf( temp_data, new_size ,"%s", message);
	write( 0, temp_data, strlen( temp_data) );
	free( temp_data );
	
	if( input != NULL )
	{
		new_size = strlen( input ) + 1;
		temp_data = calloc( new_size, sizeof( char ) );	
		snprintf( temp_data, new_size ,"%s", input );
		write( 0, temp_data, strlen( temp_data) );
		free( temp_data );
		size = new_size - 1;
	}
	
	int i = size;	
	char ch; 
	char back = '\b';
	while( ( ch = getch( window ) ) != 13 )	 // 13 is enter key
	{
		if( ch == 27 ) // escape 
		{
			free( temp );
			return NULL;
		}
		
		if ( ch == 127 )// backspace
		{
			if( i > 0)
			{
				i--;
				write(STDOUT_FILENO, &back, 1);
				write(STDOUT_FILENO, " ", 1);
				write(STDOUT_FILENO, &back, 1);
			}
		}
		else if( isprint( ch ) && !strchr( "<>:\"|? 	", ch ) )
		{
			write(STDOUT_FILENO, &ch, 1);
			if( i >= size - 1  )
			{
				size++;
				temp = realloc( temp, size * sizeof( char ) );
			}
			temp[i] = ch;
			i++;
		}
	}
	if( i >= size - 1  )
	{
		size++;
		temp = realloc( temp, size * sizeof( char ) );
	}

	temp[i] = '\0';
	return temp;
}

char * get_input2( char * input, const char * message, Window * window )
{
	i32 size = 0;
	char * temp = NULL;
	//write prompt
	if( input == NULL )	
	{
		size = 2;
		temp = calloc( size, sizeof( char ) );
	}
	else
		temp = input;
	//move cursor to bottom of screen
	i32 new_size = int_to_str_size( window->rows ) + int_to_str_size( 13 ) + strlen("\x1b[;H") + 1; 
	char * temp_data = calloc( new_size, sizeof( char ) );	
	snprintf( temp_data, new_size, "\x1b[%d;%dH", window->rows, 13 );
	write( 0, temp_data, strlen( temp_data ) );
	free( temp_data );
	
	write( 0, "\33[K", 3 );// clear row after cursor
	
	new_size = strlen( message ) + 1;
	temp_data = calloc( new_size, sizeof( char ) );	
	snprintf( temp_data, new_size ,"%s", message);
	write( 0, temp_data, strlen( temp_data) );
	free( temp_data );
	
	if( input != NULL )
	{
		new_size = strlen( input ) + 1;
		temp_data = calloc( new_size, sizeof( char ) );	
		snprintf( temp_data, new_size ,"%s", input );
		write( 0, temp_data, strlen( temp_data) );
		free( temp_data );
		size = new_size - 1;
	}
	
	int i = size;	
	char ch; 
	char back = '\b';
	while( ( ch = getch( window ) ) != 13 )	 // 13 is enter key
	{
		if( ch == 27 ) // escape 
		{
			free( temp );
			return NULL;
		}
		
		if ( ch == 127 )// backspace
		{
#ifdef _WIN32
			if( i > 3)
#elif __linux__
			if( i > 1)
#endif
			{
				i--;
				write(STDOUT_FILENO, &back, 1);
				write(STDOUT_FILENO, " ", 1);
				write(STDOUT_FILENO, &back, 1);
			}
		}
		else if( isprint( ch ) && !strchr( "<>:\"|? 	", ch ) )
		{
			write(STDOUT_FILENO, &ch, 1);
			if( i >= size - 1  )
			{
				size++;
				temp = realloc( temp, size * sizeof( char ) );
			}
			temp[i] = ch;
			i++;
		}
	}
	
	if( i >= size - 1  )
	{
		size++;
		temp = realloc( temp, size * sizeof( char ) );
	}

	temp[i] = '\0';
	return temp;
}


