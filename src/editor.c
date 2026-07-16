#include "include/editor.h"

void update( Editor * e  );
void add_new_line( Editor * e, char * data, int size_of_data );


void init_editor_settings( Editor * e )
{
	e->saved = true;
	e->mode = NORMAL;
	return;
}



void init( Editor * e )
{
	enable_Raw_mode( &e->window );
	init_editor_settings( e );
 	init_file_tree( &e->tree );
	get_window_size( &e->window );
	strcpy( e->debug_message, "" );
	e->done = false;
	e->tabs = 0;
	e->clipboard = NULL;
	e->syntax.lexer = NULL;
	i32 i = load_file( &e->lines, NULL, e->files.file_name, e->debug_message );
	if( i == 1 )
		strcpy( e->debug_message, ": new buffer created" );
	init_cursor( &e->cursor );
	e->line_buff = init_line_buffer();
	update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );	
//	init_undo_redo_stacks( e );
	update( e );
	index_to_rx( &e->cursor, e->line_buff, e->line_nums );

	if( file_tree_toggle && e->files.file_name == NULL )
		e->mode = FLTREE;
	return;	
}




void update( Editor * e )
{
	e->line_nums = int_to_str_size( e->lines.count );
	return;
}


void render_2( Editor * e )
{
	update_cursor( &e->cursor, e->line_buff );
	index_to_rx( &e->cursor, e->line_buff, e->line_nums );
	print_cursor( &e->cursor, e->mode );
	return;
}


void render( Editor * e )
{
	update( e );
	update_cursor( &e->cursor, e->line_buff );
	index_to_rx( &e->cursor, e->line_buff, e->line_nums );
	adjust_yx_offsets( &e->cursor, &e->window, e->line_nums, e->line_buff );
 	print_chars_to_screen( e->line_buff, &e->syntax, &e->lines, &e->cursor, &e->window, e->line_nums, &e->tree, e->files.file_name );
	print_mode( &e->window, e->mode, e->debug_message );
	if( file_tree_toggle && e->mode == FLTREE )
		print_cursor( &e->tree.cursor, e->mode );
	else
		print_cursor( &e->cursor, e->mode );
	
	if( e->syntax.lexer != NULL )
		if( e->syntax.lexer->update )
		{
		}
	return;
}

void render_3( Editor * e )
{
	update( e );
	update_cursor( &e->cursor, e->line_buff );
	index_to_rx( &e->cursor, e->line_buff, e->line_nums );
	adjust_yx_offsets( &e->cursor, &e->window, e->line_nums, e->line_buff );
 	update_display_line( e->line_buff, &e->syntax, &e->cursor, &e->window, e->line_nums, e->files.file_name );
	print_mode( &e->window, e->mode, e->debug_message );
	if( file_tree_toggle && e->mode == FLTREE )
		print_cursor( &e->tree.cursor, e->mode );
	else
		print_cursor( &e->cursor, e->mode );
	
	if( e->syntax.lexer != NULL )
		if( e->syntax.lexer->update )
		{
			render( e );
		}
	return;
}



bool _delete_file( const char * file_name )
{
	if( remove( file_name ) == 0 )
		return 0;
	else return 1;
}


i32 delete_file( Editor * e, File_tree * tree, Window * window, char * file_name )
{
	i32 result = 0;
	char * input = NULL;
	i32 len = strlen( tree->lines.list_of_lines[tree->cursor.y_index]->to_display ) + strlen( tree->lines.list_of_lines[tree->cursor.y_index]->data ) + 2;
	char * temp_data = NULL;

	temp_data = calloc( len, sizeof( char ) );
	snprintf( temp_data, len , "%s/%s", tree->lines.list_of_lines[tree->cursor.y_index]->to_display, tree->lines.list_of_lines[tree->cursor.y_index]->data );
	
	if( tree->lines.list_of_lines[tree->cursor.y_index]->is_dir )
	{
		if( ( input = get_input( NULL, "This is a directory type 'yes' to delete: ", window ) ) == NULL ) 
			goto done;
		str_to_lower( input );
		if( strcmp( input, "yes") == 0 )
			result = _delete_file( temp_data );
		else 
			goto done;
	}
	else
	{
		if( ( input = get_input(  NULL, "Delete file? N/y: ", window ) ) == NULL ) 
			goto done;
		str_to_lower( input );
		if( strcmp( input, "") == 0 || strcmp( input, "n") == 0 )
			goto done;
		else if( strcmp( input, "y") == 0 )
			result = _delete_file( temp_data );
	}
	if( result == 0 )
	{
		free( input );
		input = NULL;
		refresh_file_tree( tree );
		if( file_name == NULL)
			goto done;
		if( strcmp( temp_data, file_name ) == 0 )
		{
			if( ( input = get_input( NULL,"File loaded, delete buffer? N/y: ", window ) ) == NULL ) 
				goto done;
			str_to_lower( input );
			if( strcmp( input, "") == 0 || strcmp( input, "n") == 0 )
				goto done;
			else if( strcmp( input, "y") == 0 )
			{
				//e->mode = NORMAL;
				free_file( &e->lines );	
				free( e->files.file_name );
				e->files.file_name = NULL;
				load_file( &e->lines, NULL, e->files.file_name, e->debug_message );
				init_cursor( &e->cursor );
				update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );		
				//free_undo_redo_stacks( e );
				//init_undo_redo_stacks( e );
				update( e );
				index_to_rx( &e->cursor, e->line_buff, e->line_nums );
				init_editor_settings( e );
			}
		}
	}
