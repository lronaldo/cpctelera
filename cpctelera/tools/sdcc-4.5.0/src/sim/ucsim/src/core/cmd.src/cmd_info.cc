/*
 * Simulator of microcontrollers (cmd.src/info.cc)
 *
 * Copyright (C) 1999 Drotos Daniel
 *
 * To contact author send email to dr.dkdb@gmail.com
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

#include <stdlib.h>
#include <string.h>

//#include "i_string.h"

// sim.src
#include "simcl.h"

// local
#include "cmd_infocl.h"

void
set_info_help(class cl_cmd *cmd)
{
  cmd->set_help("info subcommand",
		"Information about simulator objects",
		"Long of info");
}

/*
 * INFO BREAKPOINTS command
 */

COMMAND_DO_WORK_UC(cl_info_bp_cmd)
{
  int i;
  bool extra;
  chars sa;
  class cl_memory_cell *c;
  class cl_var *v;
  
  con->dd_printf("Num Type       Disp Hit   Cnt   Address  Cond\n");
  for (i= 0; i < uc->fbrk->count; i++)
    {
      class cl_brk *fb= (class cl_brk *)(uc->fbrk->at(i));
      class cl_memory *mem= fb->get_mem();
      if (mem)
	  sa.format("0x%06x", fb->addr);
      else
	{
	  c= fb->get_cell();
	  v= uc->vars->by_cell(c);
	  if (v)
	    sa= v->get_name();
	  else
	    sa= "cell";	  
	  sa.substr(0,8);
	}
      con->dd_printf("%-3d %-10s %s %-5d %-5d %8s %-5s ", fb->nr,
                     "fetch", (fb->perm==brkFIX)?"keep":"del ",
                     fb->hit, fb->cnt, sa.c_str(),
		     fb->condition()?"true":"false");
      //uc->print_disass(fb->addr, con);
      extra= false;
      if (!(fb->cond.empty()))
	con->dd_printf("     cond=\"%s\"", fb->cond.c_str()), extra= true;
      if (!(fb->commands.empty()))
	con->dd_printf("     cmd=\"%s\"", fb->commands.c_str()), extra= true;
      /*if (extra)*/ con->dd_printf("\n");
    }
  for (i= 0; i < uc->ebrk->count; i++)
    {
      class cl_ev_brk *eb= (class cl_ev_brk *)(uc->ebrk->at(i));
      class cl_memory *mem= eb->get_mem();
      if (mem)
	sa.format("0x%06x", eb->addr);
      else
	{
	  c= eb->get_cell();
	  v= uc->vars->by_cell(c);
	  if (v)
	    sa= v->get_name();
	  else
	    sa= "cell";
	  sa.substr(0,8);
	}
      con->dd_printf("%-3d %-10s %s %-5d %-5d %8s %s\n", eb->nr,
		     "event", (eb->perm==brkFIX)?"keep":"del ",
		     eb->hit, eb->cnt,
		     sa.c_str(), eb->id);
      extra= false;
      if (!(eb->cond.empty()))
	con->dd_printf("     cond=\"%s\"", eb->cond.c_str()), extra= true;
      if (!(eb->commands.empty()))
	con->dd_printf("     cmd=\"%s\"", eb->commands.c_str()), extra= true;
      if (extra) con->dd_printf("\n");
    }
  return(0);
}

CMDHELP(cl_info_bp_cmd,
	"info breakpoints",
	"Status of user-settable breakpoints",
	"")

/*
 * INFO REGISTERS command
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_info_reg_cmd)
{
  uc->print_regs(con);
  return(0);
}

CMDHELP(cl_info_reg_cmd,
	"info registers",
	"List of cpu registers and their contents",
	"")

/*
 * INFO HW command
 */

//int
//cl_info_hw_cmd::do_work(class cl_sim *sim,
//			class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_info_hw_cmd)
{
  class cl_hw *hw;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };

  if (cmdline->syntax_match(uc, HW)) {
    hw= params[0]->value.hw;
    con->dd_color("answer");
    hw->print_info(con);
    hw->print_cfg_info(con);
  }
  /*else if (cmdline->syntax_match(uc, STRING))
    {
      char *s= params[0]->get_svalue();
      if (s && *s && (strcmp("cpu", s)==0))
	{
	  if (uc->cpu)
	    uc->cpu->print_info(con);
	}
	}*/
  else
    syntax_error(con);

  return(false);
}

