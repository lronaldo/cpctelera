/*
 * Simulator of microcontrollers (pia.cc)
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

#include <stdio.h>
#include <ctype.h>

// cmd
#include "argcl.h"

// sim
#include "itsrccl.h"

#include "piacl.h"


cl_pia::cl_pia(class cl_uc *auc, int aid):
  cl_hw(auc, HW_PORT, aid, "pia")
{
  base= 0xc010;
}

cl_pia::cl_pia(class cl_uc *auc, int aid, t_addr the_addr):
  cl_hw(auc, HW_PORT, aid, "pia")
{
  base= the_addr;
}

int
cl_pia::init(void)
{
  cl_hw::init();

  rs[0]= register_cell(uc->rom, base+0);
  rs[1]= register_cell(uc->rom, base+1);
  rs[2]= register_cell(uc->rom, base+2);
  rs[3]= register_cell(uc->rom, base+3);

  cra = rs[1];
  ddra= cfg_cell(cfg_ddra);
  ora = cfg_cell(cfg_ora);
  ina = cfg_cell(cfg_ina);
  crb = rs[2];
  ddrb= cfg_cell(cfg_ddrb);
  orb = cfg_cell(cfg_orb);
  inb = cfg_cell(cfg_inb);
  
  oca = cfg_cell(cfg_oca);
  ddca= cfg_cell(cfg_ddca);
  inca= cfg_cell(cfg_inca);
  ocb = cfg_cell(cfg_ocb);
  ddcb= cfg_cell(cfg_ddcb);
  incb= cfg_cell(cfg_incb);

  cfg_set(cfg_reqs, 0);
  cfg_set(cfg_ca1_req, 'i');
  cfg_set(cfg_ca2_req, 'i');
  cfg_set(cfg_cb1_req, 'i');
  cfg_set(cfg_cb2_req, 'i');
  
  chars pn= chars("", "pia%d_", id);

  uc->vars->add(pn + "base", cfg, cfg_base, 7, 0, cfg_help(cfg_base));
  uc->vars->add(pn + "on", cfg, cfg_on, 7, 0, cfg_help(cfg_on));
  
  uc->vars->add(pn + "cra", uc->rom, base+1, 7, 0, "CRA Control Register port A");
  uc->vars->add(pn + "crb", uc->rom, base+3, 7, 0, "CRB Control Register port B");

  uc->vars->add(pn + "ddra", cfg, cfg_ddra, 7, 0, cfg_help(cfg_ddra));
  uc->vars->add(pn + "ora", cfg, cfg_ora, 7, 0, cfg_help(cfg_ora));
  uc->vars->add(pn + "ina", cfg, cfg_ina, 7, 0, cfg_help(cfg_ina));
  uc->vars->add(pn + "ira", cfg, cfg_ira, 7, 0, cfg_help(cfg_ira));

  uc->vars->add(pn + "ddrb", cfg, cfg_ddrb, 7, 0, cfg_help(cfg_ddrb));
  uc->vars->add(pn + "orb", cfg, cfg_orb, 7, 0, cfg_help(cfg_orb));
  uc->vars->add(pn + "inb", cfg, cfg_inb, 7, 0, cfg_help(cfg_inb));
  uc->vars->add(pn + "irb", cfg, cfg_irb, 7, 0, cfg_help(cfg_irb));

  uc->vars->add(pn + "oca", cfg, cfg_oca, 7, 0, cfg_help(cfg_oca));
  uc->vars->add(pn + "ddca", cfg, cfg_ddca, 7, 0, cfg_help(cfg_ddca));
  uc->vars->add(pn + "inca", cfg, cfg_inca, 7, 0, cfg_help(cfg_inca));

  uc->vars->add(pn + "ocb", cfg, cfg_ocb, 7, 0, cfg_help(cfg_ocb));
  uc->vars->add(pn + "ddcb", cfg, cfg_ddcb, 7, 0, cfg_help(cfg_ddcb));
  uc->vars->add(pn + "incb", cfg, cfg_incb, 7, 0, cfg_help(cfg_incb));

  uc->vars->add(pn + "reqs", cfg, cfg_reqs, 7, 0, cfg_help(cfg_reqs));
  uc->vars->add(pn + "ca1_req", cfg, cfg_ca1_req, 7, 0, cfg_help(cfg_ca1_req));
  uc->vars->add(pn + "ca2_req", cfg, cfg_ca2_req, 7, 0, cfg_help(cfg_ca2_req));
  uc->vars->add(pn + "cb1_req", cfg, cfg_cb1_req, 7, 0, cfg_help(cfg_cb1_req));
  uc->vars->add(pn + "cb2_req", cfg, cfg_cb2_req, 7, 0, cfg_help(cfg_cb2_req));

  is_ca1= new cl_it_src(uc, 1,
			cra, 1, //1,
			cra, 0x80,
			0, false, false,
			pn+"CA1",
			0);
  is_ca1->init();
  is_ca1->set_ie_value(1);
  is_ca1->set_parent(uc->search_it_src('i'));
  uc->it_sources->add(is_ca1);
  is_ca2= new cl_it_src(uc, 2,
			cra, 0x28, //0x08,
			cra, 0x40,
			0, false, false,
			pn+"CA2",
			0);
  is_ca2->init();
  is_ca2->set_ie_value(0x08);
  is_ca2->set_parent(uc->search_it_src('i'));
  uc->it_sources->add(is_ca2);
  is_cb1= new cl_it_src(uc, 3,
			crb, 1, //1,
			crb, 0x80,
			0, false, false,
			pn+"CB1",
			0);
  is_cb1->init();
  is_cb1->set_ie_value(1);
  is_cb1->set_parent(uc->search_it_src('i'));
  uc->it_sources->add(is_cb1);
  is_cb2= new cl_it_src(uc, 4,
			crb, 0x28, //0x08,
			crb, 0x40,
			0, false, false,
			pn+"CB2",
			0);
  is_cb2->init();
  is_cb2->set_parent(uc->search_it_src('i'));
  is_cb2->set_ie_value(0x08);
  uc->it_sources->add(is_cb2);
  
  return(0);
}

void
cl_pia::reset(void)
{
  cra->set(0);
  cra->write(0);
  ddra->write(0);
  ora->write(0);
  crb->set(0);
  crb->write(0);
  ddrb->write(0);
  orb->write(0);

  oca->write(2);
  ocb->write(2);

  prev_ca1= ca1();
  prev_ca2= ca2();
  prev_cb1= cb1();
  prev_cb2= cb2();
}

const char *
cl_pia::cfg_help(t_addr addr)
{
  switch ((enum pia_cfg)addr)
    {
    case cfg_on		: return "Turn/get on/off state (bool, RW)";
    case cfg_reqs	: return "IRQS of CA2, CA1, CB2, CB1 (int, RW)";
    case cfg_ca1_req	: return "Req mode of CA1 'i'=IRQ 'f'=FIRQ 'n'=NMI (int, RW)";
    case cfg_ca2_req	: return "Req mode of CA2 'i'=IRQ 'f'=FIRQ 'n'=NMI (int, RW)";
    case cfg_cb1_req	: return "Req mode of CB1 'i'=IRQ 'f'=FIRQ 'n'=NMI (int, RW)";
    case cfg_cb2_req	: return "Req mode of CB2 'i'=IRQ 'f'=FIRQ 'n'=NMI (int, RW)";
    case cfg_base	: return "Base address of the port (int, RW)";
    case cfg_ddra	: return "DDRA - Data Direction Register A (int, RW)";
    case cfg_ora	: return "ORA  - Peripheral Register A (int, RW)";
    case cfg_ina	: return "ina  - Outside value of port A pins (int, RW)";
    case cfg_ira	: return "ira  - Read value of port A (int RO)";
    case cfg_ddrb	: return "DDRB - Data Direction Register B (int, RW)";
    case cfg_orb	: return "ORB  - Peripheral Register B (int, RW)";
    case cfg_inb	: return "inb  - Outside value of port B pins (int, RW)";
    case cfg_irb	: return "irb  - Read value of port B (int RO)";
    case cfg_oca	: return "oca  - Output value of port CA (int, RW)";
    case cfg_ddca	: return "ddca - Direction of port CA (int, RW)";
    case cfg_inca	: return "inca - Outside value of port CA pins (int, RW)";
    case cfg_ocb	: return "ocb  - Output value of port CB (int, RW)";
    case cfg_ddcb	: return "ddcb - Direction of port CB (int, RW)";
    case cfg_incb	: return "incb - Outside value of port CB pins (int, RW)";
    }
  return "Not used";
}

class cl_memory_cell *
cl_pia::reg(class cl_memory_cell *cell_rs)
{
  if (cell_rs == rs[0])
    {
      if (cra->get() & 4)
	return ora;
      else
	return ddra;
    }
  else if (cell_rs == rs[1])
    return cra;
  else if (cell_rs == rs[2])
    {
      if (crb->get() & 4)
	return orb;
      else
	return ddrb;
    }
  else if (cell_rs == rs[3])
    return crb;
  return NULL;
}

t_mem
cl_pia::read(class cl_memory_cell *cell)
{
  class cl_memory_cell *r= reg(cell);
  conf(cell, NULL);
  if (r != NULL)
    {
      if (r == ora)
	{
	  cra->set(cra->get() & 0x3f);
	  if ((cra->get() & 0x30) == 0x20)
	    {
	      oca->write(oca->get() & ~0x02);
	    }
	  return ira();
	}
      if (r == orb)
	{
	  crb->set(crb->get() & 0x3f);
	  return irb();
	}
      return r->get();
    }
  return cell->get();
}

void
cl_pia::write(class cl_memory_cell *cell, t_mem *val)
{
  class cl_memory_cell *r= reg(cell);
  if (val)
    {
      if (r == cra || r == crb)
	{
	  *val&= 0x3f;
	  *val|= (r->get() & 0xc0);

	  if (r == cra)
	    {
	      if ((*val & 0x30) == 0x30)
		{
		  if (*val & 0x08)
		    oca->write(oca->get() | 0x02);
		  else
		    oca->write(oca->get() & ~0x02);
		}
	    }
	  if (r == crb)
	    {
	      if ((*val & 0x30) == 0x30)
		{
		  if (*val & 0x08)
		    ocb->write(ocb->get() | 0x02);
		  else
		    ocb->write(ocb->get() & ~0x02);
		}
	    }
	}
      if (r == orb)
	{
	  if ((crb->get() & 0x30) == 0x20)
	    ocb->write(ocb->get() & ~0x02);
	}
    }
  conf(cell, val);
  if (r)
    r->set(*val);
  check_edges();
}

t_mem
cl_pia::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  t_mem v;
  class cl_memory_cell *r= NULL;
  switch ((enum pia_cfg)addr)
    {
    case cfg_on	:  // turn this HW on/off
      if (val)
	{
	  if (*val)
	    on= true;
	  else
	    on= false;
	}
      v= on?1:0;
      cell->set(v);
      break;
    case cfg_base:
      if (val)
	{
	  int i;
	  if (uc->rom->valid_address(*val))
	    {
	      for (i= 0; i < 3; i++)
		unregister_cell(rs[i]);
	      base= *val;
	      init();
	    }
	}
      cell->set(base);
      break;
    case cfg_reqs	:
      if (val)
	{
	  u8_t ca= cra->get() & 0x3f;
	  ca|= (*val&8)?0x40:0;
	  ca|= (*val&4)?0x80:0;
	  cra->set(ca);
	  u8_t cb= crb->get() & 0x3f;
	  ca|= (*val&2)?0x40:0;
	  ca|= (*val&1)?0x80:0;
	  crb->set(cb);
	}
      else
	{
	  u8_t ca= cra->get(), cb= crb->get();
	  cell->set(
		    ((ca&0x40)?8:0) |
		    ((ca&0x80)?4:0) |
		    ((cb&0x40)?2:0) |
		    ((cb&0x80)?1:0)
		    );
	}
      break;
    case cfg_ca1_req	:
      if (val)
	if (is_ca1)
	  //is_ca1->set_pass_to(*val);
	  is_ca1->set_parent(uc->search_it_src(*val));
      break;
    case cfg_ca2_req	:
      if (val)
	if (is_ca2)
	  //is_ca2->set_pass_to(*val);
	  is_ca2->set_parent(uc->search_it_src(*val));
      break;
    case cfg_cb1_req	:
      if (val)
	if (is_cb1)
	  //is_cb1->set_pass_to(*val);
	  is_cb1->set_parent(uc->search_it_src(*val));
      break;
    case cfg_cb2_req	:
      if (val)
	if (is_cb2)
	  //is_cb2->set_pass_to(*val);
	  is_cb2->set_parent(uc->search_it_src(*val));
      break;
    case cfg_ddra	: r= ddra; break;
    case cfg_ora	: r= ora; break;
    case cfg_ina	: r= ina; break;
    case cfg_ira	:
      cell->set(ira());
      if (val)
	*val= cell->get();
      break;
    case cfg_ddrb	: r= ddrb; break;
    case cfg_orb	: r= orb; break;
    case cfg_inb	: r= inb; break;
    case cfg_irb	:
      cell->set(irb());
      if (val)
	*val= cell->get();
      break;
    case cfg_oca	: r= oca;
      if (val)
	*val&= 2;
      break;
    case cfg_ddca	: r= ddca;
      if (val)
	{
	  *val&= 2;
	  u8_t i= cra->get() & 0xdf;
	  if (*val) i|= 0x20;
	  cra->set(i);
	}
      else
	r->set((cra->get() & 0x20)?2:0);
      break;
    case cfg_inca	: r= inca;
      if (val)
	*val&= 3;
      break;
    case cfg_ocb	: r= ocb;
      if (val)
	*val&= 2;
      break;
    case cfg_ddcb	: r= ddcb;
      if (val)
	{
	  *val&= 2;
	  u8_t i= crb->get() & 0xdf;
	  if (*val) i|= 0x20;
	  crb->set(i);
	}
      else
	r->set((crb->get() & 0x20)?2:0);
      break;
    case cfg_incb	: r= incb;
      if (val)
	*val&= 3;
      break;
    }
  if (r)
    {
      if (val)
	r->set(*val);
      v= r->get();
      cell->set(v);
    }
  v= cell->get();
  return v;
}

int
cl_pia::ira()
{
  u8_t i, o, d;
  d= ddra->get();
  i= ina->get();
  o= ora->get();
  return (~d&i) | (d&o&i);
}

int
cl_pia::irb()
{
  u8_t i, o, d;
  d= ddrb->get();
  i= inb->get();
  o= orb->get();
  return (~d&i) | (d&o);	    
}

int
cl_pia::ca1()
{
  return inca->get() & 1;
}

int
cl_pia::ca2(void)
{
  u8_t ca= cra->get();
  if (ca & 0x20)
    {
      // out
      return (oca->get() & 2)?1:0;
    }
  // input
  return (inca->get() & 2)?1:0;
  return 0;
}

int
cl_pia::cb1()
{
  return incb->get() & 1;
}

int
cl_pia::cb2(void)
{
  u8_t cb= crb->get();
  if (cb & 0x20)
    {
      // out
      return (ocb->get() & 2)?1:0;
    }
  // input
  return (incb->get() & 2)?1:0;
  return 0;
}

int
cl_pia::check_edges(void)
{
  class cl_memory_cell *cr= cra;
  int signal= ca1();
  int edge= ((cr->get())&2)>>1;
  int *prev= &prev_ca1;
  
  if (*prev != signal)
    {
      *prev= signal;
      if (!(edge ^ signal))
	{
	  cr->set(cr->get() | 0x80);
	  if ((cr->get() & 0x38) == 0x20)
	    {
	      oca->write(oca->get() | 0x02);
	    }
	}
    }
  if (!(cr->get() & 0x20))
    {
      signal= ca2();
      edge= (cr->get() & 0x10) >> 4;
      prev= &prev_ca2;
      if (*prev != signal)
	{
	  *prev= signal;
	  if (!(edge ^ signal))
	    cr->set(cr->get() | 0x40);
	}
    }
  
  cr= crb;
  signal= cb1();
  edge= ((cr->get())&2)>>1;
  prev= &prev_cb1;
  if (*prev != signal)
    {
      *prev= signal;
      if (!(edge ^ signal))
	{
	  cr->set(cr->get() | 0x80);
	  if ((cr->get() & 0x38) == 0x20)
	    {
	      // TODO: CRB-b7 must first be cleared by a read of data.
	      ocb->write(ocb->get() | 0x02);
	    }
	}
    }
  if (!(cr->get() & 0x20))
    {
      signal= cb2();
      edge= (cr->get() & 0x10) >> 4;
      prev= &prev_cb2;
      if (*prev != signal)
	{
	  *prev= signal;
	  if (!(edge ^ signal))
	    cr->set(cr->get() | 0x40);
	}
    }
  return 0;
}

int
cl_pia::tick(int cycles)
{
  if ((cra->get() & 0x38) == 0x28)
    {
      oca->write(oca->get() | 0x02);
    }
  if ((crb->get() & 0x38) == 0x28)
    {
      ocb->write(ocb->get() | 0x02);
    }
  return 0;
}

bool
cl_pia::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
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
      for (i= 0; i < 3; i++)
	unregister_cell(rs[i]);
      base= a;
      init();
      return true; // handled
    }
  return false; // unhandled
}

void
cl_pia::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware pia[%d] address\n", id);
}

void
cl_pia::print_info(class cl_console_base *con)
{
  u8_t ca= cra->read();
  u8_t cb= crb->read();
  con->dd_printf("%s[%d] at 0x%06x %s\n", id_string, id, base, on?"on ":"off");
  con->dd_printf("0x%04x ", base+0);
  if (ca & 4)
    con->dd_printf(" ORA 0x%02x", ora->get());
  else
    con->dd_printf("DDRA 0x%02x", ddra->get());
  con->dd_printf("   IRA 0x%02x", ira());
  con->dd_printf("\n");
  con->dd_printf("0x%04x ", base+1);
  con->dd_printf(" CRA 0x%02x", ca);
  con->dd_printf("   IRQA1 %d", (ca&0x80)?1:0);
  con->dd_printf("   EnA1 %d", (ca&0x01)?1:0);
  con->dd_printf("   EdgA1 %c", (ca&0x02)?'R':'F');
  con->dd_printf("   CA1 %d", ca1());
  con->dd_printf("\n                ");
  con->dd_printf("   IRQA2 %d", (ca&0x40)?1:0);
  con->dd_printf("   EnA2 %d", ((ca&0x28)==0x08)?1:0);
  con->dd_printf("   EdgA2 %c", (ca&0x10)?'R':'F');
  con->dd_printf("   CA2 %d", ca2());
  con->dd_printf("\n                ");
  con->dd_printf("   DirA2 %s", (ca&0x20)?"Out":"In ");
  con->dd_printf(" ModA2 ");
       if ((ca&0x38) == 0x30) con->dd_printf("Output 0               ");
  else if ((ca&0x38) == 0x38) con->dd_printf("Output 1               ");
  else if ((ca&0x38) == 0x20) con->dd_printf("Read strobe, A1 restore");
  else if ((ca&0x38) == 0x28) con->dd_printf("Read strobe,  E restore");
  else                        con->dd_printf("Input                  ");
  con->dd_printf("\n");
  
  con->dd_printf("0x%04x ", base+2);
  if (cb & 4)
    con->dd_printf(" ORB 0x%02x", orb->get());
  else
    con->dd_printf("DDRB 0x%02x", ddrb->get());
  con->dd_printf("   IRB 0x%02x", irb());
  con->dd_printf("\n");
  con->dd_printf("0x%04x ", base+3);
  con->dd_printf(" CRB 0x%02x", cb);
  con->dd_printf("   IRQB1 %d", (cb&0x80)?1:0);
  con->dd_printf("   EnB1 %d", (cb&0x01)?1:0);
  con->dd_printf("   EdgB1 %c", (cb&0x02)?'R':'F');
  con->dd_printf("   CB1 %d", cb1());
  con->dd_printf("\n                ");
  con->dd_printf("   IRQB2 %d", (cb&0x40)?1:0);
  con->dd_printf("   EnB2 %d", ((cb&0x28)==0x08)?1:0);
  con->dd_printf("   EdgB2 %c", (cb&0x10)?'R':'F');
  con->dd_printf("   CB2 %d", cb2());
  con->dd_printf("\n                ");
  con->dd_printf("   DirB2 %s", (ca&0x20)?"Out":"In ");
  con->dd_printf(" ModB2 ");
       if ((cb&0x38) == 0x30) con->dd_printf("Output 0                ");
  else if ((cb&0x38) == 0x38) con->dd_printf("Output 1                ");
  else if ((cb&0x38) == 0x20) con->dd_printf("Write strobe, A1 restore");
  else if ((cb&0x38) == 0x28) con->dd_printf("Write strobe,  E restore");
  else                        con->dd_printf("Input                   ");
  con->dd_printf("\n");

  //print_cfg_info(con);
}

/* End of motorola.src/pia.cc */
