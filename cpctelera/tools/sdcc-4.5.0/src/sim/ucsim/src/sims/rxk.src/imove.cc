/*
 * Simulator of microcontrollers (imove.cc)
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

#include "glob.h"
#include "rxkcl.h"
#include "r4kcl.h"


int
cl_rxk::ld_dd_mn(class cl_cell16 &dd)
{
  u8_t l, h;
  l= fetch();
  h= fetch();
  dd.W(h*256+l);
  tick(5);
  return resGO;
}

int
cl_rxk::LD_imn_HL(t_mem code)
{
  u8_t l, h;
  l= fetch();
  h= fetch();
  t_addr a= h*256+l;
  rwas->write(a, rL);
  rwas->write(a+1, rH);
  vc.wr+= 2;
  tick(12);
  return resGO;
}

int
cl_rxk::LD_HL_imn(t_mem code)
{
  u8_t l, h;
  l= fetch();
  h= fetch();
  t_addr a= h*256+l;
  l= rwas->read(a);
  h= rwas->read(a+1);
  vc.rd+= 2;
  destH().W(h);
  destL().W(l);
  tick(10);
  return resGO;
}

int
cl_rxk::ld_r_n(class cl_cell8 &r)
{
  r.W(fetch());
  tick(3);
  return resGO;
}

int
cl_rxk::ld_ihl_r(u8_t op)
{
  class cl_cell8 &c= dest8iHL();
  c.W(op);
  vc.wr++;
  tick(5);
  return resGO;
}

int
cl_rxk::ld_r_ihl(class cl_cell8 &destr)
{
  u8_t v= read8(rHL);
  destr.W(v);
  tick(4);
  return resGO;
}

int
cl_rxk::ld_r_g(class cl_cell8 &dest, u8_t op)
{
  dest.W(op);
  tick(1);
  return resGO;
}

int
cl_rxk::pop_zz(class cl_cell16 &dest)
{
  u8_t l, h;
  l= rom->read(rSP);
  cSP.W(rSP+1);
  h= rom->read(rSP);
  cSP.W(rSP+1);
  dest.W(h*256+l);
  vc.rd+= 2;
  tick(6);
  return resGO;
}

int
cl_rxk::push_zz(u16_t op)
{
  u8_t h, l;
  h= op>>8;
  l= op&0xff;
  cSP.W(rSP-1);
  rom->write(rSP, h);
  cSP.W(rSP-1);
  rom->write(rSP, l);
  vc.wr+= 2;
  tick5p1(9);
  return resGO;
}

int
cl_rxk::ld_d_i(int dif)
{
  int t= 9;
  if (rwas == ioi)
    t+= 1;
  else if (rwas == ioe)
    t+= 2;
  rwas->write(rDE, rom->read(rHL));
  vc.rd++;
  vc.wr++;
  cBC.W(rBC-1);
  cDE.W(rDE+dif);
  cHL.W(rHL+dif);
  tick(t);
  if (rBC == 0)
    cF.W(rF & ~flagV);
  else
    cF.W(rF | flagV);
  return resGO;
}

int
cl_rxk::LDxR(int dif)
{
  int t= 7;
  if (rwas == ioi)
    t+= 1;
  else if (rwas == ioe)
    t+= 2;
  tick(5);
  do
    {
      rwas->write(rDE, rom->read(rHL));
      vc.rd++;
      vc.wr++;
      cBC.W(rBC-1);
      cDE.W(rDE+dif);
      cHL.W(rHL+dif);
      tick(t);
    }
  while (rBC != 0);
  cF.W(rF & ~flagV);
  return resGO;
}

int
cl_rxk::ld_iIRd_r(u8_t op)
{
  i8_t d= fetch();
  t_addr a= cIR->get();
  a+= d;
  rwas->write(a, op);
  vc.wr++;
  tick5p1(9);
  return resGO;
}

int
cl_rxk::ld_r_iIRd(class cl_cell8 &op)
{
  i8_t d= fetch();
  t_addr a= cIR->get() + d;
  op.W(rwas->read(a));
  vc.rd++;
  tick5p1(8);
  return resGO;
}

int
cl_rxk::ld_hl_op(u16_t op)
{
  destHL().W(op);
  tick(1);
  return resGO;
}

int
cl_rxk::LD_iBC_A(t_mem code)
{
  class cl_cell8 &c= dest8iBC();
  c.W(rA);
  vc.wr++;
  tick(6);
  return resGO;
}

int
cl_rxk::LD_iDE_A(t_mem code)
{
  class cl_cell8 &c= dest8iDE();
  c.W(rA);
  vc.wr++;
  tick(6);
  return resGO;
}

int
cl_rxk::LD_iMN_A(t_mem code)
{
  class cl_cell8 &c= dest8imn();
  c.W(rA);
  vc.wr++;
  tick(9);
  return resGO;
}

int
cl_rxk::LD_A_iBC(t_mem code)
{
  destA().W(read8(rBC));
  tick(5);
  return resGO;
}

int
cl_rxk::LD_A_iDE(t_mem code)
{
  destA().W(read8(rDE));
  tick(5);
  return resGO;
}

int
cl_rxk::LD_A_iMN(t_mem code)
{
  destA().W(read8(fetch16()));
  tick(8);
  return resGO;
}

int
cl_rxk::EX_AF_aAF(t_mem code)
{
  u16_t temp= rAF;
  cAF.W(raAF);
  caAF.W(temp);
  tick(1);
  return resGO;
}

int
cl_rxk::EX_aDE_HL(t_mem code)
{
  class cl_cell16 &dhl= destHL();
  u16_t t= raDE;
  caDE.W(dhl.get());
  dhl.W(t);
  tick(1);
  return resGO;
}

int
cl_rxk::EX_DE_HL(t_mem code)
{
  class cl_cell16 &dhl= destHL();
  u16_t t= rDE;
  cDE.W(dhl.get());
  dhl.W(t);
  tick(1);
  return resGO;
}

int
cl_rxk::LD_HL_iSPn(t_mem code)
{
  class cl_cell16 &dest= destHL();
  u8_t n= fetch(), l, h;
  u16_t addr= rSP+n;
  l= rom->read(addr);
  addr++;
  h= rom->read(addr);
  dest.W(h*256+l);
  vc.rd+= 2;
  tick5p1(8);
  return resGO;
}

int
cl_rxk::LD_HL_iIXd(t_mem code)
{
  i8_t d= fetch();
  class cl_cell16 &dhl= destHL();
  u16_t a= rIX+d;
  u8_t h, l;
  l= rwas->read(a);
  h= rwas->read(a+1);
  dhl.W(h*256+l);
  vc.rd+= 2;
  tick5p1(8);
  return resGO;
}

int
cl_rxk::LD_iIXd_HL(t_mem code)
{
  i8_t d= fetch();
  u16_t a= rIX+d;
  rwas->write(a, rL);
  rwas->write(a+1, rH);
  vc.wr+= 2;
  tick5p1(10);
  return resGO;
}

int
cl_rxk::LD_iSPn_HL(t_mem code)
{
  u8_t n= fetch();
  u16_t addr= rSP+n;
  rom->write(addr, rL);
  addr++;
  rom->write(addr, rH);
  vc.wr+= 2;
  tick5p1(10);
  return resGO;
}

int
cl_rxk::LD_EIR_A(t_mem code)
{
  cEIR.W(rA);
  tick(3);
  return resGO;
}

int
cl_rxk::LD_IIR_A(t_mem code)
{
  cIIR.W(rA);
  tick(3);
  return resGO;
}

int
cl_rxk::LD_A_EIR(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t res= rEIR, forg= rF & ~(flagS|flagZ);
  destA().W(res);
  if (!res) forg|= flagZ;
  if (res&0x80) forg|= flagS;
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::LD_A_IIR(t_mem code)
{
  class cl_cell8 &f= destF();
  u8_t res= rIIR, forg= rF & ~(flagS|flagZ);
  destA().W(res);
  if (!res) forg|= flagZ;
  if (res&0x80) forg|= flagS;
  f.W(forg);
  tick(3);
  return resGO;
}

int
cl_rxk::LD_IR_mn(t_mem code)
{
  u8_t h, l;
  l= fetch();
  h= fetch();
  cIR->W(h*256+l);
  tick(7);
  return resGO;
}

int
cl_rxk::POP_IR(t_mem code)
{
  u8_t h, l;
  l= rom->read(rSP);
  cSP.W(rSP+1);
  h= rom->read(rSP);
  cSP.W(rSP+1);
  vc.rd+= 2;
  cIR->write(h*256+l);
  tick(8);
  return resGO;
}

int
cl_rxk::PUSH_IR(t_mem code)
{
  u16_t v= cIR->get();
  u8_t h, l;
  h= v>>8;
  l= v;
  cSP.W(rSP-1);
  rom->write(rSP, h);
  cSP.W(rSP-1);
  rom->write(rSP, l);
  vc.wr+= 2;
  tick5p1(11);
  return resGO;
}

int
cl_rxk::EX_iSP_IR(t_mem code)
{
  u16_t temp;
  temp= rom->read(rSP+1) * 256 + rom->read(rSP);
  vc.rd+= 2;
  rom->write(rSP, cIR->get());
  rom->write(rSP+1, cIR->get()>>8);
  vc.wr+= 2;
  cIR->write(temp);
  tick(14);
  return resGO;
}

int
cl_rxk::LD_SP_IR(t_mem code)
{
  cSP.W(cIR->get());
  tick5p2(1);
  return resGO;
}

int
cl_rxk::LD_IR_iSPn(t_mem code)
{
  u8_t n= fetch(), h, l;
  u16_t addr= rSP+n;
  l= rom->read(addr);
  addr++;
  h= rom->read(addr);
  vc.rd+= 2;
  cIR->write(h*256+l);
  tick5p1(10);
  return resGO;
}

int
cl_rxk::LD_iSPn_IR(t_mem code)
{
  u16_t v= cIR->get();
  u8_t n= fetch();
  u16_t addr= rSP+n;
  rom->write(addr, v);
  addr++;
  rom->write(addr, v>>8);
  vc.wr+= 2;
  tick5p1(12);
  return resGO;
}

int
cl_rxk::LD_HL_iIRd(t_mem code)
{
  class cl_cell16 *ir= cIR;
  if (cIR == &cIX)
    ir= &cHL;
  i8_t d= fetch();
  t_addr a= ir->get() + d;
  destHL().W(read16io(a));
  tick5p1(10);
  return resGO;
}

int
cl_rxk::LD_iIRd_n(t_mem code)
{
  i8_t d= fetch();
  u8_t n= fetch();
  t_addr a= cIR->get()+d;
  rwas->write(a, n);
  tick5p1(10);
  vc.wr++;
  return resGO;
}

int
cl_rxk::ld_dd_imn(class cl_cell16 &dest)
{
  u16_t a= fetch16();
  dest.W(read16io(a));
  tick(12);
  return resGO;
}

int
cl_rxk::EXX_iSP_HL(t_mem code)
{
  class cl_cell16 &hl= destHL();
  u16_t temp= read16(rSP);
  write16(rSP, rHL);
  hl.W(temp);
  tick(14);
  return resGO;
}

int
cl_rxk::ld_add_BC_DE(class cl_cell16 &dest, u16_t src)
{
  dest.W(src);
  tick(3);
  return resGO;
}

int
cl_rxk::ld_imn_ss(u16_t src)
{
  u8_t l, h;
  l= fetch();
  h= fetch();
  u16_t a= h*256+l;
  l= src;
  h= src>>8;
  rwas->write(a, l);
  a++;
  rwas->write(a, h);
  vc.wr+= 2;
  tick(14);
  return resGO;
}

int
cl_rxk::ldp_irp_rp(u16_t addr, u16_t src)
{
  t_addr a= ((u32_t)rA << 16) + addr;
  mem->phwrite(a, src&0xff);
  addr++;
  a= ((u32_t)rA << 16) + addr; // LDP wraps around 64k page boundary
  mem->phwrite(a, (src>>8)&0xff);
  vc.wr+= 2;
  tick(11);
  return resGO;
}

int
cl_rxk::ldp_rp_irp(class cl_cell16 &dest, u16_t addr)
{
  t_addr a= ((u32_t)rA >> 16) + addr;
  u8_t l= mem->phread(a);
  addr++;
  a= ((u32_t)rA >> 16) + addr;
  u8_t h= mem->phread(a);
  dest.W(h*256+l);
  vc.rd+= 2;
  tick(9);
  return resGO;
}

int
cl_rxk::LD_XPC_A(t_mem code)
{
  cXPC.W(rA);
  atomic= true;
  tick(3);
  return resGO;
}

int
cl_rxk::LD_A_XPC(t_mem code)
{
  destA().W(rXPC);
  atomic= true;
  tick(3);
  return resGO;
}

int
cl_rxk::PUSH_IP(t_mem code)
{
  cSP.W(rSP-1);
  rom->write(rSP, rIP);
  vc.wr++;
  tick5p1(8);
  return resGO;
}

int
cl_rxk::POP_IP(t_mem code)
{
  cIP.W(rom->read(rSP));
  cSP.W(rSP+1);
  vc.rd++;
  tick(6);
  atomic= true;
  return resGO;
}

int
cl_rxk::LD_imn_IR(t_mem code)
{
  u8_t h, l;
  l= fetch();
  h= fetch();
  u16_t addr= h*256+l;
  u16_t v= cIR->get();
  h= v>>8;
  l= v;
  rwas->write(addr, l);
  addr++;
  rwas->write(addr, h);
  vc.wr+= 2;
  tick(14);
  return resGO;
}

int
cl_rxk::LD_IR_imn(t_mem code)
{
  u8_t h, l;
  l= fetch();
  h= fetch();
  u16_t addr= h*256+l;
  l= rwas->read(addr);
  addr++;
  h= rwas->read(addr);
  cIR->write(h*256+l);
  vc.rd+= 2;
  tick(12);
  return resGO;
}

int
cl_rxk::LD_HL_IR(t_mem code)
{
  destHL().W(cIR->get());
  tick(3);
  return resGO;
}

int
cl_rxk::LD_IR_HL(t_mem code)
{
  cIR->write(rHL);
  tick(3);
  return resGO;
}

int
cl_rxk::LD_iHLd_HL(t_mem code)
{
  i8_t d= fetch();
  u16_t addr= rHL+d;
  rwas->write(addr, rL);
  addr++;
  rwas->write(addr, rH);
  tick5p1(12);
  return resGO;
}


/*
 *                                                Rabbit 3000A,4000,5000
 */

