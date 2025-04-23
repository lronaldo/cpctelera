/*
 * Simulator of microcontrollers (ibranch.cc)
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

#include "appcl.h"

#include "mos6502cl.h"


int
cl_mos6502::JMPa(t_mem code)
{
  u16_t a= i16();
  PC= a;
  tick(2);
  return resGO;
}

int
cl_mos6502::JMPi(t_mem code)
{
  u16_t a= i16();
  a= read_addr(rom, a);
  PC= a;
  vc.rd+= 2;
  tick(4);
  return resGO;
}

int
cl_mos6502::JSR(t_mem code)
{
  u16_t a= fetch(); // low
  t_addr pc= PC;
  t_addr spbef= rSP;
  a+= (fetch()*256); // high
  push_addr(pc);
  class cl_stack_call *op= new cl_stack_call(instPC, a, PC, spbef, rSP);
  op->init();
  stack_write(op);
  PC= a;
  return resGO;
}

int
cl_mos6502::RTS(t_mem code)
{
  t_addr spbef= rSP;
  PC= pop_addr()+1;
  class cl_stack_ret *op= new cl_stack_ret(instPC, PC, spbef, rSP);
  op->init();
  stack_read(op);
  tick(3);
  return resGO;
}

int
cl_mos6502::branch(bool cond)
{
  i8_t rel;
  u16_t a;
  rel= fetch();
  if (cond)
    {
      a= PC+rel;
      if ((u16_t)(PC&0xff00) != (u16_t)(a&0xff00))
	tick(1);
      PC= a;
      tick(1);
    }
  tick(1);
  return resGO;
}


/* End of mos6502.src/ibranch.cc */
