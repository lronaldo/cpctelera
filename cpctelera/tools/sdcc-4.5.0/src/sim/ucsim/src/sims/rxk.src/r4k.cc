/*
 * Simulator of microcontrollers (r4k.cc)
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

#include "r4kwrap.h"
#include "7fwrap.h"
#include "glob.h"
#include "gp0m3.h"
#include "gp0m4.h"
#include "gpddm3.h"
#include "gpedm3a.h"
#include "gpddm4.h"
#include "gpedm3.h"
#include "gpedm4.h"

#include "r4kcl.h"


u32_t
px8(u32_t px, u8_t offset)
{
  bool log= ((px & 0xffff0000) == 0xffff0000);
  px+= offset;
  if (log) px|= 0xffff0000;
  return px;
}

u32_t
px8se(u32_t px, u8_t offset)
{
  bool log= ((px & 0xffff0000) == 0xffff0000);
  i32_t o= (i8_t)offset;
  px+= o;
  if (log) px|= 0xffff0000;
  return px;
}

u32_t
px16(u32_t px, u16_t offset)
{
  bool log= ((px & 0xffff0000) == 0xffff0000);
  px+= offset;
  if (log) px|= 0xffff0000;
  return px;
}

u32_t
px16se(u32_t px, u16_t offset)
{
  bool log= ((px & 0xffff0000) == 0xffff0000);
  i32_t o= (i16_t)offset;
  px+= o;
  if (log) px|= 0xffff0000;
  return px;
}


cl_r4k::cl_r4k(class cl_sim *asim):
  cl_r3ka(asim)
{
}

int
cl_r4k::init(void)
{
  cl_r3ka::init();
#define RCV(R) reg_cell_var(&c ## R , &r ## R , "" #R "" , "CPU register " #R "")
  RCV(BCDE);
  RCV(JKHL);
  RCV(aBCDE);
  RCV(aJKHL);
  RCV(J);
  RCV(K);
  RCV(JK);
  RCV(aJ);
  RCV(aK);
  RCV(aJK);
  RCV(PW);
  RCV(PX);
  RCV(PY);
  RCV(PZ);
  RCV(aPW);
  RCV(aPX);
  RCV(aPY);
  RCV(aPZ);
  RCV(HTR);
#undef RCV
  //mode2k();
  fill_7f_wrappers(itab_7f);

  LXPC= new cl_cell16(12);
  reg_cell_var(LXPC, mem->aof_lxpc(),
  	       "LXPC", "MMU register: LXPC");
  cpu->register_cell(LXPC);
  cIRR= &cBCDE;
  
  return 0;
}

const char *
cl_r4k::id_string(void)
{
  return "R4K";
}

void
cl_r4k::reset(void)
{
  cl_r3ka::reset();
  //edmr= 0;
  mode3k();  
}

void
cl_r4k::make_cpu_hw(void)
{
  add_hw(cpu= new cl_r4k_cpu(this));
  cpu->init();
}

static struct dis_entry de7f;

struct dis_entry *
cl_r4k::dis_entry(t_addr addr)
{
  u8_t code= rom->get(addr);
  int i;
  struct dis_entry *dt;
  
  if (code == 0xed)
    {
      code= rom->get(addr+1);
      
      dt= disass_pedm3;
      i= 0;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	return &dt[i];
      
      dt= disass_pedm3a;
      i= 0;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	return &dt[i];
      
      dt= disass_pedm4;
      i= 0;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	return &dt[i];
      
      return NULL;
    }
  if ((code & 0xdd) == 0xdd)
    {
      if (code == 0xdd)
	{
	  cIR= &cIX;
	}
      else
	{
	  cIR= &cIY;
	}
      code= rom->get(addr+1);
      dt= disass_pddm3;
      i= 0;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	return &dt[i];
      dt= disass_pddm4;
      i= 0;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	return &dt[i];
      return NULL;
    }

  if ((code == 0x6d) && (edmr & 0xc0))
    {
      // 6d page exists in 4k mode only!
      return dis_6d_entry(addr);
    }

  if ((code == 0x7f) && (edmr & 0xc0))
    {
      // 7f page is special in 4k mode
      code= rom->get(addr+1);
      if ((code <= 0x3f) || (code >= 0xc0))
	return NULL;
      if ((code >= 0x40) && (code <= 0x6f))
	{
	  if ((code & 0x0f) == 0x06)
	    return NULL;
	  if ((code & 0x0f) == 0x0e)
	    return NULL;
	}
      if ((code & 0xf0) == 0x70)
	{
	  if ((code == 0x7e) || (code <= 0x77))
	    return NULL;
	}
      // pick from standard page0 table
      dt= disass_p0m3;
      i= 0;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	{
	  memcpy(&de7f, &dt[i], sizeof(struct dis_entry));
	  de7f.length++;
	  return &de7f;
	}
      return NULL;
    }
  
  dt= disass_rxk;
  i= 0;
  while (((code & dt[i].mask) != dt[i].code) &&
	 dt[i].mnemonic)
    i++;
  if (dt[i].mnemonic != NULL)
    return &dt[i];

  if (edmr & 0xc0)
    {
      // mode: 4k
      dt= disass_p0m4;
      i= 0;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	return &dt[i];
    }
  else
    {
      // mode: 3k
      dt= disass_p0m3;
      i= 0;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	return &dt[i];
    }
  
  return NULL;
}

struct dis_entry disass_6d[]= {
  /* 0 */ {    0,    0, ' ', 0, NULL },
  /* 1 */ { 0x6d, 0xff, ' ', 2, "LD L,L" },
  /* 2 */ { 0x7f, 0xff, ' ', 2, "LD A,A" },
  /* 3 */ {    0,    0, ' ', 0, 0, 0, 0 }
};
char mnemo[100];

