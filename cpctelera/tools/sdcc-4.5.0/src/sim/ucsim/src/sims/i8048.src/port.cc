/*
 * Simulator of microcontrollers (port.cc)
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

#include <ctype.h>

#include "portcl.h"


cl_qport::cl_qport(class cl_uc *auc, int aid,
		   class cl_address_space *apas, t_addr aaddr,
		   enum port_widths awidth):
  cl_hw(auc, HW_PORT, aid, "port")
{
  port_as= apas;
  addr= aaddr;
  width= awidth;
  switch (width)
    {
    case port_4bit: mask= 0x0f; break;
    case port_8bit: mask= 0xff; break;
    }
}

cl_qport::cl_qport(class cl_uc *auc, int aid,
		   class cl_address_space *apas, t_addr aaddr,
		   enum port_widths awidth,
		   const char *aid_string):
  cl_hw(auc, HW_PORT, aid, aid_string)
{
  port_as= apas;
  addr= aaddr;
  width= awidth;
  switch (width)
    {
    case port_4bit: mask= 0x0f; break;
    case port_8bit: mask= 0xff; break;
    }
}

int
cl_qport::init(void)
{
  cl_hw::init();
  pcell= register_cell(port_as, addr);
  cfg_set(port_pin, 0xff & mask);
  chars n;
  n.format("port%d_", id);
  uc->vars->add(n+"pin", cfg, port_pin, cfg_help(port_pin));
  uc->vars->add(n+"value", cfg, port_value, cfg_help(port_value));
  uc->vars->add(n+"odr", cfg, port_odr, cfg_help(port_odr));
  return 0;
}

const char *
cl_qport::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case port_on: return "Turn/get on/off state (bool, RW)";
    case port_pin: return "Outside value of port pins (int, RW)";
    case port_value: return "Value of the port (int, RO)";
    case port_odr: return "Value of output register (int, RW)";
    }
  return "Not used";
}

t_mem
cl_qport::read(class cl_memory_cell *cell)
{
  if (cell == pcell)
    return(cell->get() & cfg_get(port_pin));
  conf(cell, NULL);
  return cell->get();
}

void
cl_qport::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == pcell) cfg_set(port_odr, *val & mask);
  conf(cell, val);
}

t_mem
cl_qport::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum port_cfg)addr)
    {
    case port_on: // turn this HW on/off
      if (val)
	{
	  on= (*val)?1:0;
	}
      cell->set(on?1:0);
      break;
    case port_pin: // get/set PINS
      if (val)
	cell->set(*val & mask);
      break;
    case port_value: // odr & pins (RO)
      cell->set(pcell->get() & cfg_get(port_pin) & mask);
      break;
    case port_odr: // copy value into memory cell
      if (val)
	pcell->set(*val & mask);
      cell->set(pcell->get());
      break;
    case port_nuof:
      break;
    }
  return cell->get();
}


void
cl_qport::print_info(class cl_console_base *con)
{
  uchar data;

  con->dd_printf("%s[%d]\n", id_string, id);
  data= pcell->get() & mask;
  con->dd_printf("P%d    ", id);
  con->print_bin(data, width);
  con->dd_printf(" 0x%02x %3d %c (Value in SFR register)\n",
		 data, data, isprint(data)?data:'.');

  data= cfg_get(port_pin) & mask;
  con->dd_printf("Pin%d  ", id);
  con->print_bin(data, width);
  con->dd_printf(" 0x%02x %3d %c (Output of outside circuits)\n",
		 data, data, isprint(data)?data:'.');

  data= pcell->read() & mask;
  con->dd_printf("Port%d ", id);
  con->print_bin(data, width);
  con->dd_printf(" 0x%02x %3d %c (Value on the port pins)\n",
		 data, data, isprint(data)?data:'.');
  //print_cfg_info(con);
}


/*
 * P2 port
 */

cl_p2::cl_p2(class cl_uc *auc, int aid,
	     class cl_address_space *apas, t_addr aaddr,
	     enum port_widths awidth):
  cl_qport(auc, aid, apas, aaddr, awidth)
{
}

void
cl_p2::set_low(u8_t val)
{
  u8_t r= cfg_get(port_odr);
  r&= 0xf0;
  val&= 0xf;
  r|= val;
  cfg_write(port_odr, val);
}


/*
 * 8243 Port extender
 */

cl_pext::cl_pext(class cl_uc *auc, int aid,
		 class cl_address_space *apas, t_addr aaddr,
		 class cl_p2 *the_p2):
  cl_qport(auc, aid, apas, aaddr, port_4bit, "pext")
{
  p2= the_p2;
}

