/*
 * Simulator of microcontrollers (cia.cc)
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

#include "ddconfig.h"

// sim
#include "argcl.h"
#include "itsrccl.h"

#include "ciacl.h"

enum reg_idx
  {
   sr = 0, // RO
   cr = 0, // WO
   rd = 1, // RO
   td = 1, // WO
   dr = 1, 
  };

cl_cia::cl_cia(class cl_uc *auc, int aid, t_addr abase):
  cl_serial_hw(auc, aid, "uart")
{
  base= abase;
}

cl_cia::~cl_cia(void)
{
}

int
cl_cia::init(void)
{
  int i;
  
  set_name("6850");
  cl_serial_hw::init();

  r_cr= cfg_cell(acia_cfg_cr);
  r_sr= cfg_cell(acia_cfg_sr);

  /*for (i= 0; i < dev_size(); i++)
    regs[i]= register_cell(uc->rom, base+i);*/
  map(uc->rom, base);
  regs[cr]->write(0x15);
  //pick_div();
  //pick_ctrl();

  ten= true;
  ren= true;
  s_sending= false;
  s_receiving= false;
  
  r_sr->set(0);//regs[sr]->set(0);
  show_readable(false);
  show_writable(true);
  //cfg_set(acia_cfg_req, 'i');

  cl_var *v;
  chars pn= chars("", "uart%d_", id);
  uc->vars->add(v= new cl_var(pn+"base", cfg, acia_cfg_base, cfg_help(acia_cfg_base)));
  v->init();
  uc->vars->add(v= new cl_var(pn+"cr", cfg, acia_cfg_cr,
			      cfg_help(acia_cfg_cr)));
  v->init();
  uc->vars->add(v= new cl_var(pn+"sr", cfg, acia_cfg_sr,
			      cfg_help(acia_cfg_sr)));
  v->init();
  uc->vars->add(v= new cl_var(pn+"req", cfg, acia_cfg_req,
			      cfg_help(acia_cfg_req)));
  v->init();
  
  is_t= new cl_it_src(uc, 1,
		      r_cr, 0x60, //0x20,
		      r_sr, 2,
		      0, false, false,
		      pn+"tx",
		      0);
  is_t->set_ie_value(0x20);
  is_t->init();
  is_t->set_parent(uc->search_it_src('i'));
  uc->it_sources->add(is_t);
  is_r= new cl_it_src(uc, 2,
		      r_cr, 0x80, //0x80,
		      r_sr, 1,
		      0, false, false,
		      pn+"rx",
		      0);
  is_r->init();
  is_r->set_ie_value(0x80);
  is_r->set_parent(uc->search_it_src('i'));
  uc->it_sources->add(is_r);
  
  return(0);
}

void
cl_cia::prepare_rebase(t_addr new_base)
{
  if (new_base != base)
    {
      int i;
      for (i= 0; i < 2; i++)
	{
	  t_mem v= uc->rom->get(base+i);
	  uc->rom->set(new_base+i, v);
	}
    }
}

const char *
cl_cia::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case acia_cfg_base:
      return "Base address of ACIA registers (int, RW)";
    case acia_cfg_cr:
      return "Copy of written CR value";
    case acia_cfg_sr:
      return "Simulated SR value";
    case acia_cfg_req:
      return "Req mode of ACIA 'i'=IRQ 'f'=FIRQ 'n'=NMI";
    }
  return cl_serial_hw::cfg_help(addr);
}

t_mem
cl_cia::read(class cl_memory_cell *cell)
{
  if (cell == regs[dr])
    {
      cfg_set(serconf_able_receive, 1);
      show_readable(false);
      return s_in;
    }
  if (cell == regs[sr])
    return r_sr->read();
  conf(cell, NULL);
  return cell->get();
}

