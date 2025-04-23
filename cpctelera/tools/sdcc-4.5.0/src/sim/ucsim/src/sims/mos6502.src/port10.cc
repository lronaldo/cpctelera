/*
 * Simulator of microcontrollers (port10.cc)
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

#include "port10cl.h"


cl_port10::cl_port10(class cl_uc *auc, const char *aname):
  cl_hw(auc, HW_PORT, 0, aname)
{
}

int
cl_port10::init(void)
{
  class cl_cvar *v;
  cl_hw::init();
  // 1= output, 0= input
  cddr= (class cl_cell8 *)register_cell(uc->rom, 0);
  cdr= (class cl_cell8 *)register_cell(uc->rom, 1);
  cpin= (class cl_cell8 *)cfg->get_cell(port10_pin);
  uc->vars->add(v= new cl_var("port_dir", uc->rom, 0,
			      "Port Data Direction Register"));
  v->init();
  uc->vars->add(v= new cl_var("port_dr", uc->rom, 1,
			      "Port Data Register"));
  v->init();
  uc->vars->add(v= new cl_var("port_pin", cfg, port10_pin,
			      "Port pins for input bits"));
  v->init();
  uc->vars->add(v= new cl_var("port_val", cfg, port10_port,
			      "Port value"));
  v->init();
  return 0;
}

void
cl_port10::reset(void)
{
  cl_hw::reset();
  cddr->W(0);
}

u8_t
cl_port10::val()
{
  u8_t dir= cddr->get(); // 0=in 1=out
  u8_t dat= cdr->get();
  u8_t pin= cpin->get();

  return (dir & dat) | (~dir & pin);
}

t_mem
cl_port10::read(class cl_memory_cell *cell)
{
  if (conf(cell, NULL))
    return cell->get();
  if (cell == cdr)
    {
      if (!on)
	return uc->rom->get(1);
      return val();
    }
  return cell->get();
}

t_mem
cl_port10::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch (addr)
    {
    case port10_on:
      if (val)
	on= ((*val)!=0)?1:0;
      break;
    case port10_pin:
      if (val)
	cell->set(*val);
      break;
    case port10_port:
      cell->set(this->val());
      break;
    }
  return cell->get();
}

const char *
cl_port10::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case port10_on://	= 0, // RW
      return "Turn simulation of port on/off (bool, RW)";
    case port10_pin://	= 1, // RW
      return "Pin value for input bits (int, RW)";
    case port10_port://	= 2, // RO
      return "Combined port value (int, RO)";
    }
  return "N/A";
}

void
cl_port10::print_info(class cl_console_base *con)
{
  int m, dir= cddr->get(), dat= cdr->get(), pin= cpin->get(), v= val();
  con->dd_printf("%s[%d] %s\n", id_string, id, on?"ON":"OFF");
  con->dd_printf("Dir : 0x%02x ", dir);
  for (m=0x80; m; m>>=1)
    con->dd_printf("%c", (dir&m)?'O':'I');
  con->dd_printf("\n");
  con->dd_printf("Pin : 0x%02x ", pin);
  for (m=0x80; m; m>>=1)
    con->dd_printf("%c", (dir&m)?'-':((pin&m)?'1':'0'));
  con->dd_printf("\n");
  con->dd_printf("Data: 0x%02x ", dat);
  for (m=0x80; m; m>>=1)
    con->dd_printf("%c", (~dir&m)?'-':((dat&m)?'1':'0'));
  con->dd_printf("\n");
  con->dd_printf("Port: 0x%02x ", v);
  for (m=0x80; m; m>>=1)
    con->dd_printf("%c", (v&m)?'1':'0');
  con->dd_printf("\n");
}


/* End of mos6502.src/port10.cc */
