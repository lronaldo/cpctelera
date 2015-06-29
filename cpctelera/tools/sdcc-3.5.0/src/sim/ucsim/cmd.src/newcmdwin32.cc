/*
 * Simulator of microcontrollers (cmd.src/newcmdwin32.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * Copyright (C) 2006, Borut Razem - borut.razem@siol.net
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
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>
#include <fcntl.h>
#include <windows.h>

#include "i_string.h"

// prj
#include "globals.h"
#include "utils.h"

// sim
#include "simcl.h"
#include "argcl.h"
#include "appcl.h"

// local
#include "newcmdwin32cl.h"


/*
 * Channel
 *____________________________________________________________________________
 */

inline void
cl_channel::set(void)
{
  fp = 0;
  handle = INVALID_HANDLE_VALUE;
  type = CH_UNDEF;
}

inline void
cl_channel::set(HANDLE _handle, e_handle_type _type)
{
  assert(INVALID_HANDLE_VALUE != _handle);

  fp = 0;
  handle = _handle;
  type = (_type == CH_UNDEF) ? guess_type() : _type;
}

inline void
cl_channel::set(FILE *_fp, e_handle_type _type)
{
  assert(_fp);
  fp = _fp;
  handle = (HANDLE)_get_osfhandle(fileno(fp));
  assert(INVALID_HANDLE_VALUE != handle);
  type = (_type == CH_UNDEF) ? guess_type() : _type;
}

void
cl_channel::close(void)
{
  assert(INVALID_HANDLE_VALUE != handle);

  if (CH_SOCKET == type)
    {
      shutdown((SOCKET)handle, SD_BOTH);
      closesocket((SOCKET)handle);
    }
  if (fp)
    fclose(fp);
  else if (CH_SOCKET != type)
    CloseHandle(handle);

  fp = 0;
  handle = INVALID_HANDLE_VALUE;
  type = CH_UNDEF;
}

/*
 * Command console
 *____________________________________________________________________________
 */

cl_console::cl_console(const char *fin, const char *fout, class cl_app *the_app)
{
  FILE *f;

  app = the_app;
  if (fin)
    {
      if (!(f = fopen(fin, "r")))
        fprintf(stderr, "Can't open `%s': %s\n", fin, strerror(errno));
      in.set(f, CH_FILE);
    }

  if (fout)
    {
      if (!(f = fopen(fout, "w")))
        fprintf(stderr, "Can't open `%s': %s\n", fout, strerror(errno));
      out.set(f, CH_FILE);
    }

  prompt = 0;
  flags = CONS_NONE;
  if (in.is_tty())
    flags |= CONS_INTERACTIVE;
  else
    ;//fprintf(stderr, "Warning: non-interactive console\n");
  id = 0;
  lines_printed = new cl_ustrings(100, 100, "console_cache");
}

cl_console::cl_console(FILE *fin, FILE *fout, class cl_app *the_app)
{
  app = the_app;
  in.set(fin);
  out.set(fout);

  prompt = 0;
  flags = CONS_NONE;
  if (in.is_tty())
    flags |= CONS_INTERACTIVE;
  else
    ;//fprintf(stderr, "Warning: non-interactive console\n");
  id = 0;
  lines_printed = new cl_ustrings(100, 100, "console_cache");
}

cl_console::cl_console(cl_channel _in, cl_channel _out, class cl_app *the_app)
{
  app = the_app;
  in = _in;
  out = _out;

  prompt = 0;
  flags = CONS_NONE;
  if (in.is_tty())
    flags |= CONS_INTERACTIVE;
  else
    ;//fprintf(stderr, "Warning: non-interactive console\n");
  id = 0;
  lines_printed= new cl_ustrings(100, 100, "console_cache");
}

class cl_console *
cl_console::clone_for_exec(char *fin)
{
  FILE *fi;
  if (!fin)
    return 0;

  if (!(fi = fopen(fin, "r")))
    {
      fprintf(stderr, "Can't open `%s': %s\n", fin, strerror(errno));
      return 0;
    }
  cl_channel ch_in = cl_channel(fi, CH_FILE);
  class cl_console *con= new cl_sub_console(this, ch_in, out, app);
  return con;
}

cl_console::~cl_console(void)
{
  if (CH_UNDEF != in.get_type())
    in.close();
  un_redirect();
  if (CH_UNDEF != out.get_type())
    {
      if (flags & CONS_PROMPT)
        dd_printf("\n");
      out.close();
    }
  delete prompt_option;
  delete null_prompt_option;
  delete debug_option;
}


/*
 * Output functions
 */

void
cl_console::redirect(char *fname, char *mode)
{
  FILE *fp = fopen(fname, mode);
  if (!fp)
    dd_printf("Unable to open file '%s' for %s: %s\n",
      fname, (mode[0]=='w') ? "write" : "append", strerror(errno));
  out.set(fp, CH_FILE);
}

void
cl_console::un_redirect(void)
{
  if (CH_UNDEF != rout.get_type())
    out.close();
}

