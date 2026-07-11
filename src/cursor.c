#include "include/cursor.h"


i32 int_to_str_size( i32 i )
{
	i32 n = 0;
	while( i > 0 )
	{
		i = i / 10;
		n++;
	}
	if( n == 0 )
		n++;
	return n;
}




void init_cursor( Cursor * c )
{
	c->rx = 0;
	c->y_index = 0;
	c->last_index = 0;
	c->index = 0;
	c->last_x_offset = 0;	
	c->last_y_offset = 0;	
	c->x_offset = 0;
	c->y_offset = 0;
	return;
}


void index_to_rx( Cursor * c, Buff * cbuff, int line_nums )
{
	c->rx = 0;
	for( int i = 0; i < c->index; i++ )
	{ 
		if( cbuff->contents[i] == '\t' )
			c->rx += ( TAB_STOP - 1 ) - ( c->rx % TAB_STOP );
		c->rx++;
	}
	c->rx -= c->x_offset;
	if( toggle_line_nums )
		c->rx += line_nums + 1;	
	if( file_tree_toggle == true )	
		c->rx += FILE_TREE_WIDTH; 

	return;
}


void update_cursor( Cursor * c, Buff * line_buff )
{
	c->index = c->last_index;
	if( c->index > line_buff->count )
		while( c->index > line_buff->count )
			c->index--;
	c->x_offset = c->last_x_offset;
	c->y_offset = c->last_y_offset;
	return;
}


void print_cursor( Cursor * c, int mode )
{
	char bar[] = "\e[5 q"	;
	char block[] = "\e[1 q"	;
	if( mode == 0 || mode == 2 ) // 0 normal mode 2 file tree mode
		write( STDOUT_FILENO, block, strlen( block ) );
	if( mode == 1 ) // insert mode
		write( STDOUT_FILENO, bar, strlen( bar ) );
	i32 new_size = strlen ("\x1b[;H") + int_to_str_size( c->y_index - c->y_offset + 1 ) + int_to_str_size( c->rx + 1 ) + 1;
	char * temp_data = calloc( new_size, sizeof( char ) );
	snprintf( temp_data, new_size, "\x1b[%d;%dH", c->y_index - c->y_offset + 1, c->rx + 1 );
	write( STDOUT_FILENO, temp_data, strlen( temp_data ) );
	free( temp_data );
	return;
}



