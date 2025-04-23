/*
 * Simulator of microcontrollers (m68hc11.cc)
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
/*@1@*/

#include <stdlib.h>
#include <ctype.h>

#include "globals.h"
#include "utils.h"

#include "dregcl.h"
#include "g11p0.h"
#include "g11p18.h"
#include "g11p1a.h"
#include "g11pcd.h"
#include "wraps.h"

#include "m68hc11cl.h"


instruction_wrapper_fn itab18[256];

i8_t p0ticks11[256]= {
  /*      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
  /* 0 */ 0, 2,40,40, 3, 3, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2,
  /* 1 */ 2, 2, 6, 6, 6, 6, 2, 2, 0, 2, 0, 2, 7, 7, 7, 7,
  /* 2 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  /* 3 */ 3, 3, 4, 4, 3, 3, 3, 3, 5, 5, 3,12, 4, 3,14,14,
  /* 4 */ 2, 0, 0, 2, 2, 0, 2, 2, 2, 2, 2, 0, 2, 2, 0, 2,
  /* 5 */ 2, 0, 0, 2, 2, 0, 2, 2, 2, 2, 2, 0, 2, 2, 0, 2,
  /* 6 */ 6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 6, 3, 6,
  /* 7 */ 6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 6, 3, 6,
  /* 8 */ 2, 2, 2, 4, 2, 2, 2, 0, 2, 2, 2, 2, 4, 6, 3, 3,
  /* 9 */ 3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4,
  /* a */ 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
  /* b */ 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
  /* c */ 2, 2, 2, 4, 2, 2, 2, 0, 2, 2, 2, 2, 3, 0, 3, 2,
  /* d */ 3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
  /* e */ 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
  /* f */ 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
};

i8_t p18ticks11[256]= {
  /*      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
  /* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0,
  /* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8,
  /* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 3 */ 4, 0, 0, 0, 0, 4, 0, 0, 6, 0, 4, 0, 5, 0, 0, 0,
  /* 4 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 5 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 6 */ 7, 0, 0, 7, 7, 0, 7, 7, 7, 7, 7, 0, 7, 7, 4, 7,
  /* 7 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 4,
  /* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0,
  /* a */ 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 6, 6,
  /* b */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0,
  /* c */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0,
  /* d */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5,
  /* e */ 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 5, 6,
  /* f */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 6
};

