/*
 * Simulator of microcontrollers (sim.src/simif.cc)
 *
 * Copyright (C) 2016,16 Drotos Daniel, Talker Bt.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include "i_string.h"

// prj
#include "globals.h"

// sim
//#include "simcl.h"

#include "simifcl.h"


/* Interface command */

cl_sif_command::cl_sif_command(enum sif_command cmd,
			       const char *the_name,
			       const char *the_description,
			       enum sif_answer_type the_answer_type,
			       int the_params_needed,
			       class cl_simulator_interface *the_sif):
  cl_base()
{
  command= cmd;
  set_name(the_name);
  description= strdup(the_description);
  answer_type= the_answer_type;
  sif= the_sif;
  parameters= 0;
  answer= 0;
  params_needed= the_params_needed;
  nuof_params= params_received= 0;
  answer_length= answered_bytes= 0;
  answering= false;
}

cl_sif_command::~cl_sif_command(void)
{
  if (description)
    free(description);
  clear_params();
  clear_answer();
}

int
cl_sif_command::init(void)
{
  clear_params();
  clear_answer();
  return(0);
}

void
cl_sif_command::clear_params(void)
{
  nuof_params= 0;
  params_received= 0;
  if (parameters)
    free(parameters);
}

void
cl_sif_command::clear_answer(void)
{
  answer_length= 0;
  answered_bytes= 0;
  if (answer)
    free(answer);
  answer= 0;
}

void
cl_sif_command::clear(void)
{
  clear_params();
  clear_answer();
}


bool
cl_sif_command::get_parameter(int nr, t_mem *into)
{
  if (!parameters ||
      nr >= nuof_params)
    return(false);
  if (into)
    *into= parameters[nr];
  return(true);
}


t_mem
cl_sif_command::read(class cl_memory_cell *cel)
{
  t_mem ret= cel->get();

  if (answering &&
      answer)
    {
      if (answered_bytes < answer_length)
	{
	  ret= answer[answered_bytes];
	  answered_bytes++;
	}
      if (answered_bytes >= answer_length)
	{
	  sif->finish_command();
	}
    }
  return(ret);
}

void
cl_sif_command::write(class cl_memory_cell *cel, t_mem *val)
{
  if (nuof_params &&
      params_received < nuof_params &&
      parameters)
    {
      parameters[params_received]= *val;
      params_received++;
      if (params_received >= nuof_params)
	{
	  produce_answer();
	  start_answer();
	}
    }
}


void
cl_sif_command::start(void)
{
  need_params(params_needed);
}

void
cl_sif_command::need_params(int nr)
{
  clear_params();
  if (nr <= 0)
    {
      produce_answer();
      start_answer();
    }
  nuof_params= nr;
  params_received= 0;
  parameters= (t_mem *)calloc(nr, sizeof(t_mem));
}

void
cl_sif_command::produce_answer(void)
{
  clear_answer();
}

void
cl_sif_command::set_answer(t_mem ans)
{
  clear_answer();
  answer= (t_mem *)calloc(1, sizeof(t_mem));
  answer[0]= ans;
  answer_length= 1;
}

void
cl_sif_command::set_answer(int nr, t_mem ans[])
{
  clear_answer();
  answer= (t_mem *)calloc(nr+1, sizeof(t_mem));
  answer[0]= nr;
  int i;
  for (i= 0; i < nr; i++)
    answer[i+1]= ans[i];
  answer_length= nr+1;
}

void
cl_sif_command::set_answer(const char *ans)
{
  clear_answer();
  if (ans &&
      *ans)
    {
      answer= (t_mem *)calloc(strlen(ans)+2, sizeof(t_mem));
      int i= 0;
      answer[0]= strlen(ans)+1;
      while (ans[i])
	{
	  answer[i+1]= ans[i];
	  i++;
	}
      answer[i+1]= '\0';
      answer_length= i+2;
    }
}

void
cl_sif_command::start_answer(void)
{
  if (answer_length)
    answering= true;
  else
    {
      answering= false;
      sif->finish_command();
    }
}


/* Command: get info about commands */

void
cl_sif_commands::produce_answer(void)
{
  int c, i;
  if (!sif)
    return;
  c= sif->commands->count;
  answer= (t_mem*)calloc(c+1, sizeof(t_mem));
  answer[0]= c;
  for (i= 0; i < c; i++)
    {
      answer[i+1]= 0;
      class cl_sif_command *sc=
	dynamic_cast<class cl_sif_command *>(sif->commands->object_at(i));
      if (!sc)
	continue;
      answer[i+1]= sc->get_command();
    }
  answer_length= c+1;
}


