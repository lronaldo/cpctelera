/*
 * Simulator of microcontrollers (serial_hw.cc)
 *
 * Copyright (C) 2016 Drotos Daniel
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

#include "serial_hwcl.h"


bool
cl_serial_io::input_avail(void)
{
  return cl_hw_io::input_avail();
}


cl_serial_hw::cl_serial_hw(class cl_uc *auc, int aid, chars aid_string):
  cl_hw(auc, HW_UART, aid, (const char *)aid_string)
{
  listener_io= listener_i= listener_o= NULL;
  as= NULL;
  base= 0;
}

cl_serial_hw::~cl_serial_hw(void)
{
  delete serial_in_file_option;
  delete serial_out_file_option;
  delete serial_ifirst_option;
  delete io;
}

int
cl_serial_hw::init(void)
{
  chars cs;
  int i;
  is_raw= false;
  
  cl_hw::init();
  regs= (cl_memory_cell**)malloc(dev_size() * sizeof(class cl_memory_cell *));
  for (i=0; i<dev_size(); i++)
    regs[i]= NULL;
  make_io();
  input_avail= false;
  sending_nl= false;
  skip_nl= 0;
  cfg_set(serconf_nl, 10);
  nl_send_idx= 0;

  s_sending= false;
  s_receiving= false;
  s_tx_written= false;
  s_rec_bit= 0;
  s_tr_bit= 0;
  bits= 10;

  mcnt= 0;
  cpb= 1;
  
  cs.format("serial%d_in_file", id);
  serial_in_file_option= new cl_optref(this, cs.c_str());
  cs.format("serial%d_out_file", id);
  serial_out_file_option= new cl_optref(this, cs.c_str());
  cs.format("serial%d_ifirst", id);
  serial_ifirst_option= new cl_optref(this, cs.c_str());
  cs.format("serial%d_raw", id);
  serial_raw_option= new cl_optref(this, cs.c_str());
  serial_raw_option->get_value(&is_raw);
  
  cs.format("serial%d_port", id);
  serial_port_option= new cl_optref(this, cs.c_str());

  long port= -1;
  if (serial_port_option->get_value(&port))
    {
      if (port < 0) {}
      if (port > 0)
	{
	  listener_io= new cl_serial_listener(port, application, this, sl_io);
	  listener_io->init();
	  class cl_commander_base *c= application->get_commander();
	  c->add_console(listener_io);
	}
    }
  
  cs.format("serial%d_iport", id);
  serial_iport_option= new cl_optref(this, cs.c_str());
  port= -1;
  if (serial_iport_option->get_value(&port))
    {
      if (port < 0) {}
      if (port > 0)
	{
	  listener_i= new cl_serial_listener(port, application, this, sl_i);
	  listener_i->init();
	  class cl_commander_base *c= application->get_commander();
	  c->add_console(listener_i);
	}
    }

  cs.format("serial%d_oport", id);
  serial_oport_option= new cl_optref(this, cs.c_str());
  port= -1;
  if (serial_oport_option->get_value(&port))
    {
      if (port < 0) {}
      if (port > 0)
	{
	  listener_o= new cl_serial_listener(port, application, this, sl_o);
	  listener_o->init();
	  class cl_commander_base *c= application->get_commander();
	  c->add_console(listener_o);
	}
    }
  
  const char *f_serial_in = serial_in_file_option->get_value("");
  const char *f_serial_out= serial_out_file_option->get_value("");
  bool ifirst= false;
  ifirst= serial_ifirst_option->get_value(ifirst);
  class cl_f *fi, *fo;

  if (ifirst)
    {
      if (f_serial_in && *f_serial_in)
	{
	  if (f_serial_in[0] == '\001')
	    {
	      chars c= &(f_serial_in[1]);
	      fi= (class cl_f *)(c.htoll());
	    }
	  else
	    fi= mk_io(f_serial_in, "r");
	  if (!fi->tty && !is_raw)
	    fprintf(stderr, "Warning: serial input interface connected to a "
		    "non-terminal file.\n");
	}
      else
	fi= 0;
      if (f_serial_out && *f_serial_out)
	{
	  if (f_serial_out[0] == '\001')
	    {
	      chars c= &(f_serial_out[1]);
	      fo= (class cl_f *)(c.htoll());
	    }
	  else
	    fo= mk_io(chars(f_serial_out), "w");
	  if (!fo->tty && !is_raw)
	    fprintf(stderr, "Warning: serial output interface connected to a "
		    "non-terminal file.\n");
	}
      else
	fo= 0;
    }
  else
    {
      if (f_serial_out && *f_serial_out)
	{
	  if (f_serial_out[0] == '\001')
	    {
	      chars c= &(f_serial_out[1]);
	      fo= (class cl_f *)(c.htoll());
	    }
	  else
	    fo= mk_io(chars(f_serial_out), "w");
	  if (!fo->tty && !is_raw)
	    fprintf(stderr, "Warning: serial output interface connected to a "
		    "non-terminal file.\n");
	}
      else
	fo= 0;
      if (f_serial_in && *f_serial_in)
	{
	  if (f_serial_in[0] == '\001')
	    {
	      chars c= &(f_serial_in[1]);
	      fi= (class cl_f *)(c.htoll());
	    }
	  else
	    fi= mk_io(f_serial_in, "r");
	  if (!fi->tty && !is_raw)
	    fprintf(stderr, "Warning: serial input interface connected to a "
		    "non-terminal file.\n");
	}
      else
	fi= 0;
    }

  
  io->replace_files(true, fi, fo);
  set_raw();
  
  menu= 0;
  
  cfg_set(serconf_on, true);
  cfg_set(serconf_check_often, false);
  cfg_set(serconf_escape, 'x'-'a'+1);

  chars pn("", "%s%d_", id_string, id);
  uc->vars->add(pn+"on", cfg, serconf_on, cfg_help(serconf_on));
  uc->vars->add(pn+"check_often", cfg, serconf_check_often, cfg_help(serconf_check_often));
  uc->vars->add(pn+"esc_char", cfg, serconf_escape, cfg_help(serconf_escape));
  uc->vars->add(pn+"received_char", cfg, serconf_received, cfg_help(serconf_received));
  uc->vars->add(pn+"flowctrl", cfg, serconf_flowctrl, cfg_help(serconf_flowctrl));
  uc->vars->add(pn+"able_receive", cfg, serconf_able_receive, cfg_help(serconf_able_receive));
  uc->vars->add(pn+"nl", cfg, serconf_nl, cfg_help(serconf_nl));
  cfg_set(serconf_able_receive, 1);

  return 0;
}

void
cl_serial_hw::map(class cl_address_space *new_as, t_addr new_base)
{
  int i;
  if ((as && new_as) &&
      ((as != new_as) || (base != new_base)))
    {
      for (i=0; i<dev_size(); i++)
	{
	  t_mem v= as->get(base+i);
	  new_as->set(new_base+i, v);
	}
    }
  if (as)
    {
      for (i=0; i<dev_size(); i++)
	{
	  unregister_cell(as->get_cell(base+i));
	  regs[i]= NULL;
	}
    }
  if (new_as)
    {
      for (i=0; i<dev_size(); i++)
	regs[i]= register_cell(new_as, new_base+i);
    }
  i= as==NULL;
  as= new_as;
  base= new_base;
  if (i)
    reset();
  if (var_names.nempty())
    {
      chars n;
      chars pn= chars("", "uart%d_", id);
      cl_cvar *v;
      var_names.start_parse();
      n= var_names.token(";");
      i= 0;
      while (!n.is_null())
	{
	  v= uc->get_var(pn+n);
	  if (v)
	    v->set_cell(as->get_cell(base+i));
	  n= var_names.token(";");
	  i++;
	}
    }
}

void
cl_serial_hw::set_raw(void)
{
  class cl_f *fi= io->get_fin();
  if (fi)
    {
      if (is_raw)
	fi->set_telnet(false);
      else
	{
	  fi->interactive(NULL);
	  fi->set_telnet(true);
	}
      fi->raw();
      fi->echo(NULL);
    }
}

const char *
cl_serial_hw::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case serconf_on:
      return "Turn ticking of UART on/off (bool, RW)";
    case serconf_check_often:
      return "Check input file at every cycle (bool, RW)";
    case serconf_escape:
      return "Escape char on display (int, RW)";
    case serconf_common:
      return "Not used";
    case serconf_received:
      return "Received char written by simulator (int, R)";
    case serconf_flowctrl:
      return "Flow-control simulation on/off (bool, RW)";
    case serconf_able_receive:
      return "UART enabled to receive by flow-control (bool, RW)";
    case serconf_nl:
      return "Characters to send as new line (int, RW)";
    }
  return "Not used";
}

bool
cl_serial_hw::set_cmd(class cl_cmdline *cmdline,
		      class cl_console_base *con)
{
  class cl_cmd_arg *params[2]= {
    cmdline->param(0),
    cmdline->param(1)
  };
  class cl_commander_base *c= application->get_commander();
  if (cmdline->syntax_match(uc, STRING NUMBER))
    {
      char *p1= params[0]->value.string.string;
      int port= params[1]->value.number;
      if (strcmp(p1, "port")==0)
	{
	  del_listener_i();
	  del_listener_o();
	  if ((port > 0) && c)
	    {
	      listener_io= new cl_serial_listener(port, application, this, sl_io);
	      listener_io->init();
	      c->add_console(listener_io);
	    }
	  return true;
	}
      if (strcmp(p1, "iport")==0)
	{
	  del_listener_i();
	  if ((port > 0) && c)
	    {
	      listener_i= new cl_serial_listener(port, application, this, sl_i);
	      listener_i->init();
	      c->add_console(listener_i);
	    }
	  return true;
	}
      if (strcmp(p1, "oport")==0)
	{
	  del_listener_o();
	  if ((port > 0) && c)
	    {
	      listener_o= new cl_serial_listener(port, application, this, sl_o);
	      listener_o->init();
	      c->add_console(listener_o);
	    }
	  return true;
	}
      if (strcmp(p1, "raw")==0)
	{
	  bool v= port;
	  is_raw= v;
	  set_raw();
	  return true;
	}
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING STRING))
    {
      class cl_f *fi, *fo;
      char *p1= params[0]->value.string.string;
      const char *p2= params[1]->value.string.string;
      if (strcmp(p1, "file")==0)
	{
	  if (p2 && *p2)
	    {
	      fi= mk_io(p2, "r");
	      fo= mk_io(p2, "w");
	      new_io(fi, fo);
	    }
	  return true;
	}
      if (strcmp(p1, "in")==0)
	{
	  if (p2 && *p2)
	    {
	      fi= mk_io(p2, "r");
	      if (fi->opened())
		new_i(fi);
	      else
		con->dd_cprintf("error", "Error opening file \"%s\"\n",p2);
	    }
	  return true;
	}
      if (strcmp(p1, "out")==0)
	{
	  if (p2 && *p2)
	    {
	      fo= mk_io(p2, "w");
	      new_o(fo);
	    }
	  return true;
	}
      return false;
    }
  return false;
}

void
cl_serial_hw::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware %s[%d] raw   0|1\n", get_name(), id);
  con->dd_printf("set hardware %s[%d] file  \"file\"\n", get_name(), id);
  con->dd_printf("set hardware %s[%d] in    \"file\"\n", get_name(), id);
  con->dd_printf("set hardware %s[%d] out   \"file\"\n", get_name(), id);
  con->dd_printf("set hardware %s[%d] port  nr\n", get_name(), id);
  con->dd_printf("set hardware %s[%d] iport nr\n", get_name(), id);
  con->dd_printf("set hardware %s[%d] oport nr\n", get_name(), id);
}
 

t_mem
cl_serial_hw::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum serial_cfg)addr)
    {
    case serconf_on: // turn this HW on/off
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
    case serconf_check_often:
      if (val)
	{
	  cell->set(*val?1:0);
	}
      break;
    case serconf_escape:
      if (val)
	{
	  char c= tolower(*val);
	  if ((c >= 'a') &&
	      (c <= 'z'))
	    cell->set(c - 'a'+1);
	}
    case serconf_nl:
      if (val)
	cell->set((*val)&0x00ffffff);
    default:
      break;
    }
  return cell->get();
}

u8_t
cl_serial_hw::get_input(void)
{
  if (!input_avail)
    return 0;
  if (!sending_nl)
    {
      if (!is_nl(input))
	{
	  input_avail= false;
	  return input;
	}
      skip_nl= opposite_nl(input);
      sending_nl= true;
      nl_send_idx= 0;
    }
  if (sending_nl)
    {
      u32_t nl_value= cfg_get(serconf_nl);
      u8_t v= (nl_value >> (nl_send_idx*8)) & 0xff;
      u8_t vn= (nl_value >> ((nl_send_idx+1)*8)) & 0xff;
      if (vn == 0)
	{
	  sending_nl= false;
	  input_avail= false;
	  nl_send_idx= 0;
	}
      else
	nl_send_idx++;
      return v;
    }
  input_avail= false;
  return input;
}

void
cl_serial_hw::make_io()
{
  if (!io)
    {
      io= new cl_serial_io(this);
      application->get_commander()->add_console(io);
      set_raw();
    }
}

void
cl_serial_hw::new_io(class cl_f *f_in, class cl_f *f_out)
{
  //bool raw= false;
  //serial_raw_option->get_value(&raw);
  make_io();
  if (!io)
    return;
  new_o(f_out);
  new_i(f_in);
}

void
cl_serial_hw::new_i(class cl_f *f_in)
{
  bool raw= false;

  serial_raw_option->get_value(&raw);
  io->replace_files(true, f_in, io->get_fout());
  if (!f_in)
    return;
  set_raw();
  if (f_in->tty)
    {
      char esc= (char)cfg_get(serconf_escape);
      io->tu_reset();
      io->dd_printf("%s[%d] terminal display, press ^%c to access control menu\n",
		    id_string, id,
		    'a'+esc-1);
    }
  menu= 0;
}

void
cl_serial_hw::new_o(class cl_f *f_out)
{
  bool raw= false;

  serial_raw_option->get_value(&raw);
  io->replace_files(true, io->get_fin(), f_out);
  if (!f_out)
    return;
  enum file_type ft= f_out->determine_type();
  if ((ft != F_FILE) && (ft != F_CHAR))
    {
      f_out->set_telnet(!raw);
      if (!raw)
	{
	  io->tu_reset();
	}
    }
}

void
cl_serial_hw::del_listener_i(void)
{
  class cl_commander_base *c= application->get_commander();
  if (listener_io)
    {
      if (listener_i == listener_io)
	listener_i= NULL;
      if (listener_o == listener_io)
	listener_o= NULL;
    }
  if (c && listener_io)
    c->del_console(listener_io);
  listener_io= NULL;
  if (c && listener_i)
    c->del_console(listener_i);
  listener_i= NULL;
}

void
cl_serial_hw::del_listener_o(void)
{
  class cl_commander_base *c= application->get_commander();
  if (listener_io)
    {
      if (listener_i == listener_io)
	listener_i= NULL;
      if (listener_o == listener_io)
	listener_o= NULL;
    }
  if (c && listener_io)
    c->del_console(listener_io);
  listener_io= NULL;
  if (c && listener_o)
    c->del_console(listener_o);
  listener_o= NULL;
}

bool
cl_serial_hw::proc_input(void)
{
  int c;
  char esc= (char)cfg_get(serconf_escape);
  class cl_f *fin, *fout;
  int flw= cfg_get(serconf_flowctrl);
  int able= cfg_get(serconf_able_receive);

  fin= io->get_fin();
  fout= io->get_fout();

  if (fin->eof())
    {
      if (fout &&
	  (fout->file_id == fin->file_id))
	{
	  delete fout;
	  io->replace_files(false, fin, 0);
	  fout= 0;
	}
      delete fin;
      io->replace_files(false, 0, fout);
      return true;
    }
  if (menu == 0)
    proc_not_in_menu(fin, fout);
  else
    proc_in_menu(fin, fout);
    
  return true;
}

void
cl_serial_hw::show_menu(void)
{
  char esc= (char)cfg_get(serconf_escape);
  io->dd_printf("\n");
  io->dd_cprintf("ui_title", "Simulator control menu\n");
  io->dd_cprintf("ui_mkey", " %c      ", 'a'+esc-1);
  io->dd_cprintf("ui_mitem", "Insert ^%c\n", 'a'+esc-1);
  io->dd_cprintf("ui_mkey", " s,r,g  ");
  io->dd_cprintf("ui_mitem", "Start simulation\n");
  io->dd_cprintf("ui_mkey", " p      ");
  io->dd_cprintf("ui_mitem", "Stop simulation\n");
  io->dd_cprintf("ui_mkey", " T      ");
  io->dd_cprintf("ui_mitem", "Reset CPU\n");
  io->dd_cprintf("ui_mkey", " q      ");
  io->dd_cprintf("ui_mitem", "Quit simulator\n");
  io->dd_cprintf("ui_mkey", " o      ");
  io->dd_cprintf("ui_mitem", "Close serial terminal\n");
  io->dd_cprintf("ui_mkey", " e      ");
  io->dd_cprintf("ui_mitem", "Exit menu\n");
  io->dd_cprintf("ui_mkey", " n      ");
  io->dd_cprintf("ui_mitem", "Change display\n");
}

bool
cl_serial_hw::proc_not_in_menu(cl_f *fin, cl_f *fout)
{
  int c;
  int flw= cfg_get(serconf_flowctrl);
  int able= cfg_get(serconf_able_receive);
  char esc= (char)cfg_get(serconf_escape);

  if (fin->tty /*&& !flw*/ && esc)
    {
      if (fin->read(&c, 1))
	{
	  if (c == esc)
	    {
	      menu= 'm';
	      show_menu();
	    }
	  else if (!input_avail)
	    {
	      if (skip_nl /*&& is_nl(c)*/ && (c==skip_nl))
		;
	      else
		{
		  input= c;
		  input_avail= true;
		  skip_nl= 0;
		}
	    }
	  else
	    {
	      //fin->unget(c);
	      fprintf(stderr, "%s[%d] Character %d queued for RX, skip %d\n",
		      get_name(), id, input, c);
	    }
	}
    }
  else if (!input_avail)
    {
      if (!flw ||
	  able)
	{
	  if (fin->read(&c, 1))
	    {
	      if (skip_nl /*&& is_nl(c)*/ && (c==skip_nl))
		;
	      else
		{
		  input= c;
		  input_avail= true;
		  cfg_set(serconf_able_receive, 0);
		  skip_nl= 0;
		}
	    }
	}
    }
  return true;
}

