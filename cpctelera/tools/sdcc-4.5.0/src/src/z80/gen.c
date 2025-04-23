/*-------------------------------------------------------------------------
  gen.c - code generator for Z80 and related.

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Copyright (C) 2000, Michael Hope <michaelh@juju.net.nz>
  Copyright (C) 2011-2024, Philipp Klaus Krause pkk@spth.de, philipp@informatik.uni-frankfurt.de, philipp@colecovision.eu)
  Copyright (C) 2021-2022, Sebastian 'basxto' Riedel <sdcc@basxto.de>

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

enum
{
  /* Set to enable debugging trace statements in the output assembly code. */
  DISABLE_DEBUG = 0
};

#define UNIMPLEMENTED do {wassertl (regalloc_dry_run, "Unimplemented"); cost (4000, 4000.0f);} while(0)

#undef DEBUG_DRY_COST

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
  A_EX,
  A_INC,
  A_LD,
  A_NEG,
  A_OR,
  A_RL,
  A_RLA,
  A_RLC,
  A_RLCA,
  A_RLD,
  A_RR,
  A_RRA,
  A_RRC,
  A_RRCA,
  A_RRD,
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
  "ex",
  "inc",
  "ld",
  "neg",
  "or",
  "rl",
  "rla",
  "rlc",
  "rlca",
  "rld",
  "rr",
  "rra",
  "rrc",
  "rrca",
  "rrd",
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
    const char *base;   // For addresses
    unsigned int value; // For AOP_LIT
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

static struct asmop asmop_a, asmop_b, asmop_c, asmop_d, asmop_e, asmop_h, asmop_l, asmop_iyh, asmop_iyl, asmop_hl, asmop_de, asmop_bc, asmop_iy, asmop_dehl, asmop_hlde, asmop_hlbc, asmop_debc, asmop_zero, asmop_one, asmop_mone;
static struct asmop *const ASMOP_A = &asmop_a;
static struct asmop *const ASMOP_B = &asmop_b;
static struct asmop *const ASMOP_C = &asmop_c;
static struct asmop *const ASMOP_D = &asmop_d;
static struct asmop *const ASMOP_E = &asmop_e;
static struct asmop *const ASMOP_H = &asmop_h;
static struct asmop *const ASMOP_L = &asmop_l;
static struct asmop *const ASMOP_IYH = &asmop_iyh;
static struct asmop *const ASMOP_IYL = &asmop_iyl;
static struct asmop *const ASMOP_HL = &asmop_hl;
static struct asmop *const ASMOP_DE = &asmop_de;
static struct asmop *const ASMOP_BC = &asmop_bc;
static struct asmop *const ASMOP_IY = &asmop_iy;
static struct asmop *const ASMOP_DEHL = &asmop_dehl;
static struct asmop *const ASMOP_HLDE = &asmop_hlde;
static struct asmop *const ASMOP_HLBC = &asmop_hlbc;
static struct asmop *const ASMOP_DEBC = &asmop_debc;
static struct asmop *const ASMOP_ZERO = &asmop_zero;
static struct asmop *const ASMOP_ONE = &asmop_one;
static struct asmop *const ASMOP_MONE = &asmop_mone;

static asmop *asmopregs[] = { &asmop_a, &asmop_c, &asmop_b, &asmop_e, &asmop_d, &asmop_l, &asmop_h, &asmop_iyl, &asmop_iyh };

// Init aop as a an asmop for data in registers, as given by the -1-terminated array regidx.
static void
z80_init_reg_asmop(asmop *aop, const signed char *regidx)
{
  aop->type = AOP_REG;
  aop->size = 0;
  memset (aop->regs, -1, sizeof(aop->regs));
  
  for(int i = 0; regidx[i] >= 0; i++)
    {
      aop->aopu.aop_reg[i] = regsZ80 + regidx[i];
      aop->regs[regidx[i]] = i;
      aop->size++;
    }

  aop->valinfo.anything = true;
}

void
z80_init_asmops (void)
{
  z80_init_reg_asmop(&asmop_a, (const signed char[]){A_IDX, -1});
  z80_init_reg_asmop(&asmop_b, (const signed char[]){B_IDX, -1});
  z80_init_reg_asmop(&asmop_c, (const signed char[]){C_IDX, -1});
  z80_init_reg_asmop(&asmop_d, (const signed char[]){D_IDX, -1});
  z80_init_reg_asmop(&asmop_e, (const signed char[]){E_IDX, -1});
  z80_init_reg_asmop(&asmop_h, (const signed char[]){H_IDX, -1});
  z80_init_reg_asmop(&asmop_l, (const signed char[]){L_IDX, -1});
  z80_init_reg_asmop(&asmop_iyh, (const signed char[]){IYH_IDX, -1});
  z80_init_reg_asmop(&asmop_iyl, (const signed char[]){IYL_IDX, -1});
  z80_init_reg_asmop(&asmop_bc, (const signed char[]){C_IDX, B_IDX, -1});
  z80_init_reg_asmop(&asmop_de, (const signed char[]){E_IDX, D_IDX, -1});
  z80_init_reg_asmop(&asmop_hl, (const signed char[]){L_IDX, H_IDX, -1});
  z80_init_reg_asmop(&asmop_dehl, (const signed char[]){L_IDX, H_IDX, E_IDX, D_IDX, -1});
  z80_init_reg_asmop(&asmop_hlde, (const signed char[]){E_IDX, D_IDX, L_IDX, H_IDX, -1});
  z80_init_reg_asmop(&asmop_iy, (const signed char[]){IYL_IDX, IYH_IDX, -1});
  z80_init_reg_asmop(&asmop_hlbc, (const signed char[]){C_IDX, B_IDX, L_IDX, H_IDX, -1});
  z80_init_reg_asmop(&asmop_debc, (const signed char[]){C_IDX, B_IDX, E_IDX, D_IDX, -1});
  
  asmop_zero.type = AOP_LIT;
  asmop_zero.aopu.aop_lit = constVal ("0");
  asmop_zero.size = 1;
  memset (asmop_zero.regs, -1, 9);
  asmop_zero.valinfo.anything = true;

  asmop_one.type = AOP_LIT;
  asmop_one.aopu.aop_lit = constVal ("1");
  asmop_one.size = 1;
  memset (asmop_one.regs, -1, 9);
  asmop_one.valinfo.anything = true;

  asmop_mone.type = AOP_LIT;
  asmop_mone.aopu.aop_lit = constVal ("-1");
  asmop_mone.size = 1;
  memset (asmop_mone.regs, -1, 9);
  asmop_mone.valinfo.anything = true;
}

static bool regalloc_dry_run;
static unsigned int regalloc_dry_run_cost; // Legacy: cost counted in bytes only (i.e. states have been ignored for corresponding instructions).
static unsigned long regalloc_dry_run_cost_bytes;
static float regalloc_dry_run_cost_states;
static float regalloc_dry_run_state_scale = 1.0f;

static void
cost (unsigned int bytes, float states)
{
  regalloc_dry_run_cost_bytes += bytes;
  regalloc_dry_run_cost_states += states * regalloc_dry_run_state_scale;
}

static void
cost2 (unsigned int bytes, unsigned int z80_states /* also z80n */, unsigned int z180_states, unsigned int r2k_clocks, unsigned int sm83_cycles, unsigned int tlcs90_states, unsigned int ez80_z80_cycles, unsigned int r800_cycles)
{
  regalloc_dry_run_cost_bytes += bytes;
  if (IS_Z80 || IS_Z80N)
    regalloc_dry_run_cost_states += z80_states * regalloc_dry_run_state_scale;
  else if (IS_Z180)
    regalloc_dry_run_cost_states += z180_states * regalloc_dry_run_state_scale;
  else if (IS_RAB)
    regalloc_dry_run_cost_states += r2k_clocks * regalloc_dry_run_state_scale;
  else if (IS_SM83)
    regalloc_dry_run_cost_states += sm83_cycles * regalloc_dry_run_state_scale;
  else if(IS_TLCS90)
    regalloc_dry_run_cost_states += tlcs90_states * regalloc_dry_run_state_scale;
  else if(IS_EZ80_Z80)
    regalloc_dry_run_cost_states += ez80_z80_cycles * regalloc_dry_run_state_scale;
  else if(IS_R800)
    regalloc_dry_run_cost_states += r800_cycles * regalloc_dry_run_state_scale;
  else
    wassert (0);
}

/*-----------------------------------------------------------------*/
/* isRegIdxPair - true, if specified index is register pair,       */
/*                rIdx changed to index of lower register          */
/*-----------------------------------------------------------------*/
static bool
isRegIdxPair (short *rIdx)
{
  switch (*rIdx)
    {
    case BC_IDX:
      *rIdx = C_IDX;
      break;
    case DE_IDX:
      *rIdx = E_IDX;
      break;
    case HL_IDX:
      *rIdx = L_IDX;
      break;
    case IY_IDX:
      *rIdx = IYL_IDX;
      break;
    default:
      return false;
    }
  return true;
}

/*-----------------------------------------------------------------*/
/* aopRegOffset - return register offset in the asmop              */
/*-----------------------------------------------------------------*/
static int
aopRegOffset (const asmop *aop, short rIdx)
{
  if (rIdx < 0 || aop->type != AOP_REG)
    return -1;

  if (!isRegIdxPair (&rIdx))
    return aop->regs[rIdx];

  int offset = aop->regs[rIdx];
  return (offset != -1 && aop->regs[rIdx+1] == offset+1) ? offset : -1;
}

/*-----------------------------------------------------------------*/
/* aopUseReg - return true if register is in the asmop             */
/*-----------------------------------------------------------------*/
static inline bool
aopRegUsed (const asmop *aop, short rIdx)
{
  if (aop->type != AOP_REG)
    return false;

  if (!isRegIdxPair (&rIdx))
    return aop->regs[rIdx];

  return aop->regs[rIdx] != -1 || aop->regs[rIdx+1] != -1;
}

/*-----------------------------------------------------------------*/
/* aopRegUsedRange - true if register is in specified position range [minPos;maxPos) */
/*-----------------------------------------------------------------*/
static inline bool
aopRegUsedRange (const asmop *aop, short rIdx, int minPos, int maxPos)
{
  if (aop->type != AOP_REG)
    return false;

  if (!isRegIdxPair (&rIdx))
    return aop->regs[rIdx] >= minPos && aop->regs[rIdx] < maxPos;

  return (aop->regs[rIdx] >= minPos && aop->regs[rIdx] < maxPos) ||
         (aop->regs[rIdx+1] >= minPos && aop->regs[rIdx+1] < maxPos);
}

/*-----------------------------------------------------------------*/
/* aopRS - asmop in register or on stack.                          */
/*-----------------------------------------------------------------*/
static bool
aopRS (const asmop *aop)
{
  return (aop->type == AOP_REG || aop->type == AOP_STK || aop->type == AOP_EXSTK);
}

/*-----------------------------------------------------------------*/
/* aopIsLitVal - asmop from offset is val.                         */
/* False negatives are possible.                                   */
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
      if (aop->size <= offset && !b && aop->type != AOP_LIT)
        continue;

      // Information from generalized constant propagation analysis
      if (!aop->valinfo.anything && offset < 8 &&
        ((aop->valinfo.knownbitsmask >> (offset * 8)) & 0xff) == 0xff &&
        ((aop->valinfo.knownbits >> (offset * 8)) & 0xff) == b)
        continue;

      if (aop->type != AOP_LIT)
        return (false);

      if (byteOfVal (aop->aopu.aop_lit, offset) != b)
        return (false);
    }

  return (true);
}

/*-----------------------------------------------------------------*/
/* aopIsLitBit - asmop from offset is val.                         */
/* False negatives are possible.                                   */
/*-----------------------------------------------------------------*/
static bool
aopIsLitBit (const asmop *aop, int boffset, bool val)
{
  if (!aop->valinfo.anything && boffset < 64 &&
    ((aop->valinfo.knownbitsmask >> boffset) & 1) &&
    ((aop->valinfo.knownbits >> boffset) & 1) == val)
    return(true);

  return (false);
}

/*-----------------------------------------------------------------*/
/* aopIsNotLitVal - asmop from offset is not val.                  */
/* False negatives are possible.                                   */
/* Note that both aopIsLitVal and aopIsNotLitVal can be false for  */
/* same arguments: we might just not have enough information.      */
/*-----------------------------------------------------------------*/
static bool
aopIsNotLitVal (const asmop *aop, int offset, int size, unsigned long long int val)
{
  wassert (size <= sizeof (unsigned long long int)); // Make sure we are not testing outside of argument val.

  for(; size; size--, offset++)
    {
      unsigned char b = val & 0xff;
      val >>= 8;

      // Leading zeroes
      if (aop->size <= offset && b)
        return (true);

      // Information from generalized constant propagation analysis
      if (!aop->valinfo.anything && offset < 8)
        {
          unsigned char knownbitsmask = aop->valinfo.knownbitsmask >> (offset * 8);
          unsigned char knownbits = aop->valinfo.knownbits >> (offset * 8);

          if ((knownbits & knownbitsmask) != (b & knownbitsmask))
            return (true);
          if (!offset && aop->valinfo.min > 0 && aop->valinfo.max <= 255 &&
            (aop->valinfo.min > b || aop->valinfo.min < b))
            return (true);
        }

      if (aop->type != AOP_LIT)
        continue;

      if (byteOfVal (aop->aopu.aop_lit, offset) != b)
        return (true);
    }

  return (false);
}

/*-----------------------------------------------------------------*/
/* aopInReg - asmop from offset in the register                    */
/*-----------------------------------------------------------------*/
static inline bool
aopInReg (const asmop *aop, int offset, short rIdx)
{
  if (offset >= aop->size || offset < 0)
    return (false);

  return aopRegOffset (aop, rIdx) == offset;
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

static inline int
fpOffset (int aop_stk)
{
  return aop_stk + (aop_stk > 0 ? _G.stack.param_offset : 0);
}

static int
spOffset (int aop_stk)
{
  return fpOffset (aop_stk) + _G.stack.pushed + _G.stack.offset;
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

static bool
isRegDead (short rIdx, const iCode * ic)
{
  if (!isRegIdxPair (&rIdx))
    return !bitVectBitValue (ic->rSurv, rIdx);
  return !bitVectBitValue (ic->rSurv, rIdx) && !bitVectBitValue (ic->rSurv, rIdx+1);
}

static PAIR_ID
_getTempPairId (void)
{
  if (IS_SM83)
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
  switch (id)
    {
    case PAIR_DE:
      return isRegDead (D_IDX, ic) && isRegDead (E_IDX, ic);
    case PAIR_BC:
      return isRegDead (B_IDX, ic) && isRegDead (C_IDX, ic);
    case PAIR_HL:
      return isRegDead (H_IDX, ic) && isRegDead (L_IDX, ic);
    case PAIR_IY:
      return isRegDead (IYH_IDX, ic) && isRegDead (IYL_IDX, ic);
    default:
      wassertl (0, "Only implemented for DE, BC, HL and IY");
      return FALSE;
    }
}

static PAIR_ID
getDeadPairId (const iCode *ic)
{
  if (isPairDead (PAIR_BC, ic))
    {
      return PAIR_BC;
    }
  else if (!IS_SM83 && isPairDead (PAIR_DE, ic))
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
  else if (!IS_SM83 && !isPairInUse (PAIR_DE, ic))
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
  if (offset >= 0 && offset + 2 <= aop->size)
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
  emit2 ("%s !equ !here", debugSym);
  emit2 ("!global", debugSym);
  genLine.lineElement.isDebug = 0;
}

// Todo: Handle IY correctly.
static unsigned char
ld_cost (const asmop *op1, int offset1, const asmop *op2, int offset2, bool count)
{
  AOP_TYPE op1type = op1->type;
  AOP_TYPE op2type = op2->type;

  if (offset2 >= op2->size)
    return (ld_cost (op1, offset1, ASMOP_ZERO, 0, count));

  /* Costs are symmetric */
  if (op1type != AOP_REG && (op2type == AOP_REG || op2type == AOP_DUMMY))
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
          // ld r, r is dangerous, since support for it is inconsistent even among otherwise binary-compatible Rabbit devices.
          if (IS_RAB && op1->aopu.aop_reg[offset1]->rIdx == op2->aopu.aop_reg[offset2]->rIdx)
            werror (W_INTERNAL_ERROR, __FILE__, __LINE__, "ld r, r considered");
          // eZ80 ld r, ir / ld ir, r / ld ir, ir
          if (op1->aopu.aop_reg[offset1]->rIdx == IYL_IDX || op1->aopu.aop_reg[offset1]->rIdx == IYH_IDX ||
            op2->aopu.aop_reg[offset2]->rIdx == IYL_IDX || op2->aopu.aop_reg[offset2]->rIdx == IYH_IDX)
            {
              if (count)
                cost2 (2, 8, 0, 0, 0, 0, 2, 2);
              return (2);
            }
        case AOP_DUMMY:
          if (IS_TLCS90 && (op1->aopu.aop_reg[offset1]->rIdx == A_IDX || op1type == AOP_DUMMY))
            {
              if (count)
                cost (1, 2);
              return (1);
            }
          else
            {
              if (count)
                cost2 (1 + IS_TLCS90, 4, 4, 2, 4, 4, 1, 1);
              return (1 + IS_TLCS90);
            }
        case AOP_IMMD:
        case AOP_LIT:
          if (op1->aopu.aop_reg[offset1]->rIdx == IYL_IDX || op1->aopu.aop_reg[offset1]->rIdx == IYH_IDX)
            {
              if (count)
                cost2 (3, 11, 0, 0, 0, 0, 2, 3); // ld ir, #n
              return (3);
            }
          else
            {
              if (count)
                cost2 (2, 7, 6, 4, 8, 4, 2, 2); // ld r, #n
              return (2);
            }
        case AOP_SFR:
          if (count)
            {
              cost2 (2, 11, 9, 0, 0, 0, 3, 3); // in a, (n)
              if (!aopInReg (op1, 0, A_IDX) && op1type != AOP_DUMMY)
                cost2 (1, 4, 4, 2, 4, 2, 1, 1); // ld r, a
            }
          return ((aopInReg (op1, 0, A_IDX) || op1type == AOP_DUMMY) ? 2 : 3);
        case AOP_STK:
          if (count)
            cost2 (3, 19, 14, 9, 0, 10, 4, 5); // ld r, d(ix)
          return (3);
        case AOP_HL:
          if (count)
            {
              cost2 (3, 10, 9, 6, 12, 6, 3, 3); // ld hl, #nn
              cost2 (1, 7, 6, 5, 8, 6, 2, 2); // ld r, (hl)
            }
          return (4);
        case AOP_EXSTK: // Approximation. Don't really know if this is really exstk at this point, anyway.
        case AOP_IY:
          if (count)
            {
              cost2 (4, 14, 12, 8, 0, 6, 4, 4); // ld iy, #nn
              cost2 (3, 19, 14, 9, 0, 10, 4, 5); // ld r, d(iy)
            }
          return (7);
        case AOP_PAIRPTR:
          if (op2->aopu.aop_pairId == PAIR_HL)
            {
              if (count)
                cost2 (1, 7, 6, 5, 8, 6, 2, 2); // ld r, (hl)
              return (1);
            }
          if (op2->aopu.aop_pairId == PAIR_IY || op2->aopu.aop_pairId == PAIR_IX)
            {
              if (count)
                cost2 (3, 19, 14, 9, 0, 10, 4, 5); // ld r, d(ix)
              return (3);
            }
          if (op2->aopu.aop_pairId == PAIR_BC || op2->aopu.aop_pairId == PAIR_DE)
            {
              if (count)
                {
                  cost2 (1, 7, 6, 6, 8, 6, 2, 2); // ld a, (rr)
                  if (!aopInReg (op1, 0, A_IDX))
                    cost2 (1, 4, 4, 2, 4, 2, 1, 1); // ld r, a
                }
              return ((aopInReg (op1, 0, A_IDX) || op1type == AOP_DUMMY) ? 1 : 2);
            }
        default:
          fprintf (stderr, "ld_cost op1: AOP_REG, op2: %d\n", (int) (op2type));
          wassert (0);
        }
    case AOP_SFR:
      if (count)
        cost2 (2, 11, 10, 0, 0, 0, 3, 3); // out (n), a
      if (aopInReg (op1, 0, A_IDX))
        return (2);
      else
        return (2 + ld_cost (ASMOP_A, 0, op2, offset2, count));
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
          if (count)
            cost2 (4, 19, 15, 11, 0, 12, 5, 5); // ld d(ix), n
          return (4);
        case AOP_SFR:          /* 2 from in a, (...) */
          if (count)
            {
              cost2 (2, 11, 9, 0, 0, 0, 3, 3); // in a, (n)
              cost2 (3, 19, 15, 10, 0, 10, 4, 5); // ld d(ix), a
            }
          return (5);
        case AOP_STK:
          if (count)
            {
              cost2 (3, 19, 14, 9, 0, 10, 4, 5); // ld a, d(ix)
              cost2 (3, 19, 15, 10, 0, 10, 4, 5); // ld d(ix), a
            }
          return (6);
        case AOP_HL:
          if (count)
            {
              cost2 (3, 10, 9, 6, 12, 6, 3, 3); // ld hl, nn
              cost2 (1, 7, 6, 6, 8, 6, 2, 2); // ld a, (hl)
              cost2 (3, 19, 15, 10, 0, 10, 4, 5); // ld d(ix), a
            }
          return (7);
        case AOP_EXSTK:
        case AOP_IY:
          if (count)
            {
              cost2 (4, 14, 12, 8, 0, 6, 4, 4); // ld iy, #nn
              cost2 (3, 19, 14, 9, 0, 10, 4, 5); // ld a, d(iy)
              cost2 (3, 19, 15, 10, 0, 10, 4, 5); // ld d(ix), a
            }
          return (10);
        case AOP_PAIRPTR:
          if (count)
            cost2 (3, 19, 15, 10, 0, 10, 4, 5); // ld d(ix), a
          return (3 + ld_cost (ASMOP_A, 0, op2, offset2, count));
        default:
          printf ("ld_cost op1: AOP_STK, op2: %d\n", (int) (op2type));
          wassert (0);
        }
    case AOP_HL:
      if (count)
        cost2 (3, 10, 9, 6, 12, 6, 3, 3); // ld hl, #nn
      switch (op2type)
        {
        case AOP_REG:
        case AOP_DUMMY:
          if (count)
            cost2 (1, 7, 7, 6, 8, 6, 2, 2); // ld (hl), r
          return (4);
        case AOP_IMMD:
        case AOP_LIT:
          if (count)
            cost2 (2 + IS_TLCS90, 10, 9, 7, 12, 8, 3, 3); // ld (hl), n
          return (5 + IS_TLCS90);
        case AOP_STK:
          if (count)
            {
              cost2 (3, 19, 14, 9, 0, 10, 4, 5); // ld a, d(ix)
              cost2 (1, 7, 7, 6, 8, 6, 2, 2); // ld (hl), a
            }
          return (7);
        case AOP_SFR:
          if (count)
            {
              cost2 (2, 11, 9, 0, 0, 0, 3, 3); // in a, (n)
              cost2 (1, 7, 7, 6, 8, 6, 2, 2); // ld (hl), a
            }
          return (6);
        case AOP_HL:
          if (count)
            {
              cost2 (3, 10, 9, 6, 12, 6, 3, 3); // ld hl, #nn
              cost2 (1, 7, 6, 5, 8, 6, 2, 2); // ld a, (hl)
              cost2 (1, 7, 7, 6, 8, 6, 2, 2); // ld (hl), a
            }
          return (8);
        case AOP_EXSTK:
        case AOP_IY:
          if (count)
            {
              cost2 (4, 14, 12, 8, 0, 6, 4, 4); // ld iy, #nn
              cost2 (3, 19, 14, 9, 0, 10, 4, 5); // ld a, d(iy)
              cost2 (1, 7, 7, 6, 8, 6, 2, 2); // ld (hl), a
            }
          return (11);
        default:
          printf ("ld_cost op1: AOP_HL, op2: %d", (int) (op2type));
          wassert (0);
        }
    case AOP_LIT:
    case AOP_IMMD:
      wassertl (0, "Trying to assign a value to a literal");
      break;
    case AOP_PAIRPTR:
      switch (op2type)
        {
        case AOP_REG:
          if (op1->aopu.aop_pairId == PAIR_HL)
            {
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              return (1);
            }
          else if (op1->aopu.aop_pairId == PAIR_IY || op1->aopu.aop_pairId == PAIR_IX)
            {
               cost2 (3, 19, 15, 9, 0, 10, 4, 5);
               return (3);
            }
          else
            wassert (0);
          break;
        case AOP_LIT:
          if (op1->aopu.aop_pairId == PAIR_HL)
            {
              cost2 (2, 10, 9, 7, 12, 8, 3, 3);
              return (2);
            }
          else if (op1->aopu.aop_pairId == PAIR_IY || op1->aopu.aop_pairId == PAIR_IX)
            {
               cost2 (4, 19, 15, 11, 0, 12, 5, 5);
               return (4);
            }
          else
            wassert (0);
          break;
        default:
          wassert (0);
        }
      break;
    default:
      printf ("ld_cost op1: %d\n", (int) (op1type));
      wassert (0);
    }
  return (12);                   // Fallback
}

static void
op8_cost (const asmop *op, int offset)
{
  switch (op->type)
    {
    case AOP_REG:
      if (op->aopu.aop_reg[offset]->rIdx == IYL_IDX || op->aopu.aop_reg[offset]->rIdx == IYH_IDX) // eZ80
        {
          wassert (HAS_IYL_INST);
          cost (2, 2);
          return;
        }
    case AOP_DUMMY:
      cost2 (1, 4, 4, 2, 4, 4, 1, 1);
      return;
    case AOP_IMMD:
    case AOP_LIT:
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      return;
    case AOP_STK:
      if (!IS_SM83)
        {
          cost2 (3, 19, 15, 9, 0, 10, 4, 5);
          return;
        }
      cost (1, 8); // add hl, sp
    case AOP_HL:
      cost2 (3 + 1, 10 + 7, 9 + 6, 6 + 5, 12 + 8, 6 + 6, 3 + 2, 3 + 2);
      return;
    case AOP_IY:               /* 4 from ld iy, #... */
    case AOP_EXSTK:            /* 4 from ld iy, #... */
      cost2 (4 + 3, 12 + 19, 12 + 15, 8 + 9, 0 + 0, 6 + 10, 4 + 4, 4 + 5);
      return;
    case AOP_PAIRPTR:
      if (op->aopu.aop_pairId == PAIR_HL)
        cost2 (1, 7, 6, 5, 8, 6, 2, 2);
      else if (op->aopu.aop_pairId == PAIR_IY || op->aopu.aop_pairId == PAIR_IX)
        cost2 (3, 19, 15, 9, 0, 10, 4, 5);
      else
        wassert (0);
      return;
    default:
      printf ("op8_cost op: %d\n", (int) (op->type));
      wassert (0);
    }
}

static void
incdec_cost (const asmop *op, int offset)
{
  switch (op->type)
    {
    case AOP_REG:
      if (op->aopu.aop_reg[offset]->rIdx == IYL_IDX || op->aopu.aop_reg[offset]->rIdx == IYH_IDX) // eZ80, r800
        {
          wassert (HAS_IYL_INST);
          cost (2, 2);
          return;
        }
    case AOP_DUMMY:
      cost2 (1, 4, 4, 2, 4, 2, 2, 1);
      return;
    case AOP_STK:
      if (!IS_SM83)
        {
          cost2 (3, 23, 18, 12, 0, 12, 6, 7);
          return;
        }
      cost (1, 8); // add hl, sp
    case AOP_HL:
      cost2 (3 + 1, 10 + 11, 9 + 10, 6 + 8, 12 + 12, 6 + 8, 3 + 5, 3 + 4);
      return;
    case AOP_IY:               /* 4 from ld iy, #... */
    case AOP_EXSTK:            /* 4 from ld iy, #... */
      cost2 (4 + 3, 14 + 23, 12 + 18, 8 + 12, 0 + 0, 6 + 12, 4 + 6, 4 + 7);
      return;
    case AOP_PAIRPTR:
      if (op->aopu.aop_pairId == PAIR_HL)
        {
          cost2 (1, 11, 10, 8, 12, 8, 5, 4);
          return;
        }
      if (op->aopu.aop_pairId == PAIR_IY || op->aopu.aop_pairId == PAIR_IX)
        {
          cost2 (3, 23, 18, 12, 0, 12, 6, 7);
          return;
        }
    default:
      printf ("op8_cost op: %d\n", (int) (op->type));
      wassert (0);
    }
}

static void
bit8_cost (const asmop *op)
{
  switch (op->type)
    {
    case AOP_REG:
    case AOP_DUMMY:
      cost2 (2, 8, 7, 4, 8, 4, 2, 2);
      return;
    case AOP_STK:
      if (!IS_SM83)
        {
          cost2 (4, 23, 19, 13, 0, 12, 7, 7);
          return;
        }
      cost (1, 8); // add hl, sp
    case AOP_HL:               /* 3 from ld hl, #... */
      cost2 (3 + 2, 10 + 15, 9 + 13, 6 + 10, 12 + 16, 6 + 8, 3 + 5, 3 + 5);
      return;
    case AOP_IY:               /* 4 from ld iy, #... */
    case AOP_EXSTK:            /* 4 from ld iy, #... */
      cost2 (4 + 4, 14 + 23, 12 + 19, 8 + 13, 0 + 0, 6 + 12, 4 + 7, 4 + 7);
      return;
    default:
      printf ("bit8_cost op: %d\n", (int) (op->type));
      wassert (0);
    }
}

static void
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
      cost2 (1, 4, 3, 2, 4, 2, 1, 1);
      return;
    case A_NEG:
      cost2 (2, 8, 6, 4, 0, 2, 2, 2);
      return;
    case A_RLD:
    case A_RRD:
      cost2 (2, 18, 16, 0, 0, 12, 5, 5);
      return;
    case A_LD:
      ld_cost (op1, offset1, op2, offset2, true);
      return;
    case A_ADD:
    case A_ADC:
    case A_AND:
    case A_CP:
    case A_OR:
    case A_SBC:
    case A_SUB:
    case A_XOR:
      op8_cost (op2, offset2);
      return;
    case A_DEC:
    case A_INC:
      incdec_cost (op1, offset1);
      return;
    case A_RL:
    case A_RLC:
    case A_RR:
    case A_RRC:
    case A_SLA:
    case A_SRA:
    case A_SRL:
    case A_SWAP:
      bit8_cost (op1);
      return;
    default:
      wassertl (0, "Tried get cost for unknown instruction");
    }
}

static void
emit3wCost (enum asminst inst, const asmop *op1, int offset1, const asmop *op2, int offset2)
{
  if (op2 && offset2 >= op2->size)
    op2 = ASMOP_ZERO;

  switch (inst)
    {
    case A_INC:
    case A_DEC:
      if (aopInReg (op1, offset1, IY_IDX))
        cost2 (2 - IS_TLCS90, 10, 7, 4, 0, 4, 2, 2);
      else
        cost2 (1, 6, 4, 2, 8, 4, 1, 1);
      return;
    case A_ADD:
      if (aopInReg (op1, offset1, IY_IDX))
        cost2 (2, 15, 10, 4, 0, 8, 2, 2);
      else
        cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
      return;
    case A_ADC:
    case A_SBC:
      cost2 (2, 15, 10, 4, 0, 8, 2, 2);
      return;
    case A_EX:
      cost2 (1, 4, 3, 2, 0, 2, 1, 1);
      return;
    default:
      wassertl (0, "Tried get cost for unknown instruction");
    }
}

static void
emit3_o (enum asminst inst, asmop *op1, int offset1, asmop *op2, int offset2)
{
  unsigned long cost, bytecost;
  float statecost;

  emit3Cost (inst, op1, offset1, op2, offset2);

  if (regalloc_dry_run)
    return;

  cost = regalloc_dry_run_cost;
  bytecost = regalloc_dry_run_cost_bytes;
  statecost = regalloc_dry_run_cost_states;
  if (!op1)
    emit2 ("%s", asminstnames[inst]);
  else if (!op2)
    emit2 ("%s %s", asminstnames[inst], aopGet (op1, offset1, FALSE));
  else
    {
      char *l = Safe_strdup (aopGet (op1, offset1, FALSE));
      emit2 ("%s %s, %s", asminstnames[inst], l, aopGet (op2, offset2, FALSE));
      Safe_free (l);
    }

  regalloc_dry_run_cost = cost;
  regalloc_dry_run_cost_bytes = bytecost;
  regalloc_dry_run_cost_states = statecost;
  //emitDebug(";emit3_o cost: %d total so far: %d", (int)emit3Cost(inst, op1, offset1, op2, offset2), (int)cost);
}

static void
emit3w_o (enum asminst inst, asmop *op1, int offset1, asmop *op2, int offset2)
{
  unsigned int cost, bytecost;
  float statecost;

  emit3wCost (inst, op1, offset1, op2, offset2);

  if (regalloc_dry_run)
    return;

  cost = regalloc_dry_run_cost;
  bytecost = regalloc_dry_run_cost_bytes;
  statecost = regalloc_dry_run_cost_states;
  if (!op1)
    emit2 ("%s", asminstnames[inst]);
  else if (!op2)
    emit2 ("%s %s", asminstnames[inst], aopGet (op1, offset1, true));
  else
    {
      char *l = Safe_strdup (aopGet (op1, offset1, true));
      emit2 ("%s %s, %s", asminstnames[inst], l, aopGet (op2, offset2, true));
      Safe_free (l);
    }

  regalloc_dry_run_cost = cost;
  regalloc_dry_run_cost_bytes = bytecost;
  regalloc_dry_run_cost_states = statecost;
  //emitDebug(";emit3_o cost: %d total so far: %d", (int)emit3Cost(inst, op1, offset1, op2, offset2), (int)cost);
}

static void
emit3 (enum asminst inst, asmop *op1, asmop *op2)
{
  emit3_o (inst, op1, 0, op2, 0);
}

static void
emit3w (enum asminst inst, asmop *op1, asmop *op2)
{
  emit3w_o (inst, op1, 0, op2, 0);
}

static void
_emitMove (const char *to, const char *from)
{
  if (STRCASECMP (to, from) != 0)
    {
      emit2 ("ld %s, %s", to, from);
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

/* swap pairs fiels type/base */
static void
swapPairs (PAIR_ID pair1Id, PAIR_ID pair2Id)
{
  /*AOP_TYPE tt = _G.pairs[pair1Id].last_type;
  _G.pairs[pair1Id].last_type = _G.pairs[pair2Id].last_type;
  _G.pairs[pair2Id].last_type = tt;
  const char *tb = _G.pairs[pair1Id].base;
  unsigned int tv = _G.pairs[pair1Id].value;
  _G.pairs[pair1Id].base = _G.pairs[pair2Id].base;
  _G.pairs[pair2Id].base = tb;
  _G.pairs[pair1Id].value = _G.pairs[pair2Id].value;
  _G.pairs[pair2Id].value = tv;*/
  
  // For now just spill both: Making this work would require proper tracking of de (i.e. adding spillPair (PAIR_DE) as consistently as has been done for spillPair (PAIR_HL)
  spillPair (pair1Id);
  spillPair (pair2Id);
}

static void
_push (PAIR_ID pairId)
{
  emit2 ("push %s", _pairs[pairId].name);
  if (pairId == PAIR_IX || pairId == PAIR_IY)
  	cost2 (2 - IS_TLCS90, 15, 14, 12, 16, 8, 4, 5);
  else
  	cost2 (1, 11, 11, 10, 16, 8, 3, 4);
  _G.stack.pushed += 2;
}

static void
_pop (PAIR_ID pairId)
{
  if (pairId != PAIR_INVALID)
    {
      emit2 ("pop %s", _pairs[pairId].name);
      if (pairId == PAIR_IX || pairId == PAIR_IY)
  	    cost2 (2 - IS_TLCS90, 14, 12, 9, 12, 10, 4, 4);
      else
  	    cost2 (1, 10, 9, 7, 12, 10, 3, 3);
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
          cost2 (1, 4, 4, 2, 4, 4, 1, 1);
          emit2 ("ld %s, %s", _pairs[dstPair].h, _pairs[srcPair].h);
          cost2 (1, 4, 4, 2, 4, 4, 1, 1);
        }
      break;
    default:
      wassertl (0, "Tried to move a nonphysical pair");
    }
  _G.pairs[dstPair].last_type = _G.pairs[srcPair].last_type;
  _G.pairs[dstPair].base = _G.pairs[srcPair].base;
  _G.pairs[dstPair].value = _G.pairs[srcPair].value;
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
  aop->valinfo.anything = true;
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
    return sym->aop;

  /* Assign depending on the storage class */
  if (sym->onStack || sym->iaccess)
    {
      /* The pointer that is used depends on how big the offset is.
         Normally everything is AOP_STK, but for offsets of < -128 or
         > 127 on the Z80 an extended stack pointer is used.
       */
      if (!IS_SM83 && (_G.omitFramePtr || sym->stack < INT8MIN || sym->stack > (int) (INT8MAX - getSize (sym->type))))
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

      memset (aop->regs, -1, sizeof(aop->regs));
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
      if (IS_SM83)
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
  if (IS_SM83 || IY_RESERVED)
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
  int val = 0;
  asmop *aop;
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

  if (OP_SYMBOL (IC_LEFT (ic))->onStack)
    {
      aop = newAsmop (AOP_STL);
      aop->aopu.aop_stk = (long)(OP_SYMBOL (IC_LEFT (ic))->stack) + val;
    }
  else
    {
      aop = newAsmop (AOP_IMMD);

      dbuf_init (&dbuf, 128);
      if (val)
        {
          dbuf_tprintf (&dbuf, "(%s %c %d)", OP_SYMBOL (IC_LEFT (ic))->rname, val >= 0 ? '+' : '-', abs (val) & 0xffff);
        }
      else
        {
          dbuf_append_str (&dbuf, OP_SYMBOL (IC_LEFT (ic))->rname);
        }
      aop->aopu.aop_immd = traceAlloc (&_G.trace.aops, dbuf_detach_c_str (&dbuf));
    }

  return aop;
}

#if 0 // No longer used?
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
#endif

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
sameRegs (const asmop *aop1, const asmop *aop2)
{
  int i;

  if (aop1->type == AOP_SFR || aop2->type == AOP_SFR)
    return FALSE;

  if (aop1 == aop2)
    return TRUE;

  if (!regalloc_dry_run && // Todo: Check if always enabling this even for dry runs tends to result in better code.
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
/* aopSame - two asmops refer to the same storage                  */
/*-----------------------------------------------------------------*/
static bool
aopSame (const asmop *aop1, int offset1, const asmop *aop2, int offset2, int size)
{
  if (aop1 == aop2 && offset1 == offset2)
    return (true);

  for(; size; size--, offset1++, offset2++)
    {
      if (offset1 >= aop1->size || offset2 >= aop2->size)
        return (false);

      if (aop1->type == AOP_REG && aop2->type == AOP_REG && // Same register
        aop1->aopu.aop_reg[offset1]->rIdx == aop2->aopu.aop_reg[offset2]->rIdx)
        continue;

      if (aopOnStack (aop1, offset1, 1) && aopOnStack (aop2, offset2, 1) && !regalloc_dry_run && // Same stack location - stack locations might change after register allocation, so make no assumption during dry run.
        aop1->aopu.aop_stk + offset1 == aop2->aopu.aop_stk + offset2)
        continue;

      if (aop1->type == AOP_LIT && aop2->type == AOP_LIT && // Same literal
        byteOfVal (aop1->aopu.aop_lit, offset1) == byteOfVal (aop2->aopu.aop_lit, offset2))
        continue;

      // Same file-scope variable.
      if ((aop1->type == AOP_DIR || aop1->type == AOP_HL || aop1->type == AOP_IY) &&
        (aop2->type == AOP_DIR || aop2->type == AOP_HL || aop2->type == AOP_IY) &&
        offset1 == offset2 && !strcmp(aop1->aopu.aop_dir, aop2->aopu.aop_dir))
        return (true);
  
      return (false);
    }

  return (true);
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
      if (!result)
        op->aop->valinfo = getOperandValinfo (ic, op);
      else if(ic->resultvalinfo)
        op->aop->valinfo = *ic->resultvalinfo;
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
      if (result && ic->resultvalinfo)
        valinfo_union (&(op->aop->valinfo), *ic->resultvalinfo);
      else if (result)
        op->aop->valinfo.anything = true;
      return;
    }

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aopForSym (ic, OP_SYMBOL (op), requires_a);
      if (!result)
        op->aop->valinfo = getOperandValinfo (ic, op);
      else if(ic->resultvalinfo)
        op->aop->valinfo = *ic->resultvalinfo;
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
      wassert (!sym->ruonly); // iTemp optimized out via ifxForOp shouldn'T reach here.

      wassert (!sym->accuse); // Should not happen anymore with curetn register allocator.

      /* rematerialize it NOW */
      if (sym->remat)
        {
          sym->aop = op->aop = aop = aopForRemat (sym);
          aop->size = getSize (sym->type);
          if (!result)
            aop->valinfo = getOperandValinfo (ic, op);
          else if(ic->resultvalinfo)
            aop->valinfo = *ic->resultvalinfo;
          return;
        }

      /* On-stack for dry run. */
      if (sym->nRegs && regalloc_dry_run)
        {
          sym->aop = op->aop = aop = newAsmop (_G.omitFramePtr ? AOP_EXSTK : AOP_STK);
          aop->size = getSize (sym->type);
          if (!result)
            aop->valinfo = getOperandValinfo (ic, op);
          else if(ic->resultvalinfo)
            aop->valinfo = *ic->resultvalinfo;
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
          if (!result)
            aop->valinfo = getOperandValinfo (ic, op);
          else if(ic->resultvalinfo)
            aop->valinfo = *ic->resultvalinfo;
          return;
        }

      /* else must be a dummy iTemp */
      sym->aop = op->aop = aop = newAsmop (AOP_DUMMY);
      aop->size = getSize (sym->type);
      if (!result)
        aop->valinfo = getOperandValinfo (ic, op);
      else if(ic->resultvalinfo)
        aop->valinfo = *ic->resultvalinfo;
      return;
    }

  /* must be in a register */
  sym->aop = op->aop = aop = newAsmop (AOP_REG);
  aop->size = sym->nRegs;
  if (!result)
    aop->valinfo = getOperandValinfo (ic, op);
  else if(ic->resultvalinfo)
    aop->valinfo = *ic->resultvalinfo;
  memset (aop->regs, -1, sizeof(aop->regs));
  for (i = 0; i < sym->nRegs; i++)
    {
      wassertl (sym->regs[i], "Symbol in register, but no register assigned.");
      if(!sym->regs[i])
        fprintf(stderr, "Symbol %s at ic %d.\n", sym->name, ic->key);
      aop->aopu.aop_reg[i] = sym->regs[i];
      aop->regs[sym->regs[i]->rIdx] = i;
    }
}

// Get asmop for registers containing the return type of function
// Returns 0 if the function does not have a return value or it is not returned in registers.
static asmop *
aopRet (sym_link *ftype)
{
  wassert (IS_FUNC (ftype));

  // Adjust returnregs in isReturned in peep.c accordingly when changing asmop_return here.

  int size = getSize (ftype->next);

  const bool bigreturn = (size > 4) || IS_STRUCT (ftype->next);
  if (bigreturn)
    return (0);

  if (FUNC_SDCCCALL (ftype) == 0 || FUNC_ISSMALLC (ftype) || FUNC_ISZ88DK_FASTCALL (ftype))
    switch (size)
      {
      case 1:
        return (IS_SM83 ? ASMOP_E : ASMOP_L);
      case 2:
        return (IS_SM83 ? ASMOP_DE : ASMOP_HL);
      case 3:
      case 4:
        return (IS_SM83 ? ASMOP_HLDE : ASMOP_DEHL);   
      default:
        return 0;
      }

  wassert (FUNC_SDCCCALL (ftype) == 1);

  switch (size)
    {
    case 1:
      return (ASMOP_A);
    case 2:
      if (IS_RAB || IS_TLCS90 || IS_EZ80_Z80)
        return ASMOP_HL;
      else if (IS_SM83)
        return ASMOP_BC;
      else
        return ASMOP_DE;
    case 3:
    case 4:
      return (IS_SM83 ? ASMOP_DEBC : ASMOP_HLDE);   
    default:
      return 0;
    }
}

// Get asmop for registers containing a parameter
// Returns 0 if the parameter is passed on the stack
static asmop *
aopArg (sym_link *ftype, int i)
{
  wassert (IS_FUNC (ftype));

  if (IFFUNC_HASVARARGS (ftype))
    return 0;

  value *args = FUNC_ARGS(ftype);
  wassert (args);

  if (FUNC_ISZ88DK_FASTCALL (ftype))
    {
      if (i != 1 || IS_STRUCT (args->type))
        return 0;

      switch (getSize (args->type))
        {
        case 1:
          return ASMOP_L;
        case 2:
          return ASMOP_HL;
        case 4:
          return ASMOP_DEHL;
        default:
          return 0;
        }
    }
    
  // Old SDCC calling convention: Pass everything on the stack.
  if (FUNC_SDCCCALL (ftype) == 0 || FUNC_ISSMALLC (ftype) || IFFUNC_ISBANKEDCALL (ftype))
    return 0;

  wassert (FUNC_SDCCCALL (ftype) == 1);

  if (!FUNC_HASVARARGS (ftype))
    {
      int j;
      value *arg;

      for (j = 1, arg = args; j < i; j++, arg = arg->next)
        wassert (arg);

      if (IS_STRUCT (arg->type))
        return 0;

      if (i == 1 && getSize (arg->type) == 1)
        return ASMOP_A;
      if (i == 1 && getSize (arg->type) == 2)
        return (IS_SM83 ? ASMOP_DE : ASMOP_HL);
      if (i == 1 && getSize (arg->type) == 4)
        return (IS_SM83 ? ASMOP_DEBC : ASMOP_HLDE);

      if (IS_SM83 && i == 2 && aopArg (ftype, 1) == ASMOP_A && getSize (arg->type) == 1)
        return ASMOP_E;
      if (IS_SM83 && i == 2 && aopArg (ftype, 1) == ASMOP_A && getSize (arg->type) == 2)
        return ASMOP_DE;
  
      if (IS_SM83 && i == 2 && aopArg (ftype, 1) == ASMOP_DE && getSize (arg->type) == 1)
        return ASMOP_A;
      if (IS_SM83 && i == 2 && aopArg (ftype, 1) == ASMOP_DE && getSize (arg->type) == 2)
        return ASMOP_BC;

      if (!IS_SM83 && i == 2 && aopArg (ftype, 1) == ASMOP_A && getSize (arg->type) == 1)
        return ASMOP_L;

      if ((IS_Z80 || IS_Z180 || IS_Z80N || IS_R800) && i == 2 && aopArg (ftype, 1) == ASMOP_A && getSize (arg->type) == 2)
        return ASMOP_DE;
      if ((IS_Z80 || IS_Z180 || IS_Z80N || IS_R800) && i == 2 && aopArg (ftype, 1) == ASMOP_HL && getSize (arg->type) == 2)
        return ASMOP_DE;

      if ((IS_RAB || IS_TLCS90 || IS_EZ80_Z80) && i == 2 && aopArg (ftype, 1) == ASMOP_A && getSize (arg->type) == 2)
        return ASMOP_HL;
      if ((IS_RAB || IS_TLCS90 || IS_EZ80_Z80) && i == 2 && aopArg (ftype, 1) == ASMOP_HL && getSize (arg->type) == 1)
        return ASMOP_A;
      if ((IS_RAB || IS_TLCS90 || IS_EZ80_Z80) && i == 2 && aopArg (ftype, 1) == ASMOP_HLDE && getSize (arg->type) == 1)
        return ASMOP_A;

      return 0;
    }

  return 0;
}

// Return true, iff ftype cleans up stack parameters.
static bool
isFuncCalleeStackCleanup (sym_link *ftype)
{
  wassert (IS_FUNC (ftype));

  const bool farg = !FUNC_HASVARARGS (ftype) && FUNC_ARGS (ftype) && IS_FLOAT (FUNC_ARGS (ftype)->type); 
  const bool bigreturn = (getSize (ftype->next) > 4) || IS_STRUCT (ftype->next);
  int stackparmbytes = bigreturn * 2;
  for (value *arg = FUNC_ARGS(ftype); arg && !FUNC_HASVARARGS(ftype); arg = arg->next)
    {
      int argsize = getSize (arg->type);
      if (argsize == 1 && FUNC_ISSMALLC (ftype)) // SmallC calling convention passes 8-bit stack arguments as 16 bit.
        argsize++;
      if (!SPEC_REGPARM (arg->etype))
        stackparmbytes += argsize;
    }
  if (!stackparmbytes)
    return false;

  if (IFFUNC_ISZ88DK_CALLEE (ftype))
    return true;

  if (FUNC_SDCCCALL (ftype) == 0 || FUNC_ISSMALLC (ftype) || FUNC_ISZ88DK_FASTCALL (ftype))
    return false;

  if (IFFUNC_ISBANKEDCALL (ftype))
    return false;

  if (FUNC_HASVARARGS (ftype))
    return false;

  wassert (FUNC_SDCCCALL (ftype) == 1);

  // Callee cleans up stack for all non-vararg functions on sm83.
  if (IS_SM83)
    return true;

  // Callee cleans up stack if return value has at most 16 bits or the return value is float and there is a first argument of type float.
  if (!ftype->next || getSize (ftype->next) <= 2)
    return true;
  else if (IS_FLOAT (ftype->next) && farg)
    return true;
  return false;
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

  if (aop->type == AOP_PAIRPTR && !IS_SM83 && aop->aopu.aop_pairId == PAIR_DE)
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

          dbuf_tprintf (&dbuf, with_hash ? "!immedword" : "!constword", (unsigned) (v & 0xffffu));
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
    case AOP_STL:
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
requiresHL (const asmop *aop)
{
  switch (aop->type)
    {
    case AOP_IY:
      return FALSE;
    case AOP_HL:
    case AOP_EXSTK:
    case AOP_STL:
      return true;
    case AOP_STK:
      return (IS_SM83 || _G.omitFramePtr);
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

// Updated the internally cached value for a pair.
static void
updatePair (PAIR_ID pairId, int diff)
{
  if (_G.pairs[pairId].last_type == AOP_LIT)
    _G.pairs[pairId].value = (_G.pairs[pairId].value + (unsigned int)diff) & 0xffff;
  else if (_G.pairs[pairId].last_type == AOP_IMMD || _G.pairs[pairId].last_type == AOP_IY || _G.pairs[pairId].last_type == AOP_HL ||
    _G.pairs[pairId].last_type == AOP_STK || _G.pairs[pairId].last_type == AOP_EXSTK)
    _G.pairs[pairId].offset += diff;
}

// Return 0, if adjusting the old value in the pair is sufficient.
static int
fetchLitPair (PAIR_ID pairId, asmop *left, int offset, bool f_dead, bool dry)
{
  const char *pair = _pairs[pairId].name;
  char *l = Safe_strdup (aopGetLitWordLong (left, offset, FALSE));
  char *base_str = Safe_strdup (aopGetLitWordLong (left, 0, FALSE));

//  emitDebug (";fetchLitPair %s",  pair);

  wassert (pair);

  const char *base = base_str;

  // Make offset from aopGetLitWordLong explicit.
  if (strchr (base_str, '+') && base_str[0] == '(' && base_str[1] == '_' && strchr (base_str, ' '))
    {
      long xoffset = strtol(strchr (base_str, '+') + 1, 0, 0);
      if (abs(offset < 10000) && labs(xoffset) < 10000l)
        {
          *(strchr (base_str, ' ')) = 0;
          base++;
          offset += xoffset;
        }
    }

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
                      if (dry) // Just report matching base
                        return (0);
                      adjustPair (pair, &_G.pairs[pairId].offset, offset);
                      goto adjusted;
                    }
                  if (pairId == PAIR_IY && offset == _G.pairs[pairId].offset)
                    {
                      if (dry) // Just report matching base
                        return (0);
                      goto adjusted;
                    }
                }
            }
        }

      if(dry)
        return(-1);
 
      if (pairId == PAIR_HL && left->type == AOP_LIT && _G.pairs[pairId].last_type == AOP_LIT)
        {
          unsigned new_low, new_high, old_low, old_high;
          new_low = byteOfVal (left->aopu.aop_lit, offset);
          new_high = byteOfVal (left->aopu.aop_lit, offset + 1);
          old_low = (_G.pairs[pairId].value >> 0) & 0xff;
          old_high = (_G.pairs[pairId].value >> 8) & 0xff;

          if (new_low == old_low && new_high == old_high)
            goto adjusted;
          else if (IS_RAB && !new_high && (new_low == 1 && (old_high || old_low)) && f_dead)
            {
              emit2 ("bool hl");
              cost (1, 2);
              goto adjusted;
            }
          else if (new_high == old_high && new_low == old_high)
            {
              emit3_o (A_LD, ASMOP_L, 0, ASMOP_H, 0);
              goto adjusted;
            }
          else if (new_low == old_low && new_high == old_low)
            {
              emit3_o (A_LD, ASMOP_H, 0, ASMOP_L, 0);
              goto adjusted;
            }
          /* Change lower byte only. */
          else if (new_high == old_high)
            {
              emit3_o (A_LD, ASMOP_L, 0, left, offset);
              goto adjusted;
            }
          /* Change upper byte only. */
          else if (new_low == old_low)
            {
              emit3_o (A_LD, ASMOP_H, 0, left, offset + 1);
              goto adjusted;
            }
        }
    }

  if(dry)
    return(-1);

  /* Both a lit on the right and a true symbol on the left */
  if (IS_RAB && pairId == PAIR_HL && left->type == AOP_LIT && !byteOfVal (left->aopu.aop_lit, offset + 1) && !byteOfVal (left->aopu.aop_lit, offset) && f_dead)
  {
    emit2 ("bool hl");
    emit2 ("ld l, h");
    cost (1 + 1, 2 + 2);
  }
  else
  {
    emit2 ("ld %s, !hashedstr", pair, l);
    if (pairId == PAIR_IX || pairId == PAIR_IY)
      cost2 (4 - IS_TLCS90 , 14, 12, 8, 12, 6, 4, 4);
    else
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
  }

adjusted:
  _G.pairs[pairId].last_type = left->type;
  if (left->type == AOP_LIT)
    _G.pairs[pairId].value = byteOfVal (left->aopu.aop_lit, offset) + (byteOfVal (left->aopu.aop_lit, offset + 1) << 8);
  else
    _G.pairs[pairId].base = traceAlloc (&_G.trace.aops, Safe_strdup (base));
  _G.pairs[pairId].offset = offset;
  Safe_free (base_str);
  Safe_free (l);

  return(0);
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
      else if (!IS_SM83 && !bitVectBitValue (ic->rMask, D_IDX) && !bitVectBitValue (ic->rMask, E_IDX))
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

static void
genMove_o (asmop *result, int roffset, asmop *source, int soffset, int size, bool a_dead_global, bool hl_dead_global, bool de_dead_global, bool iy_dead_global, bool f_dead);

/* If ic != 0, we can safely use isPairDead(). */
/* By now, genMove / genMove_o is as good or better than this for nearly all uses. */
static void
fetchPairLong (PAIR_ID pairId, asmop *aop, const iCode *ic, int offset)
{
  emitDebug (";fetchPairLong");

  if (aop->type == AOP_STL && !offset)
    {
      if (IS_SM83 && pairId == PAIR_DE || pairId == PAIR_BC)
        {
          _push (PAIR_HL);
          emit2 ("ld hl, !immed%d", spOffset(aop->aopu.aop_stk));
          cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          emit2 ("add hl, sp");
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          emit2 ("ld %s, l", _pairs[pairId].l);
          cost2 (1, 4, 4, 2, 4, 4, 1, 1);
          emit2 ("ld %s, h", _pairs[pairId].h);
          cost2 (1, 4, 4, 2, 4, 4, 1, 1);
          _pop (PAIR_HL);
          return;
        }
      else if (pairId == PAIR_IY)
        {
          emit2 ("ld iy, !immed%d", spOffset(aop->aopu.aop_stk));
          emit2 ("add iy, sp");
          cost2 (6 - IS_TLCS90, 29, 22, 12, 0, 14, 6, 6);
          return;
        }

      if (pairId == PAIR_DE)
        emit3w (A_EX, ASMOP_DE, ASMOP_HL);
      emit2 ("ld hl, !immed%d", spOffset(aop->aopu.aop_stk));
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("add hl, sp");
      cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
      if (pairId == PAIR_DE)
        emit3w (A_EX, ASMOP_DE, ASMOP_HL);
      spillPair (pairId);
      return;
    }
  else if (aop->type == AOP_STL && offset >= 2)
    {
      fetchLitPair (pairId, ASMOP_ZERO, 0, true, false);
      return;
    }
  else if (aop->type == AOP_STL)
    {
      UNIMPLEMENTED;
      return;
    }

  /* if this is rematerializable */
  if (isLitWord (aop))
    fetchLitPair (pairId, aop, offset, true, false);
  else
    {
      if (getPairId_o (aop, offset) == pairId)
        {
          /* Do nothing */
        }
      else if (IS_EZ80_Z80 && aop->size - offset >= 2 && aop->type == AOP_STK)
        {
          int fp_offset = aop->aopu.aop_stk + offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
          emit2 ("ld %s, %d (ix)", _pairs[pairId].name, fp_offset);
          cost (3, 5);
        }
      /* Getting the parameter by a pop / push sequence is cheaper when we have a free pair (except for the Rabbit, which has an even cheaper sp-relative load).
         SM83 is nearly twice as fast doing it byte by byte, but that's a byte bigger.
         Stack allocation can change after register allocation, so assume this optimization is not possible for the allocator's cost function (unless the stack location is for a parameter). */
      else if (!IS_RAB && (!IS_SM83 || optimize.codeSize) && aop->size - offset >= 2 &&
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
      else if (!IS_SM83 && (aop->type == AOP_IY || aop->type == AOP_HL) &&
        (aop->size >= 2 || pairId != PAIR_IY && optimize.allow_unsafe_read))
        {
          /* Instead of fetching relative to IY, just grab directly
             from the address IY refers to */
          emit2 ("ld %s, !mems", _pairs[pairId].name, aopGetLitWordLong (aop, offset, FALSE));
          if (pairId == PAIR_HL)
            cost2 (3 + IS_TLCS90, 16, 15, 11, 0, 12, 5, 5);
          else
            cost2 (4, 20, 18, 13, 0, 12, 6, 6);

          if (aop->size < 2)
            {
              emit2 ("ld %s, !zero", _pairs[pairId].h);
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
            }
        }
      /* we need to get it byte by byte */
      else if (pairId == PAIR_HL && (IS_SM83 || IY_RESERVED) && (aop->type == AOP_HL || aop->type == AOP_EXSTK || IS_SM83 && aop->type == AOP_STK) && requiresHL (aop))
        {
          if (!regalloc_dry_run)        // TODO: Fix this to get correct cost!
            aopGet (aop, offset, FALSE);
          switch (aop->size - offset)
            {
            case 1:
              emit2 ("ld l, !*hl");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              emit2 ("ld h, !immedbyte", 0u);
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
              break;
            default:
              wassertl (aop->size - offset > 1, "Attempted to fetch no data into HL");
              if (IS_RAB)
                {
                  emit2 ("ld hl, 0 (hl)");
                  cost (3, 11);
                }
              else if (IS_EZ80_Z80 || IS_TLCS90)
                {
                  emit2 ("ld hl, !*hl");
                  cost2 (2, 0, 0, 0, 0, 8, 4, 0);
                }
              else
                {
                  if (ic && bitVectBitValue (ic->rMask, A_IDX))
                    _push (PAIR_AF);

                  emit2 ("!ldahli");
                  if (IS_SM83)
                    cost (1, 8); // ldi
                  else
                    {
                      cost2 (1, 7, 6, 5, 8, 6, 2, 2); // ld , a(hl)
                      cost2 (1, 6, 4, 2, 8, 4, 1, 1); // inc hl
                    }
                  emit2 ("ld h, !*hl");
                  cost2 (1, 7, 6, 5, 8, 6, 2, 2);
                  emit3 (A_LD, ASMOP_L, ASMOP_A);

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
              cost2 (3, 0, 0, 11, 0, 12, 0, 0);
            }
          else if (isPair (aop) && (IS_RAB || IS_TLCS90) && getPairId (aop) == PAIR_HL)
            {
              emit2 ("ld iy, hl");
              cost2 (1 + IS_RAB, 0, 0, 4, 0, 6, 0, 0);
            }
          else if (isPair (aop))
            {
              _push (getPairId (aop));
              _pop (PAIR_IY);
            }
          else
            {
              bool isUsed;
              PAIR_ID id = makeFreePairId (ic, &isUsed);
              if (isUsed)
                _push (id);
              /* Can't load into parts, so load into HL then exchange. */
              genMove_o (id == PAIR_HL ? ASMOP_HL : id == PAIR_DE ? ASMOP_DE : id == PAIR_BC ? ASMOP_BC : ASMOP_IY, 0, aop, offset, 2, false, false, false, false, false);

              if ((IS_RAB || IS_TLCS90) && id == PAIR_HL)
                {
                  emit2 ("ld iy, hl");
                  cost2 (1 + IS_RAB, 0, 0, 4, 0, 6, 0, 0);
                }
              else
                {
                  _push (id);
                  _pop (PAIR_IY);
                }
              if (isUsed)
                _pop (id);
            }
        }
      else if (isUnsplitable (aop))
        {
          _push (getPairId (aop));
          _pop (pairId);
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
              if (pairId == PAIR_DE && !(ic && isPairDead (PAIR_HL, ic)))
                emit3w (A_EX, ASMOP_DE, ASMOP_HL);
              if (abs (sp_offset) <= 127)
                emit2 ("ld %s, %d (sp)", pairId == PAIR_IY ? "iy" : "hl", sp_offset);   /* Fetch relative to stack pointer. */
              else
                emit2 ("ld hl, %d (ix)", fp_offset);    /* Fetch relative to frame pointer. */
              cost (pairId == PAIR_IY ? 3 : 2, pairId == PAIR_IY ? 11 : 9);
              if (pairId == PAIR_DE)
                emit3w (A_EX, ASMOP_DE, ASMOP_HL);
            }
          /* Operand resides (partially) in the pair */
          else if (!regalloc_dry_run && !strcmp (aopGet (aop, offset + 1, FALSE), _pairs[pairId].l))    // aopGet (aop, offset + 1, FALSE) is problematic: It prevents calculation of exact cost, and results in redundant code being generated. Todo: Exact cost
            {
              _moveA3 (aop, offset);
              if (!regalloc_dry_run)
                emit2 ("ld %s, %s", _pairs[pairId].h, aopGet (aop, offset + 1, FALSE));
              ld_cost (pairId == PAIR_HL ? ASMOP_H : pairId == PAIR_DE ? ASMOP_D : ASMOP_B, 0, aop, offset + 1, true);
              emit2 ("ld %s, a", _pairs[pairId].l);
              ld_cost (ASMOP_L, 0, ASMOP_A, 0, true);
            }
          /* The Rabbit's cast to bool is a cheap way of zeroing h (similar to xor a, a for a for the Z80). */
          else if (pairId == PAIR_HL && IS_RAB && aop->size - offset == 1 && !(aop->type == AOP_REG && (aop->aopu.aop_reg[offset]->rIdx == L_IDX || aop->aopu.aop_reg[offset]->rIdx == H_IDX)))
            {
              emit2 ("bool hl");
              cost (1, 2);
              if (!regalloc_dry_run)
                emit2 ("ld %s, %s", _pairs[pairId].l, aopGet (aop, offset, false));
              ld_cost (pairId == PAIR_HL ? ASMOP_L : pairId == PAIR_DE ? ASMOP_E : ASMOP_C, 0, aop, offset, true);
            }
          else
            {
              if (pairId == PAIR_HL && (aopInReg (aop, offset, IYL_IDX) || aopInReg (aop, offset, IYH_IDX)))
                UNIMPLEMENTED;
              if (!aopInReg (aop, offset, _pairs[pairId].l_idx))
                {
                  if (!HAS_IYL_INST && (aopInReg (aop, offset, IYL_IDX) || aopInReg (aop, offset, IYH_IDX)))
                    UNIMPLEMENTED;
                  if (!regalloc_dry_run)
                    emit2 ("ld %s, %s", _pairs[pairId].l, aopGet (aop, offset, false));
                  ld_cost (pairId == PAIR_HL ? ASMOP_L : pairId == PAIR_DE ? ASMOP_E : ASMOP_C, 0, aop, offset, true);
                }
              if (pairId == PAIR_HL && (aopInReg (aop, offset + 1, IYL_IDX) || aopInReg (aop, offset + 1, IYH_IDX)))
                UNIMPLEMENTED;
              if (!aopInReg (aop, offset + 1, _pairs[pairId].h_idx))
                {
                  if (!HAS_IYL_INST && (aopInReg (aop, offset + 1, IYL_IDX) || aopInReg (aop, offset + 1, IYH_IDX)))
                    UNIMPLEMENTED;
                  if (!regalloc_dry_run)
                    emit2 ("ld %s, %s", _pairs[pairId].h, aopGet (aop, offset + 1, false));
                  ld_cost (pairId == PAIR_HL ? ASMOP_H : pairId == PAIR_DE ? ASMOP_D : ASMOP_B, 0, aop, offset + 1, true);
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
      cost2 (1, 11, 11, 10, 16, 8, 3, 4);
      offset += 2;
    }

  if (id == PAIR_DE && !IS_SM83) // TODO: Could hl be in use for sm83, so it needs to be saved and restored?
    emit3w (A_EX, ASMOP_DE, ASMOP_HL);

  if (offset < INT8MIN || offset > INT8MAX || id == PAIR_IY)
    {
      struct dbuf_s dbuf;
      PAIR_ID lid = (id == PAIR_DE) ? PAIR_HL : id;
      dbuf_init (&dbuf, sizeof(int) * 3 + 1);
      dbuf_printf (&dbuf, "%d", offset);
      emit2 ("ld %s, !hashedstr", _pairs[lid].name, dbuf_c_str (&dbuf));
      if (lid == PAIR_IY)
        cost2 (4 - IS_TLCS90, 14, 12, 8, 0, 6, 4, 4);
      else
        cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      dbuf_destroy (&dbuf);
      emit2 ("add %s, sp", _pairs[lid].name);
      if (lid == PAIR_IY)
        cost2 (2, 15, 10, 4, 0, 8, 2, 2);
      else
        cost2 (1, 11, 7, 2, 8, 8, 1, 1);
    }
  else
    {
      wassert (id == PAIR_DE || id == PAIR_HL);
      emit2 ("!ldahlsp", offset);
      if (IS_SM83)
        cost (2, 12);
      else
        {
          cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          cost2 (1, 11, 7, 2, 8, 8, 1, 1);
        }
    }

  if (id == PAIR_DE && !IS_SM83)
    emit3w (A_EX, ASMOP_DE, ASMOP_HL);
  else if (id == PAIR_DE)
    {
      genMovePairPair (PAIR_HL, PAIR_DE);
      spillPair (PAIR_HL);
    }

  if (_G.preserveCarry)
    {
      _pop (PAIR_AF);
      cost2 (1, 10, 9, 7, 12, 6, 3, 3);
      offset -= 2;
    }
    
  spillPair (id);
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
      wassertl (!IS_SM83, "The SM83 doesn't have an extended stack");

    case AOP_STK:
      ; int abso = aop->aopu.aop_stk + offset + _G.stack.offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);

      if ((_G.pairs[pairId].last_type == AOP_STK || _G.pairs[pairId].last_type == AOP_EXSTK) && abs (_G.pairs[pairId].offset - abso) < (_G.preserveCarry ? 5 : 3))
        adjustPair (_pairs[pairId].name, &_G.pairs[pairId].offset, abso);
      else
        setupPairFromSP (pairId, abso + _G.stack.pushed);

      _G.pairs[pairId].offset = abso;

      break;

    // Legacy.
    case AOP_HL:
    case AOP_IY:
      fetchLitPair (pairId, (asmop *) aop, offset, true, false);
      _G.pairs[pairId].offset = offset;
      break;

    case AOP_PAIRPTR:
      wassert (!offset);

      shiftIntoPair (pairId, (asmop *) aop); // Legacy. Todo: eliminate uses of shiftIntoPair() ?

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
      fetchLitPair (pairId, aop, 0, true, false);
      break;

    case AOP_HL:
      wassertl (pairId == PAIR_HL, "AOP_HL must be in HL");

      fetchLitPair (pairId, aop, offset, true, false);
      _G.pairs[pairId].offset = offset;
      break;

    case AOP_EXSTK:
      wassertl (!IS_SM83, "The SM83 doesn't have an extended stack");
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
          dbuf_tprintf (&dbuf, bit16 ? "hl" : "a");
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
                  dbuf_tprintf (&dbuf, "!zero");
                  break;

                case 1:
                  dbuf_tprintf (&dbuf, "!msbimmeds", aop->aopu.aop_immd);
                  break;

                case 0:
                  dbuf_tprintf (&dbuf, "!lsbimmeds", aop->aopu.aop_immd);
                  break;

                default:
                  dbuf_tprintf (&dbuf, "!zero");
                }
            }
          break;

        case AOP_DIR:
          wassert (IS_SM83);
          emit2 ("ld a, (%s+%d)", aop->aopu.aop_dir, offset);
          cost2 (3, 13, 12, 9, 16, 10, 4, 4);
          dbuf_append_char (&dbuf, 'a');
          break;

        case AOP_SFR:
          wassertl (!IS_TLCS90, "TLCS-90 does not have a separate I/O space");
          if (IS_SM83)
            {
              emit2 ("!rldh", aop->aopu.aop_dir, offset);
              cost (2, 12);
              dbuf_append_char (&dbuf, 'a');
            }
          else if (IS_RAB)
            {
              emit2 ("ioi");
              emit2 ("ld a, !mems", aop->aopu.aop_dir);
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
                  emit2 ("in0 a, !mems", aop->aopu.aop_dir);
                }
              else
                {
                  /* 8 bit mode */
                  emit2 ("in a, !mems", aop->aopu.aop_dir);
                  cost2 (2, 11, 9, 0, 0, 0, 3, 3);
                }

              dbuf_append_char (&dbuf, 'a');
            }
          break;

        case AOP_REG:
          if (bit16)
            {
              if (aopInReg (aop, offset, IY_IDX))
                dbuf_append_str (&dbuf, "iy");
              else
                {
                  dbuf_append_str (&dbuf, aop->aopu.aop_reg[offset + 1]->name);
                  dbuf_append_str (&dbuf, aop->aopu.aop_reg[offset]->name);
                }
            }
          else
            dbuf_append_str (&dbuf, aop->aopu.aop_reg[offset]->name);
          break;

        case AOP_HL:
          setupPair (PAIR_HL, aop, offset);
          dbuf_tprintf (&dbuf, "!*hl");
          break;

        case AOP_IY:
          wassert (!IS_SM83);
          setupPair (PAIR_IY, aop, offset);
          dbuf_tprintf (&dbuf, "!*iyx", offset);
          break;

        case AOP_EXSTK:
          if (!IY_RESERVED)
            {
              wassert (!IS_SM83);
              setupPair (PAIR_IY, aop, offset);
              dbuf_tprintf (&dbuf, "!*iyx", offset);
              break;
            }

        case AOP_STK:
          if (IS_TLCS90) // Try to use (sp) addressing mode.
            {
              int sp_offset = aop->aopu.aop_stk + offset + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0) + _G.stack.pushed + _G.stack.offset;

              if (!sp_offset)
                {
                  dbuf_tprintf (&dbuf, "(sp)");
                  break;
                }
            }

          if (IS_SM83 || aop->type == AOP_EXSTK)
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
          dbuf_append_str (&dbuf, aopLiteralLong (aop->aopu.aop_lit, offset, 1 + bit16));
          break;

        case AOP_PAIRPTR:
          setupPair (aop->aopu.aop_pairId, aop, offset);
          if (aop->aopu.aop_pairId == PAIR_IX)
            dbuf_tprintf (&dbuf, "!*ixx", offset);
          else if (aop->aopu.aop_pairId == PAIR_IY)
            dbuf_tprintf (&dbuf, "!*iyx", offset);
          else
            dbuf_tprintf (&dbuf, "!mems", _pairs[aop->aopu.aop_pairId].name);
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

#define AOP_NEEDSACC(x) ((x)->aop && (((x)->aop->type == AOP_CRY) || ((x)->aop->type == AOP_SFR)))
#define AOP_IS_PAIRPTR(x, p) ((x)->aop->type == AOP_PAIRPTR && (x)->aop->aopu.aop_pairId == (p))

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
      wassert (IS_SM83);
      if (strcmp (s, "a"))
        emit2 ("ld a, %s", s);
      emit2 ("ld (%s+%d),a", aop->aopu.aop_dir, offset);
      break;

    case AOP_SFR:
      wassertl (!IS_TLCS90, "TLCS-90 does not have a separate I/O space");
      if (IS_SM83)
        {
          //  wassert (IS_SM83);
          if (strcmp (s, "a"))
            emit2 ("ld a, %s", s);
          emit2 ("!lldh", aop->aopu.aop_dir, offset);
        }
      else if (IS_RAB)
        {
          if (strcmp (s, "a"))
            emit2 ("ld a, %s", s);

          /* LM 20110928: Need to fix to emit either "ioi" or "ioe"
           * (for internal vs. external I/O space
           */
          emit2 ("ioi");
          emit2 ("ld !mems,a", aop->aopu.aop_dir);
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
              emit2 ("out (c), %s", s);

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
      wassert (!IS_SM83);
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
      //wassert (IS_SM83);
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
          wassert (!IS_SM83);
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
      if (IS_SM83 || aop->type == AOP_EXSTK)
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

    case AOP_PAIRPTR:
      setupPair (aop->aopu.aop_pairId, aop, offset);
      if (aop->aopu.aop_pairId == PAIR_IX)
        emit2 ("ld !*ixx, %s", 0, s);
      else if (aop->aopu.aop_pairId == PAIR_IY)
        emit2 ("ld !*iyx, %s", 0, s);
      else
        emit2 ("ld !mems, %s", _pairs[aop->aopu.aop_pairId].name, s);
      break;

    default:
      dbuf_destroy (&dbuf); fprintf (stderr, "AOP_DIR: %d\n",AOP_DIR);
      fprintf (stderr, "aop->type: %d\n", aop->type);
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut got unsupported aop->type");
      exit (0);
    }
  dbuf_destroy (&dbuf);
}

// pop a register pair while not destroying one of the two registers in it (destroying tempreg instead, if available).
static void
poppairwithsavedreg (PAIR_ID pair, short survivingreg, short tempreg)
{
  if (tempreg >= 0)
    {
      emit2 ("ld %s, %s", regsZ80[tempreg].name, regsZ80[survivingreg].name);
      ld_cost (ASMOP_L, 0, ASMOP_H, 0, true);
      _pop (pair);
      emit2 ("ld %s, %s", regsZ80[survivingreg].name, regsZ80[tempreg].name);
      ld_cost (ASMOP_L, 0, ASMOP_H, 0, true);
      return;
    }

  // No tempreg, need to do it the hard way via stack access.
  bool isupperbyte = (survivingreg == B_IDX || survivingreg == D_IDX || survivingreg == H_IDX || survivingreg == IYH_IDX);
  _push (PAIR_AF); // Save flags
  _push (PAIR_HL); // Save hl
  emit2 ("ld hl, !immedword", 4 + isupperbyte);
  cost2 (3, 10, 9, 6, 12, 6, 3, 3);
  emit2 ("add hl, sp");
  cost2 (2, 15, 10, 4, 0, 8, 2, 2);
  emit2 ("ld (hl), %s", regsZ80[survivingreg].name);
  cost2 (1, 7, 7, 6, 8, 6, 2, 2);
  _pop (PAIR_HL);
  _pop (PAIR_AF);
  _pop (pair);
}

// Move, but try not to. Preserves flags. Cannot use xor to zero, since xor resets the carry flag.
static void
cheapMove (asmop *to, int to_offset, asmop *from, int from_offset, bool a_dead)
{
#if 0
  emitDebug ("; cheapMove");
#endif

  if (aopInReg (to, to_offset, A_IDX))
    a_dead = true;

  if (from->type == AOP_STL)
    {
      if (from_offset > 2)
        {
          cheapMove (to, to_offset, ASMOP_ZERO, 0, a_dead);
          return;
        }

      // Need free a do be able to partially restore hl below.
      bool pushed_a = false;
      if ((aopInReg (to, to_offset, L_IDX) || aopInReg (to, to_offset, H_IDX)) && !a_dead)
        {
          _push (PAIR_AF);
          pushed_a = true;
        }

      _push (PAIR_HL);
      if (!pushed_a) // Preserve f
        _push (PAIR_AF);
      emit2 ("ld hl, !immed%d", spOffset(from->aopu.aop_stk));
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("add hl, sp");
      cost2 (1, 11, 7, 2, 8, 8, 1, 1);
      if (!pushed_a)
        _pop (PAIR_AF);
      spillPair (PAIR_HL);
      cheapMove (to, to_offset, ASMOP_HL, from_offset, a_dead);
      if (aopInReg (to, to_offset, L_IDX))
        poppairwithsavedreg (PAIR_HL, H_IDX, A_IDX);
      else if (aopInReg (to, to_offset, H_IDX))
        poppairwithsavedreg (PAIR_HL, L_IDX, A_IDX);
      else
        _pop (PAIR_HL);

      if (pushed_a)
        _pop (PAIR_AF);

      return;
    }

  const bool from_index = aopInReg (from, from_offset, IYL_IDX) || aopInReg (from, from_offset, IYH_IDX);
  const bool to_index = aopInReg (to, to_offset, IYL_IDX) || aopInReg (to, to_offset, IYH_IDX);
  const bool index = to_index || from_index;

  if (to->type == AOP_REG && from->type == AOP_REG)
    {
      if (to->aopu.aop_reg[to_offset] == from->aopu.aop_reg[from_offset])
        return;

      if (!index ||
        // eZ80 can assign between any byte of an index register and any non-hl register.
        HAS_IYL_INST && !aopInReg (to, to_offset, L_IDX) && !aopInReg (to, to_offset, H_IDX) && !aopInReg (from, from_offset, L_IDX) && !aopInReg (from, from_offset, H_IDX))
        {
          if (!regalloc_dry_run)
            aopPut (to, aopGet (from, from_offset, false), to_offset);
          ld_cost (to, to_offset, from, from_offset, true);
          spillPairReg (to->aopu.aop_reg[to_offset]->name);
          return;
        }
#if 0 // Might destroy carry. Would also mess up interrupts on TLCS-90.
      if (aopInReg (from, from_offset, IYH_IDX) && !to_index && a_dead)
        {
          _push(PAIR_IY);
          _pop (PAIR_AF);
          cheapMove (to, to_offset, ASMOP_A, 0, true);
          return;
        }
#endif
    }

  if (from->type != AOP_REG && from->type != AOP_LIT && aopIsLitVal (from, from_offset, 1, 0x00))
    {
      cheapMove (to, to_offset, ASMOP_ZERO, 0, a_dead);
      return;
    }
  else if (HAS_IYL_INST && to_index && (from->type == AOP_LIT || from->type == AOP_IMMD))
    {
      if (!regalloc_dry_run)
        aopPut (to, aopGet (from, from_offset, false), to_offset);
      ld_cost (to, 0, from_offset < from->size ? from : ASMOP_ZERO, from_offset, true);
      return;
    }
  else if (to_index && HAS_IYL_INST)
    {
      if (!a_dead)
        _push (PAIR_AF);
      cheapMove (ASMOP_A, 0, from, from_offset, true);
      if (!regalloc_dry_run)
        aopPut (to, "a", to_offset);
      ld_cost (to, to_offset, ASMOP_A, 0, true);
      spillPairReg (to->aopu.aop_reg[to_offset]->name);
      if (!a_dead)
        _pop (PAIR_AF);
      return;
    }

  if (to->type == AOP_REG && from_index && !to_index && - _G.stack.pushed - _G.stack.offset >= -128 && !_G.omitFramePtr)
    {
      _push(PAIR_IY);
      if (!regalloc_dry_run)
        emit2 ("ld %s, %d (ix)", aopGet (to, to_offset, false), - _G.stack.pushed - _G.stack.offset + aopInReg (from, from_offset, IYH_IDX));
      cost2 (3, 19, 14, 9, 0, 10, 4, 5);
      spillPairReg (to->aopu.aop_reg[to_offset]->name);
      _pop(PAIR_IY);
      return;
    }
   else if (to_index && !from_index && from->type == AOP_REG && - _G.stack.pushed - _G.stack.offset >= -128 && !_G.omitFramePtr)
    {
      _push(PAIR_IY);
      if (!regalloc_dry_run)
        emit2 ("ld %d (ix), %s", - _G.stack.pushed - _G.stack.offset + aopInReg (to, to_offset, IYH_IDX), aopGet (from, from_offset, false));
      cost2 (3, 19, 15, 10, 0, 10, 4, 5);
      _pop(PAIR_IY);
      return;
    }

  if (from_index && !to_index && !aopInReg (to, to_offset, L_IDX) && !aopInReg (to, to_offset, H_IDX))
    {
      _push (PAIR_IY);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      cheapMove (to, to_offset, aopInReg (from, from_offset, IYL_IDX) ? ASMOP_L : ASMOP_H, 0, a_dead);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      _pop (PAIR_IY);
      return;
    }
  else if (to_index && !from_index && !aopInReg (from, from_offset, L_IDX) && !aopInReg (from, from_offset, H_IDX))
    {
      _push (PAIR_IY);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      cheapMove (aopInReg (to, to_offset, IYL_IDX) ? ASMOP_L : ASMOP_H, 0, from, from_offset, a_dead);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      _pop (PAIR_IY);
      return;
    }
  else if (from_index && !to_index)
    {
      wassert (aopInReg (to, to_offset, L_IDX) || aopInReg (to, to_offset, H_IDX));
      _push (PAIR_IY);
      emit3w (A_EX, ASMOP_DE, ASMOP_HL);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      cheapMove (aopInReg (to, to_offset, L_IDX) ? ASMOP_E : ASMOP_D, 0, aopInReg (from, from_offset, IYL_IDX) ? ASMOP_L : ASMOP_H, 0, a_dead);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      emit3w (A_EX, ASMOP_DE, ASMOP_HL);
      _pop (PAIR_IY);
      return;
    }
  else if (to_index && !from_index)
    {
      wassert (aopInReg (from, from_offset, L_IDX) || aopInReg (from, from_offset, H_IDX));
      _push (PAIR_IY);
      emit3w (A_EX, ASMOP_DE, ASMOP_HL);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      cheapMove (aopInReg (to, to_offset, IYL_IDX) ? ASMOP_L : ASMOP_H, 0, aopInReg (from, from_offset, L_IDX) ? ASMOP_E : ASMOP_D, 0, a_dead);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      emit3w (A_EX, ASMOP_DE, ASMOP_HL);
      _pop (PAIR_IY);
      return;
    }
  else if (to_index && from_index)
    {
      _push (PAIR_IY);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      cheapMove (aopInReg (to, to_offset, IYL_IDX) ? ASMOP_L : ASMOP_H, 0, aopInReg (to, to_offset, IYL_IDX) ? ASMOP_L : ASMOP_H, 0, a_dead);
      emit2 ("ex (sp), hl");
      cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      _pop (PAIR_IY);
      return;
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
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          emit2 ("push %s", aopInReg (from, from_offset, A_IDX) ? "af" : (aopInReg (from, from_offset, B_IDX) ? "bc" : (aopInReg (from, from_offset, D_IDX) ? "de" : (aopInReg (from, from_offset, H_IDX) ? "hl" : "iy"))));
          if (aopInReg (from, from_offset, IYH_IDX))
            cost2 (2 - IS_TLCS90, 1, 13, 12, 0, 8, 4, 5);
          else
            cost2 (1, 11, 11, 10, 16, 8, 3, 4);
          emit2 ("inc sp");
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          return;
        }
    }

  if (from->type == AOP_IY && aopInReg (to, to_offset, A_IDX) && from_offset < from->size)
    {
      emit2 ("ld a, (%s+%d)", from->aopu.aop_dir, from_offset);
      cost2 (3, 13, 12, 9, 16, 10, 4, 4);
    }
  else if (aopInReg (from, from_offset, A_IDX) && to->type == AOP_IY)
    {
      wassert (to_offset < to->size);
      emit2 ("ld (%s+%d), a", to->aopu.aop_dir, to_offset);
      cost2 (3, 13, 13, 10, 16, 10, 4, 4);
    }
  else if (!aopInReg (to, to_offset, A_IDX) && !aopInReg (from, from_offset, A_IDX) && // Go through a.
    (from->type == AOP_DIR ||
    from->type == AOP_SFR || to->type == AOP_SFR ||
    (to->type == AOP_HL || to->type == AOP_IY || to->type == AOP_EXSTK || to->type == AOP_STK) && (from->type == AOP_HL || from->type == AOP_IY || from->type == AOP_EXSTK || from->type == AOP_STK) ||
    (to->type == AOP_HL || IS_SM83 && to->type == AOP_STK || to->type == AOP_EXSTK) && (aopInReg(from, from_offset, L_IDX) || aopInReg(from, from_offset, H_IDX))) ||
    to->type == AOP_PAIRPTR && from->type == AOP_PAIRPTR)
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
      if (to->type == AOP_REG)
        spillPairReg (to->aopu.aop_reg[to_offset]->name);

      ld_cost (to, 0, from_offset < from->size ? from : ASMOP_ZERO, from_offset, true);
    }
}

static void
commitPair (asmop *aop, PAIR_ID id, const iCode *ic, bool dont_destroy) // Obsolete. Replace uses by genMove or genMove_o.
{
  int fp_offset = aop->aopu.aop_stk + (aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
  int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;

  if (getPairId (aop) == id)
    return;

  /* Stack positions will change, so do not assume this is possible in the cost function. */
  if (!regalloc_dry_run && !IS_SM83 && (aop->type == AOP_STK || aop->type == AOP_EXSTK) && !sp_offset
      && ((!IS_RAB && id == PAIR_HL) || id == PAIR_IY) && !dont_destroy)
    {
      emit2 ("ex (sp), %s", _pairs[id].name);
      if (id == PAIR_IY)
        cost2 (2, 23, 19, 15, 0, 14, 6, 6);
      else
        cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
      spillPair (id);
    }
  else if ((IS_RAB || IS_TLCS90) && (aop->type == AOP_STK || aop->type == AOP_EXSTK) && (id == PAIR_HL || id == PAIR_IY) &&
           (id == PAIR_HL && abs (fp_offset) <= 127 && aop->type == AOP_STK || abs (sp_offset) <= 127))
    {
      if (abs (sp_offset) <= 127)
        {
          emit2 ("ld %d (sp), %s", sp_offset, id == PAIR_IY ? "iy" : "hl");       /* Relative to stack pointer. */
          cost2 (2 + IS_TLCS90, 0, 0, 11, 0, 12, 0, 0);
        }
      else
        {
          emit2 ("ld %d (ix), hl", fp_offset);    /* Relative to frame pointer. */
          cost2 (3 - IS_RAB, 0, 0, 11, 0, 12, 5, 0);
        }
    }
  else if (IS_EZ80_Z80 && aop->type == AOP_STK)
    {
      emit2 ("ld %d (ix), %s", fp_offset, _pairs[id].name);
      cost (3, 5);
    }
  else if (!regalloc_dry_run && (aop->type == AOP_STK || aop->type == AOP_EXSTK) && !sp_offset)
    {
      emit2 ("inc sp");
      cost2 (1, 6, 4, 2, 8, 4, 1, 1);
      emit2 ("inc sp");
      cost2 (1, 6, 4, 2, 8, 4, 1, 1);
      emit2 ("push %s", _pairs[id].name);
      if (id == PAIR_IY)
        cost2 (2, 15, 13, 12, 0, 8, 4, 5);
      else
        cost2 (1, 11, 11, 10, 16, 8, 3, 4);
    }

  /* PENDING: Verify this. */
  else if (id == PAIR_HL && requiresHL (aop) && (IS_SM83 || IY_RESERVED && aop->type != AOP_HL && aop->type != AOP_IY))
    {
      if (!isRegDead (D_IDX, ic))
        _push (PAIR_DE);
      emit3 (A_LD, ASMOP_A, ASMOP_L);
      emit3 (A_LD, ASMOP_D, ASMOP_H);
      if (!regalloc_dry_run)
        {
          aopPut (aop, "a", 0);
          aopPut (aop, "d", 1);
        }
      ld_cost (aop, 0, ASMOP_A, 0, true);
      ld_cost (aop, 0, ASMOP_D, 0, true);
      if (!isRegDead (D_IDX, ic))
        _pop (PAIR_DE);
    }
  else
    {
      /* Special cases */
      if ((aop->type == AOP_IY || aop->type == AOP_HL) && !IS_SM83 && aop->size == 2)
        {
          if (!regalloc_dry_run)
            emit2 ("ld !mems, %s", aopGetLitWordLong (aop, 0, FALSE), _pairs[id].name);
          if (id == PAIR_HL)
            cost2 (3, 16, 16, 13, 0, 0, 5, 5);
          else
            cost2 (4, 20, 19, 15, 0, 12, 6, 6);
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
              if (!IS_SM83 && aop->type == AOP_REG && aop->aopu.aop_reg[0]->rIdx == L_IDX && aop->aopu.aop_reg[1]->rIdx == H_IDX && !dont_destroy)
                {
                  emit3w (A_EX, ASMOP_DE, ASMOP_HL);
                  swapPairs (PAIR_DE, PAIR_HL);
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
              else if (!IS_SM83 && aop->type == AOP_REG && aop->aopu.aop_reg[0]->rIdx == E_IDX && aop->aopu.aop_reg[1]->rIdx == D_IDX && !dont_destroy)
                {
                  emit3w (A_EX, ASMOP_DE, ASMOP_HL);
                  swapPairs (PAIR_DE, PAIR_HL);
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
  // Avoid overwriting source. Do not assume stack locations during dry run - they can change later.
  int dir = (!regalloc_dry_run && result->aopu.aop_stk + roffset > source->aopu.aop_stk + soffset && result->aopu.aop_stk + roffset < source->aopu.aop_stk + soffset + n) ? -1 : 1;

  for (int j = 0; j < n;)
    {
      int i = (dir >= 0) ? j : (n - j - 1);

      if (assigned[i])
        {
          j++;
          continue;
        }

      if (!aopOnStack (result, roffset + i, 1) || !aopOnStack (source, soffset + i, 1))
        {
          j++;
          continue;
        }
  
      int source_fp_offset = source->aopu.aop_stk + soffset + i + (source->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      int source_sp_offset = source_fp_offset + _G.stack.pushed + _G.stack.offset;
      int result_fp_offset = result->aopu.aop_stk + roffset + i + (result->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      int result_sp_offset = result_fp_offset + _G.stack.pushed + _G.stack.offset;
        
      if (result_fp_offset == source_fp_offset && !regalloc_dry_run) // Stack locations can change, so in dry run do not assume stack coalescing will happen.
        {
          assigned[i] = true;
          j++;
          continue;
        }

      bool source_sp = IS_RAB && source_sp_offset <= 255 || IS_TLCS90 && source_sp_offset <= 127;
      bool result_sp = IS_RAB && result_sp_offset <= 255 || IS_TLCS90 && result_sp_offset <= 127;
      if (i + 1 < n && !assigned[i + 1] && hl_free && (IS_RAB || IS_EZ80_Z80 || IS_TLCS90) && // Todo: For Rabbit, use ld hl n (sp) and ld n(sp), hl when sp_offset is <= 255.
        (result->type == AOP_STK && result_fp_offset >= -128 && result_fp_offset <= 127 || result_sp) &&
        (source->type == AOP_STK && source_fp_offset >= -128 && source_fp_offset <= 127 || source_sp))
        {
          if (!regalloc_dry_run)
            {
              if (source_sp)
                emit2 ("ld hl, %d (sp)", source_sp_offset);
              else
                emit2 ("ld hl, %s", aopGet (source, soffset + i, false));
              if (result_sp)
                emit2 ("ld %d (sp), hl", result_sp_offset);
              else
                emit2 ("ld %s, hl", aopGet (result, roffset + i, false));
            }
          cost2 (6 - 2 * IS_RAB, 0, 0, 22, 0, 21, 10, 0);

          spillPair (PAIR_HL);

          assigned[i] = true;
          assigned[i + 1] = true;
          (*size) -= 2;
          j += 2;
          continue;
        }

      if (a_free || really_do_it_now)
        {
          if ((requiresHL (result)  || requiresHL (source)) && !hl_free)
            _push (PAIR_HL);
          cheapMove (result, roffset + i, source, soffset + i, a_free);
          if ((requiresHL (result)  || requiresHL (source)) && !hl_free)
            _pop (PAIR_HL);
          assigned[i] = true;
          (*size)--;
          j++;
          continue;
        }

       j++;
    }

  wassertl_bt (*size >= 0, "genCopyStack() copied more than there is to be copied.");
}

/*-----------------------------------------------------------------*/
/* genCopy - Copy the value from one reg/stk asmop to another      */
/*-----------------------------------------------------------------*/
static void
genCopy (asmop *result, int roffset, asmop *source, int soffset, int sizex, bool a_dead, bool hl_dead, bool de_dead)
{
  int regsize, size, n = (sizex < source->size - soffset) ? sizex : (source->size - soffset);
  bool assigned[8] = {false, false, false, false, false, false, false, false};
  bool a_free, hl_free;
  int cached_byte = -1;
  bool pushed_a = false;

  wassertl_bt (n <= 8, "Invalid size for genCopy().");
  wassertl_bt (aopRS (source), "Invalid source type.");
  wassertl_bt (aopRS (result), "Invalid result type.");
  
  a_dead |= (result->regs[A_IDX] >= roffset && result->regs[A_IDX] < roffset + sizex);
  hl_dead |= (result->regs[L_IDX] >= roffset && result->regs[L_IDX] < roffset + sizex && result->regs[H_IDX] >= roffset && result->regs[H_IDX] < roffset + sizex);
  de_dead |= (result->regs[E_IDX] >= roffset && result->regs[E_IDX] < roffset + sizex && result->regs[D_IDX] >= roffset && result->regs[D_IDX] < roffset + sizex);

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
  wassert (source->type != AOP_REG || source->size < 8);
  for (int i = 0; i < n && source->type == AOP_REG;)
    {
      bool a_free = a_dead && (source->regs[A_IDX] < soffset || assigned[source->regs[A_IDX] - soffset] || i == source->regs[A_IDX] - soffset);
      bool hl_free = hl_dead && (source->regs[L_IDX] < soffset || assigned[source->regs[L_IDX] - soffset] || i == source->regs[L_IDX] - soffset) && (source->regs[H_IDX] < soffset || assigned[source->regs[H_IDX] - soffset] || i == source->regs[H_IDX] - soffset);
      bool de_free = de_dead && (source->regs[E_IDX] < soffset || assigned[source->regs[E_IDX] - soffset] || i == source->regs[E_IDX] - soffset) && (source->regs[D_IDX] < soffset || assigned[source->regs[D_IDX] - soffset] || i == source->regs[D_IDX] - soffset);

      int fp_offset = result->aopu.aop_stk + (result->aopu.aop_stk > 0 ? _G.stack.param_offset : 0) + roffset + i;
      int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;

      if (!IS_SM83 && !IS_RAB && !(IS_TLCS90 && optimize.codeSpeed) && // The sm83 doesn't have ex (sp), hl. The Rabbits and tlcs90 have it, but ld 0 (sp), hl is faster. For the Rabbits, they are also the same size.
        i + 1 < n && aopOnStack (result, roffset + i, 2) && !sp_offset &&
        aopInReg (source, soffset + i, HL_IDX) && hl_dead && // If we knew that iy was dead, we could also use ex (sp), iy here.
        !regalloc_dry_run) // Stack positions will change, so do not assume this is possible in the cost function.
        {
          emit2 ("ex (sp), hl");
          cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
          spillPair (PAIR_HL);
          assigned[i] = true;
          assigned[i + 1] = true;
          regsize -= 2;
          size -= 2;
          i += 2;        
        }
      else if (i + 1 < n && aopOnStack (result, roffset + i, 2) && (abs(fp_offset) <= 127 && !_G.omitFramePtr || IS_RAB && sp_offset <= 255 || IS_TLCS90 && sp_offset <= 127) &&
        ((aopInReg (source, soffset + i, HL_IDX) || aopInReg (source, soffset + i, IY_IDX)) && IS_RAB || (getPairId_o (source, soffset + i) != PAIR_INVALID && (IS_EZ80_Z80 || IS_TLCS90))))
        {
          bool use_sp = IS_RAB && sp_offset <= 255 || IS_TLCS90 && sp_offset <= 127;
          if (!regalloc_dry_run)
            emit2 ("ld %d %s, %s", use_sp ? sp_offset : fp_offset, use_sp ? "(sp)" : "(ix)", _pairs[getPairId_o (source, soffset + i)].name);
          cost2 (3 - IS_RAB, 0, 0, 11, 0, 12, 5, 0);
          assigned[i] = true;
          assigned[i + 1] = true;
          regsize -= 2;
          size -= 2;
          i += 2;
        }
      else if (IS_RAB && i + 1 < n && aopOnStack (result, roffset + i, 2) && aopInReg (source, soffset + i, IY_IDX) &&
        sp_offset <= 255)
        {
          emit2 ("ld %d (sp), iy", sp_offset);
          cost2 (3, 0, 0, 13, 0, 0, 0, 0);
          assigned[i] = true;
          assigned[i + 1] = true;
          regsize -= 2;
          size -= 2;
          i += 2;         
        }
      else if (i + 1 < n && aopOnStack (result, roffset + i, 2) && getPairId_o (source, soffset + i) != PAIR_INVALID && !sp_offset && !regalloc_dry_run) // Stack positions will change, so do not assume this is possible in the cost function.
        {
          bool iy = aopInReg (source, soffset + i, IY_IDX);
          emit2 ("inc sp");
          emit2 ("inc sp");
          emit2 ("push %s", _pairs[getPairId_o (source, soffset + i)].name);
          cost2 (3 + iy, 23 + 4 * iy, 19 + 3 * iy, 14 + 2 * iy, 32, 16, 5 + iy, 6 + iy);
          assigned[i] = true;
          assigned[i + 1] = true;
          regsize -= 2;
          size -= 2;
          i += 2;  
        }
      else if (IS_RAB && i + 1 < n && aopOnStack (result, roffset + i, 2) && aopInReg (source, soffset + i, DE_IDX) &&
        (sp_offset <= 255 || abs(fp_offset) <= 127 && !_G.omitFramePtr))
        {
          bool use_sp = (sp_offset <= 255);
          emit2 ("ex de, hl");
          if (!regalloc_dry_run)
            emit2 ("ld %d %s, hl", use_sp ? sp_offset : fp_offset, use_sp ? "(sp)" : "(ix)");
          cost2 (3, 0, 0, 13, 0, 0, 0, 0);
          if (!de_dead || !hl_dead || source->regs[L_IDX] >= 0 && !assigned[source->regs[L_IDX]] || source->regs[H_IDX] >= 0 && !assigned[source->regs[H_IDX]])
            emit3w (A_EX, ASMOP_DE, ASMOP_HL);
          spillPair (PAIR_HL);
          assigned[i] = true;
          assigned[i + 1] = true;
          regsize -= 2;
          size -= 2;
          i += 2;
        }
      else if (!IS_SM83 && i + 1 < n && aopOnStack (result, roffset + i, 2) && requiresHL (result) &&
        aopInReg (source, soffset + i, HL_IDX) && hl_free)
        {
          if (!de_free)
            _push (PAIR_DE);
          emit3w (A_EX, ASMOP_DE, ASMOP_HL);
          spillPair (PAIR_HL);
          genCopy (result, roffset + i, ASMOP_DE, 0, 2, a_free, true, true);
          if (!de_free)
            _pop (PAIR_DE);
          assigned[i] = true;
          assigned[i + 1] = true;
          regsize -= 2;
          size -= 2;
          i += 2;
        }
      else if (aopOnStack (result, roffset + i, 1) && requiresHL (result) && !hl_free)
        {
          _push(PAIR_HL);
          cheapMove (result, roffset + i, source, soffset + i, a_free);
          _pop(PAIR_HL);
          assigned[i] = true;
          regsize--;
          size--;
          i++;
        }
      else if (aopRS (source) && !aopOnStack (source, soffset + i, 1) && aopOnStack (result, roffset + i, 1))
        {
          cheapMove (result, roffset + i, source, soffset + i, a_free);
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
        hl_free = false;
    }
  genCopyStack (result, roffset, source, soffset, n, assigned, &size, a_free, hl_free, false);

  // Now do the register shuffling.

  // Try to use:
  // Rabbits: ld hl, iy; ld iy, hl
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

      if (IS_RAB && (getPairId_o (result, roffset + i) == PAIR_HL || getPairId_o (result, roffset + i) == PAIR_IY) && (getPairId_o (source, soffset + i) == PAIR_HL || getPairId_o (source, soffset + i) == PAIR_IY))
        {
          emit2 ("ld %s, %s", _pairs[getPairId_o (result, roffset + i)].name, _pairs[getPairId_o (source, soffset + i)].name);
          cost (2, 4);
          spillPair (getPairId_o (result, roffset + i));
        }
      else if (IS_TLCS90 && getPairId_o (result, roffset + i) != PAIR_INVALID && getPairId_o (source, soffset + i) != PAIR_INVALID)
        {
          emit2 ("ld %s, %s", _pairs[getPairId_o (result, roffset + i)].name, _pairs[getPairId_o (source, soffset + i)].name);
          bool hl = (aopInReg (result, roffset + i, HL_IDX) ^ aopInReg (source, soffset + i, HL_IDX));
          cost (2 - hl, 6 - 2 * hl);
          spillPair (getPairId_o (result, roffset + i));
        }
      else if (IS_EZ80_Z80 && getPairId_o (result, roffset + i) != PAIR_INVALID && aopInReg (source, soffset + i, IY_IDX))
        {
          emit2 ("lea %s, iy, !zero", _pairs[getPairId_o (result, roffset + i)].name);
          cost (3, 3);
          spillPair (getPairId_o (result, roffset + i));
        }
      else if (aopInReg (result, roffset + i, IY_IDX) && getPairId_o (source, soffset + i) != PAIR_INVALID ||
        getPairId_o (result, roffset + i) != PAIR_INVALID && aopInReg (source, soffset + i, IY_IDX))
        {
          _push (getPairId_o (source, soffset + i));
          _pop (getPairId_o (result, roffset + i));
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

  if (!IS_SM83)
    {
      int ex[4] = {-2, -2, -2, -2}; // Swapped bytes
      bool no = false; // Still needed byte would be overwritten

      // Find L and check that it is exchanged with E, find H and check that it is exchanged with D.
      for (int i = 0; i < n; i++)
        {
          if (assigned[i] &&
            (aopInReg (result, roffset + i, E_IDX) || aopInReg (result, roffset + i, L_IDX) || aopInReg (result, roffset + i, D_IDX) || aopInReg (result, roffset + i, H_IDX)))
            no = true;
       
          if (!assigned[i] && aopInReg (source, soffset + i, E_IDX))
            if (aopInReg (result, roffset + i, L_IDX))
              ex[0] = i;
            else
              no = true;
          if (!assigned[i] && aopInReg (source, soffset + i, L_IDX))
            if (aopInReg (result, roffset + i, E_IDX))
              ex[1] = i;
            else
              no = true;
          if (!assigned[i] && aopInReg (source, soffset + i, D_IDX))
            if (aopInReg (result, roffset + i, H_IDX))
              ex[2] = i;
            else
              no = true;
          if (!assigned[i] && aopInReg (source, soffset + i, H_IDX))
            if (aopInReg (result, roffset + i, D_IDX))
              ex[3] = i;
            else
              no = true; 
        }

      int exsum = (ex[0] >= 0) + (ex[1] >= 0) + (ex[2] >= 0) + (ex[3] >= 0);

      if (!no && exsum >= 2 && hl_dead && de_dead)
        {
          emit3w (A_EX, ASMOP_DE, ASMOP_HL);
          swapPairs (PAIR_DE, PAIR_HL);
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
          if (aopInReg (result, roffset + i, A_IDX))
            a_free = false;
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

      wassertl_bt (i != n, "genCopy error: Trying to cache non-existent byte in accumulator.");
      if (!a_free && !pushed_a)
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

  // Take de from stack first on Rabbit, while hl is still free, so we can do with just one ex de, hl.
  if (IS_RAB && hl_free &&
    result->regs[E_IDX] > roffset && result->regs[E_IDX] < roffset + n && !assigned[result->regs[E_IDX] - roffset] &&
    result->regs[D_IDX] > roffset && result->regs[D_IDX] < roffset + n && !assigned[result->regs[D_IDX] - roffset] &&
    result->regs[E_IDX] + 1 == result->regs[D_IDX])
    {
      int i = result->regs[E_IDX] - roffset;
      const int fp_offset = source->aopu.aop_stk + soffset + i + (source->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      const int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;     
      if (sp_offset <= 255 || fp_offset <= 127)
        {
          if (!regalloc_dry_run)
            if (sp_offset <= 255)
              {
                emit2 ("ld hl, %d (sp)", sp_offset);
              }
            else
              {
                emit2 ("ld hl, %s", aopGet (source, soffset + i, false));
              }
          cost (2, 9);
          emit3w (A_EX, ASMOP_DE, ASMOP_HL);
          spillPair (PAIR_HL);
          spillPair (PAIR_DE);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;
        }
    }
  
  // Last, move everything else from stack to registers.
  wassert (result->type != AOP_REG || result->size < 8);
  for (int i = 0; i < n && result->type == AOP_REG;)
    {
      const int fp_offset = source->aopu.aop_stk + soffset + i + (source->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      const int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;

      bool a_free = a_dead && (result->regs[A_IDX] < roffset || !assigned[result->regs[A_IDX] - roffset]);
      const bool hl_free = hl_dead && (result->regs[L_IDX] < roffset || !assigned[result->regs[L_IDX] - roffset]) && (result->regs[H_IDX] < roffset || !assigned[result->regs[H_IDX] - roffset]);
      const bool e_free = de_dead && (result->regs[E_IDX] < roffset || !assigned[result->regs[E_IDX] - roffset]);
      const bool d_free = de_dead && (result->regs[D_IDX] < roffset || !assigned[result->regs[D_IDX] - roffset]);
      const bool de_free = e_free && d_free;

      if (assigned[i])
        {
          i++;
          continue;
        }
      else if (i + 1 < n && !assigned[i + 1] && (source->type == AOP_STK || source->type == AOP_EXSTK) &&
        (IS_RAB && sp_offset <= 255 || IS_TLCS90 && sp_offset <= 127) &&
        (aopInReg (result, roffset + i, HL_IDX) || aopInReg (result, roffset + i, IY_IDX)))
        {
          if (!regalloc_dry_run)
            emit2 ("ld %s, %d (sp)", _pairs[getPairId_o (result, roffset + i)].name, sp_offset);
          spillPair (getPairId_o (result, roffset + i));
          cost2 (3 - IS_RAB, 0, 0, 11, 0, 12, 0, 0);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;
        }
      else if (i + 1 < n && !assigned[i + 1] && source->type == AOP_STK &&
        (aopInReg (result, roffset + i, HL_IDX) && IS_RAB ||
        (aopInReg (result, roffset + i, BC_IDX) || aopInReg (result, roffset + i, DE_IDX) || aopInReg (result, roffset + i, HL_IDX) || aopInReg (result, roffset + i, IY_IDX)) && (IS_EZ80_Z80 || IS_TLCS90)))
        {
          if (!regalloc_dry_run)
            emit2 ("ld %s, %s", _pairs[getPairId_o (result, roffset + i)].name, aopGet (source, soffset + i, false));
          spillPair (getPairId_o (result, roffset + i));
          cost2 (3 - IS_RAB, 0, 0, 11, 0, 12, 5, 0);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;
        }
      else if (i + 1 < n && !assigned[i + 1] && (source->type == AOP_STK || source->type == AOP_EXSTK) &&
        !sp_offset && getPairId_o (result, roffset + i) != PAIR_INVALID &&
        !regalloc_dry_run) // Stack locations might change.
        {
          PAIR_ID pair = getPairId_o (result, roffset + i);
          _pop (pair);
          _push (pair);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;
        }
      else if (i + 1 < n && !assigned[i + 1] && (source->type == AOP_STK || source->type == AOP_EXSTK) &&
        sp_offset == 2 && getPairId_o (result, roffset + i) != PAIR_INVALID &&
        (getPairId_o (result, roffset + i) != PAIR_HL && hl_free || getPairId_o (result, roffset + i) != PAIR_DE && de_free) &&
        (!regalloc_dry_run || source->aopu.aop_stk > 0) &&  // Stack locations might change, unless its a parameter.
        !(IS_RAB && aopInReg (result, roffset + i, DE_IDX)) && // For de, Rabbit can do it faster at same code size using ex twice
        (!IS_SM83 || optimize.codeSize) // SM83 can do it faster (worst case 2B bigger, but 1B smaller if lucky -> hl reuse)
        && !optimize.codeSpeed) // A bit slower (42 vs 38 cycles on Z80 and Z80N), so don't do it when optimizing for speed.
        {
          PAIR_ID pair = getPairId_o (result, roffset + i);
          PAIR_ID extrapair = (getPairId_o (result, roffset + i) != PAIR_HL && hl_free) ? PAIR_HL : PAIR_DE; // If we knew it is dead, we could use bc as extrapair here, too.
          _pop (extrapair);
          _pop (pair);
          _push (pair);
          _push (extrapair);
          spillPair (extrapair);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;
        }
      else if (i + 1 < n && !assigned[i + 1] &&
        (source->type == AOP_STK && fp_offset <= 127 || sp_offset <= 255) &&
        (aopInReg (result, roffset + i, DE_IDX) || result->type == AOP_REG && result->regs[L_IDX] < i && result->regs[IYL_IDX] < i && result->regs[H_IDX] < i && result->regs[IYH_IDX] < i && hl_free) &&
        IS_RAB)
        {
          if (!hl_free)
            emit2 ("ex de, hl");
          if (!regalloc_dry_run)
            if (sp_offset <= 255)
              emit2 ("ld hl, %d (sp)", sp_offset);
            else
              emit2 ("ld hl, %s", aopGet (source, soffset + i, false));
          cost2 (2 + !hl_free, 0, 0, 11 + !hl_free * 2, 0, 0, 0, 0);
          spillPair (PAIR_HL);
          if (aopInReg (result, roffset + i, DE_IDX))
            emit3w (A_EX, ASMOP_DE, ASMOP_HL);
          else
            {
              wassert (hl_free);
              emit3_o (A_LD, result, roffset + i, ASMOP_L, 0);
              emit3_o (A_LD, result, roffset + i + 1, ASMOP_H, 0);
            }
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;      
        }
      else if (i + 1 < n && !assigned[i + 1] && (source->type == AOP_STK || source->type == AOP_EXSTK) && requiresHL (source) &&
        (aopInReg (result, roffset + i, HL_IDX) || aopInReg (result, roffset + i, H_IDX) && aopInReg (result, roffset + i + 1, L_IDX))) // Stack access might go through hl.
        {
          bool a_pushed = false;
          if (!a_free && !e_free && !d_free)
            {
              _push (PAIR_AF);
              a_pushed = true;
              a_free = true;
            }
          asmop *tmpaop = a_free ? ASMOP_A : e_free ? ASMOP_E : ASMOP_D;
          cheapMove (tmpaop, 0, source, soffset + i, true);
          cheapMove (result, roffset + i + 1, source, soffset + i + 1, false);
          cheapMove (result, roffset + i, tmpaop, 0, true);
          if (a_pushed)
            _pop (PAIR_AF);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;
        }
      else if (aopRS (result) && aopOnStack (source, soffset + i, 1) && !aopOnStack (result, roffset + i, 1))
        {
          if (requiresHL (source) && !hl_free && (aopInReg (result, roffset + i, L_IDX) || aopInReg (result, roffset + i, H_IDX)))
            {
              if (!a_free)
                _push (PAIR_AF);
              _push (PAIR_HL);
              cheapMove (ASMOP_A, 0, source, soffset + i, true);
              _pop (PAIR_HL);
              cheapMove (result, roffset + i, ASMOP_A, 0, true);
              if (!a_free)
                _pop (PAIR_AF);
            }
          else
            {
              if (requiresHL (source) && !hl_free)
                _push (PAIR_HL);
              cheapMove (result, roffset + i, source, soffset + i, a_free);
              if (requiresHL (source) && source->type != AOP_REG && !hl_free)
                _pop (PAIR_HL);
              }
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
      hl_free = hl_dead && (result->regs[L_IDX] < 0 || result->regs[L_IDX] >= roffset + source->size) && (result->regs[H_IDX] < 0 || result->regs[H_IDX] >= roffset + source->size);
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
genMove_o (asmop *result, int roffset, asmop *source, int soffset, int size, bool a_dead_global, bool hl_dead_global, bool de_dead_global, bool iy_dead_global, bool f_dead)
{
  wassert (result);
  wassert (result->size >= roffset + size);
  emitDebug ("; genMove_o size %d result type %d source type %d hl_dead %d", size, result->type, source->type, hl_dead_global);

  a_dead_global |= result->type == AOP_REG && result->regs[A_IDX] >= roffset && result->regs[A_IDX] < roffset + size;
  hl_dead_global |= result->type == AOP_REG && result->regs[L_IDX] >= roffset && result->regs[L_IDX] < roffset + size && result->regs[H_IDX] >= roffset && result->regs[H_IDX] < roffset + size;
  de_dead_global |= result->type == AOP_REG && result->regs[E_IDX] >= roffset && result->regs[E_IDX] < roffset + size && result->regs[D_IDX] >= roffset && result->regs[D_IDX] < roffset + size;
  iy_dead_global |= result->type == AOP_REG && result->regs[IYL_IDX] >= roffset && result->regs[IYL_IDX] < roffset + size && result->regs[IYH_IDX] >= roffset && result->regs[IYH_IDX] < roffset + size;
  bool bc_dead_global = result->type == AOP_REG && result->regs[C_IDX] >= roffset && result->regs[C_IDX] < roffset + size && result->regs[B_IDX] >= roffset && result->regs[B_IDX] < roffset + size;

  if (aopSame (result, roffset, source, soffset, size))
    return;

  if ((result->type == AOP_REG || result->type == AOP_STK || result->type == AOP_EXSTK) && (source->type == AOP_REG || source->type == AOP_STK || source->type == AOP_EXSTK))
    {
      int csize = size > source->size - soffset ? source->size - soffset : size;
      genCopy (result, roffset, source, soffset, csize, a_dead_global, hl_dead_global, de_dead_global);
      roffset += csize;
      size -= csize;
      bool a_dead = a_dead_global && result->regs[A_IDX] < roffset;
      bool hl_dead = hl_dead_global && result->regs[H_IDX] < roffset && result->regs[L_IDX] < roffset;
      bool de_dead = de_dead_global && result->regs[D_IDX] < roffset && result->regs[E_IDX] < roffset;
      bool iy_dead = iy_dead_global && result->regs[IYH_IDX] < roffset && result->regs[IYL_IDX] < roffset;
      genMove_o (result, roffset, ASMOP_ZERO, 0, size, a_dead, hl_dead, de_dead, iy_dead, f_dead);
      return;
    }

  bool zeroed_a = false;
  long value_hl = -1;

  for (int i = 0; i < size;)
    {
      bool a_dead = a_dead_global && source->regs[A_IDX] <= soffset + i && (result->regs[A_IDX] < 0 || result->regs[A_IDX] >= roffset + i);
      bool hl_dead = hl_dead_global && source->regs[L_IDX] <= soffset + i && source->regs[H_IDX] <= soffset + i && (result->regs[L_IDX] < 0 || result->regs[L_IDX] >= roffset + i) && (result->regs[H_IDX] < 0 || result->regs[H_IDX] >= roffset + i);
      bool de_dead = de_dead_global && source->regs[E_IDX] <= soffset + i && source->regs[D_IDX] <= soffset + i && (result->regs[E_IDX] < 0 || result->regs[E_IDX] >= roffset + i) && (result->regs[D_IDX] < 0 || result->regs[D_IDX] >= roffset + i);
      bool iy_dead = iy_dead_global && source->regs[IYL_IDX] <= soffset + i && source->regs[IYH_IDX] <= soffset + i && (result->regs[IYL_IDX] < 0 || result->regs[IYL_IDX] >= roffset + i) && (result->regs[IYH_IDX] < 0 || result->regs[IYH_IDX] >= roffset + i);
      bool bc_dead = bc_dead_global && source->regs[C_IDX] <= soffset + i && source->regs[B_IDX] <= soffset + i && (result->regs[C_IDX] < 0 || result->regs[C_IDX] >= roffset + i) && (result->regs[B_IDX] < 0 || result->regs[B_IDX] >= roffset + i);

      if (source->type == AOP_STL && (soffset + i) >= 2)
        {
          genMove_o (result, roffset + i, ASMOP_ZERO, 0, size - i, a_dead, hl_dead, false, iy_dead, f_dead);
          return;
        }
      else if ((IS_TLCS90 || IS_EZ80_Z80) && source->type == AOP_STL && !(soffset + i) && getPairId_o(result, roffset) != PAIR_INVALID &&
        !_G.omitFramePtr && abs(fpOffset (source->aopu.aop_stk)) <= 127)
        {
          emit2 (IS_TLCS90 ? "lda %s, ix, !immed%d" : "lea %s, ix, !immed%d", _pairs[getPairId_o(result, roffset)].name, fpOffset (source->aopu.aop_stk));
          spillPair (getPairId_o(result, roffset));
          cost (3, IS_TLCS90 ? 10 : 3);
          i += 2;
          continue;
        }
      else if (source->type == AOP_STL && !(soffset + i) && getPairId_o(result, roffset) == PAIR_IY)
        {
          if (!f_dead)
            _push (PAIR_AF);
          emit2 ("ld iy, !immed%d", spOffset (source->aopu.aop_stk));
          emit2 ("add iy, sp");
          cost2 (6 - IS_TLCS90, 29, 22, 12, 0, 14, 6, 6);
          if (!f_dead)
            _pop (PAIR_AF);
          spillPair (PAIR_IY);
          i += 2;
          continue;
        }
      else if (!IS_SM83 && source->type == AOP_STL && !(soffset + i) && size == 2 && getPairId_o(result, roffset) == PAIR_DE) // For result in de, we don't need hl dead.
        {
          if (!hl_dead)
            emit3w (A_EX, ASMOP_DE, ASMOP_HL);
          else
            spillPair (PAIR_HL);
          if (!f_dead)
            _push (PAIR_AF);
          emit2 ("ld hl, !immed%d", spOffset (source->aopu.aop_stk));
          cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          emit2 ("add hl, sp");
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          if (!f_dead)
            _pop (PAIR_AF);
          emit3w (A_EX, ASMOP_DE, ASMOP_HL);
          spillPair (PAIR_DE);
          i += 2;
          continue;
        }
      else if (source->type == AOP_STL)
        {
          if (!hl_dead && (result->regs[L_IDX] > roffset || result->regs[H_IDX] > roffset))
            UNIMPLEMENTED;
          if (!hl_dead)
            _push (PAIR_HL);
          if (i + soffset > 1)
            UNIMPLEMENTED;
          if (!f_dead)
            _push (PAIR_AF);
          emit2 ("ld hl, !immed%d", spOffset (source->aopu.aop_stk));
          cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          emit2 ("add hl, sp");
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          if (!f_dead)
            _pop (PAIR_AF);
          spillPair (PAIR_HL);
          genMove_o (result, roffset + i, ASMOP_HL, soffset + i, size, a_dead, true, de_dead_global, iy_dead, f_dead);
          if (!hl_dead)
            _pop (PAIR_HL);
          i += 2;
          continue;
        }

      if ((IS_EZ80_Z80 || IS_RAB || IS_TLCS90) && i + 1 < size && result->type == AOP_STK &&
        source->type == AOP_LIT && (value_hl >= 0 && aopIsLitVal (source, soffset + i, 2, value_hl) || hl_dead))
        {
          if (value_hl < 0 || !aopIsLitVal (source, soffset + i, 2, value_hl))
            fetchLitPair (PAIR_HL, source, soffset + i, f_dead, false);
          if (!regalloc_dry_run)
            emit2 ("ld %s, hl", aopGet (result, roffset + i, false));
          cost2 (3 - IS_RAB, 0, 0, 11, 0, 12, 5, 0);
          value_hl = ullFromVal (source->aopu.aop_lit) >> ((soffset + i) * 8) & 0xffff;
          i += 2;
          continue;
        }
      else if (i + 1 < size && IS_SM83 && result->type != AOP_REG && requiresHL (result) && source->type == AOP_REG && requiresHL (source) && // word through de is cheaper than direct byte-by-byte, since it requires fewer updates of hl.
        de_dead_global && source->regs[E_IDX] <= i + 1 && source->regs[D_IDX] <= i + 1 &&
        hl_dead_global && source->regs[L_IDX] <= i + 1 && source->regs[H_IDX] <= i + 1)
        {
          cheapMove (ASMOP_E, 0, source, i, a_dead);
          cheapMove (ASMOP_D, 0, source, i + 1, a_dead);
          cheapMove (result, i, ASMOP_E, 0, a_dead);
          cheapMove (result, i + 1, ASMOP_D, 0, a_dead);
          i += 2;
          continue;
        }
      else if (!IS_SM83 && i + 1 < size && getPairId_o(source, soffset + i) != PAIR_INVALID &&
        (result->type == AOP_IY || result->type == AOP_DIR || result->type == AOP_HL && (getPairId_o(source, soffset + i) == PAIR_HL || !hl_dead)))
        {
          emit2 ("ld !mems, %s", aopGetLitWordLong (result, roffset + i, false), _pairs[getPairId_o(source, soffset + i)].name);
          if (getPairId_o(source, soffset + i) == PAIR_HL)
            cost2 (3, 16, 16, 13, 0, 0, 5, 5);
          else
            cost2 (4, 20, 19, 15, 0, 12, 6, 6);
          i += 2;
          continue;
        }
      else if (!IS_SM83 && i + 1 < size && soffset + i + 1 < source->size && getPairId_o(result, roffset + i) != PAIR_INVALID &&
        (source->type == AOP_IY || source->type == AOP_DIR || source->type == AOP_HL && (getPairId_o(result, roffset + i) == PAIR_HL || !hl_dead)))
        {
          emit2 ("ld %s, !mems", _pairs[getPairId_o(result, roffset + i)].name, aopGetLitWordLong (source, soffset + i, false));
          if (getPairId_o(result, roffset + i) == PAIR_HL)
            cost2 (3, 16, 15, 11, 0, 12, 5, 5);
          else
            cost2 (4, 20, 18, 13, 0, 12, 6, 6);
          spillPair (getPairId_o(result, roffset + i));
          i += 2;
          continue;      
        }
      else if (i + 1 < size && getPairId_o(result, roffset + i) != PAIR_INVALID &&
        (source->type == AOP_LIT && !(aopIsLitVal (source, soffset + i, 2, 0x0000) && zeroed_a) || source->type == AOP_IMMD))
        {
          fetchLitPair (getPairId_o(result, roffset + i), source, soffset + i, f_dead, false);
          i += 2;
          continue;
        }
      else if (!IS_SM83 && i + 1 < size &&
        (result->type == AOP_IY || result->type == AOP_DIR || result->type == AOP_HL) &&
        source->type == AOP_IMMD && hl_dead)
        {
          genMove_o (ASMOP_HL, 0, source, soffset + i, 2, a_dead, true, false, iy_dead, f_dead);
          genMove_o (result, roffset + i, ASMOP_HL, 0, 2, a_dead, true, false, iy_dead, f_dead);
          i += 2;
          continue;
        }

      // 16-bit load into register might be cheaper than 8-bit, if the latter has to go through a. For bc and de it is only worth it if a would have to be saved.
      if ((optimize.allow_unsafe_read || i + 1 == size && soffset + i + 1 <= source->size) && !IS_SM83 && !IS_TLCS90 && result->type == AOP_REG && !aopInReg (result, roffset + i, A_IDX) &&
        (i + 1 == size || soffset + i + 1 >= source->size) && (source->type == AOP_HL && fetchLitPair (PAIR_HL, source, soffset + i, f_dead, true) || source->type == AOP_IY))
        {
          bool upper = aopInReg (result, roffset + i, B_IDX) || aopInReg (result, roffset + i, D_IDX) || aopInReg (result, roffset + i, H_IDX) || aopInReg (result, roffset + i, IYH_IDX);
          PAIR_ID pair = PAIR_INVALID;
          if ((aopInReg (result, roffset + i, C_IDX) || aopInReg (result, roffset + i, B_IDX)) && bc_dead && !a_dead)
            pair = PAIR_BC;
          else if ((aopInReg (result, roffset + i, E_IDX) || aopInReg (result, roffset + i, D_IDX)) && de_dead && !a_dead)
            pair = PAIR_DE;
          else if ((aopInReg (result, roffset + i, L_IDX) || aopInReg (result, roffset + i, H_IDX)) && hl_dead)
            pair = PAIR_HL;
          else if ((aopInReg (result, roffset + i, IYL_IDX) || aopInReg (result, roffset + i, IYH_IDX)) && iy_dead)
            pair = PAIR_IY;

          if (pair != PAIR_INVALID && soffset + i - upper >= 0 && (optimize.allow_unsafe_read || upper || soffset + i + 1 < source->size))
            {
              emit2 ("ld %s, !mems", _pairs[pair].name, aopGetLitWordLong (source, soffset + i - upper, false));
              if (pair == PAIR_HL)
                cost2 (3, 16, 15, 11, 0, 12, 5, 5);
              else
                cost2 (4, 20, 18, 13, 0, 12, 6, 6);
              i++;
              spillPair (pair);
              continue;
            }
        }

      // Cache a copy of zero in a.
      if (f_dead && !zeroed_a && a_dead && source->regs[A_IDX] <= i &&
        (size > 1 && result->type != AOP_REG && aopIsLitVal (source, soffset + i, 2, 0x0000) ||
        size == 1 && (result->type == AOP_HL && fetchLitPair (PAIR_HL, result, roffset + i, f_dead, true) || result->type == AOP_IY && fetchLitPair (PAIR_IY, result, roffset + i, f_dead, true)) && aopIsLitVal (source, soffset + i, 1, 0x00)))
        {
          emit3 (A_XOR, ASMOP_A, ASMOP_A);
          zeroed_a = true;
        }

      if (result->type == AOP_HL && a_dead_global && (!hl_dead_global || source->regs[L_IDX] >= i || source->regs[H_IDX] >= i) && source->regs[A_IDX] <= i)
        {
          if (source->type == AOP_HL)
            {
              emit2 ("ld a, !mems", aopGetLitWordLong (source, soffset + i, false));
              cost2 (3, 13, 12, 9, 16, 10, 4, 4);
            }
          else if (!aopIsLitVal (source, soffset + i, 1, 0x00) || !zeroed_a)
            {
              cheapMove (ASMOP_A, 0, source, soffset + i, true);
              zeroed_a = aopIsLitVal (source, soffset + i, 1, 0x00);
            }
          emit2 ("ld !mems, a", aopGetLitWordLong (result, roffset + i, FALSE));
          cost2 (3, 13, 13, 10, 16, 10, 4, 4);
        }
      else if (aopIsLitVal (source, soffset + i, 1, 0x00) && zeroed_a)
        {
          if (requiresHL (result) && result->type != AOP_REG && !hl_dead)
            _push (PAIR_HL);
          cheapMove (result, roffset + i, ASMOP_A, 0, false);
          if (requiresHL (result) && result->type != AOP_REG && !hl_dead)
            _pop (PAIR_HL);
        }
      else if (aopIsLitVal (source, soffset + i, 1, 0x00) && aopInReg (result, roffset + i, A_IDX) && f_dead)
        {
          emit3 (A_XOR, ASMOP_A, ASMOP_A);
          zeroed_a = true;
        }
      else
        {
          bool pushed_hl = false;
          bool via_a = false;
          bool premoved_a = false;

          if (!i && a_dead && // Avoid setting up hl or iy for a single byte.
            (source->type == AOP_HL && fetchLitPair (PAIR_HL, source, soffset + i, f_dead, true) || source->type == AOP_IY && fetchLitPair (PAIR_IY, source, soffset + i, f_dead, true)) &&
            result->type == AOP_REG && (i + 1 > size || soffset + i < source->size))
            {
              if (IS_TLCS90 && result->type == AOP_REG && !aopInReg (result, roffset + i, IYL_IDX) && !aopInReg (result, roffset + i, IYH_IDX))
                {
                  if (!regalloc_dry_run)
                    emit2 ("ld %s, !mems", aopGet (result, roffset + i, false), aopGetLitWordLong (source, soffset + i, false));
                  cost (4, 10);
                  i++;
                  continue;
                }
              else
                {
                  emit2 ("ld a, !mems", aopGetLitWordLong (source, soffset + i, false));
                  cost2 (3 + IS_TLCS90, 13, 12, 9, 16, 10, 4, 4);
                  via_a = true;
                  premoved_a = true;
                }
            }
          else if ((requiresHL (result) && result->type != AOP_REG || requiresHL (source) && source->type != AOP_REG && soffset + i < source->size) && !hl_dead)
            {
              via_a = aopInReg (result, roffset + i, L_IDX) || aopInReg (result, roffset + i, H_IDX);
              if (via_a && !a_dead)
                _push (PAIR_AF);
              if (via_a && source->type == AOP_HL)
                {
                  emit2 ("ld a, !mems", aopGetLitWordLong (source, soffset + i, false));
                  cost2 (3 + IS_TLCS90, 13, 12, 9, 16, 10, 4, 4);
                  premoved_a = true;
                }
              else
                {
                  _push (PAIR_HL);
                  pushed_hl = true;
                }
            }
          else if (result->type == AOP_IY && !iy_dead && !aopInReg (source, soffset + i, A_IDX))
            {
              via_a = true;
              if (!a_dead)
                _push (PAIR_AF);
            }
          else if (!premoved_a && source->type == AOP_IY && result->type == AOP_REG && a_dead && i == 0 && i + 1 == size) // Using free a is cheaper than using iy.
            via_a = true;
          if (!premoved_a)
            {
              bool save_iy = !iy_dead && source->type == AOP_IY && (result->type == AOP_REG && !via_a && !aopInReg (result, roffset + i, A_IDX));
              if (save_iy)
                _push (PAIR_IY);
              cheapMove (via_a ? ASMOP_A : result, via_a ? 0 : (roffset + i), source, soffset + i, via_a || a_dead);
              if (save_iy)
                _pop (PAIR_IY);
            }
          if (pushed_hl)
            _pop (PAIR_HL);
          if (via_a)
            {
              if (requiresHL (result) && result->type != AOP_REG && !hl_dead)
                {
                  if (result->type == AOP_HL)
                    {
                      emit2 ("ld !mems, a", aopGetLitWordLong (result, roffset + i, FALSE));
                      cost2 (3, 13, 13, 10, 16, 10, 4, 4);
                    }
                  else
                    {
                      _push (PAIR_HL);
                      cheapMove (result, roffset + i, ASMOP_A, 0, true);
                      _pop (PAIR_HL);
                    }
                }
              else
                cheapMove (result, roffset + i, ASMOP_A, 0, true);
              if (!a_dead)
                _pop (PAIR_AF);
            }
          zeroed_a = false;
        }

      i++;
    }
}

/*-----------------------------------------------------------------*/
/* genMove - Copy the value from one asmop to another              */
/*-----------------------------------------------------------------*/
static void
genMove (asmop *result, asmop *source, bool a_dead, bool hl_dead, bool de_dead, bool iy_dead)
{
  wassert (result);
  genMove_o (result, 0, source, 0, result->size, a_dead, hl_dead, de_dead, iy_dead, true);
}

/*--------------------------------------------------------------------------*/
/* adjustStack - Adjust the stack pointer by n bytes.                       */
/*--------------------------------------------------------------------------*/
static void
adjustStack (int n, bool af_free, bool bc_free, bool de_free, bool hl_free, bool iy_free)
{
  if(n != 0)
    emitDebug("; adjustStack by %d", n);
  _G.stack.pushed -= n;
  
  iy_free &= !IS_SM83;

  int loop_bytes, loop_cycles;
  if (abs(n) > 0 && (IS_RAB || IS_SM83)) // Assume sequence of add sp, #d
    {
      loop_bytes = (abs(n) / 127 + 1) * 2;
      loop_cycles = (abs(n) / 127 + 1) * (IS_RAB ? 4 : 16);
    }
  else if (n > 0 && (IS_Z80 || IS_Z80N || optimize.codeSize) && // Assume sequence of pop rr
    (!IS_TLCS90 && af_free || bc_free || de_free || hl_free || IS_TLCS90 && iy_free)) 
    {
      loop_bytes = n / 2 + n % 2;
      if (IS_RAB)
        loop_cycles = n / 2 * 7 + n % 2 * 2;
      else if (IS_SM83)
        loop_cycles = n / 2 * 12 + n % 2 * 8;
      else if (IS_R800)
        loop_cycles = n / 2 * 4 + n % 2;
      else // Z80
        loop_cycles = n / 2 * 10 + n % 2 * 6;
    }
  else // Assume sequence of inc / dec sp
    {
      loop_bytes = abs(n);
      if (IS_RAB)
        loop_cycles = abs(n) * 2;
      else if (IS_SM83)
        loop_cycles = abs(n) * 8;
      else if (IS_R800)
        loop_cycles = abs(n);
      else // Z80
        loop_cycles = abs(n) * 6;
    }

  if (IS_TLCS90 && abs(n) > (optimize.codeSize ? 2 + (bc_free || de_free || hl_free || iy_free || n < 0) * 2 : 1))
    {
      emit2 ("add sp, !immed%d", n);
      cost (3, 6);
      n -= n;
    }
  else if ((optimize.codeSpeed ?
    (loop_cycles >= (IS_RAB ? 10 : IS_SM83 ? 28 : 27)) :
    (loop_bytes >= 5)) &&
    hl_free)
    {
      emit2 ("ld hl, !immed%d", n);
      emit2 ("add hl, sp");
      emit2 ("ld sp, hl");
      spillPair (PAIR_HL);
      cost2 (5, 27, 20, 10, 28, 18, 4, 5);
      n -= n;
    }
  else if ((optimize.codeSpeed ?
    (loop_cycles >= (IS_RAB ? 14 : 35)) :
    (loop_bytes >= 7)) &&
    hl_free)
    {
      emit2 ("ex de, hl");
      emit2 ("ld hl, !immed%d", n);
      emit2 ("add hl, sp");
      emit2 ("ld sp, hl");
      emit2 ("ex de, hl");
      spillPair (PAIR_DE);
      cost2 (7, 35, 26, 14, 0, 22, 6, 7);
      n -= n;
    }
  else if ((optimize.codeSpeed ?
    (loop_cycles >= (IS_RAB ? 16 : 39)) :
    (loop_bytes >= (IS_TLCS90 ? 7 : 8))) &&
    iy_free)
    {
      emit2 ("ld iy, !immed%d", n);
      emit2 ("add iy, sp");
      emit2 ("ld sp, iy");
      spillPair (PAIR_IY);
      cost2 (IS_TLCS90 ? 7 : 8, 39, 26, 16, 0, 20, 8, 8);
      n -= n;
    }
  else if (loop_bytes >= 9 && bc_free)
    {
      emit3 (A_LD, ASMOP_C, ASMOP_L);
      emit3 (A_LD, ASMOP_B, ASMOP_H);
      emit2 ("ld hl, !immed%d", n);
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("add hl, sp");
      cost2 (1, 11, 7, 2, 8, 8, 1, 1);
      emit2 ("ld sp, hl");
      cost2 (1, 6, 4, 2, 8, 4, 1, 1);
      emit3 (A_LD, ASMOP_L, ASMOP_C);
      emit3 (A_LD, ASMOP_H, ASMOP_B);
      n -= n;
    }

  while (abs(n))
    {
      if ((IS_RAB && abs(n) > (optimize.codeSize ? 2 : 1) ) || (IS_SM83 && abs(n) > 2))
        { // on sm83 inc/dec is nicer for 2B because it touches no flags
          int d;
          if (n > 127)
            d = 127;
          else if (n < -128)
            d = -128;
          else
            d = n;
          emit2 ("add sp, !immed%d", d);
          cost (2, IS_SM83 ? 16 : 4);
          n -= d;
        }
      // on sm83 pop is smaller and faster, but that makes detection of uninitialized memory harder
      // On TLCS-90 pop af messes up interrupts (unless we have a valid value for f on the stack from previous push af).
      else if (!IS_SM83 && !IS_TLCS90 && n >= 2 && af_free && ((IS_Z80 || IS_Z80N) || optimize.codeSize))
        {
          emit2 ("pop af");
          cost2 (1, 10, 9, 7, 12, 10, 3, 3);
          n -= 2;
        }
      else if (!IS_SM83 && n <= -2 && ((IS_Z80 || IS_Z80N) || optimize.codeSize))
        {
          emit2 ("push af");
          cost2 (1, 10, 11, 7, 12, 10, 3, 4);
          n += 2;
        }
      else if (!IS_SM83 && n >= 2 && bc_free && ((IS_Z80 || IS_Z80N) || optimize.codeSize))
        {
          emit2 ("pop bc");
          cost2 (1, 10, 9, 7, 12, 10, 3, 3);
          n -= 2;
        }
      else if (!IS_SM83 && n >= 2 && de_free && ((IS_Z80 || IS_Z80N) || optimize.codeSize))
        {
          emit2 ("pop de");
          cost2 (1, 10, 9, 7, 12, 10, 3, 3);
          n -= 2;
        }
      else if (!IS_SM83 && n >= 2 && hl_free && ((IS_Z80 || IS_Z80N) || optimize.codeSize))
        {
          emit2 ("pop hl");
          cost2 (1, 10, 9, 7, 12, 10, 3, 3);
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
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          n--;
        }
      else if (n <= -1)
        {
          emit2 ("dec sp");
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          n++;
        }
    }

  wassert(!n);
}

/** Put Acc into a register set
 */
static void
outAcc (operand * result)
{
  int size = result->aop->size;
  if (size)
    {
      cheapMove (result->aop, 0, ASMOP_A, 0, true);
      size--;
      genMove_o (result->aop, 1, ASMOP_ZERO, 0, size, true, false, false, true, true);
    }
}

/** Take the value in carry and put it into a register
 */
static void
outBitC (operand *result)
{
  /* if the result is bit */
  if (result->aop->type == AOP_CRY)
    {
      if (!IS_OP_RUONLY (result) && !regalloc_dry_run)
        aopPut (result->aop, "c", 0);  // Todo: Cost.
    }
  else
    {
      emit3 (A_LD, ASMOP_A, ASMOP_ZERO);
      emit3 (A_RLA, 0, 0);
      outAcc (result);
    }
}

/*-----------------------------------------------------------------*/
/* toBoolean - emit code for or a,operator(sizeop)                 */
/*-----------------------------------------------------------------*/
static void
_toBoolean (const operand *oper, bool needflag)
{
  int size = oper->aop->size;
  sym_link *type = operandType (oper);
  int skipbyte;

  if (size == 1 && needflag)
    {
      cheapMove (ASMOP_A, 0, oper->aop, 0, true);
      emit3 (A_OR, ASMOP_A, ASMOP_A);
      return;
    }

  if (size == 2 && oper->aop->type == AOP_STL)
    {
      _push(PAIR_HL);
      genMove (ASMOP_HL, oper->aop, true, false, false, false);
      emit2 ("ld a, l");
      emit2 ("or a, h");
      _pop (PAIR_HL);
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
        UNIMPLEMENTED;
      emit2 ("res 7, a");   // clear sign bit
      cost2 (2, 8, 7, 4, 8, 4, 2, 2);
      skipbyte = size - 1;
    }
  while (size--)
    if (size != skipbyte)
      {
        if (!HAS_IYL_INST && (aopInReg (oper->aop, size, IYL_IDX) || aopInReg (oper->aop, size, IYH_IDX)))
          UNIMPLEMENTED;
        else
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
  if (right->aop->size == 1 && !aopInReg (right->aop, 0, A_IDX))
    {
      emit3 (A_XOR, ASMOP_A, ASMOP_A);
      if (!HAS_IYL_INST && (aopInReg (right->aop, 0, IYL_IDX) || aopInReg (right->aop, 0, IYH_IDX)))
        UNIMPLEMENTED;
      else
        emit3 (A_CP, ASMOP_A, right->aop);
    }
  else
    {
      _toBoolean (right, FALSE);
      emit2 ("add a, !immedbyte", 0xffu);
      cost2	(2, 7, 6, 4, 8, 4, 2, 2);
      emit3 (A_LD, ASMOP_A, ASMOP_ZERO);
    }
  emit3 (A_RLA, 0, 0);
}

/* Shuffle src reg array into dst reg array. */
static void
regMove (const short *dst, const short *src, size_t n, bool preserve_a) // Todo: replace uses of this one by uses of genMove_o?
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
          emit3w (A_EX, ASMOP_DE, ASMOP_HL);
          swapPairs (PAIR_DE, PAIR_HL);
          assigned[ex[0]] = true;
          assigned[ex[1]] = true;
          assigned[ex[2]] = true;
          assigned[ex[3]] = true;
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

      wassertl (i != n, "regMove error: Trying to cache non-existent byte in accumulator.");
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
  if (left->aop->type == AOP_CRY)
    {
      wassertl (0, "Tried to negate a bit");
    }
  else if (IS_BOOL (operandType (left)))
    {
      cheapMove (ASMOP_A, 0, left->aop, 0, true);
      emit3 (A_XOR, ASMOP_A, ASMOP_ONE);
      cheapMove (result->aop, 0, ASMOP_A, 0, true);
      goto release;
    }
  else if (IS_RAB && left->aop->size == 2 && aopInReg (left->aop, 0, HL_IDX) && isPairDead (PAIR_HL, ic) && aopInReg (result->aop, 0, L_IDX))
    {
      emit2 ("bool hl");
      emit2 ("rr hl");
      emit2 ("ccf");
      emit2 ("adc hl, hl");
      cost (5, 10);
      goto release;
    }
  else if (IS_RAB && left->aop->size == 2 && aopInReg (left->aop, 0, HL_IDX) && isPairDead (PAIR_HL, ic))
    {
      emit2 ("bool hl");
      emit2 ("ld a, l");
      emit2 ("xor a, #0x01");
      cost (4, 8);
      cheapMove (result->aop, 0, ASMOP_A, 0, true);
      goto release;
    }

  _toBoolean (left, FALSE);

  /* Not of A:
     If A == 0, !A = 1
     else A = 0
     So if A = 0, A-1 = 0xFF and C is set, rotate C into reg. */
  emit3 (A_SUB, ASMOP_A, ASMOP_ONE);
  outBitC (result);

release:
  /* release the aops */
  freeAsmop (left, NULL);
  freeAsmop (result, NULL);
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

  wassertl (IS_SM83, "Code is only relevant to the gbz80");
  wassertl (IC_RESULT (ic)->aop->size == 4, "Only works for four bytes");

  fetchPair (PAIR_DE, left);

  emit3 (A_LD, ASMOP_A, ASMOP_E);
  emit3_o (first, ASMOP_A, 0, right, LSB);
  emit2 ("ld e, a");
  emit3 (A_LD, ASMOP_E, ASMOP_A);
  emit3 (A_LD, ASMOP_A, ASMOP_D);
  emit3_o (later, ASMOP_A, 0, right, MSB16);

  _push (PAIR_AF);

  cheapMove (IC_RESULT (ic)->aop, MSB16, ASMOP_A, 0, true);
  cheapMove (IC_RESULT (ic)->aop, LSB, ASMOP_E, 0, true);

  fetchPairLong (PAIR_DE, left, NULL, MSB24);

  if (!regalloc_dry_run)
    aopGet (right, MSB24, FALSE);

  _pop (PAIR_AF);
  emit3 (A_LD, ASMOP_A, ASMOP_E);
  emit3_o (later, ASMOP_A, 0, right, MSB24);
  emit3 (A_LD, ASMOP_E, ASMOP_A);
  emit3 (A_LD, ASMOP_A, ASMOP_D);
  emit3_o (later, ASMOP_A, 0, right, MSB32);

  cheapMove (IC_RESULT (ic)->aop, MSB32, ASMOP_A, 0, true);
  cheapMove (IC_RESULT (ic)->aop, MSB24, ASMOP_E, 0, true);
}

static void
_gbz80_emitAddSubLong (const iCode * ic, bool isAdd)
{
  _gbz80_emitAddSubLongLong (ic, IC_LEFT (ic)->aop, IC_RIGHT (ic)->aop, isAdd);
}

/* Pop saved regs from stack, taking care not to destroy result */
static void
restoreRegs (bool iy, bool de, bool bc, bool hl, const operand *result, const iCode *const ic)
{
  bool a_live, b_live, c_live, d_live, e_live, h_live, l_live, iyl_live, iyh_live;
  bool SomethingReturned;

  SomethingReturned = result && IS_ITEMP (result) && (OP_SYMBOL_CONST (result)->nRegs || OP_SYMBOL_CONST (result)->spildir)
                      || IS_TRUE_SYMOP (result);

  if (SomethingReturned)
    {
      bitVect *rv = z80_rUmaskForOp (result);
      a_live = bitVectBitValue (rv, A_IDX);
      b_live = bitVectBitValue (rv, B_IDX);
      c_live = bitVectBitValue (rv, C_IDX);
      d_live = bitVectBitValue (rv, D_IDX);
      e_live = bitVectBitValue (rv, E_IDX);
      h_live = bitVectBitValue (rv, H_IDX);
      l_live = bitVectBitValue (rv, L_IDX);
      iyh_live = bitVectBitValue (rv, IYH_IDX);
      iyl_live = bitVectBitValue (rv, IYL_IDX);
      freeBitVect (rv);
    }
  else
    {
      a_live = false;
      b_live = false;
      c_live = false;
      d_live = false;
      e_live = false;
      h_live = false;
      l_live = false;
      iyh_live = false;
      iyl_live = false;
    }

  if (ic)
    {
      if (!isRegDead (A_IDX, ic))
        a_live = true;
      if (!de && !isRegDead (D_IDX, ic))
        d_live = true;
      if (!de && !isRegDead (E_IDX, ic))
        e_live = true;
      if (!bc && !isRegDead (B_IDX, ic))
        b_live = true;
      if (!bc && !isRegDead (C_IDX, ic))
        c_live = true;
      if (!hl && !isRegDead (H_IDX, ic))
        h_live = true;
      if (!hl && !isRegDead (L_IDX, ic))
        l_live = true;
      if (!iy && !isRegDead (IYH_IDX, ic))
        iyh_live = true;
      if (!iy && !isRegDead (IYL_IDX, ic))
        iyl_live = true;
    }

  if (iy)
    {
      if (iyh_live && iyl_live)
        wassertl (0, "Shouldn't push IY if it's wiped out by the return");
      else if (iyh_live)
        poppairwithsavedreg (PAIR_IY, IYH_IDX, -1);
      else if (iyl_live)
        poppairwithsavedreg (PAIR_IY, IYL_IDX, -1);
      else
        _pop (PAIR_IY);
    }

  if (de)
    {
      if (d_live && e_live)
        wassertl (0, "Shouldn't push DE if it's wiped out by the return");
      else if (d_live && !a_live)
        poppairwithsavedreg (PAIR_DE, D_IDX, A_IDX);
      else if (d_live && !h_live)
        poppairwithsavedreg (PAIR_DE, D_IDX, H_IDX);
      else if (d_live && !b_live)
        poppairwithsavedreg (PAIR_DE, D_IDX, B_IDX);
      else if (d_live)
        poppairwithsavedreg (PAIR_DE, D_IDX, -1);
      else if (e_live && !a_live && !IS_TLCS90) // TLCS-90 has interrupt settings in f, so we can't pop af unless we did push af before.
        {
          /* Only restore D */
          _pop (PAIR_AF);
          emit2 ("ld d, a");
          cost2 (1, 4, 4, 2, 4, 2, 1, 1);
        }
      else if (e_live && !a_live)
        poppairwithsavedreg (PAIR_DE, E_IDX, A_IDX);
      else if (e_live && !l_live)
        poppairwithsavedreg (PAIR_DE, E_IDX, L_IDX);
      else if (e_live && !c_live)
        poppairwithsavedreg (PAIR_DE, E_IDX, C_IDX);
      else if (e_live)
        poppairwithsavedreg (PAIR_DE, E_IDX, -1);
      else
        _pop (PAIR_DE);
    }

  if (bc)
    {
      if (b_live && c_live)
        wassertl (0, "Shouldn't push BC if it's wiped out by the return");
      else if (b_live && !a_live)
        poppairwithsavedreg (PAIR_BC, B_IDX, A_IDX);
      else if (b_live && !h_live)
        poppairwithsavedreg (PAIR_BC, B_IDX, H_IDX);
      else if (b_live && !l_live)
        poppairwithsavedreg (PAIR_BC, B_IDX, L_IDX);
      else if (b_live)
        poppairwithsavedreg (PAIR_BC, B_IDX, -1);
      else if (c_live && !a_live && !IS_TLCS90)
        {
          /* Only restore B */
          _pop (PAIR_AF);
          emit2 ("ld b, a");
          cost2 (1, 4, 4, 2, 4, 2, 1, 1);
        }
      else if (c_live && !a_live)
        poppairwithsavedreg (PAIR_BC, C_IDX, A_IDX);
      else if (c_live && !l_live)
        poppairwithsavedreg (PAIR_BC, C_IDX, L_IDX);
      else if (c_live && !h_live)
        poppairwithsavedreg (PAIR_BC, C_IDX, H_IDX);
      else if (c_live)
        poppairwithsavedreg (PAIR_BC, C_IDX, -1);
      else
        _pop (PAIR_BC);
    }

  if (hl)
    {
      if (h_live && l_live)
        wassertl (0, "Shouldn't push HL if it's wiped out by the return");
      else if (h_live && !a_live)
        poppairwithsavedreg (PAIR_HL, H_IDX, A_IDX);
      else if (h_live && !bc && !b_live)
        poppairwithsavedreg (PAIR_HL, H_IDX, B_IDX);
      else if (h_live && !de && !d_live)
        poppairwithsavedreg (PAIR_HL, H_IDX, D_IDX);
      else if (h_live && !bc && !c_live)
        poppairwithsavedreg (PAIR_HL, H_IDX, C_IDX);
      else if (h_live && !de && !e_live)
        poppairwithsavedreg (PAIR_HL, H_IDX, E_IDX);
      else if (h_live)
        poppairwithsavedreg (PAIR_HL, H_IDX, -1);
      else if (l_live && !a_live && !IS_TLCS90)
        {
          /* Only restore H */
          _pop (PAIR_AF);
          emit2 ("ld h, a");
          cost2 (1, 4, 4, 2, 4, 2, 1, 1);
        }
      else if (l_live&& !a_live )
        poppairwithsavedreg (PAIR_HL, L_IDX, A_IDX);
      else if (l_live && !bc && !c_live )
        poppairwithsavedreg (PAIR_HL, L_IDX, C_IDX);
      else if (l_live && !de && !e_live )
        poppairwithsavedreg (PAIR_HL, L_IDX, E_IDX);
      else if (l_live && !bc && !b_live )
        poppairwithsavedreg (PAIR_HL, L_IDX, B_IDX);
      else if (l_live && !de && !d_live )
        poppairwithsavedreg (PAIR_HL, L_IDX, D_IDX);
      else if (l_live)
        poppairwithsavedreg (PAIR_HL, L_IDX, -1);
      else
        _pop (PAIR_HL);
    }
}

static void
_saveRegsForCall (const iCode *ic, bool saveHLifused, bool dontsaveIY)
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

  if (IS_FUNCPTR (dtype))
    saveHLifused = true;
  if (!_G.saves.saved)
    {
      const bool call_preserves_b = ftype->funcAttrs.preserved_regs[B_IDX] && !z80IsParmInCall(ftype, "b");
      const bool call_preserves_c = ftype->funcAttrs.preserved_regs[C_IDX] && !z80IsParmInCall(ftype, "c");
      const bool call_preserves_d = ftype->funcAttrs.preserved_regs[D_IDX] && !z80IsParmInCall(ftype, "d");
      const bool call_preserves_e = ftype->funcAttrs.preserved_regs[E_IDX] && !z80IsParmInCall(ftype, "e");
      const bool call_preserves_h = ftype->funcAttrs.preserved_regs[H_IDX] && !z80IsParmInCall(ftype, "h");
      const bool call_preserves_l = ftype->funcAttrs.preserved_regs[L_IDX] && !z80IsParmInCall(ftype, "l");
      const bool push_bc = !isRegDead (B_IDX, ic) && !call_preserves_b || !isRegDead (C_IDX, ic) && !call_preserves_c;
      const bool push_de = !isRegDead (D_IDX, ic) && !call_preserves_d || !isRegDead (E_IDX, ic) && !call_preserves_e;
      const bool push_hl = !isRegDead (H_IDX, ic) && (!call_preserves_h || saveHLifused) || !isRegDead (L_IDX, ic) && (!call_preserves_l || saveHLifused);
      const bool push_iy = !dontsaveIY && (!isRegDead (IYH_IDX, ic) || !isRegDead (IYL_IDX, ic));

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
    _saveRegsForCall (walk, true, false); /* Caller saves, and this is the first iPush. */

  sym_link *ftype = operandType (IC_LEFT (walk));
  if (walk->op == PCALL)
    ftype = ftype->next;
  const bool smallc = IFFUNC_ISSMALLC (ftype);

  /* then do the push */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);

  int size = IC_LEFT (ic)->aop->size;

  if (size == 1 && smallc) /* The SmallC calling convention pushes 8-bit parameters as 16-bit values. */
    {
      if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == C_IDX)
        {
          emit2 ("push bc");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
        }
      else if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX)
        {
          emit2 ("push de");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
        }
      else if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == L_IDX)
        {
          emit2 ("push hl");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
        }
      else if (isRegDead (HL_IDX, ic))
        {
          cheapMove (ASMOP_L, 0, IC_LEFT (ic)->aop, 0, true);
          emit2 ("push hl");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
        }
      else if (isRegDead (A_IDX, ic))
        {
          emit2 ("dec sp");
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          cheapMove (ASMOP_A, 0, IC_LEFT (ic)->aop, 0, true);
          emit2 ("push af");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
          emit2 ("inc sp");
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
        }
      else if (!IS_SM83)
        {
          emit2 ("push hl");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
          cheapMove (ASMOP_L, 0, IC_LEFT (ic)->aop, 0, false);
          emit2 ("ex (sp), hl");
          cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
          spillPair (PAIR_HL);
        }
      else
        wassert (0);

      if (!regalloc_dry_run)
        _G.stack.pushed += 2;
      goto release;
    }

    while (size)
      {
        int d = 0;

        bool a_free = isRegDead (A_IDX, ic) && (IC_LEFT (ic)->aop->regs[A_IDX] < 0 || IC_LEFT (ic)->aop->regs[A_IDX] >= size - 1);
        bool b_free = isRegDead (B_IDX, ic) && (IC_LEFT (ic)->aop->regs[B_IDX] < 0 || IC_LEFT (ic)->aop->regs[B_IDX] >= size - 1);
        bool c_free = isRegDead (C_IDX, ic) && (IC_LEFT (ic)->aop->regs[C_IDX] < 0 || IC_LEFT (ic)->aop->regs[C_IDX] >= size - 1);
        bool d_free = isRegDead (D_IDX, ic) && (IC_LEFT (ic)->aop->regs[D_IDX] < 0 || IC_LEFT (ic)->aop->regs[D_IDX] >= size - 1);
        bool e_free = isRegDead (E_IDX, ic) && (IC_LEFT (ic)->aop->regs[E_IDX] < 0 || IC_LEFT (ic)->aop->regs[E_IDX] >= size - 1);
        bool h_free = isRegDead (H_IDX, ic) && (IC_LEFT (ic)->aop->regs[H_IDX] < 0 || IC_LEFT (ic)->aop->regs[H_IDX] >= size - 1);
        bool l_free = isRegDead (L_IDX, ic) && (IC_LEFT (ic)->aop->regs[L_IDX] < 0 || IC_LEFT (ic)->aop->regs[L_IDX] >= size - 1);
        bool iyh_free = isRegDead (IYH_IDX, ic) && (IC_LEFT (ic)->aop->regs[IYH_IDX] < 0 || IC_LEFT (ic)->aop->regs[IYH_IDX] >= size - 1);
        bool iyl_free = isRegDead (IYL_IDX, ic) && (IC_LEFT (ic)->aop->regs[IYL_IDX] < 0 || IC_LEFT (ic)->aop->regs[IYL_IDX] >= size - 1);
        bool hl_free = isPairDead (PAIR_HL, ic) && (h_free || IC_LEFT (ic)->aop->regs[H_IDX] >= size - 2) && (l_free || IC_LEFT (ic)->aop->regs[L_IDX] >= size - 2);
        bool de_free = isPairDead (PAIR_DE, ic) && (d_free || IC_LEFT (ic)->aop->regs[D_IDX] >= size - 2) && (e_free || IC_LEFT (ic)->aop->regs[E_IDX] >= size - 2);
        bool bc_free = isPairDead (PAIR_BC, ic) && (b_free || IC_LEFT (ic)->aop->regs[B_IDX] >= size - 2) && (c_free || IC_LEFT (ic)->aop->regs[C_IDX] >= size - 2);

        if (getPairId_o (IC_LEFT (ic)->aop, size - 2) != PAIR_INVALID)
          {
            emit2 ("push %s", _pairs[getPairId_o (IC_LEFT (ic)->aop, size - 2)].name);
            if (getPairId_o (IC_LEFT (ic)->aop, size - 2) == PAIR_IY)
              cost2 (2 - IS_TLCS90, 15, 13, 12, 0, 8, 4, 5);
            else
              cost2 (1, 11, 11, 10, 16, 8, 3, 4);
            d = 2;
          }
#if 0 // Fails regression tests. Simulator issue regarding flags?
        // gbz80 flag handling differs from other z80 variants, allowing this hack to push a 16-bit zero.
        else if (size >= 2 && IS_SM83 && a_free &&
          IC_LEFT (ic)->aop->type == AOP_LIT && !byteOfVal (IC_LEFT(ic)->aop->aopu.aop_lit, size - 2) && !byteOfVal (IC_LEFT(ic)->aop->aopu.aop_lit, size - 1))
          {
            emit2 ("xor a, a"); // Resets all flags except for z
            emit2 ("rra");      // Resets z (and since a is already 0, c stays reset)
            emit2 ("push af");  // Pushes 0 in a, 0 for the unused upper 4 flag bits, 4 flag bits that are all reset, giving us a 16-bit 0 push.
            regalloc_dry_run_cost += 3;
            d = 2;
          }
#endif
        else if (size >= 2 &&
          (hl_free || de_free || bc_free ||
          aopInReg (IC_LEFT (ic)->aop, size - 1, B_IDX) && c_free || b_free && aopInReg (IC_LEFT (ic)->aop, size - 2, C_IDX) ||
          aopInReg (IC_LEFT (ic)->aop, size - 1, D_IDX) && e_free || d_free && aopInReg (IC_LEFT (ic)->aop, size - 2, E_IDX) ||
          aopInReg (IC_LEFT (ic)->aop, size - 1, H_IDX) && l_free || h_free && aopInReg (IC_LEFT (ic)->aop, size - 2, L_IDX)))
          {
            asmop *pair = 0;
            
            /* hl has lower priority on GB, because it's needed for stack access */
            if (!IS_SM83 && hl_free)
              pair = ASMOP_HL;
            else if (de_free)
              pair = ASMOP_DE;
            else if (bc_free)
              pair = ASMOP_BC;
            else if (hl_free)
              pair = ASMOP_HL;
              
            if (IS_SM83 && requiresHL (IC_LEFT (ic)->aop) && IC_LEFT (ic)->aop->type != AOP_REG && de_free)
              pair = ASMOP_DE;
            else if (IS_SM83 && requiresHL (IC_LEFT (ic)->aop) && IC_LEFT (ic)->aop->type != AOP_REG && bc_free)
              pair = ASMOP_BC;

            if (aopInReg (IC_LEFT (ic)->aop, size - 1, H_IDX) && l_free || h_free && aopInReg (IC_LEFT (ic)->aop, size - 2, L_IDX))
              pair = ASMOP_HL;
            else if (aopInReg (IC_LEFT (ic)->aop, size - 1, D_IDX) && e_free || d_free && aopInReg (IC_LEFT (ic)->aop, size - 2, E_IDX))
              pair = ASMOP_DE;
            else if (aopInReg (IC_LEFT (ic)->aop, size - 1, B_IDX) && c_free || b_free && aopInReg (IC_LEFT (ic)->aop, size - 2, C_IDX))
              pair = ASMOP_BC;
            
            genMove_o (pair, 0, IC_LEFT (ic)->aop, size - 2, 2, a_free, hl_free, de_free, true, true);
            emit2 ("push %s", _pairs[getPairId (pair)].name);
            cost2 (1, 11, 11, 10, 16, 8, 3, 4);
            d = 2;

            // For hl and iy, genMove_o can do better caching of literal values than what we do here. TODO: Remove this, and make genMove_o handle cahing well for bc and de, too (will require quite some spillPair() calls througout codegen).
            while (getPairId (pair) != PAIR_HL && IC_LEFT (ic)->aop->type == AOP_LIT && !IS_FLOAT (IC_LEFT (ic)->aop->aopu.aop_lit->type) && size - (d+2) >= 0)
              {
                unsigned long current = (ullFromVal(IC_LEFT (ic)->aop->aopu.aop_lit)>>((size - d    )*8)) & 0xFFFF;
                unsigned long next = (ullFromVal(IC_LEFT (ic)->aop->aopu.aop_lit)>>((size - (d+2))*8)) & 0xFFFF;
                if (current == next)
                  {
                    emitDebug ("; genIpush identical value again");
                    emit2 ("push %s", _pairs[getPairId (pair)].name);
                    cost2 (1, 11, 11, 10, 16, 8, 3, 4);
                    d+=2;
                  }
                else if ((current & 0xFF) == (next & 0xFF))
                  {
                    emitDebug ("; genIpush similar value again");
                    emit2 ("ld %s, !immedbyte", _pairs[getPairId (pair)].h, (unsigned)(next >> 8));
                    cost2 (2, 7, 6, 4, 8, 4, 2, 2);
                    emit2 ("push %s", _pairs[getPairId (pair)].name);
                    cost2 (1, 11, 11, 10, 16, 8, 3, 4);
                    d+=2;
                  }
                else if ((current & 0xFF00) == (next & 0xFF00))
                  {
                    emitDebug ("; genIpush similar value again");
                    emit2 ("ld %s, !immedbyte", _pairs[getPairId (pair)].l, (unsigned)(next & 0xffu));
                    cost2 (2, 7, 6, 4, 8, 4, 2, 2);
                    emit2 ("push %s", _pairs[getPairId (pair)].name);
                    cost2 (1, 11, 11, 10, 16, 8, 3, 4);
                    d+=2;
                  }
                else
                  break;
              }
         }
       else if (size >= 2 && IS_Z80N && (IC_LEFT (ic)->aop->type == AOP_LIT || IC_LEFT (ic)->aop->type == AOP_IMMD)) // Same size, but slower (21 vs 23 cycles) than going through a register pair other than iy. Only worth it under high register pressure.
         {
           emit2 ("push !hashedstr", aopGetLitWordLong (IC_LEFT (ic)->aop, size - 2, false));
           cost (4, 23);
           d = 2;
         }
       else if (size >= 2 && !IS_SM83 && !IY_RESERVED && isPairDead (PAIR_IY, ic) && (IC_LEFT (ic)->aop->type == AOP_LIT || IC_LEFT (ic)->aop->type == AOP_IMMD))
         {
           genMove_o (ASMOP_IY, 0, IC_LEFT (ic)->aop, size - 2, 2, a_free, hl_free, de_free, true, true);
           emit2 ("push iy");
           cost2 (2, 1, 13, 12, 0, 8, 4, 5);
           d = 2;
         }
       else if (size >= 2 && !IS_SM83)
         {
           emit2 ("push hl");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           _G.stack.pushed += 2;
           genMove_o (ASMOP_HL, 0, IC_LEFT (ic)->aop, size - 2, 2, a_free, hl_free, de_free, true, true);
           _G.stack.pushed -= 2;
           emit2 ("ex (sp), hl");
           cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
           spillPair (PAIR_HL);
           d = 2;
         }
       else if (aopInReg (IC_LEFT (ic)->aop, size - 1, A_IDX))
         {
           emit2 ("push af");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (aopInReg (IC_LEFT (ic)->aop, size - 1, B_IDX))
         {
           emit2 ("push bc");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (aopInReg (IC_LEFT (ic)->aop, size - 1, D_IDX))
         {
           emit2 ("push de");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (aopInReg (IC_LEFT (ic)->aop, size - 1, H_IDX))
         {
           emit2 ("push hl");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (aopInReg (IC_LEFT (ic)->aop, size - 1, IYH_IDX))
         {
           emit2 ("push iy");
           cost2 (2, 1, 13, 12, 0, 8, 4, 5);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (a_free)
         {
           genMove_o (ASMOP_A, 0, IC_LEFT (ic)->aop, size - 1, 1, a_free, h_free && l_free, d_free && e_free, iyh_free && iyl_free, true);
           emit2 ("push af");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (h_free)
         {
           genMove_o (ASMOP_H, 0, IC_LEFT (ic)->aop, size - 1, 1, a_free, h_free && l_free, d_free && e_free, iyh_free && iyl_free, true);
           emit2 ("push hl");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (d_free)
         {
           genMove_o (ASMOP_D, 0, IC_LEFT (ic)->aop, size - 1, 1, a_free, h_free && l_free, d_free && e_free, iyh_free && iyl_free, true);
           emit2 ("push de");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (b_free)
         {
           genMove_o (ASMOP_B, 0, IC_LEFT (ic)->aop, size - 1, 1, a_free, h_free && l_free, d_free && e_free, iyh_free && iyl_free, true);
           emit2 ("push bc");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
      else if (IS_Z80N && IC_LEFT (ic)->aop->type == AOP_LIT)
         {
           emit2 ("push !immedword", (unsigned)(byteOfVal (IC_LEFT (ic)->aop->aopu.aop_lit, size - 1) << 8));
           cost (4, 23);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else if (!IS_SM83)
         {
           emit2 ("push hl");
           cost2 (1, 11, 11, 10, 16, 8, 3, 4);
           genMove_o (ASMOP_H, 0, IC_LEFT (ic)->aop, size - 1, 1, a_free, h_free && l_free, d_free && e_free, iyh_free && iyl_free, true);
           emit2 ("ex (sp), hl");
           cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
           spillPair (PAIR_HL);
           emit2 ("inc sp");
           cost2 (1, 6, 4, 2, 8, 4, 1, 1);
           d = 1;
         }
       else
         wassert (0);

       if (!regalloc_dry_run)
          _G.stack.pushed += d;
       size -= d;
     }

release:
  freeAsmop (IC_LEFT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genPointerPush - generate code for pushing                      */
/*-----------------------------------------------------------------*/
static void
genPointerPush (const iCode *ic)
{
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
  if (!regalloc_dry_run && !_G.saves.saved) /* Cost is counted at CALL or PCALL instead */
    _saveRegsForCall (walk, true, false); /* Caller saves, and this is the first iPush. */

  sym_link *ftype = operandType (IC_LEFT (walk));
  if (walk->op == PCALL)
    ftype = ftype->next;
  const bool smallc = IFFUNC_ISSMALLC (ftype);

  /* then do the push */
  aopOp (IC_LEFT (ic), ic, false, false);

  wassertl (IC_RIGHT (ic), "IPUSH_VALUE_AT_ADDRESS without right operand");
  wassertl (IS_OP_LITERAL (IC_RIGHT (ic)), "IPUSH_VALUE_AT_ADDRESS with non-literal right operand");

  int offset = operandLitValue (IC_RIGHT(ic));

  wassert (!offset);
  wassert (!smallc);

  if (!isRegDead (HL_IDX, ic) && !(isRegDead (DE_IDX, ic) && !IS_SM83) || !isRegDead (A_IDX, ic))
    UNIMPLEMENTED;

  bool swap_de = !isRegDead (HL_IDX, ic);

  if (swap_de)
    emit3w (A_EX, ASMOP_DE, ASMOP_HL);

  genMove (ASMOP_HL, IC_LEFT (ic)->aop, true, true, swap_de ? false : isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic));

  int size = getSize (operandType (ic->left)->next);
  if (TARGET_IS_TLCS90 && size >= (optimize.codeSpeed? 3 : 4))
    {
      emit2 ("add hl, !immed%d", size - 1);
      cost (3, 6);
    }
  else if (isRegDead (BC_IDX, ic) && size > 5)
    {
      emit2 ("ld bc, !immed%d", size - 1);
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("add hl, bc");
      cost2 (1, 11, 7, 2, 8, 8, 1, 1);
    }
  else
    for(int i = 1; i < size; i++)
      emit3w (A_INC, ASMOP_HL, 0);

  for(int i = 0; i < size;)
    {
      if (i + 1 < size && isRegDead (BC_IDX, ic))
        {
          emit2 ("ld b, !*hl");
          cost2 (1, 7, 6, 6, 8, 6, 2, 2);
          emit2 ("dec hl");
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          emit2 ("ld c, !*hl");
          cost2 (1, 7, 6, 6, 8, 6, 2, 2);
          emit2 ("push bc");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
          _G.stack.pushed += 2;
          i += 2;
        }
      else
        {
          emit2 ("ld a, !*hl");
          cost2 (1, 7, 6, 6, 8, 6, 2, 2);
          emit2 ("push af");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
          emit2 ("inc sp");
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          if (!regalloc_dry_run)
            _G.stack.pushed++;
          i++;
        }

      if (i < size) // Both to save an instruction on the last byte, and to ensure we get the correct value as cached for hl.
        {
          emit2 ("dec hl");
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
        }
    }

  if (swap_de)
    emit3w (A_EX, ASMOP_DE, ASMOP_HL);

  freeAsmop (IC_LEFT (ic), 0);
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
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  
  /* Caller saves, and this is the first push/send. */
  // Scan ahead until we find the function that we are pushing/sending parameters to.
  const iCode *walk;
  for (walk = ic->next; walk; walk = walk->next)
    {
      if (walk->op == CALL || walk->op == PCALL)
        break;
    }

  if (!_G.saves.saved && !regalloc_dry_run) // Cost is counted at CALL or PCALL instead
    _saveRegsForCall (walk, requiresHL (ic->left->aop) || ic->next->op != CALL, false);

  sym_link *ftype = IS_FUNCPTR (operandType (IC_LEFT (walk))) ? operandType (IC_LEFT (walk))->next : operandType (IC_LEFT (walk));
  asmop *argreg = aopArg (ftype, ic->argreg);
  
  wassert (argreg);

  // The register argument shall not overwrite a still-needed (i.e. as further parameter or function for the call) value.
  if (!aopSame (argreg, 0, ic->left->aop, 0, argreg->size))
    for (int i = 0; i < argreg->size; i++)
      if (!isRegDead (argreg->aopu.aop_reg[i]->rIdx, ic))
        for (iCode *walk2 = ic->next; walk2; walk2 = walk2->next)
          {
            if (walk2->op != CALL && walk2->left && IS_ITEMP (walk2->left))
              UNIMPLEMENTED;

            if (walk2->op == CALL || walk2->op == PCALL)
              break;
          }

  bool a_dead = isRegDead (A_IDX, ic);
  bool hl_dead = isPairDead (PAIR_HL, ic);
  bool de_dead = isPairDead (PAIR_DE, ic);
  
  for (iCode *walk2 = ic->prev; walk2 && walk2->op == SEND; walk2 = walk2->prev)
    {
      asmop *warg = aopArg (ftype, walk2->argreg);
      wassert (warg);
      a_dead &= (warg->regs[A_IDX] < 0);
      hl_dead &= (warg->regs[L_IDX] < 0 && warg->regs[H_IDX] < 0);
      de_dead &= (warg->regs[E_IDX] < 0 && warg->regs[D_IDX] < 0);
    }

  genMove (argreg, IC_LEFT (ic)->aop, a_dead, hl_dead, de_dead, true);
  
  for (int i = 0; i < IC_LEFT (ic)->aop->size; i++)
    if (!regalloc_dry_run)
      z80_regs_used_as_parms_in_calls_from_current_function[argreg->aopu.aop_reg[i]->rIdx] = true;

  freeAsmop (IC_LEFT (ic), NULL);
}

static void
genCall (const iCode *ic)
{
  sym_link *dtype = operandType (IC_LEFT (ic));
  sym_link *etype = getSpec (dtype);
  sym_link *ftype = IS_FUNCPTR (dtype) ? dtype->next : dtype;
  int i;
  int prestackadjust = 0;
  bool tailjump = false;

  for (i = 0; i < IYH_IDX + 1; i++)
    z80_regs_preserved_in_calls_from_current_function[i] |= ftype->funcAttrs.preserved_regs[i];

  _saveRegsForCall (ic, false, false);

  aopOp (IC_LEFT (ic), ic, false, false);

  const bool bigreturn = (getSize (ftype->next) > 4) || IS_STRUCT (ftype->next); // Return value of big type or returning struct or union.
  const bool SomethingReturned = IS_ITEMP (IC_RESULT (ic)) && (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir) ||
                       IS_TRUE_SYMOP (IC_RESULT (ic));

  bool a_not_parm = !z80IsParmInCall(ftype, "a");
  bool a_free = a_not_parm && ic->left->aop->regs[A_IDX] < 0;
  bool hl_not_parm = !z80IsParmInCall(ftype, "l") && !z80IsParmInCall(ftype, "h");
  bool hl_free = hl_not_parm && ic->left->aop->regs[L_IDX] < 0 && ic->left->aop->regs[H_IDX] < 0;
  bool de_not_parm = !z80IsParmInCall(ftype, "e") && !z80IsParmInCall(ftype, "d");
  bool de_free = de_not_parm && ic->left->aop->regs[E_IDX] < 0 && ic->left->aop->regs[D_IDX] < 0;
  bool bc_not_parm = !z80IsParmInCall(ftype, "b") && !z80IsParmInCall(ftype, "c");
  bool bc_free = bc_not_parm && ic->left->aop->regs[C_IDX] < 0 && ic->left->aop->regs[B_IDX] < 0;

  
  if (SomethingReturned && !bigreturn)
    aopOp (IC_RESULT (ic), ic, true, false);

  if (bigreturn)
    {
      PAIR_ID pair;
      int fp_offset, sp_offset;

      if (ic->op == PCALL && IS_SM83 || !hl_free)
        _push (PAIR_HL);
      aopOp (IC_RESULT (ic), ic, true, false);
      wassert (IC_RESULT (ic)->aop->type == AOP_STK || IC_RESULT (ic)->aop->type == AOP_EXSTK);
      fp_offset =
        IC_RESULT (ic)->aop->aopu.aop_stk + (IC_RESULT (ic)->aop->aopu.aop_stk >
            0 ? _G.stack.param_offset : 0);
      sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
      pair = (ic->op == PCALL && !IS_SM83 && !IY_RESERVED) ? PAIR_IY : PAIR_HL;
      if (IS_SM83 && sp_offset <= 127 && sp_offset >= -128)
        {
          emit2 ("!ldahlsp", sp_offset);
          cost (2, 12);
        }
      else
        {
          emit2 ("ld %s, !immedword", _pairs[pair].name, (unsigned)sp_offset);
          if (pair == PAIR_IY)
            cost2 (4, 14, 12, 8, 0, 6, 4, 4);
          else
            cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          emit2 ("add %s, sp", _pairs[pair].name);
          if (pair == PAIR_IY)
            cost2 (2, 15, 10, 4, 0, 8, 2, 2);
          else
            cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
        }
      if (ic->op == PCALL && IS_SM83 || !hl_free)
        {
          if (de_free)
            {
              emit3 (A_LD, ASMOP_E, ASMOP_L);
              emit3 (A_LD, ASMOP_D, ASMOP_H);
              _pop (PAIR_HL);
              pair = PAIR_DE;
            }
          else
            {
              wassert (bc_free);
              emit3 (A_LD, ASMOP_C, ASMOP_L);
              emit3 (A_LD, ASMOP_B, ASMOP_H);
              _pop (PAIR_HL);
              pair = PAIR_BC;           
            }
        }
      emit2 ("push %s", _pairs[pair].name);
      if (pair == PAIR_IY)
        cost2 (2, 15, 13, 12, 0, 8, 4, 5);
      else
        cost2 (1, 11, 11, 10, 16, 8, 3, 4);
      if (!regalloc_dry_run)
        _G.stack.pushed += 2;
      freeAsmop (IC_RESULT (ic), 0);
      hl_free = false;
    }

  // Check if we can do tail call optimization.
  else if (currFunc && !IFFUNC_ISISR (currFunc->type) &&
    !ic->parmBytes &&
    !_G.stack.pushedHL && !_G.stack.pushedBC && !_G.stack.pushedDE && !_G.stack.pushedIY && // If for some reason something got pushed, we don't have the return address in place.
    (!isFuncCalleeStackCleanup (currFunc->type) || !ic->parmEscapeAlive && ic->op == CALL && 0 /* todo: test and enable depending on optimization goal - as done for stm8 - for z80 and r3ka this will be slower and bigger than without tail call optimization, but it saves RAM */) &&
    !ic->localEscapeAlive &&
    !IFFUNC_ISBANKEDCALL (dtype) && !IFFUNC_ISZ88DK_SHORTCALL (ftype) &&
    (_G.omitFramePtr || IS_SM83))
    {
      int limit = 16; // Avoid endless loops in the code putting us into an endless loop here.

      if (isFuncCalleeStackCleanup (currFunc->type))
        {
           const bool caller_bigreturn = currFunc->type->next && (getSize (currFunc->type->next) > 4) || IS_STRUCT (currFunc->type->next);
           int caller_stackparmbytes = caller_bigreturn * 2;
           for (value *caller_arg = FUNC_ARGS(currFunc->type); caller_arg; caller_arg = caller_arg->next)
             {
               wassert (caller_arg->sym);
               if (!SPEC_REGPARM (caller_arg->etype))
                 caller_stackparmbytes += getSize (caller_arg->sym->type);
             }
           prestackadjust += caller_stackparmbytes;
        }

      for (const iCode *nic = ic->next; nic && --limit;)
        {
          const symbol *targetlabel = 0;

          if (nic->op == LABEL)
            ;
          else if (nic->op == GOTO) // We dont have ebbi here, so we cant just use eBBWithEntryLabel (ebbi, ic->label). Search manually.
            targetlabel = IC_LABEL (nic);
          else if (nic->op == RETURN && (!IC_LEFT (nic) || SomethingReturned && IC_RESULT (ic)->key == IC_LEFT (nic)->key))
            targetlabel = returnLabel;
          else if (nic->op == ENDFUNCTION)
            {
              if (OP_SYMBOL (IC_LEFT (nic))->stack <= (ic->op == PCALL ? 1 : (optimize.codeSize ? 1 : 2)) + IS_RAB * 120)
                {
                  prestackadjust = OP_SYMBOL (IC_LEFT (nic))->stack;
                  tailjump = true;
                  break;
                }
              prestackadjust = 0;
              break;
            }
          else
            {
              prestackadjust = 0;
              break;
            }

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
                {
                  prestackadjust = 0;
                  tailjump = false;
                  break;
                }

              nic = nnic;
            }
          else
            nic = nic->next;
        }
    }

  if (tailjump && SomethingReturned) // Explicitly check for matching registers, as otherwise calls between __sdcccall(1) and __z88dk_fastcall will go wrong.
    for (int i = 0; i < IC_RESULT (ic)->aop->size; i++)
      if (!aopInReg (aopRet (currFunc->type), 0, aopRet (ftype)->aopu.aop_reg[0]->rIdx))
        tailjump = false;

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

      if (isLitWord (IC_LEFT (ic)->aop))
        {
          adjustStack (prestackadjust, a_free, bc_free, de_free, hl_free, false);
          emit2 (jump ? "jp %s" : "call %s", aopGetLitWordLong (IC_LEFT (ic)->aop, 0, FALSE));
          if (jump)
            cost2 (3, 10, 9, 8, 16,	8, 4, 3);
          else
            cost2 (3, 17, 16, 12, 24, 14, 5, 3);
        }
      else if (getPairId (IC_LEFT (ic)->aop) != PAIR_IY && hl_free)
        {
          spillPair (PAIR_HL);
          genMove (ASMOP_HL, IC_LEFT (ic)->aop, a_free, hl_free, de_free, true);
          adjustStack (prestackadjust, a_not_parm, bc_not_parm, de_not_parm, false, false);
          emit2 (jump ? "!jphl" : "call ___sdcc_call_hl");
          if (jump)
            cost2 (1, 4, 3, 4, 4, 8, 3, 1);
          else
            {
              cost2 (3, 17, 16, 12, 24, 14, 5, 3);
              // todo: add cycles spent in ___sdcc_call_hl here
            }
        }
      else if (!IS_SM83 && !IY_RESERVED && !z80IsParmInCall (ftype, "iy")) // Ensure that we don't access the stack via iy when reading IC_LEFT (ic).
        {
          spillPair (PAIR_IY);
          if (IC_LEFT (ic)->aop->type == AOP_EXSTK) // Ensure that we don't directly overwrite iyl while accessing the stack via iy.
            {
              _push (PAIR_HL);
              genMove (ASMOP_HL, IC_LEFT (ic)->aop, a_not_parm, true, de_not_parm, true);
              emit2 ("ex (sp), hl");
              cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
              _pop (PAIR_IY);
            }
          else
            genMove (ASMOP_IY, IC_LEFT (ic)->aop, a_not_parm, hl_not_parm, de_not_parm, true);
          adjustStack (prestackadjust, a_not_parm, bc_not_parm, de_not_parm, hl_not_parm, false);
          emit2 (jump ? "jp (iy)" : "call ___sdcc_call_iy");
          if (jump)
            cost2 (2, 8, 6, 6, 0, 8, 4, 2);
          else
            {
              cost2 (3, 17, 16, 12, 24, 14, 5, 3);
              // todo: add cycles spent in ___sdcc_call_iy here
            }
        }
      else if (bc_not_parm && (ic->left->aop->regs[B_IDX] < 0 && ic->left->aop->regs[C_IDX] < 0 || de_free)) // Try bc, since it is the only 16-bit register guarateed to be free even for __z88dk_fastcall with --reserve-regs-iy
        {
          wassert (!prestackadjust);
          wassert (IY_RESERVED || IS_SM83); // The peephole optimizer handles ret for purposes other than returning only for --reserve-regs-iy
          symbol *tlbl = 0;
          if (ic->left->aop->regs[B_IDX] >= 0 || ic->left->aop->regs[C_IDX] >= 0)
            {
              wassert (de_free);
              if (!regalloc_dry_run)
                {
                  tlbl = newiTempLabel (NULL);
                  emit2 ("ld de, !immed!tlabel", labelKey2num (tlbl->key));
                  _push (PAIR_DE);
                }
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
            }
          else if (!regalloc_dry_run)
            {
              if (!regalloc_dry_run)
                {
                  tlbl = newiTempLabel (NULL);
                  emit2 ("ld bc, !immed!tlabel", labelKey2num (tlbl->key));
                  _push (PAIR_BC);
                }
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
            }
          genMove (ASMOP_BC, IC_LEFT (ic)->aop, a_not_parm, hl_not_parm, de_not_parm, true);
          emit2 ("push bc");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
          emit2 ("ret");
          cost2 (1, 10, 9, 8, 16, 10, 5, 3);
          if (!regalloc_dry_run)
            _G.stack.pushed -= 2;
          if (tlbl)
            emitLabel (tlbl);
        }
      else if (de_not_parm && (ic->left->aop->regs[D_IDX] < 0 && ic->left->aop->regs[E_IDX] < 0 || bc_free)) // Try de.
        {
          wassert (!prestackadjust);
          wassert (IY_RESERVED || IS_SM83); // The peephole optimizer handles ret for purposes other than returning only for --reserve-regs-iy
          symbol *tlbl = 0;
          if (ic->left->aop->regs[D_IDX] >= 0 || ic->left->aop->regs[E_IDX] >= 0)
            {
              wassert (bc_free);
              if (!regalloc_dry_run)
                {
                  tlbl = newiTempLabel (NULL);
                  emit2 ("ld bc, !immed!tlabel", labelKey2num (tlbl->key));
                  _push (PAIR_BC);
                }
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
            }
          else if (!regalloc_dry_run)
            {
              if (!regalloc_dry_run)
                {
                  tlbl = newiTempLabel (NULL);
                  emit2 ("ld de, !immed!tlabel", labelKey2num (tlbl->key));
                  _push (PAIR_DE);
                }
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
            }
          genMove (ASMOP_DE, IC_LEFT (ic)->aop, a_not_parm, hl_not_parm, true, true);
          emit2 ("push de");
          cost2 (1, 11, 11, 10, 16, 8, 3, 4);
          emit2 ("ret");
          cost2 (1, 10, 9, 8, 16, 10, 5, 3);
          if (!regalloc_dry_run)
            _G.stack.pushed -= 2;
          if (tlbl)
            emitLabel (tlbl);
        }
      else
        UNIMPLEMENTED;
    }
  else
    {
      /* make the call */
      if (IFFUNC_ISBANKEDCALL (dtype))
        {
          wassert (!prestackadjust);

          char *name = OP_SYMBOL (IC_LEFT (ic))->rname[0] ? OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name;
          /* there 3 types of banked call:
               legacy - only if --legacy-banking is specified
               a:bc - only for __z88dk_fastcall __banked functions
               e:hl - default (may have optimal bank switch routine) */
          if (z80_opts.legacyBanking)
            {
              emit2 ("call ___sdcc_bcall");
              emit2 ("!dws", name);
              emit2 ("!dw !bankimmeds", name);
              regalloc_dry_run_cost += 7;
            }
          else if (IFFUNC_ISZ88DK_FASTCALL (ftype))
            {
              spillPair (PAIR_BC);
              emit2 ("ld a, !hashedbankimmeds", name);
              emit2 ("ld bc, !hashedstr", name);
              emit2 ("call ___sdcc_bcall_abc");
              regalloc_dry_run_cost += 8;
            }
          else
            {
              spillPair (PAIR_DE);
              spillPair (PAIR_HL);
              emit2 ("ld e, !hashedbankimmeds", name);
              emit2 ("ld hl, !hashedstr", name);
              emit2 ("call ___sdcc_bcall_ehl");
              regalloc_dry_run_cost += 8;
            }
        }
      else
        {
          if (currFunc && isFuncCalleeStackCleanup (currFunc->type) && prestackadjust && !IFFUNC_ISNORETURN (ftype)) // Copy return value into correct location on stack for tail call optimization.
            {
              wassert (0);
              /* todo: implement */
            }

          adjustStack (prestackadjust, false, bc_free, de_free, hl_free, false);

          if (IS_LITERAL (etype))
            {
              emit2 (jump ? "jp !constword" : "call !constword", ulFromVal (OP_VALUE (IC_LEFT (ic))));
              if (jump)
                cost2 (3, 10, 9, 8, 16,	8, 4, 3);
              else
                cost2 (3, 17, 16, 12, 24, 14, 5, 3);
            }
          else if (IFFUNC_ISZ88DK_SHORTCALL(ftype))
            {
              int rst = ftype->funcAttrs.z88dk_shortcall_rst;
              int value = ftype->funcAttrs.z88dk_shortcall_val;
              emit2 ("rst !constbyte", (unsigned)rst);
              cost2 (1, 11, 11, 8, 16, 0, 5, 4);
              if (value < 256)
                emit2 ("defb !constbyte\n", (unsigned)value);
              else
                emit2 ("defw !constword\n", (unsigned)value);
              regalloc_dry_run_cost_bytes += 2 + (value >= 256);
            }
          else
            {
              emit2 ("%s %s", jump ? "jp" : "call",
                (OP_SYMBOL (IC_LEFT (ic))->rname[0] ? OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name));
              if (jump)
                cost2 (3, 10, 9, 8, 16,	8, 4, 3);
              else
                cost2 (3, 17, 16, 12, 24, 14, 5, 3);
            }
        }
    }
  spillCached ();

  freeAsmop (IC_LEFT (ic), 0);

  _G.stack.pushed += prestackadjust;

  /* Mark the registers as restored. */
  _G.saves.saved = false;

  /* adjust the stack for parameters if required */
  if ((ic->parmBytes || bigreturn) && (IFFUNC_ISNORETURN (ftype) || isFuncCalleeStackCleanup (ftype)))
    {
      if (!regalloc_dry_run)
        {
          _G.stack.pushed -= (ic->parmBytes + bigreturn * 2);
          z80_symmParm_in_calls_from_current_function = false;
        }
    }
  else if ((ic->parmBytes || bigreturn))
    {
      bool return_in_reg = SomethingReturned && !bigreturn;
      adjustStack (ic->parmBytes + bigreturn * 2,
        !return_in_reg || !aopRet (ftype) || aopRet (ftype)->regs[A_IDX] < 0 || aopRet (ftype)->regs[A_IDX] > IC_RESULT (ic)->aop->size,
        !return_in_reg || !aopRet (ftype) || (aopRet (ftype)->regs[C_IDX] < 0 || aopRet (ftype)->regs[C_IDX] > IC_RESULT (ic)->aop->size) && (aopRet (ftype)->regs[B_IDX] < 0 || aopRet (ftype)->regs[B_IDX] > IC_RESULT (ic)->aop->size),
        !return_in_reg || !aopRet (ftype) || (aopRet (ftype)->regs[E_IDX] < 0 || aopRet (ftype)->regs[E_IDX] > IC_RESULT (ic)->aop->size) && (aopRet (ftype)->regs[D_IDX] < 0 || aopRet (ftype)->regs[D_IDX] > IC_RESULT (ic)->aop->size),
        !return_in_reg || !aopRet (ftype) || (aopRet (ftype)->regs[L_IDX] < 0 || aopRet (ftype)->regs[L_IDX] > IC_RESULT (ic)->aop->size) && (aopRet (ftype)->regs[H_IDX] < 0 || aopRet (ftype)->regs[H_IDX] > IC_RESULT (ic)->aop->size),
        !IY_RESERVED);

      if (regalloc_dry_run)
        _G.stack.pushed += ic->parmBytes + bigreturn * 2;
    }

  /* if we need assign a result value */
  if (SomethingReturned && !bigreturn)
    {
      genMove (IC_RESULT (ic)->aop, aopRet (ftype), true, true, true, true);

      freeAsmop (IC_RESULT (ic), 0);
    }

  spillCached ();

  restoreRegs (_G.stack.pushedIY, _G.stack.pushedDE, _G.stack.pushedBC, _G.stack.pushedHL, IC_RESULT (ic), ic);
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
  _G.stack.pushed = 0;

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

  if (IFFUNC_BANKED (sym->type))
    {
      int bank_number = 0;
      for (int i  = strlen (options.code_seg)-1; i >= 0; i--)
        {
          if (!isdigit (options.code_seg[i]) && options.code_seg[i+1] != '\0')
            {
              bank_number = atoi (&options.code_seg[i+1]);
              break;
            }
        }
      emit2("!bequ", sym->rname, bank_number);
    }

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
          if (IS_SM83 || IS_RAB || IS_TLCS90)
            {
              emit2 ("!di");
            }
          else
            {
              if (z80_opts.nmosZ80)
                emit2 ("call ___sdcc_critical_enter");
              else
                {
                  //get interrupt enable flag IFF2 into P/O
                  emit2 ("ld a,i");
                  //disable interrupts
                  emit2 ("!di");
                }
              //save P/O flag
              emit2 ("push af");
              _G.stack.param_offset += 2;
            }
        }
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
                  if (!IS_SM83)
                    {
                      deInUse = true;
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

  bigreturn = (getSize (ftype->next) > 4) || IS_STRUCT (ftype->next);
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

  _G.omitFramePtr = should_omit_frame_ptr;

  if (!IS_SM83 && !z80_opts.noOmitFramePtr && !stackParm && !sym->stack)
    {
      if (!regalloc_dry_run)
        _G.omitFramePtr = true;
    }
  else if (sym->stack)
    {
      if (IS_EZ80_Z80 && !_G.omitFramePtr && -sym->stack > -128 && -sym->stack <= -3 && (z80IsParmInCall (sym->type, "l") || z80IsParmInCall (sym->type, "h")))
        {
          emit2 ("push ix");
          cost (2, 4);
          emit2 ("ld ix, !immed%d", -sym->stack);
          cost (4, 4);
          emit2 ("add ix, sp");
          cost (2, 2);
          emit2 ("ld sp, ix");
          cost (2, 2);
          emit2 ("lea ix, ix, !immed%d", sym->stack);
          cost (3, 3);
        }
      else
        {
          if (!_G.omitFramePtr)
            emit2 ((optimize.codeSize && !z80IsParmInCall (sym->type, "l") && !z80IsParmInCall (sym->type, "h")) ? "!enters" : "!enter");
          if (IS_EZ80_Z80 && !_G.omitFramePtr && -sym->stack > -128 && -sym->stack <= -3 && !z80IsParmInCall (sym->type, "l") && !z80IsParmInCall (sym->type, "h"))
            {
              emit2 ("lea hl, ix, !immed%d", -sym->stack);
              cost (3, 3);
              emit2 ("ld sp, hl");
              cost (1, 1);
            }
          else
            adjustStack (-sym->stack, !z80IsParmInCall (sym->type, "a"), !z80IsParmInCall (sym->type, "c") && !z80IsParmInCall (sym->type, "v"), !z80IsParmInCall (sym->type, "e") && !z80IsParmInCall (sym->type, "d"), !z80IsParmInCall (sym->type, "l") && !z80IsParmInCall (sym->type, "h"), !IY_RESERVED);
        }
      _G.stack.pushed = 0;
    }
  else if (!_G.omitFramePtr)
    {
      emit2 ((optimize.codeSize && !z80IsParmInCall (sym->type, "l") && !z80IsParmInCall (sym->type, "h")) ? "!enters" : "!enter"); // !enters might result in a function call to a helper function.
    }

  _G.stack.offset = sym->stack;
  
  for (PAIR_ID pairId = 0; pairId < NUM_PAIRS; pairId++)
    spillPair (pairId);
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode *ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  /* __critical __interrupt without an interrupt number is the non-maskable interrupt */
  bool is_nmi = (IS_Z80 || IS_Z180 || IS_EZ80_Z80 || IS_Z80N || IS_R800) && IFFUNC_ISCRITICAL (sym->type) && FUNC_INTNO (sym->type) == INTNO_UNSPEC;
  bool bc_free = !aopRet (sym->type) || aopRet (sym->type)->regs[C_IDX] < 0 && aopRet (sym->type)->regs[B_IDX] < 0;
  bool de_free = !aopRet (sym->type) || aopRet (sym->type)->regs[E_IDX] < 0 && aopRet (sym->type)->regs[D_IDX] < 0;
  bool hl_free = !aopRet (sym->type) || aopRet (sym->type)->regs[L_IDX] < 0 && aopRet (sym->type)->regs[H_IDX] < 0;
  bool iy_free = !IY_RESERVED && (!aopRet (sym->type) || aopRet (sym->type)->regs[IYL_IDX] < 0 && aopRet (sym->type)->regs[IYH_IDX] < 0);

  wassert (!regalloc_dry_run);
  wassertl (!_G.stack.pushed, "Unbalanced stack.");

  if (IFFUNC_ISNAKED (sym->type) || IFFUNC_ISNORETURN (sym->type))
    {
      emitDebug (IFFUNC_ISNAKED (sym->type) ? "; naked function: No epilogue." : "; _Noreturn function: No epilogue.");
      return;
    }

  if (!regalloc_dry_run && IFFUNC_ISZ88DK_CALLEE (sym->type) && FUNC_HASVARARGS (sym->type))
    werror (E_Z88DK_CALLEE_VARARG); // We have no idea how many bytes on the stack we'd have to clean up.

  const bool bigreturn = (getSize (sym->type->next) > 4) || IS_STRUCT (sym->type->next);
  int stackparmbytes = bigreturn * 2;
  for (value *arg = FUNC_ARGS(sym->type); arg; arg = arg->next)
    {
      wassert (arg->sym);
      int argsize = getSize (arg->sym->type);
      if (argsize == 1 && FUNC_ISSMALLC (sym->type)) // SmallC calling convention passes 8-bit stack arguments as 16 bit.
        argsize++;
      if (!SPEC_REGPARM (arg->etype))
        stackparmbytes += argsize;
    }

  int poststackadjust = isFuncCalleeStackCleanup (sym->type) ? stackparmbytes : 0;

  if (poststackadjust && // Try to merge both stack adjustments.
    _G.omitFramePtr &&
    (IS_RAB && _G.stack.offset <= 255 || IS_TLCS90 && _G.stack.offset <= 127) &&
    (hl_free || iy_free) && 
    !_G.calleeSaves.pushedDE && !_G.calleeSaves.pushedBC &&
    !IFFUNC_ISISR (sym->type) && !IFFUNC_ISCRITICAL (sym->type))
    {
      emit2 (hl_free ? "ld hl, %d (sp)" : "ld iy, %d (sp)", _G.stack.offset);
      if (hl_free)
        cost2 (2 + IS_TLCS90, 0, 0, 9, 0, 12, 0, 0);
      else
        cost2 (3, 0, 0, 11, 0, 12, 0, 0);
      adjustStack (_G.stack.offset + 2 + poststackadjust,
      !aopRet (sym->type)  || aopRet (sym->type)->regs[A_IDX] < 0,
      bc_free,
      de_free,
      false,
      iy_free && hl_free);
      emit2 (hl_free ? "!jphl" : "jp (iy)");
      if (hl_free)
        cost2 (1 + IS_TLCS90, 4, 3, 4, 4, 8, 3, 1);
      else
        cost2 (2, 8, 6, 6, 0, 8, 4, 2);
      goto done;
    }
  else if (!IS_SM83 && !_G.omitFramePtr && sym->stack > (optimize.codeSize ? 2 : 1))
    {
      emit2 ("ld sp, ix");
      cost2 (2, 10, 7, 4, 0, 6, 2, 2);
    }
  else
    adjustStack (_G.stack.offset,
      !aopRet (sym->type)  || aopRet (sym->type)->regs[A_IDX] < 0,
      bc_free,
      de_free,
      hl_free,
      iy_free);

  if(!IS_SM83 && !_G.omitFramePtr)
    {
      emit2 ("pop ix");
      cost2 (2 - IS_TLCS90, 14, 12, 9, 0, 8, 4, 5);
    }

  wassertl(regalloc_dry_run || !(isFuncCalleeStackCleanup (sym->type) && (_G.calleeSaves.pushedDE || _G.calleeSaves.pushedBC)), "Unimplemented __z88dk_callee support for calle-saved bc/de on callee side");
  if (_G.calleeSaves.pushedDE)
    {
      emit2 ("pop de");
      cost2 (1, 10, 9, 7, 12, 8, 3, 4);
      _G.calleeSaves.pushedDE = FALSE;
    }

  if (_G.calleeSaves.pushedBC)
    {
      emit2 ("pop bc");
      cost2 (1, 10, 9, 7, 12, 8, 3, 4);
      _G.calleeSaves.pushedBC = FALSE;
    }

  /* if this is an interrupt service routine
     then restore all potentially used registers. */
  if (IFFUNC_ISISR (sym->type))
    {
      emit2 ("!popa");
      regalloc_dry_run_cost++;
    }
  else
    {
      /* This is a non-ISR function.
         If critical function then turn interrupts back on */
      if (IFFUNC_ISCRITICAL (sym->type))
        {
          if (IS_SM83 || IS_TLCS90 || IS_RAB)
            emit2 ("!ei");
          else
            {
              symbol *tlbl = newiTempLabel (NULL);
              //restore P/O flag
              if (aopRet (sym->type) && aopRet (sym->type)->regs[A_IDX] >= 0) // Preserve return value in a.
                {
                  wassert (!IS_SM83);
                  emit2 ("ex (sp), hl");
                  emit2 ("ld h, a");
                  emit2 ("ex (sp), hl");
                }
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

  if (poststackadjust)
    {
      wassertl(regalloc_dry_run || !IFFUNC_ISBANKEDCALL (sym->type), "Unimplemented __banked __z88dk_callee support on callee side");
      wassertl(regalloc_dry_run || !IFFUNC_HASVARARGS (sym->type), "__z88dk_callee function may to have variable arguments");

      if (hl_free && !IFFUNC_ISISR (sym->type))
        {
          _pop (PAIR_HL);
          // Parameters should be initialized, so reading them should be fine
          // we also exactly know which registers we can trash
          if (IS_SM83 && poststackadjust == 2 && (!aopRet (sym->type) || aopRet (sym->type)->regs[A_IDX] < 0))
            { // return >8bit
              // This has priority since it shows notUsed that A is free
              // notUsed can't make assumptions about jp (hl)
              emit2 ("pop af");
              cost2 (1, 10, 9, 7, 12, 10, 3, 3);
            }
          else if (IS_SM83 && poststackadjust == 2 && bc_free)
            { // 8bit return
              emit2 ("pop bc");
              cost2 (1, 10, 9, 7, 12, 10, 3, 3);
            }
          else
            {
              adjustStack (poststackadjust,
              !aopRet (sym->type) || aopRet (sym->type)->regs[A_IDX] < 0, bc_free, de_free, false, iy_free);
            }
          emit2 ("!jphl");
          cost2 (1 + IS_TLCS90, 4, 3, 4, 4, 8, 3, 1);
          goto done;
        }
      else if (!IS_SM83 && iy_free && !!IFFUNC_ISISR (sym->type))
        {
          _pop (PAIR_IY);
          adjustStack (poststackadjust, !aopRet (sym->type) || aopRet (sym->type)->regs[A_IDX] < 0, bc_free, de_free, hl_free, false);
          emit2 ("jp (iy)");
          cost2 (2, 8, 6, 6, 0, 8, 4, 2);
          goto done;
        }
      else if (bc_free || de_free)
       {
         _pop (bc_free ? PAIR_BC : PAIR_DE);
         adjustStack (poststackadjust, !aopRet (sym->type) || aopRet (sym->type)->regs[A_IDX] < 0, false, bc_free && de_free, false, false);
         _push (bc_free ? PAIR_BC : PAIR_DE);
       }
      else // Do it the hard way: Copy return address on stack before stack pointer adjustment.
        {
          if (poststackadjust == 1)
            {
              _push (PAIR_HL);
              _push (PAIR_DE);
              emit2 ("ld hl, !immedword", 4u);
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
              emit2 ("add hl, sp");
              cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
              emit2 ("ld e, (hl)");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              emit3w (A_INC, ASMOP_HL, 0);
              emit2 ("ld d, (hl)");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              emit2 ("ld (hl), e");
              cost2 (1, 7, 7, 6, 8, 6, 2, 2);
              emit3w (A_INC, ASMOP_HL, 0);
              emit2 ("ld (hl), d");
              cost2 (1, 7, 7, 6, 8, 6, 2, 2);
              _pop (PAIR_DE);
              _pop (PAIR_HL);     
            }
          else if (IS_SM83)
            {
              _push (PAIR_HL);
              _push (PAIR_DE);
              emit2 ("!ldahlsp", 4);
              regalloc_dry_run_cost += 2;
              emit2 ("ld e, (hl)");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              emit3w (A_INC, ASMOP_HL, 0);
              emit2 ("ld d, (hl)");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              emit2 ("ld hl, !immedword", (unsigned)(4 +  poststackadjust));
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
              emit2 ("add hl, sp");
              cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
              emit2 ("ld (hl), e");
              cost2 (1, 7, 7, 6, 8, 6, 2, 2);
              emit3w (A_INC, ASMOP_HL, 0);
              emit2 ("ld (hl), d");
              cost2 (1, 7, 7, 6, 8, 6, 2, 2);
              _pop (PAIR_DE);
              _pop (PAIR_HL);  
            }
          else
            {
              wassert (!IS_SM83);
              wassert (stackparmbytes != 1); // Avoid overwriting return address and hl.
              emit2 ("ex (sp), hl");
              cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
              _push (PAIR_DE);
              emit3w (A_EX, ASMOP_DE, ASMOP_HL);
              emit2 ("ld hl, !immedword", (unsigned)(2 +  poststackadjust));
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
              emit2 ("add hl, sp");
              cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
              emit2 ("ld (hl), e");
              cost2 (1, 7, 7, 6, 8, 6, 2, 2);
              emit3w (A_INC, ASMOP_HL, 0);
              emit2 ("ld (hl), d");
              cost2 (1, 7, 7, 6, 8, 6, 2, 2);
              _pop (PAIR_DE);
              emit2 ("ex (sp), hl");
              cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
            }

          adjustStack (poststackadjust,
          !aopRet (sym->type) || aopRet (sym->type)->regs[A_IDX] < 0,
          bc_free,
          de_free,
          false,
          iy_free);
        }
    }

  if (options.debug && currFunc)
    {
      debugFile->writeEndFunction (currFunc, ic, 1);
    }

  if (IFFUNC_ISISR (sym->type))
    {
      if (is_nmi)
        {
          emit2 ("retn");
          cost2 (2, 14, 12, 0, 0, 0, 6, 5);
        }
      else if (IS_RAB)
        {
          if (IFFUNC_ISCRITICAL (sym->type))
            {
              emit2 ("!ei");
              cost2 (1, 4, 3, 0, 4, 2, 1, 1);
            }
          emit2 ("ret");
          cost2 (1, 10, 9, 8, 16, 10, 5, 3);
        }
      else if (IS_SM83)
        {
          emit2 (IFFUNC_ISCRITICAL (sym->type) ? "reti" : "ret");
          cost (1 + IFFUNC_ISCRITICAL (sym->type), 16);
        }
      else
        {
          if (IFFUNC_ISCRITICAL (sym->type) && !is_nmi)
            {
              emit2 ("!ei");
              cost2 (1, 4, 3, 0, 4, 2, 1, 1);
            }
          emit2 ("reti");
          cost2 (2, 14, 12, 12, 16, 14, 6, 5);
        }
    }
  else
    {
      /* Both banked and non-banked just ret */
      emit2 ("ret");
      cost2 (1, 10, 9, 8, 16, 10, 5, 3);
    }

done:
  _G.flushStatics = 1;
  _G.stack.pushed = 0;
  _G.stack.offset = 0;

  emitDebug (";\tTotal %s function size at codegen: %u bytes.", sym->name, (unsigned int)regalloc_dry_run_cost);
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
  size = IC_LEFT (ic)->aop->size;

  if (size <= 4 && !IS_STRUCT (operandType (IC_LEFT (ic))))
    {
      /* TODO: get this working with floats */
      if (IC_LEFT (ic)->aop->type == AOP_LIT && size == 4 && !IS_FLOAT (IC_LEFT (ic)->aop->aopu.aop_lit->type) &&
        (aopRet (currFunc->type) == ASMOP_DEHL || aopRet (currFunc->type) == ASMOP_HLDE))
        {
          /* if we have to use two register pairs
             we can reuse values, this is also a prototype
             for the later literal on stack case */
          /* double load is slower, if one is immd, than 2B immd load,
             so we have to make sure both can be reused */
          bool first = false;
          unsigned char value[4];
          PAIR_ID regpairs[2];
          unsigned char offset[2] = {0, 2};
          unsigned long lit = ulFromVal (IC_LEFT (ic)->aop->aopu.aop_lit);
          value[0] = lit&0xff;
          lit>>=8;
          value[1] = lit&0xff;
          lit>>=8;
          value[2] = lit&0xff;
          lit>>=8;
          value[3] = lit&0xff;

          if(aopRet (currFunc->type) == ASMOP_HLDE)
            {
              regpairs[0] = PAIR_DE;
              regpairs[1] = PAIR_HL;
            }
          else
            {
              regpairs[0] = PAIR_HL;
              regpairs[1] = PAIR_DE;
            }

          /* swap if first pair can't hold the immd */
          if(value[0] == value[1] && value[2] != value[3])
            {
              PAIR_ID tmpair = regpairs[0];
              regpairs[0] = regpairs[1];
              regpairs[1] = tmpair;

              unsigned char tmp = value[0];
              value[0] = value[2];
              value[2] = tmp;
              tmp = value[1];
              value[1] = value[3];
              value[3] = tmp;

              tmp = offset[0];
              offset[0] = offset[1];
              offset[1] = tmp;
            }
          
          fetchPairLong (regpairs[0], IC_LEFT (ic)->aop, 0, offset[0]);
          if(value[2] == value[0])
            {
              emit2 ("ld %s, %s", _pairs[regpairs[1]].l, _pairs[regpairs[0]].l);
              first = true;
            }
          else if(value[2] == value[1])
            {
              emit2 ("ld %s, %s", _pairs[regpairs[1]].l, _pairs[regpairs[0]].h);
              first = true;
            }
          
          if(value[3] == value[0] && first == true)
            {
              emit2 ("ld %s, %s", _pairs[regpairs[1]].h, _pairs[regpairs[0]].l);
            }
          else if(value[3] == value[1] && first == true)
            {
              emit2 ("ld %s, %s", _pairs[regpairs[1]].h, _pairs[regpairs[0]].h);
            }
          else
            {
              /* Makes previous loads redundant, which
                 will be optimized by peep hole rules */
              fetchPairLong (regpairs[1], IC_LEFT (ic)->aop, 0, offset[1]);
            }
        }
      else if (size > 0) // SDCC supports GCC extension of returning void
        genMove (aopRet (currFunc->type), IC_LEFT (ic)->aop, true, true, true, true);
    }
  else if (IC_LEFT (ic)->aop->type == AOP_LIT)
    {
      unsigned long long lit = ullFromVal (IC_LEFT (ic)->aop->aopu.aop_lit);
      setupPairFromSP (PAIR_HL, _G.stack.offset + _G.stack.param_offset + _G.stack.pushed + (_G.omitFramePtr || IS_SM83 ? 0 : 2));
      emit2 ("!ldahli");
      regalloc_dry_run_cost += 6;
      emit2 ("ld h, !*hl");
      cost2 (1, 7, 6, 6, 8, 6, 2, 2);
      emit3 (A_LD, ASMOP_L, ASMOP_A);
      do
        {
          emit2 ("ld !*hl, !immedbyte", (unsigned)(lit & 0xffu));
          cost2 (2, 10, 9, 7, 12, 8, 3, 3);
          lit >>= 8;
          if (size > 1)
            emit3w (A_INC, ASMOP_HL, 0);
        }
      while (--size);
    }
  // gbz80 doesn't have have ldir. Rabbit 2000 to Rabbit 3000 (i.e. r2k and r2ka port) have an ldir wait state bug that affects copies between different types of memory.
  else if (!IS_SM83 && IC_LEFT (ic)->aop->type == AOP_STK || IC_LEFT (ic)->aop->type == AOP_EXSTK
           || (IC_LEFT (ic)->aop->type == AOP_DIR || IC_LEFT (ic)->aop->type == AOP_IY) && !(IS_R2K || IS_R2KA))
    {
      setupPairFromSP (PAIR_HL, _G.stack.offset + _G.stack.param_offset + _G.stack.pushed + (_G.omitFramePtr || IS_SM83 ? 0 : 2));
      emit2 ("ld e, !*hl");
      cost2 (1, 7, 6, 6, 8, 6, 2, 2);
      emit3w (A_INC, ASMOP_HL, 0);
      emit2 ("ld d, !*hl");
      cost2 (1, 7, 6, 6, 8, 6, 2, 2);
      if (IC_LEFT (ic)->aop->type == AOP_STK || IC_LEFT (ic)->aop->type == AOP_EXSTK)
        {
          int sp_offset, fp_offset;
          fp_offset =
            IC_LEFT (ic)->aop->aopu.aop_stk + (IC_LEFT (ic)->aop->aopu.aop_stk >
                0 ? _G.stack.param_offset : 0);
          sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
          // TODO: find out if offset is okay
          emit2 ("!ldahlsp", sp_offset);
          spillPair (PAIR_HL);
          regalloc_dry_run_cost += 4;
        }
      else
        fetchLitPair (PAIR_HL, IC_LEFT (ic)->aop, 0, true, false);
      emit2 ("ld bc, !immed%d", size);
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("ldir");
      cost2 (2, 21 * size - 5, 14 * size - 2, 7 * size - 1, 0, 18 * size - 4, 2 * size - 1, 4 * size);
      updatePair (PAIR_HL, size);
    }
  else
    {
      setupPairFromSP (PAIR_HL, _G.stack.offset + _G.stack.param_offset + _G.stack.pushed + (_G.omitFramePtr || IS_SM83 ? 0 : 2));
      emit2 ("ld c, !*hl");
      cost2 (1, 7, 6, 6, 8, 6, 2, 2);
      emit3w (A_INC, ASMOP_HL, 0);
      emit2 ("ld b, !*hl");
      cost2 (1, 7, 6, 6, 8, 6, 2, 2);
      updatePair (PAIR_HL, 1);
      do
        {
          cheapMove (ASMOP_A, 0, IC_LEFT (ic)->aop, offset++, true);
          emit2 ("ld !mems, a", "bc");
          cost2 (1 + IS_TLCS90, 7, 7, 7, 8, 6, 2, 2);
          if (size > 1)
            emit3w (A_INC, ASMOP_BC, 0);
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
      cost2 (3, 10, 9, 8, 16, 8, 4, 3);
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
genPlusIncr (const iCode *ic)
{
  unsigned int icount;
  unsigned int size = IC_RESULT (ic)->aop->size;
  PAIR_ID resultId = getPairId (IC_RESULT (ic)->aop);

  /* will try to generate an increment */
  /* if the right side is not a literal
     we cannot */
  if (IC_RIGHT (ic)->aop->type != AOP_LIT)
    return FALSE;

  icount = (unsigned int) ulFromVal (IC_RIGHT (ic)->aop->aopu.aop_lit);

  /* If result is a pair */
  if (resultId != PAIR_INVALID)
    {
      bool delayed_move;
      if (isLitWord (IC_LEFT (ic)->aop))
        {
          fetchLitPair (getPairId (IC_RESULT (ic)->aop), IC_LEFT (ic)->aop, icount, true, false);
          return TRUE;
        }

      if (size == 2 && icount == 256 && ic->result->aop->type == AOP_REG && ic->left->aop->type == AOP_REG && ic->result->aop->aopu.aop_reg[0]->rIdx == ic->left->aop->aopu.aop_reg[0]->rIdx &&
        (HAS_IYL_INST || !aopInReg (ic->result->aop, 1, IYL_IDX) && !aopInReg (ic->result->aop, 1, IYH_IDX)))
        {
          emit3_o (A_INC, ic->result->aop, 1, 0, 0);
          return true;
        }

      if (IS_Z80N && resultId == getPairId (IC_LEFT (ic)->aop) && resultId != PAIR_IY && icount > 3 && icount < 256 && isRegDead (A_IDX, ic)) // Saves once cycle vs. add dd, nn below.
        {
          cheapMove (ASMOP_A, 0, IC_RIGHT (ic)->aop, 0, true);
          emit2 ("add %s, a", getPairName (IC_RESULT (ic)->aop));
          cost (2, 8);
          return true;
        }
      else if (icount == 255 && resultId != PAIR_IY)
        {
          fetchPair (resultId, IC_LEFT (ic)->aop);
          emit3w (A_DEC, ic->result->aop, 0);
          emit3_o (A_INC, ic->result->aop, 1, 0, 0);
          return true;
        }
      else if (icount == 257 && resultId != PAIR_IY)
        {
          fetchPair (resultId, IC_LEFT (ic)->aop);
          emit3w (A_INC, ic->result->aop, 0);
          emit3_o (A_INC, ic->result->aop, 1, 0, 0);
          return true;
        }
      else if (resultId == getPairId (ic->left->aop) &&
        (IS_Z80N && resultId != PAIR_IY && icount > 3 || IS_TLCS90 && (resultId == PAIR_HL || resultId == PAIR_IY) && icount > 2))
        {
          emit2 ("add %s, !immed%s", getPairName (IC_RESULT (ic)->aop), aopGetLitWordLong (IC_RIGHT (ic)->aop, 0, false));
          cost2 (4 - IS_TLCS90, 16, 0, 0, 0, 6, 0, 0);
          return true;
        }

      if (isPair (IC_LEFT (ic)->aop) && getPairId (IC_LEFT (ic)->aop) != PAIR_IY && resultId == PAIR_HL && icount > 3)
        {
          if (getPairId (IC_LEFT (ic)->aop) == PAIR_HL)
            {
              PAIR_ID freep = getDeadPairId (ic);
              if (freep != PAIR_INVALID)
                {
                  fetchPair (freep, IC_RIGHT (ic)->aop);
                  emit2 ("add hl, %s", _pairs[freep].name);
                  cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
                  return TRUE;
                }
            }
          else
            {
              fetchPair (PAIR_HL, IC_RIGHT (ic)->aop);
              emit3w (A_ADD, ASMOP_HL, ic->left->aop);
              return true;
            }
        }
      if (icount > 5)
        return FALSE;
      /* Inc a pair */
      delayed_move = (getPairId (IC_RESULT (ic)->aop) == PAIR_IY && getPairId (IC_LEFT (ic)->aop) != PAIR_INVALID
                      && isPairDead (getPairId (IC_LEFT (ic)->aop), ic));
      if (!sameRegs (IC_LEFT (ic)->aop, IC_RESULT (ic)->aop))
        {
          if (icount > 3)
            return FALSE;
          if (!delayed_move)
            genMove (IC_RESULT (ic)->aop, IC_LEFT (ic)->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);
        }
      while (icount--)
        {
          PAIR_ID pair = delayed_move ? getPairId (IC_LEFT (ic)->aop) : getPairId (IC_RESULT (ic)->aop);
          emit2 ("inc %s", _pairs[pair].name);
          if (pair == PAIR_IY)
            cost2 (2 - IS_TLCS90, 10, 7, 4, 0, 4, 2, 2);
          else
            cost2 (1, 6, 4, 2, 8, 4, 1, 1);
        }
      if (delayed_move)
        fetchPair (getPairId (IC_RESULT (ic)->aop), IC_LEFT (ic)->aop);
      return true;
    }

  if (!IS_SM83 && isLitWord (IC_LEFT (ic)->aop) && size == 2 && isPairDead (PAIR_HL, ic))
    {
      fetchLitPair (PAIR_HL, IC_LEFT (ic)->aop, icount, true, false);
      genMove (IC_RESULT (ic)->aop, ASMOP_HL, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), true);
      return true;
    }

  if (icount > 4) // Not worth it if the sequence of inc gets too long.
    return false;

  if (icount > 1 && size == 1 && aopInReg (IC_LEFT (ic)->aop, 0, A_IDX)) // add a, #n is cheaper than sequence of inc a.
    return false;

  if (size == 2 && getPairId (IC_LEFT (ic)->aop) != PAIR_INVALID && icount <= 3 && isPairDead (getPairId (IC_LEFT (ic)->aop), ic))
    {
      while (icount--)
        emit3w (A_INC, ic->left->aop, 0);
      genMove (IC_RESULT (ic)->aop, IC_LEFT (ic)->aop, isRegDead (A_IDX, ic), isPairDead(PAIR_HL, ic), isPairDead(PAIR_DE, ic), true);
      return true;
    }

  if (size == 2 && icount <= 2 && isPairDead (PAIR_HL, ic) && !IS_SM83 &&
    (IC_LEFT (ic)->aop->type == AOP_HL || IC_LEFT (ic)->aop->type == AOP_IY))
    {
      genMove (ASMOP_HL, IC_LEFT (ic)->aop, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), true);
      while (icount--)
        emit3w (A_INC, ASMOP_HL, 0);
      genMove (IC_RESULT (ic)->aop, ASMOP_HL, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), true);
      return true;
    }

  /* if increment 16 bits in register */
  if (sameRegs (IC_LEFT (ic)->aop, IC_RESULT (ic)->aop) && size > 1 && icount == 1
    && (HAS_IYL_INST || size == 2 && getPairId (IC_RESULT (ic)->aop) != PAIR_INVALID || size >= 2 && !aopInReg (IC_RESULT (ic)->aop, 0, IYL_IDX) && !aopInReg (IC_RESULT (ic)->aop, 0, IYH_IDX) && !aopInReg (IC_RESULT (ic)->aop, 1, IYL_IDX) && !aopInReg (IC_RESULT (ic)->aop, 1, IYH_IDX)))
    {
      int offset = 0;
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      while (size--)
        {
          if (offset)
            regalloc_dry_run_state_scale /= 256.0f; // Cycle cost contribution of upper byte additions is negligible
          if (aopIsLitVal (ic->result->aop, offset, size + 1, 0)) // Skip known leading zero result bytes.
            {
              offset += size;
              size = 0;
              break;
            }
          if (size == 1 && getPairId_o (IC_RESULT (ic)->aop, offset) != PAIR_INVALID)
            {
              emit3w_o (A_INC, ic->result->aop, offset, 0, 0);
              size--;
              offset += 2;
              break;
            }
          if (!HAS_IYL_INST && (aopInReg (IC_RESULT (ic)->aop, offset, IYL_IDX) || aopInReg (IC_RESULT (ic)->aop, offset, IYH_IDX)))
            UNIMPLEMENTED;
          else
            emit3_o (A_INC, IC_RESULT (ic)->aop, offset++, 0, 0);
          if (size)
            {
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
              cost2 (3, 10, 9, 7, 16, 12, 4, 3); // Assume jump is taken (upper bytes are skipped).
            }
        }
      regalloc_dry_run_state_scale = 1.0f;
      if (!regalloc_dry_run)
        (IC_LEFT (ic)->aop->type == AOP_HL || IS_SM83
         && IC_LEFT (ic)->aop->type == AOP_STK) ? emitLabelSpill (tlbl) : emitLabel (tlbl);
      else if (IC_LEFT (ic)->aop->type == AOP_HL)
        spillCached ();
      return TRUE;
    }

  /* if the sizes are greater than 1 then we cannot */
  if (IC_RESULT (ic)->aop->size > 1 || IC_LEFT (ic)->aop->size > 1)
    return FALSE;

  /* If the result is in a register then we can load then increment.
   */
  if (IC_RESULT (ic)->aop->type == AOP_REG)
    {
      cheapMove (IC_RESULT (ic)->aop, LSB, IC_LEFT (ic)->aop, LSB, true);
      while (icount--)
        if (!HAS_IYL_INST && (aopInReg (IC_RESULT (ic)->aop, 0, IYL_IDX) || aopInReg (IC_RESULT (ic)->aop, 0, IYH_IDX)))
          UNIMPLEMENTED;
        else
          emit3_o (A_INC, IC_RESULT (ic)->aop, 0, 0, 0);
      return TRUE;
    }

  /* we can if the aops of the left & result match or
     if they are in registers and the registers are the
     same */
  if (sameRegs (IC_LEFT (ic)->aop, IC_RESULT (ic)->aop))
    {
      while (icount--)
        emit3 (A_INC, IC_LEFT (ic)->aop, 0);
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
  if (result->aop->type == AOP_CRY)
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
      // Assume that both values are equally likely.
      cost2 (3, 10, 7.5f, 3.5f, 14.0f, 11.0f, 3.5f, 3.0f);
      cost2 (1,	3.5f, 3.0f, 2.0f, 4.0f, 4.0f, 2.0f, 2.0f);
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
  wassertl (!IS_SM83, "Not implemented for the SM83");

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

  if (!IS_SM83)
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

  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  /* Swap the left and right operands if:

     if literal, literal on the right or
     if left requires ACC or right is already
     in ACC */
  if ((IC_LEFT (ic)->aop->type == AOP_LIT) || (AOP_NEEDSACC (IC_RIGHT (ic))) || aopInReg (IC_RIGHT (ic)->aop, 0, A_IDX) ||
    IC_LEFT (ic)->aop->regs[A_IDX] < 0 && IC_RIGHT (ic)->aop->type == AOP_STL)
    {
      operand *t = IC_RIGHT (ic);
      IC_RIGHT (ic) = IC_LEFT (ic);
      IC_LEFT (ic) = t;
    }

  leftop = IC_LEFT (ic)->aop;
  rightop = IC_RIGHT (ic)->aop;

  /* if both left & right are in bit
     space */
  if (IC_LEFT (ic)->aop->type == AOP_CRY && IC_RIGHT (ic)->aop->type == AOP_CRY)
    {
      /* Cant happen */
      wassertl (0, "Tried to add two bits");
    }

  /* if left in bit space & right literal */
  if (IC_LEFT (ic)->aop->type == AOP_CRY && IC_RIGHT (ic)->aop->type == AOP_LIT)
    {
      /* Can happen I guess */
      wassertl (0, "Tried to add a bit to a literal");
    }

  /* if I can do an increment instead
     of add then GOOD for ME */
  if (!maskedtopbyte && genPlusIncr (ic))
    goto release;

  size = IC_RESULT (ic)->aop->size;

  /* Special case when left and right are constant */
  if (!maskedtopbyte && isPair (IC_RESULT (ic)->aop))
    {
      char *left = Safe_strdup (aopGetLitWordLong (IC_LEFT (ic)->aop, 0, FALSE));
      const char *right = aopGetLitWordLong (IC_RIGHT (ic)->aop, 0, FALSE);

      if (IC_LEFT (ic)->aop->type == AOP_LIT && IC_RIGHT (ic)->aop->type == AOP_LIT && left && right)
        {
          struct dbuf_s dbuf;

          /* It's a pair */
          /* PENDING: fix */
          dbuf_init (&dbuf, 128);
          dbuf_printf (&dbuf, "!immed(%s + %s)", left, right);
          Safe_free (left);
          emit2 ("ld %s, %s", getPairName (IC_RESULT (ic)->aop), dbuf_c_str (&dbuf));
          dbuf_destroy (&dbuf);
          if (getPairId (ic->result->aop) == PAIR_IY)
            cost2 (4, 14, 12, 8, 0, 6, 4, 4);
          else
            cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          goto release;
        }
      Safe_free (left);
    }

  // eZ80 has lea.
  if ((IS_TLCS90 || IS_EZ80_Z80) && !maskedtopbyte && isPair (IC_RESULT (ic)->aop) && getPairId (IC_LEFT (ic)->aop) == PAIR_IY && IC_RIGHT (ic)->aop->type == AOP_LIT)
    {
       int lit = (int) ulFromVal (IC_RIGHT (ic)->aop->aopu.aop_lit);
       if (lit >= -128 && lit < 128)
         {
           emit2 (IS_TLCS90 ? "lda %s, iy, !immed%d" : "lea %s, iy, !immed%d", _pairs[getPairId (IC_RESULT (ic)->aop)].name, lit);
           cost (3, IS_TLCS90 ? 10 : 3);
           spillPair (getPairId (IC_RESULT (ic)->aop));
           goto release;
         }
    }

  if (!maskedtopbyte && (isPair (IC_RIGHT (ic)->aop) || isPair (IC_LEFT (ic)->aop)) && getPairId (IC_RESULT (ic)->aop) == PAIR_HL)
    {
      /* Fetch into HL then do the add */
      PAIR_ID left = getPairId (IC_LEFT (ic)->aop);
      PAIR_ID right = getPairId (IC_RIGHT (ic)->aop);

      spillPair (PAIR_HL);

      if (left == PAIR_HL && right != PAIR_INVALID && (IS_TLCS90 || right != PAIR_IY))
        {
          emit3w (A_ADD, ASMOP_HL, ic->right->aop);
          goto release;
        }
      else if (right == PAIR_HL && left != PAIR_INVALID && (IS_TLCS90 || left != PAIR_IY))
        {
          emit3w (A_ADD, ASMOP_HL, ic->left->aop);
          goto release;
        }
      else if (right != PAIR_INVALID && right != PAIR_HL && (IS_TLCS90 || right != PAIR_IY))
        {
          genMove_o (ASMOP_HL, 0, ic->left->aop, 0, 2, isRegDead (A_IDX, ic), true, isRegDead (DE_IDX, ic) && right != PAIR_DE, isRegDead (IY_IDX, ic) && right != PAIR_IY, true);
          emit3w (A_ADD, ASMOP_HL, ic->right->aop);
          goto release;
        }
      else if (left != PAIR_INVALID && left != PAIR_HL && (IS_TLCS90 || left != PAIR_IY))
        {
          genMove_o (ASMOP_HL, 0, ic->right->aop, 0, 2, isRegDead (A_IDX, ic), true, isRegDead (DE_IDX, ic) && left != PAIR_DE, isRegDead (IY_IDX, ic) && left != PAIR_IY, true);
          emit3w (A_ADD, ASMOP_HL, ic->left->aop);
          goto release;
        }
      else if (left == PAIR_HL && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)))
        {
          PAIR_ID pair = (isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC);
          asmop *raop = isPairDead (PAIR_DE, ic) ? ASMOP_DE : ASMOP_BC;
          genMove (raop, ic->right->aop, false, false, false, false);
          emit2 ("add hl, %s", _pairs[pair].name);
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          goto release;
        }
      else if (right == PAIR_HL && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)))
        {
          PAIR_ID pair = (isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC);
          asmop *raop = isPairDead (PAIR_DE, ic) ? ASMOP_DE : ASMOP_BC;
          genMove (raop, leftop, false, false, false, false);
          emit2 ("add hl, %s", _pairs[pair].name);
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          goto release;
        }
      else
        {
          /* Can't do it */
        }
    }
  else if (!maskedtopbyte && size == 2 && getPairId (ic->result->aop) == PAIR_HL && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)) &&
    (ic->right->aop->type == AOP_LIT || ic->right->aop->type == AOP_IMMD || ic->left->aop->type == AOP_IMMD && (ic->right->aop->type == AOP_HL || ic->right->aop->type == AOP_IY)))
    {
      PAIR_ID extrapair = isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC;
      genMove (ASMOP_HL, ic->left->aop, isRegDead (A_IDX, ic), true, isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic));
      genMove (extrapair == PAIR_DE ? ASMOP_DE : ASMOP_BC, ic->right->aop, isRegDead (A_IDX, ic), false, isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic));
      emit2 ("add hl, %s", _pairs[extrapair].name);
      cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
      goto release;
    }

  // Handle AOP_EXSTK conflict with hl here, since setupToPreserveCarry() would cause problems otherwise.
  if (!maskedtopbyte && IC_RESULT (ic)->aop->type == AOP_EXSTK && size <= 2 && (getPairId (IC_LEFT (ic)->aop) == PAIR_HL || getPairId (IC_RIGHT (ic)->aop) == PAIR_HL) &&
    (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)) && isPairDead (PAIR_HL, ic))
    {
      PAIR_ID extrapair = isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC;
      fetchPair (extrapair, getPairId (IC_LEFT (ic)->aop) == PAIR_HL ? IC_RIGHT (ic)->aop : IC_LEFT (ic)->aop);
      emit2 ("add hl, %s", _pairs[extrapair].name);
      cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
      spillPair (PAIR_HL);
      genMove (IC_RESULT (ic)->aop, ASMOP_HL, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), true);
      goto release;
    }
  else if (!maskedtopbyte && getPairId (IC_RESULT (ic)->aop) == PAIR_IY &&
    (getPairId (IC_LEFT (ic)->aop) == PAIR_HL && isPair (IC_RIGHT (ic)->aop) && getPairId (IC_RIGHT (ic)->aop) != PAIR_IY || getPairId (IC_RIGHT (ic)->aop) == PAIR_HL && isPair (IC_LEFT (ic)->aop) && getPairId (IC_LEFT (ic)->aop) != PAIR_IY) &&
    isPairDead (PAIR_HL, ic))
    {
      PAIR_ID pair = (getPairId (IC_LEFT (ic)->aop) == PAIR_HL ? getPairId (IC_RIGHT (ic)->aop) : getPairId (IC_LEFT (ic)->aop));
      emit2 ("add hl, %s", _pairs[pair].name);
      cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
      _push (PAIR_HL);
      _pop (PAIR_IY);
      goto release;
    }
  else if (!maskedtopbyte && getPairId (ic->result->aop) == PAIR_IY &&
    ic->left->aop->type != AOP_IY && ic->right->aop->type != AOP_IY)
    {
      bool save_pair = FALSE;
      PAIR_ID pair;

      if (getPairId (IC_RIGHT (ic)->aop) == PAIR_IY || getPairId (IC_LEFT (ic)->aop) == PAIR_BC || getPairId (IC_LEFT (ic)->aop) == PAIR_DE ||
          ic->left->aop->regs[IYL_IDX] < 0 && ic->left->aop->regs[IYH_IDX] < 0 && (ic->right->aop->type == AOP_IMMD || ic->right->aop->type == AOP_LIT))
        {
          operand *t = IC_RIGHT (ic);
          IC_RIGHT (ic) = IC_LEFT (ic);
          IC_LEFT (ic) = t;
          leftop = IC_LEFT (ic)->aop;
          rightop = IC_RIGHT (ic)->aop;
        }
      pair = getPairId (IC_RIGHT (ic)->aop);
      if (pair != PAIR_BC && pair != PAIR_DE)
        {
          if (IC_RIGHT (ic)->aop->type == AOP_REG && IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == C_IDX
              && (isRegDead (B_IDX, ic) || !isPairDead (PAIR_DE, ic)))
            pair = PAIR_BC;
          else if (IC_RIGHT (ic)->aop->type == AOP_REG && IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX
                   && (isRegDead (D_IDX, ic) || !isPairDead (PAIR_BC, ic)))
            pair = PAIR_DE;
          else
            pair = isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC;
          if (!isPairDead (pair, ic))
            save_pair = TRUE;
        }
      genMove (ASMOP_IY, IC_LEFT (ic)->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);
      if (save_pair)
        _push (pair);
      asmop *raop;
      switch (pair)
       {
       case PAIR_BC:
         raop = ASMOP_BC;
         break;
       case PAIR_DE:
         raop = ASMOP_DE;
         break;
       case PAIR_IY:
         raop = ASMOP_IY;
         break;
       default:
         raop = 0;
         wassert (0);
       }   
      genMove (raop, ic->right->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);
      emit2 ("add iy, %s", _pairs[pair].name);
      spillPair (PAIR_IY);
      cost2 (2, 15, 10, 4, 0, 8, 2, 2);
      if (save_pair)
        _pop (pair);
      goto release;
    }

  /* sm83 special case:
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
  if (!maskedtopbyte && IS_SM83)
    {
      if (IC_LEFT (ic)->aop->type == AOP_STK || IC_RIGHT (ic)->aop->type == AOP_STK || IC_RESULT (ic)->aop->type == AOP_STK)
        {
          if ((IC_LEFT (ic)->aop->size == 2 ||
               IC_RIGHT (ic)->aop->size == 2) && (IC_LEFT (ic)->aop->size <= 2 && IC_RIGHT (ic)->aop->size <= 2 || size == 2))
            {
              if (getPairId (IC_RIGHT (ic)->aop) == PAIR_BC || getPairId (IC_RIGHT (ic)->aop) == PAIR_DE && getPairId (IC_LEFT (ic)->aop) != PAIR_BC)
                {
                  /* Swap left and right */
                  operand *t = IC_RIGHT (ic);
                  IC_RIGHT (ic) = IC_LEFT (ic);
                  IC_LEFT (ic) = t;
                  leftop = IC_LEFT (ic)->aop;
                  rightop = IC_RIGHT (ic)->aop;
                }
              if (getPairId (IC_LEFT (ic)->aop) == PAIR_BC)
                {
                  fetchPair (PAIR_HL, IC_RIGHT (ic)->aop);
                  emit3w (A_ADD, ASMOP_HL, ASMOP_BC);
                }
              else
                {
                  if (!isPairDead (PAIR_DE, ic))
                    _push (PAIR_DE);

                  if (IC_RIGHT (ic)->aop->type == AOP_REG && IC_RIGHT (ic)->aop->size == 2 && IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->size == 2)
                    {
                      const short dst[4] = { E_IDX, L_IDX, D_IDX, H_IDX };
                      short src[4];
                      if (IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX
                          || IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == L_IDX)
                        {
                          src[0] = IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx;
                          src[1] = IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx;
                          src[2] = IC_RIGHT (ic)->aop->aopu.aop_reg[1]->rIdx;
                          src[3] = IC_LEFT (ic)->aop->aopu.aop_reg[1]->rIdx;
                        }
                      else
                        {
                          src[1] = IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx;
                          src[0] = IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx;
                          src[3] = IC_RIGHT (ic)->aop->aopu.aop_reg[1]->rIdx;
                          src[2] = IC_LEFT (ic)->aop->aopu.aop_reg[1]->rIdx;
                        }
                      regMove (dst, src, 4, FALSE);
                    }
                  else if (IC_RIGHT (ic)->aop->type == AOP_REG &&
                           (IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX
                            || IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == D_IDX || IC_RIGHT (ic)->aop->size == 2
                            && (IC_RIGHT (ic)->aop->aopu.aop_reg[1]->rIdx == E_IDX
                                || IC_RIGHT (ic)->aop->aopu.aop_reg[1]->rIdx == D_IDX)))
                    {
                      fetchPair (PAIR_DE, IC_RIGHT (ic)->aop);
                      fetchPair (PAIR_HL, IC_LEFT (ic)->aop);
                    }
                  else
                    {
                      fetchPair (PAIR_DE, IC_LEFT (ic)->aop);
                      fetchPair (PAIR_HL, IC_RIGHT (ic)->aop);
                    }
                  emit3w (A_ADD, ASMOP_HL, ASMOP_DE);

                  if (maskedtopbyte)
                    {
                      if (!isRegDead (A_IDX, ic))
                        _push (PAIR_AF);
                      emit3 (A_LD, ASMOP_A, ASMOP_H);
                      emit2 ("and a, #0x%02x", topbytemask);
                      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
                      emit3 (A_LD, ASMOP_H, ASMOP_A);
                      if (!isRegDead (A_IDX, ic))
                        _pop (PAIR_AF);
                    }

                  if (!isPairDead (PAIR_DE, ic))
                    _pop (PAIR_DE);
                }
              spillPair (PAIR_HL);
              genMove (IC_RESULT (ic)->aop, ASMOP_HL, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), true);
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
  if (size > 1 &&
    !(size == 2 && isPair (leftop) && (rightop->type == AOP_LIT || rightop->type == AOP_IY)) && !(size == 2 && leftop->type == AOP_STL && isPairDead (PAIR_HL, ic))) // No need to setup if a single 16 bit addition is sufficient below.
    {
      if (!couldDestroyCarry (leftop) && (couldDestroyCarry (rightop) || couldDestroyCarry (IC_RESULT (ic)->aop)))
        {
          cheapMove (ASMOP_A, 0, leftop, offset, true);
          premoved = TRUE;
        }

      if ((requiresHL (IC_RESULT (ic)->aop) && IC_RESULT (ic)->aop->type != AOP_REG || requiresHL (leftop) && leftop->type != AOP_REG || requiresHL (rightop) && rightop->type != AOP_REG) &&
        (leftop->regs[L_IDX] > 0 || leftop->regs[H_IDX] > 0 || rightop->regs[L_IDX] > 0 || rightop->regs[H_IDX] > 0))
        UNIMPLEMENTED;
      setupToPreserveCarry (IC_RESULT (ic)->aop, leftop, rightop);
    }
  // But if we don't actually want to use hl for the addition, it can make sense to setup an op to use cheaper hl instead of iy.
  if (size == 1 && !aopInReg(leftop, 0, H_IDX) && !aopInReg(leftop, 0, L_IDX) && !aopInReg(rightop, 0, H_IDX) && !aopInReg(rightop, 0, L_IDX) && isPairDead (PAIR_HL, ic))
    {
      if (couldDestroyCarry (IC_RESULT (ic)->aop) &&
        (IC_RESULT (ic)->aop == leftop || IC_RESULT (ic)->aop == rightop))
        shiftIntoPair (PAIR_HL, IC_RESULT (ic)->aop);
      else if (couldDestroyCarry (rightop))
        shiftIntoPair (PAIR_HL, rightop);
    }

  cached[0] = -1;
  cached[1] = -1;

  for (i = 0, started = false; i < size;)
    {
      bool maskedbyte = maskedtopbyte && (i + 1 == size);
      bool maskedword = maskedtopbyte && (i + 2 == size);

      const bool de_dead = isPairDead (PAIR_DE, ic) &&
        leftop->regs[E_IDX] <= i && leftop->regs[D_IDX] <= i &&
        rightop->regs[E_IDX] <= i && rightop->regs[D_IDX] <= i &&
        (IC_RESULT (ic)->aop->regs[E_IDX] < 0 || IC_RESULT (ic)->aop->regs[E_IDX] >= i) && (IC_RESULT (ic)->aop->regs[D_IDX] < 0 || IC_RESULT (ic)->aop->regs[D_IDX] >= i);
      const bool hl_dead = isPairDead (PAIR_HL, ic) &&
        leftop->regs[L_IDX] <= i && leftop->regs[H_IDX] <= i &&
        rightop->regs[L_IDX] <= i && rightop->regs[H_IDX] <= i &&
        (IC_RESULT (ic)->aop->regs[L_IDX] < 0 || IC_RESULT (ic)->aop->regs[L_IDX] >= i) && (IC_RESULT (ic)->aop->regs[H_IDX] < 0 || IC_RESULT (ic)->aop->regs[H_IDX] >= i);

      // Rematerialization of addresses on the stack.
      if (!maskedword && leftop->type == AOP_STL && !i && i + 1 < size && rightop->type == AOP_LIT && hl_dead)
        {
          emit2 ("ld hl, !immed%d", spOffset (leftop->aopu.aop_stk) + (ulFromVal (rightop->aopu.aop_lit) & 0xffff));
          cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          emit2 ("add hl, sp");
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          spillPair (PAIR_HL);
          started = true;
          genMove_o (IC_RESULT (ic)->aop, 0, ASMOP_HL, 0, 2, true, true, de_dead, false, i + 2 == size);
          i += 2;
          continue;
        }
      else if (!maskedword && leftop->type == AOP_STL && !i && i + 1 < size && hl_dead && (size <= 2 || leftop->type != AOP_EXSTK /* (hl) would be pointed to result, overwritten by addition here */))
        {
          PAIR_ID pair = getPairId (rightop);
          if (pair != PAIR_BC)
            pair = PAIR_DE;
          if (pair == PAIR_DE && !de_dead)
            _push (PAIR_DE);
          genMove (pair == PAIR_BC ? ASMOP_BC : ASMOP_DE, rightop, true, true, de_dead, false);
          genMove (ASMOP_HL, leftop, true, true, de_dead && pair != PAIR_DE, false);
          emit2 ("add hl, %s", _pairs[pair].name);
          spillPair (pair);
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          started = true;
          if (pair == PAIR_DE && !de_dead)
            _pop (PAIR_DE);
          genMove_o (IC_RESULT (ic)->aop, 0, ASMOP_HL, 0, 2, true, true, de_dead, false, i + 2 == size);
          i += 2;
          continue;
        }
      // Addition of interleaved pairs.
      else if (!maskedword && (!premoved || i) && leftop->size - i >= 2 && rightop->size - i >= 2 &&
        (aopInReg (IC_RESULT (ic)->aop, i, HL_IDX) || aopInReg (IC_RESULT (ic)->aop, i, IY_IDX) && !started))
        {
          const bool iy = aopInReg (IC_RESULT (ic)->aop, i, IY_IDX);
          PAIR_ID pair = PAIR_INVALID;

          if (aopInReg (leftop, i, iy ? IYL_IDX : L_IDX) && aopInReg (rightop, i + 1, iy ? IYH_IDX : H_IDX))
            {
              if (aopInReg (leftop, i + 1, D_IDX) && aopInReg (rightop, i, E_IDX))
                pair = PAIR_DE;
              else if (aopInReg (leftop, i + 1, B_IDX) && aopInReg (rightop, i, C_IDX))
                pair = PAIR_BC;
            }
          else if (aopInReg (leftop, i + 1, iy ? IYH_IDX : H_IDX) && aopInReg (rightop, i, iy ? IYL_IDX : L_IDX))
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
                  wassert (!iy);
                  emit2 ("adc hl, %s", _pairs[pair].name);
                  cost2 (2, 15, 10, 4, 0, 8, 2, 2);
                }
              else
                {
                  emit2 (iy ? "add iy, %s" : "add hl, %s", _pairs[pair].name);
                  started = true;
                  if (pair == PAIR_IY)
                    cost2 (2, 15, 10, 4, 0, 8, 2, 2);
                  else
                    cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
                }
              i += 2;
              continue;
            }
        }

      if (!maskedword && (!premoved || i) && !started && leftop->size - i >= 2 && rightop->size - i >= 2 &&
        aopInReg (IC_RESULT (ic)->aop, i, IY_IDX) &&
        (aopInReg (leftop, i, IY_IDX) && (aopInReg (rightop, i, BC_IDX) || aopInReg (rightop, i, DE_IDX)) || aopInReg (rightop, i, IY_IDX) && (aopInReg (leftop, i, BC_IDX) || aopInReg (leftop, i, DE_IDX))))
        {
          PAIR_ID pair = aopInReg(aopInReg (leftop, i, IY_IDX) ? rightop : leftop, i, BC_IDX) ? PAIR_BC : PAIR_DE;
          emit2 ("add iy, %s", _pairs[pair].name);
          cost2 (2, 15, 10, 4, 0, 8, 2, 2);
          started = true;
          i += 2;
        }
      else if (!maskedword && (!premoved || i) && !started && i == size - 2 && !i && isPair (rightop) && leftop->type == AOP_IMMD &&
        getPairId (rightop) != PAIR_HL && (IS_TLCS90 || getPairId (rightop) != PAIR_IY) &&
        isPairDead (PAIR_HL, ic))
        {
          genMove_o (ASMOP_HL, 0, IC_LEFT (ic)->aop, i, 2, true, true, de_dead, true, true);
          emit3w (A_ADD, ASMOP_HL, ic->right->aop);
          started = true;
          spillPair (PAIR_HL);
          genMove_o (IC_RESULT (ic)->aop, i, ASMOP_HL, 0, 2, true, true, de_dead, true, true);
          i += 2;
        }
     else  if (!maskedword && (!premoved || i) && !started && i == size - 2 && !i && isPair (leftop) && (rightop->type == AOP_LIT  || rightop->type == AOP_IMMD) &&
       getPairId (leftop) != PAIR_HL && (IS_TLCS90 || getPairId (leftop) != PAIR_IY) &&
       isPairDead (PAIR_HL, ic))
        {
          genMove_o (ASMOP_HL, 0, IC_RIGHT (ic)->aop, i, 2, true, true, de_dead, true, true);
          emit3w (A_ADD, ASMOP_HL, ic->left->aop);
          started = true;
          spillPair (PAIR_HL);
          genMove_o (IC_RESULT (ic)->aop, i, ASMOP_HL, 0, 2, true, true, de_dead, true, true);
          i += 2;
        }
      else if (!maskedword && (!premoved || i) && !started && i == size - 2 && !i && aopInReg (leftop, i, HL_IDX) &&
        isPair (rightop) && getPairId (rightop) != PAIR_HL && (IS_TLCS90 || getPairId (rightop) != PAIR_IY) &&
        isPairDead (PAIR_HL, ic))
        {
          emit3w (A_ADD, ASMOP_HL, ic->right->aop);
          started = true;
          genMove_o (IC_RESULT (ic)->aop, i, ASMOP_HL, 0, 2, true, true, de_dead, true, true);
          i += 2;
        }
      else if (!maskedword && (!premoved || i) && !started && i == size - 2 && !i &&
        isPair (leftop) && getPairId (leftop) != PAIR_HL && (IS_TLCS90 || getPairId (leftop) != PAIR_IY) &&
        aopInReg (rightop, i, HL_IDX) && isPairDead (PAIR_HL, ic))
        {
          emit3w (A_ADD, ASMOP_HL, ic->left->aop);
          started = true;
          genMove_o (IC_RESULT (ic)->aop, i, ASMOP_HL, 0, 2, true, true, de_dead, true, true);
          i += 2;
        }
      else if (!maskedword && (!premoved || i) && !started && i == size - 2 && aopInReg (ic->result->aop, i, HL_IDX) &&
        aopInReg (rightop, i, C_IDX) && isRegDead (B_IDX, ic))
        {
          if (aopInReg (rightop, i + 1, H_IDX) || aopInReg (rightop, i + 1, L_IDX))
            {
              cheapMove (ASMOP_B, 0, ic->right->aop, i + 1, true);
              genMove_o (ASMOP_HL, 0, ic->left->aop, i, 2, false, true, de_dead, false, !started);
            }
          else
            {
              bool de_free = de_dead && !aopInReg (ic->right->aop, i + 1, E_IDX) && !aopInReg (ic->right->aop, i + 1, D_IDX);
              genMove_o (ASMOP_HL, 0, ic->left->aop, i, 2, false, true, de_free, false, !started);
              cheapMove (ASMOP_B, 0, ic->right->aop, i + 1, true);
            }
          emit3w (A_ADD, ASMOP_HL, ASMOP_BC);
          started = true;
          i += 2;
        }
       else if (!maskedword && (!premoved || i) && !started && i == size - 2 && aopInReg (IC_RESULT (ic)->aop, i, HL_IDX) &&
          aopInReg (rightop, i, E_IDX) && isRegDead (D_IDX, ic))
        {
          if (aopInReg (rightop, i + 1, H_IDX) || aopInReg (rightop, i + 1, L_IDX))
            {
              cheapMove (ASMOP_D, 0, IC_RIGHT (ic)->aop, i + 1, true);
              genMove_o (ASMOP_HL, 0, ic->left->aop, i, 2, false, true, de_dead, false, true);
            }
          else
            {
              bool de_free = de_dead && !aopInReg (ic->right->aop, i + 1, E_IDX) && !aopInReg (ic->right->aop, i + 1, D_IDX);
              genMove_o (ASMOP_HL, 0, ic->left->aop, i, 2, false, true, de_free, false, true);
              cheapMove (ASMOP_D, 0, ic->right->aop, i + 1, true);
            }
          emit3w (A_ADD, ASMOP_HL, ASMOP_DE);
          started = true;
          i += 2;
        }
       else if (!maskedword && (!premoved || i) && !started && i == size - 2 && aopInReg (IC_RESULT (ic)->aop, i, HL_IDX) &&
          aopInReg (leftop, i, E_IDX) && isRegDead (D_IDX, ic))
        {
          if (aopInReg (leftop, i + 1, H_IDX) || aopInReg (leftop, i + 1, L_IDX))
            {
              cheapMove (ASMOP_D, 0, IC_LEFT (ic)->aop, i + 1, true);
              fetchPairLong (PAIR_HL, IC_RIGHT (ic)->aop, 0, i);
            }
          else
            {
              fetchPairLong (PAIR_HL, IC_RIGHT (ic)->aop, 0, i);
              cheapMove (ASMOP_D, 0, IC_LEFT (ic)->aop, i + 1, true);
            }
          emit3w (A_ADD, ASMOP_HL, ASMOP_DE);
          started = true;
          i += 2;
        }
      // When adding a literal, the 16 bit addition results in smaller, faster code than two 8-bit additions.
      else if (!maskedword && (!premoved || i) && aopInReg (IC_RESULT (ic)->aop, i, HL_IDX) && aopInReg (leftop, i, HL_IDX) && (rightop->type == AOP_LIT && !aopIsLitVal (rightop, i, 1, 0) || rightop->type == AOP_IMMD))
        {
          PAIR_ID pair = getFreePairId (ic);
          bool pair_alive;
          if (pair == PAIR_INVALID)
            pair = PAIR_DE;
          pair_alive = !isPairDead (pair, ic) ||
            IC_RESULT (ic)->aop->regs[_pairs[pair].l_idx] < i || IC_RESULT (ic)->aop->regs[_pairs[pair].h_idx] < i ||
            IC_LEFT (ic)->aop->regs[_pairs[pair].l_idx] >= i + 2 || IC_LEFT (ic)->aop->regs[_pairs[pair].h_idx] >= i + 2;
          if (pair_alive)
            _push (pair);
          fetchPairLong (pair, IC_RIGHT (ic)->aop, 0, i);
          if (started)
            {
              emit2 ("adc hl, %s", _pairs[pair].name);
              cost2 (2, 15, 10, 4, 0, 8, 2, 2);
            }
          else
            {
              emit2 ("add hl, %s", _pairs[pair].name);
              started = TRUE;
              cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
            }
          if (pair_alive)
            _pop (pair);
          i += 2;
        }
      // When adding registers the 16 bit addition results in smaller, faster code than an 8-bit addition.
      else if (!maskedbyte && (!premoved || i) && i == size - 1 && isPairDead (PAIR_HL, ic) && aopInReg (IC_RESULT (ic)->aop, i, L_IDX)
        && (aopInReg (leftop, i, L_IDX) || aopInReg (rightop, i, L_IDX))
        && (aopInReg (leftop, i, C_IDX) || aopInReg (rightop, i, C_IDX) || aopInReg (leftop, i, E_IDX) || aopInReg (rightop, i, E_IDX)))
        {
          PAIR_ID pair = (leftop->aopu.aop_reg[i]->rIdx == C_IDX
                      || rightop->aopu.aop_reg[i]->rIdx == C_IDX) ? PAIR_BC : PAIR_DE;
          if (started)
            {
              emit2 ("adc hl, %s", _pairs[pair].name);
              cost2 (2, 15, 10, 4, 0, 8, 2, 2);
            }
          else
            {
              emit2 ("add hl, %s", _pairs[pair].name);
              started = TRUE;
              cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
            }
          i++;
        }
      // When adding a literal, the 16 bit addition results in smaller, slower code than an 8-bit addition.
      else if (!maskedbyte && (!premoved || i) && optimize.codeSize && !started && i == size - 1 && isPairDead (PAIR_HL, ic)
        && rightop->type == AOP_LIT && aopInReg (IC_RESULT (ic)->aop, i, L_IDX) && aopInReg (leftop, i, L_IDX)
        && (isRegDead (C_IDX, ic) || isRegDead (E_IDX, ic)))
        {
          PAIR_ID pair = !isRegDead (C_IDX, ic) ? PAIR_DE : PAIR_BC;
          emit2 ("ld %s, !immedbyte", _pairs[pair].l, (ulFromVal (IC_RIGHT (ic)->aop->aopu.aop_lit)) & 0xffu);
          cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          emit2 ("add hl, %s", _pairs[pair].name);
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          started = true;
          i++;
        }
      // Skip over this byte.
      else if (!maskedbyte && !premoved && !started && (leftop->type == AOP_REG || IC_RESULT (ic)->aop->type == AOP_REG) && aopIsLitVal (rightop, i, 1, 0))
        {
          cheapMove (IC_RESULT (ic)->aop, i, leftop, i, true);
          i++;
        }
      // Conditional 16-bit inc.
      else if (!maskedword && i == size - 2 && started && aopIsLitVal (rightop, i, 2, 0) && (
        aopInReg (IC_RESULT (ic)->aop, i, BC_IDX) && aopInReg (leftop, i, BC_IDX) ||
        aopInReg (IC_RESULT (ic)->aop, i, DE_IDX) && aopInReg (leftop, i, DE_IDX) ||
        aopInReg (IC_RESULT (ic)->aop, i, HL_IDX) && aopInReg (leftop, i, HL_IDX) ||
        aopInReg (IC_RESULT (ic)->aop, i, IY_IDX) && aopInReg (leftop, i, IY_IDX)))
        {
          if (!tlbl && !regalloc_dry_run)
            tlbl = newiTempLabel (0);

          if (!regalloc_dry_run)
            emit2 ("jp NC, !tlabel", labelKey2num (tlbl->key));
          cost2 (2 + IS_SM83, 12, 8, 5, 12, 12, 3, 3); // Assume branch is taken. Use cost of jr as the peephole optimizer can typically optimize this jp into jr. Do not emit jr directly to still allow jump-to-jump optimization.
          regalloc_dry_run_state_scale /= 256.0f; // Carry should be rare.
          emit3w_o (A_INC, leftop, i, 0, 0);
          i += 2;
        }
      // Conditional 8-bit inc.
      else if (!maskedbyte && i == size - 1 && started && aopIsLitVal (rightop, i, 1, 0) &&
        !aopInReg (leftop, i, A_IDX) && // adc a, #0 is cheaper than conditional inc.
        (i < leftop->size &&
        leftop->type == AOP_REG && IC_RESULT (ic)->aop->type == AOP_REG &&
        leftop->aopu.aop_reg[i]->rIdx == IC_RESULT (ic)->aop->aopu.aop_reg[i]->rIdx &&
        (HAS_IYL_INST || leftop->aopu.aop_reg[i]->rIdx != IYL_IDX && leftop->aopu.aop_reg[i]->rIdx != IYH_IDX) ||
        leftop->type == AOP_STK && leftop == IC_RESULT (ic)->aop ||
        leftop->type == AOP_PAIRPTR && leftop->aopu.aop_pairId == PAIR_HL))
        {
          if (!tlbl && !regalloc_dry_run)
            tlbl = newiTempLabel (0);
          if (!regalloc_dry_run)
            emit2 ("jp NC, !tlabel", labelKey2num (tlbl->key));
          cost2 (2 + IS_SM83, 12, 8, 5, 12, 12, 3, 3); // Assume branch is taken. Use cost of jr as the peephole optimizer can typically optimize this jp into jr. Do not emit jr directly to still allow jump-to-jump optimization.
          regalloc_dry_run_state_scale /= 256.0f; // Carry should be rare.
          emit3_o (A_INC, leftop, i, 0, 0);
          i++;
        }
      else if (!started && !premoved && aopIsLitVal (leftop, i, 1, 0))
        {
          cheapMove (ic->result->aop, i, rightop, i, true);
          i++;
        }
      else
        {
          if (!HAS_IYL_INST && (aopInReg (rightop, i, IYL_IDX) || aopInReg (rightop, i, IYH_IDX)))
            if (!premoved && !aopInReg (leftop, i, IYL_IDX) && !aopInReg (leftop, i, IYL_IDX))
              {
                operand *t = IC_RIGHT (ic);
                IC_RIGHT (ic) = IC_LEFT (ic);
                IC_LEFT (ic) = t;
                leftop = IC_LEFT (ic)->aop;
                rightop = IC_RIGHT (ic)->aop;
              }
            else // Can't handle both sides in iy.
              UNIMPLEMENTED;
          else if (rightop->type == AOP_STL && i < 2) // can't handle rematerialized stack location on the right efficiently.
            {
              operand *t = IC_RIGHT (ic);
              IC_RIGHT (ic) = IC_LEFT (ic);
              IC_LEFT (ic) = t;
              leftop = IC_LEFT (ic)->aop;
              rightop = IC_RIGHT (ic)->aop;
            }

          if (aopInReg (rightop, i, A_IDX) && !aopInReg (leftop, i, A_IDX)) // Make sure we don't overwrite the other operand.
            UNIMPLEMENTED;
          else if (!premoved)
            cheapMove (ASMOP_A, 0, leftop, i, true);
          else
            premoved = FALSE;

          if (!started && aopIsLitVal (rightop, i, 1, 0))
            ; // Skip over this byte.
          // We can use inc / dec only for the only, top non-zero byte, since it neither takes into account an existing carry nor does it update the carry.
          else if (!started && i == size - 1 && (aopIsLitVal (rightop, i, 1, 1) || aopIsLitVal (rightop, i, 1, 255)))
            {
              emit3 (aopIsLitVal (rightop, i, 1, 1) ? A_INC : A_DEC, ASMOP_A, 0);
              started = true;
            }
          else if (rightop->type == AOP_STL && i < 2)
            {
              _push (PAIR_HL);
              genMove (ASMOP_HL, rightop, false, true, false, false);
              emit3 (started ? A_ADC : A_ADD, ASMOP_A, i ? ASMOP_H : ASMOP_L);
              started = true;
              _pop (PAIR_HL);
            }
          else if (!HAS_IYL_INST && (aopInReg (rightop, i, IYL_IDX) || aopInReg (rightop, i, IYH_IDX)))
            UNIMPLEMENTED;
          else
            {
              emit3_o (started ? A_ADC : A_ADD, ASMOP_A, 0, rightop, i);
              started = true;
            }
          if (maskedbyte)
            {
              emit2 ("and a, #0x%02x", topbytemask);
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
            }

          _G.preserveCarry = (i != size - 1);
          if (size &&
            (requiresHL (rightop) && rightop->size > i + 1 && rightop->type != AOP_REG || (requiresHL (leftop) && leftop->size > i + 1)
            && leftop->type != AOP_REG) && IC_RESULT (ic)->aop->type == AOP_REG
            && (IC_RESULT (ic)->aop->aopu.aop_reg[i]->rIdx == L_IDX
              || IC_RESULT (ic)->aop->aopu.aop_reg[i]->rIdx == H_IDX))
            {
              wassert (cached[0] == -1 || cached[1] == -1);
              cached[cached[0] == -1 ? 0 : 1] = offset++;
              _push (PAIR_AF);
            }
          // Avoid overwriting still-needed operand in h or l.
          else if (requiresHL (IC_RESULT (ic)->aop) && IC_RESULT (ic)->aop->type != AOP_REG && (IC_RESULT (ic)->aop->type == AOP_EXSTK || IS_SM83 || IC_RESULT (ic)->aop->type == AOP_PAIRPTR) &&
            (!isPairDead(PAIR_HL, ic) || i + 1 < size && IC_LEFT(ic)->aop->regs[L_IDX] > i || i + 1 < size && IC_LEFT(ic)->aop->regs[H_IDX] > i || i + 1 < size && IC_RIGHT(ic)->aop->regs[L_IDX] > i || i + 1 < size && IC_RIGHT(ic)->aop->regs[H_IDX] > i))
            {
              _push (PAIR_HL);
              cheapMove (IC_RESULT (ic)->aop, i, ASMOP_A, 0, true);
              _pop (PAIR_HL);
            }
          else
            cheapMove (IC_RESULT (ic)->aop, i, ASMOP_A, 0, true);
          i++;
        }
    }
  _G.preserveCarry = false;

  regalloc_dry_run_state_scale = 1.0f;
  if (tlbl)
    emitLabel (tlbl);

  for (size = 1; size >= 0; size--)
    if (cached[size] != -1)
      {
        if (IC_RESULT (ic)->aop->regs[A_IDX] >= 0 && IC_RESULT (ic)->aop->regs[A_IDX] != size) // Don't overwrite still-needed a below.
          UNIMPLEMENTED;
        _pop (PAIR_AF);
        cheapMove (IC_RESULT (ic)->aop, cached[size], ASMOP_A, 0, true);
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
  unsigned int size = IC_RESULT (ic)->aop->size;

  /* will try to generate a decrement */
  /* if the right side is not a literal we cannot */
  if (right->type != AOP_LIT)
    return false;

  /* if the literal value of the right hand side
     is greater than 4 then it is not worth it */
  if ((icount = (unsigned int) ulFromVal (right->aopu.aop_lit)) > 2)
    return false;
  /* if decrement 16 bits in register */
  if (sameRegs (left, result) && (size > 1) && isPair (result))
    {
      while (icount--)
        {
          emit2 ("dec %s", getPairName (result));
          if (getPairId (result) == PAIR_IY)
            cost2 (2, 10, 7, 4, 0, 4, 2, 2);
          else
            cost2 (1, 6, 4, 2, 8, 4, 1, 1);
        }
      return true;
    }

  /* If result is a pair */
  if (isPair (IC_RESULT (ic)->aop))
    {
      fetchPair (getPairId (result), left);
      while (icount--)
        {
          if (!regalloc_dry_run)
            emit2 ("dec %s", getPairName (result));
          if (getPairId (result) == PAIR_IY)
            cost2 (2, 10, 7, 4, 0, 4, 2, 2);
          else
            cost2 (1, 6, 4, 2, 8, 4, 1, 1);
        }
      return true;
    }

  /* if decrement 16 bits in register */
  if (sameRegs (left, result) && size == 2 && isPairDead (_getTempPairId (), ic) && !(requiresHL (left) && _getTempPairId () == PAIR_HL))
    {
      fetchPair (_getTempPairId (), left);

      while (icount--)
        {
          if (!regalloc_dry_run)
            emit2 ("dec %s", _getTempPairName ());
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
        }

      commitPair (result, _getTempPairId (), ic, false);

      return true;
    }


  /* if the sizes are greater than 1 then we cannot */
  if (result->size > 1 || left->size > 1)
    return false;

  /* we can if the aops of the left & result match or if they are in
     registers and the registers are the same */
  if (sameRegs (left, result))
    {
      while (icount--)
        emit3 (A_DEC, result, 0);
      return true;
    }

  if (result->type == AOP_REG)
    {
      cheapMove (result, 0, left, 0, true);
      while (icount--)
        emit3 (A_DEC, result, 0);
      return true;
    }

  return false;
}

/*-----------------------------------------------------------------*/
/* genSub - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
static void
genSub (const iCode *ic, asmop *result, asmop *left, asmop *right)
{
  int size, offset = 0;
  unsigned long long lit = 0L;

  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  /* special cases :- */
  /* if both left & right are in bit space */
  if (left->type == AOP_CRY && right->type == AOP_CRY)
    {
      wassertl (0, "Tried to subtract two bits");
      return;
    }

  /* if I can do an decrement instead of subtract then GOOD for ME */
  if (!maskedtopbyte && genMinusDec (ic, result, left, right) == TRUE)
    return;

  size = IC_RESULT (ic)->aop->size;

  if (right->type == AOP_LIT)
    {
      lit = ullFromVal (right->aopu.aop_lit);
      lit = -(long long) lit;
    }

  /* Same logic as genPlus */
  if (!maskedtopbyte && IS_SM83)
    {
      if (left->type == AOP_STK || right->type == AOP_STK || result->type == AOP_STK)
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

              emit2 ("ld a, %s", _pairs[leftp].l);
              cost2 (1, 4, 4, 2, 4, 2, 1, 1);
              emit2 ("sub a, %s", _pairs[rightp].l);
              cost2 (1 + IS_TLCS90, 4, 4, 2, 4, 4, 1, 1);
              emit3 (A_LD, ASMOP_E, ASMOP_A);
              emit2 ("ld a, %s", _pairs[leftp].h);
              cost2 (1, 4, 4, 2, 4, 2, 1, 1);
              emit2 ("sbc a, %s", _pairs[rightp].h);
              cost2 (1 + IS_TLCS90, 4, 4, 2, 4, 4, 1, 1);

              if (IC_RESULT (ic)->aop->size > 1)
                cheapMove (IC_RESULT (ic)->aop, 1, ASMOP_A, 0, true);
              cheapMove (IC_RESULT (ic)->aop, 0, ASMOP_E, 0, true);
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

  if ((requiresHL (result) && result->type != AOP_REG || requiresHL (left) && left->type != AOP_REG || requiresHL (right) && right->type != AOP_REG) &&
    (left->regs[L_IDX] > 0 || left->regs[H_IDX] > 0 || right->regs[L_IDX] > 0 || right->regs[H_IDX] > 0))
    UNIMPLEMENTED;
  setupToPreserveCarry (result, left, right);

  /* if literal right, add a, #-lit, else normal subb */
  while (size)
    {
      bool maskedbyte = maskedtopbyte && (size == 1);
      bool maskedword = maskedtopbyte && (size == 2);

      if (!IS_SM83 && !maskedword && size >= 2 &&
        isPairDead (PAIR_HL, ic) &&
        (aopInReg (result, offset, HL_IDX) || aopInReg (result, offset, DE_IDX) || IS_RAB && result->type == AOP_STK) &&
        (result->regs[L_IDX] < 0 || result->regs[L_IDX] >= offset) && (result->regs[H_IDX] < 0 || result->regs[H_IDX] >= offset) &&
        (aopInReg (left, offset, HL_IDX) || (left->type == AOP_LIT || left->type == AOP_IY) && right->regs[L_IDX] < offset && right->regs[H_IDX] < offset) &&
        (aopInReg (right, offset, BC_IDX) || aopInReg (right, offset, DE_IDX) || ((right->type == AOP_IY || right->type == AOP_HL || IS_RAB && right->type == AOP_STK) && (getFreePairId (ic) == PAIR_DE || getFreePairId (ic) == PAIR_BC))))
        {
          PAIR_ID rightpair;

          bool a_dead = isRegDead (A_IDX, ic) && (result->regs[A_IDX] < 0 || result->regs[A_IDX] >= offset);

          if (getPartPairId (right, offset) == PAIR_INVALID)
            {
              rightpair = getFreePairId (ic);
              genMove_o (rightpair == PAIR_DE ? ASMOP_DE : ASMOP_BC, 0, right, offset, 2, a_dead, !aopInReg (left, offset, HL_IDX), false, true, !offset);
            }
          else
            rightpair = getPartPairId (right, offset);
          if (!aopInReg (left, offset, HL_IDX))
            genMove_o (ASMOP_HL, 0, left, offset, 2, a_dead, true, false, true, !offset);

          if (!offset)
            emit3 (A_CP, ASMOP_A, ASMOP_A);
          emit2 ("sbc hl, %s", _pairs[rightpair].name);
          cost2 (2, 15, 10, 4, 0, 8, 2, 2);
          spillPair (PAIR_HL);
          genMove_o (result, offset, ASMOP_HL, 0, 2, a_dead, true, false, true, size <= 2);
          offset += 2;
          size -= 2;
          _G.preserveCarry = !!size;
          continue;
        }
        
      bool l_dead = !(!isRegDead (L_IDX, ic) || left->regs[L_IDX] > offset || right->regs[L_IDX] > offset || result->regs[L_IDX] >= 0 && result->regs[L_IDX] < offset);
      bool h_dead = !(!isRegDead (H_IDX, ic) || left->regs[H_IDX] > offset || right->regs[H_IDX] > offset || result->regs[H_IDX] >= 0 && result->regs[H_IDX] < offset);
      bool hl_dead = l_dead && h_dead;
      bool pushed_hl = false;

      if (right->type == AOP_SFR) // Right operand needs to go through a
        {
          asmop *tmpaop;

          if (aopInReg (left, offset, H_IDX))
            tmpaop = ASMOP_L;
          else if (aopInReg (left, offset, L_IDX))
            tmpaop = ASMOP_H;
          else if (!l_dead && h_dead)
            tmpaop = ASMOP_H;
          else
            tmpaop = ASMOP_L;

          bool tmpaop_dead = aopInReg (tmpaop, 0, L_IDX) ? l_dead : h_dead;
          if (!tmpaop_dead)
            {
              _push (PAIR_HL);
              pushed_hl = true;
            }

          if (aopInReg (left, offset, A_IDX) ||
            (aopInReg (tmpaop, 0, L_IDX) || aopInReg (tmpaop, 0, H_IDX)) && requiresHL (left))
            {
              cheapMove (ASMOP_A, 0, left, offset, true);
              cheapMove (tmpaop, 0, right, offset, false);
            }
          else
            {
              cheapMove (tmpaop, 0, right, offset, true);
              cheapMove (ASMOP_A, 0, left, offset, true);
            }
          emit3_o (offset ? A_SBC : A_SUB, ASMOP_A, 0, tmpaop, 0);
        }
      else if (right->type != AOP_LIT)
        {
          if ((requiresHL (left) && left->type != AOP_REG || requiresHL (right) && right->type != AOP_REG) && !hl_dead)
            {
              _push (PAIR_HL);
              pushed_hl = true;
            }

          if ((aopInReg (right, offset, IYL_IDX)  || aopInReg (right, offset, IYH_IDX)) && !HAS_IYL_INST) // From here on all codepaths needs to use right as operand.
            UNIMPLEMENTED;
          else if (right->type == AOP_STL && offset < 2)
            {
              cheapMove (ASMOP_A, 0, left, offset, true);
              if (!hl_dead && !pushed_hl)
                {
                  _push (PAIR_HL);
                  pushed_hl = true;
                }
              genMove (ASMOP_HL, right, false, true, false, false);
              emit3 (offset ? A_SBC : A_SUB, ASMOP_A, offset ? ASMOP_H : ASMOP_L);
            }
          else if (!offset)
            {
              if (aopIsLitVal (left, offset, 1, 0x00) && aopInReg (right, offset, A_IDX))
                emit3 (A_NEG, 0, 0);
              else
                {
                  if (aopIsLitVal (left, offset, 1, 0x00) && !aopInReg (left, offset, A_IDX))
                    emit3 (A_XOR, ASMOP_A, ASMOP_A);
                  else
                    cheapMove (ASMOP_A, 0, left, offset, true);
                  if ((aopInReg (right, offset, L_IDX) || aopInReg (right, offset, H_IDX)) && pushed_hl)
                    {
                      _pop (PAIR_HL);
                      pushed_hl = false;
                    }
                  emit3_o (A_SUB, ASMOP_A, 0, right, offset);
                }
            }
          else if (aopIsLitVal (left, offset, 1, 0x00) && !aopInReg (left, offset, A_IDX) && size == 1) // For the last byte, we can do an optimization that results in the same value in a, but different carry.
            {
              emit3 (A_SBC, ASMOP_A, ASMOP_A);
              emit3_o (A_SUB, ASMOP_A, 0, right, offset);
            }
          else
            {
              cheapMove (ASMOP_A, 0, left, offset, true);
              if ((aopInReg (right, offset, L_IDX) || aopInReg (right, offset, H_IDX)) && pushed_hl)
                {
                  _pop (PAIR_HL);
                  pushed_hl = false;
                }
              emit3_o (A_SBC, ASMOP_A, 0, right, offset);
            }
        }
      else // right is a literal.
        {
          if (requiresHL (left) && left->type != AOP_REG && !hl_dead)
            {
              _push (PAIR_HL);
              pushed_hl = true;
            }

          cheapMove (ASMOP_A, 0, left, offset, true);

          /* first add without previous c */
          if (!offset)
            {
              if (size == 0 && (unsigned int) (lit & 0x0FFL) == 0xFF)
                emit3 (A_DEC, ASMOP_A, 0);
              else
                {
                  if (!regalloc_dry_run)
                    emit2 ("add a, !immedbyte", (unsigned int)(lit & 0x0fful));
                  cost2 (2, 7, 6, 4, 8, 4, 2, 2);
                }
            }
          else
            emit2 ("adc a, !immedbyte", (unsigned int)((lit >> (offset * 8)) & 0x0fful));
        }

      if (maskedbyte)
        {
          emit2 ("and a, #0x%02x", topbytemask);
          cost2 (2, 7, 6, 4, 8, 4, 2, 2);
        }

      if (pushed_hl)
        _pop (PAIR_HL);
      size--;
      _G.preserveCarry = !!size;
      cheapMove (result, offset++, ASMOP_A, 0, true);

      if ((left->type == AOP_PAIRPTR && left->aopu.aop_pairId == PAIR_HL || right->type == AOP_PAIRPTR && right->aopu.aop_pairId == PAIR_HL) &&
        size &&
        (aopInReg (result, offset, L_IDX) || aopInReg (result, offset, H_IDX)))
        UNIMPLEMENTED;
    }

}

/*-----------------------------------------------------------------*/
/* genMinus - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
static void
genMinus (const iCode *ic, const iCode *ifx)
{
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RIGHT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, FALSE);

  if (ifx && ifx->generated)
    {
      wassert (ic->result->aop->size == 1 && IS_OP_LITERAL (ic->right) && ullFromVal (OP_VALUE (ic->right)) == 1);

      if (ic->result->aop->type == AOP_REG && (!aopInReg (ic->result->aop, 0, IYL_IDX) && !aopInReg (ic->result->aop, 0, IYH_IDX) || HAS_IYL_INST))
        {
          cheapMove (ic->result->aop, 0, ic->left->aop, 0, isRegDead (A_IDX, ic));
          emit3 (A_DEC, ic->result->aop, 0);
          if (!IS_SM83 && aopInReg (ic->result->aop, 0, B_IDX) && IC_TRUE (ifx)) // This jump can likely be optimized to djnz.
            {
              // cost2 can't handle negative costs, so we do this manually.
              regalloc_dry_run_cost_bytes--; 
              if (IS_Z80 || IS_Z80N || IS_Z180)
                regalloc_dry_run_cost_states += -3.0 * regalloc_dry_run_state_scale;
              else if (IS_RAB)
                regalloc_dry_run_cost_states += -2.0 * regalloc_dry_run_state_scale;
              else if (IS_TLCS90)
                regalloc_dry_run_cost_states += +2.0 * regalloc_dry_run_state_scale; // For the TLCS-90, djnz is slower (typically still worth it for code size, though).
              else if (IS_EZ80_Z80 || IS_R800)
                regalloc_dry_run_cost_states += -1.0 * regalloc_dry_run_state_scale;
            }
        }
      else
        {
          if (!isRegDead (A_IDX, ic))
            UNIMPLEMENTED;
          cheapMove (ASMOP_A, 0, ic->left->aop, 0, true);
          emit3 (A_DEC, ASMOP_A, 0);
          cheapMove (ic->result->aop, 0, ASMOP_A, 0, true);
        }
      if (IC_TRUE (ifx))
        emit2 ("jp NZ, !tlabel", labelKey2num (IC_TRUE (ifx)->key));
      else
        emit2 ("jp Z, !tlabel", labelKey2num (IC_FALSE (ifx)->key));
      cost2 (2, 9.5f, 7.0f, 5.0f, 10.0f, 6.0f, 2.5f, 2.5f); // Assume both branches equally likely. Assume jp will be optimized to jr.
    }
  else
    genSub (ic, ic->result->aop, ic->left->aop, ic->right->aop);

  _G.preserveCarry = FALSE;
  freeAsmop (IC_LEFT (ic), NULL);
  freeAsmop (IC_RIGHT (ic), NULL);
  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genUminusFloat - unary minus for floating points                */
/*-----------------------------------------------------------------*/
static void
genUminusFloat (const iCode *ic, operand *result, operand *op)
{
  emitDebug ("; genUminusFloat");

  /* for this we just need to flip the
     first bit then copy the rest in place */
     
  if (!isRegDead (A_IDX, ic))
    _push (PAIR_AF);

  cheapMove (ASMOP_A, 0, op->aop, MSB32, true);

  emit2 ("xor a,!immedbyte", 0x80u);
  cost2 (2, 7, 6, 4, 8, 4, 2, 2);
  cheapMove (result->aop, MSB32, ASMOP_A, 0, true);

  genMove_o (result->aop, 0, op->aop, 0, op->aop->size - 1, !aopInReg (result->aop, MSB32, A_IDX), false, false, true, true);
  
  if (!isRegDead (A_IDX, ic))
    _pop (PAIR_AF);
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
  if (IC_RESULT (ic)->aop->type == AOP_CRY && IC_LEFT (ic)->aop->type == AOP_CRY)
    {
      wassertl (0, "Left and right are in bit space");
      goto release;
    }

  if (IS_FLOAT (operandType (IC_LEFT (ic))))
    genUminusFloat (ic, IC_RESULT (ic), IC_LEFT (ic));
  else
    genSub (ic, IC_RESULT (ic)->aop, ASMOP_ZERO, IC_LEFT (ic)->aop);

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
  bool savedB = false;

  asmop *result = ic->result->aop;
  int resultsize = result->size;

  if (ic->left->aop->size > 1 || ic->right->aop->size > 2)
    wassertl (0, "Large multiplication is handled through support function calls.");

  if (IS_SM83)
    {
      wassertl (0, "Multiplication is handled through support function calls on sm83");
      return;
    }

  if ((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) && IC_RESULT (ic)->aop->type == AOP_REG)
    {
      if (!IS_Z80N && (resultsize > 1 ? result->aopu.aop_reg[1]->rIdx == B_IDX : isRegDead (B_IDX, ic))
          && result->aopu.aop_reg[0]->rIdx == C_IDX)
        {
          if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == C_IDX ||
              IC_RIGHT (ic)->aop->type == AOP_REG && IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == B_IDX)
            {
              cheapMove (ASMOP_C, 0, IC_LEFT (ic)->aop, LSB, true);
              cheapMove (ASMOP_B, 0, IC_RIGHT (ic)->aop, LSB, true);
            }
          else
            {
              cheapMove (ASMOP_B, 0, IC_LEFT (ic)->aop, LSB, true);
              cheapMove (ASMOP_C, 0, IC_RIGHT (ic)->aop, LSB, true);
            }
          emit2 ("mlt bc");
          cost2 (2, 8, 17, 0, 0, 0, 6, 0);
          return;
        }
      if ((resultsize > 1 ? result->aopu.aop_reg[1]->rIdx == D_IDX : isRegDead (D_IDX, ic))
          && result->aopu.aop_reg[0]->rIdx == E_IDX)
        {
          if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX ||
              IC_RIGHT (ic)->aop->type == AOP_REG && IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == D_IDX)
            {
              cheapMove (ASMOP_E, 0, IC_LEFT (ic)->aop, LSB, true);
              cheapMove (ASMOP_D, 0, IC_RIGHT (ic)->aop, LSB, true);
            }
          else
            {
              cheapMove (ASMOP_D, 0, IC_LEFT (ic)->aop, LSB, true);
              cheapMove (ASMOP_E, 0, IC_RIGHT (ic)->aop, LSB, true);
            }
          emit2 ("mlt de");
          cost2 (2, 8, 17, 0, 0, 0, 6, 0);
          return;
        }
      if (!IS_Z80N && IC_LEFT (ic)->aop->type == AOP_REG && IC_RIGHT (ic)->aop->type == AOP_REG &&
          ((IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == H_IDX && IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == L_IDX ||
            IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == L_IDX && IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == H_IDX) &&
           (resultsize > 1 ? result->aopu.aop_reg[1]->rIdx == H_IDX : isRegDead (H_IDX, ic))
           && result->aopu.aop_reg[0]->rIdx == L_IDX))
        {
          emit2 ("mlt hl");
          spillPair (PAIR_HL);
          cost2 (2, 8, 17, 0, 0, 0, 6, 0);
          return;
        }
    }

  if (IS_RAB && !IS_R2K && isPairDead (PAIR_HL, ic) && isPairDead (PAIR_BC, ic)) // A wait state bug makes mul unuseable in most scenarios on the original Rabbit 2000.
    {
      const bool save_de = (resultsize > 1 && !isRegDead (D_IDX, ic) ||
        !isRegDead (E_IDX, ic) && !(IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX) && !(IC_RIGHT (ic)->aop->type == AOP_REG && IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX));
      if (save_de)
        _push (PAIR_DE);

      if (aopInReg (ic->right->aop, 0, E_IDX))
        cheapMove (ASMOP_C, 0, ic->left->aop, 0, true);
      else if (aopInReg (ic->left->aop, 0, E_IDX))
        cheapMove (ASMOP_C, 0, ic->right->aop, 0, true);
      else if (aopInReg (ic->right->aop, 0, C_IDX))
        cheapMove (ASMOP_E, 0, ic->left->aop, 0, true);
      else if (aopInReg (ic->left->aop, 0, C_IDX))
        cheapMove (ASMOP_E, 0, ic->right->aop, 0, true);
      else
        {
          cheapMove (ASMOP_C, 0, IC_LEFT (ic)->aop, 0, true);
          cheapMove (ASMOP_E, 0, IC_RIGHT (ic)->aop, 0, true);
        }

      if (resultsize > 1)
        {
          cheapMove (ASMOP_D, 0, ASMOP_ZERO, 0, true);
          cheapMove (ASMOP_B, 0, ASMOP_D, 0, true);
        }

      emit2 ("mul");
      cost (1, 12);
      spillPair (PAIR_HL);

      genMove (result, resultsize > 1 ? ASMOP_BC : ASMOP_C, !isRegDead (A_IDX, ic), true, false, true);

      if (save_de)
        _pop (PAIR_DE);
      return;
    }

  if (IS_R800 && isRegDead (HL_IDX, ic) && isRegDead (A_IDX, ic))
    {
      if (aopInReg (ic->right->aop, 0, C_IDX) || aopInReg (ic->right->aop, 0, B_IDX) || aopInReg (ic->right->aop, 0, E_IDX) || aopInReg (ic->right->aop, 0, D_IDX))
        {
          cheapMove (ASMOP_A, 0, ic->left->aop, 0, true);
          if (!regalloc_dry_run)
            emit2 ("multu a, %s", aopGet (ic->right->aop, 0, false));
          cost (2, 14);
          goto store_hl;
        }
      else if (aopInReg (ic->left->aop, 0, C_IDX) || aopInReg (ic->left->aop, 0, B_IDX) || aopInReg (ic->left->aop, 0, E_IDX) || aopInReg (ic->left->aop, 0, D_IDX))
        {
          cheapMove (ASMOP_A, 0, ic->right->aop, 0, true);
          if (!regalloc_dry_run)
            emit2 ("multu a, %s", aopGet (ic->left->aop, 0, false));
          cost (2, 14);
          goto store_hl;
        }
      else if (isRegDead (B_IDX, ic))
        {
          if (aopInReg (ic->right->aop, 0, A_IDX))
            cheapMove (ASMOP_B, 0, ic->left->aop, 0, false);
          else
            {
              cheapMove (ASMOP_A, 0, ic->left->aop, 0, true);
              cheapMove (ASMOP_B, 0, ic->right->aop, 0, false);
            }
          emit2 ("multu a, b");
          cost (2, 14);
          goto store_hl;
        }
      else
        UNIMPLEMENTED;
      return;
    }

  if (!isPairDead (PAIR_DE, ic))
    {
      _push (PAIR_DE);
      _G.stack.pushedDE = TRUE;
    }
  if (IS_RAB && !isPairDead (PAIR_BC, ic) ||
  !(IS_Z180 || IS_EZ80_Z80) && !isRegDead (B_IDX, ic))
    {
      _push (PAIR_BC);
      savedB = TRUE;
    }

  // genMult() already swapped operands if necessary.
  if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX ||
      IC_RIGHT (ic)->aop->type == AOP_REG && IC_RIGHT (ic)->aop->aopu.aop_reg[0]->rIdx == H_IDX
      && !requiresHL (IC_LEFT (ic)->aop))
    {
      cheapMove (ASMOP_E, 0, IC_LEFT (ic)->aop, 0, true);
      cheapMove (ASMOP_H, 0, IC_RIGHT (ic)->aop, 0, true);
    }
  else
    {
      cheapMove (ASMOP_E, 0, IC_RIGHT (ic)->aop, 0, true);
      cheapMove (ASMOP_H, 0, IC_LEFT (ic)->aop, 0, true);
    }

  if (IS_Z180 || IS_EZ80_Z80)
    {
      emit3 (A_LD, ASMOP_L, ASMOP_E);
      emit2 ("mlt hl");
      cost2 (2, 8, 17, 0, 0, 0, 6, 0);
    }
  else if (IS_RAB && !IS_R2K) // A wait state bug makes mul unuseable in most scenarios on the original Rabbit 2000.
    {
      emit3 (A_LD, ASMOP_C, ASMOP_H);
      emit2 ("ld d, !immedbyte", 0x00u);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      emit3 (A_LD, ASMOP_B, ASMOP_D);
      emit2 ("mul");
      cost (1, 12);
      emit3 (A_LD, ASMOP_L, ASMOP_C);
      emit3 (A_LD, ASMOP_H, ASMOP_B);
    }
  else
    {
      tlbl1 = regalloc_dry_run ? 0 : newiTempLabel (0);
      tlbl2 = regalloc_dry_run ? 0 : newiTempLabel (0);
      emit2 ("ld l, !immedbyte", 0x00u);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      emit3 (A_LD, ASMOP_D, ASMOP_L);
      emit2 ("ld b, !immedbyte", 0x08u);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      regalloc_dry_run_state_scale = 8.0f;
      if (!regalloc_dry_run)
        emitLabel (tlbl1);
      emit3w (A_ADD, ASMOP_HL, ASMOP_HL);
      if (!regalloc_dry_run)
        emit2 ("jp NC, !tlabel", labelKey2num (tlbl2->key));
      cost2 (2 + IS_SM83, 9.5f,	7.0f, 5.0f, 10.0f, 11.0f, 2.5f, 2.5f);
      regalloc_dry_run_state_scale = 4.0f;
      emit3w (A_ADD, ASMOP_HL, ASMOP_DE);
      emitLabel (tlbl2);
      if (!regalloc_dry_run)
        emit2 ("djnz !tlabel", labelKey2num (tlbl1->key));
      cost2 (2, 12.375f, 8.75f, 5.0f, 0, 10.0f, 3.75f, 2.0f);
      regalloc_dry_run_state_scale = 1.0f;
    }

store_hl:
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

  genMove (result, ASMOP_HL, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), true);
}

/*----------------------------------------------------------------------*/
/* genMultTwoChar - generates code for 16x16->(16 to 32) multiplication */
/*----------------------------------------------------------------------*/
static void
genMultTwoChar (const iCode *ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  wassert (IS_RAB && !IS_R2K); // mul instruction is broken on Rabbit 2000.
  
  bool save_bc = !isPairDead(PAIR_BC, ic);
  bool save_de = !isPairDead(PAIR_DE, ic) && getPairId (left->aop) != PAIR_DE && getPairId (right->aop) != PAIR_DE;

  if (save_bc)
    _push (PAIR_BC);
  if (save_de)
    _push (PAIR_DE);
    
  if (getPairId (left->aop) == PAIR_BC || getPairId (right->aop) == PAIR_DE)
    {
      if (right->aop->regs[C_IDX] >= 0 || right->aop->regs[B_IDX] >= 0)
        UNIMPLEMENTED;
      genMove (ASMOP_BC, left->aop, isRegDead (A_IDX, ic), right->aop->regs[L_IDX] < 0 && right->aop->regs[H_IDX] < 0, right->aop->regs[E_IDX] < 0 && right->aop->regs[D_IDX] < 0, true);
      genMove (ASMOP_DE, right->aop, isRegDead (A_IDX, ic), true, true, true);
    }
  else
    {
      if (left->aop->regs[C_IDX] >= 0 || left->aop->regs[B_IDX] >= 0)
        UNIMPLEMENTED;
      genMove (ASMOP_BC, right->aop, isRegDead (A_IDX, ic), left->aop->regs[L_IDX] < 0 && left->aop->regs[H_IDX] < 0, left->aop->regs[E_IDX] < 0 && left->aop->regs[D_IDX] < 0, true);
      genMove (ASMOP_DE, left->aop, isRegDead (A_IDX, ic), true, true, true);
    }

  emit2 ("mul");
  cost (1, 12);

  if (save_de)
    _pop (PAIR_DE);

  genMove (ic->result->aop, ASMOP_HLBC, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), true, true);

  if (save_bc)
    {
      if (ic->result->aop->regs[B_IDX] >= 0)
        poppairwithsavedreg (PAIR_BC, B_IDX, -1);
      else if (ic->result->aop->regs[C_IDX] >= 0)
        poppairwithsavedreg (PAIR_BC, C_IDX, -1);
      else
        _pop (PAIR_BC);
    }
}

/*-----------------------------------------------------------------*/
/* genMult - generates code for multiplication                     */
/*-----------------------------------------------------------------*/
static void
genMult (iCode *ic)
{
  int val;
  /* If true then the final operation should be a subtract */
  bool active = false;
  bool byteResult;
  bool add_in_hl = false;
  int a_cost = 0, l_cost = 0;
  PAIR_ID pair;

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RIGHT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, FALSE);

  byteResult = (IC_RESULT (ic)->aop->size == 1);

  if (IC_LEFT (ic)->aop->size > 2 || IC_RIGHT (ic)->aop->size > 2)
    wassertl (0, "Large multiplication is handled through support function calls.");

  /* Swap left and right such that right is a literal */
  if (IC_LEFT (ic)->aop->type == AOP_LIT)
    {
      operand *t = IC_RIGHT (ic);
      IC_RIGHT (ic) = IC_LEFT (ic);
      IC_LEFT (ic) = t;
    }

  if (IS_RAB && !IS_R2K && IC_RIGHT (ic)->aop->type != AOP_LIT && !byteResult && IC_LEFT (ic)->aop->size == 2 && IC_RIGHT (ic)->aop->size == 2)
    {
      genMultTwoChar (ic);
      goto release;
    }  
  else if (IC_RIGHT (ic)->aop->type != AOP_LIT)
    {
      genMultOneChar (ic);
      goto release;
    }

  wassertl (IC_RIGHT (ic)->aop->type == AOP_LIT, "Right must be a literal.");

  val = (int) ulFromVal (IC_RIGHT (ic)->aop->aopu.aop_lit);
  wassertl (val != 1, "Can't multiply by 1");

  // Try to use mlt.
  if ((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) && IC_LEFT (ic)->aop->size == 1 && IC_RIGHT (ic)->aop->size == 1 &&
    (byteResult || SPEC_USIGN (getSpec (operandType (IC_LEFT (ic)))) && SPEC_USIGN (getSpec (operandType (IC_RIGHT (ic))))))
    {
      pair = getPairId (IC_RESULT (ic)->aop);
      if (pair == PAIR_INVALID && IC_RESULT (ic)->aop->type == AOP_REG)
        {
          if (isRegDead (H_IDX, ic) && IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == L_IDX)
            pair = PAIR_HL;
          else if (isRegDead (D_IDX, ic) && IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX)
            pair = PAIR_HL;
          else if (isRegDead (B_IDX, ic) && IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == C_IDX)
            pair = PAIR_HL;
        }
      else if (pair == PAIR_INVALID)
        pair = getDeadPairId (ic);

      if (pair == PAIR_INVALID)
        {
          if (!(IC_RESULT (ic)->aop->type == AOP_REG &&
            (IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == L_IDX || IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == H_IDX ||
            !byteResult && (IC_RESULT (ic)->aop->aopu.aop_reg[1]->rIdx == L_IDX || IC_RESULT (ic)->aop->aopu.aop_reg[1]->rIdx == H_IDX))))
            pair = PAIR_HL;
          else if (!(IC_RESULT (ic)->aop->type == AOP_REG &&
            (IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX || IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == D_IDX ||
            !byteResult && (IC_RESULT (ic)->aop->aopu.aop_reg[1]->rIdx == E_IDX || IC_RESULT (ic)->aop->aopu.aop_reg[1]->rIdx == D_IDX))))
            pair = PAIR_DE;
          else
            pair = PAIR_BC;
        }

      // For small operands under low register pressure, the standard approach is better than the mlt one.
      if (byteResult && val <= 6 && isPairDead (PAIR_HL, ic) && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)) &&
        !(IC_RESULT (ic)->aop->type == AOP_REG && (IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX || IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx == C_IDX)))
        goto no_mlt;

      if (IS_Z80N && pair != PAIR_DE)
        goto no_mlt;

      asmop *pairop = pair == PAIR_HL ? ASMOP_HL : (pair == PAIR_DE ? ASMOP_DE : ASMOP_BC);

      if (!isPairDead (pair, ic))
        _push (pair);

      switch (pair)
        {
        case PAIR_HL:
          if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == H_IDX)
            cheapMove (ASMOP_L, 0, IC_RIGHT (ic)->aop, 0, true);
          else
            {
              cheapMove (ASMOP_L, 0, IC_LEFT (ic)->aop, 0, true);
              cheapMove (ASMOP_H, 0, IC_RIGHT (ic)->aop, 0, true);
            }
          break;
        case PAIR_DE:
          if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == D_IDX)
            cheapMove (ASMOP_E, 0, IC_RIGHT (ic)->aop, 0, true);
          else
            {
              cheapMove (ASMOP_E, 0, IC_LEFT (ic)->aop, 0, true);
              cheapMove (ASMOP_D, 0, IC_RIGHT (ic)->aop, 0, true);
            }
          break;
        default:
          wassert (pair == PAIR_BC);
          if (IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == B_IDX)
            cheapMove (ASMOP_C, 0, IC_RIGHT (ic)->aop, 0, true);
          else
            {
              cheapMove (ASMOP_C, 0, IC_LEFT (ic)->aop, 0, true);
              cheapMove (ASMOP_B, 0, IC_RIGHT (ic)->aop, 0, true);
            }
          break;
        }

      emit2 ("mlt %s", _pairs[pair].name);
      cost2 (2, 8, 17, 0, 0, 0, 6, 0);

      genMove_o (IC_RESULT (ic)->aop, 0, pairop, 0, 2 - byteResult, true, pair == PAIR_HL || isPairDead (PAIR_HL, ic), pair == PAIR_DE || isPairDead (PAIR_DE, ic), true, true);

      if (!isPairDead (pair, ic))
        _pop (pair);

      goto release;
    }
no_mlt:

  if (IS_RAB && !IS_R2K && isPairDead(PAIR_DE, ic) && isPairDead(PAIR_BC, ic) && // mul might be cheaper than a series of additions. mul is broken on the original Rabbit 2000.
    !byteResult && (IC_LEFT (ic)->aop->size > 1 || SPEC_USIGN (getSpec (operandType (IC_LEFT (ic))))))
    {
      int num_add = 0;
      bool active = false;
      unsigned int i = val;
      for (int count = 0; count < 16; count++)
        {
          if (count != 0 && active)
            num_add++;
          if (i & 0x8000u)
            {
              active = true;
              num_add += active;
            }
          i <<= 1;
        }

      if(num_add > (optimize.codeSize ? 4 : 6))
        {
          if (getPairId (IC_LEFT (ic)->aop) == PAIR_BC)
            fetchPair (PAIR_DE, IC_RIGHT (ic)->aop);
          else
            {
              fetchPairLong (PAIR_DE, IC_LEFT(ic)->aop, ic, 0);
              fetchPair (PAIR_BC, IC_RIGHT (ic)->aop);
            }
          emit2 ("mul");
          cost (1, 12);
          spillPair (PAIR_HL);
          genMove (IC_RESULT (ic)->aop, ASMOP_BC, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);
          goto release;
        }
    }

  pair = PAIR_DE;
  if (getPairId (IC_LEFT (ic)->aop) == PAIR_BC ||
    (byteResult || isRegDead (B_IDX, ic)) && IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == C_IDX)
    pair = PAIR_BC;
  if (isPairDead (PAIR_BC, ic) && !(IC_LEFT (ic)->aop->type == AOP_REG && IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx == E_IDX))
    pair = PAIR_BC;

  if (pair == PAIR_DE && (byteResult ? !isRegDead (E_IDX, ic) : !isPairDead (PAIR_DE, ic)))
    {
      _push (PAIR_DE);
      _G.stack.pushedDE = TRUE;
    }

  /* Use 16-bit additions even for 8-bit result when the operands are in the right places. */
  if (byteResult)
    {
      if (!aopInReg (IC_LEFT (ic)->aop, 0, A_IDX))
        a_cost += ld_cost (ASMOP_A, 0, IC_LEFT (ic)->aop, 0, false);
      if (!aopInReg (IC_RESULT (ic)->aop, 0, A_IDX))
        a_cost += ld_cost (IC_RESULT (ic)->aop, 0, ASMOP_A, 0, false);
      if (IC_LEFT (ic)->aop->type != AOP_REG || IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx != L_IDX)
        l_cost += ld_cost (ASMOP_L, 0, IC_LEFT (ic)->aop, 0, false);
      if (IC_RESULT (ic)->aop->type != AOP_REG || IC_RESULT (ic)->aop->aopu.aop_reg[0]->rIdx != L_IDX)
        l_cost += ld_cost (IC_RESULT (ic)->aop, 0, ASMOP_L, 0, false);
    }
  add_in_hl = (!byteResult || isPairDead (PAIR_HL, ic) && l_cost < a_cost);

  if (byteResult)
    {
      cheapMove (add_in_hl ? ASMOP_L : ASMOP_A, 0, IC_LEFT (ic)->aop, 0, true);
      if (IC_LEFT (ic)->aop->type != AOP_REG || IC_LEFT (ic)->aop->aopu.aop_reg[0]->rIdx != (pair == PAIR_BC ? C_IDX : E_IDX))
        cheapMove (pair == PAIR_BC ? ASMOP_C : ASMOP_E, 0, add_in_hl ? ASMOP_L : ASMOP_A, 0, true);
    }
  else if (IC_LEFT (ic)->aop->size == 1 && !SPEC_USIGN (getSpec (operandType (IC_LEFT (ic)))))
    {
      cheapMove (pair == PAIR_BC ? ASMOP_C : ASMOP_E, 0, IC_LEFT (ic)->aop, 0, true);
      emit2 ("ld a, %s", _pairs[pair].l);
      ld_cost (ASMOP_A, 0, ASMOP_L, 0, true);
      emit3 (A_RLCA, 0, 0);
      emit3 (A_SBC, ASMOP_A, ASMOP_A);
      emit2 ("ld %s, a", _pairs[pair].h);
      ld_cost (ASMOP_L, 0, ASMOP_A, 0, true);
      emit2 ("ld l, %s", _pairs[pair].l);
      ld_cost (ASMOP_L, 0, pair == PAIR_HL ? ASMOP_L : pair == PAIR_DE ? ASMOP_E : ASMOP_C, 0, true);
      emit2 ("ld h, %s", _pairs[pair].h);
      ld_cost (ASMOP_L, 0, pair == PAIR_HL ? ASMOP_H : pair == PAIR_DE ? ASMOP_D : ASMOP_B, 0, true);
    }
  else
    {
      fetchPair (pair, IC_LEFT (ic)->aop);
      if (getPairId (IC_LEFT (ic)->aop) != PAIR_HL)
        {
          emit2 ("ld l, %s", _pairs[pair].l);
          ld_cost (ASMOP_L, 0, pair == PAIR_HL ? ASMOP_L : pair == PAIR_DE ? ASMOP_E : ASMOP_C, 0, true);
          emit2 ("ld h, %s", _pairs[pair].h);
          ld_cost (ASMOP_H, 0, pair == PAIR_HL ? ASMOP_H : pair == PAIR_DE ? ASMOP_D : ASMOP_B, 0, true);
        }
    }

  if (!add_in_hl) 
    {
      unsigned long long add, sub;
      int topbit, nonzero;

      wassert(!csdOfVal (&topbit, &nonzero, &add, &sub, IC_RIGHT (ic)->aop->aopu.aop_lit, 0xff));
      
      // If the leading digits of the cse are 1 0 -1 we can use 0 1 1 instead to reduce the number of shifts.
      if (topbit >= 2 && (add & (1ull << topbit)) && (sub & (1ull << (topbit - 2))))
        {
          add = (add & ~(1u << topbit)) | (3u << (topbit - 2));
          sub &= ~(1u << (topbit - 1));
          topbit--;
        }

      for (int bit = topbit - 1; bit >= 0; bit--)
        {
          emit3 (A_ADD, ASMOP_A, ASMOP_A);
          if ((add | sub) & (1ull << bit))
            {
              emit2 (add & (1ull << bit) ? "add a, %s" : "sub a, %s", _pairs[pair].l);
              cost2 (1, 4, 4, 2, 4, 4, 1, 1);
            }
        }
    }
  else // Don't try to use CSD for hl, since subtraction there is more expensive than addition.
    {
      unsigned int i = val;
      for (int count = 0; count < 16; count++)
        {
          if (count != 0 && active)
            emit3w (A_ADD, ASMOP_HL, ASMOP_HL);
          if (i & 0x8000u)
            {
              if (active)
                {
                  emit2 ("add hl, %s", _pairs[pair].name);
                  cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
                }
              active = true;
            }
          i <<= 1;
        }
      spillPair (PAIR_HL);
    }

  if (_G.stack.pushedDE)
    {
      _pop (PAIR_DE);
      _G.stack.pushedDE = false;
    }

  genMove (IC_RESULT (ic)->aop, add_in_hl ? ASMOP_HL : ASMOP_A, true, add_in_hl || isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);

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
          cost2 (1, 4, 4, 2, 4, 4, 1, 1);
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
          cost2 (2, 8, 6, 4, 8, 4, 2, 2);
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
          cost2 (2, 8, 6, 4, 8, 4, 2, 2);
          inst = "Z";
        }
    }
  /* Z80 can do a conditional long jump */
  if (!regalloc_dry_run)
    emit2 ("jp %s, !tlabel", inst, labelKey2num (jlbl->key));
  cost2 (3, 10.0f, 7.5f,7.0f, 14.0f, 11.0f, 3.5f, 3.0f); // Assume either way equally likely.
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
  bool started = false;
  bool inv = false;

  /* if left & right are bit variables */
  if (left->aop->type == AOP_CRY && right->aop->type == AOP_CRY)
    {
      /* Can't happen on the Z80 */
      wassertl (0, "Tried to compare two bits");
    }
  else
    {
      /* Do a long subtract of right from left. */
      size = max (left->aop->size, right->aop->size);

      if (right->aop->type == AOP_SFR)  /* Avoid overwriting A */
        {
          bool save_a, save_b, save_bc;
          wassertl (size == 1, "Right side sfr in comparison with more than 8 bits.");

          save_b = !isRegDead (B_IDX, ic);
          save_bc = (save_b && !isRegDead (C_IDX, ic));
          save_a = (aopInReg (left->aop, 0, A_IDX) ||
                    aopInReg (left->aop, 0, B_IDX) && save_b ||
                    aopInReg (left->aop, 0, C_IDX) && !save_b && save_bc);

          if (save_bc)
            _push (PAIR_BC);
          if (save_a)
            {
              cheapMove (ASMOP_A, 0, right->aop, 0, true);
              _push (PAIR_AF);
            }
          else
            cheapMove (ASMOP_A, 0, right->aop, 0, true);
          cheapMove (save_b ? ASMOP_C : ASMOP_B, 0, ASMOP_A, 0, true);
          if (save_a)
            _pop (PAIR_AF);
          else
            cheapMove (ASMOP_A, 0, left->aop, 0, true);
          emit3_o (A_SUB, ASMOP_A, 0, save_b ? ASMOP_C : ASMOP_B, offset);
          if (save_bc)
            _pop (PAIR_BC);
          result_in_carry = TRUE;
          goto fix;
        }

      // Preserve A if necessary
      if (ifx && size == 1 && !sign && aopInReg (left->aop, 0, A_IDX) && !isRegDead (A_IDX, ic) &&
        (right->aop->type == AOP_LIT || right->aop->type == AOP_REG && right->aop->aopu.aop_reg[offset]->rIdx != IYL_IDX && right->aop->aopu.aop_reg[offset]->rIdx != IYH_IDX || right->aop->type == AOP_STK))
        {
          emit3 (A_CP, ASMOP_A, right->aop);
          result_in_carry = true;
          goto release;
        }
      else if (ifx && size == 1 && !sign && aopInReg (right->aop, 0, A_IDX) && left->aop->type == AOP_LIT && ullFromVal (left->aop->aopu.aop_lit) < 255)
        {
          emit2 ("cp a, !immedbyte", (unsigned int)(ullFromVal (left->aop->aopu.aop_lit) + 1));
          cost2 (2, 7, 6, 4, 8, 6, 2, 2);
          result_in_carry = true;
          inv = true;
          goto release;
        }
        
      if (right->aop->type == AOP_LIT && !ullFromVal (right->aop->aopu.aop_lit)) // special case: comparison to 0. Do it here early, so we don't run into sm83 workarounds below.
        {
          if (!sign)
            {
              /* No sign so it's always false */
              emit3 (A_CP, ASMOP_A, ASMOP_A);
              result_in_carry = TRUE;
            }
          else
            {
              if (!(result->aop->type == AOP_CRY && result->aop->size) && ifx &&
                (left->aop->type == AOP_REG || left->aop->type == AOP_STK && !IS_SM83))
                {
                  if (!regalloc_dry_run)
                    emit2 ("bit 7, %s", aopGet (left->aop, left->aop->size - 1, FALSE));
                  if (left->aop->type == AOP_REG)
                    cost2 (2, 8, 6, 4, 8, 4, 2, 2);
                  else
                    cost2 (4 - IS_TLCS90, 20, 15, 10, 6, 10, 5, 5);
                  genIfxJump (ifx, "nz");
                  return;
                }
             /* Just load in the top most bit */
             cheapMove (ASMOP_A, 0, left->aop, left->aop->size - 1, true);
             if (!(result->aop->type == AOP_CRY && result->aop->size) && ifx)
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

      // On the SM83 we can't afford to adjust HL as it may trash the carry.
      if (size > 1 && (IS_SM83 || IY_RESERVED) &&
        left->aop->type != AOP_REG && requiresHL (left->aop) && left->aop->type != AOP_STL &&
        right->aop->type != AOP_REG && requiresHL (right->aop) && right->aop->type != AOP_STL)
        {
          if (!isPairDead (PAIR_DE, ic))
            _push (PAIR_DE);

          pointPairToAop (PAIR_DE, left->aop, 0);
          pointPairToAop (PAIR_HL, right->aop, 0);

          while (size--)
            {
              emit2 ("ld a, !mems", "de");
              cost2 (1, 7, 6, 6, 8, 6, 2, 2);
              if (size != 0)
                emit3w (A_INC, ASMOP_DE, 0);
              emit2 ("%s a, !*hl", offset == 0 ? "sub" : "sbc");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              if (size != 0)
                emit3w (A_INC, ASMOP_HL, 0);
              offset++;
            }
          if (sign && IS_SM83)
            {
              wassert(isPairDead (PAIR_DE, ic));
              emit2 ("ld a, !mems", "de");
              cost2 (1, 7, 6, 6, 8, 6, 2, 2);
              emit3 (A_LD, ASMOP_D, ASMOP_A);
              emit2 ("ld e, !*hl");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
            }

          spillPair (PAIR_DE);
          if (!isPairDead (PAIR_DE, ic))
            _pop (PAIR_DE);

          spillPair (PAIR_HL);
          result_in_carry = TRUE;
          goto fix;
        }
      else if (size > 1 && IS_SM83 && (requiresHL (right->aop) && right->aop->type != AOP_REG && right->aop->type != AOP_STL && !requiresHL (left->aop)))
        {
          if (!regalloc_dry_run)
            aopGet (right->aop, LSB, FALSE);

          while (size--)
            {
              cheapMove (ASMOP_A, 0, left->aop, offset, true);
              emit2 ("%s a, !*hl", offset == 0 ? "sub" : "sbc");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);

              if (size != 0)
                emit3w (A_INC, ASMOP_HL, 0);
              offset++;
            }
          if (sign)
            {
              cheapMove (ASMOP_A, 0, left->aop, offset - 1, true);
              emit3 (A_LD, ASMOP_D, ASMOP_A);
              emit2 ("ld e, !*hl");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
            }
          spillPair (PAIR_HL);
          result_in_carry = TRUE;
          goto fix;
        }
      else if (size > 1 && IS_SM83 && (!requiresHL (right->aop) && requiresHL (left->aop) && left->aop->type != AOP_REG && left->aop->type != AOP_STL))
        {
          if (!regalloc_dry_run)
            aopGet (left->aop, LSB, FALSE);

          while (size--)
            {
              emit2 ("ld a, !*hl");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              emit3_o (offset == 0 ? A_SUB : A_SBC, ASMOP_A, 0, right->aop, offset);

              if (size != 0)
                {
                  emit3w (A_INC, ASMOP_HL, 0);
                  updatePair (PAIR_HL, 1);
                }
              offset++;
            }
          if (sign)
            {
              emit2 ("ld d, !*hl");
              cost2 (1, 7, 6, 5, 8, 6, 2, 2);
              cheapMove (ASMOP_A, 0, right->aop, offset - 1, true);
              emit3 (A_LD, ASMOP_E, ASMOP_A);
            }
          result_in_carry = TRUE;
          goto fix;
        }

      if (IS_SM83 && sign && right->aop->type != AOP_LIT && !aopInReg (left->aop, offset, A_IDX))
        {
          cheapMove (ASMOP_A, 0, right->aop, size - 1, true);
          cheapMove (ASMOP_E, 0, ASMOP_A, 0, true);
          cheapMove (ASMOP_A, 0, left->aop, size - 1, true);
          cheapMove (ASMOP_D, 0, ASMOP_A, 0, true);
        }

      if (right->aop->type == AOP_LIT)
        {
          lit = ullFromVal (right->aop->aopu.aop_lit);

          while (!((lit >> (offset * 8)) & 0xffull))
            {
              size--;
              offset++;
            }

          if (sign)             /* Map signed operands to unsigned ones. This pre-subtraction workaround to lack of signed comparison is cheaper than the post-subtraction one at fix. */
            {
              if (size == 2 && !(IS_SM83 || !ifx && requiresHL(result->aop) && result->aop->type != AOP_REG) && isPairDead (PAIR_HL, ic) && (isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic)) && (getPairId (left->aop) == PAIR_HL || IS_RAB && (left->aop->type == AOP_STK || left->aop->type == AOP_EXSTK)))
                {
                  PAIR_ID litpair = (isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC);
                  fetchPair (PAIR_HL, left->aop);
                  emit2 ("ld %s, !immedbyte", _pairs[litpair].name, (unsigned) ((lit ^ 0x8000u) & 0xffffu));
                  cost2 (3, 10, 9, 6, 12, 6, 3, 3);
                  emit3w (A_ADD, ASMOP_HL, ASMOP_HL);
                  emit2 ("ccf");
                  cost2 (1, 4, 3, 2, 4, 2, 2, 1);
                  if (IS_RAB)
                    {
                      emit2 ("rr hl");
                      cost (1, 2);
                    }
                  else
                    {
                      emit3 (A_RR, ASMOP_H, 0);
                      emit3 (A_RR, ASMOP_L, 0);
                    }
                  emit2 ("sbc hl, %s", _pairs[litpair].name);
                  cost2 (2, 15, 10, 4, 0, 8, 2, 2);
                  spillPair (PAIR_HL);
                  result_in_carry = true;
                  goto release;
                }

              cheapMove (ASMOP_A, 0, left->aop, offset, true);
              if (size == 1)
                {
                  emit2 ("xor a, !immedbyte", 0x80u);
                  cost2 (2, 7, 6, 4, 8, 4, 2, 2);
                }
              emit2 ("sub a, !immedbyte", (unsigned)(((lit >> (offset * 8)) & 0xff) ^ (size == 1 ? 0x80 : 0x00)));
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
              size--;
              offset++;

              while (size--)
                {
                  cheapMove (ASMOP_A, 0, left->aop, offset, true);
                  if (!size)
                    {
                      emit3 (A_RLA, 0, 0);
                      emit2 ("ccf");
                      cost2 (1, 4, 3, 2, 4, 2, 2, 1);
                      emit3 (A_RRA, 0, 0);
                    }
                  /* Subtract through, propagating the carry */
                  emit2 ("sbc a, !immedbyte", (unsigned)(((lit >> (offset++ * 8)) & 0xff) ^ (size ? 0x00 : 0x80)));
                  cost2 (2, 7, 6, 4, 8, 4, 2, 2);
                }
              result_in_carry = true;
              goto release;
            }
        }
      if (!IS_SM83 && (!sign || size > 2) && (getPartPairId (left->aop, offset) == PAIR_HL || size == 2 && left->aop->type == AOP_IY) && isPairDead (PAIR_HL, ic) &&
        (getPartPairId (right->aop, offset) == PAIR_DE || getPartPairId (right->aop, offset) == PAIR_BC))
        {
          if (left->aop->type == AOP_DIR || left->aop->type == AOP_IY)
            fetchPair (PAIR_HL, left->aop);
          emit3 (A_XOR, ASMOP_A, ASMOP_A); // Clear carry.
          emit2 ("sbc hl, %s", _pairs[getPartPairId (right->aop, offset)].name);
          cost2 (2, 15, 10, 4, 0, 8, 2, 2);
          spillPair (PAIR_HL);
          started = true;
          size -= 2;
          offset += 2;
        }
      else if (left->aop->type == AOP_LIT && !aopInReg (right->aop, offset, A_IDX) && isRegDead (A_IDX, ic))
        {
          bool pushed_hl = false;
          if (byteOfVal (left->aop->aopu.aop_lit, offset) == 0x00)
            emit3 (A_XOR, ASMOP_A, ASMOP_A);
          else
            cheapMove (ASMOP_A, 0, left->aop, offset, true);
          if (requiresHL (right->aop) && right->aop->type != AOP_REG && !isPairDead (PAIR_HL, ic))
            {
              _push (PAIR_HL);
              pushed_hl = true;
            }
          if (size > 1)
            {
              emit3_o (A_CP, ASMOP_A, 0, right->aop, offset);
              started = true;
              a_always_byte = byteOfVal (left->aop->aopu.aop_lit, offset);
            }
          else
            emit3_o (A_SUB, ASMOP_A, 0, right->aop, offset);
          if (pushed_hl)
            _pop (PAIR_HL);
          size--;
          offset++;
        }

      /* Subtract through, propagating the carry */
      while (size)
        {
          bool left_already_in_a = (left->aop->type == AOP_LIT && byteOfVal (left->aop->aopu.aop_lit, offset) == a_always_byte);

          if (started && !sign && aopIsLitVal (left->aop, offset, size, 0) && aopIsLitVal (right->aop, offset, size, 0)) // Skip leading zeroes.
            {
              offset += size;
              size = 0;
            }
          else if (!IS_SM83 && size >= 2 && (!sign || size > 2) && !left_already_in_a &&
            isPairDead (PAIR_HL, ic) &&
            (getPartPairId (left->aop, offset) == PAIR_HL || (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD || left->aop->type == AOP_HL || left->aop->type == AOP_IY || IS_RAB && left->aop->type == AOP_STK) && right->aop->regs[L_IDX] < offset && right->aop->regs[H_IDX] < offset) &&
            (getPartPairId (right->aop, offset) == PAIR_DE || getPartPairId (right->aop, offset) == PAIR_BC))
            {
              genMove_o (ASMOP_HL, 0, left->aop, offset, 2, isRegDead (A_IDX, ic), true, false, true, !offset);
              if (!started)
                emit3 (A_CP, ASMOP_A, ASMOP_A);
              emit3w_o (A_SBC, ASMOP_HL, 0, right->aop, offset);
              spillPair (PAIR_HL);
              started = true;
              size -= 2;
              offset += 2;
            }
          else if (!IS_SM83 && size >= 2 && (!sign || size > 2) && !left_already_in_a &&
            isPairDead (PAIR_HL, ic) && isPairDead (PAIR_DE, ic) && left->aop->regs[E_IDX] < offset + 1 && left->aop->regs[D_IDX] < offset + 1 &&
            (getPartPairId (left->aop, offset) == PAIR_HL || left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD || left->aop->type == AOP_HL || left->aop->type == AOP_IY || IS_RAB && left->aop->type == AOP_STK) &&
            (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD || right->aop->type == AOP_HL || right->aop->type == AOP_IY || IS_RAB && right->aop->type == AOP_STK))
            {
              genMove_o (ASMOP_DE, 0, right->aop, offset, 2, isRegDead (A_IDX, ic), getPartPairId (left->aop, offset) != PAIR_HL, true, true, !offset);
              genMove_o (ASMOP_HL, 0, left->aop, offset, 2, isRegDead (A_IDX, ic), true, false, true, !offset);
              if (!started)
                emit3 (A_CP, ASMOP_A, ASMOP_A);
              emit3w (A_SBC, ASMOP_HL, ASMOP_DE);
              spillPair (PAIR_HL);
              started = true;
              size -= 2;
              offset += 2;
            }
          else if (right->aop->type == AOP_STL &&
            (isRegDead (B_IDX, ic) && left->aop->regs[B_IDX] <= offset || isRegDead (B_IDX, ic) && left->aop->regs[C_IDX] <= offset ))
            {
              bool use_b = isRegDead (B_IDX, ic) && left->aop->regs[B_IDX] <= offset;
              if (!left_already_in_a)
                cheapMove (ASMOP_A, 0, left->aop, offset, true);
              cheapMove (use_b ? ASMOP_B : ASMOP_C, 0, right->aop, offset, false);
              a_always_byte = -1;
              emit3_o (started ? A_SBC : A_SUB, ASMOP_A, 0, use_b ? ASMOP_B : ASMOP_C, 0);
              started = true;
              size--;
              offset++;
            }
          else if (right->aop->type != AOP_STL && !aopInReg (right->aop, offset, A_IDX))
            {
              if (!left_already_in_a)
                cheapMove (ASMOP_A, 0, left->aop, offset, true);
              a_always_byte = -1;
              emit3_o (started ? A_SBC : A_SUB, ASMOP_A, 0, right->aop, offset);
              started = true;
              size--;
              offset++;
            }
          else
            {
              UNIMPLEMENTED;
              size--;
              offset++;
            }
        }

fix:
      /* There is no good signed compare in the Z80, so we need workarounds */
      if (sign)
        {
          if (!IS_SM83)           /* Directly check for overflow, can't be done on SM83 */
            {
              if (!regalloc_dry_run)
                {
                  symbol *tlbl = newiTempLabel (NULL);
                  emit2 (IS_RAB ? "jp LZ, !tlabel" : "jp PO, !tlabel", labelKey2num (tlbl->key));
                  cost2 (2 + IS_SM83, 12, 8, 5, 12, 12, 3, 3); // Assume no overflow.
                  emit2 ("xor a, !immedbyte", 0x80u);
                  cost (2, 0); // Assume no overflow.
                  emitLabelSpill (tlbl);
                }
              result_in_carry = FALSE;
            }
          else                  /* Do it the hard way */
            {
              /* Test if one operand is negative, while the other is not. If this is the
                 case we can easily decide which one is greater, and we set/reset the carry
                 flag. If not, then the unsigned compare gave the correct result and we
                 don't change the carry flag. */
              symbol *tlbl1 = regalloc_dry_run ? 0 : newiTempLabel (0);
              symbol *tlbl2 = regalloc_dry_run ? 0 : newiTempLabel (0);
              emit2 ("bit 7, e");
              cost2 (2, 8, 6, 4, 8, 4, 2, 2);
              if (!regalloc_dry_run)
                emit2 ("jp Z, !tlabel", labelKey2num (tlbl1->key));
              emit2 ("bit 7, d");
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (tlbl2->key));
              emit2 ("cp a, a");
              if (!regalloc_dry_run)
                emit2 ("jp !tlabel", labelKey2num (tlbl2->key));
              emitLabelSpill (tlbl1);
              emit2 ("bit 7, d");
              if (!regalloc_dry_run)
                emit2 ("jp Z, !tlabel", labelKey2num (tlbl2->key));
              emit2 ("scf");
              emitLabelSpill (tlbl2);
              regalloc_dry_run_cost += 18;
              result_in_carry = true;
            }
        }
      else
        result_in_carry = true;
    }

release:
  if (result->aop->type == AOP_CRY && result->aop->size)
    {
      wassert (!inv);
      if (!result_in_carry)
        {
          /* Shift the sign bit up into carry */
          emit3 (A_RLCA, 0, 0);
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
              wassert (!inv);
              if (!IS_SM83)
                genIfxJump (ifx, "m");
              else
                {
                  emit3 (A_RLCA, 0, 0);
                  genIfxJump (ifx, "c");
                }
            }
          else
            genIfxJump (ifx, inv ? "nc" : "c");
        }
      else
        {
          wassert (!inv);
          if (!result_in_carry)
            {
              /* Shift the sign bit up into carry */
              emit3 (A_RLCA, 0, 0);
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

  if (!IS_BITVAR (operandType (left)) && (!IS_BITINT (operandType (left)) || !(SPEC_BITINTWIDTH (operandType (left)) % 8)) &&
    !IS_BITVAR (operandType (right)) && (!IS_BITINT (operandType (right)) || !(SPEC_BITINTWIDTH (operandType (right)) % 8)) &&
    aopIsLitBit (left->aop, left->aop->size * 8 - 1, false) && aopIsLitBit (right->aop, right->aop->size * 8 - 1, false))
    sign = 0;

  if (max (left->aop->size, right->aop->size) > 1 && (couldDestroyCarry (left->aop) || couldDestroyCarry (right->aop)))
    {
      if ((requiresHL (IC_RESULT (ic)->aop) && IC_RESULT (ic)->aop->type != AOP_REG || requiresHL (left->aop) && left->aop->type != AOP_REG || requiresHL (right->aop) && right->aop->type != AOP_REG) &&
        (left->aop->regs[L_IDX] > 0 || left->aop->regs[H_IDX] > 0 || right->aop->regs[L_IDX] > 0 || right->aop->regs[H_IDX] > 0) || !isPairDead (PAIR_HL, ic))
        UNIMPLEMENTED;
      else
        setupToPreserveCarry (result->aop, left->aop, right->aop);
    }

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

  if (!IS_BITVAR (operandType (left)) && (!IS_BITINT (operandType (left)) || !(SPEC_BITINTWIDTH (operandType (left)) % 8)) &&
    !IS_BITVAR (operandType (right)) && (!IS_BITINT (operandType (right)) || !(SPEC_BITINTWIDTH (operandType (right)) % 8)) &&
    aopIsLitBit (left->aop, left->aop->size * 8 - 1, false) && aopIsLitBit (right->aop, right->aop->size * 8 - 1, false))
    sign = 0;

  if (max (left->aop->size, right->aop->size) > 1 && (couldDestroyCarry (left->aop) || couldDestroyCarry (right->aop)))
    {
      if ((requiresHL (IC_RESULT (ic)->aop) && IC_RESULT (ic)->aop->type != AOP_REG || requiresHL (left->aop) && left->aop->type != AOP_REG || requiresHL (right->aop) && right->aop->type != AOP_REG) &&
        (left->aop->regs[L_IDX] > 0 || left->aop->regs[H_IDX] > 0 || right->aop->regs[L_IDX] > 0 || right->aop->regs[H_IDX] > 0) || !isPairDead (PAIR_HL, ic))
        UNIMPLEMENTED;
      else
        setupToPreserveCarry (result->aop, left->aop, right->aop);
    }

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
  int size = max (left->aop->size, right->aop->size);
  int offset = 0;
  bool a_result = false;

  /* Swap the left and right if it makes the computation easier */
  if (left->aop->type == AOP_LIT || aopInReg (right->aop, 0, A_IDX))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  /* Non-destructive compare */
  if (aopInReg (left->aop, 0, A_IDX) && !isRegDead (A_IDX, ic) &&
    (right->aop->type == AOP_LIT ||
    right->aop->type == AOP_REG && (HAS_IYL_INST || right->aop->aopu.aop_reg[offset]->rIdx != IYL_IDX && right->aop->aopu.aop_reg[offset]->rIdx != IYH_IDX) ||
    right->aop->type == AOP_STK))
    {
      bool pushed_hl = false;
      if(requiresHL (right->aop) && right->aop->type != AOP_REG && !isPairDead(PAIR_HL, ic))
        {
          _push (PAIR_HL);
          pushed_hl = true;
        }
        
      if (right->aop->type == AOP_LIT && !byteOfVal (right->aop->aopu.aop_lit, 0))
        emit3 (A_OR, ASMOP_A, ASMOP_A);
      else
        emit3 (A_CP, ASMOP_A, right->aop);
        
      if (pushed_hl)
        _pop (PAIR_HL);

      if (!regalloc_dry_run)
        emit2 ("jp NZ, !tlabel", labelKey2num (lbl->key));
      cost2 (3, 10.0f, 7.5f, 7.0f, 14.0f,  11.0f, 3.5f, 3.0f); // Assume both branches equally likely, cp not optimzed into jr.
    }
  /* if the right side is a literal then anything goes */
  else if (right->aop->type == AOP_LIT)
    {
      while (size--)
        {
          bool pushed_hl = false;
          bool next_zero = size && !byteOfVal (right->aop->aopu.aop_lit, offset + 1);

          if(requiresHL (left->aop) && left->aop->type != AOP_REG && !isPairDead(PAIR_HL, ic))
            {
              _push (PAIR_HL);
              pushed_hl = true;
            }

          // Test for 0 can be done more efficiently using or
          if (!byteOfVal (right->aop->aopu.aop_lit, offset))
            {
              if (!a_result)
                {
                  cheapMove (ASMOP_A, 0, left->aop, offset, true);
                  emit3 (A_OR, ASMOP_A, ASMOP_A);
                }
              else
                emit3_o (A_OR, ASMOP_A, 0, left->aop, offset);

              a_result = TRUE;
            }
          else if ((aopInReg (left->aop, 0, A_IDX) && isRegDead (A_IDX, ic) ||
            left->aop->type == AOP_REG && left->aop->aopu.aop_reg[offset]->rIdx != IYL_IDX && left->aop->aopu.aop_reg[offset]->rIdx != IYH_IDX && !bitVectBitValue (ic->rSurv, left->aop->aopu.aop_reg[offset]->rIdx)) &&
            byteOfVal (right->aop->aopu.aop_lit, offset) == 0x01 && !next_zero)
            {
              emit3_o (A_DEC, left->aop, offset, 0, 0);
              a_result = aopInReg (left->aop, 0, A_IDX);
            }
          else if (isRegDead (A_IDX, ic) && left->aop->regs[A_IDX] < offset && size && byteOfVal (right->aop->aopu.aop_lit, offset) == 0xff &&
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
              next_zero = size && !byteOfVal (right->aop->aopu.aop_lit, offset + 1);
              a_result = true;
            }
          else if ((aopInReg (left->aop, 0, A_IDX) && isRegDead (A_IDX, ic) ||
            left->aop->type == AOP_REG && left->aop->aopu.aop_reg[offset]->rIdx != IYL_IDX && left->aop->aopu.aop_reg[offset]->rIdx != IYH_IDX && !bitVectBitValue (ic->rSurv, left->aop->aopu.aop_reg[offset]->rIdx)) &&
            byteOfVal (right->aop->aopu.aop_lit, offset) == 0xff && !next_zero)
            {
              emit3_o (A_INC, left->aop, offset, 0, 0);
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

          if (pushed_hl)
            _pop (PAIR_HL);

          // Only emit jump now if there is no following test for 0 (which would just or to a current result in a)
          if (!(next_zero && a_result))
            {
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (lbl->key));
              cost2 (3, 10.0f, 7.5f, 7.0f, 14.0f,  11.0f, 3.5f, 3.0f); // Assume both branches equally likely, cp not optimzed into jr.
            }
          offset++;
        }
    }
  /* if the right side is in a register or
     pointed to by HL, IX or IY */
  else if (right->aop->type == AOP_REG ||
           right->aop->type == AOP_HL ||
           right->aop->type == AOP_IY ||
           right->aop->type == AOP_STK ||
           right->aop->type == AOP_EXSTK ||
           right->aop->type == AOP_IMMD ||
           AOP_IS_PAIRPTR (right, PAIR_HL) || AOP_IS_PAIRPTR (right, PAIR_IX) || AOP_IS_PAIRPTR (right, PAIR_IY))
    {
      while (size--)
        {
          bool hl_dead = isRegDead (HL_IDX, ic) && left->aop->regs[L_IDX] < offset && left->aop->regs[H_IDX] < offset && right->aop->regs[L_IDX] < offset && right->aop->regs[H_IDX] < offset;
          bool iy_dead = isRegDead (IY_IDX, ic) && left->aop->regs[IYL_IDX] < offset && left->aop->regs[IYH_IDX] < offset && right->aop->regs[IYL_IDX] < offset && right->aop->regs[IYH_IDX] < offset;

          if (aopInReg (right->aop, offset, A_IDX)  || aopInReg (right->aop, offset, HL_IDX) || aopInReg (left->aop, offset, BC_IDX) || aopInReg (left->aop, offset, DE_IDX))
            {
              operand *t = right;
              right = left;
              left = t;
            }

          if (!IS_SM83 && isPairDead (PAIR_HL, ic) &&
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
              cost2 (2, 15, 10, 4, 0, 8, 2, 2);
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (lbl->key));
              cost2 (3, 10.0f, 7.5f, 7.0f, 14.0f,  11.0f, 3.5f, 3.0f); // Assume both branches equally likely, jp not optimzed into jr.
              spillPair (PAIR_HL);
              offset += 2;
              size--;
              continue;
            }

          if (!hl_dead)
            genMove_o (ASMOP_A, 0, left->aop, offset, 1, true, false, false, iy_dead, true);
          else
            cheapMove (ASMOP_A, 0, left->aop, offset, true);
          if (right->aop->type == AOP_LIT && byteOfVal (right->aop->aopu.aop_lit, offset) == 0 || right->aop->type == AOP_STL && offset >= 2)
            {
              emit3 (A_OR, ASMOP_A, ASMOP_A);
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (lbl->key));
              cost2 (3, 10.0f, 7.5f, 7.0f, 14.0f,  11.0f, 3.5f, 3.0f); // Assume both branches equally likely, jp not optimzed into jr.
            }
          else if (right->aop->type == AOP_STL && offset < 2)
            {
              if (!hl_dead)
                _push (PAIR_HL);
              genMove_o (ASMOP_HL, 0, right->aop, 0, 2, false, true, false, false, true);
              emit3 (A_SUB, ASMOP_A, offset ? ASMOP_H : ASMOP_L);
              if (!hl_dead)
                _pop (PAIR_HL);
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (lbl->key));
              cost2 (3, 10.0f, 7.5f, 7.0f, 14.0f,  11.0f, 3.5f, 3.0f); // Assume both branches equally likely, jp not optimzed into jr.
            }
          else
            {
              emit3_o (A_SUB, ASMOP_A, 0, right->aop, offset);
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (lbl->key));
              cost2 (3, 10.0f, 7.5f, 7.0f, 14.0f,  11.0f, 3.5f, 3.0f); // Assume both branches equally likely, jp not optimzed into jr.
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
          if (((left->aop->type != AOP_PAIRPTR) || (left->aop->aopu.aop_pairId != pair)) &&
              ((right->aop->type != AOP_PAIRPTR) || (right->aop->aopu.aop_pairId != pair)))
            {
              break;
            }
        }
      _push (pair);
      while (size--)
        {
          cheapMove (pair == PAIR_BC ? ASMOP_BC : (pair == PAIR_DE ? ASMOP_DE : ASMOP_HL), 0, left->aop, offset, true);
          cheapMove (ASMOP_A, 0, right->aop, offset, true);
          emit2 ("sub a, %s", _pairs[pair].l);
          cost2 (1, 4, 4, 2, 4, 4, 1, 1);
          if (!regalloc_dry_run)
            emit2 ("jp NZ, !tlabel", labelKey2num (lbl->key));
          cost2 (3, 10.0f, 7.5f, 7.0f, 14.0f,  11.0f, 3.5f, 3.0f); // Assume both branches equally likely, cp not optimzed into jr.
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

  hl_touched = (IC_LEFT (ic)->aop->type == AOP_HL || IC_RIGHT (ic)->aop->type == AOP_HL || IS_SM83
                && IC_LEFT (ic)->aop->type == AOP_STK || IS_SM83 && IC_RIGHT (ic)->aop->type == AOP_STK);

  /* Swap operands if it makes the operation easier. ie if:
     1.  Left is a literal.
   */
  if (IC_LEFT (ic)->aop->type == AOP_LIT || IC_RIGHT (ic)->aop->type != AOP_LIT && IC_RIGHT (ic)->aop->type != AOP_REG
      && IC_LEFT (ic)->aop->type == AOP_REG)
    {
      operand *t = IC_RIGHT (ic);
      IC_RIGHT (ic) = IC_LEFT (ic);
      IC_LEFT (ic) = t;
    }

  if (ifx && !result->aop->size)
    {
      /* if they are both bit variables */
      if (left->aop->type == AOP_CRY && ((right->aop->type == AOP_CRY) || (right->aop->type == AOP_LIT)))
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
                  cost2 (1, 10, 9, 7, 12, 10, 3, 3);
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
                  cost2 (1, 10, 9, 7, 12, 10, 3, 3);
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
      goto release;
    }

  /* if they are both bit variables */
  if (left->aop->type == AOP_CRY && ((right->aop->type == AOP_CRY) || (right->aop->type == AOP_LIT)))
    {
      wassertl (0, "Tried to compare a bit to either a literal or another bit");
    }
  else
    {
      gencjne (left, right, regalloc_dry_run ? 0 : newiTempLabel (NULL), ic);
      if (result->aop->type == AOP_CRY && result->aop->size)
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
      if (result->aop->type != AOP_CRY)
        genMove (result->aop, ASMOP_A, true, isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);
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
  if (left->aop->type == AOP_CRY && right->aop->type == AOP_CRY)
    {
      wassertl (0, "Tried to and two bits");
    }
  else
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      _toBoolean (left, TRUE);
      if (!regalloc_dry_run)
        emit2 ("jp Z, !tlabel", labelKey2num (tlbl->key));
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
  if (left->aop->type == AOP_CRY && right->aop->type == AOP_CRY)
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
  // Using emitLabelSpill instead of emitLabel (esp. on sm83)
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
  bool a_free = isRegDead (A_IDX, ic) && left->aop->regs[A_IDX] <= 0 && right->aop->regs[A_IDX] <= 0;

  /* if left is a literal & right is not then exchange them */
  if ((left->aop->type == AOP_LIT && right->aop->type != AOP_LIT) || (AOP_NEEDSACC (right) && !AOP_NEEDSACC (left)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (sameRegs (result->aop, right->aop) && !AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  if (right->aop->type == AOP_LIT)
    lit = ullFromVal (right->aop->aopu.aop_lit);

  size = result->aop->size;

  if (left->aop->type == AOP_CRY)
    {
      wassertl (0, "Tried to perform an AND with a bit as an operand");
      goto release;
    }

  /* Make sure A is on the left to not overwrite it. */
  if (aopInReg (right->aop, 0, A_IDX) ||
    !aopInReg (left->aop, 0, A_IDX) && isPair (right->aop) && (getPairId (right->aop) == PAIR_HL || getPairId (right->aop) == PAIR_IY))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  // if(val & 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val & 0xZZ     - size = 1, ifx = FALSE -
  if ((right->aop->type == AOP_LIT) && (result->aop->type == AOP_CRY) && (left->aop->type != AOP_CRY))
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      int sizel;

      sizel = left->aop->size;
      if (size)
        {
          /* PENDING: Test case for this. */
          emit2 ("scf");
          cost2 (1, 4, 3, 2, 4, 2, 1, 1);
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
          if ((isLiteralBit (bytelit) == 0 || isLiteralBit (bytelit) == 7) && aopInReg (left->aop, offset, A_IDX) && isRegDead (A_IDX, ic))
            {
              emit3 (isLiteralBit (bytelit) == 0 ? A_RRCA : A_RLCA, 0 , 0);
              jumpcond = "C";
              sizel--;
              offset++;
            }
          /* Testing for the inverse of the border bits of some 32-bit registers destructively is cheap. */
          /* More combinations would be possible, but this one is the one that is common in the floating-point library. */
          else if (left->aop->type == AOP_REG && sizel >= 4 && ((lit >> (offset * 8)) & 0xffffffffull) == 0x7fffffffull &&
            !IS_SM83 && getPartPairId (left->aop, offset) == PAIR_HL && isPairDead (PAIR_HL, ic) &&
            IS_RAB && getPartPairId (left->aop, offset + 2) == PAIR_DE && isPairDead (PAIR_HL, ic))
            {
              emit3 (A_CP, ASMOP_A, ASMOP_A); // Clear carry.
              emit3w (A_ADC, ASMOP_HL, ASMOP_HL); // Cannot use "add hl, hl instead, since it does not affect zero flag.
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
              emit2 ("rl de");
              regalloc_dry_run_cost += 6;
              sizel -= 4;
              offset += 4;
            }
          /* Testing for the inverse of the border bits of some 16-bit registers destructively is cheap. */
          /* More combinations would be possible, but these are the common ones. */
          else if (left->aop->type == AOP_REG && sizel >= 2 && ((lit >> (offset * 8)) & 0xffffull) == 0x7fffull &&
            (!IS_SM83 && getPartPairId (left->aop, offset) == PAIR_HL && isPairDead (PAIR_HL, ic) ||
            IS_RAB && getPartPairId (left->aop, offset) == PAIR_DE  && isPairDead (PAIR_DE, ic)))
            {
              PAIR_ID pair;
              switch (left->aop->aopu.aop_reg[offset]->rIdx)
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
                {
                  emit2 ("adc %s, %s", _pairs[pair].name, _pairs[pair].name); // Cannot use add hl, hl instead, since it does not affect zero flag.
                  cost2 (2, 15, 10, 4, 0, 8, 2, 2);
                }
              else
                {
                  emit2 ("rl %s", _pairs[pair].name);
                  cost (1, 2);
                }
              sizel -= 2;
              offset += 2;
            }
          /* Testing for the border bits of some 16-bit registers destructively is cheap. */
          else if (left->aop->type == AOP_REG && sizel == 1 &&
            (isLiteralBit (bytelit) == 7 && (
              left->aop->aopu.aop_reg[offset]->rIdx == H_IDX && isPairDead (PAIR_HL, ic) ||
              IS_RAB && left->aop->aopu.aop_reg[offset]->rIdx == D_IDX && isPairDead (PAIR_DE, ic) ||
              left->aop->aopu.aop_reg[offset]->rIdx == IYH_IDX && isPairDead (PAIR_IY, ic)
            ) ||
            isLiteralBit (bytelit) == 0 && IS_RAB && (
              left->aop->aopu.aop_reg[offset]->rIdx == L_IDX && isPairDead (PAIR_HL, ic) ||
              left->aop->aopu.aop_reg[offset]->rIdx == E_IDX && isPairDead (PAIR_DE, ic) ||
              left->aop->aopu.aop_reg[offset]->rIdx == IYL_IDX && isPairDead (PAIR_IY, ic)
              )))
            {
              PAIR_ID pair;
              switch (left->aop->aopu.aop_reg[offset]->rIdx)
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
                {
                  emit2 ("add %s, %s", _pairs[pair].name, _pairs[pair].name);
                  if (pair == PAIR_HL)
                    cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
                  else
                    cost2 (2, 15, 10, 4, 0, 8, 2, 2);
                }
              else if (isLiteralBit (bytelit) == 7)
                {
                  emit2 ("rl %s", _pairs[pair].name);
                  cost (1, 2);
                }
              else
                {
                  emit2 ("rr %s", _pairs[pair].name);
                  cost (1, 2);
                }
              jumpcond = "C";
              sizel--;
              offset++;
            }
          /* Non-destructive and when exactly one bit per byte is set. */
          else if (isLiteralBit (bytelit) >= 0 &&
            (left->aop->type == AOP_STK || aopInReg (left->aop, offset, A_IDX) || left->aop->type == AOP_HL || left->aop->type == AOP_IY ||
              left->aop->type == AOP_REG && !aopInReg (left->aop, offset, IYL_IDX) && !aopInReg (left->aop, offset, IYH_IDX)))
            {
              if (requiresHL (left->aop) && left->aop->type != AOP_REG)
                _push (PAIR_HL);
              if (!regalloc_dry_run)
                emit2 ("bit %d, %s", isLiteralBit (bytelit), aopGet (left->aop, offset, FALSE));
              if (left->aop->type == AOP_REG)
                cost2 (2, 8, 6, 4, 8, 4, 2, 2);
              else
                cost2 (4, 20 , 15, 10, 6, 10, 5, 5);
              if (requiresHL (left->aop) && left->aop->type != AOP_REG)
                _pop (PAIR_HL);
              sizel--;
              offset++;
            }
          /* Z180 has non-destructive and. */
          else if ((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) && aopInReg (left->aop, 0, A_IDX) && !isRegDead (A_IDX, ic) && bytelit != 0x0ff)
            {
              if (!regalloc_dry_run)
                emit2 ("tst a, %s", aopGet (right->aop, 0, FALSE));
              cost2 (3, 11, 9, 0, 0, 0, 3, 0);
              sizel--;
              offset++;
            }
          else if (!isRegDead (A_IDX, ic) && bytelit == 0x0ff && !aopInReg (left->aop, offset, A_IDX) && left->aop->type == AOP_REG && !aopInReg (left->aop, offset, IYL_IDX) && !aopInReg (left->aop, offset, IYH_IDX))
            {
              emit3_o (A_RLC, left->aop, offset, 0, 0);
              emit3_o (A_RRC, left->aop, offset, 0, 0);
              sizel--;
              offset++;
            }
          /* Generic case, loading into accumulator and testing there. */
          else
            {
              if (!isRegDead (A_IDX, ic) || left->aop->regs[A_IDX] > offset || right->aop->regs[A_IDX] > offset)
                UNIMPLEMENTED;

              cheapMove (ASMOP_A, 0, left->aop, offset, true);
              if (isLiteralBit (bytelit) == 0 || isLiteralBit (bytelit) == 7)
                {
                  emit3 (isLiteralBit (bytelit) == 0 ? A_RRCA : A_RLCA, 0 , 0);
                  jumpcond = "C";
                }
              else if (bytelit != 0xffu)
                emit3_o (A_AND, ASMOP_A, 0, right->aop, offset);
              else
                emit3 (A_OR, ASMOP_A, ASMOP_A);     /* For the flags */
              sizel--;
              offset++;
            }
          if (size || ifx)  /* emit jmp only, if it is actually used */
            {
              if (!regalloc_dry_run)
                emit2 ("jp %s, !tlabel", jumpcond, labelKey2num (tlbl->key));
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

  if ((IS_RAB || IS_TLCS90) && isPair (result->aop) &&
      (getPairId (result->aop) == PAIR_HL && isPair (right->aop) && getPairId (right->aop) == PAIR_DE ||
       getPairId (result->aop) == PAIR_HL && isPair (left->aop) && getPairId (left->aop) == PAIR_DE))
    {
      if (isPair (left->aop) && getPairId (left->aop) == PAIR_DE)
        fetchPair (PAIR_HL, right->aop);
      else // right operand in DE
        fetchPair (getPairId (result->aop), left->aop);
      emit2 ("and hl, de");
      cost2 (1 + IS_TLCS90, 0, 0, 2, 0, 8, 0, 0);
      goto release;
    }

  wassertl (result->aop->type != AOP_CRY, "Result of and is in a bit");

  for (int i = 0; i < size;)
    {
      bool hl_free = isPairDead (PAIR_HL, ic) &&
        (left->aop->regs[L_IDX] < i && left->aop->regs[H_IDX] < i && right->aop->regs[L_IDX] < i && right->aop->regs[H_IDX] < i) &&
        (result->aop->regs[L_IDX] < 0 || result->aop->regs[L_IDX] >= i) && (result->aop->regs[H_IDX] < 0 || result->aop->regs[H_IDX] >= i);

      if (isRegDead (A_IDX, ic) && left->aop->regs[A_IDX] <= i && right->aop->regs[A_IDX] <= i && (result->aop->regs[A_IDX] < 0 || result->aop->regs[A_IDX] >= i))
        a_free = true;

      if (pushed_a && (aopInReg (left->aop, i, A_IDX) || aopInReg (right->aop, i, A_IDX)))
        {
          _pop (PAIR_AF);
          if (!isRegDead (A_IDX, ic))
            _push (PAIR_AF);
          else
            pushed_a = false;
        }

      if (aopIsLitVal (result->aop, i, 1, 0x00) || aopIsLitVal (right->aop, i, 1, 0x00) || aopIsLitVal (right->aop, i, 1, 0xff))
        {
          unsigned int bytelit = (aopIsLitVal (result->aop, i, 1, 0x00) || aopIsLitVal (right->aop, i, 1, 0x00)) ? 0x00 : 0xff;

          int end;
          for(end = i; end < size && (!bytelit && aopIsLitVal (result->aop, end, 1, 0x00) || aopIsLitVal (right->aop, end, 1, bytelit)); end++);
            genMove_o (result->aop, i, bytelit == 0x00 ? ASMOP_ZERO : left->aop, i, end - i, a_free, hl_free, !isPairInUse (PAIR_DE, ic), true, true);
          if (result->aop->regs[A_IDX] >= i && result->aop->regs[A_IDX] < end)
            a_free = false;
          i = end;
          continue;
        }

      if (right->aop->type == AOP_LIT)
        {
          bytelit = byteOfVal (right->aop->aopu.aop_lit, i);

          if (isLiteralBit (~bytelit & 0xffu) >= 0 && aopSame (result->aop, i, left->aop, i, 1) &&
            (result->aop->type == AOP_STK || result->aop->type == AOP_DIR || result->aop->type == AOP_REG && !aopInReg (result->aop, i, IYL_IDX) && !aopInReg (result->aop, i, IYH_IDX)))
            {
              cheapMove (result->aop, i, left->aop, i, a_free);
              if (!regalloc_dry_run)
                emit2 ("res %d, %s", isLiteralBit (~bytelit & 0xffu), aopGet (result->aop, i, false));
              if (result->aop->type == AOP_STK && !IS_SM83)
                cost2 (4, 23, 19, 13, 0, 14, 5, 7); // res b, d(ix)
              else if (result->aop->type == AOP_DIR)
                cost2 (2, 15, 13, 10, 16, 10, 3 , 5); // res b, (hl)
              else
                cost2 (2, 8, 6, 4, 8, 4, 2, 2); // res b, r
              if (aopInReg (result->aop, i, A_IDX))
                a_free = false;
              i++;
              continue;
            }
          else if (IS_RAB &&
            (aopInReg (result->aop, i, HL_IDX) && (aopInReg (left->aop, i, HL_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, DE_IDX)) ||
            aopInReg (result->aop, i, H_IDX) && aopInReg (result->aop, i + 1, L_IDX) && (aopInReg (left->aop, i, H_IDX) && aopInReg (left->aop, i + 1, L_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, D_IDX) && aopInReg (left->aop, i + 1, E_IDX))))
            {
              unsigned int mask = aopInReg (result->aop, i, L_IDX) ? (bytelit + (byteOfVal (right->aop->aopu.aop_lit, i + 1) << 8)) : (byteOfVal (right->aop->aopu.aop_lit, i + 1) + (bytelit << 8));
              bool mask_in_de = (aopInReg (left->aop, i, L_IDX) || aopInReg (left->aop, i, H_IDX));
              emit2 (mask_in_de ? "ld de, !immedword" : "ld hl, !immedword", mask);
              emit2 ("and hl, de");
              cost2 (1 + IS_TLCS90, 0, 0, 2, 0, 8, 0, 0);
              i += 2;
              continue;
            }
          else if (IS_TLCS90 &&
            (aopInReg (left->aop, i, HL_IDX) && aopInReg (result->aop, i, HL_IDX) || aopInReg (left->aop, i, H_IDX) && aopInReg (left->aop, i + 1, L_IDX) && aopInReg (result->aop, i, H_IDX) && aopInReg (result->aop, i + 1, L_IDX)))
            {
              unsigned int mask = aopInReg (result->aop, i, L_IDX) ? (bytelit + (byteOfVal (right->aop->aopu.aop_lit, i + 1) << 8)) : (byteOfVal (right->aop->aopu.aop_lit, i + 1) + (bytelit << 8));
              emit2 ("and hl, !immedword", mask);
              cost (3, 6);
              i += 2;
              continue;
            }
        }

      if (IS_RAB || IS_TLCS90)
        {
          const bool this_byte_l = aopInReg (result->aop, i, L_IDX) &&
            (aopInReg (left->aop, i, L_IDX) && aopInReg (right->aop, i, E_IDX) || aopInReg (left->aop, i, E_IDX) && aopInReg (right->aop, i, L_IDX));
          const bool this_byte_h = aopInReg (result->aop, i, H_IDX) &&
            (aopInReg (left->aop, i, H_IDX) && aopInReg (right->aop, i, D_IDX) || aopInReg (left->aop, i, D_IDX) && aopInReg (right->aop, i, H_IDX));
          const bool next_byte_l = aopInReg (result->aop, i + 1, L_IDX) &&
            (aopInReg (left->aop, i + 1, L_IDX) && aopInReg (right->aop, i + 1, E_IDX) || aopInReg (left->aop, i + 1, E_IDX) && aopInReg (right->aop, i + 1, L_IDX));
          const bool next_byte_h = aopInReg (result->aop, i + 1, H_IDX) &&
            (aopInReg (left->aop, i + 1, H_IDX) && aopInReg (right->aop, i + 1, D_IDX) || aopInReg (left->aop, i + 1, D_IDX) && aopInReg (right->aop, i + 1, H_IDX));

          const bool this_byte = this_byte_l || this_byte_h;
          const bool next_byte = next_byte_l || next_byte_h;

          const int next_byte_idx = this_byte_l ? H_IDX : L_IDX;
          const bool next_byte_unused = isRegDead (next_byte_idx, ic) &&
            left->aop->regs[next_byte_idx] <= i && right->aop->regs[next_byte_idx] <= i &&
            (result->aop->regs[next_byte_idx] < 0 || result->aop->regs[next_byte_idx] >= i);

          if (this_byte && (next_byte || next_byte_unused))
            {
              emit2 ("and hl, de");
              cost2 (1 + IS_TLCS90, 0, 0, 2, 0, 8, 0, 0);
              i += (1 + next_byte);
              continue;
            }
        }

      if (!a_free)
        {
          if (pushed_a)
            UNIMPLEMENTED;
          else
            _push (PAIR_AF);
          pushed_a = true;
          a_free = true;
        }

      // Use plain and in a.
      if (aopInReg (right->aop, i, A_IDX))
        {
          if (requiresHL (left->aop) && left->aop->type != AOP_REG && !hl_free)
            _push (PAIR_HL);
          if (!HAS_IYL_INST && (aopInReg (left->aop, i, IYL_IDX) || aopInReg (left->aop, i, IYH_IDX)))
            UNIMPLEMENTED;
          else
            emit3_o (A_AND, ASMOP_A, 0, left->aop, i);
          if (requiresHL (left->aop) && left->aop->type != AOP_REG && !hl_free)
            _pop (PAIR_HL);
        }
      else
        {
          if (requiresHL (left->aop) && left->aop->type != AOP_REG && !hl_free)
            _push (PAIR_HL);
          cheapMove (ASMOP_A, 0, left->aop, i, true);
          if (requiresHL (left->aop) && left->aop->type != AOP_REG && !hl_free)
            _pop (PAIR_HL);

          if (requiresHL (right->aop) && right->aop->type != AOP_REG && !hl_free)
            _push (PAIR_HL);

          if ((right->aop->type == AOP_SFR || (aopInReg (right->aop, i, IYL_IDX) || aopInReg (right->aop, i, IYH_IDX)) && !HAS_IYL_INST) && hl_free)
            {
              cheapMove (ASMOP_L, 0, left->aop, i, false);
              emit3 (A_AND, ASMOP_A, ASMOP_L);
            }
          else if (right->aop->type == AOP_SFR || !HAS_IYL_INST && (aopInReg (right->aop, i, IYL_IDX) || aopInReg (right->aop, i, IYH_IDX)))
            UNIMPLEMENTED;
          else
            emit3_o (A_AND, ASMOP_A, 0, right->aop, i);
          if (requiresHL (right->aop) && right->aop->type != AOP_REG && !hl_free)
            _pop (PAIR_HL);
        }

      hl_free = isPairDead (PAIR_HL, ic) &&
        (left->aop->regs[L_IDX] <= i && left->aop->regs[H_IDX] <= i && right->aop->regs[L_IDX] <= i && right->aop->regs[H_IDX] <= i) &&
        (result->aop->regs[L_IDX] < 0 || result->aop->regs[L_IDX] >= i) && (result->aop->regs[H_IDX] < 0 || result->aop->regs[H_IDX] >= i);

      genMove_o (result->aop, i, ASMOP_A, 0, 1, true, hl_free, !isPairInUse (PAIR_DE, ic), true, true);

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

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RIGHT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, FALSE);
  
  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);

  if (result->aop->type == AOP_REG && left->aop->type != AOP_REG && right->aop->type != AOP_REG)
    {
      if (!requiresHL (right->aop) || (result->aop->regs[L_IDX] < 0 && result->aop->regs[H_IDX] < 0))
        { /* only if not (right requires HL and result use HL) */
          genMove (result->aop, left->aop,
                   isRegDead (A_IDX, ic),
                   isPairDead (PAIR_HL, ic),
                   isPairDead (PAIR_DE, ic) && right->aop->regs[D_IDX] < 0 && right->aop->regs[E_IDX] < 0,
                   true);
          left = result;
        }
    }


  bool pushed_a = false;
  bool a_free = isRegDead (A_IDX, ic) && left->aop->regs[A_IDX] <= 0 && right->aop->regs[A_IDX] <= 0;

  /* if left is a literal & right is not then exchange them */
  if ((left->aop->type == AOP_LIT && right->aop->type != AOP_LIT) || (AOP_NEEDSACC (right) && !AOP_NEEDSACC (left)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (sameRegs (result->aop, right->aop) && !AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  if (right->aop->type == AOP_LIT)
    lit = ullFromVal (right->aop->aopu.aop_lit);

  size = result->aop->size;

  if (left->aop->type == AOP_CRY)
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
  if ((right->aop->type == AOP_LIT) && (result->aop->type == AOP_CRY) && (left->aop->type != AOP_CRY))
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      int sizel;

      sizel = left->aop->size;

      if (size)
        {
          wassertl (0, "Result is assigned to a bit");
        }
      /* PENDING: Modeled after the AND code which is inefficient. */
      while (sizel--)
        {
          if (isRegDead (A_IDX, ic) && left->aop->regs[A_IDX] <= offset && right->aop->regs[A_IDX] <= offset && (result->aop->regs[A_IDX] < 0 || result->aop->regs[A_IDX] >= offset))
            a_free = true;

          if (!a_free) // Hard to handle pop with ifx
            UNIMPLEMENTED;

          int bytelit = (lit >> (offset * 8)) & 0x0FFull;

          cheapMove (ASMOP_A, 0, left->aop, offset, true);

          if (bytelit != 0)
            emit3_o (A_OR, ASMOP_A, 0, right->aop, offset);
          else if (ifx)
            {
              /* For the flags */
              emit3 (A_OR, ASMOP_A, ASMOP_A);
            }

          if (ifx)              /* emit jmp only, if it is actually used */
            {
              if (!regalloc_dry_run)
                emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
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

  wassertl (result->aop->type != AOP_CRY, "Result of or is in a bit");

  for (int i = 0; i < size;)
    {
      bool hl_free = isPairDead (PAIR_HL, ic) &&
        (left->aop->regs[L_IDX] < i && left->aop->regs[H_IDX] < i && right->aop->regs[L_IDX] < i && right->aop->regs[H_IDX] < i) &&
        (result->aop->regs[L_IDX] < 0 || result->aop->regs[L_IDX] >= i) && (result->aop->regs[H_IDX] < 0 || result->aop->regs[H_IDX] >= i);

      if (isRegDead (A_IDX, ic) && left->aop->regs[A_IDX] <= i && right->aop->regs[A_IDX] <= i && (result->aop->regs[A_IDX] < 0 || result->aop->regs[A_IDX] >= i))
        a_free = true;

      if (pushed_a && (aopInReg (left->aop, i, A_IDX) || aopInReg (right->aop, i, A_IDX)))
        {
          _pop (PAIR_AF);
          if (!isRegDead (A_IDX, ic))
            _push (PAIR_AF);
          else
            pushed_a = false;
        }

      if (left->aop->type == AOP_REG && right->aop->type == AOP_REG && // Try to use ld (nn), rr, etc.
        aopIsLitVal (left->aop, i, 1, 0x00) && aopIsLitVal (right->aop, i + 1, 1, 0x00) &&
        (aopInReg (right->aop, i, C_IDX) && aopInReg (left->aop, i + 1, B_IDX) || aopInReg (right->aop, i, E_IDX) && aopInReg (left->aop, i + 1, D_IDX) || aopInReg (right->aop, i, L_IDX) && aopInReg (left->aop, i + 1, H_IDX) || aopInReg (right->aop, i, IYL_IDX) && aopInReg (left->aop, i + 1, IYH_IDX)))
        {
          asmop *source = aopInReg (right->aop, i, C_IDX) ? ASMOP_BC : aopInReg (right->aop, i, E_IDX) ? ASMOP_DE : aopInReg (right->aop, i, L_IDX) ? ASMOP_HL : ASMOP_IY;
          genMove_o (result->aop, i, source, 0, 2, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic), true);
          i += 2;
          continue;
        }
      else if (left->aop->type == AOP_REG && right->aop->type == AOP_REG && // Try to use ld (nn), rr, etc.
        aopIsLitVal (left->aop, i + 1, 1, 0x00) && aopIsLitVal (right->aop, i, 1, 0x00) &&
        (aopInReg (right->aop, i + 1, B_IDX) && aopInReg (left->aop, i, C_IDX) || aopInReg (right->aop, i + 1, D_IDX) && aopInReg (left->aop, i, E_IDX) || aopInReg (right->aop, i + 1, H_IDX) && aopInReg (left->aop, i, L_IDX) || aopInReg (right->aop, i + 1, IYH_IDX) && aopInReg (left->aop, i, IYL_IDX)) &&
        (result->aop->type == AOP_DIR ||result->aop->type == AOP_HL || result->aop->type == AOP_IY))
        {
          asmop *source = aopInReg (right->aop, i + 1, B_IDX) ? ASMOP_BC : aopInReg (right->aop, i + 1, D_IDX) ? ASMOP_DE : aopInReg (right->aop, i + 1, H_IDX) ? ASMOP_HL : ASMOP_IY;
          genMove_o (result->aop, i, source, 0, 2, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic), true);
          i += 2;
          continue;
        }
      else if (aopIsLitVal (left->aop, i, 1, 0x00) && !pushed_a)
        {
          int end;
          for(end = i; end < size && aopIsLitVal (left->aop, end, 1, 0x00); end++);
            genMove_o (result->aop, i, right->aop, i, end - i, a_free, hl_free, !isPairInUse (PAIR_DE, ic), true, true);
          if (result->aop->regs[A_IDX] >= i && result->aop->regs[A_IDX] < end)
            a_free = false;
          i = end;
          continue;
        }
      else if (aopIsLitVal (right->aop, i, 1, 0x00) && !pushed_a)
        {
          int end;
          for(end = i; end < size && aopIsLitVal (right->aop, end, 1, 0x00); end++);
            genMove_o (result->aop, i, left->aop, i, end - i, a_free, hl_free, !isPairInUse (PAIR_DE, ic), true, true);
          if (result->aop->regs[A_IDX] >= i && result->aop->regs[A_IDX] < end)
            a_free = false;
          i = end;
          continue;
        }
      else if (aopIsLitVal (left->aop, i, 1, 0xff) || aopIsLitVal (right->aop, i, 1, 0xff))
        {
          int end;
          for(end = i; end < size && (aopIsLitVal (left->aop, end, 1, 0xff) || aopIsLitVal (right->aop, end, 1, 0xff)); end++);
            genMove_o (result->aop, i, ASMOP_MONE, i, end - i, a_free, hl_free, !isPairInUse (PAIR_DE, ic), true, true);
          if (result->aop->regs[A_IDX] >= i && result->aop->regs[A_IDX] < end)
            a_free = false;
          i = end;
          continue;
        }

      if (right->aop->type == AOP_LIT)
        {
          int bytelit = byteOfVal (right->aop->aopu.aop_lit, i);

          if (isLiteralBit (bytelit) >= 0 && aopSame (result->aop, i, left->aop, i, 1) &&
            (result->aop->type == AOP_STK || result->aop->type == AOP_DIR || result->aop->type == AOP_REG && !aopInReg (result->aop, i, IYL_IDX) && !aopInReg (result->aop, i, IYH_IDX)))
            {
              cheapMove (result->aop, i, left->aop, i, a_free);
              if (!regalloc_dry_run)
                emit2 ("set %d, %s", isLiteralBit (bytelit), aopGet (result->aop, i, false));
              if (result->aop->type == AOP_STK && !IS_SM83)
                cost2 (4, 23, 19, 13, 0, 14, 5, 7); // set b, d(ix)
              else if (result->aop->type == AOP_DIR)
                cost2 (2, 15, 13, 10, 16, 10, 3 , 5); // set b, (hl)
              else
                cost2 (2, 8, 6, 4, 8, 4, 2, 2); // set b, r
              if (aopInReg (result->aop, i, A_IDX))
                a_free = false;
              i++;
              continue;
            }
          else if (IS_RAB &&
            (aopInReg (result->aop, i, HL_IDX) && (aopInReg (left->aop, i, HL_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, DE_IDX)) ||
            aopInReg (result->aop, i, H_IDX) && aopInReg (result->aop, i + 1, L_IDX) && (aopInReg (left->aop, i, H_IDX) && aopInReg (left->aop, i + 1, L_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, D_IDX) && aopInReg (left->aop, i + 1, E_IDX))))
            {
              unsigned int mask = aopInReg (result->aop, i, L_IDX) ? (bytelit + (byteOfVal (right->aop->aopu.aop_lit, i + 1) << 8)) : (byteOfVal (right->aop->aopu.aop_lit, i + 1) + (bytelit << 8));
              bool mask_in_de = (aopInReg (left->aop, i, L_IDX) || aopInReg (left->aop, i, H_IDX));
              emit2 (mask_in_de ? "ld de, !immedword" : "ld hl, !immedword", mask);
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
              emit2 ("or hl, de");
              cost2 (1, 0, 0 , 2, 0, 8, 0, 0);
              i += 2;
              continue;
            }
          else if (IS_RAB &&
            (aopInReg (result->aop, i, IY_IDX) && (aopInReg (left->aop, i, IY_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, DE_IDX)) ||
            aopInReg (result->aop, i, IYH_IDX) && aopInReg (result->aop, i + 1, IYL_IDX) && (aopInReg (left->aop, i, IYH_IDX) && aopInReg (left->aop, i + 1, IYL_IDX) && !isPairInUse (PAIR_DE, ic) || aopInReg (left->aop, i, D_IDX) && aopInReg (left->aop, i + 1, E_IDX))))
            {
              unsigned int mask = aopInReg (result->aop, i, IYL_IDX) ? (bytelit + (byteOfVal (right->aop->aopu.aop_lit, i + 1) << 8)) : (byteOfVal (right->aop->aopu.aop_lit, i + 1) + (bytelit << 8));
              bool mask_in_de = (aopInReg (left->aop, i, IYL_IDX) || aopInReg (left->aop, i, IYH_IDX));
              emit2 (mask_in_de ? "ld de, !immedword" : "ld iy, !immedword", mask);
              cost (3 + !mask_in_de, 6 + 2 * !mask_in_de);
              emit2 ("or iy, de");
              cost (2, 4);
              i += 2;
              continue;
            }
          else if (IS_TLCS90 &&
            (aopInReg (left->aop, i, HL_IDX) && aopInReg (result->aop, i, HL_IDX) || aopInReg (left->aop, i, H_IDX) && aopInReg (left->aop, i + 1, L_IDX) && aopInReg (result->aop, i, H_IDX) && aopInReg (result->aop, i + 1, L_IDX)))
            {
              unsigned int mask = aopInReg (result->aop, i, L_IDX) ? (bytelit + (byteOfVal (right->aop->aopu.aop_lit, i + 1) << 8)) : (byteOfVal (right->aop->aopu.aop_lit, i + 1) + (bytelit << 8));
              emit2 ("or hl, !immedword", mask);
              cost (3, 6);
              i += 2;
              continue;
            }
        }

      if (IS_RAB)
        {
          const bool this_byte_l = aopInReg (result->aop, i, L_IDX) &&
            (aopInReg (left->aop, i, L_IDX) && aopInReg (right->aop, i, E_IDX) || aopInReg (left->aop, i, E_IDX) && aopInReg (right->aop, i, L_IDX));
          const bool this_byte_h = aopInReg (result->aop, i, H_IDX) &&
            (aopInReg (left->aop, i, H_IDX) && aopInReg (right->aop, i, D_IDX) || aopInReg (left->aop, i, D_IDX) && aopInReg (right->aop, i, H_IDX));
          const bool next_byte_l = aopInReg (result->aop, i + 1, L_IDX) &&
            (aopInReg (left->aop, i + 1, L_IDX) && aopInReg (right->aop, i + 1, E_IDX) || aopInReg (left->aop, i + 1, E_IDX) && aopInReg (right->aop, i + 1, L_IDX));
          const bool next_byte_h = aopInReg (result->aop, i + 1, H_IDX) &&
            (aopInReg (left->aop, i + 1, H_IDX) && aopInReg (right->aop, i + 1, D_IDX) || aopInReg (left->aop, i + 1, D_IDX) && aopInReg (right->aop, i + 1, H_IDX));

          const bool this_byte_hl = this_byte_l || this_byte_h;
          const bool next_byte_hl = next_byte_l || next_byte_h;

          const int next_byte_hl_idx = this_byte_l ? H_IDX : L_IDX;
          const bool next_byte_hl_unused = isRegDead (next_byte_hl_idx, ic) &&
            left->aop->regs[next_byte_hl_idx] <= i && right->aop->regs[next_byte_hl_idx] <= i &&
            (result->aop->regs[next_byte_hl_idx] < 0 || result->aop->regs[next_byte_hl_idx] >= i);

          if (this_byte_hl && (next_byte_hl || next_byte_hl_unused))
            {
              emit2 ("or hl, de");
              cost2 (1, 0, 0 , 2, 0, 8, 0, 0);
              i += (1 + next_byte_hl);
              continue;
            }
            
          if (aopInReg (result->aop, i, IY_IDX) &&
           (aopInReg (left->aop, i, IY_IDX) && aopInReg (right->aop, i, DE_IDX) || aopInReg (left->aop, i, DE_IDX) && aopInReg (right->aop, i, IY_IDX)))
            {
              emit2 ("or iy, de");
              cost (2, 4);
              i += 2;
              continue;
            }
          else if (aopInReg (right->aop, i, DE_IDX) &&
            (left->aop->type == AOP_STK || left->aop->type == AOP_DIR || left->aop->type == AOP_IY) &&
            (aopInReg (result->aop, i, HL_IDX) || isPairDead(PAIR_HL, ic) && right->aop->regs[L_IDX] < i + 2 && right->aop->regs[H_IDX] < i + 2 && (result->aop->type == AOP_DIR || result->aop->type == AOP_IY || result->aop->type == AOP_STK)))
            {
              fetchPairLong (PAIR_HL, left->aop, ic, i);
              emit2 ("or hl, de");
              cost2 (1, 0, 0 , 2, 0, 8, 0, 0);
              genMove_o (result->aop, i, ASMOP_HL, 0, 2, a_free, true, false, true, true);
              i += 2;
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

      if (aopInReg (right->aop, i, A_IDX) || right->aop->type == AOP_SFR ||
        !HAS_IYL_INST && (aopInReg (right->aop, i, IYL_IDX) || aopInReg (right->aop, i, IYH_IDX)))
        {
          cheapMove (ASMOP_A, 0, right->aop, i, true);

          if (requiresHL (left->aop) && left->aop->type != AOP_REG && !hl_free)
            _push (PAIR_HL);
          if ((left->aop->type == AOP_SFR || (aopInReg (right->aop, i, IYL_IDX) || aopInReg (right->aop, i, IYH_IDX)) && !HAS_IYL_INST) && hl_free)
            {
              cheapMove (ASMOP_L, 0, left->aop, i, false);
              emit3 (A_OR, ASMOP_A, ASMOP_L);
            }
          else if (left->aop->type == AOP_SFR || aopInReg (right->aop, i, A_IDX) ||
            !HAS_IYL_INST && (aopInReg (right->aop, i, IYL_IDX) || aopInReg (right->aop, i, IYH_IDX)))
            UNIMPLEMENTED;
          else
            emit3_o (A_OR, ASMOP_A, 0, left->aop, i);
          if (requiresHL (left->aop) && left->aop->type != AOP_REG && !hl_free)
            _pop (PAIR_HL);
        }
      else
        {
          if (requiresHL (left->aop) && left->aop->type != AOP_REG && !hl_free)
            _push (PAIR_HL);
          cheapMove (ASMOP_A, 0, left->aop, i, true);
          if (requiresHL (left->aop) && left->aop->type != AOP_REG && !hl_free)
            _pop (PAIR_HL);

          if (requiresHL (right->aop) && right->aop->type != AOP_REG && !hl_free)
            _push (PAIR_HL);
          emit3_o (A_OR, ASMOP_A, 0, right->aop, i);
          if (requiresHL (right->aop) && right->aop->type != AOP_REG && !hl_free)
            _pop (PAIR_HL);
        }

      hl_free = isPairDead (PAIR_HL, ic) &&
        (left->aop->regs[L_IDX] <= i && left->aop->regs[H_IDX] <= i && right->aop->regs[L_IDX] <= i && right->aop->regs[H_IDX] <= i) &&
        (result->aop->regs[L_IDX] < 0 || result->aop->regs[L_IDX] >= i) && (result->aop->regs[H_IDX] < 0 || result->aop->regs[H_IDX] >= i);

      genMove_o (result->aop, i, ASMOP_A, 0, 1, true, hl_free, !isPairInUse (PAIR_DE, ic), true, true);
        
      if (aopInReg (result->aop, i, A_IDX))
        a_free = false;
      i++;
    }

  if (pushed_a)
    _pop (PAIR_AF);

release:
  freeAsmop (IC_LEFT (ic), NULL);
  freeAsmop (IC_RIGHT (ic), NULL);
  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genEor - code for xclusive or                                   */
/*-----------------------------------------------------------------*/
static void
genEor (const iCode *ic, iCode *ifx, asmop *result_aop, asmop *left_aop, asmop *right_aop)
{
  int size;
  bool pushed_a = false;

  bool a_free = isRegDead (A_IDX, ic) && left_aop->regs[A_IDX] <= 0 && right_aop->regs[A_IDX] <= 0;

  /* if left is a literal & right is not then exchange them */
  if ((left_aop->type == AOP_LIT && right_aop->type != AOP_LIT) || ((right_aop->type == AOP_SFR || right_aop->type == AOP_CRY) && !(left_aop->type == AOP_SFR || left_aop->type == AOP_CRY)))
    {
      asmop *taop = right_aop;
      right_aop = left_aop;
      left_aop = taop;
    }

  /* if result = right then exchange them */
  if (sameRegs (result_aop, right_aop) && !(left_aop->type == AOP_SFR || left_aop->type == AOP_CRY))
    {
      asmop *taop = right_aop;
      right_aop = left_aop;
      left_aop = taop;
    }

  size = result_aop->size;

  if (left_aop->type == AOP_CRY)
    {
      wassertl (0, "Tried to XOR a bit");
      return;
    }

  /* Make sure A is on the left to not overwrite it. */
  if (aopInReg (right_aop, 0, A_IDX))
    {
      wassert (!(left_aop->type == AOP_SFR || left_aop->type == AOP_CRY));
      asmop *taop = right_aop;
      right_aop = left_aop;
      left_aop = taop;
    }

  // if(val & 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val & 0xZZ     - size = 1, ifx = FALSE -
  if ((right_aop->type == AOP_LIT) && (result_aop->type == AOP_CRY) && (left_aop->type != AOP_CRY))
    {
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      int offset = 0;
      int sizel = left_aop->size;

      if (size)
        {
          /* PENDING: Test case for this. */
          wassertl (0, "Tried to XOR left against a literal with the result going into a bit");
        }
      while (sizel--)
        {
          if (isRegDead (A_IDX, ic) && left_aop->regs[A_IDX] <= offset && right_aop->regs[A_IDX] <= offset && (result_aop->regs[A_IDX] < 0 || result_aop->regs[A_IDX] >= offset))
            a_free = true;

          if (!a_free)
            {
              wassert (!pushed_a);
              _push (PAIR_AF);
              a_free = true;
              pushed_a = true;
              if (ifx) // The pop at the end is hard to deal with in case of ifx.
                UNIMPLEMENTED;
            }
          else if (pushed_a && (aopInReg (left_aop, offset, A_IDX) || aopInReg (right_aop, offset, A_IDX)))
            {
              _pop (PAIR_AF);
              if (!isRegDead (A_IDX, ic))
                _push (PAIR_AF);
              else
                pushed_a = false;
            }

          if (aopInReg (right_aop, offset, A_IDX))
            emit3_o (A_XOR, ASMOP_A, 0, left_aop, offset);
          else
            {
              cheapMove (ASMOP_A, 0, left_aop, offset, true);
              emit3_o (A_XOR, ASMOP_A, 0, right_aop, offset);
            }
          if (ifx)              /* emit jmp only, if it is actually used * */
            if (!regalloc_dry_run)
              emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
          regalloc_dry_run_cost += 3;
          offset++;
        }
      if (pushed_a)
        {
          _pop (PAIR_AF);
          pushed_a = false;
        }
      if (ifx)
        {
          jmpTrueOrFalse (ifx, tlbl);
        }
      else if (size)
        {
          wassertl (0, "Result of XOR was destined for a bit");
        }
      return;
    }

    // left & result in different registers
    if (result_aop->type == AOP_CRY)
      {
        wassertl (0, "Result of XOR is in a bit");
        return;
      }

    for (int i = 0; i < size;)
      {
        bool hl_free = isPairDead (PAIR_HL, ic) &&
          (left_aop->regs[L_IDX] < i && left_aop->regs[H_IDX] < i && right_aop->regs[L_IDX] < i && right_aop->regs[H_IDX] < i) &&
          (result_aop->regs[L_IDX] < 0 || result_aop->regs[L_IDX] >= i) && (result_aop->regs[H_IDX] < 0 || result_aop->regs[H_IDX] >= i);

        if (isRegDead (A_IDX, ic) && left_aop->regs[A_IDX] <= i && right_aop->regs[A_IDX] <= i && (result_aop->regs[A_IDX] < 0 || result_aop->regs[A_IDX] >= i))
          a_free = true;
            
        // normal case
        // result = left ^ right
        if (aopIsLitVal (right_aop, i, 1, 0x00))
          {
            int end;
            for (end = i; end < size && aopIsLitVal (right_aop, end, 1, 0x00); end++);
            if (pushed_a && left_aop->type == AOP_REG && left_aop->regs[A_IDX] >= i && left_aop->regs[A_IDX] < end)
              {
                if (result_aop->regs[A_IDX] >= 0 && result_aop->regs[A_IDX] < i)
                  UNIMPLEMENTED;
                _pop (PAIR_AF);
                if (!isRegDead (A_IDX, ic))
                  _push (PAIR_AF);
                else
                  pushed_a = false;
              }
            genMove_o (result_aop, i, left_aop, i, end - i, a_free, hl_free, !isPairInUse (PAIR_DE, ic), true, true);
            if (result_aop->type == AOP_REG &&
              (left_aop->regs[result_aop->aopu.aop_reg[i]->rIdx] >= end || right_aop->regs[result_aop->aopu.aop_reg[i]->rIdx] >= end))
              UNIMPLEMENTED;
            if (result_aop->regs[A_IDX] >= i && result_aop->regs[A_IDX] < end)
              a_free = false;
            i = end;
            continue;
          }
        else if (IS_TLCS90 && right_aop->type == AOP_LIT &&
          (aopInReg (left_aop, i, HL_IDX) && aopInReg (result_aop, i, HL_IDX) || aopInReg (left_aop, i, H_IDX) && aopInReg (left_aop, i + 1, L_IDX) && aopInReg (result_aop, i, H_IDX) && aopInReg (result_aop, i + 1, L_IDX)))
          {
            unsigned int bytelit = byteOfVal (right_aop->aopu.aop_lit, i);
            unsigned int mask = aopInReg (result_aop, i, L_IDX) ? (bytelit + (byteOfVal (right_aop->aopu.aop_lit, i + 1) << 8)) : (byteOfVal (right_aop->aopu.aop_lit, i + 1) + (bytelit << 8));
            emit2 ("xor hl, !immedword", mask);
            cost (3, 6);
            i += 2;
            continue;
          }
        else if (IS_RAB && aopIsLitVal (right_aop, i, 1, 0x01) &&
          (aopInReg (left_aop, i, L_IDX) && aopInReg (result_aop, i, L_IDX) || aopInReg (left_aop, i, E_IDX) && aopInReg (result_aop, i, E_IDX)))
          {
            bool de = aopInReg (result_aop, i, E_IDX);
            emit2 (de ? "rr de" : "rr hl");
            cost (1, 2);
            emit2 ("ccf");
            cost (1, 2);
            emit2 (de ? "rl de" : "adc hl, hl");
            cost (2 - de, 4 - 2 * de);
            i++;
            continue;
          }
        else if (IS_RAB && aopIsLitVal (right_aop, i, 1, 0x80) &&
          (aopInReg (left_aop, i, H_IDX) && aopInReg (result_aop, i, H_IDX) || aopInReg (left_aop, i, D_IDX) && aopInReg (result_aop, i, D_IDX)))
          {
            bool de = aopInReg (result_aop, i, D_IDX);
            emit2 (de ? "rl de" : "add hl, hl");
            cost (1, 2);
            emit2 ("ccf");
            cost (1, 2);
            emit2 (de ? "rr de" : "rr hl");
            cost (1, 2);
            i++;
            continue;
          }

        if (pushed_a && (aopInReg (left_aop, i, A_IDX) || aopInReg (right_aop, i, A_IDX)))
          {
            if (result_aop->regs[A_IDX] >= 0 && result_aop->regs[A_IDX] < i)
              UNIMPLEMENTED;
            _pop (PAIR_AF);
            if (!isRegDead (A_IDX, ic))
              _push (PAIR_AF);
            else
              pushed_a = false;
          }

        // faster than result <- left, anl result,right
        // and better if result is SFR
        if (!a_free)
          {
            if (pushed_a)
              UNIMPLEMENTED;
            else
              _push (PAIR_AF);
            a_free = true;
            pushed_a = true;
          }

        if (aopInReg (right_aop, i, A_IDX) && left_aop->type != AOP_STL)
          {
            if (requiresHL (left_aop) && left_aop->type != AOP_REG && !hl_free)
              _push (PAIR_HL);
            if (!HAS_IYL_INST && (aopInReg (left_aop, i, IYL_IDX) || aopInReg (left_aop, i, IYH_IDX)))
              UNIMPLEMENTED;
            else
              emit3_o (A_XOR, ASMOP_A, 0, left_aop, i);
            if (requiresHL (left_aop) && left_aop->type != AOP_REG && !hl_free)
              _pop (PAIR_HL);
          }
        else
          {
            if (requiresHL (left_aop) && left_aop->type != AOP_REG && !hl_free)
              _push (PAIR_HL);
            cheapMove (ASMOP_A, 0, left_aop, i, true);
            if (requiresHL (left_aop) && left_aop->type != AOP_REG && !hl_free)
               _pop (PAIR_HL);
            if (right_aop->type == AOP_LIT && byteOfVal (right_aop->aopu.aop_lit, i) == 0xff)
              emit3 (A_CPL, 0, 0);
            else if (right_aop->type == AOP_SFR || right_aop->type == AOP_STL || aopInReg (right_aop, i, IYL_IDX) || aopInReg (right_aop, i, IYH_IDX))
              {
                if (!hl_free)
                  _push (PAIR_HL);
                cheapMove (ASMOP_L, 0, right_aop, i, false);
                emit3_o (A_XOR, ASMOP_A, 0, ASMOP_L, 0);
                if (!hl_free)
                  _pop (PAIR_HL);
              }
            else
              {
                if (requiresHL (right_aop) && right_aop->type != AOP_REG && !hl_free)
                  _push (PAIR_HL);
                emit3_o (A_XOR, ASMOP_A, 0, right_aop, i);
                if (requiresHL (right_aop) && right_aop->type != AOP_REG && !hl_free)
                  _pop (PAIR_HL);
              }
          }
          
        hl_free = isPairDead (PAIR_HL, ic) &&
          (left_aop->regs[L_IDX] <= i && left_aop->regs[H_IDX] <= i && right_aop->regs[L_IDX] <= i && right_aop->regs[H_IDX] <= i) &&
          (result_aop->regs[L_IDX] < 0 || result_aop->regs[L_IDX] >= i) && (result_aop->regs[H_IDX] < 0 || result_aop->regs[H_IDX] >= i);

        genMove_o (result_aop, i, ASMOP_A, 0, 1, true, hl_free, !isPairInUse (PAIR_DE, ic), true, true);

        if(result_aop->type == AOP_REG &&
          (left_aop->regs[result_aop->aopu.aop_reg[i]->rIdx] > i || right_aop->regs[result_aop->aopu.aop_reg[i]->rIdx] > i))
          UNIMPLEMENTED;
        if (aopInReg (result_aop, i, A_IDX))
          a_free = false;

        i++;
     }

  if (pushed_a)
    _pop (PAIR_AF);
}

/*-----------------------------------------------------------------*/
/* genXor - code for exclusive or                                   */
/*-----------------------------------------------------------------*/
static void
genXor (const iCode *ic, iCode *ifx)
{
  aopOp (IC_LEFT (ic), ic, false, false);
  aopOp (IC_RIGHT (ic), ic, false, false);
  aopOp (IC_RESULT (ic), ic, true, false);
  
  genEor (ic, ifx, IC_RESULT (ic)->aop, IC_LEFT (ic)->aop, IC_RIGHT (ic)->aop);
  
  freeAsmop (IC_LEFT (ic), NULL);
  freeAsmop (IC_RIGHT (ic), NULL);
  freeAsmop (IC_RESULT (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genCpl - generate code for complement                           */
/*-----------------------------------------------------------------*/
static void
genCpl (const iCode *ic)
{
  /* assign asmOps to operand & result */
  aopOp (IC_LEFT (ic), ic, false, false);
  aopOp (IC_RESULT (ic), ic, true, false);

  genEor (ic, 0, IC_RESULT (ic)->aop, IC_LEFT (ic)->aop, ASMOP_MONE);

  /* release the aops */
  freeAsmop (IC_LEFT (ic), 0);
  freeAsmop (IC_RESULT (ic), 0);
}

/*-----------------------------------------------------------------*/
/* genRRC - rotate right with carry                                */
/*-----------------------------------------------------------------*/
static void
genRRC (const iCode *ic)
{
  bool pushed_a = false;

  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  aopOp (left, ic, false, false);
  aopOp (result, ic, true, false);

  int size = result->aop->size;

  wassert (size >= 2); // 8-bit rotations are handled in Rot1
  wassert (!(bitsForType (operandType (left)) % 8));
  wassert (IS_OP_LITERAL (right));

  int s = operandLitValueUll (right) % bitsForType (operandType (left));

  wassert (s == bitsForType (operandType (left)) - 1);

  int offset = size - 1;

  if (IS_Z80N && size == 2 && aopInReg (result->aop, 0, DE_IDX) && isRegDead (B_IDX, ic))
    {
      genMove (ASMOP_DE, left->aop, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), true, true);
      emit2 ("ld b, !immedbyte", 15u);
      cost (2, 7);
      emit2 ("brlc de, b");
      cost (2, 8);
    }
  else if (left->aop->type == AOP_REG || result->aop->type == AOP_STK ||
           result->aop->type == AOP_HL || result->aop->type == AOP_IY ||
           result->aop->type == AOP_EXSTK || result->aop->type == AOP_REG)
    {
      if (!isRegDead (A_IDX, ic))
        {
          _push (PAIR_AF);
          pushed_a = true;
        }
      if (left->aop->type != AOP_REG && !operandsEqu (result, left))
        {
          /* always prefer register operations */
          genMove_o (result->aop, 0, left->aop, 0, size, true, isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);
          left = result;
        }
      cheapMove (ASMOP_A, 0, left->aop, offset, true);
      emit3_o (A_RRA, 0, 0, 0, 0);
      while (--offset >= 0)
        emit3_o (A_RR, left->aop, offset, 0, 0);
      if (IS_SM83 && requiresHL (left->aop))
        { /* ldhl sp,N changes CARRY */
          emit3_o (A_RRA, 0, 0, 0, 0);
          if (!regalloc_dry_run)
            aopGet (left->aop, size - 1, false);
          emit3_o (A_RLA, 0, 0, 0, 0);
        }
      emit3_o (A_RR, left->aop, size - 1, 0, 0);
      if (!operandsEqu (result, left))
        genMove_o (result->aop, 0, left->aop, 0, size, true, isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);
    }
  else
    {
      if (!isRegDead (A_IDX, ic))
      {
        _push (PAIR_AF);
        pushed_a = true;
      }
      while (offset >= 0)
        {
          _moveA (aopGet (left->aop, offset, false));
          emit3_o (A_RRA, 0, 0, 0, 0);
          if (offset != size - 1)
            aopPut (result->aop, "a", offset);
          --offset;
        }
      _moveA (aopGet (left->aop, size - 1, false));
      emit3_o (A_RRA, 0, 0, 0, 0);
      aopPut (result->aop, "a", size - 1);
    }
  if (pushed_a)
    _pop (PAIR_AF);

  freeAsmop (IC_LEFT (ic), 0);
  freeAsmop (IC_RESULT (ic), 0);
}

/*-----------------------------------------------------------------*/
/* genRLC - generate code for rotate left                          */
/*-----------------------------------------------------------------*/
static void
genRLC (const iCode *ic)
{
  bool pushed_a = false;

  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  aopOp (left, ic, false, false);
  aopOp (result, ic, true, false);

  int size = result->aop->size;

  wassert (size >= 2); // 8-bit rotations are handled in Rot1
  wassert (!(bitsForType (operandType (left)) % 8));
  wassert (IS_OP_LITERAL (right));

  int s = operandLitValueUll (right) % bitsForType (operandType (left));

  wassert (s == 1);

  if (IS_Z80N && size == 2 &&
    (aopInReg (result->aop, 0, DE_IDX) || aopInReg (left->aop, 0, DE_IDX) && isRegDead (DE_IDX, ic)) && isRegDead (B_IDX, ic))
    {
      genMove (ASMOP_DE, left->aop, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), true, isRegDead (IY_IDX, ic));
      emit2 ("ld b, !immedbyte", s);
      emit2 ("brlc de, b");
      cost (4, 15);
      genMove (result->aop, ASMOP_DE, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), true, isRegDead (IY_IDX, ic));
    }
  else if (left->aop->type == AOP_REG || result->aop->type == AOP_STK ||
           result->aop->type == AOP_HL || result->aop->type == AOP_IY ||
           result->aop->type == AOP_EXSTK || result->aop->type == AOP_REG)
    {
      asmop *rotaop = result->aop;
      if (!isRegDead (A_IDX, ic))
        {
          _push (PAIR_AF);
          pushed_a = true;
        }
      if (size == 2 && (aopInReg (left->aop, 0, HL_IDX) && isRegDead (HL_IDX, ic) || IS_RAB && aopInReg (left->aop, 0, DE_IDX) && isRegDead (DE_IDX, ic)))
        rotaop = left->aop;
      genMove (rotaop, left->aop, true, isRegDead (HL_IDX, ic), isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic));
      cheapMove (ASMOP_A, 0, rotaop, size - 1, true);
      emit3 (A_RLA, 0, 0);
      if (IS_SM83 && requiresHL (rotaop) && rotaop->type != AOP_REG)
        { /* ldhl sp,N changes CARRY */
          emit3_o (A_RRA, 0, 0, 0, 0);
          if (!regalloc_dry_run)
            aopGet (rotaop, 0, false);
          emit3_o (A_RLA, 0, 0, 0, 0);
        }
      for (int i = 0; i < size;)
        {
          if (!IS_SM83 && i + 1 < size && aopInReg (rotaop, i, HL_IDX))
            {
              emit2 ("adc hl, hl");
              cost2 (2, 15, 10, 4, 0, 8, 2, 2);
              i += 2;
            }
          else if (IS_RAB && i + 1 < size && aopInReg (rotaop, i, DE_IDX))
            {
              emit2 ("rl de");
              cost (1, 2);
              i += 2;
            }
          else
            {
              emit3_o (A_RL, rotaop, i, 0, 0);
              i++;
            }
        }
     genMove (result->aop, rotaop, true, isRegDead (HL_IDX, ic), isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic));
    }
  else
    {
      if (!isRegDead (A_IDX, ic))
      {
        _push (PAIR_AF);
        pushed_a = true;
      }

      for (int offset = 0; offset < size; ++offset)
        {
          _moveA (aopGet (left->aop, offset, false));
          emit3_o (A_RLA, 0, 0, 0, 0);
          if (offset != 0)
            aopPut (result->aop, "a", offset);
        }
      _moveA (aopGet (left->aop, 0, false));
      emit3_o (A_RLA, 0, 0, 0, 0);
      aopPut (result->aop, "a", 0);
    }

  if (pushed_a)
    _pop (PAIR_AF);

  freeAsmop (IC_LEFT (ic), 0);
  freeAsmop (IC_RESULT (ic), 0);
}

/*-----------------------------------------------------------------*/
/* genGetByte - generates code to get a single byte                */
/*-----------------------------------------------------------------*/
static void
genGetByte (const iCode *ic)
{
  operand *left, *right, *result;
  int offset;

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (right, ic, FALSE, FALSE);
  aopOp (result, ic, TRUE, FALSE);

  offset = (int) ulFromVal (right->aop->aopu.aop_lit) / 8;
  genMove_o (result->aop, 0, left->aop, offset, 1, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);

  freeAsmop (result, NULL);
  freeAsmop (right, NULL);
  freeAsmop (left, NULL);
}

/*-----------------------------------------------------------------*/
/* genGetWord - generates code to get a 16-bit word                */
/*-----------------------------------------------------------------*/
static void
genGetWord (const iCode *ic)
{
  operand *left, *right, *result;
  int offset;

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (right, ic, FALSE, FALSE);
  aopOp (result, ic, TRUE, FALSE);

  offset = (int) ulFromVal (right->aop->aopu.aop_lit) / 8;
  genMove_o (result->aop, 0, left->aop, offset, 2, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);

  freeAsmop (result, NULL);
  freeAsmop (right, NULL);
  freeAsmop (left, NULL);
}

/*-----------------------------------------------------------------*/
/* genGetAbit - generates code get a single bit                    */
/*-----------------------------------------------------------------*/
static void
genGetAbit (const iCode * ic)
{
  operand *left, *right, *result;
  int shCount;

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (right, ic, FALSE, FALSE);
  aopOp (result, ic, TRUE, FALSE);

  shCount = (int) ulFromVal (right->aop->aopu.aop_lit);

  /* get the needed byte into a */
  cheapMove (ASMOP_A, 0, left->aop, shCount / 8, true);
  shCount %= 8;
  if (shCount == 4 && (IS_SM83 || IS_Z80N))
    {
      emit3_o (A_SWAP, ASMOP_A, 0, 0, 0);
      shCount -= 4;
    }
  if (result->aop->type == AOP_CRY)
    {
      
      if (shCount < 4)
        while (shCount-- >= 0)
          emit3_o (A_RRCA, 0, 0, 0, 0);
      else
        while (shCount++ < 8)
          emit3_o (A_RLCA, 0, 0, 0, 0);
      outBitC (result);
    }
  else
    {
      if (shCount < 5)
        while (shCount-- > 0)
          emit3_o (A_RRCA, 0, 0, 0, 0);
      else
        while (shCount++ < 8)
          emit3_o (A_RLCA, 0, 0, 0, 0);
      emit2 ("and a, !immedbyte", 0x01u);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      outAcc (result);
    }

  freeAsmop (result, NULL);
  freeAsmop (right, NULL);
  freeAsmop (left, NULL);
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
    (getPairId (result->aop) == PAIR_HL || getPairId (result->aop) == PAIR_DE))
    {
      bool op_de = (getPairId (result->aop) == PAIR_DE);
      fetchPairLong (getPairId (result->aop), left->aop, ic, offl);
      while (shCount--)
        {
          emit3 (A_CP, ASMOP_A, ASMOP_A);
          cost (1, 2);
          emit2 (op_de? "rr de" : "rr hl");
          cost (1, 2);
        }
      return;
    }
  else if (!IS_SM83 && !IS_RAB && !is_signed && aopSame (result->aop, offr, left->aop, offl, 2) && isPairDead (PAIR_HL, ic) && isRegDead (A_IDX, ic) &&
    (shCount == 4 || shCount == 5) &&
    (result->aop->type == AOP_DIR || result->aop->type == AOP_HL || result->aop->type == AOP_IY))
    {
      emit2 ("xor a, a");
      cost2 (1, 4, 4, 2, 4, 4, 1, 1);
      emit2 ("ld hl, !hashedstr+1", result->aop->aopu.aop_dir);
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit3 (A_RRD, 0, 0);
      if (shCount == 5)
        {
          emit2 ("srl (hl)");
          cost2 (2, 15, 6, 10, 16, 8, 5, 5);
        }
      emit2 ("dec hl");
      cost2 (1, 6, 4, 2, 8, 4, 1, 1);
      emit3 (A_RRD, 0, 0);
      if (shCount == 5)
        {
          emit2 ("rr (hl)");
          cost2 (2, 15, 6, 10, 16, 8, 5, 5);
        }
      return;
    }
  else if (IS_RAB && !is_signed && shCount >= 2 && isPairDead (PAIR_HL, ic) &&
      ((isPair (left->aop) && getPairId (left->aop) == PAIR_HL || isPair (result->aop)
        && getPairId (result->aop) == PAIR_HL) && isPairDead (PAIR_DE, ic) || isPair (left->aop)
       && getPairId (left->aop) == PAIR_DE))
    {
      bool op_de = (getPairId (left->aop) == PAIR_DE);
      if (op_de)
        emit2 ("ld hl, !immedword", 0xffffu >> shCount);
      else
        {
          fetchPair (PAIR_HL, left->aop);
          emit2 ("ld de, !immedword", 0xffffu >> shCount);
        }
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      while (shCount--)
        {
          emit2 (op_de ? "rr de" : "rr hl");
          cost (1, 2);
        }
      emit2 ("and hl, de");
      cost (1, 2);
      genMove (IC_RESULT (ic)->aop, ASMOP_HL, true, true, isPairDead (PAIR_DE, ic), true);
      return;
    }
  else if ((getPairId (result->aop) == PAIR_HL || getPairId (left->aop) == PAIR_HL) && isPairDead (PAIR_HL, ic) &&
    shCount == 7 && is_signed)
    {
      tlbl = regalloc_dry_run ? 0 : newiTempLabel (NULL);
      genMove (ASMOP_HL, left->aop, isRegDead (A_IDX, ic), true, isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic));
      emit3w (A_ADD, ASMOP_HL, ASMOP_HL);
      emit3 (A_LD, ASMOP_L, ASMOP_H);
      emit3 (A_LD, ASMOP_H, ASMOP_ZERO);
      if (!regalloc_dry_run)
        emit2 ("jr nc,!tlabel", labelKey2num (tlbl->key));
      emit2 ("dec h");
      if (!regalloc_dry_run)
        emitLabel (tlbl);
      cost (3, 11.5f);
      genMove (result->aop, ASMOP_HL, isRegDead (A_IDX, ic), true, isRegDead (DE_IDX, ic), isRegDead (IY_IDX, ic));
      return;
    }
  // If the leading bits are all the same, we can shift the other way, and use efficient 16-bit addition for shifts.
  else if (shCount < 8 &&
    aopInReg (left->aop, 0, HL_IDX) && aopInReg (result->aop, 0, H_IDX) && isRegDead (L_IDX, ic) && isRegDead (A_IDX, ic) &&
    shCount >= 5 - !optimize.codeSpeed) // Smaller code size for 4 and above, but at least for Z80(N), only faster from 5.
    {
      emit3 (A_XOR, ASMOP_A, ASMOP_A);
      emit2 ("add hl, hl");
      cost2 (1, 11, 7, 2, 8, 8, 1, 1);
      if (is_signed && (left->aop->valinfo.anything || left->aop->valinfo.min < 0))
        {
          tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
          if (!regalloc_dry_run)
            emit2 ("jr nc,!tlabel", labelKey2num (tlbl->key));
          emit2 ("dec a");
          cost2 (3, 11.5, 9.0, 7.0, 12.0, 7.0, 3.0, 3.0);
          if (!regalloc_dry_run)
            emitLabel (tlbl);
        }
      else if (!is_signed)
        emit3 (A_RLA, 0, 0);
      for (int i = 1; i < (8 - shCount); i++)
        {
          emit2 ("add hl, hl");
          cost2 (1, 11, 7, 2, 8, 8, 1, 1);
          emit3 (A_RLA, 0, 0);
        }
      genMove_o (result->aop, 1, ASMOP_A, 0, 1, true, false, isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic), true);
      return;
    }

  if (isPair (result->aop) && !offr)
    fetchPairLong (getPairId (result->aop), left->aop, ic, offl);
  else
    genMove_o (result->aop, offr, left->aop, offl, 2, true, isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);

  if (shCount == 0)
    return;

  /*  if (result->aop->type == AOP_REG) { */

  /* Left is already in result - so now do the shift */
  /* Optimizing for speed by default. */
  if (!optimize.codeSize || shCount <= 2)
    {
      while (shCount--)
        emitRsh2 (result->aop, size, is_signed);
    }
  else
    {
      bool use_b = (!IS_SM83 && isRegDead (B_IDX, ic)
                    && !(result->aop->type == AOP_REG
                         && (result->aop->aopu.aop_reg[0]->rIdx == B_IDX || result->aop->aopu.aop_reg[1]->rIdx == B_IDX)));

      tlbl = regalloc_dry_run ? 0 : newiTempLabel (NULL);
      
      if (requiresHL (result->aop))
        spillPair (PAIR_HL);

      emit2 ("ld %s, !immedbyte", use_b ? "b" : "a", (unsigned)shCount);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);

      regalloc_dry_run_state_scale *= (unsigned)shCount;

      if (!regalloc_dry_run)
        emitLabel (tlbl);

      emitRsh2 (result->aop, size, is_signed);

      if (use_b)
        {
          if (!regalloc_dry_run)
            emit2 ("djnz !tlabel", labelKey2num (tlbl->key));
          cost2 (2, 13, 9, 5, 0, 10, 4, 2); // Assume jump taken.
        }
      else
        {
          emit3 (A_DEC, ASMOP_A, 0);
          if (!regalloc_dry_run)
            emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
          cost2 (2, 12, 8, 5, 12, 8, 3, 3); // Assume jump taken, and optimized to jr.
        }

      regalloc_dry_run_state_scale = 1.0f;
    }
}

/*-----------------------------------------------------------------*/
/* shiftL2Left2Result - shift left two bytes from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftL2Left2Result (operand *left, operand *result, int shCount, const iCode *ic)
{
  asmop *shiftaop = result->aop;

  if (shCount == 7 && aopIsLitVal (left->aop, 1, 1, 0x00) && result->aop->type == AOP_REG &&
    result->aop->aopu.aop_reg[0]->rIdx != IYL_IDX && result->aop->aopu.aop_reg[1]->rIdx != IYL_IDX && result->aop->aopu.aop_reg[0]->rIdx != IYH_IDX && result->aop->aopu.aop_reg[1]->rIdx != IYH_IDX)
    {
      genMove_o (result->aop, 1, left->aop, 0, 1, isRegDead(A_IDX, ic), false, false, false, true);
      bool reuse_zero = left->aop->type == AOP_REG && !aopInReg (left->aop, 1, IYL_IDX) && !aopInReg (left->aop, 1, IYH_IDX) && !aopInReg (left->aop, 1, result->aop->aopu.aop_reg[1]->rIdx);
      genMove_o (result->aop, 0, reuse_zero ? left->aop : ASMOP_ZERO, 1, 1, isRegDead(A_IDX, ic) && !aopInReg (result->aop, 1, A_IDX), false, false, false, true);
      emit3_o (A_SRL, result->aop, 1, 0, 0);
      if (aopInReg (result->aop, 0, A_IDX))
        emit3 (A_RRA, 0, 0);
      else
        emit3 (A_RR, result->aop, 0);
      return;
    }
  /* For a shift of 7 we can use cheaper right shifts */
  else if (shCount == 7 && left->aop->type == AOP_REG && !bitVectBitValue (ic->rSurv, left->aop->aopu.aop_reg[0]->rIdx) && result->aop->type == AOP_REG &&
    left->aop->aopu.aop_reg[0]->rIdx != IYL_IDX && left->aop->aopu.aop_reg[1]->rIdx != IYL_IDX && left->aop->aopu.aop_reg[0]->rIdx != IYH_IDX && left->aop->aopu.aop_reg[1]->rIdx != IYH_IDX &&
    result->aop->aopu.aop_reg[0]->rIdx != IYL_IDX && result->aop->aopu.aop_reg[1]->rIdx != IYL_IDX && result->aop->aopu.aop_reg[0]->rIdx != IYH_IDX && result->aop->aopu.aop_reg[1]->rIdx != IYH_IDX &&
    (optimize.codeSpeed || getPairId (result->aop) != PAIR_HL || getPairId (left->aop) != PAIR_HL)) /* but a sequence of add hl, hl might still be cheaper code-size wise */
    {
      // Handling the low byte in A with xor clearing is cheaper.
      bool special_a = (isRegDead (A_IDX, ic) && !aopInReg (left->aop, 0, A_IDX) && !aopInReg (left->aop, 1, A_IDX));
      asmop *lowbyte = special_a ? ASMOP_A : result->aop;

      if (special_a)
        emit3 (A_XOR, ASMOP_A, ASMOP_A);
      emit3_o (A_RR, left->aop, 1, 0, 0);
      emit3_o (A_LD, result->aop, 1, left->aop, 0);
      emit3_o (A_RR, result->aop, 1, 0, 0);
      if (!special_a)
        emit3_o (A_LD, result->aop, 0, ASMOP_ZERO, 0);
      if (aopInReg (lowbyte, 0, A_IDX))
        emit3 (A_RRA, 0, 0);
      else
        emit3 (A_RR, lowbyte, 0);
      if (special_a)
        cheapMove (result->aop, 0, lowbyte, 0, true);
      return;
    }
  if ((result->aop->type == AOP_HL || result->aop->type == AOP_IY || IS_RAB && result->aop->type == AOP_STK) && // Being able to use cheap add hl, hl is worth it in most cases.
    (left->aop->type == AOP_HL || left->aop->type == AOP_IY || IS_RAB && left->aop->type == AOP_STK) &&
    !IS_SM83 && isPairDead (PAIR_HL, ic) &&
    (shCount > 1 || !sameRegs (result->aop, left->aop)) ||
    isPairDead (PAIR_HL, ic) && !IS_SM83 && getPairId (result->aop) == PAIR_DE && getPairId (left->aop) != PAIR_DE) // Shift in hl if we can cheaply move to de via ex later.
    {
      shiftaop = ASMOP_HL;
      genMove (ASMOP_HL, left->aop, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), true);
    }
  else if (result->aop->type != AOP_REG && left->aop->type == AOP_REG && left->aop->size >= 2 && !bitVectBitValue (ic->rSurv, left->aop->aopu.aop_reg[0]->rIdx) && !bitVectBitValue (ic->rSurv, left->aop->aopu.aop_reg[1]->rIdx) ||
    getPairId (left->aop) == PAIR_HL && isPairDead (PAIR_HL, ic))
    shiftaop = left->aop;
  else
    genMove_o (result->aop, 0, left->aop, 0, 2, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);

  if (shCount == 0)
    ;
  else if (getPairId (shiftaop) == PAIR_HL)
    {
      while (shCount--)
        emit3w (A_ADD, ASMOP_HL, ASMOP_HL);
    }
  else if (getPairId (shiftaop) == PAIR_IY)
    {
      while (shCount--)
        {
          emit2 ("add iy, iy");
          cost2 (2, 15, 10, 4, 0, 8, 2, 2);
        }
    }
  else if (IS_RAB && getPairId (shiftaop) == PAIR_DE && shCount <= 2 + optimize.codeSpeed)
    {
      while (shCount--)
        {
          emit3 (A_CP, ASMOP_A, ASMOP_A);
          emit2 ("rl de");
          cost (1, 2);
        }
    }
  else if (!IS_SM83 && getPairId (shiftaop) == PAIR_DE)
    {
      emit3w (A_EX, ASMOP_DE, ASMOP_HL);
      while (shCount--)
        emit3w (A_ADD, ASMOP_HL, ASMOP_HL);
      emit3w (A_EX, ASMOP_DE, ASMOP_HL);
    }
  else
    {
      int size = 2;
      int offset = 0;
      
      bool use_b = (!IS_SM83 && isRegDead (B_IDX, ic)
        && (shiftaop->type != AOP_REG || shiftaop->aopu.aop_reg[0]->rIdx != B_IDX && shiftaop->aopu.aop_reg[1]->rIdx != B_IDX));
                         
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);

      if (shiftaop->type == AOP_REG)
        {
          while (shCount--)
            {
              for (offset = 0; offset < size; offset++)
                if (aopInReg (shiftaop, offset, A_IDX))
                  emit3 (offset ? A_ADC : A_ADD, ASMOP_A, ASMOP_A);
                else
                  emit3_o (offset ? A_RL : A_SLA, shiftaop, offset, 0, 0);
            }
        }
      else
        {
          if (!use_b && !isRegDead (A_IDX, ic))
            _push (PAIR_AF);

          /* Left is already in result - so now do the shift */
          if (shCount > 1)
            {
              if (!regalloc_dry_run)
                {
                  emit2 ("ld %s, !immedbyte", use_b ? "b" : "a", (unsigned)shCount);
                  emitLabel (tlbl);
                }
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
              
              if (requiresHL (shiftaop))
                spillPair (PAIR_HL);
            }

          while (size--)
            {
              emit3_o (offset ? A_RL : A_SLA, shiftaop, offset, 0, 0);

              offset++;
            }
          if (shCount > 1)
            {
              if (use_b)
                {
                  if (!regalloc_dry_run)
                    emit2 ("djnz !tlabel", labelKey2num (tlbl->key));
                  cost2 (2, 13, 9, 5, 0, 10, 4, 2); // Assume jump taken.
                }
              else
                {
                  emit3 (A_DEC, ASMOP_A, 0);
                  if (!regalloc_dry_run)
                    emit2 ("jp NZ, !tlabel", labelKey2num (tlbl->key));
                  cost2 (2, 12, 8, 5, 12, 8, 3, 3); // Assume jump taken, and optimized to jr.
                }
            }
          if (!use_b && !isRegDead (A_IDX, ic))
            _pop (PAIR_AF);
        }
    }

  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);
  if (maskedtopbyte)
    {
      bool pushed_a = false;
      if (!isRegDead (A_IDX, ic) || shiftaop->regs[A_IDX] >= 0 && shiftaop->regs[A_IDX] != result->aop->size - 1)
        {
          _push (PAIR_AF);
          pushed_a = true;
        }
      cheapMove (ASMOP_A, 0, shiftaop, result->aop->size - 1, true);
      emit2 ("and a, #0x%02x", topbytemask);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      cheapMove (shiftaop, result->aop->size - 1, ASMOP_A, 0, true);
      if (pushed_a)
        _pop (PAIR_AF);
    }

  if (shiftaop != result->aop)
    {
      if (isPair (result->aop))
        fetchPairLong (getPairId (result->aop), shiftaop, ic, 0);
      else
        genMove_o (result->aop, 0, shiftaop, 0, 2, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);
    }
}

/*-----------------------------------------------------------------*/
/* genSwap - generates code to swap nibbles or bytes               */
/*-----------------------------------------------------------------*/
static void
genSwap (iCode * ic)
{
  operand *left, *result;
  asmop swapped_result_aop;
  
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false, false);
  aopOp (result, ic, true, false);
  bool pushed_a = false;
  switch (left->aop->size)
    {
    case 2: // swap bytes in word
      if (result->aop->type == AOP_REG) // Create result asmop with swapped bytes, let genMove handle the details.
        {
          signed char idxarray[3];
          idxarray[0] = result->aop->aopu.aop_reg[1]->rIdx;
          idxarray[1] = result->aop->aopu.aop_reg[0]->rIdx;
          idxarray[2] = -1;
          z80_init_reg_asmop (&swapped_result_aop, idxarray);
          genMove (&swapped_result_aop, left->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);
          break;
        }

      if (sameRegs (result->aop, left->aop) || operandsEqu (result, left))
        {
          // avoid push/pop by finding free register
          asmop *free_reg = ASMOP_A;
          if (isRegDead (B_IDX, ic))
            free_reg = ASMOP_B;
          if (isRegDead (C_IDX, ic))
            free_reg = ASMOP_C;
          if (isRegDead (D_IDX, ic))
            free_reg = ASMOP_D;
          if (isRegDead (E_IDX, ic))
            free_reg = ASMOP_E;
          cheapMove (ASMOP_A, 0, left->aop, 0, true);
          if (free_reg == ASMOP_A)
            {
              _push (PAIR_AF);
              pushed_a = true;
            }
          // if left and result are registers, this should get optimized away
          cheapMove (free_reg, 0, left->aop, 1, FALSE);
          cheapMove (result->aop, (free_reg == ASMOP_A ? 0 : 1), ASMOP_A, 0, FALSE);
          if(pushed_a){
            _pop (PAIR_AF);
          }
          cheapMove (result->aop, (free_reg == ASMOP_A ? 1 : 0), free_reg, 0, TRUE);
        }
      else
        {
          cheapMove (result->aop, 0, left->aop, 1, isRegDead (A_IDX, ic) && left->aop->regs[A_IDX] != 0);
          cheapMove (result->aop, 1, left->aop, 0, isRegDead (A_IDX, ic) && result->aop->regs[A_IDX] != 0);
        }
      break;

    case 4: // swap words in double word
        if (result->aop->type == AOP_REG) // Create result asmop with swapped words, let genMove handle the details.
        {
          signed char idxarray[5];
          idxarray[0] = result->aop->aopu.aop_reg[2]->rIdx;
          idxarray[1] = result->aop->aopu.aop_reg[3]->rIdx;
          idxarray[2] = result->aop->aopu.aop_reg[0]->rIdx;
          idxarray[3] = result->aop->aopu.aop_reg[1]->rIdx;
          idxarray[4] = -1;
          z80_init_reg_asmop(&swapped_result_aop, idxarray);
          genMove (&swapped_result_aop, left->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);
          break;
        }
      if (operandsEqu (result, left) && left->aop->type == AOP_STK &&
          spOffset (left->aop->aopu.aop_stk) == 0 && isPairDead (PAIR_HL, ic) &&
          (!IS_SM83 || isPairDead(PAIR_DE, ic) || isPairDead(PAIR_BC, ic)))
        { /* result & left are top of stack and there are free register pairs */
          if (IS_SM83)
            {
              if (isPairDead(PAIR_DE, ic))
                {
                  _pop (PAIR_DE);
                  _pop (PAIR_HL);
                  _push (PAIR_DE);
                  _push (PAIR_HL);
                }
              else
                {
                  _pop (PAIR_BC);
                  _pop (PAIR_HL);
                  _push (PAIR_BC);
                  _push (PAIR_HL);
                }
            }
          else
            {
              _pop (PAIR_HL);
              emit2 ("ex (sp), hl");
              cost2 (1 + IS_RAB, 19, 16, 15, 0, 14, 5, 5);
              _push (PAIR_HL);
            }
          break;
        }

      /* --== generic implementations ==-- */
      if (!operandsEqu (result, left))
        { /* result is registers or left differs than result */
          bool pushed = !isRegDead (A_IDX, ic);
          if (pushed)
            _push (PAIR_AF);
          cheapMove (ASMOP_A, 0, left->aop, 0, TRUE);
          cheapMove (result->aop, 0, left->aop, 2, FALSE);
          cheapMove (result->aop, 2, ASMOP_A, 0, FALSE);
          cheapMove (ASMOP_A, 0, left->aop, 1, TRUE);
          cheapMove (result->aop, 1, left->aop, 3, FALSE);
          cheapMove (result->aop, 3, ASMOP_A, 0, FALSE);
          if (pushed)
            _pop (PAIR_AF);
        }
      else
        {
          asmop *tmp = NULL;
          bool pushed = false;
          bool dead_a = isRegDead (A_IDX, ic);
          bool dead_de = isPairDead (PAIR_DE, ic);
          bool dead_hl = isPairDead (PAIR_HL, ic);
          if (isPairDead (PAIR_BC, ic) &&
              (left->aop->type != AOP_REG || !aopInReg (left->aop, 0, BC_IDX)))
            tmp = ASMOP_BC;
          else if (dead_hl &&
              (left->aop->type != AOP_REG || !aopInReg (left->aop, 0, HL_IDX)))
            tmp = ASMOP_HL;
          else if (dead_de &&
                   (left->aop->type != AOP_REG || !aopInReg (left->aop, 0, DE_IDX)))
            tmp = ASMOP_DE;
          else if (!IS_SM83 && !IY_RESERVED && isPairDead (PAIR_IY, ic) &&
                   (left->aop->type != AOP_REG || !aopInReg (left->aop, 0, IY_IDX)))
            tmp = ASMOP_IY;
          else 
            {
              pushed = true;
              if ((left->aop->type != AOP_REG || !aopInReg (left->aop, 0, HL_IDX)))
                tmp = ASMOP_HL;
              else if ((left->aop->type != AOP_REG || !aopInReg (left->aop, 0, BC_IDX)))
                tmp = ASMOP_BC;
              else if ((left->aop->type != AOP_REG || !aopInReg (left->aop, 0, DE_IDX)))
                tmp = ASMOP_DE;
              else
                wassertl (FALSE, "impossible case");
              _push (tmp->aopu.aop_pairId);
            }
          genMove_o (tmp, 0, left->aop, 0, 2, dead_a, dead_hl && !aopInReg (left->aop, 2, HL_IDX), dead_de && !aopInReg (left->aop, 2, DE_IDX), true, true);
          genMove_o (result->aop, 0, left->aop, 2, 2, dead_a, dead_hl && (tmp != ASMOP_HL), dead_de && (tmp != ASMOP_DE), true, true);
          genMove_o (result->aop, 2, tmp, 0, 2, dead_a, dead_hl && !aopInReg (result->aop, 0, HL_IDX), dead_de && !aopInReg (result->aop, 0, DE_IDX), true, true);
          if (pushed)
            _pop (tmp->aopu.aop_pairId);
        }
      break;
    default:
      wassertl (FALSE, "unsupported SWAP operand size");
    }

  freeAsmop (result, NULL);
  freeAsmop (left, NULL);
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
      if (IS_SM83 || IS_Z80N)
        {
          emit3 (A_SWAP, ASMOP_A, 0);
          break;
        }
      emit3 (A_RLCA, 0, 0);
    case 3:
      if (IS_SM83 || IS_Z80N)
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
      if (IS_SM83 || IS_Z80N)
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

  if (shCount <= 3 + !IS_SM83)
    while (shCount--)
      emit3 (A_ADD, ASMOP_A, ASMOP_A);
  else
    {
      /* rotate left accumulator */
      AccRol (shCount);
      /* and kill the lower order bits */
      emit2 ("and a, !immedbyte", (unsigned)(SLMask[shCount]));
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
    }
}

/*-----------------------------------------------------------------*/
/* shiftL1Left2Result - shift left one byte from left to result    */
/*-----------------------------------------------------------------*/
static void
shiftL1Left2Result (operand *left, int offl, operand *result, int offr, unsigned int shCount, const iCode *ic)
{
  if (!shCount)
    cheapMove (result->aop, offr, left->aop, offl, isRegDead (A_IDX, ic));
  // add hl, hl is cheap in code size. On Rabbits it is also fastest.
  else if (sameRegs (result->aop, left->aop) && aopInReg (result->aop, offr, L_IDX) && isPairDead(PAIR_HL, ic) && offr == offl && (!optimize.codeSpeed && IS_RAB))
    {
      while (shCount--)
        emit3w (A_ADD, ASMOP_HL, ASMOP_HL);
    }
  else if (!IS_SM83 && !IS_RAB && aopSame (result->aop, offr, left->aop, offr, 1) && !offr && shCount == 4 && isPairDead (PAIR_HL, ic) && isRegDead (A_IDX, ic) &&
    (result->aop->type == AOP_DIR || result->aop->type == AOP_HL || result->aop->type == AOP_IY))
    {
      emit2 ("xor a, a");
      cost2 (1, 4, 4, 2, 4, 4, 1, 1);
      pointPairToAop (PAIR_HL, result->aop, 0);
      emit3 (A_RLD, 0, 0);
    }
  /* If operand and result are the same we can shift in place.
     However shifting in acc using add is cheaper than shifting
     in place using sla; when shifting by more than 2 shifting in
     acc it is worth the additional effort for loading from / to acc. */
  else if (!aopInReg(result->aop, 0, A_IDX) && sameRegs (left->aop, result->aop) && shCount <= 2 && offr == offl)
    {
      while (shCount--)
        emit3 (A_SLA, result->aop, 0);
    }
  else if ((IS_Z180 && !optimize.codeSpeed || IS_EZ80_Z80 || IS_Z80N) && // Try to use mlt
    (!IS_Z80N && aopInReg (result->aop, offr, C_IDX) && isPairDead(PAIR_BC, ic) || aopInReg (result->aop, offr, E_IDX) && isPairDead(PAIR_DE, ic) || !IS_Z80N && aopInReg (result->aop, offr, L_IDX) && isPairDead(PAIR_HL, ic)))
    {
      PAIR_ID pair = aopInReg (result->aop, offr, C_IDX) ? PAIR_BC : (aopInReg (result->aop, offr, E_IDX) ? PAIR_DE : PAIR_HL);

      bool top = aopInReg (left->aop, offl, _pairs[pair].h_idx);
      if (!top)
        cheapMove (pair == PAIR_BC ? ASMOP_C : (pair == PAIR_DE ? ASMOP_E : ASMOP_L), 0, left->aop, offl, isRegDead (A_IDX, ic));

      emit2 ("ld %s, !immed%d", top ? _pairs[pair].l : _pairs[pair].h, 1 << shCount);
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("mlt %s", _pairs[pair].name);
      cost2 (2, 8, 17, 0, 0, 0, 6, 0);
    }
  else
    {
      if (!isRegDead (A_IDX, ic))
        _push (PAIR_AF);
      cheapMove (ASMOP_A, 0, left->aop, offl, true);
      /* shift left accumulator */
      AccLsh (shCount);
      cheapMove (result->aop, offr, ASMOP_A, 0, true);
      if (!isRegDead (A_IDX, ic))
        _pop (PAIR_AF);
    }


  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);
  if (maskedtopbyte)
    {
      bool pushed_a = false;
      if (!isRegDead (A_IDX, ic) || result->aop->regs[A_IDX] >= 0 && result->aop->regs[A_IDX] != result->aop->size - 1)
        {
          _push (PAIR_AF);
          pushed_a = true;
        }
      cheapMove (ASMOP_A, 0, result->aop, result->aop->size - 1, true);
      emit2 ("and a, #0x%02x", topbytemask);
      cost (1, 7);
      cheapMove (result->aop, result->aop->size - 1, ASMOP_A, 0, true);
      if (pushed_a)
        _pop (PAIR_AF);
    }
}

/*-----------------------------------------------------------------*/
/* genlshTwo - left shift two bytes by known amount                */
/*-----------------------------------------------------------------*/
static void
genlshTwo (operand *result, operand *left, unsigned int shCount, const iCode *ic)
{
  int size = result->aop->size;

  wassert (size == 2);

  if (IS_Z80N && !operandType (result) &&
    aopInReg (result->aop, 0, DE_IDX) && isRegDead (B_IDX, ic) &&
    shCount > 2 && shCount != 8) // Only worth it when shifting by more than 2 (we can get a cheap path using add hl, hl in shiftL2Left2Result (left, result, shCount, ic)).
    {
      genMove (ASMOP_DE, left->aop, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), true, true);
      emit2 ("ld b, !immedbyte", (unsigned)shCount);
      cost (2, 7);
      emit2 ("bsla de, b");
      cost (2, 8);
    }
  else if (shCount >= 8)
    {
      shCount -= 8;
      shiftL1Left2Result (left, 0, result, 1, shCount, ic);
      cheapMove (result->aop, 0, ASMOP_ZERO, 0, isRegDead (A_IDX, ic));
    }
  else
    shiftL2Left2Result (left, result, shCount, ic);
}

/*-----------------------------------------------------------------*/
/* genRot1 - generates code for rotation of 8-bit values           */
/*-----------------------------------------------------------------*/
static void
genRot1 (iCode *ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  aopOp (left, ic, false, false);
  aopOp (result, ic, true, false);

  wassert (bitsForType (operandType (left)) == 8);
  wassert (IS_OP_LITERAL (right));

  int s = operandLitValueUll (right) % 8;
  
  if (IS_SM83 && s == 4 && result->aop->type == AOP_REG)
    {
      cheapMove (result->aop, 0, left->aop, 0, true);
      emit3 (A_SWAP, result->aop, 0);
    }
  else if (IS_SM83 && s == 4 && left->aop->type == AOP_REG && !bitVectBitValue (ic->rSurv, left->aop->aopu.aop_reg[0]->rIdx))
    {
      emit3 (A_SWAP, left->aop, 0);
      cheapMove (result->aop, 0, left->aop, 0, true);
    }        
  else if ((s == 1 || s == 7) && result->aop->type == AOP_REG && !aopInReg (result->aop, 0, A_IDX) && !(aopInReg (left->aop, 0, A_IDX) && isRegDead (A_IDX, ic)))
    {
      cheapMove (result->aop, 0, left->aop, 0, true);
      emit3 (s == 1 ? A_RLC : A_RRC, result->aop, 0);
    }
  else if ((s == 1 || s == 7) && left->aop->type == AOP_REG && !bitVectBitValue (ic->rSurv, left->aop->aopu.aop_reg[0]->rIdx) && !aopInReg (left->aop, 0, A_IDX))
    {
      emit3 (s == 1 ? A_RLC : A_RRC, left->aop, 0);
      cheapMove (result->aop, 0, left->aop, 0, true);
    }
  else if (s <= 2 && aopInReg (result->aop, 0, H_IDX) && isRegDead (L_IDX, ic) && (aopInReg (left->aop, 0, H_IDX) || aopInReg (left->aop, 0, L_IDX)))
    {
      if (aopInReg (left->aop, 0, H_IDX))
        emit3_o (A_LD, ASMOP_HL, 0, ASMOP_HL, 1);
      else
        emit3_o (A_LD, ASMOP_HL, 1, ASMOP_HL, 0);
      while (s--)
        emit3w (A_ADD, ASMOP_HL, ASMOP_HL);
    }
  else if (IS_RAB && (s == 1 && aopInReg (result->aop, 0, D_IDX) && isRegDead (E_IDX, ic) || s == 7 && aopInReg (result->aop, 0, E_IDX) && isRegDead (D_IDX, ic)) &&
    (aopInReg (left->aop, 0, D_IDX) || aopInReg (left->aop, 0, E_IDX)))
    {
      if (aopInReg (left->aop, 0, D_IDX))
        emit3_o (A_LD, ASMOP_DE, 0, ASMOP_DE, 1);
      else
        emit3_o (A_LD, ASMOP_DE, 1, ASMOP_DE, 0);
      if (s == 1)
        emit2 ("rl de");
      else
        emit2 ("rr de");
      cost (1, 2);
    }
  else if (s == 4 && !IS_SM83 && !IS_RAB && (aopInReg (left->aop, 0, A_IDX) || aopSame (result->aop, 0, left->aop, 0, 1)) &&
    (result->aop->type == AOP_DIR || result->aop->type == AOP_HL || result->aop->type == AOP_IY) && isPairDead (PAIR_HL, ic))
    {
      if (!isRegDead (A_IDX, ic))
        _push (PAIR_AF);
      if (!aopSame (result->aop, 0, left->aop, 0, 1))
        cheapMove (result->aop, 0, ASMOP_A, 0, false);
      else
        cheapMove (ASMOP_A, 0, result->aop, 0, true);
      pointPairToAop (PAIR_HL, result->aop, 0);
      emit3 (A_RRD, 0, 0);
      if (!isRegDead (A_IDX, ic))
        _pop (PAIR_AF);
    }
  else if ((s == 1 || s == 7) && aopSame (result->aop, 0, left->aop, 0, 1) &&
    ((result->aop->type == AOP_EXSTK || IS_SM83 && result->aop->type == AOP_STK) || result->aop->type == AOP_DIR || result->aop->type == AOP_HL || result->aop->type == AOP_IY) && isPairDead (PAIR_HL, ic))
    {
      pointPairToAop (PAIR_HL, result->aop, 0);
      emit2 (s == 1 ? "rlc (hl)" : "rrc (hl)");
      cost2 (2, 15, 13, 10, 16, 8, 5, 5);
    }
  else if ((s == 1 || s == 7) && aopSame (result->aop, 0, left->aop, 0, 1) && result->aop->type == AOP_STK && !IS_SM83)
    {
      emit3 (s == 1 ? A_RLC : A_RRC, result->aop, 0);
    }
  else
    {
      if (!isRegDead (A_IDX, ic))
        _push (PAIR_AF);
      cheapMove (ASMOP_A, 0, left->aop, 0, true);
      AccRol (s);
      cheapMove (result->aop, 0, ASMOP_A, 0, true);
      if (!isRegDead (A_IDX, ic))
        _pop (PAIR_AF);
    }

  freeAsmop (left, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* genRot - generates code for rotation                            */
/*-----------------------------------------------------------------*/
static void
genRot (iCode *ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  unsigned int lbits = bitsForType (operandType (left));
  if (lbits == 8 && IS_OP_LITERAL (right))
    genRot1 (ic);
  else if (IS_OP_LITERAL (right) && operandLitValueUll (right) % lbits == 1)
    genRLC (ic);
  else if (IS_OP_LITERAL (right) && operandLitValueUll (right) % lbits == lbits - 1)
    genRRC (ic);
  else if (IS_OP_LITERAL (right) && (operandLitValueUll (right) % lbits) * 2 == lbits)
    genSwap (ic);
  else
    wassertl (0, "Unsupported rotation.");
}

/*------------------------------------------------------------------*/
/* genLeftShiftLiteral - left shifting by known count for size <= 2 */
/*------------------------------------------------------------------*/
static void
genLeftShiftLiteral (operand *left, operand *right, operand *result, const iCode *ic)
{
  unsigned int shCount = ulFromVal (right->aop->aopu.aop_lit);
  unsigned int size;

  freeAsmop (right, NULL);

  aopOp (left, ic, false, false);
  aopOp (result, ic, true, false);

  size = getSize (operandType (result));

  /* I suppose that the left size >= result size */
  wassert (getSize (operandType (left)) >= size);

  if (shCount >= (size * 8))
    genMove (result->aop, ASMOP_ZERO, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic) && left->aop->regs[H_IDX] < 0 && left->aop->regs[L_IDX] < 0, isPairDead (PAIR_DE, ic) && left->aop->regs[D_IDX] < 0 && left->aop->regs[E_IDX] < 0, true);
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
  asmop *shiftop;
  int countreg;
  bool shift_by_lit;
  int shiftcount = 0;
  int byteshift = 0;
  bool started;
  bool save_a_outer = false;

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic, FALSE, FALSE);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (right->aop->type == AOP_LIT && getSize (operandType (result)) <= 2)
    {
      genLeftShiftLiteral (left, right, result, ic);
      freeAsmop (right, NULL);
      return;
    }

  /* Useful for the case of shifting a size > 2 value by a literal */
  shift_by_lit = right->aop->type == AOP_LIT;
  if (shift_by_lit)
    shiftcount = ulFromVal (right->aop->aopu.aop_lit);

  aopOp (result, ic, true, false);
  aopOp (left, ic, false, false);

  bool z80n_de = ((result->aop->size == 2 && (aopInReg (result->aop, 0, DE_IDX) || aopInReg (left->aop, 0, DE_IDX)) ||
    result->aop->size == 1 && (aopInReg (result->aop, 0, D_IDX) || aopInReg (left->aop, 0, D_IDX)))) && isRegDead (PAIR_DE, ic);

  if (right->aop->type == AOP_REG && !bitVectBitValue (ic->rSurv, right->aop->aopu.aop_reg[0]->rIdx) && right->aop->aopu.aop_reg[0]->rIdx != IYL_IDX && (sameRegs (left->aop, result->aop) || left->aop->type != AOP_REG) &&
    (result->aop->type != AOP_REG ||
    result->aop->aopu.aop_reg[0]->rIdx != right->aop->aopu.aop_reg[0]->rIdx &&
    !aopInReg (result->aop, 0, right->aop->aopu.aop_reg[0]->rIdx) && !aopInReg (result->aop, 1, right->aop->aopu.aop_reg[0]->rIdx) && !aopInReg (result->aop, 2, right->aop->aopu.aop_reg[0]->rIdx) && !aopInReg (result->aop, 3, right->aop->aopu.aop_reg[0]->rIdx)))
    countreg = right->aop->aopu.aop_reg[0]->rIdx;
  else if (!IS_SM83 && isRegDead (B_IDX, ic) && (sameRegs (left->aop, result->aop) || left->aop->type != AOP_REG || shift_by_lit) &&
    !aopInReg (result->aop, 0, B_IDX) && !aopInReg (result->aop, 1, B_IDX) && !aopInReg (result->aop, 2, B_IDX) && !aopInReg (result->aop, 3, B_IDX))
    countreg = B_IDX;
  else if (isRegDead (A_IDX, ic) && result->aop->regs[A_IDX] < 0 && left->aop->regs[A_IDX] < 0)
    countreg = A_IDX;
  else if (isRegDead (B_IDX, ic) && result->aop->regs[B_IDX] < 0 && left->aop->regs[B_IDX] < 0)
    countreg = B_IDX;
  else if (isRegDead (C_IDX, ic) && result->aop->regs[C_IDX] < 0 && left->aop->regs[C_IDX] < 0)
    countreg = C_IDX;
  else if (IS_Z80N && z80n_de && aopInReg (right->aop, 0, B_IDX))
    countreg = B_IDX;
  else
    {
      UNIMPLEMENTED;
      countreg = A_IDX;
    }

  if (IS_Z80N && z80n_de && (aopInReg (right->aop, 0, B_IDX) || countreg == B_IDX))
    {
      shiftop = result->aop->size == 2 ? ASMOP_DE : ASMOP_E;
      cheapMove (ASMOP_B, 0, right->aop, 0, isRegDead (A_IDX, ic));
      genMove (shiftop, left->aop, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), true, true);
      emit2 ("bsla de, b");
      cost (2, 8);
      goto end;
    }

  save_a_outer = (!isRegDead (A_IDX, ic) && countreg == A_IDX && !(shift_by_lit && shiftcount == 1));
  
  if(save_a_outer)
    _push (PAIR_AF);
    
  if (!shift_by_lit)
    cheapMove (asmopregs[countreg], 0, right->aop, 0, true);

  bool save_a_inner = (countreg == A_IDX && !shift_by_lit) &&
    !(left->aop->type == AOP_REG && result->aop->type != AOP_REG ||
    !IS_SM83 && (left->aop->type == AOP_STK && canAssignToPtr3 (result->aop) || result->aop->type == AOP_STK && canAssignToPtr3 (left->aop)));

  shiftop = result->aop;
  if (result->aop->type != AOP_REG && left->aop->type == AOP_REG && result->aop->size == left->aop->size && left->aop->regs[countreg] < 0)
    {
      bool left_dead = true;
      for (int i = 0; i < left->aop->size; i++)
        left_dead &= isRegDead (left->aop->aopu.aop_reg[i]->rIdx, ic);
      if (left_dead)
        shiftop = left->aop;
    }

  /* now move the left to the result if they are not the
     same */
  if (!sameRegs (shiftop, left->aop) || shiftop->type == AOP_REG)
    {
      if (save_a_inner)
        _push (PAIR_AF);

      if (shift_by_lit)
      {
        byteshift = shiftcount / 8;
        shiftcount %= 8;
      }
      size = shiftop->size - byteshift;
      int lsize = left->aop->size - byteshift;

      bool hl_dead = isPairDead (PAIR_HL, ic) && (countreg != L_IDX && countreg != H_IDX || shift_by_lit);
      bool de_dead = isPairDead (PAIR_DE, ic) && (countreg != E_IDX && countreg != D_IDX || shift_by_lit);
      genMove_o (shiftop, byteshift, left->aop, 0, size <= lsize ? size : lsize, (save_a_inner || countreg != A_IDX) && (isRegDead (A_IDX, ic) || save_a_outer), hl_dead, de_dead, true, true);
      hl_dead &= shiftop->regs[L_IDX] < byteshift && shiftop->regs[L_IDX] < byteshift;
      de_dead &= shiftop->regs[E_IDX] < byteshift && shiftop->regs[D_IDX] < byteshift;
      genMove_o (shiftop, 0, ASMOP_ZERO, 0, byteshift, (save_a_inner || countreg != A_IDX) && (isRegDead (A_IDX, ic) || save_a_outer), hl_dead, de_dead, true, true);

      if (save_a_inner)
        _pop (PAIR_AF);
    }
  shiftop->valinfo.anything = true;

  if (!regalloc_dry_run)
    {
      tlbl = newiTempLabel (NULL);
      tlbl1 = newiTempLabel (NULL);
    }
  size = shiftop->size - byteshift;
  offset = byteshift;

  if (shift_by_lit && !shiftcount)
    goto end;
  if (shift_by_lit && shiftcount > 1)
    {
      emit2 ("ld %s, !immedbyte", countreg == A_IDX ? "a" : regsZ80[countreg].name, (unsigned)shiftcount);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
    }
  else if (!shift_by_lit && !aopIsNotLitVal (right->aop, 0, 1, 0))
    {
      emit2 ("inc %s", countreg == A_IDX ? "a" : regsZ80[countreg].name);
      cost2 (1, 4, 4, 2, 4, 2, 1, 1);
      if (!regalloc_dry_run)
        emit2 ("jp !tlabel", labelKey2num (tlbl1->key));
      cost2 (3, 10, 9, 8, 16, 8, 4, 3);
    }
  if (!(shift_by_lit && shiftcount == 1) && !regalloc_dry_run)
    {
      emitLabel (tlbl);
      if (requiresHL (shiftop))
        spillPair (PAIR_HL);
    }

  started = false;
  regalloc_dry_run_state_scale = shift_by_lit ? shiftcount : 2;
  while (size)
    {
      if (size >= 2 && offset + 1 >= byteshift &&
        shiftop->type == AOP_REG &&
        (!started || !IS_SM83) && // sm83 doesn't have wide adc
        (getPartPairId (shiftop, offset) == PAIR_HL ||
        !started && getPartPairId (shiftop, offset) == PAIR_IY ||
        (IS_RAB || optimize.codeSize && !started && !IS_SM83) && getPartPairId (shiftop, offset) == PAIR_DE))
        {
          if (shiftop->aopu.aop_reg[offset]->rIdx == L_IDX)
            emit3w (started ? A_ADC : A_ADD, ASMOP_HL, ASMOP_HL);
          else if (shiftop->aopu.aop_reg[offset]->rIdx == IYL_IDX)
            emit3w (A_ADD, ASMOP_IY, ASMOP_IY);
          else if (IS_RAB)
            {
              if (!started)
                emit3 (A_CP, ASMOP_A, ASMOP_A);
              emit2 ("rl de");
              cost (1, 2);
            }
          else
            {
              wassert (!IS_SM83);
              emit3w (A_EX, ASMOP_DE, ASMOP_HL);
              emit3w (started ? A_ADC : A_ADD, ASMOP_HL, ASMOP_HL);
              emit3w (A_EX, ASMOP_DE, ASMOP_HL);
            }

          started = true;
          size -= 2, offset += 2;
        }
      else
        {
          if (offset >= byteshift)
            {
              if (aopInReg (shiftop, offset, A_IDX))
                emit3 (started ? A_ADC : A_ADD, ASMOP_A, ASMOP_A);
              else
                emit3_o (started ? A_RL : A_SLA, shiftop, offset, 0, 0);
              started = true;
            }
          size--, offset++;
        }
    }

  if (!(shift_by_lit && shiftcount == 1))
    {
      if (!regalloc_dry_run)
        emitLabel (tlbl1);
      if (!IS_SM83 && countreg == B_IDX)
        {
          if (!regalloc_dry_run)
            emit2 ("djnz !tlabel", labelKey2num (tlbl->key));
          cost2 (2, 13, 9, 5, 0, 10, 4, 2); // Assume jump taken.
        }
      else
        {
          emit2 ("dec %s", regsZ80[countreg].name);
          cost2 (1, 4, 4, 2, 4, 2, 1, 1);
          if (!regalloc_dry_run)
            emit2 ("jr NZ,!tlabel", labelKey2num (tlbl->key));
          cost2 (2, 12, 8, 5, 12, 8, 3, 3); // Assume jump taken, and optimized to jr.
        }
    }

end:
  regalloc_dry_run_state_scale = 1.0f;

  if (!shift_by_lit && requiresHL (shiftop)) // Shift by 0 skips over hl adjustments.
    spillPair (PAIR_HL);

  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);
  if (maskedtopbyte)
    {
      bool pushed_a = false;
      if (!isRegDead (A_IDX, ic) || shiftop->regs[A_IDX] >= 0 && shiftop->regs[A_IDX] != result->aop->size - 1)
        {
          _push (PAIR_AF);
          pushed_a = true;
        }
      cheapMove (ASMOP_A, 0, shiftop, result->aop->size - 1, true);
      emit2 ("and a, #0x%02x", topbytemask);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      cheapMove (shiftop, result->aop->size - 1, ASMOP_A, 0, true);
      if (pushed_a)
        _pop (PAIR_AF);
    }

  genMove_o (result->aop, 0, shiftop, 0, result->aop->size, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);

  if (save_a_outer)
    _pop (PAIR_AF);

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
        emit2 ("and a, !immedbyte", 0xffu >> shCount);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
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
  int size = result->aop->size;

  wassert (size == 1);

  bool a_dead = isRegDead (A_IDX, ic);

  if ((IS_Z180 || IS_EZ80_Z80 || IS_Z80N) && !is_signed && shCount >= 3 && shCount <= 6 + a_dead && // Try to use mlt.
    (!IS_Z80N && aopInReg (result->aop, 0, B_IDX) && isPairDead(PAIR_BC, ic) || aopInReg (result->aop, 0, D_IDX) && isPairDead(PAIR_DE, ic) || !IS_Z80N && aopInReg (result->aop, 0, H_IDX) && isPairDead(PAIR_HL, ic)))
    {
      PAIR_ID pair = aopInReg (result->aop, 0, B_IDX) ? PAIR_BC : (aopInReg (result->aop, 0, D_IDX) ? PAIR_DE : PAIR_HL);
      bool top = aopInReg (left->aop, 0, _pairs[pair].h_idx);
      if (!top)
        cheapMove (pair == PAIR_BC ? ASMOP_C : (pair == PAIR_DE ? ASMOP_E : ASMOP_L), 0, left->aop, 0, a_dead);

      emit2 ("ld %s, !immed%d", top ? _pairs[pair].l : _pairs[pair].h, 1 << (8 - shCount));
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      emit2 ("mlt %s", _pairs[pair].name);
      cost2 (2, 8, 17, 0, 0, 0, 6, 0);
    }
  else if (!IS_SM83 && !IS_RAB && !is_signed && aopSame (result->aop, 0, left->aop, 0, 1) && shCount == 4 && isPairDead (PAIR_HL, ic) && isRegDead (A_IDX, ic) &&
    (result->aop->type == AOP_DIR || result->aop->type == AOP_HL || result->aop->type == AOP_IY))
    {
      emit3 (A_XOR, ASMOP_A, ASMOP_A);
      pointPairToAop (PAIR_HL, result->aop, 0);
      emit3 (A_RRD, 0, 0);
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
  else if (result->aop->type == AOP_REG) // Can shift in destination for register result.
    {
      cheapMove (result->aop, 0, left->aop, 0, a_dead);

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
genrshTwo (const iCode *ic, operand *result, operand *left, int shCount, int sign)
{
  if (IS_Z80N &&
    aopInReg (result->aop, 0, DE_IDX) && isRegDead (B_IDX, ic) &&
    shCount != 8)
    {
      genMove (ASMOP_DE, left->aop, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), true, true);
      emit2 ("ld b, !immedbyte", (unsigned)shCount);
      emit2 (sign ? "bsra de, b" : "bsrl de, b");
      cost (4, 15);
    }
  else if (shCount >= 8)
    {
      shCount -= 8;
      if (shCount)
        shiftR1Left2Result (left, MSB16, result, LSB, shCount, sign);
      else
        cheapMove (result->aop, 0, left->aop, 1, isRegDead (A_IDX, ic));
      if (sign)
        {
          /* Sign extend the result */
          if (result->aop->type != AOP_REG && left->aop->type == AOP_REG)
            cheapMove (ASMOP_A, 0, left->aop, 1, true);
          else
            cheapMove (ASMOP_A, 0, result->aop, 0, true);
          emit3 (A_RLCA, 0, 0);
          emit3 (A_SBC, ASMOP_A, ASMOP_A);
          cheapMove (result->aop, 1, ASMOP_A, 0, true);
        }
      else
        cheapMove (result->aop, 1, ASMOP_ZERO, 0, true);
    }
  else
    shiftR2Left2Result (ic, left, LSB, result, LSB, shCount, sign);
}

/*-----------------------------------------------------------------*/
/* genRightShiftLiteral - right shifting by known count              */
/*-----------------------------------------------------------------*/
static void
genRightShiftLiteral (operand *left, operand *right, operand *result, const iCode *ic, int sign)
{
  unsigned int shCount = (unsigned int) ulFromVal (right->aop->aopu.aop_lit);
  unsigned int size;

  freeAsmop (right, NULL);

  aopOp (left, ic, false, false);
  aopOp (result, ic, true, false);

  size = getSize (operandType (result));

  /* I suppose that the left size >= result size */
  wassert (getSize (operandType (left)) >= size);

  if (shCount >= (size * 8))
    {
      if (!SPEC_USIGN (getSpec (operandType (left))))
        {
          cheapMove (ASMOP_A, 0, left->aop, 0, true);
          emit3 (A_RLCA, 0, 0);
          emit3 (A_SBC, ASMOP_A, ASMOP_A);
          while (size--)
            cheapMove (result->aop, size, ASMOP_A, 0, true);
        }
      else
        genMove (result->aop, ASMOP_ZERO, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), false, true);
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
  asmop *shiftop;
  int size, offset, first = 1;
  bool is_signed;
  int countreg;
  bool shift_by_lit, shift_by_one, shift_by_zero;
  int shiftcount = 0;
  int byteoffset = 0;
  bool save_a;

  symbol *tlbl = 0, *tlbl1 = 0;

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  /* if signed then we do it the hard way preserve the
     sign bit moving it inwards */
  is_signed = !SPEC_USIGN (getSpec (operandType (left)));

  aopOp (right, ic, FALSE, FALSE);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (right->aop->type == AOP_LIT && getSize (operandType (result)) <= 2)
    {
      genRightShiftLiteral (left, right, result, ic, is_signed);
      freeAsmop (right, NULL);
      return;
    }

  /* Useful for the case of shifting a size > 2 value by a literal */
  shift_by_lit = right->aop->type == AOP_LIT;
  if (shift_by_lit)
    shiftcount = ulFromVal (right->aop->aopu.aop_lit);

  aopOp (result, ic, true, false);
  aopOp (left, ic, false, false);
    
  if (right->aop->type == AOP_REG && !bitVectBitValue (ic->rSurv, right->aop->aopu.aop_reg[0]->rIdx) && right->aop->aopu.aop_reg[0]->rIdx != IYL_IDX && (sameRegs (left->aop, result->aop) || left->aop->type != AOP_REG) &&
    (result->aop->type != AOP_REG ||
    result->aop->aopu.aop_reg[0]->rIdx != right->aop->aopu.aop_reg[0]->rIdx &&
    (result->aop->size < 2 || result->aop->aopu.aop_reg[1]->rIdx != right->aop->aopu.aop_reg[0]->rIdx &&
    (result->aop->size < 3 || result->aop->aopu.aop_reg[2]->rIdx != right->aop->aopu.aop_reg[0]->rIdx &&
    (result->aop->size < 4 || result->aop->aopu.aop_reg[3]->rIdx != right->aop->aopu.aop_reg[0]->rIdx)))))
    countreg = right->aop->aopu.aop_reg[0]->rIdx;
  else if (!IS_SM83 && isRegDead (B_IDX, ic) && (sameRegs (left->aop, result->aop) || left->aop->type != AOP_REG || shift_by_lit) &&
    (result->aop->type != AOP_REG ||
    result->aop->aopu.aop_reg[0]->rIdx != B_IDX &&
    (result->aop->size < 2 || result->aop->aopu.aop_reg[1]->rIdx != B_IDX &&
    (result->aop->size < 3 || result->aop->aopu.aop_reg[2]->rIdx != B_IDX &&
    (result->aop->size < 4 || result->aop->aopu.aop_reg[3]->rIdx != B_IDX)))))
    countreg = B_IDX;
  else
    countreg = A_IDX;

  if (IS_Z80N && isRegDead (PAIR_DE, ic) &&
    (result->aop->size == 2 && (aopInReg (result->aop, 0, DE_IDX) || aopInReg (left->aop, 0, DE_IDX)) ||
      result->aop->size == 1 && (aopInReg (result->aop, 0, D_IDX) || aopInReg (left->aop, 0, D_IDX))) &&
    (aopInReg (right->aop, 0, B_IDX) || countreg == B_IDX))
    {
      shiftop = result->aop->size == 2 ? ASMOP_DE : ASMOP_D;
      cheapMove (ASMOP_B, 0, right->aop, 0, isRegDead (A_IDX, ic));
      genMove (shiftop, left->aop, isRegDead (A_IDX, ic), isRegDead (HL_IDX, ic), true, true);
      emit2 (is_signed ? "bsra de, b" : "bsrl de, b");
      cost (2, 8);
      goto end;
    }

  if (!shift_by_lit)
    cheapMove (countreg == A_IDX ? ASMOP_A : asmopregs[countreg], 0, right->aop, 0, true);

  save_a = (countreg == A_IDX && !shift_by_lit) &&
    !(left->aop->type == AOP_REG && result->aop->type != AOP_REG ||
    !IS_SM83 && (left->aop->type == AOP_STK && canAssignToPtr3 (result->aop) || result->aop->type == AOP_STK && canAssignToPtr3 (left->aop)));

  shiftop = result->aop;
  if (result->aop->type != AOP_REG && left->aop->type == AOP_REG && result->aop->size == left->aop->size && left->aop->regs[countreg] < 0)
    {
      bool left_dead = true;
      for (int i = 0; i < left->aop->size; i++)
        left_dead &= isRegDead (left->aop->aopu.aop_reg[i]->rIdx, ic);
      if (left_dead)
        shiftop = left->aop;
    }

  /* now move the left to the shiftop if they are not the
     same */
  if (!sameRegs (shiftop, left->aop) || shiftop->type == AOP_REG)
    {
      int soffset = 0;
      size = shiftop->size;

      if (!is_signed && shift_by_lit)
      {
        byteoffset = shiftcount / 8;
        shiftcount %= 8;
        soffset = byteoffset;
        size -= byteoffset;
      }

      if (save_a)
        _push (PAIR_AF);

      bool hl_dead = isPairDead (PAIR_HL, ic) && (countreg != L_IDX && countreg != H_IDX || shift_by_lit);
      genMove_o (shiftop, 0, left->aop, soffset, size, true, hl_dead, false, true, true);
      hl_dead &= (shiftop->regs[L_IDX] < 0 || shiftop->regs[L_IDX] >= size) && (shiftop->regs[H_IDX] < 0 || shiftop->regs[H_IDX] >= size);
      genMove_o (shiftop, shiftop->size - byteoffset, ASMOP_ZERO, 0, byteoffset, true, hl_dead, false, true, true);

      if (save_a)
        _pop (PAIR_AF);
    }
  shiftop->valinfo.anything = true;

  shift_by_one = (shift_by_lit && shiftcount == 1);
  shift_by_zero = (shift_by_lit && shiftcount == 0);

  if (!regalloc_dry_run)
    {
      tlbl = newiTempLabel (NULL);
      tlbl1 = newiTempLabel (NULL);
    }
  size = result->aop->size;
  offset = size - 1;

  if (shift_by_zero)
    goto end;
  else if (shift_by_lit && shiftcount > 1)
    {
      emit2 ("ld %s, !immedbyte", countreg == A_IDX ? "a" : regsZ80[countreg].name, (unsigned)shiftcount);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
    }
  else if (!shift_by_lit && !aopIsNotLitVal (right->aop, 0, 1, 0))
    {
      emit2 ("inc %s", countreg == A_IDX ? "a" : regsZ80[countreg].name);
      cost2 (1, 4, 4, 2, 4, 2, 1, 1);
      if (!regalloc_dry_run)
        emit2 ("jp !tlabel", labelKey2num (tlbl1->key));
      cost2 (3, 10, 9, 8, 16, 8, 4, 3);
    }
  if (!shift_by_one && !regalloc_dry_run)
    IS_SM83 ? emitLabelSpill (tlbl) : emitLabel (tlbl);

  if (!shift_by_one && requiresHL (shiftop))
    spillPair (PAIR_HL);
    
  regalloc_dry_run_state_scale = shift_by_lit ? shiftcount : 2;
  while (size)
    {
      if (IS_RAB && !(is_signed && first) && size >= 2 && byteoffset < 2 && shiftop->type == AOP_REG &&
        (getPartPairId (shiftop, offset - 1) == PAIR_HL || getPartPairId (shiftop, offset - 1) == PAIR_DE))
        {
          if (first)
            {
              emit3 (A_CP, ASMOP_A, ASMOP_A);
              first = 0;
            }
          emit2 (shiftop->aopu.aop_reg[offset - 1]->rIdx == L_IDX ? "rr hl" : "rr de");
          cost (1, 2);
          size -= 2, offset -= 2;
        }
      else if (!is_signed && first && byteoffset--) // Skip known 0 bytes
        size--, offset--;
      else if (first)
        {
          emit3_o (is_signed ? A_SRA : A_SRL, shiftop, offset, 0, 0);
          first = 0;
          size--, offset--;
        }
      else
        {
          emit3_o (A_RR, shiftop, offset, 0, 0);
          size--, offset--;
        }
    }

  if (!shift_by_one)
    {
      if (!regalloc_dry_run)
        emitLabel (tlbl1);
      if (!IS_SM83 && countreg == B_IDX)
        {
          if (!regalloc_dry_run)
            emit2 ("djnz !tlabel", labelKey2num (tlbl->key));
          cost2 (2, 13, 9, 5, 0, 10, 4, 2); // Assume jump taken.
        }
      else
        {
          emit2 ("dec %s", countreg == A_IDX ? "a" : regsZ80[countreg].name);
          cost2 (1, 4, 4, 2, 4, 2, 1, 1);
          if (!regalloc_dry_run)
            emit2 ("jr NZ, !tlabel", labelKey2num (tlbl->key));
          cost2 (2, 12, 8, 5, 12, 8, 3, 3); // Assume jump taken, and optimized to jr.
        }
    }

end:
  regalloc_dry_run_state_scale = 1.0f;
  if (!shift_by_lit && requiresHL (shiftop)) // Shift by 0 skips over hl adjustments.
    spillPair (PAIR_HL);

  genMove_o (result->aop, 0, shiftop, 0, result->aop->size, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true, true);

  freeAsmop (left, NULL);
  freeAsmop (right, NULL);
  freeAsmop (result, NULL);
}

/*-----------------------------------------------------------------*/
/* unpackMaskA - generate masking code for unpacking last byte     */
/* of bitfield. And mask for unsigned, sign extension for signed.  */
/*-----------------------------------------------------------------*/
static void
unpackMaskA(sym_link *type, int len)
{
  if (SPEC_USIGN (type) || len != 1)
    {
      emit2 ("and a, !immedbyte", ((unsigned)-1 & 0xffu) >> (8 - len));
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
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
          emit2 ("bit %d, a", len - 1);
          cost2 (2, 8, 6, 4, 8, 4, 2, 2);
          if (!regalloc_dry_run)
            {
              symbol *tlbl = newiTempLabel (NULL);
              emit2 ("jp Z, !tlabel", labelKey2num (tlbl->key));
              emit2 ("or a, !immedbyte", ((0xffu << len) & 0xffu));
              emitLabel (tlbl);
            }
          cost2 (4, 12, 8, 5, 12, 8, 3, 3); // Assume nonnegative, jp optimzed to jr.
        }
    }
}

/*-----------------------------------------------------------------*/
/* genUnpackBits - generates code for unpacking bits               */
/*-----------------------------------------------------------------*/
static void
genUnpackBits (operand *result, int pair, const iCode *ic)
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
      emit2 ("ld a, !mems", _pairs[pair].name);
      regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;
      AccRol (8 - bstr);
      unpackMaskA (etype, blen);
      cheapMove (result->aop, offset++, ASMOP_A, 0, true);
      goto finish;
    }

  /* TODO: what if pair == PAIR_DE ? */
  if (getPairId (result->aop) == PAIR_HL ||
      result->aop->type == AOP_REG && rsize == 2 && (result->aop->aopu.aop_reg[0]->rIdx == L_IDX
          || result->aop->aopu.aop_reg[0]->rIdx == H_IDX))
    {
      emit2 ("!ldahli");
      regalloc_dry_run_cost++;
      if (result->aop->type != AOP_REG || result->aop->aopu.aop_reg[0]->rIdx != H_IDX)
        {
          emit2 ("ld h, !*hl");
          cost2 (1, 7, 6, 5, 8, 6, 2, 2);
          cheapMove (result->aop, offset++, ASMOP_A, 0, true);
          emit3 (A_LD, ASMOP_A, ASMOP_H);
        }
      else
        {
          emit2 ("ld l, !*hl");
          cost2 (1, 7, 6, 5, 8, 6, 2, 2);
          cheapMove (result->aop, offset++, ASMOP_A, 0, true);
          emit3 (A_LD, ASMOP_A, ASMOP_L);
        }
      unpackMaskA (etype, blen - 8);
      cheapMove (result->aop, offset++, ASMOP_A, 0, true);
      spillPair (PAIR_HL);
      return;
    }

  /* Bit field did not fit in a byte. Copy all
     but the partial byte at the end.  */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      emit2 ("ld a, !mems", _pairs[pair].name);
      cost2 (1, 7, 6, 6, 8, 6, 2, 2);
      cheapMove (result->aop, offset++, ASMOP_A, 0, true);
      if (rlen > 8)
        {
          emit2 ("inc %s", _pairs[pair].name);
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          _G.pairs[pair].offset++;
          pairincrement++;
        }
    }

  /* Handle the partial byte at the end */
  if (rlen)
    {
      emit2 ("ld a, !mems", _pairs[pair].name);
      cost2 (1, 7, 6, 6, 8, 6, 2, 2);
      unpackMaskA (etype, rlen);
      cheapMove (result->aop, offset++, ASMOP_A, 0, true);
    }

finish:
  if (!isPairDead (pair, ic))
    while (pairincrement)
      {
        emit2 ("dec %s", _pairs[pair].name);
        cost2 (1, 6, 4, 2, 8, 4, 1, 1);
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
        cheapMove (result->aop, offset++, source, 0, true);
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
      ld_cost (aop, 0, aopInReg (aop, 0, A_IDX) ? ASMOP_L : ASMOP_A, 0, true);
    }
  else
    {
      emit2 ("ld a, !mems", _pairs[pair].name);
      cost2 (1, 7, 6, 6, 8, 6, 2, 2);
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
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          if (val > 0)
            val--;
          else
            val++;
        }
    }
  else
    {
      if (save_extrapair)
        _push (extrapair);
      emit2 ("ld %s, !immedword", _pairs[extrapair].name, (unsigned)val);
      if (extrapair == PAIR_IY)
        cost2 (4 - IS_TLCS90, 14, 12, 8, 0, 6, 4, 4);
      else
       	cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("add %s, %s", _pairs[pair].name, _pairs[extrapair].name);
      if (pair == PAIR_HL)
        cost2 (1, 11, 7, 2, 8, 8, 1, 1);
      else
        cost2 (2, 15, 10, 4, 0, 8, 2, 2);
      if (save_extrapair)
        _pop (extrapair);
    }
}

/*------------------------------------------------------------------*/
/* init_stackop - initialize asmop for stack location                */
/*------------------------------------------------------------------*/
static void 
init_stackop (asmop *stackop, int size, long int stk_off)
{
  stackop->size = size;
  memset (stackop->regs, -1, 9);
  stackop->aopu.aop_stk = stk_off;

  if (!IS_SM83 && (_G.omitFramePtr || stk_off < INT8MIN || stk_off > (int) (INT8MAX - size)))
    stackop->type = AOP_EXSTK;
  else
    stackop->type = AOP_STK;

  stackop->valinfo.anything = true;
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
  bool pushed_pair = FALSE;
  bool pushed_a = FALSE;
  bool surviving_a = !isRegDead (A_IDX, ic);
  bool rightval_in_range;

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  bool bit_field = IS_BITVAR (operandType (result)); // Should be IS_BITVAR (operandType (left)->next), but conflicts with optimizations that reuses pointers (when reading from a union of a struct containing bit-fields and other types).

  aopOp (left, ic, false, false);
  aopOp (result, ic, true, false);
  size = result->aop->size;

  /* Historically GET_VALUE_AT_ADDRESS didn't have a right operand */
  wassertl (right, "GET_VALUE_AT_ADDRESS without right operand");
  wassertl (IS_OP_LITERAL (IC_RIGHT (ic)), "GET_VALUE_AT_ADDRESS with non-literal right operand");
  rightval = (int)operandLitValue (right);
  rightval_in_range = (rightval >= -128 && rightval + size - 1 < 127);

  if (IS_SM83 && left->aop->type == AOP_STK) // Try to avoid (hl) to hl copy, which requires 3 instructions and free a.
    pair = PAIR_DE;
  if ((IS_SM83 || IY_RESERVED) && requiresHL (result->aop) && size > 1 && result->aop->type != AOP_REG)
    pair = PAIR_DE;

  if (IS_SM83 && size == 1 && left->aop->type == AOP_LIT && (((unsigned long)(operandLitValue (left) + rightval) & 0xff00) == 0xff00) && isRegDead (A_IDX, ic) && !bit_field) // SM83 has special instructions for address range 0xff00 - 0xffff.
    {
      emit2 ("ldh a, !mems", aopGetLitWordLong (left->aop, rightval, true));
      cost (2, 12);
      cheapMove (result->aop, 0, ASMOP_A, 0, true);
      goto release;
    }
  if ((left->aop->type == AOP_IMMD || left->aop->type == AOP_LIT && !rightval) && size == 1 && aopInReg (result->aop, 0, A_IDX) && !bit_field)
    {
      emit2 ("ld a, !mems", aopGetLitWordLong (left->aop, rightval, true));
      cost2 (3, 13, 12, 9, 16, 10, 4, 4);
      goto release;
    }
  else if (!IS_SM83 && (left->aop->type == AOP_IMMD || left->aop->type == AOP_LIT && !rightval) && isPair (result->aop) && !bit_field)
    {
      PAIR_ID pair = getPairId (result->aop);
      emit2 ("ld %s, !mems", _pairs[pair].name, aopGetLitWordLong (left->aop, rightval, TRUE));
      if (pair == PAIR_HL)
        cost2 (3, 16 , 15, 11, 0, 12, 5, 5);
      else
        cost2 (4, 20, 18, 13, 0, 12, 6, 6);
      goto release;
    }
  else if (!IS_SM83 && (left->aop->type == AOP_IMMD) && getPartPairId (result->aop, 0) != PAIR_INVALID && getPartPairId (result->aop, 2) != PAIR_INVALID)
    {
      PAIR_ID pair;
      pair = getPartPairId (result->aop, 0);
      emit2 ("ld %s, !mems", _pairs[pair].name, aopGetLitWordLong (left->aop, rightval, TRUE));
      if (pair == PAIR_HL)
        cost2 (3, 16 , 15, 11, 0, 12, 5, 5);
      else
        cost2 (4, 20, 18, 13, 0, 12, 6, 6);
      pair = getPartPairId (result->aop, 2);
      emit2 ("ld %s, !mems", _pairs[pair].name, aopGetLitWordLong (left->aop, rightval + 2, TRUE));
      if (pair == PAIR_HL)
        cost2 (3, 16 , 15, 11, 0, 12, 5, 5);
      else
        cost2 (4, 20, 18, 13, 0, 12, 6, 6);
      goto release;
    }
  else if (left->aop->type == AOP_STL && !bit_field && size <= 4)
    {
      struct asmop saop;
      init_stackop (&saop, size, left->aop->aopu.aop_stk + rightval);
      genMove (result->aop, &saop, !surviving_a, isPairDead(PAIR_HL, ic), isPairDead(PAIR_DE, ic), isPairDead(PAIR_IY, ic));
      goto release;
    }

  if (IS_SM83)
    wassert (!rightval);

  if (isPair (left->aop) && size == 1 && !bit_field && !rightval)
    {
      /* Just do it */
      if ((getPairId (left->aop) == PAIR_HL || getPairId (left->aop) == PAIR_IY) && result->aop->type == AOP_REG)
        {
          if (!regalloc_dry_run)        // Todo: More exact cost.
            {
              struct dbuf_s dbuf;

              dbuf_init (&dbuf, 128);
              dbuf_tprintf (&dbuf, "!mems", getPairName (left->aop));
              aopPut (result->aop, dbuf_c_str (&dbuf), 0);
              dbuf_destroy (&dbuf);
            }
          ld_cost (result->aop, 0, aopInReg(result->aop, 0, A_IDX) ? ASMOP_L : ASMOP_A, 0, true);
        }
      else
        {
          if (surviving_a && !pushed_a)
            _push (PAIR_AF), pushed_a = true;
          emit2 ("ld a, !mems", getPairName (left->aop));
          if (getPairId (left->aop) == PAIR_IY)
            cost2 (3, 19, 14, 9, 0, 10, 4, 5);
          else
            cost2 (1, 7, 6, 6, 8, 6, 2, 2);
          genMove (result->aop, ASMOP_A, true, isPairDead(PAIR_HL, ic), isPairDead(PAIR_DE, ic), true);
        }

      goto release;
    }

  if (getPairId (left->aop) == PAIR_IY && !bit_field && rightval_in_range)
    {
      offset = 0;

      if (IS_RAB && getPartPairId (result->aop, 0) == PAIR_HL)
        {
          emit2 ("ld hl, %d (iy)", rightval);
          cost (2, 9);
          cost (2, 9);
          offset = 2;
          size -= 2;
        }
      else if ((IS_EZ80_Z80 || IS_TLCS90) && getPartPairId (result->aop, 0) != PAIR_INVALID)
        {
          emit2 ("ld %s, %d (iy)", _pairs[getPartPairId (result->aop, 0)].name, rightval);
          cost2 (3, 0, 0, 0, 0, 12, 5, 0);
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
              aopPut (result->aop, dbuf_c_str (&dbuf), offset);
              dbuf_destroy (&dbuf);
            }
          cost2 (3, 19, 14, 9, 0, 10, 4, 5); // Assume ld r, d(iy)
          offset++;
        }

      goto release;
    }

  // Using ldir is cheapest for large memory-to-memory transfers.
  // sm83 doesn't have ldir. Rabbit 2000 to Rabbit 3000 (i.e. r2k and r2ka ports) have a wait-state bug breaking ldir between different types of memory.
  if (!IS_SM83 && (!IS_R2K && !IS_R2KA || left->aop->type == AOP_STL) && !bit_field && (result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK) && size > 2)
    {
      int fp_offset, sp_offset;

      if(!isPairDead (PAIR_HL, ic))
        _push (PAIR_HL);
      if(!isPairDead (PAIR_DE, ic))
        _push (PAIR_DE);
      if(!isPairDead (PAIR_BC, ic))
        _push (PAIR_BC);

      fp_offset = result->aop->aopu.aop_stk + (result->aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;

      if (IS_EZ80_Z80 && !_G.omitFramePtr && fp_offset >= -128 && fp_offset < 128)
        {
          if (left->aop->type == AOP_IMMD)
            {
              emit2 ("ld hl, %s", aopGetLitWordLong (left->aop, rightval, TRUE));
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
            }
          else
            fetchPair (PAIR_HL, left->aop);
          emit2 ("lea de, ix, !immed%d", fp_offset);
          cost (3, 3);
        }
      else
        {
          if (left->aop->type == AOP_IMMD)
            {
              emit2 ("ld de, %s", aopGetLitWordLong (left->aop, rightval, TRUE));
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
            }
          else
            genMove (ASMOP_DE, left->aop, isRegDead (A_IDX, ic), true, true, true);
          emit2 ("!ldahlsp", sp_offset);
          regalloc_dry_run_cost += 4;
          emit3w (A_EX, ASMOP_DE, ASMOP_HL);
        }

      if (rightval && left->aop->type != AOP_IMMD)
          if (abs(rightval) < 4)
            {
              for(;rightval > 0; rightval--)
                emit3w (A_INC, ASMOP_HL, 0);
              for(;rightval < 0; rightval++)
                emit3w (A_DEC, ASMOP_HL, 0);
             }
          else
            {
              emit2 ("ld bc, !immedword", (unsigned)rightval);
              cost2 (3, 10, 9, 6, 12, 6, 3, 3);
              emit3w (A_ADD, ASMOP_HL, ASMOP_BC);
              rightval = 0;
            }

      emit2 ("ld bc, !immedword", (unsigned)size);
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("ldir");
      cost2 (2, 21 * size - 5, 14 * size - 2, 7 * size - 1, 0, 18 * size - 4, 2 * size - 1, 4 * size);
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

  if (!surviving_a && (getPairId (left->aop) == PAIR_BC || getPairId (left->aop) == PAIR_DE) && isPairDead (getPairId (left->aop), ic) && abs(rightval) <= 2 && !bit_field && size < 2) // Use inc ss (size < 2 condition to avoid overwriting pair with result)
    pair = getPairId (left->aop);

  /* For now we always load into temp pair */
  /* if this is rematerializable */
  if (!IS_SM83 && (getPairId (left->aop) == PAIR_BC || getPairId (left->aop) == PAIR_DE) && result->aop->type == AOP_STK && !rightval
      || getPairId (left->aop) == PAIR_IY && SPEC_BLEN (getSpec (operandType (result))) < 8 && rightval_in_range)
    pair = getPairId (left->aop);
  else
    {
      if (!isPairDead (pair, ic) && size > 1 && (getPairId (left->aop) != pair || rightval || bit_field || size > 2)) // For simple cases, restoring via dec is cheaper than push / pop.
        _push (pair), pushed_pair = TRUE;
      if (left->aop->type == AOP_IMMD)
        {
          emit2 ("ld %s, %s", _pairs[pair].name, aopGetLitWordLong (left->aop, rightval, TRUE));
          regalloc_dry_run_cost += 3;
          spillPair (pair);
          rightval = 0;
        }
      else if (pair == PAIR_HL && rightval > 2 && (getPairId (left->aop) == PAIR_BC || getPairId (left->aop) == PAIR_DE)) // Cheaper than moving to hl followed by offset adjustment.
        {
          emit2 ("ld hl, !immed%d", rightval);
          cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          emit2 ("add hl, %s", _pairs[getPairId (left->aop)].name);
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          spillPair (pair);
          rightval = 0;
        }
      else if (pair == PAIR_HL && left->aop->type == AOP_STL)
        {
          emit2 ("ld hl, !immed%d", spOffset (left->aop->aopu.aop_stk) + rightval);
          cost2 (3, 10, 9, 6, 12, 6, 3, 3);
          emit2 ("add hl, sp");
          cost2 (1 + IS_TLCS90, 11, 7, 2, 8, 8, 1, 1);
          spillPair (pair);
          rightval = 0;
        }
      else
        fetchPairLong (pair, left->aop, ic, 0);
    }

  /* if bit then unpack */
  if (bit_field)
    {
      offsetPair (pair, extrapair, !isPairDead (extrapair, ic), rightval);
      genUnpackBits (result, pair, ic);
      if (rightval)
        spillPair (pair);

      goto release;
    }

 if (isPair (result->aop) && IS_EZ80_Z80 && getPairId (left->aop) == PAIR_HL && !bit_field && !rightval)
   {
     emit2 ("ld %s, !*hl", _pairs[getPairId (result->aop)].name);
     cost (2, 4);
     goto release;
   }
 else if (pair == PAIR_HL && (getPairId (result->aop) == PAIR_HL || size == 2 && (aopInReg (result->aop, 0, L_IDX) || aopInReg (result->aop, 0, H_IDX))))
    {
      wassertl (size == 2, "HL must be of size 2");
      if (IS_RAB && getPairId (result->aop) == PAIR_HL && rightval_in_range)
        {
          emit2 ("ld hl, %d !*hl", rightval);
          cost (3, 11);
        }
      else if (IS_EZ80_Z80 && getPairId (result->aop) == PAIR_HL && !rightval)
        {
          emit2 ("ld hl, !*hl");
          cost (2, 4);
        }
      else if (aopInReg (result->aop, 1, A_IDX))
        {
          offsetPair (pair, extrapair, !isPairDead (extrapair, ic), rightval + 1);
          emit2 ("!ldahld");
          if (!regalloc_dry_run)
            aopPut (result->aop, "!*hl", 0);
          regalloc_dry_run_cost += 3;
        }
      else
        {
          if (surviving_a && !pushed_a)
            _push (PAIR_AF), pushed_a = TRUE;
          offsetPair (pair, extrapair, !isPairDead (extrapair, ic), rightval);
          emit2 ("!ldahli");
          if (!regalloc_dry_run)
            aopPut (result->aop, "!*hl", 1);
          regalloc_dry_run_cost += 3;
          cheapMove (result->aop, 0, ASMOP_A, 0, true);
        }
      spillPair (PAIR_HL);
      goto release;
    }

  offsetPair (pair, extrapair, !isPairDead (extrapair, ic), rightval);

  if (pair == PAIR_HL
           || (!IS_SM83 && (getPairId (left->aop) == PAIR_BC || getPairId (left->aop) == PAIR_DE)
               && result->aop->type == AOP_STK))
    {
      size = result->aop->size;
      offset = 0;
      int last_offset = 0;

      /* might use ld a,(hl) followed by ld d (iy),a */
      if ((result->aop->type == AOP_EXSTK || result->aop->type == AOP_STK) && surviving_a && !pushed_a)
        _push (PAIR_AF), pushed_a = TRUE;

      if (size >= 2 && pair == PAIR_HL && result->aop->type == AOP_REG)
        {
          int i, l = -10, h = -10, r;
          for (i = 0; i < size; i++)
            {
              if (result->aop->aopu.aop_reg[i]->rIdx == L_IDX)
                l = i;
              else if (result->aop->aopu.aop_reg[i]->rIdx == H_IDX)
                h = i;
            }

          if (l == -10 && h >= 0 && h < size - 1 || h == -10 && l >= 0 && l < size - 1) // One byte of result somewehere in hl. Just assign it last.
            {
              r = (l == -10 ? h : l);

              while (offset < size)
                {
                  if (offset != r)
                    _moveFrom_tpair_ (result->aop, offset, pair);

                  if (offset < size)
                    {
                      offset++;
                      emit2 ("inc %s", _pairs[pair].name);
                      cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                      _G.pairs[pair].offset++;
                    }
                }

              for (size = offset; size != r; size--)
                {
                  emit2 ("dec %s", _pairs[pair].name);
                  cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                }

              _moveFrom_tpair_ (result->aop, r, pair);

              // No fixup since result uses HL.
              spillPair (pair);
              goto release;
            }
          else if (l >= 0 && h >= 0)    // Two bytes of result somewehere in hl. Assign them last and use a for caching.
            {
              while (offset < size)
                {
                  last_offset = offset;

                  if (IS_EZ80_Z80 && offset != l && offset != h && getPairId_o (result->aop, offset) != PAIR_INVALID)
                    {
                      emit2 ("ld %s, !*hl", _pairs[getPairId_o (result->aop, offset)].name);
                      cost (2, 4);
                      offset += 2;
                      if (offset < size)
                        {
                          emit2 ("inc %s", _pairs[pair].name);
                          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                          emit2 ("inc %s", _pairs[pair].name);
                          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                          _G.pairs[pair].offset += 2;
                        }
                      continue;
                    }

                  if (offset != l && offset != h)
                    _moveFrom_tpair_ (result->aop, offset, pair);
                  offset++;

                  if (offset < size)
                    {
                      emit2 ("inc %s", _pairs[pair].name);
                      cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                      _G.pairs[pair].offset++;
                    }
                }

              r = (l > h ? l : h);
              for (size = last_offset; size != r; size--)
                {
                  emit2 ("dec %s", _pairs[pair].name);
                  cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                }
              if ((surviving_a || result->aop->regs[A_IDX] >= 0) && !pushed_a)
                _push (PAIR_AF), pushed_a = true;
              _moveFrom_tpair_ (ASMOP_A, 0, pair);

              r = (l < h ? l : h);
              for (; size != r; size--)
                {
                  emit2 ("dec %s", _pairs[pair].name);
                  cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                }
              _moveFrom_tpair_ (result->aop, r, pair);

              r = (l > h ? l : h);
              cheapMove (result->aop, r, ASMOP_A, 0, true);

              // No fixup since result uses HL.
              spillPair (pair);
              goto release;
            }
        }

      while (offset < size)
        {
          if (result->aop->regs[A_IDX] >= 0 && result->aop->regs[A_IDX] < offset)
            surviving_a = true;

          last_offset = offset;

          if (IS_EZ80_Z80 && getPairId_o (result->aop, offset) != PAIR_INVALID)
            {
              emit2 ("ld %s, !*hl", _pairs[getPairId_o (result->aop, offset)].name);
              cost (2, 4);
              offset += 2;
              if (offset < size)
                {
                  emit2 ("inc %s", _pairs[pair].name);
                  cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                  emit2 ("inc %s", _pairs[pair].name);
                  cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                  _G.pairs[pair].offset += 2;
                }
              continue;
            }

          // _moveFrom_tpair_ below might use a.
          if (result->aop->type != AOP_REG && surviving_a && !pushed_a)
            _push (PAIR_AF), pushed_a = true;
          _moveFrom_tpair_ (result->aop, offset++, pair);

          if (offset < size)
            {
              emit2 ("inc %s", _pairs[pair].name);
              cost2 (1, 6, 4, 2, 8, 4, 1, 1);
              _G.pairs[pair].offset++;
            }
        }
      /* Fixup HL back down */
      if (getPairId (left->aop) == pair && !isPairDead (pair, ic) && !pushed_pair)
        while (last_offset --> 0)
          {
            emit2 ("dec %s", _pairs[pair].name);
            cost2 (1, 6, 4, 2, 8, 4, 1, 1);
            _G.pairs[pair].offset--;
          }
       else if (rightval || result->aop->size)
         spillPair (pair);
    }
  else
    {
      size = result->aop->size;
      offset = 0;

      if (result->aop->regs[A_IDX] >= 0 && result->aop->regs[A_IDX] < offset)
        surviving_a = true;

      for (offset = 0; offset < size;)
        {
          if (surviving_a && !pushed_a)
            _push (PAIR_AF), pushed_a = true;

          /* PENDING: make this better */
          if ((pair == PAIR_HL) && result->aop->type == AOP_REG)
            {
              if (!regalloc_dry_run)
                aopPut (result->aop, "!*hl", offset++);
              ld_cost (result->aop, 0, aopInReg (result->aop, 0, A_IDX) ? ASMOP_L : ASMOP_A, 0, true);
            }
          else
            {
              emit2 ("ld a, !mems", _pairs[pair].name);
              cost2 (1, 7, 6, 6, 8, 6, 2, 2);
              cheapMove (result->aop, offset++, ASMOP_A, 0, true);
            }
          if (offset < size)
            {
              emit2 ("inc %s", _pairs[pair].name);
              cost2 (1, 6, 4, 2, 8, 4, 1, 1);
              _G.pairs[pair].offset++;
            }
        }
      if (!isPairDead (pair, ic))
        while (offset --> 1)
          {
            emit2 ("dec %s", _pairs[pair].name);
            cost2 (1, 6, 4, 2, 8, 4, 1, 1);
            _G.pairs[pair].offset--;
          }
      else if (rightval || result->aop->size)
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
  unsigned long long litval;    /* source literal value (if AOP_LIT) */
  unsigned mask;                /* bitmask within current byte */
  int extraPair;                /* a tempory register */
  bool needPopExtra = 0;        /* need to restore original value of temp reg */
  unsigned int pairincrement = 0;

  emitDebug ("; genPackBits", "");

  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  /* If the bit-field length is less than a byte */
  if (blen < 8)
    {
      mask = ((0xffu << (blen + bstr)) | (0xffu >> (8 - bstr))) & 0xffu;

      if (right->aop->type == AOP_LIT && blen == 1 && (pair == PAIR_HL || pair == PAIR_IX || pair == PAIR_IY))
        {
          litval = ullFromVal (right->aop->aopu.aop_lit);
          emit2 (litval & 1 ? "set %d, !mems" : "res %d, !mems", bstr, _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 4 : 2;
          return;
        }
      else if (right->aop->type == AOP_LIT)
        {
          /* Case with a bit-field length <8 and literal source */
          litval = (int) ulFromVal (right->aop->aopu.aop_lit);
          litval <<= bstr;
          litval &= (~mask) & 0xff;
          emit2 ("ld a, !mems", _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;
          if ((mask | litval) != 0xff)
            {
              emit2 ("and a, !immedbyte", mask);
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
            }
          if (litval)
            {
              emit2 ("or a, !immedbyte", (unsigned)litval);
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
            }
          emit2 ("ld !mems, a", _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;
          return;
        }
      else if (blen == 4 && bstr % 4 == 0 && pair == PAIR_HL && !aopInReg (right->aop, 0, A_IDX) && !requiresHL (right->aop) && (IS_Z80 || IS_Z180 || IS_EZ80_Z80 || IS_Z80N || IS_R800))
        {
          emit3 ((bstr ? A_RLD : A_RRD), 0, 0);
          cheapMove (ASMOP_A, 0, right->aop, 0, true);
          emit3 ((bstr ? A_RRD : A_RLD), 0, 0);
          return;
        }
      else
        {
          /* Case with a bit-field length <8 and arbitrary source */
          cheapMove (ASMOP_A, 0, right->aop, 0, true);
          /* shift and mask source value */
          if (blen + bstr == 8)
            AccLsh (bstr);
          else
            {
              AccRol (bstr);
              emit2 ("and a, !immedbyte", ~mask & 0xffu);
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
            }

          extraPair = getFreePairId (ic);
          if (extraPair == PAIR_INVALID)
            {
              if (pair != PAIR_HL)
                extraPair = PAIR_HL;
              else
                {
                  extraPair = PAIR_BC;
                  if (getPairId (right->aop) != PAIR_BC || !isLastUse (ic, right))
                    {
                      _push (extraPair);
                      needPopExtra = 1;
                    }
                }
            }
          emit2 ("ld %s, a", _pairs[extraPair].l);
          ld_cost (ASMOP_L, 0, ASMOP_A, 0, true);
          spillPair (extraPair);
          emit2 ("ld a, !mems", _pairs[pair].name);
          regalloc_dry_run_cost += (pair == PAIR_IX || pair == PAIR_IY) ? 3 : 1;

          emit2 ("and a, !immedbyte", mask);
          cost2 (2, 7, 6, 4, 8, 4, 2, 2);
          emit2 ("or a, %s", _pairs[extraPair].l);
          cost2 (1, 4, 4, 2, 4, 4, 1, 1);
          emit2 ("ld !mems, a", _pairs[pair].name);
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
      cheapMove (ASMOP_A, 0, right->aop, offset++, true);
      if (pair == PAIR_IX || pair == PAIR_IY)
        {
          emit2 ("ld %d !mems, a", pair_offset, _pairs[pair].name);
          regalloc_dry_run_cost += 3;
        }
      else
        {
          emit2 ("ld !mems, a", _pairs[pair].name);
          cost2 (1, 7, 7, 7, 8, 6, 2, 2);
        }
      if (rlen > 8 && pair != PAIR_IX && pair != PAIR_IY)
        {
          emit2 ("inc %s", _pairs[pair].name);
          cost2 (1, 6, 4, 2, 8, 4, 1, 1);
          pairincrement++;
          _G.pairs[pair].offset++;
        }
      else
        pair_offset++;
    }

  /* If there was a partial byte at the end */
  if (rlen)
    {
      mask = ((unsigned char)-1 << rlen) & 0xffu;

      if (right->aop->type == AOP_LIT)
        {
          /* Case with partial byte and literal source */
          litval = ullFromVal (right->aop->aopu.aop_lit);
          litval >>= (blen - rlen);
          litval &= (~mask) & 0xff;

          if (pair == PAIR_IX || pair == PAIR_IY)
            {
              emit2 ("ld a, %d !mems", pair_offset, _pairs[pair].name);
              cost2 (3, 19, 14, 9, 0, 10, 4, 5);
            }
          else
            {
              emit2 ("ld a, !mems", _pairs[pair].name);
              cost2 (1, 7, 7, 7, 8, 6, 2, 2);
            }

          if ((mask | litval) != 0xff)
            {
              emit2 ("and a, !immedbyte", mask);
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
            }
          if (litval)
            {
              emit2 ("or a, !immedbyte", (unsigned)litval);
              cost2 (2, 7, 6, 4, 8, 4, 2, 2);
            }
        }
      else
        {
          /* Case with partial byte and arbitrary source */
          cheapMove (ASMOP_A, 0, right->aop, offset++, true);
          emit2 ("and a, !immedbyte", (~mask) & 0xffu);
          cost2 (2, 7, 6, 4, 8, 4, 2, 2);

          extraPair = getFreePairId (ic);
          if (extraPair == PAIR_INVALID)
            {
              if (pair != PAIR_HL)
                extraPair = PAIR_HL;
              else
                {
                  extraPair = PAIR_BC;
                  if (getPairId (right->aop) != PAIR_BC || !isLastUse (ic, right))
                    {
                      _push (extraPair);
                      needPopExtra = 1;
                    }
                }
            }

          emit2 ("ld %s, a", _pairs[extraPair].l);
          ld_cost (ASMOP_L, 0, ASMOP_A, 0, true);
          spillPair (extraPair);

          if (pair == PAIR_IX || pair == PAIR_IY)
            {
              emit2 ("ld a, %d !mems", pair_offset, _pairs[pair].name);
              regalloc_dry_run_cost += 3;
            }
          else
            {
              emit2 ("ld a, !mems", _pairs[pair].name);
              cost2 (1, 7, 6, 6, 8, 6, 2, 2);
            }

          emit2 ("and a, !immedbyte", mask);
          cost2 (2, 7, 6, 4, 8, 4, 2, 2);
          emit2 ("or a, %s", _pairs[extraPair].l);
          cost2 (1, 4, 4, 2, 4, 4, 1, 1);
          if (needPopExtra)
            _pop (extraPair);

        }
      if (pair == PAIR_IX || pair == PAIR_IY)
        {
          emit2 ("ld %d !mems, a", pair_offset, _pairs[pair].name);
          regalloc_dry_run_cost += 3;
        }
      else
        {
          emit2 ("ld !mems, a", _pairs[pair].name);
          regalloc_dry_run_cost += 1;
        }
    }
  if (!isPairDead(pair, ic))
    while (pairincrement)
      {
        emit2 ("dec %s", _pairs[pair].name);
        cost2 (1, 6, 4, 2, 8, 4, 1, 1);
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
  bool pushed_a = false;
  bool pushed_pair = false;
  bool surviving_a = !isRegDead (A_IDX, ic);

  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);

  wassert (operandType (result)->next);
  bool bit_field = IS_BITVAR (operandType (result)->next);

  aopOp (result, ic, FALSE, FALSE);
  aopOp (right, ic, FALSE, FALSE);

  size = right->aop->size;

  if (IS_SM83)
    pairId = isRegOrLit (right->aop) ? PAIR_HL : PAIR_DE;
  else if (IY_RESERVED)
    pairId = (isRegOrLit (right->aop) || right->aop->type == AOP_STK) ? PAIR_HL : PAIR_DE;
  if (isPair (result->aop) && isPairDead (getPairId (result->aop), ic) && !(size > 1 && sameRegs (result->aop, right->aop)))
    pairId = getPairId (result->aop);

  if (IS_SM83 && size == 1 && result->aop->type == AOP_LIT && (((unsigned long)operandLitValue (result) & 0xff00) == 0xff00) && (isRegDead (A_IDX, ic) || aopInReg (right->aop, 0, A_IDX)) && !bit_field) // SM83 has special instructions for address range 0xff00 - 0xffff.
    {
      cheapMove (ASMOP_A, 0, right->aop, 0, isRegDead (A_IDX, ic));
      emit2 ("ldh !mems, a", aopGetLitWordLong (result->aop, 0, true));
      cost (2, 12);
      goto release;
    }

  /* Handle the exceptions first */
  if (isPair (result->aop) && size == 1 && !bit_field)
    {
      /* Just do it */
      const char *pair = getPairName (result->aop);
      if (canAssignToPtr3 (right->aop) && isPtr (pair))        // Todo: correct cost for pair iy.
        {
          if (!regalloc_dry_run)
            emit2 ("ld !mems, %s", pair, aopGet (right->aop, 0, FALSE));
          if (getPairId (result->aop) == PAIR_HL)
            cost2 (1, 7, 7, 6, 8, 6, 2, 2); // Assume ld (hl), r
          else if (aopInReg (right->aop, 0, A_IDX))
            cost2 (1, 7, 7, 7, 8, 6 , 2, 2); // Assume ld (rr), a
          else
            {
              ld_cost (ASMOP_A, 0, right->aop, 0, true);
              cost2 (1, 7, 7, 7, 8, 6 , 2, 2); // Assume ld (rr), a
            }
        }
      else
        {
          if (surviving_a && !pushed_a && !aopInReg (right->aop, 0, A_IDX))
            _push (PAIR_AF), pushed_a = TRUE;
          genMove_o (ASMOP_A, 0, right->aop, 0, 1, true,
            pairId != PAIR_HL && isPairDead (PAIR_HL, ic) && right->aop->regs[L_IDX] < offset && right->aop->regs[H_IDX] < offset, false, pairId != PAIR_IY && isPairDead (PAIR_IY, ic) && right->aop->regs[IYL_IDX] < offset && right->aop->regs[IYH_IDX] < offset, true);
          emit2 ("ld !mems, a", pair);
          cost2 (1, 7, 7, 7, 8, 6 , 2, 2); // Assume ld (rr), a
        }
      goto release;
    }

  /* Rematerialized stack location */
  if (result->aop->type == AOP_STL && !bit_field && size <= 4)
    {
      struct asmop saop;
      init_stackop (&saop, size, result->aop->aopu.aop_stk);
      genMove (&saop, right->aop, isRegDead (A_IDX, ic), isPairDead(PAIR_HL, ic), isPairDead(PAIR_DE, ic), isPairDead(PAIR_IY, ic));
      goto release;
    }

  // Using ldir is cheapest for large memory-to-memory transfers.
  // sm83 doesn't have ldir. Rabbit 2000 to Rabbit 3000 (i.e. r2k and r2ka ports) have a wait-state bug breaking ldir between different types of memory.
  if (!IS_SM83 && (!IS_R2K && !IS_R2KA || result->aop->type == AOP_STL) && (right->aop->type == AOP_STK || right->aop->type == AOP_EXSTK) && size > 2)
    {
      int fp_offset, sp_offset;

      if(!isPairDead (PAIR_DE, ic))
        _push (PAIR_DE);
      if(!isPairDead (PAIR_BC, ic))
        _push (PAIR_BC);
      if(!isPairDead (PAIR_HL, ic))
        _push (PAIR_HL);

      fetchPair (PAIR_DE, result->aop);

      fp_offset = right->aop->aopu.aop_stk + (right->aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0);
      sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
      emit2 ("!ldahlsp", sp_offset);
      regalloc_dry_run_cost += 4;
      emit2 ("ld bc, !immedword", (unsigned)size);
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("ldir");
      cost2 (2, 21 * size - 5, 14 * size - 2, 7 * size - 1, 0, 18 * size - 4, 2 * size - 1, 4 * size);
      spillPair (PAIR_HL);

      if(!isPairDead (PAIR_HL, ic))
        _pop (PAIR_HL);
      if(!isPairDead (PAIR_BC, ic))
        _pop (PAIR_BC);
      if(!isPairDead (PAIR_DE, ic))
        _pop (PAIR_DE);
      goto release;
    }

  if (getPairId (result->aop) == PAIR_IY && !bit_field)
    {
      /* Just do it */
      while (size--)
        {
          if (canAssignToPtr3 (right->aop))
            {
              if (!regalloc_dry_run)
                emit2 ("ld !*iyx, %s", offset, aopGet (right->aop, offset, FALSE));
              if (right->aop->type == AOP_LIT)
                cost2 (4, 19, 15, 11, 0, 12, 5, 5); // ld d (iy), n
              else
                cost2 (3, 19, 15, 10, 0, 10, 4, 5); // ld d (iy), r
            }
          else
            {
              cheapMove (ASMOP_A, 0, right->aop, offset, true);
              emit2 ("ld !*iyx, a", offset);
              cost2 (3, 19, 15, 10, 0, 10, 4, 5); // ld d (iy), r
            }
          offset++;
        }
      goto release;
    }
  else if (getPairId (result->aop) == PAIR_HL && !isPairDead (PAIR_HL, ic) && !bit_field)
    {
      while (offset < size)
        {
          last_offset = offset;

          if (IS_EZ80_Z80 && offset + 1 < size && getPairId_o (right->aop, offset) != PAIR_INVALID)
            {
              emit2 ("ld !mems, %s", _pairs[PAIR_HL].name, _pairs[getPairId_o (right->aop, offset)].name);
              regalloc_dry_run_cost += 2;
              offset += 2;

              if (offset < size)
                {
                  emit3w (A_INC, ASMOP_HL, 0);
                  emit3w (A_INC, ASMOP_HL, 0);
                  _G.pairs[PAIR_HL].offset++;
                }

              continue;
            }
          else if (isRegOrLit (right->aop) && !IS_SM83)
            {
              if (!regalloc_dry_run)
                emit2 ("ld !mems, %s", _pairs[PAIR_HL].name, aopGet (right->aop, offset, FALSE));
              ld_cost (aopInReg (right->aop, offset, A_IDX) ? ASMOP_L : ASMOP_A, 0, right->aop, offset, true);
              offset++;
            }
          else
            {
              if (surviving_a && !pushed_a && (!aopInReg (right->aop, 0, A_IDX) || offset))
                _push (PAIR_AF), pushed_a = TRUE;
              genMove_o (ASMOP_A, 0, right->aop, offset, 1, true, false, false, false, true);
              emit2 ("ld !mems, a", _pairs[PAIR_HL].name);
              cost2 (1 + IS_TLCS90,	7, 7, 6, 8, 6, 2, 2);
              offset++;
            }

          if (offset < size)
            {
              emit3w (A_INC, ASMOP_HL, 0);
              _G.pairs[PAIR_HL].offset++;
            }
        }

      /* Fixup HL back down */
      while (last_offset --> 0)
        emit3w (A_DEC, ASMOP_HL, 0);
      goto release;
    }

  if (!IS_SM83 && !bit_field && isLitWord (result->aop) && size == 2 && offset == 0 && !sameRegs (result->aop, right->aop) &&
      (right->aop->type == AOP_REG && getPairId (right->aop) != PAIR_INVALID || isLitWord (right->aop) && (isPairDead (PAIR_HL, ic) || isPairDead (PAIR_DE, ic) || isPairDead (PAIR_BC, ic))))
    {
      if (isLitWord (right->aop))
        {
          pairId = isPairDead (PAIR_HL, ic) ? PAIR_HL :  isPairDead (PAIR_DE, ic) ? PAIR_DE : PAIR_BC;
          fetchPairLong (pairId, right->aop, ic, 0);
        }
      else
        pairId = getPairId (right->aop);
      emit2 ("ld !mems, %s", aopGetLitWordLong (result->aop, offset, FALSE), _pairs[pairId].name);
      if (!IS_TLCS90 && pairId == PAIR_HL)
        cost2 (3, 16, 16, 13, 0, 0, 5, 5);
      else
        cost2 (4, 20, 19, 15, 0, 12, 6, 6);
      goto release;
    }
  if (!IS_SM83 && !bit_field && isLitWord (result->aop) && size == 4 && offset == 0 &&
    (getPartPairId (right->aop, 0) != PAIR_INVALID && getPartPairId (right->aop, 2) != PAIR_INVALID || isLitWord (right->aop)))
    {
      if (isLitWord (right->aop))
        {
          pairId = PAIR_HL;
          fetchPairLong (pairId, right->aop, ic, 0);
        }
      else
        pairId = getPartPairId (right->aop, 0);
      emit2 ("ld !mems, %s", aopGetLitWordLong (result->aop, offset, FALSE), _pairs[pairId].name);
      if (!IS_TLCS90 && pairId == PAIR_HL)
        cost2 (3, 16, 16, 13, 0, 0, 5, 5);
      else
        cost2 (4, 20, 19, 15, 0, 12, 6, 6);
      if (isLitWord (right->aop))
        {
          pairId = PAIR_HL;
          fetchPairLong (pairId, right->aop, ic, 2);
        }
      else
        pairId = getPartPairId (right->aop, 2);
      emit2 ("ld (%s+%d), %s", aopGetLitWordLong (result->aop, offset, FALSE),2,  _pairs[pairId].name); // Handling of literal addresses is somewhat broken, use explicit offset as workaround.
      regalloc_dry_run_cost += (pairId == PAIR_HL) ? 3 : 4;
      goto release;
    }

  if (getPairId (result->aop) != pairId &&
    (right->aop->regs[_pairs[pairId].l_idx] >= 0 || right->aop->regs[_pairs[pairId].h_idx] >= 0))
    UNIMPLEMENTED;

  /* if the operand is already in dptr
     then we do nothing else we move the value to dptr */
  if (bit_field && getPairId (result->aop) != PAIR_INVALID && (getPairId (result->aop) != PAIR_IY || SPEC_BLEN (getSpec (operandType (result)->next)) < 8 || isPairDead (getPairId (result->aop), ic)))   /* Avoid destroying result by increments */
    pairId = getPairId (result->aop);
  else
    {
      if (!isPairDead (pairId, ic) && getPairId (result->aop) != pairId)
        {
          _push (pairId);
          pushed_pair = true;
        }
      genMove (pairId == PAIR_HL ? ASMOP_HL : pairId == PAIR_DE ? ASMOP_DE : ASMOP_BC, result->aop,
        isRegDead(A_IDX, ic) && right->aop->regs[A_IDX] < 0,
        isPairDead (PAIR_HL, ic) && right->aop->regs[L_IDX] < 0 && right->aop->regs[H_IDX] < 0,
        isPairDead (PAIR_DE, ic) && right->aop->regs[E_IDX] < 0 && right->aop->regs[D_IDX] < 0,
        isPairDead (PAIR_IY, ic) && right->aop->regs[IYL_IDX] < 0 && right->aop->regs[IYH_IDX] < 0);
    }
  /* so hl now contains the address */
  /*freeAsmop (result, NULL, ic);*/

  /* if bit then pack */
  if (bit_field)
    {
      genPackBits (getSpec (operandType (result)->next), right, pairId, ic);
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
              emit2 ("ld !mems, %s", _pairs[pairId].name, _pairs[getPairId_o (right->aop, offset)].name);
              regalloc_dry_run_cost += 2;
              offset += 2;

              if (offset < size)
                {
                  emit2 ("inc %s", _pairs[pairId].name);
                  cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                  emit2 ("inc %s", _pairs[pairId].name);
                  cost2 (1, 6, 4, 2, 8, 4, 1, 1);
                  _G.pairs[pairId].offset++;
                }

              continue;
            }

          if (!zero_a && offset + 1 < size && aopIsLitVal (right->aop, offset, 2, 0x0000) && !surviving_a)
            {
              emit2 ("xor a, a");
              cost2 (1, 4, 4, 2, 4, 4, 1, 1);
              zero_a = true;
            }

          if (aopIsLitVal (right->aop, offset, 1, 0x00) && zero_a)
            {
              emit2 ("ld !mems, a", _pairs[pairId].name);
              cost2 (1, 7, 7, 7, 8, 6, 2, 2);
            }
          else if (isRegOrLit (right->aop) && pairId == PAIR_HL)
            {
              if (!regalloc_dry_run)
                emit2 ("ld !mems, %s", _pairs[pairId].name, aopGet (right->aop, offset, FALSE));
              ld_cost (aopInReg (right->aop, offset, A_IDX) ? ASMOP_L : ASMOP_A, 0, right->aop, offset, true);
            }
          else
            {
              if (surviving_a && !pushed_a && (!aopInReg (right->aop, 0, A_IDX) || offset))
                _push (PAIR_AF), pushed_a = true;
              genMove_o (ASMOP_A, 0, right->aop, offset, 1, true,
                pairId != PAIR_HL && isPairDead (PAIR_HL, ic) && right->aop->regs[L_IDX] < offset && right->aop->regs[H_IDX] < offset, false, pairId != PAIR_IY && isPairDead (PAIR_IY, ic) && right->aop->regs[IYL_IDX] < offset && right->aop->regs[IYH_IDX] < offset, true);
              zero_a = false;
              emit2 ("ld !mems, a", _pairs[pairId].name);
              cost2 (1, 7, 7, 7, 8, 6, 2, 2);
            }
          offset++;

          if (offset < size)
            {
              if (right->aop->regs[_pairs[pairId].l_idx] >= offset || right->aop->regs[_pairs[pairId].h_idx] >= offset)
                UNIMPLEMENTED;
              emit2 ("inc %s", _pairs[pairId].name);
              cost2 (1, 6, 4, 2, 8, 4, 1, 1);
              _G.pairs[pairId].offset++;
            }
        }
      /* Restore operand in pair. */
      if (!isPairDead (pairId, ic) && getPairId (result->aop) == pairId)
        while(last_offset --> 0)
          {
            emit2 ("dec %s", _pairs[pairId].name);
            cost2 (1, 6, 4, 2, 8, 4, 1, 1);
            _G.pairs[pairId].offset--;
          }
    }
release:
  if (pushed_pair)
    _pop (pairId);
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

  /* Special case: Condition is bool */
  if (IS_BOOL (operandType (cond)) && !aopInReg (cond->aop, 0, A_IDX) && !aopInReg (cond->aop, 0, IYL_IDX) && !aopInReg (cond->aop, 0, IYH_IDX))
    {
      if (!regalloc_dry_run)
        {
          emit2 ("bit 0, %s", aopGet (cond->aop, 0, FALSE));
          genIfxJump (ic, "nz");
        }
      bit8_cost (cond->aop); // todo: fix, bit has different cost!

      goto release;
    }
  else if (cond->aop->size == 1 && !isRegDead (A_IDX, ic) &&
    (aopInReg (cond->aop, 0, B_IDX) || aopInReg (cond->aop, 0, C_IDX) || aopInReg (cond->aop, 0, D_IDX) || aopInReg (cond->aop, 0, E_IDX) || aopInReg (cond->aop, 0, H_IDX) || aopInReg (cond->aop, 0, L_IDX)))
    {
      emit3 (A_INC, cond->aop, 0);
      emit3 (A_DEC, cond->aop, 0);
      genIfxJump (ic, "nz");

      goto release;
    }
  else if (IS_RAB && (getPairId (cond->aop) == PAIR_HL || getPairId (cond->aop) == PAIR_IY) && isPairDead (getPairId (cond->aop), ic))
    {
      emit2 ("bool %s", _pairs[getPairId (cond->aop)].name);
      cost (1 + (getPairId (cond->aop) == PAIR_IY), 2 + 2 * (getPairId (cond->aop) == PAIR_IY));
      genIfxJump (ic, "nz");
      
      goto release;
    }
  /* get the value into acc */
  else if (cond->aop->type != AOP_CRY)
    _toBoolean (cond, !popIc);
  else
    isbit = 1;
  /* the result is now in the accumulator */
  freeAsmop (cond, NULL);

  /* if the condition is  a bit variable */
  if (isbit && IS_ITEMP (cond) && SPIL_LOC (cond))
    genIfxJump (ic, SPIL_LOC (cond)->rname);
  else if (isbit && !IS_ITEMP (cond))
    genIfxJump (ic, OP_SYMBOL (cond)->rname);
  else
    genIfxJump (ic, popIc ? "a" : "nz");

  return;

release:

  freeAsmop (cond, NULL);

  return;
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
  aopOp (IC_RESULT (ic), ic, true, false);

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
      if ((IS_TLCS90 || IS_EZ80_Z80) && in_fp_range)
        {
          emit2 (IS_TLCS90 ? "lda %s, ix, !immed%d" : "lea %s, ix, !immed%d", _pairs[pair].name, fp_offset);
          cost (3, IS_TLCS90 ? 10 : 3);
        }
      else
        setupPairFromSP (pair, sp_offset);
    }
  else
    {
      pair = getPairId (IC_RESULT (ic)->aop);
      if (pair == PAIR_INVALID)
        {
          pair = IS_SM83 ? PAIR_DE : PAIR_HL;
          spillPair (pair);
        }
      emit2 ("ld %s, !hashedstr+%ld", _pairs[pair].name, sym->rname, (long)(operandLitValue (right)));
      if (pair == PAIR_IY)
        cost2 (4 - IS_TLCS90, 14, 12, 8, 0, 6, 4, 4);
      else
        cost2 (3, 10, 9, 6, 12, 6, 3, 3);
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
  
  const bool hl_dead = isPairDead (PAIR_HL, ic);

  /* Dont bother assigning if they are the same */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  aopOp (right, ic, FALSE, FALSE);
  aopOp (result, ic, TRUE, FALSE);

  /* if they are the same registers */
  if (sameRegs (right->aop, result->aop))
    {
      emitDebug ("; (locations are the same)");
      goto release;
    }

  /* if the result is a bit */
  if (result->aop->type == AOP_CRY)
    {
      wassertl (0, "Tried to assign to a bit");
    }

  /* general case */
  size = result->aop->size;
  offset = 0;

  // SM83 has special instruction for access to addresses 0xff00 to 0xffff, so use them here, when possible
  if (IS_SM83 && size == 1 && aopInReg (result->aop, 0, A_IDX) && right->aop->type == AOP_HL && SPEC_ABSA (OP_SYM_ETYPE (right)) && (SPEC_ADDR (OP_SYM_ETYPE (right)) & 0xff00) == 0xff00)
    {
      emit2 ("ldh a, !mems", right->aop->aopu.aop_dir);
      cost (2, 12);
      goto release;
    }
  else if (IS_SM83 && size == 1 && aopInReg (right->aop, 0, A_IDX) && result->aop->type == AOP_HL && SPEC_ABSA (OP_SYM_ETYPE (result)) && (SPEC_ADDR (OP_SYM_ETYPE (result)) & 0xff00) == 0xff00)
    {
      emit2 ("ldh (#%x), a", result->aop->aopu.aop_dir);
      cost (2, 12);
      goto release;
    }

  if (isPair (result->aop) && getPairId (result->aop) != PAIR_IY ||
    isPair (right->aop) && result->aop->type == AOP_IY && size == 2)
    genMove (result->aop, right->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), true);
  else if (size == 2 && isPairDead (PAIR_HL, ic) &&
    (!IS_SM83 && (right->aop->type == AOP_STK && !_G.omitFramePtr || right->aop->type == AOP_IY || right->aop->type == AOP_LIT) && result->aop->type == AOP_IY || // Use ld (nn), hl
    !IS_SM83 && right->aop->type == AOP_IY && (result->aop->type == AOP_STK && !_G.omitFramePtr || result->aop->type == AOP_IY) || // Use ld hl, (nn)
    !IS_SM83 && right->aop->type == AOP_LIT && (result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK) && (result->aop->aopu.aop_stk + offset + _G.stack.offset + (result->aop->aopu.aop_stk > 0 ? _G.stack.param_offset : 0) + _G.stack.pushed) == 0 || // Use ex (sp), hl
    (IS_RAB || IS_TLCS90) && (result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK) && (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD))) // Use ld d(sp), hl
    {
      fetchPair (PAIR_HL, right->aop);
      genMove (result->aop, ASMOP_HL, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic));
    }
  else if (size == 2 && getPairId (right->aop) != PAIR_INVALID)
    genMove (result->aop, right->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic));
  else if (size <= 2 && requiresHL (right->aop) && requiresHL (result->aop) && IS_SM83)
    genMove (result->aop, right->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic));
  else if (getPairId (right->aop) == PAIR_IY && result->aop->type != AOP_REG)
    {
      while (size--)
        {
          if (size == 0)
            {
              if (IS_TLCS90) // f needs to be preserved for tlcs90.
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
              if (result->aop->type == AOP_IY) /* Take care not to overwrite iy */
                {
                  emit2 ("ld (%s+%d), a", result->aop->aopu.aop_dir, size);
                  cost2 (3, 13, 13, 10, 16, 10, 4, 4);
                }
              else
                cheapMove (result->aop, size, ASMOP_A, 0, true);
            }
          else if (size == 1)
            {
              if (result->aop->type == AOP_IY) /* Take care not to overwrite iy */
                {
                  emit2 ("ld !mems, iy", result->aop->aopu.aop_dir);
                  cost2 (4, 20, 19, 15, 0, 12, 6, 6);
                  size--;
                }
              else if (result->aop->type == AOP_EXSTK || IS_TLCS90) /* Take care not to overwrite iy or f */
                {
                  bool pushed_pair = FALSE;
                  PAIR_ID pair = getDeadPairId (ic);
                  if (pair == PAIR_INVALID)
                  {
                    pair = PAIR_HL;
                    _push(pair);
                    pushed_pair= TRUE;
                  }
                  fetchPair (pair, right->aop);
                  commitPair (result->aop, pair, ic, FALSE);
                  if (pushed_pair)
                    _pop (pair);
                  size--;
                }
              else
                {
                  _push (PAIR_IY);
                  _pop (PAIR_AF);
                  cheapMove (result->aop, size, ASMOP_A, 0, true);
                }
            }
          else
            {
              if (result->aop->type == AOP_IY) /* Take care not to overwrite iy */
                {
                  cheapMove (ASMOP_A, 0, ASMOP_ZERO, 0, true);
                  emit2 ("ld (%s+%d), a", result->aop->aopu.aop_dir, size);
                  cost2 (3, 13, 13, 10, 16, 10, 4, 4);
                }
              else
                cheapMove (result->aop, size, ASMOP_ZERO, 0, true);
            }
        }
    }
  else if (size == 4 && (requiresHL (right->aop) && right->aop->type != AOP_REG) && (requiresHL (result->aop) && result->aop->type != AOP_REG ) && isPairDead (PAIR_DE, ic) && (IS_SM83 || IY_RESERVED))
    {
      /* Special case - simple memcpy */
      if (!regalloc_dry_run)
        {
          aopGet (right->aop, LSB, FALSE);
          emit2 ("ld d, h");
          emit2 ("ld e, l");
          aopGet (result->aop, LSB, FALSE);
        }
      regalloc_dry_run_cost += 8;       // Todo: More exact cost here!

      while (size--)
        {
          emit2 ("ld a, !mems", "de");
          cost2 (1 + IS_TLCS90, 7, 6, 6, 8, 6, 2, 2);
          if (size != 0)
            {
              emit2 ("!lldahli");
              emit2 ("inc de");
              regalloc_dry_run_cost += 3;
            }
          else
            {
              emit2 ("ld !*hl, a");
              cost2 (1 + IS_TLCS90, 7, 7, 7, 8, 6, 2, 2);
            }
        }
      spillPair (PAIR_HL);
    }
  else
    {
      // ldir could overwrite if areas overlap.
      bool down = false;
      if ((result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK) &&
        (right->aop->type == AOP_STK || right->aop->type == AOP_EXSTK))
          if (!regalloc_dry_run && result->aop->aopu.aop_stk > right->aop->aopu.aop_stk && result->aop->aopu.aop_stk < right->aop->aopu.aop_stk + size)
            down = true;

      if (!IS_SM83 && !down && // sm83 doesn't have ldir, Rabbit 2000 to Rabbit 3000 (i.e. r2k and r2ka ports) ldir is affected by a wait state bug when copying between different types of memory.
          (result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK || result->aop->type == AOP_DIR
           || result->aop->type == AOP_IY) && (right->aop->type == AOP_STK || right->aop->type == AOP_EXSTK
               || right->aop->type == AOP_DIR || right->aop->type == AOP_IY) && size >= 2)
        {
          // This estimation is only accurate, if neither operand is AOP_EXSTK, and we are optimizing for code size or targeting the Z80, Z180, eZ80, Z80N or Rabbit 3000A.
          int sizecost_n, sizecost_l, cyclecost_n, cyclecost_l;
          const bool hl_alive = !isPairDead (PAIR_HL, ic);
          const bool de_alive = !isPairDead (PAIR_DE, ic);
          const bool bc_alive = !isPairDead (PAIR_BC, ic);
          bool l_better;

          if (IS_EZ80_Z80 && result->aop->type == AOP_STK && right->aop->type == AOP_STK) // eZ80: Use 16-Bit loads, except for odd trailing byte.
            sizecost_n = (size / 2 * 6) + (size % 2 * 6);
          else if (IS_RAB && result->aop->type == AOP_STK && right->aop->type == AOP_STK) // Rabbit: Use 16-Bit loads, except for odd trailing byte.
            sizecost_n = (size / 2 * 4) + (size % 2 * 6);
          else // Use 8-Bit loads: Z80, Z180, Z80N.
            sizecost_n = 6 * size;

          sizecost_l = 13 + hl_alive * 2 + de_alive * 2 + bc_alive * 2 -
            (right->aop->type == AOP_DIR || right->aop->type == AOP_IY) -
            (result->aop->type == AOP_DIR || result->aop->type == AOP_IY) * 2;

          if (IS_EZ80_Z80 && result->aop->type == AOP_STK && right->aop->type == AOP_STK)
            cyclecost_n = (size / 2 * 10) + (size % 2 * 8);
          else if (IS_RAB && result->aop->type == AOP_STK && right->aop->type == AOP_STK)
            cyclecost_n = (size / 2 * 18) + (size % 2 * 18);
          else if (IS_EZ80_Z80)
            cyclecost_n = 8 * size;
          else if (IS_Z180)
            cyclecost_n = 30 * size;
          else if (IS_RAB)
            cyclecost_n = 18 * size;
          else // Z80, Z80N
            cyclecost_n = 38 * size;

          if (IS_EZ80_Z80)
            cyclecost_l = 2 * size + 9 + hl_alive * 6 + de_alive * 6 + bc_alive * 6; // lea is as fast as ld rr, nn. So it does not matter if the operands are on stack.
          else if (IS_Z180)
            cyclecost_l = 14 * size + 42 + hl_alive * 22 + de_alive * 22 + bc_alive * 22 -
              (right->aop->type == AOP_DIR || right->aop->type == AOP_IY) * 7 -
              (result->aop->type == AOP_DIR || result->aop->type == AOP_IY) * 10;
          else if (IS_RAB)
            cyclecost_l = 7 * size + 34 + hl_alive * 17 + de_alive * 17 + bc_alive * 17 -
              (right->aop->type == AOP_DIR || right->aop->type == AOP_IY) * 4 -
              (result->aop->type == AOP_DIR || result->aop->type == AOP_IY) * 6;    
          else // Z80
            cyclecost_l = 21 * size + 51 + hl_alive * 21 + de_alive * 21 + bc_alive * 21 -
              (right->aop->type == AOP_DIR || right->aop->type == AOP_IY) * 11 -
              (result->aop->type == AOP_DIR || result->aop->type == AOP_IY) * 15;

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

              if (result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK)
                {
                  int fp_offset =
                    result->aop->aopu.aop_stk + offset + (result->aop->aopu.aop_stk >
                        0 ? _G.stack.param_offset : 0);
                  int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
                  emit2 ("!ldahlsp", sp_offset);
                  regalloc_dry_run_cost += 4;
                  emit3w (A_EX, ASMOP_DE, ASMOP_HL);
                }
              else
                pointPairToAop (PAIR_DE, result->aop, 0);

              if (right->aop->type == AOP_STK || right->aop->type == AOP_EXSTK)
                {
                  int fp_offset =
                    right->aop->aopu.aop_stk + offset + (right->aop->aopu.aop_stk >
                        0 ? _G.stack.param_offset : 0);
                  int sp_offset = fp_offset + _G.stack.pushed + _G.stack.offset;
                  emit2 ("!ldahlsp", sp_offset);
                  spillPair (PAIR_HL);
                  regalloc_dry_run_cost += 4;
                }
              else
                pointPairToAop (PAIR_HL, right->aop, 0);

              if (size <= 2 + (!IS_RAB && optimize.codeSpeed) ||
                // Early Rabbits (up to Rabbit 3000) have a wait state bug when ldir copies between different types of memory.
                (IS_R2K || IS_R2KA) && !((right->aop->type == AOP_STK || right->aop->type == AOP_EXSTK) && (result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK)))
                for(int i = 0; i < size; i++)
                  {
                    emit2 ("ldi");
                    cost2 (2, 16, 12, 10, 0 , 14, 5, 4);
                  }
              else
                {
                  emit2 ("ld bc, !immed%d", size);
                  emit2 ("ldir");
                  regalloc_dry_run_cost += 5;
                }
              spillPair (PAIR_HL);
              spillPair (PAIR_DE);
              spillPair (PAIR_BC);

              if (bc_alive)
                _pop (PAIR_BC);
              if (de_alive)
                _pop (PAIR_DE);
              if (hl_alive)
                _pop (PAIR_HL);

              goto release;
            }
        }
      if ((result->aop->type == AOP_REG || result->aop->type == AOP_STK || result->aop->type == AOP_EXSTK || result->aop->type == AOP_IY || result->aop->type == AOP_HL) && (right->aop->type == AOP_REG || right->aop->type == AOP_STK || right->aop->type == AOP_EXSTK || right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD || right->aop->type == AOP_DIR || right->aop->type == AOP_IY || right->aop->type == AOP_HL))
        genMove (result->aop, right->aop, isRegDead (A_IDX, ic), isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic));
      else
        while (size--)
          {
            const bool hl_free = hl_dead &&
              (right->aop->regs[L_IDX] <= offset) && (right->aop->regs[H_IDX] <= offset) &&
              (result->aop->regs[L_IDX] < 0 || result->aop->regs[L_IDX] >= offset) && (result->aop->regs[H_IDX] < 0 || result->aop->regs[H_IDX] >= offset);
            const bool save_hl = !hl_free && ((IS_SM83 || IY_RESERVED) && (requiresHL (right->aop) || requiresHL (result->aop)));

            if (save_hl)
              _push (PAIR_HL);
            cheapMove (result->aop, offset, right->aop, offset, isRegDead (A_IDX, ic));
            if (save_hl)
              {
                _pop (PAIR_HL);
                spillPair (PAIR_HL);
              }
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
  bool pushed_pair = false;
  PAIR_ID pair;

  aopOp (jtcond, ic, false, false);

  wassert (isPairDead (PAIR_HL, ic));

  if (!regalloc_dry_run)
    jtab = newiTempLabel (NULL);

  if (IS_TLCS90 && // Use lda hl, hl, a
    (jtcond->aop->size == 1 || aopIsLitVal (jtcond->aop, 1, jtcond->aop->size - 1, 0)) &&
    (isRegDead (A_IDX, ic) || aopInReg (jtcond->aop, 0, A_IDX)) &&
    !aopInReg (jtcond->aop, 0, C_IDX) && !aopInReg (jtcond->aop, 0, E_IDX))
    {
      
      genMove (ASMOP_A, jtcond->aop, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic));
      if (!regalloc_dry_run)
        emit2 ("ld hl, !immed!tlabel", labelKey2num (jtab->key));
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      emit2 ("lda hl, hl, a");
      cost (2, 14);
      emit2 ("ld hl, a(hl)");
      cost (2 , 16);
      goto jump;
    }

  // Choose extra pair DE or BC for addition
  if (jtcond->aop->type == AOP_REG && jtcond->aop->aopu.aop_reg[0]->rIdx == E_IDX && isPairDead (PAIR_DE, ic))
    pair = PAIR_DE;
  else if (jtcond->aop->type == AOP_REG && jtcond->aop->aopu.aop_reg[0]->rIdx == C_IDX && isPairDead (PAIR_BC, ic))
    pair = PAIR_BC;
  else if ((pair = getDeadPairId (ic)) == PAIR_INVALID)
    pair = PAIR_DE;

  if (!isPairDead (pair, ic))
    {
      _push (pair);
      pushed_pair = true;
    }

  genMove (pair == PAIR_DE ? ASMOP_DE : ASMOP_BC, jtcond->aop, isRegDead (A_IDX, ic), true, isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic));

  if (!regalloc_dry_run)
    emit2 ("ld hl, !immed!tlabel", labelKey2num (jtab->key));
  cost2 (3, 10, 9, 6, 12, 6, 3, 3);
  emit2 ("add hl, %s", _pairs[pair].name);
  cost2 (1, 11, 7, 2, 8, 8, 1, 1);
  emit2 ("add hl, %s", _pairs[pair].name);
  cost2 (1, 11, 7, 2, 8, 8, 1, 1);
  spillPair (PAIR_HL);

  if (IS_TLCS90 || IS_EZ80_Z80)
    {
      emit2 ("ld hl, (hl)");
      cost (2, IS_TLCS90 ? 8 : 4);
    }
  else if (IS_RAB)
    {
      emit2 ("ld hl, 0(hl)");
      cost (3, 11);
    }
  else
    {
      emit2 ("ld %s, !*hl", _pairs[pair].l);
      cost2 (1, 7, 6, 5, 8, 6, 2, 2);
      emit2 ("inc hl");
      cost2 (1, 6, 4, 2, 8, 4, 1, 1);
      emit2 ("ld h, !*hl");
      cost2 (1, 7, 6, 5, 8, 6, 2, 2);
      emit3 (A_LD, ASMOP_L, pair == PAIR_DE ? ASMOP_E : ASMOP_C);
    }

jump:
  if (pushed_pair)
    _pop (pair);

  emit2 ("!jphl");
  cost2 (1 + IS_TLCS90, 4, 3, 4, 4, 8, 3, 1);

  if (!regalloc_dry_run)
    {
      emitLabelSpill (jtab);
      for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
        emit2 (".dw !tlabel", labelKey2num (jtab->key));
    }
  // regalloc_dry_run_cost += 3 // doesn't matter and might overflow cost

  freeAsmop (IC_JTCOND (ic), NULL);
}

/*-----------------------------------------------------------------*/
/* genCast - gen code for casting                                  */
/*-----------------------------------------------------------------*/
static void
genCast (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *right = IC_RIGHT (ic);
  sym_link *resulttype = operandType (result);
  sym_link *righttype = operandType (right);
  int size;
  bool surviving_a = !isRegDead (A_IDX, ic);
  bool pushed_a = FALSE;

  /* if they are equivalent then do nothing */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  aopOp (right, ic, false, false);
  aopOp (result, ic, true, false);

  /* if the result is a bit */
  if (result->aop->type == AOP_CRY)
    {
      wassertl (0, "Tried to cast to a bit");
    }

  unsigned topbytemask = (IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
   (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;

  // Cast to _BitInt can require mask of top byte.
  if (IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8) && bitsForType (resulttype) < bitsForType (righttype))
    {
      if (!isRegDead (A_IDX, ic) || result->aop->regs[A_IDX] >= 0 && result->aop->regs[A_IDX] != result->aop->size - 1)
        _push (PAIR_AF), pushed_a = true;
      genMove (result->aop, right->aop, true, isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic));
      cheapMove (ASMOP_A, 0, result->aop, result->aop->size - 1, true);
      emit2 ("and a, #0x%02x", topbytemask);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);
      if (!SPEC_USIGN (resulttype)) // Sign-extend
        {
          symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
          emit2 ("bit %d, a", (int)(SPEC_BITINTWIDTH (resulttype) % 8 - 1));
          cost2 (2, 8, 6, 4, 8, 4, 2, 2);
          if (!regalloc_dry_run)
            emit2 ("jr z, !tlabel", labelKey2num (tlbl->key));
          emit2 ("or a, #0x%02x", ~topbytemask & 0xffu);
          regalloc_dry_run_cost += 4;
          emitLabel (tlbl);
        }
      cheapMove (result->aop, result->aop->size - 1, ASMOP_A, 0, true);

      goto release;
    }

  /* casting to bool */
  if (IS_BOOL (resulttype) && IS_RAB && right->aop->size == 2 &&
    (aopInReg (right->aop, 0, HL_IDX) && isPairDead (PAIR_HL, ic) || aopInReg (right->aop, 0, IY_IDX) && isPairDead (PAIR_IY, ic)))
    {
      bool iy = aopInReg (right->aop, 0, IY_IDX);
      emit2 ("bool %s", _pairs[getPairId (right->aop)].name);
      cheapMove (result->aop, 0, iy ? ASMOP_IYL : ASMOP_L, 0, isRegDead (A_IDX, ic));
      goto release;
    }
  if (IS_BOOL (resulttype))
    {
      _castBoolean (right);
      outAcc (result);
      goto release;
    }

  /* if they are the same size or less */
  if (result->aop->size <= right->aop->size)
    {
      genAssign (ic);
      goto release;
    }

  // Now we know that the size of destination is greater than the size of the source

  /* now depending on the sign of the destination */
  size = result->aop->size - right->aop->size;
  
  /* Unsigned or not an integral type - fill with zeros */
  if (IS_BOOL (righttype) || !IS_SPEC (righttype) || SPEC_USIGN (righttype) || right->aop->type == AOP_CRY)
    {
      genMove_o (result->aop, 0, right->aop, 0, right->aop->size, !surviving_a, isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic), true);
      surviving_a |= (result->aop->regs[A_IDX] >= 0 && result->aop->regs[A_IDX] < right->aop->size);
      bool hl_dead = isPairDead (PAIR_HL, ic) && (result->aop->regs[L_IDX] < 0 || result->aop->regs[L_IDX] >= right->aop->size) && (result->aop->regs[H_IDX] < 0 || result->aop->regs[H_IDX] >= right->aop->size);
      bool de_dead = isPairDead (PAIR_DE, ic) && (result->aop->regs[E_IDX] < 0 || result->aop->regs[E_IDX] >= right->aop->size) && (result->aop->regs[D_IDX] < 0 || result->aop->regs[D_IDX] >= right->aop->size);
      bool iy_dead = isPairDead (PAIR_DE, ic) && (result->aop->regs[IYL_IDX] < 0 || result->aop->regs[IYL_IDX] >= right->aop->size) && (result->aop->regs[IYH_IDX] < 0 || result->aop->regs[IYH_IDX] >= right->aop->size);
      genMove_o (result->aop, right->aop->size, ASMOP_ZERO, 0, size, !surviving_a, hl_dead, de_dead, iy_dead, true);
    }
  else
    {
      bool maskedtopbyte = IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8) && SPEC_USIGN (resulttype);
      genMove_o (result->aop, 0, right->aop, 0, right->aop->size - 1, !surviving_a, isPairDead (PAIR_HL, ic), isPairDead (PAIR_DE, ic), isPairDead (PAIR_IY, ic), true);
      bool de_dead = isPairDead (PAIR_DE, ic) && (result->aop->regs[E_IDX] < 0 || result->aop->regs[E_IDX] >= right->aop->size) && (result->aop->regs[D_IDX] < 0 || result->aop->regs[D_IDX] >= right->aop->size);
      bool iy_dead = isPairDead (PAIR_DE, ic) && (result->aop->regs[IYL_IDX] < 0 || result->aop->regs[IYL_IDX] >= right->aop->size) && (result->aop->regs[IYH_IDX] < 0 || result->aop->regs[IYH_IDX] >= right->aop->size);
      bool hl_dead = isPairDead (PAIR_HL, ic) && (result->aop->regs[L_IDX] < 0 || result->aop->regs[L_IDX] >= right->aop->size) && (result->aop->regs[H_IDX] < 0 || result->aop->regs[H_IDX] >= right->aop->size);
      if (result->aop->type == AOP_REG && right->aop->type == AOP_REG && // Overwritten last byte of right operand
        result->aop->regs[right->aop->aopu.aop_reg[right->aop->size - 1]->rIdx] >= 0 && result->aop->regs[right->aop->aopu.aop_reg[right->aop->size - 1]->rIdx] < right->aop->size - 1)
        UNIMPLEMENTED;
      int offset = right->aop->size - 1;
      surviving_a |= (result->aop->regs[A_IDX] >= 0 && result->aop->regs[A_IDX] < offset);
      if (surviving_a && !pushed_a)
        _push (PAIR_AF), pushed_a = true;

      genMove_o (ASMOP_A, 0, right->aop, offset, 1, true, hl_dead, de_dead, iy_dead, true);
      if (right->aop->type != AOP_REG || result->aop->type != AOP_REG || right->aop->aopu.aop_reg[offset] != result->aop->aopu.aop_reg[offset])
        cheapMove (result->aop, offset, ASMOP_A, 0, true);
      offset++;
      
      surviving_a |= (result->aop->regs[A_IDX] >= 0 && result->aop->regs[A_IDX] < offset);
      if (surviving_a && !pushed_a)
        _push (PAIR_AF), pushed_a = true;

      /* we need to extend the sign */
      emit3 (A_RLCA, 0, 0);

      if (!IS_SM83 && !maskedtopbyte && isPairDead (PAIR_HL, ic) && size == 2 && /* writing AOP_HL is so cheap, it is not worth the 2-byte sbc hl, hl here */
        (aopInReg (result->aop, offset, HL_IDX) || result->aop->type == AOP_IY || (IS_RAB || IS_TLCS90 || IS_EZ80_Z80) && result->aop->type == AOP_STK))
        {
          emit2 ("sbc hl, hl");
          cost2 (2, 15, 10, 4, 0, 8, 2, 2);
          spillPair (PAIR_HL);
          genMove_o (result->aop, offset, ASMOP_HL, 0, 2, true, true, isPairDead (PAIR_DE, ic), true, false);
        }
      else
        {
          emit3 (A_SBC, ASMOP_A, ASMOP_A);
          while (size--)
            {
              if (!size && maskedtopbyte) // For casts from signed integers to wider unsigned _BitInt
                {
                  emit2 ("and a, #0x%02x", topbytemask);
                  cost2 (2, 7, 6, 4, 8, 4, 2, 2);
                }
              cheapMove (result->aop, offset++, ASMOP_A, 0, true);
            }
        }
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
  aopOp (result, ic, true, false);
  
  wassert (currFunc && ic->argreg);

  bool dead_regs[IYH_IDX + 1];
  
  for (int i = 0; i <= IYH_IDX; i++)
    dead_regs[i] = isRegDead (i, ic);

  for(iCode *nic = ic->next; nic && nic->op == RECEIVE; nic = nic->next)
    {
      asmop *narg = aopArg (currFunc->type, nic->argreg);
      wassert (narg);
      for (int i = 0; i < narg->size; i++)
        dead_regs[narg->aopu.aop_reg[i]->rIdx] = false;
    }
    
  if (result->aop->type == AOP_REG)
    for (int i = 0; i < result->aop->size; i++)
      if (!dead_regs[result->aop->aopu.aop_reg[i]->rIdx])
        UNIMPLEMENTED;

  genMove (result->aop, aopArg (currFunc->type, ic->argreg), dead_regs[A_IDX], dead_regs[L_IDX] && dead_regs[H_IDX], dead_regs[E_IDX] && dead_regs[D_IDX], true);

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
      size = op->aop->size;
      offset = 0;

      while (size--)
        {
          _moveA3 (op->aop, offset);
          offset++;
        }

      freeAsmop (op, NULL);
    }

  op = IC_LEFT (ic);
  if (op && IS_SYMOP (op))
    {
      aopOp (op, ic, FALSE, FALSE);

      /* general case */
      size = op->aop->size;
      offset = 0;

      while (size--)
        {
          _moveA3 (op->aop, offset);
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

  if (IS_SM83 || IS_RAB || IS_TLCS90)
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
          if (z80_opts.nmosZ80)
            {
              emit2 ("call ___sdcc_critical_enter");
            }
          else
            {
              //get interrupt enable flag IFF2 into P/O
              emit2 ("ld a,i");
              //disable interrupt
              emit2 ("!di");
            }
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
      if (z80_opts.nmosZ80)
        emit2 ("call ___sdcc_critical_enter");
      else
        {
          //get interrupt enable flag IFF2 into P/O
          emit2 ("ld a,i");
          //disable interrupt
          emit2 ("!di");
        }
      regalloc_dry_run_cost += 3;
      //save P/O flag
      if (!regalloc_dry_run)    // _push unbalances _G.stack.pushed.
        _push (PAIR_AF);
      else
        cost2 (1, 11, 11, 10, 16, 8, 3, 4);
    }
}

/*-----------------------------------------------------------------*/
/* genEndCritical - generate code for end of a critical sequence   */
/*-----------------------------------------------------------------*/
static void
genEndCritical (const iCode * ic)
{
  symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);

  if (IS_SM83 || IS_TLCS90 || IS_RAB)
    {
      emit2 ("!ei");
      cost2 (1, 4, 3, 0, 4, 2, 1, 1);
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
      sprintf (line, ".db !immed%d", self->buffer[0]);

      for (i = 1; i < self->pos; i++)
        {
          sprintf (line + strlen (line), ", !immed%d", self->buffer[i]);
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

      emit2 (".db !immed%u", self->pos);

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
          emit2 ("!db !immed-%u, !immedbyte", self->runLen, self->last);
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

          emit2 ("!db !immed-%u, !immedbyte", self->runLen, self->last);
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
  sym_link *type;
  RLECTX rle;
  bool isBool = FALSE;
  bool isFloat = FALSE;
  bool saved_BC = FALSE;
  bool saved_DE = FALSE;
  bool saved_HL = FALSE;

  memset (&rle, 0, sizeof (rle));

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);

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
  if (!isPairDead (PAIR_BC, ic))
    {
      _push (PAIR_BC);
      saved_BC = TRUE;
    }

  fetchPair (PAIR_DE, IC_LEFT (ic)->aop);
  emit2 ("call __initrleblock");

  type = operandType (IC_LEFT (ic));

  if (type && type->next)
    {
      if (IS_SPEC (type->next) || IS_PTR (type->next))
        {
          elementSize = getSize (type->next);
          isBool = IS_BOOL (type->next);
          isFloat = IS_FLOAT (type->next);
        }
      else if (IS_ARRAY (type->next) && type->next->next)
        {
          elementSize = getSize (type->next->next);
          isBool = IS_BOOL (type->next->next);
          isFloat = IS_FLOAT (type->next->next);
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

  wassertl ((elementSize > 0) && (elementSize <= 8), "Illegal element size in genArrayInit.");

  iLoop = IC_ARRAYILIST (ic);

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
          union
            {
              unsigned char c[sizeof(unsigned long long)];
              float f;
              unsigned long long ull;
            }
            buf;
            if (isFloat)
              {
                if (iLoop->isFloat)
                  buf.f = iLoop->value.f64;
                else
                  buf.f = iLoop->value.ull;
              }
            else
              {
                if (iLoop->isFloat)
                  buf.ull = (isBool) ? !!iLoop->value.f64 : (unsigned long long)iLoop->value.f64;
                else
                  buf.ull = (isBool) ? !!iLoop->value.ull : iLoop->value.ull;
              }
            
#ifdef WORDS_BIGENDIAN
          for (eIndex = elementSize-1; eIndex >= 0; eIndex--)
#else
          for (eIndex = 0; eIndex < elementSize; eIndex++)
#endif
            _rleAppend (&rle, buf.c[eIndex]);
        }

      iLoop = iLoop->next;
    }

  _rleFlush (&rle);
  /* Mark the end of the run. */
  emit2 (".db !zero");

  spillCached ();

  if (saved_BC)
    _pop (PAIR_BC);

  if (saved_DE)
    _pop (PAIR_DE);

  if (saved_HL)
    _pop (PAIR_HL);

  freeAsmop (IC_LEFT (ic), NULL);
}

static void
setupForMemcpy (const iCode *ic, const operand *to, const operand *from, const operand *count)
{
  /* Both are in regs. Let regMove() do the shuffling. */
  if (to->aop->type == AOP_REG && from->aop->type == AOP_REG)
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
  else if (to->aop->type == AOP_REG && count && count->aop->type == AOP_REG)
    {
      const short larray[4] = {E_IDX, D_IDX, C_IDX, B_IDX};
      short oparray[4];
      oparray[0] = to->aop->aopu.aop_reg[0]->rIdx;
      oparray[1] = to->aop->aopu.aop_reg[1]->rIdx;
      oparray[2] = count->aop->aopu.aop_reg[0]->rIdx;
      oparray[3] = count->aop->aopu.aop_reg[1]->rIdx;

      regMove (larray, oparray, 4 , false);

      genMove (ASMOP_HL, from->aop, isRegDead (A_IDX, ic), true, false, isRegDead (IY_IDX, ic));
    }
  else if (from->aop->type == AOP_REG && count && count->aop->type == AOP_REG)
    {
      const short larray[4] = {L_IDX, H_IDX, C_IDX, B_IDX};
      short oparray[4];
      oparray[0] = from->aop->aopu.aop_reg[0]->rIdx;
      oparray[1] = from->aop->aopu.aop_reg[1]->rIdx;
      oparray[2] = count->aop->aopu.aop_reg[0]->rIdx;
      oparray[3] = count->aop->aopu.aop_reg[1]->rIdx;

      regMove (larray, oparray, 4 , false);

      genMove (ASMOP_DE, to->aop, isRegDead (A_IDX, ic), false, true, isRegDead (IY_IDX, ic));
    }
  else if (count && count->aop->type == AOP_REG)
    {
      fetchPair (PAIR_BC, count->aop);
      fetchPair (PAIR_DE, to->aop);
      genMove (ASMOP_HL, from->aop, isRegDead (A_IDX, ic), true, false, isRegDead (IY_IDX, ic));
    }
  else
    {
      /* DE is free. Write it first. */
      if (from->aop->type != AOP_REG || from->aop->aopu.aop_reg[0]->rIdx != E_IDX && from->aop->aopu.aop_reg[0]->rIdx != D_IDX && from->aop->aopu.aop_reg[1]->rIdx != E_IDX && from->aop->aopu.aop_reg[1]->rIdx != D_IDX)
        {
          bool a_free = isRegDead (A_IDX, ic) && !aopInReg (from->aop, 0, A_IDX) && !aopInReg (from->aop, 1, A_IDX);
          bool iy_free = isRegDead (IY_IDX, ic) && !aopInReg (from->aop, 0, IYL_IDX) && !aopInReg (from->aop, 1, IYH_IDX) && !aopInReg (from->aop, 0, IYL_IDX) && !aopInReg (from->aop, 1, IYH_IDX);
          genMove (ASMOP_DE, to->aop, a_free, from->aop->regs[L_IDX] < 0 && from->aop->regs[H_IDX] < 0, true, iy_free);
          genMove (ASMOP_HL, from->aop, isRegDead (A_IDX, ic), true, false, isRegDead (IY_IDX, ic));
        }
      /* HL is free. Write it first. */
      else if (to->aop->type != AOP_REG || to->aop->aopu.aop_reg[0]->rIdx != L_IDX && to->aop->aopu.aop_reg[0]->rIdx != H_IDX && to->aop->aopu.aop_reg[1]->rIdx != L_IDX && to->aop->aopu.aop_reg[1]->rIdx != H_IDX)
        {
          bool a_free = isRegDead (A_IDX, ic) && !aopInReg (to->aop, 0, A_IDX) && !aopInReg (to->aop, 1, A_IDX);
          bool iy_free = isRegDead (IY_IDX, ic) && !aopInReg (to->aop, 0, IYL_IDX) && !aopInReg (to->aop, 1, IYH_IDX) && !aopInReg (to->aop, 0, IYL_IDX) && !aopInReg (to->aop, 1, IYH_IDX);
          genMove (ASMOP_HL, from->aop, a_free, true, to->aop->regs[E_IDX] < 0 && to->aop->regs[D_IDX] < 0, iy_free);
          genMove (ASMOP_DE, to->aop, isRegDead (A_IDX, ic), false, true, isRegDead (IY_IDX, ic));
        }
      /* L is free, but H is not. */
      else if ((to->aop->type != AOP_REG || to->aop->aopu.aop_reg[0]->rIdx != L_IDX && to->aop->aopu.aop_reg[1]->rIdx != L_IDX) &&
        (from->aop->type != AOP_REG || from->aop->aopu.aop_reg[0]->rIdx != L_IDX && from->aop->aopu.aop_reg[1]->rIdx != L_IDX))
        {
          cheapMove (ASMOP_L, 0, from->aop, 0, true);
          fetchPair (PAIR_DE, to->aop);
          cheapMove (ASMOP_H, 0, from->aop, 1, true);
        }
      /* H is free, but L is not. */
      else
        {
          cheapMove (ASMOP_H, 0, from->aop, 1, true);
          fetchPair (PAIR_DE, to->aop);
          cheapMove (ASMOP_L, 0, from->aop, 0, true);
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

  wassertl (!IS_SM83, "Built-in memcpy() not available on sm83.");
  wassertl (nparams == 3, "Built-in memcpy() must have three parameters.");

  count = pparams[2];
  from = pparams[1];
  to = pparams[0];

  if (pparams[2]->aop->type != AOP_LIT)
    n = UINT_MAX;
  else if (!(n = (unsigned int) ulFromVal (pparams[2]->aop->aopu.aop_lit))) /* Check for zero length copy. */
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
      emit2 ("ld a, !*hl");
      cost2 (1, 7, 6, 5, 8, 6, 2, 2);
      emit2 ("ld !mems, a", "de");
      cost2 (1, 7, 7, 7, 8, 6, 2, 2);
    }
  else if (n == 2)
    {
      emit2 ("ldi");
      cost2 (2, 16, 12, 10, 0, 14, 5, 4);
      emit2 ("ld a, !*hl");
      cost2 (1, 7, 6, 5, 8, 6, 2, 2);
      emit2 ("ld !mems, a", "de");
      cost2 (1, 7, 7, 7, 8, 6, 2, 2);
      if (!isPairDead (PAIR_BC, ic)) /* Restore bc. */
        emit3w (A_INC, ASMOP_BC, 0);
    }
  else if (n <= 4 && IS_Z80 && optimize.codeSpeed || (IS_R2K || IS_R2KA) && n <= 5)
    {
      for(unsigned int i = 0; i < n; i++)
        {
          emit2 ("ldi");
          cost2 (2, 16, 12, 10, 0 , 14, 5, 4);
        }
    }
  else
    {
      symbol *tlbl = 0;
      bool to_from_stack = isOperandOnStack (to) && isOperandOnStack (from);
      if (count->aop->type != AOP_REG) // If in reg: Has been fetched early by setupForMemcpy() above.
        fetchPair (PAIR_BC, count->aop);
      if (count->aop->type != AOP_LIT)
        {
          emit3 (A_LD, ASMOP_A, ASMOP_B);
          emit3 (A_OR, ASMOP_A, ASMOP_C);
          if (!regalloc_dry_run)
            {
              tlbl = newiTempLabel (0);
              emit2 ("jp Z, !tlabel", labelKey2num (tlbl->key));
            }
          cost2 (3, 10, 6, 7, 12, 10, 3, 3); // For cycle cost, assume that n is non-zero.
        }
      if ((IS_R2K || IS_R2KA) && !to_from_stack && optimize.codeSpeed && n != UINT_MAX) // Work around Rabbit 2000 to Rabbit 3000 ldir wait state bug, but care for speed
        {
          wassert (n > 3);
          if (n % 2)
            {
              emit2 ("ldi");
              cost2 (2, 16, 12, 10, 0 , 14, 5, 4);
            }
          if (!regalloc_dry_run)
            {
              const symbol *tlbl2 = newiTempLabel (0);
              emitLabel (tlbl2);
              emit2("ldi");
              cost2 (2, 16, 12, 10, 0 , 14, 5, 4);
              emit2("ldi");
              cost2 (2, 16, 12, 10, 0 , 14, 5, 4);
              emit2 ("jp LO, !tlabel", labelKey2num (tlbl2->key));
            }
          regalloc_dry_run_cost += 3;         
        }
      else if ((IS_R2K || IS_R2KA) && !to_from_stack) // Work around Rabbit 2000 to Rabbit 3000 ldir wait state bug.
        {
          if (!regalloc_dry_run)
            {
              const symbol *tlbl2 = newiTempLabel (0);
              emitLabel (tlbl2);
              emit2("ldi");
              cost2 (2, 16, 12, 10, 0 , 14, 5, 4);
              emit2 ("jp LO, !tlabel", labelKey2num (tlbl2->key));
            }
          regalloc_dry_run_cost += 3;
        }
      else
        {
          emit2 ("ldir");
          regalloc_dry_run_cost += 2;
        }
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
  if (dst->aop->type == AOP_REG && !direct_c && c->aop->type == AOP_REG)
    {
      const short larray[2] = {L_IDX, H_IDX};
      short oparray[2];
      bool early_a = c->aop->type == AOP_REG && (c->aop->aopu.aop_reg[0]->rIdx == L_IDX || c->aop->aopu.aop_reg[0]->rIdx == H_IDX);

      if (early_a)
        cheapMove (ASMOP_A, 0, c->aop, 0, true);

      oparray[0] = dst->aop->aopu.aop_reg[0]->rIdx;
      oparray[1] = dst->aop->aopu.aop_reg[1]->rIdx;

      regMove (larray, oparray, 2, early_a);

      if (!early_a)
        cheapMove (ASMOP_A, 0, c->aop, 0, true);
    }
  else if (c->aop->type == AOP_REG && requiresHL (c->aop))
    {
      cheapMove (ASMOP_A, 0, c->aop, 0, true);
      if (dst->aop->type == AOP_EXSTK)
        _push (PAIR_AF);
      fetchPair (PAIR_HL, dst->aop);
      if (dst->aop->type == AOP_EXSTK)
        _pop (PAIR_AF);
    }
  else
    {
      bool a_free = isRegDead (A_IDX, ic) && !aopInReg (c->aop, 0, A_IDX);
      genMove (ASMOP_HL, dst->aop, a_free, true, false, false);
      if (!direct_c)
        {
          if (requiresHL (c->aop))
            _push (PAIR_HL);
          cheapMove (ASMOP_A, 0, c->aop, 0, true);
          if (requiresHL (c->aop))
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
  bool live_BC = !isPairDead (PAIR_BC, ic), live_DE = !isPairDead (PAIR_DE, ic), live_HL = !isPairDead (PAIR_HL, ic), live_B = !isRegDead (B_IDX, ic);
  bool saved_BC = FALSE, saved_DE = FALSE, saved_HL = FALSE;

  wassertl (nParams == 3, "Built-in memset() must have three parameters");

  dst = pparams[0];
  c = pparams[1];
  n = pparams[2];

  aopOp (c, ic, FALSE, FALSE);
  aopOp (dst, ic, FALSE, FALSE);
  aopOp (n, ic, FALSE, FALSE);

  wassertl (n->aop->type == AOP_LIT, "Last parameter to builtin memset() must be literal.");

  if(n->aop->type != AOP_LIT || !(size = ulFromVal (n->aop->aopu.aop_lit)))
    goto done;

  direct_c = (c->aop->type == AOP_LIT || c->aop->type == AOP_REG &&
              c->aop->aopu.aop_reg[0]->rIdx != H_IDX && c->aop->aopu.aop_reg[0]->rIdx != L_IDX &&
              c->aop->aopu.aop_reg[0]->rIdx != IYH_IDX && c->aop->aopu.aop_reg[0]->rIdx != IYL_IDX);
  direct_cl = (c->aop->type == AOP_LIT || c->aop->type == AOP_REG &&
              c->aop->aopu.aop_reg[0]->rIdx != H_IDX && c->aop->aopu.aop_reg[0]->rIdx != L_IDX &&
              c->aop->aopu.aop_reg[0]->rIdx != IYH_IDX && c->aop->aopu.aop_reg[0]->rIdx != IYL_IDX &&
              c->aop->aopu.aop_reg[0]->rIdx != B_IDX);
  indirect_c = IS_R3KA && ulFromVal (n->aop->aopu.aop_lit) > 1 && c->aop->type == AOP_IY;

  double_loop = (size > 255 || optimize.codeSpeed);

  int sizecost_ld_a_caop = aopInReg (c->aop, 0, A_IDX) ? 0 : ld_cost (ASMOP_A, 0, c->aop, 0, false);
  sizecost_direct = 3 + 2 * size - 1 + !direct_c * sizecost_ld_a_caop;
  sizecost_direct += (live_HL) * 2;
  sizecost_loop = 9 + double_loop * 2 + ((size % 2) && double_loop) * 2 + !direct_cl * sizecost_ld_a_caop;
  sizecost_loop += (live_HL + live_B) * 2;
  sizecost_ldir = indirect_c ? 11 : (12 + !direct_c * sizecost_ld_a_caop - (IS_R3KA && !optimize.codeSpeed));
  sizecost_ldir += (live_HL + live_DE + live_BC) * 2;

  if (sizecost_direct <= sizecost_loop && sizecost_direct < sizecost_ldir) // straight-line code.
    {
      if (live_HL)
        {
          _push (PAIR_HL);
          saved_HL = TRUE;
        }

      setupForMemset (ic, dst, c, direct_c);
      
      while (size--)
        {
		  if (!regalloc_dry_run)
            emit2 ("ld !*hl, %s", aopGet (direct_c ? c->aop : ASMOP_A, 0, FALSE));
          cost2 (1, 7, 7, 6, 8, 6, 2, 2);
          if (size)
            emit3w (A_INC, ASMOP_HL, 0);
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
      if (!isRegDead (B_IDX, ic))
        {
          _push (PAIR_BC);
          saved_BC = TRUE;
        }

      setupForMemset (ic, dst, c, direct_cl);

      emit2 ("ld b, !immedbyte", double_loop ? (size / 2 + size % 2) : size);
      cost2 (2, 7, 6, 4, 8, 4, 2, 2);

      if (double_loop && size % 2)
        {
          if (!regalloc_dry_run)
            emit2 ("jr !tlabel", labelKey2num (tlbl2->key));
          regalloc_dry_run_cost += 2;
        }

      if (!regalloc_dry_run)
        {
          emitLabel (tlbl1);
          emit2 ("ld !*hl, %s", aopGet (direct_cl ? c->aop : ASMOP_A, 0, FALSE));
          emit2 ("inc hl");
          if (double_loop)
            {
              if (size % 2)
                emitLabel (tlbl2);
              emit2 ("ld !*hl, %s", aopGet (direct_cl ? c->aop : ASMOP_A, 0, FALSE));
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
          saved_HL = true;
        }
      if (live_DE)
        {
          _push (PAIR_DE);
          saved_DE = true;
        }
      if (live_BC)
        {
          _push (PAIR_BC);
          saved_BC = true;
        }
      if (indirect_c)
        {
          fetchPair (PAIR_DE, dst->aop);
          pointPairToAop (PAIR_HL, c->aop, 0);
        }
      else
        {
          setupForMemset (ic, dst, c, direct_c);

          if (!regalloc_dry_run)
            emit2 ("ld !*hl, %s", aopGet (direct_c ? c->aop : ASMOP_A, 0, FALSE));
          regalloc_dry_run_cost += (direct_c && c->aop->type == AOP_LIT) ? 2 : 1;
          if (ulFromVal (n->aop->aopu.aop_lit) <= 1)
            goto done;

          emit3 (A_LD, ASMOP_E, ASMOP_L);
          emit3 (A_LD, ASMOP_D, ASMOP_H);
          if (!IS_R3KA || optimize.codeSpeed)
            {
              emit2 ("inc de");
              cost2 (1, 6, 4, 2, 8, 4, 1, 1);
              preinc = true;
            }
        }
      emit2 ("ld bc, !immedword", (unsigned)(size - preinc));
      cost2 (3, 10, 9, 6, 12, 6, 3, 3);
      // The Rabbit 2000 to Rabbit 3000 (i.e. r2ka and r2ka port) have a ldir wait state bug that affects copies between different types of memory.
      // That is not a problem here, as we copy within an object, and thus within one memory.
      emit2 (IS_R3KA ? "lsidr" : "ldir");
      regalloc_dry_run_cost += 2;
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

  SomethingReturned = IS_ITEMP (IC_RESULT (ic)) && (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir) ||
                      IS_TRUE_SYMOP (IC_RESULT (ic));

  wassertl (nParams == 2, "Built-in strcpy() must have two parameters.");
  wassertl (!IS_SM83, "Built-in strcpy() not available for sm83.");

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
      emit2 ("cp a, !*hl");
      emit2 ("ldi");
      emit2 ("jr NZ, !tlabel", labelKey2num (tlbl->key));
    }
  regalloc_dry_run_cost += 5;

  spillPair (PAIR_HL);

  if (SomethingReturned)
    aopOp (IC_RESULT (ic), ic, true, false);

  if (!SomethingReturned || SomethingReturned && getPairId (IC_RESULT (ic)->aop) != PAIR_INVALID)
    {
      if (SomethingReturned)
        _pop (getPairId (IC_RESULT (ic)->aop));
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
      genMove (IC_RESULT (ic)->aop, ASMOP_HL, true, true, true, true);

      restoreRegs (0, saved_DE, saved_BC, saved_HL, IC_RESULT (ic), ic);
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

  wassertl (!IS_SM83, "Built-in strncpy() not available on sm83.");
  wassertl (nparams == 3, "Built-in strncpy() must have three parameters.");
  wassertl (pparams[2]->aop->type == AOP_LIT, "Last parameter to builtin strncpy() must be literal.");

  s1 = pparams[0];
  s2 = pparams[1];
  n = pparams[2];

  if (!ulFromVal (n->aop->aopu.aop_lit))
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

  fetchPair (PAIR_BC, n->aop);

  emit3 (A_XOR, ASMOP_A, ASMOP_A);
  if (!regalloc_dry_run)
    {
      symbol *tlbl1 = newiTempLabel (0);
      symbol *tlbl2 = newiTempLabel (0);
      symbol *tlbl3 = newiTempLabel (0);
      emitLabel (tlbl2);
      emit2 ("cp a,!*hl");
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

  restoreRegs (0, saved_DE, saved_BC, saved_HL, 0, ic);

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

  SomethingReturned = IS_ITEMP (IC_RESULT (ic)) && (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir) ||
                      IS_TRUE_SYMOP (IC_RESULT (ic));

  wassertl (nParams == 2, "Built-in strchr() must have two parameters.");

  s = pparams[0];
  c = pparams[1];

  for (i = 0; i < nParams; i++)
    aopOp (pparams[i], ic, FALSE, FALSE);

  if (SomethingReturned)
    aopOp (IC_RESULT (ic), ic, true, false);

  if (getPairId (s->aop) != PAIR_INVALID && getPairId (s->aop) != PAIR_IY)
    pair = getPairId (s->aop);
  else if (SomethingReturned && getPairId (IC_RESULT (ic)->aop) != PAIR_INVALID && getPairId (IC_RESULT (ic)->aop) != PAIR_IY)
    pair = getPairId (IC_RESULT (ic)->aop);
  else
    pair = PAIR_HL;

  if (c->aop->type == AOP_REG && c->aop->aopu.aop_reg[0]->rIdx != IYL_IDX && c->aop->aopu.aop_reg[0]->rIdx != IYH_IDX &&
    c->aop->aopu.aop_reg[0]->rIdx != A_IDX &&
    !(pair == PAIR_HL && (c->aop->aopu.aop_reg[0]->rIdx == L_IDX || c->aop->aopu.aop_reg[0]->rIdx == H_IDX)) &&
    !(pair == PAIR_DE && (c->aop->aopu.aop_reg[0]->rIdx == E_IDX || c->aop->aopu.aop_reg[0]->rIdx == D_IDX)) &&
    !(pair == PAIR_BC && (c->aop->aopu.aop_reg[0]->rIdx == B_IDX || c->aop->aopu.aop_reg[0]->rIdx == C_IDX)))
    direct_c = true;
  else if (c->aop->type == AOP_LIT && optimize.codeSize)
    direct_c = true;
  else
    direct_c = false;

  aop_c = direct_c ? c->aop : (pair == PAIR_DE ? ASMOP_H : ASMOP_D);

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
    cheapMove (aop_c, 0, c->aop, 0, true);
  fetchPair (pair, s->aop);

  if (!isRegDead (A_IDX, ic))
    UNIMPLEMENTED;

  if (!regalloc_dry_run)
    emitLabel (tlbl2);
  emit2 ("ld a, !mems", _pairs[pair].name);
  cost2 (1, 7, 6, 6, 8, 6, 2, 2);
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
    commitPair (IC_RESULT (ic)->aop, pair, ic, FALSE);

  restoreRegs (0, saved_DE, saved_BC, saved_HL, SomethingReturned ? IC_RESULT (ic) : 0, ic);

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
  if (resultRemat (ic))
    {
      emitDebug ("; skipping iCode since result will be rematerialized");
      return;
    }

  if (ic->generated)
    {
      emitDebug ("; skipping generated iCode");
      return;
    }

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

    case IPUSH_VALUE_AT_ADDRESS:
      emitDebug ("; genPointerPush");
      genPointerPush (ic);
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
      genMinus (ic, ic->next->op == IFX ? ic->next : 0);
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

    case GETABIT:
      emitDebug ("; genGetAbit");
      genGetAbit (ic);
      break;

    case GETBYTE:
      emitDebug ("; genGetByte");
      genGetByte (ic);
      break;

    case GETWORD:
      emitDebug ("; genGetWord");
      genGetWord (ic);
      break;

    case ROT:
      emitDebug ("; genRot");
      genRot (ic);
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
          emitDebug ("; genPointerSet");
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

    case ARRAYINIT:
      emitDebug ("; genArrayInit");
      genArrayInit (ic);
      break;

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
      wassertl (0, "Unknown iCode");
    }
}

float
dryZ80iCode (iCode * ic)
{
  regalloc_dry_run = true;
  regalloc_dry_run_cost = 0;
  regalloc_dry_run_cost_bytes = 0;
  regalloc_dry_run_cost_states = 0;

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

  const unsigned int state_cost_divider = 8u << (optimize.codeSize * 3 + !optimize.codeSpeed * 3);

  // Hack, since the legacy regalloc_dry_run_cost is still used in some places.
  regalloc_dry_run_cost_bytes += regalloc_dry_run_cost;
  regalloc_dry_run_cost_states += regalloc_dry_run_cost * 4; // Assume 4 states per byte.

  // Compensate for typically lower state count of some targets
  if (IS_RAB)
    regalloc_dry_run_cost_states *= 2;
  else if (IS_EZ80_Z80 || IS_R800)
    regalloc_dry_run_cost_states *= 3;

  return (regalloc_dry_run_cost_bytes + regalloc_dry_run_cost_states * ic->count / state_cost_divider);
}

#ifdef DEBUG_DRY_COST
static void
dryZ80Code (iCode * lic)
{
  iCode *ic;

  for (ic = lic; ic; ic = ic->next)
    if (ic->op != FUNCTION && ic->op != ENDFUNCTION && ic->op != LABEL && ic->op != GOTO && ic->op != INLINEASM)
      {
        printf ("; iCode %d total cost: %f ", ic->key, dryZ80iCode (ic));
        const unsigned int state_cost_divider = 8u << (optimize.codeSize * 3 + !optimize.codeSpeed * 3);
        printf ("(%f + %f * %f * 0.0001 / %u\n", (float)regalloc_dry_run_cost_bytes, regalloc_dry_run_cost_states, ic->count, state_cost_divider);
      }
}
#endif

/*-------------------------------------------------------------------------------------*/
/* genZ80Code - generate code for Z80 based controllers for a block of instructions    */
/*-------------------------------------------------------------------------------------*/
void
genZ80Code (iCode * lic)
{
#ifdef DEBUG_DRY_COST
  dryZ80Code (lic);
#endif

  iCode *ic;
  int cln = 0;
  regalloc_dry_run = false;

  initGenLineElement ();

  memset(z80_regs_used_as_parms_in_calls_from_current_function, 0, sizeof(bool) * (IYH_IDX + 1));
  z80_symmParm_in_calls_from_current_function = TRUE;
  memset(z80_regs_preserved_in_calls_from_current_function, 0, sizeof(bool) * (IYH_IDX + 1));

  /* if debug information required */
  if (options.debug && currFunc)
    {
      debugFile->writeFunction (currFunc, lic);
    }

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
      //regalloc_dry_run_cost = 0;
      regalloc_dry_run_cost_bytes = 0;
      regalloc_dry_run_cost_states = 0;
      genZ80iCode (ic);

#if 0 // Helpful to debug "Unbalanced stack" errors.
      printf("After ic %d (op %d): _G.stack.pushed: %d\n", ic->key, ic->op, _G.stack.pushed);
#endif

#ifdef DEBUG_DRY_COST
      emit2 ("; iCode %d (count %f) total costs: %u %lu %f\n", ic->key, ic->count, regalloc_dry_run_cost, regalloc_dry_run_cost_bytes, regalloc_dry_run_cost_states);
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

// Check if what is returned by the curent function.
bool
z80IsReturned(const char *what)
{
  if (!strcmp(what, "iy"))
    return (z80IsReturned ("iyl") || z80IsReturned ("iyh"));

  const asmop *retaop = aopRet (currFunc->type);

  if (!retaop)
    return false;
  for (int i = 0; i < retaop->size; i++)
    if (!strcmp(retaop->aopu.aop_reg[i]->name, what))
      return true;
  return false;
}

// Check if what is part of the ith argument (counting from 1) to a function of type ftype.
// If what is 0, just check if the ith argument is in registers.
bool
z80IsRegArg(struct sym_link *ftype, int i, const char *what)
{
  if (what && !strcmp(what, "iy"))
    return (z80IsRegArg (ftype, i, "iyl") || z80IsRegArg (ftype, i, "iyh"));

  const asmop *argaop = aopArg (ftype, i);

  if (!argaop)
    return false;
    
  if (!what)
    return true;
    
  for (int i = 0; i < argaop->size; i++)
    if (!strcmp(argaop->aopu.aop_reg[i]->name, what))
      return true;

  return false; 
}

bool
z80IsParmInCall(sym_link *ftype, const char *what)
{
  const value *args;
  int i;

  for (i = 1, args = FUNC_ARGS (ftype); args; args = args->next, i++)
    if (z80IsRegArg(ftype, i, what))
      return true;
  return false;
}

