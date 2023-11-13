/*
 * Simulator of microcontrollers (cmd.src/commandcl.h)
 *
 * Copyright (C) 2002,02 Drotos Daniel, Talker Bt.
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

#ifndef CMD_COMMAND_HEADER
#define CMD_COMMAND_HEADER

#include "ddconfig.h"

// prj
#include "pobjcl.h"

// local, cmd
#include "newcmdcl.h"


enum cmd_operate_on {
  operate_on_none,
  operate_on_app,
  operate_on_sim,
  operate_on_uc
};


/*
 * Command line with parameters
 */

class cl_cmdline: public cl_base
{
public:
  class cl_app *app;
  char *cmd;
  char *rest;
  //char *name;
  class cl_list *params;
  class cl_ustrings *tokens;
  const char *matched_syntax;
  class cl_console_base *con;

public:
  cl_cmdline(class cl_app *the_app, const char *acmd, class cl_console_base *acon);
  virtual ~cl_cmdline(void);
  virtual int init(void);

private:
  virtual void split_out_string(char **_start, char **_end);
  virtual void split_out_output_redirection(char **_start, char **_end);
  virtual void split_out_bit(char *dot, char *param_str);
  virtual void split_out_array(char *dot, char *param_str);
  virtual int token_length(char *start);
  virtual chars get_token(char *start);
public:
  virtual int split(void);
  virtual int shift(void);
  virtual int repeat(void);
  virtual class cl_cmd_arg *param(int num);
  virtual void insert_param(int pos, class cl_cmd_arg *param);
  virtual bool syntax_match(class cl_uc *uc, const char *syntax);
  virtual bool set_data_list(class cl_cmd_arg *parm, int *iparm);
  virtual int nuof_params(void) { return(params->get_count()); }
  virtual bool restart_at_rest(void);
private:
  char *skip_delims(char *start);
};


/*
 * Command and container
 */

class cl_cmdset;

// simple command
class cl_cmd: public cl_base
{
public:
  enum cmd_operate_on operate_on;
  class cl_strings *names;
  int  can_repeat;
  chars usage_help;
  chars short_help;
  chars long_help;

public:
  cl_cmd(enum cmd_operate_on opon,
	 const char *aname,
	 int  can_rep);
  virtual ~cl_cmd(void);

