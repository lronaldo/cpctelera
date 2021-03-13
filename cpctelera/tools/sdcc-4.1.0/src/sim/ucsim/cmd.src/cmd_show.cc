/*
 * Simulator of microcontrollers (cmd.src/show.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

//#include "ddconfig.h"

#include <stdlib.h>
#include <string.h>
//#include "i_string.h"

// prj
#include "globals.h"
#include "utils.h"
//#include "errorcl.h"

// sim
//#include "simcl.h"

// local
#include "cmd_showcl.h"


void
set_show_help(class cl_cmd *cmd)
{
  cmd->set_help("show subcommand",
		"Generic command for showing things about",
		"Long of show");
}

/*
 * Command: show copying
 *----------------------------------------------------------------------------
 */

//int
//cl_show_copying_cmd::do_work(class cl_sim *sim,
//			     class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK(cl_show_copying_cmd)
{
  con->dd_printf("%s\n", copying);
  return(false);;
}

CMDHELP(cl_show_copying_cmd,
	"show copying",
	"Conditions for redistributing copies of uCsim",
	"long help of show copying")

/*
 * Command: show warranty
 *----------------------------------------------------------------------------
 */

//int
//cl_show_warranty_cmd::do_work(class cl_sim *sim,
//			      class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK(cl_show_warranty_cmd)
{
  con->dd_printf("%s\n", warranty);
  return(false);;
}

CMDHELP(cl_show_warranty_cmd,
	"show warranty",
	"Various kinds of warranty you do not have",
	"long help of show warranty")

/*
 * Command: show option
 *----------------------------------------------------------------------------
 */
COMMAND_DO_WORK_APP(cl_show_option_cmd)
{
  class cl_cmd_arg *parm= cmdline->param(0);
  char *s= 0;

  if (!parm)
    ;
  else if (cmdline->syntax_match(0/*app->get_uc()*/, STRING)) {
    s= parm->value.string.string;
  }
  else
    syntax_error(con);

  int i;
  for (i= 0; i < app->options->count; i++)
    {
      class cl_option *o= (class cl_option *)(/*uc*/app->options->at(i));
      if (!s ||
	  !strcmp(s, o->get_name()))
	{
	  int j;
	  con->dd_printf("%d. %s (%p): ", i, object_name(o), o);
	  o->print(con);
	  con->dd_printf(" - %s\n", o->help);
	  con->dd_printf("  Type: %s\n", o->get_type_name());
	  /*union option_value *val= o->get_value();
	  con->dd_printf("  Value: \"");
	  unsigned int uj;
	  TYPE_UBYTE *d= (TYPE_UBYTE*)val;
	  for (uj= 0; uj < sizeof(*val); uj++)
	  con->print_char_octal(d[uj]);
	  con->dd_printf("\"\n");*/
	  con->dd_printf("  Hidden: %s\n", (o->hidden)?"True":"False");
	  con->dd_printf("  Creator: \"%s\"\n  %d Users:\n",
			 object_name(o->get_creator()),
			 o->users->count);
	  for (j= 0; j < o->users->count; j++)
	    {
	      class cl_optref *r= (class cl_optref *)(o->users->at(j));
	      con->dd_printf("    %2d. owner(s)=\"%s\"\n", j,
			     object_name(r->get_owner()));
	    }
	  if (i >= 0 &&
	      i < app->options->count-1)
	    con->dd_printf("\n");
	}
    }
  
  return(false);
}

CMDHELP(cl_show_option_cmd,
	"show option [name]",
	"Show internal data of options",
	"long help of show option")

// prj
#include "errorcl.h"

static void
show_error_cmd_print_node(class cl_console_base *con,
			  int indent, class cl_base *node)
{
  if (!node)
    return;
  int i;
  for (i= 0; i < indent; i++)
    con->dd_printf(" ");
  const char *name= node->get_name("unknown");
  class cl_error_class *ec= dynamic_cast<class cl_error_class *>(node);
  char *str;
  con->dd_printf("%s: %s [%s/%s]\n",
		 str= case_string(case_case, ec->get_type_name()),
		 name, get_id_string(error_on_off_names,
				     ec->get_on()),
		 (ec->is_on())?"ON":"OFF");
  free(str);
  class cl_base *c= node->first_child();
  while (c)
    {
      show_error_cmd_print_node(con, indent+2, c);
      c= node->next_child(c);
    }
}

/*
 * Command: show error
 *----------------------------------------------------------------------------
 */
COMMAND_DO_WORK_APP(cl_show_error_cmd)
{
  /*class cl_cmd_arg *parm= cmdline->param(0);
  char *s= 0;

  if (!parm)
    ;
  else if (cmdline->syntax_match(0, STRING)) {
    s= parm->value.string.string;
  }
  else
  syntax_error(con);
  */
  class cl_list *registered_errors = cl_error_registry::get_list();
  int i;
  for (i= 0; i < registered_errors->count; i++)
    {
      class cl_error_class *ec;
      ec= dynamic_cast<class cl_error_class*>(registered_errors->object_at(i));
      if (!ec->get_parent())
	show_error_cmd_print_node(con, 0, ec);
    }
  return(false);
}

CMDHELP(cl_show_error_cmd,
	"show error",
	"Show class of errors",
	"long help of show error")

//#include "newcmdposixcl.h"

/*
 * Command: show console
 *----------------------------------------------------------------------------
 */

static void
print_fio_info(class cl_console_base *con, class cl_f *ff)
{
  if (ff)
    {
      const char *n= ff->get_file_name();
      const char *t= fio_type_name(ff->type);
      con->dd_printf("\"%s\",%s,", n, t);
      con->dd_printf("%d,%d,", ff->file_id, ff->server_port);
      con->dd_printf("%s,%s",
		     ff->tty?"tty":"non-tty",
		     ff->get_cooking()?"cooked":"raw");
      class cl_f *e= ff->get_echo_to();
      if (e)
	con->dd_printf("(echo>%d)", e->file_id);
      if (ff->get_telnet())
	con->dd_printf(",telnet");
      if (ff->get_escape())
	con->dd_printf(",esc");
    }
  con->dd_printf("\n");
}

COMMAND_DO_WORK_APP(cl_show_console)
{
  class cl_commander_base *cm= app->get_commander();
  class cl_console *cn;
  int i;

  for (i= 0; i < cm->cons->count; i++)
    {
      cn= (class cl_console *)(cm->cons->at(i));
      con->dd_printf("%d %s %s(%d) ", cn->get_id(), cn->get_name(),
		     cn->prevent_quit()?"PrevQuit":"", cn->prev_quit);
      int f= cn->get_flags();
      con->dd_printf("%c", 'D'|((f&CONS_DEBUG)?0:0x20));
      con->dd_printf("%c", 'F'|((f&CONS_FROZEN)?0:0x20));
      con->dd_printf("%c", 'A'|((!(f&CONS_INACTIVE))?0:0x20));
      con->dd_printf("%c", 'W'|((!(f&CONS_NOWELCOME))?0:0x20));
      con->dd_printf("%c", 'I'|((f&CONS_INTERACTIVE)?0:0x20));
      con->dd_printf("%c", 'E'|((f&CONS_ECHO)?0:0x20));
      con->dd_printf("\n");
      class cl_f *ff= cn->get_fin();
      con->dd_printf(" <");
      print_fio_info(con, ff);
      ff= cn->get_fout();
      con->dd_printf(" >");
      print_fio_info(con, ff);
    }
  return false;
}

CMDHELP(cl_show_console,
	"",
	"",
	"");

/* End of cmd.src/cmd_show.cc */
