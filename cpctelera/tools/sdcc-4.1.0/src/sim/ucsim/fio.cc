/*
 * Simulator of microcontrollers (fio.cc)
 *
 * Copyright (C) 1997,16 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include "ddconfig.h"

#include <stdio.h>
//#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include HEADER_FD
//#include <errno.h>
#include <string.h>
#if defined HAVE_SYS_SOCKET_H
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>

// These two flags aren't defined on some platforms
#ifndef S_IRGRP
#define S_IRGRP 0
#endif
#ifndef S_IROTH
#define S_IROTH 0
#endif

#include "fiocl.h"

// prj
#include "utils.h"


cl_history::cl_history(char *aname):
  cl_ustrings(100, 10, aname)
{
  nr= 0;
  //actual_line= "";
}

cl_history::cl_history(const char *aname):
cl_ustrings(100, 10, aname)
{
  nr= 0;
  //actual_line= "";
}

cl_history::~cl_history(void)
{
}

const char *
cl_history::up(chars line)
{
  replace(line);
  if (nr > 0)
    nr--;
  return (char*)Items[nr];
}

const char *
cl_history::down(chars line)
{
  replace(line);
  if (nr < count)
    nr++;
  if (nr < count)
    return (char*)Items[nr];
  return NULL;
}

void
cl_history::enter(chars line)
{
  if (count > 1000)
    {
      free_at(0);
      /*if (nr > count)
	nr= count;*/
    }
  if (!line.empty())
    {
      add(strdup(line));
      nr= count;
    }
}

void
cl_history::replace(chars line)
{
  if (nr < count)
    {
      free(Items[nr]);
      if (line.empty())
	Items[nr]= strdup("");
      else
	Items[nr]= strdup(line);
    }
}


/****************************************************************************/

cl_f::cl_f(void)
{
  file_id= -1;
  own= false;
  tty= false;
  file_name= 0;
  file_mode= 0;
  server_port= -1;
  echo_of= NULL;
  echo_to= NULL;
  echo_color= "";
  at_end= 0;
  last_used= first_free= 0;
  cooking= 0;
  line[0]= 0;
  cursor= 0;
  esc_buffer[0]= 0;
  attributes_saved= 0;
  hist= new cl_history("history");
  proc_telnet= false;
  proc_escape= false;
}

cl_f::cl_f(chars fn, chars mode):
  cl_base()
{
  file_id= -1;
  file_name= fn;
  file_mode= mode;
  tty= false;
  own= false;
  server_port= -1;
  echo_of= NULL;
  echo_to= NULL;
  echo_color= "";
  at_end= 0;
  last_used= first_free= 0;
  cooking= 0;
  line[0]= 0;
  cursor= 0;
  esc_buffer[0]= 0;
  attributes_saved= 0;
  hist= new cl_history("history");
  proc_telnet= false;
}

cl_f::cl_f(int the_server_port)
{
  file_id= -1;
  own= false;
  tty= false;
  file_name= 0;
  file_mode= 0;
  server_port= the_server_port;
  echo_of= NULL;
  echo_to= NULL;
  echo_color= "";
  at_end= 0;
  last_used= first_free= 0;
  cooking= 0;
  line[0]= 0;
  cursor= 0;
  esc_buffer[0]= 0;
  attributes_saved= 0;
  hist= new cl_history("history");
  proc_telnet= false;
}

class cl_f *
cl_f::copy(chars mode)
{
  class cl_f *io= mk_io("", "");
  io->use_opened(file_id, mode);
  return io;
}

static int
open_flags(const char *m)
{
  if (strcmp(m, "r") == 0)
    return O_RDONLY;
  else if (strcmp(m, "r+") == 0)
    return O_RDWR | O_CREAT;
  else if (strcmp(m, "w") == 0)
    return O_WRONLY | O_TRUNC | O_CREAT;
  else if (strcmp(m, "w+") == 0)
    return O_RDWR | O_CREAT;
  else if (strcmp(m, "a") == 0)
    return O_APPEND | O_WRONLY | O_CREAT;
  else if (strcmp(m, "a+") == 0)
    return O_APPEND | O_RDWR | O_CREAT;
  return O_RDWR;
}