done:
	free( temp_data );
	if( input != NULL )
		free( input );
	return result;
}




i32 make_directory_recursive( const char * path )
{
	char * _path = strdup( path );
	char * p = _path;
	int result = -1;
	char temp = 0;
#ifdef _WIN32
	for( p = _path + 3; *p != '\0'; p++ )
#elif __linux__
	for( p = _path + 1; *p != '\0'; p++ )
#endif
	{
		errno = 0;
		if( *p == '/' || *p == '\\' )
		{
			temp = *p;
			*p = '\0';
			if( make_directory( _path ) != 0)
			{
				if( errno != EEXIST )
				goto done;
			}
			*p = temp;
		}
	}
	errno = 0;
	if( make_directory( _path ) != 0 )
		if( errno != EEXIST )
			goto done;
	result = 0;
done:
	free( _path );
	return result;
}


int _create_file(  char * file )
{
	FILE * fp = NULL;
	fp = fopen( file, "w");
	if( fp == NULL )
		return -1; 
	fclose( fp );
	return 0;
}


i32 create_file( Editor * e, File_tree * tree, Window * window )
{
	// get file name and path
	i32 new_len = strlen( tree->working_directory ) + 1;
	char * working_directory = calloc( new_len, sizeof(char ) );
	snprintf( working_directory, new_len, "%s", tree->working_directory );
	char * input = NULL;
	if( ( input = get_input2( working_directory , "", window ) ) == NULL ) 
		return 1;
	
	new_len = strlen( input );
	char * temp = strdup(input);
	// checking if directory
	bool is_dir = false;
	if( input[new_len-1] == '/' || input[new_len-1] == '\\' )
		is_dir = true;
	
	if( is_dir )
	{
		if( make_directory_recursive( input ) != -1 )
		{
			strcpy( e->debug_message, "Directory created successfuly!" ); 
			refresh_file_tree( tree );
			goto done;
		}
		else
		{

		}
	}
	else
	{
		i32 i = strlen( input );
		while( input[i] != '\\' && input[i] != '/' )
			i--;
		input[i] = '\0';
		if( make_directory_recursive( input ) != -1 )
		{
			if( _create_file( temp ) != -1 )
			{
				strcpy( e->debug_message, "File created successfuly!" ); 
				refresh_file_tree( tree );
				goto done;
			}
			else
			{
				strcpy( e->debug_message, "Could not create file!" ); 
				goto done;
			}
		}
		else
		{
			strcpy( e->debug_message, "Could not create directory!" ); 
			goto done;
		}
	}
	done:
	free( temp );
	free( input );
	return 0;
}


void insert_char_to_buff( Editor * e, char c )
{
	bool update_screen = false; 
	//e->can_redo = false;
	e->saved = false;
	Buff * temp = e->line_buff;
	temp->count++;
	if( e->line_buff->index == -1 )
			e->line_buff->index = e->cursor.index;
	e->line_buff->has_changed = true;

	while( temp->count >= temp->copacity )
		resize_buffer( temp );
	char old;
	int i = e->cursor.index;
	while( i < temp->count )
	{	
		old = temp->contents[i];
		temp->contents[i++] = c;
		c = old;
	}
	e->cursor.index++;
	index_to_rx( &e->cursor, e->line_buff, e->line_nums );
	if( e->cursor.rx >= e->window.cols  - 1 )
	{
		e->cursor.x_offset += e->window.cols / 2 ;
		update_screen = true;
	}	
	strcpy( e->debug_message, "" );
	update_line_buffer_td( temp );
	e->cursor.last_index = e->cursor.index;
	e->cursor.last_x_offset= e->cursor.x_offset;
	if( update_screen )
		render( e );
	else
		render_3(e);
	return;
}




