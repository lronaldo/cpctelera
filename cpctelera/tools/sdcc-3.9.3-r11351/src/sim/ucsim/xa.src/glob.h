/*
 * Simulator of microcontrollers (glob.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 * Other contributors include:
 *   Karl Bongers karl@turbobit.com,
 *   Johan Knol johan.knol@iduna.nl
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

#ifndef GLOB_HEADER
#define GLOB_HEADER

#include "stypes.h"

/* this needs to match char *op_mnemonic_str[] definition in glob.cc */
enum {
BAD_OPCODE=0,
ADD,
ADDC,
ADDS,
AND,
ANL,
ASL,
ASR,
BCC,
BCS,
BEQ,
BG,
BGE,
BGT,
BKPT,
BL,
BLE,
BLT,
BMI,
BNE,
BNV,
BOV,
BPL,
BR,
CALL,
CJNE,
CLR,
CMP,
CPL,
DA,
DIV_w,
DIV_d,
DIVU_b,
DIVU_w,
DIVU_d,
DJNZ,
FCALL,
FJMP,
JB,
JBC,
JMP,
JNB,
JNZ,
JZ,
LEA,
LSR,
MOV,
MOVC,
MOVS,
MOVX,
MUL_w,
MULU_b,
MULU_w,
NEG,
NOP,
NORM,
OR,
ORL,
POP,
POPU,
PUSH,
PUSHU,
RESET,
RET,
RETI,
RL,
RLC,
RR,
RRC,
SETB,
SEXT,
SUB,
SUBB,
TRAP,
XCH,
XOR,
};

extern const char *op_mnemonic_str[];

/* this classifies the operands and is used in the dissassembly
   to print the operands.  Its also used in the simulation to characterize
   the op-code function.
 */   
enum op_operands {
   // the repeating parameter encoding for ADD, ADDC, SUB, SUBB, AND, XOR, ...
  REG_REG         ,
  REG_IREG        ,
  IREG_REG        ,
  REG_IREGOFF8    ,
  IREGOFF8_REG    ,
  REG_IREGOFF16   ,
  IREGOFF16_REG   ,
  REG_IREGINC     ,
  IREGINC_REG     ,
  DIRECT_REG      ,
  REG_DIRECT      ,
  REG_DATA8       ,
  REG_DATA16      ,
  IREG_DATA8      ,
  IREG_DATA16     ,
  IREGINC_DATA8   ,
  IREGINC_DATA16  ,
  IREGOFF8_DATA8  ,
  IREGOFF8_DATA16 ,
  IREGOFF16_DATA8 ,
  IREGOFF16_DATA16,
  DIRECT_DATA8    ,
  DIRECT_DATA16   ,

// odd-ball ones
  NO_OPERANDS,  // for NOP
  CY_BIT,
  BIT_CY,
  CY_NOTBIT,
  DATA4,
  REG_DATA4,
  REG_DATA5,
  IREG_DATA4,
  IREGINC_DATA4,
  IREGOFF8_DATA4,
  IREGOFF16_DATA4,
  DIRECT_DATA4,

  REG,
  IREG,
  BIT_ALONE,
  DIRECT,
  DIRECT_DIRECT,
  RLIST,
  ADDR24,
  BIT_REL8,
  REG_REL8,
  DIRECT_REL8,
  REG_REGOFF8,
  REG_REGOFF16,

  REG_USP,
  USP_REG,

  REL8,
  REL16,

  REG_DIRECT_REL8,
  REG_DATA8_REL8,
  REG_DATA16_REL8,
  IREG_DATA8_REL8,
  IREG_DATA16_REL8,

  A_APLUSDPTR,
  A_APLUSPC,
  A_PLUSDPTR,
  IIREG
};

// table of dissassembled instructions
struct xa_dis_entry
{
  uint is1byte; /* only grab 1 byte for table lookup(most are 2 bytes) */
  uint code;    /* bits in opcode used to match table entry(with mask) */
  uint mask;    /* mask used on .code to match up a common opcode */
  char branch;  /* used by main app to implement "next" around calls */
  uchar length; /* total length of opcode, used by dissasembler and main app */
  int mnemonic; /* type of opcode(ADD, ADDC...) */
  int operands; /* unique classification of operands: Rd,Rs = REG_REG,... */
};

extern struct dis_entry glob_disass_xa[];

extern struct xa_dis_entry disass_xa[];


#endif

/* End of xa.src/glob.h */
