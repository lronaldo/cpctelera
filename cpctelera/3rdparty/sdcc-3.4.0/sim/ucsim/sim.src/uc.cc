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

#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "i_string.h"

// prj
#include "globals.h"
#include "utils.h"

// cmd.src
#include "newcmdcl.h"
#include "cmduccl.h"
#include "bpcl.h"
#include "getcl.h"
#include "setcl.h"
#include "infocl.h"
#include "timercl.h"
#include "cmdstatcl.h"
#include "cmdmemcl.h"
#include "cmdutil.h"

// local, sim.src
#include "uccl.h"
#include "hwcl.h"
#include "memcl.h"
#include "simcl.h"
#include "itsrccl.h"

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


/*
 * Abstract microcontroller
 ******************************************************************************
 */

cl_uc::cl_uc(class cl_sim *asim):
  cl_base()
{
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
  inst_exec= DD_FALSE;
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
  make_memories();
  rom= address_space(MEM_ROM_ID);
  ebrk= new brk_coll(2, 2, rom);
  fbrk= new brk_coll(2, 2, rom);
  fbrk->Duplicates= DD_FALSE;
  brk_counter= 0;
  mk_hw_elements();
  reset();
  class cl_cmdset *cs= sim->app->get_commander()->cmdset;
  build_cmdset(cs);

  for (i= 0; i < sim->app->in_files->count; i++)
    {
      char *fname= (char *)(sim->app->in_files->at(i));
      long l;
      if ((l= read_hex_file(fname)) >= 0)
        {
          sim->app->get_commander()->all_printf("%ld words read from %s\n",
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

  instPC= PC= 0;
  state = stGO;
  ticks->ticks= 0;
  isr_ticks->ticks= 0;
  idle_ticks->ticks= 0;
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
{}

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
cl_uc::mk_hw_elements(void)
{
}

void
cl_uc::build_cmdset(class cl_cmdset *cmdset)
{
  class cl_cmd *cmd;
  class cl_super_cmd *super_cmd;
  class cl_cmdset *cset;

  cmdset->add(cmd= new cl_state_cmd("state", 0,
"state              State of microcontroller",
"long help of state"));
  cmd->init();

#ifdef STATISTIC
  cmdset->add(cmd= new cl_statistic_cmd("statistic", 0,
"statistic [mem [startaddr [endaddr]]]\n"
"                   Statistic of memory accesses",
"long help of statistic"));
  cmd->init();
#endif

  cmdset->add(cmd= new cl_file_cmd("file", 0,
"file \"FILE\"        Load FILE into ROM",
"long help of file"));
  cmd->init();
  cmd->add_name("load");

  cmdset->add(cmd= new cl_dl_cmd("download", 0,
"download           Load (intel.hex) data",
"long help of download"));
  cmd->init();
  cmd->add_name("dl");

  cmdset->add(cmd= new cl_pc_cmd("pc", 0,
"pc [addr]          Set/get PC",
"long help of pc"));
  cmd->init();

  cmdset->add(cmd= new cl_reset_cmd("reset", 0,
"reset              Reset",
"long help of reset"));
  cmd->init();

  cmdset->add(cmd= new cl_dump_cmd("dump", DD_TRUE,
"dump memory_type [start [stop [bytes_per_line]]]\n"
"                   Dump memory of specified type\n"
"dump bit...        Dump bits",
"long help of dump"));
  cmd->init();

  cmdset->add(cmd= new cl_dch_cmd("dch", DD_TRUE,
"dch [start [stop]] Dump code in hex form",
"long help of dch"));
  cmd->init();

  cmdset->add(cmd= new cl_dc_cmd("dc", DD_TRUE,
"dc [start [stop]]  Dump code in disass form",
"long help of dc"));
  cmd->init();

  cmdset->add(cmd= new cl_disassemble_cmd("disassemble", DD_TRUE,
"disassemble [start [offset [lines]]]\n"
"                   Disassemble code",
"long help of disassemble"));
  cmd->init();

  cmdset->add(cmd= new cl_fill_cmd("fill", 0,
"fill memory_type start end data\n"
"                   Fill memory region with data",
"long help of fill"));
  cmd->init();

  cmdset->add(cmd= new cl_where_cmd("where", 0,
"where memory_type data...\n"
"                   Case unsensitive search for data",
"long help of where"));
  cmd->init();

  cmdset->add(cmd= new cl_Where_cmd("Where", 0,
"Where memory_type data...\n"
"                   Case sensitive search for data",
"long help of Where"));
  cmd->init();

  cmdset->add(cmd= new cl_break_cmd("break", 0,
"break addr [hit]   Set fix breakpoint\n"
"break mem_type r|w addr [hit]\n"
"                   Set fix event breakpoint",
"long help of break"));
  cmd->init();

  cmdset->add(cmd= new cl_tbreak_cmd("tbreak", 0,
"tbreak addr [hit]  Set temporary breakpoint\n"
"tbreak mem_type r|w addr [hit]\n"
"                   Set temporary event breakpoint",
"long help of tbreak"));
  cmd->init();

  cmdset->add(cmd= new cl_clear_cmd("clear", 0,
"clear [addr...]    Clear fix breakpoint",
"long help of clear"));
  cmd->init();

  cmdset->add(cmd= new cl_delete_cmd("delete", 0,
"delete [nr...]     Delete breakpoint(s)",
"long help of clear"));
  cmd->init();

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("get"));
    if (super_cmd)
      cset= super_cmd->commands;
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_get_sfr_cmd("sfr", 0,
"get sfr address...\n"
"                   Get value of addressed SFRs",
"long help of get sfr"));
    cmd->init();
    /*cset->add(cmd= new cl_get_option_cmd("option", 0,
"get option name\n"
"                   Get value of an option",
"long help of get option"));
cmd->init();*/
  }
  if (!super_cmd)
    {
      cmdset->add(cmd= new cl_super_cmd("get", 0,
"get subcommand     Get, see `get' command for more help",
"long help of get", cset));
      cmd->init();
    }

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("set"));
    if (super_cmd)
      cset= super_cmd->commands;
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_set_mem_cmd("memory", 0,
"set memory memory_type address data...\n"
"                   Place list of data into memory",
"long help of set memory"));
    cmd->init();
    cset->add(cmd= new cl_set_bit_cmd("bit", 0,
"set bit addr 0|1   Set specified bit to 0 or 1",
"long help of set bit"));
    cmd->init();
    cset->add(cmd= new cl_set_hw_cmd("hardware", 0,
"set hardware cathegory params...\n"
"                   Set parameters of specified hardware element",
"long help of set hardware"));
    cmd->add_name("hw");
    cmd->init();
  }
  if (!super_cmd)
    {
      cmdset->add(cmd= new cl_super_cmd("set", 0,
"set subcommand     Set, see `set' command for more help",
"long help of set", cset));
      cmd->init();
    }

  { // info
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("info"));
    if (super_cmd)
      cset= super_cmd->get_subcommands();
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_info_bp_cmd("breakpoints", 0,
"info breakpoints   Status of user-settable breakpoints",
"long help of info breakpoints"));
    cmd->add_name("bp");
    cmd->init();
    cset->add(cmd= new cl_info_reg_cmd("registers", 0,
"info registers     List of integer registers and their contents",
"long help of info registers"));
    cmd->init();
    cset->add(cmd= new cl_info_hw_cmd("hardware", 0,
"info hardware cathegory\n"
"                   Status of hardware elements of the CPU",
"long help of info hardware"));
    cmd->add_name("hw");
    cmd->init();
    cset->add(cmd= new cl_info_stack_cmd("stack", 0,
"info stack         Status of stack of the CPU",
"long help of info stack"));
    cmd->init();
    cset->add(cmd= new cl_info_memory_cmd("memory", 0,
"info memory        Information about memory system",
"long help of info memory"));
    cmd->init();
  }
  if (!super_cmd) {
    cmdset->add(cmd= new cl_super_cmd("info", 0,
"info subcommand    Information, see `info' command for more help",
"long help of info", cset));
    cmd->init();
  }

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("timer"));
    if (super_cmd)
      cset= super_cmd->get_subcommands();
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_timer_add_cmd("add", 0,
"timer add id [direction [in_isr]]\n"
"                   Create a clock counter (timer)",
"log help of timer add"));
    cmd->init();
    cmd->add_name("create");
    cmd->add_name("make");
    cset->add(cmd= new cl_timer_delete_cmd("delete", 0,
"timer delete id    Delete a timer",
"long help of timer delete"));
    cmd->init();
    cmd->add_name("remove");
    cset->add(cmd= new cl_timer_get_cmd("get", 0,
"timer get [id]     Get value of a timer, or all",
"long help of timer get"));
    cmd->init();
    cset->add(cmd= new cl_timer_run_cmd("run", 0,
"timer start id     Start a timer",
"long help of timer run"));
    cmd->init();
    cmd->add_name("start");
    cset->add(cmd= new cl_timer_stop_cmd("stop", 0,
"timer stop id      Stop a timer",
"long help of timer stop"));
    cmd->init();
    cset->add(cmd= new cl_timer_value_cmd("set", 0,
"timer set id value\n"
"                   Set a timer value",
"long help of timer set"));
    cmd->init();
    cmd->add_name("value");
  }
  if (!super_cmd) {
    cmdset->add(cmd= new cl_super_cmd("timer", 0,
"timer subcommand   Manage timers",
"long help of timer", cset));
    cmd->init();
  }

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("memory"));
    if (super_cmd)
      cset= super_cmd->get_subcommands();
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    /*
    cset->add(cmd= new cl_memory_cmd("_no_parameters_", 0,
"memory             Information about memory system",
"long help of memory"));
    cmd->init();
    */
    cset->add(cmd= new cl_memory_createchip_cmd("createchip", 0,
"memory createchip id size cellsize\n"
"                   Create a new memory chip",
"long help of memory createchip"));
    cmd->init();
    cmd->add_name("cchip");
    cset->add(cmd= new cl_memory_createaddressspace_cmd("createaddressspace", 0,
"memory createaddressspace id startaddr size\n"
"                   Create address space",
"long help of memory createaddressspace"));
    cmd->init();
    cmd->add_name("createaddrspace");
    cmd->add_name("createaspace");
    cmd->add_name("caddressspace");
    cmd->add_name("caddrspace");
    cmd->add_name("caspace");
    cset->add(cmd= new cl_memory_createaddressdecoder_cmd("createaddressdecoder", 0,
"memory createaddressdecoder addressspace begin end chip begin\n"
"                   Create address decoder",
"long help of memory createaddressdecoder"));
    cmd->init();
    cmd->add_name("createaddrdecoder");
    cmd->add_name("createadecoder");
    cmd->add_name("caddressdecoder");
    cmd->add_name("caddrdecoder");
    cmd->add_name("cadecoder");
  }
  if (!super_cmd) {
    cmdset->add(cmd= new cl_super_cmd("memory", 0,
"memory subcommand  Manage memory chips and address spaces",
"long help of memory", cset));
    cmd->init();
  }
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