void remove_line( Editor * e )
{
	
	Line_data * temp = e->lines.list_of_lines[e->cursor.y_index];
	Line_data * line_to_appended_to = temp->prev;
	Line_data * next_line = temp->next;
	line_to_appended_to->next = next_line;
	if( next_line != NULL )
		next_line->prev = line_to_appended_to;

	int size = line_to_appended_to->count + temp->count;

	i32 i;
	char * buff = calloc( size, sizeof( char ) );
	for( i = 0; i < line_to_appended_to->count; i++ )
		buff[i] = line_to_appended_to->data[i];
	for( i32 j = 0; j < temp->count; j++ )
	{
		buff[i] = temp->data[j];
		i++;
	}
	reset_buffer( e->line_buff );
	append_to_buffer( e->line_buff, buff, size );
	e->lines.count--;
	free( buff );
	free( temp->data );
	free( temp->to_display );
	free( temp );
	return;
}



void backspace( Editor * e )
{
	bool update_screen = false;
	if( e->cursor.index > 0 )
	{
		e->line_buff->has_changed = true;
		e->saved = false;
		if( e->line_buff->index == -1 )
			e->line_buff->index = e->cursor.index;
		e->line_buff->has_changed = true;
		e->cursor.index--;	
		e->cursor.last_index = e->cursor.index;
		Buff * temp = e->line_buff;
		temp->count--;	
		for( int i = e->cursor.index; i < temp->count; i++ )
			temp->contents[i] = temp->contents[i+1];
		index_to_rx( &e->cursor, e->line_buff, e->line_nums );
		if( e->cursor.rx <= ( e->line_nums + 1 ) && e->cursor.x_offset > 0 )
			e->cursor.x_offset -= e->window.cols - ( e->line_nums + 1 ); 
		if( e->cursor.x_offset < 0 )
			e->cursor.x_offset = 0;
		e->cursor.last_x_offset = e->cursor.x_offset;
		strcpy( e->debug_message, "" );
	}	
	else if( e->cursor.y_index > 0 )
	{
		e->line_buff->has_changed = true;
		e->saved = false;
		e->cursor.index = e->lines.list_of_lines[e->cursor.y_index-1]->count;
		e->cursor.last_index = e->cursor.index;	
		write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
		remove_line( e );
		update_list_of_lines( &e->lines );
		e->cursor.y_index--;	
		if( e->cursor.index - e->cursor.x_offset >= e->window.cols - 1)
		{
			int shift = ( e->window.cols - ( e->cursor.index - e->cursor.x_offset) ) + 1;
			e->cursor.x_offset += shift;
		}
		strcpy( e->debug_message, "" );
		update_screen = true;
	}
	update_line_buffer_td( e->line_buff );
	if( update_screen )
		render( e );
	else
		render_3( e );
	return;
}



void add_new_line( Editor * e, char * data, int size_of_data )
{
	Line_data * current_line  = e->lines.list_of_lines[e->cursor.y_index];
	Line_data * next_line = current_line->next;
	Line_data * new_line = calloc( 1, sizeof( Line_data ) );
	current_line->next = new_line;
	new_line->prev = current_line;
	new_line->next = next_line;
	if( next_line != NULL )
		next_line->prev = new_line;
	new_line->copacity = size_of_data + 50;
	new_line->count = size_of_data; 
	new_line->data = calloc( new_line->copacity, sizeof( char ) );
	reset_buffer( e->line_buff );	
	append_to_buffer( e->line_buff, data, size_of_data );	
	return;
}




void enter_key( Editor * e )
{
	write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
	e->line_buff->has_changed = true;
	Line_data * temp = e->lines.list_of_lines[e->cursor.y_index];
	i32 size = temp->count - e->cursor.index;
	char * buff = calloc( size, sizeof( char ) );
	int j = temp->count - e->cursor.index;
	add_new_line( e, &temp->data[e->cursor.index], j );
	free( buff );
	temp->count = e->cursor.index;
	update_line( temp );
	e->lines.count++;
	e->saved = false;
	update_list_of_lines( &e->lines );
	
	e->cursor.y_index++;
	if( e->cursor.y_index >= e->window.rows - 2 )
		e->cursor.y_offset++;	
	strcpy( e->debug_message, "" );
	e->cursor.last_y_offset = e->cursor.y_offset;
	e->cursor.index = 0;
	e->cursor.last_index = 0;
	e->cursor.x_offset = 0;
	update_line_buffer_td( e->line_buff );	
	render( e );
	return;
}






