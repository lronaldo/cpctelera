/*
 * Simulator of microcontrollers (tlcs.src/inst_rot_sh.cc)
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

/* $Id: inst_rot_sh.cc 517 2016-11-22 19:12:14Z drdani $ */

#include "tlcscl.h"


// RLC 8-bit
u8_t
cl_tlcs::op_rlc(u8_t data, bool set_sz)
{
  u8_t c= data & 0x80;
  reg.raf.f&= ~((set_sz?(FLAG_S|FLAG_Z):0)|FLAG_H|FLAG_N|FLAG_C);
  data<<= 1;
  if (c)
    {
      data|= 1;
      reg.raf.f|= FLAG_C;
    }

  if (set_sz)
    {
      if (!data)
	reg.raf.f|= FLAG_Z;
      if (data&0x80)
	reg.raf.f|= FLAG_S;
      set_p(data);
    }
  
  return data;
}


// RLC mem
u8_t
cl_tlcs::inst_rlc(cl_memory_cell *cell)
{
  u8_t d= op_rlc(cell->read(), true);
  vc.rd++;
  cell->write(d);
  vc.wr++;
  return d;
}


// RRC 8-bit
u8_t
cl_tlcs::op_rrc(u8_t data, bool set_sz)
{
  u8_t c= data & 0x01;
  reg.raf.f&= ~((set_sz?(FLAG_S|FLAG_Z):0)|FLAG_H|FLAG_N|FLAG_C);
  data>>= 1;
  if (c)
    {
      data|= 0x80;
      reg.raf.f|= FLAG_C;
    }

  if (set_sz)
    {
      if (!data)
	reg.raf.f|= FLAG_Z;
      if (data&0x80)
	reg.raf.f|= FLAG_S;
      set_p(data);
    }
  
  return data;
}


// RRC mem
u8_t
cl_tlcs::inst_rrc(cl_memory_cell *cell)
{
  u8_t d= op_rrc(cell->read(), true);
  vc.rd++;
  cell->write(d);
  vc.wr++;
  return d;
}


// RL 8-bit
u8_t
cl_tlcs::op_rl(u8_t data, bool set_sz)
{
  u8_t c= data & 0x80;
  data<<= 1;
  if (reg.raf.f & FLAG_C)
    data|= 1;
  reg.raf.f&= ~((set_sz?(FLAG_S|FLAG_Z):0)|FLAG_H|FLAG_N|FLAG_C);
  if (c)
    reg.raf.f|= FLAG_C;

  if (set_sz)
    {
      if (!data)
	reg.raf.f|= FLAG_Z;
      if (data&0x80)
	reg.raf.f|= FLAG_S;
      set_p(data);
    }
  
  return data;
}


// RL mem
u8_t
cl_tlcs::inst_rl(cl_memory_cell *cell)
{
  u8_t d= op_rl(cell->read(), true);
  vc.rd++;
  cell->write(d);
  vc.wr++;
  return d;
}


// RR 8-bit
u8_t
cl_tlcs::op_rr(u8_t data, bool set_sz)
{
  u8_t c= data & 0x01;
  data>>= 1;
  if (reg.raf.f & FLAG_C)
    data|= 0x80;
  reg.raf.f&= ~((set_sz?(FLAG_S|FLAG_Z):0)|FLAG_H|FLAG_N|FLAG_C);
  if (c)
    reg.raf.f|= FLAG_C;

  if (set_sz)
    {
      if (!data)
	reg.raf.f|= FLAG_Z;
      if (data&0x80)
	reg.raf.f|= FLAG_S;
      set_p(data);
    }
  
  return data;
}


// RR mem
u8_t
cl_tlcs::inst_rr(cl_memory_cell *cell)
{
  u8_t d= op_rr(cell->read(), true);
  vc.rd++;
  cell->write(d);
  vc.wr++;
  return d;
}


// SLA 8-bit
u8_t
cl_tlcs::op_sla(u8_t data, bool set_sz)
{
  u8_t c= data & 0x80;
  data<<= 1;
  reg.raf.f&= ~((set_sz?(FLAG_S|FLAG_Z):0)|FLAG_H|FLAG_N|FLAG_C);
  if (c)
    reg.raf.f|= FLAG_C;

  if (set_sz)
    {
      if (!data)
	reg.raf.f|= FLAG_Z;
      if (data&0x80)
	reg.raf.f|= FLAG_S;
      set_p(data);
    }
  
  return data;
}


// SLA mem
u8_t
cl_tlcs::inst_sla(cl_memory_cell *cell)
{
  u8_t d= op_sla(cell->read(), true);
  vc.rd++;
  cell->write(d);
  vc.wr++;
  return d;
}


// SRA 8-bit
u8_t
cl_tlcs::op_sra(u8_t data, bool set_sz)
{
  u8_t c7= data & 0x80;
  u8_t c0= data & 0x01;
  data>>= 1;
  reg.raf.f&= ~((set_sz?(FLAG_S|FLAG_Z):0)|FLAG_H|FLAG_N|FLAG_C);
  if (c0)
    reg.raf.f|= FLAG_C;
  data|= c7;
  
  if (set_sz)
    {
      if (!data)
	reg.raf.f|= FLAG_Z;
      if (data&0x80)
	reg.raf.f|= FLAG_S;
      set_p(data);
    }
  
  return data;
}


// SRA mem
u8_t
cl_tlcs::inst_sra(cl_memory_cell *cell)
{
  u8_t d= op_sra(cell->read(), true);
  vc.rd++;
  cell->write(d);
  vc.wr++;
  return d;
}


// SRL 8-bit
u8_t
cl_tlcs::op_srl(u8_t data, bool set_sz)
{
  u8_t c0= data & 0x01;
  data>>= 1;
  reg.raf.f&= ~((set_sz?(FLAG_S|FLAG_Z):0)|FLAG_H|FLAG_N|FLAG_C);
  if (c0)
    reg.raf.f|= FLAG_C;
  
  if (set_sz)
    {
      if (!data)
	reg.raf.f|= FLAG_Z;
      if (data&0x80)
	reg.raf.f|= FLAG_S;
      set_p(data);
    }
  
  return data;
}


// SRL mem
u8_t
cl_tlcs::inst_srl(cl_memory_cell *cell)
{
  u8_t d= op_srl(cell->read(), true);
  vc.rd++;
  cell->write(d);
  vc.wr++;
  return d;
}


// RLD mem
int
cl_tlcs::inst_rld(class cl_memory_cell *cell)
{
  reg.raf.f&= ~(FLAG_H|FLAG_X|FLAG_N);

  u8_t c= cell->read();
  vc.rd++;
  u8_t temp= reg.raf.a & 0x0f;
  reg.raf.a= (reg.raf.a & 0xf0) + (c >> 4);
  cell->write((c << 4) + temp);
  vc.wr++;
  set_p(reg.raf.a);
  return resGO;
}


// RRD mem
int
cl_tlcs::inst_rrd(class cl_memory_cell *cell)
{
  reg.raf.f&= ~(FLAG_H|FLAG_X|FLAG_N);

  u8_t c= cell->read();
  vc.rd++;
  u8_t temp= reg.raf.a & 0x0f;
  reg.raf.a= (reg.raf.a & 0xf0) + (c & 0x0f);
  cell->write((temp << 4) + (c >> 4));
  vc.wr++;
  set_p(reg.raf.a);
  return resGO;
}


/* End of tlcs.src/inst_rot_sh.cc */