int
cl_pext::init(void)
{
  cl_hw::init();
  pcell4= register_cell(port_as, addr+0);
  pcell5= register_cell(port_as, addr+1);
  pcell6= register_cell(port_as, addr+2);
  pcell7= register_cell(port_as, addr+3);
  cfg_set(pext_pin4, 0);
  cfg_set(pext_pin5, 0);
  cfg_set(pext_pin6, 0);
  cfg_set(pext_pin7, 0);
  cfg_write(pext_dir4, 0);
  cfg_write(pext_dir5, 0);
  cfg_write(pext_dir6, 0);
  cfg_write(pext_dir7, 0);
  cfg_write(pext_odr4, 0);
  cfg_write(pext_odr5, 0); 
  cfg_write(pext_odr6, 0);
  cfg_write(pext_odr7, 0);
  chars n;
  n.format("pext%d_", id);
  uc->vars->add(n+"pin4", cfg, pext_pin4, cfg_help(pext_pin4));
  uc->vars->add(n+"pin5", cfg, pext_pin5, cfg_help(pext_pin5));
  uc->vars->add(n+"pin6", cfg, pext_pin6, cfg_help(pext_pin6));
  uc->vars->add(n+"pin7", cfg, pext_pin7, cfg_help(pext_pin7));
  uc->vars->add(n+"odr4", cfg, pext_odr4, cfg_help(pext_odr4));
  uc->vars->add(n+"odr5", cfg, pext_odr5, cfg_help(pext_odr5));
  uc->vars->add(n+"odr6", cfg, pext_odr6, cfg_help(pext_odr6));
  uc->vars->add(n+"odr7", cfg, pext_odr7, cfg_help(pext_odr7));
  uc->vars->add(n+"dir4", cfg, pext_dir4, cfg_help(pext_dir4));
  uc->vars->add(n+"dir5", cfg, pext_dir5, cfg_help(pext_dir5));
  uc->vars->add(n+"dir6", cfg, pext_dir6, cfg_help(pext_dir6));
  uc->vars->add(n+"dir7", cfg, pext_dir7, cfg_help(pext_dir7));
  return 0;
}

void
cl_pext::reset(void)
{
  cfg_write(pext_dir4, 0);
  cfg_write(pext_dir5, 0);
  cfg_write(pext_dir6, 0);
  cfg_write(pext_dir7, 0);
}

const char *
cl_pext::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case pext_on: return "Turn/get on/off state (bool, RW)";
    case pext_pin4: return "Outside value of port 4 pins (int, RW)";
    case pext_pin5: return "Outside value of port 5 pins (int, RW)";
    case pext_pin6: return "Outside value of port 6 pins (int, RW)";
    case pext_pin7: return "Outside value of port 7 pins (int, RW)";
    case pext_odr4: return "Value of output 4 (int, RW)";
    case pext_odr5: return "Value of output 5 (int, RW)";
    case pext_odr6: return "Value of output 6 (int, RW)";
    case pext_odr7: return "Value of output 7 (int, RW)";
    case pext_dir4: return "Direction of port 4, In=0,Out=1 (int, RW)";
    case pext_dir5: return "Direction of port 5, In=0,Out=1 (int, RW)";
    case pext_dir6: return "Direction of port 6, In=0,Out=1 (int, RW)";
    case pext_dir7: return "Derection of port 7, In=0,Out=1 (int, RW)";
    }
  return "Not used";
}

t_mem
cl_pext::read(class cl_memory_cell *cell)
{
  if (cell == pcell4)
    return(cfg_get(pext_pin4));
  if (cell == pcell5)
    return(cfg_get(pext_pin5));
  if (cell == pcell6)
    return(cfg_get(pext_pin6));
  if (cell == pcell7)
    return(cfg_get(pext_pin7));
  conf(cell, NULL);
  return cell->get();
}


void
cl_pext::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == pcell4) cfg_set(pext_odr4, *val & mask);
  if (cell == pcell5) cfg_set(pext_odr5, *val & mask);
  if (cell == pcell6) cfg_set(pext_odr6, *val & mask);
  if (cell == pcell7) cfg_set(pext_odr7, *val & mask);
  conf(cell, val);
}