void move_cursor_up( Editor * e )
{
	if( e->cursor.y_index > 0 )
	{
		bool update_screen = false;
		if( e->mode == INSERT && e->line_buff->has_changed == true )
		{
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			e->line_buff->has_changed = false;
		}
		
		e->cursor.y_index--;
		
		if( e->cursor.y_offset > 0 && e->cursor.y_index - e->cursor.y_offset < 0 )
		{
			update_screen = true;
			e->cursor.y_offset--;
		}
		e->cursor.last_y_offset = e->cursor.y_offset;

		update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
		
		if( update_screen )
			render( e );
		else
			render_2( e );
	}
	return;
}


void move_cursor_down( Editor * e )
{
	if( e->cursor.y_index < e->lines.count - 1 )
	{
		bool update_screen = false;
		if( e->mode == INSERT && e->line_buff->has_changed == true )
		{
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			e->line_buff->has_changed = false;
		}
		e->cursor.y_index++;

		if( e->cursor.y_index - e->cursor.y_offset == e->window.rows -1 )	
		{
			e->cursor.y_offset++;
			update_screen = true;
		}
		e->cursor.last_y_offset = e->cursor.y_offset;
		
		update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
		if( update_screen )
			render( e );
		else
			render_2( e );
	}
	return;
}


void move_cursor_left( Editor * e )
{
	if( e->cursor.index > 0 )
	{
		bool update_screen = false;
		if( e->mode == INSERT && e->line_buff->has_changed == true )
		{
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			e->line_buff->has_changed = false;
		}
		e->cursor.index--;
		index_to_rx( &e->cursor, e->line_buff, e->line_nums );

		if( e->cursor.rx <= ( e->line_nums + 1 ) && e->cursor.x_offset > 0 )
		{
			e->cursor.x_offset -= e->window.cols - ( e->line_nums + 1 ); 
			update_screen = true;
		}

		if( e->cursor.x_offset < 0 )
			e->cursor.x_offset = 0;
		
		e->cursor.last_index = e->cursor.index;
		e->cursor.last_x_offset = e->cursor.x_offset;
		if( update_screen )
			render(e);
		else
			render_2( e );
	}
	return;
}


void move_cursor_right( Editor * e )
{
	if( e->cursor.index < e->line_buff->count )
	{
		bool update_screen = false;
		if( e->mode == INSERT && e->line_buff->has_changed == true )
		{
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			e->line_buff->has_changed = false;
		}
		
		e->cursor.index++;
		index_to_rx( &e->cursor, e->line_buff, e->line_nums );
		int shift = ( e->window.cols - ( e->cursor.index - e->cursor.x_offset) ) + 1;
		if( e->cursor.rx >= e->window.cols  - 1 )
		{
			e->cursor.x_offset += shift;
			update_screen = true;
		}
		e->cursor.last_index = e->cursor.index;
		e->cursor.last_x_offset = e->cursor.x_offset;
		if( update_screen )
			render(e);
		else
			render_2( e );
	}
	return;
}


#ifdef _WIN32
	#define BACKSPACE 8
void events_move_cursor_insert_windows( Editor * e, int c ) 	
{
	switch( c )
	{
		case 37: move_cursor_left( e ); break;
		case 38: move_cursor_up( e ); break;
		case 39: move_cursor_right( e ); break;
		case 40: move_cursor_down( e ); break;
	}
	return;
}

#elif __linux__
	#define BACKSPACE 127
bool events_move_cursor_insert_linux( Editor * e )
{
	bool is_arrow_key = false;
	char temp;
	if( ( temp = getch( &e->window ) ) != -1 )
	{
		is_arrow_key = true;
		temp = getch( &e->window );
		switch( temp )
		{
			case 'A': move_cursor_up( e );break;
			case 'B': move_cursor_down( e );break;
			case 'C': move_cursor_right( e );break;
			case 'D': move_cursor_left( e );break;
		}
	}
	return is_arrow_key;
}
#endif



