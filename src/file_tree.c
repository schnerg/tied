#include "include/file_tree.h"


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
	while( ( entry = readdir( directory ) ) )
	{ 
		temp->prev = prev;
		stat( entry->d_name, &file_stat );
		if( S_ISDIR( file_stat.st_mode ) )
			temp->is_dir = true;
		for( i = 0; i < 50 && i < strlen(entry->d_name); i++ )
			temp->data[i] = entry->d_name[i];
		temp->count = i;
		prev = temp;	
		init_line( temp );
		temp = temp->next;
		tree->lines.count++;
		resize_list( &tree->lines );
	}
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
	if( get_working_dir( tree ) == 1 ) die( "could not get working directory. :( ");
	tree->y_offset = 0;
	tree->lines.head = calloc( 1, sizeof( Line_data ) );
	if( tree->lines.head == NULL ) die( "could not allocate memory for file_tree" );
	
	tree->lines.list_of_lines = NULL; 
	init_line( tree->lines.head );
	read_working_dir( tree );
	file_tree_toggle = false;
	return;
}


void toggle_file_tree( )
{
	file_tree_toggle = ( file_tree_toggle == true )?false:true;
	return;
}
