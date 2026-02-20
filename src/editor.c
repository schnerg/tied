#include "include/editor.h"


#define CTRL_KEY(k) ((k) & 0x1f)


void update( Editor * e  );
void add_new_line( Editor * e, char * data, int size_of_data );
void move_cursor_down( Editor * e );
void remove_line( Editor * e );
void remove_line_2( Editor * e );
void update_line_buffer( Buff * line_buff, Line_data * line );;



void init_editor_settings( Editor * e )
{
	e->saved = true;
	e->mode = NORMAL;
	return;
}


void free_change( Change * change )
{
	if( change->data != NULL )
	{
		free( change->data->contents );
		free( change->data );
	}
	free( change );
	return;
}

void free_change_if_needed( py_list_t * list )
{	
	if( list->items[list->count] != NULL )
		free_change( list->items[list->count] );
	return;
}


void push_insert_to_redo_stack( Editor * e )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->data = init_buffer();
	append_to_buffer( change->data, e->lines.list_of_lines[e->cursor.y_index]->data, e->lines.list_of_lines[e->cursor.y_index]->count );
	change->line_num = e->cursor.y_index;
	append_to_py_list( e->redo_stack, change );
	return;
}


void push_insert_to_undo_stack( Editor * e )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->data = init_buffer();
	if( e->line_buff->line_deleted == false && e->line_buff->line_added == false )
		change->index = e->line_buff->index;
	append_to_buffer( change->data, e->lines.list_of_lines[e->cursor.y_index]->data, e->lines.list_of_lines[e->cursor.y_index]->count );
	change->line_num = e->cursor.y_index;
	free_change_if_needed( e->undo_stack );
	append_to_py_list( e->undo_stack, change );
	e->line_buff->num_lines_changed++;
	return;
}


void push_add_line_to_redo_stack( Editor * e )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->line_added = true;
	change->line_num = e->cursor.y_index;
	change->data = init_buffer();
	append_to_buffer( change->data, e->lines.list_of_lines[e->cursor.y_index]->data, e->lines.list_of_lines[e->cursor.y_index]->count );
	append_to_py_list( e->redo_stack, change );
	return;
}


void push_new_line_to_undo_stack( Editor * e )
{
	e->line_buff->has_changed = true;
	e->line_buff->line_added = true;
	Change * change = calloc( 1, sizeof( Change ) );
	change->line_added = true;
	change->data = init_buffer();
	change->data->index = e->cursor.index;
	append_to_buffer( change->data, e->lines.list_of_lines[e->cursor.y_index]->data, e->lines.list_of_lines[e->cursor.y_index]->count );
	change->line_num = e->cursor.y_index;
	free_change_if_needed( e->undo_stack );
	append_to_py_list( e->undo_stack, change );
	e->line_buff->num_lines_changed++;
	return;
}


void push_undo_del_line_to_redo_stack( Editor * e )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->line_deleted = true;
	change->line_num = e->cursor.y_index;
	append_to_py_list( e->redo_stack, change );
	return;
}


void push_del_line_to_undo_stack( Editor * e )
{
	e->line_buff->has_changed = true;
	e->line_buff->line_deleted = true;
	Change * change = calloc( 1, sizeof( Change ) );
	change->line_deleted = true;
	change->data = init_buffer();
	change->data->index = e->line_buff->index;
	append_to_buffer( change->data, e->lines.list_of_lines[e->cursor.y_index]->data, e->lines.list_of_lines[e->cursor.y_index]->count );
	change->line_num = e->cursor.y_index;
	free_change_if_needed( e->undo_stack );
	append_to_py_list( e->undo_stack, change );
	e->line_buff->num_lines_changed++;
	return;
}


void push_change_to_redo_stack( Editor * e, int num_lines_changed )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->num_lines_changed = num_lines_changed;
	append_to_py_list( e->redo_stack, change );
	return;
}


