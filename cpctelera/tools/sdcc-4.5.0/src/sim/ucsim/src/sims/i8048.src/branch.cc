/*
 * Simulator of microcontrollers (branch.cc)
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
cl_i8020::jmp(MP)
{
  u16_t a;
  a= (code&0xe0)<<3;
  a+= fetch();
  if (A11) a|= 0x800;
  PC= a;
  return resGO;
}

int
CL2::call(MP)
{
  u16_t a= (code&0xe0)<<3;
  a+= fetch();
  if (A11) a|= 0x800;
  push();
  PC= a;
  return resGO;
}

int
CL2::RET(MP)
{
  PC= pop(false);
  return resGO;
}

/*
int
CL2::RETR(MP)
{
  PC= pop(true);
  return resGO;
}
*/

int
CL2::jb(MP)
{
  u8_t a= fetch();
  u8_t m= 1<<((code>>5)&7);
  if (rA & m)
    {
      PC&= 0xf00;
      PC|= a;
    }
  return resGO;
}

int
CL2::jif(bool cond)
{
  u8_t a= fetch();
  if (cond)
    {
      PC&= 0xf00;
      PC|= a;
    }
  return resGO;
}

/*
int
CL2::JNZ(MP)
{
  u8_t a= fetch();
  if (rA)
    {
      PC&= 0xf00;
      PC|= a;
    }
  return resGO;
}
*/

/*
int
CL2::JZ(MP)
{
  u8_t a= fetch();
  if (!rA)
    {
      PC&= 0xf00;
      PC|= a;
    }
  return resGO;
}
*/

/*
int
CL2::JNC(MP)
{
  u8_t a= fetch();
  if (!(psw & flagC))
    {
      PC&= 0xf00;
      PC|= a;
    }
  return resGO;
}
*/

/*
int
CL2::JC(MP)
{
  u8_t a= fetch();
  if (psw & flagC)
    {
      PC&= 0xf00;
      PC|= a;
    }
  return resGO;
}
*/

/*
int
CL2::JT1(MP)
{
  u8_t a= fetch();
  if (cpu->cfg_read(i8020cpu_t1))
    {
      PC&= 0xf00;
      PC|= a;
    }
  return resGO;
}
*/

int
CL2::JMPPIA(MP)
{
  u16_t a= (PC&=0xf00)|rA;
  PC|= rom->read(a);
  return resGO;
}

int
CL2::djnz(MP)
{
  u8_t r= (code>>5)&7;
  u8_t a= fetch();
  R[r]->W(R[r]->R() - 1);
  if (R[r] != 0)
    {
      PC&= 0xf00;
      PC+= a;
    }
  return resGO;
}


/* End of i8048.src/branch.cc */
