#include "include/file_tree.h"

//Devine fucking intelect! :D
void reset_file_tree( File_tree * tree );
void free_file_tree( Line_data * head, i32 count, i32 iteration );
void sort_directory( Line_data * head, bool is_root );


void adjust_cursor_offset( File_tree * tree, i32 rows )
{
	if( ( tree->cursor.y_index - tree->cursor.y_offset ) > rows - 2 || tree->cursor.y_index < tree->cursor.y_offset || tree->cursor.y_index + tree->cursor.y_offset > tree->lines.expanded_count )
	{
		tree->cursor.y_offset = 0 ;
		while( ( tree->cursor.y_index - tree->cursor.y_offset ) > rows - 2 )
			tree->cursor.y_offset++;
	}	
	return;
}


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
			index = _update_file_tree_items( tree, temp, index, temp->dcount, iterations);
		temp = temp->next;
	}
	return index;
}


void update_file_tree_items( File_tree * tree, Lines_data * lines )
{
	i32 new_count = get_new_size( lines->head->next, lines->count );
	lines->expanded_count = lines->count + new_count;
	resize_list_expanded( lines );
	_update_file_tree_items( tree, lines->head, 0, lines->count, -1 );
	return;	
}


int read_directory( File_tree * tree )
{
	DIR * directory;
	struct dirent * entry;
	struct stat file_stat;
	char buff[1024];	
	snprintf( buff, 1024, "%s/%s", tree->lines.list_of_lines[tree->cursor.y_index]->to_display, tree->lines.list_of_lines[tree->cursor.y_index]->data );
	errno = 0;
	directory = opendir( buff );

	if( errno == EACCES )
		return errno;

	if( directory == NULL )
		die( "unable to read directory" );

	if( tree->lines.list_of_lines[tree->cursor.y_index]->head == NULL )
	{
		tree->lines.list_of_lines[tree->cursor.y_index]->head = calloc( 1, sizeof( Line_data ) );
		tree->lines.list_of_lines[tree->cursor.y_index]->head->data = calloc( 50, sizeof( char ) );	
		tree->lines.list_of_lines[tree->cursor.y_index]->head->to_display = calloc( 1024, sizeof( char ) );	
	}
	Line_data * temp = tree->lines.list_of_lines[tree->cursor.y_index]->head;
	Line_data * prev = tree->lines.list_of_lines[tree->cursor.y_index];
	Line_data * to_be_delete = NULL;
	tree->lines.list_of_lines[tree->cursor.y_index]->dcount = 0;
	i32 i;
	while( ( entry = readdir( directory ) ) != NULL )
	{ 
		if( strcmp( entry->d_name, ".") == 0 || strcmp( entry->d_name, ".." ) == 0 )
			continue;
		tree->lines.list_of_lines[tree->cursor.y_index]->dcount++;
		temp->prev = prev;	
		for( i = 0; i < 50 && i < strlen(entry->d_name); i++ )
			temp->data[i] = entry->d_name[i];
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
		to_be_delete = temp;
		temp = temp->next;
	}
	closedir( directory );
	if( to_be_delete != NULL )
	{	
		free( to_be_delete->next->data );
		free( to_be_delete->next->to_display );
		free( to_be_delete->next );
		to_be_delete->next = NULL;
	}
	return 0; 
}


