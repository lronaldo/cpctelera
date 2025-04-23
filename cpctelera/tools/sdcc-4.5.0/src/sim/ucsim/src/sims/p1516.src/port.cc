/*
 * Simulator of microcontrollers (port.cc)
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

#include "portcl.h"


/* OUTPUT port */

cl_porto::cl_porto(class cl_uc *auc, t_addr the_addr, const char *aname):
  cl_hw(auc, HW_PORT, 0, aname)
{
  addr= the_addr;
  set_name(aname);
}

int
cl_porto::init(void)
{
  cl_hw::init();
  dr= register_cell(uc->rom, addr);
  dr->decode((t_mem*)&value);

  uc->vars->add(get_name(), uc->rom, addr, 31, 0, chars("", "Data register of %s", get_name()));

  cache= 0;
  
  return 0;
}

void
cl_porto::reset(void)
{
  value= 0;
}

void
cl_porto::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == dr)
    {
      cell->set(*val);
    }

  conf(cell, val);
}

/*
t_mem
cl_porto::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum port_cfg)addr)
    {
    case port_on: // turn this HW on/off
      if (val)
	{
	  if (*val)
	    on= true;
	  else
	    on= false;
	}
      else
	{
	  cell->set(on?1:0);
	}
      break;
    case port_pin: // get/set PINS
      break;
    case port_value:
      if (val)
	*val= dr->get();//cell->set(*val);
      break;
    }
  return cell->get();
}
*/

void
cl_porto::print(class cl_console_base *con)
{
  u32_t m;
  t_mem d= value;//dr->get();
  con->dd_printf("%s at %04x %08x\n", get_name(), addr, d);
  for (m= 0x80000000; m; m>>= 1)
    {
      //if (d & m)
	con->dd_printf("%c", (d & m)?'1':'0');
	//else 	con->dd_printf("-");
    }
  con->dd_printf("\n");
}

void
cl_porto::print_info(class cl_console_base *con)
{
  print(con);
  //print_cfg_info(con);
}

void
cl_porto::refresh_display(bool force)
{
  class cl_port_io *pio= (class cl_port_io *)io;
  if (!io)
    return;

  t_mem d= value;
  if (force || d != cache)
    {
      t_mem m;
      int b, bv;
      pio->tu_go(1,6);
      pio->dd_color("answer");
      pio->dd_printf("%s at %04x %08x\n", get_name(), addr, d);
      cache= d;
      pio->tu_go(1,8);
      for (b=31; b>=0; b--)
	{
	  m= 1<<b;
	  bv= (d&m)?1:0;
	  pio->dd_printf("%d", bv);
	  if (b%8 == 0) pio->dd_printf(" ");
	}
      pio->tu_go(1,9);
      for (b=31; b>=0; b--)
	{
	  m= 1<<b;
	  bv= (d&m)?1:0;
	  if (bv)
	    {
	      pio->dd_cprintf("ui_bit1", "O");
	    }
	  else
	    {
	      pio->dd_cprintf("ui_bit0", ".");
	    }
	  if (b%8 == 0) pio->dd_printf(" ");
	}
    }
  pio->dd_color("answer");
  
  draw_state_time(force);
}

void
cl_porto::draw_display(void)
{
  class cl_port_io *pio= (class cl_port_io *)io;
  if (!io)
    return;
  pio->tu_cls();

  cl_hw::draw_display();

  pio->tu_go(1,5);
  pio->dd_printf("OPORT %s", get_name());

  pio->tu_go(1,10);
  int b;
  for (b=31; b>=0; b--)
    {
      pio->dd_printf("%d", b/10);
      if (b%8 == 0) pio->dd_printf(" ");
    }
  pio->tu_go(1,11);
  for (b=31; b>=0; b--)
    {
      pio->dd_printf("%d", b%10);
      if (b%8 == 0) pio->dd_printf(" ");
    }
  refresh_display(true);
}


/* INPUT port */

cl_porti::cl_porti(class cl_uc *auc, t_addr the_addr, const char *aname):
  cl_porto(auc, the_addr, aname)
{
  value= 0;
}

int
cl_porti::init(void)
{
  cl_porto::init();

  uc->vars->add(chars("", "%s_pins", get_name()), cfg, port_pin, 31, 0, cfg_help(port_pin));

  return 0;
}

const char *
cl_porti::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case port_pin: return "Outside value of port pins (int, RW)";
    }
  return "Not used";
}

void
cl_porti::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == dr)
    {
      *val= value;
    }

  conf(cell, val);
}


t_mem
cl_porti::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum port_cfg)addr)
    {
    case port_pin: // get/set PINS
      if (val)
	value= *val;
      else
	return value;
      break;
    default:
      return cl_porto::conf_op(cell, addr, val);
    }
  return cell->get();
}

/* End of p1516.src/port.cc */

