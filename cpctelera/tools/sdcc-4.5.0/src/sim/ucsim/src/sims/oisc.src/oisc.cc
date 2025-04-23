/*
 * Simulator of microcontrollers (oisc.cc)
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

#include <ctype.h>

#include "utils.h"

#include "dregcl.h"

#include "glob.h"

#include "oisccl.h"


cl_op_pass::cl_op_pass(class cl_memory_cell *acell, class cl_oisc *auc):
  cl_memory_operator(acell)
{
  t_addr a;
  uc= auc;
  addr= 0;
  if (uc->rom->is_owned(acell, &a))
    addr= a;
}

t_mem
cl_op_pass::read(void)
{
  return uc->read(addr);
}

t_mem
cl_op_pass::write(t_mem val)
{
  return uc->write(addr, val);
}


/*
 * CPU
 */

cl_oisc::cl_oisc(class cl_sim *asim):
  cl_uc(asim)
{
}


int
cl_oisc::init(void)
{
  cl_uc::init();
  reg_cell_var(&cA, &rA, "A", "Accumulator");
  init_alu();
  return 0;
}


void
cl_oisc::make_cpu_hw(void)
{
  /*
  cpu= new cl_oisc_cpu(this);
  add_hw(cpu);
  cpu->init();
  */
}


void
cl_oisc::mk_hw_elements(void)
{
  cl_uc::mk_hw_elements();
  class cl_hw *h;

  h= new cl_dreg(this, 0, "dreg");
  h->init();
  add_hw(h);
}

void
cl_oisc::make_memories(void)
{
  class cl_address_space *as;
  class cl_address_decoder *ad;
  class cl_memory_chip *chip;
  
  rom= as= new cl_address_space("rom", 0, 0x10000, 16);
  as->init();
  address_spaces->add(as);

  chip= new cl_chip8("rom_chip", 0x10000, 16, 0);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= rom,
			     chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}


struct dis_entry *
cl_oisc::dis_tbl(void)
{
  return NULL;
}

char *
cl_oisc::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars(), fmt= chars();
  const char *b;
  int i;
  bool first;
  //u8_t h, l, r;
  u16_t src, dst;
  u16_t a;

  src= rom->read(addr);
  dst= rom->read(addr+1);
  
  b= "mov %d,%s";

  first= true;
  work= "";
  for (i=0; b[i]; i++)
    {
      if ((b[i] == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (b[i] == '\'')
	{
	  fmt= "";
	  i++;
	  while (b[i] && (b[i]!='\''))
	    fmt.append(b[i++]);
	  if (!b[i]) i--;
	  if (fmt.empty())
	    work.append("'");
	  /*
	  if (strcmp(fmt.c_str(), "i8") == 0)
	    {
	      work.appendf("#0x%02x", rom->read(addr+1));
	    }
	  */
	  /*
	    if (comment && temp.nempty())
	    comment->append(temp);
	  */
	  continue;
	}
      if (b[i] == '%')
	{
	  i++;
	  temp= "";
	  const char *n;
	  switch (b[i])
	    {
	    case 's':
	      n= dis_src(src);
	      if (n != NULL)
		work+= n;
	      else
		work.appendf("0x%04x", src);
	      break;
	    case 'd':
	      n= dis_dst(dst);
	      if (n != NULL)
		work+= n;
	      else
		work.appendf("0x%04x", dst);
	      break;
	    }
	  /*
	    if (comment && temp.nempty())
	    comment->append(temp);
	  */
	}
      else
	work+= b[i];
    }
  *comment= dis_comment(src, dst);
  
  return(strdup(work.c_str()));
}

const char *
cl_oisc::dis_src(t_addr addr)
{
  return NULL;
}

const char *
cl_oisc::dis_dst(t_addr addr)
{
  return NULL;
}

chars
cl_oisc::dis_comment(t_addr src, t_addr dst)
{
  chars s= "";
  return s;
}


int
cl_oisc::inst_length(t_addr addr)
{
  return 2;
}

void
cl_oisc::reset(void)
{
  cl_uc::reset();
}

int
cl_oisc::exec_inst(void)
{
  bool ret= do_brk();
  if (!ret)
    {
      u16_t src= rom->read(PC);
      u16_t dst= rom->read((PC+1) & 0xffff);
      u16_t tmp= rom->read(src);
      PC+= 2;
      PC&= 0xffff;
      rom->write(dst, tmp);
      tick(4);
    }
  return ret;
}


void
cl_oisc::print_acc(class cl_console_base *con)
{
  u16_t v= rom->read(rA);
  con->dd_color("answer");
  con->dd_printf("A= %04x %5u %+5d ", rA, rA, (i16_t)rA);
  con->dd_printf(" [A]= %04x %5u %+5d ", v, v, (i16_t)v);
  con->dd_printf("\n");
}


void
cl_oisc::print_regs(class cl_console_base *con)
{
  print_acc(con);
  print_disass(PC, con);
}


void
cl_oisc::init_alu(void)
{
}

u16_t
cl_oisc::read(u16_t addr)
{
  return rom->get(addr);
}

u16_t
cl_oisc::write(u16_t addr, u16_t val)
{
  return val;
}


/* End of oisc.src/oisc.cc */
