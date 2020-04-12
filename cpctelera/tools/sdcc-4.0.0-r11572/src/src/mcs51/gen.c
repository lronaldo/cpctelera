/*-------------------------------------------------------------------------
  gen.c - source file for code generation for 8051

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Bug Fixes  -  Wojciech Stryjewski  wstryj1@tiger.lsu.edu (1999 v2.1.9a)

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
-------------------------------------------------------------------*/
/*
  Notes:
  000123 mlh  Moved aopLiteral to SDCCglue.c to help the split
              Made everything static
*/

#define D(x) do if (options.verboseAsm) {x;} while(0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "ralloc.h"
#include "rtrack.h"
#include "gen.h"
#include "dbuf_string.h"

char *aopLiteralGptr (const char *name, value * val);
extern int allocInfo;

/* this is the down and dirty file with all kinds of
   kludgy & hacky stuff. This is what it is all about
   CODE GENERATION for a specific MCU . some of the
   routines may be reusable, will have to see */

static char *zero = "#0x00";
static char *one = "#0x01";
static char *spname;

char *fReturn8051[] = { "dpl", "dph", "b", "a", "r4", "r5", "r6", "r7" };

unsigned fReturnSizeMCS51 = 4;  /* shared with ralloc.c */
char **fReturn = fReturn8051;
static char *accUse[] = { "a", "b" };

static short rbank = -1;

#define REG_WITH_INDEX   mcs51_regWithIdx

#define AOP(op) op->aop
#define AOP_TYPE(op) AOP(op)->type
#define AOP_SIZE(op) AOP(op)->size
#define IS_AOP_PREG(x) (AOP(x) && (AOP_TYPE(x) == AOP_R1 || \
                                   AOP_TYPE(x) == AOP_R0))

#define AOP_NEEDSACC(x) (AOP(x) && (AOP_TYPE(x) == AOP_CRY ||  \
                                    AOP_TYPE(x) == AOP_DPTR || \
                                    AOP(x)->paged))

#define AOP_INPREG(x) (x && (x->type == AOP_REG &&                        \
                       (x->aopu.aop_reg[0] == REG_WITH_INDEX(R0_IDX) || \
                        x->aopu.aop_reg[0] == REG_WITH_INDEX(R1_IDX) )))

#define IS_AOP_IMMEDIATE(x) (AOP(x) && (AOP_TYPE(x) == AOP_LIT || \
                                        AOP_TYPE(x) == AOP_IMMD || \
                                        AOP_TYPE(x) == AOP_STR))

#define SP_BP(sp, bp) (options.omitFramePtr ? sp : bp)
#define SYM_BP(sym)   (SPEC_OCLS (sym->etype)->paged ? SP_BP("_spx", "_bpx") : SP_BP("sp", "_bp"))

#define EQ(a, b)      (strcmp (a, b) == 0)

#define R0INB   _G.bu.bs.r0InB
#define R1INB   _G.bu.bs.r1InB
#define OPINB   _G.bu.bs.OpInB
#define BITSINB _G.bu.bs.bitsInB
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
      short bitsInB: 2;         //2 so we can see it overflow
    } bs;
    short BInUse;
  } bu;
  short accInUse;
  struct
  {
    int pushed;
    int pushedregs;
    int offset;
    int param_offset;
    int xpushed;
    int xpushedregs;
    int xoffset;
  } stack;
  set *sendSet;
  symbol *currentFunc;
}
_G;

static char *rb1regs[] =
{
  "b1_0", "b1_1", "b1_2", "b1_3", "b1_4", "b1_5", "b1_6", "b1_7",
  "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7"
};

extern struct dbuf_s *codeOutBuf;

#define RESULTONSTACK(x) \
                         (IC_RESULT (x) && IC_RESULT (x)->aop && \
                         IC_RESULT (x)->aop->type == AOP_STK )

#define MOVA(x)  mova (x)       /* use function to avoid multiple eval */
#define MOVB(x)  movb (x)

#define CLRC     emitcode ("clr","c")
#define SETC     emitcode ("setb","c")

static unsigned char SLMask[] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0,
                                  0xE0, 0xC0, 0x80, 0x00
                                };

static unsigned char SRMask[] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F,
                                  0x07, 0x03, 0x01, 0x00
                                };

#define MAX_REGISTER_BANKS 4

#define LSB     0
#define MSB16   1
#define MSB24   2
#define MSB32   3

/*-----------------------------------------------------------------*/
/* mcs51_emitDebuggerSymbol - associate the current code location  */
/*   with a debugger symbol                                        */
/*-----------------------------------------------------------------*/
void
mcs51_emitDebuggerSymbol (const char *debugSym)
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

  /* if it is a literal mov try to get it cheaper */
  if (*x == '#' && rtrackMoveALit (x))
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

  /* if it is a literal mov try to get it cheaper */
  if (*x == '#')
    {
      emitcode ("mov", "b,%s", rtrackGetLit (x));
      return;
    }

  emitcode ("mov", "b,%s", x);
}

/*-----------------------------------------------------------------*/
/* emitpush - push something on internal stack                     */
/*-----------------------------------------------------------------*/
static void
emitpush (const char *arg)
{
  char buf[] = "ar?";

  _G.stack.pushed++;
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
  _G.stack.pushed--;
  wassertl (_G.stack.pushed >= 0, "stack underflow");
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

#if 0
/*-----------------------------------------------------------------*/
/* showR0R1status - helper for debugging getFreePtr failures       */
/*-----------------------------------------------------------------*/
static void
showR0R1status(iCode * ic)
{
  bool r0iu, r1iu;
  bool r0ou, r1ou;

  r0iu = bitVectBitValue (ic->rUsed, R0_IDX);
  r1iu = bitVectBitValue (ic->rUsed, R1_IDX);
  printf ("ic->rUsed = [");
  if (r0iu)
    if (r1iu)
      printf("r0,r1");
    else
      printf("r0");
  else
    if (r1iu)
      printf("r1");
  printf("] ");
  
  r0ou = bitVectBitValue (ic->rMask, R0_IDX);
  r1ou = bitVectBitValue (ic->rMask, R1_IDX);
  printf ("ic->rMask = [");
  if (r0ou)
    if (r1ou)
      printf("r0,r1");
    else
      printf("r0");
  else
    if (r1ou)
      printf("r1");
  printf("]\n");
}
#endif

/*-----------------------------------------------------------------*/
/* getFreePtr - returns r0 or r1 whichever is free or can be pushed */
/*-----------------------------------------------------------------*/
static reg_info *
getFreePtr (iCode * ic, asmop * aop, bool result)
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
      aop->type = AOP_R0;

      return aop->aopu.aop_ptr = REG_WITH_INDEX (R0_IDX);
    }

  /* if no usage of r1 then return it */
  if (!r1iu && !r1ou)
    {
      ic->rUsed = bitVectSetBit (ic->rUsed, R1_IDX);
      aop->type = AOP_R1;

      return aop->aopu.aop_ptr = REG_WITH_INDEX (R1_IDX);
    }

  /* now we know they both have usage */
  /* if r0 not used in this instruction */
  if (!r0iu)
    {
      /* push it if not already pushed */
      if ((ic->op == IPUSH) || (ic->op == PCALL))
        {
          MOVB (REG_WITH_INDEX (R0_IDX)->dname);
          R0INB++;
        }
      else if (!_G.r0Pushed)
        {
          emitpush (REG_WITH_INDEX (R0_IDX)->dname);
          _G.r0Pushed++;
        }

      ic->rUsed = bitVectSetBit (ic->rUsed, R0_IDX);
      aop->type = AOP_R0;

      return aop->aopu.aop_ptr = REG_WITH_INDEX (R0_IDX);
    }

  /* if r1 not used then */

  if (!r1iu)
    {
      /* push it if not already pushed */
      if ((ic->op == IPUSH) || (ic->op == PCALL))
        {
          MOVB (REG_WITH_INDEX (R1_IDX)->dname);
          R1INB++;
        }
      else if (!_G.r1Pushed)
        {
          emitpush (REG_WITH_INDEX (R1_IDX)->dname);
          _G.r1Pushed++;
        }

      ic->rUsed = bitVectSetBit (ic->rUsed, R1_IDX);
      aop->type = AOP_R1;
      return REG_WITH_INDEX (R1_IDX);
    }

endOfWorld:
  /* I said end of world, but not quite end of world yet */
  /* if this is a result then we can push it on the stack */
  if (result)
    {
      aop->type = AOP_STK;
      return NULL;
    }
  /* in the case that result AND left AND right needs a pointer reg
     we can safely use the result's */
  if (bitVectBitValue (mcs51_rUmaskForOp (IC_RESULT (ic)), R0_IDX) &&
    (!OP_SYMBOL (IC_RESULT (ic)) || OP_SYMBOL (IC_RESULT (ic))->regs[getSize (operandType (IC_RESULT (ic))) - 1]->rIdx == R0_IDX))
    {
      aop->type = AOP_R0;
      return REG_WITH_INDEX (R0_IDX);
    }
  if (bitVectBitValue (mcs51_rUmaskForOp (IC_RESULT (ic)), R1_IDX) &&
    (!OP_SYMBOL (IC_RESULT (ic)) || OP_SYMBOL (IC_RESULT (ic))->regs[getSize (operandType (IC_RESULT (ic))) - 1]->rIdx == R1_IDX))
    {
      aop->type = AOP_R1;
      return REG_WITH_INDEX (R1_IDX);
    }

  /* now this is REALLY the end of the world */
  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "getFreePtr should never reach here");
  exit (EXIT_FAILURE);
}


/*-----------------------------------------------------------------*/
/* getTempRegs - initialize an array of pointers to GPR registers  */
/*               that are not in use. Returns 1 if the requested   */
/*               number of registers were available, 0 otherwise.  */
/*-----------------------------------------------------------------*/
int
getTempRegs (reg_info ** tempRegs, int size, iCode * ic)
{
  bitVect *freeRegs;
  int i;
  int offset;

  if (!ic)
    ic = genLine.lineElement.ic;
  if (!ic)
    return 0;
  if (!_G.currentFunc)
    return 0;

  freeRegs = newBitVect (8);
  bitVectSetBit (freeRegs, R2_IDX);
  bitVectSetBit (freeRegs, R3_IDX);
  bitVectSetBit (freeRegs, R4_IDX);
  bitVectSetBit (freeRegs, R5_IDX);
  bitVectSetBit (freeRegs, R6_IDX);
  bitVectSetBit (freeRegs, R7_IDX);

  if (IFFUNC_CALLEESAVES (_G.currentFunc->type))
    {
      bitVect *newfreeRegs;
      newfreeRegs = bitVectIntersect (freeRegs, _G.currentFunc->regsUsed);
      freeBitVect (freeRegs);
      freeRegs = newfreeRegs;
    }
  freeRegs = bitVectCplAnd (freeRegs, ic->rMask);

  offset = 0;
  for (i = 0; i < freeRegs->size; i++)
    {
      if (bitVectBitValue (freeRegs, i))
        tempRegs[offset++] = REG_WITH_INDEX (i);
      if (offset >= size)
        {
          freeBitVect (freeRegs);
          return 1;
        }
    }

  freeBitVect (freeRegs);
  return 0;
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
/* stackoffset - stack offset for symbol                           */
/*-----------------------------------------------------------------*/
static int
stackoffset (symbol * sym)
{
  int offset = sym->stack;
  if (options.omitFramePtr)
    {
      if (SPEC_OCLS (sym->etype)->paged)
        offset -= _G.stack.xoffset + _G.stack.xpushed;
      else
        offset -= _G.stack.offset + _G.stack.pushed;
    }
  if (sym->stack < 0)
    offset -= _G.stack.param_offset;
  return offset;
}

/*-----------------------------------------------------------------*/
/* aopPtrForSym - pointer for symbol                               */
/*-----------------------------------------------------------------*/
static void
aopPtrForSym (symbol * sym, bool accuse, int offset, asmop * aop, iCode * ic)
{
  char *base;
  struct dbuf_s tmpBuf;
  dbuf_init (&tmpBuf, 1024);
  if (sym->onStack)
    {
      dbuf_printf (&tmpBuf, "%s", SYM_BP (sym));
    }
  else
    {
      dbuf_printf (&tmpBuf, "#%s", sym->rname);
    }
  base = dbuf_detach_c_str (&tmpBuf);

  offset += stackoffset (sym);

  if (abs (offset) >= 248)
    werrorfl (ic->filename, ic->lineno, W_LIT_OVERFLOW);

  if ((abs (offset) < 3) || (accuse && (abs (offset) < 4)))
    {
      emitcode ("mov", "%s,%s", aop->aopu.aop_ptr->name, base);
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
        {
          emitcode ("xch", "a,%s", aop->aopu.aop_ptr->name);
          emitcode ("mov", "a,%s", base);
          emitcode ("add", "a,#0x%02x", offset & 0xff);
          emitcode ("xch", "a,%s", aop->aopu.aop_ptr->name);
        }
      else
        {
          emitcode ("mov", "a,%s", base);
          emitcode ("add", "a,#0x%02x", offset & 0xff);
          emitcode ("mov", "%s,a", aop->aopu.aop_ptr->name);
        }
    }
  aop->paged = SPEC_OCLS (sym->etype)->paged;
  dbuf_free (base);
}

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (iCode * ic, symbol * sym, bool result)
{
  asmop *aop;
  memmap *space;
  bool accuse = leftRightUseAcc (ic) || _G.accInUse;

  wassertl (ic != NULL, "Got a null iCode");
  wassertl (sym != NULL, "Got a null symbol");

  space = SPEC_OCLS (sym->etype);

  /* if already has one */
  if (sym->aop)
    {
      sym->aop->allocated++;
      return sym->aop;
    }

  /* assign depending on the storage class */
  /* if it is on the stack or indirectly addressable */
  /* space we need to assign either r0 or r1 to it   */
  if (sym->onStack || sym->iaccess)
    {
      sym->aop = aop = newAsmop (0);
      aop->aopu.aop_ptr = getFreePtr (ic, aop, result);
      aop->size = getSize (sym->type);

      /* now assign the address of the variable to
         the pointer register */
      if (aop->type != AOP_STK)
        {
          aopPtrForSym (sym, accuse, 0, aop, ic);
        }
      else
        {
          aop->aopu.aop_sym = sym;
        }
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
      //printf("aopForSym, using AOP_DIR for %s (%x)\n", sym->name, sym);
      //printTypeChainRaw(sym->type, NULL);
      //printf("space = %s\n", space ? space->sname : "NULL");
      sym->aop = aop = newAsmop (AOP_DIR);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
      return aop;
    }

  /* special case for a function */
  if (IS_FUNC (sym->type))
    {
      sym->aop = aop = newAsmop (AOP_IMMD);
      aop->aopu.aop_immd.aop_immd1 = Safe_strdup (sym->rname);
      aop->size = getSize (sym->type);
      return aop;
    }

  /* only remaining is far space */
  /* in which case DPTR gets the address */
  sym->aop = aop = newAsmop (AOP_DPTR);

  rtrackLoadDptrWithSym (sym->rname);

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
  sym_link *from_type = NULL;
  const char *from_name = NULL;
  struct dbuf_s dbuf;

  for (;;)
    {
      if (ic->op == '+')
        {
          val += (int) operandLitValue (IC_RIGHT (ic));
          ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
        }
      else if (ic->op == '-')
        {
          val -= (int) operandLitValue (IC_RIGHT (ic));
          ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
        }
      else if (IS_CAST_ICODE (ic))
        {
          from_type = operandType (IC_RIGHT (ic));
          from_name = IS_SYMOP (IC_RIGHT (ic)) ? OP_SYMBOL (IC_RIGHT (ic))->name : NULL;
          aop->aopu.aop_immd.from_cast_remat = 1;
          ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
        }
      else
        {
          break;
        }
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

  aop->aopu.aop_immd.aop_immd1 = dbuf_detach_c_str (&dbuf);
  /* set immd2 field if required */
  if (aop->aopu.aop_immd.from_cast_remat)
    {
      ptr_type = pointerTypeToGPByte (DCL_TYPE (from_type), from_name, sym->name);
      dbuf_init (&dbuf, 128);
      dbuf_tprintf (&dbuf, "#!constbyte", ptr_type);
      aop->aopu.aop_immd.aop_immd2 = dbuf_detach_c_str (&dbuf);
    }

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
    return TRUE;

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
aopOp (operand * op, iCode * ic, bool result)
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
      op->aop->allocated++;
      return;
    }

  /* if the underlying symbol has a aop */
  if (IS_SYMOP (op) && OP_SYMBOL (op)->aop)
    {
      op->aop = OP_SYMBOL (op)->aop;
      op->aop->allocated++;
      return;
    }

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aopForSym (ic, OP_SYMBOL (op), result);
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
      aop->size = sym->ruonly ? 1 : 0;
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

          sym->aop = op->aop = aop = newAsmop (AOP_STR);
          aop->size = getSize (sym->type);
          for (i = 0; i < fReturnSizeMCS51; i++)
            aop->aopu.aop_str[i] = fReturn[i];
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
          sym->aop = op->aop = aop = aopForSym (ic, sym->usl.spillLoc, result);
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
  if (sym->regType == REG_BIT && sym->regs[0]->type == REG_BIT)
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
  int sz;
  symbol *sym;

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
      if (R0INB)
        {
          emitcode ("mov", "r0,b");
          R0INB--;
        }
      else if (_G.r0Pushed)
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
      if (R1INB)
        {
          emitcode ("mov", "r1,b");
          R1INB--;
        }
      else if (_G.r1Pushed)
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
      sz = aop->size;
      sym = aop->aopu.aop_sym;
      bitVectUnSetBit (ic->rUsed, R0_IDX);
      bitVectUnSetBit (ic->rUsed, R1_IDX);

      getFreePtr (ic, aop, FALSE);

      aopPtrForSym (sym, FALSE, aop->size - 1, aop, ic);

      while (sz--)
        {
          emitpop ("acc");
          if (aop->paged)
            emitcode ("movx", "@%s,a", aop->aopu.aop_ptr->name);
          else
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

/*------------------------------------------------------------------*/
/* freeForBranchAsmop - partial free up of Asmop for a branch; just */
/*                      pop r0 or r1 off stack if pushed            */
/*------------------------------------------------------------------*/
static void
freeForBranchAsmop (operand * op, iCode * ic)
{
  asmop *aop;

  if (!op)
    return;

  aop = op->aop;

  if (!aop)
    return;

  if (!aop->allocated)
    return;

  switch (aop->type)
    {
    case AOP_R0:
      if (R0INB)
        {
          emitcode ("mov", "r0,b");
        }
      else if (_G.r0Pushed)
        {
          emitcode ("pop", "ar0");      /* without pushed-- */
        }
      break;

    case AOP_R1:
      if (R1INB)
        {
          emitcode ("mov", "r1,b");
        }
      else if (_G.r1Pushed)
        {
          emitcode ("pop", "ar1");      /* without pushed-- */
        }
      break;

    case AOP_STK:
    {
      int sz = aop->size;

      emitcode ("mov", "b,r0");
      aopPtrForSym (aop->aopu.aop_sym, FALSE, 0, aop, ic);

      while (sz--)
        {
          emitcode ("pop", "acc");    /* without pushed-- */
          if (aop->paged)
            emitcode ("movx", "@r0,a");
          else
            emitcode ("mov", "@r0,a");
          if (!sz)
            break;
          emitcode ("dec", "r0");
        }
      emitcode ("mov", "r0,b");
    }
    }
}

/*------------------------------------------------------------------*/
/* freeForBranchAsmops - partial free up of 3 Asmops for a branch;  */
/*                       just pop r0 or r1 off stack if pushed      */
/*------------------------------------------------------------------*/
static void
freeForBranchAsmops (operand * op1, operand * op2, operand * op3, iCode * ic)
{
  if (op1)
    freeForBranchAsmop (op1, ic);
  if (op2)
    freeForBranchAsmop (op2, ic);
  if (op3)
    freeForBranchAsmop (op3, ic);
}

/*-----------------------------------------------------------------*/
/* opIsGptr: returns non-zero if the passed operand is             */
/* a generic pointer type.                                         */
/*-----------------------------------------------------------------*/
static int
opIsGptr (operand * op)
{
  if (op && (AOP_SIZE (op) == GPTRSIZE) && (IS_GENPTR (operandType (op)) || IFFUNC_ISBANKEDCALL (operandType (op))))
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
/*-------------------------------------------------------------------*/
/*
 * NOTE: function returns a pointer to a reusable dynamically allocated
 * buffer, which should never be freed!
 * Subsequent call to aopGet() will rewrite the result of the previous
 * call, so the content of the result should be copied to an other
 * location, usually using Safe_strdup(), in order to perserve it.
 */
static const char *
aopGet (operand * oper, int offset, bool bit16, bool dname)
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

        case AOP_DPTR:
          if (aop->code && aop->coff == 0 && offset >= 1)
            {
              emitcode ("mov", "a,#0x%02x", offset);
              emitcode ("movc", "a,@a+dptr");
            }
          else
            {
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
            }
          dbuf_append_str (&dbuf, dname ? "acc" : "a");
          break;

        case AOP_IMMD:
          if (aop->aopu.aop_immd.from_cast_remat && opIsGptr (oper) && offset == GPTRSIZE - 1)
            {
              dbuf_printf (&dbuf, "%s", aop->aopu.aop_immd.aop_immd2);
            }
          else if (bit16)
            {
              dbuf_printf (&dbuf, "#%s", aop->aopu.aop_immd.aop_immd1);
            }
          else if (offset)
            {
              dbuf_printf (&dbuf, "#(%s >> %d)", aop->aopu.aop_immd.aop_immd1, offset * 8);
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
          if (!IS_OP_RUONLY (oper))
            emitcode ("mov", "c,%s", aop->aopu.aop_dir);
          emitcode ("clr", "a");
          emitcode ("rlc", "a");
          dbuf_append_str (&dbuf, dname ? "acc" : "a");
          break;

        case AOP_ACC:
          dbuf_append_str (&dbuf, (!offset && dname) ? "acc" : aop->aopu.aop_str[offset]);
          break;

        case AOP_LIT:
          if (opIsGptr (oper) && IS_FUNCPTR (operandType (oper)) && offset == GPTRSIZE - 1)
            {
              dbuf_append_str (&dbuf, aopLiteralGptr (NULL, aop->aopu.aop_lit));
            }
          else
            {
              int size = 1 + (bit16 ? 1 : 0);
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
    case AOP_DPTR:
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

    case AOP_DPTR:
      if (aop->code)
        {
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut writing to code space");
          exit (EXIT_FAILURE);
        }

      /* if not in accumulator */
      MOVA (s);

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

      emitcode ("movx", "@dptr,a");
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
static void
loadDptrFromOperand (operand *op, bool loadBToo)
{
  if (AOP_TYPE (op) != AOP_STR)
    {
      /* if this is rematerializable */
      if (AOP_TYPE (op) == AOP_IMMD)
        {
          emitcode ("mov", "dptr,%s", aopGet (op, 0, TRUE, FALSE));
          if (loadBToo)
            {
              if (AOP (op)->aopu.aop_immd.from_cast_remat)
                emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE));
              else
                {
                  wassertl (FALSE, "need pointerCode");
                  emitcode (";", "mov b,???");
                  /* genPointerGet and genPointerSet originally did different
                   ** things for this case. Both seem wrong.
                   ** from genPointerGet:
                   **  emitcode ("mov", "b,#%d", pointerCode (retype));
                   ** from genPointerSet:
                   **  emitcode ("mov", "b,%s + 1", aopGet (result, 0, TRUE, FALSE));
                   */
                }
            }
        }
      else if (AOP_TYPE (op) == AOP_LIT)
        {
          emitcode ("mov", "dptr,%s", aopGet (op, 0, TRUE, FALSE));
          if (loadBToo)
            emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE));
        }
      else if (AOP_TYPE (op) == AOP_DPTR)
        {
          emitpush (aopGet (op, 0, FALSE, FALSE));
          if (loadBToo)
            {
              emitpush (aopGet (op, 1, FALSE, FALSE));
              emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE));
              emitpop ("dph");
            }
          else
            {
              emitcode ("mov", "dph,%s", aopGet (op, 1, FALSE, FALSE));
            }
          emitpop ("dpl");
        }
      else
        {
          /* we need to get it byte by byte */
          emitcode ("mov", "dpl,%s", aopGet (op, 0, FALSE, FALSE));
          emitcode ("mov", "dph,%s", aopGet (op, 1, FALSE, FALSE));
          if (loadBToo)
            emitcode ("mov", "b,%s", aopGet (op, AOP_SIZE (op) - 1, FALSE, FALSE));
        }
    }
}

/*-----------------------------------------------------------------*/
/* reAdjustPreg - points a register back to where it should        */
/*-----------------------------------------------------------------*/
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
      while (aop->coff--)
        {
          emitcode ("lcall", "__decdptr");
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
      if (!IS_OP_RUONLY (result) && !IS_OP_ACCUSE (result))
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

  if (opIsGptr (oper))
    {
      /* assumes that banks never map to address 0x0000
         so it suffices to check dptr part only and ignore b */
      size = AOP_SIZE (oper) - 2;
    }
  else
    {
      size = AOP_SIZE (oper) - 1;
    }

  offset = 0;
  if (size && AccUsed && (AOP (oper)->type != AOP_ACC))
    {
      pushedB = pushB ();
      MOVB (aopGet (oper, offset++, FALSE, FALSE));
      while (--size)
        {
          MOVA (aopGet (oper, offset++, FALSE, FALSE));
          emitcode ("orl", "b,a");
        }
      MOVA (aopGet (oper, offset++, FALSE, FALSE));
      if (IS_FLOAT (type))
        emitcode ("anl", "a,#0x7F");    //clear sign bit
      emitcode ("orl", "a,b");
      popB (pushedB);
    }
  else
    {
      MOVA (aopGet (oper, offset++, FALSE, FALSE));
      while (size--)
        {
          emitcode ("orl", "a,%s", aopGet (oper, offset++, FALSE, FALSE));
        }
    }
}

