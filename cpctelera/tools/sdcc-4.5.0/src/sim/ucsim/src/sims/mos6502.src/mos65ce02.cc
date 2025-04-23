/*
 * Simulator of microcontrollers (mos65ce02.cc)
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

#include "appcl.h"

#include "mos65ce02cl.h"


cl_mos65ce02::cl_mos65ce02(class cl_sim *asim):
  cl_mos65c02s(asim)
{
  *my_id= "MOS65CE02";
}

int
cl_mos65ce02::init(void)
{
  cl_mos65c02s::init();
#define RCV(R) reg_cell_var(&c ## R , &r ## R , "" #R "" , "CPU register " #R "")
  RCV(B);
  RCV(Z);
#undef RCV
  return 0;
}


void
cl_mos65ce02::print_regs(class cl_console_base *con)
{
  int ojaj= jaj;
  jaj= 0;
  con->dd_color("answer");
  con->dd_printf("A= $%02x %3d %+4d %c  ", A, A, (i8_t)A, isprint(A)?A:'.');
  con->dd_printf("X= $%02x %3d %+4d %c  ", X, X, (i8_t)X, isprint(X)?X:'.');
  con->dd_printf("Y= $%02x %3d %+4d %c  ", Y, Y, (i8_t)Y, isprint(Y)?Y:'.');
  con->dd_printf("Z= $%02x %3d %+4d %c  ", Z, Z, (i8_t)Z, isprint(Z)?Z:'.');
  con->dd_printf("\n");
  con->dd_printf("P= "); con->print_bin(CC, 8);
  con->dd_printf("        B= $%02x", B);
  con->dd_printf("\n");
  con->dd_printf("   NV BDIZC\n");

  con->dd_printf("S= ");
  class cl_dump_ads ads(0x100+SP, 0x100+SP+7);
  rom->dump(0, /*0x100+SP, 0x100+SP+7*/&ads, 8, con);
  con->dd_color("answer");
  
  if (!ojaj)
    print_disass(PC, con);
  else
    {
      con->dd_printf(" ? 0x%04x ", PC);
      {
	int i, j, code= rom->read(PC), l= inst_length(PC);
	struct dis_entry *dt= dis_tbl();
	for (i=0;i<3;i++)
	  if (i<l)
	    con->dd_printf("%02x ",rom->get(PC+i));
	  else
	    con->dd_printf("   ");
	i= 0;
	while (((code & dt[i].mask) != dt[i].code) &&
	       dt[i].mnemonic)
	  i++;
	if (dt[i].mnemonic)
	  {
	    for (j=0;j<3;j++)
	      con->dd_printf("%c", dt[i].mnemonic[j]);
	  }
      }
      con->dd_printf("\n");
    }
  jaj= ojaj;
}


/* End of mos6502.src/mos65ce02.cc */
