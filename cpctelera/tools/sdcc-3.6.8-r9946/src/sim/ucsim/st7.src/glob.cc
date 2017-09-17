/*
 * Simulator of microcontrollers (glob.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

#include <stdio.h>

#include "stypes.h"

/* ST7 instructions described in PM0044
 *
 * 19.12.2012 - initial - ready for testing
 *
 */


/*
%d - direct addressing
%x - extended addressing
%p - pc relative addressing
%b - unsigned byte immediate addressing
%w - unsigned word immediate addressing
%e - unsigned 24bit immediate addressing (extmem)
%s - signed byte immediate
%1 - unsigned byte index offset
%2 - unsigned word index offset
%3 - unsigned 24bit index offset
*/

/*  uint  code, mask;  char  branch;  uchar length;  char  *mnemonic; */

/* vaclavpe - commented out is stuff from STM8 */

struct dis_entry disass_st7[]= {
//  { 0x0019, 0x00ff, ' ', 2, "adc A,(%1,SP)" },
  { 0x00a9, 0x00ff, ' ', 2, "adc A,%b" },
  { 0x00b9, 0x00ff, ' ', 2, "adc A,%d" },
  { 0x00c9, 0x00ff, ' ', 3, "adc A,%x" },
  { 0x00d9, 0x00ff, ' ', 3, "adc A,(%2,X)" },
  { 0x00e9, 0x00ff, ' ', 2, "adc A,(%1,X)" },
  { 0x00f9, 0x00ff, ' ', 1, "adc A,(X)" },

//  { 0x001b, 0x00ff, ' ', 2, "add A,(%1,SP)" },
  { 0x00ab, 0x00ff, ' ', 2, "add A,%b" },
  { 0x00bb, 0x00ff, ' ', 2, "add A,%d" },
  { 0x00cb, 0x00ff, ' ', 3, "add A,%x" },
  { 0x00db, 0x00ff, ' ', 3, "add A,(%2,X)" },
  { 0x00eb, 0x00ff, ' ', 2, "add A,(%1,X)" },
  { 0x00fb, 0x00ff, ' ', 1, "add A,(X)" },
//  { 0x005b, 0x00ff, ' ', 2, "add SP,%b" },

//  { 0x001c, 0x00ff, ' ', 3, "addw X,%w" },

//  { 0x0014, 0x00ff, ' ', 2, "and A,(%1,SP)" },
  { 0x00a4, 0x00ff, ' ', 2, "and A,%b" },
  { 0x00b4, 0x00ff, ' ', 2, "and A,%d" },
  { 0x00c4, 0x00ff, ' ', 3, "and A,%x" },
  { 0x00d4, 0x00ff, ' ', 3, "and A,(%2,X)" },
  { 0x00e4, 0x00ff, ' ', 2, "and A,(%1,X)" },
  { 0x00f4, 0x00ff, ' ', 1, "and A,(X)" },

//  { 0x0015, 0x00ff, ' ', 2, "bcp A,(%1,SP)" },
  { 0x00a5, 0x00ff, ' ', 2, "bcp A,%b" },
  { 0x00b5, 0x00ff, ' ', 2, "bcp A,%d" },
  { 0x00c5, 0x00ff, ' ', 3, "bcp A,%x" },
  { 0x00d5, 0x00ff, ' ', 3, "bcp A,(%2,X)" },
  { 0x00e5, 0x00ff, ' ', 2, "bcp A,(%1,X)" },
  { 0x00f5, 0x00ff, ' ', 1, "bcp A,(X)" },

  { 0x0011, 0x00ff, ' ', 2, "bres %d,#0" },
  { 0x0013, 0x00ff, ' ', 2, "bres %d,#1" },
  { 0x0015, 0x00ff, ' ', 2, "bres %d,#2" },
  { 0x0017, 0x00ff, ' ', 2, "bres %d,#3" },
  { 0x0019, 0x00ff, ' ', 2, "bres %d,#4" },
  { 0x001b, 0x00ff, ' ', 2, "bres %d,#5" },
  { 0x001d, 0x00ff, ' ', 2, "bres %d,#6" },
  { 0x001f, 0x00ff, ' ', 2, "bres %d,#7" },

  { 0x0010, 0x00ff, ' ', 2, "bset %d,#0" },
  { 0x0012, 0x00ff, ' ', 2, "bset %d,#1" },
  { 0x0014, 0x00ff, ' ', 2, "bset %d,#2" },
  { 0x0016, 0x00ff, ' ', 2, "bset %d,#3" },
  { 0x0018, 0x00ff, ' ', 2, "bset %d,#4" },
  { 0x001a, 0x00ff, ' ', 2, "bset %d,#5" },
  { 0x001c, 0x00ff, ' ', 2, "bset %d,#6" },
  { 0x001e, 0x00ff, ' ', 2, "bset %d,#7" },

  { 0x0001, 0x00ff, ' ', 3, "bjtf %d,#0,%p" },
  { 0x0003, 0x00ff, ' ', 3, "bjtf %d,#1,%p" },
  { 0x0005, 0x00ff, ' ', 3, "bjtf %d,#2,%p" },
  { 0x0007, 0x00ff, ' ', 3, "bjtf %d,#3,%p" },
  { 0x0009, 0x00ff, ' ', 3, "bjtf %d,#4,%p" },
  { 0x000b, 0x00ff, ' ', 3, "bjtf %d,#5,%p" },
  { 0x000d, 0x00ff, ' ', 3, "bjtf %d,#6,%p" },
  { 0x000f, 0x00ff, ' ', 3, "bjtf %d,#7,%p" },

  { 0x0000, 0x00ff, ' ', 3, "btjt %d,#0,%p" },
  { 0x0002, 0x00ff, ' ', 3, "btjt %d,#1,%p" },
  { 0x0004, 0x00ff, ' ', 3, "btjt %d,#2,%p" },
  { 0x0006, 0x00ff, ' ', 3, "btjt %d,#3,%p" },
  { 0x0008, 0x00ff, ' ', 3, "btjt %d,#4,%p" },
  { 0x000a, 0x00ff, ' ', 3, "btjt %d,#5,%p" },
  { 0x000c, 0x00ff, ' ', 3, "btjt %d,#6,%p" },
  { 0x000e, 0x00ff, ' ', 3, "btjt %d,#7,%p" },