bool
cl_serial_hw::proc_in_menu(cl_f *fin, cl_f *fout)
{
  int c;
  char esc= (char)cfg_get(serconf_escape);
  bool run= uc->sim->state & SIM_GO;

  if (fin->read(&c, 1))
    {
      switch (menu)
	{
	case 'm':
	  if ((c == esc-1+'a') ||
	      (c == esc-1+'A') ||
	      (c == esc))
	    {
	      // insert ^esc
	      if (run && !input_avail)
		{
		  input= esc, input_avail= true;
		  io->dd_printf("^%c entered.\n", 'a'+esc-1);
		}
	      else
		io->dd_printf("Control menu exited.\n");
	      menu= 0;
	    }
	  switch (c)
	    {
	    case 'e': case 'E': case 'e'-'a'+1:
	      // exit menu
	      menu= 0;
	      io->dd_printf("Control menu exited.\n");
	      break;
	    case 's': case 'S': case 's'-'a'+1:
	    case 'r': case 'R': case 'r'-'a'+1:
	    case 'g': case 'G': case 'g'-'a'+1:
	      // start
	      uc->sim->start(0, 0);
	      menu= 0;
	      io->dd_printf("Simulation started.\n");
	      break;
	    case 'p': case 'P': case 'p'-'a'+1:
	      uc->sim->stop(resSIMIF);
	      // stop
	      menu= 0;
	      io->dd_printf("Simulation stopped.\n");
	      break;
	    case 'T':
	      uc->reset();
	      menu= 0;
	      io->dd_printf("CPU reset.\n");
	      break;
	    case 'q': case 'Q': case 'q'-'a'+1:
	      // kill
	      uc->sim->state|= SIM_QUIT;
	      menu= 0;
	      io->dd_printf("Exit simulator.\n");
	      break;
	    case 'o': case 'O': case 'o'-'a'+1:
	      {
		// close
		io->dd_printf("Closing terminal.\n");
		menu= 0;
		io->convert2console();
		break;
	      }
	    case 'n': case 'N': case 'n'-'a'+1:
	      {
		class cl_hw *h= next_displayer();
		if (!h)
		  io->dd_printf("No other displayer.\n");
		else
		  {
		    io->tu_reset();
		    io->tu_cls();
		    io->pass2hw(h);
		  }
		menu= 0;
		break;
	      }
	    default:
	      menu= 0;
	      io->dd_printf("Control menu closed (%d).\n", c);
	      break;
	    }
	  break;
	}
    }
  return true;
}

