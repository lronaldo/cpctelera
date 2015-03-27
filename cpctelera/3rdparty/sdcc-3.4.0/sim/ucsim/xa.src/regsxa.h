/*
 * Simulator of microcontrollers (regsxa.h)
 *
 * Copyright (C) 1999,2002 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 * Other contributors include:
 *   Karl Bongers karl@turbobit.com,
 *   Johan Knol 
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

#define REGS_OFFSET 0x800

#ifndef REGSAVR_HEADER
#define REGSAVR_HEADER

#include "ddconfig.h"

struct t_regs
{
  int dummy;
};

/* these macros suck, what was I thinking?  Try to make it go fast
   at the our expense?  Daniels going to hate me if I continue to
   clutter up his nice C++ with old crusty C macros :)  Karl.
*/

/* store to sfr */
#define set_word_direct(addr, val) { sfr->set((t_addr) (addr), (val) & 0xff); \
                            sfr->set((t_addr) (addr+1), ((val) >> 8) & 0xff); }
#define set_byte_direct(addr, val) sfr->set((t_addr) (addr), (val) )

/* get from sfr */
#define get_byte_direct(addr) sfr->get((t_addr) (addr))
#define get_word_direct(addr) (sfr->get((t_addr) (addr)) | (sfr->get((t_addr) (addr+1)) << 8) )

/* store to idata(onchip) ram */
#define set_idata2(addr, val) { iram->set((t_addr) (addr), (val) & 0xff); \
                            iram->set((t_addr) (addr+1), ((val) >> 8) & 0xff); }
#define set_idata1(addr, val) iram->set((t_addr) (addr), (val) )

/* get from idata(onchip) ram */
#define get_idata1(addr) iram->get((t_addr) (addr))
#define get_idata2(addr) (iram->get((t_addr) (addr)) | (iram->get((t_addr) (addr+1)) << 8) )

/* store to xdata(external) ram */
#define set_xdata2(addr, val) { ram->set((t_addr) (addr), (val) & 0xff); \
                            ram->set((t_addr) (addr+1), ((val) >> 8) & 0xff); }
#define set_xdata1(addr, val) ram->set((t_addr) (addr), val)

/* get from xdata(external) ram */
#define get_xdata1(addr) ram->get((t_addr) (addr))
#define get_xdata2(addr) (ram->get((t_addr) (addr)) | (ram->get((t_addr) (addr+1)) << 8) )

/* get from code */
#define getcode1(addr) rom->get((t_addr) (addr))
#define getcode2(addr) (rom->get((t_addr) (addr)) | (rom->get((t_addr) (addr+1)) << 8) )

/* fetch from opcode code space */
#define fetch2() ((fetch() << 8) | fetch())
#define fetch1() fetch()

/* get a 1 or 2 byte register */
#define reg2(_index) get_reg(1, REGS_OFFSET + (_index<<1)) /* function in inst.cc */
#define reg1(_index) (unsigned char)get_reg(0, REGS_OFFSET + (_index))

#define set_reg1(_index, _value) { \
  set_byte_direct((REGS_OFFSET+(_index)), _value); \
}

#define set_reg2(_index, _value) { \
     set_word_direct( (REGS_OFFSET+(_index<<1)), _value); \
}

#define set_reg(_word_flag, _index, _value) { \
  if (_word_flag) \
    { set_reg2((_index), _value) } \
  else \
    { set_reg1((_index), _value) } \
}

/* R7 mirrors 1 of 2 real SP's
  note: we will probably need a real function here...
 */
#define set_sp(_value) { \
  { set_word_direct(REGS_OFFSET+(7*2), _value); } \
}

#define get_sp() ((TYPE_UWORD)(get_word_direct(REGS_OFFSET+(7*2))))

/* the program status word */
#define PSW 0x400
#define get_psw() ((TYPE_UWORD)(get_word_direct(PSW)))
#define set_psw(_flags) set_word_direct(PSW, _flags)

/* the system configuration register */
#define SCR 0x440
#define get_scr() get_byte_direct(SCR)
#define set_scr(scr) set_byte_direct(SCR, scr)

// PSW bits...(note: consider replacing with Bit defines used in s51.src code)
#define BIT_C  0x80
#define BIT_AC 0x40
#define BIT_V  0x04
#define BIT_N  0x02
#define BIT_Z  0x01
#define BIT_ALL (BIT_C | BIT_AC | BIT_V | BIT_N | BIT_Z)


#if 0
--------------------------------------------------------------------
Developer Notes.

This user guide has got the detailed information on the XA chip. 

http://www.semiconductors.philips.com/acrobat/various/XA_USER_GUIDE_1.pdf

