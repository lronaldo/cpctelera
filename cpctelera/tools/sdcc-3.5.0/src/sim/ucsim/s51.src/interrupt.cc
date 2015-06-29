/*
 * Simulator of microcontrollers (interrupt.cc)
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
#include "utils.h"

// sim
#include "itsrccl.h"

// local
#include "interruptcl.h"
#include "regs51.h"
//#include "uc51cl.h"
#include "types51.h"


cl_interrupt::cl_interrupt(class cl_uc *auc):
  cl_hw(auc, HW_INTERRUPT, 0, "irq")
{
  was_reti= DD_FALSE;
}

int
cl_interrupt::init(void)
{
  sfr= uc->address_space(MEM_SFR_ID);
  if (sfr)
    {
      //sfr->register_hw(IE, this, 0);
      register_cell(sfr, IE, 0, wtd_restore);
      register_cell(sfr, TCON, &cell_tcon, wtd_restore_write);
      bit_INT0= sfr->read(P3) & bm_INT0;
      bit_INT1= sfr->read(P3) & bm_INT1;
    }
  return(0);
}

void
cl_interrupt::added_to_uc(void)
{
  uc->it_sources->add(new cl_it_src(bmEX0, TCON, bmIE0, 0x0003, true,
                                    "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, TCON, bmIE1, 0x0013, true,
                                    "external #1", 3));
}

void
cl_interrupt::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_tcon)
    {
      bit_IT0= *val & bmIT0;
      bit_IT1= *val & bmIT1;
    }
  else
    // IE register
    was_reti= DD_TRUE;
}

/*void
cl_interrupt::mem_cell_changed(class cl_m *mem, t_addr addr)
{
}*/

int
cl_interrupt::tick(int cycles)
{
  if (!bit_IT0 && !bit_INT0)
    cell_tcon->set_bit1(bmIE0);
  if (!bit_IT1 && !bit_INT1)
    cell_tcon->set_bit1(bmIE1);
  return(resGO);
}

void
cl_interrupt::reset(void)
{
  was_reti= DD_FALSE;
}

void
cl_interrupt::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;

  if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 3)
    {
      t_mem p3n= ep->new_pins & ep->new_value;
      t_mem p3o= ep->pins & ep->prev_value;
      if (bit_IT0 &&
          !(p3n & bm_INT0) &&
          (p3o & bm_INT0))
        cell_tcon->set_bit1(bmIE0);
      if (bit_IT1 &&
          !(p3n & bm_INT1) &&
          (p3o & bm_INT1))
        cell_tcon->set_bit1(bmIE1);
      bit_INT0= p3n & bm_INT0;
      bit_INT1= p3n & bm_INT1;
    }
}


void
cl_interrupt::print_info(class cl_console_base *con)
{
  int ie= sfr->get(IE);
  int i;

  con->dd_printf("Interrupts are %s. Interrupt sources:\n",
                 (ie&bmEA)?"enabled":"disabled");
  con->dd_printf("  Handler  En  Pr Req Act Name\n");
  for (i= 0; i < uc->it_sources->count; i++)
    {
      class cl_it_src *is= (class cl_it_src *)(uc->it_sources->at(i));
      con->dd_printf("  0x%06x", is->addr);
      con->dd_printf(" %-3s", (ie&(is->ie_mask))?"en":"dis");
      con->dd_printf(" %2d", uc->it_priority(is->ie_mask));
      con->dd_printf(" %-3s",
                     (sfr->get(is->src_reg)&(is->src_mask))?
                     "YES":"no");
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
          con->dd_printf(" 0x%06x", il->addr);
          con->dd_printf(" 0x%06x", il->PC);
          con->dd_printf(" %s", (il->source)?(object_name(il->source)):
                         "nothing");
          con->dd_printf("\n");
        }
    }
}


/* End of s51.src/interrupt.cc */
