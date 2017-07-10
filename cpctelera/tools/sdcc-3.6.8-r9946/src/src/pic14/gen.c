/*-------------------------------------------------------------------------
  gen.c - source file for code generation for pic

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Bug Fixes  -  Wojciech Stryjewski  wstryj1@tiger.lsu.edu (1999 v2.1.9a)
  PIC port:
  Copyright (C) 2000, Scott Dattalo scott@dattalo.com
  Copyright (C) 2005, Raphael Neider <rneider AT web.de>

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
/*
  Notes:
  000123 mlh  Moved aopLiteral to SDCCglue.c to help the split
              Made everything static
*/

/*
 * This is the down and dirty file with all kinds of
 * kludgy & hacky stuff. This is what it is all about
 * CODE GENERATION for a specific MCU . some of the
 * routines may be reusable, will have to see.
 */

#include "device.h"
#include "gen.h"
#include "glue.h"
#include "dbuf_string.h"

/*
 * Imports
 */
extern struct dbuf_s *codeOutBuf;
extern set *externs;

static PIC_device *pic = NULL;

static pCodeOp *popGetImmd (const char *name, unsigned int offset, int index, int is_func);
static pCodeOp *popRegFromString (const char *str, int size, int offset);
static int aop_isLitLike (asmop * aop);
static void genCritical (iCode * ic);
static void genEndCritical (iCode * ic);

/* The PIC port(s) need not differentiate between POINTER and FPOINTER. */
#define PIC_IS_DATA_PTR(x)  (IS_DATA_PTR(x) || IS_FARPTR(x))

/*
 * max_key keeps track of the largest label number used in
 * a function. This is then used to adjust the label offset
 * for the next function.
 */
static int max_key = 0;
static int labelOffset = 0;
static int GpseudoStkPtr = 0;
static int pic14_inISR = 0;

static char *zero = "0x00";
static char *one = "0x01";
static char *spname = "sp";

unsigned fReturnSizePic = 4;    /* shared with ralloc.c */
static char *fReturnpic14[] = { "temp1", "temp2", "temp3", "temp4" };

static char **fReturn = fReturnpic14;

static struct
{
  short accInUse;
  short nRegsSaved;
  set *sendSet;
} _G;

/*
 * Resolved ifx structure. This structure stores information
 * about an iCode ifx that makes it easier to generate code.
 */
typedef struct resolvedIfx
{
  symbol *lbl;                  /* pointer to a label */
  int condition;                /* true or false ifx */
  int generated;                /* set true when the code associated with the ifx
                                 * is generated */
} resolvedIfx;

static pBlock *pb;

/*-----------------------------------------------------------------*/
/*  my_powof2(n) - If `n' is an integer power of 2, then the       */
/*                 exponent of 2 is returned, otherwise -1 is      */
/*                 returned.                                       */
/* note that this is similar to the function `powof2' in SDCCsymt  */
/* if(n == 2^y)                                                    */
/*   return y;                                                     */
/* return -1;                                                      */
/*-----------------------------------------------------------------*/
static int
my_powof2 (unsigned long num)
{
  if (num)
    {
      if ((num & (num - 1)) == 0)
        {
          int nshifts = -1;
          while (num)
            {
              num >>= 1;
              nshifts++;
            }
          return nshifts;
        }
    }

  return -1;
}

void
DEBUGpic14_AopType (int line_no, operand * left, operand * right, operand * result)
{

  DEBUGpic14_emitcode ("; ", "line = %d result %s=%s, size=%d, left %s=%s, size=%d, right %s=%s, size=%d",
                       line_no,
                       ((result) ? AopType (AOP_TYPE (result)) : "-"),
                       ((result) ? aopGet (AOP (result), 0, TRUE, FALSE) : "-"),
                       ((result) ? AOP_SIZE (result) : 0),
                       ((left) ? AopType (AOP_TYPE (left)) : "-"),
                       ((left) ? aopGet (AOP (left), 0, TRUE, FALSE) : "-"),
                       ((left) ? AOP_SIZE (left) : 0),
                       ((right) ? AopType (AOP_TYPE (right)) : "-"),
                       ((right) ? aopGet (AOP (right), 0, FALSE, FALSE) : "-"), ((right) ? AOP_SIZE (right) : 0));

}

static void
DEBUGpic14_AopTypeSign (int line_no, operand * left, operand * right, operand * result)
{

  DEBUGpic14_emitcode ("; ", "line = %d, signs: result %s=%c, left %s=%c, right %s=%c",
                       line_no,
                       ((result) ? AopType (AOP_TYPE (result)) : "-"),
                       ((result) ? (SPEC_USIGN (operandType (result)) ? 'u' : 's') : '-'),
                         ((left) ? AopType (AOP_TYPE (left)) : "-"),
                         ((left) ? (SPEC_USIGN (operandType (left)) ? 'u' : 's') : '-'),
                         ((right) ? AopType (AOP_TYPE (right)) : "-"),
                         ((right) ? (SPEC_USIGN (operandType (right)) ? 'u' : 's') : '-'));

}

void
DEBUGpic14_emitcode (const char *inst, const char *fmt, ...)
{
  va_list ap;

  if (!debug_verbose && !options.debug)
    return;

  va_start (ap, fmt);
  va_emitcode (inst, fmt, ap);
  va_end (ap);

  addpCode2pBlock (pb, newpCodeCharP (genLine.lineCurr->line));
}

void
emitpComment (const char *fmt, ...)
{
  va_list ap;
  struct dbuf_s dbuf;
  const char *line;

  dbuf_init (&dbuf, INITIAL_INLINEASM);

  dbuf_append_char (&dbuf, ';');
  va_start (ap, fmt);
  dbuf_vprintf (&dbuf, fmt, ap);
  va_end (ap);

  line = dbuf_detach_c_str (&dbuf);
  emit_raw (line);
  dbuf_free (line);

  addpCode2pBlock (pb, newpCodeCharP (genLine.lineCurr->line));
}

void
emitpLabel (int key)
{
  addpCode2pBlock (pb, newpCodeLabel (NULL, labelKey2num (key + labelOffset)));
}

/* gen.h defines a macro emitpcode that should be used to call emitpcode
 * as this allows for easy debugging (ever asked the question: where was
 * this instruction geenrated? Here is the answer... */
void
emitpcode_real (PIC_OPCODE poc, pCodeOp * pcop)
{
  if (pcop)
    addpCode2pBlock (pb, newpCode (poc, pcop));
  else
    {
      static int has_warned = FALSE;

      DEBUGpic14_emitcode (";", "%s  ignoring NULL pcop", __FUNCTION__);
      if (!has_warned)
        {
          has_warned = TRUE;
          fprintf (stderr, "WARNING: encountered NULL pcop--this is probably a compiler bug...\n");
        }
    }
}

static void
emitpcodeNULLop (PIC_OPCODE poc)
{
  addpCode2pBlock (pb, newpCode (poc, NULL));
}

/*-----------------------------------------------------------------*/
/* pic14_emitcode - writes the code into a file : for now it is simple    */
/*-----------------------------------------------------------------*/
void
pic14_emitcode (const char *inst, const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  va_emitcode (inst, fmt, ap);
  va_end (ap);

  if (debug_verbose)
    addpCode2pBlock (pb, newpCodeCharP (genLine.lineCurr->line));
}

/*-----------------------------------------------------------------*/
/* pic14_emitDebuggerSymbol - associate the current code location  */
/*   with a debugger symbol                                        */
/*-----------------------------------------------------------------*/
void
pic14_emitDebuggerSymbol (const char *debugSym)
{
  genLine.lineElement.isDebug = TRUE;
  pic14_emitcode ("", ";%s ==.", debugSym);
  genLine.lineElement.isDebug = FALSE;
}

/*-----------------------------------------------------------------*/
/* newAsmop - creates a new asmOp                                  */
/*-----------------------------------------------------------------*/
static asmop *
newAsmop (short type)
{
  asmop *aop;

  aop = Safe_alloc(sizeof(asmop));
  aop->type = type;
  return aop;
}

/*-----------------------------------------------------------------*/
/* resolveIfx - converts an iCode ifx into a form more useful for  */
/*              generating code                                    */
/*-----------------------------------------------------------------*/
static void
resolveIfx (resolvedIfx * resIfx, iCode * ifx)
{
  if (!resIfx)
    return;

  //  DEBUGpic14_emitcode("; ***","%s %d",__FUNCTION__,__LINE__);

  resIfx->condition = TRUE;     /* assume that the ifx is true */
  resIfx->generated = FALSE;    /* indicate that the ifx has not been used */

  if (!ifx)
    {
      resIfx->lbl = NULL;       /* this is wrong: newiTempLabel(NULL);  / * oops, there is no ifx. so create a label */
    }
  else
    {
      if (IC_TRUE (ifx))
        {
          resIfx->lbl = IC_TRUE (ifx);
        }
      else
        {
          resIfx->lbl = IC_FALSE (ifx);
          resIfx->condition = FALSE;
        }
    }

  //  DEBUGpic14_emitcode("; ***","%s lbl->key=%d, (lab offset=%d)",__FUNCTION__,resIfx->lbl->key,labelOffset);

}

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (iCode * ic, symbol * sym, bool result)
{
  asmop *aop;
  memmap *space = SPEC_OCLS (sym->etype);

  DEBUGpic14_emitcode ("; ***", "%s %d", __FUNCTION__, __LINE__);
  /* if already has one */
  if (sym->aop)
    return sym->aop;

  //DEBUGpic14_emitcode(";","%d",__LINE__);
  /* if it is in direct space */
  if (IN_DIRSPACE (space))
    {
      sym->aop = aop = newAsmop (AOP_DIR);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
      DEBUGpic14_emitcode (";", "%d sym->rname = %s, size = %d", __LINE__, sym->rname, aop->size);
      return aop;
    }

  /* special case for a function */
  if (IS_FUNC (sym->type))
    {

      sym->aop = aop = newAsmop (AOP_PCODE);
      aop->aopu.pcop = popGetImmd (sym->rname, 0, 0, 1);
      PCOI (aop->aopu.pcop)->_const = IN_CODESPACE (space);
      PCOI (aop->aopu.pcop)->_function = TRUE;
      PCOI (aop->aopu.pcop)->index = 0;
      aop->size = FPTRSIZE;
      DEBUGpic14_emitcode (";", "%d size = %d, name =%s", __LINE__, aop->size, sym->rname);
      return aop;
    }

  if (IS_ARRAY (sym->type))
    {
      sym->aop = aop = newAsmop (AOP_PCODE);
      aop->aopu.pcop = popGetImmd (sym->rname, 0, 0, 1);
      PCOI (aop->aopu.pcop)->_const = IN_CODESPACE (space);
      PCOI (aop->aopu.pcop)->_function = FALSE;
      PCOI (aop->aopu.pcop)->index = 0;
      aop->size = getSize (sym->etype) * DCL_ELEM (sym->type);

      DEBUGpic14_emitcode (";", "%d size = %d, name =%s", __LINE__, aop->size, sym->rname);
      return aop;
    }

  /* only remaining is far space */
  /* in which case DPTR gets the address */
  sym->aop = aop = newAsmop (AOP_PCODE);

  aop->aopu.pcop = popGetImmd (sym->rname, 0, 0, 0);
  PCOI (aop->aopu.pcop)->_const = IN_CODESPACE (space);
  PCOI (aop->aopu.pcop)->index = 0;

  DEBUGpic14_emitcode (";", "%d: rname %s, val %d, const = %d", __LINE__, sym->rname, 0, PCOI (aop->aopu.pcop)->_const);

  allocDirReg (IC_LEFT (ic));

  aop->size = FPTRSIZE;

  /* if it is in code space */
  if (IN_CODESPACE (space))
    aop->code = TRUE;

  return aop;
}

/*-----------------------------------------------------------------*/
/* aopForRemat - rematerialzes an object                           */
/*-----------------------------------------------------------------*/
static asmop *
aopForRemat (operand * op)      // x symbol *sym)
{
  symbol *sym = OP_SYMBOL (op);
  iCode *ic = NULL;
  asmop *aop = newAsmop (AOP_PCODE);
  int val = 0;

  ic = sym->rematiCode;

  DEBUGpic14_emitcode (";", "%s %d", __FUNCTION__, __LINE__);
  if (IS_OP_POINTER (op))
    {
      DEBUGpic14_emitcode (";", "%s %d IS_OP_POINTER", __FUNCTION__, __LINE__);
    }
  for (;;)
    {
      if (ic->op == '+')
        {
          val += (int) operandLitValue (IC_RIGHT (ic));
        }
      else if (ic->op == '-')
        {
          val -= (int) operandLitValue (IC_RIGHT (ic));
        }
      else
        break;

      ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
    }

  aop->aopu.pcop = popGetImmd (OP_SYMBOL (IC_LEFT (ic))->rname, 0, val, 0);
  PCOI (aop->aopu.pcop)->_const = IS_PTR_CONST (operandType (op));
  PCOI (aop->aopu.pcop)->index = val;

  DEBUGpic14_emitcode (";", "%d: rname %s, val %d, const = %d",
                       __LINE__, OP_SYMBOL (IC_LEFT (ic))->rname, val, IS_PTR_CONST (operandType (op)));

  //  DEBUGpic14_emitcode(";","aop type  %s",AopType(AOP_TYPE(IC_LEFT(ic))));

  allocDirReg (IC_LEFT (ic));

  return aop;
}

static int
aopIdx (asmop * aop, int offset)
{
  if (!aop)
    return -1;

  if (aop->type != AOP_REG)
    return -2;

  return aop->aopu.aop_reg[offset]->rIdx;

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
    return TRUE;

  if (sym1->rname[0] && sym2->rname[0] && strcmp (sym1->rname, sym2->rname) == 0)
    return TRUE;


  /* if left is a tmp & right is not */
  if (IS_ITEMP (op1) && !IS_ITEMP (op2) && sym1->isspilt && (sym1->usl.spillLoc == sym2))
    return TRUE;

  if (IS_ITEMP (op2) && !IS_ITEMP (op1) && sym2->isspilt && sym1->level > 0 && (sym2->usl.spillLoc == sym1))
    return TRUE;

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* pic14_sameRegs - two asmops have the same registers             */
/*-----------------------------------------------------------------*/
bool
pic14_sameRegs (asmop * aop1, asmop * aop2)
{
  int i;

  if (aop1 == aop2)
    return TRUE;

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
void
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

  {
    sym_link *type = operandType (op);
    if (IS_PTR_CONST (type))
      DEBUGpic14_emitcode (";", "%d aop type is const pointer", __LINE__);
  }

  /* if already has a asmop then continue */
  if (op->aop)
    return;

  /* if the underlying symbol has a aop */
  if (IS_SYMOP (op) && OP_SYMBOL (op)->aop)
    {
      DEBUGpic14_emitcode (";", "%d", __LINE__);
      op->aop = OP_SYMBOL (op)->aop;
      return;
    }

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      //DEBUGpic14_emitcode(";","%d - true symop",__LINE__);
      op->aop = aopForSym (ic, OP_SYMBOL (op), result);
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

      DEBUGpic14_emitcode (";", "%d", __LINE__);
      /* rematerialize it NOW */
      if (sym->remat)
        {

          sym->aop = op->aop = aop = aopForRemat (op);
          aop->size = getSize (sym->type);
          //DEBUGpic14_emitcode(";"," %d: size %d, %s\n",__LINE__,aop->size,aop->aopu.aop_immd);
          return;
        }

      if (sym->ruonly)
        {
          if (sym->isptr)       // && sym->uptr
            {
              aop = op->aop = sym->aop = newAsmop (AOP_PCODE);
              aop->aopu.pcop = newpCodeOp (NULL, PO_GPR_POINTER);       //popCopyReg(&pc_fsr);

              //PCOI(aop->aopu.pcop)->_const = 0;
              //PCOI(aop->aopu.pcop)->index = 0;
              /*
                 DEBUGpic14_emitcode(";","%d: rname %s, val %d, const = %d",
                 __LINE__,sym->rname, 0, PCOI(aop->aopu.pcop)->_const);
               */
              //allocDirReg (IC_LEFT(ic));

              aop->size = getSize (sym->type);
              DEBUGpic14_emitcode (";", "%d", __LINE__);
              return;

            }
          else
            {

              unsigned i;

              aop = op->aop = sym->aop = newAsmop (AOP_STR);
              aop->size = getSize (sym->type);
              for (i = 0; i < fReturnSizePic; i++)
                aop->aopu.aop_str[i] = fReturn[i];

              DEBUGpic14_emitcode (";", "%d", __LINE__);
              return;
            }
        }

      /* else spill location  */
      if (sym->isspilt && sym->usl.spillLoc)
        {
          asmop *oldAsmOp = NULL;

          if (getSize (sym->type) != getSize (sym->usl.spillLoc->type))
            {
              /* force a new aop if sizes differ */
              oldAsmOp = sym->usl.spillLoc->aop;
              sym->usl.spillLoc->aop = NULL;
            }
          DEBUGpic14_emitcode (";", "%s %d %s sym->rname = %s, offset %d",
                               __FUNCTION__, __LINE__, sym->usl.spillLoc->rname, sym->rname, sym->usl.spillLoc->offset);

          sym->aop = op->aop = aop = newAsmop (AOP_PCODE);
          if (getSize (sym->type) != getSize (sym->usl.spillLoc->type))
            {
              /* Don't reuse the new aop, go with the last one */
              sym->usl.spillLoc->aop = oldAsmOp;
            }
          //aop->aopu.pcop = popGetImmd(sym->usl.spillLoc->rname,0,sym->usl.spillLoc->offset);
          aop->aopu.pcop = popRegFromString (sym->usl.spillLoc->rname, getSize (sym->type), sym->usl.spillLoc->offset);
          aop->size = getSize (sym->type);

          return;
        }
    }

  {
    sym_link *type = operandType (op);
    if (IS_PTR_CONST (type))
      DEBUGpic14_emitcode (";", "%d aop type is const pointer", __LINE__);
  }

  /* must be in a register */
  DEBUGpic14_emitcode (";", "%d register type nRegs=%d", __LINE__, sym->nRegs);
  sym->aop = op->aop = aop = newAsmop (AOP_REG);
  aop->size = sym->nRegs;
  for (i = 0; i < sym->nRegs; i++)
    aop->aopu.aop_reg[i] = sym->regs[i];
}

