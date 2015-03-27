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


/*
%d - direct addressing
%x - extended addressing
%p - pc relative addressing
%b - unsigned byte immediate addressing
%w - unsigned word immediate addressing
%s - signed byte immediate
%1 - unsigned byte index offset
%2 - unsigned word index offset
*/

/*  uint  code, mask;  char  branch;  uchar length;  char  *mnemonic; */
struct dis_entry disass_hc08[]= {
  { 0x0000, 0x00ff, 'R', 3, "brset #0,%d,%p" },
  { 0x0001, 0x00ff, 'R', 3, "brclr #0,%d,%p" },
  { 0x0002, 0x00ff, 'R', 3, "brset #1,%d,%p" },
  { 0x0003, 0x00ff, 'R', 3, "brclr #1,%d,%p" },
  { 0x0004, 0x00ff, 'R', 3, "brset #2,%d,%p" },
  { 0x0005, 0x00ff, 'R', 3, "brclr #2,%d,%p" },
  { 0x0006, 0x00ff, 'R', 3, "brset #3,%d,%p" },
  { 0x0007, 0x00ff, 'R', 3, "brclr #3,%d,%p" },
  { 0x0008, 0x00ff, 'R', 3, "brset #4,%d,%p" },
  { 0x0009, 0x00ff, 'R', 3, "brclr #4,%d,%p" },
  { 0x000a, 0x00ff, 'R', 3, "brset #5,%d,%p" },
  { 0x000b, 0x00ff, 'R', 3, "brclr #5,%d,%p" },
  { 0x000c, 0x00ff, 'R', 3, "brset #6,%d,%p" },
  { 0x000d, 0x00ff, 'R', 3, "brclr #6,%d,%p" },
  { 0x000e, 0x00ff, 'R', 3, "brset #7,%d,%p" },
  { 0x000f, 0x00ff, 'R', 3, "brclr #7,%d,%p" },

  { 0x0010, 0x00ff, ' ', 2, "bset #0,%d" },
  { 0x0011, 0x00ff, ' ', 2, "bclr #0,%d" },
  { 0x0012, 0x00ff, ' ', 2, "bset #1,%d" },
  { 0x0013, 0x00ff, ' ', 2, "bclr #1,%d" },
  { 0x0014, 0x00ff, ' ', 2, "bset #2,%d" },
  { 0x0015, 0x00ff, ' ', 2, "bclr #2,%d" },
  { 0x0016, 0x00ff, ' ', 2, "bset #3,%d" },
  { 0x0017, 0x00ff, ' ', 2, "bclr #3,%d" },
  { 0x0018, 0x00ff, ' ', 2, "bset #4,%d" },
  { 0x0019, 0x00ff, ' ', 2, "bclr #4,%d" },
  { 0x001a, 0x00ff, ' ', 2, "bset #5,%d" },
  { 0x001b, 0x00ff, ' ', 2, "bclr #5,%d" },
  { 0x001c, 0x00ff, ' ', 2, "bset #6,%d" },
  { 0x001d, 0x00ff, ' ', 2, "bclr #6,%d" },
  { 0x001e, 0x00ff, ' ', 2, "bset #7,%d" },
  { 0x001f, 0x00ff, ' ', 2, "bclr #7,%d" },

  { 0x0020, 0x00ff, 'R', 2, "bra %p" },
  { 0x0021, 0x00ff, 'R', 2, "brn %p" },
  { 0x0022, 0x00ff, 'R', 2, "bhi %p" },
  { 0x0023, 0x00ff, 'R', 2, "bls %p" },
  { 0x0024, 0x00ff, 'R', 2, "bcc %p" },
  { 0x0025, 0x00ff, 'R', 2, "bcs %p" },
  { 0x0026, 0x00ff, 'R', 2, "bne %p" },
  { 0x0027, 0x00ff, 'R', 2, "beq %p" },
  { 0x0028, 0x00ff, 'R', 2, "bhcc %p" },
  { 0x0029, 0x00ff, 'R', 2, "bhcs %p" },
  { 0x002a, 0x00ff, 'R', 2, "bpl %p" },
  { 0x002b, 0x00ff, 'R', 2, "bmi %p" },
  { 0x002c, 0x00ff, 'R', 2, "bmc %p" },
  { 0x002d, 0x00ff, 'R', 2, "bms %p" },
  { 0x002e, 0x00ff, 'R', 2, "bil %p" },
  { 0x002f, 0x00ff, 'R', 2, "bih %p" },

