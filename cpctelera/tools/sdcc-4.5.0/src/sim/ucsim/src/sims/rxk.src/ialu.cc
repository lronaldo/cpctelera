/*
 * Simulator of microcontrollers (ialu.cc)
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

#include "rxkcl.h"
#include "r3kacl.h"
#include "r4kcl.h"


/*
 *                                                   Increment, decrement
 */

int
cl_rxk::inc_ss(class cl_cell16 &rp, u16_t op)
{
  rp.W(op+1);
  tick(1);
  return resGO;
}

int
cl_rxk::inc_r(class cl_cell8 &cr, u8_t op)
{
  class cl_cell8 &rf= destF();
  u8_t r, f= rF & ~(flagS|flagZ|flagV), na7, r7;
  na7= (op&0x80)^0x80;
  cr.W(r= op+1);
  r7= r&0x80;
  if (r & 0x80) f|= flagS;
  if (!r) f|= flagZ;
  if (na7 & r7) f|= flagV;
  rf.W(f);
  tick(1);
  return resGO;
}

int
cl_rxk::INC_iIRd(t_mem code)
{
  i8_t d= fetch();
  class cl_cell8 &dest= dest8iIRd(d);
  u8_t op= dest.read();
  vc.wr++;
  vc.rd++;
  tick5p1(10);
  return inc_r(dest, op);
}

int
cl_rxk::dec_ss(class cl_cell16 &rp, u16_t op)
{
  rp.W(op-1);
  tick(1);
  return resGO;
}

int
cl_rxk::dec_r(class cl_cell8 &cr, u8_t op)
{
  class cl_cell8 &rf= destF();
  u8_t r, f= rF & ~(flagS|flagZ|flagV), a7, nr7;
  a7= op&0x80;
  cr.W(r= op-1);
  nr7= (r&0x80)^0x80;
  if (r & 0x80) f|= flagS;
  if (!r) f|= flagZ;
  if (a7 & nr7) f|= flagV;
  rf.W(f);
  tick(1);
  return resGO;
}


int
cl_rxk::DEC_iIRd(t_mem code)
{
  i8_t d= fetch();
  class cl_cell8 &dest= dest8iIRd(d);
  u8_t op= dest.read();
  vc.wr++;
  vc.rd++;
  tick5p1(10);
  return dec_r(dest, op);
}

int
cl_rxk::inc_i8(t_addr addr)
{
  class cl_cell8 &f= destF();
  u8_t forg= f.R() & ~(flagS|flagZ|flagV);
  class cl_memory_cell *dest= rwas->get_cell(addr);
  u8_t org= dest->R();
  u8_t res= org+1;
  if (res & 0x80) forg|= flagS;
  if (!res) forg|= flagZ;
  if (!(org&0x80) && (res&0x80)) forg|= flagV;
  f.W(forg);
  dest->W(res);
  vc.rd++;
  vc.wr++;
  tick(7);
  return resGO;
}

int
cl_rxk::INC_IR(t_mem code)
{
  u16_t v= cIR->get();
  cIR->write(v+1);
  tick(3);
  return resGO;
}

int
cl_rxk::DEC_IR(t_mem code)
{
  u16_t v= cIR->get();
  cIR->write(v-1);
  tick(3);
  return resGO;
}

int
cl_rxk::dec_i8(t_addr addr)
{
  class cl_cell8 &f= destF();
  u8_t forg= f.R() & ~(flagS|flagZ|flagV);
  class cl_memory_cell *dest= rwas->get_cell(addr);
  u8_t org= dest->R();
  u8_t res= org-1;
  if (res & 0x80) forg|= flagS;
  if (!res) forg|= flagZ;
  if (org&res&0x80) forg|= flagV;
  f.W(forg);
  dest->W(res);
  vc.rd++;
  vc.wr++;
  tick(7);
  return resGO;
}

int
cl_rxk::add_ir_xy(u16_t op)
{
  u16_t op1= cIR->get();
  u32_t res= op1 + op;
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagC;
  if (res > 0xffff) forg|= flagC;
  cIR->write(res);
  f.W(forg);
  tick(3);
  return resGO;
}


/*
 *                                                             Rotate, shift
 */

/*
     C <-- 7..<-...0 <--+
        |               |
        +---------------+
 */
int
cl_rxk::rot8left(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t a7;
  a7= op&0x80;
  dest.W((op<<1) | (a7?1:0));
  f.W((rF & ~flagC) | (a7?flagC:0));
  tick(1);
  return resGO;
}