/*-----------------------------------------------------------------*/
/* toCarry - make boolean and move into carry                      */
/*-----------------------------------------------------------------*/
static void
toCarry (operand *oper)
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
  else if (IS_BOOL (operandType (oper)) || IS_BITFIELD (operandType (oper)) && SPEC_BLEN (getSpec (operandType (oper))) == 1) 
    {
      MOVA (aopGet (oper, 0, FALSE, FALSE));
      emitcode ("rrc", "a");
    }
  else
    {
      /* or the operand into a */
      toBoolean (oper);
      /* set C, if a >= 1 */
      emitcode ("add", "a,#0xff");
    }
}

/*-----------------------------------------------------------------*/
/* assignBit - assign operand to bit operand                       */
/*-----------------------------------------------------------------*/
static void
assignBit (operand * result, operand * right)
{
  emitcode (";", "assignBit");
  /* if the right side is a literal then
     we know what the value is */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      if ((int) operandLitValue (right))
        aopPut (result, one, 0);
      else
        aopPut (result, zero, 0);
    }
  else
    {
      toCarry (right);
      outBitC (result);
    }
}

/*-------------------------------------------------------------------*/
/* xch_a_aopGet - for exchanging acc with value of the aop           */
/*-------------------------------------------------------------------*/
static const char *
xch_a_aopGet (operand * oper, int offset, bool bit16, bool dname)
{
  const char *l;

  if (aopGetUsesAcc (oper, offset))
    {
      emitcode ("mov", "b,a");
      MOVA (aopGet (oper, offset, bit16, dname));
      emitcode ("xch", "a,b");
      aopPut (oper, "a", offset);
      emitcode ("xch", "a,b");
      l = "b";
    }
  else
    {
      l = aopGet (oper, offset, bit16, dname);
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
  aopOp (IC_LEFT (ic), ic, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE);

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
  aopOp (IC_LEFT (ic), ic, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE);

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
      l = aopGet (IC_LEFT (ic), offset++, FALSE, FALSE);
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
  while (size--)
    {
      MOVA (aopGet (IC_LEFT (ic), offset, FALSE, FALSE));
      emitcode ("cpl", "a");
      aopPut (IC_RESULT (ic), "a", offset++);
    }

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

  size = AOP_SIZE (op) - 1;

  while (size--)
    {
      aopPut (result, aopGet (op, offset, FALSE, FALSE), offset);
      offset++;
    }

  MOVA (aopGet (op, offset, FALSE, FALSE));

  emitcode ("cpl", "acc.7");
  aopPut (result, "a", offset);
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
  aopOp (IC_LEFT (ic), ic, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE);

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
  while (size--)
    {
      const char *l = aopGet (IC_LEFT (ic), offset, FALSE, FALSE);
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
/* xstackRegisters - create bitmask for registers on xstack        */
/*-----------------------------------------------------------------*/
static int
xstackRegisters (bitVect * rsave, bool push, int count, char szRegs[32])
{
  int i;
  int mask = 0;

  szRegs[0] = '\0';

  for (i = mcs51_nRegs; i >= 0; i--)
    {
      if (bitVectBitValue (rsave, i))
        {
          reg_info *reg = REG_WITH_INDEX (i);
          if (reg->type == REG_BIT)
            {
              mask |= 0x01;
              strncat (szRegs, reg->base, 31);
            }
          else
            {
              if (i == R0_IDX)
                {
                  mask |= 0x100;
                }
              else
                {
                  //set bit(n) for Rn
                  mask |= (0x01 << reg->offset);
                }
              strncat (szRegs, reg->name, 31);
            }
        }
    }
  return mask ^ 0xFF;           //invert all bits for jbc
}

/*-----------------------------------------------------------------*/
/* saveRegisters - will look for a call and save the registers     */
/*-----------------------------------------------------------------*/
static void
saveRegisters (iCode * lic)
{
  int i;
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
      if (IFFUNC_ISNAKED (type) && !IFFUNC_ISBANKEDCALL (type))
        return;
      if (IFFUNC_CALLEESAVES (type))
        return;
    }

  if (IFFUNC_CALLEESAVES (_G.currentFunc->type))
    {
      /* save all registers if the caller is callee_saves and the callee is not */
      rsave = bitVectCopy (mcs51_allBankregs ());
      if (!inExcludeList ("bits"))
        {
          rsave = bitVectUnion (rsave, mcs51_allBitregs ());
          BitBankUsed = 1;
        }
    }
  else
    /* save only the registers in use at this time */
    rsave = bitVectCopy (ic->rMask);
  /* but skip the ones for the result */
  rsave = bitVectCplAnd (rsave, mcs51_rUmaskForOp (IC_RESULT (ic)));

  ic->regsSaved = 1;
  if (options.useXstack)
    {
      bitVect *rsavebits = bitVectIntersect (bitVectCopy (mcs51_allBitregs ()), rsave);
      int nBits = bitVectnBitsOn (rsavebits);
      int count = bitVectnBitsOn (rsave);

      if (nBits != 0)
        {
          count = count - nBits + 1;
          /* remove all but the first bits as they are pushed all at once */
          rsave = bitVectCplAnd (rsave, rsavebits);
          rsave = bitVectSetBit (rsave, bitVectFirstBit (rsavebits));
        }
      freeBitVect (rsavebits);

      if (count == 1)
        {
          reg_info *reg = REG_WITH_INDEX (bitVectFirstBit (rsave));
          emitpush (REG_WITH_INDEX (R0_IDX)->dname);
          if (reg->type == REG_BIT)
            {
              emitcode ("mov", "a,%s", reg->base);
            }
          else
            {
              emitcode ("mov", "a,%s", reg->name);
            }
          emitcode ("mov", "r0,%s", spname);
          emitcode ("inc", "%s", spname);       // allocate before use
          emitcode ("movx", "@r0,a");
          _G.stack.xpushed++;
          emitpop (REG_WITH_INDEX (R0_IDX)->dname);
        }
      else if (count != 0)
        {
          if ((FUNC_REGBANK (currFunc->type) == 0) && optimize.codeSize)
            {
              char szRegs[32];
              int mask = xstackRegisters (rsave, TRUE, count, szRegs);
              if (BINUSE)
                emitpush ("b");
              emitcode ("mov", "a,#0x%02x", count);
              emitcode ("mov", "b,#0x%02x", mask & 0xFF);
              if (mask & 0x100)
                emitcode ("lcall", "___sdcc_xpush_regs_r0\t;(%s)", szRegs);
              else
                emitcode ("lcall", "___sdcc_xpush_regs\t;(%s)", szRegs);
              genLine.lineCurr->isInline = 1;
              if (BINUSE)
                emitpop ("b");
              _G.stack.xpushed += count;
            }
          else
            {
              emitpush (REG_WITH_INDEX (R0_IDX)->dname);
              emitcode ("mov", "r0,%s", spname);
              if (count == 2)
                {
                  emitcode ("inc", "%s", spname);
                  emitcode ("inc", "%s", spname);
                }
              else
                {
                  MOVA ("r0");
                  emitcode ("add", "a,#0x%02x", count);
                  emitcode ("mov", "%s,a", spname);
                }
              for (i = 0; i < mcs51_nRegs; i++)
                {
                  if (bitVectBitValue (rsave, i))
                    {
                      reg_info *reg = REG_WITH_INDEX (i);
                      if (i == R0_IDX)
                        {
                          emitpop ("acc");
                          emitpush ("acc");
                        }
                      else if (reg->type == REG_BIT)
                        {
                          emitcode ("mov", "a,%s", reg->base);
                        }
                      else
                        {
                          emitcode ("mov", "a,%s", reg->name);
                        }
                      emitcode ("movx", "@r0,a");
                      _G.stack.xpushed++;
                      if (--count)
                        {
                          emitcode ("inc", "r0");
                        }
                    }
                }
              emitpop (REG_WITH_INDEX (R0_IDX)->dname);
            }
        }
    }
  else
    {
      bool bits_pushed = FALSE;
      for (i = 0; i < mcs51_nRegs; i++)
        {
          if (bitVectBitValue (rsave, i))
            {
              bits_pushed = pushReg (i, bits_pushed);
            }
        }
    }
  freeBitVect (rsave);
}

/*-----------------------------------------------------------------*/
/* unsaveRegisters - pop the pushed registers                      */
/*-----------------------------------------------------------------*/
static void
unsaveRegisters (iCode * ic)
{
  int i;
  bitVect *rsave;

  if (IFFUNC_CALLEESAVES (_G.currentFunc->type))
    {
      /* restore all registers if the caller is callee_saves and the callee is not */
      rsave = bitVectCopy (mcs51_allBankregs ());
      if (!inExcludeList ("bits"))
        {
          rsave = bitVectUnion (rsave, mcs51_allBitregs ());
          BitBankUsed = 1;
        }
    }
  else
    /* restore only the registers in use at this time */
    rsave = bitVectCopy (ic->rMask);
  /* but skip the ones for the result */
  rsave = bitVectCplAnd (rsave, mcs51_rUmaskForOp (IC_RESULT (ic)));

  if (options.useXstack)
    {
      bitVect *rsavebits = bitVectIntersect (bitVectCopy (mcs51_allBitregs ()), rsave);
      int nBits = bitVectnBitsOn (rsavebits);
      int count = bitVectnBitsOn (rsave);

      if (nBits != 0)
        {
          count = count - nBits + 1;
          /* remove all but the first bits as they are popped all at once */
          rsave = bitVectCplAnd (rsave, rsavebits);
          rsave = bitVectSetBit (rsave, bitVectFirstBit (rsavebits));
        }
      freeBitVect (rsavebits);

      if (count == 1)
        {
          reg_info *reg = REG_WITH_INDEX (bitVectFirstBit (rsave));
          emitcode ("mov", "r0,%s", spname);
          emitcode ("dec", "r0");
          emitcode ("movx", "a,@r0");
          _G.stack.xpushed--;
          if (reg->type == REG_BIT)
            {
              emitcode ("mov", "%s,a", reg->base);
            }
          else
            {
              emitcode ("mov", "%s,a", reg->name);
            }
          emitcode ("dec", "%s", spname);
        }
      else if (count != 0)
        {
          if ((FUNC_REGBANK (currFunc->type) == 0) && optimize.codeSize)
            {
              char szRegs[32];
              int mask = xstackRegisters (rsave, FALSE, count, szRegs);
              emitcode ("mov", "b,#0x%02x", mask & 0xFF);
              if (mask & 0x100)
                emitcode ("lcall", "___sdcc_xpop_regs_r0\t;(%s)", szRegs);
              else
                emitcode ("lcall", "___sdcc_xpop_regs\t;(%s)", szRegs);
              genLine.lineCurr->isInline = 1;
              _G.stack.xpushed -= count;
            }
          else
            {
              bool resultInR0 = bitVectBitValue (mcs51_rUmaskForOp (IC_RESULT (ic)), R0_IDX);
              if (resultInR0)
                {
                  emitpush (REG_WITH_INDEX (R0_IDX)->dname);
                }
              emitcode ("mov", "r0,%s", spname);
              for (i = mcs51_nRegs; i >= 0; i--)
                {
                  if (bitVectBitValue (rsave, i))
                    {
                      reg_info *reg = REG_WITH_INDEX (i);
                      emitcode ("dec", "r0");
                      emitcode ("movx", "a,@r0");
                      _G.stack.xpushed--;
                      if (i == R0_IDX)
                        {
                          emitpush ("acc");
                        }
                      else if (reg->type == REG_BIT)
                        {
                          emitcode ("mov", "%s,a", reg->base);
                        }
                      else
                        {
                          emitcode ("mov", "%s,a", reg->name);
                        }
                    }
                }
              emitcode ("mov", "%s,r0", spname);
              if (bitVectBitValue (rsave, R0_IDX) || resultInR0)
                {
                  emitpop (REG_WITH_INDEX (R0_IDX)->dname);
                }
            }
        }
    }
  else
    {
      bool bits_popped = FALSE;
      for (i = mcs51_nRegs; i >= 0; i--)
        {
          if (bitVectBitValue (rsave, i))
            {
              bits_popped = popReg (i, bits_popped);
            }
        }
    }
  freeBitVect (rsave);
}


/*-----------------------------------------------------------------*/
/* pushSide -                                                      */
/*-----------------------------------------------------------------*/
static void
pushSide (operand * oper, int size, iCode * ic)
{
  int offset = 0;
  int nPushed = _G.r0Pushed + _G.r1Pushed;

  aopOp (oper, ic, FALSE);

  if (nPushed != _G.r0Pushed + _G.r1Pushed)
    {
      while (offset < size)
        {
          const char *l = aopGet (oper, offset, FALSE, TRUE);
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

  while (size--)
    {
      const char *l = aopGet (oper, offset++, FALSE, TRUE);
      if (AOP_TYPE (oper) != AOP_REG && AOP_TYPE (oper) != AOP_DIR)
        {
          MOVA (l);
          emitpush ("acc");
        }
      else
        {
          emitpush (l);
        }
    }

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

  if (func && IS_BIT (getSpec (operandType (func))))
    {
      outBitC (oper);
      return FALSE;
    }
  if ((size > 3) && aopPutUsesAcc (oper, fReturn[offset], offset))
    {
      emitpush ("acc");
      pushedA = TRUE;
    }
  while (size--)
    {
      if ((offset == 3) && pushedA)
        emitpop ("acc");
      accuse |= aopPut (oper, fReturn[offset], offset);
      offset++;
    }
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

  aopOp (IC_LEFT (ic), ic, FALSE);
  r = getFreePtr (ic, aop, FALSE);

  size = AOP_SIZE (IC_LEFT (ic));
  emitcode ("mov", "%s,%s", r->name, spname);

  // allocate space first
  if (size <= 2)
    {
      emitcode ("inc", "%s", spname);
      if (size == 2)
        emitcode ("inc", "%s", spname);
    }
  else
    {
      MOVA (r->name);
      emitcode ("add", "a,#0x%02x", size);
      emitcode ("mov", "%s,a", spname);
    }

  while (offset < size)
    {
      MOVA (aopGet (IC_LEFT (ic), offset++, FALSE, FALSE));
      emitcode ("movx", "@%s,a", r->name);
      emitcode ("inc", "%s", r->name);
    }
  _G.stack.xpushed += size;

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
      if (OP_SYMBOL (IC_LEFT (ic))->isspilt)
        return;

      aopOp (IC_LEFT (ic), ic, FALSE);
      size = AOP_SIZE (IC_LEFT (ic));
      /* push it on the stack */
      while (size--)
        {
          emitpush (aopGet (IC_LEFT (ic), offset++, FALSE, TRUE));
        }
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
  aopOp (IC_LEFT (ic), ic, FALSE);

  // pushSide(IC_LEFT(ic), AOP_SIZE(IC_LEFT(ic)));
  size = AOP_SIZE (IC_LEFT (ic));

  prev = Safe_strdup ("");
  while (size--)
    {
      const char *l = aopGet (IC_LEFT (ic), offset++, FALSE, TRUE);
      if (AOP_TYPE (IC_LEFT (ic)) != AOP_REG && AOP_TYPE (IC_LEFT (ic)) != AOP_DIR)
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
  if (OP_SYMBOL (IC_LEFT (ic))->isspilt)
    return;

  aopOp (IC_LEFT (ic), ic, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));
  offset = size - 1;
  while (size--)
    {
      emitpop (aopGet (IC_LEFT (ic), offset--, FALSE, TRUE));
    }

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
      int pushed = _G.stack.pushed;
      genIpop (ic);
      if (markGenerated)
        ic->generated = 1;      /* mark the icode as generated */
      else
        _G.stack.pushed = pushed;
      ic = ic->next;
    }
}

/*-----------------------------------------------------------------*/
/* emitDummyCall - emit a dummy call for --no-ret-without-call     */
/*-----------------------------------------------------------------*/
static void
emitDummyCall(void)
{
  symbol *dummyLabel;

  if (!options.no_ret_without_call)
    return;
  dummyLabel = newiTempLabel (NULL);
  emitcode ("lcall", "!tlabel", labelKey2num (dummyLabel->key));
  emitLabel (dummyLabel);
  emitcode ("dec", "sp");
  emitcode ("dec", "sp");
}

/*-----------------------------------------------------------------*/
/* saveRBank - saves an entire register bank on the stack          */
/*-----------------------------------------------------------------*/
static void
saveRBank (int bank, iCode * ic, bool pushPsw)
{
  int i;
  int count = 8 + (pushPsw ? 1 : 0);
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
          r = getFreePtr (ic, aop, FALSE);
        }
      // allocate space first
      emitcode ("mov", "%s,%s", r->name, spname);
      MOVA (r->name);
      emitcode ("add", "a,#!constbyte", count);
      emitcode ("mov", "%s,a", spname);
    }

  for (i = 0; i < 8; i++)
    {
      if (options.useXstack)
        {
          emitcode ("mov", "a,(%s+%d)", regs8051[i].base, 8 * bank + regs8051[i].offset);
          emitcode ("movx", "@%s,a", r->name);
          _G.stack.xpushed++;
          if (--count)
            emitcode ("inc", "%s", r->name);
        }
      else
        {
          char buf[16] = "";
          SNPRINTF (buf, 16, "(%s+%d)", regs8051[i].base, 8 * bank + regs8051[i].offset);
          emitpush (buf);
        }
    }

  if (pushPsw)
    {
      if (options.useXstack)
        {
          emitcode ("mov", "a,psw");
          emitcode ("movx", "@%s,a", r->name);
          _G.stack.xpushed++;
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
          r = getFreePtr (ic, aop, FALSE);
        }
      emitcode ("mov", "%s,%s", r->name, spname);
    }

  if (popPsw)
    {
      if (options.useXstack)
        {
          emitcode ("dec", "%s", r->name);
          emitcode ("movx", "a,@%s", r->name);
          emitcode ("mov", "psw,a");
          _G.stack.xpushed--;
        }
      else
        {
          emitpop ("psw");
        }
    }

  for (i = 7; i >= 0; i--)
    {
      if (options.useXstack)
        {
          emitcode ("dec", "%s", r->name);
          emitcode ("movx", "a,@%s", r->name);
          emitcode ("mov", "(%s+%d),a", regs8051[i].base, 8 * bank + regs8051[i].offset);
          _G.stack.xpushed--;
        }
      else
        {
          char buf[16] = "";
          SNPRINTF (buf, 16, "(%s+%d)", regs8051[i].base, 8 * bank + regs8051[i].offset);
          emitpop (buf);
        }
    }

  if (options.useXstack)
    {
      emitcode ("mov", "%s,%s", spname, r->name);
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

  /* first we do all bit parameters */
  for (sic = setFirstItem (sendSet); sic; sic = setNextItem (sendSet))
    {
      if (sic->argreg > 12)
        {
          int bit = sic->argreg - 13;

          aopOp (IC_LEFT (sic), sic, FALSE);

          /* if left is a literal then
             we know what the value is */
          if (AOP_TYPE (IC_LEFT (sic)) == AOP_LIT)
            {
              if (((int) operandLitValue (IC_LEFT (sic))))
                emitcode ("setb", "b[%d]", bit);
              else
                emitcode ("clr", "b[%d]", bit);
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

  if (options.useXstack || bit_count || setFirstItem (sendSet) && operandSize (IC_LEFT ((iCode *)(setFirstItem (sendSet)))) >= 6)
    {
      if (bit_count)
        BITSINB++;
      saveRegisters (setFirstItem (sendSet));
      if (bit_count)
        BITSINB--;
    }

  if (bit_count)
    {
      emitcode ("mov", "bits,b");
    }

  /* then we do all other parameters */
  for (sic = setFirstItem (sendSet); sic; sic = setNextItem (sendSet))
    {
      if (sic->argreg <= 12)
        {
          int size, offset = 0;
          aopOp (IC_LEFT (sic), sic, FALSE);
          size = AOP_SIZE (IC_LEFT (sic));

          if (sic->argreg == 1)
            {
              if (AOP_TYPE (IC_LEFT (sic)) != AOP_DPTR)
                {
                  bool pushedA = FALSE;
                  while (size--)
                    {
                      const char *l = aopGet (IC_LEFT (sic), offset, FALSE, FALSE);
                      if (!EQ (l, fReturn[offset]))
                        if (fReturn[offset][0] == 'r' && (AOP_TYPE (IC_LEFT (sic)) == AOP_REG || AOP_TYPE (IC_LEFT (sic)) == AOP_R0 || AOP_TYPE (IC_LEFT (sic)) == AOP_R1)) 
                          emitcode ("mov", "a%s,%s", fReturn[offset], l); // use register's direct address instead of name
                        else
                          emitcode ("mov", "%s,%s", fReturn[offset], l);
                      else if (EQ (l, "a") && size != 0)
                        {
                          emitpush ("acc");
                          pushedA = TRUE;
                        }
                      offset++;
                    }
                  if (pushedA)
                    emitpop ("acc");
                }
              else /* need to load dpl, dph, etc from @dptr */
                {
                  while (size--)
                    {
                      MOVA (aopGet (IC_LEFT (sic), offset, FALSE, FALSE));
                      emitpush ("acc");
                      offset++;
                    }
                  size = AOP_SIZE (IC_LEFT (sic));
                  while (size--)
                    {
                      offset--;
                      if (!EQ ("a", fReturn[offset]))
                        {
                          emitpop (fReturn[offset]);
                        }
                      else
                        {
                          emitpop ("acc");
                        }
                    }
                }
            }
          else
            {
              while (size--)
                {
                  emitcode ("mov", "%s,%s", rb1regs[sic->argreg + offset - 5], aopGet (IC_LEFT (sic), offset, FALSE, FALSE));
                  offset++;
                }
            }
          freeAsmop (IC_LEFT (sic), NULL, sic, TRUE);
        }
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
  bool swapBanks = FALSE;
  bool accuse = FALSE;
  bool accPushed = FALSE;
  bool resultInF0 = FALSE;
  bool assignResultGenerated = FALSE;

  D (emitcode (";", "genCall"));

  dtype = operandType (IC_LEFT (ic));
  etype = getSpec (dtype);
  /* if send set is not empty then assign */
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

  /* if we are calling a not _naked function that is not using
     the same register bank then we need to save the
     destination registers on the stack */
  if (currFunc && dtype && !IFFUNC_ISNAKED (dtype) &&
      (FUNC_REGBANK (currFunc->type) != FUNC_REGBANK (dtype)) && !IFFUNC_ISISR (dtype))
    {
      swapBanks = TRUE;
    }

  /* if caller saves & we have not saved then */
  if (!ic->regsSaved)
    saveRegisters (ic);

  if (swapBanks)
    {
      emitcode ("mov", "psw,#!constbyte", ((FUNC_REGBANK (dtype)) << 3) & 0xff);
    }

  /* make the call */
  if (IFFUNC_ISBANKEDCALL (dtype))
    {
      if (IFFUNC_CALLEESAVES (dtype))
        {
          werror (E_BANKED_WITH_CALLEESAVES);
        }
      else
        {
          if (IS_LITERAL (etype))
            {
              emitcode ("mov", "r0,#%s", aopLiteralLong (OP_VALUE (IC_LEFT (ic)), 0, 1));
              emitcode ("mov", "r1,#%s", aopLiteralLong (OP_VALUE (IC_LEFT (ic)), 1, 1));
              emitcode ("mov", "r2,#%s", aopLiteralLong (OP_VALUE (IC_LEFT (ic)), 2, 1));
            }
          else
            {
              char *name = (OP_SYMBOL (IC_LEFT (ic))->rname[0] ?
                            OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name);
              emitcode ("mov", "r0,#%s", name);
              emitcode ("mov", "r1,#(%s >> 8)", name);
              emitcode ("mov", "r2,#(%s >> 16)", name);
            }
          emitcode ("lcall", "__sdcc_banked_call");
        }
    }
  else
    {
      if (IS_LITERAL (etype))
        {
          emitcode ("lcall", "0x%04X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
        }
      else
        {
          emitcode ("lcall", "%s", (OP_SYMBOL (IC_LEFT (ic))->rname[0] ?
                                    OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name));
        }
    }

  if (swapBanks)
    {
      selectRegBank (FUNC_REGBANK (currFunc->type), IS_BIT (etype));
    }

  /* if we need assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic)) &&
       !IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))) &&
       (OP_SYMBOL (IC_RESULT (ic))->nRegs ||
        OP_SYMBOL (IC_RESULT (ic))->accuse ||
        OP_SYMBOL (IC_RESULT (ic))->spildir || IS_BIT (etype))) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {
      _G.accInUse++;
      aopOp (IC_RESULT (ic), ic, FALSE);
      _G.accInUse--;

      accuse = assignResultValue (IC_RESULT (ic), IC_LEFT (ic));
      assignResultGenerated = TRUE;

      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }

  /* adjust the stack for parameters if required */
  if (ic->parmBytes)
    {
      int i;
      if (ic->parmBytes > 3)
        {
          if (accuse)
            {
              emitpush ("acc");
              accPushed = TRUE;
            }
          if (IS_BIT (etype) && IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))) && !assignResultGenerated)
            {
              emitcode ("mov", "F0,c");
              resultInF0 = TRUE;
            }

          emitcode ("mov", "a,%s", spname);
          emitcode ("add", "a,#0x%02x", (-ic->parmBytes) & 0xff);
          emitcode ("mov", "%s,a", spname);
          if (options.useXstack)
            _G.stack.xpushed -= ic->parmBytes;
          else
            _G.stack.pushed -= ic->parmBytes;

          /* unsaveRegisters from xstack needs acc, but */
          /* unsaveRegisters from stack needs this popped */
          if (accPushed && !options.useXstack)
            {
              emitpop ("acc");
              accPushed = FALSE;
            }
        }
      else
        {
          for (i = 0; i < ic->parmBytes; i++)
            emitcode ("dec", "%s", spname);
          if (options.useXstack)
            _G.stack.xpushed -= ic->parmBytes;
          else
            _G.stack.pushed -= ic->parmBytes;
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

  if (IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))) && !assignResultGenerated)
    {
      if (resultInF0)
        emitcode ("mov", "c,F0");

      aopOp (IC_RESULT (ic), ic, FALSE);
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
  sym_link *etype;
  bool swapBanks = FALSE;
  bool resultInF0 = FALSE;

  D (emitcode (";", "genPcall"));

  dtype = operandType (IC_LEFT (ic))->next;
  etype = getSpec (dtype);
  /* if caller saves & we have not saved then */
  if (!ic->regsSaved)
    saveRegisters (ic);

  /* if we are calling a not _naked function that is not using
     the same register bank then we need to save the
     destination registers on the stack */
  if (currFunc && dtype && !IFFUNC_ISNAKED (dtype) &&
      (FUNC_REGBANK (currFunc->type) != FUNC_REGBANK (dtype)) && !IFFUNC_ISISR (dtype))
    {
      swapBanks = TRUE;
      // need caution message to user here
    }

  if (IS_LITERAL (etype))
    {
      /* if send set is not empty then assign */
      if (_G.sendSet)
        {
          genSend (reverseSet (_G.sendSet));
          _G.sendSet = NULL;
        }

      if (swapBanks)
        {
          emitcode ("mov", "psw,#0x%02x", ((FUNC_REGBANK (dtype)) << 3) & 0xff);
        }

      if (IFFUNC_ISBANKEDCALL (dtype))
        {
          if (IFFUNC_CALLEESAVES (dtype))
            {
              werror (E_BANKED_WITH_CALLEESAVES);
            }
          else
            {
              emitcode ("mov", "r0,#%s", aopLiteralLong (OP_VALUE (IC_LEFT (ic)), 0, 1));
              emitcode ("mov", "r1,#%s", aopLiteralLong (OP_VALUE (IC_LEFT (ic)), 1, 1));
              emitcode ("mov", "r2,#%s", aopLiteralLong (OP_VALUE (IC_LEFT (ic)), 2, 1));
              emitcode ("lcall", "__sdcc_banked_call");
            }
        }
      else
        {
          emitcode ("lcall", "0x%04X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
        }
    }
  else
    {
      if (IFFUNC_ISBANKEDCALL (dtype))
        {
          if (IFFUNC_CALLEESAVES (dtype))
            {
              werror (E_BANKED_WITH_CALLEESAVES);
            }
          else
            {
              aopOp (IC_LEFT (ic), ic, FALSE);

              emitpush (aopGet (IC_LEFT (ic), 0, FALSE, TRUE));
              emitpush (aopGet (IC_LEFT (ic), 1, FALSE, TRUE));
              emitpush (aopGet (IC_LEFT (ic), 2, FALSE, TRUE));

              freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);

              /* if send set is not empty then assign */
              if (_G.sendSet)
                {
                  genSend (reverseSet (_G.sendSet));
                  _G.sendSet = NULL;
                }

              if (swapBanks)
                {
                  char buf[8] = "";
                  int reg = ((FUNC_REGBANK (dtype)) << 3) & 0xff;
                  emitcode ("mov", "psw,#0x%02x", reg);
                  SNPRINTF (buf, 8, "0x%02x", reg + 2);
                  emitpop (buf);
                  SNPRINTF (buf, 8, "0x%02x", reg + 1);
                  emitpop (buf);
                  SNPRINTF (buf, 8, "0x%02x", reg + 0);
                  emitpop (buf);
                }
              else
                {
                  emitpop ("ar2");
                  emitpop ("ar1");
                  emitpop ("ar0");
                }
              /* make the call */
              emitcode ("lcall", "__sdcc_banked_call");
            }
        }
      else if (_G.sendSet)      /* the send set is not empty */
        {
          symbol *callLabel = newiTempLabel (NULL);
          symbol *returnLabel = newiTempLabel (NULL);

          /* create the return address on the stack */
          emitcode ("lcall", "!tlabel", labelKey2num (callLabel->key));
          emitcode ("ljmp", "!tlabel", labelKey2num (returnLabel->key));
          emitLabel (callLabel);
          _G.stack.pushed += 2;

          emitDummyCall();
          /* now push the function address */
          pushSide (IC_LEFT (ic), FARPTRSIZE, ic);

          /* send set is not empty: assign */
          genSend (reverseSet (_G.sendSet));
          _G.sendSet = NULL;

          if (swapBanks)
            {
              emitcode ("mov", "psw,#0x%02x", ((FUNC_REGBANK (dtype)) << 3) & 0xff);
            }

          /* make the call */
          emitcode ("ret", "");
          _G.stack.pushed -= 4;
          emitLabel (returnLabel);
        }
      else                      /* the send set is empty */
        {
          /* now get the called address into dptr */
          aopOp (IC_LEFT (ic), ic, FALSE);

          if (AOP_TYPE (IC_LEFT (ic)) == AOP_DPTR)
            {
              emitcode ("mov", "r0,%s", aopGet (IC_LEFT (ic), 0, FALSE, FALSE));
              emitcode ("mov", "dph,%s", aopGet (IC_LEFT (ic), 1, FALSE, FALSE));
              emitcode ("mov", "dpl,r0");
            }
          else
            {
              emitcode ("mov", "dpl,%s", aopGet (IC_LEFT (ic), 0, FALSE, FALSE));
              emitcode ("mov", "dph,%s", aopGet (IC_LEFT (ic), 1, FALSE, FALSE));
            }

          freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);

          if (swapBanks)
            {
              emitcode ("mov", "psw,#0x%02x", ((FUNC_REGBANK (dtype)) << 3) & 0xff);
            }

          /* make the call */
          emitcode ("lcall", "__sdcc_call_dptr");
        }
    }
  if (swapBanks)
    {
      selectRegBank (FUNC_REGBANK (currFunc->type), IS_BIT (etype));
    }

  /* if we need assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic)) &&
       !IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))) &&
       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->accuse || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {
      _G.accInUse++;
      aopOp (IC_RESULT (ic), ic, FALSE);
      _G.accInUse--;

      assignResultValue (IC_RESULT (ic), IC_LEFT (ic));

      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }

  /* adjust the stack for parameters if required */
  if (ic->parmBytes)
    {
      int i;
      if (ic->parmBytes > 3)
        {
          if (IS_BIT (OP_SYM_ETYPE (IC_LEFT (ic))) && IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))))
            {
              emitcode ("mov", "F0,c");
              resultInF0 = TRUE;
            }

          emitcode ("mov", "a,%s", spname);
          emitcode ("add", "a,#0x%02x", (-ic->parmBytes) & 0xff);
          emitcode ("mov", "%s,a", spname);
          if (options.useXstack)
            _G.stack.xpushed -= ic->parmBytes;
          else
            _G.stack.pushed -= ic->parmBytes;
        }
      else
        {
          for (i = 0; i < ic->parmBytes; i++)
            emitcode ("dec", "%s", spname);
          if (options.useXstack)
            _G.stack.xpushed -= ic->parmBytes;
          else
            _G.stack.pushed -= ic->parmBytes;
        }
    }

  /* if we had saved some registers then unsave them */
  if (ic->regsSaved && !IFFUNC_CALLEESAVES (dtype))
    unsaveRegisters (ic);

  if (IS_BIT (OP_SYM_ETYPE (IC_RESULT (ic))))
    {
      if (resultInF0)
        emitcode ("mov", "c,F0");

      aopOp (IC_RESULT (ic), ic, FALSE);
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
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void
genFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  sym_link *ftype = operandType (IC_LEFT (ic));
  bool switchedPSW = FALSE;
  int calleesaves_saved_register = -1;
  int stackAdjust = sym->stack;
  int accIsFree = sym->recvSize < 4;
  char *freereg = NULL;
  iCode *ric = (ic->next && ic->next->op == RECEIVE) ? ic->next : NULL;
  bool fReentrant = (IFFUNC_ISREENT (sym->type) || options.stackAuto);

  /* create the function header */
  emitcode (";", "-----------------------------------------");
  emitcode (";", " function %s", sym->name);
  emitcode (";", "-----------------------------------------");

  emitcode ("", "%s:", sym->rname);
  genLine.lineCurr->isLabel = 1;
  _G.currentFunc = sym;

  if (IFFUNC_ISNAKED (ftype))
    {
      emitcode (";", "naked function: no prologue.");
      return;
    }

  /* here we need to generate the equates for the
     register bank if required */
  if (FUNC_REGBANK (ftype) != rbank)
    {
      int i;

      rbank = FUNC_REGBANK (ftype);
      for (i = 0; i < mcs51_nRegs; i++)
        {
          if (regs8051[i].type != REG_BIT)
            {
              if (EQ (regs8051[i].base, "0"))
                emitcode ("", "%s !equ !constbyte", regs8051[i].dname, 8 * rbank + regs8051[i].offset);
              else
                emitcode ("", "%s !equ %s + !constbyte", regs8051[i].dname, regs8051[i].base, 8 * rbank + regs8051[i].offset);
            }
        }
    }

  _G.stack.param_offset = 0;
  _G.stack.offset = sym->stack;
  _G.stack.xoffset = sym->xstack;
  wassertl (_G.stack.pushed == 0, "stack over/underflow");
  wassertl (_G.stack.xpushed == 0, "xstack over/underflow");

  /* if this is an interrupt service routine then
     save acc, b, dpl, dph  */
  if (IFFUNC_ISISR (ftype))
    {
      bitVect *rsavebits;

      /* weird but possible, one should better use a different priority */
      /* if critical function then turn interrupts off */
      if (IFFUNC_ISCRITICAL (ftype))
        {
          emitcode ("clr", "ea");
        }

      rsavebits = bitVectIntersect (bitVectCopy (mcs51_allBitregs ()), sym->regsUsed);
      if (IFFUNC_HASFCALL (ftype) || !bitVectIsZero (rsavebits))
        {
          if (!inExcludeList ("bits"))
            {
              emitpush ("bits");
              BitBankUsed = 1;
            }
        }
      freeBitVect (rsavebits);

      if (!inExcludeList ("acc"))
        emitpush ("acc");
      if (!inExcludeList ("b"))
        emitpush ("b");
      if (!inExcludeList ("dpl"))
        emitpush ("dpl");
      if (!inExcludeList ("dph"))
        emitpush ("dph");
      /* if this isr has no bank i.e. is going to
         run with bank 0 , then we need to save more
         registers :-) */
      if (!FUNC_REGBANK (ftype))
        {
          int i;

          /* if this function does not call any other
             function then we can be economical and
             save only those registers that are used */
          if (!IFFUNC_HASFCALL (ftype))
            {
              /* if any registers used */
              if (!bitVectIsZero (sym->regsUsed))
                {
                  /* save the registers used */
                  for (i = 0; i < sym->regsUsed->size; i++)
                    {
                      if (bitVectBitValue (sym->regsUsed, i))
                        pushReg (i, TRUE);
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

          if (IFFUNC_HASFCALL (ftype))
            {
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
                  if (dtype && FUNC_REGBANK (dtype) != FUNC_REGBANK (ftype))
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

      /* Set the register bank to the desired value if nothing else */
      /* has done so yet. */
      if (!switchedPSW)
        {
          emitpush ("psw");
          emitcode ("mov", "psw,#!constbyte", (FUNC_REGBANK (ftype) << 3) & 0x00ff);
        }
    }
  else
    {
      /* This is a non-ISR function. */

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

      /* The caller has already switched register banks if */
      /* necessary, so just handle the callee-saves option. */

      /* if callee-save to be used for this function
         then save the registers being used in this function */
      if (IFFUNC_CALLEESAVES (ftype))
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
                      /* remember one saved register for later usage */
                      if (calleesaves_saved_register < 0)
                        calleesaves_saved_register = i;
                      bits_pushed = pushReg (i, bits_pushed);
                      _G.stack.param_offset--;
                    }
                }
            }
        }
    }

  if (fReentrant && !options.omitFramePtr)
    {
      if (options.useXstack)
        {
          if (sym->xstack || FUNC_HASSTACKPARM (ftype))
            {
              emitcode ("mov", "r0,%s", spname);
              emitcode ("inc", "%s", spname);
              emitcode ("xch", "a,_bpx");
              emitcode ("movx", "@r0,a");
              emitcode ("inc", "r0");
              emitcode ("mov", "a,r0");
              emitcode ("xch", "a,_bpx");
            }
          if (sym->stack)
            {
              /* save the callers stack, but without pushed++  */
              emitcode ("push", "_bp");
              emitcode ("mov", "_bp,sp");
            }
        }
      else
        {
          if (sym->stack || FUNC_HASSTACKPARM (ftype))
            {
              /* set up the stack */
              /* save the callers stack, but without pushed++  */
              emitcode ("push", "_bp");
              emitcode ("mov", "_bp,sp");
            }
        }
    }

  /* For some cases it is worthwhile to perform a RECEIVE iCode */
  /* before setting up the stack frame completely. */
  if (ric && ric->argreg == 1 && IC_RESULT (ric))
    {
      symbol *rsym = OP_SYMBOL (IC_RESULT (ric));

      if (rsym->isitmp)
        {
          if (rsym && rsym->regType == REG_CND)
            rsym = NULL;
          if (rsym && (rsym->accuse || rsym->ruonly))
            rsym = NULL;
          if (rsym && (rsym->isspilt || rsym->nRegs == 0) && rsym->usl.spillLoc)
            rsym = rsym->usl.spillLoc;
        }

      /* If the RECEIVE operand immediately spills to the first entry on the */
      /* stack, we can push it directly (since sp = _bp + 1 at this point) */
      /* rather than the usual @r0/r1 machinations. */
      if (!options.useXstack && rsym && rsym->onStack && rsym->stack == 1)
        {
          int ofs;

          genLine.lineElement.ic = ric;
          D (emitcode (";", "genReceive"));
          for (ofs = 0; ofs < sym->recvSize; ofs++)
            {
              emitpush (fReturn[ofs]);
              _G.stack.pushed--; /* cancel out pushed++ from emitpush()*/
            }
          stackAdjust -= sym->recvSize;
          if (stackAdjust < 0)
            {
              assert (stackAdjust >= 0);
              stackAdjust = 0;
            }
          genLine.lineElement.ic = ic;
          ric->generated = 1;
          accIsFree = 1;
        }
      /* If the RECEIVE operand is 4 registers, we can do the moves now */
      /* to free up the accumulator. */
      else if (rsym && rsym->nRegs && sym->recvSize == 4)
        {
          int ofs;

          genLine.lineElement.ic = ric;
          D (emitcode (";", "genReceive"));
          for (ofs = 0; ofs < sym->recvSize; ofs++)
            {
              emitcode ("mov", "%s,%s", rsym->regs[ofs]->name, fReturn[ofs]);
            }
          genLine.lineElement.ic = ic;
          ric->generated = 1;
          accIsFree = 1;
        }
    }

  /* If the accumulator is not free, we will need another register */
  /* to clobber. No need to worry about a possible conflict with */
  /* the above early RECEIVE optimizations since they would have */
  /* freed the accumulator if they were generated. */
  if (IFFUNC_CALLEESAVES (ftype))
    {
      /* if it's a callee-saves function we need a saved register */
      if (calleesaves_saved_register >= 0)
        {
          freereg = REG_WITH_INDEX (calleesaves_saved_register)->dname;
        }
    }
  else
    {
      /* not callee-saves, we can clobber r0 */
      freereg = "r0";
    }

  /* adjust the stack for the function */
  if (stackAdjust)
    {
      int i = stackAdjust & 0xff;
      if (stackAdjust > 256)
        werror (W_STACK_OVERFLOW, sym->name);

      if (i > 3 && accIsFree)
        {
          emitcode ("mov", "a,sp");
          emitcode ("add", "a,#!constbyte", i & 0xff);
          emitcode ("mov", "sp,a");
        }
      else if (i > 4)
        {
          if (freereg)
            {
              emitcode ("xch", "a,%s", freereg);
              emitcode ("mov", "a,sp");
              emitcode ("add", "a,#!constbyte", i & 0xff);
              emitcode ("mov", "sp,a");
              emitcode ("xch", "a,%s", freereg);
            }
          else
            {
              /* do it the hard way */
              while (i--)
                emitcode ("inc", "sp");
            }
        }
      else
        {
          while (i--)
            emitcode ("inc", "sp");
        }
    }

  if (sym->xstack)
    {
      int i = sym->xstack & 0xff;
      if (sym->xstack > 256)
        werror (W_STACK_OVERFLOW, sym->name);

      if (i > 3 && accIsFree)
        {
          emitcode ("mov", "a,_spx");
          emitcode ("add", "a,#!constbyte", i & 0xff);
          emitcode ("mov", "_spx,a");
        }
      else if (i > 4)
        {
          if (freereg)
            emitcode ("xch", "a,%s", freereg);
          else
            emitpush ("acc");
          emitcode ("mov", "a,_spx");
          emitcode ("add", "a,#0x%02x", i & 0xff);
          emitcode ("mov", "_spx,a");
          if (freereg)
            emitcode ("xch", "a,%s", freereg);
          else
            emitpop ("acc");
        }
      else
        {
          while (i--)
            emitcode ("inc", "_spx");
        }
    }

  _G.stack.param_offset = options.useXstack ? _G.stack.xpushed : _G.stack.pushed;
  _G.stack.pushedregs = _G.stack.pushed;
  _G.stack.xpushedregs = _G.stack.xpushed;
  _G.stack.pushed = 0;
  _G.stack.xpushed = 0;
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  sym_link *ftype = operandType (IC_LEFT (ic));
  bool fReentrant = (IFFUNC_ISREENT (sym->type) || options.stackAuto);
  lineNode *lineBodyEnd = genLine.lineCurr;
  lineNode *linePrologueStart = NULL;
  lineNode *lnp;
  bitVect *regsUsed;
  bitVect *regsUnneeded;
  int idx;

  _G.currentFunc = NULL;
  if (IFFUNC_ISNAKED (ftype))
    {
      emitcode (";", "naked function: no epilogue.");
      if (options.debug && currFunc)
        debugFile->writeEndFunction (currFunc, ic, 0);
      return;
    }

  _G.stack.xpushed = _G.stack.xpushedregs;
  _G.stack.pushed = _G.stack.pushedregs;

  if (fReentrant)
    {
      if (options.omitFramePtr)
        {
          bool cy_in_r0 = FALSE;
          bool acc_in_r0 = FALSE;

          if (sym->stack > 3)
            {
              if (IS_BIT (OP_SYM_ETYPE (IC_LEFT (ic))))
                {
                  emitcode ("mov", "r0,psw");   /* save cy in r0 */
                  cy_in_r0 = TRUE;
                }
              if (getSize (OP_SYM_ETYPE (IC_LEFT (ic))) >= 4)
                {
                  emitcode ("xch", "a,r0");     /* save a in r0 */
                  acc_in_r0 = TRUE;
                }

              emitcode ("mov", "a,sp");
              emitcode ("add", "a,#!constbyte", (-sym->stack) & 0xff);
              emitcode ("mov", "sp,a");
            }
          else
            {
              int i = sym->stack;
              while (i--)
                emitcode ("dec", "sp");
            }
          if (sym->xstack > 3)
            {
              if (IS_BIT (OP_SYM_ETYPE (IC_LEFT (ic))))
                {
                  if (!cy_in_r0)
                    emitcode ("mov", "r0,psw"); /* save cy in r0 */
                  cy_in_r0 = TRUE;
                }
              if (getSize (OP_SYM_ETYPE (IC_LEFT (ic))) >= 4)
                {
                  if (!acc_in_r0)
                    emitcode ("xch", "a,r0");   /* save a in r0 */
                  acc_in_r0 = TRUE;
                }

              emitcode ("mov", "a,_spx");
              emitcode ("add", "a,#!constbyte", (-sym->xstack) & 0xff);
              emitcode ("mov", "_spx,a");
            }
          else
            {
              int i = sym->xstack;
              while (i--)
                emitcode ("dec", "_spx");
            }

          if (acc_in_r0)
            emitcode ("xch", "a,r0");   /* restore a from r0 */

          if (cy_in_r0)
            emitcode ("mov", "psw,r0"); /* restore c from r0 */
        }
      else
        {
          if (options.useXstack)
            {
              if (sym->stack)
                {
                  if (sym->stack == 1)
                    emitcode ("dec", "sp");
                  else
                    emitcode ("mov", "sp,_bp");
                  emitcode ("pop", "_bp");      /* without pushed-- */
                }
              if (sym->xstack || FUNC_HASSTACKPARM (ftype))
                {
                  emitcode ("xch", "a,_bpx");
                  emitcode ("mov", "r0,a");
                  emitcode ("dec", "r0");
                  emitcode ("movx", "a,@r0");
                  emitcode ("xch", "a,_bpx");
                  emitcode ("mov", "%s,r0", spname);    //read before freeing stack space (interrupts)
                }
            }
          else if (sym->stack || FUNC_HASSTACKPARM (ftype))
            {
              if (sym->stack == 1)
                emitcode ("dec", "sp");
              else if (sym->stack)
                emitcode ("mov", "sp,_bp");
              emitcode ("pop", "_bp");  /* without pushed-- */
            }
        }
    }

  /* restore the register bank  */
  if (IFFUNC_ISISR (ftype))
    {
      if (!FUNC_REGBANK (ftype) || !options.useXstack)
        {
          /* Special case of ISR using non-zero bank with useXstack
           * is handled below.
           */
          emitpop ("psw");
        }
    }

  if (IFFUNC_ISISR (ftype))
    {
      bitVect *rsavebits;

      /* now we need to restore the registers */
      /* if this isr has no bank i.e. is going to
         run with bank 0 , then we need to save more
         registers :-) */
      if (!FUNC_REGBANK (ftype))
        {
          int i;
          /* if this function does not call any other
             function then we can be economical and
             save only those registers that are used */
          if (!IFFUNC_HASFCALL (ftype))
            {
              /* if any registers used */
              if (!bitVectIsZero (sym->regsUsed))
                {
                  /* restore the registers used */
                  for (i = sym->regsUsed->size; i >= 0; i--)
                    {
                      if (bitVectBitValue (sym->regsUsed, i))
                        popReg (i, TRUE);
                    }
                }
            }
          else
            {
              if (options.parms_in_bank1)
                {
                  for (i = 7; i >= 0; i--)
                    {
                      emitpop (rb1regs[i]);
                    }
                }
              /* this function has a function call. We cannot
                 determine register usage so we will have to pop the
                 entire bank */
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

      if (!inExcludeList ("dph"))
        emitpop ("dph");
      if (!inExcludeList ("dpl"))
        emitpop ("dpl");
      if (!inExcludeList ("b"))
        emitpop ("b");
      if (!inExcludeList ("acc"))
        emitpop ("acc");

      rsavebits = bitVectIntersect (bitVectCopy (mcs51_allBitregs ()), sym->regsUsed);
      if (IFFUNC_HASFCALL (ftype) || !bitVectIsZero (rsavebits))
        {
          if (!inExcludeList ("bits"))
            emitpop ("bits");
        }
      freeBitVect (rsavebits);

      /* weird but possible, one should better use a different priority */
      /* if critical function then turn interrupts off */
      if (IFFUNC_ISCRITICAL (ftype))
        {
          emitcode ("setb", "ea");
        }

      /* if debug then send end of function */
      if (options.debug && currFunc)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      emitcode ("reti", "");
    }
  else
    {
      if (IFFUNC_CALLEESAVES (ftype))
        {
          int i;

          /* if any registers used */
          if (sym->regsUsed)
            {
              /* save the registers used */
              for (i = sym->regsUsed->size; i >= 0; i--)
                {
                  if (bitVectBitValue (sym->regsUsed, i) || (mcs51_ptrRegReq && (i == R0_IDX || i == R1_IDX)))
                    emitpop (REG_WITH_INDEX (i)->dname);
                }
            }
          else if (mcs51_ptrRegReq)
            {
              emitpop (REG_WITH_INDEX (R1_IDX)->dname);
              emitpop (REG_WITH_INDEX (R0_IDX)->dname);
            }
        }

      if (IFFUNC_ISCRITICAL (ftype))
        {
          if (IS_BIT (OP_SYM_ETYPE (IC_LEFT (ic))))
            {
              emitcode ("rlc", "a");        /* save c in a */
              emitpop ("psw");              /* restore ea via c in psw */
              emitcode ("mov", "ea,c");
              emitcode ("rrc", "a");        /* restore c from a */
            }
          else
            {
              emitpop ("psw");              /* restore ea via c in psw */
              emitcode ("mov", "ea,c");
            }
        }

      /* if debug then send end of function */
      if (options.debug && currFunc)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      if (IFFUNC_ISBANKEDCALL (ftype))
        {
          emitcode ("ljmp", "__sdcc_banked_ret");
        }
      else
        {
          emitcode ("ret", "");
        }
    }

  wassertl (_G.stack.pushed == 0, "stack over/underflow");
  wassertl (_G.stack.xpushed == 0, "xstack over/underflow");

  if (!port->peep.getRegsRead || !port->peep.getRegsWritten || options.nopeep)
    return;

  /* If this was an interrupt handler using bank 0 that called another */
  /* function, then all registers must be saved; nothing to optimize.  */
  if (IFFUNC_ISISR (ftype) && IFFUNC_HASFCALL (ftype) && !FUNC_REGBANK (ftype))
    return;

  /* There are no push/pops to optimize if not callee-saves or ISR */
  if (!(FUNC_CALLEESAVES (ftype) || FUNC_ISISR (ftype)))
    return;

  /* If there were stack parameters, we cannot optimize without also    */
  /* fixing all of the stack offsets; this is too dificult to consider. */
  if (FUNC_HASSTACKPARM (ftype))
    return;

  /* Compute the registers actually used */
  regsUsed = newBitVect (mcs51_nRegs);
  lnp = lineBodyEnd;
  while (lnp)
    {
      /* Remove change of register bank if no registers used */
      if (lnp->ic && lnp->ic->op == FUNCTION &&
          !strncmp (lnp->line, "mov", 3) &&
          bitVectFirstBit (port->peep.getRegsWritten (lnp)) == CND_IDX &&
          !bitVectBitsInCommon (mcs51_allBankregs (), regsUsed) &&
          !IFFUNC_HASFCALL (ftype))
        {
          emitcode (";", "eliminated unneeded mov psw,# (no regs used in bank)");
          connectLine (lnp->prev, lnp->next);
        }
      else
        {
          regsUsed = bitVectUnion (regsUsed, port->peep.getRegsWritten (lnp));
        }

      if (lnp->ic && lnp->ic->op == FUNCTION)
        {
          if (!lnp->prev || (lnp->prev->ic && lnp->prev->ic->op != FUNCTION))
            break;
        }
      lnp = lnp->prev;
    }
  linePrologueStart = lnp;

  /* If this was an interrupt handler that called another function */
  /* function, then assume A, B, DPH, & DPL may be modified by it. */
  if (IFFUNC_ISISR (ftype) && IFFUNC_HASFCALL (ftype))
    {
      regsUsed = bitVectSetBit (regsUsed, DPL_IDX);
      regsUsed = bitVectSetBit (regsUsed, DPH_IDX);
      regsUsed = bitVectSetBit (regsUsed, B_IDX);
      regsUsed = bitVectSetBit (regsUsed, A_IDX);
      regsUsed = bitVectSetBit (regsUsed, CND_IDX);
    }

  /* Remove the unneeded push/pops */
  regsUnneeded = newBitVect (mcs51_nRegs);
  for (lnp = genLine.lineCurr; lnp != linePrologueStart; lnp = lnp->prev)
    {
      if (lnp->ic)
        {
          if (lnp->ic && (lnp->ic->op == FUNCTION) && !strncmp (lnp->line, "push", 4))
            {
              idx = bitVectFirstBit (port->peep.getRegsRead (lnp));
              if (idx >= 0 && !bitVectBitValue (regsUsed, idx))
                {
                  connectLine (lnp->prev, lnp->next);
                  regsUnneeded = bitVectSetBit (regsUnneeded, idx);
                }
            }
          if (lnp->ic && (lnp->ic->op == ENDFUNCTION) && !strncmp (lnp->line, "pop", 3))
            {
              idx = bitVectFirstBit (port->peep.getRegsWritten (lnp));
              if (idx >= 0 && !bitVectBitValue (regsUsed, idx))
                {
                  connectLine (lnp->prev, lnp->next);
                  regsUnneeded = bitVectSetBit (regsUnneeded, idx);
                }
            }
        }
    }

  for (idx = 0; idx < regsUnneeded->size; idx++)
    if (bitVectBitValue (regsUnneeded, idx))
      emitcode (";", "eliminated unneeded push/pop %s", REG_WITH_INDEX (idx)->dname);

  freeBitVect (regsUnneeded);
  freeBitVect (regsUsed);
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

  /* we have something to return then
     move the return value into place */
  aopOp (IC_LEFT (ic), ic, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));

  if (IS_BIT (_G.currentFunc->etype))
    {
      if (!IS_OP_RUONLY (IC_LEFT (ic)))
        toCarry (IC_LEFT (ic));
    }
  else
    {
      while (size--)
        {
          if (AOP_TYPE (IC_LEFT (ic)) == AOP_DPTR)
            {
              /* #NOCHANGE */
              emitpush (aopGet (IC_LEFT (ic), offset++, FALSE, TRUE));
              pushed++;
            }
          else
            {
              const char *l = aopGet (IC_LEFT (ic), offset, FALSE, FALSE);
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

  emitLabel (IC_LABEL (ic));
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a ljmp                                      */
/*-----------------------------------------------------------------*/
static void
genGoto (iCode * ic)
{
  emitcode ("ljmp", "!tlabel", labelKey2num (IC_LABEL (ic)->key));
}

/*-----------------------------------------------------------------*/
/* genPlusIncr :- does addition with increment if possible         */
/*-----------------------------------------------------------------*/
static bool
genPlusIncr (iCode * ic)
{
  unsigned int icount;
  unsigned int size = getDataSize (IC_RESULT (ic)), offset;

  /* will try to generate an increment */
  /* if the right side is not a literal
     we cannot */
  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    return FALSE;

  icount = (unsigned int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);

  D (emitcode (";", "genPlusIncr"));

  /* if increment >=16 bits in register or direct space */
  if ((AOP_TYPE (IC_LEFT (ic)) == AOP_REG ||
       AOP_TYPE (IC_LEFT (ic)) == AOP_DIR ||
       (IS_AOP_PREG (IC_LEFT (ic)) && !AOP_NEEDSACC (IC_LEFT (ic)))) &&
      sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))) &&
      !isOperandVolatile (IC_RESULT (ic), FALSE) && (size > 1) && (icount == 1))
    {
      symbol *tlbl;
      const char *l;

      tlbl = newiTempLabel (NULL);
      l = aopGet (IC_RESULT (ic), LSB, FALSE, FALSE);
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

      l = aopGet (IC_RESULT (ic), MSB16, FALSE, FALSE);
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

          l = aopGet (IC_RESULT (ic), offset, FALSE, FALSE);
          emitcode ("inc", "%s", l);
        }

      emitLabel (tlbl);
      return TRUE;
    }

  /* if result is dptr */
  if ((AOP_TYPE (IC_RESULT (ic)) == AOP_STR) &&
      (AOP_SIZE (IC_RESULT (ic)) == 2) &&
      !strncmp (AOP (IC_RESULT (ic))->aopu.aop_str[0], "dpl", 4) && !strncmp (AOP (IC_RESULT (ic))->aopu.aop_str[1], "dph", 4))
    {
      if (aopGetUsesAcc (IC_LEFT (ic), 0))
        return FALSE;

      if (icount > 9)
        return FALSE;

      if ((AOP_TYPE (IC_LEFT (ic)) != AOP_DIR) && (icount > 5))
        return FALSE;

      aopPut (IC_RESULT (ic), aopGet (IC_LEFT (ic), 0, FALSE, FALSE), 0);
      aopPut (IC_RESULT (ic), aopGet (IC_LEFT (ic), 1, FALSE, FALSE), 1);
      while (icount--)
        emitcode ("inc", "dptr");

      return TRUE;
    }

  /* if the literal value of the right hand side
     is greater than 4 then it is not worth it */
  if (icount > 4)
    return FALSE;

  /* if the sizes are greater than 1 then we cannot */
  if (AOP_SIZE (IC_RESULT (ic)) > 1 || AOP_SIZE (IC_LEFT (ic)) > 1)
    return FALSE;

  /* we can if the aops of the left & result match or
     if they are in registers and the registers are the
     same */
  if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
    {
      if (icount > 3)
        {
          MOVA (aopGet (IC_LEFT (ic), 0, FALSE, FALSE));
          emitcode ("add", "a,#!constbyte", ((char) icount) & 0xff);
          aopPut (IC_RESULT (ic), "a", 0);
        }
      else
        {
          while (icount--)
            {
              emitcode ("inc", "%s", aopGet (IC_LEFT (ic), 0, FALSE, FALSE));
            }
        }

      return TRUE;
    }

  if (icount == 1)
    {
      MOVA (aopGet (IC_LEFT (ic), 0, FALSE, FALSE));
      emitcode ("inc", "a");
      aopPut (IC_RESULT (ic), "a", 0);
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
  if (opIsGptr (IC_RESULT (ic)))
    {
      struct dbuf_s dbuf;

      if (opIsGptr (IC_LEFT (ic)))
        {
          if (!sameRegs (AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic))))
            {
              aopPut (IC_RESULT (ic), aopGet (IC_LEFT (ic), GPTRSIZE - 1, FALSE, FALSE), GPTRSIZE - 1);
            }
          return;
        }

      if (opIsGptr (IC_RIGHT (ic)))
        {
          if (!sameRegs (AOP (IC_RESULT (ic)), AOP (IC_RIGHT (ic))))
            {
              aopPut (IC_RESULT (ic), aopGet (IC_RIGHT (ic), GPTRSIZE - 1, FALSE, FALSE), GPTRSIZE - 1);
            }
          return;
        }

      dbuf_init (&dbuf, 128);
      if (IC_LEFT (ic) && AOP_SIZE (IC_LEFT (ic)) < GPTRSIZE &&
          IC_RIGHT (ic) && AOP_SIZE (IC_RIGHT (ic)) < GPTRSIZE &&
          !sameRegs (AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic))) && !sameRegs (AOP (IC_RESULT (ic)), AOP (IC_RIGHT (ic))))
        {
          dbuf_printf (&dbuf, "#0x%02x", pointerTypeToGPByte (pointerCode (getSpec (operandType (IC_LEFT (ic)))), NULL, NULL));
          aopPut (IC_RESULT (ic), dbuf_c_str (&dbuf), GPTRSIZE - 1);
        }
      else if (IC_LEFT (ic) && AOP_SIZE (IC_LEFT (ic)) < GPTRSIZE && !sameRegs (AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic))))
        {
          dbuf_printf (&dbuf, "#0x%02x", pointerTypeToGPByte (pointerCode (getSpec (operandType (IC_LEFT (ic)))), NULL, NULL));
          aopPut (IC_RESULT (ic), dbuf_c_str (&dbuf), GPTRSIZE - 1);
        }
      else if (IC_RIGHT (ic) && AOP_SIZE (IC_RIGHT (ic)) < GPTRSIZE && !sameRegs (AOP (IC_RESULT (ic)), AOP (IC_RIGHT (ic))))
        {
          dbuf_printf (&dbuf, "#0x%02x", pointerTypeToGPByte (pointerCode (getSpec (operandType (IC_RIGHT (ic)))), NULL, NULL));
          aopPut (IC_RESULT (ic), dbuf_c_str (&dbuf), GPTRSIZE - 1);
        }
      dbuf_destroy (&dbuf);
    }
}

