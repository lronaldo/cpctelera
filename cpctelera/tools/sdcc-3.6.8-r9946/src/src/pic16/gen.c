/*-------------------------------------------------------------------------
  gen.c - source file for code generation for pic16

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Bug Fixes  -  Wojciech Stryjewski  wstryj1@tiger.lsu.edu (1999 v2.1.9a)
  PIC port:
  Copyright (C) 2000, Scott Dattalo scott@dattalo.com
  PIC16 port:
  Copyright (C) 2002, Martin Dubuc m.dubuc@rogers.com
  Copyright (C) 2003-2006,Vangelis Rokas <vrokas AT users.sourceforge.net>
  Bug Fixes  -  Raphael Neider <rneider AT web.de> (2004,2005)
  Bug Fixes  -  Borut Razem <borut.razem AT siol.net> (2007)
  Bug Fixes  -  Mauro Giachero <maurogiachero AT users.sourceforge.net> (2008)

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
  000123 mlh    Moved aopLiteral to SDCCglue.c to help the split
                Made everything static
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "ralloc.h"
#include "pcode.h"
#include "gen.h"
#include "genutils.h"
#include "device.h"
#include "main.h"
#include "glue.h"
#include "dbuf_string.h"

/* The PIC port(s) do not need to distinguish between POINTER and FPOINTER. */
#define PIC_IS_DATA_PTR(x)      (IS_DATA_PTR(x) || IS_FARPTR(x))
#define PIC_IS_FARPTR(x)        (IS_DATA_PTR(x) || IS_FARPTR(x))
#define PIC_IS_TAGGED(x)        (IS_GENPTR(x) || IS_CODEPTR(x))
#define IS_DIRECT(op)           ((AOP_TYPE(op) == AOP_PCODE) && (AOP(op)->aopu.pcop->type == PO_DIR))

/* Wrapper to execute `code' at most once. */
#define PERFORM_ONCE(id,code)   do { static char id = 0; if (!id) { id = 1; code } } while (0)

void pic16_genMult8X8_n (operand *, operand *, operand *);
extern void pic16_printpBlock (FILE * of, pBlock * pb);
static asmop *newAsmop (short type);
static pCodeOp *pic16_popRegFromString (char *str, int size, int offset, operand * op);
extern pCode *pic16_newpCodeAsmDir (const char *asdir, const char *argfmt, ...);
static void mov2fp (pCodeOp * dst, asmop * src, int offset);
static pCodeOp *pic16_popRegFromIdx (int rIdx);
static void genCritical (iCode * ic);
static void genEndCritical (iCode * ic);

int pic16_labelOffset = 0;
extern int pic16_debug_verbose;

extern set *externs;

/* max_key keeps track of the largest label number used in
   a function. This is then used to adjust the label offset
   for the next function.
*/
static int max_key = 0;
static int GpseudoStkPtr = 0;

pCodeOp *pic16_popGetImmd (char *name, unsigned int offset, int index);

const char *pic16_AopType (short type);

void pic16_pushpCodeOp (pCodeOp * pcop);
void pic16_poppCodeOp (pCodeOp * pcop);


#define BYTEofLONG(l,b) ( (l>> (b<<3)) & 0xff)

/* set the following macro to 1 to enable passing the
 * first byte of functions parameters via WREG */
#define USE_WREG_IN_FUNC_PARAMS 0


/* this is the down and dirty file with all kinds of
   kludgy & hacky stuff. This is what it is all about
   CODE GENERATION for a specific MCU . some of the
   routines may be reusable, will have to see */
static char *zero = "#0x00";
static char *one = "#0x01";


/*
 * Function return value policy (MSB-->LSB):
 *  8 bits      -> WREG
 * 16 bits      -> PRODL:WREG
 * 24 bits      -> PRODH:PRODL:WREG
 * 32 bits      -> FSR0L:PRODH:PRODL:WREG
 * >32 bits     -> on stack, and FSR0 points to the beginning
 */
char *fReturnpic16[] = { "WREG", "PRODL", "PRODH", "FSR0L" };
int fReturnIdx[] = { IDX_WREG, IDX_PRODL, IDX_PRODH, IDX_FSR0L };

unsigned pic16_fReturnSizePic = 4;      /* shared with ralloc.c */
static char **fReturn = fReturnpic16;

static char *accUse[] = { "WREG" };

static struct
{
  short accInUse;
  short inLine;
  short debugLine;
  short nRegsSaved;
  set *sendSet;
  set *stackRegSet;
  int usefastretfie;
  bitVect *fregsUsed;           /* registers used in function */
  bitVect *sregsAlloc;
  set *sregsAllocSet;           /* registers used to store stack variables */
  int stack_lat;                /* stack offset latency */
  int resDirect;
  int useWreg;                  /* flag when WREG is used to pass function parameter */
} _G;

extern struct dbuf_s *codeOutBuf;

static unsigned char SLMask[] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0,
                                  0xE0, 0xC0, 0x80, 0x00
                                };

static unsigned char SRMask[] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F,
                                  0x07, 0x03, 0x01, 0x00
                                };

static pBlock *pb;

/*-----------------------------------------------------------------*/
/*  my_powof2(n) - If `n' is an integaer power of 2, then the      */
/*                 exponent of 2 is returned, otherwise -1 is      */
/*                 returned.                                       */
/* note that this is similar to the function `powof2' in SDCCsymt  */
/* if(n == 2^y)                                                    */
/*   return y;                                                     */
/* return -1;                                                      */
/*-----------------------------------------------------------------*/
int
pic16_my_powof2 (unsigned long num)
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
DEBUGpic16_pic16_AopType (int line_no, operand * left, operand * right, operand * result)
{
  DEBUGpic16_emitcode ("; ", "line = %d result %s=%s, left %s=%s, right %s=%s, size = %d",
                       line_no,
                       ((result) ? pic16_AopType (AOP_TYPE (result)) : "-"),
                       ((result) ? pic16_aopGet (AOP (result), 0, TRUE, FALSE) : "-"),
                       ((left) ? pic16_AopType (AOP_TYPE (left)) : "-"),
                       ((left) ? pic16_aopGet (AOP (left), 0, TRUE, FALSE) : "-"),
                       ((right) ? pic16_AopType (AOP_TYPE (right)) : "-"),
                       ((right) ? pic16_aopGet (AOP (right), 0, FALSE, FALSE) : "-"), ((result) ? AOP_SIZE (result) : 0));
}

void
DEBUGpic16_pic16_AopTypeSign (int line_no, operand * left, operand * right, operand * result)
{

  DEBUGpic16_emitcode ("; ", "line = %d, signs: result %s=%c, left %s=%c, right %s=%c",
                       line_no,
                       ((result) ? pic16_AopType (AOP_TYPE (result)) : "-"),
                       ((result) ? (SPEC_USIGN (operandType (result)) ? 'u' : 's') : '-'),
                         ((left) ? pic16_AopType (AOP_TYPE (left)) : "-"),
                         ((left) ? (SPEC_USIGN (operandType (left)) ? 'u' : 's') : '-'),
                         ((right) ? pic16_AopType (AOP_TYPE (right)) : "-"),
                         ((right) ? (SPEC_USIGN (operandType (right)) ? 'u' : 's') : '-'));

}

void
pic16_emitpcomment (char *fmt, ...)
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

  pic16_addpCode2pBlock (pb, pic16_newpCodeCharP (genLine.lineCurr->line));
}

void
DEBUGpic16_emitcode (char *inst, char *fmt, ...)
{
  va_list ap;

  if (!pic16_debug_verbose)
    return;

  va_start (ap, fmt);
  va_emitcode (inst, fmt, ap);
  va_end (ap);

  pic16_addpCode2pBlock (pb, pic16_newpCodeCharP (genLine.lineCurr->line));
}

void
pic16_emitpLabel (int key)
{
  if (key > max_key)
    max_key = key;

  pic16_addpCode2pBlock (pb, pic16_newpCodeLabel (NULL, labelKey2num (key + pic16_labelOffset)));
}

void
pic16_emitpLabelFORCE (int key)
{
  if (key > max_key)
    max_key = key;

  pic16_addpCode2pBlock (pb, pic16_newpCodeLabelFORCE (NULL, labelKey2num (key + pic16_labelOffset)));
}

/* gen.h defines a macro pic16_emitpcode that allows for debug information to be inserted on demand
 * NEVER call pic16_emitpcode_real directly, please... */
void
pic16_emitpcode_real (PIC_OPCODE poc, pCodeOp * pcop)
{

  if (pcop)
    pic16_addpCode2pBlock (pb, pic16_newpCode (poc, pcop));
  else
    DEBUGpic16_emitcode (";", "%s  ignoring NULL pcop", __FUNCTION__);
}

void
pic16_emitpinfo (INFO_TYPE itype, pCodeOp * pcop)
{
  if (pcop)
    pic16_addpCode2pBlock (pb, pic16_newpCodeInfo (itype, pcop));
  else
    DEBUGpic16_emitcode (";", "%s  ignoring NULL pcop", __FUNCTION__);
}

void
pic16_emitpcodeNULLop (PIC_OPCODE poc)
{

  pic16_addpCode2pBlock (pb, pic16_newpCode (poc, NULL));

}


#if 1
#define pic16_emitcode  DEBUGpic16_emitcode
#else
/*-----------------------------------------------------------------*/
/* pic16_emitcode - writes the code into a file : for now it is simple    */
/*-----------------------------------------------------------------*/
void
pic16_emitcode (char *inst, char *fmt, ...)
{
  va_list ap;
  char lb[INITIAL_INLINEASM];
  unsigned char *lbp = lb;

  va_start (ap, fmt);

  if (inst && *inst)
    {
      if (fmt && *fmt)
        SNPRINTF(lb, sizeof(lb), "%s\t", inst);
      else
        SNPRINTF(lb, sizeof(lb), "%s", inst);

      vsprintf (lb + (strlen (lb)), fmt, ap);
    }
  else
    vsprintf (lb, fmt, ap);

  while (isspace (*lbp))
    lbp++;

  if (lbp && *lbp)
    genLine.lineCurr = (genLine.lineCurr ?
                        connectLine (genLine.lineCurr, newLineNode (lb)) : (genLine.lineHead = newLineNode (lb)));
  genLine.lineCurr->isInline = genLine.lineElement.isInline;
  genLine.lineCurr->isDebug = genLine.lineElement.isDebug;
  genLine.lineCurr->isLabel = (lbp[strlen (lbp) - 1] == ':');
  genLine.lineCurr->isComment = (*lbp == ';');

// VR    fprintf(stderr, "lb = <%s>\n", lbp);

//    if(pic16_debug_verbose)
//      pic16_addpCode2pBlock(pb,pic16_newpCodeCharP(lb));

  va_end (ap);
}
#endif


/*-----------------------------------------------------------------*/
/* pic16_emitDebuggerSymbol - associate the current code location  */
/*   with a debugger symbol                                        */
/*-----------------------------------------------------------------*/
void
pic16_emitDebuggerSymbol (const char *debugSym)
{
  genLine.lineElement.isDebug = 1;
  pic16_emitcode (";", "%s ==.", debugSym);
  genLine.lineElement.isDebug = 0;
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
  FENTRY2;

//  DEBUGpic16_emitcode("; ***","%s %d",__FUNCTION__,__LINE__);

  if (!resIfx)
    return;


  resIfx->condition = 1;        /* assume that the ifx is true */
  resIfx->generated = 0;        /* indicate that the ifx has not been used */

  if (!ifx)
    {
      resIfx->lbl = newiTempLabel (NULL);       /* oops, there is no ifx. so create a label */

#if 1
      DEBUGpic16_emitcode ("; ***", "%s %d null ifx creating new label key =%d", __FUNCTION__, __LINE__, resIfx->lbl->key);
#endif

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
          resIfx->condition = 0;
        }

#if 1
      if (IC_TRUE (ifx))
        DEBUGpic16_emitcode ("; +++", "ifx true is non-null");
      else
        DEBUGpic16_emitcode ("; +++", "ifx true is null");
      if (IC_FALSE (ifx))
        DEBUGpic16_emitcode ("; +++", "ifx false is non-null");
      else
        DEBUGpic16_emitcode ("; +++", "ifx false is null");
#endif
    }

  DEBUGpic16_emitcode ("; ***", "%s lbl->key=%d, (lab offset=%d)", __FUNCTION__, resIfx->lbl->key, pic16_labelOffset);

}

#if 0
/*-----------------------------------------------------------------*/
/* pointerCode - returns the code for a pointer type               */
/*-----------------------------------------------------------------*/
static int
pointerCode (sym_link * etype)
{

  return PTR_TYPE (SPEC_OCLS (etype));

}
#endif

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (iCode * ic, operand * op, bool result)
{
  symbol *sym = OP_SYMBOL (op);
  asmop *aop;
  memmap *space = SPEC_OCLS (sym->etype);

  FENTRY2;

  _G.resDirect = 0;             /* clear flag that instructs the result is loaded directly from aopForSym */

//    sym = OP_SYMBOL(op);

  /* if already has one */
  if (sym->aop)
    {
      DEBUGpic16_emitcode ("; ***", "already has sym %s %d", __FUNCTION__, __LINE__);
      return sym->aop;
    }

#if 0
  /* if symbol was initially placed onStack then we must re-place it
   * to direct memory, since pic16 does not have a specific stack */
  if (sym->onStack)
    {
      fprintf (stderr, "%s:%d symbol %s on stack\n", __FILE__, __LINE__, OP_SYMBOL (op)->name);
    }
#endif


#if 0
  if (sym->iaccess)
    {
      if (space->paged)
        {
          fprintf (stderr, "%s:%d symbol %s points to paged data\n", __FILE__, __LINE__, sym->name);

          sym->aop = aop = newAsmop (AOP_PAGED);
          aop->aopu.aop_dir = sym->rname;
          aop->size = getSize (sym->type);
          DEBUGpic16_emitcode (";", "%d sym->rname = %s, size = %d", __LINE__, sym->rname, aop->size);
          pic16_allocDirReg (IC_LEFT (ic));
          return aop;
        }
      assert (0);
    }
#endif

#if 1
  /* assign depending on the storage class */
  /* if it is on the stack or indirectly addressable */
  /* space we need to assign either r0 or r1 to it   */
  if (sym->onStack)             // || sym->iaccess)
    {
      pCodeOp *pcop[4];
      int i;

      DEBUGpic16_emitcode ("; ***", "%s:%d sym->onStack:%d || sym->iaccess:%d",
                           __FUNCTION__, __LINE__, sym->onStack, sym->iaccess);

      /* acquire a temporary register -- it is saved in function */

      sym->aop = aop = newAsmop (AOP_STA);
      aop->aopu.stk.stk = sym->stack;
      aop->size = getSize (sym->type);


      DEBUGpic16_emitcode ("; +++ ", "%s:%d\top = %s", __FILE__, __LINE__, pic16_decodeOp (ic->op));
      if ((ic->op == '=' /*|| ic->op == CAST */ ) && IC_RESULT (ic) && AOP (IC_RESULT (ic))
          && (AOP_TYPE (IC_RESULT (ic)) == AOP_REG))
        {
//          pic16_DumpAop("aopForSym", AOP( IC_RESULT(ic) ));

          for (i = 0; i < aop->size; i++)
            aop->aopu.stk.pop[i] = pcop[i] = pic16_popRegFromIdx (AOP (IC_RESULT (ic))->aopu.aop_reg[i]->rIdx);
          _G.resDirect = 1;     /* notify that result will be loaded directly from aopForSym */
        }
      else if (1 && ic->op == SEND)
        {

          /* if SEND do the send here */
          _G.resDirect = 1;
        }
      else
        {
//                debugf3("symbol `%s' level = %d / %d\n", sym->name, ic->level, ic->seq);
          for (i = 0; i < aop->size; i++)
            {
              aop->aopu.stk.pop[i] = pcop[i] = pic16_popGetTempRegCond (_G.fregsUsed, _G.sregsAlloc, 0);
              _G.sregsAlloc = bitVectSetBit (_G.sregsAlloc, PCOR (pcop[i])->r->rIdx);
            }
        }


//        fprintf(stderr, "%s:%d\t%s\tsym size %d\n", __FILE__, __LINE__, __FUNCTION__, aop->size);

#if 1
      DEBUGpic16_emitcode (";", "%d sym->rname = %s, size = %d stack = %d", __LINE__, sym->rname, aop->size, sym->stack);

      // we do not need to load the value if it is to be defined...
      if (result)
        return aop;

      if (_G.accInUse)
        {
          pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_wreg));
        }

      for (i = 0; i < aop->size; i++)
        {

          /* initialise for stack access via frame pointer */
          // operands on stack are accessible via "{FRAME POINTER} + index" with index
          // starting at 2 for arguments and growing from 0 downwards for
          // local variables (index == 0 is not assigned so we add one here)
          {
            int soffs = sym->stack;
            if (soffs <= 0)
              {
                assert (soffs < 0);
                soffs++;
              }                 // if

            if (1 && ic->op == SEND)
              {
                pic16_emitpcode (POC_MOVLW, pic16_popGetLit (soffs + aop->size - i - 1 /*+ _G.stack_lat */ ));
                pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (pic16_frame_plusw),
                                 pic16_popCopyReg (pic16_stack_postdec)));
              }
            else
              {
                pic16_emitpcode (POC_MOVLW, pic16_popGetLit (soffs + i /*+ _G.stack_lat */ ));
                pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (pic16_frame_plusw), pcop[i]));
              }
          }
        }

      if (_G.accInUse)
        {
          pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_wreg));
        }

      return (aop);
#endif

#if 0
      /* now assign the address of the variable to
         the pointer register */
      if (aop->type != AOP_STK)
        {

          if (sym->onStack)
            {
              if (_G.accInUse)
                pic16_emitcode ("push", "acc");

              pic16_emitcode ("mov", "a,_bp");
              pic16_emitcode ("add", "a,#0x%02x",
                              ((sym->stack < 0) ? ((char) (sym->stack - _G.nRegsSaved)) : ((char) sym->stack)) & 0xff);
              pic16_emitcode ("mov", "%s,a", aop->aopu.aop_ptr->name);

              if (_G.accInUse)
                pic16_emitcode ("pop", "acc");
            }
          else
            pic16_emitcode ("mov", "%s,#%s", aop->aopu.aop_ptr->name, sym->rname);
          aop->paged = space->paged;
        }
      else
        aop->aopu.aop_stk = sym->stack;
      return aop;
#endif

    }
#endif

#if 1
  /* special case for a function */
  if (IS_FUNC (sym->type))
    {
      sym->aop = aop = newAsmop (AOP_PCODE);
      aop->aopu.pcop = pic16_popGetImmd (sym->rname, 0, 0);
      PCOI (aop->aopu.pcop)->_const = IN_CODESPACE (space);
      PCOI (aop->aopu.pcop)->index = 0;
      aop->size = FPTRSIZE;
      DEBUGpic16_emitcode (";", "%d size = %d, name =%s", __LINE__, aop->size, sym->rname);
      return aop;
    }
#endif



  //DEBUGpic16_emitcode(";","%d",__LINE__);
  /* if in bit space */
  if (IN_BITSPACE (space))
    {
      sym->aop = aop = newAsmop (AOP_CRY);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
      DEBUGpic16_emitcode (";", "%d sym->rname = %s, size = %d", __LINE__, sym->rname, aop->size);
      return aop;
    }
  /* if it is in direct space */
  if (IN_DIRSPACE (space))
    {
      if (!strcmp (sym->rname, "_WREG"))
        {
          sym->aop = aop = newAsmop (AOP_ACC);
          aop->size = getSize (sym->type);      /* should always be 1 */
          assert (aop->size == 1);
          DEBUGpic16_emitcode (";", "%d sym->rname (AOP_ACC) = %s, size = %d", __LINE__, sym->rname, aop->size);
          return (aop);
        }
      else
        {
          sym->aop = aop = newAsmop (AOP_DIR);
          aop->aopu.aop_dir = sym->rname;
          aop->size = getSize (sym->type);
          DEBUGpic16_emitcode (";", "%d sym->rname (AOP_DIR) = %s, size = %d", __LINE__, sym->rname, aop->size);
          pic16_allocDirReg (IC_LEFT (ic));
          return (aop);
        }
    }

  if (IN_FARSPACE (space) && !IN_CODESPACE (space))
    {
      sym->aop = aop = newAsmop (AOP_DIR);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
      DEBUGpic16_emitcode (";", "%d sym->rname = %s, size = %d", __LINE__, sym->rname, aop->size);
      pic16_allocDirReg (IC_LEFT (ic));
      return aop;
    }


  /* only remaining is far space */
  sym->aop = aop = newAsmop (AOP_PCODE);

  /* change the next if to 1 to revert to good old immediate code */
  if (IN_CODESPACE (space))
    {
      aop->aopu.pcop = pic16_popGetImmd (sym->rname, 0, 0);
      PCOI (aop->aopu.pcop)->_const = IN_CODESPACE (space);
      PCOI (aop->aopu.pcop)->index = 0;
    }
  else
    {
      /* try to allocate via direct register */
      aop->aopu.pcop = pic16_popRegFromString (sym->rname, getSize (sym->type), sym->offset, op);       // Patch 8
//              aop->size = getSize( sym->type );
    }

  DEBUGpic16_emitcode (";", "%d: rname %s, val %d, const = %d", __LINE__, sym->rname, 0, PCOI (aop->aopu.pcop)->_const);

#if 0
  if (!pic16_allocDirReg (IC_LEFT (ic)))
    return NULL;
#endif

  if (IN_DIRSPACE (space))
    aop->size = PTRSIZE;
  else if (IN_CODESPACE (space) || IN_FARSPACE (space))
    aop->size = FPTRSIZE;
  else if (IC_LEFT (ic) && AOP (IC_LEFT (ic)))
    aop->size = AOP_SIZE (IC_LEFT (ic));
  else if (IC_RIGHT (ic) && AOP (IC_RIGHT (ic)))
    aop->size = AOP_SIZE (IC_RIGHT (ic));
  else if (sym->onStack)
    {
      aop->size = PTRSIZE;
    }
  else
    {
      if (SPEC_SCLS (sym->etype) == S_PDATA)
        {
          fprintf (stderr, "%s: %d symbol in PDATA space\n", __FILE__, __LINE__);
          aop->size = FPTRSIZE;
        }
      else
        assert (0);
    }

  DEBUGpic16_emitcode (";", "%d size = %d", __LINE__, aop->size);

  /* if it is in code space */
  if (IN_CODESPACE (space))
    aop->code = 1;

  return aop;
}

/*-----------------------------------------------------------------*/
/* aopForRemat - rematerialzes an object                           */
/*-----------------------------------------------------------------*/
static asmop *
aopForRemat (operand * op, bool result) // x symbol *sym)
{
  symbol *sym = OP_SYMBOL (op);
  operand *refop;
  iCode *ic = NULL;
  asmop *aop = newAsmop (AOP_PCODE);
  int val = 0;
  int viaimmd = 0;

  FENTRY2;

  ic = sym->rematiCode;

  if (IS_OP_POINTER (op))
    {
      DEBUGpic16_emitcode (";", "%s %d IS_OP_POINTER", __FUNCTION__, __LINE__);
    }

//    if(!result)               /* fixme-vr */
  for (;;)
    {
//              chat *iLine = printILine(ic);
//              pic16_emitpcomment("ic: %s\n", iLine);
//              dbuf_free(iLine);

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

  refop = IC_LEFT (ic);

  if (!op->isaddr)
    viaimmd++;
  else
    viaimmd = 0;

  /* set the following if to 1 to revert to good old immediate code */
  if (IN_CODESPACE (SPEC_OCLS (OP_SYM_ETYPE (refop))) || viaimmd)
    {

      DEBUGpic16_emitcode (";", "%s:%d immediate, size: %d", __FILE__, __LINE__, getSize (sym->type));

      aop->aopu.pcop = pic16_popGetImmd (OP_SYMBOL (IC_LEFT (ic))->rname, 0, val);

#if 0
      PCOI (aop->aopu.pcop)->_const = IS_PTR_CONST (operandType (op));
#else
      PCOI (aop->aopu.pcop)->_const = IS_CODEPTR (operandType (op));
#endif

      PCOI (aop->aopu.pcop)->index = val;

      aop->size = getSize (sym->type);
    }
  else
    {
      DEBUGpic16_emitcode (";", "%s:%d dir size: %d", __FILE__, __LINE__, getSize (OP_SYMBOL (IC_LEFT (ic))->type));

      aop->aopu.pcop = pic16_popRegFromString (OP_SYMBOL (IC_LEFT (ic))->rname,
                       getSize (OP_SYMBOL (IC_LEFT (ic))->type), val, op);

      aop->size = getSize (OP_SYMBOL (IC_LEFT (ic))->type);
    }


  DEBUGpic16_emitcode (";", "%d: rname %s, val %d, const = %d", __LINE__, OP_SYMBOL (IC_LEFT (ic))->rname,
#if 0
                       val, IS_PTR_CONST (operandType (op)));
#else
                       val, IS_CODEPTR (operandType (op)));
#endif

//      DEBUGpic16_emitcode(";","aop type  %s",pic16_AopType(AOP_TYPE(IC_LEFT(ic))));

  pic16_allocDirReg (IC_LEFT (ic));

  if (IN_CODESPACE (SPEC_OCLS (OP_SYM_ETYPE (op))))
    aop->code = 1;

  return aop;
}

#if 0
static int
aopIdx (asmop * aop, int offset)
{
  if (!aop)
    return -1;

  if (aop->type != AOP_REG)
    return -2;

  return aop->aopu.aop_reg[offset]->rIdx;

}
#endif

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
  if (IS_ITEMP (op1) && !IS_ITEMP (op2) && sym1->isspilt && (SYM_SPIL_LOC (sym1) == sym2))
    return TRUE;

  if (IS_ITEMP (op2) && !IS_ITEMP (op1) && sym2->isspilt && sym1->level > 0 && (SYM_SPIL_LOC (sym2) == sym1))
    return TRUE;

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* pic16_sameRegs - two asmops have the same registers                   */
/*-----------------------------------------------------------------*/
bool
pic16_sameRegs (asmop * aop1, asmop * aop2)
{
  int i;

  if (aop1 == aop2)
    return TRUE;

  DEBUGpic16_emitcode (";***", "%s aop1->type = %s\taop2->type = %s\n", __FUNCTION__,
                       pic16_AopType (aop1->type), pic16_AopType (aop2->type));

  if (aop1->type == AOP_ACC && aop2->type == AOP_ACC)
    return TRUE;

  if (aop1->type != AOP_REG || aop2->type != AOP_REG)
    return FALSE;

  /* This is a bit too restrictive if one is a subset of the other...
     if (aop1->size != aop2->size )
     return FALSE ;
   */

  for (i = 0; i < min (aop1->size, aop2->size); i++)
    {
//        if(aop1->aopu.aop_reg[i]->type != aop2->aopu.aop_reg[i]->type)return FALSE;

//        if(aop1->aopu.aop_reg[i]->type == AOP_REG)
      if (strcmp (aop1->aopu.aop_reg[i]->name, aop2->aopu.aop_reg[i]->name))
        return FALSE;
    }

  return TRUE;
}

bool
pic16_sameRegsOfs (asmop * aop1, asmop * aop2, int offset)
{
  DEBUGpic16_emitcode (";***", "%s aop1->type = %s\taop2->type = %s (offset = %d)\n", __FUNCTION__,
                       pic16_AopType (aop1->type), pic16_AopType (aop2->type), offset);

  if (aop1 == aop2)
    return TRUE;
  if (aop1->type != AOP_REG || aop2->type != AOP_REG)
    return FALSE;

  if (strcmp (aop1->aopu.aop_reg[offset]->name, aop2->aopu.aop_reg[offset]->name))
    return FALSE;

  return TRUE;
}


/*-----------------------------------------------------------------*/
/* pic16_aopOp - allocates an asmop for an operand  :                    */
/*-----------------------------------------------------------------*/
void
pic16_aopOp (operand * op, iCode * ic, bool result)
{
  asmop *aop;
  symbol *sym;
  int i;

  if (!op)
    return;

  DEBUGpic16_emitcode (";", "%s %d", __FUNCTION__, __LINE__);

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
#if 0
    if (IS_PTR_CONST (type))
#else
    if (IS_CODEPTR (type))
#endif
      DEBUGpic16_emitcode (";", "%d aop type is const pointer", __LINE__);
  }

  /* if already has a asmop then continue */
  if (op->aop)
    return;

  /* if the underlying symbol has a aop */
  if (IS_SYMOP (op) && OP_SYMBOL (op)->aop)
    {
      DEBUGpic16_emitcode (";", "%d has symbol", __LINE__);
      op->aop = OP_SYMBOL (op)->aop;
      return;
    }

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      DEBUGpic16_emitcode (";", "%d - true symop", __LINE__);
      op->aop = aopForSym (ic, op, result);
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

  DEBUGpic16_emitcode ("; ***", "%d: symbol name = %s, regType = %d", __LINE__, sym->name, sym->regType);
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

//      debugf3("symbol %s\tisspilt: %d\tnRegs: %d\n", sym->rname, sym->isspilt, sym->nRegs);
      DEBUGpic16_emitcode (";", "%d", __LINE__);
      /* rematerialize it NOW */
      if (sym->remat)
        {

          sym->aop = op->aop = aop = aopForRemat (op, result);
          return;
        }

#if 1
      if (sym->accuse)
        {
          int i;
          aop = op->aop = sym->aop = newAsmop (AOP_ACC);
          aop->size = getSize (sym->type);
          for (i = 0; i < 1; i++)
            {
              aop->aopu.aop_str[i] = accUse[i];
//                aop->aopu.pcop = pic16_popRegFromString("WREG", aop->size, SYM_SPIL_LOC(sym)->offset);
            }
          fprintf (stderr, "%s:%d allocating AOP_ACC for sym= %s\n", __FILE__, __LINE__, sym->name);
          DEBUGpic16_emitcode (";", "%d size=%d", __LINE__, aop->size);
          return;
        }
#endif

#if 1
      if (sym->ruonly)
        {
          /*
             sym->aop = op->aop = aop = newAsmop(AOP_PCODE);
             aop->aopu.pcop = pic16_popGetImmd(SYM_SPIL_LOC(sym)->rname,0,SYM_SPIL_LOC(sym)->offset);
             //pic16_allocDirReg (IC_LEFT(ic));
             aop->size = getSize(sym->type);
           */

          unsigned i;

          aop = op->aop = sym->aop = newAsmop (AOP_REG);
          aop->size = getSize (sym->type);
          for (i = 0; i < pic16_fReturnSizePic; i++)
            aop->aopu.aop_reg[i] = PCOR (pic16_popRegFromIdx (fReturnIdx[i]))->r;

          DEBUGpic16_emitcode (";", "%d", __LINE__);
          return;
        }
#endif
      /* else spill location  */
      if (sym->isspilt && SYM_SPIL_LOC (sym) && getSize (sym->type) != getSize (SYM_SPIL_LOC (sym)->type))
        {
          /* force a new aop if sizes differ */
          SYM_SPIL_LOC (sym)->aop = NULL;
        }

#if 0
      DEBUGpic16_emitcode (";", "%s %d %s sym->rname = %s, offset %d",
                           __FUNCTION__, __LINE__, SYM_SPIL_LOC (sym)->rname, sym->rname, SYM_SPIL_LOC (sym)->offset);
#endif

      //aop->aopu.pcop = pic16_popGetImmd(SYM_SPIL_LOC(sym)->rname,0,SYM_SPIL_LOC(sym)->offset);
      if (sym->isspilt && SYM_SPIL_LOC (sym) && SYM_SPIL_LOC (sym)->rname)
        {
          sym->aop = op->aop = aop = newAsmop (AOP_PCODE);
          aop->aopu.pcop = pic16_popRegFromString (SYM_SPIL_LOC (sym)->rname,
                           getSize (sym->type), SYM_SPIL_LOC (sym)->offset, op);
        }
      else if (getSize (sym->type) <= 1)
        {
          //fprintf (stderr, "%s:%d called for a spillLocation -- assigning WREG instead --- CHECK (size:%u)!\n", __FUNCTION__, __LINE__, getSize(sym->type));
          pic16_emitpcomment (";!!! %s:%d called for a spillLocation -- assigning WREG instead --- CHECK", __FUNCTION__,
                              __LINE__);
          assert (getSize (sym->type) <= 1);
          sym->aop = op->aop = aop = newAsmop (AOP_PCODE);
          aop->aopu.pcop = pic16_popCopyReg (&pic16_pc_wreg);
        }
      else
        {
          /* We need some kind of dummy area for getSize(sym->type) byte,
           * use WREG for all storage locations.
           * XXX: This only works if we are implementing a `dummy read',
           *      the stored value will not be retrievable...
           *      See #1503234 for a case requiring this. */
          sym->aop = op->aop = aop = newAsmop (AOP_REG);
          aop->size = getSize (sym->type);
          for (i = 0; i < aop->size; i++)
            aop->aopu.aop_reg[i] = pic16_pc_wreg.r;
        }
      aop->size = getSize (sym->type);

      return;
    }

  {
    sym_link *type = operandType (op);
#if 0
    if (IS_PTR_CONST (type))
#else
    if (IS_CODEPTR (type))
#endif
      DEBUGpic16_emitcode (";", "%d aop type is const pointer", __LINE__);
  }

  /* must be in a register */
  DEBUGpic16_emitcode (";", "%d register type nRegs=%d", __LINE__, sym->nRegs);
  sym->aop = op->aop = aop = newAsmop (AOP_REG);
  aop->size = sym->nRegs;
  for (i = 0; i < sym->nRegs; i++)
    aop->aopu.aop_reg[i] = sym->regs[i];
}

/*-----------------------------------------------------------------*/
/* pic16_freeAsmop - free up the asmop given to an operand               */
/*----------------------------------------------------------------*/
void
pic16_freeAsmop (operand * op, asmop * aaop, iCode * ic, bool pop)
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

#if 1
  switch (aop->type)
    {
    case AOP_STA:
    {
      int i;

      /* we must store the result on stack */
      if ((op == IC_RESULT (ic)) && RESULTONSTA (ic))
        {
          // operands on stack are accessible via "FSR2 + index" with index
          // starting at 2 for arguments and growing from 0 downwards for
          // local variables (index == 0 is not assigned so we add one here)
          int soffs = OP_SYMBOL (IC_RESULT (ic))->stack;
          if (soffs <= 0)
            {
              assert (soffs < 0);
              soffs++;
            }                 // if
          if (_G.accInUse)
            pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_wreg));
          for (i = 0; i < aop->size; i++)
            {
              /* initialise for stack access via frame pointer */
              pic16_emitpcode (POC_MOVLW, pic16_popGetLit (soffs + i /*+ _G.stack_lat */ ));
              pic16_emitpcode (POC_MOVFF, pic16_popGet2p (aop->aopu.stk.pop[i], pic16_popCopyReg (pic16_frame_plusw)));
            }

          if (_G.accInUse)
            pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_wreg));
        }

      if (!_G.resDirect)
        {
          for (i = 0; i < aop->size; i++)
            {
              PCOR (aop->aopu.stk.pop[i])->r->isFree = 1;

              if (bitVectBitValue (_G.sregsAlloc, PCOR (aop->aopu.stk.pop[i])->r->rIdx))
                {
                  bitVectUnSetBit (_G.sregsAlloc, PCOR (aop->aopu.stk.pop[i])->r->rIdx);
//                      pic16_popReleaseTempReg(aop->aopu.stk.pop[i], 0);
                }
            }

          if (_G.sregsAllocSet)
            {
              reg_info *sr;

              _G.sregsAllocSet = reverseSet (_G.sregsAllocSet);
              for (sr = setFirstItem (_G.sregsAllocSet); sr; sr = setFirstItem (_G.sregsAllocSet))
                {
                  pic16_poppCodeOp (pic16_popRegFromIdx (sr->rIdx));
                  deleteSetItem (&_G.sregsAllocSet, sr);
                }
            }
        }
      _G.resDirect = 0;
    }
    break;
#if 0
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
          pic16_emitcode ("mov", "a,_bp");
          pic16_emitcode ("add", "a,#0x%02x", ((char) stk) & 0xff);
          pic16_emitcode ("mov", "%s,a", aop->aopu.aop_ptr->name);
        }
      else
        {
          pic16_emitcode ("mov", "%s,_bp", aop->aopu.aop_ptr->name);
        }

      while (sz--)
        {
          pic16_emitcode ("pop", "acc");
          pic16_emitcode ("mov", "@%s,a", aop->aopu.aop_ptr->name);
          if (!sz)
            break;
          pic16_emitcode ("dec", "%s", aop->aopu.aop_ptr->name);
        }
      op->aop = aop;
      pic16_freeAsmop (op, NULL, ic, TRUE);
      if (_G.r0Pushed)
        {
          pic16_emitcode ("pop", "ar0");
          _G.r0Pushed--;
        }

      if (_G.r1Pushed)
        {
          pic16_emitcode ("pop", "ar1");
          _G.r1Pushed--;
        }
    }
#endif

    }
#endif

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

