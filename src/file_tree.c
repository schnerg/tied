#include "include/file_tree.h"


void reset_file_tree( File_tree * tree );

void resize_list_expanded( Lines_data * lines )
{
	while( lines->expanded_count >= lines->copacity -1 )
	{
		lines->copacity *= 2;
		lines->list_of_lines = realloc( lines->list_of_lines, lines->copacity * sizeof( Line_data * ) );
		if(lines->list_of_lines == NULL ) die( "resize_list(): failed to reallocate memory for list_of_lines." );
	}
	return;
}


int get_new_size( Line_data * head, i32 count )
{
	Line_data * temp = head;
	i32 new_count = 0;
	for( i32 i = 0; i < count; i++ )
	{
		if( temp->is_dir && temp->expanded )
		{
			new_count += temp->dcount;
			new_count += get_new_size( temp->head, temp->dcount );
		}
		temp = temp->next;
	}
	return new_count;
}


int _update_file_tree_items( File_tree * tree, Line_data * head, i32 index, i32 count, i32 iterations )
{
	iterations++;
	Line_data * temp = NULL;
	if( index == 0 )
	{
		temp = head->next;
		tree->lines.list_of_lines[index] = temp;
		index++;
	}
	else
		temp = head->head;
	for( i32 i = 0; i < count; i++ )
	{
		tree->lines.list_of_lines[index] = temp;
		temp->dcopacity = iterations;
		index++;
		if( temp->is_dir && temp->expanded )
		{
			index = _update_file_tree_items( tree, temp, index, temp->dcount, iterations);
		}
		temp = temp->next;
	}
	return index;
}


void update_file_tree_items( File_tree * tree, Lines_data * lines )
{
	i32 new_count = get_new_size( lines->head->next, lines->count - 1 );
	lines->expanded_count = lines->count + new_count;
	resize_list_expanded( lines );
	_update_file_tree_items( tree, lines->head, 0, lines->count, -1 );
	return;	
}


void read_directory( File_tree * tree )
{
	DIR * directory;
	struct dirent * entry;
	struct stat file_stat;
	char buff[1024];	
	snprintf( buff, 1024, "%s/%s", tree->lines.list_of_lines[tree->cursor.y_index]->to_display, tree->lines.list_of_lines[tree->cursor.y_index]->data );
	directory = opendir( buff );
	
	//if( strlen( tree->working_directory ) + strlen( tree->lines.list_of_lines[tree->cursor.y_index]->data )  < 1024 - 1 ) 
	//snprintf( buff, 1024, "%s/%s", tree->working_directory, tree->lines.list_of_lines[tree->cursor.y_index]->data );

	if( directory == NULL )
	{
		die( "unable to read directory" );
		return;
	}

	tree->lines.list_of_lines[tree->cursor.y_index]->head = calloc( 1, sizeof( Line_data ) );
	tree->lines.list_of_lines[tree->cursor.y_index]->head->data = calloc( 50, sizeof( char ) );	
	tree->lines.list_of_lines[tree->cursor.y_index]->head->to_display = calloc( 1024, sizeof( char ) );	

	Line_data * temp = tree->lines.list_of_lines[tree->cursor.y_index]->head;
	Line_data * prev = tree->lines.list_of_lines[tree->cursor.y_index];
	tree->lines.list_of_lines[tree->cursor.y_index]->dcount = 0 ;
	i32 i;
	while( ( entry = readdir( directory ) ) != NULL )
	{ 
		if( strcmp( entry->d_name, ".") == 0 || strcmp( entry->d_name, ".." ) == 0 )
			continue;
		tree->lines.list_of_lines[tree->cursor.y_index]->dcount++;
		temp->prev = prev;	
		for( i = 0; i < 50 && i < strlen(entry->d_name); i++ )
			temp->data[i] = entry->d_name[i];
		if( i >= 50 )
			temp->data[i] = '\0';
		// checking if file is directory
		char file_name_and_path[1024];
		snprintf( file_name_and_path, 1024, "%s/%s", buff, entry->d_name );
		stat( file_name_and_path, &file_stat );
		if( S_ISDIR( file_stat.st_mode ) )
			temp->is_dir = true;	
		
		// saving path to file so can use later
		strcpy( temp->to_display, buff );
		temp->count = i;
		// incrementing
		prev = temp;	
		init_line( temp );
		temp->next->to_display = calloc( 1024, sizeof( char ) );
		temp = temp->next;
	}
	closedir( directory );	
	return; 
}


void expand_tree_at_point_of_cursor( File_tree * tree )
{
	if( strcmp( tree->lines.list_of_lines[tree->cursor.y_index]->data, ".." ) != 0 )
	{
		if( tree->lines.list_of_lines[tree->cursor.y_index]->dcount == 0 )
			read_directory( tree );
		tree->lines.list_of_lines[tree->cursor.y_index]->expanded = true;
		update_file_tree_items( tree, &tree->lines );
	}
	else if( strcmp( tree->working_directory, "/") != 0 )
	{
		i32 i = strlen( tree->working_directory );
		while( tree->working_directory[i] != '/' )
			i--;
		if( i > 0)
			tree->working_directory[i] = '\0';
		else
			tree->working_directory[i+1] = '\0';
		reset_file_tree( tree );
	}
	return;
}