struct dis_entry *
cl_r4k::dis_6d_entry(t_addr addr)
{
  u8_t h, l, code= rom->get(addr+1);
  chars op, idx, offset;
  
  if (code == 0x6d)
    return &disass_6d[1];
  if (code == 0x7f)
    return &disass_6d[2];
  
  h= code >> 4;
  l= code & 15;
  if ((l == 0xd) || (l == 0xf))
    return &disass_6d[3];

  disass_6d[0].length= 2;
  switch (h&3)
    {
    case 0: idx= "PW"; break;
    case 1: idx= "PX"; break;
    case 2: idx= "PY"; break;
    case 3: idx= "PZ"; break;
    }
  switch (h&0xc)
    {
    case 0x0:
      if (l<=3)
	op= "BC";
      else
	op= "PW";
      break;
    case 0x4:
      if (l<=3)
	op= "DE";
      else
	op= "PX";
      break;
    case 0x8: if (l<=3) op= "IX"; else op= "PY"; break;
    case 0xc: if (l<=3) op= "IY"; else op= "PZ"; break;
    }
  switch (l&6)
    {
    case 0:
      {
	u8_t d= rom->get(addr+2);
	disass_6d[0].length= 3;
	if (l&1)
	  offset.format("+%u", d);
	else
	  {
	    i8_t io= d;
	    //offset= (io<0)?"-":"+";
	    offset.format("%+d", io);
	  }
	break;
      }
    case 2: offset= "+HL"; break;
    case 3: offset= (l&1)?"IY":"IX"; break;
    case 6: offset= "";
    }

  chars mn= "LD ";
  if (l&4)
    {
      mn+= op+","+idx+offset;
    }
  else
    {
      if (l&1)
	{
	  mn+= "(";
	  mn+= idx+offset+"),"+op;
	}
      else
	{
	  mn+= op+",("+idx+offset+")";
	}
    }
  strcpy(mnemo, mn.c_str());
  disass_6d[0].mnemonic= mnemo;
  
  return &disass_6d[0];
}

void
cl_r4k::disass_irr(chars *work, bool dd)
{
  work->appendf("%s", dd?"BCDE":"JKHL");
}

void
cl_r4k::disass_irrl(chars *work, bool dd)
{
  work->appendf("%s", dd?"DE":"HL");
}

void
cl_r4k::select_IRR(bool dd)
{
  cIRR = dd?&cBCDE :&cJKHL;
  caIRR= dd?&caBCDE:&caJKHL;
}