/*-----------------------------------------------------------------*/
/* pic16_aopGet - for fetching value of the aop                          */
/*-----------------------------------------------------------------*/
char *
pic16_aopGet (asmop * aop, int offset, bool bit16, bool dname)
{
  //DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);

  /* offset is greater than size then zero */
  if (offset > (aop->size - 1) && aop->type != AOP_LIT)
    return zero;

  /* depending on type */
  switch (aop->type)
    {
    case AOP_DIR:
      if (offset > 0)
        {
          SNPRINTF(buffer, sizeof(buffer), "(%s + %d)", aop->aopu.aop_dir, offset);
          DEBUGpic16_emitcode (";", "oops AOP_DIR did this %s\n", buffer);
          return Safe_strdup(buffer);
        }
      else
        return Safe_strdup(aop->aopu.aop_dir);

    case AOP_REG:
      return aop->aopu.aop_reg[offset]->name;

    case AOP_CRY:
      return aop->aopu.aop_dir;

    case AOP_ACC:
      DEBUGpic16_emitcode (";Warning -pic port ignoring get(AOP_ACC)", "%d\toffset: %d", __LINE__, offset);
//        fprintf(stderr, "%s:%d Warning -pic port ignoring get(AOP_ACC)\n",__FILE__, __LINE__);
//        assert( 0 );
//      return aop->aopu.aop_str[offset];       //->"AOP_accumulator_bug";
      return Safe_strdup("WREG");

    case AOP_LIT:
      SNPRINTF(buffer, sizeof(buffer), "0x%02x", pic16aopLiteral (aop->aopu.aop_lit, offset));
      return Safe_strdup(buffer);

    case AOP_STR:
      aop->coff = offset;

//      if (strcmp(aop->aopu.aop_str[offset],"a") == 0 &&
//          dname)
//          return "acc";
      if (!strcmp (aop->aopu.aop_str[offset], "WREG"))
        {
          aop->type = AOP_ACC;
          return Safe_strdup ("_WREG");
        }
      DEBUGpic16_emitcode (";", "%d - %s", __LINE__, aop->aopu.aop_str[offset]);
      return aop->aopu.aop_str[offset];

    case AOP_PCODE:
    {
      pCodeOp *pcop = aop->aopu.pcop;
      DEBUGpic16_emitcode (";", "%d: pic16_aopGet AOP_PCODE type %s", __LINE__, pic16_pCodeOpType (pcop));
      if (pcop->name)
        {
          DEBUGpic16_emitcode (";", "%s offset %d", pcop->name, PCOI (pcop)->offset);
          //SNPRINTF(buffer, sizeof(buffer), "(%s+0x%02x)", pcop->name, PCOI(aop->aopu.pcop)->offset);
          if (offset > 0)
            {
              SNPRINTF(buffer, sizeof(buffer), "(%s + %d)", pic16_get_op (pcop, NULL, 0), offset);
            }
          else
            {
              SNPRINTF(buffer, sizeof(buffer), "%s", pic16_get_op (pcop, NULL, 0));
            }
        }
      else
        SNPRINTF(buffer, sizeof(buffer), "0x%02x", PCOI (aop->aopu.pcop)->offset);

    }
    return Safe_strdup(buffer);

#if 0
    case AOP_PAGED:
      DEBUGpic16_emitcode (";", "oops AOP_PAGED did this %s\n", s);
      if (offset)
        {
          SNPRINTF(buffer, sizeof(buffer), "(%s + %d)", aop->aopu.aop_dir, offset);
        }
      else
        SNPRINTF(buffer, sizeof(buffer), "%s", aop->aopu.aop_dir);
      DEBUGpic16_emitcode (";", "oops AOP_PAGED did this %s\n", s);
      return Safe_strdup(buffer);
#endif

    case AOP_STA:
      return Safe_strdup(PCOR(aop->aopu.stk.pop[offset])->r->name);

    case AOP_STK:
//        pCodeOp *pcop = aop->aop
      break;

    }

  fprintf (stderr, "%s:%d unsupported aop->type: %s\n", __FILE__, __LINE__, pic16_AopType (aop->type));
  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopget got unsupported aop->type");
  exit (0);
}



/* lock has the following meaning: When allocating temporary registers
 * for stack variables storage, the value of the temporary register is
 * saved on stack. Its value is restored at the end. This procedure is
 * done via calls to pic16_aopOp and pic16_freeAsmop functions. There is
 * a possibility that before a call to pic16_aopOp, a temporary register
 * is allocated for a while and it is freed after some time, this will
 * mess the stack and values will not be restored properly. So use lock=1
 * to allocate temporary registers used internally by the programmer, and
 * lock=0 to allocate registers for stack use. lock=1 will emit a warning
 * to inform the compiler developer about a possible bug. This is an internal
 * feature for developing the compiler -- VR */

int _TempReg_lock = 0;
/*-----------------------------------------------------------------*/
/* pic16_popGetTempReg - create a new temporary pCodeOp                  */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popGetTempReg (int lock)
{
  pCodeOp *pcop = NULL;
  symbol *cfunc;

//    DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
  if (_TempReg_lock)
    {
//      werror(W_POSSBUG2, __FILE__, __LINE__);
    }

  _TempReg_lock += lock;

  cfunc = currFunc;
  currFunc = NULL;

  pcop = pic16_newpCodeOp (NULL, PO_GPR_TEMP);
  if (pcop && pcop->type == PO_GPR_TEMP && PCOR (pcop)->r)
    {
      PCOR (pcop)->r->wasUsed = 1;
      PCOR (pcop)->r->isFree = 0;

      /* push value on stack */
      pic16_pushpCodeOp (pic16_pCodeOpCopy (pcop));
    }

  currFunc = cfunc;

  return pcop;
}

/*-----------------------------------------------------------------*/
/* pic16_popGetTempRegCond - create a new temporary pCodeOp which  */
/*                           is not part of f, but don't save if   */
/*                           inside v                              */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popGetTempRegCond (bitVect * f, bitVect * v, int lock)
{
  pCodeOp *pcop = NULL;
  symbol *cfunc;
  int i;

//    DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);

  if (_TempReg_lock)
    {
//      werror(W_POSSBUG2, __FILE__, __LINE__);
    }

  _TempReg_lock += lock;

  cfunc = currFunc;
  currFunc = NULL;

  i = bitVectFirstBit (f);
  while (i < 128)
    {

      /* bypass registers that are used by function */
      if (!bitVectBitValue (f, i))
        {

          /* bypass registers that are already allocated for stack access */
          if (!bitVectBitValue (v, i))
            {

//          debugf("getting register rIdx = %d\n", i);
              /* ok, get the operand */
              pcop = pic16_newpCodeOpReg (i);

              /* should never by NULL */
              assert (pcop != NULL);


              /* sanity check */
              if (pcop && pcop->type == PO_GPR_TEMP && PCOR (pcop)->r)
                {
                  int found = 0;

                  PCOR (pcop)->r->wasUsed = 1;
                  PCOR (pcop)->r->isFree = 0;


                  {
                    reg_info *sr;

                    for (sr = setFirstItem (_G.sregsAllocSet); sr; sr = setNextItem (_G.sregsAllocSet))
                      {

                        if (sr->rIdx == PCOR (pcop)->r->rIdx)
                          {
                            /* already used in previous steps, break */
                            found = 1;
                            break;
                          }
                      }
                  }

                  /* caller takes care of the following */
//              bitVectSetBit(v, i);

                  if (!found)
                    {
                      /* push value on stack */
                      pic16_pushpCodeOp (pic16_pCodeOpCopy (pcop));
                      addSet (&_G.sregsAllocSet, PCOR (pcop)->r);
                    }

                  break;
                }
            }
        }
      i++;
    }

  currFunc = cfunc;

  return pcop;
}


/*-----------------------------------------------------------------*/
/* pic16_popReleaseTempReg - create a new temporary pCodeOp                  */
/*-----------------------------------------------------------------*/
void
pic16_popReleaseTempReg (pCodeOp * pcop, int lock)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  _TempReg_lock -= lock;

  if (pcop && pcop->type == PO_GPR_TEMP && PCOR (pcop)->r)
    {
      PCOR (pcop)->r->isFree = 1;

      pic16_poppCodeOp (pic16_pCodeOpCopy (pcop));
    }
}

/*-----------------------------------------------------------------*/
/* pic16_popGetLabel - create a new pCodeOp of type PO_LABEL             */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popGetLabel (int key)
{

  DEBUGpic16_emitcode ("; ***", "%s  key=%d, label offset %d", __FUNCTION__, key, pic16_labelOffset);

  if (key > max_key)
    max_key = key;

  return pic16_newpCodeOpLabel (NULL, labelKey2num (key + pic16_labelOffset));
}

/*-----------------------------------------------------------------*/
/* pic16_popCopyReg - copy a pcode operator                              */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popCopyReg (pCodeOpReg * pc)
{
  pCodeOpReg *pcor;

  pcor = Safe_malloc(sizeof(pCodeOpReg));
  memcpy(pcor, pc, sizeof(pCodeOpReg));
  pcor->r->wasUsed = TRUE;

  //pcor->pcop.type = pc->pcop.type;
  if (pc->pcop.name)
    {
      if (!(pcor->pcop.name = Safe_strdup (pc->pcop.name)))
        fprintf (stderr, "oops %s %d", __FILE__, __LINE__);
    }
  else
    pcor->pcop.name = NULL;

  //pcor->r = pc->r;
  //pcor->rIdx = pc->rIdx;
  //pcor->r->wasUsed=1;
  //pcor->instance = pc->instance;

//  DEBUGpic16_emitcode ("; ***","%s  , copying %s, rIdx=%d",__FUNCTION__,pc->pcop.name,pc->rIdx);

  return PCOP (pcor);
}

/*-----------------------------------------------------------------*/
/* pic16_popGetLit - asm operator to pcode operator conversion     */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popGetLit (int lit)
{
  return pic16_newpCodeOpLit (lit);
}

/* Allow for 12 bit literals (LFSR x, <here!>). */
pCodeOp *
pic16_popGetLit12 (int lit)
{
  return pic16_newpCodeOpLit12 (lit);
}

/*-----------------------------------------------------------------*/
/* pic16_popGetLit2 - asm operator to pcode operator conversion    */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popGetLit2 (int lit, pCodeOp * arg2)
{
  return pic16_newpCodeOpLit2 (lit, arg2);
}


/*-----------------------------------------------------------------*/
/* pic16_popGetImmd - asm operator to pcode immediate conversion   */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popGetImmd (char *name, unsigned int offset, int index)
{
  return pic16_newpCodeOpImmd (name, offset, index, 0);
}


/*-----------------------------------------------------------------*/
/* pic16_popGet - asm operator to pcode operator conversion              */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popGetWithString (char *str)
{
  pCodeOp *pcop;


  if (!str)
    {
      fprintf (stderr, "NULL string %s %d\n", __FILE__, __LINE__);
      exit (1);
    }

  pcop = pic16_newpCodeOp (str, PO_STR);

  return pcop;
}

/*-----------------------------------------------------------------*/
/* pic16_popRegFromString -                                        */
/*-----------------------------------------------------------------*/
static pCodeOp *
pic16_popRegFromString (char *str, int size, int offset, operand * op)
{

  pCodeOp *pcop = Safe_alloc(sizeof(pCodeOpReg));
  pcop->type = PO_DIR;

  DEBUGpic16_emitcode (";", "%d %s %s %d/%d", __LINE__, __FUNCTION__, str, size, offset);       // patch 14
  // fprintf(stderr, "%s:%d: register name = %s pos = %d/%d\n", __FUNCTION__, __LINE__, str, offset, size);

  if (!str)
    str = "BAD_STRING";

  pcop->name = Safe_strdup(str);

  //pcop->name = Safe_strdup( ( (str) ? str : "BAD STRING"));

  PCOR (pcop)->r = pic16_dirregWithName (pcop->name);
//  PCOR(pcop)->r->wasUsed = 1;

  /* make sure that register doesn't exist,
   * and operand isn't NULL
   * and symbol isn't in codespace (codespace symbols are handled elsewhere) */
  if ((PCOR (pcop)->r == NULL) && (op) && !IN_CODESPACE (SPEC_OCLS (OP_SYM_ETYPE (op))))
    {
//      fprintf(stderr, "%s:%d - couldn't find %s in allocated regsters, size= %d ofs= %d\n",
//              __FUNCTION__, __LINE__, str, size, offset);

      PCOR (pcop)->r = pic16_allocRegByName (pcop->name, size, op);
      //fprintf(stderr, "%s:%d: WARNING: need to allocate new register by name -> %s\n", __FILE__, __LINE__, str);

    }
  PCOR (pcop)->instance = offset;

  return pcop;
}

static pCodeOp *
pic16_popRegFromIdx (int rIdx)
{
  pCodeOp *pcop;

//      DEBUGpic16_emitcode ("; ***","%s,%d\trIdx=0x%x", __FUNCTION__,__LINE__,rIdx);
//      fprintf(stderr, "%s:%d rIdx = 0x%0x\n", __FUNCTION__, __LINE__, rIdx);

  pcop = Safe_alloc(sizeof(pCodeOpReg));
  PCOR (pcop)->rIdx = rIdx;
  PCOR (pcop)->r = pic16_regWithIdx (rIdx);
  if (!PCOR (pcop)->r)
    PCOR (pcop)->r = pic16_allocWithIdx (rIdx);

  PCOR (pcop)->r->isFree = 0;
  PCOR (pcop)->r->wasUsed = 1;

  pcop->type = PCOR (pcop)->r->pc_type;

  return pcop;
}

/*---------------------------------------------------------------------------------*/
/* pic16_popGet2 - a variant of pic16_popGet to handle two memory operand commands */
/*                 VR 030601                                                       */
/*---------------------------------------------------------------------------------*/
pCodeOp *
pic16_popGet2 (asmop * aop_src, asmop * aop_dst, int offset)
{
  pCodeOp2 *pcop2 = (pCodeOp2 *) pic16_newpCodeOp2 (pic16_popGet (aop_src, offset), pic16_popGet (aop_dst, offset));
  return PCOP (pcop2);
}

/*--------------------------------------------------------------------------------.-*/
/* pic16_popGet2p - a variant of pic16_popGet to handle two memory operand commands */
/*                  VR 030601 , adapted by Hans Dorn                                */
/*--------------------------------------------------------------------------------.-*/
pCodeOp *
pic16_popGet2p (pCodeOp * src, pCodeOp * dst)
{
  pCodeOp2 *pcop2;
  pcop2 = (pCodeOp2 *) pic16_newpCodeOp2 (src, dst);
  return PCOP (pcop2);
}

/*---------------------------------------------------------------------------------*/
/* pic16_popCombine2 - combine two pCodeOpReg variables into one for use with      */
/*                     movff instruction                                           */
/*---------------------------------------------------------------------------------*/
pCodeOp *
pic16_popCombine2 (pCodeOpReg * src, pCodeOpReg * dst, int noalloc)
{
  pCodeOp2 *pcop2 = (pCodeOp2 *) pic16_newpCodeOp2 (pic16_popCopyReg (src), pic16_popCopyReg (dst));

  return PCOP (pcop2);
}

/*-----------------------------------------------------------------*/
/* pic16_popGet - asm operator to pcode operator conversion              */
/*-----------------------------------------------------------------*/
pCodeOp *
pic16_popGet (asmop * aop, int offset)  //, bool bit16, bool dname)
{
//  char *s = buffer ;
//  char *rs;
  pCodeOp *pcop;

  FENTRY2;

  /* offset is greater than size then zero */

//    if (offset > (aop->size - 1) &&
//        aop->type != AOP_LIT)
//      return NULL;  //zero;

  /* depending on type */
  switch (aop->type)
    {
    case AOP_STA:
      /* pCodeOp is already allocated from aopForSym */
      DEBUGpic16_emitcode (";---", "%d getting stack + offset %d\n", __LINE__, offset);
      pcop = pic16_pCodeOpCopy (aop->aopu.stk.pop[offset]);
      return (pcop);

    case AOP_ACC:
    {
      int rIdx = IDX_WREG;    //aop->aopu.aop_reg[offset]->rIdx;

//      fprintf (stderr, "%s:%d returning register AOP_ACC %s\n", __FILE__, __LINE__, aop->aopu.aop_str[offset]);

      DEBUGpic16_emitcode (";", "%d\tAOP_ACC", __LINE__);

      pcop = Safe_alloc(sizeof(pCodeOpReg));
      PCOR (pcop)->rIdx = rIdx;
      PCOR (pcop)->r = pic16_typeRegWithIdx (rIdx, REG_SFR, 1);       // pic16_regWithIdx(rIdx);
      PCOR (pcop)->r->wasUsed = 1;
      PCOR (pcop)->r->isFree = 0;

      PCOR (pcop)->instance = offset;
      pcop->type = PCOR (pcop)->r->pc_type;
//                              DEBUGpic16_emitcode(";","%d register idx = %d name =%s",__LINE__,rIdx,rs);
      return pcop;


//      return pic16_popRegFromString(aop->aopu.aop_str[offset], aop->size, offset);
//      return pic16_newpCodeOpRegFromStr(aop->aopu.aop_str[offset]);

//      assert( 0 );
    }

    case AOP_DIR:
      DEBUGpic16_emitcode (";", "%d\tAOP_DIR (name = %s)", __LINE__, aop->aopu.aop_dir);
      return pic16_popRegFromString (aop->aopu.aop_dir, aop->size, offset, NULL);

#if 0
    case AOP_PAGED:
      DEBUGpic16_emitcode (";", "%d\tAOP_DIR", __LINE__);
      return pic16_popRegFromString (aop->aopu.aop_dir, aop->size, offset, NULL);
#endif

    case AOP_REG:
    {
      int rIdx;

//      debugf2("aop = %p\toffset = %d\n", aop, offset);
//      assert (aop && aop->aopu.aop_reg[offset] != NULL);
      rIdx = aop->aopu.aop_reg[offset]->rIdx;

      DEBUGpic16_emitcode (";", "%d\tAOP_REG", __LINE__);

      pcop = Safe_alloc(sizeof(pCodeOpReg));
//      pcop->type = PO_GPR_REGISTER;
      PCOR (pcop)->rIdx = rIdx;
      PCOR (pcop)->r = pic16_allocWithIdx (rIdx);     //pic16_regWithIdx(rIdx);
      PCOR (pcop)->r->wasUsed = 1;
      PCOR (pcop)->r->isFree = 0;

      PCOR (pcop)->instance = offset;
      pcop->type = PCOR (pcop)->r->pc_type;

      DEBUGpic16_emitcode (";*+*", "%d\tAOP_REG type = %s", __LINE__, dumpPicOptype (pcop->type));
//      rs = aop->aopu.aop_reg[offset]->name;
//      DEBUGpic16_emitcode(";","%d register idx = %d name = %s",__LINE__,rIdx,rs);
      return pcop;
    }

    case AOP_CRY:
      DEBUGpic16_emitcode (";", "%d\tAOP_CRY", __LINE__);

      pcop = pic16_newpCodeOpBit (aop->aopu.aop_dir, -1, 1, PO_GPR_REGISTER);
      PCOR (pcop)->instance = offset;
      PCOR (pcop)->r = pic16_dirregWithName (aop->aopu.aop_dir);
      //if(PCOR(pcop)->r == NULL)
      //fprintf(stderr,"%d - couldn't find %s in allocated registers\n",__LINE__,aop->aopu.aop_dir);
      return pcop;

    case AOP_LIT:
      DEBUGpic16_emitcode (";", "%d\tAOP_LIT", __LINE__);
      return pic16_newpCodeOpLit (pic16aopLiteral (aop->aopu.aop_lit, offset));

    case AOP_STR:
      DEBUGpic16_emitcode (";", "%d AOP_STR %s", __LINE__, aop->aopu.aop_str[offset]);
      return pic16_newpCodeOpRegFromStr (aop->aopu.aop_str[offset]);

      /*
         pcop = Safe_alloc(sizeof(pCodeOpReg));
         PCOR(pcop)->r = pic16_allocRegByName(aop->aopu.aop_str[offset]);
         PCOR(pcop)->rIdx = PCOR(pcop)->r->rIdx;
         pcop->type = PCOR(pcop)->r->pc_type;
         pcop->name = PCOR(pcop)->r->name;

         return pcop;
       */

    case AOP_PCODE:
      DEBUGpic16_emitcode (";", "pic16_popGet AOP_PCODE (%s) %d %s offset %d", pic16_pCodeOpType (aop->aopu.pcop),
                           __LINE__, ((aop->aopu.pcop->name) ? (aop->aopu.pcop->name) : "no name"), offset);
      pcop = pic16_pCodeOpCopy (aop->aopu.pcop);
      switch (aop->aopu.pcop->type)
        {
        case PO_DIR:
          PCOR (pcop)->instance += offset;
          break;
        case PO_IMMEDIATE:
          PCOI (pcop)->offset = offset;
          break;
        case PO_WREG:
          assert (offset == 0);
          break;
        default:
          fprintf (stderr, "%s: unhandled aop->aopu.pcop->type %d\n", __FUNCTION__, aop->aopu.pcop->type);
          assert (0); /* should never reach here */ ;
        }
      return pcop;
    }

  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "pic16_popGet got unsupported aop->type");
  exit (0);
}

/*-------------------------------------------------------------------------*/
/* pic16_popGetImmed - immediate asm operator to pcode operator conversion */
/*-------------------------------------------------------------------------*/
static pCodeOp *
pic16_popGetImmed(asmop *aop, int offset, int overload)
{
  pCodeOp *pcop;

  FENTRY2;

  if (aop->type == AOP_PCODE)
    {
      if (aop->aopu.pcop->type == PO_IMMEDIATE)
        {
          pcop = pic16_pCodeOpCopy(aop->aopu.pcop);
          PCOI(pcop)->offset = offset;  // offset: "LOW", "HIGH", "UPPER"
          PCOI(pcop)->index += overload;  // LOW(ptr + index + overload)
          return pcop;
        }

      fprintf(stderr, "%s: Only handled PO_IMMEDIATE.\n", __FUNCTION__);
      assert(0);
    }

  werror(E_INTERNAL_ERROR, __FILE__, __LINE__, "pic16_popGetImmed supported only AOP_PCODE.");
  exit(0);
}

/*-----------------------------------------------------------------*/
/* pic16_aopPut - puts a string for a aop                                */
/*-----------------------------------------------------------------*/
void
pic16_aopPut (asmop * aop, char *s, int offset)
{
  symbol *lbl;

  return;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (aop->size && offset > (aop->size - 1))
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "pic16_aopPut got offset > aop->size");
      exit (0);
    }

  /* will assign value to value */
  /* depending on where it is ofcourse */
  switch (aop->type)
    {
    case AOP_DIR:
      if (offset > 0)
        {
          SNPRINTF(buffer, sizeof(buffer), "(%s + %d)", aop->aopu.aop_dir, offset);
          fprintf (stderr, "oops pic16_aopPut:AOP_DIR did this %s\n", s);
        }
      else
        SNPRINTF(buffer, sizeof(buffer), "%s", aop->aopu.aop_dir);

      if (strcmp(buffer, s))
        {
          DEBUGpic16_emitcode (";", "%d", __LINE__);
          if (strcmp (s, "W"))
            pic16_emitcode ("movf", "%s,w", s);

          pic16_emitcode ("movwf", "%s", buffer);

          if (strcmp (s, "W"))
            {
              pic16_emitcode (";BUG!? should have this:movf", "%s,w   %d", s, __LINE__);
              if (offset >= aop->size)
                {
                  pic16_emitpcode (POC_CLRF, pic16_popGet (aop, offset));
                  break;
                }
              else
                pic16_emitpcode (POC_MOVLW, pic16_popGetImmd (s, offset, 0));
            }

          pic16_emitpcode (POC_MOVWF, pic16_popGet (aop, offset));


        }
      break;

    case AOP_REG:
      if (strcmp (aop->aopu.aop_reg[offset]->name, s) != 0)     // &&
        {
          //strcmp(aop->aopu.aop_reg[offset]->dname,s)!= 0){
          /*
             if (*s == '@'           ||
             strcmp(s,"r0") == 0 ||
             strcmp(s,"r1") == 0 ||
             strcmp(s,"r2") == 0 ||
             strcmp(s,"r3") == 0 ||
             strcmp(s,"r4") == 0 ||
             strcmp(s,"r5") == 0 ||
             strcmp(s,"r6") == 0 ||
             strcmp(s,"r7") == 0 )
             pic16_emitcode("mov","%s,%s  ; %d",
             aop->aopu.aop_reg[offset]->dname,s,__LINE__);
             else
           */

          if (strcmp (s, "W") == 0)
            pic16_emitcode ("movf", "%s,w  ; %d", s, __LINE__);

          pic16_emitcode ("movwf", "%s", aop->aopu.aop_reg[offset]->name);

          if (strcmp (s, zero) == 0)
            {
              pic16_emitpcode (POC_CLRF, pic16_popGet (aop, offset));

            }
          else if (strcmp (s, "W") == 0)
            {
              pCodeOp *pcop = Safe_alloc(sizeof(pCodeOpReg));
              pcop->type = PO_GPR_REGISTER;

              PCOR (pcop)->rIdx = -1;
              PCOR (pcop)->r = NULL;

              DEBUGpic16_emitcode (";", "%d", __LINE__);
              pcop->name = Safe_strdup (s);
              pic16_emitpcode (POC_MOVFW, pcop);
              pic16_emitpcode (POC_MOVWF, pic16_popGet (aop, offset));
            }
          else if (strcmp (s, one) == 0)
            {
              pic16_emitpcode (POC_CLRF, pic16_popGet (aop, offset));
              pic16_emitpcode (POC_INCF, pic16_popGet (aop, offset));
            }
          else
            {
              pic16_emitpcode (POC_MOVWF, pic16_popGet (aop, offset));
            }
        }
      break;

    case AOP_STK:
      if (strcmp (s, "a") == 0)
        pic16_emitcode ("push", "acc");
      else
        pic16_emitcode ("push", "%s", s);
      break;

    case AOP_CRY:
      /* if bit variable */
      if (!aop->aopu.aop_dir)
        {
          pic16_emitcode ("clr", "a");
          pic16_emitcode ("rlc", "a");
        }
      else
        {
          if (s == zero)
            pic16_emitcode ("clr", "%s", aop->aopu.aop_dir);
          else if (s == one)
            pic16_emitcode ("setb", "%s", aop->aopu.aop_dir);
          else if (!strcmp (s, "c"))
            pic16_emitcode ("mov", "%s,c", aop->aopu.aop_dir);
          else
            {
              lbl = newiTempLabel (NULL);

              if (strcmp (s, "a"))
                {
                  MOVA (s);
                }
              pic16_emitcode ("clr", "c");
              pic16_emitcode ("jz", "%05d_DS_", labelKey2num (lbl->key));
              pic16_emitcode ("cpl", "c");
              pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));
              pic16_emitcode ("mov", "%s,c", aop->aopu.aop_dir);
            }
        }
      break;

    case AOP_STR:
      aop->coff = offset;
      if (strcmp (aop->aopu.aop_str[offset], s))
        pic16_emitcode ("mov", "%s,%s ; %d", aop->aopu.aop_str[offset], s, __LINE__);
      break;

    case AOP_ACC:
      aop->coff = offset;
      if (!offset && (strcmp (s, "acc") == 0))
        break;

      if (strcmp (aop->aopu.aop_str[offset], s))
        pic16_emitcode ("mov", "%s,%s ; %d", aop->aopu.aop_str[offset], s, __LINE__);
      break;

    default:
      fprintf (stderr, "%s:%d: unknown aop->type = 0x%x\n", __FILE__, __LINE__, aop->type);
//      werror(E_INTERNAL_ERROR,__FILE__,__LINE__,
//             "pic16_aopPut got unsupported aop->type");
//      exit(0);
    }

}

/*-----------------------------------------------------------------*/
/* pic16_mov2w - generate either a MOVLW or MOVFW based operand type     */
/*-----------------------------------------------------------------*/
void
pic16_mov2w (asmop * aop, int offset)
{
  int isWREG = (aop->type != AOP_STA && ! strcmp (pic16_aopGet(aop, 0, TRUE, FALSE), "WREG"));

  DEBUGpic16_emitcode ("; ***", "%s  %d  offset=%d", __FUNCTION__, __LINE__, offset);

  if (pic16_isLitAop (aop))
    pic16_emitpcode (POC_MOVLW, pic16_popGet (aop, offset));
  else if (isWREG)
    DEBUGpic16_emitcode ("; ***", "ignore MOVF\tWREG, W", __FUNCTION__, __LINE__);
  else
    pic16_emitpcode (POC_MOVFW, pic16_popGet (aop, offset));
}

void
pic16_mov2w_volatile (asmop * aop)
{
  int i;

  if (!pic16_isLitAop (aop))
    {
      // may need to protect this from the peepholer -- this is not nice but works...
      pic16_addpCode2pBlock (pb, pic16_newpCodeAsmDir (";", "VOLATILE READ - BEGIN"));
      for (i = 0; i < aop->size; i++)
        {
          if (i > 0)
            {
              pic16_addpCode2pBlock (pb, pic16_newpCodeAsmDir (";", "VOLATILE READ - MORE"));
            }                   // if
          pic16_emitpcode (POC_MOVFW, pic16_popGet (aop, i));
        }                       // for
      pic16_addpCode2pBlock (pb, pic16_newpCodeAsmDir (";", "VOLATILE READ - END"));
    }
}

void
pic16_mov2f (asmop * dst, asmop * src, int offset)
{
  if (pic16_isLitAop (src))
    {
      int dstIsWREG = (dst->type != AOP_STA && ! strcmp(pic16_aopGet(dst, 0, TRUE, FALSE), "WREG"));

      pic16_emitpcode (POC_MOVLW, pic16_popGet (src, offset));

      if (! dstIsWREG)
        {
          pic16_emitpcode (POC_MOVWF, pic16_popGet(dst, offset));
        }
    }
  else
    {
      int srcIsWREG;

      if (pic16_sameRegsOfs (src, dst, offset))
        return;

      srcIsWREG = (src->type != AOP_STA && ! strcmp (pic16_aopGet (src, 0, TRUE, FALSE), "WREG"));

      if (srcIsWREG)
        {
          pic16_emitpcode (POC_MOVWF, pic16_popGet (dst, offset));
        }
      else
        {
          int dstIsINTCON = (src->type != AOP_STA && ! strcmp (pic16_aopGet(src, 0, TRUE, FALSE), "INTCON"));
          int dstIsPCL = (src->type != AOP_STA && ! strcmp (pic16_aopGet(src, 0, TRUE, FALSE), "PCL"));

          if (dstIsINTCON || dstIsPCL)
            {
              pic16_emitpcode (POC_MOVFW, pic16_popGet (src, offset));
              pic16_emitpcode (POC_MOVWF, pic16_popGet (dst, offset));
            }
          else
            {
              pic16_emitpcode(POC_MOVFF, pic16_popGet2p (pic16_popGet (src, offset),
                              pic16_popGet (dst, offset)));
            }
        }
    }
}

static void
pic16_movLit2f (pCodeOp * pc, int lit)
{
  if (0 == (lit & 0x00ff))
    {
      pic16_emitpcode (POC_CLRF, pc);
    }
  else if (0xff == (lit & 0x00ff))
    {
      pic16_emitpcode (POC_SETF, pc);
    }
  else
    {
      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (lit & 0x00ff));
      if (pc->type != PO_WREG)
        pic16_emitpcode (POC_MOVWF, pc);
    }
}

static void
mov2fp (pCodeOp * dst, asmop * src, int offset)
{
  if (pic16_isLitAop (src))
    {
      pic16_emitpcode (POC_MOVLW, pic16_popGet (src, offset));

      if (dst->type != PO_WREG)
        {
          pic16_emitpcode (POC_MOVWF, dst);
        }
    }
  else
    {
      if (dst->type == PO_INTCON || dst->type == PO_PCL)
        {
          pic16_emitpcode (POC_MOVFW, pic16_popGet (src, offset));
          pic16_emitpcode (POC_MOVWF, dst);
        }
      else
        {
          pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (src, offset), dst));
        }
    }
}

void
pic16_testStackOverflow (void)
{
#define GSTACK_TEST_NAME        "_gstack_test"

  pic16_emitpcode (POC_CALL, pic16_popGetWithString (GSTACK_TEST_NAME));

  {
    symbol *sym;

    sym = newSymbol (GSTACK_TEST_NAME, 0);
    SNPRINTF(sym->rname, sizeof(sym->rname), "%s", /*port->fun_prefix, */ GSTACK_TEST_NAME);
//      strcpy(sym->rname, GSTACK_TEST_NAME);
    checkAddSym (&externs, sym);
  }
}

/* push pcop into stack */
void
pic16_pushpCodeOp (pCodeOp * pcop)
{
//      DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
  if (pcop->type == PO_LITERAL)
    {
      pic16_emitpcode (POC_MOVLW, pcop);
      pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (pic16_stack_postdec));
    }
  else
    {
      pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pcop, pic16_popCopyReg (pic16_stack_postdec)));
    }

  if (pic16_options.gstack)
    pic16_testStackOverflow ();
}

/* pop pcop from stack */
void
pic16_poppCodeOp (pCodeOp * pcop)
{
  pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (pic16_stack_preinc), pcop));
  if (pic16_options.gstack)
    pic16_testStackOverflow ();
}


/*-----------------------------------------------------------------*/
/* pushw - pushes wreg to stack                                    */
/*-----------------------------------------------------------------*/
void
pushw (void)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (pic16_stack_postdec));
  if (pic16_options.gstack)
    pic16_testStackOverflow ();
}


/*-----------------------------------------------------------------*/
/* pushaop - pushes aop to stack                                   */
/*-----------------------------------------------------------------*/
void
pushaop (asmop * aop, int offset)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (_G.resDirect)
    return;

  if (pic16_isLitAop (aop))
    {
      pic16_emitpcode (POC_MOVLW, pic16_popGet (aop, offset));
      pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (pic16_stack_postdec));
    }
  else
    {
      pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (aop, offset), pic16_popCopyReg (pic16_stack_postdec)));
    }

  if (pic16_options.gstack)
    pic16_testStackOverflow ();
}

/*-----------------------------------------------------------------*/
/* popaop - pops aop from stack                                    */
/*-----------------------------------------------------------------*/
void
popaop (asmop * aop, int offset)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  pic16_emitpcode (POC_MOVFF, pic16_popCombine2 (pic16_stack_preinc, PCOR (pic16_popGet (aop, offset)), 0));
  if (pic16_options.gstack)
    pic16_testStackOverflow ();
}

void
popaopidx (asmop * aop, int offset, int index)
{
  int ofs = 1;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (STACK_MODEL_LARGE)
    ofs++;

  pic16_emitpcode (POC_MOVLW, pic16_popGetLit (index + ofs));
  pic16_emitpcode (POC_MOVFF, pic16_popCombine2 (pic16_frame_plusw, PCOR (pic16_popGet (aop, offset)), 0));
  if (pic16_options.gstack)
    pic16_testStackOverflow ();
}

/*-----------------------------------------------------------------*/
/* pic16_getDataSize - get the operand data size                         */
/*-----------------------------------------------------------------*/
int
pic16_getDataSize (operand * op)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);


  return AOP_SIZE (op);

  // tsd- in the pic port, the genptr size is 1, so this code here
  // fails. ( in the 8051 port, the size was 4).
#if 0
  int size;
  size = AOP_SIZE (op);
  if (size == GPTRSIZE)
    {
      sym_link *type = operandType (op);
      if (IS_GENPTR (type))
        {
          /* generic pointer; arithmetic operations
           * should ignore the high byte (pointer type).
           */
          size--;
          DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
        }
    }
  return size;
#endif
}

/*-----------------------------------------------------------------*/
/* pic16_outAcc - output Acc                                             */
/*-----------------------------------------------------------------*/
void
pic16_outAcc (operand * result)
{
  int size, offset;
  DEBUGpic16_emitcode ("; ***", "%s  %d - ", __FUNCTION__, __LINE__);
  DEBUGpic16_pic16_AopType (__LINE__, NULL, NULL, result);


  size = pic16_getDataSize (result);
  if (size)
    {
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
      size--;
      offset = 1;
      /* unsigned or positive */
      while (size--)
        pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset++));
    }

}

/*-----------------------------------------------------------------*/
/* pic16_outBitC - output a bit C                                  */
/*                 Move to result the value of Carry flag -- VR    */
/*-----------------------------------------------------------------*/
void
pic16_outBitC (operand * result)
{
  int i;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  /* if the result is bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      fprintf (stderr, "%s:%d: pic16 port warning: unsupported case\n", __FILE__, __LINE__);
      pic16_aopPut (AOP (result), "c", 0);
    }
  else
    {

      i = AOP_SIZE (result);
      while (i--)
        {
          pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), i));
        }
      pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), 0));
    }
}

/*-----------------------------------------------------------------*/
/* pic16_outBitOp - output a bit from Op                           */
/*                 Move to result the value of set/clr op -- VR    */
/*-----------------------------------------------------------------*/
void
pic16_outBitOp (operand * result, pCodeOp * pcop)
{
  int i;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  /* if the result is bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      fprintf (stderr, "%s:%d: pic16 port warning: unsupported case\n", __FILE__, __LINE__);
      pic16_aopPut (AOP (result), "c", 0);
    }
  else
    {

      i = AOP_SIZE (result);
      while (i--)
        {
          pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), i));
        }
      pic16_emitpcode (POC_RRCF, pcop);
      pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), 0));
    }
}

/*-----------------------------------------------------------------*/
/* pic16_toBoolean - emit code for orl a,operator(sizeop)                */
/*-----------------------------------------------------------------*/
void
pic16_toBoolean (operand * oper)
{
  int size = AOP_SIZE (oper) - 1;
  int offset = 1;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (AOP_TYPE (oper) != AOP_ACC)
    {
      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (oper), 0));
    }
  while (size--)
    {
      pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (oper), offset++));
    }
}

/*-----------------------------------------------------------------*/
/* genUminusFloat - unary minus for floating points                */
/*-----------------------------------------------------------------*/
static void
genUminusFloat (operand * op, operand * result)
{
  int size, offset = 0;

  FENTRY;
  /* for this we just need to flip the
     first it then copy the rest in place */
  size = AOP_SIZE (op);
  assert (size == AOP_SIZE (result));

  while (size--)
    {
      pic16_mov2f (AOP (result), AOP (op), offset);
      offset++;
    }

  /* toggle the MSB's highest bit */
  pic16_emitpcode (POC_BTG, pic16_popCopyGPR2Bit (pic16_popGet (AOP (result), offset - 1), 7));
}

