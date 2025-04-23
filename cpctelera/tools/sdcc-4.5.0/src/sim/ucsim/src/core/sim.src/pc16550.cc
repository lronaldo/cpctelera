/*
 * Simulator of microcontrollers (pc16550.cc)
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "utils.h"
#include "globals.h"
//#include "fiocl.h"

#include "pc16550cl.h"

cl_pc16550::cl_pc16550(class cl_uc *auc, int aid):
  cl_serial_hw(auc, aid, chars("pc16550"))
{
  as= NULL;
  base= 0;
}

int
cl_pc16550::init(void)
{
  cl_serial_hw::init();
  cfcr.decode(&fcr);
  cdll.decode(&dll);
  cdlm.decode(&dlm);
  ten= ren= true;
  return 0;
}

const char *
cl_pc16550::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case pc16550_cfg_base:
      return "Base address of pc16550 registers (int, RW)";
    }
  return cl_serial_hw::cfg_help(addr);
}

t_mem
cl_pc16550::read(class cl_memory_cell *cell)
{
  if (cell == regs[rdr])
    {
      if (dlab)
	{
	  return dll;
	}
      else
	{
	  cfg_set(serconf_able_receive, 1);
	  show_readable(false);
	  return s_in;
	}
    }
  if (cell == regs[rier])
    {
      if (dlab)
	return dlm;
      return cell->get();
    }
  /*if (cell == regs[stat])
    return r_sr->read();*/
  conf(cell, NULL);
  return cell->get();
}

void
cl_pc16550::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  if (cell == regs[rier])
    {
      if (dlab)
	{
	  dlm= *val;
	  *val= cell->get();
	}
      else
	{
	  *val&= 0xf;
	}
    }
  else if (cell == regs[rlcr])
    {
      dlab= *val & 0x80;
    }
  else if (cell == regs[rlsr])
    {
      *val= cell->get();
    }
  else
    {
      if (cell == regs[tdr])
	{
	  if (dlab)
	    {
	      dll= *val;
	      *val= cell->get();
	    }
	  else
	    {
	      //cell->set(*val);
	      s_txd= *val & 0xff;
	      s_tx_written= true;
	      show_writable(false);
	      if (!s_sending)
		{
		  start_send();
		}
	      *val= cell->get();
	    }
	}
    }
}


t_mem
cl_pc16550::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  if (addr < serconf_nr)
    return cl_serial_hw::conf_op(cell, addr, val);
  switch ((enum pc16550_cfg)addr)
    {
    case pc16550_cfg_base:
      if (val)
	{
	  int i;
	  if (as && as->valid_address(*val))
	    {
	      map(as, (*val)&0xffff);
	    }
	}
      else
	{
	  cell->set(base);
	}
      break;
    default:
      break;
    }
  return cell->get();
}


bool
cl_pc16550::set_cmd(class cl_cmdline *cmdline,
		    class cl_console_base *con)
{
  class cl_cmd_arg *params[2]= {
    cmdline->param(0),
    cmdline->param(1)
  };

  if (cmdline->syntax_match(uc, MEMORY ADDRESS))
    {
      class cl_memory *mem= params[0]->value.memory.memory;
      t_addr a= params[1]->value.address;
      if (!mem->is_address_space())
	{
	  con->dd_printf("%s is not an address space\n");
	  return true;
	}
      if (!mem->valid_address(a))
	{
	  con->dd_printf("Address must be between 0x%x and 0x%x\n",
			 AU(mem->lowest_valid_address()),
			 AU(mem->highest_valid_address()));
	  return true;
	}
      if (mem != 0)
	map((class cl_address_space *)mem, a);
      return true;
    }
  else if (cmdline->syntax_match(uc, NUMBER))
    {
      int i;
      t_addr a= params[0]->value.number;
      if (!as->valid_address(a))
	{
	  con->dd_printf("Address must be between 0x%x and 0x%x\n",
			 AU(as->lowest_valid_address()),
			 AU(as->highest_valid_address()));
	  return true;
	}
      map(as, a);
      return true; // handled
    }
  return cl_serial_hw::set_cmd(cmdline, con);
}

void
cl_pc16550::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware %s[%d] address\n", get_name(), id);
  cl_serial_hw::set_help(con);
}


