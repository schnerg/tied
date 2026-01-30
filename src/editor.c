#include "include/editor.h"

#define TAB_STOP 4
#define CTRL_KEY(k) ((k) & 0x1f)




void die(const char *s)
{
	perror(s);
	exit(1);
}

void set_debug_message(Editor *e,char * s)
{
	strcpy(e->debug_message, s);
	return;
}

void disable_raw_mode(Editor * e)
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &e->window.orig_termios) == -1)
    die("tcsetattr");
}



void enable_Raw_mode(Editor *  e) {
  if (tcgetattr(STDIN_FILENO, &e->window.orig_termios) == -1) die("tcgetattr");
  struct termios raw = e->window.orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}


bool get_window_size(Editor *e)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	if(e->window.rows!= w.ws_row || e->window.cols != w.ws_col)
	{
		e->window.rows =  w.ws_row;
		e->window.cols = w.ws_col;
		return true;  
	}
  return false;  
} 




void realloc_data( Line_data * temp )
{
	temp->copacity *=2;
	temp->data = realloc( temp->data, temp->copacity * sizeof( char ) );
}


void init_line(Editor * e,Line_data * temp)
{
	e->lines.count++;
	temp->next = calloc(1,sizeof(Line_data));
	temp->next->data = calloc(50,sizeof(char));
	temp->next->copacity = 50;
	temp->next->count = 0;
	temp->next->next = NULL;
}


void resize_list( Editor * e )
{
	if( e->lines.list_of_lienes == NULL )
	{
		e->lines.copacity = 10;
		e->lines.list_of_lienes = calloc(e->lines.copacity,sizeof(Line_data*));
	}
	else
	{
		if( e->lines.count >= e->lines.copacity -1 )
		{
			e->lines.copacity *= 2;
			e->lines.list_of_lienes = realloc( e->lines.list_of_lienes, e->lines.copacity * sizeof( Line_data* ) );
		}
	}
	return;	
}


void save_file(Editor *e )
{
	FILE * fp = fopen(e->file_name,"w");
	if(fp==NULL) die("file read failed?");
	Line_data * temp = e->lines.head;
	for(int row =0; row < e->lines.count;row++)
	{
		for(int col =0; col < temp->count ;col++)
		{
			fputc(temp->data[col],fp);
		}
		temp = temp->next;
		if(row < e->lines.count )
			fputc('\n',fp);
	}
	fclose(fp);
	set_debug_message(e,": file saved");
	e->saved = true;
	return;
}

void save_file_name(Editor * e, char * file_name)
{
	if(strlen(file_name) < 255)
	{
		strcpy(e->file_name,file_name);	
	}
	else 
		die("lenght of filename is too long");
	return;
}

//this is getting cluseter fucked
int read_file( Editor *e, char * file_name )
{
	save_file_name( e, file_name );
	e->lines.list_of_lienes = NULL;
	resize_list( e );
	e->lines.head = calloc( 1, sizeof( Line_data ) );
	e->lines.count =1;
	e->lines.head->next=NULL;
	e->lines.head->prev = NULL;
	e->lines.head->data = calloc(50,sizeof(char));
	e->lines.head->count = 0;
	e->lines.head->copacity = 50;
	Line_data * prev = e->lines.head;
	Line_data * temp = e->lines.head;
	e->lines.list_of_lienes[ e->lines.count - 1 ] = temp;
	resize_list( e );
	FILE * fp = fopen( file_name, "r" );
	if(fp == NULL)
		return 1;
	int c;	
	register unsigned int i = 0;
	while((c=fgetc(fp)) !=EOF)
	{
		if(c=='\n')
		{
			prev = temp;
			init_line(e,temp);
			resize_list(e);
			temp = temp->next;
			temp->prev = prev;
			e->lines.list_of_lienes[e->lines.count -1] = temp;
			i=0;
			continue;
		}
		if(temp->count==temp->copacity -1)
		{
			realloc_data(temp);
		}
		temp->data[i] = c;
		temp->count++;
		i++;
	}
	e->lines.count--;
	fclose(fp);
	return 0;
}