/*-----------------------------------------------------------------*/
/* genPlus - generates code for addition                           */
/*-----------------------------------------------------------------*/
static void
genPlus (iCode * ic)
{
  int size, offset = 0;
  int skip_bytes = 0;
  char *add = "add";
  bool swappedLR = FALSE;
  operand *leftOp, *rightOp;
  operand *op;

  D (emitcode (";", "genPlus"));

  /* special cases :- */

  aopOp (IC_LEFT (ic), ic, FALSE);
  aopOp (IC_RIGHT (ic), ic, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE);

  /* if literal, literal on the right or
     if left requires ACC or right is already
     in ACC */
  if ((AOP_TYPE (IC_LEFT (ic)) == AOP_LIT) || (AOP_NEEDSACC (IC_LEFT (ic))) || AOP_TYPE (IC_RIGHT (ic)) == AOP_ACC)
    {
      swapOperands (&IC_LEFT (ic), &IC_RIGHT (ic));
      swappedLR = TRUE;
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
          if (ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit) != 0L)
            emitcode ("cpl", "c");
          outBitC (IC_RESULT (ic));
        }
      else
        {
          size = getDataSize (IC_RESULT (ic));
          while (size--)
            {
              MOVA (aopGet (IC_RIGHT (ic), offset, FALSE, FALSE));
              emitcode ("addc", "a,%s", zero);
              aopPut (IC_RESULT (ic), "a", offset++);
            }
        }
      goto release;
    }

  /* if I can do an increment instead
     of add then GOOD for ME */
  if (genPlusIncr (ic) == TRUE)
    goto release;

  size = getDataSize (IC_RESULT (ic));
  leftOp = IC_LEFT (ic);
  rightOp = IC_RIGHT (ic);
  op = IC_LEFT (ic);

  /* if this is an add for an array access
     at a 256 byte boundary */
  if (2 == size
      && AOP_TYPE (op) == AOP_IMMD
      && IS_SYMOP (op)
      && IS_SPEC (OP_SYM_ETYPE (op)) && SPEC_ABSA (OP_SYM_ETYPE (op)) && (SPEC_ADDR (OP_SYM_ETYPE (op)) & 0xff) == 0)
    {
      D (emitcode (";", "genPlus aligned array"));
      aopPut (IC_RESULT (ic), aopGet (rightOp, 0, FALSE, FALSE), 0);

      if (1 == getDataSize (IC_RIGHT (ic)))
        {
          aopPut (IC_RESULT (ic), aopGet (leftOp, 1, FALSE, FALSE), 1);
        }
      else
        {
          MOVA (aopGet (IC_LEFT (ic), 1, FALSE, FALSE));
          emitcode ("add", "a,%s", aopGet (rightOp, 1, FALSE, FALSE));
          aopPut (IC_RESULT (ic), "a", 1);
        }
      goto release;
    }

  /* if the lower bytes of a literal are zero skip the addition */
  if (AOP_TYPE (IC_RIGHT (ic)) == AOP_LIT)
    {
      while ((0 == ((unsigned int) ullFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit) & (0xff << skip_bytes * 8))) &&
             (skip_bytes + 1 < size))
        {
          skip_bytes++;
        }
      if (skip_bytes)
        D (emitcode (";", "genPlus shortcut"));
    }

  while (size--)
    {
      if (offset >= skip_bytes)
        {
          if (aopGetUsesAcc (leftOp, offset) && aopGetUsesAcc (rightOp, offset))
            {
              bool pushedB;
              MOVA (aopGet (leftOp, offset, FALSE, FALSE));
              pushedB = pushB ();
              emitcode ("xch", "a,b");
              MOVA (aopGet (rightOp, offset, FALSE, FALSE));
              emitcode (add, "a,b");
              popB (pushedB);
            }
          else if (aopGetUsesAcc (leftOp, offset))
            {
              MOVA (aopGet (leftOp, offset, FALSE, FALSE));
              emitcode (add, "a,%s", aopGet (rightOp, offset, FALSE, FALSE));
            }
          else
            {
              MOVA (aopGet (rightOp, offset, FALSE, FALSE));
              emitcode (add, "a,%s", aopGet (leftOp, offset, FALSE, FALSE));
            }
          aopPut (IC_RESULT (ic), "a", offset);
          add = "addc";         /* further adds must propagate carry */
        }
      else
        {
          if (!sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))) || isOperandVolatile (IC_RESULT (ic), FALSE))
            {
              /* just move */
              aopPut (IC_RESULT (ic), aopGet (leftOp, offset, FALSE, FALSE), offset);
            }
        }
      offset++;
    }

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
  unsigned int icount;
  unsigned int size = getDataSize (IC_RESULT (ic));

  /* will try to generate a decrement */
  /* if the right side is not a literal
     we cannot */
  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    return FALSE;

  /* if the literal value of the right hand side
     is greater than 4 then it is not worth it */
  if ((icount = (unsigned int) ullFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit)) > 4)
    return FALSE;

  D (emitcode (";", "genMinusDec"));

  /* if decrement >=16 bits in register or direct space */
  if ((AOP_TYPE (IC_LEFT (ic)) == AOP_REG ||
       AOP_TYPE (IC_LEFT (ic)) == AOP_DIR ||
       (IS_AOP_PREG (IC_LEFT (ic)) && !AOP_NEEDSACC (IC_LEFT (ic)))) &&
      sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))) && (size > 1) && (icount == 1))
    {
      symbol *tlbl;
      const char *l;

      tlbl = newiTempLabel (NULL);
      l = aopGet (IC_RESULT (ic), LSB, FALSE, FALSE);
      emitcode ("dec", "%s", l);

      if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG || IS_AOP_PREG (IC_RESULT (ic)))
        {
          emitcode ("cjne", "%s,#!constbyte,!tlabel", l, 0xff, labelKey2num (tlbl->key));
        }
      else
        {
          emitcode ("mov", "a,#!constbyte", 0xff);
          emitcode ("cjne", "a,%s,!tlabel", l, labelKey2num (tlbl->key));
        }
      l = aopGet (IC_RESULT (ic), MSB16, FALSE, FALSE);
      emitcode ("dec", "%s", l);
      if (size > 2)
        {
          if (EQ (l, "acc"))
            {
              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
            }
          else if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG || IS_AOP_PREG (IC_RESULT (ic)))
            {
              emitcode ("cjne", "%s,#!constbyte,!tlabel", l, 0xff, labelKey2num (tlbl->key));
            }
          else
            {
              emitcode ("cjne", "a,%s,!tlabel", l, labelKey2num (tlbl->key));
            }
          l = aopGet (IC_RESULT (ic), MSB24, FALSE, FALSE);
          emitcode ("dec", "%s", l);
        }
      if (size > 3)
        {
          if (EQ (l, "acc"))
            {
              emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
            }
          else if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG || IS_AOP_PREG (IC_RESULT (ic)))
            {
              emitcode ("cjne", "%s,#!constbyte,!tlabel", l, 0xff, labelKey2num (tlbl->key));
            }
          else
            {
              emitcode ("cjne", "a,%s,!tlabel", l, labelKey2num (tlbl->key));
            }
          emitcode ("dec", "%s", aopGet (IC_RESULT (ic), MSB32, FALSE, FALSE));
        }
      emitLabel (tlbl);
      return TRUE;
    }

  /* if the sizes are greater than 1 then we cannot */
  if (AOP_SIZE (IC_RESULT (ic)) > 1 || AOP_SIZE (IC_LEFT (ic)) > 1)
    return FALSE;

  /* we can if the aops of the left & result match or
     if they are in registers and the registers are the
     same */
  if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
    {
      const char *l;

      if (aopGetUsesAcc (IC_LEFT (ic), 0))
        {
          MOVA (aopGet (IC_RESULT (ic), 0, FALSE, FALSE));
          l = "a";
        }
      else
        {
          l = aopGet (IC_RESULT (ic), 0, FALSE, FALSE);
        }

      while (icount--)
        {
          emitcode ("dec", "%s", l);
        }

      if (AOP_NEEDSACC (IC_RESULT (ic)))
        aopPut (IC_RESULT (ic), "a", 0);

      return TRUE;
    }

  if (icount == 1)
    {
      MOVA (aopGet (IC_LEFT (ic), 0, FALSE, FALSE));
      emitcode ("dec", "a");
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
      emitcode ("jnb", "%s,!tlabel", AOP (IC_LEFT (ic))->aopu.aop_dir, labelKey2num (lbl->key));
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

  D (emitcode (";", "genMinus"));

  aopOp (IC_LEFT (ic), ic, FALSE);
  aopOp (IC_RIGHT (ic), ic, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE);

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

  size = getDataSize (IC_RESULT (ic));

  /* if literal, add a,#-lit, else normal subb */
  if (AOP_TYPE (IC_RIGHT (ic)) == AOP_LIT)
    {
      unsigned long long lit = 0L;
      bool useCarry = FALSE;

      lit = ullFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);
      lit = -(long long) lit;

      while (size--)
        {
          if (useCarry || ((lit >> (offset * 8)) & 0x0ffll))
            {
              MOVA (aopGet (IC_LEFT (ic), offset, FALSE, FALSE));
              if (!offset && !size && lit == (unsigned long long) - 1)
                {
                  emitcode ("dec", "a");
                }
              else if (!useCarry)
                {
                  /* first add without previous c */
                  emitcode ("add", "a,#!constbyte", (unsigned int) ((lit >> (offset * 8)) & 0x0ffll));
                  useCarry = TRUE;
                }
              else
                {
                  emitcode ("addc", "a,#!constbyte", (unsigned int) ((lit >> (offset * 8)) & 0x0ffll));
                }
              aopPut (IC_RESULT (ic), "a", offset++);
            }
          else
            {
              /* no need to add zeroes */
              if (!sameRegs (AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic))))
                {
                  aopPut (IC_RESULT (ic), aopGet (IC_LEFT (ic), offset, FALSE, FALSE), offset);
                }
              offset++;
            }
        }
    }
  else
    {
      operand *leftOp, *rightOp;

      leftOp = IC_LEFT (ic);
      rightOp = IC_RIGHT (ic);

      while (size--)
        {
          if (aopGetUsesAcc (rightOp, offset))
            {
              if (aopGetUsesAcc (leftOp, offset))
                {
                  bool pushedB;

                  MOVA (aopGet (rightOp, offset, FALSE, FALSE));
                  pushedB = pushB ();
                  emitcode ("mov", "b,a");
                  if (offset == 0)
                    CLRC;
                  MOVA (aopGet (leftOp, offset, FALSE, FALSE));
                  emitcode ("subb", "a,b");
                  popB (pushedB);
                }
              else
                {
                  /* reverse subtraction with 2's complement */
                  if (offset == 0)
                    emitcode ("setb", "c");
                  else
                    emitcode ("cpl", "c");
                  wassertl (!aopGetUsesAcc (leftOp, offset), "accumulator clash");
                  MOVA (aopGet (rightOp, offset, FALSE, FALSE));
                  emitcode ("subb", "a,%s", aopGet (leftOp, offset, FALSE, FALSE));
                  emitcode ("cpl", "a");
                  if (size)     /* skip if last byte */
                    emitcode ("cpl", "c");
                }
            }
          else
            {
              MOVA (aopGet (leftOp, offset, FALSE, FALSE));
              if (offset == 0)
                CLRC;
              emitcode ("subb", "a,%s", aopGet (rightOp, offset, FALSE, FALSE));
            }

          aopPut (IC_RESULT (ic), "a", offset++);
        }
    }

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
genMultbits (operand * left, operand * right, operand * result)
{
  D (emitcode (";", "genMultbits"));

  emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
  emitcode ("anl", "c,%s", AOP (right)->aopu.aop_dir);
  outBitC (result);
}

