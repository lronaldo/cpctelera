/*
 * Simulator of microcontrollers (cmd.src/info.cc)
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

#include <stdlib.h>
#include "i_string.h"

// sim.src
#include "simcl.h"

// local
#include "cmd_infocl.h"


/*
 * INFO BREAKPOINTS command
 */

COMMAND_DO_WORK_UC(cl_info_bp_cmd)
{
  int i;

  con->dd_printf("Num Type       Disp Hit   Cnt   Address  Cond  What\n");
  for (i= 0; i < uc->fbrk->count; i++)
    {
      class cl_brk *fb= (class cl_brk *)(uc->fbrk->at(i));
      const char *s= uc->disass(fb->addr, NULL);
      con->dd_printf("%-3d %-10s %s %-5d %-5d 0x%06x %-5s %s\n", fb->nr,
                     "fetch", (fb->perm==brkFIX)?"keep":"del ",
                     fb->hit, fb->cnt, fb->addr,
		     fb->condition()?"true":"false",
		     s);
      if (!(fb->commands.empty()))
	con->dd_printf("     %s\n", (char*)(fb->commands));
      free((char *)s);
    }
  for (i= 0; i < uc->ebrk->count; i++)
    {
      class cl_ev_brk *eb= (class cl_ev_brk *)(uc->ebrk->at(i));
      con->dd_printf("%-3d %-10s %s %-5d %-5d 0x%06x %s\n", eb->nr,
		     "event", (eb->perm==brkFIX)?"keep":"del ",
		     eb->hit, eb->cnt,
		     eb->addr, eb->id);
      if (!(eb->commands.empty()))
	con->dd_printf("     %s\n", (char*)(eb->commands));
    }
  return(0);
}


/*
 * INFO REGISTERS command
 */

COMMAND_DO_WORK_UC(cl_info_reg_cmd)
{
  uc->print_regs(con);
  return(0);
}


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
    hw->print_info(con);
  }
  else
    con->dd_printf("%s\n", short_help?short_help:"Error: wrong syntax\n");

  return(false);
}


/*
 * INFO STACK command
 */

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


/*
 * INFO VARIABLES command
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_info_var_cmd)
{
  class cl_var *v;
  int i;
  class cl_cmd_arg *params[1]= { cmdline->param(0) };
  char *s= NULL;
  
  if (cmdline->syntax_match(uc, STRING))
    {
      s= params[0]->get_svalue();
      if (!s ||
	  !*s)
	s= NULL;
    }
  for (i= 0; i < uc->vars->count; i++)
    {
      v= (class cl_var *)(uc->vars->at(i));
      if ((s == NULL) ||
	  (
	   (strstr(v->as->get_name(), s) != NULL) ||
	   (strstr(v->get_name(), s) != NULL)
	   )
	  )
      v->print_info(con);
    }
  return 0;
}


/* End of cmd.src/cmd_info.cc */