/*-----------------------------------------------------------------*/
/* genUminus - unary minus code generation                         */
/*-----------------------------------------------------------------*/
static void
genUminus (iCode * ic)
{
  int lsize, rsize, i;
  sym_link *optype;
  symbol *label;
  int needLabel = 0;

  FENTRY;

  /* assign asmops */
  pic16_aopOp (IC_LEFT (ic), ic, FALSE);
  pic16_aopOp (IC_RESULT (ic), ic, TRUE);

  /* if both in bit space then special case */
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_CRY && AOP_TYPE (IC_LEFT (ic)) == AOP_CRY)
    {

      pic16_emitpcode (POC_BCF, pic16_popGet (AOP (IC_RESULT (ic)), 0));
      pic16_emitpcode (POC_BTFSS, pic16_popGet (AOP (IC_LEFT (ic)), 0));
      pic16_emitpcode (POC_BSF, pic16_popGet (AOP (IC_RESULT (ic)), 0));
      goto release;
    }

  optype = operandType (IC_LEFT (ic));

  /* if float then do float stuff */
  if (IS_FLOAT (optype) || IS_FIXED (optype))
    {
      if (IS_FIXED (optype))
        debugf ("implement fixed16x16 type\n", 0);

      genUminusFloat (IC_LEFT (ic), IC_RESULT (ic));
      goto release;
    }

  /* otherwise subtract from zero by taking the 2's complement */
  lsize = AOP_SIZE (IC_LEFT (ic));
  rsize = AOP_SIZE (IC_RESULT (ic));
  label = newiTempLabel (NULL);

  if (pic16_sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
    {
      /* If the result is longer than the operand,
         store sign extension (0x00 or 0xff) in W */
      if (rsize > lsize)
        {
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0x00));
          pic16_emitpcode (POC_BTFSS, pic16_popCopyGPR2Bit (pic16_popGet (AOP (IC_LEFT (ic)), lsize - 1), 7));
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xFF));
        }
      for (i = rsize - 1; i > 0; --i)
        {
          if (i > lsize - 1)
            {
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (IC_RESULT (ic)), i));
            }
          else
            {
              pic16_emitpcode (POC_COMF, pic16_popGet (AOP (IC_RESULT (ic)), i));
            }                   // if
        }                       // for
      pic16_emitpcode (POC_NEGF, pic16_popGet (AOP (IC_RESULT (ic)), 0));
      for (i = 1; i < rsize; ++i)
        {
          if (i == rsize - 1)
            {
              emitSKPNZ;
            }
          else
            {
              pic16_emitpcode (POC_BNZ, pic16_popGetLabel (label->key));
              needLabel++;
            }
          pic16_emitpcode (POC_INCF, pic16_popGet (AOP (IC_RESULT (ic)), i));
        }                       // for
    }
  else
    {
      for (i = min (rsize, lsize) - 1; i >= 0; i--)
        {
          pic16_emitpcode (POC_COMFW, pic16_popGet (AOP (IC_LEFT (ic)), i));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (IC_RESULT (ic)), i));
        }                       // for
      /* Sign extend if the result is longer than the operand */
      if (rsize > lsize)
        {
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0x00));
          pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (IC_RESULT (ic)), lsize - 1), 7));
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xFF));
          for (i = rsize - 1; i > lsize - 1; --i)
            {
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (IC_RESULT (ic)), i));
            }                   // for
        }                       // if
      if (rsize > 1)
        {
          for (i = 0; i < rsize - 2; i++)
            {
              pic16_emitpcode (POC_INCF, pic16_popGet (AOP (IC_RESULT (ic)), i));
              pic16_emitpcode (POC_BNZ, pic16_popGetLabel (label->key));
              needLabel++;
            }                   // for
          pic16_emitpcode (POC_INFSNZ, pic16_popGet (AOP (IC_RESULT (ic)), rsize - 2));
        }                       // if
      pic16_emitpcode (POC_INCF, pic16_popGet (AOP (IC_RESULT (ic)), rsize - 1));
    }
  if (needLabel)
    pic16_emitpLabel (label->key);

release:
  /* release the aops */
  pic16_freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? 0 : 1));
  pic16_freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
}

void
pic16_loadFromReturn (operand * op, int offset, pCodeOp * src)
{
  if ((AOP (op)->type == AOP_PCODE) && (AOP (op)->aopu.pcop->type == PO_IMMEDIATE))
    {
      pic16_emitpcode (POC_MOVFW, src);
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (op), offset));
    }
  else
    {
      pic16_emitpcode (POC_MOVFF, pic16_popGet2p (src, pic16_popGet (AOP (op), offset)));
    }
}

/*-----------------------------------------------------------------*/

static void
free_stack_parameters (int Parameter_count)
{
  if (Parameter_count < 1)
    {
      return;
    }

  if (Parameter_count == 1)
    {
      pic16_emitpcode (POC_MOVF, pic16_popCopyReg (pic16_stack_postinc));
    }
  else if (Parameter_count == 2)
    {
      pic16_emitpcode (POC_MOVF, pic16_popCopyReg (pic16_stack_postinc));
      pic16_emitpcode (POC_MOVF, pic16_popCopyReg (pic16_stack_postinc));
    }
  else
    {
      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (Parameter_count));
      pic16_emitpcode (POC_ADDWF, pic16_popCopyReg (pic16_stackpnt_lo));  // &pic16_pc_fsr1l));

      if (STACK_MODEL_LARGE)
        {
          emitSKPNC;
          pic16_emitpcode (POC_INCF, pic16_popCopyReg (pic16_stackpnt_hi)); // &pic16_pc_fsr1h));
        }
    }
}

/*-----------------------------------------------------------------*/
/* assignResultValue - assign results to oper, rescall==1 is       */
/*                     called from genCall() or genPcall()         */
/*-----------------------------------------------------------------*/
static void
assignResultValue (operand * oper, int res_size, int rescall)
{
  int size = AOP_SIZE (oper);
  int offset = 0;

  FENTRY2;
//    DEBUGpic16_emitcode ("; ***","%s  %d rescall:%d size:%d",__FUNCTION__,__LINE__,rescall,size); // patch 14
  DEBUGpic16_pic16_AopType (__LINE__, oper, NULL, NULL);

  if (rescall)
    {
      /* assign result from a call/pcall function() */

      /* function results are stored in a special order,
       * see top of file with Function return policy, or manual */

      if (size <= 4)
        {
          /* 8-bits, result in WREG */
          if (AOP_TYPE(oper) != AOP_ACC)
            {
              /* If destination NOT WREG. */
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (oper), 0));
            }
          else
            {
              DEBUGpic16_emitcode ("; ***", "ignore MOVWF\tWREG", __FUNCTION__, __LINE__);
            }

          if (size > 1 && res_size > 1)
            {
              /* 16-bits, result in PRODL:WREG */
              pic16_loadFromReturn (oper, 1, pic16_popCopyReg (&pic16_pc_prodl));
            }

          if (size > 2 && res_size > 2)
            {
              /* 24-bits, result in PRODH:PRODL:WREG */
              pic16_loadFromReturn (oper, 2, pic16_popCopyReg (&pic16_pc_prodh));       // patch 14
            }

          if (size > 3 && res_size > 3)
            {
              /* 32-bits, result in FSR0L:PRODH:PRODL:WREG */
              pic16_loadFromReturn (oper, 3, pic16_popCopyReg (&pic16_pc_fsr0l));       // patch14
            }

          pic16_addSign (oper, res_size, IS_UNSIGNED (operandType (oper)));

        }
      else
        {
          /* >32-bits, result on stack, and FSR0 points to beginning.
           * Fix stack when done */
          /* FIXME FIXME */
//      debugf("WARNING: Possible bug when returning more than 4-bytes\n");
          while (size--)
            {
//          DEBUGpic16_emitcode("; ", "POC_MOVLW %d", GpseudoStkPtr);
//          DEBUGpic16_emitcode("; ", "POC_MOVFW PLUSW2");

              popaopidx (AOP (oper), size, GpseudoStkPtr);
              GpseudoStkPtr++;
            }

          /* fix stack */
          free_stack_parameters (AOP_SIZE (oper));
        }
    }
  else
    {
      int areg = 0;             /* matching argument register */

//      debugf("_G.useWreg = %d\tGpseudoStkPtr = %d\n", _G.useWreg, GpseudoStkPtr);
      areg = SPEC_ARGREG (OP_SYM_ETYPE (oper)) - 1;


      /* its called from genReceive (probably) -- VR */
      /* I hope this code will not be called from somewhere else in the future!
       * We manually set the pseudo stack pointer in genReceive. - dw
       */
      if (!GpseudoStkPtr && _G.useWreg)
        {
//        DEBUGpic16_emitcode("; ", "pop %d", GpseudoStkPtr);

          /* The last byte in the assignment is in W */
          if (areg <= GpseudoStkPtr)
            {
              size--;

              if (AOP_TYPE(oper) != AOP_ACC)
                {
                  /* If destination NOT WREG. */
                  pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (oper), offset /*size*/));
                }
              else
                {
                  DEBUGpic16_emitcode ("; ***", "ignore MOVWF\tWREG", __FUNCTION__, __LINE__);
                }

              offset++;
//          debugf("receive from WREG\n", 0);
            }
          GpseudoStkPtr++;      /* otherwise the calculation below fails (-_G.useWreg) */
        }
//      GpseudoStkPtr++;
      _G.stack_lat = AOP_SIZE (oper) - 1;

      while (size)
        {
          size--;
          GpseudoStkPtr++;
          popaopidx (AOP (oper), offset, GpseudoStkPtr - _G.useWreg);
//        debugf("receive from STACK\n", 0);
          offset++;
        }
    }
}


/*-----------------------------------------------------------------*/
/* genIpush - generate code for pushing this gets a little complex */
/*-----------------------------------------------------------------*/
static void
genIpush (iCode * ic)
{
//  int size, offset=0;

  FENTRY;
  DEBUGpic16_emitcode ("; ***", "%s  %d - WARNING no code generated", __FUNCTION__, __LINE__);

  if (ic->parmPush)
    {
      pic16_aopOp (IC_LEFT (ic), ic, FALSE);

      /* send to stack as normal */
      addSet (&_G.sendSet, ic);
//    addSetHead(&_G.sendSet,ic);
      pic16_freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);
    }


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

      pic16_aopOp (IC_LEFT (ic), ic, FALSE);
      size = AOP_SIZE (IC_LEFT (ic));
      /* push it on the stack */
      while (size--)
        {
          l = pic16_aopGet (AOP (IC_LEFT (ic)), offset++, FALSE, TRUE);
          if (*l == '#')
            {
              MOVA (l);
              l = "acc";
            }
          pic16_emitcode ("push", "%s", l);
        }
      return;
    }

  pic16_freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);
#endif
}

/*-----------------------------------------------------------------*/
/* genIpop - recover the registers: can happen only for spilling   */
/*-----------------------------------------------------------------*/
static void
genIpop (iCode * ic)
{
  FENTRY;
  DEBUGpic16_emitcode ("; ***", "%s  %d - WARNING no code generated", __FUNCTION__, __LINE__);
#if 0
  int size, offset;


  /* if the temp was not pushed then */
  if (OP_SYMBOL (IC_LEFT (ic))->isspilt)
    return;

  pic16_aopOp (IC_LEFT (ic), ic, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));
  offset = (size - 1);
  while (size--)
    pic16_emitcode ("pop", "%s", pic16_aopGet (AOP (IC_LEFT (ic)), offset--, FALSE, TRUE));

  pic16_freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);
#endif
}

static int
wparamCmp (void *p1, void *p2)
{
  return (!strcmp ((char *) p1, (char *) p2));
}

int
inWparamList (char *s)
{
  return isinSetWith (wparamList, s, wparamCmp);
}


/*-----------------------------------------------------------------*/
/* genCall - generates a call statement                            */
/*-----------------------------------------------------------------*/
static void
genCall (iCode * ic)
{
  sym_link *ftype;
  int stackParms = 0;
  int use_wreg = 0;
  int inwparam = 0;
  char *fname;

  FENTRY;

  ftype = OP_SYM_TYPE (IC_LEFT (ic));
  /* if caller saves & we have not saved then */
//    if (!ic->regsSaved)
//      saveRegisters(ic);

  /* initialise stackParms for IPUSH pushes */
//      stackParms = pseudoStkPtr;
//      fprintf(stderr, "%s:%d ic parmBytes = %d\n", __FILE__, __LINE__, ic->parmBytes);
  fname = OP_SYMBOL (IC_LEFT (ic))->rname[0] ? OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name;
  inwparam = (inWparamList (OP_SYMBOL (IC_LEFT (ic))->name)) || (FUNC_ISWPARAM (OP_SYM_TYPE (IC_LEFT (ic))));

#if 0
  gpsimDebug_StackDump (__FILE__, __LINE__, fname);
#endif

  /* if send set is not empty the assign */
  if (_G.sendSet)
    {
      iCode *sic;
      int pseudoStkPtr = -1;
      int firstTimeThruLoop = 1;


      /* reverse sendSet if function is not reentrant */
      if (!IFFUNC_ISREENT (ftype))
        _G.sendSet = reverseSet (_G.sendSet);

      /* First figure how many parameters are getting passed */
      stackParms = 0;
      use_wreg = 0;

      for (sic = setFirstItem (_G.sendSet); sic; sic = setNextItem (_G.sendSet))
        {
          int size;
//          int offset = 0;

          pic16_aopOp (IC_LEFT (sic), sic, FALSE);
          size = AOP_SIZE (IC_LEFT (sic));

          stackParms += size;

          /* pass the last byte through WREG */
          if (inwparam)
            {

              while (size--)
                {
                  DEBUGpic16_emitcode ("; ", "%d left %s", __LINE__, pic16_AopType (AOP_TYPE (IC_LEFT (sic))));
                  DEBUGpic16_emitcode ("; ", "push %d", pseudoStkPtr - 1);

                  if (!firstTimeThruLoop)
                    {
                      /* If this is not the first time we've been through the loop
                       * then we need to save the parameter in a temporary
                       * register. The last byte of the last parameter is
                       * passed in W. */

                      pushw ();
//                  --pseudoStkPtr;             // sanity check
                      use_wreg = 1;
                    }

                  firstTimeThruLoop = 0;

                  pic16_mov2w (AOP (IC_LEFT (sic)), size);

//                offset++;
                }
            }
          else
            {
              /* all arguments are passed via stack */
              use_wreg = 0;

              while (size--)
                {
                  DEBUGpic16_emitcode ("; ", "%d left %s", __LINE__, pic16_AopType (AOP_TYPE (IC_LEFT (sic))));
                  DEBUGpic16_emitcode ("; ", "push %d", pseudoStkPtr - 1);

//                pushaop(AOP(IC_LEFT(sic)), size);
                  pic16_mov2w (AOP (IC_LEFT (sic)), size);

                  if (!_G.resDirect)
                    pushw ();
                }
            }

          pic16_freeAsmop (IC_LEFT (sic), NULL, sic, TRUE);
        }

      if (inwparam)
        {
          if (IFFUNC_HASVARARGS (ftype) || IFFUNC_ISREENT (ftype))
            {
              pushw ();         /* save last parameter to stack if functions has varargs */
              use_wreg = 0;
            }
          else
            use_wreg = 1;
        }
      else
        use_wreg = 0;

      _G.stackRegSet = _G.sendSet;
      _G.sendSet = NULL;
    }

  /* make the call */
  pic16_emitpcode (POC_CALL, pic16_popGetWithString (fname));

  GpseudoStkPtr = 0;

  /* if we need to assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic))
       && (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {

      _G.accInUse++;
      pic16_aopOp (IC_RESULT (ic), ic, FALSE);
      _G.accInUse--;

      /* Must not assign an 8-bit result to a 16-bit variable;
       * this would use (used...) the uninitialized PRODL! */
      /* FIXME: Need a proper way to obtain size of function result type,
       * OP_SYM_ETYPE does not work: it dereferences pointer types! */
      assignResultValue (IC_RESULT (ic), getSize (OP_SYM_TYPE (IC_LEFT (ic))->next), 1);

      DEBUGpic16_emitcode ("; ", "%d left %s", __LINE__, pic16_AopType (AOP_TYPE (IC_RESULT (ic))));

      pic16_freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }

  if (!stackParms && ic->parmBytes)
    {
      stackParms = ic->parmBytes;
    }

  stackParms -= use_wreg;
  free_stack_parameters (stackParms);

#if 0
  gpsimDebug_StackDump (__FILE__, __LINE__, fname);
#endif

  /* adjust the stack for parameters if required */
//    fprintf(stderr, "%s:%d: %s ic->parmBytes= %d\n", __FILE__, __LINE__, OP_SYMBOL(IC_LEFT(ic))->name, ic->parmBytes);

#if 0
  /* if register bank was saved then pop them */
  if (ic->bankSaved)
    unsaverbank (FUNC_REGBANK (dtype), ic, TRUE);

  /* if we hade saved some registers then unsave them */
  if (ic->regsSaved && !IFFUNC_CALLEESAVES (dtype))
    unsaveRegisters (ic);
#endif
}



/*-----------------------------------------------------------------*/
/* genPcall - generates a call by pointer statement                */
/*            new version, created from genCall - HJD              */
/*-----------------------------------------------------------------*/
static void
genPcall (iCode * ic)
{
  sym_link *fntype;
  int stackParms = 0;
  symbol *retlbl = newiTempLabel (NULL);
  pCodeOp *pcop_lbl = pic16_popGetLabel (retlbl->key);

  FENTRY;

  fntype = operandType (IC_LEFT (ic))->next;

  /* if send set is not empty the assign */
  if (_G.sendSet)
    {
      iCode *sic;
      int pseudoStkPtr = -1;

      /* reverse sendSet if function is not reentrant */
      if (!IFFUNC_ISREENT (fntype))
        _G.sendSet = reverseSet (_G.sendSet);

      stackParms = 0;

      for (sic = setFirstItem (_G.sendSet); sic; sic = setNextItem (_G.sendSet))
        {
          int size;

          pic16_aopOp (IC_LEFT (sic), sic, FALSE);
          size = AOP_SIZE (IC_LEFT (sic));
          stackParms += size;

          /* all parameters are passed via stack, since WREG is clobbered
           * by the calling sequence */
          while (size--)
            {
              DEBUGpic16_emitcode ("; ", "%d left %s", __LINE__, pic16_AopType (AOP_TYPE (IC_LEFT (sic))));
              DEBUGpic16_emitcode ("; ", "push %d", pseudoStkPtr - 1);

              pic16_mov2w (AOP (IC_LEFT (sic)), size);
              pushw ();
            }

          pic16_freeAsmop (IC_LEFT (sic), NULL, sic, TRUE);
        }

      _G.stackRegSet = _G.sendSet;
      _G.sendSet = NULL;
    }

  pic16_aopOp (IC_LEFT (ic), ic, FALSE);

  // push return address
  // push $ on return stack, then replace with retlbl

  /* Thanks to Thorsten Klose for pointing out that the following
   * snippet should be interrupt safe */
  pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (&pic16_pc_intcon), pic16_popCopyReg (&pic16_pc_postdec1)));
  pic16_emitpcode (POC_BCF, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_intcon), 7));

  pic16_emitpcodeNULLop (POC_PUSH);

  pic16_emitpcode (POC_MOVLW, pic16_popGetImmd (pcop_lbl->name, 0, 0));
  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tosl));
  pic16_emitpcode (POC_MOVLW, pic16_popGetImmd (pcop_lbl->name, 1, 0));
  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tosh));
  pic16_emitpcode (POC_MOVLW, pic16_popGetImmd (pcop_lbl->name, 2, 0));
  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tosu));

  /* Conditionally re-enable interrupts, but keep interrupt flags in
   * INTCON intact (thanks to J. van der Boon, #3420588). */
  pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_preinc1), 7));
  pic16_emitpcode (POC_BSF, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_intcon), 7));

  /* make the call by writing the pointer into pc */
  mov2fp (pic16_popCopyReg (&pic16_pc_pclatu), AOP (IC_LEFT (ic)), 2);
  mov2fp (pic16_popCopyReg (&pic16_pc_pclath), AOP (IC_LEFT (ic)), 1);

  // note: MOVFF to PCL not allowed
  pic16_mov2w (AOP (IC_LEFT (ic)), 0);
  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_pcl));


  /* return address is here: (X) */
  pic16_emitpLabelFORCE (retlbl->key);

  pic16_freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);

  GpseudoStkPtr = 0;
  /* if we need assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic))
       && (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {

      _G.accInUse++;
      pic16_aopOp (IC_RESULT (ic), ic, FALSE);
      _G.accInUse--;

      /* FIXME: Need proper way to obtain the function result's type.
       * OP_SYM_TYPE(IC_LEFT(ic))->next does not work --> points to function pointer */
      assignResultValue (IC_RESULT (ic), getSize (OP_SYM_TYPE (IC_LEFT (ic))->next->next), 1);

      DEBUGpic16_emitcode ("; ", "%d left %s", __LINE__, pic16_AopType (AOP_TYPE (IC_RESULT (ic))));

      pic16_freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
    }

//    stackParms -= use_wreg;

  free_stack_parameters (stackParms);
}

/*-----------------------------------------------------------------*/
/* resultRemat - result  is rematerializable                       */
/*-----------------------------------------------------------------*/
static int
resultRemat (iCode * ic)
{
  //    DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
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

#if 0
/*-----------------------------------------------------------------*/
/* inExcludeList - return 1 if the string is in exclude Reg list   */
/*-----------------------------------------------------------------*/
static bool
inExcludeList (char *s)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d - WARNING no code generated", __FUNCTION__, __LINE__);
  int i = 0;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (options.excludeRegs[i] && STRCASECMP (options.excludeRegs[i], "none") == 0)
    return FALSE;

  for (i = 0; options.excludeRegs[i]; i++)
    {
      if (options.excludeRegs[i] && STRCASECMP (s, options.excludeRegs[i]) == 0)
        return TRUE;
    }
  return FALSE;
}
#endif

/*-----------------------------------------------------------------*/
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void
genFunction (iCode * ic)
{
  symbol *sym;
  sym_link *ftype;

  FENTRY;
  DEBUGpic16_emitcode ("; ***", "%s  %d curr label offset=%dprevious max_key=%d ", __FUNCTION__, __LINE__, pic16_labelOffset,
                       max_key);

  pic16_labelOffset += (max_key + 4);
  max_key = 0;
  GpseudoStkPtr = 0;
  _G.nRegsSaved = 0;

  ftype = operandType (IC_LEFT (ic));
  sym = OP_SYMBOL (IC_LEFT (ic));

  if (IFFUNC_ISISR (sym->type))
    {
      /* create an absolute section at the interrupt vector:
       * that is 0x0008 for interrupt 1 (high), 0x0018 interrupt 2 (low) */
      symbol *asym;
      char asymname[128];
      pBlock *apb;

      //debugf ("interrupt number: %hhi\n", FUNC_INTNO (sym->type));

      if (FUNC_INTNO (sym->type) == INTNO_UNSPEC)
        SNPRINTF(asymname, sizeof(asymname), "ivec_%s", sym->name);
      else
        SNPRINTF(asymname, sizeof(asymname), "ivec_0x%x_%s", FUNC_INTNO (sym->type), sym->name);

      /* when an interrupt is declared as naked, do not emit the special
       * wrapper segment at vector address. The user should take care for
       * this instead. -- VR */

      if (!IFFUNC_ISNAKED (ftype) && (FUNC_INTNO (sym->type) != INTNO_UNSPEC))
        {
          asym = newSymbol (asymname, 0);
          apb = pic16_newpCodeChain (NULL, 'A', pic16_newpCodeCharP ("; Starting pCode block for absolute section"));
          pic16_addpBlock (apb);

          pic16_addpCode2pBlock (apb, pic16_newpCodeCharP (";-----------------------------------------"));
          pic16_addpCode2pBlock (apb, pic16_newpCodeFunction (moduleName, asym->name));
          //pic16_addpCode2pBlock (apb, pic16_newpCode (POC_GOTO, pic16_popGetWithString (sym->rname)));
          //pic16_addpCode2pBlock (apb, pic16_newpCode (POC_GOTO, pic16_newpCodeOpLabel (sym->rname, 0)));
          pic16_addpCode2pBlock (apb, pic16_newpCodeAsmDir ("GOTO", "%s", sym->rname)); /* this suppresses a warning in LinkFlow */

          /* mark the end of this tiny function */
          pic16_addpCode2pBlock (apb, pic16_newpCodeFunction (NULL, NULL));
        }
      else
        {
          SNPRINTF(asymname, sizeof(asymname), "%s", sym->rname);
        }

      {
        absSym *abSym;

        abSym = Safe_alloc(sizeof(absSym));
        strncpy(abSym->name, asymname, sizeof(abSym->name) - 1);
        abSym->name[sizeof(abSym->name) - 1] = '\0';

        switch (FUNC_INTNO (sym->type))
          {
          case 0:
            abSym->address = 0x000000;
            break;
          case 1:
            abSym->address = 0x000008;
            break;
          case 2:
            abSym->address = 0x000018;
            break;
          default:
            //fprintf (stderr, "no interrupt number is given\n");
            abSym->address = -1;
            break;
          }

        /* relocate interrupt vectors if needed */
        if (abSym->address != -1)
          abSym->address += pic16_options.ivt_loc;

        addSet (&absSymSet, abSym);
      }
    }

  /* create the function header */
  pic16_emitcode (";", "-----------------------------------------");
  pic16_emitcode (";", " function %s", sym->name);
  pic16_emitcode (";", "-----------------------------------------");

  /* prevent this symbol from being emitted as 'extern' */
  pic16_stringInSet (sym->rname, &pic16_localFunctions, 1);

  pic16_emitcode ("", "%s:", sym->rname);
  pic16_addpCode2pBlock (pb, pic16_newpCodeFunction (moduleName, sym->rname));

  {
    absSym *ab;

    for (ab = setFirstItem (absSymSet); ab; ab = setNextItem (absSymSet))
      {
        if (!strcmp (ab->name, sym->rname))
          {
            pic16_pBlockConvert2Absolute (pb);
            break;
          }
      }
  }

  currFunc = sym;               /* update the currFunc symbol */
  _G.fregsUsed = sym->regsUsed;
  _G.sregsAlloc = newBitVect (128);

  if (IFFUNC_ISNAKED (ftype))
    {
      DEBUGpic16_emitcode ("; ***", "__naked function, no prologue");
      return;
    }

  /* if this is an interrupt service routine then
   * save wreg, status, bsr, prodl, prodh, fsr0l, fsr0h */
  if (IFFUNC_ISISR (sym->type))
    {
      _G.usefastretfie = 1;     /* use shadow registers by default */

      /* an ISR should save: WREG, STATUS, BSR, PRODL, PRODH, FSR0L, FSR0H */
      if (!FUNC_ISSHADOWREGS (sym->type))
        {
          /* do not save WREG, STATUS, BSR for high priority interrupts
           * because they are stored in the hardware shadow registers already */
          _G.usefastretfie = 0;
          pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_status));
          pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_bsr));
          pushw ();
        }

      /* these should really be optimized somehow, because not all
       * interrupt handlers modify them */
      pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_prodl));
      pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_prodh));
      pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_fsr0l));
      pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_fsr0h));
      pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_pclath));
      pic16_pushpCodeOp (pic16_popCopyReg (&pic16_pc_pclatu));

      //pic16_pBlockConvert2ISR (pb);
    }

  /* emit code to setup stack frame if user enabled,
   * and function is not main () */

  //debugf (stderr, "function name: %s ARGS=%p\n", sym->name, FUNC_ARGS (sym->type));
  if (strcmp (sym->name, "main"))
    {
      if (0 || !options.omitFramePtr
          //|| sym->regsUsed
          || IFFUNC_ARGS (sym->type) || FUNC_HASSTACKPARM (sym->etype))
        {
          /* setup the stack frame */
          if (STACK_MODEL_LARGE)
            pic16_pushpCodeOp (pic16_popCopyReg (pic16_framepnt_hi));
          pic16_pushpCodeOp (pic16_popCopyReg (pic16_framepnt_lo));

          if (STACK_MODEL_LARGE)
            pic16_emitpcode (POC_MOVFF, pic16_popCombine2 (pic16_stackpnt_hi, pic16_framepnt_hi, 0));
          pic16_emitpcode (POC_MOVFF, pic16_popCombine2 (pic16_stackpnt_lo, pic16_framepnt_lo, 0));
        }
    }

  if ((IFFUNC_ISREENT (sym->type) || options.stackAuto) && sym->stack)
    {
      if (sym->stack > 127)
        werror (W_STACK_OVERFLOW, sym->name);

      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (sym->stack));
      pic16_emitpcode (POC_SUBWF, pic16_popCopyReg (pic16_stackpnt_lo));        //&pic16_pc_fsr1l));
      emitSKPC;
      pic16_emitpcode (POC_DECF, pic16_popCopyReg (pic16_stackpnt_hi)); //&pic16_pc_fsr1h));
    }

  if (inWparamList (sym->name) || FUNC_ISWPARAM (sym->type))
    {
      if (IFFUNC_HASVARARGS (sym->type) || IFFUNC_ISREENT (sym->type))
        _G.useWreg = 0;
      else
        _G.useWreg = 1;
    }
  else
    _G.useWreg = 0;

  /* if callee-save to be used for this function
   * then save the registers being used in this function */
  //if (IFFUNC_CALLEESAVES (sym->type))
  if (strcmp (sym->name, "main"))
    {
      int i;

      /* if any registers used */
      if (sym->regsUsed)
        {
          pic16_emitpinfo (INF_LOCALREGS, pic16_newpCodeOpLocalRegs (LR_ENTRY_BEGIN));

          if (!pic16_options.xinst)
            {
              /* save the registers used */
              DEBUGpic16_emitcode ("; **", "Saving used registers in stack");
              for (i = 0; i < sym->regsUsed->size; i++)
                {
                  if (bitVectBitValue (sym->regsUsed, i))
                    {
#if 0
                      fprintf (stderr, "%s:%d local register w/rIdx = %d is used in function\n", __FUNCTION__, __LINE__, i);
#endif
                      pic16_pushpCodeOp (pic16_popRegFromIdx (i));
                      _G.nRegsSaved++;

                      if (!pic16_regWithIdx (i)->wasUsed)
                        {
                          fprintf (stderr, "%s:%d register %s is used in function but was wasUsed = 0\n",
                                   __FILE__, __LINE__, pic16_regWithIdx (i)->name);
                          pic16_regWithIdx (i)->wasUsed = 1;
                        }
                    }
                }
            }
          else
            {
              /* xinst */
              DEBUGpic16_emitcode ("; **", "Allocate a space in stack to be used as temporary registers");
              for (i = 0; i < sym->regsUsed->size; i++)
                {
                  if (bitVectBitValue (sym->regsUsed, i))
                    _G.nRegsSaved++;
                }

              //pic16_emitpcode (POC_ADDFSR, pic16_popGetLit2 (2, pic16_popGetLit (_G.nRegsSaved)));
            }

          pic16_emitpinfo (INF_LOCALREGS, pic16_newpCodeOpLocalRegs (LR_ENTRY_END));
        }
    }

  /* if critical function then turn interrupts off */
  if (IFFUNC_ISCRITICAL (ftype))
    {
      genCritical (NULL);
    }                           // if

  DEBUGpic16_emitcode ("; ", "need to adjust stack = %d", sym->stack);
  //fprintf (stderr, "Function '%s' uses %d bytes of stack\n", sym->name, sym->stack);
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));

  FENTRY;

  if (IFFUNC_ISNAKED (sym->type))
    {
      DEBUGpic16_emitcode ("; ***", "__naked function, no epilogue");
      return;
    }

  if (IFFUNC_ISCRITICAL (sym->type))
    {
      genEndCritical (NULL);
    }                           // if

  _G.stack_lat = 0;

  //sym->regsUsed = _G.fregsUsed;

  /* now we need to restore the registers */
  /* if any registers used */

  /* first restore registers that might be used for stack access */
  if (_G.sregsAllocSet)
    {
      reg_info *sr;

      _G.sregsAllocSet = reverseSet (_G.sregsAllocSet);
      for (sr = setFirstItem (_G.sregsAllocSet); sr; sr = setNextItem (_G.sregsAllocSet))
        {
          pic16_poppCodeOp (pic16_popRegFromIdx (sr->rIdx));
        }
    }

  if (strcmp (sym->name, "main") && sym->regsUsed)
    {
      int i;

      pic16_emitpinfo (INF_LOCALREGS, pic16_newpCodeOpLocalRegs (LR_EXIT_BEGIN));
      /* restore registers used */
      DEBUGpic16_emitcode ("; **", "Restoring used registers from stack");
      for (i = sym->regsUsed->size; i >= 0; i--)
        {
          if (bitVectBitValue (sym->regsUsed, i))
            {
              pic16_poppCodeOp (pic16_popRegFromIdx (i));
              _G.nRegsSaved--;
            }
        }
      pic16_emitpinfo (INF_LOCALREGS, pic16_newpCodeOpLocalRegs (LR_EXIT_END));
    }

  if ((IFFUNC_ISREENT (sym->type) || options.stackAuto) && sym->stack)
    {
      if (sym->stack == 1)
        {
          pic16_emitpcode (POC_INFSNZ, pic16_popCopyReg (pic16_stackpnt_lo));
          pic16_emitpcode (POC_INCF, pic16_popCopyReg (pic16_stackpnt_hi));
        }
      else
        {
          // we have to add more than one...
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (pic16_stack_postinc));  // this holds a return value!
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (sym->stack - 1));
          pic16_emitpcode (POC_ADDWF, pic16_popCopyReg (pic16_stackpnt_lo));
          emitSKPNC;
          pic16_emitpcode (POC_INCF, pic16_popCopyReg (pic16_stackpnt_hi));
          pic16_emitpcode (POC_COMF, pic16_popCopyReg (&pic16_pc_wreg));        // WREG = - (WREG+1)!
          pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (pic16_stack_plusw));    // this holds a retrun value!
        }
    }

  if (strcmp (sym->name, "main"))
    {
      if (0 || !options.omitFramePtr
          //|| sym->regsUsed
          || IFFUNC_ARGS (sym->type) || FUNC_HASSTACKPARM (sym->etype))
        {
          /* restore stack frame */
          pic16_poppCodeOp (pic16_popCopyReg (pic16_framepnt_lo));
          if (STACK_MODEL_LARGE)
            pic16_poppCodeOp (pic16_popCopyReg (pic16_framepnt_hi));
        }
    }

  _G.useWreg = 0;

  if (IFFUNC_ISISR (sym->type))
    {
      pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_pclatu));
      pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_pclath));
      pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_fsr0h));
      pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_fsr0l));
      pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_prodh));
      pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_prodl));

      if (!FUNC_ISSHADOWREGS (sym->type))
        {
          /* do not restore interrupt vector for WREG, STATUS, BSR
           * for high priority interrupt, see genFunction */
          pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (pic16_stack_preinc));
          pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_bsr));
          pic16_poppCodeOp (pic16_popCopyReg (&pic16_pc_status));
        }
      //_G.interruptvector = 0;         /* sanity check */

      /* if debug then send end of function */
      /* if (options.debug && currFunc)  */
      if (currFunc)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      if (_G.usefastretfie)
        pic16_emitpcode (POC_RETFIE, pic16_newpCodeOpLit (1));
      else
        pic16_emitpcodeNULLop (POC_RETFIE);

      pic16_addpCode2pBlock (pb, pic16_newpCodeFunction (NULL, NULL));

      _G.usefastretfie = 0;
      return;
    }

  /* if debug then send end of function */
  if (currFunc)
    {
      debugFile->writeEndFunction (currFunc, ic, 1);
    }

  /* insert code to restore stack frame, if user enabled it
   * and function is not main () */

  pic16_emitpcodeNULLop (POC_RETURN);

  /* Mark the end of a function */
  pic16_addpCode2pBlock (pb, pic16_newpCodeFunction (NULL, NULL));
}


void
pic16_storeForReturn (iCode * ic, /*operand *op, */ int offset, pCodeOp * dest)
{
  unsigned long lit = 1;
  operand *op;

  op = IC_LEFT (ic);

  // this fails for pic16_isLitOp(op) (if op is an AOP_PCODE)
  if (AOP_TYPE (op) == AOP_LIT)
    {
      if (!IS_FLOAT (operandType (op)))
        {
          lit = ulFromVal (AOP (op)->aopu.aop_lit);
        }
      else
        {
          union
          {
            unsigned long lit_int;
            float lit_float;
          } info;

          /* take care if literal is a float */
          info.lit_float = (float)floatFromVal (AOP (op)->aopu.aop_lit);
          lit = info.lit_int;
        }
    }

  if (AOP_TYPE (op) == AOP_LIT)
    {
      /* FIXME: broken for
       *   char __at(0x456) foo;
       *   return &foo;
       * (upper byte is 0x00 (__code space) instead of 0x80 (__data) */
      pic16_movLit2f (dest, (lit >> (8ul * offset)));
    }
  else if (AOP_TYPE (op) == AOP_PCODE && AOP (op)->aopu.pcop->type == PO_IMMEDIATE)
    {
      /* char *s= "aaa"; return s; */
      /* XXX: Using UPPER(__str_0) will yield 0b00XXXXXX, so
       *      that the generic pointer is interpreted correctly
       *      as referring to __code space, but this is fragile! */
      pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (op), offset));
      /* XXX: should check that dest != WREG */
      pic16_emitpcode (POC_MOVWF, dest);
    }
  else
    {
      if (dest->type == PO_WREG && (offset == 0))
        {
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (op), offset));
          return;
        }
      pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (op), offset), dest));
    }
}

/*-----------------------------------------------------------------*/
/* genRet - generate code for return statement                     */
/*-----------------------------------------------------------------*/
static void
genRet (iCode * ic)
{
  int size;
  operand *left;

  FENTRY;
  /* if we have no return value then
   * just generate the "ret" */

  if (!IC_LEFT (ic))
    goto jumpret;

  /* we have something to return then
   * move the return value into place */
  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  size = AOP_SIZE (IC_LEFT (ic));

  if (size <= 4)
    {
      if (size > 3)
        pic16_storeForReturn (ic, /*IC_LEFT(ic), */ 3, pic16_popCopyReg (&pic16_pc_fsr0l));

      if (size > 2)
        pic16_storeForReturn (ic, /*IC_LEFT(ic), */ 2, pic16_popCopyReg (&pic16_pc_prodh));

      if (size > 1)
        pic16_storeForReturn (ic, /*IC_LEFT(ic), */ 1, pic16_popCopyReg (&pic16_pc_prodl));

      pic16_storeForReturn (ic, /*IC_LEFT(ic), */ 0, pic16_popCopyReg (&pic16_pc_wreg));

    }
  else
    {
      /* >32-bits, setup stack and FSR0 */
      while (size--)
        {
//                      DEBUGpic16_emitcode("; ", "POC_MOVLW %d", GpseudoStkPtr);
//                      DEBUGpic16_emitcode("; ", "POC_MOVFW PLUSW2");

          pic16_pushpCodeOp (pic16_popGet (AOP (IC_LEFT (ic)), size));

//                      popaopidx(AOP(oper), size, GpseudoStkPtr);
          GpseudoStkPtr++;
        }

      /* setup FSR0 */
      pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (pic16_stackpnt_lo), pic16_popCopyReg (&pic16_pc_fsr0l)));

      if (STACK_MODEL_LARGE)
        {
          pic16_emitpcode (POC_MOVFF,
                           pic16_popGet2p (pic16_popCopyReg (pic16_stackpnt_hi), pic16_popCopyReg (&pic16_pc_fsr0h)));
        }
      else
        {
          pic16_emitpcode (POC_CLRF, pic16_popCopyReg (pic16_stackpnt_hi));
        }
    }

  pic16_freeAsmop (IC_LEFT (ic), NULL, ic, TRUE);

