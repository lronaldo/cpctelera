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

#include "mos6502cl.h"


int
cl_mos6502::INY(t_mem code)
{
  cY.W(rY+1);
  rF&= ~(flagZ|flagS);
  if (!rY) rF|= flagZ;
  if (rY & 0x80) rF|= flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::INX(t_mem code)
{
  cX.W(rX+1);
  rF&= ~(flagZ|flagS);
  if (!rX) rF|= flagZ;
  if (rX & 0x80) rF|= flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::inc(class cl_cell8 &op)
{
  u8_t v;
  op.W(v= op.R()+1);
  rF&= ~(flagZ|flagS);
  if (!v) rF|= flagZ;
  if (v & 0x80) rF|= flagS;
  cF.W(rF);
  return resGO;
}

int
cl_mos6502::DEY(t_mem code)
{
  cY.W(rY-1);
  rF&= ~(flagZ|flagS);
  if (!rY) rF|= flagZ;
  if (rY & 0x80) rF|= flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::DEX(t_mem code)
{
  cX.W(rX-1);
  rF&= ~(flagZ|flagS);
  if (!rX) rF|= flagZ;
  if (rX & 0x80) rF|= flagS;
  cF.W(rF);
  tick(1);
  return resGO;
}

int
cl_mos6502::dec(class cl_cell8 &op)
{
  u8_t v;
  op.W(v= op.R()-1);
  rF&= ~(flagZ|flagS);
  if (!v) rF|= flagZ;
  if (v & 0x80) rF|= flagS;
  cF.W(rF);
  return resGO;
}

int
cl_mos6502::ora(class cl_cell8 &op)
{
  u8_t f= rF & ~(flagZ|flagS);
  cA.W(rA | op.R());
  if (!rA) f|= flagZ;
  if (rA&0x80) f|= flagS;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::And(class cl_cell8 &op)
{
  u8_t f= rF & ~(flagZ|flagS);
  cA.W(rA & op.R());
  if (!rA) f|= flagZ;
  if (rA&0x80) f|= flagS;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::eor(class cl_cell8 &op)
{
  u8_t f= rF & ~(flagZ|flagS);
  cA.W(rA ^ op.R());
  if (!rA) f|= flagZ;
  if (rA&0x80) f|= flagS;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::adc(class cl_cell8 &op)
{
  u8_t Op= op.R(), f, oA= rA;
  u16_t res;
  u8_t C= (rF&flagC)?1:0;
  f= rF & ~(flagZ|flagC|flagN);

  if (!(rF & flagD))
    {
      f&= ~flagV;
      res= rA + Op + C;
      cA.W(res);
      if (!rA) f|= flagZ;
      if (rA & 0x80) f|= flagN;
      if (res > 255) f|= flagC;
      if ( ((res^oA)&0x80) && !((oA^Op)&0x80) ) f|= flagV;
    }
  else
    {
      int opint= ((Op & 0xf0) >> 4) * 10;
      opint+= (Op & 0x0f);
      int accint= ((oA & 0xf0) >> 4) * 10;
      accint+= (oA & 0x0f);
      int sum= opint + accint + C;
      if (sum > 99)
	{
	  f|= flagC;
	  sum-= 100;
	}
      else
	{
	  f&= ~flagC;
	}
      u8_t resA= 0;
      if (sum >= 0 && sum < 100)
	{
	  int tens= sum/10;
	  int units= sum%10;
	  resA= ((u8_t)tens << 4 | (u8_t)units);
	}
      cA.W(resA);
      if (!resA) f|= flagZ;
      if (resA&0x80) f|= flagN;
    }
  cF.W(f);
  return resGO;
}

int
cl_mos6502::sbc(class cl_cell8 &op)
{
  u8_t Op= op.R();
  u16_t res;
  u8_t C= (rF&flagC)?1:0;
  u8_t f= rF & ~(flagC|flagZ|flagN);
  
  if (!(rF & flagD))
    {
      f&= ~flagV;
      res= rA-Op-1+C;
      if (0x80&( (res&Op&~rA) | (~res&~Op&rA) )) f|= flagV;
      if (res < 0x100) f|= flagC;
      if (!(res & 0xff)) f|= flagZ;
      if (res & 0x80) f|= flagN;
      cA.W(res);
    }
  else
    {
      int opint= ((Op & 0xf0) >> 4) * 10;
      opint+= (Op & 0x0f);
      int accint= ((rA & 0xf0) >> 4) * 10;
      accint+= (rA & 0x0f);
      int res= accint - opint - (C?0:1);
      if (res < 0)
	{
	  f&= ~flagC;
	  res+= 100;
	}
      else
	f|= flagC;
      u8_t resA= 0;
      if (res >= 0 && res < 100)
	{
	  int tens= res / 10;
	  int units= res % 10;
	  resA= ((u8_t)tens << 4 | (u8_t)units);
	}
      if (!resA) f|= flagZ;
      if (resA&0x80) f|= flagN;
      cA.W(resA);
    }
  cF.W(f);
  return resGO;
}

int
cl_mos6502::cmp(class cl_cell8 &op1, class cl_cell8 &op2)
{
  u16_t res= op1.R() - op2.R();
  u8_t f= rF & ~(flagZ|flagN|flagC);
  if (res < 256) f|= flagC;
  if (!(res&0xff)) f|= flagZ;
  if (res&0x80) f|= flagN;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::asl(class cl_cell8 &op)
{
  u8_t f= rF & ~(flagZ|flagN|flagC);
  u8_t v= op.R();
  if (v&0x80) f|= flagC;
  op.W(v<<=1);
  if (!v) f|= flagZ;
  if (v&0x80) f|= flagN;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::lsr(class cl_cell8 &op)
{
  u8_t f= rF & ~(flagZ|flagN|flagC);
  u8_t v= op.R();
  if (v&1) f|= flagC;
  op.W(v>>=1);
  if (!v) f|= flagZ;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::rol(class cl_cell8 &op)
{
  u8_t C= (rF&flagC)?1:0;
  u8_t f= rF & ~(flagZ|flagN|flagC);
  u8_t v= op.R();
  if (v&0x80) f|= flagC;
  v= (v<<1)|C;
  op.W(v);
  if (!v) f|= flagZ;
  if (v&0x80) f|= flagN;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::ror(class cl_cell8 &op)
{
  u8_t C= (rF&flagC)?0x80:0;
  u8_t f= rF & ~(flagZ|flagN|flagC);
  u8_t v= op.R();
  if (v&1) f|= flagC;
  v= (v>>1)|C;
  op.W(v);
  if (!v) f|= flagZ;
  if (v&0x80) f|= flagN;
  cF.W(f);
  return resGO;
}

int
cl_mos6502::bit(class cl_cell8 &op)
{
  u8_t v= op.R();
  u8_t f= rF & ~(flagZ|flagN|flagV);
  if (v&0x80) f|= flagN;
  if (v&0x40) f|= flagV;
  if (!(rA & v)) f|= flagZ;
  cF.W(f);
  return resGO;
}


/* End of mos6502.src/ialu.cc */
