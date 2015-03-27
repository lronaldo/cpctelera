/*-------------------------------------------------------------------------
  SDCCgen51.c - source file for code generation for 8051

  Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)
         and -  Jean-Louis VERN.jlvern@writeme.com (1999)
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

  In other words, you are welcome to use, share and improve this program.
  You are forbidden to forbid anyone else to use, share and improve
  what you give them.   Help stamp out software-hoarding!

-------------------------------------------------------------------------*/

//#define D(x)
#define D(x) x

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SDCCglobl.h"
#include "newalloc.h"

#include "common.h"
#include "SDCCpeeph.h"
#include "ralloc.h"
#include "gen.h"

extern int allocInfo;

/* this is the down and dirty file with all kinds of
   kludgy & hacky stuff. This is what it is all about
   CODE GENERATION for a specific MCU . some of the
   routines may be reusable, will have to see */

static struct
  {
    short inLine;
    short debugLine;
    short stackExtend;
    short nRegsSaved;
    short parmsPushed;
    set *sendSet;
  }
_G;

extern int xa51_ptrRegReq;
extern int xa51_nRegs;
extern struct dbuf_s *codeOutBuf;

static lineNode *lineHead = NULL;
static lineNode *lineCurr = NULL;

#define LSB     0
#define MSB16   1
#define MSB24   2
#define MSB32   3

static char *MOV="mov";
static char *MOVB="mov.b";
static char *MOVW="mov.w";
static char *MOVC="movc";
static char *MOVCB="movc.b";
static char *MOVCW="movc.w";

static char *R1L="r1l";
static char *R1="r1";

void bailOut (char *mesg) {
  fprintf (stderr, "%s: bailing out\n", mesg);
  exit (1);
}

/*-----------------------------------------------------------------*/
/* emitcode - writes the code into a file : for now it is simple    */
/*-----------------------------------------------------------------*/
static void emitcode (char *inst, char *fmt,...) {
  va_list ap;
  char lb[INITIAL_INLINEASM];
  char *lbp = lb;

  va_start (ap, fmt);

  if (inst && *inst)
    {
      if (fmt && *fmt)
        sprintf (lb, "%s\t", inst);
      else
        sprintf (lb, "%s", inst);
      vsprintf (lb + (strlen (lb)), fmt, ap);
    }
  else
    vsprintf (lb, fmt, ap);

  while (isspace (*lbp))
    lbp++;

  if (lbp && *lbp)
    lineCurr = (lineCurr ?
                connectLine (lineCurr, newLineNode (lb)) :
                (lineHead = newLineNode (lb)));
  lineCurr->isInline = _G.inLine;
  lineCurr->isDebug = _G.debugLine;
  va_end (ap);
}

/*-----------------------------------------------------------------*/
/* xa51_emitDebuggerSymbol - associate the current code location  */
/*   with a debugger symbol                                        */
/*-----------------------------------------------------------------*/
void
xa51_emitDebuggerSymbol (const char * debugSym)
{
  _G.debugLine = 1;
  emitcode ("", "%s ==.", debugSym);
  _G.debugLine = 0;
}


char *getStackOffset(int stack) {
  static char gsoBuf[1024];
  sprintf (gsoBuf, "r7+(%d%+d%+d)", stack,
           currFunc->stack, _G.nRegsSaved);
  return gsoBuf;
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
  return aop;
}

char *aopTypeName(asmop *aop) {
  switch (aop->type)
    {
    case AOP_LIT: return "lit";
    case AOP_REG: return "reg";
    case AOP_DIR: return "dir";
    case AOP_FAR: return "far";
    case AOP_CODE: return "code";
    case AOP_GPTR: return "gptr";
    case AOP_STK: return "stack";
    case AOP_IMMD: return "imm";
    case AOP_BIT: return "bit";
    }
  return "unknown";
}

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *aopForSym(symbol *sym,
                        bool canUsePointer, bool canUseOffset) {
  int size;
  asmop *aop;

  sym->aop = aop = newAsmop(0);
  size=aop->size=getSize(sym->type);

  // if the sym has registers
  if (sym->nRegs && sym->regs[0]) {
    aop->type=AOP_REG;
    sprintf (aop->name[0], "%s", sym->regs[0]->name);
    if (size > 2) {
      sprintf (aop->name[1], "%s", sym->regs[1]->name);
    }
    return aop;
  }

  // if it is on stack
  if (sym->onStack) {
    if (!canUsePointer || !canUseOffset) {
      aop->type=AOP_REG;
      switch (size)
        {
        case 1:
          emitcode ("mov.b", "r0l,[%s] ;aopForSym:stack:1", getStackOffset(sym->stack));
          sprintf (aop->name[0], "r0l");
          return aop;
        case 2:
          emitcode ("mov.w", "r0,[%s] ;aopForSym:stack:2", getStackOffset(sym->stack));
          sprintf (aop->name[0], "r0");
          return aop;
        case 3:
          emitcode ("mov.w", "r0,[%s] ;aopForSym:stack:3.w", getStackOffset(sym->stack));
          sprintf (aop->name[0], "r0");
          emitcode ("mov.b", "r1l,[%s] ;aopForSym:stack:3.b", getStackOffset(sym->stack+2));
          sprintf (aop->name[1], "r1l");
          return aop;
        case 4:
          emitcode ("mov.w", "r0,[%s] ;aopForSym:stack:4", getStackOffset(sym->stack));
          sprintf (aop->name[0], "r0");
          emitcode ("mov.w", "r1,[%s] ;aopForSym:stack:4", getStackOffset(sym->stack+2));
          sprintf (aop->name[1], "r1");
          return aop;
        }
    }
    aop->type=AOP_STK;
    sprintf (aop->name[0], "[%s]", getStackOffset(sym->stack));
    if (size > 2) {
      sprintf (aop->name[1], "[%s]", getStackOffset(sym->stack+2));
    }
    return aop;
  }

  // if it has a spillLoc
  if (sym->usl.spillLoc) {
    return aopForSym (sym->usl.spillLoc, canUsePointer, canUseOffset);
  }

  // if in bit space
  if (IN_BITSPACE(SPEC_OCLS(sym->etype))) {
    aop->type=AOP_BIT;
    sprintf (aop->name[0], "%s", sym->rname);
    return aop;
  }

  // if in direct space
  if (IN_DIRSPACE(SPEC_OCLS(sym->etype))) {
    aop->type=AOP_DIR;
    sprintf (aop->name[0], "%s", sym->rname);
    if (size>2) {
      sprintf (aop->name[1], "%s+2", sym->rname);
    }
    return aop;
  }

  // if in code space
  if (IN_CODESPACE(SPEC_OCLS(sym->etype))) {
    if (!canUsePointer) {
      aop->type=AOP_REG;
      switch (size)
        {
        case 1:
          emitcode (MOV, "r0,#%s", sym->rname);
          emitcode (MOVC, "r0l,[r0+]");
          sprintf (aop->name[0], "r0l");
          return aop;
        case 2:
          emitcode (MOV, "r0,#%s", sym->rname);
          emitcode (MOVC, "r0,[r0+]");
          sprintf (aop->name[0], "r0");
          return aop;
        case 3:
          emitcode (MOV, "r0,#%s", sym->rname);
          emitcode (MOVC, "r0,[r0+]");
          sprintf (aop->name[1], "r0");
          emitcode (MOV, "r1l,[r0+]");
          sprintf (aop->name[0], "r1l");
          return aop;
        case 4:
          emitcode (MOV, "r0,#%s", sym->rname);
          emitcode (MOVC, "r1,[r0+]");
          emitcode (MOVC, "r0,[r0+]");
          emitcode ("xch", "r0,r1");
          sprintf (aop->name[0], "r0");
          sprintf (aop->name[1], "r1");
          return aop;
        }

    } else {
      aop->type=AOP_CODE;
      emitcode ("mov", "r0,#%s ; aopForSym:code", sym->rname);
      sprintf (aop->name[0], "[r0]");
      if (size>2) {
        sprintf (aop->name[1], "[r0+2]");
      }
    }
    return aop;
  }

  // if in far space
  if (IN_FARSPACE(SPEC_OCLS(sym->etype))) {
    if (!canUsePointer) {
      aop->type=AOP_REG;
      switch (size)
        {
        case 1:
          emitcode (MOV, "r0,#%s", sym->rname);
          emitcode (MOV, "r0l,[r0]");
          sprintf (aop->name[0], "r0l");
          return aop;
        case 2:
          emitcode (MOV, "r0,#%s", sym->rname);
          emitcode (MOV, "r0,[r0]");
          sprintf (aop->name[0], "r0");
          return aop;
        case 3:
          emitcode (MOV, "r0,#%s", sym->rname);
          emitcode (MOV, "r1l,[r0+2]");
          sprintf (aop->name[1], "r1l");
          emitcode (MOV, "r0,[r0]");
          sprintf (aop->name[0], "r0");
          return aop;
        case 4:
          emitcode (MOV, "r0,#%s", sym->rname);
          emitcode (MOV, "r1,[r0+2]");
          sprintf (aop->name[1], "r1");
          emitcode (MOV, "r0,[r0]");
          sprintf (aop->name[0], "r0");
          return aop;
        }
    } else {
      aop->type=AOP_FAR;
      emitcode ("mov.w", "r0,#%s ; aopForSym:far", sym->rname);
      sprintf (aop->name[0], "[r0]");
      if (size>2) {
        sprintf (aop->name[1], "[r0+2]");
      }
      return aop;
    }
  }

  bailOut("aopForSym");
  return NULL;
}