int
cl_console::cmd_do_print(const char *format, va_list ap)
{
  FILE *f = get_out()->get_fp();

  if (f)
   {
      int ret = vfprintf(f, format, ap);
      fflush(f);
      return ret;
    }
  else
    return 0;
}

/*
 * Input functions
 */

char *
cl_console::read_line(void)
{
#define BUF_LEN 1024

  TRACE("%d-%s\n", get_id(), __PRETTY_FUNCTION__);

  char *s = NULL;
  FILE *fp = in.get_fp();
  assert(fp);

#ifdef HAVE_GETLINE
  if (getline(&s, 0, fp) < 0)
    return(0);
#elif defined HAVE_GETDELIM
  size_t n = BUF_LEN;
  s = (char *)malloc(n);
  if (getdelim(&s, &n, '\n', fp) < 0)
    {
      free(s);
      return(0);
    }
#elif defined HAVE_FGETS
  s = (char *)malloc(BUF_LEN);
  if (fgets(s, BUF_LEN, fp) == NULL)
    {
      free(s);
      return(0);
    }
#endif
  s[strlen(s)-1]= '\0';
  if (s[strlen(s)-1] == '\r')
    s[strlen(s)-1]= '\0';
  flags&= ~CONS_PROMPT;
  return(s);
}


/*
 * This console cl_listen_console on a socket and can accept connection requests
 */

cl_listen_console::cl_listen_console(int serverport, class cl_app *the_app)
{
  SOCKET sock;
  app = the_app;

  if (INVALID_SOCKET != (sock = make_server_socket(serverport)))
    {
      if (SOCKET_ERROR == listen(sock, 10))
        fprintf(stderr, "Can't listen on port %d: %d\n", serverport, WSAGetLastError());
    }
  in.set((HANDLE)sock, CH_SOCKET);
}

int
cl_listen_console::proc_input(class cl_cmdset *cmdset)
{
  class cl_commander_base *cmd = app->get_commander();

  struct sockaddr_in sock_addr;
  ACCEPT_SOCKLEN_T size = sizeof(struct sockaddr);
  SOCKET newsock = accept((SOCKET)get_in_fd(), (struct sockaddr*)&sock_addr, &size);

  if (INVALID_SOCKET == newsock)
    {
      fprintf(stderr, "Can't accept: %d\n", WSAGetLastError());
      return(0);
    }

  int fh = _open_osfhandle((intptr_t)newsock, _O_TEXT);
  if (-1 == fh)
    {
      fprintf(stderr, "Can't _open_osfhandle\n");
    }
  FILE *fp = fdopen(fh, "r");
  if (!fp)
    fprintf(stderr, "Can't open port for input\n");
  cl_channel ch_in = cl_channel(fp, CH_SOCKET);

  fh = _open_osfhandle((intptr_t)newsock, _O_TEXT);
  if (-1 == fh)
    {
      fprintf(stderr, "Can't _open_osfhandle\n");
    }
  fp = fdopen(fh, "w");
  if (!fp)
    fprintf(stderr, "Can't open port for output\n");
  cl_channel ch_out = cl_channel(fp, CH_SOCKET);

  class cl_console_base *c = new cl_console(ch_in, ch_out, app);
  c->flags |= CONS_INTERACTIVE;
  cmd->add_console(c);

  return 0;
}


/*
 * Sub-console
 */

cl_sub_console::cl_sub_console(class cl_console_base *the_parent,
  cl_channel _in, cl_channel _out, class cl_app *the_app):
  cl_console(_in, _out, the_app)
{
  parent = the_parent;
}

cl_sub_console::~cl_sub_console(void)
{
  class cl_commander_base *c = app->get_commander();

  if (parent && c)
    {
      c->activate_console(parent);
    }
}

int
cl_sub_console::init(void)
{
  class cl_commander_base *c = app->get_commander();

  if (parent && c)
    {
      c->deactivate_console(parent);
    }
  cl_console::init();
  flags |= CONS_ECHO;
  return 0;
}


/*
 * Command interpreter
 *____________________________________________________________________________
 */