/*-----------------------------------------------------------------*/
/* freeAsmop - free up the asmop given to an operand               */
/*----------------------------------------------------------------*/
void
freeAsmop (operand * op, asmop * aaop, iCode * ic, bool pop)
{
  asmop *aop;

  if (!op)
    aop = aaop;
  else
    aop = op->aop;

  if (!aop)
    return;

  aop->freed = TRUE;

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

/*-----------------------------------------------------------------*/
/* pic14aopLiteral - string from a literal value                   */
/*-----------------------------------------------------------------*/
static unsigned int
pic14aopLiteral (value * val, int offset)
{
  union
  {
    float f;
    unsigned char c[4];
  } fl;

  /* if it is a float then it gets tricky */
  /* otherwise it is fairly simple */
  if (!IS_FLOAT (val->type))
    {
      unsigned long v = ulFromVal (val);

      return ((v >> (offset * 8)) & 0xff);
    }

  /* it is type float */
  fl.f = (float) floatFromVal (val);
#ifdef WORDS_BIGENDIAN
  return fl.c[3 - offset];
#else
  return fl.c[offset];
#endif
}

/*-----------------------------------------------------------------*/
/* aopGet - for fetching value of the aop                          */
/*-----------------------------------------------------------------*/
char *
aopGet (asmop * aop, int offset, bool bit16, bool dname)
{
  //DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
  /* offset is greater than
     size then zero */
  assert (aop);
  if (offset > (aop->size - 1) && aop->type != AOP_LIT)
    return zero;

  /* depending on type */
  switch (aop->type)
  {
    case AOP_IMMD:
      if (bit16)
        SNPRINTF(buffer, sizeof(buffer), "%s", aop->aopu.aop_immd);
      else if (offset)
        SNPRINTF(buffer, sizeof(buffer), "(%s >> %d)", aop->aopu.aop_immd, offset * 8);
      else
        SNPRINTF(buffer, sizeof(buffer), "%s", aop->aopu.aop_immd);
      DEBUGpic14_emitcode (";", "%d immd %s", __LINE__, buffer);
      return Safe_strdup(buffer);

    case AOP_DIR:
      if (offset)
        {
          SNPRINTF(buffer, sizeof(buffer), "(%s + %d)", aop->aopu.aop_dir, offset);
          DEBUGpic14_emitcode (";", "oops AOP_DIR did this %s\n", buffer);
        }
      else
        SNPRINTF(buffer, sizeof(buffer), "%s", aop->aopu.aop_dir);
      return Safe_strdup(buffer);

    case AOP_REG:
      //if (dname)
      //    return aop->aopu.aop_reg[offset]->dname;
      //else
      return aop->aopu.aop_reg[offset]->name;

    case AOP_CRY:
      //pic14_emitcode(";","%d",__LINE__);
      return aop->aopu.aop_dir;

    case AOP_LIT:
      SNPRINTF(buffer, sizeof(buffer), "0x%02x", pic14aopLiteral (aop->aopu.aop_lit, offset));
      return Safe_strdup(buffer);

    case AOP_STR:
      aop->coff = offset;
      if (strcmp (aop->aopu.aop_str[offset], "a") == 0 && dname)
        return "acc";
      DEBUGpic14_emitcode (";", "%d - %s", __LINE__, aop->aopu.aop_str[offset]);
      return aop->aopu.aop_str[offset];

    case AOP_PCODE:
    {
      pCodeOp *pcop = aop->aopu.pcop;
      DEBUGpic14_emitcode (";", "%d: aopGet AOP_PCODE type %s", __LINE__, pCodeOpType (pcop));
      if (pcop->name)
        {
          if (pcop->type == PO_IMMEDIATE)
            {
              offset += PCOI (pcop)->index;
            }

          if (offset)
            {
              DEBUGpic14_emitcode (";", "%s offset %d", pcop->name, offset);
              SNPRINTF(buffer, sizeof(buffer), "(%s+%d)", pcop->name, offset);
            }
          else
            {
              DEBUGpic14_emitcode (";", "%s", pcop->name);
              SNPRINTF(buffer, sizeof(buffer), "%s", pcop->name);
            }
        }
      else
        SNPRINTF(buffer, sizeof(buffer), "0x%02x", PCOI (aop->aopu.pcop)->offset);
    }

    return Safe_strdup(buffer);
  }

  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopget got unsupported aop->type");
  exit (0);
}

/*-----------------------------------------------------------------*/
/* popGetTempReg - create a new temporary pCodeOp                  */
/*-----------------------------------------------------------------*/
static pCodeOp *
popGetTempReg (void)
{

  pCodeOp *pcop;

  pcop = newpCodeOp (NULL, PO_GPR_TEMP);
  if (pcop && pcop->type == PO_GPR_TEMP && PCOR (pcop)->r)
    {
      PCOR (pcop)->r->wasUsed = TRUE;
      PCOR (pcop)->r->isFree = FALSE;
    }

  return pcop;
}

/*-----------------------------------------------------------------*/
/* popReleaseTempReg - create a new temporary pCodeOp                  */
/*-----------------------------------------------------------------*/
static void
popReleaseTempReg (pCodeOp * pcop)
{

  if (pcop && pcop->type == PO_GPR_TEMP && PCOR (pcop)->r)
    PCOR (pcop)->r->isFree = TRUE;

}

/*-----------------------------------------------------------------*/
/* popGetLabel - create a new pCodeOp of type PO_LABEL             */
/*-----------------------------------------------------------------*/
pCodeOp *
popGetLabel (unsigned int key)
{

  DEBUGpic14_emitcode ("; ***", "%s  key=%d, label offset %d", __FUNCTION__, key, labelOffset);

  if (key > (unsigned int) max_key)
    max_key = key;

  return newpCodeOpLabel (NULL, labelKey2num (key + labelOffset));
}

/*-------------------------------------------------------------------*/
/* popGetHighLabel - create a new pCodeOp of type PO_LABEL with offset=1 */
/*-------------------------------------------------------------------*/
static pCodeOp *
popGetHighLabel (unsigned int key)
{
  pCodeOp *pcop;
  pcop = popGetLabel (key);
  PCOLAB (pcop)->offset = 1;
  return pcop;
}

/*-----------------------------------------------------------------*/
/* popGetLit - asm operator to pcode operator conversion               */
/*-----------------------------------------------------------------*/
pCodeOp *
popGetLit (unsigned int lit)
{

  return newpCodeOpLit ((unsigned char) lit);
}

/*-----------------------------------------------------------------*/
/* popGetImmd - asm operator to pcode immediate conversion         */
/*-----------------------------------------------------------------*/
static pCodeOp *
popGetImmd (const char *name, unsigned int offset, int index, int is_func)
{

  return newpCodeOpImmd (name, offset, index, 0, is_func);
}

/*-----------------------------------------------------------------*/
/* popGetWithString - asm operator to pcode operator conversion            */
/*-----------------------------------------------------------------*/
static pCodeOp *
popGetWithString (const char *str, int isExtern)
{
  pCodeOp *pcop;


  if (!str)
    {
      fprintf (stderr, "NULL string %s %d\n", __FILE__, __LINE__);
      exit (1);
    }

  pcop = newpCodeOp (str, PO_STR);
  PCOS (pcop)->isPublic = isExtern ? 1 : 0;

  return pcop;
}

pCodeOp *
popGetExternal (const char *str, int isReg)
{
  pCodeOp *pcop;

  if (isReg)
    {
      pcop = newpCodeOpRegFromStr (str);
    }
  else
    {
      pcop = popGetWithString (str, 1);
    }

  if (str)
    {
      symbol *sym;

      for (sym = setFirstItem (externs); sym; sym = setNextItem (externs))
        {
          if (!strcmp (str, sym->rname))
            break;
        }

      if (!sym)
        {
          sym = newSymbol (str, 0);
          strncpy (sym->rname, str, SDCC_NAME_MAX);
          addSet (&externs, sym);
        }                       // if
      sym->used++;
    }
  return pcop;
}

/*-----------------------------------------------------------------*/
/* popRegFromString -                                              */
/*-----------------------------------------------------------------*/
static pCodeOp *
popRegFromString (const char *str, int size, int offset)
{

  pCodeOp *pcop = Safe_alloc(sizeof(pCodeOpReg));
  pcop->type = PO_DIR;

  DEBUGpic14_emitcode (";", "%d", __LINE__);

  if (!str)
    str = "BAD_STRING";

  pcop->name = Safe_strdup(str);

  //pcop->name = Safe_strdup( ( (str) ? str : "BAD STRING"));

  PCOR (pcop)->r = dirregWithName (pcop->name);
  if (PCOR (pcop)->r == NULL)
    {
      //fprintf(stderr,"%d - couldn't find %s in allocated registers, size =%d\n",__LINE__,aop->aopu.aop_dir,aop->size);
      PCOR (pcop)->r = allocRegByName (pcop->name, size);
      DEBUGpic14_emitcode (";", "%d  %s   offset=%d - had to alloc by reg name", __LINE__, pcop->name, offset);
    }
  else
    {
      DEBUGpic14_emitcode (";", "%d  %s   offset=%d", __LINE__, pcop->name, offset);
    }
  PCOR (pcop)->instance = offset;

  return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static pCodeOp *
popRegFromIdx (int rIdx)
{
  pCodeOp *pcop;

  DEBUGpic14_emitcode ("; ***", "%s,%d  , rIdx=0x%x", __FUNCTION__, __LINE__, rIdx);

  pcop = Safe_alloc(sizeof(pCodeOpReg));

  PCOR (pcop)->rIdx = rIdx;
  PCOR (pcop)->r = typeRegWithIdx (rIdx, REG_STK, 1);
  PCOR (pcop)->r->isFree = FALSE;
  PCOR (pcop)->r->wasUsed = TRUE;

  pcop->type = PCOR (pcop)->r->pc_type;


  return pcop;
}

/*-----------------------------------------------------------------*/
/* popGet - asm operator to pcode operator conversion              */
/*-----------------------------------------------------------------*/
pCodeOp *
popGet (asmop * aop, int offset)        //, bool bit16, bool dname)
{
  //char *s = buffer ;
  //char *rs;

  pCodeOp *pcop;

  //DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
  /* offset is greater than
     size then zero */

  assert (aop);


  /* XXX: still needed for BIT operands (AOP_CRY) */
  if ((offset >= aop->size) && (aop->type != AOP_LIT) && (aop->type != AOP_PCODE))
    {
      printf ("%s: (offset[%d] >= AOP_SIZE(op)[%d]) && (AOP_TYPE(op)[%d] != { AOP_LIT, AOP_PCODE })\n",
              __FUNCTION__, offset, aop->size, aop->type);
      return NULL;              //zero;
    }

  /* depending on type */
  switch (aop->type)
    {

    case AOP_IMMD:
      DEBUGpic14_emitcode (";", "%d", __LINE__);
      return popGetImmd (aop->aopu.aop_immd, offset, 0, 0);

    case AOP_DIR:
      return popRegFromString (aop->aopu.aop_dir, aop->size, offset);

    case AOP_REG:
    {
      int rIdx;
      assert (offset < aop->size);
      rIdx = aop->aopu.aop_reg[offset]->rIdx;

      pcop = Safe_alloc(sizeof(pCodeOpReg));
      PCOR (pcop)->rIdx = rIdx;
      PCOR (pcop)->r = pic14_regWithIdx (rIdx);
      PCOR (pcop)->r->wasUsed = TRUE;
      PCOR (pcop)->r->isFree = FALSE;

      PCOR (pcop)->instance = offset;
      pcop->type = PCOR (pcop)->r->pc_type;
      //rs = aop->aopu.aop_reg[offset]->name;
      DEBUGpic14_emitcode (";", "%d rIdx = r0x%X ", __LINE__, rIdx);
      return pcop;
    }

    case AOP_CRY:
      pcop = newpCodeOpBit (aop->aopu.aop_dir, -1, 1);
      PCOR (pcop)->r = dirregWithName (aop->aopu.aop_dir);
      //if(PCOR(pcop)->r == NULL)
      //fprintf(stderr,"%d - couldn't find %s in allocated registers\n",__LINE__,aop->aopu.aop_dir);
      return pcop;

    case AOP_LIT:
      return newpCodeOpLit (pic14aopLiteral (aop->aopu.aop_lit, offset));

    case AOP_STR:
      DEBUGpic14_emitcode (";", "%d  %s", __LINE__, aop->aopu.aop_str[offset]);
      return newpCodeOpRegFromStr (aop->aopu.aop_str[offset]);

    case AOP_PCODE:
      pcop = NULL;
      DEBUGpic14_emitcode (";", "popGet AOP_PCODE (%s + %i) %d %s", pCodeOpType (aop->aopu.pcop), offset,
                           __LINE__, ((aop->aopu.pcop->name) ? (aop->aopu.pcop->name) : "no name"));
      //emitpComment ("popGet; name %s, offset: %i, pcop-type: %s\n", aop->aopu.pcop->name, offset, pCodeOpType (aop->aopu.pcop));
      switch (aop->aopu.pcop->type)
        {
        case PO_IMMEDIATE:
          pcop = pCodeOpCopy (aop->aopu.pcop);
          /* usually we want to access the memory at "<symbol> + offset" (using ->index),
           * but sometimes we want to access the high byte of the symbol's address (using ->offset) */
          PCOI (pcop)->index += offset;
          //PCOI(pcop)->offset = 0;
          break;
        case PO_DIR:
          pcop = pCodeOpCopy (aop->aopu.pcop);
          PCOR (pcop)->instance = offset;
          break;
        default:
          assert (!"unhandled pCode type");
          break;
        }                       // switch
      return pcop;
    }

  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "popGet got unsupported aop->type");
  exit (0);
}

/*-----------------------------------------------------------------*/
/* popGetAddr - access the low/high word of a symbol (immediate)   */
/*              (for non-PO_IMMEDIATEs this is the same as popGet) */
/*-----------------------------------------------------------------*/
pCodeOp *
popGetAddr (asmop * aop, int offset, int index)
{
  if (aop->type == AOP_PCODE && aop->aopu.pcop->type == PO_IMMEDIATE)
    {
      pCodeOp *pcop = aop->aopu.pcop;
      assert (offset <= GPTRSIZE);

      /* special case: index >= 2 should return GPOINTER-style values */
      if (offset == 2)
        {
          pcop = popGetLit (aop->code ? GPTRTAG_CODE : GPTRTAG_DATA);
          return pcop;
        }

      pcop = pCodeOpCopy (pcop);
      /* usually we want to access the memory at "<symbol> + offset" (using ->index),
       * but sometimes we want to access the high byte of the symbol's address (using ->offset) */
      PCOI (pcop)->offset += offset;
      PCOI (pcop)->index += index;
      //fprintf (stderr, "is PO_IMMEDIATE: %s+o%d+i%d (new o%d,i%d)\n", pcop->name,PCOI(pcop)->offset,PCOI(pcop)->index, offset, index);
      return pcop;
    }
  else
    {
      return popGet (aop, offset + index);
    }
}

/*-----------------------------------------------------------------*/
/* aopPut - puts a string for a aop                                */
/*-----------------------------------------------------------------*/
void
aopPut (asmop * aop, const char *s, int offset)
{
  symbol *lbl;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (aop->size && offset > (aop->size - 1))
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut got offset > aop->size");
      exit (0);
    }

  /* will assign value to value */
  /* depending on where it is ofcourse */
  switch (aop->type)
    {
    case AOP_DIR:
      if (offset)
        {
          SNPRINTF(buffer, sizeof(buffer), "(%s + %d)", aop->aopu.aop_dir, offset);
          fprintf (stderr, "oops aopPut:AOP_DIR did this %s\n", s);
        }
      else
        SNPRINTF(buffer, sizeof(buffer), "%s", aop->aopu.aop_dir);

      if (strcmp (buffer, s))
        {
          DEBUGpic14_emitcode (";", "%d", __LINE__);
          if (strcmp (s, "W"))
            pic14_emitcode ("movf", "%s,w", s);

          pic14_emitcode ("movwf", "%s", buffer);

          if (strcmp (s, "W"))
            {
              pic14_emitcode (";BUG!? should have this:movf", "%s,w   %d", s, __LINE__);
              if (offset >= aop->size)
                {
                  emitpcode (POC_CLRF, popGet (aop, offset));
                  break;
                }
              else
                {
                  emitpcode (POC_MOVFW, popGetImmd (s, 0, offset, 0));
                }
            }
          emitpcode (POC_MOVWF, popGet (aop, offset));
        }
      break;

    case AOP_REG:
      if (strcmp (aop->aopu.aop_reg[offset]->name, s) != 0)
        {
          if (strcmp (s, "W") == 0)
            pic14_emitcode ("movf", "%s,w  ; %d", s, __LINE__);

          pic14_emitcode ("movwf", "%s", aop->aopu.aop_reg[offset]->name);

          if (strcmp (s, zero) == 0)
            {
              emitpcode (POC_CLRF, popGet (aop, offset));

            }
          else if (strcmp (s, "W") == 0)
            {
              pCodeOp *pcop = Safe_alloc(sizeof(pCodeOpReg));
              pcop->type = PO_GPR_REGISTER;

              PCOR (pcop)->rIdx = -1;
              PCOR (pcop)->r = NULL;

              DEBUGpic14_emitcode (";", "%d", __LINE__);
              pcop->name = Safe_strdup (s);
              emitpcode (POC_MOVFW, pcop);
              emitpcode (POC_MOVWF, popGet (aop, offset));
            }
          else if (strcmp (s, one) == 0)
            {
              emitpcode (POC_CLRF, popGet (aop, offset));
              emitpcode (POC_INCF, popGet (aop, offset));
            }
          else
            {
              emitpcode (POC_MOVWF, popGet (aop, offset));
            }
        }
      break;

    case AOP_STK:
      if (strcmp (s, "a") == 0)
        pic14_emitcode ("push", "acc");
      else
        pic14_emitcode ("push", "%s", s);
      break;

    case AOP_CRY:
      /* if bit variable */
      if (!aop->aopu.aop_dir)
        {
          pic14_emitcode ("clr", "a");
          pic14_emitcode ("rlc", "a");
        }
      else
        {
          if (s == zero)
            pic14_emitcode ("clr", "%s", aop->aopu.aop_dir);
          else if (s == one)
            pic14_emitcode ("setb", "%s", aop->aopu.aop_dir);
          else if (!strcmp (s, "c"))
            pic14_emitcode ("mov", "%s,c", aop->aopu.aop_dir);
          else
            {
              lbl = newiTempLabel (NULL);

              if (strcmp (s, "a"))
                {
                  MOVA (s);
                }

              pic14_emitcode ("clr", "c");
              pic14_emitcode ("jz", "%05d_DS_", labelKey2num (lbl->key));
              pic14_emitcode ("cpl", "c");
              pic14_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));
              pic14_emitcode ("mov", "%s,c", aop->aopu.aop_dir);
            }
        }
      break;

    case AOP_STR:
      aop->coff = offset;
      if (strcmp (aop->aopu.aop_str[offset], s))
        pic14_emitcode ("mov", "%s,%s ; %d", aop->aopu.aop_str[offset], s, __LINE__);
      break;

    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopPut got unsupported aop->type");
      exit(0);
    }
}

/*-----------------------------------------------------------------*/
/* mov2w_op - generate either a MOVLW or MOVFW based operand type  */
/*-----------------------------------------------------------------*/
static void
mov2w_op (operand * op, int offset)
{
  assert (op);
  FENTRY;

  /* for PO_IMMEDIATEs: use address or value? */
  if (op_isLitLike (op))
    {
      /* access address of op */
      if (AOP_TYPE (op) != AOP_LIT)
        {
          assert (offset < 3);
        }
      if (IS_SYMOP (op) && IS_GENPTR (OP_SYM_TYPE (op)) && AOP_SIZE (op) < offset)
        {
          if (offset == GPTRSIZE - 1)
            emitpcode (POC_MOVLW, popGetLit (GPTRTAG_DATA));
          else
            emitpcode (POC_MOVLW, popGetLit (0));
        }
      else
        emitpcode (POC_MOVLW, popGetAddr (AOP (op), offset, 0));
    }
  else
    {
      /* access value stored in op */
      mov2w (AOP (op), offset);
    }
}


/*-----------------------------------------------------------------*/
/* mov2w - generate either a MOVLW or MOVFW based operand type     */
/*-----------------------------------------------------------------*/
void
mov2w (asmop * aop, int offset)
{

  if (!aop)
    return;

  DEBUGpic14_emitcode ("; ***", "%s  %d  offset=%d", __FUNCTION__, __LINE__, offset);

  if (aop_isLitLike (aop))
    emitpcode (POC_MOVLW, popGetAddr (aop, offset, 0));
  else
    emitpcode (POC_MOVFW, popGet (aop, offset));

}

static void
movwf (asmop * op, int offset)
{
  emitpcode (POC_MOVWF, popGet (op, offset));
}

static pCodeOp *
get_argument_pcop (int idx)
{
  assert (idx > 0 && "the 0th (first) argument is passed via WREG");
  return popRegFromIdx (Gstack_base_addr - (idx - 1));
}

static pCodeOp *
get_return_val_pcop (int offset)
{
  assert (offset > 0 && "the most significant byte is returned via WREG");
  return popRegFromIdx (Gstack_base_addr - (offset - 1));
}

static void
pass_argument (operand * op, int offset, int idx)
{
  if (op)
    mov2w_op (op, offset);
  if (idx != 0)
    emitpcode (POC_MOVWF, get_argument_pcop (idx));
}

static void
get_returnvalue (operand * op, int offset, int idx)
{
  if (idx != 0)
    emitpcode (POC_MOVFW, get_return_val_pcop (idx));
  movwf (AOP (op), offset);
}

static void
call_libraryfunc (char *name)
{
  symbol *sym;

  /* library code might reside in different page... */
  emitpcode (POC_PAGESEL, popGetWithString (name, 1));
  /* call the library function */
  emitpcode (POC_CALL, popGetExternal (name, 0));
  /* might return from different page... */
  emitpcode (POC_PAGESEL, popGetWithString ("$", 0));

  /* create symbol, mark it as `extern' */
  sym = findSym (SymbolTab, NULL, name);
  if (!sym)
    {
      sym = newSymbol (name, 0);
      strncpy (sym->rname, name, SDCC_NAME_MAX);
      addSym (SymbolTab, sym, sym->rname, 0, 0, 0);
      addSet (&externs, sym);
    }                           // if
  sym->used++;
}

/*-----------------------------------------------------------------*/
/* pic14_getDataSize - get the operand data size                   */
/*-----------------------------------------------------------------*/
int
pic14_getDataSize (operand * op)
{
  int size;

  size = AOP_SIZE (op);
  return size;
}

/*-----------------------------------------------------------------*/
/* pic14_outAcc - output Acc                                       */
/*-----------------------------------------------------------------*/
void
pic14_outAcc (operand * result)
{
  int size, offset;
  DEBUGpic14_emitcode ("; ***", "%s  %d - ", __FUNCTION__, __LINE__);
  DEBUGpic14_AopType (__LINE__, NULL, NULL, result);


  size = pic14_getDataSize (result);
  if (size)
    {
      emitpcode (POC_MOVWF, popGet (AOP (result), 0));
      size--;
      offset = 1;
      /* unsigned or positive */
      while (size--)
        emitpcode (POC_CLRF, popGet (AOP (result), offset++));
    }

}

/*-----------------------------------------------------------------*/
/* pic14_outBitC - output a bit C                                  */
/*-----------------------------------------------------------------*/
static void
pic14_outBitC (operand * result)
{

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* if the result is bit */
  if (AOP_TYPE (result) == AOP_CRY)
    aopPut (AOP (result), "c", 0);
  else
    {
      pic14_emitcode ("clr", "a  ; %d", __LINE__);
      pic14_emitcode ("rlc", "a");
      pic14_outAcc (result);
    }
}

/*-----------------------------------------------------------------*/
/* pic14_toBoolean - emit code for orl a,operator(sizeop)          */
/*-----------------------------------------------------------------*/
static void
pic14_toBoolean (operand * oper)
{
  int size = AOP_SIZE (oper);
  int offset = 0;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  assert (size > 0);

  if (size == 1)
    {
      /* MOVFW does not load the flags... */
      emitpcode (POC_MOVLW, popGetLit (0));
      offset = 0;
    }
  else
    {
      emitpcode (POC_MOVFW, popGet (AOP (oper), 0));
      offset = 1;
    }

  while (offset < size)
    {
      emitpcode (POC_IORFW, popGet (AOP (oper), offset++));
    }
  /* Z is set iff (oper == 0) */
}


/*-----------------------------------------------------------------*/
/* genNot - generate code for ! operation                          */
/*-----------------------------------------------------------------*/
static void
genNot (iCode * ic)
{
  //symbol *tlbl;
  int size;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* assign asmOps to operand & result */
  aopOp (IC_LEFT (ic), ic, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE);

  DEBUGpic14_AopType (__LINE__, IC_LEFT (ic), NULL, IC_RESULT (ic));
  /* if in bit space then a special case */
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_CRY)
    {
      if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY)
        {
          emitpcode (POC_MOVLW, popGet (AOP (IC_LEFT (ic)), 0));
          emitpcode (POC_XORWF, popGet (AOP (IC_RESULT (ic)), 0));
        }
      else
        {
          emitpcode (POC_CLRF, popGet (AOP (IC_RESULT (ic)), 0));
          emitpcode (POC_BTFSS, popGet (AOP (IC_LEFT (ic)), 0));
          emitpcode (POC_INCF, popGet (AOP (IC_RESULT (ic)), 0));
        }
      goto release;
    }

  size = AOP_SIZE (IC_LEFT (ic));
  mov2w (AOP (IC_LEFT (ic)), 0);
  while (--size > 0)
    {
      if (op_isLitLike (IC_LEFT (ic)))
        emitpcode (POC_IORLW, popGetAddr (AOP (IC_LEFT (ic)), size, 0));
      else
        emitpcode (POC_IORFW, popGet (AOP (IC_LEFT (ic)), size));
    }
  emitpcode (POC_MOVLW, popGetLit (0));
  emitSKPNZ;
  emitpcode (POC_MOVLW, popGetLit (1));
  movwf (AOP (IC_RESULT (ic)), 0);

  for (size = 1; size < AOP_SIZE (IC_RESULT (ic)); size++)
    {
      emitpcode (POC_CLRF, popGet (AOP (IC_RESULT (ic)), size));
    }
  goto release;

release:
  /* release the aops */
  freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? 0 : 1));
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
}


/*-----------------------------------------------------------------*/
/* genCpl - generate code for complement                           */
/*-----------------------------------------------------------------*/
static void
genCpl (iCode * ic)
{
  operand *left, *result;
  int size, offset = 0;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  /* if both are in bit space then
     a special case */
  if (AOP_TYPE (result) == AOP_CRY && AOP_TYPE (left) == AOP_CRY)
    {

      pic14_emitcode ("mov", "c,%s", left->aop->aopu.aop_dir);
      pic14_emitcode ("cpl", "c");
      pic14_emitcode ("mov", "%s,c", result->aop->aopu.aop_dir);
      goto release;
    }

  size = AOP_SIZE (result);
  if (AOP_SIZE (left) < size)
    size = AOP_SIZE (left);
  while (size--)
    {
      emitpcode (POC_COMFW, popGet (AOP (left), offset));
      emitpcode (POC_MOVWF, popGet (AOP (result), offset));
      offset++;
    }
  addSign (result, AOP_SIZE (left), !SPEC_USIGN (operandType (result)));


release:
  /* release the aops */
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? 0 : 1));
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genUminusFloat - unary minus for floating points                */
/*-----------------------------------------------------------------*/
static void
genUminusFloat (operand * op, operand * result)
{
  int size;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* for this we just need to flip the
     first it then copy the rest in place */
  size = AOP_SIZE (op) - 1;

  mov2w_op (op, size);
  emitpcode (POC_XORLW, popGetLit (0x80));
  movwf (AOP (result), size);

  while (size--)
    {
      mov2w_op (op, size);
      movwf (AOP (result), size);
    }                           // while
}

/*-----------------------------------------------------------------*/
/* genUminus - unary minus code generation                         */
/*-----------------------------------------------------------------*/
static void
genUminus (iCode * ic)
{
  int size, i;
  sym_link *optype;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* assign asmops */
  aopOp (IC_LEFT (ic), ic, FALSE);
  aopOp (IC_RESULT (ic), ic, TRUE);

  /* if both in bit space then special
     case */
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY && AOP_TYPE (IC_LEFT (ic)) == AOP_CRY)
    {

      emitpcode (POC_BCF, popGet (AOP (IC_RESULT (ic)), 0));
      emitpcode (POC_BTFSS, popGet (AOP (IC_LEFT (ic)), 0));
      emitpcode (POC_BSF, popGet (AOP (IC_RESULT (ic)), 0));

      goto release;
    }

  optype = operandType (IC_LEFT (ic));

  /* if float then do float stuff */
  if (IS_FLOAT (optype))
    {
      genUminusFloat (IC_LEFT (ic), IC_RESULT (ic));
      goto release;
    }

  /* otherwise subtract from zero by taking the 2's complement */
  size = AOP_SIZE (IC_LEFT (ic));

  for (i = 0; i < size; i++)
    {
      if (pic14_sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
        emitpcode (POC_COMF, popGet (AOP (IC_LEFT (ic)), i));
      else
        {
          emitpcode (POC_COMFW, popGet (AOP (IC_LEFT (ic)), i));
          emitpcode (POC_MOVWF, popGet (AOP (IC_RESULT (ic)), i));
        }
    }

  emitpcode (POC_INCF, popGet (AOP (IC_RESULT (ic)), 0));
  for (i = 1; i < size; i++)
    {
      emitSKPNZ;
      emitpcode (POC_INCF, popGet (AOP (IC_RESULT (ic)), i));
    }

release:
  /* release the aops */
  freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? 0 : 1));
  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* saverbank - saves an entire register bank on the stack          */
/*-----------------------------------------------------------------*/
static void
saverbank (int bank, iCode * ic, bool pushPsw)
{
  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d - WARNING no code generated", __FUNCTION__, __LINE__);
#if 0
  int i;
  asmop *aop;
  regs *r = NULL;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (options.useXstack)
    {

      aop = newAsmop (0);
      r = getFreePtr (ic, &aop, FALSE);
      pic14_emitcode ("mov", "%s,_spx", r->name);

    }

  for (i = 0; i < pic14_nRegs; i++)
    {
      if (options.useXstack)
        {
          pic14_emitcode ("inc", "%s", r->name);
          //pic14_emitcode("mov","a,(%s+%d)",
          //       regspic14[i].base,8*bank+regspic14[i].offset);
          pic14_emitcode ("movx", "@%s,a", r->name);
        }
      else
        pic14_emitcode ("push", "");    // "(%s+%d)",
      //regspic14[i].base,8*bank+regspic14[i].offset);
    }

  if (pushPsw)
    {
      if (options.useXstack)
        {
          pic14_emitcode ("mov", "a,psw");
          pic14_emitcode ("movx", "@%s,a", r->name);
          pic14_emitcode ("inc", "%s", r->name);
          pic14_emitcode ("mov", "_spx,%s", r->name);
          freeAsmop (NULL, aop, ic, TRUE);

        }
      else
        pic14_emitcode ("push", "psw");

      pic14_emitcode ("mov", "psw,#0x%02x", (bank << 3) & 0x00ff);
    }
  ic->bankSaved = 1;
#endif
}

/*-----------------------------------------------------------------*/
/* saveRegisters - will look for a call and save the registers     */
/*-----------------------------------------------------------------*/
static void
saveRegisters (iCode * lic)
{
  iCode *ic;
  sym_link *dtype;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  /* look for call */
  for (ic = lic; ic; ic = ic->next)
    if (ic->op == CALL || ic->op == PCALL)
      break;

  if (!ic)
    {
      fprintf (stderr, "found parameter push with no function call\n");
      return;
    }

  /* if the registers have been saved already then do nothing */
  if (ic->regsSaved || IFFUNC_CALLEESAVES (OP_SYMBOL (IC_LEFT (ic))->type))
    return;

  /* find the registers in use at this time
     and push them away to safety */
  bitVectCplAnd (bitVectCopy (ic->rMask), ic->rUsed);

  ic->regsSaved = 1;

  //fprintf(stderr, "ERROR: saveRegisters did not do anything to save registers, please report this as a bug.\n");

  dtype = operandType (IC_LEFT (ic));
  if (currFunc && dtype &&
      (FUNC_REGBANK (currFunc->type) != FUNC_REGBANK (dtype)) && IFFUNC_ISISR (currFunc->type) && !ic->bankSaved)
    {
      saverbank (FUNC_REGBANK (dtype), ic, TRUE);
    }
}

/*-----------------------------------------------------------------*/
/* unsaveRegisters - pop the pushed registers                      */
/*-----------------------------------------------------------------*/
static void
unsaveRegisters (iCode * ic)
{
  int i;
  bitVect *rsave;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* find the registers in use at this time
     and push them away to safety */
  rsave = bitVectCplAnd (bitVectCopy (ic->rMask), ic->rUsed);

  if (options.useXstack)
    {
      pic14_emitcode ("mov", "r0,%s", spname);
      for (i = pic14_nRegs; i >= 0; i--)
        {
          if (bitVectBitValue (rsave, i))
            {
              pic14_emitcode ("dec", "r0");
              pic14_emitcode ("movx", "a,@r0");
              pic14_emitcode ("mov", "%s,a", pic14_regWithIdx (i)->name);
            }

        }
      pic14_emitcode ("mov", "%s,r0", spname);
    }                           //else
  //for (i =  pic14_nRegs ; i >= 0 ; i--) {
  //  if (bitVectBitValue(rsave,i))
  //  pic14_emitcode("pop","%s",pic14_regWithIdx(i)->dname);
  //}

}


