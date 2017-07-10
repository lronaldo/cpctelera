/*-------------------------------------------------------------------------

  SDCCicode.h - intermediate code generation etc.
                Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)

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
#include "SDCCbitv.h"
#include "SDCCset.h"

#ifndef SDCCICODE_H
#define SDCCICODE_H 1

extern symbol *returnLabel;
extern symbol *entryLabel;
extern int iCodeKey;
extern int operandKey;

enum
{
  CONDITIONAL = 0,
  EXPRESSION,
  STATEMENT,
  LEAF
};

typedef enum
{
  SYMBOL = 1,
  VALUE,
  TYPE
}
OPTYPE;

#define IS_SYMOP(op) (op && op->type == SYMBOL)
#define IS_VALOP(op) (op && op->type == VALUE)
#define IS_TYPOP(op) (op && op->type == TYPE)

#define ADDTOCHAIN(x) addSetHead(&iCodeChain,x)

#define LRFTYPE       sym_link *ltype = operandType(left), \
                           *rtype = operandType(right) ;
#define LRETYPE       sym_link *letype= getSpec(ltype)   , \
                           *retype= getSpec(rtype);
#define LRTYPE        LRFTYPE LRETYPE
#define IS_ITEMP(op)       (IS_SYMOP(op) && op->svt.symOperand->isitmp == 1)
#define IS_PARM(op)        (IS_SYMOP(op) && op->svt.symOperand->_isparm)
#define IS_ITEMPLBL(op)    (IS_ITEMP(op) && op->svt.symOperand->isilbl == 1)
#define IS_OP_VOLATILE(op) (IS_SYMOP(op) && op->isvolatile)
#define IS_OP_LITERAL(op)  (op && op->isLiteral)
#define IS_OP_GLOBAL(op)   (IS_SYMOP(op) && op->isGlobal)
#define IS_OP_POINTER(op)  (IS_SYMOP(op) && op->isPtr)
#define IS_OP_PARM(op)     (IS_SYMOP(op) && op->isParm)
#define OP_ISLIVE_FCALL(op) (IS_ITEMP(op) && OP_SYMBOL(op)->isLiveFcall)
#define SYM_SPIL_LOC(sym)  sym->usl.spillLoc

/* typedef for operand */
typedef struct operand
{
  OPTYPE type;                      /* type of operand */
  unsigned int isaddr:1;            /* is an address   */
  unsigned int aggr2ptr:2;          /* 1: must change aggregate to pointer to aggregate */
  /* 2: aggregate has been changed to pointer to aggregate */
  unsigned int isvolatile:1;        /* is a volatile operand */
  unsigned int isGlobal:1;          /* is a global operand */
  unsigned int isPtr:1;             /* is assigned a pointer */
  unsigned int isGptr:1;            /* is a generic pointer  */
  unsigned int isParm:1;            /* is a parameter        */
  unsigned int isLiteral:1;         /* operand is literal    */
  unsigned int isConstElimnated:1;  /* if original const casted to non-const */

  int key;
  union
  {
    struct symbol *symOperand;      /* operand is of type symbol */
    struct value *valOperand;       /* operand is of type value  */
    struct sym_link *typeOperand;   /* operand is of type typechain */
  }
  svt;

  bitVect *usesDefs;                /* which definitions are used by this */
  struct asmop *aop;                /* asm op for this operand */
}
operand;

extern operand *validateOpType (operand * op,
                                const char *macro, const char *args, OPTYPE type, const char *file, unsigned line);

extern const operand *validateOpTypeConst (const operand * op,
                                           const char *macro, const char *args, OPTYPE type, const char *file, unsigned line);