jumpret:
  /* generate a jump to the return label
   * if the next is not the return statement */
  if (!(ic->next && ic->next->op == LABEL && IC_LABEL (ic->next) == returnLabel))
    {

      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (returnLabel->key));
      pic16_emitcode ("goto", "_%05d_DS_", labelKey2num (returnLabel->key + pic16_labelOffset));
    }
}

static set *critical_temps = NULL;

static void
genCritical (iCode * ic)
{
  pCodeOp *saved_intcon;

  if (!critical_temps)
    critical_temps = newSet ();

  saved_intcon = pic16_popGetTempReg (0);
  pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (&pic16_pc_intcon), saved_intcon));
  pic16_emitpcode (POC_BCF, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_intcon), 7));
  addSetHead (&critical_temps, saved_intcon);
}

static void
genEndCritical (iCode * ic)
{
  pCodeOp *saved_intcon = NULL;

  saved_intcon = getSet (&critical_temps);
  if (!saved_intcon)
    {
      fprintf (stderr, "Critical section left, but none entered -- ignoring for now.\n");
      return;
    }                           // if

  pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (saved_intcon, 7));
  pic16_emitpcode (POC_BSF, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_intcon), 7));
  pic16_popReleaseTempReg (saved_intcon, 0);
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void
genLabel (iCode * ic)
{
  FENTRY;

  /* special case never generate */
  if (IC_LABEL (ic) == entryLabel)
    return;

  pic16_emitpLabel (IC_LABEL (ic)->key);
//  pic16_emitcode("","_%05d_DS_:",labelKey2num (IC_LABEL(ic)->key + pic16_labelOffset));
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a goto                                      */
/*-----------------------------------------------------------------*/
//tsd
static void
genGoto (iCode * ic)
{
  FENTRY;
  pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_LABEL (ic)->key));
//  pic16_emitcode ("goto","_%05d_DS_",labelKey2num (IC_LABEL(ic)->key + pic16_labelOffset));
}


/*-----------------------------------------------------------------*/
/* genMultbits :- multiplication of bits                           */
/*-----------------------------------------------------------------*/
static void
genMultbits (operand * left, operand * right, operand * result)
{
  FENTRY;

  if (!pic16_sameRegs (AOP (result), AOP (right)))
    pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), 0));

  pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (right), 0));
  pic16_emitpcode (POC_BTFSS, pic16_popGet (AOP (left), 0));
  pic16_emitpcode (POC_BCF, pic16_popGet (AOP (result), 0));

}


/*-----------------------------------------------------------------*/
/* genMultOneByte : 8 bit multiplication & division                */
/*-----------------------------------------------------------------*/
static void
genMultOneByte (operand * left, operand * right, operand * result)
{

  FENTRY;
  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);
  DEBUGpic16_pic16_AopTypeSign (__LINE__, left, right, result);

  /* (if two literals, the value is computed before) */
  /* if one literal, literal on the right */
  if (AOP_TYPE (left) == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
    }

  /* size is already checked in genMult == 1 */
//      size = AOP_SIZE(result);

  if (AOP_TYPE (right) == AOP_LIT)
    {
      pic16_emitpcomment ("multiply lit val:%s by variable %s and store in %s",
                          pic16_aopGet (AOP (right), 0, FALSE, FALSE),
                          pic16_aopGet (AOP (left), 0, FALSE, FALSE), pic16_aopGet (AOP (result), 0, FALSE, FALSE));
    }
  else
    {
      pic16_emitpcomment ("multiply variable :%s by variable %s and store in %s",
                          pic16_aopGet (AOP (right), 0, FALSE, FALSE),
                          pic16_aopGet (AOP (left), 0, FALSE, FALSE), pic16_aopGet (AOP (result), 0, FALSE, FALSE));
    }

  pic16_genMult8X8_n (left, right, result);
}

#if 0
/*-----------------------------------------------------------------*/
/* genMultOneWord : 16 bit multiplication                          */
/*-----------------------------------------------------------------*/
static void
genMultOneWord (operand * left, operand * right, operand * result)
{
  FENTRY;
  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);
  DEBUGpic16_pic16_AopTypeSign (__LINE__, left, right, result);

  /* (if two literals, the value is computed before)
   * if one literal, literal on the right */
  if (AOP_TYPE (left) == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
    }

  /* size is checked already == 2 */
//  size = AOP_SIZE(result);

  if (AOP_TYPE (right) == AOP_LIT)
    {
      pic16_emitpcomment ("multiply lit val:%s by variable %s and store in %s",
                          pic16_aopGet (AOP (right), 0, FALSE, FALSE),
                          pic16_aopGet (AOP (left), 0, FALSE, FALSE), pic16_aopGet (AOP (result), 0, FALSE, FALSE));
    }
  else
    {
      pic16_emitpcomment ("multiply variable :%s by variable %s and store in %s",
                          pic16_aopGet (AOP (right), 0, FALSE, FALSE),
                          pic16_aopGet (AOP (left), 0, FALSE, FALSE), pic16_aopGet (AOP (result), 0, FALSE, FALSE));
    }

  pic16_genMult16X16_16 (left, right, result);
}
#endif

#if 0
/*-----------------------------------------------------------------*/
/* genMultOneLong : 32 bit multiplication                          */
/*-----------------------------------------------------------------*/
static void
genMultOneLong (operand * left, operand * right, operand * result)
{
  FENTRY;
  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);
  DEBUGpic16_pic16_AopTypeSign (__LINE__, left, right, result);

  /* (if two literals, the value is computed before)
   * if one literal, literal on the right */
  if (AOP_TYPE (left) == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
    }

  /* size is checked already == 4 */
//  size = AOP_SIZE(result);

  if (AOP_TYPE (right) == AOP_LIT)
    {
      pic16_emitpcomment ("multiply lit val:%s by variable %s and store in %s",
                          pic16_aopGet (AOP (right), 0, FALSE, FALSE),
                          pic16_aopGet (AOP (left), 0, FALSE, FALSE), pic16_aopGet (AOP (result), 0, FALSE, FALSE));
    }
  else
    {
      pic16_emitpcomment ("multiply variable :%s by variable %s and store in %s",
                          pic16_aopGet (AOP (right), 0, FALSE, FALSE),
                          pic16_aopGet (AOP (left), 0, FALSE, FALSE), pic16_aopGet (AOP (result), 0, FALSE, FALSE));
    }

  pic16_genMult32X32_32 (left, right, result);
}
#endif



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
  /* assign the amsops */
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (right, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);

  /* special cases first *
   * both are bits */
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

#if 0
  /* if both are of size == 2 */
  if (AOP_SIZE (left) == 2 && AOP_SIZE (right) == 2)
    {
      genMultOneWord (left, right, result);
      goto release;
    }

  /* if both are of size == 4 */
  if (AOP_SIZE (left) == 4 && AOP_SIZE (right) == 4)
    {
      genMultOneLong (left, right, result);
      goto release;
    }
#endif

  fprintf (stderr, "%s: should have been transformed into function call\n", __FUNCTION__);
  assert (!"Multiplication should have been transformed into function call!");

  pic16_emitcode ("multiply ", "sizes are greater than 4 ... need to insert proper algor.");


  fprintf (stderr, "operand sizes result: %d left: %d right: %d\n", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right));
  /* should have been converted to function call */
  assert (0);

release:
  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

#if 0
/*-----------------------------------------------------------------*/
/* genDivbits :- division of bits                                  */
/*-----------------------------------------------------------------*/
static void
genDivbits (operand * left, operand * right, operand * result)
{
  char *l;

  FENTRY;
  /* the result must be bit */
  pic16_emitcode ("mov", "b,%s", pic16_aopGet (AOP (right), 0, FALSE, FALSE));
  l = pic16_aopGet (AOP (left), 0, FALSE, FALSE);

  MOVA (l);

  pic16_emitcode ("div", "ab");
  pic16_emitcode ("rrc", "a");
  pic16_aopPut (AOP (result), "c", 0);
}

/*-----------------------------------------------------------------*/
/* genDivOneByte : 8 bit division                                  */
/*-----------------------------------------------------------------*/
static void
genDivOneByte (operand * left, operand * right, operand * result)
{
  sym_link *opetype = operandType (result);
  char *l;
  symbol *lbl;
  int size, offset;

  /* result = divident / divisor
   * - divident may be a register or a literal,
   * - divisor may be a register or a literal,
   * so there are 3 cases (literal / literal is optimized
   * by the front-end) to handle.
   * In addition we must handle signed and unsigned, which
   * result in 6 final different cases -- VR */

  FENTRY;

  size = AOP_SIZE (result) - 1;
  offset = 1;
  /* signed or unsigned */
  if (SPEC_USIGN (opetype))
    {
      pCodeOp *pct1,            /* count */
              *pct2,                   /* reste */
              *pct3;                   /* temp */
      symbol *label1, *label2, *label3;;


      /* unsigned is easy */

      pct1 = pic16_popGetTempReg (1);
      pct2 = pic16_popGetTempReg (1);
      pct3 = pic16_popGetTempReg (1);

      label1 = newiTempLabel (NULL);
      label2 = newiTempLabel (NULL);
      label3 = newiTempLabel (NULL);

      /* the following algorithm is extracted from divuint.c */

      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (8));
      pic16_emitpcode (POC_MOVWF, pic16_pCodeOpCopy (pct1));

      pic16_emitpcode (POC_CLRF, pic16_pCodeOpCopy (pct2));

      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), 0));

      pic16_emitpLabel (label1->key);

      emitCLRC;
      pic16_emitpcode (POC_RLCF, pic16_pCodeOpCopy (pct2));


      emitCLRC;
      pic16_emitpcode (POC_RLCF, pic16_popCopyReg (&pic16_pc_wreg));


      emitSKPNC;
      pic16_emitpcode (POC_INCF, pic16_pCodeOpCopy (pct2));

      pic16_emitpcode (POC_MOVWF, pic16_pCodeOpCopy (pct3));
      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), 0));

      pic16_emitpcode (POC_CPFSLT, pic16_pCodeOpCopy (pct2));
      pic16_emitpcode (POC_BRA, pic16_popGetLabel (label3->key));
      pic16_emitpcode (POC_BRA, pic16_popGetLabel (label2->key));

      pic16_emitpLabel (label3->key);
      pic16_emitpcode (POC_SUBWF, pic16_pCodeOpCopy (pct2));
      pic16_emitpcode (POC_INCF, pic16_pCodeOpCopy (pct3));



      pic16_emitpLabel (label2->key);
      pic16_emitpcode (POC_MOVFW, pic16_pCodeOpCopy (pct3));
      pic16_emitpcode (POC_DECFSZ, pic16_pCodeOpCopy (pct1));
      pic16_emitpcode (POC_BRA, pic16_popGetLabel (label1->key));

      /* result is in wreg */
      if (AOP_TYPE (result) != AOP_ACC)
        pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));

      pic16_popReleaseTempReg (pct3, 1);
      pic16_popReleaseTempReg (pct2, 1);
      pic16_popReleaseTempReg (pct1, 1);

      return;
    }

  /* signed is a little bit more difficult */

  /* save the signs of the operands */
  l = pic16_aopGet (AOP (left), 0, FALSE, FALSE);
  MOVA (l);
  pic16_emitcode ("xrl", "a,%s", pic16_aopGet (AOP (right), 0, FALSE, TRUE));
  pic16_emitcode ("push", "acc");       /* save it on the stack */

  /* now sign adjust for both left & right */
  l = pic16_aopGet (AOP (right), 0, FALSE, FALSE);
  MOVA (l);
  lbl = newiTempLabel (NULL);
  pic16_emitcode ("jnb", "acc.7,%05d_DS_", labelKey2num (lbl->key));
  pic16_emitcode ("cpl", "a");
  pic16_emitcode ("inc", "a");
  pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));
  pic16_emitcode ("mov", "b,a");

  /* sign adjust left side */
  l = pic16_aopGet (AOP (left), 0, FALSE, FALSE);
  MOVA (l);

  lbl = newiTempLabel (NULL);
  pic16_emitcode ("jnb", "acc.7,%05d_DS_", labelKey2num (lbl->key));
  pic16_emitcode ("cpl", "a");
  pic16_emitcode ("inc", "a");
  pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));

  /* now the division */
  pic16_emitcode ("div", "ab");
  /* we are interested in the lower order
     only */
  pic16_emitcode ("mov", "b,a");
  lbl = newiTempLabel (NULL);
  pic16_emitcode ("pop", "acc");
  /* if there was an over flow we don't
     adjust the sign of the result */
  pic16_emitcode ("jb", "ov,%05d_DS_", labelKey2num (lbl->key));
  pic16_emitcode ("jnb", "acc.7,%05d_DS_", labelKey2num (lbl->key));
  CLRC;
  pic16_emitcode ("clr", "a");
  pic16_emitcode ("subb", "a,b");
  pic16_emitcode ("mov", "b,a");
  pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));

  /* now we are done */
  pic16_aopPut (AOP (result), "b", 0);
  if (size > 0)
    {
      pic16_emitcode ("mov", "c,b.7");
      pic16_emitcode ("subb", "a,acc");
    }
  while (size--)
    pic16_aopPut (AOP (result), "a", offset++);

}
#endif

/*-----------------------------------------------------------------*/
/* genDiv - generates code for division                            */
/*-----------------------------------------------------------------*/
static void
genDiv (iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);
  int negated = 0;
  int leftVal = 0, rightVal = 0;
  int signedLits = 0;
  char *functions[2][2] = { {"__divschar", "__divuchar"}, {"__modschar", "__moduchar"} };
  int op = 0;

  /* Division is a very lengthy algorithm, so it is better
   * to call support routines than inlining algorithm.
   * Division functions written here just in case someone
   * wants to inline and not use the support libraries -- VR */

  FENTRY;

  /* assign the amsops */
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (right, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  if (ic->op == '/')
    op = 0;
  else if (ic->op == '%')
    op = 1;
  else
    assert (!"invalid operation requested in genDivMod");

  /* get literal values */
  if (IS_VALOP (left))
    {
      leftVal = (int) ulFromVal (OP_VALUE (left));
      assert (leftVal >= -128 && leftVal < 256);
      if (leftVal < 0)
        {
          signedLits++;
        }
    }
  if (IS_VALOP (right))
    {
      rightVal = (int) ulFromVal (OP_VALUE (right));
      assert (rightVal >= -128 && rightVal < 256);
      if (rightVal < 0)
        {
          signedLits++;
        }
    }

  /* We should only come here to convert all
   * / : {u8_t, s8_t} x {u8_t, s8_t} -> {u8_t, s8_t}
   * with exactly one operand being s8_t into
   * u8_t x u8_t -> u8_t. All other cases should have been
   * turned into calls to support routines beforehand... */
  if ((AOP_SIZE (left) == 1 || IS_VALOP (left)) && (AOP_SIZE (right) == 1 || IS_VALOP (right)))
    {
      if ((!IS_UNSIGNED (operandType (right)) || rightVal < 0) && (!IS_UNSIGNED (operandType (left)) || leftVal < 0))
        {
          /* Both operands are signed or negative, use _divschar
           * instead of _divuchar */
          pushaop (AOP (right), 0);
          pushaop (AOP (left), 0);

          /* call _divschar */
          pic16_emitpcode (POC_CALL, pic16_popGetWithString (functions[op][0]));

          {
            symbol *sym;
            sym = newSymbol (functions[op][0], 0);
            sym->used++;
            strcpy (sym->rname, functions[op][0]);
            checkAddSym (&externs, sym);
          }

          /* assign result */
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
          if (AOP_SIZE (result) > 1)
            {
              pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (&pic16_pc_prodl), pic16_popGet (AOP (result), 1)));
              /* sign extend */
              pic16_addSign (result, 2, 1);
            }

          /* clean up stack */
          pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (pic16_stack_preinc));
          pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (pic16_stack_preinc));

          goto release;
        }

      /* push right operand */
      if (IS_VALOP (right))
        {
          if (rightVal < 0)
            {
              pic16_pushpCodeOp (pic16_popGetLit (-rightVal));
              negated++;
            }
          else
            {
              pushaop (AOP (right), 0);
            }
        }
      else if (!IS_UNSIGNED (operandType (right)))
        {
          pic16_mov2w (AOP (right), 0);
          pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (right), 0, 7));
          pic16_emitpcode (POC_NEGF, pic16_popCopyReg (&pic16_pc_wreg));
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (pic16_stack_postdec));
          negated++;
        }
      else
        {
          pushaop (AOP (right), 0);
        }

      /* push left operand */
      if (IS_VALOP (left))
        {
          if (leftVal < 0)
            {
              pic16_pushpCodeOp (pic16_popGetLit (-leftVal));
              negated++;
            }
          else
            {
              pushaop (AOP (left), 0);
            }
        }
      else if (!IS_UNSIGNED (operandType (left)))
        {
          pic16_mov2w (AOP (left), 0);
          pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (left), 0, 7));
          pic16_emitpcode (POC_NEGF, pic16_popCopyReg (&pic16_pc_wreg));
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (pic16_stack_postdec));
          negated++;
        }
      else
        {
          pushaop (AOP (left), 0);
        }

      /* call _divuchar */
      pic16_emitpcode (POC_CALL, pic16_popGetWithString (functions[op][1]));

      {
        symbol *sym;
        sym = newSymbol (functions[op][1], 0);
        sym->used++;
        strcpy (sym->rname, functions[op][1]);
        checkAddSym (&externs, sym);
      }

      /* Revert negation(s) from above.
       * This is inefficient: if both operands are negative, this
       * should not touch WREG. However, determining that exactly
       * one operand was negated costs at least 3 instructions,
       * so there is nothing to be gained here, is there?
       *
       * I negate WREG because either operand might share registers with
       * result, so assigning first might destroy an operand. */

      /* For the modulus operator, (a/b)*b == a shall hold.
       * Thus: a>0, b>0 --> a/b >= 0 and a%b >= 0
       *       a>0, b<0 --> a/b <= 0 and a%b >= 0 (e.g. 128 / -5 = -25, -25*(-5) =  125 and +3 remaining)
       *       a<0, b>0 --> a/b <= 0 and a%b < 0  (e.g. -128 / 5 = -25, -25*  5  = -125 and -3 remaining)
       *       a<0, b<0 --> a/b >= 0 and a%b < 0  (e.g. -128 / -5 = 25,  25*(-5) = -125 and -3 remaining)
       * Only invert the result if the left operand is negative (sigh).
       */
      if (AOP_SIZE (result) <= 1 || !negated)
        {
          if (ic->op == '/')
            {
              if (IS_VALOP (right))
                {
                  if (rightVal < 0)
                    {
                      /* we negated this operand above */
                      pic16_emitpcode (POC_NEGF, pic16_popCopyReg (&pic16_pc_wreg));
                    }
                }
              else if (!IS_UNSIGNED (operandType (right)))
                {
                  pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (right), 0, 7));
                  pic16_emitpcode (POC_NEGF, pic16_popCopyReg (&pic16_pc_wreg));
                }
            }

          if (IS_VALOP (left))
            {
              if (leftVal < 0)
                {
                  /* we negated this operand above */
                  pic16_emitpcode (POC_NEGF, pic16_popCopyReg (&pic16_pc_wreg));
                }
            }
          else if (!IS_UNSIGNED (operandType (left)))
            {
              pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (left), 0, 7));
              pic16_emitpcode (POC_NEGF, pic16_popCopyReg (&pic16_pc_wreg));
            }

          /* Move result to destination. */
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));

          /* Zero-extend:  no operand was signed (or result is just a byte). */
          pic16_addSign (result, 1, 0);
        }
      else
        {
          assert (AOP_SIZE (result) > 1);
          pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), 1));
          if (ic->op == '/')
            {
              if (IS_VALOP (right))
                {
                  if (rightVal < 0)
                    {
                      /* we negated this operand above */
                      pic16_emitpcode (POC_COMF, pic16_popGet (AOP (result), 1));
                    }
                }
              else if (!IS_UNSIGNED (operandType (right)))
                {
                  pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (right), 0, 7));
                  pic16_emitpcode (POC_COMF, pic16_popGet (AOP (result), 1));
                }
            }

          if (IS_VALOP (left))
            {
              if (leftVal < 0)
                {
                  /* we negated this operand above */
                  pic16_emitpcode (POC_COMF, pic16_popGet (AOP (result), 1));
                }
            }
          else if (!IS_UNSIGNED (operandType (left)))
            {
              pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (left), 0, 7));
              pic16_emitpcode (POC_COMF, pic16_popGet (AOP (result), 1));
            }

          /* Move result to destination. */
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));

          /* Negate result if required. */
          pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (result), 1, 7));
          pic16_emitpcode (POC_NEGF, pic16_popGet (AOP (result), 0));

          /* Sign-extend. */
          pic16_addSign (result, 2, 1);
        }

      /* clean up stack */
      pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (pic16_stack_preinc));
      pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (pic16_stack_preinc));
      goto release;
    }

#if 0
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
#endif

  /* should have been converted to function call */
  assert (0);
release:
  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

#if 0
/*-----------------------------------------------------------------*/
/* genModbits :- modulus of bits                                   */
/*-----------------------------------------------------------------*/
static void
genModbits (operand * left, operand * right, operand * result)
{
  char *l;

  FENTRY;

  werror (W_POSSBUG2, __FILE__, __LINE__);
  /* the result must be bit */
  pic16_emitcode ("mov", "b,%s", pic16_aopGet (AOP (right), 0, FALSE, FALSE));
  l = pic16_aopGet (AOP (left), 0, FALSE, FALSE);

  MOVA (l);

  pic16_emitcode ("div", "ab");
  pic16_emitcode ("mov", "a,b");
  pic16_emitcode ("rrc", "a");
  pic16_aopPut (AOP (result), "c", 0);
}

/*-----------------------------------------------------------------*/
/* genModOneByte : 8 bit modulus                                   */
/*-----------------------------------------------------------------*/
static void
genModOneByte (operand * left, operand * right, operand * result)
{
  sym_link *opetype = operandType (result);
  char *l;
  symbol *lbl;

  FENTRY;
  werror (W_POSSBUG2, __FILE__, __LINE__);

  /* signed or unsigned */
  if (SPEC_USIGN (opetype))
    {
      /* unsigned is easy */
      pic16_emitcode ("mov", "b,%s", pic16_aopGet (AOP (right), 0, FALSE, FALSE));
      l = pic16_aopGet (AOP (left), 0, FALSE, FALSE);
      MOVA (l);
      pic16_emitcode ("div", "ab");
      pic16_aopPut (AOP (result), "b", 0);
      return;
    }

  /* signed is a little bit more difficult */

  /* save the signs of the operands */
  l = pic16_aopGet (AOP (left), 0, FALSE, FALSE);
  MOVA (l);

  pic16_emitcode ("xrl", "a,%s", pic16_aopGet (AOP (right), 0, FALSE, FALSE));
  pic16_emitcode ("push", "acc");       /* save it on the stack */

  /* now sign adjust for both left & right */
  l = pic16_aopGet (AOP (right), 0, FALSE, FALSE);
  MOVA (l);

  lbl = newiTempLabel (NULL);
  pic16_emitcode ("jnb", "acc.7,%05d_DS_", labelKey2num (lbl->key));
  pic16_emitcode ("cpl", "a");
  pic16_emitcode ("inc", "a");
  pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));
  pic16_emitcode ("mov", "b,a");

  /* sign adjust left side */
  l = pic16_aopGet (AOP (left), 0, FALSE, FALSE);
  MOVA (l);

  lbl = newiTempLabel (NULL);
  pic16_emitcode ("jnb", "acc.7,%05d_DS_", labelKey2num (lbl->key));
  pic16_emitcode ("cpl", "a");
  pic16_emitcode ("inc", "a");
  pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));

  /* now the multiplication */
  pic16_emitcode ("div", "ab");
  /* we are interested in the lower order
     only */
  lbl = newiTempLabel (NULL);
  pic16_emitcode ("pop", "acc");
  /* if there was an over flow we don't
     adjust the sign of the result */
  pic16_emitcode ("jb", "ov,%05d_DS_", labelKey2num (lbl->key));
  pic16_emitcode ("jnb", "acc.7,%05d_DS_", labelKey2num (lbl->key));
  CLRC;
  pic16_emitcode ("clr", "a");
  pic16_emitcode ("subb", "a,b");
  pic16_emitcode ("mov", "b,a");
  pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));

  /* now we are done */
  pic16_aopPut (AOP (result), "b", 0);

}
#endif

/*-----------------------------------------------------------------*/
/* genMod - generates code for division                            */
/*-----------------------------------------------------------------*/
static void
genMod (iCode * ic)
{
  /* Task deferred to genDiv */
  genDiv (ic);
#if 0
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  FENTRY;

  /* assign the amsops */
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (right, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

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
  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
#endif
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

  /* if true label then we jump if condition
     supplied is true */
  if (IC_TRUE (ic))
    {

      if (strcmp (jval, "a") == 0)
        emitSKPZ;
      else if (strcmp (jval, "c") == 0)
        emitSKPNC;
      else
        {
          DEBUGpic16_emitcode ("; ***", "%d - assuming %s is in bit space", __LINE__, jval);
          pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit (jval, -1, 1, PO_GPR_REGISTER));
        }

      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ic)->key));
      pic16_emitcode (" goto", "_%05d_DS_", labelKey2num (IC_TRUE (ic)->key + pic16_labelOffset));

    }
  else
    {
      /* false label is present */
      if (strcmp (jval, "a") == 0)
        emitSKPNZ;
      else if (strcmp (jval, "c") == 0)
        emitSKPC;
      else
        {
          DEBUGpic16_emitcode ("; ***", "%d - assuming %s is in bit space", __LINE__, jval);
          pic16_emitpcode (POC_BTFSS, pic16_newpCodeOpBit (jval, -1, 1, PO_GPR_REGISTER));
        }

      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_FALSE (ic)->key));
      pic16_emitcode (" goto", "_%05d_DS_", labelKey2num (IC_FALSE (ic)->key + pic16_labelOffset));

    }


  /* mark the icode as generated */
  ic->generated = 1;
}

static void
genIfxpCOpJump (iCode * ic, pCodeOp * jop)
{
  FENTRY;

  /* if true label then we jump if condition
     supplied is true */
  if (IC_TRUE (ic))
    {
      DEBUGpic16_emitcode ("; ***", "%d - assuming is in bit space", __LINE__);
      pic16_emitpcode (POC_BTFSC, jop);

      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ic)->key));
      pic16_emitcode (" goto", "_%05d_DS_", labelKey2num (IC_TRUE (ic)->key + pic16_labelOffset));

    }
  else
    {
      /* false label is present */
      DEBUGpic16_emitcode ("; ***", "%d - assuming is in bit space", __LINE__);
      pic16_emitpcode (POC_BTFSS, jop);

      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_FALSE (ic)->key));
      pic16_emitcode (" goto", "_%05d_DS_", labelKey2num (IC_FALSE (ic)->key + pic16_labelOffset));
    }


  /* mark the icode as generated */
  ic->generated = 1;
}

#if 0
// not needed ATM

/*-----------------------------------------------------------------*/
/* genSkip                                                         */
/*-----------------------------------------------------------------*/
static void
genSkip (iCode * ifx, int status_bit)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (!ifx)
    return;

  if (IC_TRUE (ifx))
    {
      switch (status_bit)
        {
        case 'z':
          emitSKPNZ;
          break;

        case 'c':
          emitSKPNC;
          break;

        case 'd':
          emitSKPDC;
          break;

        }

      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ifx)->key));
      // pic16_emitcode("goto","_%05d_DS_",labelKey2num (IC_TRUE(ifx)->key + pic16_labelOffset));

    }
  else
    {

      switch (status_bit)
        {

        case 'z':
          emitSKPZ;
          break;

        case 'c':
          emitSKPC;
          break;

        case 'd':
          emitSKPDC;
          break;
        }
      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_FALSE (ifx)->key));
      // pic16_emitcode("goto","_%05d_DS_",labelKey2num (IC_FALSE(ifx)->key + pic16_labelOffset));

    }

}
#endif

/*-----------------------------------------------------------------*/
/* genSkipc                                                        */
/*-----------------------------------------------------------------*/
static void
genSkipc (resolvedIfx * rifx)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d rifx= %p", __FUNCTION__, __LINE__, rifx);

  if (!rifx)
    return;

  if (rifx->condition)
    emitSKPNC;
  else
    emitSKPC;

  pic16_emitpcode (POC_GOTO, pic16_popGetLabel (rifx->lbl->key));
  rifx->generated = 1;
}

/*-----------------------------------------------------------------*/
/* mov2w_regOrLit :- move to WREG either the offset's byte from    */
/*                  aop (if it's NOT a literal) or from lit (if    */
/*                  aop is a literal)                              */
/*-----------------------------------------------------------------*/
void
mov2w_regOrLit (asmop * aop, unsigned long lit, int offset)
{
  if (aop->type == AOP_LIT)
    {
      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (lit >> (offset * 8)));
    }
  else
    {
      pic16_emitpcode (POC_MOVFW, pic16_popGet (aop, offset));
    }
}

/*-----------------------------------------------------------------*/
/* genCmp :- greater or less than comparison                       */
/*-----------------------------------------------------------------*/

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

  FENTRY;

  assert (left && right);
  assert (AOP_SIZE (left) == AOP_SIZE (right));

  size = AOP_SIZE (right) - 1;
  mask = (0x100UL << (size * 8)) - 1;
  // in the end CARRY holds "left < right" (performedLt == 1) or "left >= right" (performedLt == 0)
  performedLt = 1;
  templbl = NULL;
  lit = 0;

  resolveIfx (&rIfx, ifx);

  /* handle for special cases */
  if (pic16_genCmp_special (left, right, result, ifx, &rIfx, sign))
    return;

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
      //pic16_emitpcode (POC_BTFSS, pic16_popCopyGPR2Bit (AOP(left),0), PCORB(pcleft)->bit);
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
              pic16_emitpcode (POC_BTFSS, pic16_popCopyGPR2Bit (pic16_popGet (AOP (left), size), 7));
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
      if (AOP_TYPE (left) == AOP_PCODE && AOP (left)->aopu.pcop->type == PO_IMMEDIATE)
        {
          // right >= left(PO_IMMEDIATE)
          pic16_emitpcode (POC_MOVLW, pic16_popGetImmed (AOP (left), size, 1)); // left+1
          pic16_emitpcode (POC_SUBFW, pic16_popGet(AOP (right), size));
          performedLt = 0;
        }
      else if (AOP_TYPE (right) == AOP_PCODE && AOP (right)->aopu.pcop->type == PO_IMMEDIATE)
        {
          // left < right(PO_IMMEDIATE)
          pic16_emitpcode (POC_MOVLW, pic16_popGetImmed (AOP (right), size, 0));  // right+0
          pic16_emitpcode (POC_SUBFW, pic16_popGet (AOP (left), size));
        }
      else
        {
          mov2w_regOrLit (AOP(right), lit, size);
          pic16_emitpcode (POC_SUBFW, pic16_popGet (AOP (left), size));
        }
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
              pic16_mov2w (AOP (left), size);
              pic16_emitpcode (POC_XORLW, pic16_popGetLit (0x80));      // set ZERO flag
              emitSETC;
            }
          else
            {
              // left >= LIT <-> left + (-LIT) >= 0 <-> left + (0x100-LIT) >= 0x100
              pic16_mov2w (AOP (left), size);
              pic16_emitpcode (POC_ADDLW, pic16_popGetLit (0x80));
              pic16_emitpcode (POC_ADDLW, pic16_popGetLit ((0x100 - (litbyte + 0x80)) & 0x00FF));
            }                   // if
        }
      else
        {
          /* using PRODL as a temporary register here */
          pCodeOp *pctemp = pic16_popCopyReg (&pic16_pc_prodl);
          //pCodeOp *pctemp = pic16_popGetTempReg(1);
          pic16_mov2w (AOP (left), size);
          pic16_emitpcode (POC_ADDLW, pic16_popGetLit (0x80));
          pic16_emitpcode (POC_MOVWF, pctemp);
          pic16_mov2w (AOP (right), size);
          pic16_emitpcode (POC_ADDLW, pic16_popGetLit (0x80));
          pic16_emitpcode (POC_SUBFW, pctemp);
          //pic16_popReleaseTempReg(pctemp, 1);
        }
    }                           // if (!sign)

  // compare remaining bytes (treat as unsigned case from above)
  templbl = newiTempLabel (NULL);
  offs = size;
  while (offs--)
    {
      //DEBUGpc ("comparing bytes at offset %d", offs);
      pic16_emitpcode (POC_BNZ, pic16_popGetLabel (templbl->key));

      if (AOP_TYPE (left) == AOP_PCODE && AOP (left)->aopu.pcop->type == PO_IMMEDIATE)
        {
          // right >= left(PO_IMMEDIATE)
          pic16_emitpcode (POC_MOVLW, pic16_popGetImmed( AOP (left), offs, 1)); // left+1
          pic16_emitpcode (POC_SUBFW, pic16_popGet (AOP (right), offs));
          performedLt = 0;
        }
      else if (AOP_TYPE (right) == AOP_PCODE && AOP (right)->aopu.pcop->type == PO_IMMEDIATE)
        {
          // left < right(PO_IMMEDIATE)
          pic16_emitpcode (POC_MOVLW, pic16_popGetImmed (AOP (right), offs, 0));  // right+0
          pic16_emitpcode (POC_SUBFW, pic16_popGet (AOP (left), offs));
        }
      else
        {
          mov2w_regOrLit (AOP (right), lit, offs);
          pic16_emitpcode (POC_SUBFW, pic16_popGet (AOP (left), offs));
        }
    }                           // while (offs)
  pic16_emitpLabel (templbl->key);
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
      if (result && AOP_TYPE (result) != AOP_CRY)
        {
          // value will be stored
          emitTOGC;
        }
      else
        {
          // value wil only be used in the following genSkipc()
          rIfx.condition ^= 1;
        }
    }                           // if

correct_result_in_carry:

  // assign result to variable (if neccessary)
  if (result && AOP_TYPE (result) != AOP_CRY)
    {
      //DEBUGpc ("assign result");
      size = AOP_SIZE (result);
      while (size--)
        {
          pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), size));
        }                       // while
      pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), 0));
    }                           // if (result)

  // perform conditional jump
  if (ifx)
    {
      //DEBUGpc ("generate control flow");
      genSkipc (&rIfx);
      ifx->generated = 1;
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
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (right, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  genCmp (right, left, result, ifx, sign);

  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
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
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (right, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  genCmp (left, right, result, ifx, sign);

  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* pic16_isLitOp - check if operand has to be treated as literal   */
/*-----------------------------------------------------------------*/
bool
pic16_isLitOp (operand * op)
{
  return ((AOP_TYPE (op) == AOP_LIT)
          || ((AOP_TYPE (op) == AOP_PCODE)
              && ((AOP (op)->aopu.pcop->type == PO_LITERAL) || (AOP (op)->aopu.pcop->type == PO_IMMEDIATE))));
}

/*-----------------------------------------------------------------*/
/* pic16_isLitAop - check if operand has to be treated as literal  */
/*-----------------------------------------------------------------*/
bool
pic16_isLitAop (asmop * aop)
{
  return ((aop->type == AOP_LIT)
          || ((aop->type == AOP_PCODE) && ((aop->aopu.pcop->type == PO_LITERAL) || (aop->aopu.pcop->type == PO_IMMEDIATE))));
}



/*-----------------------------------------------------------------*/
/* genCmpEq - generates code for equal to                          */
/*-----------------------------------------------------------------*/
static void
genCmpEq (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  symbol *falselbl = newiTempLabel (NULL);
  symbol *donelbl = newiTempLabel (NULL);

  int preserve_result = 0;
  int generate_result = 0;
  int i = 0;
  unsigned long lit = -1;

  FENTRY;

  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  pic16_aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  pic16_aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);

  if ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (left) == AOP_CRY))
    {
      werror (W_POSSBUG2, __FILE__, __LINE__);
      DEBUGpic16_emitcode ("; ***", "%s  %d -- ERROR", __FUNCTION__, __LINE__);
      fprintf (stderr, "%s  %d error - left/right CRY operands not supported\n", __FUNCTION__, __LINE__);
      goto release;
    }

  if (pic16_isLitOp (left) || (AOP_TYPE (right) == AOP_ACC))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  if (AOP_TYPE (right) == AOP_LIT)
    {
      lit = ulFromVal (AOP (right)->aopu.aop_lit);
    }

  if (regsInCommon (left, result) || regsInCommon (right, result))
    preserve_result = 1;

  if (result && AOP_SIZE (result))
    generate_result = 1;

  if (generate_result && !preserve_result)
    {
      for (i = 0; i < AOP_SIZE (result); i++)
        pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), i));
    }

  assert (AOP_SIZE (left) == AOP_SIZE (right));
  for (i = 0; i < AOP_SIZE (left); i++)
    {
      if (AOP_TYPE (left) != AOP_ACC)
        {
          if (pic16_isLitOp (left))
            pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (left), i));
          else
            pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), i));
        }
      if (pic16_isLitOp (right))
        {
          if (pic16_isLitOp (left) || (0 != ((lit >> (8 * i)) & 0x00FF)))
            {
              pic16_emitpcode (POC_XORLW, pic16_popGet (AOP (right), i));
            }
        }
      else
        pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (right), i));

      pic16_emitpcode (POC_BNZ, pic16_popGetLabel (falselbl->key));
    }

  // result == true

  if (generate_result && preserve_result)
    {
      for (i = 0; i < AOP_SIZE (result); i++)
        pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), i));
    }

  if (generate_result)
    pic16_emitpcode (POC_INCF, pic16_popGet (AOP (result), 0)); // result = true

  if (generate_result && preserve_result)
    pic16_emitpcode (POC_GOTO, pic16_popGetLabel (donelbl->key));

  if (ifx && IC_TRUE (ifx))
    pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ifx)->key));

  if (ifx && IC_FALSE (ifx))
    pic16_emitpcode (POC_GOTO, pic16_popGetLabel (donelbl->key));

  pic16_emitpLabel (falselbl->key);

  // result == false

  if (ifx && IC_FALSE (ifx))
    pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_FALSE (ifx)->key));

  if (generate_result && preserve_result)
    {
      for (i = 0; i < AOP_SIZE (result); i++)
        pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), i));
    }

  pic16_emitpLabel (donelbl->key);

  if (ifx)
    ifx->generated = 1;

release:
  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);

}


#if 0
// old version kept for reference