/*-----------------------------------------------------------------*/
/* pushSide -                */
/*-----------------------------------------------------------------*/
static void
pushSide (operand * oper, int size)
{
#if 0
  int offset = 0;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  while (size--)
    {
      char *l = aopGet (AOP (oper), offset++, FALSE, TRUE);
      if (AOP_TYPE (oper) != AOP_REG && AOP_TYPE (oper) != AOP_DIR && strcmp (l, "a"))
        {
          pic14_emitcode ("mov", "a,%s", l);
          pic14_emitcode ("push", "acc");
        }
      else
        pic14_emitcode ("push", "%s", l);
    }
#endif
}

/*-----------------------------------------------------------------*/
/* assignResultValue -               */
/*-----------------------------------------------------------------*/
static void
assignResultValue (operand * oper)
{
  int size = AOP_SIZE (oper);
  int offset = 0;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  DEBUGpic14_AopType (__LINE__, oper, NULL, NULL);

  /* assign MSB first (passed via WREG) */
  while (size--)
    {
      get_returnvalue (oper, size, offset + GpseudoStkPtr);
      GpseudoStkPtr++;
    }
}


/*-----------------------------------------------------------------*/
/* genIpush - genrate code for pushing this gets a little complex  */
/*-----------------------------------------------------------------*/
static void
genIpush (iCode * ic)
{
  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d - WARNING no code generated", __FUNCTION__, __LINE__);
#if 0
  int size, offset = 0;
  char *l;


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
          l = aopGet (AOP (IC_LEFT (ic)), offset++, FALSE, TRUE);
          if (*l == '#')
            {
              MOVA (l);
              l = "acc";
            }
          pic14_emitcode ("push", "%s", l);
        }
      return;
    }

  /* this is a paramter push: in this case we call
     the routine to find the call and save those
     registers that need to be saved */
  saveRegisters (ic);

  /* then do the push */
  aopOp (IC_LEFT (ic), ic, FALSE);


  // pushSide(IC_LEFT(ic), AOP_SIZE(IC_LEFT(ic)));
  size = AOP_SIZE (IC_LEFT (ic));

  while (size--)
    {
      l = aopGet (AOP (IC_LEFT (ic)), offset++, FALSE, TRUE);
      if (AOP_TYPE (IC_LEFT (ic)) != AOP_REG && AOP_TYPE (IC_LEFT (ic)) != AOP_DIR && strcmp (l, "a"))
        {
          pic14_emitcode ("mov", "a,%s", l);
          pic14_emitcode ("push", "acc");
        }
      else
        pic14_emitcode ("push", "%s", l);
    }

  freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);
#endif
}

/*-----------------------------------------------------------------*/
/* genIpop - recover the registers: can happen only for spilling   */
/*-----------------------------------------------------------------*/
static void
genIpop (iCode * ic)
{
  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d - WARNING no code generated", __FUNCTION__, __LINE__);
  assert (!"genIpop -- unimplemented");
#if 0
  int size, offset;


  /* if the temp was not pushed then */
  if (OP_SYMBOL (IC_LEFT (ic))->isspilt)
    return;

  aopOp (IC_LEFT (ic), ic, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));
  offset = (size - 1);
  while (size--)
    pic14_emitcode ("pop", "%s", aopGet (AOP (IC_LEFT (ic)), offset--, FALSE, TRUE));

  freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);
#endif
}

/*-----------------------------------------------------------------*/
/* unsaverbank - restores the resgister bank from stack            */
/*-----------------------------------------------------------------*/
static void
unsaverbank (int bank, iCode * ic, bool popPsw)
{
  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d - WARNING no code generated", __FUNCTION__, __LINE__);
#if 0
  int i;
  asmop *aop;
  regs *r = NULL;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (popPsw)
    {
      if (options.useXstack)
        {
          aop = newAsmop (0);
          r = getFreePtr (ic, &aop, FALSE);


          pic14_emitcode ("mov", "%s,_spx", r->name);
          pic14_emitcode ("movx", "a,@%s", r->name);
          pic14_emitcode ("mov", "psw,a");
          pic14_emitcode ("dec", "%s", r->name);

        }
      else
        pic14_emitcode ("pop", "psw");
    }

  for (i = (pic14_nRegs - 1); i >= 0; i--)
    {
      if (options.useXstack)
        {
          pic14_emitcode ("movx", "a,@%s", r->name);
          //pic14_emitcode("mov","(%s+%d),a",
          //     regspic14[i].base,8*bank+regspic14[i].offset);
          pic14_emitcode ("dec", "%s", r->name);

        }
      else
        pic14_emitcode ("pop", "");     //"(%s+%d)",
      //regspic14[i].base,8*bank); //+regspic14[i].offset);
    }

  if (options.useXstack)
    {

      pic14_emitcode ("mov", "_spx,%s", r->name);
      freeAsmop (NULL, aop, ic, TRUE);

    }
#endif
}

/*-----------------------------------------------------------------*/
/* genCall - generates a call statement                            */
/*-----------------------------------------------------------------*/
static void
genCall (iCode * ic)
{
  sym_link *dtype;
  symbol *sym;
  char *name;
  int isExtern;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  /* if caller saves & we have not saved then */
  if (!ic->regsSaved)
    saveRegisters (ic);

  /* if we are calling a function that is not using
     the same register bank then we need to save the
     destination registers on the stack */
  dtype = operandType (IC_LEFT (ic));
  if (currFunc && dtype &&
      (FUNC_REGBANK (currFunc->type) != FUNC_REGBANK (dtype)) && IFFUNC_ISISR (currFunc->type) && !ic->bankSaved)

    saverbank (FUNC_REGBANK (dtype), ic, TRUE);

  /* if send set is not empty the assign */
  if (_G.sendSet)
    {
      iCode *sic;
      /* For the Pic port, there is no data stack.
       * So parameters passed to functions are stored
       * in registers. (The pCode optimizer will get
       * rid of most of these :).
       */
      int pseudoStkPtr = -1;
      int firstTimeThruLoop = 1;

      _G.sendSet = reverseSet (_G.sendSet);

      /* First figure how many parameters are getting passed */
      for (sic = setFirstItem (_G.sendSet); sic; sic = setNextItem (_G.sendSet))
        {

          aopOp (IC_LEFT (sic), sic, FALSE);
          pseudoStkPtr += AOP_SIZE (IC_LEFT (sic));
          freeAsmop (IC_LEFT (sic), NULL, sic, FALSE);
        }

      for (sic = setFirstItem (_G.sendSet); sic; sic = setNextItem (_G.sendSet))
        {
          int size, offset = 0;

          aopOp (IC_LEFT (sic), sic, FALSE);
          size = AOP_SIZE (IC_LEFT (sic));

          while (size--)
            {
              DEBUGpic14_emitcode ("; ", "%d left %s", __LINE__, AopType (AOP_TYPE (IC_LEFT (sic))));

              if (!firstTimeThruLoop)
                {
                  /* If this is not the first time we've been through the loop
                   * then we need to save the parameter in a temporary
                   * register. The last byte of the last parameter is
                   * passed in W. */
                  emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - --pseudoStkPtr));

                }
              firstTimeThruLoop = 0;

              mov2w_op (IC_LEFT (sic), offset);
              offset++;
            }
          freeAsmop (IC_LEFT (sic), NULL, sic, TRUE);
        }
      _G.sendSet = NULL;
    }
  /* make the call */
  sym = OP_SYMBOL (IC_LEFT (ic));
  name = sym->rname[0] ? sym->rname : sym->name;
  /*
   * As SDCC emits code as soon as it reaches the end of each
   * function's definition, prototyped functions that are implemented
   * after the current one are always considered EXTERN, which
   * introduces many unneccessary PAGESEL instructions.
   * XXX: Use a post pass to iterate over all `CALL _name' statements
   * and insert `PAGESEL _name' and `PAGESEL $' around the CALL
   * only iff there is no definition of the function in the whole
   * file (might include this in the PAGESEL pass).
   */
  isExtern = IS_EXTERN (sym->etype) || pic14_inISR;
#if 0
  if (isExtern)
    {
      /* Extern functions and ISRs maybe on a different page;
       * must call pagesel */
#endif
      emitpcode (POC_PAGESEL, popGetWithString (name, 1));
#if 0
    }
#endif

  emitpcode (POC_CALL, popGetWithString (name, isExtern));
#if 0
  if (isExtern)
    {
      /* May have returned from a different page;
       * must use pagesel to restore PCLATH before next
       * goto or call instruction */
#endif
      emitpcode (POC_PAGESEL, popGetWithString ("$", 0));
#if 0
    }
#endif
  GpseudoStkPtr = 0;
  /* if we need assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic)) &&
       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {
      _G.accInUse++;
      aopOp (IC_RESULT (ic), ic, FALSE);
      _G.accInUse--;

      assignResultValue (IC_RESULT (ic));

      DEBUGpic14_emitcode ("; ", "%d left %s", __LINE__, AopType (AOP_TYPE (IC_RESULT (ic))));

      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }

  /* if register bank was saved then pop them */
  if (ic->bankSaved)
    unsaverbank (FUNC_REGBANK (dtype), ic, TRUE);

  /* if we hade saved some registers then unsave them */
  if (ic->regsSaved && !IFFUNC_CALLEESAVES (dtype))
    unsaveRegisters (ic);
}

/*-----------------------------------------------------------------*/
/* genPcall - generates a call by pointer statement                */
/*-----------------------------------------------------------------*/
static void
genPcall (iCode * ic)
{
  sym_link *dtype;
  symbol *albl = newiTempLabel (NULL);
  symbol *blbl = newiTempLabel (NULL);
  PIC_OPCODE poc;
  pCodeOp *pcop;
  operand *left;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* if caller saves & we have not saved then */
  if (!ic->regsSaved)
    saveRegisters (ic);

  /* if we are calling a function that is not using
     the same register bank then we need to save the
     destination registers on the stack */
  dtype = operandType (IC_LEFT (ic));
  if (currFunc && dtype && IFFUNC_ISISR (currFunc->type) && (FUNC_REGBANK (currFunc->type) != FUNC_REGBANK (dtype)))
    saverbank (FUNC_REGBANK (dtype), ic, TRUE);

  left = IC_LEFT (ic);
  aopOp (left, ic, FALSE);
  DEBUGpic14_AopType (__LINE__, left, NULL, NULL);

  poc = (op_isLitLike (IC_LEFT (ic)) ? POC_MOVLW : POC_MOVFW);

  pushSide (IC_LEFT (ic), FPTRSIZE);

  /* if send set is not empty, assign parameters */
  if (_G.sendSet)
    {
      DEBUGpic14_emitcode ("; ***", "%s  %d - WARNING arg-passing to indirect call not supported", __FUNCTION__, __LINE__);
      /* no way to pass args - W always gets used to make the call */
    }
  /* first idea - factor out a common helper function and call it.
     But don't know how to get it generated only once in its own block

     if(AOP_TYPE(IC_LEFT(ic)) == AOP_DIR) {
     char *rname;
     char *buffer;
     size_t len;

     rname = IC_LEFT(ic)->aop->aopu.aop_dir;
     DEBUGpic14_emitcode ("; ***","%s  %d AOP_DIR %s",__FUNCTION__,__LINE__,rname);
     len = strlen(rname) + 16;
     buffer = Safe_alloc(len);
     SNPRINTF(buffer, len, "%s_goto_helper", rname);
     addpCode2pBlock(pb,newpCode(POC_CALL,newpCodeOp(buffer,PO_STR)));
     free(buffer);
     }
   */
  emitpcode (POC_CALL, popGetLabel (albl->key));
  pcop = popGetLabel (blbl->key);
  emitpcode (POC_PAGESEL, pcop);        /* Must restore PCLATH before goto, without destroying W */
  emitpcode (POC_GOTO, pcop);
  emitpLabel (albl->key);

  emitpcode (poc, popGetAddr (AOP (left), 1, 0));
  emitpcode (POC_MOVWF, popCopyReg (&pc_pclath));
  emitpcode (poc, popGetAddr (AOP (left), 0, 0));
  emitpcode (POC_MOVWF, popCopyReg (&pc_pcl));

  emitpLabel (blbl->key);

  freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);

  /* if we need to assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic)) &&
       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {

      _G.accInUse++;
      aopOp (IC_RESULT (ic), ic, FALSE);
      _G.accInUse--;

      GpseudoStkPtr = 0;

      assignResultValue (IC_RESULT (ic));

      freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }

  /* if register bank was saved then unsave them */
  if (currFunc && dtype && (FUNC_REGBANK (currFunc->type) != FUNC_REGBANK (dtype)))
    unsaverbank (FUNC_REGBANK (dtype), ic, TRUE);

  /* if we hade saved some registers then
     unsave them */
  if (ic->regsSaved)
    unsaveRegisters (ic);
}

/*-----------------------------------------------------------------*/
/* resultRemat - result  is rematerializable                       */
/*-----------------------------------------------------------------*/
static int
resultRemat (iCode * ic)
{
  //  DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
  FENTRY;

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
  symbol *sym;
  sym_link *ftype;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d curr label offset=%dprevious max_key=%d ", __FUNCTION__, __LINE__, labelOffset,
                       max_key);

  labelOffset += (max_key + 4);
  max_key = 0;
  GpseudoStkPtr = 0;
  _G.nRegsSaved = 0;
  /* create the function header */
  pic14_emitcode (";", "-----------------------------------------");
  pic14_emitcode (";", " function %s", (sym = OP_SYMBOL (IC_LEFT (ic)))->name);
  pic14_emitcode (";", "-----------------------------------------");

  /* prevent this symbol from being emitted as 'extern' */
  pic14_stringInSet (sym->rname, &pic14_localFunctions, 1);

  pic14_emitcode ("", "%s:", sym->rname);
  addpCode2pBlock (pb, newpCodeFunction (moduleName, sym->rname, !IS_STATIC (sym->etype), IFFUNC_ISISR (sym->type)));

  /* mark symbol as NOT extern (even if it was declared so previously) */
  assert (IS_SPEC (sym->etype));
  SPEC_EXTR (sym->etype) = 0;
  sym->cdef = 0;
  if (!SPEC_OCLS (sym->etype))
    SPEC_OCLS (sym->etype) = code;
  addSetIfnotP (&SPEC_OCLS (sym->etype)->syms, sym);

  ftype = operandType (IC_LEFT (ic));

  /* here we need to generate the equates for the
     register bank if required */
#if 0
  if (FUNC_REGBANK (ftype) != rbank)
    {
      int i;

      rbank = FUNC_REGBANK (ftype);
      for (i = 0; i < pic14_nRegs; i++)
        {
          if (strcmp (regspic14[i].base, "0") == 0)
            pic14_emitcode ("", "%s = 0x%02x", regspic14[i].dname, 8 * rbank + regspic14[i].offset);
          else
            pic14_emitcode ("", "%s = %s + 0x%02x", regspic14[i].dname, regspic14[i].base, 8 * rbank + regspic14[i].offset);
        }
    }
#endif

  /* if this is an interrupt service routine */
  pic14_inISR = 0;
  if (IFFUNC_ISISR (sym->type))
    {
      pic14_inISR = 1;

      /* generate ISR prolog if and not naked */
      if (!IFFUNC_ISNAKED (sym->type))
        {
          if (pic->isEnhancedCore)
            {
              /*
               * Enhanced CPUs have automatic context saving for W,
               * STATUS, BSR, FSRx, and PCLATH in shadow registers.
               */
              emitpcode (POC_CLRF, popCopyReg (&pc_pclath));
            }
          else
            {
              emitpcode (POC_MOVWF, popCopyReg (&pc_wsave));
              emitpcode (POC_SWAPFW, popCopyReg (&pc_status));
              /* XXX: Why? Does this assume that ssave and psave reside
               * in a shared bank or bank0? We cannot guarantee the
               * latter...
               */
              emitpcode (POC_CLRF, popCopyReg (&pc_status));
              emitpcode (POC_MOVWF, popCopyReg (&pc_ssave));
              //emitpcode(POC_MOVWF,  popGetExternal("___sdcc_saved_status",1 ));
              emitpcode (POC_MOVFW, popCopyReg (&pc_pclath));
              /* during an interrupt PCLATH must be cleared before a goto or call statement */
              emitpcode (POC_CLRF, popCopyReg (&pc_pclath));
              emitpcode (POC_MOVWF, popCopyReg (&pc_psave));
              //emitpcode(POC_MOVWF,  popGetExternal("___sdcc_saved_pclath", 1));
              emitpcode (POC_MOVFW, popCopyReg (&pc_fsr));
              emitpcode (POC_MOVWF, popGetExternal ("___sdcc_saved_fsr", 1));
            }
        }

      pBlockConvert2ISR (pb);
      pic14_hasInterrupt = 1;
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
              /* save the registers used */
              for (i = 0; i < sym->regsUsed->size; i++)
                {
                  if (bitVectBitValue (sym->regsUsed, i))
                    {
                      //pic14_emitcode("push","%s",pic14_regWithIdx(i)->dname);
                      _G.nRegsSaved++;
                    }
                }
            }
        }
    }

  /* if critical function then turn interrupts off */
  if (IFFUNC_ISCRITICAL (ftype))
    {
      genCritical (NULL);
      if (IFFUNC_ARGS (sym->type))
        {
          fprintf (stderr, "PIC14: Functions with __critical (%s) must not have arguments for now.\n", sym->name);
          exit (1);
        }                       // if
    }                           // if

  /* set the register bank to the desired value */
  if (FUNC_REGBANK (sym->type) || FUNC_ISISR (sym->type))
    {
      pic14_emitcode ("push", "psw");
      pic14_emitcode ("mov", "psw,#0x%02x", (FUNC_REGBANK (sym->type) << 3) & 0x00ff);
    }

  if (IFFUNC_ISREENT (sym->type) || options.stackAuto)
    {

      if (options.useXstack)
        {
          pic14_emitcode ("mov", "r0,%s", spname);
          pic14_emitcode ("mov", "a,_bp");
          pic14_emitcode ("movx", "@r0,a");
          pic14_emitcode ("inc", "%s", spname);
        }
      else
        {
          /* set up the stack */
          pic14_emitcode ("push", "_bp");       /* save the callers stack  */
        }
      pic14_emitcode ("mov", "_bp,%s", spname);
    }

  /* adjust the stack for the function */
  if (sym->stack)
    {

      int i = sym->stack;
      if (i > 256)
        werror (W_STACK_OVERFLOW, sym->name);

      if (i > 3 && sym->recvSize < 4)
        {

          pic14_emitcode ("mov", "a,sp");
          pic14_emitcode ("add", "a,#0x%02x", ((char) sym->stack & 0xff));
          pic14_emitcode ("mov", "sp,a");

        }
      else
        while (i--)
          pic14_emitcode ("inc", "sp");
    }

  if (sym->xstack)
    {

      pic14_emitcode ("mov", "a,_spx");
      pic14_emitcode ("add", "a,#0x%02x", ((char) sym->xstack & 0xff));
      pic14_emitcode ("mov", "_spx,a");
    }

}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (IFFUNC_ISREENT (sym->type) || options.stackAuto)
    {
      pic14_emitcode ("mov", "%s,_bp", spname);
    }

  /* if use external stack but some variables were
     added to the local stack then decrement the
     local stack */
  if (options.useXstack && sym->stack)
    {
      pic14_emitcode ("mov", "a,sp");
      pic14_emitcode ("add", "a,#0x%02x", ((char) - sym->stack) & 0xff);
      pic14_emitcode ("mov", "sp,a");
    }


  if ((IFFUNC_ISREENT (sym->type) || options.stackAuto))
    {
      if (options.useXstack)
        {
          pic14_emitcode ("mov", "r0,%s", spname);
          pic14_emitcode ("movx", "a,@r0");
          pic14_emitcode ("mov", "_bp,a");
          pic14_emitcode ("dec", "%s", spname);
        }
      else
        {
          pic14_emitcode ("pop", "_bp");
        }
    }

  /* restore the register bank    */
  if (FUNC_REGBANK (sym->type) || FUNC_ISISR (sym->type))
    pic14_emitcode ("pop", "psw");

  /* if critical function then turn interrupts off */
  if (IFFUNC_ISCRITICAL (sym->type))
    {
      genEndCritical (NULL);
    }                           // if

  if (IFFUNC_ISISR (sym->type))
    {

      /* now we need to restore the registers */
      /* if this isr has no bank i.e. is going to
         run with bank 0 , then we need to save more
         registers :-) */
      if (!FUNC_REGBANK (sym->type))
        {

          /* if this function does not call any other
             function then we can be economical and
             save only those registers that are used */
          if (!IFFUNC_HASFCALL (sym->type))
            {
              int i;

              /* if any registers used */
              if (sym->regsUsed)
                {
                  /* save the registers used */
                  for (i = sym->regsUsed->size; i >= 0; i--)
                    {
                      if (bitVectBitValue (sym->regsUsed, i))
                        {
                          pic14_emitcode ("pop", "junk");       //"%s",pic14_regWithIdx(i)->dname);
                        }
                    }
                }

            }
          else
            {
              /* this function has a function call; cannot
                 determines register usage so we will have the
                 entire bank */
              unsaverbank (0, ic, FALSE);
            }
        }

      /* if debug then send end of function */
      if (options.debug && debugFile && currFunc)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      /* generate ISR epilog if not enhanced core and not naked */
      if (!pic->isEnhancedCore && !IFFUNC_ISNAKED (sym->type))
        {
          emitpcode (POC_MOVFW, popGetExternal ("___sdcc_saved_fsr", 1));
          emitpcode (POC_MOVWF, popCopyReg (&pc_fsr));
          //emitpcode(POC_MOVFW,  popGetExternal("___sdcc_saved_pclath", 1));
          emitpcode (POC_MOVFW, popCopyReg (&pc_psave));
          emitpcode (POC_MOVWF, popCopyReg (&pc_pclath));
          emitpcode (POC_CLRF, popCopyReg (&pc_status));        // see genFunction
          //emitpcode(POC_SWAPFW, popGetExternal("___sdcc_saved_status", 1));
          emitpcode (POC_SWAPFW, popCopyReg (&pc_ssave));
          emitpcode (POC_MOVWF, popCopyReg (&pc_status));
          emitpcode (POC_SWAPF, popCopyReg (&pc_wsave));
          emitpcode (POC_SWAPFW, popCopyReg (&pc_wsave));
        }
      addpCode2pBlock (pb, newpCodeLabel ("END_OF_INTERRUPT", -1));
      emitpcodeNULLop (POC_RETFIE);
    }
  else
    {
      if (IFFUNC_ISCRITICAL (sym->type))
        pic14_emitcode ("setb", "ea");

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
                    {
                      pic14_emitcode ("pop", "junk");   //"%s",pic14_regWithIdx(i)->dname);
                    }
                }
            }
        }

      /* if debug then send end of function */
      if (options.debug && debugFile && currFunc)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      pic14_emitcode ("return", "");
      emitpcodeNULLop (POC_RETURN);

      /* Mark the end of a function */
      addpCode2pBlock (pb, newpCodeFunction (moduleName, NULL, 0, 0));
    }

}

/*-----------------------------------------------------------------*/
/* genRet - generate code for return statement                     */
/*-----------------------------------------------------------------*/
static void
genRet (iCode * ic)
{
  int size, offset = 0;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* if we have no return value then
     just generate the "ret" */
  if (!IC_LEFT (ic))
    goto jumpret;

  /* we have something to return then
     move the return value into place */
  aopOp (IC_LEFT (ic), ic, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));

  for (offset = 0; offset < size; offset++)
    {
      pass_argument (IC_LEFT (ic), offset, size - 1 - offset);
    }

  freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);

jumpret:
  /* generate a jump to the return label
     if the next is not the return statement */
  if (!(ic->next && ic->next->op == LABEL && IC_LABEL (ic->next) == returnLabel))
    {

      emitpcode (POC_GOTO, popGetLabel (returnLabel->key));
    }

}

static set *critical_temps = NULL;

static void
genCritical (iCode * ic)
{
  pCodeOp *saved_intcon;

  (void) ic;

  if (!critical_temps)
    critical_temps = newSet ();

  saved_intcon = popGetTempReg ();
  addSetHead (&critical_temps, saved_intcon);

  /* This order saves one BANKSEL back to INTCON. */
  emitpcode (POC_MOVFW, popCopyReg (&pc_intcon));
  emitpcode (POC_BCF, popCopyGPR2Bit (popCopyReg (&pc_intcon), 7));
  emitpcode (POC_MOVWF, pCodeOpCopy (saved_intcon));
}

