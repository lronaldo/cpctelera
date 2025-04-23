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

#include <stdio.h>

#include "stypes.h"
#include "glob.h"
#include "regspblaze.h"


/*
%a - 10b address (to ROM)
%A - 12b address (to ROM); picoblaze6
%k - immediate
%K - immediate (at different position); picoblaze6
%m - 6b address (to RAM)
%M - 8b address (to RAM); picoblaze6
%p - pc relative addressing
%P - pc relative addressing; picoblaze6
%r - register as first operand
%s - register as second operand
*/

/*  uint  code, mask;  char  branch;  uchar length;  char  *mnemonic; */
struct dis_entry disass_pblaze_x[]= {
  { 0, 0, ' ', 1, "BAD_OPCODE" }};


struct dis_entry_pblaze disass_pblaze3[]= {
  { 0x18000, 0x3f000, ' ', 1, "ADD %r,%k", ADD,     REG_CONSTANT },     // 0 1 1 0 0 0 x x x x k k k k k k k k
  { 0x19000, 0x3f00f, ' ', 1, "ADD %r,%s", ADD,     REG_REG },          // 0 1 1 0 0 1 x x x x y y y y 0 0 0 0
  { 0x1a000, 0x3f000, ' ', 1, "ADDCY %r,%k", ADDCY,    REG_CONSTANT },     // 0 1 1 0 1 0 x x x x k k k k k k k k
  { 0x1b000, 0x3f00f, ' ', 1, "ADDCY %r,%s", ADDCY,   REG_REG },          // 0 1 1 0 1 1 x x x x y y y y 0 0 0 0

  { 0x0a000, 0x3f000, ' ', 1, "AND %r,%k", AND,     REG_CONSTANT },     // 0 0 1 0 1 0 x x x x k k k k k k k k
  { 0x0b000, 0x3f00f, ' ', 1, "AND %r,%s", AND,     REG_REG },          // 0 0 1 0 1 1 x x x x y y y y 0 0 0 0

  { 0x30000, 0x3fc00, ' ', 1, "CALL %a", CALL,          ADDRESS10 },         // 1 1 0 0 0 0 0 0 a a a a a a a a a a
  { 0x31800, 0x3fc00, ' ', 1, "CALL C %a", CALL_C,        ADDRESS10 },        // 1 1 0 0 0 1 1 0 a a a a a a a a a a
  { 0x31c00, 0x3fc00, ' ', 1, "CALL NC %a", CALL_NC,       ADDRESS10 },        // 1 1 0 0 0 1 1 1 a a a a a a a a a a
  { 0x31400, 0x3fc00, ' ', 1, "CALL NZ %a", CALL_NZ,       ADDRESS10 },        // 1 1 0 0 0 1 0 1 a a a a a a a a a a
  { 0x31000, 0x3fc00, ' ', 1, "CALL Z %a", CALL_Z,        ADDRESS10 },        // 1 1 0 0 0 1 0 0 a a a a a a a a a a

  { 0x14000, 0x3f000, ' ', 1, "COMPARE %r,%k", COMPARE, REG_CONSTANT },        // 0 1 0 1 0 0 x x x x k k k k k k k k
  { 0x15000, 0x3f00f, ' ', 1, "COMPARE %r,%s", COMPARE, REG_REG },        // 0 1 0 1 0 1 x x x x y y y y 0 0 0 0

  { 0x3c000, 0x3ffff, ' ', 1, "DISABLE INTERRUPT", DISABLE_INTERRUPT, NO_OPERAND },        // 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0
  { 0x3c001, 0x3ffff, ' ', 1, "ENABLE INTERRUPT", ENABLE_INTERRUPT, NO_OPERAND },        // 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 1

  { 0x06000, 0x3f0c0, ' ', 1, "FETCH %r, %m", FETCH,        REG_ADDRESS6 },        // 0 0 0 1 1 0 x x x x 0 0 s s s s s s
  { 0x07000, 0x3f00f, ' ', 1, "FETCH %r,(%s)", FETCH,        REG_REG },        // 0 0 0 1 1 1 x x x x y y y y 0 0 0 0

