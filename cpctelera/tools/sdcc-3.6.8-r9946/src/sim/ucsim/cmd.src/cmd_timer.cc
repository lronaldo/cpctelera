/*
 * Simulator of microcontrollers (cmd.src/timer.cc)
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

#include "ddconfig.h"

#include "stdio.h"
#include "i_string.h"

// sim
#include "simcl.h"

// local
#include "cmd_timercl.h"


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
      con->dd_printf("Timer id is missing.");
      return(false);
    }
  if (params[0]->as_number())
    {
      as_nr= true;
      id_nr= params[0]->value.number;
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


/*
 * Command: timer add
 *-----------------------------------------------------------------------------
 * Add a new timer to the list
 */

COMMAND_DO_WORK_UC(cl_timer_add_cmd)
  //add(class cl_uc *uc, class cl_cmdline *cmdline, class cl_console *con)
{
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };
  long dir= +1, in_isr= 0;

  if (cl_timer_cmd::do_work(uc, cmdline, con))
    return(false);
  if (ticker)
    {
      if (!as_nr)
	con->dd_printf("Error: Timer \"%s\" already exists\n", id_str);
      else
	con->dd_printf("Error: Timer %d already exists\n", id_nr);
      return(false);
    }

  if (cmdline->nuof_params() > 0)
    {
      if (cmdline->syntax_match(uc, NUMBER))
	dir= params[0]->value.number;
      else if (cmdline->syntax_match(uc, NUMBER NUMBER))
	{
	  dir= params[0]->value.number;
	  in_isr= params[1]->value.number;
	}
    }

  if (!as_nr)
    {
      ticker= new cl_ticker(dir, in_isr, id_str);
      uc->add_counter(ticker, id_str);
    }
  else
    {
      ticker= new cl_ticker(dir, in_isr, 0);
      uc->add_counter(ticker, id_nr);
    }

  return(false);
}

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
	con->dd_printf("Timer \"%s\" does not exist\n", id_str);
      else
	con->dd_printf("Timer %d does not exist\n", id_nr);
      return(false);
    }
  if (!as_nr)
    uc->del_counter(id_str);
  else
    uc->del_counter(id_nr);

  return(false);
}

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
    ticker->dump(id_nr, uc->xtal, con);
  else
    {
      uc->ticks->dump(0, uc->xtal, con);
      uc->isr_ticks->dump(0, uc->xtal, con);
      uc->idle_ticks->dump(0, uc->xtal, con);
      for (id_nr= 0; id_nr < uc->counters->count; id_nr++)
	{
	  ticker= uc->get_counter(id_nr);
	  if (ticker)
	    ticker->dump(id_nr, uc->xtal, con);
	}
    }

  return(false);
}

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
	con->dd_printf("Timer %d does not exist\n", id_str);
      else
	con->dd_printf("Timer %d does not exist\n", id_nr);
      return(0);
    }
  ticker->options|= TICK_RUN;

  return(false);
}

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
	con->dd_printf("Timer %d does not exist\n", id_str);
      else
	con->dd_printf("Timer %d does not exist\n", id_nr);
      return(false);
    }
  ticker->options&= ~TICK_RUN;

  return(false);
}


/*
 * Command: timer value
 *-----------------------------------------------------------------------------
 * Set a timer to a specified value
 */

COMMAND_DO_WORK_UC(cl_timer_value_cmd)
  //val(class cl_uc *uc, class cl_cmdline *cmdline, class cl_console *con)
{
  class cl_cmd_arg *params[4]= { cmdline->param(0),
				 cmdline->param(1),
				 cmdline->param(2),
				 cmdline->param(3) };
  
  if (cl_timer_cmd::do_work(uc, cmdline, con))
    return(false);
  if (!ticker)
    {
      if (!as_nr)
	con->dd_printf("Error: Timer %d does not exist\n", id_str);
      else
	con->dd_printf("Error: Timer %d does not exist\n", id_nr);
      return(false);
    }
  if (params[2])
    {
      con->dd_printf("Error: Value is missing\n");
      return(false);
    }
  long val;
  if (!params[2]->get_ivalue(&val))
    {
      con->dd_printf("Error: Wrong parameter\n");
      return(false);
    }
  ticker->ticks= val;

  return(false);
}


/* End of cmd.src/cmd_timer.cc */