static void
genEndCritical (iCode * ic)
{
  pCodeOp *saved_intcon = NULL;

  (void) ic;

  saved_intcon = getSet (&critical_temps);
  if (!saved_intcon)
    {
      fprintf (stderr, "Critical section left, but none entered -- ignoring for now.\n");
      return;
    }                           // if

  emitpcode (POC_BTFSC, popCopyGPR2Bit (pCodeOpCopy (saved_intcon), 7));
  emitpcode (POC_BSF, popCopyGPR2Bit (popCopyReg (&pc_intcon), 7));
  popReleaseTempReg (saved_intcon);
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void
genLabel (iCode * ic)
{
  FENTRY;

  /* special case never generate */
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (IC_LABEL (ic) == entryLabel)
    return;

  emitpLabel (IC_LABEL (ic)->key);
  pic14_emitcode ("", "_%05d_DS_:", labelKey2num (IC_LABEL (ic)->key + labelOffset));
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a goto                                      */
/*-----------------------------------------------------------------*/
//tsd
static void
genGoto (iCode * ic)
{
  FENTRY;

  emitpcode (POC_GOTO, popGetLabel (IC_LABEL (ic)->key));
  pic14_emitcode ("goto", "_%05d_DS_", labelKey2num (IC_LABEL (ic)->key + labelOffset));
}


/*-----------------------------------------------------------------*/
/* genMultbits :- multiplication of bits                           */
/*-----------------------------------------------------------------*/
static void
genMultbits (operand * left, operand * right, operand * result)
{
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (!pic14_sameRegs (AOP (result), AOP (right)))
    emitpcode (POC_BSF, popGet (AOP (result), 0));

  emitpcode (POC_BTFSC, popGet (AOP (right), 0));
  emitpcode (POC_BTFSS, popGet (AOP (left), 0));
  emitpcode (POC_BCF, popGet (AOP (result), 0));

}


/*-----------------------------------------------------------------*/
/* genMultOneByte : 8 bit multiplication & division                */
/*-----------------------------------------------------------------*/
static void
genMultOneByte (operand * left, operand * right, operand * result)
{
  char *func[] = { NULL, "__mulchar", "__mulint", NULL, "__mullong" };

  // symbol *lbl ;
  int size, offset, i;


  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  DEBUGpic14_AopType (__LINE__, left, right, result);
  DEBUGpic14_AopTypeSign (__LINE__, left, right, result);

  /* (if two literals, the value is computed before) */
  /* if one literal, literal on the right */
  if (AOP_TYPE (left) == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
    }

  assert (AOP_SIZE (left) == AOP_SIZE (right));

  size = min (AOP_SIZE (result), AOP_SIZE (left));
  offset = Gstack_base_addr - (2 * size - 1);

  /* pass right operand as argument */
  for (i = 0; i < size; i++)
    {
      mov2w (AOP (right), i);
      emitpcode (POC_MOVWF, popRegFromIdx (++offset));
    }                           // for

  /* pass left operand as argument */
  for (i = 0; i < size; i++)
    {
      mov2w (AOP (left), i);
      if (i != size - 1)
        emitpcode (POC_MOVWF, popRegFromIdx (++offset));
    }                           // for
  assert (offset == Gstack_base_addr);

  /* call library routine */
  assert (size > 0 && size <= 4);
  call_libraryfunc (func[size]);

  /* assign result */
  movwf (AOP (result), size - 1);
  for (i = 0; i < size - 1; i++)
    {
      emitpcode (POC_MOVFW, popRegFromIdx (Gstack_base_addr - i));
      movwf (AOP (result), size - 2 - i);
    }                           // for

  /* now (zero-/sign) extend the result to its size */
  addSign (result, AOP_SIZE (left), !SPEC_USIGN (operandType (result)));
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

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* assign the amsops */
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  DEBUGpic14_AopType (__LINE__, left, right, result);

  /* special cases first */
  /* both are bits */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      genMultbits (left, right, result);
      goto release;
    }

  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
    {
      genMultOneByte (left, right, result);
      goto release;
    }

  /* should have been converted to function call */
  assert (0);

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genDivbits :- division of bits                                  */
/*-----------------------------------------------------------------*/
static void
genDivbits (operand * left, operand * right, operand * result)
{

  char *l;

  FENTRY;

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* the result must be bit */
  pic14_emitcode ("mov", "b,%s", aopGet (AOP (right), 0, FALSE, FALSE));
  l = aopGet (AOP (left), 0, FALSE, FALSE);

  MOVA (l);

  pic14_emitcode ("div", "ab");
  pic14_emitcode ("rrc", "a");
  aopPut (AOP (result), "c", 0);
}

/*-----------------------------------------------------------------*/
/* genDivOneByte : 8 bit division                                  */
/*-----------------------------------------------------------------*/
static void
genDivOneByte (operand * left, operand * right, operand * result)
{
  int sign;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  assert (AOP_SIZE (right) == 1);
  assert (AOP_SIZE (left) == 1);

  sign = !(SPEC_USIGN (operandType (left)) && SPEC_USIGN (operandType (right)));

  if (AOP_TYPE (right) == AOP_LIT)
    {
      /* XXX: might add specialized code */
    }

  if (!sign)
    {
      /* unsigned division */
#if 1
      mov2w (AOP (right), 0);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
      mov2w (AOP (left), 0);
      call_libraryfunc ("__divuchar");
      movwf (AOP (result), 0);
#else
      pCodeOp *temp;
      symbol *lbl;

      temp = popGetTempReg ();
      lbl = newiTempLabel (NULL);

      /* XXX: improve this naive approach:
         [result] = [a] / [b]
         ::= [result] = 0; while ([a] > [b]) do [a] -= [b]; [result]++ done

         In PIC assembler:
         movf  left,W
         movwf temp       // temp <-- left
         movf  right,W    // W <-- right
         clrf  result
         label1:
         incf  result
         subwf temp,F     // temp <-- temp - W
         skipNC       // subwf clears CARRY (i.e. sets BORROW) if temp < W
         goto  label1
         decf result      // we just subtract once too often
       */

      /* XXX: This loops endlessly on DIVIDE BY ZERO */
      /* We need 1..128 iterations of the loop body (`4 / 5' .. `255 / 2'). */

      mov2w (AOP (left), 0);
      emitpcode (POC_MOVWF, temp);
      mov2w (AOP (right), 0);
      emitpcode (POC_CLRF, popGet (AOP (result), 0));

      emitpLabel (lbl->key);
      emitpcode (POC_INCF, popGet (AOP (result), 0));
      emitpcode (POC_SUBWF, temp);
      emitSKPNC;

      if (pic->isEnhancedCore)
        {
          emitpcode (POC_BRA, popGetLabel (lbl->key));
        }
      else
        {
          emitpcode (POC_GOTO, popGetLabel (lbl->key));
        }

      emitpcode (POC_DECF, popGet (AOP (result), 0));
      popReleaseTempReg (temp);
#endif
    }
  else
    {
      /* signed division */
      mov2w (AOP (right), 0);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
      mov2w (AOP (left), 0);
      call_libraryfunc ("__divschar");
      movwf (AOP (result), 0);
    }

  /* now performed the signed/unsigned division -- extend result */
  addSign (result, 1, sign);
}

/*-----------------------------------------------------------------*/
/* genDiv - generates code for division                */
/*-----------------------------------------------------------------*/
static void
genDiv (iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* assign the amsops */
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
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genModOneByte : 8 bit modulus                                   */
/*-----------------------------------------------------------------*/
static void
genModOneByte (operand * left, operand * right, operand * result)
{
  int sign;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  assert (AOP_SIZE (right) == 1);
  assert (AOP_SIZE (left) == 1);

  sign = !(SPEC_USIGN (operandType (left)) && SPEC_USIGN (operandType (right)));

  if (AOP_TYPE (right) == AOP_LIT)
    {
      /* XXX: might add specialized code */
    }

  if (!sign)
    {
      /* unsigned division */
#if 1
      mov2w (AOP (right), 0);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
      mov2w (AOP (left), 0);
      call_libraryfunc ("__moduchar");
      movwf (AOP (result), 0);
#else
      pCodeOp *temp;
      symbol *lbl;

      lbl = newiTempLabel (NULL);

      assert (!pic14_sameRegs (AOP (right), AOP (result)));

      /* XXX: improve this naive approach:
         [result] = [a] % [b]
         ::= [result] = [a]; while ([result] > [b]) do [result] -= [b]; done

         In PIC assembler:
         movf  left,W
         movwf result     // result <-- left
         movf  right,W    // W <-- right
         label1:
         subwf result,F   // result <-- result - W
         skipNC       // subwf clears CARRY (i.e. sets BORROW) if result < W
         goto  label1
         addwf result, F  // we just subtract once too often
       */

      /* XXX: This loops endlessly on DIVIDE BY ZERO */
      /* We need 1..128 iterations of the loop body (`4 % 5' .. `255 % 2'). */

      if (!pic14_sameRegs (AOP (left), AOP (result)))
        {
          mov2w (AOP (left), 0);
          emitpcode (POC_MOVWF, popGet (AOP (result), 0));
        }
      mov2w (AOP (right), 0);

      emitpLabel (lbl->key);
      emitpcode (POC_SUBWF, popGet (AOP (result), 0));
      emitSKPNC;

      if (pic->isEnhancedCore)
        {
          emitpcode (POC_BRA, popGetLabel (lbl->key));
        }
      else
        {
          emitpcode (POC_GOTO, popGetLabel (lbl->key));
        }

      emitpcode (POC_ADDWF, popGet (AOP (result), 0));
#endif
    }
  else
    {
      /* signed division */
      mov2w (AOP (right), 0);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
      mov2w (AOP (left), 0);
      call_libraryfunc ("__modschar");
      movwf (AOP (result), 0);
    }

  /* now we performed the signed/unsigned modulus -- extend result */
  addSign (result, 1, sign);
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

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* assign the amsops */
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
    {
      genModOneByte (left, right, result);
      goto release;
    }

  /* should have been converted to function call */
  assert (0);

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genIfxJump :- will create a jump depending on the ifx           */
/*-----------------------------------------------------------------*/
/*
note: May need to add parameter to indicate when a variable is in bit space.
*/
static void
genIfxJump (iCode * ic, char *jval)
{

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* if true label then we jump if condition
     supplied is true */
  if (IC_TRUE (ic))
    {

      if (strcmp (jval, "a") == 0)
        emitSKPZ;
      else if (strcmp (jval, "c") == 0)
        emitSKPC;
      else
        {
          DEBUGpic14_emitcode ("; ***", "%d - assuming %s is in bit space", __LINE__, jval);
          emitpcode (POC_BTFSC, newpCodeOpBit (jval, -1, 1));
        }

      emitpcode (POC_GOTO, popGetLabel (IC_TRUE (ic)->key));
      pic14_emitcode (" goto", "_%05d_DS_", labelKey2num (IC_TRUE (ic)->key + labelOffset));

    }
  else
    {
      /* false label is present */
      if (strcmp (jval, "a") == 0)
        emitSKPNZ;
      else if (strcmp (jval, "c") == 0)
        emitSKPNC;
      else
        {
          DEBUGpic14_emitcode ("; ***", "%d - assuming %s is in bit space", __LINE__, jval);
          emitpcode (POC_BTFSS, newpCodeOpBit (jval, -1, 1));
        }

      emitpcode (POC_GOTO, popGetLabel (IC_FALSE (ic)->key));
      pic14_emitcode (" goto", "_%05d_DS_", labelKey2num (IC_FALSE (ic)->key + labelOffset));

    }


  /* mark the icode as generated */
  ic->generated = TRUE;
}

/*-----------------------------------------------------------------*/
/* genSkipc                                                        */
/*-----------------------------------------------------------------*/
static void
genSkipc (resolvedIfx * rifx)
{
  FENTRY;
  if (!rifx)
    return;

  if (rifx->condition)
    emitSKPNC;
  else
    emitSKPC;

  emitpcode (POC_GOTO, popGetLabel (rifx->lbl->key));
  emitpComment ("%s:%u: created from rifx:%p", __FUNCTION__, __LINE__, rifx);
  rifx->generated = TRUE;
}

#define isAOP_REGlike(x)  (AOP_TYPE(x) == AOP_REG || AOP_TYPE(x) == AOP_DIR || AOP_TYPE(x) == AOP_PCODE)
#define isAOP_LIT(x)      (AOP_TYPE(x) == AOP_LIT)
#define DEBUGpc           emitpComment

/*-----------------------------------------------------------------*/
/* mov2w_regOrLit :- move to WREG either the offset's byte from    */
/*                  aop (if it's NOT a literal) or from lit (if    */
/*                  aop is a literal)                              */
/*-----------------------------------------------------------------*/
static void
pic14_mov2w_regOrLit (asmop * aop, unsigned long lit, int offset)
{
  if (aop->type == AOP_LIT)
    {
      emitpcode (POC_MOVLW, popGetLit ((lit >> (offset * 8)) & 0x00FF));
    }
  else
    {
      emitpcode (POC_MOVFW, popGet (aop, offset));
    }
}

/* genCmp performs a left < right comparison, stores
 * the outcome in result (if != NULL) and generates
 * control flow code for the ifx (if != NULL).
 *
 * This version leaves in sequences like
 * "B[CS]F STATUS,0; BTFS[CS] STATUS,0"
 * which should be optmized by the peephole
 * optimizer - RN 2005-01-01 */
static void
genCmp (operand * left, operand * right, operand * result, iCode * ifx, int sign)
{
  resolvedIfx rIfx;
  int size;
  int offs;
  symbol *templbl;
  operand *dummy;
  unsigned long lit;
  unsigned long mask;
  int performedLt;
  int invert_result = 0;

  FENTRY;

  assert (AOP_SIZE (left) == AOP_SIZE (right));
  assert (left && right);

  size = AOP_SIZE (right) - 1;
  mask = (0x100UL << (size * 8)) - 1;
  // in the end CARRY holds "left < right" (performedLt == 1) or "left >= right" (performedLt == 0)
  performedLt = 1;
  templbl = NULL;
  lit = 0;

  resolveIfx (&rIfx, ifx);

  /**********************************************************************
   * handle bits - bit compares are promoted to int compares seemingly! *
   **********************************************************************/
#if 0
  // THIS IS COMPLETELY UNTESTED!
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      pCodeOp *pcleft = pic16_popGet (AOP (left), 0);
      pCodeOp *pcright = pic16_popGet (AOP (right), 0);
      assert (pcleft->type == PO_GPR_BIT && pcright->type == PO_GPR_BIT);

      emitSETC;
      // 1 < {0,1} is false --> clear C by skipping the next instruction
      //pic16_emitpcode (POC_BTFSS, pic16_popCopyGPR2Bit (AOP (left),0), PCORB (pcleft)->bit);
      pic16_emitpcode (POC_BTFSS, pic16_popGet (AOP (left), 0));
      // {0,1} < 0 is false --> clear C by NOT skipping the next instruction
      pic16_emitpcode (POC_BTFSS, pic16_popCopyGPR2Bit (pic16_popGet (AOP (right), 0), PCORB (pcright)->bit));
      emitCLRC;                 // only skipped for left=0 && right=1

      goto correct_result_in_carry;
    }                           // if
#endif

  /*************************************************
   * make sure that left is register (or the like) *
   *************************************************/
  if (!isAOP_REGlike (left))
    {
      DEBUGpc ("swapping arguments (AOP_TYPEs %d/%d)", AOP_TYPE (left), AOP_TYPE (right));
      assert (isAOP_LIT (left));
      assert (isAOP_REGlike (right));
      // swap left and right
      // left < right <==> right > left <==> (right >= left + 1)
      lit = ulFromVal (AOP (left)->aopu.aop_lit);

      if ((!sign && (lit & mask) == mask) || (sign && (lit & mask) == (mask >> 1)))
        {
          // MAXVALUE < right? always false
          if (performedLt)
            emitCLRC;
          else
            emitSETC;
          goto correct_result_in_carry;
        }                       // if

      // This fails for lit = 0xFF (unsigned) AND lit = 0x7F (signed),
      // that's why we handled it above.
      lit++;

      dummy = left;
      left = right;
      right = dummy;

      performedLt ^= 1;         // instead of "left < right" we check for "right >= left+1, i.e. "right < left+1"
    }
  else if (isAOP_LIT (right))
    {
      lit = ulFromVal (AOP (right)->aopu.aop_lit);
    }                           // if

  assert (isAOP_REGlike (left));        // left must be register or the like
  assert (isAOP_REGlike (right) || isAOP_LIT (right));  // right may be register-like or a literal

  /*************************************************
   * special cases go here                         *
   *************************************************/

  if (isAOP_LIT (right))
    {
      if (!sign)
        {
          // unsigned comparison to a literal
          DEBUGpc ("unsigned compare: left %s lit(0x%X=%lu), size=%d", performedLt ? "<" : ">=", lit, lit, size + 1);
          if (lit == 0)
            {
              // unsigned left < 0? always false
              if (performedLt)
                emitCLRC;
              else
                emitSETC;
              goto correct_result_in_carry;
            }
        }
      else
        {
          // signed comparison to a literal
          DEBUGpc ("signed compare: left %s lit(0x%X=%ld), size=%d, mask=%x", performedLt ? "<" : ">=", lit, lit, size + 1,
                   mask);
          if ((lit & mask) == ((0x80 << (size * 8)) & mask))
            {
              // signed left < 0x80000000? always false
              if (performedLt)
                emitCLRC;
              else
                emitSETC;
              goto correct_result_in_carry;
            }
          else if (lit == 0)
            {
              // compare left < 0; set CARRY if SIGNBIT(left) is set
              if (performedLt)
                emitSETC;
              else
                emitCLRC;
              emitpcode (POC_BTFSS, newpCodeOpBit (aopGet (AOP (left), size, FALSE, FALSE), 7, 0));
              if (performedLt)
                emitCLRC;
              else
                emitSETC;
              goto correct_result_in_carry;
            }
        }                       // if (!sign)
    }                           // right is literal

  /*************************************************
   * perform a general case comparison             *
   * make sure we get CARRY==1 <==> left >= right  *
   *************************************************/
  // compare most significant bytes
  //DEBUGpc ("comparing bytes at offset %d", size);
  if (!sign)
    {
      // unsigned comparison
      pic14_mov2w_regOrLit (AOP (right), lit, size);
      emitpcode (POC_SUBFW, popGet (AOP (left), size));
    }
  else
    {
      // signed comparison
      // (add 2^n to both operands then perform an unsigned comparison)
      if (isAOP_LIT (right))
        {
          // left >= LIT <-> LIT-left <= 0 <-> LIT-left == 0 OR !(LIT-left >= 0)
          unsigned char litbyte = (lit >> (8 * size)) & 0xFF;

          if (litbyte == 0x80)
            {
              // left >= 0x80 -- always true, but more bytes to come
              mov2w (AOP (left), size);
              emitpcode (POC_XORLW, popGetLit (0x80));  // set ZERO flag
              emitSETC;
            }
          else
            {
              // left >= LIT <-> left + (-LIT) >= 0 <-> left + (0x100-LIT) >= 0x100
              mov2w (AOP (left), size);
              emitpcode (POC_ADDLW, popGetLit (0x80));
              emitpcode (POC_ADDLW, popGetLit ((0x100 - (litbyte + 0x80)) & 0x00FF));
            }                   // if
        }
      else
        {
          pCodeOp *pctemp = popGetTempReg ();
          mov2w (AOP (left), size);
          emitpcode (POC_ADDLW, popGetLit (0x80));
          emitpcode (POC_MOVWF, pctemp);
          mov2w (AOP (right), size);
          emitpcode (POC_ADDLW, popGetLit (0x80));
          emitpcode (POC_SUBFW, pctemp);
          popReleaseTempReg (pctemp);
        }
    }                           // if (!sign)

  // compare remaining bytes (treat as unsigned case from above)
  templbl = newiTempLabel (NULL);
  offs = size;
  while (offs--)
    {
      //DEBUGpc ("comparing bytes at offset %d", offs);
      emitSKPZ;

      if (pic->isEnhancedCore)
        {
          emitpcode (POC_BRA, popGetLabel (templbl->key));
        }
      else
        {
          emitpcode (POC_GOTO, popGetLabel (templbl->key));
        }

      pic14_mov2w_regOrLit (AOP (right), lit, offs);
      emitpcode (POC_SUBFW, popGet (AOP (left), offs));
    }                           // while (offs)
  emitpLabel (templbl->key);
  goto result_in_carry;

result_in_carry:

  /****************************************************
   * now CARRY contains the result of the comparison: *
   * SUBWF sets CARRY iff                             *
   * F-W >= 0 <==> F >= W <==> !(F < W)               *
   * (F=left, W=right)                                *
   ****************************************************/

  if (performedLt)
    {
      invert_result = 1;
      // value will be used in the following genSkipc ()
      rIfx.condition ^= TRUE;
    }                           // if

correct_result_in_carry:

  // assign result to variable (if neccessary), but keep CARRY intact to be used below
  if (result && AOP_TYPE (result) != AOP_CRY)
    {
      //DEBUGpc ("assign result");
      size = AOP_SIZE (result);
      while (size--)
        {
          emitpcode (POC_CLRF, popGet (AOP (result), size));
        }                       // while
      if (invert_result)
        {
          emitSKPC;
          emitpcode (POC_BSF, newpCodeOpBit (aopGet (AOP (result), 0, FALSE, FALSE), 0, 0));
        }
      else
        {
          emitpcode (POC_RLF, popGet (AOP (result), 0));
          if (ifx)
            {
              /* Result is expected to be in CARRY by genSkipc () below. */
              emitpcode (POC_RRFW, popGet (AOP (result), 0));
            }                   // if
        }                       // if
    }                           // if (result)

  // perform conditional jump
  if (ifx)
    {
      //DEBUGpc ("generate control flow");
      genSkipc (&rIfx);
      ifx->generated = TRUE;
    }                           // if
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

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
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

  /* assign the amsops */
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  genCmp (right, left, result, ifx, sign);

  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
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
  int sign;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
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

  /* assign the amsops */
  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  genCmp (left, right, result, ifx, sign);

  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genCmpEq - generates code for equal to                          */
/*-----------------------------------------------------------------*/
static void
genCmpEq (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size;
  symbol *false_label;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (ifx)
    DEBUGpic14_emitcode ("; ifx is non-null", "");
  else
    DEBUGpic14_emitcode ("; ifx is null", "");

  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  DEBUGpic14_AopType (__LINE__, left, right, result);

  /* if literal, move literal to right */
  if (op_isLitLike (IC_LEFT (ic)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  false_label = NULL;
  if (ifx && !IC_TRUE (ifx))
    {
      assert (IC_FALSE (ifx));
      false_label = IC_FALSE (ifx);
    }

  size = min (AOP_SIZE (left), AOP_SIZE (right));
  assert (!pic14_sameRegs (AOP (result), AOP (left)));
  assert (!pic14_sameRegs (AOP (result), AOP (right)));

  /* assume left != right */
  {
    int i;
    for (i = 0; i < AOP_SIZE (result); i++)
      {
        emitpcode (POC_CLRF, popGet (AOP (result), i));
      }
  }

  if (AOP_TYPE (right) == AOP_LIT)
    {
      unsigned long lit = ulFromVal (AOP (right)->aopu.aop_lit);
      int i;
      size = AOP_SIZE (left);
      assert (!op_isLitLike (left));

      switch (lit)
        {
        case 0:
          mov2w (AOP (left), 0);
          for (i = 1; i < size; i++)
            emitpcode (POC_IORFW, popGet (AOP (left), i));
          /* now Z is set iff `left == right' */
          emitSKPZ;
          if (!false_label)
            false_label = newiTempLabel (NULL);
          emitpcode (POC_GOTO, popGetLabel (false_label->key));
          break;

        default:
          for (i = 0; i < size; i++)
            {
              mov2w (AOP (left), i);
              emitpcode (POC_XORLW, popGetLit (lit >> (8 * i)));
              /* now Z is cleared if `left != right' */
              emitSKPZ;
              if (!false_label)
                false_label = newiTempLabel (NULL);
              emitpcode (POC_GOTO, popGetLabel (false_label->key));
            }                   // for i
          break;
        }                       // switch (lit)
    }
  else
    {
      /* right is no literal */
      int i;

      for (i = 0; i < size; i++)
        {
          mov2w (AOP (right), i);
          emitpcode (POC_XORFW, popGet (AOP (left), i));
          /* now Z is cleared if `left != right' */
          emitSKPZ;
          if (!false_label)
            false_label = newiTempLabel (NULL);
          emitpcode (POC_GOTO, popGetLabel (false_label->key));
        }                       // for i
    }

  /* if we reach here, left == right */

  if (AOP_SIZE (result) > 0)
    {
      emitpcode (POC_INCF, popGet (AOP (result), 0));
    }

  if (ifx && IC_TRUE (ifx))
    {
      emitpcode (POC_GOTO, popGetLabel (IC_TRUE (ifx)->key));
    }

  if (false_label && (!ifx || IC_TRUE (ifx)))
    emitpLabel (false_label->key);

  if (ifx)
    ifx->generated = TRUE;

  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genAndOp - for && operation                                     */
/*-----------------------------------------------------------------*/
static void
genAndOp (iCode * ic)
{
  operand *left, *right, *result;
  /*     symbol *tlbl; */

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* note here that && operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, FALSE);

  DEBUGpic14_AopType (__LINE__, left, right, result);

  emitpcode (POC_MOVFW, popGet (AOP (left), 0));
  emitpcode (POC_ANDFW, popGet (AOP (right), 0));
  emitpcode (POC_MOVWF, popGet (AOP (result), 0));

  /* if both are bit variables */
  /*     if (AOP_TYPE(left) == AOP_CRY && */
  /*         AOP_TYPE(right) == AOP_CRY ) { */
  /*         pic14_emitcode("mov","c,%s",AOP(left)->aopu.aop_dir); */
  /*         pic14_emitcode("anl","c,%s",AOP(right)->aopu.aop_dir); */
  /*         pic14_outBitC(result); */
  /*     } else { */
  /*         tlbl = newiTempLabel(NULL); */
  /*         pic14_toBoolean(left);     */
  /*         pic14_emitcode("jz","%05d_DS_",labelKey2num (tlbl->key)); */
  /*         pic14_toBoolean(right); */
  /*         pic14_emitcode("","%05d_DS_:",labelKey2num (tlbl->key)); */
  /*         pic14_outBitAcc(result); */
  /*     } */

  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
}


/*-----------------------------------------------------------------*/
/* genOrOp - for || operation                                      */
/*-----------------------------------------------------------------*/
/*
tsd pic port -
modified this code, but it doesn't appear to ever get called
*/

static void
genOrOp (iCode * ic)
{
  operand *left, *right, *result;
  symbol *tlbl;
  int i;

  /* note here that || operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, FALSE);

  DEBUGpic14_AopType (__LINE__, left, right, result);

  for (i = 0; i < AOP_SIZE (result); i++)
    {
      emitpcode (POC_CLRF, popGet (AOP (result), i));
    }                           // for i

  tlbl = newiTempLabel (NULL);
  pic14_toBoolean (left);
  emitSKPZ;
  emitpcode (POC_GOTO, popGetLabel (tlbl->key));
  pic14_toBoolean (right);
  emitpLabel (tlbl->key);
  /* here Z is clear IFF `left || right' */
  emitSKPZ;
  emitpcode (POC_INCF, popGet (AOP (result), 0));

  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
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

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
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
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (IC_TRUE (ic))
    {
      emitpcode (POC_GOTO, popGetLabel (labelKey2num (IC_TRUE (ic)->key)));
      pic14_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_FALSE (ic)->key));
    }
  ic->generated = TRUE;
}

/*-----------------------------------------------------------------*/
/* jmpIfTrue -                                                     */
/*-----------------------------------------------------------------*/
static void
jumpIfTrue (iCode * ic)
{
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (!IC_TRUE (ic))
    {
      emitpcode (POC_GOTO, labelKey2num (popGetLabel (IC_TRUE (ic)->key)));
      pic14_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_FALSE (ic)->key));
    }
  ic->generated = TRUE;
}

/*-----------------------------------------------------------------*/
/* jmpTrueOrFalse -                                                */
/*-----------------------------------------------------------------*/
static void
jmpTrueOrFalse (iCode * ic, symbol * tlbl)
{
  FENTRY;
  // ugly but optimized by peephole
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (IC_TRUE (ic))
    {
      symbol *nlbl = newiTempLabel (NULL);
      pic14_emitcode ("sjmp", "%05d_DS_", labelKey2num (nlbl->key));
      pic14_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
      pic14_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_TRUE (ic)->key));
      pic14_emitcode ("", "%05d_DS_:", labelKey2num (nlbl->key));
    }
  else
    {
      pic14_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_FALSE (ic)->key));
      pic14_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
    }
  ic->generated = TRUE;
}

