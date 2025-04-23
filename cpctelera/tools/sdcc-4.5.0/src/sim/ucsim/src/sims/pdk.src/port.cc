/*
 * Simulator of microcontrollers (pdk.src/port.cc)
 *
 * Copyright (C) 2017 Drotos Daniel
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

#include "portcl.h"

cl_port::cl_port(class cl_uc *auc, t_addr abase/*, int aid*/, const char *aname):
	cl_hw(auc, HW_PORT, /*aid*/0, aname)
{
  base = abase;
  set_name(aname);
}

int
cl_port::init(void)
{
  cl_hw::init();
  // ODR
  cell_p= register_cell(uc->rom, base + 0);
  // IDR
  cell_in= register_cell(uc->rom, base + 1);
  // DDR: 0=input, 1=output
  cell_dir= register_cell(uc->rom, base + 2);

  chars pn= get_name();
  uc->vars->add(pn+"_ddr", uc->rom, base+2, 7, 0,
                         "Direction register");
  uc->vars->add(pn+"_odr", uc->rom, base+0, 7, 0,
                         "Output data register");
  uc->vars->add(pn+"_idr", uc->rom, base+1, 7, 0,
                         "Input data register (outside value of port pins)");
  uc->vars->add(pn+"_pin", uc->rom, base+1, 7, 0,
                         "Outside value of port pins");
  uc->vars->add(pn+"_pins", uc->rom, base+1, 7, 0,
                         "Outside value of port pins");

  return 0;
}

void
cl_port::reset(void)
{
  cell_dir->write(0);
  cell_p->write(0);
  cell_in->write(0);
}

void
cl_port::write(class cl_memory_cell *cell, t_mem *val)
{
  if ((cell == cell_p) ||
      (cell == cell_in) ||
      (cell == cell_dir))
    {
      cell->set(*val);
      t_mem p= cell_p->get();
      t_mem i= cell_in->get();
      t_mem d= cell_dir->get();
      i&= ~d;
      i|= (p & d);
      cell_in->set(i);
      if (cell == cell_in)
	*val= i;
    }
}

void
cl_port::print_info(class cl_console_base *con)
{
  int m;
  t_mem o= cell_p->get(),
    i= cell_in->get(),
    d= cell_dir->get();
  con->dd_printf("%s at 0x%04x\n", get_name(), base);
  con->dd_printf("dir: 0x%02x ", d);
  for (m= 0x80; m; m>>= 1)
    con->dd_printf("%c", (d & m)?'O':'I');
  con->dd_printf("\n");
  con->dd_printf("out: 0x%02x ", o);
  for (m= 0x80; m; m>>= 1)
    {
      if (d & m)
	con->dd_printf("%c", (o & m)?'1':'0');
      else
	con->dd_printf("-");
    }
  con->dd_printf("\n");
  con->dd_printf("in : 0x%02x ", i);
  for (m= 0x80; m; m>>= 1)
    {
      //if (!(d & m))
	con->dd_printf("%c", (i & m)?'1':'0');
	//else
	//con->dd_printf("-");
    }
  con->dd_printf("\n");
  //print_cfg_info(con);
}


/* End of pdk.src/port.cc */