i8_t pcdticks11[256]= {
  /*      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
  /* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 3 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 4 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 5 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 6 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 7 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* a */ 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
  /* b */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* c */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* d */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* e */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6,
  /* f */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

i8_t p1aticks11[256]= {
  /*      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
  /* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 3 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 4 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 5 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 6 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 7 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 8 */ 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 9 */ 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* a */ 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
  /* b */ 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* c */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* d */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* e */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0,
  /* f */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0
};


int
cl_m68hcbase::init(void)
{
  cl_m6800::init();
  
#define RCV(R) reg_cell_var(&c ## R , &r ## R , "" #R "" , "CPU register " #R "")
  RCV(IY);
  RCV(D);
#undef RCV

  cIY.name= "Y";
  
  fill_def_18_wrappers(itab18);
  // un-wrap some codes, where m6800 version is capable to handle
  // indexing with IY
#define UW(code) itab18[0x ## code ]= instruction_wrapper_ ## code
  UW(38); UW(3a); UW(3c);
  UW(60); UW(63); UW(64); UW(66); UW(67); UW(68);
  UW(69); UW(6a); UW(6c); UW(6d); UW(6e); UW(6f);
  UW(8f);
  UW(a0); UW(a1); UW(a2); UW(a3); UW(a4); UW(a5); UW(a6); UW(a7); 
  UW(a8); UW(a9); UW(aa); UW(ab);         UW(ad); UW(ae); UW(af); 
  UW(e0); UW(e1); UW(e2); UW(e3); UW(e4); UW(e5); UW(e6); UW(e7); 
  UW(e8); UW(e9); UW(ea); UW(eb); UW(ec); UW(ed);
  UW(1c); UW(1d); UW(1e); UW(1f);
#undef UW
  return 0;
}

void
cl_m68hcbase::print_regs(class cl_console_base *con)
{
  con->dd_color("answer");
  con->dd_printf("A= $%02x %3d %+4d %c  ", rA, rA, (i8_t)rA, isprint(rA)?rA:'.');
  con->dd_printf("B= $%02x %3d %+4d %c  ", rB, rB, (i8_t)rB, isprint(rB)?rB:'.');
  con->dd_printf("D= $%04x %5d %+5d ", rD, rD, (i16_t)rD);
  con->dd_printf("\n");
  con->dd_printf("CC= "); con->print_bin(rF, 8); con->dd_printf("\n");
  con->dd_printf("      HINZVC\n");

  con->dd_printf("IX= ");
  class cl_dump_ads ads(IX, IX+7);
  rom->dump(0, /*IX, IX+7*/&ads, 8, con);
  con->dd_color("answer");
  
  con->dd_printf("IY= ");
  ads._ss(IY, IY+7);
  rom->dump(0, /*IY, IY+7*/&ads, 8, con);
  con->dd_color("answer");
  
  con->dd_printf("SP= ");
  ads._ss(SP, SP+7);
  rom->dump(0, /*SP, SP+7*/&ads, 8, con);
  con->dd_color("answer");
  
  print_disass(PC, con);
}


cl_m68hc11::cl_m68hc11(class cl_sim *asim):
  cl_m68hcbase(asim)
{
  cCC.decode(&CC);
}


int
cl_m68hc11::init(void)
{
  cl_m68hcbase::init();
  
  //set_xtal(8000000);
  
  return 0;
}


const char *
cl_m68hc11::id_string(void)
{
  return "M68HC11";
}

void
cl_m68hc11::reset(void)
{
  cl_m68hcbase::reset();
}

i8_t *
CL11::tick_tab(t_mem code)
{
  if (code == 0x18)
    return p18ticks11;
  else if (code == 0x1a)
    return p1aticks11;
  else if (code == 0xcd)
    return pcdticks11;
  return p0ticks11;
}

int
CL11::tickt(t_mem code)
{
  i8_t *tt= tick_tab(code);
  if ((code == 0x18) ||
      (code == 0x1a) ||
      (code == 0xcd))
    code= rom->read(PC);
  if (tt == NULL)
    return tick(1);
  int t= tt[code];
  if (t)
    return tick(t);
  return 0;
}


/*
struct dis_entry *
cl_m68hc11::dis_tbl(void)
{
  return disass_m68hc11;
}
*/

struct dis_entry *
cl_m68hc11::get_dis_entry(t_addr addr)
{
  struct dis_entry *dt= dis_tbl();//, *dis_e;
  int i= 0;
  t_mem code= rom->get(addr);

  if (code == 0x18)
    {
      cI= &cY;
      i= 0;
      code= rom->get(addr+1);
      dt= disass11p18;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic)
	return &dt[i];
      return &dt[i];
    }
  
  if (code == 0xcd)
    {
      cI= &cY;
      i= 0;
      code= rom->get(addr+1);
      dt= disass11pcd;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic)
	return &dt[i];
      return &dt[i];
    }
  
  if (code == 0x1a)
    {
      cI= &cX;
      i= 0;
      code= rom->get(addr+1);
      dt= disass11p1a;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic)
	return &dt[i];
      return &dt[i];
    }
  
  if (dt == NULL)
    return NULL;
  while (((code & dt[i].mask) != dt[i].code) &&
	 dt[i].mnemonic)
    i++;
  if (dt[i].mnemonic)
    return &dt[i];

  i= 0;
  dt= disass11p0;
  while (((code & dt[i].mask) != dt[i].code) &&
	 dt[i].mnemonic)
    i++;
  if (dt[i].mnemonic)
    return &dt[i];

  return &dt[i];
}
/*
char *
cl_m68hc11::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars();
  const char *b;
  //t_mem code= rom->get(addr);
  struct dis_entry *dis_e;
  int i;
  bool first;
  
  if ((dis_e= get_dis_entry(addr)) == NULL)
    return NULL;
  if (dis_e->mnemonic == NULL)
    return strdup("-- UNKNOWN/INVALID");
  b= dis_e->mnemonic;

  first= true;
  work= "";
  for (i=0; b[i]; i++)
    {
      if ((b[i] == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (b[i] == '%')
	{
	  t_addr a;
	  u8_t h, l;
	  i++;
	  temp= "";
	  switch (b[i])
	    {
	    case 'x': case 'X': // indexed
	      h= rom->read(addr+1);
	      a= rX+h;
	      work.appendf("$%02x,X", h);
	      //add_spaces(&work, 20);
	      if (b[i]=='x')
		temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      else
		temp.appendf("; [$%04x]=$%04x", a, read_addr(rom, a));
	      break;
	    case 'e': case 'E': // extended
	      h= rom->read(addr+1);
	      l= rom->read(addr+2);
	      a= h*256 + l;
	      work.appendf("$%04x", a);
	      //add_spaces(&work, 20);
	      if (b[i]=='e')
		temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      else
		temp.appendf("; [$%04x]=$%04x", a,
			     read_addr(rom, a));
	      break;
	    case 'd': case 'D': // direct
	      h= a= rom->read(addr+1);
	      work.appendf("$00%02x", h);
	      //add_spaces(&work, 20);
	      if (b[i]=='d')
		temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      else
	        temp.appendf("; [$%04x]=$%04x", a,
			     read_addr(rom, a));
	      break;
	    case 'b': // immediate 8 bit
	      work.appendf("#$%02x",
			   rom->read(addr+1));
	      break;
	    case 'B': // immediate 16 bit
	      work.appendf("#$%04x",
			   read_addr(rom, addr+1));
	      break;
	    case 'r': // relative
	      work.appendf("$%04x",
			   (addr+2+(i8_t)(rom->read(addr+1))) & 0xffff );
	      break;
	    }
	  //work+= temp;
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }

  return(strdup(work.c_str()));
}
*/