int
cl_r3ka::LDxSR(int dif)
{
  int t= 7;
  if (rwas == ioi)
    t+= 1;
  else if (rwas == ioe)
    t+= 2;
  tick(5);
  do
    {
      rwas->write(rDE, rom->read(rHL));
      vc.rd++;
      vc.wr++;
      cBC.W(rBC-1);
      cHL.W(rHL+dif);
      tick(t);
    }
  while (rBC != 0);
  cF.W(rF & ~flagV);
  return resGO;
}

int
cl_r3ka::LSxDR(int dif)
{
  int t= 7;
  if (rwas == ioi)
    t+= 1;
  else if (rwas == ioe)
    t+= 2;
  tick(5);
  do
    {
      rom->write(rDE, rwas->read(rHL));
      vc.rd++;
      vc.wr++;
      cBC.W(rBC-1);
      cDE.W(rDE+dif);
      tick(t);
    }
  while (rBC != 0);
  cF.W(rF & ~flagV);
  return resGO;
}

int
cl_r3ka::LSxR(int dif)
{
  int t= 7;
  if (rwas == ioi)
    t+= 1;
  else if (rwas == ioe)
    t+= 2;
  tick(5);
  do
    {
      rom->write(rDE, rwas->read(rHL));
      vc.rd++;
      vc.wr++;
      cBC.W(rBC-1);
      cDE.W(rDE+dif);
      cHL.W(rHL+dif);
      tick(t);
    }
  while (rBC != 0);
  cF.W(rF & ~flagV);
  return resGO;
}

