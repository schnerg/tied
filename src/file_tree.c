#include "include/file_tree.h"

// I think I might be stupid. :(
void reset_file_tree( File_tree * tree );
void free_file_tree( Line_data * head, i32 count, i32 iteration );
void sort_directory( Line_data * head, bool is_root );
Line_data * init_file();


bool is_directory( const char * file_name )
{
#ifdef _WIN32
	DWORD attributes = GetFileAttributesA( file_name );	
	if( attributes == INVALID_FILE_ATTRIBUTES )
		return false;
	return ( attributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
#elif __linux__
	struct stat file_stat;
	stat( file_name, &file_stat );
	if( S_ISDIR( file_stat.st_mode ) )
		return true;
	return false;
#endif
}

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
	if( lines->copacity <= 0 )
		lines->copacity = 1;
	while( lines->expanded_count >= lines->copacity -1 )
		lines->copacity *= 2;
	lines->list_of_lines = realloc( lines->list_of_lines, lines->copacity * sizeof( Line_data * ) );
	if(lines->list_of_lines == NULL ) die( "resize_list_expanded(): failed to reallocate memory for list_of_lines." );
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


int _update_file_tree_items( File_tree * tree, Line_data * head, i32 count, i32 index, i32 iterations )
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
			index = _update_file_tree_items( tree, temp, temp->dcount, index, iterations);
		temp = temp->next;
	}
	return index;
}


void update_file_tree_items( File_tree * tree, Lines_data * lines )
{
	i32 new_count = get_new_size( lines->head->next, lines->count );
	lines->expanded_count = lines->count + new_count;
	resize_list_expanded( lines );
	_update_file_tree_items( tree, lines->head, lines->count, 0, -1 );
	return;	
}


i32 read_directory( File_tree * tree )
{
	DIR * directory;
	struct dirent * entry;
	i32 len = strlen( tree->lines.list_of_lines[tree->cursor.y_index]->to_display ) + strlen( tree->lines.list_of_lines[tree->cursor.y_index]->data ) + 2;
	char * buff = calloc( len, sizeof( char ) );
	snprintf( buff, len, "%s/%s", tree->lines.list_of_lines[tree->cursor.y_index]->to_display, tree->lines.list_of_lines[tree->cursor.y_index]->data );
	errno = 0;
	directory = opendir( buff );
	if( errno == EACCES )
	{
		free( buff );
		return errno;
	}

	if( directory == NULL )
	{
		free( buff );
		return 0;
	}

	if( tree->lines.list_of_lines[tree->cursor.y_index]->head == NULL )
	{
		tree->lines.list_of_lines[tree->cursor.y_index]->head = init_file();
	}
	Line_data * temp = tree->lines.list_of_lines[tree->cursor.y_index]->head;
	Line_data * prev = tree->lines.list_of_lines[tree->cursor.y_index];
	Line_data * to_be_delete = NULL;
	tree->lines.list_of_lines[tree->cursor.y_index]->dcount = 0;

	while( ( entry = readdir( directory ) ) != NULL )
	{ 
		if( strcmp( entry->d_name, ".") == 0 || strcmp( entry->d_name, ".." ) == 0 )
			continue;	
		tree->lines.list_of_lines[tree->cursor.y_index]->dcount++;
		temp->prev = prev;
		
		long unsigned int new_len = strlen( entry->d_name ) + 1;
		char * temp_data = realloc( temp->data, new_len * sizeof( char ) );
		if( temp_data != NULL )
		{
			temp->data = temp_data; 
			strcpy( temp->data, entry->d_name );
			temp->count = new_len - 1 ;
		}	
		// checking if file is directory
		new_len = strlen( buff ) + 1 ;
		temp_data = realloc( temp->to_display, new_len * sizeof( char ) ); 
		if( temp_data != NULL )
		{
			temp->to_display = temp_data;
			strcpy( temp->to_display, buff );
		}
		
		new_len = strlen( buff ) + strlen( entry->d_name) + 2;
		temp_data = calloc( new_len, sizeof( char ) );
		snprintf( temp_data, new_len, "%s/%s", buff, entry->d_name );

		if( is_directory( temp_data ) )
			temp->is_dir = true;	
		free( temp_data );
		
/*j
		stat( temp_data, &file_stat );
		if( S_ISDIR( file_stat.st_mode ) )
			temp->is_dir = true;	
		free( temp_data );
	*/	
		temp->next = init_file();
		to_be_delete = temp;
		temp = temp->next;
	}
	closedir( directory );	
	if( tree->lines.list_of_lines[tree->cursor.y_index]->dcount == 0 )
	{
		free( tree->lines.list_of_lines[tree->cursor.y_index]->head );
		tree->lines.list_of_lines[tree->cursor.y_index]->head = NULL;
	}
	
	if( to_be_delete != NULL )
	{	
		free( to_be_delete->next->data );
		free( to_be_delete->next->to_display );
		free( to_be_delete->next );
		to_be_delete->next = NULL;
	}
	free( buff );
	return 0; 
}


