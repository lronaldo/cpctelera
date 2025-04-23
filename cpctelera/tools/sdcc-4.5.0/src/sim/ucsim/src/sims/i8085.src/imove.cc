/*
 * Simulator of microcontrollers (imove.cc)
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

#include "i8080cl.h"


int
cl_i8080::mvi8(class cl_memory_cell &dst)
{
  u8_t d= fetch();
  dst.W(d);
  return resGO;
}

int
cl_i8080::lxi16(class cl_memory_cell &dst)
{
  dst.W(fetch16());
  return resGO;
}

int
cl_i8080::LDA_a16(t_mem code)
{
  cA.W(rom->read(fetch16()));
  vc.rd++;
  return resGO;
}

int
cl_i8080::STA_a16(t_mem code)
{
  rom->write(fetch16(), rA);
  vc.wr++;
  return resGO;
}

int
cl_i8080::LHLD_a16(t_mem code)
{
  cHL.W(read_addr(rom, fetch16()));
  vc.rd+= 2;
  return resGO;
}

int
cl_i8080::SHLD_a16(t_mem code)
{
  u16_t a= fetch16();
  rom->write(a, rL);
  rom->write(a+1, rH);
  vc.wr+= 2;
  return resGO;
}

int
cl_i8080::ldax(u16_t a)
{
  cA.W(rom->read(a));
  vc.rd++;
  return resGO;
}

int
cl_i8080::stax(u16_t a)
{
  rom->write(a, rA);
  vc.wr++;
  return resGO;
}

int
cl_i8080::XCHG(t_mem code)
{
  u16_t t= rHL;
  cHL.W(rDE);
  cDE.W(t);
  return resGO;
}

int
cl_i8080::IN_INST(t_mem code)
{
  /*u8_t a=*/ fetch();
  // TODO
  return resGO;
}

int
cl_i8080::OUT_INST(t_mem code)
{
  /*u8_t a=*/ fetch();
  // TODO
  return resGO;
}

int
cl_i8080::XTHL(t_mem code)
{
  u16_t temp= read_addr(rom, rSP);
  vc.rd+= 2;
  rom->write(rSP, rL);
  rom->write(rSP+1, rH);
  vc.wr+= 2;
  cHL.W(temp);
  return resGO;
}

int
cl_i8080::SPHL(t_mem code)
{
  cSP.W(rHL);
  return resGO;
}


/* End of i8085.src/imove.cc */