int
cl_r3ka::PUSH_SU(t_mem code)
{
  cSP.W(rSP-1);
  rom->write(rSP, rSU);
  vc.wr++;
  tick5p1(8);
  atomic= true;
  return resGO;
}

int
cl_r3ka::POP_SU(t_mem code)
{
  cSU.W(rom->read(rSP));
  cSP.W(rSP+1);
  vc.rd++;
  atomic= true;
  tick5p1(8);
  return resGO;
}


/*
 *                                                     Rabbit 4000, 5000
 */

int
cl_r4k::LD_A_iIRA(t_mem code)
{
  class cl_cell8 &a= destA();
  u16_t addr= cIR->get() + rA;
  a.W(rwas->read(addr));
  vc.rd++;
  tick5p1(7);
  return resGO;
}

int
cl_r4k::CBM_N(t_mem code)
{
  u8_t n= fetch();
  u8_t temp;

  temp= rom->read(rHL);
  vc.rd++;
  temp&= ~n;
  temp|= (rA & n);
  rom->write(rHL, temp);
  vc.wr++;

  rwas->write(rDE, temp);
  vc.wr++;

  tick(14);
  return resGO;
}

int
cl_r4k::ld_pd_ihtr_hl(class cl_cell32 &dest)
{
  dest.W(read32(rHTR+rHL));
  tick5p1(13);
  return resGO;
}

