/*
 * Simulator of microcontrollers (wdt.cc)
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

#include <ctype.h>

// local
#include "wdtcl.h"
#include "regs51.h"


cl_wdt::cl_wdt(class cl_uc *auc, long resetvalue):
  cl_hw(auc, HW_WDT, 0, "wdt")
{
  reset_value= resetvalue;
  wdt= -1;
  written_since_reset= DD_FALSE;
}

int
cl_wdt::init(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);

  if (!sfr)
    {
      fprintf(stderr, "No SFR to register WDT into\n");
    }
  //wdtrst= sfr->register_hw(WDTRST, this, (int*)0);
  register_cell(sfr, WDTRST, &wdtrst, wtd_restore);
  return(0);
}

void
cl_wdt::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == wdtrst &&
      (((*val)&0xff) == 0xe1) &&
      (wdtrst->get() == 0x1e) &&
      written_since_reset)
    {
      wdt= 0;
      /*uc->sim->app->get_commander()->
        debug("%g sec (%d tick): Watchdog timer enabled/reset PC= 0x%06x"
        "\n", uc->get_rtime(), uc->ticks->ticks, uc51r->PC);*/
    }
  written_since_reset= DD_TRUE;
}

int
cl_wdt::tick(int cycles)
{
  if (wdt >= 0)
    {
      wdt+= cycles;
      if (wdt > reset_value)
        {
          /*sim->app->get_commander()->
            debug("%g sec (%d ticks): Watchdog timer resets the CPU, "
            "PC= 0x%06x\n", get_rtime(), ticks->ticks, PC);*/
          uc->reset();
          //return(resWDTRESET);
        }
    }
  return(0);
}

void
cl_wdt::reset(void)
{
  written_since_reset= DD_FALSE;
  wdt= -1;
}

void
cl_wdt::print_info(class cl_console_base *con)
{
  con->dd_printf("%s[%d] %s counter=%d (remains=%d)\n", id_string, id,
                 (wdt>=0)?"ON":"OFF", wdt, (wdt>=0)?(reset_value-wdt):0);
}


/* End of s51.src/wdt.cc */