int
cl_f::init(void)
{
  if (server_port > 0)
    {
      file_id= mk_srv_socket(server_port);
      if (file_id <= 0)
	{
	  file_id= -1;
	  own= false;
	  return -1;
	}
      listen(file_id, 50);
      own= true;
      tty= false;
      changed();
    }
  else if (!file_name.empty())
    {
      if (file_mode.empty())
	file_mode= "r+";
      if ((file_id= ::open(file_name, open_flags(file_mode), (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))) >= 0)
	{
	  tty= isatty(file_id);
	  own= true;
	  save_attributes();
	  changed();
	}
      else
	{
	  file_id= -1;
	  own= false;
	}
    }
  return file_id;
}

int
cl_f::use_opened(int opened_file_id, const char *mode)
{
  close();
  if (mode)
    file_mode= mode;
  else
    file_mode= "r+";
  own= false;
  if (opened_file_id >= 0)
    {
      file_id= opened_file_id;
      tty= isatty(file_id);
      changed();
    }
  return file_id;
}

int
cl_f::own_opened(int opened_file_id, const char *mode)
{
  use_opened(opened_file_id, mode);
  own= true;
  return file_id;
}

int
cl_f::use_opened(FILE *f, const char *mode)
{
  close();
  if (f)
    {
      file_mode= mode;
      if ((file_id= fileno(f)) >= 0)
	{
	  tty= isatty(file_id);
	  own= false;
	  changed();
	}
      else
	file_id= -1;
    }
  return file_id;
}

int
cl_f::own_opened(FILE *f, const char *mode)
{
  use_opened(f, mode);
  own= true;
  return file_id;
}

int
cl_f::open(const char *fn)
{
  close();
  if (fn)
    file_name= fn;
  return init();
}

int
cl_f::open(const char *fn, const char *mode)
{
  close();
  if (mode)
    file_mode= mode;
  if (fn)
    file_name= fn;
  return init();
}

void
cl_f::changed(void)
{
}

int
cl_f::close(void)
{
  int i= 0;

  if (file_id >= 0)
    ::close(file_id);
  file_id= -1;
  own= false;
  file_name= 0;
  file_mode= 0;
  changed();
  return i;
}

int
cl_f::stop_use(void)
{
  //printf("cl_f stop_use fid=%d\n", file_id);
  file_id= -1;
  own= false;
  file_name= 0;
  file_mode= 0;
  attributes_saved= 0;
  changed();
  return 0;
}

cl_f::~cl_f(void)
{
  delete hist;
}

/* Buffer handling */

int
cl_f::put(int c)
{
  int n= (first_free + 1) % 1024;
  if (n == last_used)
    {
      printf("put: %d FULL!\n",c);
      return -1;
    }
  buffer[first_free]= c;
  first_free= n;
  return 0;
}

int
cl_f::get(void)
{
  if (last_used == first_free)
    {
      return -1;
    }
  int c= buffer[last_used] & 0xff;
  last_used= (last_used + 1) % 1024;
  return c;
}

int
cl_f::free_place(void)
{
  if (first_free >= last_used)
    return 1024 - (first_free - last_used) -1;
  return last_used - first_free -1;
}

int
cl_f::finish_esc(int k)
{
  esc_buffer[0]= 0;
  return k;
}

int
cl_f::process_telnet(unsigned char ci)
{
  int l= strlen(esc_buffer);
  esc_buffer[l]= ci;
  l++;
  esc_buffer[l]= 0;
  if ((ci == 0xff) &&
      (l == 2))
    {
      return finish_esc(0xff);
    }
  if (l == 3)
    {
      return finish_esc(0);
    }
  return 0;
}