/*-----------------------------------------------------------------*/
/* genCmpEq - generates code for equal to                          */
/*-----------------------------------------------------------------*/
static void
genCmpEq (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  unsigned long lit = 0L;
  int size, offset = 0;
  symbol *falselbl = newiTempLabel (NULL);


  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  if (ifx)
    DEBUGpic16_emitcode ("; ifx is non-null", "");
  else
    DEBUGpic16_emitcode ("; ifx is null", "");

  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  pic16_aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  pic16_aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  size = max (AOP_SIZE (left), AOP_SIZE (right));

  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);

  /* if literal, literal on the right or
     if the right is in a pointer register and left
     is not */
  if ((AOP_TYPE (IC_LEFT (ic)) == AOP_LIT))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }


  if (ifx && !AOP_SIZE (result))
    {
      symbol *tlbl;
      DEBUGpic16_emitcode ("; ***", "%s  %d CASE 1", __FUNCTION__, __LINE__);
      /* if they are both bit variables */
      if (AOP_TYPE (left) == AOP_CRY && ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (right) == AOP_LIT)))
        {
          DEBUGpic16_emitcode ("; ***", "%s  %d CASE 11", __FUNCTION__, __LINE__);
          if (AOP_TYPE (right) == AOP_LIT)
            {
              unsigned long lit = ulFromVal (AOP (right)->aopu.aop_lit);
              if (lit == 0L)
                {
                  pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                  pic16_emitcode ("cpl", "c");
                }
              else if (lit == 1L)
                {
                  pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                }
              else
                {
                  pic16_emitcode ("clr", "c");
                }
              /* AOP_TYPE(right) == AOP_CRY */
            }
          else
            {
              symbol *lbl = newiTempLabel (NULL);
              pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
              pic16_emitcode ("jb", "%s,%05d_DS_", AOP (right)->aopu.aop_dir, labelKey2num (lbl->key));
              pic16_emitcode ("cpl", "c");
              pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));
            }
          /* if true label then we jump if condition
             supplied is true */
          tlbl = newiTempLabel (NULL);
          if (IC_TRUE (ifx))
            {
              pic16_emitcode ("jnc", "%05d_DS_", labelKey2num (tlbl->key));
              pic16_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_TRUE (ifx)->key));
            }
          else
            {
              pic16_emitcode ("jc", "%05d_DS_", labelKey2num (tlbl->key));
              pic16_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_FALSE (ifx)->key));
            }
          pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key + pic16_labelOffset));

          {
            /* left and right are both bit variables, result is carry */
            resolvedIfx rIfx;

            resolveIfx (&rIfx, ifx);

            pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (left), 0));
            pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (left), 0));
            pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (right), 0));
            pic16_emitpcode (POC_ANDLW, pic16_popGet (AOP (left), 0));
            genSkipz2 (&rIfx, 0);
          }
        }
      else
        {

          DEBUGpic16_emitcode ("; ***", "%s  %d CASE 12", __FUNCTION__, __LINE__);

          /* They're not both bit variables. Is the right a literal? */
          if (AOP_TYPE (right) == AOP_LIT)
            {
              lit = ulFromVal (AOP (right)->aopu.aop_lit);

              switch (size)
                {

                case 1:
                  switch (lit & 0xff)
                    {
                    case 1:
                      if (IC_TRUE (ifx))
                        {
                          pic16_emitpcode (POC_DECFW, pic16_popGet (AOP (left), offset));
                          emitSKPNZ;
                          pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ifx)->key));
                        }
                      else
                        {
                          pic16_emitpcode (POC_DECFSZW, pic16_popGet (AOP (left), offset));
                          pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_FALSE (ifx)->key));
                        }
                      break;
                    case 0xff:
                      if (IC_TRUE (ifx))
                        {
                          pic16_emitpcode (POC_INCFW, pic16_popGet (AOP (left), offset));
                          emitSKPNZ;
                          pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ifx)->key));
                        }
                      else
                        {
                          pic16_emitpcode (POC_INCFSZW, pic16_popGet (AOP (left), offset));
                          pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_FALSE (ifx)->key));
                        }
                      break;
                    default:
                      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
                      if (lit)
                        pic16_emitpcode (POC_XORLW, pic16_popGetLit (lit & 0xff));
                      genSkip (ifx, 'z');
                    }           // switch lit


                  /* end of size == 1 */
                  break;

                case 2:
                  genc16bit2lit (left, lit, offset);
                  genSkip (ifx, 'z');
                  break;
                  /* end of size == 2 */

                default:
                  /* size is 4 */
                  if (lit == 0)
                    {
                      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), 0));
                      pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (left), 1));
                      pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (left), 2));
                      pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (left), 3));
                      genSkip (ifx, 'z');
                    }
                  else
                    {
                      /* search for patterns that can be optimized */

                      genc16bit2lit (left, lit, 0);
                      lit >>= 16;
                      if (lit)
                        {
                          if (IC_TRUE (ifx))
                            emitSKPZ;   // if hi word unequal
                          else
                            emitSKPNZ;  // if hi word equal
                          // fail early
                          pic16_emitpcode (POC_GOTO, pic16_popGetLabel (falselbl->key));
                          genc16bit2lit (left, lit, 2);
                          genSkip (ifx, 'z');
                        }
                      else
                        {
                          pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (left), 2));
                          pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (left), 3));
                          genSkip (ifx, 'z');
                        }
                    }
                  pic16_emitpLabel (falselbl->key);
                  break;

                }               // switch size

              ifx->generated = 1;
              goto release;


            }
          else if (AOP_TYPE (right) == AOP_CRY)
            {
              /* we know the left is not a bit, but that the right is */
              pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
              pic16_emitpcode (((IC_TRUE (ifx)) ? POC_BTFSC : POC_BTFSS), pic16_popGet (AOP (right), offset));
              pic16_emitpcode (POC_XORLW, pic16_popGetLit (1));

              /* if the two are equal, then W will be 0 and the Z bit is set
               * we could test Z now, or go ahead and check the high order bytes if
               * the variable we're comparing is larger than a byte. */

              while (--size)
                pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (left), offset));

              if (IC_TRUE (ifx))
                {
                  emitSKPNZ;
                  pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ifx)->key));
                  // pic16_emitcode(" goto","_%05d_DS_",labelKey2num (IC_TRUE(ifx)->key + pic16_labelOffset));
                }
              else
                {
                  emitSKPZ;
                  pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_FALSE (ifx)->key));
                  // pic16_emitcode(" goto","_%05d_DS_",labelKey2num (IC_FALSE(ifx)->key + pic16_labelOffset));
                }

            }
          else
            {
              /* They're both variables that are larger than bits */
              int s = size;

              tlbl = newiTempLabel (NULL);

              while (size--)
                {
                  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
                  pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (right), offset));

                  if (IC_TRUE (ifx))
                    {
                      if (size)
                        {
                          emitSKPZ;

                          DEBUGpic16_emitcode (";", "\tIC_TRUE emitSKPZ");

                          pic16_emitpcode (POC_GOTO, pic16_popGetLabel (tlbl->key));
                          pic16_emitcode (" goto", "_%05d_DS_", labelKey2num (tlbl->key + pic16_labelOffset));
                        }
                      else
                        {
                          emitSKPNZ;

                          DEBUGpic16_emitcode (";", "\tIC_TRUE emitSKPNZ");


                          pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ifx)->key));
                          pic16_emitcode (" goto", "_%05d_DS_", labelKey2num (IC_TRUE (ifx)->key + pic16_labelOffset));
                        }
                    }
                  else
                    {
                      emitSKPZ;

                      DEBUGpic16_emitcode (";", "\tnot IC_TRUE emitSKPZ");

                      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_FALSE (ifx)->key));
                      pic16_emitcode (" goto", "_%05d_DS_", labelKey2num (IC_FALSE (ifx)->key + pic16_labelOffset));
                    }
                  offset++;
                }
              if (s > 1 && IC_TRUE (ifx))
                {
                  pic16_emitpLabel (tlbl->key);
                  pic16_emitcode ("", "_%05d_DS_:", labelKey2num (tlbl->key + pic16_labelOffset));
                }
            }
        }
      /* mark the icode as generated */
      ifx->generated = 1;
      goto release;
    }

  /* if they are both bit variables */
  if (AOP_TYPE (left) == AOP_CRY && ((AOP_TYPE (right) == AOP_CRY) || (AOP_TYPE (right) == AOP_LIT)))
    {
      DEBUGpic16_emitcode ("; ***", "%s  %d CASE 2", __FUNCTION__, __LINE__);
      if (AOP_TYPE (right) == AOP_LIT)
        {
          unsigned long lit = ulFromVal (AOP (right)->aopu.aop_lit);
          if (lit == 0L)
            {
              pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
              pic16_emitcode ("cpl", "c");
            }
          else if (lit == 1L)
            {
              pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
            }
          else
            {
              pic16_emitcode ("clr", "c");
            }
          /* AOP_TYPE(right) == AOP_CRY */
        }
      else
        {
          symbol *lbl = newiTempLabel (NULL);
          pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
          pic16_emitcode ("jb", "%s,%05d_DS_", AOP (right)->aopu.aop_dir, labelKey2num (lbl->key));
          pic16_emitcode ("cpl", "c");
          pic16_emitcode ("", "%05d_DS_:", labelKey2num (lbl->key));
        }
      /* c = 1 if egal */
      if (AOP_TYPE (result) == AOP_CRY && AOP_SIZE (result))
        {
          pic16_outBitC (result);
          goto release;
        }
      if (ifx)
        {
          genIfxJump (ifx, "c");
          goto release;
        }
      /* if the result is used in an arithmetic operation
         then put the result in place */
      pic16_outBitC (result);
    }
  else
    {

      DEBUGpic16_emitcode ("; ***", "%s  %d CASE 3", __FUNCTION__, __LINE__);
      gencjne (left, right, result, ifx);
      /*
         if(ifx)
         gencjne(left,right,newiTempLabel(NULL));
         else {
         if(IC_TRUE(ifx)->key)
         gencjne(left,right,IC_TRUE(ifx)->key);
         else
         gencjne(left,right,IC_FALSE(ifx)->key);
         ifx->generated = 1;
         goto release ;
         }
         if (AOP_TYPE(result) == AOP_CRY && AOP_SIZE(result)) {
         pic16_aopPut(AOP(result),"a",0);
         goto release ;
         }

         if (ifx) {
         genIfxJump (ifx,"a");
         goto release ;
         }
       */
      /* if the result is used in an arithmetic operation
         then put the result in place */
      /*
         if (AOP_TYPE(result) != AOP_CRY)
         pic16_outAcc(result);
       */
      /* leave the result in acc */
    }

release:
  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
}
#endif

/*-----------------------------------------------------------------*/
/* genAndOp - for && operation                                     */
/*-----------------------------------------------------------------*/
static void
genAndOp (iCode * ic)
{
  operand *left, *right, *result;
  /*     symbol *tlbl; */

  FENTRY;

  /* note here that && operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  pic16_aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  pic16_aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);

  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), 0));
  pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (right), 0));
  pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));

  /* if both are bit variables */
  /*     if (AOP_TYPE(left) == AOP_CRY && */
  /*         AOP_TYPE(right) == AOP_CRY ) { */
  /*         pic16_emitcode("mov","c,%s",AOP(left)->aopu.aop_dir); */
  /*         pic16_emitcode("anl","c,%s",AOP(right)->aopu.aop_dir); */
  /*         pic16_outBitC(result); */
  /*     } else { */
  /*         tlbl = newiTempLabel(NULL); */
  /*         pic16_toBoolean(left);     */
  /*         pic16_emitcode("jz","%05d_DS_",labelKey2num (tlbl->key)); */
  /*         pic16_toBoolean(right); */
  /*         pic16_emitcode("","%05d_DS_:",labelKey2num (tlbl->key)); */
  /*         pic16_outBitAcc(result); */
  /*     } */

  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
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

  FENTRY;

  /* note here that || operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  pic16_aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  pic16_aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);

  /* if both are bit variables */
  if (AOP_TYPE (left) == AOP_CRY && AOP_TYPE (right) == AOP_CRY)
    {
      pic16_emitcode ("clrc", "");
      pic16_emitcode ("btfss", "(%s >> 3), (%s & 7)", AOP (left)->aopu.aop_dir, AOP (left)->aopu.aop_dir);
      pic16_emitcode ("btfsc", "(%s >> 3), (%s & 7)", AOP (right)->aopu.aop_dir, AOP (right)->aopu.aop_dir);
      pic16_emitcode ("setc", "");

    }
  else
    {
      tlbl = newiTempLabel (NULL);
      pic16_toBoolean (left);
      emitSKPZ;
      pic16_emitcode ("goto", "%05d_DS_", labelKey2num (tlbl->key + pic16_labelOffset));
      pic16_toBoolean (right);
      pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key + pic16_labelOffset));

      pic16_outBitAcc (result);
    }

  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
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

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
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
  if (IC_TRUE (ic))
    pic16_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_TRUE (ic)->key));
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* jmpIfTrue -                                                     */
/*-----------------------------------------------------------------*/
static void
jumpIfTrue (iCode * ic)
{
  FENTRY;
  if (!IC_TRUE (ic))
    pic16_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_FALSE (ic)->key));
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* jmpTrueOrFalse -                                                */
/*-----------------------------------------------------------------*/
static void
jmpTrueOrFalse (iCode * ic, symbol * tlbl)
{
  // ugly but optimized by peephole
  FENTRY;
  if (IC_TRUE (ic))
    {
      symbol *nlbl = newiTempLabel (NULL);
      pic16_emitcode ("sjmp", "%05d_DS_", labelKey2num (nlbl->key));
      pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
      pic16_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_TRUE (ic)->key));
      pic16_emitcode ("", "%05d_DS_:", labelKey2num (nlbl->key));
    }
  else
    {
      pic16_emitcode ("ljmp", "%05d_DS_", labelKey2num (IC_FALSE (ic)->key));
      pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
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
  unsigned long lit = 0L;
  resolvedIfx rIfx;

  FENTRY;

  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  pic16_aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  pic16_aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  resolveIfx (&rIfx, ifx);

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (pic16_sameRegs (AOP (result), AOP (right)))
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

  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);

  // if(bit & yy)
  // result = bit & yy;
  if (AOP_TYPE (left) == AOP_CRY)
    {
      // c = bit & literal;
      if (AOP_TYPE (right) == AOP_LIT)
        {
          if (lit & 1)
            {
              if (size && pic16_sameRegs (AOP (result), AOP (left)))
                // no change
                goto release;
              pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
            }
          else
            {
              // bit(result) = 0;
              if (size && (AOP_TYPE (result) == AOP_CRY))
                {
                  pic16_emitcode ("clr", "%s", AOP (result)->aopu.aop_dir);
                  goto release;
                }
              if ((AOP_TYPE (result) == AOP_CRY) && ifx)
                {
                  jumpIfTrue (ifx);
                  goto release;
                }
              pic16_emitcode ("clr", "c");
            }
        }
      else
        {
          if (AOP_TYPE (right) == AOP_CRY)
            {
              // c = bit & bit;
              pic16_emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
              pic16_emitcode ("anl", "c,%s", AOP (left)->aopu.aop_dir);
            }
          else
            {
              // c = bit & val;
              MOVA (pic16_aopGet (AOP (right), 0, FALSE, FALSE));
              // c = lsb
              pic16_emitcode ("rrc", "a");
              pic16_emitcode ("anl", "c,%s", AOP (left)->aopu.aop_dir);
            }
        }
      // bit = c
      // val = c
      if (size)
        pic16_outBitC (result);
      // if(bit & ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c");
      goto release;
    }

  // if (val & 0xZZ)      - size = 0, ifx != FALSE -
  // bit = val & 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      symbol *tlbl = newiTempLabel (NULL);
      int sizel = AOP_SIZE (left);
      int nonnull = 0;
      char emitBra;

      if (size)
        emitSETC;

      /* get number of non null bytes in literal */
      while (sizel--)
        {
          if (lit & (0xff << (sizel * 8)))
            ++nonnull;
        }

      emitBra = nonnull || rIfx.condition;

      for (sizel = AOP_SIZE (left); sizel--; ++offset, lit >>= 8)
        {
          unsigned char bytelit = lit & 0xFF;

          if (bytelit != 0)
            {
              int posbit;

              --nonnull;

              /* patch provided by Aaron Colwell */
              if ((posbit = isLiteralBit (bytelit)) != 0)
                {
                  if (nonnull)
                    {
                      pic16_emitpcode (POC_BTFSC,
                                       pic16_newpCodeOpBit (pic16_aopGet (AOP (left), offset, FALSE, FALSE), posbit - 1, 0,
                                                            PO_GPR_REGISTER));
                      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (rIfx.condition ? rIfx.lbl->key : tlbl->key));
                    }
                  else
                    {
                      pic16_emitpcode (rIfx.condition ? POC_BTFSC : POC_BTFSS,
                                       pic16_newpCodeOpBit (pic16_aopGet (AOP (left), offset, FALSE, FALSE), posbit - 1, 0,
                                                            PO_GPR_REGISTER));
                    }
                }
              else
                {
                  if (bytelit == 0xff)
                    {
                      /* Aaron had a MOVF instruction here, changed to MOVFW cause
                       * a peephole could optimize it out -- VR */
                      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
                    }
                  else
                    {
                      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
                      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (bytelit));
                    }
                  if (nonnull)
                    {
                      if (rIfx.condition)
                        {
                          emitSKPZ;
                          pic16_emitpcode (POC_GOTO, pic16_popGetLabel (rIfx.lbl->key));        /* to false */
                        }
                      else
                        {
                          pic16_emitpcode (POC_BNZ, pic16_popGetLabel (tlbl->key));     /* to true */
                        }
                    }
                  else
                    {
                      /* last non null byte */
                      if (rIfx.condition)
                        emitSKPZ;
                      else
                        emitSKPNZ;
                    }
                }
            }
        }

      // bit = left & literal
      if (size)
        {
          emitCLRC;
          pic16_emitpLabel (tlbl->key);
        }

      // if(left & literal)
      else
        {
          if (ifx)
            {
              if (emitBra)
                pic16_emitpcode (POC_GOTO, pic16_popGetLabel (rIfx.lbl->key));
              ifx->generated = 1;
            }
          pic16_emitpLabel (tlbl->key);
          goto release;
        }
      pic16_outBitC (result);
      goto release;
    }

  /* if left is same as result */
  if (pic16_sameRegs (AOP (result), AOP (left)))
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
//        pic16_emitcode("clrf","%s",pic16_aopGet(AOP(result),offset,FALSE,FALSE));
                  pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset));
                  break;
                case 0xff:
                  /* and'ing with 0xff is a nop when the result and left are the same */
                  break;

                default:
                {
                  int p = pic16_my_powof2 ((~lit) & 0xff);
                  if (p >= 0)
                    {
                      /* only one bit is clear in the literal, so use a bcf instruction */
//            pic16_emitcode("bcf","%s,%d",pic16_aopGet(AOP(left),offset,FALSE,TRUE),p);
                      pic16_emitpcode (POC_BCF,
                                       pic16_newpCodeOpBit (pic16_aopGet (AOP (left), offset, FALSE, FALSE), p, 0,
                                                            PO_GPR_REGISTER));

                    }
                  else
                    {
                      if (AOP_TYPE (left) == AOP_ACC)
                        {
                          pic16_emitcode ("andlw", "0x%x", lit & 0xff);
                          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (lit & 0xff));
                          know_W = -1;
                        }
                      else
                        {
                          if (know_W != (lit & 0xff))
                            {
                              pic16_emitcode ("movlw", "0x%x", lit & 0xff);
                              pic16_emitpcode (POC_MOVLW, pic16_popGetLit (lit & 0xff));
                            }

                          know_W = lit & 0xff;
                          pic16_emitcode ("andwf", "%s,f", pic16_aopGet (AOP (left), offset, FALSE, TRUE));
                          pic16_emitpcode (POC_ANDWF, pic16_popGet (AOP (left), offset));
                        }
                    }
                }
                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (right), offset));
                }
              else
                {
                  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
                  pic16_emitpcode (POC_ANDWF, pic16_popGet (AOP (left), offset));

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
            pic16_emitcode ("setb", "c");
          while (sizer--)
            {
              MOVA (pic16_aopGet (AOP (right), offset, FALSE, FALSE));
              pic16_emitcode ("anl", "a,%s", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
              pic16_emitcode ("jnz", "%05d_DS_", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
              pic16_outBitC (result);
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
                      pic16_emitcode ("clrf", "%s", pic16_aopGet (AOP (result), offset, FALSE, FALSE));
                      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset));
                      break;
                    case 0xff:
                      pic16_emitcode ("movf", "%s,w", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
                      pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (result), offset, FALSE, FALSE));
                      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
                      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                      break;
                    default:
                    {
                      if (AOP_TYPE (left) == AOP_ACC)
                        {
                          pic16_emitcode ("andlw", "0x%02x", t);
                          pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (result), offset, FALSE, FALSE));
                          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (t));
                          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                        }
                      else
                        {
                          pic16_emitcode ("movlw", "0x%02x", t);
                          pic16_emitcode ("andwf", "%s,w", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
                          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (t));
                          pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (left), offset));

                          if (AOP_TYPE (result) != AOP_ACC)
                            {
                              pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (result), offset, FALSE, FALSE));
                              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                            }
                          else
                            {
                              DEBUGpic16_emitcode ("; ***", "ignore MOVWF\tWREG", __FUNCTION__, __LINE__);
                            }
                        }
                    }
                    }
                  continue;
                }

              if (AOP_TYPE (left) == AOP_ACC)
                {
                  pic16_emitcode ("andwf", "%s,w", pic16_aopGet (AOP (right), offset, FALSE, FALSE));
                  pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (right), offset));
                }
              else
                {
                  pic16_emitcode ("movf", "%s,w", pic16_aopGet (AOP (right), offset, FALSE, FALSE));
                  pic16_emitcode ("andwf", "%s,w", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
                  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
                  pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (left), offset));
                }
              pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (result), offset, FALSE, FALSE));
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
            }
        }
    }

release:
  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
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
  resolvedIfx rIfx;

  FENTRY;

  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  pic16_aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  pic16_aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  resolveIfx (&rIfx, ifx);

  /* if left is a literal & right is not then exchange them */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || AOP_NEEDSACC (left))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (pic16_sameRegs (AOP (result), AOP (right)))
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

  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);

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
                    pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), 0));
                  //pic16_emitcode("bsf","(%s >> 3), (%s & 7)",
                  //     AOP(result)->aopu.aop_dir,
                  //     AOP(result)->aopu.aop_dir);
                  else if (ifx)
                    continueIfTrue (ifx);
                  goto release;
                }
            }
          else
            {
              // lit == 0 => result = left
              if (size && pic16_sameRegs (AOP (result), AOP (left)))
                goto release;
              pic16_emitcode (";XXX mov", "c,%s  %s,%d", AOP (left)->aopu.aop_dir, __FILE__, __LINE__);
            }
        }
      else
        {
          if (AOP_TYPE (right) == AOP_CRY)
            {
              if (pic16_sameRegs (AOP (result), AOP (left)))
                {
                  // c = bit | bit;
                  pic16_emitpcode (POC_BCF, pic16_popGet (AOP (result), 0));
                  pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (right), 0));
                  pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), 0));

                }
              else
                {
                  if (AOP_TYPE (result) == AOP_ACC)
                    {
                      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0));
                      pic16_emitpcode (POC_BTFSS, pic16_popGet (AOP (right), 0));
                      pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (left), 0));
                      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (1));

                    }
                  else
                    {

                      pic16_emitpcode (POC_BCF, pic16_popGet (AOP (result), 0));
                      pic16_emitpcode (POC_BTFSS, pic16_popGet (AOP (right), 0));
                      pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (left), 0));
                      pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), 0));

                    }
                }
            }
          else
            {
              // c = bit | val;
              symbol *tlbl = newiTempLabel (NULL);
              pic16_emitcode (";XXX ", " %s,%d", __FILE__, __LINE__);


              pic16_emitpcode (POC_BCF, pic16_popGet (AOP (result), 0));
              if (AOP_TYPE (right) == AOP_ACC)
                {
                  pic16_emitpcode (POC_IORLW, pic16_popGetLit (0));
                  emitSKPNZ;
                  pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (left), 0));
                  pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), 0));
                }



              if (!((AOP_TYPE (result) == AOP_CRY) && ifx))
                pic16_emitcode (";XXX setb", "c");
              pic16_emitcode (";XXX jb", "%s,%05d_DS_", AOP (left)->aopu.aop_dir, labelKey2num (tlbl->key));
              pic16_toBoolean (right);
              pic16_emitcode (";XXX jnz", "%05d_DS_", labelKey2num (tlbl->key));
              if ((AOP_TYPE (result) == AOP_CRY) && ifx)
                {
                  jmpTrueOrFalse (ifx, tlbl);
                  goto release;
                }
              else
                {
                  CLRC;
                  pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
                }
            }
        }
      // bit = c
      // val = c
      if (size)
        pic16_outBitC (result);
      // if(bit | ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c");
      goto release;
    }

  // if(val | 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val | 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      if (IS_OP_VOLATILE (left))
        {
          pic16_mov2w_volatile (AOP (left));
        }                       // if
      if (lit)
        {
          if (rIfx.condition)
            pic16_emitpcode (POC_GOTO, pic16_popGetLabel (rIfx.lbl->key));      /* to false */
          ifx->generated = 1;
        }
      else
        wassert (0);

      goto release;
    }

  /* if left is same as result */
  if (pic16_sameRegs (AOP (result), AOP (left)))
    {
      int know_W = -1;
      for (; size--; offset++, lit >>= 8)
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              if (((lit & 0xff) == 0) && !IS_OP_VOLATILE (left))
                {
                  /*  or'ing with 0 has no effect */
                  continue;
                }
              else
                {
                  int p = pic16_my_powof2 (lit & 0xff);
                  if (p >= 0)
                    {
                      /* only one bit is set in the literal, so use a bsf instruction */
                      pic16_emitpcode (POC_BSF,
                                       pic16_newpCodeOpBit (pic16_aopGet (AOP (left), offset, FALSE, FALSE), p, 0,
                                                            PO_GPR_REGISTER));
                    }
                  else
                    {
                      if (AOP_TYPE (left) == AOP_ACC)
                        {
                          pic16_emitpcode (POC_IORLW, pic16_popGetLit (lit & 0xff));
                          know_W = -1;
                        }
                      else
                        {
                          if (know_W != (lit & 0xff))
                            {
                              pic16_emitpcode (POC_MOVLW, pic16_popGetLit (lit & 0xff));
                            }

                          know_W = lit & 0xff;
                          pic16_emitpcode (POC_IORWF, pic16_popGet (AOP(left), offset));
                        }
                    }

                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (right), offset));
                }
              else
                {
                  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
                  pic16_emitpcode (POC_IORWF, pic16_popGet (AOP (left), offset));
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
          pic16_emitcode (";XXX ", " %s,%d", __FILE__, __LINE__);


          if (size)
            pic16_emitcode (";XXX setb", "c");
          while (sizer--)
            {
              MOVA (pic16_aopGet (AOP (right), offset, FALSE, FALSE));
              pic16_emitcode (";XXX orl", "a,%s", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
              pic16_emitcode (";XXX jnz", "%05d_DS_", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
              pic16_outBitC (result);
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
                    pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
                    pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                    break;
                  default:
                  {
                    if (AOP_TYPE (left) == AOP_ACC)
                      {
                        pic16_emitcode ("iorlw", "0x%02x", t);
                        pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (result), offset, FALSE, FALSE));
                        pic16_emitpcode (POC_IORLW, pic16_popGetLit (t));
                        pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                      }
                    else
                      {
                        pic16_emitcode("movlw", "0x%02x", t);
                        pic16_emitcode("iorwf", "%s", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
                        pic16_emitpcode(POC_MOVLW, pic16_popGetLit (t));
                        pic16_emitpcode(POC_IORFW, pic16_popGet (AOP (left), offset));

                        if (AOP_TYPE (result) != AOP_ACC)
                          {
                            pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (result), offset, FALSE, FALSE));
                            pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                          }
                        else
                          {
                            DEBUGpic16_emitcode ("; ***", "ignore MOVWF\tWREG", __FUNCTION__, __LINE__);
                          }
                      }
                  }
                  }
                continue;
              }

            // faster than result <- left, anl result,right
            // and better if result is SFR
            if (AOP_TYPE (left) == AOP_ACC)
              {
                pic16_emitpcode (POC_IORWF, pic16_popGet (AOP (right), offset));
              }
            else
              {
                pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
                pic16_emitpcode (POC_IORFW, pic16_popGet (AOP (left), offset));
              }
            pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
          }
    }

release:
  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
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
  resolvedIfx rIfx;

  FENTRY;

  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  pic16_aopOp ((right = IC_RIGHT (ic)), ic, FALSE);
  pic16_aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  resolveIfx (&rIfx, ifx);

  /* if left is a literal & right is not ||
     if left needs acc & right does not */
  if ((AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT) || (AOP_NEEDSACC (left) && !AOP_NEEDSACC (right)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if result = right then exchange them */
  if (pic16_sameRegs (AOP (result), AOP (right)))
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
                      pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), offset));
                    }
                  else if (ifx)
                    continueIfTrue (ifx);
                  goto release;
                }
              pic16_emitcode ("setb", "c");
            }
          else
            {
              // lit == (0 or 1)
              if (lit == 0)
                {
                  // lit == 0, result = left
                  if (size && pic16_sameRegs (AOP (result), AOP (left)))
                    goto release;
                  pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                }
              else
                {
                  // lit == 1, result = not(left)
                  if (size && pic16_sameRegs (AOP (result), AOP (left)))
                    {
                      pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (result), offset));
                      pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (result), offset));
                      pic16_emitcode ("cpl", "%s", AOP (result)->aopu.aop_dir);
                      goto release;
                    }
                  else
                    {
                      pic16_emitcode ("mov", "c,%s", AOP (left)->aopu.aop_dir);
                      pic16_emitcode ("cpl", "c");
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
              pic16_emitcode ("mov", "c,%s", AOP (right)->aopu.aop_dir);
            }
          else
            {
              int sizer = AOP_SIZE (right);
              // c = bit ^ val
              // if val>>1 != 0, result = 1
              pic16_emitcode ("setb", "c");
              while (sizer)
                {
                  MOVA (pic16_aopGet (AOP (right), sizer - 1, FALSE, FALSE));
                  if (sizer == 1)
                    // test the msb of the lsb
                    pic16_emitcode ("anl", "a,#0xfe");
                  pic16_emitcode ("jnz", "%05d_DS_", labelKey2num (tlbl->key));
                  sizer--;
                }
              // val = (0,1)
              pic16_emitcode ("rrc", "a");
            }
          pic16_emitcode ("jnb", "%s,%05d_DS_", AOP (left)->aopu.aop_dir, labelKey2num (tlbl->key));
          pic16_emitcode ("cpl", "c");
          pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
        }
      // bit = c
      // val = c
      if (size)
        pic16_outBitC (result);
      // if(bit | ...)
      else if ((AOP_TYPE (result) == AOP_CRY) && ifx)
        genIfxJump (ifx, "c");
      goto release;
    }

  // if(val ^ 0xZZ)       - size = 0, ifx != FALSE  -
  // bit = val ^ 0xZZ     - size = 1, ifx = FALSE -
  if ((AOP_TYPE (right) == AOP_LIT) && (AOP_TYPE (result) == AOP_CRY) && (AOP_TYPE (left) != AOP_CRY))
    {
      symbol *tlbl = newiTempLabel (NULL);
      int sizel;

      if (size)
        emitSETC;

      for (sizel = AOP_SIZE (left); sizel--; ++offset, lit >>= 8)
        {
          unsigned char bytelit = lit & 0xFF;

          switch (bytelit)
            {
            case 0xff:
              pic16_emitpcode (POC_COMFW, pic16_popGet (AOP (left), offset));
              break;

            case 0x00:
              pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
              break;

            default:
              pic16_emitpcode (POC_MOVLW, pic16_popGetLit (bytelit));
              pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (left), offset));
              break;
            }
          if (sizel)
            {
              if (rIfx.condition)
                {
                  /* rIfx.lbl might be far away... */
                  emitSKPZ;
                  pic16_emitpcode (POC_GOTO, pic16_popGetLabel (rIfx.lbl->key));        /* to false */
                }
              else
                {
                  pic16_emitpcode (POC_BNZ, pic16_popGetLabel (tlbl->key));     /* to true */
                }
            }
          else
            {
              /* last non null byte */
              if (rIfx.condition)
                emitSKPZ;
              else
                emitSKPNZ;
            }
        }

      // bit = left ^ literal
      if (size)
        {
          emitCLRC;
          pic16_emitpLabel (tlbl->key);
        }
      // if (left ^ literal)
      else
        {
          if (ifx)
            {
              pic16_emitpcode (POC_GOTO, pic16_popGetLabel (rIfx.lbl->key));
              ifx->generated = 1;
            }
          pic16_emitpLabel (tlbl->key);
          goto release;
        }

      pic16_outBitC (result);
      goto release;
    }

  if (pic16_sameRegs (AOP (result), AOP (left)))
    {
      /* if left is same as result */
      int know_W = -1;

      for (; size--; offset++)
        {
          if (AOP_TYPE (right) == AOP_LIT)
            {
              int t = (lit >> (offset * 8)) & 0x0FFL;
              if (t == 0x00L)
                continue;
              else
                {
                  int p = pic16_my_powof2 (t);

                  if (p >= 0)
                    {
                      /* Only one bit is toggle in the literal, so use a btg instruction. */
                      pic16_emitpcode (POC_BTG,
                                       pic16_newpCodeOpBit (pic16_aopGet (AOP (left), offset, FALSE, FALSE), p, 0, PO_GPR_REGISTER));
                    }
                  else
                    {
                      if (AOP_TYPE (left) == AOP_ACC)
                        {
                          pic16_emitcode ("xorlw", "0x%02x", t);
                          pic16_emitpcode (POC_XORLW, pic16_popGetLit (t));
                          know_W = -1;
                        }
                      else
                        {
                          if (know_W != t)
                            {
                              pic16_emitcode ("movlw", "0x%02x", t);
                              pic16_emitpcode (POC_MOVLW, pic16_popGetLit (t));
                            }

                          know_W = t;
                        }

                      pic16_emitcode ("xorwf", "%s", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
                      pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (left), offset));
                    }
                }
            }
          else
            {
              if (AOP_TYPE (left) == AOP_ACC)
                pic16_emitcode ("xrl", "a,%s", pic16_aopGet (AOP (right), offset, FALSE, FALSE));
              else
                {
                  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
                  pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (left), offset));
                }
            }
        }
    }
  else
    {
      // left ^ result in different registers
      if (AOP_TYPE (result) == AOP_CRY)
        {
          // result = bit
          // if(size), result in bit
          // if(!size && ifx), conditional oper: if(left ^ right)
          symbol *tlbl = newiTempLabel (NULL);
          int sizer = max (AOP_SIZE (left), AOP_SIZE (right));
          if (size)
            pic16_emitcode ("setb", "c");
          while (sizer--)
            {
              if ((AOP_TYPE (right) == AOP_LIT) && (((lit >> (offset * 8)) & 0x0FFL) == 0x00L))
                {
                  MOVA (pic16_aopGet (AOP (left), offset, FALSE, FALSE));
                }
              else
                {
                  MOVA (pic16_aopGet (AOP (right), offset, FALSE, FALSE));
                  pic16_emitcode ("xrl", "a,%s", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
                }
              pic16_emitcode ("jnz", "%05d_DS_", labelKey2num (tlbl->key));
              offset++;
            }
          if (size)
            {
              CLRC;
              pic16_emitcode ("", "%05d_DS_:", labelKey2num (tlbl->key));
              pic16_outBitC (result);
            }
          else if (ifx)
            jmpTrueOrFalse (ifx, tlbl);
        }
      else
        {
          for (; (size--); offset++)
            {
              // normal case
              // result = left ^ right
              if (AOP_TYPE (right) == AOP_LIT)
                {
                  int t = (lit >> (offset * 8)) & 0x0FFL;
                  switch (t)
                    {
                    case 0x00:
                      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offset));
                      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                      break;

                    case 0xff:
                      pic16_emitpcode (POC_COMFW, pic16_popGet (AOP (left), offset));
                      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                      break;

                    default:
                    {
                      if (AOP_TYPE (left) == AOP_ACC)
                        {
                          pic16_emitcode ("xorlw", "0x%02x", t);
                          pic16_emitpcode (POC_XORLW, pic16_popGetLit (t));
                        }
                      else
                        {
                          pic16_emitcode ("movlw", "0x%02x", t);
                          pic16_emitcode ("xorwf", "%s", pic16_aopGet (AOP (left), offset, FALSE, FALSE));
                          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (t));
                          pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (left), offset));
                        }

                      pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (result), offset, FALSE, FALSE));
                      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                    }
                    }
                  continue;
                }

              // faster than result <- left, anl result,right
              // and better if result is SFR
              if (AOP_TYPE (left) == AOP_ACC)
                {
                  pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (right), offset));
                }
              else
                {
                  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
                  pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (left), offset));
                }
              if (AOP_TYPE (result) != AOP_ACC)
                {
                  pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                }
              else
                {
                  DEBUGpic16_emitcode ("; ***", "ignore MOVWF\tWREG", __FUNCTION__, __LINE__);
                }
            }
        }
    }