/*-----------------------------------------------------------------*/
/* genAnd  - code for and                                          */
/*-----------------------------------------------------------------*/
static void
genAnd (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long lit = 0L;
  int bytelit = 0;
  resolvedIfx rIfx;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  resolveIfx (&rIfx, ifx);

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (pic14_sameRegs (AOP (result), AOP (right)))
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
    lit = ulFromVal (AOP (right)->aopu.aop_lit);

  size = AOP_SIZE (result);

  DEBUGpic14_AopType (__LINE__, left, right, result);

  // if(bit & yy)
  // result = bit & yy;
  if (AOP_TYPE (left) == AOP_CRY)
    {
      // c = bit & literal;
      if (AOP_TYPE (right) == AOP_LIT)
        {
          if (lit & 1)
            {
              if (size && pic14_sameRegs (AOP (result), AOP (left)))
                // no change
                goto release;
              pic14_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
            }
          else
            {
              // bit(result) = 0;
              if (size && (AOP_TYPE (result) == AOP_CRY))
                {
                  pic14_emitcode ("clr", "%s", AOP (result)->aopu.aop_dir);
                  goto release;
                }
              if ((AOP_TYPE (result) == AOP_CRY) && ifx)
                {
                  jumpIfTrue (ifx);
                  goto release;
                }
              pic14_emitcode ("clr", "c");
            }
        }
      else
        {
          if (AOP_TYPE (right) == AOP_CRY)
            {
              // c = bit & bit;
              pic14_emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
              pic14_emitcode ("anl", "c,%s", AOP (left)->aopu.aop_dir);
            }
          else
            {
              // c = bit & val;
              MOVA (aopGet (AOP (right), 0, FALSE, FALSE));
              // c = lsb
              pic14_emitcode ("rrc", "a");
              pic14_emitcode ("anl", "c,%s", AOP (left)->aopu.aop_dir);
            }
        }
      // bit = c
      // val = c
      if (size)
        pic14_outBitC (result);
      // if(bit & ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c");
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
          //MOVA(aopGet(AOP(left),posbit>>3,FALSE,FALSE));
          // bit = left & 2^n
          if (size)
            pic14_emitcode ("mov", "c,acc.%d", posbit & 0x07);
          // if(left &  2^n)
          else
            {
              if (ifx)
                {
                  int offset = 0;
                  while (posbit > 7)
                    {
                      posbit -= 8;
                      offset++;
                    }
                  emitpcode (((rIfx.condition) ? POC_BTFSC : POC_BTFSS),
                             newpCodeOpBit (aopGet (AOP (left), offset, FALSE, FALSE), posbit, 0));
                  emitpcode (POC_GOTO, popGetLabel (rIfx.lbl->key));

                  ifx->generated = TRUE;
                }
              goto release;
            }
        }
      else
        {
          symbol *tlbl = newiTempLabel (NULL);
          int sizel = AOP_SIZE (left);
          if (size)
            pic14_emitcode ("setb", "c");
          while (sizel--)
            {
              if ((bytelit = ((lit >> (offset * 8)) & 0x0FFL)) != 0x0L)
                {
                  // byte ==  2^n ?
                  if ((posbit = isLiteralBit (bytelit)) != 0)
                    {
                      emitpcode (POC_BTFSC,
                                 newpCodeOpBit (aopGet (AOP (left), offset, FALSE, FALSE), posbit - 1, 0));
                    }
                  else
                    {
                      mov2w (AOP (left), offset);
                      emitpcode (POC_ANDLW, newpCodeOpLit (bytelit & 0x0ff));
                      emitSKPZ;
                    }
                  emitpcode (POC_GOTO, popGetLabel (rIfx.condition ?
                                                    rIfx.lbl->key : tlbl->key));
                }
                offset++;
            }
          if (!rIfx.condition)
            {
              emitpcode (POC_GOTO, popGetLabel (rIfx.lbl->key));
            }
          emitpLabel(tlbl->key);
          ifx->generated = TRUE;
          // bit = left & literal
          if (size)
            {
              pic14_emitcode ("clr", "c");
              pic14_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
            }
          // if(left & literal)
          else
            {
              if (ifx)
                jmpTrueOrFalse (ifx, tlbl);
              goto release;
            }
        }
      pic14_outBitC (result);
      goto release;
    }

  /* if left is same as result */
  if (pic14_sameRegs (AOP (result), AOP (left)))
    {
      int know_W = -1;
      for (; size--; offset++, lit >>= 8)
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              switch (lit & 0xff)
                {
                case 0x00:
                  /*  and'ing with 0 has clears the result */
                  emitpcode (POC_CLRF, popGet (AOP (result), offset));
                  break;
                case 0xff:
                  /* and'ing with 0xff is a nop when the result and left are the same */
                  break;

                default:
                {
                  int p = my_powof2 ((~lit) & 0xff);
                  if (p >= 0)
                    {
                      /* only one bit is set in the literal, so use a bcf instruction */
                      emitpcode (POC_BCF, newpCodeOpBit (aopGet (AOP (left), offset, FALSE, FALSE), p, 0));

                    }
                  else
                    {
                      if (know_W != (int) (lit & 0xff))
                        emitpcode (POC_MOVLW, popGetLit (lit & 0xff));
                      know_W = lit & 0xff;
                      emitpcode (POC_ANDWF, popGet (AOP (left), offset));
                    }
                }
                }
            }
          else
            {
              emitpcode (POC_MOVFW, popGet (AOP (right), offset));
              emitpcode (POC_ANDWF, popGet (AOP (left), offset));
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
            pic14_emitcode ("setb", "c");
          while (sizer--)
            {
              MOVA (aopGet (AOP (right), offset, FALSE, FALSE));
              pic14_emitcode ("anl", "a,%s", aopGet (AOP (left), offset, FALSE, FALSE));
              pic14_emitcode ("jnz", "%05d_DS_", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              pic14_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
              pic14_outBitC (result);
            }
          else if (ifx)
            jmpTrueOrFalse (ifx, tlbl);
        }
      else
        {
          for (; (size--); offset++)
            {
              // normal case
              // result = left & right
              if (AOP_TYPE (right) == AOP_LIT)
                {
                  int t = (lit >> (offset * 8)) & 0x0FFL;
                  switch (t)
                    {
                    case 0x00:
                      emitpcode (POC_CLRF, popGet (AOP (result), offset));
                      break;
                    case 0xff:
                      emitpcode (POC_MOVFW, popGet (AOP (left), offset));
                      emitpcode (POC_MOVWF, popGet (AOP (result), offset));
                      break;
                    default:
                      emitpcode (POC_MOVLW, popGetLit (t));
                      emitpcode (POC_ANDFW, popGet (AOP (left), offset));
                      emitpcode (POC_MOVWF, popGet (AOP (result), offset));
                    }
                  continue;
                }

              emitpcode (POC_MOVFW, popGet (AOP (right), offset));
              emitpcode (POC_ANDFW, popGet (AOP (left), offset));
              emitpcode (POC_MOVWF, popGet (AOP (result), offset));
            }
        }
    }

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
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

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  DEBUGpic14_AopType (__LINE__, left, right, result);

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (pic14_sameRegs (AOP (result), AOP (right)))
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

  DEBUGpic14_AopType (__LINE__, left, right, result);

  if (AOP_TYPE (right) == AOP_LIT)
    lit = ulFromVal (AOP (right)->aopu.aop_lit);

  size = AOP_SIZE (result);

  // if(bit | yy)
  // xx = bit | yy;
  if (AOP_TYPE (left) == AOP_CRY)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          // c = bit & literal;
          if (lit)
            {
              // lit != 0 => result = 1
              if (AOP_TYPE (result) == AOP_CRY)
                {
                  if (size)
                    emitpcode (POC_BSF, popGet (AOP (result), 0));
                  //pic14_emitcode("bsf","(%s >> 3), (%s & 7)",
                  //   AOP(result)->aopu.aop_dir,
                  //   AOP(result)->aopu.aop_dir);
                  else if (ifx)
                    continueIfTrue (ifx);
                  goto release;
                }
            }
          else
            {
              // lit == 0 => result = left
              if (size && pic14_sameRegs (AOP (result), AOP (left)))
                goto release;
              pic14_emitcode (";XXX mov", "c,%s  %s,%d", AOP (left)->aopu.aop_dir, __FILE__, __LINE__);
            }
        }
      else
        {
          if (AOP_TYPE (right) == AOP_CRY)
            {
              if (pic14_sameRegs (AOP (result), AOP (left)))
                {
                  // c = bit | bit;
                  emitpcode (POC_BCF, popGet (AOP (result), 0));
                  emitpcode (POC_BTFSC, popGet (AOP (right), 0));
                  emitpcode (POC_BSF, popGet (AOP (result), 0));

                  pic14_emitcode ("bcf", "(%s >> 3), (%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
                  pic14_emitcode ("btfsc", "(%s >> 3), (%s & 7)", AOP (right)->aopu.aop_dir, AOP (right)->aopu.aop_dir);
                  pic14_emitcode ("bsf", "(%s >> 3), (%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
                }
              else
                {
                  emitpcode (POC_BCF, popGet (AOP (result), 0));
                  emitpcode (POC_BTFSS, popGet (AOP (right), 0));
                  emitpcode (POC_BTFSC, popGet (AOP (left), 0));
                  emitpcode (POC_BSF, popGet (AOP (result), 0));
                }
            }
          else
            {
              // c = bit | val;
              symbol *tlbl = newiTempLabel (NULL);
              pic14_emitcode (";XXX ", " %s,%d", __FILE__, __LINE__);


              emitpcode (POC_BCF, popGet (AOP (result), 0));

              if (!((AOP_TYPE (result) == AOP_CRY) && ifx))
                pic14_emitcode (";XXX setb", "c");
              pic14_emitcode (";XXX jb", "%s,%05d_DS_", AOP (left)->aopu.aop_dir, labelKey2num (tlbl->key));
              pic14_toBoolean (right);
              pic14_emitcode (";XXX jnz", "%05d_DS_", labelKey2num (tlbl->key));
              if ((AOP_TYPE (result) == AOP_CRY) && ifx)
                {
                  jmpTrueOrFalse (ifx, tlbl);
                  goto release;
                }
              else
                {
                  CLRC;
                  pic14_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
                }
            }
        }
      // bit = c
      // val = c
      if (size)
        pic14_outBitC (result);
      // if(bit | ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c");
      goto release;
    }

  // if(val | 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val | 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      if (lit)
        {
          pic14_emitcode (";XXX ", " %s,%d", __FILE__, __LINE__);
          // result = 1
          if (size)
            pic14_emitcode (";XXX setb", "%s", AOP (result)->aopu.aop_dir);
          else
            continueIfTrue (ifx);
          goto release;
        }
      else
        {
          pic14_emitcode (";XXX ", " %s,%d", __FILE__, __LINE__);
          // lit = 0, result = boolean(left)
          if (size)
            pic14_emitcode (";XXX setb", "c");
          pic14_toBoolean (right);
          if (size)
            {
              symbol *tlbl = newiTempLabel (NULL);
              pic14_emitcode (";XXX jnz", "%05d_DS_", labelKey2num (tlbl->key));
              CLRC;
              pic14_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
            }
          else
            {
              genIfxJump (ifx, "a");
              goto release;
            }
        }
      pic14_outBitC (result);
      goto release;
    }

  /* if left is same as result */
  if (pic14_sameRegs (AOP (result), AOP (left)))
    {
      int know_W = -1;
      for (; size--; offset++, lit >>= 8)
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              if ((lit & 0xff) == 0)
                /*  or'ing with 0 has no effect */
                continue;
              else
                {
                  int p = my_powof2 (lit & 0xff);
                  if (p >= 0)
                    {
                      /* only one bit is set in the literal, so use a bsf instruction */
                      emitpcode (POC_BSF, newpCodeOpBit (aopGet (AOP (left), offset, FALSE, FALSE), p, 0));
                    }
                  else
                    {
                      if (know_W != (int) (lit & 0xff))
                        emitpcode (POC_MOVLW, popGetLit (lit & 0xff));
                      know_W = lit & 0xff;
                      emitpcode (POC_IORWF, popGet (AOP (left), offset));
                    }

                }
            }
          else
            {
              emitpcode (POC_MOVFW, popGet (AOP (right), offset));
              emitpcode (POC_IORWF, popGet (AOP (left), offset));
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
          pic14_emitcode (";XXX ", " %s,%d", __FILE__, __LINE__);


          if (size)
            pic14_emitcode (";XXX setb", "c");
          while (sizer--)
            {
              MOVA (aopGet (AOP (right), offset, FALSE, FALSE));
              pic14_emitcode (";XXX orl", "a,%s", aopGet (AOP (left), offset, FALSE, FALSE));
              pic14_emitcode (";XXX jnz", "%05d_DS_", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              pic14_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
              pic14_outBitC (result);
            }
          else if (ifx)
            jmpTrueOrFalse (ifx, tlbl);
        }
      else
        for (; (size--); offset++)
          {
            // normal case
            // result = left | right
            if (AOP_TYPE (right) == AOP_LIT)
              {
                int t = (lit >> (offset * 8)) & 0x0FFL;
                switch (t)
                  {
                  case 0x00:
                    emitpcode (POC_MOVFW, popGet (AOP (left), offset));
                    emitpcode (POC_MOVWF, popGet (AOP (result), offset));

                    break;
                  default:
                    emitpcode (POC_MOVLW, popGetLit (t));
                    emitpcode (POC_IORFW, popGet (AOP (left), offset));
                    emitpcode (POC_MOVWF, popGet (AOP (result), offset));
                  }
                continue;
              }

            // faster than result <- left, anl result,right
            // and better if result is SFR
            emitpcode (POC_MOVFW, popGet (AOP (right), offset));
            emitpcode (POC_IORFW, popGet (AOP (left), offset));
            emitpcode (POC_MOVWF, popGet (AOP (result), offset));
          }
    }

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
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

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  /* if left is a literal & right is not ||
     if left needs acc & right does not */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || (AOP_NEEDSACC (left) && !AOP_NEEDSACC (right)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (pic14_sameRegs (AOP (result), AOP (right)))
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
    lit = ulFromVal (AOP (right)->aopu.aop_lit);

  size = AOP_SIZE (result);

  // if(bit ^ yy)
  // xx = bit ^ yy;
  if (AOP_TYPE (left) == AOP_CRY)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          // c = bit & literal;
          if (lit >> 1)
            {
              // lit>>1  != 0 => result = 1
              if (AOP_TYPE (result) == AOP_CRY)
                {
                  if (size)
                    {
                      emitpcode (POC_BSF, popGet (AOP (result), offset));
                      pic14_emitcode ("setb", "%s", AOP (result)->aopu.aop_dir);
                    }
                  else if (ifx)
                    continueIfTrue (ifx);
                  goto release;
                }
              pic14_emitcode ("setb", "c");
            }
          else
            {
              // lit == (0 or 1)
              if (lit == 0)
                {
                  // lit == 0, result = left
                  if (size && pic14_sameRegs (AOP (result), AOP (left)))
                    goto release;
                  pic14_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                }
              else
                {
                  // lit == 1, result = not(left)
                  if (size && pic14_sameRegs (AOP (result), AOP (left)))
                    {
                      emitpcode (POC_MOVLW, popGet (AOP (result), offset));
                      emitpcode (POC_XORWF, popGet (AOP (result), offset));
                      pic14_emitcode ("cpl", "%s", AOP (result)->aopu.aop_dir);
                      goto release;
                    }
                  else
                    {
                      assert (!"incomplete genXor");
                      pic14_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                      pic14_emitcode ("cpl", "c");
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
              pic14_emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
            }
          else
            {
              int sizer = AOP_SIZE (right);
              // c = bit ^ val
              // if val>>1 != 0, result = 1
              pic14_emitcode ("setb", "c");
              while (sizer)
                {
                  MOVA (aopGet (AOP (right), sizer - 1, FALSE, FALSE));
                  if (sizer == 1)
                    // test the msb of the lsb
                    pic14_emitcode ("anl", "a,#0xfe");
                  pic14_emitcode ("jnz", "%05d_DS_", labelKey2num (tlbl->key));
                  sizer--;
                }
              // val = (0,1)
              pic14_emitcode ("rrc", "a");
            }
          pic14_emitcode ("jnb", "%s,%05d_DS_", AOP (left)->aopu.aop_dir, (labelKey2num (tlbl->key)));
          pic14_emitcode ("cpl", "c");
          pic14_emitcode ("", "%05d_DS_:", (labelKey2num (tlbl->key)));
        }
      // bit = c
      // val = c
      if (size)
        pic14_outBitC (result);
      // if(bit | ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c");
      goto release;
    }

  if (pic14_sameRegs (AOP (result), AOP (left)))
    {
      /* if left is same as result */
      for (; size--; offset++)
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              int t = (lit >> (offset * 8)) & 0x0FFL;
              if (t == 0x00L)
                continue;
              else
                {
                  emitpcode (POC_MOVLW, popGetLit (t));
                  emitpcode (POC_XORWF, popGet (AOP (left), offset));
                }
            }
          else
            {
              emitpcode (POC_MOVFW, popGet (AOP (right), offset));
              emitpcode (POC_XORWF, popGet (AOP (left), offset));
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
            pic14_emitcode ("setb", "c");
          while (sizer--)
            {
              if ((AOP_TYPE (right) == AOP_LIT) && (((lit >> (offset * 8)) & 0x0FFL) == 0x00L))
                {
                  MOVA (aopGet (AOP (left), offset, FALSE, FALSE));
                }
              else
                {
                  MOVA (aopGet (AOP (right), offset, FALSE, FALSE));
                  pic14_emitcode ("xrl", "a,%s", aopGet (AOP (left), offset, FALSE, FALSE));
                }
              pic14_emitcode ("jnz", "%05d_DS_", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              pic14_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
              pic14_outBitC (result);
            }
          else if (ifx)
            jmpTrueOrFalse (ifx, tlbl);
        }
      else
        for (; (size--); offset++)
          {
            // normal case
            // result = left & right
            if (AOP_TYPE (right) == AOP_LIT)
              {
                int t = (lit >> (offset * 8)) & 0x0FFL;
                switch (t)
                  {
                  case 0x00:
                    emitpcode (POC_MOVFW, popGet (AOP (left), offset));
                    emitpcode (POC_MOVWF, popGet (AOP (result), offset));
                    break;
                  case 0xff:
                    emitpcode (POC_COMFW, popGet (AOP (left), offset));
                    emitpcode (POC_MOVWF, popGet (AOP (result), offset));
                    break;
                  default:
                    emitpcode (POC_MOVLW, popGetLit (t));
                    emitpcode (POC_XORFW, popGet (AOP (left), offset));
                    emitpcode (POC_MOVWF, popGet (AOP (result), offset));
                  }
                continue;
              }

            // faster than result <- left, anl result,right
            // and better if result is SFR
            emitpcode (POC_MOVFW, popGet (AOP (right), offset));
            emitpcode (POC_XORFW, popGet (AOP (left), offset));
            emitpcode (POC_MOVWF, popGet (AOP (result), offset));
          }
    }

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genInline - write the inline code out                           */
/*-----------------------------------------------------------------*/
static void
pic14_genInline (iCode * ic)
{
  char *buffer, *bp, *bp1;
  bool inComment = FALSE;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  genLine.lineElement.isInline += (!options.asmpeep);

  buffer = bp = bp1 = Safe_strdup (IC_INLINE (ic));

  while (*bp)
    {
      switch (*bp)
        {
        case ';':
          inComment = TRUE;
          ++bp;
          break;

        case '\x87':
        case '\n':
          inComment = FALSE;
          *bp++ = '\0';
          if (*bp1)
            addpCode2pBlock (pb, newpCodeAsmDir (bp1, NULL));   // inline directly, no process
          bp1 = bp;
          break;

        default:
          /* Add \n for labels, not dirs such as c:\mydir */
          if (!inComment && (*bp == ':') && (isspace ((unsigned char) bp[1])))
            {
              ++bp;
              *bp = '\0';
              ++bp;
              /* print label, use this special format with NULL directive
               * to denote that the argument should not be indented with tab */
              addpCode2pBlock (pb, newpCodeAsmDir (NULL, bp1)); // inline directly, no process
              bp1 = bp;
            }
          else
            ++bp;
          break;
        }
    }
  if ((bp1 != bp) && *bp1)
    addpCode2pBlock (pb, newpCodeAsmDir (bp1, NULL));   // inline directly, no process

  Safe_free (buffer);

  /* consumed; we can free it here */
  dbuf_free (IC_INLINE (ic));

  genLine.lineElement.isInline -= (!options.asmpeep);
}

/*-----------------------------------------------------------------*/
/* genRRC - rotate right with carry                                */
/*-----------------------------------------------------------------*/
static void
genRRC (iCode * ic)
{
  operand *left, *result;
  int size, offset = 0, same;

  FENTRY;
  /* rotate right with carry */
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  DEBUGpic14_AopType (__LINE__, left, NULL, result);

  same = pic14_sameRegs (AOP (result), AOP (left));

  size = AOP_SIZE (result);

  /* get the lsb and put it into the carry */
  emitpcode (POC_RRFW, popGet (AOP (left), size - 1));

  offset = 0;

  while (size--)
    {
      if (same)
        {
          emitpcode (POC_RRF, popGet (AOP (left), offset));
        }
      else
        {
          emitpcode (POC_RRFW, popGet (AOP (left), offset));
          emitpcode (POC_MOVWF, popGet (AOP (result), offset));
        }

      offset++;
    }

  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genRLC - generate code for rotate left with carry               */
/*-----------------------------------------------------------------*/
static void
genRLC (iCode * ic)
{
  operand *left, *result;
  int size, offset = 0;
  int same;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* rotate right with carry */
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  DEBUGpic14_AopType (__LINE__, left, NULL, result);

  same = pic14_sameRegs (AOP (result), AOP (left));

  /* move it to the result */
  size = AOP_SIZE (result);

  /* get the msb and put it into the carry */
  emitpcode (POC_RLFW, popGet (AOP (left), size - 1));

  offset = 0;

  while (size--)
    {
      if (same)
        {
          emitpcode (POC_RLF, popGet (AOP (left), offset));
        }
      else
        {
          emitpcode (POC_RLFW, popGet (AOP (left), offset));
          emitpcode (POC_MOVWF, popGet (AOP (result), offset));
        }

      offset++;
    }


  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

static void
genGetABit (iCode * ic)
{
  operand *left, *right, *result;
  int shCount;
  int offset;
  int i;

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);

  aopOp (left, ic, FALSE);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  shCount = (int) ulFromVal (AOP (right)->aopu.aop_lit);
  offset = shCount / 8;
  shCount %= 8;

  /* load and mask the source byte */
  mov2w (AOP (left), offset);
  emitpcode (POC_ANDLW, popGetLit (1 << shCount));

  /* move selected bit to bit 0 */
  switch (shCount)
    {
    case 0:
      /* nothing more to do */
      break;
    default:
      /* keep W==0, force W=0x01 otherwise */
      emitSKPZ;
      emitpcode (POC_MOVLW, popGetLit (1));
      break;
    }                           // switch

  /* write result */
  emitpcode (POC_MOVWF, popGet (AOP (result), 0));

  for (i = 1; i < AOP_SIZE (result); ++i)
    {
      emitpcode (POC_CLRF, popGet (AOP (result), i));
    }                           // for

  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGetHbit - generates code get highest order bit               */
/*-----------------------------------------------------------------*/
static void
genGetHbit (iCode * ic)
{
  operand *left, *result;
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* get the highest order byte into a */
  MOVA (aopGet (AOP (left), AOP_SIZE (left) - 1, FALSE, FALSE));
  if (AOP_TYPE (result) == AOP_CRY)
    {
      pic14_emitcode ("rlc", "a");
      pic14_outBitC (result);
    }
  else
    {
      pic14_emitcode ("rl", "a");
      pic14_emitcode ("anl", "a,#0x01");
      pic14_outAcc (result);
    }


  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* AccLsh - shift left accumulator by known count                  */
/* MARK: pic14 always rotates through CARRY!                       */
/*-----------------------------------------------------------------*/
static void
AccLsh (pCodeOp * pcop, int shCount)
{
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  shCount &= 0x0007;            // shCount : 0..7
  switch (shCount)
    {
    case 0:
      return;
      break;
    case 1:
      emitCLRC;
      emitpcode (POC_RLF, pcop);
      return;
      break;
    case 2:
      emitpcode (POC_RLF, pcop);
      emitpcode (POC_RLF, pcop);
      break;
    case 3:
      emitpcode (POC_RLF, pcop);
      emitpcode (POC_RLF, pcop);
      emitpcode (POC_RLF, pcop);
      break;
    case 4:
      emitpcode (POC_SWAPF, pcop);
      break;
    case 5:
      emitpcode (POC_SWAPF, pcop);
      emitpcode (POC_RLF, pcop);
      break;
    case 6:
      emitpcode (POC_SWAPF, pcop);
      emitpcode (POC_RLF, pcop);
      emitpcode (POC_RLF, pcop);
      break;
    case 7:
      emitpcode (POC_RRFW, pcop);
      emitpcode (POC_RRF, pcop);
      break;
    }
  /* clear invalid bits */
  emitpcode (POC_MOVLW, popGetLit ((unsigned char) (~((1UL << shCount) - 1))));
  emitpcode (POC_ANDWF, pcop);
}

/*-----------------------------------------------------------------*/
/* AccRsh - shift right accumulator by known count                 */
/* MARK: pic14 always rotates through CARRY!                       */
/* maskmode - 0: leave invalid bits undefined (caller should mask) */
/*            1: mask out invalid bits (zero-extend)               */
/*            2: sign-extend result (pretty slow)                  */
/*-----------------------------------------------------------------*/
static void
AccRsh (pCodeOp * pcop, int shCount, int mask_mode)
{
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  shCount &= 0x0007;            // shCount : 0..7
  switch (shCount)
    {
    case 0:
      return;
      break;
    case 1:
      /* load sign if needed */
      if (mask_mode == 2)
        emitpcode (POC_RLFW, pcop);
      else if (mask_mode == 1)
        emitCLRC;
      emitpcode (POC_RRF, pcop);
      return;
      break;
    case 2:
      /* load sign if needed */
      if (mask_mode == 2)
        emitpcode (POC_RLFW, pcop);
      emitpcode (POC_RRF, pcop);
      /* load sign if needed */
      if (mask_mode == 2)
        emitpcode (POC_RLFW, pcop);
      emitpcode (POC_RRF, pcop);
      if (mask_mode == 2)
        return;
      break;
    case 3:
      /* load sign if needed */
      if (mask_mode == 2)
        emitpcode (POC_RLFW, pcop);
      emitpcode (POC_RRF, pcop);
      /* load sign if needed */
      if (mask_mode == 2)
        emitpcode (POC_RLFW, pcop);
      emitpcode (POC_RRF, pcop);
      /* load sign if needed */
      if (mask_mode == 2)
        emitpcode (POC_RLFW, pcop);
      emitpcode (POC_RRF, pcop);
      if (mask_mode == 2)
        return;
      break;
    case 4:
      emitpcode (POC_SWAPF, pcop);
      break;
    case 5:
      emitpcode (POC_SWAPF, pcop);
      emitpcode (POC_RRF, pcop);
      break;
    case 6:
      emitpcode (POC_SWAPF, pcop);
      emitpcode (POC_RRF, pcop);
      emitpcode (POC_RRF, pcop);
      break;
    case 7:
      if (mask_mode == 2)
        {
          /* load sign */
          emitpcode (POC_RLFW, pcop);
          emitpcode (POC_CLRF, pcop);
          emitSKPNC;
          emitpcode (POC_COMF, pcop);
          return;
        }
      else
        {
          emitpcode (POC_RLFW, pcop);
          emitpcode (POC_RLF, pcop);
        }
      break;
    }

  if (mask_mode == 0)
    {
      /* leave invalid bits undefined */
      return;
    }

  /* clear invalid bits -- zero-extend */
  emitpcode (POC_MOVLW, popGetLit (0x00ff >> shCount));
  emitpcode (POC_ANDWF, pcop);

  if (mask_mode == 2)
    {
      /* sign-extend */
      emitpcode (POC_MOVLW, popGetLit (0x00ff << (8 - shCount)));
      emitpcode (POC_BTFSC, newpCodeOpBit (get_op (pcop, NULL, 0), 7 - shCount, 0));
      emitpcode (POC_IORWF, pcop);
    }
}

/*-----------------------------------------------------------------*/
/* movLeft2Result - move byte from left to result                  */
/*-----------------------------------------------------------------*/
static void
movLeft2Result (operand * left, int offl, operand * result, int offr)
{
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (!pic14_sameRegs (AOP (left), AOP (result)) || (offl != offr))
    {
      aopGet (AOP (left), offl, FALSE, FALSE);

      emitpcode (POC_MOVFW, popGet (AOP (left), offl));
      emitpcode (POC_MOVWF, popGet (AOP (result), offr));
    }
}

/*-----------------------------------------------------------------*/
/* shiftLeft_Left2ResultLit - shift left by known count            */
/*-----------------------------------------------------------------*/

static void
shiftLeft_Left2ResultLit (operand * left, operand * result, int shCount)
{
  int size, same, offr, i;

  size = AOP_SIZE (left);
  if (AOP_SIZE (result) < size)
    size = AOP_SIZE (result);

  same = pic14_sameRegs (AOP (left), AOP (result));

  offr = shCount / 8;
  shCount = shCount & 0x07;

  size -= offr;

  switch (shCount)
    {
    case 0:                    /* takes 0 or 2N cycles (for offr==0) */
      if (!same || offr)
        {
          for (i = size - 1; i >= 0; i--)
            movLeft2Result (left, i, result, offr + i);
        }                       // if
      break;

    case 1:                    /* takes 1N+1 or 2N+1 cycles (or offr==0) */
      if (same && offr)
        {
          shiftLeft_Left2ResultLit (left, result, 8 * offr);
          shiftLeft_Left2ResultLit (result, result, shCount);
          return;               /* prevent clearing result again */
        }
      else
        {
          if (pic->isEnhancedCore)
            {
              for (i = 0; i < size; i++)
                {
                  if (same && !offr)
                    {
                      if (i == 0)
                        {
                          emitpcode (POC_LSLF, popGet (AOP (left), i));
                        }
                      else
                        {
                          emitpcode (POC_RLF, popGet (AOP (left), i));
                        }
                    }
                  else
                    {
                      if (i == 0)
                        {
                          emitpcode (POC_LSLFW, popGet (AOP (left), i));
                        }
                      else
                        {
                          emitpcode (POC_RLFW, popGet (AOP (left), i));
                        }

                      emitpcode (POC_MOVWF, popGet (AOP (result), i + offr));
                    }
                }
            }
          else
            {
              emitCLRC;
              for (i = 0; i < size; i++)
                {
                  if (same && !offr)
                    {
                      emitpcode (POC_RLF, popGet (AOP (left), i));
                    }
                  else
                    {
                      emitpcode (POC_RLFW, popGet (AOP (left), i));
                      emitpcode (POC_MOVWF, popGet (AOP (result), i + offr));
                    }           // if
                }               // for
            }                   // if (pic->isEnhancedCore)
        }                       // if (same && offr)
      break;

    case 4:                    /* takes 3+5(N-1) = 5N-2 cycles (for offr==0) */
      /* works in-place/with offr as well */
      emitpcode (POC_SWAPFW, popGet (AOP (left), size - 1));
      emitpcode (POC_ANDLW, popGetLit (0xF0));
      emitpcode (POC_MOVWF, popGet (AOP (result), size - 1 + offr));

      for (i = size - 2; i >= 0; i--)
        {
          emitpcode (POC_SWAPFW, popGet (AOP (left), i));
          emitpcode (POC_MOVWF, popGet (AOP (result), i + offr));
          emitpcode (POC_ANDLW, popGetLit (0x0F));
          emitpcode (POC_IORWF, popGet (AOP (result), i + offr + 1));
          emitpcode (POC_XORWF, popGet (AOP (result), i + offr));
        }                       // for i
      break;

    case 7:                    /* takes 2(N-1)+3 = 2N+1 cycles */
      /* works in-place/with offr as well */
      emitpcode (POC_RRFW, popGet (AOP (left), size - 1));
      for (i = size - 2; i >= 0; i--)
        {
          emitpcode (POC_RRFW, popGet (AOP (left), i));
          emitpcode (POC_MOVWF, popGet (AOP (result), offr + i + 1));
        }                       // for i
      emitpcode (POC_CLRF, popGet (AOP (result), offr));
      emitpcode (POC_RRF, popGet (AOP (result), offr));
      break;

    default:
      shiftLeft_Left2ResultLit (left, result, offr * 8 + shCount - 1);
      shiftLeft_Left2ResultLit (result, result, 1);
      return;                   /* prevent clearing result again */
      break;
    }                           // switch

  while (0 < offr--)
    {
      emitpcode (POC_CLRF, popGet (AOP (result), offr));
    }                           // while
}

/*-----------------------------------------------------------------*/
/* shiftRight_Left2ResultLit - shift right by known count          */
/*-----------------------------------------------------------------*/

static void
shiftRight_Left2ResultLit (operand * left, operand * result, int shCount, int sign)
{
  int size, same, offr, i;

  size = AOP_SIZE (left);
  if (AOP_SIZE (result) < size)
    size = AOP_SIZE (result);

  same = pic14_sameRegs (AOP (left), AOP (result));

  offr = shCount / 8;
  shCount = shCount & 0x07;

  size -= offr;

  if (size)
    {
      switch (shCount)
        {
        case 0:                /* takes 0 or 2N cycles (for offr==0) */
          if (!same || offr)
            {
              for (i = 0; i < size; i++)
                movLeft2Result (left, i + offr, result, i);
            }                   // if
          break;

        case 1:                /* takes 1N+1(3) or 2N+1(3) cycles (or offr==0) */
          emitpComment ("%s:%d: shCount=%d, size=%d, sign=%d, same=%d, offr=%d", __FUNCTION__, __LINE__, shCount, size, sign,
                        same, offr);
          if (same && offr)
            {
              shiftRight_Left2ResultLit (left, result, 8 * offr, sign);
              shiftRight_Left2ResultLit (result, result, shCount, sign);
              return;           /* prevent sign-extending result again */
            }
          else
            {
              if (pic->isEnhancedCore)
                {
                  for (i = size - 1; i >= 0; i--)
                    {
                      if (same && !offr)
                        {
                          if (i == (size - 1))
                            {
                              if (sign)
                                {
                                  emitpcode (POC_ASRF, popGet (AOP (left), i));
                                }
                              else
                                {
                                  emitpcode (POC_LSRF, popGet (AOP (left), i));
                                }
                            }
                          else
                            {
                              emitpcode (POC_RRF, popGet (AOP (left), i));
                            }
                        }
                      else
                        {
                          if (i == (size - 1))
                            {
                              if (sign)
                                {
                                  emitpcode (POC_ASRFW, popGet (AOP (left), i));
                                }
                              else
                                {
                                  emitpcode (POC_LSRFW, popGet (AOP (left), i));
                                }
                            }
                          else
                            {
                              emitpcode (POC_RRFW, popGet (AOP (left), i + offr));
                            }

                          emitpcode (POC_MOVWF, popGet (AOP (result), i));
                        }       // if (same && !offr)
                    }           // for i
                }
              else
                {
                  emitCLRC;
                  if (sign)
                    {
                      emitpcode (POC_BTFSC, newpCodeOpBit (aopGet (AOP (left), AOP_SIZE (left) - 1, FALSE, FALSE), 7, 0));
                      emitSETC;
                    }

                  for (i = size - 1; i >= 0; i--)
                    {
                      if (same && !offr)
                        {
                          emitpcode (POC_RRF, popGet (AOP (left), i));
                        }
                      else
                        {
                          emitpcode (POC_RRFW, popGet (AOP (left), i + offr));
                          emitpcode (POC_MOVWF, popGet (AOP (result), i));
                        }
                    }           // for i
                }               // if (pic->isEnhancedCore)  
            }                   // if (same && offr)
          break;

        case 4:                /* takes 3(6)+5(N-1) = 5N-2(+1) cycles (for offr==0) */
          /* works in-place/with offr as well */
          emitpcode (POC_SWAPFW, popGet (AOP (left), offr));
          emitpcode (POC_ANDLW, popGetLit (0x0F));
          emitpcode (POC_MOVWF, popGet (AOP (result), 0));

          for (i = 1; i < size; i++)
            {
              emitpcode (POC_SWAPFW, popGet (AOP (left), i + offr));
              emitpcode (POC_MOVWF, popGet (AOP (result), i));
              emitpcode (POC_ANDLW, popGetLit (0xF0));
              emitpcode (POC_IORWF, popGet (AOP (result), i - 1));
              emitpcode (POC_XORWF, popGet (AOP (result), i));
            }                   // for i

          if (sign)
            {
              emitpcode (POC_MOVLW, popGetLit (0xF0));
              emitpcode (POC_BTFSC, newpCodeOpBit (aopGet (AOP (result), size - 1, FALSE, FALSE), 3, 0));
              emitpcode (POC_IORWF, popGet (AOP (result), size - 1));
            }                   // if
          break;

        case 7:                /* takes 2(N-1)+3(4) = 2N+1(2) cycles */
          /* works in-place/with offr as well */
          emitpcode (POC_RLFW, popGet (AOP (left), offr));
          for (i = 0; i < size - 1; i++)
            {
              emitpcode (POC_RLFW, popGet (AOP (left), offr + i + 1));
              emitpcode (POC_MOVWF, popGet (AOP (result), i));
            }                   // for i
          emitpcode (POC_CLRF, popGet (AOP (result), size - 1));
          if (!sign)
            {
              emitpcode (POC_RLF, popGet (AOP (result), size - 1));
            }
          else
            {
              emitSKPNC;
              emitpcode (POC_DECF, popGet (AOP (result), size - 1));
            }
          break;

        default:
          shiftRight_Left2ResultLit (left, result, offr * 8 + shCount - 1, sign);
          shiftRight_Left2ResultLit (result, result, 1, sign);
          return;               /* prevent sign extending result again */
          break;
        }                       // switch
    }                           // if

  addSign (result, size, sign);
}

/*-----------------------------------------------------------------*
* genMultiAsm - repeat assembly instruction for size of register.
* if endian == 1, then the high byte (i.e base address + size of
* register) is used first else the low byte is used first;
*-----------------------------------------------------------------*/
static void
genMultiAsm (PIC_OPCODE poc, operand * reg, int size, int endian)
{

  int offset = 0;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (!reg)
    return;

  if (!endian)
    {
      endian = 1;
    }
  else
    {
      endian = -1;
      offset = size - 1;
    }

  while (size--)
    {
      emitpcode (poc, popGet (AOP (reg), offset));
      offset += endian;
    }

}

/*-----------------------------------------------------------------*/
/* loadSignToC - load the operand's sign bit into CARRY            */
/*-----------------------------------------------------------------*/

static void
loadSignToC (operand * op)
{
  FENTRY;
  assert (op && AOP (op) && AOP_SIZE (op));

  emitCLRC;
  emitpcode (POC_BTFSC, newpCodeOpBit (aopGet (AOP (op), AOP_SIZE (op) - 1, FALSE, FALSE), 7, 0));
  emitSETC;
}

/*-----------------------------------------------------------------*/
/* genRightShift - generate code for right shifting                */
/*-----------------------------------------------------------------*/
static void
genGenericShift (iCode * ic, int shiftRight)
{
  operand *right, *left, *result;
  int size;
  symbol *tlbl, *tlbl1, *inverselbl;

  FENTRY;
  /* if signed then we do it the hard way preserve the
     sign bit moving it inwards */
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

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
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      int lit = (int) ulFromVal (AOP (right)->aopu.aop_lit);
      if (lit < 0)
        {
          lit = -lit;
          shiftRight = !shiftRight;
        }

      if (shiftRight)
        shiftRight_Left2ResultLit (left, result, lit, !SPEC_USIGN (operandType (left)));
      else
        shiftLeft_Left2ResultLit (left, result, lit);
      //genRightShiftLiteral (left,right,result,ic, 0);
      return;
    }

  /* shift count is unknown then we have to form
     a loop get the loop count in B : Note: we take
     only the lower order byte since shifting
     more that 32 bits make no sense anyway, ( the
     largest size of an object can be only 32 bits ) */

  /* we must not overwrite the shift counter */
  assert (!pic14_sameRegs (AOP (right), AOP (result)));

  /* now move the left to the result if they are not the
     same */
  if (!pic14_sameRegs (AOP (left), AOP (result)))
    {
      size = min (AOP_SIZE (result), AOP_SIZE (left));
      while (size--)
        {
          mov2w (AOP (left), size);
          movwf (AOP (result), size);
        }
      addSign (result, AOP_SIZE (left), !SPEC_USIGN (operandType (left)));
    }

  tlbl = newiTempLabel (NULL);
  tlbl1 = newiTempLabel (NULL);
  inverselbl = NULL;
  size = AOP_SIZE (result);

  mov2w (AOP (right), 0);
  if (!SPEC_USIGN (operandType (right)))
    {
      inverselbl = newiTempLabel (NULL);
      /* signed shift count -- invert shift direction for c<0 */
      emitpcode (POC_BTFSC, newpCodeOpBit (aopGet (AOP (right), 0, FALSE, FALSE), 7, 0));
      emitpcode (POC_GOTO, popGetLabel (inverselbl->key));
    }                           // if
  emitpcode (POC_SUBLW, popGetLit (0)); /* -count in WREG, 0-x > 0 --> BORROW = !CARRY --> CARRY is clear! */
  /* check for `a = b >> c' with `-c == 0' */
  emitSKPNZ;
  emitpcode (POC_GOTO, popGetLabel (tlbl1->key));
  emitpLabel (tlbl->key);
  /* propagate the sign bit inwards for SIGNED result */
  if (shiftRight && !SPEC_USIGN (operandType (result)))
    loadSignToC (result);
  genMultiAsm (shiftRight ? POC_RRF : POC_RLF, result, size, shiftRight);
  emitpcode (POC_ADDLW, popGetLit (1)); /* clears CARRY (unless W==0 afterwards) */
  emitSKPC;
  emitpcode (POC_GOTO, popGetLabel (tlbl->key));

  if (!SPEC_USIGN (operandType (right)))
    {
      symbol *inv_loop = newiTempLabel (NULL);

      shiftRight = !shiftRight; /* invert shift direction */

      /* we came here from the code above -- we are done */
      emitpcode (POC_GOTO, popGetLabel (tlbl1->key));

      /* emit code for shifting N<0 steps, count is already in W */
      emitpLabel (inverselbl->key);
      if (!shiftRight || SPEC_USIGN (operandType (result)))
        emitCLRC;
      emitpLabel (inv_loop->key);
      /* propagate the sign bit inwards for SIGNED result */
      if (shiftRight && !SPEC_USIGN (operandType (result)))
        loadSignToC (result);
      genMultiAsm (shiftRight ? POC_RRF : POC_RLF, result, size, shiftRight);
      emitpcode (POC_ADDLW, popGetLit (1));
      emitSKPC;
      emitpcode (POC_GOTO, popGetLabel (inv_loop->key));
    }                           // if

  emitpLabel (tlbl1->key);

  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

static void
genRightShift (iCode * ic)
{
  genGenericShift (ic, 1);
}

static void
genLeftShift (iCode * ic)
{
  genGenericShift (ic, 0);
}

/*-----------------------------------------------------------------*/
/* SetIrp - Set IRP bit                                            */
/*-----------------------------------------------------------------*/
static void
SetIrp (operand * result)
{
  FENTRY;
  if (AOP_TYPE (result) == AOP_LIT)
    {
      unsigned lit = (unsigned) double2ul (operandLitValue (result));
      if (lit & 0x100)
        emitSETIRP;
      else
        emitCLRIRP;
    }
  else if ((AOP_TYPE (result) == AOP_PCODE) && (AOP (result)->aopu.pcop->type == PO_LITERAL))
    {
      int addrs = PCOL (AOP (result)->aopu.pcop)->lit;
      if (addrs & 0x100)
        emitSETIRP;
      else
        emitCLRIRP;
    }
  else if ((AOP_TYPE (result) == AOP_PCODE) && (AOP (result)->aopu.pcop->type == PO_IMMEDIATE))
    {
      emitCLRIRP;           /* always ensure this is clear as it may have previously been set */
      emitpcode (POC_MOVLW, popGetAddr (AOP (result), 1, 0));
      emitpcode (POC_ANDLW, popGetLit (0x01));
      emitSKPZ;
      emitSETIRP;
    }
  else
    {
      emitCLRIRP;           /* always ensure this is clear as it may have previouly been set */
      if (AOP_SIZE (result) > 1)
        {
          emitpcode (POC_BTFSC, newpCodeOpBit (aopGet (AOP (result), 1, FALSE, FALSE), 0, 0));
          emitSETIRP;
        }
    }
}

static void
setup_fsr (operand * ptr)
{
  if (pic->isEnhancedCore)
    {
      mov2w_op (ptr, 0);
      emitpcode (POC_MOVWF, popCopyReg (&pc_fsr0l));
      mov2w_op (ptr, 1);
      emitpcode (POC_MOVWF, popCopyReg (&pc_fsr0h));
    }
  else
    {
      mov2w_op (ptr, 0);
      emitpcode (POC_MOVWF, popCopyReg (&pc_fsr));

      /* also setup-up IRP */
      SetIrp (ptr);
    }
}

static void
inc_fsr (int delta)
{
  if (0 == delta)
    {
      /* Nothing to do. */
      return;
    } // if

  if (pic->isEnhancedCore)
    {
      if (pic14_options.no_ext_instr)
        {
          /*
           * Not sure if we may modify W here, so implement this without
           * touching W.
           *
           * Efficiency is not too important here, as enhanced cores
           * will most likely use extended instructions here. This is
           * only a workaround for gputils 0.13.7, which supports the
           * 16f1934 enhanced core, but fails to assemble ADDFSR.
           */
          while (delta > 0)
            {
              emitpcode (POC_INCFSZ, popCopyReg (&pc_fsr0l));
              emitpcode (POC_DECF, popCopyReg (&pc_fsr0h));
              emitpcode (POC_INCF, popCopyReg (&pc_fsr0h));
              --delta;
            } // while
          while (delta < 0)
            {
              addpCode2pBlock (pb, newpCodeAsmDir("MOVF", "FSR0L, 1"));
              emitSKPNZ;
              emitpcode (POC_DECF, popCopyReg (&pc_fsr0h));
              emitpcode (POC_DECF, popCopyReg (&pc_fsr0l));
              ++delta;
            } // while
        }
      else
        {
          assert (delta >= -32);
          assert (delta < 32);
          /* Hack: Turn this into a PCI (not that easy due to the argument structure). */
          addpCode2pBlock (pb, newpCodeAsmDir ("ADDFSR", "FSR0, %d", delta));
        } // if
    }
  else
    {
      while (delta > 0)
        {
          emitpcode (POC_INCF, popCopyReg (&pc_fsr));
          --delta;
        } // while
      while (delta < 0)
        {
          emitpcode (POC_DECF, popCopyReg (&pc_fsr));
          ++delta;
        } // while
    } // if
}

/*-----------------------------------------------------------------*/
/* emitPtrByteGet - emits code to get a byte into WREG from an     */
/*                  arbitrary pointer (__code, __data, generic)    */
/*-----------------------------------------------------------------*/
static void
emitPtrByteGet (operand * src, int p_type, bool alreadyAddressed)
{
  FENTRY;
  switch (p_type)
    {
    case POINTER:
    case FPOINTER:
      if (!alreadyAddressed)
        setup_fsr (src);
      emitpcode (POC_MOVFW, popCopyReg (pc_indf));
      break;

    case CPOINTER:
      assert (AOP_SIZE (src) == 2);
      mov2w_op (src, 0);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - 1));
      mov2w_op (src, 1);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
      emitpcode (POC_MOVLW, popGetLit (GPTRTAG_CODE));  /* GPOINTER tag for __code space */
      call_libraryfunc ("__gptrget1");
      break;

    case GPOINTER:
      assert (AOP_SIZE (src) == 3);
      mov2w_op (src, 0);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - 1));
      mov2w_op (src, 1);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
      mov2w_op (src, 2);
      call_libraryfunc ("__gptrget1");
      break;

    default:
      assert (!"unhandled pointer type");
      break;
    }
}