int
cl_f::process_csi(void)
{
  int l= strlen(esc_buffer);
  if (l < 3)
    return 0;
  int /*f,*/ ret= 0;
  char c= esc_buffer[l-1];
  
  switch (esc_buffer[2])
    {
    case 'M':
      if (l == 6)
	{
	  switch (esc_buffer[3])
	    {
	    case ' ': ret= TU_BTN1; break;
	    case '!': ret= TU_BTN2; break;
	    case '"': ret= TU_BTN3; break;
	    case '0': ret= TU_CBTN1; break;
	    case '1': ret= TU_CBTN2; break;
	    case '2': ret= TU_CBTN3; break;
	    case '(': ret= TU_ABTN1; break;
	    case ')': ret= TU_ABTN2; break;
	    case '*': ret= TU_ABTN3; break;
	    case '`': ret= TU_SUP; break;
	    case 'a': ret= TU_SDOWN; break;
	    case 'p': ret= TU_CSUP; break;
	    case 'q': ret= TU_CSDOWN; break;
	    }
	  //f= ret;
	  ret&= ~0xffff00;
	  int x= (esc_buffer[4] - 0x20) & 0xff;
	  int y= (esc_buffer[5] - 0x20) & 0xff;
	  ret|= x << 16;
	  ret|= y << 8;
	  //fprintf(stderr, "Mouse: 0x%0x (f=%d,0x%x)\n", ret, f, f);
	  return finish_esc(ret);
	}
      return 0;
      break;
    }
  // first char not recognized, check the last
  switch (c)
    {
    case 'A': return finish_esc(TU_UP);
    case 'B': return finish_esc(TU_DOWN);
    case 'C': return finish_esc(TU_RIGHT);
    case 'D': return finish_esc(TU_LEFT);
    case 'H': return finish_esc(TU_HOME);
    case 'F': return finish_esc(TU_END);
    case 'E': return finish_esc(0); // NumPad 5
    case '~':
      {
	int n;
	n= strtol(&esc_buffer[2], 0, 0);
	switch (n)
	  {
	  case 1: return finish_esc(TU_HOME);
	  case 2: return finish_esc(TU_INS);
	  case 3: return finish_esc(TU_DEL);
	  case 4: return finish_esc(TU_END);
	  case 5: return finish_esc(TU_PGUP);
	  case 6: return finish_esc(TU_PGDOWN);
	  case 11: return finish_esc(TU_F1);
	  case 12: return finish_esc(TU_F2);
	  case 13: return finish_esc(TU_F3);
	  case 14: return finish_esc(TU_F4);
	  case 15: return finish_esc(TU_F5);
	  case 17: return finish_esc(TU_F6);
	  case 18: return finish_esc(TU_F7);
	  case 19: return finish_esc(TU_F8);
	  case 20: return finish_esc(TU_F9);
	  case 21: return finish_esc(TU_F10);
	  case 23: return finish_esc(TU_F11);
	  case 24: return finish_esc(TU_F12);
	  default: return finish_esc(c);
	  }
      }
    }
  return 0;
}

int
cl_f::process_esc(char c)
{
  int l;
  //char s[100];
  unsigned int ci= c&0xff, b0= esc_buffer[0]&0xff;
  
  if (b0 == '\033')
    {
      l= strlen(esc_buffer);
      esc_buffer[l]= c;
      l++;
      esc_buffer[l]= 0;
      switch (esc_buffer[1])
	{
	case 'O':
	  if (l < 3)
	    return 0;
	  switch (c)
	    {
	    case 'P': return finish_esc(TU_F1);
	    case 'Q': return finish_esc(TU_F2);
	    case 'R': return finish_esc(TU_F3);
	    case 'S': return finish_esc(TU_F4);
	    case 'H': return finish_esc(TU_HOME);
	    case 'F': return finish_esc(TU_END);
	    default: return finish_esc(c);
	    }
	  break;
	case 'N':
	  if (l < 3)
	    return 0;
	  switch (c)
	    {
	    default: return finish_esc(c);
	    }
	  break;
	case '[':
	  return process_csi();
	  break;
	default:
	  return finish_esc(c);
	}
    }
  else if (b0 == 0xff)
    {
      return process_telnet(ci);
    }
  else // start sequence
    {
      if (ci == '\033')
	{
	  esc_buffer[0]= '\033', esc_buffer[1]= 0;
	  return 0;
	}
      if (ci == 0xff)
	{
	  esc_buffer[0]= 0xff, esc_buffer[1]= 0;
	  return 0;
	}
    }
  return c;
}

int j= 0;

