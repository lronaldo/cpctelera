/*
 * Simulator of microcontrollers (bus.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include "buscl.h"


cl_bus::cl_bus(class cl_uc *auc):
  cl_hw(auc, HW_GPIO, 0, "bus")
{
}

int
cl_bus::init(void)
{
  cl_hw::init();
  //uc->vars->add("bus_on", cfg, tbus_on, cfg_help(tbus_on));
  uc->vars->add("bus_in", cfg, tbus_in, cfg_help(tbus_in));
  uc->vars->add("bus0_in", cfg, tbus_in, cfg_help(tbus_in));
  uc->vars->add("bus_out", cfg, tbus_out, cfg_help(tbus_out));
  uc->vars->add("bus0_out", cfg, tbus_out, cfg_help(tbus_out));
  out_ff= cfg_cell(tbus_out);
  return 0;
}

void
cl_bus::reset(void)
{
}

void
cl_bus::print_info(class cl_console_base *con)
{
  con->dd_printf("IN=0x%02x OUT=0x%02x",
		 cfg_get(tbus_in), cfg_get(tbus_out));
  con->dd_printf("\n");
}


const char *
cl_bus::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case tbus_on: return cl_hw::cfg_help(addr);
    case tbus_in: return "Outside value of pins (8 bit, RW)";
    case tbus_out: return "Value of output latch (8 bit, RW)";
    }
  return "Not used";
}

t_mem
cl_bus::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch (addr)
    {
    case tbus_on: return cl_hw::conf_op(cell, addr, val);
    case tbus_in:
      if (val)
	{
	  *val&= 0xff;
	  cell->set(*val);
	}
      break;
    case tbus_out:
      if (val)
	{
	  *val&= 0xff;
	  cell->set(*val);
	}
      break;
    }
  return cell->get();
}

void
cl_bus::latch(u8_t val)
{
  out_ff->set(val);
}


/* End of i8048.src/bus.cc */
