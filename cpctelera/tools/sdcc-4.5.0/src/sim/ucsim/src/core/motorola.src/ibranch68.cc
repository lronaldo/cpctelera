/*
 * Simulator of microcontrollers (ibranch68.cc)
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
cl_m6800::call(t_addr a)
{
  u8_t h= PC>>8, l= PC;
  rom->write(rSP, l);
  rSP--;
  rom->write(rSP, h);
  cSP.W(rSP-1);
  PC= a;
  vc.wr+= 2;
  return resGO;
}

int
cl_m6800::RTS(t_mem code)
{
  u8_t h, l;
  rSP++;
  h= rom->read(rSP);
  cSP.W(rSP+1);
  l= rom->read(rSP);
  PC= h*256 + l;
  vc.rd+= 2;
  return resGO;
}

int
cl_m6800::branch(t_addr a, bool cond)
{
  if (cond)
    PC= a&0xffff;
  return resGO;
}

int
cl_m6800::JMPi(t_mem code)
{
  t_addr a= fetch();
  a+= rX;
  PC= a;
  return resGO;
}

int
cl_m6800::JMPe(t_mem code)
{
  u8_t h, l;
  h= fetch();
  l= fetch();
  PC= h*256 + l;
  return resGO;
}


/* End of motorola.src/ibranch68.cc */
