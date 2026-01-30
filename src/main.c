#include "include/main.h"

int main(int argc, char * argv[])
{
	if( argc > 1 )
	{
		system( "clear" );
		Editor e;	
		init( &e, argv[1] );
		render( &e );
		while( !e.done )
		{
			events( &e );
		}
		quit( &e );
	}
	return 0;
}
