/*
 * Simulator of microcontrollers (cmd.src/timer.cc)
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

//#include <stdio.h>

//#include "ddconfig.h"

//#include "i_string.h"

// sim
#include "utils.h"
#include "simcl.h"

// local
#include "cmd_timercl.h"


void
set_timer_help(class cl_cmd *cmd)
{
  cmd->set_help("timer subcommand",
		"Manage timers",
		"Long of timer");
}

/*
 * Command: timer
 *----------------------------------------------------------------------------
 */

//int
//cl_timer_cmd::do_work(class cl_sim *sim,
//		      class cl_cmdline *cmdline, class cl_console *con)
COMMAND_DO_WORK_UC(cl_timer_cmd)
{
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };
  
  if (!params[0])
    {
      con->dd_printf("Timer id is missing.\n");
      return(false);
    }
  if (params[0]->as_number())
    {
      as_nr= true;
      id_nr= params[0]->value.number;
      id_str= 0;
      if (id_nr <= 0)
	{
	  con->dd_printf("Error: "
			 "Timer id must be greater than zero or a string\n");
	  return(true);
	}
      ticker= uc->get_counter(id_nr);
    }
  else
    {
      as_nr= false;
      id_str= params[0]->s_value;
      ticker= uc->get_counter(id_str);
    }
  cmdline->shift();
  return(false);
}

CMDHELP(cl_timer_cmd,
	"timer subcommand",
	"Manage timers",
	"")

/*
 * Command: timer add
 *-----------------------------------------------------------------------------
 * Add a new timer to the list
 */

COMMAND_DO_WORK_UC(cl_timer_add_cmd)
  //add(class cl_uc *uc, class cl_cmdline *cmdline, class cl_console *con)
{
  enum ticker_type type = TICK_ANY;
  long dir= +1;

  if (cl_timer_cmd::do_work(uc, cmdline, con))
    return(false);
  if (ticker)
    {
      if (!as_nr)
	con->dd_printf("Error: Timer \"%s\" already exists\n", id_str.c_str());
      else
	con->dd_printf("Error: Timer %d already exists\n", id_nr);
      return(false);
    }

  if (cmdline->nuof_params() > 0)
    {
      if (cmdline->syntax_match(uc, NUMBER))
        dir= cmdline->param(0)->value.number;
      else if (cmdline->syntax_match(uc, NUMBER NUMBER))
        {
          dir= cmdline->param(0)->value.number;
          if (cmdline->param(1)->value.number)
            type = TICK_INISR;
        }
      else if (cmdline->syntax_match(uc, NUMBER STRING))
        {
          dir= cmdline->param(0)->value.number;
          if (!strcmp(cmdline->param(1)->value.string.string, "isr"))
            type = TICK_INISR;
          else if (!strcmp(cmdline->param(1)->value.string.string, "idle"))
            type = TICK_IDLE;
          else if (!strcmp(cmdline->param(1)->value.string.string, "halt"))
            type = TICK_HALT;
        }
      else
        {
          con->dd_printf("Error: Wrong parameters\n");
          return(false);
        }
    }

  if (!as_nr)
    {
      ticker= new cl_ticker(dir, type, id_str);
      uc->add_counter(ticker, id_str);
    }
  else
    {
      ticker= new cl_ticker(dir, type, 0);
      uc->add_counter(ticker, id_nr);
    }

  return(false);
}

CMDHELP(cl_timer_add_cmd,
	"timer add id [direction [isr|idle|halt]]",
	"Create a clock counter (timer)",
	"log help of timer add")

/*
 * Command: timer delete
 *-----------------------------------------------------------------------------
 * Delete a timer from the list
 */

COMMAND_DO_WORK_UC(cl_timer_delete_cmd)
  //del(class cl_uc *uc, class cl_cmdline *cmdline, class cl_console *con)
{
  if (cl_timer_cmd::do_work(uc, cmdline, con))
    return(false);
  if (!ticker)
    {
      if (!as_nr)
	con->dd_printf("Timer \"%s\" does not exist\n", id_str.c_str());
      else
	con->dd_printf("Timer %d does not exist\n", id_nr);
      return(false);
    }
  if (!ticker->user)
    {
      con->dd_printf("Timer is not user and cannot be deleted\n");
      return(false);
    }
  if (!as_nr)
    uc->del_counter(id_str);
  else
    uc->del_counter(id_nr);

  return(false);
}

CMDHELP(cl_timer_delete_cmd,
	"timer delete id",
	"Delete a timer",
	"")

/*
 * Command: timer get
 *-----------------------------------------------------------------------------
 * Get the value of just one timer or all of them
 */

