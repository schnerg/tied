#include "include/fileio.h"


bool does_file_exist( File_tree * tree, char * file_name )
{
 	refresh_file_tree( tree );
	for( int i = 1; i < tree->lines.count; i++ )
		if( strcmp( tree->lines.list_of_lines[i]->data, file_name ) == 0 )
			return true;
	return false;
}

// -1 = file does no exist 
// 0 = no permissions
// 1 = read permissions
// 2 = read and write permissions
i32 file_permissions( const char * file_name )
{
	if( access( file_name, 0 ) == -1 ) // does_file_exist
		return -1;
	if( access( file_name, 6 ) == 0 ) // read and write permission
		return 2;
	if( access( file_name, 4 ) == 0 ) // just read
		return 1;
	return 0;
}






char * get_file_name( File_tree * tree, Window * window, char * debug_message )
{
	
	i32 size = 2;
	char * temp = calloc( size, sizeof( char ) );
	

	i32 new_size = int_to_str_size( window->rows ) + int_to_str_size( 13 ) + strlen("\x1b[;H") + 1; 
	char * temp_data = calloc( new_size, sizeof( char ) );	
	snprintf( temp_data, new_size, "\x1b[%d;%dH", window->rows, 13 );
	write( 0, temp_data, strlen( temp_data ) );
	free( temp_data );
	write( 0, "\33[J", 3 );
	
	new_size = strlen( "Save file as: " ) + 1;
	temp_data = calloc( new_size, sizeof( char ) );	
	snprintf( temp_data, new_size ,"Save file as: " );
	write( 0, temp_data, strlen( temp_data) );
	free( temp_data );

	//write prompt
	//read input
	int i = 0;	
	char ch; 
	char back = '\b';
	while( ( ch = getch( window ) ) != 13 )	
	{
		if( ch == 27 ) // escape 
		{
			strcpy( debug_message, " File not saved" );
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
	if( does_file_exist( tree, temp ) )
	{
		strcpy( debug_message, "\33[41mFILE ALREADY EXISTS!\33[0m");
		free( temp );
		return NULL;
	}
	return temp;
}


void free_file( Lines_data * lines)
{
	Line_data * prev = NULL;
	Line_data * temp = lines->head;
	while( temp != NULL )
	{
		prev = temp;
		temp = temp->next;
		free( prev->data );
		if( prev->to_display != NULL )
			free( prev->to_display );
		free( prev );
	}
	free( lines->list_of_lines );
	lines->count = 0;
	return;
}


int save_file( Files * file, Lines_data * lines, File_tree * tree, Window * window, char * debug_message )
{
	if( file->file_name == NULL )
		if( (file->file_name = get_file_name( tree, window, debug_message ) ) == NULL )
			return 1;
	/*
	if( is_directory( file_name ) )
	{
		free( file_name );
		return 1;
	}
*/
	errno = 0;
	FILE * fp = fopen( file->file_name, "w" );
	if( fp == NULL )
	{ 
		strcpy( debug_message, "cannot save file, permission denied");
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


int load_file( Lines_data * lines, FILE * file, char * file_name, char * debug_message )
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

	if( file_name == NULL )
		return 1;

	if( file_permissions( file_name ) == 1 )
		strcpy( debug_message, ": FILE READ ONLY" );
	
	
	if( file == NULL )
	{
		FILE * fp = fopen( file_name, "r" );
		if( fp == NULL )
			return 1;
		file = fp;
	}
	int c;	
	register unsigned int i = 0;
	while( ( c = fgetc ( file ) ) != EOF )
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
	fclose( file );
	return 0;
}