/*-----------------------------------------------------------------*/
/* genMultOneByte : 8*8=8/16 bit multiplication                    */
/*-----------------------------------------------------------------*/
static void
genMultOneByte (operand * left, operand * right, operand * result)
{
  symbol *lbl;
  int size = AOP_SIZE (result);
  bool runtimeSign, compiletimeSign;
  bool lUnsigned, rUnsigned, pushedB;

  D (emitcode (";", "genMultOneByte"));

  if (size < 1 || size > 2)
    {
      /* this should never happen */
      fprintf (stderr, "size!=1||2 (%d) in %s at line:%d \n", AOP_SIZE (result), __FILE__, lineno);
      exit (EXIT_FAILURE);
    }

  /* (if two literals: the value is computed before) */
  /* if one literal, literal on the right */
  if (AOP_TYPE (left) == AOP_LIT || AOP_TYPE (right) == AOP_ACC)
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

  if (size == 1                 /* no, this is not a bug; with a 1 byte result there's
                                   no need to take care about the signedness! */
      || (lUnsigned && rUnsigned))
    {
      /* just an unsigned 8 * 8 = 8 multiply
         or 8u * 8u = 16u */
      /* emitcode (";","unsigned"); */
      /* TODO: check for accumulator clash between left & right aops? */

      /*if (AOP_TYPE (right) == AOP_ACC)
        MOVB (aopGet (left, 0, FALSE, FALSE));
      else*/ if (AOP_TYPE (right) == AOP_LIT)
        {
          /* moving to accumulator first helps peepholes */
          MOVA (aopGet (left, 0, FALSE, FALSE));
          MOVB (aopGet (right, 0, FALSE, FALSE));
        }
      else
        {
          emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE));
          MOVA (aopGet (left, 0, FALSE, FALSE));
        }

      emitcode ("mul", "ab");
      aopPut (result, "a", 0);
      if (size == 2)
        aopPut (result, "b", 1);

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
        emitcode ("clr", "F0"); /* reset sign flag */
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
        emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE));
      else
        {
          MOVA (aopGet (right, 0, FALSE, FALSE));
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc.7,!tlabel", labelKey2num (lbl->key));
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
      MOVA (aopGet (left, 0, FALSE, FALSE));

      if (!lUnsigned)
        {
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc.7,!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "F0");       /* complement sign flag */
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
        }
    }

  /* now the multiplication */
  emitcode ("mul", "ab");
  if (runtimeSign || compiletimeSign)
    {
      lbl = newiTempLabel (NULL);
      if (runtimeSign)
        emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");    /* lsb 2's complement */
      if (size != 2)
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
  aopPut (result, "a", 0);
  if (size == 2)
    aopPut (result, "b", 1);

  popB (pushedB);
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
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  /* special cases first */
  /* both are bits */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      genMultbits (left, right, result);
      goto release;
    }

  /* if both are of size == 1 */
#if 0                           // one of them can be a sloc shared with the result
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
#else
  if (getSize (operandType (left)) == 1 && getSize (operandType (right)) == 1)
#endif
    {
      genMultOneByte (left, right, result);
      goto release;
    }

  /* should have been converted to function call */
  fprintf (stderr, "left: %d right: %d\n", getSize (OP_SYMBOL (left)->type), getSize (OP_SYMBOL (right)->type));
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
genDivbits (operand * left, operand * right, operand * result)
{
  bool pushedB;

  D (emitcode (";", "genDivbits"));

  pushedB = pushB ();

  /* the result must be bit */
  emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE));

  MOVA (aopGet (left, 0, FALSE, FALSE));

  emitcode ("div", "ab");
  emitcode ("rrc", "a");

  popB (pushedB);

  aopPut (result, "c", 0);
}

/*-----------------------------------------------------------------*/
/* genDivOneByte : 8 bit division                                  */
/*-----------------------------------------------------------------*/
static void
genDivOneByte (operand * left, operand * right, operand * result)
{
  bool lUnsigned, rUnsigned, pushedB;
  bool runtimeSign, compiletimeSign;
  bool accuse = FALSE;
  bool pushedA = FALSE;
  symbol *lbl;
  int size, offset;

  D (emitcode (";", "genDivOneByte"));

  /* Why is it necessary that genDivOneByte() can return an int result?
     Have a look at:

     volatile unsigned char uc;
     volatile signed char sc1, sc2;
     volatile int i;

     uc  = 255;
     sc1 = -1;
     i = uc / sc1;

     Or:

     sc1 = -128;
     sc2 = -1;
     i = sc1 / sc2;

     In all cases a one byte result would overflow, the following cast to int
     would return the wrong result.

     Two possible solution:
     a) cast operands to int, if ((unsigned) / (signed)) or
     ((signed) / (signed))
     b) return an 16 bit signed int; this is what we're doing here!
   */

  size = AOP_SIZE (result) - 1;
  offset = 1;
  lUnsigned = SPEC_USIGN (getSpec (operandType (left)));
  rUnsigned = SPEC_USIGN (getSpec (operandType (right)));

  pushedB = pushB ();

  /* signed or unsigned */
  if (lUnsigned && rUnsigned)
    {
      /* unsigned is easy */
      MOVB (aopGet (right, 0, FALSE, FALSE));
      MOVA (aopGet (left, 0, FALSE, FALSE));
      emitcode ("div", "ab");
      aopPut (result, "a", 0);
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
        emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE));
      else
        {
          MOVA (aopGet (right, 0, FALSE, FALSE));
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc.7,!tlabel", labelKey2num (lbl->key));
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
      MOVA (aopGet (left, 0, FALSE, FALSE));

      if (!lUnsigned)
        {
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc.7,!tlabel", labelKey2num (lbl->key));
          emitcode ("cpl", "F0");       /* complement sign flag */
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
        }
    }

  /* now the division */
  emitcode ("div", "ab");

  if (runtimeSign || compiletimeSign)
    {
      lbl = newiTempLabel (NULL);
      if (runtimeSign)
        emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");    /* lsb 2's complement */
      emitcode ("inc", "a");
      emitLabel (lbl);

      accuse = aopPut (result, "a", 0);
      if (size > 0)
        {
          /* msb is 0x00 or 0xff depending on the sign */
          if (runtimeSign)
            {
              if (accuse)
                {
                  emitpush ("acc");
                  pushedA = TRUE;
                }
              emitcode ("mov", "c,F0");
              emitcode ("subb", "a,acc");
              while (size--)
                aopPut (result, "a", offset++);
            }
          else                  /* compiletimeSign */
            {
              if (aopPutUsesAcc (result, "#0xff", offset))
                {
                  emitpush ("acc");
                  pushedA = TRUE;
                }
              while (size--)
                aopPut (result, "#0xff", offset++);
            }
        }
    }
  else
    {
      aopPut (result, "a", 0);
      while (size--)
        aopPut (result, zero, offset++);
    }

  if (pushedA)
    emitpop ("acc");
  popB (pushedB);
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
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  /* special cases first */
  /* both are bits */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      genDivbits (left, right, result);
      goto release;
    }

  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
    {
      genDivOneByte (left, right, result);
      goto release;
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
genModbits (operand * left, operand * right, operand * result)
{
  bool pushedB;

  D (emitcode (";", "genModbits"));

  pushedB = pushB ();

  /* the result must be bit */
  emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE));

  MOVA (aopGet (left, 0, FALSE, FALSE));

  emitcode ("div", "ab");
  emitcode ("mov", "a,b");
  emitcode ("rrc", "a");

  popB (pushedB);

  aopPut (result, "c", 0);
}