#define OP_SYMBOL(op)        validateOpType(op, "OP_SYMBOL", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand
#define OP_SYMBOL_CONST(op)  validateOpTypeConst(op, "OP_SYMBOL", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand
#define OP_VALUE(op)         validateOpType(op, "OP_VALUE", #op, VALUE, __FILE__, __LINE__)->svt.valOperand
#define OP_SYM_TYPE(op)      validateOpType(op, "OP_SYM_TYPE", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->type
#define OP_SYM_ETYPE(op)     validateOpType(op, "OP_SYM_ETYPE", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->etype
#define SPIL_LOC(op)         validateOpType(op, "SPIL_LOC", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->usl.spillLoc
#define OP_LIVEFROM(op)      validateOpType(op, "OP_LIVEFROM", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->liveFrom
#define OP_LIVETO(op)        validateOpType(op, "OP_LIVETO", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->liveTo
#define OP_REQV(op)          validateOpType(op, "OP_REQV", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->reqv
#define OP_KEY(op)           validateOpType(op, "OP_REQV", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->key
#define OP_TYPE(op)          validateOpType(op, "OP_TYPE", #op, TYPE, __FILE__, __LINE__)->svt.typeOperand

/* definition for intermediate code */
#define IC_RESULT(x)     (x)->ulrrcnd.lrr.result
#define IC_LEFT(x)       (x)->ulrrcnd.lrr.left
#define IC_RIGHT(x)      (x)->ulrrcnd.lrr.right
#define IC_COND(x)       (x)->ulrrcnd.cnd.condition
#define IC_TRUE(x)       (x)->ulrrcnd.cnd.trueLabel
#define IC_FALSE(x)      (x)->ulrrcnd.cnd.falseLabel
#define IC_LABEL(x)      (x)->label
#define IC_JTCOND(x)     (x)->ulrrcnd.jmpTab.condition
#define IC_JTLABELS(x)   (x)->ulrrcnd.jmpTab.labels
#define IC_INLINE(x)     (x)->inlineAsm
#define IC_ARRAYILIST(x) (x)->arrayInitList

typedef struct iCode
{
  unsigned int op;              /* operation defined */
  int key;                      /* running key for this iCode */
  int seq;                      /* sequence number within routine */
  int seqPoint;                 /* sequence point */
  short depth;                  /* loop depth of this iCode */
  short level;                  /* scope level */
  short block;                  /* sequential block number */
  unsigned nosupdate:1;         /* don't update spillocation with this */
  unsigned generated:1;         /* code generated for this one */
  unsigned parmPush:1;          /* parameter push Vs spill push */
  unsigned supportRtn:1;        /* will cause a call to a support routine */
  unsigned regsSaved:1;         /* registers have been saved */
  unsigned bankSaved:1;         /* register bank has been saved */
  unsigned builtinSEND:1;       /* SEND for parameter of builtin function */

  struct iCode *next;           /* next in chain */
  struct iCode *prev;           /* previous in chain */
  set *movedFrom;               /* if this iCode gets moved to another block */
  bitVect *rlive;               /* ranges that are live at this point */
  int defKey;                   /* key for the operand being defined  */
  bitVect *uses;                /* vector of key of used symbols      */
  bitVect *rUsed;               /* registers used by this instruction */
  bitVect *rMask;               /* registers in use during this instruction */
  bitVect *rSurv;               /* registers that survive this instruction (i.e. they are in use, it is not their last use and they are not in the return) */
  union
  {
    struct
    {
      operand *left;            /* left if any   */
      operand *right;           /* right if any  */
      operand *result;          /* result of this op */
    }
    lrr;

    struct
    {
      operand *condition;       /* if this is a conditional */
      symbol *trueLabel;        /* true for conditional     */
      symbol *falseLabel;       /* false for conditional    */
    }
    cnd;

    struct
    {
      operand *condition;       /* condition for the jump */
      set *labels;              /* ordered set of labels  */
    }
    jmpTab;

  }
  ulrrcnd;

  symbol *label;                /* for a goto statement     */

  const char *inlineAsm;        /* pointer to inline assembler code */
  literalList *arrayInitList;   /* point to array initializer list. */

  int lineno;                   /* file & lineno for debug information */
  char *filename;

  int parmBytes;                /* if call/pcall, count of parameter bytes
                                   on stack */
  int argreg;                   /* argument regno for SEND/RECEIVE */
  int eBBlockNum;               /* belongs to which eBBlock */
  char riu;                     /* after ralloc, the registers in use */
  float count;                  /* An execution count or probability */
  float pcount;                 /* For propagation of count */

  struct ast *tree;             /* ast node for this iCode (if not NULL) */
}
iCode;

