/*
 * Simulator of microcontrollers (tlcs.src/inst_bit.cc)
 *
 * Copyright (C) 2016,16 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

/* $Id:$ */

#include "tlcscl.h"


// TSET 8-bit
u8_t
cl_tlcs::op_tset(u8_t val, u8_t bitnr)
{
  reg.raf.f&= ~(FLAG_Z|FLAG_N);
  reg.raf.f|= FLAG_H;
  
  bitnr&= 0x07;
  if ((val & (1 << bitnr)) == 0)
    reg.raf.f|= FLAG_Z;
  val|= (1 << bitnr);
  return val;
}


// TSET mem
u8_t
cl_tlcs::inst_tset(cl_memory_cell *cell, u8_t bitnr)
{
  u8_t v= cell->read();
  vc.rd++;
  v= op_tset(v, bitnr);
  cell->write(v);
  vc.wr++;
  return v;
}


// BIT 8-bit
u8_t
cl_tlcs::op_bit(u8_t val, u8_t bitnr)
{
  reg.raf.f&= ~(FLAG_Z|FLAG_N);
  reg.raf.f|= FLAG_H;

  bitnr&= 7;

  if ((val & (1 << bitnr)) == 0)
    reg.raf.f|= FLAG_Z;

  return val;
}


// BIT mem
u8_t
cl_tlcs::inst_bit(cl_memory_cell *cell, u8_t bitnr)
{
  u8_t v= cell->read();
  vc.rd++;
  v= op_bit(v, bitnr);
  cell->write(v);
  vc.wr++;
  return v;
}


// RES 8-bit
u8_t
cl_tlcs::op_res(u8_t val, u8_t bitnr)
{
  bitnr&= 7;

  val&= ~(1 << bitnr);

  return val;
}


// RES mem
u8_t
cl_tlcs::inst_res(cl_memory_cell *cell, u8_t bitnr)
{
  u8_t v= cell->read();
  vc.rd++;
  v= op_res(v, bitnr);
  cell->write(v);
  vc.wr++;
  return v;
}


// SET 8-bit
u8_t
cl_tlcs::op_set(u8_t val, u8_t bitnr)
{
  bitnr&= 7;

  val|= (1 << bitnr);

  return val;
}


// SET mem
u8_t
cl_tlcs::inst_set(cl_memory_cell *cell, u8_t bitnr)
{
  u8_t v= cell->read();
  vc.rd++;
  v= op_set(v, bitnr);
  cell->write(v);
  vc.wr++;
  return v;
}


/* End of tlcs.src/inst_bit.cc */
