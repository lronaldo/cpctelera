/*
 * Simulator of microcontrollers (cmd.src/cmdset.cc)
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

//#include <stdarg.h>
#include <string.h>

//#include "ddconfig.h"

// prj
//#include "i_string.h"
#include "utils.h"
#include "globals.h"

// sim.src
//#include "simcl.h"
//#include "uccl.h"

// local, cmd.src
#include "cmd_execcl.h"
//#include "cmdutil.h"


/*
 * Command: run
 *----------------------------------------------------------------------------
 */

//int
//cl_run_cmd::do_work(class cl_sim *sim,
//		    class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_SIM(cl_run_cmd)
{
  class cl_brk *b;
  t_addr start, end;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };

  if (params[0])
    if (!(params[0]->get_address(sim->uc, &start)))
      {
	con->dd_printf(cchars("Error: wrong start address\n"));
	return(false);
      }
  if (params[1])
    if (!(params[1]->get_address(sim->uc, &end)))
      {
	con->dd_printf(cchars("Error: wromg end address\n"));
	return(false);
      }
  if (params[0])
    {
      if (!sim->uc->inst_at(start))
	con->dd_printf(cchars("Warning: maybe not instruction at 0x%06x\n"),
		       AI(start));
      sim->uc->PC= start;
      if (params[1])
	{
	  if (start == end)
	    {
	      con->dd_printf(cchars("Addresses must be different.\n"));
	      return(false);
	    }
	  if ((b= sim->uc->fbrk_at(end)))
	    {
	    }
	  else
	    {
	      b= new cl_fetch_brk(sim->uc->rom,
				  sim->uc->make_new_brknr(), end,
				  brkDYNAMIC, 1);
	      sim->uc->fbrk->add_bp(b);
	    }
	}
    }
  con->dd_printf(cchars("Simulation started, PC=0x%06x\n"), AI(sim->uc->PC));
  /*
  if (sim->uc->fbrk_at(sim->uc->PC))
    sim->uc->do_inst(1);
  */
  sim->start(con, 0);
  return(false);
}

CMDHELP(cl_run_cmd,
	"run [start [stop]]",
	"Go",
	"long help of run")

/*
 * Command: stop
 *----------------------------------------------------------------------------
 */

//int
//cl_stop_cmd::do_work(class cl_sim *sim,
//		     class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_SIM(cl_stop_cmd)
{
  sim->stop(resUSER);
  sim->uc->print_disass(sim->uc->PC, con);
  return(false);
}

CMDHELP(cl_stop_cmd,
	"stop",
	"Stop",
	"long help of stop")

/*
 * Command: step
 *----------------------------------------------------------------------------
 */

