#include "include/fault.h"


void error_message( char * buffer, char * message )
{
	snprintf( buffer, 1000, "( %s )\n--------------------\n       o\n       o\n        o  @..@\n          (----)\n         ( )--( )\n          o0..0o\n", message );
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
