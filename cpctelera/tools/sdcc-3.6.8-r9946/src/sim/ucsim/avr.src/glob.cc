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


struct dis_entry disass_avr[]= {
  { 0x0000, 0xffff, ' ', 1, "nop" },
  { 0x9488, 0xffff, ' ', 1, "clc" },
  { 0x94d8, 0xffff, ' ', 1, "clh" },
  { 0x94f8, 0xffff, ' ', 1, "cli" },
  { 0x94a8, 0xffff, ' ', 1, "cln" },
  { 0x94c8, 0xffff, ' ', 1, "cls" },
  { 0x94e8, 0xffff, ' ', 1, "clt" },
  { 0x94b8, 0xffff, ' ', 1, "clv" },
  { 0x9498, 0xffff, ' ', 1, "clz" },
  { 0x9408, 0xffff, ' ', 1, "sec" },
  { 0x9458, 0xffff, ' ', 1, "seh" },
  { 0x9478, 0xffff, ' ', 1, "sei" },
  { 0x9428, 0xffff, ' ', 1, "sen" },
  { 0x9448, 0xffff, ' ', 1, "ses" },
  { 0x9468, 0xffff, ' ', 1, "set" },
  { 0x9438, 0xffff, ' ', 1, "sev" },
  { 0x9418, 0xffff, ' ', 1, "sez" },
  { 0x1c00, 0xfc00, ' ', 1, "adc %d,%r" },
  { 0x0c00, 0xfc00, ' ', 1, "add %d,%r" },
  { 0x9600, 0xff00, ' ', 1, "adiw %2,%6" },
  { 0x2000, 0xfc00, ' ', 1, "and %d,%r" },
  { 0x7000, 0xf000, ' ', 1, "andi %D,%K" },
  { 0x9405, 0xfe0f, ' ', 1, "asr %d" },
  { 0x9488, 0xff8f, ' ', 1, "bclr %s" },
  { 0xf800, 0xfe08, ' ', 1, "bld %d,%b" },
  { 0xf400, 0xfc07, ' ', 1, "brcc %k" },
  { 0xf000, 0xfc07, ' ', 1, "brcs %k" },
  { 0xf001, 0xfc07, ' ', 1, "breq %k" },
  { 0xf404, 0xfc07, ' ', 1, "brge %k" },
  { 0xf405, 0xfc07, ' ', 1, "brhc %k" },
  { 0xf005, 0xfc07, ' ', 1, "brhs %k" },
  { 0xf407, 0xfc07, ' ', 1, "brid %k" },
  { 0xf007, 0xfc07, ' ', 1, "brie %k" },
  { 0xf000, 0xfc07, ' ', 1, "brlo %k" },
  { 0xf004, 0xfc07, ' ', 1, "brlt %k" },
  { 0xf002, 0xfc07, ' ', 1, "brmi %k" },
  { 0xf401, 0xfc07, ' ', 1, "brne %k" },
  { 0xf402, 0xfc07, ' ', 1, "brpl %k" },
  { 0xf400, 0xfc07, ' ', 1, "brsh %k" },
  { 0xf406, 0xfc07, ' ', 1, "brtc %k" },
  { 0xf006, 0xfc07, ' ', 1, "brts %k" },
  { 0xf403, 0xfc07, ' ', 1, "brvc %k" },
  { 0xf003, 0xfc07, ' ', 1, "brvs %k" },
  { 0xf400, 0xfc00, ' ', 1, "brbc %b,%k" },
  { 0xf000, 0xfc00, ' ', 1, "brbs %b,%k" },
  { 0x9408, 0xff8f, ' ', 1, "bset %s" },
  { 0xfa00, 0xfe00, ' ', 1, "bst %d,%b" },
  { 0x940e, 0xfe0e, 'l', 2, "call %A", true },
  { 0x9800, 0xff00, ' ', 1, "cbi %P,%b" },
  { 0x9400, 0xfe0f, ' ', 1, "com %d" },
  { 0x1400, 0xfc00, ' ', 1, "cp %d,%r" },
  { 0x0400, 0xfc00, ' ', 1, "cpc %d,%r" },
  { 0x3000, 0xf000, ' ', 1, "cpi %D,%K" },
  { 0x1000, 0xfc00, ' ', 1, "cpse %d,%r" },
  { 0x940a, 0xfe0f, ' ', 1, "dec %d" },
  { 0x2400, 0xfc00, ' ', 1, "eor %d,%r" },
  { 0x9509, 0xff0f, ' ', 1, "icall" },
  { 0x9409, 0xff0f, ' ', 1, "ijmp" },
  { 0xb000, 0xf800, ' ', 1, "in %d,%p" },
  { 0x9403, 0xfe0f, ' ', 1, "inc %d" },
  { 0x940c, 0xfe0e, ' ', 2, "jmp %A" },
  { 0x900c, 0xfe0f, ' ', 1, "ld %d,X" },
  { 0x900d, 0xfe0f, ' ', 1, "ld %d,X+" },
  { 0x900e, 0xfe0f, ' ', 1, "ld %d,-X" },
  { 0x8008, 0xfe0f, ' ', 1, "ld %d,Y" },
  { 0x9009, 0xfe0f, ' ', 1, "ld %d,Y+" },
  { 0x900a, 0xfe0f, ' ', 1, "ld %d,-Y" },
  { 0x8008, 0xd208, ' ', 1, "ldd %d,Y+%q" },
  { 0x8000, 0xfe0f, ' ', 1, "ld %d,Z" },
  { 0x9001, 0xfe0f, ' ', 1, "ld %d,Z+" },
  { 0x9002, 0xfe0f, ' ', 1, "ld %d,-Z" },
  { 0x8000, 0xd208, ' ', 1, "ldd %d,Z+%q" },
  { 0xe000, 0xf000, ' ', 1, "ldi %D,%K" },
  { 0x9000, 0xfe0f, ' ', 2, "lds %d,%R" },
  { 0x95c8, 0xffff, ' ', 1, "lpm" },
  { 0x95d8, 0xffff, ' ', 1, "elpm" }, // in some devices equal to lpm
  { 0x9406, 0xfe0f, ' ', 1, "lsr %d" },
  { 0x2c00, 0xfc00, ' ', 1, "mov %d,%r" },
  { 0x9c00, 0xfc00, ' ', 1, "mul %d,%r" },
  { 0x9401, 0xfe0f, ' ', 1, "neg %d" },
  { 0x2800, 0xfc00, ' ', 1, "or %d,%r" },
  { 0x6000, 0xf000, ' ', 1, "ori %d,%K" },
  { 0xb800, 0xf800, ' ', 1, "out %p,%d" },
  { 0x900f, 0xfe0f, ' ', 1, "pop %d" },
  { 0x920f, 0xfe0f, ' ', 1, "push %d" },
  { 0xd000, 0xf000, ' ', 1, "rcall %a" },
  { 0x9508, 0xff9f, ' ', 1, "ret" },
  { 0x9518, 0xff9f, ' ', 1, "reti" },
  { 0xc000, 0xf000, ' ', 1, "rjmp %a" },
  { 0x9407, 0xfe0f, ' ', 1, "ror %d" },
  { 0x0800, 0xfc00, ' ', 1, "sbc %d,%r" },
  { 0x4000, 0xf000, ' ', 1, "sbci %D,%K" },
  { 0x9a00, 0xff00, ' ', 1, "sbi %P,%b" },
  { 0x9900, 0xff00, ' ', 1, "sbic %P,%b" },
  { 0x9b00, 0xff00, ' ', 1, "sbis %P,%b" },
  { 0x9700, 0xff00, ' ', 1, "sbiw %2,%6" },
  { 0x6000, 0xf000, ' ', 1, "sbr %D,%K" },
  { 0xfc00, 0xfe00, ' ', 1, "sbrc %d,%b" },
  { 0xfe00, 0xfe00, ' ', 1, "sbrs %d,%b" },
  { 0xef0f, 0xff0f, ' ', 1, "ser %D" },
  { 0x9588, 0xffef, ' ', 1, "sleep" },
  { 0x920c, 0xfe0f, ' ', 1, "st X,%d" },
  { 0x920d, 0xfe0f, ' ', 1, "st X+,%d" },
  { 0x920e, 0xfe0f, ' ', 1, "st -X,%d" },
  { 0x8208, 0xfe0f, ' ', 1, "st Y,%d" },
  { 0x9209, 0xfe0f, ' ', 1, "st Y+,%d" },
  { 0x920a, 0xfe0f, ' ', 1, "st -Y,%d" },
  { 0x8208, 0xd208, ' ', 1, "std Y+%q,%d" },
  { 0x8200, 0xfe0f, ' ', 1, "st Z,%d" },
  { 0x9201, 0xfe0f, ' ', 1, "st Z+,%d" },
  { 0x9202, 0xfe0f, ' ', 1, "st -Z,%d" },
  { 0x8200, 0xd208, ' ', 1, "std Z+%q,%d" },
  { 0x9200, 0xfe0f, ' ', 2, "sts %R,%d" },
  { 0x1800, 0xfc00, ' ', 1, "sub %d,%r" },
  { 0x5000, 0xf000, ' ', 1, "subi %D,%K" },
  { 0x9402, 0xfe0f, ' ', 1, "swap %d" },
  { 0x95a8, 0xffef, ' ', 1, "wdr" },
  { 0, 0, 0, 0, NULL }
};


/* End of avr.src/glob.cc */
