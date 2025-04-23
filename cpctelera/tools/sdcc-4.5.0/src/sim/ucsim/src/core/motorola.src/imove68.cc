/*
 * Simulator of microcontrollers (imove68.cc)
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

#include "m6800cl.h"


int
cl_m6800::clr(class cl_memory_cell &dest)
{
  u8_t f= rF & ~(flagN|flagV|flagC);
  dest.W(0);
  f|= flagZ;
  cCC.W(f);
  return resGO;
}

int
cl_m6800::lda(class cl_memory_cell &dest, u8_t op)
{
  u8_t f= rF & ~(flagN|flagV|flagC);
  dest.W(op);
  if (!op) f|= flagZ;
  if (op&0x80) f|= flagN;
  cCC.W(f);
  return resGO;
}

int
cl_m6800::sta(class cl_memory_cell &dest, u8_t op)
{
  u8_t f= rF & ~(flagN|flagV|flagC);
  dest.W(op);
  if (!op) f|= flagZ;
  if (op&0x80) f|= flagN;
  cCC.W(f);
  return resGO;
}

int
cl_m6800::ldsx(class cl_cell16 &dest, u16_t op)
{
  u8_t f= rF & ~(flagN|flagV|flagC);
  dest.W(op);
  if (!op) f|= flagZ;
  if (op&0x8000) f|= flagN;
  cCC.W(f);
  return resGO;
}

int
cl_m6800::stsx(t_addr a, u16_t op)
{
  u8_t f= rF & ~(flagN|flagV|flagC);
  rom->write(a, op>>8);
  rom->write(a+1, op&0xff);
  vc.wr+= 2;
  if (!op) f|= flagZ;
  if (op&0x8000) f|= flagN;
  cCC.W(f);
  return resGO;
}


int
cl_m6800::TAP(t_mem code)
{
  cF.W(rA);
  return resGO;
}

int
cl_m6800::TPA(t_mem code)
{
  cA.W(rF);
  return resGO;
}

int
cl_m6800::TAB(t_mem code)
{
  u8_t f= rCC & ~(flagN|flagZ|flagV);
  cB.W(rA);
  if (!rA) f|= flagZ;
  if (rA&0x80) f|= flagN;
  cCC.W(f);
  return resGO;
}

int
cl_m6800::TBA(t_mem code)
{
  u8_t f= rF & ~(flagN|flagZ|flagV);
  cA.W(rB);
  if (!rB) f|= flagZ;
  if (rB&0x80) f|= flagN;
  cF.W(f);
  return resGO;
}

int
cl_m6800::TSX(t_mem code)
{
  cIX.W(rSP+1);
  return resGO;
}

int
cl_m6800::PULA(t_mem code)
{
  cSP.W(rSP+1);
  cA.W(rom->read(rSP));
  vc.rd++;
  return resGO;
}

int
cl_m6800::PULB(t_mem code)
{
  cSP.W(rSP+1);
  cB.W(rom->read(rSP));
  vc.rd++;
  return resGO;
}

int
cl_m6800::TXS(t_mem code)
{
  cSP.W(rIX-1);
  return resGO;
}

int
cl_m6800::PSHA(t_mem code)
{
  rom->write(rSP, rA);
  cSP.W(rSP-1);
  vc.wr++;
  return resGO;
}

int
cl_m6800::PSHB(t_mem code)
{
  rom->write(rSP, rB);
  cSP.W(rSP-1);
  vc.wr++;
  return resGO;
}


/* End of motorola.src/imove68.cc */
