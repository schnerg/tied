#include "include/main.h"


int main( int argc, char * argv[] )
{
	Editor e;
	e.file_name[0] = '\0';
	if( argc > 1 )
	{
		if( strlen( argv[1] ) < 255 )
			strcpy( e.file_name, argv[1] );	
		else
			die( "main(): File name too long." );
	}	
		
	init( &e );
	render( &e );
	while( e.done == false )
	{
		events( &e );
	}
	quit( &e );
	

	//	else
	//	printf("Usage:\ntied <path/to/file.txt>\n");
	return 0;
}
