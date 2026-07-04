#include "include/data.h"


void update_line( Line_data * line )
{
	int i, j;
	int tabs = 0;
	if( line->to_display != NULL )
		free( line->to_display );

	for( i = 0; i < line->count; i++ )
		if( line->data[i] == '\t' )
			tabs++;

	line->to_display = malloc( ( line->count + ( tabs * ( TAB_STOP - 1 ) ) + 1 ) * sizeof( char ) );
	if( line->to_display == NULL ) die("update_line(): malloc failed to allocate memory!\n");

	j = 0;
	for( i = 0; i < line->count; i++ )
	{
		if( line->data[i] == '\t' )
		{
			line->to_display[j++] = ' ';
			while( j % TAB_STOP != 0 )
				line->to_display[j++] = ' ';
		}
		else
			line->to_display[j++] = line->data[i];
	}
	line->to_display[j] = '\0';
	line->dcount = j;
	return;
}


void realloc_data( Line_data * line )
{
	i32 new_copacity = line->copacity * 2;
	char * temp_data = realloc( line->data, new_copacity * sizeof( char ) );
	if( temp_data != NULL )
	{
		line->copacity = new_copacity;
		line->data = temp_data;
	}
	return;
}


void init_line( Line_data * temp )
{
	temp->next = calloc( 1, sizeof( Line_data ) );
	if( temp->next == NULL ) die( "init_line(): failed to calloc memory for next line." );
	temp->next->data = calloc( 50, sizeof( char ) );
	if( temp->next->data == NULL ) die( "init_line(): failed to calloc memory for next lines data." );
	temp->next->copacity = 50;
	temp->next->count = 0;
	temp->next->dcount = 0;
	temp->next->is_dir = false;
	temp->head = NULL;
	temp->next->to_display = NULL;
	temp->next->next = NULL;
	//temp->next->prev = NULL;
	return;
}



void resize_list( Lines_data * lines )
{
	
	if( lines->list_of_lines == NULL )
	{
		lines->copacity = 10;
		lines->list_of_lines = calloc( lines->copacity, sizeof( Line_data* ) );
		if(lines->list_of_lines == NULL ) die( "resize_list(): failed to allocate memory for list_of_lines." );
		return;
	}
	
	if( lines->count >= lines->copacity -1 )
	{
		lines->copacity *= 2;
		lines->list_of_lines = realloc( lines->list_of_lines, lines->copacity * sizeof( Line_data * ) );
		if(lines->list_of_lines == NULL ) die( "resize_list(): failed to reallocate memory for list_of_lines." );
	}
	return;	
}


void update_list_of_lines( Lines_data * lines )
{
	resize_list( lines );
	Line_data * temp = lines->head;
	for( int i = 0; i < lines->count; i++ )
	{
		lines->list_of_lines[i] = temp;
		temp = temp->next;
	}
	return;	
}