/* Command: get info about a command */

void
cl_sif_cmdinfo::produce_answer(void)
{
  int i;
  if (!sif)
    return;
  t_mem cm;
  if (!get_parameter(0, &cm))
    return;
  answer= (t_mem*)calloc(1+2, sizeof(t_mem));
  answer[0]= 2;
  class cl_sif_command *about= 0;
  for (i= 0; i < sif->commands->count; i++)
    {
      class cl_sif_command *sc=
	dynamic_cast<class cl_sif_command *>(sif->commands->object_at(i));
      if (sc->get_command() == (enum sif_command)cm)
	{
	  about= sc;
	  break;
	}
    }
  if (about)
    {
      answer[1]= about->get_params_needed();
      answer[2]= about->get_answer_type();
    }
  answer_length= 3;
}


/* Command: get help about a command */

void
cl_sif_cmdhelp::produce_answer(void)
{
  int i;
  if (!sif)
    return;
  t_mem cm;
  if (!get_parameter(0, &cm))
    return;
  class cl_sif_command *about= 0;
  for (i= 0; i < sif->commands->count; i++)
    {
      class cl_sif_command *sc=
	dynamic_cast<class cl_sif_command *>(sif->commands->object_at(i));
      if (sc->get_command() == (enum sif_command)cm)
	{
	  about= sc;
	  break;
	}
    }
  if (about)
    set_answer(about->get_description());
  else
    set_answer((t_mem)0);
}


/* Command: stop simulation */

void
cl_sif_stop::produce_answer(void)
{
  class cl_sim *sim= 0;
  if (sif)
    if (sif->uc)
      sim= sif->uc->sim;
  set_answer(SIFCM_STOP);
  if (sim)
    sim->stop(resSIMIF);
}


/* Command: print character */

void
cl_sif_print::produce_answer(void)
{
  t_mem cm;
  if (get_parameter(0, &cm))
    {
      //printf("** SIF_PRINT 0x%02x,'%c'\n", cm, cm);
      putchar(cm);
      fflush(stdout);
    }
  if (sif)
    sif->finish_command();
}


/* Command: print character in hex */

void
cl_sif_hex::produce_answer(void)
{
  t_mem cm;
  if (get_parameter(0, &cm))
    {
      //printf("** SIF_HEX 0x%02x,'%c'\n", cm, cm);
      printf("%02x", cm);
      fflush(stdout);
    }
  if (sif)
    sif->finish_command();
}


/* Command: write character to output file */

void
cl_sif_write::produce_answer(void)
{
  t_mem cm;
  if (sif)
    {
      if (get_parameter(0, &cm))
	{
	  if (sif->fout)
	    {
	      char c= cm;
	      sif->fout->write(&c, 1);
	    }
	}
    }
  if (sif)
    sif->finish_command();
}


/* Command: check input file */

void
cl_sif_fin_check::produce_answer(void)
{
  int i= 0;
  if (sif)
    {
      if (sif->fin)
	{
	  i= sif->fin->input_avail();
	  if (i)
	    {
	      if (sif->fin->eof())
		i= 0;
	    }
	  i= i?1:0;
	}
    }
  set_answer(i);
  answer_length= 1;
}


/* Command: read from input file */

void
cl_sif_read::produce_answer(void)
{
  int i= 0;
  if (sif)
    {
      if (sif->fin)
	{
	  int c;
	  if (sif->fin->input_avail())
	    {
	      i= sif->fin->read(&c, 1);
	      if (i != 0)
		i= c;
	    }
	}
    }
  set_answer(i);
  answer_length= 1;
}


/* Command: reset */

void
cl_sif_reset::produce_answer(void)
{
  if (sif)
    {
      if (sif->uc)
	sif->uc->reset();
      sif->finish_command();
    }
}


/*
 * Virtual HW: simulator interface
 */

cl_simulator_interface::cl_simulator_interface(class cl_uc *auc):
  cl_hw(auc, HW_SIMIF, 0, "simif")
{
  version= 1;
  as_name= NULL;
  addr= 0;
  commands= new cl_list(2, 2, "sif_commands");
  active_command= 0;
}

cl_simulator_interface::~cl_simulator_interface(void)
{
  if (as_name)
    free((void*)as_name);
  delete commands;
}

