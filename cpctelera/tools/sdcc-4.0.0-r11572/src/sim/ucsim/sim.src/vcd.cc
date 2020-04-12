/*
 * Simulator of microcontrollers (sim.src/vcd.cc)
 *
 * Copyright (C) 2017,17 Drotos Daniel, Talker Bt.
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
#include <time.h>
#include <string.h>

// prj
#include "utils.h"

// sim
#include "argcl.h"

// local
#include "vcdcl.h"


cl_vcd::cl_vcd(class cl_uc *auc, int aid, chars aid_string):
       cl_hw(auc, HW_DUMMY, aid, aid_string)
{
  locs= new cl_list(2, 2, "vcd_locs");
  started= false;
  paused= false;
  fout= 0;
  change= 0;
  modul= chars("", "ucsim_vcd_%d", id);
}

void
cl_vcd::add(class cl_memory_cell *cell)
{
  if (!cell)
    return;
  if (!locs->index_of(cell, NULL))
    {
      locs->add(cell);
      register_cell(cell);
    }
}

void
cl_vcd::del(class cl_memory_cell *cell)
{
  if (!cell)
    return;
  if (locs->index_of(cell, NULL))
    {
      unregister_cell(cell);
      locs->disconn(cell);
    }
}

bool
cl_vcd::add(class cl_memory *m, t_addr a, class cl_console_base *con)
{
  if (!m->is_address_space())
    {
      if (con) con->dd_printf("%s is not an address space\n");
      return false;
    }
  if (!m->valid_address(a))
    {
      if (con) con->dd_printf("Address must be between 0x%x and 0x%x\n",
			      AU(m->lowest_valid_address()),
			      AU(m->highest_valid_address()));
      return false;
    }
  cl_memory_cell *c= ((cl_address_space*)m)->get_cell(a);
  if (c->get_flag(CELL_NON_DECODED))
    {
      if (con) con->dd_printf("Cell is not decoded\n");
      return false;
    }
  add(c);
  return true;
}

bool
cl_vcd::del(class cl_memory *m, t_addr a, class cl_console_base *con)
{
  if (!m->is_address_space())
    {
      if (con) con->dd_printf("%s is not an address space\n");
      return false;
    }
  if (!m->valid_address(a))
    {
      if (con) con->dd_printf("Address must be between 0x%x and 0x%x\n",
			      AU(m->lowest_valid_address()),
			      AU(m->highest_valid_address()));
      return false;
    }
  del(((cl_address_space*)m)->get_cell(a));
  return true;
}

void
cl_vcd::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
  class cl_cmd_arg *params[3]= {
    cmdline->param(0),
    cmdline->param(1),
    cmdline->param(2)
  };

  if (cmdline->syntax_match(uc, MEMORY ADDRESS)) // ADD
    {
      if (started)
	{
	  con->dd_printf("Already started\n");
	  return;
	}
      class cl_memory *mem= params[0]->value.memory.memory;
      t_addr a= params[1]->value.address;
      if (!mem->is_address_space())
	{
	  con->dd_printf("%s is not an address space\n");
	  return;
	}
      if (!mem->valid_address(a))
	{
	  con->dd_printf("Address must be between 0x%x and 0x%x\n",
			 AU(mem->lowest_valid_address()),
			 AU(mem->highest_valid_address()));
	  return;
	}
      add(mem, a, con);
      return;
    }
  else if (cmdline->syntax_match(uc, CELL)) // ADD
    {
      if (started)
	{
	  con->dd_printf("Already started\n");
	  return;
	}
      if (params[0]->value.cell->get_flag(CELL_NON_DECODED))
	con->dd_printf("Cell is not decoded\n");
      else
	add(params[0]->value.cell);
      return;
    }
  else if (cmdline->syntax_match(uc, STRING MEMORY ADDRESS)) // DEL|ADD
    {
      if (started)
	{
	  con->dd_printf("Already started\n");
	  return;
	}
      params[0]->as_string();
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
	{
	  if (strcmp(p1, "add") == 0)
	    {
	      if (add(params[1]->value.memory.memory, params[2]->value.address, con))
		return;
	    }
	  if (strstr(p1, "del") == p1)
	    {
	      if (del(params[1]->value.memory.memory, params[2]->value.address, con))
		return;
	    }
	}
    }
  else if (cmdline->syntax_match(uc, STRING CELL)) // DEL|ADD
    {
      if (started)
	{
	  con->dd_printf("Already started\n");
	  return;
	}
      params[0]->as_string();
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
	{
	  if (strcmp(p1, "add") == 0)
	    {
	      if (params[1]->value.cell->get_flag(CELL_NON_DECODED))
		con->dd_printf("Cell is not decoded\n");
	      else
		add(params[1]->value.cell);
	      return;
	    }
	  if (strstr(p1, "del") == p1)
	    {
	      del(params[1]->value.cell);
	      return;
	    }
	}
    }
  else if (cmdline->syntax_match(uc, STRING NUMBER)) // NEW id
    {
      params[0]->as_string();
      char *p1= params[0]->value.string.string;
      if (p1 && *p1 &&
	  (strcmp(p1, "new") == 0))
	{
	  params[1]->as_number();
	  int nid= params[1]->value.number;
	  if (uc->get_hw((char*)id_string, nid, NULL) != NULL)
	    {
	      con->dd_printf("Already exists\n");
	      return;
	    }
	  cl_hw *h= new cl_vcd(uc, nid, id_string);
	  h->init();
	  uc->add_hw(h);
	  return;
	}
    }
  else if (cmdline->syntax_match(uc, STRING STRING)) // FILE, MOD
    {
      params[0]->as_string();
      params[1]->as_string();
      char *p1= params[0]->value.string.string;
      char *p2= params[1]->value.string.string;
      if (started)
	{
	  con->dd_printf("Already started\n");
	  return;
	}
      if (p1 && *p1)
	{
	  if (!p2 || !*p2)
	    {
	      con->dd_printf("Name missing\n");
	      return;
	    }
	  if ((strcmp(p1, "fout") == 0) ||
	      (strcmp(p1, "file") == 0))
	    {
	      if ((fout= mk_io(p2, "w")) == NULL)
		con->dd_printf("File open error\n");
	      return;
	    }
	  if (strstr(p1, "mod") == p1)
	    {
	      modul= chars(p2);
	      return;
	    }
	}
    }
  else if (cmdline->syntax_match(uc, STRING)) // [RE]START, PAUSE, STOP
    {
      params[0]->as_string();
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
	{
	  if ((strstr(p1, "re") == p1) ||
	      (strcmp(p1, "start") == 0))
	    {
	      if (started)
		paused= false;
	      else
		{
		  if (!fout)
		    con->dd_printf("Output unspecified\n");
		  else
		    {
		      // generate vcd file header
		      time_t t= time(NULL);
		      fout->write_str("$date\n");
		      fout->write_str(ctime(&t));
		      fout->write_str("$end\n");
		      fout->write_str("$version\n");
		      fout->prntf("ucsim\n");
		      fout->write_str("$end\n");
		      fout->write_str("$timescale 1ns $end\n");
		      fout->prntf("$scope module %s $end\n", (char*)modul);
		      int i;
		      for (i= 0; i < locs->count; i++)
			{
			  cl_memory_cell *c= (cl_memory_cell *)
			    (locs->at(i));
			  chars n= uc->cell_name(c);
			  fout->prntf("$var wire %d %c %s $end\n",
				      c->get_width(), 33+i,
				      (char*)n);
			}
		      fout->write_str("$upscope $end\n");
		      fout->write_str("$enddefinitions $end\n");
		      fout->write_str("$dumpvars\n");
		      for (i= 0; i < locs->count; i++)
			{
			  cl_memory_cell *c= (cl_memory_cell *)
			    (locs->at(i));
			  report(c, i);
			}
		      fout->write_str("$end\n");
		      started= true;
		      paused= false;
		      change= false;
		    }
		}
	      return;
	    }
	  if (strstr(p1, "paus") == p1)
	    {
	      if (started)
		paused= !paused;
	      return;
	    }
	  if (strcmp(p1, "stop") == 0)
	    {
	      if (started)
		{
		  if (fout)
		    delete fout;
		  fout= NULL;
		}
	      started= paused= change= false;
	      return;
	    }
	  if (strcmp(p1, "info") == 0)
	    {
	      print_info(con);
	      return;
	    }
	}
    }
  //else
    {
      con->dd_printf("set hardware vcd[id] [add] memory address\n");
      con->dd_printf("set hardware vcd[id] del[ete] memory address\n");
      con->dd_printf("set hardware vcd[id] fout|file \"vcd_file_name\"|gtkwave\n");
      con->dd_printf("set hardware vcd[id] mod[ule] module_name\n");
      con->dd_printf("set hardware vcd[id] start\n");
      con->dd_printf("set hardware vcd[id] pause\n");
      con->dd_printf("set hardware vcd[id] [re]start\n");
      con->dd_printf("set hardware vcd[id] stop\n");
      con->dd_printf("set hardware vcd[id] new id\n");
    }
}

t_mem
cl_vcd::read(class cl_memory_cell *cell)
{

  conf(cell, NULL);
  return cell->get();
}

void
cl_vcd::write(class cl_memory_cell *cell, t_mem *val)
{
  if (started &&
      !paused)
    {
      if (cell->def_data != *val)
	{
	  //change_time= uc->get_rtime();
	  change= true;
	}
    }      
  if (conf(cell, val))
    return;
}

t_mem
cl_vcd::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  if (addr >= 1)
    return cell->get();
  switch (addr)
    {
    }
  return cell->get();
}

char *
cl_vcd::cfg_help(t_addr addr)
{
  return (char*)"Not used";
}

void
cl_vcd::report(class cl_memory_cell *cell, int nr)
{
  t_mem v= cell->get();
  if (fout)
    {
      int w= cell->get_width();
      if (w == 1)
	{
	  fout->prntf("%d%c\n", v?1:0, nr+33);
	}
      else
	{
	  fout->write((char*)"b", 1);
	  fout->prntf("%s %c\n", (char*)cbin(v, w), nr+33);
	}
    }
  cell->def_data= v;
}

int
cl_vcd::tick(int cycles)
{
  if (change)
    {
      int i;
      change_time= uc->get_rtime();
      if (fout)
	fout->prntf("#%lu\n", (unsigned long)(change_time * 1000000000));
      for (i= 0; i < locs->count; i++)
	{
	  class cl_memory_cell *c= (cl_memory_cell*)(locs->at(i));
	  if (c->get() != c->def_data)
	    {
	      report(c, i);
	    }
	}
      change= false;
    }
  return 0;
}

void
cl_vcd::print_info(class cl_console_base *con)
{
  int i;
  con->dd_printf("%s[%d] value change dump\n", id_string, id);
  con->dd_printf("Started: %s Paused: %s\n",
		 started?"YES":"no",
		 paused?"YES":"no");
  const char *fn= fout?(fout->get_file_name()):"(none)";
  con->dd_printf("Modul: %s File: %s\n", (char*)modul, fn);
  con->dd_printf("Memory cells:\n");
  for (i= 0; i < locs->count; i++)
    {
      cl_memory_cell *c= (cl_memory_cell*)(locs->at(i));
      cl_address_space *as;
      t_addr a= 0;
      as= uc->address_space(c, &a);
      con->dd_printf("  %s[0x%x] %s\n", as?(as->get_name()):"?", AU(a), (char*)(uc->cell_name(c)));
    }
  print_cfg_info(con);
}


/* End of sim.src/vcd.cc */