void push_change_to_undo_stack( Editor * e, bool has_changed, bool line_deleted, bool line_added )
{
	if( has_changed )
	{
		Change * change = calloc( 1, sizeof( Change ) );
		if( line_deleted )
			change->line_deleted = true;
		if( line_added )
			change->line_added = true;
		change->num_lines_changed = e->line_buff->num_lines_changed;
		change->index = e->line_buff->index;
		free_change_if_needed( e->undo_stack );
		append_to_py_list( e->undo_stack, change );
		e->redo_stack->count = 0;
		e->line_buff->num_lines_changed = 0;
		e->line_buff->has_changed = false;
		e->line_buff->line_deleted = false;
		e->line_buff->line_added = false;
		e->line_buff->index = -1;
		e->can_undo = true;
	}
	return;
}




void redo_change( Editor * e )
{
	if( e->can_redo )
	{
		Change * change = e->redo_stack->items[e->redo_stack->count - 1];
		int changes = change->num_lines_changed;
		e->undo_stack->count++;
		e->redo_stack->count--;
		free_change( change );
		for( int i = 0; i < changes; i++ )
		{
			change = e->redo_stack->items[e->redo_stack->count - 1];
			if( change->line_deleted == false && change->line_added == false )
			{
				e->cursor.y_index = change->line_num;
				write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], change->data );
				free_change( change );
			}
			else if( change->line_deleted )
			{
				e->cursor.y_index = change->line_num + 1;
				remove_line_2( e );
				update_list_of_lines( &e->lines );
				e->cursor.y_index--;
				free_change( change );
			}
			//why does this work!?
			//still no idea why this works!
			else if( change->line_added )
			{
				write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], change->data );
				add_new_line( e, change->data->contents, change->data->count );
				e->lines.count++;
				update_list_of_lines( &e->lines );
				e->cursor.y_index++;
				if( e->cursor.y_index - e->cursor.y_offset == e->window.rows -1 )	
					e->cursor.y_offset++;
				e->cursor.last_y_offset = e->cursor.y_offset;
				free_change( change );
			}
			e->undo_stack->count++;
			e->redo_stack->count--;
		}
		if( e->redo_stack->count == 0 )
			e->can_redo = false;
		e->can_undo = true;
		update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
		render( e );
	}
	return;
}


void undo_change( Editor * e )
{
	if( e->can_undo )
	{
		Change * change = e->undo_stack->items[e->undo_stack->count - 1];
		int changes = change->num_lines_changed;
		e->undo_stack->count--;
		for( int i = 0; i < changes; i++ )
		{
			change = e->undo_stack->items[e->undo_stack->count -1 ];
			if( change->line_deleted == false && change->line_added == false )
			{
				e->cursor.y_index = change->line_num;
				e->cursor.index = change->index;
				e->cursor.last_index = e->cursor.index;
				push_insert_to_redo_stack( e );
				write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], change->data );
			}
			else if( change->line_deleted )
			{	
				e->cursor.y_index = change->line_num - 1;
				if( e->cursor.y_index < 0 )
				e->cursor.y_index = 0;

				add_new_line( e, change->data->contents, change->data->count );
				e->lines.count++;
				update_list_of_lines( &e->lines );
				push_undo_del_line_to_redo_stack( e );
				e->cursor.y_index++;
				if( e->cursor.y_index - e->cursor.y_offset == e->window.rows -1 )	
					e->cursor.y_offset++;
				e->cursor.last_y_offset = e->cursor.y_offset;
				write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			}
			else if( change->line_added )
			{
				e->cursor.y_index = change->line_num + 1;
				remove_line_2( e );
				update_list_of_lines( &e->lines );
				e->cursor.y_index--;
				push_add_line_to_redo_stack( e );
				write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], change->data );
				e->cursor.index = change->data->index;
			}
			e->undo_stack->count--;
		}	
		push_change_to_redo_stack( e, changes );
		update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
		render( e );
		if( e->undo_stack->count == 0 )
			e->can_undo = false;
		e->can_redo = true;
	}
	return;
}


void init_undo_redo_stacks( Editor * e )
{
	e->can_undo = false;
	e->can_redo = false;
	e->undo_stack = init_py_list( sizeof( Change * ) );
	e->redo_stack = init_py_list( sizeof( Change * ) );
	return;
}


