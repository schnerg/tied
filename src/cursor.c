#include "include/cursor.h"


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
			c->rx += ( TAB_STOP -1 ) - ( c->rx % TAB_STOP );
		c->rx++;
	}
	c->rx -= c->x_offset;
	c->rx += line_nums +1;	
	return;
}


void update_cursor( Cursor * c, Buff * line_buff )
{
	c->index = c->last_index;
	if( c->index > line_buff->count )
	{
		while( c->index > line_buff->count )
		{
			c->index--;
		}
	}
	c->x_offset = c->last_x_offset;
	c->y_offset = c->last_y_offset;
	return;
}


void print_cursor( Cursor * c, int mode )
{
	char bar[] = "\e[5 q"	;
	char block[] = "\e[1 q"	;
	if( mode == 0 ) // normal mode 
		write( STDOUT_FILENO, block, strlen( block ) );
	if( mode == 1 ) // insert mode
		write( STDOUT_FILENO, bar, strlen( bar ) );
	char buff[40];
	sprintf( buff, "\x1b[%d;%dH", c->y_index - c->y_offset + 1, c->rx + 1 );
	write( STDOUT_FILENO, buff, strlen( buff ) );
	return;
}