  { 0x05000, 0x3f00f, ' ', 1, "INPUT %r,(%s)", IINPUT,        REG_REG },        // 0 0 0 1 0 1 x x x x y y y y 0 0 0 0
  { 0x04000, 0x3f000, ' ', 1, "INPUT %r,%p", IINPUT,        REG_PORT },        // 0 0 0 1 0 0 x x x x p p p p p p p p

  { 0x34000, 0x3fc00, ' ', 1, "JUMP %a", JUMP,        ADDRESS10 },            // 1 1 0 1 0 0 0 0 a a a a a a a a a a
  { 0x35800, 0x3fc00, ' ', 1, "JUMP C %a", JUMP_C,        ADDRESS10 },        // 1 1 0 1 0 1 1 0 a a a a a a a a a a
  { 0x35c00, 0x3fc00, ' ', 1, "JUMP NC %a", JUMP_NC,        ADDRESS10 },        // 1 1 0 1 0 1 1 1 a a a a a a a a a a
  { 0x35400, 0x3fc00, ' ', 1, "JUMP NZ %a", JUMP_NZ,        ADDRESS10 },        // 1 1 0 1 0 1 0 1 a a a a a a a a a a
  { 0x35000, 0x3fc00, ' ', 1, "JUMP Z %a", JUMP_Z,        ADDRESS10 },        // 1 1 0 1 0 1 0 0 a a a a a a a a a a

  { 0x00000, 0x3f000, ' ', 1, "LOAD %r,%k", LOAD,        REG_CONSTANT },        // 0 0 0 0 0 0 x x x x k k k k k k k k
  { 0x01000, 0x3f00f, ' ', 1, "LOAD %r,%s", LOAD,        REG_REG },        // 0 0 0 0 0 1 x x x x y y y y 0 0 0 0

  { 0x0c000, 0x3f000, ' ', 1, "OR %r,%k", OR,        REG_CONSTANT },        // 0 0 1 1 0 0 x x x x k k k k k k k k
  { 0x0d000, 0x3f00f, ' ', 1, "OR %r,%s", OR,        REG_REG },        // 0 0 1 1 0 1 x x x x y y y y 0 0 0 0

  { 0x2d000, 0x3f00f, ' ', 1, "OUTPUT %r,(%s)", OUTPUT,        REG_REG },        // 1 0 1 1 0 1 x x x x y y y y 0 0 0 0
  { 0x2c000, 0x3f000, ' ', 1, "OUTPUT %r,%p", OUTPUT,        REG_PORT },        // 1 0 1 1 0 0 x x x x p p p p p p p p

  { 0x2a000, 0x3ffff, ' ', 1, "RETURN", RETURN,        NO_OPERAND },        // 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0
  { 0x2b800, 0x3ffff, ' ', 1, "RETURN C", RETURN_C,        NO_OPERAND },        // 1 0 1 0 1 1 1 0 0 0 0 0 0 0 0 0 0 0
  { 0x2bc00, 0x3ffff, ' ', 1, "RETURN NC", RETURN_NC,        NO_OPERAND },        // 1 0 1 0 1 1 1 1 0 0 0 0 0 0 0 0 0 0
  { 0x2b400, 0x3ffff, ' ', 1, "RETURN NZ", RETURN_NZ,        NO_OPERAND },        // 1 0 1 0 1 1 0 1 0 0 0 0 0 0 0 0 0 0
  { 0x2b000, 0x3ffff, ' ', 1, "RETURN Z", RETURN_Z,        NO_OPERAND },        // 1 0 1 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0

  { 0x38000, 0x3ffff, ' ', 1, "RETURNI DISABLE", RETURNI_DISABLE,        NO_OPERAND },        // 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
  { 0x38001, 0x3ffff, ' ', 1, "RETURNI ENABLE", RETURNI_ENABLE,        NO_OPERAND },        // 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1

