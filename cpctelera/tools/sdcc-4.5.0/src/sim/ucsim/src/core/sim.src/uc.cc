/*
 * Simulator of microcontrollers (uc.cc)
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

//#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
//#include "i_string.h"

// prj
#include "globals.h"
#include "utils.h"

// cmd.src
//#include "newcmdcl.h"
//#include "cmdutil.h"
#include "cmd_uccl.h"
#include "cmd_bpcl.h"
#include "cmd_getcl.h"
#include "cmd_setcl.h"
#include "cmd_infocl.h"
#include "cmd_timercl.h"
#include "cmd_statcl.h"
#include "cmd_memcl.h"
#include "cmd_execcl.h"
#include "cmd_showcl.h"

// local, sim.src
#include "uccl.h"
//#include "hwcl.h"
//#include "memcl.h"
//#include "simcl.h"
#include "itsrccl.h"
#include "simifcl.h"
#include "vcdcl.h"


static class cl_uc_error_registry uc_error_registry;

/*
 * Clock counter
 */

cl_ticker::cl_ticker(int adir, enum ticker_type atype, const char *aname, bool auser)
{
  dir= adir;
  type= atype;
  ticks= 0;
  run = true;
  set_name(aname);
  user= auser;
}

cl_ticker::~cl_ticker(void) {}

void
cl_ticker::tick(int nr, double time)
{
  if (dir >= 0)
    {
      ticks+= nr;
      rtime+= time;
    }
  else
    {
      ticks-= nr;
      rtime-= time;
    }
}

void
cl_ticker::dump(int nr, class cl_console_base *con)
{
  const char *type_names[] = { "", ",ISR", ",IDLE", ",HALT" };

  con->dd_printf("timer #%d(\"%s\") %s%s %.15f sec (%lu clks)\n",
		 nr, get_name("unnamed"),
		 (run ? "ON" : "OFF"),
		 type_names[type],
		 rtime, ticks);
}


/*
 * Options of uc
 */

cl_xtal_option::cl_xtal_option(class cl_uc *the_uc):
  cl_optref(the_uc)
{
  uc= the_uc;
}

void
cl_xtal_option::option_changed(void)
{
  if (!uc)
    return;
  double d;
  option->get_value(&d);
  uc->set_xtal(d);
}

cl_stop_selfjump_option::cl_stop_selfjump_option(class cl_uc *the_uc):
  cl_optref(the_uc)
{
  uc= the_uc;
}

int
cl_stop_selfjump_option::init(void)
{
  cl_optref::init();
  create(uc, bool_opt, "selfjump_stop", "Stop when jump branches to itself");
  return 0;
}

void
cl_stop_selfjump_option::option_changed(void)
{
  bool b;
  option->get_value(&b);
  uc->stop_selfjump= b;
}


cl_analyzer_opt::cl_analyzer_opt(class cl_uc *Iuc):
  cl_bool_option(Iuc, "analyzer", "Analyze and label code")
{
  uc= Iuc;
}

void
cl_analyzer_opt::set_value(const char *s)
{
  bool b;
  cl_bool_option::set_value(s);
  get_value(&b);
  uc->set_analyzer(b);
  if (uc->analyser_opt) uc->analyser_opt->set_raw(b);
}


cl_analyser_opt::cl_analyser_opt(class cl_uc *Iuc):
  cl_bool_option(Iuc, "analyser", "Analyse and label code")
{
  uc= Iuc;
}

void
cl_analyser_opt::set_value(const char *s)
{
  bool b;
  cl_bool_option::set_value(s);
  get_value(&b);
  uc->set_analyzer(b);
  if (uc->analyzer_opt) uc->analyzer_opt->set_raw(b);
}

/* Time measurer */

cl_time_measurer::cl_time_measurer(class cl_uc *the_uc):
  cl_base()
{
  to_reach= 0;
  uc= the_uc;
}

void
cl_time_measurer::set_reach(unsigned long val)
{
  to_reach= val;
}

void
cl_time_measurer::from_now(unsigned long val)
{
  set_reach(now() + val);
}

bool
cl_time_measurer::reached()
{
  return to_reach &&
    (now() >= to_reach);
}

unsigned long
cl_time_measurer::now()
{
  return 0;
}


/* value of xtal clock */

unsigned long
cl_time_clk::now()
{
  if (!uc) return 0;
  return uc->ticks->get_ticks();
}


/* value of virtual clocks */

unsigned long
cl_time_vclk::now()
{
  if (!uc) return 0;
  return uc->vc.fetch + uc->vc.rd + uc->vc.wr;
}


/* value of fetches */

unsigned long
cl_time_fclk::now()
{
  if (!uc) return 0;
  return uc->vc.fetch;
}


/* value of reads */

unsigned long
cl_time_rclk::now()
{
  if (!uc) return 0;
  return uc->vc.rd;
}


/* value of writes */

unsigned long
cl_time_wclk::now()
{
  if (!uc) return 0;
  return uc->vc.wr;
}


/* OMF file record */

cl_omf_rec::cl_omf_rec(void):
  cl_base()
{
  offset= 0;
  f_offset= 0;
  type= 0;
  len= 0;
  rec= NULL;
  chk= 0;
}

cl_omf_rec::~cl_omf_rec(void)
{
  if (rec)
    free(rec);
}

unsigned char
cl_omf_rec::g(cl_f *f)
{
  unsigned char c= f->get_c();
  offset++;
  return c;
}

u16_t
cl_omf_rec::pick_word(int i)
{
  unsigned char h, l;
  u16_t w;
  
  if (i >= len-1)
    return 0;
  l= rec[i];
  h= rec[i+1];
  w= h*256+l;
  return w;
}

chars
cl_omf_rec::pick_str(int i)
{
  unsigned char l, j;
  chars s= "";
  
  if (i >= len-1)
    return chars("");
  l= rec[i];
  j= 0;
  while (l &&
	 (i < len))
    {
      s+= rec[i+1+j];
      l--;
      j++;
    }
  return s;
}

bool
cl_omf_rec::read(cl_f *f)
{
  unsigned char c;
  int i, l, h;

  if (rec)
    {
      free(rec);
      rec= NULL;
    }
  f_offset= offset;
  if (f->eof())
    return false;
  c= g(f);
  type= c;
  if (f->eof())
    return false;
  c= g(f);
  l= c;
  if (f->eof())
    return false;
  c= g(f);
  h= c;
  if (f->eof())
    return false;
  len= h*256+l-1;
  rec= (u8_t*)malloc(len);
  for (i= 0; i < len; i++)
    {
      rec[i]= g(f);
      if (f->eof())
	return false;
    }
  chk= g(f);
  
  return true;
}


/*
 * Execution history
 */

cl_exec_hist::cl_exec_hist(class cl_uc *auc):
  cl_base()
{
  uc= auc;
  len= 10001;
  hist= (struct t_hist_elem*)malloc(sizeof(struct t_hist_elem) * len);
  t= h= 0;
}

cl_exec_hist::~cl_exec_hist(void)
{
  if (hist)
    free(hist);
}

int
cl_exec_hist::init(void)
{
  return 0;
}

void
cl_exec_hist::put(void)
{
  t_addr pc;
  if (!uc)
    return;
  pc= uc->PC;
  if (t != h)
    {
      if (hist[h].addr == pc)
	{
	  hist[h].nr++;
	  return;
	}
    }
  int nh= (h+1)%len;
  if (t == nh)
    t= (t+1)%len;
  h= nh;
  hist[h].addr= pc;
  hist[h].nr= 1;
}

void
cl_exec_hist::list(class cl_console_base *con, bool inc, int nr)
{
  int s, p, ta, l;
  if (!con)
    return;
  if (t==h)
    return;
  if (nr > len-1)
    nr= len-1;
  if (nr > get_used())
    nr= get_used();
  s= h-nr+1;
  if (s<0)
    s+= len;
  //s%= len;
  ta= (t+1)%len;

  p= inc?s:h;
  do
    {
      if (!uc)
	{
	  l= con->dd_cprintf("dump_address", "0x%06x", AU(hist[p].addr));
	}
      else
	{
	  l= uc->print_disass(hist[p].addr, con, false);
	}
      if (hist[p].nr > 1)
	{
	  l++; con->dd_printf(" ");
	  while (l%8 != 0)
	    l++, con->dd_printf(" ");
	  con->dd_printf("(%d times)", hist[p].nr);
	}
      con->dd_printf("\n");
      if (inc)
	{
	  if (p==h)
	    break;
	  p= (p+1)%len;
	}
      else
	{
	  if (p==ta)
	    break;
	  if (p==0)
	    p=len-1;
	  else
	    p= (p-1)%len;
	}
      con->dd_color("answer");
    }
  while (1);
}

void
cl_exec_hist::keep(int nr)
{
  if (nr < 0)
    nr= 0;
  if (t==h)
    return;
  while (get_used() > nr)
    t= (t+1)%len;
}

int
cl_exec_hist::get_used()
{
  if (t==h)
    return 0;
  if (h>t)
    return h-t;
  return len-t + h;
}

unsigned int
cl_exec_hist::get_insts()
{
  unsigned int i= 0;
  int p;
  if (t==h)
    return 0;
  p= (t+1)%len;
  do
    {
      i+= hist[p].nr;
      if (p==h)
	break;
      p= (p+1)%len;
    }
  while (1);
  return i;
}


/*
 * Abstract microcontroller
 ******************************************************************************
 */

cl_uc::cl_uc(class cl_sim *asim):
  cl_itab()
{
  PCmask= 0xffff;
  type= NULL;
  //int i;
  sim = asim;
  //mems= new cl_list(MEM_TYPES, 1);
  memchips= new cl_memory_list(this, "memchips");
  address_spaces= new cl_memory_list(this, "address_spaces");
  //address_decoders= new cl_list(2, 2);
  rom= 0;

  hws = new cl_hws();
  //options= new cl_list(2, 2);
  //for (i= MEM_ROM; i < MEM_TYPES; i++) mems->add(0);
  xtal_option= new cl_xtal_option(this);
  xtal_option->init();
  stop_selfjump_option= new cl_stop_selfjump_option(this);
  stop_selfjump_option->init();
  analyzer_opt= NULL;
  analyser_opt= NULL;
  counters= new cl_list(2, 2, "counters");
  ticks= new cl_ticker(+1, TICK_ANY, "time", false);
  add_counter(ticks, ticks->get_name());
  isr_ticks= new cl_ticker(+1, TICK_INISR, "isr", false);
  add_counter(isr_ticks, isr_ticks->get_name());
  idle_ticks= new cl_ticker(+1, TICK_IDLE, "idle", false);
  add_counter(idle_ticks, idle_ticks->get_name());
  halt_ticks= new cl_ticker(+1, TICK_HALT, "halt", false);
  add_counter(halt_ticks, halt_ticks->get_name());
  it_levels= new cl_list(2, 2, "it levels");
  it_sources= new cl_irqs(2, 2);
  class it_level *il= new it_level(-1, 0, 0, 0);
  it_levels->push(il);
  stack_ops= new cl_list(2, 2, "stack operations");
  errors= new cl_list(2, 2, "errors in uc");
  events= new cl_list(2, 2, "events in uc");
  sp_most= 0;
  //sp_avg= 0;
  inst_exec= false;
  hist= new cl_exec_hist(this);
}