int
cl_rxk::rlc(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t a7, res, forg= rF & ~flagAll;
  a7= op&0x80;
  dest.W(res= ((op<<1) | (a7?1:0)));
  if (!res) forg|= flagZ;
  if (res&0x80) forg|= flagS;
  if (res&0xf0) forg|= flagL;
  if (a7) forg|= flagC;
  f.W(forg);
  tick(3);
  return resGO;
}


/*
     C <-- 15..<-...0 <--+
         |               |
         +---------------+
 */
int
cl_rxk::rot16left(class cl_cell16 &dest, u16_t op)
{
  u32_t cf;
  cf= op & 0x8000;
  op<<= 1;
  if (cf) op|= 1;
  
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  if (op & 0x8000) forg|= flagS;
  if (!op) forg|= flagZ;
  if (op & 0xf000) forg|= flagL;
  if (cf) forg|= flagC;
  f.W(forg);
  dest.W(op);
  tick(1);
  return resGO;
}


/*
     C <-- 31..<-...0 <--+
         |               |
         +---------------+
 */
int
cl_rxk::rot32left(class cl_cell32 &dest, u32_t op, int nr)
{
  u32_t cf= 0;
  while (nr)
    {
      cf= op & 0x80000000;
      op<<= 1;
      if (cf) op|= 1;
      nr--;
    }
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  if (op & 0x80000000) forg|= flagS;
  if (!op) forg|= flagZ;
  if (op & 0xf0000000) forg|= flagL;
  if (cf) forg|= flagC;
  f.W(forg);
  dest.W(op);
  tick(3);
  return resGO;
}


/*
     C <-- 7..<-...0 <--+
     |                  |
     +------------------+
 */
int
cl_rxk::rot9left(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t a7, c= rF&flagC;
  a7= op&0x80;
  dest.W((op<<1) | (c?1:0));
  f.W((rF & ~flagC) | (a7?flagC:0));
  tick(1);
  return resGO;
}

int
cl_rxk::rl(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t a7, c= rF&flagC, res, forg;
  a7= op&0x80;
  dest.W(res= ((op<<1) | (c?1:0)));
  forg= rF & ~flagAll;
  if (!res) forg|= flagZ;
  if (res&0x80) forg|= flagS;
  if (res&0xf0) forg|= flagL;
  if (a7) forg|= flagC;
  f.W(forg);
  tick(3);
  return resGO;
}


/*
     C <-- 15..<-...0 <--+
     |                   |
     +-------------------+
 */
int
cl_rxk::rot17left(class cl_cell16 &dest, u16_t op)
{
  class cl_cell8 &f= destF();
  u8_t forg;
  u16_t a15, c= rF & flagC, res;
  forg= rF & ~flagAll;
  a15= op&0x8000;
  dest.W(res= (op<<1) | (c?1:0));
  if (a15) forg|= flagC;
  if (res & 0x8000) forg|= flagS;
  if (!res) forg|= flagZ;
  if (res & 0xf000) forg|= flagL;
  f.W(forg);
  tick(1);
  return resGO;
}

/*
     C <-- 31..<-...0 <--+
     |                   |
     +-------------------+
 */
int
cl_rxk::rot33left(class cl_cell32 &dest, u32_t op, int nr)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF;
  bool cf= forg & flagC;;
  while (nr)
    {
      bool msb= op & 0x80000000;
      op<<= 1;
      if (cf) op|= 1;
      cf= msb;
      nr--;
    }
  forg&= ~flagAll;
  if (cf) forg|= flagC;
  if (!op) forg|= flagZ;
  if (op & 0x80000000) forg|= flagS;
  if (op & 0xf0000000) forg|= flagL;
  f.W(forg);
  dest.W(op);
  tick(3);
  return resGO;
}

/*   7..->...0 --> C
     |         |
     +---------+
 */
int
cl_rxk::rot8right(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t a0;
  a0= op&0x01;
  dest.W((op>>1) | (a0?0x80:0));
  f.W((rF & ~flagC) | (a0?flagC:0));
  tick(1);
  return resGO;
}


/*   15...->.....0 --> C
     |           |
     +-----------+
 */
int
cl_rxk::rot16right(class cl_cell16 &dest, u16_t op)
{
  class cl_cell8 &f= destF();
  u8_t forg;
  bool cf= false;
  cf= op & 1;
  op>>= 1;
  if (cf) op|= 0x8000;
    
  forg= rF & ~flagAll;
  if (cf) forg|= flagC;
  if (!op) forg|= flagZ;
  if (op & 0x8000) forg|= flagS;
  if (op & 0xf000) forg|= flagL;
  f.W(forg);
  dest.W(op);
  tick(1);
  return resGO;
}


/*   31.....->......0 --> C
     |              |
     +--------------+
 */
