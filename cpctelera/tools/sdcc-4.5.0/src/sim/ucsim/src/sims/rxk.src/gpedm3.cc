/*
 * Simulator of microcontrollers (gpedm3.cc)
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

#include "gpedm3.h"

struct dis_entry disass_pedm3[]=
  {
    { 0x47, 0xff, ' ', 2, "LD EIR,A" },
    { 0x4f, 0xff, ' ', 2, "LD IIR,A" },
    { 0x57, 0xff, ' ', 2, "LD A,EIR" },
    { 0x5f, 0xff, ' ', 2, "LD A,IIR" },
    { 0xa0, 0xff, ' ', 2, "LDI" },
    { 0xa8, 0xff, ' ', 2, "LDD" },
    { 0xb8, 0xff, ' ', 2, "LDDR" },
    { 0xb0, 0xff, ' ', 2, "LDIR" },
    { 0x54, 0xff, ' ', 2, "EXX (SP),HL" },
    { 0x4b, 0xff, ' ', 4, "LD BC,(%w)" },
    { 0x5b, 0xff, ' ', 4, "LD DE,(%w)" },
    { 0x6b, 0xff, ' ', 4, "LD HL,(%w)" },
    { 0x7b, 0xff, ' ', 4, "LD SP,(%w)" },
    { 0x49, 0xff, ' ', 2, "LD BC',BC" }, // '
    { 0x59, 0xff, ' ', 2, "LD DE',BC" }, // '
    { 0x69, 0xff, ' ', 2, "LD HL',BC" }, // '
    { 0x41, 0xff, ' ', 2, "LD BC',DE" }, // '
    { 0x51, 0xff, ' ', 2, "LD DE',DE" }, // '
    { 0x61, 0xff, ' ', 2, "LD HL',DE" }, // '
    { 0x43, 0xff, ' ', 4, "LD (%w),BC" },
    { 0x53, 0xff, ' ', 4, "LD (%w),DE" },
    { 0x63, 0xff, ' ', 4, "LD (%w),HL" },
    { 0x73, 0xff, ' ', 4, "LD (%w),SP" },
    { 0x44, 0xff, ' ', 2, "NEG" },
    { 0x45, 0xff, ' ', 2, "LRET" },
    { 0x46, 0xff, ' ', 2, "IPSET 0" },
    { 0x56, 0xff, ' ', 2, "IPSET 1" },
    { 0x4e, 0xff, ' ', 2, "IPSET 2" },
    { 0x5e, 0xff, ' ', 2, "IPSET 3" },
    { 0x4d, 0xff, ' ', 2, "RETI" },
    { 0x5d, 0xff, ' ', 2, "IPRES" },
    { 0x64, 0xff, ' ', 2, "LDP (HL),HL" },
    { 0x65, 0xff, ' ', 4, "LDP (%w),HL" },
    { 0x6c, 0xff, ' ', 2, "LDP HL,(HL)" },
    { 0x6d, 0xff, ' ', 4, "LDP HL,(%w)" },
    { 0x67, 0xff, ' ', 2, "LD XPC,A" },
    { 0x77, 0xff, ' ', 2, "LD A,XPC" },
    { 0x76, 0xff, ' ', 2, "PUSH IP" },
    { 0x7e, 0xff, ' ', 2, "POP IP" },
    
    { 0x42, 0xff, ' ', 2, "SBC HL,BC" },
    { 0x52, 0xff, ' ', 2, "SBC HL,DE" },
    { 0x62, 0xff, ' ', 2, "SBC HL,HL" },
    { 0x72, 0xff, ' ', 2, "SBC HL,SP" },
    { 0x4a, 0xff, ' ', 2, "ADC HL,BC" },
    { 0x5a, 0xff, ' ', 2, "ADC HL,DE" },
    { 0x6a, 0xff, ' ', 2, "ADC HL,HL" },
    { 0x7a, 0xff, ' ', 2, "ADC HL,SP" },

    { 0, 0, 0, 0, 0, 0, 0 }
  };
  
/* End of rxk.src/gpedm3.cc */
