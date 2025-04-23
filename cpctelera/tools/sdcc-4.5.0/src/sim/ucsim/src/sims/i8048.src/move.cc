/*
 * Simulator of microcontrollers (move.cc)
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
CL2::in(int port_addr)
{
  if (port_addr >= 4)
    cA.W(pext->in(port_addr));
  else
    cA.W(ports->read(port_addr));
  return resGO;
}

int
CL2::out(int port_addr)
{
  if (port_addr >= 4)
    pext->out(port_addr, rA);
  else
    ports->write(port_addr, rA);
  return resGO;
}

int
CL2::xch(class cl_memory_cell *op)
{
  u8_t t= cA.R();
  cA.W(op->read());
  op->write(t);
  return resGO;
}

int
CL2::xchd(class cl_memory_cell *op)
{
  u8_t o= op->read(), a= cA.R();
  u8_t to= o&0x0f, ta= a&0x0f;
  cA.W((a&0xf0)|to);
  op->write((o&0xf0)|ta);
  return resGO;
}

int
CL2::SWAPA(MP)
{
  u8_t l= rA&0x0f;
  cA.W((rA>>4)|(l<<4));
  return resGO;
}

int
CL2::movp(void)
{
  u16_t a= PC & 0xf00;
  a|= rA;
  cA.W(rom->read(a));
  if (a >= inner_rom)
    bus->latch(a);
  return resGO;
}

int
CL2::movp3(void)
{
  u16_t a= PC & 0x800;
  a|= (0x300+rA);
  cA.W(rom->read(a));
  if (a >= inner_rom)
    bus->latch(a);
  return resGO;
}


/* End of i8048.src/move.cc */
