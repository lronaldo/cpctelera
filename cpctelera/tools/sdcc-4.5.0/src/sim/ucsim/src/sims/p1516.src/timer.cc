/*
 * Simulator of microcontrollers (timer.cc)
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

#include "timercl.h"


cl_timer::cl_timer(class cl_uc *auc, t_addr the_addr, const char *aname):
  cl_hw(auc, HW_TIMER, 0, aname)
{
  addr= the_addr;
  set_name(aname);
  cps= 25;
}


int
cl_timer::init(void)
{
  cl_hw::init();
  ctrl= register_cell(uc->rom, addr+0);
  ar  = register_cell(uc->rom, addr+1);
  cnt = register_cell(uc->rom, addr+2);
  stat= register_cell(uc->rom, addr+3);
  return 0;
}


void
cl_timer::reset(void)
{
  if (ctrl) ctrl->W(0);
  if (ar) ar->W(0);
  if (cnt) cnt->W(0);
  overflow= 2;
  pre_cnt= 0;
}

t_mem
cl_timer::read(class cl_memory_cell *cell)
{
  if (cell == stat)
    {
      u32_t s= ctrl->get() & 1;
      if (cnt->get() == ar->get())
	overflow= 2;
      s|= overflow;
      cell->set(s);
    }
  conf(cell, NULL);
  return cell->get();
}

void
cl_timer::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  
  if (cell == stat)
    {
      if (*val & 2)
	overflow= 0;
      *val= ctrl->get() & 1;
    }
  else if ((cell == cnt) || (cell == ar))
    {
      cell->set(*val);
    }
  if (cnt->get() == ar->get())
    overflow= 2;
  cell->set(*val);
}


int
cl_timer::tick(int cycles)
{
  t_mem p= cps;
  t_mem n, a;
  if (!(ctrl->get() & 1))
    return 0;
  if (p)
    {
      pre_cnt+= cycles;
      if (pre_cnt > p)
	{
	  if (cnt->get() == (a= ar->get()))
	    cnt->W(n= 0);
	  else
	    cnt->W(n= cnt->get() + 1);
	  pre_cnt-= p;
	  if (n == a)
	    overflow= 2;
	}
    }
  return 0;
}

void
cl_timer::print_info(class cl_console_base *con)
{
  u32_t c= ctrl->get();
  con->dd_printf("cps= %u pre_cnt=%u         \n",
		 MU(cps), MU(pre_cnt));
  con->dd_printf("Ctrl  : on=%s  ie=%s\n",
		 (c&1)?"on ":"off",
		 (c&2)?"on ":"off");
  con->dd_printf("AR    = %u                \n",
		 MU(ar->get()));
  con->dd_printf("Value = %u             \n",
		 MU(cnt->get()));
  con->dd_printf("Status: overflow= %s\n",
		 overflow?"true ":"false");
}


/* End of p1516.src/timer.cc */