int
cl_f::process(char c)
{
  int i;
  unsigned int ci= c&0xff;

  if (!cooking)
    {
      if (proc_escape)
	{
	  if ((ci == '\033') ||
	      (esc_buffer[0] != 0))
	    {
	      i= process_esc(ci);
	      if (i != 0)
		return put(i);
	      else
		return 0;
	    }
	}
      if (proc_telnet)
	{
	  if ((ci == 0xff) ||
	      (esc_buffer[0] != 0))
	    {
	      ci= process_telnet(ci);
	      if (!ci)
		{
		  return last_ln= 0;
		}
	    }
	  
	  if ((ci == '\n') ||
	      (ci == '\r') ||
	      (ci == 0) ||
	      (last_ln != 0))
	    {
	      if ((last_ln == 0) &&
		  (ci != 0))
		{
		  last_ln= ci;
		}
	      else
		{
		  if (last_ln != (int)ci)
		    {
		      return last_ln= 0;
		    }
		  if (ci == 0)
		    {
		      return last_ln= 0;
		    }		      
		}
	    }
	  
	}
      if ((ci<31) &&
	  (ci!='\n') &&
	  (ci!='\r'))
	{
	  char s[3]= "^ ";
	  s[1]= 'A'+ci-1;
	  echo_write_str(s);
	}
      else if (ci >= 127)
	{
	  char s[100];
	  sprintf(s, "\\%02x", ci);
	  echo_write_str(s);
	}
      else
	{
	  echo_write(&c, 1);
	}
      last_ln= 0;
      return put(c);
    }

  int l= strlen(line);
  int k= process_esc(c);
  int ret= 0;
  if (!k)
    return last_ln= 0;
  // CURSOR MOVEMENT
  if (k == TU_LEFT)
    {
      if (cursor > 0)
	{
	  cursor--;
	  echo_cursor_go_left(1);
	}
    }
  else if (k == TU_RIGHT)
    {
      if (line[cursor] != 0)
	{
	  cursor++;
	  echo_cursor_go_right(1);
	}
    }
  else if ((k == TU_HOME) ||
	   (k == 'A'-'A'+1))
    {
      if (cursor > 0)
	{
	  echo_cursor_go_left(cursor);
	  cursor= 0;
	}
    }
  else if ((k == TU_END) ||
	   (k == 'E'-'A'+1))
    {
      if (line[cursor] != 0)
	{
	  echo_cursor_go_right(l-cursor);
	  cursor= l;
	}
    }
  // HISTORY
  else if (k == TU_UP)
    {
      const char *s= hist->up(line);
      if (cursor > 0)
	echo_cursor_go_left(cursor);
      echo_cursor_save();
      while (l--)
	echo_write_str(" ");
      echo_cursor_restore();
      line[cursor= 0]= 0;
      if (s != NULL)
	{
	  strcpy(line, s);
	  echo_write_str(s);
	  cursor= strlen(s);
	}
    }
  else if (k == TU_DOWN)
    {
      const char *s= hist->down(line);
      if (cursor > 0)
	echo_cursor_go_left(cursor);
      echo_cursor_save();
      while (l--)
	echo_write_str(" ");
      echo_cursor_restore();
      line[cursor= 0]= 0;
      if (s != NULL)
	{
	  strcpy(line, s);
	  echo_write_str(s);
	  cursor= strlen(s);
	}
    }
  // FINISH EDITING
  else if ((k == '\n') ||
	   (k == '\r') ||
	   (k == 0))
    {
      if (last_ln &&
	  (last_ln != k))
	{
	  last_ln= 0;
	  return 0;
	}
      last_ln= k;
      for (i= 0; i<l; i++)
	put(line[i]);
      put('\n');
      hist->enter(line);
      //tu_cooked();
      line[cursor= 0]= 0;
      esc_buffer[0]= 0;
      ret= l+1;
      echo_write_str("\n");
    }
  // DELETING
  else if ((k == 127) || /*DEL*/
	   (k == 8 /*BS*/))
    {
      if (cursor > 0)
	{
	  for (i= cursor; line[i]; i++)
	    line[i-1]= line[i];
	  l--;
	  line[l]= 0;
	  cursor--;
	  echo_cursor_go_left(1);
	  echo_cursor_save();
	  if (line[cursor])
	    echo_write_str(&line[cursor]);
	  echo_write_str(" ");
	  echo_cursor_restore();
	}
    }
  else if (//(k == 127) || /*DEL*/
	   (k == TU_DEL))
    {
      if (line[cursor] != 0)
	{
	  for (i= cursor+1; line[i]; i++)
	    line[i-1]= line[i];
	  l--;
	  line[l]= 0;
	  echo_cursor_save();
	  if (line[cursor])
	    echo_write_str(&line[cursor]);
	  echo_write_str(" ");
	  echo_cursor_restore();
	}
    }
  else if (k == 'K'-'A'+1)
    {
      if (cursor > 0)
	echo_cursor_go_left(cursor);
      echo_cursor_save();
      while (l--)
	//write(STDOUT_FILENO, " ", 1);
	echo_write_str(" ");
      echo_cursor_restore();
      line[cursor= 0]= 0;
    }
  else if (k < 0)
    ;
  else if (isprint(k))
    {
      if (l < /*tu_buf_size*/1023)
	{
	  if (line[cursor] == 0)
	    {
	      line[cursor++]= k;
	      line[cursor]= 0;
	      echo_write(&c, 1);
	    }
	  else
	    {
	      int j;
	      for (j= l; j>=cursor; j--)
		line[j+1]= line[j];
	      line[cursor++]= k;
	      echo_cursor_save();
	      echo_write(&line[cursor-1], l-cursor+2);
	      echo_cursor_restore();
	      echo_cursor_go_right(1);
	    }
	}
    }
  return ret;
}