void
cl_r4k::print_regs(class cl_console_base *con)
{
  con->dd_color("answer");
  con->dd_printf("A= 0x%02x %3d %c  ",
                 rA, rA, isprint(rA)?rA:'.');
  con->dd_printf("F= "); con->print_bin(rF, 8);
  con->dd_printf(" 0x%02x %3d %c  ", rF, rF, isprint(rF)?rF:'.');
  if (edmr & 0xc0)
    {
      con->dd_color("ui_mkey");
      con->dd_printf("Mode:4k");
    }
  else
    {
      con->dd_color("ui_title");
      con->dd_printf("Mode:3k");
    }
  con->dd_printf("\n");
  con->dd_color("answer");
  con->dd_printf("                  SZxxxVxC\n");

  con->dd_printf("XPC= 0x%03x IP= 0x%02x IIR= 0x%02x EIR= 0x%02x\n",
		 mem->get_xpc(), rIP, rIIR, rEIR);
  
  con->dd_printf("BC= ");
  class cl_dump_ads ads(rBC, rBC+7);
  rom->dump(0, /*rBC, rBC+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("DE= ");
  ads._ss7(rDE);
  rom->dump(0, /*rDE, rDE+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("HL= ");
  ads._ss7(rHL);
  rom->dump(0, /*rHL, rHL+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("IX= ");
  ads._ss7(rIX);
  rom->dump(0, /*rIX, rIX+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("IY= ");
  ads._ss7(rIY);
  rom->dump(0, /*rIY, rIY+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("JK= ");
  ads._ss7(rJK);
  rom->dump(0, /*rJK, rJK+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("SP= ");
  ads._ss7(rSP);
  rom->dump(0, /*rSP, rSP+7*/&ads, 8, con);
  con->dd_color("answer");

  con->dd_printf("aAF= 0x%02x-%02x  ", raA, raF);
  con->dd_printf("aBC= 0x%02x-%02x  ", raB, raC);
  con->dd_printf("aDE= 0x%02x-%02x  ", raD, raE);
  con->dd_printf("aHL= 0x%02x-%02x  ", raH, raL);
  con->dd_printf("aJK= 0x%02x-%02x  ", raJ, raK);
  con->dd_printf("\n");

  con->dd_printf(" PW= 0x%04x-%04x  ", rPW>>16, rPW&0xffff);
  con->dd_printf(" PX= 0x%04x-%04x  ", rPX>>16, rPX&0xffff);
  con->dd_printf(" PY= 0x%04x-%04x  ", rPY>>16, rPY&0xffff);
  con->dd_printf(" PZ= 0x%04x-%04x  ", rPZ>>16, rPZ&0xffff);
  con->dd_printf("\n");
  con->dd_printf("aPW= 0x%04x-%04x  ", raPW>>16, raPW&0xffff);
  con->dd_printf("aPX= 0x%04x-%04x  ", raPX>>16, raPX&0xffff);
  con->dd_printf("aPY= 0x%04x-%04x  ", raPY>>16, raPY&0xffff);
  con->dd_printf("aPZ= 0x%04x-%04x  ", raPZ>>16, raPZ&0xffff);
  con->dd_printf("\n");
    
  print_disass(PC, con);
}

