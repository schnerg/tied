#include "include/file_tree.h"


int get_working_dir( File_tree * tree )
{
	if( getcwd( tree->working_directory, 1024 ) != NULL ) 
		return 0;
	return 1;
}


void init_file_tree( File_tree * tree )
{
	get_working_dir( tree );
	file_tree_toggle = false;
	return;
}


void toggle_file_tree( )
{
	file_tree_toggle = ( file_tree_toggle == true )?false:true;
	return;
}