int
cl_f::pick(void)
{
  char b[100];
  int fp= free_place();
  if (fp < 5)
    return 0;
  int i= ::read(file_id, b, (fp>101)?99:fp-1);
  if (i > 0)
    {
      int j;
      for (j= 0; j < i; j++)
	{
	  process(b[j]);
	}
    }
  if (i == 0)
    {
      at_end= 1;
    }
  if (i < 0)
    {}
  return i;
}

int
cl_f::pick(char c)
{
  int i= /*put*/process(c);
  return i;
}

int
cl_f::pick(const char *s)
{
  int i, ret= 0;

  if (s)
    for (i= 0; s[i]; i++)
      ret= pick(s[i]);
  return ret;
}

int
cl_f::input_avail(void)
{
  int ret= check_dev();
  if (ret)
    return ret;
  return at_end;
}

int
cl_f::read(int *buf, int max)
{
  return read_dev(buf, max);
}

int
cl_f::get_c(void)
{
  int c;
  while (!check_dev())
    ;
  int i= read_dev(&c, 1);
  if (i > 0)
    return c;
  else
    return i;
}

chars
cl_f::get_s(void)
{
  chars s= "";
  char c;

  if (eof())
    return s;
  c= get_c();
  while ((c == '\n') ||
	 (c == '\r'))
    {
      if (eof())
	return s;
      c= get_c();
    }
  if (eof())
    return s;
  s+= c;
  c= get_c();
  while (!eof() &&
	 (c != '\n') &&
	 (c != '\r'))
    {
      s+= c;
      c= get_c();
    }
  return s;
}


/* IO primitives */

int
cl_f::read_dev(int *buf, int max)
{
  int i= 0, c;
  
  if (max == 0)
    return -1;

  while (i < max)
    {
      c= get();
      if (c == -1)
	{
	  if (i>0)
	    // got something
	    return i;
	  if (at_end)
	    // no more data, and we are at the end
	    return 0;
	  // buffer is empty now, but no eof detected yet
	  return -1;
	}
      buf[i]= c;
      i++;
    }
  if (i>0)
    return i;
  if (at_end)
    return 0;
  return -2;
}


int
cl_f::write(const char *buf, int count)
{
  int i;
  if (file_id >= 0)
    {
      if (type != F_SOCKET)
	{
	  return ::write(file_id, buf, count);
	}
      // on socket, assume telnet
      for (i= 0; i < count; i++)
	{
	  int j;
	  if ((buf[i] == '\r') ||
	      (buf[i] == '\n'))
	    {
	      j= ::write(file_id, "\r\n", 2);
	      if (j != 2)
	        {}
	    }
	  else
	    {
	      j= ::write(file_id, &buf[i], 1);
	      if (j != 1)
	        {}
	    }
	}
      return i;
    }
  return -1;
}


