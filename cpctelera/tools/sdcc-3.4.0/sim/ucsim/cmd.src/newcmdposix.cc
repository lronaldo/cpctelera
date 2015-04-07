/*
 * Simulator of microcontrollers (cmd.src/newcmdposix.cc)
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
#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef SOCKET_AVAIL
# include HEADER_SOCKET
# if defined HAVE_SYS_SOCKET_H
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
# endif
#endif
#if FD_HEADER_OK
# include HEADER_FD
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "i_string.h"

// prj
#include "globals.h"
#include "utils.h"

// sim
#include "simcl.h"
#include "argcl.h"
#include "appcl.h"

// local
#include "newcmdposixcl.h"


/*
 * Command console
 *____________________________________________________________________________
 */

cl_console::cl_console(const char *fin, const char *fout, class cl_app *the_app)
{
  FILE *f;

  app= the_app;
  in= 0;
  if (fin)
    if (f= fopen(fin, "r"), in= f, !f)
      fprintf(stderr, "Can't open `%s': %s\n", fin, strerror(errno));
  out= 0;
  if (fout)
    if (f= fopen(fout, "w"), out= f, !f)
      fprintf(stderr, "Can't open `%s': %s\n", fout, strerror(errno));
  prompt= 0;
  flags= CONS_NONE;
  if (is_tty())
    flags|= CONS_INTERACTIVE;
  else
    ;//fprintf(stderr, "Warning: non-interactive console\n");
  rout= 0;
  id= 0;
  lines_printed= new cl_ustrings(100, 100, "console_cache");
}

cl_console::cl_console(FILE *fin, FILE *fout, class cl_app *the_app)
{
  app= the_app;
  in = fin;
  out= fout;
  prompt= 0;
  flags= CONS_NONE;
  if (is_tty())
    flags|= CONS_INTERACTIVE;
  else
    ;//fprintf(stderr, "Warning: non-interactive console\n");
  rout= 0;
  id= 0;
  lines_printed= new cl_ustrings(100, 100, "console_cache");
}

class cl_console *
cl_console::clone_for_exec(char *fin)
{
  FILE *fi= 0, *fo= 0;

  if (!fin)
    return(0);
  if (fi= fopen(fin, "r"), !fi)
    {
      fprintf(stderr, "Can't open `%s': %s\n", fin, strerror(errno));
      return(0);
    }
  if ((fo= fdopen(dup(fileno(out)), "a")) == 0)
    {
      fclose(fi);
      fprintf(stderr, "Can't re-open output file: %s\n", strerror(errno));
      return(0);
    }
  class cl_console *con= new cl_sub_console(this, fi, fo, app);
  return(con);
}

cl_console::~cl_console(void)
{
  if (in)
    fclose(in);
  un_redirect();
  if (out)
    {
      if (flags & CONS_PROMPT)
        fprintf(out, "\n");
      fflush(out);
      fclose(out);
    }
  delete prompt_option;
  delete null_prompt_option;
  delete debug_option;
#ifdef SOCKET_AVAIL
  /*  if (sock)
    {
      shutdown(sock, 2);
      close(sock);
      }*/
#endif
}


/*
 * Output functions
 */

void
cl_console::redirect(char *fname, char *mode)
{
  if ((rout= fopen(fname, mode)) == NULL)
    dd_printf("Unable to open file '%s' for %s: %s\n",
              fname, (mode[0]=='w')?"write":"append", strerror(errno));
}

void
cl_console::un_redirect(void)
{
  if (!rout)
    return;
  fclose(rout);
  rout = NULL;
}

int
cl_console::cmd_do_print(const char *format, va_list ap)
{
  int ret;
  FILE *f = get_out();

  if (f)
   {
      ret= vfprintf(f, format, ap);
      fflush(f);
    }
  else
    ret = 0;

  return ret;
}

/*
 * Input functions
 */