void expand_tree_at_point_of_cursor( File_tree * tree, char * debug_message)
{
	if( strcmp( tree->lines.list_of_lines[tree->cursor.y_index]->data, ".." ) != 0 )
	{
		if( tree->lines.list_of_lines[tree->cursor.y_index]->dcount == 0 )
		{
			if( read_directory( tree ) == EACCES )
				strcpy( debug_message, "PERMISSION DENIED!" );
			sort_directory( tree->lines.list_of_lines[tree->cursor.y_index], false );
		}
		tree->lines.list_of_lines[tree->cursor.y_index]->expanded = true;
		update_file_tree_items( tree, &tree->lines );
	}



#ifdef _WIN32
	else if( strcmp( tree->working_directory, "\\" ) != 0 || strcmp( tree->working_directory, "/") != 0 )
	{
		i32 i = strlen( tree->working_directory );
		while( tree->working_directory[i] != '\\'  && tree->working_directory[i] != '/' )
			i--;
		if( (tree->working_directory[i] != '/' || tree->working_directory[i-1] != ':') &&  i > 0 )
			tree->working_directory[i] = '\0';
		else
			tree->working_directory[i+1] = '\0';
		reset_file_tree( tree );
	}

#elif __linux__
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
#endif
	return;
}


void change_dir_at_point_of_cursor( File_tree * tree, char * debug_message )
{
	
	if( tree->lines.list_of_lines[tree->cursor.y_index]->is_dir )
	{
		i32 new_len = strlen( tree->lines.list_of_lines[tree->cursor.y_index]->to_display ) + strlen( tree->lines.list_of_lines[tree->cursor.y_index]->data ) + 2;	
		char * temp_data = calloc( new_len, sizeof( char ) );
		snprintf( temp_data, new_len, "%s/%s", tree->lines.list_of_lines[tree->cursor.y_index]->to_display, tree->lines.list_of_lines[tree->cursor.y_index]->data );	
		errno = 0;
		DIR * directory = opendir( temp_data );
		if( directory == NULL && errno == EACCES )
		{
			strcpy( debug_message, "PERMISSION DENIED!" );
			return;
		}
		else if( directory == NULL )
		{
			strcpy( debug_message, "Something went wrong?!?!" );
			free( temp_data );
			return;
		}
		free( tree->working_directory );
		tree->working_directory = temp_data;
		reset_file_tree( tree );
		closedir( directory );
		return;
	}
	return;
}


void swap( Line_data * a, Line_data * b )
{	
	i32 dcount;
	i32 dcopacity;
	i32 count;
	bool is_dir;
	bool expanded;	
	char * temp;

	Line_data * head = NULL;
	
	head = a->head;
	a->head = b->head;
	b->head = head;
	
	temp = a->data;
	a->data = b->data;
	b->data = temp;

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
	for( long unsigned int i = 0; i < strlen( str ); i++ )
		str[i] = tolower( str[i] );
	return;
}


