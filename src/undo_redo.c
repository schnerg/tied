#include "include/undo_redo.h"


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


void push_insert_to_redo_stack( py_list_t * redo_stack, Line_data * line, Cursor * c )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->data = init_buffer();
	append_to_buffer( change->data, line->data, line->count );
	change->line_num = c->y_index;
	append_to_py_list( redo_stack, change );
	return;
}


void push_insert_to_undo_stack( py_list_t * undo_stack, Buff * line_buff, Line_data * line, Cursor * c )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->data = init_buffer();
	if( line_buff->line_deleted == false && line_buff->line_added == false )
		change->index = line_buff->index;
	append_to_buffer( change->data, line->data, line->count );
	change->line_num = c->y_index;
	free_change_if_needed( undo_stack );
	append_to_py_list( undo_stack, change );
	line_buff->num_lines_changed++;
	return;
}


void push_add_line_to_redo_stack( py_list_t * redo_stack, Line_data * line, Cursor * c )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->line_added = true;
	change->line_num = c->y_index;
	change->data = init_buffer();
	append_to_buffer( change->data, line->data, line->count );
	append_to_py_list( redo_stack, change );
	return;
}


void push_new_line_to_undo_stack( py_list_t * undo_stack, Buff * line_buff, Cursor * c, Line_data * line )
{
	line_buff->has_changed = true;
	line_buff->line_added = true;
	Change * change = calloc( 1, sizeof( Change ) );
	change->line_added = true;
	change->data = init_buffer();
	change->data->index = c->index;
	append_to_buffer( change->data, line->data, line->count );
	change->line_num = c->y_index;
	free_change_if_needed( undo_stack );
	append_to_py_list( undo_stack, change );
	line_buff->num_lines_changed++;
	return;
}


void push_undo_del_line_to_redo_stack( py_list_t * redo_stack, Cursor * c )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->line_deleted = true;
	change->line_num = c->y_index;
	append_to_py_list( redo_stack, change );
	return;
}


void push_del_line_to_undo_stack( py_list_t * undo_stack, Buff * line_buff, Cursor * c, Line_data * line  )
{
	line_buff->has_changed = true;
	line_buff->line_deleted = true;
	Change * change = calloc( 1, sizeof( Change ) );
	change->line_deleted = true;
	change->data = init_buffer();
	change->data->index = line_buff->index;
	append_to_buffer( change->data, line->data, line->count );
	change->line_num = c->y_index;
	free_change_if_needed( undo_stack );
	append_to_py_list( undo_stack, change );
	line_buff->num_lines_changed++;
	return;
}


void push_change_to_redo_stack( py_list_t * redo_stack,  int num_lines_changed )
{
	Change * change = calloc( 1, sizeof( Change ) );
	change->num_lines_changed = num_lines_changed;
	append_to_py_list( redo_stack, change );
	return;
}


void push_change_to_undo_stack( py_list_t * undo_stack, py_list_t * redo_stack, Buff * line_buff, bool * can_undo )
{
	if( line_buff->has_changed )
	{
		Change * change = calloc( 1, sizeof( Change ) );
		if( line_buff->line_deleted )
			change->line_deleted = true;
		if( line_buff->line_added )
			change->line_added = true;
		change->num_lines_changed = line_buff->num_lines_changed;
		change->index = line_buff->index;
		free_change_if_needed( undo_stack );
		append_to_py_list( undo_stack, change );
		redo_stack->count = 0;
		line_buff->num_lines_changed = 0;
		line_buff->has_changed = false;
		line_buff->line_deleted = false;
		line_buff->line_added = false;
		line_buff->index = -1;
		*can_undo = true;
	}
	return;
}


void remove_line_2( Lines_data * lines, Cursor * c )
{
	Line_data * temp = lines->list_of_lines[c->y_index];
	Line_data * line_to_appended_to = temp->prev;
	Line_data * next_line = temp->next;
	line_to_appended_to->next = next_line;
	if( next_line != NULL )
		next_line->prev = line_to_appended_to;
	lines->count--;
	free( temp->data );
	free( temp );
	return;
}