int
cl_rxk::rot32right(class cl_cell32 &dest, u32_t op, int nr)
{
  class cl_cell8 &f= destF();
  u8_t forg;
  bool cf= false;
  while (nr)
    {
      cf= op & 1;
      op>>= 1;
      if (cf) op|= 0x80000000;
      nr--;
    }
  forg= rF & ~flagAll;
  if (cf) forg|= flagC;
  if (!op) forg|= flagZ;
  if (op & 0x80000000) forg|= flagS;
  if (op & 0xf0000000) forg|= flagL;
  f.W(forg);
  dest.W(op);
  tick(3);
  return resGO;
}

int
cl_rxk::rrc(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t a0, res, forg= rF & ~flagAll;
  a0= op&0x01;
  dest.W(res= ((op>>1) | (a0?0x80:0)));
  if (!res) forg|= flagZ;
  if (res&0x80) forg|= flagS;
  if (res&0xf0) forg|= flagL;
  if (a0) forg|= flagC;
  f.W(forg);
  tick(3);
  return resGO;
}


/*
     7..->...0 --> C
     |             |
     +-------------+
 */
int
cl_rxk::rot9right(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t a0, c= rF&flagC;
  a0= op&0x01;
  dest.W((op>>1) | (c?0x80:0));
  f.W((rF & ~flagC) | (a0?flagC:0));
  tick(1);
  return resGO;
}

int
cl_rxk::rr(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t a0, c= rF&flagC, res, forg;
  a0= op&0x01;
  dest.W(res= ((op>>1) | (c?0x80:0)));
  forg= rF & ~flagAll;
  if (!res) forg|= flagZ;
  if (res&0x80) forg|= flagS;
  if (res&0xf0) forg|= flagL;
  if (a0) forg|= flagC;
  f.W(forg);
  tick(3);
  return resGO;
}


/*
     C --> 15..->...0 -->+
     |                   |
     +<------------------+
 */
int
cl_rxk::rot17right(class cl_cell16 &dest, u16_t op)
{
  class cl_cell8 &f= destF();
  u8_t forg;
  u16_t a0, c= rF & flagC, res;
  forg= rF & ~flagAll;
  a0= op&1;
  dest.W(res= (c?0x8000:0) | (op>>1));
  if (a0) forg|= flagC;
  if (res & 0x8000) forg|= flagS;
  if (!res) forg|= flagZ;
  if (res & 0xf000) forg|= flagL;
  f.W(forg);
  tick(1);
  return resGO;
}


/*
     C --> 31..->...0 -->+
     |                   |
     +<------------------+
 */
int
cl_rxk::rot33right(class cl_cell32 &dest, u32_t op, int nr)
{
  class cl_cell8 &f= destF();
  u8_t forg;
  u32_t cf= rF & flagC;
  while (nr)
    {
      op>>= 1;
      if (cf) op|= 0x80000000;
      cf= op & 1;
      nr--;
    }
  forg= rF & ~flagAll;
  if (cf) forg|= flagC;
  if (!op) forg|= flagZ;
  if (op & 0x80000000) forg|= flagS;
  if (op & 0xf0000000) forg|= flagL;
  f.W(forg);
  dest.W(op);
  tick(3);
  return resGO;
}


/*
   C <--  7..<-..0  <-- 0
*/
int
cl_rxk::sla8(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t res, forg= rF & ~flagAll;
  if (op & 0x80) forg|= flagC;
  dest.W(res= (op<<1));
  if (res & 0x80) forg|= flagS;
  if (!res) forg|= flagZ;
  if (res & 0xf0) forg|= flagL;
  f.W(forg);
  tick(3);
  return resGO;
}

/*
   C <--  31..<-..0  <-- 0
*/
int
cl_rxk::sla32(class cl_cell32 &dest, u32_t op, int nr)
{
  class cl_cell8 &f= destF();
  u32_t forg= rF & ~flagAll;
  u32_t cf= 0;
  while (nr)
    {
      cf= op & 0x80000000;
      op<<= 1;
      nr--;
    }
  if (cf) forg|= flagC;
  if (op & 0x80000000) forg|= flagS;
  if (!op) forg|= flagZ;
  if (op & 0xf0000000) forg|= flagL;
  f.W(forg);
  tick(3);
  return resGO;
}


/*
  +--> 7..->..0  --> C
  |    |
  +----+  
*/
int
cl_rxk::sra8(class cl_cell8 &dest, i8_t op)
{
  class cl_cell8 &f= destF();
  i8_t res, forg= rF & ~flagAll;
  if (op & 1) forg|= flagC;
  dest.W(res= (op>>1));
  if (res & 0x8000) forg|= flagS;
  if (!res) forg|= flagZ;
  if (res & 0xf000) forg|= flagL;
  f.W(forg);
  tick(3);
  return resGO;
}