void init_cursor(Editor * e)
{
	e->cursor.rx=0;
	e->cursor.y_index=0;
	e->cursor.last_index=0;
	e->cursor.index=0;
	e->cursor.last_x_offset = 0;	
	e->cursor.last_y_offset = 0;	
	e->cursor.x_offset =0;
	e->cursor.y_offset =0;
	return;
}

void init_settings(Editor *e)
{
	e->saved = true;
	e->mode = NORMAL;
	return;
}
void init( Editor * e, char * file_name )
{
	enable_Raw_mode(e);
	init_settings(e);
	get_window_size(e);
	set_debug_message(e,"");
	e->done =false;
	e->tabs =0;
	e->tabs_space =0;
	if(read_file(e,file_name))
	{
		set_debug_message(e,": new buffer created");
	}
	init_cursor(e);
	return;	
}


void index_to_rx(Editor *e)
{
	e->cursor.rx =0;
	for(int i =e->cursor.x_offset; i < e->cursor.index ;i++)
	{ 
		if(e->lines.list_of_lienes[e->cursor.y_index]->data[i] == 9)
			e->cursor.rx += (TAB_STOP -1) - (e->cursor.rx % TAB_STOP);
		e->cursor.rx++;
	}
	e->cursor.rx += e->line_nums +1;	
	return;
}

void update(Editor * e)
{
	char buf[40];
	snprintf(buf,40,"%i",e->lines.count);
	e->line_nums = strlen(buf);
	return;
}


void update_cursor(Editor * e)
{
	update(e);
	e->cursor.index = e->cursor.last_index;
	if(e->cursor.index > e->lines.list_of_lienes[e->cursor.y_index]->count)
	{
		while(e->cursor.index > e->lines.list_of_lienes[e->cursor.y_index]->count)
		{
			e->cursor.index--;
		}
	}
	index_to_rx(e);
	e->cursor.x_offset = e->cursor.last_x_offset;
	e->cursor.y_offset = e->cursor.last_y_offset;
	if(e->cursor.rx+ e->cursor.x_offset > e->lines.list_of_lienes[e->cursor.y_index]->count-1)
	{	
		if(e->lines.list_of_lienes[e->cursor.y_index]->count-1 - e->cursor.x_offset <= 0)
			while(e->cursor.x_offset > e->lines.list_of_lienes[e->cursor.y_index]->count-1 && e->cursor.x_offset >0 )
			{
				e->cursor.x_offset -= e->window.cols - (e->tabs * TAB_STOP) - e->line_nums;
			}
		if(e->cursor.x_offset <0)
			e->cursor.x_offset = 0;
	}
	char bar[] = "\e[5 q"	;
	char block[] = "\e[1 q"	;
	if(e->mode == NORMAL)
		write(STDOUT_FILENO,block,strlen(block));
	if(e->mode == INSERT)
		write(STDOUT_FILENO,bar,strlen(bar));
	return;
}

void print_cursor(Editor * e)
{
	char buff[40];
	sprintf(buff, "\x1b[%d;%dH", e->cursor.y_index - e->cursor.y_offset + 1, e->cursor.rx + 1);
	write(0,buff,strlen(buff));
	return;
}

