/*
 * Simulator of microcontrollers (mos65c02.cc)
 *
 * Copyright (C) 2020 Drotos Daniel
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

#include <stdlib.h>
#include <ctype.h>

#include "glob.h"

#include "mos65c02cl.h"


cl_mos65c02::cl_mos65c02(class cl_sim *asim):
  cl_mos6502(asim)
{
  *my_id= "MOS65C02";
}

int
cl_mos65c02::init(void)
{
  int i;
  cl_mos6502::init();
  // Map all 0x_3 into NOP 1,1
  for (i=0x13; i<=0xf3; i+= 0x10)
    itab[i]= instruction_wrapper_03;
  // Map all 0x_b into NOP 1,1
  for (i=0x0b; i<=0xfb; i+= 0x10)
    itab[i]= instruction_wrapper_03;
  return 0;
}

void
cl_mos65c02::reset(void)
{
  cl_mos6502::reset();
  CC&= ~flagD;
}

struct dis_entry *
cl_mos65c02::get_dis_entry(t_addr addr)
{
  struct dis_entry *de= cl_mos6502::get_dis_entry(addr);
  if (de != NULL)
    return de;

  t_mem code= rom->get(addr);
  for (de = disass_mos65c02; de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
        return de;
    }
  return NULL;
}

int
cl_mos65c02::inst_length(t_addr addr)
{
  struct dis_entry *de= get_dis_entry(addr);
  if (!de)
    return 1;
  return (de->mnemonic)?(de->length):1;
}


int
cl_mos65c02::accept_it(class it_level *il)
{
  class cl_it_src *is= il->source;

  tick(2);
  push_addr(PC);
  rom->write(0x0100 + rSP, rF|0x20);
  if (set_b)
    rF&= ~flagB;
  // All interrupts (incl BRK) clear D flag
  rF&= ~flagD;
  cSP.W(rSP-1);
  tick(1);
  vc.wr++;
  
  t_addr a= read_addr(rom, is->addr);
  tick(2);
  vc.rd+= 2;
  PC= a;

  rF|= flagI;
  is->clear();
  it_levels->push(il);
  
  return resGO;
}


int
cl_mos65c02::nopft(int nuof_fetches, int nuof_ticks)
{
  while (nuof_fetches--)
    fetch();
  if (nuof_ticks) tick(nuof_ticks);
  return resGO;
}

int
cl_mos65c02::BIT8(t_mem code)
{
  u8_t v= fetch();
  u8_t f= rF & ~(flagZ);
  if (!(rA & v)) f|= flagZ;
  cF.W(f);
  return resGO;
}

int
cl_mos65c02::DEA(t_mem code)
{
  cA.W(rA-1);
  rF&= ~(flagZ|flagS);
  if (!rA) rF|= flagZ;
  if (rA & 0x80) rF|= flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos65c02::INA(t_mem code)
{
  cA.W(rA+1);
  rF&= ~(flagZ|flagS);
  if (!rA) rF|= flagZ;
  if (rA & 0x80) rF|= flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos65c02::JMP7c(t_mem code)
{
  u16_t a1= i16();
  u16_t a2= a1 + rX;
  u16_t a= read_addr(rom, a2);
  PC= a;
  tick(5);
  return resGO;
}

int
cl_mos65c02::tsb(class cl_cell8 &op)
{
  u8_t o= op.R();
  u8_t r1= o & rA, r2= o | rA;
  u8_t f= rF & ~flagZ;
  if (!r1) f|= flagZ;
  op.W(r2);
  cF.W(f);
  tick(2);
  return resGO;
}

int
cl_mos65c02::trb(class cl_cell8 &op)
{
  u8_t o= op.R();
  u8_t r1= o & rA, r2= o & ~rA;
  u8_t f= rF & ~flagZ;
  if (!r1) f|= flagZ;
  op.W(r2);
  cF.W(f);
  tick(2);
  return resGO;
}

int
cl_mos65c02::stz(class cl_cell8 &op)
{
  op.W(0);
  return resGO;
}

int
cl_mos65c02::PHY(t_mem code)
{
  rom->write(0x0100 + rSP, rY);
  vc.wr++;
  t_addr spbef= rSP;
  cSP.W(rSP-1);
  class cl_stack_push *op= new cl_stack_push(instPC, rY, spbef, rSP);
  op->init();
  stack_write(op);
  tick(2);
  return resGO;
}

int
cl_mos65c02::PLY(t_mem code)
{
  t_addr spbef= rSP;
  cSP.W(rSP+1);
  cY.W(rom->read(0x0100 + rSP));
  class cl_stack_pop *op= new cl_stack_pop(instPC, rY, spbef, rSP);
  op->init();
  stack_read(op);
  u8_t f= rF & ~(flagN|flagZ);
  if (!rY) f|= flagZ;
  if (rY&0x80) f|= flagN;
  cF.W(f);
  vc.rd++;
  tick(3);
  return resGO;
}

int
cl_mos65c02::PHX(t_mem code)
{
  rom->write(0x0100 + rSP, rX);
  vc.wr++;
  t_addr spbef= rSP;
  cSP.W(rSP-1);
  class cl_stack_push *op= new cl_stack_push(instPC, rX, spbef, rSP);
  op->init();
  stack_write(op);
  tick(2);
  return resGO;
}

int
cl_mos65c02::PLX(t_mem code)
{
  t_addr spbef= rSP;
  cSP.W(rSP+1);
  cX.W(rom->read(0x0100 + rSP));
  class cl_stack_pop *op= new cl_stack_pop(instPC, rX, spbef, rSP);
  op->init();
  stack_read(op);
  u8_t f= rF & ~(flagN|flagZ);
  if (!rX) f|= flagZ;
  if (rX&0x80) f|= flagN;
  cF.W(f);
  vc.rd++;
  tick(3);
  return resGO;
}


/* End of mos6502.src/mos65c02.cc */