void events_insert( Editor * e )
{
	int c = getch( &e->window );
	#ifdef _WIN32	
		if( c <= 40 && c >= 37 )
		{
			events_move_cursor_insert_windows( e, c );
			return;
		}
	#endif
	switch( c )
	{
		case 27: 
		{		
			#ifdef _WIN32	
				e->mode = NORMAL;
				if( e->line_buff->has_changed )
				{
					write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
					e->line_buff->has_changed = false;
				}
				update_cursor( &e->cursor, e->line_buff );
				print_mode( &e->window, e->mode, e->debug_message );
				print_cursor( &e->cursor, e->mode );
			#elif __linux__
				if( !events_move_cursor_insert_linux( e ) )
				{
					e->mode = NORMAL;
					if( e->line_buff->has_changed )
					{
						write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
						e->line_buff->has_changed = false;
					}
					update_cursor( &e->cursor, e->line_buff );
					print_mode( &e->window, e->mode, e->debug_message );
					print_cursor( &e->cursor, e->mode );
			//		e->line_buff->has_changed = false;
				}
			#endif
		}break;
		case 13: enter_key( e );break; //enter key
		case BACKSPACE: backspace( e ); break;
		default:
		{
			if( isprint( c ) || c== '\t' )
				insert_char_to_buff( e, c );
		}break;
	}
	return;
}


char await_input( Window * window )
{
	char underline[] = "\e[4 q";
	char block[] = "\e[1 q";
	write( STDOUT_FILENO, underline, strlen( underline ) );
	char c;
	while( true )
	{
		c = getch( window );
		if( c != -1 )
			goto done;
	}
	done:
	write( STDOUT_FILENO, block, strlen( block ) );
	return c;
}

int char_to_int( char c )
{
	return c - 48;
}



void paste( Editor * e )
{	
	if( e->clipboard == NULL )
		return;
	//write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
	int len = strlen( e->clipboard );
	for( int i = 0; i < len; i++ )
	{
		if( e->clipboard[i] == '\n' )
		{
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			e->cursor.index = e->lines.list_of_lines[e->cursor.y_index]->count;
			enter_key( e );
			continue;
		}
		insert_char_to_buff( e, e->clipboard[i] );
	}
	write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
	e->line_buff->has_changed = false;
	return;
}


void copy_line( Editor * e, int line_num )
{
	int len, i, j;
	if( line_num < e->lines.count )
	{
		if( e->clipboard == NULL )
		{
			e->clipboard = calloc( e->lines.list_of_lines[line_num]->count + 3, sizeof( char ) );
			e->clipboard[0] = '\n';
			e->clipboard[1] = '\0';
			len = strlen( e->clipboard );
		}
		else
		{
			e->clipboard = realloc( e->clipboard, ( strlen( e->clipboard) + e->lines.list_of_lines[line_num]->count + 2  )  * sizeof( char ) );
			len = strlen( e->clipboard );
			e->clipboard[len++] = '\n';
			e->clipboard[len] = '\0';
		}
		 i = len;
		 j = 0;
		for( j = 0;  j < e->lines.list_of_lines[line_num]->count; j++ )
		{
			e->clipboard[i++] = e->lines.list_of_lines[line_num]->data[j];
		}
		e->clipboard[i] = '\0';
	}
	return;
}


void copy_lines( Editor * e, int n )
{
	if( e->clipboard != NULL )
	{
		free( e->clipboard );
		e->clipboard = NULL;
	}
	for( int i = 0; i < n; i++ )
		copy_line( e, e->cursor.y_index + i );
	return;
}


void delete_line( Editor * e, int line_num )
{
	Line_data * temp = NULL;	
	if( line_num == 0 && e->lines.count == 1 )
	{
		e->lines.list_of_lines[line_num]->count = 0;
		reset_buffer( e->line_buff );	
		update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
		render(e);
		return;
	}
	if( line_num == 0 && e->lines.count > 1 )
	{
		temp = e->lines.list_of_lines[line_num];
		e->lines.head = temp->next;
		e->lines.head->prev = NULL;
	}
	else
	{
		temp = e->lines.list_of_lines[line_num];
		temp->prev->next = temp->next;
		if( temp->next != NULL )
			temp->next->prev = temp->prev;
	}
	free( temp->data );
	free( temp->to_display );
	free( temp );
	e->lines.count--;

	if( e->cursor.y_index > e->lines.count -1 )
		e->cursor.y_index--;
	update_list_of_lines( &e->lines );
	reset_buffer( e->line_buff );	
	update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
	render(e);
	return;
}


void delete_lines( Editor * e, int n )
{
	while( e->cursor.y_index + n > e->lines.count )
		n--;
	copy_lines( e, n );
	for( int i = 0; i < n; i++ )
		delete_line( e, e->cursor.y_index );
	return;
}

