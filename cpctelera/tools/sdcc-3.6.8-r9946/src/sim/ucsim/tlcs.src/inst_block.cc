/*
 * Simulator of microcontrollers (tlcs.src/inst_block.cc)
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

/* $Id: inst_block.cc 517 2016-11-22 19:12:14Z drdani $ */

#include "tlcscl.h"


// 08
int
cl_tlcs::ex_de_hl()
{
  u16_t temp= reg.de;
  reg.de= reg.hl;
  reg.hl= temp;
  return resGO;
}


// 09
int
cl_tlcs::ex_af_alt_af()
{
  u16_t temp= reg.af;
  reg.af= reg.alt_af;
  reg.alt_af= temp;
  return resGO;
}


// 0A
int
cl_tlcs::exx()
{
  u16_t temp= reg.bc;
  reg.bc= reg.alt_bc;
  reg.alt_bc= temp;
  temp= reg.de;
  reg.de= reg.alt_de;
  reg.alt_de= temp;
  temp= reg.hl;
  reg.hl= reg.alt_hl;
  reg.alt_hl= temp;
  return resGO;
}


// FE 58
int
cl_tlcs::ldi()
{
  nas->write(reg.de, nas->read(reg.hl));
  vc.rd++;
  vc.wr++;
  reg.de++;
  reg.hl++;
  reg.bc--;
  reg.raf.f&= ~(FLAG_H|FLAG_N|FLAG_V);
  if (reg.bc)
    reg.raf.f|= FLAG_V;
  return resGO;
}


// FE 59
int
cl_tlcs::ldir()
{
  ldi();
  if (reg.bc)
    PC-= 2;
  return resGO;
}


// FE 5a
int
cl_tlcs::ldd()
{
  nas->write(reg.de, nas->read(reg.hl));
  vc.rd++;
  vc.wr++;
  reg.de--;
  reg.hl--;
  reg.bc--;
  reg.raf.f&= ~(FLAG_H|FLAG_N|FLAG_V);
  if (reg.bc)
    reg.raf.f|= FLAG_V;
  return resGO;
}


// FE 5a
int
cl_tlcs::lddr()
{
  ldd();
  if (reg.bc)
    PC-= 2;
  return resGO;
}


// FE 5C
int
cl_tlcs::cpi()
{
  reg.raf.f&= ~(FLAG_Z|FLAG_S|FLAG_N|FLAG_H|FLAG_X|FLAG_V);
  reg.raf.f|= FLAG_N;
  int a= reg.raf.a;
  int d= nas->read(reg.hl);
  vc.rd++;
  int r= a-d;
  
  reg.hl++;
  reg.bc--;

  if (r == 0)
    reg.raf.f|= FLAG_Z;
  if (reg.bc != 0)
    reg.raf.f|= FLAG_V;
  if (a == d)
    reg.raf.f|= FLAG_N;
  if (r & 0x80)
    reg.raf.f|= FLAG_S;
  if (((a&0xf) + ((~d+1)&0xf)) > 0xf)
    reg.raf.f|= FLAG_H;
  if ((unsigned int)r > 0xff)
    reg.raf.f|= FLAG_X;

  return resGO;
}


// FE 5D
int
cl_tlcs::cpir()
{
  cpi();
  if ((reg.bc != 0) &&
      ((reg.raf.f&FLAG_Z) == 0))
    PC-= 2;
  return resGO;
}


// FE 5E
int
cl_tlcs::cpd()
{
  reg.raf.f&= ~(FLAG_Z|FLAG_S|FLAG_N|FLAG_H|FLAG_X|FLAG_V);
  reg.raf.f|= FLAG_N;
  int a= reg.raf.a;
  int d= nas->read(reg.hl);
  int r= a-d;
  vc.rd++;
  
  reg.hl--;
  reg.bc--;

  if (r == 0)
    reg.raf.f|= FLAG_Z;
  if (reg.bc != 0)
    reg.raf.f|= FLAG_V;
  if (a == d)
    reg.raf.f|= FLAG_N;
  if (r & 0x80)
    reg.raf.f|= FLAG_S;
  if (((a&0xf) + ((~d+1)&0xf)) > 0xf)
    reg.raf.f|= FLAG_H;
  if ((unsigned int)r > 0xff)
    reg.raf.f|= FLAG_X;

  return resGO;
}


// FE 5D
int
cl_tlcs::cpdr()
{
  cpd();
  if ((reg.bc != 0) &&
      ((reg.raf.f&FLAG_Z) == 0))
    PC-= 2;
  return resGO;
}


/* End of tlcs.src/inst_block.cc */
