/*
 * Simulator of microcontrollers (s51.src/timer1.cc)
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

#include "timer1cl.h"
#include "regs51.h"


cl_timer1::cl_timer1(class cl_uc *auc, int aid, const char *aid_string):
  cl_timer0(auc, aid, aid_string)
{
  make_partner(HW_UART, 0);
}

/*int
cl_timer1::init(void)
{
  return(0);
}*/

/*void
cl_timer1::added(class cl_hw *new_hw)
{
  if (new_hw->cathegory == HW_UART)
    hws_to_inform->add(new_hw);
}*/

int
cl_timer1::do_mode3(int cycles)
{
  return(0);
}

/*void
cl_timer1::overflow(void)
{
  inform_partners(EV_OVERFLOW, 0);
}*/

void
cl_timer1::print_info(class cl_console_base *con)
{
  const char *modes[]= { "13 bit", "16 bit", "8 bit autoreload", "stop" };
  //int tmod= cell_tmod->get();
  int on;
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);

  con->dd_printf("%s[%d] 0x%04x", id_string, id,
                 256*cell_th->get()+cell_tl->get());
  //int mode= (tmod & (bmM11|bmM01)) >> 4;
  con->dd_printf(" %s", modes[mode]);
  con->dd_printf(" %s", (/*tmod&bm*/C_T/*1*/)?"counter":"timer");
  if (/*tmod&bm*/GATE/*1*/)
    {
      con->dd_printf(" gated");
      on= INT;
    }
  else
    on= cell_tcon->get() & mask_TR;
  con->dd_printf(" %s", on?"ON":"OFF");
  con->dd_printf(" irq=%c", (cell_tcon->get()&mask_TF)?'1':'0');
  con->dd_printf(" %s", sfr?"?":((sfr->get(IE)&bmET1)?"en":"dis"));
  con->dd_printf(" prio=%d", uc->it_priority(bmPT1));
  con->dd_printf("\n");
}


/* End of s51.src/timer1.cc */
