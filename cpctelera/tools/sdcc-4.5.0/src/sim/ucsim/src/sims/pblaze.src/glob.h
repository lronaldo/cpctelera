/*
 * Copyright (C) 2012-2013 Jiří Šimek
 * Copyright (C) 2013 Zbyněk Křivka <krivka@fit.vutbr.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef GLOB_HEADER
#define GLOB_HEADER

#include "stypes.h"

extern struct cpu_entry cpus_pblaze[];

extern struct dis_entry_pblaze disass_pblaze3[];
extern struct dis_entry_pblaze disass_pblaze6[];
extern struct dis_entry disass_pblaze_x[];

extern struct name_entry sfr_tab[];
extern struct name_entry bit_tab[];

enum instructions_pblaze {
    BAD_OPCODE = 0,
    ADD,
    ADDCY,
    AND,
    CALL,
    CALL_C,
    CALL_NC,
    CALL_NZ,
    CALL_Z,
    CALL_AT,
    COMPARE,
    COMPARECY,
    DISABLE_INTERRUPT,
    ENABLE_INTERRUPT,
    FETCH,
    HWBUILD,
    IINPUT,
    JUMP,
    JUMP_C,
    JUMP_NC,
    JUMP_NZ,
    JUMP_Z,
    JUMP_AT,
    LOAD,
    LOAD_RETURN,
    OR,
    OUTPUT,
    OUTPUTK,
    REGBANK_A,
    REGBANK_B,
    RETURN,
    RETURN_C,
    RETURN_NC,
    RETURN_NZ,
    RETURN_Z,
    RETURNI_DISABLE,
    RETURNI_ENABLE,
    RL,
    RR,
    SL0,
    SL1,
    SLA,
    SLX,
    SR0,
    SR1,
    SRA,
    SRX,
    STAR,
    STORE,
    SUB,
    SUBCY,
    TEST,
    TESTCY,
    XOR
};

enum op_operands_pblaze {
    NO_OPERAND = 0,
    REG,
    REG_REG,
    REG_CONSTANT,
    REG_PORT,
    REG_ADDRESS6,
    REG_ADDRESS8,
    CONSTANT_PORT,
    ADDRESS10,
    ADDRESS12
};

struct dis_entry_pblaze
{
  uint  code;               /* bits in opcode used to match table entry(with mask) */
  uint  mask;               /* mask used on .code to match up a common opcode */
  char  branch;
  uchar length;             /* total length of instruction */
  const char *mnemonic;     /* type of opcode(ADD, ADDC...) */
  uint instruction;          /* instruction (enum instructions_pblaze) */
  uint operands;             /* classification of operands (enum op_operands_pblaze) */
};

#endif

/* End of pblaze.src/glob.h */