/*
  +--> 31......->......0  --> C
  |     |
  +-----+  
*/
int
cl_rxk::sra32(class cl_cell32 &dest, i32_t op, int nr)
{
  class cl_cell8 &f= destF();
  i8_t forg= rF & ~flagAll;
  bool cf= false;
  while (nr)
    {
      cf= op & 1;
      op>>= 1;
      nr--;
    }
  if (op & 0x80000000) forg|= flagS;
  if (!op) forg|= flagZ;
  if (op & 0xf0000000) forg|= flagL;
  if (cf) forg|= flagC;
  f.W(forg);
  dest.W(op);
  tick(3);
  return resGO;
}

/*
  0-->  7..->..0  --> C
*/
int
cl_rxk::srl8(class cl_cell8 &dest, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t res, forg= rF & ~flagAll;
  if (op & 1) forg|= flagC;
  dest.W(res= (op>>1));
  if (res & 0x8000) forg|= flagS;
  if (!res) forg|= flagZ;
  if (res & 0xf000) forg|= flagL;
  f.W(forg);
  tick(3);
  return resGO;
}

/*
  0-->  31.....->.....0  --> C
*/
int
cl_rxk::srl32(class cl_cell32 &dest, u32_t op, int nr)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  bool cf= false;
  while (nr)
    {
      cf= op & 1;
      op>>= 1;
      nr--;
    }
  if (op & 0x80000000) forg|= flagS;
  if (!op) forg|= flagZ;
  if (op & 0xf0000000) forg|= flagL;
  if (cf) forg|= flagC;
  f.W(forg);
  dest.W(op);
  tick(3);
  return resGO;
}

int
cl_rxk::bit_r(u8_t b, u8_t op)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagZ, res;
  res= (1<<b) & op;
  if (!res) forg|= flagZ;
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::bit_iHL(u8_t b)
{
  u8_t forg= rF & ~flagZ, res, op;
  op= rwas->read(rHL);
  vc.rd++;
  res= (1<<b) & op;
  if (!res) forg|= flagZ;
  cF.W(forg);
  tick(6);
  return resGO;
}

int
cl_rxk::bit_iIRd(u8_t b, i8_t d)
{
  u8_t forg= rF & ~flagZ, res, op;
  op= rwas->read(cIR->get()+d);
  vc.rd++;
  res= (1<<b) & op;
  if (!res) forg|= flagZ;
  cF.W(forg);
  tick5p1(9);
  return resGO;
}

int
cl_rxk::res_r(u8_t b, class cl_cell8 &dest, u8_t op)
{
  u8_t res;
  res= ~(1<<b) & op;
  dest.W(res);
  tick(3);
  return resGO;
}

int
cl_rxk::res_iHL(u8_t b)
{
  u8_t res, op;
  op= rwas->read(rHL);
  vc.rd++;
  res= ~(1<<b) & op;
  rwas->write(rHL, res);
  vc.wr++;
  tick(9);
  return resGO;
}

int
cl_rxk::res_iIRd(u8_t b, i8_t d)
{
  u8_t res, op;
  t_addr a= cIR->get()+d;
  op= rwas->read(a);
  vc.rd++;
  res= ~(1<<b) & op;
  rwas->write(a, res);
  vc.wr++;
  tick5p1(12);
  return resGO;
}


int
cl_rxk::set_r(u8_t b, class cl_cell8 &dest, u8_t op)
{
  u8_t res;
  res= (1<<b) | op;
  dest.W(res);
  tick(3);
  return resGO;
}

int
cl_rxk::set_iHL(u8_t b)
{
  u8_t res, op;
  op= rwas->read(rHL);
  vc.rd++;
  res= (1<<b) | op;
  rwas->write(rHL, res);
  vc.wr++;
  tick(9);
  return resGO;
}

int
cl_rxk::set_iIRd(u8_t b, i8_t d)
{
  u8_t res, op;
  t_addr a= cIR->get()+d;
  op= rwas->read(a);
  vc.rd++;
  res= (1<<b) | op;
  rwas->write(a, res);
  vc.wr++;
  tick5m1(11);
  return resGO;
}


/*
 *                                                           Bitwise, logical
 */

int
cl_rxk::BOOL_HL(t_mem code)
{
  class cl_cell16 &dhl= destHL();
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  if (rHL)
    dhl.W(1);
  else
    forg|= flagZ;
  f.W(forg);
  tick(1);
  return resGO;
}

