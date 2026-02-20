#include "include/screen.h"


void enter_alt_screen()
{
	write( STDOUT_FILENO, "\x1b[?1049h", 8 );
	return;
}


void leave_alt_screen()
{
	write( STDOUT_FILENO, "\x1b[?1049l", 8 );
	return;
}


void disable_raw_mode( Window * window )
{
	leave_alt_screen();
	#ifdef _WIN32
		SetConsoleMode( window->hstdin, window->original_mode );
		HANDLE out = GetStdHandle( STD_OUTPUT_HANDLE );
		SetConsoleMode( out, window->original_mode );
	#elif __linux__
		tcsetattr( STDIN_FILENO, TCSAFLUSH, &window->orig_termios );
	#endif
	return;
}


void enable_Raw_mode( Window * window )
{
	enter_alt_screen();
	#ifdef _WIN32
		HANDLE in = GetStdHandle( STD_INPUT_HANDLE );
		HANDLE out = GetStdHandle( STD_OUTPUT_HANDLE );
		window->hstdin = in;
		DWORD vertual_terminal, raw; 
		GetConsoleMode( out, &vertual_terminal );
		GetConsoleMode( in, &window->original_mode );
		raw = window->original_mode & ~( ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS );
		vertual_terminal |=( ENABLE_VIRTUAL_TERMINAL_PROCESSING |  ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | DISABLE_NEWLINE_AUTO_RETURN) ;
		SetConsoleMode( out, vertual_terminal );
		SetConsoleMode( in, raw );
	#elif __linux__
		tcgetattr( STDIN_FILENO, &window->orig_termios );
		struct termios raw = window->orig_termios;
		raw.c_iflag &= ~( BRKINT | ICRNL | INPCK | ISTRIP | IXON );
		raw.c_oflag &= ~( OPOST );
		raw.c_cflag |= ( CS8 );
		raw.c_lflag &= ~( ECHO | ICANON | IEXTEN | ISIG );
		raw.c_cc[VMIN] = 0;
		raw.c_cc[VTIME] = 1;
		tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw);
	#endif
	return;
}


bool get_window_size( Window * window )
{
	int rows, cols;
	#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
	 	GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );
		cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	#elif __linux__
		struct winsize w;
		ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
		rows =  w.ws_row;
		cols = w.ws_col;
	#endif
		if( window->rows != rows || window->cols != cols )
		{
			window->rows =  rows;
			window->cols = cols;
			return true;  
		}
	return false;  
} 
void editorRefreshScreen() 
{
  write(STDOUT_FILENO, "\e[?25l", 6); // hide cursor
  write(STDOUT_FILENO, "\x1b[H", 3); // move cursor to top left of screen
	return;
}

void adjust_yx_offsets( Cursor * c, Window * window, int line_nums, Buff * cbuff  )
{
	if( c->rx > window->cols || c->rx < c->x_offset )
	{
		c->x_offset = 0;
		
 		index_to_rx( c, cbuff, line_nums );
		while( c->rx > window->cols )
		{
			c->x_offset += window->cols / 2 ;
 			index_to_rx( c, cbuff, line_nums );
		}
	}
	c->last_x_offset = c->x_offset;
	if( c->y_index - c->y_offset > window->rows || c->y_index < c->y_offset )
	{	
		c->y_offset = 0;
		while( c->y_index - c->y_offset > window->rows )
			c->y_offset += window->rows / 2 ;
	}
	c->last_y_offset = c->y_offset;
	return;
}
//TODO: rename function! >:(
void print_chars_to_screen( Buff * line_buff, Lines_data * lines, Cursor * c, Window * window, int line_nums )
{
	editorRefreshScreen();
	Buff * buffer = init_buffer();
	Line_data * temp = lines->list_of_lines[c->y_offset];
	char buff[window->cols + 1];
	int i = 0;
	int tabs = 0;
	for( int y = 0; y < window->rows - 1; y++ )
	{
		if( y < lines->count - c->y_offset )
		{	
			append_to_buffer( buffer, "\x1b[K", 3 ); //clear rowl;
			//append line num;
			char line_num[40];
			snprintf( line_num, 40, "%i", y + c->y_offset );
			int line_padding  = line_nums - strlen( line_num );
			for( int i =0; i < line_padding; i++ )
				append_to_buffer( buffer, " ", 1 );
			append_to_buffer( buffer, "\033[0;33m", 7 );
			append_to_buffer( buffer, line_num, strlen(line_num) );
			append_to_buffer( buffer, "\033[0m", 4 );
			append_to_buffer( buffer, " ", 1 );
			
			//buffer stuff!	
			if( y == c->y_index - c->y_offset )
			{
				int len = line_buff->dcount - c->x_offset;
				if( len < 0 ) 
					len = 0;
				if( len > window->cols -( line_nums + 1) )
					len = window->cols - (line_nums + 1);
				append_to_buffer( buffer, &line_buff->to_display[c->x_offset], len );
			}
			//other line stuff	
			else
			{
				int len = temp->dcount - c->x_offset;
				if( len < 0 ) len = 0;
				if( len > window->cols - ( line_nums + 1 ) ) len = window->cols - ( line_nums + 1 );
				append_to_buffer( buffer, &temp->to_display[ c->x_offset], len );
			}
			append_to_buffer( buffer, "\x1b[K", 3 ); ////clear rowl;
		
			// print new line at end of line!
		if( y < window->rows - 2 )
				append_to_buffer( buffer, "\n\r", 2 );
			temp = temp->next;
		}
		else
		{
			if( y < window->rows - 1 )
			{
				append_to_buffer( buffer, "\x1b[K", 3 ); //clear rowl;
				append_to_buffer( buffer, "~\n\r", 3 );
			}
		}
	}
	append_to_buffer(buffer, "\e[?25h",6); // show cursor	
	write( STDOUT_FILENO, buffer->contents, buffer->count );
	free( buffer->contents );
	free( buffer );
	return;	
}


void print_mode( Window * window, int mode, char * debug_message )
{
	char bottom[40];
	sprintf( bottom, "\x1b[%d;%dH", window->rows, 1 );
	write( STDOUT_FILENO, bottom, strlen( bottom ) );
	char clear_row[40];
	sprintf( clear_row, "\x1b[K" );
	write( STDOUT_FILENO, clear_row, strlen( clear_row ) );
	char buff[80];
	if( mode == 0 ) // normal mode
		sprintf( buff, "\033[33;44mNORMAL MODE \033[0m%s", debug_message );
	else if( mode == 1 ) // insert mode
		sprintf( buff, "\033[33;44mINSERT MODE \033[0m%s", debug_message );
	write( STDOUT_FILENO, buff, strlen( buff ) );
	sprintf( buff, "\033[0m" );
	write( STDOUT_FILENO, buff, strlen( buff ) );
	return;
}