  { 0x00bd, 0x00ff, ' ', 2, "call %d", true },
  { 0x00cd, 0x00ff, ' ', 3, "call %x", true },
  { 0x00dd, 0x00ff, ' ', 3, "call (%2,X)", true },
  { 0x00ed, 0x00ff, ' ', 2, "call (%1,X)", true },
  { 0x00fd, 0x00ff, ' ', 1, "call (X)", true },
//  { 0x008d, 0x00ff, ' ', 4, "callf %e" },
  { 0x00ad, 0x00ff, ' ', 2, "callr %p", true },

//  { 0x008c, 0x00ff, ' ', 1, "ccf" },

//  { 0x000f, 0x00ff, ' ', 2, "clr (%1,SP)" },
  { 0x003f, 0x00ff, ' ', 2, "clr %d" },
  { 0x004f, 0x00ff, ' ', 1, "clr A" },
  { 0x006f, 0x00ff, ' ', 2, "clr (%1,X)" },
  { 0x007f, 0x00ff, ' ', 1, "clr (X)" },
  { 0x005f, 0x00ff, ' ', 1, "clr X" },

//  { 0x0011, 0x00ff, ' ', 2, "cp A,(%1,SP)" },
  { 0x00a1, 0x00ff, ' ', 2, "cp A,%b" },
  { 0x00b1, 0x00ff, ' ', 2, "cp A,%d" },
  { 0x00c1, 0x00ff, ' ', 3, "cp A,%x" },
  { 0x00d1, 0x00ff, ' ', 3, "cp A,(%2,X)" },
  { 0x00e1, 0x00ff, ' ', 2, "cp A,(%1,X)" },
  { 0x00f1, 0x00ff, ' ', 1, "cp A,(X)" },
//  { 0x0013, 0x00ff, ' ', 2, "cpw X,(%1,SP)" },
  { 0x00a3, 0x00ff, ' ', 3, "cp X,%w" },
  { 0x00b3, 0x00ff, ' ', 2, "cp X,%d" },
  { 0x00c3, 0x00ff, ' ', 3, "cp X,%x" },
  { 0x00d3, 0x00ff, ' ', 3, "cp X,(%2,X)" },
  { 0x00e3, 0x00ff, ' ', 2, "cp X,(%1,X)" },
  { 0x00f3, 0x00ff, ' ', 1, "cp X,(X)" },

//  { 0x0003, 0x00ff, ' ', 2, "cpl (%1,SP)" },
  { 0x0033, 0x00ff, ' ', 2, "cpl %d" },
  { 0x0043, 0x00ff, ' ', 1, "cpl A" },
  { 0x0063, 0x00ff, ' ', 2, "cpl (%1,X)" },
  { 0x0073, 0x00ff, ' ', 1, "cpl (X)" },
  { 0x0053, 0x00ff, ' ', 1, "cpl X" },

//  { 0x000a, 0x00ff, ' ', 2, "dec (%1,SP)" },
  { 0x003a, 0x00ff, ' ', 2, "dec %d" },
  { 0x004a, 0x00ff, ' ', 1, "dec A" },
  { 0x006a, 0x00ff, ' ', 2, "dec (%1,X)" },
  { 0x007a, 0x00ff, ' ', 1, "dec (X)" },
  { 0x005a, 0x00ff, ' ', 1, "dec X" },

//  { 0x0062, 0x00ff, ' ', 1, "div X,A" },
//  { 0x0065, 0x00ff, ' ', 1, "divw X,Y" },
//  { 0x0031, 0x00ff, ' ', 3, "exg A,%x" },
//  { 0x0041, 0x00ff, ' ', 1, "exg A,XL" },
//  { 0x0051, 0x00ff, ' ', 1, "exgw X,Y" },
//  { 0x0061, 0x00ff, ' ', 1, "exg A,YL" },
  { 0x008e, 0x00ff, ' ', 1, "halt" },

//  { 0x000c, 0x00ff, ' ', 2, "inc (%1,SP)" },
  { 0x003c, 0x00ff, ' ', 2, "inc %d" },
  { 0x004c, 0x00ff, ' ', 1, "inc A" },
  { 0x006c, 0x00ff, ' ', 2, "inc (%1,X)" },
  { 0x007c, 0x00ff, ' ', 1, "inc (X)" },
  { 0x005c, 0x00ff, ' ', 1, "inc X" },

  { 0x0080, 0x00ff, ' ', 1, "iret" },

  { 0x00bc, 0x00ff, ' ', 2, "jp %d" },
  { 0x00cc, 0x00ff, ' ', 3, "jp %x" },
  { 0x00dc, 0x00ff, ' ', 3, "jp (%2,X)" },
  { 0x00ec, 0x00ff, ' ', 2, "jp (%1,X)" },
  { 0x00fc, 0x00ff, ' ', 1, "jp (X)" },
//  { 0x00ad, 0x00ff, ' ', 4, "jpf %e" },
  { 0x0020, 0x00ff, ' ', 2, "jra %p" },

