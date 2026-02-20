#include "include/search.h"

void search( Window * window, Cursor * c, int mode, char * debug_message, Lines_data * lines )
{
	strcpy( debug_message, "" );
	print_mode( window, mode, debug_message );
	print_cursor( c, mode );
	// move curor down to menu bar
	char buff[40];
	sprintf( buff, "\x1b[%d;%dH", window->rows, 13 );
	write( 0, buff, strlen( buff ) );
	int i = 0;	
	char ch; 
	char back = '\b';
	while( ( ch = getch( window ) ) != 13 && i < 40 )	
	{
		if( ch == 27 ) // escape 
		{
			strcpy( debug_message, "" );
			return;
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
		else if( isprint( ch ) )
		{
			write(STDOUT_FILENO, &ch, 1);
			buff[i] = ch;
			i++;
		}
	}
	buff[i] = '\0';
	// search
	char * index = NULL;
	for( int i = c->y_index; i < lines->count; i++ )
	{
		index = strstr( lines->list_of_lines[i]->data, buff );
		if( index != NULL )
		{
			c->y_index = i; 
			c->index = index - lines->list_of_lines[i]->data;
			c->last_index = c->index;
			c->last_x_offset = c->x_offset;
			break;	
		}
	}
	if( index == NULL )
	{
		strcpy( debug_message, "reached bottom: searching from top" );
		for( int i = 0; i < c->y_index; i++ )
		{
			index = strstr( lines->list_of_lines[i]->data, buff );
			if( index != NULL )
			{
				c->y_index = i; 
				c->index = index - lines->list_of_lines[i]->data;
				c->last_index = c->index;
				c->last_x_offset = c->x_offset;
				break;	
			}
		}
		if( index == NULL )
		{
			char message[40];
			snprintf( message, 40, "Pattern not found: %s", buff );
			strcpy( debug_message, message );
		}
	}
	return;
}
