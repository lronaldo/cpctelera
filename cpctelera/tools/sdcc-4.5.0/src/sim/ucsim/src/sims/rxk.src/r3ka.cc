/*
 * Simulator of microcontrollers (r3ka.cc)
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

#include <ctype.h>

#include "appcl.h"

#include "glob.h"
#include "gp0m3.h"
#include "gpddm3.h"
#include "gpedm3.h"
#include "gpedm3a.h"

#include "r3kacl.h"


cl_r3ka::cl_r3ka(class cl_sim *asim):
  cl_r3k(asim)
{
}

int
cl_r3ka::init(void)
{
  cl_r3k::init();
#define RCV(R) reg_cell_var(&c ## R , &r ## R , "" #R "" , "CPU register " #R "")
  RCV(SU);
#undef RCV
  return 0;
}

const char *
cl_r3ka::id_string(void)
{
  return "R3KA";
}

void
cl_r3ka::reset(void)
{
  cl_r3k::reset();
  edmr= 0;
  //mode3k();  
}

struct dis_entry *
cl_r3ka::dis_entry(t_addr addr)
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
      return NULL;
    }
  
  dt= disass_rxk;
  i= 0;
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
cl_r3ka::disassc(t_addr addr, chars *comment)
{
  t_mem code= rom->get(addr);
  if (code == 0x5b)
    {
      if (rom->get(addr-1) != 0x76)
	return strdup("IDET");
    }
  return cl_r3k::disassc(addr, comment);
}

void
cl_r3ka::print_regs(class cl_console_base *con)
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

  con->dd_printf("XPC= 0x%02x IP= 0x%02x IIR= 0x%02x EIR= 0x%02x SU= 0x%02x\n",
		 mem->get_xpc(), rIP, rIIR, rEIR, rSU);
  
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
  con->dd_printf("SP= ");
  ads._ss7(rSP);
  rom->dump(0, /*rSP, rSP+7*/&ads, 8, con);
  con->dd_color("answer");

  con->dd_printf("aAF= 0x%02x-0x%02x  ", raA, raF);
  con->dd_printf("aBC= 0x%02x-0x%02x  ", raB, raC);
  con->dd_printf("aDE= 0x%02x-0x%02x  ", raD, raE);
  con->dd_printf("aHL= 0x%02x-0x%02x  ", raH, raL);
  con->dd_printf("\n");
  
  print_disass(PC, con);
}

/* End of rxk.src/r3ka.cc */