int
CL11::PAGE18(t_mem code)
{
  cI= &cIY;
  code= fetch();
  return itab18[code](this, code);
}

int
CL11::PAGE1A(t_mem code)
{
  cI= &cX;
  code= fetch();
  switch (code)
    {
    case 0x83: return cp16(cD, i16()); break;
    case 0x93: return cp16(cD, dop16()); break;
    case 0xa3: return cp16(cD, iop16()); break;
    case 0xb3: return cp16(cD, eop16()); break;

    case 0xad: return cp16(cY, iop16()); break;
    case 0xee: return ldsx(cY, iop16()); break;
    case 0xef: return stsx(iaddr(), rY); break;
    }
  return resINV;
}

int
CL11::PAGECD(t_mem code)
{
  cI= &cY;
  code= fetch();
  switch (code)
    {
    case 0xa3: return cp16(cD, iop16()); break;

    case 0xad: return cp16(cX, iop16()); break;
    case 0xee: return ldsx(cX, iop16()); break;
    case 0xef: return stsx(iaddr(), rX); break;
    }
  return resINV;
}


/* 
 * OTHER instructions
 */

int
CL11::TEST(t_mem code)
{
  return resGO;
}


int
CL11::STOP(t_mem code)
{
  if (!(rF & flagN))
    state= stIDLE;
  return resGO;
}


/*
 * MOVE instructions
 */

int
CL11::ldd16(u16_t op)
{
  u8_t f= rF & ~(flagN|flagZ);
  cD.W(op);
  if (!op) f|= flagZ;
  if (op & 0x8000) f|= flagN;
  cF.W(f);
  return resGO;
}


int
CL11::std16(t_addr addr)
{
  u8_t f= rF & ~(flagN|flagZ);
  rom->write(addr, rA);
  rom->write(u16_t(addr+1), rB);
  vc.wr+= 2;
  if (!rD) f|= flagZ;
  if (rD & 0x8000) f|= flagN;
  cF.W(f);
  return resGO;
}


int
CL11::PULxy(t_mem code)
{
  u8_t h, l;
  h= rom->read(++rSP);
  l= rom->read(++rSP);
  cSP.W(rSP);
  cI->W(h*256+l);
  vc.rd+= 2;
  return resGO;
}


int
CL11::PSHxy(t_mem code)
{
  u16_t r= cI->get();
  rom->write(rSP--, r);
  rom->write(rSP--, r>>8);
  cSP.W(rSP);
  vc.wr+= 2;
  return resGO;
}


int
CL11::XGDxy(t_mem code)
{
  u16_t t= rD;
  cD.W(cI->get());
  cI->W(t);
  return resGO;
}


int
CL11::TSY(t_mem code)
{
  cIY.W(rSP+1);
  return resGO;
}


int
CL11::TYS(t_mem code)
{
  cSP.W(rIY-1);
  return resGO;
}


/*
 * ALU instructions
 */

int
CL11::sub16(class cl_cell16 &dest, u16_t op, bool c)
{
  u8_t orgc= rF&flagC;
  u8_t f= rF & ~(flagN|flagZ|flagV|flagC);
  u16_t a= dest.R(), b= op, r;
  r= a-b;
  if (c && orgc) r--;
  if (r&0x8000) f|= flagN;
  if (!r) f|= flagZ;
  if (( (~a&b)|(b&r)|(r&~a) ) & 0x8000) f|= flagC;
  if (( (a&~b&~r)|(~a&b&r) ) & 0x8000) f|= flagV;
  dest.W(r);
  cF.W(f);
  return resGO;
}


