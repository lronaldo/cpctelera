/*
 * Simulator of microcontrollers (wdt.cc)
 *
 * Copyright (C) 2024 Drotos Daniel
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

#include "osccl.h"

#include "wdtcl.h"


cl_wdt::cl_wdt(class cl_uc *auc, const char *aname):
  cl_hw(auc, HW_TIMER, 0, aname)
{
  puc= (class cl_pdk *)auc;
}

int
cl_wdt::init(void)
{
  mod= register_cell(puc->sfr, 3);
  misc= register_cell(puc->sfr, 0x49);
  cl_hw::init();
  uc->mk_mvar(cfg, wdt_cnt, "WDT", cfg_help(wdt_cnt));
  reset();
  return 0;
}

const char *
cl_wdt::cfg_help(t_addr addr)
{
  switch ((enum wdt_cfg)addr)
    {
    case wdt_on: return "Turn ticking of WDT on/off (bool, RW)";
    case wdt_cnt: return "WDT counter value (RW)";
    case wdt_nuof: return "";
    }
  return "Not used";
}

void
cl_wdt::reset(void)
{
  cnt= 0;
  last= 0;
  mod->set(0);
  recalc();
}

void
cl_wdt::recalc(void)
{
  run= mod->get() & 2;
  set_len();
  last= puc->osc->ilrc;
}

void
cl_wdt::set_len(void)
{
  u8_t v= misc->get();
  switch (v&3)
    {
    case 0: len= 8192; break;
    case 1: len= 16384; break;
    case 2: len= 65536; break;
    case 3: len= 262144; break;
    }
}

void
cl_wdt::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  if (cell == mod)
    {
      if ((*val & 0xff) != cell->get())
	{
	  cell->set(*val);
	  recalc();
	}
    }
  else if (cell == misc)
    {
      if ((*val & 0xff) != cell->get())
	{
	  cell->set(*val);
	  recalc();
	}
    }
  /*else if (cell == irq)
    {
    }*/
  cell->set(*val);
}

t_mem
cl_wdt::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch (addr)
    {
    case wdt_on: // turn this HW on/off
      if (val)
	on= *val;
      else
	cell->set(on?1:0);
      break;
    case wdt_cnt:
      if (val)
	cnt= *val & 0xffffff;
      else
	cell->set(cnt);
      break;
    }
    return cell->get();
}

int
cl_wdt::tick(int cycles)
{
  t_mem act= puc->osc->ilrc;
  if (run && (act != last))
    {
      int d= act - last, i;
      if (d<0) d= -d;
      if (d)
	{
	  cnt+= d;
	  if (cnt > len)
	    puc->reset();
	}
      last= act;
    }
  return 0;
}

void
cl_wdt::print_info(class cl_console_base *con)
{
  con->dd_printf("WDT run=%s len=%u\n",
		 run?"YES":"NO", MU(len));
  con->dd_printf("cnt= %6u 0x%08x\n", MU(cnt), MU(cnt));
}


/* End of pdk.src/wdt.cc */