cl_uc::~cl_uc(void)
{
  //delete mems;
  delete hws;
  //delete options;
  delete ticks;
  delete isr_ticks;
  delete idle_ticks;
  delete halt_ticks;
  delete counters;
  events->disconn_all();
  delete events;
  delete fbrk;
  delete ebrk;
  delete displays;
  delete it_levels;
  delete it_sources;
  delete stack_ops;
  errors->free_all();
  delete errors;
  delete xtal_option;
  delete address_spaces;
  delete memchips;
  //delete address_decoders;
  delete hist;
}


int
cl_uc::init(void)
{
  int i;
  double x= 0;
  
  set_name("controller");
  cl_base::init();
  if (xtal_option->use("xtal"))
    x= xtal_option->get_value(xtal);
  if (x==0)
    x= def_xtal();
  set_xtal(x);
  stop_selfjump= false;
  stop_selfjump_option->option->set_value(stop_selfjump);
  analyzer= false;
  analyzer_opt= new cl_analyzer_opt(this);
  analyzer_opt->init();
  analyzer_opt->set_raw(analyzer);
  analyser_opt= new cl_analyser_opt(this);
  analyser_opt->init();
  analyser_opt->set_raw(analyzer);
  application->options->add(analyzer_opt);
  application->options->add(analyser_opt);
  vars= new cl_var_list();
  make_variables();
  // Some app level vars:
  reg_cell_var(&(application->cperiod),
	       &(application->period),
	       "cperiod",
	       "Cycles between input checks");
  reg_cell_var(&(application->ccyc),
	       &(application->cyc),
	       "ccyc",
	       "Nr of cycles executed during simulation");
  reg_cell_var(&cPC, &PC, "PC", "Program counter");
  // Memories
  make_memories();
  if (rom == NULL)
    rom= address_space("rom"/*MEM_ROM_ID*/);
  ebrk= new brk_coll(2, 2, rom);
  fbrk= new brk_coll(2, 2, rom);
  displays= new cl_display_list(2, 2, "displays");
  fbrk->Duplicates= false;
  brk_counter= 0;
  stop_at_time= 0;
  make_cpu_hw();
  mk_hw_elements();
  class cl_cmdset *cs= sim->app->get_commander()->cmdset;
  build_cmdset(cs);
  irq= false;
  vcd_break= false;
  //reset();

  return 0;
  for (i= 0; i < sim->app->in_files->count; i++)
    {
      const char *fname= (const char *)(sim->app->in_files->at(i));
      long l;
      if ((l= read_hex_file(fname)) >= 0)
	{
	  /*sim->app->get_commander()->all_*/printf("%ld words read from %s\n",
						l, fname);
	}
    }
  return(0);
}

const char *
cl_uc::id_string(void)
{
  return("unknown microcontroller");
}

void
cl_uc::reset(void)
{
  class it_level *il;

  irq= false;
  instPC= PC= 0;
  state = stGO;
  ticks->set(0, 0);
  isr_ticks->set(0, 0);
  idle_ticks->set(0, 0);
  halt_ticks->set(0, 0);
  vc.inst= vc.fetch= vc.rd= vc.wr= 0;
  /*FIXME should we clear user counters?*/
  il= (class it_level *)(it_levels->top());
  while (il &&
	 il->level >= 0)
    {
      il= (class it_level *)(it_levels->pop());
      delete il;
      il= (class it_level *)(it_levels->top());
    }
  sp_most= 0;
  //sp_avg= 0;

  stack_ops->free_all();

  int i;
  for (i= 0; i < hws->count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(hws->at(i));
      hw->reset();
    }
}

void
cl_uc::reg_cell_var(class cl_memory_cell *cell,
		    void *store,
		    chars vname, chars vdesc)
{
  if (cell)
    {
      cell->init();
      if (store)
	cell->decode(store);
      if (vname.nempty())
	{
	  class cl_cvar *v;
	  vars->add(v= new cl_cvar(vname, cell, vdesc));
	  v->init();
	  v->set_by(VBY_PRE);
	}
    }
}

void
cl_uc::mk_cvar(class cl_memory_cell *cell, chars vname, chars vdesc,
	       enum var_by vby)
{
  class cl_cvar *v;
  vars->add(v= new cl_cvar(vname, cell, vdesc));
  v->init();
  v->set_by(vby);
}

void
cl_uc::mk_mvar(class cl_memory *m, t_addr a,
	       chars vname, chars vdesc,
	       enum var_by vby)
{
  class cl_cvar *v;
  vars->add(v= new cl_var(vname, m, a, vdesc));
  v->init();
  v->set_by(vby);
}


/*
 * Making elements
 */

void
cl_uc::make_memories(void)
{
}

void
cl_uc::make_variables(void)
{
  class cl_address_space *as;
  class cl_option *o= sim->app->options->get_option("var_size");
  long l, i;
  
  if (o)
    o->get_value(&l);
  else
    l= 0x100;

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  if (l > 0)
    {
      variables= as= new cl_address_space("variables", 0, l, 32);
      as->init();
      address_spaces->add(as);

      chip= new cl_chip32("variable_storage", l, 32);
      chip->init();
      memchips->add(chip);
      ad= new cl_address_decoder(variables, chip, 0, l-1, 0);
      ad->init();
      variables->decoders->add(ad);
      ad->activate(0);

      for (i= 0; i < l; i++)
	variables->set(i, 0);
    }
}

/*t_addr
cl_uc::get_mem_size(char *id)
{
  class cl_memory *m= memory(id);
  return(m?(m->get_size()):0);
}

int
cl_uc::get_mem_width(char *id)
{
  class cl_memory *m= memory(id);
  return(m?(m->width):8);
}
*/
void
cl_uc::make_cpu_hw(void)
{
  cpu= NULL;
}

void
cl_uc::mk_hw_elements(void)
{
  class cl_hw *h;

  init_add_hw(h= new cl_simulator_interface(this));
  init_add_hw(h= new cl_vcd(this, 0, "vcd"));
}

void
cl_uc::build_cmdset(class cl_cmdset *cmdset)
{
  class cl_cmd *cmd;
  class cl_super_cmd *super_cmd;
  class cl_cmdset *cset;

  cmdset->add(cmd= new cl_state_cmd("state", 0));
  cmd->init();

#ifdef STATISTIC
  cmdset->add(cmd= new cl_statistic_cmd("statistic", 0));
  cmd->init();
#endif

  cmdset->add(cmd= new cl_file_cmd("file", 0));
  cmd->init();
  cmd->add_name("load");

  cmdset->add(cmd= new cl_dl_cmd("download", 0));
  cmd->init();
  cmd->add_name("dl");

  cmdset->add(cmd= new cl_check_cmd("check", 0));
  cmd->init();

  cmdset->add(cmd= new cl_pc_cmd("pc", 0));
  cmd->init();

  cmdset->add(cmd= new cl_reset_cmd("reset", 0));
  cmd->init();

  cmdset->add(cmd= new cl_tick_cmd("tick", 0));
  cmd->init();

  cmdset->add(cmd= new cl_dump_cmd("dump", true));
  cmd->init();
  cmd->add_name("d");

  cmdset->add(cmd= new cl_dch_cmd("dch", true));
  cmd->init();

  cmdset->add(cmd= new cl_dc_cmd("dc", true));
  cmd->init();

  cmdset->add(cmd= new cl_disassemble_cmd("disassemble", true));
  cmd->init();

  cmdset->add(cmd= new cl_fill_cmd("fill", 0));
  cmd->init();

  cmdset->add(cmd= new cl_where_cmd("where", 0));
  cmd->init();

  cmdset->add(cmd= new cl_Where_cmd("Where", 0));
  cmd->init();

  cmdset->add(cmd= new cl_hole_cmd("hole", 0));
  cmd->init();
  
  cmdset->add(cmd= new cl_break_cmd("break", 0));
  cmd->init();

  cmdset->add(cmd= new cl_tbreak_cmd("tbreak", 0));
  cmd->init();

  cmdset->add(cmd= new cl_clear_cmd("clear", 0));
  cmd->init();

  cmdset->add(cmd= new cl_delete_cmd("delete", 0));
  cmd->init();

  cmdset->add(cmd= new cl_commands_cmd("commands", 0));
  cmd->init();

  cmdset->add(cmd= new cl_display_cmd("display", 0));
  cmd->init();
  
  cmdset->add(cmd= new cl_undisplay_cmd("undisplay", 0));
  cmd->init();
  
  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("get"));
    if (super_cmd)
      cset= super_cmd->commands;
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_get_sfr_cmd("sfr", 0));
    cmd->init();
    /*cset->add(cmd= new cl_get_option_cmd("option", 0));
      cmd->init();*/
    if (!super_cmd)
      {
	cmdset->add(cmd= new cl_super_cmd("get", 0, cset));
	cmd->init();
	set_get_help(cmd);
      }
  }

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("set"));
    if (super_cmd)
      cset= super_cmd->commands;
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_set_mem_cmd("memory", 0));
    cmd->add_name("bits");
    cmd->init();
    cset->add(cmd= new cl_set_hw_cmd("hardware", 0));
    cmd->add_name("hw");
    cmd->init();
    if (!super_cmd)
      {
	cmdset->add(cmd= new cl_super_cmd("set", 0, cset));
	cmd->init();
	set_set_help(cmd);
      }
  }

  { // info
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("info"));
    if (super_cmd)
      cset= super_cmd->get_subcommands();
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_info_bp_cmd("breakpoints", 0));
    cmd->add_name("bp");
    cmd->init();
    cset->add(cmd= new cl_info_reg_cmd("registers", 0));
    cmd->init();
    cset->add(cmd= new cl_info_hw_cmd("hardware", 0));
    cmd->add_name("hw");
    cmd->add_name("h");
    cmd->init();
    /*
    cset->add(cmd= new cl_info_stack_cmd("stack", 0,
    "info stack         Status of stack of the CPU",
    "long help of info stack"));
    cmd->init();
    */
    cset->add(cmd= new cl_info_memory_cmd("memory", 0));
    cmd->init();
    cset->add(cmd= new cl_info_var_cmd("variables", 0));
    cmd->init();
    cmd->add_name("vars");
    cset->add(cmd= new cl_hist_info_cmd("history", 0));
    cmd->init();
    if (!super_cmd) {
      cmdset->add(cmd= new cl_super_cmd("info", 0, cset));
      cmd->init();
      set_info_help(cmd);
    }
  }

  { // show
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("show"));
    if (super_cmd)
      cset= super_cmd->get_subcommands();
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_show_reg_cmd("registers", 0));
    cmd->init();
    if (!super_cmd) {
      cmdset->add(cmd= new cl_super_cmd("show", 0, cset));
      cmd->init();
      set_show_help(cmd);
    }
  }

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("timer"));
    if (super_cmd)
      cset= super_cmd->get_subcommands();
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_timer_add_cmd("add", 0));
    cmd->init();
    cmd->add_name("create");
    cmd->add_name("make");
    cset->add(cmd= new cl_timer_delete_cmd("delete", 0));
    cmd->init();
    cmd->add_name("remove");
    cset->add(cmd= new cl_timer_get_cmd("get", 0));
    cmd->init();
    cset->add(cmd= new cl_timer_run_cmd("run", 0));
    cmd->init();
    cmd->add_name("start");
    cset->add(cmd= new cl_timer_stop_cmd("stop", 0));
    cmd->init();
    cset->add(cmd= new cl_timer_value_cmd("set", 0));
    cmd->init();
    cmd->add_name("value");
    cset->add(cmd= new cl_timer_list_cmd("list", 0));
    cmd->init();
    if (!super_cmd) {
      cmdset->add(cmd= new cl_super_cmd("timer", 0, cset));
      cmd->init();
      set_timer_help(cmd);
    }
  }

  {
    class cl_super_cmd *mem_create, *mem_remove;
    class cl_cmdset *mem_create_cset, *mem_remove_cset;
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("memory"));
    if (super_cmd)
      cset= super_cmd->get_subcommands();
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    /*
      cset->add(cmd= new cl_memory_cmd("_no_parameters_", 0));
      cmd->init();
    */
    mem_create= (class cl_super_cmd *)cset->get_cmd("create");
    if (mem_create)
      mem_create_cset= mem_create->get_subcommands();
    else {
      mem_create_cset= new cl_cmdset();
      mem_create_cset->init();
    }
    
    mem_create_cset->add(cmd= new cl_memory_create_chip_cmd("chip", 0));
    cmd->init();

    mem_create_cset->add(cmd= new cl_memory_create_addressspace_cmd("addressspace", 0));
    cmd->init();
    cmd->add_name("addrspace");
    cmd->add_name("aspace");
    cmd->add_name("as");
    cmd->add_name("addrs");
    cmd->add_name("addr");

    mem_create_cset->add(cmd= new cl_memory_create_addressdecoder_cmd("addressdecoder", 0));
    cmd->init();
    cmd->add_name("addrdecoder");
    cmd->add_name("adecoder");
    cmd->add_name("addressdec");
    cmd->add_name("addrdec");
    cmd->add_name("adec");
    cmd->add_name("ad");

    mem_create_cset->add(cmd= new cl_memory_create_banker_cmd("banker", 0));
    cmd->init();
    cmd->add_name("bankswitcher");
    cmd->add_name("banksw");
    cmd->add_name("bsw");
    cmd->add_name("bs");

    mem_create_cset->add(cmd= new cl_memory_create_bank_cmd("bank", 0));
    cmd->init();

    mem_create_cset->add(cmd= new cl_memory_create_bander_cmd("bander", 0));
    cmd->init();
    cmd->add_name("bitbander");
    cmd->add_name("bitband");
    cmd->add_name("band");
    cmd->add_name("bb");

    if (!mem_create)
      cset->add(mem_create= new cl_super_cmd("create", 0, mem_create_cset));
    mem_create->init();
    mem_create->add_name("add");
    set_memory_create_help(mem_create);
    
    cset->add(cmd= new cl_info_memory_cmd("info", 0));
    cmd->init();
    cset->add(cmd= new cl_memory_cell_cmd("cell", 0));
    cmd->init();
    if (!super_cmd) {
      cmdset->add(cmd= new cl_super_cmd("memory", 0, cset));
      cmd->init();
      set_memory_help(cmd);
    }

    mem_remove= (class cl_super_cmd *)cset->get_cmd("remove");
    if (mem_remove)
      mem_remove_cset= mem_remove->get_subcommands();
    else {
      mem_remove_cset= new cl_cmdset();
      mem_remove_cset->init();
    }
    
    mem_remove_cset->add(cmd= new cl_memory_remove_chip_cmd("chip", 0));
    cmd->init();

    if (!mem_remove)
      cset->add(mem_remove= new cl_super_cmd("remove", 0, mem_remove_cset));
    mem_remove->init();
    mem_remove->add_name("del");
    mem_remove->add_name("rm");
    set_memory_remove_help(mem_remove);
  }

  super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("history"));
  if (super_cmd)
    cset= super_cmd->get_subcommands();
  else
    {
      cset= new cl_cmdset();
      cset->init();
      cmdset->add(cmd= new cl_super_cmd("history", 0, cset));
      cmd->init();	
    }
  cset->add(cmd= new cl_hist_cmd("_no_parameters_", 0));
  cmd->init();
  cset->add(cmd= new cl_hist_info_cmd("information", 0));
  cmd->init();
  cset->add(cmd= new cl_hist_clear_cmd("clear", 0));
  cmd->init();
  cset->add(cmd= new cl_hist_list_cmd("list", 0));
  cmd->add_name("print");
  cmd->init();
    
  cmdset->add(cmd= new cl_var_cmd("var", 0));
  cmd->init();
  cmd->add_name("variable");
  cmdset->add(cmd= new cl_rmvar_cmd("rmvar", 0));
  cmd->init();

  cmdset->add(cmd= new cl_analyze_cmd("analyse", 0));
  cmd->add_name("analyze");
  cmd->init();
}


