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

#include "m68hc12cl.h"


int
CL12::sub16(class cl_cell16 &dest, u16_t op)
{
  u8_t f= rF & ~(flagN|flagZ|flagV|flagC);
  u16_t a= dest.R(), b= op, r;
  r= a-b;
  if (r&0x8000) f|= flagN;
  if (!r) f|= flagZ;
  if (( (~a&b)|(b&r)|(r&~a) ) & 0x8000) f|= flagC;
  if (( (a&~b&~r)|(~a&b&r) ) & 0x8000) f|= flagV;
  dest.W(r);
  cF.W(f);
  return resGO;
}

int
CL12::add16(class cl_cell16 &dest, u16_t op)
{
  u8_t f= rF & ~(flagN|flagZ|flagV|flagC);
  u16_t a= dest.R(), b= op, r;
  r= a+b;
  if (r&0x8000) f|= flagN;
  if (!r) f|= flagZ;
  if (( (a&b)|(b&~r)|(~r&a) ) & 0x8000) f|= flagC;
  if (( (a&b&~r)|(~a&~b&r) ) & 0x8000) f|= flagV;
  dest.W(r);
  cF.W(f);
  return resGO;
}

int
CL12::cp16(u16_t op1, u16_t op2)
{
  u8_t f= rF & ~(flagN|flagZ|flagV|flagC);
  u16_t a= op1, b= op2, r;
  r= a-b;
  if (r&0x8000) f|= flagN;
  if (!r) f|= flagZ;
  if (( (~a&b)|(b&r)|(r&~a) ) & 0x8000) f|= flagC;
  if (( (a&~b&~r)|(~a&b&r) ) & 0x8000) f|= flagV;
  cF.W(f);
  return resGO;
}

int
CL12::lsr16(class cl_memory_cell &dest)
{
  u16_t op= dest.R(), f= rF & ~(flagN|flagZ|flagV|flagC);
  if (op&1) f|= flagC|flagV;
  op>>= 1;
  dest.W(op);
  if (!op) f|= flagZ;
  cCC.W(f);
  return resGO;
}

int
CL12::asl16(class cl_memory_cell &dest)
{
  u16_t op= dest.R();
  u8_t f;
  f= rF & ~(flagN|flagZ|flagV|flagC);
  if (op&0x8000) f|= flagC;
  op<<= 1;
  dest.W(op);
  if (!op) f|= flagZ;
  if (op&0x8000) f|= flagN;
  if (((f&flagN)?1:0) ^ ((f&flagC)?1:0)) f|= flagV;
  cCC.W(f);
  return resGO;
}

int
CL12::inxy(class cl_memory_cell &dest)
{
  u8_t f= rF & ~flagZ;
  u16_t v;
  dest.W(v= dest.R()+1);
  if (!v)
    f|= flagZ;
  cF.W(f);
  return resGO;
}

int
CL12::dexy(class cl_memory_cell &dest)
{
  u8_t f= rF & ~flagZ, v;
  dest.W(v= dest.R()-1);
  if (!v)
    f|= flagZ;
  cF.W(f);
  return resGO;
}

int
CL12::ediv(void)
{
  u32_t op1= (((u32_t)rY)<<16)+rD;
  u16_t op2= rX;
  u32_t res, rem;
  u8_t f= rF & ~(flagN|flagZ|flagV|flagC);
  if (op2 == 0)
    f|= flagC;
  else
    {
      res= op1/op2;
      rem= op1%op2;
      if (res & 0x8000)
	f|= flagN;
      if (res > 0xffff)
	f|= flagV;
      res&= 0xffff;
      if (!res)
	f|= flagZ;
      cY.W(res);
      cD.W(rem);
    }
  cF.W(f);
  return resGO;
}

int
CL12::mul(void)
{
  u8_t f= rF&~flagC;
  rD= rA*rB;
  if (rB & 0x80)
    f|= flagC;
  cD.W(rD);
  cF.W(f);
  return resGO;
}

