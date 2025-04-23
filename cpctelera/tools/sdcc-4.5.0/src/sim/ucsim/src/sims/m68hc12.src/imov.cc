/*
 * Simulator of microcontrollers (imov.cc)
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
CL12::exec_b7(void)
{
  u8_t pb= fetch();
  u8_t ls= pb&0x7, ms=(pb>>4)&0x7, ws, wd;
  class cl_memory_cell *cs, *cd;
  u16_t src;
  
  if (pb & 0x08)
    return resINV;
  cs= tex_cells[ms];
  cd= (ls==3)?(&cTMP2):(tex_cells[ls]);
  ws= cs->get_width();
  wd= cd->get_width();
  src= cs->get();
  if (!(pb & 0x80))
    {
      // transfer
      if (ws == wd)
	;
      else if (ws == 8)
	{
	  // 8bit -> 16bit SEX source
	  src&= 0x00ff;
	  if (src & 0x80)
	    src|= 0xff00;
	}
      else
	// 16bit -> 8bit
	;
      //cd->W(src);
    }
  else
    {
      // exchange
      if (ws == wd)
	{
	  cs->W(cd->get());
	  //cd->W(src);
	}
      else if (ws == 8)
	{
	  // 0L <-> 8
	  src&= 0x00ff;
	  cs->W(cd->get());
	  //cd->W(src);
	}
      else
	{
	  // 16 <-> 8
	  if (ls & 3)
	    cs->W(0xff00 | (cd->get()));
	  else
	    cs->W(cd->get());
	  //cd->W(src);
	}
    }
  if (cd == &cF)
    {
      u8_t ox= rF&flagX, nx=src&flagX;
      src&= ~flagX;
      src|= (ox|nx);
    }
  cd->W(src);
  return resGO;
}

int
CL12::i_psh8(u8_t op)
{
  cSP.W(rSP-1);
  rom->write(rSP, op);
  vc.wr+= 1;
  return resGO;
}

int
CL12::i_pul8(class cl_memory_cell &dest)
{
  u8_t v;
  v= rom->read(rSP);
  cSP.W(rSP+1);
  dest.W(v);
  vc.rd+= 1;
  return resGO;
}

int
CL12::i_psh16(u16_t op)
{
  cSP.W(rSP-1);
  rom->write(rSP, op);
  cSP.W(rSP-1);
  rom->write(rSP, op>>8);
  vc.wr+= 2;
  return resGO;
}

int
CL12::i_pul16(class cl_memory_cell &dest)
{
  u16_t v;
  v= read_addr(rom, rSP);
  cSP.W(rSP+2);
  dest.W(v);
  vc.rd+= 2;
  return resGO;
}

int
CL12::movw_imid(void)
{
  t_addr aof_xb= PC;
  u8_t xb= fetch();
  u8_t ih= fetch();
  u8_t il= fetch();
  int xt= xb_type(xb);
  if ((xt==1) || (xt==3) || (xt==4))
    {
      t_addr a= naddr(&aof_xb, NULL, PC);
      if (xb_PC(xb))
	a+= 2;
      rom->write(a, ih);
      rom->write((a+1)&0xffff, il);
      vc.wr+= 2;
    }
  return resGO;
}

int
CL12::movb_imid(void)
{
  t_addr aof_xb= PC;
  u8_t xb= fetch();
  u8_t i= fetch();
  int xt= xb_type(xb);
  if ((xt==1) || (xt==3) || (xt==4))
    {
      t_addr a= naddr(&aof_xb, NULL, PC);
      if (xb_PC(xb))
	a+= 1;
      rom->write(a, i);
      vc.wr+= 1;
    }
  return resGO;
}

int
CL12::movw_exid(void)
{
  t_addr aof_xb= PC;
  u8_t xb= fetch();
  u8_t eh= fetch();
  u8_t el= fetch();
  int xt= xb_type(xb);
  if ((xt==1) || (xt==3) || (xt==4))
    {
      u8_t h, l;
      u16_t ea= eh*256+el;
      t_addr a= naddr(&aof_xb, NULL, PC);
      if (xb_PC(xb))
	a+= 2;
      h= rom->read(ea);
      l= rom->read(ea+1);
      vc.rd+= 2;
      rom->write(a, h);
      rom->write((a+1)&0xffff, l);
      vc.wr+= 2;
    }
  return resGO;
}

int
CL12::movb_exid(void)
{
  t_addr aof_xb= PC;
  u8_t xb= fetch();
  u8_t eh= fetch();
  u8_t el= fetch();
  int xt= xb_type(xb);
  if ((xt==1) || (xt==3) || (xt==4))
    {
      u8_t v;
      u16_t ea= eh*256+el;
      t_addr a= naddr(&aof_xb, NULL, PC);
      if (xb_PC(xb))
	a+= 2;
      v= rom->read(ea);
      vc.rd++;
      rom->write(a, v);
      vc.wr++;
    }
  return resGO;
}

int
CL12::movw_idid(void)
{
  t_addr aof_xbsrc= PC;
  u8_t xbsrc= fetch();
  t_addr aof_xbdst= PC;
  u8_t xbdst= fetch();
  int xtsrc= xb_type(xbsrc), xtdst= xb_type(xbdst);
  if ((xtsrc==1) || (xtsrc==3) || (xtsrc==4))
    {
      if ((xtdst==1) || (xtdst==3) || (xtdst==4))
	{
	  u8_t v;
	  u16_t asrc= naddr(&aof_xbsrc, NULL, PC);
	  u16_t adst= naddr(&aof_xbdst, NULL, PC);
	  if (xb_PC(xbsrc))
	    asrc+= -1;
	  if (xb_PC(xbdst))
	    adst+= 1;
	  v= rom->read(asrc);
	  rom->write(adst, v);
	  v= rom->read(asrc+1);
	  rom->write(adst+1, v);
	  vc.rd+= 2;
	  vc.wr+= 2;
	}
    }
  return resGO;
}

int
CL12::movb_idid(void)
{
  t_addr aof_xbsrc= PC;
  u8_t xbsrc= fetch();
  t_addr aof_xbdst= PC;
  u8_t xbdst= fetch();
  int xtsrc= xb_type(xbsrc), xtdst= xb_type(xbdst);
  if ((xtsrc==1) || (xtsrc==3) || (xtsrc==4))
    {
      if ((xtdst==1) || (xtdst==3) || (xtdst==4))
	{
	  u8_t v;
	  u16_t asrc= naddr(&aof_xbsrc, NULL, PC);
	  u16_t adst= naddr(&aof_xbdst, NULL, PC);
	  if (xb_PC(xbsrc))
	    asrc+= -1;
	  if (xb_PC(xbdst))
	    adst+= 1;
	  v= rom->read(asrc);
	  rom->write(adst, v);
	  vc.rd++;
	  vc.wr++;
	}
    }
  return resGO;
}

int
CL12::movw_imex(void)
{
  u8_t ih= fetch();
  u8_t il= fetch();
  u8_t eh= fetch();
  u8_t el= fetch();
  u16_t a= eh*256+el;
  
  rom->write(a, ih);
  rom->write(a+1, il);
  vc.wr+= 2;
  
  return resGO;
}

int
CL12::movb_imex(void)
{
  u8_t i= fetch();
  u8_t eh= fetch();
  u8_t el= fetch();
  u16_t a= eh*256+el;
  
  rom->write(a, i);
  vc.wr+= 1;
  
  return resGO;
}

int
CL12::movw_exex(void)
{
  u8_t sh= fetch();
  u8_t sl= fetch();
  u8_t dh= fetch();
  u8_t dl= fetch();
  u16_t sa= sh*256+sl;
  u16_t da= dh*256+dl;
  u8_t v;
  
  v= rom->read(sa);
  rom->write(da, v);
  v= rom->read(sa+1);
  rom->write(da+1, v);
  vc.rd+= 2;
  vc.wr+= 2;
  
  return resGO;
}

int
CL12::movb_exex(void)
{
  u8_t sh= fetch();
  u8_t sl= fetch();
  u8_t dh= fetch();
  u8_t dl= fetch();
  u16_t sa= sh*256+sl;
  u16_t da= dh*256+dl;
  u8_t v;
  
  v= rom->read(sa);
  rom->write(da, v);
  vc.rd++;
  vc.wr++;
  
  return resGO;
}

int
CL12::movw_idex(void)
{
  t_addr aof_xb= PC;
  u8_t xb= fetch();
  u8_t eh= fetch();
  u8_t el= fetch();
  int xt= xb_type(xb);
  if ((xt==1) || (xt==3) || (xt==4))
    {
      u8_t h, l;
      u16_t ea= eh*256+el;
      u16_t a= naddr(&aof_xb, NULL, PC);
      if (xb_PC(xb))
	a+= -2;
      h= rom->read(a);
      l= rom->read(a+1);
      vc.rd+= 2;
      rom->write(ea, h);
      rom->write(ea+1, l);
      vc.wr+= 2;
    }
  return resGO;
}

int
CL12::movb_idex(void)
{
  t_addr aof_xb= PC;
  u8_t xb= fetch();
  u8_t eh= fetch();
  u8_t el= fetch();
  int xt= xb_type(xb);
  if ((xt==1) || (xt==3) || (xt==4))
    {
      u8_t v;
      u16_t ea= eh*256+el;
      u16_t a= naddr(&aof_xb, NULL, PC);
      if (xb_PC(xb))
	a+= -2;
      v= rom->read(a);
      vc.rd++;
      rom->write(ea, v);
      vc.wr++;
    }
  return resGO;
}


/* End of m68hc12.src/imov.cc */
