/*
 * Simulator of microcontrollers (pdk.cc)
 *
 * Copyright (C) 2016 Drotos Daniel
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

//#include <ctype.h>
//#include <stdarg.h> /* for va_list */
//#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "i_string.h"

// prj
#include "globals.h"
//#include "pobjcl.h"

// sim
//#include "simcl.h"
#include "dregcl.h"
#include "simifcl.h"

// local
#include "glob.h"
#include "pdk16cl.h"
#include "t16cl.h"
#include "osccl.h"
#include "wdtcl.h"
//#include "portcl.h"
//#include "regspdk.h"

/*******************************************************************/

cl_fpp::cl_fpp(int aid, class cl_pdk *the_puc, class cl_sim *asim):
  cl_uc(asim)
{
  id= aid;
  puc= the_puc;
  PCmask= 0xfff;
}


/*
 * Base type of PDK controllers
 */

cl_fpp::cl_fpp(int aid, class cl_pdk *the_puc, struct cpu_entry *IType, class cl_sim *asim) : cl_uc(asim)
{
  id= aid;
  puc= the_puc;
  type = IType;
}

int cl_fpp::init(void) {
  cl_uc::init(); /* Memories now exist */

  //set_xtal(8000000);

  // rom = address_space(MEM_ROM_ID);
  // ram = mem(MEM_XRAM);
  // ram = rom;

  // zero out ram(this is assumed in regression tests)
  // for (int i = 0x0; i < 0x8000; i++) {
  //   ram->set((t_addr)i, 0);
  // }

  cA.init();
  cA.decode(&rA);

  if (puc)
    {
      chars n, d;
      n.format("A%d", id);
      d.format("Accumulator of FPP%d", id);
      puc->mk_cvar(&cA, n, d);
      if (id == 0)
	puc->mk_cvar(&cA, "A", "Accumulator");
      n.format("PC%d", id);
      d.format("Program counter of FPP%d", id);
      puc->mk_cvar(&cPC, n, d);
    }
  
  return (0);
}


void
cl_fpp::act(void)
{
  cSP->decode(&rSP);
  cF ->decode(&rF);
}

void cl_fpp::reset(void) {
  cl_uc::reset();
  sp_most = 0x00;

  PC = id;
  rA = 0;
  //for (t_addr i = 0; i < io_size; ++i) store_io(i, 0);
  rTMP= 0;
}

void cl_fpp::mk_hw_elements(void)
{
  // TODO: Add hardware stuff here.
  /*
  class cl_hw *h;
  cl_uc::mk_hw_elements();
  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();
  */
}

void cl_fpp::make_memories(void)
{
  class cl_address_space *as;
  int rom_storage= 0x2000, ram_storage= 0x200;
  int rom_width= 16;

  switch (type->type)
    {
    case CPU_PDK13: rom_width= 13; break;
    case CPU_PDK14: rom_width= 14; break;
    case CPU_PDK15: rom_width= 15; break;
    case CPU_PDK16: rom_width= 16; break;
    default: rom_width= 16;
    }
  
  if (puc != NULL)
    {
      ram= puc->ram;
      rom= puc->rom;
      sfr= puc->sfr;
    }
  else
    {
      rom = as = new cl_address_space("rom", 0, rom_storage, rom_width);
      as->init();
      address_spaces->add(as);
      ram = as = new cl_address_space("ram", 0, ram_storage, 8);
      as->init();
      address_spaces->add(as);
      sfr = as = new cl_address_space("regs8", 0, io_size, 8);
      as->init();
      address_spaces->add(as);
      
      class cl_address_decoder *ad;
      class cl_memory_chip *chip;
    
      chip = new cl_chip16("rom_chip", rom_storage, rom_width);
      chip->init();
      memchips->add(chip);
      
      ad = new cl_address_decoder(as = rom, chip, 0, rom_storage-1, 0);
      ad->init();
      as->decoders->add(ad);
      ad->activate(0);
      
      chip = new cl_chip16("ram_chip", ram_storage, 8);
      chip->init();
      memchips->add(chip);
      
      ad = new cl_address_decoder(as = ram, chip, 0, ram_storage-1, 0);
      ad->init();
      as->decoders->add(ad);
      ad->activate(0);
      
      chip = new cl_chip16("io_chip", io_size, 8);
      chip->init();
      memchips->add(chip);
      
      ad = new cl_address_decoder(as = sfr, chip, 0, io_size-1, 0);
      ad->init();
      as->decoders->add(ad);
      ad->activate(0);
   
      // extra byte of the IO memory will point to the A register just for the debugger
      sfr->get_cell(io_size)->decode(&(rA));
    }
  
  cSP= sfr->get_cell(2);
  cF = sfr->get_cell(0);
  act();
}