  { 0x0030, 0x00ff, ' ', 2, "neg %d" },
  { 0x0031, 0x00ff, 'R', 3, "cbeq %d,%p" },
  { 0x0032, 0x00ff, ' ', 3, "ldhx %x" },  //HCS08 only
  { 0x0033, 0x00ff, ' ', 2, "com %d" },
  { 0x0034, 0x00ff, ' ', 2, "lsr %d" },
  { 0x0035, 0x00ff, ' ', 2, "sthx %d" },
  { 0x0036, 0x00ff, ' ', 2, "ror %d" },
  { 0x0037, 0x00ff, ' ', 2, "asr %d" },
  { 0x0038, 0x00ff, ' ', 2, "lsl %d" },
  { 0x0039, 0x00ff, ' ', 2, "rol %d" },
  { 0x003a, 0x00ff, ' ', 2, "dec %d" },
  { 0x003b, 0x00ff, 'R', 3, "dbnz %d,%d" },
  { 0x003c, 0x00ff, ' ', 2, "inc %d" },
  { 0x003d, 0x00ff, ' ', 2, "tst %d" },
  { 0x003e, 0x00ff, ' ', 3, "cphx %x" },  //HCS08 only
  { 0x003f, 0x00ff, ' ', 2, "clr %d" },

  { 0x0040, 0x00ff, ' ', 1, "nega" },
  { 0x0041, 0x00ff, 'R', 3, "cbeqa %b,%p" },
  { 0x0042, 0x00ff, ' ', 1, "mul" },
  { 0x0043, 0x00ff, ' ', 1, "coma" },
  { 0x0044, 0x00ff, ' ', 1, "lsra" },
  { 0x0045, 0x00ff, ' ', 3, "ldhx %w" },
  { 0x0046, 0x00ff, ' ', 1, "rora" },
  { 0x0047, 0x00ff, ' ', 1, "asra" },
  { 0x0048, 0x00ff, ' ', 1, "lsla" },
  { 0x0049, 0x00ff, ' ', 1, "rola" },
  { 0x004a, 0x00ff, ' ', 1, "deca" },
  { 0x004b, 0x00ff, 'R', 2, "dbnza %p" },
  { 0x004c, 0x00ff, ' ', 1, "inca" },
  { 0x004d, 0x00ff, ' ', 1, "tsta" },
  { 0x004e, 0x00ff, ' ', 3, "mov %d,%d" },
  { 0x004f, 0x00ff, ' ', 1, "clra" },

  { 0x0050, 0x00ff, ' ', 1, "negx" },
  { 0x0051, 0x00ff, 'R', 3, "cbeqx %b,%p" },
  { 0x0052, 0x00ff, ' ', 1, "div" },
  { 0x0053, 0x00ff, ' ', 1, "comx" },
  { 0x0054, 0x00ff, ' ', 1, "lsrx" },
  { 0x0055, 0x00ff, ' ', 2, "ldhx %d" },
  { 0x0056, 0x00ff, ' ', 1, "rorx" },
  { 0x0057, 0x00ff, ' ', 1, "asrx" },
  { 0x0058, 0x00ff, ' ', 1, "lslx" },
  { 0x0059, 0x00ff, ' ', 1, "rolx" },
  { 0x005a, 0x00ff, ' ', 1, "decx" },
  { 0x005b, 0x00ff, 'R', 2, "dbnzx %p" },
  { 0x005c, 0x00ff, ' ', 1, "incx" },
  { 0x005d, 0x00ff, ' ', 1, "tstx" },
  { 0x005e, 0x00ff, ' ', 3, "mov %d,x+" },
  { 0x005f, 0x00ff, ' ', 1, "clrx" },