/*
 * Read/write simulated memory
 */

t_mem
cl_uc::read_mem(const char *id, t_addr addr)
{
  class cl_address_space *m= address_space(id);

  return(m?(m->read(addr)):0);
}

t_mem
cl_uc::get_mem(const char *id, t_addr addr)
{
  class cl_address_space *m= address_space(id);

  return(m?(m->get(addr)):0);
}

void
cl_uc::write_mem(const char *id, t_addr addr, t_mem val)
{
  class cl_address_space *m= address_space(id);

  if (m)
    m->write(addr, val);
}

void
cl_uc::set_mem(const char *id, t_addr addr, t_mem val)
{
  class cl_address_space *m= address_space(id);

  if(m)
    m->set(addr, val);
}


class cl_address_space *
cl_uc::address_space(const char *id)
{
  int i;

  if (!id ||
      !(*id))
    return(0);
  for (i= 0; i < address_spaces->count; i++)
    {
      class cl_address_space *m= (cl_address_space *)(address_spaces->at(i));
      if (!m ||
	  !m->have_real_name())
	continue;
      if (m->is_inamed(id))
	return(m);
    }
  return(0);
}

class cl_address_space *
cl_uc::address_space(class cl_memory_cell *cell)
{
  return(address_space(cell, (t_addr*)NULL));
}

class cl_address_space *
cl_uc::address_space(class cl_memory_cell *cell, t_addr *addr)
{
  int i;

  for (i= 0; i < address_spaces->count; i++)
    {
      class cl_address_space *m= (cl_address_space *)(address_spaces->at(i));
      if (!m)
	continue;
      if (m->is_owned(cell, addr))
	return(m);
    }
  return(0);
}

class cl_memory *
cl_uc::memory(const char *id)
{
  int i;

  if (!id ||
      !(*id))
    return(0);
  for (i= 0; i < address_spaces->count; i++)
    {
      class cl_base *b= address_spaces->object_at(i);
      class cl_memory *m= (class cl_memory *)(b);
      if (!m ||
	  !m->have_real_name())
	continue;
      if (m->is_inamed(id))
	return(m);
    }
  for (i= 0; i < memchips->count; i++)
    {
      class cl_memory *m= (cl_memory *)(memchips->at(i));
      if (!m ||
	  !m->have_real_name())
	continue;
      if (m->is_inamed(id))
	return(m);
    }
  return(0);
}

void
cl_uc::remove_chip(class cl_memory *chip)
{
  class cl_address_space *as;
  class cl_address_decoder *ad;
  int i, j;
  t_index idx;
  i= memchips->index_of(chip, &idx);
  if (!i)
    return;
  for (i= 0; i < address_spaces->get_count(); i++)
    {
      as= (class cl_address_space *)(address_spaces->at(i));
      j= 0;
      class cl_list bankers;
      bankers.init();
      while (j < as->decoders->get_count())
	{
	  for (j= 0; j < as->decoders->get_count(); j++)
	    {
	      t_addr as_start, as_end;
	      ad= (class cl_address_decoder *)(as->decoders->at(j));
	      as_start= ad->as_begin;
	      as_end= ad->as_end;
	      if (ad->memchip == chip)
		{
		  as->undecode_area(NULL, as_start, as_end, NULL);
		  break;
		}
	      if (ad->is_banker())
		{
		  if (ad->uses_chip(chip))
		    {
		      bankers.add(ad);
		    }
		}
	    }
	}
      if (bankers.count > 0)
	{
	  int i;
	  for (i= 0; i < bankers.count; i++)
	    {
	      class cl_address_decoder *ad;
	      ad= (class cl_address_decoder *)(bankers.at(i));
	      as->decoders->disconn(ad);
	      delete ad;
	    }
	  bankers.disconn_all();
	}
    }
  memchips->disconn(chip);
  delete chip;
}


static long
ReadInt(cl_f *f, bool *ok, int bytes)
{
  char s2[3];
  long l= 0;
  int c;
  
  *ok= false;
  while (bytes)
    {
      if (f->eof())
	return(0);
      c= f->get_c();
      if ((c < 0) ||
	  (c == 0) ||
	  (c > 0xff))
	return 0;
      s2[0]= c;
      if (f->eof())
	return(0);
      c= f->get_c();
      if ((c < 0) ||
	  (c == 0) ||
	  (c > 0xff))
	return 0;
      s2[1]= c;
      s2[2]= '\0';
      l= l*256 + strtol(s2, NULL, 16);
      bytes--;
    }
  *ok= true;
  return(l);
}


/*
 * Reading intel hexa file into EROM
 *____________________________________________________________________________
 *
 * If parameter is a NULL pointer, this function reads data from `cmd_in'
 *
 */

void
cl_uc::set_rom(t_addr addr, t_mem val)
{
  t_addr size= rom->get_size();
  if (addr < size)
    {
      rom->download(addr, val);
      return;
    }
  t_addr bank, caddr;
  bank= addr / size;
  caddr= addr % size;
  class cl_banker *d= (class cl_banker *)(rom->get_decoder_of(caddr));
  if (d)
    {
      if (!d->is_banker())
	{
	  return;
	}
      d->switch_to(bank, NULL);
      rom->download(caddr, val);
      d->activate(NULL);
    }
  else
    {
    }
}

long
cl_uc::read_hex_file(const char *nam)
{
  cl_f *f;
  
  if (!nam)
    {
      fprintf(stderr, "cl_uc::read_hex_file File name not specified\n");
      return(-1);
    }
  else
    if ((f= /*fopen*/mk_io(nam, "r")) == NULL)
      {
	fprintf(stderr, "Can't open `%s': %s\n", nam, strerror(errno));
	return(-1);
      }
  long l= read_hex_file(f);
  delete f;
  return l;
}

long
cl_uc::read_hex_file(cl_console_base *con)
{
  cl_f *f;
  if (con == NULL)
    return -1;
  f= con->get_fin();
  if (f == NULL)
    return -1;
  long l= read_hex_file(f);
  return l;
}

