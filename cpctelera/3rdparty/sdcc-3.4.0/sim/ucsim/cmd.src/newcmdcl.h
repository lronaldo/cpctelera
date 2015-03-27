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

// sim.src
#include "appcl.h"

// local, cmd
#include "commandcl.h"


// Flags of consoles
#define CONS_NONE        0
#define CONS_DEBUG       0x01   // Print debug messages on this console
#define CONS_FROZEN      0x02   // Console is frozen (g command issued)
#define CONS_PROMPT      0x04   // Prompt is out, waiting for input
#define CONS_INTERACTIVE 0x08   // Interactive console
#define CONS_NOWELCOME   0x10   // Do not print welcome message
#define CONS_INACTIVE    0x20   // Do not do any action
#define CONS_ECHO        0x40   // Echo commands

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
  class cl_cmdline *last_cmdline;

public:
  cl_console_base(void): cl_base() { app = 0; flags = 0; prompt = 0; }

  virtual class cl_console_base *clone_for_exec(char *fin) = 0;

  virtual void redirect(char *fname, char *mode) = 0;
  virtual void un_redirect(void) = 0;
  virtual int cmd_do_print(const char *format, va_list ap) = 0;
  virtual bool is_tty(void) const = 0;
  virtual bool is_eof(void) const = 0;
  virtual int input_avail(void) = 0;
  virtual char *read_line(void) = 0;

  virtual int init(void);
  virtual void welcome(void);
  virtual int proc_input(class cl_cmdset *cmdset);

  void print_prompt(void);
  int dd_printf(const char *format, ...);
  int debug(const char *format, ...);
  void print_bin(long data, int bits);
  void print_char_octal(char c);

  bool interpret(char *cmd);
  int get_id(void) const { return(id); }
  void set_id(int new_id);
  void set_prompt(char *p);
  
  bool input_active(void) const;
  bool accept_last(void) { return is_tty() ? DD_TRUE : DD_FALSE; }

public:
  int flags; // See CONS_XXXX

protected:
  class cl_app *app;
  char *prompt;
  int id;
};

/*
 * Command interpreter
 */

class cl_commander_base: public cl_base
{
public:
  class cl_app *app;
  class cl_list *cons;
  class cl_console_base *actual_console, *frozen_console;
  class cl_cmdset *cmdset;

public:
  cl_commander_base(class cl_app *the_app, class cl_cmdset *acmdset);
  virtual ~cl_commander_base(void);

  void add_console(class cl_console_base *console);
  void del_console(class cl_console_base *console);
  void activate_console(class cl_console_base *console);
  void deactivate_console(class cl_console_base *console);

  void prompt(void);
  int all_printf(const char *format, ...);        // print to all consoles
  int dd_printf(const char *format, va_list ap);  // print to actual_console
  int dd_printf(const char *format, ...);         // print to actual_console
  int debug(const char *format, ...);             // print consoles with debug flag set
  int debug(const char *format, va_list ap);      // print consoles with debug flag set
  int flag_printf(int iflags, const char *format, ...);
  int input_avail_on_frozen(void);
  void exec_on(class cl_console_base *cons, char *file_name);

  virtual int init(void) = 0;
  virtual void set_fd_set(void) = 0;
  virtual int proc_input(void) = 0;
  virtual int input_avail(void) = 0;
  virtual int wait_input(void) = 0;
};


#endif

/* End of cmd.src/newcmdcl.h */