  { 0x0021, 0x00ff, ' ', 2, "jrf %p" },
  { 0x0022, 0x00ff, ' ', 2, "jrugt %p" },
  { 0x0023, 0x00ff, ' ', 2, "jrule %p" },
  { 0x0024, 0x00ff, ' ', 2, "jruge %p" },
  { 0x0025, 0x00ff, ' ', 2, "jrult %p" },
  { 0x0026, 0x00ff, ' ', 2, "jrne %p" },
  { 0x0027, 0x00ff, ' ', 2, "jreq %p" },
  { 0x0028, 0x00ff, ' ', 2, "jrnh %p" },
  { 0x0029, 0x00ff, ' ', 2, "jrh %p" },
  { 0x002a, 0x00ff, ' ', 2, "jrpl %p" },
  { 0x002b, 0x00ff, ' ', 2, "jrmi %p" },
  { 0x002c, 0x00ff, ' ', 2, "jrnm %p" },
  { 0x002d, 0x00ff, ' ', 2, "jrm %p" },
  { 0x002e, 0x00ff, ' ', 2, "jril %p" },
  { 0x002f, 0x00ff, ' ', 2, "jrih %p" },

//  { 0x007b, 0x00ff, ' ', 2, "ld A,(%1,SP)" },
  { 0x00a6, 0x00ff, ' ', 2, "ld A,%b" },
  { 0x00b6, 0x00ff, ' ', 2, "ld A,%d" },
  { 0x00c6, 0x00ff, ' ', 3, "ld A,%x" },
  { 0x00d6, 0x00ff, ' ', 3, "ld A,(%2,X)" },
  { 0x00e6, 0x00ff, ' ', 2, "ld A,(%1,X)" },
  { 0x00f6, 0x00ff, ' ', 1, "ld A,(X)" },
//  { 0x006b, 0x00ff, ' ', 2, "ld (%1,SP),A" },
//  { 0x00a7, 0x00ff, ' ', 2, "ld %b,A" },
  { 0x00b7, 0x00ff, ' ', 2, "ld %d,A" },
  { 0x00c7, 0x00ff, ' ', 3, "ld %x,A" },
  { 0x00d7, 0x00ff, ' ', 3, "ld (%2,X),A" },
  { 0x00e7, 0x00ff, ' ', 2, "ld (%1,X),A" },
  { 0x00f7, 0x00ff, ' ', 1, "ld (X),A" },
  { 0x0093, 0x00ff, ' ', 1, "ld X,Y" },
  { 0x0094, 0x00ff, ' ', 1, "ld S,X" },
  { 0x0095, 0x00ff, ' ', 1, "ld S,A" },
  { 0x0096, 0x00ff, ' ', 1, "ld X,S" },
  { 0x0097, 0x00ff, ' ', 1, "ld X,A" },
  { 0x009e, 0x00ff, ' ', 1, "ld A,S" },
  { 0x009f, 0x00ff, ' ', 1, "ld A,X" },

//  { 0x00a7, 0x00ff, ' ', 2, "ldf (%3,X),A" },
//  { 0x00af, 0x00ff, ' ', 2, "ldf A,(%3,X)" },
//  { 0x00bd, 0x00ff, ' ', 2, "ldf %e,A" },
//  { 0x00bc, 0x00ff, ' ', 2, "ldf A,%e" },
//  { 0x001e, 0x00ff, ' ', 2, "ldw X,(%1,SP)" },
  { 0x00ae, 0x00ff, ' ', 2, "ld X,%b" },
  { 0x00be, 0x00ff, ' ', 2, "ld X,%d" },
  { 0x00ce, 0x00ff, ' ', 3, "ld X,%x" },
  { 0x00de, 0x00ff, ' ', 3, "ld X,(%2,X)" },
  { 0x00ee, 0x00ff, ' ', 2, "ld X,(%1,X)" },
  { 0x00fe, 0x00ff, ' ', 1, "ld X,(X)" },
//  { 0x001f, 0x00ff, ' ', 2, "ldw (%1,SP),X" },
  { 0x00bf, 0x00ff, ' ', 2, "ld %d,X" },
  { 0x00cf, 0x00ff, ' ', 3, "ld %x,X" },
  { 0x00df, 0x00ff, ' ', 3, "ld (%2,X),Y" },
  { 0x00ef, 0x00ff, ' ', 2, "ld (%1,X),Y" },
  { 0x00ff, 0x00ff, ' ', 1, "ld (X),Y" },
//  { 0x0016, 0x00ff, ' ', 2, "ldw Y,(%1,SP)" },
//  { 0x0017, 0x00ff, ' ', 2, "ldw (%1,SP),Y" },

//  { 0x0035, 0x00ff, ' ', 4, "mov %x,%b" },
//  { 0x0045, 0x00ff, ' ', 3, "mov %d,%d" },
//  { 0x0055, 0x00ff, ' ', 5, "mov %x,%x" },
  { 0x0042, 0x00ff, ' ', 1, "mul X,A" },

//  { 0x0000, 0x00ff, ' ', 2, "neg (%1,SP)" },
  { 0x0030, 0x00ff, ' ', 2, "neg %d" },
  { 0x0040, 0x00ff, ' ', 1, "neg A" },
  { 0x0060, 0x00ff, ' ', 2, "neg (%1,X)" },
  { 0x0070, 0x00ff, ' ', 1, "neg (X)" },
  { 0x0050, 0x00ff, ' ', 1, "neg X" },

  { 0x009d, 0x00ff, ' ', 1, "nop" },

//  { 0x001a, 0x00ff, ' ', 2, "or A,(%1,SP)" },
  { 0x00aa, 0x00ff, ' ', 2, "or A,%b" },
  { 0x00ba, 0x00ff, ' ', 2, "or A,%d" },
  { 0x00ca, 0x00ff, ' ', 3, "or A,%x" },
  { 0x00da, 0x00ff, ' ', 3, "or A,(%2,X)" },
  { 0x00ea, 0x00ff, ' ', 2, "or A,(%1,X)" },
  { 0x00fa, 0x00ff, ' ', 1, "or A,(X)" },

  { 0x0084, 0x00ff, ' ', 1, "pop A" },
  { 0x0085, 0x00ff, ' ', 1, "pop X" },
  { 0x0086, 0x00ff, ' ', 1, "pop CC" },
//  { 0x0032, 0x00ff, ' ', 3, "pop %x" },
  { 0x0088, 0x00ff, ' ', 1, "push A" },
  { 0x0089, 0x00ff, ' ', 1, "push X" },
  { 0x008a, 0x00ff, ' ', 1, "push CC" },
//  { 0x003b, 0x00ff, ' ', 3, "push %x" },
//  { 0x004b, 0x00ff, ' ', 2, "push %b" },

  { 0x0098, 0x00ff, ' ', 1, "rcf" },
  { 0x0081, 0x00ff, ' ', 1, "ret" },
//  { 0x0087, 0x00ff, ' ', 1, "retf" },
  { 0x009a, 0x00ff, ' ', 1, "rim" },

//  { 0x0009, 0x00ff, ' ', 2, "rlc (%1,SP)" },
  { 0x0039, 0x00ff, ' ', 2, "rlc %d" },
  { 0x0049, 0x00ff, ' ', 1, "rlc A" },
  { 0x0069, 0x00ff, ' ', 2, "rlc (%1,X)" },
  { 0x0079, 0x00ff, ' ', 1, "rlc (X)" },
  { 0x0059, 0x00ff, ' ', 1, "rlc X" },
//  { 0x0002, 0x00ff, ' ', 1, "rlwa X,A" },
//  { 0x0006, 0x00ff, ' ', 2, "rrc (%1,SP)" },
  { 0x0036, 0x00ff, ' ', 2, "rrc %d" },
  { 0x0046, 0x00ff, ' ', 1, "rrc A" },
  { 0x0066, 0x00ff, ' ', 2, "rrc (%1,X)" },
  { 0x0076, 0x00ff, ' ', 1, "rrc (X)" },
  { 0x0056, 0x00ff, ' ', 1, "rrc X" },
//  { 0x0001, 0x00ff, ' ', 1, "rrwa X,A" },

  { 0x009c, 0x00ff, ' ', 1, "rsp" },

//  { 0x0012, 0x00ff, ' ', 2, "sbc A,(%1,SP)" },
  { 0x00a2, 0x00ff, ' ', 2, "sbc A,%b" },
  { 0x00b2, 0x00ff, ' ', 2, "sbc A,%d" },
  { 0x00c2, 0x00ff, ' ', 3, "sbc A,%x" },
  { 0x00d2, 0x00ff, ' ', 3, "sbc A,(%2,X)" },
  { 0x00e2, 0x00ff, ' ', 2, "sbc A,(%1,X)" },
  { 0x00f2, 0x00ff, ' ', 1, "sbc A,(X)" },

