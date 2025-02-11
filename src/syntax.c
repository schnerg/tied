#include "include/syntax.h"

char * keywords[] ={"break","case","continue","default","do","else","for","return","if","while","switch","sizeof","goto","bool"};
char*  types[] = {"float","char","signed","double","int","short","long","unsigned","void"};
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
typedef struct
{
	unsigned int i;	
	char c;
	char * data;
}lexer_t;


lexer_t * init_lexer(char * data)
{
	lexer_t * lexer = calloc(1,sizeof(lexer_t));
	lexer->i =0;
	lexer->data = data;
	lexer->c= lexer->data[lexer->i];
	return lexer;
}
void advance_lexer(lexer_t * lexer)
{
	lexer->i++;
	lexer->c  = lexer->data[lexer->i];
	return;
}


void get_next_word(lexer_t * lexer,char * buffer,const int len)
{	
	int i =0;
	buffer[i]	= lexer->c;
	advance_lexer(lexer);
	i++;
	while(lexer->i<len)
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

void append_proper(Buff * buffer,char * word)
{
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
	else	
	{
		append_to_buffer(buffer,word,strlen(word));
	}
}

void get_string(lexer_t * lexer,char * buffer, const int len)
{
	int i =0;
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


void syntax_highlighting(Buff * buffer, char * str, const int len)
{
	lexer_t * lexer = init_lexer(str);
	char word[len];
	while(lexer->i < len )
	{
		if(isalpha(lexer->c))
		{
			get_next_word(lexer,word,len);
			append_proper(buffer,word);
			continue;
		}	
		if(isspace(lexer->c))
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
	free(lexer);
}


