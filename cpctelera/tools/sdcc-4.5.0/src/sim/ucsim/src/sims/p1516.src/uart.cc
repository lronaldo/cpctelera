/*
 * Simulator of microcontrollers (p1516.src/uart.cc)
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

#include <ctype.h>

#include "ddconfig.h"

// sim
#include "argcl.h"
#include "itsrccl.h"

#include "uartcl.h"

cl_uart::cl_uart(class cl_uc *auc, int aid, t_addr abase):
  cl_serial_hw(auc, aid, "uart")
{
  base= abase;
}

cl_uart::~cl_uart(void)
{
}

int
cl_uart::init(void)
{
  int i;
  
  set_name("uart");
  cl_serial_hw::init();
  /*  
  for (i= 0; i < dev_size(); i++)
    regs[i]= register_cell(uc->rom, base+i);
  */
  map(uc->rom, base);
  
  ten= false;
  ren= false;
  s_sending= false;
  s_receiving= false;

  show_readable(false);
  show_writable(true);

  cl_var *v;
  chars pn= chars("", "uart%d_", id);
  uc->vars->add(v= new cl_var(pn+"base", cfg, uart_cfg_base, cfg_help(uart_cfg_base)));
  v->init();

  return(0);
}

const char *
cl_uart::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case uart_cfg_base:
      return "Base address of UART registers (int, RW)";
      /*    case acia_cfg_cr:
      return "Copy of written CR value";
    case acia_cfg_sr:
      return "Simulated SR value";
    case acia_cfg_req:
    return "Req mode of ACIA 'i'=IRQ 'f'=FIRQ 'n'=NMI";*/
      }
  return cl_serial_hw::cfg_help(addr);
}

t_mem
cl_uart::read(class cl_memory_cell *cell)
{
  if (cell == regs[dr])
    {
      cfg_set(serconf_able_receive, 1);
      show_readable(false);
      return s_in;
    }
  /*if (cell == regs[stat])
    return r_sr->read();*/
  conf(cell, NULL);
  return cell->get();
}

void
cl_uart::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  if (cell == regs[ctrl])
    {
      regs[ctrl]->set(*val);
      pick_div();
      pick_ctrl();
      *val= cell->get();
    }
  if (cell == regs[rstat])
    {
      *val= regs[rstat]->get();
    }
  if (cell == regs[tstat])
    {
      *val= regs[tstat]->get();
    }
  else
    {
      cell->set(*val);
      if (cell == regs[dr])
	{
	  s_txd= *val & 0xff;
	  s_tx_written= true;
	  show_writable(false);
	  if (!s_sending)
	    {
	      start_send();
	    }      
	}
    }
}


t_mem
cl_uart::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  if (addr < serconf_nr)
    return cl_serial_hw::conf_op(cell, addr, val);
  switch ((enum uart_cfg)addr)
    {
    case uart_cfg_base:
      if (val)
	{
	  int i;
	  if (uc->rom->valid_address(*val))
	    {
	      /*
	      for (i= 0; i < dev_size(); i++)
		unregister_cell(regs[i]);
	      base= *val;
	      init();
	      */
	      map(uc->rom, *val);
	    }
	  else
	    *val= base;
	}
      else
	{
	  cell->set(base);
	}
      break;
      /*
    case acia_cfg_req:
      if (val)
	{
	  //is_r->set_pass_to(*val);
	  //is_t->set_pass_to(*val);
	  is_r->set_parent(uc->search_it_src(*val));
	  is_t->set_parent(uc->search_it_src(*val));
	}
      break;
      */
      /*
    case acia_cfg_cr: break;
      */
      /*
    case acia_cfg_sr:
      if (val)
	{
	  cell->set(*val&= 0x7f);
	}
      break;
      */
    default:
      break;
    }
  return cell->get();
}

bool
cl_uart::set_cmd(class cl_cmdline *cmdline,
				class cl_console_base *con)
{
  class cl_cmd_arg *params[2]= {
    cmdline->param(0),
    cmdline->param(1)
  };

  if (cmdline->syntax_match(uc, NUMBER))
    {
      int i;
      t_addr a= params[0]->value.number;
      if (!uc->rom->valid_address(a))
	{
	  con->dd_printf("Address must be between 0x%x and 0x%x\n",
			 AU(uc->rom->lowest_valid_address()),
			 AU(uc->rom->highest_valid_address()));
	  return true;
	}
      /*
      for (i= 0; i < dev_size(); i++)
	unregister_cell(regs[i]);
      base= a;
      init();
      */
      map(uc->rom, a);
      return true; // handled
    }
  return cl_serial_hw::set_cmd(cmdline, con);
}