/*-----------------------------------------------------------------*/
/* emitPtrByteSet - emits code to set a byte from src through a    */
/* pointer register INDF (legacy 8051 uses R0, R1, or DPTR).       */
/*-----------------------------------------------------------------*/
static void
emitPtrByteSet (operand * dst, int p_type, bool alreadyAddressed)
{
  FENTRY;
  switch (p_type)
    {
    case POINTER:
    case FPOINTER:
      if (!alreadyAddressed)
        setup_fsr (dst);
      emitpcode (POC_MOVWF, popCopyReg (pc_indf));
      break;

    case CPOINTER:
      assert (!"trying to assign to __code pointer");
      break;

    case GPOINTER:
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - 2));
      mov2w_op (dst, 0);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - 1));
      mov2w_op (dst, 1);
      emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
      mov2w_op (dst, 2);
      call_libraryfunc ("__gptrput1");
      break;

    default:
      assert (!"unhandled pointer type");
      break;
    }
}

/*-----------------------------------------------------------------*/
/* genUnpackBits - generates code for unpacking bits               */
/*-----------------------------------------------------------------*/
static void
genUnpackBits (operand * result, operand * left, int ptype, iCode * ifx)
{
  sym_link *etype;              /* bitfield type information */
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  etype = getSpec (operandType (result));
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  /* single bit field case */
  if (blen == 1)
    {
      if (ifx)
        {
          /* that is for an if statement */
          pCodeOp *pcop;
          resolvedIfx rIfx;

          resolveIfx (&rIfx, ifx);
          if (ptype == -1)      /* direct */
            pcop = newpCodeOpBit (aopGet (AOP (left), 0, FALSE, FALSE), bstr, 0);
          else
            {
              setup_fsr (left);
              pcop = newpCodeOpBit (pc_indf->pcop.name, bstr, 0);
            }
          emitpcode ((rIfx.condition) ? POC_BTFSC : POC_BTFSS, pcop);
          emitpcode (POC_GOTO, popGetLabel (rIfx.lbl->key));
          ifx->generated = TRUE;
        }
      else
        {
          /*
           * In case of a volatile bitfield read such as
           * (void)PORTCbits.RC3;
           * we end up having no result ...
           */
          int haveResult = !!AOP_SIZE(result);

          if (haveResult)
            {
              assert (!pic14_sameRegs (AOP (result), AOP (left)));
              emitpcode (POC_CLRF, popGet (AOP (result), 0));
            } // if

          switch (ptype)
            {
            case -1:
                emitpcode (POC_BTFSC, newpCodeOpBit (aopGet (AOP (left), 0, FALSE, FALSE), bstr, 0));
                /* If haveResult, adjust result below, otherwise: */
                if (!haveResult)
                  {
                    /* Dummy instruction to allow bit-test above (volatile dummy bitfield read). */
                    emitpcode (POC_MOVLW, popGetLit (0));
                  } // if
              break;

            case POINTER:
            case FPOINTER:
            case GPOINTER:
            case CPOINTER:
              emitPtrByteGet (left, ptype, FALSE);
              if (haveResult)
                {
                  emitpcode (POC_ANDLW, popGetLit (1UL << bstr));
                  emitSKPZ;
                  /* adjust result below */
                } // if
              break;

            default:
              assert (!"unhandled pointer type");
            }                   // switch

          /* move sign-/zero extended bit to result */
          if (haveResult)
            {
              if (SPEC_USIGN (OP_SYM_ETYPE (left)))
                emitpcode (POC_INCF, popGet (AOP (result), 0));
              else
                emitpcode (POC_DECF, popGet (AOP (result), 0));
              addSign (result, 1, !SPEC_USIGN (OP_SYM_ETYPE (left)));
            } // if
        }
      return;
    }
  else if (blen <= 8 && ((blen + bstr) <= 8))
    {
      /* blen > 1 */
      int i;

      for (i = 0; i < AOP_SIZE (result); i++)
        emitpcode (POC_CLRF, popGet (AOP (result), i));

      switch (ptype)
        {
        case -1:
          mov2w (AOP (left), 0);
          break;

        case POINTER:
        case FPOINTER:
        case GPOINTER:
        case CPOINTER:
          emitPtrByteGet (left, ptype, FALSE);
          break;

        default:
          assert (!"unhandled pointer type");
        }                       // switch

      if (blen < 8)
        emitpcode (POC_ANDLW, popGetLit ((((1UL << blen) - 1) << bstr) & 0x00ff));
      movwf (AOP (result), 0);
      AccRsh (popGet (AOP (result), 0), bstr, 1);       /* zero extend the bitfield */

      if (!SPEC_USIGN (OP_SYM_ETYPE (left)) && (bstr + blen != 8))
        {
          /* signed bitfield */
          assert (bstr + blen > 0);
          emitpcode (POC_MOVLW, popGetLit (0x00ff << (bstr + blen)));
          emitpcode (POC_BTFSC, newpCodeOpBit (aopGet (AOP (result), 0, FALSE, FALSE), bstr + blen - 1, 0));
          emitpcode (POC_IORWF, popGet (AOP (result), 0));
        }
      addSign (result, 1, !SPEC_USIGN (OP_SYM_ETYPE (left)));
      return;
    }

  assert (!"bitfields larger than 8 bits or crossing byte boundaries are not yet supported");
}