long
cl_uc::read_hex_file(cl_f *f)
{
  int c;
  long written= 0, recnum= 0;

  uint  base= 0;  // extended address, added to every address
  uchar dnum;     // data number
  uchar rtyp=0;   // record type
  uint  addr= 0;  // address
  uchar rec[300]; // data record
  uchar sum ;     // checksum
  uchar chk ;     // check
  int  i;
  bool ok;
  int get_low= 0;
  uchar lows[4]= { 0, 0, 0, 0 };

  if (!rom)
    {
      sim->app->get_commander()->
	dd_printf("No ROM address space to read in.\n");
      return(-1);
    }

  //memset(inst_map, '\0', sizeof(inst_map));
  ok= true;
  while (ok &&
	 rtyp != 1)
    {
      while (((c= /*getc(f)*/f->get_c()) != ':') &&
	     (/*c != EOF*/!f->eof()));
      if (c != ':')
	{
	  fprintf(stderr, ": not found\n");
	  break;
	}
      recnum++;
      dnum= ReadInt(f, &ok, 1);
      chk = dnum;
      addr= ReadInt(f, &ok, 2);
      chk+= (addr & 0xff);
      chk+= ((addr >> 8) & 0xff);
      rtyp= ReadInt(f, &ok, 1);
      chk+= rtyp;
      for (i= 0; ok && (i < dnum); i++)
	{
	  rec[i]= ReadInt(f, &ok, 1);
	  chk+= rec[i];
	}
      if (ok)
	{
	  sum= ReadInt(f, &ok, 1);
	  if (ok)
	    {
	      if (((sum + chk) & 0xff) == 0)
		{
		  if (rtyp == 0)
		    {
		      if (rom->width > 8)
			addr/= 2;
		      for (i= 0; i < dnum; i++)
			{
			  if (rom->width <= 8)
			    {
			      set_rom(base+addr, rec[i]);
			      addr++;
			      written++;
			    }
			  else if (rom->width <= 16)
			    {
			      switch (get_low)
				{
				case 0: lows[0]= rec[i]; get_low++; break;
				case 1: lows[1]= rec[i];
				  set_rom(base+addr, (lows[1]*256)+lows[0]);
				  addr++;
				  written++;
				  get_low= 0;
				  break;
				}
			    }
			  else if (rom->width <= 32)
			    {
			      switch (get_low)
				{
				case 0: lows[0]= rec[i]; get_low++; break;
				case 1: lows[1]= rec[i]; get_low++; break;
				case 2: lows[2]= rec[i]; get_low++; break;
				case 3: lows[3]= rec[i];
				  set_rom(base+addr,
					  (lows[3]<<24)+
					  (lows[2]<<16)+
					  (lows[1]<<8)+
					  (lows[0]));
				  get_low= 0;
				  lows[3]= lows[2]= lows[1]= lows[0]= 0;
				  addr++;
				  written++;
				  break;
				}
			    }
			}
		    }
		  else if (rtyp == 4)
		    {
		      if (dnum >= 2)
			{
			  base= (rec[0]*256+rec[1]) << 16;
			}
		    }
		  else
		    if (rtyp != 1)
		      fprintf(stderr, "Unknown record type %d(0x%x)\n",
			      rtyp, rtyp);
		}
	      else
		fprintf(stderr, "Checksum error (%x instead of %x) in "
			"record %ld.\n", chk, sum, recnum);
	    }
	  else
	    fprintf(stderr, "Read error in record %ld.\n", recnum);
	}
    }
  if (rom->width > 8)
    {
      for (i= get_low; i<4; i++)
	lows[i]= 0;
      rom->set(addr,
	       (lows[3]<<24)+
	       (lows[2]<<16)+
	       (lows[1]<<8)+
	       (lows[0]));
    }
  
  return(written);
}

long
cl_uc::read_omf_file(cl_f *f)
{
  long written= 0;
  class cl_omf_rec rec;
  while (rec.read(f))
    {
      if (rec.type == 0x06)
	{
	  // content
	  u16_t addr= rec.pick_word(1);
	  int i= 3;
	  while (i < rec.len)
	    {
	      set_rom(addr+i, rec.rec[i]);
	      written++;
	      i++;
	    }
	}
    }
  return (written);
}

long
cl_uc::read_asc_file(cl_f *f)
{
  int c;
  chars line= chars();
  bool in;
  t_addr addr= 0;
  
  in= true;
  while ((c= f->get_c()) &&
	 (!f->eof()))
    {
      if (in)
	{
	  if ((c=='\n') || (c=='\r'))
	    {
	      in= false;
	      {
		chars word= chars();
		const char *s;
		// process
		line.trim();
		line.start_parse();
		word= line.token(" ");
		s= word.c_str();
		if (isxdigit(*s))
		  {
		    t_mem d= chars(s).htoi();//strtoll(s, 0, 16);
		    set_rom(addr, d);
		    addr++;
		  }
		line= "";
	      }
	    }
	  else
	    line.append(c);
	}
      else // out
	{
	  if ((c=='\n') || (c=='\r'))
	    ;
	  else
	    {
	      in= true;
	      line.append(c);
	    }
	}
    }
  return addr;
}

long
cl_uc::read_p2h_file(cl_f *f, bool just_check)
{
  chars line= chars();
  int c;
  long nr= 0;
  while ((c= f->get_c()) &&
	 (!f->eof()))
    {
      if ((c=='\n') || (c=='\r'))
	{
	  line.trim();
	  line.start_parse();
	  chars w1= line.token(" \t");
	  chars w2= line.token(" \t");
	  chars w3= line.token(" \t");
	  if (w1.nempty() && w2.nempty() && w3.nempty())
	    {
	      if ((w2 == "//C") || (w2 == "//I"))
		{
		  t_mem v= w1.htoi();//strtol(w1.c_str(), 0, 16);
		  t_addr a= w3.htoi();//strtol(w3.c_str(), 0, 16);
		  if (just_check)
		    {
		      t_mem mv= rom->read(a);
		      if (mv != v)
			{
			  application->dd_printf("Diff at %08x, FILE=%08x MEM=%08x\n",
						 AU32(a), MU32(v), MU32(mv));
			}
		    }
		  else
		    set_rom(a, v);
		  nr++;		  
		}
	    }
	  line= "";
	}
      else
	line.append(c);
    }
  return nr;
}

long
cl_uc::read_cdb_file(cl_f *f)
{
  class cl_cdb_recs *fns= new cl_cdb_recs();
  chars ln;
  const char *lc;
  long cnt= 0;
  class cl_cdb_rec *r;

  ln= f->get_s();
  while (!ln.empty())
    {
      lc= ln.c_str();
      if (lc[0] == 'F')
	{
	  if (ln.len() > 5)
	    {
	      if ((lc[1] == ':') &&
		  (lc[2] == 'G'))
		{
		  ln.start_parse(4);
		  chars n= ln.token("$");
		  if ((r= fns->rec(n)) != NULL)
		    {
		      class cl_cvar *v;
		      v= vars->add(n, rom, r->addr, "");
		      v->set_by(VBY_DEBUG);
		      fns->del(n);
		      cnt++;
		    }
		  else
		    fns->add(new cl_cdb_rec(n));
		}
	    }
	}
      else if (lc[0] == 'L')
	{
	  if (ln.len() > 5)
	    {
	      if ((lc[1] == ':') &&
		  (lc[2] == 'G'))
		{
		  ln.start_parse(4);
		  chars n= ln.token("$");
		  chars t= ln.token(":");
		  t= ln.token(" ");
		  t_addr a= t.htoi();//strtol(t.c_str(), 0, 16);
		  if ((r= fns->rec(n)) != NULL)
		    {
		      fns->del(n);
		      class cl_cvar *v;
		      v= vars->add(n, rom, a, "");
		      v->set_by(VBY_DEBUG);
		      cnt++;
		    }
		  else
		    fns->add(new cl_cdb_rec(n, a));
		}
	    }
	}
      ln= f->get_s();
    }
  fns->free_all();
  delete fns;
  return cnt;
}

static bool is_area(chars w1, chars w2, chars w3)
{
  if (w1.empty() || w2.empty() || w3.empty())
    return false;
  if (w1 != "Area")
    return false;
  if (w2 != "Addr")
    return false;
  if (w3 != "Size")
    return false;
  return true;
}

static bool is_code_seg(chars wlast)
{
  return wlast == "(REL,CON,CODE)";
}

static bool is_ncode_seg(chars w1, chars wlast)
{
  return (wlast == "(REL,CON)")
    &&
    (
     (w1 == "CODE") ||
     (w1 == "_CODE")
     );
}

static bool is_addr(chars w)
{
  int l= w.len();
  if (l != 8) return false;
  const char *s= w.c_str();
  int i;
  for (i= 0; i<l; i++)
    if (!isxdigit(s[i])) return false;
  return true;
}

long
cl_uc::read_map_file(cl_f *f)
{
  int cnt= 0;
  chars ln;
  chars s, w1, w2, w3, wlast, w;
  enum { s_wait_area, s_wait_seginfo, s_wait_sym, s_wait_nsym } stat;
  class cl_cvar *v;
  
  stat= s_wait_area;
  ln= f->get_s();
  while (ln.nempty())
    {
      ln= f->get_s();
      s= ln.c_str();
      ln.start_parse();
      w1= ln.token(" \t\v");
      if (w1.nempty()) wlast= w1;
      w2= ln.token(" \t\v");
      if (w2.nempty()) wlast= w2;
      w3= ln.token(" \t\v");
      if (w3.nempty()) wlast= w3;
      w= ln.token(" \t\v");
      while (w.nempty())
	{
	  wlast= w;
	  w= ln.token(" \t\v");
	}
      switch (stat)
	{
	case s_wait_area:
	  if (is_area(w1, w2, w3))
	    stat= s_wait_seginfo;
	  break;
	case s_wait_seginfo:
	  if (is_code_seg(wlast))
	    stat= s_wait_sym;
	  else if (is_ncode_seg(w1, wlast))
	    stat= s_wait_nsym;
	  break;
	case s_wait_sym:
	  if (is_area(w1, w2, w3))
	    stat= s_wait_seginfo;
	  else
	    {
	      if ((w1 == "C:") && is_addr(w2) && (w3.c(0) == '_'))
		{
		  w3= &(w3.c_str()[1]);
		  t_addr a= w2.htoi();//strtol(w2.c_str(), NULL, 16);
		  //printf("%s %08x\n", w3.c_str(), AU(a));
		  v= vars->add(w3, rom, a, "");
		  v->set_by(VBY_DEBUG);
		  cnt++;
		}
	    }
	  break;
	case s_wait_nsym:
	  if (is_area(w1, w2, w3))
	    stat= s_wait_seginfo;
	  else
	    {
	      if (is_addr(w1) && (w2.c(0) == '_'))
		{
		  w2= &(w2.c_str()[1]);
		  t_addr a= w1.htoi();//strtol(w1.c_str(), NULL, 16);
		  //printf("%s %08x\n", w2.c_str(), AU(a));
		  v= vars->add(w2, rom, a, "");
		  v->set_by(VBY_DEBUG);
		  cnt++;
		}
	    }
	  break;
	}
    }
  return cnt;
}

static int h1(const char *s, int p)
{
  int r= 0;
  if (isxdigit(s[p]))
    {
      if (s[p]>='a')
	r= 10+(s[p]-'a');
      else if (s[p]>='A')
	r= 10+(s[p]-'A');
      else
	r= s[p]-'0';
    }
  return r;
}

static int h2(const char *s, int p)
{
  int r= 0;
  if (isxdigit(s[p]) && isxdigit(s[p+1]))
    {
      r= 16 * h1(s, p);
      r+= h1(s, p+1);
    }
  return r;
}