COMMAND_DO_WORK_UC(cl_timer_get_cmd)
  //get(class cl_uc *uc, class cl_cmdline *cmdline, class cl_console *con)
{
  if (cmdline->nuof_params())
    {
      if (cl_timer_cmd::do_work(uc, cmdline, con))
	return(false);
    }
  else
    ticker= 0;
  if (ticker)
    ticker->dump(id_nr, con);
  else
    {
      for (id_nr= 0; id_nr < uc->counters->count; id_nr++)
	{
	  ticker= uc->get_counter(id_nr);
	  if (ticker)
	    ticker->dump(id_nr, con);
	}
    }

  return(false);
}

CMDHELP(cl_timer_get_cmd,
	"timer get [id]",
	"Get value of a timer, or all",
	"")

/*
 * Command: timer run
 *-----------------------------------------------------------------------------
 * Allow a timer to run
 */

COMMAND_DO_WORK_UC(cl_timer_run_cmd)
  //run(class cl_uc *uc, class cl_cmdline *cmdline, class cl_console *con)
{
  if (cl_timer_cmd::do_work(uc, cmdline, con))
    return(false);
  if (!ticker)
    {
      if (!as_nr)
	con->dd_printf("Timer %s does not exist\n", id_str.c_str());
      else
	con->dd_printf("Timer %d does not exist\n", id_nr);
      return(0);
    }
  ticker->run = true;

  return(false);
}

CMDHELP(cl_timer_run_cmd,
	"timer start id",
	"Start a timer",
	"")

/*
 * Command: timer stop
 *-----------------------------------------------------------------------------
 * Stop a timer
 */

COMMAND_DO_WORK_UC(cl_timer_stop_cmd)
  //stop(class cl_uc *uc, class cl_cmdline *cmdline, class cl_console *con)
{
  if (cl_timer_cmd::do_work(uc, cmdline, con))
    return(false);

  if (!ticker)
    {
      if (!as_nr)
	con->dd_printf("Timer %s does not exist\n", id_str.c_str());
      else
	con->dd_printf("Timer %d does not exist\n", id_nr);
      return(false);
    }
  ticker->run = false;

  return(false);
}

CMDHELP(cl_timer_stop_cmd,
	"timer stop id",
	"Stop a timer",
	"")

/*
 * Command: timer value
 *-----------------------------------------------------------------------------
 * Set a timer to a specified value
 */

COMMAND_DO_WORK_UC(cl_timer_value_cmd)
  //val(class cl_uc *uc, class cl_cmdline *cmdline, class cl_console *con)
{
  /*class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2) };*/
  
  if (cl_timer_cmd::do_work(uc, cmdline, con))
    return(false);
  if (!ticker)
    {
      if (!as_nr)
	con->dd_printf("Error: Timer %s does not exist\n", id_str.c_str());
      else
	con->dd_printf("Error: Timer %d does not exist\n", id_nr);
      return(false);
    }
  if (cmdline->nuof_params() > 0)
    {
      if (cmdline->syntax_match(uc, NUMBER))
        {
          long ticks= cmdline->param(0)->value.number;
          ticker->set(ticks, ticks * uc->get_xtal_tick());
        }
      else if (cmdline->syntax_match(uc, NUMBER STRING))
        {
          const char *units;
          double time = (double)cmdline->param(0)->value.number * strtoscale(cmdline->param(1)->value.string.string, &units);
          if (units[0] != '\0' && units[0] != 's')
            con->dd_printf("Expected units to be in seconds not \"%s\"\n", cmdline->param(1)->value.string.string);
          else
            ticker->set((int)(time / uc->get_xtal_tick()), time);
        }
      else if (cmdline->syntax_match(uc, NUMBER NUMBER STRING))
        {
          const char *units;
          double time = (double)cmdline->param(1)->value.number * strtoscale(cmdline->param(2)->value.string.string, &units);
          if (units[0] != '\0' && units[0] != 's')
            con->dd_printf("Expected units to be in seconds not \"%s\"\n", cmdline->param(1)->value.string.string);
          else
            ticker->set(cmdline->param(0)->value.number, time);
        }
      else
        con->dd_printf("Error: Wrong parameters\n");
    }
  else
    ticker->dump(id_nr, con);

  return(false);
}

CMDHELP(cl_timer_value_cmd,
	"timer set id [ticks] [time [muµnp]sec]",
	"Set a timer",
	"")


/*
 * Command: timer list
 *----------------------------------------------------------------------------
 */

COMMAND_DO_WORK_UC(cl_timer_list_cmd)
{
  int i;
  for (i=0; i<uc->counters->count; i++)
    {
      class cl_ticker *t= (class cl_ticker *)(uc->counters->at(i));
      if (t)
	t->dump(i, con);
    }
  return false;
}

CMDHELP(cl_timer_list_cmd,
	"timer list",
	"List all defined timers",
	"")


/* End of cmd.src/cmd_timer.cc */
