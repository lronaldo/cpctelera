/*
 * Simulator of microcontrollers (uc.cc)
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

cl_ticker::cl_ticker(int adir, int in_isr, const char *aname)
{
  options= TICK_RUN;
  if (in_isr)
    options|= TICK_INISR;
  dir= adir;
  ticks= 0;
  set_name(aname);
}

cl_ticker::~cl_ticker(void) {}

int
cl_ticker::tick(int nr)
{
  if (options&TICK_RUN)
    ticks+= dir*nr;
  return(ticks);
}

double
cl_ticker::get_rtime(double xtal)
{
  double d;

  d= (double)ticks/xtal;
  return(d);
}

void
cl_ticker::dump(int nr, double xtal, class cl_console_base *con)
{
  con->dd_printf("timer #%d(\"%s\") %s%s: %g sec (%lu clks)\n",
		 nr, get_name("unnamed"),
		 (options&TICK_RUN)?"ON":"OFF",
		 (options&TICK_INISR)?",ISR":"",
		 get_rtime(xtal), ticks);
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
  uc->xtal= d;
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
  return uc->ticks->ticks;
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
  //printf("l=%02x\n", c);
  if (f->eof())
    return false;
  c= g(f);
  h= c;
  //printf("h=%02x\n", c);
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
 * Abstract microcontroller
 ******************************************************************************
 */

cl_uc::cl_uc(class cl_sim *asim):
  cl_base()
{
  type= NULL;
  //int i;
  sim = asim;
  //mems= new cl_list(MEM_TYPES, 1);
  memchips= new cl_list(2, 2, "memchips");
  address_spaces= new cl_address_space_list(this);
  //address_decoders= new cl_list(2, 2);
  rom= 0;

  hws = new cl_hws();
  //options= new cl_list(2, 2);
  //for (i= MEM_ROM; i < MEM_TYPES; i++) mems->add(0);
  xtal_option= new cl_xtal_option(this);
  xtal_option->init();
  ticks= new cl_ticker(+1, 0, "time");
  isr_ticks= new cl_ticker(+1, TICK_INISR, "isr");
  idle_ticks= new cl_ticker(+1, TICK_IDLE, "idle");
  counters= new cl_list(2, 2, "counters");
  it_levels= new cl_list(2, 2, "it levels");
  it_sources= new cl_irqs(2, 2);
  class it_level *il= new it_level(-1, 0, 0, 0);
  it_levels->push(il);
  stack_ops= new cl_list(2, 2, "stack operations");
  errors= new cl_list(2, 2, "errors in uc");
  events= new cl_list(2, 2, "events in uc");
  sp_max= 0;
  sp_avg= 0;
  inst_exec= false;
}


cl_uc::~cl_uc(void)
{
  //delete mems;
  delete hws;
  //delete options;
  delete ticks;
  delete isr_ticks;
  delete idle_ticks;
  delete counters;
  events->disconn_all();
  delete events;
  delete fbrk;
  delete ebrk;
  delete it_levels;
  delete it_sources;
  delete stack_ops;
  errors->free_all();
  delete errors;
  delete xtal_option;
  delete address_spaces;
  delete memchips;
  //delete address_decoders;
}


int
cl_uc::init(void)
{
  int i;

  set_name("controller");
  cl_base::init();
  if (xtal_option->use("xtal"))
    xtal= xtal_option->get_value(xtal);
  else
    xtal= 11059200;
  vars= new cl_var_list();
  make_variables();
  make_memories();
  if (rom == NULL)
    rom= address_space(cchars("rom")/*MEM_ROM_ID*/);
  ebrk= new brk_coll(2, 2, rom);
  fbrk= new brk_coll(2, 2, rom);
  fbrk->Duplicates= false;
  brk_counter= 0;
  stop_at_time= 0;
  make_cpu_hw();
  mk_hw_elements();
  class cl_cmdset *cs= sim->app->get_commander()->cmdset;
  build_cmdset(cs);
  irq= false;
  reset();

  return 0;
  for (i= 0; i < sim->app->in_files->count; i++)
    {
      char *fname= (char *)(sim->app->in_files->at(i));
      long l;
      if ((l= read_hex_file(fname)) >= 0)
	{
	  /*sim->app->get_commander()->all_*/printf("%ld words read from %s\n",
						l, fname);
	}
    }
  return(0);
}

char *
cl_uc::id_string(void)
{
  return((char*)"unknown microcontroller");
}