int
cl_rxk::xor8(class cl_cell8 &dest, u8_t op1, u8_t op2)
{
  class cl_cell8 &f= destF();
  u8_t forg= f.R() & ~flagAll;
  u8_t res= op1 ^ op2;
  dest.W(res);
  if (res & 0x80) forg|= flagS;
  if (!res) forg|= flagZ;
  if (res&0xf0) forg|= flagL;
  f.W(forg);
  tick(1);
  return resGO;
}

int
cl_rxk::xor16(class cl_cell16 &dest, u16_t op1, u16_t op2)
{
  class cl_cell8 &f= destF();
  u8_t forg= f.R() & ~flagAll;
  u16_t res= op1 ^ op2;
  dest.W(res);
  if (res & 0x8000) forg|= flagS;
  if (!res) forg|= flagZ;
  if (res&0xf000) forg|= flagL;
  f.W(forg);
  tick5m2(3);
  return resGO;
}

int
cl_rxk::or8(class cl_cell8 &dest, u8_t op1, u8_t op2)
{
  class cl_cell8 &f= destF();
  u8_t forg= f.R() & ~flagAll;
  u8_t res= op1 | op2;
  dest.W(res);
  if (res & 0x80) forg|= flagS;
  if (!res) forg|= flagZ;
  if (res&0xf0) forg|= flagL;
  f.W(forg);
  tick(1);
  return resGO;
}

int
cl_rxk::or16(class cl_cell16 &dest, u16_t op1, u16_t op2)
{
  class cl_cell8 &f= destF();
  //class cl_cell16 &dhl= destHL();
  u16_t res= op1/*rHL*/ | op2/*rDE*/;
  u8_t forg= rF & ~flagAll;
  dest.W(res);
  if (res & 0x8000)
    forg|= flagS;
  if (!res)
    forg|= flagZ;
  // flagL?
  if (res & 0xf000)
    forg|= flagL;
  f.W(forg);
  tick(1);
  return resGO;
}

int
cl_rxk::and8(class cl_cell8 &dest, u8_t op1, u8_t op2)
{
  class cl_cell8 &f= destF();
  u8_t res= op1 & op2, forg= rF & ~flagAll;
  if (!res) forg|= flagZ;
  if (res&0x80) forg|= flagS;
  if (res&0xf0) forg|= flagL;
  dest.W(res);
  f.W(forg);
  tick(1);
  return resGO;
}

int
cl_rxk::and16(class cl_cell16 &dest, u16_t op1, u16_t op2)
{
  class cl_cell8 &f= destF();
  //class cl_cell16 &dhl= destHL();
  u16_t res= op1/*rHL*/ & op2/*rDE*/;
  u8_t forg= rF & ~flagAll;
  dest.W(res);
  if (res & 0x8000)
    forg|= flagS;
  if (!res)
    forg|= flagZ;
  // flagL?
  if (res & 0xf000)
    forg|= flagL;
  f.W(forg);
  tick(1);
  return resGO;
}

int
cl_rxk::XOR_A_iIRd(t_mem code)
{
  i8_t d= fetch();
  class cl_cell8 &a= destA(), &f= destF();
  u8_t forg= rF & ~flagAll, res= rA ^ rwas->read(cIR->get() + d);
  vc.rd++;
  if (!res) forg|= flagZ;
  if (res & 0x80) forg|= flagS;
  if (res & 0xf0) forg|= flagL;
  a.W(res);
  f.W(forg);
  tick5p1(8);
  return resGO;
}

int
cl_rxk::OR_A_iIRd(t_mem code)
{
  i8_t d= fetch();
  class cl_cell8 &a= destA(), &f= destF();
  u8_t forg= rF & ~flagAll, res= rA | rwas->read(cIR->get() + d);
  vc.rd++;
  if (!res) forg|= flagZ;
  if (res & 0x80) forg|= flagS;
  if (res & 0xf0) forg|= flagL;
  a.W(res);
  f.W(forg);
  tick5p1(8);
  return resGO;
}

int
cl_rxk::AND_A_iIRd(t_mem code)
{
  i8_t d= fetch();
  class cl_cell8 &a= destA(), &f= destF();
  u8_t forg= rF & ~flagAll, res= rA & rwas->read(cIR->get() + d);
  vc.rd++;
  if (!res) forg|= flagZ;
  if (res & 0x80) forg|= flagS;
  if (res & 0xf0) forg|= flagL;
  a.W(res);
  f.W(forg);
  tick5p1(8);
  return resGO;
}

int
cl_rxk::BOOL_IR(t_mem code)
{
  u16_t v= cIR->get();
  if (v)
    cIR->W(v=1);
  u8_t f= rF & ~flagAll;
  //if (v&0x8000) f|= flagS;
  if (!v) f|= flagZ;
  cF.W(f);
  tick(3);
  return resGO;
}