/* various functions associated to iCode */
typedef struct icodeFuncTable
{
  int icode;
  char *printName;
  void (*iCodePrint) (struct dbuf_s *, iCode *, char *);
  iCode *(*iCodeCopy) (iCode *);
}
iCodeTable;

/* useful macros */
#define SKIP_IC2(x)  (x->op == GOTO         ||    \
                      x->op == LABEL        ||    \
                      x->op == FUNCTION     ||    \
                      x->op == INLINEASM    ||    \
                      x->op == ENDFUNCTION  )

#define SKIP_IC1(x)  (x->op == CALL         ||    \
                      SKIP_IC2(x) )

#define SKIP_IC(x)   (x->op == PCALL        ||    \
                      x->op == IPUSH        ||    \
                      x->op == IPOP         ||    \
                      x->op == JUMPTABLE    ||    \
                      x->op == RECEIVE      ||    \
                      x->op == ARRAYINIT    ||    \
                      SKIP_IC1(x)           ||    \
                      x->op == CRITICAL     ||    \
                      x->op == ENDCRITICAL  ||    \
                      x->op == SEND         )

#define SKIP_IC3(x) (SKIP_IC2(x)            ||    \
                     x->op == JUMPTABLE )

#define IS_CONDITIONAL(x) (x->op == EQ_OP || \
                           x->op == '<'   || \
                           x->op == '>'   || \
                           x->op == LE_OP || \
                           x->op == GE_OP || \
                           x->op == NE_OP )

#define IS_TRUE_SYMOP(op) (op && IS_SYMOP(op) && !IS_ITEMP(op))

#define POINTER_SET(ic) ( ic && ic->op == '='           \
                             && (IS_ITEMP(IC_RESULT(ic)) || IS_OP_LITERAL(IC_RESULT(ic)))\
                             && IC_RESULT(ic)->isaddr )

#define POINTER_GET(ic) ( ic && ic->op == GET_VALUE_AT_ADDRESS  \
                             && (IS_ITEMP(IC_LEFT(ic)) || IS_OP_LITERAL(IC_LEFT(ic)))\
                             && IC_LEFT(ic)->isaddr )

#define IS_ARITHMETIC_OP(x) (x && (x->op == '+' || \
                                   x->op == '-' || \
                                   x->op == '/' || \
                                   x->op == '*' || \
                                   x->op == '%'))
#define IS_BITWISE_OP(x) (x && (x->op == BITWISEAND || \
                                x->op == '|'        || \
                                x->op == '^'))

#define IS_COMMUTATIVE(x) (x && (x->op == EQ_OP      || \
                                 x->op == NE_OP      || \
                                 x->op == '+'        || \
                                 x->op == '*'        || \
                                 x->op == BITWISEAND || \
                                 x->op == '|'        || \
                                 x->op == '^'))

#define ASSIGNMENT(ic) ( ic && ic->op == '=')

#define ASSIGN_SYM_TO_ITEMP(ic) (ic && ic->op == '=' && \
                             IS_TRUE_SYMOP(IC_RIGHT(ic)) && \
                             IS_ITEMP(IC_RESULT(ic)))

#define ASSIGN_ITEMP_TO_SYM(ic) (ic && ic->op == '=' && \
                             IS_TRUE_SYMOP(IC_RESULT(ic)) && \
                             IS_ITEMP(IC_RIGHT(ic)))

#define ASSIGN_ITEMP_TO_ITEMP(ic) (ic && ic->op == '=' &&\
                                   !POINTER_SET(ic)    &&\
                                   IS_ITEMP(IC_RIGHT(ic)) &&\
                                   IS_ITEMP(IC_RESULT(ic)))

