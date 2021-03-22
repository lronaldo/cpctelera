/*
 * Simulator of microcontrollers (hw.cc)
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

//#include "ddconfig.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
//#include "i_string.h"

//#include "stypes.h"
#include "globals.h"

#include "hwcl.h"


/*
 *____________________________________________________________________________
 */

cl_hw::cl_hw(class cl_uc *auc, enum hw_cath cath, int aid, const char *aid_string):
  cl_guiobj()
{
  flags= HWF_INSIDE;
  uc= auc;
  cathegory= cath;
  id= aid;
  if (aid_string &&
      *aid_string)
    id_string= strdup(aid_string);
  else
    id_string= strdup("unknown hw element");
  set_name(id_string);
  char *s= (char*)malloc(strlen(get_name("hw"))+100);
  sprintf(s, "partners of %s", get_name("hw"));
  partners= new cl_list(2, 2, s);
  sprintf(s, "watched cells of %s", get_name("hw"));
  free(s);
  cfg= 0;
  io= 0;
}

cl_hw::~cl_hw(void)
{
  free(const_cast<char *>(id_string));
  delete partners;
}

int
cl_hw::init(void)
{
  chars n(id_string);
  char s[100];
  int i;

  on= true;
  
  snprintf(s, 99, "%d", id);
  n+= '_';
  n+= s;
  n+= "_cfg";

  cfg= new cl_address_space(n, 0, cfg_size(), sizeof(t_mem)*8);
  cfg->init();
  cfg->hidden= true;
  uc->address_spaces->add(cfg);

  for (i= 0; i < cfg_size(); i++)
    {
      cfg->register_hw(i, this, false);
    }

  cache_run= -1;
  cache_time= 0;
  return 0;
}

void
cl_hw::new_hw_adding(class cl_hw *new_hw)
{
}

void
cl_hw::new_hw_added(class cl_hw *new_hw)
{
  int i;

  for (i= 0; i < partners->count; i++)
    {
      class cl_partner_hw *ph= (class cl_partner_hw *)(partners->at(i));
      ph->refresh(new_hw);
    }
}

class cl_hw *
cl_hw::make_partner(enum hw_cath cath, int id)
{
  class cl_partner_hw *ph;
  class cl_hw *hw;

  ph= new cl_partner_hw(uc, cath, id);
  partners->add(ph);
  hw= ph->get_partner();
  return(hw);
}

t_mem
cl_hw::read(class cl_memory_cell *cell)
{
  conf(cell, NULL);
  return cell->get();
}

void
cl_hw::write(class cl_memory_cell *cell, t_mem *val)
{
  conf(cell, val);    
}

bool
cl_hw::conf(class cl_memory_cell *cell, t_mem *val)
{
  t_addr a;
  if (cfg->is_owned(cell, &a))
    {
      conf_op(cell, a, val);
      if (val)
	cell->set(*val);
      return true;
    }
  return false;
}

t_mem
cl_hw::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  return cell->get();
}

class cl_memory_cell *
cl_hw::cfg_cell(t_addr addr)
{
  if (addr >= cfg_size())
    return 0;
  return cfg->get_cell(addr);
}

void
cl_hw::cfg_set(t_addr addr, t_mem val)
{
  cfg->set(addr, val);
}

void
cl_hw::cfg_write(t_addr addr, t_mem val)
{
  cfg->write(addr, val);
}

t_mem
cl_hw::cfg_get(t_addr addr)
{
  return cfg->get(addr);
}

t_mem
cl_hw::cfg_read(t_addr addr)
{
  return cfg->read(addr);
}

const char *
cl_hw::cfg_help(t_addr addr)
{
  return "N/A";
}

void
cl_hw::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
  con->dd_printf("Nothing to do\n");
}

class cl_memory_cell *
cl_hw::register_cell(class cl_address_space *mem, t_addr addr)
{
  if (mem)
    mem->register_hw(addr, this, false);
  else
    printf("regcell JAJ no mem\n");
  return mem->get_cell(addr);
}

class cl_memory_cell *
cl_hw::register_cell(class cl_memory_cell *cell)
{
  if (cell)
    {
      cell->add_hw(this);
    }
  return cell;
}

void
cl_hw::unregister_cell(class cl_memory_cell *the_cell)
{
  if (the_cell)
    the_cell->remove_hw(this);
}


/*
 * Simulating `cycles' number of machine cycle
 */

int
cl_hw::tick(int cycles)
{
  return(0);
}

void
cl_hw::inform_partners(enum hw_event he, void *params)
{
  int i;

  for (i= 0; i < partners->count; i++)
    {
      class cl_partner_hw *ph= (class cl_partner_hw *)(partners->at(i));
      ph->happen(this, he, params);
    }
}

void
cl_hw::touch(void)
{
  refresh_display(false);
}

void
cl_hw::make_io()
{
  if (!io)
    {
      io= new cl_hw_io(this);
      io->init();
      application->get_commander()->add_console(io);
    }
}

