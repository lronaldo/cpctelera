/*
 * Simulator of microcontrollers (stm8.src/uid.cc)
 *
 * Copyright (C) 2017,17 Drotos Daniel, Talker Bt.
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

#include "uidcl.h"


cl_uid::cl_uid(class cl_uc *auc, t_addr abase):
       cl_hw(auc, HW_DUMMY, 0, "uid")
{
  base= abase;
}

static u8_t uid[12]= {
  0x00,
  0x5b,
  0x00,
  0x16,
  0x11,
  0x47,
  0x30,
  0x31,
  0x38,
  0x35,
  0x35,
  0x36
};

int
cl_uid::init(void)
{
  int i;
  cl_hw::init();
  for (i= 0; i < 12; i++)
    {
      //regs[i]= register_cell(uc->rom, base+i);
      uc->rom->download(base+i, uid[i]);
      uc->rom->set_cell_flag(base+i, true, CELL_READ_ONLY);
    }
  return 0;
}

t_mem
cl_uid::read(class cl_memory_cell *cell)
{
  t_mem v= cell->get();
  //t_addr a;
  
  if (conf(cell, NULL))
    return v;
  /*
  if (!uc->rom->is_owned(cell, &a))
    return v;
  if ((a < base) ||
      (a >= base+12))
    return v;
  a-= base;

  cell->set(v= uid[a]);
  */
  return v;
}

void
cl_uid::write(class cl_memory_cell *cell, t_mem *val)
{
  //t_addr a;

  if (conf(cell, val))
    return;
  /*
  if (!uc->rom->is_owned(cell, &a))
    return;  
  if ((a < base) ||
      (a >= base+12))
    return;
  a-= base;
  *val= uid[a];
  */
}

void
cl_uid::print_info(class cl_console_base *con)
{
  /*
  con->dd_printf("base= 0x%04x\n", base);
  con->dd_printf("end = 0x%04x\n", base+12);
  con->dd_printf("uid =");
  int i;
  for (i= 0; i < 12; i++)
    con->dd_printf(" %02x", uc->rom->get(base+i));
  con->dd_printf("\n");
  */
  uc->rom->dump(base, base+12, 16, con->get_fout());
}


/* End of stm8.src/uid.cc */