static t_addr a16(const char *s, int p)
{
  t_addr a;
  a= h2(s, p);
  a<<= 8;
  a+= h2(s, p+2);
  return a;
}

static t_addr a24(const char *s, int p)
{
  t_addr a;
  a= h2(s, p);
  a<<= 8;
  a+= h2(s, p+2);
  a<<= 8;
  a+= h2(s, p+4);
  return a;
}

static t_addr a32(const char *s, int p)
{
  t_addr a;
  a= h2(s, p);
  a<<= 8;
  a+= h2(s, p+2);
  a<<= 8;
  a+= h2(s, p+4);
  a<<= 8;
  a+= h2(s, p+6);
  return a;
}

static int s19(class cl_uc *uc, t_addr a, const char *s, int p, int data_bytes)
{
  int b= 0;
  if (data_bytes < 1)
    return 0;
  while (b < data_bytes)
    {
      u8_t d= h2(s, p);
      uc->set_rom(a, d);
      a++;
      p+= 2;
      b++;
    }
  return data_bytes;
}

long
cl_uc::read_s19_file(cl_f *f)
{
  chars line;
  int cnt;
  long int written= 0;
  while (!f->eof())
    {
      line= f->get_s();
      if (line.empty())
	continue;
      const char *s= line.c_str();
      if (s[0] != 'S')
	continue;
      if (line.length() < 4)
	continue;
      cnt= h2(s, 2);
      if (line.length() < 4+cnt*2)
	continue;
      int b= 0, sum= 0, p= 2;
      while (b<cnt)
	{
	  if (s[p] && s[p+1])
	    sum+= h2(s, p);
	  b++;
	  p+= 2;
	}
      if (b!=cnt)
	continue;
      int chk= h2(s, 2+cnt*2);
      sum&= 0xff;
      sum^= 0xff;
      if (sum != chk)
	continue;
      t_addr a;
      switch (s[1])
	{
	case '0': break;
	case '1':
	  a= a16(s, 4);
	  p=  8;
	  written+= s19(this, a, s, p, cnt-2-1);
	  break;
	case '2':
	  a= a24(s, 4);
	  p= 10;
	  written+= s19(this, a, s, p, cnt-3-1);
	  break;
	case '3':
	  a= a32(s, 4);
	  p= 12;
	  written+= s19(this, a, s, p, cnt-4-1);
	  break;
	case '4': break;
	case '5': break;
	case '6': break;
	case '7': break;
	case '8': break;
	case '9': break;
	}
    }
  return written;
}

cl_f *
cl_uc::find_loadable_file(chars nam)
{
  cl_f *f;
  bool o;
  chars c;
  
  f= mk_io(nam, "r");
  o= (f->opened());
  if (o)
    return f;

  c= chars("", "%s.p2h", nam.c_str());
  f->open(c, "r");
  o= (f->opened());
  if (o)
    return f;
  c= chars("", "%s.asc", nam.c_str());
  f->open(c, "r");
  o= (f->opened());
  if (o)
    return f;
  c= chars("", "%s.ihx", nam.c_str());
  f->open(c, "r");
  o= (f->opened());
  if (o)
    return f;
  c= chars("", "%s.hex", nam.c_str());
  f->open(c, "r");
  o= (f->opened());
  if (o)
    return f;
  c= chars("", "%s.ihex", nam.c_str());
  f->open(c, "r");
  o= (f->opened());
  if (o)
    return f;
  c= chars("", "%s.s19", nam.c_str());
  f->open(c, "r");
  o= (f->opened());
  if (o)
    return f;

  c= chars("", "%s.omf", nam.c_str());
  f->open(c, "r");
  o= (f->opened());
  if (o)
    return f;

  delete f;
  return NULL;
}

long
cl_uc::read_file(chars nam, class cl_console_base *con, bool just_check)
{
  cl_f *f= find_loadable_file(nam);
  long l= 0;
  
  if (!f)
    {
      if (con)
	con->dd_printf("no loadable file found (%s)\n", nam.c_str());
      else
	printf("no loadable file found (%s)\n", nam.c_str());
      return 0;
    }
  if (!application->quiet)
    printf("Loading from %s\n", f->get_file_name());
  if (is_p2h_file(f))
    {
      l= read_p2h_file(f, just_check);
      if (!application->quiet)
	printf("%ld words read from %s\n", l, f->get_fname());
    }
  if (is_asc_file(f))
    {
      l= read_asc_file(f);
      if (!application->quiet)
	printf("%ld words read from %s\n", l, f->get_fname());
    }
  if (is_hex_file(f))
    {
      l= read_hex_file(f);
      if (!application->quiet)
	printf("%ld words read from %s\n", l, f->get_fname());
    }
  else if (is_s19_file(f))
    {
      l= read_s19_file(f);
      if (!application->quiet)
	printf("%ld words read from %s\n", l, f->get_fname());
    }
  else if (is_omf_file(f))
    {
      l= read_omf_file(f);
      if (!application->quiet)
	printf("%ld words read from %s\n", l, f->get_fname());
    }
  else if (is_cdb_file(f))
    {
      l= read_cdb_file(f);
      if (!application->quiet)
	printf("%ld symbols read from %s\n", l, f->get_fname());
    }
  else if (is_map_file(f))
    {
      l= read_map_file(f);
      if (!application->quiet)
	printf("%ld symbols read from %s\n", l, f->get_fname());
    }
  if (strcmp(nam, f->get_fname()) != 0)
    {
      chars n= nam;
      n+= ".cdb";
      cl_f *c= mk_io(n, "r");
      if (c->opened())
	{
	  l= read_cdb_file(c);
	  if (!application->quiet)
	    printf("%ld symbols read from %s\n", l, c->get_fname());
	}
      delete c;
    }
  if (strcmp(nam, f->get_fname()) != 0)
    {
      chars n= nam;
      n+= ".map";
      cl_f *c= mk_io(n, "r");
      if (c->opened())
	{
	  l= read_map_file(c);
	  if (!application->quiet)
	    printf("%ld symbols read from %s\n", l, c->get_fname());
	}
      delete c;
    }
  delete f;

  if (!just_check)
    analyze_init();
  return l;
}


void
cl_uc::set_analyzer(bool val)
{
  analyzer= val;
  if (rom)
    analyze_init();
}

void
cl_uc::analyze_init(void)
{
  // Forget everything we knew previously.
  for (t_addr addr = rom->get_start_address(); addr < rom->highest_valid_address(); addr++)
    del_inst_at(addr);

  t_index i = 0;
  while (i < vars->by_name.count)
    {
      class cl_cvar *v = vars->by_name.at(i);
      if (*(v->get_name()) == '.')
        vars->del(v->get_name());
      else
        i++;
    }

  if (analyzer)
    analyze_start();
}

void
cl_uc::analyze_start(void)
{
  class cl_var *v = new cl_var(".reset", rom, reset_addr(),
			       chars("Auto-generated by analyze"), -1, -1);
  v->init();
  vars->add(v);
  v->set_by(VBY_ANALYZE);
  
  analyze(0);
}

void
cl_uc::analyze(t_addr addr)
{
  set_inst_at(addr);

  // If we jumped we should make sure its labeled. However we don't know if the
  // target has a valid instruction and only a microprocessor specific analyze
  // implementation can follow the execution path. So we tell a white lie.
  bool was_inst = inst_at(PC);
  set_inst_at(PC);

  if (PC != addr + inst_length(addr))
    analyze_jump(addr, PC, 'j');

  if (!was_inst)
    del_inst_at(PC);
}

void
cl_uc::analyze_jump(t_addr addr, t_addr target, char type, unsigned int bit)
{
  // If the target isn't already labeled we'll create one ourselves.
  t_index var_i;
  if (!vars->by_addr.search(rom, target, -1, -1, var_i) &&
      !vars->by_addr.search(rom, target, rom->width, 0, var_i))
    {
      const char *var_name = "";
      const char *suffix = "";

      switch (type)
        {
          case 's': // subroutine call
            var_name = "func";
            break;

          case 't':
          case 'f':
            suffix = (type == 't' ? "_isset" : "_unset");
            switch (bit)
              {
                default:
                case 0: var_name= "bit0"; break;
                case 1: var_name= "bit1"; break;
                case 2: var_name= "bit2"; break;
                case 3: var_name= "bit3"; break;
                case 4: var_name= "bit4"; break;
                case 5: var_name= "bit5"; break;
                case 6: var_name= "bit6"; break;
                case 7: var_name= "bit7"; break;
              }
            break;

          default:
            var_name = (target <= addr ? "loop" : "label");
            break;
        }

      chars label("", ".%s%s$%u", var_name, suffix, label_index++);
      class cl_var *v = new cl_var(label, rom, target, chars("Auto-generated by analyze"), -1, -1);
      v->init();
      vars->add(v);
      v->set_by(VBY_ANALYZE);
    }

  // If we didn't know the target was code we do now, but don't cross
  // into bankers - we don't know what bank would be selected at
  // execution time.
  class cl_address_decoder *ad;
  if (!inst_at(target) && (ad = rom->get_decoder_of(target)) && !ad->is_banker())
    analyze(target);
}

/*
 * Handling instruction map
 *
 * `inst_at' is checking if the specified address is in instruction
 * map and `set_inst_at' marks the address in the map and
 * `del_inst_at' deletes the mark. `there_is_inst' checks if there is
 * any mark in the map
 */

bool
cl_uc::inst_at(t_addr addr)
{
  if (!rom)
    return(0);
  return(rom->get_cell_flag(addr, CELL_INST));
}

void
cl_uc::set_inst_at(t_addr addr)
{
  if (rom)
    rom->set_cell_flag(addr, true, CELL_INST);
}

void
cl_uc::del_inst_at(t_addr addr)
{
  if (rom)
    rom->set_cell_flag(addr, false, CELL_INST);
}

bool
cl_uc::there_is_inst(void)
{
  if (!rom)
    return(0);
  bool got= false;
  t_addr addr;
  for (addr= 0; rom->valid_address(addr) && !got; addr++)
    got= rom->get_cell_flag(addr, CELL_INST);
  return(got);
}


/*
 * Manipulating HW elements of the CPU
 *****************************************************************************
 */

/* Register callback hw objects for mem read/write */

/*void
cl_uc::register_hw_read(enum mem_class type, t_addr addr, class cl_hw *hw)
{
  class cl_m *m;
  class cl_memloc *l;

  if ((m= (class cl_m*)mems->at(type)))
    {
      if ((l= m->read_locs->get_loc(addr)) == 0)
	{
	  l= new cl_memloc(addr);
	  l->init();
	  m->read_locs->add(l);
	}
      l->hws->add(hw);
    }
  else
    printf("cl_uc::register_hw_read TROUBLE\n");
}*/

/*void
cl_uc::register_hw_write(enum mem_class type, t_addr addr, class cl_hw *hw)
{
}*/

void
cl_uc::add_hw(class cl_hw *hw)
{
  int i;
  if (!hw)
    return;
  for (i= 0; i < hws->count; i++)
    {
      class cl_hw *h= (class cl_hw *)(hws->at(i));
      h->new_hw_adding(hw);
    }
  hws->add(hw);
  for (i= 0; i < hws->count; i++)
    {
      class cl_hw *h= (class cl_hw *)(hws->at(i));
      if (h != hw)
	h->new_hw_added(hw);
    }  
}

