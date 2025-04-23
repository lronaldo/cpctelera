/*
 * Simulator of microcontrollers (gp0m4.cc)
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

#include "gp0m4.h"

struct dis_entry disass_p0m4[]=
  {
    { 0x42, 0xff, ' ', 1, "RL HL" },
    { 0x62, 0xff, ' ', 1, "RL BC" },
    { 0x45, 0xff, ' ', 1, "SUB HL,JK" },
    { 0x55, 0xff, ' ', 1, "SUB HL,DE" },
    { 0x4c, 0xff, ' ', 1, "TEST HL" },
    { 0x48, 0xff, ' ', 2, "CP HL,%d" },
    { 0x60, 0xff, ' ', 1, "RLC BC" },
    { 0x50, 0xff, ' ', 1, "RLC DE" },
    { 0x61, 0xff, ' ', 1, "RRC BC" },
    { 0x51, 0xff, ' ', 1, "RRC DE" },
    { 0x54, 0xff, ' ', 1, "XOR HL,DE" },
    { 0x63, 0xff, ' ', 1, "RR BC" },
    { 0x65, 0xff, ' ', 1, "ADD HL,JK" },
    { 0x81, 0xff, ' ', 1, "LD HL,BC" },
    { 0x91, 0xff, ' ', 1, "LD BC,HL" },
    { 0xa1, 0xff, ' ', 1, "LD HL,DE" },
    { 0x82, 0xff, ' ', 4, "LD (%l),HL" },
    { 0x82, 0xff, ' ', 4, "LD HL,(%l)" },
    { 0x83, 0xff, ' ', 3, "LD (%w),BCDE" },
    { 0x84, 0xff, ' ', 3, "LD (%w),JKHL" },
    { 0x93, 0xff, ' ', 3, "LD BCDE,(%w)" },
    { 0x94, 0xff, ' ', 3, "LD JKHL,(%w)" },
    { 0x85, 0xff, ' ', 2, "LD HL,(PW%d)" },
    { 0x95, 0xff, ' ', 2, "LD HL,(PX%d)" },
    { 0xa5, 0xff, ' ', 2, "LD HL,(PY%d)" },
    { 0xb5, 0xff, ' ', 2, "LD HL,(PZ%d)" },
    { 0x86, 0xff, ' ', 2, "LD (PW%d),HL" },
    { 0x96, 0xff, ' ', 2, "LD (PX%d),HL" },
    { 0xa6, 0xff, ' ', 2, "LD (PY%d),HL" },
    { 0xb6, 0xff, ' ', 2, "LD (PZ%d),HL" },
    { 0x87, 0xff, ' ', 5, "LLJP %w,%w" },
    { 0x89, 0xff, ' ', 3, "LD (%w),JK" },
    { 0x99, 0xff, ' ', 3, "LD JK,(%w)" },
    { 0x8a, 0xff, ' ', 4, "LDF (%l),A" },
    { 0x9a, 0xff, ' ', 4, "LDF A,(%l)" },
    { 0x8b, 0xff, ' ', 1, "LD A,(PW+HL)" },
    { 0x9b, 0xff, ' ', 1, "LD A,(PX+HL)" },
    { 0xab, 0xff, ' ', 1, "LD A,(PY+HL)" },
    { 0xbb, 0xff, ' ', 1, "LD A,(PZ+HL)" },
    { 0x8c, 0xff, ' ', 1, "LD (PW+HL),A" },
    { 0x9c, 0xff, ' ', 1, "LD (PX+HL),A" },
    { 0xac, 0xff, ' ', 1, "LD (PY+HL),A" },
    { 0xbc, 0xff, ' ', 1, "LD (PZ+HL),A" },
    { 0x8d, 0xff, ' ', 1, "LD A,(PW%d)" },
    { 0x9d, 0xff, ' ', 1, "LD A,(PX%d)" },
    { 0xad, 0xff, ' ', 1, "LD A,(PY%d)" },
    { 0xbd, 0xff, ' ', 1, "LD A,(PZ%d)" },
    { 0x8d, 0xff, ' ', 1, "LD (PW%d),A" },
    { 0x9d, 0xff, ' ', 1, "LD (PX%d),A" },
    { 0xad, 0xff, ' ', 1, "LD (PY%d),A" },
    { 0xbd, 0xff, ' ', 1, "LD (PZ%d),A" },
    { 0x8f, 0xff, ' ', 5, "LLCALL %w,%w" },
    { 0x97, 0xff, ' ', 1, "LD LXPC,HL" },
    { 0x9f, 0xff, ' ', 1, "LD HL,LXPC" },
    { 0x98, 0xff, ' ', 3, "JRE %R" },
    { 0xa0, 0xff, ' ', 2, "JR GT,%r" },
    { 0xb0, 0xff, ' ', 2, "JR LT,%r" },
    { 0xa8, 0xff, ' ', 2, "JR GTU,%r" },
    { 0xb8, 0xff, ' ', 2, "JR V,%r" },
    { 0xa2, 0xff, ' ', 3, "JR GT,%w" },
    { 0xb2, 0xff, ' ', 3, "JR LT,%w" },
    { 0xaa, 0xff, ' ', 3, "JR GTU,%w" },
    { 0xba, 0xff, ' ', 3, "JR V,%w" },
    { 0xa3, 0xff, ' ', 2, "LD BCDE,%d" },
    { 0xa4, 0xff, ' ', 2, "LD JKHL,%d" },
    { 0xa7, 0xff, ' ', 1, "MULU" },
    { 0xa9, 0xff, ' ', 3, "LD JK,%w" },
    { 0xb1, 0xff, ' ', 1, "LD DE,HL" },
    { 0xb3, 0xff, ' ', 1, "EX BC,HL" },
    { 0xb4, 0xff, ' ', 1, "EX JKHL,BCDE" },
    { 0xb9, 0xff, ' ', 1, "EX JK,HL" },
    { 0xbf, 0xff, ' ', 1, "CLR HL" },
    
    { 0, 0, 0, 0, 0, 0, 0 }
  };
  
/* End of rxk.src/gp0m4.cc */