int
cl_commander::init(void)
{
  TRACE("%s\n", __PRETTY_FUNCTION__);

  class cl_optref console_on_option(this);
  class cl_optref config_file_option(this);
  class cl_optref port_number_option(this);
  class cl_console_base *con;

  console_on_option.init();
  console_on_option.use("console_on");
  config_file_option.init();
  config_file_option.use("config_file");
  port_number_option.init();

  cl_base::init();
  set_name("Commander");

  bool need_config = DD_TRUE;

  if (port_number_option.use("port_number"))
    add_console(new cl_listen_console(port_number_option.get_value((long)0), app));

  /* The following code is commented out because it produces gcc warnings
   * newcmd.cc: In member function `virtual int cl_commander::init()':
   * newcmd.cc:785: warning: 'Config' might be used uninitialized in this function
   * newcmd.cc:786: warning: 'cn' might be used uninitialized in this function
   */
  /*
  char *Config= config_file_option.get_value(Config);
  char *cn= console_on_option.get_value(cn);
   */
  /* Here shoud probably be something else, but is still better then the former code... */
  char *Config = config_file_option.get_value("");
  char *cn = console_on_option.get_value("");

  if (cn)
    {
      add_console(con = new cl_console(cn, cn, app));
      exec_on(con, Config);
      need_config = DD_FALSE;
    }
  if (cons->get_count() == 0)
    {
      add_console(con = new cl_console(stdin, stdout, app));
      exec_on(con, Config);
      need_config = DD_FALSE;
    }
  if (need_config && Config && *Config)
    {
      FILE *fc = fopen(Config, "r");
      if (!fc)
        fprintf(stderr, "Can't open `%s': %s\n", Config, strerror(errno));
      else
        {
          con = new cl_console(fc, stderr, app);
          con->flags |= CONS_NOWELCOME | CONS_ECHO;
          add_console(con);
        }
    }
  return(0);
}

void
cl_commander::set_fd_set(void)
{
  TRACE("%s\n", __PRETTY_FUNCTION__);

  int i;

  FD_ZERO(&read_set);

  for (i = 0; i < cons->count; i++)
    {
      class cl_console *c= dynamic_cast<class cl_console*>((class cl_console_base*)(cons->at(i)));

      if (c->input_active() && CH_SOCKET == c->in.get_type())
        {
          HANDLE fd = c->get_in_fd();
          assert(INVALID_HANDLE_VALUE != fd);

          FD_SET((SOCKET)fd, &read_set);
        }
    }
}

int
cl_commander::console_count(void)
{
  int i = 0;

  for (int j = 0; j < cons->count; j++)
    {
      class cl_console *c = dynamic_cast<class cl_console*>((class cl_console_base*)(cons->at(j)));

      if (c->input_active())
        {
          switch (c->in.get_type())
            {
            case CH_CONSOLE:
            case CH_FILE:
            case CH_SERIAL:
              ++i;
              break;

            default:
              break;
            }
        }
    }

  return i;
}

int
cl_commander::console_input_avail(void)
{
  int i = 0;

  FD_ZERO(&console_active_set);
  for (int j = 0; j < cons->count; j++)
    {
      class cl_console *c = dynamic_cast<class cl_console*>((class cl_console_base*)(cons->at(j)));

      if (c->input_avail())
        {
          HANDLE fd = c->get_in_fd();
          assert(INVALID_HANDLE_VALUE != fd);

          switch (c->in.get_type())
            {
            case CH_CONSOLE:
            case CH_FILE:
            case CH_SERIAL:
              FD_SET((SOCKET)fd, &console_active_set);
              ++i;
              break;

            default:
              break;
            }
        }
    }

  return i;
}

int
cl_commander::socket_input_avail(long timeout, bool sleep)
{
  active_set = read_set;

  if (active_set.fd_count)
    {
      struct timeval tv = {0, 0};

      struct timeval *tvp = sleep ? NULL : &tv;

      int i = select(0, &active_set, NULL, NULL, tvp);
      if (SOCKET_ERROR == i)
        {
          fprintf(stderr, "Can't select: %d\n", WSAGetLastError());
          return 0;
        }

      return i;
    }
  else
    {
      Sleep(timeout / 1000);
      return 0;
    }
}

int
cl_commander::input_avail_timeout(long timeout)
{
  TRACE("%s\n", __PRETTY_FUNCTION__);

  int n;
  if (0 != (n = console_input_avail()))
    FD_ZERO(&active_set);
  else
  	n = socket_input_avail(timeout, false);

  return n;
}

#define CONSOLE_TIMEOUT 300000

int
cl_commander::wait_input(void)
{
  TRACE("%s\n", __PRETTY_FUNCTION__);

  prompt();

  if (0 < console_count())
    {
      int n;

      while (0 == (n = input_avail_timeout(CONSOLE_TIMEOUT)))
        ;

      return n;
    }
  else
    {
      FD_ZERO(&console_active_set);
      return socket_input_avail(0, true);
    }
}

int
cl_commander::proc_input(void)
{
  TRACE("%s\n", __PRETTY_FUNCTION__);

  for (int j = 0; j < cons->count; j++)
    {
      class cl_console *c = dynamic_cast<class cl_console*>((class cl_console_base*)(cons->at(j)));

      if (c->input_active())
        {
          HANDLE fd = c->get_in_fd();
          assert(INVALID_HANDLE_VALUE != fd);

          if (FD_ISSET(fd, &active_set) || FD_ISSET(fd, &console_active_set))
            {
              actual_console = c;
              if (c->proc_input(cmdset))
                {
                  del_console(c);
                  delete c;
                }
              actual_console = 0;
              return 0 == cons->count;
            }
        }
    }
  return 0;
}


/* End of cmd.src/newcmdwin32.cc */
