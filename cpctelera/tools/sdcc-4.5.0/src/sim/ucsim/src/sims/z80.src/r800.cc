/*
 * Simulator of microcontrollers (r800.cc)
 *
 * Copyright (C) 2023 Drotos Daniel
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

#include "r800cl.h"


cl_r800::cl_r800(struct cpu_entry *Itype, class cl_sim *asim):
  cl_ez80(Itype, asim)
{
}

int
cl_r800::init(void)
{
  cl_ez80::init();
  // FIXME
  r800_ttab_ed[0xc3]= 37;
  r800_ttab_ed[0xd3]= 37;
  r800_ttab_ed[0xe3]= 37;
  r800_ttab_ed[0xf3]= 37;

  ttab_00= r800_ttab_00;
  ttab_dd= r800_ttab_dd;
  ttab_cb= r800_ttab_cb;
  ttab_ed= r800_ttab_ed;
  ttab_fd= r800_ttab_fd;
  ttab_ddcb= r800_ttab_ddcb;
  ttab_fdcb= r800_ttab_fdcb;

  return 0;
}

const char *
cl_r800::id_string(void)
{
  return ("R800");
}

/*
  dd/fd 88+p   ADC   A,IXYp
  dd/fd 80+p   ADD   A,IXYp
  dd/fd a0+p   AND   IXYp
  dd/fd b8 oo  CP    (IXY+oo)
  dd/fd 05+8*p DEC   IXYp
  dd/fd 04+8*p INX   IXYp
  dd/fd 78+p   LD    A,IXYp
  dd/fd 40+p   LD    B,IXYp
  dd/fd 48+p   LD    C,IXYp
  dd/fd 50+p   LD    D,IXYp
  dd/fd 58+p   LD    E,IXYp
  dd/fd 60+p   LD    IXYh,p
  dd/fd 68+p   LD    IXYl,p
  ed c1+8*r    MULUB A,r
  dd/fd b0+p   OR    IXYp
  dd/fd 98+p   SBC   A,IXYp
  dd/fd 90+p   SUB   IXYp
  dd/fd a8+p   XOR   IXYp
 */

int
cl_r800::inst_ed_(t_mem code)
{
  switch (code)
    {
      // Z280/R800 multu HL=A*r
      // ED 11rr r001
    case 0xc1: case 0xc9:
    case 0xd1: case 0xd9:
    case 0xe1: case 0xe9:
    case 0xf1: case 0xf9:
      {
	unsigned int result = (unsigned int)(regs.raf.A) * reg_g_read((code >> 3) & 0x07);
	regs.HL = result;
	regs.raf.F &= ~(BIT_S | BIT_Z | BIT_P | BIT_C);
	if (!result)
	  regs.raf.F |= BIT_Z;
	if (result >= 0x100)
	  regs.raf.F |= BIT_C;
	return(resGO);
      }
      

      // Z280/R800 multuw
      // multuw DE:HL= HL*ss
      // ED 11ss 0011
    case 0xc3: case 0xd3: case 0xe3: case 0xf3:
      {
	unsigned long result = (unsigned long)(regs.HL) *  (unsigned long)(regrp_ss_read((code >> 4)&3));
	regs.HL = (result >> 0) & 0xff;
	regs.DE = (result >> 16) & 0xff;
	regs.raf.F &= ~(BIT_S | BIT_Z | BIT_P | BIT_C);
	if (!result)
	  regs.raf.F |= BIT_Z;
	if (regs.DE)
	  regs.raf.F |= BIT_C;
	return(resGO);
      }

    default:
      return cl_ez80::inst_ed_(code);
    }
  
  return resINV_INST;
}


/* End of z80.src/r800.cc */