void init( Editor * e )
{
	enable_Raw_mode( &e->window );
	init_editor_settings( e );
	get_window_size( &e->window );
	strcpy( e->debug_message, "" );
	e->done = false;
	e->tabs = 0;
	if( read_file( &e->lines, e->file_name ) )
		strcpy( e->debug_message, ": new buffer created" );
	init_cursor( &e->cursor );
	e->line_buff = init_line_buffer();
	update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );	
	init_undo_redo_stacks( e );
	update( e );
	index_to_rx( &e->cursor, e->line_buff, e->line_nums );
	return;	
}



void update( Editor * e )
{
	char buf[40];
	snprintf( buf,40,"%i", e->lines.count );
	e->line_nums = strlen( buf );
	return;
}


void render( Editor * e )
{
	update( e );
	update_cursor( &e->cursor, e->line_buff );
	index_to_rx( &e->cursor, e->line_buff, e->line_nums );
	adjust_yx_offsets( &e->cursor, &e->window, e->line_nums, e->line_buff );
 	print_chars_to_screen( e->line_buff, &e->lines, &e->cursor, &e->window, e->line_nums );
	print_mode( &e->window, e->mode, e->debug_message );
	print_cursor( &e->cursor, e->mode );
	return;
}

//I hate windows! >:(
char getch( Editor * e ) 
{
	char c = -1;
	int nread;
	#ifdef _WIN32	
		DWORD numread;
		INPUT_RECORD input_record_buffer[2];
		ReadConsoleInput( e->window.hstdin, input_record_buffer, 2, &numread );
		for( int i = 0; i < numread; i++ )
			if( input_record_buffer[i].Event.KeyEvent.bKeyDown )
			{
				c = input_record_buffer[i].Event.KeyEvent.uChar.AsciiChar; 
				if( c == 0 && input_record_buffer[i].Event.KeyEvent.wVirtualKeyCode != 17 )
					c = input_record_buffer[i].Event.KeyEvent.wVirtualKeyCode; 
			}
	#elif __linux__
		nread = read( STDIN_FILENO, &c, 1);
	#endif
  return c;
}



void insert_char_to_buff( Editor * e, char c )
{
	e->can_redo = false;
	Buff * temp = e->line_buff;
	temp->count++;
	if( e->line_buff->index == -1 )
			e->line_buff->index = e->cursor.index;
	e->line_buff->has_changed = true;

	if( temp->count >= temp->copacity )
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
		e->cursor.x_offset += e->window.cols / 2 ;
	
	strcpy( e->debug_message, "" );
	update_line_buffer_td( temp );
	e->cursor.last_index = e->cursor.index;
	e->cursor.last_x_offset= e->cursor.x_offset;
	render(e);
	return;
}


void remove_line_2( Editor * e )
{
	if( e->cursor.y_index > 0 )
	{
		Line_data * temp = e->lines.list_of_lines[e->cursor.y_index];
		Line_data * line_to_appended_to = temp->prev;
		Line_data * next_line = temp->next;
		line_to_appended_to->next = next_line;
		
		if( next_line != NULL )
			next_line->prev = line_to_appended_to;
		e->lines.count--;
		free( temp->data );
		free( temp );
	}
	else
	{
		Line_data * temp = e->lines.list_of_lines[e->cursor.y_index];
		e->lines.head = temp->next;
		update_list_of_lines( &e->lines );
		e->lines.count--;
		free( temp->data );
		free( temp );
	}
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

	char buff[size];
	memset( buff, 0, size );
	strncat( buff, line_to_appended_to->data, line_to_appended_to->count );
	strncat( buff, temp->data, temp->count );
	reset_buffer( e->line_buff );
	append_to_buffer( e->line_buff, buff, size );
	
	e->lines.count--;
	free( temp->data );
	free( temp );
	return;
}


void backspace( Editor * e, char c )
{
	if( e->cursor.index > 0 )
	{
		e->can_redo = false;
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
		
	}	
	else if( e->cursor.y_index > 0 )
	{
		e->can_redo = false;
		e->cursor.index = e->lines.list_of_lines[e->cursor.y_index-1]->count;
		e->cursor.last_index = e->cursor.index;	
		push_del_line_to_undo_stack( e );
		write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
		remove_line( e );
		update_list_of_lines( &e->lines );
		e->cursor.y_index--;	
		if( e->cursor.index - e->cursor.x_offset >= e->window.cols - 1)
		{
			int shift = ( e->window.cols - ( e->cursor.index - e->cursor.x_offset) ) + 1;
			e->cursor.x_offset += shift;
		}
	}
	update_line_buffer_td( e->line_buff );
	render( e );
	return;
}





