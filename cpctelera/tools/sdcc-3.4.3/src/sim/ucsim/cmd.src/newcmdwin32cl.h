/*
 * Simulator of microcontrollers (cmd.src/newcmdwin32cl.h)
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


class cl_channel
{
public:
  cl_channel(void) { set(); }
  cl_channel(HANDLE _handle, e_handle_type _type = CH_UNDEF) { set(_handle, _type); }
  cl_channel(FILE *_fp, e_handle_type _type = CH_UNDEF) { set(_fp, _type); }

  void set(void);
  void set(HANDLE _handle, e_handle_type _type = CH_UNDEF);
  void set(FILE *_fp, e_handle_type _type = CH_UNDEF);

  void close(void);
  bool input_avail(void) const { return ::input_avail(handle, type); }
  enum e_handle_type get_type(void) const { return type; }
  HANDLE get_handle(void) const { return handle; }
  FILE *get_fp(void) const { return fp; }
  bool is_tty(void) const { return CH_FILE != type; }
  bool is_eof(void) const { return get_fp() ? feof(get_fp()) : true; }

private:
  e_handle_type guess_type(void) { return get_handle_type(handle); }

  e_handle_type type;
  HANDLE handle;
  FILE *fp;
};


/*
 * Command socket console
 */

class cl_console: public cl_console_base
{
  friend class cl_commander;

protected:
  cl_channel in, out, rout/*redirected output*/;

public:
  cl_console(void) { in = cl_channel(); out = cl_channel(); rout = cl_channel(); }
  cl_console(const char *fin, const char *fout, class cl_app *the_app);
  cl_console(FILE *fin, FILE *fout, class cl_app *the_app);
  cl_console(cl_channel _in, cl_channel _out, class cl_app *the_app);

  int cmd_do_print(const char *format, va_list ap);

  virtual ~cl_console(void);
  virtual class cl_console *clone_for_exec(char *fin);

  virtual void redirect(char *fname, char *mode);
  virtual void un_redirect(void);
  virtual bool is_tty(void) const { return CH_FILE != in.get_type(); }
  virtual bool is_eof(void) const { return in.is_eof(); }
  virtual HANDLE get_in_fd(void) { return in.get_handle(); }
  virtual bool input_avail(void) { return input_active() ? in.input_avail() : false; }
  virtual char *read_line(void);

private:
  class cl_channel *get_out(void) { return (CH_UNDEF != rout.get_type()) ? &rout : &out; }
};

class cl_listen_console: public cl_console
{
public:
  cl_listen_console(int serverport, class cl_app *the_app);

  virtual void welcome(void) {}
  virtual int proc_input(class cl_cmdset *cmdset);
};

class cl_sub_console: public cl_console
{
private:
  class cl_console_base *parent;

public:
  cl_sub_console(class cl_console_base *the_parent,
                 cl_channel _in, cl_channel _out, class cl_app *the_app);
  virtual ~cl_sub_console(void);
  virtual int init(void);
};


/*
 * Command interpreter
 */

class cl_commander: public cl_commander_base
{
private:
  fd_set read_set, active_set, console_active_set;

public:
  cl_commander(class cl_app *the_app, class cl_cmdset *acmdset)
    : cl_commander_base(the_app, acmdset)
  {
  }

  virtual int init(void);
  virtual void set_fd_set(void);
  virtual int input_avail(void) { return input_avail_timeout(0); }
  virtual int wait_input(void);
  virtual int proc_input(void);

private:
  int console_count(void);
  int console_input_avail(void);
  int socket_input_avail(long timeout, bool sleep);
  int input_avail_timeout(long timeout);
};

#endif

/* End of cmd.src/newcmdwin32cl.h */
