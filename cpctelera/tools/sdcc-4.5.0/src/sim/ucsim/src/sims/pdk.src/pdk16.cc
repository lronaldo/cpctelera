/*
 * Simulator of microcontrollers (pdk16.cc)
 *
 * Copyright (C) 2016 Drotos Daniel
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

#include "glob.h"

#include "t16cl.h"
#include "wdtcl.h"

#include "pdk16cl.h"


cl_fpp16::cl_fpp16(int aid, class cl_pdk *the_puc, class cl_sim *asim):
  cl_fpp15(aid, the_puc, asim)
{
  type= new struct cpu_entry;
  type->type= CPU_PDK16;
}

cl_fpp16::cl_fpp16(int aid, class cl_pdk *the_puc, struct cpu_entry *IType, class cl_sim *asim):
  cl_fpp15(aid, the_puc, IType, asim)
{
}


int
cl_fpp16::init(void)
{
  cl_fpp15::init();
  return 0;
}

const char *
cl_fpp16::id_string(void)
{
  return "pdk16";
}


struct dis_entry *cl_fpp16::dis_tbl(void)
{
  return disass_pdk_16;
}


int
cl_fpp16::execute(unsigned int code)
{
  int c, i, n;
  unsigned int u;
  u8_t u8, m;
  
  switch (code & 0xffff)
    {
    case 0x0032: // push af
      pushlh(rA, rF);
      return resGO;
    case 0x0033: // pop af
      cF->W(pop());
      cA.W(pop());
      return resGO;
    case 0x0010: // addc a
      cA.W(add_to(rA, 0, fC));
      return resGO;
    case 0x0011: // subc a
      cA.W(sub_to(rA, 0, fC));
      return resGO;
    case 0x001a: // sr a
      store_flag(flag_c, rA & 1);
      cA.W(rA >>= 1);
      return resGO;
    case 0x001c: // src a
      c= rA & 1;
      rA>>= 1;
      cA.W(rA | (fC << 7));
      SETC(c);
    return resGO;
    case 0x001b: // sl a
      SETC(rA & 0x80);
      cA.W(rA << 1);
      return resGO;
    case 0x001d: // slc a
      c = rA & 0x80;
      rA <<= 1;
      cA.W(rA | fC);
      SETC(c);
      return resGO;
    case 0x001e: // swap a
      cA.W((rA>>4)|(rA<<4));
      return resGO;
    case 0x0018: // not a
      cA.W(~rA);
      SETZ(!rA);
      return resGO;
    case 0x0019: // neg a
      cA.W(-rA);
      SETZ(!rA);
      return resGO;
    case 0x003c: // mul
      u= rA * sfr->read(8);
      cA.W(u);
      if (puc) puc->rMULRH= u >> 8;
      return resGO;
    case 0x0012: // izsn a
      cA.W(add_to(rA, 1));
      if (!rA)
	PC++;
      return resGO;
    case 0x0013: // dzsn a
      cA.W(sub_to(rA, 1));
      if (!rA)
	PC++;
      return resGO;
    case 0x001f: // delay a
      if (puc && (puc->single))
	return resINV;
      cA.W(rA-1);
      if (rA)
	PC--;
      return resGO;
    case 0x003a: // ret
      cSP->W(rSP-2);
      PC= rd16(rSP);
      vc.rd+= 2;
      return resGO;
    case 0x003b: // reti
      return resNOT_DONE;
      return resGO;
    case 0x0000: // nop
      return resGO;
    case 0x0017: // pcadd a
      PC+= rA-1;
      return resGO;
    case 0x0038: // engint
      return resNOT_DONE;
      return resGO;
    case 0x0039: // disgint
      return resNOT_DONE;
      return resGO;
    case 0x0036: // stopsys
      if (puc) puc->mode= pm_pd;
      return resGO;
    case 0x0037: // stopexe
      if (puc) puc->mode= pm_ps;
      return resGO;
    case 0x0035: // reset
      reset();
      return resGO;
    case 0x0030: // wdtreset
      if (puc) puc->wdt->clear();
      return resGO;
    }

  switch (code & 0xfff0)
    {
    case 0x0070: // pushw pcN
      if (puc)
	u= puc->get_pc(code&0xf);
      else
	u= 0;
      push(u);
      tick(1);
      return resGO;
    case 0x0060: // popw pcN
      u= pop()*256+pop();
      if (puc)
	puc->set_pc(code&0xf, u);
      return resGO;
    }

  switch (code & 0xffe0)
    {
    case 0x0040: // pmode k
      return resNOT_DONE;
      return resGO;
    }

  switch (code & 0xffc0)
    {
    case 0x00c0: // mov a,IO
      cA.W(sfr->read(code & 0x3f));
      return resGO;
    case 0x0080: // mov IO,a
      sfr->write(code&0x3f, rA);
      return resGO;
    case 0x1040: // xor a,IO
      cA.W(rA ^ sfr->read(code&0x3f));
      SETZ(!rA);
      return resGO;
    case 0x1000: // xor IO,a
      sfr->write(code&0x3f, sfr->read(code&0x3f) ^ rA);
      return resGO;
    }

  switch (code & 0xff00)
    {
    case 0x1f00: // mov a,k
      cA.W(code);
      return resGO;
    case 0x1800: // add a,k
      cA.W(add_to(rA, code&0xff));
      return resGO;
    case 0x1900: // sub a,k
      cA.W(sub_to(rA, code&0xff));
      return resGO;
    case 0x1c00: // and a,k
      cA.W(rA & code & 0xff);
      SETZ(!rA);
      return resGO;
    case 0x1d00: // or a,k
      cA.W(rA | (code & 0xff));
      SETZ(!rA);
      return resGO;
    case 0x1e00: // xor a,k
      cA.W(rA ^ (code & 0xff));
      SETZ(!rA);
      return resGO;
    case 0x1a00: // ceqsn a,k
      sub_to(rA, code&0xff);
      if (rA == (code&0xff))
	PC++;
      return resGO;
    case 0x1b00: // cneqsn a,k
      sub_to(rA, code&0xff);
      if (rA != (code&0xff))
	PC++;
      return resGO;
    case 0x0e00: // delay k
      if (puc && (puc->single))
	return resINV;
      if (!rTMP)
	{
	  rTMP= code;
	  PC--;
	}
      else
	{
	  if (--rTMP)
	    PC--;
	}
      cA.W(rTMP);
      return resGO;
    case 0x0f00: // ret k
      cA.W(code);
      PC= pop()*256 + pop();
      return resGO;
    }

  switch (code & 0xfe00)
    {
    case 0x5c00: // mov M,a
      wr8(code & 0x1ff, rA);
      return resGO;
    case 0x5e00: // mov a,M
      cA.W(ram->read(code & 0x1ff));
      return resGO;
    case 0x3200: // nmov M,a
      wr8(code & 0x1ff, -rA);
      return resGO;
    case 0x3000: // nmov a,M
      cA.W(-get_mem(code & 0x1ff));
      SETZ(!rA);
      return resGO;
    case 0x0400:
      if (code & 1)
	{
	  // pushw word
	  u= rd16(code&0x1fe);
	  push(u);
	}
      else
	{
	  // popw word
	  u= pop()*256+pop();
	  wr16(code&0x1fe, u);
	}
      tick(1);
      return resGO;
    case 0x0a00:
      if (code & 1)
	{
	  // ldtabh index
	  u= rd16(code & 0x1fe);
	  cA.W(rom->read(u+1));
	}
      else
	{
	  // ldtabl index
	  u= rd16(code & 0x1fe);
	  cA.W(rom->read(u));
	}
      tick(1);
      return resGO;
    case 0x0200:
      u= code & 0x1fe;
      if (code & 1)
	{
	  // ldt16 word
	  wr16(code & 0x01fe, puc?(puc->t16->cnt):0);
	}
      else
	{
	  // stt16 word
	  if (puc)
	    puc->t16->cnt= rd16(code & 0x01fe);
	}
      return resNOT_DONE;
      return resGO;
    case 0x6e00: // xch M
      u= code & 0x1ff;
      u8= rd8(u);
      wr8(u, rA);
      cA.W(u8);
      return resGO;
    case 0x6c00: // clear M
      wr8(code & 0x1ff, 0);
      return resGO;
    case 0x0800:
      if (code & 1)
	{
	  // idxm a,M
	  u= rd16(code & 0x1fe);
	  cA.W(rd8(u));
	}
      else
	{
	  // idxm M,a
	  u= rd16(code & 0x1fe);
	  wr8(rd8(u), rA);
	}
      tick(1);
      return resGO;
    case 0x4200: // add a,M
      cA.W(add_to(rA, rd8(code & 0x1ff)));
      return resGO;
    case 0x4000: // add M,a
      u= code & 0x1ff;
      wr8(u, add_to(rA, rd8(u)));
      return resGO;
    case 0x4a00: //  addc a,M
      cA.W(add_to(rA, rd8(code & 0x1ff), fC));
      return resGO;
    case 0x4800: // addc M,a
      u= code & 0x1ff;
      wr8(u, add_to(rA, rd8(u), fC));
      return resGO;
    case 0x6000: // addc M
      u= code & 0x1ff;
      wr8(u, add_to(rd8(u), fC));
      return resGO;
    case 0x3400: // nadd a,M
      cA.W(add_to(rd8(code & 0x1ff), -rA));
      return resGO;
    case 0x3600: // nadd M,a
      u= code & 0x1ff;
      wr8(u, add_to(-rd8(u), rA));
      return resGO;
    case 0x4600: // sub a,M
      cA.W(sub_to(rA, rd8(code & 0x1ff)));
      return resGO;
    case 0x4400: // sub M,a
      u= code & 0x1ff;
      wr8(u, sub_to(rd8(u), rA));
      return resGO;
    case 0x4e00: // subc a,M
      cA.W(sub_to(rA, rd8(code & 0x1ff), fC));
      return resGO;
    case 0x4c00: // subc M,a
      u= code & 0x1ff;
      wr8(u, sub_to(rd8(u), rA, fC));
      return resGO;
    case 0x6200: // subc M
      u= code & 0x1ff;
      wr8(u, sub_to(rd8(u), fC));
      return resGO;
    case 0x6800: // inc M
      u= code & 0x1ff;
      wr8(u, add_to(rd8(u), 1));
      return resGO;
    case 0x6a00: // dec M
      u= code & 0x1ff;
      wr8(u, sub_to(rd8(u), 1));
      return resGO;
    case 0x7400: // sr M
      u= code & 0x1ff;
      u8= rd8(u);
      SETC(u8 & 1);
      wr8(u, u8>>1);
      return resGO;
    case 0x7800: // src M
      u= code & 0x1ff;
      u8= rd8(u);
      c= u8 & 1;
      wr8(u, (u8>>1) | (fC<<7));
      SETC(c);
      return resGO;
    case 0x7600: // sl M
      u= code & 0x1ff;
      u8= rd8(u);
      SETC(u8 & 0x80);
      wr8(u, u8<<1);
      return resGO;
    case 0x7a00: // slc M
      u= code & 0x1ff;
      u8= rd8(u);
      c= u8 & 0x80;
      wr8(u, (u8<<1)|fC);
      SETC(c);
      return resGO;
    case 0x7c00: // swap M
      u= code & 0x1ff;
      u8= rd8(u);
      wr8(u, (u8<<4)|(u8>>4));
      return resGO;
    case 0x5200: // and a,M
      cA.W(rA & rd8(code & 0x1ff));
      SETZ(!rA);
      return resGO;
    case 0x5000: // and M,a
      u= code & 0x1ff;
      u8= rA & rd8(u);
      SETZ(!u8);
      wr8(u, u8);
      return resGO;
    case 0x5600: // or a,M
      cA.W(rA | rd8(code & 0x1ff));
      SETZ(!rA);
      return resGO;
    case 0x5400: // or M,a
      u= code & 0x1ff;
      u8= rA | rd8(u);
      SETZ(!u8);
      wr8(u, u8);
      return resGO;
    case 0x5a00: // xor a,M
      cA.W(rA ^ rd8(code & 0x1ff));
      SETZ(!rA);
      return resGO;
    case 0x5800: // xor M,a
      u= code & 0x1ff;
      u8= rA ^ rd8(u);
      SETZ(!u8);
      wr8(u, u8);
      return resGO;
    case 0x7000: // not M
      u= code & 0x1ff;
      u8= ~rd8(u);
      SETZ(!u8);
      wr8(u, u8);
      return resGO;
    case 0x7200: // neg M
      u= code & 0x1ff;
      u8= -rd8(u);
      SETZ(!u8);
      wr8(u, u8);
      return resGO;
    case 0x3c00: // comp a,M
      sub_to(rA, rd8(code & 0x1ff));
      return resGO;
    case 0x3e00: // comp M,a
      sub_to(rd8(code & 0x1ff), rA);
      return resGO;
    case 0x3800: // ceqsn a,M
      u8= rd8(code & 0x1ff);
      sub_to(rA, u8);
      if (rA == u8)
	PC++;		   
      return resGO;
    case 0x3a00: // ceqsn M,a
      u8= rd8(code & 0x1ff);
      sub_to(u8, rA);
      if (rA == u8)
	PC++;		   
      return resGO;
    case 0x1600: // cneqsn a,M
      u8= rd8(code & 0x1ff);
      sub_to(rA, u8);
      if (rA != u8)
	PC++;  
      return resGO;
    case 0x1400: // cneqsn M,a
      u8= rd8(code & 0x1ff);
      sub_to(u8, rA);
      if (rA != u8)
	PC++;  
      return resGO;
    case 0x6400: // izsn M
      u= code & 0x1ff;
      u8= add_to(rd8(u), 1);
      wr8(u, u8);
      if (!u8)
	PC++;
      return resGO;
    case 0x6600: // dzsn M
      u= code & 0x1ff;
      u8= sub_to(rd8(u), 1);
      wr8(u, u8);
      if (!u8)
	PC++;
      return resGO;
    case 0x2400: // set0 IO.n
      u= code & 0x03f;
      n= (code>>6)&7;
      u8= sfr->read(u);
      u8&= ~(1<<n);
      sfr->write(u, u8);
      return resGO;
    case 0x2600: // set1 IO.n
      u= code & 0x3f;
      n= (code>>6)&7;
      u8= sfr->read(u);
      u8|= (1<<n);
      sfr->write(u, u8);
      return resGO;
    case 0x2e00: // swapc IO.n
      u= code & 0x3f;
      n= (code>>6)&7;
      m= 1<<n;
      u8= sfr->read(u);
      c= u8 & m;
      fC?(u8|=m):(u8&=~m);
      SETC(c);
      sfr->write(u, u8);
      return resGO;
    case 0x2000: // t0sn IO.n
      n= (code>>6)&7;
      u8= sfr->read(code & 0x3f);
      if (!(u8 & (1<<n)))
	PC++;
      return resGO;
    case 0x2200: // t1sn IO.n
      n= (code>>6)&7;
      u8= sfr->read(code & 0x3f);
      if (u8 & (1<<n))
	PC++;
      return resGO;
    case 0x2a00: // wait0 IO.n
      if (puc && (puc->single))
	return resINV;
      n= (code>>6)&7;
      u8= sfr->read(code & 0x3f);
      if (u8 & (1<<n))
	PC--;
      return resGO;
    case 0x2c00: // wait1 IO.n
      if (puc && (puc->single))
	return resINV;
      n= (code>>6)&7;
      u8= sfr->read(code & 0x3f);
      if (!(u8 & (1<<n)))
	PC--;
      return resGO;
    case 0x0600:
      // TODO assumption
      u= code & 0x1fe;
      if (code & 1)
	{
	  // icall M
	  push(PC);
	  PC= rd16(u);
	}
      else
	{
	  // igoto M
	  PC= rd16(u);
	}
      tick(1);
      return resGO;
    case 0x7e00: // delay M
      if (puc && (puc->single))
	return resINV;
      if (!rTMP)
	{
	  rTMP= rd8(code & 0x1ff);
	  PC--;
	}
      else
	{
	  if (--rTMP)
	    PC--;
	}
      cA.W(rTMP);
      return resGO;
    }

  switch (code & 0xf000)
    {
    case 0xa000: // set0 M,n
      u= code & 0x1ff;
      n= (code>>9)&7;
      u8= rd8(u);
      u8&= ~(1<<n);
      wr8(u, u8);
      return resGO;
    case 0xb000: // set1 M,n
      u= code & 0x1ff;
      n= (code>>9)&7;
      u8= rd8(u);
      u8|= (1<<n);
      wr8(u, u8);
      return resGO;
    case 0x8000: // t0sn M,n
      u8= rd8(code & 0x1ff);
      n= (code>>9)&7;
      if (!(u8 & (1>>n)))
	PC++;
      return resGO;
    case 0x9000: // t1sn M,n
      u8= rd8(code & 0x1ff);
      n= (code>>9)&7;
      if (u8 & (1>>n))
	PC++;
      return resGO;
    }

  switch (code & 0xe000)
    {
    case 0xe000: // call label
      push(PC);
      PC= code & 0x1fff;
      tick(1);
      return resGO;
    case 0xc000: // goto label
      PC= code & 0x1fff;
      tick(1);
      return resGO;
    }
  
  return resINV;
}


/* End of pdk.src/pdk16.cc */