int
cl_f::write_str(const char *s)
{
  if (!s ||
      !*s)
    return 0;
  return write(s, strlen(s));
}


int
cl_f::vprintf(const char *format, va_list ap)
{
  char *s= vformat_string(format, ap);
  int i= write_str(s);
  free(s);
  return i;
}

int
cl_f::prntf(const char *format, ...)
{
  va_list ap;
  int ret= 0;

  va_start(ap, format);
  ret= vprintf(format, ap);
  va_end(ap);

  return ret;
}

bool
cl_f::eof(void)
{
  if (file_id < 0)
    return true;
  return at_end && (last_used == first_free);
}


/*void
cl_f::flush(void)
{
}*/


/* Echoing */

void
cl_f::echo_cursor_save()
{
  if (echo_to)
    {
      echo_to->write("\033[s", 3);
      //echo_to->flush();
    }
}

void
cl_f::echo_cursor_restore()
{
  if (echo_to)
    {
      echo_to->write("\033[u", 3);
      //echo_to->flush();
    }
}

void
cl_f::echo_cursor_go_left(int n)
{
  char b[100];
  if (echo_to)
    {
      snprintf(b, 99, "\033[%dD", n);
      echo_to->write_str(b);
      //echo_to->flush();
    }
}

void
cl_f::echo_cursor_go_right(int n)
{
  char b[100];
  if (echo_to)
    {
      snprintf(b, 99, "\033[%dC", n);
      echo_to->write_str(b);
      //echo_to->flush();
    }
}

void
cl_f::echo_write(const char *b, int l)
{
  if (echo_to)
    {
      if (echo_color.nempty())
	echo_to->prntf("%s", echo_color.c_str());
      echo_to->write(b, l);
      //echo_to->flush();
    }
}

void
cl_f::echo_write_str(const char *s)
{
  if (echo_to)
    {
      if (echo_color.nempty())
	echo_to->prntf("%s", echo_color.c_str());
      echo_to->write_str(s);
      //echo_to->flush();
    }
}

void
cl_f::set_echo_color(chars col)
{
  echo_color= col;
}


/* Device handling */

void
cl_f::prepare_terminal()
{
}

void
cl_f::save_attributes()
{
}

void
cl_f::restore_attributes()
{
}

int
cl_f::raw(void)
{
  if (cooking)
    {
      int l= strlen(line), i;
      for (i= 0; i<l; i++)
	put(line[i]);
    }
  cooking= 0;
  return 0;
}


int
cl_f::cooked(void)
{
  line[cursor= 0]= 0;
  esc_buffer[0]= 0;
  last_ln= 0;
  if (tty)
    {
      cooking= 1;
    }
  else if (type == F_SOCKET)
    {
      cooking= 1;
    }
  else
    {
    }
  return 0;
}

int
cl_f::echo(class cl_f *out)
{
  if (echo_to)
    {
      echo_to->echo_of= NULL;
      echo_to= NULL;
    }
  if (out != NULL)
    {
      out->echo_of= this;
      echo_to= out;
    }
  return 0;
}


void
cl_f::interactive(class cl_f *echo_out)
{
  save_attributes();
  echo(echo_out);
  cooked();
  prepare_terminal();
}


void
cl_f::set_telnet(bool val)
{
  proc_telnet= val;
}

void
cl_f::set_escape(bool val)
{
  proc_escape= val;
}


const char *
fio_type_name(enum file_type t)
{
  switch (t)
    {
    case F_UNKNOWN: return "unknown";
    case F_FILE: return "file";
    case F_CHAR: return "char";
    case F_SOCKET: return "socket";
    case F_LISTENER: return "listener";
    case F_PIPE: return "pipe";
    case F_CONSOLE: return "console";
    case F_SERIAL: return "serial";
    }
  return "undef";
}


/* End of fio.cc */
