/*
 * Simulator of microcontrollers (rxk.cc)
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

#include "glob.h"
#include "gp0m3.h"
#include "gpedm3.h"
#include "gpddm3.h"
#include "rmemcl.h"
#include "ddwrap.h"
#include "edwrap.h"

#include "rxkcl.h"


cl_rxk_base::cl_rxk_base(class cl_sim *asim):
  cl_uc(asim)
{
}

cl_rxk::cl_rxk(class cl_sim *asim):
  cl_rxk_base(asim)
{
  altd= 0;
  cRtab[0]= &cB;
  cRtab[1]= &cC;
  cRtab[2]= &cD;
  cRtab[3]= &cE;
  cRtab[4]= &cH;
  cRtab[5]= &cL;
  cRtab[7]= &cA;
  caRtab[0]= &caB;
  caRtab[1]= &caC;
  caRtab[2]= &caD;
  caRtab[3]= &caE;
  caRtab[4]= &caH;
  caRtab[5]= &caL;
  caRtab[7]= &caA;
}

int
cl_rxk::init(void)
{
  cl_uc::init();
  altd= prefix= false;
  fill_def_wrappers(itab);
  fill_dd_wrappers(itab_dd);
  fill_ed_wrappers(itab_ed);
  
  //set_xtal(1000000);

#define RCV(R) reg_cell_var(&c ## R , &r ## R , "" #R "" , "CPU register " #R "")
  RCV(A);
  RCV(aA);
  RCV(F);
  RCV(aF);
  RCV(AF);
  RCV(aAF);
  RCV(B);
  RCV(aB);
  RCV(C);
  RCV(aC);
  RCV(BC);
  RCV(aBC);
  RCV(D);
  RCV(aD);
  RCV(E);
  RCV(aE);
  RCV(DE);
  RCV(aDE);
  RCV(H);
  RCV(aH);
  RCV(L);
  RCV(aL);
  RCV(HL);
  RCV(aHL);

  RCV(IX);
  RCV(IY);
  RCV(SP);

  RCV(IP);
  RCV(IIR);
  RCV(EIR);
#undef RCV

  XPC= new cl_cell8(8);
  reg_cell_var(XPC, mem->aof_xpc(),
  	       "XPC", "MMU register: XPC");
  cpu->register_cell(XPC);
  
  cIR= &cIX;
  
  ioi->set(0x11, 0); // stackseg
  ioi->set(0x12, 0); // dataseg
  ioi->set(0x13, 0xff); // segsize
  mem->re_decode();
  
  return 0;
}


const char *
cl_rxk::id_string(void)
{
  return "RXK";
}

void
cl_rxk::reset(void)
{
  cl_uc::reset();

  // MMU reset
  mem->set_dataseg(0);
  mem->set_segsize(0xff);
  mem->set_stackseg(0);
  mem->set_xpc(0);

  rIP= 0xff;
  rIIR= 0;
  rEIR= 0;
  rSP= 0;
}

  
void
cl_rxk::set_PC(t_addr addr)
{
  PC= addr;
}

void
cl_rxk::mk_hw_elements(void)
{
  class cl_hw *h;
  
  cl_uc::mk_hw_elements();

  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();
}

void
cl_rxk::make_cpu_hw(void)
{
  add_hw(cpu= new cl_rxk_cpu(this));
  cpu->init();
}

void
cl_rxk::make_memories(void)
{
  class cl_memory_chip *chip;
  class cl_address_space *as;
  class cl_address_decoder *ad;

  chip= new cl_chip8("rom_chip", chip_size(), 8, 0);
  chip->init();
  memchips->add(chip);

  rwas= rom= as= mem= new cl_ras("rom", chip);
  as->init();
  address_spaces->add(as);

  /* IO */
  ioi= as= new cl_address_space("ioi", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  chip= new cl_chip8("ioi_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as,
			     chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  ioe= as= new cl_address_space("ioe", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  chip= new cl_chip8("ioe_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as,
			     chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}

/*
struct dis_entry *
cl_rxk::dis_tbl(void)
{
  return(disass_rxk);
}
*/

struct dis_entry *
cl_rxk::dis_entry(t_addr addr)
{
  u8_t code= rom->get(addr);
  int i;
  struct dis_entry *dt;
  i= 0;

  if (code == 0xed)
    {
      dt= disass_pedm3;
      code= rom->get(addr+1);
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
      dt= disass_pddm3;
      code= rom->get(addr+1);
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      if (dt[i].mnemonic != NULL)
	return &dt[i];
      return NULL;
    }
  
  dt= disass_rxk;
  while (((code & dt[i].mask) != dt[i].code) &&
	 dt[i].mnemonic)
    i++;
  if (dt[i].mnemonic != NULL)
    return &dt[i];

  dt= disass_p0m3;
  i= 0;
  while (((code & dt[i].mask) != dt[i].code) &&
	 dt[i].mnemonic)
    i++;
  if (dt[i].mnemonic != NULL)
    return &dt[i];
  
  return &dt[i];
}

char *
cl_rxk::disassc(t_addr addr, chars *comment)
{
  chars work, temp;
  const char *b, *nIR;
  t_mem code= rom->get(addr);
  struct dis_entry *dt;//= dis_tbl();//, *dis_e;
  int i;
  bool first, dd;
  unsigned int x, h, l;
  //t_addr start_at= addr;
  
  if (code == 0xcb)
    return disassc_cb(addr, comment);
  
  dt= dis_entry(addr);
  if (code == 0xed)
    code= rom->get(++addr);
  else if ((code == 0xdd) || (code == 0xfd))
    {
      code= rom->get(++addr);
      if (code == 0xcb)
	return disassc_dd_cb(addr-1, comment);
    }
  if (!dt)
    return strdup("-- unknown");

  if (dt->mnemonic == NULL)
    return strdup("-- UNKNOWN/INVALID");
  b= dt->mnemonic;

  nIR= (cIR==&cIX)?"IX":"IY";
  dd = (cIR==&cIX);
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
	  temp= "";
	  i++;
	  switch (b[i])
	    {
	    case 'l': // 24 bit unsigned
	      l= rom->get(++addr);
	      h= rom->get(++addr);
	      x= rom->get(++addr);
	      work.appendf("0x%06x", x*0x10000+h*256+l);
	      break;
	    case 'w': // 16 bit unsigned
	      l= rom->get(++addr);
	      h= rom->get(++addr);
	      work.appendf("0x%04x", h*256+l);
	      break;
	    case 'b': // 8 bit unsigned
	      l= rom->get(++addr);
	      work.appendf("0x%02x", l);
	      break;
	    case 'd': // 8 bit signed
	      {
		i8_t r= rom->read(++addr);
		work.appendf("%+d", r);
	      }
	      break;
	    case 'r': // 8 bit relative jump
	      {
		i8_t r= rom->get(++addr);
		t_addr a= addr + 1 + r;
		work.appendf("0x%04x", AU16(a));
	      }
	      break;
	    case 'R': // 16 bit signed PC relative
	      {
		u8_t el, eh;
		el= rom->get(++addr);
		eh= rom->get(++addr);
		i16_t ee= eh*256+el;
		t_addr a= (addr+1 + ee) & 0xffff;
		work.appendf("0x%04x", a);
		break;
	      }
	    case 'I':
	      work.appendf("%s", nIR);
	      break;
	    case '3':
	      disass_irr(&work, dd);
	      break;
	    case 'n':
	      disass_irrl(&work, dd);
	      break;
	    case 'J':
	      if (cIR == &cIX)
		work.append("HL");
	      else
		work.append("IY");
	      break;
	    case 'X': // LXPC 0 1 2 3 xpl xph
	      {
		u16_t x;
		x= rom->read(addr+4) * 256;
		x+= rom->read(addr+3);
		work.appendf("0x%03x", x);
		break;
	      }
	    case 'x': // 32 bit unsigned
	      {
		u32_t n, m, l, h;
		n= rom->get(++addr);
		m= rom->get(++addr);
		l= rom->get(++addr);
		h= rom->get(++addr);
		work.appendf("0x%08x", (h<<24)+(l<<16)+(m<<8)+n);
	      }
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }

  return(strdup(work.c_str()));
}

char *
cl_rxk::disassc_cb(t_addr addr, chars *comment)
{
  u8_t code= rom->get(++addr);
  u8_t x, y, z;
  chars work, temp;
  char b[100];
  bool first= true;
  int i;
  const char *r[8]= { "B","C","D","E","H","L","(HL)","A" };

  x= code>>6;
  y= (code>>3)&7;
  z= code&7;
  switch (x)
    {
    case 0:
      switch (y)
	{
	case 0: strcpy(b, "RLC %r"); break;
	case 1: strcpy(b, "RRC %r"); break;
	case 2: strcpy(b, "RL %r"); break;
	case 3: strcpy(b, "RR %r"); break;
	case 4: strcpy(b, "SLA %r"); break;
	case 5: strcpy(b, "SRA %r"); break;
	case 6: return strdup("-- UNKNOWN/INVALID");
	case 7: strcpy(b, "SRL %r"); break;
	}
      break;
    case 1:
      strcpy(b, "BIT %y,%r");
      break;
    case 2:
      strcpy(b, "RES %y,%r");
      break;
    case 3:
      strcpy(b, "SET %y,%r");
      break;
    }
  temp= "";
  work= "";
  for (i= 0; b[i]; i++)
    {
      if ((b[i] == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (b[i] == '%')
	{
	  temp= "";
	  i++;
	  switch (b[i])
	    {
	    case 'y': work.appendf("%d", y); break;
	    case 'r': work.append(r[z]); break;
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }

  return(strdup(work.c_str()));
}

char *
cl_rxk::disassc_dd_cb(t_addr addr, chars *comment)
{
  u8_t d, code;
  u8_t x, y, z;
  chars work, temp;
  char b[100];
  bool first= true;
  int i;
  t_addr a;
  
  a= addr+2;
  d= rom->get(a);
  a++;
  code= rom->get(a);
  
  x= code>>6;
  y= (code>>3)&7;
  z= code&7;
  if ((z != 6) || (code == 0x36))
    {
      temp.appendf("-- dd/cb (addr=%x,d=%02x,code=%02x,a=%x) UNKNOWN/INVALID", addr, d, code, a);
      return strdup(temp.c_str());
    }

  switch (x)
    {
    case 0:
      switch (y)
	{
	case 0: strcpy(b, "RLC (%I+%d)"); break;
	case 1: strcpy(b, "RRC (%I+%d)"); break;
	case 2: strcpy(b, "RL (%I+%d)"); break;
	case 3: strcpy(b, "RR (%I+%d)"); break;
	case 4: strcpy(b, "SLA (%I+%d)"); break;
	case 5: strcpy(b, "SRA (%I+%d)"); break;
	case 6: return strdup("-- dd/cb UNKNOWN/INVALID");
	case 7: strcpy(b, "SRL (%I+%d)"); break;
	}
      break;
    case 1:
      strcpy(b, "BIT %y,(%I+%d)");
      break;
    case 2:
      strcpy(b, "RES %y,(%I+%d)");
      break;
    case 3:
      strcpy(b, "SET %y,(%I+%d)");
      break;
    }
  temp= "";
  work= "";
  for (i= 0; b[i]; i++)
    {
      if ((b[i] == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (b[i] == '%')
	{
	  temp= "";
	  i++;
	  switch (b[i])
	    {
	    case 'y': work.appendf("%d", y); break;
	    case 'I':
	      if (cIR == &cIX)
		work+= "IX";
	      else
		work+= "IY";
	      break;
	    case 'b':
	      work.appendf("0x%02x", d);
	      break;
	    case 'd':
	      work.appendf("%d", d);
	      break;
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }
  return strdup(work.c_str());
}


int
cl_rxk::inst_length(t_addr addr)
{
  u8_t code= rom->get(addr);
  if (code == 0xcb)
    return 2;
  if ((code == 0xdd) || (code == 0xfd))
    {
      if (rom->get(addr+1) == 0xcb)
	return 4;
    }
  struct dis_entry *dt= dis_entry(addr);
  if (!dt) return 1;
  if (dt->mnemonic == NULL) return 1;
  return dt->length;
}


static FILE *log_file= NULL;
static unsigned int cyc= 0;

void
cl_rxk::save_hist()
{
  cl_uc::save_hist();
  if (juj&2)
    {
      if (log_file==NULL && PC==0x16) log_file= fopen("log.txt","w");
      if (log_file!=NULL) {
	fprintf(log_file, "%6u %06x ", cyc, AU(PC));
	fprintf(log_file, "%02x %02x ", rA, rF&0xc1);
	fprintf(log_file, "%04x %04x %04x ", rBC, rDE, rHL);
	fprintf(log_file, "%04x %04x %04x ", rIX, rIY, rSP);
	fprintf(log_file, "\n");
	cyc++;
      }
    }
}

void
cl_rxk::print_regs(class cl_console_base *con)
{
  if (jaj)
    {
      con->dd_color("answer");
      con->dd_printf("SZ-A-PNC  Flags= 0x%02x %3d %c  ",
		     rF, rF, isprint(rF)?rF:'.');
      con->dd_printf("A= 0x%02x %3d %c\n",
		     rA, rA, isprint(rA)?rA:'.');
      con->dd_printf("%c%c-%c-%c%c%c\n",
		     (rF&flagS)?'1':'0',
		     (rF&flagZ)?'1':'0',
		     'x',//(rF&flagA)?'1':'0',
		     (rF&flagV)?'1':'0',
		     'x',//(rF&BIT_N)?'1':'0',
		     (rF&flagC)?'1':'0');
      con->dd_printf("BC= 0x%04x [BC]= %02x %3d %c  ",
		     rBC, rom->get(rBC), rom->get(rBC),
		     isprint(rom->get(rBC))?rom->get(rBC):'.');
      con->dd_printf("DE= 0x%04x [DE]= %02x %3d %c  ",
		     rDE, rom->get(rDE), rom->get(rDE),
		     isprint(rom->get(rDE))?rom->get(rDE):'.');
      con->dd_printf("HL= 0x%04x [HL]= %02x %3d %c\n",
		     rHL, rom->get(rHL), rom->get(rHL),
		     isprint(rom->get(rHL))?rom->get(rHL):'.');
      con->dd_printf("IX= 0x%04x [IX]= %02x %3d %c  ",
		     rIX, rom->get(rIX), rom->get(rIX),
		     isprint(rom->get(rIX))?rom->get(rIX):'.');
      con->dd_printf("IY= 0x%04x [IY]= %02x %3d %c  ",
		     rIY, rom->get(rIY), rom->get(rIY),
		     isprint(rom->get(rIY))?rom->get(rIY):'.');
      con->dd_printf("SP= 0x%04x [SP]= %02x %3d %c\n",
		     rSP, rom->get(rSP), rom->get(rSP),
		     isprint(rom->get(rSP))?rom->get(rSP):'.');
      //con->dd_printf("SP limit= 0x%04x\n", AU(sp_limit));
      print_disass(PC, con);
      return;
    }
  con->dd_color("answer");
  con->dd_printf("A= 0x%02x %3d %c  ",
                 rA, rA, isprint(rA)?rA:'.');
  con->dd_printf("F= "); con->print_bin(rF, 8);
  con->dd_printf(" 0x%02x %3d %c", rF, rF, isprint(rF)?rF:'.');
  con->dd_printf("\n");
  con->dd_printf("                  SZxxxVxC\n");

  con->dd_printf("XPC= 0x%02x IP= 0x%02x IIR= 0x%02x EIR= 0x%02x\n",
		 mem->get_xpc(), rIP, rIIR, rEIR);
  
  con->dd_printf("BC= ");
  class cl_dump_ads ads(rBC, rBC+7);
  rom->dump(0, /*rBC, rBC+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("DE= ");
  ads._ss(rDE, rDE+7);
  rom->dump(0, /*rDE, rDE+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("HL= ");
  ads._ss(rHL, rHL+7);
  rom->dump(0, /*rHL, rHL+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("IX= ");
  ads._ss(rIX, rIX+7);
  rom->dump(0, /*rIX, rIX+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("IY= ");
  ads._ss(rIY, rIY+7);
  rom->dump(0, /*rIY, rIY+7*/&ads, 8, con);
  con->dd_color("answer");
  con->dd_printf("SP= ");
  ads._ss(rSP, rSP+7);
  rom->dump(0, /*rSP, rSP+7*/&ads, 8, con);
  con->dd_color("answer");

  con->dd_printf("aAF= 0x%02x-0x%02x  ", raA, raF);
  con->dd_printf("aBC= 0x%02x-0x%02x  ", raB, raC);
  con->dd_printf("aDE= 0x%02x-0x%02x  ", raD, raE);
  con->dd_printf("aHL= 0x%02x-0x%02x  ", raH, raL);
  con->dd_printf("\n");

  print_disass(PC, con);
}

class cl_cell8 *
cl_rxk::cR(u8_t z)
{
  if (z == 6)
    return ((cl_cell8*)(rom->get_cell(rHL)));
  return cRtab[z];
}

u8_t
cl_rxk::rR(u8_t z)
{
  class cl_cell8 *c= cR(z);
  u8_t v= c->get();
  return v;//cR(z).get();
}

class cl_cell8 *
cl_rxk::destR(u8_t z)
{
  if (z == 6)
    {
      return ((cl_cell8*)(rwas->get_cell(rHL)));
    }
  if (altd)
    return caRtab[z];
  return cRtab[z];
}

int
cl_rxk::exec_inst(void)
{
  t_mem code;
  int res= resGO;

  if (!prefix)
    {
      rwas= rom;
      altd= false;
    }
  prefix= false;
  atomic= false;
  
  instPC= PC;
  if (fetch(&code))
    return resBREAKPOINT;
  tick(1);
  if (code == 0xed)
    {
      code= fetch();
      return itab_ed[code](this, code);
    }
  if ((code & 0xdd) == 0xdd)
    {
      if (code == 0xdd)
	{
	  cIR= &cIX;
	  select_IRR(true);
	}
      else
	{
	  cIR= &cIY;
	  select_IRR(false);
	}
      {
      code= fetch();
      return itab_dd[code](this, code);
    }
    }
  res= itab[code](this, code);
  if (res == resNOT_DONE)
    {
      PC= instPC;
      code= rom->read(PC);
      res= inst_unknown(code);
    }
  
  return res;
}

int
cl_rxk::inst_unknown(t_mem code)
{
  cl_uc::inst_unknown(code);
  return resINV_INST;
}

u8_t
cl_rxk::op8_BC(void)
{
  u8_t v= rom->read(rBC);
  vc.rd++;
  return v;
}

u8_t
cl_rxk::op8_DE(void)
{
  u8_t v= rom->read(rDE);
  vc.rd++;
  return v;
}


u8_t
cl_rxk::op8_HL(void)
{
  u8_t v= rom->read(rHL);
  vc.rd++;
  return v;
}

u16_t
cl_rxk::op16_BC(void)
{
  u8_t l, h;
  l= rom->read(rBC);
  h= rom->read(rBC+1);
  vc.rd+= 2;
  return h*256+l;
}

u16_t
cl_rxk::op16_DE(void)
{
  u8_t l, h;
  l= rom->read(rDE);
  h= rom->read(rDE+1);
  vc.rd+= 2;
  return h*256+l;
}

u16_t
cl_rxk::op16_HL(void)
{
  u8_t l, h;
  l= rom->read(rHL);
  h= rom->read(rHL+1);
  vc.rd+= 2;
  return h*256+l;
}


/*
 * CPU peripheral: MMU functions
 */

cl_rxk_cpu::cl_rxk_cpu(class cl_uc *auc):
  cl_hw(auc, HW_CPU, 0, "cpu")
{
  ruc= (class cl_rxk *)auc;
}

int
cl_rxk_cpu::init(void)
{
  class cl_cvar *v;
  cl_hw::init();
  
  //stackseg= (cl_cell8*)ruc->ioi->get_cell(0x11);
  //dataseg = (cl_cell8*)ruc->ioi->get_cell(0x12);
  //segsize = (cl_cell8*)ruc->ioi->get_cell(0x13);

  stackseg= register_cell(ruc->ioi, 0x11);
  dataseg = register_cell(ruc->ioi, 0x12);
  segsize = register_cell(ruc->ioi, 0x13);
  
  uc->vars->add(v= new cl_cvar("STACKSEG", stackseg,
			       "MMU register: STACKSEG"));
  v->init();
  uc->vars->add(v= new cl_cvar("DATASEG", dataseg,
			       "MMU register: DATASEG"));
  v->init();
  uc->vars->add(v= new cl_cvar("SEGSIZE", segsize,
			       "MMU register: SEGSIZE"));
  v->init();

  return 0;
}

const char *
cl_rxk_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
      //case rxk_cpu_xpc: return "MMU register: XPC";
      //case rxk_cpu_nuof: return "";
    }
  return "Not used";
}

void
cl_rxk_cpu::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == segsize)
    {
      ruc->mem->set_segsize(*val);
    }
  else if (cell == dataseg)
    {
      ruc->mem->set_dataseg(*val);
    }
  else if (cell == stackseg)
    {
      ruc->mem->set_stackseg(*val);
    }
  
  else if (cell == ruc->XPC)
    {
      (*val)&= 0xfff;
      ruc->mem->set_lxpc(*val);
    }
}

/*
t_mem
cl_rxk_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch ((enum rxkcpu_cfg)addr)
    {
    case rxk_cpu_xpc:
      if (val) cell->set(ruc->mem->xpc= *val);
      return ruc->mem->xpc;
    case rxk_cpu_nuof:
      return 0;
    }
  return 0;

  if (val)
    cell->set(*val);
  return cell->get();
}
*/

void
cl_rxk_cpu::print_info(class cl_console_base *con)
{
  u8_t ss= ruc->mem->get_segsize();
  t_addr x, y;
  x= ss>>4;
  y= ss&0x0f;
  con->dd_color("answer");
  con->dd_printf("%s[%d]\n", id_string, id);
  con->dd_printf("(L)XPC  : 0x%03x\n", ruc->mem->get_xpc());
  con->dd_printf("SEGSIZE : 0x%03x\n", segsize->read());
  con->dd_printf("DATASEG : 0x%03x\n", ruc->mem->get_dataseg());
  con->dd_printf("STACKSEG: 0x%03x\n", ruc->mem->get_stackseg());
  //con->dd_printf("XPC     : 0x%02x\n", xpc->read());
  con->dd_printf("Prefix  : %s\n", ruc->prefix?"true":"false");
  con->dd_printf("ALTD    : %s\n", ruc->altd?"true":"false");
  con->dd_printf("Mem op  : %s\n", ruc->rwas->get_name());

  con->dd_printf("Segments: ");
  con->dd_cprintf("answer", "C=code ");
  con->dd_cprintf("dump_label", "D=data ");
  con->dd_cprintf("ui_title", "S=stack ");
  con->dd_cprintf("error", "X=xmem");
  con->dd_cprintf("answer", "\n");
  int l, r;
  t_addr la, pa;
  class cl_memory_cell *c;
  char t;
  for (l= 0, r= 8; l<8; l++, r++)
    {
      la= l*0x1000;
      if (la >= 0xe000) t= 'X', con->dd_color("error");
      else if (la >= (x<<12)) t= 'S', con->dd_color("ui_title");
      else if (la >= (y<<12)) t= 'D', con->dd_color("dump_label");
      else t= 'C', con->dd_color("answer");
      c= ruc->mem->get_cell(la);
      ruc->mem->chip->is_slot(c->get_data(), &pa);
      con->dd_printf("0x%xxxx -> 0x%03xxxx %c", l, pa>>12, t);
      con->dd_printf("   ");
      la= r*0x1000;
      if (la >= 0xe000) t= 'X', con->dd_color("error");
      else if (la >= (x<<12)) t= 'S', con->dd_color("ui_title");
      else if (la >= (y<<12)) t= 'D', con->dd_color("dump_label");
      else t= 'C', con->dd_color("answer");
      c= ruc->mem->get_cell(la);
      ruc->mem->chip->is_slot(c->get_data(), &pa);
      con->dd_printf("0x%xxxx -> 0x%03xxxx %c", r, pa>>12, t);
      con->dd_color("answer");
      con->dd_printf("\n");
    }
  
}


/* End of rxk.src/rxk.cc */