void comand_mode( Editor * e, char c )
{
	if( isdigit( c ) )
	{
		int n = char_to_int( c );
		while( true )
		{
			c = await_input( &e->window );
			if( !isdigit( c ) )
			{
				if( c != 'y' && c != 'd' )
					goto done;
				
				if( c == 'y' )	
				{
					if( ( c = await_input( &e->window ) ) != 'y' ) 
						goto done;
					copy_lines( e, n );
					goto done;
				}

				if( c == 'd' )	
				{
					if( ( c = await_input( &e->window ) ) != 'd' ) 
						goto done;
					delete_lines( e, n );
					goto done;
				}
			}
			else if( isdigit( c ) )
				n *= 10 + char_to_int( c );
		}
	}

	switch( c )
	{
		case 'y':
		{
			c = await_input( &e->window );
			if( c == 'y' )
			{
				if( e->clipboard != NULL )
				{
					free( e->clipboard );
					e->clipboard = NULL;
				}
				copy_line( e, e->cursor.y_index );
			}
		}break;
		case 'd':
		{
			c = await_input( &e->window );
			if( c == 'd' )
			{
				if( e->clipboard != NULL )
				{
					free( e->clipboard );
					e->clipboard = NULL;
				}
				copy_line( e, e->cursor.y_index );
				delete_line( e, e->cursor.y_index );
			}
		}break;
	}
done:
	return;
}



void events_normal( Editor * e )
{	
	char c = getch( &e->window );
	if( isdigit( c ) )
		comand_mode( e, c );
	#ifdef _WIN32	
		if( c <= 40 && c >= 37 )
		{
			events_move_cursor_insert_windows( e, c );
			return;
		}
	#endif
	switch( c )
	{
		case '/':
		{
 			search( &e->window, &e->cursor, e->mode, e->debug_message, &e->lines );
			adjust_yx_offsets( &e->cursor, &e->window, e->line_nums, e->line_buff );
			update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
			render( e );
		}break;
		
		case 'o':
		{
			//e->cursor.index = 0;
			e->cursor.index = e->lines.list_of_lines[e->cursor.y_index]->count;
			enter_key( e );
			e->mode = INSERT;
			update_cursor( &e->cursor, e->line_buff );
			print_mode( &e->window, e->mode, e->debug_message );
			print_cursor( &e->cursor, e->mode );
		}break;
		
		case 'i':
		{
			e->mode = INSERT;
			update_cursor( &e->cursor, e->line_buff );
			print_mode( &e->window, e->mode, e->debug_message );
			print_cursor( &e->cursor, e->mode );
		}break;
		#ifdef __linux__
			case 27: // escape
			{
				events_move_cursor_insert_linux( e );	
			}break;
		#endif
		case CTRL_KEY( 'r' ):
		{
 			//redo_change( e->redo_stack, e->undo_stack, &e->cursor, &e->lines, e->line_buff, &e->window, &e->can_redo,  &e->can_undo );
			//render( e );
		}break;
		case 'p':
		{
			paste( e );
		}break;
		case 'd':
		{
			comand_mode( e, c );
		}break;

		case 'y':
		{
			comand_mode( e, c );
		}break;

		case CTRL_KEY( 'u' ):
		{
 			//undo_change( e->redo_stack, e->undo_stack, &e->cursor, &e->lines, e->line_buff, &e->window, &e->can_redo,  &e->can_undo );
			//render( e );
		}break;
		case CTRL_KEY( 's' ):
		{
			//write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			if( save_file( &e->files, &e->lines, &e->tree, &e->window, e->debug_message ) == 0 )
			{
				strcpy( e->debug_message, ": file saved" );
				e->saved = true;
			}
			print_mode( &e->window, e->mode, e->debug_message );
			print_cursor( &e->cursor, e->mode );
		}break;
		case CTRL_KEY( 'q' ):
		{
			if( e->saved == true )
				e->done = true;
			else
				strcpy( e->debug_message, ": no write since last change");
		}break;
		case CTRL_KEY( 'x' ):
		{
			e->done = true;
		}break;
		case CTRL_KEY( 'f' ):
		{
			toggle_file_tree();
			render( e );
		}break;
		case CTRL_KEY( 'h' ):
		{
			if( file_tree_toggle )
			{
				e->mode = FLTREE;
				print_mode( &e->window, e->mode, e->debug_message );
				print_cursor( &e->tree.cursor, e->mode );
			}
		}break;
		case 13:// enter key
		{
			move_cursor_down( e );
		}break;
		case 'k': move_cursor_up( e );	 break;
		case 'j': move_cursor_down( e ); break; 
		case 'h': move_cursor_left( e ); break;
		case 'l': move_cursor_right( e );break;
	}
	return;
}