void add_new_line( Editor * e, char * data, int size_of_data )
{
	if( e->cursor.y_index >= 0 )
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
		new_line->data = calloc( new_line->copacity, sizeof( char ) );
		reset_buffer( e->line_buff );	
		append_to_buffer( e->line_buff, data, size_of_data );	
	}
	else if( e->cursor.y_index < 0 )
	{
		Line_data * new_line = calloc( 1, sizeof( Line_data ) );
		new_line->prev = NULL;
		new_line->next = e->lines.head;
		e->lines.head->prev = new_line;
		e->lines.head = new_line;
		update_list_of_lines( &e->lines );
		new_line->copacity = size_of_data + 50;
		new_line->data = calloc( new_line->copacity, sizeof( char ) );
		reset_buffer( e->line_buff );	
		append_to_buffer( e->line_buff, data, size_of_data );	
	}
	return;
}




void enter_key( Editor * e, char c )
{
	e->can_redo = false;
	push_insert_to_undo_stack( e );
	write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
	Line_data * temp = e->lines.list_of_lines[e->cursor.y_index];
	char buff[temp->count - e->cursor.index];
		int j =0;
	for( int i = e->cursor.index; i < temp->count; i++ )
	{
		buff[j] = temp->data[i];
		j++;
	}
 	push_new_line_to_undo_stack( e );
	add_new_line( e, buff, j );
	temp->count = e->cursor.index;
	update_line( temp );
	e->lines.count++;
	e->saved = false;
	update_list_of_lines( &e->lines );
	if( e->cursor.y_index == e->window.rows - 2 )
		e->cursor.y_offset++;	
	e->cursor.y_index++;
	e->cursor.last_y_offset = e->cursor.y_offset;
	e->cursor.index = 0;
	e->cursor.last_index = 0;
	e->cursor.x_offset = 0;
	update_line_buffer_td( e->line_buff );	
	render( e );
	return;
}


//TODO: refactor to deal with commands
void search( Editor * e )
{
	strcpy( e->debug_message, "" );
	print_mode( &e->window, e->mode, e->debug_message );
	print_cursor( &e->cursor, e->mode );
	// move curor down to menu bar
	char buff[40];
	sprintf( buff, "\x1b[%d;%dH", e->window.rows, 13 );
	write( 0, buff, strlen( buff ) );
	// getstring()
	int i = 0;	
	char c; 
	char back = '\b';
	while( ( c = getch( e ) ) != 13 && i < 40 )	
	{
		if( c == 27 ) // escape 
		{
			strcpy( e->debug_message, "" );
			render( e );
			return;
		}
		if ( c == 127 )// backspace
		{
			if(i > 0)
			{
				i--;
				write(STDOUT_FILENO, &back, 1);
				write(STDOUT_FILENO, " ", 1);
				write(STDOUT_FILENO, &back, 1);
			}
		}
		else if( isprint( c ) )
		{
			write(STDOUT_FILENO, &c, 1);
			buff[i] = c;
			i++;
		}
	}
	buff[i] = '\0';
	// search
	char * index = NULL;
	for( int i = e->cursor.y_index; i < e->lines.count; i++ )
	{
		index = strstr( e->lines.list_of_lines[i]->data, buff );
		if( index != NULL )
		{
			e->cursor.y_index = i; 
			e->cursor.index = index - e->lines.list_of_lines[i]->data;
			e->cursor.last_index = e->cursor.index;
			e->cursor.last_x_offset = e->cursor.x_offset;
			break;	
		}
	}
	if( index == NULL )
	{
		strcpy( e->debug_message, "reached bottom: searching from top" );
		for( int i = 0; i < e->cursor.y_index; i++ )
		{
			index = strstr( e->lines.list_of_lines[i]->data, buff );
			if( index != NULL )
			{
				e->cursor.y_index = i; 
				e->cursor.index = index - e->lines.list_of_lines[i]->data;
				e->cursor.last_index = e->cursor.index;
				e->cursor.last_x_offset = e->cursor.x_offset;
				break;	
			}
		}
		if( index == NULL )
		{
			char message[40];
			snprintf( message, 40, "Pattern not found: %s", buff );
			strcpy( e->debug_message, message );
		}
	}
	adjust_yx_offsets( &e->cursor, &e->window, e->line_nums, e->line_buff );
	render( e ); 
	return;
}