void
cl_uc::reset(void)
{
  class it_level *il;

  irq= false;
  instPC= PC= 0;
  state = stGO;
  ticks->ticks= 0;
  isr_ticks->ticks= 0;
  idle_ticks->ticks= 0;
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
  sp_max= 0;
  sp_avg= 0;

  stack_ops->free_all();

  int i;
  for (i= 0; i < hws->count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(hws->at(i));
      hw->reset();
    }
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

      chip= new cl_memory_chip("variable_storage", l, 32);
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

  add_hw(h= new cl_simulator_interface(this));
  h->init();
  add_hw(h= new cl_vcd(this, 0, "vcd"));
  h->init();
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

  cmdset->add(cmd= new cl_pc_cmd("pc", 0));
  cmd->init();

  cmdset->add(cmd= new cl_reset_cmd("reset", 0));
  cmd->init();

  cmdset->add(cmd= new cl_dump_cmd("dump", true));
  cmd->init();

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
  }
  if (!super_cmd)
    {
      cmdset->add(cmd= new cl_super_cmd("get", 0, cset));
      cmd->init();
      set_get_help(cmd);
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
    cmd->init();
    cset->add(cmd= new cl_set_bit_cmd("bit", 0));
    cmd->init();
    cset->add(cmd= new cl_set_hw_cmd("hardware", 0));
    cmd->add_name("hw");
    cmd->init();
  }
  if (!super_cmd)
    {
      cmdset->add(cmd= new cl_super_cmd("set", 0, cset));
      cmd->init();
      set_set_help(cmd);
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
  }
  if (!super_cmd) {
    cmdset->add(cmd= new cl_super_cmd("info", 0, cset));
    cmd->init();
    set_info_help(cmd);
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
  }
  if (!super_cmd) {
    cmdset->add(cmd= new cl_super_cmd("timer", 0, cset));
    cmd->init();
    set_timer_help(cmd);
  }

  {
    class cl_super_cmd *mem_create;
    class cl_cmdset *mem_create_cset;
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
  }
  if (!super_cmd) {
    cmdset->add(cmd= new cl_super_cmd("memory", 0, cset));
    cmd->init();
    set_memory_help(cmd);
  }

  cmdset->add(cmd= new cl_var_cmd("var", 0));
  cmd->init();
  cmd->add_name("variable");
}


/*
 * Read/write simulated memory
 */

t_mem
cl_uc::read_mem(char *id, t_addr addr)
{
  class cl_address_space *m= address_space(id);

  return(m?(m->read(addr)):0);
}

t_mem
cl_uc::get_mem(char *id, t_addr addr)
{
  class cl_address_space *m= address_space(id);

  return(m?(m->get(addr)):0);
}

void
cl_uc::write_mem(char *id, t_addr addr, t_mem val)
{
  class cl_address_space *m= address_space(id);

  if (m)
    m->write(addr, val);
}

void
cl_uc::set_mem(char *id, t_addr addr, t_mem val)
{
  class cl_address_space *m= address_space(id);

  if(m)
    m->set(addr, val);
}