void
cl_uc::init_add_hw(class cl_hw *hw)
{
  if (hw)
    {
      hw->init();
      add_hw(hw);
    }
}

int
cl_uc::nuof_hws(void)
{
  return hws->count;
}

/* Looking for a specific HW element */

class cl_hw *
cl_uc::get_hw(int idx)
{
  if (idx >= hws->count)
    return NULL;
  return (class cl_hw *)(hws->at(idx));
}

class cl_hw *
cl_uc::get_hw(enum hw_cath cath, int *idx)
{
  class cl_hw *hw= 0;
  int i= 0;

  if (idx)
    i= *idx;
  for (; i < hws->count; i++)
    {
      hw= (class cl_hw *)(hws->at(i));
      if (hw->category == cath)
	break;
    }
  if (i >= hws->count)
    return(0);
  if (idx)
    *idx= i;
  return(hw);
}

class cl_hw *
cl_uc::get_hw(const char *id_string, int *idx)
{
  class cl_hw *hw= 0;
  int i= 0;

  if (idx)
    i= *idx;
  if (strcmp(id_string, "cpu") == 0)
    return cpu;
  for (; i < hws->count; i++)
    {
      hw= (class cl_hw *)(hws->at(i));
      if (strstr(hw->id_string, id_string) == hw->id_string)
	break;
    }
  if (i >= hws->count)
    return(0);
  if (idx)
    *idx= i;
  return(hw);
}

class cl_hw *
cl_uc::get_hw(enum hw_cath cath, int hwid, int *idx)
{
  class cl_hw *hw;
  int i= 0;

  if (idx)
    i= *idx;
  hw= get_hw(cath, &i);
  while (hw &&
	 hw->id != hwid)
    {
      i++;
      hw= get_hw(cath, &i);
    }
  if (hw &&
      idx)
    *idx= i;
  return(hw);
}

class cl_hw *
cl_uc::get_hw(const char *id_string, int hwid, int *idx)
{
  class cl_hw *hw;
  int i= 0;

  if (idx)
    i= *idx;
  hw= get_hw(id_string, &i);
  while (hw &&
	 hw->id != hwid)
    {
      i++;
      hw= get_hw(id_string, &i);
    }
  if (hw &&
      idx)
    *idx= i;
  return(hw);
}

int
cl_uc::get_max_hw_id(enum hw_cath cath)
{
  class cl_hw *hw;
  int i, max= -1;

  for (i= 0; i < hws->count; i++)
    {
      hw= (class cl_hw *)(hws->at(i));
      if (hw->id > max)
	max= hw->id;
    }
  return max;
}

/*
 * Help of the command interpreter
 */

struct dis_entry *
cl_uc::dis_tbl(void)
{
  static struct dis_entry empty= { 0, 0, 0, 0, NULL };
  return(&empty);
}

char *
cl_uc::disass(t_addr addr)
{
  return strdup("uc::disass() unimplemented\n");
}

int
cl_uc::print_disass(t_addr addr, class cl_console_base *con, bool nl)
{
  char *dis;
  chars cdis, comment;
  class cl_brk *b;
  int i, l, len= 0;
  class cl_option *o= sim->app->options->get_option("black_and_white");
  bool bw= false;
  if (o) o->get_value(&bw);
  
  if (!rom)
    return 0;

  cl_vars_iterator vi(vars);
  const class cl_var *var = NULL;
  if ((var = vi.first(rom, addr)))
    {
      len+= con->dd_printf("\n");

      do {
        len+= con->dd_cprintf("dump_address", rom->addr_format, addr);
        len+= con->dd_cprintf("answer", "   ");
        len+= con->dd_cprintf("dump_label", " <%s>:\n", var->get_name());
      } while ((var = vi.next()));
    }

  b= fbrk_at(addr);
  dis= disassc(addr, &comment);
  cdis= dis;
  len+= con->dd_cprintf("dump_address", rom->addr_format, addr);
  len+= con->dd_cprintf("answer", " %c", (b ? (b->perm == brkFIX ? 'F' : 'D') : ' '));
  len+= con->dd_cprintf("answer", "%c", inst_at(addr)?' ':'?');
  l= inst_length(addr);
  for (i= 0; i < l; i++)
    {
      len+= con->dd_printf(" ");
      len+= con->dd_cprintf("dump_number", rom->data_format, rom->get(addr+i));
    }
  int padding= (longest_inst() - i) * ((rom->width + 3) / 4 + 1);
  len+= con->dd_printf("%*.*s", padding, padding, "");
  if (comment.nempty())
    while (cdis.len() < 25) cdis.append(' ');
  len+= con->dd_cprintf("dump_char", " %s", cdis.c_str());
  if (comment.nempty())
    len+= con->dd_cprintf("comment", " %s", comment.c_str());
  if (nl)
    {
      if (!bw)
	{
	  con->dd_printf("\033[0K");
	}
      else
	{
	  while (++len < 70) con->dd_printf(" ");
	}
      con->dd_printf("\n");
    }
  free((char *)dis);
  return len;
}

int
cl_uc::print_disass(t_addr addr, class cl_console_base *con)
{
  return print_disass(addr, con, true);
}

void
cl_uc::print_regs(class cl_console_base *con)
{
  con->dd_printf("No registers\n");
}

int
cl_uc::inst_length(t_addr addr)
{
  struct dis_entry *tabl= dis_tbl();
  int i;
  t_mem code;

  if (!rom)
    return(0);

  code = rom->get(addr);
  for (i= 0; tabl[i].mnemonic && (code & tabl[i].mask) != tabl[i].code; i++) ;
  return(tabl[i].mnemonic?tabl[i].length:1);
}

int
cl_uc::inst_branch(t_addr addr)
{
  struct dis_entry *tabl= dis_tbl();
  int i;
  t_mem code;

  if (!rom)
    return(0);

  code = rom->get(addr);
  for (i= 0; tabl[i].mnemonic && (code & tabl[i].mask) != tabl[i].code; i++)
    ;
  return tabl[i].branch;
}

bool
cl_uc::is_call(t_addr addr)
{
  struct dis_entry *tabl= dis_tbl();
  int i;
  t_mem code;

  if (!rom)
    return(0);

  code = rom->get(addr);
  for (i= 0; tabl[i].mnemonic && (code & tabl[i].mask) != tabl[i].code; i++)
    ;
  return tabl[i].is_call;
}

int
cl_uc::longest_inst(void)
{
  struct dis_entry *de= dis_tbl();
  int max= 0;

  while (de &&
	 de->mnemonic)
    {
      if (de->length > max)
	max= de->length;
      de++;
    }
  return(max);
}

const class cl_var *
cl_uc::addr_name(t_addr addr,
		 class cl_memory *mem,
		 int bitnr_high,
		 int bitnr_low,
		 chars *buf,
		 const class cl_var *context)
{
  t_index i;
  const cl_var *var = NULL;

  if (!mem)
    return NULL;

  if (vars->by_addr.search(mem, addr, bitnr_high, bitnr_low, i))
    var = vars->by_addr.at(i);
  else if (vars->by_addr.search(mem, addr, mem->width - 1, 0, i))
    var = vars->by_addr.at(i);
  else if (bitnr_high >= 0 && vars->by_addr.search(mem, addr, -1, -1, i))
    var = vars->by_addr.at(i);
  else if (mem->is_address_space())
    {
      cl_address_decoder *ad = ((cl_address_space *)mem)->get_decoder_of(addr);
      if (ad)
        {
          class cl_memory *chip = ad->memchip;
	  if (chip)
	    {
	      addr = ad->as_to_chip(addr);
	      
	      if (vars->by_addr.search(chip, addr, bitnr_high, bitnr_low, i))
		var = vars->by_addr.at(i);
	      else if (vars->by_addr.search(chip, addr, chip->width - 1, 0, i))
		var = vars->by_addr.at(i);
	      else if (bitnr_high >= 0 && vars->by_addr.search(chip, addr, -1, -1, i))
		var = vars->by_addr.at(i);
	    }
	  else if (vars->by_addr.search(mem, addr, -1, -1, i))
	    var = vars->by_addr.at(i);
	}
    }

  if (var)
    {
      const char *name = var->get_name();

      // If there is a context var and its name prefixes the var for this
      // addr we strip the prefix off.
      size_t len= 0;
      if (context && (len = strlen(context->get_name())) &&
          !strncmp(name, context->get_name(), len) &&
          (name[len] == '\0' || name[len] == '_'))
        {
          if (name[len] == '\0')
            {
              // Same as context, nothing more to add
              return var;
            }
          else if (name[len] == '_')
            {
              // We don't need the prefix - we already had the context
              if (buf) buf->appendf(" <%s", &name[len + 1]);
            }
        }
      else
        {
          // It's all significant, nothing to do with context
          if (buf) buf->appendf(" <%s", name);
        }

      if (bitnr_high >= 0 &&
          (var->bitnr_high != bitnr_high || var->bitnr_low != bitnr_low))
        {
          if (bitnr_high == bitnr_low)
            {
	      if (buf) buf->appendf(".%d", bitnr_high);
	    }
          else
            {
	      if (buf) buf->appendf("[%d:%d]", bitnr_high, bitnr_low);
	    }
        }

      if (buf) buf->appendf(">");
    }

  return var;
}

bool
cl_uc::symbol2address(char *sym,
		      class cl_memory **mem,
		      t_addr *addr)
{
  t_index i;

  if (!sym ||
      !*sym)
    return false;
  if (vars->by_name.search(sym, i))
    {
      class cl_cvar *v= vars->by_name.at(i);
      /*if (v->bitnr_low >= 0)
	return false;*/
      if (!v->is_mem_var())
	return false;
      if (mem)
	*mem= v->get_mem();
      if (addr)
	*addr= v->get_addr();
      return true;
    }
  return false;
}

bool
cl_uc::symbol2cell(char *sym,
		   class cl_memory_cell **cell)
{
  t_index i;

  if (!sym ||
      !*sym)
    return false;
  if (vars->by_name.search(sym, i))
    {
      class cl_cvar *v= vars->by_name.at(i);
      class cl_memory_cell *c= v->get_cell();
      if (c)
	{
	  if (cell)
	    *cell= c;
	  return true;
	}
    }
  return false;
}

class cl_cvar *
cl_uc::get_var(chars name)
{
  t_index i;
  if (vars->by_name.search(name.c_str(), i))
    return vars->by_name.at(i);
  return NULL;
}


/*
 * Searching for a name in the specified table
 */

struct name_entry *
cl_uc::get_name_entry(struct name_entry tabl[], char *name)
{
  int i= 0;
  char *p;

  if (!tabl ||
      !name ||
      !(*name))
    return(0);
  for (p= name; *p; *p= toupper(*p), p++);
  while (tabl[i].name &&
	 (!(tabl[i].cpu_type & type->type) ||
	 (strcmp(tabl[i].name, name) != 0)))
    {
      i++;
    }
  if (tabl[i].name != NULL)
    return(&tabl[i]);
  else
    return(0);
}