class cl_memory *
cl_uc::memory(const char *id)
{
  int i;

  if (!id ||
      !(*id))
    return(0);
  for (i= 0; i < address_spaces->count; i++)
    {
      class cl_memory *m= (cl_memory *)(address_spaces->at(i));
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
ReadInt(FILE *f, bool *ok, int bytes)
{
  char s2[3];
  long l= 0;

  *ok= DD_FALSE;
  while (bytes)
    {
      if (fscanf(f, "%2c", &s2[0]) == EOF)
        return(0);
      s2[2]= '\0';
      l= l*256 + strtol(s2, NULL, 16);
      bytes--;
    }
  *ok= DD_TRUE;
  return(l);
}


/*
 * Reading intel hexa file into EROM
 *____________________________________________________________________________
 *
 * If parameter is a NULL pointer, this function reads data from `cmd_in'
 *
 */

long
cl_uc::read_hex_file(const char *nam)
{
  FILE *f;
  int c;
  long written= 0, recnum= 0;

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

  if (!nam)
    {
      sim->app->get_commander()->
        dd_printf("cl_uc::read_hex_file File name not specified\n");
      return(-1);
    }
  else
    if ((f= fopen(nam, "r")) == NULL)
      {
        fprintf(stderr, "Can't open `%s': %s\n", nam, strerror(errno));
        return(-1);
      }

  //memset(inst_map, '\0', sizeof(inst_map));
  ok= DD_TRUE;
  while (ok &&
         rtyp != 1)
    {
      while (((c= getc(f)) != ':') &&
             (c != EOF)) ;
      if (c != ':')
        {fprintf(stderr, ": not found\n");break;}
      recnum++;
      dnum= ReadInt(f, &ok, 1);//printf("dnum=%02x",dnum);
      chk = dnum;
      addr= ReadInt(f, &ok, 2);//printf("addr=%04x",addr);
      chk+= (addr & 0xff);
      chk+= ((addr >> 8) & 0xff);
      rtyp= ReadInt(f, &ok, 1);//printf("rtyp=%02x ",rtyp);
      chk+= rtyp;
      for (i= 0; ok && (i < dnum); i++)
        {
          rec[i]= ReadInt(f, &ok, 1);//printf("%02x",rec[i]);
          chk+= rec[i];
        }
      if (ok)
        {
          sum= ReadInt(f, &ok, 1);//printf(" sum=%02x\n",sum);
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
                              rom->set(addr, rec[i]);
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
                                  rom->set(addr, (high*256)+low);
                                  addr++;
                                  written++;
                                  get_low= 1;
                                }
                            }
                        }
                    }
                  else
                    if (rtyp != 1)
                      application->debug("Unknown record type %d(0x%x)\n",
                                         rtyp, rtyp);
                }
              else
                application->debug("Checksum error (%x instead of %x) in "
                                   "record %ld.\n", chk, sum, recnum);
            }
          else
            application->debug("Read error in record %ld.\n", recnum);
        }
    }
  if (rom->width > 8 &&
      !get_low)
    rom->set(addr, low);

  if (nam)
    fclose(f);
  application->debug("%ld records have been read\n", recnum);
  analyze(0);
  return(written);
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
    rom->set_cell_flag(addr, DD_TRUE, CELL_INST);
}

