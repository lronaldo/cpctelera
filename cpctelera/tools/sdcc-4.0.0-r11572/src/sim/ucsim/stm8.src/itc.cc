/*
 * Simulator of microcontrollers (itc.cc)
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

// prj

//#include <stdarg.h>
#include "utils.h"

// sim
#include "itsrccl.h"

// local
#include "itccl.h"


cl_itc::cl_itc(class cl_uc *auc):
  cl_hw(auc, HW_INTERRUPT, 0, "itc")
{
  int i;
  for (i= 0; i < 8; i++)
    spr[i]= 0;
}

int
cl_itc::init(void)
{
  int i;
  cl_hw::init();
  for (i= 0; i < 8; i++)
    {
      if (spr[i])
	unregister_cell(spr[i]);
      spr[i]= register_cell(uc->rom, 0x7f70+i);
    }
  return(0);
}

void
cl_itc::added_to_uc(void)
{
}

void
cl_itc::new_hw_added(class cl_hw *new_hw)
{
}

t_mem
cl_itc::read(class cl_memory_cell *cell)
{
  return cell->get();
}

void
cl_itc::write(class cl_memory_cell *cell, t_mem *val)
{
  t_addr a;
  if (uc->rom->is_owned(cell, &a) &&
      (a >= 0x7f70) &&
      (a <  0x7f70+8))
    {
      u8_t mask= 0xff;
      if ((*val & 0x03) == 0x02)
	mask&= ~0x03;
      if ((*val & 0x0c) == 0x08)
	mask&= ~0x0c;
      if ((*val & 0x30) == 0x20)
	mask&= ~0x30;
      if ((*val & 0xc0) == 0x80)
	mask&= ~0xc0;
      u8_t o= cell->get(), v= *val;
      o&= ~mask;
      v&= mask;
      o|= v;
      *val= o;
    }
}

/*void
cl_itc::mem_cell_changed(class cl_m *mem, t_addr addr)
{
}*/

int
cl_itc::tick(int cycles)
{
  return(resGO);
}

void
cl_itc::reset(void)
{
  int i;
  for (i= 0; i < 8; i++)
    spr[i]->write(0xff);
}

void
cl_itc::happen(class cl_hw *where, enum hw_event he, void *params)
{
}


void
cl_itc::print_info(class cl_console_base *con)
{
  int i;

  con->dd_printf("Interrupts are %s. Interrupt sources:\n",
		 (uc->it_enabled())?"enabled":"disabled");
  con->dd_printf("  Handler  En  Pr Req Act Name\n");
  for (i= 0; i < uc->it_sources->count; i++)
    {
      class cl_it_src *is= (class cl_it_src *)(uc->it_sources->at(i));
      con->dd_printf("  0x%06x", AU(is->addr));
      con->dd_printf(" %-3s", (is->enabled())?"en":"dis");
      con->dd_printf(" %2d", uc->priority_of(/*is->ie_mask*/is->nuof));
      con->dd_printf(" %-3s", (is->pending())?"YES":"no");
      con->dd_printf(" %-3s", (is->active)?"act":"no");
      con->dd_printf(" %s", object_name(is));
      con->dd_printf("\n");
    }
  con->dd_printf("Active interrupt service(s):\n");
  con->dd_printf("  Pr Handler  PC       Source\n");
  for (i= 0; i < uc->it_levels->count; i++)
    {
      class it_level *il= (class it_level *)(uc->it_levels->at(i));
      if (il->level >= 0)
	{
	  con->dd_printf("  %2d", il->level);
	  con->dd_printf(" 0x%06x", AU(il->addr));
	  con->dd_printf(" 0x%06x", AU(il->PC));
	  con->dd_printf(" %s", (il->source)?(object_name(il->source)):
			 "nothing");
	  con->dd_printf("\n");
	}
    }
  print_cfg_info(con);
}


/* End of s51.src/itc.cc */