#if 1
/*-----------------------------------------------------------------*/
/* genDataPointerGet - generates code when ptr offset is known     */
/*-----------------------------------------------------------------*/
static void
genDataPointerGet (operand * left, operand * result, iCode * ic)
{
  unsigned int size;
  int offset = 0;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);


  /* optimization - most of the time, left and result are the same
   * address, but different types. for the pic code, we could omit
   * the following
   */
  aopOp (result, ic, TRUE);

  if (pic14_sameRegs (AOP (left), AOP (result)))
    return;

  DEBUGpic14_AopType (__LINE__, left, NULL, result);

  //emitpcode(POC_MOVFW, popGet(AOP(left),0));

  size = AOP_SIZE (result);
  if (size > getSize (OP_SYM_ETYPE (left)))
    size = getSize (OP_SYM_ETYPE (left));

  offset = 0;
  while (size--)
    {
      emitpcode (POC_MOVFW, popGet (AOP (left), offset));
      emitpcode (POC_MOVWF, popGet (AOP (result), offset));
      offset++;
    }

  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}
#endif

/*-----------------------------------------------------------------*/
/* genNearPointerGet - pic14_emitcode for near pointer fetch       */
/*-----------------------------------------------------------------*/
static void
genNearPointerGet (operand * left, operand * result, iCode * ic)
{
  asmop *aop = NULL;
  sym_link *ltype = operandType (left);
  sym_link *rtype = operandType (result);
  sym_link *retype = getSpec (rtype);   /* bitfield type information */
  int direct = 0;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);


  aopOp (left, ic, FALSE);

  /* if left is rematerialisable and
     result is not bit variable type and
     the left is pointer to data space i.e
     lower 128 bytes of space */
  if (AOP_TYPE (left) == AOP_PCODE &&   //AOP_TYPE(left) == AOP_IMMD &&
      !IS_BITVAR (retype) && PIC_IS_DATA_PTR (ltype))
    {
      genDataPointerGet (left, result, ic);
      return;
    }

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp (result, ic, FALSE);

  /* Check if can access directly instead of via a pointer */
  if ((AOP_TYPE (left) == AOP_PCODE) && (AOP (left)->aopu.pcop->type == PO_IMMEDIATE) && (AOP_SIZE (result) <= 1))
    {
      direct = 1;
    }

  if (IS_BITFIELD (getSpec (operandType (result))))
    {
      genUnpackBits (result, left, direct ? -1 : POINTER, ifxForOp (IC_RESULT (ic), ic));
      goto release;
    }

  /* If the pointer value is not in a the FSR then need to put it in */
  /* Must set/reset IRP bit for use with FSR. */
  if (!direct)
    setup_fsr (left);

//  sym_link *etype;
  /* if bitfield then unpack the bits */
  {
    /* we have can just get the values */
    int size = AOP_SIZE (result);
    int offset = 0;

    DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

    while (size--)
      {
        if (direct)
          emitpcode (POC_MOVWF, popGet (AOP (left), 0));
        else
          emitpcode (POC_MOVFW, popCopyReg (pc_indf));
        if (AOP_TYPE (result) == AOP_LIT)
          {
            emitpcode (POC_MOVLW, popGet (AOP (result), offset));
          }
        else
          {
            emitpcode (POC_MOVWF, popGet (AOP (result), offset));
          }
        if (size && !direct)
          {
            inc_fsr (1);
          }
        offset++;
      }
  }

  /* now some housekeeping stuff */
  if (aop)
    {
      /* we had to allocate for this iCode */
      DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      freeAsmop (NULL, aop, ic, TRUE);
    }
  else if (!direct)
    {
      /* nothing to do */
    }
  else
    {
      /* we did not allocate which means left
         already in a pointer register, then
         if size > 0 && this could be used again
         we have to point it back to where it
         belongs */
      DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      if (AOP_SIZE (result) > 1 && !OP_SYMBOL (left)->remat && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth))
        {
          int size = AOP_SIZE (result) - 1;
          inc_fsr (-size);
        }
    }

release:
  /* done */
  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);

}

/*-----------------------------------------------------------------*/
/* genGenPointerGet - gget value from generic pointer space        */
/*-----------------------------------------------------------------*/
static void
genGenPointerGet (operand * left, operand * result, iCode * ic)
{
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);


  DEBUGpic14_AopType (__LINE__, left, NULL, result);

  if (IS_BITFIELD (getSpec (operandType (result))))
    {
      genUnpackBits (result, left, GPOINTER, ifxForOp (IC_RESULT (ic), ic));
      return;
    }

  {
    /* emit call to __gptrget */
    char *func[] = { NULL, "__gptrget1", "__gptrget2", "__gptrget3", "__gptrget4" };
    int size = AOP_SIZE (result);
    int idx = 0;

    assert (size > 0 && size <= 4);

    /* pass arguments */
    assert (AOP_SIZE (left) == 3);
    mov2w (AOP (left), 0);
    emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - 1));
    mov2w (AOP (left), 1);
    emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
    mov2w (AOP (left), 2);
    call_libraryfunc (func[size]);

    /* save result */
    movwf (AOP (result), --size);
    while (size--)
      {
        emitpcode (POC_MOVFW, popRegFromIdx (Gstack_base_addr - idx++));
        movwf (AOP (result), size);
      }                         // while
  }

  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);

}

/*-----------------------------------------------------------------*/
/* genConstPointerGet - get value from const generic pointer space */
/*-----------------------------------------------------------------*/
static void
genConstPointerGet (operand * left, operand * result, iCode * ic)
{
  //sym_link *retype = getSpec(operandType(result));
#if 0
  symbol *albl, *blbl;          //, *clbl;
  pCodeOp *pcop;
#endif
  int i, lit;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp (left, ic, FALSE);
  aopOp (result, ic, FALSE);

  DEBUGpic14_AopType (__LINE__, left, NULL, result);

  DEBUGpic14_emitcode ("; ", " %d getting const pointer", __LINE__);

  lit = op_isLitLike (left);

  if (IS_BITFIELD (getSpec (operandType (result))))
    {
      genUnpackBits (result, left, lit ? -1 : CPOINTER, ifxForOp (IC_RESULT (ic), ic));
      goto release;
    }

  {
    char *func[] = { NULL, "__gptrget1", "__gptrget2", "__gptrget3", "__gptrget4" };
    int size = AOP_SIZE (result);
    assert (size > 0 && size <= 4);

    mov2w_op (left, 0);
    emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - 1));
    mov2w_op (left, 1);
    emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
    emitpcode (POC_MOVLW, popGetLit (GPTRTAG_CODE));    /* GPOINTER tag for __code space */
    call_libraryfunc (func[size]);

    movwf (AOP (result), size - 1);
    for (i = 1; i < size; i++)
      {
        emitpcode (POC_MOVFW, popRegFromIdx (Gstack_base_addr + 1 - i));
        movwf (AOP (result), size - 1 - i);
      }                         // for
  }

release:
  freeAsmop (left, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);

}

/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void
genPointerGet (iCode * ic)
{
  operand *left, *result;
  sym_link *type, *etype;
  int p_type = -1;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  /* depending on the type of pointer we need to
     move it to the correct pointer register */
  type = operandType (left);
  etype = getSpec (type);

  if (IS_PTR_CONST (type))
    DEBUGpic14_emitcode ("; ***", "%d - const pointer", __LINE__);

  /* if left is of type of pointer then it is simple */
  if (IS_PTR (type) && !IS_FUNC (type->next))
    p_type = DCL_TYPE (type);
  else
    {
      /* we have to go by the storage class */
      p_type = PTR_TYPE (SPEC_OCLS (etype));

      DEBUGpic14_emitcode ("; ***", "%d - resolve pointer by storage class", __LINE__);

      if (SPEC_OCLS (etype)->codesp)
        {
          DEBUGpic14_emitcode ("; ***", "%d - cpointer", __LINE__);
          //p_type = CPOINTER ;
        }
      else if (SPEC_OCLS (etype)->fmap && !SPEC_OCLS (etype)->paged)
        DEBUGpic14_emitcode ("; ***", "%d - fpointer", __LINE__);
      /*p_type = FPOINTER ; */
      else if (SPEC_OCLS (etype)->fmap && SPEC_OCLS (etype)->paged)
        DEBUGpic14_emitcode ("; ***", "%d - ppointer", __LINE__);
      /*        p_type = PPOINTER; */
      else if (SPEC_OCLS (etype) == idata)
        DEBUGpic14_emitcode ("; ***", "%d - ipointer", __LINE__);
      /*      p_type = IPOINTER; */
      else
        DEBUGpic14_emitcode ("; ***", "%d - pointer", __LINE__);
      /*      p_type = POINTER ; */
    }

  /* now that we have the pointer type we assign
     the pointer values */
  switch (p_type)
    {

    case POINTER:
    case FPOINTER:
      //case IPOINTER:
      genNearPointerGet (left, result, ic);
      break;
      /*
         case PPOINTER:
         genPagedPointerGet(left,result,ic);
         break;

         case FPOINTER:
         genFarPointerGet (left,result,ic);
         break;
       */
    case CPOINTER:
      genConstPointerGet (left, result, ic);
      break;

    case GPOINTER:
      genGenPointerGet (left, result, ic);
      break;
    default:
      assert (!"unhandled pointer type");
      break;
    }

}

/*-----------------------------------------------------------------*/
/* genPackBits - generates code for packed bit storage             */
/*-----------------------------------------------------------------*/
static void
genPackBits (sym_link * etype, operand * result, operand * right, int p_type)
{
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */
  int litval;                   /* source literal value (if AOP_LIT) */
  unsigned char mask;           /* bitmask within current byte */

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  /* If the bitfield length is less than a byte and does not cross byte boundaries */
  if ((blen <= 8) && ((bstr + blen) <= 8))
    {
      mask = ((unsigned char) (0xFF << (blen + bstr)) | (unsigned char) (0xFF >> (8 - bstr)));

      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with a bitfield length <8 and literal source */
          int lit = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          if (blen == 1)
            {
              pCodeOp *pcop;

              switch (p_type)
                {
                case -1:
                  if (AOP (result)->type == AOP_PCODE)
                    pcop = newpCodeOpBit (aopGet (AOP (result), 0, FALSE, FALSE), bstr, 0);
                  else
                    pcop = popGet (AOP (result), 0);
                  emitpcode (lit ? POC_BSF : POC_BCF, pcop);
                  break;

                case POINTER:
                case FPOINTER:
                  setup_fsr (result);
                  emitpcode (lit ? POC_BSF : POC_BCF, newpCodeOpBit (PCOP (pc_indf)->name, bstr, 0));
                  break;

                case CPOINTER:
                  assert (!"trying to assign to bitfield via pointer to __code space");
                  break;

                case GPOINTER:
                  emitPtrByteGet (result, p_type, FALSE);
                  if (lit)
                    {
                      emitpcode (POC_IORLW, newpCodeOpLit (1UL << bstr));
                    }
                  else
                    {
                      emitpcode (POC_ANDLW, newpCodeOpLit ((~(1UL << bstr)) & 0x0ff));
                    }
                  emitPtrByteSet (result, p_type, TRUE);
                  break;

                default:
                  assert (!"unhandled pointer type");
                  break;
                }               // switch (p_type)
            }
          else
            {
              /* blen > 1 */
              litval = lit << bstr;
              litval &= (~mask) & 0x00ff;

              switch (p_type)
                {
                case -1:
                  mov2w (AOP (result), 0);
                  if ((litval | mask) != 0x00ff)
                    emitpcode (POC_ANDLW, popGetLit (mask));
                  if (litval != 0x00)
                    emitpcode (POC_IORLW, popGetLit (litval));
                  movwf (AOP (result), 0);
                  break;

                case POINTER:
                case FPOINTER:
                case GPOINTER:
                  emitPtrByteGet (result, p_type, FALSE);
                  if ((litval | mask) != 0x00ff)
                    emitpcode (POC_ANDLW, popGetLit (mask));
                  if (litval != 0x00)
                    emitpcode (POC_IORLW, popGetLit (litval));
                  emitPtrByteSet (result, p_type, TRUE);
                  break;

                case CPOINTER:
                  assert (!"trying to assign to bitfield via pointer to __code space");
                  break;

                default:
                  assert (!"unhandled pointer type");
                  break;
                }               // switch
            }                   // if (blen > 1)
        }
      else
        {
          /* right is no literal */
          if (blen == 1)
            {
              switch (p_type)
                {
                case -1:
                  /* Note more efficient code, of pre clearing bit then only setting it if required,
                   * can only be done if it is known that the result is not a SFR */
                  emitpcode (POC_RRFW, popGet (AOP (right), 0));
                  emitSKPC;
                  emitpcode (POC_BCF, newpCodeOpBit (aopGet (AOP (result), 0, FALSE, FALSE), bstr, 0));
                  emitSKPNC;
                  emitpcode (POC_BSF, newpCodeOpBit (aopGet (AOP (result), 0, FALSE, FALSE), bstr, 0));
                  break;

                case POINTER:
                case FPOINTER:
                case GPOINTER:
                  emitPtrByteGet (result, p_type, FALSE);
                  emitpcode (POC_BTFSS, newpCodeOpBit (aopGet (AOP (right), 0, FALSE, FALSE), bstr, 0));
                  emitpcode (POC_ANDLW, newpCodeOpLit (~(1UL << bstr) & 0x0ff));
                  emitpcode (POC_BTFSC, newpCodeOpBit (aopGet (AOP (right), 0, FALSE, FALSE), bstr, 0));
                  emitpcode (POC_IORLW, newpCodeOpLit ((1UL << bstr) & 0x0ff));
                  emitPtrByteSet (result, p_type, TRUE);
                  break;

                case CPOINTER:
                  assert (!"trying to assign to bitfield via pointer to __code space");
                  break;

                default:
                  assert (!"unhandled pointer type");
                  break;
                }               // switch
              return;
            }
          else
            {
              /* Case with a bitfield 1 < length <= 8 and arbitrary source */
              pCodeOp *temp = popGetTempReg ();

              mov2w (AOP (right), 0);
              if (blen < 8)
                {
                  emitpcode (POC_ANDLW, popGetLit ((1UL << blen) - 1));
                }
              emitpcode (POC_MOVWF, temp);
              if (bstr)
                {
                  AccLsh (temp, bstr);
                }

              switch (p_type)
                {
                case -1:
                  mov2w (AOP (result), 0);
                  emitpcode (POC_ANDLW, popGetLit (mask));
                  emitpcode (POC_IORFW, temp);
                  movwf (AOP (result), 0);
                  break;

                case POINTER:
                case FPOINTER:
                case GPOINTER:
                  emitPtrByteGet (result, p_type, FALSE);
                  emitpcode (POC_ANDLW, popGetLit (mask));
                  emitpcode (POC_IORFW, temp);
                  emitPtrByteSet (result, p_type, TRUE);
                  break;

                case CPOINTER:
                  assert (!"trying to assign to bitfield via pointer to __code space");
                  break;

                default:
                  assert (!"unhandled pointer type");
                  break;
                }               // switch

              popReleaseTempReg (temp);
            }                   // if (blen > 1)
        }                       // if (AOP(right)->type != AOP_LIT)
      return;
    }                           // if (blen <= 8 && ((blen + bstr) <= 8))

  assert (!"bitfields larger than 8 bits or crossing byte boundaries are not yet supported");
}

/*-----------------------------------------------------------------*/
/* genDataPointerSet - remat pointer to data space                 */
/*-----------------------------------------------------------------*/
static void
genDataPointerSet (operand * right, operand * result, iCode * ic)
{
  int size = 0;
  int offset = 0;
  sym_link *rtype = operandType(right);

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, FALSE);

  assert (IS_SYMOP (result));
  assert (IS_PTR (OP_SYM_TYPE (result)));

  /*
   * Determine size from right operand (not result):
   * The result might be a rematerialized pointer to (the first field in) a struct,
   * which then assumes the type (and size) of the struct rather than the first field.
   */
  size = AOP_SIZE(right);

  /*test the right operand has a pointer value*/
  if ((AOP_TYPE(right) == AOP_PCODE) && PIC_IS_DATA_PTR(rtype))
  {
    while (size--)
    {
      emitpcode(POC_MOVLW, popGetAddr(AOP(right), size, 0));
      emitpcode(POC_MOVWF, popGet(AOP(result), size));
    }
  }
  else
  {
    // tsd, was l+1 - the underline `_' prefix was being stripped
    while (size--)
    {
      emitpComment ("%s:%u: size=%d, offset=%d, AOP_TYPE(res)=%d", __FILE__, __LINE__, size, offset,
                    AOP_TYPE (result));

      if (AOP_TYPE (right) == AOP_LIT)
      {
        unsigned int lit = pic14aopLiteral (AOP (IC_RIGHT (ic))->aopu.aop_lit, offset);
        //fprintf (stderr, "%s:%u: lit %d 0x%x\n", __FUNCTION__,__LINE__, lit, lit);
        if (lit & 0xff)
        {
          emitpcode (POC_MOVLW, popGetLit (lit & 0xff));
          emitpcode (POC_MOVWF, popGet (AOP (result), offset));
        }
        else
        {
          emitpcode (POC_CLRF, popGet (AOP (result), offset));
        }
      }
      else
      {
        //fprintf (stderr, "%s:%u: no lit\n", __FUNCTION__,__LINE__);
        emitpcode (POC_MOVFW, popGet (AOP (right), offset));
        emitpcode (POC_MOVWF, popGet (AOP (result), offset));
      }
      offset++;
    }
  }

  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genNearPointerSet - pic14_emitcode for near pointer put         */
/*-----------------------------------------------------------------*/
static void
genNearPointerSet (operand * right, operand * result, iCode * ic)
{
  asmop *aop = NULL;
  sym_link *ptype = operandType (result);
  sym_link *retype = getSpec (operandType (right));
  sym_link *letype = getSpec (ptype);
  int direct = 0;


  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp (result, ic, FALSE);

#if 1
  /* if the result is rematerializable &
     in data space & not a bit variable */
  //if (AOP_TYPE(result) == AOP_IMMD &&
  if (AOP_TYPE (result) == AOP_PCODE && PIC_IS_DATA_PTR (ptype) && !IS_BITVAR (retype) && !IS_BITVAR (letype))
    {
      genDataPointerSet (right, result, ic);
      freeAsmop (result, NULL, ic, TRUE);
      return;
    }
#endif

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp (right, ic, FALSE);
  DEBUGpic14_AopType (__LINE__, NULL, right, result);

  /* Check if can access directly instead of via a pointer */
  if ((AOP_TYPE (result) == AOP_PCODE) && (AOP (result)->aopu.pcop->type == PO_IMMEDIATE) && (AOP_SIZE (right) == 1))
    {
      direct = 1;
    }

  if (IS_BITFIELD (letype))
    {
      genPackBits (letype, result, right, direct ? -1 : POINTER);
      return;
    }

  /* If the pointer value is not in a the FSR then need to put it in */
  /* Must set/reset IRP bit for use with FSR. */
  /* Note only do this once - assuming that never need to cross a bank boundary at address 0x100. */
  if (!direct)
    setup_fsr (result);

  {
    /* we have can just get the values */
    int size = AOP_SIZE (right);
    int offset = 0;

    DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
    while (size--)
      {
        char *l = aopGet (AOP (right), offset, FALSE, TRUE);
        if (*l == '@')
          {
            emitpcode (POC_MOVFW, popCopyReg (pc_indf));
          }
        else
          {
            if (AOP_TYPE (right) == AOP_LIT)
              {
                emitpcode (POC_MOVLW, popGet (AOP (right), offset));
              }
            else
              {
                emitpcode (POC_MOVFW, popGet (AOP (right), offset));
              }
            if (direct)
              emitpcode (POC_MOVWF, popGet (AOP (result), 0));
            else
              emitpcode (POC_MOVWF, popCopyReg (pc_indf));
          }
        if (size && !direct)
          inc_fsr (1);
        offset++;
      }
  }

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* now some housekeeping stuff */
  if (aop)
    {
      /* we had to allocate for this iCode */
      freeAsmop (NULL, aop, ic, TRUE);
    }
  else if (!direct)
    {
      /* nothing to do */
    }
  else
    {
      /* we did not allocate which means left
         already in a pointer register, then
         if size > 0 && this could be used again
         we have to point it back to where it
         belongs */
      DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      if (AOP_SIZE (right) > 1 && !OP_SYMBOL (result)->remat && (OP_SYMBOL (result)->liveTo > ic->seq || ic->depth))
        {
          int size = AOP_SIZE (right) - 1;
          inc_fsr (-size);
        }
    }

  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* done */

  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGenPointerSet - set value from generic pointer space         */
/*-----------------------------------------------------------------*/
static void
genGenPointerSet (operand * right, operand * result, iCode * ic)
{
  sym_link *retype = getSpec (operandType (result));

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  aopOp (right, ic, FALSE);
  aopOp (result, ic, FALSE);


  DEBUGpic14_AopType (__LINE__, right, NULL, result);

  if (IS_BITFIELD (retype))
    {
      genPackBits (retype, result, right, GPOINTER);
      return;
    }

  {
    /* emit call to __gptrput */
    char *func[] = { NULL, "__gptrput1", "__gptrput2", "__gptrput3", "__gptrput4" };
    int size = AOP_SIZE (right);
    int idx = 0;

    /* The following assertion fails for
     *   struct foo { char a; char b; } bar;
     *   void demo(struct foo *dst, char c) { dst->b = c; }
     * as size will be 1 (sizeof(c)), whereas dst->b will be accessed
     * using (((char *)dst)+1), whose OP_SYM_ETYPE still is struct foo
     * of size 2.
     * The frontend seems to guarantee that IC_LEFT has the correct size,
     * it works fine both for larger and smaller types of `char c'.
     * */
    //assert (size == getSize(OP_SYM_ETYPE(result)));
    assert (size > 0 && size <= 4);

    /* pass arguments */
    /* - value (MSB in Gstack_base_addr-2, growing downwards) */
    {
      int off = size;
      idx = 2;
      while (off--)
        {
          mov2w_op (right, off);
          emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - idx++));
        }
      idx = 0;
    }
    /* - address */
    assert (AOP_SIZE (result) == 3);
    mov2w (AOP (result), 0);
    emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr - 1));
    mov2w (AOP (result), 1);
    emitpcode (POC_MOVWF, popRegFromIdx (Gstack_base_addr));
    mov2w (AOP (result), 2);
    call_libraryfunc (func[size]);
  }

  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genPointerSet - stores the value into a pointer location        */
