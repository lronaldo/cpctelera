/*
 * Simulator of microcontrollers (imove.cc)
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

#include "mos6502cl.h"


int
cl_mos6502::TYA(t_mem code)
{
  cA.W(rY);
  if (rA)
    rF&= ~flagZ;
  else
    rF|= flagZ;
  if (rA & 0x80)
    rF|= flagS;
  else
    rF&= ~flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::TAY(t_mem code)
{
  cY.W(rA);
  if (rY)
    rF&= ~flagZ;
  else
    rF|= flagZ;
  if (rY & 0x80)
    rF|= flagS;
  else
    rF&= ~flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::TXA(t_mem code)
{
  cA.W(rX);
  if (rA)
    rF&= ~flagZ;
  else
    rF|= flagZ;
  if (rA & 0x80)
    rF|= flagS;
  else
    rF&= ~flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::TXS(t_mem code)
{
  cSP.W(rX);
  tick(1);
  return resGO;
}

int
cl_mos6502::TAX(t_mem code)
{
  cX.W(rA);
  if (rX)
    rF&= ~flagZ;
  else
    rF|= flagZ;
  if (rX & 0x80)
    rF|= flagS;
  else
    rF&= ~flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::TSX(t_mem code)
{
  cX.W(rSP);
  if (rX)
    rF&= ~flagZ;
  else
    rF|= flagZ;
  if (rX & 0x80)
    rF|= flagS;
  else
    rF&= ~flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::st(u8_t reg, class cl_cell8 &op)
{
  op.W(reg);
  return resGO;
}

int
cl_mos6502::sta(class cl_cell8 &op)
{
  op.W(rA);
  return resGO;
}

int
cl_mos6502::lda(class cl_cell8 &op)
{
  u8_t f= rF & ~(flagZ|flagN);
  cA.W(op.R());
  if (!rA) f|= flagZ;
  if (rA & 0x80) f|= flagN;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::ld(class cl_cell8 &reg, class cl_cell8 &op)
{
  u8_t v, f= rF & ~(flagZ|flagN);
  reg.W(v= op.R());
  if (!v) f|= flagZ;
  if (v & 0x80) f|= flagN;
  cF.W(f);
  return resGO;
}

/* End of mos6502.src/imove.cc */