int
cl_r4k::ld_irr_iird(class cl_cell16 &ir)
{
  u32_t v= 0;
  u16_t a= ir.get();
  i8_t d= fetch();
  a+= d;
  v= (v>>8) + rwas->read(a++);
  v= (v>>8) + rwas->read(a++);
  v= (v>>8) + rwas->read(a++);
  v= (v>>8) + rwas->read(a);
  vc.rd+= 4;
  destIRR()->W(v);
  tick5p1(14);
  return resGO;
}

int
cl_r4k::ld_iird_irr(class cl_cell16 &ir)
{
  u32_t v= cIRR->get();
  u16_t a= ir.get();
  i8_t d= fetch();
  a+= d;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a  , v);
  vc.wr+= 4;
  tick5p1(18);
  return resGO;
}

int
cl_r4k::SBOX_A(t_mem code)
{
  destA().W(sbox_tab[rA]);
  tick(3);
  return resGO;
}

int
cl_r4k::IBOX_A(t_mem code)
{
  destA().W(ibox_tab[rA]);
  tick(3);
  return resGO;
}

int
cl_r4k::PUSH_MN(t_mem code)
{
  u8_t m, n;
  n= fetch();
  m= fetch();
  cSP.W(rSP-1);
  rom->write(rSP, m);
  cSP.W(rSP-1);
  rom->write(rSP, n);
  vc.wr+= 2;
  tick5p1(14);
  return resGO;
}

int
cl_r4k::LD_IRR_iHL(t_mem code)
{
  u32_t v= 0;
  u16_t a= rHL;
  v+= rwas->read(a++);
  v+= rwas->read(a++) << 8;
  v+= rwas->read(a++) << 16;
  v+= rwas->read(a  ) << 24;
  vc.rd+= 4;
  destIRR()->W(v);
  tick(13);
  return resGO;
}

int
cl_r4k::LD_iHL_IRR(t_mem code)
{
  u32_t v= cIRR->get();
  u16_t a= rHL;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a  , v);
  vc.wr+= 4;
  tick(17);
  return resGO;
}

int
cl_r4k::LD_IRR_iSPn(t_mem code)
{
  u32_t v= 0;
  u16_t a= rSP;
  u8_t n= fetch();
  a+= n;
  v+= rwas->read(a++);
  v+= rwas->read(a++) << 8;
  v+= rwas->read(a++) << 16;
  v+= rwas->read(a  ) << 24;
  vc.rd+= 4;
  destIRR()->W(v);
  tick5p1(14);
  return resGO;
}

int
cl_r4k::LD_iSPn_IRR(t_mem code)
{
  u32_t v= cIRR->get();
  u16_t a= rSP;
  u8_t n= fetch();
  a+= n;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a++, v); v>>= 8;
  rwas->write(a  , v);
  vc.wr+= 4;
  tick5p1(18);
  return resGO;
}