/*
void free_undo_redo_stacks( Editor * e )
{
	for( int i = 0; i < e->undo_stack->copacity; i++ )
	{
		if( e->undo_stack->items[i] != NULL )
			free_change( e->undo_stack->items[i] );
	}
	free( e->undo_stack->items );
	free( e->undo_stack );

	for( int i = 0; i < e->redo_stack->copacity; i++ )
	{
		if( e->redo_stack->items[i] != NULL )
			free_change( e->redo_stack->items[i] );
	}
	free( e->redo_stack->items );
	free( e->redo_stack );
	return;
}
*/

void events_file_tree( Editor * e ) 
{
	int c = getch( &e->window );
	
	#ifdef _WIN32	
		if( c <= 40 && c >= 37 )
		{
			switch( c )
			{
				case 38:
				{
					if( e->tree.cursor.y_index > 1 )
					{
						e->tree.cursor.y_index--;
						strcpy( e->debug_message, "" );
						render( e );
						print_cursor( &e->tree.cursor, e->mode );
					}
				}break;
				case 40:
				{
					if( e->tree.cursor.y_index < e->tree.lines.expanded_count )
					{
						e->tree.cursor.y_index++;
						strcpy( e->debug_message, "" );
						render( e );
						print_cursor( &e->tree.cursor, e->mode );
					}
				}break;
			}
			return;
		}
	#endif
	switch( c )
	{
		#ifdef __linux__
			case 27: // escape
			{
				char temp;
				if( ( temp = getch( &e->window ) ) != -1 )
				{
					temp = getch( &e->window );
					switch( temp )
					{
						case 'A':
						{
							if( e->tree.cursor.y_index > 1 )
							{
								e->tree.cursor.y_index--;
								strcpy( e->debug_message, "" );
								render( e );
								print_cursor( &e->tree.cursor, e->mode );
							}
						};break;
						case 'B':
						{
							if( e->tree.cursor.y_index < e->tree.lines.expanded_count )
							{
								e->tree.cursor.y_index++;
								strcpy( e->debug_message, "" );
								render( e );
								print_cursor( &e->tree.cursor, e->mode );
							}
						}break;
					}
				}
			}break;
		#endif
		case CTRL_KEY( 'l' ):
		{
			e->mode = NORMAL;
			print_mode( &e->window, e->mode, e->debug_message );
			print_cursor( &e->cursor, e->mode );
		}break;
		case 'k': 
		{
			if( e->tree.cursor.y_index > 1 )
			{
				e->tree.cursor.y_index--;
				
				if( e->tree.cursor.y_offset > 0 && e->tree.cursor.y_index - e->tree.cursor.y_offset < 1 )
				e->tree.cursor.y_offset--;

				strcpy( e->debug_message, "" );
				render( e );
				print_cursor( &e->tree.cursor, e->mode );
			}
		}break;
		case 'j':
		{					
			if( e->tree.cursor.y_index < e->tree.lines.expanded_count )
			{
				e->tree.cursor.y_index++;
				if( ( e->tree.cursor.y_index - e->tree.cursor.y_offset ) == e->window.rows - 1 )
					e->tree.cursor.y_offset++;
				strcpy( e->debug_message, "" );
				render( e );
				print_cursor( &e->tree.cursor, e->mode );
			}
		}break;
		case 13:// enter key
		{

			if( e->tree.lines.list_of_lines[e->tree.cursor.y_index]->is_dir )
			{
				if( !e->tree.lines.list_of_lines[e->tree.cursor.y_index]->expanded )
					expand_tree_at_point_of_cursor( &e->tree, e->debug_message );
				else
				{
					e->tree.lines.list_of_lines[e->tree.cursor.y_index]->expanded = false;
					update_file_tree_items( &e->tree, &e->tree.lines );
				}
				render( e );
				print_cursor( &e->tree.cursor, e->mode );
			}
			else if( !e->tree.lines.list_of_lines[e->tree.cursor.y_index]->is_dir )
			{
				i32 new_len = 0;	
				char * temp_data = NULL;
				new_len = strlen( e->tree.lines.list_of_lines[e->tree.cursor.y_index]->to_display ) + strlen( e->tree.lines.list_of_lines[e->tree.cursor.y_index]->data ) + 2;
				temp_data = calloc( new_len, sizeof( char ) );
				snprintf( temp_data, new_len, "%s/%s", e->tree.lines.list_of_lines[e->tree.cursor.y_index]->to_display, e->tree.lines.list_of_lines[e->tree.cursor.y_index]->data );

				if( e->files.file_name == NULL )
					goto open_file;

				else if( strcmp( e->files.file_name, temp_data ) != 0 )
					goto open_file;
				else 
				{
					free( temp_data );
					break;
				}
open_file:
				if( e->saved == false )
				{
					strcpy( e->debug_message, "FILE NOT SAVED!");
					free( temp_data );
					render( e );
					break;
				}
				
				if( file_permissions( temp_data ) != -1 ) 
				{
					FILE * file = fopen( temp_data, "r" );
					if( file == NULL )
					{
						free( temp_data );
						break;
					}
					else
					{
						free_file( &e->lines );
						if( load_file( &e->lines, file, temp_data, e->debug_message ) != 1 )
						{

							if( e->syntax.lexer != NULL )
								free( e->syntax.lexer );
							e->syntax.lexer = NULL;
							e->mode = NORMAL;
							if( e->files.file_name != NULL )
								free( e->files.file_name );
							e->files.file_name = temp_data;
							init_cursor( &e->cursor );
							update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );		
							//free_undo_redo_stacks( e );
							//init_undo_redo_stacks( e );
							update( e );
							index_to_rx( &e->cursor, e->line_buff, e->line_nums );
							init_editor_settings( e );
							render( e );
						}
						break;
					}
				}
				free( temp_data );
			}
		}break;
		case CTRL_KEY( 'r' ):
		{
			refresh_file_tree( &e->tree );
			render( e );
			print_cursor( &e->tree.cursor, e->mode );
		}break;
		
		case CTRL_KEY( 'a' ):
		{
			create_file( e, &e->tree, &e->window );
			render( e );
			print_cursor( &e->tree.cursor, e->mode );
		}break;

		case CTRL_KEY( 'd' ):
		{
			if( delete_file( e, &e->tree, &e->window, e->files.file_name ) != 0 )
				strcpy( e->debug_message, "Could not Delete file or directory!" );
			render( e );
			print_cursor( &e->tree.cursor, e->mode );
		}break;
		case CTRL_KEY( 'c' ):
		{
			change_dir_at_point_of_cursor( &e->tree, e->debug_message );
			render( e );
			print_cursor( &e->tree.cursor, e->mode );
		}break;

	}
	return;
}