int
CL11::add16(class cl_cell16 &dest, u16_t op, bool c)
{
  u8_t orgc= rF&flagC;
  u8_t f= rF & ~(flagN|flagZ|flagV|flagC);
  u16_t a= dest.R(), b= op, r;
  r= a+b;
  if (c && orgc) r++;
  if (r&0x8000) f|= flagN;
  if (!r) f|= flagZ;
  if (( (a&b)|(b&~r)|(~r&a) ) & 0x8000) f|= flagC;
  if (( (a&b&~r)|(~a&~b&r) ) & 0x8000) f|= flagV;
  dest.W(r);
  cF.W(f);
  return resGO;
}


int
CL11::bset(class cl_memory_cell &dest)
{
  u8_t m= fetch();
  u8_t r= dest.R();
  u8_t f= rF & ~(flagN|flagZ);
  r|= m;
  dest.W(r);
  if (!r) f|= flagZ;
  if (r & 0x80) f|= flagN;
  cF.W(f);
  return resGO;
}


int
CL11::bclr(class cl_memory_cell &dest)
{
  u8_t m= fetch();
  u8_t r= dest.R();
  u8_t f= rF & ~(flagN|flagZ|flagV);
  r&= ~m;
  dest.W(r);
  if (!r) f|= flagZ;
  if (r & 0x80) f|= flagN;
  cF.W(f);
  return resGO;
}


int
CL11::cp16(class cl_cell16 &dest, u16_t op)
{
  u16_t a= dest.get(), b= op, r;
  u8_t f= rF & ~(flagC|flagN|flagZ|flagV);
  r= a-b;
  if (r&0x8000) f|= flagN;
  if (!r) f|= flagZ;
  if (( (a&~b&~r)|(~a&b&r) ) & 0x8000) f|= flagV;
  if (( (~a&b)|(b&r)|(~a&r) ) & 0x8000) f|= flagC;
  cCC.W(f);
  return resGO;
}


int
CL11::IDIV(t_mem code)
{
  u16_t q, r;
  u8_t f= rF & ~(flagZ|flagV|flagC);
  if (rX == 0)
    {
      q= 0xffff;
      r= rX;
      f|= flagC;
    }
  else
    {
      cX.W(q= rD/rX);
      cD.W(r= rD%rX);
    }
  if (!q) f|= flagZ;
  cF.W(f);
  return resGO;
}


int
CL11::FDIV(t_mem code)
{
  u32_t n= rD << 16;
  u16_t q, r;
  u8_t f= rF & ~(flagZ|flagV|flagC);
  if( rX <= rD) f|= flagV;
  if (rX == 0)
    {
      q= 0xffff;
      r= rX;
      f|= flagC;
    }
  else
    {
      cX.W(q= n/rX);
      cD.W(r= n%rX);
    }
  if (!q) f|= flagZ;
  cF.W(f);
  return resGO;
}


int
CL11::LSRD(t_mem code)
{
  u8_t newc= rB&1;
  u8_t f= rF & ~(flagN|flagZ|flagV|flagC);
  cD.W(rD>>1);
  if (!rD) f|= flagZ;
  if (newc) f|= (flagC|flagV);
  cF.W(f);
  return resGO;
}


int
CL11::ASLD(t_mem code)
{
  u8_t newc= rA&0x80;
  u8_t f= rF & ~(flagN|flagZ|flagV|flagC);
  cD.W(rD<<1);
  if (!rD) f|= flagZ;
  if (newc) f|= flagC;
  if (rA&0x80) f|= flagN;
  if ((f ^ (f<<3)) & flagN) f|= flagV;
  cF.W(f);
  return resGO;
}


int
CL11::ABxy(t_mem code)
{
  cI->W(rX+rB);
  return resGO;
}


int
CL11::MUL(t_mem code)
{
  u8_t f= rF & ~flagC;
  u16_t r= rA * rB;
  cD.W(r);
  if (rB & 0x80) f|= flagC;
  cF.W(f);
  return resGO;
}


int
CL11::INY(t_mem code)
{
  if (++rY)
    rF&= ~mZ;
  else
    rF|= mZ;
  cY.W(rY);
  cF.W(rF);
  return resGO;
}


int
CL11::DEY(t_mem code)
{
  if (--rY)
    rF&= ~mZ;
  else
    rF|= mZ;
  cY.W(rY);
  cF.W(rF);
  return resGO;
}


/*
 * BRANCH instructions
 */

int
CL11::brset(u8_t op)
{
  u8_t m= fetch();
  i8_t r= fetch();
  if ((~op & m) == 0)
    branch(PC+r, true);
  return resGO;
}


int
CL11::brclr(u8_t op)
{
  u8_t m= fetch();
  i8_t r= fetch();
  if ((op & m) == 0)
    branch(PC+r, true);
  return resGO;
}


int
CL11::BRN(t_mem code)
{
  fetch();
  return resGO;
}
  

/* End of motorola.src/m68hc11.cc */