int
CL12::emul(void)
{
  u8_t f= rF&~(flagC|flagZ|flagN);
  u32_t res= rD*rY;
  if (res & 0x8000)
    f|= flagC;
  if (res & 0x80000000)
    f|= flagN;
  if (!res)
    f|= flagZ;
  cD.W(res);
  cY.W(res>>16);
  cF.W(f);
  return resGO;
}

int
CL12::daa(void)
{
  u8_t ah= rA>>4;
  u8_t al= rA&0xf;
  int C= rF&flagC;
  int H= rF&flagH;
  u8_t nc= 0;

  if (!C && /*(ah>=0x0) &&*/ (ah<=0x9) && !H && /*(al>=0x0) &&*/ (al<=0x9)) { rA+= 0x00; nc= 0; }
  if (!C && /*(ah>=0x0) &&*/ (ah<=0x8) && !H && (al>=0xa) && (al<=0xf)) { rA+= 0x06; nc= 0; }
  if (!C && /*(ah>=0x0) &&*/ (ah<=0x9) &&  H && /*(al>=0x0) &&*/ (al<=0x3)) { rA+= 0x06; nc= 0; }

  if (!C && (ah>=0xa) && (ah<=0xf) && !H && /*(al>=0x0) &&*/ (al<=0x9)) { rA+= 0x60; nc= 1; }
  if (!C && (ah>=0x9) && (ah<=0xf) && !H && (al>=0xa) && (al<=0xf)) { rA+= 0x66; nc= 1; }
  if (!C && (ah>=0xa) && (ah<=0xf) &&  H && /*(al>=0x0) &&*/ (al<=0x3)) { rA+= 0x66; nc= 1; }

  if ( C && /*(ah>=0x0) &&*/ (ah<=0x2) && !H && /*(al>=0x0) &&*/ (al<=0x9)) { rA+= 0x60; nc= 1; }
  if ( C && /*(ah>=0x0) &&*/ (ah<=0x2) && !H && (al>=0xa) && (al<=0xf)) { rA+= 0x66; nc= 1; }
  if ( C && /*(ah>=0x0) &&*/ (ah<=0x3) &&  H && /*(al>=0x0) &&*/ (al<=0x3)) { rA+= 0x66; nc= 1; }

  rF&= ~(flagN|flagZ|flagC);
  if (nc) rF|= flagC;
  if (rA==0) rF|= flagZ;
  if (rA&0x80) rF|= flagN;
  cF.W(rF);
  cA.W(rA);
  return resGO;
}

int
CL12::idiv(void)
{
  u8_t f= rF&~(flagZ|flagV|flagC);
  if (rX == 0)
    {
      cX.W(0xffff);
      f|= flagC;
    }
  else
    {
      u16_t q, r;
      q= rD/rX;
      r= rD%rX;
      if (!q)
	f|= flagZ;
      cX.W(q);
      cD.W(r);
    }
  cF.W(f);
  return resGO;
}

int
CL12::fdiv(void)
{
  u8_t f= rF&~(flagZ|flagV|flagC);
  if (rX == 0)
    {
      cX.W(0xffff);
      f|= flagC;
    }
  else if (rX <= rD)
    {
      cX.W(0xffff);
      f|= flagV;
    }
  else
    {
      u32_t n;
      u16_t d, q, r;
      n= rD<<16;
      d= rX;
      q= n/d;
      r= n%d;
      if (!q)
	f|= flagZ;
      cX.W(q);
      cD.W(r);
    }
  cF.W(f);
  return resGO;
}

int
CL12::emacs(void)
{
  i16_t mx= read_addr(rom, rX);
  i16_t my= read_addr(rom, rY);
  u8_t h, l;
  h= fetch();
  l= fetch();
  u16_t a= h*256+l;
  i32_t m;
  m= read_addr(rom, a);
  m<<= 16;
  m+= read_addr(rom, a+2);
  vc.rd+= 8;
  i32_t i= mx*my;
  i32_t r= i + m;
  u8_t f= rF&~(flagN|flagZ|flagV|flagC);
  if (r & 0x80000000) f|= flagN;
  if (!r) f|= flagZ;
  if (0x80000000 & ((m&i&~r)|(~m|~i|~r))) f|= flagV;
  if (0x8000 & ((m&i)|(i&~r)|(~r&m))) f|= flagC;
  rom->write(a+0, r>>24);
  rom->write(a+1, r>>16);
  rom->write(a+2, r>> 8);
  rom->write(a+3, r    );
  vc.wr+= 4;
  cF.W(f);
  return resGO;
}

