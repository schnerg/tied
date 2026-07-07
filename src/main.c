#include "include/main.h"


int main( int argc, char * argv[] )
{
	Editor e;
	e.file_name[0] = '\0';
	if( argc > 1 )
	{
		if( strlen( argv[1] ) < 255 && !is_directory( argv[1] ) )
			strcpy( e.file_name, argv[1] );	
		else if( is_directory( argv[1] ) )
		{
			errno = 1;
			die( "Cannot read file becuase it is a directory!" );
		}
		else
			die( "main(): File name too long." );
	}	
	init( &e );
	render( &e );
	while( e.done == false )
		events( &e );
	quit( &e );
	return 0;
}
