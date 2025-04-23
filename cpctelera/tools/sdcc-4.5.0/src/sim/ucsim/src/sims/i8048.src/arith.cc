/*
 * Simulator of microcontrollers (arith.cc)
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

#include "i8020cl.h"


int
cl_i8020::add(u8_t op2, bool addc)
{
  u16_t r, c= addc?((psw&flagC)?1:0):0;
  rF&= ~flagCA;
  r= cA.R()+op2+c;
  if (r > 0xff) rF|= flagC;
  if (((rA&0xf) + (op2&0xf) + c) > 0xf) rF|= flagA;
  cF.W(rF);
  cA.W(r);
  return resGO;
}

int
CL2::dec(class cl_memory_cell *op)
{
  op->write(op->read() - 1);
  return resGO;
}

int
CL2::inc(class cl_memory_cell *op)
{
  op->write(op->read() + 1);
  return resGO;
}

int
CL2::orl(class cl_memory_cell *op)
{
  cA.W(rA | op->read());
  return resGO;
}

int
CL2::anl(class cl_memory_cell *op)
{
  cA.W(rA & op->read());
  return resGO;
}

int
CL2::xrl(class cl_memory_cell *op)
{
  cA.W(rA ^ op->read());
  return resGO;
}

int
CL2::daa(void)
{
  if (((rA & 0x0f) > 9) || (rF & flagA))
    {
      if (rA > 0xf9) rF|= flagC;
      rA+= 6;
    }
  if (((rA & 0xf0) > 0x90) || (rF & flagC))
    {
      rA+= 0x60;
      rF|= flagC;
    }
  cA.W(rA);
  cF.W(rF);
  return resGO;
}

int
CL2::RRC(MP)
{
  u8_t orgc= rF & flagC;
  rF&= ~flagC;
  if (rA & 1) rF|= flagC;
  rA>>= 1;
  if (orgc) rA|= 0x80;
  cA.W(rA);
  cF.W(rF);
  return resGO;
}

int
CL2::RR(MP)
{
  u8_t a0= (rA&1)?0x80:0;
  rA>>= 1;
  rA|= a0;
  cA.W(rA);
  return resGO;
}

int
CL2::RL(MP)
{
  u8_t a7= (rA&0x80)?1:0;
  rA<<= 1;
  rA|= a7;
  cA.W(rA);
  return resGO;
}

int
CL2::RLC(MP)
{
  u8_t orgc= rF & flagC;
  rF&= ~flagC;
  if (rA & 0x80) rF|= flagC;
  rA<<= 1;
  if (orgc) rA|= 1;
  cA.W(rA);
  cF.W(rF);
  return resGO;
}

int
CL2::orld(int addr, cl_memory_cell *op2)
{
  u8_t v2= op2->R() & 0xf;
  pext->orl(addr, v2);
  return resGO;
}

int
CL2::anld(int addr, cl_memory_cell *op2)
{
  u8_t v2= op2->R() & 0xf;
  pext->anl(addr, v2);
  return resGO;
}


/* End of i8048.src/arith.cc */