void
cl_r4k::mode3k(void)
{
  itab[0x40]= instruction_wrapper_40;
  itab[0x41]= instruction_wrapper_41;
  itab[0x43]= instruction_wrapper_43;
  itab[0x44]= instruction_wrapper_44;
  itab[0x49]= instruction_wrapper_49;
  itab[0x4a]= instruction_wrapper_4a;
  itab[0x4b]= instruction_wrapper_4b;
  itab[0x52]= instruction_wrapper_52;
  itab[0x53]= instruction_wrapper_53;
  itab[0x58]= instruction_wrapper_58;
  itab[0x59]= instruction_wrapper_59;
  itab[0x5a]= instruction_wrapper_5a;
  itab[0x5c]= instruction_wrapper_5c;
  itab[0x5d]= instruction_wrapper_5d;
  itab[0x64]= instruction_wrapper_64;
  itab[0x67]= instruction_wrapper_67;
  itab[0x68]= instruction_wrapper_68;
  itab[0x69]= instruction_wrapper_69;
  itab[0x6a]= instruction_wrapper_6a;
  itab[0x6b]= instruction_wrapper_6b;
  itab[0x6c]= instruction_wrapper_6c;
  itab[0x80]= instruction_wrapper_80;
  itab[0x88]= instruction_wrapper_88;
  itab[0x90]= instruction_wrapper_90;

  itab[0x45]= instruction_wrapper_45;
  itab[0x48]= instruction_wrapper_48;
  itab[0x4c]= instruction_wrapper_4c;
  itab[0x4d]= instruction_wrapper_4d;

  itab[0x50]= instruction_wrapper_50;
  itab[0x51]= instruction_wrapper_51;
  itab[0x54]= instruction_wrapper_54;
  itab[0x55]= instruction_wrapper_55;

  itab[0x60]= instruction_wrapper_60;
  itab[0x61]= instruction_wrapper_61;
  itab[0x62]= instruction_wrapper_62;
  itab[0x63]= instruction_wrapper_63;
  itab[0x65]= instruction_wrapper_65;
  itab[0x6d]= instruction_wrapper_6d;

  itab[0x7f]= instruction_wrapper_7f;

  itab[0x81]= instruction_wrapper_81;
  itab[0x82]= instruction_wrapper_82;
  itab[0x83]= instruction_wrapper_83;
  itab[0x84]= instruction_wrapper_84;
  itab[0x85]= instruction_wrapper_85;
  itab[0x86]= instruction_wrapper_86;
  itab[0x87]= instruction_wrapper_87;
  itab[0x89]= instruction_wrapper_89;
  itab[0x8a]= instruction_wrapper_8a;
  itab[0x8b]= instruction_wrapper_8b;
  itab[0x8c]= instruction_wrapper_8c;
  itab[0x8d]= instruction_wrapper_8d;
  itab[0x8e]= instruction_wrapper_8e;
  itab[0x8f]= instruction_wrapper_8f;

  itab[0x91]= instruction_wrapper_91;
  itab[0x92]= instruction_wrapper_92;
  itab[0x93]= instruction_wrapper_93;
  itab[0x94]= instruction_wrapper_94;
  itab[0x95]= instruction_wrapper_95;
  itab[0x96]= instruction_wrapper_96;
  itab[0x97]= instruction_wrapper_97;
  itab[0x98]= instruction_wrapper_98;
  itab[0x99]= instruction_wrapper_99;
  itab[0x9a]= instruction_wrapper_9a;
  itab[0x9b]= instruction_wrapper_9b;
  itab[0x9c]= instruction_wrapper_9c;
  itab[0x9d]= instruction_wrapper_9d;
  itab[0x9e]= instruction_wrapper_9e;
  itab[0x9f]= instruction_wrapper_9f;

  itab[0xa0]= instruction_wrapper_a0;
  itab[0xa1]= instruction_wrapper_a1;
  itab[0xa2]= instruction_wrapper_a2;
  itab[0xa3]= instruction_wrapper_a3;
  itab[0xa4]= instruction_wrapper_a4;
  itab[0xa5]= instruction_wrapper_a5;
  itab[0xa6]= instruction_wrapper_a6;
  itab[0xa7]= instruction_wrapper_a7;
  itab[0xa8]= instruction_wrapper_a8;
  itab[0xa9]= instruction_wrapper_a9;
  itab[0xaa]= instruction_wrapper_aa;
  itab[0xab]= instruction_wrapper_ab;
  itab[0xac]= instruction_wrapper_ac;
  itab[0xad]= instruction_wrapper_ad;
  itab[0xae]= instruction_wrapper_ae;

  itab[0xb0]= instruction_wrapper_b0;
  itab[0xb1]= instruction_wrapper_b1;
  itab[0xb2]= instruction_wrapper_b2;
  itab[0xb3]= instruction_wrapper_b3;
  itab[0xb4]= instruction_wrapper_b4;
  itab[0xb5]= instruction_wrapper_b5;
  itab[0xb6]= instruction_wrapper_b6;
  itab[0xb8]= instruction_wrapper_b8;
  itab[0xb9]= instruction_wrapper_b9;
  itab[0xba]= instruction_wrapper_ba;
  itab[0xbb]= instruction_wrapper_bb;
  itab[0xbc]= instruction_wrapper_bc;
  itab[0xbd]= instruction_wrapper_bd;
  itab[0xbe]= instruction_wrapper_be;
  itab[0xbf]= instruction_wrapper_bf;
}