  { 0x0099, 0x00ff, ' ', 1, "scf" },
  { 0x009b, 0x00ff, ' ', 1, "sim" },

//  { 0x0008, 0x00ff, ' ', 2, "sla (%1,SP)" },
  { 0x0038, 0x00ff, ' ', 2, "sla %d" },
  { 0x0048, 0x00ff, ' ', 1, "sla A" },
  { 0x0068, 0x00ff, ' ', 2, "sla (%1,X)" },
  { 0x0078, 0x00ff, ' ', 1, "sla (X)" },
  { 0x0058, 0x00ff, ' ', 1, "sla X" },
//  { 0x0007, 0x00ff, ' ', 2, "sra (%1,SP)" },
  { 0x0037, 0x00ff, ' ', 2, "sra %d" },
  { 0x0047, 0x00ff, ' ', 1, "sra A" },
  { 0x0067, 0x00ff, ' ', 2, "sra (%1,X)" },
  { 0x0077, 0x00ff, ' ', 1, "sra (X)" },
  { 0x0057, 0x00ff, ' ', 1, "sra X" },
//  { 0x0004, 0x00ff, ' ', 2, "srl (%1,SP)" },
  { 0x0034, 0x00ff, ' ', 2, "srl %d" },
  { 0x0044, 0x00ff, ' ', 1, "srl A" },
  { 0x0064, 0x00ff, ' ', 2, "srl (%1,X)" },
  { 0x0074, 0x00ff, ' ', 1, "srl (X)" },
  { 0x0054, 0x00ff, ' ', 1, "srl X" },

//  { 0x0010, 0x00ff, ' ', 2, "sub A,(%1,SP)" },
  { 0x00a0, 0x00ff, ' ', 2, "sub A,%b" },
  { 0x00b0, 0x00ff, ' ', 2, "sub A,%d" },
  { 0x00c0, 0x00ff, ' ', 3, "sub A,%x" },
  { 0x00d0, 0x00ff, ' ', 3, "sub A,(%2,X)" },
  { 0x00e0, 0x00ff, ' ', 2, "sub A,(%1,X)" },
  { 0x00f0, 0x00ff, ' ', 1, "sub A,(X)" },
//  { 0x0052, 0x00ff, ' ', 2, "sub SP,%b" },

//  { 0x001d, 0x00ff, ' ', 3, "subw X,%w" },

//  { 0x000e, 0x00ff, ' ', 2, "swap (%1,SP)" },
  { 0x003e, 0x00ff, ' ', 2, "swap %d" },
  { 0x004e, 0x00ff, ' ', 1, "swap A" },
  { 0x006e, 0x00ff, ' ', 2, "swap (%1,X)" },
  { 0x007e, 0x00ff, ' ', 1, "swap (X)" },
  { 0x005e, 0x00ff, ' ', 1, "swapw X" },

//  { 0x000d, 0x00ff, ' ', 2, "tnz (%1,SP)" },
  { 0x003d, 0x00ff, ' ', 2, "tnz %d" },
  { 0x004d, 0x00ff, ' ', 1, "tnz A" },
  { 0x006d, 0x00ff, ' ', 2, "tnz (%1,X)" },
  { 0x007d, 0x00ff, ' ', 1, "tnz (X)" },
  { 0x005d, 0x00ff, ' ', 1, "tnz X" },

  { 0x0083, 0x00ff, ' ', 1, "trap", true },
  { 0x008f, 0x00ff, ' ', 1, "wfi" },

//  { 0x0018, 0x00ff, ' ', 2, "xor A,(%1,SP)" },
  { 0x00a8, 0x00ff, ' ', 2, "xor A,%b" },
  { 0x00b8, 0x00ff, ' ', 2, "xor A,%d" },
  { 0x00c8, 0x00ff, ' ', 3, "xor A,%x" },
  { 0x00d8, 0x00ff, ' ', 3, "xor A,(%2,X)" },
  { 0x00e8, 0x00ff, ' ', 2, "xor A,(%1,X)" },
  { 0x00f8, 0x00ff, ' ', 1, "xor A,(X)" },

  { 0, 0, 0, 0, NULL }
};

struct dis_entry disass_st7_90[]= {

  { 0x00d9, 0x00ff, ' ', 3, "adc A,(%2,Y)" },
  { 0x00e9, 0x00ff, ' ', 2, "adc A,(%1,Y)" },
  { 0x00f9, 0x00ff, ' ', 1, "adc A,(Y)" },
  { 0x00db, 0x00ff, ' ', 3, "add A,(%2,Y)" },
  { 0x00eb, 0x00ff, ' ', 2, "add A,(%1,Y)" },
  { 0x00fb, 0x00ff, ' ', 1, "add A,(Y)" },
  { 0x00d4, 0x00ff, ' ', 3, "and A,(%2,Y)" },
  { 0x00e4, 0x00ff, ' ', 2, "and A,(%1,Y)" },
  { 0x00f4, 0x00ff, ' ', 1, "and A,(Y)" },
  { 0x00d5, 0x00ff, ' ', 3, "bcp A,(%2,Y)" },
  { 0x00e5, 0x00ff, ' ', 2, "bcp A,(%1,Y)" },
  { 0x00f5, 0x00ff, ' ', 1, "bcp A,(Y)" },

//  { 0x0011, 0x00ff, ' ', 3, "bccm %d,#0" },
//  { 0x0013, 0x00ff, ' ', 3, "bccm %d,#1" },
//  { 0x0015, 0x00ff, ' ', 3, "bccm %d,#2" },
//  { 0x0017, 0x00ff, ' ', 3, "bccm %d,#3" },
//  { 0x0019, 0x00ff, ' ', 3, "bccm %d,#4" },
//  { 0x001b, 0x00ff, ' ', 3, "bccm %d,#5" },
//  { 0x001d, 0x00ff, ' ', 3, "bccm %d,#6" },
//  { 0x001f, 0x00ff, ' ', 3, "bccm %d,#7" },

//  { 0x0010, 0x00ff, ' ', 3, "bcpl %d,#0" },
//  { 0x0012, 0x00ff, ' ', 3, "bcpl %d,#1" },
//  { 0x0014, 0x00ff, ' ', 3, "bcpl %d,#2" },
//  { 0x0016, 0x00ff, ' ', 3, "bcpl %d,#3" },
//  { 0x0018, 0x00ff, ' ', 3, "bcpl %d,#4" },
//  { 0x001a, 0x00ff, ' ', 3, "bcpl %d,#5" },
//  { 0x001c, 0x00ff, ' ', 3, "bcpl %d,#6" },
//  { 0x001e, 0x00ff, ' ', 3, "bcpl %d,#7" },

