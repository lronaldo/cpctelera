/*
 * Simulator of microcontrollers (gpddm3.cc)
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

#include "gpddm3.h"

struct dis_entry disass_pddm3[]=
  {
    { 0x21, 0xff, ' ', 4, "LD %I,%w" },
    
    { 0x09, 0xff, ' ', 2, "ADD %I,BC" },
    { 0x19, 0xff, ' ', 2, "ADD %I,DE" },
    { 0x29, 0xff, ' ', 2, "ADD %I,%I" },
    { 0x39, 0xff, ' ', 2, "ADD %I,SP" },
    { 0x34, 0xff, ' ', 3, "INC (%I%d)" },
    { 0x35, 0xff, ' ', 3, "DEC (%I%d)" },
    { 0xbe, 0xff, ' ', 3, "CP (%I%d)" },
    { 0x9e, 0xff, ' ', 3, "SBC A,(%I%d)" },
    { 0x96, 0xff, ' ', 3, "SUB A,(%I%d)" },
    { 0x86, 0xff, ' ', 3, "ADD A,(%I%d)" },
    { 0x8e, 0xff, ' ', 3, "ADC A,(%I%d)" },
    { 0x23, 0xff, ' ', 2, "INC %I" },
    { 0x2b, 0xff, ' ', 2, "DEC %I" },
    { 0xfc, 0xff, ' ', 2, "RR %I" },

    { 0xae, 0xff, ' ', 3, "XOR A,(%I%d)" },
    { 0xa6, 0xff, ' ', 3, "AND A,(%I%d)" },
    { 0xb6, 0xff, ' ', 3, "OR A,(%I%d)" },
    { 0xcc, 0xff, ' ', 2, "BOOL %I" },
    { 0xdc, 0xff, ' ', 2, "AND %I,DE" },
    { 0xec, 0xff, ' ', 2, "OR %I,DE" },

    { 0xe1, 0xff, ' ', 2, "POP %I" },
    { 0xe5, 0xff, ' ', 2, "PUSH %I" },
    { 0xe3, 0xff, ' ', 2, "EX (SP),%I" },

    { 0x77, 0xff, ' ', 3, "LD (%I%d),A" },
    { 0x70, 0xff, ' ', 3, "LD (%I%d),B" },
    { 0x71, 0xff, ' ', 3, "LD (%I%d),C" },
    { 0x72, 0xff, ' ', 3, "LD (%I%d),D" },
    { 0x73, 0xff, ' ', 3, "LD (%I%d),E" },
    { 0x74, 0xff, ' ', 3, "LD (%I%d),H" },
    { 0x75, 0xff, ' ', 3, "LD (%I%d),L" },
    { 0x7e, 0xff, ' ', 3, "LD A,(%I%d)" },
    { 0x46, 0xff, ' ', 3, "LD B,(%I%d)" },
    { 0x4e, 0xff, ' ', 3, "LD C,(%I%d)" },
    { 0x56, 0xff, ' ', 3, "LD D,(%I%d)" },
    { 0x5e, 0xff, ' ', 3, "LD E,(%I%d)" },
    { 0x66, 0xff, ' ', 3, "LD H,(%I%d)" },
    { 0x6e, 0xff, ' ', 3, "LD L,(%I%d)" },

    { 0xf9, 0xff, ' ', 2, "LD SP,%I" },
    { 0xc4, 0xff, ' ', 3, "LD %I,(SP+%b)" },
    { 0xd4, 0xff, ' ', 3, "LD (SP+%b),%I" },
    { 0xe4, 0xff, ' ', 3, "LD HL,(%J%d)" },
    { 0x36, 0xff, ' ', 4, "LD (%I%d),%b" },
    { 0x06, 0xff, ' ', 2, "LD A,(%I+A)" },
    { 0x22, 0xff, ' ', 4, "LD (%w),%I" },
    { 0x2a, 0xff, ' ', 4, "LD %I,(%w)" },
    { 0x7c, 0xff, ' ', 2, "LD HL,%I" },
    { 0x7d, 0xff, ' ', 2, "LD %I,HL" },
    { 0xf4, 0xff, ' ', 3, "LD (HL%d),HL" },
    
    { 0x64, 0xff, ' ', 2, "LDP (%I),HL" },
    { 0x64, 0xff, ' ', 4, "LDP (%w),%I" },
    { 0x6c, 0xff, ' ', 2, "LDP HL,(%I)" },
    { 0x6d, 0xff, ' ', 2, "LDP %I,(%w)" },

    { 0xe9, 0xff, ' ', 2, "JP (%I)" },

    { 0, 0, 0, 0, 0, 0, 0 }
  };
  
/* End of rxk.src/gpddm3.cc */