/*-----------------------------------------------------------------*/
static void
genPointerSet (iCode * ic)
{
  operand *right, *result;
  sym_link *type, *etype;
  int p_type;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

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

      /*  if (SPEC_OCLS(etype)->codesp ) { */
      /*      p_type = CPOINTER ;  */
      /*  } */
      /*  else */
      /*      if (SPEC_OCLS(etype)->fmap && !SPEC_OCLS(etype)->paged) */
      /*    p_type = FPOINTER ; */
      /*      else */
      /*    if (SPEC_OCLS(etype)->fmap && SPEC_OCLS(etype)->paged) */
      /*        p_type = PPOINTER ; */
      /*    else */
      /*        if (SPEC_OCLS(etype) == idata ) */
      /*      p_type = IPOINTER ; */
      /*        else */
      /*      p_type = POINTER ; */
    }

  /* now that we have the pointer type we assign
     the pointer values */
  switch (p_type)
    {
    case POINTER:
    case FPOINTER:
      //case IPOINTER:
      genNearPointerSet (right, result, ic);
      break;
      /*
         case PPOINTER:
         genPagedPointerSet (right,result,ic);
         break;

         case FPOINTER:
         genFarPointerSet (right,result,ic);
         break;
       */
    case GPOINTER:
      genGenPointerSet (right, result, ic);
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

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  aopOp (cond, ic, FALSE);

  /* get the value into acc */
  if (AOP_TYPE (cond) != AOP_CRY)
    pic14_toBoolean (cond);
  else
    isbit = 1;

  /* if there was something to be popped then do it */
  if (popIc)
    genIpop (popIc);

  if (isbit)
    {
      /* This assumes that CARRY is set iff cond is true */
      if (IC_TRUE (ic))
        {
          assert (!IC_FALSE (ic));
          emitpcode (POC_BTFSC, popGet (AOP (cond), 0));
          //emitSKPNC;
          emitpcode (POC_GOTO, popGetLabel (IC_TRUE (ic)->key));
        }
      else
        {
          assert (IC_FALSE (ic));
          emitpcode (POC_BTFSS, popGet (AOP (cond), 0));
          //emitSKPC;
          emitpcode (POC_GOTO, popGetLabel (IC_FALSE (ic)->key));
        }
      if (0)
        {
          static int hasWarned = 0;
          if (!hasWarned)
            {
              fprintf (stderr, "WARNING: using untested code for %s:%u -- please check the .asm output and report bugs.\n",
                       ic->filename, ic->lineno);
              hasWarned = 1;
            }
        }
    }
  else
    {
      /* now Z is set iff !cond */
      if (IC_TRUE (ic))
        {
          assert (!IC_FALSE (ic));
          emitSKPZ;
          emitpcode (POC_GOTO, popGetLabel (IC_TRUE (ic)->key));
        }
      else
        {
          emitSKPNZ;
          emitpcode (POC_GOTO, popGetLabel (IC_FALSE (ic)->key));
        }
    }

  ic->generated = TRUE;

  /* the result is now in the accumulator */
  freeAsmop (cond, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void
genAddrOf (iCode * ic)
{
  operand *right, *result, *left;
  int size, offset;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);


  //aopOp(IC_RESULT(ic),ic,FALSE);

  aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  DEBUGpic14_AopType (__LINE__, left, right, result);
  assert (IS_SYMOP (left));

  /* sanity check: generic pointers to code space are not yet supported,
   * pionters to codespace must not be assigned addresses of __data values. */
#if 0
  fprintf (stderr, "result: %s, left: %s\n", OP_SYMBOL (result)->name, OP_SYMBOL (left)->name);
  fprintf (stderr, "result->type : ");
  printTypeChain (OP_SYM_TYPE (result), stderr);
  fprintf (stderr, ", codesp:%d, codeptr:%d, constptr:%d\n", IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_TYPE (result)))),
           IS_CODEPTR (OP_SYM_TYPE (result)), IS_PTR_CONST (OP_SYM_TYPE (result)));
  fprintf (stderr, "result->etype: ");
  printTypeChain (OP_SYM_ETYPE (result), stderr);
  fprintf (stderr, ", codesp:%d, codeptr:%d, constptr:%d\n", IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_ETYPE (result)))),
           IS_CODEPTR (OP_SYM_ETYPE (result)), IS_PTR_CONST (OP_SYM_ETYPE (result)));
  fprintf (stderr, "left->type   : ");
  printTypeChain (OP_SYM_TYPE (left), stderr);
  fprintf (stderr, ", codesp:%d, codeptr:%d, constptr:%d\n", IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_TYPE (left)))),
           IS_CODEPTR (OP_SYM_TYPE (left)), IS_PTR_CONST (OP_SYM_TYPE (left)));
  fprintf (stderr, "left->etype  : ");
  printTypeChain (OP_SYM_ETYPE (left), stderr);
  fprintf (stderr, ", codesp:%d, codeptr:%d, constptr:%d\n", IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_ETYPE (left)))),
           IS_CODEPTR (OP_SYM_ETYPE (left)), IS_PTR_CONST (OP_SYM_ETYPE (left)));
#endif

  if (IS_SYMOP (result) && IS_CODEPTR (OP_SYM_TYPE (result)) && !IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_TYPE (left)))))
    {
      fprintf (stderr, "trying to assign __code pointer (%s) an address in __data space (&%s) -- expect trouble\n",
               IS_SYMOP (result) ? OP_SYMBOL (result)->name : "unknown", OP_SYMBOL (left)->name);
    }
  else if (IS_SYMOP (result) && !IS_CODEPTR (OP_SYM_TYPE (result)) && IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_TYPE (left)))))
    {
      fprintf (stderr, "trying to assign __data pointer (%s) an address in __code space (&%s) -- expect trouble\n",
               IS_SYMOP (result) ? OP_SYMBOL (result)->name : "unknown", OP_SYMBOL (left)->name);
    }

  size = AOP_SIZE (IC_RESULT (ic));
  if (IS_SYMOP (result) && IS_GENPTR (OP_SYM_TYPE (result)))
    {
      /* strip tag */
      if (size > GPTRSIZE - 1)
        size = GPTRSIZE - 1;
    }
  offset = 0;

  while (size--)
    {
      /* fixing bug #863624, reported from (errolv) */
      emitpcode (POC_MOVLW, popGetImmd (OP_SYMBOL (left)->rname, offset, 0, IS_FUNC (OP_SYM_TYPE (left))));
      emitpcode (POC_MOVWF, popGet (AOP (result), offset));

#if 0
      emitpcode (POC_MOVLW, popGet (AOP (left), offset));
      emitpcode (POC_MOVWF, popGet (AOP (result), offset));
#endif
      offset++;
    }

  if (IS_SYMOP (result) && IS_GENPTR (OP_SYM_TYPE (result)))
    {
      /* provide correct tag */
      int isCode = IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_TYPE (left))));
      emitpcode (POC_MOVLW, popGetLit (isCode ? GPTRTAG_CODE : GPTRTAG_DATA));
      movwf (AOP (result), 2);
    }

  freeAsmop (left, NULL, ic, FALSE);
  freeAsmop (result, NULL, ic, TRUE);

}

/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void
genAssign (iCode * ic)
{
  operand *result, *right;
  int size, offset, know_W;
  unsigned long lit = 0L;

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  /* if they are the same */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  aopOp (right, ic, FALSE);
  aopOp (result, ic, TRUE);

  DEBUGpic14_AopType (__LINE__, NULL, right, result);

  /* if they are the same registers */
  if (pic14_sameRegs (AOP (right), AOP (result)))
    goto release;

  /* special case: assign from __code */
  if (!IS_ITEMP (right)         /* --> iTemps never reside in __code */
      && IS_SYMOP (right)       /* --> must be an immediate (otherwise we would be in genConstPointerGet) */
      && !IS_FUNC (OP_SYM_TYPE (right)) /* --> we would want its address instead of the first instruction */
      && !IS_CODEPTR (OP_SYM_TYPE (right))      /* --> get symbols address instread */
      && IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_TYPE (right)))))
    {
      emitpComment ("genAssign from CODESPACE");
      genConstPointerGet (right, result, ic);
      goto release;
    }

  /* just for symmetry reasons... */
  if (!IS_ITEMP (result) && IS_SYMOP (result) && IN_CODESPACE (SPEC_OCLS (getSpec (OP_SYM_TYPE (result)))))
    {
      assert (!"cannot write to CODESPACE");
    }

  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {

      /* if the right size is a literal then
         we know what the value is */
      if (AOP_TYPE (right) == AOP_LIT)
        {

          emitpcode ((((int) operandLitValue (right)) ? POC_BSF : POC_BCF), popGet (AOP (result), 0));

          if (((int) operandLitValue (right)))
            pic14_emitcode ("bsf", "(%s >> 3),(%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
          else
            pic14_emitcode ("bcf", "(%s >> 3),(%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
          goto release;
        }

      /* the right is also a bit variable */
      if (AOP_TYPE (right) == AOP_CRY)
        {
          emitpcode (POC_BCF, popGet (AOP (result), 0));
          emitpcode (POC_BTFSC, popGet (AOP (right), 0));
          emitpcode (POC_BSF, popGet (AOP (result), 0));

          pic14_emitcode ("bcf", "(%s >> 3),(%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
          pic14_emitcode ("btfsc", "(%s >> 3),(%s & 7)", AOP (right)->aopu.aop_dir, AOP (right)->aopu.aop_dir);
          pic14_emitcode ("bsf", "(%s >> 3),(%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
          goto release;
        }

      /* we need to or */
      emitpcode (POC_BCF, popGet (AOP (result), 0));
      pic14_toBoolean (right);
      emitSKPZ;
      emitpcode (POC_BSF, popGet (AOP (result), 0));
      //aopPut(AOP(result),"a",0);
      goto release;
    }

  /* bit variables done */
  /* general case */
  size = AOP_SIZE (result);
  offset = 0;
  if (AOP_TYPE (right) == AOP_DIR && (AOP_TYPE (result) == AOP_REG) && size == 1)
    {
      DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      if (aopIdx (AOP (result), 0) == 4)
        {
          DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
          emitpcode (POC_MOVFW, popGet (AOP (right), offset));
          emitpcode (POC_MOVWF, popGet (AOP (result), offset));
          goto release;
        }
      else
        DEBUGpic14_emitcode ("; WARNING", "%s  %d ignoring register storage", __FUNCTION__, __LINE__);
    }

  know_W = -1;
  while (size--)
    {

      DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      if (AOP_TYPE (right) == AOP_LIT)
        {
          lit = (unsigned long) pic14aopLiteral (AOP (right)->aopu.aop_lit, offset) & 0x0ff;
          if (lit & 0xff)
            {
              if (know_W != (int) (lit & 0xff))
                emitpcode (POC_MOVLW, popGetLit (lit & 0xff));
              know_W = lit & 0xff;
              emitpcode (POC_MOVWF, popGet (AOP (result), offset));
            }
          else
            emitpcode (POC_CLRF, popGet (AOP (result), offset));

        }
      else if (AOP_TYPE (right) == AOP_CRY)
        {
          emitpcode (POC_CLRF, popGet (AOP (result), offset));
          if (offset == 0)
            {
              emitpcode (POC_BTFSS, popGet (AOP (right), 0));
              emitpcode (POC_INCF, popGet (AOP (result), 0));
            }
        }
      else
        {
          mov2w_op (right, offset);
          emitpcode (POC_MOVWF, popGet (AOP (result), offset));
        }

      offset++;
    }


release:
  freeAsmop (right, NULL, ic, FALSE);
  freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genJumpTab - generates code for jump table                      */
/*-----------------------------------------------------------------*/
static void
genJumpTab (iCode * ic)
{
  symbol *jtab;
  char *l;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  aopOp (IC_JTCOND (ic), ic, FALSE);
  /* get the condition into accumulator */
  l = aopGet (AOP (IC_JTCOND (ic)), 0, FALSE, FALSE);
  MOVA (l);
  /* multiply by three */
  pic14_emitcode ("add", "a,acc");
  pic14_emitcode ("add", "a,%s", aopGet (AOP (IC_JTCOND (ic)), 0, FALSE, FALSE));

  jtab = newiTempLabel (NULL);
  pic14_emitcode ("mov", "dptr,#%05d_DS_", labelKey2num (jtab->key));
  pic14_emitcode ("jmp", "@a+dptr");
  pic14_emitcode ("", "%05d_DS_:", labelKey2num (jtab->key));

  emitpcode (POC_MOVLW, popGetHighLabel (jtab->key));
  emitpcode (POC_MOVWF, popCopyReg (&pc_pclath));
  emitpcode (POC_MOVLW, popGetLabel (jtab->key));
  emitpcode (POC_ADDFW, popGet (AOP (IC_JTCOND (ic)), 0));
  emitSKPNC;
  emitpcode (POC_INCF, popCopyReg (&pc_pclath));
  emitpcode (POC_MOVWF, popCopyReg (&pc_pcl));
  emitpLabel (jtab->key);

  freeAsmop (IC_JTCOND (ic), NULL, ic, TRUE);

  /* now generate the jump labels */
  for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
    {
      pic14_emitcode ("ljmp", "%05d_DS_", labelKey2num (jtab->key));
      emitpcode (POC_GOTO, popGetLabel (jtab->key));

    }

}

/*-----------------------------------------------------------------*/
/* genCast - gen code for casting                                  */
/*-----------------------------------------------------------------*/
static void
genCast (iCode * ic)
{
  operand *result = IC_RESULT (ic);
  sym_link *restype = operandType (result);
  sym_link *rtype = operandType (IC_RIGHT (ic));
  operand *right = IC_RIGHT (ic);
  int size, offset;

  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* if they are equivalent then do nothing */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  aopOp (right, ic, FALSE);
  aopOp (result, ic, FALSE);

  DEBUGpic14_AopType (__LINE__, NULL, right, result);

  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      assert (!"assigning to bit variables is not supported");
    }

  if ((AOP_TYPE (right) == AOP_CRY) && (AOP_TYPE (result) == AOP_REG))
    {
      int offset = 1;
      size = AOP_SIZE (result);

      DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

      emitpcode (POC_CLRF, popGet (AOP (result), 0));
      emitpcode (POC_BTFSC, popGet (AOP (right), 0));
      emitpcode (POC_INCF, popGet (AOP (result), 0));

      while (size--)
        emitpcode (POC_CLRF, popGet (AOP (result), offset++));

      goto release;
    }

  if (IS_BOOL (operandType (result)))
    {
      pic14_toBoolean (right);
      emitSKPNZ;
      emitpcode (POC_MOVLW, popGetLit (1));
      emitpcode (POC_MOVWF, popGet (AOP (result), 0));
      goto release;
    }

  if (IS_PTR (restype))
    {
      operand *result = IC_RESULT (ic);
      //operand *left = IC_LEFT(ic);
      operand *right = IC_RIGHT (ic);
      int tag = 0xff;

      /* copy common part */
      int max, size = AOP_SIZE (result);
      if (size > AOP_SIZE (right))
        size = AOP_SIZE (right);
      DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

      /* warn if we discard generic opinter tag */
      if (!IS_GENPTR (restype) && IS_GENPTR (rtype) && (AOP_SIZE (result) < AOP_SIZE (right)))
        {
          //fprintf (stderr, "%s:%u: discarding generic pointer type tag\n", __FUNCTION__, __LINE__);
        }                       // if

      max = size;
      while (size--)
        {
          mov2w_op (right, size);
          movwf (AOP (result), size);
        }                       // while

      /* upcast into generic pointer type? */
      if (IS_GENPTR (restype) && (size < AOP_SIZE (result)) && (!IS_GENPTR (rtype) || AOP_SIZE (right) < GPTRSIZE))
        {
          //fprintf (stderr, "%s:%u: must determine pointer type\n", __FUNCTION__, __LINE__);
          if (IS_PTR (rtype))
            {
              switch (DCL_TYPE (rtype))
                {
                case POINTER:  /* __data */
                case FPOINTER: /* __data */
                  assert (AOP_SIZE (right) == 2);
                  tag = GPTRTAG_DATA;
                  break;

                case CPOINTER: /* __code */
                  assert (AOP_SIZE (right) == 2);
                  tag = GPTRTAG_CODE;
                  break;

                case GPOINTER: /* unknown destination, __data or __code */
                  /* assume __data space (address of immediate) */
                  assert (AOP_TYPE (right) == AOP_PCODE && AOP (right)->aopu.pcop->type == PO_IMMEDIATE);
                  if (AOP (right)->code)
                    tag = GPTRTAG_CODE;
                  else
                    tag = GPTRTAG_DATA;
                  break;

                default:
                  assert (!"unhandled pointer type");
                }               // switch
            }
          else
            {
              /* convert other values into pointers to __data space */
              tag = GPTRTAG_DATA;
            }

          assert (AOP_SIZE (result) == 3);
          if (tag == 0)
            {
              emitpcode (POC_CLRF, popGet (AOP (result), 2));
            }
          else
            {
              emitpcode (POC_MOVLW, popGetLit (tag));
              movwf (AOP (result), 2);
            }
        }
      else
        {
          addSign (result, max, 0);
        }                       // if
      goto release;
    }

  /* if they are the same size : or less */
  if (AOP_SIZE (result) <= AOP_SIZE (right))
    {

      /* if they are in the same place */
      if (pic14_sameRegs (AOP (right), AOP (result)))
        goto release;

      DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      if (IS_PTR_CONST (rtype))
        DEBUGpic14_emitcode ("; ***", "%d - right is const pointer", __LINE__);
      if (IS_PTR_CONST (operandType (IC_RESULT (ic))))
        DEBUGpic14_emitcode ("; ***", "%d - result is const pointer", __LINE__);

      if ((AOP_TYPE (right) == AOP_PCODE) && AOP (right)->aopu.pcop->type == PO_IMMEDIATE)
        {
          emitpcode (POC_MOVLW, popGetAddr (AOP (right), 0, 0));
          emitpcode (POC_MOVWF, popGet (AOP (result), 0));
          emitpcode (POC_MOVLW, popGetAddr (AOP (right), 1, 0));
          emitpcode (POC_MOVWF, popGet (AOP (result), 1));
          if (AOP_SIZE (result) < 2)
            fprintf (stderr, "%d -- result is not big enough to hold a ptr\n", __LINE__);

        }
      else
        {

          /* if they in different places then copy */
          size = AOP_SIZE (result);
          offset = 0;
          while (size--)
            {
              emitpcode (POC_MOVFW, popGet (AOP (right), offset));
              emitpcode (POC_MOVWF, popGet (AOP (result), offset));

              //aopPut(AOP(result),
              // aopGet(AOP(right),offset,FALSE,FALSE),
              // offset);

              offset++;
            }
        }
      goto release;
    }

  /* so we now know that the size of destination is greater
     than the size of the source. */

  /* we move to result for the size of source */
  size = AOP_SIZE (right);
  offset = 0;
  while (size--)
    {
      emitpcode (POC_MOVFW, popGet (AOP (right), offset));
      emitpcode (POC_MOVWF, popGet (AOP (result), offset));
      offset++;
    }

  addSign (result, AOP_SIZE (right), !SPEC_USIGN (rtype));

release:
  freeAsmop (right, NULL, ic, TRUE);
  freeAsmop (result, NULL, ic, TRUE);

}

/*-----------------------------------------------------------------*/
/* genDjnz - generate decrement & jump if not zero instrucion      */
/*-----------------------------------------------------------------*/
static int
genDjnz (iCode * ic, iCode * ifx)
{
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (!ifx)
    return 0;

  /* if the if condition has a false label
     then we cannot save */
  if (IC_FALSE (ifx))
    return 0;

  /* if the minus is not of the form
     a = a - 1 */
  if (!isOperandEqual (IC_RESULT (ic), IC_LEFT (ic)) || !IS_OP_LITERAL (IC_RIGHT (ic)))
    return 0;

  if (operandLitValue (IC_RIGHT (ic)) != 1)
    return 0;

  /* if the size of this greater than one then no
     saving */
  if (getSize (operandType (IC_RESULT (ic))) > 1)
    return 0;

  /* otherwise we can save BIG */
  aopOp (IC_RESULT (ic), ic, FALSE);

  emitpcode (POC_DECFSZ, popGet (AOP (IC_RESULT (ic)), 0));
  emitpcode (POC_GOTO, popGetLabel (IC_TRUE (ifx)->key));

  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
  ifx->generated = TRUE;
  return 1;
}

/*-----------------------------------------------------------------*/
/* genReceive - generate code for a receive iCode                  */
/*-----------------------------------------------------------------*/
static void
genReceive (iCode * ic)
{
  FENTRY;
  DEBUGpic14_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (isOperandInFarSpace (IC_RESULT (ic)) && (OP_SYMBOL (IC_RESULT (ic))->isspilt || IS_TRUE_SYMOP (IC_RESULT (ic))))
    {

      int size = getSize (operandType (IC_RESULT (ic)));
      int offset = fReturnSizePic - size;
      while (size--)
        {
          pic14_emitcode ("push", "%s", (strcmp (fReturn[fReturnSizePic - offset - 1], "a") ?
                                         fReturn[fReturnSizePic - offset - 1] : "acc"));
          offset++;
        }
      aopOp (IC_RESULT (ic), ic, FALSE);
      size = AOP_SIZE (IC_RESULT (ic));
      offset = 0;
      while (size--)
        {
          pic14_emitcode ("pop", "acc");
          aopPut (AOP (IC_RESULT (ic)), "a", offset++);
        }

    }
  else
    {
      _G.accInUse++;
      aopOp (IC_RESULT (ic), ic, FALSE);
      _G.accInUse--;
      GpseudoStkPtr = ic->parmBytes;    // address used arg on stack
      assignResultValue (IC_RESULT (ic));
    }

  freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genDummyRead - generate code for dummy read of volatiles        */
/*-----------------------------------------------------------------*/
static void
genDummyRead (iCode * ic)
{
  FENTRY;
  pic14_emitcode ("; genDummyRead", "");
  pic14_emitcode ("; not implemented", "");

  ic = ic;
}

/*-----------------------------------------------------------------*/
/* genpic14Code - generate code for pic14 based controllers        */
/*-----------------------------------------------------------------*/
/*
* At this point, ralloc.c has gone through the iCode and attempted
* to optimize in a way suitable for a PIC. Now we've got to generate
* PIC instructions that correspond to the iCode.
*
* Once the instructions are generated, we'll pass through both the
* peep hole optimizer and the pCode optimizer.
*-----------------------------------------------------------------*/

void
genpic14Code (iCode * lic)
{
  iCode *ic;
  int cln = 0;
  const char *cline;

  FENTRY;

  pic = pic14_getPIC();

  pb = newpCodeChain (GcurMemmap, 0, newpCodeCharP ("; Starting pCode block"));
  addpBlock (pb);

  /* if debug information required */
  if (options.debug && debugFile && currFunc)
    {
      debugFile->writeFunction (currFunc, lic);
    }


  for (ic = lic; ic; ic = ic->next)
    {
      initGenLineElement ();

      //DEBUGpic14_emitcode(";ic","");
      //fprintf (stderr, "in ic loop\n");
      //pic14_emitcode ("", ";\t%s:%d: %s", ic->filename,
      //ic->lineno, printCLine(ic->filename, ic->lineno));

      if (!options.noCcodeInAsm && (cln != ic->lineno))
        {
          cln = ic->lineno;
          //fprintf (stderr, "%s\n", printCLine (ic->filename, ic->lineno));
          cline = printCLine (ic->filename, ic->lineno);
          if (!cline || strlen (cline) == 0)
            cline = printCLine (ic->filename, ic->lineno);
          addpCode2pBlock (pb, newpCodeCSource (ic->lineno, ic->filename, cline));
          //emitpComment ("[C-SRC] %s:%d: %s", ic->filename, cln, cline);
        }

      if (options.iCodeInAsm)
        {
          const char *iLine = printILine (ic);
          emitpComment ("[ICODE] %s:%d: %s", ic->filename, ic->lineno, printILine (ic));
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
          /* IPOP happens only when trying to restore a
             spilt live range, if there is an ifx statement
             following this pop then the if statement might
             be using some of the registers being popped which
             would destory the contents of the register so
             we need to check for this condition and handle it */
          if (ic->next && ic->next->op == IFX && regsInCommon (IC_LEFT (ic), IC_COND (ic->next)))
            genIfx (ic->next, ic);
          else
            genIpop (ic);
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
             during parsing SDCC.y */
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
          pic14_genInline (ic);
          break;

        case RRC:
          genRRC (ic);
          break;

        case RLC:
          genRLC (ic);
          break;

        case GETABIT:
          genGetABit (ic);
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
          genPointerGet (ic);
          break;

        case '=':
          if (POINTER_SET (ic))
            genPointerSet (ic);
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

        default:
          fprintf (stderr, "UNHANDLED iCode: ");
          piCode (ic, stderr);
          ic = ic;
          break;
        }
    }


  /* now we are ready to call the
     peep hole optimizer */
  if (!options.nopeep)
    {
      peepHole (&genLine.lineHead);
    }
  /* now do the actual printing */
  printLine (genLine.lineHead, codeOutBuf);

#ifdef PCODE_DEBUG
  DFPRINTF ((stderr, "printing pBlock\n\n"));
  printpBlock (stdout, pb);
#endif

  /* destroy the line list */
  destroy_line_list ();
}

/* This is not safe, as a AOP_PCODE/PO_IMMEDIATE might be used both as literal
 * (meaning: representing its own address) or not (referencing its contents).
 * This can only be decided based on the operand's type. */
static int
aop_isLitLike (asmop * aop)
{
  assert (aop);
  if (aop->type == AOP_LIT)
    return TRUE;
  if (aop->type == AOP_IMMD)
    return TRUE;
  if ((aop->type == AOP_PCODE) &&
      ((aop->aopu.pcop->type == PO_LITERAL) ||
       (aop->aopu.pcop->type == PO_IMMEDIATE)))
    {
      /* this should be treated like a literal/immediate (use MOVLW/ADDLW/SUBLW
       * instead of MOVFW/ADDFW/SUBFW, use popGetAddr instead of popGet) */
      return TRUE;
    }
  return FALSE;
}

int
op_isLitLike (operand * op)
{
  assert (op);
  if (aop_isLitLike (AOP (op)))
    return TRUE;
  if (IS_SYMOP (op) && IS_FUNC (OP_SYM_TYPE (op)))
    return TRUE;
  if (IS_SYMOP (op) && IS_PTR (OP_SYM_TYPE (op)) && (AOP_TYPE (op) == AOP_PCODE) && (AOP (op)->aopu.pcop->type == PO_IMMEDIATE))
    {
      return TRUE;
    }

  return FALSE;
}
