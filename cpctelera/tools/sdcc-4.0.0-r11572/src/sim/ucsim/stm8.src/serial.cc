/*
 * Simulator of microcontrollers (serial.cc)
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

#include "ddconfig.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <ctype.h>
//#include <errno.h>
//#include <fcntl.h>
//#include <sys/time.h>
//#include <strings.h>

// prj
//#include "globals.h"
//#include "utils.h"

// cmd
//#include "cmdutil.h"

// sim
#include "itsrccl.h"

// local
#include "clkcl.h"
#include "serialcl.h"


enum reg_idx {
  sr	= 0,
  dr	= 1,
  brr1	= 2,
  brr2	= 3,
  cr1	= 4,
  cr2	= 5,
  cr3	= 6,
  cr4	= 7,
  cr5	= 8,
  cr6	= 9,
  gtr	= 10,
  pscr	= 11
};


cl_serial::cl_serial(class cl_uc *auc,
		     t_addr abase,
		     int ttype, int atxit, int arxit):
  cl_serial_hw(auc, ttype, "uart")
{
  type= ttype;
  base= abase;
  txit= atxit;
  rxit= arxit;
}


cl_serial::~cl_serial(void)
{
}

int
cl_serial::init(void)
{
  int i;
  class cl_it_src *is;
  
  set_name("stm8_uart");
  cl_serial_hw::init();
  clk_enabled= false;
  for (i= 0; i < 12; i++)
    {
      regs[i]= register_cell(uc->rom, base+i);
    }
  pick_div();
  pick_ctrl();

  uc->it_sources->add(is= new cl_it_src(uc, txit,
					regs[cr2], 0x80,
					regs[sr], 0x80,
					0x8008+txit*4, false, false,
					chars("", "usart%d transmit register empty", id), 20*10+1));
  is->init();
  uc->it_sources->add(is= new cl_it_src(uc, txit,
					regs[cr2], 0x40,
					regs[sr], 0x40,
					0x8008+txit*4, false, false,
					chars("", "usart%d transmit complete", id), 20*10+2));
  is->init();
  uc->it_sources->add(is= new cl_it_src(uc, rxit,
					regs[cr2], 0x20,
					regs[sr], 0x20,
					0x8008+rxit*4, false, false,
					chars("", "usart%d receive", id), 20*10+3));
  is->init();

  sr_read= false;

  return(0);
}


void
cl_serial::new_hw_added(class cl_hw *new_hw)
{
}

void
cl_serial::added_to_uc(void)
{
}

t_mem
cl_serial::read(class cl_memory_cell *cell)
{
  if (cell == regs[dr])
    {
      if (sr_read)
	regs[sr]->set_bit0(0x1f);
      regs[sr]->set_bit0(0x20);
      cfg_set(serconf_able_receive, 1);
      return s_in;
    }
  sr_read= (cell == regs[sr]);
  conf(cell, NULL);
  return cell->get();
}

void
cl_serial::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  if (cell == regs[sr])
    {
      u8_t v= cell->get();
      if ((*val & 0x40) == 0)
	{
	  v&= ~0x40;
	  *val= v;
	}
    }
  else
    {
      cell->set(*val);
      if ((cell == regs[brr1]) ||
	  (cell == regs[brr2]))
	{
	  pick_div();
	}
      else if ((cell == regs[cr1]) ||
	       (cell == regs[cr2]))
	{
	  pick_ctrl();
	}
  
      else if (cell == regs[dr])
	{
	  s_txd= *val;
	  s_tx_written= true;
	  show_writable(false);
	  if (sr_read)
	    show_tx_complete(false);
	  if (!s_sending)
	    {
	      start_send();
	    }      
	}
    }

  sr_read= false;
}

t_mem
cl_serial::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  if (addr < serconf_common)
    return cl_serial_hw::conf_op(cell, addr, val);
  switch ((enum serial_cfg)addr)
    {
      /*
    case serial_:
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
      */
    default:
      break;
    }
  return cell->get();
}