int
cl_simulator_interface::init(void)
{
  cl_option *oas= application->options->get_option("simif_memory");
  cl_option *oa = application->options->get_option("simif_address");
  char *oas_v;
  long oa_v;
  
  cl_hw::init();
  fin= fout= NULL;

  if (oas &&
      oa)
    {
      oas->get_value(&oas_v);
      oa->get_value(&oa_v);
      if (oas_v &&
	  (strlen(oas_v) > 0))
	{
	  if (as_name)
	    free((void*)as_name);
	  as_name= strdup(oas_v);
	  addr= oa_v;
	}
    }
  
  if (as_name)
    {
      as= uc->address_space(as_name);
      if (as)
	{
	  address= addr;
	  if (addr < 0)
	    address= as->highest_valid_address();
	  cell= register_cell(as, address);
	}
      else
	fprintf(stderr, "Simif: %s is not a valid address space name\n", as_name);
    }
  else
    {
      as= NULL;
      cell= NULL;
    }

  class cl_option *fo= application->options->get_option("simif_infile");
  char *s;
  if (fo)
    {
      fo->get_value(&s);
      if (s &&
	  (strlen(s) > 0))
	fin= mk_io(s, "r");
    }
  fo= application->options->get_option("simif_outfile");
  if (fo)
    {
      fo->get_value(&s);
      if (s &&
	  (strlen(s) > 0))
	fout= mk_io(s, "w");
    }

  class cl_sif_command *c;
  commands->add(c= new cl_sif_detect(this));
  c->init();
  commands->add(c= new cl_sif_commands(this));
  c->init();
  commands->add(c= new cl_sif_ifver(this));
  c->init();
  commands->add(c= new cl_sif_simver(this));
  c->init();
  commands->add(c= new cl_sif_ifreset(this));
  c->init();
  commands->add(c= new cl_sif_cmdinfo(this));
  c->init();
  commands->add(c= new cl_sif_cmdhelp(this));
  c->init();
  commands->add(c= new cl_sif_stop(this));
  c->init();
  commands->add(c= new cl_sif_print(this));
  c->init();
  commands->add(c= new cl_sif_hex(this));
  c->init();
  commands->add(c= new cl_sif_fin_check(this));
  c->init();
  commands->add(c= new cl_sif_read(this));
  c->init();
  commands->add(c= new cl_sif_write(this));
  c->init();

  cl_var *v;
  uc->vars->add(v= new cl_var(cchars("simif_on"), cfg, simif_on,
			      cfg_help(simif_on)));
  v->init();
  cfg_set(simif_on, 1);
  uc->vars->add(v= new cl_var(cchars("sim_run"), cfg, simif_run,
			      cfg_help(simif_run)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_start"), cfg, simif_start,
			      cfg_help(simif_start)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_stop"), cfg, simif_stop,
			      cfg_help(simif_stop)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_quit"), cfg, simif_quit,
			      cfg_help(simif_quit)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_reason"), cfg, simif_reason,
			      cfg_help(simif_reason)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_xtal"), cfg, simif_xtal,
			      cfg_help(simif_xtal)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_ticks"), cfg, simif_ticks,
			      cfg_help(simif_ticks)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_isr_ticks"), cfg, simif_isr_ticks,
			      cfg_help(simif_isr_ticks)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_idle_ticks"), cfg, simif_idle_ticks,
			      cfg_help(simif_idle_ticks)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_real_time"), cfg, simif_real_time,
			      cfg_help(simif_real_time)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_vclk"), cfg, simif_vclk,
			      cfg_help(simif_vclk)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("PC"), cfg, simif_pc,
			      cfg_help(simif_pc)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_print"), cfg, simif_print,
			      cfg_help(simif_print)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("sim_write"), cfg, simif_write,
			      cfg_help(simif_write)));
  v->init();

  return(0);
}

char *
cl_simulator_interface::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case simif_on	: return (char*)"Turn simif on/off (bool, RW)";
    case simif_run	: return (char*)"WR: sets running state, RD: check if simulation is running";
    case simif_start	: return (char*)"WR: start simulation, RD: true if running";
    case simif_stop	: return (char*)"WR: stop simulation, RD: true if stopped";
    case simif_quit	: return (char*)"Quit simulator (any, WO)";
    case simif_reason	: return (char*)"Reason of last stop (int, RO)";
    case simif_xtal	: return (char*)"Xtal frequency in Hz (int, RW)";
    case simif_ticks	: return (char*)"Nuof ticks simulated so far (int, RO)";
    case simif_isr_ticks: return (char*)"Ticks spent in ISR (int, RO)";
    case simif_idle_ticks:return (char*)"Ticks spent in idle state (int, RO)";
    case simif_real_time: return (char*)"Real time since reset in msec (int, RO)";
    case simif_vclk	: return (char*)"Nuof simulated virtual clocks (int, RO)";
    case simif_pc	: return (char*)"PC register (int, RW)";
    case simif_print	: return (char*)"Print char on stdout (int, WO)";
    case simif_write	: return (char*)"Write char to simif output (int, WO)";
    }
  return (char*)"Not used";
}

