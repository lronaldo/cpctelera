/*
 * Simulator of microcontrollers (clock.cc)
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
/*@1@*/

#include "clockcl.h"


cl_clock::cl_clock(class cl_uc *auc, t_addr the_addr, const char *aname):
  cl_hw(auc, HW_PORT, 0, aname)
{
  addr= the_addr;
  set_name(aname);
}


int
cl_clock::init(void)
{
  cl_hw::init();
  pre= register_cell(uc->rom, addr+1);
  clock= register_cell(uc->rom, addr);
  pre->W(0);
  clock->W(0);
  return 0;
}

void
cl_clock::reset(void)
{
  if (pre) pre->W(0);
  if (clock) clock->W(0);
  pre_cnt= 0;
}

void
cl_clock::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  if (cell == pre)
    {
      pre_cnt= 0;
    }
  if (cell == clock)
    {
    }
  cell->set(*val);
}

int
cl_clock::tick(int cycles)
{
  t_mem p= pre->get();
  if (p)
    {
      pre_cnt+= cycles;
      if (pre_cnt > p)
	{
	  int i;
	  clock->W(clock->R() + 1);
	  pre_cnt-= p;
	  for (i=2; i<16; i++)
	    {
	      t_mem m= uc->rom->read(addr+i);
	      if (m)
		uc->rom->write(addr+i, m-1);
	    }
	}
    }
  return 0;
}

void
cl_clock::print_info(class cl_console_base *con)
{
  con->dd_printf("Pre= %u             \n",
		 MU(pre->get()));
  con->dd_printf("Cnt= %u             \n",
		 MU(pre_cnt));
  con->dd_printf("Clk= %u             \n",
		 MU(clock->get()));
  int i;
  for (i=2; i<16; i++)
    con->dd_printf("Bcnt[%2d]= %u             \n",
		   i, MU(uc->rom->get(addr+i)));
}


/* End of p1516.src/clock.cc */
