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

#include "fiocl.h"
#include "newcmdcl.h"
#include "cmdutil.h"


/*
 * Command fd console
 */

class cl_console: public cl_console_base
{
 protected:
  //FILE *in/*, *out, *rout*//*redirected output*/;
  cl_f *fin, *fout, *frout;
  
 public:
  cl_console(void) { fin= fout= frout= 0; }
  cl_console(const char *_fin, const char *_fout, class cl_app *the_app);
  //cl_console(FILE *_fin, FILE *_fout, class cl_app *the_app);
  cl_console(cl_f *_fin, cl_f *_fout, class cl_app *the_app);

  virtual ~cl_console(void);
  //virtual void set_id(int new_id);
  virtual class cl_console *clone_for_exec(char *_fin);
  virtual void drop_files(void); // do not close, just ignore
  virtual void close_files(void);
  virtual void replace_files(bool close_old, cl_f *new_in, cl_f *new_out);

  virtual void redirect(char *fname, char *mode);
  virtual void un_redirect(void);
  virtual UCSOCKET_T get_in_fd(void) { return(fin ? (fin->file_id) : -1); }
  virtual bool is_tty(void) const { return fin && (fin->tty); }
  virtual bool is_eof(void) const { return fin ? (fin->eof()) : true; }
  virtual bool input_avail(void);// { return input_active() ? (fin->input_avail()) : false; };
  virtual int read_line(void);
  virtual bool need_check(void);
  virtual bool set_cooked(bool new_val);
  
 public:
  //FILE *get_out(void) { return rout ? rout : out; }
  class cl_f *get_fout(void) { return frout ? frout : fout; }
  class cl_f *get_fin(void) { return fin; }
};

//#ifdef SOCKET_AVAIL
class cl_listen_console: public cl_console
{
 public:
  cl_listen_console(int serverport, class cl_app *the_app);

  //virtual void set_id(int new_id);
  virtual void welcome(void) {}

  //virtual UCSOCKET_T get_in_fd(void) { return(sock); }
  virtual int proc_input(class cl_cmdset *cmdset);
  virtual bool set_cooked(bool new_val) { return false; }
};
//#endif


class cl_sub_console: public cl_console
{
 private:
  class cl_console_base *parent;

 public:
  //cl_sub_console(class cl_console_base *the_parent, FILE *fin, FILE *fout, class cl_app *the_app);
  cl_sub_console(class cl_console_base *the_parent,
                 class cl_f *fin, class cl_f *fout, class cl_app *the_app);
  virtual ~cl_sub_console(void);
  virtual int init(void);
  //virtual void set_id(int new_id);
};


/*
 * Command interpreter
 */

class cl_commander: public cl_commander_base
{
 private:
  //fd_set read_set, active_set;
  //UCSOCKET_T fd_num;
  
 public:
  cl_commander(class cl_app *the_app, class cl_cmdset *acmdset)
    : cl_commander_base(the_app, acmdset)
  {
  }

  virtual int init(void);
  virtual void update_active(void);
  virtual int input_avail(void);
  virtual int wait_input(void);
  virtual int proc_input(void);
  virtual void check(void);
};

#endif

/* End of cmd.src/newcmdposixcl.h */
