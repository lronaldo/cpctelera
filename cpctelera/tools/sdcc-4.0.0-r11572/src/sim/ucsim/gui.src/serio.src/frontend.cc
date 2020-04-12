/******************************************************************************
 * to emulate the serial input and output of an 8051 controller               *
 * frontend.cc - the ncurses frontend                                         *
 ******************************************************************************/

#include <stdio.h>
#include <ctype.h>
//#include <sys/types.h>
//#include <iostream>
#include <stdlib.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <curses.h>
//#include <errno.h>
#include <string.h>
//#include <unistd.h>

#include "frontend.hh"

char *
flt_name(enum filter_t f)
{
  switch (f)
    {
    case flt_none	: return (char*)"none";
    case flt_hex	: return (char*)"hex";
    }
  return (char*)"unknown";
}

Viewer::Viewer()
{
  /* initalise the output screen */
  initscr();
  cbreak();
  noecho();
  nl();
  intrflush(stdscr,FALSE);
  keypad(stdscr, TRUE);

  flt_in= flt_none;
  flt_out= flt_none;
  ohex_ptr= 0;
  ocnt= icnt= 0;
  line_length= 8;
  
  /* clear the screen and off you go */
  refresh();

  // get the coordinates for the box
  /* create the subwindow */
  win_c.min_x = win_c.min_y = 0;
  getmaxyx(stdscr, win_c.max_y, win_c.max_x);
  
  /* define the boxed size */
  topleft.x = win_c.min_x + 1;
  bottomright.x = win_c.max_x - 2;
  topleft.y = win_c.min_y + 1;
  bottomright.y = win_c.max_y - 2;
  middle_y = (int)((bottomright.y-topleft.y)/2)+1;
  middle_x = (int)((bottomright.x-topleft.x)/2)+1;

  // draw the two subwindows
  inp_c.min_x = outp_c.min_x = topleft.x;
  inp_c.max_x = outp_c.max_x = bottomright.x;
  inp_c.min_y = topleft.y;
  inp_c.max_y = middle_y-topleft.y;
  outp_c.min_y = middle_y+1;
  outp_c.max_y = bottomright.y-middle_y;
  inp = subwin(stdscr, inp_c.max_y, inp_c.max_x, inp_c.min_y, inp_c.min_x);
  outp = subwin(stdscr, outp_c.max_y, outp_c.max_x, outp_c.min_y,outp_c.min_x);
  
  // initalise the windows
  touchwin(inp);
  werase(inp);
  wrefresh(inp);
  scrollok(inp, TRUE);
  
  touchwin(outp);
  werase(outp);
  wrefresh(outp);
  scrollok(outp, TRUE);
  refresh();
  
  nodelay(inp, TRUE);
  
  // flush the input buffers
  flushinp();
  
  move(topleft.x,topleft.y);
  DrawBox();
}

Viewer::~Viewer()
{
  delwin(inp);
  delwin(outp);
  erase();
  refresh();
  endwin();
}

void
Viewer::iflt_mode(enum filter_t iflt)
{
  char s[100];
  flt_in= iflt;
  sprintf(s, "Input filter: %s\n", flt_name(flt_in));
  waddstr(inp, s);
  wrefresh(inp);
  ihex_high= 1;
  ihex_ptr= 0;
}

void
Viewer::oflt_mode(enum filter_t oflt)
{
  char s[100];
  flt_out= oflt;
  sprintf(s, "Otput filter: %s\n", flt_name(flt_out));
  waddstr(outp, s);
  wrefresh(outp);
  wrefresh(inp);
  ohex_ptr= 0;
}

void
Viewer::set_length(int l)
{
  if (l > (bottomright.x-2-7) / 4)
    l= ((bottomright.x-2-7)/4) - 1;
  line_length= l;
}