void
cl_r4k::mode4k(void)
{
  itab[0x40]= instruction_wrapper_4knone;
  itab[0x41]= instruction_wrapper_4knone;
  itab[0x43]= instruction_wrapper_4knone;
  itab[0x44]= instruction_wrapper_4knone;
  itab[0x49]= instruction_wrapper_4knone;
  itab[0x4a]= instruction_wrapper_4knone;
  itab[0x4b]= instruction_wrapper_4knone;
  itab[0x52]= instruction_wrapper_4knone;
  itab[0x53]= instruction_wrapper_4knone;
  itab[0x58]= instruction_wrapper_4knone;
  itab[0x59]= instruction_wrapper_4knone;
  itab[0x5a]= instruction_wrapper_4knone;
  itab[0x5c]= instruction_wrapper_4knone;
  itab[0x5d]= instruction_wrapper_4knone;
  itab[0x64]= instruction_wrapper_4knone;
  itab[0x67]= instruction_wrapper_4knone;
  itab[0x68]= instruction_wrapper_4knone;
  itab[0x69]= instruction_wrapper_4knone;
  itab[0x6a]= instruction_wrapper_4knone;
  itab[0x6b]= instruction_wrapper_4knone;
  itab[0x6c]= instruction_wrapper_4knone;
  itab[0x80]= instruction_wrapper_4knone;
  itab[0x88]= instruction_wrapper_4knone;
  itab[0x90]= instruction_wrapper_4knone;
  
  itab[0x45]= instruction_wrapper_4k45;
  itab[0x48]= instruction_wrapper_4k48;
  itab[0x4c]= instruction_wrapper_4k4c;
  itab[0x4d]= instruction_wrapper_4k4d;

  itab[0x50]= instruction_wrapper_4k50;
  itab[0x51]= instruction_wrapper_4k51;
  itab[0x54]= instruction_wrapper_4k54;
  itab[0x55]= instruction_wrapper_4k55;

  itab[0x60]= instruction_wrapper_4k60;
  itab[0x61]= instruction_wrapper_4k61;
  itab[0x62]= instruction_wrapper_4k62;
  itab[0x63]= instruction_wrapper_4k63;
  itab[0x65]= instruction_wrapper_4k65;
  itab[0x6d]= instruction_wrapper_4k6d;

  itab[0x7f]= instruction_wrapper_4k7f;

  itab[0x81]= instruction_wrapper_4k81;
  itab[0x82]= instruction_wrapper_4k82;
  itab[0x83]= instruction_wrapper_4k83;
  itab[0x84]= instruction_wrapper_4k84;
  itab[0x85]= instruction_wrapper_4k85;
  itab[0x86]= instruction_wrapper_4k86;
  itab[0x87]= instruction_wrapper_4k87;
  itab[0x89]= instruction_wrapper_4k89;
  itab[0x8a]= instruction_wrapper_4k8a;
  itab[0x8b]= instruction_wrapper_4k8b;
  itab[0x8c]= instruction_wrapper_4k8c;
  itab[0x8d]= instruction_wrapper_4k8d;
  itab[0x8e]= instruction_wrapper_4k8e;
  itab[0x8f]= instruction_wrapper_4k8f;

  itab[0x91]= instruction_wrapper_4k91;
  itab[0x92]= instruction_wrapper_4k92;
  itab[0x93]= instruction_wrapper_4k93;
  itab[0x94]= instruction_wrapper_4k94;
  itab[0x95]= instruction_wrapper_4k95;
  itab[0x96]= instruction_wrapper_4k96;
  itab[0x97]= instruction_wrapper_4k97;
  itab[0x98]= instruction_wrapper_4k98;
  itab[0x99]= instruction_wrapper_4k99;
  itab[0x9a]= instruction_wrapper_4k9a;
  itab[0x9b]= instruction_wrapper_4k9b;
  itab[0x9c]= instruction_wrapper_4k9c;
  itab[0x9d]= instruction_wrapper_4k9d;
  itab[0x9e]= instruction_wrapper_4k9e;
  itab[0x9f]= instruction_wrapper_4k9f;

  itab[0xa0]= instruction_wrapper_4ka0;
  itab[0xa1]= instruction_wrapper_4ka1;
  itab[0xa2]= instruction_wrapper_4ka2;
  itab[0xa3]= instruction_wrapper_4ka3;
  itab[0xa4]= instruction_wrapper_4ka4;
  itab[0xa5]= instruction_wrapper_4ka5;
  itab[0xa6]= instruction_wrapper_4ka6;
  itab[0xa7]= instruction_wrapper_4ka7;
  itab[0xa8]= instruction_wrapper_4ka8;
  itab[0xa9]= instruction_wrapper_4ka9;
  itab[0xaa]= instruction_wrapper_4kaa;
  itab[0xab]= instruction_wrapper_4kab;
  itab[0xac]= instruction_wrapper_4kac;
  itab[0xad]= instruction_wrapper_4kad;
  itab[0xae]= instruction_wrapper_4kae;

  itab[0xb0]= instruction_wrapper_4kb0;
  itab[0xb1]= instruction_wrapper_4kb1;
  itab[0xb2]= instruction_wrapper_4kb2;
  itab[0xb3]= instruction_wrapper_4kb3;
  itab[0xb4]= instruction_wrapper_4kb4;
  itab[0xb5]= instruction_wrapper_4kb5;
  itab[0xb6]= instruction_wrapper_4kb6;
  itab[0xb8]= instruction_wrapper_4kb8;
  itab[0xb9]= instruction_wrapper_4kb9;
  itab[0xba]= instruction_wrapper_4kba;
  itab[0xbb]= instruction_wrapper_4kbb;
  itab[0xbc]= instruction_wrapper_4kbc;
  itab[0xbd]= instruction_wrapper_4kbd;
  itab[0xbe]= instruction_wrapper_4kbe;
  itab[0xbf]= instruction_wrapper_4kbf;
}