  { 0x20002, 0x3f0ff, ' ', 1, "RL %r", RL,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 0 0 1 0
  { 0x2000c, 0x3f0ff, ' ', 1, "RR %r", RR,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 1 1 0 0

  { 0x20006, 0x3f0ff, ' ', 1, "SL0 %r", SL0,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 0 1 1 0
  { 0x20007, 0x3f0ff, ' ', 1, "SL1 %r", SL1,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 0 1 1 1
  { 0x20000, 0x3f0ff, ' ', 1, "SLA %r", SLA,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 0 0 0 0
  { 0x20004, 0x3f0ff, ' ', 1, "SLX %r", SLX,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 0 1 0 0

  { 0x2000e, 0x3f0ff, ' ', 1, "SR0 %r", SR0,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 1 1 1 0
  { 0x2000f, 0x3f0ff, ' ', 1, "SR1 %r", SR1,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 1 1 1 1
  { 0x20008, 0x3f0ff, ' ', 1, "SRA %r", SRA,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 1 0 0 0
  { 0x2000a, 0x3f0ff, ' ', 1, "SRX %r", SRX,        REG },        // 1 0 0 0 0 0 x x x x 0 0 0 0 1 0 1 0

  { 0x2e000, 0x3f0c0, ' ', 1, "STORE %r,%m", STORE,        REG_ADDRESS6 },        // 1 0 1 1 1 0 x x x x 0 0 s s s s s s
  { 0x2f000, 0x3f00f, ' ', 1, "STORE %r,(%s)", STORE,        REG_REG },        // 1 0 1 1 1 1 x x x x y y y y 0 0 0 0
  { 0x1c000, 0x3f000, ' ', 1, "SUB %r,%k", SUB,        REG_CONSTANT },        // 0 1 1 1 0 0 x x x x k k k k k k k k
  { 0x1d000, 0x3f00f, ' ', 1, "SUB %r,%s", SUB,        REG_REG },        // 0 1 1 1 0 1 x x x x y y y y 0 0 0 0
  { 0x1e000, 0x3f000, ' ', 1, "SUBCY %r,%k", SUBCY,        REG_CONSTANT },        // 0 1 1 1 1 0 x x x x k k k k k k k k
  { 0x1f000, 0x3f00f, ' ', 1, "SUBCY %r,%s", SUBCY,        REG_REG },        // 0 1 1 1 1 1 x x x x y y y y 0 0 0 0
  { 0x12000, 0x3f000, ' ', 1, "TEST %r,%k", TEST,        REG_CONSTANT },        // 0 1 0 0 1 0 x x x x k k k k k k k k
  { 0x13000, 0x3f00f, ' ', 1, "TEST %r,%s", TEST,        REG_REG },        // 0 1 0 0 1 1 x x x x y y y y 0 0 0 0
  { 0x0e000, 0x3f000, ' ', 1, "XOR %r,%k", XOR,        REG_CONSTANT },        // 0 0 1 1 1 0 x x x x k k k k k k k k
  { 0x0f000, 0x3f00f, ' ', 1, "XOR %r,%s", XOR,        REG_REG },        // 0 0 1 1 1 1 x x x x y y y y 0 0 0 0

  { 0, 0, ' ', 1, "BAD_OPCODE", BAD_OPCODE, NO_OPERAND }
};

struct dis_entry_pblaze disass_pblaze6[]= {
  { 0x10000, 0x3f00f, ' ', 1, "ADD %r,%s", ADD,     REG_REG },
  { 0x11000, 0x3f000, ' ', 1, "ADD %r,%k", ADD,     REG_CONSTANT },

  { 0x12000, 0x3f00f, ' ', 1, "ADDCY %r,%s", ADDCY,   REG_REG },
  { 0x13000, 0x3f000, ' ', 1, "ADDCY %r,%k", ADDCY,    REG_CONSTANT },

  { 0x02000, 0x3f00f, ' ', 1, "AND %r,%s", AND,     REG_REG },
  { 0x03000, 0x3f000, ' ', 1, "AND %r,%k", AND,     REG_CONSTANT },

  { 0x20000, 0x3f000, ' ', 1, "CALL %A", CALL,          ADDRESS12 },
  { 0x30000, 0x3f000, ' ', 1, "CALL C %A", CALL_Z,        ADDRESS12 },
  { 0x34000, 0x3f000, ' ', 1, "CALL NZ %A", CALL_NZ,       ADDRESS12 },
  { 0x38000, 0x3f000, ' ', 1, "CALL Z %A", CALL_C,        ADDRESS12 },
  { 0x3c000, 0x3f000, ' ', 1, "CALL NC %A", CALL_NC,       ADDRESS12 },
  { 0x24000, 0x3f00f, ' ', 1, "CALL@ (%r, %s)", CALL_AT,       REG_REG },

  { 0x1c000, 0x3f00f, ' ', 1, "COMPARE %r,%s", COMPARE, REG_REG },
  { 0x1d000, 0x3f000, ' ', 1, "COMPARE %r,%k", COMPARE, REG_CONSTANT },

  { 0x1e000, 0x3f00f, ' ', 1, "COMPARECY %r,%s", COMPARECY, REG_REG },      //TEST
  { 0x1f000, 0x3f000, ' ', 1, "COMPARECY %r,%k", COMPARECY, REG_CONSTANT }, //TEST

  { 0x28000, 0x3ffff, ' ', 1, "DISABLE INTERRUPT", DISABLE_INTERRUPT, NO_OPERAND },
  { 0x28001, 0x3ffff, ' ', 1, "ENABLE INTERRUPT", ENABLE_INTERRUPT, NO_OPERAND },

  { 0x0a000, 0x3f00f, ' ', 1, "FETCH %r,(%s)", FETCH,        REG_REG },
  { 0x0b000, 0x3f000, ' ', 1, "FETCH %r, %M", FETCH,        REG_ADDRESS8 },

  { 0x14080, 0x3f0ff, ' ', 1, "HWBUILD %r", HWBUILD,        REG },

  { 0x08000, 0x3f00f, ' ', 1, "INPUT %r,(%s)", IINPUT,        REG_REG },
  { 0x09000, 0x3f000, ' ', 1, "INPUT %r,%p", IINPUT,        REG_PORT },

  { 0x22000, 0x3f000, ' ', 1, "JUMP %A", JUMP,        ADDRESS12 },
  { 0x32000, 0x3f000, ' ', 1, "JUMP Z %A", JUMP_Z,        ADDRESS12 },
  { 0x36000, 0x3f000, ' ', 1, "JUMP NZ %A", JUMP_NZ,        ADDRESS12 },
  { 0x3a000, 0x3f000, ' ', 1, "JUMP C %A", JUMP_C,        ADDRESS12 },
  { 0x3e000, 0x3f000, ' ', 1, "JUMP NC %A", JUMP_NC,        ADDRESS12 },
  { 0x26000, 0x3f00f, ' ', 1, "JUMP@ (%r, %s)", JUMP_AT,        REG_REG },

  { 0x00000, 0x3f00f, ' ', 1, "LOAD %r,%s", LOAD,        REG_REG },
  { 0x01000, 0x3f000, ' ', 1, "LOAD %r,%k", LOAD,        REG_CONSTANT },

  { 0x04000, 0x3f00f, ' ', 1, "OR %r,%s", OR,        REG_REG },
  { 0x05000, 0x3f000, ' ', 1, "OR %r,%k", OR,        REG_CONSTANT },

  { 0x2c000, 0x3f00f, ' ', 1, "OUTPUT %r,(%s)", OUTPUT,        REG_REG },
  { 0x2d000, 0x3f000, ' ', 1, "OUTPUT %r,%p", OUTPUT,        REG_PORT },
  { 0x2b000, 0x3f000, ' ', 1, "OUTPUTK %K,%P", OUTPUTK,        CONSTANT_PORT },

  { 0x37000, 0x3ffff, ' ', 1, "REGBANK A", REGBANK_A,        NO_OPERAND },
  { 0x37001, 0x3ffff, ' ', 1, "REGBANK B", REGBANK_B,        NO_OPERAND },

  { 0x25000, 0x3ffff, ' ', 1, "RETURN", RETURN,        NO_OPERAND },
  { 0x31000, 0x3ffff, ' ', 1, "RETURN Z", RETURN_Z,        NO_OPERAND },
  { 0x35000, 0x3ffff, ' ', 1, "RETURN NZ", RETURN_NZ,        NO_OPERAND },
  { 0x39000, 0x3ffff, ' ', 1, "RETURN C", RETURN_C,        NO_OPERAND },
  { 0x3d000, 0x3ffff, ' ', 1, "RETURN NC", RETURN_NC,        NO_OPERAND },
  { 0x21000, 0x3f000, ' ', 1, "LOAD&RETURN %r,%k", LOAD_RETURN,        REG_CONSTANT },

  { 0x29000, 0x3ffff, ' ', 1, "RETURNI DISABLE", RETURNI_DISABLE,        NO_OPERAND },
  { 0x29001, 0x3ffff, ' ', 1, "RETURNI ENABLE", RETURNI_ENABLE,        NO_OPERAND },

  { 0x14002, 0x3f0ff, ' ', 1, "RL %r", RL,        REG },
  { 0x1400c, 0x3f0ff, ' ', 1, "RR %r", RR,        REG },

  { 0x14006, 0x3f0ff, ' ', 1, "SL0 %r", SL0,        REG },
  { 0x14007, 0x3f0ff, ' ', 1, "SL1 %r", SL1,        REG },
  { 0x14004, 0x3f0ff, ' ', 1, "SLX %r", SLX,        REG },
  { 0x14000, 0x3f0ff, ' ', 1, "SLA %r", SLA,        REG },

  { 0x1400e, 0x3f0ff, ' ', 1, "SR0 %r", SR0,        REG },
  { 0x1400f, 0x3f0ff, ' ', 1, "SR1 %r", SR1,        REG },
  { 0x1400a, 0x3f0ff, ' ', 1, "SRX %r", SRX,        REG },
  { 0x14008, 0x3f0ff, ' ', 1, "SRA %r", SRA,        REG },

  { 0x16000, 0x3f00f, ' ', 1, "STAR %r,%s", STAR,        REG_REG },

  { 0x2e000, 0x3f00f, ' ', 1, "STORE %r,(%s)", STORE,        REG_REG },
  { 0x2f000, 0x3f000, ' ', 1, "STORE %r,%M", STORE,        REG_ADDRESS8 },

  { 0x18000, 0x3f00f, ' ', 1, "SUB %r,%s", SUB,        REG_REG },
  { 0x19000, 0x3f000, ' ', 1, "SUB %r,%k", SUB,        REG_CONSTANT },

  { 0x1a000, 0x3f00f, ' ', 1, "SUBCY %r,%s", SUBCY,        REG_REG },
  { 0x1b000, 0x3f000, ' ', 1, "SUBCY %r,%k", SUBCY,        REG_CONSTANT },

  { 0x0c000, 0x3f00f, ' ', 1, "TEST %r,%s", TEST,        REG_REG },
  { 0x0d000, 0x3f000, ' ', 1, "TEST %r,%k", TEST,        REG_CONSTANT },

  { 0x0e000, 0x3f00f, ' ', 1, "TESTCY %r,%s", TESTCY,        REG_REG },     //TEST
  { 0x0f000, 0x3f000, ' ', 1, "TESTCY %r,%k", TESTCY,        REG_CONSTANT },//TEST

  { 0x06000, 0x3f00f, ' ', 1, "XOR %r,%s", XOR,        REG_REG },
  { 0x07000, 0x3f000, ' ', 1, "XOR %r,%k", XOR,        REG_CONSTANT },

  { 0, 0, ' ', 1, "BAD_OPCODE", BAD_OPCODE, NO_OPERAND }
};



/*
 * Names of SFR cells
 */

struct name_entry sfr_tab[] =
{
  {CPU_ALL_PBLAZE, S0, "S0"},
  {CPU_ALL_PBLAZE, S1, "S1"},
  {CPU_ALL_PBLAZE, S2, "S2"},
  {CPU_ALL_PBLAZE, S3, "S3"},
  {CPU_ALL_PBLAZE, S4, "S4"},
  {CPU_ALL_PBLAZE, S5, "S5"},
  {CPU_ALL_PBLAZE, S6, "S6"},
  {CPU_ALL_PBLAZE, S7, "S7"},
  {CPU_ALL_PBLAZE, S8, "S8"},
  {CPU_ALL_PBLAZE, S9, "S9"},
  {CPU_ALL_PBLAZE, SA, "SA"},
  {CPU_ALL_PBLAZE, SB, "SB"},
  {CPU_ALL_PBLAZE, SC, "SC"},
  {CPU_ALL_PBLAZE, SD, "SD"},
  {CPU_ALL_PBLAZE, SE, "SE"},
  {CPU_ALL_PBLAZE, SF, "SF"},

  {CPU_PBLAZE_6, S0b, "S0b"},
  {CPU_PBLAZE_6, S1b, "S1b"},
  {CPU_PBLAZE_6, S2b, "S2b"},
  {CPU_PBLAZE_6, S3b, "S3b"},
  {CPU_PBLAZE_6, S4b, "S4b"},
  {CPU_PBLAZE_6, S5b, "S5b"},
  {CPU_PBLAZE_6, S6b, "S6b"},
  {CPU_PBLAZE_6, S7b, "S7b"},
  {CPU_PBLAZE_6, S8b, "S8b"},
  {CPU_PBLAZE_6, S9b, "S9b"},
  {CPU_PBLAZE_6, SAb, "SAb"},
  {CPU_PBLAZE_6, SBb, "SBb"},
  {CPU_PBLAZE_6, SCb, "SCb"},
  {CPU_PBLAZE_6, SDb, "SDb"},
  {CPU_PBLAZE_6, SEb, "SEb"},
  {CPU_PBLAZE_6, SFb, "SFb"},

  {CPU_ALL_PBLAZE, FLAGS, "FLAGS"},
  {CPU_ALL_PBLAZE, SP, "SP"},

  {0, 0, NULL}
};

// Second value indicates bitaddress.
// Higher byte of bitaddress is address of FLAGS register (0x20)
// Lower byte of bittaddress is position of bit in FLAGS register (see constants bmC, bmZ and bmI)
struct name_entry bit_tab[] =
{
  {CPU_ALL_PBLAZE, 0x20, "C"},
  {CPU_ALL_PBLAZE, 0x21, "Z"},
  {CPU_ALL_PBLAZE, 0x22, "I"},

  {0, 0, NULL}
};


struct cpu_entry cpus_pblaze[] =
{
  {"KCPSM3", CPU_PBLAZE_3, CPU_CMOS, "PicoBlaze 3", "fpga" },
  {"KCPSM6", CPU_PBLAZE_6, CPU_CMOS, "PicoBlaze 6", "fpga" },
  {"PB3"   , CPU_PBLAZE_3, CPU_CMOS, "PicoBlaze 3", "fpga" },
  {"PB6"   , CPU_PBLAZE_6, CPU_CMOS, "PicoBlaze 6", "fpga" },
  {"3"     , CPU_PBLAZE_3, CPU_CMOS, "PicoBlaze 3", "fpga" },
  {"6"     , CPU_PBLAZE_6, CPU_CMOS, "PicoBlaze 6", "fpga" },

  {NULL, CPU_NONE, 0, NULL, NULL}
};

/* End of pblaze.src/glob.cc */