/*
class cl_memory *
cl_uc::mem(enum mem_class type)
{
  class cl_m *m;

  if (mems->count < type)
    m= (class cl_m *)(mems->at(MEM_DUMMY));
  else
    m= (class cl_m *)(mems->at(type));
  return(m);
}
*/

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
      class cl_memory *m= dynamic_cast<cl_memory *>(b);
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
  //printf("rom[%06lx]=%02x\n", addr, val);
  t_addr size= rom->get_size();
  if (addr < size)
    {
      rom->download(addr, val);
      return;
    }
  t_addr bank, caddr;
  bank= addr / size;
  caddr= addr % size;
  //printf("getting decoder of %ld/%lx\n", bank, caddr);
  class cl_banker *d= (class cl_banker *)(rom->get_decoder_of(caddr));
  if (d)
    {
      if (!d->is_banker())
	{
	  //printf("cell at %lx has no banker\n", caddr);
	  return;
	}
      //printf("setting %ld/rom[%lx]=%x\n", bank, caddr, val);
      d->switch_to(bank, NULL);
      rom->download(caddr, val);
      d->activate(NULL);
    }
  else
    ;//printf("no decoder at %lx\n", caddr);
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

  uint  base= 0;  // extended address, added to every adress
  uchar dnum;     // data number
  uchar rtyp=0;   // record type
  uint  addr= 0;  // address
  uchar rec[300]; // data record
  uchar sum ;     // checksum
  uchar chk ;     // check
  int  i;
  bool ok, get_low= 1;
  uchar low= 0, high;

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
	     (/*c != EOF*/!f->eof())) /*printf("search_record=%c\n",c)*/;
      if (c != ':')
	{fprintf(stderr, ": not found\n");break;}
      recnum++;
      dnum= ReadInt(f, &ok, 1);//printf("%ld:dnum=%02x ",recnum,dnum);
      chk = dnum;
      addr= ReadInt(f, &ok, 2);//printf("%ld:addr=%04x ",recnum,addr);
      chk+= (addr & 0xff);
      chk+= ((addr >> 8) & 0xff);
      rtyp= ReadInt(f, &ok, 1);//printf("%ld:rtyp=%02x ",recnum,rtyp);
      chk+= rtyp;
      for (i= 0; ok && (i < dnum); i++)
	{
	  rec[i]= ReadInt(f, &ok, 1);//printf("%02x",rec[i]);
	  chk+= rec[i];
	}
      if (ok)
	{
	  sum= ReadInt(f, &ok, 1);//printf(" %ld:sum=%02x\n",recnum,sum);
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
			      if (get_low)
				{
				  low= rec[i];
				  get_low= 0;
				}
			      else
				{
				  high= rec[i];
				  set_rom(base+addr, (high*256)+low);
				  addr++;
				  written++;
				  get_low= 1;
				}
			    }
			}
		    }
		  else if (rtyp == 4)
		    {
		      //printf("hex record type=4\n");
		      if (dnum >= 2)
			{
			  base= (rec[0]*256+rec[1]) << 16;
			  //printf("hex base=%x\n", base);
			}
		    }
		  else
		    if (rtyp != 1)
		      /*application->debug*/fprintf(stderr, "Unknown record type %d(0x%x)\n",
					 rtyp, rtyp);
		}
	      else
		/*application->debug*/fprintf(stderr, "Checksum error (%x instead of %x) in "
				   "record %ld.\n", chk, sum, recnum);
	    }
	  else
	    /*application->debug*/fprintf(stderr, "Read error in record %ld.\n", recnum);
	}
    }
  if (rom->width > 8 &&
      !get_low)
    rom->set(addr, low);

  analyze(0);
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
cl_uc::read_cdb_file(cl_f *f)
{
  class cl_cdb_recs *fns= new cl_cdb_recs();
  chars ln;
  char *lc;
  long cnt= 0;
  class cl_cdb_rec *r;
  class cl_var *v;
  
  ln= f->get_s();
  while (!ln.empty())
    {
      //printf("CBD LN=%s\n",(char*)ln);
      lc= (char*)ln;
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
		      vars->add(v= new cl_var(n, rom, r->addr, ""));
		      v->init();
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
	      if ((ln[1] == ':') &&
		  (lc[2] == 'G'))
		{
		  ln.start_parse(4);
		  chars n= ln.token("$");
		  chars t= ln.token(":");
		  t= ln.token(" ");
		  t_addr a= strtol((char*)t, 0, 16);
		  if ((r= fns->rec(n)) != NULL)
		    {
		      fns->del(n);
		      vars->add(v= new cl_var(n, rom, a, ""));
		      v->init();
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

  c= chars("", "%s.ihx", (char*)nam);
  f->open(c, chars("r"));
  o= (f->opened());
  if (o)
    return f;
  c= chars("", "%s.hex", (char*)nam);
  f->open(c, chars("r"));
  o= (f->opened());
  if (o)
    return f;
  c= chars("", "%s.ihex", (char*)nam);
  f->open(c, chars("r"));
  o= (f->opened());
  if (o)
    return f;

  c= chars("", "%s.omf", (char*)nam);
  f->open(c, chars("r"));
  o= (f->opened());
  if (o)
    return f;

  delete f;
  return NULL;
}

long
cl_uc::read_file(chars nam, class cl_console_base *con)
{
  cl_f *f= find_loadable_file(nam);
  long l= 0;
  
  if (!f)
    {
      if (con) con->dd_printf("no loadable file found\n");
      return 0;
    }
  /*if (con) con->dd_*/printf("Loading from %s\n", f->get_file_name());
  if (is_hex_file(f))
    {
      l= read_hex_file(f);
      printf("%ld words read from %s\n", l, f->get_fname());
    }
  else if (is_omf_file(f))
    {
      l= read_omf_file(f);
      printf("%ld words read from %s\n", l, f->get_fname());
    }
  else if (is_cdb_file(f))
    {
      l= read_cdb_file(f);
      printf("%ld symbols read from %s\n", l, f->get_fname());
    }
  if (strcmp(nam, f->get_fname()) != 0)
    {
      chars n= nam;
      n+= (char*)".cdb";
      cl_f *c= mk_io(n, "r");
      if (c->opened())
	{
	  l= read_cdb_file(c);
	  printf("%ld symbols read from %s\n", l, c->get_fname());
	}
      delete c;
    }
  delete f;
  return l;
}


/*
 * Handling instruction map
 *
 * `inst_at' is checking if the specified address is in instruction
 * map and `set_inst_at' marks the address in the map and
 * `del_inst_at' deletes the mark. `there_is_inst' cheks if there is
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
      if (hw->cathegory == cath)
	break;
    }
  if (i >= hws->count)
    return(0);
  if (idx)
    *idx= i;
  return(hw);
}

class cl_hw *
cl_uc::get_hw(char *id_string, int *idx)
{
  class cl_hw *hw= 0;
  int i= 0;

  if (idx)
    i= *idx;
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
cl_uc::get_hw(char *id_string, int hwid, int *idx)
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
cl_uc::disass(t_addr addr, const char *sep)
{
  char *buf;

  buf= (char*)malloc(100);
  strcpy(buf, "uc::disass() unimplemented\n");
  return(buf);
}

void
cl_uc::print_disass(t_addr addr, class cl_console_base *con)
{
  char *dis;
  class cl_brk *b;
  int i, l;

  if (!rom)
    return;

  t_mem code= rom->get(addr);
  b= fbrk_at(addr);
  dis= disass(addr, NULL);
  if (b)
    con->dd_cprintf("answer", "%c", (b->perm == brkFIX)?'F':'D');
  else
    con->dd_printf(" ");
  con->dd_cprintf("answer", "%c ", inst_at(addr)?' ':'?');
  con->dd_cprintf("dump_address", rom->addr_format, addr); con->dd_printf(" ");
  con->dd_cprintf("dump_number", rom->data_format, code);
  l= inst_length(addr);
  for (i= 1; i < l; i++)
    {
      con->dd_printf(" ");
      con->dd_cprintf("dump_number", rom->data_format, rom->get(addr+i));
    }
  int li= longest_inst();
  while (i < li)
    {
      int j;
      j= rom->width/4 + ((rom->width%4)?1:0) + 1;
      while (j)
	con->dd_printf(" "), j--;
      i++;
    }
  con->dd_cprintf("dump_char", " %s\n", dis);
  free((char *)dis);
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

bool
cl_uc::addr_name(t_addr addr, class cl_address_space *as, char *buf)
{
  t_index i;
  
  for (i= 0; i < vars->count; i++)
    {
      class cl_var *v= (cl_var *)(vars->at(i));
      if ((v->as == as) &&
	  (v->addr == addr))
	{
	  strcpy(buf, v->get_name());
	  return true;
	}
    }
  unsigned int a= addr;
  sprintf(buf, "%02x", a);
  return false;
}

bool
cl_uc::addr_name(t_addr addr, class cl_address_space *as, int bitnr, char *buf)
{
  t_index i;
  
  for (i= 0; i < vars->count; i++)
    {
      class cl_var *v= (cl_var *)(vars->at(i));
      if ((v->as == as) &&
	  (v->addr == addr) &&
	  (v->bitnr == bitnr))
	{
	  strcpy(buf, v->get_name());
	  return true;
	}
    }
  unsigned int a= addr;
  sprintf(buf, "%02x.%d", a, bitnr);
  return false;
}

bool
cl_uc::symbol2address(char *sym,
		      class cl_address_space **as,
		      t_addr *addr)
{
  class cl_var *v;
  t_index i;

  if (!sym ||
      !*sym)
    return false;
  if (vars->search(sym, i))
    {
      v= (class cl_var *)(vars->at(i));
      if (v->bitnr >= 0)
	return false;
      if (as)
	*as= v->as;
      if (addr)
	*addr= v->addr;
      return true;
    }
  return false;
}
  
char *
cl_uc::symbolic_bit_name(t_addr bit_address,
			 class cl_memory *mem,
			 t_addr mem_addr,
			 t_mem bit_mask)
{
  //char *sym_name= 0;
  int i;
  chars c= chars("", mem?(mem->addr_format):"0x%06lx", (unsigned long)mem_addr);
  /*if (!sym_name)
    {
      sym_name= (char *)malloc(16);
      sprintf(sym_name, mem?(mem->addr_format):"0x%06lx", (unsigned long)mem_addr);
      }*/
  /*sym_name= (char *)realloc(sym_name, strlen(sym_name)+2);
    strcat(sym_name, ".");*/
  c+= cchars(".");
  i= 0;
  while (bit_mask > 1)
    {
      bit_mask>>=1;
      i++;
    }
  //char bitnumstr[10];
  /*sprintf(bitnumstr, "%1d", i);
    strcat(sym_name, bitnumstr);*/
  c.append("%d", i);
  return(/*sym_name*/strdup((char*)c));
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
      //printf("tabl[%d].name=%s <-> %s\n",i,tabl[i].name,name);
      i++;
    }
  if (tabl[i].name != NULL)
    return(&tabl[i]);
  else
    return(0);
}

chars
cl_uc::cell_name(class cl_memory_cell *cell)
{
  if (cell == NULL)
    return chars("");
  if (cell->get_flag(CELL_VAR))
    {
      int i;
      for (i= 0; i < vars->count; i++)
	{
	  class cl_var *v= (cl_var*)(vars->at(i));
	  if (v->get_cell() &&
	      (cell == v->get_cell()))
	    return chars(v->get_name());
	}
    }
  class cl_address_space *as;
  t_addr a;
  as= address_space(cell, &a);
  if (as == NULL)
    return chars("");
  return chars("", "%s_%06x", as->get_name(), a);
}

class cl_var *
cl_uc::var(char *nam)
{
  if (!vars)
    return NULL;
  t_index i;
  if (!vars->search(nam, i))
    return NULL;
  class cl_var *v= (cl_var*)(vars->at(i));
  return v;
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
	try {
	  class cl_event_address_space_added &e=
	    dynamic_cast<class cl_event_address_space_added &>(event);
	  address_space_added(e.as);
	  e.handle();
	}
	catch (...)
	  { break; }
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
  //printf("error adding: %s...\n", error->get_class()->get_name());
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
      //printf("error list: %d items\n", errors->count);
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
		con= c->frozen_console;
	      if (con)
		{
		  con->dd_printf("Erronouse instruction: ");
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
cl_uc::bit2mem(t_addr bitaddr, t_addr *memaddr, t_mem *bitmask)
{
  if (memaddr)
    *memaddr= bitaddr;
  if (bitmask)
    *bitmask= 1 << (bitaddr & 0x7);
  return(0); // abstract...
}


/*
 * Execution
 */

int
cl_uc::tick_hw(int cycles)
{
  class cl_hw *hw;
  int i;//, cpc= clock_per_cycle();

  // tick hws
  for (i= 0; i < hws->count; i++)
    {
      hw= (class cl_hw *)(hws->at(i));
      if ((hw->flags & HWF_INSIDE) &&
	  (hw->on))
	hw->tick(cycles);
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
  int i, cpc= clock_per_cycle();

  // increase time
  ticks->tick(cycles * cpc);
  class it_level *il= (class it_level *)(it_levels->top());
  if (il->level >= 0)
    isr_ticks->tick(cycles * cpc);
  if (state == stIDLE)
    idle_ticks->tick(cycles * cpc);
  for (i= 0; i < counters->count; i++)
    {
      class cl_ticker *t= (class cl_ticker *)(counters->at(i));
      if (t)
	{
	  if ((t->options&TICK_INISR) ||
	      il->level < 0)
	    t->tick(cycles * cpc);
	}
    }

  // tick for hardwares
  inst_ticks+= cycles;
  return(0);
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
  PC= rom->inc_address(PC);
  vc.fetch++;
  return(code);
}

/*
 * Fetch but checking for breakpoint hit first, returns TRUE if
 * a breakpoint is hit
 */

bool
cl_uc::fetch(t_mem *code)
{
  class cl_brk *brk;
  int idx;

  if (!code)
    return(0);
  if ((sim->state & SIM_GO) &&
      rom &&
      (sim->steps_done > 0))
    {
      if (rom->get_cell_flag(PC, CELL_FETCH_BRK))
	if ((brk= fbrk->get_bp(PC, &idx)))
	  if (brk->do_hit())
	    {
	      if (brk->perm == brkDYNAMIC)
		fbrk->del_bp(PC);
	      return(1);
	    }
    }
  *code= fetch();
  return(0);
}

int
cl_uc::do_inst(int step)
{
  t_addr PCsave;
  int res= resGO;

  if (step < 0)
    step= 1;
  while (step-- &&
         res == resGO)
    {
      pre_inst();
      PCsave = PC;
      res= exec_inst();

      if (res == resINV_INST)
	/* backup to start of instruction */
	PC = PCsave;
      
      post_inst();

      if ((res == resGO) &&
	  irq)
	{
	  //printf("DO INTERRUPT PC=%lx\n", PC);
	  int r= do_interrupt();
	  if (r != resGO)
	    res= r;
	}

      if (stop_at_time &&
	  stop_at_time->reached())
	{
	  delete stop_at_time;
	  stop_at_time= NULL;
	  res= resBREAKPOINT;
	}
    }
  if (res != resGO)
    sim->stop(res);
  return(res);
}

void
cl_uc::pre_inst(void)
{
  inst_exec= true;
  inst_ticks= 0;
  events->disconn_all();
  vc.inst++;
}

int
cl_uc::exec_inst(void)
{
  instPC= PC;
  return(resGO);
}

int
cl_uc::exec_inst_tab(instruction_wrapper_fn itab[])
{
  t_mem c;
  int res= resGO;
  instPC= PC;
  if (fetch(&c))
    return resBREAKPOINT;
  if (itab[c] == NULL)
    {
      PC= instPC;
      return resNOT_DONE;
    }
  res= itab[c](this, c);
  if (res == resNOT_DONE)
    {
      PC= instPC;
      return res;
    }
  tick(1);
  return res;
}


void
cl_uc::post_inst(void)
{
  tick_hw(inst_ticks);
  if (errors->count)
    check_errors();
  if (events->count)
    check_events();
  inst_exec= false;
}


/*
 * Interrupt processing
 */

int
cl_uc::do_interrupt(void)
{
  int i;
  // NMI?

  // Maskable interrupts
  if (!it_enabled())
    {
      //printf("do_interrupt skip (it disabled)\n");
      return resGO;
    }
  class it_level *il= (class it_level *)(it_levels->top()), *IL= 0;
  irq= false;
  //printf("Checking IRQs...\n");
  for (i= 0; i < it_sources->count; i++)
    {
      class cl_it_src *is= (class cl_it_src *)(it_sources->at(i));
       if (is->is_active() &&
	  is->enabled() &&
	  is->pending())
	{
	  int pr= priority_of(is->nuof);
	  int ap;
	  irq= true;
	  if (il &&
	      il->level >= 0)
	    ap= il->level;
	  else
	    ap= priority_main();
	  if (ap >= pr)
	    continue;
	  is->clear();
	  sim->app->get_commander()->
	    debug("%g sec (%d clks): Accepting interrupt `%s' PC= 0x%06x\n",
			  get_rtime(), ticks->ticks, object_name(is), PC);
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


/*
 * Time related functions
 */

double
cl_uc::get_rtime(void)
{
  /*  double d;

  d= (double)ticks/xtal;
  return(d);*/
  return(ticks->get_rtime(xtal));
}

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

void
cl_uc::check_events(void)
{
  int i;
  //sim->stop(resEVENTBREAK);
  for (i= 0; i < events->count; i++)
    {
      class cl_ev_brk *brk=
	dynamic_cast<class cl_ev_brk *>(events->object_at(i));
      sim->stop(resEVENTBREAK, brk);
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
  //FILE *f= c->get_out();
  /*cmd_fprintf(f,*/c->dd_printf("%s: unknown instruction code at ", get_type_name());
  if (uc->rom)
    {
      /*cmd_fprintf(f,*/c->dd_printf(uc->rom->addr_format, PC);
      /*cmd_fprintf(f,*/c->dd_printf(" (");
      /*cmd_fprintf(f,*/c->dd_printf(uc->rom->data_format, uc->rom->get(PC));
      /*cmd_fprintf(f,*/c->dd_printf(")");
    }
  else
    /*cmd_fprintf(f,*/c->dd_printf("0x%06x", AU(PC));
  /*cmd_fprintf(f,*/c->dd_printf("\n");
}


cl_uc_error_registry::cl_uc_error_registry(void)
{
  class cl_error_class *prev = uc_error_registry.find("non-classified");
  prev = register_error(new cl_error_class(err_error, "unknown_code", prev, ERROR_OFF));
}

/* End of uc.cc */