f: {unused slot(word accessable only) for R8-R15}
e: R7h,R7l  Stack pointer, ptr to USP(PSW.SM=0), or SSP(PSW.SM=1)
c: R6h,R6l
a: R5h,R5l
8: R4h,R4l
below are the banked registers which mirror(B0..B3) depending on
PSW.(RS0,RS1)
6: R3h,R3l
4: R2h,R2l
2: R1h,R1l
0: R0h,R0l

Registers are all bit addressable as:
2: bx1f,bx1e...b8(R0h)  bx17,bx16..bx10(R0l)
0: bxf,bxe...b8(R0h)  b7,b6..b0(R0l)

Memory is little endian:
addr0: LSB
addr1: MSB

Data word access limited to word boundaries.  If non-word address used,
then will act as lesser word alignment used(addr b0=0).

Internal memory takes precedence over external memory, unless
explicit movx used.

64K segment memory layout, bank registers used include:
DS(data segment) and ES(extra segment) and forms high byte of
24 bit address.  Stack is in DS, so ES typically used to access
user data.

SFR(1K direct space) is above normal 1K direct address space(0-3FFH)
between 400H to 7FFH.

Branch targets must reside on even boundaries

MOVC instructions use either PC(SSEL.4=0) or CS(SSEL.4=1) register.

Core SFRs:
PCON, SCR, SSEL, PSWH, PSWL, CS, ES, DS
(1K SFR space)
400H-43FH are bit or byte accesable.
400H-5FFH is for built in SFR hardware.
600H-7FFH is for external SFR hardware access.
SFR access is independent of segment regs.
SFR inacessable from indirect addressing(must use direct-addr in opcodes).

Bit space:
0 to ffH - R0 to R15
100H to 1ffH - 20h to 3fH(direct ram, relative to DS)
200H to 3FFH - 400H to 43FH(on board SFRs)

PSW Flags: Carry(C), Aux Carry(AC), Overflow(V), Negative(N), Zero(Z).

Stack ptr is pre-decremented, followed by load(word operation),
default SPs are set to 100H.  So first PUSH would go to FEH-FFH.

DIRECT MEMORY SPACE

When we speak of direct memory space we refer to opcodes like
MOV Rd, direct
The "direct" part is always composed of 11 bits in the opcode.
So the total size of "direct" space is 2K bytes.

1.) This direct memory space contains the SFRs starting at 0x400 offset.

Internal onchip memory(SFRs and onchip RAM) always override
external memory.  Read the specific Chip documentation for the
location of SFRs and RAM.

The codes space is independent.

The registers: 4 banks of 8 bytes(R0-R3), R4-R7 8 bytes, and stack
pointers are self contained and not part of any address space.
(The CS,ES,DS appear to reside in SFR space).

This is still confusing, let take some examples.

---------------------------
XA-G49 chip has 2k bytes built in RAM.

According to the XA-G49 datasheet:

With the DS set to 0, then all indirect address references
between 0-7FFH reference the onchip 2K RAM.  Direct address
references below 0x400 access onchip 2K RAM.

With the DS not set to 0, then all indirect address references
between 0-7FFH reference external memory.  Direct address
references below 0x400 access external memory.

Any direct address references between 400H and 7FFH access the SFRs
regardless of the segment register contents.

To access any external memory which overlaps the 2K onchip memory
ues the MOVX instruction.

---------------------------
Proposed segment layout use for SDCC/XA compiler:

XDATA -> external memory(use indirect addressing, ignore direct
   addressing, ignore any overlap with onchip memory).

IDATA -> onchip memory(use indirect addressing, ignore direct
   addressing, assume small model where DS,ES always 0).

DATA -> SFR memory access using direct addressing.

CODE -> Far calls/returns are available.

(Johan, Im just trying to spell this out explicitly for
my own understanding.)

---------------------------
Proposed segment layout use for ucSim XA simulator.

ram -> external memory.

rom -> external/internal code.

sfr -> SFR register space.  Include registers/register banks here
in some unused location to provide a means to dump all the register
file contents using the "ds" command.  Could make sfr memory larger
than 0x800, and use the space above 0x800 to hold registers/sp-s.

idata -> onchip memory.

I think we can determine the size of idata memory at run time, so
this could allow for various sized onchip memorys.  So indirect
memory accesses like this:
set_indirect1(addr, value) {
  if (addr < mem_size(idata)) {
    set_idata(addr,value);
  } else {
    set_xdata(addr,value);
  }
}

----------------------------------------------
#endif


#endif
/* End of xa.src/regsxa.h */
