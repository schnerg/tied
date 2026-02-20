#include "include/main.h"

int main( int argc, char * argv[] )
{
	if( argc > 1 )
	{
		Editor e;	
		if( strlen( argv[1] ) < 255 )
			strcpy( e.file_name, argv[1] );	
		else
			exit( 1 );
		init( &e  );
		render( &e );
		while( e.done == false )
		{
			events( &e );
		}
		quit( &e );
	}
	return 0;
}
