/*
 * Simulator of microcontrollers (gpddm4.cc)
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

#include "gpddm4.h"

struct dis_entry disass_pddm4[]=
  {
    { 0x06, 0xff, ' ', 2, "LD A,(%I+A)" },
    { 0x4c, 0xff, ' ', 2, "TEST %I" },
    { 0x1a, 0xff, ' ', 2, "LD %3,(HL)" },
    { 0xce, 0xff, ' ', 3, "LD %3,(IX%d)" },
    { 0xde, 0xff, ' ', 3, "LD %3,(IY%d)" },
    { 0xee, 0xff, ' ', 3, "LD %3,(SP+%b)" },
    { 0x1b, 0xff, ' ', 2, "LD (HL),%3" },
    { 0xcf, 0xff, ' ', 3, "LD (IX%d),%3" },
    { 0xdf, 0xff, ' ', 3, "LD (IY%d),%3" },
    { 0xef, 0xff, ' ', 3, "LD (SP+%b),%3" },
    { 0x4d, 0xff, ' ', 2, "NEG %3" },
    { 0xf1, 0xff, ' ', 2, "POP %3" },
    { 0xf5, 0xff, ' ', 2, "PUSH %3" },
    { 0x68, 0xff, ' ', 2, "RL 1,%3" },
    { 0x69, 0xff, ' ', 2, "RL 2,%3" },
    { 0x6b, 0xff, ' ', 2, "RL 4,%3" },
    { 0x48, 0xff, ' ', 2, "RLC 1,%3" },
    { 0x49, 0xff, ' ', 2, "RLC 2,%3" },
    { 0x4b, 0xff, ' ', 2, "RLC 4,%3" },
    { 0x4f, 0xff, ' ', 2, "RLC 8,%3" },
    { 0x6f, 0xff, ' ', 2, "RLB A,%3" },
    { 0x88, 0xff, ' ', 2, "SLA 1,%3" },
    { 0x89, 0xff, ' ', 2, "SLA 2,%3" },
    { 0x8b, 0xff, ' ', 2, "SLA 4,%3" },
    { 0xa8, 0xff, ' ', 2, "SLL 1,%3" },
    { 0xa9, 0xff, ' ', 2, "SLL 2,%3" },
    { 0xab, 0xff, ' ', 2, "SLL 4,%3" },
    { 0x5c, 0xff, ' ', 2, "TEST %3" },
    { 0x78, 0xff, ' ', 2, "RR 1,%3" },
    { 0x79, 0xff, ' ', 2, "RR 2,%3" },
    { 0x7b, 0xff, ' ', 2, "RR 4,%3" },
    { 0x58, 0xff, ' ', 2, "RRC 1,%3" },
    { 0x59, 0xff, ' ', 2, "RRC 2,%3" },
    { 0x5b, 0xff, ' ', 2, "RRC 4,%3" },
    { 0x5f, 0xff, ' ', 2, "RRC 8,%3" },
    { 0x7f, 0xff, ' ', 2, "RRB A,%3" },
    { 0x98, 0xff, ' ', 2, "SRA 1,%3" },
    { 0x99, 0xff, ' ', 2, "SRA 2,%3" },
    { 0x9b, 0xff, ' ', 2, "SRA 4,%3" },
    { 0xb8, 0xff, ' ', 2, "SRL 1,%3" },
    { 0xb9, 0xff, ' ', 2, "SRL 2,%3" },
    { 0xbb, 0xff, ' ', 2, "SRL 4,%3" },
    
    { 0x0a, 0xff, ' ', 5, "LDF %3,(%l)" },
    { 0x0b, 0xff, ' ', 5, "LDF (%l),%3" },
    { 0x0c, 0xff, ' ', 2, "LD %3,(PW+HL)" },
    { 0x1c, 0xff, ' ', 2, "LD %3,(PX+HL)" },
    { 0x2c, 0xff, ' ', 2, "LD %3,(PY+HL)" },
    { 0x3c, 0xff, ' ', 2, "LD %3,(PZ+HL)" },
    { 0x0d, 0xff, ' ', 2, "LD (PW+HL),%3" },
    { 0x1d, 0xff, ' ', 2, "LD (PX+HL),%3" },
    { 0x2d, 0xff, ' ', 2, "LD (PY+HL),%3" },
    { 0x3d, 0xff, ' ', 2, "LD (PZ+HL),%3" },
    { 0x0e, 0xff, ' ', 2, "LD %3,(PW%d)" },
    { 0x1e, 0xff, ' ', 2, "LD %3,(PX%d)" },
    { 0x2e, 0xff, ' ', 2, "LD %3,(PY%d)" },
    { 0x3e, 0xff, ' ', 2, "LD %3,(PZ%d)" },
    { 0x0f, 0xff, ' ', 2, "LD (PW%d),%3" },
    { 0x1f, 0xff, ' ', 2, "LD (PX%d),%3" },
    { 0x2f, 0xff, ' ', 2, "LD (PY%d),%3" },
    { 0x3f, 0xff, ' ', 2, "LD (PZ%d),%3" },
    { 0x8c, 0xff, ' ', 2, "LDL PW,%I" },
    { 0x9c, 0xff, ' ', 2, "LDL PX,%I" },
    { 0xac, 0xff, ' ', 2, "LDL PY,%I" },
    { 0xbc, 0xff, ' ', 2, "LDL PZ,%I" },
    { 0x8d, 0xff, ' ', 2, "LD PW,%3" },
    { 0x9d, 0xff, ' ', 2, "LD PX,%3" },
    { 0xad, 0xff, ' ', 2, "LD PY,%3" },
    { 0xbd, 0xff, ' ', 2, "LD PZ,%3" },
    { 0x8f, 0xff, ' ', 2, "LDL PW,%n" },
    { 0x9f, 0xff, ' ', 2, "LDL PX,%n" },
    { 0xaf, 0xff, ' ', 2, "LDL PY,%n" },
    { 0xbf, 0xff, ' ', 2, "LDL PZ,%n" },
    { 0xcd, 0xff, ' ', 2, "LD %3,PW" },
    { 0xdd, 0xff, ' ', 2, "LD %3,PX" },
    { 0xed, 0xff, ' ', 2, "LD %3,PY" },
    { 0xfd, 0xff, ' ', 2, "LD %3,PZ" },
    { 0xff, 0xff, ' ', 2, "LD (SP+HL),%3" },
    { 0xea, 0xff, ' ', 2, "CALL (%I)" },
    { 0xfe, 0xff, ' ', 2, "LD %3,(SP+HL)" },

    { 0, 0, 0, 0, 0, 0, 0 }
  };
  
/* End of rxk.src/gpddm4.cc */
