/*
 * Simulator of microcontrollers (inc.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

#include "ddconfig.h"

// local
#include "uc51cl.h"
#include "regs51.h"


/*
 * 0x04 1 12 INC A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_inc_a(uchar code)
{
  acc->wadd(1);
  return(resGO);
}


/*
 * 0x05 2 12 INC addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_inc_addr(uchar code)
{
  class cl_memory_cell *cell= get_direct(fetch());

  t_mem d= cell->read(HW_PORT);
  cell->write(d+1);
  return(resGO);
}


/*
 * 0x06-0x07 1 12 INC @Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_inc_Sri(uchar code)
{
  class cl_memory_cell *cell;

  cell= iram->get_cell(get_reg(code & 0x01)->read());
  cell->wadd(1);
  return(resGO);
}


/*
 * 0x08-0x0f 1 12 INC Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_inc_rn(uchar code)
{
  class cl_memory_cell *reg= get_reg(code & 0x07);

  reg->wadd(1);
  return(resGO);
}


/*
 * 0x14 1 12 DEC A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_dec_a(uchar code)
{
  acc->wadd(-1);

  return(resGO);
}


/*
 * 0x15 2 12 DEC addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_dec_addr(uchar code)
{
  class cl_memory_cell *cell;

  cell= get_direct(fetch());
  t_mem d= cell->read(HW_PORT);
  cell->write(d-1);
  return(resGO);
}


/*
 * 0x16-0x17 1 12 DEC @Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_dec_Sri(uchar code)
{
  class cl_memory_cell *cell;

  cell= iram->get_cell(get_reg(code & 0x01)->read());
  cell->add(-1);
  return(resGO);
}


/*
 * 0x18-0x1f 1 12 DEC Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_dec_rn(uchar code)
{
  class cl_memory_cell *reg= get_reg(code & 0x07);

  reg->wadd(-1);
  return(resGO);
}


/*
 * 0xa3 1 24 INC DPTR
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_inc_dptr(uchar code)
{
  uint dptr;

  dptr= sfr->read(DPH)*256 + sfr->read(DPL) + 1;
  sfr->write(DPH, (dptr >> 8) & 0xff);
  sfr->write(DPL, dptr & 0xff);
  tick(1);
  return(resGO);
}


/* End of s51.src/inc.cc */
