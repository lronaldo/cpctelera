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

#include "i8080cl.h"


int
cl_i8080::JMP(t_mem code)
{
  u16_t a= fetch16();
  PC= a;
  return resGO;
}

int
cl_i8080::jmp_if(bool cond)
{
  u16_t a= fetch16();
  if (cond)
    {
      PC= a;
      tick_shift= 8;
    }
  return resGO;
}

int
cl_i8080::CALL(t_mem code)
{
  u16_t a= fetch16();
  push2(PC);
  PC= a;
  return resGO;
}

int
cl_i8080::call_if(bool cond)
{
  u16_t a= fetch16();
  if (cond)
    {
      push2(PC);
      PC= a;
      tick_shift= 8;
    }
  return resGO;
}

int
cl_i8080::RET(t_mem code)
{
  PC= pop2();
  return resGO;
}

int
cl_i8080::ret_if(bool cond)
{
  if (cond)
    {
      PC= pop2();
      tick_shift= 8;
    }
  return resGO;
}

int
cl_i8080::PCHL(t_mem code)
{
  PC= rHL;
  return resGO;
}

int
cl_i8080::rst(t_mem code)
{
  u16_t a= code & 0x28;
  push2(PC);
  PC= a;
  return resGO;
}


/* End of i8085.src/ibranch.cc */