  { 0x00dd, 0x00ff, ' ', 3, "call (%2,Y)", true },
  { 0x00ed, 0x00ff, ' ', 2, "call (%1,Y)", true },
  { 0x00fd, 0x00ff, ' ', 1, "call (Y)", true },

//  { 0x004f, 0x00ff, ' ', 3, "clr (%2,Y)" },
  { 0x006f, 0x00ff, ' ', 2, "clr (%1,Y)" },
  { 0x007f, 0x00ff, ' ', 1, "clr (Y)" },
  { 0x005f, 0x00ff, ' ', 1, "clr Y" },

  { 0x00d1, 0x00ff, ' ', 3, "cp A,(%2,Y)" },
  { 0x00e1, 0x00ff, ' ', 2, "cp A,(%1,Y)" },
  { 0x00f1, 0x00ff, ' ', 1, "cp A,(Y)" },
  { 0x00a3, 0x00ff, ' ', 2, "cp Y,%b" },
  { 0x00b3, 0x00ff, ' ', 2, "cp Y,%d" },
  { 0x00c3, 0x00ff, ' ', 3, "cp Y,%x" },
  { 0x00d3, 0x00ff, ' ', 3, "cp Y,(%2,Y)" },
  { 0x00e3, 0x00ff, ' ', 2, "cp Y,(%1,Y)" },
  { 0x00f3, 0x00ff, ' ', 1, "cp Y,(Y)" },

//  { 0x0043, 0x00ff, ' ', 3, "cpl (%2,Y)" },
  { 0x0063, 0x00ff, ' ', 2, "cpl (%1,Y)" },
  { 0x0073, 0x00ff, ' ', 1, "cpl (Y)" },
  { 0x0053, 0x00ff, ' ', 1, "cpl Y" },

//  { 0x004a, 0x00ff, ' ', 3, "dec (%2,Y)" },
  { 0x006a, 0x00ff, ' ', 2, "dec (%1,Y)" },
  { 0x007a, 0x00ff, ' ', 1, "dec (Y)" },
  { 0x005a, 0x00ff, ' ', 1, "dec Y" },

//  { 0x0062, 0x00ff, ' ', 1, "div Y,A" },

//  { 0x004c, 0x00ff, ' ', 3, "inc (%2,Y)" },
  { 0x006c, 0x00ff, ' ', 2, "inc (%1,Y)" },
  { 0x007c, 0x00ff, ' ', 1, "inc (Y)" },
  { 0x005c, 0x00ff, ' ', 1, "inc Y" },

  { 0x00dc, 0x00ff, ' ', 3, "jp (%2,Y)" },
  { 0x00ec, 0x00ff, ' ', 2, "jp (%1,Y)" },
  { 0x00fc, 0x00ff, ' ', 1, "jp (Y)" },

  //{ 0x0028, 0x00ff, ' ', 2, "jrnh %p" },
  //{ 0x0029, 0x00ff, ' ', 2, "jrh %p" },
  //{ 0x002c, 0x00ff, ' ', 2, "jrnm %p" },
  //{ 0x002d, 0x00ff, ' ', 2, "jrm %p" },
  //{ 0x002e, 0x00ff, ' ', 2, "jril %p" },
  //{ 0x002f, 0x00ff, ' ', 2, "jrih %p" },

  { 0x00d6, 0x00ff, ' ', 3, "ld A,(%2,Y)" },
  { 0x00e6, 0x00ff, ' ', 2, "ld A,(%1,Y)" },
  { 0x00f6, 0x00ff, ' ', 1, "ld A,(Y)" },
  { 0x00d7, 0x00ff, ' ', 3, "ld (%2,Y),A" },
  { 0x00e7, 0x00ff, ' ', 2, "ld (%1,Y),A" },
  { 0x00f7, 0x00ff, ' ', 1, "ld (Y),A" },

  { 0x0093, 0x00ff, ' ', 1, "ld Y,X" },
  { 0x0094, 0x00ff, ' ', 1, "ld S,Y" },
  { 0x0096, 0x00ff, ' ', 1, "ld Y,S" },
  { 0x0097, 0x00ff, ' ', 1, "ld Y,A" },
  { 0x009f, 0x00ff, ' ', 1, "ld A,Y" },
//  { 0x00a7, 0x00ff, ' ', 2, "ldf (%e,Y),A" },
//  { 0x00af, 0x00ff, ' ', 2, "ldf A,(%e,Y)" },
  { 0x00ae, 0x00ff, ' ', 2, "ld Y,%b" },
  { 0x00be, 0x00ff, ' ', 2, "ld Y,%d" },
  { 0x00ce, 0x00ff, ' ', 3, "ld Y,%x" },
  { 0x00de, 0x00ff, ' ', 3, "ld Y,(%2,Y)" },
  { 0x00ee, 0x00ff, ' ', 2, "ld Y,(%1,Y)" },
  { 0x00fe, 0x00ff, ' ', 1, "ld Y,(Y)" },

  { 0x00bf, 0x00ff, ' ', 2, "ld %d,Y" },
  { 0x00cf, 0x00ff, ' ', 3, "ld %x,Y" },
  { 0x00df, 0x00ff, ' ', 3, "ld (%2,Y),Y" },
  { 0x00ef, 0x00ff, ' ', 2, "ld (%1,Y),Y" },
  { 0x00ff, 0x00ff, ' ', 1, "ld (Y),Y" },

  { 0x0042, 0x00ff, ' ', 1, "mul Y,A" },

//  { 0x0040, 0x00ff, ' ', 3, "neg (%2,Y)" },
  { 0x0060, 0x00ff, ' ', 2, "neg (%1,Y)" },
  { 0x0070, 0x00ff, ' ', 1, "neg (Y)" },
  { 0x0050, 0x00ff, ' ', 1, "neg Y" },

  { 0x00da, 0x00ff, ' ', 3, "or A,(%2,Y)" },
  { 0x00ea, 0x00ff, ' ', 2, "or A,(%1,Y)" },
  { 0x00fa, 0x00ff, ' ', 1, "or A,(Y)" },

  { 0x0085, 0x00ff, ' ', 1, "pop Y" },
  { 0x0089, 0x00ff, ' ', 1, "push Y" },

//  { 0x0049, 0x00ff, ' ', 3, "rlc (%2,Y)" },
  { 0x0069, 0x00ff, ' ', 2, "rlc (%1,Y)" },
  { 0x0079, 0x00ff, ' ', 1, "rlc (Y)" },
  { 0x0059, 0x00ff, ' ', 1, "rlc Y" },
//  { 0x0002, 0x00ff, ' ', 1, "rlwa Y,A" },
//  { 0x0046, 0x00ff, ' ', 3, "rrc (%2,Y)" },
  { 0x0066, 0x00ff, ' ', 2, "rrc (%1,Y)" },
  { 0x0076, 0x00ff, ' ', 1, "rrc (Y)" },
  { 0x0056, 0x00ff, ' ', 1, "rrc Y" },
//  { 0x0001, 0x00ff, ' ', 1, "rrwa Y,A" },
  