//int
//cl_step_cmd::do_work(class cl_sim *sim,
//		     class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_SIM(cl_step_cmd)
{
  class cl_cmd_arg *params[2];
  params[0]= cmdline->param(0);
  params[1]= cmdline->param(1);
  int instrs= 1;
  if (params[0] != NULL)
    {
      instrs= params[0]->i_value;
    }
  class cl_uc *uc= sim->get_uc();
  if (uc && (params[1] != NULL))
    {
      chars s= params[1]->get_svalue();
      unsigned long do_clk;
      class cl_time_measurer *tm= NULL;
      do_clk= instrs;
      if (s == "clk")
	{
	  tm= new cl_time_clk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else if ((s == "s") || (s == "sec"))
	{
	  do_clk= uc->clocks_of_time(instrs);
	  tm= new cl_time_clk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else if ((s == "ms") || (s == "msec"))
	{
	  do_clk= uc->clocks_of_time(instrs/1000.0);
	  tm= new cl_time_clk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else if ((s == "us") || (s == "usec"))
	{
	  do_clk= uc->clocks_of_time(instrs/1000000.0);
	  tm= new cl_time_clk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else if ((s == "ns") || (s == "nsec"))
	{
	  do_clk= uc->clocks_of_time(instrs/1000000000.0);
	  tm= new cl_time_clk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else if (s == "vclk")
	{
	  tm= new cl_time_vclk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else if ((s == "fclk") || (s == "fetch"))
	{
	  tm= new cl_time_fclk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else if ((s == "rclk") || (s == "read"))
	{
	  tm= new cl_time_rclk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else if ((s == "wclk") || (s == "write"))
	{
	  tm= new cl_time_wclk(uc);
	  tm->init();
	  tm->from_now(do_clk);
	}
      else
	{
	  con->dd_printf("Unknown unit.\n");
	  return 0;
	}
      if (tm)
	{
	  uc->stop_when(tm);
	  sim->start(con, 0);
	}
      return 0;
    }
  if (instrs <= 0)
    instrs= 1;
  sim->start(con, instrs);
  return(0);
}

CMDHELP(cl_step_cmd,
	"step [number [unit]]",
	"Step",
	"long help of step")
/*
 * Command: next
 *----------------------------------------------------------------------------
 */

//int
//cl_next_cmd::do_work(class cl_sim *sim,
//		     class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_SIM(cl_next_cmd)
{
  class cl_brk *b;
  t_addr next;
  int inst_len;

  bool is_call;
  
#if 0
  struct dis_entry *de;
  t_mem code= sim->uc->rom->get(sim->uc->PC);
  int i= 0;
  de= &(sim->uc->dis_tbl()[i]);
  while ((code & de->mask) != de->code &&
	 de->mnemonic)
    {
      i++;
      de= &(sim->uc->dis_tbl()[i]);
    }
#endif

  inst_len = sim->uc->inst_length(sim->uc->PC);

  is_call= sim->uc->is_call(sim->uc->PC);

  if (is_call)
    {
      next= sim->uc->PC + inst_len;
      if (!sim->uc->fbrk_at(next))
	{
	  b= new cl_fetch_brk(sim->uc->rom,
			      sim->uc->make_new_brknr(),
			      next, brkDYNAMIC, 1);

	  b->init();
//	  sim->uc->fbrk->add_bp(b);

	  sim->uc->fbrk->add(b);
	  b->activate();
	}
      /*if (sim->uc->fbrk_at(sim->uc->PC))
	sim->uc->do_inst(1);*/
      sim->start(con, 0);
      //sim->uc->do_inst(-1);
    }
  else {
    //sim->uc->do_inst(1);
    sim->start(con, 1);
    //sim->step();
    //sim->stop(resSTEP);
    //sim->uc->print_regs(con);
  }
  return(false);
}

CMDHELP(cl_next_cmd,
	"next",
	"Next",
	"long help of next")

/*
 * Command: help
 *----------------------------------------------------------------------------
 */

//int
//cl_help_cmd::do_work(class cl_sim *sim,
//		     class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_APP(cl_help_cmd)
{
  class cl_commander_base *commander;
  class cl_cmdset *cmdset= 0;
  int i;
  class cl_cmd_arg *parm= cmdline->param(0);
  
  if ((commander= app->get_commander()) != 0)
    cmdset= commander->cmdset;
  if (!cmdset)
    return(false);
  if (!parm) {
    for (i= 0; i < cmdset->count; i++)
      {
	class cl_cmd *c= (class cl_cmd *)(cmdset->at(i));
	/*if (c->short_help.nempty())
	  con->dd_printf("%s\n", (char*)c->short_help);
	else
	con->dd_printf("%s\n", (char*)(c->names->at(0)));*/
	c->print_short(con);
      }
  }
  else
    {
      matches= 0;
      do_set(cmdline, 0, cmdset, con);
      if (matches == 1 &&
	  cmd_found)
	{
	  int names;
	  con->dd_printf("Names of command:");
	  for (names= 0; names < cmd_found->names->count; names++)
	    con->dd_printf(" %s", (char*)(cmd_found->names->at(names)));
	  con->dd_printf("\n");
	  class cl_cmdset *subset= cmd_found->get_subcommands();
	  if (subset)
	    {
	      con->dd_printf("\"%s\" must be followed by the name of a "
			     "subcommand\nList of subcommands:\n",
			     (char*)(cmd_found->names->at(0)));
	      for (i= 0; i < subset->count; i++)
		{
		  class cl_cmd *c=
		    dynamic_cast<class cl_cmd *>(subset->object_at(i));
		  //con->dd_printf("%s\n", (char*)c->short_help);
		  c->print_short(con);
		}
	    }
	  if (cmd_found->long_help.nempty())
	    con->dd_printf("%s\n", (char*)cmd_found->long_help);
	}
      if (!matches ||
	  !cmd_found)
	con->dd_printf("No such command.\n");
    }
  return(false);
}

CMDHELP(cl_help_cmd,
	"help [command [subcommand]]",
	"List of known commands, or description of specified command",
	"Long help of help command")

bool
cl_help_cmd::do_set(class cl_cmdline *cmdline, int pari,
		    class cl_cmdset *cmdset,
		    class cl_console_base *con)
{
  int i;
  for (i= 0; i < cmdset->count; i++)
    {
      class cl_cmd *cmd= dynamic_cast<class cl_cmd *>(cmdset->object_at(i));
      if (!cmd)
	continue;
      if (pari >= cmdline->nuof_params())
	return(false);
      class cl_cmd_arg *param= cmdline->param(pari);
      if (!param)
	return(false);
      class cl_cmdset *next_set= cmd->get_subcommands();
      if (cmd->name_match(param->s_value, false))
	{
	  if (pari+1 >= cmdline->nuof_params())
	    {
	      matches++;
	      cmd_found= cmd;
	      /*if (cmd->short_help.nempty())
		con->dd_printf("%s\n", (char*)cmd->short_help);
	      else
	      con->dd_printf("%s\n", (char*)(cmd->names->at(0)));*/
	      cmd->print_short(con);
	      //continue;
	    }
	  else
	    if (next_set)
	      do_set(cmdline, pari+1, next_set, con);
	}
    }
  return(true);
}


/*
 * Command: quit
 *----------------------------------------------------------------------------
 */

//int
//cl_quit_cmd::do_work(class cl_sim *sim,
//		     class cl_cmdline */*cmdline*/, class cl_console */*con*/)
COMMAND_DO_WORK(cl_quit_cmd)
{
  return(1);
}

CMDHELP(cl_quit_cmd,
	"quit",
	"Quit",
	"long help of quit")

/*
 * Command: kill
 *----------------------------------------------------------------------------
 */

//int
//cl_kill_cmd::do_work(class cl_sim *sim,
//		     class cl_cmdline */*cmdline*/, class cl_console */*con*/)
COMMAND_DO_WORK_APP(cl_kill_cmd)
{
  app->going= 0;
  if (app->sim)
    app->sim->state|= SIM_QUIT;
  return(1);
}

CMDHELP(cl_kill_cmd,
	"kill",
	"Shutdown simulator",
	"long help of kill")

/*
 * EXEC file
 */

COMMAND_DO_WORK_APP(cl_exec_cmd)
{
  class cl_cmd_arg *parm= cmdline->param(0);
  char *fn= 0;

  if (cmdline->syntax_match(0, STRING)) {
    fn= parm->value.string.string;
  }
  if (!fn || !*fn)
    {
      syntax_error(con);
      return (false);
    }

  class cl_commander_base *c= app->get_commander();
  c->exec_on(con, fn);

  return(false);
}

CMDHELP(cl_exec_cmd,
	"exec \"file\"",
	"Execute commands from file",
	"long help of exec")

/*
 * expression expression
 */

COMMAND_DO_WORK_APP(cl_expression_cmd)
{
  char *s= cmdline->cmd;
  char *fmt= NULL;
  int fmt_len= 0;
  if (!s ||
      !*s)
    return(false);
  int i= strspn(s, " \t\v\n\r");
  s+= i;
  i= strspn(s, "abcdefghijklmnopqrstuvwxyz");
  s+= i;
  i= strspn(s, " \t\v\n");
  s+= i;
  t_mem v= 0;
  if (s && *s)
    {
      if (*s == '/')
	{
	  i= strcspn(s, " \t\v\n\r");
	  fmt= s+1;
	  fmt_len= i;
	  s+= i;
	  i= strspn(s, " \t\v\n\r");
	  s+= i;
	}
      if (s && *s)
	v= application->eval(s);
      if (fmt)
	{
	  for (i= 0; i < fmt_len; i++)
	    {
	      switch (fmt[i])
		{
		case 'x': con->dd_printf("%x\n", MU(v)); break;
		case 'X': con->dd_printf("0x%x\n", MU(v)); break;
		case '0': con->dd_printf("0x%08x\n", MU32(v)); break;
		case 'd': con->dd_printf("%d\n", MI(v)); break;
		case 'o': con->dd_printf("%o\n", MU(v)); break;
		case 'u': con->dd_printf("%u\n", MU(v)); break;
		case 'b': con->dd_printf("%s\n", (char*)cbin(v,8*sizeof(v))); break;
		}
	    }
	}
      else
	con->dd_printf("%d\n", MI(v));
    }
  return(false);
}

CMDHELP(cl_expression_cmd,
	"expression [/format] expr",
	"Evaluate the expression",
	"long help of expression ")

/* End of cmd.src/cmd_exec.cc */