release:
  pic16_freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? FALSE : TRUE));
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genInline - write the inline code out                           */
/*-----------------------------------------------------------------*/
static void
pic16_genInline (iCode * ic)
{
  char *buffer, *bp, *bp1;
  bool inComment = FALSE;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  genLine.lineElement.isInline += (!options.asmpeep);

  buffer = bp = bp1 = Safe_strdup (IC_INLINE (ic));

  while ((bp1 = strstr (bp, "\\n")))
    {
      *bp1++ = '\n';
      *bp1++ = ' ';
      bp = bp1;
    }
  bp = bp1 = buffer;

#if 0
  /* This is an experimental code for #pragma inline
     and is temporarily disabled for 2.5.0 release */
  if (asmInlineMap)
    {
      symbol *sym;
      char *s;
      char *cbuf;
      int cblen;

      cbuf = Safe_strdup (buffer);
      cblen = strlen (buffer) + 1;
      memset (cbuf, 0, cblen);

      bp = buffer;
      bp1 = cbuf;
      while (*bp)
        {
          if (*bp != '%')
            *bp1++ = *bp++;
          else
            {
              int i;

              bp++;
              i = *bp - '0';
              if (i > elementsInSet (asmInlineMap))
                break;

              bp++;
              s = indexSet (asmInlineMap, i);
              DEBUGpc ("searching symbol s = `%s'", s);
              sym = findSym (SymbolTab, NULL, s);

              if (sym->reqv)
                {
                  strcat (bp1, sym->reqv->operand.symOperand->regs[0]->name);
                }
              else
                {
                  strcat (bp1, sym->rname);
                }

              while (*bp1)
                bp1++;
            }

          if (strlen (bp1) > cblen - 16)
            {
              int i = strlen (cbuf);
              cblen += 50;
              cbuf = realloc (cbuf, cblen);
              memset (cbuf + i, 0, 50);
              bp1 = cbuf + i;
            }
        }

      free (buffer);
      buffer = Safe_strdup (cbuf);
      free (cbuf);

      bp = bp1 = buffer;
    }
#endif /* 0 */

  /* emit each line as a code */
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
            pic16_addpCode2pBlock (pb, pic16_newpCodeAsmDir (bp1, NULL));       // inline directly, no process
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
              pic16_addpCode2pBlock (pb, pic16_newpCodeAsmDir (NULL, bp1));     // inline directly, no process
              bp1 = bp;
            }
          else
            ++bp;
          break;
        }
    }

  if ((bp1 != bp) && *bp1)
    pic16_addpCode2pBlock (pb, pic16_newpCodeAsmDir (bp1, NULL));       // inline directly, no process

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
  int size, same;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  /* rotate right with carry */
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  DEBUGpic16_pic16_AopType (__LINE__, left, NULL, result);

  same = pic16_sameRegs (AOP (result), AOP (left));

  size = AOP_SIZE (result);

  DEBUGpic16_emitcode ("; ***", "%s  %d size:%d same:%d", __FUNCTION__, __LINE__, size, same);

  /* get the lsb and put it into the carry */
  pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), 0));

  while (size--)
    {

      if (same)
        {
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (left), size));
        }
      else
        {
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), size));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), size));
        }
    }

  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
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

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* rotate right with carry */
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  DEBUGpic16_pic16_AopType (__LINE__, left, NULL, result);

  same = pic16_sameRegs (AOP (result), AOP (left));

  /* move it to the result */
  size = AOP_SIZE (result);

  /* get the msb and put it into the carry */
  pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), size - 1));

  offset = 0;

  while (size--)
    {

      if (same)
        {
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (left), offset));
        }
      else
        {
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), offset));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
        }

      offset++;
    }


  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
}


/* gpasm can get the highest order bit with HIGH/UPPER
 * so the following probably is not needed -- VR */

/*-----------------------------------------------------------------*/
/* genGetHbit - generates code get highest order bit               */
/*-----------------------------------------------------------------*/
static void
genGetHbit (iCode * ic)
{
  operand *left, *result;
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (result, ic, FALSE);

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* get the highest order byte into a */
  MOVA (pic16_aopGet (AOP (left), AOP_SIZE (left) - 1, FALSE, FALSE));
  if (AOP_TYPE (result) == AOP_CRY)
    {
      pic16_emitcode ("rlc", "a");
      pic16_outBitC (result);
    }
  else
    {
      pic16_emitcode ("rl", "a");
      pic16_emitcode ("anl", "a,#0x01");
      pic16_outAcc (result);
    }


  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
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

  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (right, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  shCount = (int) ulFromVal (AOP (right)->aopu.aop_lit);
  offset = shCount / 8;
  shCount %= 8;

  /* load and mask the source byte */
  pic16_mov2w (AOP (left), offset);
  pic16_emitpcode (POC_ANDLW, pic16_popGetLit (1 << shCount));

  /* move selected bit to bit 0 */
  switch (shCount)
    {
    case 0:
      /* nothing more to do */
      break;
    case 1:
      /* shift bit 1 into bit 0 */
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 4:
      /* shift bit 4 into bit 0 */
      pic16_emitpcode (POC_SWAPFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 7:
      /* shift bit 7 into bit 0 */
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    default:
      /* keep W==0, force W=0x01 otherwise */
      emitSKPZ;
      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (1));
      break;
    }                           // switch

  /* write result */
  pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));

  for (i = 1; i < AOP_SIZE (result); ++i)
    {
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), i));
    }                           // for

  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (right, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

#if 0
/*-----------------------------------------------------------------*/
/* AccRol - rotate left accumulator by known count                 */
/*-----------------------------------------------------------------*/
static void
AccRol (int shCount)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  shCount &= 0x0007;            // shCount : 0..7
  switch (shCount)
    {
    case 0:
      break;
    case 1:
      pic16_emitcode ("rl", "a");
      break;
    case 2:
      pic16_emitcode ("rl", "a");
      pic16_emitcode ("rl", "a");
      break;
    case 3:
      pic16_emitcode ("swap", "a");
      pic16_emitcode ("rr", "a");
      break;
    case 4:
      pic16_emitcode ("swap", "a");
      break;
    case 5:
      pic16_emitcode ("swap", "a");
      pic16_emitcode ("rl", "a");
      break;
    case 6:
      pic16_emitcode ("rr", "a");
      pic16_emitcode ("rr", "a");
      break;
    case 7:
      pic16_emitcode ("rr", "a");
      break;
    }
}
#endif

/*-----------------------------------------------------------------*/
/* AccLsh - left shift accumulator by known count                  */
/*-----------------------------------------------------------------*/
static void
AccLsh (int shCount, int doMask)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  switch (shCount)
    {
    case 0:
      return;
      break;
    case 1:
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 2:
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 3:
      pic16_emitpcode (POC_SWAPFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 4:
      pic16_emitpcode (POC_SWAPFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 5:
      pic16_emitpcode (POC_SWAPFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 6:
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 7:
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    }
  if (doMask)
    {
      /* no masking is required in genPackBits */
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (SLMask[shCount]));
    }
}

/*-----------------------------------------------------------------*/
/* AccRsh - right shift accumulator by known count                 */
/*-----------------------------------------------------------------*/
static void
AccRsh (int shCount, int andmask)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  assert ((shCount >= 0) && (shCount <= 8));
  switch (shCount)
    {
    case 0:
      return;
      break;
    case 1:
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 2:
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 3:
      pic16_emitpcode (POC_SWAPFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 4:
      pic16_emitpcode (POC_SWAPFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 5:
      pic16_emitpcode (POC_SWAPFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 6:
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    case 7:
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      break;
    default:
      // Rotating by 8 is a NOP.
      break;
    }

  if (andmask)
    pic16_emitpcode (POC_ANDLW, pic16_popGetLit (SRMask[shCount]));
  else
    DEBUGpic16_emitcode ("; ***", "%s omitting masking the result", __FUNCTION__);
}

/*-----------------------------------------------------------------*/
/* shiftR1Left2Result - shift right one byte from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftR1Left2ResultSigned (operand * left, int offl, operand * result, int offr, int shCount)
{
  int same;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  assert ((shCount >= 0) && (shCount <= 8));

  same = ((left == result) || (AOP (left) == AOP (result))) && (offl == offr);

  /* Do NOT use result for intermediate results, it might be an SFR!. */
  switch (shCount)
    {
    case 0:
      if (!same)
        {
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }
      break;

    case 1:
      pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), offl));
      if (same)
        pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
      else
        {
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }
      break;

    case 2:
      pic16_emitpcode (POC_RRNCFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x3f));      // keep sign bit in W<5>
      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (PCOP (&pic16_pc_wreg), 5));
      pic16_emitpcode (POC_IORLW, pic16_popGetLit (0xc0));      // sign-extend
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 3:
      pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x1f));      // keep sign in W<4>
      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (PCOP (&pic16_pc_wreg), 4));
      pic16_emitpcode (POC_IORLW, pic16_popGetLit (0xe0));      // sign-extend
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 4:
      pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x0f));      // keep sign in W<3>
      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (PCOP (&pic16_pc_wreg), 3));
      pic16_emitpcode (POC_IORLW, pic16_popGetLit (0xf0));      // sign-extend
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 5:
      pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x07));      // keep sign in W<2>
      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (PCOP (&pic16_pc_wreg), 2));
      pic16_emitpcode (POC_IORLW, pic16_popGetLit (0xf8));      // sign-extend
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 6:
      pic16_emitpcode (POC_RLNCFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x03));      // keep sign bit in W<1>
      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (PCOP (&pic16_pc_wreg), 1));
      pic16_emitpcode (POC_IORLW, pic16_popGetLit (0xfc));      // sign-extend
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 7:
      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0x00));
      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (left), offl), 7));
      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xff));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    default:
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offr));
      break;
    }
}

/*-----------------------------------------------------------------*/
/* shiftR1Left2Result - shift right one byte from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftR1Left2Result (operand * left, int offl, operand * result, int offr, int shCount, int sign)
{
  int same;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  assert ((shCount >= 0) && (shCount <= 8));

  same = ((left == result) || (AOP (left) == AOP (result))) && (offl == offr);

  /* Copy the msb into the carry if signed. */
  if (sign)
    {
      shiftR1Left2ResultSigned (left, offl, result, offr, shCount);
      return;
    }

  /* Do NOT use result for intermediate results, it might be an SFR!. */
  switch (shCount)
    {
    case 0:
      if (!same)
        {
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }
      break;

    case 1:
      if (same)
        {
          emitCLRC;
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
        }
      else
        {
          pic16_emitpcode (POC_RRNCFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x7f));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }
      break;

    case 2:
      pic16_emitpcode (POC_RRNCFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x3f));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 3:
      pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x1f));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 4:
      pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x0f));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 5:
      pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x07));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 6:
      pic16_emitpcode (POC_RLNCFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x03));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 7:
      pic16_emitpcode (POC_RLNCFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x01));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    default:
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offr));
      break;
    }
}

/*-----------------------------------------------------------------*/
/* shiftL1Left2Result - shift left one byte from left to result    */
/*-----------------------------------------------------------------*/
static void
shiftL1Left2Result (operand * left, int offl, operand * result, int offr, int shCount)
{
  int same;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  assert ((shCount >= 0) && (shCount <= 8));

  same = ((left == result) || (AOP (left) == AOP (result))) && (offl == offr);

  /* Do NOT use result for intermediate results, it might be an SFR!. */
  switch (shCount)
    {
    case 0:
      if (!same)
        {
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }
      break;

    case 1:
      if (same)
        {
          emitCLRC;
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (left), offl));
        }
      else
        {
          pic16_emitpcode (POC_RLNCFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xfe));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }
      break;

    case 2:
    {
      if (same)
        {
          if (AOP_TYPE (left) == AOP_ACC)
            {
              /* Reduces the number of instructions. */
              pic16_emitpcode(POC_ADDFW, pic16_popGet (AOP (left), offl));
              pic16_emitpcode(POC_ADDFW, pic16_popGet (AOP (left), offl));
            }
          else
            {
              emitCLRC;
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (left), offl));
              emitCLRC;
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (left), offl));
            }
        }
      else
        {
          pic16_emitpcode (POC_RLNCFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xfc));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }

      break;
    }

    case 3:
      if (same && AOP_TYPE(left) == AOP_ACC)
        {
          /* Reduces the number of instructions. */
          pic16_emitpcode (POC_ADDFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_ADDFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_ADDFW, pic16_popGet (AOP (left), offl));
        }
      else
        {
          pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xf8));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }
      break;

    case 4:
      pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xf0));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 5:
      pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RLNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xe0));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 6:
      pic16_emitpcode (POC_RRNCFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RRNCFW, pic16_popCopyReg (&pic16_pc_wreg));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xc0));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    case 7:
      pic16_emitpcode (POC_RRNCFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x80));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      break;

    default:
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offr));
      break;
    }
}

/*-----------------------------------------------------------------*/
/* movLeft2Result - move byte from left to result                  */
/*-----------------------------------------------------------------*/
static void
movLeft2Result (operand * left, int offl, operand * result, int offr)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  if (!pic16_sameRegs (AOP (left), AOP (result)) || (offl != offr))
    {
      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
    }
}

/*-----------------------------------------------------------------*/
/* shiftL2Left2Result - shift left two bytes from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftL2Left2Result (operand * left, int offl, operand * result, int offr, int shCount)
{
  int same = pic16_sameRegs (AOP (result), AOP (left));
  int i;

  DEBUGpic16_emitcode ("; ***", "%s  %d shCount:%d same:%d offl:%d offr:%d", __FUNCTION__, __LINE__, shCount, same, offl, offr);

  if (same && (offl != offr))   // shift bytes
    {
      if (offr > offl)
        {
          for (i = 1; i > -1; i--)
            {
              pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (result), offl + i));
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + i));
            }
        }
      else                      // just treat as different later on
        {
          same = 0;
        }
    }

  if (same)
    {
      switch (shCount)
        {
        case 0:
          break;
        case 1:
        case 2:
        case 3:

          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_ADDWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));

          while (--shCount)
            {
              emitCLRC;
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr));
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));
            }

          break;
        case 4:
        case 5:
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0x0f));
          pic16_emitpcode (POC_ANDWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_SWAPF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_SWAPF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_ADDWF, pic16_popGet (AOP (result), offr + MSB16));
          if (shCount >= 5)
            {
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr));
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));
            }
          break;
        case 6:
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xc0));
          pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          break;
        case 7:
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
        }

    }
  else
    {
      switch (shCount)
        {
        case 0:
          break;
        case 1:
        case 2:
        case 3:
          /* note, use a mov/add for the shift since the mov has a
             chance of getting optimized out */
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_ADDWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), offl + MSB16));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));

          while (--shCount)
            {
              emitCLRC;
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr));
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));
            }
          break;

        case 4:
        case 5:
          pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl + MSB16));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xF0));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x0F));
          pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_ADDWF, pic16_popGet (AOP (result), offr + MSB16));


          if (shCount == 5)
            {
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr));
              pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));
            }
          break;
        case 6:
          pic16_emitpcode (POC_RRNCFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RRNCF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RRNCFW, pic16_popGet (AOP (left), offl + MSB16));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RRNCF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xc0));
          pic16_emitpcode (POC_ANDWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_IORWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (result), offr));
          break;
        case 7:
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), offl + MSB16));
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
        }
    }

}

/*-----------------------------------------------------------------*/
/* shiftR2Left2Result - shift right two bytes from left to result  */
/*-----------------------------------------------------------------*/
static void
shiftR2Left2Result (operand * left, int offl, operand * result, int offr, int shCount, int sign)
{
  int same = pic16_sameRegs (AOP (result), AOP (left));
  int i;
  DEBUGpic16_emitcode ("; ***", "%s  %d shCount:%d same:%d sign:%d", __FUNCTION__, __LINE__, shCount, same, sign);

  if (same && (offl != offr))   // shift right bytes
    {
      if (offr < offl)
        {
          for (i = 0; i < 2; i++)
            {
              pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (result), offl + i));
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + i));
            }
        }
      else                      // just treat as different later on
        {
          same = 0;
        }
    }

  switch (shCount)
    {
    case 0:
      break;
    case 1:
    case 2:
    case 3:
      /* obtain sign from left operand */
      if (sign)
        pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), offr + MSB16));
      else
        emitCLRC;

      if (same)
        {
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
        }
      else
        {
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), offl + MSB16));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }

      while (--shCount)
        {
          if (sign)
            /* now get sign from already assigned result (avoid BANKSEL) */
            pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (result), offr + MSB16));
          else
            emitCLRC;
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
        }
      break;
    case 4:
    case 5:
      if (same)
        {

          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xf0));
          pic16_emitpcode (POC_ANDWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_SWAPF, pic16_popGet (AOP (result), offr));

          pic16_emitpcode (POC_SWAPF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_ANDFW, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_ADDWF, pic16_popGet (AOP (result), offr));
        }
      else
        {
          pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x0f));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));

          pic16_emitpcode (POC_SWAPFW, pic16_popGet (AOP (left), offl + MSB16));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0xf0));
          pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_ADDWF, pic16_popGet (AOP (result), offr));
        }

      if (shCount >= 5)
        {
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (result), offr));
        }

      if (sign)
        {
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xf0 + (shCount - 4) * 8));
          pic16_emitpcode (POC_BTFSC,
                           pic16_newpCodeOpBit (pic16_aopGet (AOP (result), offr + MSB16, FALSE, FALSE), 7 - shCount, 0,
                                                PO_GPR_REGISTER));
          pic16_emitpcode (POC_ADDWF, pic16_popGet (AOP (result), offr + MSB16));
        }

      break;

    case 6:
      if (same)
        {

          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));

          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x03));
          if (sign)
            {
              pic16_emitpcode (POC_BTFSC,
                               pic16_newpCodeOpBit (pic16_aopGet (AOP (result), offr, FALSE, FALSE), 0, 0, PO_GPR_REGISTER));
              pic16_emitpcode (POC_IORLW, pic16_popGetLit (0xfc));
            }
          pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_XORWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_XORFW, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
        }
      else
        {
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), offl));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), offl + MSB16));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr));
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (result), offr + MSB16));
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit (0x03));
          if (sign)
            {
              pic16_emitpcode (POC_BTFSC,
                               pic16_newpCodeOpBit (pic16_aopGet (AOP (result), offr + MSB16, FALSE, FALSE), 0, 0,
                                                    PO_GPR_REGISTER));
              pic16_emitpcode (POC_IORLW, pic16_popGetLit (0xfc));
            }
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr + MSB16));
          //pic16_emitpcode(POC_RLCF,  pic16_popGet(AOP(result),offr));


        }

      break;
    case 7:
      pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), offl));
      pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), offl + MSB16));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offr));
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offr + MSB16));
      if (sign)
        {
          emitSKPNC;
          pic16_emitpcode (POC_DECF, pic16_popGet (AOP (result), offr + MSB16));
        }
      else
        pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (result), offr + MSB16));
    }
}


/*-----------------------------------------------------------------*/
/* shiftLLeftOrResult - shift left one byte from left, or to result*/
/*-----------------------------------------------------------------*/
static void
shiftLLeftOrResult (operand * left, int offl, operand * result, int offr, int shCount)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offl));
  /* shift left accumulator */
  AccLsh (shCount, 1);
  /* or with result */
  /* back to result */
  pic16_emitpcode (POC_IORWF, pic16_popGet (AOP (result), offr));
}

/*-----------------------------------------------------------------*/
/* shiftRLeftOrResult - shift right one byte from left,or to result*/
/*-----------------------------------------------------------------*/
static void
shiftRLeftOrResult (operand * left, int offl, operand * result, int offr, int shCount)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), offl));
  /* shift right accumulator */
  AccRsh (shCount, 1);
  /* or with result */
  /* back to result */
  pic16_emitpcode (POC_IORWF, pic16_popGet (AOP (result), offr));
}

/*-----------------------------------------------------------------*/
/* genlshOne - left shift a one byte quantity by known count       */
/*-----------------------------------------------------------------*/
static void
genlshOne (operand * result, operand * left, int shCount)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  shiftL1Left2Result (left, LSB, result, LSB, shCount);
}

/*-----------------------------------------------------------------*/
/* genlshTwo - left shift two bytes by known amount != 0           */
/*-----------------------------------------------------------------*/
static void
genlshTwo (operand * result, operand * left, int shCount)
{
  int size;

  DEBUGpic16_emitcode ("; ***", "%s  %d shCount:%d", __FUNCTION__, __LINE__, shCount);
  size = pic16_getDataSize (result);

  /* if shCount >= 8 */
  if (shCount >= 8)
    {
      shCount -= 8;

      if (size > 1)
        {
          if (shCount)
            shiftL1Left2Result (left, LSB, result, MSB16, shCount);
          else
            movLeft2Result (left, LSB, result, MSB16);
        }
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), LSB));
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
/* offr = LSB or MSB16                                             */
/*-----------------------------------------------------------------*/
static void
shiftLLong (operand * left, operand * result, int offr)
{
  int size = AOP_SIZE (result);
  int same = pic16_sameRegs (AOP (left), AOP (result));
  int i;

  DEBUGpic16_emitcode ("; ***", "%s  %d  offr:%d size:%d", __FUNCTION__, __LINE__, offr, size);

  if (same && (offr == MSB16))  //shift one byte
    {
      for (i = size - 1; i >= MSB16; i--)
        {
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), i - 1));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (left), i));
        }
    }
  else
    {
      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), LSB));
    }

  if (size > LSB + offr)
    {
      if (same)
        {
          pic16_emitpcode (POC_ADDWF, pic16_popGet (AOP (left), LSB + offr));
        }
      else
        {
          pic16_emitpcode (POC_ADDFW, pic16_popGet (AOP (left), LSB));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), LSB + offr));
        }
    }

  if (size > MSB16 + offr)
    {
      if (same)
        {
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (left), MSB16 + offr));
        }
      else
        {
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), MSB16));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), MSB16 + offr));
        }
    }

  if (size > MSB24 + offr)
    {
      if (same)
        {
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (left), MSB24 + offr));
        }
      else
        {
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), MSB24));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), MSB24 + offr));
        }
    }

  if (size > MSB32 + offr)
    {
      if (same)
        {
          pic16_emitpcode (POC_RLCF, pic16_popGet (AOP (left), MSB32 + offr));
        }
      else
        {
          pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), MSB32));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), MSB32 + offr));
        }
    }
  if (offr != LSB)
    pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), LSB));

}

/*-----------------------------------------------------------------*/
/* genlshFour - shift four byte by a known amount != 0             */
/*-----------------------------------------------------------------*/
static void
genlshFour (operand * result, operand * left, int shCount)
{
  int size;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  size = AOP_SIZE (result);

  /* if shifting more that 3 bytes */
  if (shCount >= 24)
    {
      shCount -= 24;
      if (shCount)
        /* lowest order of left goes to the highest
           order of the destination */
        shiftL1Left2Result (left, LSB, result, MSB32, shCount);
      else
        movLeft2Result (left, LSB, result, MSB32);

      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), LSB));
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), MSB16));
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), MSB24));

      return;
    }

  /* more than two bytes */
  else if (shCount >= 16)
    {
      /* lower order two bytes goes to higher order two bytes */
      shCount -= 16;
      /* if some more remaining */
      if (shCount)
        shiftL2Left2Result (left, LSB, result, MSB24, shCount);
      else
        {
          movLeft2Result (left, MSB16, result, MSB32);
          movLeft2Result (left, LSB, result, MSB24);
        }
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), LSB));
      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), MSB16));
      return;
    }

  /* if more than 1 byte */
  else if (shCount >= 8)
    {
      /* lower order three bytes goes to higher order  three bytes */
      shCount -= 8;
      if (size == 2)
        {
          if (shCount)
            shiftL1Left2Result (left, LSB, result, MSB16, shCount);
          else
            movLeft2Result (left, LSB, result, MSB16);
        }
      else                      /* size = 4 */
        {
          if (shCount == 0)
            {
              movLeft2Result (left, MSB24, result, MSB32);
              movLeft2Result (left, MSB16, result, MSB24);
              movLeft2Result (left, LSB, result, MSB16);
              pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), LSB));
            }
          else if (shCount == 1)
            shiftLLong (left, result, MSB16);
          else
            {
              shiftL2Left2Result (left, MSB16, result, MSB24, shCount);
              shiftL1Left2Result (left, LSB, result, MSB16, shCount);
              shiftRLeftOrResult (left, LSB, result, MSB24, 8 - shCount);
              pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), LSB));
            }
        }
    }

  /* 1 <= shCount <= 7 */
  else if (shCount <= 3)
    {
      shiftLLong (left, result, LSB);
      while (--shCount >= 1)
        shiftLLong (result, result, LSB);
    }
  /* 3 <= shCount <= 7, optimize */
  else
    {
      shiftL2Left2Result (left, MSB24, result, MSB24, shCount);
      shiftRLeftOrResult (left, MSB16, result, MSB24, 8 - shCount);
      shiftL2Left2Result (left, LSB, result, LSB, shCount);
    }
}

/*-----------------------------------------------------------------*/
/* genLeftShiftLiteral - left shifting by known count              */
/*-----------------------------------------------------------------*/
void
pic16_genLeftShiftLiteral (operand * left, operand * right, operand * result, iCode * ic)
{
  int shCount = abs ((int) ulFromVal (AOP (right)->aopu.aop_lit));
  int size;

  FENTRY;
  DEBUGpic16_emitcode ("; ***", "shCount:%d", shCount);
  pic16_freeAsmop (right, NULL, ic, TRUE);

  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  size = getSize (operandType (result));

#if VIEW_SIZE
  pic16_emitcode ("; shift left ", "result %d, left %d", size, AOP_SIZE (left));
#endif

  /* I suppose that the left size >= result size */
  if (shCount == 0)
    {
      while (size--)
        {
          movLeft2Result (left, size, result, size);
        }
    }

  else if (shCount >= (size * 8))
    while (size--)
      pic16_aopPut (AOP (result), zero, size);
  else
    {
      switch (size)
        {
        case 1:
          genlshOne (result, left, shCount);
          break;

        case 2:
        case 3:
          genlshTwo (result, left, shCount);
          break;

        case 4:
          genlshFour (result, left, shCount);
          break;
        }
    }
  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
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

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

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
      pic16_emitpcode (poc, pic16_popGet (AOP (reg), offset));
      offset += endian;
    }

}

/*-----------------------------------------------------------------*/
/* genrshOne - right shift a one byte quantity by known count      */
/*-----------------------------------------------------------------*/
static void
genrshOne (operand * result, operand * left, int shCount, int sign)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  shiftR1Left2Result (left, LSB, result, LSB, shCount, sign);
}

/*-----------------------------------------------------------------*/
/* genrshTwo - right shift two bytes by known amount != 0          */
/*-----------------------------------------------------------------*/
static void
genrshTwo (operand * result, operand * left, int shCount, int sign)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d shCount:%d", __FUNCTION__, __LINE__, shCount);
  /* if shCount >= 8 */
  if (shCount >= 8)
    {
      shCount -= 8;
      if (shCount)
        shiftR1Left2Result (left, MSB16, result, LSB, shCount, sign);
      else
        movLeft2Result (left, MSB16, result, LSB);

      pic16_addSign (result, 1, sign);
    }

  /*  1 <= shCount <= 7 */
  else
    shiftR2Left2Result (left, LSB, result, LSB, shCount, sign);
}

/*-----------------------------------------------------------------*/
/* shiftRLong - shift right one long from left to result           */
/* offl = LSB or MSB16                                             */
/*-----------------------------------------------------------------*/
static void
shiftRLong (operand * left, int offl, operand * result, int sign)
{
  int size = AOP_SIZE (result);
  int same = pic16_sameRegs (AOP (left), AOP (result));
  int i;
  DEBUGpic16_emitcode ("; ***", "%s  %d  offl:%d size:%d", __FUNCTION__, __LINE__, offl, size);

  if (same && (offl == MSB16))  //shift one byte right
    {
      for (i = MSB16; i < size; i++)
        {
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (left), i));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (left), i - 1));
        }
    }

  if (sign)
    pic16_emitpcode (POC_RLCFW, pic16_popGet (AOP (left), MSB32));
  else
    emitCLRC;

  if (same)
    {
      if (offl == LSB)
        pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (left), MSB32));
    }
  else
    {
      pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), MSB32));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), MSB32 - offl));
    }

  if (offl == MSB16)
    {
      /* add sign of "a" */
      pic16_addSign (result, MSB32, sign);
    }

  if (same)
    {
      pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (left), MSB24));
    }
  else
    {
      pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), MSB24));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), MSB24 - offl));
    }

  if (same)
    {
      pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (left), MSB16));
    }
  else
    {
      pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), MSB16));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), MSB16 - offl));
    }

  if (same)
    {
      pic16_emitpcode (POC_RRCF, pic16_popGet (AOP (left), LSB));
    }
  else
    {
      if (offl == LSB)
        {
          pic16_emitpcode (POC_RRCFW, pic16_popGet (AOP (left), LSB));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), LSB));
        }
    }
}

/*-----------------------------------------------------------------*/
/* genrshFour - shift four byte by a known amount != 0             */
/*-----------------------------------------------------------------*/
static void
genrshFour (operand * result, operand * left, int shCount, int sign)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* if shifting more that 3 bytes */
  if (shCount >= 24)
    {
      shCount -= 24;
      if (shCount)
        shiftR1Left2Result (left, MSB32, result, LSB, shCount, sign);
      else
        movLeft2Result (left, MSB32, result, LSB);

      pic16_addSign (result, MSB16, sign);
    }
  else if (shCount >= 16)
    {
      shCount -= 16;
      if (shCount)
        shiftR2Left2Result (left, MSB24, result, LSB, shCount, sign);
      else
        {
          movLeft2Result (left, MSB24, result, LSB);
          movLeft2Result (left, MSB32, result, MSB16);
        }
      pic16_addSign (result, MSB24, sign);
    }
  else if (shCount >= 8)
    {
      shCount -= 8;
      if (shCount == 1)
        shiftRLong (left, MSB16, result, sign);
      else if (shCount == 0)
        {
          movLeft2Result (left, MSB16, result, LSB);
          movLeft2Result (left, MSB24, result, MSB16);
          movLeft2Result (left, MSB32, result, MSB24);
          pic16_addSign (result, MSB32, sign);
        }
      else                      //shcount >= 2
        {
          shiftR2Left2Result (left, MSB16, result, LSB, shCount, 0);
          shiftLLeftOrResult (left, MSB32, result, MSB16, 8 - shCount);
          /* the last shift is signed */
          shiftR1Left2Result (left, MSB32, result, MSB24, shCount, sign);
          pic16_addSign (result, MSB32, sign);
        }
    }
  else                          /* 1 <= shCount <= 7 */
    {
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
/* genRightShiftLiteral - right shifting by known count            */
/*-----------------------------------------------------------------*/
static void
genRightShiftLiteral (operand * left, operand * right, operand * result, iCode * ic, int sign)
{
  int shCount = abs ((int) ulFromVal (AOP (right)->aopu.aop_lit));
  int lsize, res_size;

  pic16_freeAsmop (right, NULL, ic, TRUE);

  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  DEBUGpic16_emitcode ("; ***", "%s  %d shCount:%d result:%d left:%d", __FUNCTION__, __LINE__, shCount, AOP_SIZE (result),
                       AOP_SIZE (left));

#if VIEW_SIZE
  pic16_emitcode ("; shift right ", "result %d, left %d", AOP_SIZE (result), AOP_SIZE (left));
#endif

  lsize = pic16_getDataSize (left);
  res_size = pic16_getDataSize (result);
  /* test the LEFT size !!! */

  /* I suppose that the left size >= result size */
  if (shCount == 0)
    {
      assert (res_size <= lsize);
      while (res_size--)
        {
          pic16_mov2f (AOP (result), AOP (left), res_size);
        }                       // for
    }
  else if (shCount >= (lsize * 8))
    {
      if (sign)
        {
          /*
           * Do NOT use
           *    CLRF    result
           *    BTFSC   left, 7
           *    SETF    result
           * even for 8-bit operands; result might be an SFR.
           */
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0x00));
          pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (left), lsize - 1), 7));
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xff));
          while (res_size--)
            {
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), res_size));
            }
        }
      else                      // unsigned
        {
          while (res_size--)
            {
              pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), res_size));
            }
        }
    }
  else                          // 0 < shCount < 8*lsize
    {
      switch (res_size)
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
          break;
        }
    }

  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGenericShift - generates code for left or right shifting     */
/*-----------------------------------------------------------------*/
static void
genGenericShift (iCode * ic, int isShiftLeft)
{
  operand *left, *right, *result;
  int offset;
  int sign, signedCount;
  symbol *label_complete, *label_loop_pos, *label_loop_neg, *label_negative;
  PIC_OPCODE pos_shift, neg_shift;

  FENTRY;

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  pic16_aopOp (right, ic, FALSE);
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);

  sign = !SPEC_USIGN (operandType (left));
  signedCount = !SPEC_USIGN (operandType (right));

  /* if the shift count is known then do it
     as efficiently as possible */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      long lit = (long) ulFromVal (AOP (right)->aopu.aop_lit);
      if (signedCount && lit < 0)
        {
          lit = -lit;
          isShiftLeft = !isShiftLeft;
        }
      // we should modify right->aopu.aop_lit here!
      // Instead we use abs(shCount) in genXXXShiftLiteral()...
      // lit > 8*size is handled in pic16_genXXXShiftLiteral()
      if (isShiftLeft)
        pic16_genLeftShiftLiteral (left, right, result, ic);
      else
        genRightShiftLiteral (left, right, result, ic, sign);

      goto release;
    }                           // if (right is literal)

  /* shift count is unknown then we have to form a loop.
   * Note: we take only the lower order byte since shifting
   * more than 32 bits make no sense anyway, ( the
   * largest size of an object can be only 32 bits )
   * Note: we perform arithmetic shifts if the left operand is
   * signed and we do an (effective) right shift, i. e. we
   * shift in the sign bit from the left. */

  label_complete = newiTempLabel (NULL);
  label_loop_pos = newiTempLabel (NULL);
  label_loop_neg = NULL;
  label_negative = NULL;
  pos_shift = isShiftLeft ? POC_RLCF : POC_RRCF;
  neg_shift = isShiftLeft ? POC_RRCF : POC_RLCF;

  if (signedCount)
    {
      // additional labels needed
      label_loop_neg = newiTempLabel (NULL);
      label_negative = newiTempLabel (NULL);
    }                           // if

  /*
   * The code below overwrites the shift count for `val = (1 << val)'
   * when it assigns LEFT to RESULT (== RIGHT == shift count).
   * XXX: This problem should have been/is also addressed in ralloc.c,
   *      but the code there seems not to catch this case ...
   */
  if (pic16_sameRegs (AOP (right), AOP (result)))
    {
      // We abuse FSR0L as a temporary, pic16_popGetTempReg() is too costly.
      pic16_mov2w (AOP (right), 0);
      pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_fsr0l));
    }                           // if

  // copy source to result -- this will effectively truncate the left operand to the size of result!
  // (e.g. char c = 0x100 << -3 will become c = 0x00 >> 3 == 0x00 instad of 0x20)
  // This is fine, as it only occurs for left shifting with negative count which is not standardized!
  for (offset = 0; offset < min (AOP_SIZE (left), AOP_SIZE (result)); offset++)
    {
      pic16_mov2f (AOP (result), AOP (left), offset);
    }                           // for

  // if result is longer than left, fill with zeros (or sign)
  if (AOP_SIZE (left) < AOP_SIZE (result))
    {
      if (sign && AOP_SIZE (left) > 0)
        {
          // shift signed operand -- fill with sign
          pic16_emitpcode (POC_CLRF, pic16_popCopyReg (&pic16_pc_wreg));
          pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (result), AOP_SIZE (left) - 1), 7));
          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xFF));
          for (offset = AOP_SIZE (left); offset < AOP_SIZE (result); offset++)
            {
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
            }                   // for
        }
      else
        {
          // shift unsigned operand -- fill result with zeros
          for (offset = AOP_SIZE (left); offset < AOP_SIZE (result); offset++)
            {
              pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset));
            }                   // for
        }
    }                           // if (size mismatch)

  /* load/restore shift count */
  if (pic16_sameRegs (AOP (right), AOP (result)))
    {
      pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (&pic16_pc_fsr0l));
    }
  else
    {
      pic16_mov2w (AOP (right), 0);
    }                           // if

  pic16_emitpcode (POC_BZ, pic16_popGetLabel (label_complete->key));
  if (signedCount)
    pic16_emitpcode (POC_BN, pic16_popGetLabel (label_negative->key));

#if 0
  // perform a shift by one (shift count is positive)
  // cycles used for shifting {unsigned,signed} values on n bytes by [unsigned,signed] shift count c>0:
  // 2n+[2,3]+({1,3}+n+3)c-2+[0,2]=({4,6}+n)c+2n+[0,3]          ({5,7}c+[2,5] / {6,8}c+[4, 7] / {8,10}c+[ 8,11])
  pic16_emitpLabel (label_loop_pos->key);
  emitCLRC;
  if (sign && (pos_shift == POC_RRCF))
    {
      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (result), AOP_SIZE (result) - 1), 7));
      emitSETC;
    }                           // if
  genMultiAsm (pos_shift, result, AOP_SIZE (result), pos_shift == POC_RRCF);
  pic16_emitpcode (POC_DECFSZ, pic16_popCopyReg (&pic16_pc_wreg));
  pic16_emitpcode (POC_BRA, pic16_popGetLabel (label_loop_pos->key));
#else
  // perform a shift by one (shift count is positive)
  // cycles used for shifting {unsigned,signed} values on n bytes by [unsigned,signed] shift count c>0:
  // 2n+[2,3]+2+({0,2}+n+3)c-1+[0,2]=({3,5}+n)c+2n+[3,6]        ({4,6}c+[5,8] / {5,7}c+[7,10] / {7, 9}c+[11,14])
  // This variant is slower for 0<c<3, equally fast for c==3, and faster for 3<c.
  pic16_emitpcode (POC_NEGF, pic16_popCopyReg (&pic16_pc_wreg));
  emitCLRC;
  pic16_emitpLabel (label_loop_pos->key);
  if (sign && (pos_shift == POC_RRCF))
    {
      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (result), AOP_SIZE (result) - 1), 7));
      emitSETC;
    }                           // if
  genMultiAsm (pos_shift, result, AOP_SIZE (result), pos_shift == POC_RRCF);
  //pic16_emitpcode (POC_INCF, pic16_popCopyReg (&pic16_pc_wreg)); // gpsim does not like this...
  pic16_emitpcode (POC_ADDLW, pic16_popGetLit (0x01));
  pic16_emitpcode (POC_BNC, pic16_popGetLabel (label_loop_pos->key));
#endif

  if (signedCount)
    {
      pic16_emitpcode (POC_BRA, pic16_popGetLabel (label_complete->key));

      pic16_emitpLabel (label_negative->key);
      // perform a shift by -1 (shift count is negative)
      // 2n+4+1+({0,2}+n+3)*c-1=({3,5}+n)c+2n+4                   ({4,6}c+6 / {5,7}c+8 / {7,9}c+12)
      emitCLRC;
      pic16_emitpLabel (label_loop_neg->key);
      if (sign && (neg_shift == POC_RRCF))
        {
          pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (result), AOP_SIZE (result) - 1), 7));
          emitSETC;
        }                       // if
      genMultiAsm (neg_shift, result, AOP_SIZE (result), neg_shift == POC_RRCF);
      //pic16_emitpcode (POC_INCF, pic16_popCopyReg (&pic16_pc_wreg)); // gpsim does not like this...
      pic16_emitpcode (POC_ADDLW, pic16_popGetLit (0x01));
      pic16_emitpcode (POC_BNC, pic16_popGetLabel (label_loop_neg->key));
    }                           // if (signedCount)

  pic16_emitpLabel (label_complete->key);