  { 0x0060, 0x00ff, ' ', 2, "neg %1,x" },
  { 0x0061, 0x00ff, 'R', 3, "cbeq %1,x+,%p" },
  { 0x0062, 0x00ff, ' ', 1, "nsa" },
  { 0x0063, 0x00ff, ' ', 2, "com %1,x" },
  { 0x0064, 0x00ff, ' ', 2, "lsr %1,x" },
  { 0x0065, 0x00ff, ' ', 3, "cphx %w" },
  { 0x0066, 0x00ff, ' ', 2, "ror %1,x" },
  { 0x0067, 0x00ff, ' ', 2, "asr %1,x" },
  { 0x0068, 0x00ff, ' ', 2, "lsl %1,x" },
  { 0x0069, 0x00ff, ' ', 2, "rol %1,x" },
  { 0x006a, 0x00ff, ' ', 2, "dec %1,x" },
  { 0x006b, 0x00ff, 'R', 2, "dbnz %1,x,%p" },
  { 0x006c, 0x00ff, ' ', 2, "inc %1,x" },
  { 0x006d, 0x00ff, ' ', 2, "tst %1,x" },
  { 0x006e, 0x00ff, ' ', 3, "mov %b,%d" },
  { 0x006f, 0x00ff, ' ', 2, "clr %1,x" },

/*
  { 0x0070, 0x00ff, ' ', 2, "neg %b,sp" },
  { 0x0071, 0x00ff, 'R', 3, "cbeq %b,sp,%d" },
  { 0x0073, 0x00ff, ' ', 2, "com %b,sp" },
  { 0x0074, 0x00ff, ' ', 2, "lsr %b,sp" },
  { 0x0076, 0x00ff, ' ', 2, "ror %b,sp" },
  { 0x0077, 0x00ff, ' ', 2, "asr %b,sp" },
  { 0x0078, 0x00ff, ' ', 2, "lsl %b,sp" },
  { 0x0079, 0x00ff, ' ', 2, "rol %b,sp" },
  { 0x007a, 0x00ff, ' ', 2, "dec %b,sp" },
  { 0x007b, 0x00ff, 'R', 2, "dbnz %b,sp,%d" },
  { 0x007c, 0x00ff, ' ', 2, "inc %b,sp" },
  { 0x007d, 0x00ff, ' ', 2, "tst %b,sp" },
  { 0x007f, 0x00ff, ' ', 2, "clr %b,sp" },
*/

  { 0x0070, 0x00ff, ' ', 1, "neg ,x" },
  { 0x0071, 0x00ff, 'R', 2, "cbeq ,x+,%p" },
  { 0x0072, 0x00ff, ' ', 1, "daa" },
  { 0x0073, 0x00ff, ' ', 1, "com ,x" },
  { 0x0074, 0x00ff, ' ', 1, "lsr ,x" },
  { 0x0075, 0x00ff, ' ', 2, "cphx %d" },
  { 0x0076, 0x00ff, ' ', 1, "ror ,x" },
  { 0x0077, 0x00ff, ' ', 1, "asr ,x" },
  { 0x0078, 0x00ff, ' ', 1, "lsl ,x" },
  { 0x0079, 0x00ff, ' ', 1, "rol ,x" },
  { 0x007a, 0x00ff, ' ', 1, "dec ,x" },
  { 0x007b, 0x00ff, 'R', 2, "dbnz ,x,%p" },
  { 0x007c, 0x00ff, ' ', 1, "inc ,x" },
  { 0x007d, 0x00ff, ' ', 1, "tst ,x" },
  { 0x007e, 0x00ff, ' ', 2, "mov ,x+,%d" },
  { 0x007f, 0x00ff, ' ', 1, "clr ,x" },

  { 0x0080, 0x00ff, ' ', 1, "rti" },
  { 0x0081, 0x00ff, ' ', 1, "rts" },
  { 0x0082, 0x00ff, ' ', 1, "bgnd" },  //HCS08 only
  { 0x0083, 0x00ff, ' ', 1, "swi" },
  { 0x0084, 0x00ff, ' ', 1, "tap" },
  { 0x0085, 0x00ff, ' ', 1, "tpa" },
  { 0x0086, 0x00ff, ' ', 1, "pula" },
  { 0x0087, 0x00ff, ' ', 1, "psha" },
  { 0x0088, 0x00ff, ' ', 1, "pulx" },
  { 0x0089, 0x00ff, ' ', 1, "pshx" },
  { 0x008a, 0x00ff, ' ', 1, "pulh" },
  { 0x008b, 0x00ff, ' ', 1, "pshh" },
  { 0x008c, 0x00ff, ' ', 1, "clrh" },
  { 0x008e, 0x00ff, ' ', 1, "stop" },
  { 0x008f, 0x00ff, ' ', 1, "wait" },