int
cl_r4k::POP_IRR(t_mem code)
{
  u32_t v= 0;
  u16_t a= rSP;
  v+= rom->read(a++);
  v+= rom->read(a++) << 8;
  v+= rom->read(a++) << 16;
  v+= rom->read(a++) << 24;
  cSP.W(a);
  destIRR()->W(v);
  tick(12);
  return resGO;
}

int
cl_r4k::PUSH_IRR(t_mem code)
{
  u32_t v= cIRR->get();
  u16_t a= rSP;
  rom->write(--a, v>>24);
  rom->write(--a, v>>16);
  rom->write(--a, v>>8 );
  rom->write(--a, v    );
  cSP.W(a);
  vc.wr+= 4;
  tick5p1(17);
  return resGO;
}

int
cl_r4k::LDF_IRR_iLMN(t_mem code)
{
  u32_t v= 0;
  t_addr a= fetch();
  a<<= 8; a+= fetch();
  a<<= 8; a+= fetch();
  v+= mem->phread(a); a= (a+1)&0xffffff;
  v<<= 8; v+= mem->phread(a); a= (a+1)&0xffffff;
  v<<= 8; v+= mem->phread(a); a= (a+1)&0xffffff;
  v<<= 8; v+= mem->phread(a);
  vc.rd+= 4;
  destIRR()->write(v);
  tick(18);
  return resGO;
}

int
cl_r4k::LDF_iLMN_IRR(t_mem code)
{
  u32_t v= cIRR->get();
  t_addr a= fetch();
  a<<= 8; a+= fetch();
  a<<= 8; a+= fetch();
  mem->phwrite(a++, v>>24); a&= 0xffffff;
  mem->phwrite(a++, v>>16); a&= 0xffffff;
  mem->phwrite(a++, v>>8 ); a&= 0xffffff;
  mem->phwrite(a  , v    );
  vc.wr+= 4;
  tick(22);
  return resGO;
}

int
cl_r4k::LDF_iLMN_HL(t_mem code)
{
  u16_t v= rHL;
  t_addr a= fetch();
  a<<= 8; a+= fetch();
  a<<= 8; a+= fetch();
  mem->phwrite(a++, v>>8 ); a&= 0xffffff;
  mem->phwrite(a  , v    );
  vc.wr+= 2;
  tick5m2(14);
  return resGO;
}

int
cl_r4k::LDF_HL_iLMN(t_mem code)
{
  u16_t v= 0;
  t_addr a= fetch();
  a<<= 8; a+= fetch();
  a<<= 8; a+= fetch();
  v+= mem->phread(a); a= (a+1)&0xffffff;
  v<<= 8; v+= mem->phread(a);
  vc.rd+= 2;
  destHL().W(v);
  tick(12);
  return resGO;
}

int
cl_r4k::LDF_ilmn_A(t_mem code)
{
  u32_t a= fetch();
  a+= fetch()*256;
  a+= fetch()*256*256;
  mem->phwrite(a, rA);
  tick(11);
  return resGO;
}

int
cl_r4k::LDF_A_ilmn(t_mem code)
{
  u32_t a= fetch();
  a+= fetch()*256;
  a+= fetch()*256*256;
  u8_t v= mem->phread(a);
  destA().W(v);
  tick(10);
  return resGO;
}

int
cl_r4k::ldf_pd_ilmn(class cl_cell32 &pd)
{
  u32_t a= fetch();
  a+= fetch()*256;
  a+= fetch()*256*256;
  u32_t v= mem->phread(a++);
  v+= mem->phread(a++)<<8;
  v+= mem->phread(a++)<<16;
  v+= mem->phread(a  )<<24;
  vc.rd+= 4;
  pd.W(v);
  tick(18);
  return resGO;
}

int
cl_r4k::ldf_ilmn_ps(u32_t ps)
{
  u32_t a= fetch();
  a+= fetch()*256;
  a+= fetch()*256*256;
  mem->phwrite(a++, ps);
  mem->phwrite(a++, ps>>8);
  mem->phwrite(a++, ps>>16);
  mem->phwrite(a  , ps>>24);
  vc.wr+= 4;
  tick(22);
  return resGO;
}

int
cl_r4k::ldf_rr_ilmn(class cl_cell16 &rr)
{
  u32_t a= fetch();
  a+= fetch()*256;
  a+= fetch()*256*256;
  u16_t v= mem->phread(a++);
  v+= mem->phread(a++)<<8;
  vc.rd+= 2;
  rr.W(v);
  tick(14);
  return resGO;
}

int
cl_r4k::ldf_ilmn_rr(u16_t rr)
{
  u32_t a= fetch();
  a+= fetch()*256;
  a+= fetch()*256*256;
  mem->phwrite(a++, rr);
  mem->phwrite(a  , rr>>8);
  vc.wr+= 2;
  tick(16);
  return resGO;
}