release:
  pic16_freeAsmop (right, NULL, ic, TRUE);
  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

static void
genLeftShift (iCode * ic)
{
  genGenericShift (ic, 1);
}

static void
genRightShift (iCode * ic)
{
  genGenericShift (ic, 0);
}


/* load FSR0 with address of/from op according to pic16_isLitOp() or if lit is 1 */
void
pic16_loadFSR0 (operand * op, int lit)
{
  if ((IS_SYMOP (op) && OP_SYMBOL (op)->remat) || pic16_isLitOp (op))
    {
      if (AOP_TYPE (op) == AOP_LIT)
        {
          /* handle 12 bit integers correctly */
          unsigned int val = (unsigned int) ulFromVal (AOP (op)->aopu.aop_lit);
          if ((val & 0x0fff) != val)
            {
              fprintf (stderr, "WARNING: Accessing memory at 0x%x truncated to 0x%x.\n", val, (val & 0x0fff));
              val &= 0x0fff;
            }
          pic16_emitpcode (POC_LFSR, pic16_popGetLit2 (0, pic16_popGetLit12 (val)));
        }
      else
        {
          pic16_emitpcode (POC_LFSR, pic16_popGetLit2 (0, pic16_popGet (AOP (op), 0)));
        }
    }
  else
    {
      assert (!IS_SYMOP (op) || !OP_SYMBOL (op)->remat);
      // set up FSR0 with address of result
      pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (op), 0), pic16_popCopyReg (&pic16_pc_fsr0l)));
      pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (op), 1), pic16_popCopyReg (&pic16_pc_fsr0h)));
    }
}

/*----------------------------------------------------------------*/
/* pic16_derefPtr - move one byte from the location ptr points to */
/*                  to WREG (doWrite == 0) or one byte from WREG   */
/*                  to the location ptr points to (doWrite != 0)   */
/*----------------------------------------------------------------*/
static void
pic16_derefPtr (operand * ptr, int p_type, int doWrite, int *fsr0_setup)
{
  if (!IS_PTR (operandType (ptr)))
    {
      if (doWrite)
        pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (ptr), 0));
      else
        pic16_mov2w (AOP (ptr), 0);
      return;
    }

  //assert (IS_DECL(operandType(ptr)) && (p_type == DCL_TYPE(operandType(ptr))));
  /* We might determine pointer type right here: */
  p_type = DCL_TYPE (operandType (ptr));

  switch (p_type)
    {
    case POINTER:
    case FPOINTER:
    case IPOINTER:
    case PPOINTER:
      if (!fsr0_setup || !*fsr0_setup)
        {
          pic16_loadFSR0 (ptr, 0);
          if (fsr0_setup)
            *fsr0_setup = 1;
        }
      if (doWrite)
        pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_indf0));
      else
        pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (&pic16_pc_indf0));
      break;

    case GPOINTER:
      if (AOP (ptr)->aopu.aop_reg[2])
        {
          if (doWrite)
            pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (pic16_stack_postdec));
          // prepare call to __gptrget1, this is actually genGenPointerGet(result, WREG, ?ic?)
          mov2fp (pic16_popCopyReg (&pic16_pc_fsr0l), AOP (ptr), 0);
          mov2fp (pic16_popCopyReg (&pic16_pc_prodl), AOP (ptr), 1);
          pic16_mov2w (AOP (ptr), 2);
          pic16_callGenericPointerRW (doWrite, 1);
        }
      else
        {
          // data pointer (just 2 byte given)
          if (!fsr0_setup || !*fsr0_setup)
            {
              pic16_loadFSR0 (ptr, 0);
              if (fsr0_setup)
                *fsr0_setup = 1;
            }
          if (doWrite)
            pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_indf0));
          else
            pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (&pic16_pc_indf0));
        }
      break;

    case CPOINTER:
      /* XXX: Writing to CPOINTERs not (yet) implemented. */
      assert (!doWrite && "Cannot write into __code space!");
      if ((AOP_TYPE (ptr) == AOP_PCODE)
          && ((AOP (ptr)->aopu.pcop->type == PO_IMMEDIATE) || (AOP (ptr)->aopu.pcop->type == PO_DIR)))
        {
          pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (ptr), 0));
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptrl));
          pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (ptr), 1));
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptrh));
          pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (ptr), 2));
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptru));
        }
      else
        {
          mov2fp (pic16_popCopyReg (&pic16_pc_tblptrl), AOP (ptr), 0);
          mov2fp (pic16_popCopyReg (&pic16_pc_tblptrh), AOP (ptr), 1);
          mov2fp (pic16_popCopyReg (&pic16_pc_tblptru), AOP (ptr), 2);
        }                       // if

      pic16_emitpcodeNULLop (POC_TBLRD_POSTINC);
      pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (&pic16_pc_tablat));
      break;

    default:
      assert (0 && "invalid pointer type specified");
      break;
    }
}

/*-----------------------------------------------------------------*/
/* genUnpackBits - generates code for unpacking bits               */
/*-----------------------------------------------------------------*/
static void
genUnpackBits (operand * result, operand * left, char *rname, int ptype)
{
  int shCnt;
  sym_link *etype;
  unsigned blen = 0, bstr = 0;
  int same;
  pCodeOp *op;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  etype = getSpec (operandType (result));

  //    if(IS_BITFIELD(etype)) {
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);
  //    }

  DEBUGpic16_emitcode ("; ***", "%s  %d - reading %s bitfield int %s destination", __FUNCTION__, __LINE__,
                       SPEC_USIGN (OP_SYM_ETYPE (left)) ? "an unsigned" : "a signed",
                       SPEC_USIGN (OP_SYM_TYPE (result)) ? "an unsigned" : "a signed");

#if 1
  if ((blen == 1) && (bstr < 8) && (!IS_PTR (operandType (left)) || IS_DIRECT (left) || PIC_IS_DATA_PTR (operandType (left))))
    {
      /* it is a single bit, so use the appropriate bit instructions */
      DEBUGpic16_emitcode (";", "%s %d optimize bit read", __FUNCTION__, __LINE__);

      same = pic16_sameRegs (AOP (left), AOP (result));
      op = (same ? pic16_popCopyReg (&pic16_pc_wreg) : pic16_popGet (AOP (result), 0));
      pic16_emitpcode (POC_CLRF, op);

      if (!IS_PTR (operandType (left)) || IS_DIRECT (left))
        {
          /* workaround to reduce the extra lfsr instruction */
          pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (left), 0), bstr));
        }
      else
        {
          assert (PIC_IS_DATA_PTR (operandType (left)));
          pic16_loadFSR0 (left, 0);
          pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_indf0), bstr));
        }

      if (SPEC_USIGN (OP_SYM_ETYPE (left)))
        {
          /* unsigned bitfields result in either 0 or 1 */
          pic16_emitpcode (POC_INCF, op);
        }
      else
        {
          /* signed bitfields result in either 0 or -1 */
          pic16_emitpcode (POC_DECF, op);
        }
      if (same)
        {
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
        }

      pic16_addSign (result, 1, !SPEC_USIGN (OP_SYM_TYPE (result)));
      return;
    }

#endif

  if (!IS_PTR (operandType (left)) || IS_DIRECT (left))
    {
      // access symbol directly
      pic16_mov2w (AOP (left), 0);
    }
  else
    {
      pic16_derefPtr (left, ptype, 0, NULL);
    }

  /* if we have bitdisplacement then it fits   */
  /* into this byte completely or if length is */
  /* less than a byte                          */
  if ((shCnt = SPEC_BSTR (etype)) || (SPEC_BLEN (etype) <= 8))
    {

      /* shift right acc */
      AccRsh (shCnt, 0);

      pic16_emitpcode (POC_ANDLW, pic16_popGetLit ((((unsigned char) - 1) >> (8 - SPEC_BLEN (etype))) & SRMask[shCnt]));

      /* VR -- normally I would use the following, but since we use the hack,
       * to avoid the masking from AccRsh, why not mask it right now? */

      /*
         pic16_emitpcode(POC_ANDLW, pic16_popGetLit(((unsigned char) -1)>>(8 - SPEC_BLEN(etype))));
       */

      /* extend signed bitfields to 8 bits */
      if (!SPEC_USIGN (OP_SYM_ETYPE (left)) && (bstr + blen < 8))
        {
          assert (blen + bstr > 0);
          pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_wreg), bstr + blen - 1));
          pic16_emitpcode (POC_IORLW, pic16_popGetLit (0xFF << (bstr + blen)));
        }

      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));

      pic16_addSign (result, 1, !SPEC_USIGN (OP_SYM_TYPE (result)));
      return;
    }

  fprintf (stderr, "SDCC pic16 port error: the port currently does not support *reading*\n");
  fprintf (stderr, "bitfields of size >=8. Instead of generating wrong code, bailing out...\n");
  exit (EXIT_FAILURE);
}


static void
genDataPointerGet (operand * left, operand * result, iCode * ic)
{
  int size, offset = 0, leoffset = 0;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  pic16_aopOp (result, ic, TRUE);

  FENTRY;

  size = AOP_SIZE (result);
//      fprintf(stderr, "%s:%d size= %d\n", __FILE__, __LINE__, size);


#if 1
  if (!strcmp (pic16_aopGet (AOP (result), 0, TRUE, FALSE), pic16_aopGet (AOP (left), 0, TRUE, FALSE)))
    {
      DEBUGpic16_emitcode ("; ***", "left and result names are same, skipping moving");
      goto release;
    }
#endif

  if (AOP (left)->aopu.pcop->type == PO_DIR)
    leoffset = PCOR (AOP (left)->aopu.pcop)->instance;

  DEBUGpic16_pic16_AopType (__LINE__, left, NULL, result);

  while (size--)
    {
      DEBUGpic16_emitcode ("; ***", "%s loop offset=%d leoffset=%d", __FUNCTION__, offset, leoffset);

//              pic16_DumpOp("(result)",result);
      if (pic16_isLitAop (AOP (result)))
        {
          pic16_mov2w (AOP (left), offset);     // patch 8
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
        }
      else
        {
          pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (left), offset),        //patch 8
                           pic16_popGet (AOP (result), offset)));
        }

      offset++;
      leoffset++;
    }

release:
  pic16_freeAsmop (result, NULL, ic, TRUE);
}



/*-----------------------------------------------------------------*/
/* genNearPointerGet - pic16_emitcode for near pointer fetch             */
/*-----------------------------------------------------------------*/
static void
genNearPointerGet (operand * left, operand * result, iCode * ic)
{
//  asmop *aop = NULL;
  //regs *preg = NULL ;
  sym_link *rtype, *retype;
  sym_link *ltype, *letype;

  FENTRY;

  rtype = operandType (result);
  retype = getSpec (rtype);
  ltype = operandType (left);
  letype = getSpec (ltype);

  pic16_aopOp (left, ic, FALSE);

//    pic16_DumpOp("(left)",left);
//    pic16_DumpOp("(result)",result);

  /* if left is rematerialisable and
   * result is not bit variable type and
   * the left is pointer to data space i.e
   * lower 128 bytes of space */

  if (AOP_TYPE (left) == AOP_PCODE && !IS_BITFIELD (retype) && DCL_TYPE (ltype) == POINTER)
    {

      genDataPointerGet (left, result, ic);
      pic16_freeAsmop (left, NULL, ic, TRUE);
      return;
    }

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  pic16_aopOp (result, ic, TRUE);

  DEBUGpic16_pic16_AopType (__LINE__, left, NULL, result);

#if 1
  if (IS_BITFIELD (retype) && (SPEC_BLEN (retype) == 1))
    {
      iCode *nextic;
      pCodeOp *jop;
      int bitstrt;

      /* if this is bitfield of size 1, see if we are checking the value
       * of a single bit in an if-statement,
       * if yes, then don't generate usual code, but execute the
       * genIfx directly -- VR */

      nextic = ic->next;

      /* CHECK: if next iCode is IFX
       * and current result operand is nextic's conditional operand
       * and current result operand live ranges ends at nextic's key number
       */
      if ((nextic->op == IFX) && (result == IC_COND (nextic)) && (OP_LIVETO (result) == nextic->seq) && (OP_SYMBOL (left)->remat)       // below fails for "if (p->bitfield)"
         )
        {
          /* everything is ok then */
          /* find a way to optimize the genIfx iCode */

          bitstrt = SPEC_BSTR (retype) % 8;

          jop = pic16_popCopyGPR2Bit (pic16_popGet (AOP (left), 0), bitstrt);

          genIfxpCOpJump (nextic, jop);

          pic16_freeAsmop (left, NULL, ic, TRUE);
          pic16_freeAsmop (result, NULL, ic, TRUE);
          return;
        }
    }
#endif

  /* if bitfield then unpack the bits */
  if (IS_BITFIELD (letype))
    genUnpackBits (result, left, NULL, POINTER);
  else
    {
      /* we have can just get the values */
      int size = AOP_SIZE (result);
      int offset = 0;

      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

      pic16_loadFSR0 (left, 0);

      while (size--)
        {
          if (size)
            {
              pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (&pic16_pc_postinc0),
                               pic16_popGet (AOP (result), offset++)));
            }
          else
            {
              pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (&pic16_pc_indf0),
                               pic16_popGet (AOP (result), offset++)));
            }
        }
    }

#if 0
  /* now some housekeeping stuff */
  if (aop)
    {
      /* we had to allocate for this iCode */
      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      pic16_freeAsmop (NULL, aop, ic, TRUE);
    }
  else
    {
      /* we did not allocate which means left
       * already in a pointer register, then
       * if size > 0 && this could be used again
       * we have to point it back to where it
       * belongs */
      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      if (AOP_SIZE (result) > 1 && !OP_SYMBOL (left)->remat && (OP_SYMBOL (left)->liveTo > ic->seq || ic->depth))
        {
//        int size = AOP_SIZE(result) - 1;
//        while (size--)
//          pic16_emitcode("dec","%s",rname);
        }
    }
#endif

  /* done */
  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGenPointerGet - gget value from generic pointer space        */
/*-----------------------------------------------------------------*/
static void
genGenPointerGet (operand * left, operand * result, iCode * ic)
{
  int size;
  sym_link *letype = getSpec (operandType (left));

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);
  size = AOP_SIZE (result);

  DEBUGpic16_pic16_AopType (__LINE__, left, NULL, result);

  /* if bit then unpack */
  if (IS_BITFIELD (letype))
    {
      genUnpackBits (result, left, "BAD", GPOINTER);
      goto release;
    }

  /* set up WREG:PRODL:FSR0L with address from left */
  mov2fp (pic16_popCopyReg (&pic16_pc_fsr0l), AOP (left), 0);
  mov2fp (pic16_popCopyReg (&pic16_pc_prodl), AOP (left), 1);
  pic16_mov2w (AOP (left), 2);
  pic16_callGenericPointerRW (0, size);

  assignResultValue (result, size, 1);

release:
  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genConstPointerGet - get value from const generic pointer space */
/*-----------------------------------------------------------------*/
static void
genConstPointerGet (operand * left, operand * result, iCode * ic)
{
  //sym_link *retype = getSpec(operandType(result));
  // symbol *albl = newiTempLabel(NULL);        // patch 15
  // symbol *blbl = newiTempLabel(NULL);        //
  // PIC_OPCODE poc;                            // patch 15
  int size;
  int offset = 0;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  pic16_aopOp (left, ic, FALSE);
  pic16_aopOp (result, ic, TRUE);
  size = AOP_SIZE (result);

  /* if bit then unpack */
  if (IS_BITFIELD (getSpec (operandType (left))))
    {
      genUnpackBits (result, left, "BAD", GPOINTER);
      goto release;
    }                           // if

  DEBUGpic16_pic16_AopType (__LINE__, left, NULL, result);

  DEBUGpic16_emitcode ("; ", " %d getting const pointer", __LINE__);

  // set up table pointer
  if ((AOP_TYPE (left) == AOP_PCODE)
      && ((AOP (left)->aopu.pcop->type == PO_IMMEDIATE) || (AOP (left)->aopu.pcop->type == PO_DIR)))
    {
      pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (left), 0));
      pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptrl));
      pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (left), 1));
      pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptrh));
      pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (left), 2));
      pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptru));
    }
  else
    {
      mov2fp (pic16_popCopyReg (&pic16_pc_tblptrl), AOP (left), 0);
      mov2fp (pic16_popCopyReg (&pic16_pc_tblptrh), AOP (left), 1);
      mov2fp (pic16_popCopyReg (&pic16_pc_tblptru), AOP (left), 2);
    }

  while (size--)
    {
      pic16_emitpcodeNULLop (POC_TBLRD_POSTINC);
      pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (&pic16_pc_tablat), pic16_popGet (AOP (result), offset)));
      offset++;
    }

release:
  pic16_freeAsmop (left, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
}


/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void
genPointerGet (iCode * ic)
{
  operand *left, *result;
  sym_link *type, *etype;
  int p_type;

  FENTRY;

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  /* depending on the type of pointer we need to
     move it to the correct pointer register */
  type = operandType (left);
  etype = getSpec (type);

#if 0
  if (IS_PTR_CONST (type))
#else
  if (IS_CODEPTR (type))
#endif
    DEBUGpic16_emitcode ("; ***", "%d - const pointer", __LINE__);

  /* if left is of type of pointer then it is simple */
  if (IS_PTR (type) && !IS_FUNC (type->next))
    p_type = DCL_TYPE (type);
  else
    {
      /* we have to go by the storage class */
      p_type = PTR_TYPE (SPEC_OCLS (etype));

      DEBUGpic16_emitcode ("; ***", "%d - resolve pointer by storage class", __LINE__);

      if (SPEC_OCLS (etype)->codesp)
        {
          DEBUGpic16_emitcode ("; ***", "%d - cpointer", __LINE__);
          //p_type = CPOINTER ;
        }
      else if (SPEC_OCLS (etype)->fmap && !SPEC_OCLS (etype)->paged)
        {
          DEBUGpic16_emitcode ("; ***", "%d - fpointer", __LINE__);
          /*p_type = FPOINTER ; */
        }
      else if (SPEC_OCLS (etype)->fmap && SPEC_OCLS (etype)->paged)
        {
          DEBUGpic16_emitcode ("; ***", "%d - ppointer", __LINE__);
          /* p_type = PPOINTER; */
        }
      else if (SPEC_OCLS (etype) == idata)
        {
          DEBUGpic16_emitcode ("; ***", "%d - ipointer", __LINE__);
          /* p_type = IPOINTER; */
        }
      else
        {
          DEBUGpic16_emitcode ("; ***", "%d - pointer", __LINE__);
          /* p_type = POINTER ; */
        }
    }

  /* now that we have the pointer type we assign
     the pointer values */
  switch (p_type)
    {
    case POINTER:
    case FPOINTER:
    case IPOINTER:
    case PPOINTER:
      genNearPointerGet (left, result, ic);
      break;

    case CPOINTER:
      genConstPointerGet (left, result, ic);
      //pic16_emitcodePointerGet (left,result,ic);
      break;

    case GPOINTER:
#if 0
      if (IS_PTR_CONST (type))
        genConstPointerGet (left, result, ic);
      else
#endif
        genGenPointerGet (left, result, ic);
      break;

    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "genPointerGet: illegal pointer type");

    }
}

/*-----------------------------------------------------------------*/
/* genPackBits - generates code for packed bit storage             */
/*-----------------------------------------------------------------*/
static void
genPackBits (sym_link * etype, operand * result, operand * right, char *rname, int p_type)
{
  unsigned shCnt = 0;
  int offset = 0;
  int rLen = 0;
  unsigned blen, bstr;
  int shifted_and_masked = 0;
  unsigned long lit = (unsigned long) - 1;
  sym_link *retype;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  retype = getSpec (operandType (right));

  if (AOP_TYPE (right) == AOP_LIT)
    {
      lit = ulFromVal (AOP (right)->aopu.aop_lit);

      if ((blen == 1) && (bstr < 8))
        {
          /* it is a single bit, so use the appropriate bit instructions */

          DEBUGpic16_emitcode (";", "%s %d optimize bit assignment", __FUNCTION__, __LINE__);

          if (!IS_PTR (operandType (result)) || IS_DIRECT (result))
            {
              /* workaround to reduce the extra lfsr instruction */
              if (lit)
                {
                  pic16_emitpcode (POC_BSF, pic16_popCopyGPR2Bit (pic16_popGet (AOP (result), 0), bstr));
                }
              else
                {
                  pic16_emitpcode (POC_BCF, pic16_popCopyGPR2Bit (pic16_popGet (AOP (result), 0), bstr));
                }
            }
          else
            {
              if (PIC_IS_DATA_PTR (operandType (result)))
                {
                  pic16_loadFSR0 (result, 0);
                  pic16_emitpcode (lit ? POC_BSF : POC_BCF, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_indf0), bstr));
                }
              else
                {
                  /* get old value */
                  pic16_derefPtr (result, p_type, 0, NULL);
                  pic16_emitpcode (lit ? POC_BSF : POC_BCF, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_wreg), bstr));
                  /* write back new value */
                  pic16_derefPtr (result, p_type, 1, NULL);
                }
            }

          return;
        }
      /* IORLW below is more efficient */
      //pic16_emitpcode(POC_MOVLW, pic16_popGetLit((lit & ((1UL << blen) - 1)) << bstr));
      lit = (lit & ((1UL << blen) - 1)) << bstr;
      shifted_and_masked = 1;
      offset++;
    }
  else if (IS_DIRECT (result) && !IS_PTR (operandType (result))
           && IS_BITFIELD (retype) && (AOP_TYPE (right) == AOP_REG || AOP_TYPE (right) == AOP_DIR) && (blen == 1))
    {
      int rbstr;

      rbstr = SPEC_BSTR (retype);

      if (IS_BITFIELD (etype))
        {
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (result), 0));
          pic16_emitpcode (POC_BCF, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_wreg), bstr));
        }
      else
        {
          pic16_emitpcode (POC_CLRF, pic16_popCopyReg (&pic16_pc_wreg));
        }

      pic16_emitpcode (POC_BTFSC, pic16_popCopyGPR2Bit (pic16_popGet (AOP (right), 0), rbstr));

      if (IS_BITFIELD (etype))
        {
          pic16_emitpcode (POC_BSF, pic16_popCopyGPR2Bit (pic16_popCopyReg (&pic16_pc_wreg), bstr));
        }
      else
        {
          pic16_emitpcode (POC_INCF, pic16_popCopyReg (&pic16_pc_wreg));
        }

      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));

      return;
    }
  else
    {
      /* move right to W */
      pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset++));
    }

  /* if the bit length is less than or   */
  /* it exactly fits a byte then         */
  if ((shCnt = SPEC_BSTR (etype)) || SPEC_BLEN (etype) <= 8)
    {
      int fsr0_setup = 0;

      if (blen != 8 || (bstr % 8) != 0)
        {
          // we need to combine the value with the old value
          if (!shifted_and_masked)
            {
              pic16_emitpcode (POC_ANDLW, pic16_popGetLit ((1U << blen) - 1));

              DEBUGpic16_emitcode (";", "shCnt = %d SPEC_BSTR(etype) = %d:%d", shCnt, SPEC_BSTR (etype), SPEC_BLEN (etype));

              /* shift left acc, do NOT mask the result again */
              AccLsh (shCnt, 0);

              /* using PRODH as a temporary register here */
              pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_prodh));
            }

          if ((IS_SYMOP (result) && !IS_PTR (operandType (result))) || IS_DIRECT (result))
            {
              /* access symbol directly */
              pic16_mov2w (AOP (result), 0);
            }
          else
            {
              /* get old value */
              pic16_derefPtr (result, p_type, 0, &fsr0_setup);
            }
#if 1
          pic16_emitpcode (POC_ANDLW, pic16_popGetLit ((unsigned char) ((unsigned char) (0xff << (blen + bstr)) |
                           (unsigned char) (0xff >> (8 - bstr)))));
          if (!shifted_and_masked)
            {
              pic16_emitpcode (POC_IORFW, pic16_popCopyReg (&pic16_pc_prodh));
            }
          else
            {
              /* We have the shifted and masked (literal) right value in `lit' */
              if (lit != 0)
                pic16_emitpcode (POC_IORLW, pic16_popGetLit (lit));
            }
        }
      else                      // if (blen == 8 && (bstr % 8) == 0)
        {
          if (shifted_and_masked)
            {
              // move right (literal) to WREG (only case where right is not yet in WREG)
              pic16_mov2w (AOP (right), (bstr / 8));
            }
        }

      /* write new value back */
      if ((IS_SYMOP (result) && !IS_PTR (operandType (result))) || IS_DIRECT (result))
        {
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
        }
      else
        {
          pic16_derefPtr (result, p_type, 1, &fsr0_setup);
        }
#endif

      return;
    }


#if 0
  fprintf (stderr, "SDCC pic16 port error: the port currently does not support\n");
  fprintf (stderr, "bitfields of size >=8. Instead of generating wrong code, bailing out...\n");
  exit (EXIT_FAILURE);
#endif


  pic16_loadFSR0 (result, 0);   // load FSR0 with address of result
  rLen = SPEC_BLEN (etype) - 8;

  /* now generate for lengths greater than one byte */
  while (1)
    {
      rLen -= 8;
      if (rLen <= 0)
        {
          mov2fp (pic16_popCopyReg (&pic16_pc_prodh), AOP (right), offset);
          break;
        }

      switch (p_type)
        {
        case POINTER:
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_postinc0));
          break;

          /*
             case FPOINTER:
             MOVA(l);
             pic16_emitcode("movx","@dptr,a");
             break;

             case GPOINTER:
             MOVA(l);
             DEBUGpic16_emitcode(";lcall","__gptrput");
             break;
           */
        default:
          assert (0);
        }


      pic16_mov2w (AOP (right), offset++);
    }

  /* last last was not complete */
  if (rLen)
    {
      /* save the byte & read byte */
      switch (p_type)
        {
        case POINTER:
          //                pic16_emitpcode(POC_MOVWF, pic16_popCopyReg(&pic16_pc_prodl));
          pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (&pic16_pc_indf0));
          break;

          /*
             case FPOINTER:
             pic16_emitcode ("mov","b,a");
             pic16_emitcode("movx","a,@dptr");
             break;

             case GPOINTER:
             pic16_emitcode ("push","b");
             pic16_emitcode ("push","acc");
             pic16_emitcode ("lcall","__gptrget");
             pic16_emitcode ("pop","b");
             break;
           */
        default:
          assert (0);
        }
      DEBUGpic16_emitcode (";", "rLen = %i", rLen);
      pic16_emitpcode (POC_ANDLW, pic16_popGetLit ((unsigned char) - 1 << -rLen));
      pic16_emitpcode (POC_IORFW, pic16_popCopyReg (&pic16_pc_prodh));
      //        pic16_emitcode ("anl","a,#0x%02x",((unsigned char)-1 << -rLen) );
      //        pic16_emitcode ("orl","a,b");
    }

  //    if (p_type == GPOINTER)
  //        pic16_emitcode("pop","b");

  switch (p_type)
    {

    case POINTER:
      pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_indf0));
      //        pic16_emitcode("mov","@%s,a",rname);
      break;
      /*
         case FPOINTER:
         pic16_emitcode("movx","@dptr,a");
         break;

         case GPOINTER:
         DEBUGpic16_emitcode(";lcall","__gptrput");
         break;
       */
    default:
      assert (0);
    }

  //    pic16_freeAsmop(right, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genDataPointerSet - remat pointer to data space                 */
/*-----------------------------------------------------------------*/
static void
genDataPointerSet (operand * right, operand * result, iCode * ic)
{
  int size, offset = 0, resoffset = 0;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  pic16_aopOp (right, ic, FALSE);

  size = AOP_SIZE (right);

//      fprintf(stderr, "%s:%d size= %d\n", __FILE__, __LINE__, size);

#if 0
  if (AOP_TYPE (result) == AOP_PCODE)
    {
      fprintf (stderr, "genDataPointerSet   %s, %d\n",
               AOP (result)->aopu.pcop->name,
               (AOP (result)->aopu.pcop->type == PO_DIR) ?
               PCOR (AOP (result)->aopu.pcop)->instance : PCOI (AOP (result)->aopu.pcop)->offset);
    }
#endif

  if (AOP (result)->aopu.pcop->type == PO_DIR)
    resoffset = PCOR (AOP (result)->aopu.pcop)->instance;

  while (size--)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          unsigned int lit = pic16aopLiteral (AOP (IC_RIGHT (ic))->aopu.aop_lit, offset);
          pic16_movLit2f (pic16_popGet (AOP (result), offset), lit);
        }
      else
        {
          pic16_mov2w (AOP (right), offset);
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));     // patch 8
        }
      offset++;
      resoffset++;
    }

  pic16_freeAsmop (right, NULL, ic, TRUE);
}



/*-----------------------------------------------------------------*/
/* genNearPointerSet - pic16_emitcode for near pointer put         */
/*-----------------------------------------------------------------*/
static void
genNearPointerSet (operand * right, operand * result, iCode * ic)
{
  asmop *aop = NULL;
  sym_link *retype;
  sym_link *ptype = operandType (result);
  sym_link *resetype;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  retype = getSpec (operandType (right));
  resetype = getSpec (operandType (result));

  pic16_aopOp (result, ic, FALSE);

  /* if the result is rematerializable &
   * in data space & not a bit variable */

  /* and result is not a bit variable */
  if (AOP_TYPE (result) == AOP_PCODE && DCL_TYPE (ptype) == POINTER && !IS_BITFIELD (retype) && !IS_BITFIELD (resetype))
    {

      genDataPointerSet (right, result, ic);
      pic16_freeAsmop (result, NULL, ic, TRUE);
      return;
    }

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  pic16_aopOp (right, ic, FALSE);
  DEBUGpic16_pic16_AopType (__LINE__, NULL, right, result);

  /* if bitfield then unpack the bits */
  if (IS_BITFIELD (resetype))
    {
      genPackBits (resetype, result, right, NULL, POINTER);
    }
  else
    {
      /* we have can just get the values */
      int size = AOP_SIZE (right);
      int offset = 0;

      pic16_loadFSR0 (result, 0);

      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      while (size--)
        {
          if (pic16_isLitOp (right))
            {
              pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (right), offset));
              if (size)
                {
                  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_postinc0));
                }
              else
                {
                  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_indf0));
                }
            }
          else                  // no literal
            {
              if (size)
                {
                  pic16_emitpcode (POC_MOVFF,
                                   pic16_popGet2p (pic16_popGet (AOP (right), offset), pic16_popCopyReg (&pic16_pc_postinc0)));
                }
              else
                {
                  pic16_emitpcode (POC_MOVFF,
                                   pic16_popGet2p (pic16_popGet (AOP (right), offset), pic16_popCopyReg (&pic16_pc_indf0)));
                }
            }

          offset++;
        }
    }

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* now some housekeeping stuff */
  if (aop)
    {
      /* we had to allocate for this iCode */
      pic16_freeAsmop (NULL, aop, ic, TRUE);
    }
  else
    {
      /* we did not allocate which means left
       * already in a pointer register, then
       * if size > 0 && this could be used again
       * we have to point it back to where it
       * belongs */
      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
      if (AOP_SIZE (right) > 1 && !OP_SYMBOL (result)->remat && (OP_SYMBOL (result)->liveTo > ic->seq || ic->depth))
        {

          int size = AOP_SIZE (right) - 1;

          while (size--)
            pic16_emitcode ("decf", "fsr0,f");
          //pic16_emitcode("dec","%s",rname);
        }
    }

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
  /* done */
//release:
  pic16_freeAsmop (right, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genGenPointerSet - set value from generic pointer space         */
/*-----------------------------------------------------------------*/
static void
genGenPointerSet (operand * right, operand * result, iCode * ic)
{
  int size;
  sym_link *retype = getSpec (operandType (result));

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  pic16_aopOp (result, ic, FALSE);
  pic16_aopOp (right, ic, FALSE);
  size = AOP_SIZE (right);

  DEBUGpic16_pic16_AopType (__LINE__, NULL, right, result);


  /* if bit then unpack */
  if (IS_BITFIELD (retype))
    {
//      pic16_emitpcode(POC_LFSR,pic16_popGetLit2(0,pic16_popGetLit(lit)));
      genPackBits (retype, result, right, "dptr", GPOINTER);
      goto release;
    }

  size = AOP_SIZE (right);

  DEBUGpic16_emitcode ("; ***", "%s  %d size=%d", __FUNCTION__, __LINE__, size);


  /* load value to write in TBLPTRH:TBLPTRL:PRODH:[stack] */

  /* value of right+0 is placed on stack, which will be retrieved
   * by the support function thus restoring the stack. The important
   * thing is that there is no need to manually restore stack pointer
   * here */
  pushaop (AOP (right), 0);
//    mov2fp(pic16_popCopyReg(&pic16_pc_postdec1), AOP(right), 0);
  if (size > 1)
    mov2fp (pic16_popCopyReg (&pic16_pc_prodh), AOP (right), 1);
  if (size > 2)
    mov2fp (pic16_popCopyReg (&pic16_pc_tblptrl), AOP (right), 2);
  if (size > 3)
    mov2fp (pic16_popCopyReg (&pic16_pc_tblptrh), AOP (right), 3);

  /* load address to write to in WREG:FSR0H:FSR0L */
  pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (result), 0), pic16_popCopyReg (&pic16_pc_fsr0l)));
  pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (result), 1), pic16_popCopyReg (&pic16_pc_prodl)));
  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (result), 2));

  pic16_callGenericPointerRW (1, size);

release:
  pic16_freeAsmop (right, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
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

      /*      if (SPEC_OCLS(etype)->codesp ) { */
      /*          p_type = CPOINTER ;  */
      /*      } */
      /*      else */
      /*          if (SPEC_OCLS(etype)->fmap && !SPEC_OCLS(etype)->paged) */
      /*              p_type = FPOINTER ; */
      /*          else */
      /*              if (SPEC_OCLS(etype)->fmap && SPEC_OCLS(etype)->paged) */
      /*                  p_type = PPOINTER ; */
      /*              else */
      /*                  if (SPEC_OCLS(etype) == idata ) */
      /*                      p_type = IPOINTER ; */
      /*                  else */
      /*                      p_type = POINTER ; */
    }

  /* now that we have the pointer type we assign
     the pointer values */
  switch (p_type)
    {
    case POINTER:
    case FPOINTER:
    case IPOINTER:
    case PPOINTER:
      genNearPointerSet (right, result, ic);
      break;

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

  pic16_aopOp (cond, ic, FALSE);

  /* get the value into acc */
  if (AOP_TYPE (cond) != AOP_CRY)
    pic16_toBoolean (cond);
  else
    isbit = 1;
  /* the result is now in the accumulator */
  pic16_freeAsmop (cond, NULL, ic, TRUE);

  /* if there was something to be popped then do it */
  if (popIc)
    genIpop (popIc);

  /* if the condition is  a bit variable */
  if (isbit && IS_ITEMP (cond) && SPIL_LOC (cond))
    {
      genIfxJump (ic, "c");
      DEBUGpic16_emitcode ("; isbit  SPIL_LOC", "%s", SPIL_LOC (cond)->rname);
    }
  else
    {
      if (isbit && !IS_ITEMP (cond))
        genIfxJump (ic, OP_SYMBOL (cond)->rname);
      else
        genIfxJump (ic, "a");
    }
  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void
genAddrOf (iCode * ic)
{
  operand *result, *left;
  int size;
  symbol *sym;                  // = OP_SYMBOL(IC_LEFT(ic));
  pCodeOp *pcop0, *pcop1, *pcop2;

  FENTRY;

  pic16_aopOp ((result = IC_RESULT (ic)), ic, TRUE);

  sym = OP_SYMBOL (IC_LEFT (ic));

  if (sym->onStack)
    {
      /* get address of symbol on stack */
      DEBUGpic16_emitcode (";    ", "%s symbol %s on stack", __FUNCTION__, sym->name);
#if 0
      fprintf (stderr, "%s:%d symbol %s on stack offset %i\n", __FILE__, __LINE__,
               OP_SYMBOL (IC_LEFT (ic))->name, OP_SYMBOL (IC_LEFT (ic))->stack);
#endif

      // operands on stack are accessible via "FSR2 + index" with index
      // starting at 2 for arguments and growing from 0 downwards for
      // local variables (index == 0 is not assigned so we add one here)
      {
        int soffs = OP_SYMBOL (IC_LEFT (ic))->stack;

        if (soffs <= 0)
          {
            assert (soffs < 0);
            soffs++;
          }                     // if

        DEBUGpic16_emitcode ("*!*", "accessing stack symbol at offset=%d", soffs);
        pic16_emitpcode (POC_MOVLW, pic16_popGetLit (soffs & 0x00FF));
        pic16_emitpcode (POC_ADDFW, pic16_popCopyReg (pic16_framepnt_lo));
        pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
        pic16_emitpcode (POC_MOVLW, pic16_popGetLit ((soffs >> 8) & 0x00FF));
        pic16_emitpcode (POC_ADDFWC, pic16_popCopyReg (pic16_framepnt_hi));
        pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 1));
      }

      goto release;
    }

//      if(pic16_debug_verbose) {
//              fprintf(stderr, "%s:%d %s symbol %s , codespace=%d\n",
//                      __FILE__, __LINE__, __FUNCTION__, sym->name, IN_CODESPACE( SPEC_OCLS(sym->etype)));
//      }

  pic16_aopOp ((left = IC_LEFT (ic)), ic, FALSE);
  size = AOP_SIZE (IC_RESULT (ic));

  pcop0 = PCOP (pic16_newpCodeOpImmd (sym->rname, 0, 0, IN_CODESPACE (SPEC_OCLS (sym->etype))));
  pcop1 = PCOP (pic16_newpCodeOpImmd (sym->rname, 1, 0, IN_CODESPACE (SPEC_OCLS (sym->etype))));
  pcop2 = PCOP (pic16_newpCodeOpImmd (sym->rname, 2, 0, IN_CODESPACE (SPEC_OCLS (sym->etype))));

  if (size == 3)
    {
      pic16_emitpcode (POC_MOVLW, pcop0);
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
      pic16_emitpcode (POC_MOVLW, pcop1);
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 1));
      pic16_emitpcode (POC_MOVLW, pcop2);
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 2));
    }
  else if (size == 2)
    {
      pic16_emitpcode (POC_MOVLW, pcop0);
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
      pic16_emitpcode (POC_MOVLW, pcop1);
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 1));
    }
  else
    {
      pic16_emitpcode (POC_MOVLW, pcop0);
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
    }

  pic16_freeAsmop (left, NULL, ic, FALSE);
