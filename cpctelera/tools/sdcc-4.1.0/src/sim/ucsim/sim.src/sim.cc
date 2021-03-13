/*
 * Simulator of microcontrollers (sim.cc)
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

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
//#include "i_string.h"

// prj
#include "globals.h"
#include "utils.h"

// cmd
#include "cmd_execcl.h"

// local, sim.src
//#include "simcl.h"
//#include "appcl.h"
#include "simifcl.h"


/*
 * Simulator
 */

cl_sim::cl_sim(class cl_app *the_app):
  cl_base()
{
  app= the_app;
  uc= 0;
  state= SIM_NONE;
  //arguments= new cl_list(2, 2);
  //accept_args= more_args?strdup(more_args):0;
  gui= new cl_gui(this);
}

int
cl_sim::init(void)
{
  cl_base::init();
  build_cmdset(app->get_commander()->cmdset);
  if (!(uc= mk_controller()))
    return(1);
  uc->init();
  simif= uc->get_hw("simif", 0);
  return(0);
}

cl_sim::~cl_sim(void)
{
  if (uc)
    delete uc;
}

class cl_uc *
cl_sim::mk_controller(void)
{
  return(new cl_uc(this));
}


int
cl_sim::step(void)
{
  if (state & SIM_GO)
    {
      if (steps_done == 0)
	{
	  start_at= dnow();
	}
      uc->save_hist();
      if (uc->do_inst(1) == resGO)
	steps_done++;
      if ((steps_todo > 0) &&
	  (steps_done >= steps_todo))
	stop(resSTEP);
    }
  return(0);
}

/*int
cl_sim::do_cmd(char *cmdstr, class cl_console *console)
{
  class cl_cmdline *cmdline;
  class cl_cmd *cm;
  int retval= 0;

  cmdline= new cl_cmdline(cmdstr, console);
  cmdline->init();
  cm= cmd->cmdset->get_cmd(cmdline);
  if (cm)
    retval= cm->work(cmdline, console);
  delete cmdline;
  if (cm)
    return(retval);
  return(console->interpret(cmdstr));
}*/

void
cl_sim::start(class cl_console_base *con, unsigned long steps_to_do)
{
  state|= SIM_GO;
  if (con)
    {
      con->set_flag(CONS_FROZEN, true);
      app->get_commander()->frozen_console= con;
      app->get_commander()->update_active();
    }
  if (uc)
    start_tick= uc->ticks->ticks;
  steps_done= 0;
  steps_todo= steps_to_do;
}