void
cl_fpp::build_cmdset(class cl_cmdset *cmdset)
{
  if (puc == NULL)
    cl_uc::build_cmdset(cmdset);
}


/*
 * Help command interpreter
 */
/*
struct dis_entry *cl_fppa::dis_tbl(void) {
  switch (type->type) {
    case CPU_PDK13:
      return (disass_pdk_13);
  case CPU_PDK14:
    return (disass_pdk_14);
  case CPU_PDK15:
    return (disass_pdk_15);
  default:
    return NULL;//__builtin_unreachable();
  }
}
*/

int cl_fpp::inst_length(t_addr /*addr*/) {
  return 1;
}

int cl_fpp::inst_branch(t_addr addr) {
  int b;

  get_disasm_info(addr, NULL, &b, NULL, NULL);

  return b;
}

bool cl_fpp::is_call(t_addr addr) {
  struct dis_entry *e;

  get_disasm_info(addr, NULL, NULL, NULL, &e);

  return e ? (e->is_call) : false;
}

int cl_fpp::longest_inst(void) { return 1; }

const char *cl_fpp::get_disasm_info(t_addr addr, int *ret_len, int *ret_branch,
                                    int *immed_offset,
                                    struct dis_entry **dentry) {
  const char *b = NULL;
  int immed_n = 0;
  int start_addr = addr;
  struct dis_entry *instructions= dis_tbl();
  /*
  switch (type->type) {
      case CPU_PDK13:
        instructions = disass_pdk_13;
        break;
      case CPU_PDK14:
        instructions = disass_pdk_14;
        break;
      case CPU_PDK15:
        instructions = disass_pdk_15;
        break;
      default:
        return "";//__builtin_unreachable();
  }
  */
  uint code = rom->get(addr++);
  int i = 0;
  while ((code & instructions[i].mask) != instructions[i].code &&
         instructions[i].mnemonic)
    i++;
  dis_entry *dis_e = &instructions[i];
  b = instructions[i].mnemonic;

  if (ret_branch) {
    *ret_branch = dis_e->branch;
  }

  if (immed_offset) {
    if (immed_n > 0)
      *immed_offset = immed_n;
    else
      *immed_offset = (addr - start_addr);
  }

  if (ret_len) *ret_len = 1;

  if (dentry) *dentry = dis_e;

  return b;
}
/*
char *cl_fppa::disass(t_addr addr)
{
  chars work, temp;
  const char *b;
  int len = 0;
  int immed_offset = 0;
  struct dis_entry *dis_e;
  bool first= true;
  
  work= "";

  b = get_disasm_info(addr, &len, NULL, &immed_offset, &dis_e);

  if (b == NULL)
    {
      return (strdup("UNKNOWN/INVALID"));
    }

  while (*b)
    {
      if ((*b == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (*b == '%')
	{
	  temp= "";
	  b++;
	  uint code = rom->get(addr) & ~(uint)dis_e->mask;
	  switch (*(b++))
	    {
	    case 'k':  // k    immediate addressing
	      temp.format("#0x%x", code);
	      break;
	    case 'm':  // m    memory addressing
	      if (*b == 'n')
		{
		  code &= m_mask();
		  b++;
		}
	      temp.format("0x%x", code);
	      break;
	    case 'i':  // i    IO addressing
	      // TODO: Maybe add pretty printing.
	      if (*b == 'n')
		{
		  code &= io_mask();
		  ++b;
		}
	      temp.format("[0x%x]", code);
	      break;
	    case 'n':  // n    N-bit addressing
	      uint n;
	      switch (type->type) {
	      case CPU_PDK13:
		n = (code & 0xE0) >> 5;
		break;
	      case CPU_PDK14:
		n = (code & 0x1C0) >> 6;
		break;
	      case CPU_PDK15:
		n = (code & 0x380) >> 7;
		break;
	      default:
		n= 0;//__builtin_unreachable();
	      }
	      temp.format("#0x%x", n);
	      break;
	    default:
	      temp= "%?";
	      break;
	    }
	  work+= temp;
	}
      else
	work+= *(b++);
    }

  return strdup(work.c_str());
}
*/

