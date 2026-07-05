#include "include/fileio.h"


bool does_file_exist( File_tree * tree, char * file_name )
{
	for( int i = 1; i < tree->lines.count; i++ )
		if( strcmp( tree->lines.list_of_lines[i]->data, file_name ) == 0 )
			return true;
	return false;
}


int get_file_name( char * file_name, i32 size, File_tree * tree, Window * window, char * debug_message )
{
	//write prompt
	char buff[size];
	sprintf( buff, "\x1b[%d;%dH", window->rows, 13 );
	write( 0, buff, strlen( buff ) );
	sprintf( buff, "\33[J" );
	write( 0, buff, strlen( buff ) );
	sprintf( buff, "Save file as: ");
	write( 0, buff, strlen( buff ) );
//read input
	int i = 0;	
	char ch; 
	char back = '\b';
retry:
	while( ( ch = getch( window ) ) != 13 && i < size )	
	{
		if( ch == 27 ) // escape 
		{
			strcpy( debug_message, " File not saved" );
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
	
	if( does_file_exist( tree, buff ) )
	{
		strcpy( debug_message, "\33[41mFILE ALREADY EXISTS!\33[0m");
		return 1;
	}

	strcpy( file_name, buff );
	return 0;
}


void free_file( Lines_data * lines)
{
	Line_data * prev = NULL;
	Line_data * temp = lines->head;
	for( int i =0; i< lines->count; i++ )
	{
		prev = temp;
		temp = temp->next;
		free( prev->data );
		free( prev );
	}
	lines->count = 0;
	return;
}


int save_file( char * file_name, Lines_data * lines, File_tree * tree, Window * window, char * debug_message )
{
	if( file_name[0] == '\0' )
		if( get_file_name( file_name, 255, tree, window, debug_message ) )
			return 1;
	
	errno = 0;
	FILE * fp = fopen( file_name, "w" );
	if( fp == NULL )
	{ 
		die( "save_file(): Failed to open file."); 
		return 1;
	}
	Line_data * temp = lines->head;	
	bool written = false;
	for( int row = 0; row < lines->count; row++ )
	{
		for( int col = 0; col < temp->count; col++ )
		{
			fputc( temp->data[col], fp );
			written = true;
		}
		temp = temp->next;
		if( row < lines->count && written == true )
			fputc( '\n', fp );
	}
	fclose( fp );
	return 0;
}


int load_file( Lines_data * lines, char * file_name )
{
	lines->list_of_lines = NULL;
	resize_list( lines );
	lines->head = calloc( 1, sizeof( Line_data ) );
	lines->count = 1;
	lines->head->next = NULL;
	lines->head->prev = NULL;
	lines->head->data = calloc( 50, sizeof( char ) );
	lines->head->count = 0;
	lines->head->copacity = 50;
	Line_data * prev = lines->head;
	Line_data * temp = lines->head;
	temp->to_display = NULL;
	lines->list_of_lines[lines->count - 1] = temp;
	resize_list( lines );

	if( file_name[0] == '\0')
		return 1;

	FILE * fp = fopen( file_name, "r" );
	if( fp == NULL )
		return 1;
	
	int c;	
	register unsigned int i = 0;
	while( ( c = fgetc ( fp ) ) != EOF )
	{
		if( c == '\n' )
		{
			update_line( temp );
			prev = temp;
			lines->count++;
			init_line( temp );
			resize_list( lines );
			temp = temp->next;
			temp->prev = prev;
			lines->list_of_lines[lines->count -1] = temp;
			i = 0;
			continue;
		}
		if( temp->count == temp->copacity -1 )
			realloc_data( temp );

		if( isprint( c ) || c == '\t' )
		{
			temp->data[i] = c;
			temp->count++;
			i++;
		}
	}
	lines->count--;
	if( lines->count == 0 )
		lines->count++;
	fclose(fp);
	return 0;
}