void
cl_simulator_interface::set_cmd(class cl_cmdline *cmdline,
				class cl_console_base *con)
{
  class cl_cmd_arg *params[2]= {
    cmdline->param(0),
    cmdline->param(1)
  };

  if (cmdline->syntax_match(uc, MEMORY ADDRESS))
    {
      class cl_memory *mem= params[0]->value.memory.memory;
      t_addr a= params[1]->value.address;
      if (!mem->is_address_space())
	{
	  con->dd_printf("%s is not an address space\n");
	  return;
	}
      if (!mem->valid_address(a))
	{
	  con->dd_printf("Address must be between 0x%x and 0x%x\n",
			 AU(mem->lowest_valid_address()),
			 AU(mem->highest_valid_address()));
	  return;
	}
      as_name= strdup((char*)mem->get_name());
      addr= a;
      if ((as= dynamic_cast<class cl_address_space *>(mem)) != 0)
	{
	  address= addr;
	  if (addr < 0)
	    address= as->highest_valid_address();
	  if (cell != NULL)
	    unregister_cell(cell);
	  cell= register_cell(as, address);
	}
    }
  else if (cmdline->syntax_match(uc, STRING STRING))
    {
      char *p1= params[0]->value.string.string;
      char *p2= params[1]->value.string.string;
      if (strcmp(p1, "fout") == 0)
	{
	  if (fout)
	    delete fout;
	  fout= 0;
	  if ((strcmp(p2, "NULL") != 0) &&
	      (strcmp(p2, "(NULL)") != 0))
	    {
	      fout= mk_io(p2, "w");
	    }
	}
      else if (strcmp(p1, "fin") == 0)
	{
	  if (fin)
	    delete fin;
	  fin= 0;
	  if ((strcmp(p2, "NULL") != 0) &&
	      (strcmp(p2, "(NULL)") != 0))
	    {
	      fin= mk_io(p2, "r");
	    }
	}
    }
  else
    {
      con->dd_printf("set hardware simif memory address\n");
      con->dd_printf("set hardware simif fin \"input_file_name\"\n");
      con->dd_printf("set hardware simif fout \"output_file_name\"\n");
    }
}


t_mem
cl_simulator_interface::read(class cl_memory_cell *cel)
{
  if (conf(cel, NULL))
    return cel->get();
  if (!active_command)
    {
      t_mem d= cel->get();
      return(~d & cel->get_mask());
    }
  else
    {
      t_mem ret= active_command->read(cel);
      return(ret);
    }
  return(cel->get());
}

void
cl_simulator_interface::write(class cl_memory_cell *cel, t_mem *val)
{
  if (conf(cel, val))
    return;
  if (!active_command)
    {
      int i;
      for (i= 0; i < commands->count; i++)
	{
	  class cl_sif_command *c=
	    dynamic_cast<class cl_sif_command *>(commands->object_at(i));
	  if (!c)
	    continue;
	  enum sif_command cm= c->get_command();
	  if ((enum sif_command)(*val) == cm)
	    {
	      active_command= c;
	      c->start();
	      return;
	    }
	}
    }
  else
    {
      active_command->write(cel, val);
    }
}

