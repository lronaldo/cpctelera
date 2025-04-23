/*
 * Simulator of microcontrollers (inst.cc)
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


int
cl_rxk::ALTD(t_mem code)
{
  altd= prefix= true;
  tick(1);
  return resGO;
}

int
cl_rxk::IOI(t_mem code)
{
  prefix= true;
  rwas= ioi;
  tick(1);
  return resGO;
}

int
cl_rxk::IOE(t_mem code)
{
  prefix= true;
  rwas= ioe;
  tick(1);
  return resGO;
}

int
cl_rxk::NOP(t_mem code)
{
  tick(1);
  return resGO;
}

int
cl_rxk::SCF(t_mem code)
{
  destF().W(destF().R() | flagC);
  tick(1);
  return resGO;
}

int
cl_rxk::CCF(t_mem code)
{
  destF().W(destF().R() ^ flagC);
  tick(1);
  return resGO;
}

int
cl_rxk::EXX(t_mem code)
{
  u16_t t;
  t= rBC;
  cBC.W(raBC);
  caBC.W(t);

  t= rDE;
  cBC.W(raDE);
  caDE.W(t);
  
  t= rHL;
  cBC.W(raHL);
  caHL.W(t);
  
  tick(1);
  return resGO;
}

int
cl_rxk::ipset(u8_t n)
{
  u8_t v= rIP << 2;
  v|= (n&3);
  cIP.W(v);
  tick(3);
  return resGO;
}

int
cl_rxk::IPRES(t_mem code)
{
  u8_t v= (rIP >> 2) & 0x3f;
  u8_t l= rIP << 6;
  cIP.W(v|l);
  tick(3);
  return resGO;
}


int
cl_rxk::PAGE_CB(t_mem code)
{
  u8_t x, y, z;
  code= fetch();
  x= code>>6;
  y= (code>>3)&7;
  z= code&7;
  switch (x)
    {
    case 0:
      if (z == 6)
	{
	  vc.rd++;
	  vc.wr++;
	  tick(6);
	}
      switch (y)
	{
	case 0: return rlc (*destR(z), rR(z));
	case 1: return rrc (*destR(z), rR(z));
	case 2: return rl  (*destR(z), rR(z));
	case 3: return rr  (*destR(z), rR(z));
	case 4: return sla8(*destR(z), rR(z));
	case 5: return sra8(*destR(z), rR(z));
	case 6: return resINV_INST;
	case 7: return srl8(*destR(z), rR(z));
	}
      break;
    case 1: // BIT y,r
      if (z == 6)
	return bit_iHL(y);
      return bit_r(y, rR(z));
      break;
    case 2: // RES y,r
      if (z == 6)
	return res_iHL(y);
      return res_r(y, *destR(z), rR(z));
      break;
    case 3: // SET y,r
      if (z == 6)
	return set_iHL(y);
      return set_r(y, *destR(z), rR(z));
      break;
    }
  return resGO;
}

int
cl_rxk::PAGE_DD_CB(t_mem code)
{
  u8_t x, y, z, d;
  d= fetch();
  class cl_cell8 &dest= dest8iIRd(d);
  code= fetch();
  x= code>>6;
  y= (code>>3)&7;
  z= code&7;
  if (z != 6)
    return resINV_INST;
  switch (x)
    {
    case 0:
      switch (y)
	{
	case 0: return rlc (dest, dest.get());
	case 1: return rrc (dest, dest.get());
	case 2: return rl  (dest, dest.get());
	case 3: return rr  (dest, dest.get());
	case 4: return sla8(dest, dest.get());
	case 5: return sra8(dest, dest.get());
	case 6: return resINV_INST;
	case 7: return srl8(dest, dest.get());
	}
      break;
    case 1:
      return bit_iIRd(y, d);
      break;
    case 2:
      return res_iIRd(y, d);
      break;
    case 3:
      return set_iIRd(y, d);
      break;
    }
  return resGO;
}


/*
 *                                                    R3000A,R4000,R5000
 */

int
cl_r3ka::SETUSR(t_mem code)
{
  u8_t v= rSU << 2;
  v|= 1;
  atomic= true;
  tick(3);
  return resGO;
}

int
cl_r3ka::SURES(t_mem code)
{
  u8_t v= ((rSU >> 2) /*& 0x3f*/) | ((rSU << 6) /*& 0xc0*/);
  cSU.W(v);
  atomic= true;
  tick(3);
  return resGO;
}

int
cl_r3ka::RDMODE(t_mem code)
{
  u8_t v= rF & ~flagC;
  if (rSU & 1) v|= flagC;
  cF.W(v);
  tick(3);
  return resGO;
}

int
cl_r3ka::instruction_5b(t_mem code)
{
  ld_r_g(destE(), rE);
  if (!altd)
    {
      if ((edmr&1) && (rSU&1))
	{
	  // TODO: set System Violation interrupt flag
	}
    }
  return resGO;
}

int
cl_r4k::SETUSRP(t_mem code)
{
  u8_t n, m;
  rSU&= 0xfc;
  rSU|= 0x01;
  cSU.W(rSU);
  n= fetch();
  m= fetch();
  u16_t a= rSP;
  rom->write(--a, m);
  rom->write(--a, n);
  cSP.W(rSP);
  tick5p1(14);
  atomic= true;
  return resGO;
}

int
cl_r4k::SETSYSP(t_mem code)
{
  cSU.W((rSU>>2) | (rSU<<6));
  u8_t tl, th, n, m;
  n= fetch();
  m= fetch();
  u16_t a= rSP;
  tl= rom->read(a++);
  th= rom->read(a++);
  vc.rd+= 2;
  cSP.W(rSP);
  atomic= true;
  if ((th!=m) && (tl!=n))
    return resERROR;
  return resGO;
}

int
cl_r4k::convc_pp(class cl_cell32 &pp)
{
  pp.W(mem->log2phy(pp.read()));
  tick(7);
  return resGO;
}

int
cl_r4k::convd_pp(class cl_cell32 &pp)
{
  pp.W(mem->log2phy(pp.read()));
  tick(7);
  return resGO;
}


/* End of rxk.src/inst.cc */