/*-----------------------------------------------------------------*/
/* genModOneByte : 8 bit modulus                                   */
/*-----------------------------------------------------------------*/
static void
genModOneByte (operand * left, operand * right, operand * result)
{
  bool lUnsigned, rUnsigned, pushedB;
  bool runtimeSign, compiletimeSign;
  symbol *lbl;
  int size, offset;

  D (emitcode (";", "genModOneByte"));

  size = AOP_SIZE (result) - 1;
  offset = 1;
  lUnsigned = SPEC_USIGN (getSpec (operandType (left)));
  rUnsigned = SPEC_USIGN (getSpec (operandType (right)));

  /* if right is a literal, check it for 2^n */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      unsigned char val = abs ((int) operandLitValue (right));
      symbol *lbl2 = NULL;

      switch (val)
        {
        case 1:                /* sometimes it makes sense (on tricky code and hardware)... */
        case 2:
        case 4:
        case 8:
        case 16:
        case 32:
        case 64:
        case 128:
          if (lUnsigned)
            werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                    "modulus of unsigned char by 2^n literal shouldn't be processed here");
          /* because iCode should have been changed to genAnd  */
          /* see file "SDCCopt.c", function "convertToFcall()" */

          MOVA (aopGet (left, 0, FALSE, FALSE));
          emitcode ("mov", "c,acc.7");
          emitcode ("anl", "a,#0x%02x", val - 1);
          lbl = newiTempLabel (NULL);
          emitcode ("jz", "!tlabel", labelKey2num (lbl->key));
          emitcode ("jnc", "!tlabel", labelKey2num (lbl->key));
          emitcode ("orl", "a,#0x%02x", 0xff ^ (val - 1));
          if (size)
            {
              int size2 = size;
              int offs2 = offset;

              aopPut (result, "a", 0);
              while (size2--)
                aopPut (result, "#0xff", offs2++);
              lbl2 = newiTempLabel (NULL);
              emitcode ("sjmp", "!tlabel", labelKey2num (lbl2->key));
            }
          emitLabel (lbl);
          aopPut (result, "a", 0);
          while (size--)
            aopPut (result, zero, offset++);
          if (lbl2)
            {
              emitLabel (lbl2);
            }
          return;

        default:
          break;
        }
    }

  pushedB = pushB ();

  /* signed or unsigned */
  if (lUnsigned && rUnsigned)
    {
      /* unsigned is easy */
      MOVB (aopGet (right, 0, FALSE, FALSE));
      MOVA (aopGet (left, 0, FALSE, FALSE));
      emitcode ("div", "ab");
      aopPut (result, "b", 0);
      while (size--)
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
        emitcode ("mov", "b,%s", aopGet (right, 0, FALSE, FALSE));
      else
        {
          MOVA (aopGet (right, 0, FALSE, FALSE));
          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc.7,!tlabel", labelKey2num (lbl->key));
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
      MOVA (aopGet (left, 0, FALSE, FALSE));

      if (!lUnsigned)
        {
          runtimeSign = TRUE;
          emitcode ("clr", "F0");       /* clear sign flag */

          lbl = newiTempLabel (NULL);
          emitcode ("jnb", "acc.7,!tlabel", labelKey2num (lbl->key));
          emitcode ("setb", "F0");      /* set sign flag */
          emitcode ("cpl", "a");        /* 2's complement */
          emitcode ("inc", "a");
          emitLabel (lbl);
        }
    }

  /* now the modulus */
  emitcode ("div", "ab");

  if (runtimeSign || compiletimeSign)
    {
      emitcode ("mov", "a,b");
      lbl = newiTempLabel (NULL);
      if (runtimeSign)
        emitcode ("jnb", "F0,!tlabel", labelKey2num (lbl->key));
      emitcode ("cpl", "a");    /* 2's complement */
      emitcode ("inc", "a");
      emitLabel (lbl);

      aopPut (result, "a", 0);
      if (size > 0)
        {
          /* msb is 0x00 or 0xff depending on the sign */
          if (runtimeSign)
            {
              emitcode ("mov", "c,F0");
              emitcode ("subb", "a,acc");
              while (size--)
                aopPut (result, "a", offset++);
            }
          else                  /* compiletimeSign */
            while (size--)
              aopPut (result, "#0xff", offset++);
        }
    }
  else
    {
      aopPut (result, "b", 0);
      while (size--)
        aopPut (result, zero, offset++);
    }

  popB (pushedB);
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
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  /* special cases first */
  /* both are bits */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      genModbits (left, right, result);
      goto release;
    }

  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
    {
      genModOneByte (left, right, result);
      goto release;
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
genIfxJump (iCode * ic, const char *jval, operand * left, operand * right, operand * result, iCode * popIc)
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
    emitcode (inst, "%s,!tlabel", jval, labelKey2num (tlbl->key));
  else
    emitcode (inst, "!tlabel", labelKey2num (tlbl->key));
  freeForBranchAsmops (result, right, left, ic);
  emitcode ("ljmp", "!tlabel", labelKey2num (jlbl->key));
  emitLabel (tlbl);

  /* mark the icode as generated */
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genCmp :- greater or less than comparison                       */
/*-----------------------------------------------------------------*/
static void
genCmp (operand * left, operand * right, operand * result, iCode * ifx, int sign, iCode * ic)
{
  int size, offset = 0;
  unsigned long long lit = 0L;
  bool rightInB;

  D (emitcode (";", "genCmp"));

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
          emitpush (aopGet (right, offset++, FALSE, TRUE));
        }
      loadDptrFromOperand (left, TRUE);
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
          char *l = Safe_strdup (aopGet (left, offset, FALSE, FALSE));
          symbol *lbl = newiTempLabel (NULL);
          emitcode ("cjne", "%s,%s,!tlabel", l, aopGet (right, offset, FALSE, FALSE), labelKey2num (lbl->key));
          Safe_free (l);
          emitLabel (lbl);
        }
      else
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              lit = ullFromVal (AOP (right)->aopu.aop_lit);
              /* optimize if(x < 0) or if(x >= 0) */
              if (lit == 0ll)
                {
                  if (!sign)
                    {
                      CLRC;
                    }
                  else
                    {
                      MOVA (aopGet (left, AOP_SIZE (left) - 1, FALSE, FALSE));
                      if (!(AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result)) && ifx)
                        {
                          genIfxJump (ifx, "acc.7", left, right, result, ic->next);
                          freeAsmop (right, NULL, ic, TRUE);
                          freeAsmop (left, NULL, ic, TRUE);

                          return;
                        }
                      else
                        {
                          emitcode ("rlc", "a");
                        }
                    }
                  goto release;
                }
              else
                {
                  //nonzero literal
                  int bytelit = ((lit >> (offset * 8)) & 0x0ffll);
                  while (size && (bytelit == 0))
                    {
                      offset++;
                      bytelit = ((lit >> (offset * 8)) & 0x0ffll);
                      size--;
                    }
                  CLRC;
                  while (size--)
                    {
                      MOVA (aopGet (left, offset, FALSE, FALSE));
                      if (sign && size == 0)
                        {
                          emitcode ("xrl", "a,#0x80");
                          emitcode ("subb", "a,#0x%02x", 0x80 ^ (unsigned int) ((lit >> (offset * 8)) & 0x0ffll));
                        }
                      else
                        {
                          emitcode ("subb", "a,%s", aopGet (right, offset, FALSE, FALSE));
                        }
                      offset++;
                    }
                  goto release;
                }
            }
          CLRC;
          while (size--)
            {
              bool pushedB = FALSE;
              rightInB = aopGetUsesAcc (right, offset);
              if (rightInB)
                {
                  pushedB = pushB ();
                  emitcode ("mov", "b,%s", aopGet (right, offset, FALSE, FALSE));
                }
              MOVA (aopGet (left, offset, FALSE, FALSE));
              if (sign && size == 0)
                {
                  emitcode ("xrl", "a,#0x80");
                  if (!rightInB)
                    {
                      pushedB = pushB ();
                      rightInB++;
                      MOVB (aopGet (right, offset, FALSE, FALSE));
                    }
                  emitcode ("xrl", "b,#0x80");
                  emitcode ("subb", "a,b");
                }
              else
                {
                  if (rightInB)
                    emitcode ("subb", "a,b");
                  else
                    emitcode ("subb", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              if (rightInB)
                popB (pushedB);
              offset++;
            }
        }
    }

release:
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
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
          genIfxJump (ifx, "c", NULL, NULL, result, ic->next);
        }
      else
        {
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
  int sign = 0;

  D (emitcode (";", "genCmpGt"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);

  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    {
      letype = getSpec (operandType (left));
      retype = getSpec (operandType (right));
      sign = !((SPEC_USIGN (letype) && !(IS_CHAR (letype) && IS_LITERAL (letype))) ||
               (SPEC_USIGN (retype) && !(IS_CHAR (retype) && IS_LITERAL (retype))));
    }
  /* assign the asmops */
  aopOp (result, ic, TRUE);
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);

  genCmp (right, left, result, ifx, sign, ic);

  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genCmpLt - less than comparisons                                */
/*-----------------------------------------------------------------*/
static void
genCmpLt (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  sym_link *letype, *retype;
  int sign = 0;

  D (emitcode (";", "genCmpLt"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);

  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    {
      letype = getSpec (operandType (left));
      retype = getSpec (operandType (right));
      sign = !((SPEC_USIGN (letype) && !(IS_CHAR (letype) && IS_LITERAL (letype))) ||
               (SPEC_USIGN (retype) && !(IS_CHAR (retype) && IS_LITERAL (retype))));
    }
  /* assign the asmops */
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  genCmp (left, right, result, ifx, sign, ic);

  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* gencjneshort - compare and jump if not equal                    */
/*-----------------------------------------------------------------*/
static void
gencjneshort (operand * left, operand * right, symbol * lbl)
{
  int size = max (AOP_SIZE (left), AOP_SIZE (right));
  int offset = 0;

  D (emitcode (";", "gencjneshort"));

  /* if the left side is a immediate/literal or
     if the right is in a pointer register and left is not */
  if (IS_AOP_IMMEDIATE (left) ||
      (AOP_TYPE (left) == AOP_DIR && !IS_AOP_IMMEDIATE (right)) ||
      (IS_AOP_PREG (right) && !IS_AOP_PREG (left)))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  /* generic pointers require special handling since all NULL pointers must compare equal */
  if (opIsGptr (left) || opIsGptr (right))
    {
      /* push right */
      while (offset < size)
        {
          emitpush (aopGet (right, offset++, FALSE, TRUE));
        }
      loadDptrFromOperand (left, TRUE);
      emitcode ("lcall", "___gptr_cmp");
      for (offset = 0; offset < GPTRSIZE; offset++)
        emitpop (NULL);
      emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));
    }

  /* if the right side is a literal then anything goes */
  else if (IS_AOP_IMMEDIATE (right) &&
           AOP_TYPE (left) != AOP_DIR && !IS_AOP_IMMEDIATE (left))
    {
      while (size--)
        {
          char *l = Safe_strdup (aopGet (left, offset, FALSE, FALSE));
          const char *r = aopGet (right, offset, FALSE, FALSE);
          if (EQ (l, "a") && EQ (r, zero))
            emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));
          else
            emitcode ("cjne", "%s,%s,!tlabel", l, r, labelKey2num (lbl->key));
          Safe_free (l);
          offset++;
        }
    }

  /* if the right side is in a register or in direct space or
     if the left is a pointer register & right is not */
  else if (AOP_TYPE (right) == AOP_REG ||
           AOP_TYPE (right) == AOP_DIR ||
           IS_AOP_IMMEDIATE (right) ||
           (IS_AOP_PREG (left) && !IS_AOP_PREG (right)))
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          int   val[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
          bool  chk[8]  = {TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
          int   rsize   = AOP_SIZE (right);
          int   pidx    = -1;
          int   lidx, cidx;
          unsigned long long lit = ullFromVal (AOP (right)->aopu.aop_lit);

          switch (rsize)
            {
            case 8:
              val[7]  = (lit >> 56) & 0xff;
              chk[7]  = TRUE;
              val[6]  = (lit >> 48) & 0xff;
              chk[6]  = TRUE;
              val[5]  = (lit >> 40) & 0xff;
              chk[5]  = TRUE;
              val[4]  = (lit >> 32) & 0xff;
              chk[4]  = TRUE;
              //fallthrough
            case 4:
              val[3]  = (lit >> 24) & 0xff;
              chk[3]  = TRUE;
              //fallthrough
            case 3:
              val[2]  = (lit >> 16) & 0xff;
              chk[2]  = TRUE;
              //fallthrough
            case 2:
              val[1]  = (lit >> 8) & 0xff;
              chk[1]  = TRUE;
              //fallthrough
            default:
              val[0]  = (lit >> 0) & 0xff;
              chk[0]  = TRUE;
            }
          if (optimize.codeSize && (rsize > 1))
            {
              if ((!chk[0] || val[0] == 0x00) &&
                  (!chk[1] || val[1] == 0x00) &&
                  (!chk[2] || val[2] == 0x00) &&
                  (!chk[3] || val[3] == 0x00) &&
                  (!chk[4] || val[4] == 0x00) &&
                  (!chk[5] || val[5] == 0x00) &&
                  (!chk[6] || val[6] == 0x00) &&
                  (!chk[7] || val[7] == 0x00))
                {
                  MOVA(aopGet(left, 0, FALSE, FALSE));
                  for (cidx = 1; cidx < size; cidx++)
                    if (chk[cidx])
                      emitcode ("orl", "a,%s", aopGet(left, cidx, FALSE, FALSE));
                  emitcode ("jnz", "%05d$", lbl->key + 100);
                  return;
                }
              if ((!chk[0] || val[0] == 0xFF) &&
                  (!chk[1] || val[1] == 0xFF) &&
                  (!chk[2] || val[2] == 0xFF) &&
                  (!chk[3] || val[3] == 0xFF) &&
                  (!chk[4] || val[4] == 0xFF) &&
                  (!chk[5] || val[5] == 0xFF) &&
                  (!chk[6] || val[6] == 0xFF) &&
                  (!chk[7] || val[7] == 0xFF))
                {
                  MOVA(aopGet(left, 0, FALSE, FALSE));
                  for (cidx = 1; cidx < size; cidx++)
                    if (chk[cidx])
                      emitcode ("anl", "a,%s", aopGet(left, cidx, FALSE, FALSE));
                  emitcode ("cjne", "a,#0xFF,%05d$", lbl->key + 100);
                  return;
                }
            }

          for (lidx = 0; lidx < size; lidx++)
            {
              if (chk[lidx] && !aopGetUsesAcc(left, lidx))
                {
                  if (pidx >= 0)
                    {
                      if (val[pidx] == val[lidx])
                        {
                          chk[lidx] = FALSE;
                        }
                      if ((~val[pidx] & 0xff) == val[lidx])
                        {
                          emitcode ("cpl", "a");
                          chk[lidx] = FALSE;
                        }
                      else if (((val[pidx] + 1) & 0xff) == val[lidx])
                        {
                          emitcode ("inc", "a");
                          chk[lidx] = FALSE;
                        }
                      else if (((val[pidx] - 1) & 0xff) == val[lidx])
                        {
                          emitcode ("dec", "a");
                          chk[lidx] = FALSE;
                        }
                    }
                  pidx = lidx;
                  if (chk[lidx])
                    {
                      MOVA(aopGet(right, lidx, FALSE, FALSE));
                      chk[lidx] = FALSE;
                    }
                  emitcode ("cjne", "a,%s,%05d$", aopGet(left, lidx, FALSE, TRUE), lbl->key + 100);

                  for (cidx = lidx + 1; cidx < size; cidx++)
                    {
                      if (chk[cidx] && val[lidx] == val[cidx] && !IS_AOP_PREG (left))
                        {
                          chk[cidx] = FALSE;
                          emitcode ("cjne", "a,%s,%05d$", aopGet(left, cidx, FALSE, TRUE), lbl->key + 100);
                        }
                    }
                }
            }
          return;
        }

      while (size--)
        {
          const char *r;
          MOVA (aopGet (left, offset, FALSE, FALSE));
          r = aopGet (right, offset, FALSE, TRUE);
          if (EQ (r, zero))
            emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));
          else
            emitcode ("cjne", "a,%s,!tlabel", r, labelKey2num (lbl->key));
          offset++;
        }
    }
  else
    {
      /* right is a pointer reg need both a & b */
      while (size--)
        {
          //if B in use: push B; mov B,left; mov A,right; clrc; subb A,B; pop B; jnz
          wassertl (!BINUSE, "B was in use");
          MOVB (aopGet (left, offset, FALSE, FALSE));
          MOVA (aopGet (right, offset, FALSE, FALSE));
          emitcode ("cjne", "a,b,!tlabel", labelKey2num (lbl->key));
          offset++;
        }
    }
}

/*-----------------------------------------------------------------*/
/* gencjne - compare and jump if not equal                         */
/*-----------------------------------------------------------------*/
static void
gencjne (operand * left, operand * right, symbol * lbl, bool useCarry)
{
  symbol *tlbl = newiTempLabel (NULL);

  D (emitcode (";", "gencjne"));

  gencjneshort (left, right, lbl);

  if (useCarry)
    SETC;
  else
    MOVA (one);
  emitcode ("sjmp", "!tlabel", labelKey2num (tlbl->key));
  emitLabel (lbl);
  if (useCarry)
    CLRC;
  else
    MOVA (zero);
  emitLabel (tlbl);
}

/*-----------------------------------------------------------------*/
/* genCmpEq - generates code for equal to                          */
/*-----------------------------------------------------------------*/
static void
genCmpEq (iCode * ic, iCode * ifx)
{
  bool swappedLR = FALSE;
  operand *left, *right, *result;
  iCode *popIc = ic->next;

  D (emitcode (";", "genCmpEq"));

  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  /* if literal, literal on the right or
     if the right is in a pointer register and left
     is not */
  if ((AOP_TYPE (left) == AOP_LIT) || (IS_AOP_PREG (right) && !IS_AOP_PREG (left)))
    {
      swapOperands (&left, &right);
      swappedLR = TRUE;
    }

  if (ifx && !AOP_SIZE (result))
    {
      symbol *tlbl;
      /* if they are both bit variables */
      if (AOP_TYPE (left) == AOP_CRY && ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (right) == AOP_LIT)))
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              unsigned long lit = ulFromVal (AOP (right)->aopu.aop_lit);
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
              emitcode ("jb", "%s,!tlabel", AOP (right)->aopu.aop_dir, labelKey2num ((lbl->key)));
              emitcode ("cpl", "c");
              emitLabel (lbl);
            }
          /* if true label then we jump if condition
             supplied is true */
          tlbl = newiTempLabel (NULL);
          if (IC_TRUE (ifx))
            {
              emitcode ("jnc", "!tlabel", labelKey2num (tlbl->key));
              freeForBranchAsmops (result, right, left, ic);
              popForBranch (popIc, FALSE);
              emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ifx)->key));
            }
          else
            {
              emitcode ("jc", "!tlabel", labelKey2num (tlbl->key));
              freeForBranchAsmops (result, right, left, ic);
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
              freeForBranchAsmops (result, right, left, ic);
              popForBranch (popIc, FALSE);
              emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ifx)->key));
              emitLabel (tlbl);
            }
          else
            {
              symbol *lbl = newiTempLabel (NULL);
              emitcode ("sjmp", "!tlabel", labelKey2num (lbl->key));
              emitLabel (tlbl);
              freeForBranchAsmops (result, right, left, ic);
              popForBranch (popIc, FALSE);
              emitcode ("ljmp", "!tlabel", labelKey2num (IC_FALSE (ifx)->key));
              emitLabel (lbl);
            }
        }
      /* mark the icode as generated */
      ifx->generated = 1;
      goto release;
    }

  /* if they are both bit variables */
  if (AOP_TYPE (left) == AOP_CRY && ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (right) == AOP_LIT)))
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          unsigned long lit = ulFromVal (AOP (right)->aopu.aop_lit);
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
      /* c = 1 if egal */
      if (AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result))
        {
          outBitC (result);
          goto release;
        }
      if (ifx)
        {
          genIfxJump (ifx, "c", left, right, result, popIc);
          goto release;
        }
      /* if the result is used in an arithmetic operation
         then put the result in place */
      outBitC (result);
    }
  else
    {
      bool useCarry = (AOP_TYPE (result) == AOP_CRY);
      gencjne (left, right, newiTempLabel (NULL), useCarry);
      if (useCarry && AOP_SIZE (result))
        {
          aopPut (result, "c", 0);
          goto release;
        }
      if (ifx)
        {
          genIfxJump (ifx, useCarry ? "c" : "a", left, right, result, popIc);
          goto release;
        }
      /* if the result is used in an arithmetic operation
         then put the result in place */
      if (!useCarry)
        outAcc (result);
      /* leave the result in acc */
    }

release:
  freeAsmop (result, NULL, ic, TRUE);
  if (swappedLR)
    swapOperands (&left, &right);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
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
  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, FALSE);

  /* if both are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
      emitcode ("anl", "c,%s", AOP (right)->aopu.aop_dir);
      outBitC (result);
    }
  else
    {
      tlbl = newiTempLabel (NULL);
      toBoolean (left);
      emitcode ("jz", "!tlabel", labelKey2num (tlbl->key));
      toBoolean (right);
      emitLabel (tlbl);
      outBitAcc (result);
    }

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
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
  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, FALSE);

  /* if both are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
      emitcode ("orl", "c,%s", AOP (right)->aopu.aop_dir);
      outBitC (result);
    }
  else
    {
      tlbl = newiTempLabel (NULL);
      toBoolean (left);
      emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
      toBoolean (right);
      emitLabel (tlbl);
      outBitAcc (result);
    }

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
}

/*-----------------------------------------------------------------*/
/* isLiteralBit - test if lit == 2^n                               */
/*-----------------------------------------------------------------*/
static int
isLiteralBit (unsigned long long lit)
{
  unsigned long long w = 1;
  int idx;

  for (idx = 0; idx < 64; idx++, w<<=1)
    if (lit == w)
      return idx + 1;
  return 0;
}

