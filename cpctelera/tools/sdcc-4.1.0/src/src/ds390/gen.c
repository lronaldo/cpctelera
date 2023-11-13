/*-------------------------------------------------------------------------
  gen.c - source file for code generation for DS80C390

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Bug Fixes - Wojciech Stryjewski  wstryj1@tiger.lsu.edu (1999 v2.1.9a)
  DS390 adaptation:
  Copyright (C) 2000, Kevin Vigor <kevin@vigor.nu>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2, or (at your option) any
  later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
-------------------------------------------------------------------------*/

//#define D(x)
#define D(x) do if (options.verboseAsm) {x;} while(0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "main.h"
#include "ralloc.h"
#include "gen.h"
#include "dbuf_string.h"

extern int allocInfo;

/* this is the down and dirty file with all kinds of
   kludgy & hacky stuff. This is what it is all about
   CODE GENERATION for a specific MCU . some of the
   routines may be reusable, will have to see */

static char *zero = "#0x00";
static char *one = "#0x01";
static char *spname;

#define TR_DPTR(s) if (options.model != MODEL_FLAT24) { emitcode(";", " Use_DPTR1 %s ", s); }
#define TR_AP(s) if (options.model != MODEL_FLAT24) { emitcode(";", " Use_AP %s ", s); }

unsigned fReturnSizeDS390 = 9;
static char *fReturn24[] = { "dpl", "dph", "dpx", "b", "a", "r4", "r5", "r6", "r7" };
static char *fReturn16[] = { "dpl", "dph", "b", "a", "r4", "r5", "r6", "r7" };

static char **fReturn = fReturn24;
char **fReturnDS390 = fReturn24;
static char *accUse[] = { "a", "b" };

static char *dptrn[2][3];
static char *javaRet[] = { "r0", "r1", "r2", "r3" };

static short rbank = -1;

#define REG_WITH_INDEX   ds390_regWithIdx

#define AOP(op) op->aop
#define AOP_TYPE(op) AOP(op)->type
#define AOP_SIZE(op) AOP(op)->size
#define IS_AOP_PREG(x) (AOP(x) && (AOP_TYPE(x) == AOP_R1 || \
                        AOP_TYPE(x) == AOP_R0))

#define AOP_NEEDSACC(x) (AOP(x) && (AOP_TYPE(x) == AOP_CRY ||  \
                         AOP_TYPE(x) == AOP_DPTR || AOP_TYPE(x) == AOP_DPTR2 || \
                         AOP(x)->paged))

#define AOP_INPREG(x) (x && (x->type == AOP_REG &&                        \
                       (x->aopu.aop_reg[0] == REG_WITH_INDEX(R0_IDX) || \
                        x->aopu.aop_reg[0] == REG_WITH_INDEX(R1_IDX) )))
#define AOP_INDPTRn(x) (AOP_TYPE(x) == AOP_DPTRn)
#define AOP_USESDPTR(x) ((AOP_TYPE(x) == AOP_DPTR) || (AOP_TYPE(x) == AOP_STR))
#define AOP_USESDPTR2(x) ((AOP_TYPE(x) == AOP_DPTR2) || (AOP_TYPE(x) == AOP_DPTRn))

// The following macro can be used even if the aop has not yet been aopOp'd.
#define AOP_IS_DPTRn(x) (IS_SYMOP(x) && OP_SYMBOL(x)->dptr)

/* Workaround for DS80C390 bug: div ab may return bogus results
 * if A is accessed in instruction immediately before the div.
 *
 * Will be fixed in B4 rev of processor, Dallas claims.
 */

#define LOAD_AB_FOR_DIV(LEFT, RIGHT) \
  if (!AOP_NEEDSACC (RIGHT)) \
    { \
      /* We can load A first, then B, since \
       * B (the RIGHT operand) won't clobber A, \
       * thus avoiding touching A right before the div. \
       */  \
      D (emitcode (";", "DS80C390 div bug: rearranged ops.")); \
      MOVA (aopGet (LEFT, 0, FALSE, FALSE, NULL)); \
      MOVB (aopGet (RIGHT, 0, FALSE, FALSE, "b")); \
    } \
  else  \
    { \
      /* Just stuff in a nop after loading A. */ \
      emitcode ("mov", "b,%s", aopGet (RIGHT, 0, FALSE, FALSE, NULL)); \
      MOVA (aopGet (LEFT, 0, FALSE, FALSE, NULL)); \
      emitcode("nop", "; workaround for DS80C390 div bug."); \
    }

#define EQ(a, b)      (strcmp (a, b) == 0)

#define R0INB   _G.bu.bs.r0InB
#define R1INB   _G.bu.bs.r1InB
#define OPINB   _G.bu.bs.OpInB
#define BINUSE  _G.bu.BInUse

static struct
{
  short r0Pushed;
  short r1Pushed;
  union
  {
    struct
    {
      short r0InB: 2;           //2 so we can see it overflow
      short r1InB: 2;           //2 so we can see it overflow
      short OpInB: 2;           //2 so we can see it overflow
    } bs;
    short BInUse;
  } bu;
  short accInUse;
  short nRegsSaved;
  short dptrInUse;
  short dptr1InUse;
  set *sendSet;
  symbol *currentFunc;
}
_G;

static char *rb1regs[] =
{
  "b1_0", "b1_1", "b1_2", "b1_3", "b1_4", "b1_5", "b1_6", "b1_7",
  "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7"
};

static void saveRBank (int, iCode *, bool);

#define RESULTONSTACK(x) \
                         (IC_RESULT (x) && IC_RESULT (x)->aop && \
                         IC_RESULT (x)->aop->type == AOP_STK )

#define MOVA(x)  mova (x)       /* use function to avoid multiple eval */
#define MOVB(x)  movb (x)

#define CLRC     emitcode ("clr","c")
#define SETC     emitcode ("setb","c")

/* A scratch register which will be used to hold
 * result bytes from operands in far space via DPTR2. */
#define DP2_RESULT_REG  "acc1"

static unsigned char SLMask[] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0,
                                  0xE0, 0xC0, 0x80, 0x00
                                };

static unsigned char SRMask[] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F,
                                  0x07, 0x03, 0x01, 0x00
                                };

#define LSB     0
#define MSB16   1
#define MSB24   2
#define MSB32   3
#define PROTECT_SP      {                                                       \
                          if (options.protect_sp_update)                        \
                            {                                                   \
                              symbol *lbl = newiTempLabel (NULL);               \
                              emitcode ("setb", "F1");                          \
                              emitcode ("jbc", "EA,!tlabel", labelKey2num (lbl->key)); \
                              emitcode ("clr", "F1");                           \
                              emitLabel (lbl);                                  \
                            }                                                   \
                        }
#define UNPROTECT_SP    {                                                       \
                          if (options.protect_sp_update)                        \
                            {                                                   \
                                emitcode ("mov", "EA,F1");                      \
                            }                                                   \
                        }

static int _currentDPS;         /* Current processor DPS. */
static int _desiredDPS;         /* DPS value compiler thinks we should be using. */
static int _lazyDPS = 0;        /* if non-zero, we are doing lazy evaluation of DPS changes. */

/*-----------------------------------------------------------------*/
/* ds390_emitDebuggerSymbol - associate the current code location  */
/*   with a debugger symbol                                        */
/*-----------------------------------------------------------------*/
void
ds390_emitDebuggerSymbol (const char *debugSym)
{
  genLine.lineElement.isDebug = 1;
  emitcode ("", "%s ==.", debugSym);
  genLine.lineElement.isDebug = 0;
}

/*-----------------------------------------------------------------*/
/* mova - moves specified value into accumulator                   */
/*-----------------------------------------------------------------*/
static void
mova (const char *x)
{
  /* do some early peephole optimization */
  if (!strncmp (x, "a", 2) || !strncmp (x, "acc", 4))
    return;

  /* another early peephole optimization */
  if (EQ (x, "#0x00"))
    {
      emitcode ("clr", "a");
      return;
    }

  emitcode ("mov", "a,%s", x);
}

/*-----------------------------------------------------------------*/
/* movb - moves specified value into register b                    */
/*-----------------------------------------------------------------*/
static void
movb (const char *x)
{
  /* do some early peephole optimization */
  if (!strncmp (x, "b", 2))
    return;

  emitcode ("mov", "b,%s", x);
}

/*-----------------------------------------------------------------*/
/* emitpush - push something on internal stack                     */
/*-----------------------------------------------------------------*/
static void
emitpush (const char *arg)
{
  char buf[] = "ar?";

  if (!arg)
    {
      emitcode ("inc", "sp");
      return;
    }
  else if (EQ (arg, "a"))
    {
      arg = "acc";
    }
  else if ((*arg == '@') || (*arg == '#'))
    {
      MOVA (arg);
      arg = "acc";
    }
  else if (EQ (arg, "r0") || EQ (arg, "r1") || EQ (arg, "r2") || EQ (arg, "r3") ||
           EQ (arg, "r4") || EQ (arg, "r5") || EQ (arg, "r6") || EQ (arg, "r7"))
    {
      buf[2] = arg[1];
      arg = buf;
    }
  emitcode ("push", arg);
}

/*-----------------------------------------------------------------*/
/* emitpop - pop something from internal stack                     */
/*-----------------------------------------------------------------*/
static void
emitpop (const char *arg)
{
  if (!arg)
    emitcode ("dec", "sp");
  else
    emitcode ("pop", arg);
}

/*-----------------------------------------------------------------*/
/* pushB - saves register B if necessary                           */
/*-----------------------------------------------------------------*/
static bool
pushB (void)
{
  bool pushedB = FALSE;

  if (BINUSE)
    {
      emitpush ("b");
//    printf("B was in use !\n");
      pushedB = TRUE;
    }
  else
    {
      OPINB++;
    }
  return pushedB;
}

/*-----------------------------------------------------------------*/
/* popB - restores value of register B if necessary                */
/*-----------------------------------------------------------------*/
static void
popB (bool pushedB)
{
  if (pushedB)
    {
      emitpop ("b");
    }
  else
    {
      OPINB--;
    }
}

/*-----------------------------------------------------------------*/
/* pushReg - saves register                                        */
/*-----------------------------------------------------------------*/
static bool
pushReg (int index, bool bits_pushed)
{
  const reg_info *reg = REG_WITH_INDEX (index);
  if (reg->type == REG_BIT)
    {
      if (!bits_pushed)
        emitpush (reg->base);
      return TRUE;
    }
  else
    emitpush (reg->dname);
  return bits_pushed;
}

/*-----------------------------------------------------------------*/
/* popReg - restores register                                      */
/*-----------------------------------------------------------------*/
static bool
popReg (int index, bool bits_popped)
{
  const reg_info *reg = REG_WITH_INDEX (index);
  if (reg->type == REG_BIT)
    {
      if (!bits_popped)
        emitpop (reg->base);
      return TRUE;
    }
  else
    emitpop (reg->dname);
  return bits_popped;
}

/*-----------------------------------------------------------------*/
/* getFreePtr - returns r0 or r1 whichever is free or can be pushed */
/*-----------------------------------------------------------------*/
static reg_info *
getFreePtr (iCode * ic, asmop ** aopp, bool result)
{
  bool r0iu, r1iu;
  bool r0ou, r1ou;

  /* the logic: if r0 & r1 used in the instruction
     then we are in trouble otherwise */

  /* first check if r0 & r1 are used by this
     instruction, in which case we are in trouble */
  r0iu = bitVectBitValue (ic->rUsed, R0_IDX);
  r1iu = bitVectBitValue (ic->rUsed, R1_IDX);
  if (r0iu && r1iu)
    {
      goto endOfWorld;
    }

  r0ou = bitVectBitValue (ic->rMask, R0_IDX);
  r1ou = bitVectBitValue (ic->rMask, R1_IDX);

  /* if no usage of r0 then return it */
  if (!r0iu && !r0ou)
    {
      ic->rUsed = bitVectSetBit (ic->rUsed, R0_IDX);
      (*aopp)->type = AOP_R0;

      return (*aopp)->aopu.aop_ptr = REG_WITH_INDEX (R0_IDX);
    }

  /* if no usage of r1 then return it */
  if (!r1iu && !r1ou)
    {
      ic->rUsed = bitVectSetBit (ic->rUsed, R1_IDX);
      (*aopp)->type = AOP_R1;

      return (*aopp)->aopu.aop_ptr = REG_WITH_INDEX (R1_IDX);
    }

  /* now we know they both have usage */
  /* if r0 not used in this instruction */
  if (!r0iu)
    {
      /* push it if not already pushed */
      if (!_G.r0Pushed)
        {
          emitpush (REG_WITH_INDEX (R0_IDX)->dname);
          _G.r0Pushed++;
        }

      ic->rUsed = bitVectSetBit (ic->rUsed, R0_IDX);
      (*aopp)->type = AOP_R0;

      return (*aopp)->aopu.aop_ptr = REG_WITH_INDEX (R0_IDX);
    }

  /* if r1 not used then */

  if (!r1iu)
    {
      /* push it if not already pushed */
      if (!_G.r1Pushed)
        {
          emitpush (REG_WITH_INDEX (R1_IDX)->dname);
          _G.r1Pushed++;
        }

      ic->rUsed = bitVectSetBit (ic->rUsed, R1_IDX);
      (*aopp)->type = AOP_R1;
      return REG_WITH_INDEX (R1_IDX);
    }

endOfWorld:
  /* I said end of world, but not quite end of world yet */
  /* if this is a result then we can push it on the stack */
  if (result)
    {
      (*aopp)->type = AOP_STK;
      return NULL;
    }

  /* now this is REALLY the end of the world */
  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "getFreePtr should never reach here");
  exit (EXIT_FAILURE);

  return NULL;                  // notreached, but makes compiler happy.
}


/*-----------------------------------------------------------------*/
/* genSetDPTR: generate code to select which DPTR is in use (zero  */
/* selects standard DPTR (DPL/DPH/DPX), non-zero selects DS390     */
/* alternate DPTR (DPL1/DPH1/DPX1).                                */
/*-----------------------------------------------------------------*/
static void
genSetDPTR (int n)
{
  /* If we are doing lazy evaluation, simply note the desired
   * change, but don't emit any code yet.
   */
  if (_lazyDPS)
    {
      _desiredDPS = n;
      return;
    }

  if (!n)
    {
      emitcode ("mov", "dps,#0");
    }
  else
    {
      TR_DPTR ("#1");
      emitcode ("mov", "dps,#1");
    }
}

/*------------------------------------------------------------------*/
/* _startLazyDPSEvaluation: call to start doing lazy DPS evaluation */
/*                                                                  */
/* Any code that operates on DPTR (NB: not on the individual        */
/* components, like DPH) *must* call _flushLazyDPS() before using   */
/* DPTR within a lazy DPS evaluation block.                         */
/*                                                                  */
/* Note that aopPut and aopGet already contain the proper calls to  */
/* _flushLazyDPS, so it is safe to use these calls within a lazy    */
/* DPS evaluation block.                                            */
/*                                                                  */
/* Also, _flushLazyDPS must be called before any flow control       */
/* operations that could potentially branch out of the block.       */
/*                                                                  */
/* Lazy DPS evaluation is simply an optimization (though an         */
/* important one), so if in doubt, leave it out.                    */
/*------------------------------------------------------------------*/
static void
_startLazyDPSEvaluation (void)
{
  _currentDPS = 0;
  _desiredDPS = 0;
  _lazyDPS++;
}

/*------------------------------------------------------------------*/
/* _flushLazyDPS: emit code to force the actual DPS setting to the  */
/* desired one. Call before using DPTR within a lazy DPS evaluation */
/* block.                                                           */
/*------------------------------------------------------------------*/
static void
_flushLazyDPS (void)
{
  if (!_lazyDPS)
    {
      /* nothing to do. */
      return;
    }

  if (_desiredDPS != _currentDPS)
    {
      if (_desiredDPS)
        {
          emitcode ("inc", "dps");
        }
      else
        {
          emitcode ("dec", "dps");
        }
      _currentDPS = _desiredDPS;
    }
}

/*-----------------------------------------------------------------*/
/* _endLazyDPSEvaluation: end lazy DPS evaluation block.           */
/*                                                                 */
/* Forces us back to the safe state (standard DPTR selected).      */
/*-----------------------------------------------------------------*/
static void
_endLazyDPSEvaluation (void)
{
  _lazyDPS--;
  if (!_lazyDPS)
    {
      if (_currentDPS)
        {
          genSetDPTR (0);
          _flushLazyDPS ();
        }
      _currentDPS = 0;
      _desiredDPS = 0;
    }
}


/*-----------------------------------------------------------------*/
/* newAsmop - creates a new asmOp                                  */
/*-----------------------------------------------------------------*/
static asmop *
newAsmop (short type)
{
  asmop *aop;

  aop = Safe_calloc (1, sizeof (asmop));
  aop->type = type;
  aop->allocated = 1;
  return aop;
}

/*-----------------------------------------------------------------*/
/* pointerCode - returns the code for a pointer type               */
/*-----------------------------------------------------------------*/
static int
pointerCode (sym_link * etype)
{
  return PTR_TYPE (SPEC_OCLS (etype));
}

/*-----------------------------------------------------------------*/
/* leftRightUseAcc - returns size of accumulator use by operands   */
/*-----------------------------------------------------------------*/
static int
leftRightUseAcc (iCode * ic)
{
  operand *op;
  int size;
  int accuseSize = 0;
  int accuse = 0;

  if (!ic)
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "null iCode pointer");
      return 0;
    }

  if (ic->op == IFX)
    {
      op = IC_COND (ic);
      if (IS_OP_ACCUSE (op))
        {
          accuse = 1;
          size = getSize (OP_SYMBOL (op)->type);
          if (size > accuseSize)
            accuseSize = size;
        }
    }
  else if (ic->op == JUMPTABLE)
    {
      op = IC_JTCOND (ic);
      if (IS_OP_ACCUSE (op))
        {
          accuse = 1;
          size = getSize (OP_SYMBOL (op)->type);
          if (size > accuseSize)
            accuseSize = size;
        }
    }
  else
    {
      op = IC_LEFT (ic);
      if (IS_OP_ACCUSE (op))
        {
          accuse = 1;
          size = getSize (OP_SYMBOL (op)->type);
          if (size > accuseSize)
            accuseSize = size;
        }
      op = IC_RIGHT (ic);
      if (IS_OP_ACCUSE (op))
        {
          accuse = 1;
          size = getSize (OP_SYMBOL (op)->type);
          if (size > accuseSize)
            accuseSize = size;
        }
    }

  if (accuseSize)
    return accuseSize;
  else
    return accuse;
}

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (iCode * ic, symbol * sym, bool result, bool useDP2)
{
  asmop *aop;
  memmap *space;
  bool accuse = leftRightUseAcc (ic) || _G.accInUse;
  char *dpl = useDP2 ? "dpl1" : "dpl";
  char *dph = useDP2 ? "dph1" : "dph";
  char *dpx = useDP2 ? "dpx1" : "dpx";

  wassertl (ic != NULL, "Got a null iCode");
  wassertl (sym != NULL, "Got a null symbol");

  space = SPEC_OCLS (sym->etype);

  /* if already has one */
  if (sym->aop)
    {
      if ((sym->aop->type == AOP_DPTR && useDP2) || (sym->aop->type == AOP_DPTR2 && !useDP2))
        sym->aop = NULL;
      else
        {
          sym->aop->allocated++;
          return sym->aop;
        }
    }

  /* assign depending on the storage class */
  /* if it is on the stack or indirectly addressable */
  /* space we need to assign either r0 or r1 to it   */
  if ((sym->onStack && !options.stack10bit) || sym->iaccess)
    {
      sym->aop = aop = newAsmop (0);
      aop->aopu.aop_ptr = getFreePtr (ic, &aop, result);
      aop->size = getSize (sym->type);

      /* now assign the address of the variable to
         the pointer register */
      if (aop->type != AOP_STK)
        {
          if (sym->onStack)
            {
              signed char offset = ((sym->stack < 0) ?
                                    ((signed char) (sym->stack - _G.nRegsSaved)) : ((signed char) sym->stack)) & 0xff;

              if ((abs (offset) <= 3) || (accuse && (abs (offset) <= 7)))
                {
                  emitcode ("mov", "%s,_bp", aop->aopu.aop_ptr->name);
                  while (offset < 0)
                    {
                      emitcode ("dec", aop->aopu.aop_ptr->name);
                      offset++;
                    }
                  while (offset > 0)
                    {
                      emitcode ("inc", aop->aopu.aop_ptr->name);
                      offset--;
                    }
                }
              else
                {
                  if (accuse)
                    emitcode ("push", "acc");
                  emitcode ("mov", "a,_bp");
                  emitcode ("add", "a,#!constbyte", offset);
                  emitcode ("mov", "%s,a", aop->aopu.aop_ptr->name);
                  if (accuse)
                    emitcode ("pop", "acc");
                }
            }
          else
            {
              emitcode ("mov", "%s,#%s", aop->aopu.aop_ptr->name, sym->rname);
            }
          aop->paged = space->paged;
        }
      else
        aop->aopu.aop_stk = sym->stack;
      return aop;
    }

  if (sym->onStack && options.stack10bit)
    {
      short stack_val = -((sym->stack < 0) ? ((short) (sym->stack - _G.nRegsSaved)) : ((short) sym->stack));
      if (_G.dptrInUse)
        {
          emitcode ("push", dpl);
          emitcode ("push", dph);
          emitcode ("push", dpx);
        }
      /* It's on the 10 bit stack, which is located in
       * far data space.
       */
      if (stack_val < 0 && stack_val > -5)
        {
          /* between -5 & -1 */
          if (options.model == MODEL_FLAT24)
            {
              emitcode ("mov", "%s,#!constbyte", dpx, (options.stack_loc >> 16) & 0xff);
            }
          emitcode ("mov", "%s,_bpx+1", dph);
          emitcode ("mov", "%s,_bpx", dpl);
          if (useDP2)
            {
              emitcode ("mov", "dps,#1");
            }
          stack_val = -stack_val;
          while (stack_val--)
            {
              emitcode ("inc", "dptr");
            }
          if (useDP2)
            {
              emitcode ("mov", "dps,#0");
            }
        }
      else
        {
          if (accuse)
            emitcode ("push", "acc");

          emitcode ("mov", "a,_bpx");
          emitcode ("clr", "c");
          emitcode ("subb", "a,#!constbyte", stack_val & 0xff);
          emitcode ("mov", "%s,a", dpl);
          emitcode ("mov", "a,_bpx+1");
          emitcode ("subb", "a,#!constbyte", (stack_val >> 8) & 0xff);
          emitcode ("mov", "%s,a", dph);
          if (options.model == MODEL_FLAT24)
            {
              emitcode ("mov", "%s,#!constbyte", dpx, (options.stack_loc >> 16) & 0xff);
            }

          if (accuse)
            emitcode ("pop", "acc");
        }
      sym->aop = aop = newAsmop ((short) (useDP2 ? AOP_DPTR2 : AOP_DPTR));
      aop->size = getSize (sym->type);
      return aop;
    }

  /* if in bit space */
  if (IN_BITSPACE (space))
    {
      sym->aop = aop = newAsmop (AOP_CRY);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
      return aop;
    }
  /* if it is in direct space */
  if (IN_DIRSPACE (space))
    {
      sym->aop = aop = newAsmop (AOP_DIR);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
      return aop;
    }

  /* special case for a function */
  if (IS_FUNC (sym->type) && !(sym->isitmp))
    {
      sym->aop = aop = newAsmop (AOP_IMMD);
      aop->aopu.aop_immd.aop_immd1 = Safe_strdup (sym->rname);
      aop->size = getSize (sym->type);
      return aop;
    }

  /* only remaining is far space */
  /* in which case DPTR gets the address */
  sym->aop = aop = newAsmop ((short) (useDP2 ? AOP_DPTR2 : AOP_DPTR));
  if (useDP2)
    {
      genSetDPTR (1);
      _flushLazyDPS ();
      emitcode ("mov", "dptr,#%s", sym->rname);
      genSetDPTR (0);
    }
  else
    {
      emitcode ("mov", "dptr,#%s", sym->rname);
    }
  aop->size = getSize (sym->type);

  /* if it is in code space */
  if (IN_CODESPACE (space))
    aop->code = 1;

  return aop;
}

/*-----------------------------------------------------------------*/
/* aopForRemat - rematerializes an object                          */
/*-----------------------------------------------------------------*/
static asmop *
aopForRemat (symbol * sym)
{
  iCode *ic = sym->rematiCode;
  asmop *aop = newAsmop (AOP_IMMD);
  int ptr_type = 0;
  int val = 0;
  struct dbuf_s dbuf;

  for (;;)
    {
      if (ic->op == '+')
        val += (int) operandLitValue (IC_RIGHT (ic));
      else if (ic->op == '-')
        val -= (int) operandLitValue (IC_RIGHT (ic));
      else if (IS_CAST_ICODE (ic))
        {
          sym_link *from_type = operandType (IC_RIGHT (ic));
          aop->aopu.aop_immd.from_cast_remat = 1;
          ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
          ptr_type = pointerTypeToGPByte (DCL_TYPE (from_type),
                                          IS_SYMOP (IC_RIGHT (ic)) ? OP_SYMBOL (IC_RIGHT (ic))->name : NULL, sym->name);
          continue;
        }
      else
        break;

      ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
    }

  dbuf_init (&dbuf, 128);
  if (IS_ASSIGN_ICODE (ic) && isOperandLiteral (IC_RIGHT (ic)))
    {
      val = (val + (int) operandLitValue (IC_RIGHT (ic))) & 0xffffff;
      dbuf_printf (&dbuf, "0x%06x", val);
    }
  else if (val)
    {
      dbuf_printf (&dbuf, "(%s %c 0x%06x)", OP_SYMBOL (IC_LEFT (ic))->rname, val >= 0 ? '+' : '-', abs (val) & 0xffffff);
    }
  else
    {
      dbuf_append_str (&dbuf, OP_SYMBOL (IC_LEFT (ic))->rname);
    }

  aop->aopu.aop_immd.aop_immd1 = dbuf_detach_c_str (&dbuf);
  /* set immd2 field if required */
  if (aop->aopu.aop_immd.from_cast_remat)
    {
      dbuf_init (&dbuf, 128);
      dbuf_tprintf (&dbuf, "#!constbyte", ptr_type);
      aop->aopu.aop_immd.aop_immd2 = dbuf_detach_c_str (&dbuf);
    }

  return aop;
}

/*-----------------------------------------------------------------*/
/* aopHasRegs - returns true if aop has regs between from-to       */
/*-----------------------------------------------------------------*/
static int
aopHasRegs (asmop * aop, int from, int to)
{
  int size = 0;

  if (aop->type != AOP_REG)
    return 0;                   /* if not assigned to regs */

  for (; size < aop->size; size++)
    {
      int reg;
      for (reg = from; reg <= to; reg++)
        if (aop->aopu.aop_reg[size] == REG_WITH_INDEX (reg))
          return 1;
    }
  return 0;
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

  /* if they're not symbols */
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
    return TRUE;

  /* if they have the same rname */
  if (sym1->rname[0] && sym2->rname[0] && EQ (sym1->rname, sym2->rname) && !(IS_PARM (op2) && IS_ITEMP (op1)))
    return TRUE;

  /* if left is a tmp & right is not */
  if (IS_ITEMP (op1) && !IS_ITEMP (op2) && sym1->isspilt && (sym1->usl.spillLoc == sym2))
    return TRUE;

  if (IS_ITEMP (op2) && !IS_ITEMP (op1) && sym2->isspilt && sym1->level > 0 && (sym2->usl.spillLoc == sym1))
    return TRUE;

  /* are they spilt to the same location */
  if (IS_ITEMP (op2) && IS_ITEMP (op1) && sym2->isspilt && sym1->isspilt && (sym1->usl.spillLoc == sym2->usl.spillLoc))
    return TRUE;

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* sameByte - two asmops have the same address at given offsets    */
/*-----------------------------------------------------------------*/
static bool
sameByte (asmop * aop1, int off1, asmop * aop2, int off2)
{
  if (aop1 == aop2 && off1 == off2)
    return TRUE;

  if (aop1->type != AOP_REG && aop1->type != AOP_CRY)
    return FALSE;

  if (aop1->type != aop2->type)
    return FALSE;

  if (aop1->aopu.aop_reg[off1] != aop2->aopu.aop_reg[off2])
    return FALSE;

  return TRUE;
}

/*-----------------------------------------------------------------*/
/* sameRegs - two asmops have the same registers                   */
/*-----------------------------------------------------------------*/
static bool
sameRegs (asmop * aop1, asmop * aop2)
{
  int i;

  if (aop1 == aop2)
    {
      if (aop1->type == AOP_DPTR || aop1->type == AOP_DPTR2)
        {
          return FALSE;
        }
      return TRUE;
    }

  if (aop1->type != AOP_REG && aop1->type != AOP_CRY)
    return FALSE;

  if (aop1->type != aop2->type)
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
aopOp (operand * op, iCode * ic, bool result, bool useDP2)
{
  asmop *aop;
  symbol *sym;
  int i;

  if (!op)
    return;

  /* if this a literal */
  if (IS_OP_LITERAL (op))
    {
      op->aop = aop = newAsmop (AOP_LIT);
      aop->aopu.aop_lit = OP_VALUE (op);
      aop->size = getSize (operandType (op));
      return;
    }

  /* if already has a asmop then continue */
  if (op->aop)
    {
      if ((op->aop->type == AOP_DPTR && useDP2) || (op->aop->type == AOP_DPTR2 && !useDP2))
        op->aop = NULL;
      else
        {
          op->aop->allocated++;
          return;
        }
    }

  /* if the underlying symbol has a aop */
  if (IS_SYMOP (op) && OP_SYMBOL (op)->aop)
    {
      op->aop = OP_SYMBOL (op)->aop;
      if ((op->aop->type == AOP_DPTR && useDP2) || (op->aop->type == AOP_DPTR2 && !useDP2))
        op->aop = NULL;
      else
        {
          op->aop->allocated++;
          return;
        }
    }

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aopForSym (ic, OP_SYMBOL (op), result, useDP2);
      return;
    }

  /* this is a temporary : this has
     only five choices :
     a) register
     b) spillocation
     c) rematerialize
     d) conditional
     e) can be a return use only */

  sym = OP_SYMBOL (op);

  /* if the type is a conditional */
  if (sym->regType == REG_CND)
    {
      sym->aop = op->aop = aop = newAsmop (AOP_CRY);
      aop->size = 0;
      return;
    }

  /* if it is spilt then two situations
     a) is rematerialize
     b) has a spill location */
  if (sym->isspilt || sym->nRegs == 0)
    {
      /* rematerialize it NOW */
      if (sym->remat)
        {
          sym->aop = op->aop = aop = aopForRemat (sym);
          aop->size = operandSize (op);
          return;
        }

      if (sym->accuse)
        {
          int i;
          sym->aop = op->aop = aop = newAsmop (AOP_ACC);
          aop->size = getSize (sym->type);
          for (i = 0; i < 2; i++)
            aop->aopu.aop_str[i] = accUse[i];
          return;
        }

      if (sym->ruonly)
        {
          unsigned i;

          if (useDP2)
            {
              /* a AOP_STR uses DPTR, but DPTR is already in use;
               * we're just hosed.
               */
              werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "AOP_STR with DPTR in use!");
            }

          sym->aop = op->aop = aop = newAsmop (AOP_STR);
          aop->size = getSize (sym->type);
          for (i = 0; i < fReturnSizeDS390; i++)
            aop->aopu.aop_str[i] = fReturn[i];
          return;
        }

      if (sym->dptr)
        {
          /* has been allocated to a DPTRn */
          sym->aop = op->aop = aop = newAsmop (AOP_DPTRn);
          aop->size = getSize (sym->type);
          aop->aopu.dptr = sym->dptr;
          return;
        }

      if (sym->isspilt && sym->usl.spillLoc)
        {
          asmop *oldAsmOp = NULL;

          if (getSize (sym->type) != getSize (sym->usl.spillLoc->type))
            {
              /* force a new aop if sizes differ */
              oldAsmOp = sym->usl.spillLoc->aop;
              sym->usl.spillLoc->aop = NULL;
            }
          sym->aop = op->aop = aop = aopForSym (ic, sym->usl.spillLoc, result, useDP2);
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

  /* if the type is a bit register */
  if (sym->regType == REG_BIT)
    {
      sym->aop = op->aop = aop = newAsmop (AOP_CRY);
      aop->size = sym->nRegs;   //1???
      aop->aopu.aop_reg[0] = sym->regs[0];
      aop->aopu.aop_dir = sym->regs[0]->name;
      return;
    }

  /* must be in a register */
  sym->aop = op->aop = aop = newAsmop (AOP_REG);
  aop->size = sym->nRegs;
  for (i = 0; i < sym->nRegs; i++)
    aop->aopu.aop_reg[i] = sym->regs[i];
}

/*-----------------------------------------------------------------*/
/* freeAsmop - free up the asmop given to an operand               */
/*-----------------------------------------------------------------*/
static void
freeAsmop (operand * op, asmop * aaop, iCode * ic, bool pop)
{
  asmop *aop;

  if (!op)
    aop = aaop;
  else
    aop = op->aop;

  if (!aop)
    return;

  aop->allocated--;

  if (aop->allocated)
    goto dealloc;

  /* depending on the asmop type only three cases need work
     AOP_R0, AOP_R1 & AOP_STK */
  switch (aop->type)
    {
    case AOP_R0:
      if (_G.r0Pushed)
        {
          if (pop)
            {
              emitpop ("ar0");
              _G.r0Pushed--;
            }
        }
      bitVectUnSetBit (ic->rUsed, R0_IDX);
      break;

    case AOP_R1:
      if (_G.r1Pushed)
        {
          if (pop)
            {
              emitpop ("ar1");
              _G.r1Pushed--;
            }
        }
      bitVectUnSetBit (ic->rUsed, R1_IDX);
      break;

    case AOP_STK:
    {
      int sz = aop->size;
      int stk = aop->aopu.aop_stk + aop->size;
      bitVectUnSetBit (ic->rUsed, R0_IDX);
      bitVectUnSetBit (ic->rUsed, R1_IDX);

      getFreePtr (ic, &aop, FALSE);

      if (options.stack10bit)
        {
          /* I'm not sure what to do here yet... */
          /* #STUB */
          fprintf (stderr, "*** Warning: probably generating bad code for " "10 bit stack mode.\n");
        }

      if (stk)
        {
          emitcode ("mov", "a,_bp");
          emitcode ("add", "a,#!constbyte", ((char) stk) & 0xff);
          emitcode ("mov", "%s,a", aop->aopu.aop_ptr->name);
        }
      else
        {
          emitcode ("mov", "%s,_bp", aop->aopu.aop_ptr->name);
        }

      while (sz--)
        {
          emitpop ("acc");
          emitcode ("mov", "@%s,a", aop->aopu.aop_ptr->name);
          if (!sz)
            break;
          emitcode ("dec", "%s", aop->aopu.aop_ptr->name);
        }
      op->aop = aop;
      freeAsmop (op, NULL, ic, TRUE);
      if (_G.r1Pushed)
        {
          emitpop ("ar1");
          _G.r1Pushed--;
        }
      if (_G.r0Pushed)
        {
          emitpop ("ar0");
          _G.r0Pushed--;
        }
    }
    case AOP_DPTR2:
      if (_G.dptr1InUse)
        {
          emitpop ("dpx1");
          emitpop ("dph1");
          emitpop ("dpl1");
        }
      break;
    case AOP_DPTR:
      if (_G.dptrInUse)
        {
          emitpop ("dpx");
          emitpop ("dph");
          emitpop ("dpl");
        }
      break;
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

#define DEFAULT_ACC_WARNING 0
static int saveAccWarn = DEFAULT_ACC_WARNING;

/*-----------------------------------------------------------------*/
/* opIsGptr: returns non-zero if the passed operand is             */
/* a generic pointer type.                                         */
/*-----------------------------------------------------------------*/
static int
opIsGptr (operand * op)
{
  if (op && (AOP_SIZE (op) == GPTRSIZE) && IS_GENPTR (operandType (op)))
    {
      return 1;
    }
  return 0;
}

/*-----------------------------------------------------------------*/
/* swapOperands - swap two operands                                */
/*-----------------------------------------------------------------*/
static void
swapOperands (operand ** left, operand ** right)
{
  operand *t = *right;
  *right = *left;
  *left = t;
}

/*-----------------------------------------------------------------*/
/* aopGetUsesAcc - indicates ahead of time whether aopGet() will   */
/*                 clobber the accumulator                         */
/*-----------------------------------------------------------------*/
static bool
aopGetUsesAcc (operand * oper, int offset)
{
  asmop *aop = AOP (oper);

  if (offset > (aop->size - 1))
    return FALSE;

  switch (aop->type)
    {
    case AOP_R0:
    case AOP_R1:
      if (aop->paged)
        return TRUE;
      return FALSE;
    case AOP_DPTR:
    case AOP_DPTR2:
    case AOP_DPTRn:
      return TRUE;
    case AOP_IMMD:
      return FALSE;
    case AOP_DIR:
      return FALSE;
    case AOP_REG:
      wassert (!EQ (aop->aopu.aop_reg[offset]->name, "a"));
      return FALSE;
    case AOP_CRY:
      return TRUE;
    case AOP_ACC:
      if (offset)
        return FALSE;
      return TRUE;
    case AOP_LIT:
      return FALSE;
    case AOP_STR:
      if (EQ (aop->aopu.aop_str[offset], "a"))
        return TRUE;
      return FALSE;
    case AOP_DUMMY:
      return FALSE;
    default:
      /* Error case --- will have been caught already */
      wassert (0);
      return FALSE;
    }
}

/*-------------------------------------------------------------------*/
/* aopGet - for fetching value of the aop                            */
/*                                                                   */
/* Set saveAcc to NULL if you are sure it is OK to clobber the value */
/* in the accumulator. Set it to the name of a free register         */
/* if acc must be preserved; the register will be used to preserve   */
/* acc temporarily and to return the result byte.                    */
/*-------------------------------------------------------------------*/
/*
 * NOTE: function returns a pointer to a reusable dynamically allocated
 * buffer, which should never be freed!
 * Subsequent call to aopGet() will rewrite the result of the previous
 * call, so the content of the result should be copied to an other
 * location, usually using Safe_strdup(), in order to perserve it.
 */
static const char *
aopGet (operand * oper, int offset, bool bit16, bool dname, char *saveAcc)
{
  asmop *aop = AOP (oper);
  static struct dbuf_s dbuf = { 0 };

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

  /* offset is greater than
     size then zero */
  if (offset > (aop->size - 1) && aop->type != AOP_LIT)
    {
      dbuf_append_str (&dbuf, zero);
    }
  else
    {
      /* depending on type */
      switch (aop->type)
        {
        case AOP_DUMMY:
          dbuf_append_str (&dbuf, zero);
          break;

        case AOP_R0:
        case AOP_R1:
          /* if we need to increment it */
          while (offset > aop->coff)
            {
              emitcode ("inc", "%s", aop->aopu.aop_ptr->name);
              aop->coff++;
            }

          while (offset < aop->coff)
            {
              emitcode ("dec", "%s", aop->aopu.aop_ptr->name);
              aop->coff--;
            }

          aop->coff = offset;
          if (aop->paged)
            {
              emitcode ("movx", "a,@%s", aop->aopu.aop_ptr->name);
              dbuf_append_str (&dbuf, dname ? "acc" : "a");
            }
          else
            {
              dbuf_printf (&dbuf, "@%s", aop->aopu.aop_ptr->name);
            }
          break;

        case AOP_DPTRn:
          assert (offset <= 3);
          dbuf_append_str (&dbuf, dptrn[aop->aopu.dptr][offset]);
          break;

        case AOP_DPTR:
        case AOP_DPTR2:

          if (aop->type == AOP_DPTR2)
            {
              genSetDPTR (1);
            }

          if (saveAcc)
            {
              TR_AP ("#1");
//          if (aop->type != AOP_DPTR2)
//          {
//              if (saveAccWarn) { fprintf(stderr, "saveAcc for DPTR...\n"); }
//              emitcode(";", "spanky: saveAcc for DPTR");
//          }

              emitcode ("xch", "a, %s", saveAcc);
            }

          _flushLazyDPS ();

          while (offset > aop->coff)
            {
              emitcode ("inc", "dptr");
              aop->coff++;
            }

          while (offset < aop->coff)
            {
              emitcode ("lcall", "__decdptr");
              aop->coff--;
            }

          aop->coff = offset;
          if (aop->code)
            {
              emitcode ("clr", "a");
              emitcode ("movc", "a,@a+dptr");
            }
          else
            {
              emitcode ("movx", "a,@dptr");
            }

          if (aop->type == AOP_DPTR2)
            {
              genSetDPTR (0);
            }

          if (saveAcc)
            {
              TR_AP ("#2");
              emitcode ("xch", "a, %s", saveAcc);
//        if (strcmp(saveAcc, "acc1"))
//          {
//            emitcode(";", "spiffy: non acc1 return from aopGet.");
//          }

              dbuf_append_str (&dbuf, saveAcc);
            }
          else
            {
              dbuf_append_str (&dbuf, dname ? "acc" : "a");
            }
          break;

        case AOP_IMMD:
          if (aop->aopu.aop_immd.from_cast_remat && (offset == (aop->size - 1)))
            {
              dbuf_printf (&dbuf, "%s", aop->aopu.aop_immd.aop_immd2);
            }
          else if (bit16)
            {
              dbuf_printf (&dbuf, "#%s", aop->aopu.aop_immd.aop_immd1);
            }
          else if (offset)
            {
              switch (offset)
                {
                case 1:
                  dbuf_tprintf (&dbuf, "#!his", aop->aopu.aop_immd.aop_immd1);
                  break;
                case 2:
                  dbuf_tprintf (&dbuf, "#!hihis", aop->aopu.aop_immd.aop_immd1);
                  break;
                case 3:
                  dbuf_tprintf (&dbuf, "#!hihihis", aop->aopu.aop_immd.aop_immd1);
                  break;
                default:       /* should not need this (just in case) */
                  dbuf_printf (&dbuf, "#(%s >> %d)", aop->aopu.aop_immd.aop_immd1, offset * 8);
                }
            }
          else
            {
              dbuf_printf (&dbuf, "#%s", aop->aopu.aop_immd.aop_immd1);
            }
          break;

        case AOP_DIR:
          if ((SPEC_SCLS (getSpec (operandType (oper))) == S_SFR) && (aop->size > 1))
            {
              dbuf_printf (&dbuf, "((%s >> %d) & 0xFF)", aop->aopu.aop_dir, offset * 8);
            }
          else if (offset)
            {
              dbuf_printf (&dbuf, "(%s + %d)", aop->aopu.aop_dir, offset);
            }
          else
            {
              dbuf_printf (&dbuf, "%s", aop->aopu.aop_dir);
            }
          break;

        case AOP_REG:
          dbuf_append_str (&dbuf, dname ? aop->aopu.aop_reg[offset]->dname : aop->aopu.aop_reg[offset]->name);
          break;

        case AOP_CRY:
          emitcode ("mov", "c,%s", aop->aopu.aop_dir);
          emitcode ("clr", "a");
          emitcode ("rlc", "a");
          dbuf_append_str (&dbuf, dname ? "acc" : "a");
          break;

        case AOP_ACC:
          dbuf_append_str (&dbuf, (!offset && dname) ? "acc" : aop->aopu.aop_str[offset]);
          break;

        case AOP_LIT:
        {
          int size = 1 + (bit16 ? (options.model == MODEL_FLAT24 ? 2 : 1) : 0);
          dbuf_append_str (&dbuf, aopLiteralLong (aop->aopu.aop_lit, offset, size));
        }
        break;

        case AOP_STR:
          aop->coff = offset;
          if (EQ (aop->aopu.aop_str[offset], "a") && dname)
            dbuf_append_str (&dbuf, "acc");
          else
            dbuf_append_str (&dbuf, aop->aopu.aop_str[offset]);
          break;

        default:
          dbuf_destroy (&dbuf);
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopget got unsupported aop->type");
          exit (EXIT_FAILURE);
        }
    }
  return dbuf_c_str (&dbuf);
}

/*-----------------------------------------------------------------*/
/* aopPutUsesAcc - indicates ahead of time whether aopPut() will   */
/*                 clobber the accumulator                         */
/*-----------------------------------------------------------------*/
static bool
aopPutUsesAcc (operand * oper, const char *s, int offset)
{
  asmop *aop = AOP (oper);

  if (offset > (aop->size - 1))
    return FALSE;

  switch (aop->type)
    {
    case AOP_DUMMY:
      return TRUE;
    case AOP_DIR:
      return FALSE;
    case AOP_REG:
      wassert (!EQ (aop->aopu.aop_reg[offset]->name, "a"));
      return FALSE;
    case AOP_DPTRn:
      return FALSE;
    case AOP_DPTR:
    case AOP_DPTR2:
      return TRUE;
    case AOP_R0:
    case AOP_R1:
      return ((aop->paged) || (*s == '@'));
    case AOP_STK:
      return (*s == '@');
    case AOP_CRY:
      return (!aop->aopu.aop_dir || !EQ (s, aop->aopu.aop_dir));
    case AOP_STR:
      return FALSE;
    case AOP_IMMD:
      return FALSE;
    case AOP_ACC:
      return FALSE;
    default:
      /* Error case --- will have been caught already */
      wassert (0);
      return FALSE;
    }
}

/*-----------------------------------------------------------------*/
/* aopPut - puts a string for a aop and indicates if acc is in use */
/*-----------------------------------------------------------------*/
static bool
aopPut (operand * result, const char *s, int offset)
{
  bool bvolatile = isOperandVolatile (result, FALSE);
  bool accuse = FALSE;
  asmop *aop = AOP (result);
  const char *d = NULL;
  static struct dbuf_s dbuf = { 0 };

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

  if (aop->size && offset > (aop->size - 1))
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut got offset > aop->size");
      exit (EXIT_FAILURE);
    }

  /* will assign value to value */
  /* depending on where it is ofcourse */
  switch (aop->type)
    {
    case AOP_DUMMY:
      MOVA (s);                 /* read s in case it was volatile */
      accuse = TRUE;
      break;

    case AOP_DIR:
      if ((SPEC_SCLS (getSpec (operandType (result))) == S_SFR) && (aop->size > 1))
        {
          dbuf_printf (&dbuf, "((%s >> %d) & 0xFF)", aop->aopu.aop_dir, offset * 8);
        }
      else if (offset)
        {
          dbuf_printf (&dbuf, "(%s + %d)", aop->aopu.aop_dir, offset);
        }
      else
        {
          dbuf_append_str (&dbuf, aop->aopu.aop_dir);
        }

      if (!EQ (dbuf_c_str (&dbuf), s) || bvolatile)
        {
          emitcode ("mov", "%s,%s", dbuf_c_str (&dbuf), s);
        }
      if (EQ (dbuf_c_str (&dbuf), "acc"))
        {
          accuse = TRUE;
        }
      break;

    case AOP_REG:
      if (!EQ (aop->aopu.aop_reg[offset]->name, s) && !EQ (aop->aopu.aop_reg[offset]->dname, s))
        {
          if (*s == '@' ||
              EQ (s, "r0") || EQ (s, "r1") || EQ (s, "r2") || EQ (s, "r3") ||
              EQ (s, "r4") || EQ (s, "r5") || EQ (s, "r6") || EQ (s, "r7"))
            {
              emitcode ("mov", "%s,%s", aop->aopu.aop_reg[offset]->dname, s);
            }
          else
            {
              emitcode ("mov", "%s,%s", aop->aopu.aop_reg[offset]->name, s);
            }
        }
      break;

    case AOP_DPTRn:
      emitcode ("mov", "%s,%s", dptrn[aop->aopu.dptr][offset], s);
      break;

    case AOP_DPTR:
    case AOP_DPTR2:

      if (aop->type == AOP_DPTR2)
        {
          genSetDPTR (1);
        }
      _flushLazyDPS ();

      if (aop->code)
        {
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut writing to code space");
          exit (EXIT_FAILURE);
        }

      while (offset > aop->coff)
        {
          aop->coff++;
          emitcode ("inc", "dptr");
        }

      while (offset < aop->coff)
        {
          aop->coff--;
          emitcode ("lcall", "__decdptr");
        }

      aop->coff = offset;

      /* if not in accumulator */
      MOVA (s);

      emitcode ("movx", "@dptr,a");

      if (aop->type == AOP_DPTR2)
        {
          genSetDPTR (0);
        }
      break;

    case AOP_R0:
    case AOP_R1:
      while (offset > aop->coff)
        {
          aop->coff++;
          emitcode ("inc", "%s", aop->aopu.aop_ptr->name);
        }
      while (offset < aop->coff)
        {
          aop->coff--;
          emitcode ("dec", "%s", aop->aopu.aop_ptr->name);
        }
      aop->coff = offset;

      if (aop->paged)
        {
          MOVA (s);
          emitcode ("movx", "@%s,a", aop->aopu.aop_ptr->name);
        }
      else if (*s == '@')
        {
          MOVA (s);
          emitcode ("mov", "@%s,a", aop->aopu.aop_ptr->name);
        }
      else if (EQ (s, "r0") || EQ (s, "r1") || EQ (s, "r2") || EQ (s, "r3") ||
               EQ (s, "r4") || EQ (s, "r5") || EQ (s, "r6") || EQ (s, "r7"))
        {
          dbuf_printf (&dbuf, "a%s", s);
          emitcode ("mov", "@%s,%s", aop->aopu.aop_ptr->name, dbuf_c_str (&dbuf));
        }
      else
        {
          emitcode ("mov", "@%s,%s", aop->aopu.aop_ptr->name, s);
        }
      break;

    case AOP_STK:
      emitpush (s);
      break;

    case AOP_CRY:
      // destination is carry for return-use-only
      d = (IS_OP_RUONLY (result)) ? "c" : aop->aopu.aop_dir;

      // source is no literal and not in carry
      if (!EQ (s, zero) && !EQ (s, one) && !EQ (s, "c"))
        {
          MOVA (s);
          /* set C, if a >= 1 */
          emitcode ("add", "a,#!constbyte", 0xff);
          s = "c";
        }
      // now source is zero, one or carry

      /* if result no bit variable */
      if (!d)
        {
          if (EQ (s, "c"))
            {
              /* inefficient: move carry into A and use jz/jnz */
              emitcode ("clr", "a");
              emitcode ("rlc", "a");
              accuse = TRUE;
            }
          else
            {
              MOVA (s);
              accuse = TRUE;
            }
        }
      else if (EQ (s, zero))
        emitcode ("clr", "%s", d);
      else if (EQ (s, one))
        emitcode ("setb", "%s", d);
      else if (!EQ (s, d))
        emitcode ("mov", "%s,c", d);
      break;

    case AOP_STR:
      aop->coff = offset;
      if (!EQ (aop->aopu.aop_str[offset], s) || bvolatile)
        emitcode ("mov", "%s,%s", aop->aopu.aop_str[offset], s);
      break;

    case AOP_ACC:
      accuse = TRUE;
      aop->coff = offset;
      if (!offset && EQ (s, "acc") && !bvolatile)
        break;

      if (!EQ (aop->aopu.aop_str[offset], s) && !bvolatile)
        emitcode ("mov", "%s,%s", aop->aopu.aop_str[offset], s);
      break;

    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut got unsupported aop->type");
      exit (EXIT_FAILURE);
    }

  return accuse;
}

/*--------------------------------------------------------------------*/
/* loadDptrFromOperand - load dptr (and optionally B) from operand op */
/*--------------------------------------------------------------------*/
static int
loadDptrFromOperand (operand * op, bool loadBToo)
{
  int dopi = 1;

  /* if the operand is already in dptr
     then we do nothing else we move the value to dptr */
  if (AOP_TYPE (op) != AOP_STR && !AOP_INDPTRn (op))
    {
      /* if this is rematerializable */
      if (AOP_TYPE (op) == AOP_IMMD)
        {
          emitcode ("mov", "dptr,%s", aopGet (op, 0, TRUE, FALSE, NULL));
          if (loadBToo)
            {
              if (AOP (op)->aopu.aop_immd.from_cast_remat)
                emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE, NULL));
              else
                {
                  wassertl (FALSE, "need pointerCode");
                  emitcode (";", "mov b,???");
                  /* genPointerGet and genPointerSet originally did different
                   ** things for this case. Both seem wrong.
                   ** from genPointerGet:
                   **  emitcode ("mov", "b,#%d", pointerCode (retype));
                   ** from genPointerSet:
                   **  emitcode ("mov", "b,%s + 1", aopGet (result, 0, TRUE, ANY));
                   */
                }
            }
        }
      else if (AOP_TYPE (op) == AOP_LIT)
        {
          emitcode ("mov", "dptr,%s", aopGet (op, 0, TRUE, FALSE, NULL));
          if (loadBToo)
            emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE, NULL));
        }
      else if (AOP_TYPE (op) == AOP_DPTR)
        {
          /* we need to get it byte by byte */
          _startLazyDPSEvaluation ();
          /* We need to generate a load to DPTR indirect through DPTR. */
          D (emitcode (";", "genFarPointerGet -- indirection special case."));
          emitpush (aopGet (op, 0, FALSE, TRUE, NULL));
          if (loadBToo || options.model == MODEL_FLAT24)
            {
              emitpush (aopGet (op, 1, FALSE, TRUE, NULL));
              if (options.model == MODEL_FLAT24)
                {
                  if (loadBToo)
                    {
                      emitpush (aopGet (op, 2, FALSE, TRUE, NULL));
                      emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE, NULL));
                      emitpop ("dpx");
                    }
                  else
                    {
                      emitcode ("mov", "dpx,%s", aopGet (op, 2, FALSE, FALSE, NULL));
                    }
                }
              else
                {
                  emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE, NULL));
                }
              emitpop ("dph");
            }
          emitpop ("dpl");
          _endLazyDPSEvaluation ();
          dopi = 0;
        }
      else
        {
          /* we need to get it byte by byte */
          _startLazyDPSEvaluation ();
          emitcode ("mov", "dpl,%s", aopGet (op, 0, FALSE, FALSE, NULL));
          emitcode ("mov", "dph,%s", aopGet (op, 1, FALSE, FALSE, NULL));
          if (options.model == MODEL_FLAT24)
            emitcode ("mov", "dpx,%s", aopGet (op, 2, FALSE, FALSE, NULL));
          if (loadBToo)
            emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE, NULL));
          _endLazyDPSEvaluation ();
        }
    }
  return dopi;
}

/*--------------------------------------------------------------------*/
/* reAdjustPreg - points a register back to where it should (coff==0) */
/*--------------------------------------------------------------------*/
static void
reAdjustPreg (asmop * aop)
{
  if ((aop->coff == 0) || (aop->size <= 1))
    return;

  switch (aop->type)
    {
    case AOP_R0:
    case AOP_R1:
      while (aop->coff--)
        emitcode ("dec", "%s", aop->aopu.aop_ptr->name);
      break;
    case AOP_DPTR:
    case AOP_DPTR2:
      if (aop->type == AOP_DPTR2)
        {
          genSetDPTR (1);
          _flushLazyDPS ();
        }
      while (aop->coff--)
        {
          emitcode ("lcall", "__decdptr");
        }

      if (aop->type == AOP_DPTR2)
        {
          genSetDPTR (0);
        }
      break;
    }
  aop->coff = 0;
}

/*-----------------------------------------------------------------*/
/* getDataSize - get the operand data size                         */
/*-----------------------------------------------------------------*/
static int
getDataSize (operand * op)
{
  int size = AOP_SIZE (op);

  if (size == GPTRSIZE)
    {
      sym_link *type = operandType (op);
      if (IS_GENPTR (type))
        {
          /* generic pointer; arithmetic operations
           * should ignore the high byte (pointer type).
           */
          size--;
        }
    }
  return size;
}

/*-----------------------------------------------------------------*/
/* outAcc - output Acc                                             */
/*-----------------------------------------------------------------*/
static void
outAcc (operand * result)
{
  int size, offset;
  size = getDataSize (result);
  if (size)
    {
      aopPut (result, "a", 0);
      size--;
      offset = 1;
      /* unsigned or positive */
      while (size--)
        {
          aopPut (result, zero, offset++);
        }
    }
}

/*-----------------------------------------------------------------*/
/* outBitC - output a bit C                                        */
/*-----------------------------------------------------------------*/
static void
outBitC (operand * result)
{
  /* if the result is bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      aopPut (result, "c", 0);
    }
  else if (AOP_TYPE (result) != AOP_DUMMY)
    {
      emitcode ("clr", "a");
      emitcode ("rlc", "a");
      outAcc (result);
    }
}

/*-----------------------------------------------------------------*/
/* toBoolean - emit code for orl a,operator(sizeop)                */
/*-----------------------------------------------------------------*/
static void
toBoolean (operand * oper)
{
  int size = AOP_SIZE (oper) - 1;
  int offset = 1;
  bool AccUsed;
  sym_link *type = operandType (oper);
  bool pushedB;

  /* always need B for float */
  AccUsed = IS_FLOAT (type);

  while (!AccUsed && size--)
    {
      AccUsed |= aopGetUsesAcc (oper, offset++);
    }

  /* The generic part of a generic pointer should
   * not participate in it's truth value.
   *
   * i.e. 0x10000000 is zero.
   */
  if (opIsGptr (oper))
    {
      D (emitcode (";", "toBoolean: generic ptr special case."));
      size = AOP_SIZE (oper) - 2;
    }
  else
    {
      size = AOP_SIZE (oper) - 1;
    }

  offset = 0;
  _startLazyDPSEvaluation ();
  if (size && AccUsed && (AOP (oper)->type != AOP_ACC))
    {
      pushedB = pushB ();
      MOVB (aopGet (oper, offset++, FALSE, FALSE, NULL));
      while (--size)
        {
          MOVA (aopGet (oper, offset++, FALSE, FALSE, NULL));
          emitcode ("orl", "b,a");
        }
      MOVA (aopGet (oper, offset++, FALSE, FALSE, NULL));
      if (IS_FLOAT (type))
        emitcode ("anl", "a,#0x7F");    //clear sign bit
      emitcode ("orl", "a,b");
      popB (pushedB);
    }
  else
    {
      MOVA (aopGet (oper, offset++, FALSE, FALSE, NULL));
      while (size--)
        {
          emitcode ("orl", "a,%s", aopGet (oper, offset++, FALSE, FALSE, NULL));
        }
    }
  _endLazyDPSEvaluation ();
}

/*-----------------------------------------------------------------*/
/* toCarry - make boolean and move into carry                      */
/*-----------------------------------------------------------------*/
static void
toCarry (operand * oper)
{
  /* if the operand is a literal then
     we know what the value is */
  if (AOP_TYPE (oper) == AOP_LIT)
    {
      if ((int) operandLitValue (oper))
        SETC;
      else
        CLRC;
    }
  else if (AOP_TYPE (oper) == AOP_CRY)
    {
      if (!IS_OP_ACCUSE (oper))
        emitcode ("mov", "c,%s", oper->aop->aopu.aop_dir);
    }
  else
    {
      /* or the operand into a */
      toBoolean (oper);
      /* set C, if a >= 1 */
      emitcode ("add", "a,#0xff");
    }
}
#if 0 // as of yet still unused
/*-----------------------------------------------------------------*/
/* assignBit - assign operand to bit operand                       */
/*-----------------------------------------------------------------*/
static void
assignBit(operand * result, operand * right)
{
	/* if the right side is a literal then
	we know what the value is */
	if (AOP_TYPE(right) == AOP_LIT)
	{
		if ((int)operandLitValue(right))
			aopPut(result, one, 0);
		else
			aopPut(result, zero, 0);
	}
	else
	{
		toCarry(right);
		outBitC(result);
	}
}
#endif
/*-------------------------------------------------------------------*/
/* xch_a_aopGet - for exchanging acc with value of the aop           */
/*-------------------------------------------------------------------*/
static const char *
xch_a_aopGet (operand * oper, int offset, bool bit16, bool dname, char *saveAcc)
{
  const char *l;

  if (aopGetUsesAcc (oper, offset))
    {
      emitcode ("mov", "b,a");
      MOVA (aopGet (oper, offset, bit16, dname, saveAcc));
      emitcode ("xch", "a,b");
      aopPut (oper, "a", offset);
      emitcode ("xch", "a,b");
      l = "b";
    }
  else
    {
      l = aopGet (oper, offset, bit16, dname, saveAcc);
      emitcode ("xch", "a,%s", l);
    }
  return l;
}

/*-----------------------------------------------------------------*/
/* genNot - generate code for ! operation                          */
/*-----------------------------------------------------------------*/
static void
genNot (iCode * ic)
{
  symbol *tlbl;

  D (emitcode (";", "genNot"));

  /* assign asmOps to operand & result */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, AOP_USESDPTR (IC_LEFT (ic)));

  /* if in bit space then a special case */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_CRY)
    {
      /* if left==result then cpl bit */
      if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
        {
          emitcode ("cpl", "%s", IC_LEFT (ic)->aop->aopu.aop_dir);
        }
      else
        {
          toCarry (IC_LEFT (ic));
          emitcode ("cpl", "c");
          outBitC (IC_RESULT (ic));
        }
      goto release;
    }

  toBoolean (IC_LEFT (ic));

  /* set C, if a == 0 */
  tlbl = newiTempLabel (NULL);
  emitcode ("cjne", "a,#0x01,!tlabel", labelKey2num (tlbl->key));
  emitLabel (tlbl);
  outBitC (IC_RESULT (ic));

release:
  /* release the aops */
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
  freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? 0 : 1));
}

/*-----------------------------------------------------------------*/
/* genCpl - generate code for complement                           */
/*-----------------------------------------------------------------*/
static void
genCpl (iCode * ic)
{
  int offset = 0;
  int size;
  symbol *tlbl;
  sym_link *letype = getSpec (operandType (IC_LEFT (ic)));

  D (emitcode (";", "genCpl"));

  /* assign asmOps to operand & result */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, AOP_USESDPTR (IC_LEFT (ic)));

  /* special case if in bit space */
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY)
    {
      const char *l;

      if (AOP_TYPE (IC_LEFT (ic)) == AOP_CRY || (SPEC_USIGN (letype) && IS_CHAR (letype)))
        {
          /* promotion rules are responsible for this strange result:
             bit -> int -> ~int -> bit
             uchar -> int -> ~int -> bit
           */
          emitcode ("setb", "%s", IC_RESULT (ic)->aop->aopu.aop_dir);
          goto release;
        }

      tlbl = newiTempLabel (NULL);
      l = aopGet (IC_LEFT (ic), offset++, FALSE, FALSE, NULL);
      if ((AOP_TYPE (IC_LEFT (ic)) == AOP_ACC && offset == 0) ||
          AOP_TYPE (IC_LEFT (ic)) == AOP_REG || IS_AOP_PREG (IC_LEFT (ic)))
        {
          emitcode ("cjne", "%s,#0xFF,!tlabel", l, labelKey2num (tlbl->key));
        }
      else
        {
          MOVA (l);
          emitcode ("cjne", "a,#0xFF,!tlabel", labelKey2num (tlbl->key));
        }
      emitLabel (tlbl);
      outBitC (IC_RESULT (ic));
      goto release;
    }

  size = AOP_SIZE (IC_RESULT (ic));
  _startLazyDPSEvaluation ();
  while (size--)
    {
      MOVA (aopGet (IC_LEFT (ic), offset, FALSE, FALSE, NULL));
      emitcode ("cpl", "a");
      aopPut (IC_RESULT (ic), "a", offset++);
    }
  _endLazyDPSEvaluation ();

release:
  /* release the aops */
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
  freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? 0 : 1));
}

/*-----------------------------------------------------------------*/
/* genUminusFloat - unary minus for floating points                */
/*-----------------------------------------------------------------*/
static void
genUminusFloat (operand * op, operand * result)
{
  int size, offset = 0;

  D (emitcode (";", "genUminusFloat"));

  /* for this we just copy and then flip the bit */

  _startLazyDPSEvaluation ();
  size = AOP_SIZE (op) - 1;

  while (size--)
    {
      aopPut (result, aopGet (op, offset, FALSE, FALSE, NULL), offset);
      offset++;
    }

  MOVA (aopGet (op, offset, FALSE, FALSE, NULL));

  emitcode ("cpl", "acc[7]");
  aopPut (result, "a", offset);
  _endLazyDPSEvaluation ();
}

/*-----------------------------------------------------------------*/
/* genUminus - unary minus code generation                         */
/*-----------------------------------------------------------------*/
static void
genUminus (iCode * ic)
{
  int offset, size;
  sym_link *optype;

  D (emitcode (";", "genUminus"));

  /* assign asmops */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE, (AOP_TYPE (IC_LEFT (ic)) == AOP_DPTR));

  /* if both in bit space then special
     case */
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY && AOP_TYPE (IC_LEFT (ic)) == AOP_CRY)
    {

      emitcode ("mov", "c,%s", IC_LEFT (ic)->aop->aopu.aop_dir);
      emitcode ("cpl", "c");
      emitcode ("mov", "%s,c", IC_RESULT (ic)->aop->aopu.aop_dir);
      goto release;
    }

  optype = operandType (IC_LEFT (ic));

  /* if float then do float stuff */
  if (IS_FLOAT (optype))
    {
      genUminusFloat (IC_LEFT (ic), IC_RESULT (ic));
      goto release;
    }

  /* otherwise subtract from zero */
  size = AOP_SIZE (IC_LEFT (ic));
  offset = 0;
  _startLazyDPSEvaluation ();
  while (size--)
    {
      const char *l = aopGet (IC_LEFT (ic), offset, FALSE, FALSE, NULL);
      if (EQ (l, "a"))
        {
          if (offset == 0)
            SETC;
          emitcode ("cpl", "a");
          emitcode ("addc", "a,#0x00");
        }
      else
        {
          if (offset == 0)
            CLRC;
          emitcode ("clr", "a");
          emitcode ("subb", "a,%s", l);
        }
      aopPut (IC_RESULT (ic), "a", offset++);
    }
  _endLazyDPSEvaluation ();

  /* if any remaining bytes in the result */
  /* we just need to propagate the sign   */
  if ((size = (AOP_SIZE (IC_RESULT (ic)) - AOP_SIZE (IC_LEFT (ic)))))
    {
      emitcode ("rlc", "a");
      emitcode ("subb", "a,acc");
      while (size--)
        aopPut (IC_RESULT (ic), "a", offset++);
    }

release:
  /* release the aops */
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
  freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? 0 : 1));
}

/*-----------------------------------------------------------------*/
/* savermask - saves registers in the mask                         */
/*-----------------------------------------------------------------*/
static void
savermask (bitVect * rs_mask)
{
  int i;

  if (options.useXstack)
    {
      if (bitVectBitValue (rs_mask, R0_IDX))
        emitcode ("mov", "b,r0");
      emitcode ("mov", "r0,%s", spname);
      for (i = 0; i < ds390_nRegs; i++)
        {
          if (bitVectBitValue (rs_mask, i))
            {
              if (i == R0_IDX)
                emitcode ("mov", "a,b");
              else
                emitcode ("mov", "a,%s", REG_WITH_INDEX (i)->name);
              emitcode ("movx", "@r0,a");
              emitcode ("inc", "r0");
            }
        }
      emitcode ("mov", "%s,r0", spname);
      if (bitVectBitValue (rs_mask, R0_IDX))
        emitcode ("mov", "r0,b");
    }
  else
    {
      bool bits_pushed = FALSE;
      for (i = 0; i < ds390_nRegs; i++)
        {
          if (bitVectBitValue (rs_mask, i))
            {
              bits_pushed = pushReg (i, bits_pushed);
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* saveRegisters - will look for a call and save the registers     */
/*-----------------------------------------------------------------*/
static void
saveRegisters (iCode * lic)
{
  iCode *ic;
  bitVect *rsave;

  /* look for call */
  for (ic = lic; ic; ic = ic->next)
    if (ic->op == CALL || ic->op == PCALL)
      break;

  if (!ic)
    {
      fprintf (stderr, "found parameter push with no function call\n");
      return;
    }

  /* if the registers have been saved already or don't need to be then
     do nothing */
  if (ic->regsSaved)
    return;
  if (IS_SYMOP (IC_LEFT (ic)))
    {
      sym_link *type = OP_SYM_TYPE (IC_LEFT (ic));
      if (IFFUNC_ISNAKED (type) && !TARGET_IS_DS400)
        return;
    }

  /* special case if DPTR alive across a function call then must save it
     even though callee saves */
  if (IS_SYMOP (IC_LEFT (ic)) && IFFUNC_CALLEESAVES (OP_SYMBOL (IC_LEFT (ic))->type))
    {
      int i;
      rsave = newBitVect (ic->rMask->size);
      for (i = DPL_IDX; i <= B_IDX; i++)
        {
          if (bitVectBitValue (ic->rMask, i))
            rsave = bitVectSetBit (rsave, i);
        }
      rsave = bitVectCplAnd (rsave, ds390_rUmaskForOp (IC_RESULT (ic)));
    }
  else
    {
      /* save the registers in use at this time but skip the
         ones for the result */
      rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic)));
    }
  ic->regsSaved = 1;
  savermask (rsave);
}

/*-----------------------------------------------------------------*/
/* unsavermask - restore registers with mask                       */
/*-----------------------------------------------------------------*/
static void
unsavermask (bitVect * rs_mask)
{
  int i;

  if (options.useXstack)
    {
      emitcode ("mov", "r0,%s", spname);
      for (i = ds390_nRegs; i >= 0; i--)
        {
          if (bitVectBitValue (rs_mask, i))
            {
              reg_info *reg = REG_WITH_INDEX (i);
              emitcode ("dec", "r0");
              emitcode ("movx", "a,@r0");
              if (i == R0_IDX)
                {
                  emitcode ("push", "acc");
                }
              else
                {
                  emitcode ("mov", "%s,a", reg->name);
                }
            }
        }
      emitcode ("mov", "%s,r0", spname);
      if (bitVectBitValue (rs_mask, R0_IDX))
        {
          emitcode ("pop", "ar0");
        }
    }
  else
    {
      bool bits_popped = FALSE;
      for (i = ds390_nRegs; i >= 0; i--)
        {
          if (bitVectBitValue (rs_mask, i))
            {
              bits_popped = popReg (i, bits_popped);
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* unsaveRegisters - pop the pushed registers                      */
/*-----------------------------------------------------------------*/
static void
unsaveRegisters (iCode * ic)
{
  bitVect *rsave;

  if (IS_SYMOP (IC_LEFT (ic)) && IFFUNC_CALLEESAVES (OP_SYMBOL (IC_LEFT (ic))->type))
    {
      int i;
      rsave = newBitVect (ic->rMask->size);
      for (i = DPL_IDX; i <= B_IDX; i++)
        {
          if (bitVectBitValue (ic->rMask, i))
            rsave = bitVectSetBit (rsave, i);
        }
      rsave = bitVectCplAnd (rsave, ds390_rUmaskForOp (IC_RESULT (ic)));
    }
  else
    {
      /* restore the registers in use at this time but skip the
         ones for the result */
      rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic)));
    }
  unsavermask (rsave);
}


/*-----------------------------------------------------------------*/
/* pushSide -                                                      */
/*-----------------------------------------------------------------*/
static void
pushSide (operand * oper, int size, iCode * ic)
{
  int offset = 0;
  int nPushed = _G.r0Pushed + _G.r1Pushed;

  aopOp (oper, ic, FALSE, FALSE);

  if (nPushed != _G.r0Pushed + _G.r1Pushed)
    {
      while (offset < size)
        {
          const char *l = aopGet (oper, offset, FALSE, TRUE, NULL);
          emitcode ("mov", "%s,%s", fReturn[offset++], l);
        }
      freeAsmop (oper, NULL, ic, TRUE);
      offset = 0;
      while (offset < size)
        {
          emitpush (fReturn[offset++]);
        }
      return;
    }

  _startLazyDPSEvaluation ();
  while (size--)
    {
      const char *l = aopGet (oper, offset++, FALSE, TRUE, NULL);
      if (AOP_TYPE (oper) != AOP_REG && AOP_TYPE (oper) != AOP_DIR && strcmp (l, "a"))
        {
          MOVA (l);
          emitpush ("acc");
        }
      else
        {
          emitpush (l);
        }
    }
  _endLazyDPSEvaluation ();
  freeAsmop (oper, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* assignResultValue - also indicates if acc is in use afterwards  */
/*-----------------------------------------------------------------*/
static bool
assignResultValue (operand * oper, operand * func)
{
  int offset = 0;
  int size = AOP_SIZE (oper);
  bool accuse = FALSE;
  bool pushedA = FALSE;

  if (func && IS_BIT (OP_SYM_ETYPE (func)))
    {
      outBitC (oper);
      return FALSE;
    }

  if (AOP_NEEDSACC (oper))
    {
      int i;
      for (i=0; i<size; i++)
        {
          if (strcmp(fReturn[i],"a")==0)
            {
              emitcode (";", "assignResultValue special case for ACC.");
              emitpush ("acc");
              pushedA = TRUE;
              break;
            }
        }
    }
              
  _startLazyDPSEvaluation ();
  while (size--)
    {
      if (pushedA && strcmp(fReturn[offset],"a")==0)
        emitpop ("acc");
      accuse |= aopPut (oper, fReturn[offset], offset);
      offset++;
    }
  _endLazyDPSEvaluation ();

  return accuse;
}


/*-----------------------------------------------------------------*/
/* genXpush - pushes onto the external stack                       */
/*-----------------------------------------------------------------*/
static void
genXpush (iCode * ic)
{
  asmop *aop = newAsmop (0);
  reg_info *r;
  int size, offset = 0;

  D (emitcode (";", "genXpush"));

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  r = getFreePtr (ic, &aop, FALSE);

  size = AOP_SIZE (IC_LEFT (ic));

  if (size == 1)
    {
      MOVA (aopGet (IC_LEFT (ic), 0, FALSE, FALSE, NULL));
      emitcode ("mov", "%s,_spx", r->name);
      emitcode ("inc", "_spx"); // allocate space first
      emitcode ("movx", "@%s,a", r->name);
    }
  else
    {
      // allocate space first
      emitcode ("mov", "%s,_spx", r->name);
      MOVA (r->name);
      emitcode ("add", "a,#%d", size);
      emitcode ("mov", "_spx,a");

      _startLazyDPSEvaluation ();
      while (size--)
        {
          MOVA (aopGet (IC_LEFT (ic), offset++, FALSE, FALSE, NULL));
          emitcode ("movx", "@%s,a", r->name);
          emitcode ("inc", "%s", r->name);
        }
      _endLazyDPSEvaluation ();
    }

  freeAsmop (NULL, aop, ic, TRUE);
  freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genIpush - generate code for pushing this gets a little complex */
/*-----------------------------------------------------------------*/
static void
genIpush (iCode * ic)
{
  int size, offset = 0;
  char *prev;

  D (emitcode (";", "genIpush"));

  /* if this is not a parm push : ie. it is spill push
     and spill push is always done on the local stack */
  if (!ic->parmPush)
    {
      /* and the item is spilt then do nothing */
      if (OP_SYMBOL (IC_LEFT (ic))->isspilt || OP_SYMBOL (IC_LEFT (ic))->dptr)
        return;

      aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
      size = AOP_SIZE (IC_LEFT (ic));
      /* push it on the stack */
      _startLazyDPSEvaluation ();
      while (size--)
        {
          emitpush (aopGet (IC_LEFT (ic), offset++, FALSE, TRUE, NULL));
        }
      _endLazyDPSEvaluation ();
      return;
    }

  /* this is a parameter push: in this case we call
     the routine to find the call and save those
     registers that need to be saved */
  saveRegisters (ic);

  /* if use external stack then call the external
     stack pushing routine */
  if (options.useXstack)
    {
      genXpush (ic);
      return;
    }

  /* then do the push */
  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);

  // pushSide(IC_LEFT(ic), AOP_SIZE(IC_LEFT(ic)));
  size = AOP_SIZE (IC_LEFT (ic));

  _startLazyDPSEvaluation ();
  prev = Safe_strdup ("");
  while (size--)
    {
      const char *l = aopGet (IC_LEFT (ic), offset++, FALSE, TRUE, NULL);
      if (AOP_TYPE (IC_LEFT (ic)) != AOP_REG && AOP_TYPE (IC_LEFT (ic)) != AOP_DIR && strcmp (l, "acc"))
        {
          if (!EQ (l, prev) || *l == '@')
            MOVA (l);
          emitpush ("acc");
        }
      else
        {
          emitpush (l);
        }
      Safe_free (prev);
      prev = Safe_strdup (l);
    }
  Safe_free (prev);
  _endLazyDPSEvaluation ();

  freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genIpop - recover the registers: can happen only for spilling   */
/*-----------------------------------------------------------------*/
static void
genIpop (iCode * ic)
{
  int size, offset;

  D (emitcode (";", "genIpop"));

  /* if the temp was not pushed then */
  if (OP_SYMBOL (IC_LEFT (ic))->isspilt || OP_SYMBOL (IC_LEFT (ic))->dptr)
    return;

  aopOp (IC_LEFT (ic), ic, FALSE, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));
  offset = (size - 1);
  _startLazyDPSEvaluation ();
  while (size--)
    {
      emitpop (aopGet (IC_LEFT (ic), offset--, FALSE, TRUE, NULL));
    }
  _endLazyDPSEvaluation ();

  freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* popForBranch - recover the spilt registers for a branch         */
/*-----------------------------------------------------------------*/
static void
popForBranch (iCode * ic, bool markGenerated)
{
  while (ic && ic->op == IPOP)
    {
      genIpop (ic);
      if (markGenerated)
        ic->generated = 1;      /* mark the icode as generated */
      ic = ic->next;
    }
}

/*-----------------------------------------------------------------*/
/* saveRBank - saves an entire register bank on the stack          */
/*-----------------------------------------------------------------*/
static void
saveRBank (int bank, iCode * ic, bool pushPsw)
{
  int i;
  int count = 8 + (ds390_nBitRegs / 8) + (pushPsw ? 1 : 0);
  asmop *aop = NULL;
  reg_info *r = NULL;

  if (options.useXstack)
    {
      if (!ic)
        {
          /* Assume r0 is available for use. */
          r = REG_WITH_INDEX (R0_IDX);
        }
      else
        {
          aop = newAsmop (0);
          r = getFreePtr (ic, &aop, FALSE);
        }
      // allocate space first
      emitcode ("mov", "%s,_spx", r->name);
      MOVA (r->name);
      emitcode ("add", "a,#!constbyte", count);
      emitcode ("mov", "_spx,a");
    }

  for (i = 0; i < 8; i++)       /* only R0-R7 needs saving */
    {
      if (options.useXstack)
        {
          emitcode ("mov", "a,(%s+%d)", regs390[i].base, 8 * bank + regs390[i].offset);
          emitcode ("movx", "@%s,a", r->name);
          if (--count)
            emitcode ("inc", "%s", r->name);
        }
      else
        emitcode ("push", "(%s+%d)", regs390[i].base, 8 * bank + regs390[i].offset);
    }

  if (ds390_nBitRegs > 0)
    {
      if (options.useXstack)
        {
          emitcode ("mov", "a,bits");
          emitcode ("movx", "@%s,a", r->name);
          if (--count)
            emitcode ("inc", "%s", r->name);
        }
      else
        {
          emitpush ("bits");
        }
      BitBankUsed = 1;
    }

  if (pushPsw)
    {
      if (options.useXstack)
        {
          emitcode ("mov", "a,psw");
          emitcode ("movx", "@%s,a", r->name);
        }
      else
        {
          emitpush ("psw");
        }

      emitcode ("mov", "psw,#!constbyte", (bank << 3) & 0x00ff);
    }

  if (aop)
    {
      freeAsmop (NULL, aop, ic, TRUE);
    }

  if (ic)
    {
      ic->bankSaved = 1;
    }
}

/*-----------------------------------------------------------------*/
/* unsaveRBank - restores the register bank from stack             */
/*-----------------------------------------------------------------*/
static void
unsaveRBank (int bank, iCode * ic, bool popPsw)
{
  int i;
  asmop *aop = NULL;
  reg_info *r = NULL;

  if (options.useXstack)
    {
      if (!ic)
        {
          /* Assume r0 is available for use. */
          r = REG_WITH_INDEX (R0_IDX);
        }
      else
        {
          aop = newAsmop (0);
          r = getFreePtr (ic, &aop, FALSE);
        }
      emitcode ("mov", "%s,_spx", r->name);
    }

  if (popPsw)
    {
      if (options.useXstack)
        {
          emitcode ("dec", "%s", r->name);
          emitcode ("movx", "a,@%s", r->name);
          emitcode ("mov", "psw,a");
        }
      else
        {
          emitpop ("psw");
        }
    }

  if (ds390_nBitRegs > 0)
    {
      if (options.useXstack)
        {
          emitcode ("dec", "%s", r->name);
          emitcode ("movx", "a,@%s", r->name);
          emitcode ("mov", "bits,a");
        }
      else
        {
          emitpop ("bits");
        }
    }

  for (i = 7; i >= 0; i--)      /* only R7-R0 needs to be popped */
    {
      if (options.useXstack)
        {
          emitcode ("dec", "%s", r->name);
          emitcode ("movx", "a,@%s", r->name);
          emitcode ("mov", "(%s+%d),a", regs390[i].base, 8 * bank + regs390[i].offset);
        }
      else
        {
          char buf[16] = "";
          SNPRINTF (buf, 16, "(%s+%d)", regs390[i].base, 8 * bank + regs390[i].offset);
          emitpop (buf);
        }
    }

  if (options.useXstack)
    {
      emitcode ("mov", "_spx,%s", r->name);
    }

  if (aop)
    {
      freeAsmop (NULL, aop, ic, TRUE);
    }
}

/*-----------------------------------------------------------------*/
/* genSend - gen code for SEND                                     */
/*-----------------------------------------------------------------*/
static void
genSend (set * sendSet)
{
  iCode *sic;
  int bit_count = 0;
  int sendCount = 0;
  static int rb1_count = 0;

  /* first we do all bit parameters */
  for (sic = setFirstItem (sendSet); sic; sic = setNextItem (sendSet))
    {
      if (sic->argreg > 12)
        {
          int bit = sic->argreg - 13;

          aopOp (IC_LEFT (sic), sic, FALSE, (IS_OP_RUONLY (IC_LEFT (sic)) ? FALSE : TRUE));

          /* if left is a literal then
             we know what the value is */
          if (AOP_TYPE (IC_LEFT (sic)) == AOP_LIT)
            {
              if (((int) operandLitValue (IC_LEFT (sic))))
                emitcode ("setb", "b[%d]", bit);
              else
                emitcode ("clr", "b[%d]", bit);
            }
          else if (AOP_TYPE (IC_LEFT (sic)) == AOP_CRY)
            {
              char *l = AOP (IC_LEFT (sic))->aopu.aop_dir;
              if (strcmp (l, "c"))
                emitcode ("mov", "c,%s", l);
              emitcode ("mov", "b[%d],c", bit);
            }
          else
            {
              /* we need to or */
              toCarry (IC_LEFT (sic));
              emitcode ("mov", "b[%d],c", bit);
            }
          bit_count++;
          BitBankUsed = 1;

          freeAsmop (IC_LEFT (sic), NULL, sic, TRUE);
        }
    }

  if (bit_count)
    {
      saveRegisters (setFirstItem (sendSet));
      emitcode ("mov", "bits,b");
    }

  /* then we do all other parameters */
  for (sic = setFirstItem (sendSet); sic; sic = setNextItem (sendSet))
    {
      if (sic->argreg <= 12)
        {
          int size, offset = 0;

          size = getSize (operandType (IC_LEFT (sic)));
          D (emitcode (";", "genSend argreg = %d, size = %d ", sic->argreg, size));
          if (sendCount == 0)
            {
              bool pushedA = FALSE;
              /* first parameter */
              // we know that dpl(hxb) is the result, so
              rb1_count = 0;
              _startLazyDPSEvaluation ();
              if (size > 1)
                {
                  aopOp (IC_LEFT (sic), sic, FALSE, (IS_OP_RUONLY (IC_LEFT (sic)) ? FALSE : TRUE));
                }
              else
                {
                  aopOp (IC_LEFT (sic), sic, FALSE, FALSE);
                }
              while (size--)
                {
                  const char *l = aopGet (IC_LEFT (sic), offset, FALSE, FALSE, NULL);
                  if (!EQ (l, fReturn[offset]))
                    if (fReturn[offset][0] == 'r' && (AOP_TYPE (IC_LEFT (sic)) == AOP_REG || AOP_TYPE (IC_LEFT (sic)) == AOP_R0 || AOP_TYPE (IC_LEFT (sic)) == AOP_R1)) 
                      emitcode ("mov", "a%s,%s", fReturn[offset], l); // use register's direct address instead of name
                    else
                      emitcode ("mov", "%s,%s", fReturn[offset], l);
                    if (size && (strcmp(fReturn[offset],"a")==0) && AOP_NEEDSACC( IC_LEFT (sic)))
                      {
                        emitpush ("acc");
                        pushedA = TRUE;
                      }
                  offset++;
                }
              if (pushedA)
                {
                  emitpop ("acc");
                  pushedA = FALSE;
                }
              _endLazyDPSEvaluation ();
              freeAsmop (IC_LEFT (sic), NULL, sic, TRUE);
              rb1_count = 0;
            }
          else
            {
              /* if more parameter in registers */
              aopOp (IC_LEFT (sic), sic, FALSE, TRUE);
              while (size--)
                {
                  emitcode ("mov", "b1_%d,%s", rb1_count++, aopGet (IC_LEFT (sic), offset, FALSE, FALSE, NULL));
                  offset++;
                }
              freeAsmop (IC_LEFT (sic), NULL, sic, TRUE);
            }
          sendCount++;
        }
    }
}

static void
adjustEsp (const char *reg)
{
  emitcode ("anl", "%s,#3", reg);
  if (TARGET_IS_DS400)
    {
      emitcode ("orl", "%s,#!constbyte", reg, (options.stack_loc >> 8) & 0xff);
    }
}

/*-----------------------------------------------------------------*/
/* selectRegBank - emit code to select the register bank           */
/*-----------------------------------------------------------------*/
static void
selectRegBank (short bank, bool keepFlags)
{
  /* if f.e. result is in carry */
  if (keepFlags)
    {
      emitcode ("anl", "psw,#0xE7");
      if (bank)
        emitcode ("orl", "psw,#0x%02x", (bank << 3) & 0xff);
    }
  else
    {
      emitcode ("mov", "psw,#0x%02x", (bank << 3) & 0xff);
    }
}

/*-----------------------------------------------------------------*/
/* genCall - generates a call statement                            */
/*-----------------------------------------------------------------*/
static void
genCall (iCode * ic)
{
  sym_link *dtype;
  sym_link *etype;
  bool restoreBank = FALSE;
  bool swapBanks = FALSE;
  bool accuse = FALSE;
  bool accPushed = FALSE;
  bool resultInF0 = FALSE;
  bool assignResultGenerated = FALSE;

  D (emitcode (";", "genCall"));

  /* if we are calling a not _naked function that is not using
     the same register bank then we need to save the
     destination registers on the stack */
  dtype = operandType (IC_LEFT (ic));
  etype = getSpec (dtype);
  if (currFunc && dtype && (!IFFUNC_ISNAKED (dtype) || TARGET_IS_DS400) &&
      (FUNC_REGBANK (currFunc->type) != FUNC_REGBANK (dtype)) && IFFUNC_ISISR (currFunc->type))
    {
      if (!ic->bankSaved)
        {
          /* This is unexpected; the bank should have been saved in
           * genFunction.
           */
          saveRBank (FUNC_REGBANK (dtype), ic, FALSE);
          restoreBank = TRUE;
        }
      swapBanks = TRUE;
    }

  /* if caller saves & we have not saved then */
  if (!ic->regsSaved)
    saveRegisters (ic);

  /* if send set is not empty then assign */
  /* We've saved all the registers we care about;
   * therefore, we may clobber any register not used
   * in the calling convention (i.e. anything not in
   * fReturn.
   */
  if (_G.sendSet)
    {
      if (IFFUNC_ISREENT (dtype))
        {
          /* need to reverse the send set */
          genSend (reverseSet (_G.sendSet));
        }
      else
        {
          genSend (_G.sendSet);
        }
      _G.sendSet = NULL;
    }

  if (swapBanks)
    {
      emitcode ("mov", "psw,#!constbyte", ((FUNC_REGBANK (dtype)) << 3) & 0xff);
    }

  /* make the call */
  if (IS_LITERAL (etype))
    {
      if (options.model == MODEL_FLAT24)
        emitcode ("lcall", "0x%06X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
      else
        emitcode ("lcall", "0x%04X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
    }
  else
    {
      emitcode ("lcall", "%s", (OP_SYMBOL (IC_LEFT (ic))->rname[0] ?
                                OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name));
    }

  if (swapBanks)
    {
      selectRegBank (FUNC_REGBANK (currFunc->type), IS_BIT (etype));
    }

  /* if we need assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic)) &&
       !IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))) &&
       (OP_SYMBOL (IC_RESULT (ic))->nRegs ||
        OP_SYMBOL (IC_RESULT (ic))->accuse || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {
      if (isOperandInFarSpace (IC_RESULT (ic)) && getSize (operandType (IC_RESULT (ic))) <= 2)
        {
          int size = getSize (operandType (IC_RESULT (ic)));
          bool pushedB = FALSE;

          /* Special case for 1 or 2 byte return in far space. */
          MOVA (fReturn[0]);
          if (size > 1)
            {
              pushedB = pushB ();
              emitcode ("mov", "b,%s", fReturn[1]);
            }

          _G.accInUse++;
          aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
          _G.accInUse--;

          popB (pushedB);

          aopPut (IC_RESULT (ic), "a", 0);

          if (size > 1)
            {
              aopPut (IC_RESULT (ic), "b", 1);
            }
          assignResultGenerated = TRUE;
          freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
        }
      else
        {
          bool pushedB = pushB ();
          aopOp (IC_RESULT (ic), ic, FALSE, TRUE);
          popB (pushedB);

          accuse = assignResultValue (IC_RESULT (ic), IC_LEFT (ic));
          assignResultGenerated = TRUE;
          freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
        }
    }

  /* adjust the stack for parameters if required */
  if (ic->parmBytes)
    {
      int i;
      if (options.stack10bit)
        {
          if (ic->parmBytes <= 10)
            {
              emitcode (";", "stack adjustment for parms");
              for (i = 0; i < ic->parmBytes; i++)
                {
                  emitpop ("acc");
                }
            }
          else
            {
              PROTECT_SP;
              emitcode ("clr", "c");
              emitcode ("mov", "a,sp");
              emitcode ("subb", "a,#!constbyte", ic->parmBytes & 0xff);
              emitcode ("mov", "sp,a");
              emitcode ("mov", "a,esp");
              adjustEsp ("a");
              emitcode ("subb", "a,#!constbyte", (ic->parmBytes >> 8) & 0xff);
              emitcode ("mov", "esp,a");
              UNPROTECT_SP;
            }
        }
      else
        {
          if (ic->parmBytes > 3)
            {
              if (accuse)
                {
                  emitcode ("push", "acc");
                  accPushed = TRUE;
                }
              if (IS_BIT (OP_SYM_ETYPE (IC_LEFT (ic))) && IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))) && !assignResultGenerated)
                {
                  emitcode ("mov", "F0,c");
                  resultInF0 = TRUE;
                }

              emitcode ("mov", "a,%s", spname);
              emitcode ("add", "a,#!constbyte", (-ic->parmBytes) & 0xff);
              emitcode ("mov", "%s,a", spname);

              /* unsaveRegisters from xstack needs acc, but */
              /* unsaveRegisters from stack needs this popped */
              if (accPushed && !options.useXstack)
                {
                  emitpop ("acc");
                  accPushed = FALSE;
                }
            }
          else
            for (i = 0; i < ic->parmBytes; i++)
              emitcode ("dec", "%s", spname);
        }
    }

  /* if we had saved some registers then unsave them */
  if (ic->regsSaved && !IFFUNC_CALLEESAVES (dtype))
    {
      if (accuse && !accPushed && options.useXstack)
        {
          /* xstack needs acc, but doesn't touch normal stack */
          emitpush ("acc");
          accPushed = TRUE;
        }
      unsaveRegisters (ic);
    }

  /* if register bank was saved then pop them */
  if (restoreBank)
    unsaveRBank (FUNC_REGBANK (dtype), ic, FALSE);

  if (IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))) && !assignResultGenerated)
    {
      if (resultInF0)
        emitcode ("mov", "c,F0");

      aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
      assignResultValue (IC_RESULT (ic), IC_LEFT (ic));
      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }

  if (accPushed)
    emitpop ("acc");
}

/*-----------------------------------------------------------------*/
/* genPcall - generates a call by pointer statement                */
/*-----------------------------------------------------------------*/
static void
genPcall (iCode * ic)
{
  sym_link *dtype;
  symbol *rlbl = newiTempLabel (NULL);
  bool restoreBank = FALSE;
  bool resultInF0 = FALSE;

  D (emitcode (";", "genPcall"));

  dtype = operandType (IC_LEFT (ic))->next;
  /* if caller saves & we have not saved then */
  if (!ic->regsSaved)
    saveRegisters (ic);

  /* if we are calling a not _naked function that is not using
     the same register bank then we need to save the
     destination registers on the stack */
  if (currFunc && dtype && (!IFFUNC_ISNAKED (dtype) || TARGET_IS_DS400) &&
      IFFUNC_ISISR (currFunc->type) && (FUNC_REGBANK (currFunc->type) != FUNC_REGBANK (dtype)))
    {
      saveRBank (FUNC_REGBANK (dtype), ic, TRUE);
      restoreBank = TRUE;
    }

  /* push the return address on to the stack */
  emitcode ("mov", "a,#!tlabel", labelKey2num (rlbl->key));
  emitpush ("acc");
  emitcode ("mov", "a,#!hil", labelKey2num (rlbl->key));
  emitpush ("acc");

  if (options.model == MODEL_FLAT24)
    {
      emitcode ("mov", "a,#!hihil", labelKey2num (rlbl->key));
      emitpush ("acc");
    }

  /* now push the function address */
  pushSide (IC_LEFT (ic), FARPTRSIZE, ic);

  /* if send set is not empty then assign */
  if (_G.sendSet)
    {
      genSend (reverseSet (_G.sendSet));
      _G.sendSet = NULL;
    }

  /* make the call */
  emitcode ("ret", "");
  emitLabel (rlbl);

  /* if we need assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic)) &&
       !IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))) &&
       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {
      _G.accInUse++;
      aopOp (IC_RESULT (ic), ic, FALSE, TRUE);
      _G.accInUse--;

      assignResultValue (IC_RESULT (ic), IC_LEFT (ic));

      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }

  /* adjust the stack for parameters if required */
  if (ic->parmBytes)
    {
      int i;
      if (options.stack10bit)
        {
          if (ic->parmBytes <= 10)
            {
              emitcode (";", "stack adjustment for parms");
              for (i = 0; i < ic->parmBytes; i++)
                {
                  emitpop ("acc");
                }
            }
          else
            {
              if (IS_BIT (OP_SYM_ETYPE (IC_LEFT (ic))) && IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))))
                {
                  emitcode ("mov", "F0,c");
                  resultInF0 = TRUE;
                }

              PROTECT_SP;
              emitcode ("clr", "c");
              emitcode ("mov", "a,sp");
              emitcode ("subb", "a,#!constbyte", ic->parmBytes & 0xff);
              emitcode ("mov", "sp,a");
              emitcode ("mov", "a,esp");
              adjustEsp ("a");
              emitcode ("subb", "a,#!constbyte", (ic->parmBytes >> 8) & 0xff);
              emitcode ("mov", "esp,a");
              UNPROTECT_SP;
            }
        }
      else
        {
          if (ic->parmBytes > 3)
            {
              if (IS_BIT (OP_SYM_ETYPE (IC_LEFT (ic))) && IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))))
                {
                  emitcode ("mov", "F0,c");
                  resultInF0 = TRUE;
                }

              emitcode ("mov", "a,%s", spname);
              emitcode ("add", "a,#!constbyte", (-ic->parmBytes) & 0xff);
              emitcode ("mov", "%s,a", spname);
            }
          else
            for (i = 0; i < ic->parmBytes; i++)
              emitcode ("dec", "%s", spname);
        }
    }
  /* if register bank was saved then unsave them */
  if (restoreBank)
    unsaveRBank (FUNC_REGBANK (dtype), ic, TRUE);

  /* if we had saved some registers then unsave them */
  if (ic->regsSaved)
    unsaveRegisters (ic);

  if (IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))))
    {
      if (resultInF0)
        emitcode ("mov", "c,F0");

      aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
      assignResultValue (IC_RESULT (ic), IC_LEFT (ic));
      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }
}

/*-----------------------------------------------------------------*/
/* resultRemat - result  is rematerializable                       */
/*-----------------------------------------------------------------*/
static int
resultRemat (iCode * ic)
{
  if (SKIP_IC (ic) || ic->op == IFX)
    return 0;

  if (IC_RESULT (ic) && IS_ITEMP (IC_RESULT (ic)))
    {
      symbol *sym = OP_SYMBOL (IC_RESULT (ic));
      if (sym->remat && !POINTER_SET (ic))
        return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* inExcludeList - return 1 if the string is in exclude Reg list   */
/*-----------------------------------------------------------------*/
static int
regsCmp (void *p1, void *p2)
{
  return (STRCASECMP ((char *) p1, (char *) (p2)) == 0);
}

static bool
inExcludeList (char *s)
{
  const char *p = setFirstItem (options.excludeRegsSet);

  if (p == NULL || STRCASECMP (p, "none") == 0)
    return FALSE;

  return isinSetWith (options.excludeRegsSet, s, regsCmp);
}

/*-----------------------------------------------------------------*/
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void
genFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  sym_link *ftype;
  bool switchedPSW = FALSE;
  bool fReentrant = (IFFUNC_ISREENT (sym->type) || options.stackAuto);

  D (emitcode (";", "genFunction"));

  _G.nRegsSaved = 0;
  /* create the function header */
  emitcode (";", "-----------------------------------------");
  emitcode (";", " function %s", sym->name);
  emitcode (";", "-----------------------------------------");

  emitcode ("", "%s:", sym->rname);
  genLine.lineCurr->isLabel = 1;
  ftype = operandType (IC_LEFT (ic));
  _G.currentFunc = sym;

  if (IFFUNC_ISNAKED (ftype))
    {
      emitcode (";", "naked function: no prologue.");
      return;
    }

  if (options.stack_probe)
    emitcode ("lcall", "__stack_probe");

  /* here we need to generate the equates for the
     register bank if required */
  if (FUNC_REGBANK (ftype) != rbank)
    {
      int i;

      rbank = FUNC_REGBANK (ftype);
      for (i = 0; i < ds390_nRegs; i++)
        {
          if (regs390[i].print)
            {
              if (strcmp (regs390[i].base, "0") == 0)
                emitcode ("", "%s !equ !constbyte", regs390[i].dname, 8 * rbank + regs390[i].offset);
              else
                emitcode ("", "%s !equ %s + !constbyte", regs390[i].dname, regs390[i].base, 8 * rbank + regs390[i].offset);
            }
        }
    }

  /* if this is an interrupt service routine then
     save acc, b, dpl, dph  */
  if (IFFUNC_ISISR (sym->type))
    {
      if (!inExcludeList ("acc"))
        emitpush ("acc");
      if (!inExcludeList ("b"))
        emitpush ("b");
      if (!inExcludeList ("dpl"))
        emitpush ("dpl");
      if (!inExcludeList ("dph"))
        emitpush ("dph");
      if (options.model == MODEL_FLAT24 && !inExcludeList ("dpx"))
        {
          emitpush ("dpx");
          /* Make sure we're using standard DPTR */
          emitpush ("dps");
          emitcode ("mov", "dps,#0");
          if (options.stack10bit)
            {
              /* This ISR could conceivably use DPTR2. Better save it. */
              emitpush ("dpl1");
              emitpush ("dph1");
              emitpush ("dpx1");
              emitpush (DP2_RESULT_REG);
            }
        }
      /* if this isr has no bank i.e. is going to
         run with bank 0 , then we need to save more
         registers :-) */
      if (!FUNC_REGBANK (sym->type))
        {
          int i;

          /* if this function does not call any other
             function then we can be economical and
             save only those registers that are used */
          if (!IFFUNC_HASFCALL (sym->type))
            {
              /* if any registers used */
              if (!bitVectIsZero (sym->regsUsed))
                {
                  bool bits_pushed = FALSE;
                  /* save the registers used */
                  for (i = 0; i < sym->regsUsed->size; i++)
                    {
                      if (bitVectBitValue (sym->regsUsed, i))
                        bits_pushed = pushReg (i, bits_pushed);
                    }
                }
            }
          else
            {
              /* this function has a function call. We cannot
                 determine register usage so we will have to push the
                 entire bank */
              saveRBank (0, ic, FALSE);
              if (options.parms_in_bank1)
                {
                  for (i = 0; i < 8; i++)
                    {
                      emitpush (rb1regs[i]);
                    }
                }
            }
        }
      else
        {
          /* This ISR uses a non-zero bank.
           *
           * We assume that the bank is available for our
           * exclusive use.
           *
           * However, if this ISR calls a function which uses some
           * other bank, we must save that bank entirely.
           */
          unsigned long banksToSave = 0;

          if (IFFUNC_HASFCALL (sym->type))
            {

#define MAX_REGISTER_BANKS 4

              iCode *i;
              int ix;

              for (i = ic; i; i = i->next)
                {
                  sym_link *dtype = NULL;

                  if (i->op == ENDFUNCTION)
                    {
                      /* we got to the end OK. */
                      break;
                    }

                  if (i->op == CALL)
                    {
                      dtype = operandType (IC_LEFT (i));
                    }
                  if (i->op == PCALL)
                    {
                      /* This is a mess; we have no idea what
                       * register bank the called function might
                       * use.
                       *
                       * The only thing I can think of to do is
                       * throw a warning and hope.
                       */
//                      werror (W_FUNCPTR_IN_USING_ISR);
                      dtype = operandType (IC_LEFT (i))->next;
                    }
                  if (dtype && FUNC_REGBANK (dtype) != FUNC_REGBANK (sym->type))
                    {
                      /* Mark this bank for saving. */
                      if (FUNC_REGBANK (dtype) >= MAX_REGISTER_BANKS)
                        {
                          werror (E_NO_SUCH_BANK, FUNC_REGBANK (dtype));
                        }
                      else
                        {
                          banksToSave |= (1 << FUNC_REGBANK (dtype));
                        }

                      /* And note that we don't need to do it in
                       * genCall.
                       */
                      i->bankSaved = 1;
                    }
                }

              if (banksToSave && options.useXstack)
                {
                  /* Since we aren't passing it an ic,
                   * saveRBank will assume r0 is available to abuse.
                   *
                   * So switch to our (trashable) bank now, so
                   * the caller's R0 isn't trashed.
                   */
                  emitpush ("psw");
                  emitcode ("mov", "psw,#!constbyte", (FUNC_REGBANK (sym->type) << 3) & 0x00ff);
                  switchedPSW = TRUE;
                }

              for (ix = 0; ix < MAX_REGISTER_BANKS; ix++)
                {
                  if (banksToSave & (1 << ix))
                    {
                      saveRBank (ix, NULL, FALSE);
                    }
                }
            }
          // TODO: this needs a closer look
          SPEC_ISR_SAVED_BANKS (currFunc->etype) = banksToSave;
        }
    }
  else
    {
      /* if callee-save to be used for this function
         then save the registers being used in this function */
      if (IFFUNC_CALLEESAVES (sym->type))
        {
          int i;

          /* if any registers used */
          if (sym->regsUsed)
            {
              bool bits_pushed = FALSE;
              /* save the registers used */
              for (i = 0; i < sym->regsUsed->size; i++)
                {
                  if (bitVectBitValue (sym->regsUsed, i))
                    {
                      bits_pushed = pushReg (i, bits_pushed);
                      _G.nRegsSaved++;
                    }
                }
            }
        }
    }

  /* set the register bank to the desired value */
  if ((FUNC_REGBANK (sym->type) || FUNC_ISISR (sym->type)) && !switchedPSW)
    {
      emitpush ("psw");
      emitcode ("mov", "psw,#!constbyte", (FUNC_REGBANK (sym->type) << 3) & 0x00ff);
    }

  if (fReentrant && (sym->stack || FUNC_HASSTACKPARM (sym->type)))
    {
      if (options.stack10bit)
        {
          emitcode ("push", "_bpx");
          emitcode ("push", "_bpx+1");
          emitcode ("mov", "_bpx,%s", spname);
          emitcode ("mov", "_bpx+1,esp");
          adjustEsp ("_bpx+1");
        }
      else
        {
          if (options.useXstack)
            {
              emitcode ("mov", "r0,%s", spname);
              emitcode ("mov", "a,_bp");
              emitcode ("movx", "@r0,a");
              emitcode ("inc", "%s", spname);
            }
          else
            {
              /* set up the stack */
              emitcode ("push", "_bp"); /* save the callers stack  */
            }
          emitcode ("mov", "_bp,%s", spname);
        }
    }

  /* adjust the stack for the function */
  if (sym->stack)
    {
      int i = sym->stack;
      if (options.stack10bit)
        {
          if (i > 1024)
            werror (W_STACK_OVERFLOW, sym->name);
          assert (sym->recvSize <= 4);
          if (sym->stack <= 8)
            {
              while (i--)
                emitcode ("push", "acc");
            }
          else
            {
              PROTECT_SP;
              emitcode ("mov", "a,sp");
              emitcode ("add", "a,#!constbyte", ((short) sym->stack & 0xff));
              emitcode ("mov", "sp,a");
              emitcode ("mov", "a,esp");
              adjustEsp ("a");
              emitcode ("addc", "a,#!constbyte", (((short) sym->stack) >> 8) & 0xff);
              emitcode ("mov", "esp,a");
              UNPROTECT_SP;
            }
        }
      else
        {
          if (i > 256)
            werror (W_STACK_OVERFLOW, sym->name);

          if (i > 3 && sym->recvSize < 4)
            {

              emitcode ("mov", "a,sp");
              emitcode ("add", "a,#!constbyte", ((char) sym->stack & 0xff));
              emitcode ("mov", "sp,a");

            }
          else
            {
              while (i--)
                emitcode ("inc", "sp");
            }
        }
    }

  if (sym->xstack)
    {

      emitcode ("mov", "a,_spx");
      emitcode ("add", "a,#!constbyte", ((char) sym->xstack & 0xff));
      emitcode ("mov", "_spx,a");
    }

  /* if critical function then turn interrupts off */
  if (IFFUNC_ISCRITICAL (ftype))
    {
      symbol *tlbl = newiTempLabel (NULL);
      emitcode ("setb", "c");
      emitcode ("jbc", "ea,!tlabel", labelKey2num (tlbl->key)); /* atomic test & clear */
      emitcode ("clr", "c");
      emitLabel (tlbl);
      emitpush ("psw");         /* save old ea via c in psw */
    }
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  lineNode *lnp = genLine.lineCurr;
  bitVect *regsUsed;
  bitVect *regsUsedPrologue;
  bitVect *regsUnneeded;
  int idx;

  D (emitcode (";", "genEndFunction"));

  _G.currentFunc = NULL;
  if (IFFUNC_ISNAKED (sym->type))
    {
      emitcode (";", "naked function: no epilogue.");
      if (options.debug && currFunc)
        debugFile->writeEndFunction (currFunc, ic, 0);
      return;
    }

  if (IFFUNC_ISCRITICAL (sym->type))
    {
      if (IS_BIT (OP_SYM_ETYPE (IC_LEFT (ic))))
        {
          emitcode ("rlc", "a");        /* save c in a */
          emitpop ("psw");      /* restore ea via c in psw */
          emitcode ("mov", "ea,c");
          emitcode ("rrc", "a");        /* restore c from a */
        }
      else
        {
          emitpop ("psw");      /* restore ea via c in psw */
          emitcode ("mov", "ea,c");
        }
    }

  if ((IFFUNC_ISREENT (sym->type) || options.stackAuto) && (sym->stack || FUNC_HASSTACKPARM (sym->type)))
    {

      if (options.stack10bit)
        {
          PROTECT_SP;
          emitcode ("mov", "sp,_bpx", spname);
          emitcode ("mov", "esp,_bpx+1", spname);
          UNPROTECT_SP;
        }
      else
        {
          emitcode ("mov", "%s,_bp", spname);
        }
    }

  /* if use external stack but some variables were
     added to the local stack then decrement the
     local stack */
  if (options.useXstack && sym->stack)
    {
      emitcode ("mov", "a,sp");
      emitcode ("add", "a,#!constbyte", ((char) - sym->stack) & 0xff);
      emitcode ("mov", "sp,a");
    }


  if ((IFFUNC_ISREENT (sym->type) || options.stackAuto) && (sym->stack || FUNC_HASSTACKPARM (sym->type)))
    {

      if (options.useXstack)
        {
          emitcode ("mov", "r0,%s", spname);
          emitcode ("movx", "a,@r0");
          emitcode ("mov", "_bp,a");
          emitcode ("dec", "%s", spname);
        }
      else
        {
          if (options.stack10bit)
            {
              emitcode ("pop", "_bpx+1");
              emitcode ("pop", "_bpx");
            }
          else
            {
              emitcode ("pop", "_bp");
            }
        }
    }

  /* restore the register bank  */
  if (FUNC_REGBANK (sym->type) || IFFUNC_ISISR (sym->type))
    {
      if (!FUNC_REGBANK (sym->type) || !IFFUNC_ISISR (sym->type) || !options.useXstack)
        {
          /* Special case of ISR using non-zero bank with useXstack
           * is handled below.
           */
          emitpop ("psw");
        }
    }

  if (IFFUNC_ISISR (sym->type))
    {

      /* now we need to restore the registers */
      /* if this isr has no bank i.e. is going to
         run with bank 0 , then we need to save more
         registers :-) */
      if (!FUNC_REGBANK (sym->type))
        {
          int i;
          /* if this function does not call any other
             function then we can be economical and
             save only those registers that are used */
          if (!IFFUNC_HASFCALL (sym->type))
            {
              /* if any registers used */
              if (sym->regsUsed)
                {
                  bool bits_popped = FALSE;
                  /* restore the registers used */
                  for (i = sym->regsUsed->size; i >= 0; i--)
                    {
                      if (bitVectBitValue (sym->regsUsed, i))
                        bits_popped = popReg (i, bits_popped);
                    }
                }
            }
          else
            {
              /* this function has a function call. We cannot
                 determine register usage so we will have to pop the
                 entire bank */
              if (options.parms_in_bank1)
                {
                  for (i = 7; i >= 0; i--)
                    {
                      emitpop (rb1regs[i]);
                    }
                }
              unsaveRBank (0, ic, FALSE);
            }
        }
      else
        {
          /* This ISR uses a non-zero bank.
           *
           * Restore any register banks saved by genFunction
           * in reverse order.
           */
          unsigned savedBanks = SPEC_ISR_SAVED_BANKS (currFunc->etype);
          int ix;

          for (ix = MAX_REGISTER_BANKS - 1; ix >= 0; ix--)
            {
              if (savedBanks & (1 << ix))
                {
                  unsaveRBank (ix, NULL, FALSE);
                }
            }

          if (options.useXstack)
            {
              /* Restore bank AFTER calling unsaveRBank,
               * since it can trash r0.
               */
              emitpop ("psw");
            }
        }

      if (options.model == MODEL_FLAT24 && !inExcludeList ("dpx"))
        {
          if (options.stack10bit)
            {
              emitpop (DP2_RESULT_REG);
              emitpop ("dpx1");
              emitpop ("dph1");
              emitpop ("dpl1");
            }
          emitpop ("dps");
          emitpop ("dpx");
        }
      if (!inExcludeList ("dph"))
        emitpop ("dph");
      if (!inExcludeList ("dpl"))
        emitpop ("dpl");
      if (!inExcludeList ("b"))
        emitpop ("b");
      if (!inExcludeList ("acc"))
        emitpop ("acc");

      /* if debug then send end of function */
      if (options.debug && currFunc)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      emitcode ("reti", "");
    }
  else
    {
      if (IFFUNC_CALLEESAVES (sym->type))
        {
          int i;

          /* if any registers used */
          if (sym->regsUsed)
            {
              /* save the registers used */
              for (i = sym->regsUsed->size; i >= 0; i--)
                {
                  if (bitVectBitValue (sym->regsUsed, i))
                    emitpop (REG_WITH_INDEX (i)->dname);
                }
            }
        }

      /* if debug then send end of function */
      if (options.debug && currFunc)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      emitcode ("ret", "");
    }

  if (!port->peep.getRegsRead || !port->peep.getRegsWritten || options.nopeep)
    return;

  /* If this was an interrupt handler using bank 0 that called another */
  /* function, then all registers must be saved; nothing to optimize.  */
  if (IFFUNC_ISISR (sym->type) && IFFUNC_HASFCALL (sym->type) && !FUNC_REGBANK (sym->type))
    return;

  /* There are no push/pops to optimize if not callee-saves or ISR */
  if (!(FUNC_CALLEESAVES (sym->type) || FUNC_ISISR (sym->type)))
    return;

  /* If there were stack parameters, we cannot optimize without also    */
  /* fixing all of the stack offsets; this is too dificult to consider. */
  if (FUNC_HASSTACKPARM (sym->type))
    return;

  /* Compute the registers actually used */
  regsUsed = newBitVect (ds390_nRegs);
  regsUsedPrologue = newBitVect (ds390_nRegs);
  while (lnp)
    {
      if (lnp->ic && lnp->ic->op == FUNCTION)
        regsUsedPrologue = bitVectUnion (regsUsedPrologue, port->peep.getRegsWritten (lnp));
      else
        regsUsed = bitVectUnion (regsUsed, port->peep.getRegsWritten (lnp));

      if (lnp->ic && lnp->ic->op == FUNCTION && lnp->prev && lnp->prev->ic && lnp->prev->ic->op == ENDFUNCTION)
        break;
      if (!lnp->prev)
        break;
      lnp = lnp->prev;
    }

  if (bitVectBitValue (regsUsedPrologue, DPS_IDX) && !bitVectBitValue (regsUsed, DPS_IDX))
    {
      bitVectUnSetBit (regsUsedPrologue, DPS_IDX);
    }

  if (bitVectBitValue (regsUsedPrologue, CND_IDX) && !bitVectBitValue (regsUsed, CND_IDX))
    {
      regsUsed = bitVectUnion (regsUsed, regsUsedPrologue);
      if (IFFUNC_ISISR (sym->type) && !FUNC_REGBANK (sym->type) && !sym->stack && !FUNC_ISCRITICAL (sym->type))
        bitVectUnSetBit (regsUsed, CND_IDX);
    }
  else
    regsUsed = bitVectUnion (regsUsed, regsUsedPrologue);

  /* If this was an interrupt handler that called another function */
  /* function, then assume working registers may be modified by it. */
  if (IFFUNC_ISISR (sym->type) && IFFUNC_HASFCALL (sym->type))
    {
      regsUsed = bitVectSetBit (regsUsed, AP_IDX);
      regsUsed = bitVectSetBit (regsUsed, DPX1_IDX);
      regsUsed = bitVectSetBit (regsUsed, DPL1_IDX);
      regsUsed = bitVectSetBit (regsUsed, DPH1_IDX);
      regsUsed = bitVectSetBit (regsUsed, DPX_IDX);
      regsUsed = bitVectSetBit (regsUsed, DPL_IDX);
      regsUsed = bitVectSetBit (regsUsed, DPH_IDX);
      regsUsed = bitVectSetBit (regsUsed, DPS_IDX);
      regsUsed = bitVectSetBit (regsUsed, B_IDX);
      regsUsed = bitVectSetBit (regsUsed, A_IDX);
      regsUsed = bitVectSetBit (regsUsed, CND_IDX);
    }

  /* Remove the unneeded push/pops */
  regsUnneeded = newBitVect (ds390_nRegs);
  while (lnp)
    {
      if (lnp->ic && (lnp->ic->op == FUNCTION || lnp->ic->op == ENDFUNCTION))
        {
          if (!strncmp (lnp->line, "push", 4))
            {
              idx = bitVectFirstBit (port->peep.getRegsRead (lnp));
              if (idx >= 0 && !bitVectBitValue (regsUsed, idx))
                {
                  connectLine (lnp->prev, lnp->next);
                  regsUnneeded = bitVectSetBit (regsUnneeded, idx);
                }
            }
          if (!strncmp (lnp->line, "pop", 3) || !strncmp (lnp->line, "mov", 3))
            {
              idx = bitVectFirstBit (port->peep.getRegsWritten (lnp));
              if (idx >= 0 && !bitVectBitValue (regsUsed, idx))
                {
                  connectLine (lnp->prev, lnp->next);
                  regsUnneeded = bitVectSetBit (regsUnneeded, idx);
                }
            }
        }
      lnp = lnp->next;
    }

  for (idx = 0; idx < regsUnneeded->size; idx++)
    if (bitVectBitValue (regsUnneeded, idx))
      emitcode (";", "eliminated unneeded push/pop %s", REG_WITH_INDEX (idx)->dname);

  freeBitVect (regsUnneeded);
  freeBitVect (regsUsed);
  freeBitVect (regsUsedPrologue);
}

/*-----------------------------------------------------------------*/
/* genJavaNativeRet - generate code for return JavaNative          */
/*-----------------------------------------------------------------*/
static void
genJavaNativeRet (iCode * ic)
{
  int i, size;

  aopOp (IC_LEFT (ic), ic, FALSE, IS_OP_RUONLY (IC_LEFT (ic)) ? FALSE : TRUE);
  size = AOP_SIZE (IC_LEFT (ic));

  assert (size <= 4);

  /* it is assigned to GPR0-R3 then push them */
  if (aopHasRegs (AOP (IC_LEFT (ic)), R0_IDX, R1_IDX) || aopHasRegs (AOP (IC_LEFT (ic)), R2_IDX, R3_IDX))
    {
      for (i = 0; i < size; i++)
        {
          emitcode ("push", "%s", aopGet (IC_LEFT (ic), i, FALSE, TRUE, DP2_RESULT_REG));
        }
      for (i = (size - 1); i >= 0; i--)
        {
          emitcode ("pop", "a%s", javaRet[i]);
        }
    }
  else
    {
      for (i = 0; i < size; i++)
        emitcode ("mov", "%s,%s", javaRet[i], aopGet (IC_LEFT (ic), i, FALSE, TRUE, DP2_RESULT_REG));
    }
  for (i = size; i < 4; i++)
    emitcode ("mov", "%s,#0", javaRet[i]);
  return;
}

/*-----------------------------------------------------------------*/
/* genRet - generate code for return statement                     */
/*-----------------------------------------------------------------*/
static void
genRet (iCode * ic)
{
  int size, offset = 0, pushed = 0;
  bool pushedA = FALSE;

  D (emitcode (";", "genRet"));

  /* if we have no return value then
     just generate the "ret" */
  if (!IC_LEFT (ic))
    goto jumpret;

  /* if this is a JavaNative function then return
     value in different register */
  if (IFFUNC_ISJAVANATIVE (currFunc->type))
    {
      genJavaNativeRet (ic);
      goto jumpret;
    }
  /* we have something to return then
     move the return value into place */
  aopOp (IC_LEFT (ic), ic, FALSE, (IS_OP_RUONLY (IC_LEFT (ic)) ? FALSE : TRUE));
  size = AOP_SIZE (IC_LEFT (ic));

  _startLazyDPSEvaluation ();

  if (IS_BIT (_G.currentFunc->etype))
    {
      if (!IS_OP_RUONLY (IC_LEFT (ic)))
        toCarry (IC_LEFT (ic));
      _endLazyDPSEvaluation ();
    }
  else
    {
      while (size--)
        {
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_DPTR)
            {
              emitpush (aopGet (IC_LEFT (ic), offset++, FALSE, TRUE, NULL));
              pushed++;
            }
          else
            {
              const char *l = aopGet (IC_LEFT (ic), offset, FALSE, FALSE, NULL);
              if (!EQ (fReturn[offset], l))
                if (fReturn[offset][0] == 'r' && (AOP_TYPE (IC_LEFT (ic)) == AOP_REG || AOP_TYPE (IC_LEFT (ic)) == AOP_R0 || AOP_TYPE (IC_LEFT (ic)) == AOP_R1)) 
                  emitcode ("mov", "a%s,%s", fReturn[offset], l); // use register's direct address instead of name
                else
                  emitcode ("mov", "%s,%s", fReturn[offset], l);
              if (size && !strcmp(fReturn[offset], "a") && aopGetUsesAcc (IC_LEFT (ic), offset+1))
                {
                  emitpush ("acc");
                  pushedA = TRUE;
                }
              offset++;
            }
        }
      _endLazyDPSEvaluation ();

      if (pushedA)
        {
           emitpop ("acc");
        }

      while (pushed)
        {
          pushed--;
          if (!EQ (fReturn[pushed], "a"))
            emitpop (fReturn[pushed]);
          else
            emitpop ("acc");
        }
    }
  freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);

jumpret:
  /* generate a jump to the return label
     if the next is not the return statement */
  if (!(ic->next && ic->next->op == LABEL && IC_LABEL (ic->next) == returnLabel))
    {
      emitcode ("ljmp", "!tlabel", labelKey2num (returnLabel->key));
    }
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void
genLabel (iCode * ic)
{
  /* special case never generate */
  if (IC_LABEL (ic) == entryLabel)
    return;

  D (emitcode (";", "genLabel"));

  emitLabel (IC_LABEL (ic));
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a ljmp                                      */
/*-----------------------------------------------------------------*/
static void
genGoto (iCode * ic)
{
  D (emitcode (";", "genGoto"));

  emitcode ("ljmp", "!tlabel", labelKey2num (IC_LABEL (ic)->key));
}

/*-----------------------------------------------------------------*/
/* findLabelBackwards: walks back through the iCode chain looking  */
/* for the given label. Returns number of iCode instructions       */
/* between that label and given ic.                                */
/* Returns zero if label not found.                                */
/*-----------------------------------------------------------------*/
static int
findLabelBackwards (iCode * ic, int key)
{
  int count = 0;

  while (ic->prev)
    {
      ic = ic->prev;
      count++;

      /* If we have any pushes or pops, we cannot predict the distance.
         I don't like this at all, this should be dealt with in the
         back-end */
      if (ic->op == IPUSH || ic->op == IPOP)
        {
          return 0;
        }

      if (ic->op == LABEL && IC_LABEL (ic)->key == key)
        {
          /* printf("findLabelBackwards = %d\n", count); */
          return count;
        }
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* genPlusIncr :- does addition with increment if possible         */
/*-----------------------------------------------------------------*/
static bool
genPlusIncr (iCode *ic)
{
  unsigned long long icount;
  unsigned int size = getDataSize (IC_RESULT (ic)), offset;

  /* will try to generate an increment */
  /* if the right side is not a literal
     we cannot */
  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    return FALSE;

  /* if the literal value of the right hand side
     is greater than 4 then it is not worth it */
  if ((icount = ullFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit)) > 4)
    return FALSE;

  if (size == 1 && AOP (IC_LEFT (ic)) == AOP (IC_RESULT (ic)) && AOP_TYPE (IC_LEFT (ic)) == AOP_DIR)
    {
      while (icount--)
        {
          emitcode ("inc", "%s", aopGet (IC_RESULT (ic), 0, FALSE, FALSE, NULL));
        }
      return TRUE;
    }
  /* if increment 16 bits in register */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG &&
      AOP_TYPE (IC_RESULT (ic)) == AOP_REG &&
      sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))) && (size > 1) && (icount == 1))
    {
      symbol *tlbl;
      int emitTlbl;
      int labelRange;
      const char *l;

      /* If the next instruction is a goto and the goto target
       * is <= 5 instructions previous to this, we can generate
       * jumps straight to that target.
       */
      if (ic->next && ic->next->op == GOTO
          && (labelRange = findLabelBackwards (ic, IC_LABEL (ic->next)->key)) != 0 && labelRange <= 5)
        {
          D (emitcode (";", "tail increment optimized (range %d)", labelRange));
          tlbl = IC_LABEL (ic->next);
          emitTlbl = 0;
        }
      else
        {
          tlbl = newiTempLabel (NULL);
          emitTlbl = 1;
        }
      l = aopGet (IC_RESULT (ic), LSB, FALSE, FALSE, NULL);
      emitcode ("inc", "%s", l);
      if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG || IS_AOP_PREG (IC_RESULT (ic)))
        {
          emitcode ("cjne", "%s,%s,!tlabel", l, zero, labelKey2num (tlbl->key));
        }
      else
        {
          emitcode ("clr", "a");
          emitcode ("cjne", "a,%s,!tlabel", l, labelKey2num (tlbl->key));
        }

      l = aopGet (IC_RESULT (ic), MSB16, FALSE, FALSE, NULL);
      emitcode ("inc", "%s", l);
      for(offset = 2; size > 2; size--, offset++)
        {
          if (EQ (l, "acc"))
            {
              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
            }
          else if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG || IS_AOP_PREG (IC_RESULT (ic)))
            {
              emitcode ("cjne", "%s,%s,!tlabel", l, zero, labelKey2num (tlbl->key));
            }
          else
            {
              emitcode ("cjne", "a,%s,!tlabel", l, labelKey2num (tlbl->key));
            }

          l = aopGet (IC_RESULT (ic), offset, FALSE, FALSE, NULL);
          emitcode ("inc", "%s", l);
        }

      if (emitTlbl)
        {
          emitLabel (tlbl);
        }
      return TRUE;
    }

  if (AOP_TYPE (IC_RESULT (ic)) == AOP_STR && IS_ITEMP (IC_RESULT (ic)) &&
      !AOP_USESDPTR (IC_LEFT (ic)) && icount <= 5 && size <= 3 && options.model == MODEL_FLAT24)
    {
      if (AOP_SIZE (IC_RESULT (ic)) == 4)
        {
          emitcode ("mov", "b,%s", aopGet (IC_LEFT (ic), 3, FALSE, FALSE, NULL));
        }
      switch (size)
        {
        case 3:
          emitcode ("mov", "dpx,%s", aopGet (IC_LEFT (ic), 2, FALSE, FALSE, NULL));
        case 2:
          emitcode ("mov", "dph,%s", aopGet (IC_LEFT (ic), 1, FALSE, FALSE, NULL));
        case 1:
          emitcode ("mov", "dpl,%s", aopGet (IC_LEFT (ic), 0, FALSE, FALSE, NULL));
          break;
        }
      while (icount--)
        emitcode ("inc", "dptr");

      return TRUE;
    }

  if (AOP_INDPTRn (IC_LEFT (ic)) && AOP_INDPTRn (IC_RESULT (ic)) &&
      AOP (IC_LEFT (ic))->aopu.dptr == AOP (IC_RESULT (ic))->aopu.dptr && icount <= 5)
    {
      emitcode ("mov", "dps,#!constbyte", AOP (IC_LEFT (ic))->aopu.dptr);
      while (icount--)
        emitcode ("inc", "dptr");
      emitcode ("mov", "dps,#0");
      return TRUE;
    }

  /* if the sizes are greater than 1 then we cannot */
  if (AOP_SIZE (IC_RESULT (ic)) > 1 || AOP_SIZE (IC_LEFT (ic)) > 1)
    return FALSE;

  /* we can if the aops of the left & result match or
     if they are in registers and the registers are the
     same */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG &&
      AOP_TYPE (IC_RESULT (ic)) == AOP_REG && sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
    {
      if (icount > 3)
        {
          MOVA (aopGet (IC_LEFT (ic), 0, FALSE, FALSE, NULL));
          emitcode ("add", "a,#!constbyte", ((char) icount) & 0xff);
          aopPut (IC_RESULT (ic), "a", 0);
        }
      else
        {
          _startLazyDPSEvaluation ();
          while (icount--)
            {
              emitcode ("inc", "%s", aopGet (IC_LEFT (ic), 0, FALSE, FALSE, NULL));
            }
          _endLazyDPSEvaluation ();
        }

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
  symbol *tlbl = newiTempLabel (NULL);
  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      aopPut (result, "a", 0);
    }
  else
    {
      emitcode ("jz", "!tlabel", labelKey2num (tlbl->key));
      emitcode ("mov", "a,%s", one);
      emitLabel (tlbl);
      outAcc (result);
    }
}

/*-----------------------------------------------------------------*/
/* genPlusBits - generates code for addition of two bits           */
/*-----------------------------------------------------------------*/
static void
genPlusBits (iCode * ic)
{
  D (emitcode (";", "genPlusBits"));

  emitcode ("mov", "c,%s", AOP (IC_LEFT (ic))->aopu.aop_dir);
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY)
    {
      symbol *lbl = newiTempLabel (NULL);
      emitcode ("jnb", "%s,!tlabel", AOP (IC_RIGHT (ic))->aopu.aop_dir, labelKey2num (lbl->key));
      emitcode ("cpl", "c");
      emitLabel (lbl);
      outBitC (IC_RESULT (ic));
    }
  else
    {
      emitcode ("clr", "a");
      emitcode ("rlc", "a");
      emitcode ("mov", "c,%s", AOP (IC_RIGHT (ic))->aopu.aop_dir);
      emitcode ("addc", "a,%s", zero);
      outAcc (IC_RESULT (ic));
    }
}

static void
adjustArithmeticResult (iCode * ic)
{
  if (opIsGptr (IC_RESULT (ic)) && opIsGptr (IC_LEFT (ic)) && !sameRegs (AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic))))
    {
      aopPut (IC_RESULT (ic), aopGet (IC_LEFT (ic), GPTRSIZE - 1, FALSE, FALSE, NULL), GPTRSIZE - 1);
    }

  if (opIsGptr (IC_RESULT (ic)) && opIsGptr (IC_RIGHT (ic)) && !sameRegs (AOP (IC_RESULT (ic)), AOP (IC_RIGHT (ic))))
    {
      aopPut (IC_RESULT (ic), aopGet (IC_RIGHT (ic), GPTRSIZE - 1, FALSE, FALSE, NULL), GPTRSIZE - 1);
    }

  if (opIsGptr (IC_RESULT (ic)) &&
      IC_LEFT (ic) && AOP_SIZE (IC_LEFT (ic)) < GPTRSIZE &&
      IC_RIGHT (ic) && AOP_SIZE (IC_RIGHT (ic)) < GPTRSIZE &&
      !sameRegs (AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic))) && !sameRegs (AOP (IC_RESULT (ic)), AOP (IC_RIGHT (ic))))
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      dbuf_printf (&dbuf, "#%02x", pointerTypeToGPByte (pointerCode (getSpec (operandType (IC_LEFT (ic)))), NULL, NULL));
      aopPut (IC_RESULT (ic), dbuf_c_str (&dbuf), GPTRSIZE - 1);
      dbuf_destroy (&dbuf);
    }
}

// The guts of AOP_OP_3_NOFATAL. Generates the left & right opcodes of an IC,
// generates the result if possible. If result is generated, returns TRUE; otherwise
// returns false and caller must deal with fact that result isn't aopOp'd.
bool
aopOp3 (iCode * ic)
{
  bool dp1InUse, dp2InUse;
  bool useDp2;

  // First, generate the right opcode. DPTR may be used if neither left nor result are
  // of type AOP_STR.

//    D (emitcode(";", "aopOp3: IS_OP_RUONLY left: %s right: %s result: %s",
//             IS_OP_RUONLY(IC_LEFT(ic)) ? "true" : "false",
//             IS_OP_RUONLY(IC_RIGHT(ic)) ? "true" : "false",
//             IS_OP_RUONLY(IC_RESULT(ic)) ? "true" : "false");
//      );
//    D (emitcode(";", "aopOp3: AOP_IS_DPTRn left: %s right: %s result: %s",
//             AOP_IS_DPTRn(IC_LEFT(ic)) ? "true" : "false",
//             AOP_IS_DPTRn(IC_RIGHT(ic)) ? "true" : "false",
//             AOP_IS_DPTRn(IC_RESULT(ic)) ? "true" : "false");
//      );

  // Right uses DPTR unless left or result is an AOP_STR; however,
  // if right is an AOP_STR, it must use DPTR regardless.
  if ((IS_OP_RUONLY (IC_LEFT (ic)) || IS_OP_RUONLY (IC_RESULT (ic))) && !IS_OP_RUONLY (IC_RIGHT (ic)))
    {
      useDp2 = TRUE;
    }
  else
    {
      useDp2 = FALSE;
    }

  aopOp (IC_RIGHT (ic), ic, FALSE, useDp2);

  // if the right used DPTR, left MUST use DPTR2.
  // if the right used DPTR2, left MUST use DPTR.
  // if both are still available, we prefer to use DPTR. But if result is an AOP_STR
  // and left is not an AOP_STR, then we will get better code if we use DP2 for left,
  // enabling us to assign DPTR to result.

  if (AOP_USESDPTR (IC_RIGHT (ic)))
    {
      useDp2 = TRUE;
    }
  else if (AOP_USESDPTR2 (IC_RIGHT (ic)))
    {
      useDp2 = FALSE;
    }
  else
    {
      if (IS_OP_RUONLY (IC_RESULT (ic)) && !IS_OP_RUONLY (IC_LEFT (ic)))
        {
          useDp2 = TRUE;
        }
      else
        {
          useDp2 = FALSE;
        }
    }

  aopOp (IC_LEFT (ic), ic, FALSE, useDp2);


  // We've op'd the left & right. So, if left or right are the same operand as result,
  // we know aopOp will succeed, and we can just do it & bail.
  if (isOperandEqual (IC_LEFT (ic), IC_RESULT (ic)))
    {
      aopOp (IC_RESULT (ic), ic, TRUE, AOP_USESDPTR2 (IC_LEFT (ic)));
      return TRUE;
    }
  if (isOperandEqual (IC_RIGHT (ic), IC_RESULT (ic)))
    {
//      D (emitcode(";", "aopOp3: (left | right) & result equal"));
      aopOp (IC_RESULT (ic), ic, TRUE, AOP_USESDPTR2 (IC_RIGHT (ic)));
      return TRUE;
    }

  // Operands may be equivalent (but not equal) if they share a spill location. If
  // so, use the same DPTR or DPTR2.
  if (operandsEqu (IC_LEFT (ic), IC_RESULT (ic)))
    {
      aopOp (IC_RESULT (ic), ic, TRUE, AOP_USESDPTR2 (IC_LEFT (ic)));
      return TRUE;
    }
  if (operandsEqu (IC_RIGHT (ic), IC_RESULT (ic)))
    {
      aopOp (IC_RESULT (ic), ic, TRUE, AOP_USESDPTR2 (IC_RIGHT (ic)));
      return TRUE;
    }

  // Note which dptrs are currently in use.
  dp1InUse = AOP_USESDPTR (IC_LEFT (ic)) || AOP_USESDPTR (IC_RIGHT (ic));
  dp2InUse = AOP_USESDPTR2 (IC_LEFT (ic)) || AOP_USESDPTR2 (IC_RIGHT (ic));

  // OK, now if either left or right uses DPTR and the result is an AOP_STR, we cannot
  // generate it.
  if (dp1InUse && IS_OP_RUONLY (IC_RESULT (ic)))
    {
      return FALSE;
    }

  // Likewise, if left or right uses DPTR2 and the result is a DPTRn, we cannot generate it.
  if (dp2InUse && AOP_IS_DPTRn (IC_RESULT (ic)))
    {
      return FALSE;
    }

  // or, if both dp1 & dp2 are in use and the result needs a dptr, we're out of luck
  if (dp1InUse && dp2InUse && isOperandInFarSpace (IC_RESULT (ic)))
    {
      return FALSE;
    }

  aopOp (IC_RESULT (ic), ic, TRUE, dp1InUse);

  // Some sanity checking...
  if (dp1InUse && AOP_USESDPTR (IC_RESULT (ic)))
    {
      fprintf (stderr, "Internal error: got unexpected DPTR (%s:%d %s:%d)\n", __FILE__, __LINE__, ic->filename, ic->lineno);
      emitcode (";", ">>> unexpected DPTR here.");
    }

  if (dp2InUse && AOP_USESDPTR2 (IC_RESULT (ic)))
    {
      fprintf (stderr, "Internal error: got unexpected DPTR2 (%s:%d %s:%d)\n", __FILE__, __LINE__, ic->filename, ic->lineno);
      emitcode (";", ">>> unexpected DPTR2 here.");
    }

  return TRUE;
}

// Macro to aopOp all three operands of an ic. If this cannot be done,
// the IC_LEFT and IC_RIGHT operands will be aopOp'd, and the rc parameter
// will be set TRUE. The caller must then handle the case specially, noting
// that the IC_RESULT operand is not aopOp'd.
//
#define AOP_OP_3_NOFATAL(ic, rc) \
            do { rc = !aopOp3(ic); } while (0)

// aopOp the left & right operands of an ic.
#define AOP_OP_2(ic) \
    aopOp (IC_RIGHT (ic), ic, FALSE, IS_OP_RUONLY (IC_LEFT (ic))); \
    aopOp (IC_LEFT (ic), ic, FALSE, AOP_USESDPTR (IC_RIGHT (ic)));

// convienience macro.
#define AOP_SET_LOCALS(ic) \
    left = IC_LEFT(ic); \
    right = IC_RIGHT(ic); \
    result = IC_RESULT(ic);


// Given an integer value of pushedSize bytes on the stack,
// adjust it to be resultSize bytes, either by discarding
// the most significant bytes or by zero-padding.
//
// On exit from this macro, pushedSize will have been adjusted to
// equal resultSize, and ACC may be trashed.
#define ADJUST_PUSHED_RESULT(pushedSize, resultSize)            \
      /* If the pushed data is bigger than the result,          \
       * simply discard unused bytes. Icky, but works.          \
       */                                                       \
      while (pushedSize > resultSize)                           \
      {                                                         \
          D (emitcode (";", "discarding unused result byte.")); \
          emitcode ("pop", "acc");                              \
          pushedSize--;                                         \
      }                                                         \
      if (pushedSize < resultSize)                              \
      {                                                         \
          emitcode ("clr", "a");                                \
          /* Conversly, we haven't pushed enough here.          \
           * just zero-pad, and all is well.                    \
           */                                                   \
          while (pushedSize < resultSize)                       \
          {                                                     \
              emitcode("push", "acc");                          \
              pushedSize++;                                     \
          }                                                     \
      }                                                         \
      assert(pushedSize == resultSize);

/*-----------------------------------------------------------------*/
/* genPlus - generates code for addition                           */
/*-----------------------------------------------------------------*/
static void
genPlus (iCode * ic)
{
  int size, offset = 0;
  bool pushResult;
  int rSize;
  bool swappedLR = FALSE;

  D (emitcode (";", "genPlus"));

  /* special cases :- */
  if (IS_OP_RUONLY (IC_LEFT (ic)) && isOperandLiteral (IC_RIGHT (ic)) && IS_OP_RUONLY (IC_RESULT (ic)))
    {
      aopOp (IC_RIGHT (ic), ic, TRUE, FALSE);
      size = (int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);
      if (size <= 9)
        {
          while (size--)
            emitcode ("inc", "dptr");
        }
      else
        {
          emitcode ("mov", "a,dpl");
          emitcode ("add", "a,#!constbyte", size & 0xff);
          emitcode ("mov", "dpl,a");
          emitcode ("mov", "a,dph");
          emitcode ("addc", "a,#!constbyte", (size >> 8) & 0xff);
          emitcode ("mov", "dph,a");
          emitcode ("mov", "a,dpx");
          emitcode ("addc", "a,#!constbyte", (size >> 16) & 0xff);
          emitcode ("mov", "dpx,a");
        }
      freeAsmop (IC_RIGHT (ic), NULL, ic, FALSE);
      return;
    }
  if (IS_SYMOP (IC_LEFT (ic)) && OP_SYMBOL (IC_LEFT (ic))->remat && isOperandInFarSpace (IC_RIGHT (ic)))
    {
      operand *op = IC_RIGHT (ic);
      IC_RIGHT (ic) = IC_LEFT (ic);
      IC_LEFT (ic) = op;
    }

  AOP_OP_3_NOFATAL (ic, pushResult);

  if (pushResult)
    {
      D (emitcode (";", "genPlus: must push result: 3 ops in far space"));
    }

  if (!pushResult)
    {
      /* if literal, literal on the right or
         if left requires ACC or right is already
         in ACC */
      if ((AOP_TYPE (IC_LEFT (ic)) == AOP_LIT) ||
          ((AOP_NEEDSACC (IC_LEFT (ic))) && !(AOP_NEEDSACC (IC_RIGHT (ic)))) || AOP_TYPE (IC_RIGHT (ic)) == AOP_ACC)
        {
          operand *t = IC_RIGHT (ic);
          IC_RIGHT (ic) = IC_LEFT (ic);
          IC_LEFT (ic) = t;
          swappedLR = TRUE;
          D (emitcode (";", "Swapped plus args."));
        }

      /* if both left & right are in bit space */
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_CRY && AOP_TYPE (IC_RIGHT (ic)) == AOP_CRY)
        {
          genPlusBits (ic);
          goto release;
        }

      /* if left in bit space & right literal */
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_CRY && AOP_TYPE (IC_RIGHT (ic)) == AOP_LIT)
        {
          emitcode ("mov", "c,%s", AOP (IC_LEFT (ic))->aopu.aop_dir);
          /* if result in bit space */
          if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY)
            {
              if (ullFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit))
                emitcode ("cpl", "c");
              outBitC (IC_RESULT (ic));
            }
          else
            {
              size = getDataSize (IC_RESULT (ic));
              _startLazyDPSEvaluation ();
              while (size--)
                {
                  MOVA (aopGet (IC_RIGHT (ic), offset, FALSE, FALSE, NULL));
                  emitcode ("addc", "a,%s", zero);
                  aopPut (IC_RESULT (ic), "a", offset++);
                }
              _endLazyDPSEvaluation ();
            }
          goto release;
        }

      /* if I can do an increment instead
         of add then GOOD for ME */
      if (genPlusIncr (ic) == TRUE)
        {
          D (emitcode (";", "did genPlusIncr"));
          goto release;
        }

    }
  size = getDataSize (pushResult ? IC_LEFT (ic) : IC_RESULT (ic));

  _startLazyDPSEvaluation ();
  while (size--)
    {
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_ACC && !AOP_NEEDSACC (IC_RIGHT (ic)))
        {
          MOVA (aopGet (IC_LEFT (ic), offset, FALSE, FALSE, NULL));
          if (offset == 0)
            emitcode ("add", "a,%s", aopGet (IC_RIGHT (ic), offset, FALSE, FALSE, NULL));
          else
            emitcode ("addc", "a,%s", aopGet (IC_RIGHT (ic), offset, FALSE, FALSE, NULL));
        }
      else
        {
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_ACC && (offset == 0))
            {
              /* right is going to use ACC or we would have taken the
               * above branch.
               */
              assert (AOP_NEEDSACC (IC_RIGHT (ic)));
              TR_AP ("#3");
              D (emitcode (";", "+ AOP_ACC special case."););
              emitcode ("xch", "a, %s", DP2_RESULT_REG);
            }
          MOVA (aopGet (IC_RIGHT (ic), offset, FALSE, FALSE, NULL));
          if (offset == 0)
            {
              if (AOP_TYPE (IC_LEFT (ic)) == AOP_ACC)
                {
                  TR_AP ("#4");
                  emitcode ("add", "a, %s", DP2_RESULT_REG);
                }
              else
                {
                  emitcode ("add", "a,%s", aopGet (IC_LEFT (ic), offset, FALSE, FALSE, DP2_RESULT_REG));
                }
            }
          else
            {
              emitcode ("addc", "a,%s", aopGet (IC_LEFT (ic), offset, FALSE, FALSE, DP2_RESULT_REG));
            }
        }
      if (!pushResult)
        {
          aopPut (IC_RESULT (ic), "a", offset);
        }
      else
        {
          emitcode ("push", "acc");
        }
      offset++;
    }
  _endLazyDPSEvaluation ();

  if (pushResult)
    {
      aopOp (IC_RESULT (ic), ic, TRUE, FALSE);
      adjustArithmeticResult (ic);

      size = getDataSize (IC_LEFT (ic));
      rSize = getDataSize (IC_RESULT (ic));

      ADJUST_PUSHED_RESULT (size, rSize);

      _startLazyDPSEvaluation ();
      while (size--)
        {
          emitcode ("pop", "acc");
          aopPut (IC_RESULT (ic), "a", size);
        }
      _endLazyDPSEvaluation ();
    }
  else
    adjustArithmeticResult (ic);

release:
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
  if (swappedLR)
    swapOperands (&IC_LEFT (ic), &IC_RIGHT (ic));
  freeAsmop (IC_RIGHT (ic), NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}

/*-----------------------------------------------------------------*/
/* genMinusDec :- does subtraction with decrement if possible      */
/*-----------------------------------------------------------------*/
static bool
genMinusDec (iCode * ic)
{
  unsigned long long icount;
  unsigned int size = getDataSize (IC_RESULT (ic));

  /* will try to generate a decrement */
  /* if the right side is not a literal
     we cannot */
  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    return FALSE;

  /* if the literal value of the right hand side
     is greater than 4 then it is not worth it */
  if ((icount = ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit)) > 4)
    return FALSE;

  if (size == 1 && AOP (IC_LEFT (ic)) == AOP (IC_RESULT (ic)) && AOP_TYPE (IC_LEFT (ic)) == AOP_DIR)
    {
      while (icount--)
        {
          emitcode ("dec", "%s", aopGet (IC_RESULT (ic), 0, FALSE, FALSE, NULL));
        }
      return TRUE;
    }
  /* if decrement 16 bits in register */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG &&
      AOP_TYPE (IC_RESULT (ic)) == AOP_REG &&
      sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))) && (size > 1) && (icount == 1))
    {
      symbol *tlbl;
      int emitTlbl;
      int labelRange;
      const char *l;

      /* If the next instruction is a goto and the goto target
       * is <= 5 instructions previous to this, we can generate
       * jumps straight to that target.
       */
      if (ic->next && ic->next->op == GOTO
          && (labelRange = findLabelBackwards (ic, IC_LABEL (ic->next)->key)) != 0 && labelRange <= 5)
        {
          D (emitcode (";", "tail decrement optimized (range %d)", labelRange));
          tlbl = IC_LABEL (ic->next);
          emitTlbl = 0;
        }
      else
        {
          tlbl = newiTempLabel (NULL);
          emitTlbl = 1;
        }

      l = aopGet (IC_RESULT (ic), LSB, FALSE, FALSE, NULL);
      emitcode ("dec", "%s", l);

      if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG || AOP_TYPE (IC_RESULT (ic)) == AOP_DPTR || IS_AOP_PREG (IC_RESULT (ic)))
        {
          emitcode ("cjne", "%s,#!constbyte,!tlabel", l, 0xff, labelKey2num (tlbl->key));
        }
      else
        {
          emitcode ("mov", "a,#!constbyte", 0xff);
          emitcode ("cjne", "a,%s,!tlabel", l, labelKey2num (tlbl->key));
        }
      l = aopGet (IC_RESULT (ic), MSB16, FALSE, FALSE, NULL);
      emitcode ("dec", "%s", l);
      if (size > 2)
        {
          if (EQ (l, "acc"))
            {
              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
            }
          else if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG ||
                   AOP_TYPE (IC_RESULT (ic)) == AOP_DPTR || IS_AOP_PREG (IC_RESULT (ic)))
            {
              emitcode ("cjne", "%s,#!constbyte,!tlabel", l, 0xff, labelKey2num (tlbl->key));
            }
          else
            {
              emitcode ("cjne", "a,%s,!tlabel", l, labelKey2num (tlbl->key));
            }
          l = aopGet (IC_RESULT (ic), MSB24, FALSE, FALSE, NULL);
          emitcode ("dec", "%s", l);
        }
      if (size > 3)
        {
          if (EQ (l, "acc"))
            {
              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
            }
          else if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG ||
                   AOP_TYPE (IC_RESULT (ic)) == AOP_DPTR || IS_AOP_PREG (IC_RESULT (ic)))
            {
              emitcode ("cjne", "%s,#!constbyte,!tlabel", l, 0xff, labelKey2num (tlbl->key));
            }
          else
            {
              emitcode ("cjne", "a,%s,!tlabel", l, labelKey2num (tlbl->key));
            }
          emitcode ("dec", "%s", aopGet (IC_RESULT (ic), MSB32, FALSE, FALSE, NULL));
        }
      if (emitTlbl)
        {
          emitLabel (tlbl);
        }
      return TRUE;
    }

  /* if the sizes are greater than 1 then we cannot */
  if (AOP_SIZE (IC_RESULT (ic)) > 1 || AOP_SIZE (IC_LEFT (ic)) > 1)
    return FALSE;

  /* we can if the aops of the left & result match or
     if they are in registers and the registers are the
     same */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG &&
      AOP_TYPE (IC_RESULT (ic)) == AOP_REG && sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
    {
      const char *l;

      if (aopGetUsesAcc (IC_LEFT (ic), 0))
        {
          MOVA (aopGet (IC_RESULT (ic), 0, FALSE, FALSE, NULL));
          l = "a";
        }
      else
        {
          l = aopGet (IC_RESULT (ic), 0, FALSE, FALSE, NULL);
        }

      _startLazyDPSEvaluation ();
      while (icount--)
        {
          emitcode ("dec", "%s", l);
        }
      _endLazyDPSEvaluation ();

      if (AOP_NEEDSACC (IC_RESULT (ic)))
        aopPut (IC_RESULT (ic), "a", 0);

      return TRUE;
    }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* addSign - complete with sign                                    */
/*-----------------------------------------------------------------*/
static void
addSign (operand * result, int offset, int sign)
{
  int size = (getDataSize (result) - offset);
  if (size > 0)
    {
      _startLazyDPSEvaluation ();
      if (sign)
        {
          emitcode ("rlc", "a");
          emitcode ("subb", "a,acc");
          while (size--)
            {
              aopPut (result, "a", offset++);
            }
        }
      else
        {
          while (size--)
            {
              aopPut (result, zero, offset++);
            }
        }
      _endLazyDPSEvaluation ();
    }
}

/*-----------------------------------------------------------------*/
/* genMinusBits - generates code for subtraction  of two bits      */
/*-----------------------------------------------------------------*/
static void
genMinusBits (iCode * ic)
{
  symbol *lbl = newiTempLabel (NULL);

  D (emitcode (";", "genMinusBits"));

  if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY)
    {
      emitcode ("mov", "c,%s", AOP (IC_LEFT (ic))->aopu.aop_dir);
      emitcode ("jnb", "%s,!tlabel", AOP (IC_RIGHT (ic))->aopu.aop_dir, labelKey2num (lbl->key));
      emitcode ("cpl", "c");
      emitLabel (lbl);
      outBitC (IC_RESULT (ic));
    }
  else
    {
      emitcode ("mov", "c,%s", AOP (IC_RIGHT (ic))->aopu.aop_dir);
      emitcode ("subb", "a,acc");
      emitcode ("jnb", "%s,!tlabel", AOP (IC_LEFT (ic))->aopu.aop_dir, labelKey2num ((lbl->key)));
      emitcode ("inc", "a");
      emitLabel (lbl);
      aopPut (IC_RESULT (ic), "a", 0);
      addSign (IC_RESULT (ic), MSB16, SPEC_USIGN (getSpec (operandType (IC_RESULT (ic)))));
    }
}

/*-----------------------------------------------------------------*/
/* genMinus - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
static void
genMinus (iCode * ic)
{
  int size, offset = 0;
  int rSize;
  long long lit = 0L;
  bool pushResult;

  D (emitcode (";", "genMinus"));

  AOP_OP_3_NOFATAL (ic, pushResult);

  if (!pushResult)
    {
      /* special cases :- */
      /* if both left & right are in bit space */
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_CRY && AOP_TYPE (IC_RIGHT (ic)) == AOP_CRY)
        {
          genMinusBits (ic);
          goto release;
        }

      /* if I can do a decrement instead
         of subtract then GOOD for ME */
      if (genMinusDec (ic) == TRUE)
        goto release;

    }

  size = getDataSize (pushResult ? IC_LEFT (ic) : IC_RESULT (ic));

  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    {
      CLRC;
    }
  else
    {
      lit = ullFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);
      lit = -lit;
    }


  /* if literal, add a,#-lit, else normal subb */
  _startLazyDPSEvaluation ();
  while (size--)
    {
      if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
        {
          if (AOP_USESDPTR (IC_RIGHT (ic)))
            {
              emitcode ("mov", "b,%s", aopGet (IC_RIGHT (ic), offset, FALSE, FALSE, NULL));
              MOVA (aopGet (IC_LEFT (ic), offset, FALSE, FALSE, NULL));
              emitcode ("subb", "a,b");
            }
          else
            {
              MOVA (aopGet (IC_LEFT (ic), offset, FALSE, FALSE, NULL));
              emitcode ("subb", "a,%s", aopGet (IC_RIGHT (ic), offset, FALSE, FALSE, DP2_RESULT_REG));
            }
        }
      else
        {
          MOVA (aopGet (IC_LEFT (ic), offset, FALSE, FALSE, NULL));
          /* first add without previous c */
          if (!offset)
            {
              if (!size && lit == -1)
                {
                  emitcode ("dec", "a");
                }
              else
                {
                  emitcode ("add", "a,#!constbyte", (unsigned int) (lit & 0x0FFL));
                }
            }
          else
            {
              emitcode ("addc", "a,#!constbyte", (unsigned int) ((lit >> (offset * 8)) & 0x0FFL));
            }
        }

      if (pushResult)
        {
          emitcode ("push", "acc");
        }
      else
        {
          aopPut (IC_RESULT (ic), "a", offset);
        }
      offset++;
    }
  _endLazyDPSEvaluation ();

  if (pushResult)
    {
      aopOp (IC_RESULT (ic), ic, TRUE, FALSE);
      adjustArithmeticResult (ic);

      size = getDataSize (IC_LEFT (ic));
      rSize = getDataSize (IC_RESULT (ic));

      ADJUST_PUSHED_RESULT (size, rSize);

      _startLazyDPSEvaluation ();
      while (size--)
        {
          emitcode ("pop", "acc");
          aopPut (IC_RESULT (ic), "a", size);
        }
      _endLazyDPSEvaluation ();
    }
  else
    adjustArithmeticResult (ic);

release:
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
  freeAsmop (IC_RIGHT (ic), NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}


/*-----------------------------------------------------------------*/
/* genMultbits :- multiplication of bits                           */
/*-----------------------------------------------------------------*/
static void
genMultbits (operand * left, operand * right, operand * result, iCode * ic)
{
  D (emitcode (";", "genMultbits"));

  emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
  emitcode ("anl", "c,%s", AOP (right)->aopu.aop_dir);
  aopOp (result, ic, TRUE, FALSE);
  outBitC (result);
}

/*-----------------------------------------------------------------*/
/* genMultOneByte : 8*8=8/16 bit multiplication                    */
/*-----------------------------------------------------------------*/
static void
genMultOneByte (operand * left, operand * right, operand * result, iCode * ic)
{
  symbol *lbl;
  int size, offset = 0;
  bool runtimeSign, compiletimeSign;
  bool lUnsigned, rUnsigned, pushedB;

  /* (if two literals: the value is computed before) */
  /* if one literal, literal on the right */
  if (AOP_TYPE (left) == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
      /* emitcode (";", "swapped left and right"); */
    }
  /* if no literal, unsigned on the right: shorter code */
  if (AOP_TYPE (right) != AOP_LIT && SPEC_USIGN (getSpec (operandType (left))))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  lUnsigned = SPEC_USIGN (getSpec (operandType (left)));
  rUnsigned = SPEC_USIGN (getSpec (operandType (right)));

  pushedB = pushB ();

  if ((lUnsigned && rUnsigned)
      /* sorry, I don't know how to get size
         without calling aopOp (result,...);
         see Feature Request  */
      /* || size == 1 */ )
    /* no, this is not a bug; with a 1 byte result there's
       no need to take care about the signedness! */
    {
      /* just an unsigned 8 * 8 = 8 multiply
         or 8u * 8u = 16u */
      /* emitcode (";","unsigned"); */
      emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE, NULL));
      MOVA (aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("mul", "ab");

      _G.accInUse++;
      aopOp (result, ic, TRUE, FALSE);
      size = AOP_SIZE (result);

      if (size < 1)
        {
          /* this should never happen */
          fprintf (stderr, "size!=1||2 (%d) in %s at line:%d \n", size, __FILE__, lineno);
          exit (EXIT_FAILURE);
        }

      aopPut (result, "a", offset++);
      _G.accInUse--;
      if (size != 1)
        aopPut (result, "b", offset++);

      while (size-- > 2)
        aopPut (result, zero, offset++);

      popB (pushedB);
      return;
    }

  /* we have to do a signed multiply */
  /* emitcode (";", "signed"); */

  /* now sign adjust for both left & right */

  /* let's see what's needed: */
  /* apply negative sign during runtime */
  runtimeSign = FALSE;
  /* negative sign from literals */
  compiletimeSign = FALSE;

  if (!lUnsigned)
    {
      if (AOP_TYPE (left) == AOP_LIT)
        {
          /* signed literal */
          signed char val = (char) ulFromVal (AOP (left)->aopu.aop_lit);
          if (val < 0)
            compiletimeSign = TRUE;
        }
      else
        /* signed but not literal */
        runtimeSign = TRUE;
    }

  if (!rUnsigned)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* signed literal */
          signed char val = (char) ulFromVal (AOP (right)->aopu.aop_lit);
          if (val < 0)
            compiletimeSign ^= TRUE;
        }
      else
        /* signed but not literal */
        runtimeSign = TRUE;
    }

  /* initialize F0, which stores the runtime sign */
  if (runtimeSign)
    {
      if (compiletimeSign)
        emitcode ("setb", "F0");        /* set sign flag */
      else
        emitcode ("clr", "F0");         /* reset sign flag */
    }

  /* save the signs of the operands */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (right)->aopu.aop_lit);

      if (!rUnsigned && val < 0)
        emitcode ("mov", "b,#!constbyte", -val);
      else
        emitcode ("mov", "b,#!constbyte", (unsigned char) val);
    }
  else                          /* ! literal */
    {
      if (rUnsigned)            /* emitcode (";", "signed"); */
        emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE, NULL));
      else
        {
          MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "F0");       /* complement sign flag */
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
          emitcode ("mov", "b,a");
        }
    }

  if (AOP_TYPE (left) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (left)->aopu.aop_lit);

      if (!lUnsigned && val < 0)
        emitcode ("mov", "a,#!constbyte", -val);
      else
        emitcode ("mov", "a,#!constbyte", (unsigned char) val);
    }
  else                          /* ! literal */
    {
      MOVA (aopGet (left, 0, FALSE, FALSE, NULL));

      if (!lUnsigned)           /* emitcode (";", "signed"); */
        {
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "F0");       /* complement sign flag */
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
        }
    }

  /* now the multiplication */
  emitcode ("mul", "ab");
  _G.accInUse++;
  aopOp (result, ic, TRUE, FALSE);
  size = AOP_SIZE (result);

  if (size < 1)
    {
      /* this should never happen */
      fprintf (stderr, "size!=1||2 (%d) in %s at line:%d \n", size, __FILE__, lineno);
      exit (EXIT_FAILURE);
    }

  if (runtimeSign || compiletimeSign)
    {
      lbl = newiTempLabel (NULL);
      if (runtimeSign)
        emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");    /* lsb 2's complement */
      if (size == 1)
        emitcode ("inc", "a");  /* inc doesn't set carry flag */
      else
        {
          emitcode ("add", "a,#0x01");  /* this sets carry flag */
          emitcode ("xch", "a,b");
          emitcode ("cpl", "a");        /* msb 2's complement */
          emitcode ("addc", "a,#0x00");
          emitcode ("xch", "a,b");
        }
      emitLabel (lbl);
    }
  aopPut (result, "a", offset++);
  _G.accInUse--;
  if (size != 1)
    aopPut (result, "b", offset++);

  if (size > 2)
    {
      emitcode ("mov", "c,b.7");
      emitcode ("subb", "a,acc");
      _G.accInUse++;
      while (size-- > 2)
        aopPut (result, "a", offset++);
      _G.accInUse--;
    }

  popB (pushedB);
}

/*-----------------------------------------------------------------*/
/* genMultTwoByte - use the DS390 MAC unit to do 16*16 multiply    */
/*-----------------------------------------------------------------*/
static void
genMultTwoByte (operand * left, operand * right, operand * result, iCode * ic)
{
  sym_link *retype = getSpec (operandType (right));
  sym_link *letype = getSpec (operandType (left));
  int umult = SPEC_USIGN (retype) | SPEC_USIGN (letype);
  symbol *lbl;

  if (AOP_TYPE (left) == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
    }
  /* save EA bit in F1 */
  lbl = newiTempLabel (NULL);
  emitcode ("setb", "F1");
  emitcode ("jbc", "EA,!tlabel", labelKey2num (lbl->key));
  emitcode ("clr", "F1");
  emitLabel (lbl);

  /* load up MB with right */
  if (!umult)
    {
      emitcode ("clr", "F0");
      if (AOP_TYPE (right) == AOP_LIT)
        {
          int val = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          if (val < 0)
            {
              emitcode ("setb", "F0");
              val = -val;
            }
          emitcode ("mov", "mb,#!constbyte", val & 0xff);
          emitcode ("mov", "mb,#!constbyte", (val >> 8) & 0xff);
        }
      else
        {
          lbl = newiTempLabel (NULL);
          emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE, NULL));
          emitcode ("mov", "a,%s", aopGet (right, 1, FALSE, FALSE, NULL));
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("xch", "a,b");
          emitcode ("cpl", "a");
          emitcode ("add", "a,#1");
          emitcode ("xch", "a,b");
          emitcode ("cpl", "a");        // msb
          emitcode ("addc", "a,#0");
          emitcode ("setb", "F0");
          emitLabel (lbl);
          emitcode ("mov", "mb,b");
          emitcode ("mov", "mb,a");
        }
    }
  else
    {
      emitcode ("mov", "mb,%s", aopGet (right, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "mb,%s", aopGet (right, 1, FALSE, FALSE, NULL));
    }
  /* load up MA with left */
  if (!umult)
    {
      lbl = newiTempLabel (NULL);
      emitcode ("mov", "b,%s", aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "a,%s", aopGet (left, 1, FALSE, FALSE, NULL));
      emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
      emitcode ("xch", "a,b");
      emitcode ("cpl", "a");
      emitcode ("add", "a,#1");
      emitcode ("xch", "a,b");
      emitcode ("cpl", "a");    // msb
      emitcode ("addc", "a,#0");
      emitcode ("jbc", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("setb", "F0");
      emitLabel (lbl);
      emitcode ("mov", "ma,b");
      emitcode ("mov", "ma,a");
    }
  else
    {
      emitcode ("mov", "ma,%s", aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "ma,%s", aopGet (left, 1, FALSE, FALSE, NULL));
    }
  /* wait for multiplication to finish */
  lbl = newiTempLabel (NULL);
  emitLabel (lbl);
  emitcode ("mov", "a,mcnt1");
  emitcode ("anl", "a,#!constbyte", 0x80);
  emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));

  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (result, ic, TRUE, FALSE);

  /* if unsigned then simple */
  if (umult)
    {
      emitcode ("mov", "a,ma");
      if (AOP_SIZE (result) >= 4)
        aopPut (result, "a", 3);
      emitcode ("mov", "a,ma");
      if (AOP_SIZE (result) >= 3)
        aopPut (result, "a", 2);
      aopPut (result, "ma", 1);
      aopPut (result, "ma", 0);
    }
  else
    {
      emitcode ("push", "ma");
      emitcode ("push", "ma");
      emitcode ("push", "ma");
      MOVA ("ma");
      /* negate result if needed */
      lbl = newiTempLabel (NULL);
      emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");
      emitcode ("add", "a,#1");
      emitLabel (lbl);
      if (AOP_TYPE (result) == AOP_ACC)
        {
          D (emitcode (";", "ACC special case."));
          /* We know result is the only live aop, and
           * it's obviously not a DPTR2, so AP is available.
           */
          emitcode ("mov", "%s,acc", DP2_RESULT_REG);
        }
      else
        {
          aopPut (result, "a", 0);
        }

      emitcode ("pop", "acc");
      lbl = newiTempLabel (NULL);
      emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");
      emitcode ("addc", "a,#0");
      emitLabel (lbl);
      aopPut (result, "a", 1);
      emitcode ("pop", "acc");
      if (AOP_SIZE (result) >= 3)
        {
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "a");
          emitcode ("addc", "a,#0");
          emitLabel (lbl);
          aopPut (result, "a", 2);
        }
      emitcode ("pop", "acc");
      if (AOP_SIZE (result) >= 4)
        {
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "a");
          emitcode ("addc", "a,#0");
          emitLabel (lbl);
          aopPut (result, "a", 3);
        }
      if (AOP_TYPE (result) == AOP_ACC)
        {
          /* We stashed the result away above. */
          emitcode ("mov", "acc,%s", DP2_RESULT_REG);
        }

    }
  freeAsmop (result, NULL, ic, TRUE);

  /* restore EA bit in F1 */
  lbl = newiTempLabel (NULL);
  emitcode ("jnb", "F1,!tlabel", labelKey2num (lbl->key));
  emitcode ("setb", "EA");
  emitLabel (lbl);
  return;
}

/*-----------------------------------------------------------------*/
/* genMult - generates code for multiplication                     */
/*-----------------------------------------------------------------*/
static void
genMult (iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  D (emitcode (";", "genMult"));

  /* assign the asmops */
  AOP_OP_2 (ic);

  /* special cases first */
  /* both are bits */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      genMultbits (left, right, result, ic);
      goto release;
    }

  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
    {
      genMultOneByte (left, right, result, ic);
      goto release;
    }

  if (AOP_SIZE (left) == 2 && AOP_SIZE (right) == 2)
    {
      /* use the ds390 ARITHMETIC accel UNIT */
      genMultTwoByte (left, right, result, ic);
      return;
    }
  /* should have been converted to function call */
  assert (0);

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}

/*-----------------------------------------------------------------*/
/* genDivbits :- division of bits                                  */
/*-----------------------------------------------------------------*/
static void
genDivbits (operand * left, operand * right, operand * result, iCode * ic)
{
  bool pushedB;

  D (emitcode (";", "genDivbits"));

  pushedB = pushB ();

  /* the result must be bit */
  LOAD_AB_FOR_DIV (left, right);
  emitcode ("div", "ab");
  emitcode ("rrc", "a");
  aopOp (result, ic, TRUE, FALSE);

  popB (pushedB);

  aopPut (result, "c", 0);
}

/*-----------------------------------------------------------------*/
/* genDivOneByte : 8 bit division                                  */
/*-----------------------------------------------------------------*/
static void
genDivOneByte (operand * left, operand * right, operand * result, iCode * ic)
{
  bool lUnsigned, rUnsigned, pushedB;
  bool runtimeSign, compiletimeSign;
  symbol *lbl;
  int size, offset;

  D (emitcode (";", "genDivOneByte"));

  offset = 1;
  lUnsigned = SPEC_USIGN (getSpec (operandType (left)));
  rUnsigned = SPEC_USIGN (getSpec (operandType (right)));

  pushedB = pushB ();

  /* signed or unsigned */
  if (lUnsigned && rUnsigned)
    {
      /* unsigned is easy */
      LOAD_AB_FOR_DIV (left, right);
      emitcode ("div", "ab");

      _G.accInUse++;
      aopOp (result, ic, TRUE, FALSE);
      aopPut (result, "a", 0);
      _G.accInUse--;

      size = AOP_SIZE (result) - 1;

      while (size--)
        aopPut (result, zero, offset++);

      popB (pushedB);
      return;
    }

  /* signed is a little bit more difficult */

  /* now sign adjust for both left & right */

  /* let's see what's needed: */
  /* apply negative sign during runtime */
  runtimeSign = FALSE;
  /* negative sign from literals */
  compiletimeSign = FALSE;

  if (!lUnsigned)
    {
      if (AOP_TYPE (left) == AOP_LIT)
        {
          /* signed literal */
          signed char val = (char) ulFromVal (AOP (left)->aopu.aop_lit);
          if (val < 0)
            compiletimeSign = TRUE;
        }
      else
        /* signed but not literal */
        runtimeSign = TRUE;
    }

  if (!rUnsigned)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* signed literal */
          signed char val = (char) ulFromVal (AOP (right)->aopu.aop_lit);
          if (val < 0)
            compiletimeSign ^= TRUE;
        }
      else
        /* signed but not literal */
        runtimeSign = TRUE;
    }

  /* initialize F0, which stores the runtime sign */
  if (runtimeSign)
    {
      if (compiletimeSign)
        emitcode ("setb", "F0");        /* set sign flag */
      else
        emitcode ("clr", "F0"); /* reset sign flag */
    }

  /* save the signs of the operands */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (right)->aopu.aop_lit);

      if (!rUnsigned && val < 0)
        emitcode ("mov", "b,#0x%02x", -val);
      else
        emitcode ("mov", "b,#0x%02x", (unsigned char) val);
    }
  else                          /* ! literal */
    {
      if (rUnsigned)
        emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE, NULL));
      else
        {
          MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "F0");       /* complement sign flag */
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
          emitcode ("mov", "b,a");
        }
    }

  if (AOP_TYPE (left) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (left)->aopu.aop_lit);

      if (!lUnsigned && val < 0)
        emitcode ("mov", "a,#0x%02x", -val);
      else
        emitcode ("mov", "a,#0x%02x", (unsigned char) val);
    }
  else                          /* ! literal */
    {
      MOVA (aopGet (left, 0, FALSE, FALSE, NULL));

      if (!lUnsigned)
        {
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "F0");       /* complement sign flag */
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
        }
    }

  /* now the division */
  emitcode ("nop", "; workaround for DS80C390 div bug.");
  emitcode ("div", "ab");

  if (runtimeSign || compiletimeSign)
    {
      lbl = newiTempLabel (NULL);
      if (runtimeSign)
        emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");    /* lsb 2's complement */
      emitcode ("inc", "a");
      emitLabel (lbl);

      _G.accInUse++;
      aopOp (result, ic, TRUE, FALSE);
      size = AOP_SIZE (result) - 1;

      if (size > 0)
        {
          /* 123 look strange, but if (OP_SYMBOL (op)->accuse == 1)
             then the result will be in b, a */
          emitcode ("mov", "b,a");      /* 1 */
          /* msb is 0x00 or 0xff depending on the sign */
          if (runtimeSign)
            {
              emitcode ("mov", "c,F0");
              emitcode ("subb", "a,acc");
              emitcode ("xch", "a,b");  /* 2 */
              while (size--)
                aopPut (result, "b", offset++); /* write msb's */
            }
          else                  /* compiletimeSign */
            while (size--)
              aopPut (result, "#0xff", offset++);       /* write msb's */
        }
      aopPut (result, "a", 0);  /* 3: write lsb */
    }
  else
    {
      _G.accInUse++;
      aopOp (result, ic, TRUE, FALSE);
      size = AOP_SIZE (result) - 1;

      aopPut (result, "a", 0);
      while (size--)
        aopPut (result, zero, offset++);
    }
  _G.accInUse--;
  popB (pushedB);
}

/*-----------------------------------------------------------------*/
/* genDivTwoByte - use the DS390 MAC unit to do 16/16 divide       */
/*-----------------------------------------------------------------*/
static void
genDivTwoByte (operand * left, operand * right, operand * result, iCode * ic)
{
  sym_link *retype = getSpec (operandType (right));
  sym_link *letype = getSpec (operandType (left));
  int umult = SPEC_USIGN (retype) | SPEC_USIGN (letype);
  symbol *lbl;

  /* save EA bit in F1 */
  lbl = newiTempLabel (NULL);
  emitcode ("setb", "F1");
  emitcode ("jbc", "EA,!tlabel", labelKey2num (lbl->key));
  emitcode ("clr", "F1");
  emitLabel (lbl);

  /* load up MA with left */
  if (!umult)
    {
      emitcode ("clr", "F0");
      lbl = newiTempLabel (NULL);
      emitcode ("mov", "b,%s", aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "a,%s", aopGet (left, 1, FALSE, FALSE, NULL));
      emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
      emitcode ("xch", "a,b");
      emitcode ("cpl", "a");
      emitcode ("add", "a,#1");
      emitcode ("xch", "a,b");
      emitcode ("cpl", "a");    // msb
      emitcode ("addc", "a,#0");
      emitcode ("setb", "F0");
      emitLabel (lbl);
      emitcode ("mov", "ma,b");
      emitcode ("mov", "ma,a");
    }
  else
    {
      emitcode ("mov", "ma,%s", aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "ma,%s", aopGet (left, 1, FALSE, FALSE, NULL));
    }

  /* load up MB with right */
  if (!umult)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          int val = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          if (val < 0)
            {
              lbl = newiTempLabel (NULL);
              emitcode ("jbc", "F0,!tlabel", labelKey2num (lbl->key));
              emitcode ("setb", "F0");
              emitLabel (lbl);
              val = -val;
            }
          emitcode ("mov", "mb,#!constbyte", val & 0xff);
          emitcode ("mov", "mb,#!constbyte", (val >> 8) & 0xff);
        }
      else
        {
          lbl = newiTempLabel (NULL);
          emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE, NULL));
          emitcode ("mov", "a,%s", aopGet (right, 1, FALSE, FALSE, NULL));
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("xch", "a,b");
          emitcode ("cpl", "a");
          emitcode ("add", "a,#1");
          emitcode ("xch", "a,b");
          emitcode ("cpl", "a");        // msb
          emitcode ("addc", "a,#0");
          emitcode ("jbc", "F0,!tlabel", labelKey2num (lbl->key));
          emitcode ("setb", "F0");
          emitLabel (lbl);
          emitcode ("mov", "mb,b");
          emitcode ("mov", "mb,a");
        }
    }
  else
    {
      emitcode ("mov", "mb,%s", aopGet (right, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "mb,%s", aopGet (right, 1, FALSE, FALSE, NULL));
    }

  /* wait for multiplication to finish */
  lbl = newiTempLabel (NULL);
  emitLabel (lbl);
  emitcode ("mov", "a,mcnt1");
  emitcode ("anl", "a,#!constbyte", 0x80);
  emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));

  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (result, ic, TRUE, FALSE);

  /* if unsigned then simple */
  if (umult)
    {
      aopPut (result, "ma", 1);
      aopPut (result, "ma", 0);
    }
  else
    {
      emitcode ("push", "ma");
      MOVA ("ma");
      /* negate result if needed */
      lbl = newiTempLabel (NULL);
      emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");
      emitcode ("add", "a,#1");
      emitLabel (lbl);
      aopPut (result, "a", 0);
      emitcode ("pop", "acc");
      lbl = newiTempLabel (NULL);
      emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");
      emitcode ("addc", "a,#0");
      emitLabel (lbl);
      aopPut (result, "a", 1);
    }
  freeAsmop (result, NULL, ic, TRUE);
  /* restore EA bit in F1 */
  lbl = newiTempLabel (NULL);
  emitcode ("jnb", "F1,!tlabel", labelKey2num (lbl->key));
  emitcode ("setb", "EA");
  emitLabel (lbl);
  return;
}

/*-----------------------------------------------------------------*/
/* genDiv - generates code for division                            */
/*-----------------------------------------------------------------*/
static void
genDiv (iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  D (emitcode (";", "genDiv"));

  /* assign the asmops */
  AOP_OP_2 (ic);

  /* special cases first */
  /* both are bits */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      genDivbits (left, right, result, ic);
      goto release;
    }

  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
    {
      genDivOneByte (left, right, result, ic);
      goto release;
    }

  if (AOP_SIZE (left) == 2 && AOP_SIZE (right) == 2)
    {
      /* use the ds390 ARITHMETIC accel UNIT */
      genDivTwoByte (left, right, result, ic);
      return;
    }
  /* should have been converted to function call */
  assert (0);
release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}

/*-----------------------------------------------------------------*/
/* genModbits :- modulus of bits                                   */
/*-----------------------------------------------------------------*/
static void
genModbits (operand * left, operand * right, operand * result, iCode * ic)
{
  bool pushedB;

  D (emitcode (";", "genModbits"));

  pushedB = pushB ();

  /* the result must be bit */
  LOAD_AB_FOR_DIV (left, right);
  emitcode ("div", "ab");
  emitcode ("mov", "a,b");
  emitcode ("rrc", "a");
  aopOp (result, ic, TRUE, FALSE);

  popB (pushedB);

  aopPut (result, "c", 0);
}

/*-----------------------------------------------------------------*/
/* genModOneByte : 8 bit modulus                                   */
/*-----------------------------------------------------------------*/
static void
genModOneByte (operand * left, operand * right, operand * result, iCode * ic)
{
  bool lUnsigned, rUnsigned, pushedB;
  bool runtimeSign, compiletimeSign;
  symbol *lbl;
  int size, offset;

  D (emitcode (";", "genModOneByte"));

  offset = 1;
  lUnsigned = SPEC_USIGN (getSpec (operandType (left)));
  rUnsigned = SPEC_USIGN (getSpec (operandType (right)));

  pushedB = pushB ();

  /* signed or unsigned */
  if (lUnsigned && rUnsigned)
    {
      /* unsigned is easy */
      LOAD_AB_FOR_DIV (left, right);
      emitcode ("div", "ab");
      aopOp (result, ic, TRUE, FALSE);
      aopPut (result, "b", 0);

      for (size = AOP_SIZE (result) - 1; size--;)
        aopPut (result, zero, offset++);

      popB (pushedB);
      return;
    }

  /* signed is a little bit more difficult */

  /* now sign adjust for both left & right */

  /* modulus: sign of the right operand has no influence on the result! */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      signed char val = (signed char) operandLitValue (right);

      if (!rUnsigned && val < 0)
        emitcode ("mov", "b,#0x%02x", -val);
      else
        emitcode ("mov", "b,#0x%02x", (unsigned char) val);
    }
  else                          /* not literal */
    {
      if (rUnsigned)
        emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE, NULL));
      else
        {
          MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
          emitcode ("mov", "b,a");
        }
    }

  /* let's see what's needed: */
  /* apply negative sign during runtime */
  runtimeSign = FALSE;
  /* negative sign from literals */
  compiletimeSign = FALSE;

  /* sign adjust left side */
  if (AOP_TYPE (left) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (left)->aopu.aop_lit);

      if (!lUnsigned && val < 0)
        {
          compiletimeSign = TRUE;       /* set sign flag */
          emitcode ("mov", "a,#0x%02x", -val);
        }
      else
        emitcode ("mov", "a,#0x%02x", (unsigned char) val);
    }
  else                          /* ! literal */
    {
      MOVA (aopGet (left, 0, FALSE, FALSE, NULL));

      if (!lUnsigned)
        {
          runtimeSign = TRUE;
          emitcode ("clr", "F0");       /* clear sign flag */

          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("setb", "F0");      /* set sign flag */
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
        }
    }

  /* now the modulus */
  emitcode ("nop", "; workaround for DS80C390 div bug.");
  emitcode ("div", "ab");

  if (runtimeSign || compiletimeSign)
    {
      emitcode ("mov", "a,b");
      lbl = newiTempLabel (NULL);
      if (runtimeSign)
        emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");    /* lsb 2's complement */
      emitcode ("inc", "a");
      emitLabel (lbl);

      _G.accInUse++;
      aopOp (result, ic, TRUE, FALSE);
      size = AOP_SIZE (result) - 1;

      if (size > 0)
        {
          /* 123 look strange, but if (OP_SYMBOL (op)->accuse == 1)
             then the result will be in b, a */
          emitcode ("mov", "b,a");      /* 1 */
          /* msb is 0x00 or 0xff depending on the sign */
          if (runtimeSign)
            {
              emitcode ("mov", "c,F0");
              emitcode ("subb", "a,acc");
              emitcode ("xch", "a,b");  /* 2 */
              while (size--)
                aopPut (result, "b", offset++); /* write msb's */
            }
          else                  /* compiletimeSign */
            while (size--)
              aopPut (result, "#0xff", offset++);       /* write msb's */
        }
      aopPut (result, "a", 0);  /* 3: write lsb */
    }
  else
    {
      _G.accInUse++;
      aopOp (result, ic, TRUE, FALSE);
      size = AOP_SIZE (result) - 1;

      aopPut (result, "b", 0);
      while (size--)
        aopPut (result, zero, offset++);
    }
  _G.accInUse--;
  popB (pushedB);
}

/*-----------------------------------------------------------------*/
/* genModTwoByte - use the DS390 MAC unit to do 16%16 modulus      */
/*-----------------------------------------------------------------*/
static void
genModTwoByte (operand * left, operand * right, operand * result, iCode * ic)
{
  sym_link *retype = getSpec (operandType (right));
  sym_link *letype = getSpec (operandType (left));
  int umult = SPEC_USIGN (retype) | SPEC_USIGN (letype);
  symbol *lbl;

  /* load up MA with left */
  /* save EA bit in F1 */
  lbl = newiTempLabel (NULL);
  emitcode ("setb", "F1");
  emitcode ("jbc", "EA,!tlabel", labelKey2num (lbl->key));
  emitcode ("clr", "F1");
  emitLabel (lbl);

  if (!umult)
    {
      lbl = newiTempLabel (NULL);
      emitcode ("mov", "b,%s", aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "a,%s", aopGet (left, 1, FALSE, FALSE, NULL));
      emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
      emitcode ("xch", "a,b");
      emitcode ("cpl", "a");
      emitcode ("add", "a,#1");
      emitcode ("xch", "a,b");
      emitcode ("cpl", "a");    // msb
      emitcode ("addc", "a,#0");
      emitLabel (lbl);
      emitcode ("mov", "ma,b");
      emitcode ("mov", "ma,a");
    }
  else
    {
      emitcode ("mov", "ma,%s", aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "ma,%s", aopGet (left, 1, FALSE, FALSE, NULL));
    }

  /* load up MB with right */
  if (!umult)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          int val = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          if (val < 0)
            {
              val = -val;
            }
          emitcode ("mov", "mb,#!constbyte", val & 0xff);
          emitcode ("mov", "mb,#!constbyte", (val >> 8) & 0xff);
        }
      else
        {
          lbl = newiTempLabel (NULL);
          emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE, NULL));
          emitcode ("mov", "a,%s", aopGet (right, 1, FALSE, FALSE, NULL));
          emitcode ("jnb", "acc[7],!tlabel", labelKey2num (lbl->key));
          emitcode ("xch", "a,b");
          emitcode ("cpl", "a");
          emitcode ("add", "a,#1");
          emitcode ("xch", "a,b");
          emitcode ("cpl", "a");        // msb
          emitcode ("addc", "a,#0");
          emitLabel (lbl);
          emitcode ("mov", "mb,b");
          emitcode ("mov", "mb,a");
        }
    }
  else
    {
      emitcode ("mov", "mb,%s", aopGet (right, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "mb,%s", aopGet (right, 1, FALSE, FALSE, NULL));
    }

  /* wait for multiplication to finish */
  lbl = newiTempLabel (NULL);
  emitLabel (lbl);
  emitcode ("mov", "a,mcnt1");
  emitcode ("anl", "a,#!constbyte", 0x80);
  emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));

  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (result, ic, TRUE, FALSE);

  aopPut (result, "mb", 1);
  aopPut (result, "mb", 0);
  freeAsmop (result, NULL, ic, TRUE);

  /* restore EA bit in F1 */
  lbl = newiTempLabel (NULL);
  emitcode ("jnb", "F1,!tlabel", labelKey2num (lbl->key));
  emitcode ("setb", "EA");
  emitLabel (lbl);
}

/*-----------------------------------------------------------------*/
/* genMod - generates code for division                            */
/*-----------------------------------------------------------------*/
static void
genMod (iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  D (emitcode (";", "genMod"));

  /* assign the asmops */
  AOP_OP_2 (ic);

  /* special cases first */
  /* both are bits */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      genModbits (left, right, result, ic);
      goto release;
    }

  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
    {
      genModOneByte (left, right, result, ic);
      goto release;
    }

  if (AOP_SIZE (left) == 2 && AOP_SIZE (right) == 2)
    {
      /* use the ds390 ARITHMETIC accel UNIT */
      genModTwoByte (left, right, result, ic);
      return;
    }

  /* should have been converted to function call */
  assert (0);

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}

/*-----------------------------------------------------------------*/
/* genIfxJump :- will create a jump depending on the ifx           */
/*-----------------------------------------------------------------*/
static void
genIfxJump (iCode * ic, const char *jval, iCode * popIc)
{
  symbol *jlbl;
  symbol *tlbl = newiTempLabel (NULL);
  char *inst;

  /* if there is something to be popped then do it first */
  popForBranch (popIc, TRUE);

  D (emitcode (";", "genIfxJump"));

  /* if true label then we jump if condition
     supplied is true */
  if (IC_TRUE (ic))
    {
      jlbl = IC_TRUE (ic);
      inst = ((EQ (jval, "a") ? "jz" : (EQ (jval, "c") ? "jnc" : "jnb")));
    }
  else
    {
      /* false label is present */
      jlbl = IC_FALSE (ic);
      inst = ((EQ (jval, "a") ? "jnz" : (EQ (jval, "c") ? "jc" : "jb")));
    }
  if (EQ (inst, "jb") || EQ (inst, "jnb"))
    emitcode (inst, "%s,!tlabel", jval, labelKey2num ((tlbl->key)));
  else
    emitcode (inst, "!tlabel", labelKey2num (tlbl->key));
  emitcode ("ljmp", "!tlabel", labelKey2num (jlbl->key));
  emitLabel (tlbl);

  /* mark the icode as generated */
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* isHexChar :- check if a char is a digit or 'a-f' or 'A-F'       */
/*-----------------------------------------------------------------*/
static int isHexChar (const char c)
{
  const char hc[] = "0123456789ABCDEFabcdef";
  size_t i;
  for (i = 0; i < strlen (hc); i++)
    if (c == hc[i])
      return 1;
  return 0;
}

/*-----------------------------------------------------------------*/
/* genCmp :- greater or less than comparison                       */
/*-----------------------------------------------------------------*/
static void
genCmp (operand * left, operand * right, iCode * ic, iCode * ifx, int sign)
{
  int size, offset = 0;
  unsigned long long lit = 0;
  operand *result;

  D (emitcode (";", "genCmp"));

  result = IC_RESULT (ic);

  /* if left & right are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
      emitcode ("anl", "c,%s", AOP (left)->aopu.aop_dir);
    }
  /* generic pointers require special handling since all NULL pointers must compare equal */
  else if (opIsGptr (left) || opIsGptr (right))
    {
      /* push right */
      while (offset < GPTRSIZE)
        {
          emitpush (aopGet (left, offset++, FALSE, TRUE, NULL));
        }
      loadDptrFromOperand (right, TRUE);
      emitcode ("lcall", "___gptr_cmp");
      for (offset = 0; offset < GPTRSIZE; offset++)
        emitpop (NULL);
    }
  else
    {
      /* subtract right from left if at the
         end the carry flag is set then we know that
         left is greater than right */
      size = max (AOP_SIZE (left), AOP_SIZE (right));

      /* if unsigned char cmp with lit, do cjne left,#right,zz */
      if (size == 1 && !sign && AOP_TYPE (right) == AOP_LIT && AOP_TYPE (left) != AOP_DIR && AOP_TYPE (left) != AOP_STR)
        {
          char *l = Safe_strdup (aopGet (left, offset, FALSE, FALSE, NULL));
          symbol *lbl = newiTempLabel (NULL);
          if (AOP(left)->type != AOP_IMMD || ((AOP(right)->type != AOP_IMMD) && AOP(right)->type != AOP_LIT))
            emitcode ("cjne", "%s,%s,!tlabel", l, aopGet (right, offset, FALSE, FALSE, NULL), labelKey2num (lbl->key));
          else
            {
              const char *l0 = l;
              const char *r0 = aopGet (right, offset, FALSE, FALSE, NULL);
              long l1, r1;
              while (!isHexChar (*l0))
                l0++;
              while (!isHexChar (*r0))
                r0++;
              assert (sscanf(l0, "0x%lx", &l1) == 1);
              assert (sscanf(r0, "0x%lx", &r1) == 1);
              if (l1 != r1)
                emitcode ("sjmp", "!tlabel", labelKey2num (lbl->key));
            }
          Safe_free (l);
          emitLabel (lbl);
        }
      else
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              lit = ullFromVal (AOP (right)->aopu.aop_lit);
              /* optimize if(x < 0) or if(x >= 0) */
              if (lit == 0L)
                {
                  if (!sign)
                    {
                      CLRC;
                    }
                  else
                    {
                      MOVA (aopGet (left, AOP_SIZE (left) - 1, FALSE, FALSE, NULL));

                      freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
                      freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));

                      aopOp (result, ic, FALSE, FALSE);

                      if (!(AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result)) && ifx)
                        {
                          freeAsmop (result, NULL, ic, TRUE);
                          genIfxJump (ifx, "acc[7]", ic->next);
                          return;
                        }
                      else
                        {
                          emitcode ("rlc", "a");
                        }
                      goto release_freedLR;
                    }
                  goto release;
                }
            }
          CLRC;
          while (size--)
            {
              // emitcode (";", "genCmp #1: %d/%d/%d", size, sign, offset);
              MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
              // emitcode (";", "genCmp #2");
              if (sign && (size == 0))
                {
                  // emitcode (";", "genCmp #3");
                  emitcode ("xrl", "a,#!constbyte", 0x80);
                  if (AOP_TYPE (right) == AOP_LIT)
                    {
                      unsigned long long lit = ullFromVal (AOP (right)->aopu.aop_lit);
                      // emitcode (";", "genCmp #3.1");
                      emitcode ("subb", "a,#!constbyte", 0x80 ^ (unsigned int) ((lit >> (offset * 8)) & 0x0FFL));
                    }
                  else
                    {
                      // emitcode (";", "genCmp #3.2");
                      saveAccWarn = 0;
                      MOVB (aopGet (right, offset++, FALSE, FALSE, "b"));
                      saveAccWarn = DEFAULT_ACC_WARNING;
                      emitcode ("xrl", "b,#!constbyte", 0x80);
                      emitcode ("subb", "a,b");
                    }
                }
              else
                {
                  const char *s;

                  // emitcode (";", "genCmp #4");
                  saveAccWarn = 0;
                  s = aopGet (right, offset++, FALSE, FALSE, "b");
                  saveAccWarn = DEFAULT_ACC_WARNING;

                  emitcode ("subb", "a,%s", s);
                }
            }
        }
    }

release:
  /* Don't need the left & right operands any more; do need the result. */
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));

  aopOp (result, ic, FALSE, FALSE);

release_freedLR:

  if (AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result))
    {
      outBitC (result);
    }
  else
    {
      /* if the result is used in the next
         ifx conditional branch then generate
         code a little differently */
      if (ifx)
        {
          genIfxJump (ifx, "c", ic->next);
        }
      else
        {
          outBitC (result);
        }
      /* leave the result in acc */
    }
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genCmpGt :- greater than comparison                             */
/*-----------------------------------------------------------------*/
static void
genCmpGt (iCode * ic, iCode * ifx)
{
  operand *left, *right;
  sym_link *letype, *retype;
  int sign = 0;

  D (emitcode (";", "genCmpGt"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);

  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    {
      letype = getSpec (operandType (left));
      retype = getSpec (operandType (right));
      sign = !((SPEC_USIGN (letype) && !(IS_CHAR (letype) && IS_LITERAL (letype))) ||
               (SPEC_USIGN (retype) && !(IS_CHAR (retype) && IS_LITERAL (retype))));
    }
  /* assign the left & right asmops */
  AOP_OP_2 (ic);

  genCmp (right, left, ic, ifx, sign);
}

/*-----------------------------------------------------------------*/
/* genCmpLt - less than comparisons                                */
/*-----------------------------------------------------------------*/
static void
genCmpLt (iCode * ic, iCode * ifx)
{
  operand *left, *right;
  sym_link *letype, *retype;
  int sign = 0;

  D (emitcode (";", "genCmpLt"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);

  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    {
      letype = getSpec (operandType (left));
      retype = getSpec (operandType (right));
      sign = !((SPEC_USIGN (letype) && !(IS_CHAR (letype) && IS_LITERAL (letype))) ||
               (SPEC_USIGN (retype) && !(IS_CHAR (retype) && IS_LITERAL (retype))));
    }
  /* assign the left & right asmops */
  AOP_OP_2 (ic);

  genCmp (left, right, ic, ifx, sign);
}

/*-----------------------------------------------------------------*/
/* gencjneshort - compare and jump if not equal                    */
/*-----------------------------------------------------------------*/
static void
gencjneshort (operand * left, operand * right, symbol * lbl)
{
  int size = max (AOP_SIZE (left), AOP_SIZE (right));
  int offset = 0;
  unsigned long long lit = 0;

  D (emitcode (";", "gencjneshort"));

  /* if the left side is a literal or
     if the right is in a pointer register and left is not */
  if ((AOP_TYPE (left) == AOP_LIT) ||
      (AOP_TYPE (left) == AOP_IMMD) || (AOP_TYPE (left) == AOP_DIR) || (IS_AOP_PREG (right) && !IS_AOP_PREG (left)))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  if (AOP_TYPE (right) == AOP_LIT)
    lit = ullFromVal (AOP (right)->aopu.aop_lit);

  /* generic pointers require special handling since all NULL pointers must compare equal */
  if (opIsGptr (left) || opIsGptr (right))
    {
      /* push left */
      while (offset < size)
        {
          emitpush (aopGet (left, offset++, FALSE, TRUE, NULL));
        }
      loadDptrFromOperand (right, TRUE);
      emitcode ("lcall", "___gptr_cmp");
      for (offset = 0; offset < GPTRSIZE; offset++)
        emitpop (NULL);
      emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));
    }

  /* if the right side is a literal then anything goes */
  else if (AOP_TYPE (right) == AOP_LIT &&
           AOP_TYPE (left) != AOP_DIR && AOP_TYPE (left) != AOP_IMMD &&
           AOP_TYPE (left) != AOP_STR && AOP_TYPE (left) != AOP_DPTRn)
    {
      while (size--)
        {
          char *l = Safe_strdup (aopGet (left, offset, FALSE, FALSE, NULL));
          emitcode ("cjne", "%s,%s,!tlabel", l, aopGet (right, offset, FALSE, FALSE, NULL), labelKey2num (lbl->key));
          Safe_free (l);
          offset++;
        }
    }

  /* if the right side is in a register or in direct space or
     if the left is a pointer register & right is not */
  else if (AOP_TYPE (right) == AOP_REG ||
           AOP_TYPE (right) == AOP_DIR ||
           AOP_TYPE (right) == AOP_LIT ||
           AOP_TYPE (right) == AOP_IMMD ||
           (AOP_TYPE (left) == AOP_DIR && AOP_TYPE (right) == AOP_LIT) || (IS_AOP_PREG (left) && !IS_AOP_PREG (right)))
    {
      while (size--)
        {
          MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
          if ((AOP_TYPE (left) == AOP_DIR && AOP_TYPE (right) == AOP_LIT) &&
              ((unsigned int) ((lit >> (offset * 8)) & 0x0FFL) == 0))
            emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));
          else
            emitcode ("cjne", "a,%s,!tlabel", aopGet (right, offset, FALSE, TRUE, DP2_RESULT_REG), labelKey2num (lbl->key));
          offset++;
        }
    }
  else
    {
      /* right is a pointer reg need both a & b */
      while (size--)
        {
          MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
          MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
          emitcode ("cjne", "a,b,!tlabel", labelKey2num (lbl->key));
          offset++;
        }
    }
}

/*-----------------------------------------------------------------*/
/* gencjne - compare and jump if not equal                         */
/*-----------------------------------------------------------------*/
static void
gencjne (operand * left, operand * right, symbol * lbl)
{
  symbol *tlbl = newiTempLabel (NULL);

  D (emitcode (";", "gencjne"));

  gencjneshort (left, right, lbl);

  MOVA (one);
  emitcode ("sjmp", "!tlabel", labelKey2num (tlbl->key));
  emitLabel (lbl);
  MOVA (zero);
  emitLabel (tlbl);
}

/*-----------------------------------------------------------------*/
/* genCmpEq - generates code for equal to                          */
/*-----------------------------------------------------------------*/
static void
genCmpEq (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  iCode *popIc = ic->next;

  D (emitcode (";", "genCmpEq"));

  AOP_OP_2 (ic);
  AOP_SET_LOCALS (ic);

  /* if literal, literal on the right or
     if the right is in a pointer register and left
     is not */
  if ((AOP_TYPE (left) == AOP_LIT) || (IS_AOP_PREG (right) && !IS_AOP_PREG (left)))
    {
      swapOperands (&left, &right);
    }

  if (ifx &&                    /* !AOP_SIZE(result) */
      OP_SYMBOL (result) && OP_SYMBOL (result)->regType == REG_CND)
    {
      symbol *tlbl;
      /* if they are both bit variables */
      if (AOP_TYPE (left) == AOP_CRY && ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (right) == AOP_LIT)))
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              unsigned long long lit = ullFromVal (AOP (right)->aopu.aop_lit);
              if (lit == 0L)
                {
                  emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                  emitcode ("cpl", "c");
                }
              else if (lit == 1L)
                {
                  emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                }
              else
                {
                  emitcode ("clr", "c");
                }
              /* AOP_TYPE(right) == AOP_CRY */
            }
          else
            {
              symbol *lbl = newiTempLabel (NULL);
              emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
              emitcode ("jb", "%s,!tlabel", AOP (right)->aopu.aop_dir, labelKey2num (lbl->key));
              emitcode ("cpl", "c");
              emitLabel (lbl);
            }
          /* if true label then we jump if condition
             supplied is true */
          tlbl = newiTempLabel (NULL);
          if (IC_TRUE (ifx))
            {
              emitcode ("jnc", "!tlabel", labelKey2num (tlbl->key));
              popForBranch (popIc, FALSE);
              emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ifx)->key));
            }
          else
            {
              emitcode ("jc", "!tlabel", labelKey2num (tlbl->key));
              popForBranch (popIc, FALSE);
              emitcode ("ljmp", "!tlabel", labelKey2num (IC_FALSE (ifx)->key));
            }
          emitLabel (tlbl);
        }
      else
        {
          tlbl = newiTempLabel (NULL);
          gencjneshort (left, right, tlbl);
          if (IC_TRUE (ifx))
            {
              popForBranch (popIc, FALSE);
              emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ifx)->key));
              emitLabel (tlbl);
            }
          else
            {
              symbol *lbl = newiTempLabel (NULL);
              emitcode ("sjmp", "!tlabel", labelKey2num (lbl->key));
              emitLabel (tlbl);
              popForBranch (popIc, FALSE);
              emitcode ("ljmp", "!tlabel", labelKey2num (IC_FALSE (ifx)->key));
              emitLabel (lbl);
            }
        }
      /* mark the icode as generated */
      ifx->generated = 1;

      freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
      freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
      return;
    }

  /* if they are both bit variables */
  if (AOP_TYPE (left) == AOP_CRY && ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (right) == AOP_LIT)))
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          unsigned long long lit = ullFromVal (AOP (right)->aopu.aop_lit);
          if (lit == 0L)
            {
              emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
              emitcode ("cpl", "c");
            }
          else if (lit == 1L)
            {
              emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
            }
          else
            {
              emitcode ("clr", "c");
            }
          /* AOP_TYPE(right) == AOP_CRY */
        }
      else
        {
          symbol *lbl = newiTempLabel (NULL);
          emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
          emitcode ("jb", "%s,!tlabel", AOP (right)->aopu.aop_dir, labelKey2num (lbl->key));
          emitcode ("cpl", "c");
          emitLabel (lbl);
        }

      freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
      freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));

      aopOp (result, ic, TRUE, FALSE);

      /* c = 1 if egal */
      if (AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result))
        {
          outBitC (result);
          goto release;
        }
      if (ifx)
        {
          genIfxJump (ifx, "c", popIc);
          goto release;
        }
      /* if the result is used in an arithmetic operation
         then put the result in place */
      outBitC (result);
    }
  else
    {
      gencjne (left, right, newiTempLabel (NULL));

      freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
      freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));

      aopOp (result, ic, TRUE, FALSE);

      if (AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result))
        {
          aopPut (result, "a", 0);
          goto release;
        }
      if (ifx)
        {
          genIfxJump (ifx, "a", popIc);
          goto release;
        }
      /* if the result is used in an arithmetic operation
         then put the result in place */
      if (AOP_TYPE (result) != AOP_CRY)
        outAcc (result);
      /* leave the result in acc */
    }

release:
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* hasInc - operand is incremented before any other use            */
/*-----------------------------------------------------------------*/
static iCode *
hasInc (operand * op, iCode * ic, int osize)
{
  sym_link *type = operandType (op);
  sym_link *retype = getSpec (type);
  iCode *lic = ic->next;
  int isize;

  /* this could from a cast, e.g.: "(char xdata *) 0x7654;" */
  if (!IS_SYMOP (op))
    return NULL;

  if (IS_BITVAR (retype) || !IS_PTR (type))
    return NULL;
  if (IS_AGGREGATE (type->next))
    return NULL;
  if (osize != (isize = getSize (type->next)))
    return NULL;

  while (lic)
    {
      /* if operand of the form op = op + <sizeof *op> */
      if (lic->op == '+' && isOperandEqual (IC_LEFT (lic), op) &&
          isOperandEqual (IC_RESULT (lic), op) &&
          isOperandLiteral (IC_RIGHT (lic)) && operandLitValue (IC_RIGHT (lic)) == isize)
        {
          return lic;
        }
      /* if the operand used or deffed */
      if (bitVectBitValue (OP_USES (op), lic->key) || lic->defKey == op->key)
        {
          return NULL;
        }
      /* if GOTO or IFX */
      if (lic->op == IFX || lic->op == GOTO || lic->op == LABEL)
        break;
      lic = lic->next;
    }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* genAndOp - for && operation                                     */
/*-----------------------------------------------------------------*/
static void
genAndOp (iCode * ic)
{
  operand *left, *right, *result;
  symbol *tlbl;

  D (emitcode (";", "genAndOp"));

  /* note here that && operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  AOP_OP_2 (ic);
  AOP_SET_LOCALS (ic);

  /* if both are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
      emitcode ("anl", "c,%s", AOP (right)->aopu.aop_dir);
      freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
      freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));

      aopOp (result, ic, FALSE, FALSE);
      outBitC (result);
    }
  else
    {
      tlbl = newiTempLabel (NULL);
      toBoolean (left);
      emitcode ("jz", "!tlabel", labelKey2num (tlbl->key));
      toBoolean (right);
      emitLabel (tlbl);
      freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
      freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));

      aopOp (result, ic, FALSE, FALSE);
      outBitAcc (result);
    }

  freeAsmop (result, NULL, ic, TRUE);
}


/*-----------------------------------------------------------------*/
/* genOrOp - for || operation                                      */
/*-----------------------------------------------------------------*/
static void
genOrOp (iCode * ic)
{
  operand *left, *right, *result;
  symbol *tlbl;

  D (emitcode (";", "genOrOp"));

  /* note here that || operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  AOP_OP_2 (ic);
  AOP_SET_LOCALS (ic);

  /* if both are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
      emitcode ("orl", "c,%s", AOP (right)->aopu.aop_dir);
      freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
      freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));

      aopOp (result, ic, FALSE, FALSE);

      outBitC (result);
    }
  else
    {
      tlbl = newiTempLabel (NULL);
      toBoolean (left);
      emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
      toBoolean (right);
      emitLabel (tlbl);
      freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
      freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));

      aopOp (result, ic, FALSE, FALSE);

      outBitAcc (result);
    }

  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* isLiteralBit - test if lit == 2^n                               */
/*-----------------------------------------------------------------*/
static int
isLiteralBit (unsigned long lit)
{
  unsigned long pw[32] = { 1L, 2L, 4L, 8L, 16L, 32L, 64L, 128L,
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
      return idx + 1;
  return 0;
}

/*-----------------------------------------------------------------*/
/* continueIfTrue -                                                */
/*-----------------------------------------------------------------*/
static void
continueIfTrue (iCode * ic)
{
  if (IC_TRUE (ic))
    {
      emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ic)->key));
    }
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* jmpIfTrue -                                                     */
/*-----------------------------------------------------------------*/
static void
jumpIfTrue (iCode * ic)
{
  if (!IC_TRUE (ic))
    {
      emitcode ("ljmp", "!tlabel", labelKey2num (IC_FALSE (ic)->key));
    }
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* jmpTrueOrFalse -                                                */
/*-----------------------------------------------------------------*/
static void
jmpTrueOrFalse (iCode * ic, symbol * tlbl)
{
  // ugly but optimized by peephole
  if (IC_TRUE (ic))
    {
      symbol *nlbl = newiTempLabel (NULL);
      emitcode ("sjmp", "!tlabel", labelKey2num (nlbl->key));
      emitLabel (tlbl);
      emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ic)->key));
      emitLabel (nlbl);
    }
  else
    {
      emitcode ("ljmp", "!tlabel", labelKey2num (IC_FALSE (ic)->key));
      emitLabel (tlbl);
    }
  ic->generated = 1;
}

// Generate code to perform a bit-wise logic operation
// on two operands in far space (assumed to already have been
// aopOp'd by the AOP_OP_3_NOFATAL macro), storing the result
// in far space. This requires pushing the result on the stack
// then popping it into the result.
static void
genFarFarLogicOp (iCode * ic, char *logicOp)
{
  int size, resultSize, compSize;
  int offset = 0;

  TR_AP ("#5");
  D (emitcode (";", "%s special case for 3 far operands.", logicOp););
  compSize = AOP_SIZE (IC_LEFT (ic)) < AOP_SIZE (IC_RIGHT (ic)) ? AOP_SIZE (IC_LEFT (ic)) : AOP_SIZE (IC_RIGHT (ic));

  _startLazyDPSEvaluation ();
  for (size = compSize; (size--); offset++)
    {
      MOVA (aopGet (IC_LEFT (ic), offset, FALSE, FALSE, NULL));
      emitcode ("mov", "%s, acc", DP2_RESULT_REG);
      MOVA (aopGet (IC_RIGHT (ic), offset, FALSE, FALSE, NULL));

      emitcode (logicOp, "a,%s", DP2_RESULT_REG);
      emitcode ("push", "acc");
    }
  _endLazyDPSEvaluation ();

  freeAsmop (IC_LEFT (ic), NULL, ic, RESULTONSTACK (ic) ? FALSE : TRUE);
  freeAsmop (IC_RIGHT (ic), NULL, ic, RESULTONSTACK (ic) ? FALSE : TRUE);
  aopOp (IC_RESULT (ic), ic, TRUE, FALSE);

  resultSize = AOP_SIZE (IC_RESULT (ic));

  ADJUST_PUSHED_RESULT (compSize, resultSize);

  _startLazyDPSEvaluation ();
  while (compSize--)
    {
      emitcode ("pop", "acc");
      aopPut (IC_RESULT (ic), "a", compSize);
    }
  _endLazyDPSEvaluation ();
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
}


/*-----------------------------------------------------------------*/
/* genAnd  - code for and                                          */
/*-----------------------------------------------------------------*/
static void
genAnd (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0L;
  int bytelit = 0;
  bool pushResult;

  D (emitcode (";", "genAnd"));

  AOP_OP_3_NOFATAL (ic, pushResult);
  AOP_SET_LOCALS (ic);

  if (pushResult)
    {
      genFarFarLogicOp (ic, "anl");
      return;
    }

#ifdef DEBUG_TYPE
  emitcode (";", "Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right));
  emitcode (";", "Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right));
#endif

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT)
#ifdef LOGIC_OPS_BROKEN
      || AOP_NEEDSACC (left)
#endif
     )
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange left and right */
  if (sameRegs (AOP (result), AOP (right)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if right is bit then exchange them */
  if (AOP_TYPE (right) == AOP_CRY && AOP_TYPE (left) != AOP_CRY)
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }
  if (AOP_TYPE (right) == AOP_LIT)
    {
      lit = ullFromVal (AOP (right)->aopu.aop_lit);
    }

  size = AOP_SIZE (result);

  // if(bit & yy)
  // result = bit & yy;
  if (AOP_TYPE (left) == AOP_CRY)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          // c = bit & literal;
          if (lit & 1)
            {
              if (size && sameRegs (AOP (result), AOP (left)))
                // no change
                goto release;
              emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
            }
          else
            {
              // bit(result) = 0;
              if (size && (AOP_TYPE (result) == AOP_CRY))
                {
                  emitcode ("clr", "%s", AOP (result)->aopu.aop_dir);
                  goto release;
                }
              if ((AOP_TYPE (result) == AOP_CRY) && ifx)
                {
                  jumpIfTrue (ifx);
                  goto release;
                }
              emitcode ("clr", "c");
            }
        }
      else
        {
          if (AOP_TYPE (right) == AOP_CRY)
            {
              // c = bit & bit;
              if (IS_OP_ACCUSE (left))
                {
                  emitcode ("anl", "c,%s", AOP (right)->aopu.aop_dir);
                }
              else if (IS_OP_ACCUSE (right))
                {
                  emitcode ("anl", "c,%s", AOP (left)->aopu.aop_dir);
                }
              else
                {
                  emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
                  emitcode ("anl", "c,%s", AOP (left)->aopu.aop_dir);
                }
            }
          else if (AOP_TYPE (right) == AOP_DIR && IS_BOOL (operandType (right)) && AOP_TYPE (left) == AOP_CRY)
            {
              MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
              emitcode ("anl", "c,acc.0");
            }
          else
            {
              // c = bit & val;
              MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
              // c = lsb
              emitcode ("rrc", "a");
              emitcode ("anl", "c,%s", AOP (left)->aopu.aop_dir);
            }
        }
      // bit = c
      // val = c
      if (size)
        outBitC (result);
      // if(bit & ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c", ic->next);
      goto release;
    }

  // if(val & 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val & 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      int posbit = isLiteralBit (lit);
      /* left &  2^n */
      if (posbit)
        {
          posbit--;
          MOVA (aopGet (left, posbit >> 3, FALSE, FALSE, NULL));
          // bit = left & 2^n
          if (size)
            {
              switch (posbit & 0x07)
                {
                case 0:
                  emitcode ("rrc", "a");
                  break;
                case 7:
                  emitcode ("rlc", "a");
                  break;
                default:
                  emitcode ("mov", "c,acc[%d]", posbit & 0x07);
                  break;
                }
            }
          // if(left &  2^n)
          else
            {
              if (ifx)
                {
                  struct dbuf_s dbuf;

                  dbuf_init (&dbuf, 128);
                  dbuf_printf (&dbuf, "acc[%d]", posbit & 0x07);
                  genIfxJump (ifx, dbuf_c_str (&dbuf), ic->next);
                  dbuf_destroy (&dbuf);
                }
              else
                {
                  emitcode ("anl", "a,#!constbyte", 1 << (posbit & 0x07));
                }
              goto release;
            }
        }
      else
        {
          symbol *tlbl = newiTempLabel (NULL);
          int sizel = AOP_SIZE (left);
          if (size)
            emitcode ("setb", "c");
          while (sizel--)
            {
              if ((bytelit = ((lit >> (offset * 8)) & 0x0FFL)) != 0x0L)
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  // byte ==  2^n ?
                  if ((posbit = isLiteralBit (bytelit)) != 0)
                    emitcode ("jb", "acc[%d],!tlabel", (posbit - 1) & 0x07, labelKey2num (tlbl->key));
                  else
                    {
                      if (bytelit != 0x0FFL)
                        emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, TRUE, DP2_RESULT_REG));
                      emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
                    }
                }
              offset++;
            }
          // bit = left & literal
          if (size)
            {
              emitcode ("clr", "c");
              emitLabel (tlbl);
            }
          // if(left & literal)
          else
            {
              if (ifx)
                jmpTrueOrFalse (ifx, tlbl);
              else
                emitLabel (tlbl);
              goto release;
            }
        }
      outBitC (result);
      goto release;
    }

  /* if left is same as result */
  if (sameRegs (AOP (result), AOP (left)))
    {
      for (; size--; offset++)
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              bytelit = (int) ((lit >> (offset * 8)) & 0x0FFL);
              if (bytelit == 0x0FF)
                {
                  /* dummy read of volatile operand */
                  if (isOperandVolatile (left, FALSE))
                    MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  else
                    continue;
                }
              else if (bytelit == 0)
                {
                  aopPut (result, zero, offset);
                }
              else if (IS_AOP_PREG (result))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, TRUE, DP2_RESULT_REG));
                  aopPut (result, "a", offset);
                }
              else
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE, NULL));
                  emitcode ("anl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE, NULL));
                  Safe_free (l);
                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  if (offset)
                    emitcode ("mov", "b,a");
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
                  MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  emitcode ("anl", "a,b");
                  aopPut (result, "a", offset);
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  aopPut (result, "a", offset);
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  if (IS_AOP_PREG (result))
                    {
                      emitcode ("anl", "a,%s", aopGet (left, offset, FALSE, TRUE, DP2_RESULT_REG));
                      aopPut (result, "a", offset);
                    }
                  else
                    {
                      emitcode ("anl", "%s,a", aopGet (left, offset, FALSE, TRUE, DP2_RESULT_REG));
                    }
                }
            }
        }
    }
  else
    {
      // left & result in different registers
      if (AOP_TYPE (result) == AOP_CRY)
        {
          // result = bit
          // if(size), result in bit
          // if(!size && ifx), conditional oper: if(left & right)
          symbol *tlbl = newiTempLabel (NULL);
          int sizer = min (AOP_SIZE (left), AOP_SIZE (right));
          if (size)
            emitcode ("setb", "c");
          while (sizer--)
            {
              if ((AOP_TYPE (right) == AOP_REG || IS_AOP_PREG (right) || AOP_TYPE (right) == AOP_DIR)
                  && AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE, NULL));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                      emitcode ("anl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                      emitcode ("anl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
                  MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  emitcode ("anl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  emitcode ("anl", "a,%s", aopGet (left, offset, FALSE, FALSE, DP2_RESULT_REG));
                }

              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              emitLabel (tlbl);
              outBitC (result);
            }
          else if (ifx)
            jmpTrueOrFalse (ifx, tlbl);
          else
            emitLabel (tlbl);
        }
      else
        {
          for (; (size--); offset++)
            {
              // normal case
              // result = left & right
              if (AOP_TYPE (right) == AOP_LIT)
                {
                  bytelit = (int) ((lit >> (offset * 8)) & 0x0FFL);
                  if (bytelit == 0x0FF)
                    {
                      aopPut (result, aopGet (left, offset, FALSE, FALSE, NULL), offset);
                      continue;
                    }
                  else if (bytelit == 0)
                    {
                      /* dummy read of volatile operand */
                      if (isOperandVolatile (left, FALSE))
                        MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                      aopPut (result, zero, offset);
                      continue;
                    }
                  else if (AOP_TYPE (left) == AOP_ACC)
                    {
                      char *l = Safe_strdup (aopGet (left, offset, FALSE, FALSE, NULL));
                      emitcode ("anl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE, NULL));
                      aopPut (result, l, offset);
                      Safe_free (l);
                      continue;
                    }
                }
              // faster than result <- left, anl result,right
              // and better if result is SFR
              if ((AOP_TYPE (right) == AOP_REG || IS_AOP_PREG (right) || AOP_TYPE (right) == AOP_DIR)
                  && AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE, NULL));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                      emitcode ("anl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                      emitcode ("anl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
                  MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  emitcode ("anl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  emitcode ("anl", "a,%s", aopGet (left, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              aopPut (result, "a", offset);
            }
        }
    }

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}

/*-----------------------------------------------------------------*/
/* genOr  - code for or                                            */
/*-----------------------------------------------------------------*/
static void
genOr (iCode *ic, iCode *ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0;
  int bytelit = 0;
  bool pushResult;

  D (emitcode (";", "genOr"));

  AOP_OP_3_NOFATAL (ic, pushResult);
  AOP_SET_LOCALS (ic);

  if (pushResult)
    {
      genFarFarLogicOp (ic, "orl");
      return;
    }


#ifdef DEBUG_TYPE
  emitcode (";", "Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right));
  emitcode (";", "Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right));
#endif

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT)
#ifdef LOGIC_OPS_BROKEN
      || AOP_NEEDSACC (left)    // I think this is a net loss now.
#endif
     )
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange left and right */
  if (sameRegs (AOP (result), AOP (right)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if right is bit then exchange them */
  if (AOP_TYPE (right) == AOP_CRY && AOP_TYPE (left) != AOP_CRY)
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }
  if (AOP_TYPE (right) == AOP_LIT)
    {
      lit = ullFromVal (AOP (right)->aopu.aop_lit);
    }

  size = AOP_SIZE (result);

  // if(bit | yy)
  // xx = bit | yy;
  if (AOP_TYPE (left) == AOP_CRY)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          // c = bit | literal;
          if (lit)
            {
              // lit != 0 => result = 1
              if (AOP_TYPE (result) == AOP_CRY)
                {
                  if (size)
                    emitcode ("setb", "%s", AOP (result)->aopu.aop_dir);
                  else if (ifx)
                    continueIfTrue (ifx);
                  goto release;
                }
              emitcode ("setb", "c");
            }
          else
            {
              // lit == 0 => result = left
              if (size && sameRegs (AOP (result), AOP (left)))
                goto release;
              emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
            }
        }
      else
        {
          if (AOP_TYPE (right) == AOP_CRY)
            {
              // c = bit | bit;
              if (IS_OP_ACCUSE (left))
                {
                  emitcode ("orl", "c,%s", AOP (right)->aopu.aop_dir);
                }
              else if (IS_OP_ACCUSE (right))
                {
                  emitcode ("orl", "c,%s", AOP (left)->aopu.aop_dir);
                }
              else
                {
                  emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
                  emitcode ("orl", "c,%s", AOP (left)->aopu.aop_dir);
                }
            }
          else
            {
              // c = bit | val;
              symbol *tlbl = newiTempLabel (NULL);
              if (!((AOP_TYPE (result) == AOP_CRY) && ifx))
                emitcode ("setb", "c");
              emitcode ("jb", "%s,!tlabel", AOP (left)->aopu.aop_dir, labelKey2num (tlbl->key));
              toBoolean (right);
              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
              if ((AOP_TYPE (result) == AOP_CRY) && ifx)
                {
                  jmpTrueOrFalse (ifx, tlbl);
                  goto release;
                }
              else
                {
                  CLRC;
                  emitLabel (tlbl);
                }
            }
        }
      // bit = c
      // val = c
      if (size)
        outBitC (result);
      // if(bit | ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c", ic->next);
      goto release;
    }

  // if(val | 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val | 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      if (lit)
        {
          // result = 1
          if (size)
            emitcode ("setb", "%s", AOP (result)->aopu.aop_dir);
          else if (ifx)
            continueIfTrue (ifx);
          goto release;
        }
      else
        {
          // lit = 0, result = boolean(left)
          if (size)
            SETC;
          toBoolean (left);
          if (size)
            {
              symbol *tlbl = newiTempLabel (NULL);
              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
              CLRC;
              emitLabel (tlbl);
            }
          else
            {
              /* FIXME, thats pretty fishy, check for ifx!=0, testcase .. */
              assert (ifx);
              genIfxJump (ifx, "a", ic->next);
              goto release;
            }
        }
      outBitC (result);
      goto release;
    }

  /* if left is same as result */
  if (sameRegs (AOP (result), AOP (left)))
    {
      for (; size--; offset++)
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              bytelit = (int) ((lit >> (offset * 8)) & 0x0FFL);
              if (bytelit == 0)
                {
                  /* dummy read of volatile operand */
                  if (isOperandVolatile (left, FALSE))
                    MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  else
                    continue;
                }
              else if (bytelit == 0x0FF)
                {
                  aopPut (result, "#0xff", offset);
                }
              else if (IS_AOP_PREG (left))
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  emitcode ("orl", "a,%s", aopGet (left, offset, FALSE, TRUE, DP2_RESULT_REG));
                  aopPut (result, "a", offset);
                }
              else
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE, NULL));
                  emitcode ("orl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  Safe_free (l);
                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  if (offset)
                    emitcode ("mov", "b,a");
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
                  MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  emitcode ("orl", "a,b");
                  aopPut (result, "a", offset);
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  aopPut (result, "a", offset);
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  if (IS_AOP_PREG (left))
                    {
                      emitcode ("orl", "a,%s", aopGet (left, offset, FALSE, TRUE, DP2_RESULT_REG));
                      aopPut (result, "a", offset);
                    }
                  else
                    {
                      emitcode ("orl", "%s,a", aopGet (left, offset, FALSE, TRUE, DP2_RESULT_REG));
                    }
                }
            }
        }
    }
  else
    {
      // left & result in different registers
      if (AOP_TYPE (result) == AOP_CRY)
        {
          // result = bit
          // if(size), result in bit
          // if(!size && ifx), conditional oper: if(left | right)
          symbol *tlbl = newiTempLabel (NULL);
          int sizer = max (AOP_SIZE (left), AOP_SIZE (right));
          if (size)
            emitcode ("setb", "c");
          while (sizer--)
            {
              if ((AOP_TYPE (right) == AOP_REG || IS_AOP_PREG (right) || AOP_TYPE (right) == AOP_DIR)
                  && AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                      emitcode ("orl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                      emitcode ("orl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
                  MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  emitcode ("orl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  emitcode ("orl", "a,%s", aopGet (left, offset, FALSE, FALSE, DP2_RESULT_REG));
                }

              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              emitLabel (tlbl);
              outBitC (result);
            }
          else if (ifx)
            jmpTrueOrFalse (ifx, tlbl);
          else
            emitLabel (tlbl);
        }
      else
        {
          _startLazyDPSEvaluation ();
          for (; (size--); offset++)
            {
              // normal case
              // result = left | right
              if (AOP_TYPE (right) == AOP_LIT)
                {
                  bytelit = (int) ((lit >> (offset * 8)) & 0x0FFL);
                  if (bytelit == 0)
                    {
                      aopPut (result, aopGet (left, offset, FALSE, FALSE, NULL), offset);
                      continue;
                    }
                  else if (bytelit == 0x0FF)
                    {
                      /* dummy read of volatile operand */
                      if (isOperandVolatile (left, FALSE))
                        MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                      aopPut (result, "#0xff", offset);
                      continue;
                    }
                }
              // faster than result <- left, orl result,right
              // and better if result is SFR
              if ((AOP_TYPE (right) == AOP_REG || IS_AOP_PREG (right) || AOP_TYPE (right) == AOP_DIR)
                  && AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                      emitcode ("orl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                      emitcode ("orl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
                  MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  emitcode ("orl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  emitcode ("orl", "a,%s", aopGet (left, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              aopPut (result, "a", offset);
            }
          _endLazyDPSEvaluation ();
        }
    }

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}

/*-----------------------------------------------------------------*/
/* genXor - code for xclusive or                                   */
/*-----------------------------------------------------------------*/
static void
genXor (iCode *ic, iCode *ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0;
  int bytelit = 0;
  bool pushResult;

  D (emitcode (";", "genXor"));

  AOP_OP_3_NOFATAL (ic, pushResult);
  AOP_SET_LOCALS (ic);

  if (pushResult)
    {
      genFarFarLogicOp (ic, "xrl");
      return;
    }

#ifdef DEBUG_TYPE
  emitcode (";", "Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right));
  emitcode (";", "Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right));
#endif

  /* if left is a literal & right is not ||
     if left needs acc & right does not */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT)
#ifdef LOGIC_OPS_BROKEN
      || (AOP_NEEDSACC (left) && !AOP_NEEDSACC (right))
#endif
     )
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange left and right */
  if (sameRegs (AOP (result), AOP (right)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if right is bit then exchange them */
  if (AOP_TYPE (right) == AOP_CRY && AOP_TYPE (left) != AOP_CRY)
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }
  if (AOP_TYPE (right) == AOP_LIT)
    {
      lit = ullFromVal (AOP (right)->aopu.aop_lit);
    }

  size = AOP_SIZE (result);

  // if(bit ^ yy)
  // xx = bit ^ yy;
  if (AOP_TYPE (left) == AOP_CRY)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          // c = bit ^ literal;
          if (lit >> 1)
            {
              // lit>>1  != 0 => result = 1
              if (AOP_TYPE (result) == AOP_CRY)
                {
                  if (size)
                    emitcode ("setb", "%s", AOP (result)->aopu.aop_dir);
                  else if (ifx)
                    continueIfTrue (ifx);
                  goto release;
                }
              emitcode ("setb", "c");
            }
          else
            {
              // lit == (0 or 1)
              if (lit == 0)
                {
                  // lit == 0, result = left
                  if (size && sameRegs (AOP (result), AOP (left)))
                    goto release;
                  emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                }
              else
                {
                  // lit == 1, result = not(left)
                  if (size && sameRegs (AOP (result), AOP (left)))
                    {
                      emitcode ("cpl", "%s", AOP (result)->aopu.aop_dir);
                      goto release;
                    }
                  else
                    {
                      emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                      emitcode ("cpl", "c");
                    }
                }
            }
        }
      else
        {
          // right != literal
          symbol *tlbl = newiTempLabel (NULL);
          if (AOP_TYPE (right) == AOP_CRY)
            {
              // c = bit ^ bit;
              if (IS_OP_ACCUSE (left))
                {
                  // left already is in the carry
                  operand *tmp = right;
                  right = left;
                  left = tmp;
                }
              else
                {
                  toCarry (right);
                }
            }
          else
            {
              // c = bit ^ val
              toCarry (right);
            }
          emitcode ("jnb", "%s,!tlabel", AOP (left)->aopu.aop_dir, labelKey2num (tlbl->key));
          emitcode ("cpl", "c");
          emitLabel (tlbl);
        }
      // bit = c
      // val = c
      if (size)
        outBitC (result);
      // if(bit ^ ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c", ic->next);
      goto release;
    }

  /* if left is same as result */
  if (sameRegs (AOP (result), AOP (left)))
    {
      for (; size--; offset++)
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              bytelit = (int) ((lit >> (offset * 8)) & 0x0FFL);
              if (bytelit == 0)
                {
                  /* dummy read of volatile operand */
                  if (isOperandVolatile (left, FALSE))
                    MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  else
                    continue;
                }
              else if (IS_AOP_PREG (left))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, TRUE, DP2_RESULT_REG));
                  aopPut (result, "a", offset);
                }
              else
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE, NULL));
                  emitcode ("xrl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  Safe_free (l);
                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  if (offset)
                    emitcode ("mov", "b,a");
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
                  MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  emitcode ("xrl", "a,b");
                  aopPut (result, "a", offset);
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  aopPut (result, "a", offset);
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  if (IS_AOP_PREG (left))
                    {
                      emitcode ("xrl", "a,%s", aopGet (left, offset, FALSE, TRUE, DP2_RESULT_REG));
                      aopPut (result, "a", offset);
                    }
                  else
                    {
                      emitcode ("xrl", "%s,a", aopGet (left, offset, FALSE, TRUE, DP2_RESULT_REG));
                    }
                }
            }
        }
    }
  else
    {
      // left & result in different registers
      if (AOP_TYPE (result) == AOP_CRY)
        {
          // result = bit
          // if(size), result in bit
          // if(!size && ifx), conditional oper: if(left ^ right)
          symbol *tlbl = newiTempLabel (NULL);
          int sizer = max (AOP_SIZE (left), AOP_SIZE (right));

          if (size)
            emitcode ("setb", "c");
          while (sizer--)
            {
              if ((AOP_TYPE (right) == AOP_LIT) && (((lit >> (offset * 8)) & 0x0FFL) == 0x00L))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                }
              else if ((AOP_TYPE (right) == AOP_REG || IS_AOP_PREG (right) || AOP_TYPE (right) == AOP_DIR)
                       && AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                      emitcode ("xrl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                      emitcode ("xrl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE, NULL));
                  MOVA (aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                  emitcode ("xrl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE, NULL));
                  emitcode ("xrl", "a,%s", aopGet (left, offset, FALSE, TRUE, DP2_RESULT_REG));
                }

              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              emitLabel (tlbl);
              outBitC (result);
            }
          else if (ifx)
            jmpTrueOrFalse (ifx, tlbl);
          else                  // need a target here
            emitLabel (tlbl);
        }
      else
        {
          for (; (size--); offset++)
            {
              // normal case
              // result = left ^ right
              if (AOP_TYPE (right) == AOP_LIT)
                {
                  bytelit = (int) ((lit >> (offset * 8)) & 0x0FFL);
                  if (bytelit == 0)
                    {
                      aopPut (result, aopGet (left, offset, FALSE, FALSE, NULL), offset);
                      continue;
                    }
                  D (emitcode (";", "better literal XOR."));
                  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                  emitcode ("xrl", "a, %s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                }
              else
                {
                  // faster than result <- left, anl result,right
                  // and better if result is SFR
                  if (AOP_TYPE (left) == AOP_ACC)
                    {
                      emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE, DP2_RESULT_REG));
                    }
                  else
                    {
                      const char *rOp = aopGet (right, offset, FALSE, FALSE, NULL);
                      if (!strcmp (rOp, "a") || !strcmp (rOp, "acc"))
                        {
                          emitcode ("mov", "b,a");
                          rOp = "b";
                        }

                      rOp = Safe_strdup (rOp);
                      MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
                      emitcode ("xrl", "a,%s", rOp);
                      Safe_free ((void *) rOp);
                    }
                }
              aopPut (result, "a", offset);
            }
        }
    }

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}

/*-----------------------------------------------------------------*/
/* genRRC - rotate right with carry                                */
/*-----------------------------------------------------------------*/
static void
genRRC (iCode * ic)
{
  operand *left, *result;
  int size, offset;

  D (emitcode (";", "genRRC"));

  /* rotate right with carry */
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

  /* move it to the result */
  size = AOP_SIZE (result);
  offset = size - 1;
  _startLazyDPSEvaluation ();
  /* no need to clear carry, bit7 will be written later */
  while (size--)
    {
      MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
      emitcode ("rrc", "a");
      if (AOP_SIZE (result) > 1)
        aopPut (result, "a", offset--);
    }
  _endLazyDPSEvaluation ();

  /* now we need to put the carry into the
     highest order byte of the result */
  if (AOP_SIZE (result) > 1)
    {
      MOVA (aopGet (result, AOP_SIZE (result) - 1, FALSE, FALSE, NULL));
    }
  emitcode ("mov", "acc[7],c");
  aopPut (result, "a", AOP_SIZE (result) - 1);
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genRLC - generate code for rotate left with carry               */
/*-----------------------------------------------------------------*/
static void
genRLC (iCode * ic)
{
  operand *left, *result;
  int size, offset;

  D (emitcode (";", "genRLC"));

  /* rotate right with carry */
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

  /* move it to the result */
  size = AOP_SIZE (result);
  offset = 0;
  if (size--)
    {
      MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
      emitcode ("rlc", "a");    /* bit0 will be written later */
      if (AOP_SIZE (result) > 1)
        {
          aopPut (result, "a", offset++);
        }

      _startLazyDPSEvaluation ();
      while (size--)
        {
          MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
          emitcode ("rlc", "a");
          if (AOP_SIZE (result) > 1)
            aopPut (result, "a", offset++);
        }
      _endLazyDPSEvaluation ();
    }
  /* now we need to put the carry into the
     highest order byte of the result */
  if (AOP_SIZE (result) > 1)
    {
      MOVA (aopGet (result, 0, FALSE, FALSE, NULL));
    }
  emitcode ("mov", "acc[0],c");
  aopPut (result, "a", 0);
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGetHbit - generates code get highest order bit               */
/*-----------------------------------------------------------------*/
static void
genGetHbit (iCode * ic)
{
  operand *left, *result;

  D (emitcode (";", "genGetHbit"));

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

  /* get the highest order byte into a */
  MOVA (aopGet (left, AOP_SIZE (left) - 1, FALSE, FALSE, NULL));
  if (AOP_TYPE (result) == AOP_CRY)
    {
      emitcode ("rlc", "a");
      outBitC (result);
    }
  else
    {
      emitcode ("rl", "a");
      emitcode ("anl", "a,#0x01");
      outAcc (result);
    }


  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genSwap - generates code to swap nibbles or bytes               */
/*-----------------------------------------------------------------*/
static void
genSwap (iCode * ic)
{
  operand *left, *result;

  D (emitcode (";", "genSwap"));

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

  _startLazyDPSEvaluation ();
  switch (AOP_SIZE (left))
    {
    case 1:                    /* swap nibbles in byte */
      MOVA (aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("swap", "a");
      aopPut (result, "a", 0);
      break;
    case 2:                    /* swap bytes in word */
      if (AOP_TYPE (left) == AOP_REG && sameRegs (AOP (left), AOP (result)))
        {
          MOVA (aopGet (left, 0, FALSE, FALSE, NULL));
          aopPut (result, aopGet (left, 1, FALSE, FALSE, NULL), 0);
          aopPut (result, "a", 1);
        }
      else if (operandsEqu (left, result))
        {
          char *reg = "a";
          bool pushedB = FALSE, leftInB = FALSE;

          MOVA (aopGet (left, 0, FALSE, FALSE, NULL));
          if (aopGetUsesAcc (left, 1) || aopGetUsesAcc (result, 0))
            {
              pushedB = pushB ();
              emitcode ("mov", "b,a");
              reg = "b";
              leftInB = TRUE;
            }
          aopPut (result, aopGet (left, 1, FALSE, FALSE, NULL), 0);
          aopPut (result, reg, 1);

          if (leftInB)
            popB (pushedB);
        }
      else
        {
          aopPut (result, aopGet (left, 1, FALSE, FALSE, NULL), 0);
          aopPut (result, aopGet (left, 0, FALSE, FALSE, NULL), 1);
        }
      break;
    default:
      wassertl (FALSE, "unsupported SWAP operand size");
    }
  _endLazyDPSEvaluation ();

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
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
    case 0:
      break;
    case 1:
      emitcode ("rl", "a");
      break;
    case 2:
      emitcode ("rl", "a");
      emitcode ("rl", "a");
      break;
    case 3:
      emitcode ("swap", "a");
      emitcode ("rr", "a");
      break;
    case 4:
      emitcode ("swap", "a");
      break;
    case 5:
      emitcode ("swap", "a");
      emitcode ("rl", "a");
      break;
    case 6:
      emitcode ("rr", "a");
      emitcode ("rr", "a");
      break;
    case 7:
      emitcode ("rr", "a");
      break;
    }
}

/*-----------------------------------------------------------------*/
/* AccLsh - left shift accumulator by known count                  */
/*-----------------------------------------------------------------*/
static void
AccLsh (int shCount)
{
  if (shCount != 0)
    {
      if (shCount == 1)
        emitcode ("add", "a,acc");
      else if (shCount == 2)
        {
          emitcode ("add", "a,acc");
          emitcode ("add", "a,acc");
        }
      else
        {
          /* rotate left accumulator */
          AccRol (shCount);
          /* and kill the lower order bits */
          emitcode ("anl", "a,#!constbyte", SLMask[shCount]);
        }
    }
}

/*-----------------------------------------------------------------*/
/* AccRsh - right shift accumulator by known count                 */
/*-----------------------------------------------------------------*/
static void
AccRsh (int shCount)
{
  if (shCount != 0)
    {
      if (shCount == 1)
        {
          CLRC;
          emitcode ("rrc", "a");
        }
      else
        {
          /* rotate right accumulator */
          AccRol (8 - shCount);
          /* and kill the higher order bits */
          emitcode ("anl", "a,#!constbyte", SRMask[shCount]);
        }
    }
}

/*-----------------------------------------------------------------*/
/* AccSRsh - signed right shift accumulator by known count                 */
/*-----------------------------------------------------------------*/
static void
AccSRsh (int shCount)
{
  symbol *tlbl;
  if (shCount != 0)
    {
      if (shCount == 1)
        {
          emitcode ("mov", "c,acc[7]");
          emitcode ("rrc", "a");
        }
      else if (shCount == 2)
        {
          emitcode ("mov", "c,acc[7]");
          emitcode ("rrc", "a");
          emitcode ("mov", "c,acc[7]");
          emitcode ("rrc", "a");
        }
      else
        {
          tlbl = newiTempLabel (NULL);
          /* rotate right accumulator */
          AccRol (8 - shCount);
          /* and kill the higher order bits */
          emitcode ("anl", "a,#!constbyte", SRMask[shCount]);
          emitcode ("jnb", "acc[%d],!tlabel", 7 - shCount, labelKey2num (tlbl->key));
          emitcode ("orl", "a,#!constbyte", (unsigned char) ~SRMask[shCount]);
          emitLabel (tlbl);
        }
    }
}

/*-----------------------------------------------------------------*/
/* shiftR1Left2Result - shift right one byte from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftR1Left2Result (operand * left, int offl, operand * result, int offr, int shCount, int sign)
{
  MOVA (aopGet (left, offl, FALSE, FALSE, NULL));
  /* shift right accumulator */
  if (sign)
    AccSRsh (shCount);
  else
    AccRsh (shCount);
  aopPut (result, "a", offr);
}

/*-----------------------------------------------------------------*/
/* shiftL1Left2Result - shift left one byte from left to result    */
/*-----------------------------------------------------------------*/
static void
shiftL1Left2Result (operand * left, int offl, operand * result, int offr, int shCount)
{
  MOVA (aopGet (left, offl, FALSE, FALSE, NULL));
  /* shift left accumulator */
  AccLsh (shCount);
  aopPut (result, "a", offr);
}

/*-----------------------------------------------------------------*/
/* movLeft2Result - move byte from left to result                  */
/*-----------------------------------------------------------------*/
static void
movLeft2Result (operand * left, int offl, operand * result, int offr, int sign)
{
  const char *l;
  if (!sameRegs (AOP (left), AOP (result)) || (offl != offr))
    {
      l = aopGet (left, offl, FALSE, FALSE, NULL);

      if (*l == '@' && (IS_AOP_PREG (result)))
        {
          emitcode ("mov", "a,%s", l);
          aopPut (result, "a", offr);
        }
      else
        {
          if (!sign)
            {
              aopPut (result, l, offr);
            }
          else
            {
              /* MSB sign in acc.7 ! */
              if (getDataSize (left) == offl + 1)
                {
                  MOVA (l);
                  aopPut (result, "a", offr);
                }
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* AccAXRrl1 - right rotate c->a:x->c by 1                         */
/*-----------------------------------------------------------------*/
static void
AccAXRrl1 (const char *x)
{
  emitcode ("rrc", "a");
  emitcode ("xch", "a,%s", x);
  emitcode ("rrc", "a");
  emitcode ("xch", "a,%s", x);
}

/*-----------------------------------------------------------------*/
/* AccAXLrl1 - left rotate c<-a:x<-c by 1                          */
/*-----------------------------------------------------------------*/
static void
AccAXLrl1 (const char *x)
{
  emitcode ("xch", "a,%s", x);
  emitcode ("rlc", "a");
  emitcode ("xch", "a,%s", x);
  emitcode ("rlc", "a");
}

/*-----------------------------------------------------------------*/
/* AccAXLsh1 - left shift a:x<-0 by 1                              */
/*-----------------------------------------------------------------*/
static void
AccAXLsh1 (const char *x)
{
  emitcode ("xch", "a,%s", x);
  emitcode ("add", "a,acc");
  emitcode ("xch", "a,%s", x);
  emitcode ("rlc", "a");
}

/*-----------------------------------------------------------------*/
/* AccAXLsh - left shift a:x by known count (0..7)                 */
/*-----------------------------------------------------------------*/
static void
AccAXLsh (const char *x, int shCount)
{
  unsigned char mask;

  switch (shCount)
    {
    case 0:
      break;
    case 1:
      AccAXLsh1 (x);
      break;
    case 2:
      AccAXLsh1 (x);
      AccAXLsh1 (x);
      break;
    case 3:
    case 4:
    case 5:                    // AAAAABBB:CCCCCDDD
      mask = SLMask[shCount];
      AccRol (shCount);         // BBBAAAAA:CCCCCDDD
      emitcode ("anl", "a,#!constbyte", mask);  // BBB00000:CCCCCDDD
      emitcode ("xch", "a,%s", x);      // CCCCCDDD:BBB00000
      AccRol (shCount);         // DDDCCCCC:BBB00000
      emitcode ("xch", "a,%s", x);      // BBB00000:DDDCCCCC
      emitcode ("xrl", "a,%s", x);      // (BBB^DDD)CCCCC:DDDCCCCC
      emitcode ("xch", "a,%s", x);      // DDDCCCCC:(BBB^DDD)CCCCC
      emitcode ("anl", "a,#!constbyte", mask);  // DDD00000:(BBB^DDD)CCCCC
      emitcode ("xch", "a,%s", x);      // (BBB^DDD)CCCCC:DDD00000
      emitcode ("xrl", "a,%s", x);      // BBBCCCCC:DDD00000
      break;
    case 6:                    // AAAAAABB:CCCCCCDD
      mask = SRMask[shCount];
      emitcode ("anl", "a,#!constbyte", mask);  // 000000BB:CCCCCCDD
      emitcode ("mov", "c,acc[0]");     // c = B
      emitcode ("xch", "a,%s", x);      // CCCCCCDD:000000BB
      emitcode ("rrc", "a");
      emitcode ("xch", "a,%s", x);
      emitcode ("rrc", "a");
      emitcode ("mov", "c,acc[0]");     //<< get correct bit
      emitcode ("xch", "a,%s", x);
      emitcode ("rrc", "a");
      emitcode ("xch", "a,%s", x);
      emitcode ("rrc", "a");
      emitcode ("xch", "a,%s", x);
      break;
    case 7:                    // a:x <<= 7
      mask = SRMask[shCount];
      emitcode ("anl", "a,#!constbyte", mask);  // 0000000B:CCCCCCCD
      emitcode ("mov", "c,acc[0]");     // c = B
      emitcode ("xch", "a,%s", x);      // CCCCCCCD:0000000B
      AccAXRrl1 (x);            // BCCCCCCC:D0000000
      break;
    default:
      break;
    }
}

/*-----------------------------------------------------------------*/
/* AccAXRsh - right shift a:x known count (0..7)                   */
/*-----------------------------------------------------------------*/
static void
AccAXRsh (const char *x, int shCount)
{
  unsigned char mask = SRMask[shCount];

  switch (shCount)
    {
    case 0:
      break;
    case 1:
      CLRC;
      AccAXRrl1 (x);            // 0->a:x
      break;
    case 2:
      CLRC;
      AccAXRrl1 (x);            // 0->a:x
      CLRC;
      AccAXRrl1 (x);            // 0->a:x
      break;
    case 3:
    case 4:
    case 5:                    // AAAAABBB:CCCCCDDD = a:x
      AccRol (8 - shCount);     // BBBAAAAA:DDDCCCCC
      emitcode ("xch", "a,%s", x);      // CCCCCDDD:BBBAAAAA
      AccRol (8 - shCount);     // DDDCCCCC:BBBAAAAA
      emitcode ("anl", "a,#!constbyte", mask);  // 000CCCCC:BBBAAAAA
      emitcode ("xrl", "a,%s", x);      // BBB(CCCCC^AAAAA):BBBAAAAA
      emitcode ("xch", "a,%s", x);      // BBBAAAAA:BBB(CCCCC^AAAAA)
      emitcode ("anl", "a,#!constbyte", mask);  // 000AAAAA:BBB(CCCCC^AAAAA)
      emitcode ("xch", "a,%s", x);      // BBB(CCCCC^AAAAA):000AAAAA
      emitcode ("xrl", "a,%s", x);      // BBBCCCCC:000AAAAA
      emitcode ("xch", "a,%s", x);      // 000AAAAA:BBBCCCCC
      break;
    case 6:                    // AABBBBBB:CCDDDDDD
      emitcode ("mov", "c,acc[7]");
      AccAXLrl1 (x);            // ABBBBBBC:CDDDDDDA
      emitcode ("mov", "c,acc[7]");
      AccAXLrl1 (x);            // BBBBBBCC:DDDDDDAA
      emitcode ("xch", "a,%s", x);      // DDDDDDAA:BBBBBBCC
      emitcode ("anl", "a,#!constbyte", mask);  // 000000AA:BBBBBBCC
      break;
    case 7:                    // ABBBBBBB:CDDDDDDD
      emitcode ("mov", "c,acc[7]");     // c = A
      AccAXLrl1 (x);            // BBBBBBBC:DDDDDDDA
      emitcode ("xch", "a,%s", x);      // DDDDDDDA:BBBBBBCC
      emitcode ("anl", "a,#!constbyte", mask);  // 0000000A:BBBBBBBC
      break;
    default:
      break;
    }
}

/*-----------------------------------------------------------------*/
/* AccAXRshS - right shift signed a:x known count (0..7)           */
/*-----------------------------------------------------------------*/
static void
AccAXRshS (const char *x, int shCount)
{
  symbol *tlbl;
  unsigned char mask = SRMask[shCount];

  switch (shCount)
    {
    case 0:
      break;
    case 1:
      emitcode ("mov", "c,acc[7]");
      AccAXRrl1 (x);            // s->a:x
      break;
    case 2:
      emitcode ("mov", "c,acc[7]");
      AccAXRrl1 (x);            // s->a:x
      emitcode ("mov", "c,acc[7]");
      AccAXRrl1 (x);            // s->a:x
      break;
    case 3:
    case 4:
    case 5:                    // AAAAABBB:CCCCCDDD = a:x
      tlbl = newiTempLabel (NULL);
      AccRol (8 - shCount);     // BBBAAAAA:CCCCCDDD
      emitcode ("xch", "a,%s", x);      // CCCCCDDD:BBBAAAAA
      AccRol (8 - shCount);     // DDDCCCCC:BBBAAAAA
      emitcode ("anl", "a,#!constbyte", mask);  // 000CCCCC:BBBAAAAA
      emitcode ("xrl", "a,%s", x);      // BBB(CCCCC^AAAAA):BBBAAAAA
      emitcode ("xch", "a,%s", x);      // BBBAAAAA:BBB(CCCCC^AAAAA)
      emitcode ("anl", "a,#!constbyte", mask);  // 000AAAAA:BBB(CCCCC^AAAAA)
      emitcode ("xch", "a,%s", x);      // BBB(CCCCC^AAAAA):000AAAAA
      emitcode ("xrl", "a,%s", x);      // BBBCCCCC:000AAAAA
      emitcode ("xch", "a,%s", x);      // 000SAAAA:BBBCCCCC
      emitcode ("jnb", "acc[%d],!tlabel", 7 - shCount, labelKey2num (tlbl->key));
      mask = ~SRMask[shCount];
      emitcode ("orl", "a,#!constbyte", mask);  // 111AAAAA:BBBCCCCC
      emitLabel (tlbl);
      break;                    // SSSSAAAA:BBBCCCCC
    case 6:                    // AABBBBBB:CCDDDDDD
      tlbl = newiTempLabel (NULL);
      emitcode ("mov", "c,acc[7]");
      AccAXLrl1 (x);            // ABBBBBBC:CDDDDDDA
      emitcode ("mov", "c,acc[7]");
      AccAXLrl1 (x);            // BBBBBBCC:DDDDDDAA
      emitcode ("xch", "a,%s", x);      // DDDDDDAA:BBBBBBCC
      emitcode ("anl", "a,#!constbyte", mask);  // 000000AA:BBBBBBCC
      emitcode ("jnb", "acc[%d],!tlabel", 7 - shCount, labelKey2num (tlbl->key));
      mask = ~SRMask[shCount];
      emitcode ("orl", "a,#!constbyte", mask);  // 111111AA:BBBBBBCC
      emitLabel (tlbl);
      break;
    case 7:                    // ABBBBBBB:CDDDDDDD
      tlbl = newiTempLabel (NULL);
      emitcode ("mov", "c,acc[7]");      // c = A
      AccAXLrl1 (x);            // BBBBBBBC:DDDDDDDA
      emitcode ("xch", "a,%s", x);      // DDDDDDDA:BBBBBBCC
      emitcode ("anl", "a,#!constbyte", mask);  // 0000000A:BBBBBBBC
      emitcode ("jnb", "acc[%d],!tlabel", 7 - shCount, labelKey2num (tlbl->key));
      mask = ~SRMask[shCount];
      emitcode ("orl", "a,#!constbyte", mask);  // 1111111A:BBBBBBBC
      emitLabel (tlbl);
      break;
    default:
      break;
    }
}

static void
_loadLeftIntoAx (const char **lsb, operand * left, operand * result, int offl, int offr)
{
  // Get the initial value from left into a pair of registers.
  // MSB must be in A, LSB can be any register.
  //
  // If the result is held in registers, it is an optimization
  // if the LSB can be held in the register which will hold the,
  // result LSB since this saves us from having to copy it into
  // the result following AccAXLsh.
  //
  // If the result is addressed indirectly, this is not a gain.
  if (AOP_NEEDSACC (result))
    {
      _startLazyDPSEvaluation ();
      if (AOP_TYPE (left) == AOP_DPTR2)
        {
          // Get MSB in A.
          MOVA (aopGet (left, offl + MSB16, FALSE, FALSE, NULL));
          // get LSB in DP2_RESULT_REG.
          assert (!strcmp (aopGet (left, offl, FALSE, FALSE, DP2_RESULT_REG), DP2_RESULT_REG));
        }
      else
        {
          const char *leftByte;

          // get LSB into DP2_RESULT_REG
          leftByte = aopGet (left, offl, FALSE, FALSE, NULL);
          if (strcmp (leftByte, DP2_RESULT_REG))
            {
              TR_AP ("#7");
              emitcode ("mov", "%s,%s", DP2_RESULT_REG, leftByte);
            }
          // And MSB in A.
          leftByte = aopGet (left, offl + MSB16, FALSE, FALSE, NULL);
          assert (strcmp (leftByte, DP2_RESULT_REG));
          MOVA (leftByte);
        }
      _endLazyDPSEvaluation ();
      *lsb = DP2_RESULT_REG;
    }
  else
    {
      if (sameRegs (AOP (result), AOP (left)) && ((offl + MSB16) == offr))
        {
          /* don't crash result[offr] */
          MOVA (aopGet (left, offl, FALSE, FALSE, NULL));
          emitcode ("xch", "a,%s", aopGet (left, offl + MSB16, FALSE, FALSE, DP2_RESULT_REG));
        }
      else
        {
          movLeft2Result (left, offl, result, offr, 0);
          MOVA (aopGet (left, offl + MSB16, FALSE, FALSE, NULL));
        }
      *lsb = aopGet (result, offr, FALSE, FALSE, DP2_RESULT_REG);
      assert (strcmp (*lsb, "a"));
    }
}

static void
_storeAxResults (const char *lsb, operand * result, int offr)
{
  _startLazyDPSEvaluation ();
  if (AOP_NEEDSACC (result))
    {
      /* We have to explicitly update the result LSB.
       */
      emitcode ("xch", "a,%s", lsb);
      aopPut (result, "a", offr);
      emitcode ("mov", "a,%s", lsb);
    }
  if (getDataSize (result) > 1)
    {
      aopPut (result, "a", offr + MSB16);
    }
  _endLazyDPSEvaluation ();
}

/*-----------------------------------------------------------------*/
/* shiftL2Left2Result - shift left two bytes from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftL2Left2Result (operand * left, int offl, operand * result, int offr, int shCount)
{
  const char *lsb;

  _loadLeftIntoAx (&lsb, left, result, offl, offr);

  AccAXLsh (lsb, shCount);

  _storeAxResults (lsb, result, offr);
}

/*-----------------------------------------------------------------*/
/* shiftR2Left2Result - shift right two bytes from left to result  */
/*-----------------------------------------------------------------*/
static void
shiftR2Left2Result (operand * left, int offl, operand * result, int offr, int shCount, int sign)
{
  const char *lsb;

  _loadLeftIntoAx (&lsb, left, result, offl, offr);

  /* a:x >> shCount (x = lsb(result)) */
  if (sign)
    AccAXRshS (lsb, shCount);
  else
    AccAXRsh (lsb, shCount);


  _storeAxResults (lsb, result, offr);
}

/*------------------------------------------------------------------*/
/* shiftLLeftOrResult - shift left one byte from left, or to result */
/*------------------------------------------------------------------*/
static void
shiftLLeftOrResult (operand * left, int offl, operand * result, int offr, int shCount)
{
  MOVA (aopGet (left, offl, FALSE, FALSE, NULL));
  /* shift left accumulator */
  AccLsh (shCount);
  /* or with result */
  if (aopGetUsesAcc (result, offr))
    {
      emitcode ("xch", "a,b");
      MOVA (aopGet (result, offr, FALSE, FALSE, DP2_RESULT_REG));
      emitcode ("orl", "a,b");
    }
  else
    {
      emitcode ("orl", "a,%s", aopGet (result, offr, FALSE, FALSE, DP2_RESULT_REG));
    }
  /* back to result */
  aopPut (result, "a", offr);
}

/*-----------------------------------------------------------------*/
/* shiftRLeftOrResult - shift right one byte from left,or to result */
/*-----------------------------------------------------------------*/
static void
shiftRLeftOrResult (operand * left, int offl, operand * result, int offr, int shCount)
{
  MOVA (aopGet (left, offl, FALSE, FALSE, NULL));
  /* shift right accumulator */
  AccRsh (shCount);
  /* or with result */
  if (aopGetUsesAcc (result, offr))
    {
      emitcode ("xch", "a,b");
      MOVA (aopGet (result, offr, FALSE, FALSE, DP2_RESULT_REG));
      emitcode ("orl", "a,b");
    }
  else
    {
      emitcode ("orl", "a,%s", aopGet (result, offr, FALSE, FALSE, DP2_RESULT_REG));
    }
  /* back to result */
  aopPut (result, "a", offr);
}

/*-----------------------------------------------------------------*/
/* genlshTwo - left shift two bytes by known amount != 0           */
/*-----------------------------------------------------------------*/
static void
genlshTwo (operand * result, operand * left, int shCount)
{
  int size;

  D (emitcode (";", "genlshTwo"));

  size = getDataSize (result);

  /* if shCount >= 8 */
  if (shCount >= 8)
    {
      shCount -= 8;

      if (size > 1)
        {
          if (shCount)
            {
              _startLazyDPSEvaluation ();
              _endLazyDPSEvaluation ();
              shiftL1Left2Result (left, LSB, result, MSB16, shCount);
              aopPut (result, zero, LSB);
            }
          else
            {
              _startLazyDPSEvaluation ();
              movLeft2Result (left, LSB, result, MSB16, 0);
              aopPut (result, zero, LSB);
              _endLazyDPSEvaluation ();
            }
        }
      else
        {
          _startLazyDPSEvaluation ();
          aopPut (result, zero, LSB);
          _endLazyDPSEvaluation ();
        }
    }

  /*  1 <= shCount <= 7 */
  else
    {
      if (size == 1)
        shiftL1Left2Result (left, LSB, result, LSB, shCount);
      else
        shiftL2Left2Result (left, LSB, result, LSB, shCount);
    }
}

/*-----------------------------------------------------------------*/
/* shiftLLong - shift left one long from left to result            */
/* offl = LSB or MSB16                                             */
/*-----------------------------------------------------------------*/
static void
shiftLLong (operand * left, operand * result, int offr)
{
  int offl = LSB;
  int size = AOP_SIZE (result);
  int useXch = (sameRegs (AOP (left), AOP (result)) && size >= MSB16 + offr && offr != LSB);

  if (size > offl + offr)
    {
      MOVA (aopGet (left, offl, FALSE, FALSE, NULL));
      emitcode ("add", "a,acc");
      if (useXch)
        xch_a_aopGet (left, offl + offr, FALSE, FALSE, NULL);
      else
        aopPut (result, "a", offl + offr);
    }

  for (offl = LSB + 1; offl < LSB + 8; offl++)
    {
      if (size > offl + offr)
        {
          if (!useXch)
            MOVA (aopGet (left, offl, FALSE, FALSE, NULL));
          emitcode ("rlc", "a");
          if (useXch)
            xch_a_aopGet (left, offl + offr, FALSE, FALSE, NULL);
          else
            aopPut (result, "a", offl + offr);
        }
    }
}

/*-----------------------------------------------------------------*/
/* genlshFixed - shift four byte by a known amount != 0            */
/*-----------------------------------------------------------------*/
static void
genlshFixed (operand *result, operand *left, int shCount)
{
  int size, b;
  int full_bytes;

  D (emitcode (";", "genlshFixed"));

  size = AOP_SIZE (result);

  full_bytes = shCount / 8;
  shCount -= full_bytes * 8;
  if (shCount == 0)
    {
      for (b = size - 1; b > full_bytes - 1; b--)
        movLeft2Result (left, b - full_bytes, result, b, 0);
    }
  else if ((shCount == 1) && (full_bytes < 2))
    {
      shiftLLong (left, result, full_bytes);
    }
  else if ((shCount == 2) && (full_bytes == 0) && !isOperandVolatile (result, FALSE))
    {
      shiftLLong (left, result, full_bytes);
      shiftLLong (result, result, full_bytes);
    }
  else
    {
      int off;
      for (off = size - 2; off - full_bytes >= 0; off -= 2)
        {
          shiftL2Left2Result (left, off - full_bytes, result, off, shCount);
          if (off - full_bytes - 1 >= 0)
              shiftRLeftOrResult (left, off - full_bytes - 1, result, off, 8 - shCount);
        }
      if (off - full_bytes == -1)
        {
          shiftL1Left2Result (left, LSB, result, full_bytes, shCount);
        }
    }
  for (b = LSB; b < full_bytes; b++)
    aopPut (result, zero, b);
  return;
}

/*-----------------------------------------------------------------*/
/* genLeftShiftLiteral - left shifting by known count              */
/*-----------------------------------------------------------------*/
static void
genLeftShiftLiteral (operand * left, operand * right, operand * result, iCode * ic)
{
  int shCount = (int) ulFromVal (AOP (right)->aopu.aop_lit);
  int size;

  size = getSize (operandType (result));

  D (emitcode (";", "genLeftShiftLiteral (%d), size %d", shCount, size));

  freeAsmop (right, NULL, ic, TRUE);

  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

#if VIEW_SIZE
  emitcode ("; shift left ", "result %d, left %d", size, AOP_SIZE (left));
#endif

  /* I suppose that the left size >= result size */
  if (shCount == 0)
    {
      _startLazyDPSEvaluation ();
      while (size--)
        {
          movLeft2Result (left, size, result, size, 0);
        }
      _endLazyDPSEvaluation ();
    }
  else if (shCount >= (size * 8))
    {
      _startLazyDPSEvaluation ();
      while (size--)
        {
          aopPut (result, zero, size);
        }
      _endLazyDPSEvaluation ();
    }
  else
    {
      switch (size)
        {
        case 2:
          genlshTwo (result, left, shCount);
          break;

        case 1:
        case 4:
        case 8:
          genlshFixed (result, left, shCount);
          break;

        default:
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "*** ack! mystery literal shift!\n");
          break;
        }
    }
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genLeftShift - generates code for left shifting                 */
/*-----------------------------------------------------------------*/
static void
genLeftShift (iCode * ic)
{
  operand *left, *right, *result;
  int size, offset;
  symbol *tlbl, *tlbl1;
  bool pushedB;

  D (emitcode (";", "genLeftShift"));

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic, FALSE, FALSE);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      genLeftShiftLiteral (left, right, result, ic);
      return;
    }

  /* shift count is unknown then we have to form
     a loop get the loop count in B : Note: we take
     only the lower order byte since shifting
     more that 32 bits makes no sense anyway, ( the
     largest size of an object can be only 32 bits ) */

  pushedB = pushB ();
  if (AOP_TYPE (right) == AOP_LIT)
    {
      /* Really should be handled by genLeftShiftLiteral,
       * but since I'm too lazy to fix that today, at least we can make
       * some small improvement.
       */
      emitcode ("mov", "b,#!constbyte", ((int) ulFromVal (AOP (right)->aopu.aop_lit)) + 1);
    }
  else
    {
      MOVB (aopGet (right, 0, FALSE, FALSE, "b"));
      emitcode ("inc", "b");
    }
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

  /* now move the left to the result if they are not the same */
  if (!sameRegs (AOP (left), AOP (result)) && AOP_SIZE (result) > 1)
    {
      size = AOP_SIZE (result);
      offset = 0;
      _startLazyDPSEvaluation ();
      while (size--)
        {
          const char *l = aopGet (left, offset, FALSE, TRUE, NULL);
          if (*l == '@' && (IS_AOP_PREG (result)))
            {

              emitcode ("mov", "a,%s", l);
              aopPut (result, "a", offset);
            }
          else
            aopPut (result, l, offset);
          offset++;
        }
      _endLazyDPSEvaluation ();
    }

  tlbl = newiTempLabel (NULL);
  size = AOP_SIZE (result);
  offset = 0;
  tlbl1 = newiTempLabel (NULL);

  /* if it is only one byte then */
  if (size == 1)
    {
      symbol *tlbl1 = newiTempLabel (NULL);

      MOVA (aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("sjmp", "!tlabel", labelKey2num (tlbl1->key));
      emitLabel (tlbl);
      emitcode ("add", "a,acc");
      emitLabel (tlbl1);
      emitcode ("djnz", "b,!tlabel", labelKey2num (tlbl->key));
      popB (pushedB);
      aopPut (result, "a", 0);
      goto release;
    }

  reAdjustPreg (AOP (result));

  emitcode ("sjmp", "!tlabel", labelKey2num (tlbl1->key));
  emitLabel (tlbl);
  MOVA (aopGet (result, offset, FALSE, FALSE, NULL));
  emitcode ("add", "a,acc");
  aopPut (result, "a", offset++);
  _startLazyDPSEvaluation ();
  while (--size)
    {
      MOVA (aopGet (result, offset, FALSE, FALSE, NULL));
      emitcode ("rlc", "a");
      aopPut (result, "a", offset++);
    }
  _endLazyDPSEvaluation ();
  reAdjustPreg (AOP (result));

  emitLabel (tlbl1);
  emitcode ("djnz", "b,!tlabel", labelKey2num (tlbl->key));
  popB (pushedB);
release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genrshOne - right shift a one byte quantity by known count      */
/*-----------------------------------------------------------------*/
static void
genrshOne (operand * result, operand * left, int shCount, int sign)
{
  D (emitcode (";", "genrshOne"));

  shiftR1Left2Result (left, LSB, result, LSB, shCount, sign);
}

/*-----------------------------------------------------------------*/
/* genrshTwo - right shift two bytes by known amount != 0          */
/*-----------------------------------------------------------------*/
static void
genrshTwo (operand * result, operand * left, int shCount, int sign)
{
  D (emitcode (";", "genrshTwo"));

  /* if shCount >= 8 */
  if (shCount >= 8)
    {
      shCount -= 8;
      _startLazyDPSEvaluation ();
      if (shCount)
        shiftR1Left2Result (left, MSB16, result, LSB, shCount, sign);
      else
        movLeft2Result (left, MSB16, result, LSB, sign);
      addSign (result, MSB16, sign);
      _endLazyDPSEvaluation ();
    }

  /*  1 <= shCount <= 7 */
  else
    {
      shiftR2Left2Result (left, LSB, result, LSB, shCount, sign);
    }
}

/*-----------------------------------------------------------------*/
/* shiftRLong - shift right one long from left to result           */
/* offl = LSB or MSB16                                             */
/*-----------------------------------------------------------------*/
static void
shiftRLong (operand * left, int offl, operand * result, int sign)
{
  bool overlapping = regsInCommon (left, result) || operandsEqu (left, result);

  if (overlapping && offl > 1)
    {
      // we are in big trouble, but this shouldn't happen
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__);
    }

  MOVA (aopGet (left, MSB32, FALSE, FALSE, NULL));

  if (offl == MSB16)
    {
      // shift is > 8
      if (sign)
        {
          emitcode ("rlc", "a");
          emitcode ("subb", "a,acc");
          if (overlapping && sameByte (AOP (left), MSB32, AOP (result), MSB32))
            {
              xch_a_aopGet (left, MSB32, FALSE, FALSE, DP2_RESULT_REG);
            }
          else
            {
              aopPut (result, "a", MSB32);
              MOVA (aopGet (left, MSB32, FALSE, FALSE, DP2_RESULT_REG));
            }
        }
      else
        {
          if (aopPutUsesAcc (result, zero, MSB32))
            {
              emitcode ("xch", "a,b");
              aopPut (result, zero, MSB32);
              emitcode ("xch", "a,b");
            }
          else
            {
              aopPut (result, zero, MSB32);
            }
        }
    }

  if (!sign)
    {
      emitcode ("clr", "c");
    }
  else
    {
      emitcode ("mov", "c,acc[7]");
    }

  emitcode ("rrc", "a");

  if (overlapping && offl == MSB16 && sameByte (AOP (left), MSB24, AOP (result), MSB32 - offl))
    {
      xch_a_aopGet (left, MSB24, FALSE, FALSE, DP2_RESULT_REG);
    }
  else
    {
      aopPut (result, "a", MSB32 - offl);
      MOVA (aopGet (left, MSB24, FALSE, FALSE, NULL));
    }

  emitcode ("rrc", "a");
  if (overlapping && offl == MSB16 && sameByte (AOP (left), MSB16, AOP (result), MSB24 - offl))
    {
      xch_a_aopGet (left, MSB16, FALSE, FALSE, DP2_RESULT_REG);
    }
  else
    {
      aopPut (result, "a", MSB24 - offl);
      MOVA (aopGet (left, MSB16, FALSE, FALSE, NULL));
    }

  emitcode ("rrc", "a");
  if (offl != LSB)
    {
      aopPut (result, "a", MSB16 - offl);
    }
  else
    {
      if (overlapping && sameByte (AOP (left), LSB, AOP (result), MSB16 - offl))
        {
          xch_a_aopGet (left, LSB, FALSE, FALSE, DP2_RESULT_REG);
        }
      else
        {
          aopPut (result, "a", MSB16 - offl);
          MOVA (aopGet (left, LSB, FALSE, FALSE, NULL));
        }
      emitcode ("rrc", "a");
      aopPut (result, "a", LSB);
    }
}

/*-----------------------------------------------------------------*/
/* genrshFour - shift four byte by a known amount != 0             */
/*-----------------------------------------------------------------*/
static void
genrshFour (operand * result, operand * left, int shCount, int sign)
{
  D (emitcode (";", "genrshFour"));

  /* if shifting more that 3 bytes */
  if (shCount >= 24)
    {
      shCount -= 24;
      _startLazyDPSEvaluation ();
      if (shCount)
        shiftR1Left2Result (left, MSB32, result, LSB, shCount, sign);
      else
        movLeft2Result (left, MSB32, result, LSB, sign);
      addSign (result, MSB16, sign);
      _endLazyDPSEvaluation ();
    }
  else if (shCount >= 16)
    {
      shCount -= 16;
      _startLazyDPSEvaluation ();
      if (shCount)
        shiftR2Left2Result (left, MSB24, result, LSB, shCount, sign);
      else
        {
          movLeft2Result (left, MSB24, result, LSB, 0);
          movLeft2Result (left, MSB32, result, MSB16, sign);
        }
      addSign (result, MSB24, sign);
      _endLazyDPSEvaluation ();
    }
  else if (shCount >= 8)
    {
      shCount -= 8;
      _startLazyDPSEvaluation ();
      if (shCount == 1)
        {
          shiftRLong (left, MSB16, result, sign);
        }
      else if (shCount == 0)
        {
          movLeft2Result (left, MSB16, result, LSB, 0);
          movLeft2Result (left, MSB24, result, MSB16, 0);
          movLeft2Result (left, MSB32, result, MSB24, sign);
          addSign (result, MSB32, sign);
        }
      else
        {
          shiftR2Left2Result (left, MSB16, result, LSB, shCount, 0);
          shiftLLeftOrResult (left, MSB32, result, MSB16, 8 - shCount);
          /* the last shift is signed */
          shiftR1Left2Result (left, MSB32, result, MSB24, shCount, sign);
          addSign (result, MSB32, sign);
        }
      _endLazyDPSEvaluation ();
    }
  else
    {
      /* 1 <= shCount <= 7 */
      if (shCount <= 2)
        {
          shiftRLong (left, LSB, result, sign);
          if (shCount == 2)
            shiftRLong (result, LSB, result, sign);
        }
      else
        {
          shiftR2Left2Result (left, LSB, result, LSB, shCount, 0);
          shiftLLeftOrResult (left, MSB24, result, MSB16, 8 - shCount);
          shiftR2Left2Result (left, MSB24, result, MSB24, shCount, sign);
        }
    }
}

/*-----------------------------------------------------------------*/
/* genrshAny - shift any number of bytes by a known amount != 0    */
/*-----------------------------------------------------------------*/
static void
genrshAny (operand *result, operand *left, int shCount, int sign)
{
  int size, size2, offset;

  D (emitcode (";", "genrshAny"));

  size = AOP_SIZE (result);

  if (!operandsEqu (result, left))
    for (size2 = size, offset = 0; size2 > 0; size2--, offset++)
      aopPut (result, aopGet (left, offset, FALSE, FALSE, NULL), offset);

  while (shCount--)
    {
      MOVA (aopGet (result, size - 1, FALSE, FALSE, NULL));
      if (!sign)
        emitcode ("clr", "c");
      else
        emitcode ("mov", "c,acc.7");
      emitcode ("rrc", "a");
      aopPut (result, "a", size - 1);

      for(size2 = size - 1, offset = size - 2; size2 > 0; size2--, offset--)
        {

          MOVA (aopGet (result, offset, FALSE, FALSE, NULL));
          emitcode ("rrc", "a");
          aopPut (result, "a", offset);
        }
    }
}

/*-----------------------------------------------------------------*/
/* genRightShiftLiteral - right shifting by known count            */
/*-----------------------------------------------------------------*/
static void
genRightShiftLiteral (operand * left, operand * right, operand * result, iCode * ic, int sign)
{
  int shCount = (int) ulFromVal (AOP (right)->aopu.aop_lit);
  int size;

  size = getSize (operandType (result));

  D (emitcode (";", "genRightShiftLiteral (%d), size %d", shCount, size));

  freeAsmop (right, NULL, ic, TRUE);

  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

#if VIEW_SIZE
  emitcode ("; shift right ", "result %d, left %d", AOP_SIZE (result), AOP_SIZE (left));
#endif

  /* test the LEFT size !!! */

  /* I suppose that the left size >= result size */
  if (shCount == 0)
    {
      size = getDataSize (result);
      _startLazyDPSEvaluation ();
      while (size--)
        movLeft2Result (left, size, result, size, 0);
      _endLazyDPSEvaluation ();
    }
  else if (shCount >= (size * 8))
    {
      if (sign)
        {
          /* get sign in acc.7 */
          MOVA (aopGet (left, size - 1, FALSE, FALSE, NULL));
        }
      addSign (result, LSB, sign);
    }
  else
    {
      switch (size)
        {
        case 1:
          genrshOne (result, left, shCount, sign);
          break;

        case 2:
          genrshTwo (result, left, shCount, sign);
          break;

        case 4:
          genrshFour (result, left, shCount, sign);
          break;

        default:
          genrshAny (result, left, shCount, sign);
          break;
        }
    }
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genSignedRightShift - right shift of signed number              */
/*-----------------------------------------------------------------*/
static void
genSignedRightShift (iCode * ic)
{
  operand *right, *left, *result;
  int size, offset;
  symbol *tlbl, *tlbl1;
  bool pushedB;

  D (emitcode (";", "genSignedRightShift"));

  /* we do it the hard way put the shift count in b
     and loop thru preserving the sign */

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic, FALSE, FALSE);

  if (AOP_TYPE (right) == AOP_LIT)
    {
      genRightShiftLiteral (left, right, result, ic, 1);
      return;
    }
  /* shift count is unknown then we have to form
     a loop get the loop count in B : Note: we take
     only the lower order byte since shifting
     more that 32 bits make no sense anyway, ( the
     largest size of an object can be only 32 bits ) */

  pushedB = pushB ();
  if (AOP_TYPE (right) == AOP_LIT)
    {
      /* Really should be handled by genRightShiftLiteral,
       * but since I'm too lazy to fix that today, at least we can make
       * some small improvement.
       */
      emitcode ("mov", "b,#!constbyte", ((int) ulFromVal (AOP (right)->aopu.aop_lit)) + 1);
    }
  else
    {
      MOVB (aopGet (right, 0, FALSE, FALSE, "b"));
      emitcode ("inc", "b");
    }
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

  /* now move the left to the result if they are not the
     same */
  if (!sameRegs (AOP (left), AOP (result)) && AOP_SIZE (result) > 1)
    {

      size = AOP_SIZE (result);
      offset = 0;
      _startLazyDPSEvaluation ();
      while (size--)
        {
          const char *l = aopGet (left, offset, FALSE, TRUE, NULL);
          if (*l == '@' && IS_AOP_PREG (result))
            {

              emitcode ("mov", "a,%s", l);
              aopPut (result, "a", offset);
            }
          else
            aopPut (result, l, offset);
          offset++;
        }
      _endLazyDPSEvaluation ();
    }

  /* mov the highest order bit to OVR */
  tlbl = newiTempLabel (NULL);
  tlbl1 = newiTempLabel (NULL);

  size = AOP_SIZE (result);
  offset = size - 1;
  MOVA (aopGet (left, offset, FALSE, FALSE, NULL));
  emitcode ("rlc", "a");
  emitcode ("mov", "ov,c");
  /* if it is only one byte then */
  if (size == 1)
    {
      MOVA (aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("sjmp", "!tlabel", labelKey2num (tlbl1->key));
      emitLabel (tlbl);
      emitcode ("mov", "c,ov");
      emitcode ("rrc", "a");
      emitLabel (tlbl1);
      emitcode ("djnz", "b,!tlabel", labelKey2num (tlbl->key));
      popB (pushedB);
      aopPut (result, "a", 0);
      goto release;
    }

  reAdjustPreg (AOP (result));
  emitcode ("sjmp", "!tlabel", labelKey2num (tlbl1->key));
  emitLabel (tlbl);
  emitcode ("mov", "c,ov");
  _startLazyDPSEvaluation ();
  while (size--)
    {
      MOVA (aopGet (result, offset, FALSE, FALSE, NULL));
      emitcode ("rrc", "a");
      aopPut (result, "a", offset--);
    }
  _endLazyDPSEvaluation ();
  reAdjustPreg (AOP (result));
  emitLabel (tlbl1);
  emitcode ("djnz", "b,!tlabel", labelKey2num (tlbl->key));
  popB (pushedB);

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genRightShift - generate code for right shifting                */
/*-----------------------------------------------------------------*/
static void
genRightShift (iCode * ic)
{
  operand *right, *left, *result;
  sym_link *letype;
  int size, offset;
  symbol *tlbl, *tlbl1;
  bool pushedB;

  D (emitcode (";", "genRightShift"));

  /* if signed then we do it the hard way preserve the
     sign bit moving it inwards */
  letype = getSpec (operandType (IC_LEFT (ic)));

  if (!SPEC_USIGN (letype))
    {
      genSignedRightShift (ic);
      return;
    }

  /* signed & unsigned types are treated the same : i.e. the
     signed is NOT propagated inwards : quoting from the
     ANSI - standard : "for E1 >> E2, is equivalent to division
     by 2**E2 if unsigned or if it has a non-negative value,
     otherwise the result is implementation defined ", MY definition
     is that the sign does not get propagated */

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic, FALSE, FALSE);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      genRightShiftLiteral (left, right, result, ic, 0);
      return;
    }

  /* shift count is unknown then we have to form
     a loop get the loop count in B : Note: we take
     only the lower order byte since shifting
     more that 32 bits make no sense anyway, ( the
     largest size of an object can be only 32 bits ) */

  pushedB = pushB ();
  if (AOP_TYPE (right) == AOP_LIT)
    {
      /* Really should be handled by genRightShiftLiteral,
       * but since I'm too lazy to fix that today, at least we can make
       * some small improvement.
       */
      emitcode ("mov", "b,#!constbyte", ((int) ulFromVal (AOP (right)->aopu.aop_lit)) + 1);
    }
  else
    {
      MOVB (aopGet (right, 0, FALSE, FALSE, "b"));
      emitcode ("inc", "b");
    }
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (left, ic, FALSE, FALSE);
  aopOp (result, ic, FALSE, AOP_USESDPTR (left));

  /* now move the left to the result if they are not the
     same */
  if (!sameRegs (AOP (left), AOP (result)) && AOP_SIZE (result) > 1)
    {
      size = AOP_SIZE (result);
      offset = 0;
      _startLazyDPSEvaluation ();
      while (size--)
        {
          const char *l = aopGet (left, offset, FALSE, TRUE, NULL);
          if (*l == '@' && IS_AOP_PREG (result))
            {

              emitcode ("mov", "a,%s", l);
              aopPut (result, "a", offset);
            }
          else
            aopPut (result, l, offset);
          offset++;
        }
      _endLazyDPSEvaluation ();
    }

  tlbl = newiTempLabel (NULL);
  tlbl1 = newiTempLabel (NULL);
  size = AOP_SIZE (result);
  offset = size - 1;

  /* if it is only one byte then */
  if (size == 1)
    {
      MOVA (aopGet (left, 0, FALSE, FALSE, NULL));
      emitcode ("sjmp", "!tlabel", labelKey2num (tlbl1->key));
      emitLabel (tlbl);
      CLRC;
      emitcode ("rrc", "a");
      emitLabel (tlbl1);
      emitcode ("djnz", "b,!tlabel", labelKey2num (tlbl->key));
      popB (pushedB);
      aopPut (result, "a", 0);
      goto release;
    }

  reAdjustPreg (AOP (result));
  emitcode ("sjmp", "!tlabel", labelKey2num (tlbl1->key));
  emitLabel (tlbl);
  CLRC;
  _startLazyDPSEvaluation ();
  while (size--)
    {
      MOVA (aopGet (result, offset, FALSE, FALSE, NULL));
      emitcode ("rrc", "a");
      aopPut (result, "a", offset--);
    }
  _endLazyDPSEvaluation ();
  reAdjustPreg (AOP (result));

  emitLabel (tlbl1);
  emitcode ("djnz", "b,!tlabel", labelKey2num (tlbl->key));
  popB (pushedB);

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* emitPtrByteGet - emits code to get a byte into A through a      */
/*                  pointer register (R0, R1, or DPTR). The        */
/*                  original value of A can be preserved in B.     */
/*-----------------------------------------------------------------*/
static void
emitPtrByteGet (const char *rname, int p_type, bool preserveAinB)
{
  switch (p_type)
    {
    case IPOINTER:
    case POINTER:
      if (preserveAinB)
        emitcode ("mov", "b,a");
      emitcode ("mov", "a,@%s", rname);
      break;

    case PPOINTER:
      if (preserveAinB)
        emitcode ("mov", "b,a");
      emitcode ("movx", "a,@%s", rname);
      break;

    case FPOINTER:
      if (preserveAinB)
        emitcode ("mov", "b,a");
      emitcode ("movx", "a,@dptr");
      break;

    case CPOINTER:
      if (preserveAinB)
        emitcode ("mov", "b,a");
      emitcode ("clr", "a");
      emitcode ("movc", "a,@a+dptr");
      break;

    case GPOINTER:
      if (preserveAinB)
        {
          emitpush ("b");
          emitpush ("acc");
        }
      emitcode ("lcall", "__gptrget");
      if (preserveAinB)
        emitpop ("b");
      break;
    }
}

/*-----------------------------------------------------------------*/
/* emitPtrByteSet - emits code to set a byte from src through a    */
/*                  pointer register (R0, R1, or DPTR).            */
/*-----------------------------------------------------------------*/
static void
emitPtrByteSet (const char *rname, int p_type, const char *src)
{
  switch (p_type)
    {
    case IPOINTER:
    case POINTER:
      if (*src == '@')
        {
          MOVA (src);
          emitcode ("mov", "@%s,a", rname);
        }
      else
        emitcode ("mov", "@%s,%s", rname, src);
      break;

    case PPOINTER:
      MOVA (src);
      emitcode ("movx", "@%s,a", rname);
      break;

    case FPOINTER:
      MOVA (src);
      emitcode ("movx", "@dptr,a");
      break;

    case GPOINTER:
      MOVA (src);
      emitcode ("lcall", "__gptrput");
      break;
    }
}

/*-----------------------------------------------------------------*/
/* genUnpackBits - generates code for unpacking bits               */
/*-----------------------------------------------------------------*/
static void
genUnpackBits (operand * result, const char *rname, int ptype)
{
  int offset = 0;               /* result byte offset */
  int rsize;                    /* result size */
  int rlen = 0;                 /* remaining bitfield length */
  sym_link *etype;              /* bitfield type information */
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */

  D (emitcode (";", "genUnpackBits"));

  etype = getSpec (operandType (result));
  rsize = getSize (operandType (result));
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  /* If the bitfield length is less than a byte */
  if (blen < 8)
    {
      emitPtrByteGet (rname, ptype, FALSE);
      AccRol (8 - bstr);
      emitcode ("anl", "a,#!constbyte", ((unsigned char) - 1) >> (8 - blen));
      if (!SPEC_USIGN (etype))
        {
          /* signed bitfield */
          symbol *tlbl = newiTempLabel (NULL);

          emitcode ("jnb", "acc[%d],!tlabel", blen - 1, labelKey2num (tlbl->key));
          emitcode ("orl", "a,#0x%02x", (unsigned char) (0xff << blen));
          emitLabel (tlbl);
        }
      aopPut (result, "a", offset++);
      goto finish;
    }

  /* Bit field did not fit in a byte. Copy all
     but the partial byte at the end.  */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      emitPtrByteGet (rname, ptype, FALSE);
      aopPut (result, "a", offset++);
      if (rlen > 8)
        emitcode ("inc", "%s", rname);
    }

  /* Handle the partial byte at the end */
  if (rlen)
    {
      emitPtrByteGet (rname, ptype, FALSE);
      emitcode ("anl", "a,#!constbyte", ((unsigned char) - 1) >> (8 - rlen));
      if (!SPEC_USIGN (etype))
        {
          /* signed bitfield */
          symbol *tlbl = newiTempLabel (NULL);

          emitcode ("jnb", "acc[%d],!tlabel", rlen - 1, labelKey2num (tlbl->key));
          emitcode ("orl", "a,#0x%02x", (unsigned char) (0xff << rlen));
          emitLabel (tlbl);
        }
      aopPut (result, "a", offset++);
    }

finish:
  if (offset < rsize)
    {
      char *source;

      if (SPEC_USIGN (etype))
        source = zero;
      else
        {
          /* signed bitfield: sign extension with 0x00 or 0xff */
          emitcode ("rlc", "a");
          emitcode ("subb", "a,acc");

          source = "a";
        }
      rsize -= offset;
      while (rsize--)
        aopPut (result, source, offset++);
    }
}


/*-----------------------------------------------------------------*/
/* genDataPointerGet - generates code when ptr offset is known     */
/*-----------------------------------------------------------------*/
static void
genDataPointerGet (operand * left, operand * result, iCode * ic)
{
  const char *l;
  int size, offset = 0;

  D (emitcode (";", "genDataPointerGet"));

  aopOp (result, ic, TRUE, FALSE);

  /* get the string representation of the name */
  l = aopGet (left, 0, FALSE, TRUE, NULL) + 1;  // remove #
  size = AOP_SIZE (result);
  _startLazyDPSEvaluation ();
  while (size--)
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      if (offset)
        {
          dbuf_printf (&dbuf, "(%s + %d)", l, offset);
        }
      else
        {
          dbuf_append_str (&dbuf, l);
        }
      aopPut (result, dbuf_c_str (&dbuf), offset++);
      dbuf_destroy (&dbuf);
    }
  _endLazyDPSEvaluation ();

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genNearPointerGet - emitcode for near pointer fetch             */
/*-----------------------------------------------------------------*/
static void
genNearPointerGet (operand * left, operand * result, iCode * ic, iCode * pi)
{
  asmop *aop = NULL;
  reg_info *preg = NULL;
  const char *rname;
  sym_link *rtype, *retype, *letype;
  sym_link *ltype = operandType (left);

  D (emitcode (";", "genNearPointerGet"));

  rtype = operandType (result);
  retype = getSpec (rtype);
  letype = getSpec (ltype);

  aopOp (left, ic, FALSE, FALSE);

  /* if left is rematerialisable and
     result is not bitfield variable type and
     the left is pointer to data space i.e
     lower 128 bytes of space */
  if (AOP_TYPE (left) == AOP_IMMD && !IS_BITFIELD (retype) && !IS_BITFIELD (letype) && DCL_TYPE (ltype) == POINTER)
    {
      genDataPointerGet (left, result, ic);
      return;
    }

  /* if the value is already in a pointer register
     then don't need anything more */
  if (!AOP_INPREG (AOP (left)))
    {
      /* otherwise get a free pointer register */
      aop = newAsmop (0);
      preg = getFreePtr (ic, &aop, FALSE);
      emitcode ("mov", "%s,%s", preg->name, aopGet (left, 0, FALSE, TRUE, DP2_RESULT_REG));
      rname = preg->name;
    }
  else
    rname = aopGet (left, 0, FALSE, FALSE, DP2_RESULT_REG);

  freeAsmop (left, NULL, ic, TRUE);
  aopOp (result, ic, FALSE, FALSE);

  /* if bitfield then unpack the bits */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    genUnpackBits (result, rname, POINTER);
  else
    {
      /* we can just get the values */
      int size = AOP_SIZE (result);
      int offset = 0;

      while (size--)
        {
          if (IS_AOP_PREG (result) || AOP_TYPE (result) == AOP_STK)
            {

              emitcode ("mov", "a,@%s", rname);
              aopPut (result, "a", offset);
            }
          else
            {
              struct dbuf_s dbuf;

              dbuf_init (&dbuf, 128);
              dbuf_printf (&dbuf, "@%s", rname);
              aopPut (result, dbuf_c_str (&dbuf), offset);
              dbuf_destroy (&dbuf);
            }
          offset++;
          if (size || pi)
            emitcode ("inc", "%s", rname);
        }
    }

  /* now some housekeeping stuff */
  if (aop)                      /* we had to allocate for this iCode */
    {
      if (pi)
        {
          /* post increment present */
          aopPut (left, rname, 0);
        }
      freeAsmop (NULL, aop, ic, TRUE);
    }
  else
    {
      /* we did not allocate which means left
         already in a pointer register, then
         if size > 0 && this could be used again
         we have to point it back to where it
         belongs */
      if (AOP_SIZE (result) > 1 && !OP_SYMBOL (left)->remat && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth) && !pi)
        {
          int size = AOP_SIZE (result) - 1;
          while (size--)
            emitcode ("dec", "%s", rname);
        }
    }

  /* done */
  freeAsmop (result, NULL, ic, TRUE);
  if (pi)
    pi->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genPagedPointerGet - emitcode for paged pointer fetch           */
/*-----------------------------------------------------------------*/
static void
genPagedPointerGet (operand * left, operand * result, iCode * ic, iCode * pi)
{
  asmop *aop = NULL;
  reg_info *preg = NULL;
  const char *rname;
  sym_link *rtype, *retype, *letype;

  D (emitcode (";", "genPagedPointerGet"));

  rtype = operandType (result);
  retype = getSpec (rtype);
  letype = getSpec (operandType (left));
  aopOp (left, ic, FALSE, FALSE);

  /* if the value is already in a pointer register
     then don't need anything more */
  if (!AOP_INPREG (AOP (left)))
    {
      /* otherwise get a free pointer register */
      aop = newAsmop (0);
      preg = getFreePtr (ic, &aop, FALSE);
      emitcode ("mov", "%s,%s", preg->name, aopGet (left, 0, FALSE, TRUE, NULL));
      rname = preg->name;
    }
  else
    rname = aopGet (left, 0, FALSE, FALSE, NULL);

  freeAsmop (left, NULL, ic, TRUE);
  aopOp (result, ic, FALSE, FALSE);

  /* if bitfield then unpack the bits */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    genUnpackBits (result, rname, PPOINTER);
  else
    {
      /* we have can just get the values */
      int size = AOP_SIZE (result);
      int offset = 0;

      while (size--)
        {

          emitcode ("movx", "a,@%s", rname);
          aopPut (result, "a", offset);

          offset++;

          if (size || pi)
            emitcode ("inc", "%s", rname);
        }
    }

  /* now some housekeeping stuff */
  if (aop)                      /* we had to allocate for this iCode */
    {
      if (pi)
        aopPut (left, rname, 0);
      freeAsmop (NULL, aop, ic, TRUE);
    }
  else
    {
      /* we did not allocate which means left
         already in a pointer register, then
         if size > 0 && this could be used again
         we have to point it back to where it
         belongs */
      if (AOP_SIZE (result) > 1 && !OP_SYMBOL (left)->remat && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth) && !pi)
        {
          int size = AOP_SIZE (result) - 1;
          while (size--)
            emitcode ("dec", "%s", rname);
        }
    }

  /* done */
  freeAsmop (result, NULL, ic, TRUE);
  if (pi)
    pi->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genFarPointerGet - get value from far space                     */
/*-----------------------------------------------------------------*/
static void
genFarPointerGet (operand * left, operand * result, iCode * ic, iCode * pi)
{
  int size, offset, dopi;
  sym_link *retype = getSpec (operandType (result));
  sym_link *letype = getSpec (operandType (left));

  D (emitcode (";", "genFarPointerGet"));

  aopOp (left, ic, FALSE, FALSE);
  dopi = loadDptrFromOperand (left, FALSE);

  /* so dptr now contains the address */
  aopOp (result, ic, FALSE, (AOP_INDPTRn (left) ? FALSE : TRUE));

  /* if bit then unpack */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    {
      if (AOP_INDPTRn (left))
        {
          genSetDPTR (AOP (left)->aopu.dptr);
        }
      genUnpackBits (result, "dptr", FPOINTER);
      if (AOP_INDPTRn (left))
        {
          genSetDPTR (0);
        }
    }
  else
    {
      size = AOP_SIZE (result);
      offset = 0;

      if (AOP_INDPTRn (left) && AOP_USESDPTR (result))
        {
          while (size--)
            {
              genSetDPTR (AOP (left)->aopu.dptr);
              emitcode ("movx", "a,@dptr");
              if (size || (dopi && pi && AOP_TYPE (left) != AOP_IMMD))
                emitcode ("inc", "dptr");
              genSetDPTR (0);
              aopPut (result, "a", offset++);
            }
        }
      else
        {
          _startLazyDPSEvaluation ();
          while (size--)
            {
              if (AOP_INDPTRn (left))
                {
                  genSetDPTR (AOP (left)->aopu.dptr);
                }
              else
                {
                  genSetDPTR (0);
                }
              _flushLazyDPS ();

              emitcode ("movx", "a,@dptr");
              if (size || (dopi && pi && AOP_TYPE (left) != AOP_IMMD))
                emitcode ("inc", "dptr");

              aopPut (result, "a", offset++);
            }
          _endLazyDPSEvaluation ();
        }
    }
  if (dopi && pi && AOP_TYPE (left) != AOP_IMMD)
    {
      if (!AOP_INDPTRn (left))
        {
          _startLazyDPSEvaluation ();
          aopPut (left, "dpl", 0);
          aopPut (left, "dph", 1);
          if (options.model == MODEL_FLAT24)
            aopPut (left, "dpx", 2);
          _endLazyDPSEvaluation ();
        }
      pi->generated = 1;
    }
  else if ((IS_OP_RUONLY (left) || AOP_INDPTRn (left)) &&
           AOP_SIZE (result) > 1 && IS_SYMOP (left) && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth))
    {
      size = AOP_SIZE (result) - 1;
      if (AOP_INDPTRn (left))
        {
          genSetDPTR (AOP (left)->aopu.dptr);
        }
      while (size--)
        emitcode ("lcall", "__decdptr");
      if (AOP_INDPTRn (left))
        {
          genSetDPTR (0);
        }
    }

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genCodePointerGet - get value from code space                   */
/*-----------------------------------------------------------------*/
static void
genCodePointerGet (operand * left, operand * result, iCode * ic, iCode * pi)
{
  int size, offset, dopi;
  sym_link *retype = getSpec (operandType (result));

  D (emitcode (";", "genCodePointerGet"));

  aopOp (left, ic, FALSE, FALSE);
  dopi = loadDptrFromOperand (left, FALSE);

  /* so dptr now contains the address */
  aopOp (result, ic, FALSE, (AOP_INDPTRn (left) ? FALSE : TRUE));

  /* if bit then unpack */
  if (IS_BITFIELD (retype))
    {
      if (AOP_INDPTRn (left))
        {
          genSetDPTR (AOP (left)->aopu.dptr);
        }
      genUnpackBits (result, "dptr", CPOINTER);
      if (AOP_INDPTRn (left))
        {
          genSetDPTR (0);
        }
    }
  else
    {
      size = AOP_SIZE (result);
      offset = 0;
      if (AOP_INDPTRn (left) && AOP_USESDPTR (result))
        {
          while (size--)
            {
              genSetDPTR (AOP (left)->aopu.dptr);
              emitcode ("clr", "a");
              emitcode ("movc", "a,@a+dptr");
              if (size || (dopi && pi && AOP_TYPE (left) != AOP_IMMD))
                emitcode ("inc", "dptr");
              genSetDPTR (0);
              aopPut (result, "a", offset++);
            }
        }
      else
        {
          _startLazyDPSEvaluation ();
          while (size--)
            {
              if (AOP_INDPTRn (left))
                {
                  genSetDPTR (AOP (left)->aopu.dptr);
                }
              else
                {
                  genSetDPTR (0);
                }
              _flushLazyDPS ();

              emitcode ("clr", "a");
              emitcode ("movc", "a,@a+dptr");
              if (size || (dopi && pi && AOP_TYPE (left) != AOP_IMMD))
                emitcode ("inc", "dptr");
              aopPut (result, "a", offset++);
            }
          _endLazyDPSEvaluation ();
        }
    }
  if (dopi && pi && AOP_TYPE (left) != AOP_IMMD)
    {
      if (!AOP_INDPTRn (left))
        {
          _startLazyDPSEvaluation ();

          aopPut (left, "dpl", 0);
          aopPut (left, "dph", 1);
          if (options.model == MODEL_FLAT24)
            aopPut (left, "dpx", 2);

          _endLazyDPSEvaluation ();
        }
      pi->generated = 1;
    }
  else if (IS_SYMOP (left) &&
           (IS_OP_RUONLY (left) || AOP_INDPTRn (left)) &&
           AOP_SIZE (result) > 1 && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth))
    {
      size = AOP_SIZE (result) - 1;
      if (AOP_INDPTRn (left))
        {
          genSetDPTR (AOP (left)->aopu.dptr);
        }
      while (size--)
        emitcode ("lcall", "__decdptr");
      if (AOP_INDPTRn (left))
        {
          genSetDPTR (0);
        }
    }

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGenPointerGet - get value from generic pointer space         */
/*-----------------------------------------------------------------*/
static void
genGenPointerGet (operand * left, operand * result, iCode * ic, iCode * pi)
{
  int size, offset, dopi;
  bool pushedB;
  sym_link *retype = getSpec (operandType (result));
  sym_link *letype = getSpec (operandType (left));

  D (emitcode (";", "genGenPointerGet"));

  aopOp (left, ic, FALSE, FALSE);

  pushedB = pushB ();
  dopi = loadDptrFromOperand (left, TRUE);

  /* so dptr-b now contains the address */
  aopOp (result, ic, FALSE, (AOP_INDPTRn (left) ? FALSE : TRUE));

  /* if bit then unpack */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    {
      genUnpackBits (result, "dptr", GPOINTER);
    }
  else
    {
      size = AOP_SIZE (result);
      offset = 0;

      while (size--)
        {
          if (size)
            {
              // Get two bytes at a time, results in _AP & A.
              // dptr will be incremented ONCE by __gptrgetWord.
              //
              // Note: any change here must be coordinated
              // with the implementation of __gptrgetWord
              // in device/lib/_gptrget.c
              emitcode ("lcall", "__gptrgetWord");
              aopPut (result, "a", offset++);
              aopPut (result, DP2_RESULT_REG, offset++);
              size--;
            }
          else
            {
              // Only one byte to get.
              emitcode ("lcall", "__gptrget");
              aopPut (result, "a", offset++);
            }

          if (size || (dopi && pi && AOP_TYPE (left) != AOP_IMMD))
            {
              emitcode ("inc", "dptr");
            }
        }
    }

  if (dopi && pi && AOP_TYPE (left) != AOP_IMMD)
    {
      _startLazyDPSEvaluation ();

      aopPut (left, "dpl", 0);
      aopPut (left, "dph", 1);
      if (options.model == MODEL_FLAT24)
        {
          aopPut (left, "dpx", 2);
          aopPut (left, "b", 3);
        }
      else
        aopPut (left, "b", 2);

      _endLazyDPSEvaluation ();

      pi->generated = 1;
    }
  else if (IS_OP_RUONLY (left) && AOP_SIZE (result) > 1 && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth))
    {
      size = AOP_SIZE (result) - 1;
      while (size--)
        emitcode ("lcall", "__decdptr");
    }
  popB (pushedB);

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void
genPointerGet (iCode * ic, iCode * pi)
{
  operand *left, *result;
  sym_link *type, *etype;
  int p_type;

  D (emitcode (";", "genPointerGet"));

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  /* depending on the type of pointer we need to
     move it to the correct pointer register */
  type = operandType (left);
  etype = getSpec (type);
  /* if left is of type of pointer then it is simple */
  if (IS_PTR (type) && !IS_FUNC (type->next))
    {
      p_type = DCL_TYPE (type);
    }
  else
    {
      /* we have to go by the storage class */
      p_type = PTR_TYPE (SPEC_OCLS (etype));
    }

  /* special case when cast remat */
  if (p_type == GPOINTER && IS_SYMOP (left) && OP_SYMBOL (left)->remat && IS_CAST_ICODE (OP_SYMBOL (left)->rematiCode))
    {
      left = IC_RIGHT (OP_SYMBOL (left)->rematiCode);
      type = operandType (left);
      p_type = DCL_TYPE (type);
    }
  /* now that we have the pointer type we assign
     the pointer values */
  switch (p_type)
    {
    case POINTER:
    case IPOINTER:
      genNearPointerGet (left, result, ic, pi);
      break;

    case PPOINTER:
      genPagedPointerGet (left, result, ic, pi);
      break;

    case FPOINTER:
      genFarPointerGet (left, result, ic, pi);
      break;

    case CPOINTER:
      genCodePointerGet (left, result, ic, pi);
      break;

    case GPOINTER:
      genGenPointerGet (left, result, ic, pi);
      break;
    }
}


/*-----------------------------------------------------------------*/
/* genPackBits - generates code for packed bit storage             */
/*-----------------------------------------------------------------*/
static void
genPackBits (sym_link * etype, operand * right, const char *rname, int p_type)
{
  int offset = 0;               /* source byte offset */
  int rlen = 0;                 /* remaining bitfield length */
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */
  int litval;                   /* source literal value (if AOP_LIT) */
  unsigned char mask;           /* bitmask within current byte */

  D (emitcode (";", "genPackBits"));

  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  /* If the bitfield length is less than a byte */
  if (blen < 8)
    {
      mask = ((unsigned char) (0xFF << (blen + bstr)) | (unsigned char) (0xFF >> (8 - bstr)));

      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with a bitfield length <8 and literal source
           */
          litval = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          litval <<= bstr;
          litval &= (~mask) & 0xff;
          emitPtrByteGet (rname, p_type, FALSE);
          if ((mask | litval) != 0xff)
            emitcode ("anl", "a,#!constbyte", mask);
          if (litval)
            emitcode ("orl", "a,#!constbyte", litval);
        }
      else
        {
          if ((blen == 1) && (p_type != GPOINTER))
            {
              /* Case with a bitfield length == 1 and no generic pointer
               */
              if (AOP_TYPE (right) == AOP_CRY)
                emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
              else
                {
                  MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
                  emitcode ("rrc", "a");
                }
              emitPtrByteGet (rname, p_type, FALSE);
              emitcode ("mov", "acc[%d],c", bstr);
            }
          else
            {
              bool pushedB;
              /* Case with a bitfield length < 8 and arbitrary source
               */
              MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
              /* shift and mask source value */
              AccLsh (bstr);
              emitcode ("anl", "a,#!constbyte", (~mask) & 0xff);

              pushedB = pushB ();
              /* transfer A to B and get next byte */
              emitPtrByteGet (rname, p_type, TRUE);

              emitcode ("anl", "a,#!constbyte", mask);
              emitcode ("orl", "a,b");
              if (p_type == GPOINTER)
                emitpop ("b");

              popB (pushedB);
            }
        }

      emitPtrByteSet (rname, p_type, "a");
      return;
    }

  /* Bit length is greater than 7 bits. In this case, copy  */
  /* all except the partial byte at the end                 */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      emitPtrByteSet (rname, p_type, aopGet (right, offset++, FALSE, TRUE, NULL));
      if (rlen > 8)
        emitcode ("inc", "%s", rname);
    }

  /* If there was a partial byte at the end */
  if (rlen)
    {
      mask = (((unsigned char) - 1 << rlen) & 0xff);

      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with partial byte and literal source
           */
          litval = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          litval >>= (blen - rlen);
          litval &= (~mask) & 0xff;
          emitPtrByteGet (rname, p_type, FALSE);
          if ((mask | litval) != 0xff)
            emitcode ("anl", "a,#!constbyte", mask);
          if (litval)
            emitcode ("orl", "a,#!constbyte", litval);
        }
      else
        {
          bool pushedB;
          /* Case with partial byte and arbitrary source
           */
          MOVA (aopGet (right, offset++, FALSE, FALSE, NULL));
          emitcode ("anl", "a,#!constbyte", (~mask) & 0xff);

          pushedB = pushB ();
          /* transfer A to B and get next byte */
          emitPtrByteGet (rname, p_type, TRUE);

          emitcode ("anl", "a,#!constbyte", mask);
          emitcode ("orl", "a,b");
          if (p_type == GPOINTER)
            emitpop ("b");

          popB (pushedB);
        }
      emitPtrByteSet (rname, p_type, "a");
    }
}


/*-----------------------------------------------------------------*/
/* genDataPointerSet - remat pointer to data space                 */
/*-----------------------------------------------------------------*/
static void
genDataPointerSet (operand * right, operand * result, iCode * ic)
{
  int size, offset;
  char *l;

  D (emitcode (";", "genDataPointerSet"));

  aopOp (right, ic, FALSE, FALSE);

  size = max (AOP_SIZE (right), AOP_SIZE (result));
  //remove #
  l = Safe_strdup (aopGet (result, 0, FALSE, TRUE, NULL) + 1);
  for (offset = 0; offset < size; offset++)
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      if (offset)
        dbuf_printf (&dbuf, "(%s + %d)", l, offset);
      else
        dbuf_append_str (&dbuf, l);
      emitcode ("mov", "%s,%s", dbuf_c_str (&dbuf), aopGet (right, offset, FALSE, FALSE, NULL));
      dbuf_destroy (&dbuf);
    }
  Safe_free (l);

  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genNearPointerSet - emitcode for near pointer put               */
/*-----------------------------------------------------------------*/
static void
genNearPointerSet (operand * right, operand * result, iCode * ic, iCode * pi)
{
  asmop *aop = NULL;
  reg_info *preg = NULL;
  const char *rname;
  sym_link *retype, *letype;
  sym_link *ptype = operandType (result);

  D (emitcode (";", "genNearPointerSet"));

  retype = getSpec (operandType (right));
  letype = getSpec (ptype);

  aopOp (result, ic, FALSE, FALSE);

  /* if the result is rematerializable &
     in data space & not a bit variable */
  if (AOP_TYPE (result) == AOP_IMMD && DCL_TYPE (ptype) == POINTER && !IS_BITVAR (retype) && !IS_BITVAR (letype))
    {
      genDataPointerSet (right, result, ic);
      return;
    }

  /* if the value is already in a pointer register
     then don't need anything more */
  if (!AOP_INPREG (AOP (result)))
    {
      /* otherwise get a free pointer register */

      aop = newAsmop (0);
      preg = getFreePtr (ic, &aop, FALSE);
      emitcode ("mov", "%s,%s", preg->name, aopGet (result, 0, FALSE, TRUE, NULL));
      rname = preg->name;
    }
  else
    {
      rname = aopGet (result, 0, FALSE, FALSE, NULL);
    }

  aopOp (right, ic, FALSE, FALSE);

  rname = Safe_strdup (rname);
  /* if bitfield then unpack the bits */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    genPackBits ((IS_BITFIELD (retype) ? retype : letype), right, rname, POINTER);
  else
    {
      /* we can just get the values */
      int size = AOP_SIZE (right);
      int offset = 0;

      while (size--)
        {
          const char *l = aopGet (right, offset, FALSE, TRUE, NULL);
          if ((*l == '@') || (EQ (l, "acc")))
            {
              MOVA (l);
              emitcode ("mov", "@%s,a", rname);
            }
          else
            emitcode ("mov", "@%s,%s", rname, l);
          if (size || pi)
            emitcode ("inc", "%s", rname);
          offset++;
        }
    }

  /* now some housekeeping stuff */
  if (aop)                      /* we had to allocate for this iCode */
    {
      if (pi)
        aopPut (result, rname, 0);
      freeAsmop (NULL, aop, ic, TRUE);
    }
  else
    {
      /* we did not allocate which means left
         already in a pointer register, then
         if size > 0 && this could be used again
         we have to point it back to where it
         belongs */
      if ((AOP_SIZE (right) > 1 && !OP_SYMBOL (result)->remat && (OP_SYMBOL (result)->liveTo > ic->seq || ic->depth)) && !pi)
        {
          int size = AOP_SIZE (right) - 1;
          while (size--)
            emitcode ("dec", "%s", rname);
        }
    }
  Safe_free ((void *) rname);

  /* done */
  if (pi)
    pi->generated = 1;
  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genPagedPointerSet - emitcode for Paged pointer put             */
/*-----------------------------------------------------------------*/
static void
genPagedPointerSet (operand * right, operand * result, iCode * ic, iCode * pi)
{
  asmop *aop = NULL;
  reg_info *preg = NULL;
  const char *rname;
  sym_link *retype, *letype;

  D (emitcode (";", "genPagedPointerSet"));

  retype = getSpec (operandType (right));
  letype = getSpec (operandType (result));

  aopOp (result, ic, FALSE, FALSE);

  /* if the value is already in a pointer register
     then don't need anything more */
  if (!AOP_INPREG (AOP (result)))
    {
      /* otherwise get a free pointer register */

      aop = newAsmop (0);
      preg = getFreePtr (ic, &aop, FALSE);
      emitcode ("mov", "%s,%s", preg->name, aopGet (result, 0, FALSE, TRUE, NULL));
      rname = preg->name;
    }
  else
    {
      rname = aopGet (result, 0, FALSE, FALSE, NULL);
    }

  aopOp (right, ic, FALSE, FALSE);

  rname = Safe_strdup (rname);
  /* if bitfield then unpack the bits */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    genPackBits ((IS_BITFIELD (retype) ? retype : letype), right, rname, PPOINTER);
  else
    {
      /* we can just get the values */
      int size = AOP_SIZE (right);
      int offset = 0;

      while (size--)
        {
          MOVA (aopGet (right, offset, FALSE, TRUE, NULL));
          emitcode ("movx", "@%s,a", rname);
          if (size || pi)
            emitcode ("inc", "%s", rname);
          offset++;
        }
    }

  /* now some housekeeping stuff */
  if (aop)                      /* we had to allocate for this iCode */
    {
      if (pi)
        aopPut (result, rname, 0);
      freeAsmop (NULL, aop, ic, TRUE);
    }
  else
    {
      /* we did not allocate which means left
         already in a pointer register, then
         if size > 0 && this could be used again
         we have to point it back to where it
         belongs */
      if (AOP_SIZE (right) > 1 && !OP_SYMBOL (result)->remat && (OP_SYMBOL (result)->liveTo > ic->seq || ic->depth) && !pi)
        {
          int size = AOP_SIZE (right) - 1;
          while (size--)
            emitcode ("dec", "%s", rname);
        }
    }
  Safe_free ((void *) rname);

  /* done */
  if (pi)
    pi->generated = 1;
  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genFarPointerSet - set value from far space                     */
/*-----------------------------------------------------------------*/
static void
genFarPointerSet (operand * right, operand * result, iCode * ic, iCode * pi)
{
  int size, offset, dopi;
  sym_link *retype = getSpec (operandType (right));
  sym_link *letype = getSpec (operandType (result));

  D (emitcode (";", "genFarPointerSet"));

  aopOp (result, ic, FALSE, FALSE);
  dopi = loadDptrFromOperand (result, FALSE);

  /* so dptr now contains the address */
  aopOp (right, ic, FALSE, (AOP_INDPTRn (result) ? FALSE : TRUE));

  /* if bit then unpack */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    {
      if (AOP_INDPTRn (result))
        {
          genSetDPTR (AOP (result)->aopu.dptr);
        }
      genPackBits ((IS_BITFIELD (retype) ? retype : letype), right, "dptr", FPOINTER);
      if (AOP_INDPTRn (result))
        {
          genSetDPTR (0);
        }
    }
  else
    {
      size = AOP_SIZE (right);
      offset = 0;
      if (AOP_INDPTRn (result) && AOP_USESDPTR (right))
        {
          while (size--)
            {
              MOVA (aopGet (right, offset++, FALSE, FALSE, NULL));

              genSetDPTR (AOP (result)->aopu.dptr);
              emitcode ("movx", "@dptr,a");
              if (size || (dopi && pi && AOP_TYPE (result) != AOP_IMMD))
                emitcode ("inc", "dptr");
              genSetDPTR (0);
            }
        }
      else if (AOP_USESDPTR (result) && AOP_USESDPTR (right))
        {
          int i;
          _startLazyDPSEvaluation ();
          for (i = size - 1; i > 0; i--)
            emitcode ("push", aopGet (right, i, FALSE, FALSE, NULL));
          while (size--)
            {
              if (offset++)
                emitcode ("pop", "acc");
              else
                MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
              genSetDPTR (0);
              _flushLazyDPS ();
              emitcode ("movx", "@dptr,a");
              if (size || (dopi && pi && AOP_TYPE (result) != AOP_IMMD))
                emitcode ("inc", "dptr");
            }
          _endLazyDPSEvaluation ();
        }
      else
        {
          _startLazyDPSEvaluation ();
          while (size--)
            {
              MOVA (aopGet (right, offset++, FALSE, FALSE, NULL));

              if (AOP_INDPTRn (result))
                {
                  genSetDPTR (AOP (result)->aopu.dptr);
                }
              else
                {
                  genSetDPTR (0);
                }
              _flushLazyDPS ();

              emitcode ("movx", "@dptr,a");
              if (size || (dopi && pi && AOP_TYPE (result) != AOP_IMMD))
                emitcode ("inc", "dptr");
            }
          _endLazyDPSEvaluation ();
        }
    }

  if (dopi && pi && AOP_TYPE (result) != AOP_IMMD)
    {
      if (!AOP_INDPTRn (result))
        {
          _startLazyDPSEvaluation ();

          aopPut (result, "dpl", 0);
          aopPut (result, "dph", 1);
          if (options.model == MODEL_FLAT24)
            aopPut (result, "dpx", 2);

          _endLazyDPSEvaluation ();
        }
      pi->generated = 1;
    }
  else if (IS_SYMOP (result) &&
           (IS_OP_RUONLY (result) || AOP_INDPTRn (result)) &&
           (AOP_SIZE (right) > 1) &&
           (OP_SYMBOL (result)->liveTo > ic->seq || ic->depth))
    {
      size = AOP_SIZE (right) - 1;
      if (AOP_INDPTRn (result))
        {
          genSetDPTR (AOP (result)->aopu.dptr);
        }
      while (size--)
        emitcode ("lcall", "__decdptr");
      if (AOP_INDPTRn (result))
        {
          genSetDPTR (0);
        }
    }
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGenPointerSet - set value from generic pointer space         */
/*-----------------------------------------------------------------*/
static void
genGenPointerSet (operand * right, operand * result, iCode * ic, iCode * pi)
{
  int size, offset, dopi;
  bool pushedB;
  sym_link *retype = getSpec (operandType (right));
  sym_link *letype = getSpec (operandType (result));

  D (emitcode (";", "genGenPointerSet"));

  aopOp (result, ic, FALSE, FALSE);
  pushedB = pushB ();
  dopi = loadDptrFromOperand (result, TRUE);

  /* so dptr-b now contains the address */
  aopOp (right, ic, FALSE, (AOP_INDPTRn (result) ? FALSE : TRUE));

  /* if bit then unpack */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    {
      genPackBits ((IS_BITFIELD (retype) ? retype : letype), right, "dptr", GPOINTER);
    }
  else
    {
      size = AOP_SIZE (right);
      offset = 0;

      _startLazyDPSEvaluation ();
      while (size--)
        {
          if (size)
            {
              // Set two bytes at a time, passed in AP & A.
              // dptr will be incremented ONCE by __gptrputWord.
              //
              // Note: any change here must be coordinated
              // with the implementation of __gptrputWord
              // in device/lib/_gptrput.c
              emitcode ("mov", "acc1, %s", aopGet (right, offset++, FALSE, FALSE, NULL));
              MOVA (aopGet (right, offset++, FALSE, FALSE, NULL));

              genSetDPTR (0);
              _flushLazyDPS ();
              emitcode ("lcall", "__gptrputWord");
              size--;
            }
          else
            {
              // Only one byte to put.
              MOVA (aopGet (right, offset++, FALSE, FALSE, NULL));

              genSetDPTR (0);
              _flushLazyDPS ();
              emitcode ("lcall", "__gptrput");
            }

          if (size || (dopi && pi && AOP_TYPE (result) != AOP_IMMD))
            {
              emitcode ("inc", "dptr");
            }
        }
      _endLazyDPSEvaluation ();
    }

  if (dopi && pi && AOP_TYPE (result) != AOP_IMMD)
    {
      _startLazyDPSEvaluation ();

      aopPut (result, "dpl", 0);
      aopPut (result, "dph", 1);
      if (options.model == MODEL_FLAT24)
        {
          aopPut (result, "dpx", 2);
          aopPut (result, "b", 3);
        }
      else
        {
          aopPut (result, "b", 2);
        }
      _endLazyDPSEvaluation ();

      pi->generated = 1;
    }
  else if (IS_SYMOP (result) && IS_OP_RUONLY (result) && AOP_SIZE (right) > 1 &&
           (OP_SYMBOL (result)->liveTo > ic->seq || ic->depth))
    {
      size = AOP_SIZE (right) - 1;
      while (size--)
        emitcode ("lcall", "__decdptr");
    }
  popB (pushedB);

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genPointerSet - stores the value into a pointer location        */
/*-----------------------------------------------------------------*/
static void
genPointerSet (iCode * ic, iCode * pi)
{
  operand *right, *result;
  sym_link *type, *etype;
  int p_type;

  D (emitcode (";", "genPointerSet"));

  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);

  /* depending on the type of pointer we need to
     move it to the correct pointer register */
  type = operandType (result);
  etype = getSpec (type);
  /* if left is of type of pointer then it is simple */
  if (IS_PTR (type) && !IS_FUNC (type->next))
    {
      p_type = DCL_TYPE (type);
    }
  else
    {
      /* we have to go by the storage class */
      p_type = PTR_TYPE (SPEC_OCLS (etype));
    }

  /* special case when cast remat */
  if (p_type == GPOINTER && IS_SYMOP (result) && OP_SYMBOL (result)->remat && IS_CAST_ICODE (OP_SYMBOL (result)->rematiCode))
    {
      result = IC_RIGHT (OP_SYMBOL (result)->rematiCode);
      type = operandType (result);
      p_type = DCL_TYPE (type);
    }

  /* now that we have the pointer type we assign
     the pointer values */
  switch (p_type)
    {
    case POINTER:
    case IPOINTER:
      genNearPointerSet (right, result, ic, pi);
      break;

    case PPOINTER:
      genPagedPointerSet (right, result, ic, pi);
      break;

    case FPOINTER:
      genFarPointerSet (right, result, ic, pi);
      break;

    case GPOINTER:
      genGenPointerSet (right, result, ic, pi);
      break;

    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "genPointerSet: illegal pointer type");
    }
}

/*-----------------------------------------------------------------*/
/* genIfx - generate code for Ifx statement                        */
/*-----------------------------------------------------------------*/
static void
genIfx (iCode * ic, iCode * popIc)
{
  operand *cond = IC_COND (ic);
  int isbit = 0;
  char *dup = NULL;

  D (emitcode (";", "genIfx"));

  aopOp (cond, ic, FALSE, FALSE);

  /* get the value into acc */
  if (AOP_TYPE (cond) != AOP_CRY)
    {
      toBoolean (cond);
    }
  else
    {
      isbit = 1;
      if (AOP (cond)->aopu.aop_dir)
        /* TODO: borutr: is really necessary to strdup it? */
        dup = Safe_strdup (AOP (cond)->aopu.aop_dir);
    }

  /* the result is now in the accumulator or a directly addressable bit */
  freeAsmop (cond, NULL, ic, TRUE);

  /* if the condition is a bit variable */
  if (isbit && dup)
    genIfxJump (ic, dup, popIc);
  else if (isbit && IS_ITEMP (cond) && SPIL_LOC (cond))
    genIfxJump (ic, SPIL_LOC (cond)->rname, popIc);
  else if (isbit && !IS_ITEMP (cond))
    genIfxJump (ic, OP_SYMBOL (cond)->rname, popIc);
  else
    genIfxJump (ic, "a", popIc);

  if (dup)
    Safe_free (dup);

  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void
genAddrOf (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  int size, offset;
  bool pushedA = FALSE;

  D (emitcode (";", "genAddrOf"));

  aopOp (IC_RESULT (ic), ic, FALSE, FALSE);

  /* if the operand is on the stack then we
     need to get the stack offset of this
     variable */
  if (sym->onStack)
    {
      /* if 10 bit stack */
      if (options.stack10bit)
        {
          struct dbuf_s dbuf;
          int offset;

          dbuf_init (&dbuf, 128);
          dbuf_tprintf (&dbuf, "#!constbyte", (options.stack_loc >> 16) & 0xff);
          /* if it has an offset then we need to compute it */
          /*        emitcode ("subb", "a,#!constbyte", */
          /*                  -((sym->stack < 0) ? */
          /*                    ((short) (sym->stack - _G.nRegsSaved)) : */
          /*                    ((short) sym->stack)) & 0xff); */
          /*        emitcode ("mov","b,a"); */
          /*        emitcode ("mov","a,#!constbyte",(-((sym->stack < 0) ? */
          /*                                       ((short) (sym->stack - _G.nRegsSaved)) : */
          /*                                       ((short) sym->stack)) >> 8) & 0xff); */
          if (sym->stack)
            {
              emitcode ("mov", "a,_bpx");
              emitcode ("add", "a,#!constbyte", ((sym->stack < 0) ?
                                                 ((char) (sym->stack - _G.nRegsSaved)) : ((char) sym->stack)) & 0xff);
              emitcode ("mov", "b,a");
              emitcode ("mov", "a,_bpx+1");

              offset = (((sym->stack < 0) ? ((short) (sym->stack - _G.nRegsSaved)) : ((short) sym->stack)) >> 8) & 0xff;

              emitcode ("addc", "a,#!constbyte", offset);

              if (aopPutUsesAcc (IC_RESULT (ic), "b", 0))
                {
                  emitpush ("acc");
                  pushedA = TRUE;
                }
              aopPut (IC_RESULT (ic), "b", 0);
              if (pushedA)
                emitpop ("acc");
              aopPut (IC_RESULT (ic), "a", 1);
              aopPut (IC_RESULT (ic), dbuf_c_str (&dbuf), 2);
            }
          else
            {
              /* we can just move _bp */
              aopPut (IC_RESULT (ic), "_bpx", 0);
              aopPut (IC_RESULT (ic), "_bpx+1", 1);
              aopPut (IC_RESULT (ic), dbuf_c_str (&dbuf), 2);
            }
          dbuf_destroy (&dbuf);
        }
      else
        {
          /* if it has an offset then we need to compute it */
          if (sym->stack)
            {
              emitcode ("mov", "a,_bp");
              emitcode ("add", "a,#!constbyte", ((char) sym->stack & 0xff));
              aopPut (IC_RESULT (ic), "a", 0);
            }
          else
            {
              /* we can just move _bp */
              aopPut (IC_RESULT (ic), "_bp", 0);
            }
          /* fill the result with zero */
          size = AOP_SIZE (IC_RESULT (ic)) - 1;


          if (options.stack10bit && size < (FARPTRSIZE - 1))
            {
              fprintf (stderr, "*** warning: pointer to stack var truncated.\n");
            }

          offset = 1;
          while (size--)
            {
              aopPut (IC_RESULT (ic), zero, offset++);
            }
        }
      goto release;
    }

  /* object not on stack then we need the name */
  size = getDataSize (IC_RESULT (ic));
  offset = 0;

  while (size--)
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      if (offset)
        {
          switch (offset)
            {
            case 1:
              dbuf_tprintf (&dbuf, "#!his", sym->rname);
              break;
            case 2:
              dbuf_tprintf (&dbuf, "#!hihis", sym->rname);
              break;
            case 3:
              dbuf_tprintf (&dbuf, "#!hihihis", sym->rname);
              break;
            default:           /* should not need this (just in case) */
              dbuf_printf (&dbuf, "#(%s >> %d)", sym->rname, offset * 8);
            }
        }
      else
        {
          dbuf_printf (&dbuf, "#%s", sym->rname);
        }
      aopPut (IC_RESULT (ic), dbuf_c_str (&dbuf), offset++);
      dbuf_destroy (&dbuf);
    }
  if (opIsGptr (IC_RESULT (ic)))
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      dbuf_printf (&dbuf, "#0x%02x", pointerTypeToGPByte (pointerCode (getSpec (operandType (IC_LEFT (ic)))), NULL, NULL));
      aopPut (IC_RESULT (ic), dbuf_c_str (&dbuf), GPTRSIZE - 1);
      dbuf_destroy (&dbuf);
    }

release:
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genFarFarAssign - assignment when both are in far space         */
/*-----------------------------------------------------------------*/
static void
genFarFarAssign (operand * result, operand * right, iCode * ic)
{
  int size = AOP_SIZE (right);
  int offset = 0;
  symbol *rSym = NULL;

  if (size == 1)
    {
      /* quick & easy case. */
      D (emitcode (";", "genFarFarAssign (1 byte case)"));
      MOVA (aopGet (right, 0, FALSE, FALSE, NULL));
      freeAsmop (right, NULL, ic, FALSE);
      /* now assign DPTR to result */
      _G.accInUse++;
      aopOp (result, ic, FALSE, FALSE);
      _G.accInUse--;
      aopPut (result, "a", 0);
      freeAsmop (result, NULL, ic, FALSE);
      return;
    }

  /* See if we've got an underlying symbol to abuse. */
  if (IS_SYMOP (result) && OP_SYMBOL (result))
    {
      if (IS_TRUE_SYMOP (result))
        {
          rSym = OP_SYMBOL (result);
        }
      else if (IS_ITEMP (result) && OP_SYMBOL (result)->isspilt && OP_SYMBOL (result)->usl.spillLoc)
        {
          rSym = OP_SYMBOL (result)->usl.spillLoc;
        }
    }

  if (size > 1 && rSym && rSym->rname && !rSym->onStack && !IS_OP_RUONLY (right))
    {
      /* We can use the '390 auto-toggle feature to good effect here. */

      D (emitcode (";", "genFarFarAssign (390 auto-toggle fun)"));
      emitcode ("mov", "dps,#!constbyte", 0x21);        /* Select DPTR2 & auto-toggle. */
      emitcode ("mov", "dptr,#%s", rSym->rname);
      /* DP2 = result, DP1 = right, DP1 is current. */
      while (size)
        {
          if (AOP (right)->code)
            {
              emitcode ("clr", "a");
              emitcode ("movc", "a,@a+dptr");
            }
          else
            {
              emitcode ("movx", "a,@dptr");
            }
          emitcode ("movx", "@dptr,a");
          if (--size)
            {
              emitcode ("inc", "dptr");
              emitcode ("inc", "dptr");
            }
        }
      emitcode ("mov", "dps,#0");
      freeAsmop (right, NULL, ic, FALSE);
#if 0
      some alternative code for processors without auto - toggle
      no time to test now, so later well put in ... kpb D (emitcode (";", "genFarFarAssign (dual-dptr fun)"));
      emitcode ("mov", "dps,#1");       /* Select DPTR2. */
      emitcode ("mov", "dptr,#%s", rSym->rname);
      /* DP2 = result, DP1 = right, DP1 is current. */
      while (size)
        {
          --size;
          emitcode ("movx", "a,@dptr");
          if (size)
            emitcode ("inc", "dptr");
          emitcode ("inc", "dps");
          emitcode ("movx", "@dptr,a");
          if (size)
            emitcode ("inc", "dptr");
          emitcode ("inc", "dps");
        }
      emitcode ("mov", "dps,#0");
      freeAsmop (right, NULL, ic, FALSE);
#endif
    }
  else
    {
      D (emitcode (";", "genFarFarAssign"));
      aopOp (result, ic, TRUE, TRUE);

      _startLazyDPSEvaluation ();

      while (size--)
        {
          aopPut (result, aopGet (right, offset, FALSE, FALSE, NULL), offset);
          offset++;
        }
      _endLazyDPSEvaluation ();
      freeAsmop (result, NULL, ic, FALSE);
      freeAsmop (right, NULL, ic, FALSE);
    }
}

/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void
genAssign (iCode * ic)
{
  operand *result, *right;
  int size, offset;
  unsigned long long lit = 0ull;

  D (emitcode (";", "genAssign"));

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);

  /* if they are the same */
  if (operandsEqu (result, right) && !isOperandVolatile (result, FALSE) && !isOperandVolatile (right, FALSE))
    return;

  /* if both are ruonly */
  if (IS_OP_RUONLY (right) && IS_OP_RUONLY (result))
    return;

  aopOp (right, ic, FALSE, IS_OP_RUONLY (result));

  emitcode (";", "genAssign: resultIsFar = %s", isOperandInFarSpace (result) ? "TRUE" : "FALSE");

  /* special case both in far space */
  if ((AOP_TYPE (right) == AOP_DPTR || AOP_TYPE (right) == AOP_DPTR2 || IS_OP_RUONLY (right)) &&
      /* IS_TRUE_SYMOP(result)       && */
      isOperandInFarSpace (result))
    {
      genFarFarAssign (result, right, ic);
      return;
    }

  aopOp (result, ic, TRUE, FALSE);

  /* if they are the same registers */
  if (sameRegs (AOP (right), AOP (result)) && !isOperandVolatile (result, FALSE) && !isOperandVolatile (right, FALSE))
    goto release;

  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)     /* works only for true symbols */
    {
      /* if the right size is a literal then
         we know what the value is */
      if (AOP_TYPE (right) == AOP_LIT)
        {
          if (((int) operandLitValue (right)))
            aopPut (result, one, 0);
          else
            aopPut (result, zero, 0);
          goto release;
        }

      /* the right is also a bit variable */
      if (AOP_TYPE (right) == AOP_CRY)
        {
          emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
          aopPut (result, "c", 0);
          goto release;
        }

      /* we need to or */
      toBoolean (right);
      aopPut (result, "a", 0);
      goto release;
    }

  /* bit variables done */
  /* general case */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      if (!IS_FLOAT (operandType (right)))
        {
          lit = ullFromVal (AOP (right)->aopu.aop_lit);
        }
      else
        {
          union
          {
            float f;
            unsigned char c[4];
          } fl;

          fl.f = (float) floatFromVal (AOP (right)->aopu.aop_lit);
#ifdef WORDS_BIGENDIAN
          lit = (fl.c[3] << 0) | (fl.c[2] << 8) | (fl.c[1] << 16) | (fl.c[0] << 24);
#else
          lit = (fl.c[0] << 0) | (fl.c[1] << 8) | (fl.c[2] << 16) | (fl.c[3] << 24);
#endif
        }
    }

  size = getDataSize (result);
  offset = 0;

  if ((size > 1) && (AOP_TYPE (result) != AOP_REG) && (AOP_TYPE (right) == AOP_LIT))
    {
      _startLazyDPSEvaluation ();
      while (size && ((unsigned long long) (lit >> (offset * 8)) != 0))
        {
          aopPut (result, aopGet (right, offset, FALSE, FALSE, NULL), offset);
          offset++;
          size--;
        }
      /* And now fill the rest with zeros. */
      if (size)
        {
          emitcode ("clr", "a");
        }
      while (size--)
        {
          aopPut (result, "a", offset++);
        }
      _endLazyDPSEvaluation ();
    }
  else
    {
      _startLazyDPSEvaluation ();
      while (size--)
        {
          aopPut (result, aopGet (right, offset, FALSE, FALSE, NULL), offset);
          offset++;
        }
      _endLazyDPSEvaluation ();
    }
  adjustArithmeticResult (ic);

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genJumpTab - generates code for jump table                      */
/*-----------------------------------------------------------------*/
static void
genJumpTab (iCode * ic)
{
  symbol *jtab;

  D (emitcode (";", "genJumpTab"));

  aopOp (IC_JTCOND (ic), ic, FALSE, FALSE);
  /* get the condition into accumulator */
  MOVA (aopGet (IC_JTCOND (ic), 0, FALSE, FALSE, NULL));
  /* multiply by four! */
  emitcode ("add", "a,acc");
  emitcode ("add", "a,acc");
  freeAsmop (IC_JTCOND (ic), NULL, ic, TRUE);

  jtab = newiTempLabel (NULL);
  emitcode ("mov", "dptr,#!tlabel", labelKey2num (jtab->key));
  emitcode ("jmp", "@a+dptr");
  emitLabel (jtab);
  /* now generate the jump labels */
  for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
    emitcode ("ljmp", "!tlabel", labelKey2num (jtab->key));
}

/*-----------------------------------------------------------------*/
/* genCast - gen code for casting                                  */
/*-----------------------------------------------------------------*/
static void
genCast (iCode * ic)
{
  operand *result = IC_RESULT (ic);
  sym_link *ctype = operandType (IC_LEFT (ic));
  sym_link *rtype = operandType (IC_RIGHT (ic));
  operand *right = IC_RIGHT (ic);
  int size, offset;

  D (emitcode (";", "genCast"));

  /* if they are equivalent then do nothing */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  /* if casting to <= size and both ruonly then do nothing */
  if (IS_OP_RUONLY (right) && IS_OP_RUONLY (result))
    return;

  aopOp (right, ic, FALSE, IS_OP_RUONLY (result));
  aopOp (result, ic, FALSE, (AOP_TYPE (right) == AOP_DPTR));

  /* if the result is a bit (and not a bitfield) */
  if (IS_BOOLEAN (OP_SYMBOL (result)->type))
    {
      /* if the right size is a literal then
         we know what the value is */
      if (AOP_TYPE (right) == AOP_LIT)
        {
          if (((int) operandLitValue (right)))
            aopPut (result, one, 0);
          else
            aopPut (result, zero, 0);

          goto release;
        }

      /* the right is also a bit variable */
      if (AOP_TYPE (right) == AOP_CRY)
        {
          emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
          aopPut (result, "c", 0);
          goto release;
        }

      /* we need to or */
      toCarry (right);
      outBitC (result);
      goto release;
    }

  /* if they are the same size : or less */
  if (AOP_SIZE (result) <= AOP_SIZE (right) && !IS_BOOLEAN (operandType (result)))
    {
      /* if they are in the same place */
      if (sameRegs (AOP (right), AOP (result)))
        goto release;

      /* if they in different places then copy */
      size = AOP_SIZE (result);
      offset = 0;
      _startLazyDPSEvaluation ();
      while (size--)
        {
          aopPut (result, aopGet (right, offset, FALSE, FALSE, NULL), offset);
          offset++;
        }
      _endLazyDPSEvaluation ();
      goto release;
    }

  /* if the result is of type pointer */
  if (IS_PTR (ctype) && !IS_INTEGRAL (rtype))
    {
      int p_type;
      sym_link *type = operandType (right);

      /* pointer to generic pointer */
      if (IS_GENPTR (ctype))
        {
          if (IS_PTR (type) || IS_FUNC (type))
            {
              p_type = DCL_TYPE (type);
            }
          else
            {
#if OLD_CAST_BEHAVIOR
              /* KV: we are converting a non-pointer type to
               * a generic pointer. This (ifdef'd out) code
               * says that the resulting generic pointer
               * should have the same class as the storage
               * location of the non-pointer variable.
               *
               * For example, converting an int (which happens
               * to be stored in DATA space) to a pointer results
               * in a DATA generic pointer; if the original int
               * in XDATA space, so will be the resulting pointer.
               *
               * I don't like that behavior, and thus this change:
               * all such conversions will be forced to XDATA and
               * throw a warning. If you want some non-XDATA
               * type, or you want to suppress the warning, you
               * must go through an intermediate cast, like so:
               *
               * char _generic *gp = (char _xdata *)(intVar);
               */
              sym_link *etype = getSpec (type);

              /* we have to go by the storage class */
              if (SPEC_OCLS (etype) != generic)
                {
                  p_type = PTR_TYPE (SPEC_OCLS (etype));
                }
              else
#endif
                {
                  /* Converting unknown class (i.e. register variable)
                   * to generic pointer. This is not good, but
                   * we'll make a guess (and throw a warning).
                   */
                  p_type = FPOINTER;
                  werror (W_INT_TO_GEN_PTR_CAST);
                }
            }

          /* the first two bytes are known */
          size = GPTRSIZE - 1;
          offset = 0;
          _startLazyDPSEvaluation ();
          while (size--)
            {
              aopPut (result, aopGet (right, offset, FALSE, FALSE, NULL), offset);
              offset++;
            }
          _endLazyDPSEvaluation ();

          /* the last byte depending on type */
          {
            int gpVal = pointerTypeToGPByte (p_type, NULL, NULL);
            char gpValStr[10];

            if (gpVal == -1)
              {
                // pointerTypeToGPByte will have warned, just copy.
                aopPut (result, aopGet (right, offset, FALSE, FALSE, NULL), offset);
              }
            else
              {
                SNPRINTF (gpValStr, sizeof (gpValStr), "#0x%02x", gpVal);
                aopPut (result, gpValStr, GPTRSIZE - 1);
              }
          }
          goto release;
        }

      /* just copy the pointers */
      size = AOP_SIZE (result);
      offset = 0;
      _startLazyDPSEvaluation ();
      while (size--)
        {
          aopPut (result, aopGet (right, offset, FALSE, FALSE, NULL), offset);
          offset++;
        }
      _endLazyDPSEvaluation ();
      goto release;
    }

  /* so we now know that the size of destination is greater
     than the size of the source */
  /* we move to result for the size of source */
  size = AOP_SIZE (right);
  offset = 0;
  _startLazyDPSEvaluation ();
  while (size--)
    {
      aopPut (result, aopGet (right, offset, FALSE, FALSE, NULL), offset);
      offset++;
    }
  _endLazyDPSEvaluation ();

  /* now depending on the sign of the source && destination */
  size = AOP_SIZE (result) - AOP_SIZE (right);
  /* if unsigned or not an integral type */
  /* also, if the source is a bit, we don't need to sign extend, because
   * it can't possibly have set the sign bit.
   */
  if (!IS_SPEC (rtype) || SPEC_USIGN (rtype) || AOP_TYPE (right) == AOP_CRY)
    {
      while (size--)
        {
          aopPut (result, zero, offset++);
        }
    }
  else
    {
      /* we need to extend the sign :{ */
      MOVA (aopGet (right, AOP_SIZE (right) - 1, FALSE, FALSE, NULL));
      emitcode ("rlc", "a");
      emitcode ("subb", "a,acc");
      while (size--)
        aopPut (result, "a", offset++);
    }

  /* we are done hurray !!!! */

release:
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genMemcpyX2X - gen code for memcpy xdata to xdata               */
/*-----------------------------------------------------------------*/
static void
genMemcpyX2X (iCode * ic, int nparms, operand ** parms, int fromc)
{
  operand *from, *to, *count;
  symbol *lbl;
  bitVect *rsave;
  int i;

  /* we know it has to be 3 parameters */
  assert (nparms == 3);

  rsave = newBitVect (16);
  /* save DPTR if it needs to be saved */
  for (i = DPL_IDX; i <= B_IDX; i++)
    {
      if (bitVectBitValue (ic->rMask, i))
        rsave = bitVectSetBit (rsave, i);
    }
  rsave = bitVectIntersect (rsave, bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));
  savermask (rsave);

  to = parms[0];
  from = parms[1];
  count = parms[2];

  aopOp (from, ic->next, FALSE, FALSE);

  /* get from into DPTR1 */
  emitcode ("mov", "dpl1,%s", aopGet (from, 0, FALSE, FALSE, NULL));
  emitcode ("mov", "dph1,%s", aopGet (from, 1, FALSE, FALSE, NULL));
  if (options.model == MODEL_FLAT24)
    {
      emitcode ("mov", "dpx1,%s", aopGet (from, 2, FALSE, FALSE, NULL));
    }

  freeAsmop (from, NULL, ic, FALSE);
  aopOp (to, ic, FALSE, FALSE);
  /* get "to" into DPTR */
  /* if the operand is already in dptr
     then we do nothing else we move the value to dptr */
  if (AOP_TYPE (to) != AOP_STR)
    {
      /* if already in DPTR then we need to push */
      if (AOP_TYPE (to) == AOP_DPTR)
        {
          emitcode ("push", "%s", aopGet (to, 0, FALSE, TRUE, NULL));
          emitcode ("push", "%s", aopGet (to, 1, FALSE, TRUE, NULL));
          if (options.model == MODEL_FLAT24)
            emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
          emitcode ("pop", "dph");
          emitcode ("pop", "dpl");
        }
      else
        {
          _startLazyDPSEvaluation ();
          /* if this is remateriazable */
          if (AOP_TYPE (to) == AOP_IMMD)
            {
              emitcode ("mov", "dptr,%s", aopGet (to, 0, TRUE, FALSE, NULL));
            }
          else
            {
              /* we need to get it byte by byte */
              emitcode ("mov", "dpl,%s", aopGet (to, 0, FALSE, FALSE, NULL));
              emitcode ("mov", "dph,%s", aopGet (to, 1, FALSE, FALSE, NULL));
              if (options.model == MODEL_FLAT24)
                {
                  emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
                }
            }
          _endLazyDPSEvaluation ();
        }
    }
  freeAsmop (to, NULL, ic, FALSE);
  _G.dptrInUse = _G.dptr1InUse = 1;
  aopOp (count, ic->next->next, FALSE, FALSE);
  lbl = newiTempLabel (NULL);

  /* now for the actual copy */
  if (AOP_TYPE (count) == AOP_LIT && (int) ulFromVal (AOP (count)->aopu.aop_lit) <= 256)
    {
      emitcode ("mov", "b,%s", aopGet (count, 0, FALSE, FALSE, NULL));
      if (fromc)
        {
          emitcode ("lcall", "__bi_memcpyc2x_s");
        }
      else
        {
          emitcode ("lcall", "__bi_memcpyx2x_s");
        }
      freeAsmop (count, NULL, ic, FALSE);
    }
  else
    {
      symbol *lbl1 = newiTempLabel (NULL);

      emitcode (";", " Auto increment but no djnz");
      emitcode ("mov", "acc1,%s", aopGet (count, 0, FALSE, TRUE, NULL));
      emitcode ("mov", "b,%s", aopGet (count, 1, FALSE, TRUE, NULL));
      freeAsmop (count, NULL, ic, FALSE);
      emitcode ("mov", "dps,#!constbyte", 0x21);        /* Select DPTR2 & auto-toggle. */
      emitLabel (lbl);
      if (fromc)
        {
          emitcode ("clr", "a");
          emitcode ("movc", "a,@a+dptr");
        }
      else
        emitcode ("movx", "a,@dptr");
      emitcode ("movx", "@dptr,a");
      emitcode ("inc", "dptr");
      emitcode ("inc", "dptr");
      emitcode ("mov", "a,b");
      emitcode ("orl", "a,acc1");
      emitcode ("jz", "!tlabel", labelKey2num (lbl1->key));
      emitcode ("mov", "a,acc1");
      emitcode ("add", "a,#!constbyte", 0xFF);
      emitcode ("mov", "acc1,a");
      emitcode ("mov", "a,b");
      emitcode ("addc", "a,#!constbyte", 0xFF);
      emitcode ("mov", "b,a");
      emitcode ("sjmp", "!tlabel", labelKey2num (lbl->key));
      emitLabel (lbl1);
    }
  emitcode ("mov", "dps,#0");
  _G.dptrInUse = _G.dptr1InUse = 0;
  unsavermask (rsave);

}

/*-----------------------------------------------------------------*/
/* genMemcmpX2X - gen code for memcmp xdata to xdata               */
/*-----------------------------------------------------------------*/
static void
genMemcmpX2X (iCode * ic, int nparms, operand ** parms, int fromc)
{
  operand *from, *to, *count;
  symbol *lbl, *lbl2;
  bitVect *rsave;
  int i;

  /* we know it has to be 3 parameters */
  assert (nparms == 3);

  rsave = newBitVect (16);
  /* save DPTR if it needs to be saved */
  for (i = DPL_IDX; i <= B_IDX; i++)
    {
      if (bitVectBitValue (ic->rMask, i))
        rsave = bitVectSetBit (rsave, i);
    }
  rsave = bitVectIntersect (rsave, bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));
  savermask (rsave);

  to = parms[0];
  from = parms[1];
  count = parms[2];

  aopOp (from, ic->next, FALSE, FALSE);

  /* get from into DPTR1 */
  emitcode ("mov", "dpl1,%s", aopGet (from, 0, FALSE, FALSE, NULL));
  emitcode ("mov", "dph1,%s", aopGet (from, 1, FALSE, FALSE, NULL));
  if (options.model == MODEL_FLAT24)
    {
      emitcode ("mov", "dpx1,%s", aopGet (from, 2, FALSE, FALSE, NULL));
    }

  freeAsmop (from, NULL, ic, FALSE);
  aopOp (to, ic, FALSE, FALSE);
  /* get "to" into DPTR */
  /* if the operand is already in dptr
     then we do nothing else we move the value to dptr */
  if (AOP_TYPE (to) != AOP_STR)
    {
      /* if already in DPTR then we need to push */
      if (AOP_TYPE (to) == AOP_DPTR)
        {
          emitcode ("push", "%s", aopGet (to, 0, FALSE, TRUE, NULL));
          emitcode ("push", "%s", aopGet (to, 1, FALSE, TRUE, NULL));
          if (options.model == MODEL_FLAT24)
            emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
          emitcode ("pop", "dph");
          emitcode ("pop", "dpl");
        }
      else
        {
          _startLazyDPSEvaluation ();
          /* if this is remateriazable */
          if (AOP_TYPE (to) == AOP_IMMD)
            {
              emitcode ("mov", "dptr,%s", aopGet (to, 0, TRUE, FALSE, NULL));
            }
          else
            {
              /* we need to get it byte by byte */
              emitcode ("mov", "dpl,%s", aopGet (to, 0, FALSE, FALSE, NULL));
              emitcode ("mov", "dph,%s", aopGet (to, 1, FALSE, FALSE, NULL));
              if (options.model == MODEL_FLAT24)
                {
                  emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
                }
            }
          _endLazyDPSEvaluation ();
        }
    }
  freeAsmop (to, NULL, ic, FALSE);
  _G.dptrInUse = _G.dptr1InUse = 1;
  aopOp (count, ic->next->next, FALSE, FALSE);
  lbl = newiTempLabel (NULL);
  lbl2 = newiTempLabel (NULL);

  /* now for the actual compare */
  if (AOP_TYPE (count) == AOP_LIT && (int) ulFromVal (AOP (count)->aopu.aop_lit) <= 256)
    {
      emitcode ("mov", "b,%s", aopGet (count, 0, FALSE, FALSE, NULL));
      if (fromc)
        emitcode ("lcall", "__bi_memcmpc2x_s");
      else
        emitcode ("lcall", "__bi_memcmpx2x_s");
      freeAsmop (count, NULL, ic, FALSE);
      aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
      aopPut (IC_RESULT (ic), "a", 0);
      freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
    }
  else
    {
      symbol *lbl1 = newiTempLabel (NULL);

      emitcode ("push", "ar0");
      emitcode (";", " Auto increment but no djnz");
      emitcode ("mov", "acc1,%s", aopGet (count, 0, FALSE, TRUE, NULL));
      emitcode ("mov", "b,%s", aopGet (count, 1, FALSE, TRUE, NULL));
      freeAsmop (count, NULL, ic, FALSE);
      emitcode ("mov", "dps,#!constbyte", 0x21);        /* Select DPTR2 & auto-toggle. */
      emitLabel (lbl);
      if (fromc)
        {
          emitcode ("clr", "a");
          emitcode ("movc", "a,@a+dptr");
        }
      else
        emitcode ("movx", "a,@dptr");
      emitcode ("mov", "r0,a");
      emitcode ("movx", "a,@dptr");
      emitcode ("clr", "c");
      emitcode ("subb", "a,r0");
      emitcode ("jnz", "!tlabel", labelKey2num (lbl2->key));
      emitcode ("inc", "dptr");
      emitcode ("inc", "dptr");
      emitcode ("mov", "a,b");
      emitcode ("orl", "a,acc1");
      emitcode ("jz", "!tlabel", labelKey2num (lbl1->key));
      emitcode ("mov", "a,acc1");
      emitcode ("add", "a,#!constbyte", 0xFF);
      emitcode ("mov", "acc1,a");
      emitcode ("mov", "a,b");
      emitcode ("addc", "a,#!constbyte", 0xFF);
      emitcode ("mov", "b,a");
      emitcode ("sjmp", "!tlabel", labelKey2num (lbl->key));
      emitLabel (lbl1);
      emitcode ("clr", "a");
      emitLabel (lbl2);
      aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
      aopPut (IC_RESULT (ic), "a", 0);
      freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
      emitcode ("pop", "ar0");
      emitcode ("mov", "dps,#0");
    }
  _G.dptrInUse = _G.dptr1InUse = 0;
  unsavermask (rsave);

}

/*-----------------------------------------------------------------*/
/* genInp - gen code for __builtin_inp read data from a mem mapped */
/* port, first parameter output area second parameter pointer to   */
/* port third parameter count                                      */
/*-----------------------------------------------------------------*/
static void
genInp (iCode * ic, int nparms, operand ** parms)
{
  operand *from, *to, *count;
  symbol *lbl;
  bitVect *rsave;
  int i;

  /* we know it has to be 3 parameters */
  assert (nparms == 3);

  rsave = newBitVect (16);
  /* save DPTR if it needs to be saved */
  for (i = DPL_IDX; i <= B_IDX; i++)
    {
      if (bitVectBitValue (ic->rMask, i))
        rsave = bitVectSetBit (rsave, i);
    }
  rsave = bitVectIntersect (rsave, bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));
  savermask (rsave);

  to = parms[0];
  from = parms[1];
  count = parms[2];

  aopOp (from, ic->next, FALSE, FALSE);

  /* get from into DPTR1 */
  emitcode ("mov", "dpl1,%s", aopGet (from, 0, FALSE, FALSE, NULL));
  emitcode ("mov", "dph1,%s", aopGet (from, 1, FALSE, FALSE, NULL));
  if (options.model == MODEL_FLAT24)
    {
      emitcode ("mov", "dpx1,%s", aopGet (from, 2, FALSE, FALSE, NULL));
    }

  freeAsmop (from, NULL, ic, FALSE);
  aopOp (to, ic, FALSE, FALSE);
  /* get "to" into DPTR */
  /* if the operand is already in dptr
     then we do nothing else we move the value to dptr */
  if (AOP_TYPE (to) != AOP_STR)
    {
      /* if already in DPTR then we need to push */
      if (AOP_TYPE (to) == AOP_DPTR)
        {
          emitcode ("push", "%s", aopGet (to, 0, FALSE, TRUE, NULL));
          emitcode ("push", "%s", aopGet (to, 1, FALSE, TRUE, NULL));
          if (options.model == MODEL_FLAT24)
            emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
          emitcode ("pop", "dph");
          emitcode ("pop", "dpl");
        }
      else
        {
          _startLazyDPSEvaluation ();
          /* if this is remateriazable */
          if (AOP_TYPE (to) == AOP_IMMD)
            {
              emitcode ("mov", "dptr,%s", aopGet (to, 0, TRUE, FALSE, NULL));
            }
          else
            {
              /* we need to get it byte by byte */
              emitcode ("mov", "dpl,%s", aopGet (to, 0, FALSE, FALSE, NULL));
              emitcode ("mov", "dph,%s", aopGet (to, 1, FALSE, FALSE, NULL));
              if (options.model == MODEL_FLAT24)
                {
                  emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
                }
            }
          _endLazyDPSEvaluation ();
        }
    }
  freeAsmop (to, NULL, ic, FALSE);

  _G.dptrInUse = _G.dptr1InUse = 1;
  aopOp (count, ic->next->next, FALSE, FALSE);
  lbl = newiTempLabel (NULL);

  /* now for the actual copy */
  if (AOP_TYPE (count) == AOP_LIT && (int) ulFromVal (AOP (count)->aopu.aop_lit) <= 256)
    {
      emitcode (";", "OH  JOY auto increment with djnz (very fast)");
      emitcode ("mov", "dps,#!constbyte", 0x1); /* Select DPTR2 */
      emitcode ("mov", "b,%s", aopGet (count, 0, FALSE, FALSE, NULL));
      freeAsmop (count, NULL, ic, FALSE);
      emitLabel (lbl);
      emitcode ("movx", "a,@dptr");     /* read data from port */
      emitcode ("dec", "dps");  /* switch to DPTR */
      emitcode ("movx", "@dptr,a");     /* save into location */
      emitcode ("inc", "dptr"); /* point to next area */
      emitcode ("inc", "dps");  /* switch to DPTR2 */
      emitcode ("djnz", "b,!tlabel", labelKey2num (lbl->key));
    }
  else
    {
      symbol *lbl1 = newiTempLabel (NULL);

      emitcode (";", " Auto increment but no djnz");
      emitcode ("mov", "acc1,%s", aopGet (count, 0, FALSE, TRUE, NULL));
      emitcode ("mov", "b,%s", aopGet (count, 1, FALSE, TRUE, NULL));
      freeAsmop (count, NULL, ic, FALSE);
      emitcode ("mov", "dps,#!constbyte", 0x1); /* Select DPTR2 */
      emitLabel (lbl);
      emitcode ("movx", "a,@dptr");
      emitcode ("dec", "dps");  /* switch to DPTR */
      emitcode ("movx", "@dptr,a");
      emitcode ("inc", "dptr");
      emitcode ("inc", "dps");  /* switch to DPTR2 */
      /*      emitcode ("djnz","b,!tlabel",lbl->key+100); */
      /*      emitcode ("djnz","acc1,!tlabel",lbl->key+100); */
      emitcode ("mov", "a,b");
      emitcode ("orl", "a,acc1");
      emitcode ("jz", "!tlabel", labelKey2num (lbl1->key));
      emitcode ("mov", "a,acc1");
      emitcode ("add", "a,#!constbyte", 0xFF);
      emitcode ("mov", "acc1,a");
      emitcode ("mov", "a,b");
      emitcode ("addc", "a,#!constbyte", 0xFF);
      emitcode ("mov", "b,a");
      emitcode ("sjmp", "!tlabel", labelKey2num (lbl->key));
      emitLabel (lbl1);
    }
  emitcode ("mov", "dps,#0");
  _G.dptrInUse = _G.dptr1InUse = 0;
  unsavermask (rsave);

}

/*-----------------------------------------------------------------*/
/* genOutp - gen code for __builtin_inp write data to a mem mapped */
/* port, first parameter output area second parameter pointer to   */
/* port third parameter count                                      */
/*-----------------------------------------------------------------*/
static void
genOutp (iCode * ic, int nparms, operand ** parms)
{
  operand *from, *to, *count;
  symbol *lbl;
  bitVect *rsave;
  int i;

  /* we know it has to be 3 parameters */
  assert (nparms == 3);

  rsave = newBitVect (16);
  /* save DPTR if it needs to be saved */
  for (i = DPL_IDX; i <= B_IDX; i++)
    {
      if (bitVectBitValue (ic->rMask, i))
        rsave = bitVectSetBit (rsave, i);
    }
  rsave = bitVectIntersect (rsave, bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));
  savermask (rsave);

  to = parms[0];
  from = parms[1];
  count = parms[2];

  aopOp (from, ic->next, FALSE, FALSE);

  /* get from into DPTR1 */
  emitcode ("mov", "dpl1,%s", aopGet (from, 0, FALSE, FALSE, NULL));
  emitcode ("mov", "dph1,%s", aopGet (from, 1, FALSE, FALSE, NULL));
  if (options.model == MODEL_FLAT24)
    {
      emitcode ("mov", "dpx1,%s", aopGet (from, 2, FALSE, FALSE, NULL));
    }

  freeAsmop (from, NULL, ic, FALSE);
  aopOp (to, ic, FALSE, FALSE);
  /* get "to" into DPTR */
  /* if the operand is already in dptr
     then we do nothing else we move the value to dptr */
  if (AOP_TYPE (to) != AOP_STR)
    {
      /* if already in DPTR then we need to push */
      if (AOP_TYPE (to) == AOP_DPTR)
        {
          emitcode ("push", "%s", aopGet (to, 0, FALSE, TRUE, NULL));
          emitcode ("push", "%s", aopGet (to, 1, FALSE, TRUE, NULL));
          if (options.model == MODEL_FLAT24)
            emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
          emitcode ("pop", "dph");
          emitcode ("pop", "dpl");
        }
      else
        {
          _startLazyDPSEvaluation ();
          /* if this is remateriazable */
          if (AOP_TYPE (to) == AOP_IMMD)
            {
              emitcode ("mov", "dptr,%s", aopGet (to, 0, TRUE, FALSE, NULL));
            }
          else
            {
              /* we need to get it byte by byte */
              emitcode ("mov", "dpl,%s", aopGet (to, 0, FALSE, FALSE, NULL));
              emitcode ("mov", "dph,%s", aopGet (to, 1, FALSE, FALSE, NULL));
              if (options.model == MODEL_FLAT24)
                {
                  emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
                }
            }
          _endLazyDPSEvaluation ();
        }
    }
  freeAsmop (to, NULL, ic, FALSE);

  _G.dptrInUse = _G.dptr1InUse = 1;
  aopOp (count, ic->next->next, FALSE, FALSE);
  lbl = newiTempLabel (NULL);

  /* now for the actual copy */
  if (AOP_TYPE (count) == AOP_LIT && (int) ulFromVal (AOP (count)->aopu.aop_lit) <= 256)
    {
      emitcode (";", "OH  JOY auto increment with djnz (very fast)");
      emitcode ("mov", "dps,#!constbyte", 0x0); /* Select DPTR */
      emitcode ("mov", "b,%s", aopGet (count, 0, FALSE, FALSE, NULL));
      emitLabel (lbl);
      emitcode ("movx", "a,@dptr");     /* read data from port */
      emitcode ("inc", "dps");  /* switch to DPTR2 */
      emitcode ("movx", "@dptr,a");     /* save into location */
      emitcode ("inc", "dptr"); /* point to next area */
      emitcode ("dec", "dps");  /* switch to DPTR */
      emitcode ("djnz", "b,!tlabel", labelKey2num (lbl->key));
      freeAsmop (count, NULL, ic, FALSE);
    }
  else
    {
      symbol *lbl1 = newiTempLabel (NULL);

      emitcode (";", " Auto increment but no djnz");
      emitcode ("mov", "acc1,%s", aopGet (count, 0, FALSE, TRUE, NULL));
      emitcode ("mov", "b,%s", aopGet (count, 1, FALSE, TRUE, NULL));
      freeAsmop (count, NULL, ic, FALSE);
      emitcode ("mov", "dps,#!constbyte", 0x0); /* Select DPTR */
      emitLabel (lbl);
      emitcode ("movx", "a,@dptr");
      emitcode ("inc", "dptr");
      emitcode ("inc", "dps");  /* switch to DPTR2 */
      emitcode ("movx", "@dptr,a");
      emitcode ("dec", "dps");  /* switch to DPTR */
      emitcode ("mov", "a,b");
      emitcode ("orl", "a,acc1");
      emitcode ("jz", "!tlabel", labelKey2num (lbl1->key));
      emitcode ("mov", "a,acc1");
      emitcode ("add", "a,#!constbyte", 0xFF);
      emitcode ("mov", "acc1,a");
      emitcode ("mov", "a,b");
      emitcode ("addc", "a,#!constbyte", 0xFF);
      emitcode ("mov", "b,a");
      emitcode ("sjmp", "!tlabel", labelKey2num (lbl->key));
      emitLabel (lbl1);
    }
  emitcode ("mov", "dps,#0");
  _G.dptrInUse = _G.dptr1InUse = 0;
  unsavermask (rsave);

}

/*-----------------------------------------------------------------*/
/* genSwapW - swap lower & high order bytes                        */
/*-----------------------------------------------------------------*/
static void
genSwapW (iCode * ic, int nparms, operand ** parms)
{
  operand *dest;
  operand *src;
  assert (nparms == 1);

  src = parms[0];
  dest = IC_RESULT (ic);

  assert (getSize (operandType (src)) == 2);

  aopOp (src, ic, FALSE, FALSE);
  emitcode ("mov", "a,%s", aopGet (src, 0, FALSE, FALSE, NULL));
  _G.accInUse++;
  MOVB (aopGet (src, 1, FALSE, FALSE, "b"));
  _G.accInUse--;
  freeAsmop (src, NULL, ic, FALSE);

  aopOp (dest, ic, FALSE, FALSE);
  aopPut (dest, "b", 0);
  aopPut (dest, "a", 1);
  freeAsmop (dest, NULL, ic, FALSE);
}

/*-----------------------------------------------------------------*/
/* genMemsetX - gencode for memSetX data                           */
/*-----------------------------------------------------------------*/
static void
genMemsetX (iCode * ic, int nparms, operand ** parms)
{
  operand *to, *val, *count;
  symbol *lbl;
  int i;
  bitVect *rsave;

  /* we know it has to be 3 parameters */
  assert (nparms == 3);

  to = parms[0];
  val = parms[1];
  count = parms[2];

  /* save DPTR if it needs to be saved */
  rsave = newBitVect (16);
  for (i = DPL_IDX; i <= B_IDX; i++)
    {
      if (bitVectBitValue (ic->rMask, i))
        rsave = bitVectSetBit (rsave, i);
    }
  rsave = bitVectIntersect (rsave, bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));
  savermask (rsave);

  aopOp (to, ic, FALSE, FALSE);
  /* get "to" into DPTR */
  /* if the operand is already in dptr
     then we do nothing else we move the value to dptr */
  if (AOP_TYPE (to) != AOP_STR)
    {
      /* if already in DPTR then we need to push */
      if (AOP_TYPE (to) == AOP_DPTR)
        {
          emitcode ("push", "%s", aopGet (to, 0, FALSE, TRUE, NULL));
          emitcode ("push", "%s", aopGet (to, 1, FALSE, TRUE, NULL));
          if (options.model == MODEL_FLAT24)
            emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
          emitcode ("pop", "dph");
          emitcode ("pop", "dpl");
        }
      else
        {
          _startLazyDPSEvaluation ();
          /* if this is remateriazable */
          if (AOP_TYPE (to) == AOP_IMMD)
            {
              emitcode ("mov", "dptr,%s", aopGet (to, 0, TRUE, FALSE, NULL));
            }
          else
            {
              /* we need to get it byte by byte */
              emitcode ("mov", "dpl,%s", aopGet (to, 0, FALSE, FALSE, NULL));
              emitcode ("mov", "dph,%s", aopGet (to, 1, FALSE, FALSE, NULL));
              if (options.model == MODEL_FLAT24)
                {
                  emitcode ("mov", "dpx,%s", aopGet (to, 2, FALSE, FALSE, NULL));
                }
            }
          _endLazyDPSEvaluation ();
        }
    }
  freeAsmop (to, NULL, ic, FALSE);

  aopOp (val, ic->next->next, FALSE, FALSE);
  aopOp (count, ic->next->next, FALSE, FALSE);
  lbl = newiTempLabel (NULL);
  /* now for the actual copy */
  if (AOP_TYPE (count) == AOP_LIT && (int) ulFromVal (AOP (count)->aopu.aop_lit) <= 256)
    {
      char *l = Safe_strdup (aopGet (val, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "b,%s", aopGet (count, 0, FALSE, FALSE, NULL));
      MOVA (l);
      Safe_free (l);
      emitLabel (lbl);
      emitcode ("movx", "@dptr,a");
      emitcode ("inc", "dptr");
      emitcode ("djnz", "b,!tlabel", labelKey2num (lbl->key));
    }
  else
    {
      symbol *lbl1 = newiTempLabel (NULL);

      emitcode ("mov", "acc1,%s", aopGet (count, 0, FALSE, TRUE, NULL));
      emitcode ("mov", "b,%s", aopGet (count, 1, FALSE, TRUE, NULL));
      emitLabel (lbl);
      MOVA (aopGet (val, 0, FALSE, FALSE, NULL));
      emitcode ("movx", "@dptr,a");
      emitcode ("inc", "dptr");
      emitcode ("mov", "a,b");
      emitcode ("orl", "a,acc1");
      emitcode ("jz", "!tlabel", labelKey2num (lbl1->key));
      emitcode ("mov", "a,acc1");
      emitcode ("add", "a,#!constbyte", 0xFF);
      emitcode ("mov", "acc1,a");
      emitcode ("mov", "a,b");
      emitcode ("addc", "a,#!constbyte", 0xFF);
      emitcode ("mov", "b,a");
      emitcode ("sjmp", "!tlabel", labelKey2num (lbl->key));
      emitLabel (lbl1);
    }
  freeAsmop (count, NULL, ic, FALSE);
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genNatLibLoadPrimitive - calls TINI api function to load primitive */
/*-----------------------------------------------------------------*/
static void
genNatLibLoadPrimitive (iCode * ic, int nparms, operand ** parms, int size)
{
  bitVect *rsave;
  operand *pnum, *result;
  int i;

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  pnum = parms[0];
  aopOp (pnum, ic, FALSE, FALSE);
  emitcode ("mov", "a,%s", aopGet (pnum, 0, FALSE, FALSE, DP2_RESULT_REG));
  freeAsmop (pnum, NULL, ic, FALSE);
  emitcode ("lcall", "NatLib_LoadPrimitive");
  aopOp (result = IC_RESULT (ic), ic, FALSE, FALSE);
  if (aopHasRegs (AOP (result), R0_IDX, R1_IDX) || aopHasRegs (AOP (result), R2_IDX, R3_IDX))
    {
      for (i = (size - 1); i >= 0; i--)
        {
          emitcode ("push", "a%s", javaRet[i]);
        }
      for (i = 0; i < size; i++)
        {
          emitcode ("pop", "a%s", aopGet (result, i, FALSE, FALSE, DP2_RESULT_REG));
        }
    }
  else
    {
      for (i = 0; i < size; i++)
        {
          aopPut (result, javaRet[i], i);
        }
    }
  freeAsmop (result, NULL, ic, FALSE);
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genNatLibLoadPointer - calls TINI api function to load pointer  */
/*-----------------------------------------------------------------*/
static void
genNatLibLoadPointer (iCode * ic, int nparms, operand ** parms)
{
  bitVect *rsave;
  operand *pnum, *result;
  int size = 3;
  int i;

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  pnum = parms[0];
  aopOp (pnum, ic, FALSE, FALSE);
  emitcode ("mov", "a,%s", aopGet (pnum, 0, FALSE, FALSE, DP2_RESULT_REG));
  freeAsmop (pnum, NULL, ic, FALSE);
  emitcode ("lcall", "NatLib_LoadPointer");
  aopOp (result = IC_RESULT (ic), ic, FALSE, FALSE);
  if (AOP_TYPE (result) != AOP_STR)
    {
      for (i = 0; i < size; i++)
        {
          aopPut (result, fReturn[i], i);
        }
    }
  freeAsmop (result, NULL, ic, FALSE);
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genNatLibInstallStateBlock -                                    */
/*-----------------------------------------------------------------*/
static void
genNatLibInstallStateBlock (iCode * ic, int nparms, operand ** parms, const char *name)
{
  bitVect *rsave;
  operand *psb, *handle;
  assert (nparms == 2);

  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));
  psb = parms[0];
  handle = parms[1];

  /* put pointer to state block into DPTR1 */
  aopOp (psb, ic, FALSE, FALSE);
  if (AOP_TYPE (psb) == AOP_IMMD)
    {
      emitcode ("mov", "dps,#1");
      emitcode ("mov", "dptr,%s", aopGet (psb, 0, TRUE, FALSE, DP2_RESULT_REG));
      emitcode ("mov", "dps,#0");
    }
  else
    {
      emitcode ("mov", "dpl1,%s", aopGet (psb, 0, FALSE, FALSE, DP2_RESULT_REG));
      emitcode ("mov", "dph1,%s", aopGet (psb, 1, FALSE, FALSE, DP2_RESULT_REG));
      emitcode ("mov", "dpx1,%s", aopGet (psb, 2, FALSE, FALSE, DP2_RESULT_REG));
    }
  freeAsmop (psb, NULL, ic, FALSE);

  /* put libraryID into DPTR */
  emitcode ("mov", "dptr,#LibraryID");

  /* put handle into r3:r2 */
  aopOp (handle, ic, FALSE, FALSE);
  if (aopHasRegs (AOP (handle), R2_IDX, R3_IDX))
    {
      emitcode ("push", "%s", aopGet (handle, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("push", "%s", aopGet (handle, 1, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("pop", "ar3");
      emitcode ("pop", "ar2");
    }
  else
    {
      emitcode ("mov", "r2,%s", aopGet (handle, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("mov", "r3,%s", aopGet (handle, 1, FALSE, TRUE, DP2_RESULT_REG));
    }
  freeAsmop (psb, NULL, ic, FALSE);

  /* make the call */
  emitcode ("lcall", "NatLib_Install%sStateBlock", name);

  /* put return value into place */
  _G.accInUse++;
  aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
  _G.accInUse--;
  aopPut (IC_RESULT (ic), "a", 0);
  freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genNatLibRemoveStateBlock -                                     */
/*-----------------------------------------------------------------*/
static void
genNatLibRemoveStateBlock (iCode * ic, int nparms, const char *name)
{
  bitVect *rsave;

  assert (nparms == 0);

  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  /* put libraryID into DPTR */
  emitcode ("mov", "dptr,#LibraryID");
  /* make the call */
  emitcode ("lcall", "NatLib_Remove%sStateBlock", name);
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genNatLibGetStateBlock -                                        */
/*-----------------------------------------------------------------*/
static void
genNatLibGetStateBlock (iCode * ic, int nparms, operand ** parms, const char *name)
{
  bitVect *rsave;
  symbol *lbl = newiTempLabel (NULL);

  assert (nparms == 0);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  /* put libraryID into DPTR */
  emitcode ("mov", "dptr,#LibraryID");
  /* make the call */
  emitcode ("lcall", "NatLib_Remove%sStateBlock", name);
  emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));

  /* put return value into place */
  aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
  if (aopHasRegs (AOP (IC_RESULT (ic)), R2_IDX, R3_IDX))
    {
      emitcode ("push", "ar3");
      emitcode ("push", "ar2");
      emitcode ("pop", "%s", aopGet (IC_RESULT (ic), 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("pop", "%s", aopGet (IC_RESULT (ic), 1, FALSE, TRUE, DP2_RESULT_REG));
    }
  else
    {
      aopPut (IC_RESULT (ic), "r2", 0);
      aopPut (IC_RESULT (ic), "r3", 1);
    }
  freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
  emitLabel (lbl);
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genMMMalloc -                                                   */
/*-----------------------------------------------------------------*/
static void
genMMMalloc (iCode * ic, int nparms, operand ** parms, int size, const char *name)
{
  bitVect *rsave;
  operand *bsize;
  symbol *rsym;
  symbol *lbl = newiTempLabel (NULL);

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  bsize = parms[0];
  aopOp (bsize, ic, FALSE, FALSE);

  /* put the size in R4-R2 */
  if (aopHasRegs (AOP (bsize), R2_IDX, (size == 3 ? R4_IDX : R3_IDX)))
    {
      emitcode ("push", "%s", aopGet (bsize, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("push", "%s", aopGet (bsize, 1, FALSE, TRUE, DP2_RESULT_REG));
      if (size == 3)
        {
          emitcode ("push", "%s", aopGet (bsize, 2, FALSE, TRUE, DP2_RESULT_REG));
          emitcode ("pop", "ar4");
        }
      emitcode ("pop", "ar3");
      emitcode ("pop", "ar2");
    }
  else
    {
      emitcode ("mov", "r2,%s", aopGet (bsize, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("mov", "r3,%s", aopGet (bsize, 1, FALSE, TRUE, DP2_RESULT_REG));
      if (size == 3)
        {
          emitcode ("mov", "r4,%s", aopGet (bsize, 2, FALSE, TRUE, DP2_RESULT_REG));
        }
    }
  freeAsmop (bsize, NULL, ic, FALSE);

  /* make the call */
  emitcode ("lcall", "MM_%s", name);
  emitcode ("jz", "!tlabel", labelKey2num (lbl->key));
  emitcode ("mov", "r2,#!constbyte", 0xff);
  emitcode ("mov", "r3,#!constbyte", 0xff);
  emitLabel (lbl);
  /* we don't care about the pointer : we just save the handle */
  rsym = OP_SYMBOL (IC_RESULT (ic));
  if (rsym->liveFrom != rsym->liveTo)
    {
      aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
      if (aopHasRegs (AOP (IC_RESULT (ic)), R2_IDX, R3_IDX))
        {
          emitcode ("push", "ar3");
          emitcode ("push", "ar2");
          emitcode ("pop", "%s", aopGet (IC_RESULT (ic), 0, FALSE, TRUE, DP2_RESULT_REG));
          emitcode ("pop", "%s", aopGet (IC_RESULT (ic), 1, FALSE, TRUE, DP2_RESULT_REG));
        }
      else
        {
          aopPut (IC_RESULT (ic), "r2", 0);
          aopPut (IC_RESULT (ic), "r3", 1);
        }
      freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
    }
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genMMDeref -                                                    */
/*-----------------------------------------------------------------*/
static void
genMMDeref (iCode * ic, int nparms, operand ** parms)
{
  bitVect *rsave;
  operand *handle;

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  handle = parms[0];
  aopOp (handle, ic, FALSE, FALSE);

  /* put the size in R4-R2 */
  if (aopHasRegs (AOP (handle), R2_IDX, R3_IDX))
    {
      emitcode ("push", "%s", aopGet (handle, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("push", "%s", aopGet (handle, 1, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("pop", "ar3");
      emitcode ("pop", "ar2");
    }
  else
    {
      emitcode ("mov", "r2,%s", aopGet (handle, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("mov", "r3,%s", aopGet (handle, 1, FALSE, TRUE, DP2_RESULT_REG));
    }
  freeAsmop (handle, NULL, ic, FALSE);

  /* make the call */
  emitcode ("lcall", "MM_Deref");

  {
    symbol *rsym = OP_SYMBOL (IC_RESULT (ic));
    if (rsym->liveFrom != rsym->liveTo)
      {
        aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
        if (AOP_TYPE (IC_RESULT (ic)) != AOP_STR)
          {
            _startLazyDPSEvaluation ();

            aopPut (IC_RESULT (ic), "dpl", 0);
            aopPut (IC_RESULT (ic), "dph", 1);
            aopPut (IC_RESULT (ic), "dpx", 2);

            _endLazyDPSEvaluation ();

          }
      }
  }
  freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genMMUnrestrictedPersist -                                      */
/*-----------------------------------------------------------------*/
static void
genMMUnrestrictedPersist (iCode * ic, int nparms, operand ** parms)
{
  bitVect *rsave;
  operand *handle;

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  handle = parms[0];
  aopOp (handle, ic, FALSE, FALSE);

  /* put the size in R3-R2 */
  if (aopHasRegs (AOP (handle), R2_IDX, R3_IDX))
    {
      emitcode ("push", "%s", aopGet (handle, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("push", "%s", aopGet (handle, 1, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("pop", "ar3");
      emitcode ("pop", "ar2");
    }
  else
    {
      emitcode ("mov", "r2,%s", aopGet (handle, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("mov", "r3,%s", aopGet (handle, 1, FALSE, TRUE, DP2_RESULT_REG));
    }
  freeAsmop (handle, NULL, ic, FALSE);

  /* make the call */
  emitcode ("lcall", "MM_UnrestrictedPersist");

  {
    symbol *rsym = OP_SYMBOL (IC_RESULT (ic));
    if (rsym->liveFrom != rsym->liveTo)
      {
        aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
        aopPut (IC_RESULT (ic), "a", 0);
        freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
      }
  }
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genSystemExecJavaProcess -                                      */
/*-----------------------------------------------------------------*/
static void
genSystemExecJavaProcess (iCode * ic, int nparms, operand ** parms)
{
  bitVect *rsave;
  operand *handle, *pp;

  assert (nparms == 2);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  pp = parms[0];
  handle = parms[1];

  /* put the handle in R3-R2 */
  aopOp (handle, ic, FALSE, FALSE);
  if (aopHasRegs (AOP (handle), R2_IDX, R3_IDX))
    {
      emitcode ("push", "%s", aopGet (handle, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("push", "%s", aopGet (handle, 1, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("pop", "ar3");
      emitcode ("pop", "ar2");
    }
  else
    {
      emitcode ("mov", "r2,%s", aopGet (handle, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("mov", "r3,%s", aopGet (handle, 1, FALSE, TRUE, DP2_RESULT_REG));
    }
  freeAsmop (handle, NULL, ic, FALSE);

  /* put pointer in DPTR */
  aopOp (pp, ic, FALSE, FALSE);
  if (AOP_TYPE (pp) == AOP_IMMD)
    {
      emitcode ("mov", "dptr,%s", aopGet (pp, 0, TRUE, FALSE, NULL));
    }
  else if (AOP_TYPE (pp) != AOP_STR)
    {
      /* not already in dptr */
      emitcode ("mov", "dpl,%s", aopGet (pp, 0, FALSE, FALSE, NULL));
      emitcode ("mov", "dph,%s", aopGet (pp, 1, FALSE, FALSE, NULL));
      emitcode ("mov", "dpx,%s", aopGet (pp, 2, FALSE, FALSE, NULL));
    }
  freeAsmop (handle, NULL, ic, FALSE);

  /* make the call */
  emitcode ("lcall", "System_ExecJavaProcess");

  /* put result in place */
  {
    symbol *rsym = OP_SYMBOL (IC_RESULT (ic));
    if (rsym->liveFrom != rsym->liveTo)
      {
        aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
        aopPut (IC_RESULT (ic), "a", 0);
        freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
      }
  }

  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genSystemRTCRegisters -                                         */
/*-----------------------------------------------------------------*/
static void
genSystemRTCRegisters (iCode * ic, int nparms, operand ** parms, char *name)
{
  bitVect *rsave;
  operand *pp;

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  pp = parms[0];
  /* put pointer in DPTR */
  aopOp (pp, ic, FALSE, FALSE);
  if (AOP_TYPE (pp) == AOP_IMMD)
    {
      emitcode ("mov", "dps,#1");
      emitcode ("mov", "dptr,%s", aopGet (pp, 0, TRUE, FALSE, NULL));
      emitcode ("mov", "dps,#0");
    }
  else
    {
      emitcode ("mov", "dpl1,%s", aopGet (pp, 0, FALSE, FALSE, DP2_RESULT_REG));
      emitcode ("mov", "dph1,%s", aopGet (pp, 1, FALSE, FALSE, DP2_RESULT_REG));
      emitcode ("mov", "dpx1,%s", aopGet (pp, 2, FALSE, FALSE, DP2_RESULT_REG));
    }
  freeAsmop (pp, NULL, ic, FALSE);

  /* make the call */
  emitcode ("lcall", "System_%sRTCRegisters", name);

  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genSystemThreadSleep -                                          */
/*-----------------------------------------------------------------*/
static void
genSystemThreadSleep (iCode * ic, int nparms, operand ** parms, char *name)
{
  bitVect *rsave;
  operand *to, *s;

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  to = parms[0];
  aopOp (to, ic, FALSE, FALSE);
  if (aopHasRegs (AOP (to), R2_IDX, R3_IDX) || aopHasRegs (AOP (to), R0_IDX, R1_IDX))
    {
      emitcode ("push", "%s", aopGet (to, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("push", "%s", aopGet (to, 1, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("push", "%s", aopGet (to, 2, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("push", "%s", aopGet (to, 3, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("pop", "ar3");
      emitcode ("pop", "ar2");
      emitcode ("pop", "ar1");
      emitcode ("pop", "ar0");
    }
  else
    {
      emitcode ("mov", "r0,%s", aopGet (to, 0, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("mov", "r1,%s", aopGet (to, 1, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("mov", "r2,%s", aopGet (to, 2, FALSE, TRUE, DP2_RESULT_REG));
      emitcode ("mov", "r3,%s", aopGet (to, 3, FALSE, TRUE, DP2_RESULT_REG));
    }
  freeAsmop (to, NULL, ic, FALSE);

  /* suspend in acc */
  s = parms[1];
  aopOp (s, ic, FALSE, FALSE);
  emitcode ("mov", "a,%s", aopGet (s, 0, FALSE, TRUE, NULL));
  freeAsmop (s, NULL, ic, FALSE);

  /* make the call */
  emitcode ("lcall", "System_%s", name);

  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genSystemThreadResume -                                         */
/*-----------------------------------------------------------------*/
static void
genSystemThreadResume (iCode * ic, int nparms, operand ** parms)
{
  bitVect *rsave;
  operand *tid, *pid;

  assert (nparms == 2);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  tid = parms[0];
  pid = parms[1];

  /* PID in R0 */
  aopOp (pid, ic, FALSE, FALSE);
  emitcode ("mov", "r0,%s", aopGet (pid, 0, FALSE, TRUE, DP2_RESULT_REG));
  freeAsmop (pid, NULL, ic, FALSE);

  /* tid into ACC */
  aopOp (tid, ic, FALSE, FALSE);
  emitcode ("mov", "a,%s", aopGet (tid, 0, FALSE, TRUE, DP2_RESULT_REG));
  freeAsmop (tid, NULL, ic, FALSE);

  emitcode ("lcall", "System_ThreadResume");

  /* put result into place */
  {
    symbol *rsym = OP_SYMBOL (IC_RESULT (ic));
    if (rsym->liveFrom != rsym->liveTo)
      {
        aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
        aopPut (IC_RESULT (ic), "a", 0);
        freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
      }
  }
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genSystemProcessResume -                                        */
/*-----------------------------------------------------------------*/
static void
genSystemProcessResume (iCode * ic, int nparms, operand ** parms)
{
  bitVect *rsave;
  operand *pid;

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  pid = parms[0];

  /* pid into ACC */
  aopOp (pid, ic, FALSE, FALSE);
  emitcode ("mov", "a,%s", aopGet (pid, 0, FALSE, TRUE, DP2_RESULT_REG));
  freeAsmop (pid, NULL, ic, FALSE);

  emitcode ("lcall", "System_ProcessResume");

  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genSystem -                                                     */
/*-----------------------------------------------------------------*/
static void
genSystem (iCode * ic, int nparms, char *name)
{
  assert (nparms == 0);

  emitcode ("lcall", "System_%s", name);
}

/*-----------------------------------------------------------------*/
/* genSystemPoll -                                                  */
/*-----------------------------------------------------------------*/
static void
genSystemPoll (iCode * ic, int nparms, operand ** parms, char *name)
{
  bitVect *rsave;
  operand *fp;

  assert (nparms == 1);
  /* save registers that need to be saved */
  savermask (rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ds390_rUmaskForOp (IC_RESULT (ic))));

  fp = parms[0];
  aopOp (fp, ic, FALSE, FALSE);
  if (AOP_TYPE (fp) == AOP_IMMD)
    {
      emitcode ("mov", "dptr,%s", aopGet (fp, 0, TRUE, FALSE, DP2_RESULT_REG));
    }
  else if (AOP_TYPE (fp) != AOP_STR)
    {
      /* not already in dptr */
      emitcode ("mov", "dpl,%s", aopGet (fp, 0, FALSE, FALSE, DP2_RESULT_REG));
      emitcode ("mov", "dph,%s", aopGet (fp, 1, FALSE, FALSE, DP2_RESULT_REG));
      emitcode ("mov", "dpx,%s", aopGet (fp, 2, FALSE, FALSE, DP2_RESULT_REG));
    }
  freeAsmop (fp, NULL, ic, FALSE);

  emitcode ("lcall", "System_%sPoll", name);

  /* put result into place */
  {
    symbol *rsym = OP_SYMBOL (IC_RESULT (ic));
    if (rsym->liveFrom != rsym->liveTo)
      {
        aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
        aopPut (IC_RESULT (ic), "a", 0);
        freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
      }
  }
  unsavermask (rsave);
}

/*-----------------------------------------------------------------*/
/* genSystemGetCurrentID -                                         */
/*-----------------------------------------------------------------*/
static void
genSystemGetCurrentID (iCode * ic, int nparms, operand ** parms, char *name)
{
  assert (nparms == 0);

  emitcode ("lcall", "System_GetCurrent%sId", name);
  /* put result into place */
  {
    symbol *rsym = OP_SYMBOL (IC_RESULT (ic));
    if (rsym->liveFrom != rsym->liveTo)
      {
        aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
        aopPut (IC_RESULT (ic), "a", 0);
        freeAsmop (IC_RESULT (ic), NULL, ic, FALSE);
      }
  }
}

/*-----------------------------------------------------------------*/
/* genDjnz - generate decrement & jump if not zero instruction     */
/*-----------------------------------------------------------------*/
static int
genDjnz (iCode * ic, iCode * ifx)
{
  symbol *lbl, *lbl1;
  if (!ifx)
    return 0;

  /* if the if condition has a false label
     then we cannot save */
  if (IC_FALSE (ifx))
    return 0;

  /* if the minus is not of the form a = a - 1 */
  if (!isOperandEqual (IC_RESULT (ic), IC_LEFT (ic)) || !IS_OP_LITERAL (IC_RIGHT (ic)))
    return 0;

  if (operandLitValue (IC_RIGHT (ic)) != 1)
    return 0;

  /* if the size of this greater than one then no saving */
  if (getSize (operandType (IC_RESULT (ic))) > 1)
    return 0;

  /* otherwise we can save BIG */

  popForBranch (ic->next, TRUE);

  D (emitcode (";", "genDjnz"));

  lbl = newiTempLabel (NULL);
  lbl1 = newiTempLabel (NULL);

  aopOp (IC_RESULT (ic), ic, FALSE, FALSE);

  if (AOP_NEEDSACC (IC_RESULT (ic)))
    {
      /* If the result is accessed indirectly via
       * the accumulator, we must explicitly write
       * it back after the decrement.
       */
      const char *rByte = aopGet (IC_RESULT (ic), 0, FALSE, FALSE, NULL);

      if (!EQ (rByte, "a"))
        {
          /* Something is hopelessly wrong */
          fprintf (stderr, "*** warning: internal error at %s:%d\n", __FILE__, __LINE__);
          /* We can just give up; the generated code will be inefficient,
           * but what the hey.
           */
          freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
          return 0;
        }
      emitcode ("dec", "%s", rByte);
      aopPut (IC_RESULT (ic), rByte, 0);
      emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));
    }
  else if (IS_AOP_PREG (IC_RESULT (ic)))
    {
      emitcode ("dec", "%s", aopGet (IC_RESULT (ic), 0, FALSE, FALSE, NULL));
      MOVA (aopGet (IC_RESULT (ic), 0, FALSE, FALSE, NULL));
      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
      ifx->generated = 1;
      emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));
    }
  else
    {
      emitcode ("djnz", "%s,!tlabel", aopGet (IC_RESULT (ic), 0, FALSE, TRUE, NULL), labelKey2num (lbl->key));
    }
  emitcode ("sjmp", "!tlabel", labelKey2num (lbl1->key));
  emitLabel (lbl);
  emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ifx)->key));
  emitLabel (lbl1);

  if (!ifx->generated)
    freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
  ifx->generated = 1;
  return 1;
}

/*-----------------------------------------------------------------*/
/* genReceive - generate code for a receive iCode                  */
/*-----------------------------------------------------------------*/
static void
genReceive (iCode * ic)
{
  int size = getSize (operandType (IC_RESULT (ic)));
  int offset = 0;
  int rb1off;

  D (emitcode (";", "genReceive"));

  if (ic->argreg == 1)
    {
      /* first parameter */
      if (IS_OP_RUONLY (IC_RESULT (ic)))
        {
          /* Nothing to do: it's already in the proper place. */
          return;
        }
      else
        {
          bool useDp2;

          useDp2 = isOperandInFarSpace (IC_RESULT (ic)) &&
                   (OP_SYMBOL (IC_RESULT (ic))->isspilt || IS_TRUE_SYMOP (IC_RESULT (ic)));

          _G.accInUse++;
          aopOp (IC_RESULT (ic), ic, FALSE, useDp2);
          _G.accInUse--;

          /* Sanity checking... */
          if (AOP_USESDPTR (IC_RESULT (ic)))
            {
              werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "genReceive got unexpected DPTR.");
            }
          assignResultValue (IC_RESULT (ic), NULL);
        }
    }
  else if (ic->argreg > 12)
    {
      /* bit parameters */
      reg_info *reg = OP_SYMBOL (IC_RESULT (ic))->regs[0];

      if (!reg || reg->rIdx != ic->argreg - 5)
        {
          aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
          emitcode ("mov", "c,%s", rb1regs[ic->argreg - 5]);
          outBitC (IC_RESULT (ic));
        }
    }
  else
    {
      /* second receive onwards */
      /* this gets a little tricky since unused receives will be
         eliminated, we have saved the reg in the type field . and
         we use that to figure out which register to use */
      aopOp (IC_RESULT (ic), ic, FALSE, FALSE);
      rb1off = ic->argreg;
      while (size--)
        {
          aopPut (IC_RESULT (ic), rb1regs[rb1off++ - 5], offset++);
        }
    }
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genDummyRead - generate code for dummy read of volatiles        */
/*-----------------------------------------------------------------*/
static void
genDummyRead (iCode * ic)
{
  operand *op;
  int size, offset;

  D (emitcode (";", "genDummyRead"));

  op = IC_RIGHT (ic);
  if (op && IS_SYMOP (op))
    {
      aopOp (op, ic, FALSE, FALSE);

      /* if the result is a bit */
      if (AOP_TYPE (op) == AOP_CRY)
        emitcode ("mov", "c,%s", AOP (op)->aopu.aop_dir);
      else
        {
          /* bit variables done */
          /* general case */
          size = AOP_SIZE (op);
          offset = 0;
          while (size--)
            {
              MOVA (aopGet (op, offset, FALSE, FALSE, FALSE));
              offset++;
            }
        }

      freeAsmop (op, NULL, ic, TRUE);
    }

  op = IC_LEFT (ic);
  if (op && IS_SYMOP (op))
    {
      aopOp (op, ic, FALSE, FALSE);

      /* if the result is a bit */
      if (AOP_TYPE (op) == AOP_CRY)
        emitcode ("mov", "c,%s", AOP (op)->aopu.aop_dir);
      else
        {
          /* bit variables done */
          /* general case */
          size = AOP_SIZE (op);
          offset = 0;
          while (size--)
            {
              MOVA (aopGet (op, offset, FALSE, FALSE, FALSE));
              offset++;
            }
        }

      freeAsmop (op, NULL, ic, TRUE);
    }
}

/*-----------------------------------------------------------------*/
/* genCritical - generate code for start of a critical sequence    */
/*-----------------------------------------------------------------*/
static void
genCritical (iCode * ic)
{
  symbol *tlbl = newiTempLabel (NULL);

  D (emitcode (";", "genCritical"));

  if (IC_RESULT (ic))
    {
      aopOp (IC_RESULT (ic), ic, TRUE, FALSE);
      aopPut (IC_RESULT (ic), one, 0);  /* save old ea in an operand */
      emitcode ("jbc", "ea,!tlabel", labelKey2num (tlbl->key)); /* atomic test & clear */
      aopPut (IC_RESULT (ic), zero, 0);
      emitLabel (tlbl);
      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }
  else
    {
      emitcode ("setb", "c");
      emitcode ("jbc", "ea,!tlabel", labelKey2num (tlbl->key)); /* atomic test & clear */
      emitcode ("clr", "c");
      emitLabel (tlbl);
      emitpush ("psw");         /* save old ea via c in psw on top of stack */
    }
}

/*-----------------------------------------------------------------*/
/* genEndCritical - generate code for end of a critical sequence   */
/*-----------------------------------------------------------------*/
static void
genEndCritical (iCode * ic)
{
  D (emitcode (";", "genEndCritical"));

  if (IC_RIGHT (ic))
    {
      aopOp (IC_RIGHT (ic), ic, FALSE, FALSE);
      if (AOP_TYPE (IC_RIGHT (ic)) == AOP_CRY)
        {
          emitcode ("mov", "c,%s", IC_RIGHT (ic)->aop->aopu.aop_dir);
          emitcode ("mov", "ea,c");
        }
      else
        {
          MOVA (aopGet (IC_RIGHT (ic), 0, FALSE, FALSE, FALSE));
          emitcode ("rrc", "a");
          emitcode ("mov", "ea,c");
        }
      freeAsmop (IC_RIGHT (ic), NULL, ic, TRUE);
    }
  else
    {
      emitpop ("psw");          /* restore ea via c in psw on top of stack */
      emitcode ("mov", "ea,c");
    }
}



/*-----------------------------------------------------------------*/
/* genBuiltIn - calls the appropriate function to  generating code */
/* for a built in function                                         */
/*-----------------------------------------------------------------*/
static void
genBuiltIn (iCode * ic)
{
  operand *bi_parms[MAX_BUILTIN_ARGS];
  int nbi_parms;
  iCode *bi_iCode;
  symbol *bif;

  /* get all the arguments for a built in function */
  bi_iCode = getBuiltinParms (ic, &nbi_parms, bi_parms);

  /* which function is it */
  bif = OP_SYMBOL (IC_LEFT (bi_iCode));
  if (strcmp (bif->name, "__builtin_memcpy_x2x") == 0)
    {
      genMemcpyX2X (bi_iCode, nbi_parms, bi_parms, 0);
    }
  else if (strcmp (bif->name, "__builtin_memcpy_c2x") == 0)
    {
      genMemcpyX2X (bi_iCode, nbi_parms, bi_parms, 1);
    }
  else if (strcmp (bif->name, "__builtin_memcmp_x2x") == 0)
    {
      genMemcmpX2X (bi_iCode, nbi_parms, bi_parms, 0);
    }
  else if (strcmp (bif->name, "__builtin_memcmp_c2x") == 0)
    {
      genMemcmpX2X (bi_iCode, nbi_parms, bi_parms, 1);
    }
  else if (strcmp (bif->name, "__builtin_memset_x") == 0)
    {
      genMemsetX (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "__builtin_inp") == 0)
    {
      genInp (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "__builtin_outp") == 0)
    {
      genOutp (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "__builtin_swapw") == 0)
    {
      genSwapW (bi_iCode, nbi_parms, bi_parms);
      /* JavaNative builtIns */
    }
  else if (strcmp (bif->name, "NatLib_LoadByte") == 0)
    {
      genNatLibLoadPrimitive (bi_iCode, nbi_parms, bi_parms, 1);
    }
  else if (strcmp (bif->name, "NatLib_LoadShort") == 0)
    {
      genNatLibLoadPrimitive (bi_iCode, nbi_parms, bi_parms, 2);
    }
  else if (strcmp (bif->name, "NatLib_LoadInt") == 0)
    {
      genNatLibLoadPrimitive (bi_iCode, nbi_parms, bi_parms, 4);
    }
  else if (strcmp (bif->name, "NatLib_LoadPointer") == 0)
    {
      genNatLibLoadPointer (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "NatLib_InstallImmutableStateBlock") == 0)
    {
      genNatLibInstallStateBlock (bi_iCode, nbi_parms, bi_parms, "Immutable");
    }
  else if (strcmp (bif->name, "NatLib_InstallEphemeralStateBlock") == 0)
    {
      genNatLibInstallStateBlock (bi_iCode, nbi_parms, bi_parms, "Ephemeral");
    }
  else if (strcmp (bif->name, "NatLib_RemoveImmutableStateBlock") == 0)
    {
      genNatLibRemoveStateBlock (bi_iCode, nbi_parms, "Immutable");
    }
  else if (strcmp (bif->name, "NatLib_RemoveEphemeralStateBlock") == 0)
    {
      genNatLibRemoveStateBlock (bi_iCode, nbi_parms, "Ephemeral");
    }
  else if (strcmp (bif->name, "NatLib_GetImmutableStateBlock") == 0)
    {
      genNatLibGetStateBlock (bi_iCode, nbi_parms, bi_parms, "Immutable");
    }
  else if (strcmp (bif->name, "NatLib_GetEphemeralStateBlock") == 0)
    {
      genNatLibGetStateBlock (bi_iCode, nbi_parms, bi_parms, "Ephemeral");
    }
  else if (strcmp (bif->name, "MM_XMalloc") == 0)
    {
      genMMMalloc (bi_iCode, nbi_parms, bi_parms, 3, "XMalloc");
    }
  else if (strcmp (bif->name, "MM_Malloc") == 0)
    {
      genMMMalloc (bi_iCode, nbi_parms, bi_parms, 2, "Malloc");
    }
  else if (strcmp (bif->name, "MM_ApplicationMalloc") == 0)
    {
      genMMMalloc (bi_iCode, nbi_parms, bi_parms, 2, "ApplicationMalloc");
    }
  else if (strcmp (bif->name, "MM_Free") == 0)
    {
      genMMMalloc (bi_iCode, nbi_parms, bi_parms, 2, "Free");
    }
  else if (strcmp (bif->name, "MM_Deref") == 0)
    {
      genMMDeref (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "MM_UnrestrictedPersist") == 0)
    {
      genMMUnrestrictedPersist (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "System_ExecJavaProcess") == 0)
    {
      genSystemExecJavaProcess (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "System_GetRTCRegisters") == 0)
    {
      genSystemRTCRegisters (bi_iCode, nbi_parms, bi_parms, "Get");
    }
  else if (strcmp (bif->name, "System_SetRTCRegisters") == 0)
    {
      genSystemRTCRegisters (bi_iCode, nbi_parms, bi_parms, "Set");
    }
  else if (strcmp (bif->name, "System_ThreadSleep") == 0)
    {
      genSystemThreadSleep (bi_iCode, nbi_parms, bi_parms, "ThreadSleep");
    }
  else if (strcmp (bif->name, "System_ThreadSleep_ExitCriticalSection") == 0)
    {
      genSystemThreadSleep (bi_iCode, nbi_parms, bi_parms, "ThreadSleep_ExitCriticalSection");
    }
  else if (strcmp (bif->name, "System_ProcessSleep") == 0)
    {
      genSystemThreadSleep (bi_iCode, nbi_parms, bi_parms, "ProcessSleep");
    }
  else if (strcmp (bif->name, "System_ProcessSleep_ExitCriticalSection") == 0)
    {
      genSystemThreadSleep (bi_iCode, nbi_parms, bi_parms, "ProcessSleep_ExitCriticalSection");
    }
  else if (strcmp (bif->name, "System_ThreadResume") == 0)
    {
      genSystemThreadResume (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "System_SaveThread") == 0)
    {
      genSystemThreadResume (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "System_ThreadResume") == 0)
    {
      genSystemThreadResume (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "System_ProcessResume") == 0)
    {
      genSystemProcessResume (bi_iCode, nbi_parms, bi_parms);
    }
  else if (strcmp (bif->name, "System_SaveJavaThreadState") == 0)
    {
      genSystem (bi_iCode, nbi_parms, "SaveJavaThreadState");
    }
  else if (strcmp (bif->name, "System_RestoreJavaThreadState") == 0)
    {
      genSystem (bi_iCode, nbi_parms, "RestoreJavaThreadState");
    }
  else if (strcmp (bif->name, "System_ProcessYield") == 0)
    {
      genSystem (bi_iCode, nbi_parms, "ProcessYield");
    }
  else if (strcmp (bif->name, "System_ProcessSuspend") == 0)
    {
      genSystem (bi_iCode, nbi_parms, "ProcessSuspend");
    }
  else if (strcmp (bif->name, "System_RegisterPoll") == 0)
    {
      genSystemPoll (bi_iCode, nbi_parms, bi_parms, "Register");
    }
  else if (strcmp (bif->name, "System_RemovePoll") == 0)
    {
      genSystemPoll (bi_iCode, nbi_parms, bi_parms, "Remove");
    }
  else if (strcmp (bif->name, "System_GetCurrentThreadId") == 0)
    {
      genSystemGetCurrentID (bi_iCode, nbi_parms, bi_parms, "Thread");
    }
  else if (strcmp (bif->name, "System_GetCurrentProcessId") == 0)
    {
      genSystemGetCurrentID (bi_iCode, nbi_parms, bi_parms, "Process");
    }
  else
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "unknown builtin function encountered\n");
      return;
    }
  return;
}

/*-----------------------------------------------------------------*/
/* gen390Code - generate code for Dallas 390 based controllers     */
/*-----------------------------------------------------------------*/
void
gen390Code (iCode * lic)
{
  iCode *ic;
  int cln = 0;

  _G.currentFunc = NULL;

  dptrn[1][0] = "dpl1";
  dptrn[1][1] = "dph1";
  dptrn[1][2] = "dpx1";

  if (options.model == MODEL_FLAT24)
    {
      fReturnSizeDS390 = 5;
      fReturn = fReturn24;
    }
  else
    {
      fReturnSizeDS390 = 4;
      fReturn = fReturn16;
      options.stack10bit = 0;
    }
  /* print the allocation information */
  if (allocInfo && currFunc)
    printAllocInfo (currFunc, codeOutBuf);
  /* if debug information required */
  if (options.debug && currFunc)
    {
      debugFile->writeFunction (currFunc, lic);
    }
  /* stack pointer name */
  if (options.useXstack)
    spname = "_spx";
  else
    spname = "sp";

  for (ic = lic; ic; ic = ic->next)
    {
      initGenLineElement ();

      genLine.lineElement.ic = ic;

      if (ic->lineno && cln != ic->lineno)
        {
          if (options.debug)
            {
              debugFile->writeCLine (ic);
            }
          if (!options.noCcodeInAsm)
            {
              emitcode (";", "%s:%d: %s", ic->filename, ic->lineno, printCLine (ic->filename, ic->lineno));
            }
          cln = ic->lineno;
        }
      if (options.iCodeInAsm)
        {
          const char *iLine;
          iLine = printILine (ic);
          emitcode (";", "ic:%d: %s", ic->key, iLine);
          dbuf_free (iLine);
        }
      /* if the result is marked as
         spilt and rematerializable or code for
         this has already been generated then
         do nothing */
      if (resultRemat (ic) || ic->generated)
        continue;

      /* depending on the operation */
      switch (ic->op)
        {
        case '!':
          genNot (ic);
          break;

        case '~':
          genCpl (ic);
          break;

        case UNARYMINUS:
          genUminus (ic);
          break;

        case IPUSH:
          genIpush (ic);
          break;

        case IPOP:
        {
          iCode *ifxIc, *popIc;
          bool CommonRegs = FALSE;

          /* IPOP happens only when trying to restore a
             spilt live range, if there is an ifx statement
             following this pop (or several) then the if statement might
             be using some of the registers being popped which
             would destroy the contents of the register so
             we need to check for this condition and handle it */
          for (ifxIc = ic->next; ifxIc && ifxIc->op == IPOP; ifxIc = ifxIc->next);
          for (popIc = ic; popIc && popIc->op == IPOP; popIc = popIc->next)
            CommonRegs |= (ifxIc && ifxIc->op == IFX && !ifxIc->generated && regsInCommon (IC_LEFT (popIc), IC_COND (ifxIc)));
          if (CommonRegs)
            genIfx (ifxIc, ic);
          else
            genIpop (ic);
        }
        break;

        case CALL:
          genCall (ic);
          break;

        case PCALL:
          genPcall (ic);
          break;

        case FUNCTION:
          genFunction (ic);
          break;

        case ENDFUNCTION:
          genEndFunction (ic);
          break;

        case RETURN:
          genRet (ic);
          break;

        case LABEL:
          genLabel (ic);
          break;

        case GOTO:
          genGoto (ic);
          break;

        case '+':
          genPlus (ic);
          break;

        case '-':
          if (!genDjnz (ic, ifxForOp (IC_RESULT (ic), ic)))
            genMinus (ic);
          break;

        case '*':
          genMult (ic);
          break;

        case '/':
          genDiv (ic);
          break;

        case '%':
          genMod (ic);
          break;

        case '>':
          genCmpGt (ic, ifxForOp (IC_RESULT (ic), ic));
          break;

        case '<':
          genCmpLt (ic, ifxForOp (IC_RESULT (ic), ic));
          break;

        case LE_OP:
        case GE_OP:
        case NE_OP:

          /* note these two are xlated by algebraic equivalence
             in decorateType() in SDCCast.c */
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "got '>=' or '<=' shouldn't have come here");
          break;

        case EQ_OP:
          genCmpEq (ic, ifxForOp (IC_RESULT (ic), ic));
          break;

        case AND_OP:
          genAndOp (ic);
          break;

        case OR_OP:
          genOrOp (ic);
          break;

        case '^':
          genXor (ic, ifxForOp (IC_RESULT (ic), ic));
          break;

        case '|':
          genOr (ic, ifxForOp (IC_RESULT (ic), ic));
          break;

        case BITWISEAND:
          genAnd (ic, ifxForOp (IC_RESULT (ic), ic));
          break;

        case INLINEASM:
          genInline (ic);
          break;

        case RRC:
          genRRC (ic);
          break;

        case RLC:
          genRLC (ic);
          break;

        case GETHBIT:
          genGetHbit (ic);
          break;

        case LEFT_OP:
          genLeftShift (ic);
          break;

        case RIGHT_OP:
          genRightShift (ic);
          break;

        case GET_VALUE_AT_ADDRESS:
          genPointerGet (ic, hasInc (IC_LEFT (ic), ic, getSize (operandType (IC_RESULT (ic)))));
          break;

        case '=':
          if (POINTER_SET (ic))
            genPointerSet (ic, hasInc (IC_RESULT (ic), ic, getSize (operandType (IC_RIGHT (ic)))));
          else
            genAssign (ic);
          break;

        case IFX:
          genIfx (ic, NULL);
          break;

        case ADDRESS_OF:
          genAddrOf (ic);
          break;

        case JUMPTABLE:
          genJumpTab (ic);
          break;

        case CAST:
          genCast (ic);
          break;

        case RECEIVE:
          genReceive (ic);
          break;

        case SEND:
          if (ic->builtinSEND)
            genBuiltIn (ic);
          else
            addSet (&_G.sendSet, ic);
          break;

        case DUMMY_READ_VOLATILE:
          genDummyRead (ic);
          break;

        case CRITICAL:
          genCritical (ic);
          break;

        case ENDCRITICAL:
          genEndCritical (ic);
          break;

        case SWAP:
          genSwap (ic);
          break;

        default:
          /* This should never happen, right? */
          fprintf (stderr, "*** Probable error: unsupported op 0x%x (%c) in %s @ %d\n", ic->op, ic->op, __FILE__, __LINE__);
          ic = ic;
        }
    }

  /* now we are ready to call the
     peep hole optimizer */
  if (!options.nopeep)
    peepHole (&genLine.lineHead);

  /* now do the actual printing */
  printLine (genLine.lineHead, codeOutBuf);

  /* destroy the line list */
  destroy_line_list ();
}