void editorRefreshScreen() {
	//system("clear");
	//"\e[?25l"
  write(STDOUT_FILENO, "\e[?25l", 6);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void correct_string_becuase_of_tabs(Editor * e,char * string_to_print, char * buff)
{ 
	int j =0; 
	for(int i =0; i< strlen(buff) ;i++)
	{
		if(buff[i] == '\t')
		{
			string_to_print[j] = ' ';
			j++;
			while(j % TAB_STOP != 0 )
			{
				string_to_print[j] = ' ';
				j++;
			}
		}
		else
		{
			string_to_print[j] = buff[i];
			j++;
		}
	}
	string_to_print[j]='\0';
	return;	
}


Buff * init_buffer(void )
{
	Buff * buff = calloc(1,sizeof(Buff));
	buff->count = 0;
	buff->copacity = 50;
	buff->contents = calloc(buff->copacity, sizeof(char));
	return buff;	
}


void resize_buffer(Buff * buff)
{
	buff->copacity *=2;
	buff->contents = realloc(buff->contents,buff->copacity * sizeof(char));
	return;
}


void append_to_buffer(Buff * buff,char * str, int size)
{
	while(buff->count + size >= buff->copacity-1)
			resize_buffer(buff);
	int j = buff->count;
	//I dont' under stand what is wrong with this code!	
	
	for(int i =0; i < size; i++)
	{
		buff->contents[j] = str[i];
		j++;
	}
	buff->contents[j]='\0';
	buff->count += size;
}


int calculate_tabs_space(Editor * e,int tabs,int  line_num)
{
	int j =0;
	for(int i =0; i <= tabs; i++)
			j += (TAB_STOP -1) - (j % TAB_STOP);
	return j;
}



void print_chars_to_screen(Editor * e)
{
	editorRefreshScreen();
	Buff * buffer = init_buffer();
	//somehow this line below is effecting things.
	Line_data * temp = e->lines.list_of_lienes[e->cursor.y_offset];
	//Line_data * temp = e->lines.head;


	char buff[e->window.cols+1];
	int i =0;
	int tabs = 0;
	for(int y =0; y < e->window.rows-1; y++)
	{
		if( y < e->lines.count - e->cursor.y_offset )
		{
			tabs=0;
			i=0;
			for(int x = e->cursor.x_offset; x < e->window.cols  + e->cursor.x_offset && x < temp->count ; x++)
			{
				buff[i] = temp->data[x];
				if(temp->data[x] ==9)
					tabs++;
				i++;
			}
			if(tabs > e->tabs)
				e->tabs = tabs;
			buff[i] ='\0';
			e->tabs_space = calculate_tabs_space(e,tabs,y+ e->cursor.y_offset);
			
			append_to_buffer(buffer,"\x1b[K",3); //clear rowl;
			
			char string_to_print[strlen(buff) + tabs *(TAB_STOP -1 ) +1];
			correct_string_becuase_of_tabs(e,string_to_print,buff)	;
		
			char line_num[40];
			snprintf(line_num,40,"%i",y +e->cursor.y_offset);
		
			int len = strlen(string_to_print);
			
			int line_padding  = e->line_nums - strlen(line_num);
			
			if(len >= (e->window.cols - e->line_nums - e->tabs_space)  ) len = e->window.cols -  e->line_nums - e->tabs_space +2;

			for(int i =0; i< line_padding; i++)
				append_to_buffer(buffer," ",1);
			append_to_buffer(buffer,"\033[0;33m",7);
			append_to_buffer(buffer,line_num,strlen(line_num));
			append_to_buffer(buffer,"\033[0m",4);
			append_to_buffer(buffer," ",1);
		
			
		if(strstr(e->file_name, ".c" ))
				syntax_highlighting(buffer,string_to_print,len);
		else	
			append_to_buffer(buffer,string_to_print,len);

			append_to_buffer(buffer,"\x1b[K",3); //clear rowl;
			if(y < e->window.rows-2)
				append_to_buffer(buffer,"\n\r",2);
			temp = temp->next;
		}
		else
		{
			if(y < e->window.rows-2)
			{
				append_to_buffer(buffer,"\x1b[K",3); //clear rowl;
				append_to_buffer(buffer,"~\n\r",3);
			}
		}
	}
	append_to_buffer(buffer, "\e[?25h",6);	
	write(STDOUT_FILENO,buffer->contents,buffer->count);
	free(buffer->contents);
	free(buffer);
	return;	
}


void print_mode(Editor *e)
{
	char bottom[40];
	sprintf(bottom, "\x1b[%d;%dH", e->window.rows,0);
	write(0,bottom,strlen(bottom));
	//write(STDOUT_FILENO,"\n\r",2);
	
	char clear_row[40];
	sprintf(clear_row,"\x1b[K");
	write(STDOUT_FILENO,clear_row,strlen(clear_row));

	char buff[80];
	if( e->mode == NORMAL )
		sprintf( buff,"\033[33;44mNORMAL MODE \033[0m%s",e->debug_message);
	else if( e->mode == INSERT )
		sprintf(buff,"\033[33;44mINSERT MODE \033[0m%s",e->debug_message);
	write(STDIN_FILENO,buff,strlen(buff));
	sprintf(buff,"\033[0m");
	write(STDIN_FILENO,buff,strlen(buff));
	return;
}

void render(Editor*e)
{
	update_cursor(e);
	print_chars_to_screen(e);
	print_mode(e);
	print_cursor(e);
	return;
}

void adjust(Editor * e)
{
	// when window resizes, adjust x_offset so that cursor is still on screen in proper pos;
	// this is hard to read
	if(e->cursor.index - e->cursor.x_offset > e->window.cols || e->cursor.index < e->cursor.x_offset)
	{
		e->cursor.x_offset =0;
		while(e->cursor.index - e->cursor.x_offset > e->window.cols)
		{
			e->cursor.x_offset += e->window.cols - (e->tabs * TAB_STOP) - e->line_nums;
		}
	}
	e->cursor.last_x_offset = e->cursor.x_offset;
	e->cursor.last_index = e->cursor.index;
	//adjust y_offset so cursor is still on screen when resized;
	if(e->cursor.y_index - e->cursor.y_offset > e->window.rows ||e->cursor.y_index < e->cursor.y_offset )
	{	
		e->cursor.y_offset = 0;
		while(e->cursor.y_index - e->cursor.y_offset > e->window.rows)
			e->cursor.y_offset += e->window.rows;
		e->cursor.last_y_offset = e->cursor.y_offset;
	}
		e->cursor.last_y_offset = e->cursor.y_offset;
}


char getch() 
{
  int nread;
  char c = -1;
  nread = read(STDIN_FILENO, &c, 1);
  return c;
}

void insert_char(Editor * e, char c)
{
	Line_data * temp = e->lines.list_of_lienes[e->cursor.y_index];
	temp->count++;
	if(temp->count >= temp->copacity)
		realloc_data( temp );
		char old;
	int i=e->cursor.index;
	while(i< temp->count)
	{	
		old = temp->data[i];
		temp->data[i]=c;
		i++;
		c = old;
	}
	e->cursor.index++;
	e->saved = false;
	set_debug_message(e,"");
	adjust(e);
	render(e);
	return;
}


void remove_line( Editor * e )
{
	Line_data * temp = e->lines.list_of_lienes[e->cursor.y_index];
	Line_data * line_to_appended_to = temp->prev;
	Line_data * next_line = temp->next;


	//this is is pointing to the wrong address
	line_to_appended_to->next = next_line;


	if( next_line != NULL )
	{
		next_line->prev = line_to_appended_to;
	}
	while( line_to_appended_to->count + temp->count >= line_to_appended_to->copacity )
	{
		realloc_data( line_to_appended_to );
	}
	int k=0;
	for(int  j = line_to_appended_to->count; j < line_to_appended_to->count + temp->count; j++ )
	{
		line_to_appended_to->data[j] = temp->data[k];
		k++;
	}
	e->lines.count--;
	line_to_appended_to->count += temp->count;
	free( temp );
	return;
}


void list_of_lienes(Editor * e);


void backspace(Editor* e,char c)
{
	if( e->cursor.index > 0 )
	{
		e->cursor.index--;	
		Line_data * temp = e->lines.list_of_lienes[e->cursor.y_index];
		temp->count--;	
		for( int i = e->cursor.index; i < temp->count; i++ )
		{
			temp->data[i] = temp->data[i+1];
		}
	}
	else if( e->cursor.y_index > 0 )
	{
		e->cursor.index = e->lines.list_of_lienes[e->cursor.y_index-1]->count;
		e->cursor.last_index = e->cursor.index;
		remove_line( e );
		list_of_lienes( e );
		e->cursor.y_index--;
		system( "clear" );
	}
	adjust( e );
	render( e );
	return;
}

void list_of_lienes( Editor * e )
{
	resize_list( e );
	Line_data * temp = e->lines.head;
	for( int i = 0; i < e->lines.count; i++ )
	{
		e->lines.list_of_lienes[i] = temp;
		temp = temp->next;
	}
	return;	
}


void add_new_line( Editor * e, char * data, int size_of_data )
{
	Line_data * current_line  = e->lines.list_of_lienes[e->cursor.y_index]; // 0
	Line_data * new_line = calloc( 1, sizeof( Line_data ) );
	Line_data * next_line = current_line->next;
	current_line->next = new_line;
	new_line->prev = current_line;
	new_line->next = next_line;
	if( next_line != NULL )
		next_line->prev = new_line;
	new_line->count = size_of_data;
	new_line->copacity = size_of_data + 50 * 2;
	new_line->data = calloc( new_line->copacity, sizeof( char ) );
	for( int i = 0; i < size_of_data; i++ )
	{
		new_line->data[i] = data[i];
	}
	return;
}


void move_cursor_down( Editor * e );



void enter_key( Editor * e, char c )
{
	Line_data* temp = e->lines.list_of_lienes[ e->cursor.y_index ];
	char buff[ temp->count - e->cursor.index ];
		int j =0;
	for( int i = e->cursor.index; i < temp->count; i++ )
	{
		buff[j] = temp->data[i];
		j++;
	}
	add_new_line( e, buff, j );
	temp->count = e->cursor.index;
	e->cursor.index = 0;
	e->lines.count++;
	e->saved = false;
	list_of_lienes( e );
	move_cursor_down( e );	
	adjust( e );
	render( e );
	return;
}


//TODO: refactor to deal with commands
void search( Editor * e )
{
	// move curor down to menu bar
	char buff[40];
	sprintf(buff, "\x1b[%d;%dH", e->window.rows, 13);
	write(0,buff,strlen(buff));
	// getstring()
	int i = 0;	
	char c; 
	char back = '\b';
	while( ( c = getch() ) != 13 && i < 40 )	
	{	
		if ( c == 127 )// backspace
		{
			if(i > 0)
			{
				i--;
				write(STDOUT_FILENO, &back, 1);
				write(STDOUT_FILENO, " ", 1);
				write(STDOUT_FILENO, &back, 1);
			}
		}
		else if( isprint( c ) )
		{
			write(STDOUT_FILENO, &c, 1);
			buff[i] = c;
			i++;
		}
	}
	buff[i] = '\0';
	// search()
	char * index = NULL;
	for( int i = e->cursor.y_index; i < e->lines.count; i++ )
	{
		index = strstr( e->lines.list_of_lienes[i]->data, buff );
		if( index != NULL )
		{
			e->cursor.y_index = i; 
			e->cursor.index = index - e->lines.list_of_lienes[i]->data;
			e->cursor.last_index = e->cursor.index;
			e->cursor.last_x_offset = e->cursor.x_offset;
			render( e ); 
			break;	
		}
	}
	if(index== NULL)
	{
		set_debug_message( e, "reached bottom: searching from top" );	
		for( int i = 0; i < e->cursor.y_index; i++ )
		{
			index = strstr( e->lines.list_of_lienes[i]->data, buff );
			if( index != NULL )
			{
				e->cursor.y_index = i; 
				e->cursor.index = index - e->lines.list_of_lienes[i]->data;
				e->cursor.last_index = e->cursor.index;
				e->cursor.last_x_offset = e->cursor.x_offset;
				render( e ); 
				break;	
			}
		}
	}
	render( e ); 
	return;
}



void move_cursor_up( Editor * e )
{
	if(e->cursor.y_index >0)
		e->cursor.y_index--;
	if( e->cursor.y_offset > 0 && e->cursor.y_index - e->cursor.y_offset < 0 )
		e->cursor.y_offset--;
	e->cursor.last_y_offset = e->cursor.y_offset;
	render(e);
	return;
}
void move_cursor_down( Editor * e )
{
	if(e->cursor.y_index < e->lines.count-1)
		e->cursor.y_index++;
	if(e->cursor.y_index - e->cursor.y_offset == e->window.rows -1)	
		e->cursor.y_offset++;
	e->cursor.last_y_offset = e->cursor.y_offset;
	render(e);
	return;
}
void move_cursor_left( Editor * e )
{
	if(e->cursor.index >0)
		e->cursor.index--;
	if(e->cursor.rx == e->line_nums+1 && e->cursor.x_offset >0)
		e->cursor.x_offset -= e->window.cols - (e->tabs * TAB_STOP) - e->line_nums; 
	if(e->cursor.x_offset <0) e->cursor.x_offset = 0;
	e->cursor.last_index = e->cursor.index;
	e->cursor.last_x_offset = e->cursor.x_offset;
	render(e);
	return;
}
void move_cursor_right( Editor * e )
{
	if(e->cursor.index <= e->lines.list_of_lienes[e->cursor.y_index]->count )
		e->cursor.index++;
	if(e->cursor.rx >= e->window.cols)			
		e->cursor.x_offset += e->window.cols - (e->tabs * TAB_STOP) - e->line_nums; 
	e->cursor.last_index = e->cursor.index;
	e->cursor.last_x_offset = e->cursor.x_offset;

	render(e);
	return;
}


void events_insert( Editor * e )
{
	char c = getch();
	switch( c )
	{
		case 27: // escape
		{
			char temp;
			if( temp = getch() != 0 )
				temp = getch();
			switch( temp )
			{
				case 'A': move_cursor_up( e );break;
				case 'B': move_cursor_down( e );break;
				case 'C': move_cursor_right( e );break;
				case 'D': move_cursor_left( e );break;
				default: 
				{
					e->mode = NORMAL;
					update_cursor( e );
				}break;
			}
		}break;
		case 13: enter_key(e,c);break; //enter key
		case 127: backspace(e,c); break;
		default:
		{
			if(isprint(c) ||c=='\t')
				insert_char(e,c);
			render(e);
		}break;
	}
	return;
}


void events_normal( Editor * e )
{	
	char c=getch();
	switch(c)
	{
		case '/':
		{
			search( e );
		}break;
		case 'o':
		{
			e->cursor.index = e->lines.list_of_lienes[e->cursor.y_index]->count;
			enter_key(e,c);
			e->mode = INSERT;
		}break;
		case 'i': e->mode = INSERT; break;
		case 27: // escape
		{
			char temp;
			if( temp = getch() != 0 )
				temp = getch();
			switch( temp )
			{
				case 'A': move_cursor_up( e );break;
				case 'B': move_cursor_down( e );break;
				case 'C': move_cursor_right( e );break;
				case 'D': move_cursor_left( e );break;
				default: e->mode = NORMAL;break;
			}
		}break;
		case CTRL_KEY( 'c'):
		{
			//shift_everything();
		}break;

		case CTRL_KEY('s'):
		{
				save_file( e);
				e->saved = true;
		}break;
		case CTRL_KEY('q'):
		{
			if(e->saved == true)
				e->done = true;
			else
				set_debug_message(e,": no write since last change");
		}break;
		case CTRL_KEY('x'):
		{
			e->done = true;
		}break;
		case 13:// enter key
		{
			move_cursor_down( e );
		}break;
		case 'k': move_cursor_up( e );	 break;
		case 'j': move_cursor_down( e ); break; 
		case 'h': move_cursor_left( e ); break;
		case 'l': move_cursor_right( e );break;
	}
	
		return;
}


void events( Editor * e )
{
	switch( e->mode )
	{
		case NORMAL:
		{
			events_normal( e );
		}break;
		case INSERT:
		{
			events_insert( e );
		}break;
	}
	if( get_window_size( e ) )
		adjust( e );

	return;
}


void quit(Editor * e)
{
	free(e->lines.list_of_lienes);
	Line_data * prev = NULL;
	Line_data * temp = e->lines.head ;
	for(int i =0; i< e->lines.count;i++)
	{
		prev = temp;
		temp= temp->next;
		free(prev);
	}
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	disable_raw_mode(e);
	return;
}


