#include "include/main.h"


int main( int argc, char * argv[] )
{
	Editor e;
	e.file_name = NULL;
	if( argc > 1 )
	{
		if( !is_directory( argv[1] ) )
		{
			int length = strlen( argv[1] ) + 1;
			e.file_name = calloc( length, sizeof( char ) );
			strcpy( e.file_name, argv[1] );	
		}
		else if( is_directory( argv[1] ) )
		{
			errno = 1;
			die( "Cannot read file becuase it is a directory!" );
		}
	}
	init( &e );
	render( &e );
	while( e.done == false )
		events( &e );
	quit( &e );
	return 0;
}