int
cl_pc16550::tick(int cycles)
{
  char c;
  
  if (!on)
    return 0;

  if (!prediv_bitcnt(cycles))
    return 0;
  
  if (s_sending &&
      (s_tr_bit >= bits))
    {
      cl_f *fo= io->get_fout();
      if ((fo == NULL) || (fo->writable()))
	{
	  s_sending= false;
	  io->write((char*)&s_out, 1);
	  s_tr_bit-= 8;
	  if (s_tx_written)
	    restart_send();
	  else
	    finish_send();
	}
    }
  if (/*(ren) &&*/
      //io->get_fin() &&
      !s_receiving)
    {
      if (cfg_get(serconf_check_often))
	{
	  if (io->get_fin())
	    {
	      if (io->input_avail())
		io->proc_input(0);
	    }
	}
      if (input_avail)
	{
	  s_receiving= true;
	  s_rec_bit= 0;
	}
      else
	show_idle(true);
    }
  if (s_receiving &&
      (s_rec_bit >= bits))
    {
	{
	  c= get_input();
	  //input_avail= false;
	  s_in= c;
	  received();
	}
      s_receiving= false;
      s_rec_bit-= 8;
    }
  
  return(0);
}

void
cl_pc16550::start_send()
{
  //if (ten)
    {
      s_out= s_txd;
      s_tx_written= false;
      s_sending= true;
      s_tr_bit= 0;
      show_writable(true);
    }
}

void
cl_pc16550::restart_send()
{
  //if (ten)
    {
      s_out= s_txd;
      s_tx_written= false;
      s_sending= true;
      s_tr_bit= 0;
      show_writable(true);
    }
}

void
cl_pc16550::finish_send()
{
  show_writable(true);
  show_tx_complete(true);
}

void
cl_pc16550::received()
{
  set_rdr(s_in);
  cfg_write(serconf_received, s_in);
  show_readable(true);
}

void
cl_pc16550::reset(void)
{
  if (!as)
    return;
  show_writable(true);
  show_readable(false);
  
  pick_div();
  pick_ctrl();
}


void
cl_pc16550::pick_div()
{

  mcnt= 0;
}

void
cl_pc16550::pick_ctrl()
{
  //u32_t r= regs[ctrl]->get();
  //ren= r & 1;
  //ten= r & 2;
  s_rec_bit= s_tr_bit= 0;
  s_receiving= false;
  s_tx_written= false;
}


void
cl_pc16550::show_writable(bool val)
{
  u8_t r= regs[rlsr]->get();
  if (val)
    r|= 0x20;
  else
    r&= ~0x20;
  regs[rlsr]->set(r);
}

void
cl_pc16550::show_readable(bool val)
{
  u8_t r= regs[rlsr]->get();
  if (val)
    r|= 1;
  else
    r&= ~1;
  regs[rlsr]->set(r);
}

void
cl_pc16550::show_tx_complete(bool val)
{
  u8_t r= regs[rlsr]->get();
  if (val)
    r|= 0x60;
  else
    r&= ~0x60;
  regs[rlsr]->set(r);
}

void
cl_pc16550::show_idle(bool val)
{
}

void
cl_pc16550::set_rdr(t_mem val)
{
  regs[rdr]->set(val);
}

void
cl_pc16550::print_info(class cl_console_base *con)
{
  /*
  u8_t ru8= regs[rstat]->get();
  u8_t tu8= regs[tstat]->get();
  */
  con->dd_printf("%s[%d] at %s[0x%06x] %s\n", id_string, id,
		 as?(as->get_name()):"none",
		 base, on?"ON ":"OFF");
  con->dd_printf("Input: ");
  class cl_f *fin= io->get_fin(), *fout= io->get_fout();
  if (fin)
    con->dd_printf("%30s/%3d ", fin->get_file_name(), fin->file_id);
  con->dd_printf("Output: ");
  if (fout)
    con->dd_printf("%30s/%3d", fout->get_file_name(), fout->file_id);
  con->dd_printf("\n");
  con->dd_printf("mcnt=%d\n", mcnt);
  con->dd_printf("Sending: %s, en, %2d/8 bits\n",
		 s_sending?"yes":"no ",
		 s_tr_bit);
  con->dd_printf("Receiving: %s, en, %2d/8 bits\n",
		 s_receiving?"yes":"no ",
		 s_rec_bit);
  con->dd_printf("Cpb=%8d bits=8\n", cpb);
  /*
  con->dd_printf("RXSR: ");
  con->print_bin(ru8, 8);
  con->dd_printf(" 0x%02x", ru8);
  con->dd_printf(" TXSR: ");
  con->print_bin(tu8, 8);
  con->dd_printf(" 0x%02x", tu8);
  con->dd_printf(" RXNE=%d TC=%d",
		 (ru8&1)?1:0,
		 (tu8&1)?1:0);
  */
  con->dd_printf("\n");
}

/* End of sim.src/pc16550.cc */