void move_cursor_up( Editor * e )
{
	if( e->cursor.y_index > 0 )
	{
		if( e->mode == INSERT && e->line_buff->has_changed == true )
		{
			push_insert_to_undo_stack( e );	
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			push_change_to_undo_stack( e, e->line_buff->has_changed, e->line_buff->line_deleted, e->line_buff->line_added );
		}
		e->cursor.y_index--;
		
		if( e->cursor.y_offset > 0 && e->cursor.y_index - e->cursor.y_offset < 0 )
			e->cursor.y_offset--;
		e->cursor.last_y_offset = e->cursor.y_offset;

		update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
		render( e );
	}
	return;
}


void move_cursor_down( Editor * e )
{
	if( e->cursor.y_index < e->lines.count - 1 )
	{
		if( e->mode == INSERT && e->line_buff->has_changed == true )
		{
			push_insert_to_undo_stack( e );
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			push_change_to_undo_stack( e, e->line_buff->has_changed, e->line_buff->line_deleted, e->line_buff->line_added );
		}
		e->cursor.y_index++;

		if( e->cursor.y_index - e->cursor.y_offset == e->window.rows -1 )	
			e->cursor.y_offset++;
		e->cursor.last_y_offset = e->cursor.y_offset;
		
		update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
		render( e );
	}
	return;
}


void move_cursor_left( Editor * e )
{
	if( e->cursor.index > 0 )
	{
		if( e->mode == INSERT && e->line_buff->has_changed == true )
		{
			push_insert_to_undo_stack( e );
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			push_change_to_undo_stack( e, e->line_buff->has_changed, e->line_buff->line_deleted, e->line_buff->line_added );
		}
		e->cursor.index--;
		
		index_to_rx( &e->cursor, e->line_buff, e->line_nums );

		if( e->cursor.rx <= ( e->line_nums + 1 ) && e->cursor.x_offset > 0 )
			e->cursor.x_offset -= e->window.cols - ( e->line_nums + 1 ); 

		if( e->cursor.x_offset < 0 )
			e->cursor.x_offset = 0;
		
		e->cursor.last_index = e->cursor.index;
		e->cursor.last_x_offset = e->cursor.x_offset;
		render(e);
	}
	return;
}