int
cl_r4k::ld_irr_ips_hl(t_mem code)
{
  class cl_cell32 *ps= &cPW;
  switch (code & 0x30)
    {
    case 0x00: ps= &cPW; break;
    case 0x10: ps= &cPX; break;
    case 0x20: ps= &cPY; break;
    case 0x30: ps= &cPZ; break;
    }
  u32_t p= ps->get();
  u32_t v   = mem->pxread(px16se(p, rHL+0));
  v<<= 8; v+= mem->pxread(px16se(p, rHL+1));
  v<<= 8; v+= mem->pxread(px16se(p, rHL+2));
  v<<= 8; v+= mem->pxread(px16se(p, rHL+3));
  vc.rd+= 4;
  destIRR()->write(v);
  tick5p1(13);
  return resGO;
}


int
cl_r4k::ld_irr_ips_d(t_mem code)
{
  class cl_cell32 *ps= &cPW;
  switch (code & 0x30)
    {
    case 0x00: ps= &cPW; break;
    case 0x10: ps= &cPX; break;
    case 0x20: ps= &cPY; break;
    case 0x30: ps= &cPZ; break;
    }
  u32_t p= ps->get();
  u8_t d= fetch();
  u32_t v   = mem->pxread(px8se(p, d)+0);
  v<<= 8; v+= mem->pxread(px8se(p, d)+1);
  v<<= 8; v+= mem->pxread(px8se(p, d)+2);
  v<<= 8; v+= mem->pxread(px8se(p, d)+3);
  vc.rd+= 4;
  destIRR()->write(v);
  tick5p1(13);
  return resGO;
}

int
cl_r4k::ld_ips_hl_irr(t_mem code)
{
  class cl_cell32 *ps= &cPW;
  switch (code & 0x30)
    {
    case 0x00: ps= &cPW; break;
    case 0x10: ps= &cPX; break;
    case 0x20: ps= &cPY; break;
    case 0x30: ps= &cPZ; break;
    }
  u32_t p= ps->get();
  u32_t v= cIRR->get();
  mem->pxwrite(px16se(p, rHL+0), v);
  mem->pxwrite(px16se(p, rHL+1), v>>8);
  mem->pxwrite(px16se(p, rHL+2), v>>16);
  mem->pxwrite(px16se(p, rHL+3), v>>24);
  vc.wr+= 4;
  tick5p1(17);
  return resGO;
}

int
cl_r4k::ld_ips_d_irr(t_mem code)
{
  class cl_cell32 *ps= &cPW;
  switch (code & 0x30)
    {
    case 0x00: ps= &cPW; break;
    case 0x10: ps= &cPX; break;
    case 0x20: ps= &cPY; break;
    case 0x30: ps= &cPZ; break;
    }
  u32_t p= ps->get();
  u32_t v= cIRR->get();
  u8_t d= fetch();
  mem->pxwrite(px16se(p, d)+0, v);
  mem->pxwrite(px16se(p, d)+1, v>>8);
  mem->pxwrite(px16se(p, d)+2, v>>16);
  mem->pxwrite(px16se(p, d)+3, v>>24);
  vc.wr+= 4;
  tick5p1(18);
  return resGO;
}

int
cl_r4k::ldl_px_ir(t_mem code)
{
  switch (code & 0x30)
    {
    case 0x00: destPW().W(0xffff0000+cIR->get()); break;
    case 0x10: destPX().W(0xffff0000+cIR->get()); break;
    case 0x20: destPY().W(0xffff0000+cIR->get()); break;
    case 0x30: destPZ().W(0xffff0000+cIR->get()); break;
    }
  tick(3);
  return resGO;
}

int
cl_r4k::ldl_px_irrl(t_mem code)
{
  switch (code & 0x30)
    {
    case 0x00: destPW().W(0xffff0000|cIRR->get()); break;
    case 0x10: destPX().W(0xffff0000|cIRR->get()); break;
    case 0x20: destPY().W(0xffff0000|cIRR->get()); break;
    case 0x30: destPZ().W(0xffff0000|cIRR->get()); break;
    }
  tick(3);
  return resGO;
}

int
cl_r4k::ld_px_irr(t_mem code)
{
  switch (code & 0x30)
    {
    case 0x00: cPW.W(cIRR->get()); break;
    case 0x10: cPX.W(cIRR->get()); break;
    case 0x20: cPY.W(cIRR->get()); break;
    case 0x30: cPZ.W(cIRR->get()); break;
    }
  tick(3);
  return resGO;
}

int
cl_r4k::ld_irr_px(t_mem code)
{
  switch (code & 0x30)
    {
    case 0x00: destIRR()->W(rPW); break;
    case 0x10: destIRR()->W(rPX); break;
    case 0x20: destIRR()->W(rPY); break;
    case 0x30: destIRR()->W(rPZ); break;
    }
  tick(3);
  return resGO;
}

int
cl_r4k::LD_iSP_HL_IRR(t_mem code)
{
  u16_t a= rSP + rHL;
  u32_t v= cIRR->get();
  mem->write(a++, v); v>>= 8;
  mem->write(a++, v); v>>= 8;
  mem->write(a++, v); v>>= 8;
  mem->write(a  , v);
  vc.wr+= 4;
  tick5p1(17);
  return resGO;
}

int
cl_r4k::EXP(t_mem code)
{
  u32_t temp;
  temp= rPW; cPW.W(raPW); caPW.W(temp);
  temp= rPX; cPX.W(raPX); caPX.W(temp);
  temp= rPY; cPY.W(raPY); caPY.W(temp);
  temp= rPZ; cPZ.W(raPZ); caPZ.W(temp);
  tick(3);
  return resGO;
}