t_mem
cl_simulator_interface::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum simif_cfg)addr)
    {
    case simif_on: // turn this HW on/off
      if (val)
	{
	  if (*val)
	    on= true;
	  else
	    on= false;
	}
      else
	{
	  cell->set(on?1:0);
	}
      break;
    case simif_run: // simulator running: true= run, false= stop
      if (val)
	{
	  if (*val)
	    {
	      uc->sim->start(0, 0), *val= 1;
	    }
	  else
	    uc->sim->stop(resSIMIF), *val= 0;
	}
      else
	{
	  int i= uc->sim->state;
	  i= i & SIM_GO;
	  cell->set(i?1:0);
	}
      break;
    case simif_start: // start simulation
      if (val)
	uc->sim->start(0, 0), *val= 1;
      else
	{
	  int i= uc->sim->state;
	  i= i & SIM_GO;
	  cell->set(i?1:0);
	}
      break;
    case simif_stop: // stop simulation
      if (val)
	uc->sim->stop(resSIMIF), *val= 1;
      else
	{
	  int i= uc->sim->state;
	  i= i & SIM_GO;
	  cell->set((!i)?1:0);
	}
      break;
    case simif_quit: // quit
      if (val)
	uc->sim->state|= SIM_QUIT;
      break;
    case simif_reason: // reason of last stop
      if (val)
	*val= cell->get();
      break;
    case simif_xtal: // xtal frequ
      if (val)
	uc->xtal= *val;
      cell->set(uc->xtal);
      break;
    case simif_ticks: // tick counter
      if (val)
	*val= cell->get();
      cell->set(uc->ticks->ticks);
      break;
    case simif_isr_ticks: // isr tick counter
      if (val)
	*val= cell->get();
      cell->set(uc->isr_ticks->ticks);
      break;
    case simif_idle_ticks: // idle tick counter
      if (val)
	*val= cell->get();
      cell->set(uc->idle_ticks->ticks);
      break;
    case simif_real_time: // real time in msec
      if (val)
	*val= cell->get();
      cell->set(uc->get_rtime() * 1000);
      break;
    case simif_vclk: // virtual clock
      if (val)
	*val= cell->get();
      cell->set(uc->vc.fetch + uc->vc.rd + uc->vc.wr);
      break;
    case simif_pc: // PC of uc
      if (val)
	{
	  t_addr addr= *val;
	  class cl_address_space *rom= uc->rom;
	  if (rom)
	    {
	      if (addr > rom->highest_valid_address())
		addr= rom->highest_valid_address();
	    }
	  uc->PC= addr;
	}
      cell->set(uc->PC);
      break;
    case simif_print:
      if (val)
	{
	  printf("%c", (char)(*val&0xff));
	  fflush(stdout);
	}
      break;
    case simif_write:
      if (val)
	{
	  char c= *val & 0xff;
	  if (fout)
	    fout->write(&c, 1);
	}
      break;
    case simif_nuof:
      break;
    }
  return cell->get();
}

void
cl_simulator_interface::finish_command(void)
{
  if (active_command)
    {
      active_command->clear_answer();
    }
  active_command= 0;
}


void
cl_simulator_interface::print_info(class cl_console_base *con)
{
  con->dd_printf("uCsim simulator interface, version %d, ", version);
  con->dd_printf("at %s[", as_name);
  if (as)
    con->dd_printf(as->addr_format, address);
  else
    con->dd_printf("0x%x", AU(address));
  con->dd_printf("]\n");
  
  con->dd_printf("Active command: ");
  if (!active_command)
    con->dd_printf("none.\n");
  else
    {
      class cl_sif_command *c= active_command;
      con->dd_printf("0x%02x %s %s\n", c->get_command(),
		     c->get_name(), c->get_description());
      con->dd_printf("Parameters received %d bytes of %d\n",
		     c->get_params_received(), c->get_nuof_params());
      if (c->get_nuof_params())
	{
	  con->dd_printf(" ");
	  int i;
	  for (i= 0; i < c->get_nuof_params(); i++)
	    {
	      t_mem p;
	      if (c->get_parameter(i, &p))
		con->dd_printf(" %02x", MU8(p));
	      else
		con->dd_printf(" --");
	    }
	  con->dd_printf("\n");
	}
      con->dd_printf("Answered %d bytes of %d\n",
		     c->get_answered_bytes(), c->get_answer_length());
      con->dd_printf("Answering: %s\n", (c->get_answering())?"yes":"no");
    }
  
  int i;
  con->dd_printf("Known commands:\n");
  for (i= 0; i < commands->count; i++)
    {
      class cl_sif_command *c=
	dynamic_cast<class cl_sif_command *>(commands->object_at(i));
      if (!c)
	continue;
      int cmd= c->get_command();
      con->dd_printf("0x%02x/%c %s: %s\n", cmd,
		     isprint(cmd)?cmd:' ',
		     c->get_name(), c->get_description());
    }
  
  con->dd_printf("Input file: ");
  if (fin)
    con->dd_printf("%s", fin->get_file_name());
  con->dd_printf("\n");
  
  con->dd_printf("Output file: ");
  if (fout)
    con->dd_printf("%s", fout->get_file_name());
  con->dd_printf("\n");

  print_cfg_info(con);
}


/* End of sim.src/simif.cc */