  { 0x00d2, 0x00ff, ' ', 3, "sbc A,(%2,Y)" },
  { 0x00e2, 0x00ff, ' ', 2, "sbc A,(%1,Y)" },
  { 0x00f2, 0x00ff, ' ', 1, "sbc A,(Y)" },

//  { 0x0048, 0x00ff, ' ', 3, "sla (%2,Y)" },
  { 0x0068, 0x00ff, ' ', 2, "sla (%1,Y)" },
  { 0x0078, 0x00ff, ' ', 1, "sla (Y)" },
  { 0x0058, 0x00ff, ' ', 1, "sla Y" },
//  { 0x0047, 0x00ff, ' ', 3, "sra (%2,Y)" },
  { 0x0067, 0x00ff, ' ', 2, "sra (%1,Y)" },
  { 0x0077, 0x00ff, ' ', 1, "sra (Y)" },
  { 0x0057, 0x00ff, ' ', 1, "sra Y" },
//  { 0x0044, 0x00ff, ' ', 3, "srl (%2,Y)" },
  { 0x0064, 0x00ff, ' ', 2, "srl (%1,Y)" },
  { 0x0074, 0x00ff, ' ', 1, "srl (Y)" },
  { 0x0054, 0x00ff, ' ', 1, "srl Y" },

  { 0x00d0, 0x00ff, ' ', 3, "sub A,(%2,Y)" },
  { 0x00e0, 0x00ff, ' ', 2, "sub A,(%1,Y)" },
  { 0x00f0, 0x00ff, ' ', 1, "sub A,(Y)" },

//  { 0x004e, 0x00ff, ' ', 3, "swap (%2,Y)" },
  { 0x006e, 0x00ff, ' ', 2, "swap (%1,Y)" },
  { 0x007e, 0x00ff, ' ', 1, "swap (Y)" },
  { 0x005e, 0x00ff, ' ', 1, "swap Y" },

//  { 0x004d, 0x00ff, ' ', 3, "tnz (%2,Y)" },
  { 0x006d, 0x00ff, ' ', 2, "tnz (%1,Y)" },
  { 0x007d, 0x00ff, ' ', 1, "tnz (Y)" },
  { 0x005d, 0x00ff, ' ', 1, "tnz Y" },

  { 0x00d8, 0x00ff, ' ', 3, "xor A,(%2,Y)" },
  { 0x00e8, 0x00ff, ' ', 2, "xor A,(%1,Y)" },
  { 0x00f8, 0x00ff, ' ', 1, "xor A,(Y)" },

  { 0, 0, 0, 0, NULL }
};

struct dis_entry disass_st7_91[]= {

  { 0x00d9, 0x00ff, ' ', 2, "adc A,([%1.w],Y)" },
  { 0x00e9, 0x00ff, ' ', 2, "adc A,([%1],Y)" },
  { 0x00db, 0x00ff, ' ', 2, "add A,([%1.w],Y)" },
  { 0x00eb, 0x00ff, ' ', 2, "add A,([%1],Y)" },
  { 0x00d4, 0x00ff, ' ', 2, "and A,([%1.w],Y)" },
  { 0x00e4, 0x00ff, ' ', 2, "and A,([%1],Y)" },
  { 0x00d5, 0x00ff, ' ', 2, "bcp A,([%1.w],Y)" },
  { 0x00e5, 0x00ff, ' ', 2, "bcp A,([%1],Y)" },
  { 0x00dd, 0x00ff, ' ', 2, "call ([%1.w],Y)", true },
  { 0x00ed, 0x00ff, ' ', 2, "call ([%1],Y)", true },
  { 0x006f, 0x00ff, ' ', 2, "clr ([%1],Y)" },
  { 0x00d1, 0x00ff, ' ', 2, "cp A,([%1.w],Y)" },
  { 0x00e1, 0x00ff, ' ', 2, "cp A,([%1],Y)" },
  { 0x00b3, 0x00ff, ' ', 2, "cp Y,[%1]" },
  { 0x00c3, 0x00ff, ' ', 2, "cp Y,[%1.w]" },
  { 0x00d3, 0x00ff, ' ', 2, "cp Y,([%1.w],Y)" },
  { 0x00e3, 0x00ff, ' ', 2, "cp Y,([%1],Y)" },
  { 0x0063, 0x00ff, ' ', 2, "cpl ([%1],Y)" },
  { 0x006a, 0x00ff, ' ', 2, "dec ([%1],Y)" },
  { 0x006c, 0x00ff, ' ', 2, "inc ([%1],Y)" },
  { 0x00dc, 0x00ff, ' ', 2, "jp ([%1.w],Y)" },
  { 0x00ec, 0x00ff, ' ', 2, "jp ([%1],Y)" },
  { 0x00d6, 0x00ff, ' ', 2, "ld A,([%1.w],Y)" },
  { 0x00e6, 0x00ff, ' ', 2, "ld A,([%1],Y)" },
  { 0x00d7, 0x00ff, ' ', 2, "ld ([%1.w],Y),A" },
  { 0x00e7, 0x00ff, ' ', 2, "ld ([%1],Y),A" },
//  { 0x00a7, 0x00ff, ' ', 2, "ldf ([%2.e],Y),A" },
//  { 0x00af, 0x00ff, ' ', 2, "ldf A,([%2.e],Y)" },
  { 0x00be, 0x00ff, ' ', 3, "ld Y,[%1]" },
  { 0x00ce, 0x00ff, ' ', 3, "ld Y,[%1.w]" },
  { 0x00de, 0x00ff, ' ', 3, "ld Y,([%1.w],Y)" },
  { 0x00ee, 0x00ff, ' ', 3, "ld Y,([%1],Y)" },
  { 0x00bf, 0x00ff, ' ', 3, "ld [%1],Y" },
  { 0x00cf, 0x00ff, ' ', 3, "ld [%1.w],Y" },
  { 0x00df, 0x00ff, ' ', 3, "ld ([%1.w],Y),Y" },
  { 0x00ef, 0x00ff, ' ', 3, "ld ([%1],Y),Y" },
  { 0x0060, 0x00ff, ' ', 2, "neg ([%1],Y)" },
  { 0x00da, 0x00ff, ' ', 2, "or A,([%1.w],Y)" },
  { 0x00ea, 0x00ff, ' ', 2, "or A,([%1],Y)" },
  { 0x0069, 0x00ff, ' ', 2, "rlc ([%1],Y)" },
  { 0x0066, 0x00ff, ' ', 2, "rrc ([%1],Y)" },
  { 0x00d2, 0x00ff, ' ', 2, "sbc A,([%1.w],Y)" },
  { 0x00e2, 0x00ff, ' ', 2, "sbc A,([%1],Y)" },
  { 0x0068, 0x00ff, ' ', 2, "sla ([%1],Y)" },
  { 0x0067, 0x00ff, ' ', 2, "sra ([%1],Y)" },
  { 0x0064, 0x00ff, ' ', 2, "srl ([%1],Y)" },
  { 0x00d0, 0x00ff, ' ', 2, "sub A,([%1.w],Y)" },
  { 0x00e0, 0x00ff, ' ', 2, "sub A,([%1],Y)" },
  { 0x006e, 0x00ff, ' ', 2, "swap ([%1],Y)" },
  { 0x006d, 0x00ff, ' ', 2, "tnz ([%1],Y)" },
  { 0x00d8, 0x00ff, ' ', 2, "xor A,([%1.w],Y)" },
  { 0x00e8, 0x00ff, ' ', 2, "xor A,([%1],Y)" },

