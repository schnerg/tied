#include "include/syntax.h"


char * keywords[] ={"break","case","continue","default","do","else","for","return","if","while","switch","sizeof","goto","bool"};
char *  types[] = {"float","char","signed","double","int","short","long","unsigned","void"};
char *storage_class[]= {"auto","const","enum","extern","register","static","struct","union" ,"volatile ","typedef"};


bool word_in_keywords(char * word)
{
	for(int i =0;i<14;i++)
	{
		if(strcmp(keywords[i],word)==0)
			return true;
	}
	return false;	
}


bool word_in_types(char * word)
{
	for(int i =0;i<9;i++)
	{
		if(strcmp(types[i],word)==0)
			return true;
	}
	return false;	

}


bool word_in_storage_class(char *word)
{
	for(int i =0;i<10;i++)
	{
		if(strcmp(storage_class[i],word)==0)
			return true;
	}
	return false;	
}



lexer_t * init_lexer( char * data, int len )
{
	lexer_t * lexer = calloc( 1, sizeof(lexer_t) );
	lexer->i = 0;
	lexer->len = len;
	lexer->data = data;
	lexer->is_string = false;
	lexer->update = false;
	lexer->is_comment = false;
	if( lexer->data != NULL )
		lexer->c = lexer->data[lexer->i];
	return lexer;
}


void advance_lexer( lexer_t * lexer )
{
	lexer->i++;
	lexer->c  = lexer->data[lexer->i];
	return;
}


void get_next_word(lexer_t * lexer,char * buffer,const int len)
{	
	int i = 0;
	buffer[i]	= lexer->c;
	advance_lexer(lexer);
	i++;
	while( lexer->i < len )
	{
		if(strchr( ",.()+-/*=~%<>[];:", lexer->c ) != NULL || isspace(lexer->c) )
			break;
		buffer[i] = lexer->c;
		advance_lexer(lexer);
		i++;	
	}
	buffer[i]= '\0';
	return;
}


void get_next_diget(lexer_t * lexer,char * buffer, const int len)
{
	int i =0;
	buffer[i]	= lexer->c;
	advance_lexer(lexer);
	i++;
	while(  lexer->i < len)
	{
		if(strchr(",.()+-/*=~%<>[];", lexer->c) || isspace(lexer->c))
			break;
		buffer[i] = lexer->c;
		advance_lexer(lexer);
		i++;	
	}
	buffer[i]= '\0';
	return;
}

char lexer_peak( lexer_t * lexer )
{
	int i = lexer->i + 1;
	if( i < lexer->len )
		return lexer->data[i];
	return 0;
}

char lexer_peak_skip_whitespace( lexer_t * lexer )
{
	int i = lexer->i;
	while( i < lexer->len )
	{
		if( isspace( lexer->data[i] ) )
			i++;
		else 
			return lexer->data[i];
	}
	return 0;
}

void append_proper( lexer_t * lexer, Buff * buffer,char * word)
{

	char c;
	if(word_in_keywords(word))
	{
		append_to_buffer(buffer,"\033[1;31m",7);
		append_to_buffer(buffer,word,strlen(word));
		append_to_buffer(buffer,"\033[0m",4);
	}
	else if(word_in_types(word))
	{
		append_to_buffer(buffer,"\033[1;36m",7);
		append_to_buffer(buffer,word,strlen(word));
		append_to_buffer(buffer,"\033[0m",4);
	}
	else if(word_in_storage_class(word))
	{
		append_to_buffer(buffer,"\033[1;34m",7);
		append_to_buffer(buffer,word,strlen(word));
		append_to_buffer(buffer,"\033[0m",4);
	}
	else if( ( c = lexer_peak_skip_whitespace( lexer ) ) == '(' )
	{
		append_to_buffer(buffer,"\033[1;34m",7);
		append_to_buffer(buffer,word,strlen(word));
		append_to_buffer(buffer,"\033[0m",4);
	}
	else	
	{
		append_to_buffer(buffer,word,strlen(word));
	}
}

void get_string(lexer_t * lexer,char * buffer, const int len)
{
	int i = 0;
	buffer[i] = lexer->c;
	do
	{
		advance_lexer(lexer);
		i++;	
		buffer[i] = lexer->c;
	}while( lexer->i < len && lexer->c !='"' );
	i++;	
	buffer[i]= '\0';
	return;
}


bool is_end_of_comment(lexer_t * lexer, char * buffer, int i )
{
	if( lexer->c == '*' && lexer_peak( lexer ) == '/' )
	{
		buffer[i++] = lexer->c;
		advance_lexer( lexer );
		buffer[i++] = lexer->c;
		buffer[i] = '\0';
		advance_lexer( lexer );
		return true;
	}
	return false;
}


void get_comment(lexer_t * lexer, char * buffer, const int len )
{
	int i = 0;
	if( is_end_of_comment( lexer, buffer, i ) )
		goto done;
	buffer[i++] = lexer->c;
	advance_lexer( lexer );
	while( lexer->i < len )
	{
		if( is_end_of_comment( lexer, buffer, i ) )
			goto done;
		buffer[i++] = lexer->c;
		advance_lexer(lexer);
	}
	buffer[i]= '\0';
done:
	return;
}


void syntax_highlighting(  Buff * buffer, Syntax * syntax, char * str, const int len )
{		
	
	if( syntax->lexer == NULL) 
		syntax->lexer = init_lexer( str, len );
	else
	{
		syntax->lexer->update = false;
		syntax->lexer->data = str;
		syntax->lexer->len = len;
		syntax->lexer->i = 0;
		if( str != NULL )
			syntax->lexer->c = str[0];
	}
	lexer_t * lexer = syntax->lexer;
	char * word = calloc( len + 1, sizeof( char ) );
	while( lexer->i < len )
	{	
		if( isalpha(lexer->c) )
		{
			get_next_word( lexer, word, len );
			append_proper( lexer, buffer, word );
			continue;
		}	
		if( isspace(lexer->c) )
		{
			append_to_buffer(buffer,&lexer->c,1);
			advance_lexer(lexer);
			continue;
		}
		if(lexer->c =='"')
		{
			get_string(lexer,word,len);
			append_to_buffer(buffer,"\033[1;35m",7);
			append_to_buffer(buffer,word,strlen(word));
			append_to_buffer(buffer,"\033[0m",4);
			advance_lexer(lexer);
			continue;
		}		
		if( lexer->c == '/')
		{
			if( lexer_peak( lexer ) == '/' )
			{
				int temp = len - lexer->i;
				append_to_buffer( buffer, "\033[0;90m",7 );
				append_to_buffer( buffer, &str[lexer->i], temp );
				append_to_buffer( buffer, "\033[0m", 4);
				break;
			}
		}
		if(isdigit(lexer->c))
		{
			get_next_diget(lexer,word,len);
			append_to_buffer(buffer,"\033[1;35m",7);
			append_to_buffer(buffer,word,strlen(word));
			append_to_buffer(buffer,"\033[0m",4);
			continue;
		}
		
		if(isprint(lexer->c))
		{
			append_to_buffer(buffer,&lexer->c,1);
			advance_lexer(lexer);
			continue;
		}
	}
	free( word );
	//free(lexer);
	return;
}