void
cl_hw::new_io(class cl_f *f_in, class cl_f *f_out)
{
  make_io();
  if (!io)
    return ;
  io->tu_reset();
  io->replace_files(true, f_in, f_out);
  if (f_in)
    {
      f_in->interactive(NULL);
      f_in->raw();
      f_in->echo(NULL);
    }
  draw_display();
  //application->get_commander()->update_active();
}

void
cl_hw::new_i(class cl_f *f_in)
{
  make_io();
  if (!io)
    return ;
  io->tu_reset();
  io->replace_files(true, f_in, io->get_fout());
  if (f_in)
    {
      f_in->interactive(NULL);
      f_in->raw();
      f_in->echo(NULL);
    }
  draw_display();
}

void
cl_hw::new_o(class cl_f *f_out)
{
  make_io();
  if (!io)
    return ;
  io->tu_reset();
  io->replace_files(true, io->get_fin(), f_out);
  draw_display();
}

class cl_hw_io *
cl_hw::get_io(void)
{
  return io;
}

bool
cl_hw::proc_input(void)
{
  int c;

  if (!io)
    return false;
  
  class cl_f *fin= io->get_fin();
  class cl_f *fout= io->get_fout();
  
  if (fin)
    {
      if (fin->eof())
	{
	  if (fout &&
	      (fout->file_id == fin->file_id))
	    {
	      io->tu_reset();
	      delete fout;
	      io->replace_files(false, fin, 0);
	    }
	  delete fin;
	  io->replace_files(false, 0, 0);
	  return true;
	}
      fin->read(&c, 1);
      return handle_input(c);
    }
  return false;
}

bool
cl_hw::handle_input(int c)
{
  if (!io)
    return false;

  io->tu_go(1,3);
  io->tu_cll();
  switch (c)
    {
    case 's'-'a'+1: case 'r'-'a'+1: case 'g'-'a'+1:
      uc->sim->change_run();
      if (uc->sim->state & SIM_GO)
	io->dd_printf("Simulation started.");
      else
	io->dd_printf("Simulation stopped.");
      break;
    case 't'-'a'+1:
      uc->reset();
      io->dd_printf("CPU reset.");
      break;
    case 'q'-'a'+1:
      uc->sim->state|= SIM_QUIT;
      io->dd_printf("Exit simulator.");
      io->tu_reset();
      break;
    case 'o'-'a'+1:
      io->dd_printf("Closing display.");
      io->tu_reset();
      io->tu_cls();
      io->convert2console();
      break;
    case 'l'-'a'+1:
      draw_display();
      break;
    case 'n'-'a'+1:
      {
	class cl_hw *h= next_displayer();
	if (!h)
	  io->dd_printf("No other displayer.");
	else
	  {
	    io->tu_reset();
	    io->tu_cls();
	    io->pass2hw(h);
	  }
	break;
      }
    default:
      return false;
      break;
    }
  return true;
}

void
cl_hw::draw_state_time(bool force)
{
  int n= uc->sim->state & SIM_GO;
  if ((n != cache_run) ||
      force)
    {
      io->tu_go(66,1);
      if (n)
	io->dd_cprintf("ui_run" , "%s", "Run ");
      else
	io->dd_cprintf("ui_stop", "%s", "Stop");
      cache_run= n;
    }
  unsigned int t= (unsigned int)(uc->get_rtime()) * 1000;
  if ((t != cache_time) ||
      force)
    {
      io->tu_go(28,2);
      io->dd_cprintf("ui_time", "%u ms", t);
      if (t < cache_time)
	io->dd_printf("                ");
      cache_time= t;
    }
}

void
cl_hw::refresh_display(bool force)
{
  if (!io)
    return ;

  io->tu_hide();
  io->tu_go(1,4);
  io->dd_color("answer");
  print_info(io);
  draw_state_time(force);
  io->tu_show();
}

void
cl_hw::draw_display(void)
{
  if (!io)
    return ;
  io->tu_go(1, 1);
  io->dd_cprintf("ui_mkey", "[^s] ");
  io->dd_cprintf("ui_mitem", "Start/stop  ");
  io->dd_cprintf("ui_mkey", "[^t] ");
  io->dd_cprintf("ui_mitem", "reseT  ");
  io->dd_cprintf("ui_mkey", "[^q] ");
  io->dd_cprintf("ui_mitem", "Quit  ");
  io->dd_cprintf("ui_mkey", "[^o] ");
  io->dd_cprintf("ui_mitem", "clOse  ");
  io->dd_cprintf("ui_mkey", "[^l] ");
  io->dd_cprintf("ui_mitem", "redraw\n");
  io->dd_cprintf("ui_mkey", "[^n] ");
  io->dd_cprintf("ui_mitem", "chaNge display  ");
  io->dd_cprintf("ui_label", "Time: ");
  io->tu_go(66,2);
  chars s("", "%s[%d]", id_string, id);
  io->dd_cprintf("ui_title", "%-13s", s.c_str());

  io->tu_go(1,3);
  io->dd_printf("\033[2K"); // entire line
  io->dd_printf("\033[0J"); // from cursor to end of screen
  io->dd_printf("\n");
}