int
CL12::emuls(void)
{
  i32_t r= (i16_t)rD*(i16_t)rY;
  u8_t f= rF&~(flagN|flagZ|flagC);
  if (r & 0x80000000) f|= flagN;
  if (!r) f|= flagZ;
  if (r & 0x8000) f|= flagC;
  cY.W(r>>16);
  cD.W(r);
  cF.W(f);
  return resGO;
}

int
CL12::edivs(void)
{
  i32_t op1= (rY<<16)+rD, q;
  i16_t r;
  u8_t f= rF&~(flagN|flagZ|flagV|flagC);
  if (rX != 0)
    {
      q= op1 / (i16_t)rX;
      r= op1 % (i16_t)rX;
      if (q&0x8000) f|= flagN;
      if (!q) f|= flagZ;
      if ((q > 32767) || (q < -32768))
	f|= flagV;
      cY.W(q);
      cD.W(r);
    }
  else
    f|= flagC;
  cF.W(f);
  return resGO;
}

int
CL12::idivs(void)
{
  i32_t q;
  i16_t r;
  u8_t f= rF&~(flagN|flagZ|flagV|flagC);
  if (rX == 0)
    {
      f|= flagC;
    }
  else
    {
      q= (i16_t)rD / (i16_t)rX;
      r= (i16_t)rD % (i16_t)rX;
      if ((q > 32767) || (q < -32768))
	f|= flagV;
      if (!q) f|= flagZ;
      if (q & 0x8000) f|= flagN;
      cX.W(q);
      cD.W(r);
    }
  cF.W(f);
  return resGO;
}

int
CL12::maxa(void)
{
  u16_t a= naddr(NULL, NULL, 0);
  u8_t op= rom->read(a);
  vc.rd++;
  cmp(rA, op);
  if (rF&flagC)
    cA.W(op);
  return resGO;
}

int
CL12::mina(void)
{
  u16_t a= naddr(NULL, NULL, 0);
  u8_t op= rom->read(a);
  vc.rd++;
  cmp(rA, op);
  if (!(rF&flagC))
    cA.W(op);
  return resGO;
}

int
CL12::emaxd(void)
{
  u16_t a= naddr(NULL, NULL, 0);
  u16_t op= read_addr(rom, a);
  vc.rd+= 2;
  cp16(rD, op);
  if (rF&flagC)
    cD.W(op);
  return resGO;
}

int
CL12::emind(void)
{
  u16_t a= naddr(NULL, NULL, 0);
  u16_t op= read_addr(rom, a);
  vc.rd+= 2;
  cp16(rD, op);
  if (!(rF&flagC))
    cD.W(op);
  return resGO;
}

int
CL12::maxm(void)
{
  u16_t a= naddr(NULL, NULL, 0);
  u8_t op= rom->read(a);
  vc.rd++;
  cmp(rA, op);
  if (!(rF&flagC))
    {
      rom->write(a, rA);
      vc.wr++;
    }
  return resGO;
}

int
CL12::minm(void)
{
  u16_t a= naddr(NULL, NULL, 0);
  u8_t op= rom->read(a);
  vc.rd++;
  cmp(rA, op);
  if (rF&flagC)
    {
      rom->write(a, rA);
      vc.wr++;
    }
  return resGO;
}

int
CL12::emaxm(void)
{
  u16_t a= naddr(NULL, NULL, 0);
  u16_t op= read_addr(rom, a);
  vc.rd+= 2;
  cp16(rD, op);
  if (!(rF&flagC))
    {
      rom->write(a, rD>>8);
      rom->write(a+1, rD);
      vc.wr+= 2;
    }
  return resGO;
}