bool compare( Line_data * a, Line_data * b )
{
	if( a->is_dir > b->is_dir )
		return true;
	else if( a->is_dir < b->is_dir )
		return false;


	char * buff_1 = calloc( strlen( a->data ) + 1, sizeof( char ) );
	char * buff_2 = calloc( strlen( b->data ) + 1, sizeof( char ) );
	strcpy( buff_1, a->data );
	strcpy( buff_2, b->data );
	
	str_to_lower(buff_1);
	str_to_lower(buff_2);

	if( strcmp( buff_1, buff_2 ) < 0 )
	{
		free( buff_1 );
		free( buff_2 );
		return true;
	}
	free( buff_1 );
	free( buff_2 );
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

Line_data * init_file()
{
	Line_data * temp = calloc( 1, sizeof( Line_data ) );
	temp->next = NULL;
	temp->prev = NULL;
	temp->head = NULL;
	temp->data = NULL;
	temp->to_display = NULL;
	
	temp->copacity =0;
	temp->count = 0;
	temp->dcount = 0;
	temp->dcopacity = 0;

	temp->is_dir = false;
	temp->deleted = false;
	temp->expanded = false;
	
	return temp;
}

void read_working_dir( File_tree * tree )
{
	//resize_list( &tree->lines );
	DIR * directory;
	struct dirent * entry;
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
	char * temp_data = NULL;
	i32 new_len = 0;;
	while( ( entry = readdir( directory ) ) != NULL )
	{ 
		if( strcmp( entry->d_name, ".") == 0 )
			continue;	

		tree->lines.count++;
		temp->prev = prev;
		// save file name;	
		new_len = strlen( entry->d_name ) + 1;
		temp_data = calloc( new_len, sizeof( char ) );
		if( temp_data != NULL )
		{
			temp->data = temp_data;
			strcpy( temp->data, entry->d_name );
			temp->count = new_len - 1 ;
		}			
		// save working directory
		new_len = strlen( tree->working_directory ) + 1;
		temp_data = NULL;
		temp_data = calloc( new_len ,sizeof( char ) );
		
		if( temp_data != NULL )
		{
			temp->to_display = temp_data;
			strcpy( temp->to_display, tree->working_directory );
		}
		new_len = strlen( tree->working_directory ) + strlen( temp->data ) + 2;
		temp_data = calloc( new_len, sizeof( char ) );
		snprintf( temp_data, new_len , "%s/%s", tree->working_directory, entry->d_name );
		
		if( is_directory( temp_data ) )
			temp->is_dir = true;	
		free( temp_data );
		/*	
		stat( temp_data , &file_stat );
		if( S_ISDIR( file_stat.st_mode ) )
			temp->is_dir = true;
		free( temp_data );
		*/
		
		temp->next = init_file();
		to_be_delete = temp;
		temp = temp->next;
	}
	closedir( directory );	
	tree->lines.expanded_count = tree->lines.count;
	free( to_be_delete->next );
	to_be_delete->next = NULL;
	return; 
}


int get_working_dir( File_tree * tree )
{
	if( ( tree->working_directory = getcwd( NULL, 0 ) ) != NULL ) 
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
	//while( temp != NULL )
	{
		if( temp->is_dir )
			free_file_tree( temp, temp->dcount, 1 );
		next = temp->next;
		free( temp->data );
		free( temp->to_display);
		free( temp );
		temp = next;
	}
	return;
}


i32 check_dir_for_new_items( Line_data * head, const char * dir, const bool root )
{
	DIR * directory;
	struct dirent * entry;
	directory = opendir( dir );
	if( directory == NULL )
	{
		die( "check_dir_for_new_items(): unable to read directory" );
		return 0;
	}	
	Line_data * temp = NULL;
	Line_data * next = NULL;
	Line_data * prev = head;
	bool new;
	i32 new_count = 0;
	i32 new_len = 0;
	char * temp_data = NULL;
	while( ( entry = readdir( directory ) ) != NULL )
	{
		new = true;
		if( root )
		{
			if( strcmp( entry->d_name, "." ) == 0 )
				continue;
		 	temp = head->next;
		}
		else if( !root )
		{
			if( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, "..") == 0 )
				continue;
			temp = head->head;
		}

		while( temp != NULL )
		{
			if( strcmp( entry->d_name, temp->data ) == 0 )
			{
				new = false;
				break;
			}
			temp = temp->next;
		}
		if( new )
		{	
			new_count++;
			Line_data * new = init_file();
			new_len = strlen( entry->d_name ) + 1;
			temp_data = calloc( new_len, sizeof( char ) );
			if( temp_data != NULL )
			{
				new->data = temp_data;
				strcpy( new->data, entry->d_name );
				new->count = new_len - 1;
			}
			new_len = strlen( dir ) + 1;
			temp_data = calloc( new_len, sizeof( char ) );
			if( temp_data != NULL )
			{
				new->to_display = temp_data;
				strcpy( new->to_display, dir );
			}
			new_len = strlen( new->to_display ) +  strlen( new->data ) + 2;
			temp_data = calloc( new_len, sizeof( char ) );
			snprintf( temp_data, new_len, "%s/%s", new->to_display, new->data );
			
			if( is_directory( temp_data) )
				new->is_dir = true;
			free( temp_data );

			/*j
			stat( temp_data, &file_stat );
			if( S_ISDIR( file_stat.st_mode ) )
				new->is_dir = true;
			free( temp_data );
			*/
			if( root )
			{
				next = head->next;
				head->next = new;
				new->next = next;
				new->prev = prev;
			}
			else 
			{
				next = head->head;
				head->head = new;
				new->next = next;
				new->prev = prev;
			}
		}
	}
	closedir( directory );	
	return new_count; 
}


