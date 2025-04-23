/*
 * Simulator of microcontrollers (iother.cc)
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

#include "m68hc12cl.h"


int
CL12::andcc(u8_t op)
{
  u8_t f= rF;
  cF.W(f & op);
  if ((f & flagI) && !(rF & flagI))
    block_irq= true;
  return resGO;
}

int
CL12::orcc(u8_t op)
{
  u8_t f= rF;
  cF.W(f | (op&~flagX));
  return resGO;
}

int
CL12::lea(class cl_memory_cell &dest)
{
  u8_t p= rom->read(PC);
  t_addr a= naddr(NULL, NULL);
  if (!xb_indirect(p))
    {
      dest.W(a&0xffff);
    }
  return resGO;
}


/* End of m68hc12.src/iother.cc */
