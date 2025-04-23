/*
 * Simulator of microcontrollers (misc16.cc)
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

#include "uartcl.h"

#include "misc16cl.h"


/*

  MISC16 CPU design by:
  - Juergen Pintaske
  - Harry Siebert
  - Bernd Paysan

  Toolset is by Steve Teal, at https://github.com/Steve-Teal/eforth-misc16
  
+---------+-------------+------------------+
| Address | Source      | Destination      |
+---------+-------------+------------------+
| 0x0000  | PC          | PC               |   
| 0x0001  | PC+2        | PC if A < 0      |
| 0x0002  | PC+4        | PC if A = 0      |
| 0x0003  | PC+6        | -                |
| 0x0004  | -           | PC if C = 1      |
| 0x0005  | -           | -                |
| 0x0006  | -           | -                |
| 0x0007  | [A]         | [A]              |
| 0x0008  | A           | A                |
| 0x0009  | -           | A = A - source   |
| 0x000A  | -           | -                |
| 0x000B  | -           | A = A + source   |
| 0x000C  | -           | A = A xor source |
| 0x000D  | -           | A = A or source  |
| 0x000E  | -           | A = A and source |
| 0x000F  | -           | A = source >> 1  | 
+---------+-------------+------------------+
*/

cl_misc16::cl_misc16(class cl_sim *asim):
  cl_oisc(asim)
{
}

void
cl_misc16::mk_hw_elements(void)
{
  class cl_hw *h;
  cl_oisc::mk_hw_elements();

  add_hw(h= new cl_uart(this, 0, 0xfffb));
  h->init();
}

void
cl_misc16::reset(void)
{
  cl_oisc::reset();
  PC= 0x10;
}

u16_t
cl_misc16::sub(u16_t a, u16_t b)
{
  u16_t s;

  b= ~b;
  s= a+b+1;
  rC= (((a&b)|((a|b)&~s))&0x8000)?0:1;
  return s;
}

u16_t
cl_misc16::add(u16_t a, u16_t b)
{
  u16_t s;
  s= a+b;
  rC= (((a&b)|((a|b)&~s))&0x8000)?1:0;
  return s;
}

u16_t
cl_misc16::shift(u16_t a)
{
  u16_t s;
  s= a>>1;
  if (rC) s|= 0x8000;
  rC= a&1;
  return s;
}  

void
cl_misc16::init_alu(void)
{
  id_str= "MISC16";
  reg_cell_var(&cC, &rC, "C", "Carry flag");
  cC.append_operator(new cl_cy_op(&cC));
  class cl_memory_cell *c;
  c= rom->get_cell(0); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(1); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(2); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(3); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(4); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(7); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(8); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(9); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(11); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(12); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(13); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(14); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(15); c->append_operator(new cl_op_pass(c, this));
}

void
cl_misc16::print_regs(class cl_console_base *con)
{
  print_acc(con);
  con->dd_printf("CY= %d", rC);
  con->dd_printf("\n");
  print_disass(PC, con);
}


const char *
cl_misc16::dis_src(t_addr addr)
{
  switch (addr)
    {
    case 0: return "PC";
    case 1: return "PC+2";
    case 2: return "PC+4";
    case 3: return "PC+8";
    case 7: return "[A]";
    case 8: return "A";
    }
  return NULL;
}

const char *
cl_misc16::dis_dst(t_addr addr)
{
  switch (addr)
    {
    case 0: return "PC";
    case 1: return "PC(A<0)";
    case 2: return "PC(A==0)";
    case 4: return "PC(C)";
    case 7: return "[A]";
    case 8: return "A";
    case 9: return "A-";
    case 11: return "A+";
    case 12: return "A^";
    case 13: return "A|";
    case 14: return "A&";
    case 15: return ">>";
    }
  return NULL;
}

chars
cl_misc16::dis_comment(t_addr src, t_addr dst)
{
  chars s= "";
  if (dst==7)
    s.appendf("; dst=0x%04x", rA);
  if ((dst==0) || (dst==1) || (dst==2) || (dst==4) || (dst==7) ||
      (dst==9) || (dst==11) || (dst==12) || (dst==13) || (dst==14) || (dst==15))
    {
      u16_t sa= src;
      if (sa == 7)
	sa= rA;
      if (s.nempty()) s+= ", ";
      else s+= "; ";
      s.appendf("[0x%04x]=0x%04x", sa, rom->read(sa));
    }
  else if (src==7)
    {
      if (s.nempty()) s+= ", ";
      else s+= "; ";
      s.appendf("[src=0x%04x]=0x%04x", rA, rom->read(rA));
    }
  else
    {
      if (s.nempty()) s+= ", ";
      else s+= "; ";
      s.appendf("[src=0x%04x]=0x%04x", src, rom->read(src));
    }
  return s;
}


u16_t
cl_misc16::read(u16_t addr)
{
  switch (addr)
    {
    case 0: return PC;
    case 1: return (PC+2)&0xffff;
    case 2: return (PC+4)&0xffff;
    case 3: return (PC+6)&0xffff;
    case 7: return (rA==7)?rA:(rom->read(rA));
    case 8: return rA;
    }
  return rom->get(addr);
}

u16_t
cl_misc16::write(u16_t addr, u16_t val)
{
  switch (addr)
    {
    case 0: PC= val; break;
    case 1: if ((i16_t)rA < 0) PC= val; break;
    case 2: if (!rA) PC= val; break;
    case 4: if (rC) PC= val; break;
    case 7: if (rA != 7) rom->write(rA, val); break;
    case 8: cA.W(val); break;
    case 9: cA.W(val= sub(rA, val)); break;
    case 11: cA.W(val= add(rA, val)); break;
    case 12: cA.W(val= rA^val); break;
    case 13: cA.W(val= rA|val); break;
    case 14: cA.W(val= rA&val); break;
    case 15: cA.W(val= shift(val)); break;
    }
  return val;
}


/*
int
cl_misc16::exec_inst(void)
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
*/


/* End of oisc.src/misc16.cc */