void
cl_cia::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  if (cell == regs[cr])
    {
      r_cr->set(*val);
      pick_div();
      pick_ctrl();
      //*val= cell->get();
      set_sr_irq();
    }
  else
    {
      cell->set(*val);
      if (cell == regs[dr])
	{
	  s_txd= *val;
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
cl_cia::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  if (addr < serconf_nr)
    return cl_serial_hw::conf_op(cell, addr, val);
  switch ((enum acia_cfg)addr)
    {
    case acia_cfg_base:
      if (val)
	{
	  int i;
	  if (uc->rom->valid_address(*val))
	    {
	      /*
	      for (i= 0; i < dev_size(); i++)
		unregister_cell(regs[i]);
	      prepare_rebase(*val);
	      base= *val;
	      init();
	      */
	      map(uc->rom, (*val)&0xffff);
	    }
	}
      else
	{
	  cell->set(base);
	}
      break;
    case acia_cfg_req:
      if (val)
	{
	  //is_r->set_pass_to(*val);
	  //is_t->set_pass_to(*val);
	  is_r->set_parent(uc->search_it_src(*val));
	  is_t->set_parent(uc->search_it_src(*val));
	}
      break;
      
    case acia_cfg_cr: break;
    case acia_cfg_sr:
      if (val)
	{
	  cell->set(*val&= 0x7f);
	  set_sr_irq();
	}
      break;
      
    default:
      break;
    }
  return cell->get();
}

bool
cl_cia::set_cmd(class cl_cmdline *cmdline,
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
      prepare_rebase(a);
      base= a;
      init();
      */
      map(uc->rom, a);
      return true; // handled
    }
  return cl_serial_hw::set_cmd(cmdline, con);
}

void
cl_cia::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware uart[%d] address\n", id);
  cl_serial_hw::set_help(con);
}

int
cl_cia::tick(int cycles)
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
cl_cia::start_send()
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
cl_cia::restart_send()
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
cl_cia::finish_send()
{
  show_writable(true);
  show_tx_complete(true);
}

void
cl_cia::received()
{
  set_dr(s_in);
  cfg_write(serconf_received, s_in);
  show_readable(true);
}

void
cl_cia::reset(void)
{
  show_writable(true);
  show_readable(false);
}

void
cl_cia::happen(class cl_hw *where, enum hw_event he,
		  void *params)
{
}


void
cl_cia::pick_div()
{
  switch (r_cr->get() & 0x03)
    {
    case 0x00: cpb= 1; break;
    case 0x01: cpb= 16; break;
    case 0x02: cpb= 64; break;
    case 0x03: /*Master reset*/ cpb= 1; break;
    }
  mcnt= 0;
}

void
cl_cia::pick_ctrl()
{
  switch ((r_cr->get() >> 2) & 7)
    {
    case 0: bits= 10; break;
    case 1: bits= 10; break;
    case 2: bits=  9; break;
    case 3: bits=  9; break;
    case 4: bits= 10; break;
    case 5: bits=  9; break;
    case 6: bits= 10; break;
    case 7: bits= 10; break;
    }
  s_rec_bit= s_tr_bit= 0;
  s_receiving= false;
  s_tx_written= false;
}


void
cl_cia::show_writable(bool val)
{
  // TDRE: Transmit Data Register Empty: sr.1
  if (val)
    r_sr->set(r_sr->get() | 2);
  else
    r_sr->set(r_sr->get() & ~2);
  set_sr_irq();
}

void
cl_cia::show_readable(bool val)
{
  // RDRF: Receive Data Register Full (sr.0)
  if (val)
    r_sr->set(r_sr->get() | 1);
  else
    r_sr->set(r_sr->get() & ~1);
  set_sr_irq();
}

void
cl_cia::show_tx_complete(bool val)
{
  show_writable(val);
}

void
cl_cia::show_idle(bool val)
{
}

void
cl_cia::set_dr(t_mem val)
{
  regs[dr]->set(val);
}

void
cl_cia::set_sr_irq(void)
{
  bool t= false, r= false;
  u8_t c= r_cr->get(), s= r_sr->get();
  t= ((c & 0x60) == 0x20) && (s & 2);
  r= (c & 0x80) && (s & 1);
  s&= 0x7f;
  if (r || t)
    s|= 0x80;
  r_sr->set(s);
}

void
cl_cia::print_info(class cl_console_base *con)
{
  u8_t u8= r_sr->get();//regs[sr]->get();
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
  con->print_bin(r_cr->get(), 8);
  con->dd_printf(" 0x%02x", r_cr->get());
  con->dd_printf(" cpb=%8d bits=%2d\n", cpb, bits);
  con->dd_printf("SR: ");
  con->print_bin(u8, 8);
  con->dd_printf(" 0x%02x", u8);
  con->dd_printf(" RDRF=%d TDRE=%d\n",
		 (u8&1)?1:0,
		 (u8&2)?1:0);
  //print_cfg_info(con);
}

/* End of motorola.src/cia.cc */
