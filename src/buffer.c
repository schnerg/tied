#include "include/buffer.h"

Buff * init_buffer()
{
	Buff * buff = calloc( 1, sizeof( Buff ) );
	buff->count = 0;
	buff->dcount = 0;
	buff->copacity = 50;
	buff->contents = calloc( buff->copacity, sizeof( char ) );
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


void resize_buffer( Buff * buff )
{
	buff->copacity *= 2;
	buff->contents = realloc( buff->contents, buff->copacity * sizeof( char ) );
	return;
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
