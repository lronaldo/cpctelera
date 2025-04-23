/*
 * Simulator of microcontrollers (i8085.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include <ctype.h>

#include "utils.h"

#include "i8085cl.h"


/*
 * Flags
 */

t_mem
cl_flag85_op::write(t_mem val)
{
  val&= ~0x08;
  return val;
}


/*
 * CPU
 */

cl_i8085::cl_i8085(class cl_sim *asim):
  cl_i8080(asim)
{
}

int
cl_i8085::init(void)
{
  cl_i8080::init();
  fill_def_wrappers(itab);
  //set_xtal(1000000);
  return 0;
}


const char *
cl_i8085::id_string(void)
{
  return "i8085";
}

void
cl_i8085::reset(void)
{
  cl_i8080::reset();
}

void
cl_i8085::set_PC(t_addr addr)
{
  PC= addr;
}


void
cl_i8085::mk_hw_elements(void)
{
  //class cl_hw *h;
  cl_i8080::mk_hw_elements();
}


void
cl_i8085::make_cpu_hw(void)
{
  cpu= new cl_i8085_cpu(this);
  add_hw(cpu);
  cpu->init();
}

void
cl_i8085::make_memories(void)
{
  /*
  class cl_address_space *as;
  class cl_address_decoder *ad;
  class cl_memory_chip *chip;
  */
  
  cl_i8080::make_memories();
}

class cl_memory_operator *
cl_i8085::make_flag_op(void)
{
  class cl_memory_operator *o;
  class cl_cell8 *c8;
  class cl_memory_cell *c;
  c8= &cF;
  c= (class cl_memory_cell *)c8;
  o= new cl_flag85_op(c);
  o->init();
  o->set_name("8085_flag_operator");
  return o;
}

struct dis_entry *
cl_i8085::get_dis_entry(t_addr addr)
{
  t_mem code= rom->get(addr);
  struct dis_entry *de;
  
  for (de= disass_8085; de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
        return de;
    }
  for (de= disass_common; de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
        return de;
    }

  return NULL;
}

char *
cl_i8085::disassc(t_addr addr, chars *comment)
{
  return cl_i8080::disassc(addr, comment);
}

void
cl_i8085::print_regs(class cl_console_base *con)
{
  con->dd_color("answer");
  con->dd_printf("SZXA-PVC  Flags= 0x%02x %3d %c  ",
                 rF, rF, isprint(rF)?rF:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
                 rA, rA, isprint(rA)?rA:'.');
  con->dd_printf("%s\n", cbin(rF, 8).c_str());
  con->dd_printf("BC= 0x%04x [BC]= 0x%02x %3d %c\n",
                 rBC, rom->get(rBC), rom->get(rBC),
                 isprint(rom->get(rBC))?rom->get(rBC):'.');
  con->dd_printf("DE= 0x%04x [DE]= 0x%02x %3d %c\n",
                 rDE, rom->get(rDE), rom->get(rDE),
                 isprint(rom->get(rDE))?rom->get(rDE):'.');
  con->dd_printf("HL= 0x%04x [HL]= 0x%02x %3d %c\n",
                 rHL, rom->get(rHL), rom->get(rHL),
                 isprint(rom->get(rHL))?rom->get(rHL):'.');

  int i;
  con->dd_cprintf("answer", "SP= ");
  con->dd_cprintf("dump_address", "0x%04x ->", rSP);
  for (i= 0; i < 2*12; i+= 2)
    {
      t_addr al, ah;
      al= (rSP+i)&0xffff;
      ah= (al+1)&0xffff;
      con->dd_cprintf("dump_number", " %02x%02x",
		      (u8_t)(rom->read(al)),
		      (u8_t)(rom->read(ah)));
    }
  con->dd_printf("\n");
  
  print_disass(PC, con);
}



int
cl_i8085::inx(class cl_memory_cell &op)
{
  u16_t r= op.get();
  rF&= ~flagK;
  if (r == 0xffff) rF|= flagK;
  r++;
  op.W(r);
  cF.W(rF);
  return resGO;
}