char *
cl_console::read_line(void)
{
  char *s= NULL;

#ifdef HAVE_GETLINE
  if (getline(&s, 0, in) < 0)
    return(0);
#elif defined HAVE_GETDELIM
  size_t n= 30;
  s= (char *)malloc(n);
  if (getdelim(&s, &n, '\n', in) < 0)
    {
      free(s);
      return(0);
    }
#elif defined HAVE_FGETS
  s= (char *)malloc(300);
  if (fgets(s, 300, in) == NULL)
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
 * This console listen on a socket and can accept connection requests
 */
#ifdef SOCKET_AVAIL

cl_listen_console::cl_listen_console(int serverport, class cl_app *the_app)
{
  app= the_app;
  if ((sock= make_server_socket(serverport)) >= 0)
    {
      if (listen(sock, 10) < 0)
        fprintf(stderr, "Listen on port %d: %s\n",
                serverport, strerror(errno));
    }
  in= out= rout= 0;
}

int
cl_listen_console::proc_input(class cl_cmdset *cmdset)
{
  int newsock;
  ACCEPT_SOCKLEN_T size;
  struct sockaddr_in sock_addr;
  class cl_commander_base *cmd;
  FILE *in, *out;

  cmd= app->get_commander();
  size= sizeof(struct sockaddr);
  newsock= accept(sock, (struct sockaddr*)&sock_addr, &size);
  if (newsock < 0)
    {
      perror("accept");
      return(0);
    }
  if (!(in= fdopen(newsock, "r")))
    fprintf(stderr, "cannot open port for input\n");
  if (!(out= fdopen(newsock, "w")))
    fprintf(stderr, "cannot open port for output\n");
  class cl_console_base *c= new cl_console(in, out, app);
  c->flags|= CONS_INTERACTIVE;
  cmd->add_console(c);
  return(0);
}

#endif /* SOCKET_AVAIL */


/*
 * Sub-console
 */

cl_sub_console::cl_sub_console(class cl_console_base *the_parent,
                               FILE *fin, FILE *fout, class cl_app *the_app):
  cl_console(fin, fout, the_app)
{
  parent= the_parent;
}

cl_sub_console::~cl_sub_console(void)
{
  class cl_commander_base *c= app->get_commander();

  if (parent && c)
    {
      c->activate_console(parent);
    }
}

int
cl_sub_console::init(void)
{
  class cl_commander_base *c= app->get_commander();

  if (parent && c)
    {
      c->deactivate_console(parent);
    }
  cl_console::init();
  flags|= CONS_ECHO;
  return(0);
}


/*
 * Command interpreter
 *____________________________________________________________________________
 */

int
cl_commander::init(void)
{
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

  bool need_config= DD_TRUE;

#ifdef SOCKET_AVAIL
  if (port_number_option.use("port_number"))
    add_console(new cl_listen_console(port_number_option.get_value((long)0), app));
#endif

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
  char *Config= config_file_option.get_value("");
  char *cn= console_on_option.get_value("");

  if (cn)
    {
      add_console(con= new cl_console(cn, cn, app));
      exec_on(con, Config);
      need_config= DD_FALSE;
    }
  if (cons->get_count() == 0)
    {
      add_console(con= new cl_console(stdin, stdout, app));
      exec_on(con, Config);
      need_config= DD_FALSE;
    }
  if (need_config &&
      Config &&
      *Config)
    {
      FILE *fc= fopen(Config, "r");
      if (!fc)
        fprintf(stderr, "Can't open `%s': %s\n", Config, strerror(errno));
      else
        {
          con= new cl_console(fc, stderr, app);
          con->flags|= CONS_NOWELCOME|CONS_ECHO;
          add_console(con);
        }
    }
  return(0);
}

void
cl_commander::set_fd_set(void)
{
  int i;

  FD_ZERO(&read_set);
  fd_num = 0;
  for (i = 0; i < cons->count; i++)
    {
      class cl_console *c= dynamic_cast<class cl_console*>((class cl_console_base*)(cons->at(i)));

      if (c->input_active())
        {
          UCSOCKET_T fd = c->get_in_fd();
          assert(0 <= fd);

          FD_SET(fd, &read_set);
          if (fd > fd_num)
            fd_num = fd;
        }
    }
  fd_num++;
}

int
cl_commander::input_avail(void)
{
  struct timeval tv = {0, 0};
  active_set = read_set;

  int i = select(fd_num, &active_set, NULL, NULL, &tv);
  if (i < 0)
    perror("select");

  return i;
}

int
cl_commander::wait_input(void)
{
  prompt();
  active_set = read_set;
  int i = select(fd_num, &active_set, NULL, NULL, NULL);
  return i;
}

int
cl_commander::proc_input(void)
{
  for (int j = 0; j < cons->count; j++)
    {
      class cl_console *c = dynamic_cast<class cl_console*>((class cl_console_base*)(cons->at(j)));

      if (c->input_active())
        {
          UCSOCKET_T fd = c->get_in_fd();
          assert(0 <= fd);

          if (FD_ISSET(fd, &active_set))
            {
              actual_console = c;
              int retval = c->proc_input(cmdset);
              if (retval)
                {
                  del_console(c);
                  delete c;
                }
              actual_console = 0;
              return(0 == cons->count);
            }
        }
    }
  return 0;
}


/* End of cmd.src/newcmdposix.cc */
