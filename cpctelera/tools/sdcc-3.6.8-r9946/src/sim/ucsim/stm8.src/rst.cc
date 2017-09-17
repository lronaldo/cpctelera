/*
 * Simulator of microcontrollers (stm8.src/rst.cc)
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

/* $Id: rst.cc 614 2017-01-27 08:39:01Z drdani $ */

#include "rstcl.h"


cl_rst::cl_rst(class cl_uc *auc, t_addr abase, t_mem amask):
  cl_hw(auc, HW_RESET, 0, "rst")
{
  base= abase;
  rst_sr= 0;
  mask= amask;
}

int
cl_rst::init(void)
{
  rst_sr= register_cell(uc->rom, base);
  return 0;
}

t_mem
cl_rst::read(class cl_memory_cell *cell)
{
  return cell->get();
}

void
cl_rst::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == rst_sr)
    {
      u8_t v= *val & mask, o= cell->get();
      *val= o & ~v;
    }
}


/* End of stm8.src/rst.cc */