  { 0x0090, 0x00ff, 'R', 2, "bge %p" },
  { 0x0091, 0x00ff, 'R', 2, "blt %p" },
  { 0x0092, 0x00ff, 'R', 2, "bgt %p" },
  { 0x0093, 0x00ff, 'R', 2, "ble %p" },
  { 0x0094, 0x00ff, ' ', 1, "txs" },
  { 0x0095, 0x00ff, ' ', 1, "tsx" },
  { 0x0096, 0x00ff, ' ', 3, "sthx %x" },  //HCS08 only
  { 0x0097, 0x00ff, ' ', 1, "tax" },
  { 0x0098, 0x00ff, ' ', 1, "clc" },
  { 0x0099, 0x00ff, ' ', 1, "sec" },
  { 0x009a, 0x00ff, ' ', 1, "cli" },
  { 0x009b, 0x00ff, ' ', 1, "sei" },
  { 0x009c, 0x00ff, ' ', 1, "rsp" },
  { 0x009d, 0x00ff, ' ', 1, "nop" },
  { 0x009f, 0x00ff, ' ', 1, "txa" },

  { 0x00a0, 0x00ff, ' ', 2, "sub %b" },
  { 0x00a1, 0x00ff, ' ', 2, "cmp %b" },
  { 0x00a2, 0x00ff, ' ', 2, "sbc %b" },
  { 0x00a3, 0x00ff, ' ', 2, "cpx %b" },
  { 0x00a4, 0x00ff, ' ', 2, "and %b" },
  { 0x00a5, 0x00ff, ' ', 2, "bit %b" },
  { 0x00a6, 0x00ff, ' ', 2, "lda %b" },
  { 0x00a7, 0x00ff, ' ', 2, "ais %s" },
  { 0x00a8, 0x00ff, ' ', 2, "eor %b" },
  { 0x00a9, 0x00ff, ' ', 2, "adc %b" },
  { 0x00aa, 0x00ff, ' ', 2, "ora %b" },
  { 0x00ab, 0x00ff, ' ', 2, "add %b" },
  { 0x00ad, 0x00ff, 'R', 2, "bsr %d" },
  { 0x00ae, 0x00ff, ' ', 2, "ldx %b" },
  { 0x00af, 0x00ff, ' ', 2, "aix %s" },

  { 0x00b0, 0x00ff, ' ', 2, "sub %d" },
  { 0x00b1, 0x00ff, ' ', 2, "cmp %d" },
  { 0x00b2, 0x00ff, ' ', 2, "sbc %d" },
  { 0x00b3, 0x00ff, ' ', 2, "cpx %d" },
  { 0x00b4, 0x00ff, ' ', 2, "and %d" },
  { 0x00b5, 0x00ff, ' ', 2, "bit %d" },
  { 0x00b6, 0x00ff, ' ', 2, "lda %d" },
  { 0x00b7, 0x00ff, ' ', 2, "sta %d" },
  { 0x00b8, 0x00ff, ' ', 2, "eor %d" },
  { 0x00b9, 0x00ff, ' ', 2, "adc %d" },
  { 0x00ba, 0x00ff, ' ', 2, "ora %d" },
  { 0x00bb, 0x00ff, ' ', 2, "add %d" },
  { 0x00bc, 0x00ff, 'A', 2, "jmp %d" },
  { 0x00bd, 0x00ff, 'A', 2, "jsr %d" },
  { 0x00be, 0x00ff, ' ', 2, "ldx %d" },
  { 0x00bf, 0x00ff, ' ', 2, "stx %d" },

  { 0x00c0, 0x00ff, ' ', 3, "sub %x" },
  { 0x00c1, 0x00ff, ' ', 3, "cmp %x" },
  { 0x00c2, 0x00ff, ' ', 3, "sbc %x" },
  { 0x00c3, 0x00ff, ' ', 3, "cpx %x" },
  { 0x00c4, 0x00ff, ' ', 3, "and %x" },
  { 0x00c5, 0x00ff, ' ', 3, "bit %x" },
  { 0x00c6, 0x00ff, ' ', 3, "lda %x" },
  { 0x00c7, 0x00ff, ' ', 3, "sta %x" },
  { 0x00c8, 0x00ff, ' ', 3, "eor %x" },
  { 0x00c9, 0x00ff, ' ', 3, "adc %x" },
  { 0x00ca, 0x00ff, ' ', 3, "ora %x" },
  { 0x00cb, 0x00ff, ' ', 3, "add %x" },
  { 0x00cc, 0x00ff, 'A', 3, "jmp %x" },
  { 0x00cd, 0x00ff, 'A', 3, "jsr %x" },
  { 0x00ce, 0x00ff, ' ', 3, "ldx %x" },
  { 0x00cf, 0x00ff, ' ', 3, "stx %x" },