i32 delete_files_from_chain( Line_data * head, const bool root )
{
	Line_data * last, * temp, * current;
	bool first_iteration = true;
	i32 count = 0;
	if( root )
		current = head->next;
	else
		current = head->head;
	while( current != NULL )
	{
		if( current->deleted )	
		{
			count++;
			temp = current;
			current = current->next;
			if( temp->is_dir )
				free_file_tree( temp, temp->dcount, 1 );
			free( temp->data );
			free( temp->to_display );
			free( temp );
			if( first_iteration )
			{
				if( root )
					head->next = current;
				else
					head->head = current;
			}
			else
				last->next = current;	

//			last = current;
//			if( current != NULL )
//				current = current->next;
//			first_iteration = false;
	//		continue;
		}
		first_iteration = false;
		last = current;
		current = current->next;
	}	
	return count;
}


// this is ugly code :(
i32 check_dir_for_deleted_items( Line_data * head, const char * dir, const bool root )
{
	DIR * directory;
	struct dirent * entry;
	directory = opendir( dir );
	if( directory == NULL )
	{
		die( "check_dir_for_deleted_items(): unable to read directory" );
		return 0;
	}	

	Line_data * temp = NULL;
	if( root )
		 	temp = head->next;
	else
		temp = head->head;
	
	while( temp != NULL )
	{
		temp->deleted = true;
		temp = temp->next;
	}
	
	while( ( entry = readdir( directory ) ) != NULL )
	{
		if( root )
		{
			if( strcmp( entry->d_name, "." ) == 0 )
				continue;
		 	temp = head->next;
		}
		else if( !root )
		{
			if( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, "..") == 0 )
				continue;
			temp = head->head;
		}	
		while( temp != NULL )
		{
			if( strcmp( entry->d_name, temp->data ) == 0 )
			{
				temp->deleted = false;
				break;
			}
			temp = temp->next;
		}
	}
	closedir( directory );		
	return delete_files_from_chain( head, root );	
}


void _refresh_file_tree( Line_data * head, i32 count, bool root )
{
	Line_data * temp = NULL;
	if( root )
		temp = head->next;
	else
		temp = head->head;
	
	i32 len = 0;
	char * temp_data;

	for( i32 i = 0; i < count; i++ )
	{
		if( temp->is_dir && temp->expanded )
		{
			len = strlen( temp->to_display ) + strlen( temp->data ) + 2;
			temp_data = calloc( len, sizeof( char ) );
			snprintf( temp_data, len, "%s/%s", temp->to_display, temp->data );
			
			temp->dcount -= check_dir_for_deleted_items( temp, temp_data, false );
			temp->dcount += check_dir_for_new_items( temp, temp_data, false );
			free( temp_data );
			_refresh_file_tree( temp, temp->dcount, false );
			sort_directory( temp, false );
		}
		temp = temp->next;
	}
	return;
}


void refresh_file_tree( File_tree * tree )
{	
	tree->lines.count -= check_dir_for_deleted_items( tree->lines.head, tree->working_directory, true );
	tree->lines.count += check_dir_for_new_items( tree->lines.head, tree->working_directory, true );
	_refresh_file_tree( tree->lines.head, tree->lines.count, true );
	sort_directory( tree->lines.head, true );
	update_file_tree_items( tree, &tree->lines );
	return;
}


void reset_file_tree( File_tree * tree )
{
	free_file_tree( tree->lines.head, tree->lines.count, 0 );
	if( tree->lines.head->data != NULL )	
		free( tree->lines.head->data );
	if( tree->lines.head->to_display != NULL )	
		free( tree->lines.head->to_display );
	free( tree->lines.head );
	
	free( tree->lines.list_of_lines );
	init_cursor( &tree->cursor);
	tree->cursor.index = 1;
	tree->cursor.y_index = 1;
	tree->lines.head = init_file();
	if( tree->lines.head == NULL ) die( "could not allocate memory for file_tree" );
	tree->lines.list_of_lines = NULL; 
	tree->lines.head->next = init_file();
	read_working_dir( tree );
	sort_directory( tree->lines.head, true );
	update_file_tree_items( tree, &tree->lines );
	return;
}


void init_file_tree( File_tree * tree )
{
	init_cursor( &tree->cursor);
	tree->cursor.index = 1;
	tree->cursor.y_index = 1;
	if( get_working_dir( tree ) == 1 ) die( "could not get working directory. :( ");
	tree->lines.head = init_file();
	if( tree->lines.head == NULL ) die( "could not allocate memory for file_tree" );
	tree->lines.list_of_lines = NULL; 
	tree->lines.head->next = init_file(); 
	read_working_dir( tree );
	sort_directory( tree->lines.head, true );
	update_file_tree_items( tree, &tree->lines );
	return;
}


void toggle_file_tree()
{
	file_tree_toggle = ( file_tree_toggle == true )?false:true;
	return;
}