void sort_file_tree( File_tree * tree )
{
	// sort alphabetically
	Line_data * temp = NULL;
	bool sorted = false;
	while( !sorted )
	{
		sorted = true;
		for( i32 i = 1; i < tree->lines.count - 1 ; i++ )
		{
			if( strcmp( tree->lines.list_of_lines[i]->data, tree->lines.list_of_lines[i+1]->data ) < 0 )
			{
				sorted = false;
				temp = tree->lines.list_of_lines[i];
				tree->lines.list_of_lines[i] = tree->lines.list_of_lines[i+1];
				tree->lines.list_of_lines[i+1] = temp;
				break;
			}
		}
	}

	// sort by file first
	for( i32 i = 1; i < tree->lines.count; i++ )
	{
		if( tree->lines.list_of_lines[i]->is_dir )
			continue;
		for( i32 j = i + 1; j < tree->lines.count; j++ )
		{
			if( tree->lines.list_of_lines[j]->is_dir )
			{
				temp = tree->lines.list_of_lines[i];
				tree->lines.list_of_lines[i] = tree->lines.list_of_lines[j];
				tree->lines.list_of_lines[j] = temp;
			}
		}
	}
/*
	// sort by alphabetically, again :(
	while( !sorted )
	{
		sorted = true;
		for( i32 i = 1; i < tree->lines.count - 1 ; i++ )
		{
			if( strcmp( tree->lines.list_of_lines[i]->data, tree->lines.list_of_lines[i+1]->data ) > 0 )
			{
				sorted = false;
				temp = tree->lines.list_of_lines[i];
				tree->lines.list_of_lines[i] = tree->lines.list_of_lines[i+1];
				tree->lines.list_of_lines[i+1] = temp;
				break;
			}
		}
	}
*/
	return;
}


void read_working_dir( File_tree * tree )
{
	resize_list( &tree->lines );
	DIR * directory;
	struct dirent * entry;
	struct stat file_stat;
	directory = opendir( tree->working_directory );
	if( directory == NULL )
	{
		alert( "unable to read directory" );
		return;
	}
	
	i32 files = 0;
	Line_data * temp = tree->lines.head->next;
	Line_data * prev = tree->lines.head;
	tree->lines.count = 0;
	i32 i;
	while( ( entry = readdir( directory ) ) != NULL )
	{ 
		if( strcmp( entry->d_name, ".") == 0 )
			continue;
		tree->lines.count++;
		temp->prev = prev;	
		for( i = 0; i < 50 && i < strlen( entry->d_name ); i++ )
			temp->data[i] = entry->d_name[i];		
		if( i >= 50 )
			temp->data[i] = '\0';
		temp->count = i ;
		
		// saving path to file so can use later
		if( strlen( tree->working_directory ) < 1024 )
			strcpy( temp->to_display, tree->working_directory );

		char buff[1024];
		snprintf( buff, 1024, "%s/%s", tree->working_directory, temp->data );
		stat( buff, &file_stat );
		if( S_ISDIR( file_stat.st_mode ) )
			temp->is_dir = true;
		
		prev = temp;	
		init_line( temp );
		temp->next->to_display = calloc( 1024, sizeof( char ) );
		temp = temp->next;
	}
	tree->lines.expanded_count = tree->lines.count;
	update_list_of_lines( &tree->lines );
	closedir( directory );	
	return; 
}


int get_working_dir( File_tree * tree )
{
	if( getcwd( tree->working_directory, 1024 ) != NULL ) 
		return 0;
	return 1;
}


void free_file_tree( Line_data * head, i32 count, i32 iteration )	
{
	Line_data * temp = head;
	if( iteration == 0 )
		temp = head->next;
	else
		temp = head->head;	
	Line_data * next = temp;
	for( int i = 0; i < count; i++)
	{
		temp = next;
		if( temp->is_dir )
			free_file_tree( temp, temp->dcount, 1 );
		next = temp->next;
		free( temp->data );
		free( temp->to_display);
		free( temp );
	}
	return;
}



void reset_file_tree( File_tree * tree )
{
	free_file_tree( tree->lines.head, tree->lines.count, 0 );
	free( tree->lines.head );
	init_cursor( &tree->cursor);
	tree->cursor.index = 1;
	tree->cursor.y_index = 1;
	tree->lines.head = calloc( 1, sizeof( Line_data ) );
	if( tree->lines.head == NULL ) die( "could not allocate memory for file_tree" );
	init_line( tree->lines.head );
	tree->lines.head->next->to_display = calloc( 1024, sizeof( char ) );
	read_working_dir( tree );
	return;
}



void init_file_tree( File_tree * tree )
{
	init_cursor( &tree->cursor);
	tree->cursor.index = 1;
	tree->cursor.y_index = 1;
	if( get_working_dir( tree ) == 1 ) die( "could not get working directory. :( ");
	tree->lines.head = calloc( 1, sizeof( Line_data ) );
	if( tree->lines.head == NULL ) die( "could not allocate memory for file_tree" );
	tree->lines.list_of_lines = NULL; 
	init_line( tree->lines.head );
	tree->lines.head->next->to_display = calloc( 1024, sizeof( char ) );
	read_working_dir( tree );
	//sort_file_tree( tree );
	file_tree_toggle = false;
	return;
}


void toggle_file_tree()
{
	file_tree_toggle = ( file_tree_toggle == true )?false:true;
	return;
}
