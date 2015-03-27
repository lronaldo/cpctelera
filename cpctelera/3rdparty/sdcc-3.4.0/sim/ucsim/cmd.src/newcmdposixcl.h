/*
 * Simulator of microcontrollers (cmd.src/newcmdposixcl.h)
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

#ifndef CMD_NEWCMDFDCL_HEADER
#define CMD_NEWCMDFDCL_HEADER

#include "newcmdcl.h"
#include "cmdutil.h"


/*
 * Command fd console
 */

class cl_console: public cl_console_base
{
protected:
  FILE *in, *out, *rout/*redirected output*/;

public:
  cl_console(void) { in = out = rout = 0; }
  cl_console(const char *fin, const char *fout, class cl_app *the_app);
  cl_console(FILE *fin, FILE *fout, class cl_app *the_app);
  int cmd_do_print(const char *format, va_list ap);

  virtual ~cl_console(void);
  virtual class cl_console *clone_for_exec(char *fin);

  virtual void redirect(char *fname, char *mode);
  virtual void un_redirect(void);
  virtual UCSOCKET_T get_in_fd(void) { return(in ? fileno(in) : -1); }
  virtual bool is_tty(void) const { return in && isatty(fileno(in)); }
  virtual bool is_eof(void) const { return in ? feof(in) : true; }
  virtual bool input_avail(void) { return input_active() ? ::input_avail(fileno(in)) : false; };
  virtual char *read_line(void);

private:
  FILE *get_out(void) { return rout ? rout : out; }
};

#ifdef SOCKET_AVAIL
class cl_listen_console: public cl_console
{
private:
  int sock;

public:
  cl_listen_console(int serverport, class cl_app *the_app);

  virtual void welcome(void) {}

  virtual UCSOCKET_T get_in_fd(void) { return(sock); }
  virtual int proc_input(class cl_cmdset *cmdset);
};
#endif


class cl_sub_console: public cl_console
{
private:
  class cl_console_base *parent;

public:
  cl_sub_console(class cl_console_base *the_parent,
                 FILE *fin, FILE *fout, class cl_app *the_app);
  virtual ~cl_sub_console(void);
  virtual int init(void);
};


/*
 * Command interpreter
 */

class cl_commander: public cl_commander_base
{
private:
  fd_set read_set, active_set;
  UCSOCKET_T fd_num;

public:
  cl_commander(class cl_app *the_app, class cl_cmdset *acmdset)
    : cl_commander_base(the_app, acmdset)
  {
  }

  virtual int init(void);
  virtual void set_fd_set(void);
  virtual int input_avail(void);
  virtual int wait_input(void);
  virtual int proc_input(void);
};

#endif

/* End of cmd.src/newcmdposixcl.h */