int
cl_i8085::dcx(class cl_memory_cell &op)
{
  u16_t r= op.get();
  rF&= ~flagK;
  if (!r) rF|= flagK;
  r--;
  op.W(r);
  cF.W(rF);
  return resGO;
}

int
cl_i8085::RIM(t_mem code)
{
  // TODO
  return resGO;
}

int
cl_i8085::SIM(t_mem code)
{
  // TODO
  return resGO;
}

int
cl_i8085::ARHL(t_mem code)
{
  rF&= ~flagC;
  if (rL&1) rF|= flagC;
  i16_t v= rHL;
  v>>= 1;
  cHL.W(v);
  cF.W(rF);
  return resGO;
}

int
cl_i8085::RDEL(t_mem code)
{
  u8_t oldC= rF&flagC;
  u8_t newC= (rD&0x80)?flagC:0;
  rF&= ~(flagC|flagV);
  if (newC) rF|= flagC;
  // TODO: V=?
  if ((rD ^ (rD<<1)) & 0x80) rF|= flagV;
  rDE<<= 1;
  if (oldC) rE|= 1;
  cDE.W(rDE);
  cF.W(rF);
  return resGO;
}

int
cl_i8085::DSUB(t_mem code)
{
  u16_t a= rHL, b= rBC;
  u32_t r;
  b= ~b+1;
  r= a+b;
  rF&= ~fAll;
  if (r <= 0xffff) rF|= flagC;
  if (r & 0x8000) rF|= flagS;
  r&= 0xffff;
  if (!r) rF|= flagZ;
  if (((a&0xfff) + (b&0xfff)) > 0xfff) rF|= flagA;
  rF|= ptab[r>>8];
  cHL.W(r);
  cF.W(rF);
  return resGO;
}

int
cl_i8085::JNX5(t_mem code)
{
  return jmp_if(rF & flagX5);
}

int
cl_i8085::JX5(t_mem code)
{
  return jmp_if(!(rF & flagX5));
}

int
cl_i8085::RSTV(t_mem code)
{
  if (rF & flagV)
    {
      push2(PC);
      PC= 0x40;
      tick_shift= 8;
    }
  return resGO;
}

int
cl_i8085::LDHI(t_mem code)
{
  u16_t a= rHL + fetch();
  cDE.W(read_addr(rom, a));
  vc.rd+= 2;
  return resGO;
}

int
cl_i8085::LDSI(t_mem code)
{
  u16_t a= rSP + fetch();
  cDE.W(read_addr(rom, a));
  vc.rd+= 2;
  return resGO;
}

int
cl_i8085::LHLX(t_mem code)
{
  u16_t a= rDE;
  cHL.W(read_addr(rom, a));
  vc.rd+= 2;
  return resGO;
}

int
cl_i8085::SHLX(t_mem code)
{
  u16_t a= rDE;
  rom->write(a++, rL);
  rom->write(a  , rH);
  vc.wr+= 2;
  return resGO;
}


cl_i8085_cpu::cl_i8085_cpu(class cl_uc *auc):
  cl_hw(auc, HW_CPU, 0, "cpu")
{
}

int
cl_i8085_cpu::init(void)
{
  cl_hw::init();

  cl_var *v;
  uc->vars->add(v= new cl_var("sp_limit", cfg, i8085cpu_sp_limit,
			      cfg_help(i8085cpu_sp_limit)));
  v->init();

  return 0;
}

const char *
cl_i8085_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case i8085cpu_sp_limit:
      return "Stack overflows when SP is below this limit";
    }
  return "Not used";
}

t_mem
cl_i8085_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  class cl_i8085 *u= (class cl_i8085 *)uc;
  if (val)
    cell->set(*val);
  switch ((enum i8085cpu_confs)addr)
    {
    case i8085cpu_sp_limit:
      if (val)
	u->sp_limit= *val & 0xffff;
      else
	cell->set(u->sp_limit);
      break;
    case i8085cpu_nuof: break;
    }
  return cell->get();
}


/* End of i8085.src/i8085.cc */