CMDHELP(cl_info_hw_cmd,
	"info hardware category",
	"Status of hardware elements of the CPU",
	"")

/*
 * INFO STACK command
 */
/*
COMMAND_DO_WORK_UC(cl_info_stack_cmd)
{
  int i;

  cl_stack_op::info_head(con);
  for (i= uc->stack_ops->count-1; i >= 0; i--)
    {
      class cl_stack_op *so= (class cl_stack_op *)(uc->stack_ops->at(i));
      so->info(con, uc);
    }
  return(false);
}
*/

/*CMDHELP(cl_info_stack_cmd,
	"info stack",
	"Status of stack of the CPU",
	"")*/

/*
 * INFO MEMORY command
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_info_memory_cmd)
{
  int i;
  class cl_memory *mem= NULL;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };

  if (cmdline->syntax_match(uc, MEMORY))
    {
      mem= params[0]->value.memory.memory;
      if (mem)
	{
	  mem->print_info("", con);
	}
      return false;
    }
  con->dd_printf("Memory chips:\n");
  for (i= 0; i < uc->memchips->count; i++)
    {
      class cl_memory_chip *m= (class cl_memory_chip *)(uc->memchips->at(i));
      m->print_info("  ", con);
    }
  con->dd_printf("Address spaces:\n");
  for (i= 0; i < uc->address_spaces->count; i++)
    {
      class cl_address_space *m=
	(class cl_address_space *)(uc->address_spaces->at(i));
      m->print_info("  ", con);
    }
  con->dd_printf("Address decoders:\n");
  for (i= 0; i < uc->address_spaces->count; i++)
    {
      class cl_address_space *m=
	(class cl_address_space *)(uc->address_spaces->at(i));
      int j;
      for (j= 0; j < m->decoders->count; j++)
	{
	  class cl_address_decoder *d=
	    (class cl_address_decoder *)(m->decoders->at(j));
	  d->print_info(chars("  "), con);
	}
    }
  return(0);
}

CMDHELP(cl_info_memory_cmd,
	"info memory",
	"Information about memory system",
	"")

/*
 * INFO VARIABLES command
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_info_var_cmd)
{
  class cl_cvar *v;
  int i;
  //class cl_cmd_arg *params[2]= { cmdline->param(0), cmdline->param(1) };
  chars src;
  chars filter_by;

  i= 0;
  class cl_cmd_arg *a= cmdline->param(i);
  while (a != NULL)
    {
      char *ps= a->get_svalue();
      if (ps && *ps)
	{
	  if (ps[0] == '/')
	    filter_by= &ps[1];
	  else
	    src= ps;
	}	
      i++;
      a= cmdline->param(i);
    }

  for (i= 0; i < uc->vars->by_name.count; i++)
    {
      v= uc->vars->by_name.at(i);
      if ((src.empty() && *(v->get_name()) != '.') ||
          (src.nempty() && strstr(v->get_name(), src.c_str()) != NULL))
	{
	  if (filter_by.nempty())
	    {
	      if (strchr(filter_by.c_str(), (char)(v->defined_by)) == NULL)
		continue;
	    }
	  v->print_info(con);
	}
    }
  return 0;
}

CMDHELP(cl_info_var_cmd,
	"info variables [[/filter] search]",
	"Information about variables",
	"This command prints info about all or selected variables.\n"
	"To select some variables, \"search\" string can be used,\n"
	"in this case those variables will be printed which name\n"
	"contains the specified search string.\n"
	"Filter option can be used to select variables according to\n"
	"place where the variable is defined.\n"
	" /p list predefined variables,\n"
	" /u list user defined variables (by var command),\n"
	" /d list variables read from debug info (cdb) file.\n" 
	)


/* End of cmd.src/cmd_info.cc */