void move_cursor_right( Editor * e )
{
	if( e->cursor.index < e->line_buff->count )
	{
		if( e->mode == INSERT && e->line_buff->has_changed == true )
		{
			push_insert_to_undo_stack( e );
			write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
			push_change_to_undo_stack( e, e->line_buff->has_changed, e->line_buff->line_deleted, e->line_buff->line_added );
		}		
		
		e->cursor.index++;
		index_to_rx( &e->cursor, e->line_buff, e->line_nums );
		int shift = ( e->window.cols - ( e->cursor.index - e->cursor.x_offset) ) + 1;
		if( e->cursor.rx >= e->window.cols  - 1 )
			e->cursor.x_offset += shift;
			//e->cursor.x_offset += e->window.cols - ( e->line_nums + 1 ); 
		e->cursor.last_index = e->cursor.index;
		e->cursor.last_x_offset = e->cursor.x_offset;
		render(e);
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
	if( ( temp = getch( e ) ) != -1 )
	{
		is_arrow_key = true;
		temp = getch( e );
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
	int c = getch( e );
	#ifdef _WIN32	
		if( c <= 40 && c >= 37 )
		{
			events_move_cursor_insert_windows( e, c );
			return;
		}
	#endif
	switch( c )
	{
		case 27: // escape not working for some reason!
		{		
			#ifdef _WIN32	
				e->mode = NORMAL;
				push_insert_to_undo_stack( e );
				write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
				push_change_to_undo_stack( e, e->line_buff->has_changed, e->line_buff->line_deleted, e->line_buff->line_added );
				update_cursor( &e->cursor, e->line_buff );
				print_mode( &e->window, e->mode, e->debug_message );
				print_cursor( &e->cursor, e->mode );
			#elif __linux__
				if( !events_move_cursor_insert_linux( e ) )
				{
					e->mode = NORMAL;
					push_insert_to_undo_stack( e );
					write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
					push_change_to_undo_stack( e, e->line_buff->has_changed, e->line_buff->line_deleted, e->line_buff->line_added );
					update_cursor( &e->cursor, e->line_buff );
					print_mode( &e->window, e->mode, e->debug_message );
					print_cursor( &e->cursor, e->mode );
				}
			#endif
		}break;
		case 13: enter_key( e, c );break; //enter key
		case BACKSPACE: backspace( e, c ); break;
		default:
		{
			if( isprint( c ) || c== '\t' )
				insert_char_to_buff( e, c );
			//render(e);
		}break;
	}
	return;
}

char comand_mode( Editor * e )
{
	char underline[] = "\e[4 q";
	char block[] = "\e[1 q";
	write( STDOUT_FILENO, underline, strlen( underline ) );
	char c;
	while( true )
	{
		c = getch( e );
		if( c != -1 )
			goto done;
	}
	done:
	write( STDOUT_FILENO, block, strlen( block ) );
	return c;
}

void events_normal( Editor * e )
{	
	int c = getch( e );
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
			search( e );
		}break;
		case 'o':
		{
			e->cursor.index = 0;//	e->lines.list_of_lines[e->cursor.y_index]->count;
			enter_key( e, c );
			e->mode = INSERT;
			update_cursor( &e->cursor, e->line_buff );
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
			redo_change( e );
		}break;
	
		case 'd':
		{
			comand_mode( e );
			switch( c )
			{
				case 'd':
				{
					/*
					push_del_line_to_undo_stack( e );
					push_change_to_undo_stack( e, e->line_buff->has_changed, e->line_buff->line_deleted, e->line_buff->line_added );
					remove_line_2( e );
					update_list_of_lines( &e->lines );
					update_line_buffer( e->line_buff, e->lines.list_of_lines[e->cursor.y_index] );
					render( e );
					*/
				}break;
			}
		}break;

		case CTRL_KEY( 'u' ):
		{
			undo_change( e );
		}break;
		case CTRL_KEY( 'c' ):
		{
			//shift_everything();
		}break;

		case CTRL_KEY( 's' ):
		{
				write_line_buffer_to_line( e->lines.list_of_lines[e->cursor.y_index], e->line_buff );
				save_file( e->file_name, &e->lines );
				strcpy( e->debug_message, ": file saved" );
				e->saved = true;
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
	}
	if( get_window_size( &e->window ) )
	{
		adjust_yx_offsets( &e->cursor, &e->window, e->line_nums, e->line_buff );
		render( e );
	}
	return;
}


void quit( Editor * e )
{
	Line_data * prev = NULL;
	Line_data * temp = e->lines.head;
	for( int i =0; i< e->lines.count; i++ )
	{
		prev = temp;
		temp= temp->next;
		free( prev->data );
		free( prev );
	}
	free( e->lines.list_of_lines );


	//clear screen!
	write( STDOUT_FILENO, "\x1b[2J", 4 );
	write( STDOUT_FILENO, "\x1b[H", 3 );

	for( int i = 0; i < e->undo_stack->count; i++ )
	{
		if( e->undo_stack->items[i] != NULL )
			free_change( e->undo_stack->items[i] );
	}
	free( e->undo_stack->items );
	free( e->undo_stack );
	
	for( int i = 0; i < e->redo_stack->count; i++ )
	{
		if( e->redo_stack->items[i] != NULL )
			free_change( e->redo_stack->items[i] );
	}
	free( e->redo_stack->items );
	free( e->redo_stack );
	
	free( e->line_buff->contents );
	free( e->line_buff );
	disable_raw_mode( &e->window );
	return;
}