/*-----------------------------------------------------------------*/
/* aopForVal - for a value                                         */
/*-----------------------------------------------------------------*/
static asmop *aopForVal(operand *op) {
  asmop *aop;

  if (IS_OP_LITERAL(op)) {
    op->aop = aop = newAsmop (AOP_LIT);
    switch ((aop->size=getSize(operandType(op))))
      {
      case 1:
        sprintf (aop->name[0], "#0x%02x",
                 SPEC_CVAL(operandType(op)).v_int & 0xff);
        sprintf (aop->name[1], "#0");
        break;
      case 2:
        sprintf (aop->name[0], "#0x%04x",
                 SPEC_CVAL(operandType(op)).v_int & 0xffff);
        sprintf (aop->name[1], "#0");
        break;
      case 3:
        // must be a generic pointer, can only be zero
        // ?? if (v!=0) fprintf (stderr, "invalid val op for gptr\n"); exit(1);
        sprintf (aop->name[0], "#0x%04x",
                 SPEC_CVAL(operandType(op)).v_uint & 0xffff);
        sprintf (aop->name[1], "#0");
        break;
      case 4:
        sprintf (aop->name[0], "#0x%04x",
                 SPEC_CVAL(operandType(op)).v_ulong & 0xffff);
        sprintf (aop->name[1], "#0x%04x",
                 SPEC_CVAL(operandType(op)).v_ulong >> 16);
        break;
      default:
        bailOut("aopForVal");
      }
    return aop;
  }

  // must be immediate
  if (IS_SYMOP(op)) {
    op->aop = aop = newAsmop (AOP_IMMD);
    switch ((aop->size=getSize(OP_SYMBOL(op)->type)))
      {
      case 1:
      case 2:
        sprintf (aop->name[0], "#%s", OP_SYMBOL(op)->rname);
        return aop;
      case 3: // generic pointer
        sprintf (aop->name[0], "#0x%02x", DCL_TYPE(operandType(op)));
        sprintf (aop->name[1], "#%s", OP_SYMBOL(op)->rname);
        return aop;
      }
  }

  bailOut ("aopForVal: unknown type");
  return NULL;
}

static int aopOp(operand *op,
                  bool canUsePointer, bool canUseOffset) {

  if (IS_SYMOP(op)) {
    op->aop=aopForSym (OP_SYMBOL(op), canUsePointer, canUseOffset);
    return AOP_SIZE(op);
  }
  if (IS_VALOP(op)) {
    op->aop=aopForVal (op);
    return AOP_SIZE(op);
  }

  bailOut("aopOp: unexpected operand");
  return 0;
}

bool aopEqual(asmop *aop1, asmop *aop2, int offset) {
  if (strcmp(aop1->name[offset], aop2->name[offset])) {
    return FALSE;
  }
  return TRUE;
}

bool aopIsDir(operand *op) {
  return AOP_TYPE(op)==AOP_DIR;
}

bool aopIsBit(operand *op) {
  return AOP_TYPE(op)==AOP_BIT;
}

bool aopIsPtr(operand *op) {
  if (AOP_TYPE(op)==AOP_STK ||
      AOP_TYPE(op)==AOP_CODE ||
      AOP_TYPE(op)==AOP_FAR) {
    return TRUE;
  } else {
    return FALSE;
  }
}

char *opRegName(operand *op, int offset, char *opName, bool decorate) {

  if (IS_SYMOP(op)) {
    if (OP_SYMBOL(op)->onStack) {
      sprintf (opName, "[%s]", getStackOffset(OP_SYMBOL(op)->stack));
      return opName;
    }
    if (IS_TRUE_SYMOP(op))
      return OP_SYMBOL(op)->rname;
    else if (OP_SYMBOL(op)->regs[offset])
      return OP_SYMBOL(op)->regs[offset]->name;
    else
      bailOut("opRegName: unknown regs");
  }

  if (IS_VALOP(op)) {
    switch (SPEC_NOUN(OP_VALUE(op)->type)) {
    case V_SBIT:
    case V_BIT:
      if (SPEC_CVAL(OP_VALUE(op)->type).v_int &&
          SPEC_CVAL(OP_VALUE(op)->type).v_int != 1) {
        bailOut("opRegName: invalid bit value");
      }
      // fall through
    case V_CHAR:
      sprintf (opName, "#%s0x%02x", decorate?"(char)":"",
               SPEC_CVAL(OP_VALUE(op)->type).v_int);
      break;
    case V_INT:
      if (SPEC_LONG(OP_VALUE(op)->type)) {
        sprintf (opName, "#%s0x%02x", decorate?"(long)":"",
                 SPEC_CVAL(OP_VALUE(op)->type).v_long);
      } else {
        sprintf (opName, "#%s0x%02x", decorate?"(int)":"",
                 SPEC_CVAL(OP_VALUE(op)->type).v_int);
      }
      break;
    case V_FLOAT:
      sprintf (opName, "#%s%f", decorate?"(float)":"",
               SPEC_CVAL(OP_VALUE(op)->type).v_float);
      break;
    default:
      bailOut("opRegName: unexpected noun");
    }
    return opName;
  }
  bailOut("opRegName: unexpected operand type");
  return NULL;
}