void
cl_serial_hw::reset(void)
{
  cfg_set(serconf_able_receive, 1);
  sending_nl= false;
  nl_send_idx= 0;
  skip_nl= 0;
}

bool
cl_serial_hw::prediv_bitcnt(int cycles)
{
  mcnt+= cycles;
  if (mcnt >= cpb)
    {
      mcnt-= cpb;
      if (ten && (s_tr_bit < bits))
	s_tr_bit++;
      if (ren && (s_rec_bit < bits))
	s_rec_bit++;
      return true;
    }
  return false;
}


/*
 * Network listener for UART
 */

cl_serial_listener::cl_serial_listener(int serverport, class cl_app *the_app,
				       class cl_serial_hw *the_serial,
				       enum ser_listener_for slf):
  cl_listen_console(serverport, the_app)
{
  serial_hw= the_serial;
  sl_for= slf;
}

int
cl_serial_listener::init(void)
{
  if (serial_hw)
    set_name(chars("", "serial_listener_%s_%d\n", serial_hw->get_name(), serial_hw->id));
  return 0;
}

int
cl_serial_listener::proc_input(class cl_cmdset *cmdset)
{
  class cl_f *i, *o;
  bool raw= false;

  serial_hw->serial_raw_option->get_value(&raw);
  switch (sl_for)
    {
    case sl_io:
      srv_accept(fin, &i, &o);
      serial_hw->new_io(i, o);
      break;
    case sl_i:
      srv_accept(fin, &i, NULL);
      serial_hw->new_i(i);
      break;
    case sl_o:
      srv_accept(fin, NULL, &o);
      serial_hw->new_o(o);
      break;
    }
  return 0;
}


/* End of sim.src/serial_hw.cc */