  { 0x00d0, 0x00ff, ' ', 3, "sub %2,x" },
  { 0x00d1, 0x00ff, ' ', 3, "cmp %2,x" },
  { 0x00d2, 0x00ff, ' ', 3, "sbc %2,x" },
  { 0x00d3, 0x00ff, ' ', 3, "cpx %2,x" },
  { 0x00d4, 0x00ff, ' ', 3, "and %2,x" },
  { 0x00d5, 0x00ff, ' ', 3, "bit %2,x" },
  { 0x00d6, 0x00ff, ' ', 3, "lda %2,x" },
  { 0x00d7, 0x00ff, ' ', 3, "sta %2,x" },
  { 0x00d8, 0x00ff, ' ', 3, "eor %2,x" },
  { 0x00d9, 0x00ff, ' ', 3, "adc %2,x" },
  { 0x00da, 0x00ff, ' ', 3, "ora %2,x" },
  { 0x00db, 0x00ff, ' ', 3, "add %2,x" },
  { 0x00dc, 0x00ff, ' ', 3, "jmp %2,x" },
  { 0x00dd, 0x00ff, ' ', 3, "jsr %2,x" },
  { 0x00de, 0x00ff, ' ', 3, "ldx %2,x" },
  { 0x00df, 0x00ff, ' ', 3, "stx %2,x" },

  { 0x00e0, 0x00ff, ' ', 2, "sub %1,x" },
  { 0x00e1, 0x00ff, ' ', 2, "cmp %1,x" },
  { 0x00e2, 0x00ff, ' ', 2, "sbc %1,x" },
  { 0x00e3, 0x00ff, ' ', 2, "cpx %1,x" },
  { 0x00e4, 0x00ff, ' ', 2, "and %1,x" },
  { 0x00e5, 0x00ff, ' ', 2, "bit %1,x" },
  { 0x00e6, 0x00ff, ' ', 2, "lda %1,x" },
  { 0x00e7, 0x00ff, ' ', 2, "sta %1,x" },
  { 0x00e8, 0x00ff, ' ', 2, "eor %1,x" },
  { 0x00e9, 0x00ff, ' ', 2, "adc %1,x" },
  { 0x00ea, 0x00ff, ' ', 2, "ora %1,x" },
  { 0x00eb, 0x00ff, ' ', 2, "add %1,x" },
  { 0x00ec, 0x00ff, ' ', 2, "jmp %1,x" },
  { 0x00ed, 0x00ff, ' ', 2, "jsr %1,x" },
  { 0x00ee, 0x00ff, ' ', 2, "ldx %1,x" },
  { 0x00ef, 0x00ff, ' ', 2, "stx %1,x" },

  { 0x00f0, 0x00ff, ' ', 1, "sub ,x" },
  { 0x00f1, 0x00ff, ' ', 1, "cmp ,x" },
  { 0x00f2, 0x00ff, ' ', 1, "sbc ,x" },
  { 0x00f3, 0x00ff, ' ', 1, "cpx ,x" },
  { 0x00f4, 0x00ff, ' ', 1, "and ,x" },
  { 0x00f5, 0x00ff, ' ', 1, "bit ,x" },
  { 0x00f6, 0x00ff, ' ', 1, "lda ,x" },
  { 0x00f7, 0x00ff, ' ', 1, "sta ,x" },
  { 0x00f8, 0x00ff, ' ', 1, "eor ,x" },
  { 0x00f9, 0x00ff, ' ', 1, "adc ,x" },
  { 0x00fa, 0x00ff, ' ', 1, "ora ,x" },
  { 0x00fb, 0x00ff, ' ', 1, "add ,x" },
  { 0x00fc, 0x00ff, ' ', 1, "jmp ,x" },
  { 0x00fd, 0x00ff, ' ', 1, "jsr ,x" },
  { 0x00fe, 0x00ff, ' ', 1, "ldx ,x" },
  { 0x00ff, 0x00ff, ' ', 1, "stx ,x" },