char * printOp (operand *op) {
  static char line[132];
  sym_link *optype=operandType(op);
  bool isPtr = IS_PTR(optype);

  if (IS_SYMOP(op)) {
    symbol *sym=OP_SYMBOL(op);
    if (!sym->regs[0] && SYM_SPIL_LOC(sym)) {
      sym=SYM_SPIL_LOC(sym);
    }
    if (isPtr) {
      sprintf (line, "[");
      if (DCL_TYPE(optype)==FPOINTER)
        strcat (line, "far * ");
      else if (DCL_TYPE(optype)==CPOINTER)
        strcat (line, "code * ");
      else if (DCL_TYPE(optype)==GPOINTER)
        strcat (line, "gen * ");
      else if (DCL_TYPE(optype)==POINTER)
        strcat (line, "near * ");
      else
        strcat (line, "unknown * ");
      strcat (line, "(");
      strcat (line, nounName(sym->etype));
      strcat (line, ")");
      strcat (line, sym->name);
      strcat (line, "]:");
    } else {
      sprintf (line, "(%s)%s:", nounName(sym->etype), sym->name);
    }
    if (sym->regs[0]) {
      strcat (line, sym->regs[0]->name);
      if (sym->regs[1]) {
        strcat (line, ",");
        strcat (line, sym->regs[1]->name);
      }
      return line;
    }
    if (sym->onStack) {
      sprintf (line+strlen(line), "stack%+d", sym->stack);
      return line;
    }
    if (IN_CODESPACE(SPEC_OCLS(sym->etype))) {
      strcat (line, "code");
      return line;
    }
    if (IN_FARSPACE(SPEC_OCLS(sym->etype))) {
      strcat (line, "far");
      return line;
    }
    if (IN_BITSPACE(SPEC_OCLS(sym->etype))) {
      strcat (line, "bit");
      return line;
    }
    if (IN_DIRSPACE(SPEC_OCLS(sym->etype))) {
      strcat (line, "dir");
      return line;
    }
    strcat (line, "unknown");
    return line;
  } else if (IS_VALOP(op)) {
    opRegName(op, 0, line, 1);
  } else if (IS_TYPOP(op)) {
    sprintf (line, "(");
    if (isPtr) {
      if (DCL_TYPE(optype)==FPOINTER)
        strcat (line, "far * ");
      else if (DCL_TYPE(optype)==CPOINTER)
        strcat (line, "code * ");
      else if (DCL_TYPE(optype)==GPOINTER)
        strcat (line, "gen * ");
      else if (DCL_TYPE(optype)==POINTER)
        strcat (line, "near * ");
      else
        strcat (line, "unknown * ");
    }
    // forget about static, volatile, ... for now
    if (SPEC_USIGN(operandType(op))) strcat (line, "unsigned ");
    if (SPEC_LONG(operandType(op))) strcat (line, "long ");
    strcat (line, nounName(operandType(op)));
    strcat (line, ")");
  } else {
    bailOut("printOp: unexpected operand type");
  }
  return line;
}

void printIc (bool printToStderr,
              char *op, iCode * ic, bool result, bool left, bool right) {
  char line[132];

  sprintf (line, "%s(%d)", op, ic->lineno);
  if (result) {
    strcat (line, " result=");
    strcat (line, printOp (IC_RESULT(ic)));
  }
  if (left) {
    strcat (line, " left=");
    strcat (line, printOp (IC_LEFT(ic)));
  }
  if (right) {
    strcat (line, " right=");
    strcat (line, printOp (IC_RIGHT(ic)));
  }
  emitcode (";", line);
  if (printToStderr) {
    fprintf (stderr, "%s\n", line);
  }
}

/*-----------------------------------------------------------------*/
/* toBoolean - return carry for operand!=0                           */
/*-----------------------------------------------------------------*/
static char *toBoolean (operand * op) {
  symbol *tlbl=newiTempLabel(NULL);

  switch (AOP_SIZE(op))
    {
    case 1:
    case 2:
      emitcode ("cjne", "%s,#1,%05d$; %s", AOP_NAME(op), tlbl->key+100,
                "This needs a second thought");

      emitcode ("", "%05d$:", tlbl->key+100);
      return "c";
    }

  bailOut("toBoolean: unknown size");
  return NULL;
}

/*-----------------------------------------------------------------*/
/* regsInCommon - two operands have some registers in common       */
/*-----------------------------------------------------------------*/
static bool regsInCommon (operand * op1, operand * op2) {
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
/* resultRemat - result  is rematerializable                       */
/*-----------------------------------------------------------------*/
static int resultRemat (iCode * ic) {
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
/* genNot - generate code for ! operation                          */
/*-----------------------------------------------------------------*/
static void genNot (iCode * ic) {
  printIc (0, "genNot:", ic, 1,1,0);
}

/*-----------------------------------------------------------------*/
/* genCpl - generate code for complement                           */
/*-----------------------------------------------------------------*/
static void genCpl (iCode * ic) {
  printIc (0, "genCpl", ic, 1,1,0);
}

/*-----------------------------------------------------------------*/
/* genUminus - unary minus code generation                         */
/*-----------------------------------------------------------------*/
static void genUminus (iCode * ic) {
  printIc (0, "genUminus", ic, 1,1,0);
}

/*-----------------------------------------------------------------*/
/* genIpush - generate code for pushing                            */
/*-----------------------------------------------------------------*/
static void genIpush (iCode * ic) {
  operand *left=IC_LEFT(ic);

  printIc (0, "genIpush", ic, 0,1,0);
  aopOp(left,FALSE,FALSE);


  if (AOP_TYPE(left)==AOP_LIT) {
    switch (AOP_SIZE(left))
      {
      case 1:
        emitcode ("mov", "r1l,%s", AOP_NAME(left)[0]);
        emitcode ("push", "r1l");
        _G.parmsPushed++;
        return;
      case 2:
        emitcode ("mov", "r1,%s", AOP_NAME(left)[0]);
        emitcode ("push", "r1");
        _G.parmsPushed++;
        return;
      case 3:
        emitcode ("mov", "r1l,%s", AOP_NAME(left)[1]);
        emitcode ("push", "r1l");
        emitcode ("mov", "r1,%s", AOP_NAME(left)[0]);
        emitcode ("push", "r1");
        _G.parmsPushed += 2;
        return;
      case 4:
        emitcode ("mov", "r1,%s", AOP_NAME(left)[1]);
        emitcode ("push", "r1");
        emitcode ("mov", "r1,%s", AOP_NAME(left)[0]);
        emitcode ("push", "r1");
        _G.parmsPushed += 2;
        return;
      }
  } else {
    if (AOP_SIZE(left)>2) {
      emitcode ("push", "%s", AOP_NAME(left)[1]);
      _G.parmsPushed++;
    }
    emitcode ("push", "%s", AOP_NAME(left)[0]);
    _G.parmsPushed++;
  }
}

/*-----------------------------------------------------------------*/
/* genIpop - recover the registers: can happen only for spilling   */
/*-----------------------------------------------------------------*/
static void genIpop (iCode * ic) {
  printIc (0, "genIpop", ic, 0,1,0);
}

/*-----------------------------------------------------------------*/
/* genCall - generates a call statement                            */
/*-----------------------------------------------------------------*/
static void genCall (iCode * ic) {
  operand *result=IC_RESULT(ic);

  emitcode (";", "genCall(%d) %s result=%s", ic->lineno,
            OP_SYMBOL(IC_LEFT(ic))->name,
            printOp (IC_RESULT(ic)));
  emitcode ("call", "%s", OP_SYMBOL(IC_LEFT(ic))->rname);

  /* readjust the stack if we have pushed some parms */
  if (_G.parmsPushed) {
    emitcode ("add", "r7,#0x%02x", _G.parmsPushed*2);
    _G.parmsPushed=0;
  }

  /* if we need to assign a result value */
  if (IS_ITEMP (IC_RESULT(ic)) &&
      OP_SYMBOL (IC_RESULT (ic))->nRegs) {
    aopOp(result,FALSE,FALSE);
    switch (AOP_SIZE(result))
      {
      case 1:
        emitcode ("mov", "%s,r0l", AOP_NAME(result)[0]);
        return;
      case 2:
        emitcode ("mov", "%s,r0", AOP_NAME(result)[0]);
        return;
      case 3:
        // generic pointer
        emitcode ("mov", "%s,r1l", AOP_NAME(result)[1]);
        emitcode ("mov", "%s,r0", AOP_NAME(result)[0]);
        return;
      case 4:
        emitcode ("mov", "%s,r1", AOP_NAME(result)[1]);
        emitcode ("mov", "%s,r0", AOP_NAME(result)[0]);
        return;
      }
    bailOut("genCall");
  }
}

/*-----------------------------------------------------------------*/
/* genPcall - generates a call by pointer statement                */
/*-----------------------------------------------------------------*/
static void
genPcall (iCode * ic)
{
  emitcode (";", "genPcall %s\n", OP_SYMBOL(IC_LEFT(ic))->name);
}

/*-----------------------------------------------------------------*/
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void genFunction (iCode * ic) {
  symbol *sym=OP_SYMBOL(IC_LEFT(ic));
  sym_link *type=sym->type;

  emitcode (";", "genFunction %s", sym->rname);

  /* print the allocation information */
  printAllocInfo (currFunc, codeOutBuf);

  emitcode ("", "%s:", sym->rname);

  if (IFFUNC_ISNAKED(type))
  {
      emitcode(";", "naked function: no prologue.");
      return;
  }

  /* adjust the stack for locals used in this function */
  if (sym->stack) {
    emitcode ("sub", "r7,#%d\t; create stack space for locals", sym->stack);
  }
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));

  printIc (0, "genEndFunction", ic, 0,0,0);

  if (IFFUNC_ISNAKED(sym->type)) {
      emitcode(";", "naked function: no epilogue.");
      if (options.debug && currFunc)
        debugFile->writeEndFunction (currFunc, ic, 0);
      return;
  }

  /* readjust the stock for locals used in this function */
  if (sym->stack) {
    emitcode ("add", "r7,#%d\t; release stack space for locals", sym->stack);
  }

  if (options.debug && currFunc) {
    debugFile->writeEndFunction (currFunc, ic, 1);
  }

  if (IFFUNC_ISISR(sym->type)) {
    emitcode ("reti", "");
  } else {
    emitcode ("ret", "");
  }
}

