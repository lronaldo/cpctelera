/*
 * Simulator of microcontrollers (cmd.src/newcmdcl.h)
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

#ifndef CMD_NEWCMDCL_HEADER
#define CMD_NEWCMDCL_HEADER


#include "ddconfig.h"

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>

// prj
#include "pobjcl.h"
#include "optioncl.h"

// sim.src
//#include "appcl.h"

// local, cmd
#include "commandcl.h"


// Flags of consoles
enum con_flags {
  CONS_NONE        = 0,
  CONS_DEBUG       = 0x01,   // Print debug messages on this console
  CONS_FROZEN      = 0x02,   // Console is frozen (g command issued)
  CONS_INTERACTIVE = 0x08,   // Interactive console
  CONS_NOWELCOME   = 0x10,   // Do not print welcome message
  CONS_INACTIVE    = 0x20,   // Do not do any action
  CONS_ECHO        = 0x40,   // Echo commands
  CONS_REDIRECTED  = 0x80,   // Console is actually redirected
};

#define SY_ADDR         'a'
#define ADDRESS         "a"
#define SY_NUMBER       'n'
#define NUMBER          "n"
#define SY_DATA         'd'
#define DATA            "d"
#define SY_STRING       's'
#define STRING          "s"
#define SY_MEMORY       'm'
#define MEMORY          "m"
#define SY_HW           'h'
#define HW              "h"
#define SY_DATALIST     'D'
#define DATALIST        "D"
#define SY_BIT          'b'
#define BIT             "b"
#define SY_CELL		'c'
#define CELL		"c"


class cl_prompt_option: public cl_optref
{
protected:
  class cl_console_base *con;
public:
  cl_prompt_option(class cl_console_base *console);
  virtual int init(void);
  virtual void option_changed(void);
};

class cl_debug_option: public cl_prompt_option
{
public:
  cl_debug_option(class cl_console_base *console);
  virtual int init(void);
  virtual void option_changed(void);
};

/*
 * Command console
 */

class cl_console_base: public cl_base
{
 protected:
  class cl_prompt_option *prompt_option;
  class cl_optref *null_prompt_option;
  class cl_debug_option *debug_option;
  class cl_ustrings *lines_printed;
  class cl_cmd *last_command;
  //class cl_cmdline *last_cmdline;
  chars last_cmd;

  char nl;
  chars lbuf;

  int tu_bg_color, tu_fg_color;
 public:
    int prev_quit;

 public:
  cl_console_base(void);
  virtual ~cl_console_base(void);
  
  virtual class cl_console_base *clone_for_exec(char *fin) = 0;

  virtual void redirect(char *fname, char *mode) = 0;
  virtual void un_redirect(void) = 0;
  virtual bool is_tty(void) const = 0;
  virtual bool is_eof(void) const = 0;
  virtual bool input_avail(void) = 0;
  virtual int read_line(void) = 0;
  virtual class cl_f *get_fout(void)= 0;
  virtual class cl_f *get_fin(void)= 0;
  virtual void drop_files(void)= 0; // do not close, just ignore
  virtual void close_files(void)= 0;
  virtual void replace_files(bool close_old, cl_f *new_in, cl_f *new_out)= 0;
  
  virtual int init(void);
  virtual void welcome(void);
  virtual int proc_input(class cl_cmdset *cmdset);
  virtual bool need_check(void) { return false; }
  
  virtual void print_prompt(void);
  virtual int dd_printf(const char *format, ...);
  virtual int debug(const char *format, ...);
  virtual void print_bin(long data, int bits);
  virtual void print_char_octal(char c);
  virtual int cmd_do_print(const char *format, va_list ap);
  //virtual void flush(void);
  virtual void tu_cls(void);
  virtual void tu_clc(void);
  virtual void tu_cll(void);
  virtual void tu_go(int x1, int y1);
  virtual void tu_save(void);
  virtual void tu_restore(void);
  virtual void tu_hide(void);
  virtual void tu_show(void);
  virtual void tu_color(int bg, int fg);
  virtual void tu_mouse_on(void);
  virtual void tu_mouse_off(void);
  virtual void tu_reset(void);
  
  virtual bool interpret(char *cmd);
  virtual int get_id(void) const { return(id); }
  virtual void set_id(int new_id);
  virtual void set_prompt(char *p);
  
  virtual bool input_active(void) const;
  //virtual bool accept_last(void) { return /*is_tty() ? DD_TRUE : DD_FALSE;*/flags&CONS_INTERACTIVE; }
  virtual bool prevent_quit(void) { return (prev_quit>=0)?prev_quit:true; }
  
 private:
  int flags; // See CONS_XXXX
 public:
  virtual int set_flag(int flag, bool value);
  virtual void set_interactive(bool new_val);
  virtual bool get_flag(int flag);
  virtual int get_flags() { return flags; };
  virtual bool is_interactive() { return get_flag(CONS_INTERACTIVE); }
  virtual bool is_frozen() { return get_flag(CONS_FROZEN); }
  virtual bool set_cooked(bool new_val);
  
 protected:
  class cl_app *app;
  char *prompt;
  int id;
};

class cl_console_dummy: public cl_console_base
{
 public:
 cl_console_dummy(void): cl_console_base() {}

  virtual class cl_console_base *clone_for_exec(char *fin) { return NULL; }

  virtual void redirect(char *fname, char *mode) {}
  virtual void un_redirect(void) {}
  virtual bool is_tty(void) const { return false; }
  virtual bool is_eof(void) const { return false; }
  virtual bool input_avail(void) { return false; }
  virtual int read_line(void) { return 0; }
  virtual class cl_f *get_fout(void) { return NULL; }
  virtual class cl_f *get_fin(void) { return NULL; }
  virtual void drop_files(void) {}
  virtual void close_files(void) {}
  virtual void replace_files(bool close_old, cl_f *new_in, cl_f *new_out) {}
};

/*
 * Command interpreter
 */

class cl_commander_base: public cl_base
{
 public:
  class cl_app *app;
  class cl_list *cons;
  class cl_console_base *actual_console, *frozen_console, *config_console;
  class cl_cmdset *cmdset;
 protected:
  class cl_list *active_inputs;
  class cl_list *check_list;

 public:
  cl_commander_base(class cl_app *the_app, class cl_cmdset *acmdset);
  virtual ~cl_commander_base(void);

  virtual void add_console(class cl_console_base *console);
  virtual void del_console(class cl_console_base *console);
  virtual void activate_console(class cl_console_base *console);
  virtual void deactivate_console(class cl_console_base *console);
  virtual int consoles_prevent_quit(void);
  
  //void prompt(void);
  int all_printf(const char *format, ...);        // print to all consoles
  int dd_printf(const char *format, va_list ap);  // print to actual_console
  int dd_printf(const char *format, ...);         // print to actual_console
  int debug(const char *format, ...);             // print consoles with debug flag set
  int debug(const char *format, va_list ap);      // print consoles with debug flag set
  int flag_printf(int iflags, const char *format, ...);
  int input_avail_on_frozen(void);
  class cl_console_base *exec_on(class cl_console_base *cons, char *file_name);
  
  virtual int init(void) = 0;
  virtual void update_active(void) = 0;
  virtual int proc_input(void) = 0;
  virtual int input_avail(void) = 0;
  virtual int wait_input(void) = 0;
  virtual void check(void) { return; }
};


#endif

/* End of cmd.src/newcmdcl.h */