void
cl_sim::stop(int reason, class cl_ev_brk *ebrk)
{
  class cl_commander_base *cmd= app->get_commander();
  class cl_option *o= app->options->get_option("quit");
  bool q_opt= false;

  if (o)
    o->get_value(&q_opt);
  
  state&= ~SIM_GO;
  stop_at= dnow();
  if (simif)
    simif->cfg_set(simif_reason, reason);

  class cl_brk *b= NULL;
  if (reason == resBREAKPOINT)
    {
      b= uc->fbrk_at(uc->PC);
    }
  else if (ebrk != NULL)
    {
      b= ebrk;
    }
  if (b)
    {
      class cl_option *o;
      o= app->options->get_option("beep_break");
      bool e= false;
      if (o) o->get_value(&e);
      if (e)
	cmd->frozen_console->dd_printf("\007");
    
      if (!(b->commands.empty()))
	{
	  o= app->options->get_option("echo_script");
	  e= false;
	  if (o) o->get_value(&e);
	  if (e)
	    cmd->dd_printf("%s\n", b->commands.c_str());
	  application->exec(b->commands);
	  steps_done= 0;
	}
    }
  
  if (!(state & SIM_GO) &&
      cmd->frozen_console)
    {
      if (reason == resUSER &&
	  cmd->frozen_console->input_avail())
	cmd->frozen_console->read_line();
      cmd->frozen_console->un_redirect();
      cmd->frozen_console->dd_color("debug");
      cmd->frozen_console->dd_printf("Stop at 0x%06x: (%d) ", AU(uc->PC), reason);
      switch (reason)
	{
	case resHALT:
	  cmd->frozen_console->dd_printf("Halted\n");
	  break;
	case resINV_ADDR:
	  cmd->frozen_console->dd_printf("Invalid address\n");
	  break;
	case resSTACK_OV:
	  cmd->frozen_console->dd_printf("Stack overflow\n");
	  break;
	case resBREAKPOINT:
	  cmd->frozen_console->dd_printf("Breakpoint\n");
	  if (cmd->frozen_console)
	    uc->print_regs(cmd->frozen_console);
	  break;
	case resEVENTBREAK:
	  cmd->frozen_console->dd_printf("Event break\n");
	  //uc->print_regs(cmd->frozen_console);
	  if (b)
	    {
	      class cl_ev_brk *eb= (cl_ev_brk*)b;
	      class cl_address_space *m= eb->get_mem();
	      char *dis = uc->disass(uc->instPC, " ");
	      cmd->frozen_console->dd_printf("Event `%s' at %s[0x%x]: 0x%x %s\n",
					     eb->id, m?(m->get_name()):"mem?",
					     AU(eb->addr),
					     AU(uc->instPC),
					     dis);
	      free(dis);
    	    }
	  break;
	case resINTERRUPT:
	  cmd->frozen_console->dd_printf("Interrupt\n");
	  break;
	case resWDTRESET:
	  cmd->frozen_console->dd_printf("Watchdog reset\n");
	  break;
	case resUSER:
	  cmd->frozen_console->dd_printf("User stopped\n");
	  break;
	case resINV_INST:
	  {
	    cmd->frozen_console->dd_printf("Invalid instruction");
	    if (uc->rom)
	      cmd->frozen_console->dd_printf(" 0x%04x\n",
					     MU32(uc->rom->get(uc->PC)));
	  }
         break;
	case resSTEP:
	  cmd->frozen_console->dd_printf("\n");
	  uc->print_regs(cmd->frozen_console);
	  break;
	case resERROR:
	  // uc::check_error prints error messages...
	  break;
	case resSIMIF:
	  cmd->frozen_console->dd_printf("Program stopped itself\n");
	  break;
	case resSELFJUMP:
	  cmd->frozen_console->dd_printf("Jump to itself\n");
	  break;
	default:
	  cmd->frozen_console->dd_printf("Unknown reason\n");
	  break;
	}
      cmd->frozen_console->dd_printf("F 0x%06x\n", AU(uc->PC)); // for sdcdb
      unsigned long dt= uc?(uc->ticks->ticks - start_tick):0;
      if ((reason != resSTEP) ||
	  (steps_done > 1))
	{
	  cmd->frozen_console->dd_printf("Simulated %lu ticks (%.3e sec)\n",
					 dt,
					 dt*(1/uc->xtal));
	  cmd->frozen_console->dd_printf("Host usage: %f sec, rate=%f\n",
					 stop_at - start_at,
					 (dt*(1/uc->xtal)) / (stop_at - start_at));
	}
      //if (cmd->actual_console != cmd->frozen_console)
      cmd->frozen_console->set_flag(CONS_FROZEN, false);
      //cmd->frozen_console->dd_printf("_s_");
      cmd->frozen_console->print_prompt();
      cmd->frozen_console= 0;
    }
  if (!(state & SIM_GO) &&
      q_opt)
    state|= SIM_QUIT;
  cmd->update_active();
}
/*
void
cl_sim::stop(class cl_ev_brk *brk)
{
  class cl_commander_base *cmd= app->get_commander();
  class cl_option *o= app->options->get_option("quit");
  bool q_opt= false;

  if (o)
    o->get_value(&q_opt);

  //state&= ~SIM_GO;
  if (simif)
    simif->cfg_set(simif_reason, resEVENTBREAK);

  if (brk)
    {
      if (!(brk->commands.empty()))
	{
	  application->exec(brk->commands);
	  steps_done= 0;
	  printf("event brk PC=%ld, simgo=%d\n",uc->PC,state&SIM_GO);
	}
    }

  if (!(state & SIM_GO) &&
      cmd->frozen_console)
    {
      class cl_console_base *con= cmd->frozen_console;
      con->dd_printf("Event `%s' at %s[0x%x]: 0x%x %s\n",
		     brk->id, brk->get_mem()->get_name(), (int)brk->addr,
		     (int)uc->instPC,
		     uc->disass(uc->instPC, " "));
    }
  if (!(state & SIM_GO) &&
      q_opt)
    state|= SIM_QUIT;
}
*/

void
cl_sim::change_run(int reason)
{
  if (state & SIM_GO)
    stop(reason);
  else
    start(0, 0);
}

/*
 */

void
cl_sim::build_cmdset(class cl_cmdset *cmdset)
{
  class cl_cmd *cmd;

  cmdset->add(cmd= new cl_run_cmd("run", 0));
  cmd->init();
  cmd->add_name("go");
  cmd->add_name("r");
  cmd->add_name("continue");
  
  cmdset->add(cmd= new cl_stop_cmd("stop", 0));
  cmd->init();

  cmdset->add(cmd= new cl_step_cmd("step", true));
  cmd->init();
  cmd->add_name("s");

  cmdset->add(cmd= new cl_next_cmd("next", true));
  cmd->init();
  cmd->add_name("n");

  //class cl_super_cmd *super_cmd;
  //class cl_cmdset *cset;
  /*
    {
    // info
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("info"));
    if (super_cmd)
      cset= super_cmd->get_subcommands();
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    if (!super_cmd) {
    cmdset->add(cmd= new cl_super_cmd("info", 0, cset));
    cmd->init();
    set_info_help(cmd);
    }
    }
  */
}


/*
 * Messages to broadcast
 */
/*
void
cl_sim::mem_cell_changed(class cl_address_space *mem, t_addr addr)
{
  if (uc)
    uc->mem_cell_changed(mem, addr);
  else
    printf("JAJ sim\n");
}
*/

/* End of sim.src/sim.cc */