/*-----------------------------------------------------------------*/
/* genRet - generate code for return statement                     */
/*-----------------------------------------------------------------*/
static void genRet (iCode * ic) {

  if (!IC_LEFT(ic)) {
    printIc (0, "genRet", ic, 0, 0, 0);
  } else {
    printIc (0, "genRet", ic, 0, 1, 0);
    aopOp(IC_LEFT(ic), TRUE, TRUE);
    switch (AOP_SIZE(IC_LEFT(ic)))
      {
      case 4:
        emitcode ("mov", "r1,%s", AOP_NAME(IC_LEFT(ic))[1]);
        emitcode ("mov", "r0,%s", AOP_NAME(IC_LEFT(ic))[0]);
        break;
      case 3:
        emitcode ("mov", "r1l,%s", AOP_NAME(IC_LEFT(ic))[1]);
        // fall through
      case 2:
        emitcode ("mov", "r0,%s", AOP_NAME(IC_LEFT(ic))[0]);
        break;
      case 1:
        emitcode ("mov", "r0l,%s", AOP_NAME(IC_LEFT(ic))[0]);
        break;
      default:
        bailOut("genRet");
      }
  }

  emitcode ("jmp", "%05d$", returnLabel->key+100);
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void genLabel (iCode * ic) {
  /* special case never generate */
  if (IC_LABEL (ic) == entryLabel)
    return;

  emitcode (";", "genLabel(%d) %s", ic->lineno, IC_LABEL(ic)->name);
  emitcode ("", "%05d$:", (IC_LABEL (ic)->key + 100));
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a jmp                                      */
/*-----------------------------------------------------------------*/
static void genGoto (iCode * ic) {
  emitcode (";", "genGoto(%d) %s", ic->lineno, IC_LABEL(ic)->name);
  emitcode ("jmp", "%05d$", (IC_LABEL (ic)->key + 100));
}

/*-----------------------------------------------------------------*/
/* genPlus - generates code for addition                           */
/*-----------------------------------------------------------------*/
static void genPlus (iCode * ic) {
  operand *result=IC_RESULT(ic), *left=IC_LEFT(ic), *right=IC_RIGHT(ic);
  int size;
  char *instr;

  printIc (0, "genPlus", ic, 1,1,1);

  size=aopOp(result, TRUE, TRUE);

  /* if left is a literal, then exchange them */
  if (IS_LITERAL(operandType(left))) {
    operand *tmp = right;
    right = left;
    left = tmp;
  }

  if (aopIsBit(result)) {
    if (IS_LITERAL(operandType(right))) {
      if (operandLitValue(right)) {
        emitcode ("setb", AOP_NAME(result)[0]);
        return;
      }
      aopOp(left, TRUE, TRUE);
      emitcode ("mov", "%s,%s", AOP_NAME(result)[0], toBoolean(left));
      return;
    }
    bailOut("genPlus: unfinished genPlus bit");
  }

  aopOp(left, !aopIsPtr(result), !aopIsDir(result));
  aopOp(right, !aopIsPtr(result), !aopIsDir(result));

  // special case for "* = * + char", needs a closer look
  // heck, this shouldn't have come here but bug-223113 does
  if (size==3 && AOP_SIZE(right)==1) {
    emitcode ("mov", "r1l,%s", AOP_NAME(right)[0]);
    emitcode ("mov", "r1h,#0"); // ptr arith unsigned????????????
    emitcode ("mov", "%s,%s", AOP_NAME(result)[0], AOP_NAME(left)[0]);
    emitcode ("add.w", "%s,r1", AOP_NAME(result)[0]);
    emitcode ("mov", "%s,%s", AOP_NAME(result)[1], AOP_NAME(left)[1]);
    return;
  }

  // special case for "xdata * = xdata * + char", needs a closer look
  // heck, this shouldn't have come here but bug-441448 does
  if (size==2 && AOP_SIZE(right)==1) {
    emitcode ("mov", "r1l,%s", AOP_NAME(right)[0]);
    emitcode ("mov", "r1h,#0"); // ptr arith unsigned????????????
    emitcode ("mov", "%s,%s", AOP_NAME(result)[0], AOP_NAME(left)[0]);
    emitcode ("add.w", "%s,r1", AOP_NAME(result)[0]);
    return;
  }

  if (size>1) {
    instr="add.w";
  } else {
    instr="add.b";
  }
  if (!aopEqual(result->aop, left->aop, 0)) {
    emitcode ("mov", "%s,%s", AOP_NAME(result)[0], AOP_NAME(left)[0]);
  }
  emitcode (instr, "%s,%s", AOP_NAME(result)[0], AOP_NAME(right)[0]);
  if (size>2) {
    if (!aopEqual(result->aop, left->aop, 1)) {
      emitcode ("mov", "%s,%s", AOP_NAME(result)[1], AOP_NAME(left)[1]);
    }
    if (size==3) {
      // generic pointer
    } else {
      emitcode ("addc.w", "%s,%s", AOP_NAME(result)[1], AOP_NAME(right)[1]);
    }
  }
  return;
}

/*-----------------------------------------------------------------*/
/* genMinus - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
static void genMinus (iCode * ic) {
  operand *result=IC_RESULT(ic), *left=IC_LEFT(ic), *right=IC_RIGHT(ic);
  int size;
  char *instr;

  printIc (0, "genMinus", ic, 1,1,1);

  size=aopOp(result, TRUE, TRUE);

  /* if left is a literal, then exchange them */
  if (IS_LITERAL(operandType(left))) {
    operand *tmp = right;
    right = left;
    left = tmp;
  }

  if (aopIsBit(result)) {
    if (IS_LITERAL(operandType(right))) {
      if (operandLitValue(right)) {
        emitcode ("clr", AOP_NAME(result)[0]);
        return;
      }
      aopOp(left, TRUE, TRUE);
      emitcode ("mov", "%s,%s", AOP_NAME(result)[0], toBoolean(left));
      return;
    }
    bailOut("genPlus: unfinished genPlus bit");
  }

  if (isOperandEqual(result,left)) {
    left->aop=result->aop;
  } else {
    aopOp(left, !aopIsPtr(result), !aopIsDir(result));
  }
  aopOp(right, !aopIsPtr(result), !aopIsDir(result));

  if (size>1) {
    instr="sub.w";
  } else {
    instr="sub.b";
  }
  if (!aopEqual(result->aop, left->aop, 0)) {
    emitcode ("mov", "%s,%s", AOP_NAME(result)[0], AOP_NAME(left)[0]);
  }
  emitcode (instr, "%s,%s", AOP_NAME(result)[0], AOP_NAME(right)[0]);
  if (size>2) {
    if (!aopEqual(result->aop, left->aop, 1)) {
      emitcode ("mov", "%s,%s", AOP_NAME(result)[1], AOP_NAME(left)[1]);
    }
    if (size==3) {
      // generic pointer
    } else {
      emitcode ("subb.w", "%s,%s", AOP_NAME(result)[1], AOP_NAME(right)[1]);
    }
  }
  return;
}

/*-----------------------------------------------------------------*/
/* genMult - generates code for multiplication                     */
/*-----------------------------------------------------------------*/
static void genMult (iCode * ic) {
  printIc (0, "genMult", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* genDiv - generates code for division                            */
/*-----------------------------------------------------------------*/
static void genDiv (iCode * ic) {
  printIc (0, "genDiv", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* genMod - generates code for division                            */
/*-----------------------------------------------------------------*/
static void genMod (iCode * ic) {
  printIc (0, "genMod", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* ifxForOp - returns the icode containing the ifx for operand     */
/*-----------------------------------------------------------------*/
static iCode *ifxForOp (operand * op, iCode * ic) {
  /* if true symbol then needs to be assigned */
  if (IS_TRUE_SYMOP (op))
    return NULL;

  /* if this has register type condition and
     the next instruction is ifx with the same operand
     and live to of the operand is upto the ifx only then */
  if (ic->next &&
      ic->next->op == IFX &&
      IC_COND (ic->next)->key == op->key &&
      OP_SYMBOL (op)->liveTo <= ic->next->seq)
    return ic->next;

  return NULL;
}

/*-----------------------------------------------------------------*/
/* genCmp - compares whatever                                      */
/*-----------------------------------------------------------------*/
static void genCmp (iCode * ic, char *trueInstr, char *falseInstr) {
  iCode *ifx=ifxForOp(IC_RESULT(ic),ic);
  operand *left=IC_LEFT(ic), *right=IC_RIGHT(ic);
  int size;
  bool isTrue;
  char *instr;
  int jlbl;


  size=aopOp(left, TRUE, TRUE);
  aopOp(right, !aopIsPtr(left), TRUE);

  if (size==1) {
    instr="cmp.b";
  } else {
    instr="cmp.w";
  }

  if (IC_TRUE(ifx)) {
    isTrue=TRUE;
    jlbl=IC_TRUE(ifx)->key+100;
  } else {
    isTrue=FALSE;
    jlbl=IC_FALSE(ifx)->key+100;
  }

  if (!ifx) {
    aopOp(IC_RESULT(ic), !aopIsPtr(left), TRUE);
    jlbl=newiTempLabel(NULL)->key+100;
    emitcode("mov", "%s,#-1", AOP_NAME(IC_RESULT(ic))[0]);
    emitcode(instr, "%s,%s", AOP_NAME(left)[0], AOP_NAME(right)[0]);
    emitcode(isTrue ? trueInstr : falseInstr, "%05d$", jlbl);
    emitcode("cpl", "%s", AOP_NAME(IC_RESULT(ic))[0]);
    emitcode("", "%05d$:", jlbl);
  } else {
    emitcode(instr, "%s,%s", AOP_NAME(left)[0], AOP_NAME(right)[0]);
    emitcode(isTrue ? trueInstr : falseInstr, "%05d$", jlbl);
    ifx->generated=1;
  }

  if (size>2) {
    bailOut("genCmp: size > 2");
  }

}

/*-----------------------------------------------------------------*/
/* genCmpEq :- generates code for equal to                         */
/*-----------------------------------------------------------------*/
static void genCmpEq (iCode * ic) {
  printIc (0, "genCmpEq", ic, 0,1,1);
  genCmp(ic, "beq", "bne"); // no sign
}

/*-----------------------------------------------------------------*/
/* genCmpGt :- greater than comparison                             */
/*-----------------------------------------------------------------*/
static void genCmpGt (iCode * ic) {
  printIc (0, "genCmpGt", ic, 0,1,1);
  if (SPEC_USIGN(operandType(IC_LEFT(ic))) ||
      SPEC_USIGN(operandType(IC_RIGHT(ic)))) {
    genCmp(ic, "bg", "bl"); // unsigned
  } else {
    genCmp(ic, "bgt", "ble"); // signed
  }
}

/*-----------------------------------------------------------------*/
/* genCmpLt - less than comparisons                                */
/*-----------------------------------------------------------------*/
static void genCmpLt (iCode * ic) {
  printIc (0, "genCmpLt", ic, 0,1,1);
  if (SPEC_USIGN(operandType(IC_LEFT(ic))) ||
      SPEC_USIGN(operandType(IC_RIGHT(ic)))) {
    genCmp(ic, "bcs", "bcc"); // unsigned
  } else {
    genCmp(ic, "blt", "bge"); // signed
  }
}

/*-----------------------------------------------------------------*/
/* hasInc - operand is incremented before any other use            */
/*-----------------------------------------------------------------*/
static iCode *hasInc (operand *op, iCode *ic, int osize) {
  sym_link *type = operandType(op);
  sym_link *retype = getSpec (type);
  iCode *lic = ic->next;
  int isize ;

  /* this could from a cast, e.g.: "(char xdata *) 0x7654;" */
  if (!IS_SYMOP(op)) return NULL;

  if (IS_BITVAR(retype)||!IS_PTR(type)) return NULL;
  if (IS_AGGREGATE(type->next)) return NULL;
  if (osize != (isize = getSize(type->next))) return NULL;

  while (lic) {
    /* if operand of the form op = op + <sizeof *op> */
    if (lic->op == '+') {
      if (isOperandEqual(IC_LEFT(lic),op) &&
          //isOperandEqual(IC_RESULT(lic),op) &&
          isOperandLiteral(IC_RIGHT(lic)) &&
          operandLitValue(IC_RIGHT(lic)) == isize) {
        emitcode (";", "Found hasInc");
        return lic;
      }
    }
    /* if the operand used or deffed */
    if (bitVectBitValue(OP_USES(op),lic->key) || lic->defKey == op->key) {
      return NULL;
    }
    /* if GOTO or IFX */
    if (lic->op == IFX || lic->op == GOTO || lic->op == LABEL) break;
    lic = lic->next;
  }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* genAndOp - for && operation                                     */
/*-----------------------------------------------------------------*/
static void genAndOp (iCode * ic) {
  printIc (0, "genAndOp(&&)", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* genOrOp - for || operation                                      */
/*-----------------------------------------------------------------*/
static void genOrOp (iCode * ic) {
  printIc (0, "genOrOp(||)", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* genAnd  - code for and                                            */
/*-----------------------------------------------------------------*/
static void genAnd (iCode * ic, iCode * ifx) {
  printIc (0, "genAnd", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* genOr  - code for or                                            */
/*-----------------------------------------------------------------*/
static void genOr (iCode * ic, iCode * ifx) {
  operand *result=IC_RESULT(ic), *left=IC_LEFT(ic), *right=IC_RIGHT(ic);
  int size;
  char *instr;

  printIc (0, "genOr", ic, 1,1,1);

  size=aopOp(result, TRUE, TRUE);

  /* if left is a literal, then exchange them */
  if (IS_LITERAL(operandType(left))) {
    operand *tmp = right;
    right = left;
    left = tmp;
  }

  if (aopIsBit(result)) {
    if (IS_LITERAL(operandType(right))) {
      if (operandLitValue(right)) {
        emitcode ("setb", AOP_NAME(result)[0]);
        return;
      }
      aopOp(left, TRUE, TRUE);
      emitcode ("mov", "%s,%s", AOP_NAME(result)[0], toBoolean(left));
      return;
    }
  }

  aopOp(left, !aopIsPtr(result), !aopIsDir(result));
  aopOp(right, !aopIsPtr(result), !aopIsDir(result));

  if (size>1) {
    instr="or.w";
  } else {
    instr="or.b";
  }
  if (!aopEqual(result->aop, left->aop, 0)) {
    emitcode ("mov", "%s,%s", AOP_NAME(result)[0], AOP_NAME(left)[0]);
  }
  emitcode (instr, "%s,%s", AOP_NAME(result)[0], AOP_NAME(right)[0]);
  if (size>2) {
    if (!aopEqual(result->aop, left->aop, 1)) {
      emitcode ("mov", "%s,%s", AOP_NAME(result)[1], AOP_NAME(left)[1]);
    }
    emitcode (instr, "%s,%s", AOP_NAME(result)[1], AOP_NAME(right)[1]);
  }
  return;
}

/*-----------------------------------------------------------------*/
/* genXor - code for xclusive or                                   */
/*-----------------------------------------------------------------*/
static void genXor (iCode * ic, iCode * ifx) {
  printIc (0, "genXor", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* genInline - write the inline code out                           */
/*-----------------------------------------------------------------*/
static void genInline (iCode * ic) {

  printIc (0, "genInline", ic, 0,0,0);

  emitcode ("", IC_INLINE(ic));
}

/*-----------------------------------------------------------------*/
/* genRRC - rotate right with carry                                */
/*-----------------------------------------------------------------*/
static void genRRC (iCode * ic) {
  printIc (0, "genRRC", ic, 1,1,0);
}

/*-----------------------------------------------------------------*/
/* genRLC - generate code for rotate left with carry               */
/*-----------------------------------------------------------------*/
static void genRLC (iCode * ic) {
  printIc (0, "genRLC", ic, 1,1,0);
}

/*-----------------------------------------------------------------*/
/* genGetHbit - generates code get highest order bit               */
/*-----------------------------------------------------------------*/
static void genGetHbit (iCode * ic) {
  printIc (0, "genGetHbit", ic, 1,1,0);
}

/*-----------------------------------------------------------------*/
/* genLeftShift - generates code for left shifting                 */
/*-----------------------------------------------------------------*/
static void genLeftShift (iCode * ic) {
  printIc (0, "genLeftShift", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* genRightShift - generate code for right shifting                */
/*-----------------------------------------------------------------*/
static void genRightShift (iCode * ic) {
  printIc (0, "genRightShift", ic, 1,1,1);
}

/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void genPointerGet (iCode * ic, iCode *pi) {
  char *instr, *scratchReg;
  operand *result=IC_RESULT(ic), *left=IC_LEFT(ic);
  bool codePointer=IS_CODEPTR(operandType(left));
  int size;

  if (pi) {
    printIc (0, "genPointerGet pi", ic, 1,1,0);
  } else {
    printIc (0, "genPointerGet", ic, 1,1,0);
  }

  if (!IS_PTR(operandType(left))) {
    bailOut ("genPointerGet: pointer required");
  }

  aopOp(left,FALSE,FALSE);
  size=aopOp(result,TRUE,aopIsDir(left));

  if (IS_GENPTR(operandType(left))) {
    symbol *tlbl1=newiTempLabel(NULL);
    symbol *tlbl2=newiTempLabel(NULL);
    emitcode ("cmp", "%s,#0x%02x", AOP_NAME(left)[1], CPOINTER);
    emitcode ("beq", "%05d$", tlbl1->key+100);
    // far/near pointer
    if (pi) {
      emitcode ("mov", "%s,[%s+]", AOP_NAME(result)[0], AOP_NAME(left)[0]);
      pi->generated=1;
    } else {
      emitcode ("mov", "%s,[%s]", AOP_NAME(result)[0], AOP_NAME(left)[0]);
    }
    if (size>2) {
      if (pi) {
        emitcode ("mov", "%s,[%s+]", AOP_NAME(result)[1], AOP_NAME(left)[0]);
      } else {
        emitcode ("mov", "%s,[%s+2]", AOP_NAME(result)[1], AOP_NAME(left)[0]);
      }
    }
    emitcode ("br", "%05d$", tlbl2->key+100);
    emitcode ("", "%05d$:", tlbl1->key+100);
    // code pointer
    if (pi) {
      emitcode ("movc", "%s,[%s+]", AOP_NAME(result)[0], AOP_NAME(left)[0]);
      pi->generated=1;
    } else {
      emitcode ("mov", "r0,%s", AOP_NAME(left)[0]);
      emitcode ("movc", "%s,[r0+]", AOP_NAME(result)[0]);
    }
    if (size>2) {
      if (pi) {
        emitcode ("movc", "%s,[%s+]", AOP_NAME(result)[1], AOP_NAME(left)[0]);
      } else {
        emitcode ("movc", "%s,[r0+]", AOP_NAME(result)[1]);
      }
    }
    emitcode ("", "%05d$:", tlbl2->key+100);
    return;
  }

  switch (AOP_TYPE(left))
    {
    case AOP_LIT:
      emitcode("mov","r1,%s", AOP_NAME(left)[0]);
      sprintf (AOP_NAME(left)[0], "r1");
      // fall through
    case AOP_REG:
      if (size>1) {
        if (codePointer) {
          instr=MOVCW;
        } else {
          instr=MOVW;
        }
        scratchReg=R1;
      } else {
        if (codePointer) {
          instr=MOVCB;
        } else {
          instr=MOVB;
        }
        scratchReg=R1L;
      }
      if (AOP_TYPE(result)==AOP_STK) {
        emitcode (MOV, "%s,[%s]", scratchReg, AOP_NAME(left)[0]);
        emitcode (MOV, "%s,%s", AOP_NAME(result)[0], scratchReg);
      } else {
        if (pi) {
          emitcode (instr, "%s,[%s+]", AOP_NAME(result)[0],
                    AOP_NAME(left)[0]);
          pi->generated=1;
        } else {
          if (codePointer) {
            emitcode (MOV, "r1,%s", AOP_NAME(left)[0]);
            emitcode (instr, "%s,[r1+]", AOP_NAME(result)[0]);
          } else {
            emitcode (instr, "%s,[%s]", AOP_NAME(result)[0],
                      AOP_NAME(left)[0]);
          }
        }
      }
      if (size > 2) {
        if (size==3) {
          if (codePointer) {
            instr=MOVCB;
          } else {
            instr=MOVB;
          }
          scratchReg=R1L;
        }
        if (AOP_TYPE(result)==AOP_STK) {
          emitcode (MOV, "%s,[%s+2]", scratchReg, AOP_NAME(left)[0]);
          emitcode (MOV, "%s,%s", AOP_NAME(result)[1], scratchReg);
        } else {
          if (pi) {
            emitcode (instr, "%s,[%s+]", AOP_NAME(result)[1],
                      AOP_NAME(left)[0]);
          } else {
            if (codePointer) {
              emitcode (instr, "%s,[r1]", AOP_NAME(result)[1]);
            } else {
              emitcode (instr, "%s,[%s+2]", AOP_NAME(result)[1],
                        AOP_NAME(left)[0]);
            }
          }
        }
      }
      return;
    }
  bailOut ("genPointerGet");
}

/*-----------------------------------------------------------------*/
/* genPointerSet - stores the value into a pointer location        */
/*-----------------------------------------------------------------*/
static void genPointerSet (iCode * ic, iCode *pi) {
  char *instr;
  operand *result=IC_RESULT(ic), *right=IC_RIGHT(ic);
  int size;

  printIc (0, "genPointerSet", ic, 1,0,1);

  if (!IS_PTR(operandType(result))) {
    bailOut ("genPointerSet: pointer required");
  }

  aopOp(result,FALSE,FALSE);
  size=aopOp(right,FALSE, FALSE);

  if (IS_GENPTR(operandType(result))) {
    emitcode (";", "INLINE\t_gptrset ; [%s %s] = %s %s",
              AOP_NAME(result)[0], AOP_NAME(result)[1],
              AOP_NAME(right)[0], AOP_NAME(right)[1]);
    return;
  }

  switch (AOP_TYPE(right))
    {
    case AOP_LIT:
    case AOP_REG:
      if (size>1) {
        instr=MOVW;
      } else {
        instr=MOVB;
      }
      if (pi) {
        emitcode (instr, "[%s+],%s", AOP_NAME(result)[0], AOP_NAME(right)[0]);
        pi->generated=1;
      } else {
        emitcode (instr, "[%s],%s", AOP_NAME(result)[0], AOP_NAME(right)[0]);
      }
      if (size > 2) {
        if (size==3) {
          instr=MOVB;
        }
        if (pi) {
          emitcode (instr, "[%s+],%s", AOP_NAME(result)[0],
                    AOP_NAME(right)[1]);
        } else {
          emitcode (instr, "[%s+2],%s", AOP_NAME(result)[0],
                    AOP_NAME(right)[1]);
        }
      }
      return;
    }
  bailOut ("genPointerSet");
}

/*-----------------------------------------------------------------*/
/* genIfx - generate code for Ifx statement                        */
/*-----------------------------------------------------------------*/
static void genIfx (iCode * ic, iCode * popIc) {
  int size;
  char *instr;
  bool trueOrFalse;
  symbol *jlbl, *tlbl=newiTempLabel(NULL);
  operand *cond=IC_COND(ic);

  emitcode (";", "genIfx(%d) cond=%s trueLabel:%s falseLabel:%s",
            ic->lineno, printOp(cond),
            IC_TRUE(ic) ? IC_TRUE(ic)->name : "NULL",
            IC_FALSE(ic) ? IC_FALSE(ic)->name : "NULL");

  size=aopOp(cond,TRUE,TRUE);

  if (IC_TRUE(ic)) {
    trueOrFalse=TRUE;
    jlbl=IC_TRUE(ic);
  } else {
    trueOrFalse=FALSE;
    jlbl=IC_FALSE(ic);
  }

  switch (AOP_TYPE(cond) )
    {
    case AOP_BIT:
      emitcode (trueOrFalse ? "jnb" : "jb", "%s,%05d$",
                AOP_NAME(cond)[0], tlbl->key+100);
      emitcode ("jmp", "%05d$", jlbl->key+100);
      emitcode ("", "%05d$:", tlbl->key+100);
      return;
    case AOP_REG:
    case AOP_DIR:
    case AOP_FAR:
    case AOP_STK:
      if (size>1) {
        instr="cmp.w";
      } else {
        instr="cmp.b";
      }
      emitcode (instr, "%s,#0", AOP_NAME(cond)[0]);
      emitcode (trueOrFalse ? "beq" : "bne", "%05d$", tlbl->key+100);
      if (size > 2) {
        if (size==3) {
          // generic pointer, forget the generic part
        } else {
          emitcode (instr, "%s,#0", AOP_NAME(cond)[1]);
          emitcode (trueOrFalse ? "beq" : "bne", "%05d$", tlbl->key+100);
        }
      }
      emitcode ("jmp", "%05d$", jlbl->key+100);
      emitcode ("", "%05d$:", tlbl->key+100);
      return;
    }
  bailOut ("genIfx");
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void genAddrOf (iCode * ic) {
  int size;
  operand *left=IC_LEFT(ic);

  printIc (0, "genAddrOf", ic, 1,1,0);

  size=aopOp (IC_RESULT(ic), FALSE, TRUE);

  if (isOperandOnStack(left)) {
    emitcode ("lea", "%s,%s", AOP_NAME(IC_RESULT(ic))[0],
              getStackOffset(OP_SYMBOL(left)->stack));
    if (size > 2) {
      // this must be a generic pointer
      emitcode ("mov.b", "%s,#0x%02x", AOP_NAME(IC_RESULT(ic))[1], FPOINTER);
    }
    return;
  }

  if (isOperandInDirSpace(left) ||
      isOperandInFarSpace(left) ||
      isOperandInCodeSpace(left)) {
    emitcode ("mov.w", "%s,#%s", AOP_NAME(IC_RESULT(ic))[0],
              OP_SYMBOL(left)->rname);
    if (size > 2) {
      // this must be a generic pointer
      int space=0; // dir space
      if (isOperandInFarSpace(left)) {
        space=1;
      } else if (isOperandInCodeSpace(left)) {
        space=2;
      }
      emitcode ("mov.b", "%s,#0x%02x", AOP_NAME(IC_RESULT(ic))[1], space);
    }
    return;
  }

  bailOut("genAddrOf");
}

/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void genAssign (iCode * ic) {
  operand *result=IC_RESULT(ic), *right=IC_RIGHT(ic);
  int size;
  char *instr;

  printIc (0, "genAssign", ic, 1,0,1);

  if (!IS_SYMOP(result)) {
    bailOut("genAssign: result is not a symbol");
  }

  aopOp(result, TRUE, TRUE);
  aopOp(right, !aopIsPtr(result), AOP_TYPE(result)!=AOP_DIR);
  size=AOP_SIZE(result);

  /* if result is a bit */
  if (AOP_TYPE(result) == AOP_BIT) {
    /* if right is literal, we know what the value is */
    if (AOP_TYPE(right) == AOP_LIT) {
      if (operandLitValue(right)) {
        emitcode ("setb", AOP_NAME(result)[0]);
      } else {
        emitcode ("clr", AOP_NAME(result)[0]);
      }
      return;
    }
    /* if right is also a bit */
    if (AOP_TYPE(right) == AOP_BIT) {
      emitcode ("mov", "c,%s", AOP_NAME(right));
      emitcode ("mov", "%s,c", AOP_NAME(result));
      return;
    }
    /* we need to or */
    emitcode ("mov", "%s,%s; toBoolean", AOP_NAME(result), toBoolean(right));
    return;
  }

  // TODO: if (-8 >= right==lit <= 7) instr=MOVS
  /* general case */
  if (size>1) {
    instr=MOVW;
  } else {
    instr=MOVB;
  }
  emitcode (instr, "%s,%s", AOP_NAME(result)[0], AOP_NAME(right)[0]);

  if (size > 2) {
    if (size==3) {
      // generic pointer
      instr=MOVB;
    }
    emitcode (instr, "%s,%s", AOP_NAME(result)[1], AOP_NAME(right)[1]);
    return;
  }
}

/*-----------------------------------------------------------------*/
/* genJumpTab - genrates code for jump table                       */
/*-----------------------------------------------------------------*/
static void genJumpTab (iCode * ic) {
  printIc (0, "genJumpTab", ic, 0,0,0);
}

/*-----------------------------------------------------------------*/
/* genCast - gen code for casting                                  */
/*-----------------------------------------------------------------*/
static void genCast (iCode * ic) {
  int size;
  operand *result=IC_RESULT(ic);
  operand *right=IC_RIGHT(ic);
  sym_link *ctype=operandType(IC_LEFT(ic));
  sym_link *rtype=operandType(IC_RIGHT(ic));
  sym_link *etype=getSpec(rtype);
  short ptrType, signedness;

  printIc (0, "genCast", ic, 1,1,1);

  aopOp(result, TRUE, TRUE);
  aopOp(right, !aopIsPtr(result), AOP_TYPE(result)!=AOP_DIR);
  size=AOP_SIZE(result);

  /* if result is a bit */
  if (AOP_TYPE(result) == AOP_BIT) {
    /* if right is literal, we know what the value is */
    if (AOP_TYPE(right) == AOP_LIT) {
      if (operandLitValue(right)) {
        emitcode ("setb", AOP_NAME(result)[0]);
      } else {
        emitcode ("clr", AOP_NAME(result)[0]);
      }
      return;
    }
    /* if right is also a bit */
    if (AOP_TYPE(right) == AOP_BIT) {
      emitcode ("mov", "c,%s", AOP_NAME(right));
      emitcode ("mov", "%s,c", AOP_NAME(result));
      return;
    }
    /* we need to or */
    emitcode ("mov", "%s,%s; toBoolean", AOP_NAME(result), toBoolean(right));
    return;
  }

  /* if right is a bit */
  if (AOP_TYPE(right)==AOP_BIT) {
    emitcode ("mov", "c,%s", AOP_NAME(right));
    emitcode ("mov", "%s,#0", AOP_NAME(result)[0]);
    emitcode ("rlc", "%s,#1", AOP_NAME(result)[0]);
    if (size>2) {
      emitcode ("mov", "%s,#0", AOP_NAME(result)[1]);
    }
    return;
  }

  /* if the result is of type pointer */
  if (IS_PTR (ctype)) {

    if (AOP_SIZE(right)>1) {
      emitcode ("mov", "%s,%s",  AOP_NAME(result)[0], AOP_NAME(right)[0]);
    } else {
      emitcode ("mov", "r1l,%s", AOP_NAME(right)[0]);
      emitcode ("sext", "r1h");
      emitcode ("mov", "%s,r1",  AOP_NAME(result)[0]);
    }

    /* if pointer to generic pointer */
    if (IS_GENPTR (ctype)) {

      if (IS_GENPTR (rtype)) {
        bailOut("genCast: gptr -> gptr");
      }

      if (IS_PTR (rtype)) {
        ptrType = DCL_TYPE (rtype);
      } else {
        /* we have to go by the storage class */
        if (!SPEC_OCLS(etype)) {
          ptrType=0; // hush the compiler
          bailOut("genCast: unknown storage class");
        } else {
          ptrType = PTR_TYPE (SPEC_OCLS (etype));
        }
      }

      /* the generic part depends on the type */
      switch (ptrType)
        {
        case POINTER:
          emitcode ("mov.b", "%s,#0x00", AOP_NAME(result)[1]);
          break;
        case FPOINTER:
          emitcode ("mov.b", "%s,#0x01", AOP_NAME(result)[1]);
          break;
        case CPOINTER:
          emitcode ("mov.b", "%s,#0x02", AOP_NAME(result)[1]);
          break;
        default:
          bailOut("genCast: got unknown storage class");
        }
    }
    return;
  }

  /* do we have to sign extend? */
  signedness = SPEC_USIGN(rtype);

  /* now depending on the size */
  switch ((AOP_SIZE(result)<<4) + AOP_SIZE(right))
    {
    case 0x44:
    case 0x33:
      emitcode("mov", "%s,%s", AOP_NAME(result)[1], AOP_NAME(right)[1]);
      // fall through
    case 0x24:
    case 0x22:
    case 0x11:
      emitcode("mov", "%s,%s", AOP_NAME(result)[0], AOP_NAME(right)[0]);
      return;
    case 0x42:
      emitcode("mov", "%s,%s", AOP_NAME(result)[0], AOP_NAME(right)[0]);
      if (signedness) {
        emitcode("sext", "%s", AOP_NAME(result)[1]);
      } else {
        emitcode("mov", "%s,#0", AOP_NAME(result)[1]);
      }
      return;
    case 0x41:
    case 0x21:
      emitcode("mov", "r1l,%s", AOP_NAME(right)[0]);
      if (signedness) {
        emitcode("sext", "r1h");
      } else {
        emitcode("mov", "r1h,#0");
      }
      emitcode("mov", "%s,r1", AOP_NAME(result)[0]);
      if (size==2)
        return;
      // fall through: case 0x41
      if (signedness) {
        emitcode("sext", "r1");
      } else {
        emitcode("mov", "r1,#0");
      }
      emitcode("mov", "%s,r1", AOP_NAME(result)[1]);
      return;
    case 0x14:
    case 0x12:
      emitcode("mov", "r1,%s", AOP_NAME(right)[0]);
      emitcode("mov", "%s,r1l", AOP_NAME(result)[0]);
      return;
    }
  fprintf(stderr, "genCast: unknown size: %d:%d\n",
          AOP_SIZE(result), AOP_SIZE(right));
  bailOut("genCast: unknown size");
}


/*-----------------------------------------------------------------*/
/* genDjnz - generate decrement & jump if not zero instrucion      */
/*-----------------------------------------------------------------*/
static bool genDjnz (iCode * ic, iCode * ifx) {
  symbol *lbl, *lbl1;

  if (!ifx)
    return 0;

  /* if the if condition has a false label
     then we cannot save */
  if (IC_FALSE (ifx))
    return 0;

  /* if the minus is not of the form
     a = a - 1 */
  if (!isOperandEqual (IC_RESULT (ic), IC_LEFT (ic)) ||
      !IS_OP_LITERAL (IC_RIGHT (ic)))
    return 0;

  if (operandLitValue (IC_RIGHT (ic)) != 1)
    return 0;

  /* if the size of this greater than two then no
     saving */
  if (getSize (operandType (IC_RESULT (ic))) > 2)
    return 0;

  printIc (0, "genDjnz", ic, 1,1,1);

  /* otherwise we can save BIG */
  lbl = newiTempLabel (NULL);
  lbl1 = newiTempLabel (NULL);

  aopOp (IC_RESULT (ic), FALSE, TRUE);

  if (AOP_TYPE(IC_RESULT(ic))==AOP_REG || AOP_TYPE(IC_RESULT(ic))==AOP_DIR) {
    emitcode ("djnz", "%s,%05d$", AOP_NAME(IC_RESULT(ic)), lbl->key+100);
    emitcode ("br", "%05d$", lbl1->key + 100);
    emitcode ("", "%05d$:", lbl->key + 100);
    emitcode ("jmp", "%05d$", IC_TRUE (ifx)->key + 100);
    emitcode ("", "%05d$:", lbl1->key + 100);
    return TRUE;
  }

  bailOut("genDjnz: aop type");
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* genReceive - generate code for a receive iCode                  */
/*-----------------------------------------------------------------*/
static void genReceive (iCode * ic) {
  printIc (0, "genReceive", ic, 1,0,0);
}

/*-----------------------------------------------------------------*/
/* genDummyRead - generate code for dummy read of volatiles        */
/*-----------------------------------------------------------------*/
static void
genDummyRead (iCode * ic)
{
  emitcode (";     genDummyRead","");

  ic = ic;
}

/*-----------------------------------------------------------------*/
/* gen51Code - generate code for 8051 based controllers            */
/*-----------------------------------------------------------------*/
void genXA51Code (iCode * lic) {
  iCode *ic;
  int cln = 0;

  lineHead = lineCurr = NULL;

  /* if debug information required */
  if (options.debug && currFunc)
    {
      debugFile->writeFunction (currFunc, lic);
    }

  for (ic = lic; ic; ic = ic->next) {
    if (ic->lineno && cln != ic->lineno) {
      if (options.debug) {
        debugFile->writeCLine (ic);
      }
      if (!options.noCcodeInAsm) {
        emitcode ("", ";\t%s:%d: %s", ic->filename, ic->lineno,
                  printCLine(ic->filename, ic->lineno));
      }
      cln = ic->lineno;
    }
    if (options.iCodeInAsm) {
      const char *iLine = printILine(ic);
      emitcode("", ";ic:%d: %s", ic->key, iLine);
      dbuf_free(iLine);
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
        if (ic->next &&
            ic->next->op == IFX &&
            regsInCommon (IC_LEFT (ic), IC_COND (ic->next)))
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
        genCmpGt (ic);
        break;

      case '<':
        genCmpLt (ic);
        break;

      case LE_OP:
      case GE_OP:
      case NE_OP:

        /* note these two are xlated by algebraic equivalence
           during parsing SDCC.y */
        werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                "got '>=' or '<=' shouldn't have come here");
        break;

      case EQ_OP:
        genCmpEq (ic);
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
        genPointerGet (ic, hasInc(IC_LEFT(ic), ic, getSize(operandType(IC_RESULT(ic)))));
        break;

      case '=':
        if (POINTER_SET (ic))
          genPointerSet (ic, hasInc(IC_RESULT(ic), ic, getSize(operandType(IC_RIGHT(ic)))));
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

      default:
        ic = ic;
      }
  }


  /* now we are ready to call the
     peep hole optimizer */
  if (!options.nopeep)
    peepHole (&lineHead);

  /* now do the actual printing */
  printLine (lineHead, codeOutBuf);
  return;
}
