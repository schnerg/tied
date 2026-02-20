#include "include/fileio.h"



void save_file( char * file_name, Lines_data * lines )
{
	FILE * fp = fopen( file_name, "w" );
 	Line_data * temp = lines->head;;
	for( int row = 0; row < lines->count; row++ )
	{
		for( int col =0; col < temp->count; col++ )
		{
			fputc( temp->data[col], fp );
		}
		temp = temp->next;
		if( row < lines->count )
			fputc( '\n', fp );
	}
	fclose( fp );
	return;
}


int read_file( Lines_data * lines, char * file_name )
{
	lines->list_of_lines = NULL;
	resize_list( lines );
	lines->head = calloc( 1, sizeof( Line_data ) );
	lines->count = 1;
	lines->head->next=NULL;
	lines->head->prev = NULL;
	lines->head->data = calloc( 50, sizeof( char ) );
	lines->head->count = 0;
	lines->head->copacity = 50;
	Line_data * prev = lines->head;
	Line_data * temp = lines->head;
	temp->to_display = NULL;
	lines->list_of_lines[lines->count - 1] = temp;
	resize_list( lines );
	FILE * fp = fopen( file_name, "r" );
	if( fp == NULL )
		return 1;
	int c;	
	register unsigned int i = 0;
	while( ( c = fgetc ( fp ) ) != EOF )
	{
		if(c=='\n')
		{
			update_line( temp );
			prev = temp;
			lines->count++;
			init_line( temp );
			resize_list( lines );
			temp = temp->next;
			temp->prev = prev;
			lines->list_of_lines[lines->count -1] = temp;
			i=0;
			continue;
		}
		if( temp->count == temp->copacity -1 )
			realloc_data( temp );
		temp->data[i] = c;
		temp->count++;
		i++;
	}
	lines->count--;
	if( lines->count == 0 )
		lines->count++;
	fclose(fp);
	return 0;
}