  { 0, 0, 0, 0, NULL }
};


struct dis_entry disass_hc08_9e[]= {
  { 0x0060, 0x00ff, ' ', 2, "neg %1,sp" },
  { 0x0061, 0x00ff, 'R', 3, "cbeq %1,sp,%p" },
  { 0x0063, 0x00ff, ' ', 2, "com %1,sp" },
  { 0x0064, 0x00ff, ' ', 2, "lsr %1,sp" },
  { 0x0066, 0x00ff, ' ', 2, "ror %1,sp" },
  { 0x0067, 0x00ff, ' ', 2, "asr %1,sp" },
  { 0x0068, 0x00ff, ' ', 2, "lsl %1,sp" },
  { 0x0069, 0x00ff, ' ', 2, "rol %1,sp" },
  { 0x006a, 0x00ff, ' ', 2, "dec %1,sp" },
  { 0x006b, 0x00ff, 'R', 2, "dbnz %1,sp,%p" },
  { 0x006c, 0x00ff, ' ', 2, "inc %1,sp" },
  { 0x006d, 0x00ff, ' ', 2, "tst %1,sp" },
  { 0x006f, 0x00ff, ' ', 2, "clr %1,sp" },

  { 0x00ae, 0x00ff, ' ', 1, "ldhx ,x" },  //HCS08 only

  { 0x00be, 0x00ff, ' ', 3, "ldhx %2,x" },  //HCS08 only

  { 0x00ce, 0x00ff, ' ', 2, "ldhx %1,x" },  //HCS08 only

  { 0x00d0, 0x00ff, ' ', 3, "sub %2,sp" },
  { 0x00d1, 0x00ff, ' ', 3, "cmp %2,sp" },
  { 0x00d2, 0x00ff, ' ', 3, "sbc %2,sp" },
  { 0x00d3, 0x00ff, ' ', 3, "cpx %2,sp" },
  { 0x00d4, 0x00ff, ' ', 3, "and %2,sp" },
  { 0x00d5, 0x00ff, ' ', 3, "bit %2,sp" },
  { 0x00d6, 0x00ff, ' ', 3, "lda %2,sp" },
  { 0x00d7, 0x00ff, ' ', 3, "sta %2,sp" },
  { 0x00d8, 0x00ff, ' ', 3, "eor %2,sp" },
  { 0x00d9, 0x00ff, ' ', 3, "adc %2,sp" },
  { 0x00da, 0x00ff, ' ', 3, "ora %2,sp" },
  { 0x00db, 0x00ff, ' ', 3, "add %2,sp" },
  { 0x00de, 0x00ff, ' ', 3, "ldx %2,sp" },
  { 0x00df, 0x00ff, ' ', 3, "stx %2,sp" },

  { 0x00e0, 0x00ff, ' ', 2, "sub %1,sp" },
  { 0x00e1, 0x00ff, ' ', 2, "cmp %1,sp" },
  { 0x00e2, 0x00ff, ' ', 2, "sbc %1,sp" },
  { 0x00e3, 0x00ff, ' ', 2, "cpx %1,sp" },
  { 0x00e4, 0x00ff, ' ', 2, "and %1,sp" },
  { 0x00e5, 0x00ff, ' ', 2, "bit %1,sp" },
  { 0x00e6, 0x00ff, ' ', 2, "lda %1,sp" },
  { 0x00e7, 0x00ff, ' ', 2, "sta %1,sp" },
  { 0x00e8, 0x00ff, ' ', 2, "eor %1,sp" },
  { 0x00e9, 0x00ff, ' ', 2, "adc %1,sp" },
  { 0x00ea, 0x00ff, ' ', 2, "ora %1,sp" },
  { 0x00eb, 0x00ff, ' ', 2, "add %1,sp" },
  { 0x00ee, 0x00ff, ' ', 2, "ldx %1,sp" },
  { 0x00ef, 0x00ff, ' ', 2, "stx %1,sp" },

  { 0x00f3, 0x00ff, ' ', 2, "cphx %1,sp" },  //HCS08 only
  { 0x00fe, 0x00ff, ' ', 2, "ldhx %1,sp" },  //HCS08 only
  { 0x00ff, 0x00ff, ' ', 2, "sthx %1,sp" },  //HCS08 only

  { 0, 0, 0, 0, NULL }
};


/* glob.cc */