int
cl_r4k::EXX(t_mem code)
{
  u16_t t;

  cl_rxk::EXX(code);
  
  t= rJK;
  cBC.W(raJK);
  caJK.W(t);

  return resGO;
}

int
cl_r4k::PAGE_4K7F(t_mem code)
{
  code= fetch();
  return itab_7f[code](this, code);
}

int
cl_r4k::PAGE_4K6D(t_mem code)
{
  u8_t h, l;
  class cl_memory_cell *op= &cPX, *idx= &cPW;
  t_addr addr;
  
  code= fetch();
  if (code == 0x6d)
    return ld_r_g(destL(), rL);
  if (code == 0x7f)
    return ld_r_g(destA(), rA);
  
  h= code>>4;
  l= code&0xf;
  if ((l == 0xd) || (l == 0xf))
    return resINV;
  
  switch (h&3)
    {
    case 0: idx= &cPW; break;
    case 1: idx= &cPX; break;
    case 2: idx= &cPY; break;
    case 3: idx= &cPZ; break;
    }
  switch (h&0xc)
    {
    case 0x0:
      if (l<=3)
	op= (l&1)?(&cBC):&destBC();
      else
	op= &cPW;
      break;
    case 0x4:
      if (l<=3)
	op= (l&1)?(&cDE):&destDE();
      else
	op= &cPX;
      break;
    case 0x8: if (l<=3) op= &cIX; else op= &cPY; break;
    case 0xc: if (l<=3) op= &cIY; else op= &cPZ; break;
    }
  addr= idx->get();
  switch (l&6)
    {
    case 0:
      {
	u8_t d= fetch();
	if (l&1)
	  {
	    u8_t offset= d;
	    addr= px8(addr, offset);
	  }
	else
	  {
	    i8_t offset= d;
	    addr= px8se(addr, offset);
	  }
	break;
      }
    case 2: addr= px16(addr, rHL); break;
    case 4: addr= px16(addr, (l&1)?rIY:rIX); break;
    case 6: break;
    }

  if (l&4)
    {
      // reg->reg
      op->W(addr);
    }
  else
    {
      // mem rd/wr
      u32_t v;
      if (l&1)
	{
	  v= op->get();
	  // Write
	  mem->pxwrite(addr, v); addr++; v>>= 8; vc.wr++;
	  mem->pxwrite(addr, v); addr++; v>>= 8; vc.wr++;
	  if (op->get_width() > 16)
	    {
	      mem->pxwrite(addr, v); addr++; v>>= 8; vc.wr++;
	      mem->pxwrite(addr, v); addr++; v>>= 8; vc.wr++;
	    }
	}
      else
	{
	  // Read
	  u8_t b;
	  v= 0;
	  b= mem->pxread(addr); addr++; v= (v<<8)|b; vc.rd++;
	  b= mem->pxread(addr); addr++; v= (v<<8)|b; vc.rd++;
	  if (op->get_width() > 16)
	    {
	      b= mem->pxread(addr); addr++; v= (v<<8)|b; vc.rd++;
	      b= mem->pxread(addr); addr++; v= (v<<8)|b; vc.rd++;
	    }
	  op->W(v);
	}
    }
  
  return resGO;
}


