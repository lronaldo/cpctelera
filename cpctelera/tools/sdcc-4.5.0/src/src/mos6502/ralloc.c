/*------------------------------------------------------------------------

  SDCCralloc.c - source file for register allocation. M6502 specific

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
  -------------------------------------------------------------------------*/

#include "ralloc.h"
#include "gen.h"
#include "dbuf_string.h"

/* Flags to turn on debugging code.
 */
enum
  {
    D_ALLOC = 0,
  };

#if 1
#define D(_a, _s)       if (_a)  { printf _s; fflush(stdout); }
#else
#define D(_a, _s)
#endif

/** Local static variables */
static struct
{
  bitVect *spiltSet;
  set *stackSpil;
  short blockSpil;
  int slocNum;
  int stackExtend;
  int dataExtend;
} _G;

/* 6502 registers */
reg_info regsm6502[] =
  {
    {REG_GPR, A_IDX,   "a",  M6502MASK_A,  NULL, 0, 1},
    {REG_GPR, X_IDX,   "x",  M6502MASK_X,  NULL, 0, 1},
    {REG_GPR, Y_IDX,   "y",  M6502MASK_Y,  NULL, 0, 1},
    {REG_CND, CND_IDX, "C",  0, NULL, 0, 1},
    {REG_GPR, XA_IDX,  "xa", M6502MASK_XA, NULL, 0, 1},
    //  {REG_GPR, YA_IDX,  "ya", M6502MASK_YA, NULL, 0, 1},
    {REG_GPR, YX_IDX,  "yx", M6502MASK_YX, NULL, 0, 1},
    {0,       SP_IDX,  "sp", 0, NULL, 0, 1},
  };

/* Shared with gen.c */
int m6502_ptrRegReq;             /* one byte pointer register required */

int m6502_nRegs = sizeof(regsm6502)/sizeof(reg_info);

reg_info *m6502_reg_a;
reg_info *m6502_reg_x;
reg_info *m6502_reg_y;
reg_info *m6502_reg_xa;
//reg_info *m6502_reg_ya;
reg_info *m6502_reg_yx;
reg_info *m6502_reg_sp;

static void m6502SpillThis (symbol *);
static void updateRegUsage (iCode * ic);
extern void genm6502Code (iCode *);


/*-----------------------------------------------------------------*/
/* m6502_regWithIdx - returns pointer to register with index number */
/*-----------------------------------------------------------------*/
reg_info *
m6502_regWithIdx (int idx)
{
  int i;

  for (i = 0; i < m6502_nRegs; i++)
    if (regsm6502[i].rIdx == idx)
      return &regsm6502[i];

  printf("error: regWithIdx %d not found\n",idx);
  exit (1);
}

/*-----------------------------------------------------------------*/
/* m6502_freeReg - frees a register                                */
/*-----------------------------------------------------------------*/
void
m6502_freeReg (reg_info * reg)
{
  if (!reg)
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
              "m6502_freeReg - Freeing NULL register");
      exit (1);
    }

  reg->isFree = 1;

  switch (reg->rIdx)
    {
    case A_IDX:
      if (m6502_reg_x->isFree)
	m6502_reg_xa->isFree = 1;
      break;
    case X_IDX:
      if (m6502_reg_a->isFree)
	m6502_reg_xa->isFree = 1;
      if (m6502_reg_y->isFree)
	m6502_reg_yx->isFree = 1;
      break;
    case Y_IDX:
      if (m6502_reg_x->isFree)
	m6502_reg_yx->isFree = 1;
      break;
    case XA_IDX:
      m6502_reg_x->isFree = 1;
      m6502_reg_a->isFree = 1;
      if (m6502_reg_y->isFree)
	m6502_reg_yx->isFree = 1;
      break;
      //      case YA_IDX:
      //        m6502_reg_y->isFree = 1;
      //        m6502_reg_a->isFree = 1;
      //        if (m6502_reg_x->isFree)
      //          m6502_reg_yx->isFree = 1;
      //        break;
    case YX_IDX:
      m6502_reg_y->isFree = 1;
      m6502_reg_x->isFree = 1;
      if (m6502_reg_a->isFree)
	m6502_reg_xa->isFree = 1;
      break;
    default:
      break;
    }
}

/*-----------------------------------------------------------------*/
/* noOverLap - will iterate through the list looking for over lap  */
/*-----------------------------------------------------------------*/
static int
noOverLap (set * itmpStack, symbol * fsym)
{
  symbol *sym;

  for (sym = setFirstItem (itmpStack); sym; sym = setNextItem (itmpStack))
    {
      if (bitVectBitValue(sym->clashes,fsym->key)) 
	return 0;
    }
  return 1;
}

