/*
 * Simulator of microcontrollers (imove.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include "f8cl.h"


int
cl_f8::ld8_a_i(u8_t op2)
{
  acc8->W(op2);
  return resGO;
}

int
cl_f8::ld8_a_m(class cl_cell8 &m)
{
  u8_t v = m.R();
  rF&= ~(flagN|flagZ);
  if (v & 0x80) rF|= flagN;
  if (!v) rF|= flagZ;
  acc8->W(v);
  vc.rd++;
  return resGO;
}

int
cl_f8::ld8_m_a(class cl_cell8 &m)
{
  m.W(acc8->get());
  vc.wr++;
  return resGO;
}

int
cl_f8::ld8_a_r(class cl_cell8 &r)
{
  class cl_cell8 *op1, *op2;
  IFSWAP
    {
      op2= acc8;
      op1= &r;
    }
  else
    {
      op1= acc8;
      op2= &r;
    }
  op1->W(op2->R());
  return resGO;
}

int
cl_f8::ldw_a_i(u16_t op2)
{
  acc16->W(op2);
  rF&= ~(flagN|flagZ);
  if (op2 & 0x8000) rF|= flagN;
  if (!op2) rF|= flagZ;
  return resGO;
}

int
cl_f8::ldw_a_m(u16_t addr)
{
  u16_t v= rom->read(addr);
  v+= (rom->read(addr+1))*256;
  rF&= ~(flagN|flagZ);
  if (v & 0x8000) rF|= flagN;
  if (!v) rF|= flagZ;
  acc16->W(v);
  vc.rd+= 2;
  return resGO;
}

int
cl_f8::ldw_m_a(u16_t addr)
{
  u16_t v= acc16->get();
  rom->write(addr, v);
  rom->write(addr+1, v>>8);
  vc.wr+= 2;
  return resGO;
}

int
cl_f8::ldw_m_r(u16_t addr, u16_t r)
{
  rom->write(addr, r);
  rom->write(addr+1, r>>8);
  vc.wr+= 2;
  return resGO;
}

int
cl_f8::ldw_r_m(u16_t addr)
{
  u16_t v= rom->read(addr);
  v+= (rom->read(addr+1))*256;
  rF&= ~(flagN|flagZ);
  if (v & 0x8000) rF|= flagN;
  if (!v) rF|= flagZ;
  rop16->W(v);
  vc.rd+= 2;
  return resGO;
}

int
cl_f8::ldw_a_r(u16_t r)
{
  acc16->W(r);
  return resGO;
}

int
cl_f8::LDW_A_SP(t_mem code)
{
  IFSWAP
    cSP.W(acc16->get());
  else
    acc16->W(rSP);
  return resGO;
}

int
cl_f8::XCH_F_NSP(t_mem code)
{
  class cl_cell8 &c= m_n_sp();
  u8_t t= rF;
  rF = c.R();
  vc.rd++;
  c.W(t);
  vc.wr++;
  return resGO;
}

int
cl_f8::LDW_DSP_A(t_mem code)
{
  i8_t d= fetch();
  u16_t pa= rSP+d;
  u16_t a= read_addr(rom, pa);
  vc.rd+= 2;
  u16_t v= acc16->get();
  rom->write(a++, v);
  rom->write(a, v>>8);
  vc.wr+= 2;
  return resGO;
}

int
cl_f8::LDI_Y_Z(t_mem code)
{
  u8_t v= rom->read(rZ++);
  rom->write(a_n_y(), v);
  rF&= ~(flagN|flagZ);
  if (v & 0x80) rF|= flagN;
  if (!v) rF|= flagZ;
  vc.rd++;
  vc.wr++;
  return resGO;
}

int
cl_f8::LDWI_Y_Z(t_mem code)
{
  u16_t addr = a_n_y();
  u16_t v= rom->read(rZ++);
  v+= (rom->read(rZ++))*256;
  rom->write(addr, v);
  rom->write(addr+1, v >> 8);
  rF&= ~(flagN|flagZ);
  if (v & 0x8000) rF|= flagN;
  if (!v) rF|= flagZ;
  vc.rd += 2;
  vc.wr += 2;
  return resGO;
}

int
cl_f8::PUSH_M(t_mem code)
{
  class cl_cell8 &c= m_mm();
  push1(c.R());
  vc.rd++;
  return resGO;
}

int
cl_f8::PUSH_NSP(t_mem code)
{
  class cl_cell8 &c= m_n_sp();
  push1(c.R());
  vc.rd++;
  return resGO;
}

int
cl_f8::PUSH_A(t_mem code)
{
  push1(acc8->get()); // TODO?
  return resGO;
}

int
cl_f8::PUSH_NY(t_mem code)
{
  class cl_cell8 &c= m_n_y();
  push1(c.R());
  vc.rd++;
  return resGO;
}

int
cl_f8::PUSH_I(t_mem code)
{
  push1(fetch());
  return resGO;
}

int
cl_f8::PUSHW_M(t_mem code)
{
  u16_t a= a_mm();
  u16_t v= read_addr(rom, a);
  vc.rd+= 2;
  push2(v);
  return resGO;
}

int
cl_f8::PUSHW_NSP(t_mem code)
{
  u16_t a= a_n_sp();
  u16_t v= read_addr(rom, a);
  vc.rd+= 2;
  push2(v);
  return resGO;
}

int
cl_f8::PUSHW_NNZ(t_mem code)
{
  u16_t a= a_nn_z();
  u16_t v= read_addr(rom, a);
  vc.rd+= 2;
  push2(v);
  return resGO;
}

int
cl_f8::PUSHW_A(t_mem code)
{
  u16_t v= acc16->get();
  push2(v);
  return resGO;
}

int
cl_f8::PUSHW_I(t_mem code)
{
  u8_t l, h;
  l= fetch();
  h= fetch();
  push2(h, l);
  return resGO;
}

int
cl_f8::POP_A(t_mem code)
{
  acc8->W(pop1());
  return resGO;
}

int
cl_f8::POPW_A(t_mem code)
{
  acc16->W(pop2());
  return resGO;
}

int
cl_f8::XCH_A_NSP(t_mem code)
{
  class cl_cell8 &c= m_n_sp();
  u8_t t= acc8->get();
  acc8->W(c.R());
  vc.rd++;
  c.W(t);
  vc.wr++;
  return resGO;
}

int
cl_f8::XCH_A_Y(t_mem code)
{
  class cl_cell8 &c= m_y();
  u8_t t= acc8->get();
  acc8->W(c.R());
  vc.rd++;
  c.W(t);
  vc.wr++;
  return resGO;
}

int
cl_f8::XCH_A_A(t_mem code)
{
  u16_t t= acc16->get();
  u8_t h, l;
  h= t>>8;
  l= t;
  acc16->W(l*256+h);
  return resGO;
}

int
cl_f8::XCHW_X_Y(t_mem code)
{
  u16_t t= read_addr(rom, acc16->get());
  vc.rd+= 2;
  rom->write(acc16->get()  , (rop16->get() >> 0) & 0xff);
  rom->write(acc16->get()+1, (rop16->get() >> 8) & 0xff);
  vc.wr+= 2;
  rop16->W(t);
  return resGO;
}

int
cl_f8::XCHW_Y_NSP(t_mem code)
{
  u16_t a= a_n_sp();
  u16_t t= read_addr(rom, a);
  vc.rd+= 2;
  rom->write(a  , (acc16->get() >> 0) & 0xff);
  rom->write(a+1, (acc16->get() >> 8) & 0xff);
  vc.wr+= 2;
  acc16->W(t);
  return resGO;
}

int
cl_f8::CAX(t_mem code)
{
  // if ((y) == xh) (z) = xl; else xh = (y);
  u8_t my= rom->read(rY);
  vc.rd++;
  if (my == rXH)
    {
      rom->write(rZ, rXL);
      vc.wr++;
    }
  else
    {
      cXH.W(my);
    }
  return 0;
}

int
cl_f8::CAXW(t_mem code)
{
  // if ((y) == z) (y) = x; else x = (y);
  u16_t my= read_addr(rom, rY);
  vc.rd+= 2;
  if (my == rZ)
    {
      rom->write(rY  , rX);
      rom->write(rY+1, rX>>8);
      vc.wr+= 2;
    }
  else
    {
      cX.W(my);
    }
  return 0;
}

int
cl_f8::CLR_M(t_mem code)
{
  class cl_cell8 &c= m_mm();
  c.W(0);
  vc.wr++;
  return resGO;
}

int
cl_f8::CLR_NSP(t_mem code)
{
  class cl_cell8 &c= m_n_sp();
  c.W(0);
  vc.wr++;
  return resGO;
}

int
cl_f8::CLR_A(t_mem code)
{
  acc8->W(0);
  return resGO;
}

int
cl_f8::CLR_NY(t_mem code)
{
  class cl_cell8 &c= m_n_y();
  c.W(0);
  vc.wr++;
  return resGO;
}

int
cl_f8::CLRW_M(t_mem code)
{
  u16_t a= a_mm();
  rom->write(a  , 0);
  rom->write(a+1, 0);
  vc.wr+= 2;
  return resGO;
}

int
cl_f8::CLRW_NSP(t_mem code)
{
  u16_t a= a_n_sp();
  rom->write(a  , 0);
  rom->write(a+1, 0);
  vc.wr+= 2;
  return resGO;
}

int
cl_f8::CLRW_NNZ(t_mem code)
{
  u16_t a= a_nn_z();
  rom->write(a  , 0);
  rom->write(a+1, 0);
  vc.wr+= 2;
  return resGO;
}

int
cl_f8::CLRW_A(t_mem code)
{
  acc16->write(0);
  return resGO;
}

int
cl_f8::xchb(int b)
{
  b&= 7;
  u8_t mask= 1<<b;
  class cl_cell8 &c= m_mm();
  u8_t t= c.R(), a= acc8->get();
  u8_t mbit= t&mask;
  vc.rd++;
  t&= ~mask;
  if (a & 1) t|= mask;
  acc8->W(mbit?1:0);
  c.write(t);
  rF&= ~flagZ;
  if (!mbit) rF|= flagZ;
  cF.W(rF);
  vc.wr++;
  return resGO;
}


/* End of f8.src/imove.cc */