/*
 * CPU peripheral for r4k
 */

cl_r4k_cpu::cl_r4k_cpu(class cl_uc *auc):
  cl_rxk_cpu(auc)
{
  r4uc= (class cl_r4k *)auc;
  edmr= new cl_cell8();
}

int
cl_r4k_cpu::init(void)
{
  cl_rxk_cpu::init();

  uc->reg_cell_var(edmr, &(r4uc->edmr),
		   "EDMR", "Enable dual-mode register");
  edmr->add_hw(this);

  stacksegl= register_cell(ruc->ioi, 0x1a,
			   "STACKSEGL", "MMU register STACKSEGL");
  stacksegh= register_cell(ruc->ioi, 0x1b,
			   "STACKSEGH", "MMU register STACKSEGH");
  datasegl= register_cell(ruc->ioi, 0x1e,
			  "DATASEGL", "MMU register DATASEGL");
  datasegh= register_cell(ruc->ioi, 0x1f,
			  "DATASEGH", "MMU register DATASEGH");
  return 0;
}

t_mem
cl_r4k_cpu::read(class cl_memory_cell *cell)
{
  if (cell == edmr)
    return edmr->get();
  return cell->get();
}

void
cl_r4k_cpu::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == edmr)
    {
      if (*val & 0xc0)
	r4uc->mode4k();
      else
	r4uc->mode3k();
    }
  
  if (cell == dataseg)
    {
      (*val)&= 0xff;
      datasegl->set(*val);
      ruc->mem->set_dataseg(datasegh->read() * 256 + *val);
    }
  else if (cell == datasegl)
    {
      (*val)&= 0xff;
      dataseg->set(*val);
      ruc->mem->set_dataseg(datasegh->read() * 256 + *val);
    }
  else if (cell == datasegh)
    {
      (*val)&= 0x0f;
      ruc->mem->set_dataseg((*val) * 256 + datasegl->read());
    }
  
  else if (cell == stackseg)
    {
      (*val)&= 0xff;
      stacksegl->set(*val);
      ruc->mem->set_stackseg(stacksegh->read() * 256 + *val);
    }
  else if (cell == stacksegl)
    {
      (*val)&= 0xff;
      stackseg->set(*val);
      ruc->mem->set_stackseg(stacksegh->read() * 256 + *val);
    }
  else if (cell == stacksegh)
    {
      (*val)&= 0x0f;
      ruc->mem->set_stackseg((*val) * 256 + stacksegl->read());
    }

  else if (cell == segsize)
    {
      (*val)&= 0xff;
      ruc->mem->set_segsize(*val);
    }

  else if (cell == ruc->XPC)
    {
      (*val)&= 0xff;
      ruc->mem->set_xpc(*val);
    }
  else if (cell == r4uc->LXPC)
    {
      (*val)&= 0xfff;
      ruc->mem->set_lxpc(*val);
    }
}

const char *
cl_r4k_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
      //case rxk_cpu_xpc: return "MMU register: XPC";
      //case rxk_cpu_nuof: return "";
    }
  return "Not used";
}


void
cl_r4k_cpu::print_info(class cl_console_base *con)
{
  cl_rxk_cpu::print_info(con);
  con->dd_printf("EDMR    : 0x%02x\n", edmr->read());
}


/* End of rxk.src/r4k.cc */