struct dis_entry *
cl_fpp::get_dis_entry(t_addr addr)
{
  t_mem code;
  code= rom->get(addr);
  int i;
  i= 0;
  while ((code & dis_tbl()[i].mask) != dis_tbl()[i].code &&
	 dis_tbl()[i].mnemonic)
    i++;
  return &dis_tbl()[i];
}

char *
cl_fpp::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars(), fmt= chars();
  const char *b;
  t_mem code;
  int i;
  bool first;
  struct dis_entry *de;

  code= rom->get(addr);
  de= get_dis_entry(addr);
  if (!de || !de->mnemonic)
    return strdup("-- UNKNOWN/INVALID");

  b= de->mnemonic;
  
  first= true;
  work= "";
  for (i=0; b[i]; i++)
    {
      if ((b[i] == ' ') && first)
	{
	  first= false;
	  while (work.len() < 9) work.append(' ');
	}
      if (b[i] == '\'')
	{
	  fmt= "";
	  i++;
	  while (b[i] && (b[i]!='\''))
	    fmt.append(b[i++]);
	  if (!b[i]) i--;
	  if (fmt.empty())
	    work.append("'");
	  if (fmt == "i5")
	    work.appendf("0x%02x", code & 0x1f);
	  if (fmt == "m4")
	    work.appendf("[0x%04x]", code & 0xf);
	  if (fmt == "m6")
	    work.appendf("[0x%04x]", code & 0x3f);
	  if (fmt == "m7")
	    work.appendf("[0x%04x]", code & 0x7f);
	  if (fmt == "m8")
	    work.appendf("[0x%04x]", code & 0xff);
	  if (fmt == "M5")
	    work.appendf("[0x%04x]", code & 0x1f & ~1);
	  if (fmt == "M7")
	    work.appendf("[0x%04x]", code & 0x7f & ~1);
	  if (fmt == "M8")
	    work.appendf("[0x%04x]", code & 0xff & ~1);
	  if (fmt == "M9")
	    work.appendf("[0x%04x]", code & 0x1ff & ~1);
	  if (fmt == "n5")
	    work.appendf("%d", (code>>5)&7);
	  if (fmt == "n6")
	    work.appendf("%d", (code>>6)&7);
	  if (fmt == "n7")
	    work.appendf("%d", (code>>7)&7);
	  if (fmt == "n9")
	    work.appendf("%d", (code>>9)&7);
	  if (fmt == "a8")
	    work.appendf("0x%04x", code & 0xff);
	  if (fmt == "a10")
	    work.appendf("0x%04x", code & 0x3ff);
	  if (fmt == "a11")
	    work.appendf("0x%04x", code & 0x7ff);
	  if (fmt == "a12")
	    work.appendf("0x%04x", code & 0xfff);
	  if (fmt == "a13")
	    work.appendf("0x%04x", code & 0x1fff);
	  if (fmt == "d4")
	    work.appendf("%d", code & 0xf);
	  if (fmt == "d5")
	    work.appendf("%d", code & 0x1f);
	  if (comment && temp.nempty())
	    comment->append(temp);
	  continue;
	}
      if (b[i] == '%')
	{
	  i++;
	  temp= "";
	  switch (b[i])
	    {
	    case 'm':
	      work.appendf("[0x%04x]", code & m_mask());
	      break;
	    case 'M':
	      work.appendf("[0x%04x]", code & m_mask() & ~1);
	      break;
	    case 'k':
	      work.appendf("#0x%02x", code & 0xff);
	      break;
	    case 'i':
	      work.appendf("0x%02x", code & io_mask());
	      break;
	    case 'n': // == n5
	      work.appendf("%d", (code>>5)&7);
	      break;
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }

  return strdup(work.c_str());
}

void
cl_fpp::print_regs(class cl_console_base *con)
{
  act();
  con->dd_color("answer");
  con->dd_printf("A = %02x %3u\n", rA, rA);
  con->dd_printf("       OACZ\n");
  con->dd_printf("F = %02x ", rF);
  con->dd_printf("%d%d%d%d\n", fO, fA, fC, fZ);
  con->dd_printf("SP= %02x\n", rSP);
  print_disass(PC, con);
}

/*
 * Execution
 */

int cl_fpp::exec_inst(void)
{
  t_mem code;

  act();
  instPC= PC;
  if (fetch(&code)) {
    return (resBREAKPOINT);
  }
  tick(1);

  int status = execute(code);
  if (status == resINV_INST)
    {
      return (resINV_INST);
    }
  return (status);
}


void
cl_fpp::push(u16_t word)
{
  u8_t b= rSP;
  ram->write(rSP, word);
  ram->write(rSP+1, word>>8);
  cSP->W(rSP+2);
  vc.wr+= 2;
  stack_check_overflow(b);
}

void
cl_fpp::pushlh(u8_t low, u8_t high)
{
  u8_t b= rSP;
  ram->write(rSP, low);
  ram->write(rSP+1, high);
  cSP->W(rSP+2);
  vc.wr+= 2;
  stack_check_overflow(b);
}

void
cl_fpp::stack_check_overflow(void)
{
  if (0)
    {
      class cl_stack_op *op;
      op= new cl_stack_op(stack_push, instPC, rSP-2, rSP);
      class cl_error_stack_overflow *e=
	new cl_error_stack_overflow(op);
      e->init();
      error(e);
    }
}

void
cl_fpp::stack_check_overflow(t_addr sp_before)
{
  if (0)
    {
      class cl_stack_op *op;
      op= new cl_stack_op(stack_push, instPC, sp_before, rSP);
      class cl_error_stack_overflow *e=
	new cl_error_stack_overflow(op);
      e->init();
      error(e);
    }
}

/****************************************************************************/


/* Set nr of active FPP */

t_mem
cl_act_cell::write(t_mem val)
{
  val= puc->set_act(val);
  return cl_pdk_cell::write(val);
}

/* Set nr of FPPs */

t_mem
cl_nuof_cell::write(t_mem val)
{
  val= puc->set_nuof(val);
  return cl_pdk_cell::write(val);
}


cl_fppen_op::cl_fppen_op(class cl_pdk *the_puc, class cl_memory_cell *acell):
  cl_memory_operator(acell)
{
  puc= the_puc;
}

t_mem
cl_fppen_op::write(t_mem val)
{
  return puc->set_fppen(val);
}


t_mem
cl_xtal_writer::write(t_mem val)
{
  u32_t u= puc->osc->frsys;
  puc->set_xtal(u);
  return u;
}


cl_mulrh_op::cl_mulrh_op(class cl_pdk *the_puc, class cl_memory_cell *acell):
  cl_memory_operator(acell)
{
  puc= the_puc;
}

t_mem
cl_mulrh_op::read(void)
{
  return puc->rMULRH;
}


/*
 * PDK uc
 */

cl_pdk::cl_pdk(struct cpu_entry *IType, class cl_sim *asim):
  cl_uc(asim)
{
  int i;
  type = IType;
  for (i= 0; i<8; i++)
    fpps[i]= NULL;
}

int
cl_pdk::init(void)
{
  cl_uc::init();
  class cl_memory_operator *op;
  fpps[0]= mk_fpp(0);

  cFPPEN= sfr->get_cell(1);
  cFPPEN->decode(&rFPPEN);
  op= new cl_fppen_op(this, cFPPEN);
  op->init();
  cFPPEN->append_operator(op);

  op= new cl_mulrh_op(this, sfr->get_cell(9));
  sfr->get_cell(9)->append_operator(op);
  
  mk_mvar(sfr, 0, "FLAG", "ACC Status Flag Register");
  mk_mvar(sfr, 0, "F", "ACC Status Flag Register");
  mk_mvar(sfr, 1, "FPPEN", "FPP unit Enable Register");
  mk_mvar(sfr, 2, "SP", "Stack Pointer Register");
  mk_mvar(sfr, 3, "CLKMD", "Clock Mode Register");
  mk_mvar(sfr, 5, "INTRQ", "Interrupt Request Register");
  mk_mvar(sfr, 6, "T16M", "Timer16 Mode Register");
  mk_mvar(sfr, 8, "MULOP", "Multplier Operand Register");
  mk_mvar(sfr, 8, "MISC", "MISC Register");
  mk_mvar(sfr, 9, "MULRH", "Multplier Result High Byte Register");
  mk_mvar(sfr, 0xa, "EOSCR", "External Oscillator Setting Register");
  mk_mvar(sfr, 0xc, "INTEGS", "Interrupt Edge Select Register");
  
  cact= new cl_act_cell(this);
  reg_cell_var(cact, &act, "fpp", "ID of actual FPPA");
  nuof_fpp= 1;
  cnuof_fpp= new cl_nuof_cell(this);
  reg_cell_var(cnuof_fpp, &nuof_fpp, "nuof_fpp", "Number of FPPs");

  if (type->type == CPU_PDKX)
    {
      fpps[1]= mk_fpp(1);
      fpps[2]= mk_fpp(2);
      fpps[3]= mk_fpp(3);
      fpps[4]= mk_fpp(4);
      fpps[5]= mk_fpp(5);
      fpps[6]= mk_fpp(6);
      fpps[7]= mk_fpp(7);
      nuof_fpp= 8;
    }
  
  act= 0;
  rFPPEN= 1;
  single= true;
  cPC.decode(&(fpps[0]->PC));

  return 0;
}

const char *
cl_pdk::id_string(void)
{
  switch (type->type)
    {
    case CPU_PDK13:
      return("pdk13");
    case CPU_PDK14:
      return("pdk14");
    case CPU_PDK15:
      return("pdk15");
    case CPU_PDK16:
      return("pdk16");
    default:
      return("pdk");
  }
}

void
cl_pdk::make_memories(void)
{
  class cl_address_space *as;
  class cl_address_decoder *ad;
  class cl_memory_chip *chip;
  int rom_size= 0x2000, ram_size=0x200;

  rom = as = new cl_address_space("rom", 0, rom_size, 16);
  as->init();
  address_spaces->add(as);
  ram = as = new cl_address_space("ram", 0, ram_size, 8);
  as->init();
  address_spaces->add(as);
  sfr = as = new cl_address_space("regs8", 0, io_size, 8);
  as->init();
  address_spaces->add(as);

  chip = new cl_chip16("rom_chip", rom_size, 16);
  chip->init();
  memchips->add(chip);

  ad = new cl_address_decoder(as = rom, chip, 0, rom_size-1, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip = new cl_chip16("ram_chip", ram_size, 8);
  chip->init();
  memchips->add(chip);
  
  ad = new cl_address_decoder(as = ram, chip, 0, ram_size-1, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
    
  chip = new cl_chip16("io_chip", io_size, 8);
  chip->init();
  memchips->add(chip);

  ad = new cl_address_decoder(as = sfr, chip, 0, io_size-1, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}

void
cl_pdk::mk_hw_elements(void)
{
  //class cl_hw *h;
  cl_uc::mk_hw_elements();

  add_hw(osc= new cl_osc(this, "osc"));
  osc->init();

  add_hw(t16= new cl_t16(this, "t16"));
  t16->init();

  add_hw(wdt= new cl_wdt(this, "wdt"));
  wdt->init();

  class cl_memory_cell *c;
  class cl_hw *simif= get_hw("simif", 0);
  if (simif)
    {
      c= simif->cfg_cell(simif_xtal);
      c->prepend_operator(new cl_xtal_writer(this, c));
    }
}

class cl_fpp *
cl_pdk::mk_fpp(int id)
{
  class cl_fpp *fppa;
  switch (type->type)
    {
    case CPU_PDK13: fppa= new cl_fpp13(id, this, sim); break;
    case CPU_PDK14: fppa= new cl_fpp14(id, this, sim); break;
    case CPU_PDK15: fppa= new cl_fpp15(id, this, sim); break;
    case CPU_PDK16: fppa= new cl_fpp16(id, this, sim); break;
    case CPU_PDKX:  fppa= new cl_fpp15(id, this, sim); break;
    default: fppa= new cl_fpp14(id, this, sim); break;
    }  
  fppa->init();
  return fppa;
}

void
cl_pdk::reset(void)
{
  int i;
  mode= pm_run;
  for (i=0; i<nuof_fpp; i++)
    fpps[i]->reset();
  
  instPC= PC= 0;
  state = stGO;
  ticks->set(0, 0);
  isr_ticks->set(0, 0);
  idle_ticks->set(0, 0);
  halt_ticks->set(0, 0);
  vc.inst= vc.fetch= vc.rd= vc.wr= 0;

  stack_ops->free_all();

  for (i= 0; i < hws->count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(hws->at(i));
      hw->reset();
    }
}

u8_t
cl_pdk::set_fppen(u8_t val)
{
  int i, n;
  u8_t m;
  if (val == 0)
    val= 1;
  for (i=0, m=1, n=0; i<8; i++, m<<=1)
    {
      if (fpps[i] == NULL)
	val&= ~m;
      else
	n++;
    }
  single= n==1;
  return val;
}

u8_t
cl_pdk::set_act(u8_t val)
{
  u8_t r= 0;
  if (val < nuof_fpp)
    r= val;
  cPC.decode(&(fpps[act]->PC));
  PC= fpps[act]->PC;
  return r;
}

u8_t
cl_pdk::set_nuof(u8_t val)
{
  int i;
  if (val > 8)
    val= 8;
  if (val<1)
    val= 1;
  for (i=0; i<8; i++)
    {
      if (i<val)
	{
	  if (fpps[i] == NULL)
	    fpps[i]= mk_fpp(i);
	  else
	    fpps[i]->reset();
	}
      else
	{
	  if (fpps[i] != NULL)
	    {
	      delete fpps[i];
	      fpps[i]= NULL;
	    }
	}
    }
  if (rFPPEN == 0)
    set_fppen(1);
  else
    set_fppen(rFPPEN);
  return val;
}

t_addr
cl_pdk::get_pc(int id)
{
  if (id >= nuof_fpp)
    return 0;
  return fpps[id]->PC;
}

void
cl_pdk::set_pc(int id, t_addr new_pc)
{
  if (id >= nuof_fpp)
    return;
  fpps[id]->cPC.W(new_pc);
}


int
cl_pdk::exec_inst(void)
{
  int it, ret= resHALT;
  if (mode == pm_pd)
    return resHALT;
  while (!(rFPPEN & (1<<act)))
    act= (act+1)%nuof_fpp;
  inst_ticks= 0;
  if (mode == pm_run)
    {
      fpps[act]->pre_inst();
      ret= fpps[act]->exec_inst();
      fpps[act]->post_inst();
      it= inst_ticks= fpps[act]->inst_ticks;
      tick(it);
      inst_ticks= it;
      int i;
      vc.inst= vc.fetch= vc.rd= vc.wr= 0;
      for (i= 0; i<nuof_fpp; i++)
	{
	  vc.inst+= fpps[i]->vc.inst;
	  vc.fetch+= fpps[i]->vc.fetch;
	  vc.rd+= fpps[i]->vc.rd;
	  vc.wr+= fpps[i]->vc.wr;
	}
      if (rFPPEN != 1)
	{
	  do
	    act= (act+1)%nuof_fpp;
	  while (!(rFPPEN & (1<<act)));
	  cPC.decode(&(fpps[act]->PC));
	}
    }
  PC= fpps[act]->PC;
  return ret;
}


char *
cl_pdk::disassc(t_addr addr, chars *comment)
{
  return fpps[0]->disassc(addr, comment);
}


void
cl_pdk::print_regs(class cl_console_base *con)
{
  int i;
  
  for (i= 0; i<nuof_fpp; i++)
    {
      //con->dd_color((i==act)?"result":"answer");
      if (rFPPEN & (1<<i))
	con->dd_cprintf("ui_run", "FPP%d:EN   ", i);
      else
	con->dd_cprintf("ui_stop", "FPP%d:DIS  ", i);
    }
  con->dd_printf("\n");
  for (i= 0; i<nuof_fpp; i++)
    {
      con->dd_color((i==act)?"result":"answer");
      con->dd_printf("A=%02x %3u  ", fpps[i]->rA, fpps[i]->rA);
    }
  con->dd_printf("\n");
  for (i= 0; i<nuof_fpp; i++)
    {
      con->dd_color((i==act)?"result":"answer");
      con->dd_printf("  OACZ    ");
    }
  con->dd_printf("\n");
  for (i= 0; i<nuof_fpp; i++)
    {
      con->dd_color((i==act)?"result":"answer");
      con->dd_printf("F=", fpps[i]->rF);
      con->dd_printf("%d%d%d%d    ",
		     ((fpps[i]->rF&BIT_OV)>>BITPOS_OV),
		     ((fpps[i]->rF&BIT_AC)>>BITPOS_AC),
		     ((fpps[i]->rF&BIT_C )>>BITPOS_C ),
		     ((fpps[i]->rF&BIT_Z )>>BITPOS_Z ));
    }
  con->dd_printf("\n");
  for (i= 0; i<nuof_fpp; i++)
    {
      con->dd_color((i==act)?"result":"answer");
      con->dd_printf("SP=%02x     ", fpps[i]->rSP);
    }
  con->dd_printf("\n");

  for (i=0; i<nuof_fpp; i++)
    {
      con->dd_color((i==act)?"result":"answer");
      con->dd_printf("%cFPP%d%c: ", (rFPPEN&(1<<act))?'+':'-', i, (act==i)?'*':' ');
      fpps[0]->print_disass(fpps[i]->PC, con);
    }
}


/* End of pdk.src/pdk.cc */
