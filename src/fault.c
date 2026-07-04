#include "include/fault.h"


void error_message( char * buffer, char * message )
{
	snprintf( buffer, 1000, "( %s )\n\r--------------------\n\r       o\n\r       o\n\r        o  @..@\n\r          (----)\n\r         ( )--( )\n\r          o0..0o\n\r", message );
	return;
}


void die( char * message )
{
	char buffer[1000];
	error_message( buffer, message );
	perror( buffer );
	exit( 1 );
	return;
}


void alert( char * message )
{
	perror( message );
	return;
}