int
cl_r4k::LD_HTR_A(t_mem code)
{
  cHTR.W(rA);
  tick(3);
  return resGO;
}

int
cl_r4k::LD_A_HTR(t_mem code)
{
  cA.W(rHTR);
  tick(3);
  return resGO;
}

int
cl_r4k::LD_BC_HL(t_mem code)
{
  destBC().W(rHL);
  tick(1);
  return resGO;
}

int
cl_r4k::ld32_imn(u32_t op)
{
  u16_t a= fetch();
  a+= 256*fetch();
  rwas->write(a++, op); op>>= 8;
  rwas->write(a++, op); op>>= 8;
  rwas->write(a++, op); op>>= 8;
  rwas->write(a  , op);
  vc.wr+= 4;
  tick(18);
  return resGO;
}

int
cl_r4k::ld_r32_imn(class cl_cell32 &dest)
{
  u16_t a= fetch();
  a+= 256*fetch();
  u32_t v= 0;
  u8_t b;
  b= rwas->read(a++); v|= b<<0;
  b= rwas->read(a++); v|= b<<8;
  b= rwas->read(a++); v|= b<<16;
  b= rwas->read(a  ); v|= b<<24;
  dest.write(v);
  vc.rd+= 4;
  tick(14);
  return resGO;
}

int
cl_r4k::ld_hl_ipsd(u32_t ps)
{
  u8_t d= fetch();
  u32_t a= px8se(ps, d);
  u8_t h, l;
  l= mem->pxread(a);
  a= px8se(ps, d+1);
  h= mem->pxread(a);
  destHL().W(h*256+l);
  vc.rd+= 2;
  tick5p1(8);
  return resGO;
}

int
cl_r4k::ld_ipsd_hl(u32_t ps)
{
  u8_t d= fetch();
  u32_t a= px8se(ps, d);
  mem->pxwrite(a, rHL);
  a= px8se(ps, d+1);
  mem->pxwrite(a, rHL>>8);
  vc.wr+= 2;
  tick5p1(10);
  return resGO;
}

int
cl_r4k::LD_imn_JK(t_mem code)
{
  u16_t a= fetch();
  a+= fetch()*256;
  rwas->write(a, rK);
  a++;
  rwas->write(a, rJ);
  vc.wr+= 2;
  tick(12);
  return resGO;
}

int
cl_r4k::LD_JK_imn(t_mem code)
{
  u16_t a= fetch(), v;
  a+= fetch()*256;
  v= rwas->read(a);
  v+= rwas->read(++a)*256;
  destJK().W(v);
  vc.rd+= 2;
  tick(10);
  return resGO;
}

int
cl_r4k::ld_a_ipshl(u32_t ps)
{
  u32_t a= px16se(ps, rHL);
  u8_t v= mem->pxread(a);
  vc.rd++;
  destA().W(v);
  tick5p1(5);
  return resGO;
}

int
cl_r4k::ld_ipshl_a(u32_t ps)
{
  u32_t a= px16se(ps, rHL);
  mem->pxwrite(a, rA);
  vc.wr++;
  tick5p1(6);
  return resGO;
}

int
cl_r4k::ld_a_ipsd(u32_t ps)
{
  u8_t d= fetch();
  u32_t a= px8se(ps, d);
  u8_t v= mem->pxread(a);
  vc.rd++;
  destA().W(v);
  tick5p1(6);
  return resGO;
}

int
cl_r4k::ld_ipsd_a(u32_t ps)
{
  u8_t d= fetch();
  u32_t a= px8se(ps, d);
  mem->pxwrite(a, rA);
  vc.wr++;
  tick5p1(7);
  return resGO;
}

int
cl_r4k::LD_LXPC_HL(t_mem code)
{
  LXPC->W(rHL);
  atomic= true;
  tick(1);
  return resGO;
}

int
cl_r4k::LD_HL_LXPC(t_mem code)
{
  destHL().W(rXPC);
  atomic= true;
  tick(1);
  return resGO;
}

int
cl_r4k::LD_BCDE_d(t_mem code)
{
  i8_t d= fetch();
  i32_t v;
  v= d;
  destBCDE().W(v);
  tick(3);
  return resGO;
}

int
cl_r4k::LD_JKHL_d(t_mem code)
{
  i8_t d= fetch();
  i32_t v;
  v= d;
  destJKHL().W(v);
  tick(3);
  return resGO;
}

int
cl_r4k::LD_JK_mn(t_mem code)
{
  u16_t v= fetch();
  v+= fetch()*256;
  destJK().W(v);
  tick(5);
  return resGO;
}

int
cl_r4k::LD_DE_HL(t_mem code)
{
  destDE().W(rHL);
  tick(1);
  return resGO;
}

int
cl_r4k::EX_BC_HL(t_mem code)
{
  class cl_cell16 &hl= destHL();
  u16_t t;
  t= rBC;
  cBC.W(hl.read());
  hl.W(t);
  tick5p12(1);
  return resGO;
}