  { 0, 0, 0, 0, NULL }
};

struct dis_entry disass_st7_92[]= {

  { 0x00b9, 0x00ff, ' ', 2, "adc A,[%1]" },
  { 0x00c9, 0x00ff, ' ', 2, "adc A,[%1.w]" },
  { 0x00d9, 0x00ff, ' ', 2, "adc A,([%1.w],X)" },
  { 0x00e9, 0x00ff, ' ', 2, "adc A,([%1],X)" },
  { 0x00bb, 0x00ff, ' ', 2, "add A,[%1]" },
  { 0x00cb, 0x00ff, ' ', 2, "add A,[%1.w]" },
  { 0x00db, 0x00ff, ' ', 2, "add A,([%1.w],X)" },
  { 0x00eb, 0x00ff, ' ', 2, "add A,([%1],X)" },
  { 0x00b4, 0x00ff, ' ', 2, "and A,[%1]" },
  { 0x00c4, 0x00ff, ' ', 2, "and A,[%1.w]" },
  { 0x00d4, 0x00ff, ' ', 2, "and A,([%1.w],X)" },
  { 0x00e4, 0x00ff, ' ', 2, "and A,([%1],X)" },
  { 0x00b5, 0x00ff, ' ', 2, "bcp A,[%1]" },
  { 0x00c5, 0x00ff, ' ', 2, "bcp A,[%1.w]" },
  { 0x00d5, 0x00ff, ' ', 2, "bcp A,([%1.w],X)" },
  { 0x00e5, 0x00ff, ' ', 2, "bcp A,([%1],X)" },

  { 0x0011, 0x00ff, ' ', 2, "bres [%1],#0" },
  { 0x0013, 0x00ff, ' ', 2, "bres [%1],#1" },
  { 0x0015, 0x00ff, ' ', 2, "bres [%1],#2" },
  { 0x0017, 0x00ff, ' ', 2, "bres [%1],#3" },
  { 0x0019, 0x00ff, ' ', 2, "bres [%1],#4" },
  { 0x001b, 0x00ff, ' ', 2, "bres [%1],#5" },
  { 0x001d, 0x00ff, ' ', 2, "bres [%1],#6" },
  { 0x001f, 0x00ff, ' ', 2, "bres [%1],#7" },

  { 0x0010, 0x00ff, ' ', 2, "bset [%1],#0" },
  { 0x0012, 0x00ff, ' ', 2, "bset [%1],#1" },
  { 0x0014, 0x00ff, ' ', 2, "bset [%1],#2" },
  { 0x0016, 0x00ff, ' ', 2, "bset [%1],#3" },
  { 0x0018, 0x00ff, ' ', 2, "bset [%1],#4" },
  { 0x001a, 0x00ff, ' ', 2, "bset [%1],#5" },
  { 0x001c, 0x00ff, ' ', 2, "bset [%1],#6" },
  { 0x001e, 0x00ff, ' ', 2, "bset [%1],#7" },

  { 0x0001, 0x00ff, ' ', 3, "bjtf [%1],#0,%p" },
  { 0x0003, 0x00ff, ' ', 3, "bjtf [%1],#1,%p" },
  { 0x0005, 0x00ff, ' ', 3, "bjtf [%1],#2,%p" },
  { 0x0007, 0x00ff, ' ', 3, "bjtf [%1],#3,%p" },
  { 0x0009, 0x00ff, ' ', 3, "bjtf [%1],#4,%p" },
  { 0x000b, 0x00ff, ' ', 3, "bjtf [%1],#5,%p" },
  { 0x000d, 0x00ff, ' ', 3, "bjtf [%1],#6,%p" },
  { 0x000f, 0x00ff, ' ', 3, "bjtf [%1],#7,%p" },

  { 0x0000, 0x00ff, ' ', 3, "btjt [%1],#0,%p" },
  { 0x0002, 0x00ff, ' ', 3, "btjt [%1],#1,%p" },
  { 0x0004, 0x00ff, ' ', 3, "btjt [%1],#2,%p" },
  { 0x0006, 0x00ff, ' ', 3, "btjt [%1],#3,%p" },
  { 0x0008, 0x00ff, ' ', 3, "btjt [%1],#4,%p" },
  { 0x000a, 0x00ff, ' ', 3, "btjt [%1],#5,%p" },
  { 0x000c, 0x00ff, ' ', 3, "btjt [%1],#6,%p" },
  { 0x000e, 0x00ff, ' ', 3, "btjt [%1],#7,%p" },

  { 0x00cd, 0x00ff, ' ', 2, "call [%1]", true },
  { 0x00cd, 0x00ff, ' ', 2, "call [%1.w]", true },
  { 0x00dd, 0x00ff, ' ', 2, "call ([%1.w],X)", true },
  { 0x00dd, 0x00ff, ' ', 2, "call ([%1],X)", true },
  { 0x00ad, 0x00ff, ' ', 2, "callr [%1]", true },
//  { 0x008d, 0x00ff, ' ', 3, "callf [%2.e]", true },
  { 0x003f, 0x00ff, ' ', 2, "clr [%1]" },
  { 0x006f, 0x00ff, ' ', 2, "clr ([%1],X)" },
  { 0x00b1, 0x00ff, ' ', 2, "cp A,[%1]" },
  { 0x00c1, 0x00ff, ' ', 2, "cp A,[%1.w]" },
  { 0x00d1, 0x00ff, ' ', 2, "cp A,([%1.w],X)" },
  { 0x00e1, 0x00ff, ' ', 2, "cp A,([%1],X)" },
  { 0x00b3, 0x00ff, ' ', 2, "cp X,[%1.w]" },
  { 0x00c3, 0x00ff, ' ', 2, "cp X,[%1.w]" },
  { 0x00d3, 0x00ff, ' ', 2, "cp X,([%1.w],X)" },
  { 0x00e3, 0x00ff, ' ', 2, "cp X,([%1.w],X)" },
  { 0x0033, 0x00ff, ' ', 2, "cpl [%1]" },
  { 0x0063, 0x00ff, ' ', 2, "cpl ([%1],X)" },
  { 0x003a, 0x00ff, ' ', 2, "dec [%1]" },
  { 0x006a, 0x00ff, ' ', 2, "dec ([%1],X)" },
  { 0x003c, 0x00ff, ' ', 2, "inc [%1]" },
  { 0x006c, 0x00ff, ' ', 2, "inc ([%1],X)" },
  { 0x00bc, 0x00ff, ' ', 2, "jp [%1]" },
  { 0x00cc, 0x00ff, ' ', 2, "jp [%1.w]" },
  { 0x00dc, 0x00ff, ' ', 2, "jp ([%1.w],X)" },
  { 0x00ec, 0x00ff, ' ', 2, "jp ([%1],X)" },
//  { 0x00ac, 0x00ff, ' ', 3, "jpf [%2.e]" },
  { 0x0020, 0x00ff, ' ', 2, "jra [%1]" },