void
cl_uc::del_inst_at(t_addr addr)
{
  if (rom)
    rom->set_cell_flag(addr, DD_FALSE, CELL_INST);
}

bool
cl_uc::there_is_inst(void)
{
  if (!rom)
    return(0);
  bool got= DD_FALSE;
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

/* Looking for a specific HW element */

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


/*
 * Help of the command interpreter
 */

struct dis_entry *
cl_uc::dis_tbl(void)
{
  static struct dis_entry empty= { 0, 0, 0, 0, NULL };
  return(&empty);
}

struct name_entry *
cl_uc::sfr_tbl(void)
{
  static struct name_entry empty= { 0, 0 };
  return(&empty);
}

struct name_entry *
cl_uc::bit_tbl(void)
{
  static struct name_entry empty= { 0, 0 };
  return(&empty);
}

const char *
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
  const char *dis;
  class cl_brk *b;
  int i;

  if (!rom)
    return;

  t_mem code= rom->get(addr);
  b= fbrk_at(addr);
  dis= disass(addr, NULL);
  if (b)
    con->dd_printf("%c", (b->perm == brkFIX)?'F':'D');
  else
    con->dd_printf(" ");
  con->dd_printf("%c ", inst_at(addr)?' ':'?');
  con->dd_printf(rom->addr_format, addr); con->dd_printf(" ");
  con->dd_printf(rom->data_format, code);
  for (i= 1; i < inst_length(addr); i++)
    {
      con->dd_printf(" ");
      con->dd_printf(rom->data_format, rom->get(addr+i));
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
  con->dd_printf(" %s\n", dis);
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
cl_uc::get_name(t_addr addr, struct name_entry tab[], char *buf)
{
  int i;

  i= 0;
  while (tab[i].name &&
         (!(tab[i].cpu_type & type) ||
         (tab[i].addr != addr)))
    i++;
  if (tab[i].name)
    strcpy(buf, tab[i].name);
  return(tab[i].name != NULL);
}

bool
cl_uc::symbol2address(char *sym, struct name_entry tab[],
                      t_addr *addr)
{
  int i;

  if (!sym ||
      !*sym)
    return(DD_FALSE);
  i= 0;
  while (tab[i].name &&
         (!(tab[i].cpu_type & type) ||
          strcasecmp(sym, tab[i].name) != 0))
    i++;
  if (tab[i].name)
    {
      if (addr)
        *addr= tab[i].addr;
      return(DD_TRUE);
    }
  return(DD_FALSE);
}

char *
cl_uc::symbolic_bit_name(t_addr bit_address,
                         class cl_memory *mem,
                         t_addr mem_addr,
                         t_mem bit_mask)
{
  char *sym_name= 0;
  int i;

  i= 0;
  while (bit_tbl()[i].name &&
         (bit_tbl()[i].addr != bit_address))
    i++;
  if (bit_tbl()[i].name)
    {
      sym_name= strdup(bit_tbl()[i].name);
      return(sym_name);
    }

  if (mem &&
      mem->have_real_name() &&
      strstr(mem->get_name(), "sfr") == mem->get_name())
    {
      i= 0;
      while (sfr_tbl()[i].name &&
             (sfr_tbl()[i].addr != mem_addr))
        i++;
      if (sfr_tbl()[i].name)
        sym_name= strdup(sfr_tbl()[i].name);
      else
        sym_name= 0;
    }
  if (!sym_name)
    {
      sym_name= (char *)malloc(16);
      sprintf(sym_name, mem?(mem->addr_format):"0x%06x", mem_addr);
    }
  sym_name= (char *)realloc(sym_name, strlen(sym_name)+2);
  strcat(sym_name, ".");
  i= 0;
  while (bit_mask > 1)
    {
      bit_mask>>=1;
      i++;
    }
  char bitnumstr[10];
  sprintf(bitnumstr, "%1d", i);
  strcat(sym_name, bitnumstr);
  return(sym_name);
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
  return(DD_FALSE);
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
  if (hws)
    hws->address_space_added(as);
  else
    printf("JAJ uc\n");//FIXME
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
  bool must_stop= DD_FALSE;

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
      if (hw->flags & HWF_INSIDE)
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
cl_uc::get_counter(char *nam)
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
cl_uc::add_counter(class cl_ticker *ticker, char */*nam*/)
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
cl_uc::del_counter(char *nam)
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
      rom)
    {
      if (rom->get_cell_flag(PC, CELL_FETCH_BRK) &&
          (brk= fbrk->get_bp(PC, &idx)) &&
          (brk->do_hit()))
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
  TYPE_UWORD  PCsave;
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
    }
  if (res != resGO)
    sim->stop(res);
  return(res);
}

void
cl_uc::pre_inst(void)
{
  inst_exec= DD_TRUE;
  inst_ticks= 0;
  events->disconn_all();
}

int
cl_uc::exec_inst(void)
{
  instPC= PC;
  return(resGO);
}

void
cl_uc::post_inst(void)
{
  tick_hw(inst_ticks);
  if (errors->count)
    check_errors();
  if (events->count)
    check_events();
  inst_exec= DD_FALSE;
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

int
cl_uc::clock_per_cycle(void)
{
  return(1);
}


/*
 * Stack tracking system
 */

void
cl_uc::stack_write(class cl_stack_op *op)
{
  if (op->get_op() & stack_read_operation)
    {
      class cl_error_stack_tracker_wrong_handle *e= new
        cl_error_stack_tracker_wrong_handle(DD_FALSE);
      //fprintf(stderr, "%06"_A_"x cl_uc::stack_read() should be called for "
      //"%s\n", op->get_pc(), op->get_op_name());
      e->init();
      error(e);
      return;
    }
  stack_ops->push(op);
}

void
cl_uc::stack_read(class cl_stack_op *op)
{
  class cl_stack_op *top= (class cl_stack_op *)(stack_ops->top());

  if (op->get_op() & stack_write_operation)
    {
      class cl_error_stack_tracker_wrong_handle *e= new
        cl_error_stack_tracker_wrong_handle(DD_TRUE);
      e->init();
      error(e);
      //fprintf(stderr, "%06"_A_"x cl_uc::stack_write() should be called for "
      //"%s\n", op->get_pc(), op->get_op_name());
      return;
    }
  if (!top)
    {
      class cl_error *e= new cl_error_stack_tracker_empty(op);
      /*printf("0x%06"_A_"x %s operation on stack but no operation was before\n
        ",
        op->get_pc(), op->get_op_name());*/
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
          /*printf("0x%06"_A_"x %s operation on stack but last was %s\n",
            op->get_pc(), op->get_op_name(), top->get_op_name());*/
        }
      int top_size= top->data_size(), op_size= op->data_size();
      if (top_size != op_size)
        {
          application->debug("0x%06"_A_"x %d bytes to read out of stack "
                             "but %d was pushed in last operation\n",
                             op->get_pc(), op_size, top_size);
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
      application->debug("0x%06"_A_"x %d ops removed from stack-tracker "
                         "when %s happened, top pc=0x%06"_A_"x "
                         "top before=0x%06"_A_"x op after=0x%06"_A_"x\n",
                         op->get_pc(), removed, op->get_op_name(),
                         top?(top->get_pc()):0, top?(top->get_before()):0,
                         op->get_after());
    }

  if (top)
    {
      int ta= top->get_after(), oa= op->get_after();
      if (ta != oa)
        {
          application->debug("0x%06"_A_"x stack still inconsistent after %s, "
                             "%d byte(s) should be read out; top after"
                             "=0x%06"_A_"x op after=0x%06"_A_"x\n",
                             op->get_pc(), op->get_op_name(), abs(ta-oa),
                             ta, oa);
          class cl_error *e=
            new cl_error_stack_tracker_inconsistent(op, abs(ta-oa));
          e->init();
          error(e);
        }
    }

  delete op;
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
      return(DD_TRUE);
    }
  else if ((bp= brk_by_nr(ebrk, nr)))
    {
      ebrk->del_bp(ebrk->index_of(bp), 0);
      return(DD_TRUE);
    }
  return(DD_FALSE);
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
  for (i= 0; i < events->count; i++)
    {
      class cl_ev_brk *brk=
        dynamic_cast<class cl_ev_brk *>(events->object_at(i));
      sim->stop(brk);
    }
  sim->stop(resBREAKPOINT);
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
  c->dd_printf("%s: unknown instruction code at ", get_type_name());
  if (uc->rom)
    {
      c->dd_printf(uc->rom->addr_format, PC);
      c->dd_printf(" (");
      c->dd_printf(uc->rom->data_format, uc->rom->get(PC));
      c->dd_printf(")");
    }
  else
    c->dd_printf("0x%06x", PC);
  c->dd_printf("\n");
}

cl_uc_error_registry::cl_uc_error_registry(void)
{
  class cl_error_class *prev = uc_error_registry.find("non-classified");
  prev = register_error(new cl_error_class(err_error, "unknown_code", prev, ERROR_OFF));
}


/* End of uc.cc */