class cl_hw *
cl_hw::next_displayer(void)
{
  if (!uc)
    return NULL;
  return uc->hws->next_displayer(this);
}


void
cl_hw::print_info(class cl_console_base *con)
{
  con->dd_printf("%s[%d]\n", id_string, id);
  //print_cfg_info(con);
}

void
cl_hw::print_cfg_info(class cl_console_base *con)
{
  t_mem v;
  t_addr a, s, e;
  con->dd_printf("Configuration memory of %s\n", get_name());
  if (cfg)
    {      
      s= cfg->get_start_address();
      e= s + cfg->get_size()-1;
      for (a= s; a <= e; a++)
	{
	  v= cfg->read(a);
	  con->dd_cprintf("dump_address", "0x%02x ", AU(a));
	  con->dd_cprintf("dump_number", "%08x ",v);
	  if ((v < 128) &&
	      isprint((int)v))
	    con->dd_cprintf("dump_char", "%c", v);
	  else
	    con->dd_cprintf("dump_char", ".");
	  con->dd_printf(" %s\n", cfg_help(a));
	}
    }
}

/*
 * List of hw
 */

t_index
cl_hws::add(void *item)
{
  int i;
  t_index res;

  // pre-add
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->new_hw_adding((class cl_hw *)item);
    }
  // add
  res= cl_list::add(item);
  // post-add
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->new_hw_added((class cl_hw *)item);
    }
  ((class cl_hw *)item)->added_to_uc();
  return(res);
}

class cl_hw *
cl_hws::next_displayer(class cl_hw *hw)
{
  int i, j;
  cl_hw_io *io;
  cl_f *fi, *fo;
  
  if (!index_of(hw, &i))
    return NULL;

  for (j= i+1; j < count; j++)
    {
      class cl_hw *h= (class cl_hw *)(at(j));
      h->make_io();
      if ((io= h->get_io()))
	{
	  fi= io->get_fin();
	  fo= io->get_fout();
	  if (!fi &&
	      !fo)
	    return h;
	}
    }
  for (j= 0; j < i; j++)
    {
      class cl_hw *h= (class cl_hw *)(at(j));
      h->make_io();
      if ((io= h->get_io()))
	{
	  fi= io->get_fin();
	  fo= io->get_fout();
	  if (!fi &&
	      !fo)
	    return h;
	}
    }
  return NULL;
}

		       
/*
 *____________________________________________________________________________
 */

cl_partner_hw::cl_partner_hw(class cl_uc *auc, enum hw_cath cath, int aid):
  cl_base()
{
  uc= auc;
  cathegory= cath;
  id= aid;
  partner= uc->get_hw(cathegory, id, 0);
}

class cl_hw *
cl_partner_hw::get_partner(void)
{
  return(partner);
}

void
cl_partner_hw::refresh(void)
{
  class cl_hw *hw= uc->get_hw(cathegory, id, 0);

  if (!hw)
    return;
  if (partner)
    {
      // partner is already set
      if (partner != hw)
	{
	  // partner changed?
	  partner= hw;
	}
      else
	partner= hw;
    }
  partner= hw;
}

void
cl_partner_hw::refresh(class cl_hw *new_hw)
{
  if (!new_hw)
    return;
  if (cathegory == new_hw->cathegory &&
      id == new_hw->id)
    {
      if (partner)
	{
	  // partner changed?
	  partner= new_hw;
	}
      else
	partner= new_hw;
    }
}

void
cl_partner_hw::happen(class cl_hw *where, enum hw_event he, void *params)
{
  if (partner)
    partner->happen(where, he, params);
}


/*
 *____________________________________________________________________________
 */

cl_hw_io::cl_hw_io(class cl_hw *ihw):
  cl_console()
{
  hw= ihw;
  set_name(chars("", "%s[%d]", ihw->id_string, ihw->id));
}

int
cl_hw_io::init(void)
{
  set_flag(CONS_NOWELCOME, true);
  return 0;
}

int
cl_hw_io::proc_input(class cl_cmdset *cmdset)
{
  if (hw)
    hw->proc_input();
  else
    {
      int c;
      fin->read(&c, 1);
      dd_printf("Unhandled hwio command: %c %d 0x%02x\n", isprint(c)?c:'?', c, c);
    }
  return 0;
}


void
cl_hw_io::convert2console(void)
{
  if (fin &&
      fout)
    {
      class cl_console *con= new cl_console(fin, fout, application);
      con->init();
      application->get_commander()->add_console(con);
    }
  drop_files();
}

void
cl_hw_io::pass2hw(class cl_hw *new_hw)
{
  if (new_hw)
    new_hw->new_io(fin, fout);
  drop_files();
}


/* End of hw.cc */