void Viewer::DrawBox(void)
{
  int height, width;
  COORDINATES current;
  
  // save the current position
  getyx(stdscr, current.y, current.x);
  
  height = (bottomright.y - topleft.y)+1;
  width = (bottomright.x - topleft.y)+1;
  
  mvaddch(topleft.y-1, topleft.x-1, ACS_ULCORNER);
  mvaddch(topleft.y-1, bottomright.x+1, ACS_URCORNER);
  mvaddch(bottomright.y+1, bottomright.x+1, ACS_LRCORNER);
  mvaddch(bottomright.y+1, topleft.x-1, ACS_LLCORNER);
  
  /* wmove (screen, y, x) */
  /* top */
  move(topleft.y-1, topleft.x);
  hline(ACS_HLINE, width);
  /* bottom */
  move(bottomright.y+1, topleft.x);
  hline(ACS_HLINE, width);
  move(bottomright.y+1, topleft.x);
  hline(ACS_HLINE, width);
  
  /* left */
  move(topleft.y, topleft.x-1);
  vline(ACS_VLINE, height);
  
  /* right */
  move(topleft.y, bottomright.x+1);
  vline(ACS_VLINE, height);
  
  /* the divider */
  mvaddch(middle_y, bottomright.x+1, ACS_RTEE);
  mvaddch(middle_y, topleft.x-1, ACS_LTEE);
  hline(ACS_HLINE, width);
  
  // the window titles
  mvaddstr(inp_c.min_y-1, middle_x-(strlen("Input")/2), "Input");
  mvaddstr(middle_y, middle_x-(strlen("Output")/2), "Output");
  move(current.y, current.x);
  refresh();
}

void Viewer::AddStrOutWin(char *string)
{
  waddstr(outp, string);
  wrefresh(outp);
  wrefresh(inp);
}

void Viewer::GetStrInWin(char *string)
{
  if(wgetstr(inp, string) == ERR) {
    string[0] = 0;
  } else {
    waddstr(inp, string);
    wrefresh(inp);
  }
}

void Viewer::AddChOutWin(char b)
{
  switch (flt_out)
    {
    case flt_none:
      waddch(outp, b);
      break;
    case flt_hex:
      {
	char s[10];
	unsigned int u= b&0xff;
	int i;
	ohex_buf[ohex_ptr++]= b;
	sprintf(s, "%02x ", u);
	waddstr(outp, s);
	if (ohex_ptr >= line_length)
	  {
	    for (i= 0; i < line_length; i++)
	      {
		u= ohex_buf[i];
		waddch(outp, isprint(u)?u:'.');
	      }
	    waddch(outp, '\n');
	    ohex_ptr= 0;
	    sprintf(s, "%06x ", ocnt);
	    waddstr(outp, s);
	  }
	break;
      }
    }
  ocnt++;
  wrefresh(outp);
  wrefresh(inp);
}

int Viewer::GetChInWin(char *res)
{
  int b = wgetch(inp);
  int ret= 1;
  char c= b;
  
  if (b==ERR)
    {
      return 0;
    }
  else
    {
      b= b & 0xff;
      if (b == 4)
	return -2;
      switch (flt_in)
	{
	case flt_none:
	  {
	    char c= b;
	    if (!isprint(b) &&
		(b != '\n') &&
		(b != '\r'))
	      c= ' ';
	    waddch(inp, c);
	    wrefresh(inp);
	    break;
	  }
	case flt_hex:
	  {
	    char s[10];
	    s[0]= c;
	    s[1]= 0;
	    if (!isxdigit(b))
	      return 0;
	    if (ihex_high)
	      {
		ihex_val= strtol(s, NULL, 16);
		ihex_high= 0;
		waddch(inp, c);
		wrefresh(inp);
		return 0;
	      }
	    else
	      {
		ihex_val*= 16;
		ihex_val+= strtol(s, NULL, 16);
		waddch(inp, c);
		waddch(inp, ' ');
		wrefresh(inp);
		ihex_high= 1;
		b= ihex_val;
		ret= 1;
	      }
	    break;
	  }
	}
    }
  
  if (res)
    *res= b;

  return ret;
}