  virtual int init(void) { set_help(); return 0; }
  virtual void set_help(void) {}
  virtual void set_help(const char *usage_hlp, const char *short_hlp, const char *long_hlp);
  virtual class cl_cmdset *get_subcommands(void) { return(0); }
  virtual void add_name(const char *nam);
  virtual int name_match(const char *aname, int strict);
  virtual int name_match(class cl_cmdline *cmdline, int strict);
  virtual int syntax_ok(class cl_cmdline *cmdline);
  virtual int work(class cl_app *app,
		   class cl_cmdline *cmdline, class cl_console_base *con);
  virtual int do_work(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual int do_work(class cl_app *app,
		      class cl_cmdline *cmdline, class cl_console_base *con);
  virtual int do_work(class cl_sim *sim,
		      class cl_cmdline *cmdline, class cl_console_base *con);
  virtual int do_work(class cl_uc *uc,
		      class cl_cmdline *cmdline, class cl_console_base *con);
  virtual void print_short(class cl_console_base *con);
  virtual void syntax_error(class cl_console_base *con);
};

#define COMMAND_HEAD(CLASS_NAME) \
class CLASS_NAME : public cl_cmd\
{
#define COMMAND_HEAD_ANCESTOR(CLASS_NAME,ANCESTOR) \
class CLASS_NAME : public ANCESTOR \
{

#define COMMAND_METHODS(CLASS_NAME) \
public:\
  CLASS_NAME (const char *aname,\
              int  can_rep):\
  cl_cmd(operate_on_none, aname, can_rep) {} \
  virtual int do_work(class cl_cmdline *cmdline, class cl_console_base *con);\
  virtual void set_help(void);

#define COMMAND_METHODS_ON(ON,CLASS_NAME) \
public:\
  CLASS_NAME (const char *aname,\
              int  can_rep):\
  cl_cmd(operate_on_ ## ON, aname, can_rep) {} \
  virtual int do_work(class cl_ ## ON * ON ,\
		      class cl_cmdline *cmdline, class cl_console_base *con);\
  virtual void set_help(void);

#define COMMAND_METHODS_ANCESTOR(CLASS_NAME,ANCESTOR) \
public:\
  CLASS_NAME (const char *aname, int  can_rep):\
  ANCESTOR (aname, can_rep) {}	\
  virtual int do_work(class cl_cmdline *cmdline, class cl_console_base *con);\
  virtual void set_help(void);

#define COMMAND_METHODS_ANCESTOR_ON(ON,CLASS_NAME,ANCESTOR) \
public:\
  CLASS_NAME (const char *aname, int  can_rep):\
  ANCESTOR (aname, can_rep) {}	\
  virtual int do_work(class cl_ ## ON * ON ,\
		      class cl_cmdline *cmdline, class cl_console_base *con);\
  virtual void set_help(void); \


#define COMMAND_TAIL }

#define COMMAND(CLASS_NAME) \
COMMAND_HEAD(CLASS_NAME) \
COMMAND_METHODS(CLASS_NAME) \
COMMAND_TAIL

#define COMMAND_ON(ON,CLASS_NAME) \
COMMAND_HEAD(CLASS_NAME) \
COMMAND_METHODS_ON(ON,CLASS_NAME) \
COMMAND_TAIL

#define COMMAND_DATA(CLASS_NAME,DATA) \
COMMAND_HEAD(CLASS_NAME) \
public: DATA ; \
COMMAND_METHODS(CLASS_NAME)\
COMMAND_TAIL

#define COMMAND_DATA_ON(ON,CLASS_NAME,DATA) \
COMMAND_HEAD(CLASS_NAME) \
public: DATA ; \
COMMAND_METHODS_ON(ON,CLASS_NAME)\
COMMAND_TAIL

#define COMMAND_ANCESTOR_ON(ON,CLASS_NAME,ANCESTOR) \
COMMAND_HEAD_ANCESTOR(CLASS_NAME,ANCESTOR) \
COMMAND_METHODS_ANCESTOR_ON(ON,CLASS_NAME,ANCESTOR) \
COMMAND_TAIL

#define COMMAND_DATA_ANCESTOR(CLASS_NAME,ANCESTOR,DATA) \
COMMAND_HEAD_ANCESTOR(CLASS_NAME,ANCESTOR) \
public: DATA ; \
COMMAND_METHODS_ANCESTOR(CLASS_NAME,ANCESTOR)\
COMMAND_TAIL

#define COMMAND_DATA_ANCESTOR_ON(ON,CLASS_NAME,ANCESTOR,DATA) \
COMMAND_HEAD_ANCESTOR(CLASS_NAME,ANCESTOR) \
public: DATA ; \
COMMAND_METHODS_ANCESTOR_ON(ON,CLASS_NAME,ANCESTOR)\
COMMAND_TAIL

#define COMMAND_DO_WORK(CLASS_NAME) \
int \
CLASS_NAME::do_work(class cl_cmdline *cmdline, class cl_console_base *con)
#define COMMAND_DO_WORK_APP(CLASS_NAME) \
int \
CLASS_NAME::do_work(class cl_app *app,\
		    class cl_cmdline *cmdline, class cl_console_base *con)
#define COMMAND_DO_WORK_SIM(CLASS_NAME) \
int \
CLASS_NAME::do_work(class cl_sim *sim,\
		    class cl_cmdline *cmdline, class cl_console_base *con)
#define COMMAND_DO_WORK_UC(CLASS_NAME) \
int \
CLASS_NAME::do_work(class cl_uc *uc,\
		    class cl_cmdline *cmdline, class cl_console_base *con)

#define CMDHELP(CLASS_NAME,USAGE_HLP,SHORT_HLP,LONG_HLP) \
  void \
  CLASS_NAME::set_help(void) \
  { \
    usage_help=USAGE_HLP;		\
    short_help=SHORT_HLP;		\
    long_help=LONG_HLP;			\
  }

// Command set is list of cl_cmd objects
class cl_cmdset: public cl_list
{
public:
  //class cl_sim *sim;
  //class cl_cmd *last_command;

public:
  cl_cmdset(void);
  //cl_cmdset(class cl_sim *asim);

  virtual class cl_cmd *get_cmd(class cl_cmdline *cmdline, bool accept_last);
  virtual class cl_cmd *get_cmd(const char *cmd_name);
  virtual void del(char *nam);
  virtual void replace(char *nam, class cl_cmd *cmd);
};

// subset of commands
class cl_super_cmd: public cl_cmd
{
public:
  class cl_cmdset *commands;

public:
  cl_super_cmd(const char *aname,
	       int  can_rep,
	       class cl_cmdset *acommands);
  virtual ~cl_super_cmd(void);

  virtual class cl_cmdset *get_subcommands(void) { return(commands); }
  virtual int work(class cl_app *app,
		   class cl_cmdline *cmdline, class cl_console_base *con);
};


#endif

/* End of cmd.src/commandcl.h */