chars
cl_uc::cell_name(class cl_memory_cell *cell, int bitnr_high, int bitnr_low)
{
  class cl_address_space *as;
  t_addr addr;
  int i;

  if (!cell || !(as = address_space(cell, &addr)))
    return chars("");

  if (vars->by_addr.search(as, addr, bitnr_high, bitnr_low, i))
    return chars(vars->by_addr.at(i)->get_name());

  if (bitnr_high != - 1 && vars->by_addr.search(as, addr, -1, -1, i))
    {
      if (bitnr_high != bitnr_low)
        return chars("", "%s[%d:%d]", vars->by_addr.at(i)->get_name(), bitnr_high, bitnr_low);
      else
        return chars("", "%s.%d", vars->by_addr.at(i)->get_name(), bitnr_low);
    }

  if (bitnr_high == -1)
    return chars("", "%s_%06x", as->get_name(), addr);
  else if (bitnr_high != bitnr_low)
    return chars("", "%s_%06x[%d:%d]", as->get_name(), addr, bitnr_high, bitnr_low);
  else
    return chars("", "%s_%06x.%d", as->get_name(), addr, bitnr_high);
}

t_addr
cl_uc::read_addr(class cl_memory *m, t_addr start_addr)
{
  if (!m) return 0;
  // 16 bit little endian, by default
  return m->read(start_addr) + 256*m->read(start_addr+1);
}

/*
 * Messages to broadcast
 */

bool
cl_uc::handle_event(class cl_event &event)
{
  switch (event.what)
    {
    case ev_address_space_added:
      {
	//try {
	  class cl_event_address_space_added &e=
	    (class cl_event_address_space_added &)(event);
	  address_space_added(e.as);
	  e.handle();
	  //}
	  //catch (...)
	  //{ break; }
	break;
      }
    default:
      return(pass_event_down(event));
      break;
    }
  return(false);
}

/*
void
cl_uc::mem_cell_changed(class cl_address_space *mem, t_addr addr)
{
  if (hws)
    hws->mem_cell_changed(mem, addr);
  else
    printf("JAJ uc\n");//FIXME
  if (mems &&
      mems->count)
    {
      int i;
      for (i= 0; i < mems->count; i++)
	{
	}
    }
}
*/

void
cl_uc::address_space_added(class cl_address_space *as)
{
  /*
  if (hws)
    hws->address_space_added(as);
  else
    printf("JAJ uc\n");//FIXME
  */
}


/*
 * Error handling
 */

void
cl_uc::error(class cl_error *error)
{
  errors->add(error);
  if ((error->inst= inst_exec))
    error->PC= instPC;
}

void
cl_uc::check_errors(void)
{
  int i;
  class cl_commander_base *c= sim->app->get_commander();
  bool must_stop= false;

  if (c)
    {
      for (i= 0; i < errors->count; i++)
	{
	  class cl_error *error= (class cl_error *)(errors->at(i));
	  if (!error->is_on())
	    continue;
	  error->print(c);
	  must_stop= must_stop || (error->get_type() & err_stop);
	  if (error->inst)
	    {
	      class cl_console_base *con;
	      con= c->actual_console;
	      if (!con)
		con= c->frozen_or_actual();
	      if (con)
		{
		  con->dd_printf("Erroneous instruction: ");
		  print_disass(error->PC, con);
		}
	    }
	}
      errors->free_all();
    }
  else
    fprintf(stderr, "no actual console, %d errors\n", errors->count);
  if (must_stop)
    sim->stop(resERROR);
}


/*
 * Converting bit address into real memory
 */

class cl_address_space *
cl_uc::bit2mem(t_addr bitaddr, t_addr *memaddr, int *bitnr_high, int *bitnr_low)
{
  if (memaddr)
    *memaddr= bitaddr;

  return rom;
}


/*
 * Execution
 */

int
cl_uc::tick_hw(int cycles)
{
  class cl_hw *hw;
  int c= cycles;
  int i;//, cpc= clock_per_cycle();

  // tick hws
  while (c-- > 0)
    {
      int hc= hws->count;
      for (i= 0; i < hc; i++)
        {
          hw= (class cl_hw *)(hws->at(i));
          if ((hw->flags & HWF_INSIDE) &&
              (hw->on))
	    {
	      hw->tick(1);
	    }
        }
    }
  do_extra_hw(cycles);
  return(0);
}

void
cl_uc::do_extra_hw(int cycles)
{}

int
cl_uc::tick(int cycles)
{
  //class cl_hw *hw;
  int i, clocks= cycles * clock_per_cycle();
  double time = clocks * xtal_tick;

  // increase time
  class it_level *il= (class it_level *)(it_levels->top());
  for (i= 0; i < counters->count; i++)
    {
      class cl_ticker *t= (class cl_ticker *)(counters->at(i));
      if (t && t->run)
        {
          if (t->get_type() == TICK_ANY ||
              (il->level >= 0 && (t->get_type() == TICK_INISR)) ||
              (state == stIDLE && (t->get_type() == TICK_IDLE)) ||
              (state == stPD && (t->get_type() == TICK_HALT)))
            t->tick(clocks, time);
        }
    }

  //tick_hw(cycles);
  inst_ticks+= cycles;
  return(0);
}

int
cl_uc::tickt(t_mem code)
{
  i8_t *tt= tick_tab(code);
  if (tt == NULL)
    return tick(1);
  int t= tt[code];
  if (t)
    return tick(t);
  return 0;
}

class cl_ticker *
cl_uc::get_counter(int nr)
{
  if (nr >= counters->count)
    return(0);
  return((class cl_ticker *)(counters->at(nr)));
}

class cl_ticker *
cl_uc::get_counter(const char *nam)
{
  int i;

  if (!nam)
    return(0);
  for (i= 0; i < counters->count; i++)
    {
      class cl_ticker *t= (class cl_ticker *)(counters->at(i));
      if (t &&
	  t->get_name() &&
	  strcmp(t->get_name(), nam) == 0)
	return(t);
    }
  return(0);
}

void
cl_uc::add_counter(class cl_ticker *ticker, int nr)
{
  while (counters->count <= nr)
    counters->add(0);
  counters->put_at(nr, ticker);
}

void
cl_uc::add_counter(class cl_ticker *ticker, const char */*nam*/)
{
  int i;

  if (counters->count < 1)
    counters->add(0);
  for (i= 1; i < counters->count; i++)
    {
      class cl_ticker *t= (class cl_ticker *)(counters->at(i));
      if (!t)
	{
	  counters->put_at(i, ticker);
	  return;
	}
    }
  counters->add(ticker);
}

void
cl_uc::del_counter(int nr)
{
  class cl_ticker *t;

  if (nr >= counters->count)
    return;
  if ((t= (class cl_ticker *)(counters->at(0))) != 0)
    delete t;
  counters->put_at(nr, 0);
}

void
cl_uc::del_counter(const char *nam)
{
  int i;

  if (!nam)
    return;
  for (i= 0; i < counters->count; i++)
    {
      class cl_ticker *t= (class cl_ticker *)(counters->at(i));
      if (t &&
	  t->get_name() &&
	  strcmp(t->get_name(), nam) == 0)
	{
	  delete t;
	  counters->put_at(i, 0);
	  return;
	}
    }
}

/*
 * Fetch without checking for breakpoint hit
 */

t_mem
cl_uc::fetch(void)
{
  ulong code;

  if (!rom)
    return(0);

  code= rom->read(PC);
  set_PC((PC+1)&PCmask);
  vc.fetch++;
  return(code);
}

/*
 * Fetch but checking for breakpoint hit first, returns TRUE if
 * a breakpoint is hit
 */

bool
cl_uc::do_brk(void)
{
  class cl_brk *brk;
  int idx;
  bool ret= false;

  if ((sim->state & SIM_GO) &&
      rom &&
      (sim->steps_done > 0))
    {
      if (vcd_break)
        {
          vcd_break = false;
          ret= true;
        }
      if (rom->get_cell_flag(PC, CELL_FETCH_BRK))
	if ((brk= fbrk->get_bp(PC, &idx)))
	  if (brk->do_hit())
	    {
	      if (brk->perm == brkDYNAMIC)
		fbrk->del_bp(PC);
	      ret= true;
	    }
    }

  return ret;
}

bool
cl_uc::fetch(t_mem *code)
{
  bool ret = do_brk();

  if (!ret)
    *code= fetch();

  return ret;
}

int
cl_uc::do_inst(void)
{
  int res= resGO;

  save_hist();
  if (state == stGO)
    {
      pre_inst();
      instPC= PC;
      res= exec_inst();
      if (res == resINV_INST)
	/* backup to start of instruction */
	PC= instPC;
      else
	{
	  if (analyzer)
	    {
	      if (res == resGO && !inst_at(instPC))
		{
		  analyze(instPC);
		}
	    }
	}
    }
  post_inst();

  if ((res == resGO) && (PC == instPC) && stop_selfjump && !repeating)
    {
      res= resSELFJUMP;
      return res;
    }
      
  if ((res == resGO || res == resNOT_DONE))
    {
      int r= do_interrupt();
      if (r != resGO)
	res= r;
    }

  return(res);
}

int
cl_uc::do_emu(void)
{
  int res= resGO;

  if (state == stGO)
    {
      pre_emu();
      instPC= PC;
      res= exec_inst();
      if (res == resINV_INST)
	/* backup to start of instruction */
	PC= instPC;
    }
  post_emu();

  if ((res == resGO || res == resNOT_DONE))
    {
      int r= do_interrupt();
      if (r != resGO)
	res= r;
    }

  return(res);
}

void
cl_uc::pre_inst(void)
{
  inst_exec= true;
  events->disconn_all();
  vc.inst++;
  inst_ticks= 0;
  repeating= false;
}

void
cl_uc::pre_emu(void)
{
  inst_exec= true;
  events->disconn_all();
  vc.inst++;
  inst_ticks= 0;
}

int
cl_uc::exec_inst(void)
{
  return(resGO);
}

int
cl_uc::exec_inst_tab(instruction_wrapper_fn itab[])
{
  t_mem c;
  int res= resGO;
  if (fetch(&c))
    return resBREAKPOINT;
  if (itab[c] == NULL)
    {
      PC= instPC;
      return resNOT_DONE;
    }
  tickt(c);
  res= itab[c](this, c);
  if (res == resNOT_DONE)
    {
      PC= instPC;
      return res;
    }
  //tick(1);
  return res;
}


int
cl_uc::exec_inst_uctab()
{
  t_mem c;
  int res= resGO;
  if (fetch(&c))
    return resBREAKPOINT;
  if (uc_itab[c] == NULL)
    {
      PC= instPC;
      return resNOT_DONE;
    }
  tickt(c);
  res= (this->*uc_itab[c])(c);
  if (res == resNOT_DONE)
    {
      PC= instPC;
      return res;
    }
  //tick(1);
  return res;
}


void
cl_uc::post_inst(void)
{
  cPC.W(PC);
  tick_hw(inst_ticks);
  if (errors->count)
    check_errors();
  if (events->count)
    check_events();
  inst_exec= false;
}

void
cl_uc::post_emu(void)
{
  tick_hw(inst_ticks);
  errors->free_all();
  inst_exec= false;
}


static FILE *pc_dump= NULL;

