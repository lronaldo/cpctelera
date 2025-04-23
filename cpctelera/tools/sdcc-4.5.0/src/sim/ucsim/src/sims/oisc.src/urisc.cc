/*
 * Simulator of microcontrollers (urisc.cc)
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

#include "urisccl.h"


/*
  Simulation of CPU designed by Douglas W. Jones
  Published in: ACM Computer Architecture News, 16, 3 (June 1988), pages 48-55.
  https://doi.org/10.1145/48675.48683
  
  Revised design documented at: https://homepage.cs.uiowa.edu/~jones/arch/risc/

  
        address |      write      |     read
      ----------+-----------------+--------------
          FFF0  |  acc = data     |  data = acc
          FFF1  |  acc = acc-data |  data = N
      ----------+-----------------+--------------
          FFF2  |  acc = data-acc |  data = Z
          FFF3  |  acc = data+acc |  data = O
          FFF4  |  acc = data^acc |  data = C
          FFF5  |  acc = data&acc |  data = N^O
          FFF6  |  acc = data|acc |  data = (N^O)|Z
          FFF7  |  acc = data>>1  |  data = C^not(Z)
	  FFFF  |  PC             |  PC
*/

cl_urisc::cl_urisc(class cl_sim *asim):
  cl_oisc(asim)
{
}

u16_t
cl_urisc::add(u16_t a, u16_t b, u16_t c)
{
  u32_t sf, ss;
  c= c?1:0;
  sf= a+b+c;
  ss= (a&0x7fff)+(b+0x7fff)+c;
  u8_t c1, c2;
  c1= (sf>0xffff)?1:0;
  c2= (ss>0x7fff)?1:0;
  rF= 0;
  if (c1) rF|= flagC;
  if (c1^c2) rF|= flagO;
  if ((sf&0xffff) == 0) rF|= flagZ;
  if (sf&0x8000) rF|= flagS;
  return sf;
}

void
cl_urisc::init_alu(void)
{
  id_str= "Ultimate_RISC";
  reg_cell_var(&cF, &rF, "F", "Flags");
}

void
cl_urisc::print_regs(class cl_console_base *con)
{
  print_acc(con);
  con->dd_printf("   ONCZ\n");
  con->dd_printf("F= ");
  con->print_bin(rF, 4);
  con->dd_printf(" 0x%02x", rF);
  con->dd_printf("\n");
  print_disass(PC, con);
  class cl_memory_cell *c;
  c= rom->get_cell(0xfff0); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(0xfff1); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(0xfff2); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(0xfff3); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(0xfff4); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(0xfff5); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(0xfff6); c->append_operator(new cl_op_pass(c, this));
  c= rom->get_cell(0xfff7); c->append_operator(new cl_op_pass(c, this));
}


const char *
cl_urisc::dis_src(t_addr addr)
{
  switch (addr)
    {
    case 0xfff0: return "acc";
    case 0xfff1: return "N";
    case 0xfff2: return "Z";
    case 0xfff3: return "O";
    case 0xfff4: return "C";
    case 0xfff5: return "N^O";
    case 0xfff6: return "(N^O)|Z";
    case 0xfff7: return "C^~Z";
    case 0xffff: return "PC";
    }
  return NULL;
}

const char *
cl_urisc::dis_dst(t_addr addr)
{
  switch (addr)
    {
    case 0xfff0: return "acc";
    case 0xfff1: return "acc-";
    case 0xfff2: return "-acc";
    case 0xfff3: return "acc+";
    case 0xfff4: return "acc^";
    case 0xfff5: return "acc&";
    case 0xfff6: return "acc|";
    case 0xfff7: return ">>";
    case 0xffff: return "PC";
    }
  return NULL;
}

chars
cl_urisc::dis_comment(t_addr src, t_addr dst)
{
  chars s= "";
  if ((dst >= 0xfff0) && (dst <= 0xfff7) &&
      ((src < 0xfff0) || (src > 0xfff7)) &&
      (src != 0xffff))
    s.appendf("; 0x%04x", rom->read(src));
  return s;
}


u16_t
cl_urisc::read(u16_t addr)
{
  u8_t c, n, o, z, nz;
  switch (addr)
    {
    case 0xfff0: return rA;
    case 0xfff1: return (rF&flagN)?2:0;
    case 0xfff2: return (rF&flagZ)?2:0;
    case 0xfff3: return (rF&flagO)?2:0;
    case 0xfff4: return (rF&flagC)?2:0;
    case 0xfff5: n=(rF&flagN)?2:0; o=(rF&flagO)?2:0; return n^o;
    case 0xfff6:
      n= (rF&flagN)?2:0;
      o= (rF&flagO)?2:0;
      z= (rF&flagZ)?2:0;
      return (n^o)|z;
    case 0xfff7:
      c= (rF&flagC)?2:0;
      nz= (rF&flagZ)?0:2;
      return c^nz;
    case 0xffff:
      return PC;
    }
  return rom->get(addr);
}

u16_t
cl_urisc::write(u16_t addr, u16_t val)
{
  switch (addr)
    {
    case 0xfff0: cA.W(val); break;
    case 0xfff1: cA.W(add(rA, ~val, 1)); break;
    case 0xfff2: cA.W(add(val, ~rA, 1)); break;
    case 0xfff3: cA.W(add(rA, val, 0)); break;
    case 0xfff4: cA.W(rA^val); break;
    case 0xfff5: cA.W(rA&val); break;
    case 0xfff6: cA.W(rA|val); break;
    case 0xfff7: cA.W(val>>1); break;
    case 0xffff: PC= val; break;
    }
  return val;
}


/* End of oisc.src/urisc.cc */
