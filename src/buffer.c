#include "include/buffer.h"

Buff * init_buffer()
{
	Buff * buff = calloc( 1, sizeof( Buff ) );
	if( buff == NULL )
		return NULL;
	buff->count = 0;
	buff->dcount = 0;
	buff->copacity = 50;
	buff->contents = calloc( buff->copacity, sizeof( char ) );
	if( buff->contents == NULL )
	{
		free( buff );
		return NULL;
	}
	buff->to_display = NULL;
	buff->num_lines_changed = 0;
	buff->line_deleted = false;
	buff->line_added = false;
	buff->has_changed = false;
	buff->index = -1;
	return buff;	
}


void reset_buffer( Buff * buff )
{
	buff->count = 0;
	return;
}


int resize_buffer( Buff * buff )
{
	i32 temp_copacity = buff->copacity * 2;
	char * temp = realloc( buff->contents, temp_copacity * sizeof( char ) );
	if( temp != NULL )
	{
		buff->contents = temp;
		buff->copacity = temp_copacity;
		return 0;
	}
	return 1;
}


void append_to_buffer( Buff * buff, char * str, int size )
{
	while( buff->count + size >= buff->copacity - 1 )
		resize_buffer( buff );
	int j = buff->count;
	for( int i = 0; i < size; i++ )
		buff->contents[j++] = str[i];
	buff->contents[j] = '\0';
	buff->count += size;
	return;
}


void write_line_buffer_to_line( Line_data * line, Buff * buff )
{
	while( buff->count >= line->copacity )
		realloc_data( line );
	memcpy( line->data, buff->contents, buff->count );
	line->count = buff->count;
	update_line( line );
	return;
}


void update_line_buffer_td( Buff * line_buff )
{
	int i, j;
	int tabs = 0;
	
	if( line_buff->to_display != NULL )
		free( line_buff->to_display );
	
	for( i = 0; i < line_buff->count; i++ )
		if( line_buff->contents[i] == '\t' )
			tabs++;
	
	line_buff->to_display = malloc( ( line_buff->count + ( tabs * ( TAB_STOP - 1 )  ) + 1 ) * sizeof( char ) );

	if( line_buff->to_display == NULL )
		die( "update_line_buffer_td():failed to allocate memory for line buffer" );

	j=0;
	for( i = 0; i < line_buff->count; i++ )
	{
		if( line_buff->contents[i] == '\t' )
		{
			line_buff->to_display[j++] = ' ';
			while( j % TAB_STOP != 0 )
				line_buff->to_display[j++] = ' ';
		}
		else
			line_buff->to_display[j++] = line_buff->contents[i];
	}
	line_buff->to_display[j] = '\0';
	line_buff->dcount = j;
	return;
}


void update_line_buffer( Buff * line_buff, Line_data * line )
{
	reset_buffer( line_buff );	
	append_to_buffer( line_buff, line->data, line->count );
	update_line_buffer_td( line_buff );
	return;
}


Buff * init_line_buffer()
{
	Buff * line_buff = init_buffer();
	if( line_buff == NULL )
		die( "init_line_buffer(): failed to init line_buffer" );
	return line_buff;
}
