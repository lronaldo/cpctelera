/*
 * Simulator of microcontrollers (inst.cc)
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

#include "globals.h"

#include "mos6502cl.h"

int
cl_mos6502::NOP(t_mem code)
{
  return resGO;
}

int
cl_mos6502::BRK(t_mem code)
{
  set_b= true;
  PC= (PC+1)&0xffff;//fetch();
  cF.W(rF|flagB);
  src_brk->request();
  return resGO;
}

int
cl_mos6502::RTI(t_mem code)
{
  u8_t f;
  cSP.W(rSP+1);
  f= rom->read(0x0100 + rSP);
  f&= ~(0x20|flagB);
  cF.W(f);
  vc.rd+= 1;
  /*cSP.W(rSP+1);
  l= rom->read(0x0100 + rSP);
  cSP.W(rSP+1);
  h= rom->read(0x0100 + rSP);*/
  PC= pop_addr();//h*256 + l;
  {
    class it_level *il= (class it_level *)(it_levels->top());
    if (il &&
	il->level >= 0)
      {
	il= (class it_level *)(it_levels->pop());
	delete il;
      }
  }
  tick(3);
  return resGO;
}

int
cl_mos6502::CLI(t_mem code)
{
  cF.W(rF&= ~flagI);
  tick(1);
  return resGO;
}

int
cl_mos6502::SEI(t_mem code)
{
  cF.W(rF|= flagI);
  tick(1);
  return resGO;
}

int
cl_mos6502::PHP(t_mem code)
{
  u8_t v= rF|0x20|flagB;
  rom->write(0x0100 + rSP, v);
  vc.wr++;
  t_addr spbef= rSP;
  cSP.W(rSP-1);
  class cl_stack_push *op= new cl_stack_push(instPC, v, spbef, rSP);
  op->init();
  stack_write(op);
  tick(2);
  return resGO;
}

int
cl_mos6502::CLC(t_mem code)
{
  cF.W(rF&= ~flagC);
  tick(1);
  return resGO;
}

int
cl_mos6502::PLP(t_mem code)
{
  t_addr spbef= rSP;
  cSP.W(rSP+1);
  u8_t v= rom->read(0x0100 + rSP);
  v&= ~(0x20|flagB);
  class cl_stack_pop *op= new cl_stack_pop(instPC, v, spbef, rSP);
  op->init();
  stack_read(op);
  cF.W(v);
  vc.rd++;
  tick(3);
  return resGO;
}

int
cl_mos6502::SEc(t_mem code)
{
  cF.W(rF|= flagC);
  tick(1);
  return resGO;
}

int
cl_mos6502::PHA(t_mem code)
{
  rom->write(0x0100 + rSP, rA);
  vc.wr++;
  t_addr spbef= rSP;
  cSP.W(rSP-1);
  class cl_stack_push *op= new cl_stack_push(instPC, rA, spbef, rSP);
  op->init();
  stack_write(op);
  tick(2);
  return resGO;
}

int
cl_mos6502::PLA(t_mem code)
{
  t_addr spbef= rSP;
  cSP.W(rSP+1);
  cA.W(rom->read(0x0100 + rSP));
  class cl_stack_pop *op= new cl_stack_pop(instPC, rA, spbef, rSP);
  op->init();
  stack_read(op);
  u8_t f= rF & ~(flagN|flagZ);
  if (!rA) f|= flagZ;
  if (rA&0x80) f|= flagN;
  cF.W(f);
  vc.rd++;
  tick(3);
  return resGO;
}

int
cl_mos6502::CLV(t_mem code)
{
  cF.W(rF&= ~flagV);
  tick(1);
  return resGO;
}

int
cl_mos6502::CLD(t_mem code)
{
  cF.W(rF&= ~flagD);
  tick(1);
  return resGO;
}


int
cl_mos6502::SED(t_mem code)
{
  cF.W(rF|= flagD);
  tick(1);
  return resGO;
}


/* End of mos6502.src/inst.cc */
