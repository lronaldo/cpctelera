/*
 * Simulator of microcontrollers (em.cc)
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

#include "emcl.h"

/*
  Extended MISC16
  
+---------+-------------+------------------+
| Address | Source      | Destination      |
+---------+-------------+------------------+
| 0x0000  | PC          | PC               |   
| 0x0001  | PC+2        | PC if A < 0      |
| 0x0002  | PC+4        | PC if A = 0      |
| 0x0003  | PC+6        | -                |
| 0x0004  | -           | PC if C = 1      |
| 0x0005  | -         P | -              P |
| 0x0006  | -     [P++] | -          [--P] |
| 0x0007  | [A]         | [A]              |
| 0x0008  | A           | A                |
| 0x0009  | -           | A = A - source   |
| 0x000A  | -     [A++] | -          [A++] |
| 0x000B  | -           | A = A + source   |
| 0x000C  | -           | A = A xor source |
| 0x000D  | -           | A = A or source  |
| 0x000E  | -           | A = A and source |
| 0x000F  | -           | A = source >> 1  | 
+---------+-------------+------------------+
 */

cl_em::cl_em(class cl_sim *asim):
  cl_misc16(asim)
{
}

void
cl_em::print_regs(class cl_console_base *con)
{
  u16_t v= rom->read(rP);
  print_acc(con);
  con->dd_printf("P= %04x %5u %+5d ", rP, rP, rP);
  con->dd_printf(" [P]= %04x %5u %+5d\n", v, v, v);
  con->dd_printf("CY= %d", rC);
  con->dd_printf("\n");
  print_disass(PC, con);
}


const char *
cl_em::dis_src(t_addr addr)
{
  switch (addr)
    {
    case 5: return "P";
    case 6: return "[P++]";
    case 10: return "[A++]";
    }
  return cl_misc16::dis_src(addr);
}

const char *
cl_em::dis_dst(t_addr addr)
{
  switch (addr)
    {
    case 5: return "P";
    case 6: return "[--P]";
    case 10: return "[A++]";
    }
  return cl_misc16::dis_dst(addr);
}

chars
cl_em::dis_comment(t_addr src, t_addr dst)
{
  chars s= "";
  if (src==6)
    {
      s.appendf("; [src=0x%04x++]=0x%04x", rP, rom->get(rP));
    }
  if (dst==6)
    {
      if (s.empty()) s+= "; ";
      if (s.nempty()) s+= ", ";
      s.appendf("[dst=--0x%04x]=0x%04x", rP, rom->get(rP-1));
    }
  if (src==10)
    {
      if (s.empty()) s+= "; ";
      if (s.nempty()) s+= ", ";
      s.appendf("[src=0x%04x++]=0x%04x", rA, rom->get(rA));
    }
  if (dst==10)
    {
      if (s.empty()) s+= "; ";
      if (s.nempty()) s+= ", ";
      s.appendf("[dst=0x%04x++]=0x%04x", rA, rom->get(rA));
    }
  return cl_misc16::dis_comment(src, dst);
}


void
cl_em::init_alu(void)
{
  cl_misc16::init_alu();
  id_str= "Extended-MISC16";
  reg_cell_var(&cP, &rP, "P", "Pointer register");
  class cl_memory_cell *c;
  c= rom->get_cell(5); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(6); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(10); c->append_operator(new cl_op_pass(c, this));
}

u16_t
cl_em::read(u16_t addr)
{
  switch (addr)
    {
    case 5: return rP;
    case 6: return rom->read(rP++);
    case 10: return rom->read(rA++);
    }
  return cl_misc16::read(addr);
}

u16_t
cl_em::write(u16_t addr, u16_t val)
{
  switch (addr)
    {
    case 5: cP.W(val); return val;
    case 6: rom->write(--rP, val); return val;
    case 10: rom->write(rA++, val); return val;
    }
  return cl_misc16::write(addr, val);
}


/* End of oisc.src/em.cc */
