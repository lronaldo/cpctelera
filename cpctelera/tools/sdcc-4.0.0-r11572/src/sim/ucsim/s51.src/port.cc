/*
 * Simulator of microcontrollers (port.cc)
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

#include <stdio.h>
#include <ctype.h>

// cmd
#include "argcl.h"

// local
#include "regs51.h"
#include "types51.h"

#include "portcl.h"


cl_port::cl_port(class cl_uc *auc, int aid):
  cl_hw(auc, HW_PORT, aid, "port")
{
  port_pins= 0xff;
}

cl_port::cl_port(class cl_uc *auc, int aid, t_addr the_addr):
  cl_hw(auc, HW_PORT, aid, "port")
{
  port_pins= 0xff;
  addr_p= the_addr;
}

int
cl_port::init(void)
{
  cl_hw::init();
  switch (id)
    {
    case 0: addr_p= P0; break;
    case 1:
      {
	addr_p= P1;
	/*class cl_hw *hw;
	if ((hw= uc->get_hw(HW_TIMER, 2, 0)))
	hws_to_inform->add(hw);*/
	make_partner(HW_TIMER, 2);
	make_partner(HW_PCA, 0);
	break;
      }
    case 2: addr_p= P2; break;
    case 3:
      {
	addr_p= P3;
	//class cl_hw *hw;
	/*if ((hw= uc->get_hw(HW_TIMER, 0, 0)))
	  hws_to_inform->add(hw);
	if ((hw= uc->get_hw(HW_TIMER, 1, 0)))
	  hws_to_inform->add(hw);
	if ((hw= uc->get_hw(HW_DUMMY, 0, 0)))
	hws_to_inform->add(hw);*/
	make_partner(HW_TIMER, 0);
	make_partner(HW_TIMER, 1);
	make_partner(HW_INTERRUPT, 0);
	make_partner(HW_DUMMY, 0);
	break;
      }
    default: break;//addr_p= P0; return(1);
    }
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  bas= uc->address_space("bits");
  if (!sfr)
    {
      fprintf(stderr, "No SFR to register port into\n");
    }
  cell_p= register_cell(sfr, addr_p);
  if ((addr_p % 8) == 0)
    {
      int i;
      for (i= 0; i < 8; i++)
	bit_cells[i]= register_cell(bas, addr_p+i);
    }
  prev= cell_p->get();
  cell_in= cfg->get_cell(port_pin);
  cfg->set(port_value, prev & cell_in->get());
  
  cl_var *v;
  chars pn;
  pn= cchars("port");
  pn.append("%d_", id);
  uc->vars->add(v= new cl_var(pn+chars("on"), cfg, port_on,
			      cfg_help(port_on)));
  v->init();
  uc->vars->add(v= new cl_var(pn+chars("pin"), cfg, port_pin,
			      cfg_help(port_pin)));
  v->init();
  uc->vars->add(v= new cl_var(pn+chars("pins"), cfg, port_pin,
			      cfg_help(port_pin)));
  v->init();
  uc->vars->add(v= new cl_var(pn+chars("value"), cfg, port_value,
			      cfg_help(port_value)));
  v->init();
  chars p= chars("pin");
  p.append("%d", id);
  uc->vars->add(v= new cl_var(p, cfg, port_pin,
			      cfg_help(port_pin)));
  v->init();
  p= chars("pins");
  p.append("%d", id);
  uc->vars->add(v= new cl_var(p, cfg, port_pin,
			      cfg_help(port_pin)));
  v->init();
  
  return(0);
}

char *
cl_port::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case port_on: return (char*)"Turn/get on/off state (bool, RW)";
    case port_pin: return (char*)"Outside value of port pins (int, RW)";
    case port_value: return (char*)"Value of the port (int, RO)";
    }
  return (char*)"Not used";
}

t_mem
cl_port::read(class cl_memory_cell *cell)
{
  if (cell == cell_p)
    return(cell->get() & port_pins);
  t_addr ba;
  if (bas->is_owned(cell, &ba))
    {
      int bi= ba - addr_p;
      bool cv= cell->get();
      bool pv= port_pins & (1 << bi);
      return (cv && pv)?1:0;
    }
  conf(cell, NULL);
  return cell->get();
}

void
cl_port::write(class cl_memory_cell *cell, t_mem *val)
{
  struct ev_port_changed ep;
  u8_t nv= *val;
  t_addr ba;
  
  if (cell == cell_p)
    {
      (*val)&= 0xff; // 8 bit port
      nv= *val;
      if ((port_pins & nv) != cfg->get(port_value))
	cfg->write(port_value, port_pins & nv);
    }
  
  if (bas->is_owned(cell, &ba))
    {
      int bi= ba - addr_p;
      int m= 1 << bi;
      nv= cell_p->get();
      if (*val)
	nv|= m;
      else
	nv&= ~m;
      if ((port_pins & nv) != cfg->get(port_value))
	cfg->write(port_value, port_pins & nv);
    }

  conf(cell, val);
  
  ep.id= id;
  ep.addr= addr_p;
  ep.prev_value= cell_p->get();
  ep.new_value= nv;
  ep.pins= ep.new_pins= port_pins;
  if (ep.prev_value != ep.new_value)
    inform_partners(EV_PORT_CHANGED, &ep);
  prev= cell_p->get();
}

t_mem
cl_port::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
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
      if (val)
	set_pin(*val);
      else
	cell->set(port_pins);
      break;
    case port_value:
      if (val)
	*val= cell->get();//cell->set(*val);
      break;
    }
  return cell->get();
}

void
cl_port::set_pin(t_mem val)
{
  struct ev_port_changed ep;
  t_mem value= val & 0xff;

  ep.id= id;
  ep.addr= addr_p;
  ep.pins= port_pins;
  port_pins= value;
  ep.prev_value= cell_p->get();
  ep.new_value= cell_p->get();
  ep.new_pins= port_pins;
  if (ep.pins != ep.new_pins)
    inform_partners(EV_PORT_CHANGED, &ep);
  if ((port_pins & ep.prev_value) != cfg->get(port_value))
    cfg->write(port_value, port_pins & ep.prev_value);
}

void
cl_port::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
  class cl_cmd_arg *params[1]= { cmdline->param(0) };

  if (cmdline->syntax_match(uc, NUMBER))
    {
      set_pin(params[0]->value.number);
    }
  else
    {
      con->dd_printf("set hardware port[%d] pins_value\n                   Set port pins\n",
		     id);
    }
}

void
cl_port::print_info(class cl_console_base *con)
{
  uchar data;

  con->dd_printf("%s[%d]\n", id_string, id);
  data= cell_p->get();
  con->dd_printf("P%d    ", id);
  con->print_bin(data, 8);
  con->dd_printf(" 0x%02x %3d %c (Value in SFR register)\n",
		 data, data, isprint(data)?data:'.');

  data= /*uc->*/port_pins/*[id]*/;
  con->dd_printf("Pin%d  ", id);
  con->print_bin(data, 8);
  con->dd_printf(" 0x%02x %3d %c (Output of outside circuits)\n",
		 data, data, isprint(data)?data:'.');

  data= cell_p->read();
  con->dd_printf("Port%d ", id);
  con->print_bin(data, 8);
  con->dd_printf(" 0x%02x %3d %c (Value on the port pins)\n",
		 data, data, isprint(data)?data:'.');
  print_cfg_info(con);
}


/* End of s51.src/port.cc */