int
cl_serial::tick(int cycles)
{
  char c;

  if (!en ||
      !clk_enabled)
    return 0;
  
  if ((mcnt+= cycles) >= div)
    {
      mcnt-= div;
      if (ten)
	s_tr_bit++;
      if (ren)
	s_rec_bit++;
    }
  else
    return 0;
  
  if (s_sending &&
      (s_tr_bit >= bits))
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
  if ((ren) &&
      io->get_fin() &&
      !s_receiving)
    {
      if (cfg_get(serconf_check_often))
	{
	  if (io->input_avail())
	    io->proc_input(0);
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
cl_serial::start_send()
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
cl_serial::restart_send()
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
cl_serial::finish_send()
{
  show_writable(true);
  show_tx_complete(true);
}

void
cl_serial::received()
{
  set_dr(s_in);
  cfg_write(serconf_received, s_in);
  if (regs[sr]->get() & 0x20)
    regs[sr]->set_bit1(0x08); // overrun
  show_readable(true);
}

void
cl_serial::reset(void)
{
  int i;
  regs[sr]->set(0xc0);
  for (i= 2; i < 12; i++)
    regs[i]->set(0);
}

void
cl_serial::happen(class cl_hw *where, enum hw_event he,
		  void *params)
{
  if ((he == EV_CLK_ON) ||
      (he == EV_CLK_OFF))
    {
      cl_clk_event *e= (cl_clk_event *)params;
      if ((e->cath == HW_UART) &&
	  (e->id == id))
	clk_enabled= he == EV_CLK_ON;
    }
}

void
cl_serial::pick_div()
{
  u8_t b1= regs[brr1]->get();
  u8_t b2= regs[brr2]->get();
  div= ((((b2&0xf0)<<4) + b1)<<4) + (b2&0xf);
  mcnt= 0;
}

void
cl_serial::pick_ctrl()
{
  u8_t c1= regs[cr1]->get();
  u8_t c2= regs[cr2]->get();
  en= !(c1 & 0x20);
  ten= c2 & 0x08;
  ren= c2 & 0x04;
  bits= 10;
  s_rec_bit= s_tr_bit= 0;
  s_receiving= false;
  s_tx_written= false;
}

void
cl_serial::show_writable(bool val)
{
  if (val)
    // TXE=1
    regs[sr]->write_bit1(0x80);
  else
    // TXE=0
    regs[sr]->write_bit0(0x80);
}

void
cl_serial::show_readable(bool val)
{
  if (val)
    regs[sr]->write_bit1(0x20);
  else
    regs[sr]->write_bit0(0x20);
}

void
cl_serial::show_tx_complete(bool val)
{
  if (val)
    regs[sr]->write_bit1(0x40);
  else
    regs[sr]->write_bit0(0x40);
}

void
cl_serial::show_idle(bool val)
{
  if (val)
    regs[sr]->write_bit1(0x10);
  else
    regs[sr]->write_bit0(0x10);
}

void
cl_serial::set_dr(t_mem val)
{
  regs[dr]->set(val);
}

void
cl_serial::print_info(class cl_console_base *con)
{
  con->dd_printf("%s[%d] at 0x%06x %s\n", id_string, id, base, on?"on":"off");
  con->dd_printf("clk %s\n", clk_enabled?"enabled":"disabled");
  con->dd_printf("Input: ");
  class cl_f *fin= io->get_fin(), *fout= io->get_fout();
  if (fin)
    con->dd_printf("%s/%d ", fin->get_file_name(), fin->file_id);
  con->dd_printf("Output: ");
  if (fout)
    con->dd_printf("%s/%d", fout->get_file_name(), fout->file_id);
  con->dd_printf("\n");
  print_cfg_info(con);
}


/* End of stm8.src/serial.cc */