#define ADD_SUBTRACT_ITEMP(ic) (ic && (ic->op == '+' || ic->op == '-') && \
                                IS_ITEMP(IC_RESULT(ic)) && \
                                ( ( IS_ITEMP(IC_LEFT(ic)) ) ||  ( IS_SYMOP(IC_LEFT(ic)) ) ) && \
                                  IS_OP_LITERAL(IC_RIGHT(ic)))

#define ASSIGNMENT_TO_SELF(ic) (!POINTER_SET(ic) && !POINTER_GET(ic) && \
                                ic->op == '=' && IC_RESULT(ic)->key == IC_RIGHT(ic)->key )

#define IS_CAST_ICODE(ic) (ic && ic->op == CAST)
#define SET_ISADDR(op,v) {op = operandFromOperand(op); op->isaddr = v;}
#define SET_RESULT_RIGHT(ic) {SET_ISADDR(IC_RIGHT(ic),0); SET_ISADDR(IC_RESULT(ic),0);}
#define IS_ASSIGN_ICODE(ic) (ASSIGNMENT(ic) && !POINTER_SET(ic))

#define OP_DEFS(op) validateOpType(op, "OP_DEFS", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->defs
#define OP_USES(op) validateOpType(op, "OP_USES", #op, SYMBOL, __FILE__, __LINE__)->svt.symOperand->uses
/*-----------------------------------------------------------------*/
/* forward references for functions                                */
/*-----------------------------------------------------------------*/
iCode *reverseiCChain ();
bool isOperandOnStack (operand *);
int isOperandVolatile (const operand *, bool);
int isOperandGlobal (const operand *);
void printiCChain (iCode *, FILE *);
operand *ast2iCode (ast *, int);
operand *geniCodePtrPtrSubtract (operand *, operand *);
void initiCode ();
iCode *iCodeFromAst (ast *);
int isiCodeEqual (iCode *, iCode *);
int isOperandEqual (const operand *, const operand *);
iCodeTable *getTableEntry (int);
int isOperandLiteral (const operand * const);
operand *operandOperation (operand *, operand *, int, sym_link *);
double operandLitValue (operand *);
unsigned long long operandLitValueUll (operand *);
operand *operandFromLit (double);
operand *operandFromOperand (operand *);
int isParameterToCall (value *, operand *);
iCode *newiCodeLabelGoto (int, symbol *);
symbol *newiTemp (const char *);
symbol *newiTempLabel (const char *);
#define LOOPEXITLBL "loopExitLbl"
symbol *newiTempLoopHeaderLabel (bool);
iCode *newiCode (int, operand *, operand *);
sym_link *operandType (const operand *);
unsigned int operandSize (operand *);
operand *operandFromValue (value *);
operand *operandFromSymbol (symbol *);
operand *operandFromLink (sym_link *);
sym_link *aggrToPtr (sym_link *, bool);
int aggrToPtrDclType (sym_link *, bool);
void setOClass (sym_link * ptr, sym_link * spec);
int piCode (void *, FILE *);
int dbuf_printOperand (operand *, struct dbuf_s *);
int printOperand (operand *, FILE *);
void setOperandType (operand *, sym_link *);
bool isOperandInFarSpace (operand *);
bool isOperandInPagedSpace (operand *);
bool isOperandInDirSpace (operand *);
bool isOperandInBitSpace (operand *);
bool isOperandInCodeSpace (operand *);
operand *opFromOpWithDU (operand *, bitVect *, bitVect *);
iCode *copyiCode (iCode *);
operand *newiTempOperand (sym_link *, char);
operand *newiTempFromOp (operand *);
iCode *getBuiltinParms (iCode *, int *, operand **);
int isiCodeInFunctionCall (iCode *);
/*-----------------------------------------------------------------*/
/* declaration of exported variables                               */
/*-----------------------------------------------------------------*/
extern char *filename;
extern int lineno;
#endif

