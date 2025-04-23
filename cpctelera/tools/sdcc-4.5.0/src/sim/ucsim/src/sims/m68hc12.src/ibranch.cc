/*
 * Simulator of microcontrollers (ibranch.cc)
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

#include "m68hc12cl.h"

int
CL12::call_e(void)
{
  u8_t h, l, p;
  u16_t a;
  h= fetch();
  l= fetch();
  p= fetch();
  a= h*256+l;

  h= PC>>8;
  l= PC&0xff;
  cSP.W(rSP-1);
  rom->write(rSP, l);
  cSP.W(rSP-1);
  rom->write(rSP, h);
  cSP.W(rSP-1);
  rom->write(rSP, cpu12->ppage_read());
  
  cpu12->ppage_write(p);
  PC= a;
  
  return resGO;
}

int
CL12::call_id(void)
{
  u8_t p, xb, h, l;
  xb= rom->read(PC);
  u16_t a= naddr(NULL, &p);
  if (!xb_indirect(xb))
    p= fetch();
  
  h= PC>>8;
  l= PC&0xff;
  cSP.W(rSP-1);
  rom->write(rSP, l);
  cSP.W(rSP-1);
  rom->write(rSP, h);
  cSP.W(rSP-1);
  rom->write(rSP, cpu12->ppage_read());
  
  cpu12->ppage_write(p);
  PC= a;
  
  return resGO;
}

int
CL12::brset(class cl_memory_cell &m)
{
  u8_t mask= fetch();
  i8_t r= fetch();
  if ((~(m.R()) & mask) == 0)
    {
      PC+= r;
      PC&= 0xffff;
    }
  return resGO;
}

int
CL12::brset_d(void)
{
  class cl_memory_cell &m= dir();
  return brset(m);  
}

int
CL12::brset_id(void)
{
  class cl_memory_cell &m= xb();
  return brset(m);
}

int
CL12::brset_e(void)
{
  class cl_memory_cell &m= ext();
  return brset(m);
}

int
CL12::brclr(class cl_memory_cell &m)
{
  u8_t mask= fetch();
  i8_t r= fetch();
  if (((m.R()) & mask) == 0)
    {
      PC+= r;
      PC&= 0xffff;
    }
  return resGO;
}

int
CL12::brclr_d(void)
{
  class cl_memory_cell &m= dir();
  return brclr(m);  
}

int
CL12::brclr_id(void)
{
  class cl_memory_cell &m= xb();
  return brclr(m);
}

int
CL12::brclr_e(void)
{
  class cl_memory_cell &m= ext();
  return brclr(m);
}

int
CL12::branch(t_addr a, bool cond)
{
  if (cond)
    {
      PC= a&0xffff;
      extra_ticks+= 2;
    }
  return resGO;
}

int
CL12::jump(t_addr a)
{
  PC= a&0xffff;
  return resGO;
}

int
CL12::bsr(void)
{
  i8_t r= fetch();
  u16_t s= rSP;
  rom->write(--s, PC);
  rom->write(--s, PC>>8);
  vc.wr+= 2;
  cSP.W(s);
  PC= (PC+r)&0xffff;
  return resGO;
}

int
CL12::jsr(t_addr a)
{
  u16_t s= rSP;
  rom->write(--s, PC);
  rom->write(--s, PC>>8);
  vc.wr+= 2;
  cSP.W(s);
  PC= (a)&0xffff;
  return resGO;
}

int
CL12::rtc(void)
{
  u16_t s= rSP;
  u8_t p, h, l;
  p= rom->read(s++);
  h= rom->read(s++);
  l= rom->read(s++);
  cSP.W(s);
  vc.rd+= 3;
  cpu12->ppage_write(p);
  PC= h*256+l;
  return resGO;
}

int
CL12::rts(void)
{
  u16_t s= rSP;
  u8_t h, l;
  h= rom->read(s++);
  l= rom->read(s++);
  cSP.W(s);
  vc.rd+= 2;
  PC= h*256+l;
  return resGO;
}

int
CL12::swi(void)
{
  u16_t s= rSP;
  rom->write(--s, PC);
  rom->write(--s, PC>>8);
  rom->write(--s, rIY);
  rom->write(--s, rIY>>8);
  rom->write(--s, rIX);
  rom->write(--s, rIX>>8);
  rom->write(--s, rA);
  rom->write(--s, rB);
  rom->write(--s, rF);
  cF.W(rF|flagI);
  vc.wr+= 9;
  cSP.W(s);
  //swi_src->request();
  u8_t h= rom->read(SWI_AT);
  u8_t l= rom->read(SWI_AT+1);
  PC= h*256+l;
  return resGO;
}

int
CL12::rti(void)
{
  u16_t s= rSP;
  u8_t f, fx, h, l;

  f= rom->read(s++);
  rF&= ~(flagStop|flagH|flagI|flagN|flagZ|flagV|flagC);
  fx= f&flagX;
  f&= ~flagX;
  rF&= fx;
  rF|= f;
  cF.W(f);
  
  h= rom->read(s++);
  l= rom->read(s++);
  cB.W(h);
  cA.W(l);
  h= rom->read(s++);
  l= rom->read(s++);
  cX.W(h*256+l);
  h= rom->read(s++);
  l= rom->read(s++);
  cY.W(h*256+l);
  h= rom->read(s++);
  l= rom->read(s++);
  PC= h*256+l;

  cSP.W(s);
  vc.rd+= 9;
  return resGO;
}

typedef bool (*cond_t)(u8_t);

#undef rF
bool cond_a (u8_t rF) { return ifA ; }
bool cond_n (u8_t rF) { return ifN ; }
bool cond_hi(u8_t rF) { return ifHI; }
bool cond_ls(u8_t rF) { return ifLS; }
bool cond_cc(u8_t rF) { return ifCC; }
bool cond_cs(u8_t rF) { return ifCS; }
bool cond_ne(u8_t rF) { return ifNE; }
bool cond_eq(u8_t rF) { return ifEQ; }
bool cond_vc(u8_t rF) { return ifVC; }
bool cond_vs(u8_t rF) { return ifVS; }
bool cond_pl(u8_t rF) { return ifPL; }
bool cond_mi(u8_t rF) { return ifMI; }
bool cond_ge(u8_t rF) { return ifGE; }
bool cond_lt(u8_t rF) { return ifLT; }
bool cond_gt(u8_t rF) { return ifGT; }
bool cond_le(u8_t rF) { return ifLE; }
#define rF  (CC.cc16.cc8.ccl)

cond_t cond_tab[16]= {
  cond_a,
  cond_n,
  cond_hi,
  cond_ls,
  cond_cc,
  cond_cs,
  cond_ne,
  cond_eq,
  cond_vc,
  cond_vs,
  cond_pl,
  cond_mi,
  cond_ge,
  cond_lt,
  cond_gt,
  cond_le
};
  
int
CL12::lbranch(u8_t code)
{
  bool cond= cond_tab[code&0xf](rF);
  i16_t r= fetch()*256;
  r+= fetch();
  if (cond)
    {
      extra_ticks= 2;
      u16_t n= PC;
      n+= r;
      PC= n;
    }
  return resGO;
}

int
CL12::loop(u8_t code)
{
  i16_t r= fetch();
  if ((code & 0xc0) == 0xc0)
    return resINV;
  
  class cl_memory_cell *c= loop_cells[code&0x7];
  if (c == NULL)
    return resINV;

  if (code & 0x80) c->W(c->R() + 1);
  if (code & 0x40) c->W(c->R() - 1);
  int brz= (code & 0x20)?0:1;
  int isz= (c->R() == 0)?1:0;
  if (brz == isz)
    {
      u16_t a= PC;
      if (code & 0x10)
	r|= 0xff00;
      a+= r;
      PC= a;
    }

  return resGO;
}


/* ENd of m68hc12.src/ibranch.cc */