void expand_tree_at_point_of_cursor( File_tree * tree, char * debug_message)
{
	if( strcmp( tree->lines.list_of_lines[tree->cursor.y_index]->data, ".." ) != 0 )
	{
		if( tree->lines.list_of_lines[tree->cursor.y_index]->dcount == 0 )
		{
			if( read_directory( tree ) == EACCES )
			{
				strcpy( debug_message, "PERMISSION DENIED!" );
			}
			sort_directory( tree->lines.list_of_lines[tree->cursor.y_index], false );
		}
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


void change_dir_at_point_of_cursor( File_tree * tree, char * debug_message )
{

	char temp[1024];
	snprintf( temp, 1024, "%s/%s", tree->lines.list_of_lines[tree->cursor.y_index]->to_display, tree->lines.list_of_lines[tree->cursor.y_index]->data );	
	errno = 0;
	DIR * directory = opendir( temp );
	if( directory == NULL && errno == EACCES )
	{
		strcpy( debug_message, "PERMISSION DENIED!" );
		return;
	}

	if( tree->lines.list_of_lines[tree->cursor.y_index]->is_dir )
		if( ( strlen(  tree->working_directory ) + strlen( tree->lines.list_of_lines[tree->cursor.y_index]->data ) )  < 1024 - 1 )
			snprintf( tree->working_directory, 1024, "%s/%s", tree->lines.list_of_lines[tree->cursor.y_index]->to_display, tree->lines.list_of_lines[tree->cursor.y_index]->data );	
	reset_file_tree( tree );
	return;
}


void swap( Line_data * a, Line_data * b )
{	
	i32 dcount;
	i32 dcopacity;
	i32 count;
	bool is_dir;
	bool expanded;	
	char temp[50];

	strcpy( temp, a->data );
	strcpy( a->data, b->data );
	strcpy( b->data, temp );

	dcopacity = a->dcopacity;
	a->dcopacity = b->dcopacity;
	b->dcopacity = dcopacity;

	dcount = a->dcount;
	a->dcount = b->dcount;
	b->dcount = dcount;
	
	count = a->count;
	a->count = b->count;
	b->count = count;

	is_dir = a->is_dir;
	a->is_dir = b->is_dir;
	b->is_dir = is_dir;

	expanded = a->expanded;
	a->expanded = b->expanded;
	b->expanded = expanded;

	return;
}


void str_to_lower( char * str )
{
	for( i32 i = 0; i < strlen( str ); i++ )
		str[i] = tolower( str[i] );
	return;
}


bool compare( Line_data * a, Line_data * b )
{
	if( a->is_dir > b->is_dir )
		return true;
	else if( a->is_dir < b->is_dir )
		return false;

	char buff_1[50];
	char buff_2[50];
	strcpy( buff_1, a->data );
	strcpy( buff_2, b->data );
	str_to_lower(buff_1);
	str_to_lower(buff_2);
	if( strcmp( buff_1, buff_2 ) < 0 )
		return true;
	return false;
}



void sort_directory( Line_data * head, bool root )
{
	Line_data * index = NULL;
	Line_data * current = NULL;
	Line_data * last;
	bool done = false;

	if( root )
		current = head->next;
	else if( !root )
		current = head->head;
	while( !done )
	{
		done = true;
		index = current;
		last = current; 
		while( index != NULL )
		{

			if( compare( index, last ) )
			{
				swap( last, index  );
				done = false;
			}
			last = index;
			index = index->next;
		}
	}
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
	Line_data * temp = tree->lines.head->next;
	Line_data * prev = tree->lines.head;
	Line_data * to_be_delete = NULL;
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
		to_be_delete = temp;
		temp = temp->next;
	}
	tree->lines.expanded_count = tree->lines.count;
	closedir( directory );	
	free( to_be_delete->next->data );
	free( to_be_delete->next->to_display );
	free( to_be_delete->next );
	to_be_delete->next = NULL;
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
	free( tree->lines.list_of_lines );
	init_cursor( &tree->cursor);
	tree->cursor.index = 1;
	tree->cursor.y_index = 1;
	tree->lines.head = calloc( 1, sizeof( Line_data ) );
	if( tree->lines.head == NULL ) die( "could not allocate memory for file_tree" );
	tree->lines.list_of_lines = NULL; 
	init_line( tree->lines.head );
	tree->lines.head->next->to_display = calloc( 1024, sizeof( char ) );
	read_working_dir( tree );
	sort_directory( tree->lines.head, true );
	//update_list_of_lines( &tree->lines );
	update_file_tree_items( tree, &tree->lines );
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
	sort_directory( tree->lines.head, true );
	update_file_tree_items( tree, &tree->lines );
	file_tree_toggle = false;
	return;
}


void toggle_file_tree()
{
	file_tree_toggle = ( file_tree_toggle == true )?false:true;
	return;
}