void add_new_line_2( Buff * line_buff, Lines_data * lines, Cursor * c, char * data, int size_of_data )
{
	Line_data * current_line  = lines->list_of_lines[c->y_index];
	Line_data * next_line = current_line->next;
	Line_data * new_line = calloc( 1, sizeof( Line_data ) );
	current_line->next = new_line;
	new_line->prev = current_line;
	new_line->next = next_line;
	if( next_line != NULL )
		next_line->prev = new_line;
	
	new_line->copacity = size_of_data + 50;
	new_line->data = calloc( new_line->copacity, sizeof( char ) );
	reset_buffer( line_buff );	
	append_to_buffer( line_buff, data, size_of_data );	
	return;
}


void redo_change( py_list_t * redo_stack, py_list_t * undo_stack, Cursor * c, Lines_data * lines, Buff * line_buff, Window * window, bool * can_redo, bool * can_undo )
{
	if( *can_redo )
	{
		Change * change = redo_stack->items[redo_stack->count - 1];
		int changes = change->num_lines_changed;
		undo_stack->count++;
		redo_stack->count--;
		free_change( change );
		for( int i = 0; i < changes; i++ )
		{
			change = redo_stack->items[redo_stack->count - 1];
			if( change->line_deleted == false && change->line_added == false )
			{
				c->y_index = change->line_num;
				write_line_buffer_to_line( lines->list_of_lines[c->y_index], change->data );
				free_change( change );
			}
			else if( change->line_deleted )
			{
				c->y_index = change->line_num + 1;
				remove_line_2( lines, c );
				update_list_of_lines( lines );
				c->y_index--;
				free_change( change );
			}
			//why does this work!?
			//still no idea why this works!
			else if( change->line_added )
			{
				write_line_buffer_to_line( lines->list_of_lines[c->y_index], change->data );
 				add_new_line_2( line_buff, lines, c, change->data->contents, change->data->count );
				lines->count++;
				update_list_of_lines( lines );
				c->y_index++;
				if( c->y_index - c->y_offset == window->rows -1 )	
					c->y_offset++;
				c->last_y_offset = c->y_offset;
				free_change( change );
			}
			undo_stack->count++;
			redo_stack->count--;
		}
		if( redo_stack->count == 0 )
			*can_redo = false;
		*can_undo = true;
		update_line_buffer( line_buff, lines->list_of_lines[c->y_index] );
		//render( e );
	}
	return;
}


void undo_change( py_list_t * redo_stack, py_list_t * undo_stack, Cursor * c, Lines_data * lines, Buff * line_buff, Window * window, bool * can_redo, bool * can_undo )
{
	if( *can_undo )
	{
		Change * change = undo_stack->items[undo_stack->count - 1];
		int changes = change->num_lines_changed;
		undo_stack->count--;
		for( int i = 0; i < changes; i++ )
		{
			change = undo_stack->items[undo_stack->count -1 ];
			if( change->line_deleted == false && change->line_added == false )
			{
				c->y_index = change->line_num;
				c->index = change->index;
				c->last_index = c->index;
 				push_insert_to_redo_stack( redo_stack, lines->list_of_lines[c->y_index], c );
				write_line_buffer_to_line( lines->list_of_lines[c->y_index], change->data );
			}
			else if( change->line_deleted )
			{	
				c->y_index = change->line_num - 1;
				if( c->y_index < 0 )
				c->y_index = 0;
 				add_new_line_2( line_buff, lines, c, change->data->contents, change->data->count );
				lines->count++;
				update_list_of_lines( lines );
				push_undo_del_line_to_redo_stack( redo_stack, c );
				c->y_index++;
				if( c->y_index - c->y_offset == window->rows -1 )	
					c->y_offset++;
				c->last_y_offset = c->y_offset;
				write_line_buffer_to_line( lines->list_of_lines[c->y_index], line_buff );
			}
			else if( change->line_added )
			{
				c->y_index = change->line_num + 1;
				remove_line_2( lines, c );
				update_list_of_lines( lines );
				c->y_index--;
 				push_add_line_to_redo_stack( redo_stack, lines->list_of_lines[c->y_index], c );
				write_line_buffer_to_line( lines->list_of_lines[c->y_index], change->data );
				c->index = change->data->index;
			}
			undo_stack->count--;
		}	
 		push_change_to_redo_stack( redo_stack, changes );
		update_line_buffer( line_buff, lines->list_of_lines[c->y_index] );
		if( undo_stack->count == 0 )
			*can_undo = false;
		*can_redo = true;
	}
	return;
}