t_mem
cl_pext::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum pext_cfg)addr)
    {
    case pext_on: // turn this HW on/off
      if (val)
	{
	  on= (*val)?1:0;
	}
      cell->set(on?1:0);
      break;
    case pext_pin4: // get/set PINS
    case pext_pin5:
    case pext_pin6:
    case pext_pin7:
      if (val)
	cell->set(*val & mask);
      break;
    case pext_dir4: // set direction, 0=in 1=out
    case pext_dir5:
    case pext_dir6:
    case pext_dir7:
      if (val)
	{
	  if (*val)
	    *val= mask;	    
	  cell->set(*val);
	}
      break;
    case pext_odr4: // copy value into memory cell
      if (val)
	pcell4->set(*val & mask);
      cell->set(pcell4->get());
      break;
    case pext_odr5: // copy value into memory cell
      if (val)
	pcell5->set(*val & mask);
      cell->set(pcell5->get());
      break;
    case pext_odr6: // copy value into memory cell
      if (val)
	pcell6->set(*val & mask);
      cell->set(pcell6->get());
      break;
    case pext_odr7: // copy value into memory cell
      if (val)
	pcell7->set(*val & mask);
      cell->set(pcell7->get());
      break;
    case pext_nuof:
      break;
    }
  return cell->get();
}

u8_t
cl_pext::in(u8_t addr)
{
  int rdir, rpin;
  u8_t r;
  switch (addr)
    {
    case 4: rdir= pext_dir4; rpin= pext_pin4; break;
    case 5: rdir= pext_dir5; rpin= pext_pin5; break;
    case 6: rdir= pext_dir6; rpin= pext_pin6; break;
    case 7: rdir= pext_dir7; rpin= pext_pin7; break;
    default: return 0;
    }
  cfg_write(rdir, 0);
  r= cfg_read(rpin);
  p2->set_low(r);
  return r;
}

void
cl_pext::out(u8_t addr, u8_t val)
{
  int rdir, rodr;
  switch (addr)
    {
    case 4: rdir= pext_dir4; rodr= pext_odr4; break;
    case 5: rdir= pext_dir5; rodr= pext_odr5; break;
    case 6: rdir= pext_dir6; rodr= pext_odr6; break;
    case 7: rdir= pext_dir7; rodr= pext_odr7; break;
    default: return ;
    }
  cfg_write(rdir, 1);
  cfg_write(rodr, val);
  p2->set_low(val);
}

void
cl_pext::orl(u8_t addr, u8_t val)
{
  int rdir, rodr;
  switch (addr)
    {
    case 4: rdir= pext_dir4; rodr= pext_odr4; break;
    case 5: rdir= pext_dir5; rodr= pext_odr5; break;
    case 6: rdir= pext_dir6; rodr= pext_odr6; break;
    case 7: rdir= pext_dir7; rodr= pext_odr7; break;
    default: return ;
    }
  cfg_write(rdir, 1);
  val&= 0xf;
  u8_t r= cfg_read(rodr);
  val|= r;
  cfg_write(rodr, val);
  p2->set_low(val);
}

void
cl_pext::anl(u8_t addr, u8_t val)
{
  int rdir, rodr;
  switch (addr)
    {
    case 4: rdir= pext_dir4; rodr= pext_odr4; break;
    case 5: rdir= pext_dir5; rodr= pext_odr5; break;
    case 6: rdir= pext_dir6; rodr= pext_odr6; break;
    case 7: rdir= pext_dir7; rodr= pext_odr7; break;
    default: return ;
    }
  cfg_write(rdir, 1);
  val&= 0xf;
  u8_t r= cfg_read(rodr);
  val&= r;
  cfg_write(rodr, val);
  p2->set_low(val);
}  

void
cl_pext::print_info(class cl_console_base *con)
{
  con->dd_printf("port4 --> ");
  if (cfg_get(pext_dir4))
    con->print_bin(cfg_get(pext_odr4), width);
  else
    con->dd_printf("xxxx");
  con->dd_printf("    ");
  con->dd_printf("port5 --> ");
  if (cfg_get(pext_dir4))
    con->print_bin(cfg_get(pext_odr5), width);
  else
    con->dd_printf("xxxx");
  con->dd_printf("    ");
  con->dd_printf("port6 --> ");
  if (cfg_get(pext_dir4))
    con->print_bin(cfg_get(pext_odr6), width);
  else
    con->dd_printf("xxxx");
  con->dd_printf("    ");
  con->dd_printf("port7 --> ");
  if (cfg_get(pext_dir4))
   con->print_bin(cfg_get(pext_odr7), width);
  else
    con->dd_printf("xxxx");
  con->dd_printf("    ");
  con->dd_printf("\n");

  con->dd_printf("port4 <-- ");
  con->print_bin(cfg_get(pext_pin4), width);
  con->dd_printf("    ");
  con->dd_printf("port5 <-- ");
  con->print_bin(cfg_get(pext_pin5), width);
  con->dd_printf("    ");
  con->dd_printf("port6 <-- ");
  con->print_bin(cfg_get(pext_pin6), width);
  con->dd_printf("    ");
  con->dd_printf("port7 <-- ");
  con->print_bin(cfg_get(pext_pin7), width);
  con->dd_printf("    ");
  con->dd_printf("\n");
}


/* End of i8048.src/port.cc */