void
cl_uc::save_hist()
{
  if (juj & 1)
    {
      if (pc_dump==NULL) pc_dump= fopen("addr.txt","w");
      if (pc_dump!=NULL)
	{
	  fprintf(pc_dump,"%x 0x%06x\n", MU(application->cyc), AU(PC));
	  fflush(pc_dump);
	}
    }
  hist->put();
}

int
cl_uc::inst_unknown(t_mem code)
{
  //PC--;
  class cl_error_unknown_code *e= new cl_error_unknown_code(this);
  error(e);
  return(resGO);
}


/*
 * Interrupt processing
 */

int
cl_uc::do_interrupt(void)
{
  int i;
  bool is_en= it_enabled();
  class it_level *il= (class it_level *)(it_levels->top()), *IL= 0;

  irq= false;
  for (i= 0; i < it_sources->count; i++)
    {
      class cl_it_src *is= (class cl_it_src *)(it_sources->at(i));
      is->pass_over();
    }
  for (i= 0; i < it_sources->count; i++)
    {
      class cl_it_src *is= (class cl_it_src *)(it_sources->at(i));
      if (is->is_slave())
	{
	  continue;
	}
      if (!is->is_nmi())
	{
	  if (!is_en)
	    {
	      continue;
	    }
	}
      bool A= is->is_active();
      bool E= is->enabled();
      bool P= is->pending();
      if (A && E && P)
	{
	  int pr= priority_of(is->nuof);
	  int ap;
	  irq= true;
	  if (il &&
	      il->level >= 0)
	    {
	      ap= il->level;
	    }
	  else
	    {
	      ap= priority_main();
	    }
	  if (ap >= pr)
	    {
	      continue;
	    }
	  if (state == stIDLE)
	    state= stGO;
	  is->clear();
	  sim->app->get_commander()->
	    debug("%g sec (%d clks): Accepting interrupt `%s' PC= 0x%06x\n",
			  ticks->get_rtime(), ticks->get_ticks(), object_name(is), PC);
	  IL= new it_level(pr, is->addr, PC, is);
	  return(accept_it(IL));
	}
    }
  return resGO;
}

int
cl_uc::accept_it(class it_level *il)
{
  it_levels->push(il);
  return resGO;
}

class cl_it_src *
cl_uc::search_it_src(int cid_or_nr)
{
  class cl_it_src *it;
  int i;
  for (i= 0; i<it_sources->get_count(); i++)
    {
      it= (class cl_it_src *)(it_sources->at(i));
      if (it &&
	  (
	   (it->cid != 0 && it->cid == cid_or_nr) ||
	   (it->cid != 0 && toupper(it->cid) == toupper(cid_or_nr)) ||
	   (it->nuof == cid_or_nr)
	   )
	  )
	return it;
    }
  return NULL;
}

  
/*
 * Time related functions
 */

unsigned long
cl_uc::clocks_of_time(double t)
{
  return (unsigned long)(t * xtal);
}

int
cl_uc::clock_per_cycle(void)
{
  return(1);
}

void
cl_uc::touch(void)
{
  class cl_hw *hw;
  int i;
  for (i= 0; i < hws->count; i++)
    {
      hw= (class cl_hw *)(hws->at(i));
      hw->touch();
    }
}


/*
 * Stack tracking system
 */

void
cl_uc::stack_write(void)
{
  stack_check_overflow();
  return ;
}

void
cl_uc::stack_write(t_addr sp_before)
{
  stack_check_overflow(sp_before);
  return ;
}

void
cl_uc::stack_write(class cl_stack_op *op)
{
  stack_check_overflow(op);
  delete op;
  return ;
  if (op->get_op() & stack_read_operation)
    {
      class cl_error_stack_tracker_wrong_handle *e= new
	cl_error_stack_tracker_wrong_handle(false);
      e->init();
      error(e);
      return;
    }
  stack_ops->push(op);
}

void
cl_uc::stack_read(void)
{
  return ;
}

void
cl_uc::stack_read(class cl_stack_op *op)
{
  delete op;
  return ;
  class cl_stack_op *top= (class cl_stack_op *)(stack_ops->top());

  if (op->get_op() & stack_write_operation)
    {
      class cl_error_stack_tracker_wrong_handle *e= new
	cl_error_stack_tracker_wrong_handle(true);
      e->init();
      error(e);
      return;
    }
  if (!top)
    {
      class cl_error *e= new cl_error_stack_tracker_empty(op);
      e->init();
      error(e);
      return;
    }

  if (top)
    {
      if (!top->match(op))
	{
	  class cl_error *e= new cl_error_stack_tracker_unmatch(top, op);
	  e->init();
	  error(e);
	}
      int top_size= top->data_size(), op_size= op->data_size();
      if (top_size != op_size)
	{
	  application->debug("0x%06x %d bytes to read out of stack "
			     "but %d was pushed in last operation\n",
			     (int)op->get_pc(), op_size, top_size);
	}
    }

  int removed= 0;
  while (top &&
	 top->can_removed(op))
    {
      top= (class cl_stack_op *)stack_ops->pop();
      delete top;
      top= (class cl_stack_op *)stack_ops->top();
      removed++;
    }
  if (removed != 1)
    {
      application->debug("0x%06x %d ops removed from stack-tracker "
			 "when %s happened, top pc=0x%06x "
			 "top before=0x%06x op after=0x%06x\n",
			 (int)op->get_pc(), removed, op->get_op_name(),
                         top?((int)top->get_pc()):0,
			 top?((int)top->get_before()):0,
                         (int)op->get_after());
    }

  if (top)
    {
      int ta= top->get_after(), oa= op->get_after();
      if (ta != oa)
	{
	  application->debug("0x%06x stack still inconsistent after %s, "
			     "%d byte(s) should be read out; top after"
			     "=0x%06x op after=0x%06x\n",
			     (int)op->get_pc(),
			     op->get_op_name(),
			     abs(ta-oa),
			     ta, oa);
	  class cl_error *e=
	    new cl_error_stack_tracker_inconsistent(op, abs(ta-oa));
	  e->init();
          error(e);
        }
    }

  delete op;
}

void
cl_uc::stack_check_overflow(class cl_stack_op *op)
{
}

void
cl_uc::stack_check_overflow(void)
{
}

void
cl_uc::stack_check_overflow(t_addr sp_before)
{
}

  
/*
 * Breakpoint handling
 */

class cl_fetch_brk *
cl_uc::fbrk_at(t_addr addr)
{
  int idx;

  return((class cl_fetch_brk *)(fbrk->get_bp(addr, &idx)));
}

class cl_ev_brk *
cl_uc::ebrk_at(t_addr addr, char *id)
{
  int i;
  class cl_ev_brk *eb;

  for (i= 0; i < ebrk->count; i++)
    {
      eb= (class cl_ev_brk *)(ebrk->at(i));
      if (eb->addr == addr &&
	  !strcmp(eb->id, id))
	return(eb);
    }
  return(0);
}

/*void
cl_uc::rm_fbrk(long addr)
{
  fbrk->del_bp(addr);
}*/

/* Get a breakpoint specified by its number */

class cl_brk *
cl_uc::brk_by_nr(int nr)
{
  class cl_brk *bp;

  if ((bp= fbrk->get_bp(nr)))
    return(bp);
  if ((bp= ebrk->get_bp(nr)))
    return(bp);
  return(0);
}

/* Get a breakpoint from the specified collection by its number */

class cl_brk *
cl_uc::brk_by_nr(class brk_coll *bpcoll, int nr)
{
  class cl_brk *bp;

  if ((bp= bpcoll->get_bp(nr)))
    return(bp);
  return(0);
}

/* Remove an event breakpoint specified by its address and id */

void
cl_uc::rm_ebrk(t_addr addr, char *id)
{
  int i;
  class cl_ev_brk *eb;

  for (i= 0; i < ebrk->count; i++)
    {
      eb= (class cl_ev_brk *)(ebrk->at(i));
      if (eb->addr == addr &&
	  !strcmp(eb->id, id))
	ebrk->del_bp(i, 0);
    }
}

/* Remove a breakpoint specified by its number */

bool
cl_uc::rm_brk(int nr)
{
  class cl_brk *bp;

  if ((bp= brk_by_nr(fbrk, nr)))
    {
      fbrk->del_bp(bp->addr);
      return(true);
    }
  else if ((bp= brk_by_nr(ebrk, nr)))
    {
      ebrk->del_bp(ebrk->index_of(bp), 0);
      return(true);
    }
  return(false);
}

void
cl_uc::put_breaks(void)
{}

/* Remove all fetch and event breakpoints */

void
cl_uc::remove_all_breaks(void)
{
  while (fbrk->count)
    {
      class cl_brk *brk= (class cl_brk *)(fbrk->at(0));
      fbrk->del_bp(brk->addr);
    }
  while (ebrk->count)
    ebrk->del_bp(ebrk->count-1, 0);
}

int
cl_uc::make_new_brknr(void)
{
  if (brk_counter == 0)
    return(brk_counter= 1);
  if (fbrk->count == 0 &&
      ebrk->count == 0)
    return(brk_counter= 1);
  return(++brk_counter);
}

class cl_ev_brk *
cl_uc::mk_ebrk(enum brk_perm perm, class cl_address_space *mem,
	       char op, t_addr addr, int hit)
{
  class cl_ev_brk *b;
  op= toupper(op);

  b= new cl_ev_brk(mem, make_new_brknr(), addr, perm, hit, op);
  b->init();
  return(b);
}

class cl_ev_brk *
cl_uc::mk_ebrk(enum brk_perm perm, class cl_memory_cell *cell,
	       char op, int hit)
{
  class cl_ev_brk *b;
  op= toupper(op);

  b= new cl_ev_brk(cell, make_new_brknr(), perm, hit, op);
  b->init();
  return(b);
}

void
cl_uc::check_events(void)
{
  int i;
  if (events->count)
    sim->stop(resEVENTBREAK);
  for (i= 0; i < events->count; i++)
    {
      class cl_ev_brk *brk=
	(class cl_ev_brk *)(events->object_at(i));
      brk->breaking();
    }
}

void
cl_uc::stop_when(class cl_time_measurer *t)
{
  if (stop_at_time != NULL)
    delete stop_at_time;
  stop_at_time= t;
}


/*
 * Errors
 *----------------------------------------------------------------------------
 */

cl_error_unknown_code::cl_error_unknown_code(class cl_uc *the_uc)
{
  uc= the_uc;
  classification= uc_error_registry.find("unknown_code");
}

void
cl_error_unknown_code::print(class cl_commander_base *c)
{
  c->dd_printf("%s: unknown instruction code at ", (char*)get_type_name());
  if (uc->rom)
    {
      c->dd_printf(uc->rom->addr_format, PC);
      c->dd_printf(" (");
      c->dd_printf(uc->rom->data_format, uc->rom->get(PC));
      c->dd_printf(")");
    }
  else
    c->dd_printf("0x%06x", AU(PC));
  c->dd_printf("\n");
}


cl_uc_error_registry::cl_uc_error_registry(void)
{
  class cl_error_class *prev = uc_error_registry.find("non-classified");
  prev = register_error(new cl_error_class(err_error, "unknown_code", prev, ERROR_OFF));
}

/* End of sim.src/uc.cc */
