/*
 * Simulator of microcontrollers (ibranch.cc)
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

#include "f8cl.h"


int
cl_f8::JP_I(t_mem code)
{
  u16_t a= fetch();
  a+= fetch()*256;
  PC= a;
  return resGO;
}

int
cl_f8::JP_A(t_mem code)
{
  PC= acc16->get();
  return resGO;
}

int
cl_f8::CALL_I(t_mem code)
{
  u16_t a= fetch();
  a+= fetch()*256;
  push2(PC);
  PC= a;
  return resGO;
}

int
cl_f8::CALL_A(t_mem code)
{
  push2(PC);
  PC= acc16->get();
  return resGO;
}

int
cl_f8::RET(t_mem code)
{
  set_PC(pop2());
  return resGO;
}

int
cl_f8::RETI(t_mem code)
{
  // TODO
  set_PC(pop2());
  return resGO;
}

int
cl_f8::jr(bool cond)
{
  i8_t d= fetch();
  u16_t a= PC - 2;
  if (cond)
    set_PC(a+d);
  return resGO;
}

int
cl_f8::DNJNZ(t_mem code)
{
  i8_t d= fetch();
  u16_t a= PC - 2;
  cYH.W(rYH-1);
  rF&= ~(flagZ|flagN);
  if (rYH==0) rF|= flagZ;
  if (rYH&0x80) rF|= flagN;
  cF.W(rF);
  if (rYH)
    set_PC(a+d);
  return resGO;
}


/* End of f8.src/ibranch.cc */
