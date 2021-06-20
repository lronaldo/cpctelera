/*
 * Simulator of microcontrollers (tlcs.src/inst_cpu_others.cc)
 *
 * Copyright (C) 2016,16 Drotos Daniel, Talker Bt.
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

/* $Id: inst_cpu_others.cc 517 2016-11-22 19:12:14Z drdani $ */

#include "tlcscl.h"


// 0B
int
cl_tlcs::inst_daa_a()
{
  if (((reg.raf.a & 0x0f) > 9) ||
      (reg.raf.f & FLAG_H))
    {
      int al= reg.raf.a & 0x0f;
      if (al + 6 > 15)
	reg.raf.f|= FLAG_H;
      else
	reg.raf.f&= ~FLAG_H;
      if ((int)reg.raf.a + 6 > 255)
	reg.raf.f|= FLAG_C|FLAG_X;
      else
	reg.raf.f&= ~(FLAG_C|FLAG_X);
      reg.raf.a+= 6;
    }
  if (((reg.raf.a & 0xf0) > 0x90) ||
      (reg.raf.f & FLAG_C))
    {
      if ((int)reg.raf.a + 0x60 > 255)
	reg.raf.f|= FLAG_C;
      else
	reg.raf.f&= ~FLAG_C;
      reg.raf.a+= 0x60;
    }
  return resGO;
}


// 10
int
cl_tlcs::inst_cpl_a()
{
  reg.raf.a= ~reg.raf.a;
  reg.raf.f|= FLAG_H|FLAG_N;
  return resGO;
}


// 11
int
cl_tlcs::inst_neg_a()
{
  reg.raf.f&= ~(FLAG_S|FLAG_Z|FLAG_H/*|FLAG_X*/|FLAG_V|FLAG_C);
  reg.raf.f|= FLAG_N;
  if (reg.raf.a == 0x80)
    reg.raf.f|= FLAG_V;
  if (reg.raf.a)
    reg.raf.f|= (FLAG_C|FLAG_X);

  //uint8_t a= ~reg.raf.a;
  if ((reg.raf.a & 0x0f) == 0)//if (a&0xf + 1 > 15)
    reg.raf.f|= FLAG_H;
  reg.raf.a= 0-reg.raf.a;

  if (reg.raf.a & 0x80)
    reg.raf.f|= FLAG_S;
  if (!reg.raf.a)
    reg.raf.f|= FLAG_Z;
  
  return resGO;
}


// 0e
int
cl_tlcs::inst_ccf()
{
  if (reg.raf.f & FLAG_C)
    reg.raf.f&= ~(FLAG_C);
  else
    reg.raf.f|= FLAG_C;
  if (reg.raf.f & FLAG_X)
    reg.raf.f&= ~(FLAG_X);
  else
    reg.raf.f|= FLAG_X;
  reg.raf.f&= ~FLAG_N;
  return resGO;
}


// 0d
int
cl_tlcs::inst_scf()
{
  reg.raf.f|= FLAG_C|FLAG_X;
  reg.raf.f&= ~(FLAG_N|FLAG_H);
  return resGO;
}


// 0c
int
cl_tlcs::inst_rcf()
{
  reg.raf.f&= ~(FLAG_C|FLAG_X|FLAG_N|FLAG_H);
  return resGO;
}


// ff
int
cl_tlcs::inst_swi()
{
  t_addr iPC= PC-1;
  reg.raf.f&= ~FLAG_I;
  exec_intr(iPC, 0x0010, PC);
  exec_push(iPC, reg.af);
  PC= 0x0010;
  return resGO;
}


// MUL HL,mem
int
cl_tlcs::inst_mul_hl(class cl_memory_cell *cell)
{
  reg.hl= reg.rhl.l * cell->read();
  vc.rd++;
  return resGO;
}


// DIV HL,mem
int
cl_tlcs::inst_div_hl(class cl_memory_cell *cell)
{
  u8_t m= cell->read();
  vc.rd++;
  return inst_div_hl(m);
}


// DIV HL,val
int
cl_tlcs::inst_div_hl(u8_t d)
{
  u8_t m= d;
  reg.raf.f&= ~FLAG_V;
  if ((m == 0) ||
      ((reg.hl / m) > 255))
    reg.raf.f|= FLAG_V;
  else
    reg.rhl.l= reg.hl / m;
  reg.rhl.h= reg.hl % m;
  return resGO;
}


/* End of tlcs.src/inst_cpu_others.cc */