void events( Editor * e )
{
	switch( e->mode )
	{
		case NORMAL:
		{
			events_normal( e );
		}break;
		case INSERT:
		{
			events_insert( e );
		}break;
		case FLTREE:
		{
			events_file_tree( e );
		}break;
	}
	if( get_window_size( &e->window ) )
	{
		adjust_yx_offsets( &e->cursor, &e->window, e->line_nums, e->line_buff );
		if( file_tree_toggle )
			adjust_cursor_offset( &e->tree, e->window.rows );
		render( e );
		if( e->mode == FLTREE )
			print_cursor( &e->tree.cursor, e->mode );
	}
	return;
}


void quit( Editor * e )
{
	// free files
	Line_data * prev = NULL;
	Line_data * temp = e->lines.head;
	while( temp != NULL )
	{
		prev = temp;
		temp= temp->next;
		free( prev->data );
		if( prev->to_display != NULL )
			free( prev->to_display );
		free( prev );
	}
	free( e->lines.list_of_lines );
	free( e->files.file_name );
	// free file tree
	free_file_tree( e->tree.lines.head, e->tree.lines.count, 0 );
	free( e->tree.lines.head );
	free( e->tree.lines.list_of_lines );
	free( e->tree.working_directory );
	//clear screen!
	write( STDOUT_FILENO, "\x1b[2J", 4 );
	write( STDOUT_FILENO, "\x1b[H", 3 );
	//free_undo_redo_stacks( e );
	free( e->line_buff->contents );
	if( e->line_buff->to_display != NULL )
		free( e->line_buff->to_display );
	free( e->line_buff );
	disable_raw_mode( &e->window );
	// free clipboard;
	if( e->clipboard != NULL )
		free( e->clipboard );
	if( e->syntax.lexer != NULL )
		free( e->syntax.lexer );
	return;
}