/*-----------------------------------------------------------------*/
/* isFree - will return 1 if the a free spil location is found     */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (isFree)
{
  symbol *sym = item;
  V_ARG (symbol **, sloc);
  V_ARG (symbol *, fsym);

  /* if already found */
  if (*sloc)
    return 0;

  /* if it is free && and the itmp assigned to
     this does not have any overlapping live ranges
     with the one currently being assigned and
     the size can be accommodated  */
  if (sym->isFree
      && noOverLap (sym->usl.itmpStack, fsym)
      && getSize (sym->type) >= getSize (fsym->type))
    {
      *sloc = sym;
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* createStackSpil - create a location somewhere to spill          */
/*-----------------------------------------------------------------*/
static symbol *
createStackSpil (symbol * sym)
{
  symbol *sloc = NULL;
  struct dbuf_s dbuf;

  /* first go try and find a free one that is already
     existing on the stack */
  if (applyToSet (_G.stackSpil, isFree, &sloc, sym))
    {
      /* found a free one : just update & return */
      sym->usl.spillLoc = sloc;
      sym->stackSpil = 1;
      sloc->isFree = 0;
      addSetHead (&sloc->usl.itmpStack, sym);
      return sym;
    }

  /* could not then have to create one , this is the hard part
     we need to allocate this on the stack : this is really a
     hack!! but cannot think of anything better at this time */

  D (D_ALLOC, ("createStackSpil: for sym %p %s (old currFunc->stack %ld)\n", sym, sym->name, (long)(currFunc->stack)));

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "sloc%d", _G.slocNum++);
  sloc = newiTemp (dbuf_c_str (&dbuf));
  dbuf_destroy (&dbuf);

  /* set the type to the spilling symbol */
  sloc->type = copyLinkChain (sym->type);
  sloc->etype = getSpec (sloc->type);
  SPEC_SCLS (sloc->etype) = (options.xdata_spill)?S_XDATA:S_DATA;
  SPEC_EXTR (sloc->etype) = 0;
  SPEC_STAT (sloc->etype) = 0;
  SPEC_VOLATILE(sloc->etype) = 0;
  SPEC_ABSA(sloc->etype) = 0;

  /* we don't allow it to be allocated
     onto the external stack since : so we
     temporarily turn it off ; we also
     turn off memory model to prevent
     the spil from going to the external storage
  */

  //  useXstack = options.useXstack;
  //  model = options.model;
  /*     noOverlay = options.noOverlay; */
  /*     options.noOverlay = 1; */
  //  options.model = options.useXstack = 0;

  allocLocal (sloc);

  //  options.useXstack = useXstack;
  //  options.model = model;
  /*     options.noOverlay = noOverlay; */
  sloc->isref = 1;              /* to prevent compiler warning */

  /* if it is on the stack then update the stack */
  if (IN_STACK (sloc->etype))
    {
      currFunc->stack += getSize (sloc->type);
      _G.stackExtend += getSize (sloc->type);
    }
  else
    {
      _G.dataExtend += getSize (sloc->type);
    }

  /* add it to the stackSpil set */
  addSetHead (&_G.stackSpil, sloc);
  sym->usl.spillLoc = sloc;
  sym->stackSpil = 1;

  /* add it to the set of itempStack set
     of the spill location */
  addSetHead (&sloc->usl.itmpStack, sym);

  D (D_ALLOC, ("createStackSpil: created new %s\n", sloc->name));
  return sym;
}

/*-----------------------------------------------------------------*/
/* spillThis - spills a specific operand                           */
/*-----------------------------------------------------------------*/
static void
m6502SpillThis (symbol * sym)
{
  int i;
  /* if this is rematerializable or has a spillLocation
     we are okay, else we need to create a spillLocation
     for it */
  if (!(sym->remat || sym->usl.spillLoc))
    createStackSpil (sym);

  /* mark it as spilt & put it in the spilt set */
  sym->isspilt = sym->spillA = 1;
  _G.spiltSet = bitVectSetBit (_G.spiltSet, sym->key);

  for (i = 0; i < sym->nRegs; i++)
    {
      if (sym->regs[i])
        {
          m6502_freeReg (sym->regs[i]);
          sym->regs[i] = NULL;
        }
    }

  if (sym->usl.spillLoc && !sym->remat)
    sym->usl.spillLoc->allocreq++;
  return;
}

/*-----------------------------------------------------------------*/
/* deassignLRs - check the live to and if they have registers &    */
/*               are not spilt then free up the registers          */
/*-----------------------------------------------------------------*/
static void
deassignLRs (iCode * ic, eBBlock * ebp)
{
  symbol *sym;
  int k;

  for (sym = hTabFirstItem (liveRanges, &k); sym;
       sym = hTabNextItem (liveRanges, &k))
    {
      /* if it does not end here */
      if (sym->liveTo > ic->seq)
        continue;

      /* if it was spilt on stack then we can
         mark the stack spil location as free */
      if (sym->isspilt)
        {
          if (sym->stackSpil)
            {
              sym->usl.spillLoc->isFree = 1;
              sym->stackSpil = 0;
            }
          continue;
        }
    }
}

/*------------------------------------------------------------------*/
/* verifyRegsAssigned - make sure an iTemp is properly initialized; */
/* it should either have registers or have been spilled. Otherwise, */
/* there was an uninitialized variable, so just spill this to get   */
/* the operand in a valid state.                                    */
/*------------------------------------------------------------------*/
static void
verifyRegsAssigned (operand *op, iCode * ic)
{
  symbol * sym;

  if (!op)
    return;
  if (!IS_ITEMP (op))
    return;

  sym = OP_SYMBOL (op);
  if (sym->isspilt)
    return;
  if (!sym->nRegs)
    return;
  if (sym->regs[0])
    return;

  m6502SpillThis (sym);
}

/*-----------------------------------------------------------------*/
/* rUmaskForOp :- returns register mask for an operand             */
/*-----------------------------------------------------------------*/
bitVect *
m6502_rUmaskForOp (operand * op)
{
  bitVect *rumask;
  symbol *sym;
  int j;

  /* only temporaries are assigned registers */
  if (!IS_ITEMP (op))
    return NULL;

  sym = OP_SYMBOL (op);

  /* if spilt or no registers assigned to it
     then nothing */
  if (sym->isspilt || !sym->nRegs)
    return NULL;

  rumask = newBitVect (m6502_nRegs);

  for (j = 0; j < sym->nRegs; j++)
    {
      rumask = bitVectSetBit (rumask, sym->regs[j]->rIdx);
    }

  return rumask;
}

/*-----------------------------------------------------------------*/
/* regTypeNum - computes the type & number of registers required   */
/*-----------------------------------------------------------------*/
static void
regTypeNum (void)
{
  symbol *sym;
  int k;

  /* for each live range do */
  for (sym = hTabFirstItem (liveRanges, &k); sym;
       sym = hTabNextItem (liveRanges, &k))
    {
      /* if used zero times then no registers needed */
      if ((sym->liveTo - sym->liveFrom) == 0)
        continue;

      D (D_ALLOC, ("regTypeNum: loop on sym %p\n", sym));

      /* if the live range is a temporary */
      if (sym->isitmp)
        {
          /* if the type is marked as a conditional */
          if (sym->regType == REG_CND)
            continue;

          /* if used in return only then we don't
             need registers */
          if (sym->ruonly || sym->accuse)
            {
              if (IS_AGGREGATE (sym->type) || sym->isptr)
                sym->type = aggrToPtr (sym->type, false);
              continue;
            }

          /* if not then we require registers */
          D (D_ALLOC,
             ("regTypeNum: isagg %u nRegs %u type %p\n", IS_AGGREGATE (sym->type) || sym->isptr, sym->nRegs, sym->type));
          sym->nRegs = ((IS_AGGREGATE (sym->type) || sym->isptr) 
			? getSize (sym->type = aggrToPtr (sym->type, false)) 
			: getSize (sym->type));
          D (D_ALLOC, ("regTypeNum: setting nRegs of %s (%p) to %u\n", sym->name, sym, sym->nRegs));

          D (D_ALLOC, ("regTypeNum: setup to assign regs sym %p\n", sym));

          if (sym->nRegs > 8)
            {
              fprintf (stderr, "allocated more than 8 registers for type ");
              printTypeChain (sym->type, stderr);
              fprintf (stderr, "\n");
            }

          /* determine the type of register required */
          if (sym->nRegs == 1 && IS_PTR (sym->type) && sym->uptr)
            sym->regType = REG_PTR;
          else
            sym->regType = REG_GPR;
        }
      else
        {
          /* for the first run we don't provide */
          /* registers for true symbols we will */
          /* see how things go                  */
          D (D_ALLOC, ("regTypeNum: #2 setting num of %p to 0\n", sym));
          sym->nRegs = 0;
        }
    }
}

#if 0
/** Transform weird SDCC handling of writes via pointers
    into something more sensible. */
static void
transformPointerSet (eBBlock **ebbs, int count)
{
  /* for all blocks */
  for (int i = 0; i < count; i++)
    {
      iCode *ic;

      /* for all instructions do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        if (POINTER_SET (ic))
          {
            IC_LEFT (ic) = IC_RESULT (ic);
            IC_RESULT (ic) = 0;
            ic->op = SET_VALUE_AT_ADDRESS;
          }
    }
}
#endif

/*-----------------------------------------------------------------*/
/* deallocStackSpil - this will set the stack pointer back         */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (deallocStackSpil)
{
  symbol *sym = item;

  deallocLocal (sym);
  return 0;
}

/*-----------------------------------------------------------------*/
/* packRegsForAssign - register reduction for assignment           */
/*-----------------------------------------------------------------*/
static int
packRegsForAssign (iCode * ic, eBBlock * ebp)
{
  iCode *dic, *sic;

  if (!IS_ITEMP (IC_RIGHT (ic))
      || OP_SYMBOL (IC_RIGHT (ic))->isind
      || OP_LIVETO (IC_RIGHT (ic)) > ic->seq)
    {
      return 0;
    }

  /* if the true symbol is defined in far space or on stack
     then we should not since this will increase register pressure */
#if 0
  if (isOperandInFarSpace(IC_RESULT(ic)) && !farSpacePackable(ic))
    {
      return 0;
    }
#endif

  /* find the definition of iTempNN scanning backwards if we find
     a use of the true symbol before we find the definition then
     we cannot */
  for (dic = ic->prev; dic; dic = dic->prev)
    {
      int crossedCall = 0;

      /* We can pack across a function call only if it's a local */
      /* variable or our parameter. Never pack global variables */
      /* or parameters to a function we call. */
      if ((dic->op == CALL || dic->op == PCALL))
        {
          if (!OP_SYMBOL (IC_RESULT (ic))->ismyparm
              && !OP_SYMBOL (IC_RESULT (ic))->islocal)
            {
              crossedCall = 1;
            }
        }

      /* Don't move an assignment out of a critical block */
      if (dic->op == CRITICAL)
        {
          dic = NULL;
          break;
        }

      if (SKIP_IC2 (dic))
        continue;

      if (dic->op == IFX)
        {
          if (IS_SYMOP (IC_COND (dic))
              && (IC_COND (dic)->key == IC_RESULT (ic)->key
                  || IC_COND (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }
        }
      else
        {
          if (IS_TRUE_SYMOP (IC_RESULT (dic))
              && IS_OP_VOLATILE (IC_RESULT (dic)))
            {
              dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_RESULT (dic))
              && IC_RESULT (dic)->key == IC_RIGHT (ic)->key)
            {
              if (POINTER_SET (dic))
                dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_RIGHT (dic))
              && (IC_RIGHT (dic)->key == IC_RESULT (ic)->key
                  || IC_RIGHT (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_LEFT (dic))
              && (IC_LEFT (dic)->key == IC_RESULT (ic)->key
                  || IC_LEFT (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }

          if (POINTER_SET (dic) &&
              IC_RESULT (dic)->key == IC_RESULT (ic)->key)
            {
              dic = NULL;
              break;
            }

          if (crossedCall)
            {
              dic = NULL;
              break;
            }
        }
    }

  if (!dic)
    return 0;                   /* did not find */

  /* if assignment then check that right is not a bit */
  if (ASSIGNMENT (ic) && !POINTER_SET (ic))
    {
      sym_link *etype = operandType (IC_RESULT (dic));
      if (IS_BITFIELD (etype))
        {
          /* if result is a bit too then it's ok */
          etype = operandType (IC_RESULT (ic));
          if (!IS_BITFIELD (etype))
            {
              return 0;
            }
        }
    }

  /* found the definition */

  /* delete from liverange table also
     delete from all the points inbetween and the new
     one */
  for (sic = dic; sic != ic; sic = sic->next)
    {
      bitVectUnSetBit (sic->rlive, IC_RESULT (ic)->key);
      if (IS_ITEMP (IC_RESULT (dic)))
        bitVectSetBit (sic->rlive, IC_RESULT (dic)->key);
    }

  /* replace the result with the result of */
  /* this assignment and remove this assignment */
  bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
  ReplaceOpWithCheaperOp (&IC_RESULT (dic), IC_RESULT (ic));

  if (IS_ITEMP (IC_RESULT (dic)) && OP_SYMBOL (IC_RESULT (dic))->liveFrom > dic->seq)
    {
      OP_SYMBOL (IC_RESULT (dic))->liveFrom = dic->seq;
    }
  // TODO: and the otherway around?

  remiCodeFromeBBlock (ebp, ic);
  bitVectUnSetBit (OP_DEFS (IC_RESULT (ic)), ic->key);
  hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
  OP_DEFS (IC_RESULT (dic)) = bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
  return 1;
}

/*------------------------------------------------------------------*/
/* findAssignToSym : scanning backwards looks for first assig found */
/*------------------------------------------------------------------*/
static iCode *
findAssignToSym (operand * op, iCode * ic)
{
  iCode *dic;

  /* This routine is used to find sequences like
     iTempAA = FOO;
     ...;  (intervening ops don't use iTempAA or modify FOO)
     blah = blah + iTempAA;

     and eliminate the use of iTempAA, freeing up its register for
     other uses.
  */
  for (dic = ic->prev; dic; dic = dic->prev)
    {
      if (dic->op == '=' &&
          !POINTER_SET (dic) &&
          IC_RESULT (dic)->key == op->key
          &&  IS_TRUE_SYMOP(IC_RIGHT(dic))
	  )
        break;  /* found where this temp was defined */

      /* if we find an usage then we cannot delete it */
      if (IC_LEFT (dic) && IC_LEFT (dic)->key == op->key)
        return NULL;

      if (IC_RIGHT (dic) && IC_RIGHT (dic)->key == op->key)
        return NULL;

      if (POINTER_SET (dic) && IC_RESULT (dic)->key == op->key)
        return NULL;
    }

  if (!dic)
    return NULL;   /* didn't find any assignment to op */
  /* we are interested only if defined in far space */
  /* or in stack space in case of + & - */

  /* if assigned to a non-symbol then don't repack regs */
  if (!IS_SYMOP (IC_RIGHT (dic)))
    return NULL;

  /* if the symbol's address has been taken, there might be a */
  /* non-obvious assignment to it, and so we should not */
  if (OP_SYMBOL (IC_RIGHT (dic))->addrtaken)
    return NULL;

  /* if the symbol is volatile then we should not */
  if (isOperandVolatile (IC_RIGHT (dic), true))
    return NULL;
  /* XXX TODO --- should we be passing false to isOperandVolatile()?
     What does it mean for an iTemp to be volatile, anyway? Passing
     true is more cautious but may prevent possible optimizations */

  /* if the symbol is in far space then we should not */
  /* if (isOperandInFarSpace (IC_RIGHT (dic)))
     return NULL; */


  /* now make sure that the right side of dic
     is not defined between ic & dic */
  if (dic)
    {
      iCode *sic = dic->next;

      for (; sic != ic; sic = sic->next)
        if (IC_RESULT (sic) &&
            IC_RESULT (sic)->key == IC_RIGHT (dic)->key)
          return NULL;
    }

  return dic;
}

/*-----------------------------------------------------------------*/
/* reassignAliasedSym - used by packRegsForSupport to replace      */
/*                      redundant iTemp with equivalent symbol     */
/*-----------------------------------------------------------------*/
static void
reassignAliasedSym (eBBlock *ebp, iCode *assignment, iCode *use, operand *op)
{
  iCode *ic;
  unsigned oldSymKey, newSymKey;

  oldSymKey = op->key;
  newSymKey = IC_RIGHT(assignment)->key;

  /* only track live ranges of compiler-generated temporaries */
  if (!IS_ITEMP(IC_RIGHT(assignment)))
    newSymKey = 0;

  /* update the live-value bitmaps */
  for (ic = assignment; ic != use; ic = ic->next) {
    bitVectUnSetBit (ic->rlive, oldSymKey);
    if (newSymKey != 0)
      ic->rlive = bitVectSetBit (ic->rlive, newSymKey);
  }

  /* update the sym of the used operand */
  OP_SYMBOL(op) = OP_SYMBOL(IC_RIGHT(assignment));
  op->key = OP_SYMBOL(op)->key;

  /* update the sym's liverange */
  if ( OP_LIVETO(op) < ic->seq )
    setToRange(op, ic->seq, false);

  /* remove the assignment iCode now that its result is unused */
  remiCodeFromeBBlock (ebp, assignment);
  bitVectUnSetBit(OP_SYMBOL(IC_RESULT(assignment))->defs, assignment->key);
  hTabDeleteItem (&iCodehTab, assignment->key, assignment, DELETE_ITEM, NULL);
}


/*-----------------------------------------------------------------*/
/* packRegsForSupport :- reduce some registers for support calls   */
/*-----------------------------------------------------------------*/
static int
packRegsForSupport (iCode * ic, eBBlock * ebp)
{
  iCode *dic;
  int changes = 0;

  /* for the left & right operand :- look to see if the
     left was assigned a true symbol in far space in that
     case replace them */

  if (IS_ITEMP (IC_LEFT (ic)) &&
      OP_SYMBOL (IC_LEFT (ic))->liveTo <= ic->seq)
    {
      dic = findAssignToSym (IC_LEFT (ic), ic);

      if (dic)
        {
          /* found it we need to remove it from the block */
          reassignAliasedSym (ebp, dic, ic, IC_LEFT(ic));
          changes++;
        }
    }

  /* do the same for the right operand */
  if (IS_ITEMP (IC_RIGHT (ic)) &&
      OP_SYMBOL (IC_RIGHT (ic))->liveTo <= ic->seq)
    {
      iCode *dic = findAssignToSym (IC_RIGHT (ic), ic);

      if (dic)
        {
          /* found it we need to remove it from the block */
          reassignAliasedSym (ebp, dic, ic, IC_RIGHT(ic));
          changes++;
        }
    }

  return changes;
}

/*-----------------------------------------------------------------*/
/* isBitwiseOptimizable - requirements of JEAN LOUIS VERN          */
/*-----------------------------------------------------------------*/
static bool
isBitwiseOptimizable (iCode * ic)
{
  sym_link *ltype = getSpec (operandType (IC_LEFT (ic)));
  sym_link *rtype = getSpec (operandType (IC_RIGHT (ic)));

  /* bitwise operations are considered optimizable
     under the following conditions (Jean-Louis VERN)

     x & lit
     bit & bit
     bit & x
     bit ^ bit
     bit ^ x
     x   ^ lit
     x   | lit
     bit | bit
     bit | x
  */
  if (IS_LITERAL(rtype) ||
      (IS_BITVAR (ltype) && IN_BITSPACE (SPEC_OCLS (ltype))))
    return true;
  else
    return false;
}

/*-----------------------------------------------------------------*/
/* packForPush - heuristics to reduce iCode for pushing            */
/*-----------------------------------------------------------------*/
static void
packForPush (iCode * ic, eBBlock ** ebpp, int count)
{
  iCode *dic, *lic;
  bitVect *dbv;
  int disallowHiddenAssignment = 0;
  eBBlock * ebp = ebpp[ic->eBBlockNum];

  if ((ic->op != IPUSH && ic->op != SEND) || !IS_ITEMP (IC_LEFT (ic)))
    return;

  /* must have only definition & one usage */
  if (bitVectnBitsOn (OP_DEFS (IC_LEFT (ic))) != 1 ||
      bitVectnBitsOn (OP_USES (IC_LEFT (ic))) != 1)
    return;

  /* find the definition */
  if (!(dic = hTabItemWithKey (iCodehTab,
                               bitVectFirstBit (OP_DEFS (IC_LEFT (ic))))))
    return;

  if (dic->op != '=' || POINTER_SET (dic))
    return;

  if (dic->seq < ebp->fSeq || dic->seq > ebp->lSeq) // Evelyn did this
    {
      int i;
      for (i=0; i<count; i++)
        {
          if (dic->seq >= ebpp[i]->fSeq && dic->seq <= ebpp[i]->lSeq)
            {
              ebp=ebpp[i];
              break;
            }
        }
      if (i==count) // Abort if we can't find the definition's block
        return;
    }

  if (IS_SYMOP(IC_RIGHT(dic)))
    {
      if (IC_RIGHT (dic)->isvolatile)
        return;

      if (OP_SYMBOL (IC_RIGHT (dic))->addrtaken || isOperandGlobal (IC_RIGHT (dic)))
        disallowHiddenAssignment = 1;

      /* make sure the right side does not have any definitions
         inbetween */
      dbv = OP_DEFS(IC_RIGHT(dic));
      for (lic = ic; lic && lic != dic ; lic = lic->prev)
        {
          if (bitVectBitValue(dbv,lic->key))
            return ;
          if (disallowHiddenAssignment && (lic->op == CALL || lic->op == PCALL || POINTER_SET (lic)))
            return;
        }
      /* make sure they have the same type */
      if (IS_SPEC(operandType(IC_LEFT(ic))))
        {
          sym_link *itype=operandType(IC_LEFT(ic));
          sym_link *ditype=operandType(IC_RIGHT(dic));

          if (SPEC_USIGN(itype)!=SPEC_USIGN(ditype) ||
              SPEC_LONG(itype)!=SPEC_LONG(ditype))
            return;
        }
      /* extend the live range of replaced operand if needed */
      if (OP_SYMBOL(IC_RIGHT(dic))->liveTo < ic->seq)
        {
          OP_SYMBOL(IC_RIGHT(dic))->liveTo = ic->seq;
        }
      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
    }
  if (IS_ITEMP (IC_RIGHT (dic)))
    OP_USES (IC_RIGHT (dic)) = bitVectSetBit (OP_USES (IC_RIGHT (dic)), ic->key);

  /* we now we know that it has one & only one def & use
     and the that the definition is an assignment */
  ReplaceOpWithCheaperOp(&IC_LEFT (ic), IC_RIGHT (dic));
  remiCodeFromeBBlock (ebp, dic);
  hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
}

/*------------------------------------------------------------------*/
/* moveSendToCall - move SEND to immediately precede its CALL/PCALL */
/*------------------------------------------------------------------*/
static iCode *
moveSendToCall (iCode *sic, eBBlock *ebp)
{
  iCode * prev = sic->prev;
  iCode * sic2 = NULL;
  iCode * cic;
  
  /* Go find the CALL/PCALL */
  cic = sic;
  while (cic && cic->op != CALL && cic->op != PCALL)
    cic = cic->next;
  if (!cic)
    return sic;

  /* Is there a second SEND? If so, we'll need to move it too. */
  if (sic->next->op == SEND)
    sic2 = sic->next;
  
  /* relocate the SEND(s) */
  remiCodeFromeBBlock (ebp, sic);
  addiCodeToeBBlock (ebp, sic, cic);
  if (sic2)
    {
      remiCodeFromeBBlock (ebp, sic2);
      addiCodeToeBBlock (ebp, sic2, cic);
    }

  /* Return the iCode to continue processing at. */
  if (prev)
    return prev->next;
  else
    return ebp->sch;
}


/*---------------------------------------------------------------------*/
/* packPointerOp - see if we can move an offset from addition iCode    */
/*                 to the pointer iCode to used indexed addr mode      */
/* The z80-related ports do a similar thing in SDCCopt.c, offsetFold() */
/*---------------------------------------------------------------------*/
static void
packPointerOp (iCode * ic, eBBlock ** ebpp)
{
  operand * pointer;
  operand * offsetOp;
  operand * nonOffsetOp;
  iCode * dic;
  iCode * uic;
  int key;

  if (POINTER_SET (ic))
    {
      pointer = IC_RESULT (ic);
      offsetOp = IC_LEFT (ic);
    }
  else if (POINTER_GET (ic))
    {
      pointer = IC_LEFT (ic);
      offsetOp = IC_RIGHT (ic);
    }
  else
    return;

  if (!IS_ITEMP (pointer))
    return;

  /* If the pointer is rematerializable, it's already fully optimized */
  if (OP_SYMBOL (pointer)->remat)
    return;

  if (offsetOp && IS_OP_LITERAL (offsetOp) && operandLitValue (offsetOp) != 0)
    return;
  if (offsetOp && IS_SYMOP (offsetOp))
    return;

  /* There must be only one definition */
  if (bitVectnBitsOn (OP_DEFS (pointer)) != 1)
    return;
  /* find the definition */
  if (!(dic = hTabItemWithKey (iCodehTab, bitVectFirstBit (OP_DEFS (pointer)))))
    return;

  if (dic->op == '+' && (IS_OP_LITERAL (IC_RIGHT (dic)) ||
			 (IS_ITEMP (IC_RIGHT (dic)) && OP_SYMBOL (IC_RIGHT (dic))->remat)))
    {
      nonOffsetOp = IC_LEFT (dic);
      offsetOp = IC_RIGHT (dic);
    }
  else if (dic->op == '+' && IS_ITEMP (IC_LEFT (dic)) && OP_SYMBOL (IC_LEFT (dic))->remat)
    {
      nonOffsetOp = IC_RIGHT (dic);
      offsetOp = IC_LEFT (dic);
    }
  else
    return;


  /* Now check all of the uses to make sure they are all get/set pointer */
  /* and don't already have a non-zero offset operand */
  for (key=0; key<OP_USES (pointer)->size; key++)
    {
      if (bitVectBitValue (OP_USES (pointer), key))
        {
          uic = hTabItemWithKey (iCodehTab, key);
          if (POINTER_GET (uic))
            {
              if (IC_RIGHT (uic) && IS_OP_LITERAL (IC_RIGHT (uic)) && operandLitValue (IC_RIGHT (uic)) != 0)
                return;
              if (IC_RIGHT (uic) && IS_SYMOP (IC_RIGHT (uic)))
                return;
            }
          else if (POINTER_SET (uic))
            {
	      // FIXME: this code will make bug-3385.c fail
	      // it seems there are some corner cases where this 
	      // optimization is unsafe.
	      // Other cases might also be unsafe but no test triggers it.
	      // HC08 is also affected by the same issue.
	      //  return;
	      // remove the return above to trigger the failure.
              if (IC_LEFT (uic) && IS_OP_LITERAL (IC_LEFT (uic)) && operandLitValue (IC_LEFT (uic)) != 0)
                return;
              if (IC_LEFT (uic) && IS_SYMOP (IC_LEFT (uic)))
                return;
            }
          else
            return;
        }
    }

  /* Everything checks out. Move the literal or rematerializable offset */
  /* to the pointer get/set iCodes */
  for (key=0; key<OP_USES (pointer)->size; key++)
    {
      if (bitVectBitValue (OP_USES (pointer), key))
        {
          uic = hTabItemWithKey (iCodehTab, key);
          if (POINTER_GET (uic))
            {
              IC_RIGHT (uic) = offsetOp;
              if (IS_SYMOP (offsetOp))
                OP_USES (offsetOp) = bitVectSetBit (OP_USES (offsetOp), key);
            }
          else if (POINTER_SET (uic))
            {
              IC_LEFT (uic) = offsetOp;
              if (IS_SYMOP (offsetOp))
                OP_USES (offsetOp) = bitVectSetBit (OP_USES (offsetOp), key);
            }
          else
            return;
        }
    }

  /* Put the remaining operand on the right and convert to assignment     */
  if (IS_SYMOP (offsetOp))
    bitVectUnSetBit (OP_USES (offsetOp), dic->key);
  IC_RIGHT (dic) = nonOffsetOp;
  IC_LEFT (dic) = NULL;
  SET_ISADDR (IC_RESULT (dic), 0);
  dic->op = '=';
}

/*-----------------------------------------------------------------*/
/* packRegisters - does some transformations to reduce register    */
/*                 pressure                                        */
/*-----------------------------------------------------------------*/
static void
packRegisters (eBBlock ** ebpp, int count)
{
  iCode *ic;
  int change = 0;
  int blockno;

  for (blockno=0; blockno<count; blockno++)
    {
      eBBlock *ebp = ebpp[blockno];

      do
	{
	  change = 0;
	  /* look for assignments of the form */
	  /* iTempNN = TrueSym (someoperation) SomeOperand */
	  /*       ....                       */
	  /* TrueSym := iTempNN:1             */
	  for (ic = ebp->sch; ic; ic = ic->next)
	    {
	      /* find assignment of the form TrueSym := iTempNN:1 */
	      if (ic->op == '=' && !POINTER_SET (ic))
		change += packRegsForAssign (ic, ebp);
	    }
	}
      while (change);

      for (ic = ebp->sch; ic; ic = ic->next)
	{
	  //packRegsForLiteral (ic);
      
	  /* move SEND to immediately precede its CALL/PCALL */
	  if (ic->op == SEND && ic->next &&
	      ic->next->op != CALL && ic->next->op != PCALL)
	    {
	      ic = moveSendToCall (ic, ebp);
	    }
      
	  /* if this is an itemp & result of an address of a true sym
	     then mark this as rematerialisable   */
	  if (ic->op == ADDRESS_OF &&
	      IS_ITEMP (IC_RESULT (ic)) &&
	      IS_TRUE_SYMOP (IC_LEFT (ic)) &&
	      bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 &&
	      !OP_SYMBOL (IC_LEFT (ic))->onStack)
	    {
	      OP_SYMBOL (IC_RESULT (ic))->remat = 1;
	      OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
	      OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
	    }

	  /* Safe: just propagates the remat flag */
	  /* if straight assignment then carry remat flag if this is the
	     only definition */
	  if (ic->op == '=' &&
	      !POINTER_SET (ic) &&
	      IS_SYMOP (IC_RIGHT (ic)) &&
	      OP_SYMBOL (IC_RIGHT (ic))->remat &&
	      bitVectnBitsOn (OP_SYMBOL (IC_RESULT (ic))->defs) <= 1 &&
	      !OP_SYMBOL (IC_RESULT (ic))->_isparm &&
	      !OP_SYMBOL (IC_RESULT (ic))->addrtaken &&
	      !isOperandGlobal (IC_RESULT (ic)))
	    {
	      OP_SYMBOL (IC_RESULT (ic))->remat = OP_SYMBOL (IC_RIGHT (ic))->remat;
	      OP_SYMBOL (IC_RESULT (ic))->rematiCode = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
	    }

	  /* if cast to a pointer & the pointer being
	     cast is remat, then we can remat this cast as well */
	  if (ic->op == CAST &&
	      IS_SYMOP(IC_RIGHT(ic)) &&
	      OP_SYMBOL(IC_RIGHT(ic))->remat &&
	      bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 &&
	      !OP_SYMBOL (IC_RESULT (ic))->_isparm &&
	      !OP_SYMBOL (IC_RESULT (ic))->addrtaken &&
	      !isOperandGlobal (IC_RESULT (ic)))
	    {
	      sym_link *to_type = operandType(IC_LEFT(ic));
	      sym_link *from_type = operandType(IC_RIGHT(ic));
              if (IS_PTR(to_type) && IS_PTR(from_type))
		{
		  OP_SYMBOL (IC_RESULT (ic))->remat = 1;
		  OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
		  OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
		}
	    }

	  /* if this is a +/- operation with a rematerializable
	     then mark this as rematerializable as well */
	  if ((ic->op == '+' || ic->op == '-') &&
	      (IS_SYMOP (IC_LEFT (ic)) &&
	       IS_ITEMP (IC_RESULT (ic)) &&
	       IS_OP_LITERAL (IC_RIGHT (ic))) &&
	      OP_SYMBOL (IC_LEFT (ic))->remat &&
	      (!IS_SYMOP (IC_RIGHT (ic)) || !IS_CAST_ICODE(OP_SYMBOL (IC_RIGHT (ic))->rematiCode)) &&
	      bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1)
	    {
	      OP_SYMBOL (IC_RESULT (ic))->remat = 1;
	      OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
	      OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
	    }
	  /* mark the pointer usages */
	  if (POINTER_SET (ic) && IS_SYMOP (IC_RESULT (ic)))
	    OP_SYMBOL (IC_RESULT (ic))->uptr = 1;

	  if (POINTER_GET (ic) && IS_SYMOP (IC_LEFT (ic)))
	    OP_SYMBOL (IC_LEFT (ic))->uptr = 1;

	  /* reduce for support function calls */
	  if (ic->supportRtn || (ic->op != IFX && ic->op != JUMPTABLE))
	    packRegsForSupport (ic, ebp);

	  /* if the condition of an if instruction
	     is defined in the previous instruction and
	     this is the only usage then
	     mark the itemp as a conditional */
	  if ((IS_CONDITIONAL (ic) ||
	       (IS_BITWISE_OP(ic) && isBitwiseOptimizable (ic))) &&
	      ic->next && ic->next->op == IFX &&
	      bitVectnBitsOn (OP_USES(IC_RESULT(ic)))==1 &&
	      isOperandEqual (IC_RESULT (ic), IC_COND (ic->next)) &&
	      OP_SYMBOL (IC_RESULT (ic))->liveTo <= ic->next->seq)
	    {
	      OP_SYMBOL (IC_RESULT (ic))->regType = REG_CND;
	      continue;
	    }

	  /* pack for PUSH
	     iTempNN := (some variable in farspace) V1
	     push iTempNN ;
	     -------------
	     push V1
	  */
	  if (ic->op == IPUSH || ic->op == SEND)
	    {
              packForPush (ic, ebpp, count);
	    }

	  if (POINTER_SET (ic) || POINTER_GET (ic))
	    packPointerOp (ic, ebpp);

        }
    }
}

/*-----------------------------------------------------------------*/
/*   Mark variables for assignment by the register allocator.      */
/*-----------------------------------------------------------------*/
static void
serialRegMark (eBBlock ** ebbs, int count)
{
  int i;
  short int max_alloc_bytes = SHRT_MAX; // Byte limit. Set this to a low value to pass only few variables to the register allocator. This can be useful for debugging.

  D (D_ALLOC, ("serialRegMark for %s, currFunc->stack %d\n", currFunc->name, currFunc->stack));

  /* for all blocks */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath
          && (ebbs[i]->entryLabel != entryLabel
              && ebbs[i]->entryLabel != returnLabel))
        continue;

      /* for all instructions do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          updateRegUsage(ic);

          /* if result is present && is a true symbol */
          if (IC_RESULT (ic) && ic->op != IFX &&
              IS_TRUE_SYMOP (IC_RESULT (ic)))
            {
              OP_SYMBOL (IC_RESULT (ic))->allocreq++;
            }

          /* take away registers from live
             ranges that end at this instruction */
          deassignLRs (ic, ebbs[i]);

          /* some don't need registers */
          if (SKIP_IC2 (ic) ||
              ic->op == JUMPTABLE ||
              ic->op == IFX ||
              ic->op == IPUSH ||
              (IC_RESULT (ic) && POINTER_SET (ic)))
	    continue;

          /* now we need to allocate registers only for the result */
          if (IC_RESULT (ic))
            {
              symbol *sym = OP_SYMBOL (IC_RESULT (ic));

              /* Make sure any spill location is definitely allocated */
              if (sym->isspilt && !sym->remat && sym->usl.spillLoc &&
                  !sym->usl.spillLoc->allocreq)
                {
                  sym->usl.spillLoc->allocreq++;
                }

              /* if it does not need or is spilt
                 or is already assigned to registers
                 or will not live beyond this instructions */
              if (!sym->nRegs ||
                  sym->isspilt ||
                  sym->liveTo <= ic->seq)
                {
                  D (D_ALLOC, ("serialRegMark: won't live long enough.\n"));
                  continue;
                }

              /* if some liverange has been spilt at the block level
                 and this one live beyond this block then spill this
                 to be safe */
              if (_G.blockSpil && sym->liveTo > ebbs[i]->lSeq)
                {
                  m6502SpillThis (sym);
                  continue;
                }

              if (sym->remat)
                {
                  m6502SpillThis (sym);
                  continue;
                }

              if (max_alloc_bytes >= sym->nRegs)
                {
                  sym->for_newralloc = 1;
                  max_alloc_bytes -= sym->nRegs;
                }
              else if (!sym->for_newralloc)
                {
                  m6502SpillThis (sym);
                  printf ("Spilt %s due to byte limit.\n", sym->name);
                }
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* RegFix                                                          */
/*-----------------------------------------------------------------*/
void
m6502RegFix (eBBlock ** ebbs, int count)
{
  int i;

  /* Check for and fix any problems with uninitialized operands */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath && (ebbs[i]->entryLabel != entryLabel && ebbs[i]->entryLabel != returnLabel))
        continue;

      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          deassignLRs (ic, ebbs[i]);

          if (SKIP_IC2 (ic))
            continue;

          if (ic->op == IFX)
            {
              verifyRegsAssigned (IC_COND (ic), ic);
              continue;
            }

          if (ic->op == JUMPTABLE)
            {
              verifyRegsAssigned (IC_JTCOND (ic), ic);
              continue;
            }

          verifyRegsAssigned (IC_RESULT (ic), ic);
          verifyRegsAssigned (IC_LEFT (ic), ic);
          verifyRegsAssigned (IC_RIGHT (ic), ic);
        }
    }
}

/*-----------------------------------------------------------------*/
/* assignRegisters - assigns registers to each live range as need  */
/*-----------------------------------------------------------------*/
void
m6502_assignRegisters (ebbIndex *ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  iCode *ic;
  int i;

  m6502_ptrRegReq = _G.stackExtend = _G.dataExtend = 0;
  // TODO: add asserts to make sure IDX is correct
  m6502_reg_a = m6502_regWithIdx(A_IDX);
  m6502_reg_x = m6502_regWithIdx(X_IDX);
  m6502_reg_y = m6502_regWithIdx(Y_IDX);
  m6502_reg_xa = m6502_regWithIdx(XA_IDX);
  //  m6502_reg_ya = m6502_regWithIdx(YA_IDX);
  m6502_reg_yx = m6502_regWithIdx(YX_IDX);
  m6502_reg_sp = m6502_regWithIdx(SP_IDX);

  //  transformPointerSet (ebbs, count);

  /* change assignments this will remove some
     live ranges reducing some register pressure */

  packRegisters (ebbs, count);

  /* liveranges probably changed by register packing
     so we compute them again */
  recomputeLiveRanges (ebbs, count, false);

  if (options.dump_i_code)
    dumpEbbsToFileExt (DUMP_PACK, ebbi);

  /* first determine for each live range the number of
     registers & the type of registers required for each */
  regTypeNum ();

  /* Mark variables for assignment by the new allocator */
  serialRegMark (ebbs, count);

  /* Invoke optimal register allocator */
  ic = m6502_ralloc2_cc (ebbi);

  /* if stack was extended then tell the user */
  if (_G.stackExtend)
    {
      //      printf("Stack spills for %s: %d\n", currFunc->name, _G.stackExtend);
      _G.stackExtend = 0;
    }

  if (_G.dataExtend)
    {
      //      printf("Data spills for %s: %d\n", currFunc->name, _G.dataExtend);
      _G.dataExtend = 0;
    }

  /* redo that offsets for stacked automatic variables */
  if (currFunc)
    {
      redoStackOffsets ();
    }

  if (options.dump_i_code)
    {
      dumpEbbsToFileExt (DUMP_RASSGN, ebbi);
      dumpLiveRanges (DUMP_LRANGE, liveRanges);
    }

  /* now get back the chain */
  ic = iCodeLabelOptimize (iCodeFromeBBlock (ebbs, count));

  genm6502Code (ic);

  /* free up any _G.stackSpil locations allocated */
  applyToSet (_G.stackSpil, deallocStackSpil);
  _G.slocNum = 0;
  setToNull ((void *) &_G.stackSpil);
  setToNull ((void *) &_G.spiltSet);

  /* mark all registers as free */
  for (i = 0; i < m6502_nRegs; i++) {
    reg_info *reg = m6502_regWithIdx (i);
    m6502_freeReg (reg);
    m6502_dirtyReg (reg);
  }

  return;
}

/*-----------------------------------------------------------------*/
/* m6502_useReg - marks a register  as used                        */
/*-----------------------------------------------------------------*/
void
m6502_useReg (reg_info * reg)
{
  reg->isFree = 0;

  switch (reg->rIdx)
    {
    case A_IDX:
      m6502_reg_xa->aop = NULL;
      m6502_reg_xa->isFree = 0;
      //        m6502_reg_ya->aop = NULL;
      //        m6502_reg_ya->isFree = 0;
      break;
    case X_IDX:
      m6502_reg_xa->aop = NULL;
      m6502_reg_xa->isFree = 0;
      m6502_reg_yx->aop = NULL;
      m6502_reg_yx->isFree = 0;
      break;
    case Y_IDX:
      //        m6502_reg_ya->aop = NULL;
      //        m6502_reg_ya->isFree = 0;
      m6502_reg_yx->aop = NULL;
      m6502_reg_yx->isFree = 0;
      break;
    case XA_IDX:
      m6502_reg_x->aop = NULL;
      m6502_reg_x->isFree = 0;
      m6502_reg_a->aop = NULL;
      m6502_reg_a->isFree = 0;
      break;
      //      case YA_IDX:
      //        m6502_reg_y->aop = NULL;
      //        m6502_reg_y->isFree = 0;
      //        m6502_reg_a->aop = NULL;
      //        m6502_reg_a->isFree = 0;
      //        break;
    case YX_IDX:
      m6502_reg_y->aop = NULL;
      m6502_reg_y->isFree = 0;
      m6502_reg_x->aop = NULL;
      m6502_reg_x->isFree = 0;
      break;
    default:
      break;
    }
}

/*-----------------------------------------------------------------*/
/* m6502_dirtyReg - marks a register as dirty                      */
/*-----------------------------------------------------------------*/
void
m6502_dirtyReg (reg_info * reg)
{
  reg->aop = NULL;
  reg->isLitConst=0;

  switch (reg->rIdx)
    {
    case A_IDX:
      m6502_reg_xa->aop = NULL;
      m6502_reg_xa->isLitConst = 0;
      //	m6502_reg_ya->aop = NULL;
      //	m6502_reg_ya->isLitConst = 0;
      break;
    case X_IDX:
      m6502_reg_xa->aop = NULL;
      m6502_reg_xa->isLitConst = 0;
      m6502_reg_yx->aop = NULL;
      m6502_reg_yx->isLitConst = 0;
      break;
    case Y_IDX:
      //      m6502_reg_ya->aop = NULL;
      //	m6502_reg_ya->isLitConst = 0;
      m6502_reg_yx->aop = NULL;
      m6502_reg_yx->isLitConst = 0;
      break;
    case XA_IDX:
      m6502_reg_x->aop = NULL;
      m6502_reg_x->isLitConst = 0;
      m6502_reg_a->aop = NULL;
      m6502_reg_a->isLitConst = 0;
      break;
      //      case YA_IDX:
      //        m6502_reg_y->aop = NULL;
      //	m6502_reg_y->isLitConst = 0;
      //        m6502_reg_a->aop = NULL;
      //	m6502_reg_a->isLitConst = 0;
      break;
    case YX_IDX:
      m6502_reg_y->aop = NULL;
      m6502_reg_y->isLitConst = 0;
      m6502_reg_x->aop = NULL;
      m6502_reg_x->isLitConst = 0;
      break;
    default:
      break;
    }
}

/*-----------------------------------------------------------------*/
/* updateRegUsage -  update the registers in use at the start of   */
/*                   this icode                                    */
/*-----------------------------------------------------------------*/
static void
updateRegUsage (iCode * ic)
{
  int reg;

  // update the registers in use at the start of this icode
  for (reg=0; reg<m6502_nRegs; reg++)
    {
      if (regsm6502[reg].isFree)
        {
          ic->riu &= ~(regsm6502[reg].mask);
        }
      else
        {
          ic->riu |= (regsm6502[reg].mask);
        }
    }
}