void
cl_uart::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware uart[%d] address\n", id);
  cl_serial_hw::set_help(con);
}

int
cl_uart::tick(int cycles)
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
	  //io->dd_printf("%c", s_out);
	  io->write((char*)&s_out, 1);
	  s_tr_bit-= bits;
	  if (s_tx_written)
	    restart_send();
	  else
	    finish_send();
	}
    }
  if ((ren) &&
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
	  c= input;
	  input_avail= false;
	  s_in= c;
	  received();
	}
      s_receiving= false;
      s_rec_bit-= bits;
    }
  
  return(0);
}

void
cl_uart::start_send()
{
  if (ten)
    {
      s_out= s_txd;
      s_tx_written= false;
      s_sending= true;
      s_tr_bit= 0;
      show_writable(true);
    }
}

void
cl_uart::restart_send()
{
  if (ten)
    {
      s_out= s_txd;
      s_tx_written= false;
      s_sending= true;
      s_tr_bit= 0;
      show_writable(true);
    }
}

void
cl_uart::finish_send()
{
  show_writable(true);
  show_tx_complete(true);
}

void
cl_uart::received()
{
  set_dr(s_in);
  cfg_write(serconf_received, s_in);
  show_readable(true);
}

void
cl_uart::reset(void)
{
  show_writable(true);
  show_readable(false);
  regs[ctrl]->set(0);
  regs[rcpb]->set(cpb= 217);
  pick_div();
  pick_ctrl();
}

void
cl_uart::happen(class cl_hw *where, enum hw_event he,
		  void *params)
{
}


void
cl_uart::pick_div()
{
  cpb= regs[rcpb]->get();
  mcnt= 0;
}

void
cl_uart::pick_ctrl()
{
  u32_t r= regs[ctrl]->get();
  ren= r & 1;
  ten= r & 2;
  s_rec_bit= s_tr_bit= 0;
  s_receiving= false;
  s_tx_written= false;
  bits= 8;
}


void
cl_uart::show_writable(bool val)
{
  u32_t r= regs[tstat]->get();
  if (val)
    r|= 1;
  else
    r&= ~1;
  regs[tstat]->set(r);
}

void
cl_uart::show_readable(bool val)
{
  u32_t r= regs[rstat]->get();
  if (val)
    r|= 1;
  else
    r&= ~1;
  regs[rstat]->set(r);
}

void
cl_uart::show_tx_complete(bool val)
{
  show_writable(val);
}

void
cl_uart::show_idle(bool val)
{
}

void
cl_uart::set_dr(t_mem val)
{
  regs[dr]->set(val);
}

void
cl_uart::print_info(class cl_console_base *con)
{
  u8_t ru8= regs[rstat]->get();
  u8_t tu8= regs[tstat]->get();
  con->dd_printf("%s[%d] at 0x%06x %s\n", id_string, id, base, on?"on ":"off");
  con->dd_printf("Input: ");
  class cl_f *fin= io->get_fin(), *fout= io->get_fout();
  if (fin)
    con->dd_printf("%30s/%3d ", fin->get_file_name(), fin->file_id);
  con->dd_printf("Output: ");
  if (fout)
    con->dd_printf("%30s/%3d", fout->get_file_name(), fout->file_id);
  con->dd_printf("\n");
  con->dd_printf("mcnt=%d\n", mcnt);
  con->dd_printf("Sending: %s, %s, %2d/%2d bits\n",
		 s_sending?"yes":"no ",
		 ten?"en ":"dis",
		 s_tr_bit, bits);
  con->dd_printf("Receiving: %s, %s, %2d/%2d bits\n",
		 s_receiving?"yes":"no ",
		 ren?"en ":"dis",
		 s_rec_bit, bits);
  con->dd_printf("CR: ");
  con->print_bin(regs[ctrl]->get(), 8);
  con->dd_printf(" 0x%02x", regs[ctrl]->get());
  con->dd_printf(" cpb=%8d bits=%2d\n", cpb, bits);
  con->dd_printf("RXSR: ");
  con->print_bin(ru8, 8);
  con->dd_printf(" 0x%02x", ru8);
  con->dd_printf(" TXSR: ");
  con->print_bin(tu8, 8);
  con->dd_printf(" 0x%02x", tu8);
  con->dd_printf(" RXNE=%d TC=%d\n",
		 (ru8&1)?1:0,
		 (tu8&1)?1:0);
  //print_cfg_info(con);
}

/* End of p1516.src/uart.cc */