release:
  pic16_freeAsmop (result, NULL, ic, TRUE);
}


/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void
genAssign (iCode * ic)
{
  operand *result, *right;
  sym_link *restype, *rtype;
  int size, offset, know_W;
  unsigned long lit = 0L;

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);

  FENTRY;

  /* if they are the same */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  /* reversed order operands are aopOp'ed so that result operand
   * is effective in case right is a stack symbol. This maneauver
   * allows to use the _G.resDirect flag later */
  pic16_aopOp (result, ic, TRUE);
  pic16_aopOp (right, ic, FALSE);

  DEBUGpic16_pic16_AopType (__LINE__, NULL, right, result);

  /* if they are the same registers */
  if (pic16_sameRegs (AOP (right), AOP (result)))
    goto release;

  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {
      /* if the right size is a literal then
         we know what the value is */
      if (AOP_TYPE (right) == AOP_LIT)
        {

          pic16_emitpcode ((((int) operandLitValue (right)) ? POC_BSF : POC_BCF), pic16_popGet (AOP (result), 0));

          if (((int) operandLitValue (right)))
            pic16_emitcode ("bsf", "(%s >> 3),(%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
          else
            pic16_emitcode ("bcf", "(%s >> 3),(%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);

          goto release;
        }

      /* the right is also a bit variable */
      if (AOP_TYPE (right) == AOP_CRY)
        {
          pic16_emitpcode (POC_BCF, pic16_popGet (AOP (result), 0));
          pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (right), 0));
          pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), 0));

          goto release;
        }

      /* we need to or */
      pic16_emitpcode (POC_BCF, pic16_popGet (AOP (result), 0));
      pic16_toBoolean (right);
      emitSKPZ;
      pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), 0));
      //pic16_aopPut(AOP(result),"a",0);
      goto release;
    }

  /* bit variables done */
  /* general case */
  size = AOP_SIZE (result);
  offset = 0;

  /* bit variables done */
  /* general case */
  size = AOP_SIZE (result);
  restype = operandType (result);
  rtype = operandType (right);
  offset = 0;

  if (AOP_TYPE (right) == AOP_LIT)
    {
      if (!(IS_FLOAT (operandType (right)) || IS_FIXED (operandType (right))))
        {
          lit = ulFromVal (AOP (right)->aopu.aop_lit);

          /* patch tag for literals that are cast to pointers */
          if (IS_CODEPTR (restype))
            {
              //fprintf (stderr, "%s:%u: INFO: `(__code*)literal'\n", ic->filename, ic->lineno);
              lit = (lit & 0x00ffff) | (GPTR_TAG_CODE << 16);
            }
          else
            {
              if (IS_GENPTR (restype))
                {
                  if (IS_CODEPTR (rtype))
                    {
                      //fprintf (stderr, "%s:%u: INFO: `(generic*)(literal __code*)'\n", ic->filename, ic->lineno);
                      lit = (lit & 0x00ffff) | (GPTR_TAG_CODE << 16);
                    }
                  else if (PIC_IS_DATA_PTR (rtype))
                    {
                      //fprintf (stderr, "%s:%u: INFO: `(generic*)(literal __data*)'\n", ic->filename, ic->lineno);
                      lit = (lit & 0x00ffff) | (GPTR_TAG_DATA << 16);
                    }
                  else if (!IS_PTR (rtype) || IS_GENPTR (rtype))
                    {
                      //fprintf (stderr, "%s:%u: INFO: `(generic*)literal' -- accepting specified tag %02x\n", ic->filename, ic->lineno, (unsigned char)(lit >> 16));
                    }
                  else if (IS_PTR (rtype))
                    {
                      fprintf (stderr, "%s:%u: WARNING: `(generic*)literal' -- assuming __data space\n", ic->filename,
                               ic->lineno);
                      lit = (lit & 0x00ffff) | (GPTR_TAG_DATA << 16);
                    }
                }
            }
        }
      else
        {
          union
          {
            unsigned long lit_int;
            float lit_float;
          } info;


          if (IS_FIXED16X16 (operandType (right)))
            {
              lit = (unsigned long) fixed16x16FromDouble (floatFromVal (AOP (right)->aopu.aop_lit));
            }
          else
            {
              /* take care if literal is a float */
              info.lit_float = (float)floatFromVal (AOP (right)->aopu.aop_lit);
              lit = info.lit_int;
            }
        }
    }

//  fprintf(stderr, "%s:%d: assigning value 0x%04lx (%d:%d)\n", __FUNCTION__, __LINE__, lit,
//                      sizeof(unsigned long int), sizeof(float));


  if (AOP_TYPE (right) == AOP_REG)
    {
      DEBUGpic16_emitcode (";   ", "%s:%d assign from register\n", __FUNCTION__, __LINE__);
      while (size--)
        {
          pic16_emitpcode (POC_MOVFF, pic16_popGet2 (AOP (right), AOP (result), offset++));
        }                       // while
      goto release;
    }

  /* when do we have to read the program memory?
   * - if right itself is a symbol in code space
   *   (we don't care what it points to if it's a pointer)
   * - AND right is not a function (we would want its address)
   */
  if (AOP_TYPE (right) != AOP_LIT
      && IN_CODESPACE (SPEC_OCLS (OP_SYM_ETYPE (right))) && !IS_FUNC (OP_SYM_TYPE (right)) && !IS_ITEMP (right))
    {

      DEBUGpic16_emitcode (";   ", "%s:%d symbol in code space, take special care\n", __FUNCTION__, __LINE__);
      //fprintf(stderr, "%s:%d symbol %s = [ %s ] is in code space\n", __FILE__, __LINE__, OP_SYMBOL(result)->name, OP_SYMBOL(right)->name);

      // set up table pointer
      if (pic16_isLitOp (right))
        {
//      fprintf(stderr, "%s:%d inside block 1\n", __FILE__, __LINE__);
          pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (right), 0));
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptrl));
          pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (right), 1));
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptrh));
          pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (right), 2));
          pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_tblptru));
        }
      else
        {
//      fprintf(stderr, "%s:%d inside block 2\n", __FILE__, __LINE__);
          pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (right), 0), pic16_popCopyReg (&pic16_pc_tblptrl)));
          pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (right), 1), pic16_popCopyReg (&pic16_pc_tblptrh)));
          pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popGet (AOP (right), 2), pic16_popCopyReg (&pic16_pc_tblptru)));
        }

      /* must fetch 3 bytes for pointers (was OP_SYM_ETYPE before) */
      size = min ((int)getSize (OP_SYM_TYPE (right)), AOP_SIZE (result));
      while (size--)
        {
          pic16_emitpcodeNULLop (POC_TBLRD_POSTINC);
          pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (&pic16_pc_tablat),
                           pic16_popGet (AOP (result), offset)));
          offset++;
        }

      /* FIXME: for pointers we need to extend differently (according
       * to pointer type DATA/CODE/EEPROM/... :*/
      size = getSize (OP_SYM_TYPE (right));
      if (AOP_SIZE (result) > size)
        {
          size = AOP_SIZE (result) - size;
          while (size--)
            {
              pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset));
              offset++;
            }
        }
      goto release;
    }

#if 0
  /* VR - What is this?! */
  if (AOP_TYPE (right) == AOP_DIR && (AOP_TYPE (result) == AOP_REG) && size == 1)
    {
      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

      if (aopIdx (AOP (result), 0) == 4)
        {
          /* this is a workaround to save value of right into wreg too,
           * value of wreg is going to be used later */
          DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
          pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
          goto release;
        }
      else
//      assert(0);
        DEBUGpic16_emitcode ("; WARNING", "%s  %d ignoring register storage", __FUNCTION__, __LINE__);
    }
#endif

  size = AOP_SIZE (right);
  if (size > AOP_SIZE (result))
    size = AOP_SIZE (result);
  know_W = -1;
  while (size--)
    {
      DEBUGpic16_emitcode ("; ***", "%s  %d size %d", __FUNCTION__, __LINE__, size);
      if (AOP_TYPE (right) == AOP_LIT)
        {
          if (lit & 0xff)
            {
              if (know_W != (lit & 0xff))
                pic16_emitpcode (POC_MOVLW, pic16_popGetLit (lit & 0xff));
              know_W = lit & 0xff;

              if (AOP_TYPE (result) != AOP_ACC)
                {
                  pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                }
            }
          else
            pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset));

          lit >>= 8;

        }
      else if (AOP_TYPE (right) == AOP_CRY)
        {
          pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset));
          if (offset == 0)
            {
              //debugf("%s: BTFSS offset == 0\n", __FUNCTION__);
              pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (right), 0));
              pic16_emitpcode (POC_INCF, pic16_popGet (AOP (result), 0));
            }
        }
      else if ((AOP_TYPE (right) == AOP_PCODE) && (AOP (right)->aopu.pcop->type == PO_IMMEDIATE))
        {
          pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (right), offset));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
        }
      else
        {
          DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

          if (!_G.resDirect)    /* use this aopForSym feature */
            {
              if (AOP_TYPE (result) == AOP_ACC)
                {
                  pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
                }
              else if (AOP_TYPE (right) == AOP_ACC)
                {
                  pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
                }
              else
                {
                  pic16_emitpcode (POC_MOVFF, pic16_popGet2 (AOP (right), AOP (result), offset));
                }
            }
        }

      offset++;
    }
  pic16_addSign (result, AOP_SIZE (right), !IS_UNSIGNED (operandType (right)));

release:
  pic16_freeAsmop (result, NULL, ic, TRUE);
  pic16_freeAsmop (right, NULL, ic, FALSE);
}

/*-----------------------------------------------------------------*/
/* genJumpTab - generates code for jump table                       */
/*-----------------------------------------------------------------*/
static void
genJumpTab (iCode * ic)
{
  symbol *jtab;
  char *l;
  pCodeOp *jt_label;

  FENTRY;

  pic16_aopOp (IC_JTCOND (ic), ic, FALSE);
  /* get the condition into accumulator */
  l = pic16_aopGet (AOP (IC_JTCOND (ic)), 0, FALSE, FALSE);
  MOVA (l);
  /* multiply by three */
  pic16_emitcode ("add", "a,acc");
  pic16_emitcode ("add", "a,%s", pic16_aopGet (AOP (IC_JTCOND (ic)), 0, FALSE, FALSE));

  jtab = newiTempLabel (NULL);
  pic16_emitcode ("mov", "dptr,#%05d_DS_", labelKey2num (jtab->key));
  pic16_emitcode ("jmp", "@a+dptr");
  pic16_emitcode ("", "%05d_DS_:", labelKey2num (jtab->key));

#if 0
  pic16_emitpcode (POC_MOVLW, pic16_popGetLabel (jtab->key));
  pic16_emitpcode (POC_ADDFW, pic16_popGet (AOP (IC_JTCOND (ic)), 0));
  emitSKPNC;
  pic16_emitpcode (POC_INCF, pic16_popCopyReg (&pic16_pc_pclath));
  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_pcl));
  pic16_emitpLabel (jtab->key);

#else

  jt_label = pic16_popGetLabel (jtab->key);
  //fprintf (stderr, "Creating jump table...\n");

  // calculate offset into jump table (idx * sizeof(GOTO))
  // This solution does not use temporary registers and the code is shorter.
  pic16_emitpcode (POC_CLRF  , pic16_popCopyReg (&pic16_pc_pclath));
  pic16_emitpcode (POC_CLRF  , pic16_popCopyReg (&pic16_pc_pclatu));
  pic16_emitpcode (POC_RLCFW , pic16_popGet (AOP (IC_JTCOND (ic)), 0));
  pic16_emitpcode (POC_RLCF  , pic16_popCopyReg (&pic16_pc_pclath));
  pic16_emitpcode (POC_RLCFW , pic16_popCopyReg (&pic16_pc_wreg));
  pic16_emitpcode (POC_RLCF  , pic16_popCopyReg (&pic16_pc_pclath));
  pic16_emitpcode (POC_ANDLW , pic16_popGetLit (0xFC));
  pic16_emitpcode (POC_ADDLW , pic16_popGetImmd (jt_label->name, 0, 0));
  pic16_emitpcode (POC_MOVWF , pic16_popCopyReg (pic16_stack_postdec));
  pic16_emitpcode (POC_MOVLW , pic16_popGetImmd (jt_label->name, 1, 0));
  pic16_emitpcode (POC_ADDWFC, pic16_popCopyReg (&pic16_pc_pclath));
  pic16_emitpcode (POC_MOVLW , pic16_popGetImmd (jt_label->name, 2, 0));
  pic16_emitpcode (POC_ADDWFC, pic16_popCopyReg (&pic16_pc_pclatu));

  // jump into the table
  pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (pic16_stack_preinc));
  pic16_emitpcode (POC_MOVWF, pic16_popCopyReg (&pic16_pc_pcl));

  pic16_emitpLabelFORCE (jtab->key);
#endif

  pic16_freeAsmop (IC_JTCOND (ic), NULL, ic, TRUE);
//          pic16_emitpinfo(INF_LOCALREGS, pic16_newpCodeOpLocalRegs(LR_ENTRY_BEGIN));

  pic16_emitpinfo (INF_OPTIMIZATION, pic16_newpCodeOpOpt (OPT_JUMPTABLE_BEGIN, ""));
  /* now generate the jump labels */
  for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
    {
//        pic16_emitcode("ljmp","%05d_DS_",labelKey2num (jtab->key));
      pic16_emitpcode (POC_GOTO, pic16_popGetLabel (jtab->key));

    }
  pic16_emitpinfo (INF_OPTIMIZATION, pic16_newpCodeOpOpt (OPT_JUMPTABLE_END, ""));

}

/*-----------------------------------------------------------------*/
/* genMixedOperation - gen code for operators between mixed types  */
/*-----------------------------------------------------------------*/
/*
  TSD - Written for the PIC port - but this unfortunately is buggy.
  This routine is good in that it is able to efficiently promote
  types to different (larger) sizes. Unfortunately, the temporary
  variables that are optimized out by this routine are sometimes
  used in other places. So until I know how to really parse the
  iCode tree, I'm going to not be using this routine :(.
*/
static int
genMixedOperation (iCode * ic)
{
#if 0
  operand *result = IC_RESULT (ic);
  sym_link *ctype = operandType (IC_LEFT (ic));
  operand *right = IC_RIGHT (ic);
  int ret = 0;
  int big, small;
  int offset;

  iCode *nextic;
  operand *nextright = NULL, *nextleft = NULL, *nextresult = NULL;

  pic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  nextic = ic->next;
  if (!nextic)
    return 0;

  nextright = IC_RIGHT (nextic);
  nextleft = IC_LEFT (nextic);
  nextresult = IC_RESULT (nextic);

  pic16_aopOp (right, ic, FALSE);
  pic16_aopOp (result, ic, FALSE);
  pic16_aopOp (nextright, nextic, FALSE);
  pic16_aopOp (nextleft, nextic, FALSE);
  pic16_aopOp (nextresult, nextic, FALSE);

  if (pic16_sameRegs (AOP (IC_RESULT (ic)), AOP (IC_RIGHT (nextic))))
    {

      operand *t = right;
      right = nextright;
      nextright = t;

      pic16_emitcode (";remove right +", "");

    }
  else if (pic16_sameRegs (AOP (IC_RESULT (ic)), AOP (IC_LEFT (nextic))))
    {
      /*
         operand *t = right;
         right = nextleft;
         nextleft = t;
       */
      pic16_emitcode (";remove left +", "");
    }
  else
    return 0;

  big = AOP_SIZE (nextleft);
  small = AOP_SIZE (nextright);

  switch (nextic->op)
    {

    case '+':
      pic16_emitcode (";optimize a +", "");
      /* if unsigned or not an integral type */
      if (AOP_TYPE (IC_LEFT (nextic)) == AOP_CRY)
        {
          pic16_emitcode (";add a bit to something", "");
        }
      else
        {

          pic16_emitcode ("movf", "%s,w", AOP (nextright)->aopu.aop_dir);

          if (!pic16_sameRegs (AOP (IC_LEFT (nextic)), AOP (IC_RESULT (nextic))))
            {
              pic16_emitcode ("addwf", "%s,w", AOP (nextleft)->aopu.aop_dir);
              pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (IC_RESULT (nextic)), 0, FALSE, FALSE));
            }
          else
            pic16_emitcode ("addwf", "%s,f", AOP (nextleft)->aopu.aop_dir);

          offset = 0;
          while (--big)
            {

              offset++;

              if (--small)
                {
                  if (!pic16_sameRegs (AOP (IC_LEFT (nextic)), AOP (IC_RESULT (nextic))))
                    {
                      pic16_emitcode ("movf", "%s,w", pic16_aopGet (AOP (IC_LEFT (nextic)), offset, FALSE, FALSE));
                      pic16_emitcode ("movwf", "%s,f", pic16_aopGet (AOP (IC_RESULT (nextic)), offset, FALSE, FALSE));
                    }

                  pic16_emitcode ("movf", "%s,w", pic16_aopGet (AOP (IC_LEFT (nextic)), offset, FALSE, FALSE));
                  emitSKPNC;
                  pic16_emitcode ("btfsc", "(%s >> 3), (%s & 7)",
                                  AOP (IC_RIGHT (nextic))->aopu.aop_dir, AOP (IC_RIGHT (nextic))->aopu.aop_dir);
                  pic16_emitcode (" incf", "%s,w", pic16_aopGet (AOP (IC_LEFT (nextic)), offset, FALSE, FALSE));
                  pic16_emitcode ("movwf", "%s", pic16_aopGet (AOP (IC_RESULT (nextic)), offset, FALSE, FALSE));

                }
              else
                {
                  pic16_emitcode ("rlf", "known_zero,w");

                  /*
                     if right is signed
                     btfsc  right,7
                     addlw ff
                   */
                  if (!pic16_sameRegs (AOP (IC_LEFT (nextic)), AOP (IC_RESULT (nextic))))
                    {
                      pic16_emitcode ("addwf", "%s,w", pic16_aopGet (AOP (IC_LEFT (nextic)), offset, FALSE, FALSE));
                      pic16_emitcode ("movwf", "%s,f", pic16_aopGet (AOP (IC_RESULT (nextic)), offset, FALSE, FALSE));
                    }
                  else
                    {
                      pic16_emitcode ("addwf", "%s,f", pic16_aopGet (AOP (IC_RESULT (nextic)), offset, FALSE, FALSE));
                    }
                }
            }
          ret = 1;
        }
    }
  ret = 1;

release:
  pic16_freeAsmop (right, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);
  pic16_freeAsmop (nextright, NULL, ic, TRUE);
  pic16_freeAsmop (nextleft, NULL, ic, TRUE);
  if (ret)
    nextic->generated = 1;

  return ret;
#else
  return 0;
#endif
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
  sym_link *restype = operandType (IC_RESULT (ic));
  operand *right = IC_RIGHT (ic);
  int size, offset;


  FENTRY;

  /* if they are equivalent then do nothing */
//      if (operandsEqu(IC_RESULT(ic),IC_RIGHT(ic)))
//              return ;

  pic16_aopOp (result, ic, FALSE);
  pic16_aopOp (right, ic, FALSE);

  DEBUGpic16_pic16_AopType (__LINE__, NULL, right, result);


  /* if the result is a bit */
  if (AOP_TYPE (result) == AOP_CRY)
    {

      /* if the right size is a literal then
       * we know what the value is */
      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

      if (AOP_TYPE (right) == AOP_LIT)
        {
          pic16_emitpcode ((((int) operandLitValue (right)) ? POC_BSF : POC_BCF), pic16_popGet (AOP (result), 0));

          if (((int) operandLitValue (right)))
            pic16_emitcode ("bsf", "(%s >> 3), (%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
          else
            pic16_emitcode ("bcf", "(%s >> 3), (%s & 7)", AOP (result)->aopu.aop_dir, AOP (result)->aopu.aop_dir);
          goto release;
        }

      /* the right is also a bit variable */
      if (AOP_TYPE (right) == AOP_CRY)
        {
          emitCLRC;
          pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (right), 0));

          pic16_emitcode ("clrc", "");
          pic16_emitcode ("btfsc", "(%s >> 3), (%s & 7)", AOP (right)->aopu.aop_dir, AOP (right)->aopu.aop_dir);
          pic16_aopPut (AOP (result), "c", 0);
          goto release;
        }

      /* we need to or */
      if (AOP_TYPE (right) == AOP_REG)
        {
          pic16_emitpcode (POC_BCF, pic16_popGet (AOP (result), 0));
          pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit (pic16_aopGet (AOP (right), 0, FALSE, FALSE), 0, 0, PO_GPR_REGISTER));
          pic16_emitpcode (POC_BSF, pic16_popGet (AOP (result), 0));
        }
      pic16_toBoolean (right);
      pic16_aopPut (AOP (result), "a", 0);
      goto release;
    }

  if ((AOP_TYPE (right) == AOP_CRY) && (AOP_TYPE (result) == AOP_REG))
    {
      int offset = 1;

      size = AOP_SIZE (result);

      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

      pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), 0));
      pic16_emitpcode (POC_BTFSC, pic16_popGet (AOP (right), 0));
      pic16_emitpcode (POC_INCF, pic16_popGet (AOP (result), 0));

      while (size--)
        pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset++));

      goto release;
    }

  if (IS_BOOL (operandType (result)))
    {
      pic16_toBoolean (right);
      emitSKPNZ;
      pic16_emitpcode (POC_MOVLW, pic16_popGetLit (1));
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));
      goto release;
    }

  if (IS_BITFIELD (getSpec (restype)) && IS_BITFIELD (getSpec (rtype)))
    {
      DEBUGpic16_emitcode ("***", "%d casting a bit to another bit", __LINE__);
    }

  /* port from pic14 to cope with generic pointers */
  if (PIC_IS_TAGGED (restype))
    {
      operand *result = IC_RESULT (ic);
      //operand *left = IC_LEFT(ic);
      operand *right = IC_RIGHT (ic);
      int tag = 0xff;

      /* copy common part */
      int max, size = AOP_SIZE (result);
      if (size > AOP_SIZE (right))
        size = AOP_SIZE (right);
      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

      max = size;
      while (size--)
        {
          pic16_mov2w (AOP (right), size);
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), size));
        }                       // while

      /* upcast into generic pointer type? */
      if (IS_GENPTR (restype) && !PIC_IS_TAGGED (rtype) && (AOP_SIZE (result) > max))
        {
          /* determine appropriate tag for right */
          if (PIC_IS_DATA_PTR (rtype))
            tag = GPTR_TAG_DATA;
          else if (IS_CODEPTR (rtype))
            tag = GPTR_TAG_CODE;
          else if (PIC_IS_DATA_PTR (ctype))
            {
              //fprintf (stderr, "%s:%u: WARNING: casting `(generic*)(__data*)(non-pointer)'\n", ic->filename, ic->lineno);
              tag = GPTR_TAG_DATA;
            }
          else if (IS_CODEPTR (ctype))
            {
              //fprintf (stderr, "%s:%u: WARNING: casting `(generic*)(__code*)(non-pointer)'\n", ic->filename, ic->lineno);
              tag = GPTR_TAG_CODE;
            }
          else if (IS_PTR (rtype))
            {
              PERFORM_ONCE (weirdcast,
                            fprintf (stderr, "%s:%u: WARNING: casting `(generic*)(unknown*)' -- assuming __data space\n",
                                     ic->filename, ic->lineno););
              tag = GPTR_TAG_DATA;
            }
          else
            {
              PERFORM_ONCE (weirdcast,
                            fprintf (stderr, "%s:%u: WARNING: casting `(generic*)(non-pointer)' -- assuming __data space\n",
                                     ic->filename, ic->lineno););
              tag = GPTR_TAG_DATA;
            }

          assert (AOP_SIZE (result) == 3);
          /* zero-extend address... */
          for (size = max; size < AOP_SIZE (result) - 1; size++)
            pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), size));
          /* ...and add tag */
          pic16_movLit2f (pic16_popGet (AOP (result), AOP_SIZE (result) - 1), tag);
        }
      else if (IS_CODEPTR (restype) && AOP_SIZE (result) > max)
        {
          //fprintf (stderr, "%s:%u: INFO: code pointer\n", ic->filename, ic->lineno);
          for (size = max; size < AOP_SIZE (result) - 1; size++)
            pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), size));
          /* add __code tag */
          pic16_movLit2f (pic16_popGet (AOP (result), AOP_SIZE (result) - 1), GPTR_TAG_CODE);
        }
      else if (AOP_SIZE (result) > max)
        {
          /* extend non-pointers */
          //fprintf (stderr, "%s:%u: zero-extending value cast to pointer\n", ic->filename, ic->lineno);
          pic16_addSign (result, max, 0);
        }                       // if
      goto release;
    }

  /* if they are the same size : or less */
  if (AOP_SIZE (result) <= AOP_SIZE (right))
    {

      /* if they are in the same place */
      if (pic16_sameRegs (AOP (right), AOP (result)))
        goto release;

      DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);
#if 0
      if (IS_PTR_CONST (rtype))
#else
      if (IS_CODEPTR (rtype))
#endif
        DEBUGpic16_emitcode ("; ***", "%d - right is const pointer", __LINE__);

#if 0
      if (IS_PTR_CONST (operandType (IC_RESULT (ic))))
#else
      if (IS_CODEPTR (operandType (IC_RESULT (ic))))
#endif
        DEBUGpic16_emitcode ("; ***", "%d - result is const pointer", __LINE__);

      if ((AOP_TYPE (right) == AOP_PCODE) && AOP (right)->aopu.pcop->type == PO_IMMEDIATE)
        {
          pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (right), 0));
          pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 0));

          if (AOP_SIZE (result) < 2)
            {
              fprintf (stderr, "%d -- casting a ptr to a char\n", __LINE__);
            }
          else
            {
              pic16_emitpcode (POC_MOVLW, pic16_popGet (AOP (right), 1));
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 1));
            }
        }
      else
        {
          /* if they in different places then copy */
          size = AOP_SIZE (result);
          offset = 0;
          while (size--)
            {
              pic16_emitpcode (POC_MOVFW, pic16_popGet (AOP (right), offset));
              pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset));
              offset++;
            }
        }
      goto release;
    }

  /* if the result is of type pointer */
  if (IS_PTR (ctype))
    {
      int p_type;
      sym_link *type = operandType (right);
      sym_link *etype = getSpec (type);

      DEBUGpic16_emitcode ("; ***", "%s  %d - pointer cast", __FUNCTION__, __LINE__);

      /* pointer to generic pointer */
      if (IS_GENPTR (ctype))
        {

          if (IS_PTR (type))
            p_type = DCL_TYPE (type);
          else
            {
              /* we have to go by the storage class */
              p_type = PTR_TYPE (SPEC_OCLS (etype));

              /*              if (SPEC_OCLS(etype)->codesp )  */
              /*                  p_type = CPOINTER ;  */
              /*              else */
              /*                  if (SPEC_OCLS(etype)->fmap && !SPEC_OCLS(etype)->paged) */
              /*                      p_type = FPOINTER ; */
              /*                  else */
              /*                      if (SPEC_OCLS(etype)->fmap && SPEC_OCLS(etype)->paged) */
              /*                          p_type = PPOINTER; */
              /*                      else */
              /*                          if (SPEC_OCLS(etype) == idata ) */
              /*                              p_type = IPOINTER ; */
              /*                          else */
              /*                              p_type = POINTER ; */
            }

          /* the first two bytes are known */
          DEBUGpic16_emitcode ("; ***", "%s  %d - pointer cast2", __FUNCTION__, __LINE__);
          size = GPTRSIZE - 1;
          offset = 0;
          while (size--)
            {
              if (offset < AOP_SIZE (right))
                {
                  DEBUGpic16_emitcode ("; ***", "%s  %d - pointer cast3 ptype = 0x%x", __FUNCTION__, __LINE__, p_type);
                  pic16_mov2f (AOP (result), AOP (right), offset);
                  /*
                     if ((AOP_TYPE(right) == AOP_PCODE) &&
                     AOP(right)->aopu.pcop->type == PO_IMMEDIATE) {
                     pic16_emitpcode(POC_MOVLW, pic16_popGet(AOP(right),offset));
                     pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),offset));
                     } else {

                     pic16_aopPut(AOP(result),
                     pic16_aopGet(AOP(right),offset,FALSE,FALSE),
                     offset);
                     }
                   */
                }
              else
                pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset));
              offset++;
            }
          /* the last byte depending on type */
          switch (p_type)
            {
            case POINTER:
            case FPOINTER:
            case IPOINTER:
            case PPOINTER:
              pic16_movLit2f (pic16_popGet (AOP (result), GPTRSIZE - 1), GPTR_TAG_DATA);
              break;

            case CPOINTER:
              pic16_emitpcode (POC_MOVFF, pic16_popGet2 (AOP (right), AOP (result), GPTRSIZE - 1));
              break;

            case GPOINTER:
              if (GPTRSIZE > AOP_SIZE (right))
                {
                  // assume __data pointer... THIS MIGHT BE WRONG!
                  pic16_movLit2f (pic16_popGet (AOP (result), GPTRSIZE - 1), GPTR_TAG_DATA);
                }
              else
                {
                  pic16_emitpcode (POC_MOVFF, pic16_popGet2 (AOP (right), AOP (result), GPTRSIZE - 1));
                }
              break;

            default:
              /* this should never happen */
              werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "got unknown pointer type");
              exit (1);
            }
          //pic16_aopPut(AOP(result),l, GPTRSIZE - 1);
          goto release;
        }


      assert (0);
      /* just copy the pointers */
      size = AOP_SIZE (result);
      offset = 0;
      while (size--)
        {
          pic16_aopPut (AOP (result), pic16_aopGet (AOP (right), offset, FALSE, FALSE), offset);
          offset++;
        }
      goto release;
    }



  /* so we now know that the size of destination is greater
     than the size of the source.
     Now, if the next iCode is an operator then we might be
     able to optimize the operation without performing a cast.
   */
  if (genMixedOperation (ic))
    goto release;

  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

  /* we move to result for the size of source */
  size = AOP_SIZE (right);
  offset = 0;

  while (size--)
    {
      if (!_G.resDirect)
        pic16_mov2f (AOP (result), AOP (right), offset);
      offset++;
    }

  /* now depending on the sign of the destination */
  size = AOP_SIZE (result) - AOP_SIZE (right);
  /* if unsigned or not an integral type */
  if (SPEC_USIGN (getSpec (rtype)) || !IS_SPEC (rtype))
    {
      while (size--)
        pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset++));
    }
  else
    {
      /* we need to extend the sign :( */

      if (size == 1)
        {
          /* Save one instruction of casting char to int */
          pic16_emitpcode (POC_CLRF, pic16_popGet (AOP (result), offset));
          pic16_emitpcode (POC_BTFSC,
                           pic16_newpCodeOpBit (pic16_aopGet (AOP (right), offset - 1, FALSE, FALSE), 7, 0, PO_GPR_REGISTER));
          pic16_emitpcode (POC_SETF, pic16_popGet (AOP (result), offset));
        }
      else
        {
          pic16_emitpcode (POC_CLRF, pic16_popCopyReg (&pic16_pc_wreg));

          if (offset)
            pic16_emitpcode (POC_BTFSC,
                             pic16_newpCodeOpBit (pic16_aopGet (AOP (right), offset - 1, FALSE, FALSE), 7, 0, PO_GPR_REGISTER));
          else
            pic16_emitpcode (POC_BTFSC,
                             pic16_newpCodeOpBit (pic16_aopGet (AOP (right), offset, FALSE, FALSE), 7, 0, PO_GPR_REGISTER));

          pic16_emitpcode (POC_MOVLW, pic16_popGetLit (0xff));

          while (size--)
            pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), offset++));
        }
    }

release:
  pic16_freeAsmop (right, NULL, ic, TRUE);
  pic16_freeAsmop (result, NULL, ic, TRUE);

}

/*-----------------------------------------------------------------*/
/* genDjnz - generate decrement & jump if not zero instrucion      */
/*-----------------------------------------------------------------*/
static int
genDjnz (iCode * ic, iCode * ifx)
{
  DEBUGpic16_emitcode ("; ***", "%s  %d", __FUNCTION__, __LINE__);

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
  pic16_aopOp (IC_RESULT (ic), ic, FALSE);

  pic16_emitpcode (POC_DECFSZ, pic16_popGet (AOP (IC_RESULT (ic)), 0));
  pic16_emitpcode (POC_GOTO, pic16_popGetLabel (IC_TRUE (ifx)->key));

  pic16_freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
  ifx->generated = 1;
  return 1;
}

/*-----------------------------------------------------------------*/
/* genReceive - generate code for a receive iCode                  */
/*-----------------------------------------------------------------*/
static void
genReceive (iCode * ic)
{

  FENTRY;

#if 0
  fprintf (stderr, "%s:%d %s for symbol %s\tonStack: %d\n", __FILE__, __LINE__, __FUNCTION__,
           OP_SYMBOL (IC_RESULT (ic))->rname, OP_SYMBOL (IC_RESULT (ic))->onStack);
#endif
//  pic16_DumpOp(__FUNCTION__, IC_RESULT(ic));

  if (isOperandInFarSpace (IC_RESULT (ic)) && (OP_SYMBOL (IC_RESULT (ic))->isspilt || IS_TRUE_SYMOP (IC_RESULT (ic))))
    {

      int size = getSize (operandType (IC_RESULT (ic)));
      int offset = pic16_fReturnSizePic - size;

      assert (0);
      while (size--)
        {
          pic16_emitcode ("push", "%s", (strcmp (fReturn[pic16_fReturnSizePic - offset - 1], "a") ?
                                         fReturn[pic16_fReturnSizePic - offset - 1] : "acc"));
          offset++;
        }

      DEBUGpic16_emitcode ("; ***", "1 %s  %d", __FUNCTION__, __LINE__);

      pic16_aopOp (IC_RESULT (ic), ic, FALSE);
      size = AOP_SIZE (IC_RESULT (ic));
      offset = 0;
      while (size--)
        {
          pic16_emitcode ("pop", "acc");
          pic16_aopPut (AOP (IC_RESULT (ic)), "a", offset++);
        }
    }
  else
    {
      DEBUGpic16_emitcode ("; ***", "2 %s  %d argreg = %d", __FUNCTION__, __LINE__,
                           SPEC_ARGREG (OP_SYM_ETYPE (IC_RESULT (ic))));
      _G.accInUse++;
      pic16_aopOp (IC_RESULT (ic), ic, FALSE);
      _G.accInUse--;

      /* set pseudo stack pointer to where it should be - dw */
      GpseudoStkPtr = ic->parmBytes;

      /* setting GpseudoStkPtr has side effects here: */
      /* FIXME: What's the correct size of the return(ed) value?
       *        For now, assuming '4' as before... */
      assignResultValue (IC_RESULT (ic), 4, 0);
    }

  pic16_freeAsmop (IC_RESULT (ic), NULL, ic, TRUE);
}

/*-----------------------------------------------------------------*/
/* genDummyRead - generate code for dummy read of volatiles        */
/*-----------------------------------------------------------------*/
static void
genDummyRead (iCode * ic)
{
  operand *op;

  op = IC_RIGHT (ic);
  if (op && IS_SYMOP (op))
    {
      if (IN_CODESPACE (SPEC_OCLS (OP_SYM_ETYPE (op))))
        {
          fprintf (stderr, "%s: volatile symbols in codespace?!? -- might go wrong...\n", __FUNCTION__);
          return;
        }
      pic16_aopOp (op, ic, FALSE);
      pic16_mov2w_volatile (AOP (op));
      pic16_freeAsmop (op, NULL, ic, TRUE);
    }
  else if (op)
    {
      fprintf (stderr, "%s: not implemented for non-symbols (volatile operand might not be read)\n", __FUNCTION__);
    }                           // if
}

/*-----------------------------------------------------------------*/
/* genpic16Code - generate code for pic16 based controllers        */
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
genpic16Code (iCode * lic)
{
  iCode *ic;
  int cln = 0;

  pb = pic16_newpCodeChain (GcurMemmap, 0, pic16_newpCodeCharP ("; Starting pCode block"));
  pic16_addpBlock (pb);

#if 0
  /* if debug information required */
  if (options.debug && currFunc)
    {
      if (currFunc)
        {
          cdbSymbol (currFunc, cdbFile, FALSE, TRUE);
        }
    }
#endif

  for (ic = lic; ic; ic = ic->next)
    {
      initGenLineElement ();

      DEBUGpic16_emitcode (";ic ", "\t%c 0x%x\t(%s)", ic->op, ic->op, pic16_decodeOp (ic->op));
      if (cln != ic->lineno)
        {
          if (options.debug)
            {
              debugFile->writeCLine (ic);
            }

          if (!options.noCcodeInAsm)
            {
              pic16_addpCode2pBlock (pb, pic16_newpCodeCSource (ic->lineno, ic->filename,
                                     printCLine (ic->filename, ic->lineno)));
            }

          cln = ic->lineno;
        }

      if (options.iCodeInAsm)
        {
          const char *iLine;

          /* insert here code to print iCode as comment */
          iLine = printILine (ic);
          pic16_emitpcomment ("ic:%d: %s", ic->seq, iLine);
          dbuf_free (iLine);
        }

      /* if the result is marked as
       * spilt and rematerializable or code for
       * this has already been generated then
       * do nothing */
      if (resultRemat (ic) || ic->generated)
        continue;

      /* depending on the operation */
      switch (ic->op)
        {
        case '!':
          pic16_genNot (ic);
          break;

        case '~':
          pic16_genCpl (ic);
          break;

        case UNARYMINUS:
          genUminus (ic);
          break;

        case IPUSH:
          genIpush (ic);
          break;

        case IPOP:
          /* IPOP happens only when trying to restore a
           * spilt live range, if there is an ifx statement
           * following this pop then the if statement might
           * be using some of the registers being popped which
           * would destroy the contents of the register so
           * we need to check for this condition and handle it */
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
          pic16_genPlus (ic);
          break;

        case '-':
          if (!genDjnz (ic, ifxForOp (IC_RESULT (ic), ic)))
            pic16_genMinus (ic);
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
           * during parsing SDCC.y */
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
          pic16_genInline (ic);
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
        }
    }


  /* now we are ready to call the
     peep hole optimizer */
  if (!options.nopeep)
    peepHole (&genLine.lineHead);

  /* now do the actual printing */
  printLine (genLine.lineHead, codeOutBuf);

#ifdef PCODE_DEBUG
  DFPRINTF ((stderr, "printing pBlock\n\n"));
  pic16_printpBlock (stdout, pb);
#endif

  /* destroy the line list */
  destroy_line_list ();
}