  { 0x0021, 0x00ff, ' ', 2, "jrf [%1]" },
  { 0x0022, 0x00ff, ' ', 2, "jrugt [%1]" },
  { 0x0023, 0x00ff, ' ', 2, "jrule [%1]" },
  { 0x0024, 0x00ff, ' ', 2, "jruge [%1]" },
  { 0x0025, 0x00ff, ' ', 2, "jrult [%1]" },
  { 0x0026, 0x00ff, ' ', 2, "jrne [%1]" },
  { 0x0027, 0x00ff, ' ', 2, "jreq [%1]" },
  { 0x0028, 0x00ff, ' ', 2, "jrnh [%1]" },
  { 0x0029, 0x00ff, ' ', 2, "jrh [%1]" },
  { 0x002a, 0x00ff, ' ', 2, "jrpl [%1]" },
  { 0x002b, 0x00ff, ' ', 2, "jrmi [%1]" },
  { 0x002c, 0x00ff, ' ', 2, "jrnm [%1]" },
  { 0x002d, 0x00ff, ' ', 2, "jrm [%1]" },
  { 0x002e, 0x00ff, ' ', 2, "jril [%1]" },
  { 0x002f, 0x00ff, ' ', 2, "jrih [%1]" },

  { 0x00b6, 0x00ff, ' ', 2, "ld A,[%1]" },
  { 0x00c6, 0x00ff, ' ', 2, "ld A,[%1.w]" },
  { 0x00d6, 0x00ff, ' ', 2, "ld A,([%1.w],X)" },
  { 0x00e6, 0x00ff, ' ', 2, "ld A,([%1],X)" },
  { 0x00b7, 0x00ff, ' ', 2, "ld [%1],A" },
  { 0x00c7, 0x00ff, ' ', 2, "ld [%1.w],A" },
  { 0x00d7, 0x00ff, ' ', 2, "ld ([%1.w],X),A" },
  { 0x00e7, 0x00ff, ' ', 2, "ld ([%1],X),A" },
//  { 0x00a7, 0x00ff, ' ', 2, "ldf ([%2.e],X),A" },
//  { 0x00af, 0x00ff, ' ', 2, "ldf A,([%2.e],X)" },
  { 0x00be, 0x00ff, ' ', 2, "ld X,[%1]" },
  { 0x00ce, 0x00ff, ' ', 2, "ld X,[%1.w]" },
  { 0x00de, 0x00ff, ' ', 2, "ld X,([%1.w],X)" },
  { 0x00ee, 0x00ff, ' ', 2, "ld X,([%1],X)" },
  { 0x00bf, 0x00ff, ' ', 2, "ld [%1],X" },
  { 0x00cf, 0x00ff, ' ', 2, "ld [%1.w],X" },
  { 0x00df, 0x00ff, ' ', 2, "ld ([%1.w],X),X" },
  { 0x00ef, 0x00ff, ' ', 2, "ld ([%1],X),X" },

  { 0x0030, 0x00ff, ' ', 2, "neg [%1]" },
  { 0x0060, 0x00ff, ' ', 2, "neg ([%1],X)" },
  { 0x00ba, 0x00ff, ' ', 2, "or A,[%1]" },
  { 0x00ca, 0x00ff, ' ', 2, "or A,[%1.w]" },
  { 0x00da, 0x00ff, ' ', 2, "or A,([%1.w],X)" },
  { 0x00ea, 0x00ff, ' ', 2, "or A,([%1],X)" },
  { 0x0039, 0x00ff, ' ', 2, "rlc [%1]" },
  { 0x0069, 0x00ff, ' ', 2, "rlc ([%1],X)" },
  { 0x0036, 0x00ff, ' ', 2, "rrc [%1]" },
  { 0x0066, 0x00ff, ' ', 2, "rrc ([%1],X)" },
  { 0x00b2, 0x00ff, ' ', 2, "sbc A,[%1]" },
  { 0x00c2, 0x00ff, ' ', 2, "sbc A,[%1.w]" },
  { 0x00d2, 0x00ff, ' ', 2, "sbc A,([%1.w],X)" },
  { 0x00e2, 0x00ff, ' ', 2, "sbc A,([%1],X)" },
  { 0x0038, 0x00ff, ' ', 2, "sla [%1]" },
  { 0x0068, 0x00ff, ' ', 2, "sla ([%1],X)" },
  { 0x0037, 0x00ff, ' ', 2, "sra [%1]" },
  { 0x0067, 0x00ff, ' ', 2, "sra ([%1],X)" },
  { 0x0034, 0x00ff, ' ', 2, "srl [%1]" },
  { 0x0064, 0x00ff, ' ', 2, "srl ([%1],X)" },
  { 0x00b0, 0x00ff, ' ', 2, "sub A,[%1]" },
  { 0x00c0, 0x00ff, ' ', 2, "sub A,[%1.w]" },
  { 0x00d0, 0x00ff, ' ', 2, "sub A,([%1.w],X)" },
  { 0x00e0, 0x00ff, ' ', 2, "sub A,([%1],X)" },
  { 0x003e, 0x00ff, ' ', 2, "swap [%1]" },
  { 0x006e, 0x00ff, ' ', 2, "swap ([%1],X)" },
  { 0x003d, 0x00ff, ' ', 2, "tnz [%1]" },
  { 0x006d, 0x00ff, ' ', 2, "tnz ([%1],X)" },
  { 0x00b8, 0x00ff, ' ', 2, "xor A,[%1]" },
  { 0x00c8, 0x00ff, ' ', 2, "xor A,[%1.w]" },
  { 0x00d8, 0x00ff, ' ', 2, "xor A,([%1.w],X)" },
  { 0x00e8, 0x00ff, ' ', 2, "xor A,([%1],X)" },

  { 0, 0, 0, 0, NULL }
};

/* glob.cc */
