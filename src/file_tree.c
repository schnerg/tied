#include "include/file_tree.h"


void expand_tree_at_point_of_cursor()
{
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
		tree->lines.count++;
		temp->prev = prev;	
		
		for( i = 0; i < 50 && i < strlen(entry->d_name); i++ )
			temp->data[i] = entry->d_name[i];	
		
		if( i >= 50 )
			temp->data[i] = '\0';
		else
			temp->data[i+1] = '\0';
		
		stat( entry->d_name, &file_stat );
		if( S_ISDIR( file_stat.st_mode ) )
			temp->is_dir = true;
		
		temp->count = i ;
		prev = temp;	
		init_line( temp );
		temp = temp->next;
		resize_list( &tree->lines );
	}

//	tree->lines.count--;
//	if( tree->lines.count == 0 )
//		tree->lines.count++;
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
	read_working_dir( tree );
	sort_file_tree( tree );
	file_tree_toggle = false;
	return;
}


void toggle_file_tree()
{
	file_tree_toggle = ( file_tree_toggle == true )?false:true;
	return;
}