int
CL12::eminm(void)
{
  u16_t a= naddr(NULL, NULL, 0);
  u16_t op= read_addr(rom, a);
  vc.rd+= 2;
  cp16(rD, op);
  if (rF&flagC)
    {
      rom->write(a, rD>>8);
      rom->write(a+1, rD);
      vc.wr+= 2;
    }
  return resGO;
}

int
CL12::tbl(void)
{
  u8_t xb= rom->read(PC);
  int t= xb_type(xb);
  u16_t a= naddr(NULL, NULL, 0);
  if ((t==1) || (t==3) || (t==4))
    {
      u8_t y1, y2;
      int diff;
      double b= rB/256.0;
      y1= rom->read(a);
      y2= rom->read(a+1);
      vc.rd+= 2;
      diff= y2-y1;
      cA.W((u32_t)(y1 + b*diff));
      u8_t f= rF&~(flagN|flagZ);
      if (rA & 0x80) f|= flagN;
      if (!rA) f|= flagZ;
      cF.W(f);
    }
  else
    return resINV;
  return resGO;
}

int
CL12::etbl(void)
{
  u8_t xb= rom->read(PC);
  int t= xb_type(xb);
  u16_t a= naddr(NULL, NULL, 0);
  if ((t==1) || (t==3) || (t==4))
    {
      u16_t y1, y2;
      long int diff;
      double b= rB/256.0;
      y1= read_addr(rom, a);
      y2= read_addr(rom, a+2);
      vc.rd+= 4;
      diff= y2-y1;
      cD.W((u32_t)(y1 + b*diff));
      u8_t f= rF&~(flagN|flagZ);
      if (rD & 0x8000) f|= flagN;
      if (!rD) f|= flagZ;
      cF.W(f);
    }
  else
    return resINV;
  return resGO;
}

int
CL12::mem(void)
{
  u16_t a= rX;
  u8_t p1, p2, s1, s2;
  p1= rom->read(a++);
  p2= rom->read(a++);
  s1= rom->read(a++);
  s2= rom->read(a++);
  vc.rd+= 4;
  cX.W(a);
  int d1, d2;
  d1= rA-p1;
  d2= p2-rA;
  bool d12n= false;
  if ((d1 < 0) || (d2 < 0))
    d12n= true;
  int g1= 0, g2= 0;
  if (!d12n)
    {
      g1= s1*d1;
      g2= s2*d2;
    }
  int g;
  if (((s2 == 0) || (g2 > 0xff)) && (!d12n))
    g= 0xff;
  else
    g= g2;
  if (((s1 == 0) || (g1 > 0xff)) && (!d12n))
    /*g= g*/;
  else
    g= g1;
  rom->write(rY, g);
  cY.W(rY+1);
  vc.wr++;
  return resGO;
}

int
CL12::rev(void)
{
  bool V= rF&flagV;
  switch (rev_st)
    {
    case 0: case 2: // prepare
      rd_Rx= rom->read(rX);
      vc.rd++;
      cX.W(rX+1);
      rev_st= 1;
      tick(3);
      PC= instPC;
      break;
    case 1: // step
      Rx= rd_Rx;
      if ((Rx == 0xfe) || (Rx == 0xff))
	{
	  rd_Fy= rom->read(rY+Rx);
	  vc.rd++;
	}
      if (Rx == 0xFE)
	{
	  if (V)
	    cA.W(0xff);
	  cF.W(rF^flagV);
	  V= rF&flagV;
	}
      if (Rx != 0xff)
	{
	  rd_Rx= rom->read(rX);
	  cX.W(rX+1);
	}
      if (!V)
	{
	  Fy= rd_Fy;
	  if (Rx == 0xfe)
	    {
	      if (Fy < rA)
		cA.W(Fy);
	    }
	  // else {}
	}
      else
	{
	  Fy= rd_Fy;
	  if ((Rx == 0xfe) || (Rx == 0xff))
	    {
	      if (rA > Fy)
		{
		  rom->write(rY+Rx, rA);
		  vc.wr++;
		}
	    }
	}
      if (Rx == 0xff)
	rev_st= 2;
      else
	PC= instPC;
      break;
    }
  return resGO;
}


/* End of m68hc12.src/ialu.cc */
