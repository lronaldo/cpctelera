/*-------------------------------------------------------------------------
  gen.c - code generator for Z80 / Z180 / GBZ80.

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Copyright (C) 2000, Michael Hope <michaelh@juju.net.nz>
  Copyright (C) 2011-2018, Philipp Klaus Krause pkk@spth.de, philipp@informatik.uni-frankfurt.de)

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2, or (at your option) any
  later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "z80.h"
#include "gen.h"
#include "dbuf_string.h"

/* This is the down and dirty file with all kinds of kludgy & hacky
   stuff. This is what it is all about CODE GENERATION for a specific MCU.
   Some of the routines may be reusable, will have to see */

/* Z80 calling convention description.
   Parameters are passed right to left.  As the stack grows downwards,
   the parameters are arranged in left to right in memory.

   Everything is caller saves. i.e. the caller must save any registers
   that it wants to preserve over the call, except for ix, which is callee saves.
   GB: The return value is returned in DEHL.  DE is normally used as a
   working register pair.  Caller saves allows it to be used for a
   return value.
   va args functions do not use register parameters.  All arguments
   are passed on the stack.
   IX is used as an index register to the top of the local variable
   area.  ix-0 is the top most local variable.
*/

enum
{
  /* Set to enable debugging trace statements in the output assembly code. */
  DISABLE_DEBUG = 0
};

// #define DEBUG_DRY_COST

extern struct dbuf_s *codeOutBuf;

enum
{
  INT8MIN = -128,
  INT8MAX = 127
};

/** Enum covering all the possible register pairs.
 */
typedef enum
{
  PAIR_INVALID,
  PAIR_AF,
  PAIR_BC,
  PAIR_DE,
  PAIR_HL,
  PAIR_IY,
  PAIR_IX,
  NUM_PAIRS
} PAIR_ID;

static struct
{
  const char *name;
  const char *l;
  const char *h;
  int l_idx;
  int h_idx;
} _pairs[NUM_PAIRS] =
{
  {
    "??1", "?2", "?3", -1, -1
  },
  {
    "af", "f", "a", -1, A_IDX
  },
  {
    "bc", "c", "b", C_IDX, B_IDX
  },
  {
    "de", "e", "d", E_IDX, D_IDX
  },
  {
    "hl", "l", "h", L_IDX, H_IDX
  },
  {
    "iy", "iyl", "iyh", IYL_IDX, IYH_IDX
  },
  {
    "ix", "ixl", "ixh", -1, -1
  }
};

enum
{
  LSB,
  MSB16,
  MSB24,
  MSB32
};

enum asminst
{
  A_ADD,
  A_ADC,
  A_AND,
  A_CP,
  A_CPL,
  A_DEC,
  A_INC,
  A_LD,
  A_NEG,
  A_OR,
  A_RL,
  A_RLA,
  A_RLC,
  A_RLCA,
  A_RR,
  A_RRA,
  A_RRC,
  A_RRCA,
  A_SBC,
  A_SLA,
  A_SRA,
  A_SRL,
  A_SUB,
  A_XOR,
  A_SWAP
};

static const char *asminstnames[] =
{
  "add",
  "adc",
  "and",
  "cp",
  "cpl",
  "dec",
  "inc",
  "ld",
  "neg",
  "or",
  "rl",
  "rla",
  "rlc",
  "rlca",
  "rr",
  "rra",
  "rrc",
  "rrca",
  "sbc",
  "sla",
  "sra",
  "srl",
  "sub",
  "xor",
  "swap"
};

/** Code generator persistent data.
 */
static struct
{
  /** Used to optimise setting up of a pair by remembering what it
      contains and adjusting instead of reloading where possible.
  */
  struct
  {
    AOP_TYPE last_type;
    const char *base;
    int offset;
  } pairs[NUM_PAIRS];
  struct
  {
//    int last;
    int pushed;
    int param_offset;
    int offset;
    int pushedHL;
    int pushedBC;
    int pushedDE;
    int pushedIY;
  } stack;

  struct
  {
    int pushedBC;
    int pushedDE;
  } calleeSaves;

  bool omitFramePtr;
  int frameId;
  int receiveOffset;
  bool flushStatics;
  bool in_home;
  const char *lastFunctionName;
  iCode *current_iCode;
  bool preserveCarry;

  set *sendSet;

  struct
  {
    /** TRUE if the registers have already been saved. */
    bool saved;
  } saves;

  struct
  {
    allocTrace trace;
  } lines;

  struct
  {
    allocTrace aops;
  } trace;
} _G;

bool z80_regs_used_as_parms_in_calls_from_current_function[IYH_IDX + 1];
bool z80_symmParm_in_calls_from_current_function;
bool z80_regs_preserved_in_calls_from_current_function[IYH_IDX + 1];

static const char *aopGet (asmop *aop, int offset, bool bit16);

static struct asmop asmop_a, asmop_b, asmop_c, asmop_d, asmop_e, asmop_h, asmop_l, asmop_iyh, asmop_iyl, asmop_zero, asmop_one, asmop_return;
static struct asmop *const ASMOP_A = &asmop_a;
static struct asmop *const ASMOP_B = &asmop_b;
static struct asmop *const ASMOP_C = &asmop_c;
static struct asmop *const ASMOP_D = &asmop_d;
static struct asmop *const ASMOP_E = &asmop_e;
static struct asmop *const ASMOP_H = &asmop_h;
static struct asmop *const ASMOP_L = &asmop_l;
static struct asmop *const ASMOP_IYH = &asmop_iyh;
static struct asmop *const ASMOP_IYL = &asmop_iyl;
static struct asmop *const ASMOP_ZERO = &asmop_zero;
static struct asmop *const ASMOP_ONE = &asmop_one;
static struct asmop *const ASMOP_RETURN = &asmop_return;

static asmop *asmopregs[] = { &asmop_a, &asmop_c, &asmop_b, &asmop_e, &asmop_d, &asmop_l, &asmop_h, &asmop_iyl, &asmop_iyh };

void
z80_init_asmops (void)
{
  asmop_a.type = AOP_REG;
  asmop_a.size = 1;
  asmop_a.aopu.aop_reg[0] = regsZ80 + A_IDX;
  memset (asmop_a.regs, -1, 9);
  asmop_a.regs[A_IDX] = 0;
  asmop_b.type = AOP_REG;
  asmop_b.size = 1;
  asmop_b.aopu.aop_reg[0] = regsZ80 + B_IDX;
  memset (asmop_b.regs, -1, 9);
  asmop_b.regs[B_IDX] = 0;
  asmop_c.type = AOP_REG;
  asmop_c.size = 1;
  asmop_c.aopu.aop_reg[0] = regsZ80 + C_IDX;
  memset (asmop_c.regs, -1, 9);
  asmop_c.regs[C_IDX] = 0;
  asmop_d.type = AOP_REG;
  asmop_d.size = 1;
  asmop_d.aopu.aop_reg[0] = regsZ80 + D_IDX;
  memset (asmop_d.regs, -1, 9);
  asmop_d.regs[D_IDX] = 0;
  asmop_e.type = AOP_REG;
  asmop_e.size = 1;
  asmop_e.aopu.aop_reg[0] = regsZ80 + E_IDX;
  memset (asmop_e.regs, -1, 9);
  asmop_e.regs[E_IDX] = 0;
  asmop_h.type = AOP_REG;
  asmop_h.size = 1;
  asmop_h.aopu.aop_reg[0] = regsZ80 + H_IDX;
  memset (asmop_h.regs, -1, 9);
  asmop_h.regs[H_IDX] = 0;
  asmop_l.type = AOP_REG;
  asmop_l.size = 1;
  asmop_l.aopu.aop_reg[0] = regsZ80 + L_IDX;
  memset (asmop_l.regs, -1, 9);
  asmop_l.regs[L_IDX] = 0;
  asmop_iyh.type = AOP_REG;
  asmop_iyh.size = 1;
  asmop_iyh.aopu.aop_reg[0] = regsZ80 + IYH_IDX;
  memset (asmop_iyh.regs, -1, 9);
  asmop_iyh.regs[IYH_IDX] = 0;
  asmop_iyl.type = AOP_REG;
  asmop_iyl.size = 1;
  asmop_iyl.aopu.aop_reg[0] = regsZ80 + IYL_IDX;
  memset (asmop_iyl.regs, -1, 9);
  asmop_iyl.regs[IYL_IDX] = 0;

  /*asmop_hl.type = AOP_REG;
  asmop_hl.size = 2;
  asmop_hl.aopu.aop_reg[0] = regsZ80 + L_IDX;
  asmop_hl.aopu.aop_reg[1] = regsZ80 + H_IDX;
  memset (asmop_hl.regs, -1, 9);
  asmop_hl.regs[L_IDX] = 0;
  asmop_hl.regs[H_IDX] = 1;*/

  asmop_zero.type = AOP_LIT;
  asmop_zero.aopu.aop_lit = constVal ("0");
  asmop_zero.size = 1;
  memset (asmop_zero.regs, -1, 9);

  asmop_one.type = AOP_LIT;
  asmop_one.aopu.aop_lit = constVal ("1");
  asmop_one.size = 1;
  memset (asmop_one.regs, -1, 9);

  asmop_return.type = AOP_REG;
  asmop_return.size = 4;
  memset (asmop_return.regs, -1, 9);
  if (IS_GB)
    {
      asmop_return.aopu.aop_reg[0] = regsZ80 + E_IDX;
      asmop_return.regs[E_IDX] = 0;
      asmop_return.aopu.aop_reg[1] = regsZ80 + D_IDX;
      asmop_return.regs[D_IDX] = 1;
      asmop_return.aopu.aop_reg[2] = regsZ80 + L_IDX;
      asmop_return.regs[L_IDX] = 2;
      asmop_return.aopu.aop_reg[3] = regsZ80 + H_IDX;
      asmop_return.regs[H_IDX] = 2;
    }
  else
    {
      asmop_return.aopu.aop_reg[0] = regsZ80 + L_IDX;
      asmop_return.regs[L_IDX] = 0;
      asmop_return.aopu.aop_reg[1] = regsZ80 + H_IDX;
      asmop_return.regs[H_IDX] = 1;
      asmop_return.aopu.aop_reg[2] = regsZ80 + E_IDX;
      asmop_return.regs[E_IDX] = 2;
      asmop_return.aopu.aop_reg[3] = regsZ80 + D_IDX;
      asmop_return.regs[D_IDX] = 3;
    }
}

static bool regalloc_dry_run;
static unsigned int regalloc_dry_run_cost;

static void
cost(unsigned int bytes, unsigned int cycles)
{
  regalloc_dry_run_cost += bytes;
}

static void
cost2(unsigned int bytes, unsigned int cycles_z80, unsigned int cycles_z180, unsigned int cycles_r2k, unsigned int cycles_gbz80, unsigned int cycles_tlcs90, unsigned int cycles_ez80_z80)
{
  regalloc_dry_run_cost += bytes;
}

/*-----------------------------------------------------------------*/
/* aopRS - asmop in register or on stack                           */
/*-----------------------------------------------------------------*/
static bool
aopRS (const asmop *aop)
{
  return (aop->type == AOP_REG || aop->type == AOP_STK || aop->type == AOP_EXSTK);
}

/*-----------------------------------------------------------------*/
/* aopIsLitVal - asmop from offset is val                          */
/*-----------------------------------------------------------------*/
static bool
aopIsLitVal (const asmop *aop, int offset, int size, unsigned long long int val)
{
  wassert (size <= sizeof (unsigned long long int)); // Make sure we are not testing outside of argument val.

  for(; size; size--, offset++)
    {
      unsigned char b = val & 0xff;
      val >>= 8;

      // Leading zeroes
      if (aop->size <= offset && !b)
        continue;

      if (aop->type != AOP_LIT)
        return (FALSE);

      if (byteOfVal (aop->aopu.aop_lit, offset) != b)
        return (FALSE);
    }

  return (TRUE);
}

/*-----------------------------------------------------------------*/
/* aopInReg - asmop from offset in the register                    */
/*-----------------------------------------------------------------*/
static inline bool
aopInReg (const asmop *aop, int offset, short rIdx)
{
  if (!(aop->type == AOP_REG))
    return (false);

  if (offset >= aop->size || offset < 0)
    return (false);

  if (rIdx == IY_IDX)
    return (aopInReg (aop, offset, IYL_IDX) && aopInReg (aop, offset + 1, IYH_IDX));
  if (rIdx == BC_IDX)
    return (aopInReg (aop, offset, C_IDX) && aopInReg (aop, offset + 1, B_IDX));
  if (rIdx == DE_IDX)
    return (aopInReg (aop, offset, E_IDX) && aopInReg (aop, offset + 1, D_IDX));
  if (rIdx == HL_IDX)
    return (aopInReg (aop, offset, L_IDX) && aopInReg (aop, offset + 1, H_IDX));

  return (aop->aopu.aop_reg[offset]->rIdx == rIdx);
}

/*-----------------------------------------------------------------*/
/* aopOnStack - asmop from offset on stack in consecutive memory   */
/*-----------------------------------------------------------------*/
static bool
aopOnStack (const asmop *aop, int offset, int size)
{
  if (!(aop->type == AOP_STK || aop->type == AOP_EXSTK))
    return (false);

  if (offset + size > aop->size)
    return (false);

  return (true);
}

/* WARNING: This function is dangerous to use. It works literally:
   It will return true if ic the the last use of op, even if ic might
   be executed again, e.g. due to a loop. Most of the time you will want
   to use isPairDead(), or ic->rSurv instead of this function. */
static bool
isLastUse (const iCode * ic, operand * op)
{
  bitVect *uses = bitVectCopy (OP_USES (op));

  while (!bitVectIsZero (uses))
    {
      if (bitVectFirstBit (uses) == ic->key)
        {
          if (bitVectnBitsOn (uses) == 1)
            {
              return TRUE;
            }
          else
            {
              return FALSE;
            }
        }
      bitVectUnSetBit (uses, bitVectFirstBit (uses));
    }

  return FALSE;
}

static PAIR_ID
_getTempPairId (void)
{
  if (IS_GB)
    {
      return PAIR_DE;
    }
  else
    {
      return PAIR_HL;
    }
}

static const char *
_getTempPairName (void)
{
  return _pairs[_getTempPairId ()].name;
}

static bool
isPairInUse (PAIR_ID id, const iCode * ic)
{
  if (id == PAIR_DE)
    {
      return bitVectBitValue (ic->rMask, D_IDX) || bitVectBitValue (ic->rMask, E_IDX);
    }
  else if (id == PAIR_BC)
    {
      return bitVectBitValue (ic->rMask, B_IDX) || bitVectBitValue (ic->rMask, C_IDX);
    }
  else
    {
      wassertl (0, "Only implemented for DE and BC");
      return TRUE;
    }
}

static bool
isPairDead (PAIR_ID id, const iCode * ic)
{
  const bitVect *r = (!options.oldralloc ? ic->rSurv :
                      (POINTER_SET (ic) ? ic->rMask :
                       (bitVectCplAnd (bitVectCopy (ic->rMask), z80_rUmaskForOp (IC_RESULT (ic))))));

  if (id == PAIR_DE)
    return !(bitVectBitValue (r, D_IDX) || bitVectBitValue (r, E_IDX));
  else if (id == PAIR_BC)
    return !(bitVectBitValue (r, B_IDX) || bitVectBitValue (r, C_IDX));
  else if (id == PAIR_HL)
    return !(bitVectBitValue (r, H_IDX) || bitVectBitValue (r, L_IDX));
  else if (id == PAIR_IY)
    return !(bitVectBitValue (r, IYH_IDX) || bitVectBitValue (r, IYL_IDX));
  else
    {
      wassertl (0, "Only implemented for DE, BC, HL and IY");
      return TRUE;
    }
}

static PAIR_ID
getDeadPairId (const iCode *ic)
{
  if (isPairDead (PAIR_BC, ic))
    {
      return PAIR_BC;
    }
  else if (!IS_GB && isPairDead (PAIR_DE, ic))
    {
      return PAIR_DE;
    }
  else
    {
      return PAIR_INVALID;
    }
}

static PAIR_ID
getFreePairId (const iCode *ic)
{
  if (!isPairInUse (PAIR_BC, ic))
    {
      return PAIR_BC;
    }
  else if (!IS_GB && !isPairInUse (PAIR_DE, ic))
    {
      return PAIR_DE;
    }
  else
    {
      return PAIR_INVALID;
    }
}

static void
_tidyUp (char *buf)
{
  /* Clean up the line so that it is 'prettier' */
  /* If it is a label - can't do anything */
  if (!strchr (buf, ':'))
    {
      /* Change the first (and probably only) ' ' to a tab so
         everything lines up.
       */
      while (*buf)
        {
          if (*buf == ' ')
            {
              *buf = '\t';
              break;
            }
          buf++;
        }
    }
}

static void
_vemit2 (const char *szFormat, va_list ap)
{
  struct dbuf_s dbuf;
  char *buffer, *p, *nextp;

  dbuf_init (&dbuf, INITIAL_INLINEASM);

  dbuf_tvprintf (&dbuf, szFormat, ap);

  buffer = p = dbuf_detach_c_str (&dbuf);

  _tidyUp (p);

  /* Decompose multiline macros */
  while ((nextp = strchr (p, '\n')))
    {
      *nextp = '\0';
      emit_raw (p);
      p = nextp + 1;
    }

  emit_raw (p);

  dbuf_free (buffer);
}

static void
emitDebug (const char *szFormat, ...)
{
  if (!DISABLE_DEBUG && !regalloc_dry_run && options.verboseAsm)
    {
      va_list ap;

      va_start (ap, szFormat);
      _vemit2 (szFormat, ap);
      va_end (ap);
    }
}

static void
emit2 (const char *szFormat, ...)
{
  if (!regalloc_dry_run)
    {
      va_list ap;

      va_start (ap, szFormat);
      _vemit2 (szFormat, ap);
      va_end (ap);
    }
}

static PAIR_ID
getPartPairId (const asmop *aop, int offset)
{
  if (aop->size <= offset + 1 || offset < 0)
    return PAIR_INVALID;

  if (aop->type != AOP_REG)
    return PAIR_INVALID;

  wassert (aop->aopu.aop_reg[offset] && aop->aopu.aop_reg[offset + 1]);

  if ((aop->aopu.aop_reg[offset]->rIdx == C_IDX) && (aop->aopu.aop_reg[offset + 1]->rIdx == B_IDX))
    return PAIR_BC;
  if ((aop->aopu.aop_reg[offset]->rIdx == E_IDX) && (aop->aopu.aop_reg[offset + 1]->rIdx == D_IDX))
    return PAIR_DE;
  if ((aop->aopu.aop_reg[offset]->rIdx == L_IDX) && (aop->aopu.aop_reg[offset + 1]->rIdx == H_IDX))
    return PAIR_HL;
  if ((aop->aopu.aop_reg[offset]->rIdx == IYL_IDX) && (aop->aopu.aop_reg[offset + 1]->rIdx == IYH_IDX))
    return PAIR_IY;

  return PAIR_INVALID;
}

static PAIR_ID
getPairId_o (const asmop *aop, int offset)
{
  if (offset + 2 <= aop->size)
    {
      if (aop->type == AOP_REG)
        {
          wassert (aop->aopu.aop_reg[offset] && aop->aopu.aop_reg[offset + 1]);

          if ((aop->aopu.aop_reg[offset]->rIdx == C_IDX) && (aop->aopu.aop_reg[offset + 1]->rIdx == B_IDX))
            {
              return PAIR_BC;
            }
          if ((aop->aopu.aop_reg[offset]->rIdx == E_IDX) && (aop->aopu.aop_reg[offset + 1]->rIdx == D_IDX))
            {
              return PAIR_DE;
            }
          if ((aop->aopu.aop_reg[offset]->rIdx == L_IDX) && (aop->aopu.aop_reg[offset + 1]->rIdx == H_IDX))
            {
              return PAIR_HL;
            }
          if ((aop->aopu.aop_reg[offset]->rIdx == IYL_IDX) && (aop->aopu.aop_reg[offset + 1]->rIdx == IYH_IDX))
            {
              return PAIR_IY;
            }
        }
      else if (aop->type == AOP_STR)
        {
          int i;
          for (i = 0; i < NUM_PAIRS; i++)
            {
              if (!strcmp (aop->aopu.aop_str[offset], _pairs[i].l) && !strcmp (aop->aopu.aop_str[offset + 1], _pairs[i].h))
                return i;
            }
        }
    }
  return PAIR_INVALID;
}

static PAIR_ID
getPairId (const asmop *aop)
{
  if (aop->size != 2)
    return PAIR_INVALID;
  return (getPairId_o (aop, 0));
}


/*-----------------------------------------------------------------*/
/* z80_emitDebuggerSymbol - associate the current code location    */
/*   with a debugger symbol                                        */
/*-----------------------------------------------------------------*/
void
z80_emitDebuggerSymbol (const char *debugSym)
{
  genLine.lineElement.isDebug = 1;
  emit2 ("%s !equ .", debugSym);
  emit2 ("!global", debugSym);
  genLine.lineElement.isDebug = 0;
}

// Todo: Handle IY correctly.
static unsigned char
ld_cost (const asmop *op1, const asmop *op2)
{
  AOP_TYPE op1type = op1->type;
  AOP_TYPE op2type = op2->type;

  /* Costs are symmetric */
  if (op2type == AOP_REG || op2type == AOP_DUMMY)
    {
      const asmop *tmp = op1;
      op1 = op2;
      op2 = tmp;
      op1type = op1->type;
      op2type = op2->type;
    }

  switch (op1type)
    {
    case AOP_REG:
    case AOP_DUMMY:
      switch (op2type)
        {
        case AOP_REG:
        case AOP_DUMMY:
          return (1);
        case AOP_IMMD:
        case AOP_LIT:
          return (2);
        case AOP_SFR:          /* 2 from in a, (...) */
          return ((aopInReg (op1, 0, A_IDX) || op1type == AOP_DUMMY) ? 2 : 3);
        case AOP_STK:
          return (3);
        case AOP_HL:           /* 3 from ld hl, #... */
          return (4);
        case AOP_IY:           /* 4 from ld iy, #... */
        case AOP_EXSTK:        /* 4 from ld iy, #... */
          return (7);
        case AOP_PAIRPTR:
          if (op2->aopu.aop_pairId == PAIR_HL)
            return (1);
          if (op2->aopu.aop_pairId == PAIR_IY || op2->aopu.aop_pairId == PAIR_IX)
            return (3);
          if (op2->aopu.aop_pairId == PAIR_BC || op2->aopu.aop_pairId == PAIR_DE)
            return ((aopInReg (op1, 0, A_IDX) || op1type == AOP_DUMMY) ? 1 : 2);
        default:
          printf ("ld_cost op1: AOP_REG, op2: %d\n", (int) (op2type));
          wassert (0);
        }
    case AOP_SFR:              /* 2 from out (...), a */
      switch (op2type)
        {
        case AOP_REG:
        case AOP_DUMMY:
          return (2);
        case AOP_IMMD:
        case AOP_LIT:
          return (4);
        case AOP_STK:
          return (5);
        case AOP_HL:           /* 3 from ld hl, #... */
          return (6);
        case AOP_SFR:
          return (4);
        case AOP_IY:           /* 4 from ld iy, #... */
        case AOP_EXSTK:        /* 4 from ld iy, #... */
          return (9);
        default:
          printf ("ld_cost op1: AOP_SFR, op2: %d\n", (int) (op2type));
          wassert (0);
        }
    case AOP_IY:               /* 4 from ld iy, #... */
    case AOP_EXSTK:            /* 4 from ld iy, #... */
      switch (op2type)
        {
        case AOP_IMMD:
        case AOP_LIT:
          return (8);
        case AOP_SFR:          /* 2 from in a, (...) */
          return (9);
        case AOP_STK:
        case AOP_HL:           /* 3 from ld hl, #... */
          return (10);
        case AOP_IY:
        case AOP_EXSTK:
          return (16);
        default:
          printf ("ld_cost op1: AOP_IY, op2: %d\n", (int) (op2type));
          wassert (0);
        }
    case AOP_STK:
      switch (op2type)
        {
        case AOP_IMMD:
        case AOP_LIT:
          return (4);
        case AOP_SFR:          /* 2 from in a, (...) */
          return (5);
        case AOP_STK:
          return (6);
        case AOP_HL:
          return (7);
        case AOP_IY:           /* 4 from ld iy, #... */
        case AOP_EXSTK:
          return (10);
        case AOP_PAIRPTR:
          if (op2->aopu.aop_pairId == PAIR_HL || op2->aopu.aop_pairId == PAIR_BC || op2->aopu.aop_pairId == PAIR_DE)
            return (4);
          if (op2->aopu.aop_pairId == PAIR_IY || op2->aopu.aop_pairId == PAIR_IX)
            return (6);
        default:
          printf ("ld_cost op1: AOP_STK, op2: %d\n", (int) (op2type));
          wassert (0);
        }
    case AOP_HL:               /* 3 from ld hl, #... */
      switch (op2type)
        {
        case AOP_REG:
        case AOP_DUMMY:
          return (4);
        case AOP_IMMD:
        case AOP_LIT:
          return (5);
        case AOP_STK:
          return (7);
        case AOP_SFR:
        case AOP_HL:
          return (6);
        case AOP_IY:           /* 4 from ld iy, #... */
        case AOP_EXSTK:
          return (11);
        default:
          printf ("ld_cost op1: AOP_HL, op2: %d", (int) (op2type));
          wassert (0);
        }
    case AOP_LIT:
    case AOP_IMMD:
      wassertl (0, "Trying to assign a value to a literal");
      break;
    default:
      printf ("ld_cost op1: %d\n", (int) (op1type));
      wassert (0);
    }
  return (8);                   // Fallback
}

static unsigned char
op8_cost (const asmop * op2)
{
  switch (op2->type)
    {
    case AOP_REG:
    case AOP_DUMMY:
      return (1);
    case AOP_IMMD:
    case AOP_LIT:
      return (2);
    case AOP_STK:
      return (3);
    case AOP_HL:
      return (4);
    case AOP_IY:               /* 4 from ld iy, #... */
    case AOP_EXSTK:            /* 4 from ld iy, #... */
      return (7);
    case AOP_PAIRPTR:
      if (op2->aopu.aop_pairId == PAIR_HL)
        return (1);
      if (op2->aopu.aop_pairId == PAIR_IY || op2->aopu.aop_pairId == PAIR_IX)
        return (3);
    default:
      printf ("op8_cost op2: %d\n", (int) (op2->type));
      wassert (0);
    }
  return (8);                   // Fallback
}

static unsigned char
bit8_cost (const asmop * op1)
{
  switch (op1->type)
    {
    case AOP_REG:
    case AOP_DUMMY:
      return (2);
    case AOP_STK:
      return (4);
    case AOP_HL:
      return (5);
    case AOP_IY:               /* 4 from ld iy, #... */
    case AOP_EXSTK:            /* 4 from ld iy, #... */
      return (8);
    default:
      printf ("bit8_cost op1: %d\n", (int) (op1->type));
      wassert (0);
    }
  return (8);                   //Fallback
}

static unsigned char
emit3Cost (enum asminst inst, const asmop *op1, int offset1, const asmop *op2, int offset2)
{
  if (op2 && offset2 >= op2->size)
    op2 = ASMOP_ZERO;

  switch (inst)
    {
    case A_CPL:
    case A_RLA:
    case A_RLCA:
    case A_RRA:
    case A_RRCA:
      return (1);
    case A_NEG:
      return(2);
    case A_LD:
      return (ld_cost (op1, op2));
    case A_ADD:
    case A_ADC:
    case A_AND:
    case A_CP:
    case A_OR:
    case A_SBC:
    case A_SUB:
    case A_XOR:
      return (op8_cost (op2));
    case A_DEC:
    case A_INC:
      return (op8_cost (op1));
    case A_RL:
    case A_RLC:
    case A_RR:
    case A_RRC:
    case A_SLA:
    case A_SRA:
    case A_SRL:
    case A_SWAP:
      return (bit8_cost (op1));
    default:
      wassertl (0, "Tried get cost for unknown instruction");
    }
  return (0);
}

static void
emit3_o (enum asminst inst, asmop *op1, int offset1, asmop *op2, int offset2)
{
  unsigned char cost;

  regalloc_dry_run_cost += emit3Cost (inst, op1, offset1, op2, offset2);
  if (regalloc_dry_run)
    return;

  cost = regalloc_dry_run_cost;
  if (!op1)
    emit2 ("%s", asminstnames[inst]);
  else if (!op2)
    emit2 ("%s %s", asminstnames[inst], aopGet (op1, offset1, FALSE));
  else
    {
      char *l = Safe_strdup (aopGet (op1, offset1, FALSE));
      //emit2("%s %s, %s", asminstnames[inst], aopGet(op1, offset1, FALSE), aopGet(op2, offset2, FALSE));
      emit2 ("%s %s, %s", asminstnames[inst], l, aopGet (op2, offset2, FALSE));
      Safe_free (l);
    }

  regalloc_dry_run_cost = cost;
  //emitDebug(";emit3_o cost: %d total so far: %d", (int)emit3Cost(inst, op1, offset1, op2, offset2), (int)cost);
}

static void
emit3 (enum asminst inst, asmop *op1, asmop *op2)
{
  emit3_o (inst, op1, 0, op2, 0);
}

static void
_emitMove (const char *to, const char *from)
{
  if (STRCASECMP (to, from) != 0)
    {
      emit2 ("ld %s,%s", to, from);
    }
  else
    {
      // Optimise it out.
      // Could leave this to the peephole, but sometimes the peephole is inhibited.
    }
}

static void
_emitMove3 (asmop *to, int to_offset, asmop *from, int from_offset)
{
  /* Todo: Longer list of moves that can be optimized out. */
  if (to_offset == from_offset)
    {
      if (to->type == AOP_REG && from->type == AOP_REG && to->aopu.aop_reg[to_offset] == from->aopu.aop_reg[from_offset])
        return;
    }

  emit3_o (A_LD, to, to_offset, from, from_offset);
}

#if 0
static const char *aopNames[] =
{
  "AOP_INVALID",
  "AOP_LIT",
  "AOP_REG",
  "AOP_DIR",
  "AOP_SFR",
  "AOP_STK",
  "AOP_IMMD",
  "AOP_STR",
  "AOP_CRY",
  "AOP_IY",
  "AOP_HL",
  "AOP_EXSTK",
  "AOP_PAIRPT",
  "AOP_DUMMY"
};

static void
aopDump (const char *plabel, asmop * aop)
{
  int i;
  char regbuf[9];
  char *rbp = regbuf;

  emitDebug ("; Dump of %s: type %s size %u", plabel, aopNames[aop->type], aop->size);
  switch (aop->type)
    {
    case AOP_EXSTK:
    case AOP_STK:
      emitDebug (";  aop_stk %d", aop->aopu.aop_stk);
      break;
    case AOP_REG:
      for (i = aop->size - 1; i >= 0; i--)
        *rbp++ = *(aop->aopu.aop_reg[i]->name);
      *rbp = '\0';
      emitDebug (";  reg = %s", regbuf);
      break;
    case AOP_PAIRPTR:
      emitDebug (";  pairptr = (%s)", _pairs[aop->aopu.aop_pairId].name);

    default:
      /* No information. */
      break;
    }
}
#endif

static void
_moveA (const char *moveFrom)
{
  _emitMove ("a", moveFrom);
}

/* Load aop into A */
static void
_moveA3 (asmop * from, int offset)
{
  _emitMove3 (ASMOP_A, 0, from, offset);
}

static const char *
getPairName (asmop *aop)
{
  if (aop->type == AOP_REG)
    {
      switch (aop->aopu.aop_reg[0]->rIdx)
        {
        case C_IDX:
          return "bc";
          break;
        case E_IDX:
          return "de";
          break;
        case L_IDX:
          return "hl";
          break;
        case IYL_IDX:
          return "iy";
          break;
        }
    }
  else if (aop->type == AOP_STR)
    {
      int i;
      for (i = 0; i < NUM_PAIRS; i++)
        {
          if (strcmp (aop->aopu.aop_str[0], _pairs[i].l) == 0)
            return _pairs[i].name;
        }
    }
  wassertl (0, "Tried to get the pair name of something that isn't a pair");
  return NULL;
}

/** Returns TRUE if the registers used in aop form a pair (BC, DE, HL) */
static bool
isPair (const asmop *aop)
{
  return (getPairId (aop) != PAIR_INVALID);
}

/** Returns TRUE if the registers used in aop cannot be split into high
    and low halves */
static bool
isUnsplitable (const asmop * aop)
{
  switch (getPairId (aop))
    {
    case PAIR_IX:
    case PAIR_IY:
      return TRUE;
    default:
      return FALSE;
    }
  return FALSE;
}

static bool
isPtrPair (const asmop * aop)
{
  PAIR_ID pairId = getPairId (aop);
  switch (pairId)
    {
    case PAIR_HL:
    case PAIR_IY:
    case PAIR_IX:
      return TRUE;
    default:
      return FALSE;
    }
}

static void
spillPair (PAIR_ID pairId)
{
  _G.pairs[pairId].last_type = AOP_INVALID;
  _G.pairs[pairId].base = NULL;
}

/* Given a register name, spill the pair (if any) the register is part of */
static void
spillPairReg (const char *regname)
{
  if (strlen (regname) == 1)
    {
      switch (*regname)
        {
        case 'h':
        case 'l':
          spillPair (PAIR_HL);
          break;
        case 'd':
        case 'e':
          spillPair (PAIR_DE);
          break;
        case 'b':
        case 'c':
          spillPair (PAIR_BC);
          break;
        }
    }
}

static void
_push (PAIR_ID pairId)
{
  emit2 ("push %s", _pairs[pairId].name);
  regalloc_dry_run_cost += (pairId == PAIR_IX || pairId == PAIR_IY ? 2 : 1);
  _G.stack.pushed += 2;
}

static void
_pop (PAIR_ID pairId)
{
  if (pairId != PAIR_INVALID)
    {
      emit2 ("pop %s", _pairs[pairId].name);
      regalloc_dry_run_cost += (pairId == PAIR_IX || pairId == PAIR_IY ? 2 : 1);
      _G.stack.pushed -= 2;
      spillPair (pairId);
    }
}

static void
genMovePairPair (PAIR_ID srcPair, PAIR_ID dstPair)
{
  switch (dstPair)
    {
    case PAIR_IX:
    case PAIR_IY:
    case PAIR_AF:
      _push (srcPair);
      _pop (dstPair);
      break;
    case PAIR_BC:
    case PAIR_DE:
    case PAIR_HL:
      if (srcPair == PAIR_IX || srcPair == PAIR_IY)
        {
          _push (srcPair);
          _pop (dstPair);
        }
      else
        {
          emit2 ("ld %s, %s", _pairs[dstPair].l, _pairs[srcPair].l);
          emit2 ("ld %s, %s", _pairs[dstPair].h, _pairs[srcPair].h);
          regalloc_dry_run_cost += 2;
        }
      break;
    default:
      wassertl (0, "Tried to move a nonphysical pair");
    }
  _G.pairs[dstPair].last_type = _G.pairs[srcPair].last_type;
  _G.pairs[dstPair].base = _G.pairs[srcPair].base;
  _G.pairs[dstPair].offset = _G.pairs[srcPair].offset;
}


/*-----------------------------------------------------------------*/
/* newAsmop - creates a new asmOp                                  */
/*-----------------------------------------------------------------*/
static asmop *
newAsmop (short type)
{
  asmop *aop;

  aop = traceAlloc (&_G.trace.aops, Safe_alloc (sizeof (asmop)));
  aop->type = type;
  memset (aop->regs, -1, 9);
  return aop;
}

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (const iCode * ic, symbol * sym, bool requires_a)
{
  asmop *aop;
  memmap *space;

  wassert (ic);
  wassert (sym);
  wassert (sym->etype);

  space = SPEC_OCLS (sym->etype);

  /* if already has one */
  if (sym->aop)
    {
      return sym->aop;
    }

  /* Assign depending on the storage class */
  if (sym->onStack || sym->iaccess)
    {
      /* The pointer that is used depends on how big the offset is.
         Normally everything is AOP_STK, but for offsets of < -128 or
         > 127 on the Z80 an extended stack pointer is used.
       */
      if (!IS_GB && (_G.omitFramePtr || sym->stack < INT8MIN || sym->stack > (int) (INT8MAX - getSize (sym->type))))
        {
          emitDebug ("; AOP_EXSTK for %s, _G.omitFramePtr %d, sym->stack %d, size %d", sym->rname, (int) (_G.omitFramePtr),
                     sym->stack, getSize (sym->type));
          sym->aop = aop = newAsmop (AOP_EXSTK);
        }
      else
        {
          emitDebug ("; AOP_STK for %s", sym->rname);
          sym->aop = aop = newAsmop (AOP_STK);
        }

      aop->size = getSize (sym->type);
      aop->aopu.aop_stk = sym->stack;
      return aop;
    }

  /* special case for a function */
  if (IS_FUNC (sym->type))
    {
      sym->aop = aop = newAsmop (AOP_IMMD);
      aop->aopu.aop_immd = traceAlloc (&_G.trace.aops, Safe_strdup (sym->rname));
      aop->size = 2;
      return aop;
    }

  if (IN_REGSP (space))
    {
      /*.p.t.20030716 minor restructure to add SFR support to the Z80 */
      if (IS_GB)
        {
          /* if it is in direct space */
          if (!requires_a)
            {
              sym->aop = aop = newAsmop (AOP_SFR);
              aop->aopu.aop_dir = sym->rname;
              aop->size = getSize (sym->type);
              /* emitDebug ("; AOP_SFR for %s", sym->rname); */
              return aop;
            }
        }
      else
        {
          /*.p.t.20030716 adding SFR support to the Z80 port */
          aop = newAsmop (AOP_SFR);
          sym->aop = aop;
          aop->aopu.aop_dir = sym->rname;
          aop->size = getSize (sym->type);
          aop->paged = FUNC_REGBANK (sym->type);
          aop->bcInUse = isPairInUse (PAIR_BC, ic);
          /* emitDebug (";Z80 AOP_SFR for %s banked:%d bc:%d", sym->rname, FUNC_REGBANK (sym->type), aop->bcInUse); */

          return (aop);
        }
    }

  /* only remaining is far space */
  /* in which case DPTR gets the address */
  if (IS_GB || IY_RESERVED)
    {
      /* emitDebug ("; AOP_HL for %s", sym->rname); */
      sym->aop = aop = newAsmop (AOP_HL);
    }
  else
    sym->aop = aop = newAsmop (AOP_IY);

  aop->size = getSize (sym->type);
  aop->aopu.aop_dir = sym->rname;

  /* if it is in code space */
  if (IN_CODESPACE (space))
    aop->code = 1;

  return aop;
}

/*-----------------------------------------------------------------*/
/* aopForRemat - rematerializes an object                          */
/*-----------------------------------------------------------------*/
static asmop *
aopForRemat (symbol *sym)
{
  iCode *ic = sym->rematiCode;
  asmop *aop = newAsmop (AOP_IMMD);
  int val = 0;
  struct dbuf_s dbuf;

  wassert(ic);

  for (;;)
    {
      if (ic->op == '+')
        {
          if (isOperandLiteral (IC_RIGHT (ic)))
            {
              val += (int) operandLitValue (IC_RIGHT (ic));
              ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
            }
          else
            {
              val += (int) operandLitValue (IC_LEFT (ic));
              ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
            }
        }
      else if (ic->op == '-')
        {
          val -= (int) operandLitValue (IC_RIGHT (ic));
          ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
        }
      else if (IS_CAST_ICODE (ic))
        {
          ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
        }
      else if (ic->op == ADDRESS_OF)
        {
          val += (int) operandLitValue (IC_RIGHT (ic));
          break;
        }
      else
        break;
    }

  dbuf_init (&dbuf, 128);
  if (val)
    {
      dbuf_printf (&dbuf, "(%s %c 0x%04x)", OP_SYMBOL (IC_LEFT (ic))->rname, val >= 0 ? '+' : '-', abs (val) & 0xffff);
    }
  else
    {
      dbuf_append_str (&dbuf, OP_SYMBOL (IC_LEFT (ic))->rname);
    }

  aop->aopu.aop_immd = traceAlloc (&_G.trace.aops, dbuf_detach_c_str (&dbuf));
  return aop;
}

/*-----------------------------------------------------------------*/
/* regsInCommon - two operands have some registers in common       */
/*-----------------------------------------------------------------*/
static bool
regsInCommon (operand * op1, operand * op2)
{
  symbol *sym1, *sym2;
  int i;

  /* if they have registers in common */
  if (!IS_SYMOP (op1) || !IS_SYMOP (op2))
    return FALSE;

  sym1 = OP_SYMBOL (op1);
  sym2 = OP_SYMBOL (op2);

  if (sym1->nRegs == 0 || sym2->nRegs == 0)
    return FALSE;

  for (i = 0; i < sym1->nRegs; i++)
    {
      int j;
      if (!sym1->regs[i])
        continue;

      for (j = 0; j < sym2->nRegs; j++)
        {
          if (!sym2->regs[j])
            continue;

          if (sym2->regs[j] == sym1->regs[i])
            return TRUE;
        }
    }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* operandsEqu - equivalent                                        */
/*-----------------------------------------------------------------*/
static bool
operandsEqu (operand * op1, operand * op2)
{
  symbol *sym1, *sym2;

  /* if they not symbols */
  if (!IS_SYMOP (op1) || !IS_SYMOP (op2))
    return FALSE;

  sym1 = OP_SYMBOL (op1);
  sym2 = OP_SYMBOL (op2);

  /* if both are itemps & one is spilt
     and the other is not then false */
  if (IS_ITEMP (op1) && IS_ITEMP (op2) && sym1->isspilt != sym2->isspilt)
    return FALSE;

  /* if they are the same */
  if (sym1 == sym2)
    return 1;

  if (sym1->rname[0] && sym2->rname[0] && strcmp (sym1->rname, sym2->rname) == 0)
    return 2;

  /* if left is a tmp & right is not */
  if (IS_ITEMP (op1) && !IS_ITEMP (op2) && sym1->isspilt && (sym1->usl.spillLoc == sym2))
    return 3;

  if (IS_ITEMP (op2) && !IS_ITEMP (op1) && sym2->isspilt && sym1->level > 0 && (sym2->usl.spillLoc == sym1))
    return 4;

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* sameRegs - two asmops have the same registers                   */
/*-----------------------------------------------------------------*/
static bool
sameRegs (asmop * aop1, asmop * aop2)
{
  int i;

  if (aop1->type == AOP_SFR || aop2->type == AOP_SFR)
    return FALSE;

  if (aop1 == aop2)
    return TRUE;

  if (! regalloc_dry_run && // Todo: Check if always enabling this even for dry runs tends to result in better code.
    (aop1->type == AOP_STK && aop2->type == AOP_STK ||
    aop1->type == AOP_EXSTK && aop2->type == AOP_EXSTK))
    return (aop1->aopu.aop_stk == aop2->aopu.aop_stk);

  if (aop1->type != AOP_REG || aop2->type != AOP_REG)
    return FALSE;

  if (aop1->size != aop2->size)
    return FALSE;

  for (i = 0; i < aop1->size; i++)
    if (aop1->aopu.aop_reg[i] != aop2->aopu.aop_reg[i])
      return FALSE;

  return TRUE;
}

/*-----------------------------------------------------------------*/
/* aopOp - allocates an asmop for an operand  :                    */
/*-----------------------------------------------------------------*/
static void
aopOp (operand *op, const iCode *ic, bool result, bool requires_a)
{
  asmop *aop;
  symbol *sym;
  int i;

  if (!op)
    return;

  /* if this a literal */
  if (IS_OP_LITERAL (op)) /* TODO:  && !op->isaddr, handle address literals in a sane way */
    {
      op->aop = aop = newAsmop (AOP_LIT);
      aop->aopu.aop_lit = OP_VALUE (op);
      aop->size = getSize (operandType (op));
      return;
    }

  /* if already has a asmop then continue */
  if (op->aop)
    {
      if (op->aop->type == AOP_SFR)
        {
          op->aop->bcInUse = isPairInUse (PAIR_BC, ic);
        }
      return;
    }

  /* if the underlying symbol has a aop */
  if (IS_SYMOP (op) && OP_SYMBOL (op)->aop)
    {
      op->aop = OP_SYMBOL (op)->aop;
      if (op->aop->type == AOP_SFR)
        {
          op->aop->bcInUse = isPairInUse (PAIR_BC, ic);
        }
      return;
    }

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aopForSym (ic, OP_SYMBOL (op), requires_a);
      return;
    }

  /* this is a temporary : this has
     only four choices :
     a) register
     b) spillocation
     c) rematerialize
     d) conditional
     e) can be a return use only */

  sym = OP_SYMBOL (op);

  /* if the type is a conditional */
  if (sym->regType == REG_CND)
    {
      aop = op->aop = sym->aop = newAsmop (AOP_CRY);
      aop->size = 0;
      return;
    }

  /* if it is spilt then two situations
     a) is rematerialize
     b) has a spill location */
  if (sym->isspilt || sym->nRegs == 0)
    {
      if (sym->ruonly)
        {
          int i;
          aop = op->aop = sym->aop = newAsmop (AOP_STR);
          aop->size = getSize (sym->type);
          for (i = 0; i < 4; i++)
            aop->aopu.aop_str[i] = ASMOP_RETURN->aopu.aop_reg[i]->name;
          return;
        }

      if (sym->accuse)
        {
          if (sym->accuse == ACCUSE_A) /* For compability with old register allocator only */
            {
              sym->aop = op->aop = aop = newAsmop (AOP_REG);
              aop->size = getSize (sym->type);
              wassertl (aop->size == 1, "Internal error: Caching in A, but too big to fit in A");
              aop->aopu.aop_reg[0] = regsZ80 + A_IDX;
            }
          else if (sym->accuse == ACCUSE_IY) /* For compability with old register allocator only */
            {
              sym->aop = op->aop = aop = newAsmop (AOP_REG);
              aop->size = getSize (sym->type);
              wassertl (aop->size <= 2, "Internal error: Caching in IY, but too big to fit in IY");
              aop->aopu.aop_reg[0] = regsZ80 + IYL_IDX;
              aop->aopu.aop_reg[0] = regsZ80 + IYH_IDX;
            }
          else
            {
              wassertl (0, "Marked as being allocated into A or IY but is actually in neither");
            }
          return;
        }

      /* rematerialize it NOW */
      if (sym->remat)
        {
          sym->aop = op->aop = aop = aopForRemat (sym);
          aop->size = getSize (sym->type);
          return;
        }

      /* On-stack for dry run. */
      if (sym->nRegs && regalloc_dry_run)
        {
          sym->aop = op->aop = aop = newAsmop (AOP_STK);
          aop->size = getSize (sym->type);
          return;
        }

      /* On stack. */
      if (sym->isspilt && sym->usl.spillLoc)
        {
          asmop *oldAsmOp = NULL;

          if (getSize (sym->type) != getSize (sym->usl.spillLoc->type))
            {
              /* force a new aop if sizes differ */
              oldAsmOp = sym->usl.spillLoc->aop;
              sym->usl.spillLoc->aop = NULL;
            }
          sym->aop = op->aop = aop = aopForSym (ic, sym->usl.spillLoc, requires_a);
          if (getSize (sym->type) != getSize (sym->usl.spillLoc->type))
            {
              /* Don't reuse the new aop, go with the last one */
              sym->usl.spillLoc->aop = oldAsmOp;
            }
          aop->size = getSize (sym->type);
          return;
        }

      /* else must be a dummy iTemp */
      sym->aop = op->aop = aop = newAsmop (AOP_DUMMY);
      aop->size = getSize (sym->type);
      return;
    }

  /* must be in a register */
  sym->aop = op->aop = aop = newAsmop (AOP_REG);
  aop->size = sym->nRegs;
  for (i = 0; i < sym->nRegs; i++)
    {
      wassertl (sym->regs[i], "Symbol in register, but no register assigned.");
      if(!sym->regs[i])
        fprintf(stderr, "Symbol %s at ic %d.\n", sym->name, ic->key);
      aop->aopu.aop_reg[i] = sym->regs[i];
      aop->regs[sym->regs[i]->rIdx] = i;
    }
}

/*-----------------------------------------------------------------*/
/* freeAsmop - free up the asmop given to an operand               */
/*----------------------------------------------------------------*/
static void
freeAsmop (operand * op, asmop *aaop)
{
  asmop *aop;

  if (!op)
    aop = aaop;
  else
    aop = op->aop;

  if (!aop)
    return;

  if (aop->freed)
    goto dealloc;

  aop->freed = 1;

  if (aop->type == AOP_PAIRPTR && !IS_GB && aop->aopu.aop_pairId == PAIR_DE)
    {
      _pop (aop->aopu.aop_pairId);
    }

  if (getPairId (aop) == PAIR_HL)
    {
      spillPair (PAIR_HL);
    }

dealloc:
  /* all other cases just dealloc */
  if (op)
    {
      op->aop = NULL;
      if (IS_SYMOP (op))
        {
          OP_SYMBOL (op)->aop = NULL;
          /* if the symbol has a spill */
          if (SPIL_LOC (op))
            SPIL_LOC (op)->aop = NULL;
        }
    }

}

static bool
isLitWord (const asmop *aop)
{
  /*    if (aop->size != 2)
     return FALSE; */
  switch (aop->type)
    {
    case AOP_IMMD:
    case AOP_LIT:
      return TRUE;
    default:
      return FALSE;
    }
}

static const char *
aopGetLitWordLong (const asmop *aop, int offset, bool with_hash)
{
  static struct dbuf_s dbuf = { 0 };

  if (dbuf_is_initialized (&dbuf))
    {
      dbuf_set_length (&dbuf, 0);
    }
  else
    {
      dbuf_init (&dbuf, 128);
    }

  /* depending on type */
  switch (aop->type)
    {
    case AOP_HL:
    case AOP_IY:
    case AOP_IMMD:
      /* PENDING: for re-target */
      if (with_hash)
        {
          dbuf_tprintf (&dbuf, "!hashedstr + %d", aop->aopu.aop_immd, offset);
        }
      else if (offset == 0)
        {
          dbuf_tprintf (&dbuf, "%s", aop->aopu.aop_immd);
        }
      else
        {
          dbuf_tprintf (&dbuf, "%s + %d", aop->aopu.aop_immd, offset);
        }
      break;

    case AOP_LIT:
    {
      value *val = aop->aopu.aop_lit;
      /* if it is a float then it gets tricky */
      /* otherwise it is fairly simple */
      if (!IS_FLOAT (val->type))
        {
          unsigned long long v = ullFromVal (val);

          v >>= (offset * 8);

          dbuf_tprintf (&dbuf, with_hash ? "!immedword" : "!constword", (unsigned long) (v & 0xffffull));
        }
      else
        {
          union
          {
            float f;
            unsigned char c[4];
          }
          fl;
          unsigned int i;

          /* it is type float */
          fl.f = (float) floatFromVal (val);

#ifdef WORDS_BIGENDIAN
          i = fl.c[3 - offset] | (fl.c[3 - offset - 1] << 8);
#else
          i = fl.c[offset] | (fl.c[offset + 1] << 8);
#endif
          dbuf_tprintf (&dbuf, with_hash ? "!immedword" : "!constword", i);
        }
    }
    break;

    case AOP_REG:
    case AOP_STK:
    case AOP_DIR:
    case AOP_SFR:
    case AOP_STR:
    case AOP_CRY:
    case AOP_EXSTK:
    case AOP_PAIRPTR:
    case AOP_DUMMY:
      break;

    default:
      dbuf_destroy (&dbuf);
      fprintf (stderr, "aop->type: %d\n", aop->type);
      wassertl (0, "aopGetLitWordLong got unsupported aop->type");
      exit (0);
    }
  return dbuf_c_str (&dbuf);
}

static bool
isPtr (const char *s)
{
  if (!strcmp (s, "hl"))
    return TRUE;
  if (!strcmp (s, "ix"))
    return TRUE;
  if (!strcmp (s, "iy"))
    return TRUE;
  return FALSE;
}

static void
adjustPair (const char *pair, int *pold, int new_val)
{
  wassert (pair);

  while (*pold < new_val)
    {
      emit2 ("inc %s", pair);
      (*pold)++;
    }
  while (*pold > new_val)
    {
      emit2 ("dec %s", pair);
      (*pold)--;
    }
}

static void
spillCached (void)
{
  spillPair (PAIR_HL);
  spillPair (PAIR_IY);
}

static bool
requiresHL (const asmop * aop)
{
  switch (aop->type)
    {
    case AOP_IY:
      return FALSE;
    case AOP_HL:
    case AOP_EXSTK:
      return TRUE;
    case AOP_STK:
      return (IS_GB || _G.omitFramePtr);
    case AOP_REG:
    {
      int i;
      for (i = 0; i < aop->size; i++)
        {
          wassert (aop->aopu.aop_reg[i]);
          if (aop->aopu.aop_reg[i]->rIdx == L_IDX || aop->aopu.aop_reg[i]->rIdx == H_IDX)
            return TRUE;
        }
    }
    case AOP_PAIRPTR:
      return (aop->aopu.aop_pairId == PAIR_HL);
    default:
      return FALSE;
    }
}

/*----------------------------------------------------------*/
/* strtoul_z80: a wrapper to strtoul, which can also handle */
/* hex numbers with a $ prefix.                             */
/*----------------------------------------------------------*/
static unsigned long int 
strtoul_z80asm (const char *nptr, char **endptr, int base)
{
  char *p = NULL;
  int i, flag = 0, len;
  unsigned long ret;

  if (nptr != NULL && (p = malloc ((len = strlen (nptr)) + 1 + 1)) != NULL)
    {
      memset (p, 0, len + 2);
      for (i = 0; i < len; i++)
        {
          if (!flag)
            if (isspace (nptr[i]))
              p[i] = nptr[i];
            else if (nptr[i] == '$')
              {
                p[i] = '0';
                p[i + 1] = 'x';
                flag = 1;
              }
            else
              break;
          else
            p[i + 1] = nptr[i];
        }
    }

  if (flag)
    ret = strtoul (p, endptr, base);
  else
    ret = strtoul (nptr, endptr, base);

  if (p)
    free (p);
  return ret;
}

static void
fetchLitPair (PAIR_ID pairId, asmop *left, int offset)
{
  const char *pair = _pairs[pairId].name;
  char *l = Safe_strdup (aopGetLitWordLong (left, offset, FALSE));
  char *base_str = Safe_strdup (aopGetLitWordLong (left, 0, FALSE));
  const char *base = base_str;

  wassert (pair);

  emitDebug (";fetchLitPair");

  if (isPtr (pair))
    {
      if (pairId == PAIR_HL || pairId == PAIR_IY)
        {
          if (pairId == PAIR_HL && base[0] == '0')      // Ugly workaround
            {
              unsigned int tmpoffset;
              const char *tmpbase;
              if (sscanf (base, "%xd", &tmpoffset) && (tmpbase = strchr (base, '+')))
                {
                  offset = tmpoffset;
                  base = tmpbase++;
                }
            }
          if ((_G.pairs[pairId].last_type == AOP_IMMD && left->type == AOP_IMMD) ||
              (_G.pairs[pairId].last_type == AOP_IY && left->type == AOP_IY) ||
              (_G.pairs[pairId].last_type == AOP_HL && left->type == AOP_HL))
            {
              if (!regalloc_dry_run && _G.pairs[pairId].base && !strcmp (_G.pairs[pairId].base, base))  // Todo: Exact cost.
                {
                  if (pairId == PAIR_HL && abs (_G.pairs[pairId].offset - offset) < 3)
                    {
                      adjustPair (pair, &_G.pairs[pairId].offset, offset);
                      goto adjusted;
                    }
                  if (pairId == PAIR_IY && offset == _G.pairs[pairId].offset)
                    goto adjusted;
                }
            }
        }

      if (pairId == PAIR_HL && left->type == AOP_LIT && _G.pairs[pairId].last_type == AOP_LIT &&
          !IS_FLOAT (left->aopu.aop_lit->type) && offset == 0 && _G.pairs[pairId].offset == 0)
        {
          unsigned new_low, new_high, old_low, old_high;
          unsigned long v_new = ulFromVal (left->aopu.aop_lit);
          unsigned long v_old = strtoul_z80asm (_G.pairs[pairId].base, NULL, 0);
          new_low = (v_new >> 0) & 0xff;
          new_high = (v_new >> 8) & 0xff;
          old_low = (v_old >> 0) & 0xff;
          old_high = (v_old >> 8) & 0xff;

          /* Change lower byte only. */
          if (new_high == old_high)
            {
              emit3_o (A_LD, ASMOP_L, 0, left, 0);
              goto adjusted;
            }
          /* Change upper byte only. */
          else if (new_low == old_low)
            {
              emit3_o (A_LD, ASMOP_H, 0, left, 1);
              goto adjusted;
            }
        }


      _G.pairs[pairId].last_type = left->type;
      _G.pairs[pairId].base = traceAlloc (&_G.trace.aops, Safe_strdup (base));
      _G.pairs[pairId].offset = offset;
    }
  /* Both a lit on the right and a true symbol on the left */
  emit2 ("ld %s, !hashedstr", pair, l);
  regalloc_dry_run_cost += (pairId == PAIR_IX || pairId == PAIR_IY) ? 4 : 3;
  Safe_free (base_str);
  Safe_free (l);
  return;

adjusted:
  _G.pairs[pairId].last_type = left->type;
  _G.pairs[pairId].base = traceAlloc (&_G.trace.aops, Safe_strdup (base));
  _G.pairs[pairId].offset = offset;
  Safe_free (base_str);
  Safe_free (l);
}

static PAIR_ID
makeFreePairId (const iCode * ic, bool * pisUsed)
{
  *pisUsed = FALSE;

  if (ic != NULL)
    {
      if (!bitVectBitValue (ic->rMask, B_IDX) && !bitVectBitValue (ic->rMask, C_IDX))
        {
          return PAIR_BC;
        }
      else if (!IS_GB && !bitVectBitValue (ic->rMask, D_IDX) && !bitVectBitValue (ic->rMask, E_IDX))
        {
          return PAIR_DE;
        }
      else
        {
          *pisUsed = TRUE;
          return PAIR_HL;
        }
    }
  else
    {
      *pisUsed = TRUE;
      return PAIR_HL;
    }
}

/* If ic != 0, we can safely use isPairDead(). */
static void
fetchPairLong (PAIR_ID pairId, asmop *aop, const iCode *ic, int offset)
{
  emitDebug (";fetchPairLong");

  /* if this is rematerializable */
  if (isLitWord (aop))
    fetchLitPair (pairId, aop, offset);
  else
    {
      if (getPairId (aop) == pairId)
        {
          /* Do nothing */
        }
      else if (IS_EZ80_Z80 && aop->size - offset >= 2 && aop->type == AOP_STK)
        {
          int fp_offset = aop->aopu.aop_stk + offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
          emit2 ("ld %s, %d (ix)", _pairs[pairId].name, fp_offset);
          regalloc_dry_run_cost += 3;
        }
      /* Getting the parameter by a pop / push sequence is cheaper when we have a free pair (except for the Rabbit, which has an even cheaper sp-relative load).
         Stack allocation can change after register allocation, so assume this optimization is not possible for the allocator's cost function (unless the stack location is for a parameter). */
      else if (!IS_RAB && aop->size - offset >= 2 &&
               (aop->type == AOP_STK || aop->type == AOP_EXSTK) && (!regalloc_dry_run || aop->aopu.aop_stk > 0)
               && (aop->aopu.aop_stk + offset + _G.stack.offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0) +
                   _G.stack.pushed) == 2 && ic && getFreePairId (ic) != PAIR_INVALID && getFreePairId (ic) != pairId)
        {
          PAIR_ID extrapair = getFreePairId (ic);
          _pop (extrapair);
          _pop (pairId);
          _push (pairId);
          _push (extrapair);
        }
      /* Todo: Use even cheaper ex hl, (sp) and ex iy, (sp) when possible. */
      else if ((!IS_RAB || pairId == PAIR_BC || pairId == PAIR_DE) && aop->size - offset >= 2 &&
               (aop->type == AOP_STK || aop->type == AOP_EXSTK) && (!regalloc_dry_run || aop->aopu.aop_stk > 0)
               && (aop->aopu.aop_stk + offset + _G.stack.offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0) +
                   _G.stack.pushed) == 0)
        {
          _pop (pairId);
          _push (pairId);
        }
      else if (!IS_GB && (aop->type == AOP_IY || aop->type == AOP_HL) && !(pairId == PAIR_IY && aop->size < 2))
        {
          /* Instead of fetching relative to IY, just grab directly
             from the address IY refers to */
          emit2 ("ld %s, (%s)", _pairs[pairId].name, aopGetLitWordLong (aop, offset, FALSE));
          regalloc_dry_run_cost += (pairId == PAIR_HL ? 3 : 4);

          if (aop->size < 2)
            {
              emit2 ("ld %s, !zero", _pairs[pairId].h);
              regalloc_dry_run_cost += 2;
            }
        }
      /* we need to get it byte by byte */
      else if (pairId == PAIR_HL && (IS_GB || (IY_RESERVED && (aop->type == AOP_HL || aop->type == AOP_EXSTK))) && requiresHL (aop))
        {
          if (!regalloc_dry_run)        // TODO: Fix this to get correct cost!
            aopGet (aop, offset, FALSE);
          switch (aop->size - offset)
            {
            case 1:
              emit2 ("ld l, !*hl");
              emit2 ("ld h, !immedbyte", 0);
              regalloc_dry_run_cost += 3;
              break;
            default:
              wassertl (aop->size - offset > 1, "Attempted to fetch no data into HL");
              if (IS_RAB || IS_TLCS90)
                {
                  emit2 ("ld hl, 0 (hl)");
                  regalloc_dry_run_cost += 3;
                }
              else if (IS_EZ80_Z80)
                {
                  emit2 ("ld hl, (hl)");
                  regalloc_dry_run_cost += 2;
                }
              else
                {
                  if (ic && bitVectBitValue (ic->rMask, A_IDX))
                    _push (PAIR_AF);

                  emit2 ("ld a, !*hl");
                  emit2 ("inc hl");
                  emit2 ("ld h, !*hl");
                  emit2 ("ld l, a");
                  regalloc_dry_run_cost += 4;

                  if (ic && bitVectBitValue (ic->rMask, A_IDX))
                    _pop (PAIR_AF);
                }
              break;
            }
        }
      else if (pairId == PAIR_IY)
        {
          /* The Rabbit has the ld iy, n (sp) instruction. */
          int fp_offset = aop->aopu.aop_stk + offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
          int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
          if ((IS_RAB || IS_TLCS90) && (aop->type == AOP_STK || aop->type == AOP_EXSTK) && abs (sp_offset) <= 127)
            {
              emit2 ("ld iy, %d (sp)", sp_offset);
              regalloc_dry_run_cost += 3;
            }
          else if (isPair (aop) && (IS_RAB || IS_TLCS90) && getPairId (aop) == PAIR_HL)
            {
              emit2 ("ld iy, hl");
              regalloc_dry_run_cost += (1 + IS_RAB);
            }
          else if (isPair (aop))
            {
              emit2 ("push %s", _pairs[getPairId (aop)].name);
              emit2 ("pop iy");
              regalloc_dry_run_cost += 3;
            }
          else
            {
              bool isUsed;
              PAIR_ID id = makeFreePairId (ic, &isUsed);
              if (isUsed)
                _push (id);
              /* Can't load into parts, so load into HL then exchange. */
              if (!regalloc_dry_run)
                {
                  emit2 ("ld %s, %s", _pairs[id].l, aopGet (aop, offset, FALSE));
                  emit2 ("ld %s, %s", _pairs[id].h, aopGet (aop, offset + 1, FALSE));
                }
              regalloc_dry_run_cost += ld_cost (ASMOP_L, aop) + ld_cost (ASMOP_H, aop);

              if ((IS_RAB || IS_TLCS90) && id == PAIR_HL)
                {
                  emit2 ("ld iy, hl");
                  regalloc_dry_run_cost += (1 + IS_RAB);
                }
              else
                {
                  emit2 ("push %s", _pairs[id].name);
                  emit2 ("pop iy");
                  regalloc_dry_run_cost += 3;
                }
              if (isUsed)
                _pop (id);
            }
        }
      else if (isUnsplitable (aop))
        {
          emit2 ("push %s", _pairs[getPairId (aop)].name);
          emit2 ("pop %s", _pairs[pairId].name);
          regalloc_dry_run_cost += (pairId == PAIR_IY ? 2 : 1) + (getPairId (aop) == PAIR_IY ? 2 : 1);
        }
      else
        {
          /* The Rabbit has the ld hl, n (sp) and ld hl, n (ix) instructions. */
          int fp_offset = aop->aopu.aop_stk + offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
          int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
          if ((IS_RAB || IS_TLCS90) && aop->size - offset >= 2 && (aop->type == AOP_STK || aop->type == AOP_EXSTK)
              && (pairId == PAIR_HL || pairId == PAIR_IY || pairId == PAIR_DE) && (abs (fp_offset) <= 127 && pairId == PAIR_HL
                  && aop->type == AOP_STK
                  || abs (sp_offset) <= 127))
            {
              if (pairId == PAIR_DE)
                {
                  emit2 ("ex de, hl");
                  regalloc_dry_run_cost += 1;
                }
              if (abs (sp_offset) <= 127)
                emit2 ("ld %s, %d (sp)", pairId == PAIR_IY ? "iy" : "hl", sp_offset);   /* Fetch relative to stack pointer. */
              else
                emit2 ("ld hl, %d (ix)", fp_offset);    /* Fetch relative to frame pointer. */
              regalloc_dry_run_cost += (pairId == PAIR_IY ? 3 : 2);
              if (pairId == PAIR_DE)
                {
                  emit2 ("ex de, hl");
                  regalloc_dry_run_cost += 1;
                }
            }
          /* Operand resides (partially) in the pair */
          else if (!regalloc_dry_run && !strcmp (aopGet (aop, offset + 1, FALSE), _pairs[pairId].l))    // aopGet (aop, offset + 1, FALSE) is problematic: It prevents calculation of exact cost, and results in redundant code being generated. Todo: Exact cost
            {
              _moveA3 (aop, offset);
              if (!regalloc_dry_run)
                emit2 ("ld %s, %s", _pairs[pairId].h, aopGet (aop, offset + 1, FALSE));
              regalloc_dry_run_cost += ld_cost (ASMOP_A, aop);
              emit2 ("ld %s, a", _pairs[pairId].l);
              regalloc_dry_run_cost += 1;
            }
          /* The Rabbit's cast to bool is a cheap way of zeroing h (similar to xor a, a for a for the Z80). */
          else if (pairId == PAIR_HL && IS_RAB && aop->size - offset == 1 && !(aop->type == AOP_REG && (aop->aopu.aop_reg[offset]->rIdx == L_IDX || aop->aopu.aop_reg[offset]->rIdx == H_IDX)))
            {
              emit2 ("bool hl");
              regalloc_dry_run_cost++;
              if (!regalloc_dry_run)
                emit2 ("ld %s, %s", _pairs[pairId].l, aopGet (aop, offset, FALSE));
              regalloc_dry_run_cost += ld_cost (ASMOP_L, aop);
            }
          else
            {
              if (!aopInReg (aop, offset, _pairs[pairId].l_idx))
                {
                   if (!regalloc_dry_run)
                     emit2 ("ld %s, %s", _pairs[pairId].l, aopGet (aop, offset, FALSE));
                   regalloc_dry_run_cost += ld_cost (ASMOP_L, aop);
                }
              if (!aopInReg (aop, offset + 1, _pairs[pairId].h_idx))
                {
                   if (!regalloc_dry_run)
                     emit2 ("ld %s, %s", _pairs[pairId].h, aopGet (aop, offset + 1, FALSE));
                   regalloc_dry_run_cost += ld_cost (ASMOP_H, aop);
                }
            }
        }
      /* PENDING: check? */
      spillPair (pairId);
    }
}

static void
fetchPair (PAIR_ID pairId, asmop *aop)
{
  fetchPairLong (pairId, aop, NULL, 0);
}

static void
setupPairFromSP (PAIR_ID id, int offset)
{
  wassertl (id == PAIR_HL || id == PAIR_DE || id == PAIR_IY, "Setup relative to SP only implemented for HL, DE, IY");

  if (_G.preserveCarry)
    {
      _push (PAIR_AF);
      regalloc_dry_run_cost++;
      offset += 2;
    }

  if (id == PAIR_DE && !IS_GB) // TODO: Could hl be in use for gbz80, so it needs to be saved and restored?
    {
      emit2 ("ex de, hl");
      regalloc_dry_run_cost++;
    }

  if (offset < INT8MIN || offset > INT8MAX || id == PAIR_IY)
    {
      struct dbuf_s dbuf;
      PAIR_ID lid = (id == PAIR_DE) ? PAIR_HL : id;
      dbuf_init (&dbuf, sizeof(int) * 3 + 1);
      dbuf_printf (&dbuf, "%d", offset);
      emit2 ("ld %s, !hashedstr", _pairs[lid].name, dbuf_c_str (&dbuf));
      dbuf_destroy (&dbuf);
      emit2 ("add %s, sp", _pairs[lid].name);
      regalloc_dry_run_cost += 4 + (id == PAIR_IY) * 2;
    }
  else
    {
      wassert (id == PAIR_DE || id == PAIR_HL);
      emit2 ("!ldahlsp", offset);
      regalloc_dry_run_cost += 4 - IS_GB * 2;
    }

  if (id == PAIR_DE && !IS_GB)
    {
      emit2 ("ex de, hl");
      regalloc_dry_run_cost++;
    }
  else if (id == PAIR_DE)
    {
      genMovePairPair (PAIR_HL, PAIR_DE);
      spillPair (PAIR_HL);
    }

  if (_G.preserveCarry)
    {
      _pop (PAIR_AF);
      regalloc_dry_run_cost++;
      offset -= 2;
    }
}

static void
shiftIntoPair (PAIR_ID id, asmop *aop);

/*-----------------------------------------------------------------*/
/* pointPairToAop() make a register pair point to a byte of an aop */
/*-----------------------------------------------------------------*/
static void pointPairToAop (PAIR_ID pairId, const asmop *aop, int offset)
{
  switch (aop->type)
    {
    case AOP_EXSTK:
      wassertl (!IS_GB, "The GBZ80 doesn't have an extended stack");

    case AOP_STK:
      ; int abso = aop->aopu.aop_stk + offset + _G.stack.offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);

      if ((_G.pairs[pairId].last_type == AOP_STK || _G.pairs[pairId].last_type == AOP_EXSTK) && abs (_G.pairs[pairId].offset - abso) < 3)
        adjustPair (_pairs[pairId].name, &_G.pairs[pairId].offset, abso);
      else
        setupPairFromSP (pairId, abso + _G.stack.pushed);

      _G.pairs[pairId].offset = abso;

      break;

    case AOP_HL: // Legacy.
      fetchLitPair (pairId, (asmop *) aop, offset);
      _G.pairs[pairId].offset = offset;
      break;

    case AOP_PAIRPTR:
      wassert (!offset);
      
      shiftIntoPair (pairId, (asmop *) aop); // Legacy. Todo eliminate uses of shiftIntoPair() ?

      break;

    default:
      wassertl (0, "Unsupported aop type for pointPairToAop()");
    }

  _G.pairs[pairId].last_type = aop->type;
}

// Weird function. Sometimes offset is used, sometimes not.
// Callers rely on that behaviour. Uses of this should be replaced
// by pointPairToAop() above after the 3.7.0 release.
static void
setupPair (PAIR_ID pairId, asmop *aop, int offset)
{
  switch (aop->type)
    {
    case AOP_IY:
      wassertl (pairId == PAIR_IY || pairId == PAIR_HL, "AOP_IY must be in IY or HL");
      fetchLitPair (pairId, aop, 0);
      break;

    case AOP_HL:
      wassertl (pairId == PAIR_HL, "AOP_HL must be in HL");

      fetchLitPair (pairId, aop, offset);
      _G.pairs[pairId].offset = offset;
      break;

    case AOP_EXSTK:
      wassertl (!IS_GB, "The GBZ80 doesn't have an extended stack");
      wassertl (pairId == PAIR_IY || pairId == PAIR_HL, "The Z80 extended stack must be in IY or HL");

      {
        int offset = aop->aopu.aop_stk + _G.stack.offset;

        if (aop->aopu.aop_stk >= 0)
          offset += _G.stack.param_offset;

        if (_G.pairs[pairId].last_type == aop->type && abs(_G.pairs[pairId].offset - offset) <= 3)
          adjustPair (_pairs[pairId].name, &_G.pairs[pairId].offset, offset);
        else
          {
            struct dbuf_s dbuf;

            /* PENDING: Do this better. */
            if (_G.preserveCarry)
              _push (PAIR_AF);
            dbuf_init (&dbuf, 128);
            dbuf_printf (&dbuf, "%d", offset + _G.stack.pushed);
            emit2 ("ld %s, !hashedstr", _pairs[pairId].name, dbuf_c_str (&dbuf));
            dbuf_destroy (&dbuf);
            emit2 ("add %s, sp", _pairs[pairId].name);
            _G.pairs[pairId].last_type = aop->type;
            _G.pairs[pairId].offset = offset;
            if (_G.preserveCarry)
              _pop (PAIR_AF);
          }
      }
      break;

    case AOP_STK:
    {
      /* Doesnt include _G.stack.pushed */
      int abso = aop->aopu.aop_stk + offset + _G.stack.offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);

      assert (pairId == PAIR_HL);
      /* In some cases we can still inc or dec hl */
      if (_G.pairs[pairId].last_type == AOP_STK && abs (_G.pairs[pairId].offset - abso) < 3)
        {
          adjustPair (_pairs[pairId].name, &_G.pairs[pairId].offset, abso);
        }
      else
        {
          setupPairFromSP (PAIR_HL, abso + _G.stack.pushed);
        }
      _G.pairs[pairId].offset = abso;
      break;
    }

    case AOP_PAIRPTR:
      if (pairId != aop->aopu.aop_pairId)
        genMovePairPair (aop->aopu.aop_pairId, pairId);
      adjustPair (_pairs[pairId].name, &_G.pairs[pairId].offset, offset);
      break;

    default:
      wassert (0);
    }
  _G.pairs[pairId].last_type = aop->type;
}

static void
emitLabelSpill (symbol *tlbl)
{
  emitLabel (tlbl);
  spillCached ();
}

/*-----------------------------------------------------------------*/
/* aopGet - for fetching value of the aop                          */
/*-----------------------------------------------------------------*/
static const char *
aopGet (asmop *aop, int offset, bool bit16)
{
  static struct dbuf_s dbuf = { 0 };

  wassert_bt (!regalloc_dry_run);

  if (dbuf_is_initialized (&dbuf))
    {
      /* reuse the dynamically allocated buffer */
      dbuf_set_length (&dbuf, 0);
    }
  else
    {
      /* first time: initialize the dynamically allocated buffer */
      dbuf_init (&dbuf, 128);
    }

  /* offset is greater than size then zero */
  /* PENDING: this seems a bit screwed in some pointer cases. */
  if (offset > (aop->size - 1) && aop->type != AOP_LIT)
    {
      dbuf_tprintf (&dbuf, "!zero");
    }
  else
    {
      /* depending on type */
      switch (aop->type)
        {
        case AOP_DUMMY:
          dbuf_append_char (&dbuf, 'a');
          break;

        case AOP_IMMD:
          /* PENDING: re-target */
          if (bit16)
            dbuf_tprintf (&dbuf, "!immedword", aop->aopu.aop_immd);
          else
            {
              switch (offset)
                {
                case 2:
                  // dbuf_tprintf (&dbuf, "!bankimmeds", aop->aopu.aop_immd); Bank support not fully implemented yet.
                  dbuf_tprintf (&dbuf, "#0x00");
                  break;

                case 1:
                  dbuf_tprintf (&dbuf, "!msbimmeds", aop->aopu.aop_immd);
                  break;

                case 0:
                  dbuf_tprintf (&dbuf, "!lsbimmeds", aop->aopu.aop_immd);
                  break;

                default:
                  dbuf_tprintf (&dbuf, "#0x00");
                }
            }
          break;

        case AOP_DIR:
          wassert (IS_GB);
          emit2 ("ld a, (%s+%d)", aop->aopu.aop_dir, offset);
          regalloc_dry_run_cost += 3;
          dbuf_append_char (&dbuf, 'a');
          break;

        case AOP_SFR:
          wassertl (!IS_TLCS90, "TLCS-90 does not have a separate I/O space");
          if (IS_GB)
            {
              emit2 ("ldh a, (%s+%d)", aop->aopu.aop_dir, offset);
              regalloc_dry_run_cost += 2;
              dbuf_append_char (&dbuf, 'a');
            }
          else if (IS_RAB)
            {
              emit2 ("ioi");
              emit2 ("ld a, (%s)", aop->aopu.aop_dir);
              emit2 ("nop");    /* Workaround for Rabbit 2000 hardware bug. see TN302 for details. */
              dbuf_append_char (&dbuf, 'a');
            }
          else
            {
              /*.p.t.20030716 handling for i/o port read access for Z80 */
              if (aop->paged)
                {
                  /* banked mode */
                  /* reg A goes to address bits 15-8 during "in a,(x)" instruction */
                  emit2 ("ld a, !msbimmeds", aop->aopu.aop_dir);
                  emit2 ("in a, (!lsbimmeds)", aop->aopu.aop_dir);
                }
              else if (z80_opts.port_mode == 180)
                {
                  /* z180 in0/out0 mode */
                  emit2 ("in0 a, (%s)", aop->aopu.aop_dir);
                }
              else
                {
                  /* 8 bit mode */
                  emit2 ("in a, (%s)", aop->aopu.aop_dir);
                }

              dbuf_append_char (&dbuf, 'a');
            }
          break;

        case AOP_REG:
          dbuf_append_str (&dbuf, aop->aopu.aop_reg[offset]->name);
          break;

        case AOP_HL:
          setupPair (PAIR_HL, aop, offset);
          dbuf_tprintf (&dbuf, "!*hl");
          break;

        case AOP_IY:
          wassert (!IS_GB);
          setupPair (PAIR_IY, aop, offset);
          dbuf_tprintf (&dbuf, "!*iyx", offset);
          break;

        case AOP_EXSTK:
          if (!IY_RESERVED)
            {
              wassert (!IS_GB);
              setupPair (PAIR_IY, aop, offset);
              dbuf_tprintf (&dbuf, "!*iyx", offset);
              break;
            }

        case AOP_STK:
          if (IS_GB || aop->type == AOP_EXSTK)
            {
              pointPairToAop (PAIR_HL, aop, offset);
              dbuf_tprintf (&dbuf, "!*hl");
            }
          else if (_G.omitFramePtr)
            {
              if (aop->aopu.aop_stk >= 0)
                offset += _G.stack.param_offset;
              setupPair (PAIR_IX, aop, offset);
              dbuf_tprintf (&dbuf, "!*ixx", offset);
            }
          else
            {
              if (aop->aopu.aop_stk >= 0)
                offset += _G.stack.param_offset;
              dbuf_tprintf (&dbuf, "!*ixx", aop->aopu.aop_stk + offset);
            }
          break;

        case AOP_CRY:
          wassertl (0, "Tried to fetch from a bit variable");
          break;

        case AOP_LIT:
          dbuf_append_str (&dbuf, aopLiteral (aop->aopu.aop_lit, offset));
          break;

        case AOP_STR:
          aop->coff = offset;
          dbuf_append_str (&dbuf, aop->aopu.aop_str[offset]);
          break;

        case AOP_PAIRPTR:
          setupPair (aop->aopu.aop_pairId, aop, offset);
          if (aop->aopu.aop_pairId == PAIR_IX)
            dbuf_tprintf (&dbuf, "!*ixx", offset);
          else if (aop->aopu.aop_pairId == PAIR_IY)
            dbuf_tprintf (&dbuf, "!*iyx", offset);
          else
            dbuf_printf (&dbuf, "(%s)", _pairs[aop->aopu.aop_pairId].name);
          break;

        default:
          dbuf_destroy (&dbuf);
          fprintf (stderr, "aop->type: %d\n", aop->type);
          wassertl (0, "aopGet got unsupported aop->type");
          exit (0);
        }
    }
  return dbuf_c_str (&dbuf);
}

static bool
isRegString (const char *s)
{
  if (!strcmp (s, "b") || !strcmp (s, "c") || !strcmp (s, "d") || !strcmp (s, "e") ||
      !strcmp (s, "a") || !strcmp (s, "h") || !strcmp (s, "l"))
    return TRUE;
  return FALSE;
}

static bool
isConstantString (const char *s)
{
  /* This is a bit of a hack... */
  return (*s == '#' || *s == '$');
}

#define AOP(op) op->aop
#define AOP_TYPE(op) AOP(op)->type
#define AOP_SIZE(op) AOP(op)->size
#define AOP_NEEDSACC(x) (AOP(x) && ((AOP_TYPE(x) == AOP_CRY) || (AOP_TYPE(x) == AOP_SFR)))
#define AOP_IS_PAIRPTR(x, p) (AOP_TYPE (x) == AOP_PAIRPTR && AOP (x)->aopu.aop_pairId == p)

static bool
canAssignToPtr (const char *s)
{
  if (isRegString (s))
    return TRUE;
  if (isConstantString (s))
    return TRUE;
  return FALSE;
}

static bool
canAssignToPtr3 (const asmop *aop)
{
  if (aop->type == AOP_REG)
    return (TRUE);
  if (aop->type == AOP_IMMD || aop->type == AOP_LIT)
    return (TRUE);
  return (FALSE);
}

/*-----------------------------------------------------------------*/
/* aopPut - puts a string for a aop                                */
/*-----------------------------------------------------------------*/
static void
aopPut (asmop *aop, const char *s, int offset)
{
  struct dbuf_s dbuf;

  wassert (!regalloc_dry_run);

  if (aop->size && offset > (aop->size - 1))
    {
      werror_bt (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut got offset > aop->size");
      exit (0);
    }

  // PENDING
  dbuf_init (&dbuf, 128);
  dbuf_tprintf (&dbuf, s);
  s = dbuf_c_str (&dbuf);

  /* will assign value to value */
  /* depending on where it is of course */
  switch (aop->type)
    {
    case AOP_DUMMY:
      _moveA (s);               /* in case s is volatile */
      break;

    case AOP_DIR:
      /* Direct.  Hmmm. */
      wassert (IS_GB);
      if (strcmp (s, "a"))
        emit2 ("ld a, %s", s);
      emit2 ("ld (%s+%d),a", aop->aopu.aop_dir, offset);
      break;

    case AOP_SFR:
      wassertl (!IS_TLCS90, "TLCS-90 does not have a separate I/O space");
      if (IS_GB)
        {
          //  wassert (IS_GB);
          if (strcmp (s, "a"))
            emit2 ("ld a, %s", s);
          emit2 ("ldh (%s+%d),a", aop->aopu.aop_dir, offset);
        }
      else if (IS_RAB)
        {
          if (strcmp (s, "a"))
            emit2 ("ld a, %s", s);

          /* LM 20110928: Need to fix to emit either "ioi" or "ioe"
           * (for internal vs. external I/O space
           */
          emit2 ("ioi");
          emit2 ("ld (%s),a", aop->aopu.aop_dir);
          emit2 ("nop");        /* Workaround for Rabbit 2000 hardware bug. see TN302 for details. */
        }
      else
        {
          /*.p.t.20030716 handling for i/o port read access for Z80 */
          if (aop->paged)
            {
              /* banked mode */
              if (aop->bcInUse)
                emit2 ("push bc");

              if (strlen (s) != 1 || (s[0] != 'a' && s[0] != 'd' && s[0] != 'e' && s[0] != 'h' && s[0] != 'l'))
                {
                  emit2 ("ld a, %s", s);
                  s = "a";
                }

              emit2 ("ld bc, !hashedstr", aop->aopu.aop_dir);
              emit2 ("out (c),%s", s);

              if (aop->bcInUse)
                emit2 ("pop bc");
              else
                spillPair (PAIR_BC);
            }
          else if (z80_opts.port_mode == 180)
            {
              /* z180 in0/out0 mode */
              emit2 ("ld a, %s", s);
              emit2 ("out0 (%s), a", aop->aopu.aop_dir);
            }
          else
            {
              /* 8 bit mode */
              if (strcmp (s, "a"))
                emit2 ("ld a, %s", s);
              emit2 ("out (%s), a", aop->aopu.aop_dir);
            }
        }
      break;

    case AOP_REG:
      if (!strcmp (aop->aopu.aop_reg[offset]->name, s))
        ;
      else if (!strcmp (s, "!*hl"))
        emit2 ("ld %s,!*hl", aop->aopu.aop_reg[offset]->name);
      else
        emit2 ("ld %s, %s", aop->aopu.aop_reg[offset]->name, s);
      spillPairReg (aop->aopu.aop_reg[offset]->name);
      break;

    case AOP_IY:
      wassert (!IS_GB);
      if (!canAssignToPtr (s))
        {
          emit2 ("ld a, %s", s);
          setupPair (PAIR_IY, aop, offset);
          emit2 ("ld !*iyx, a", offset);
        }
      else
        {
          setupPair (PAIR_IY, aop, offset);
          emit2 ("ld !*iyx, %s", offset, s);
        }
      break;

    case AOP_HL:
      //wassert (IS_GB);
      /* PENDING: for re-target */
      if (!strcmp (s, "!*hl") || !strcmp (s, "(hl)") || !strcmp (s, "[hl]"))
        {
          emit2 ("ld a, !*hl");
          s = "a";
        }
      else if (strstr (s, "(ix)") || strstr (s, "(iy)"))
        {
          emit2 ("ld a, %s", s);
          s = "a";
        }
      setupPair (PAIR_HL, aop, offset);

      emit2 ("ld !*hl, %s", s);
      break;

    case AOP_EXSTK:
      if(!IY_RESERVED)
        {
          wassert (!IS_GB);
          if (!canAssignToPtr (s))
            {
              emit2 ("ld a, %s", s);
              setupPair (PAIR_IY, aop, offset);
              emit2 ("ld !*iyx, a", offset);
            }
          else
            {
              setupPair (PAIR_IY, aop, offset);
              emit2 ("ld !*iyx, %s", offset, s);
            }
          break;
       }

    case AOP_STK:
      if (IS_GB || aop->type == AOP_EXSTK)
        {
          /* PENDING: re-target */
          if (!strcmp (s, "!*hl") || !strcmp (s, "(hl)") || !strcmp (s, "[hl]"))
            {
              emit2 ("ld a, !*hl");
              s = "a";
            }
          pointPairToAop (PAIR_HL, aop, offset);
          if (!canAssignToPtr (s))
            {
              emit2 ("ld a, %s", s);
              emit2 ("ld !*hl, a");
            }
          else
            emit2 ("ld !*hl, %s", s);
        }
      else
        {
          if (aop->aopu.aop_stk >= 0)
            offset += _G.stack.param_offset;
          if (!canAssignToPtr (s))
            {
              emit2 ("ld a, %s", s);
              emit2 ("ld !*ixx, a", aop->aopu.aop_stk + offset);
            }
          else
            {
              emit2 ("ld !*ixx, %s", aop->aopu.aop_stk + offset, s);
            }
        }
      break;

    case AOP_CRY:
      /* if bit variable */
      if (!aop->aopu.aop_dir)
        {
          emit2 ("ld a, !zero");
          emit2 ("rla");
        }
      else
        {
          /* In bit space but not in C - cant happen */
          wassertl (0, "Tried to write into a bit variable");
        }
      break;

    case AOP_STR:
      aop->coff = offset;
      if (strcmp (aop->aopu.aop_str[offset], s))
        {
          emit2 ("ld %s, %s", aop->aopu.aop_str[offset], s);
        }
      spillPairReg (aop->aopu.aop_str[offset]);
      break;

#if 0
    case AOP_ACC:
      aop->coff = offset;
      if (!offset && (strcmp (s, "acc") == 0))
        break;
      if (offset > 0)
        {
          wassertl (0, "Tried to access past the end of A");
        }
      else
        {
          wassert (aop->aopu.aop_str[offset]);
          wassert (s);
          if (strcmp (aop->aopu.aop_str[offset], s))
            {
              emit2 ("ld %s, %s", aop->aopu.aop_str[offset], s);
              spillPairReg (aop->aopu.aop_str[offset]);
            }
        }
      break;
#endif

    case AOP_PAIRPTR:
      setupPair (aop->aopu.aop_pairId, aop, offset);
      if (aop->aopu.aop_pairId == PAIR_IX)
        emit2 ("ld !*ixx, %s", 0, s);
      else if (aop->aopu.aop_pairId == PAIR_IY)
        emit2 ("ld !*iyx, %s", 0, s);
      else
        emit2 ("ld (%s), %s", _pairs[aop->aopu.aop_pairId].name, s);
      break;

    default:
      dbuf_destroy (&dbuf); fprintf (stderr, "AOP_DIR: %d\n",AOP_DIR);
      fprintf (stderr, "aop->type: %d\n", aop->type);
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut got unsupported aop->type");
      exit (0);
    }
  dbuf_destroy (&dbuf);
}

// Move, but try not to. Cannot use xor to zero, since xor resets the carry flag.
static void
cheapMove (asmop *to, int to_offset, asmop *from, int from_offset, bool a_dead)
{
  if (aopInReg (to, to_offset, A_IDX))
    a_dead = true;

  if (to->type == AOP_REG && from->type == AOP_REG)
    {
      if (to->aopu.aop_reg[to_offset] == from->aopu.aop_reg[from_offset])
        return;
      bool from_index = aopInReg (from, from_offset, IYL_IDX) || aopInReg (from, from_offset, IYH_IDX);
      bool to_index = aopInReg (to, to_offset, IYL_IDX)  || aopInReg (to, to_offset, IYH_IDX);
      bool index = to_index || from_index;
      if (!index || IS_EZ80_Z80)
        {
          bool a = aopInReg (to, to_offset, A_IDX) || aopInReg (from, from_offset, A_IDX);
          if (!regalloc_dry_run)
            aopPut (to, aopGet (from, from_offset, false), to_offset);
          regalloc_dry_run_cost += 1 + (IS_TLCS90 + !a) + index;
          return;
        }
      if (aopInReg (from, from_offset, IYL_IDX) && !to_index && a_dead)
        {
          _push(PAIR_IY);
          _pop (PAIR_AF);
          cheapMove (to, to_offset, ASMOP_A, 0, true);
          return;
        }
      if (from_index && !to_index && _G.stack.pushed + _G.stack.offset + 2 <= 127 && !_G.omitFramePtr)
        {
          _push(PAIR_IY);
          if (!regalloc_dry_run)
            emit2 ("ld %s, %d (ix)", aopGet (to, to_offset, false), _G.stack.pushed + _G.stack.offset);
          regalloc_dry_run_cost += 3;
          _pop(PAIR_IY);
          return;
        }

      // Can't do it (todo: implement something there - will be expensive though, probably at least 7B of code).
      regalloc_dry_run_cost += 100;
      wassert (regalloc_dry_run);
    }

  // Try to push to avoid setting up temporary stack pointer in hl or iy.
  if ((to->type == AOP_STK || to->type == AOP_EXSTK) && _G.omitFramePtr &&
    (aopInReg (to, to_offset, A_IDX) || aopInReg (to, to_offset, B_IDX) || aopInReg (to, to_offset, D_IDX) || aopInReg (to, to_offset, H_IDX) || aopInReg (to, to_offset, IYH_IDX)))
    {
      int fp_offset = to->aopu.aop_stk + to_offset + (to->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;

      if (!sp_offset)
        {
          emit2 ("inc sp");
          emit2 ("push %s", aopInReg (from, from_offset, A_IDX) ? "af" : (aopInReg (from, from_offset, B_IDX) ? "bc" : (aopInReg (from, from_offset, D_IDX) ? "de" : (aopInReg (from, from_offset, H_IDX) ? "hl" : "iy"))));
          emit2 ("inc sp");
          regalloc_dry_run_cost += 3 + aopInReg (from, from_offset, IYH_IDX);
          return;
        }
    }

  if (aopInReg (from, from_offset, A_IDX) && to->type == AOP_IY)
    {
      emit2 ("ld (%s+%d), a", to->aopu.aop_dir, to_offset);
      regalloc_dry_run_cost += 3;
    }
  else if (!aopInReg (to, to_offset, A_IDX) && !aopInReg (from, from_offset, A_IDX) && (from->type == AOP_DIR || from->type == AOP_SFR || to->type == AOP_IY && from->type == AOP_EXSTK)) // Go through a.
    {
      if (!a_dead)
        _push (PAIR_AF);

      cheapMove (ASMOP_A, 0, from, from_offset, true);
      cheapMove (to, to_offset, ASMOP_A, 0, true);

      if (!a_dead)
        _pop (PAIR_AF);
    }
  else
    {
      if (!regalloc_dry_run)
        aopPut (to, aopGet (from, from_offset, false), to_offset);

      regalloc_dry_run_cost += ld_cost (to, from_offset < from->size ? from : ASMOP_ZERO);
    }
}

static void
commitPair (asmop *aop, PAIR_ID id, const iCode *ic, bool dont_destroy)
{
  int fp_offset = aop->aopu.aop_stk + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
  int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;

  if (getPairId (aop) == id)
    return;

  /* Stack positions will change, so do not assume this is possible in the cost function. */
  if (!regalloc_dry_run && !IS_GB && (aop->type == AOP_STK || aop->type == AOP_EXSTK) && !sp_offset
      && ((!IS_RAB && id == PAIR_HL) || id == PAIR_IY) && !dont_destroy)
    {
      emit2 ("ex (sp), %s", _pairs[id].name);
      regalloc_dry_run_cost += ((id == PAIR_IY || IS_RAB) ? 2 : 1);
      spillPair (id);
    }
  else if ((IS_RAB || IS_TLCS90) && (aop->type == AOP_STK || aop->type == AOP_EXSTK) && (id == PAIR_HL || id == PAIR_IY) &&
           (id == PAIR_HL && abs (fp_offset) <= 127 && aop->type == AOP_STK || abs (sp_offset) <= 127))
    {
      if (abs (sp_offset) <= 127)
        emit2 ("ld %d (sp), %s", sp_offset, id == PAIR_IY ? "iy" : "hl");       /* Relative to stack pointer. */
      else
        emit2 ("ld %d (ix), hl", fp_offset);    /* Relative to frame pointer. */
      regalloc_dry_run_cost += (id == PAIR_HL ? 2 : 3);
    }
  else if (IS_EZ80_Z80 && aop->type == AOP_STK)
    {
      emit2 ("ld %d (ix), %s", fp_offset, _pairs[id].name);
      regalloc_dry_run_cost += 3;
    }
  else if (!regalloc_dry_run && (aop->type == AOP_STK || aop->type == AOP_EXSTK) && !sp_offset)
    {
      emit2 ("inc sp");
      emit2 ("inc sp");
      emit2 ("push %s", _pairs[id].name);
      regalloc_dry_run_cost += (id == PAIR_IY ? 5 : 4);
    }

  /* PENDING: Verify this. */
  else if (id == PAIR_HL && requiresHL (aop) && (IS_GB || IY_RESERVED && aop->type != AOP_HL && aop->type != AOP_IY))
    {
      if (bitVectBitValue (ic->rSurv, D_IDX))
        _push (PAIR_DE);
      if (!regalloc_dry_run)
        {
          emit2 ("ld a, l");
          emit2 ("ld d, h");
          aopPut (aop, "a", 0);
          aopPut (aop, "d", 1);
        }
      regalloc_dry_run_cost += (2 + ld_cost (aop, ASMOP_A) + ld_cost (aop, ASMOP_D));
      if (bitVectBitValue (ic->rSurv, D_IDX))
        _pop (PAIR_DE);
    }
  else
    {
      /* Special cases */
      if ((aop->type == AOP_IY || aop->type == AOP_HL) && !IS_GB && aop->size == 2)
        {
          if (!regalloc_dry_run)
            {
              emit2 ("ld (%s), %s", aopGetLitWordLong (aop, 0, FALSE), _pairs[id].name);
            }
          regalloc_dry_run_cost += (id == PAIR_HL ? 3 : 4);
        }
      else
        {
          switch (id)
            {
            case PAIR_BC:
              cheapMove (aop, 0, ASMOP_C, 0, true);
              cheapMove (aop, 1, ASMOP_B, 0, true);
              break;
            case PAIR_DE:
              if (!IS_GB && aop->type == AOP_REG && aop->aopu.aop_reg[0]->rIdx == L_IDX && aop->aopu.aop_reg[1]->rIdx == H_IDX && !dont_destroy)
                {
                  emit2 ("ex de, hl");
                  regalloc_dry_run_cost++;
                }
              else
                {
                  cheapMove (aop, 0, ASMOP_E, 0, true);
                  cheapMove (aop, 1, ASMOP_D, 0, true);
                }
              break;
            case PAIR_HL:
              if (aop->type == AOP_REG && aop->aopu.aop_reg[0]->rIdx == H_IDX && aop->aopu.aop_reg[1]->rIdx == L_IDX)
                {
                  cheapMove (ASMOP_A, 0, ASMOP_L, 0, true);
                  cheapMove (aop, 1, ASMOP_H, 0, true);
                  cheapMove (aop, 0, ASMOP_A, 0, true);
                }
              else if (aop->type == AOP_REG && aop->aopu.aop_reg[0]->rIdx == H_IDX)     // Do not overwrite upper byte.
                {
                  cheapMove (aop, 1, ASMOP_H, 0, true);
                  cheapMove (aop, 0, ASMOP_L, 0, true);
                }
              else if (!IS_GB && aop->type == AOP_REG && aop->aopu.aop_reg[0]->rIdx == E_IDX && aop->aopu.aop_reg[1]->rIdx == D_IDX && !dont_destroy)
                {
                  emit2 ("ex de, hl");
                  regalloc_dry_run_cost++;
                }
              else
                {
                  cheapMove (aop, 0, ASMOP_L, 0, true);
                  cheapMove (aop, 1, ASMOP_H, 0, true);
                }
              break;
            case PAIR_IY:
              cheapMove (aop, 0, ASMOP_IYL, 0, true);
              cheapMove (aop, 1, ASMOP_IYH, 0, true);
              break;
            default:
              wassertl (0, "Unknown pair id in commitPair()");
              fprintf (stderr, "pair %s\n", _pairs[id].name);
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* genCopyStack - Copy the value - stack to stack only             */
/*-----------------------------------------------------------------*/
static void
genCopyStack (asmop *result, int roffset, asmop *source, int soffset, int n, bool *assigned, int *size, bool a_free, bool hl_free, bool really_do_it_now)
{
  for (int i = 0; i < n;)
    {
      if (assigned[i])
        {
          i++;
          continue;
        }

      if (!aopOnStack (result, roffset + i, 1) || !aopOnStack (source, soffset + i, 1))
        {
          i++;
          continue;
        }

      if (i + 1 < n && !assigned[i + 1] && hl_free && (IS_RAB || IS_EZ80_Z80 || IS_TLCS90))
        {
          if (!regalloc_dry_run)
            {
              emit2 ("ld hl, %s", aopGet (source, soffset + i, false));
              emit2 ("ld %s, hl", aopGet (result, roffset + i, false));
            }
          cost2 (6 - 2 * IS_RAB, 0, 0, 22, 0, 21, 10);

          spillPair (PAIR_HL);

          assigned[i] = true;
          assigned[i + 1] = true;
          (*size) -= 2;
          i += 2;
          continue;
        }

      if (a_free || really_do_it_now)
        {
          cheapMove (result, roffset + i, source, soffset + i, a_free);
          assigned[i] = true;
          (*size)--;
          i++;
          continue;
        }

       i++;
    }

  wassertl_bt (*size >= 0, "genCopyStack() copied more than there is to be copied.");
}

/*-----------------------------------------------------------------*/
/* genCopy - Copy the value from one reg/stk asmop to another      */
/*-----------------------------------------------------------------*/
static void
genCopy (asmop *result, int roffset, asmop *source, int soffset, int sizex, bool a_dead, bool hl_dead)
{
  int regsize, size, n = (sizex < source->size - soffset) ? sizex : (source->size - soffset);
  bool assigned[8] = {false, false, false, false, false, false, false, false};
  bool a_free, hl_free;
  int cached_byte = -1;
  bool pushed_a = false;

  wassertl_bt (n <= 8, "Invalid size for genCopy().");
  wassertl_bt (aopRS (source), "Invalid source type.");
  wassertl_bt (aopRS (result), "Invalid result type.");

  size = n;
  regsize = 0;
  for (int i = 0; i < n; i++)
    regsize += (source->type == AOP_REG);

  // Do nothing for coalesced bytes.
  for (int i = 0; i < n; i++)
    if (result->type == AOP_REG && source->type == AOP_REG && result->aopu.aop_reg[roffset + i] == source->aopu.aop_reg[soffset + i])
      {
        assigned[i] = true;
        regsize--;
        size--;
      }

  // Move everything from registers to the stack.
  for (int i = 0; i < n;)
    {
      if (i + 1 < n && result->type == AOP_STK &&
        (aopInReg (source, soffset + i, HL_IDX) && IS_RAB ||
        (aopInReg (source, soffset + i, BC_IDX) || aopInReg (source, soffset + i, DE_IDX) || aopInReg (source, soffset + i, HL_IDX) || aopInReg (source, soffset + i, IY_IDX)) && (IS_EZ80_Z80 || IS_TLCS90)))
        {
          if (!regalloc_dry_run)
            emit2 ("ld %s, %s", aopGet (result, roffset + i, false), _pairs[getPairId_o (source, soffset + i)].name);
          cost2 (3 - IS_RAB, 0, 0, 11, 0, 12, 5);
          assigned[i] = true;
          assigned[i + 1] = true;
          regsize -= 2;
          size -= 2;
          i += 2;
        }
      else if (aopRS (source) && !aopOnStack (source, soffset + i, 1) && aopOnStack (result, roffset + i, 1))
        {
          cheapMove (result, roffset + i, source, soffset + i, true);
          assigned[i] = true;
          regsize--;
          size--;
          i++;
        }
      else // This byte is not a register-to-stack copy.
        i++;
    }

  // Copy (stack-to-stack) what we can with whatever free regs we have.
  a_free = a_dead;
  hl_free = hl_dead;
  for (int i = 0; i < n; i++)
    {
      asmop *operand;
      int offset;

      if (!assigned[i])
        {
          operand = source;
          offset = soffset + i;
        }
      else
        {
          operand = result;
          offset = roffset + i;
        }

      if (aopInReg (operand, offset, A_IDX))
        a_free = false;
      else if (aopInReg (operand, offset, L_IDX) || aopInReg (operand, offset, H_IDX))
        hl_free = FALSE;
    }
  genCopyStack (result, roffset, source, soffset, n, assigned, &size, a_free, hl_free, false);

  // Now do the register shuffling.

  // Try to use:
  // TLCS-90 ld rr, rr
  // eZ80 lea rr, iy.
  // All: push rr / pop iy
  // All: push iy / pop rr
  for (int i = 0; i + 1 < n; i++)
    {
      if (assigned[i] || assigned[i + 1])
        continue;

      for (int j = 0; j + 1 < n; j++)
        {
          if (!assigned[j] && i != j && i + 1 != j && !aopOnStack(result, roffset + i, 2) && !aopOnStack(source, soffset + i, 1) &&
            (result->aopu.aop_reg[roffset + i] == source->aopu.aop_reg[soffset + j] || result->aopu.aop_reg[roffset + i + 1] == source->aopu.aop_reg[soffset + j]))
            goto skip_byte_push_iy; // We can't write this one without overwriting the source.
        }

      if (IS_TLCS90 && getPairId_o (result, roffset + i) != PAIR_INVALID && getPairId_o (source, soffset + i) != PAIR_INVALID)
        {
          emit2 ("ld %s, %s", _pairs[getPairId_o (result, roffset + i)].name, _pairs[getPairId_o (source, soffset + i)].name);
          regalloc_dry_run_cost += 1 + (!aopInReg (result, roffset + i, HL_IDX) && !aopInReg (source, soffset + i, HL_IDX));
        }
      else if (IS_EZ80_Z80 && getPairId_o (result, roffset + i) != PAIR_INVALID && aopInReg (source, soffset + i, IY_IDX))
        {
          emit2 ("lea %s, iy, #0", _pairs[getPairId_o (result, roffset + i)].name);
          regalloc_dry_run_cost += 3;
        }
      else if (aopInReg (result, roffset + i, IY_IDX) && getPairId_o (source, soffset + i) != PAIR_INVALID ||
        getPairId_o (result, roffset + i) != PAIR_INVALID && aopInReg (source, soffset + i, IY_IDX))
        {
          emit2 ("push %s", _pairs[getPairId_o (source, soffset + i)].name);
          emit2 ("pop %s", _pairs[getPairId_o (result, roffset + i)].name);
          regalloc_dry_run_cost += 3;
        }
      else
        continue;

      regsize -= 2;
      size -= 2;
      assigned[i] = true;
      assigned[i + 1] = true;

skip_byte_push_iy:
        ;
    }

  // Try to use ex de, hl. TODO: Also do so when only some bytes are used, while others are dead (useful e.g. for emulating ld de, hl or ld hl, de).
  if (regsize >= 4)
    {
      int ex[4] = {-2, -2, -2, -2};

      // Find L and check that it is exchanged with E, find H and check that it is exchanged with D.
      for (int i = 0; i < n; i++)
        {
          if (!assigned[i] && aopInReg (result, roffset + i, L_IDX) && aopInReg (source, soffset + i, E_IDX))
            ex[0] = i;
          if (!assigned[i] && aopInReg (result, roffset + i, E_IDX) && aopInReg (source, soffset + i, L_IDX))
            ex[1] = i;
          if (!assigned[i] && aopInReg (result, roffset + i, H_IDX) && aopInReg (source, soffset + i, D_IDX))
            ex[2] = i;
          if (!assigned[i] && aopInReg (result, roffset + i, D_IDX) && aopInReg (source, soffset + i, H_IDX))
            ex[3] = i;
        }

      int exsum = (ex[0] >= 0) + (ex[1] >= 0) + (ex[2] >= 0) + (ex[3] >= 0);

      if (exsum == 4)
        {
          emit2 ("ex de, hl");
          regalloc_dry_run_cost += 1; // TODO: Use cost() to enable better optimization for speed.
          if(ex[0] >= 0)
            assigned[ex[0]] = TRUE;
          if(ex[1] >= 0)
            assigned[ex[1]] = TRUE;
          if(ex[2] >= 0)
            assigned[ex[2]] = TRUE;
          if(ex[3] >= 0)
            assigned[ex[3]] = TRUE;
          regsize -= exsum;
          size -= exsum;
        }
    }

  while (regsize && result->type == AOP_REG && source->type == AOP_REG)
    {
      int i;

      // Find lowest byte that can be assigned and needs to be assigned.
      for (i = 0; i < n; i++)
        {
          if (assigned[i])
            continue;

          for (int j = 0; j < n; j++)
            {
              if (!assigned[j] && i != j && result->aopu.aop_reg[roffset + i] == source->aopu.aop_reg[soffset + j])
                goto skip_byte; // We can't write this one without overwriting the source.
            }

          break;                // Found byte that can be written safely.

skip_byte:
          ;
        }

      if (i < n)
        {
          cheapMove (result, roffset + i, source, soffset + i, false);       // We can safely assign a byte.
          regsize--;
          size--;
          assigned[i] = true;
          continue;
        }

      // No byte can be assigned safely (i.e. the assignment is a permutation). Cache one in the accumulator.

      if (cached_byte != -1)
        {
          // Already one cached. Can happen when the assignment is a permutation consisting of multiple cycles.
          cheapMove (result, roffset + cached_byte, ASMOP_A, 0, true);
          cached_byte = -1;
          continue;
        }

      for (i = 0; i < n; i++)
        if (!assigned[i])
          break;

      wassertl_bt (i != n, "genCopy error: Trying to cache non-existant byte in accumulator.");
      if (a_free && !pushed_a)
        {
          _push (PAIR_AF);
          pushed_a = TRUE;
        }
      cheapMove (ASMOP_A, 0, source, soffset + i, true);
      regsize--;
      size--;
      assigned[i] = TRUE;
      cached_byte = i;
    }

  // Copy (stack-to-stack) what we can with whatever free regs we have now.
  a_free = a_dead;
  hl_free = hl_dead;
  for (int i = 0; i < n; i++)
    {
      if (!assigned[i])
        continue;
      if (aopInReg (result, roffset + i, A_IDX))
        a_free = false;
      else if (aopInReg (result, roffset + i, L_IDX) || aopInReg (result, roffset + i, H_IDX))
        hl_free = false;
    }
  genCopyStack (result, roffset, source, soffset, n, assigned, &size, a_free, hl_free, false);

  // Last, move everything from stack to registers.
  for (int i = 0; i < n;)
    {
      if (i + 1 < n && source->type == AOP_STK &&
        (aopInReg (result, roffset + i, HL_IDX) && IS_RAB ||
        (aopInReg (result, roffset + i, BC_IDX) || aopInReg (result, roffset + i, DE_IDX) || aopInReg (result, roffset + i, HL_IDX) || aopInReg (result, roffset + i, IY_IDX)) && (IS_EZ80_Z80 || IS_TLCS90)))
        {
          if (!regalloc_dry_run)
            emit2 ("ld %s, %s", _pairs[getPairId_o (result, roffset + i)].name, aopGet (source, soffset + i, false));
          cost2 (3 - IS_RAB, 0, 0, 11, 0, 9, 5);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;
        }
      else if (aopRS (result) && aopOnStack (source, soffset + i, 1) && !aopOnStack (result, roffset + i, 1))
        {
          cheapMove (result, roffset + i, source, soffset + i, true);
          assigned[i] = true;
          size--;
          i++;
        }
      else // This byte is not a register-to-stack copy.
        i++;
    }

  // Free a reg to copy (stack-to-stack) whatever is left.
  if (size)
    {
      a_free = a_dead && (result->regs[A_IDX] < 0 || result->regs[A_IDX] >= roffset + source->size);
      hl_free = a_dead && (result->regs[L_IDX] < 0 || result->regs[L_IDX] >= roffset + source->size) && (result->regs[H_IDX] < 0 || result->regs[H_IDX] >= roffset + source->size);
      if (!a_free)
        _push (PAIR_AF);
      genCopyStack (result, roffset, source, soffset, n, assigned, &size, true, hl_free, true);
      if (!a_free)
        _pop (PAIR_AF);
    }

  wassertl_bt (size >= 0, "genCopy() copied more than there is to be copied.");

  a_free = a_dead && (result->regs[A_IDX] < 0 || result->regs[A_IDX] >= roffset + source->size);

  // Place leading zeroes.

  // todo

  if (cached_byte != -1)
    cheapMove (result, roffset + cached_byte, ASMOP_A, 0, true);

  if (pushed_a)
    _pop (PAIR_AF);
}

/*-----------------------------------------------------------------*/
/* genMove_o - Copy part of one asmop to another                   */
/*-----------------------------------------------------------------*/
static void
genMove_o (asmop *result, int roffset, asmop *source, int soffset, int size, bool a_dead_global, bool hl_dead_global)
{
  emitDebug ("; genMove_o");

  if ((result->type == AOP_REG || !IS_GB && result->type == AOP_STK) && (source->type == AOP_REG || !IS_GB && source->type == AOP_STK))
    {
      int csize = size > source->size - soffset ? source->size - soffset : size;
      genCopy (result, roffset, source, soffset, csize, a_dead_global, hl_dead_global);
      roffset += csize;
      size -= csize;
      genMove_o (result, roffset, ASMOP_ZERO, 0, size, a_dead_global && result->regs[A_IDX] < roffset, hl_dead_global && result->regs[H_IDX] < roffset && result->regs[L_IDX] < roffset);
      return;
    }

  bool zeroed_a = false;
  long value_hl = -1;
  bool a_dead = a_dead_global;
  bool hl_dead = hl_dead_global;
  for (unsigned int i = 0; i < size;)
    {
      if ((IS_EZ80_Z80 || IS_RAB || IS_TLCS90) && i + 1 < size && result->type == AOP_STK &&
        source->type == AOP_LIT && (value_hl >= 0 && aopIsLitVal (source, soffset + i, 2, value_hl) || hl_dead))
        {
          if (value_hl < 0 || !aopIsLitVal (source, soffset + i, 2, value_hl))
            fetchLitPair (PAIR_HL, source, soffset + i);
          if (!regalloc_dry_run)
            emit2 ("ld %s, hl", aopGet (result, roffset + i, false));
          cost2 (3 - IS_RAB, 0, 0, 11, 0, 12, 5);
          regalloc_dry_run_cost += 3;
          value_hl = ullFromVal (source->aopu.aop_lit) >> ((soffset + i) * 8) & 0xffff;
          i += 2;
          continue;
        }

      // Cache a copy of zero in a.
      if (result->type != AOP_REG && aopIsLitVal (source, soffset + i, 2, 0x0000) && !zeroed_a && a_dead)
        {
          emit3 (A_XOR, ASMOP_A, ASMOP_A);
          regalloc_dry_run_cost += 1;
          zeroed_a = true;
        }

      if (result->type == AOP_HL && a_dead_global && (!hl_dead_global || source->regs[L_IDX] != -1 || source->regs[H_IDX] != -1))
        {
          if (!aopIsLitVal (source, soffset + i, 1, 0x00) || !zeroed_a)
            {
              cheapMove (ASMOP_A, 0, source, soffset + i, true);
              zeroed_a = aopIsLitVal (source, soffset + i, 1, 0x00);
            }
          emit2 ("ld (%s), a", aopGetLitWordLong (result, roffset + i, FALSE));
          regalloc_dry_run_cost += 3;
        }
      else if (aopIsLitVal (source, soffset + i, 1, 0x00) && zeroed_a)
        cheapMove (result, roffset + i, ASMOP_A, 0, false);
      else if (aopIsLitVal (source, soffset + i, 1, 0x00) && aopInReg (result, roffset + i, A_IDX))
        {
          emit3 (A_XOR, ASMOP_A, ASMOP_A);
          regalloc_dry_run_cost += 1;
          zeroed_a = true;
        }
      else
        {
          cheapMove (result, roffset + i, source, soffset + i, a_dead_global);
          zeroed_a = false;
        }

      if (aopInReg (result, roffset + i, A_IDX))
        a_dead = false;
      if (aopInReg (result, roffset + i, H_IDX) || aopInReg (result, roffset + i, L_IDX))
        hl_dead = false;

      i++;
    }
}

/*-----------------------------------------------------------------*/
/* genMove - Copy the value from one asmop to another              */
/*-----------------------------------------------------------------*/
static void
genMove (asmop *result, asmop *source, bool a_dead, bool hl_dead)
{
  genMove_o (result, 0, source, 0, result->size, a_dead, hl_dead);
}

/*-----------------------------------------------------------------*/
/* getDataSize - get the operand data size                         */
/*-----------------------------------------------------------------*/
static int
getDataSize (operand * op)
{
  int size;
  size = AOP_SIZE (op);
  if (size == 3)
    {
      /* pointer */
      wassertl (0, "Somehow got a three byte data pointer");
    }
  return size;
}

/*--------------------------------------------------------------------------*/
/* adjustStack - Adjust the stack pointer by n bytes.                       */
/*--------------------------------------------------------------------------*/
static void
adjustStack (int n, bool af_free, bool bc_free, bool hl_free, bool iy_free)
{
  _G.stack.pushed -= n;

  if (IS_TLCS90 && abs(n) > (optimize.codeSize ? 2 + (af_free || bc_free || hl_free || iy_free || n < 0) * 2: 1))
    {
      emit2 ("add sp, !immed%d", n);
      cost (3, 6);
      n -= n;
    }
  else if (abs(n) > ((IS_RAB || IS_GB) ? 127 * 4 - 1 : (optimize.codeSize ? 8 : 5)) && hl_free)
    {
      spillCached ();
      emit2 ("ld hl, !immed%d", n);
      emit2 ("add hl, sp");
      emit2 ("ld sp, hl");
      cost2 (5, 27, 20, 10, 28, 18, 4);
      regalloc_dry_run_cost += 5;
      n -= n;
    }
  else if (!IS_GB && abs(n) > ((IS_RAB || IS_GB) ? 127 * 4 - 1 : 8) && iy_free)
    {
      spillCached ();
      emit2 ("ld iy, !immed%d", n);
      emit2 ("add iy, sp");
      emit2 ("ld sp, iy");
      regalloc_dry_run_cost += 8;
      n -= n;
    }
  else if (abs(n) > ((IS_RAB || IS_GB) ? 127 * 4 - 1 : 8) && bc_free)
    {
      emit2 ("ld c, l");
      emit2 ("ld b, h");
      emit2 ("ld hl, !immed%d", n);
      emit2 ("add hl, sp");
      emit2 ("ld sp, hl");
      emit2 ("ld l, c");
      emit2 ("ld h, b");
      regalloc_dry_run_cost += 9;
      n -= n;
    }

  while (abs(n))
    {
      if ((IS_RAB || IS_GB) && abs(n) > (optimize.codeSize ? 2 : 1))
        {
          int d;
          if (n > 127)
            d = 127;
          else if (n < -128)
            d = -128;
          else
            d = n;
          emit2 ("add sp, !immed%d", d);
          cost (2, IS_GB ? 16 : 4);
          n -= d;
        }
      else if (n >= 2 && af_free && (IS_Z80 || optimize.codeSize))
        {
          emit2 ("pop af");
          cost2 (1, 10, 9, 7, 12, 10, 3);
          n -= 2;
        }
      else if (n <= -2 && (IS_Z80 || optimize.codeSize))
        {
          emit2 ("push af");
          cost2 (1, 10, 11, 7, 12, 10, 3);
          n += 2;
        }
      else if (n >= 2 && bc_free && (IS_Z80 || optimize.codeSize))
        {
          emit2 ("pop bc");
          cost2 (1, 10, 9, 7, 12, 10, 3);
          n -= 2;
        }
      else if (n >= 2 && hl_free && (IS_Z80 || optimize.codeSize))
        {
          emit2 ("pop hl");
          cost2 (1, 10, 9, 7, 12, 10, 3);
          n -= 2;
        }
      else if (IS_TLCS90 && n >= 2 && iy_free && optimize.codeSize)
        {
          emit2 ("pop iy");
          cost (1, 10);
          n -= 2;
        }
      else if (n >= 1)
        {
          emit2 ("inc sp");
          cost2 (1, 6, 4, 2, 8, 4, 1);
          n--;
        }
      else if (n <= -1)
        {
          emit2 ("dec sp");
          cost2 (1, 6, 4, 2, 8, 4, 1);
          n++;
        }
    }

  wassert(!n);
}

/*-----------------------------------------------------------------*/
/* movLeft2Result - move byte from left to result                  */
/*-----------------------------------------------------------------*/
static void
movLeft2Result (operand *left, int offl, operand *result, int offr, int sign)
{
  if (!sameRegs (AOP (left), AOP (result)) || (offl != offr))
    {
      if (!sign)
        cheapMove (AOP (result), offr, AOP (left), offl, true);
      else
        {
          if (getDataSize (left) == offl + 1)
            {
              cheapMove (ASMOP_A, 0, AOP (left), offl, true);
              cheapMove (AOP (result), offr, ASMOP_A, 0, true);
            }
        }
    }
}

/** Put Acc into a register set
 */
static void
outAcc (operand * result)
{
  int size = getDataSize (result);
  if (size)
    {
      cheapMove (AOP (result), 0, ASMOP_A, 0, true);
      size--;
      genMove_o (result->aop, 1, ASMOP_ZERO, 0, size, true, false);
    }
}

/** Take the value in carry and put it into a register
 */
static void
outBitC (operand * result)
{
  /* if the result is bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      if (!IS_OP_RUONLY (result) && !regalloc_dry_run)
        aopPut (AOP (result), "c", 0);  // Todo: Cost.
    }
  else
    {
      emit2 ("ld a, !zero");
      emit2 ("rla");
      regalloc_dry_run_cost += 3;
      outAcc (result);
    }
}

/*-----------------------------------------------------------------*/
/* toBoolean - emit code for or a,operator(sizeop)                 */
/*-----------------------------------------------------------------*/
static void
_toBoolean (const operand *oper, bool needflag)
{
  int size = AOP_SIZE (oper);
  sym_link *type = operandType (oper);
  int skipbyte;

  if (size == 1 && needflag)
    {
      cheapMove (ASMOP_A, 0, oper->aop, 0, true);
      emit3 (A_OR, ASMOP_A, ASMOP_A);
      return;
    }

  // Special handling to not overwrite a.
  if (oper->aop->regs[A_IDX] >= 0)
    skipbyte = oper->aop->regs[A_IDX];
  else
    {
      cheapMove (ASMOP_A, 0, oper->aop, size - 1, true);
      skipbyte = size - 1;
    }

  if (IS_FLOAT (type))
    {
      if (skipbyte != size - 1)
        {
          wassert (regalloc_dry_run);
          regalloc_dry_run_cost += 120;
        }
      emit2 ("res 7, a");   //clear sign bit
      regalloc_dry_run_cost += 2;
      skipbyte = size - 1;
    }
  while (size--)
    if (size != skipbyte)
      {
        if (aopInReg (oper->aop, size, IYL_IDX) || aopInReg (oper->aop, size, IYH_IDX))
          {
            regalloc_dry_run_cost += 100;
            wassert (regalloc_dry_run);
          }
        emit3_o (A_OR, ASMOP_A, 0, oper->aop, size);
      }
}

/*-----------------------------------------------------------------*/
/* castBoolean - emit code for casting operand to boolean in a     */
/*-----------------------------------------------------------------*/
static void
_castBoolean (const operand *right)
{
  emitDebug ("; Casting to bool");

  /* Can do without OR-ing for small arguments */
  if (AOP_SIZE (right) == 1 && !aopInReg (right->aop, 0, A_IDX))
    {
      emit3 (A_XOR, ASMOP_A, ASMOP_A);
      emit3 (A_CP, ASMOP_A, AOP (right));
    }
  else
    {
      _toBoolean (right, FALSE);
      emit2 ("add a, !immedbyte", 0xff);
      emit2 ("ld a, !zero");
      regalloc_dry_run_cost += 4;
    }
  emit2 ("rla");
  regalloc_dry_run_cost += 1;
}

/* Shuffle src reg array into dst reg array. */
static void
regMove (const short *dst, const short *src, size_t n, bool preserve_a)
{
  bool assigned[9] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };
  int cached_byte = -1;
  size_t size = n;
  int ex[4] = {-1, -1, -1, -1};
  size_t i;
  bool pushed_a = FALSE;

  wassert (n <= 9);

  // Try to use ex de, hl
  if (size >= 4)
    {
      // Find E and check that it is exchanged with L.
      for (i = 0; i < n; i++)
        if (dst[i] == E_IDX && src[i] == L_IDX)
          ex[0] = i;
      for (i = 0; i < n; i++)
        if (dst[i] == L_IDX && src[i] == E_IDX)
          ex[1] = i;
      // Find D and check that it is exchanged with H.
      for (i = 0; i < n; i++)
        if (dst[i] == D_IDX && src[i] == H_IDX)
          ex[2] = i;
      for (i = 0; i < n; i++)
        if (dst[i] == H_IDX && src[i] == D_IDX)
          ex[3] = i;
      if (ex[0] >= 0 && ex[1] >= 0 && ex[2] >= 0 && ex[3] >= 0)
        {
          emit2 ("ex de, hl");
          regalloc_dry_run_cost++;
          assigned[ex[0]] = TRUE;
          assigned[ex[1]] = TRUE;
          assigned[ex[2]] = TRUE;
          assigned[ex[3]] = TRUE;
          size -= 4;
        }
    }

  // We need to be able to handle any assignment here, ensuring not to overwrite any parts of the source that we still need.
  while (size)
    {
      // Find lowest byte that can be assigned and needs to be assigned.
      for (i = 0; i < n; i++)
        {
          size_t j;

          if (assigned[i])
            continue;

          for (j = 0; j < n; j++)
            {
              if (!assigned[j] && i != j && dst[i] == src[j])
                goto skip_byte; // We can't write this one without overwriting the source.
            }

          break;                // Found byte that can be written safely.

skip_byte:
          ;
        }

      if (i < n)
        {
          cheapMove (asmopregs[dst[i]], 0, asmopregs[src[i]], 0, false);       // We can safely assign a byte.
          size--;
          assigned[i] = TRUE;
          continue;
        }

      // No byte can be assigned safely (i.e. the assignment is a permutation). Cache one in the accumulator.

      if (cached_byte != -1)
        {
          // Already one cached. Can happen when the assignment is a permutation consisting of multiple cycles.
          cheapMove (asmopregs[dst[cached_byte]], 0, ASMOP_A, 0, true);
          cached_byte = -1;
          continue;
        }

      for (i = 0; i < n; i++)
        if (!assigned[i])
          break;

      wassertl (i != n, "regMove error: Trying to cache non-existant byte in accumulator.");
      if (preserve_a && !pushed_a)
        {
          _push (PAIR_AF);
          pushed_a = TRUE;
        }
      cheapMove (ASMOP_A, 0, asmopregs[src[i]], 0, true);
      size--;
      assigned[i] = TRUE;
      cached_byte = i;
    }

  if (cached_byte != -1)
    cheapMove (asmopregs[dst[cached_byte]], 0, ASMOP_A, 0, true);

  if (pushed_a)
    _pop (PAIR_AF);
}

/*-----------------------------------------------------------------*/
/* genNot - generate code for ! operation                          */
/*-----------------------------------------------------------------*/
static void
genNot (const iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *result = IC_RESULT (ic);

  /* assign asmOps to operand & result */
  aopOp (left, ic, FALSE, TRUE);
  aopOp (result, ic, TRUE, FALSE);

  /* if in bit space then a special case */
  if (AOP_TYPE (left) == AOP_CRY)
    {
      wassertl (0, "Tried to negate a bit");
    }
  else if (IS_BOOL (operandType (left)))
    {
      cheapMove (ASMOP_A, 0, AOP (left), 0, true);
      emit2 ("xor a, !immedbyte", 0x01);
      regalloc_dry_run_cost += 2;
      cheapMove (AOP (result), 0, ASMOP_A, 0, true);
      goto release;
    }

  _toBoolean (left, FALSE);

  /* Not of A:
     If A == 0, !A = 1
     else A = 0
     So if A = 0, A-1 = 0xFF and C is set, rotate C into reg. */
  emit2 ("sub a,!one");
  regalloc_dry_run_cost += 2;
  outBitC (result);

release:
  /* release the aops */
  freeAsmop (left, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genCpl - generate code for complement                           */
/*-----------------------------------------------------------------*/
static void
genCpl (const iCode *ic)
{
  int skip_byte = -1;

  bool a_dead = !bitVectBitValue (ic->rSurv, A_IDX);
  bool pushed_a = false;

  /* assign asmOps to operand & result */
  aopOp (IC_LEFT (ic), ic, false, false);
  aopOp (IC_RESULT (ic), ic, true, false);

  /* if both are in bit space then
     a special case */
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY && AOP_TYPE (IC_LEFT (ic)) == AOP_CRY)
    wassertl (0, "Left and the result are in bit space");

  int size = IC_RESULT (ic)->aop->size;

  if (IC_LEFT (ic)->aop->regs[A_IDX] >= 0 && IC_LEFT (ic)->aop->regs[A_IDX] < size)
    {
      int i = IC_LEFT (ic)->aop->regs[A_IDX];
      emit3 (A_CPL, 0, 0);
      cheapMove (IC_RESULT (ic)->aop, i, ASMOP_A, 0, true);
      skip_byte = i;

      if (aopInReg (IC_RESULT (ic)->aop, i, A_IDX))
        a_dead = false;

      // Do not overwrite still-needed value
      if (IC_RESULT (ic)->aop->type == AOP_REG && !aopInReg (IC_RESULT (ic)->aop, i, A_IDX))
        {
          int j = IC_LEFT (ic)->aop->regs[IC_RESULT (ic)->aop->aopu.aop_reg[i]->rIdx];
          if (j >= 0 && j != skip_byte && j < size)
            {
              regalloc_dry_run_cost += 150;
              wassert (regalloc_dry_run);
            } 
        }
    }

  for (int i = 0; i < size; i++)
    {
      if (i == skip_byte)
        continue;

      if (!a_dead && !pushed_a)
        {
          _push (PAIR_AF);
          pushed_a = true;
        }

      cheapMove (ASMOP_A, 0, IC_LEFT (ic)->aop, i, true);
      emit3 (A_CPL, 0, 0);
      cheapMove (IC_RESULT (ic)->aop, i, ASMOP_A, 0, true);

      if (aopInReg (IC_RESULT (ic)->aop, i, A_IDX))
        a_dead = false;

      // Do not overwrite still-needed value
      if (IC_RESULT (ic)->aop->type == AOP_REG && !aopInReg (IC_RESULT (ic)->aop, i, A_IDX))
        {
          int j = IC_LEFT (ic)->aop->regs[IC_RESULT (ic)->aop->aopu.aop_reg[i]->rIdx];
          if (j > i && j < size && j != skip_byte)
            {
              regalloc_dry_run_cost += 150;
              wassert (regalloc_dry_run);
            } 
        }
    }

  if (pushed_a)
    _pop (PAIR_AF);

  /* release the aops */
  freeAsmop (IC_LEFT (ic), 0);
  freeAsmop (IC_RESULT (ic), 0);
}

static void
_gbz80_emitAddSubLongLong (const iCode * ic, asmop * left, asmop * right, bool isAdd)
{
  enum asminst first = isAdd ? A_ADD : A_SUB;
  enum asminst later = isAdd ? A_ADC : A_SBC;


  /* Logic:
     ld de,right.lw
     setup hl to left
     de = hl - de
     push flags
     store de into result
     pop flags
     ld de,right.hw
     setup hl
     de = hl -de
     store de into result
   */

  wassertl (IS_GB, "Code is only relevant to the gbz80");
  wassertl (AOP (IC_RESULT (ic))->size == 4, "Only works for four bytes");

  fetchPair (PAIR_DE, left);

  emit2 ("ld a, e");
  regalloc_dry_run_cost += 1;
  emit3_o (first, ASMOP_A, 0, right, LSB);
  emit2 ("ld e, a");
  emit2 ("ld a, d");
  regalloc_dry_run_cost += 2;
  emit3_o (later, ASMOP_A, 0, right, MSB16);

  _push (PAIR_AF);

  cheapMove (AOP (IC_RESULT (ic)), MSB16, ASMOP_A, 0, true);
  cheapMove (AOP (IC_RESULT (ic)), LSB, ASMOP_E, 0, true);

  fetchPairLong (PAIR_DE, left, NULL, MSB24);

  if (!regalloc_dry_run)
    aopGet (right, MSB24, FALSE);

  _pop (PAIR_AF);
  emit2 ("ld a, e");
  emit3_o (later, ASMOP_A, 0, right, MSB24);
  emit2 ("ld e, a");
  emit2 ("ld a, d");
  regalloc_dry_run_cost += 2;
  emit3_o (later, ASMOP_A, 0, right, MSB32);

  cheapMove (AOP (IC_RESULT (ic)), MSB32, ASMOP_A, 0, true);
  cheapMove (AOP (IC_RESULT (ic)), MSB24, ASMOP_E, 0, true);
}

static void
_gbz80_emitAddSubLong (const iCode * ic, bool isAdd)
{
  _gbz80_emitAddSubLongLong (ic, AOP (IC_LEFT (ic)), AOP (IC_RIGHT (ic)), isAdd);
}

/*-----------------------------------------------------------------*/
/* assignResultValue -               */
/*-----------------------------------------------------------------*/
static void
assignResultValue (operand * oper)
{
  int size = oper->aop->size;

  wassertl (size <= 4, "Got a result that is bigger than four bytes");

  if (IS_GB && size == 4 && requiresHL (AOP (oper)))
    {
      /* We do it the hard way here. */
      _push (PAIR_HL);
      cheapMove (oper->aop, 0, ASMOP_RETURN, 0, true);
      cheapMove (oper->aop, 1, ASMOP_RETURN, 1, true);
      _pop (PAIR_DE);
      cheapMove (oper->aop, 2, ASMOP_E, 0, true);
      cheapMove (oper->aop, 3, ASMOP_D, 0, true);
    }
  else
    genMove (oper->aop, ASMOP_RETURN, true, true);
}

/* Pop saved regs from stack, taking care not to destroy result */
static void
restoreRegs (bool iy, bool de, bool bc, bool hl, const operand *result)
{
  bool bInRet, cInRet, dInRet, eInRet, hInRet, lInRet;
  bool SomethingReturned;

  SomethingReturned = (result && IS_ITEMP (result) &&
                      (OP_SYMBOL_CONST (result)->nRegs ||
                      OP_SYMBOL_CONST (result)->spildir ||
                      OP_SYMBOL_CONST (result)->accuse == ACCUSE_A)) || IS_TRUE_SYMOP (result);

  if (SomethingReturned)
    {
      bitVect *rv = z80_rUmaskForOp (result);
      bInRet = bitVectBitValue (rv, B_IDX);
      cInRet = bitVectBitValue (rv, C_IDX);
      dInRet = bitVectBitValue (rv, D_IDX);
      eInRet = bitVectBitValue (rv, E_IDX);
      hInRet = bitVectBitValue (rv, H_IDX);
      lInRet = bitVectBitValue (rv, L_IDX);
      freeBitVect (rv);
    }
  else
    {
      bInRet = FALSE;
      cInRet = FALSE;
      dInRet = FALSE;
      eInRet = FALSE;
      hInRet = FALSE;
      lInRet = FALSE;
    }

  if (iy)
    _pop (PAIR_IY);

  if (de)
    {
      if (dInRet && eInRet)
        wassertl (0, "Shouldn't push DE if it's wiped out by the return");
      else if (dInRet)
        {
          /* Only restore E */
          emit2 ("ld a, d");
          regalloc_dry_run_cost += 1;
          _pop (PAIR_DE);
          emit2 ("ld d, a");
          regalloc_dry_run_cost += 1;
        }
      else if (eInRet)
        {
          /* Only restore D */
          _pop (PAIR_AF);
          emit2 ("ld d, a");
          regalloc_dry_run_cost += 1;
        }
      else
        _pop (PAIR_DE);
    }

  if (bc)
    {
      if (bInRet && cInRet)
        wassertl (0, "Shouldn't push BC if it's wiped out by the return");
      else if (bInRet)
        {
          /* Only restore C */
          emit2 ("ld a, b");
          regalloc_dry_run_cost += 1;
          _pop (PAIR_BC);
          emit2 ("ld b, a");
          regalloc_dry_run_cost += 1;
        }
      else if (cInRet)
        {
          /* Only restore B */
          _pop (PAIR_AF);
          emit2 ("ld b, a");
          regalloc_dry_run_cost += 1;
        }
      else
        _pop (PAIR_BC);
    }

  if (hl)
    {
      if (hInRet && lInRet)
        wassertl (0, "Shouldn't push HL if it's wiped out by the return");
      else if (hInRet)
        {
          /* Only restore E */
          emit2 ("ld a, h");
          regalloc_dry_run_cost += 1;
          _pop (PAIR_HL);
          emit2 ("ld h, a");
          regalloc_dry_run_cost += 1;
        }
      else if (lInRet)
        {
          /* Only restore D */
          _pop (PAIR_AF);
          emit2 ("ld h, a");
          regalloc_dry_run_cost += 1;
        }
      else
        _pop (PAIR_HL);
    }
}

static void
_saveRegsForCall (const iCode * ic, bool dontsaveIY)
{
  /* Rules:
     o Stack parameters are pushed before this function enters
     o DE and BC may be used in this function.
     o HL and DE may be used to return the result.
     o HL and DE may be used to send variables.
     o DE and BC may be used to store the result value.
     o HL may be used in computing the sent value of DE
     o The iPushes for other parameters occur before any addSets

     Logic: (to be run inside the first iPush or if none, before sending)
     o Compute if DE, BC, HL, IY are in use over the call
     o Compute if DE is used in the send set
     o Compute if DE and/or BC are used to hold the result value
     o If (DE is used, or in the send set) and is not used in the result, push.
     o If BC is used and is not in the result, push
     o
     o If DE is used in the send set, fetch
     o If HL is used in the send set, fetch
     o Call
     o ...
   */

  sym_link *dtype = operandType (IC_LEFT (ic));
  sym_link *ftype = IS_FUNCPTR (dtype) ? dtype->next : dtype;

  if (_G.saves.saved == FALSE)
    {
      bool push_bc, push_de, push_hl, push_iy;

      if (options.oldralloc)
        {
          bool deInUse, bcInUse;
          bool bcInRet = FALSE, deInRet = FALSE;
          bitVect *rInUse;

          rInUse = bitVectCplAnd (bitVectCopy (ic->rMask), z80_rUmaskForOp (IC_RESULT (ic)));

          deInUse = bitVectBitValue (rInUse, D_IDX) || bitVectBitValue (rInUse, E_IDX);
          bcInUse = bitVectBitValue (rInUse, B_IDX) || bitVectBitValue (rInUse, C_IDX);

          emitDebug ("; _saveRegsForCall: deInUse: %u bcInUse: %u", deInUse, bcInUse);

          push_bc = bcInUse && !bcInRet;
          push_de = deInUse && !deInRet;
          push_hl = FALSE;
          push_iy = FALSE;
        }
      else
        {
          push_bc = bitVectBitValue (ic->rSurv, B_IDX) && !ftype->funcAttrs.preserved_regs[B_IDX] || bitVectBitValue (ic->rSurv, C_IDX) && !ftype->funcAttrs.preserved_regs[C_IDX];
          push_de = bitVectBitValue (ic->rSurv, D_IDX) && !ftype->funcAttrs.preserved_regs[D_IDX] || bitVectBitValue (ic->rSurv, E_IDX) && !ftype->funcAttrs.preserved_regs[E_IDX];
          push_hl = bitVectBitValue (ic->rSurv, H_IDX) || bitVectBitValue (ic->rSurv, L_IDX);
          push_iy = !dontsaveIY && (bitVectBitValue (ic->rSurv, IYH_IDX) || bitVectBitValue (ic->rSurv, IYL_IDX));
        }

      if (push_hl)
        {
          _push (PAIR_HL);
          _G.stack.pushedHL = TRUE;
        }
      if (push_bc)
        {
          _push (PAIR_BC);
          _G.stack.pushedBC = TRUE;
        }
      if (push_de)
        {
          _push (PAIR_DE);
          _G.stack.pushedDE = TRUE;
        }
      if (push_iy)
        {
          _push (PAIR_IY);
          _G.stack.pushedIY = TRUE;
        }

      if (!regalloc_dry_run)
        _G.saves.saved = TRUE;
    }
  else
    {
      /* Already saved. */
    }
}

/*-----------------------------------------------------------------*/
/* genIpush - genrate code for pushing this gets a little complex  */
/*-----------------------------------------------------------------*/
static void
genIpush (const iCode *ic)
{
  int size, offset = 0;

  /* if this is not a parm push : ie. it is spill push
     and spill push is always done on the local stack */
  if (!ic->parmPush)
    {
      wassertl (0, "Encountered an unsupported spill push.");
      return;
    }
      
  /* Scan ahead until we find the function that we are pushing parameters to.
     Count the number of addSets on the way to figure out what registers
     are used in the send set.
   */
  int nAddSets = 0;
  iCode *walk = ic->next;

  while (walk)
    {
      if (walk->op == SEND && !_G.saves.saved && !regalloc_dry_run)
        nAddSets++;
      else if (walk->op == CALL || walk->op == PCALL)
        break; // Found it.

      walk = walk->next; // Keep looking.
    }
  if (!regalloc_dry_run && !_G.saves.saved && !regalloc_dry_run) /* Cost is counted at CALL or PCALL instead */
    _saveRegsForCall (walk, false); /* Caller saves, and this is the first iPush. */

  const bool smallc = IFFUNC_ISSMALLC (operandType (IC_LEFT (walk)));

  /* then do the push */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);

  size = AOP_SIZE (IC_LEFT (ic));

  if (isPair (AOP (IC_LEFT (ic))) && size == 2)
    {
      if (!regalloc_dry_run)
        {
          _G.stack.pushed += 2;
          emit2 ("push %s", getPairName (AOP (IC_LEFT (ic))));
        }
      regalloc_dry_run_cost += (getPairId (AOP (IC_LEFT (ic))) == PAIR_IY ? 2 : 1);
    }
  else
    {
      if (size == 1 && smallc) /* The SmallC calling convention pushes 8-bit parameters as 16-bit values. */
        {
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[2]->rIdx == C_IDX)
            {
              emit2 ("push bc");
              regalloc_dry_run_cost += 1;
            }
          else if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[2]->rIdx == E_IDX)
            {
              emit2 ("push de");
              regalloc_dry_run_cost += 1;
            }
          else if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[2]->rIdx == L_IDX)
            {
              emit2 ("push hl");
              regalloc_dry_run_cost += 1;
            }
          else
            {
              emit2 ("dec sp");
              cheapMove (ASMOP_A, 0, AOP (IC_LEFT (ic)), 0, true);
              emit2 ("push af");
              emit2 ("inc sp");
              regalloc_dry_run_cost += 3;
            }
          if (!regalloc_dry_run)
            _G.stack.pushed += 2;
          goto release;
        }
      else if (size == 2)
        {
          PAIR_ID pair = getDeadPairId (ic);
          if (pair == PAIR_INVALID || isPairDead (PAIR_HL, ic))
            pair = PAIR_HL;     /* hl sometimes is cheaper to load than other pairs. */

          fetchPairLong (pair, AOP (IC_LEFT (ic)), ic, 0);
          if (!regalloc_dry_run)
            {
              emit2 ("push %s", _pairs[pair].name);
              _G.stack.pushed += 2;
            }
          regalloc_dry_run_cost += 1;
          goto release;
        }
      if (size == 4)
        {
          if (getPairId_o (IC_LEFT (ic)->aop, 2) != PAIR_INVALID)
            {
              emit2 ("push %s", _pairs[getPairId_o (IC_LEFT (ic)->aop, 2)].name);
              regalloc_dry_run_cost += 1 + (getPairId_o (IC_LEFT (ic)->aop, 2) == PAIR_IY);
            }
          else
            {
              fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), 0, 2);
              emit2 ("push hl");
              regalloc_dry_run_cost += 1;
            }
          if (!regalloc_dry_run)
            _G.stack.pushed += 2;

          if (getPairId_o (IC_LEFT (ic)->aop, 0) != PAIR_INVALID)
            {
              emit2 ("push %s", _pairs[getPairId_o (IC_LEFT (ic)->aop, 0)].name);
              regalloc_dry_run_cost += 1 + (getPairId_o (IC_LEFT (ic)->aop, 2) == PAIR_IY);
            }
          else
            {
              fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), 0, 0);
              emit2 ("push hl");
              regalloc_dry_run_cost += 1;
            }
          if (!regalloc_dry_run)
            _G.stack.pushed += 2;

          goto release;
        }
      offset = size;
      while (size--)
        {
          if (size && getFreePairId (ic) != PAIR_INVALID)
            {
              offset -= 2;

              PAIR_ID pair = getFreePairId (ic);
              fetchPairLong (pair, AOP (IC_LEFT (ic)), 0, offset);
              emit2 ("push %s", _pairs[pair].name);
              regalloc_dry_run_cost += 1;
              size--;

              if (!regalloc_dry_run)
                _G.stack.pushed += 2;

              continue;
            }
          else if (AOP (IC_LEFT (ic))->type == AOP_IY)
            {
              wassertl (!bitVectBitValue (ic->rSurv, A_IDX), "Loading from address destroys A, which must survive.");
              emit2 ("ld a, (%s)", aopGetLitWordLong (AOP (IC_LEFT (ic)), --offset, FALSE));
              emit2 ("push af");
              regalloc_dry_run_cost += 4;
            }
          else
            {
              offset--;
              if (AOP (IC_LEFT (ic))->type == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[offset]->rIdx == B_IDX)
                {
                  emit2 ("push bc");
                  regalloc_dry_run_cost += 1;
                }
              else if (AOP (IC_LEFT (ic))->type == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[offset]->rIdx == D_IDX)
                {
                  emit2 ("push de");
                  regalloc_dry_run_cost += 1;
                }
              else if (AOP (IC_LEFT (ic))->type == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[offset]->rIdx == H_IDX)
                {
                  emit2 ("push hl");
                  regalloc_dry_run_cost += 1;
                }
              else
                {
                  if (AOP_TYPE (IC_LEFT (ic)) != AOP_SFR && !regalloc_dry_run && !strcmp (aopGet (AOP (IC_LEFT (ic)), offset, FALSE), "h"))   // todo: More exact cost!
                    emit2 ("push hl");
                  else
                    {
                      wassertl (aopInReg (IC_LEFT (ic)->aop, 0, A_IDX) || !bitVectBitValue (ic->rSurv, A_IDX), "Push operand destroys A, which must survive.");
                      if (AOP_TYPE (IC_LEFT (ic)) == AOP_LIT && byteOfVal (AOP (IC_LEFT (ic))->aopu.aop_lit, offset) == 0x00)
                        emit3 (A_XOR, ASMOP_A, ASMOP_A);
                      else
                        {
                          cheapMove (ASMOP_A, 0, AOP (IC_LEFT (ic)), offset, true);
                          if (!aopInReg (IC_LEFT (ic)->aop, 0, A_IDX))
                            regalloc_dry_run_cost += ld_cost (ASMOP_A, AOP (IC_LEFT (ic)));
                        }
                      emit2 ("push af");
                      regalloc_dry_run_cost += 1;
                    }
                }
            }
          if (!regalloc_dry_run)
            {
              emit2 ("inc sp");
              _G.stack.pushed++;
            }
          regalloc_dry_run_cost += 1;
        }
    }
release:
  freeAsmop (IC_LEFT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genIpop - recover the registers: can happen only for spilling   */
/*-----------------------------------------------------------------*/
static void
genIpop (const iCode * ic)
{
  int size, offset;

  wassert (!regalloc_dry_run);

  /* if the temp was not pushed then */
  if (OP_SYMBOL (IC_LEFT (ic))->isspilt)
    return;

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));
  offset = (size - 1);
  if (isPair (AOP (IC_LEFT (ic))))
    {
      emit2 ("pop %s", getPairName (AOP (IC_LEFT (ic))));
    }
  else
    {
      while (size--)
        {
          emit2 ("dec sp");
          emit2 ("pop hl");
          spillPair (PAIR_HL);
          aopPut (AOP (IC_LEFT (ic)), "l", offset--);
        }
    }

  freeAsmop (IC_LEFT (ic), NULL);
}

/* This is quite unfortunate */
static void
setArea (int inHome)
{
  /*
     static int lastArea = 0;

     if (_G.in_home != inHome) {
     if (inHome) {
     const char *sz = port->mem.code_name;
     port->mem.code_name = "HOME";
     emit2("!area", CODE_NAME);
     port->mem.code_name = sz;
     }
     else
     emit2("!area", CODE_NAME); */
  _G.in_home = inHome;
  //    }
}

static bool
isInHome (void)
{
  return _G.in_home;
}

/** Emit the code for a register parameter
 */
static void genSend (const iCode *ic)
{
  int size;

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));

  wassertl (ic->next->op == CALL || ic->next->op == PCALL, "Sending register parameter for missing call");
  wassertl (!IS_GB, "Register parameters are not supported in gbz80 port");

  if (_G.saves.saved == FALSE && !regalloc_dry_run /* Cost is counted at CALL or PCALL instead */ )
    {
      /* Caller saves, and this is the first iPush. */
      /* Scan ahead until we find the function that we are pushing parameters to.
         Count the number of addSets on the way to figure out what registers
         are used in the send set.
       */
      int nAddSets = 0;
      iCode *walk = ic->next;

      while (walk)
        {
          if (walk->op == SEND)
            {
              nAddSets++;
            }
          else if (walk->op == CALL || walk->op == PCALL)
            {
              /* Found it. */
              break;
            }
          else
            {
              /* Keep looking. */
            }
          walk = walk->next;
        }
      _saveRegsForCall (walk, FALSE);
    }
  if (size == 2)
    {
      fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), ic, 0);
      z80_regs_used_as_parms_in_calls_from_current_function[L_IDX] = true;
      z80_regs_used_as_parms_in_calls_from_current_function[H_IDX] = true;
    }
  else if (size <= 4)
    {
      if (size == 4 && ASMOP_RETURN->aopu.aop_reg[0]->rIdx == L_IDX && ASMOP_RETURN->aopu.aop_reg[1]->rIdx == H_IDX &&
        ASMOP_RETURN->aopu.aop_reg[2]->rIdx == E_IDX && ASMOP_RETURN->aopu.aop_reg[3]->rIdx == D_IDX)
        {
          if (!isPairDead (PAIR_DE, ic) && getPairId_o (AOP (IC_LEFT (ic)), 2) != PAIR_DE)
            {
              regalloc_dry_run_cost += 100;
              wassertl (regalloc_dry_run, "Register parameter overwrites value that is still needed");
            }
          fetchPairLong (PAIR_DE, AOP (IC_LEFT (ic)), ic, 2);
          z80_regs_used_as_parms_in_calls_from_current_function[E_IDX] = true;
          z80_regs_used_as_parms_in_calls_from_current_function[D_IDX] = true;
          fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), ic, 0);
          z80_regs_used_as_parms_in_calls_from_current_function[L_IDX] = true;
          z80_regs_used_as_parms_in_calls_from_current_function[H_IDX] = true;
        }
      else
        {
          for (int i = 0; i < AOP_SIZE (IC_LEFT (ic)); i++)
            if (!regalloc_dry_run)
              z80_regs_used_as_parms_in_calls_from_current_function[ASMOP_RETURN->aopu.aop_reg[i]->rIdx] = true;

          genMove_o (ASMOP_RETURN, 0, IC_LEFT (ic)->aop, 0, IC_LEFT (ic)->aop->size, true, true);
        }
    }

  freeAsmop (IC_LEFT (ic), NULL);
}

/** Emit the code for a call statement
 */
static void
genCall (const iCode *ic)
{
  sym_link *dtype = operandType (IC_LEFT (ic));
  sym_link *etype = getSpec (dtype);
  sym_link *ftype = IS_FUNCPTR (dtype) ? dtype->next : dtype;
  int i;
  int prestackadjust = 0;
  bool tailjump = false;

  const bool z88dk_callee = IFFUNC_ISZ88DK_CALLEE (ftype);

  for (i = 0; i < IYH_IDX + 1; i++)
    z80_regs_preserved_in_calls_from_current_function[i] |= ftype->funcAttrs.preserved_regs[i];

  _saveRegsForCall (ic, FALSE);

  const bool bigreturn = (getSize (ftype->next) > 4); // Return value of big type or returning struct or union.
  const bool SomethingReturned = (IS_ITEMP (IC_RESULT (ic)) &&
                       (OP_SYMBOL (IC_RESULT (ic))->nRegs ||
                        OP_SYMBOL (IC_RESULT (ic))->spildir ||
                        OP_SYMBOL (IC_RESULT (ic))->accuse == ACCUSE_A)) || IS_TRUE_SYMOP (IC_RESULT (ic));

  aopOp (IC_LEFT (ic), ic, false, false);
  if (SomethingReturned && !bigreturn)
    aopOp (IC_RESULT (ic), ic, false, false);

  if (bigreturn)
    {
      PAIR_ID pair;
      int fp_offset, sp_offset;

      if (ic->op == PCALL && IS_GB)
        _push (PAIR_HL);
      aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
      wassert (AOP_TYPE (IC_RESULT (ic)) == AOP_STK || AOP_TYPE (IC_RESULT (ic)) == AOP_EXSTK);
      fp_offset =
        AOP (IC_RESULT (ic))->aopu.aop_stk + (AOP (IC_RESULT (ic))->aopu.aop_stk >
            0 ? _G.stack.param_offset : 0);
      sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
      pair = (ic->op == PCALL && !IS_GB && !IY_RESERVED) ? PAIR_IY : PAIR_HL;
      emit2 ("ld %s, !immedword", _pairs[pair].name, sp_offset);
      emit2 ("add %s, sp", _pairs[pair].name);
      regalloc_dry_run_cost += (pair == PAIR_IY ? 6 : 4);
      if (ic->op == PCALL && IS_GB)
        {
          emit2 ("ld e, l");
          emit2 ("ld d, h");
          regalloc_dry_run_cost += 2;
          _pop (PAIR_HL);
          pair = PAIR_DE;
        }
      emit2 ("push %s", _pairs[pair].name);
      regalloc_dry_run_cost += (pair == PAIR_IY ? 2 : 1);
      if (!regalloc_dry_run)
        _G.stack.pushed += 2;
      freeAsmop (IC_RESULT (ic), NULL);
    }
  // Check if we can do tail call optimization.
  else if (!(currFunc && IFFUNC_ISISR (currFunc->type)) &&
    (!SomethingReturned || IC_RESULT (ic)->aop->size == 1 && aopInReg (IC_RESULT (ic)->aop, 0, IS_GB ? E_IDX : L_IDX) || IC_RESULT (ic)->aop->size == 2 && aopInReg (IC_RESULT (ic)->aop, 0, IS_GB ? DE_IDX : HL_IDX)) &&
    !ic->parmBytes && !ic->localEscapeAlive && !IFFUNC_ISBANKEDCALL (dtype) && !IFFUNC_ISZ88DK_SHORTCALL (ftype) && _G.omitFramePtr &&
    (ic->op != PCALL || !IFFUNC_ISZ88DK_FASTCALL (ftype)))
    {
      int limit = 16; // Avoid endless loops in the code putting us into an endless loop here.

      for (const iCode *nic = ic->next; nic && --limit;)
        {
          const symbol *targetlabel = 0;

          if (nic->op == LABEL)
            ;
          else if (nic->op == GOTO) // We dont have ebbi here, so we cant jsut use eBBWithEntryLabel (ebbi, ic->label). Search manually.
            targetlabel = IC_LABEL (nic);
          else if (nic->op == RETURN && (!IC_LEFT (nic) || SomethingReturned && IC_RESULT (ic)->key == IC_LEFT (nic)->key))
            targetlabel = returnLabel;
          else if (nic->op == ENDFUNCTION)
            {
              if (OP_SYMBOL (IC_LEFT (nic))->stack <= (ic->op == PCALL ? 1 : (optimize.codeSize ? 1 : 2)) + IS_RAB * 120)
                {
                  prestackadjust = OP_SYMBOL (IC_LEFT (nic))->stack;
                  tailjump = true;
                }
              break;
            }
          else
            break;

          if (targetlabel)
            {
              const iCode *nnic = 0;
              for (nnic = nic->next; nnic; nnic = nnic->next)
                if (nnic->op == LABEL && IC_LABEL (nnic)->key == targetlabel->key)
                  break;
              if (!nnic)
                for (nnic = nic->prev; nnic; nnic = nnic->prev)
                  if (nnic->op == LABEL && IC_LABEL (nnic)->key == targetlabel->key)
                    break;
              if (!nnic)
                break;

              nic = nnic;
            }
          else
            nic = nic->next;
        }
    }

  const bool jump = tailjump || !ic->parmBytes && !bigreturn && ic->op != PCALL && !IFFUNC_ISBANKEDCALL (dtype) && !IFFUNC_ISZ88DK_SHORTCALL(ftype) && IFFUNC_ISNORETURN (ftype);

  if (ic->op == PCALL)
    {
      if (IFFUNC_ISBANKEDCALL (dtype))
        {
          werror (W_INDIR_BANKED);
        }
      else if (IFFUNC_ISZ88DK_SHORTCALL (ftype)) 
       {
          wassertl(0, "__z88dk_short_call via function pointer not implemented");
       }

      if (isLitWord (AOP (IC_LEFT (ic))))
        {
          adjustStack (prestackadjust, false, false, false, false);
          emit2 (jump ? "jp %s" : "call %s", aopGetLitWordLong (AOP (IC_LEFT (ic)), 0, FALSE));
          regalloc_dry_run_cost += 3;
        }
      else if (getPairId (AOP (IC_LEFT (ic))) != PAIR_IY && !IFFUNC_ISZ88DK_FASTCALL (ftype))
        {
          spillPair (PAIR_HL);
          fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), ic, 0);
          adjustStack (prestackadjust, false, false, false, false);
          emit2 (jump ? "jp (hl)" : "call ___sdcc_call_hl");
          regalloc_dry_run_cost += 3;
        }
      else if (!IS_GB && !IY_RESERVED)
        {
          spillPair (PAIR_IY);
          fetchPairLong (PAIR_IY, IC_LEFT (ic)->aop, ic, 0);
          adjustStack (prestackadjust, false, false, false, false);
          emit2 (jump ? "jp (iy)" : "call ___sdcc_call_iy");
          regalloc_dry_run_cost += 3;
        }
      else // Use bc, since it is the only 16-bit register guarateed to be free even for __z88dk_fastcall with --reserve-regs-iy
        {
          wassert (!prestackadjust);
          wassert (IY_RESERVED); // The peephole optimizer handles ret for purposes other than returning only for --reserve-regs-iy
          if (aopInReg (IC_LEFT (ic)->aop, 0, B_IDX) || aopInReg (IC_LEFT (ic)->aop, 0, C_IDX) || aopInReg (IC_LEFT (ic)->aop, 1, B_IDX) || aopInReg (IC_LEFT (ic)->aop, 1, C_IDX))
            {
              regalloc_dry_run_cost += 100;
              wassertl (regalloc_dry_run, "Unimplemented function pointer in bc");
            }
          symbol *tlbl = 0;
          if (!regalloc_dry_run)
            {
              tlbl = newiTempLabel (NULL);
              emit2 ("ld bc, #!tlabel", labelKey2num (tlbl->key));
              emit2 ("push bc");
              regalloc_dry_run_cost += 4;
            }
          fetchPairLong (PAIR_BC, IC_LEFT (ic)->aop, 0, 0);
          emit2 ("push bc");
          emit2 ("ret");
          regalloc_dry_run_cost += 2;
          if (tlbl)
            emit2 ("!tlabeldef", labelKey2num (tlbl->key));
        }
    }
  else
    {
      /* make the call */
      if (IFFUNC_ISBANKEDCALL (dtype))
        {
          wassert (!prestackadjust);

          char *name = OP_SYMBOL (IC_LEFT (ic))->rname[0] ? OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name;
          emit2 ("call banked_call");
          emit2 ("!dws", name);
          emit2 ("!dw !bankimmeds", name);
          regalloc_dry_run_cost += 6;
        }
      else
        {
          adjustStack (prestackadjust, false, false, false, false);

          if (IS_LITERAL (etype))
            {
              emit2 (jump ? "jp 0x%04X" : "call 0x%04X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
              regalloc_dry_run_cost += 3;
            }
          else if (IFFUNC_ISZ88DK_SHORTCALL(ftype)) 
            {
              int rst = ftype->funcAttrs.z88dk_shortcall_rst;
              int value = ftype->funcAttrs.z88dk_shortcall_val;
              emit2 ("rst 0x%02x", rst);
              if ( value < 256 ) 
                emit2 ("defb 0x%02x\n",value);
              else
                emit2 ("defw 0x%04x\n",value);
              regalloc_dry_run_cost += 3;
            }
          else
            {
              emit2 ("%s %s", jump ? "jp" : "call",
                (OP_SYMBOL (IC_LEFT (ic))->rname[0] ? OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name));
              regalloc_dry_run_cost += 3;
            }
        }
    }
  spillCached ();

  freeAsmop (IC_LEFT (ic), 0);

  _G.stack.pushed += prestackadjust;

  /* Mark the registers as restored. */
  _G.saves.saved = FALSE;

  /* adjust the stack for parameters if required */
  if ((ic->parmBytes || bigreturn) && (IFFUNC_ISNORETURN (ftype) || z88dk_callee))
    {
      if (!regalloc_dry_run)
        {
          _G.stack.pushed -= (ic->parmBytes + bigreturn * 2);
          z80_symmParm_in_calls_from_current_function = FALSE;
        }
    }
  else if ((ic->parmBytes || bigreturn))
    {
      adjustStack (ic->parmBytes + bigreturn * 2, !IS_TLCS90, TRUE, !SomethingReturned || bigreturn, !IY_RESERVED);

      if (regalloc_dry_run)
        _G.stack.pushed += ic->parmBytes + bigreturn * 2;
    }

  /* if we need assign a result value */
  if (SomethingReturned && !bigreturn)
    {
      assignResultValue (IC_RESULT (ic));

      freeAsmop (IC_RESULT (ic), NULL);
    }

  spillCached ();

  restoreRegs (_G.stack.pushedIY, _G.stack.pushedDE, _G.stack.pushedBC, _G.stack.pushedHL, IC_RESULT (ic));
  _G.stack.pushedIY = FALSE;
  _G.stack.pushedDE = FALSE;
  _G.stack.pushedBC = FALSE;
  _G.stack.pushedHL = FALSE;
}

/*-----------------------------------------------------------------*/
/* resultRemat - result  is rematerializable                       */
/*-----------------------------------------------------------------*/
static int
resultRemat (const iCode * ic)
{
  if (SKIP_IC (ic) || ic->op == IFX)
    return 0;

  if (IC_RESULT (ic) && IS_ITEMP (IC_RESULT (ic)))
    {
      const symbol *sym = OP_SYMBOL_CONST (IC_RESULT (ic));
      if (sym->remat && !POINTER_SET (ic) && sym->isspilt)
        return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void
genFunction (const iCode * ic)
{
  bool stackParm;

  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  sym_link *ftype;

  bool bcInUse = FALSE;
  bool deInUse = FALSE;
  bool bigreturn;

  setArea (IFFUNC_NONBANKED (sym->type));
  wassert (!_G.stack.pushed);

  /* PENDING: Reset the receive offset as it
     doesn't seem to get reset anywhere else.
   */
  _G.receiveOffset = 0;
  _G.stack.param_offset = sym->type->funcAttrs.z88dk_params_offset;

  /* Record the last function name for debugging. */
  _G.lastFunctionName = sym->rname;

  /* Create the function header */
  emit2 ("!functionheader", sym->name);

  emitDebug (z80_assignment_optimal ? "; Register assignment is optimal." : "; Register assignment might be sub-optimal.");
  emitDebug ("; Stack space usage: %d bytes.", sym->stack);

  if (IS_STATIC (sym->etype))
    emit2 ("!functionlabeldef", sym->rname);
  else
    emit2 ("!globalfunctionlabeldef", sym->rname);
 
  if (!regalloc_dry_run)
    genLine.lineCurr->isLabel = 1;

  ftype = operandType (IC_LEFT (ic));

  if (IFFUNC_ISNAKED (ftype))
    {
      emitDebug ("; naked function: no prologue.");
      return;
    }

  /* if this is an interrupt service routine
     then save all potentially used registers. */
  if (IFFUNC_ISISR (sym->type))
    {
      if (!IFFUNC_ISCRITICAL (sym->type))
        {
          emit2 ("!ei");
        }

      emit2 ("!pusha");
    }
  else
    {
      /* This is a non-ISR function.
         If critical function then turn interrupts off */
      if (IFFUNC_ISCRITICAL (sym->type))
        {
          if (IS_GB || IS_RAB || IS_TLCS90)
            {
              emit2 ("!di");
            }
          else
            {
              //get interrupt enable flag IFF2 into P/O
              emit2 ("ld a,i");
              emit2 ("!di");
              //save P/O flag
              emit2 ("push af");
              _G.stack.param_offset += 2;
            }
        }
    }

  if (options.profile)
    {
      emit2 ("!profileenter");
    }

  if (z80_opts.calleeSavesBC)
    {
      bcInUse = TRUE;
    }

  /* Detect which registers are used. */
  if (IFFUNC_CALLEESAVES (sym->type) && sym->regsUsed)
    {
      int i;
      for (i = 0; i < sym->regsUsed->size; i++)
        {
          if (bitVectBitValue (sym->regsUsed, i))
            {
              switch (i)
                {
                case C_IDX:
                case B_IDX:
                  bcInUse = TRUE;
                  break;
                case D_IDX:
                case E_IDX:
                  if (!IS_GB)
                    {
                      deInUse = TRUE;
                    }
                  else
                    {
                      /* Other systems use DE as a temporary. */
                    }
                  break;
                }
            }
        }
    }

  if (bcInUse)
    {
      emit2 ("push bc");
      _G.stack.param_offset += 2;
    }

  _G.calleeSaves.pushedBC = bcInUse;

  if (deInUse)
    {
      emit2 ("push de");
      _G.stack.param_offset += 2;
    }

  _G.calleeSaves.pushedDE = deInUse;

  /* adjust the stack for the function */
//  _G.stack.last = sym->stack;

  bigreturn = (getSize (ftype->next) > 4);
  _G.stack.param_offset += bigreturn * 2;

  stackParm = FALSE;
  for (sym = setFirstItem (istack->syms); sym; sym = setNextItem (istack->syms))
    {
      if (sym->_isparm && !IS_REGPARM (sym->etype))
        {
          stackParm = TRUE;
          break;
        }
    }
  sym = OP_SYMBOL (IC_LEFT (ic));

  _G.omitFramePtr = options.oldralloc ? (!IS_GB && options.omitFramePtr) : should_omit_frame_ptr;

  if (!IS_GB && !z80_opts.noOmitFramePtr && !stackParm && !sym->stack)
    {
      if (!regalloc_dry_run)
        _G.omitFramePtr = TRUE;
    }
  else if (sym->stack)
    {
      if (!_G.omitFramePtr)
        emit2 ((optimize.codeSize && !IFFUNC_ISZ88DK_FASTCALL (ftype)) ? "!enters" : "!enter");
      if (IS_EZ80_Z80 && !_G.omitFramePtr && -sym->stack > -128 && -sym->stack <= -3 && !IFFUNC_ISZ88DK_FASTCALL (ftype))
        {
          emit2 ("lea hl, ix, !immed%d", -sym->stack);
          emit2 ("ld sp, hl");
          regalloc_dry_run_cost += 4;
        }
      else
        adjustStack (-sym->stack, !IS_TLCS90, TRUE, !IFFUNC_ISZ88DK_FASTCALL (ftype), !IY_RESERVED);
      _G.stack.pushed = 0;
    }
  else if (!_G.omitFramePtr)
    {
      emit2 ((optimize.codeSize && !IFFUNC_ISZ88DK_FASTCALL (ftype)) ? "!enters" : "!enter");
    }

  _G.stack.offset = sym->stack;
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  int retsize = getSize (sym->type->next);
  /* __critical __interrupt without an interrupt number is the non-maskable interrupt */
  bool is_nmi = (IS_Z80 || IS_Z180 || IS_EZ80_Z80) && IFFUNC_ISCRITICAL (sym->type) && FUNC_INTNO (sym->type) == INTNO_UNSPEC; 

  wassert (!regalloc_dry_run);
  wassertl (!_G.stack.pushed, "Unbalanced stack.");

  if (IFFUNC_ISNAKED (sym->type) || IFFUNC_ISNORETURN (sym->type))
    {
      emitDebug (IFFUNC_ISNAKED (sym->type) ? "; naked function: No epilogue." : "; _Noreturn function: No epilogue.");
      return;
    }

  wassertl(regalloc_dry_run || !IFFUNC_ISZ88DK_CALLEE(sym->type), "Unimplemented __z88dk_callee support on callee side");

  if (!IS_GB && !_G.omitFramePtr && sym->stack > (optimize.codeSize ? 2 : 1))
    {
      emit2 ("ld sp, ix");
      cost2 (2, 10, 7, 4, 0, 6, 2);
    }
  else
    adjustStack (_G.stack.offset, !IS_TLCS90, TRUE, retsize == 0 || retsize > 4, !IY_RESERVED);

  if(!IS_GB && !_G.omitFramePtr)
    emit2 ("pop ix");

  if (_G.calleeSaves.pushedDE)
    {
      emit2 ("pop de");
      _G.calleeSaves.pushedDE = FALSE;
    }

  if (_G.calleeSaves.pushedBC)
    {
      emit2 ("pop bc");
      _G.calleeSaves.pushedBC = FALSE;
    }

  if (options.profile)
    {
      emit2 ("!profileexit");
    }

  /* if this is an interrupt service routine
     then save all potentially used registers. */
  if (IFFUNC_ISISR (sym->type))
    emit2 ("!popa");
  else
    {
      /* This is a non-ISR function.
         If critical function then turn interrupts back on */
      if (IFFUNC_ISCRITICAL (sym->type))
        {
          if (IS_GB || IS_TLCS90)
            emit2 ("!ei");
          else if (IS_RAB)
            emit2 ("ipres");
          else
            {
              symbol *tlbl = newiTempLabel (NULL);
              //restore P/O flag
              emit2 ("pop af");
              //parity odd <==> P/O=0 <==> interrupt enable flag IFF2 was 0 <==>
              //don't enable interrupts as they were off before
              emit2 ("jp PO,!tlabel", labelKey2num (tlbl->key));
              emit2 ("!ei");
              emit2 ("!tlabeldef", labelKey2num (tlbl->key));
              genLine.lineCurr->isLabel = 1;
            }
        }
    }

  if (options.debug && currFunc)
    {
      debugFile->writeEndFunction (currFunc, ic, 1);
    }

  if (IFFUNC_ISISR (sym->type))
    {
      if (is_nmi)
        emit2 ("retn");
      else if (IS_RAB && IFFUNC_ISCRITICAL (sym->type) && FUNC_INTNO (sym->type) == INTNO_UNSPEC)
        {
          emit2 ("ipres");
          emit2 ("ret");
        }
      else if (IS_GB)
        emit2 (IFFUNC_ISCRITICAL (sym->type) ? "reti" : "ret");
      else
        {
          if (IFFUNC_ISCRITICAL (sym->type) && !is_nmi)
            emit2 ("!ei");
          emit2 ("reti");
        }
    }
  else
    {
      /* Both banked and non-banked just ret */
      emit2 ("ret");
    }

  _G.flushStatics = 1;
  _G.stack.pushed = 0;
  _G.stack.offset = 0;
}

/*-----------------------------------------------------------------*/
/* genRet - generate code for return statement                     */
/*-----------------------------------------------------------------*/
static void
genRet (const iCode *ic)
{
  /* Errk.  This is a hack until I can figure out how
     to cause dehl to spill on a call */
  int size, offset = 0;

  /* if we have no return value then
     just generate the "ret" */
  if (!IC_LEFT (ic))
    goto jumpret;

  /* we have something to return then
     move the return value into place */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));

  if (size == 2)
    {
      fetchPairLong (IS_GB ? PAIR_DE : PAIR_HL, AOP (IC_LEFT (ic)), ic, 0);
    }
  else if (size <= 4)
    {
      if (IS_GB && size == 4 && requiresHL (AOP (IC_LEFT (ic))))
        {
          fetchPairLong (PAIR_DE, AOP (IC_LEFT (ic)), 0, 0);
          fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), 0, 2);
        }
      else if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG)
        genMove_o (ASMOP_RETURN, 0, IC_LEFT (ic)->aop, 0, IC_LEFT (ic)->aop->size, true, true);
      else
        {
          bool skipbytes[4] = {false, false, false, false}; // Take care to not overwrite hl.
          for (offset = 0; offset < size; offset++)
            {
              if (requiresHL (AOP (IC_LEFT (ic))) && (ASMOP_RETURN->aopu.aop_reg[offset]->rIdx == H_IDX || ASMOP_RETURN->aopu.aop_reg[offset]->rIdx == L_IDX))
                {
                  skipbytes[offset] = true;
                  continue;
                }
              cheapMove (ASMOP_RETURN, offset, IC_LEFT (ic)->aop, offset, true);
            }
          for (offset = 0; offset < size; offset++)
            if (skipbytes[offset] && offset + 1 < size && ASMOP_RETURN->aopu.aop_reg[offset]->rIdx == L_IDX && ASMOP_RETURN->aopu.aop_reg[offset + 1]->rIdx == H_IDX)
              {
                fetchPairLong (PAIR_HL, IC_LEFT (ic)->aop, 0, offset);
                break;
              }
            else if (skipbytes[offset])
              {
                cheapMove (ASMOP_RETURN, offset, IC_LEFT (ic)->aop, offset, true);
              }
        }
    }
  else if (AOP_TYPE (IC_LEFT (ic)) == AOP_LIT)
    {
      unsigned long long lit = ullFromVal (AOP (IC_LEFT (ic))->aopu.aop_lit);
      setupPairFromSP (PAIR_HL, _G.stack.offset + _G.stack.param_offset + _G.stack.pushed + (_G.omitFramePtr || IS_GB ? 0 : 2));
      emit2 ("ld a, (hl)");
      emit2 ("inc hl");
      emit2 ("ld h, (hl)");
      emit2 ("ld l, a");
      regalloc_dry_run_cost += 8;
      do
        {
          emit2 ("ld (hl), !immedbyte", (unsigned long) (lit & 0xff));
          regalloc_dry_run_cost += 2;
          lit >>= 8;
          if (size > 1)
            {
              emit2 ("inc hl");
              regalloc_dry_run_cost++;
            }
        }
      while (--size);
    }
  else if (!IS_GB && AOP_TYPE (IC_LEFT (ic)) == AOP_STK || AOP_TYPE (IC_LEFT (ic)) == AOP_EXSTK
           || AOP_TYPE (IC_LEFT (ic)) == AOP_DIR || AOP_TYPE (IC_LEFT (ic)) == AOP_IY)
    {
      setupPairFromSP (PAIR_HL, _G.stack.offset + _G.stack.param_offset + _G.stack.pushed + (_G.omitFramePtr || IS_GB ? 0 : 2));
      emit2 ("ld e, (hl)");
      emit2 ("inc hl");
      emit2 ("ld d, (hl)");
      regalloc_dry_run_cost += 7;
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_STK || AOP_TYPE (IC_LEFT (ic)) == AOP_EXSTK)
        {
          int sp_offset, fp_offset;
          fp_offset =
            AOP (IC_LEFT (ic))->aopu.aop_stk + (AOP (IC_LEFT (ic))->aopu.aop_stk >
                0 ? _G.stack.param_offset : 0);
          sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
          emit2 ("ld hl, !immed%d", sp_offset);
          emit2 ("add hl, sp");
          regalloc_dry_run_cost += 4;
        }
      else
        {
          emit2 ("ld hl, !hashedstr", AOP (IC_LEFT (ic))->aopu.aop_dir);
          regalloc_dry_run_cost += 3;
        }
      emit2 ("ld bc, !immed%d", size);
      emit2 ("ldir");
      regalloc_dry_run_cost += 5;
    }
  else
    {
      setupPairFromSP (PAIR_HL, _G.stack.offset + _G.stack.param_offset + _G.stack.pushed + (_G.omitFramePtr || IS_GB ? 0 : 2));
      emit2 ("ld c, (hl)");
      emit2 ("inc hl");
      emit2 ("ld b, (hl)");
      regalloc_dry_run_cost += 7;
      spillPair (PAIR_HL);
      do
        {
          cheapMove (ASMOP_A, 0, AOP (IC_LEFT (ic)), offset++, true);
          emit2 ("ld (bc), a");
          regalloc_dry_run_cost++;
          if (size > 1)
            {
              emit2 ("inc bc");
              regalloc_dry_run_cost++;
            }
        }
      while (--size);
    }
  freeAsmop (IC_LEFT (ic), NULL);

jumpret:
  /* generate a jump to the return label
     if the next is not the return statement */
  if (!(ic->next && ic->next->op == LABEL && IC_LABEL (ic->next) == returnLabel))
    {
      if (!regalloc_dry_run)
        emit2 ("jp !tlabel", labelKey2num (returnLabel->key));
      regalloc_dry_run_cost += 3;
    }
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void
genLabel (const iCode * ic)
{
  /* special case never generate */
  if (IC_LABEL (ic) == entryLabel)
    return;

  emitLabelSpill (IC_LABEL (ic));
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a ljmp                                      */
/*-----------------------------------------------------------------*/
static void
genGoto (const iCode * ic)
{
  emit2 ("jp !tlabel", labelKey2num (IC_LABEL (ic)->key));
}

/*-----------------------------------------------------------------*/
/* genPlusIncr :- does addition with increment if possible         */
/*-----------------------------------------------------------------*/
static bool
genPlusIncr (const iCode * ic)
{
  unsigned int icount;
  unsigned int size = getDataSize (IC_RESULT (ic));
  PAIR_ID resultId = getPairId (AOP (IC_RESULT (ic)));

  /* will try to generate an increment */
  /* if the right side is not a literal
     we cannot */
  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    return FALSE;

  icount = (unsigned int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);

  /* If result is a pair */
  if (resultId != PAIR_INVALID)
    {
      bool delayed_move;
      if (isLitWord (AOP (IC_LEFT (ic))))
        {
          fetchLitPair (getPairId (AOP (IC_RESULT (ic))), AOP (IC_LEFT (ic)), icount);
          return TRUE;
        }
      if (isPair (AOP (IC_LEFT (ic))) && resultId == PAIR_HL && icount > 3)
        {
          if (getPairId (AOP (IC_LEFT (ic))) == PAIR_HL)
            {
              PAIR_ID freep = getDeadPairId (ic);
              if (freep != PAIR_INVALID)
                {
                  fetchPair (freep, AOP (IC_RIGHT (ic)));
                  emit2 ("add hl, %s", _pairs[freep].name);
                  regalloc_dry_run_cost += 1;
                  return TRUE;
                }
            }
          else
            {
              fetchPair (PAIR_HL, AOP (IC_RIGHT (ic)));
              emit2 ("add hl, %s", getPairName (AOP (IC_LEFT (ic))));
              regalloc_dry_run_cost += 1;
              return TRUE;
            }
        }
      if (icount > 5)
        return FALSE;
      /* Inc a pair */
      delayed_move = (getPairId (AOP (IC_RESULT (ic))) == PAIR_IY && getPairId (AOP (IC_LEFT (ic))) != PAIR_INVALID
                      && isPairDead (getPairId (AOP (IC_LEFT (ic))), ic));
      if (!sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
        {
          if (icount > 3)
            return FALSE;
          if (!delayed_move)
            fetchPair (getPairId (AOP (IC_RESULT (ic))), AOP (IC_LEFT (ic)));
        }
      while (icount--)
        {
          PAIR_ID pair = delayed_move ? getPairId (AOP (IC_LEFT (ic))) : getPairId (AOP (IC_RESULT (ic)));
          emit2 ("inc %s", _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IY ? 2 : 1);
        }
      if (delayed_move)
        fetchPair (getPairId (AOP (IC_RESULT (ic))), AOP (IC_LEFT (ic)));
      return TRUE;
    }

  if (!IS_GB && isLitWord (AOP (IC_LEFT (ic))) && size == 2 && isPairDead (PAIR_HL, ic))
    {
      fetchLitPair (PAIR_HL, AOP (IC_LEFT (ic)), icount);
      commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, FALSE);
      return TRUE;
    }

  if (icount > 4) // Not worth it if the sequence of inc gets too long.
    return false;

  if (icount > 1 && size == 1 && aopInReg (IC_LEFT (ic)->aop, 0, A_IDX)) // add a, #n is cheaper than sequence of inc a.
    return false;

  if (size == 2 && getPairId (AOP (IC_LEFT (ic))) != PAIR_INVALID && icount <= 3 && isPairDead (getPairId (AOP (IC_LEFT (ic))), ic))
    {
      PAIR_ID pair = getPairId (AOP (IC_LEFT (ic)));
      while (icount--)
        emit2 ("inc %s", _pairs[pair].name);
      commitPair (AOP (IC_RESULT (ic)), pair, ic, FALSE);
      return true;
    }

  if (size == 2 && icount <= 2 && isPairDead (PAIR_HL, ic) && !IS_GB &&
    (IC_LEFT (ic)->aop->type == AOP_HL || IC_LEFT (ic)->aop->type == AOP_IY))
    {
      fetchPair (PAIR_HL, AOP (IC_LEFT (ic)));
      while (icount--)
        emit2 ("inc hl");
      regalloc_dry_run_cost++;
      commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, FALSE);
      return true;
    }

  /* if increment 16 bits in register */
  if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))) && size > 1 && icount == 1)
    {
      int offset = 0;
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      while (size--)
        {
          if (size == 1 && getPairId_o (AOP (IC_RESULT (ic)), offset) != PAIR_INVALID)
            {
              emit2 ("inc %s", _pairs[getPairId_o (AOP (IC_RESULT (ic)), offset)].name);
              size--;
              offset += 2;
              break;
            }
          emit3_o (A_INC, AOP (IC_RESULT (ic)), offset++, 0, 0);
          if (size)
            {
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
              regalloc_dry_run_cost += 3;
            }
        }
      if (!regalloc_dry_run)
        (AOP_TYPE (IC_LEFT (ic)) == AOP_HL || IS_GB
         && AOP_TYPE (IC_LEFT (ic)) == AOP_STK) ? emitLabelSpill (tlbl) : emitLabel (tlbl);
      else if (AOP_TYPE (IC_LEFT (ic)) == AOP_HL)
        spillCached ();
      return TRUE;
    }

  /* if the sizes are greater than 1 then we cannot */
  if (AOP_SIZE (IC_RESULT (ic)) > 1 || AOP_SIZE (IC_LEFT (ic)) > 1)
    return FALSE;

  /* If the result is in a register then we can load then increment.
   */
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG)
    {
      cheapMove (AOP (IC_RESULT (ic)), LSB, AOP (IC_LEFT (ic)), LSB, true);
      while (icount--)
        emit3_o (A_INC, AOP (IC_RESULT (ic)), LSB, 0, 0);
      return TRUE;
    }

  /* we can if the aops of the left & result match or
     if they are in registers and the registers are the
     same */
  if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
    {
      while (icount--)
        emit3 (A_INC, AOP (IC_LEFT (ic)), 0);
      return TRUE;
    }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* outBitAcc - output a bit in acc                                 */
/*-----------------------------------------------------------------*/
static void
outBitAcc (operand * result)
{
  symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      wassertl (0, "Tried to write A into a bit");
    }
  else
    {
      if (!regalloc_dry_run)
        {
          emit2 ("jp Z, !tlabel", labelKey2num (tlbl->key));
          emit2 ("ld a, !one");
          emitLabel (tlbl);
        }
      regalloc_dry_run_cost += 5;
      outAcc (result);
    }
}

static bool
couldDestroyCarry (const asmop *aop)
{
  if (aop)
    {
      if (aop->type == AOP_EXSTK || aop->type == AOP_IY)
        {
          return TRUE;
        }
    }
  return FALSE;
}

static void
shiftIntoPair (PAIR_ID id, asmop *aop)
{
  wassertl (!IS_GB, "Not implemented for the GBZ80");

  emitDebug ("; Shift into pair");

  switch (id)
    {
    case PAIR_HL:
      setupPair (PAIR_HL, aop, 0);
      break;
    case PAIR_DE:
      _push (PAIR_DE);
      setupPair (PAIR_IY, aop, 0);
      emit2 ("push iy");
      emit2 ("pop %s", _pairs[id].name);
      break;
    case PAIR_IY:
      setupPair (PAIR_IY, aop, 0);
      break;
    default:
      wassertl (0, "Internal error - hit default case");
    }

  aop->type = AOP_PAIRPTR;
  aop->aopu.aop_pairId = id;
  _G.pairs[id].offset = 0;
  _G.pairs[id].last_type = aop->type;
}

static void
setupToPreserveCarry (asmop *result, asmop *left, asmop *right)
{
  wassert (left && right);

  if (!IS_GB)
    {
      if (couldDestroyCarry (right) && couldDestroyCarry (result))
        {
          shiftIntoPair (PAIR_HL, right);
          /* check result again, in case right == result */
          if (couldDestroyCarry (result))
            {
              if (couldDestroyCarry (left))
                shiftIntoPair (PAIR_DE, result);
              else
                shiftIntoPair (PAIR_IY, result);
            }
        }
      else if (couldDestroyCarry (right))
        {
          if (getPairId (result) == PAIR_HL)
            _G.preserveCarry = TRUE;
          else
            shiftIntoPair (PAIR_HL, right);
        }
      else if (couldDestroyCarry (result))
        {
          shiftIntoPair (PAIR_HL, result);
        }
    }
}

/*-----------------------------------------------------------------*/
/* genPlus - generates code for addition                           */
/*-----------------------------------------------------------------*/
static void
genPlus (iCode * ic)
{
  int size, i, offset = 0;
  signed char cached[2];
  bool premoved, started;
  asmop *leftop;
  asmop *rightop;
  symbol *tlbl = 0;

  /* special cases :- */

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RIGHT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, FALSE);

  /* Swap the left and right operands if:

     if literal, literal on the right or
     if left requires ACC or right is already
     in ACC */
  if ((AOP_TYPE (IC_LEFT (ic)) == AOP_LIT) || (AOP_NEEDSACC (IC_RIGHT (ic))) || aopInReg (IC_RIGHT (ic)->aop, 0, A_IDX))
    {
      operand *t = IC_RIGHT (ic);
      IC_RIGHT (ic) = IC_LEFT (ic);
      IC_LEFT (ic) = t;
    }

  leftop = IC_LEFT (ic)->aop;
  rightop = IC_RIGHT (ic)->aop;

  /* if both left & right are in bit
     space */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_CRY && AOP_TYPE (IC_RIGHT (ic)) == AOP_CRY)
    {
      /* Cant happen */
      wassertl (0, "Tried to add two bits");
    }

  /* if left in bit space & right literal */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_CRY && AOP_TYPE (IC_RIGHT (ic)) == AOP_LIT)
    {
      /* Can happen I guess */
      wassertl (0, "Tried to add a bit to a literal");
    }

  /* if I can do an increment instead
     of add then GOOD for ME */
  if (genPlusIncr (ic) == TRUE)
    goto release;

  size = getDataSize (IC_RESULT (ic));

  /* Special case when left and right are constant */
  if (isPair (AOP (IC_RESULT (ic))))
    {
      char *left = Safe_strdup (aopGetLitWordLong (AOP (IC_LEFT (ic)), 0, FALSE));
      const char *right = aopGetLitWordLong (AOP (IC_RIGHT (ic)), 0, FALSE);

      if (AOP_TYPE (IC_LEFT (ic)) == AOP_LIT && AOP_TYPE (IC_RIGHT (ic)) == AOP_LIT && left && right)
        {
          struct dbuf_s dbuf;

          /* It's a pair */
          /* PENDING: fix */
          dbuf_init (&dbuf, 128);
          dbuf_printf (&dbuf, "#(%s + %s)", left, right);
          Safe_free (left);
          emit2 ("ld %s, %s", getPairName (AOP (IC_RESULT (ic))), dbuf_c_str (&dbuf));
          dbuf_destroy (&dbuf);
          regalloc_dry_run_cost += (getPairId (AOP (IC_RESULT (ic))) == PAIR_IY ? 4 : 3);
          goto release;
        }
      Safe_free (left);
    }

  // eZ80 has lea.
  if (IS_EZ80_Z80 && isPair (IC_RESULT (ic)->aop) && getPairId (IC_LEFT (ic)->aop) == PAIR_IY && IC_RIGHT (ic)->aop->type == AOP_LIT)
    {
       int lit = (int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);
       if (lit >= -128 && lit < 128)
         {
           emit2 ("lea %s, iy, !immed%d", _pairs[getPairId (IC_RESULT (ic)->aop)].name, lit);
           regalloc_dry_run_cost += 3;
           spillPair (getPairId (IC_RESULT (ic)->aop));
           goto release;
         }
    }

  if ((isPair (AOP (IC_RIGHT (ic))) || isPair (AOP (IC_LEFT (ic)))) && getPairId (AOP (IC_RESULT (ic))) == PAIR_HL)
    {
      /* Fetch into HL then do the add */
      PAIR_ID left = getPairId (AOP (IC_LEFT (ic)));
      PAIR_ID right = getPairId (AOP (IC_RIGHT (ic)));

      spillPair (PAIR_HL);

      if (left == PAIR_HL && right != PAIR_INVALID)
        {
          emit2 ("add hl, %s", _pairs[right].name);
          regalloc_dry_run_cost += 1;
          goto release;
        }
      else if (right == PAIR_HL && left != PAIR_INVALID)
        {
          emit2 ("add hl, %s", _pairs[left].name);
          regalloc_dry_run_cost += 1;
          goto release;
        }
      else if (right != PAIR_INVALID && right != PAIR_HL)
        {
          fetchPair (PAIR_HL, AOP (IC_LEFT (ic)));
          emit2 ("add hl, %s", getPairName (AOP (IC_RIGHT (ic))));
          regalloc_dry_run_cost += 1;
          goto release;
        }
      else if (left != PAIR_INVALID && left != PAIR_HL)
        {
          fetchPair (PAIR_HL, AOP (IC_RIGHT (ic)));
          emit2 ("add hl, %s", getPairName (AOP (IC_LEFT (ic))));
          regalloc_dry_run_cost += 1;
          goto release;
        }
      else if (left == PAIR_HL && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)))
        {
          PAIR_ID pair = (isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC);
          fetchPair (pair, AOP (IC_RIGHT (ic)));
          emit2 ("add hl, %s", _pairs[pair].name);
          regalloc_dry_run_cost += 1;
          goto release;
        }
      else if (right == PAIR_HL && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)))
        {
          PAIR_ID pair = (isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC);
          fetchPair (pair, AOP (IC_LEFT (ic)));
          emit2 ("add hl, %s", _pairs[pair].name);
          regalloc_dry_run_cost += 1;
          goto release;
        }
      else
        {
          /* Can't do it */
        }
    }
  else if (getPairId (IC_RESULT (ic)->aop) == PAIR_HL && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)) && IC_RIGHT (ic)->aop->type == AOP_LIT)
    {
      PAIR_ID extrapair = isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC;
      fetchPair (PAIR_HL, IC_LEFT (ic)->aop);
      fetchPair (extrapair, IC_RIGHT (ic)->aop);
      emit2 ("add hl, %s", _pairs[extrapair].name);
      regalloc_dry_run_cost += 1;
      goto release;     
    }

  // Handle AOP_EXSTK conflict with hl here, since setupToPreserveCarry() would cause problems otherwise.
  if (IC_RESULT (ic)->aop->type == AOP_EXSTK && (getPairId (IC_LEFT (ic)->aop) == PAIR_HL || getPairId (IC_RIGHT (ic)->aop) == PAIR_HL) &&
    (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)) && isPairDead (PAIR_HL, ic))
    {
      PAIR_ID extrapair = isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC;
      fetchPair (extrapair, getPairId (IC_LEFT (ic)->aop) == PAIR_HL ? IC_RIGHT (ic)->aop : IC_LEFT (ic)->aop);
      emit2 ("add hl, %s", _pairs[extrapair].name);
      regalloc_dry_run_cost += 1;
      spillPair (PAIR_HL);
      commitPair (IC_RESULT (ic)->aop, PAIR_HL, ic, FALSE);
      goto release;
    }
  else if (getPairId (AOP (IC_RESULT (ic))) == PAIR_IY &&
    (getPairId (AOP (IC_LEFT (ic))) == PAIR_HL && isPair (AOP (IC_RIGHT (ic))) && getPairId (AOP (IC_RIGHT (ic))) != PAIR_IY || getPairId (AOP (IC_RIGHT (ic))) == PAIR_HL && isPair (AOP (IC_LEFT (ic))) && getPairId (AOP (IC_LEFT (ic))) != PAIR_IY) &&
    isPairDead (PAIR_HL, ic))
    {
      PAIR_ID pair = (getPairId (AOP (IC_LEFT (ic))) == PAIR_HL ? getPairId (AOP (IC_RIGHT (ic))) : getPairId (AOP (IC_LEFT (ic))));
      emit2 ("add hl, %s", _pairs[pair].name);
      _push (PAIR_HL);
      _pop (PAIR_IY);
      goto release;
    }
  else if (getPairId (AOP (IC_RESULT (ic))) == PAIR_IY)
    {
      bool save_pair = FALSE;
      PAIR_ID pair;

      if (getPairId (AOP (IC_RIGHT (ic))) == PAIR_IY || getPairId (AOP (IC_LEFT (ic))) == PAIR_BC
          || getPairId (AOP (IC_LEFT (ic))) == PAIR_DE || getPairId (AOP (IC_LEFT (ic))) != PAIR_IY
          && (AOP_TYPE (IC_RIGHT (ic)) == AOP_IMMD || AOP_TYPE (IC_RIGHT (ic)) == AOP_LIT))
        {
          operand *t = IC_RIGHT (ic);
          IC_RIGHT (ic) = IC_LEFT (ic);
          IC_LEFT (ic) = t;
        }
      pair = getPairId (AOP (IC_RIGHT (ic)));
      if (pair != PAIR_BC && pair != PAIR_DE)
        {
          if (AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == C_IDX
              && (!bitVectBitValue (ic->rSurv, B_IDX) || !isPairDead (PAIR_DE, ic)))
            pair = PAIR_BC;
          else if (AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == E_IDX
                   && (!bitVectBitValue (ic->rSurv, D_IDX) || !isPairDead (PAIR_BC, ic)))
            pair = PAIR_DE;
          else
            pair = isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC;
          if (!isPairDead (pair, ic))
            save_pair = TRUE;
        }
      fetchPair (PAIR_IY, AOP (IC_LEFT (ic)));
      if (save_pair)
        _push (pair);
      fetchPair (pair, AOP (IC_RIGHT (ic)));
      emit2 ("add iy, %s", _pairs[pair].name);
      spillPair (PAIR_IY);
      regalloc_dry_run_cost += 2;
      if (save_pair)
        _pop (pair);
      goto release;
    }

  /* gbz80 special case:
     ld hl,sp+n trashes C so we can't afford to do it during an
     add with stack based variables.  Worst case is:
     ld  hl,sp+left
     ld  a,(hl)
     ld  hl,sp+right
     add (hl)
     ld  hl,sp+result
     ld  (hl),a
     ld  hl,sp+left+1
     ld  a,(hl)
     ld  hl,sp+right+1
     adc (hl)
     ld  hl,sp+result+1
     ld  (hl),a
     So you can't afford to load up hl if either left, right, or result
     is on the stack (*sigh*)  The alt is:
     ld  hl,sp+left
     ld  de,(hl)
     ld  hl,sp+right
     ld  hl,(hl)
     add hl,de
     ld  hl,sp+result
     ld  (hl),hl
     Combinations in here are:
     * If left or right are in bc then the loss is small - trap later
     * If the result is in bc then the loss is also small
   */
  if (IS_GB)
    {
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_STK || AOP_TYPE (IC_RIGHT (ic)) == AOP_STK || AOP_TYPE (IC_RESULT (ic)) == AOP_STK)
        {
          if ((AOP_SIZE (IC_LEFT (ic)) == 2 ||
               AOP_SIZE (IC_RIGHT (ic)) == 2) && (AOP_SIZE (IC_LEFT (ic)) <= 2 && AOP_SIZE (IC_RIGHT (ic)) <= 2 || size == 2))
            {
              if (getPairId (AOP (IC_RIGHT (ic))) == PAIR_BC || getPairId (AOP (IC_RIGHT (ic))) == PAIR_DE)
                {
                  /* Swap left and right */
                  operand *t = IC_RIGHT (ic);
                  IC_RIGHT (ic) = IC_LEFT (ic);
                  IC_LEFT (ic) = t;
                }
              if (getPairId (AOP (IC_LEFT (ic))) == PAIR_BC)
                {
                  fetchPair (PAIR_HL, AOP (IC_RIGHT (ic)));
                  emit2 ("add hl, bc");
                  regalloc_dry_run_cost += 1;
                }
              else
                {
                  if (AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP_SIZE (IC_RIGHT (ic)) == 2 && AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP_SIZE (IC_LEFT (ic)) == 2)
                    {
                      const short dst[4] = { E_IDX, L_IDX, D_IDX, H_IDX };
                      short src[4];
                      if (AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == E_IDX
                          || AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == L_IDX)
                        {
                          src[0] = AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx;
                          src[1] = AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx;
                          src[2] = AOP (IC_RIGHT (ic))->aopu.aop_reg[1]->rIdx;
                          src[3] = AOP (IC_LEFT (ic))->aopu.aop_reg[1]->rIdx;
                        }
                      else
                        {
                          src[1] = AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx;
                          src[0] = AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx;
                          src[3] = AOP (IC_RIGHT (ic))->aopu.aop_reg[1]->rIdx;
                          src[2] = AOP (IC_LEFT (ic))->aopu.aop_reg[1]->rIdx;
                        }
                      regMove (dst, src, size, FALSE);
                    }
                  else if (AOP_TYPE (IC_RIGHT (ic)) == AOP_REG &&
                           (AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == E_IDX
                            || AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == D_IDX || AOP_SIZE (IC_RIGHT (ic)) == 2
                            && (AOP (IC_RIGHT (ic))->aopu.aop_reg[1]->rIdx == E_IDX
                                || AOP (IC_RIGHT (ic))->aopu.aop_reg[1]->rIdx == D_IDX)))
                    {
                      fetchPair (PAIR_DE, AOP (IC_RIGHT (ic)));
                      fetchPair (PAIR_HL, AOP (IC_LEFT (ic)));
                    }
                  else
                    {
                      fetchPair (PAIR_DE, AOP (IC_LEFT (ic)));
                      fetchPair (PAIR_HL, AOP (IC_RIGHT (ic)));
                    }
                  emit2 ("add hl, de");
                  regalloc_dry_run_cost += 1;
                }
              spillPair (PAIR_HL);
              commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, FALSE);
              goto release;
            }
        }
      if (size == 4)
        {
          /* Be paranoid on the GB with 4 byte variables due to how C
             can be trashed by lda hl,n(sp).
           */
          _gbz80_emitAddSubLong (ic, TRUE);
          goto release;
        }
    }

  // Avoid overwriting operand in h or l when setupToPreserveCarry () loads hl - only necessary if carry is actually used during addition.
  premoved = FALSE;
  if (size > 1 && !(size == 2 && (isPair (leftop) && rightop->type == AOP_LIT)))
    {
      if (!couldDestroyCarry (leftop) && (couldDestroyCarry (rightop) || couldDestroyCarry (AOP (IC_RESULT (ic)))))
        {
          cheapMove (ASMOP_A, 0, leftop, offset, true);
          premoved = TRUE;
        }

      setupToPreserveCarry (AOP (IC_RESULT (ic)), leftop, rightop);
    }
  // But if we don't actually want to use hl for the addition, it can make sense to setup an op to use cheaper hl instead of iy.
  if (size == 1 && !aopInReg(leftop, 0, H_IDX) && !aopInReg(leftop, 0, L_IDX) && isPairDead (PAIR_HL, ic))
    {
      if (couldDestroyCarry (AOP (IC_RESULT (ic))) &&
        (AOP (IC_RESULT (ic)) == leftop || AOP (IC_RESULT (ic)) == rightop))
        shiftIntoPair (PAIR_HL, AOP (IC_RESULT (ic)));
      else if (couldDestroyCarry (rightop))
        shiftIntoPair (PAIR_HL, rightop);
    }

  cached[0] = -1;
  cached[1] = -1;

  for (i = 0, started = FALSE; i < size;)
    {
      // Addition of interleaved pairs.
      if ((!premoved || i) && aopInReg (AOP (IC_RESULT (ic)), i, HL_IDX) && leftop->size - i >= 2 && rightop->size - i >= 2)
        {
          PAIR_ID pair = PAIR_INVALID;

          if (aopInReg (leftop, i, L_IDX) && aopInReg (rightop, i + 1, H_IDX))
            {
              if (aopInReg (leftop, i + 1, D_IDX) && aopInReg (rightop, i, E_IDX))
                pair = PAIR_DE;
              else if (aopInReg (leftop, i + 1, B_IDX) && aopInReg (rightop, i, C_IDX))
                pair = PAIR_BC;
            }
          else if (aopInReg (leftop, i + 1, H_IDX) && aopInReg (rightop, i, L_IDX))
            {
              if (aopInReg (leftop, i, E_IDX) && aopInReg (rightop, i + 1, D_IDX))
                pair = PAIR_DE;
              else if (aopInReg (leftop, i, C_IDX) && aopInReg (rightop, i + 1, B_IDX))
                pair = PAIR_BC;
            }

          if (pair != PAIR_INVALID)
            {
              if (started)
                {
                  emit2 ("adc hl, %s", _pairs[pair].name);
                  regalloc_dry_run_cost += 2;
                }
              else
                {
                  emit2 ("add hl, %s", _pairs[pair].name);
                  started = TRUE;
                  regalloc_dry_run_cost += 1;
                }
              i += 2;
              continue;
            }
        }

      if ((!premoved || i) && !started && i == size - 2 && !i && isPair (AOP (IC_RIGHT (ic))) && AOP_TYPE (IC_LEFT (ic)) == AOP_IMMD && getPairId (AOP (IC_RIGHT (ic))) != PAIR_HL
          && isPairDead (PAIR_HL, ic))
        {
          fetchPair (PAIR_HL, AOP (IC_LEFT (ic)));
          emit2 ("add hl, %s", getPairName (AOP (IC_RIGHT (ic))));
          started = TRUE;
          regalloc_dry_run_cost += 1;
          spillPair (PAIR_HL);
          commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, FALSE);
          i += 2;
        }
     else  if ((!premoved || i) && !started && i == size - 2 && !i && isPair (AOP (IC_LEFT (ic))) && (rightop->type == AOP_LIT  || rightop->type == AOP_IMMD) && getPairId (AOP (IC_LEFT (ic))) != PAIR_HL
          && isPairDead (PAIR_HL, ic))
        {
          fetchPair (PAIR_HL, AOP (IC_RIGHT (ic)));
          emit2 ("add hl, %s", getPairName (AOP (IC_LEFT (ic))));
          started = TRUE;
          regalloc_dry_run_cost += 1;
          spillPair (PAIR_HL);
          commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, FALSE);
          i += 2;
        }
      else if ((!premoved || i) && !started && i == size - 2 && !i && aopInReg (leftop, i, HL_IDX) && isPair (AOP (IC_RIGHT (ic))) && isPairDead (PAIR_HL, ic))
        {
          emit2 ("add hl, %s", getPairName (AOP (IC_RIGHT (ic))));
          started = TRUE;
          regalloc_dry_run_cost += 1;
          commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, FALSE);
          i += 2;
        }
      else if ((!premoved || i) && !started && i == size - 2 && !i && isPair (AOP (IC_LEFT (ic))) && aopInReg (rightop, i, HL_IDX) && isPairDead (PAIR_HL, ic))
        {
          emit2 ("add hl, %s", getPairName (AOP (IC_LEFT (ic))));
          started = TRUE;
          regalloc_dry_run_cost += 1;
          commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, FALSE);
          i += 2;
        }
      else if ((!premoved || i) && !started && i == size - 2 && aopInReg (AOP (IC_RESULT (ic)), i, HL_IDX) &&
          aopInReg (rightop, i, C_IDX) && !bitVectBitValue (ic->rSurv, B_IDX))
        {
          if (aopInReg (rightop, i + 1, H_IDX) || aopInReg (rightop, i + 1, L_IDX))
            {
              cheapMove (ASMOP_B, 0, AOP (IC_RIGHT (ic)), i + 1, true);
              fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), 0, i);
            }
          else
            {
              fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), 0, i);
              cheapMove (ASMOP_B, 0, AOP (IC_RIGHT (ic)), i + 1, true);
            }
          emit2 ("add hl, bc");
          started = TRUE;
          regalloc_dry_run_cost += 1;
          i += 2;
        }
      else if (!options.oldralloc && (!premoved || i) && !started && i == size - 2 && aopInReg (AOP (IC_RESULT (ic)), i, HL_IDX) &&
          aopInReg (leftop, i, C_IDX) && !bitVectBitValue (ic->rSurv, B_IDX))
        {
          if (aopInReg (leftop, i + 1, H_IDX) || aopInReg (leftop, i + 1, L_IDX))
            {
              cheapMove (ASMOP_B, 0, AOP (IC_LEFT (ic)), i + 1, true);
              fetchPairLong (PAIR_HL, AOP (IC_RIGHT (ic)), 0, i);
            }
          else
            {
              fetchPairLong (PAIR_HL, AOP (IC_RIGHT (ic)), 0, i);
              cheapMove (ASMOP_B, 0, AOP (IC_LEFT (ic)), i + 1, true);
            }
          emit2 ("add hl, bc");
          started = TRUE;
          regalloc_dry_run_cost += 1;
          i += 2;
        }
       else if (!options.oldralloc && (!premoved || i) && !started && i == size - 2 && aopInReg (AOP (IC_RESULT (ic)), i, HL_IDX) &&
          aopInReg (rightop, i, E_IDX) && !bitVectBitValue (ic->rSurv, D_IDX))
        {
          if (aopInReg (rightop, i + 1, H_IDX) || aopInReg (rightop, i + 1, L_IDX))
            {
              cheapMove (ASMOP_D, 0, AOP (IC_RIGHT (ic)), i + 1, true);
              fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), 0, i);
            }
          else
            {
              fetchPairLong (PAIR_HL, AOP (IC_LEFT (ic)), 0, i);
              cheapMove (ASMOP_D, 0, AOP (IC_RIGHT (ic)), i + 1, true);
            }
          emit2 ("add hl, de");
          started = TRUE;
          regalloc_dry_run_cost += 1;
          i += 2;
        }
       else if (!options.oldralloc && (!premoved || i) && !started && i == size - 2 && aopInReg (AOP (IC_RESULT (ic)), i, HL_IDX) && 
          aopInReg (leftop, i, E_IDX) && !bitVectBitValue (ic->rSurv, D_IDX))
        {
          if (aopInReg (leftop, i + 1, H_IDX) || aopInReg (leftop, i + 1, L_IDX))
            {
              cheapMove (ASMOP_D, 0, AOP (IC_LEFT (ic)), i + 1, true);
              fetchPairLong (PAIR_HL, AOP (IC_RIGHT (ic)), 0, i);
            }
          else
            {
              fetchPairLong (PAIR_HL, AOP (IC_RIGHT (ic)), 0, i);
              cheapMove (ASMOP_D, 0, AOP (IC_LEFT (ic)), i + 1, true);
            }
          emit2 ("add hl, de");
          started = TRUE;
          regalloc_dry_run_cost += 1;
          i += 2;
        }
      // When adding a literal, the 16 bit addition results in smaller, faster code than two 8-bit additions.
      else if ((!premoved || i) && aopInReg (AOP (IC_RESULT (ic)), i, HL_IDX) && aopInReg (leftop, i, HL_IDX) && (rightop->type == AOP_LIT && !aopIsLitVal (rightop, i, 1, 0) || rightop->type == AOP_IMMD))
        {
          PAIR_ID pair = getFreePairId (ic);
          bool pair_alive;
          if (pair == PAIR_INVALID)
            pair = PAIR_DE;
          if (pair_alive = !isPairDead (pair, ic))
            _push (pair);
          fetchPairLong (pair, AOP (IC_RIGHT (ic)), 0, i);
          if (started)
            {
              emit2 ("adc hl, %s", _pairs[pair].name);
              regalloc_dry_run_cost += 2;
            }
          else
            {
              emit2 ("add hl, %s", _pairs[pair].name);
              started = TRUE;
              regalloc_dry_run_cost += 1;
            }
          regalloc_dry_run_cost += 1;
          if (pair_alive)
            _pop (pair);
          i += 2;
        }
      // When adding registers the 16 bit addition results in smaller, faster code than an 8-bit addition.
      else if ((!premoved || i) && i == size - 1 && isPairDead (PAIR_HL, ic) && aopInReg (AOP (IC_RESULT (ic)), i, L_IDX)
        && (aopInReg (leftop, i, L_IDX) || aopInReg (rightop, i, L_IDX))
        && (aopInReg (leftop, i, C_IDX) || aopInReg (rightop, i, C_IDX) || aopInReg (leftop, i, E_IDX) || aopInReg (rightop, i, E_IDX)))
        {
          PAIR_ID pair = (leftop->aopu.aop_reg[i]->rIdx == C_IDX
                      || rightop->aopu.aop_reg[i]->rIdx == C_IDX) ? PAIR_BC : PAIR_DE;
          if (started)
            {
              emit2 ("adc hl, %s", _pairs[pair].name);
              regalloc_dry_run_cost += 2;
            }
          else
            {
              emit2 ("add hl, %s", _pairs[pair].name);
              started = TRUE;
              regalloc_dry_run_cost += 1;
            }
          i++;
        }
      // When adding a literal, the 16 bit addition results in smaller, slower code than an 8-bit addition.
      else if ((!premoved || i) && optimize.codeSize && !started && i == size - 1 && isPairDead (PAIR_HL, ic)
        && rightop->type == AOP_LIT && aopInReg (AOP (IC_RESULT (ic)), i, L_IDX) && aopInReg (leftop, i, L_IDX)
        && (!bitVectBitValue (ic->rSurv, C_IDX) || !bitVectBitValue (ic->rSurv, E_IDX)))
        {
          PAIR_ID pair = bitVectBitValue (ic->rSurv, C_IDX) ? PAIR_DE : PAIR_BC;
          emit2 ("ld %s, !immedbyte", _pairs[pair].l, ((unsigned int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit)) & 0xff);
          emit2 ("add hl, %s", _pairs[pair].name);
          started = TRUE;
          regalloc_dry_run_cost += 3;
          i++;
        }
      // Skip over this byte.
      else if (!premoved && !started && (leftop->type == AOP_REG || AOP (IC_RESULT (ic))->type == AOP_REG) && aopIsLitVal (rightop, i, 1, 0))
        {
          cheapMove (AOP (IC_RESULT (ic)), i, leftop, i, true);
          i++;
        }
      // Conditional 16-bit inc.
      else if (i == size - 2 && started && aopIsLitVal (rightop, i, 2, 0) && (
        aopInReg (AOP (IC_RESULT (ic)), i, BC_IDX) && aopInReg (leftop, i, BC_IDX) ||
        aopInReg (AOP (IC_RESULT (ic)), i, DE_IDX) && aopInReg (leftop, i, DE_IDX) ||
        aopInReg (AOP (IC_RESULT (ic)), i, HL_IDX) && aopInReg (leftop, i, HL_IDX) ||
        aopInReg (AOP (IC_RESULT (ic)), i, IY_IDX) && aopInReg (leftop, i, IY_IDX)))
        {
          PAIR_ID pair = getPairId_o (leftop, i);

          if (!tlbl && !regalloc_dry_run)
            tlbl = newiTempLabel (0);

          if (!regalloc_dry_run)
            emit2 ("jp NC, !tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 2; // Use cost of jr as the peephole optimizer can typically optimize this jp into jr. Do not emit jr directly to still allow jump-to-jump optimization.
          emit2 ("inc %s", _pairs[pair].name);
          regalloc_dry_run_cost += (1 + (pair == PAIR_IY));
          i += 2;
        }
      // Conditional 8-bit inc.
      else if (i == size - 1 && started && aopIsLitVal (rightop, i, 1, 0) &&
        !aopInReg (leftop, i, A_IDX) && // adc a, #0 is cheaper than conditional inc.
        (i < leftop->size &&
        leftop->type == AOP_REG && AOP (IC_RESULT (ic))->type == AOP_REG &&
        leftop->aopu.aop_reg[i]->rIdx == AOP (IC_RESULT (ic))->aopu.aop_reg[i]->rIdx &&
        leftop->aopu.aop_reg[i]->rIdx != IYL_IDX && leftop->aopu.aop_reg[i]->rIdx != IYH_IDX ||
        leftop->type == AOP_STK && leftop == AOP (IC_RESULT (ic)) ||
        leftop->type == AOP_PAIRPTR && leftop->aopu.aop_pairId == PAIR_HL))
        {
          if (!tlbl && !regalloc_dry_run)
            tlbl = newiTempLabel (0);

          if (!regalloc_dry_run)
            emit2 ("jp NC, !tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 2; // Use cost of jr as the peephole optimizer can typically optimize this jp into jr. Do not emit jr directly to still allow jump-to-jump optimization.
          emit3_o (A_INC, leftop, i, 0, 0);
          i++;
        }
      else
        {
          if (!premoved)
            cheapMove (ASMOP_A, 0, leftop, i, true);
          else
            premoved = FALSE;

          // Can't handle overwritten operand in hl.
          if (started && (IC_RESULT (ic)->aop->type == AOP_EXSTK ||  IC_RESULT (ic)->aop->type == AOP_PAIRPTR) && requiresHL (IC_RESULT (ic)->aop) &&
            (aopInReg (leftop, i, L_IDX) || aopInReg (leftop, i, H_IDX) || aopInReg (rightop, i, L_IDX) || aopInReg (rightop, i, H_IDX)))
            {
              wassert (regalloc_dry_run);
              regalloc_dry_run_cost += 1000;
            }

          if (!started && aopIsLitVal (rightop, i, 1, 0))
            ; // Skip over this byte.
          // We can use inc / dec only for the only, top non-zero byte, since it neither takes into account an existing carry nor does it update the carry.
          else if (!started && i == size - 1 && (aopIsLitVal (rightop, i, 1, 1) || aopIsLitVal (rightop, i, 1, 255)))
            {
              emit3 (aopIsLitVal (rightop, i, 1, 1) ? A_INC : A_DEC, ASMOP_A, 0);
              started = TRUE;
            }
          else
            {
              emit3_o (started ? A_ADC : A_ADD, ASMOP_A, 0, rightop, i);
              started = TRUE;
            }

          _G.preserveCarry = (i != size - 1);
          if (size &&
            (requiresHL (rightop) && rightop->type != AOP_REG || requiresHL (leftop)
            && leftop->type != AOP_REG) && AOP_TYPE (IC_RESULT (ic)) == AOP_REG
            && (AOP (IC_RESULT (ic))->aopu.aop_reg[i]->rIdx == L_IDX
              || AOP (IC_RESULT (ic))->aopu.aop_reg[i]->rIdx == H_IDX))
            {
              wassert (cached[0] == -1 || cached[1] == -1);
              cached[cached[0] == -1 ? 0 : 1] = offset++;
              _push (PAIR_AF);
            }
          else
            cheapMove (AOP (IC_RESULT (ic)), i, ASMOP_A, 0, true);
          i++;
        }
    }

  _G.preserveCarry = FALSE;

  if (tlbl)
    emitLabel (tlbl);

  for (size = 1; size >= 0; size--)
    if (cached[size] != -1)
      {
        _pop (PAIR_AF);
        cheapMove (AOP (IC_RESULT (ic)), cached[size], ASMOP_A, 0, true);
      }

release:
  _G.preserveCarry = FALSE;
  freeAsmop (IC_LEFT (ic), NULL);
  freeAsmop (IC_RIGHT (ic), NULL);
  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genSubDec :- does subtraction with decrement if possible        */
/*-----------------------------------------------------------------*/
static bool
genMinusDec (const iCode *ic, asmop *result, asmop *left, asmop *right)
{
  unsigned int icount;
  unsigned int size = getDataSize (IC_RESULT (ic));

  /* will try to generate a decrement */
  /* if the right side is not a literal we cannot */
  if (right->type != AOP_LIT)
    return FALSE;

  /* if the literal value of the right hand side
     is greater than 4 then it is not worth it */
  if ((icount = (unsigned int) ulFromVal (right->aopu.aop_lit)) > 2)
    return FALSE;

  size = getDataSize (IC_RESULT (ic));

  /* if decrement 16 bits in register */
  if (sameRegs (left, result) && (size > 1) && isPair (result))
    {
      while (icount--)
        emit2 ("dec %s", getPairName (result));
      return TRUE;
    }

  /* If result is a pair */
  if (isPair (AOP (IC_RESULT (ic))))
    {
      fetchPair (getPairId (result), left);
      while (icount--)
        if (!regalloc_dry_run)
          emit2 ("dec %s", getPairName (result));
      regalloc_dry_run_cost += 1;
      return TRUE;
    }

  /* if decrement 16 bits in register */
  if (sameRegs (left, result) && size == 2 && isPairDead (_getTempPairId (), ic) && !(requiresHL (left) && _getTempPairId () == PAIR_HL))
    {
      fetchPair (_getTempPairId (), left);

      while (icount--)
        if (!regalloc_dry_run)
          emit2 ("dec %s", _getTempPairName ());
      regalloc_dry_run_cost += 1;

      commitPair (result, _getTempPairId (), ic, FALSE);

      return TRUE;
    }


  /* if the sizes are greater than 1 then we cannot */
  if (result->size > 1 || left->size > 1)
    return FALSE;

  /* we can if the aops of the left & result match or if they are in
     registers and the registers are the same */
  if (sameRegs (left, result))
    {
      while (icount--)
        emit3 (A_DEC, result, 0);
      return TRUE;
    }

  if (result->type == AOP_REG)
    {
      cheapMove (result, 0, left, 0, true);
      while (icount--)
        emit3 (A_DEC, result, 0);
      return TRUE;
    }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* genSub - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
static void
genSub (const iCode *ic, asmop *result, asmop *left, asmop *right)
{
  int size, offset = 0;
  unsigned long long lit = 0L;

  /* special cases :- */
  /* if both left & right are in bit space */
  if (left->type == AOP_CRY && right->type == AOP_CRY)
    {
      wassertl (0, "Tried to subtract two bits");
      return;
    }

  /* if I can do an decrement instead of subtract then GOOD for ME */
  if (genMinusDec (ic, result, left, right) == TRUE)
    return;

  size = getDataSize (IC_RESULT (ic));

  if (right->type == AOP_LIT)
    {
      lit = ullFromVal (right->aopu.aop_lit);
      lit = -(long long) lit;
    }

  /* Same logic as genPlus */
  if (IS_GB)
    {
      if (left->type == AOP_STK || right->type == AOP_STK || AOP_TYPE (IC_RESULT (ic)) == AOP_STK)
        {
          if ((left->size == 2 ||
               right->size == 2) && (left->size <= 2 && right->size <= 2))
            {
              PAIR_ID leftp = getPairId (left);
              PAIR_ID rightp = getPairId (right);

              if (leftp == PAIR_INVALID && rightp == PAIR_INVALID)
                {
                  leftp = PAIR_DE;
                  rightp = PAIR_HL;
                }
              else if (rightp == PAIR_INVALID)
                rightp = PAIR_DE;
              else if (leftp == PAIR_INVALID)
                leftp = PAIR_DE;

              fetchPair (leftp, left);
              /* Order is important.  Right may be HL */
              fetchPair (rightp, right);

              if (!regalloc_dry_run)
                {
                  emit2 ("ld a, %s", _pairs[leftp].l);
                  emit2 ("sub a, %s", _pairs[rightp].l);
                  emit2 ("ld e, a");
                  emit2 ("ld a, %s", _pairs[leftp].h);
                  emit2 ("sbc a, %s", _pairs[rightp].h);
                }
              regalloc_dry_run_cost += 5;

              if (AOP_SIZE (IC_RESULT (ic)) > 1)
                cheapMove (AOP (IC_RESULT (ic)), 1, ASMOP_A, 0, true);
              cheapMove (AOP (IC_RESULT (ic)), 0, ASMOP_E, 0, true);
              return;
            }
        }
      if (size == 4)
        {
          /* Be paranoid on the GB with 4 byte variables due to how C
             can be trashed by lda hl,n(sp).
           */
          _gbz80_emitAddSubLongLong (ic, left, right, FALSE);
          return;
        }
    }

  setupToPreserveCarry (result, left, right);

  /* if literal right, add a, #-lit, else normal subb */
  while (size)
    {
      if (!IS_GB &&
        aopInReg (result, offset, HL_IDX) &&
        (aopInReg (left, offset, HL_IDX) || left->type == AOP_LIT || left->type == AOP_IY) &&
        (aopInReg (right, offset, BC_IDX) || aopInReg (right, offset, DE_IDX) || ((right->type == AOP_IY || right->type == AOP_HL) && getFreePairId (ic) != PAIR_INVALID)))
        {
          PAIR_ID rightpair;

          if (left->type == AOP_LIT || left->type == AOP_IY)
            fetchPairLong (PAIR_HL, left, ic, offset);
          if (right->type == AOP_IY || right->type == AOP_HL)
            {
              rightpair = getFreePairId (ic);
              fetchPairLong (rightpair, right, ic, offset);
            }
          else
            rightpair = getPartPairId (right, offset);

          if (!offset)
            emit3 (A_CP, ASMOP_A, ASMOP_A);
          emit2 ("sbc hl, %s", _pairs[rightpair].name);
          regalloc_dry_run_cost += 2;
          offset += 2;
          size -= 2;
          _G.preserveCarry = !!size;
          continue;
        }
      
      if (right->type != AOP_LIT)
        {    
          if (!offset)
            {
              if (left->type == AOP_LIT && byteOfVal (left->aopu.aop_lit, offset) == 0x00 && aopInReg (right, offset, A_IDX))
                emit3 (A_NEG, 0, 0);
              else
                {
                  if (left->type == AOP_LIT && byteOfVal (left->aopu.aop_lit, offset) == 0x00)
                    emit3 (A_XOR, ASMOP_A, ASMOP_A);
                  else
                    cheapMove (ASMOP_A, 0, left, offset, true);
                  emit3_o (A_SUB, ASMOP_A, 0, right, offset);
                }
            }
          else
            {
              cheapMove (ASMOP_A, 0, left, offset, true);
              emit3_o (A_SBC, ASMOP_A, 0, right, offset);
            }
        }
      else
        {
          cheapMove (ASMOP_A, 0, left, offset, true);

          /* first add without previous c */
          if (!offset)
            {
              if (size == 0 && (unsigned int) (lit & 0x0FFL) == 0xFF)
                emit3 (A_DEC, ASMOP_A, 0);
              else
                {
                  if (!regalloc_dry_run)
                    emit2 ("add a, !immedbyte", (unsigned int) (lit & 0x0FFL));
                  regalloc_dry_run_cost += 2;
                }
            }
          else
            emit2 ("adc a, !immedbyte", (unsigned int) ((lit >> (offset * 8)) & 0x0FFL));
        }
      size--;
      _G.preserveCarry = !!size;
      cheapMove (result, offset++, ASMOP_A, 0, true);
    }

  if (AOP_SIZE (IC_RESULT (ic)) == 3 && left->size == 3 && !sameRegs (result, left))
    {
      wassertl (0, "Tried to subtract on a long pointer");
    }
}

/*-----------------------------------------------------------------*/
/* genMinus - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
static void
genMinus (const iCode *ic)
{
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RIGHT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, FALSE);

  genSub (ic, AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic)), AOP (IC_RIGHT (ic)));

  _G.preserveCarry = FALSE;
  freeAsmop (IC_LEFT (ic), NULL);
  freeAsmop (IC_RIGHT (ic), NULL);
  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genUminusFloat - unary minus for floating points                */
/*-----------------------------------------------------------------*/
static void
genUminusFloat (operand *op, operand *result)
{
  emitDebug ("; genUminusFloat");

  /* for this we just need to flip the
     first bit then copy the rest in place */

  cheapMove (ASMOP_A, 0, AOP (op), MSB32, true);

  emit2 ("xor a,!immedbyte", 0x80);
  regalloc_dry_run_cost += 2;
  cheapMove (AOP (result), MSB32, ASMOP_A, 0, true);

  if (operandsEqu (result, op))
    return;

  genMove_o (result->aop, 0, op->aop, 0, AOP_SIZE (op) - 1, !aopInReg(result->aop, MSB32, A_IDX), false);
}

/*-----------------------------------------------------------------*/
/* genUminus - unary minus code generation                         */
/*-----------------------------------------------------------------*/
static void
genUminus (const iCode *ic)
{
  /* assign asmops */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, FALSE);

  /* if both in bit space then special
     case */
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY && AOP_TYPE (IC_LEFT (ic)) == AOP_CRY)
    {
      wassertl (0, "Left and right are in bit space");
      goto release;
    }

  if (IS_FLOAT (operandType (IC_LEFT (ic))))
    genUminusFloat (IC_LEFT (ic), IC_RESULT (ic));
  else
    genSub (ic, AOP (IC_RESULT (ic)), ASMOP_ZERO, AOP (IC_LEFT (ic)));

release:
  _G.preserveCarry = FALSE;
  freeAsmop (IC_LEFT (ic), NULL);
  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genMultOneChar - generates code for unsigned 8x8 multiplication */
/*-----------------------------------------------------------------*/
static void
genMultOneChar (const iCode * ic)
{
  symbol *tlbl1, *tlbl2;
  bool savedB = FALSE;

  asmop *result = AOP (IC_RESULT (ic));
  int resultsize = AOP_SIZE (IC_RESULT (ic));

  if (IS_GB)
    {
      wassertl (0, "Multiplication is handled through support function calls on gbz80");
      return;
    }

  if ((IS_Z180 || IS_EZ80_Z80) && AOP_TYPE (IC_RESULT (ic)) == AOP_REG)
    {
      if ((resultsize > 1 ? result->aopu.aop_reg[1]->rIdx == B_IDX : !bitVectBitValue (ic->rSurv, B_IDX))
          && result->aopu.aop_reg[0]->rIdx == C_IDX)
        {
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == C_IDX ||
              AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == B_IDX)
            {
              cheapMove (ASMOP_C, 0, AOP (IC_LEFT (ic)), LSB, true);
              cheapMove (ASMOP_B, 0, AOP (IC_RIGHT (ic)), LSB, true);
            }
          else
            {
              cheapMove (ASMOP_B, 0, AOP (IC_LEFT (ic)), LSB, true);
              cheapMove (ASMOP_C, 0, AOP (IC_RIGHT (ic)), LSB, true);
            }
          emit2 ("mlt bc");
          regalloc_dry_run_cost += 2;
          return;
        }
      if ((resultsize > 1 ? result->aopu.aop_reg[1]->rIdx == D_IDX : !bitVectBitValue (ic->rSurv, D_IDX))
          && result->aopu.aop_reg[0]->rIdx == E_IDX)
        {
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == E_IDX ||
              AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == D_IDX)
            {
              cheapMove (ASMOP_E, 0, AOP (IC_LEFT (ic)), LSB, true);
              cheapMove (ASMOP_D, 0, AOP (IC_RIGHT (ic)), LSB, true);
            }
          else
            {
              cheapMove (ASMOP_D, 0, AOP (IC_LEFT (ic)), LSB, true);
              cheapMove (ASMOP_E, 0, AOP (IC_RIGHT (ic)), LSB, true);
            }
          emit2 ("mlt de");
          regalloc_dry_run_cost += 2;
          return;
        }
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP_TYPE (IC_RIGHT (ic)) == AOP_REG &&
          ((AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == H_IDX && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == L_IDX ||
            AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == L_IDX && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == H_IDX) &&
           (resultsize > 1 ? result->aopu.aop_reg[1]->rIdx == H_IDX : !bitVectBitValue (ic->rSurv, H_IDX))
           && result->aopu.aop_reg[0]->rIdx == L_IDX))
        {
          emit2 ("mlt hl");
          regalloc_dry_run_cost += 2;
          return;
        }
    }

  if (IS_RAB && isPairDead (PAIR_HL, ic) && isPairDead (PAIR_BC, ic))
    {
      const bool save_de = (resultsize > 1 && bitVectBitValue (ic->rSurv, D_IDX) ||
        bitVectBitValue (ic->rSurv, E_IDX) && !(AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == E_IDX) && !(AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == E_IDX));
      if (save_de)
        _push (PAIR_DE);

      if (AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == E_IDX)
         cheapMove (ASMOP_C, 0, AOP (IC_LEFT (ic)), 0, true);
      else if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == E_IDX)
         cheapMove (ASMOP_C, 0, AOP (IC_RIGHT (ic)), 0, true);
      else if (AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == C_IDX)
         cheapMove (ASMOP_E, 0, AOP (IC_LEFT (ic)), 0, true);
      else if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == C_IDX)
         cheapMove (ASMOP_E, 0, AOP (IC_RIGHT (ic)), 0, true);
      else 
        {
          cheapMove (ASMOP_C, 0, AOP (IC_LEFT (ic)), 0, true);
          cheapMove (ASMOP_E, 0, AOP (IC_RIGHT (ic)), 0, true);
        }

      if (resultsize > 1)
        {
          cheapMove (ASMOP_D, 0, ASMOP_ZERO, 0, true);
          cheapMove (ASMOP_B, 0, ASMOP_D, 0, true);
        }

      emit2 ("mul");
      regalloc_dry_run_cost++;

      if (resultsize > 1)
        commitPair (result, PAIR_BC, ic, FALSE);
      else
        cheapMove (result, 0, ASMOP_C, 0, true);

      if (save_de)
        _pop (PAIR_DE);
      return;
    }

  if (!isPairDead (PAIR_DE, ic))
    {
      _push (PAIR_DE);
      _G.stack.pushedDE = TRUE;
    }
  if (IS_RAB && !isPairDead (PAIR_BC, ic) ||
      !(IS_Z180 || IS_EZ80_Z80) && (!options.oldralloc && bitVectBitValue (ic->rSurv, B_IDX) ||
                   options.oldralloc && bitVectBitValue (ic->rMask, B_IDX) && !(getPairId (AOP (IC_RESULT (ic))) == PAIR_BC)))
    {
      _push (PAIR_BC);
      savedB = TRUE;
    }

  // genMult() already swapped operands if necessary.
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == E_IDX ||
      AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[0]->rIdx == H_IDX
      && !requiresHL (AOP (IC_LEFT (ic))))
    {
      cheapMove (ASMOP_E, 0, AOP (IC_LEFT (ic)), 0, true);
      cheapMove (ASMOP_H, 0, AOP (IC_RIGHT (ic)), 0, true);
    }
  else
    {
      cheapMove (ASMOP_E, 0, AOP (IC_RIGHT (ic)), 0, true);
      cheapMove (ASMOP_H, 0, AOP (IC_LEFT (ic)), 0, true);
    }

  if (IS_Z180 || IS_EZ80_Z80)
    {
      emit2 ("ld l, e");
      emit2 ("mlt hl");
      regalloc_dry_run_cost += 3;
    }
  else if (IS_RAB)
    {
      emit2 ("ld c, h");
      emit2 ("ld d, !immedbyte", 0x00);
      emit2 ("ld b, d");
      emit2 ("mul");
      emit2 ("ld l, c");
      emit2 ("ld h, b");
      regalloc_dry_run_cost += 7;
    }
  else if (!regalloc_dry_run)
    {
      tlbl1 = newiTempLabel (NULL);
      tlbl2 = newiTempLabel (NULL);
      emit2 ("ld l, !immedbyte", 0x00);
      emit2 ("ld d, l");
      emit2 ("ld b, !immedbyte", 0x08);
      emitLabel (tlbl1);
      emit2 ("add hl, hl");
      emit2 ("jp NC, !tlabel", labelKey2num (tlbl2->key));
      emit2 ("add hl, de");
      emitLabel (tlbl2);
      emit2 ("djnz !tlabel", labelKey2num (tlbl1->key));
      regalloc_dry_run_cost += 12;
    }
  else
    regalloc_dry_run_cost += 12;


  spillPair (PAIR_HL);

  if (savedB)
    {
      _pop (PAIR_BC);
    }
  if (_G.stack.pushedDE)
    {
      _pop (PAIR_DE);
      _G.stack.pushedDE = FALSE;
    }

  if (result->type != AOP_HL)
    {
      if (resultsize == 1)
        cheapMove (result, 0, ASMOP_L, 0, true);
      else
        commitPair (result, PAIR_HL, ic, FALSE);
    }
  else
    {
      if (resultsize == 1)
        {
          emit2 ("ld a, l");
          regalloc_dry_run_cost += 1;
          cheapMove (result, 0, ASMOP_A, 0, true);
        }
      else
        {
          if (!isPairDead (PAIR_DE, ic))
            {
              _push (PAIR_DE);
              _G.stack.pushedDE = TRUE;
            }
          emit2 ("ld e, l");
          emit2 ("ld d, h");
          regalloc_dry_run_cost += 2;
          commitPair (result, PAIR_DE, ic, FALSE);
          if (!isPairDead (PAIR_DE, ic))
            {
              _pop (PAIR_DE);
              _G.stack.pushedDE = FALSE;
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* genMult - generates code for multiplication                     */
/*-----------------------------------------------------------------*/
static void
genMult (iCode * ic)
{
  int val;
  int count, i;
  /* If true then the final operation should be a subtract */
  bool active = FALSE;
  bool byteResult;
  bool add_in_hl = FALSE;
  int a_cost = 0, l_cost = 0;
  PAIR_ID pair;

  /* Shouldn't occur - all done through function calls */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RIGHT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, FALSE);

  byteResult = (AOP_SIZE (IC_RESULT (ic)) == 1);

  if (AOP_SIZE (IC_LEFT (ic)) > 2 || AOP_SIZE (IC_RIGHT (ic)) > 2 || AOP_SIZE (IC_RESULT (ic)) > 2)
    wassertl (0, "Large multiplication is handled through support function calls.");

  /* Swap left and right such that right is a literal */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_LIT)
    {
      operand *t = IC_RIGHT (ic);
      IC_RIGHT (ic) = IC_LEFT (ic);
      IC_LEFT (ic) = t;
    }

  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    {
      genMultOneChar (ic);
      goto release;
    }

  wassertl (AOP_TYPE (IC_RIGHT (ic)) == AOP_LIT, "Right must be a literal.");

  val = (int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);
  wassertl (val != 1, "Can't multiply by 1");

  // Try to use mlt.
  if ((IS_Z180 || IS_EZ80_Z80) && AOP_SIZE (IC_LEFT (ic)) == 1 && AOP_SIZE (IC_RIGHT (ic)) == 1 &&
    (byteResult || SPEC_USIGN (getSpec (operandType (IC_LEFT (ic)))) && SPEC_USIGN (getSpec (operandType (IC_RIGHT (ic))))))
    {
      pair = getPairId (AOP (IC_RESULT (ic)));
      if (pair == PAIR_INVALID && AOP_TYPE (IC_RESULT (ic)) == AOP_REG)
        {
          if (!bitVectBitValue (ic->rSurv, H_IDX) && AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == L_IDX)
            pair = PAIR_HL;
          else if (!bitVectBitValue (ic->rSurv, D_IDX) && AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == E_IDX)
            pair = PAIR_HL;
          else if (!bitVectBitValue (ic->rSurv, B_IDX) && AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == C_IDX)
            pair = PAIR_HL;
        }
      else if (pair == PAIR_INVALID)
        pair = getDeadPairId (ic);

      if (pair == PAIR_INVALID)
        {
          if (!(AOP_TYPE (IC_RESULT (ic)) == AOP_REG &&
            (AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == L_IDX || AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == H_IDX ||
            !byteResult && (AOP (IC_RESULT (ic))->aopu.aop_reg[1]->rIdx == L_IDX || AOP (IC_RESULT (ic))->aopu.aop_reg[1]->rIdx == H_IDX))))
            pair = PAIR_HL;
          else if (!(AOP_TYPE (IC_RESULT (ic)) == AOP_REG &&
            (AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == E_IDX || AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == D_IDX ||
            !byteResult && (AOP (IC_RESULT (ic))->aopu.aop_reg[1]->rIdx == E_IDX || AOP (IC_RESULT (ic))->aopu.aop_reg[1]->rIdx == D_IDX))))
            pair = PAIR_DE;
          else
            pair = PAIR_BC;
        }

      // For small operands under low register pressure, the standard approach is better than the mlt one.
      if (byteResult && val <= 6 && isPairDead (PAIR_HL, ic) && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)) &&
        !(AOP_TYPE (IC_RESULT (ic)) == AOP_REG && (AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == E_IDX || AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == C_IDX)))
        goto no_mlt;

      if (!isPairDead (pair, ic))
        _push (pair);

      switch (pair)
        {
        case PAIR_HL:
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == H_IDX)
            cheapMove (ASMOP_L, 0, AOP (IC_RIGHT (ic)), 0, true);
          else
            {
              cheapMove (ASMOP_L, 0, AOP (IC_LEFT (ic)), 0, true);
              cheapMove (ASMOP_H, 0, AOP (IC_RIGHT (ic)), 0, true);
            }
          break;
        case PAIR_DE:
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == D_IDX)
            cheapMove (ASMOP_E, 0, AOP (IC_RIGHT (ic)), 0, true);
          else
            {
              cheapMove (ASMOP_E, 0, AOP (IC_LEFT (ic)), 0, true);
              cheapMove (ASMOP_D, 0, AOP (IC_RIGHT (ic)), 0, true);
            }
          break;
        default:
          wassert (pair == PAIR_BC);
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == B_IDX)
            cheapMove (ASMOP_C, 0, AOP (IC_RIGHT (ic)), 0, true);
          else
            {
              cheapMove (ASMOP_C, 0, AOP (IC_LEFT (ic)), 0, true);
              cheapMove (ASMOP_B, 0, AOP (IC_RIGHT (ic)), 0, true);
            }
          break;
        }

      emit2 ("mlt %s", _pairs[pair].name);
      regalloc_dry_run_cost += 2;

      if (byteResult)
        cheapMove (AOP (IC_RESULT (ic)), 0, pair == PAIR_HL ? ASMOP_L : (pair == PAIR_DE ? ASMOP_E : ASMOP_C), 0, true);
      else
        commitPair (AOP (IC_RESULT (ic)), pair, ic, FALSE);

      if (!isPairDead (pair, ic))
        _pop (pair);

      goto release;
    }
no_mlt:

  pair = PAIR_DE;
  if (getPairId (AOP (IC_LEFT (ic))) == PAIR_BC ||
    (byteResult || !bitVectBitValue (ic->rSurv, B_IDX)) && AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == C_IDX)
    pair = PAIR_BC;
  if (isPairDead (PAIR_BC, ic) && !(AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == E_IDX))
    pair = PAIR_BC;

  if (pair == PAIR_DE && (byteResult ? bitVectBitValue (ic->rSurv, E_IDX) : !isPairDead (PAIR_DE, ic)))
    {
      _push (PAIR_DE);
      _G.stack.pushedDE = TRUE;
    }

  /* Use 16-bit additions even for 8-bit result when the operands are in the right places. */
  if (byteResult)
    {
      if (!aopInReg (IC_LEFT (ic)->aop, 0, A_IDX))
        a_cost += ld_cost (ASMOP_A, AOP (IC_LEFT (ic)));
      if (!aopInReg (IC_RESULT (ic)->aop, 0, A_IDX))
        a_cost += ld_cost (AOP (IC_RESULT (ic)), ASMOP_A);
      if (AOP_TYPE (IC_LEFT (ic)) != AOP_REG || AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx != L_IDX)
        l_cost += ld_cost (ASMOP_L, AOP (IC_LEFT (ic)));
      if (AOP_TYPE (IC_RESULT (ic)) != AOP_REG || AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx != L_IDX)
        l_cost += ld_cost (AOP (IC_RESULT (ic)), ASMOP_L);
    }
  add_in_hl = (!byteResult || isPairDead (PAIR_HL, ic) && l_cost < a_cost);

  if (byteResult)
    {
      cheapMove (add_in_hl ? ASMOP_L : ASMOP_A, 0, AOP (IC_LEFT (ic)), 0, true);
      if (AOP_TYPE (IC_LEFT (ic)) != AOP_REG || AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx != (pair == PAIR_BC ? C_IDX : E_IDX))
        cheapMove (pair == PAIR_BC ? ASMOP_C : ASMOP_E, 0, add_in_hl ? ASMOP_L : ASMOP_A, 0, true);
    }
  else if (AOP_SIZE (IC_LEFT (ic)) == 1 && !SPEC_USIGN (getSpec (operandType (IC_LEFT (ic)))))
    {
      cheapMove (pair == PAIR_BC ? ASMOP_C : ASMOP_E, 0, AOP (IC_LEFT (ic)), 0, true);
      emit2 ("ld a, %s", _pairs[pair].l);
      emit2 ("rlc a");
      emit2 ("sbc a, a");
      emit2 ("ld %s, a", _pairs[pair].h);
      regalloc_dry_run_cost += 5;
      emit2 ("ld l, %s", _pairs[pair].l);
      emit2 ("ld h, %s", _pairs[pair].h);
      regalloc_dry_run_cost += 2;
    }
  else
    {
      fetchPair (pair, AOP (IC_LEFT (ic)));
      if (getPairId (AOP (IC_LEFT (ic))) != PAIR_HL)
        {
          emit2 ("ld l, %s", _pairs[pair].l);
          emit2 ("ld h, %s", _pairs[pair].h);
          regalloc_dry_run_cost += 2;
        }
    }

  i = val;

  for (count = 0; count < 16; count++)
    {
      if (count != 0 && active)
        {
          if (!add_in_hl)
            emit2 ("add a, a");
          else
            emit2 ("add hl, hl");
          regalloc_dry_run_cost += 1;
        }
      if (i & 0x8000U)
        {
          if (active)
            {
              if (!add_in_hl)
                emit2 ("add a, %s", _pairs[pair].l);
              else
                emit2 ("add hl, %s", _pairs[pair].name);
              regalloc_dry_run_cost += 1;
            }
          active = TRUE;
        }
      i <<= 1;
    }

  spillPair (PAIR_HL);

  if (_G.stack.pushedDE)
    {
      _pop (PAIR_DE);
      _G.stack.pushedDE = FALSE;
    }

  if (byteResult)
    cheapMove (AOP (IC_RESULT (ic)), 0, add_in_hl ? ASMOP_L : ASMOP_A, 0, true);
  else
    commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, FALSE);

release:
  freeAsmop (IC_LEFT (ic), NULL);
  freeAsmop (IC_RIGHT (ic), NULL);
  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genDiv - generates code for division                            */
/*-----------------------------------------------------------------*/
static void
genDiv (const iCode * ic)
{
  /* Shouldn't occur - all done through function calls */
  wassertl (0, "Division is handled through support function calls");
}

/*-----------------------------------------------------------------*/
/* genMod - generates code for division                            */
/*-----------------------------------------------------------------*/
static void
genMod (const iCode * ic)
{
  /* Shouldn't occur - all done through function calls */
  wassert (0);
}

/*-----------------------------------------------------------------*/
/* genIfxJump :- will create a jump depending on the ifx           */
/*-----------------------------------------------------------------*/
static void
genIfxJump (iCode * ic, char *jval)
{
  symbol *jlbl;
  const char *inst;

  /* if true label then we jump if condition
     supplied is true */
  if (IC_TRUE (ic))
    {
      jlbl = IC_TRUE (ic);
      if (!strcmp (jval, "a"))
        {
          emit3 (A_OR, ASMOP_A, ASMOP_A);
          inst = "NZ";
        }
      else if (!strcmp (jval, "z"))
        {
          inst = "Z";
        }
      else if (!strcmp (jval, "nz"))
        {
          inst = "NZ";
        }
      else if (!strcmp (jval, "c"))
        {
          inst = "C";
        }
      else if (!strcmp (jval, "nc"))
        {
          inst = "NC";
        }
      else if (!strcmp (jval, "m"))
        {
          inst = "M";
        }
      else if (!strcmp (jval, "p"))
        {
          inst = "P";
        }
      else if (!strcmp (jval, "po"))
        {
          inst = "PO";
        }
      else if (!strcmp (jval, "pe"))
        {
          inst = "PE";
        }
      else
        {
          /* The buffer contains the bit on A that we should test */
          emit2 ("bit %s, a", jval);
          regalloc_dry_run_cost += 2;
          inst = "NZ";
        }
    }
  else
    {
      /* false label is present */
      jlbl = IC_FALSE (ic);
      if (!strcmp (jval, "a"))
        {
          emit3 (A_OR, ASMOP_A, ASMOP_A);
          inst = "Z";
        }
      else if (!strcmp (jval, "z"))
        {
          inst = "NZ";
        }
      else if (!strcmp (jval, "nz"))
        {
          inst = "Z";
        }
      else if (!strcmp (jval, "c"))
        {
          inst = "NC";
        }
      else if (!strcmp (jval, "nc"))
        {
          inst = "C";
        }
      else if (!strcmp (jval, "m"))
        {
          inst = "P";
        }
      else if (!strcmp (jval, "p"))
        {
          inst = "M";
        }
      else if (!strcmp (jval, "po"))
        {
          inst = "PE";
        }
      else if (!strcmp (jval, "pe"))
        {
          inst = "PO";
        }
      else
        {
          /* The buffer contains the bit on A that we should test */
          emit2 ("bit %s, a", jval);
          regalloc_dry_run_cost += 2;
          inst = "Z";
        }
    }
  /* Z80 can do a conditional long jump */
  if (!regalloc_dry_run)
    emit2 ("jp %s, !tlabel", inst, labelKey2num (jlbl->key));
  regalloc_dry_run_cost += 3;

  /* mark the icode as generated */
  if (!regalloc_dry_run)
    ic->generated = 1;
}

#if DISABLED
static const char *
_getPairIdName (PAIR_ID id)
{
  return _pairs[id].name;
}
#endif

/** Generic compare for > or <
 */
static void
genCmp (operand * left, operand * right, operand * result, iCode * ifx, int sign, const iCode * ic)
{
  int size, offset = 0;
  unsigned long long lit = 0ull;
  bool result_in_carry = FALSE;
  int a_always_byte = -1;

  /* if left & right are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      /* Can't happen on the Z80 */
      wassertl (0, "Tried to compare two bits");
    }
  else
    {
      /* Do a long subtract of right from left. */
      size = max (AOP_SIZE (left), AOP_SIZE (right));

      if (AOP_TYPE (right) == AOP_SFR)  /* Avoid overwriting A */
        {
          bool save_a, save_b, save_bc;
          wassertl (size == 1, "Right side sfr in comparison with more than 8 bits.");

          save_b = bitVectBitValue (ic->rSurv, B_IDX);
          save_bc = (save_b && bitVectBitValue (ic->rSurv, C_IDX));
          save_a = (aopInReg (left->aop, 0, A_IDX) ||
                    aopInReg (left->aop, 0, B_IDX) && save_b ||
                    aopInReg (left->aop, 0, C_IDX) && !save_b && save_bc);

          if (save_bc)
            _push (PAIR_BC);
          if (save_a)
            {
              cheapMove (ASMOP_A, 0, AOP (right), 0, true);
              _push (PAIR_AF);
            }
          else
            cheapMove (ASMOP_A, 0, AOP (right), 0, true);
          cheapMove (save_b ? ASMOP_C : ASMOP_B, 0, ASMOP_A, 0, true);
          if (save_a)
            _pop (PAIR_AF);
          else
            cheapMove (ASMOP_A, 0, AOP (left), 0, true);
          emit3_o (A_SUB, ASMOP_A, 0, save_b ? ASMOP_C : ASMOP_B, offset);
          if (save_bc)
            _pop (PAIR_BC);
          result_in_carry = TRUE;
          goto fix;
        }

      // Preserve A if necessary
      if (ifx && size == 1 && !sign && aopInReg (left->aop, 0, A_IDX) && bitVectBitValue (ic->rSurv, A_IDX) &&
        (AOP_TYPE (right) == AOP_LIT || AOP_TYPE (right) == AOP_REG && AOP (right)->aopu.aop_reg[offset]->rIdx != IYL_IDX && AOP (right)->aopu.aop_reg[offset]->rIdx != IYH_IDX || AOP_TYPE (right) == AOP_STK))
        {
          emit3 (A_CP, ASMOP_A, AOP (right));
          result_in_carry = TRUE;
          goto release;
        }

      // On the Gameboy we can't afford to adjust HL as it may trash the carry.
      if (size > 1 && (IS_GB || IY_RESERVED) && left->aop->type != AOP_REG && right->aop->type != AOP_REG && (requiresHL (AOP (right)) && requiresHL (AOP (left))))
        {
          if (!isPairDead (PAIR_DE, ic))
            _push (PAIR_DE);

          pointPairToAop (PAIR_DE, left->aop, 0);
          pointPairToAop (PAIR_HL, right->aop, 0);

          while (size--)
            {
              emit2 ("ld a, (de)");
              emit2 ("%s a, (hl)", offset == 0 ? "sub" : "sbc");
              regalloc_dry_run_cost += 2;

              if (size != 0)
                {
                  emit2 ("inc hl");
                  emit2 ("inc de");
                  regalloc_dry_run_cost += 2;
                }
              offset++;
            }
          if (sign && IS_GB)
            {
              wassert(isPairDead (PAIR_DE, ic));
              emit2 ("ld a, (de)");
              emit2 ("ld d, a");
              emit2 ("ld e, (hl)");
              regalloc_dry_run_cost += 3;
            }

          spillPair (PAIR_DE);
          if (!isPairDead (PAIR_DE, ic))
            _pop (PAIR_DE);

          spillPair (PAIR_HL);
          result_in_carry = TRUE;
          goto fix;
        }
      else if (size > 1 && IS_GB && (requiresHL (AOP (right)) && !requiresHL (AOP (left))))
        {
          if (!regalloc_dry_run)
            aopGet (AOP (right), LSB, FALSE);

          while (size--)
            {
              cheapMove (ASMOP_A, 0, AOP (left), offset, true);
              emit2 ("%s a, (hl)", offset == 0 ? "sub" : "sbc");
              regalloc_dry_run_cost += 1;

              if (size != 0)
                {
                  emit2 ("inc hl");
                  regalloc_dry_run_cost += 1;
                }
              offset++;
            }
          if (sign)
            {
              cheapMove (ASMOP_A, 0, AOP (left), offset - 1, true);
              emit2 ("ld d, a");
              emit2 ("ld e, (hl)");
              regalloc_dry_run_cost += 2;
            }
          spillPair (PAIR_HL);
          result_in_carry = TRUE;
          goto fix;
        }
      else if (size > 1 && IS_GB && (!requiresHL (AOP (right)) && requiresHL (AOP (left))))
        {
          if (!regalloc_dry_run)
            aopGet (AOP (left), LSB, FALSE);

          while (size--)
            {
              emit2 ("ld a, (hl)");
              regalloc_dry_run_cost += 1;
              emit3_o (offset == 0 ? A_SUB : A_SBC, ASMOP_A, 0, AOP (right), offset);

              if (size != 0)
                {
                  emit2 ("inc hl");
                  regalloc_dry_run_cost += 1;
                }
              offset++;
            }
          if (sign)
            {
              emit2 ("ld d, (hl)");
              regalloc_dry_run_cost += 1;
              cheapMove (ASMOP_A, 0, AOP (right), offset - 1, true);
              emit2 ("ld e, a");
              regalloc_dry_run_cost += 1;
            }
          spillPair (PAIR_HL);
          result_in_carry = TRUE;
          goto fix;
        }

      if (IS_GB && sign && AOP_TYPE (right) != AOP_LIT)
        {
          cheapMove (ASMOP_A, 0, AOP (right), size - 1, true);
          cheapMove (ASMOP_E, 0, ASMOP_A, 0, true);
          cheapMove (ASMOP_A, 0, AOP (left), size - 1, true);
          cheapMove (ASMOP_D, 0, ASMOP_A, 0, true);
        }

      if (AOP_TYPE (right) == AOP_LIT)
        {
          lit = ullFromVal (AOP (right)->aopu.aop_lit);

          /* optimize if(x < 0) or if(x >= 0) */
          if (lit == 0ull)
            {
              if (!sign)
                {
                  /* No sign so it's always false */
                  emit3 (A_CP, ASMOP_A, ASMOP_A);
                  result_in_carry = TRUE;
                }
              else
                {
                  if (!(AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result)) && ifx &&
                    (AOP_TYPE (left) == AOP_REG || AOP_TYPE (left) == AOP_STK && !IS_GB))
                    {
                      if (!regalloc_dry_run)
                        emit2 ("bit 7, %s", aopGet (AOP (left), AOP_SIZE (left) - 1, FALSE));
                      regalloc_dry_run_cost += ((AOP_TYPE (left) == AOP_REG) ? 2 : 4);
                      genIfxJump (ifx, "nz");
                      return;
                    }
                  /* Just load in the top most bit */
                  cheapMove (ASMOP_A, 0, AOP (left), AOP_SIZE (left) - 1, true);
                  if (!(AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result)) && ifx)
                    {
                      genIfxJump (ifx, "7");
                      return;
                    }
                  else
                    {
                      if (ifx)
                        {
                          genIfxJump (ifx, "nc");
                          return;
                        }
                      result_in_carry = FALSE;
                    }
                }
              goto release;
            }

          while (!((lit >> (offset * 8)) & 0xffull))
            {
              size--;
              offset++;
            }

          if (sign)             /* Map signed operands to unsigned ones. This pre-subtraction workaround to lack of signed comparison is cheaper than the post-subtraction one at fix. */
            {
              if (size == 2 && !(IS_GB || !ifx && requiresHL(AOP(result)) && AOP_TYPE (result) != AOP_REG) && isPairDead (PAIR_HL, ic) && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)) && (getPairId (AOP (left)) == PAIR_HL || IS_RAB && (AOP_TYPE (left) == AOP_STK || AOP_TYPE (left) == AOP_EXSTK)))
                {
                  PAIR_ID litpair = (isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC);
                  fetchPair (PAIR_HL, AOP (left));
                  emit2 ("ld %s, !immedbyte", _pairs[litpair].name, (unsigned long) ((lit ^ 0x8000u) & 0xffffu));
                  regalloc_dry_run_cost += 3;
                  emit2 ("add hl, hl");
                  emit2 ("ccf");
                  regalloc_dry_run_cost += 2;
                  if (IS_RAB)
                    {
                      emit2 ("rr hl");
                      regalloc_dry_run_cost += 1;
                    }
                  else
                    {
                      emit2 ("rr h");
                      emit2 ("rr l");
                      regalloc_dry_run_cost += 2;
                    }
                  emit2 ("sbc hl, %s", _pairs[litpair].name);
                  regalloc_dry_run_cost += 2;
                  result_in_carry = TRUE;
                  goto release;
                }

              cheapMove (ASMOP_A, 0, AOP (left), offset, true);
              if (size == 1)
                {
                  emit2 ("xor a, !immedbyte", 0x80);
                  regalloc_dry_run_cost += 2;
                }
              emit2 ("sub a, !immedbyte", (unsigned long) (((lit >> (offset * 8)) & 0xff) ^ (size == 1 ? 0x80 : 0x00)));
              regalloc_dry_run_cost += 2;
              size--;
              offset++;

              while (size--)
                {
                  cheapMove (ASMOP_A, 0, AOP (left), offset, true);
                  if (!size)
                    {
                      emit2 ("rla");
                      emit2 ("ccf");
                      emit2 ("rra");
                      regalloc_dry_run_cost += 3;
                    }
                  /* Subtract through, propagating the carry */
                  emit2 ("sbc a, !immedbyte", (unsigned long) (((lit >> (offset++ * 8)) & 0xff) ^ (size ? 0x00 : 0x80)));
                  regalloc_dry_run_cost += 2;
                }
              result_in_carry = TRUE;
              goto release;
            }
        }
      if (!IS_GB && (!sign || size > 2) && (getPartPairId (left->aop, offset) == PAIR_HL || size == 2 && left->aop->type == AOP_IY) && isPairDead (PAIR_HL, ic) &&
        (getPartPairId (right->aop, offset) == PAIR_DE || getPartPairId (right->aop, offset) == PAIR_BC))
        {
          if (left->aop->type == AOP_DIR || left->aop->type == AOP_IY)
            fetchPair (PAIR_HL, left->aop);
          emit3 (A_XOR, ASMOP_A, ASMOP_A); // Clear carry.
          emit2 ("sbc hl, %s", _pairs[getPartPairId (AOP (right), offset)].name);
          regalloc_dry_run_cost += 2;
          size -= 2;
          offset += 2;
        }
      else
        {
          if (AOP_TYPE (left) == AOP_LIT && byteOfVal (AOP (left)->aopu.aop_lit, offset) == 0x00)
            emit3 (A_XOR, ASMOP_A, ASMOP_A);
          else
            cheapMove (ASMOP_A, 0, AOP (left), offset, true);
          if (size > 1 && AOP_TYPE (left) == AOP_LIT)
            {
              emit3_o (A_CP, ASMOP_A, 0, AOP (right), offset);
              a_always_byte = byteOfVal (AOP (left)->aopu.aop_lit, offset);
            }
          else
            emit3_o (A_SUB, ASMOP_A, 0, AOP (right), offset);
          size--;
          offset++;
        }

      /* Subtract through, propagating the carry */
      while (size)
        {
          if (!IS_GB && (!sign || size > 2) &&
            isPairDead (PAIR_HL, ic) &&
            (getPartPairId (left->aop, offset) == PAIR_HL || left->aop->type == AOP_LIT && right->aop->regs[L_IDX] < offset && right->aop->regs[H_IDX] < offset) &&
            (getPartPairId (right->aop, offset) == PAIR_DE || getPartPairId (right->aop, offset) == PAIR_BC))
            {
              fetchPairLong (PAIR_HL, left->aop, 0, offset);
              emit2 ("sbc hl, %s", _pairs[getPartPairId (AOP (right), offset)].name);
              regalloc_dry_run_cost += 2;
              size -= 2;
              offset += 2;
            }
          else
            {
              if (!(left->aop->type == AOP_LIT && byteOfVal (left->aop->aopu.aop_lit, offset) == a_always_byte))
                cheapMove (ASMOP_A, 0, left->aop, offset, true);
              a_always_byte = -1;
              emit3_o (A_SBC, ASMOP_A, 0, right->aop, offset);
              size--;
              offset++;
            }
        }

fix:
      /* There is no good signed compare in the Z80, so we need workarounds */
      if (sign)
        {
          if (!IS_GB)           /* Directly check for overflow, can't be done on GBZ80 */
            {
              if (!regalloc_dry_run)
                {
                  symbol *tlbl = newiTempLabel (NULL);
                  emit2 (IS_RAB ? "jp LZ, !tlabel" : "jp PO, !tlabel", labelKey2num (tlbl->key));
                  emit2 ("xor a, !immedbyte", 0x80);
                  emitLabelSpill (tlbl);
                }
              regalloc_dry_run_cost += 5;
              result_in_carry = FALSE;
            }
          else                  /* Do it the hard way */
            {
              /* Test if one operand is negative, while the other is not. If this is the
                 case we can easily decide which one is greater, and we set/reset the carry
                 flag. If not, then the unsigned compare gave the correct result and we
                 don't change the carry flag. */
              if (!regalloc_dry_run)
                {
                  symbol *tlbl1 = newiTempLabel (NULL);
                  symbol *tlbl2 = newiTempLabel (NULL);
                  emit2 ("bit 7, e");
                  emit2 ("jp Z, !tlabel", labelKey2num (tlbl1->key));
                  emit2 ("bit 7, d");
                  emit2 ("jp NZ, !tlabel", labelKey2num (tlbl2->key));
                  emit2 ("cp a, a");
                  emit2 ("jp !tlabel", labelKey2num (tlbl2->key));
                  emitLabelSpill (tlbl1);
                  emit2 ("bit 7, d");
                  emit2 ("jp Z, !tlabel", labelKey2num (tlbl2->key));
                  emit2 ("scf");
                  emitLabelSpill (tlbl2);
                }
              regalloc_dry_run_cost += 20;
              result_in_carry = TRUE;
            }
        }
      else
        result_in_carry = TRUE;
    }

release:
  if (AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result))
    {
      if (!result_in_carry)
        {
          /* Shift the sign bit up into carry */
          emit2 ("rlca");
          regalloc_dry_run_cost += 1;
        }
      outBitC (result);
    }
  else
    {
      /* if the result is used in the next
         ifx conditional branch then generate
         code a little differently */
      if (ifx)
        {
          if (!result_in_carry)
            {
              if (!IS_GB)
                genIfxJump (ifx, "m");
              else
                {
                  emit2 ("rlca");
                  regalloc_dry_run_cost += 1;
                  genIfxJump (ifx, "c");
                }
            }
          else
            genIfxJump (ifx, "c");
        }
      else
        {
          if (!result_in_carry)
            {
              /* Shift the sign bit up into carry */
              emit2 ("rlca");
              regalloc_dry_run_cost += 1;
            }
          outBitC (result);
        }
      /* leave the result in acc */
    }
}

/*-----------------------------------------------------------------*/
/* genCmpGt :- greater than comparison                             */
/*-----------------------------------------------------------------*/
static void
genCmpGt (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  sym_link *letype, *retype;
  int sign;

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);

  sign = 0;
  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    {
      letype = getSpec (operandType (left));
      retype = getSpec (operandType (right));
      sign = !(SPEC_USIGN (letype) | SPEC_USIGN (retype));
    }

  /* assign the asmops */
  aopOp (left, ic, FALSE, FALSE);
  aopOp (right, ic, FALSE, FALSE);
  aopOp (result, ic, TRUE, FALSE);

  setupToPreserveCarry (AOP (result), AOP (left), AOP (right));

  genCmp (right, left, result, ifx, sign, ic);

  _G.preserveCarry = FALSE;
  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genCmpLt - less than comparisons                                */
/*-----------------------------------------------------------------*/
static void
genCmpLt (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  sym_link *letype, *retype;
  int sign;

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);

  sign = 0;
  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    {
      letype = getSpec (operandType (left));
      retype = getSpec (operandType (right));
      sign = !(SPEC_USIGN (letype) | SPEC_USIGN (retype));
    }

  /* assign the asmops */
  aopOp (left, ic, FALSE, FALSE);
  aopOp (right, ic, FALSE, FALSE);
  aopOp (result, ic, TRUE, FALSE);

  setupToPreserveCarry (AOP (result), AOP (left), AOP (right));

  genCmp (left, right, result, ifx, sign, ic);

  _G.preserveCarry = FALSE;
  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* gencjneshort - compare and jump if not equal                    */
/* returns pair that still needs to be popped                      */
/*-----------------------------------------------------------------*/
static PAIR_ID
gencjneshort (operand *left, operand *right, symbol *lbl, const iCode *ic)
{
  int size = max (AOP_SIZE (left), AOP_SIZE (right));
  int offset = 0;
  bool a_result = FALSE;
  bool next_zero;

  /* Swap the left and right if it makes the computation easier */
  if (AOP_TYPE (left) == AOP_LIT || aopInReg (right->aop, 0, A_IDX))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  /* Non-destructive compare */
  if (aopInReg (left->aop, 0, A_IDX) && bitVectBitValue (ic->rSurv, A_IDX) &&
    (AOP_TYPE (right) == AOP_LIT || AOP_TYPE (right) == AOP_REG && AOP (right)->aopu.aop_reg[offset]->rIdx != IYL_IDX && AOP (right)->aopu.aop_reg[offset]->rIdx != IYH_IDX || AOP_TYPE (right) == AOP_STK))
    {
      if (AOP_TYPE (right) == AOP_LIT && !byteOfVal (AOP (right)->aopu.aop_lit, 0))
        emit3 (A_OR, ASMOP_A, ASMOP_A);
      else
        emit3 (A_CP, ASMOP_A, AOP (right));
      if (!regalloc_dry_run)
        emit2 ("jp NZ,!tlabel", labelKey2num (lbl->key));
      regalloc_dry_run_cost += 3;
    }
  /* if the right side is a literal then anything goes */
  else if (AOP_TYPE (right) == AOP_LIT)
    {
      while (size--)
        {
          next_zero = size && !byteOfVal (AOP (right)->aopu.aop_lit, offset + 1);

          // Test for 0 can be done more efficiently using or
          if (!byteOfVal (AOP (right)->aopu.aop_lit, offset))
            {
              if (!a_result)
                {
                  cheapMove (ASMOP_A, 0, AOP (left), offset, true);
                  emit3 (A_OR, ASMOP_A, ASMOP_A);
                }
              else
                emit3_o (A_OR, ASMOP_A, 0, AOP (left), offset);

              a_result = TRUE;
            }
          else if ((aopInReg (left->aop, 0, A_IDX) && !bitVectBitValue (ic->rSurv, A_IDX) ||
            AOP_TYPE (left) == AOP_REG && AOP (left)->aopu.aop_reg[offset]->rIdx != IYL_IDX && AOP (left)->aopu.aop_reg[offset]->rIdx != IYH_IDX && !bitVectBitValue (ic->rSurv, AOP (left)->aopu.aop_reg[offset]->rIdx)) &&
            byteOfVal (AOP (right)->aopu.aop_lit, offset) == 0x01 && !next_zero)
            {
              if(!regalloc_dry_run)
                emit2 ("dec %s", aopGet (AOP (left), offset, FALSE));
              regalloc_dry_run_cost++;
              a_result = aopInReg (left->aop, 0, A_IDX);
            }
          else if (!bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] < offset && size && byteOfVal (right->aop->aopu.aop_lit, offset) == 0xff &&
            (left->aop->type == AOP_REG || left->aop->type == AOP_STK) &&
            byteOfVal (right->aop->aopu.aop_lit, offset) == byteOfVal (right->aop->aopu.aop_lit, offset + 1))
            {
              cheapMove (ASMOP_A, 0, left->aop, offset, true);
              while (byteOfVal (right->aop->aopu.aop_lit, offset + 1) == 0xff && size)
                {
                  emit3_o (A_AND, ASMOP_A, 0, left->aop, ++offset);
                  size--;
                }
              emit3 (A_INC, ASMOP_A, 0);
              next_zero = size && !byteOfVal (AOP (right)->aopu.aop_lit, offset + 1);
              a_result = true;
            }
          else if ((aopInReg (left->aop, 0, A_IDX) && !bitVectBitValue (ic->rSurv, A_IDX) ||
            AOP_TYPE (left) == AOP_REG && AOP (left)->aopu.aop_reg[offset]->rIdx != IYL_IDX && AOP (left)->aopu.aop_reg[offset]->rIdx != IYH_IDX && !bitVectBitValue (ic->rSurv, AOP (left)->aopu.aop_reg[offset]->rIdx)) &&
            byteOfVal (AOP (right)->aopu.aop_lit, offset) == 0xff && !next_zero)
            {
              if(!regalloc_dry_run)
                emit2 ("inc %s", aopGet (AOP (left), offset, FALSE));
              regalloc_dry_run_cost++;
              a_result = aopInReg (left->aop, 0, A_IDX);
            }
          else
            {
              cheapMove (ASMOP_A, 0, left->aop, offset, true);

              if (byteOfVal (right->aop->aopu.aop_lit, offset) == 0x01)
                emit3 (A_DEC, ASMOP_A, 0);
              else if (byteOfVal (right->aop->aopu.aop_lit, offset) == 0xff)
                emit3 (A_INC, ASMOP_A, 0);
              else
                emit3_o (A_SUB, ASMOP_A, 0, right->aop, offset);

              a_result = true;
            }

          // Only emit jump now if there is no following test for 0 (which would just or to a current result in a)
          if (!(next_zero && a_result))
            {
              if (!regalloc_dry_run)
                emit2 ("jp NZ,!tlabel", labelKey2num (lbl->key));
              regalloc_dry_run_cost += 3;
            }
          offset++;
        }
    }
  /* if the right side is in a register or
     pointed to by HL, IX or IY */
  else if (AOP_TYPE (right) == AOP_REG ||
           AOP_TYPE (right) == AOP_HL ||
           AOP_TYPE (right) == AOP_IY ||
           AOP_TYPE (right) == AOP_STK ||
           AOP_TYPE (right) == AOP_EXSTK ||
           AOP_TYPE (right) == AOP_IMMD ||
           AOP_IS_PAIRPTR (right, PAIR_HL) || AOP_IS_PAIRPTR (right, PAIR_IX) || AOP_IS_PAIRPTR (right, PAIR_IY))
    {
      while (size--)
        {
          if (aopInReg (right->aop, offset, A_IDX)  || aopInReg (right->aop, offset, HL_IDX) || aopInReg (left->aop, offset, BC_IDX) || aopInReg (left->aop, offset, DE_IDX))
            {
              operand *t = right;
              right = left;
              left = t;
            }

          if (!IS_GB && isPairDead (PAIR_HL, ic) &&
            (aopInReg (left->aop, offset, HL_IDX) && (aopInReg (right->aop, offset, BC_IDX) || aopInReg (right->aop, offset, DE_IDX) || getFreePairId (ic) != PAIR_INVALID) ||
            size == 1 && (aopInReg (right->aop, offset, BC_IDX) || aopInReg (right->aop, offset, DE_IDX))))
            {
              PAIR_ID pair = getPairId_o (right->aop, offset);
              if (pair == PAIR_INVALID)
                pair = getFreePairId (ic);

              fetchPairLong (PAIR_HL, left->aop, ic, offset);
              fetchPairLong (pair, right->aop, 0, offset);
              emit3 (A_CP, ASMOP_A, ASMOP_A);
              emit2 ("sbc hl, %s", _pairs[pair].name);
              if (!regalloc_dry_run)
                emit2 ("jp NZ,!tlabel", labelKey2num (lbl->key));
              regalloc_dry_run_cost += 5;

              offset += 2;
              size--;
              continue;
            }

          cheapMove (ASMOP_A, 0, AOP (left), offset, true);
          if (AOP_TYPE (right) == AOP_LIT && byteOfVal (AOP (right)->aopu.aop_lit, offset) == 0)
            {
              emit3 (A_OR, ASMOP_A, ASMOP_A);
              if (!regalloc_dry_run)
                emit2 ("jp NZ,!tlabel", labelKey2num (lbl->key));
              regalloc_dry_run_cost += 3;
            }
          else
            {
              emit3_o (A_SUB, ASMOP_A, 0, AOP (right), offset);
              if (!regalloc_dry_run)
                emit2 ("jp NZ,!tlabel", labelKey2num (lbl->key));
              regalloc_dry_run_cost += 3;
            }
          offset++;
        }
    }
  /* right is in direct space or a pointer reg, need both a & b */
  else
    {
      PAIR_ID pair;
      for (pair = PAIR_BC; pair <= PAIR_HL; pair++)
        {
          if (((AOP_TYPE (left) != AOP_PAIRPTR) || (AOP (left)->aopu.aop_pairId != pair)) &&
              ((AOP_TYPE (right) != AOP_PAIRPTR) || (AOP (right)->aopu.aop_pairId != pair)))
            {
              break;
            }
        }
      _push (pair);
      while (size--)
        {
          if (!regalloc_dry_run)
            _emitMove (_pairs[pair].l, aopGet (AOP (left), offset, FALSE));
          else
            regalloc_dry_run_cost += ld_cost (ASMOP_E, AOP (left));
          cheapMove (ASMOP_A, 0, AOP (right), offset, true);
          emit2 ("sub a,%s", _pairs[pair].l);
          regalloc_dry_run_cost += 1;
          if (!regalloc_dry_run)
            emit2 ("jp NZ,!tlabel", labelKey2num (lbl->key));
          regalloc_dry_run_cost += 3;
          offset++;
        }
      return pair;
    }
  return PAIR_INVALID;
}

/*-----------------------------------------------------------------*/
/* gencjne - compare and jump if not equal                         */
/*-----------------------------------------------------------------*/
static void
gencjne (operand * left, operand * right, symbol * lbl, const iCode *ic)
{
  symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
  PAIR_ID pop;

  pop = gencjneshort (left, right, lbl, ic);

  /* PENDING: ?? */
  if (!regalloc_dry_run)
    {
      emit2 ("ld a,!one");
      emit2 ("jp !tlabel", labelKey2num (tlbl->key));
      emitLabelSpill (lbl);
      emit2 ("xor a,a");
      emitLabel (tlbl);
    }
  regalloc_dry_run_cost += 6;
  _pop (pop);
}

/*-----------------------------------------------------------------*/
/* genCmpEq - generates code for equal to                          */
/*-----------------------------------------------------------------*/
static void
genCmpEq (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  bool hl_touched;

  aopOp ((left = IC_LEFT (ic)), ic, FALSE, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE, FALSE);

  hl_touched = (AOP_TYPE (IC_LEFT (ic)) == AOP_HL || AOP_TYPE (IC_RIGHT (ic)) == AOP_HL || IS_GB
                && AOP_TYPE (IC_LEFT (ic)) == AOP_STK || IS_GB && AOP_TYPE (IC_RIGHT (ic)) == AOP_STK);

  /* Swap operands if it makes the operation easier. ie if:
     1.  Left is a literal.
   */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_LIT || AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT && AOP_TYPE (IC_RIGHT (ic)) != AOP_REG
      && AOP_TYPE (IC_LEFT (ic)) == AOP_REG)
    {
      operand *t = IC_RIGHT (ic);
      IC_RIGHT (ic) = IC_LEFT (ic);
      IC_LEFT (ic) = t;
    }

  if (ifx && !AOP_SIZE (result))
    {
      /* if they are both bit variables */
      if (AOP_TYPE (left) == AOP_CRY && ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (right) == AOP_LIT)))
        {
          wassertl (0, "Tried to compare two bits");
        }
      else
        {
          PAIR_ID pop;
          symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
          pop = gencjneshort (left, right, tlbl, ic);
          if (IC_TRUE (ifx))
            {
              if (pop != PAIR_INVALID)
                {
                  emit2 ("pop %s", _pairs[pop].name);
                  regalloc_dry_run_cost += 1;
                }
              if (!regalloc_dry_run)
                emit2 ("jp !tlabel", labelKey2num (IC_TRUE (ifx)->key));
              regalloc_dry_run_cost += 3;
              if (!regalloc_dry_run)
                hl_touched ? emitLabelSpill (tlbl) : emitLabel (tlbl);
              else if (hl_touched)
                spillCached ();
              _pop (pop);
            }
          else
            {
              /* PENDING: do this better */
              symbol *lbl = regalloc_dry_run ? 0 : newiTempLabel (0);
              if (pop != PAIR_INVALID)
                {
                  emit2 ("pop %s", _pairs[pop].name);
                  regalloc_dry_run_cost += 1;
                }
              if (!regalloc_dry_run)
                emit2 ("jp !tlabel", labelKey2num (lbl->key));
              regalloc_dry_run_cost += 3;
              if (!regalloc_dry_run)
                hl_touched ? emitLabelSpill (tlbl) : emitLabel (tlbl);
              else if (hl_touched)
                spillCached ();
              _pop (pop);
              if (!regalloc_dry_run)
                {
                  emit2 ("jp !tlabel", labelKey2num (IC_FALSE (ifx)->key));
                  emitLabel (lbl);
                }
              regalloc_dry_run_cost += 3;
            }
        }
      /* mark the icode as generated */
      ifx->generated = 1;
      goto release;
    }

  /* if they are both bit variables */
  if (AOP_TYPE (left) == AOP_CRY && ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (right) == AOP_LIT)))
    {
      wassertl (0, "Tried to compare a bit to either a literal or another bit");
    }
  else
    {
      gencjne (left, right, regalloc_dry_run ? 0 : newiTempLabel (NULL), ic);
      if (AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result))
        {
          wassert (0);
        }
      if (ifx)
        {
          genIfxJump (ifx, "a");
          goto release;
        }
      /* if the result is used in an arithmetic operation
         then put the result in place */
      if (AOP_TYPE (result) != AOP_CRY)
        outAcc (result);
      /* leave the result in acc */
    }

release:
  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genAndOp - for && operation                                     */
/*-----------------------------------------------------------------*/
static void
genAndOp (const iCode * ic)
{
  operand *left, *right, *result;

  /* note here that && operations that are in an if statement are
     taken away by backPatchLabels only those used in arthmetic
     operations remain */
  aopOp ((left = IC_LEFT (ic)), ic, FALSE, TRUE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE, TRUE);
  aopOp ((result = IC_RESULT (ic)), ic, FALSE, FALSE);

  /* if both are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      wassertl (0, "Tried to and two bits");
    }
  else
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      _toBoolean (left, TRUE);
      if (!regalloc_dry_run)
        emit2 ("jp Z,!tlabel", labelKey2num (tlbl->key));
      regalloc_dry_run_cost += 3;
      _toBoolean (right, FALSE);
      if (!regalloc_dry_run)
        emitLabel (tlbl);
      outBitAcc (result);
    }

  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genOrOp - for || operation                                      */
/*-----------------------------------------------------------------*/
static void
genOrOp (const iCode * ic)
{
  operand *left, *right, *result;

  /* note here that || operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  aopOp ((left = IC_LEFT (ic)), ic, FALSE, TRUE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE, TRUE);
  aopOp ((result = IC_RESULT (ic)), ic, FALSE, FALSE);

  /* if both are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      wassertl (0, "Tried to OR two bits");
    }
  else
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      _toBoolean (left, TRUE);
      if (!regalloc_dry_run)
        emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
      regalloc_dry_run_cost += 3;
      _toBoolean (right, FALSE);
      if (!regalloc_dry_run)
        emitLabel (tlbl);
      outBitAcc (result);
    }

  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* isLiteralBit - test if lit == 2^n                               */
/*-----------------------------------------------------------------*/
static int
isLiteralBit (unsigned long lit)
{
  unsigned long pw[32] =
  {
    1L, 2L, 4L, 8L, 16L, 32L, 64L, 128L,
    0x100L, 0x200L, 0x400L, 0x800L,
    0x1000L, 0x2000L, 0x4000L, 0x8000L,
    0x10000L, 0x20000L, 0x40000L, 0x80000L,
    0x100000L, 0x200000L, 0x400000L, 0x800000L,
    0x1000000L, 0x2000000L, 0x4000000L, 0x8000000L,
    0x10000000L, 0x20000000L, 0x40000000L, 0x80000000L
  };
  int idx;

  for (idx = 0; idx < 32; idx++)
    if (lit == pw[idx])
      return idx;
  return -1;
}

/*-----------------------------------------------------------------*/
/* jmpTrueOrFalse -                                                */
/*-----------------------------------------------------------------*/
static void
jmpTrueOrFalse (iCode * ic, symbol * tlbl)
{
  // ugly but optimized by peephole
  // Using emitLabelSpill instead of emitLabel (esp. on gbz80)
  // We could jump there from locations with different values in hl.
  // This should be changed to a more efficient solution that spills
  // only what and when necessary.
  if (IC_TRUE (ic))
    {
      if (!regalloc_dry_run)
        {
          symbol *nlbl = newiTempLabel (NULL);
          emit2 ("jp !tlabel", labelKey2num (nlbl->key));
          emitLabelSpill (tlbl);
          emit2 ("jp !tlabel", labelKey2num (IC_TRUE (ic)->key));
          emitLabelSpill (nlbl);
        }
      regalloc_dry_run_cost += 6;
    }
  else
    {
      if (!regalloc_dry_run)
        {
          emit2 ("jp !tlabel", labelKey2num (IC_FALSE (ic)->key));
          emitLabelSpill (tlbl);
        }
      regalloc_dry_run_cost += 3;
    }
  if (!regalloc_dry_run)
    ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genAnd  - code for and                                          */
/*-----------------------------------------------------------------*/
static void
genAnd (const iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0L;
  unsigned int bytelit = 0;

  aopOp ((left = IC_LEFT (ic)), ic, FALSE, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE, FALSE);

  bool pushed_a = false;
  bool a_free = !bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] <= 0 && right->aop->regs[A_IDX] <= 0;

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || (AOP_NEEDSACC (right) && !AOP_NEEDSACC (left)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (sameRegs (AOP (result), AOP (right)) && !AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  if (AOP_TYPE (right) == AOP_LIT)
    lit = ullFromVal (AOP (right)->aopu.aop_lit);

  size = AOP_SIZE (result);

  if (AOP_TYPE (left) == AOP_CRY)
    {
      wassertl (0, "Tried to perform an AND with a bit as an operand");
      goto release;
    }

  /* Make sure A is on the left to not overwrite it. */
  if (aopInReg (right->aop, 0, A_IDX) ||
    !aopInReg (left->aop, 0, A_IDX) && isPair (AOP (right)) && (getPairId (AOP (right)) == PAIR_HL || getPairId (AOP (right)) == PAIR_IY))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  // if(val & 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val & 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      int sizel;

      sizel = AOP_SIZE (left);
      if (size)
        {
          /* PENDING: Test case for this. */
          emit2 ("scf");
          regalloc_dry_run_cost += 1;
        }
      while (sizel)
        {
          char *jumpcond = "NZ";

          if ((bytelit = ((lit >> (offset * 8)) & 0x0ffull)) == 0x00ull)
            {
              sizel--;
              offset++;
              continue;
            }

          /* Testing for the border bits of the accumulator destructively is cheap. */
          if ((isLiteralBit (bytelit) == 0 || isLiteralBit (bytelit) == 7) && aopInReg (left->aop, 0, A_IDX) && !bitVectBitValue (ic->rSurv, A_IDX))
            {
              emit3 (isLiteralBit (bytelit) == 0 ? A_RRCA : A_RLCA, 0 , 0);
              jumpcond = "C";
              sizel--;
              offset++;
            }
          /* Testing for the inverse of the border bits of some 32-bit registers destructively is cheap. */
          /* More combinations would be possible, but this one is the one that is common in the floating-point library. */
          else if (AOP_TYPE (left) == AOP_REG && sizel >= 4 && ((lit >> (offset * 8)) & 0xffffffffull) == 0x7fffffffull &&
            !IS_GB && getPartPairId (AOP (left), offset) == PAIR_HL && isPairDead (PAIR_HL, ic) &&
            IS_RAB && getPartPairId (AOP (left), offset + 2) == PAIR_DE && isPairDead (PAIR_HL, ic))
            {
              emit3 (A_CP, ASMOP_A, ASMOP_A); // Clear carry.
              emit2 ("adc hl, hl"); // Cannot use "add hl, hl instead, since it does not affect zero flag.
              if (!regalloc_dry_run)
                emit2 ("jp NZ,!tlabel", labelKey2num (tlbl->key));
              emit2 ("rl de");
              regalloc_dry_run_cost += 6;
              sizel -= 4;
              offset += 4;
            }
          /* Testing for the inverse of the border bits of some 16-bit registers destructively is cheap. */
          /* More combinations would be possible, but these are the common ones. */
          else if (AOP_TYPE (left) == AOP_REG && sizel >= 2 && ((lit >> (offset * 8)) & 0xffffull) == 0x7fffull &&
            (!IS_GB && getPartPairId (AOP (left), offset) == PAIR_HL && isPairDead (PAIR_HL, ic) ||
            IS_RAB && getPartPairId (AOP (left), offset) == PAIR_DE  && isPairDead (PAIR_DE, ic)))
            {
              PAIR_ID pair;
              switch (AOP (left)->aopu.aop_reg[offset]->rIdx)
                {
                case L_IDX:
                case H_IDX:
                  pair = PAIR_HL;
                  break;
                case E_IDX:
                case D_IDX:
                  pair = PAIR_DE;
                  break;
                default:
                  pair = PAIR_INVALID;
                  wassertl (0, "Invalid pair");
                }
              emit3 (A_CP, ASMOP_A, ASMOP_A); // Clear carry.
              if (pair == PAIR_HL)
                emit2 ("adc %s, %s", _pairs[pair].name, _pairs[pair].name); // Cannot use "add hl, hl instead, since it does not affect zero flag.
              else
                emit2 ("rl %s", _pairs[pair].name);
              regalloc_dry_run_cost += (pair == PAIR_DE ? 1 : 2);
              sizel -= 2;
              offset += 2;
            }
          /* Testing for the border bits of some 16-bit registers destructively is cheap. */
          else if (AOP_TYPE (left) == AOP_REG && sizel == 1 &&
            (isLiteralBit (bytelit) == 7 && (
              AOP (left)->aopu.aop_reg[offset]->rIdx == H_IDX && isPairDead (PAIR_HL, ic) ||
              IS_RAB && AOP (left)->aopu.aop_reg[offset]->rIdx == D_IDX && isPairDead (PAIR_DE, ic) ||
              AOP (left)->aopu.aop_reg[offset]->rIdx == IYH_IDX && isPairDead (PAIR_IY, ic)
            ) ||
            isLiteralBit (bytelit) == 0 && IS_RAB && (
              AOP (left)->aopu.aop_reg[offset]->rIdx == L_IDX && isPairDead (PAIR_HL, ic) ||
              AOP (left)->aopu.aop_reg[offset]->rIdx == E_IDX && isPairDead (PAIR_DE, ic) ||
              AOP (left)->aopu.aop_reg[offset]->rIdx == IYL_IDX && isPairDead (PAIR_IY, ic)
              )))
            {
              PAIR_ID pair;
              switch (AOP (left)->aopu.aop_reg[offset]->rIdx)
                {
                case L_IDX:
                case H_IDX:
                  pair = PAIR_HL;
                  break;
                case E_IDX:
                case D_IDX:
                  pair = PAIR_DE;
                  break;
                case IYL_IDX:
                case IYH_IDX:
                  pair = PAIR_IY;
                  break;
                default:
                  pair = PAIR_INVALID;
                  wassertl (0, "Invalid pair");
                }
              if ((pair == PAIR_HL || pair == PAIR_IY) && isLiteralBit (bytelit) == 7)
                emit2 ("add %s, %s", _pairs[pair].name, _pairs[pair].name);
              else if (isLiteralBit (bytelit) == 7)
                emit2 ("rl %s", _pairs[pair].name);
              else
                emit2 ("rr %s", _pairs[pair].name);
              regalloc_dry_run_cost += (pair == PAIR_IY ? 2 : 1);
              jumpcond = "C";
              sizel--;
              offset++;
            }
          /* Non-destructive and when exactly one bit per byte is set. */
          else if (isLiteralBit (bytelit) >= 0 &&
            (AOP_TYPE (left) == AOP_STK || aopInReg (left->aop, 0, A_IDX) || AOP_TYPE (left) == AOP_HL || AOP_TYPE (left) == AOP_IY || AOP_TYPE (left) == AOP_REG && AOP (left)->aopu.aop_reg[0]->rIdx != IYL_IDX))
            {
              if (!regalloc_dry_run)
                emit2 ("bit %d, %s", isLiteralBit (bytelit), aopGet (AOP (left), offset, FALSE));
              regalloc_dry_run_cost += (AOP_TYPE (left) == AOP_STK || AOP_TYPE (left) == AOP_IY) ? 4 : 2;
              sizel--;
              offset++;
            }
          /* Z180 has non-destructive and. */
          else if ((IS_Z180 || IS_EZ80_Z80) && aopInReg (left->aop, 0, A_IDX) && bitVectBitValue (ic->rSurv, A_IDX) && bytelit != 0x0ff)
            {
              if (!regalloc_dry_run)
                emit2 ("tst a, %s", aopGet (AOP (right), 0, FALSE));
              regalloc_dry_run_cost += (AOP_TYPE (right) == AOP_LIT ? 3 : 2);
              sizel--;
              offset++;
            }
          /* Generic case, loading into accumulator and testing there. */
          else
            {
              if (bitVectBitValue (ic->rSurv, A_IDX) || left->aop->regs[A_IDX] > offset || right->aop->regs[A_IDX] > offset)
                {
                  regalloc_dry_run_cost += 100;
                  wassert (regalloc_dry_run);
                }

              cheapMove (ASMOP_A, 0, left->aop, offset, true);
              if (isLiteralBit (bytelit) == 0 || isLiteralBit (bytelit) == 7)
                {
                  emit3 (isLiteralBit (bytelit) == 0 ? A_RRCA : A_RLCA, 0 , 0);
                  jumpcond = "C";
                }
              else if (bytelit != 0xffu)
                emit3_o (A_AND, ASMOP_A, 0, AOP (right), offset);
              else
                emit3 (A_OR, ASMOP_A, ASMOP_A);     /* For the flags */
              sizel--;
              offset++;
            }
          if (size || ifx)  /* emit jmp only, if it is actually used */
            {
              if (!regalloc_dry_run)
                emit2 ("jp %s,!tlabel", jumpcond, labelKey2num (tlbl->key));
              regalloc_dry_run_cost += 3;
            }
        }
      // bit = left & literal
      if (size)
        {
          emit2 ("clr c");
          if (!regalloc_dry_run)
            emit2 ("!tlabeldef", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 3;
          genLine.lineCurr->isLabel = 1;
        }
      // if(left & literal)
      else
        {
          if (ifx)
            jmpTrueOrFalse (ifx, tlbl);
          goto release;
        }
      outBitC (result);
      goto release;
    }

  if (IS_RAB && isPair (AOP (result)) &&
      (getPairId (AOP (result)) == PAIR_HL && isPair (AOP (right)) && getPairId (AOP (right)) == PAIR_DE ||
       getPairId (AOP (result)) == PAIR_HL && isPair (AOP (left)) && getPairId (AOP (left)) == PAIR_DE ||
       isPair (AOP (left)) && getPairId (AOP (left)) == PAIR_IY && getPairId (AOP (result)) == PAIR_IY && isPair (AOP (right))
       && getPairId (AOP (right)) == PAIR_DE))
    {
      if (isPair (AOP (left)) && getPairId (AOP (left)) == PAIR_DE)
        fetchPair (PAIR_HL, AOP (right));
      else                      /* right operand in DE */
        fetchPair (getPairId (AOP (result)), AOP (left));
      emit2 ("and hl, de");
      regalloc_dry_run_cost += (getPairId (AOP (result)) == PAIR_HL ? 1 : 2);
      goto release;
    }

  wassertl (AOP_TYPE (result) != AOP_CRY, "Result of and is in a bit");

  for (int i = 0; i < size;)
    {
      if (!bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] <= i && right->aop->regs[A_IDX] <= i && (result->aop->regs[A_IDX] < 0 || result->aop->regs[A_IDX] >= i))
        a_free = true;

      if (pushed_a && (aopInReg (left->aop, i, A_IDX) || aopInReg (right->aop, i, A_IDX)))
        {
          _pop (PAIR_AF);
          if (bitVectBitValue (ic->rSurv, A_IDX))
            _push (PAIR_AF);
          else
            pushed_a = false;
        }

      if (AOP_TYPE (right) == AOP_LIT)
        {
          bytelit = byteOfVal (right->aop->aopu.aop_lit, i);

          if (bytelit == 0x00)
            {
              cheapMove (result->aop, i, ASMOP_ZERO, 0, a_free);
              if (aopInReg (result->aop, i, A_IDX))
                a_free = false;
              i++;
              continue;
            }
          else if (bytelit == 0xff)
            {
              cheapMove (result->aop, i, AOP (left), i, a_free);
              if (aopInReg (result->aop, i, A_IDX))
                a_free = false;
              i++;
              continue;
            }
          else if (isLiteralBit (~bytelit & 0xffu) >= 0 &&
            (AOP_TYPE (result) == AOP_REG || left == right && (AOP_TYPE (result) == AOP_STK || AOP_TYPE (result) == AOP_DIR)))
            {
              cheapMove (result->aop, i, left->aop, i, a_free);
              if (!regalloc_dry_run)
                emit2 ("res %d, %s", isLiteralBit (~bytelit & 0xffu), aopGet (result->aop, i, false));
              regalloc_dry_run_cost += 2;
              if (aopInReg (result->aop, i, A_IDX))
                a_free = false;
              i++;
              continue;
            }
          else if (IS_RAB &&
            (aopInReg (result->aop, i, HL_IDX) && (aopInReg (left->aop, i, HL_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, DE_IDX)) ||
            aopInReg (result->aop, i, H_IDX) && aopInReg (result->aop, i + 1, L_IDX) && (aopInReg (left->aop, i, H_IDX) && aopInReg (left->aop, i + 1, L_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, D_IDX) && aopInReg (left->aop, i + 1, E_IDX))))
            {
              unsigned short mask = aopInReg (result->aop, i, L_IDX) ? (bytelit + (byteOfVal (right->aop->aopu.aop_lit, i + 1) << 8)) : (byteOfVal (right->aop->aopu.aop_lit, i + 1) + (bytelit << 8));
              bool mask_in_de = (aopInReg (left->aop, i, L_IDX) | aopInReg (left->aop, i, H_IDX));
              emit2 (mask_in_de ? "ld de, !immedword" : "ld hl, !immedword", mask);
              emit2 ("and hl, de");
              regalloc_dry_run_cost += 4;
              i += 2;
              continue;
            }
        }

      if (IS_RAB)
        {
          const bool this_byte_l = aopInReg (result->aop, i, L_IDX) &&
            (aopInReg (left->aop, i, L_IDX) && aopInReg (left->aop, i, E_IDX) || aopInReg (left->aop, i, E_IDX) && aopInReg (left->aop, i, L_IDX));
          const bool this_byte_h = aopInReg (result->aop, i, H_IDX) &&
            (aopInReg (left->aop, i, H_IDX) && aopInReg (left->aop, i, D_IDX) || aopInReg (left->aop, i, D_IDX) && aopInReg (left->aop, i, H_IDX));
          const bool next_byte_l = aopInReg (result->aop, i + 1, L_IDX) &&
            (aopInReg (left->aop, i + 1, L_IDX) && aopInReg (left->aop, i + 1, E_IDX) || aopInReg (left->aop, i + 1, E_IDX) && aopInReg (left->aop, i + 1, L_IDX));
          const bool next_byte_h = aopInReg (result->aop, i + 1, H_IDX) &&
            (aopInReg (left->aop, i + 1, H_IDX) && aopInReg (left->aop, i + 1, D_IDX) || aopInReg (left->aop, i + 1, D_IDX) && aopInReg (left->aop, i + 1, H_IDX));

          const bool this_byte = this_byte_l || this_byte_h;
          const bool next_byte = next_byte_l || next_byte_h;

          const bool next_byte_unused = !bitVectBitValue (ic->rMask, this_byte_l ? H_IDX : L_IDX);

          if (this_byte && (next_byte || next_byte_unused))
            {
              emit2 ("and hl, de");
              regalloc_dry_run_cost++;
              i += (1 + next_byte);
              continue;
            }
        }

      if (!a_free)
        {
          wassert (!pushed_a);
          _push (PAIR_AF);
          pushed_a = true;
          a_free = true;
        }

      // Use plain and in a.
      if (aopInReg (right->aop, i, A_IDX))
        emit3_o (A_AND, ASMOP_A, 0, left->aop, i);
      else
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true);
          emit3_o (A_AND, ASMOP_A, 0, right->aop, i);
        }
      cheapMove (result->aop, i, ASMOP_A, 0, true);

      if (aopInReg (result->aop, i, A_IDX))
        a_free = false;

      i++;
    }
  if (pushed_a)
    _pop (PAIR_AF);

release:
  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genOr  - code for or                                            */
/*-----------------------------------------------------------------*/
static void
genOr (const iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0;
  int bytelit = 0;

  aopOp ((left = IC_LEFT (ic)), ic, FALSE, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE, FALSE);

  bool pushed_a = false;
  bool a_free = !bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] <= 0 && right->aop->regs[A_IDX] <= 0;

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || (AOP_NEEDSACC (right) && !AOP_NEEDSACC (left)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (sameRegs (AOP (result), AOP (right)) && !AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  if (AOP_TYPE (right) == AOP_LIT)
    lit = ullFromVal (AOP (right)->aopu.aop_lit);

  size = AOP_SIZE (result);

  if (AOP_TYPE (left) == AOP_CRY)
    {
      wassertl (0, "Tried to OR where left is a bit");
      goto release;
    }

  /* Make sure A is on the left to not overwrite it. */
  if (aopInReg (right->aop, 0, A_IDX))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  // if(val | 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val | 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      int sizel;

      sizel = AOP_SIZE (left);

      if (size)
        {
          wassertl (0, "Result is assigned to a bit");
        }
      /* PENDING: Modeled after the AND code which is inefficient. */
      while (sizel--)
        {
          if (!bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] <= offset && right->aop->regs[A_IDX] <= offset && (result->aop->regs[A_IDX] < 0 || result->aop->regs[A_IDX] >= offset))
            a_free = true;

          if (!a_free) // Hard to handle pop with ifx
            {
               regalloc_dry_run_cost += 100;
               wassert (regalloc_dry_run);
            }

          bytelit = (lit >> (offset * 8)) & 0x0FFull;

          cheapMove (ASMOP_A, 0, left->aop, offset, true);  

          if (bytelit != 0)
            emit3_o (A_OR, ASMOP_A, 0, AOP (right), offset);
          else if (ifx)
            {
              /* For the flags */
              emit3 (A_OR, ASMOP_A, ASMOP_A);
            }

          if (ifx)              /* emit jmp only, if it is actually used */
            {
              if (!regalloc_dry_run)
                emit2 ("jp NZ,!tlabel", labelKey2num (tlbl->key));
              regalloc_dry_run_cost += 3;
            }

          offset++;
        }
      if (ifx)
        {
          jmpTrueOrFalse (ifx, tlbl);
        }
      goto release;
    }

  wassertl (AOP_TYPE (result) != AOP_CRY, "Result of or is in a bit");

  for (int i = 0; i < size;)
    {
      if (!bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] <= i && right->aop->regs[A_IDX] <= i && (result->aop->regs[A_IDX] < 0 || result->aop->regs[A_IDX] >= i))
        a_free = true;

      if (pushed_a && (aopInReg (left->aop, i, A_IDX) || aopInReg (right->aop, i, A_IDX)))
        {
          _pop (PAIR_AF);
          if (bitVectBitValue (ic->rSurv, A_IDX))
            _push (PAIR_AF);
          else
            pushed_a = false;
        }

      if (AOP_TYPE (right) == AOP_LIT)
        {
          bytelit = byteOfVal (right->aop->aopu.aop_lit, i);

          if (bytelit == 0xff)
            {
              // TODO
            }
          else if (bytelit == 0x00)
            {
              cheapMove (result->aop, i, left->aop, i, a_free);
              if (aopInReg (result->aop, i, A_IDX))
                a_free = false;
              i++;
              continue;
            }
          else if (isLiteralBit (bytelit) >= 0 &&
            (AOP_TYPE (result) == AOP_REG || left == right && (AOP_TYPE (result) == AOP_STK || AOP_TYPE (result) == AOP_DIR)))
            {
              cheapMove (result->aop, i, left->aop, i, a_free);
              if (!regalloc_dry_run)
                emit2 ("set %d, %s", isLiteralBit (bytelit), aopGet (result->aop, i, false));
              regalloc_dry_run_cost += 2;
              if (aopInReg (result->aop, i, A_IDX))
                a_free = false;
              i++;
              continue;
            }
          else if (IS_RAB &&
            (aopInReg (result->aop, i, HL_IDX) && (aopInReg (left->aop, i, HL_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, DE_IDX)) ||
            aopInReg (result->aop, i, H_IDX) && aopInReg (result->aop, i + 1, L_IDX) && (aopInReg (left->aop, i, H_IDX) && aopInReg (left->aop, i + 1, L_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, D_IDX) && aopInReg (left->aop, i + 1, E_IDX))))
            {
              unsigned short mask = aopInReg (result->aop, i, L_IDX) ? (bytelit + (byteOfVal (right->aop->aopu.aop_lit, i + 1) << 8)) : (byteOfVal (right->aop->aopu.aop_lit, i + 1) + (bytelit << 8));
              bool mask_in_de = (aopInReg (left->aop, i, L_IDX) | aopInReg (left->aop, i, H_IDX));
              emit2 (mask_in_de ? "ld de, !immedword" : "ld hl, !immedword", mask);
              emit2 ("or hl, de");
              regalloc_dry_run_cost += 4;
              i += 2;
              continue;
            }
        }

      if (IS_RAB)
        {
          const bool this_byte_l = aopInReg (result->aop, i, L_IDX) &&
            (aopInReg (left->aop, i, L_IDX) && aopInReg (left->aop, i, E_IDX) || aopInReg (left->aop, i, E_IDX) && aopInReg (left->aop, i, L_IDX));
          const bool this_byte_h = aopInReg (result->aop, i, H_IDX) &&
            (aopInReg (left->aop, i, H_IDX) && aopInReg (left->aop, i, D_IDX) || aopInReg (left->aop, i, D_IDX) && aopInReg (left->aop, i, H_IDX));
          const bool next_byte_l = aopInReg (result->aop, i + 1, L_IDX) &&
            (aopInReg (left->aop, i + 1, L_IDX) && aopInReg (left->aop, i + 1, E_IDX) || aopInReg (left->aop, i + 1, E_IDX) && aopInReg (left->aop, i + 1, L_IDX));
          const bool next_byte_h = aopInReg (result->aop, i + 1, H_IDX) &&
            (aopInReg (left->aop, i + 1, H_IDX) && aopInReg (left->aop, i + 1, D_IDX) || aopInReg (left->aop, i + 1, D_IDX) && aopInReg (left->aop, i + 1, H_IDX));

          const bool this_byte = this_byte_l || this_byte_h;
          const bool next_byte = next_byte_l || next_byte_h;

          const bool next_byte_unused = !bitVectBitValue (ic->rMask, this_byte_l ? H_IDX : L_IDX);

          if (this_byte && (next_byte || next_byte_unused))
            {
              emit2 ("or hl, de");
              regalloc_dry_run_cost++;
              i += (1 + next_byte);
              continue;
            }
        }

      // Use plain or in a.
      if (!a_free)
        {
          wassert (!pushed_a);
          _push (PAIR_AF);
          pushed_a = true;
          a_free = true;
        }

      if (aopInReg (right->aop, i, A_IDX))
        emit3_o (A_OR, ASMOP_A, 0, left->aop, i);
      else
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true);
          emit3_o (A_OR, ASMOP_A, 0, right->aop, i);
        }
      cheapMove (result->aop, i, ASMOP_A, 0, true);
      if (aopInReg (result->aop, i, A_IDX))
        a_free = false;
      i++;
    }

  if (pushed_a)
    _pop (PAIR_AF);

release:
  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genXor - code for xclusive or                                   */
/*-----------------------------------------------------------------*/
static void
genXor (const iCode *ic, iCode *ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0;
  bool pushed_a = false;

  aopOp ((left = IC_LEFT (ic)), ic, FALSE, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE, FALSE);

  bool a_free = !bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] <= 0 && right->aop->regs[A_IDX] <= 0;

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || (AOP_NEEDSACC (right) && !AOP_NEEDSACC (left)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (sameRegs (result->aop, AOP (right)) && !AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  if (AOP_TYPE (right) == AOP_LIT)
    lit = ullFromVal (AOP (right)->aopu.aop_lit);

  size = AOP_SIZE (result);

  if (AOP_TYPE (left) == AOP_CRY)
    {
      wassertl (0, "Tried to XOR a bit");
      goto release;
    }

  /* Make sure A is on the left to not overwrite it. */
  if (aopInReg (right->aop, 0, A_IDX))
    {
      wassert (!AOP_NEEDSACC (left));
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  // if(val & 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val & 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      int sizel;

      sizel = AOP_SIZE (left);

      if (size)
        {
          /* PENDING: Test case for this. */
          wassertl (0, "Tried to XOR left against a literal with the result going into a bit");
        }
      while (sizel--)
        {
          if (!bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] <= offset && right->aop->regs[A_IDX] <= offset && (result->aop->regs[A_IDX] < 0 || result->aop->regs[A_IDX] >= offset))
            a_free = true;

          if (!a_free)
            {
              wassert (!pushed_a);
              _push (PAIR_AF);
              a_free = true;
              pushed_a = true;
              if (ifx) // The pop at the end is hard to eal with in case of ifx.
                {
                  regalloc_dry_run_cost += 100;
                  wassert (regalloc_dry_run);
                }
            }
          else if (pushed_a && (aopInReg (left->aop, offset, A_IDX) || aopInReg (right->aop, offset, A_IDX)))
            {
              _pop (PAIR_AF);
              if (bitVectBitValue (ic->rSurv, A_IDX))
                _push (PAIR_AF);
              else
                pushed_a = false;
            }

          if (aopInReg (right->aop, offset, A_IDX))
            emit3_o (A_XOR, ASMOP_A, 0, left->aop, offset);
          else
            {
              cheapMove (ASMOP_A, 0, left->aop, offset, true);
              emit3_o (A_XOR, ASMOP_A, 0, right->aop, offset);
            }
          if (ifx)              /* emit jmp only, if it is actually used * */
            if (!regalloc_dry_run)
              emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 3;
          offset++;
        }
      if (pushed_a)
        _pop (PAIR_AF);
      if (ifx)
        {
          jmpTrueOrFalse (ifx, tlbl);
        }
      else if (size)
        {
          wassertl (0, "Result of XOR was destined for a bit");
        }
      goto release;
    }

    // left & result in different registers
    if (AOP_TYPE (result) == AOP_CRY)
      {
        wassertl (0, "Result of XOR is in a bit");
      }
    else
      for (; (size--); offset++)
        {
          if (!bitVectBitValue (ic->rSurv, A_IDX) && left->aop->regs[A_IDX] <= offset && right->aop->regs[A_IDX] <= offset && (result->aop->regs[A_IDX] < 0 || result->aop->regs[A_IDX] >= offset))
            a_free = true;

          if (pushed_a && (aopInReg (left->aop, offset, A_IDX) || aopInReg (right->aop, offset, A_IDX)))
            {
              _pop (PAIR_AF);
              if (bitVectBitValue (ic->rSurv, A_IDX))
                _push (PAIR_AF);
              else
                pushed_a = false;
            }

          // normal case
          // result = left & right
          if (right->aop->type == AOP_LIT)
            {
              if (((lit >> (offset * 8)) & 0x0FFL) == 0x00L)
                {
                  cheapMove (result->aop, offset, left->aop, offset, a_free);
                  if (aopInReg (result->aop, offset, A_IDX))
                    a_free = false;
                  continue;
                }
            }
          // faster than result <- left, anl result,right
          // and better if result is SFR
          if (!a_free)
            {
              wassert (!pushed_a);
              _push (PAIR_AF);
              a_free = true;
              pushed_a = true;
            }

          if (aopInReg (right->aop, offset, A_IDX))
            emit3_o (A_XOR, ASMOP_A, 0, left->aop, offset);
          else
            {
              cheapMove (ASMOP_A, 0, left->aop, offset, true);
              if (right->aop->type == AOP_LIT && ((lit >> (offset * 8)) & 0xff) == 0xff)
                emit3 (A_CPL, 0, 0);
              else
                emit3_o (A_XOR, ASMOP_A, 0, right->aop, offset);
            }
          cheapMove (result->aop, offset, ASMOP_A, 0, true);
          if (aopInReg (result->aop, offset, A_IDX))
            a_free = false;
        }
  if (pushed_a)
     _pop (PAIR_AF);

release:
  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genRRC - rotate right with carry                                */
/*-----------------------------------------------------------------*/
static void
genRRC (const iCode * ic)
{
  wassert (0);
}

/*-----------------------------------------------------------------*/
/* genRLC - generate code for rotate left with carry               */
/*-----------------------------------------------------------------*/
static void
genRLC (const iCode * ic)
{
  wassert (0);
}

/*-----------------------------------------------------------------*/
/* genGetHbit - generates code get highest order bit               */
/*-----------------------------------------------------------------*/
static void
genGetHbit (const iCode * ic)
{
  operand *left, *result;
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, FALSE);

  /* get the highest order byte into a */
  cheapMove (ASMOP_A, 0, left->aop, AOP_SIZE (left) - 1, true);

  if (AOP_TYPE (result) == AOP_CRY)
    {
      emit3 (A_RL, ASMOP_A, 0);
      outBitC (result);
    }
  else
    {
      emit3 (A_RLC, ASMOP_A, 0);
      emit2 ("and a, !one");
      regalloc_dry_run_cost += 2;
      outAcc (result);
    }


  freeAsmop (left, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genGetAbit - generates code get a single bit                    */
/*-----------------------------------------------------------------*/
static void
genGetAbit (const iCode * ic)
{
  wassert (0);
}

static void
emitRsh2 (asmop * aop, int size, int is_signed)
{
  int offset = 0;

  while (size--)
    {
      if (offset == 0)
        emit3_o (is_signed ? A_SRA : A_SRL, aop, size, 0, 0);
      else
        emit3_o (A_RR, aop, size, 0, 0);
      offset++;
    }
}

/*-----------------------------------------------------------------*/
/* shiftR2Left2Result - shift right two bytes from left to result  */
/*-----------------------------------------------------------------*/
static void
shiftR2Left2Result (const iCode *ic, operand *left, int offl, operand *result, int offr, int shCount, int is_signed)
{
  int size = 2;
  symbol *tlbl;

  if (IS_RAB && !is_signed && shCount < 4 &&
    (getPairId (AOP (result)) == PAIR_HL || getPairId (AOP (result)) == PAIR_DE))
    {
      bool op_de = (getPairId (AOP (result)) == PAIR_DE);
      fetchPairLong (getPairId (AOP (result)), AOP(left), ic, offl);
      while (shCount--)
        {
          emit3 (A_CP, ASMOP_A, ASMOP_A);
          emit2 (op_de? "rr de" : "rr hl");
          regalloc_dry_run_cost++;
        }
      return;
    }
  else if (IS_RAB && !is_signed && shCount >= 2 && isPairDead (PAIR_HL, ic) &&
      ((isPair (AOP (left)) && getPairId (AOP (left)) == PAIR_HL || isPair (AOP (result))
        && getPairId (AOP (result)) == PAIR_HL) && isPairDead (PAIR_DE, ic) || isPair (AOP (left))
       && getPairId (AOP (left)) == PAIR_DE))
    {
      bool op_de = (getPairId (AOP (left)) == PAIR_DE);
      if (op_de)
        emit2 ("ld hl, !immedword", 0xffff >> shCount);
      else
        {
          fetchPair (PAIR_HL, AOP (left));
          emit2 ("ld de, !immedword", 0xffff >> shCount);
        }
      regalloc_dry_run_cost += 3;
      while (shCount--)
        {
          emit2 (op_de ? "rr de" : "rr hl");
          regalloc_dry_run_cost++;
        }
      emit2 ("and hl, de");
      regalloc_dry_run_cost += 1;
      commitPair (AOP (IC_RESULT (ic)), PAIR_HL, ic, TRUE);
      return;
    }

  if (isPair (AOP (result)) && !offr)
    fetchPairLong (getPairId (AOP (result)), AOP(left), ic, offl);
  else
    genMove_o (result->aop, offr, left->aop, offl, 2, true, isPairDead (PAIR_HL, ic));

  if (shCount == 0)
    return;

  /*  if (AOP(result)->type == AOP_REG) { */

  /* Left is already in result - so now do the shift */
  /* Optimizing for speed by default. */
  if (!optimize.codeSize || shCount <= 2)
    {
      while (shCount--)
        {
          emitRsh2 (AOP (result), size, is_signed);
        }
    }
  else
    {
      bool use_b = (!IS_GB && !bitVectBitValue (ic->rSurv, B_IDX)
                    && !(AOP_TYPE (result) == AOP_REG
                         && (AOP (result)->aopu.aop_reg[0]->rIdx == B_IDX || AOP (result)->aopu.aop_reg[1]->rIdx == B_IDX)));

      tlbl = regalloc_dry_run ? 0 : newiTempLabel (NULL);

      if (!regalloc_dry_run)
        {
          emit2 ("ld %s, !immedbyte", use_b ? "b" : "a", shCount);
          emitLabel (tlbl);
        }
      regalloc_dry_run_cost += 2;

      emitRsh2 (AOP (result), size, is_signed);

      if (!regalloc_dry_run)
        {
          if (use_b)
            emit2 ("djnz !tlabel", labelKey2num (tlbl->key));
          else
            {
              emit2 ("dec a");
              emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
            }
        }
      regalloc_dry_run_cost += use_b ? 2 : 4;
    }
}

/*-----------------------------------------------------------------*/
/* shiftL2Left2Result - shift left two bytes from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftL2Left2Result (operand *left, int offl, operand *result, int offr, int shCount, const iCode *ic)
{
  operand *shiftoperand = result;

  if (sameRegs (AOP (result), AOP (left)) && ((offl + MSB16) == offr))
    {
      wassert (0);
    }

  /* For a shift of 7 we can use cheaper right shifts */
  else if (shCount == 7 && AOP_TYPE (left) == AOP_REG && !bitVectBitValue (ic->rSurv, AOP (left)->aopu.aop_reg[0]->rIdx) && AOP_TYPE (result) == AOP_REG &&
    AOP (left)->aopu.aop_reg[0]->rIdx != IYL_IDX && AOP (left)->aopu.aop_reg[1]->rIdx != IYL_IDX && AOP (left)->aopu.aop_reg[0]->rIdx != IYH_IDX && AOP (left)->aopu.aop_reg[1]->rIdx != IYH_IDX &&
    AOP (result)->aopu.aop_reg[0]->rIdx != IYL_IDX && AOP (result)->aopu.aop_reg[1]->rIdx != IYL_IDX && AOP (result)->aopu.aop_reg[0]->rIdx != IYH_IDX && AOP (result)->aopu.aop_reg[1]->rIdx != IYH_IDX &&
    (optimize.codeSpeed || getPairId (AOP (result)) != PAIR_HL || getPairId (AOP (left)) != PAIR_HL)) /* but a sequence of add hl, hl might still be cheaper code-size wise */
    {
      // Handling the low byte in A with xor clearing is cheaper.
      bool special_a = (!bitVectBitValue (ic->rSurv, A_IDX) && !aopInReg (AOP (left), 0, A_IDX) && !aopInReg (AOP (left), 0, A_IDX));
      asmop *lowbyte = special_a ? ASMOP_A : AOP (result);

      if (special_a)
        emit3 (A_XOR, ASMOP_A, ASMOP_A);
      emit3_o (A_RR, AOP (left), 1, 0, 0);
      emit3_o (A_LD, AOP (result), 1, AOP (left), 0);
      emit3_o (A_RR, AOP (result), 1, 0, 0);
      if (!special_a)
        emit3_o (A_LD, AOP (result), 0, ASMOP_ZERO, 0);
      if (aopInReg (lowbyte, 0, A_IDX))
        emit3 (A_RRA, 0, 0);
      else
        emit3 (A_RR, lowbyte, 0);
      if (special_a)
        cheapMove (result->aop, 0, lowbyte, 0, true);
      return;
    }

  if (AOP_TYPE (result) != AOP_REG && AOP_TYPE (left) == AOP_REG && AOP_SIZE (left) >= 2 && !bitVectBitValue (ic->rSurv, AOP (left)->aopu.aop_reg[0]->rIdx) && !bitVectBitValue (ic->rSurv, AOP (left)->aopu.aop_reg[1]->rIdx) ||
    getPairId (AOP (left)) == PAIR_HL && isPairDead (PAIR_HL, ic))
    shiftoperand = left;
  else if (isPair (AOP (result)) && !offr)
    fetchPairLong (getPairId (AOP (result)), AOP(left), ic, offl);
  else
    genMove_o (result->aop, offr, left->aop, offl, 2, true, isPairDead (PAIR_HL, ic));

  if (shCount == 0)
    ;
  else if (getPairId (AOP (shiftoperand)) == PAIR_HL)
    {
      while (shCount--)
        {
          emit2 ("add hl, hl");
          regalloc_dry_run_cost += 1;
        }
    }
  else if (getPairId (AOP (shiftoperand)) == PAIR_IY)
    {
      while (shCount--)
        {
          emit2 ("add iy, iy");
          regalloc_dry_run_cost += 2;
        }
    }
  else if (IS_RAB && getPairId (AOP (shiftoperand)) == PAIR_DE)
    {
      while (shCount--)
        {
          emit3 (A_CP, ASMOP_A, ASMOP_A);
          emit2 ("rl de");
          regalloc_dry_run_cost++;
        }
    }
  else
    {
      int size = 2;
      int offset = 0;
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      symbol *tlbl1 = regalloc_dry_run ? 0 : newiTempLabel (0);

      if (AOP (shiftoperand)->type == AOP_REG)
        {
          while (shCount--)
            {
              for (offset = 0; offset < size; offset++)
                emit3_o (offset ? A_RL : A_SLA, AOP (shiftoperand), offset, 0, 0);
            }
        }
      else
        {
          /* Left is already in result - so now do the shift */
          if (shCount > 1)
            {
              if (!regalloc_dry_run)
                {
                  emit2 ("ld a, !immedbyte+1", shCount);
                  emit2 ("jp !tlabel", labelKey2num (tlbl1->key));
                  emitLabel (tlbl);
                }
              regalloc_dry_run_cost += 4;
            }

          while (size--)
            {
              emit3_o (offset ? A_RL : A_SLA, AOP (shiftoperand), offset, 0, 0);

              offset++;
            }
          if (shCount > 1)
            {
              if (!regalloc_dry_run)
                {
                  emitLabel (tlbl1);
                  emit2 ("dec a");
                  emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
                }
              regalloc_dry_run_cost += 4;
            }
        }
    }

  if (shiftoperand != result)
    {
      if (isPair (AOP (result)) && !offr)
        fetchPairLong (getPairId (AOP (result)), AOP(shiftoperand), ic, offl);
      else if (isPair (AOP (shiftoperand)))
        commitPair (AOP (result), getPairId (AOP (shiftoperand)), ic, FALSE);
      else
        {
          /* Copy left into result */
          movLeft2Result (shiftoperand, offl, result, offr, 0);
          movLeft2Result (shiftoperand, offl + 1, result, offr + 1, 0);
        }
    }
}

/*-----------------------------------------------------------------*/
/* AccRol - rotate left accumulator by known count                 */
/*-----------------------------------------------------------------*/
static void
AccRol (int shCount)
{
  shCount &= 0x0007;            // shCount : 0..7

  switch (shCount)
    {
    case 4:
      if (IS_GB)
        {
          emit3 (A_SWAP, ASMOP_A, 0);
          break;
        }
      emit3 (A_RLCA, 0, 0);
    case 3:
      if (IS_GB)
        {
          emit3 (A_SWAP, ASMOP_A, 0);
          emit3 (A_RRCA, 0, 0);
          break;
        }
      emit3 (A_RLCA, 0, 0);
    case 2:
      emit3 (A_RLCA, 0, 0);
    case 1:
      emit3 (A_RLCA, 0, 0);
    case 0:
      break;
    case 5:
      if (IS_GB)
        {
          emit3 (A_SWAP, ASMOP_A, 0);
          emit3 (A_RLCA, 0, 0);
          break;
        }
      emit3 (A_RRCA, 0, 0);
    case 6:
      emit3 (A_RRCA, 0, 0);
    case 7:
      emit3 (A_RRCA, 0, 0);
      break;
    }
}

/*-----------------------------------------------------------------*/
/* AccLsh - left shift accumulator by known count                  */
/*-----------------------------------------------------------------*/
static void
AccLsh (unsigned int shCount)
{
  static const unsigned char SLMask[] =
  {
    0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00
  };

  if (shCount <= 3 + !IS_GB)
    while (shCount--)
      emit3 (A_ADD, ASMOP_A, ASMOP_A);
  else
    {
      /* rotate left accumulator */
      AccRol (shCount);
      /* and kill the lower order bits */
      emit2 ("and a, !immedbyte", SLMask[shCount]);
      regalloc_dry_run_cost += 2;
    }
}

/*-----------------------------------------------------------------*/
/* shiftL1Left2Result - shift left one byte from left to result    */
/*-----------------------------------------------------------------*/
static void
shiftL1Left2Result (operand *left, int offl, operand *result, int offr, unsigned int shCount, const iCode *ic)
{
  /* If operand and result are the same we can shift in place.
     However shifting in acc using add is cheaper than shifting
     in place using sla; when shifting by more than 2 shifting in
     acc it is worth the additional effort for loading from / to acc. */
  if (!aopInReg(result->aop, 0, A_IDX) && sameRegs (AOP (left), AOP (result)) && shCount <= 2 && offr == offl)
    {
      while (shCount--)
        emit3 (A_SLA, AOP (result), 0);
    }
  else if ((IS_Z180 && !optimize.codeSpeed || IS_EZ80_Z80) && // Try to use mlt
    (aopInReg (result->aop, offr, C_IDX) && isPairDead(PAIR_BC, ic) || aopInReg (result->aop, offr, E_IDX) && isPairDead(PAIR_DE, ic) || aopInReg (result->aop, offr, L_IDX) && isPairDead(PAIR_HL, ic)))
    {
      PAIR_ID pair = aopInReg (result->aop, offr, C_IDX) ? PAIR_BC : (aopInReg (result->aop, offr, E_IDX) ? PAIR_DE : PAIR_HL);

      bool top = aopInReg (left->aop, offl, _pairs[pair].h_idx);
      if (!top)
        cheapMove (pair == PAIR_BC ? ASMOP_C : (pair == PAIR_DE ? ASMOP_E : ASMOP_L), 0, left->aop, offl, !bitVectBitValue (ic->rSurv, A_IDX));

      emit2 ("ld %s, !immed%d", top ? _pairs[pair].l : _pairs[pair].h, 1 << shCount);
      emit2 ("mlt %s", _pairs[pair].name);
      regalloc_dry_run_cost += 4;
    }
  else
    {
      if (bitVectBitValue (ic->rSurv, A_IDX))
        _push (PAIR_AF);
      cheapMove (ASMOP_A, 0, left->aop, offl, true);
      /* shift left accumulator */
      AccLsh (shCount);
      cheapMove (AOP (result), offr, ASMOP_A, 0, true);
      if (bitVectBitValue (ic->rSurv, A_IDX))
        _pop (PAIR_AF);
    }
}

/*-----------------------------------------------------------------*/
/* genlshTwo - left shift two bytes by known amount                */
/*-----------------------------------------------------------------*/
static void
genlshTwo (operand *result, operand *left, unsigned int shCount, const iCode *ic)
{
  int size = AOP_SIZE (result);

  wassert (size == 2);

  /* if shCount >= 8 */
  if (shCount >= 8)
    {
      shCount -= 8;
      if (size > 1)
        {
          if (shCount)
            shiftL1Left2Result (left, 0, result, 1, shCount, ic);
          else
            movLeft2Result (left, LSB, result, MSB16, 0);
        }
      cheapMove (result->aop, 0, ASMOP_ZERO, 0, true);
    }
  /*  0 <= shCount <= 7 */
  else
    {
      if (size == 1)
        {
          wassert (0);
        }
      else
        {
          shiftL2Left2Result (left, LSB, result, LSB, shCount, ic);
        }
    }
}

/*------------------------------------------------------------------*/
/* genLeftShiftLiteral - left shifting by known count for size <= 2 */
/*------------------------------------------------------------------*/
static void
genLeftShiftLiteral (operand *left, operand *right, operand *result, const iCode *ic)
{
  unsigned int shCount = ulFromVal (AOP (right)->aopu.aop_lit);
  unsigned int size;

  freeAsmop (right, NULL);

  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, FALSE);

  size = getSize (operandType (result));

  /* I suppose that the left size >= result size */
  wassert (getSize (operandType (left)) >= size);

  if (shCount >= (size * 8))
    genMove (result->aop, ASMOP_ZERO, !bitVectBitValue (ic->rSurv, A_IDX), isPairDead (PAIR_HL, ic) && left->aop->regs[H_IDX] < 0 && left->aop->regs[L_IDX] < 0);
  else
    {
      switch (size)
        {
        case 1:
          shiftL1Left2Result (left, 0, result, 0, shCount, ic);
          break;
        case 2:
          genlshTwo (result, left, shCount, ic);
          break;
        case 4:
          wassertl (0, "Shifting of longs should be handled by generic function.");
          break;
        default:
          wassert (0);
        }
    }
  freeAsmop (left, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genLeftShift - generates code for left shifting                 */
/*-----------------------------------------------------------------*/
static void
genLeftShift (const iCode *ic)
{
  int size, offset;
  symbol *tlbl = 0, *tlbl1 = 0;
  operand *left, *right, *result;
  int countreg;
  bool shift_by_lit;
  int shiftcount = 0;
  int byteshift = 0;
  bool started;
  bool save_a;

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic, FALSE, FALSE);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (AOP_TYPE (right) == AOP_LIT && getSize (operandType (result)) <= 2)
    {
      genLeftShiftLiteral (left, right, result, ic);
      freeAsmop (right, NULL);
      return;
    }

  /* Useful for the case of shifting a size > 2 value by a literal */
  shift_by_lit = AOP_TYPE (right) == AOP_LIT;
  if (shift_by_lit)
    shiftcount = ulFromVal (AOP (right)->aopu.aop_lit);

  aopOp (result, ic, FALSE, FALSE);
  aopOp (left, ic, FALSE, FALSE);

  if (AOP_TYPE (right) == AOP_REG && !bitVectBitValue (ic->rSurv, AOP (right)->aopu.aop_reg[0]->rIdx) && AOP (right)->aopu.aop_reg[0]->rIdx != IYL_IDX && (sameRegs (AOP (left), AOP (result)) || AOP_TYPE (left) != AOP_REG) &&
    (AOP_TYPE (result) != AOP_REG ||
    AOP (result)->aopu.aop_reg[0]->rIdx != AOP (right)->aopu.aop_reg[0]->rIdx &&
    !aopInReg (AOP (result), 0, AOP (right)->aopu.aop_reg[0]->rIdx) && !aopInReg (AOP (result), 1, AOP (right)->aopu.aop_reg[0]->rIdx) && !aopInReg (AOP (result), 2, AOP (right)->aopu.aop_reg[0]->rIdx) && !aopInReg (AOP (result), 3, AOP (right)->aopu.aop_reg[0]->rIdx)))
    countreg = AOP (right)->aopu.aop_reg[0]->rIdx;
  else if (!IS_GB && !bitVectBitValue (ic->rSurv, B_IDX) && (sameRegs (AOP (left), AOP (result)) || AOP_TYPE (left) != AOP_REG || shift_by_lit) &&
    !aopInReg (AOP (result), 0, B_IDX) && !aopInReg (AOP (result), 1, B_IDX) && !aopInReg (AOP (result), 2, B_IDX) && !aopInReg (AOP (result), 3, B_IDX))
    countreg = B_IDX;
  else
    countreg = A_IDX;

  if (!shift_by_lit)
    cheapMove (asmopregs[countreg], 0, right->aop, 0, true);

  save_a = (countreg == A_IDX && !shift_by_lit) &&
    !(AOP_TYPE (left) == AOP_REG && AOP_TYPE (result) != AOP_REG ||
    !IS_GB && (AOP_TYPE (left) == AOP_STK && canAssignToPtr3 (result->aop) || AOP_TYPE (result) == AOP_STK && canAssignToPtr3 (left->aop)));

  /* now move the left to the result if they are not the
     same */
  if (!sameRegs (AOP (left), AOP (result)))
    {
      if (save_a)
        _push (PAIR_AF);

      if (shift_by_lit)
      {
        byteshift = shiftcount / 8;
        shiftcount %= 8;
      }
      size = AOP_SIZE (result) - byteshift;
      int lsize = AOP_SIZE (left) - byteshift;

      genMove_o (result->aop, byteshift, left->aop, 0, size <= lsize ? size : lsize, save_a || countreg != A_IDX, false);

      genMove_o (result->aop, 0, ASMOP_ZERO, 0, byteshift, save_a || countreg != A_IDX, false);

      if (save_a)
        _pop (PAIR_AF);
    }

  if (!regalloc_dry_run)
    {
      tlbl = newiTempLabel (NULL);
      tlbl1 = newiTempLabel (NULL);
    }
  size = AOP_SIZE (result);
  offset = 0;

  if (shift_by_lit && !shiftcount)
    goto end;
  if (shift_by_lit && shiftcount > 1)
    {
      emit2 ("ld %s, !immedbyte", countreg == A_IDX ? "a" : regsZ80[countreg].name, shiftcount);
      regalloc_dry_run_cost += 2;
    }
  else if (!shift_by_lit)
    {
      emit2 ("inc %s", countreg == A_IDX ? "a" : regsZ80[countreg].name);
      regalloc_dry_run_cost += 1;
      if (!regalloc_dry_run)
        emit2 ("jp !tlabel", labelKey2num (tlbl1->key));
      regalloc_dry_run_cost += 3;
    }
  if (!(shift_by_lit && shiftcount == 1) && !regalloc_dry_run)
    emitLabel (tlbl);

  if (requiresHL (AOP (result)))
    spillPair (PAIR_HL);

  started = false;
  while (size)
    {
      if (size >= 2 && offset + 1 >= byteshift &&
         AOP_TYPE (result) == AOP_REG &&
        (getPartPairId (AOP (result), offset) == PAIR_HL ||
         IS_RAB && getPartPairId (AOP (result), offset) == PAIR_DE))
        {
          if (AOP (result)->aopu.aop_reg[offset]->rIdx == L_IDX)
          {
            emit2 (started ? "adc hl, hl" : "add hl, hl");
            regalloc_dry_run_cost += 1 + started;
          }
          else
          {
            if (!started)
              emit3 (A_CP, ASMOP_A, ASMOP_A);
            emit2 ("rl de");
            regalloc_dry_run_cost++;
          }
          
          started = true;
          size -= 2, offset += 2;
        }
      else
        {
          if (offset >= byteshift)
            {
              if (aopInReg (AOP (result) , offset, A_IDX))
                emit3 (started ? A_ADC : A_ADD, ASMOP_A, ASMOP_A);
              else
                emit3_o (started ? A_RL : A_SLA, AOP (result), offset, 0, 0);
              started = true;
            }
          size--, offset++;
        }
    }

  if (!(shift_by_lit && shiftcount == 1))
    {
      if (!regalloc_dry_run)
        emitLabel (tlbl1);
      if (!IS_GB && countreg == B_IDX)
        {
          if (!regalloc_dry_run)
            emit2 ("djnz !tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 2;
        }
      else
        {
          emit2 ("dec %s", countreg == A_IDX ? "a" : regsZ80[countreg].name);
          if (!regalloc_dry_run)
            emit2 ("jr NZ,!tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 3;
        }
    }

end:
  if (!shift_by_lit && requiresHL (AOP (result))) // Shift by 0 skips over hl adjustments.
    spillPair (PAIR_HL);

  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* AccRsh - right shift accumulator by known count                 */
/*-----------------------------------------------------------------*/
static void
AccRsh (int shCount)
{
  if (shCount >= 2)
    {
      /* rotate right accumulator */
      AccRol (8 - shCount);
      /* and kill the higher order bits */
      if (!regalloc_dry_run)
        emit2 ("and a, !immedbyte", 0xff >> shCount);
      regalloc_dry_run_cost += 2;
    }
  else if(shCount)
    emit3 (A_SRL, ASMOP_A, 0);
}

/*-----------------------------------------------------------------*/
/* genrshOne - right shift one byte by known amount                */
/*-----------------------------------------------------------------*/
static void
genrshOne (operand *result, operand *left, int shCount, int is_signed, const iCode *ic)
{
  /* Errk */
  int size = AOP_SIZE (result);

  wassert (size == 1);

  bool a_dead = !bitVectBitValue (ic->rSurv, A_IDX);
  
  if ((IS_Z180 || IS_EZ80_Z80) && !is_signed && shCount >= 3 && shCount <= 6 + a_dead && // Try to use mlt.
    (aopInReg (result->aop, 0, B_IDX) && isPairDead(PAIR_BC, ic) || aopInReg (result->aop, 0, D_IDX) && isPairDead(PAIR_DE, ic) || aopInReg (result->aop, 0, H_IDX) && isPairDead(PAIR_HL, ic)))
    {
      PAIR_ID pair = aopInReg (result->aop, 0, B_IDX) ? PAIR_BC : (aopInReg (result->aop, 0, D_IDX) ? PAIR_DE : PAIR_HL);
      bool top = aopInReg (left->aop, 0, _pairs[pair].h_idx);
      if (!top)
        cheapMove (pair == PAIR_BC ? ASMOP_C : (pair == PAIR_DE ? ASMOP_E : ASMOP_L), 0, left->aop, 0, a_dead);

      emit2 ("ld %s, !immed%d", top ? _pairs[pair].l : _pairs[pair].h, 1 << (8 - shCount));
      emit2 ("mlt %s", _pairs[pair].name);
      regalloc_dry_run_cost += 4;
    }
  else if (!is_signed && // Shifting in the accumulator is cheap for unsigned operands.
    (aopInReg (result->aop, 0, A_IDX) ||
    result->aop->type != AOP_REG ||
    (shCount >= 4 + 2 * a_dead || shCount >= 2 * a_dead && aopInReg (left->aop, 0, A_IDX))))
    {
      if (!a_dead)
        _push (PAIR_AF);
      cheapMove (ASMOP_A, 0, left->aop, 0, true);
      AccRsh (shCount);
      cheapMove (result->aop, 0, ASMOP_A, 0, true);
      if (!a_dead)
        _pop (PAIR_AF);
    }
  else if (AOP (result)->type == AOP_REG) // Can shift in destination for register result.
    {
      cheapMove (AOP (result), 0, AOP (left), 0, a_dead);

      while (shCount--)
        emit3 (is_signed ? A_SRA : A_SRL, result->aop, 0);
    }
  else
    {
      if (!a_dead)
        _push (PAIR_AF);
      cheapMove (ASMOP_A, 0, left->aop, 0, true);
      while (shCount--)
        emit3 (is_signed ? A_SRA : A_SRL, ASMOP_A, 0);
      cheapMove (result->aop, 0, ASMOP_A, 0, true);
      if (!a_dead)
        _pop (PAIR_AF);
    }
}

/*-----------------------------------------------------------------*/
/* shiftR1Left2Result - shift right one byte from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftR1Left2Result (operand *left, int offl, operand *result, int offr, int shCount, int sign)
{
  cheapMove (ASMOP_A, 0, left->aop, offl, true);
  if (sign)
    {
      while (shCount--)
        emit3 (sign ? A_SRA : A_SRL, ASMOP_A, 0);
    }
  else
    AccRsh (shCount);
  cheapMove (result->aop, offr, ASMOP_A, 0, true);
}

/*-----------------------------------------------------------------*/
/* genrshTwo - right shift two bytes by known amount               */
/*-----------------------------------------------------------------*/
static void
genrshTwo (const iCode * ic, operand * result, operand * left, int shCount, int sign)
{
  /* if shCount >= 8 */
  if (shCount >= 8)
    {
      shCount -= 8;
      if (shCount)
        {
          shiftR1Left2Result (left, MSB16, result, LSB, shCount, sign);
        }
      else
        {
          movLeft2Result (left, MSB16, result, LSB, sign);
        }
      if (sign)
        {
          /* Sign extend the result */
          cheapMove (ASMOP_A, 0, result->aop, 0, true);
          emit3 (A_RLC, ASMOP_A, 0);
          emit3 (A_SBC, ASMOP_A, ASMOP_A);
          cheapMove (result->aop, 1, ASMOP_A, 0, true);
        }
      else
        cheapMove (result->aop, 1, ASMOP_ZERO, 0, true);
    }
  /*  0 <= shCount <= 7 */
  else
    shiftR2Left2Result (ic, left, LSB, result, LSB, shCount, sign);
}

/*-----------------------------------------------------------------*/
/* genRightShiftLiteral - right shifting by known count              */
/*-----------------------------------------------------------------*/
static void
genRightShiftLiteral (operand * left, operand * right, operand * result, const iCode *ic, int sign)
{
  unsigned int shCount = (unsigned int) ulFromVal (AOP (right)->aopu.aop_lit);
  unsigned int size;

  freeAsmop (right, NULL);

  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, FALSE);

  size = getSize (operandType (result));

  /* I suppose that the left size >= result size */
  wassert (getSize (operandType (left)) >= size);

  if (shCount >= (size * 8))
    {
      if (!SPEC_USIGN (getSpec (operandType (left))))
        {
          cheapMove (ASMOP_A, 0, left->aop, 0, true);
          emit3 (A_RLC, ASMOP_A, 0);
          emit3 (A_SBC, ASMOP_A, ASMOP_A);
          while (size--)
            cheapMove (result->aop, size, ASMOP_A, 0, true);
        }
      else
        genMove (result->aop, ASMOP_ZERO, !bitVectBitValue (ic->rSurv, A_IDX), isPairDead (PAIR_HL, ic));
    }
  else
    {
      switch (size)
        {
        case 1:
          genrshOne (result, left, shCount, sign, ic);
          break;
        case 2:
          genrshTwo (ic, result, left, shCount, sign);
          break;
        case 4:
          wassertl (0, "Asked to shift right a long which should be handled in generic right shift function.");
          break;
        default:
          wassertl (0, "Entered default case in right shift delegate");
        }
    }
  freeAsmop (left, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genRightShift - generate code for right shifting                */
/*-----------------------------------------------------------------*/
static void
genRightShift (const iCode * ic)
{
  operand *right, *left, *result;
  sym_link *retype;
  int size, offset, first = 1;
  bool is_signed;
  int countreg;
  bool shift_by_lit, shift_by_one, shift_by_zero;
  int shiftcount = 0;
  int byteoffset = 0;
  bool save_a;

  symbol *tlbl = 0, *tlbl1 = 0;

  /* if signed then we do it the hard way preserve the
     sign bit moving it inwards */
  retype = getSpec (operandType (IC_RESULT (ic)));

  is_signed = !SPEC_USIGN (retype);

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic, FALSE, FALSE);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (AOP_TYPE (right) == AOP_LIT && getSize (operandType (result)) <= 2)
    {
      genRightShiftLiteral (left, right, result, ic, is_signed);
      freeAsmop (right, NULL);
      return;
    }

  /* Useful for the case of shifting a size > 2 value by a literal */
  shift_by_lit = AOP_TYPE (right) == AOP_LIT;
  if (shift_by_lit)
    shiftcount = ulFromVal (AOP (right)->aopu.aop_lit);

  aopOp (result, ic, FALSE, FALSE);
  aopOp (left, ic, FALSE, FALSE);

  if (AOP_TYPE (right) == AOP_REG && !bitVectBitValue (ic->rSurv, AOP (right)->aopu.aop_reg[0]->rIdx) && AOP (right)->aopu.aop_reg[0]->rIdx != IYL_IDX && (sameRegs (AOP (left), AOP (result)) || AOP_TYPE (left) != AOP_REG) &&
    (AOP_TYPE (result) != AOP_REG ||
    AOP (result)->aopu.aop_reg[0]->rIdx != AOP (right)->aopu.aop_reg[0]->rIdx &&
    (AOP_SIZE (result) < 2 || AOP (result)->aopu.aop_reg[1]->rIdx != AOP (right)->aopu.aop_reg[0]->rIdx &&
    (AOP_SIZE (result) < 3 || AOP (result)->aopu.aop_reg[2]->rIdx != AOP (right)->aopu.aop_reg[0]->rIdx &&
    (AOP_SIZE (result) < 4 || AOP (result)->aopu.aop_reg[3]->rIdx != AOP (right)->aopu.aop_reg[0]->rIdx)))))
    countreg = AOP (right)->aopu.aop_reg[0]->rIdx;
  else if (!IS_GB && !bitVectBitValue (ic->rSurv, B_IDX) && (sameRegs (AOP (left), AOP (result)) || AOP_TYPE (left) != AOP_REG || shift_by_lit) &&
    (AOP_TYPE (result) != AOP_REG ||
    AOP (result)->aopu.aop_reg[0]->rIdx != B_IDX &&
    (AOP_SIZE (result) < 2 || AOP (result)->aopu.aop_reg[1]->rIdx != B_IDX &&
    (AOP_SIZE (result) < 3 || AOP (result)->aopu.aop_reg[2]->rIdx != B_IDX &&
    (AOP_SIZE (result) < 4 || AOP (result)->aopu.aop_reg[3]->rIdx != B_IDX)))))
    countreg = B_IDX;
  else
    countreg = A_IDX;

  if (!shift_by_lit)
    cheapMove (countreg == A_IDX ? ASMOP_A : asmopregs[countreg], 0, right->aop, 0, true);

  save_a = (countreg == A_IDX && !shift_by_lit) &&
    !(AOP_TYPE (left) == AOP_REG && AOP_TYPE (result) != AOP_REG ||
    !IS_GB && (AOP_TYPE (left) == AOP_STK && canAssignToPtr3 (result->aop) || AOP_TYPE (result) == AOP_STK && canAssignToPtr3 (left->aop)));

  /* now move the left to the result if they are not the
     same */
  if (!sameRegs (AOP (left), AOP (result)))
    {
      int soffset = 0;
      size = AOP_SIZE (result);

      if (!is_signed && shift_by_lit)
      {
        byteoffset = shiftcount / 8;
        shiftcount %= 8;
        soffset = byteoffset;
        size -= byteoffset;
      }

      if (save_a)
        _push (PAIR_AF);

      genMove_o (result->aop, 0, left->aop, soffset, size, true, isPairDead (PAIR_HL, ic));

      genMove_o (result->aop, result->aop->size - byteoffset, ASMOP_ZERO, 0, byteoffset, true, false);

      if (save_a)
        _pop (PAIR_AF);
    }

  shift_by_one = (shift_by_lit && shiftcount == 1);
  shift_by_zero = (shift_by_lit && shiftcount == 0);

  if (!regalloc_dry_run)
    {
      tlbl = newiTempLabel (NULL);
      tlbl1 = newiTempLabel (NULL);
    }
  size = AOP_SIZE (result);
  offset = size - 1;

  if (shift_by_zero)
    goto end;
  else if (shift_by_lit && shiftcount > 1)
    {
      emit2 ("ld %s, !immedbyte", countreg == A_IDX ? "a" : regsZ80[countreg].name, shiftcount);
      regalloc_dry_run_cost += 2;
    }
  else if (!shift_by_lit)
    {
      emit2 ("inc %s", countreg == A_IDX ? "a" : regsZ80[countreg].name);
      regalloc_dry_run_cost += 1;
      if (!regalloc_dry_run)
        emit2 ("jp !tlabel", labelKey2num (tlbl1->key));
      regalloc_dry_run_cost += 3;
    }
  if (!shift_by_one && !regalloc_dry_run)
    IS_GB ? emitLabelSpill (tlbl) : emitLabel (tlbl);

  if (requiresHL (AOP (result)))
    spillPair (PAIR_HL);

  while (size)
    { 
      if (IS_RAB && !(is_signed && first) && size >= 2 && byteoffset < 2 && AOP_TYPE (result) == AOP_REG &&
        (getPartPairId (AOP (result), offset - 1) == PAIR_HL || getPartPairId (AOP (result), offset - 1) == PAIR_DE))
        {
          if (first)
            {
              emit3 (A_CP, ASMOP_A, ASMOP_A);
              first = 0;
            }
          emit2 (AOP (result)->aopu.aop_reg[offset - 1]->rIdx == L_IDX ? "rr hl" : "rr de");
          regalloc_dry_run_cost++;
          size -= 2, offset -= 2;
        }
      else if (!is_signed && first && byteoffset--) // Skip known 0 bytes
        size--, offset--;
      else if (first)
        {
          emit3_o (is_signed ? A_SRA : A_SRL, AOP (result), offset, 0, 0);
          first = 0;
          size--, offset--;
        }
      else
        {
          emit3_o (A_RR, AOP (result), offset, 0, 0);
          size--, offset--;
        }
    }

  if (!shift_by_one)
    {
      if (!regalloc_dry_run)
        emitLabel (tlbl1);
      if (!IS_GB && countreg == B_IDX)
        {
          if (!regalloc_dry_run)
            emit2 ("djnz !tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 2;
        }
      else
        {
          emit2 ("dec %s", countreg == A_IDX ? "a" : regsZ80[countreg].name);
          if (!regalloc_dry_run)
            emit2 ("jr NZ, !tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 3;
        }
    }

end:
  if (!shift_by_lit && requiresHL (AOP (result))) // Shift by 0 skips over hl adjustments.
    spillPair (PAIR_HL);

  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* unpackMaskA - generate masking code for unpacking last byte     */
/* of bitfiled. And mask for unsigned, sign extension for signed.  */
/*-----------------------------------------------------------------*/
static void
unpackMaskA(sym_link *type, int len)
{
  if (SPEC_USIGN (type) || len != 1)
    {
      emit2 ("and a, !immedbyte", ((unsigned char) - 1) >> (8 - len));
      regalloc_dry_run_cost += 2;
    }
  if (!SPEC_USIGN (type))
    {
      if (len == 1)
        {
          emit3(A_RRA, 0, 0);
          emit3(A_SBC, ASMOP_A, ASMOP_A);
        }
      else
        {
          if (!regalloc_dry_run)
            {
              symbol *tlbl = newiTempLabel (NULL);
              emit2 ("bit %d, a", len - 1);
              emit2 ("jp Z, !tlabel", labelKey2num (tlbl->key));
              emit2 ("or a, !immedbyte", (unsigned char) (0xff << len));
              emitLabel (tlbl);
            }
          regalloc_dry_run_cost += 7;
        }
    }
}

/*-----------------------------------------------------------------*/
/* genUnpackBits - generates code for unpacking bits               */
/*-----------------------------------------------------------------*/
static void
genUnpackBits (operand * result, int pair, const iCode *ic)
{
  int offset = 0;               /* result byte offset */
  int rsize;                    /* result size */
  int rlen = 0;                 /* remaining bit-field length */
  sym_link *etype;              /* bit-field type information */
  unsigned blen;                /* bit-field length */
  unsigned bstr;                /* bit-field starting bit within byte */
  unsigned int pairincrement = 0;

  emitDebug ("; genUnpackBits");

  etype = getSpec (operandType (result));
  rsize = getSize (operandType (result));
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  /* If the bit-field length is less than a byte */
  if (blen < 8)
    {
      emit2 ("ld a, !*pair", _pairs[pair].name);
      regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;
      AccRol (8 - bstr);
      unpackMaskA (etype, blen);
      cheapMove (AOP (result), offset++, ASMOP_A, 0, true);
      goto finish;
    }

  /* TODO: what if pair == PAIR_DE ? */
  if (getPairId (AOP (result)) == PAIR_HL ||
      AOP_TYPE (result) == AOP_REG && rsize >= 2 && (AOP (result)->aopu.aop_reg[0]->rIdx == L_IDX
          || AOP (result)->aopu.aop_reg[0]->rIdx == H_IDX))
    {
      wassertl (rsize == 2, "HL must be of size 2");
      emit2 ("ld a, !*hl");
      emit2 ("inc hl");
      if (AOP_TYPE (result) != AOP_REG || AOP (result)->aopu.aop_reg[0]->rIdx != H_IDX)
        {
          emit2 ("ld h, !*hl");
          cheapMove (AOP (result), offset++, ASMOP_A, 0, true);
          emit2 ("ld a, h");
        }
      else
        {
          emit2 ("ld l, !*hl");
          cheapMove (AOP (result), offset++, ASMOP_A, 0, true);
          emit2 ("ld a, l");
        }
      regalloc_dry_run_cost += 5;
      unpackMaskA (etype, blen - 8);
      cheapMove (AOP (result), offset++, ASMOP_A, 0, true);
      regalloc_dry_run_cost += 1;
      spillPair (PAIR_HL);
      return;
    }

  /* Bit field did not fit in a byte. Copy all
     but the partial byte at the end.  */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      emit2 ("ld a, !*pair", _pairs[pair].name);
      regalloc_dry_run_cost += 1;
      cheapMove (AOP (result), offset++, ASMOP_A, 0, true);
      if (rlen > 8)
        {
          emit2 ("inc %s", _pairs[pair].name);
          regalloc_dry_run_cost += 1;
          _G.pairs[pair].offset++;
          pairincrement++;
        }
    }

  /* Handle the partial byte at the end */
  if (rlen)
    {
      emit2 ("ld a, !*pair", _pairs[pair].name);
      regalloc_dry_run_cost++;
      unpackMaskA (etype, rlen);
      cheapMove (AOP (result), offset++, ASMOP_A, 0, true);
    }

finish:
  if (!isPairDead(pair, ic))
    while (pairincrement)
      {
        emit2 ("dec %s", _pairs[pair].name);
        regalloc_dry_run_cost += 1;
        pairincrement--;
        _G.pairs[pair].offset--;
      }

  if (offset < rsize)
    {
      asmop *source;

      if (SPEC_USIGN (etype))
        source = ASMOP_ZERO;
      else
        {
          /* signed bit-field: sign extension with 0x00 or 0xff */
          emit3 (A_RLA, 0, 0);
          emit3 (A_SBC, ASMOP_A, ASMOP_A);
          source = ASMOP_A;
        }
      rsize -= offset;
      while (rsize--)
        cheapMove (AOP (result), offset++, source, 0, true);
    }
}

static void
_moveFrom_tpair_ (asmop * aop, int offset, PAIR_ID pair)
{
  emitDebug ("; _moveFrom_tpair_()");
  if (pair == PAIR_HL && aop->type == AOP_REG)
    {
      if (!regalloc_dry_run)
        aopPut (aop, "!*hl", offset);
      regalloc_dry_run_cost += ld_cost (aop, ASMOP_A);
    }
  else
    {
      emit2 ("ld a, !*pair", _pairs[pair].name);
      regalloc_dry_run_cost += 1;
      cheapMove (aop, offset, ASMOP_A, 0, true);
    }
}

static void offsetPair (PAIR_ID pair, PAIR_ID extrapair, bool save_extrapair, int val)
{
  if (abs (val) < (save_extrapair ? 6 : 4) || pair != PAIR_HL && pair != PAIR_IY && pair != PAIR_IX)
    {
      while (val)
        {
          emit2 (val > 0 ? "inc %s" : "dec %s", _pairs[pair].name);
          if (val > 0)
            val--;
          else
            val++;
          regalloc_dry_run_cost++;
        }
    }
  else
    {
      if (save_extrapair)
        _push (extrapair);
      emit2 ("ld %s, !immedword", _pairs[extrapair].name, val);
      emit2 ("add %s, %s", _pairs[pair].name, _pairs[extrapair].name);
      regalloc_dry_run_cost += (pair == PAIR_HL ? 4 : 5);
      if (save_extrapair)
        _pop (extrapair);
    }
}

/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void
genPointerGet (const iCode *ic)
{
  operand *left, *right, *result;
  int size, offset, rightval;
  int pair = PAIR_HL, extrapair;
  sym_link *retype;
  bool pushed_pair = FALSE;
  bool pushed_a = FALSE;
  bool surviving_a = !options.oldralloc && bitVectBitValue (ic->rSurv, A_IDX);
  bool rightval_in_range;

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  retype = getSpec (operandType (result));

  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, FALSE);
  size = AOP_SIZE (result);

  /* Historically GET_VALUE_AT_ADDRESS didn't have a right operand */
  wassertl (right, "GET_VALUE_AT_ADDRESS without right operand");
  wassertl (IS_OP_LITERAL (IC_RIGHT (ic)), "GET_VALUE_AT_ADDRESS with non-literal right operand");
  rightval = (int)operandLitValue (right);
  rightval_in_range = (rightval >= -128 && rightval + size - 1 < 127);
  if (IS_GB)
    wassert (!rightval);

  if (IS_GB && left->aop->type == AOP_STK) // Try to avoid (hl) to hl copy, which requires 3 instructions and free a.
    pair = PAIR_DE;
  if ((IS_GB || IY_RESERVED) && requiresHL (AOP (result)) && size > 1 && AOP_TYPE (result) != AOP_REG)
    pair = PAIR_DE;

  if (AOP_TYPE (left) == AOP_IMMD && size == 1 && aopInReg (result->aop, 0, A_IDX) && !IS_BITVAR (retype))
    {
      emit2 ("ld a, (%s)", aopGetLitWordLong (AOP (left), rightval, TRUE));
      regalloc_dry_run_cost += 3;
      goto release;
    }
  else if (!IS_GB && AOP_TYPE (left) == AOP_IMMD && isPair (AOP (result)) && !IS_BITVAR (retype))
    {
      PAIR_ID pair = getPairId (AOP (result));
      emit2 ("ld %s, (%s)", _pairs[pair].name, aopGetLitWordLong (AOP (left), rightval, TRUE));
      regalloc_dry_run_cost += (pair == PAIR_HL ? 3 : 4);
      goto release;
    }
  else if (!IS_GB && AOP_TYPE (left) == AOP_IMMD && getPartPairId (AOP (result), 0) != PAIR_INVALID && getPartPairId (AOP (result), 2) != PAIR_INVALID)
   {
      PAIR_ID pair;
      pair = getPartPairId (AOP (result), 0);
      emit2 ("ld %s, (%s)", _pairs[pair].name, aopGetLitWordLong (AOP (left), rightval, TRUE));
      regalloc_dry_run_cost += (pair == PAIR_HL ? 3 : 4);
      pair = getPartPairId (AOP (result), 2);
      emit2 ("ld %s, (%s)", _pairs[pair].name, aopGetLitWordLong (AOP (left), rightval + 2, TRUE));
      regalloc_dry_run_cost += (pair == PAIR_HL ? 3 : 4);
      goto release;
   }

  if (isPair (AOP (left)) && size == 1 && !IS_BITVAR (retype) && !rightval)
    {
      /* Just do it */
      if (isPtrPair (AOP (left)))
        {
          if (!regalloc_dry_run)        // Todo: More exact cost.
            {
              struct dbuf_s dbuf;

              dbuf_init (&dbuf, 128);
              dbuf_tprintf (&dbuf, "!*pair", getPairName (AOP (left)));
              aopPut (AOP (result), dbuf_c_str (&dbuf), 0);
              dbuf_destroy (&dbuf);
            }
          regalloc_dry_run_cost += ld_cost (AOP (result), ASMOP_A);
        }
      else
        {
          if (surviving_a && !pushed_a)
            _push (PAIR_AF), pushed_a = TRUE;
          emit2 ("ld a, !*pair", getPairName (AOP (left)));
          regalloc_dry_run_cost += (getPairId (AOP (left)) == PAIR_IY ? 3 : 1);
          cheapMove (AOP (result), 0, ASMOP_A, 0, true);
        }

      goto release;
    }

  if (getPairId (AOP (left)) == PAIR_IY && !IS_BITVAR (retype) && rightval_in_range)
    {
      offset = 0;

      if ((IS_RAB || IS_TLCS90) && getPartPairId (AOP (result), 0) == PAIR_HL)
        {
          emit2 ("ld hl, %d (iy)", rightval);
          regalloc_dry_run_cost += 3;
          offset = 2;
          size -= 2;
        }
      else if (IS_EZ80_Z80 && getPartPairId (AOP (result), 0) != PAIR_INVALID)
        {
          emit2 ("ld %s, %d (iy)", _pairs[getPartPairId (AOP (result), 0)].name, rightval);
          regalloc_dry_run_cost += 3;
          offset = 2;
          size -= 2;
        }

      if (!size)
        goto release;

      /* Just do it */
      if (surviving_a && !pushed_a)
        _push (PAIR_AF), pushed_a = TRUE;

      while (size--)
        {
          if (!regalloc_dry_run)
            {
              struct dbuf_s dbuf;

              dbuf_init (&dbuf, 128);
              dbuf_tprintf (&dbuf, "!*iyx", rightval + offset);
              aopPut (AOP (result), dbuf_c_str (&dbuf), offset);
              dbuf_destroy (&dbuf);
            }
          regalloc_dry_run_cost += ld_cost (AOP (result), ASMOP_A) + 2; // Todo: More exact cost.
          offset++;
        }

      goto release;
    }

  /* Using ldir is cheapest for large memory-to-memory transfers. */
  if (!IS_GB && (AOP_TYPE (result) == AOP_STK || AOP_TYPE (result) == AOP_EXSTK) && size > 2 && (!rightval || AOP_TYPE (left) == AOP_IMMD))
    {
      int fp_offset, sp_offset;

      if(!isPairDead (PAIR_HL, ic))
        _push (PAIR_HL);
      if(!isPairDead (PAIR_DE, ic))
        _push (PAIR_DE);
      if(!isPairDead (PAIR_BC, ic))
        _push (PAIR_BC);

      if (!rightval)
        fetchPair (PAIR_DE, AOP (left));
      else
        {
          emit2 ("ld de, %s", aopGetLitWordLong (AOP (left), rightval, TRUE));
          regalloc_dry_run_cost += 3;
        }

      fp_offset = AOP (result)->aopu.aop_stk + (AOP (result)->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
      emit2 ("ld hl, !immedword", sp_offset);
      emit2 ("add hl, sp");
      emit2 ("ex de, hl");
      emit2 ("ld bc, !immedword", size);
      emit2 ("ldir");
      regalloc_dry_run_cost += 10;
      spillPair (PAIR_HL);
      spillPair (PAIR_DE);
      spillPair (PAIR_BC);

      if(!isPairDead (PAIR_BC, ic))
        _pop (PAIR_BC);
      if(!isPairDead (PAIR_DE, ic))
        _pop (PAIR_DE);
      if(!isPairDead (PAIR_HL, ic))
        _pop (PAIR_HL);

      goto release;
    }

  extrapair = isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC;

  /* For now we always load into temp pair */
  /* if this is rematerializable */
  if (!IS_GB && (getPairId (AOP (left)) == PAIR_BC || getPairId (AOP (left)) == PAIR_DE) && AOP_TYPE (result) == AOP_STK && !rightval
      || getPairId (AOP (left)) == PAIR_IY && SPEC_BLEN (getSpec (operandType (result))) < 8 && rightval_in_range)
    pair = getPairId (AOP (left));
  else
    {
      if (!isPairDead (pair, ic) && size > 1 && (getPairId (AOP (left)) != pair || rightval || IS_BITVAR (retype) || size > 2)) // For simple cases, restoring via dec is cheaper than push / pop.
        _push (pair), pushed_pair = TRUE;
      if (AOP_TYPE(left) == AOP_IMMD)
        {
          emit2 ("ld %s, %s", _pairs[pair].name, aopGetLitWordLong (AOP (left), rightval, TRUE));
          spillPair (pair);
          regalloc_dry_run_cost += 3;
          rightval = 0;
        }
      else
        fetchPair (pair, AOP (left));
    }

  /* if bit then unpack */
  if (IS_BITVAR (retype))
    {
      offsetPair (pair, extrapair, !isPairDead (extrapair, ic), rightval);
      genUnpackBits (result, pair, ic);
      if (rightval)
        spillPair (pair);

      goto release;
    }

 if (isPair (AOP (result)) && IS_EZ80_Z80 && getPairId (AOP (left)) == PAIR_HL && !IS_BITVAR (retype) && !rightval)
   {
     emit2 ("ld %s, (hl)", _pairs[getPairId (AOP (result))].name);
     regalloc_dry_run_cost += 2;
     goto release;
   }
 else if (getPairId (AOP (result)) == PAIR_HL || size == 2 && (aopInReg (result->aop, 0, L_IDX) || aopInReg (result->aop, 0, H_IDX)))
    {
      wassertl (size == 2, "HL must be of size 2");
      if (IS_RAB && getPairId (AOP (result)) == PAIR_HL && rightval_in_range)
        {
          emit2 ("ld hl, %d (hl)", rightval);
          regalloc_dry_run_cost += 3;
        }
      else if (IS_EZ80_Z80 && getPairId (AOP (result)) == PAIR_HL && !rightval)
        {
          emit2 ("ld hl, (hl)");
          regalloc_dry_run_cost += 2;
        }
      else if (aopInReg (result->aop, 1, A_IDX))
        {
          offsetPair (pair, extrapair, !isPairDead (extrapair, ic), rightval + 1);
          emit2 ("ld a, !*hl");
          emit2 ("dec hl");
          if (!regalloc_dry_run)
            aopPut (AOP (result), "!*hl", 0);
          regalloc_dry_run_cost += 3;
        }
      else
        {
          if (surviving_a && !pushed_a)
            _push (PAIR_AF), pushed_a = TRUE;
          offsetPair (pair, extrapair, !isPairDead (extrapair, ic), rightval);
          emit2 ("ld a, !*hl");
          emit2 ("inc hl");
          if (!regalloc_dry_run)
            aopPut (AOP (result), "!*hl", 1);
          regalloc_dry_run_cost += 3;
          cheapMove (result->aop, 0, ASMOP_A, 0, true);
        }
      spillPair (PAIR_HL);
      goto release;
    }

  offsetPair (pair, extrapair, !isPairDead (extrapair, ic), rightval);

  if (pair == PAIR_HL
           || (!IS_GB && (getPairId (AOP (left)) == PAIR_BC || getPairId (AOP (left)) == PAIR_DE)
               && AOP_TYPE (result) == AOP_STK))
    {
      size = AOP_SIZE (result);
      offset = 0;
      int last_offset = 0;

      /* might use ld a,(hl) followed by ld d (iy),a */
      if ((AOP_TYPE (result) == AOP_EXSTK || AOP_TYPE (result) == AOP_STK) && surviving_a && !pushed_a)
        _push (PAIR_AF), pushed_a = TRUE;

      if (size >= 2 && pair == PAIR_HL && AOP_TYPE (result) == AOP_REG)
        {
          int i, l = -10, h = -10, r;
          for (i = 0; i < size; i++)
            {
              if (AOP (result)->aopu.aop_reg[i]->rIdx == L_IDX)
                l = i;
              else if (AOP (result)->aopu.aop_reg[i]->rIdx == H_IDX)
                h = i;
            }

          if (l == -10 && h >= 0 && h < size - 1 || h == -10 && l >= 0 && l < size - 1) // One byte of result somewehere in hl. Just assign it last.
            {
              r = (l == -10 ? h : l);

              while (offset < size)
                {
                  if (offset != r)
                    _moveFrom_tpair_ (AOP (result), offset, pair);

                  if (offset < size)
                    {
                      offset++;
                      emit2 ("inc %s", _pairs[pair].name);
                      regalloc_dry_run_cost += 1;
                      _G.pairs[pair].offset++;
                    }
                }

              for (size = offset; size != r; size--)
                {
                  emit2 ("dec %s", _pairs[pair].name);
                  regalloc_dry_run_cost += 1;
                }

              _moveFrom_tpair_ (AOP (result), r, pair);

              // No fixup since result uses HL.
              spillPair (pair);
              goto release;
            }
          else if (l >= 0 && h >= 0)    // Two bytes of result somewehere in hl. Assign it last and use a for caching.
            {
              while (offset < size)
                {
                  last_offset = offset;

                  if (IS_EZ80_Z80 && offset != l && offset != h && getPairId_o (result->aop, offset) != PAIR_INVALID)
                    {
                      emit2 ("ld %s, !*hl", _pairs[getPairId_o (result->aop, offset)].name);
                      regalloc_dry_run_cost += 2;
                      offset += 2;
                      if (offset < size)
                        {
                          emit2 ("inc %s", _pairs[pair].name);
                          emit2 ("inc %s", _pairs[pair].name);
                          regalloc_dry_run_cost += 2;
                          _G.pairs[pair].offset += 2;
                        }
                      continue;
                    }

                  if (offset != l && offset != h)
                    _moveFrom_tpair_ (AOP (result), offset, pair);
                  offset++;

                  if (offset < size)
                    {
                      emit2 ("inc %s", _pairs[pair].name);
                      regalloc_dry_run_cost += 1;
                      _G.pairs[pair].offset++;
                    }
                }

              r = (l > h ? l : h);
              for (size = last_offset; size != r; size--)
                {
                  emit2 ("dec %s", _pairs[pair].name);
                  regalloc_dry_run_cost += 1;
                }
              if ((surviving_a || result->aop->regs[A_IDX] >= 0 && result->aop->regs[A_IDX] < r) && !pushed_a)
                _push (PAIR_AF), pushed_a = TRUE;
              _moveFrom_tpair_ (ASMOP_A, 0, pair);

              r = (l < h ? l : h);
              for (; size != r; size--)
                {
                  emit2 ("dec %s", _pairs[pair].name);
                  regalloc_dry_run_cost += 1;
                }
              _moveFrom_tpair_ (AOP (result), r, pair);

              r = (l > h ? l : h);
              cheapMove (result->aop, r, ASMOP_A, 0, true);

              // No fixup since result uses HL.
              spillPair (pair);
              goto release;
            }
        }

      while (offset < size)
        {
          last_offset = offset;

          if (IS_EZ80_Z80 && getPairId_o (result->aop, offset) != PAIR_INVALID)
            {
              emit2 ("ld %s, !*hl", _pairs[getPairId_o (result->aop, offset)].name);
              regalloc_dry_run_cost += 2;
              offset += 2;
              if (offset < size)
                {
                  emit2 ("inc %s", _pairs[pair].name);
                  emit2 ("inc %s", _pairs[pair].name);
                  regalloc_dry_run_cost += 2;
                  _G.pairs[pair].offset += 2;
                }
              continue;
            }

          _moveFrom_tpair_ (AOP (result), offset++, pair);

          if (offset < size)
            {
              emit2 ("inc %s", _pairs[pair].name);
              regalloc_dry_run_cost += 1;
              _G.pairs[pair].offset++;
            }
        }
      /* Fixup HL back down */
      if (getPairId (AOP (left)) == pair && !isPairDead (pair, ic) && !pushed_pair)
        while (last_offset --> 0)
          {
            emit2 ("dec %s", _pairs[pair].name);
            regalloc_dry_run_cost += 1;
            _G.pairs[pair].offset--;
          }
       else if (rightval || AOP_SIZE (result))
         spillPair (pair);
    }
  else
    {
      size = AOP_SIZE (result);
      offset = 0;

      for (offset = 0; offset < size;)
        {
          if (surviving_a && !pushed_a)
            _push (PAIR_AF), pushed_a = TRUE;

          /* PENDING: make this better */
          if ((pair == PAIR_HL) && AOP_TYPE (result) == AOP_REG)
            {
              if (!regalloc_dry_run)
                aopPut (AOP (result), "!*hl", offset++);
              regalloc_dry_run_cost += ld_cost (AOP (result), ASMOP_A);
            }
          else
            {
              emit2 ("ld a,!*pair", _pairs[pair].name);
              regalloc_dry_run_cost += 1;
              cheapMove (result->aop, offset++, ASMOP_A, 0, true);
            }
          if (offset < size)
            {
              emit2 ("inc %s", _pairs[pair].name);
              regalloc_dry_run_cost += 1;
              _G.pairs[pair].offset++;
            }
        }
      if (rightval || AOP_SIZE (result))
         spillPair (pair);
    }

release:
  if (pushed_a)
    _pop (PAIR_AF);
  if (pushed_pair)
    _pop (pair);

  freeAsmop (left, NULL);
  freeAsmop (result, NULL);
}

static bool
isRegOrLit (asmop * aop)
{
  if (aop->type == AOP_REG || aop->type == AOP_LIT || aop->type == AOP_IMMD)
    return true;
  return false;
}


/*-----------------------------------------------------------------*/
/* genPackBits - generates code for packed bit storage             */
/*-----------------------------------------------------------------*/
static void
genPackBits (sym_link * etype, operand * right, int pair, const iCode * ic)
{
  int offset = 0;               /* source byte offset */
  int pair_offset = 0;
  int rlen = 0;                 /* remaining bit-field length */
  unsigned blen;                /* bit-field length */
  unsigned bstr;                /* bit-field starting bit within byte */
  int litval;                   /* source literal value (if AOP_LIT) */
  unsigned char mask;           /* bitmask within current byte */
  int extraPair;                /* a tempory register */
  bool needPopExtra = 0;        /* need to restore original value of temp reg */
  unsigned int pairincrement = 0;

  emitDebug ("; genPackBits", "");

  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  /* If the bit-field length is less than a byte */
  if (blen < 8)
    {
      mask = ((unsigned char) (0xFF << (blen + bstr)) | (unsigned char) (0xFF >> (8 - bstr)));

      if (AOP_TYPE (right) == AOP_LIT && blen == 1 && (pair == PAIR_HL || pair == PAIR_IX || pair == PAIR_IY))
        {
          litval = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          emit2 (litval & 1 ? "set %d, !*pair" : "res %d, !*pair", bstr, _pairs[pair].name);
          regalloc_dry_run_cost = (pair == PAIR_IX || pair == PAIR_IY) ? 4 : 2;
          return;
        }
      else if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with a bit-field length <8 and literal source */
          litval = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          litval <<= bstr;
          litval &= (~mask) & 0xff;
          emit2 ("ld a, !*pair", _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;
          if ((mask | litval) != 0xff)
            {
              emit2 ("and a, !immedbyte", mask);
              regalloc_dry_run_cost += 2;
            }
          if (litval)
            {
              emit2 ("or a, !immedbyte", litval);
              regalloc_dry_run_cost += 1;
            }
          emit2 ("ld !*pair,a", _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;
          return;
        }
      else if (blen == 4 && bstr % 4 == 0 && pair == PAIR_HL && !aopInReg (right->aop, 0, A_IDX) && !requiresHL (right->aop) && (IS_Z80 || IS_Z180 || IS_EZ80_Z80))
        {
          emit2 (bstr ? "rld" : "rrd");
          regalloc_dry_run_cost += 2;
          cheapMove (ASMOP_A, 0, AOP (right), 0, true);
          emit2 (bstr ? "rrd" : "rld");
          regalloc_dry_run_cost += 2;
          return;
        }
      else
        {
          /* Case with a bit-field length <8 and arbitrary source */
          cheapMove (ASMOP_A, 0, AOP (right), 0, true);
          /* shift and mask source value */
          if (blen + bstr == 8)
            AccLsh (bstr);
          else
            {
              AccRol (bstr);
              emit2 ("and a, !immedbyte", (~mask) & 0xff);
              regalloc_dry_run_cost += 2;
            }

          extraPair = getFreePairId (ic);
          if (extraPair == PAIR_INVALID)
            {
              if (pair != PAIR_HL)
                extraPair = PAIR_HL;
              else
                {
                  extraPair = PAIR_BC;
                  if (getPairId (AOP (right)) != PAIR_BC || !isLastUse (ic, right))
                    {
                      _push (extraPair);
                      needPopExtra = 1;
                    }
                }
            }
          emit2 ("ld %s, a", _pairs[extraPair].l);
          spillPair (extraPair);
          regalloc_dry_run_cost += 1;
          emit2 ("ld a, !*pair", _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;

          emit2 ("and a, !immedbyte", mask);
          regalloc_dry_run_cost += 2;
          emit2 ("or a, %s", _pairs[extraPair].l);
          regalloc_dry_run_cost += 1;
          emit2 ("ld !*pair, a", _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;
          if (needPopExtra)
            _pop (extraPair);
          return;
        }
    }

  /* Bit length is greater than 7 bits. In this case, copy  */
  /* all except the partial byte at the end                 */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      cheapMove (ASMOP_A, 0, AOP (right), offset++, true);
      if (pair == PAIR_IX || pair == PAIR_IY)
        {
          emit2 ("ld %d !*pair,a", pair_offset, _pairs[pair].name);
          regalloc_dry_run_cost += 3;
        }
      else
        {
          emit2 ("ld !*pair,a", _pairs[pair].name);
          regalloc_dry_run_cost += 1;
        }
      if (rlen > 8 && pair != PAIR_IX && pair != PAIR_IY)
        {
          emit2 ("inc %s", _pairs[pair].name);
          regalloc_dry_run_cost += 1;
          pairincrement++;
          _G.pairs[pair].offset++;
        }
      else
        pair_offset++;
    }

  /* If there was a partial byte at the end */
  if (rlen)
    {
      mask = (((unsigned char) - 1 << rlen) & 0xff);

      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with partial byte and literal source */
          litval = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          litval >>= (blen - rlen);
          litval &= (~mask) & 0xff;

          if (pair == PAIR_IX || pair == PAIR_IY)
            {
              emit2 ("ld a, %d !*pair", pair_offset, _pairs[pair].name);
              regalloc_dry_run_cost += 3;
            }
          else
            {
              emit2 ("ld a, !*pair", _pairs[pair].name);
              regalloc_dry_run_cost += 1;
            }

          if ((mask | litval) != 0xff)
            emit2 ("and a, !immedbyte", mask);
          if (litval)
            emit2 ("or a, !immedbyte", litval);
        }
      else
        {
          /* Case with partial byte and arbitrary source */
          cheapMove (ASMOP_A, 0, AOP (right), offset++, true);
          emit2 ("and a, !immedbyte", (~mask) & 0xff);
          regalloc_dry_run_cost += 2;

          extraPair = getFreePairId (ic);
          if (extraPair == PAIR_INVALID)
            {
              if (pair != PAIR_HL)
                extraPair = PAIR_HL;
              else
                {
                  extraPair = PAIR_BC;
                  if (getPairId (AOP (right)) != PAIR_BC || !isLastUse (ic, right))
                    {
                      _push (extraPair);
                      needPopExtra = 1;
                    }
                }
            }

          emit2 ("ld %s,a", _pairs[extraPair].l);
          spillPair (extraPair);
          regalloc_dry_run_cost += 1;

          if (pair == PAIR_IX || pair == PAIR_IY)
            {
              emit2 ("ld a, %d !*pair", pair_offset, _pairs[pair].name);
              regalloc_dry_run_cost += 3;
            }
          else
            {
              emit2 ("ld a, !*pair", _pairs[pair].name);
              regalloc_dry_run_cost += 1;
            }

          emit2 ("and a, !immedbyte", mask);
          regalloc_dry_run_cost += 2;
          emit2 ("or a, %s", _pairs[extraPair].l);
          regalloc_dry_run_cost += 1;
          if (needPopExtra)
            _pop (extraPair);

        }
      if (pair == PAIR_IX || pair == PAIR_IY)
        {
          emit2 ("ld %d !*pair, a", pair_offset, _pairs[pair].name);
          regalloc_dry_run_cost += 3;
        }
      else
        {
          emit2 ("ld !*pair, a", _pairs[pair].name);
          regalloc_dry_run_cost += 1;
        }
    }
  if (!isPairDead(pair, ic))
    while (pairincrement)
      {
        emit2 ("dec %s", _pairs[pair].name);
        regalloc_dry_run_cost += 1;
        pairincrement--;
        _G.pairs[pair].offset--;
      }
}

/*-----------------------------------------------------------------*/
/* genPointerSet - stores the value into a pointer location        */
/*-----------------------------------------------------------------*/
static void
genPointerSet (iCode *ic)
{
  int size, offset = 0;
  int last_offset = 0;
  operand *right, *result;
  PAIR_ID pairId = PAIR_HL;
  bool isBitvar;
  sym_link *retype;
  sym_link *letype;
  bool pushed_a = FALSE;
  bool surviving_a = !options.oldralloc && bitVectBitValue (ic->rSurv, A_IDX);

  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  retype = getSpec (operandType (right));
  letype = getSpec (operandType (result));

  aopOp (result, ic, FALSE, FALSE);
  aopOp (right, ic, FALSE, FALSE);

  if (IS_GB)
    pairId = isRegOrLit (AOP (right)) ? PAIR_HL : PAIR_DE;
  else if (IY_RESERVED)
    pairId = (isRegOrLit (AOP (right)) || AOP_TYPE (right) == AOP_STK) ? PAIR_HL : PAIR_DE;
  if (isPair (AOP (result)) && isPairDead (getPairId (AOP (result)), ic))
    pairId = getPairId (AOP (result));

  size = AOP_SIZE (right);

  isBitvar = IS_BITVAR (retype) || IS_BITVAR (letype);
  emitDebug ("; isBitvar = %d", isBitvar);

  /* Handle the exceptions first */
  if (isPair (AOP (result)) && size == 1 && !isBitvar)
    {
      /* Just do it */
      const char *pair = getPairName (AOP (result));
      if (canAssignToPtr3 (AOP (right)) && isPtr (pair))        // Todo: correct cost for pair iy.
        {
          if (!regalloc_dry_run)
            emit2 ("ld !*pair, %s", pair, aopGet (AOP (right), 0, FALSE));
          regalloc_dry_run_cost += ld_cost (ASMOP_A, AOP (right)) + (getPairId (AOP (result)) != PAIR_IY ? 0 : 2);
        }
      else
        {
          if (surviving_a && !pushed_a && !aopInReg (right->aop, 0, A_IDX))
            _push (PAIR_AF), pushed_a = TRUE;
          if (AOP_TYPE (right) == AOP_LIT && byteOfVal (AOP (right)->aopu.aop_lit, offset) == 0x00)
            emit3 (A_XOR, ASMOP_A, ASMOP_A);
          else
            cheapMove (ASMOP_A, 0, AOP (right), 0, true);
          emit2 ("ld !*pair, a", pair);
          regalloc_dry_run_cost += (getPairId (AOP (result)) != PAIR_IY ? 1 : 3);
        }
      goto release;
    }

  /* Using ldir is cheapest for large memory-to-memory transfers. */
  if (!IS_GB && (AOP_TYPE (right) == AOP_STK || AOP_TYPE (right) == AOP_EXSTK) && size > 2)
    {
      int fp_offset, sp_offset;

      if(!isPairDead (PAIR_DE, ic))
        _push (PAIR_DE);
      if(!isPairDead (PAIR_BC, ic))
        _push (PAIR_BC);
      if(!isPairDead (PAIR_HL, ic))
        _push (PAIR_HL);

      fetchPair (PAIR_DE, AOP (result));

      fp_offset = AOP (right)->aopu.aop_stk + (AOP (right)->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
      emit2 ("ld hl, !immedword", sp_offset);
      emit2 ("add hl, sp");
      emit2 ("ld bc, !immedword", size);
      emit2 ("ldir");
      regalloc_dry_run_cost += 9;

      if(!isPairDead (PAIR_HL, ic))
        _pop (PAIR_HL);
      if(!isPairDead (PAIR_BC, ic))
        _pop (PAIR_BC);
      if(!isPairDead (PAIR_DE, ic))
        _pop (PAIR_DE);
      goto release;
    }

  if (getPairId (AOP (result)) == PAIR_IY && !isBitvar)
    {
      /* Just do it */
      while (size--)
        {
          if (canAssignToPtr3 (AOP (right)))
            {
              if (!regalloc_dry_run)
                emit2 ("ld !*iyx, %s", offset, aopGet (AOP (right), offset, FALSE));
              regalloc_dry_run_cost += 3;       // Todo: More exact cost here!
            }
          else
            {
              cheapMove (ASMOP_A, 0, AOP (right), offset, true);
              emit2 ("ld !*iyx, a", offset);
              regalloc_dry_run_cost += 3;
            }
          offset++;
        }
      goto release;
    }
  else if (getPairId (result->aop) == PAIR_HL && !isPairDead (PAIR_HL, ic) && !isBitvar)
    {
      while (offset < size)
        {
          last_offset = offset;

          if (IS_EZ80_Z80 && offset + 1 < size && getPairId_o (right->aop, offset) != PAIR_INVALID)
            {
              emit2 ("ld !*pair, %s", _pairs[PAIR_HL].name, _pairs[getPairId_o (right->aop, offset)].name);
              regalloc_dry_run_cost += 2;
              offset += 2;

              if (offset < size)
                {
                  emit2 ("inc %s", _pairs[PAIR_HL].name);
                  emit2 ("inc %s", _pairs[PAIR_HL].name);
                  regalloc_dry_run_cost += 2;
                  _G.pairs[PAIR_HL].offset++;
                }

              continue;
            }
          else if (isRegOrLit (AOP (right)) && !IS_GB)
            {
              if (!regalloc_dry_run)
                emit2 ("ld !*pair, %s", _pairs[PAIR_HL].name, aopGet (right->aop, offset, FALSE));
              regalloc_dry_run_cost += ld_cost (ASMOP_A, right->aop);
              offset++;
            }
          else
            {
              if (surviving_a && !pushed_a && (!aopInReg (right->aop, 0, A_IDX) || offset))
                _push (PAIR_AF), pushed_a = TRUE;
              cheapMove (ASMOP_A, 0, right->aop, offset, true);
              emit2 ("ld !*pair, a", _pairs[PAIR_HL].name);
              regalloc_dry_run_cost += 1;
              offset++;
            }

          if (offset < size)
            {
              emit2 ("inc %s", _pairs[PAIR_HL].name);
              regalloc_dry_run_cost += 1;
              _G.pairs[PAIR_HL].offset++;
            }
        }

      /* Fixup HL back down */
      while (last_offset --> 0)
        {
          emit2 ("dec %s", _pairs[PAIR_HL].name);
          regalloc_dry_run_cost += 1;
        }
      goto release;
    }

  if (!IS_GB && !isBitvar && isLitWord (AOP (result)) && size == 2 && offset == 0 &&
      (AOP_TYPE (right) == AOP_REG && getPairId (AOP (right)) != PAIR_INVALID || isLitWord (AOP (right))))
    {
      if (isLitWord (AOP (right)))
        {
          pairId = PAIR_HL;
          fetchPairLong (pairId, AOP (right), ic, 0);
        }
      else
        pairId = getPairId (AOP (right));
      emit2 ("ld (%s), %s", aopGetLitWordLong (AOP (result), offset, FALSE), _pairs[pairId].name);
      regalloc_dry_run_cost += (pairId == PAIR_HL) ? 3 : 4;
      goto release;
    }
  if (!IS_GB && !isBitvar && isLitWord (AOP (result)) && size == 4 && offset == 0 &&
    (getPartPairId (AOP (right), 0) != PAIR_INVALID && getPartPairId (AOP (right), 2) != PAIR_INVALID || isLitWord (AOP (right))))
    {
      if (isLitWord (AOP (right)))
        {
          pairId = PAIR_HL;
          fetchPairLong (pairId, AOP (right), ic, 0);
        }
      else
        pairId = getPartPairId (AOP (right), 0);
      emit2 ("ld (%s), %s", aopGetLitWordLong (AOP (result), offset, FALSE), _pairs[pairId].name);
      regalloc_dry_run_cost += (pairId == PAIR_HL) ? 3 : 4;
      if (isLitWord (AOP (right)))
        {
          pairId = PAIR_HL;
          fetchPairLong (pairId, AOP (right), ic, 2);
        }
      else
        pairId = getPartPairId (AOP (right), 2);
      emit2 ("ld (%s+%d), %s", aopGetLitWordLong (AOP (result), offset, FALSE),2,  _pairs[pairId].name); // Handling of literal addresses is somewhat broken, use explicit offset as workaround.
      regalloc_dry_run_cost += (pairId == PAIR_HL) ? 3 : 4;
      goto release;
    }

  /* if the operand is already in dptr
     then we do nothing else we move the value to dptr */
  if (AOP_TYPE (result) != AOP_STR)
    {
      if (isBitvar && getPairId (AOP (result)) != PAIR_INVALID && (getPairId (AOP (result)) != PAIR_IY || SPEC_BLEN (IS_BITVAR (retype) ? retype : letype) < 8 || isPairDead (getPairId (AOP (result)), ic)))   /* Avoid destroying result by increments */
        pairId = getPairId (AOP (result));
      else
        fetchPairLong (pairId, AOP (result), ic, 0);
    }
  /* so hl now contains the address */
  /*freeAsmop (result, NULL, ic);*/

  /* if bit then unpack */
  if (isBitvar)
    {
      genPackBits ((IS_BITVAR (retype) ? retype : letype), right, pairId, ic);
      goto release;
    }
  else
    {
      bool zero_a = false;

      for (offset = 0; offset < size;)
        {
          last_offset = offset;

          if (IS_EZ80_Z80 && offset + 1 < size && pairId == PAIR_HL && getPairId_o (right->aop, offset) != PAIR_INVALID)
            {
              emit2 ("ld !*pair, %s", _pairs[pairId].name, _pairs[getPairId_o (right->aop, offset)].name);
              regalloc_dry_run_cost += 2;
              offset += 2;

              if (offset < size)
                {
                  emit2 ("inc %s", _pairs[pairId].name);
                  emit2 ("inc %s", _pairs[pairId].name);
                  regalloc_dry_run_cost += 2;
                  _G.pairs[pairId].offset++;
                }

              continue;
            }

          if (!zero_a && offset + 1 < size && aopIsLitVal (right->aop, offset, 2, 0x0000) && !surviving_a)
            {
              emit2 ("xor a, a");
              regalloc_dry_run_cost++;
              zero_a = true;
            }
 
          if (aopIsLitVal (right->aop, offset, 1, 0x00) && zero_a)
            {
              emit2 ("ld !*pair, a", _pairs[pairId].name);
              regalloc_dry_run_cost++;
            }
          else if (isRegOrLit (right->aop) && pairId == PAIR_HL)
            {
              if (!regalloc_dry_run)
                emit2 ("ld !*pair, %s", _pairs[pairId].name, aopGet (AOP (right), offset, FALSE));
              regalloc_dry_run_cost += ld_cost (ASMOP_A, AOP (right));
            }
          else
            {
              if (surviving_a && !pushed_a && (!aopInReg (right->aop, 0, A_IDX) || offset))
                _push (PAIR_AF), pushed_a = TRUE;
              cheapMove (ASMOP_A, 0, AOP (right), offset, true);
              zero_a = false;
              emit2 ("ld !*pair, a", _pairs[pairId].name);
              regalloc_dry_run_cost += 1;
            }
          offset++;

          if (offset < size)
            {
              emit2 ("inc %s", _pairs[pairId].name);
              regalloc_dry_run_cost += 1;
              _G.pairs[pairId].offset++;
            }
        }
      /* Restore operand partially in HL. */
      if (!isPairDead (pairId, ic) && AOP (result)->type == AOP_REG)
        {
          while(last_offset --> 0)
            {
              emit2 ("dec %s", _pairs[pairId].name);
              regalloc_dry_run_cost += 1;
              _G.pairs[pairId].offset--;
            }
          commitPair (AOP (result), pairId, ic, FALSE);
        }
    }
release:
  if (pushed_a)
    _pop (PAIR_AF);

  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genIfx - generate code for Ifx statement                        */
/*-----------------------------------------------------------------*/
static void
genIfx (iCode *ic, iCode *popIc)
{
  operand *cond = IC_COND (ic);
  int isbit = 0;

  aopOp (cond, ic, FALSE, TRUE);

  /* get the value into acc */
  if (AOP_TYPE (cond) != AOP_CRY && !IS_BOOL (operandType (cond)))
    _toBoolean (cond, !popIc);
  /* Special case: Condition is bool */
  else if (IS_BOOL (operandType (cond)))
    {
      if (!regalloc_dry_run)
        {
          emit2 ("bit 0, %s", aopGet (AOP (cond), 0, FALSE));
          emit2 ("jp %s, !tlabel", IC_TRUE (ic) ? "NZ" : "Z", labelKey2num ((IC_TRUE (ic) ? IC_TRUE (ic) : IC_FALSE (ic))->key));
        }
      regalloc_dry_run_cost += (bit8_cost (AOP (cond)) + 3);

      freeAsmop (cond, NULL);
      if (!regalloc_dry_run)
        ic->generated = 1;
      return;
    }
  else
    isbit = 1;
  /* the result is now in the accumulator */
  freeAsmop (cond, NULL);

  /* if there was something to be popped then do it */
  if (popIc)
    genIpop (popIc);

  /* if the condition is  a bit variable */
  if (isbit && IS_ITEMP (cond) && SPIL_LOC (cond))
    genIfxJump (ic, SPIL_LOC (cond)->rname);
  else if (isbit && !IS_ITEMP (cond))
    genIfxJump (ic, OP_SYMBOL (cond)->rname);
  else
    genIfxJump (ic, popIc ? "a" : "nz");

  if (!regalloc_dry_run)
    ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void
genAddrOf (const iCode * ic)
{
  symbol *sym;
  PAIR_ID pair;
  operand *right = IC_RIGHT (ic);
  wassert (IS_TRUE_SYMOP (IC_LEFT (ic)));
  wassert (right && IS_OP_LITERAL (IC_RIGHT (ic)));
  sym = OP_SYMBOL (IC_LEFT (ic));
  aopOp (IC_RESULT (ic), ic, FALSE, FALSE);

  if (sym->onStack)
    {
      int fp_offset = sym->stack + (sym->stack > 0 ? _G.stack.param_offset : 0) + (int)operandLitValue (right);
      int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
      bool in_fp_range = !_G.omitFramePtr && (fp_offset >= -128 && fp_offset < 128);

      if (IS_EZ80_Z80 && in_fp_range && getPairId (IC_RESULT (ic)->aop) != PAIR_INVALID)
        pair = getPairId (IC_RESULT (ic)->aop);
      else
        pair = (getPairId (IC_RESULT (ic)->aop) == PAIR_IY) ? PAIR_IY : PAIR_HL;
      spillPair (pair);
      if (IS_EZ80_Z80 && in_fp_range)
        {
          emit2 ("lea %s, ix, !immed%d", _pairs[pair].name, fp_offset);
          regalloc_dry_run_cost += 3;
        }
      else
        setupPairFromSP (pair, sp_offset);
    }
  else 
    {
      pair = getPairId (IC_RESULT (ic)->aop);
      if (pair == PAIR_INVALID)
        {
          pair = IS_GB ? PAIR_DE : PAIR_HL;
          spillPair (pair);
        }
      emit2 ("ld %s, !hashedstr+%ld", _pairs[pair].name, sym->rname, (long)(operandLitValue (right)));
      regalloc_dry_run_cost += (pair == PAIR_IY ? 4 : 3);
    }

  commitPair (IC_RESULT (ic)->aop, pair, ic, FALSE);

  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void
genAssign (const iCode *ic)
{
  operand *result, *right;
  int size, offset;

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);

  /* Dont bother assigning if they are the same */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  aopOp (right, ic, FALSE, FALSE);
  aopOp (result, ic, TRUE, FALSE);

  /* if they are the same registers */
  if (sameRegs (AOP (right), AOP (result)))
    {
      emitDebug ("; (locations are the same)");
      goto release;
    }

  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      wassertl (0, "Tried to assign to a bit");
    }

  /* general case */
  size = AOP_SIZE (result);
  offset = 0;

  if (isPair (AOP (result)))
    fetchPairLong (getPairId (AOP (result)), AOP (right), ic, LSB);
  else if (isPair (AOP (right)) && AOP_TYPE (result) == AOP_IY && size == 2)
    commitPair (AOP (result), getPairId (AOP (right)), ic, FALSE);
  else if (size == 2 && isPairDead (PAIR_HL, ic) &&
    (!IS_GB && (AOP_TYPE (right) == AOP_STK && !_G.omitFramePtr || AOP_TYPE (right) == AOP_IY || AOP_TYPE (right) == AOP_LIT) && AOP_TYPE (result) == AOP_IY || // Use ld (nn), hl
    !IS_GB && AOP_TYPE (right) == AOP_IY && (AOP_TYPE (result) == AOP_STK && !_G.omitFramePtr || AOP_TYPE (result) == AOP_IY) || // Use ld hl, (nn)
    !IS_GB && AOP_TYPE (right) == AOP_LIT && (AOP_TYPE(result) == AOP_STK || AOP_TYPE(result) == AOP_EXSTK) && (AOP(result)->aopu.aop_stk + offset + _G.stack.offset + (AOP(result)->aopu.aop_stk > 0 ? _G.stack.param_offset : 0) + _G.stack.pushed) == 0 || // Use ex (sp), hl
    (IS_RAB || IS_TLCS90) && (AOP_TYPE(result) == AOP_STK || AOP_TYPE(result) == AOP_EXSTK) && (AOP_TYPE(right) == AOP_LIT || AOP_TYPE (right) == AOP_IMMD))) // Use ld d(sp), hl
    {
      fetchPair (PAIR_HL, AOP (right));
      commitPair (AOP (result), PAIR_HL, ic, FALSE);
    }
  else if (size == 2 && getPairId (AOP (right)) != PAIR_INVALID && getPairId (AOP (right)) != PAIR_IY && AOP_TYPE (result) != AOP_REG)
    {
      commitPair (AOP (result), getPairId (AOP (right)), ic, TRUE);
    }
  else if (getPairId (AOP (right)) == PAIR_IY)
    {
      while (size--)
        {
          if (size == 0)
            {
              if (IS_TLCS90)
                {
                  emit2 ("push iy");
                  emit2 ("ld a, 0(sp)");
                  emit2 ("inc sp");
                  emit2 ("inc sp");
                  cost (5, 26);
                }
              else
                {
                  emit2 ("push iy");
                  emit2 ("dec sp");
                  emit2 ("pop af");
                  emit2 ("inc sp");
                  regalloc_dry_run_cost += 5;
                }
              if (AOP_TYPE (result) == AOP_IY) /* Take care not to overwrite iy */
                {
                  emit2 ("ld (%s+%d), a", AOP (result)->aopu.aop_dir, size);
                  regalloc_dry_run_cost += 3;
                }
              else
                cheapMove (AOP (result), size, ASMOP_A, 0, true);
            }
          else if (size == 1)
            {
              if (AOP_TYPE (result) == AOP_IY) /* Take care not to overwrite iy */
                {
                  emit2 ("ld (%s), iy", AOP (result)->aopu.aop_dir);
                  regalloc_dry_run_cost += 4;
                  size--;
                }
              else if (AOP_TYPE (result) == AOP_EXSTK || IS_TLCS90) /* Take care not to overwrite iy */
                {
                  bool pushed_pair = FALSE;
                  PAIR_ID pair = getDeadPairId (ic);
                  if (pair == PAIR_INVALID)
                  {
                    pair = PAIR_HL;
                    _push(pair);
                    pushed_pair= TRUE;
                  }
                  fetchPair (pair, AOP (right));
                  commitPair (AOP (result), pair, ic, FALSE);
                  if (pushed_pair)
                    _pop (pair);
                  size--;
                }
              else
                {
                  emit2 ("push iy");
                  emit2 ("pop af");
                  regalloc_dry_run_cost += 3;
                  cheapMove (AOP (result), size, ASMOP_A, 0, true);
                }
            }
          else
            {
              if (AOP_TYPE (result) == AOP_IY) /* Take care not to overwrite iy */
                {
                  cheapMove (ASMOP_A, 0, ASMOP_ZERO, 0, true);
                  emit2 ("ld (%s+%d), a", AOP (result)->aopu.aop_dir, size);
                  regalloc_dry_run_cost += 3;
                }
              else
                cheapMove (AOP (result), size, ASMOP_ZERO, 0, true);
            }
        }
    }
  else if (size == 2 && requiresHL (AOP (right)) && requiresHL (AOP (result)) && isPairDead (PAIR_DE, ic) && (IS_GB /*|| IY_RESERVED */ ))
    {
      /* Special case.  Load into a and d, then load out. */
      cheapMove (ASMOP_A, 0, AOP (right), 0, true);
      emit3_o (A_LD, ASMOP_E, 0, AOP (right), 1);
      cheapMove (AOP (result), 0, ASMOP_A, 0, true);
      cheapMove (AOP (result), 1, ASMOP_E, 0, true);
    }
  else if (size == 4 && requiresHL (AOP (right)) && requiresHL (AOP (result)) && isPairDead (PAIR_DE, ic) && (IS_GB /*|| IY_RESERVED */ ))
    {
      /* Special case - simple memcpy */
      if (!regalloc_dry_run)
        {
          aopGet (AOP (right), LSB, FALSE);
          emit2 ("ld d, h");
          emit2 ("ld e, l");
          aopGet (AOP (result), LSB, FALSE);
        }
      regalloc_dry_run_cost += 8;       // Todo: More exact cost here!

      while (size--)
        {
          emit2 ("ld a, (de)");
          /* Peephole will optimise this. */
          emit2 ("ld (hl), a");
          regalloc_dry_run_cost += 2;
          if (size != 0)
            {
              emit2 ("inc hl");
              emit2 ("inc de");
              regalloc_dry_run_cost += 2;
            }
        }
      spillPair (PAIR_HL);
    }
  else
    {
      if (!IS_GB &&             /* gbz80 doesn't have ldir */
          (AOP_TYPE (result) == AOP_STK || AOP_TYPE (result) == AOP_EXSTK || AOP_TYPE (result) == AOP_DIR
           || AOP_TYPE (result) == AOP_IY) && (AOP_TYPE (right) == AOP_STK || AOP_TYPE (right) == AOP_EXSTK
               || AOP_TYPE (right) == AOP_DIR || AOP_TYPE (right) == AOP_IY) && size >= 2)
        {
          /* This estimation is only accurate, if neither operand is AOP_EXSTK, and we are optimizing for code size or targeting the z80 or z180. */
          int sizecost_n, sizecost_l, cyclecost_n, cyclecost_l;
          const bool hl_alive = !isPairDead (PAIR_HL, ic);
          const bool de_alive = !isPairDead (PAIR_DE, ic);
          const bool bc_alive = !isPairDead (PAIR_BC, ic);
          bool l_better;
          sizecost_n = 6 * size;
          sizecost_l = 13 + hl_alive * 2 + de_alive * 2 + bc_alive * 2 - (AOP_TYPE (right) == AOP_DIR
                       || AOP_TYPE (right) == AOP_IY) - (AOP_TYPE (result) ==
                           AOP_DIR
                           || AOP_TYPE (result)
                           == AOP_IY) * 2;
          if (IS_Z180 || IS_EZ80_Z80)
            cyclecost_n = 30 * size;
          else                  /* Z80 */
            cyclecost_n = 38 * size;
          if (IS_Z180 || IS_EZ80_Z80)
            cyclecost_l = 14 * size + 42 + hl_alive * 22 + de_alive * 22 + bc_alive * 22 - (AOP_TYPE (right) == AOP_DIR
                          || AOP_TYPE (right) ==
                          AOP_IY) * 7 - (AOP_TYPE (result) ==
                                         AOP_DIR
                                         || AOP_TYPE (result)
                                         == AOP_IY) * 10;
          else                  /* Z80 */
            cyclecost_l = 21 * size + 51 + hl_alive * 20 + de_alive * 20 + bc_alive * 20 - (AOP_TYPE (right) == AOP_DIR
                          || AOP_TYPE (right) ==
                          AOP_IY) * 11 - (AOP_TYPE (result) ==
                                          AOP_DIR
                                          || AOP_TYPE (result)
                                          == AOP_IY) * 15;

          if (optimize.codeSize)
            l_better = (sizecost_l < sizecost_n || sizecost_l == sizecost_n && cyclecost_l < cyclecost_n);
          else
            l_better = (cyclecost_l < cyclecost_n || cyclecost_l == cyclecost_n && sizecost_l < sizecost_n);
          if (l_better)
            {
              if (hl_alive)
                _push (PAIR_HL);
              if (de_alive)
                _push (PAIR_DE);
              if (bc_alive)
                _push (PAIR_BC);

              if (AOP_TYPE (result) == AOP_STK || AOP_TYPE (result) == AOP_EXSTK)
                {
                  int fp_offset =
                    AOP (result)->aopu.aop_stk + offset + (AOP (result)->aopu.aop_stk >
                        0 ? _G.stack.param_offset : 0);
                  int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
                  emit2 ("ld hl, !immed%d", sp_offset);
                  emit2 ("add hl, sp");
                  emit2 ("ex de, hl");
                  regalloc_dry_run_cost += 5;
                }
              else
                {
                  emit2 ("ld de, !hashedstr", AOP (IC_RESULT (ic))->aopu.aop_dir);
                  regalloc_dry_run_cost += 3;
                }

              if (AOP_TYPE (right) == AOP_STK || AOP_TYPE (right) == AOP_EXSTK)
                {
                  int fp_offset =
                    AOP (right)->aopu.aop_stk + offset + (AOP (right)->aopu.aop_stk >
                        0 ? _G.stack.param_offset : 0);
                  int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
                  emit2 ("ld hl, !immed%d", sp_offset);
                  emit2 ("add hl, sp");
                  regalloc_dry_run_cost += 4;
                }
              else
                {
                  emit2 ("ld hl, !hashedstr", AOP (IC_RIGHT (ic))->aopu.aop_dir);
                  regalloc_dry_run_cost += 3;
                }
              spillPair (PAIR_HL);

              if (size <= 2 + optimize.codeSpeed)
                for(int i = 0; i < size; i++)
                  {
                    emit2 ("ldi");
                    regalloc_dry_run_cost += 2;
                  }
              else
                {
                  emit2 ("ld bc, !immed%d", size);
                  emit2 ("ldir");
                  regalloc_dry_run_cost += 5;
                }

              if (bc_alive)
                _pop (PAIR_BC);
              if (de_alive)
                _pop (PAIR_DE);
              if (hl_alive)
                _pop (PAIR_HL);

              goto release;
            }
        }
      if ((result->aop->type == AOP_REG || result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK) && (right->aop->type == AOP_REG || right->aop->type == AOP_STK || right->aop->type == AOP_LIT))
        genMove (result->aop, right->aop, !bitVectBitValue (ic->rSurv, A_IDX), isPairDead (PAIR_HL, ic));
      else
        while (size--)
          {
            /* PENDING: do this check better */
            if ((IS_GB || IY_RESERVED) && requiresHL (AOP (right)) && requiresHL (AOP (result)))
              {
                _push (PAIR_HL);
                cheapMove (ASMOP_A, 0, AOP (right), offset, true);
                cheapMove (AOP (result), offset, ASMOP_A, 0, true);
                _pop (PAIR_HL);
                spillPair (PAIR_HL);
              }
            else
              cheapMove (AOP (result), offset, AOP (right), offset, true);
            offset++;
          }
    }

release:
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genJumpTab - generate code for jump table                       */
/*-----------------------------------------------------------------*/
static void
genJumpTab (const iCode *ic)
{
  symbol *jtab = NULL;
  operand *jtcond = IC_JTCOND (ic);
  bool pushed_pair = FALSE;
  PAIR_ID pair;

  aopOp (jtcond, ic, FALSE, FALSE);

  // Choose extra pair DE or BC for addition
  if (AOP_TYPE (jtcond) == AOP_REG && AOP (jtcond)->aopu.aop_reg[0]->rIdx == E_IDX && isPairDead (PAIR_DE, ic))
    pair = PAIR_DE;
  else if (AOP_TYPE (jtcond) == AOP_REG && AOP (jtcond)->aopu.aop_reg[0]->rIdx == C_IDX && isPairDead (PAIR_BC, ic))
    pair = PAIR_BC;
  else if ((pair = getDeadPairId (ic)) == PAIR_INVALID)
    pair = PAIR_DE;

  if (!isPairDead (pair, ic))
    {
      _push (pair);
      pushed_pair = TRUE;
    }

  cheapMove (pair == PAIR_DE ? ASMOP_E : ASMOP_C, 0, AOP (jtcond), 0, true);
  if (!regalloc_dry_run)
    {
      emit2 ("ld %s, !zero", _pairs[pair].h);
      jtab = newiTempLabel (NULL);
    }
  regalloc_dry_run_cost += 2;
  spillPair (PAIR_HL);
  if (!regalloc_dry_run)
    {
      emit2 ("ld hl, !immed!tlabel", labelKey2num (jtab->key));
      emit2 ("add hl, %s", _pairs[pair].name);
      emit2 ("add hl, %s", _pairs[pair].name);
      emit2 ("add hl, %s", _pairs[pair].name);
    }
  regalloc_dry_run_cost += 5;
  freeAsmop (IC_JTCOND (ic), NULL);

  if (pushed_pair)
    _pop (pair);

  if (!regalloc_dry_run)
    {
      emit2 ("jp !*hl");
      emitLabelSpill (jtab);
    }
  regalloc_dry_run_cost += 1;
  /* now generate the jump labels */
  for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
    if (!regalloc_dry_run)
      emit2 ("jp !tlabel", labelKey2num (jtab->key));
  /*regalloc_dry_run_cost += 3 doesn't matter and might overflow cost */
}

/*-----------------------------------------------------------------*/
/* genCast - gen code for casting                                  */
/*-----------------------------------------------------------------*/
static void
genCast (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  sym_link *rtype = operandType (IC_RIGHT (ic));
  operand *right = IC_RIGHT (ic);
  int size, offset;
  bool surviving_a = !options.oldralloc && bitVectBitValue (ic->rSurv, A_IDX);
  bool pushed_a = FALSE;

  /* if they are equivalent then do nothing */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  aopOp (right, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, FALSE);

  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      wassertl (0, "Tried to cast to a bit");
    }

  /* casting to bool */
  if (IS_BOOL (operandType (result)) && IS_RAB && right->aop->size == 2 && 
    (aopInReg (right->aop, 0, HL_IDX) && isPairDead (PAIR_HL, ic)|| aopInReg (right->aop, 0, IY_IDX) && isPairDead (PAIR_IY, ic)))
    {
      bool iy = aopInReg (right->aop, 0, IY_IDX);
      emit2 ("bool %s", _pairs[getPairId (right->aop)].name);
      cheapMove (result->aop, 0, iy ? ASMOP_IYL : ASMOP_L, 0, !bitVectBitValue (ic->rSurv, A_IDX));
      goto release;
    }
  if (IS_BOOL (operandType (result)))
    {
      _castBoolean (right);
      outAcc (result);
      goto release;
    }

  /* if they are the same size or less */
  if (AOP_SIZE (result) <= AOP_SIZE (right))
    {
      genAssign (ic);
      goto release;
    }

  /* So we now know that the size of destination is greater
     than the size of the source */
  genMove_o (result->aop, 0, right->aop, 0, right->aop->size - 1, true, isPairDead (PAIR_HL, ic));

  /* now depending on the sign of the destination */
  size = result->aop->size - right->aop->size;
  offset = right->aop->size - 1;
  /* Unsigned or not an integral type - fill with zeros */
  if (IS_BOOL (rtype) || !IS_SPEC (rtype) || SPEC_USIGN (rtype) || AOP_TYPE (right) == AOP_CRY)
    {
      cheapMove (result->aop, offset, right->aop, offset, true);
      offset++;
      genMove_o (result->aop, offset, ASMOP_ZERO, 0, size, true, false);
    }
  else
    {
      if (surviving_a && !pushed_a)
        _push (PAIR_AF), pushed_a = TRUE;

      cheapMove (ASMOP_A, 0, AOP (right), offset, true);
      if (AOP (right)->type != AOP_REG || AOP (result)->type != AOP_REG || AOP (right)->aopu.aop_reg[offset] != AOP (result)->aopu.aop_reg[offset])
        cheapMove (AOP (result), offset, ASMOP_A, 0, true);
      offset++;

      /* we need to extend the sign */
      emit3 (A_RLA, 0, 0);
      emit3 (A_SBC, ASMOP_A, ASMOP_A);
      while (size--)
        cheapMove (AOP (result), offset++, ASMOP_A, 0, true);
    }

release:
  if (pushed_a)
    _pop (PAIR_AF);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genReceive - generate code for a receive iCode                  */
/*-----------------------------------------------------------------*/
static void
genReceive (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  aopOp (result, ic, FALSE, FALSE);
  
  genMove (result->aop, ASMOP_RETURN, true, isPairDead (PAIR_HL, ic));

  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genDummyRead - generate code for dummy read of volatiles        */
/*-----------------------------------------------------------------*/
static void
genDummyRead (const iCode * ic)
{
  operand *op;
  int size, offset;

  op = IC_RIGHT (ic);
  if (op && IS_SYMOP (op))
    {
      aopOp (op, ic, FALSE, FALSE);

      /* general case */
      size = AOP_SIZE (op);
      offset = 0;

      while (size--)
        {
          _moveA3 (AOP (op), offset);
          offset++;
        }

      freeAsmop (op, NULL);
    }

  op = IC_LEFT (ic);
  if (op && IS_SYMOP (op))
    {
      aopOp (op, ic, FALSE, FALSE);

      /* general case */
      size = AOP_SIZE (op);
      offset = 0;

      while (size--)
        {
          _moveA3 (AOP (op), offset);
          offset++;
        }

      freeAsmop (op, NULL);
    }
}

/*-----------------------------------------------------------------*/
/* genCritical - generate code for start of a critical sequence    */
/*-----------------------------------------------------------------*/
static void
genCritical (const iCode * ic)
{
  symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);

  if (IS_GB || IS_RAB || IS_TLCS90)
    {
      emit2 ("!di");
      regalloc_dry_run_cost += 1;
    }
  else if (IC_RESULT (ic))
    {
      aopOp (IC_RESULT (ic), ic, true, false);
      cheapMove (IC_RESULT (ic)->aop, 0, ASMOP_ZERO, 0, true);
      if (!regalloc_dry_run)
        {
          //get interrupt enable flag IFF2 into P/O
          emit2 ("ld a,i");

          //disable interrupt
          emit2 ("!di");
          //parity odd <==> P/O=0 <==> interrupt enable flag IFF2=0
          emit2 ("jp PO, !tlabel", labelKey2num (tlbl->key));
        }
      regalloc_dry_run_cost += 5;
      cheapMove (IC_RESULT (ic)->aop, 0, ASMOP_ONE, 0, true);
      if (!regalloc_dry_run)
        {
          emit2 ("!tlabeldef", labelKey2num ((tlbl->key)));
          genLine.lineCurr->isLabel = 1;
        }
      freeAsmop (IC_RESULT (ic), NULL);
    }
  else
    {
      //get interrupt enable flag IFF2 into P/O
      emit2 ("ld a,i");

      //disable interrupt
      emit2 ("!di");
      regalloc_dry_run_cost += 2;
      //save P/O flag
      if (!regalloc_dry_run)    // _push unbalances _G.stack.pushed.
        _push (PAIR_AF);
      else
        regalloc_dry_run_cost++;
    }
}

/*-----------------------------------------------------------------*/
/* genEndCritical - generate code for end of a critical sequence   */
/*-----------------------------------------------------------------*/
static void
genEndCritical (const iCode * ic)
{
  symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);

  if (IS_GB || IS_TLCS90)
    {
      emit2 ("!ei");
      regalloc_dry_run_cost += 1;
    }
  else if (IS_RAB)
    {
      emit2 ("ipres");
      regalloc_dry_run_cost += 1;
    }
  else if (IC_RIGHT (ic))
    {
      aopOp (IC_RIGHT (ic), ic, FALSE, TRUE);
      _toBoolean (IC_RIGHT (ic), TRUE);

      if (!regalloc_dry_run)
        {
          //don't enable interrupts if they were off before
          emit2 ("jp Z, !tlabel", labelKey2num (tlbl->key));
          emit2 ("!ei");
          emitLabelSpill (tlbl);
        }
      regalloc_dry_run_cost += 4;
      freeAsmop (IC_RIGHT (ic), NULL);
    }
  else
    {
      //restore P/O flag
      if (!regalloc_dry_run)    // _pop unbalances _G.stack.pushed.
        _pop (PAIR_AF);
      else
        regalloc_dry_run_cost++;
      //parity odd <==> P/O=0 <==> interrupt enable flag IFF2 was 0 <==>
      //don't enable interrupts as they were off before
      if (!regalloc_dry_run)
        {
          emit2 ("jp PO, !tlabel", labelKey2num (tlbl->key));
          emit2 ("!ei");
          emit2 ("!tlabeldef", labelKey2num ((tlbl->key)));
          genLine.lineCurr->isLabel = 1;
        }
      regalloc_dry_run_cost += 4;
    }
}

#if 0                           //Disabled since it doesn't work for arrays of float.
enum
{
  /** Maximum number of bytes to emit per line. */
  DBEMIT_MAX_RUN = 8
};

/** Context for the byte output chunker. */
typedef struct
{
  unsigned char buffer[DBEMIT_MAX_RUN];
  int pos;
} DBEMITCTX;


/** Flushes a byte chunker by writing out all in the buffer and
    reseting.
*/
static void
_dbFlush (DBEMITCTX * self)
{
  char line[256];

  if (self->pos > 0)
    {
      int i;
      sprintf (line, ".db 0x%02X", self->buffer[0]);

      for (i = 1; i < self->pos; i++)
        {
          sprintf (line + strlen (line), ", 0x%02X", self->buffer[i]);
        }
      emit2 (line);
    }
  self->pos = 0;
}

/** Write out another byte, buffering until a decent line is
    generated.
*/
static void
_dbEmit (DBEMITCTX * self, int c)
{
  if (self->pos == DBEMIT_MAX_RUN)
    {
      _dbFlush (self);
    }
  self->buffer[self->pos++] = c;
}

/** Context for a simple run length encoder. */
typedef struct
{
  unsigned last;
  unsigned char buffer[128];
  int pos;
  /** runLen may be equivalent to pos. */
  int runLen;
} RLECTX;

enum
{
  RLE_CHANGE_COST = 4,
  RLE_MAX_BLOCK = 127
};

/** Flush the buffer of a run length encoder by writing out the run or
    data that it currently contains.
*/
static void
_rleCommit (RLECTX * self)
{
  int i;
  if (self->pos != 0)
    {
      DBEMITCTX db;
      memset (&db, 0, sizeof (db));

      emit2 (".db %u", self->pos);

      for (i = 0; i < self->pos; i++)
        {
          _dbEmit (&db, self->buffer[i]);
        }
      _dbFlush (&db);
    }
  /* Reset */
  self->pos = 0;
}

/* Encoder design:
   Can get either a run or a block of random stuff.
   Only want to change state if a good run comes in or a run ends.
   Detecting run end is easy.
   Initial state?

   Say initial state is in run, len zero, last zero.  Then if you get a
   few zeros then something else then a short run will be output.
   Seems OK.  While in run mode, keep counting.  While in random mode,
   keep a count of the run.  If run hits margin, output all up to run,
   restart, enter run mode.
*/

/** Add another byte into the run length encoder, flushing as
    required.  The run length encoder uses the Amiga IFF style, where
    a block is prefixed by its run length.  A positive length means
    the next n bytes pass straight through.  A negative length means
    that the next byte is repeated -n times.  A zero terminates the
    chunks.
*/
static void
_rleAppend (RLECTX * self, unsigned c)
{
  int i;

  if (c != self->last)
    {
      /* The run has stopped.  See if it is worthwhile writing it out
         as a run.  Note that the random data comes in as runs of
         length one.
       */
      if (self->runLen > RLE_CHANGE_COST)
        {
          /* Yes, worthwhile. */
          /* Commit whatever was in the buffer. */
          _rleCommit (self);
          emit2 ("!db !immed-%u,!immedbyte", self->runLen, self->last);
        }
      else
        {
          /* Not worthwhile.  Append to the end of the random list. */
          for (i = 0; i < self->runLen; i++)
            {
              if (self->pos >= RLE_MAX_BLOCK)
                {
                  /* Commit. */
                  _rleCommit (self);
                }
              self->buffer[self->pos++] = self->last;
            }
        }
      self->runLen = 1;
      self->last = c;
    }
  else
    {
      if (self->runLen >= RLE_MAX_BLOCK)
        {
          /* Commit whatever was in the buffer. */
          _rleCommit (self);

          emit2 ("!db !immed-%u,!immedbyte", self->runLen, self->last);
          self->runLen = 0;
        }
      self->runLen++;
    }
}

static void
_rleFlush (RLECTX * self)
{
  _rleAppend (self, -1);
  _rleCommit (self);
  self->pos = 0;
  self->last = 0;
  self->runLen = 0;
}

/** genArrayInit - Special code for initialising an array with constant
   data.
*/

static void
genArrayInit (iCode * ic)
{
  literalList *iLoop;
  int ix;
  int elementSize = 0, eIndex, i;
  unsigned val, lastVal;
  sym_link *type;
  RLECTX rle;

  memset (&rle, 0, sizeof (rle));

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);

  _saveRegsForCall (ic, 0);

  fetchPair (PAIR_HL, AOP (IC_LEFT (ic)));
  emit2 ("call __initrleblock");

  type = operandType (IC_LEFT (ic));

  if (type && type->next)
    {
      if (IS_SPEC (type->next) || IS_PTR (type->next))
        {
          elementSize = getSize (type->next);
        }
      else if (IS_ARRAY (type->next) && type->next->next)
        {
          elementSize = getSize (type->next->next);
        }
      else
        {
          printTypeChainRaw (type, NULL);
          wassertl (0, "Can't determine element size in genArrayInit.");
        }
    }
  else
    {
      wassertl (0, "Can't determine element size in genArrayInit.");
    }

  wassertl ((elementSize > 0) && (elementSize <= 4), "Illegal element size in genArrayInit.");

  iLoop = IC_ARRAYILIST (ic);
  lastVal = (unsigned) - 1;

  /* Feed all the bytes into the run length encoder which will handle
     the actual output.
     This works well for mixed char data, and for random int and long
     data.
   */
  while (iLoop)
    {
      ix = iLoop->count;

      for (i = 0; i < ix; i++)
        {
          for (eIndex = 0; eIndex < elementSize; eIndex++)
            {
              val = (((int) iLoop->literalValue) >> (eIndex * 8)) & 0xff;
              _rleAppend (&rle, val);
            }
        }

      iLoop = iLoop->next;
    }

  _rleFlush (&rle);
  /* Mark the end of the run. */
  emit2 (".db 0");

  _restoreRegsAfterCall ();

  spillCached ();

  freeAsmop (IC_LEFT (ic), NULL, ic);
}
#endif

static void
setupForMemcpy (const iCode *ic, const operand *to, const operand *from, const operand *count)
{
  /* Both are in regs. Let regMove() do the shuffling. */
  if (AOP_TYPE (to) == AOP_REG && AOP_TYPE (from) == AOP_REG)
    {
      const short larray[6] = {E_IDX, D_IDX, L_IDX, H_IDX, C_IDX, B_IDX};
      short oparray[6];
      oparray[0] = to->aop->aopu.aop_reg[0]->rIdx;
      oparray[1] = to->aop->aopu.aop_reg[1]->rIdx;
      oparray[2] = from->aop->aopu.aop_reg[0]->rIdx;
      oparray[3] = from->aop->aopu.aop_reg[1]->rIdx;
      if (count && count->aop->type == AOP_REG)
        {
          oparray[4] = count->aop->aopu.aop_reg[0]->rIdx;
          oparray[5] = count->aop->aopu.aop_reg[1]->rIdx;
        }

      regMove (larray, oparray, 4 + (count && count->aop->type == AOP_REG) * 2, false);
    }
  else if (AOP_TYPE (to) == AOP_REG && count && AOP_TYPE (count) == AOP_REG)
    {
      const short larray[4] = {E_IDX, D_IDX, C_IDX, B_IDX};
      short oparray[4];
      oparray[0] = to->aop->aopu.aop_reg[0]->rIdx;
      oparray[1] = to->aop->aopu.aop_reg[1]->rIdx;
      oparray[2] = count->aop->aopu.aop_reg[0]->rIdx;
      oparray[3] = count->aop->aopu.aop_reg[1]->rIdx;

      regMove (larray, oparray, 4 , false);

      fetchPair (PAIR_HL, from->aop);
    }
  else if (AOP_TYPE (from) == AOP_REG && count && AOP_TYPE (count) == AOP_REG)
    {
      const short larray[4] = {L_IDX, H_IDX, C_IDX, B_IDX};
      short oparray[4];
      oparray[0] = from->aop->aopu.aop_reg[0]->rIdx;
      oparray[1] = from->aop->aopu.aop_reg[1]->rIdx;
      oparray[2] = count->aop->aopu.aop_reg[0]->rIdx;
      oparray[3] = count->aop->aopu.aop_reg[1]->rIdx;

      regMove (larray, oparray, 4 , false);

      fetchPair (PAIR_DE, to->aop);
    }
  else if (count && AOP_TYPE (count) == AOP_REG)
    {
      fetchPair (PAIR_BC, count->aop);
      fetchPair (PAIR_DE, to->aop);
      fetchPair (PAIR_HL, from->aop);
    }
  else
    {
      /* DE is free. Write it first. */
      if (AOP_TYPE (from) != AOP_REG || AOP (from)->aopu.aop_reg[0]->rIdx != E_IDX && AOP (from)->aopu.aop_reg[0]->rIdx != D_IDX && AOP (from)->aopu.aop_reg[1]->rIdx != E_IDX && AOP (from)->aopu.aop_reg[1]->rIdx != D_IDX)
        {
          fetchPair (PAIR_DE, AOP (to));
          fetchPair (PAIR_HL, AOP (from));
        }
      /* HL is free. Write it first. */
      else if (AOP_TYPE (to) != AOP_REG || AOP (to)->aopu.aop_reg[0]->rIdx != L_IDX && AOP (to)->aopu.aop_reg[0]->rIdx != H_IDX && AOP (to)->aopu.aop_reg[1]->rIdx != L_IDX && AOP (to)->aopu.aop_reg[1]->rIdx != H_IDX)
        {
          fetchPair (PAIR_HL, AOP (from));
          fetchPair (PAIR_DE, AOP (to));
        }
      /* L is free, but H is not. */
      else if ((AOP_TYPE (to) != AOP_REG || AOP (to)->aopu.aop_reg[0]->rIdx != L_IDX && AOP (to)->aopu.aop_reg[1]->rIdx != L_IDX) &&
        (AOP_TYPE (from) != AOP_REG || AOP (from)->aopu.aop_reg[0]->rIdx != L_IDX && AOP (from)->aopu.aop_reg[1]->rIdx != L_IDX))
        {
          cheapMove (ASMOP_L, 0, AOP (from), 0, true);
          fetchPair (PAIR_DE, AOP (to));
          cheapMove (ASMOP_H, 0, AOP (from), 1, true);
        }
      /* H is free, but L is not. */
      else
        {
          cheapMove (ASMOP_H, 0, AOP (from), 1, true);
          fetchPair (PAIR_DE, AOP (to));
          cheapMove (ASMOP_L, 0, AOP (from), 0, true);
        }
    }
}

static void
genBuiltInMemcpy (const iCode *ic, int nparams, operand **pparams)
{
  int i;
  operand *from, *to, *count;
  bool saved_BC = FALSE, saved_DE = FALSE, saved_HL = FALSE;
  unsigned int n;

  for (i = 0; i < nparams; i++)
    aopOp (pparams[i], ic, FALSE, FALSE);

  wassertl (!IS_GB, "Built-in memcpy() not available on gbz80.");
  wassertl (nparams == 3, "Built-in memcpy() must have three parameters.");
  
  count = pparams[2];
  from = pparams[1];
  to = pparams[0];

  if (pparams[2]->aop->type != AOP_LIT)
    n = UINT_MAX;
  else if (!(n = (unsigned int) ulFromVal (AOP (pparams[2])->aopu.aop_lit))) /* Check for zero length copy. */
    goto done;

  if (!isPairDead (PAIR_HL, ic))
    {
      _push (PAIR_HL);
      saved_HL = TRUE;
    }
  if (!isPairDead (PAIR_DE, ic))
    {
      _push (PAIR_DE);
      saved_DE = TRUE;
    }
  if (!isPairDead (PAIR_BC, ic) && n > 2)
    {
      _push (PAIR_BC);
      saved_BC = TRUE;
    }

 setupForMemcpy (ic, to, from, count);

  if (n == 1)
    {
      emit2 ("ld a, (hl)");
      emit2 ("ld (de), a");
      regalloc_dry_run_cost += 2;
    }
  else if (n == 2)
    {
      emit2 ("ldi");
      emit2 ("ld a, (hl)");
      emit2 ("ld (de), a");
      regalloc_dry_run_cost += 4;
      if (!isPairDead (PAIR_BC, ic)) /* Restore bc. */
        {
          emit2 ("inc bc");
          regalloc_dry_run_cost++;
        }
    }
  else
    {
      symbol *tlbl = 0;
      if (count->aop->type != AOP_REG) // If in reg: Has been fetched early by setupForMemcpy() above.
        fetchPair (PAIR_BC, count->aop);
      if (count->aop->type != AOP_LIT)
        {
          tlbl = newiTempLabel (0);
          emit2 ("ld a, b");
          emit2 ("or a, c");
          emit2 ("jp Z, !tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 5;
        }
      emit2 ("ldir");
      regalloc_dry_run_cost += 2;
      emitLabel (tlbl);
    }

  spillPair (PAIR_HL);

  if (saved_BC)
    _pop (PAIR_BC);
  if (saved_DE)
    _pop (PAIR_DE);
  if (saved_HL)
    _pop (PAIR_HL);

done:
  freeAsmop (count, NULL);
  freeAsmop (to, NULL);
  freeAsmop (from, NULL);

  /* No need to assign result - would have used ordinary memcpy() call instead. */
}

static void
setupForMemset (const iCode *ic, const operand *dst, const operand *c, bool direct_c)
{
  /* Both are in regs. Let regMove() do the shuffling. */
  if (AOP_TYPE (dst) == AOP_REG && !direct_c && AOP_TYPE (c) == AOP_REG)
    {
      const short larray[2] = {L_IDX, H_IDX};
      short oparray[2];
      bool early_a = AOP_TYPE (c) == AOP_REG && (AOP (c)->aopu.aop_reg[0]->rIdx == L_IDX || AOP (c)->aopu.aop_reg[0]->rIdx == H_IDX);

      if (early_a)
        cheapMove (ASMOP_A, 0, AOP (c), 0, true);

      oparray[0] = AOP (dst)->aopu.aop_reg[0]->rIdx;
      oparray[1] = AOP (dst)->aopu.aop_reg[1]->rIdx;

      regMove (larray, oparray, 2, early_a);

      if (!early_a)
        cheapMove (ASMOP_A, 0, AOP (c), 0, true);
    }
  else if (AOP_TYPE (c) == AOP_REG && requiresHL (AOP (c)))
    {
      cheapMove (ASMOP_A, 0, AOP (c), 0, true);
      if (AOP_TYPE (dst) == AOP_EXSTK)
        _push (PAIR_AF);
      fetchPair (PAIR_HL, AOP (dst));
      if (AOP_TYPE (dst) == AOP_EXSTK)
        _pop (PAIR_AF);
    }
  else
    {
      fetchPair (PAIR_HL, AOP (dst));
      if (!direct_c)
        {
          if (requiresHL (AOP (c)))
            _push (PAIR_HL);
          cheapMove (ASMOP_A, 0, AOP (c), 0, true);
          if (requiresHL (AOP (c)))
            _pop (PAIR_HL);
        }
    }
}

static void
genBuiltInMemset (const iCode *ic, int nParams, operand **pparams)
{
  operand *dst, *c, *n;
  bool direct_c, direct_cl;
  bool indirect_c;
  bool preinc = FALSE;
  unsigned long sizecost_ldir, sizecost_direct, sizecost_loop;
  bool double_loop;
  unsigned size;
  bool live_BC = !isPairDead (PAIR_BC, ic), live_DE = !isPairDead (PAIR_DE, ic), live_HL = !isPairDead (PAIR_HL, ic), live_B = bitVectBitValue (ic->rSurv, B_IDX);
  bool saved_BC = FALSE, saved_DE = FALSE, saved_HL = FALSE;

  wassertl (nParams == 3, "Built-in memset() must have three parameters");

  dst = pparams[0];
  c = pparams[1];
  n = pparams[2];

  aopOp (c, ic, FALSE, FALSE);
  aopOp (dst, ic, FALSE, FALSE);
  aopOp (n, ic, FALSE, FALSE);

  wassertl (AOP_TYPE (n) == AOP_LIT, "Last parameter to builtin memset() must be literal.");
  if(!(size = ulFromVal (AOP (n)->aopu.aop_lit)))
    goto done;

  direct_c = (AOP_TYPE (c) == AOP_LIT || AOP_TYPE (c) == AOP_REG && AOP (c)->aopu.aop_reg[0]->rIdx != H_IDX
              && AOP (c)->aopu.aop_reg[0]->rIdx != L_IDX);
  direct_cl = (AOP_TYPE (c) == AOP_LIT || AOP_TYPE (c) == AOP_REG && AOP (c)->aopu.aop_reg[0]->rIdx != H_IDX
              && AOP (c)->aopu.aop_reg[0]->rIdx != L_IDX && AOP (c)->aopu.aop_reg[0]->rIdx != B_IDX);
  indirect_c = IS_R3KA && ulFromVal (AOP (n)->aopu.aop_lit) > 1 && AOP_TYPE (c) == AOP_IY;

  double_loop = (size > 255 || optimize.codeSpeed);

  sizecost_direct = 3 + 2 * size - 1 + !direct_c * ld_cost (ASMOP_A, AOP (c));
  sizecost_direct += (live_HL) * 2;
  sizecost_loop = 9 + double_loop * 2 + ((size % 2) && double_loop) * 2 + !direct_cl * ld_cost (ASMOP_A, AOP (c));
  sizecost_loop += (live_HL + live_B) * 2;
  sizecost_ldir = indirect_c ? 11 : (12 + !direct_c * ld_cost (ASMOP_A, AOP (c)) - (IS_R3KA && !optimize.codeSpeed));
  sizecost_ldir += (live_HL + live_DE + live_BC) * 2;

  if (sizecost_direct <= sizecost_loop && sizecost_direct < sizecost_ldir) // straight-line code.
    {
      if (live_HL)
        {
          _push (PAIR_HL);
          saved_HL = TRUE;
        }

      setupForMemset (ic, dst, c, direct_c);

      regalloc_dry_run_cost += (size * 2 - 1);
      if (!regalloc_dry_run)
        while (size--)
		  {
            emit2 ("ld (hl), %s", aopGet (direct_c ? AOP (c) : ASMOP_A, 0, FALSE));
            if (size)
              emit2 ("inc hl");
          }
    }
  else if (size <= 510 && sizecost_loop < sizecost_ldir) // Loop
    {
      symbol *tlbl1 = regalloc_dry_run ? 0 : newiTempLabel (NULL);
      symbol *tlbl2 = regalloc_dry_run ? 0 : newiTempLabel (NULL);

      if (live_HL)
        {
          _push (PAIR_HL);
          saved_HL = TRUE;
        }
      if (bitVectBitValue (ic->rSurv, B_IDX))
        {
          _push (PAIR_BC);
          saved_BC = TRUE;
        }

      setupForMemset (ic, dst, c, direct_cl);

      emit2 ("ld b, !immedbyte", double_loop ? (size / 2 + size % 2) : size);
      regalloc_dry_run_cost += 2;

      if (double_loop && size % 2)
        {
          if (!regalloc_dry_run)
            emit2 ("jr !tlabel", labelKey2num (tlbl2->key));
          regalloc_dry_run_cost += 2;
        }

      if (!regalloc_dry_run)
        {
          emitLabel (tlbl1);
          emit2 ("ld (hl), %s", aopGet (direct_cl ? AOP (c) : ASMOP_A, 0, FALSE));
          emit2 ("inc hl");
          if (double_loop)
            {
              if (size % 2)
                emitLabel (tlbl2);
              emit2 ("ld (hl), %s", aopGet (direct_cl ? AOP (c) : ASMOP_A, 0, FALSE));
              emit2 ("inc hl");
            }
          emit2 ("djnz !tlabel", labelKey2num (tlbl1->key));
        }
      regalloc_dry_run_cost += (double_loop ? 6 : 4);
    }
  else // Use ldir / lsidr
    {
      if (live_HL)
        {
          _push (PAIR_HL);
          saved_HL = TRUE;
        }
      if (live_DE)
        {
          _push (PAIR_DE);
          saved_DE = TRUE;
        }
      if (live_BC)
        {
          _push (PAIR_BC);
          saved_BC = TRUE;
        }
	  if (indirect_c)
		{
		  fetchPair (PAIR_DE, AOP (dst));
		  emit2 ("ld hl, !hashedstr", AOP (c)->aopu.aop_dir);
		  regalloc_dry_run_cost += 3;
		}
	  else
		{
		  setupForMemset (ic, dst, c, direct_c);

		  if (!regalloc_dry_run)
		    emit2 ("ld (hl), %s", aopGet (direct_c ? AOP (c) : ASMOP_A, 0, FALSE));
		  regalloc_dry_run_cost += (direct_c && AOP_TYPE (c) == AOP_LIT) ? 2 : 1;
		  if (ulFromVal (AOP (n)->aopu.aop_lit) <= 1)
		    goto done;

		  emit2 ("ld e, l");
		  emit2 ("ld d, h");
		  regalloc_dry_run_cost += 2;
		  if (!IS_R3KA || optimize.codeSpeed)
		    {
		      emit2 ("inc de");
		      regalloc_dry_run_cost++;
		      preinc = TRUE;
		    }
		}
	  emit2 ("ld bc, !immedword", size - preinc);
	  emit2 (IS_R3KA ? "lsidr" : "ldir");
	  regalloc_dry_run_cost += 5;
    }

done:
  spillPair (PAIR_HL);

  freeAsmop (n, NULL);
  freeAsmop (c, NULL);
  freeAsmop (dst, NULL);

  
  if (saved_BC)
    _pop (PAIR_BC);
  if (saved_DE)
    _pop (PAIR_DE);
  if (saved_HL)
    _pop (PAIR_HL);

  /* No need to assign result - would have used ordinary memset() call instead. */
}

static void
genBuiltInStrcpy (const iCode *ic, int nParams, operand **pparams)
{
  operand *dst, *src;
  bool saved_BC = FALSE, saved_DE = FALSE, saved_HL = FALSE;
  int i;
  bool SomethingReturned;
  
  SomethingReturned = (IS_ITEMP (IC_RESULT (ic)) &&
                      (OP_SYMBOL (IC_RESULT (ic))->nRegs ||
                      OP_SYMBOL (IC_RESULT (ic))->spildir ||
                      OP_SYMBOL (IC_RESULT (ic))->accuse == ACCUSE_A)) || IS_TRUE_SYMOP (IC_RESULT (ic));

  wassertl (nParams == 2, "Built-in strcpy() must have two parameters.");
  wassertl (!IS_GB, "Built-in strcpy() not available for gbz80.");

  dst = pparams[0];
  src = pparams[1];

  for (i = 0; i < nParams; i++)
    aopOp (pparams[i], ic, FALSE, FALSE);

  if (!isPairDead (PAIR_HL, ic))
    {
      _push (PAIR_HL);
      saved_HL = TRUE;
    }
  if (!isPairDead (PAIR_BC, ic))
    {
      _push (PAIR_BC);
      saved_BC = TRUE;
    }
  if (!isPairDead (PAIR_DE, ic))
    {
      _push (PAIR_DE);
      saved_DE = TRUE;
    }

  setupForMemcpy (ic, dst, src, 0);

  emit3 (A_XOR, ASMOP_A, ASMOP_A);
  if (SomethingReturned)
    _push (PAIR_DE);
  if (!regalloc_dry_run)
    {
      symbol *tlbl = newiTempLabel (NULL);
      emitLabel (tlbl);
      emit2 ("cp a, (hl)");
      emit2 ("ldi");
      emit2 ("jr NZ, !tlabel", labelKey2num (tlbl->key));
    }
  regalloc_dry_run_cost += 5;

  spillPair (PAIR_HL);

  if (SomethingReturned)
    aopOp (IC_RESULT (ic), ic, FALSE, FALSE);

  if (!SomethingReturned || SomethingReturned && getPairId (AOP (IC_RESULT (ic))) != PAIR_INVALID)
    {
      if (SomethingReturned)
        _pop (getPairId (AOP (IC_RESULT (ic))));
      if (saved_DE)
        _pop (PAIR_DE);
      if (saved_BC)
        _pop (PAIR_BC);
      if (saved_HL)
        _pop (PAIR_HL);
    }
  else
    {
      _pop (PAIR_HL);
      assignResultValue (IC_RESULT (ic));

      restoreRegs (0, saved_DE, saved_BC, saved_HL, IC_RESULT (ic));
    }

  if (SomethingReturned)
    freeAsmop (IC_RESULT (ic), NULL);
  freeAsmop (src, NULL);
  freeAsmop (dst, NULL);
}

static void
genBuiltInStrncpy (const iCode *ic, int nparams, operand **pparams)
{
  int i;
  operand *s1, *s2, *n;
  bool saved_BC = FALSE, saved_DE = FALSE, saved_HL = FALSE;

  for (i = 0; i < nparams; i++)
    aopOp (pparams[i], ic, FALSE, FALSE);

  wassertl (!IS_GB, "Built-in strncpy() not available on gbz80.");
  wassertl (nparams == 3, "Built-in strncpy() must have three parameters.");
  wassertl (AOP_TYPE (pparams[2]) == AOP_LIT, "Last parameter to builtin strncpy() must be literal.");

  s1 = pparams[0];
  s2 = pparams[1];
  n = pparams[2];

  if (!ulFromVal (AOP (n)->aopu.aop_lit))
    goto done;

  if (!isPairDead (PAIR_HL, ic))
    {
      _push (PAIR_HL);
      saved_HL = TRUE;
    }
  if (!isPairDead (PAIR_BC, ic))
    {
      _push (PAIR_BC);
      saved_BC = TRUE;
    }
  if (!isPairDead (PAIR_DE, ic))
    {
      _push (PAIR_DE);
      saved_DE = TRUE;
    }

  setupForMemcpy (ic, s1, s2, 0);

  fetchPair (PAIR_BC, AOP (n));

  emit3 (A_XOR, ASMOP_A, ASMOP_A);
  if (!regalloc_dry_run)
    {
      symbol *tlbl1 = newiTempLabel (0);
      symbol *tlbl2 = newiTempLabel (0);
      symbol *tlbl3 = newiTempLabel (0);
      emitLabel (tlbl2);
      emit2 ("cp a, (hl)");
      emit2 ("ldi");
      emit2 (IS_RAB ? "jp LZ, !tlabel" : "jp PO, !tlabel", labelKey2num (tlbl1->key));
      emit2 ("jr NZ, !tlabel", labelKey2num (tlbl2->key));
      emitLabel (tlbl3);
      emit2 ("dec hl");
      emit2 ("ldi");
      emit2 (IS_RAB ? "jp LO, !tlabel" : "jp PE, !tlabel", labelKey2num (tlbl3->key));
      emitLabel (tlbl1);
    }
  regalloc_dry_run_cost += 14;

  spillPair (PAIR_HL);

  restoreRegs (0, saved_DE, saved_BC, saved_HL, 0);

done:
  freeAsmop (n, NULL);
  freeAsmop (s2, NULL);
  freeAsmop (s1, NULL);
}

static void
genBuiltInStrchr (const iCode *ic, int nParams, operand **pparams)
{
  operand *s, *c;
  bool saved_BC = FALSE, saved_DE = FALSE, saved_HL = FALSE;
  int i;
  bool SomethingReturned;
  PAIR_ID pair;
  bool direct_c;
  asmop *aop_c;
  symbol *tlbl1 = regalloc_dry_run ? 0 : newiTempLabel(0);
  symbol *tlbl2 = regalloc_dry_run ? 0 : newiTempLabel(0);

  SomethingReturned = (IS_ITEMP (IC_RESULT (ic)) &&
                      (OP_SYMBOL (IC_RESULT (ic))->nRegs ||
                      OP_SYMBOL (IC_RESULT (ic))->spildir ||
                      OP_SYMBOL (IC_RESULT (ic))->accuse == ACCUSE_A)) || IS_TRUE_SYMOP (IC_RESULT (ic));

  wassertl (nParams == 2, "Built-in strchr() must have two parameters.");

  s = pparams[0];
  c = pparams[1];

  for (i = 0; i < nParams; i++)
    aopOp (pparams[i], ic, FALSE, FALSE);

  if (SomethingReturned)
    aopOp (IC_RESULT (ic), ic, FALSE, FALSE);

  if (getPairId (AOP (s)) != PAIR_INVALID && getPairId (AOP (s)) != PAIR_IY)
    pair = getPairId (AOP (s));
  else if (SomethingReturned && getPairId (AOP (IC_RESULT (ic))) != PAIR_INVALID && getPairId (AOP (IC_RESULT (ic))) != PAIR_IY)
    pair = getPairId (AOP (IC_RESULT (ic)));
  else
    pair = PAIR_HL;

  if (AOP_TYPE (c) == AOP_REG && AOP (c)->aopu.aop_reg[0]->rIdx != IYL_IDX && AOP (c)->aopu.aop_reg[0]->rIdx != IYH_IDX &&
    !(pair == PAIR_HL && (AOP (c)->aopu.aop_reg[0]->rIdx == L_IDX || AOP (c)->aopu.aop_reg[0]->rIdx == H_IDX)) &&
    !(pair == PAIR_DE && (AOP (c)->aopu.aop_reg[0]->rIdx == E_IDX || AOP (c)->aopu.aop_reg[0]->rIdx == D_IDX)) &&
    !(pair == PAIR_BC && (AOP (c)->aopu.aop_reg[0]->rIdx == B_IDX || AOP (c)->aopu.aop_reg[0]->rIdx == C_IDX)))
    direct_c = TRUE;
  else if (AOP_TYPE (c) == AOP_LIT && optimize.codeSize)
    direct_c = TRUE;
  else
    direct_c = FALSE;

  aop_c = direct_c ? AOP (c) : (pair == PAIR_DE ? ASMOP_H : ASMOP_D);

  if ((pair == PAIR_HL || pair == PAIR_DE && !direct_c) && !isPairDead (PAIR_HL, ic))
    {
      _push (PAIR_HL);
      saved_HL = TRUE;
    }
  if (pair == PAIR_BC && !isPairDead (PAIR_BC, ic))
    {
      _push (PAIR_BC);
      saved_BC = TRUE;
    }
  if ((pair == PAIR_DE || !direct_c) && !isPairDead (PAIR_DE, ic))
    {
      _push (PAIR_DE);
      saved_DE = TRUE;
    }

  if (!direct_c)
    cheapMove (aop_c, 0, AOP (c), 0, true);
  fetchPair (pair, AOP (s));

  if (!regalloc_dry_run)
    emitLabel (tlbl2);
  emit2 ("ld a, (%s)", _pairs[pair].name);
  regalloc_dry_run_cost++;
  emit3 (A_CP, ASMOP_A, aop_c);
  if (!regalloc_dry_run)
    emit2 ("jp Z, !tlabel", labelKey2num (tlbl1->key));
  emit2 ("or a, a");
  emit2 ("inc %s", _pairs[pair].name);
  if (!regalloc_dry_run)
    emit2 ("jr NZ, !tlabel", labelKey2num (tlbl2->key));
  emit2 ("ld %s, a", _pairs[pair].l);
  emit2 ("ld %s, a", _pairs[pair].h);
  regalloc_dry_run_cost += 8; // jp will most likely be optimized into jr.
  if (!regalloc_dry_run)
    emitLabel (tlbl1);
  if (SomethingReturned)
    commitPair (AOP (IC_RESULT (ic)), pair, ic, FALSE);

  restoreRegs (0, saved_DE, saved_BC, saved_HL, SomethingReturned ? IC_RESULT (ic) : 0);

  if (SomethingReturned)
    freeAsmop (IC_RESULT (ic), NULL);
  freeAsmop (c, NULL);
  freeAsmop (s, NULL);
}

/*-----------------------------------------------------------------*/
/* genBuiltIn - calls the appropriate function to generate code    */
/* for a built in function                                         */
/*-----------------------------------------------------------------*/
static void
genBuiltIn (iCode *ic)
{
  operand *bi_parms[MAX_BUILTIN_ARGS];
  int nbi_parms;
  iCode *bi_iCode;
  symbol *bif;

  /* get all the arguments for a built in function */
  bi_iCode = getBuiltinParms (ic, &nbi_parms, bi_parms);

  /* which function is it */
  bif = OP_SYMBOL (IC_LEFT (bi_iCode));

  wassertl (!ic->prev || ic->prev->op != SEND || !ic->prev->builtinSEND, "genBuiltIn() must be called on first SEND icode only.");

  if (!strcmp (bif->name, "__builtin_memcpy"))
    {
      genBuiltInMemcpy (bi_iCode, nbi_parms, bi_parms);
    }
  else if (!strcmp (bif->name, "__builtin_strcpy"))
    {
      genBuiltInStrcpy (bi_iCode, nbi_parms, bi_parms);
    }
  else if (!strcmp (bif->name, "__builtin_strncpy"))
    {
      genBuiltInStrncpy (bi_iCode, nbi_parms, bi_parms);
    }
  else if (!strcmp (bif->name, "__builtin_strchr"))
    {
      genBuiltInStrchr (bi_iCode, nbi_parms, bi_parms);
    }
  else if (!strcmp (bif->name, "__builtin_memset"))
    {
      genBuiltInMemset (bi_iCode, nbi_parms, bi_parms);
    }
  else
    {
      wassertl (0, "Unknown builtin function encountered");
    }
}

/*-------------------------------------------------------------------------------------*/
/* genZ80iCode - generate code for Z80 based controllers for a single iCode instruction*/
/*-------------------------------------------------------------------------------------*/
static void
genZ80iCode (iCode * ic)
{
  genLine.lineElement.ic = ic;

  /* if the result is marked as
     spilt and rematerializable or code for
     this has already been generated then
     do nothing */
  if (resultRemat (ic) || ic->generated)
    return;

  /* depending on the operation */
  switch (ic->op)
    {
    case '!':
      emitDebug ("; genNot");
      genNot (ic);
      break;

    case '~':
      emitDebug ("; genCpl");
      genCpl (ic);
      break;

    case UNARYMINUS:
      emitDebug ("; genUminus");
      genUminus (ic);
      break;

    case IPUSH:
      emitDebug ("; genIpush");
      genIpush (ic);
      break;

    case IPOP:
      /* IPOP happens only when trying to restore a
         spilt live range, if there is an ifx statement
         following this pop then the if statement might
         be using some of the registers being popped which
         would destroy the contents of the register so
         we need to check for this condition and handle it */
      if (ic->next && ic->next->op == IFX && regsInCommon (IC_LEFT (ic), IC_COND (ic->next)))
        {
          emitDebug ("; genIfx");
          genIfx (ic->next, ic);
        }
      else
        {
          emitDebug ("; genIpop");
          genIpop (ic);
        }
      break;

    case CALL:
    case PCALL:
      emitDebug ("; genCall");
      genCall (ic);
      break;

    case FUNCTION:
      emitDebug ("; genFunction");
      genFunction (ic);
      break;

    case ENDFUNCTION:
      emitDebug ("; genEndFunction");
      genEndFunction (ic);
      break;

    case RETURN:
      emitDebug ("; genRet");
      genRet (ic);
      break;

    case LABEL:
      emitDebug ("; genLabel");
      genLabel (ic);
      break;

    case GOTO:
      emitDebug ("; genGoto");
      genGoto (ic);
      break;

    case '+':
      emitDebug ("; genPlus");
      genPlus (ic);
      break;

    case '-':
      emitDebug ("; genMinus");
      genMinus (ic);
      break;

    case '*':
      emitDebug ("; genMult");
      genMult (ic);
      break;

    case '/':
      emitDebug ("; genDiv");
      genDiv (ic);
      break;

    case '%':
      emitDebug ("; genMod");
      genMod (ic);
      break;

    case '>':
      emitDebug ("; genCmpGt");
      genCmpGt (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case '<':
      emitDebug ("; genCmpLt");
      genCmpLt (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case LE_OP:
    case GE_OP:
    case NE_OP:

      /* note these two are xlated by algebraic equivalence
         during parsing SDCC.y */
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "got '>=' or '<=' shouldn't have come here");
      break;

    case EQ_OP:
      emitDebug ("; genCmpEq");
      genCmpEq (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case AND_OP:
      emitDebug ("; genAndOp");
      genAndOp (ic);
      break;

    case OR_OP:
      emitDebug ("; genOrOp");
      genOrOp (ic);
      break;

    case '^':
      emitDebug ("; genXor");
      genXor (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case '|':
      emitDebug ("; genOr");
      genOr (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case BITWISEAND:
      emitDebug ("; genAnd");
      genAnd (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case INLINEASM:
      emitDebug ("; genInline");
      genInline (ic);
      break;

    case RRC:
      emitDebug ("; genRRC");
      genRRC (ic);
      break;

    case RLC:
      emitDebug ("; genRLC");
      genRLC (ic);
      break;

    case GETHBIT:
      emitDebug ("; genGetHbit");
      genGetHbit (ic);
      break;

    case GETABIT:
      emitDebug ("; genGetAbit");
      genGetAbit (ic);
      break;

    case LEFT_OP:
      emitDebug ("; genLeftShift");
      genLeftShift (ic);
      break;

    case RIGHT_OP:
      emitDebug ("; genRightShift");
      genRightShift (ic);
      break;

    case GET_VALUE_AT_ADDRESS:
      emitDebug ("; genPointerGet");
      genPointerGet (ic);
      break;

    case '=':

      if (POINTER_SET (ic))
        {
          emitDebug ("; genAssign (pointer)");
          genPointerSet (ic);
        }
      else
        {
          emitDebug ("; genAssign");
          genAssign (ic);
        }
      break;

    case IFX:
      emitDebug ("; genIfx");
      genIfx (ic, NULL);
      break;

    case ADDRESS_OF:
      emitDebug ("; genAddrOf");
      genAddrOf (ic);
      break;

    case JUMPTABLE:
      emitDebug ("; genJumpTab");
      genJumpTab (ic);
      break;

    case CAST:
      emitDebug ("; genCast");
      genCast (ic);
      break;

    case RECEIVE:
      emitDebug ("; genReceive");
      genReceive (ic);
      break;

    case SEND:
      if (ic->builtinSEND)
        {
          emitDebug ("; genBuiltIn");
          genBuiltIn (ic);
        }
      else
        {
          emitDebug ("; genSend");
          genSend (ic);
        }
      break;

#if 0
    case ARRAYINIT:
      emitDebug ("; genArrayInit");
      genArrayInit (ic);
      break;
#endif

    case DUMMY_READ_VOLATILE:
      emitDebug ("; genDummyRead");
      genDummyRead (ic);
      break;

    case CRITICAL:
      emitDebug ("; genCritical");
      genCritical (ic);
      break;

    case ENDCRITICAL:
      emitDebug ("; genEndCritical");
      genEndCritical (ic);
      break;

    default:
      ;
    }
}

unsigned char
dryZ80iCode (iCode * ic)
{
  regalloc_dry_run = TRUE;
  regalloc_dry_run_cost = 0;

  initGenLineElement ();
  _G.omitFramePtr = should_omit_frame_ptr;

  genZ80iCode (ic);

  destroy_line_list ();
  freeTrace (&_G.trace.aops);

  {
    int pairId;
    for (pairId = 0; pairId < NUM_PAIRS; pairId++)
      spillPair (pairId);
  }

  return (regalloc_dry_run_cost);
}

#ifdef DEBUG_DRY_COST
static void
dryZ80Code (iCode * lic)
{
  iCode *ic;

  for (ic = lic; ic; ic = ic->next)
    if (ic->op != FUNCTION && ic->op != ENDFUNCTION && ic->op != LABEL && ic->op != GOTO && ic->op != INLINEASM)
      printf ("; iCode %d total cost: %d\n", ic->key, (int) (dryZ80iCode (ic)));
}
#endif

/*-------------------------------------------------------------------------------------*/
/* genZ80Code - generate code for Z80 based controllers for a block of intructions     */
/*-------------------------------------------------------------------------------------*/
void
genZ80Code (iCode * lic)
{
#ifdef DEBUG_DRY_COST
  dryZ80Code (lic);
#endif

  iCode *ic;
  int cln = 0;
  regalloc_dry_run = FALSE;

  initGenLineElement ();

  memset(z80_regs_used_as_parms_in_calls_from_current_function, 0, sizeof(bool) * (IYH_IDX + 1));
  z80_symmParm_in_calls_from_current_function = TRUE;
  memset(z80_regs_preserved_in_calls_from_current_function, 0, sizeof(bool) * (IYH_IDX + 1));

  /* if debug information required */
  if (options.debug && currFunc)
    {
      debugFile->writeFunction (currFunc, lic);
    }

  for (ic = lic; ic; ic = ic->next)
    ic->generated = FALSE;

  /* Generate Code for all instructions */
  for (ic = lic; ic; ic = ic->next)
    {
      if (ic->lineno && cln != ic->lineno)
        {
          if (options.debug)
            debugFile->writeCLine (ic);
          if (!options.noCcodeInAsm)
            emit2 (";%s:%d: %s", ic->filename, ic->lineno, printCLine (ic->filename, ic->lineno));
          cln = ic->lineno;
        }
      if (options.iCodeInAsm)
        {
          const char *iLine = printILine (ic);
          emit2 (";ic:%d: %s", ic->key, iLine);
          dbuf_free (iLine);
        }
      regalloc_dry_run_cost = 0;
      genZ80iCode (ic);

#ifdef DEBUG_DRY_COST
      emit2 ("; iCode %d total cost: %d\n", ic->key, regalloc_dry_run_cost);
#endif
    }

  /* now we are ready to call the
     peep hole optimizer */
  if (!options.nopeep)
    peepHole (&genLine.lineHead);

  /* This is unfortunate */
  /* now do the actual printing */
  {
    struct dbuf_s *buf = codeOutBuf;
    if (isInHome () && codeOutBuf == &code->oBuf)
      codeOutBuf = &home->oBuf;
    printLine (genLine.lineHead, codeOutBuf);
    if (_G.flushStatics)
      {
        flushStatics ();
        _G.flushStatics = 0;
      }
    codeOutBuf = buf;
  }

  {
    int pairId;
    for (pairId = 0; pairId < NUM_PAIRS; pairId++)
      spillPair (pairId);
  }

  destroy_line_list ();
  freeTrace (&_G.trace.aops);
}