int
cl_rxk::AND_IR_DE(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  u16_t v;
  cIR->W(v= cIR->get() & rDE);
  if (!v) forg|= flagZ;
  if (v&0x8000) forg|= flagS;
  if (v^0xf000) forg|= flagL;
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::OR_IR_DE(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  u16_t v;
  cIR->W(v= cIR->get() | rDE);
  if (!v) forg|= flagZ;
  if (v&0x8000) forg|= flagS;
  if (v^0xf000) forg|= flagL;
  f.W(forg);
  tick(3);
  return resGO;
}


/*
 *                                                                Arithmetic
 */

int
cl_rxk::CPL(t_mem code)
{
  destA().W(~rA);
  tick(1);
  return resGO;
}

int
cl_rxk::add_hl_ss(u16_t op)
{
  class cl_cell16 &hl= destHL();
  class cl_cell8 &f= destF();
  u8_t forg= rF;
  u32_t res= rHL + op;
  hl.W(res);
  if (res > 0xffff)
    f.W(forg|= flagC);
  else
    f.W(forg&= ~flagC);
  tick(1);
  return resGO;
}

int
cl_rxk::adc_hl_ss(u16_t op)
{
  class cl_cell16 &hl= destHL();
  class cl_cell8 &f= destF();
  u32_t res= rHL + op + ((rF&flagC)?1:0);
  u8_t forg= rF & ~flagAll;
  u16_t c1= 0, c2;
  if (res > 0xffff)
    {
      forg|= flagC;
      c1= 0x8000;
    }
  if (!res) forg|= flagZ;
  if (res & 0x8000) forg|= flagS;
  res= (rHL&0x7fff)+(op&0x7fff)+((rF&flagC)?1:0);
  c2= res&0x8000;
  if (c1^c2) forg|= flagV;
  f.W(forg);
  hl.W(res);
  tick(3);
  return resGO;
}

int
cl_rxk::add8(u8_t op2, bool cy)
{
  class cl_cell8 &a= destA(), &f= destF();
  u8_t v1= rA, forg;
  u16_t res= v1+op2+(cy?((rF&flagC)?1:0):0);
  u8_t a7, b7, r7, na7, nb7, nr7;
  forg= rF & ~flagAll;
  a7= v1&0x80; na7= a7^0x80;
  b7= op2&0x80; nb7= b7^0x80;
  r7= res&0x80; nr7= r7^0x80;
  if ((a7&b7&nr7) | (na7&nb7&r7)) forg|= flagV;
  if (res > 0xff) forg|= flagC;
  if (!(res & 0xff)) forg|= flagZ;
  if (res & 0x80) forg|= flagS;
  a.W(res);
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::sub8(u8_t op2, bool cy)
{
  class cl_cell8 &a= destA(), &f= destF();
  u8_t v1= rA, forg;
  u16_t res= v1+(~op2)+(cy?((rF&flagC)?1:0):1);
  u8_t a7, b7, r7, na7, nb7, nr7;
  i8_t op1= rA;
  i8_t o2= op2;
  i8_t r= op1-o2;
  if (cy && (rF&flagC)) r--;
  res= r;
  forg= rF & ~(flagS|flagZ|flagV);
  a7= v1&0x80; na7= a7^0x80;
  b7= op2&0x80; nb7= b7^0x80;
  r7= res&0x80; nr7= r7^0x80;
  if ((a7&nb7&nr7) | (na7&b7&r7)) forg|= flagV;
  //if (res > 0xff) forg|= flagC;
  if (rA<op2) forg|= flagC;
  if ((rA>op2) || (!cy && (rA==op2))) forg&= ~flagC;
  if (!(res & 0xff)) forg|= flagZ;
  if (res & 0x80) forg|= flagS;
  a.W(res);
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::NEG(t_mem code)
{
  u8_t org= destA().get();
  destA().set(0);
  sub8(org, false);
  tick5p9(0);
  return resGO;
}

int
cl_rxk::sub16(u16_t op2, bool cy)
{
  class cl_cell16 &hl= destHL();
  class cl_cell8 &f= destF();
  u16_t v1= rHL;
  u8_t forg;
  u32_t res;//= //v1+(~op2)+(cy?((rF&flagC)?1:0):1);
  u16_t a15, b15, r15, na15, nb15, nr15;
  i16_t op1= rHL;
  i16_t o2= op2;
  i16_t r= op1-o2;
  if (cy && (rF&flagC)) r--;
  res= r;
  forg= rF & ~(flagS|flagZ|flagV);
  a15= v1&0x8000; na15= a15^0x8000;
  b15= op2&0x8000; nb15= b15^0x8000;
  r15= res&0x8000; nr15= r15^0x8000;
  if ((a15&nb15&nr15) | (na15&b15&r15)) forg|= flagV;
  //if (res > 0xffff) forg|= flagC;
  if (rHL<op2) forg|= flagC;
  if ((rHL>op2) || (!cy && (rA==op2))) forg&= ~flagC;
  if (!(res & 0xffff)) forg|= flagZ;
  if (res & 0x8000) forg|= flagS;
  hl.W(res);
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::sub32(u32_t op1, u32_t op2, class cl_cell32 &cRes, bool cy)
{
  class cl_cell8 &f= destF();
  u32_t v1= op1;
  u8_t forg;
  u32_t res;
  u32_t a31, b31, r31, na31, nb31, nr31;
  i32_t o2= op2;
  i32_t r= op1-o2;
  if (cy && (rF&flagC)) r--;
  res= r;
  forg= rF & ~(flagZ|flagS|flagV);
  a31=  v1&0x80000000; na31= a31^0x80000000;
  b31= op2&0x80000000; nb31= b31^0x80000000;
  r31= res&0x80000000; nr31= r31^0x80000000;
  if ((a31&nb31&nr31) | (na31&b31&r31)) forg|= flagV;
  if (op1<op2) forg|= flagC;
  if ((op1>op2) || (!cy && (op1==op2))) forg&= ~flagC;
  if (!res) forg|= flagZ;
  if (res & 0x80000000) forg|= flagS;
  cRes.W(res);
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::cp8(u8_t op1, u8_t op2)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  u8_t res= op1-op2;
  if (op1<op2) forg|= (flagS|flagC);
  if (op1==op2) forg|= flagZ;
  if ( 0x80 & ( (~res&op1&~op2) | (res&~op1&op2) ) ) forg|= flagV;
  f.W(forg);
  tick5m2(1);
  return resGO;
}

int
cl_rxk::cp16(u16_t op1, u16_t op2)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  u16_t res= op1-op2;
  if (op1<op2) forg|= (flagS|flagC);
  if (op1==op2) forg|= flagZ;
  if ( 0x8000 & ( (~res&op1&~op2) | (res&~op1&op2) ) ) forg|= flagV;
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::cp32(u32_t op1, u32_t op2)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  u32_t res= op1-op2;
  if (op1<op2) forg|= (flagS|flagC);
  if (op1==op2) forg|= flagZ;
  if ( 0x80000000 & ( (~res&op1&~op2) | (res&~op1&op2) ) ) forg|= flagV;
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::ADD_SP_d(t_mem code)
{
  i8_t d= fetch();
  u32_t res= rSP+d;
  class cl_cell8 &f= destF();
  u8_t forg= f.R();
  cSP.W(res);
  if (res > 0xffff)
    f.W(forg|= flagC);
  else
    f.W(forg&= ~flagC);
  tick(3);
  return resGO;
}

int
cl_rxk::MUL(t_mem code)
{
  i16_t op1= rBC, op2= rDE;
  i32_t res= op1*op2;
  cHL.W((res>>16)&0xffff);
  cBC.W(res);
  tick(11);
  return resGO;
}


/*
 *                                                    R3000A,R4000,R5000
 */

int
cl_r3ka::UMx(bool add)
{
  u32_t v1, v2;

  tick(7);
  do
  {
    v1= rom->read(rIX);
    vc.rd++;
    v2= rom->read(rIY);
    vc.rd++;
    v2= v2*rDE;
    v2+= raDE;
    if (rF & flagC) v2++;
    if (add)
      v1+= v2;
    else
      v1-= v2;

    rom->write(rHL, v1);
    vc.wr++;
    caDE.W(v1>>8);
    u8_t f= rF & ~flagC;
    if (v1>>24) f|= flagC;
    cF.W(f);

    cBC.W(rBC-1);
    cIX.W(rIX+1);
    cIY.W(rIY+1);
    cHL.W(rHL+1);
    tick(8);
  }
  while (rBC);
  return resGO;
}

int
cl_r4k::subhl(class cl_cell16 &dest, u16_t op)
{
  i32_t v, o;
  u8_t forg= rF & ~flagC;
  v= rHL;
  o= op;
  v= v - o;
  if (v & 0xffff0000) forg|= flagC;
  destF().W(forg);
  dest.W(v);
  tick(1);
  return resGO;
}

int
cl_r4k::test8(u8_t op)
{
  u8_t forg= rF & ~flagAll;
  if (!op) forg|= flagZ;
  if (op & 0x80) forg|= flagS;
  if (op & 0xf0) forg|= flagL;
  destF().W(forg);
  tick(1);
  return resGO;
}

int
cl_r4k::test16(u16_t op)
{
  u8_t forg= rF & ~flagAll;
  if (!op) forg|= flagZ;
  if (op & 0x8000) forg|= flagS;
  if (op & 0xf000) forg|= flagL;
  destF().W(forg);
  tick(1);
  return resGO;
}

int
cl_r4k::test32(u32_t op)
{
  u8_t forg= rF & ~flagAll;
  if (!op) forg|= flagZ;
  if (op & 0x80000000) forg|= flagS;
  if (op & 0xf0000000) forg|= flagL;
  destF().W(forg);
  tick(1);
  return resGO;
}

int
cl_r4k::flag_cc_hl(t_mem code)
{
  bool cond= false;
  if ((code & 0x60) == 0x40)
    {
      switch ((code >> 3) & 3)
	{
	case 0: cond= !(rF & flagZ); break;
	case 1: cond= rF & flagZ; break;
	case 2: cond= !(rF & flagC); break;
	case 3: cond= rF & flagC; break;
	}
    }
  else if ((code & 0x60) == 0x20)
    {
      switch ((code >> 3) & 3)
	{
	case 0: cond= cond_GT(rF); break;
	case 1: cond= cond_LT(rF); break;
	case 2: cond= cond_GTU(rF); break;
	case 3: cond= rF & flagV; break;
	}
    }
  cHL.W(cond?1:0);
  tick(3);
  return resGO;
}

int
cl_r4k::RLC_8_IRR(t_mem code)
{
  u32_t op= cIRR->get();
  u32_t msb= op >> 24;
  op<<= 8;
  op|= msb;
  cIRR->write(op);
  tick(3);
  return resGO;
}

int
cl_r4k::RLB_A_IRR(t_mem code)
{
  u32_t op= cIRR->get();
  u32_t msb= op >> 24;
  op<<= 8;
  op|= rA;
  cA.W(msb);
  cIRR->write(op);
  tick(3);
  return resGO;
}

int
cl_r4k::RRC_8_IRR(t_mem code)
{
  u32_t op= cIRR->get();
  u32_t lsb= op & 0xff;
  op>>= 8;
  op|= lsb<<24;
  cIRR->write(op);
  tick(3);
  return resGO;
}

int
cl_r4k::RRB_A_IRR(t_mem code)
{
  u32_t op= cIRR->get();
  u32_t lsb= op & 0xff;
  op>>= 8;
  op|= (u32_t)rA << 24;
  cA.W(lsb);
  cIRR->write(op);
  tick(3);
  return resGO;
}

int
cl_r4k::CP_HL_D(t_mem code)
{
  i8_t d= fetch();
  i16_t d16;
  d16= d;
  return cp16(rHL, d16);
}

int
cl_r4k::MULU(t_mem code)
{
  u32_t v= rBC * rDE;
  cBC.W(v);
  cHL.W(v>>16);
  tick(11);
  return resGO;
}

int
cl_r4k::ADD_JKHL_BCDE(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagC;
  u64_t v= rJKHL + rBCDE;
  destJKHL().W(v);
  if (v > 0xffffffff)
    forg|= flagC;
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_r4k::SUB_JKHL_BCDE(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagC;
  u32_t v= rJKHL - rBCDE;
  if (rJKHL < rBCDE)
    forg|= flagC;
  destJKHL().W(v);
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_r4k::AND_JKHL_BCDE(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  u32_t v= rJKHL & rBCDE;
  if (v & 0x80000000) forg|= flagS;
  if (v & 0xf0000000) forg|= flagL;
  if (!v) forg|= flagZ;
  destJKHL().W(v);
  f.W(forg);
  tick(3);
  return resGO;
}
  
int
cl_r4k::OR_JKHL_BCDE(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  u32_t v= rJKHL | rBCDE;
  if (v & 0x80000000) forg|= flagS;
  if (v & 0xf0000000) forg|= flagL;
  if (!v) forg|= flagZ;
  destJKHL().W(v);
  f.W(forg);
  tick(3);
  return resGO;
}
  
int
cl_r4k::XOR_JKHL_BCDE(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t forg= rF & ~flagAll;
  u32_t v= rJKHL ^ rBCDE;
  if (v & 0x80000000) forg|= flagS;
  if (v & 0xf0000000) forg|= flagL;
  if (!v) forg|= flagZ;
  destJKHL().W(v);
  f.W(forg);
  tick(3);
  return resGO;
}
  

/* End of rxk.src/ialu.cc */