/*-----------------------------------------------------------------*/
/* continueIfTrue -                                                */
/*-----------------------------------------------------------------*/
static void
continueIfTrue (iCode * ic, iCode * popIc)
{
  if (IC_TRUE (ic))
    {
      popForBranch (popIc, TRUE);
      emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ic)->key));
    }
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* jmpIfTrue -                                                     */
/*-----------------------------------------------------------------*/
static void
jumpIfTrue (iCode * ic, iCode * popIc)
{
  if (!IC_TRUE (ic))
    {
      popForBranch (popIc, TRUE);
      emitcode ("ljmp", "!tlabel", labelKey2num (IC_FALSE (ic)->key));
    }
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* jmpTrueOrFalse -                                                */
/*-----------------------------------------------------------------*/
static void
jmpTrueOrFalse (iCode * ic, symbol * tlbl, operand * left, operand * right, operand * result, iCode * popIc)
{
  // ugly but optimized by peephole
  if (IC_TRUE (ic))
    {
      symbol *nlbl = newiTempLabel (NULL);
      emitcode ("sjmp", "!tlabel", labelKey2num (nlbl->key));
      emitLabel (tlbl);
      popForBranch (popIc, FALSE);
      freeForBranchAsmops (result, right, left, ic);
      emitcode ("ljmp", "!tlabel", labelKey2num (IC_TRUE (ic)->key));
      emitLabel (nlbl);
    }
  else
    {
      popForBranch (popIc, FALSE);
      freeForBranchAsmops (result, right, left, ic);
      emitcode ("ljmp", "!tlabel", labelKey2num (IC_FALSE (ic)->key));
      emitLabel (tlbl);
    }
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genAnd  - code for and                                          */
/*-----------------------------------------------------------------*/
static void
genAnd (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0ull;
  int bytelit = 0;

  D (emitcode (";", "genAnd"));

  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

#ifdef DEBUG_TYPE
  emitcode (";", "Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right));
  emitcode (";", "Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right));
#endif

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || AOP_NEEDSACC (left))
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
                  jumpIfTrue (ifx, ic->next);
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
              MOVA (aopGet (right, 0, FALSE, FALSE));
              emitcode ("anl", "c,acc.0");
            }
          else
            {
              // c = bit & val;
              MOVA (aopGet (right, 0, FALSE, FALSE));
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
        genIfxJump (ifx, "c", left, right, result, ic->next);
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
          MOVA (aopGet (left, posbit >> 3, FALSE, FALSE));
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
                  emitcode ("mov", "c,acc.%d", posbit & 0x07);
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
                  dbuf_printf (&dbuf, "acc.%d", posbit & 0x07);
                  genIfxJump (ifx, dbuf_c_str (&dbuf), left, right, result, ic->next);
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
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  // byte ==  2^n ?
                  if ((posbit = isLiteralBit (bytelit)) != 0)
                    emitcode ("jb", "acc.%d,!tlabel", (posbit - 1) & 0x07, labelKey2num (tlbl->key));
                  else
                    {
                      if (bytelit != 0x0FFL)
                        emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, TRUE));
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
                jmpTrueOrFalse (ifx, tlbl, left, right, result, ic->next);
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
              bytelit = (int) ((lit >> (offset * 8)) & 0x0ffull);
              if (bytelit == 0x0FF)
                {
                  /* dummy read of volatile operand */
                  if (isOperandVolatile (left, FALSE))
                    MOVA (aopGet (left, offset, FALSE, FALSE));
                  else
                    continue;
                }
              else if (bytelit == 0)
                {
                  aopPut (result, zero, offset);
                }
              else if (IS_AOP_PREG (result))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  aopPut (result, "a", offset);
                }
              else if (AOP_TYPE (left) != AOP_DPTR)
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE));
                  emitcode ("anl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
                  Safe_free (l);
                }
              else
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE));
                  emitcode ("anl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
                  Safe_free (l);
                  aopPut (result, "a", offset);
                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  if (offset)
                    emitcode ("mov", "b,a");
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("anl", "a,b");
                  aopPut (result, "a", offset);
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  aopPut (result, "a", offset);
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  if (IS_AOP_PREG (result))
                    {
                      emitcode ("anl", "a,%s", aopGet (left, offset, FALSE, TRUE));
                      aopPut (result, "a", offset);
                    }
                  else
                    {
                      emitcode ("anl", "%s,a", aopGet (left, offset, FALSE, TRUE));
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
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("anl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("anl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("anl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("anl", "a,%s", aopGet (left, offset, FALSE, FALSE));
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
            jmpTrueOrFalse (ifx, tlbl, left, right, result, ic->next);
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
                  bytelit = (int) ((lit >> (offset * 8)) & 0x0ffull);
                  if (bytelit == 0x0FF)
                    {
                      aopPut (result, aopGet (left, offset, FALSE, FALSE), offset);
                      continue;
                    }
                  else if (bytelit == 0)
                    {
                      /* dummy read of volatile operand */
                      if (isOperandVolatile (left, FALSE))
                        MOVA (aopGet (left, offset, FALSE, FALSE));
                      aopPut (result, zero, offset);
                      continue;
                    }
                  else if (AOP_TYPE (left) == AOP_ACC)
                    {
                      char *l = Safe_strdup (aopGet (left, offset, FALSE, FALSE));
                      emitcode ("anl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
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
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("anl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("anl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("anl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("anl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("anl", "a,%s", aopGet (left, offset, FALSE, FALSE));
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
genOr (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long lit = 0L;
  int bytelit = 0;

  D (emitcode (";", "genOr"));

  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

#ifdef DEBUG_TYPE
  emitcode (";", "Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right));
  emitcode (";", "Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right));
#endif

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || AOP_NEEDSACC (left))
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
      lit = ulFromVal (AOP (right)->aopu.aop_lit);
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
                    continueIfTrue (ifx, ic->next);
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
              if ((AOP_TYPE (result) == AOP_CRY) && ifx)
                {
                  symbol *tlbl = newiTempLabel (NULL);
                  emitcode ("jb", "%s,!tlabel", AOP (left)->aopu.aop_dir, labelKey2num (tlbl->key));
                  toBoolean (right);
                  emitcode ("jnz", "!tlabel", labelKey2num (tlbl->key));
                  jmpTrueOrFalse (ifx, tlbl, left, right, result, ic->next);
                  goto release;
                }
              else
                {
                  toCarry (right);
                  emitcode ("orl", "c,%s", AOP (left)->aopu.aop_dir);
                }
            }
        }
      // bit = c
      // val = c
      if (size)
        outBitC (result);
      // if(bit | ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c", left, right, result, ic->next);
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
            continueIfTrue (ifx, ic->next);
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
              genIfxJump (ifx, "a", left, right, result, ic->next);
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
                    MOVA (aopGet (left, offset, FALSE, FALSE));
                  else
                    continue;
                }
              else if (bytelit == 0x0FF)
                {
                  /* dummy read of volatile operand */
                  if (isOperandVolatile (left, FALSE))
                    MOVA (aopGet (left, offset, FALSE, FALSE));
                  aopPut (result, "#0xff", offset);
                }
              else if (IS_AOP_PREG (left))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  aopPut (result, "a", offset);
                }
              else if (AOP_TYPE (left) != AOP_DPTR)
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE));
                  emitcode ("orl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
                  Safe_free (l);
                }
              else
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE));
                  emitcode ("orl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
                  Safe_free (l);
                  aopPut (result, "a", offset);
                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  if (offset)
                    emitcode ("mov", "b,a");
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("orl", "a,b");
                  aopPut (result, "a", offset);
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  aopPut (result, "a", offset);
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  if (IS_AOP_PREG (left))
                    {
                      emitcode ("orl", "a,%s", aopGet (left, offset, FALSE, TRUE));
                      aopPut (result, "a", offset);
                    }
                  else
                    {
                      emitcode ("orl", "%s,a", aopGet (left, offset, FALSE, TRUE));
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
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("orl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("orl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("orl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("orl", "a,%s", aopGet (left, offset, FALSE, FALSE));
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
            jmpTrueOrFalse (ifx, tlbl, left, right, result, ic->next);
          else
            emitLabel (tlbl);
        }
      else
        {
          for (; (size--); offset++)
            {
              // normal case
              // result = left | right
              if (AOP_TYPE (right) == AOP_LIT)
                {
                  bytelit = (int) ((lit >> (offset * 8)) & 0x0FFL);
                  if (bytelit == 0)
                    {
                      aopPut (result, aopGet (left, offset, FALSE, FALSE), offset);
                      continue;
                    }
                  else if (bytelit == 0x0FF)
                    {
                      /* dummy read of volatile operand */
                      if (isOperandVolatile (left, FALSE))
                        MOVA (aopGet (left, offset, FALSE, FALSE));
                      aopPut (result, "#0xff", offset);
                      continue;
                    }
                  else if (AOP_TYPE (left) == AOP_ACC)
                    {
                      // this should be the only use of left so A,B can be overwritten
                      char *l = Safe_strdup (aopGet (left, offset, FALSE, FALSE));
                      emitcode ("orl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
                      aopPut (result, l, offset);
                      Safe_free (l);
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
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("orl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("orl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("orl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("orl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("orl", "a,%s", aopGet (left, offset, FALSE, FALSE));
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
/* genXor - code for xclusive or                                   */
/*-----------------------------------------------------------------*/
static void
genXor (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long lit = 0L;
  int bytelit = 0;

  D (emitcode (";", "genXor"));

  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

#ifdef DEBUG_TYPE
  emitcode (";", "Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right));
  emitcode (";", "Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right));
#endif

  /* if left is a literal & right is not ||
     if left needs acc & right does not */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || (AOP_NEEDSACC (left) && !AOP_NEEDSACC (right)))
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
      lit = ulFromVal (AOP (right)->aopu.aop_lit);
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
                    continueIfTrue (ifx, ic->next);
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
        genIfxJump (ifx, "c", left, right, result, ic->next);
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
                    MOVA (aopGet (left, offset, FALSE, FALSE));
                  else
                    continue;
                }
              else if (IS_AOP_PREG (left))
                {
                  MOVA (aopGet (left, offset, FALSE, TRUE));
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  aopPut (result, "a", offset);
                }
              else if (AOP_TYPE (left) != AOP_DPTR)
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE));
                  emitcode ("xrl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
                  Safe_free (l);
                }
              else
                {
                  char *l = Safe_strdup (aopGet (left, offset, FALSE, TRUE));
                  emitcode ("xrl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
                  Safe_free (l);
                  aopPut (result, "a", offset);
                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  if (offset)
                    emitcode ("mov", "b,a");
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("xrl", "a,b");
                  aopPut (result, "a", offset);
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                  aopPut (result, "a", offset);
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  if (IS_AOP_PREG (left))
                    {
                      emitcode ("xrl", "a,%s", aopGet (left, offset, FALSE, TRUE));
                      aopPut (result, "a", offset);
                    }
                  else
                    {
                      emitcode ("xrl", "%s,a", aopGet (left, offset, FALSE, TRUE));
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
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                }
              else if ((AOP_TYPE (right) == AOP_REG || IS_AOP_PREG (right) || AOP_TYPE (right) == AOP_DIR)
                       && AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("xrl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("xrl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("xrl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("xrl", "a,%s", aopGet (left, offset, FALSE, FALSE));
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
            jmpTrueOrFalse (ifx, tlbl, left, right, result, ic->next);
          else
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
                      aopPut (result, aopGet (left, offset, FALSE, FALSE), offset);
                      continue;
                    }
                  else if (AOP_TYPE (left) == AOP_ACC)
                    {
                      // this should be the only use of left so A,B can be overwritten
                      char *l = Safe_strdup (aopGet (left, offset, FALSE, FALSE));
                      emitcode ("xrl", "%s,%s", l, aopGet (right, offset, FALSE, FALSE));
                      aopPut (result, l, offset);
                      Safe_free (l);
                      continue;
                    }
                }
              // faster than result <- left, xrl result,right
              // and better if result is SFR
              if ((AOP_TYPE (right) == AOP_REG || IS_AOP_PREG (right) || AOP_TYPE (right) == AOP_DIR)
                  && AOP_TYPE (left) == AOP_ACC)
                {
                  if (offset)
                    emitcode ("mov", "a,b");
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else if (AOP_TYPE (left) == AOP_ACC)
                {
                  if (!offset)
                    {
                      //B contains high byte of left
                      emitpush ("b");
                      emitcode ("mov", "b,a");
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("xrl", "a,b");
                      emitpop ("b");
                    }
                  else
                    {
                      MOVA (aopGet (right, offset, FALSE, FALSE));
                      emitcode ("xrl", "a,b");
                    }
                }
              else if (aopGetUsesAcc (left, offset) && aopGetUsesAcc (right, offset))
                {
                  MOVB (aopGet (left, offset, FALSE, FALSE));
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("xrl", "a,b");
                }
              else if (aopGetUsesAcc (left, offset))
                {
                  MOVA (aopGet (left, offset, FALSE, FALSE));
                  emitcode ("xrl", "a,%s", aopGet (right, offset, FALSE, FALSE));
                }
              else
                {
                  MOVA (aopGet (right, offset, FALSE, FALSE));
                  emitcode ("xrl", "a,%s", aopGet (left, offset, FALSE, FALSE));
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
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  /* move it to the result */
  size = AOP_SIZE (result);
  offset = size - 1;
  if (size == 1)
    {
      /* special case for 1 byte */
      MOVA (aopGet (left, offset, FALSE, FALSE));
      emitcode ("rr", "a");
      goto release;
    }
  /* no need to clear carry, bit7 will be written later */
  while (size--)
    {
      MOVA (aopGet (left, offset, FALSE, FALSE));
      emitcode ("rrc", "a");
      if (AOP_SIZE (result) > 1)
        aopPut (result, "a", offset--);
    }
  /* now we need to put the carry into the
     highest order byte of the result */
  if (AOP_SIZE (result) > 1)
    {
      MOVA (aopGet (result, AOP_SIZE (result) - 1, FALSE, FALSE));
    }
  emitcode ("mov", "acc.7,c");
release:
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
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  /* move it to the result */
  size = AOP_SIZE (result);
  offset = 0;
  if (size--)
    {
      MOVA (aopGet (left, offset, FALSE, FALSE));
      if (size == 0)
        {
          /* special case for 1 byte */
          emitcode ("rl", "a");
          goto release;
        }
      emitcode ("rlc", "a");    /* bit0 will be written later */
      if (AOP_SIZE (result) > 1)
        {
          aopPut (result, "a", offset++);
        }

      while (size--)
        {
          MOVA (aopGet (left, offset, FALSE, FALSE));
          emitcode ("rlc", "a");
          if (AOP_SIZE (result) > 1)
            aopPut (result, "a", offset++);
        }
    }
  /* now we need to put the carry into the
     highest order byte of the result */
  if (AOP_SIZE (result) > 1)
    {
      MOVA (aopGet (result, 0, FALSE, FALSE));
    }
  emitcode ("mov", "acc.0,c");
release:
  aopPut (result, "a", 0);
  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGetAbit - generates code get a single bit                    */
/*-----------------------------------------------------------------*/
static void
genGetAbit (iCode * ic)
{
  operand *left, *right, *result;
  int shCount;

  D (emitcode (";", "genGetAbit"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, FALSE);

  shCount = (int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);

  /* get the needed byte into a */
  MOVA (aopGet (left, shCount / 8, FALSE, FALSE));
  shCount %= 8;
  if (AOP_TYPE (result) == AOP_CRY)
    {
      if ((shCount) == 7)
        emitcode ("rlc", "a");
      else if ((shCount) == 0)
        emitcode ("rrc", "a");
      else
        emitcode ("mov", "c,acc[%d]", shCount);
      outBitC (result);
    }
  else
    {
      switch (shCount)
        {
        case 2:
          emitcode ("rr", "a");
          //fallthrough
        case 1:
          emitcode ("rr", "a");
          //fallthrough
        case 0:
          emitcode ("anl", "a,#0x01");
          break;
        case 3:
        case 5:
          emitcode ("mov", "c,acc[%d]", shCount);
          emitcode ("clr", "a");
          emitcode ("rlc", "a");
          break;
        case 4:
          emitcode ("swap", "a");
          emitcode ("anl", "a,#0x01");
          break;
        case 6:
          emitcode ("rl", "a");
          //fallthrough
        case 7:
          emitcode ("rl", "a");
          emitcode ("anl", "a,#0x01");
          break;
        }
      outAcc (result);
    }

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGetByte - generates code get a single byte                   */
/*-----------------------------------------------------------------*/
static void
genGetByte (iCode * ic)
{
  operand *left, *right, *result;
  int offset;

  D (emitcode (";", "genGetByte"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, FALSE);

  offset = (int) ulFromVal (AOP (right)->aopu.aop_lit) / 8;
  aopPut (result, aopGet (left, offset, FALSE, FALSE), 0);

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGetWord - generates code get two bytes                       */
/*-----------------------------------------------------------------*/
static void
genGetWord (iCode * ic)
{
  operand *left, *right, *result;
  int offset;

  D (emitcode (";", "genGetWord"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, FALSE);

  offset = (int) ulFromVal (AOP (right)->aopu.aop_lit) / 8;
  aopPut (result, aopGet (left, offset, FALSE, FALSE), 0);
  aopPut (result, aopGet (left, offset + 1, FALSE, FALSE), 1);

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
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
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  switch (AOP_SIZE (left))
    {
    case 1:                    /* swap nibbles in byte */
      MOVA (aopGet (left, 0, FALSE, FALSE));
      emitcode ("swap", "a");
      aopPut (result, "a", 0);
      break;
    case 2:                    /* swap bytes in word */
      if (AOP_TYPE (left) == AOP_REG && sameRegs (AOP (left), AOP (result)))
        {
          MOVA (aopGet (left, 0, FALSE, FALSE));
          aopPut (result, aopGet (left, 1, FALSE, FALSE), 0);
          aopPut (result, "a", 1);
        }
      else if (operandsEqu (left, result))
        {
          char *reg = "a";
          bool pushedB = FALSE, leftInB = FALSE;

          MOVA (aopGet (left, 0, FALSE, FALSE));
          if (aopGetUsesAcc (left, 1) || aopGetUsesAcc (result, 0))
            {
              pushedB = pushB ();
              emitcode ("mov", "b,a");
              reg = "b";
              leftInB = TRUE;
            }
          aopPut (result, aopGet (left, 1, FALSE, FALSE), 0);
          aopPut (result, reg, 1);

          if (leftInB)
            popB (pushedB);
        }
      else
        {
          aopPut (result, aopGet (left, 1, FALSE, FALSE), 0);
          aopPut (result, aopGet (left, 0, FALSE, FALSE), 1);
        }
      break;
    default:
      wassertl (FALSE, "unsupported SWAP operand size");
    }

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
          emitcode ("mov", "c,acc.7");
          emitcode ("rrc", "a");
        }
      else if (shCount == 2)
        {
          emitcode ("mov", "c,acc.7");
          emitcode ("rrc", "a");
          emitcode ("mov", "c,acc.7");
          emitcode ("rrc", "a");
        }
      else
        {
          tlbl = newiTempLabel (NULL);
          /* rotate right accumulator */
          AccRol (8 - shCount);
          /* and kill the higher order bits */
          emitcode ("anl", "a,#!constbyte", SRMask[shCount]);
          emitcode ("jnb", "acc.%d,!tlabel", 7 - shCount, labelKey2num (tlbl->key));
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
  MOVA (aopGet (left, offl, FALSE, FALSE));
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
  MOVA (aopGet (left, offl, FALSE, FALSE));
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
      l = aopGet (left, offl, FALSE, FALSE);

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
      emitcode ("mov", "c,acc.0");      // c = B
      emitcode ("xch", "a,%s", x);      // CCCCCCDD:000000BB
      emitcode ("rrc", "a");
      emitcode ("xch", "a,%s", x);
      emitcode ("rrc", "a");
      emitcode ("mov", "c,acc.0");      //<< get correct bit
      emitcode ("xch", "a,%s", x);
      emitcode ("rrc", "a");
      emitcode ("xch", "a,%s", x);
      emitcode ("rrc", "a");
      emitcode ("xch", "a,%s", x);
      break;
    case 7:                    // a:x <<= 7
      mask = SRMask[shCount];
      emitcode ("anl", "a,#!constbyte", mask);  // 0000000B:CCCCCCCD
      emitcode ("mov", "c,acc.0");      // c = B
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
      emitcode ("mov", "c,acc.7");
      AccAXLrl1 (x);            // ABBBBBBC:CDDDDDDA
      emitcode ("mov", "c,acc.7");
      AccAXLrl1 (x);            // BBBBBBCC:DDDDDDAA
      emitcode ("xch", "a,%s", x);      // DDDDDDAA:BBBBBBCC
      emitcode ("anl", "a,#!constbyte", mask);  // 000000AA:BBBBBBCC
      break;
    case 7:                    // ABBBBBBB:CDDDDDDD
      emitcode ("mov", "c,acc.7");      // c = A
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
      emitcode ("mov", "c,acc.7");
      AccAXRrl1 (x);            // s->a:x
      break;
    case 2:
      emitcode ("mov", "c,acc.7");
      AccAXRrl1 (x);            // s->a:x
      emitcode ("mov", "c,acc.7");
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
      emitcode ("jnb", "acc.%d,!tlabel", 7 - shCount, labelKey2num (tlbl->key));
      mask = ~SRMask[shCount];
      emitcode ("orl", "a,#!constbyte", mask);  // 111AAAAA:BBBCCCCC
      emitLabel (tlbl);
      break;                    // SSSSAAAA:BBBCCCCC
    case 6:                    // AABBBBBB:CCDDDDDD
      tlbl = newiTempLabel (NULL);
      emitcode ("mov", "c,acc.7");
      AccAXLrl1 (x);            // ABBBBBBC:CDDDDDDA
      emitcode ("mov", "c,acc.7");
      AccAXLrl1 (x);            // BBBBBBCC:DDDDDDAA
      emitcode ("xch", "a,%s", x);      // DDDDDDAA:BBBBBBCC
      emitcode ("anl", "a,#!constbyte", mask);  // 000000AA:BBBBBBCC
      emitcode ("jnb", "acc.%d,!tlabel", 7 - shCount, labelKey2num (tlbl->key));
      mask = ~SRMask[shCount];
      emitcode ("orl", "a,#!constbyte", mask);  // 111111AA:BBBBBBCC
      emitLabel (tlbl);
      break;
    case 7:                    // ABBBBBBB:CDDDDDDD
      tlbl = newiTempLabel (NULL);
      emitcode ("mov", "c,acc.7");      // c = A
      AccAXLrl1 (x);            // BBBBBBBC:DDDDDDDA
      emitcode ("xch", "a,%s", x);      // DDDDDDDA:BBBBBBCC
      emitcode ("anl", "a,#!constbyte", mask);  // 0000000A:BBBBBBBC
      emitcode ("jnb", "acc.%d,!tlabel", 7 - shCount, labelKey2num (tlbl->key));
      mask = ~SRMask[shCount];
      emitcode ("orl", "a,#!constbyte", mask);  // 1111111A:BBBBBBBC
      emitLabel (tlbl);
      break;
    default:
      break;
    }
}

/*-----------------------------------------------------------------*/
/* shiftL2Left2Result - shift left two bytes from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftL2Left2Result (operand * left, int offl, operand * result, int offr, int shCount)
{
  const char *x;
  bool pushedB = FALSE;
  bool usedB = FALSE;

  if (sameRegs (AOP (result), AOP (left)) && ((offl + MSB16) == offr))
    {
      /* don't crash result[offr] */
      MOVA (aopGet (left, offl, FALSE, FALSE));
      x = xch_a_aopGet (left, offl + MSB16, FALSE, FALSE);
      usedB = !strncmp (x, "b", 1);
    }
  else if (aopGetUsesAcc (result, offr))
    {
      movLeft2Result (left, offl, result, offr, 0);
      pushedB = pushB ();
      usedB = TRUE;
      emitcode ("mov", "b,%s", aopGet (left, offl + MSB16, FALSE, FALSE));
      MOVA (aopGet (result, offr, FALSE, FALSE));
      emitcode ("xch", "a,b");
      x = "b";
    }
  else
    {
      movLeft2Result (left, offl, result, offr, 0);
      MOVA (aopGet (left, offl + MSB16, FALSE, FALSE));
      x = aopGet (result, offr, FALSE, FALSE);
    }
  /* ax << shCount (x = lsb(result)) */
  AccAXLsh (x, shCount);
  if (usedB)
    {
      emitcode ("xch", "a,b");
      aopPut (result, "a", offr);
      aopPut (result, "b", offr + MSB16);
      popB (pushedB);
    }
  else
    {
      aopPut (result, "a", offr + MSB16);
    }
}

/*-----------------------------------------------------------------*/
/* shiftR2Left2Result - shift right two bytes from left to result  */
/*-----------------------------------------------------------------*/
static void
shiftR2Left2Result (operand * left, int offl, operand * result, int offr, int shCount, int sign)
{
  const char *x;
  bool pushedB = FALSE;
  bool usedB = FALSE;

  if (sameRegs (AOP (result), AOP (left)) && ((offl + MSB16) == offr))
    {
      /* don't crash result[offr] */
      MOVA (aopGet (left, offl, FALSE, FALSE));
      x = xch_a_aopGet (left, offl + MSB16, FALSE, FALSE);
      usedB = !strncmp (x, "b", 1);
    }
  else if (aopGetUsesAcc (result, offr))
    {
      movLeft2Result (left, offl, result, offr, 0);
      pushedB = pushB ();
      usedB = TRUE;
      emitcode ("mov", "b,%s", aopGet (result, offr, FALSE, FALSE));
      MOVA (aopGet (left, offl + MSB16, FALSE, FALSE));
      x = "b";
    }
  else
    {
      movLeft2Result (left, offl, result, offr, 0);
      MOVA (aopGet (left, offl + MSB16, FALSE, FALSE));
      x = aopGet (result, offr, FALSE, FALSE);
    }
  /* a:x >> shCount (x = lsb(result)) */
  if (sign)
    AccAXRshS (x, shCount);
  else
    AccAXRsh (x, shCount);
  if (usedB)
    {
      emitcode ("xch", "a,b");
      aopPut (result, "a", offr);
      emitcode ("xch", "a,b");
      popB (pushedB);
    }
  if (getDataSize (result) > 1)
    aopPut (result, "a", offr + MSB16);
}

/*------------------------------------------------------------------*/
/* shiftLLeftOrResult - shift left one byte from left, or to result */
/*------------------------------------------------------------------*/
static void
shiftLLeftOrResult (operand * left, int offl, operand * result, int offr, int shCount)
{
  MOVA (aopGet (left, offl, FALSE, FALSE));
  /* shift left accumulator */
  AccLsh (shCount);
  /* or with result */
  if (aopGetUsesAcc (result, offr))
    {
      emitcode ("xch", "a,b");
      MOVA (aopGet (result, offr, FALSE, FALSE));
      emitcode ("orl", "a,b");
    }
  else
    {
      emitcode ("orl", "a,%s", aopGet (result, offr, FALSE, FALSE));
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
  MOVA (aopGet (left, offl, FALSE, FALSE));
  /* shift right accumulator */
  AccRsh (shCount);
  /* or with result */
  if (aopGetUsesAcc (result, offr))
    {
      emitcode ("xch", "a,b");
      MOVA (aopGet (result, offr, FALSE, FALSE));
      emitcode ("orl", "a,b");
    }
  else
    {
      emitcode ("orl", "a,%s", aopGet (result, offr, FALSE, FALSE));
    }
  /* back to result */
  aopPut (result, "a", offr);
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
      MOVA (aopGet (left, offl, FALSE, FALSE));
      emitcode ("add", "a,acc");
      if (useXch)
        xch_a_aopGet (left, offl + offr, FALSE, FALSE);
      else
        aopPut (result, "a", offl + offr);
    }

  for (offl = LSB + 1; offl < LSB + 8; offl++)
    {
      if (size > offl + offr)
        {
          if (!useXch)
            MOVA (aopGet (left, offl, FALSE, FALSE));
          emitcode ("rlc", "a");
          if (useXch)
            xch_a_aopGet (left, offl + offr, FALSE, FALSE);
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
/* genlshAny - shift any number of bytes by a known amount != 0    */
/*-----------------------------------------------------------------*/
static void
genlshAny (operand *result, operand *left, int shCount)
{
  int size, size2, offset;

  D (emitcode (";", "genlshAny"));

  size = AOP_SIZE (result);

  if (!operandsEqu (result, left))
    for (size2 = size, offset = 0; size2 > 0; size2--, offset++)
      aopPut (result, aopGet (left, offset, FALSE, FALSE), offset);

  while (shCount--)
    {
      MOVA (aopGet (result, LSB, FALSE, FALSE));
      emitcode ("add", "a,acc");
      aopPut (result, "a", 0);

      for(size2 = size - 1, offset = 1; size2 > 0; size2--, offset++)
        {

          MOVA (aopGet (result, offset, FALSE, FALSE));
          emitcode ("rlc", "a");
          aopPut (result, "a", offset);
        }
    }
}

/*-----------------------------------------------------------------*/
/* genLeftShiftLiteral - left shifting by known count              */
/*-----------------------------------------------------------------*/
static void
genLeftShiftLiteral (operand * left, operand * right, operand * result, iCode * ic)
{
  unsigned int shCount = (unsigned int) ulFromVal (AOP (right)->aopu.aop_lit);
  unsigned int size;

  size = getSize (operandType (result));

  D (emitcode (";", "genLeftShiftLiteral (%d), size %d", shCount, size));

  freeAsmop (right, NULL, ic, TRUE);

  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

#if VIEW_SIZE
  emitcode ("; shift left ", "result %d, left %d", size, AOP_SIZE (left));
#endif

  switch (size)
    {
    case 1:
    case 2:
    case 4:
    case 8:
      genlshFixed (result, left, shCount);
      break;

    default:
      genlshAny (result, left, shCount);
      break;
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

  aopOp (right, ic, FALSE);

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
      MOVB (aopGet (right, 0, FALSE, FALSE));
      emitcode ("inc", "b");
    }
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  /* now move the left to the result if they are not the same */
  if (!sameRegs (AOP (left), AOP (result)) && AOP_SIZE (result) > 1)
    {
      size = AOP_SIZE (result);
      offset = 0;
      while (size--)
        {
          const char *l = aopGet (left, offset, FALSE, TRUE);
          if (*l == '@' && (IS_AOP_PREG (result)))
            {

              emitcode ("mov", "a,%s", l);
              aopPut (result, "a", offset);
            }
          else
            aopPut (result, l, offset);
          offset++;
        }
    }

  tlbl = newiTempLabel (NULL);
  size = AOP_SIZE (result);
  offset = 0;
  tlbl1 = newiTempLabel (NULL);

  /* if it is only one byte then */
  if (size == 1)
    {
      symbol *tlbl1 = newiTempLabel (NULL);

      MOVA (aopGet (left, 0, FALSE, FALSE));
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
  MOVA (aopGet (result, offset, FALSE, FALSE));
  emitcode ("add", "a,acc");
  aopPut (result, "a", offset++);
  while (--size)
    {
      MOVA (aopGet (result, offset, FALSE, FALSE));
      emitcode ("rlc", "a");
      aopPut (result, "a", offset++);
    }
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
      if (shCount)
        shiftR1Left2Result (left, MSB16, result, LSB, shCount, sign);
      else
        movLeft2Result (left, MSB16, result, LSB, sign);
      addSign (result, MSB16, sign);
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

  MOVA (aopGet (left, MSB32, FALSE, FALSE));

  if (offl == MSB16)
    {
      // shift is > 8
      if (sign)
        {
          emitcode ("rlc", "a");
          emitcode ("subb", "a,acc");
          if (overlapping && sameByte (AOP (left), MSB32, AOP (result), MSB32))
            {
              xch_a_aopGet (left, MSB32, FALSE, FALSE);
            }
          else
            {
              aopPut (result, "a", MSB32);
              MOVA (aopGet (left, MSB32, FALSE, FALSE));
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
      emitcode ("mov", "c,acc.7");
    }

  emitcode ("rrc", "a");

  if (overlapping && offl == MSB16 && sameByte (AOP (left), MSB24, AOP (result), MSB32 - offl))
    {
      xch_a_aopGet (left, MSB24, FALSE, FALSE);
    }
  else
    {
      aopPut (result, "a", MSB32 - offl);
      MOVA (aopGet (left, MSB24, FALSE, FALSE));
    }

  emitcode ("rrc", "a");
  if (overlapping && offl == MSB16 && sameByte (AOP (left), MSB16, AOP (result), MSB24 - offl))
    {
      xch_a_aopGet (left, MSB16, FALSE, FALSE);
    }
  else
    {
      aopPut (result, "a", MSB24 - offl);
      MOVA (aopGet (left, MSB16, FALSE, FALSE));
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
          xch_a_aopGet (left, LSB, FALSE, FALSE);
        }
      else
        {
          aopPut (result, "a", MSB16 - offl);
          MOVA (aopGet (left, LSB, FALSE, FALSE));
        }
      emitcode ("rrc", "a");
      aopPut (result, "a", LSB);
    }
}

/*-----------------------------------------------------------------*/
/* genrshFour - shift four byte by a known amount != 0             */
/*-----------------------------------------------------------------*/
static void
genrshFour (operand *result, operand *left, int shCount, int sign)
{
  D (emitcode (";", "genrshFour"));

  /* if shifting more that 3 bytes */
  if (shCount >= 24)
    {
      shCount -= 24;
      if (shCount)
        shiftR1Left2Result (left, MSB32, result, LSB, shCount, sign);
      else
        movLeft2Result (left, MSB32, result, LSB, sign);
      addSign (result, MSB16, sign);
    }
  else if (shCount >= 16)
    {
      shCount -= 16;
      if (shCount)
        shiftR2Left2Result (left, MSB24, result, LSB, shCount, sign);
      else
        {
          movLeft2Result (left, MSB24, result, LSB, 0);
          movLeft2Result (left, MSB32, result, MSB16, sign);
        }
      addSign (result, MSB24, sign);
    }
  else if (shCount >= 8)
    {
      shCount -= 8;
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
      aopPut (result, aopGet (left, offset, FALSE, FALSE), offset);

  while (shCount--)
    {
      MOVA (aopGet (result, size - 1, FALSE, FALSE));
      if (!sign)
        emitcode ("clr", "c");
      else
        emitcode ("mov", "c,acc.7");
      emitcode ("rrc", "a");
      aopPut (result, "a", size - 1);

      for(size2 = size - 1, offset = size - 2; size2 > 0; size2--, offset--)
        {

          MOVA (aopGet (result, offset, FALSE, FALSE));
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

  size = getSize (operandType (result));        //getDataSize (left);

  D (emitcode (";", "genRightShiftLiteral (%d), size %d", shCount, size));

  freeAsmop (right, NULL, ic, TRUE);

  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

#if VIEW_SIZE
  emitcode ("; shift right ", "result %d, left %d", AOP_SIZE (result), AOP_SIZE (left));
#endif

  /* test the LEFT size !!! */

  /* I suppose that the left size >= result size */
  wassert ((int)getSize (operandType (left)) >= size);

  if (shCount == 0)
    {
      size = getDataSize (result);
      while (size--)
        movLeft2Result (left, size, result, size, 0);
    }
  else if (shCount >= (size * 8))
    {
      if (sign)
        {
          /* get sign in acc.7 */
          MOVA (aopGet (left, size - 1, FALSE, FALSE));
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

  aopOp (right, ic, FALSE);

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
      MOVB (aopGet (right, 0, FALSE, FALSE));
      emitcode ("inc", "b");
    }
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  /* now move the left to the result if they are not the
     same */
  if (!sameRegs (AOP (left), AOP (result)) && AOP_SIZE (result) > 1)
    {

      size = AOP_SIZE (result);
      offset = 0;
      while (size--)
        {
          const char *l = aopGet (left, offset, FALSE, TRUE);
          if (*l == '@' && IS_AOP_PREG (result))
            {

              emitcode ("mov", "a,%s", l);
              aopPut (result, "a", offset);
            }
          else
            aopPut (result, l, offset);
          offset++;
        }
    }

  /* mov the highest order bit to OVR */
  tlbl = newiTempLabel (NULL);
  tlbl1 = newiTempLabel (NULL);

  size = AOP_SIZE (result);
  offset = size - 1;
  MOVA (aopGet (left, offset, FALSE, FALSE));
  emitcode ("rlc", "a");
  emitcode ("mov", "ov,c");
  /* if it is only one byte then */
  if (size == 1)
    {
      MOVA (aopGet (left, 0, FALSE, FALSE));
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
  while (size--)
    {
      MOVA (aopGet (result, offset, FALSE, FALSE));
      emitcode ("rrc", "a");
      aopPut (result, "a", offset--);
    }
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

  aopOp (right, ic, FALSE);

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
      MOVB (aopGet (right, 0, FALSE, FALSE));
      emitcode ("inc", "b");
    }
  freeAsmop (right, NULL, ic, TRUE);
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  /* now move the left to the result if they are not the
     same */
  if (!sameRegs (AOP (left), AOP (result)) && AOP_SIZE (result) > 1)
    {
      size = AOP_SIZE (result);
      offset = 0;
      while (size--)
        {
          const char *l = aopGet (left, offset, FALSE, TRUE);
          if (*l == '@' && IS_AOP_PREG (result))
            {

              emitcode ("mov", "a,%s", l);
              aopPut (result, "a", offset);
            }
          else
            aopPut (result, l, offset);
          offset++;
        }
    }

  tlbl = newiTempLabel (NULL);
  tlbl1 = newiTempLabel (NULL);
  size = AOP_SIZE (result);
  offset = size - 1;

  /* if it is only one byte then */
  if (size == 1)
    {
      MOVA (aopGet (left, 0, FALSE, FALSE));
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
  while (size--)
    {
      MOVA (aopGet (result, offset, FALSE, FALSE));
      emitcode ("rrc", "a");
      aopPut (result, "a", offset--);
    }
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
static char *
genUnpackBits (operand * result, const char *rname, int ptype, iCode * ifx)
{
  int offset = 0;               /* result byte offset */
  int rsize;                    /* result size */
  int rlen = 0;                 /* remaining bitfield length */
  sym_link *etype;              /* bitfield type information */
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */
  static char *const accBits[] = { "acc.0", "acc.1", "acc.2", "acc.3",
                                   "acc.4", "acc.5", "acc.6", "acc.7"
                                 };

  D (emitcode (";", "genUnpackBits"));

  etype = getSpec (operandType (result));
  rsize = getSize (operandType (result));
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  if (ifx && blen <= 8)
    {
      emitPtrByteGet (rname, ptype, FALSE);
      if (blen == 1)
        {
          return accBits[bstr];;
        }
      else
        {
          if (blen < 8)
            emitcode ("anl", "a,#!constbyte", (((unsigned char) - 1) >> (8 - blen)) << bstr);
          return "a";
        }
    }
  wassert (!ifx);

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

          emitcode ("jnb", "acc.%d,!tlabel", blen - 1, labelKey2num (tlbl->key));
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

          emitcode ("jnb", "acc.%d,!tlabel", rlen - 1, labelKey2num (tlbl->key));
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
  return NULL;
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

  aopOp (result, ic, TRUE);

  /* get the string representation of the name */
  l = aopGet (left, 0, FALSE, TRUE) + 1;        // remove #
  size = AOP_SIZE (result);
  while (size--)
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      if (AOP_SIZE (result) > 1)
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

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genNearPointerGet - emitcode for near pointer fetch             */
/*-----------------------------------------------------------------*/
static void
genNearPointerGet (operand * left, operand * result, iCode * ic, iCode * pi, iCode * ifx)
{
  asmop *aop = NULL;
  reg_info *preg = NULL;
  const char *rname;
  char *ifxCond = "a";
  sym_link *rtype, *retype;
  sym_link *ltype = operandType (left);

  D (emitcode (";", "genNearPointerGet"));

  rtype = operandType (result);
  retype = getSpec (rtype);

  aopOp (left, ic, FALSE);

  /* if left is rematerialisable and
     result is not bitfield variable type and
     the left is pointer to data space i.e
     lower 128 bytes of space */
  if (AOP_TYPE (left) == AOP_IMMD && !IS_BITFIELD (retype) && DCL_TYPE (ltype) == POINTER)
    {
      genDataPointerGet (left, result, ic);
      return;
    }

  //aopOp (result, ic, FALSE);
  aopOp (result, ic, result ? TRUE : FALSE);

  /* if the value is already in a pointer register
     then don't need anything more */
  if (!AOP_INPREG (AOP (left)))
    {
      if (IS_AOP_PREG (left))
        {
          // Aha, it is a pointer, just in disguise.
          rname = aopGet (left, 0, FALSE, FALSE);
          if (EQ (rname, "a"))
            {
              // It's in pdata or on xstack
              rname = AOP (left)->aopu.aop_ptr->name;
              emitcode ("mov", "%s,a", rname);
            }
          else if (*rname != '@')
            {
              fprintf (stderr, "probable internal error: unexpected rname '%s' @ %s:%d\n", rname, __FILE__, __LINE__);
            }
          else
            {
              // Expected case.
              emitcode ("mov", "a%s,%s", rname + 1, rname);
              rname++;          // skip the '@'.
            }
        }
      else
        {
          /* otherwise get a free pointer register */
          aop = newAsmop (0);
          preg = getFreePtr (ic, aop, FALSE);
          emitcode ("mov", "%s,%s", preg->name, aopGet (left, 0, FALSE, TRUE));
          rname = preg->name;
        }
    }
  else
    rname = aopGet (left, 0, FALSE, FALSE);

  /* if bitfield then unpack the bits */
  if (IS_BITFIELD (retype))
    ifxCond = genUnpackBits (result, rname, POINTER, ifx);
  else
    {
      /* we can just get the values */
      int size = AOP_SIZE (result);
      int offset = 0;

      while (size--)
        {
          if (ifx || IS_AOP_PREG (result) || AOP_TYPE (result) == AOP_STK)
            {
              emitcode ("mov", "a,@%s", rname);
              if (!ifx)
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
      freeAsmop (NULL, aop, ic, RESULTONSTACK (ic) ? FALSE : TRUE);
    }
  else
    {
      /* we did not allocate which means left
         already in a pointer register, then
         if size > 0 && this could be used again
         we have to point it back to where it
         belongs */
      if ((AOP_SIZE (result) > 1 && !OP_SYMBOL (left)->remat && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth)) && !pi)
        {
          int size = AOP_SIZE (result) - 1;
          while (size--)
            emitcode ("dec", "%s", rname);
        }
    }

  if (ifx && !ifx->generated)
    {
      genIfxJump (ifx, ifxCond, left, NULL, result, ic->next);
    }

  /* done */
  freeAsmop (result, NULL, ic, RESULTONSTACK (ic) ? FALSE : TRUE);
  freeAsmop (left, NULL, ic, TRUE);
  if (pi)
    pi->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genPagedPointerGet - emitcode for paged pointer fetch           */
/*-----------------------------------------------------------------*/
static void
genPagedPointerGet (operand * left, operand * result, iCode * ic, iCode * pi, iCode * ifx)
{
  asmop *aop = NULL;
  reg_info *preg = NULL;
  const char *rname;
  char *ifxCond = "a";
  sym_link *rtype, *retype;

  D (emitcode (";", "genPagedPointerGet"));

  rtype = operandType (result);
  retype = getSpec (rtype);

  aopOp (left, ic, FALSE);

  /* if the value is already in a pointer register
     then don't need anything more */
  if (!AOP_INPREG (AOP (left)))
    {
      const char *l;
      /* otherwise get a free pointer register */
      aop = newAsmop (0);
      preg = getFreePtr (ic, aop, FALSE);
      l = aopGet (left, 0, FALSE, TRUE);
      if (*l == '@')
        emitcode ("mov", "a%s,%s", preg->name, l);
      else
        emitcode ("mov", "%s,%s", preg->name, l);
      rname = preg->name;
    }
  else
    {
      rname = aopGet (left, 0, FALSE, FALSE);
    }

  aopOp (result, ic, TRUE);

  /* if bitfield then unpack the bits */
  if (IS_BITFIELD (retype))
    {
      ifxCond = genUnpackBits (result, rname, PPOINTER, ifx);
    }
  else
    {
      /* we can just get the values */
      int size = AOP_SIZE (result);
      int offset = 0;

      while (size--)
        {
          emitcode ("movx", "a,@%s", rname);
          if (!ifx)
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
    }
  else
    {
      /* we did not allocate which means left
         already in a pointer register, then
         if size > 1 && this could be used again
         we have to point it back to where it
         belongs */
      if ((AOP_SIZE (result) > 1 && !OP_SYMBOL (left)->remat && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth)) && !pi)
        {
          int size = AOP_SIZE (result) - 1;
          while (size--)
            emitcode ("dec", "%s", rname);
        }
    }

  /* done */
  freeAsmop (result, NULL, ic, TRUE);
  if (aop)
    {
      freeAsmop (NULL, aop, ic, TRUE);
    }
  freeAsmop (left, NULL, ic, TRUE);

  if (ifx && !ifx->generated)
    {
      genIfxJump (ifx, ifxCond, NULL, NULL, NULL, ic->next);
    }

  if (pi)
    pi->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genFarPointerGet - get value from far space                     */
/*-----------------------------------------------------------------*/
static void
genFarPointerGet (operand * left, operand * result, iCode * ic, iCode * pi, iCode * ifx)
{
  int size, offset;
  char *ifxCond = "a";
  sym_link *retype = getSpec (operandType (result));

  D (emitcode (";", "genFarPointerGet"));

  aopOp (left, ic, FALSE);
  loadDptrFromOperand (left, FALSE);

  /* so dptr now contains the address */
  aopOp (result, ic, FALSE);

  /* if bit then unpack */
  if (IS_BITFIELD (retype))
    ifxCond = genUnpackBits (result, "dptr", FPOINTER, ifx);
  else
    {
      size = AOP_SIZE (result);
      offset = 0;

      while (size--)
        {
          emitcode ("movx", "a,@dptr");
          if (!ifx)
            aopPut (result, "a", offset++);
          if (size || pi)
            emitcode ("inc", "dptr");
        }
    }

  if (pi && AOP_TYPE (left) != AOP_IMMD && AOP_TYPE (left) != AOP_STR)
    {
      aopPut (left, "dpl", 0);
      aopPut (left, "dph", 1);
      pi->generated = 1;
    }

  if (ifx && !ifx->generated)
    {
      genIfxJump (ifx, ifxCond, left, NULL, result, ic->next);
    }

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genCodePointerGet - get value from code space                   */
/*-----------------------------------------------------------------*/
static void
genCodePointerGet (operand * left, operand * result, iCode * ic, iCode * pi, iCode * ifx)
{
  int size, offset;
  char *ifxCond = "a";
  sym_link *retype = getSpec (operandType (result));

  D (emitcode (";", "genCodePointerGet"));

  aopOp (left, ic, FALSE);
  loadDptrFromOperand (left, FALSE);

  /* so dptr now contains the address */
  aopOp (result, ic, FALSE);

  /* if bit then unpack */
  if (IS_BITFIELD (retype))
    ifxCond = genUnpackBits (result, "dptr", CPOINTER, ifx);
  else
    {
      size = AOP_SIZE (result);
      offset = 0;

      while (size--)
        {
          emitcode ("clr", "a");
          emitcode ("movc", "a,@a+dptr");
          if (!ifx)
            aopPut (result, "a", offset++);
          if (size || pi)
            emitcode ("inc", "dptr");
        }
    }

  if (pi && AOP_TYPE (left) != AOP_IMMD && AOP_TYPE (left) != AOP_STR)
    {
      aopPut (left, "dpl", 0);
      aopPut (left, "dph", 1);
      pi->generated = 1;
    }

  if (ifx && !ifx->generated)
    {
      genIfxJump (ifx, ifxCond, left, NULL, result, ic->next);
    }

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGenPointerGet - get value from generic pointer space         */
/*-----------------------------------------------------------------*/
static void
genGenPointerGet (operand * left, operand * result, iCode * ic, iCode * pi, iCode * ifx)
{
  int size, offset;
  char *ifxCond = "a";
  sym_link *retype = getSpec (operandType (result));

  D (emitcode (";", "genGenPointerGet"));

  aopOp (left, ic, FALSE);
  loadDptrFromOperand (left, TRUE);

  /* so dptr-b now contains the address */
  aopOp (result, ic, FALSE);

  /* if bit then unpack */
  if (IS_BITFIELD (retype))
    {
      ifxCond = genUnpackBits (result, "dptr", GPOINTER, ifx);
    }
  else
    {
      size = AOP_SIZE (result);
      offset = 0;

      while (size--)
        {
          emitcode ("lcall", "__gptrget");
          if (!ifx)
            aopPut (result, "a", offset++);
          if (size || pi)
            emitcode ("inc", "dptr");
        }
    }

  if (pi && AOP_TYPE (left) != AOP_IMMD && AOP_TYPE (left) != AOP_STR)
    {
      aopPut (left, "dpl", 0);
      aopPut (left, "dph", 1);
      pi->generated = 1;
    }

  if (ifx && !ifx->generated)
    {
      genIfxJump (ifx, ifxCond, left, NULL, result, ic->next);
    }

  freeAsmop (result, NULL, ic, TRUE);
  freeAsmop (left, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void
genPointerGet (iCode * ic, iCode * pi, iCode * ifx)
{
  operand *left, *result;
  sym_link *type, *etype;
  int p_type;

  D (emitcode (";", "genPointerGet"));

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  if (getSize (operandType (result)) > 1)
    ifx = NULL;

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
  while (IS_SYMOP (left) && OP_SYMBOL (left)->remat && IS_CAST_ICODE (OP_SYMBOL (left)->rematiCode))
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
      genNearPointerGet (left, result, ic, pi, ifx);
      break;

    case PPOINTER:
      genPagedPointerGet (left, result, ic, pi, ifx);
      break;

    case FPOINTER:
      genFarPointerGet (left, result, ic, pi, ifx);
      break;

    case CPOINTER:
      genCodePointerGet (left, result, ic, pi, ifx);
      break;

    case GPOINTER:
      genGenPointerGet (left, result, ic, pi, ifx);
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
                  MOVA (aopGet (right, 0, FALSE, FALSE));
                  emitcode ("rrc", "a");
                }
              emitPtrByteGet (rname, p_type, FALSE);
              emitcode ("mov", "acc.%d,c", bstr);
            }
          else
            {
              bool pushedB;
              /* Case with a bitfield length < 8 and arbitrary source
               */
              MOVA (aopGet (right, 0, FALSE, FALSE));
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
      emitPtrByteSet (rname, p_type, aopGet (right, offset++, FALSE, TRUE));
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
          MOVA (aopGet (right, offset++, FALSE, FALSE));
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
/* genLiteralAssign - assignment of literal                        */
/*-----------------------------------------------------------------*/
static void
genLiteralAssign (operand * result, operand * right, int size, bool (*output_fn) (operand * result, const char *s, int offset))
{
  unsigned long long lit = 0LL;
  int offset;
  int accumulator_value = -1;   /* -1 denotes: not yet set */

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

  offset = 0;
  while (size)
    {
      /* check whether preloading the accumulator pays off:

         mov     direct,#something       3 byte, 2 cycle
         mov     direct,a                2 byte, 1 cycle

         mov     @r0,#something          2 byte, 1 cycle
         mov     @r0,a                   1 byte, 1 cycle

         mov     rx,#something           2 byte, 1 cycle
         mov     rx,a                    1 byte, 1 cycle

         clr     a                       1 byte, 1 cycle
         mov     a,#something            2 byte, 1 cycle

         (setting bytes in pdata and xdata need the accumulator anyway)
       */

      /* clr a needs an extra byte. If two bytes are zero it starts to pay off
         to preload the accumulator */
      int clr_num_bytes_saved = -1 +    /* size of clr a */
                                (int) ((((lit >> 0) & 0xff) == 0) && (size >= 1)) +
                                (int) ((((lit >> 8) & 0xff) == 0) && (size >= 2)) +
                                (int) ((((lit >> 16) & 0xff) == 0) && (size >= 3)) + (int) ((((lit >> 24) & 0xff) == 0) && (size >= 4));

      /* mov a,#something needs two extra bytes. If three bytes are identical it starts to pay off */
      int mov_num_bytes_saved = -2 +    /* size of mov a,#something */
                                (int) ((lit & 0xff) == ((lit >> 0) & 0xff) && (size >= 1)) +    /* true */
                                (int) ((lit & 0xff) == ((lit >> 8) & 0xff) && (size >= 2)) +
                                (int) ((lit & 0xff) == ((lit >> 16) & 0xff) && (size >= 3)) +
                                (int) ((lit & 0xff) == ((lit >> 24) & 0xff) && (size >= 4));

      int num_bytes_to_save_before_using_acc_takes_effect = 1;

      if (optimize.codeSpeed && (AOP_TYPE (result) != AOP_DIR))
        {
          /* require an extra byte being saved */
          num_bytes_to_save_before_using_acc_takes_effect++;
        }

      /* eventually preload accumulator */
      if ((clr_num_bytes_saved >= num_bytes_to_save_before_using_acc_takes_effect) &&
          (clr_num_bytes_saved >= mov_num_bytes_saved) && (lit & 0xff) == 0)
        {
          if (0 != accumulator_value)
            {
              accumulator_value = 0;
//              emitcode ("clr", "a");
              MOVA ("#0x00");
            }
        }
      else if ((mov_num_bytes_saved >= num_bytes_to_save_before_using_acc_takes_effect) && (mov_num_bytes_saved > clr_num_bytes_saved)) /* preferrably have 0 in acc */
        {
          if ((lit & 0xff) != accumulator_value)
            {
              accumulator_value = lit & 0xff;
//              emitcode ("mov", "a,%s", aopGet (right, offset, FALSE, FALSE));
              MOVA (aopGet (right, offset, FALSE, FALSE));
            }
        }

      /* write byte */
      if ((lit & 0xff) == accumulator_value)
        {
          /* value in accumulator can be used */
          output_fn (result, "a", offset);
        }
      else
        {
          /* otherwise use the normal path that should always work */
          char *r = Safe_strdup (aopGet (right, offset, FALSE, FALSE));
          output_fn (result, r, offset);
          Safe_free (r);
        }

      /* advance */
      lit >>= 8;
      offset++;
      size--;
    }
}

/*-----------------------------------------------------------------*/
/* dataPut - puts a string for a aop and indicates if acc is in use */
/*-----------------------------------------------------------------*/
static bool
litPut (operand * result, const char *s, int offset)
{
  emitcode ("mov", "(%s + %d),%s", aopGet (result, 0, FALSE, TRUE) + 1, offset, s);
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* genDataPointerSet - remat pointer to data space                 */
/*-----------------------------------------------------------------*/
static void
genDataPointerSet (operand * right, operand * result, iCode * ic)
{
  int size, offset;

  D (emitcode (";", "genDataPointerSet"));

  aopOp (right, ic, FALSE);

  size = max (AOP_SIZE (right), AOP_SIZE (result));
  if ((size > 1) && IS_OP_LITERAL (right))
    {
      genLiteralAssign (result, right, size, litPut);
    }
  else
    {
      //remove #
      char *l = Safe_strdup (aopGet (result, 0, FALSE, TRUE) + 1);      //remove #
      for (offset = 0; offset < size; offset++)
        {
          struct dbuf_s dbuf;

          dbuf_init (&dbuf, 128);
          if (size > 1)
            dbuf_printf (&dbuf, "(%s + %d)", l, offset);
          else
            dbuf_append_str (&dbuf, l);
          emitcode ("mov", "%s,%s", dbuf_c_str (&dbuf), aopGet (right, offset, FALSE, FALSE));
          dbuf_destroy (&dbuf);
        }
      Safe_free (l);
    }

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

  aopOp (result, ic, FALSE);

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
      if (IS_AOP_PREG (result))
        {
          // Aha, it is a pointer, just in disguise.
          rname = aopGet (result, 0, FALSE, FALSE);
          if (EQ (rname, "a"))
            {
              // It's in pdata or on xstack
              rname = AOP (result)->aopu.aop_ptr->name;
              emitcode ("mov", "%s,a", rname);
            }
          else if (*rname != '@')
            {
              fprintf (stderr, "probable internal error: unexpected rname @ %s:%d\n", __FILE__, __LINE__);
            }
          else
            {
              // Expected case.
              emitcode ("mov", "a%s,%s", rname + 1, rname);
              rname++;          // skip the '@'.
            }
        }
      else
        {
          /* otherwise get a free pointer register */
          aop = newAsmop (0);
          preg = getFreePtr (ic, aop, FALSE);
          emitcode ("mov", "%s,%s", preg->name, aopGet (result, 0, FALSE, TRUE));
          rname = preg->name;
        }
    }
  else
    {
      rname = aopGet (result, 0, FALSE, FALSE);
    }

  aopOp (right, ic, FALSE);

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
          const char *l = aopGet (right, offset, FALSE, TRUE);
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

  aopOp (result, ic, FALSE);

  /* if the value is already in a pointer register
     then don't need anything more */
  if (!AOP_INPREG (AOP (result)))
    {
      if (IS_AOP_PREG (result))
        {
          // Aha, it is a pointer, just in disguise.
          rname = aopGet (result, 0, FALSE, FALSE);
          if (*rname != '@')
            {
              fprintf (stderr, "probable internal error: unexpected rname @ %s:%d\n", __FILE__, __LINE__);
            }
          else
            {
              // Expected case.
              emitcode ("mov", "a%s,%s", rname + 1, rname);
              rname++;          // skip the '@'.
            }
        }
      else
        {
          /* otherwise get a free pointer register */
          aop = newAsmop (0);
          preg = getFreePtr (ic, aop, FALSE);
          emitcode ("mov", "%s,%s", preg->name, aopGet (result, 0, FALSE, TRUE));
          rname = preg->name;
        }
    }
  else
    {
      rname = aopGet (result, 0, FALSE, FALSE);
    }

  aopOp (right, ic, FALSE);

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
          MOVA (aopGet (right, offset, FALSE, TRUE));
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
         if size > 1 && this could be used again
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
/* genFarPointerSet - set value in far space                       */
/*-----------------------------------------------------------------*/
static void
genFarPointerSet (operand * right, operand * result, iCode * ic, iCode * pi)
{
  int size, offset;
  sym_link *retype = getSpec (operandType (right));
  sym_link *letype = getSpec (operandType (result));

  D (emitcode (";", "genFarPointerSet"));

  aopOp (result, ic, FALSE);
  loadDptrFromOperand (result, FALSE);

  /* so dptr now contains the address */
  aopOp (right, ic, FALSE);

  /* if bit then unpack */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    genPackBits ((IS_BITFIELD (retype) ? retype : letype), right, "dptr", FPOINTER);
  else
    {
      size = AOP_SIZE (right);
      offset = 0;

      while (size--)
        {
          MOVA (aopGet (right, offset, FALSE, FALSE));
          if (offset++ > 0)
            emitcode ("inc", "dptr");
          emitcode ("movx", "@dptr,a");
        }
      if (pi)
        emitcode ("inc", "dptr");
    }
  if (pi && AOP_TYPE (result) != AOP_STR && AOP_TYPE (result) != AOP_IMMD)
    {
      aopPut (result, "dpl", 0);
      aopPut (result, "dph", 1);
      pi->generated = 1;
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
  int size, offset;
  sym_link *retype = getSpec (operandType (right));
  sym_link *letype = getSpec (operandType (result));

  D (emitcode (";", "genGenPointerSet"));

  aopOp (result, ic, FALSE);
  loadDptrFromOperand (result, TRUE);

  /* so dptr-b now contains the address */
  aopOp (right, ic, FALSE);

  /* if bit then unpack */
  if (IS_BITFIELD (retype) || IS_BITFIELD (letype))
    {
      genPackBits ((IS_BITFIELD (retype) ? retype : letype), right, "dptr", GPOINTER);
    }
  else
    {
      size = AOP_SIZE (right);
      offset = 0;

      while (size--)
        {
          MOVA (aopGet (right, offset++, FALSE, FALSE));
          emitcode ("lcall", "__gptrput");
          if (size || pi)
            emitcode ("inc", "dptr");
        }
    }

  if (pi && AOP_TYPE (result) != AOP_STR && AOP_TYPE (result) != AOP_IMMD)
    {
      aopPut (result, "dpl", 0);
      aopPut (result, "dph", 1);
      pi->generated = 1;
    }
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
  while (IS_SYMOP (result) && OP_SYMBOL (result)->remat &&
         IS_CAST_ICODE (OP_SYMBOL (result)->rematiCode) &&
         !IS_BITFIELD (getSpec (operandType (result))))
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

  aopOp (cond, ic, FALSE);

  /* get the value into acc */
  if (AOP_TYPE (cond) != AOP_CRY)
    {
      toBoolean (cond);
    }
  else
    {
      isbit = 1;
      if (AOP (cond)->aopu.aop_dir)
        dup = Safe_strdup (AOP (cond)->aopu.aop_dir);
    }

  /* the result is now in the accumulator or a directly addressable bit */
  freeAsmop (cond, NULL, ic, TRUE);

  /* if the condition is a bit variable */
  if (isbit && dup)
    genIfxJump (ic, dup, NULL, NULL, NULL, popIc);
  else if (isbit && IS_OP_ACCUSE (cond))
    genIfxJump (ic, "c", NULL, NULL, NULL, popIc);
  else if (isbit && IS_ITEMP (cond) && SPIL_LOC (cond))
    genIfxJump (ic, SPIL_LOC (cond)->rname, NULL, NULL, NULL, popIc);
  else if (isbit && !IS_ITEMP (cond))
    genIfxJump (ic, OP_SYMBOL (cond)->rname, NULL, NULL, NULL, popIc);
  else
    genIfxJump (ic, "a", NULL, NULL, NULL, popIc);

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

  D (emitcode (";", "genAddrOf"));

  aopOp (IC_RESULT (ic), ic, FALSE);

  /* if the operand is on the stack then we
     need to get the stack offset of this
     variable */
  if (sym->onStack)
    {
      int stack_offset = stackoffset (sym);

      /* if it has an offset then we need to compute it */
      if (stack_offset)
        {
          if ((abs (stack_offset) == 1) && !AOP_NEEDSACC (IC_RESULT (ic)) && !isOperandVolatile (IC_RESULT (ic), FALSE))
            {
              aopPut (IC_RESULT (ic), SYM_BP (sym), 0);
              if (stack_offset > 0)
                emitcode ("inc", "%s", aopGet (IC_RESULT (ic), LSB, FALSE, FALSE));
              else
                emitcode ("dec", "%s", aopGet (IC_RESULT (ic), LSB, FALSE, FALSE));
            }
          else
            {
              emitcode ("mov", "a,%s", SYM_BP (sym));
              emitcode ("add", "a,#!constbyte", stack_offset & 0xff);
              aopPut (IC_RESULT (ic), "a", 0);
            }
        }
      else
        {
          /* we can just move _bp */
          aopPut (IC_RESULT (ic), SYM_BP (sym), 0);
        }
      /* fill the result with zero */
      size = AOP_SIZE (IC_RESULT (ic)) - 1;

      offset = 1;
      while (size--)
        {
          aopPut (IC_RESULT (ic), zero, offset++);
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
          dbuf_printf (&dbuf, "#(%s >> %d)", sym->rname, offset * 8);
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

  D (emitcode (";", "genFarFarAssign"));

  /* first push the right side on to the stack */
  while (size--)
    {
      MOVA (aopGet (right, offset++, FALSE, FALSE));
      emitpush ("acc");
    }

  freeAsmop (right, NULL, ic, FALSE);
  /* now assign DPTR to result */
  aopOp (result, ic, FALSE);
  size = AOP_SIZE (result);
  while (size--)
    {
      emitpop ("acc");
      aopPut (result, "a", --offset);
    }
  freeAsmop (result, NULL, ic, FALSE);
}

/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void
genAssign (iCode * ic)
{
  operand *result, *right;
  int size, offset;

  D (emitcode (";", "genAssign"));

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);

  /* if they are the same */
  if (operandsEqu (result, right) && !isOperandVolatile (result, FALSE) && !isOperandVolatile (right, FALSE))
    return;

  aopOp (right, ic, FALSE);

  /* special case both in far space */
  if (AOP_TYPE (right) == AOP_DPTR && IS_TRUE_SYMOP (result) && isOperandInFarSpace (result))
    {
      genFarFarAssign (result, right, ic);
      return;
    }

  aopOp (result, ic, TRUE);

  /* if they are the same registers */
  if (sameRegs (AOP (right), AOP (result)) && !isOperandVolatile (result, FALSE) && !isOperandVolatile (right, FALSE))
    goto release;

  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      assignBit (result, right);
      goto release;
    }

  /* bit variables done */
  /* general case */

  size = getDataSize (result);

  if ((size > 1) && (AOP_TYPE (result) != AOP_REG) &&   /* for registers too? (regression test passes) */
      (AOP_TYPE (right) == AOP_LIT) && !aopPutUsesAcc (result, aopGet (right, 0, FALSE, FALSE), 0))
    {
      genLiteralAssign (result, right, size, aopPut);
    }
  else
    {
      offset = 0;
      while (size--)
        {
          aopPut (result, aopGet (right, offset, FALSE, FALSE), offset);
          offset++;
        }
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
  operand *cond = IC_JTCOND (ic);
  symbol *jtab, *jtablo, *jtabhi;
  unsigned int count;
  const char *l;
  bool useB = FALSE;

  D (emitcode (";", "genJumpTab"));

  count = elementsInSet (IC_JTLABELS (ic));

  if ((count <= 7) ||
      (count <= 16 && optimize.codeSpeed) ||
      options.acall_ajmp)
    {
      /* This algorithm needs 9 cycles and 7 + 3*n bytes
         if the switch argument is in a register.
         (6 + 2*n bytes when options.acall_ajmp is set) */
      /* Peephole may not convert ljmp to sjmp or ret
         labelIsReturnOnly & labelInRange must check
         currPl->ic->op != JUMPTABLE */
      aopOp (cond, ic, FALSE);
      /* get the condition into accumulator */
      l = aopGet (cond, 0, FALSE, FALSE);
      MOVA (l);
      /* multiply by three */
      if ((AOP_TYPE (cond) == AOP_REG) || (IS_AOP_PREG (cond) && !AOP (cond)->paged && !IS_VOLATILE (operandType (cond))))
        {
          emitcode ("add", "a,%s", l);
          if (options.acall_ajmp == 0)
            emitcode ("add", "a,%s", l);
        }
      else
        {
          if (options.acall_ajmp == 0)
            {
              MOVB ("#0x03");
              emitcode ("mul", "ab");
            }
          else
            {
              emitcode ("add", "a,acc");
            }
        }
      freeAsmop (cond, NULL, ic, TRUE);

      jtab = newiTempLabel (NULL);
      emitcode ("mov", "dptr,#!tlabel", labelKey2num (jtab->key));
      emitcode ("jmp", "@a+dptr");
      emitLabel (jtab);
      /* now generate the jump labels */
      for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
        if (options.acall_ajmp)
          emitcode ("ajmp", "!tlabel", labelKey2num (jtab->key));
        else 
          emitcode ("ljmp", "!tlabel", labelKey2num (jtab->key));
    }
  else
    {
      /* this algorithm needs 14 cycles and 14 + 2*n bytes
         if the switch argument is in a register.
         For n>7 this algorithm may be more compact */
      jtablo = newiTempLabel (NULL);
      jtabhi = newiTempLabel (NULL);

      /* get the condition into accumulator.
         Using b as temporary storage, if register push/pop is needed */
      aopOp (cond, ic, FALSE);
      l = aopGet (cond, 0, FALSE, FALSE);
      if ((AOP_TYPE (cond) == AOP_R0 && _G.r0Pushed) ||
          (AOP_TYPE (cond) == AOP_R1 && _G.r1Pushed) ||
          EQ (l, "a") || EQ (l, "acc") ||
          (count > 125 && EQ (l, "dpl")) ||
          IS_VOLATILE (operandType (cond)))
        {
          // (MB) what if B is in use???
          wassertl (!BINUSE, "B was in use");
          MOVB (l);
          l = "b";
          useB = TRUE;
        }
      freeAsmop (cond, NULL, ic, TRUE);
      MOVA (l);
      if (count <= 125)
        {
          emitcode ("add", "a,#(!tlabel-3-.)", labelKey2num (jtablo->key));
          emitcode ("movc", "a,@a+pc");
          if (EQ (l, "dpl"))
            {
              emitcode ("xch", "a,dpl");
            }
          else
            {
              emitcode ("mov", "dpl,a");
              MOVA (l);
            }
          emitcode ("add", "a,#(!tlabel-3-.)", labelKey2num (jtabhi->key));
          emitcode ("movc", "a,@a+pc");
          emitcode ("mov", "dph,a");
        }
      else
        {
          /* this scales up to n<=255, but needs four more bytes
             and changes dptr */
          emitcode ("mov", "dptr,#!tlabel", labelKey2num (jtablo->key));
          emitcode ("movc", "a,@a+dptr");
          if (useB)
            {
              emitcode ("xch", "a,b");
            }
          else
            {
              emitpush ("acc");
              MOVA (l);
            }
          emitcode ("mov", "dptr,#!tlabel", labelKey2num (jtabhi->key));
          emitcode ("movc", "a,@a+dptr");
          emitcode ("mov", "dph,a");
          if (useB)
            emitcode ("mov", "dpl,b");
          else
            emitpop ("dpl");
        }

      emitcode ("clr", "a");
      emitcode ("jmp", "@a+dptr");

      /* now generate jump table, LSB */
      emitLabel (jtablo);
      for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
        emitcode (".db", "!tlabel", labelKey2num (jtab->key));

      /* now generate jump table, MSB */
      emitLabel (jtabhi);
      for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
        emitcode (".db", "!tlabel>>8", labelKey2num (jtab->key));
    }
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
  bool right_boolean;

  D (emitcode (";", "genCast"));

  /* if they are equivalent then do nothing */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  right_boolean = IS_BOOLEAN (rtype) || IS_BITFIELD (rtype) &&  SPEC_BLEN (getSpec (rtype)) == 1;

  /* if the result is a bit (and not a bitfield) */
  if (IS_BOOLEAN (OP_SYMBOL (result)->type) && !right_boolean)
    {
      assignBit (result, right);
      goto release;
    }

  /* if they are the same size : or less */
  if (AOP_SIZE (result) <= AOP_SIZE (right))
    {
      /* if they are in the same place */
      if (sameRegs (AOP (right), AOP (result)))
        goto release;

      /* if they are in different places then copy */
      size = AOP_SIZE (result);
      offset = 0;
      while (size--)
        {
          aopPut (result, aopGet (right, offset, FALSE, FALSE), offset);
          offset++;
        }
      goto release;
    }

  /* if the either is of type pointer */
  if ((IS_PTR (ctype) || IS_PTR (rtype)) && !IS_INTEGRAL (rtype))
    {
      int p_type;
      sym_link *type = operandType (right);
      sym_link *etype = getSpec (type);

      /* pointer to generic pointer or long */
      if (AOP_SIZE (result) >= GPTRSIZE)
        {
          if (IS_PTR (type))
            {
              p_type = DCL_TYPE (type);
            }
          else
            {
              if (SPEC_SCLS (etype) == S_REGISTER)
                {
                  // let's assume it is a generic pointer
                  p_type = GPOINTER;
                }
              else
                {
                  /* we have to go by the storage class */
                  p_type = PTR_TYPE (SPEC_OCLS (etype));
                }
            }

          /* the first two bytes are known */
          size = GPTRSIZE - 1;
          offset = 0;
          while (size--)
            {
              aopPut (result, aopGet (right, offset, FALSE, FALSE), offset);
              offset++;
            }
          /* the third byte depending on type */
          {
            int gpVal;

            /* If there will be no loss of precision, handle generic */
            /* pointer as special case to avoid generating warning */
            if (p_type == GPOINTER && AOP_SIZE (result) >= GPTRSIZE)
              gpVal = -1;
            else
              gpVal = pointerTypeToGPByte (p_type, NULL, NULL);

            if (gpVal == -1)
              {
                // pointerTypeToGPByte will have warned, just copy.
                aopPut (result, aopGet (right, offset, FALSE, FALSE), offset);
              }
            else
              {
                struct dbuf_s dbuf;

                dbuf_init (&dbuf, 128);
                dbuf_printf (&dbuf, "#0x%02x", gpVal);
                aopPut (result, dbuf_c_str (&dbuf), GPTRSIZE - 1);
                dbuf_destroy (&dbuf);
              }
          }

          /* 8051 uses unsigned address spaces, so no sign extension needed. */
          /* Pad the remaining bytes of the result (if any) with 0. */
          size = AOP_SIZE (result) - GPTRSIZE;
          offset = GPTRSIZE;
          while (size--)
            {
              aopPut (result, zero, offset++);
            }
          goto release;
        }

      /* just copy the pointers */
      size = AOP_SIZE (result);
      offset = 0;
      while (size--)
        {
          aopPut (result, aopGet (right, offset, FALSE, FALSE), offset);
          offset++;
        }
      goto release;
    }

  /* so we now know that the size of destination is greater
     than the size of the source */
  /* we move to result for the size of source */
  size = AOP_SIZE (right);
  offset = 0;
  while (size--)
    {
      aopPut (result, aopGet (right, offset, FALSE, FALSE), offset);
      offset++;
    }

  /* now depending on the sign of the source && destination */
  size = AOP_SIZE (result) - AOP_SIZE (right);
  /* if unsigned or not an integral type */
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
      MOVA (aopGet (right, AOP_SIZE (right) - 1, FALSE, FALSE));
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

  aopOp (IC_RESULT (ic), ic, FALSE);

  if (AOP_NEEDSACC (IC_RESULT (ic)))
    {
      /* If the result is accessed indirectly via
       * the accumulator, we must explicitly write
       * it back after the decrement.
       */
      const char *rByte = aopGet (IC_RESULT (ic), 0, FALSE, FALSE);

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
      emitcode ("dec", "%s", aopGet (IC_RESULT (ic), 0, FALSE, FALSE));
      MOVA (aopGet (IC_RESULT (ic), 0, FALSE, FALSE));
      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
      ifx->generated = 1;
      emitcode ("jnz", "!tlabel", labelKey2num (lbl->key));
    }
  else
    {
      emitcode ("djnz", "%s,!tlabel", aopGet (IC_RESULT (ic), 0, FALSE, FALSE), labelKey2num (lbl->key));
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

  D (emitcode (";", "genReceive"));

  if (ic->argreg == 1)
    {
      /* first parameter */
      if ((isOperandInFarSpace (IC_RESULT (ic)) ||
           isOperandInPagedSpace (IC_RESULT (ic))) && (OP_SYMBOL (IC_RESULT (ic))->isspilt || IS_TRUE_SYMOP (IC_RESULT (ic))))
        {
          reg_info *tempRegs[8];
          int receivingA = 0;
          int roffset = 0;

          for (offset = 0; offset < size; offset++)
            if (EQ (fReturn[offset], "a"))
              receivingA = 1;

          if (!receivingA)
            {
              if (size == 1 || getTempRegs (tempRegs, size - 1, ic))
                {
                  for (offset = size - 1; offset > 0; offset--)
                    emitcode ("mov", "%s,%s", tempRegs[roffset++]->name, fReturn[offset]);
                  emitcode ("mov", "a,%s", fReturn[0]);
                  _G.accInUse++;
                  aopOp (IC_RESULT (ic), ic, FALSE);
                  _G.accInUse--;
                  aopPut (IC_RESULT (ic), "a", offset);
                  for (offset = 1; offset < size; offset++)
                    aopPut (IC_RESULT (ic), tempRegs[--roffset]->name, offset);
                  goto release;
                }
            }
          else
            {
              if (getTempRegs (tempRegs, size, ic))
                {
                  for (offset = 0; offset < size; offset++)
                    emitcode ("mov", "%s,%s", tempRegs[offset]->name, fReturn[offset]);
                  aopOp (IC_RESULT (ic), ic, FALSE);
                  for (offset = 0; offset < size; offset++)
                    aopPut (IC_RESULT (ic), tempRegs[offset]->name, offset);
                  goto release;
                }
            }

          offset = fReturnSizeMCS51 - size;
          while (size--)
            {
              emitpush ((!EQ (fReturn[fReturnSizeMCS51 - offset - 1], "a") ? fReturn[fReturnSizeMCS51 - offset - 1] : "acc"));
              offset++;
            }
          aopOp (IC_RESULT (ic), ic, FALSE);
          size = AOP_SIZE (IC_RESULT (ic));
          offset = 0;
          while (size--)
            {
              emitpop ("acc");
              aopPut (IC_RESULT (ic), "a", offset++);
            }
        }
      else
        {
          _G.accInUse++;
          aopOp (IC_RESULT (ic), ic, FALSE);
          _G.accInUse--;
          assignResultValue (IC_RESULT (ic), NULL);
        }
    }
  else if (ic->argreg > 12)
    {
      /* bit parameters */
      reg_info *reg = OP_SYMBOL (IC_RESULT (ic))->regs[0];

      BitBankUsed = 1;
      if (!reg || reg->rIdx != ic->argreg - 5)
        {
          aopOp (IC_RESULT (ic), ic, FALSE);
          emitcode ("mov", "c,%s", rb1regs[ic->argreg - 5]);
          outBitC (IC_RESULT (ic));
        }
    }
  else
    {
      /* other parameters */
      int rb1off;
      aopOp (IC_RESULT (ic), ic, FALSE);
      rb1off = ic->argreg;
      while (size--)
        {
          aopPut (IC_RESULT (ic), rb1regs[rb1off++ - 5], offset++);
        }
    }

release:
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
      aopOp (op, ic, FALSE);

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
              MOVA (aopGet (op, offset, FALSE, FALSE));
              offset++;
            }
        }

      freeAsmop (op, NULL, ic, TRUE);
    }

  op = IC_LEFT (ic);
  if (op && IS_SYMOP (op))
    {
      aopOp (op, ic, FALSE);

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
              MOVA (aopGet (op, offset, FALSE, FALSE));
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
      aopOp (IC_RESULT (ic), ic, TRUE);
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
      aopOp (IC_RIGHT (ic), ic, FALSE);
      if (AOP_TYPE (IC_RIGHT (ic)) == AOP_CRY)
        {
          emitcode ("mov", "c,%s", IC_RIGHT (ic)->aop->aopu.aop_dir);
          emitcode ("mov", "ea,c");
        }
      else
        {
          if (AOP_TYPE (IC_RIGHT (ic)) != AOP_DUMMY)
            MOVA (aopGet (IC_RIGHT (ic), 0, FALSE, FALSE));
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
/* gen51Code - generate code for 8051 based controllers            */
/*-----------------------------------------------------------------*/
void
gen51Code (iCode * lic)
{
  iCode *ic;
  int cln = 0;
#ifdef _DEBUG
  int cseq = 0;
#endif

  _G.currentFunc = NULL;

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
#ifdef _DEBUG
      if (ic->seqPoint && ic->seqPoint != cseq)
        {
          emitcode (";", "sequence point %d", ic->seqPoint);
          cseq = ic->seqPoint;
        }
#endif
      if (options.iCodeInAsm)
        {
          char regsInUse[80];
          int i;
          const char *iLine;

#if 0
          for (i = 0; i < 8; i++)
            {
              sprintf (&regsInUse[i], "%c", ic->riu & (1 << i) ? i + '0' : '-');        /* show riu */
              regsInUse[i] = 0;
            }
#else
          strcpy (regsInUse, "--------");
          for (i = 0; i < 8; i++)
            {
              if (bitVectBitValue (ic->rMask, i))
                {
                  int offset = regs8051[i].offset;
                  regsInUse[offset] = offset + '0';     /* show rMask */
                }
            }
#endif
          iLine = printILine (ic);
          emitcode (";", "[%s] ic:%d: %s", regsInUse, ic->seq, iLine);
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
          assert (0);
          break;

        case GETABIT:
          genGetAbit (ic);
          break;

        case GETBYTE:
          genGetByte (ic);
          break;

        case GETWORD:
          genGetWord (ic);
          break;

        case LEFT_OP:
          genLeftShift (ic);
          break;

        case RIGHT_OP:
          genRightShift (ic);
          break;

        case GET_VALUE_AT_ADDRESS:
          genPointerGet (ic, hasInc (IC_LEFT (ic), ic, getSize (operandType (IC_RESULT (ic)))), ifxForOp (IC_RESULT (ic), ic));
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
          ic = ic;
        }
    }

  genLine.lineElement.ic = NULL;

  /* now we are ready to call the
     peep hole optimizer */
  if (!options.nopeep)
    peepHole (&genLine.lineHead);

  /* now do the actual printing */
  printLine (genLine.lineHead, codeOutBuf);

  /* destroy the line list */
  destroy_line_list ();
}