int
cl_r4k::EX_JKHL_BCDE(t_mem code)
{
  u32_t t;
  t= rJKHL;
  cJKHL.W(rBCDE);
  cBCDE.W(t);
  tick(1);
  return resGO;
}

int
cl_r4k::EX_JK_HL(t_mem code)
{
  class cl_cell16 &hl= destHL();
  u16_t t;
  t= rJK;
  cJK.W(hl.read());
  hl.W(t);
  tick(1);
  return resGO;
}

int
cl_r4k::CLR_HL(t_mem code)
{
  destHL().W(0);
  tick(1);
  return resGO;
}

int
cl_r4k::LD_IRR_iSP_HL(t_mem code)
{
  u32_t v;
  u16_t a= rSP+rHL;
  v= rom->read(a++);
  v+= rom->read(a++)*256;
  v+= rom->read(a++)*256*256;
  v+= rom->read(a)*256*256*256;
  destIRR()->W(v);
  tick5p1(13);
  return resGO;
}

int
cl_r4k::ldl_pd_ispn(class cl_cell32 &pd)
{
  u16_t a= rSP+fetch();
  u16_t v= rom->read(a++);
  v+= rom->read(a)*256;
  vc.rd+= 2;
  pd.W(0xffff0000 + v);
  tick5p1(10);
  return resGO;
}

int
cl_r4k::ld_pd_ispn(class cl_cell32 &pd)
{
  u16_t a= rSP+fetch();
  u32_t v= rom->read(a++);
  v+= rom->read(a++)*256;
  v+= rom->read(a++)*256*256;
  v+= rom->read(a++)*256*256*256;
  vc.rd+= 4;
  pd.W(v);
  tick5p1(14);
  return resGO;
}

int
cl_r4k::ld_ispn_ps(u32_t ps)
{
  u16_t a= rSP + fetch();
  rom->write(a++, ps);
  rom->write(a++, ps>>8);
  rom->write(a++, ps>>16);
  rom->write(a  , ps>>24);
  vc.wr+= 4;
  tick5p1(18);
  return resGO;
}

int
cl_r4k::ld_hl_ipsbc(u32_t ps)
{
  u16_t v;
  u32_t a= px16(ps, rBC);
  v= mem->pxread(a);
  a= px16(ps, rBC+1);
  v+= mem->pxread(a)*256;
  vc.rd+= 2;
  destHL().W(v);
  tick5p1(9);
  return resGO;
}

int
cl_r4k::ld_ipdbc_hl(u32_t pd)
{
  u32_t a= px16(pd, rBC);
  mem->pxwrite(a, rHL);
  a= px16(pd, rBC+1);
  mem->pxwrite(a, rHL>>8);
  vc.wr+= 2;
  tick5p1(11);
  return resGO;
}

int
cl_r4k::ld_pd_klmn(class cl_cell32 &pd)
{
  u32_t v= fetch();
  v+= fetch()<<8;
  v+= fetch()<<16;
  v+= fetch()<<24;
  pd.W(v);
  tick(11);
  return resGO;
}

int
cl_r4k::ldl_pd_mn(class cl_cell32 &pd)
{
  u32_t v= fetch();
  v+= fetch()<<8;
  pd.W(0xffff+v);
  tick(7);
  return resGO;
}

int
cl_r4k::EX_aBC_HL(t_mem code)
{
  class cl_cell16 &hl= destHL();
  u16_t t;
  t= raBC;
  caBC.W(hl.read());
  hl.W(t);
  tick(3);
  return resGO;
}

int
cl_r4k::EX_aJK_HL(t_mem code)
{
  class cl_cell16 &hl= destHL();
  u16_t t;
  t= raJK;
  caJK.W(hl.read());
  hl.W(t);
  tick(3);
  return resGO;
}

int
cl_r4k::copy(int dir)
{
  u8_t v;
  do
    {
      v= mem->phread(rPX);
      mem->phwrite(rPY, v);
      vc.rd++;
      vc.wr++;
      tick(7);
      cPX.W(rPX+dir);
      cPY.W(rPY+dir);
      cBC.W(rBC-1);
    }
  while (rBC);
  tick(5);
  return resGO;
}

int
cl_r4k::pop_pd(class cl_cell32 &pd)
{
  u16_t a= rSP;
  u32_t v;
  v= rom->read(a++);
  v+= rom->read(a++)>>8;
  v+= rom->read(a++)>>16;
  v+= rom->read(a++)>>24;
  vc.rd+= 4;
  pd.W(v);
  cSP.W(a);
  tick(12);
  return resGO;
}

int
cl_r4k::push_ps(u32_t ps)
{
  u16_t a= rSP;
  rom->write(--a, ps>>24);
  rom->write(--a, ps>>16);
  rom->write(--a, ps>>8);
  rom->write(--a, ps);
  vc.wr+= 4;
  cSP.W(a);
  tick5p1(17);
  return resGO; 
}

int
cl_r4k::LD_HL_iSPHL(t_mem code)
{
  u16_t a= rSP + rHL;
  u16_t v= rom->read(a++);
  v+= rom->read(a)*256;
  destHL().W(v);
  tick5p1(9);
  return resGO;
}


/* End of rxk.src/imove.cc */
