#include "ralloc.h"
#include "gen.h"

#include "dbuf_string.h"

reg_info stm8_regs[] =
{
  {REG_GPR, A_IDX,   "a"},
  {REG_GPR, XL_IDX,  "xl"},
  {REG_GPR, XH_IDX,  "xh"},
  {REG_GPR, YL_IDX,  "yl"},
  {REG_GPR, YH_IDX,  "yh"},
  {REG_CND, C_IDX,   "c"},
  {REG_GPR, X_IDX,   "x"},
  {REG_GPR, Y_IDX,   "y"},
  {0, SP_IDX,        "sp"},
};

/* Flags to turn on debugging code.
 */
enum
{
  D_ALLOC = 0,
};

/** Local static variables */
static struct
{
  set *stackSpil;
  int slocNum;
  int stackExtend;
  int dataExtend;
} _G;

#if 1
#define D(_a, _s)       if (_a)  { printf _s; fflush(stdout); }
#else
#define D(_a, _s)
#endif

/** noOverLap - will iterate through the list looking for over lap
 */
static int
noOverLap (set *itmpStack, symbol *fsym)
{
  symbol *sym;

  for (sym = setFirstItem (itmpStack); sym; sym = setNextItem (itmpStack))
    {
      if (bitVectBitValue (sym->clashes, fsym->key))
        return 0;
#if 0
      // if sym starts before (or on) our end point
      // and ends after (or on) our start point,
      // it is an overlap.
      if (sym->liveFrom <= fsym->liveTo && sym->liveTo >= fsym->liveFrom)
        {
          return 0;
        }
#endif
    }
  return 1;
}

/*-----------------------------------------------------------------*/
/* isFree - will return 1 if the a free spil location is found     */
/*-----------------------------------------------------------------*/
DEFSETFUNC (isFreeSTM8)
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
     the size can be accomodated  */
  if (sym->isFree && noOverLap (sym->usl.itmpStack, fsym) && getSize (sym->type) >= getSize (fsym->type))
    {
      *sloc = sym;
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* createStackSpil - create a location on the stack to spil        */
/*-----------------------------------------------------------------*/
static symbol *
createStackSpil (symbol * sym)
{
  symbol *sloc = NULL;
  struct dbuf_s dbuf;

  D (D_ALLOC, ("createStackSpil: for sym %p %s (old currFunc->stack %ld)\n", sym, sym->name, (long)(currFunc->stack)));

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "sloc%d", _G.slocNum++);
  sloc = newiTemp (dbuf_c_str (&dbuf));
  dbuf_destroy (&dbuf);

  /* set the type to the spilling symbol */
  sloc->type = copyLinkChain (sym->type);
  sloc->etype = getSpec (sloc->type);
  SPEC_SCLS (sloc->etype) = S_AUTO;
  SPEC_EXTR (sloc->etype) = 0;
  SPEC_STAT (sloc->etype) = 0;
  SPEC_VOLATILE (sloc->etype) = 0;

  allocLocal (sloc);

  sloc->isref = 1;              /* to prevent compiler warning */

  wassertl (currFunc, "Local variable used outside of function.");

  /* if it is on the stack then update the stack */
  if (IN_STACK (sloc->etype))
    {
      if (currFunc)
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
/* spillThis - spils a specific operand                            */
/*-----------------------------------------------------------------*/
void
stm8SpillThis (symbol *sym, bool force_spill)
{
  int i;

  D (D_ALLOC, ("stm8SpillThis: spilling %p (%s)\n", sym, sym->name));

  /* if this is rematerializable or has a spillLocation
     we are okay, else we need to create a spillLocation
     for it */
  if (!(sym->remat || sym->usl.spillLoc) || (sym->usl.spillLoc && !sym->usl.spillLoc->onStack)) // stm8 port currently only supports on-stack spill locations in code generation.
    createStackSpil (sym);

  /* mark it as spilt */
  sym->isspilt = sym->spillA = 1;

  if (force_spill)
    for (i = 0; i < sym->nRegs; i++)
      {
        if (sym->regs[i])
          sym->regs[i] = 0;
      }

  if (sym->usl.spillLoc && !sym->remat)
    {
      sym->usl.spillLoc->allocreq++;
    }
  return;
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
  for (sym = hTabFirstItem (liveRanges, &k); sym; sym = hTabNextItem (liveRanges, &k))
    {
      /* if used zero times then no registers needed. Exception: Variables larger than 4 bytes - these might need a spill location when they are return values */
      if ((sym->liveTo - sym->liveFrom) == 0 && getSize (sym->type) <= 4)
        continue;
      else if ((sym->liveTo - sym->liveFrom) == 0 && bitVectnBitsOn (sym->defs) <= 1)
        {
          iCode *dic = hTabItemWithKey (iCodehTab, bitVectFirstBit (sym->defs));
          if (!dic || dic->op != CALL && dic->op != PCALL)
            continue;
        }

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
                sym->type = aggrToPtr (sym->type, FALSE);
              continue;
            }

          /* if not then we require registers */
          D (D_ALLOC,
             ("regTypeNum: isagg %u nRegs %u type %p\n", IS_AGGREGATE (sym->type) || sym->isptr, sym->nRegs, sym->type));
          sym->nRegs =
            ((IS_AGGREGATE (sym->type)
              || sym->isptr) ? getSize (sym->type = aggrToPtr (sym->type, FALSE)) : getSize (sym->type));
          D (D_ALLOC, ("regTypeNum: setting nRegs of %s (%p) to %u\n", sym->name, sym, sym->nRegs));

          D (D_ALLOC, ("regTypeNum: setup to assign regs sym %p\n", sym));

          if (sym->nRegs > 8)
            {
              fprintf (stderr, "allocated more than 8 registers for type ");
              printTypeChain (sym->type, stderr);
              fprintf (stderr, "\n");
            }

          /* determine the type of register required */
          /* Always general purpose */
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


/** Register reduction for assignment.
 */
static int
packRegsForAssign (iCode *ic, eBBlock *ebp)
{
  iCode *dic, *sic;

  if (!IS_ITEMP (IC_RIGHT (ic)) || OP_SYMBOL (IC_RIGHT (ic))->isind || OP_LIVETO (IC_RIGHT (ic)) > ic->seq)
    return 0;
  
  /* Avoid having multiple named address spaces in one iCode. */
  if (IS_SYMOP (IC_RESULT (ic)) && SPEC_ADDRSPACE (OP_SYMBOL (IC_RESULT (ic))->etype))
    return 0;

  /* find the definition of iTempNN scanning backwards if we find a
     a use of the true symbol in before we find the definition then
     we cannot */
  for (dic = ic->prev; dic; dic = dic->prev)
    {
      /* PENDING: Don't pack across function calls. */
      if (dic->op == CALL || dic->op == PCALL)
        {
          dic = NULL;
          break;
        }

      if (SKIP_IC2 (dic))
        continue;

      if (dic->op == IFX)
        {
          if (IS_SYMOP (IC_COND (dic)) &&
              (IC_COND (dic)->key == IC_RESULT (ic)->key || IC_COND (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }
        }
      else
        {
          if (IS_TRUE_SYMOP (IC_RESULT (dic)) && IS_OP_VOLATILE (IC_RESULT (dic)))
            {
              dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_RESULT (dic)) && IC_RESULT (dic)->key == IC_RIGHT (ic)->key)
            {
              break;
            }

          if (IS_SYMOP (IC_RIGHT (dic)) &&
              (IC_RIGHT (dic)->key == IC_RESULT (ic)->key || IC_RIGHT (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_LEFT (dic)) &&
              (IC_LEFT (dic)->key == IC_RESULT (ic)->key || IC_LEFT (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_RESULT (dic)) && IC_RESULT (dic)->key == IC_RESULT (ic)->key)
            {
              dic = NULL;
              break;
            }
        }
    }

  if (!dic)
    return 0;                   /* did not find */

  /* if assignment then check that right is not a bit */
  if (ic->op == '=')
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

  /* For now eliminate 8-bit temporary variables only.
     The STM8 instructions operating directly on memory
     operands are 8-bit, so the most benefit is in 8-bit
     operations. On the other hand, supporting wider
     operations well in codegen is also more effort. */
  if (bitsForType (operandType (IC_RESULT (dic))) > 8 &&
    !((dic->op == LEFT_OP || dic->op == RIGHT_OP) && IS_OP_LITERAL (IC_RIGHT (dic)) && operandLitValue (IC_RIGHT (dic))  == 1 && // Can do wide shift by 1 in place.
      IS_SYMOP (IC_LEFT (dic)) && IS_SYMOP (IC_RESULT (ic)) && OP_SYMBOL (IC_LEFT (dic)) == OP_SYMBOL (IC_RESULT (ic))))
    return 0;

  /* if the result is on stack or iaccess then it must be
     the same as at least one of the operands */
  if (OP_SYMBOL (IC_RESULT (ic))->onStack || OP_SYMBOL (IC_RESULT (ic))->iaccess)
    {
      /* the operation has only one symbol
         operator then we can pack */
      if ((IC_LEFT (dic) && !IS_SYMOP (IC_LEFT (dic))) || (IC_RIGHT (dic) && !IS_SYMOP (IC_RIGHT (dic))))
        goto pack;

      if (!((IC_LEFT (dic) &&
             IC_RESULT (ic)->key == IC_LEFT (dic)->key) || (IC_RIGHT (dic) && IC_RESULT (ic)->key == IC_RIGHT (dic)->key)))
        return 0;
    }
pack:
  /* found the definition */

  /* delete from liverange table also
     delete from all the points in between and the new
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
  IC_RESULT (dic) = IC_RESULT (ic);

  if (IS_ITEMP (IC_RESULT (dic)) && OP_SYMBOL (IC_RESULT (dic))->liveFrom > dic->seq)
    {
      OP_SYMBOL (IC_RESULT (dic))->liveFrom = dic->seq;
    }

  remiCodeFromeBBlock (ebp, ic);
  // PENDING: Check vs mcs51
  bitVectUnSetBit (OP_SYMBOL (IC_RESULT (ic))->defs, ic->key);
  hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
  OP_DEFS (IC_RESULT (dic)) = bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
  return 1;
}

/** Will reduce some registers for single use.
 */
static int
packRegsForOneuse (iCode *ic, operand **opp, eBBlock *ebp)
{
  iCode *dic;

  operand *op = *opp;
//printf("packRegsForOneuse() at ic %d\n", ic->key);
  /* if returning a literal then do nothing */
  if (!IS_ITEMP (op))
    return 0;

  /* if rematerializable do nothing */
  if (OP_SYMBOL (op)->remat)
    return 0;

  /* this routine will mark the symbol as used in one
     instruction use only && if the definition is local
     (ie. within the basic block) && has only one definition  */
  if (bitVectnBitsOn (OP_USES (op)) != 1 || bitVectnBitsOn (OP_DEFS (op)) != 1)
    return 0;

  /* get the definition */
  if (!(dic = hTabItemWithKey (iCodehTab, bitVectFirstBit (OP_DEFS (op)))))
    return 0;
//printf("Found dic %d\n", dic->key);
  /* found the definition now check if it is local */
  if (dic->seq < ebp->fSeq || dic->seq > ebp->lSeq)
    return 0;                /* non-local */

  /* for now handle results from assignments from globals only */
  if (dic->op != '=' || !isOperandGlobal (IC_RIGHT (dic)))
    return 0;
  /* also make sure the intervenening instructions
     don't have any thing in far space */
  for (iCode *nic = dic->next; nic && nic != ic; nic = nic->next)
    {
      /* if there is an intervening function call then no */
      if (nic->op == CALL || nic->op == PCALL)
        return 0;

      if (nic->op == GET_VALUE_AT_ADDRESS || nic->op == SET_VALUE_AT_ADDRESS)
        return 0;

      /* if address of & the result is remat, then okay */
      if (nic->op == ADDRESS_OF && OP_SYMBOL (IC_RESULT (nic))->remat)
        continue;

      if (IS_OP_VOLATILE (IC_LEFT (nic)) ||
          IS_OP_VOLATILE (IC_RIGHT (nic)) ||
          isOperandGlobal (IC_RESULT (nic)))
        return 0;
    }

  /* Optimize out the assignment */
  *opp = operandFromOperand (IC_RIGHT(dic));
  (*opp)->isaddr = true;
  
  bitVectUnSetBit (OP_SYMBOL (op)->defs, dic->key);
  bitVectUnSetBit (OP_SYMBOL (op)->uses, ic->key);

  if (IS_ITEMP (IC_RESULT (dic)) && OP_SYMBOL (IC_RESULT (dic))->liveFrom > dic->seq)
    OP_SYMBOL (IC_RESULT (dic))->liveFrom = dic->seq;

  /* delete from liverange table also
     delete from all the points in between and the new
     one */
  for (iCode *nic = dic; nic != ic; nic = nic->next) 
    bitVectUnSetBit (nic->rlive, op->key);

  remiCodeFromeBBlock (ebp, dic);

  hTabDeleteItem (&iCodehTab, dic->key, ic, DELETE_ITEM, NULL);
  
  return 1;
}

/** Does some transformations to reduce register pressure.
 */
static void
packRegisters (eBBlock * ebp)
{
  iCode *ic;
  int change = 0;

  D (D_ALLOC, ("packRegisters: entered.\n"));

  for(;;)
    {
      change = 0;
      /* look for assignments of the form */
      /* iTempNN = TRueSym (someoperation) SomeOperand */
      /*       ....                       */
      /* TrueSym := iTempNN:1             */
      for (ic = ebp->sch; ic; ic = ic->next)
        {
          /* find assignment of the form TrueSym := iTempNN:1 */
          if (ic->op == '=')
            change += packRegsForAssign (ic, ebp);
        }
      if (!change)
        break;
    }

  for (ic = ebp->sch; ic; ic = ic->next)
    {
      D (D_ALLOC, ("packRegisters: looping on ic %p\n", ic));

      /* Safe: address of a true sym is always constant. */
      /* if this is an itemp & result of a address of a true sym
         then mark this as rematerialisable   */
      if (ic->op == ADDRESS_OF && 
        IS_ITEMP (IC_RESULT (ic)) && bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 && !IS_PARM (IC_RESULT (ic)) /* The receiving of the parameter is not accounted for in DEFS */ &&
        IS_TRUE_SYMOP (IC_LEFT (ic)))
        {
          OP_SYMBOL (IC_RESULT (ic))->remat = 1;
          OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
          OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
        }

      /* Safe: just propagates the remat flag */
      /* if straight assignment then carry remat flag if this is the
         only definition */
      if (ic->op == '=' && IS_SYMOP (IC_RIGHT (ic)) && OP_SYMBOL (IC_RIGHT (ic))->remat &&
        !isOperandGlobal (IC_RESULT (ic)) && bitVectnBitsOn (OP_SYMBOL (IC_RESULT (ic))->defs) == 1 && !IS_PARM (IC_RESULT (ic)) && /* The receiving of the paramter is not accounted for in DEFS */
        !OP_SYMBOL (IC_RESULT (ic))->addrtaken)
        {
          OP_SYMBOL (IC_RESULT (ic))->remat = OP_SYMBOL (IC_RIGHT (ic))->remat;
          OP_SYMBOL (IC_RESULT (ic))->rematiCode = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
        }

      /* if cast to a generic pointer & the pointer being
         cast is remat, then we can remat this cast as well */
      if (ic->op == CAST &&
        IS_SYMOP (IC_RIGHT (ic)) && OP_SYMBOL (IC_RIGHT (ic))->remat &&
        !isOperandGlobal (IC_RESULT (ic)) && bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 && !IS_PARM (IC_RESULT (ic)) && /* The receiving of the paramter is not accounted for in DEFS */
        !OP_SYMBOL (IC_RESULT (ic))->addrtaken)
        {
          sym_link *to_type = operandType (IC_LEFT (ic));
          sym_link *from_type = operandType (IC_RIGHT (ic));
          if (IS_GENPTR (to_type) && IS_PTR (from_type))
            {
              OP_SYMBOL (IC_RESULT (ic))->remat = 1;
              OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
              OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
            }
        }

      /* In some cases redundant moves can be eliminated */
      if (ic->op == GET_VALUE_AT_ADDRESS || ic->op == SET_VALUE_AT_ADDRESS ||
        ic->op == IFX && operandSize (IC_COND (ic)) == 1 ||
        ic->op == IPUSH && operandSize (IC_LEFT (ic)) == 1)
        packRegsForOneuse (ic, &(IC_LEFT (ic)), ebp);
    }

}

/**
  Mark variables for assignment by the register allocator.
 */
static void
serialRegMark (eBBlock ** ebbs, int count)
{
  int i;
  short int max_alloc_bytes = SHRT_MAX; // Byte limit. Set this to a low value to pass only few variables to the register allocator. This can be useful for debugging.

  stm8_call_stack_size = 2; // Saving of register to stack temporarily.

  D (D_ALLOC, ("serialRegMark for %s, currFunc->stack %d\n", currFunc->name, currFunc->stack));

  /* for all blocks */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath && (ebbs[i]->entryLabel != entryLabel && ebbs[i]->entryLabel != returnLabel))
        continue;

      /* for all instructions do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          if ((ic->op == CALL || ic->op == PCALL) && ic->parmBytes + 5 > stm8_call_stack_size)
            {
              sym_link *dtype = operandType (IC_LEFT (ic));
              sym_link *ftype = IS_FUNCPTR (dtype) ? dtype->next : dtype;

              /* 5 for saving all registers at call site + 2 for big return value */
              stm8_call_stack_size = ic->parmBytes + 5 + 2 * (getSize (ftype->next) > 4);
            }

          if (ic->op == IPOP)
            wassert (0);

          /* if result is present && is a true symbol */
          if (IC_RESULT (ic) && ic->op != IFX && IS_TRUE_SYMOP (IC_RESULT (ic)))
            OP_SYMBOL (IC_RESULT (ic))->allocreq++;

          /* some don't need registers, since there is no result. */
          if (SKIP_IC2 (ic) ||
              ic->op == JUMPTABLE || ic->op == IFX || ic->op == IPUSH || ic->op == IPOP || ic->op == SET_VALUE_AT_ADDRESS)
            continue;

          /* now we need to allocate registers only for the result */
          if (IC_RESULT (ic))
            {
              symbol *sym = OP_SYMBOL (IC_RESULT (ic));

              D (D_ALLOC, ("serialRegAssign: in loop on result %p %s\n", sym, sym->name));

              if (sym->isspilt && sym->usl.spillLoc) // todo: Remove once remat is supported!
                {
                  sym->usl.spillLoc->allocreq--;
                  sym->isspilt = FALSE;
                }

              /* Make sure any spill location is definately allocated */
              if (sym->isspilt && !sym->remat && sym->usl.spillLoc && !sym->usl.spillLoc->allocreq)
                sym->usl.spillLoc->allocreq++;

              /* if it does not need or is spilt
                 or is already marked for the new allocator
                 or will not live beyond this instructions */
              if (!sym->nRegs ||
                  sym->isspilt || sym->for_newralloc || sym->liveTo <= ic->seq && (sym->nRegs <= 4 || ic->op != CALL && ic->op != PCALL))
                {
                  D (D_ALLOC, ("serialRegMark: won't live long enough.\n"));
                  continue;
                }

              if (sym->usl.spillLoc && !sym->usl.spillLoc->_isparm) // I have no idea where these spill locations come from. Sometime two symbols even have the same spill location, whic tends to mess up stack allocation. THose that come from previous iterations in this loop would be okay, but those from outside are a problem.
                {
                  sym->usl.spillLoc = 0;
                  sym->isspilt = false;
                }

              if (sym->nRegs > 4 && ic->op == CALL) // To be allocated to stack due to the way long long return values are handled via a hidden pointer.
                {
                  sym->for_newralloc = 0;
                  stm8SpillThis (sym, TRUE);
                }
              else if (max_alloc_bytes >= sym->nRegs)
                {
                  sym->for_newralloc = 1;
                  max_alloc_bytes -= sym->nRegs;
                }
              else if (!sym->for_newralloc)
                {
                  stm8SpillThis (sym, TRUE);
                  printf ("Spilt %s due to byte limit.\n", sym->name);
                }
            }
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
verifyRegsAssigned (operand * op, iCode * ic)
{
  symbol *sym;
  int i;
  bool completely_in_regs;

  if (!op)
    return;
  if (!IS_ITEMP (op))
    return;

  sym = OP_SYMBOL (op);

  if (sym->regType == REG_CND)
    return;

  if (sym->isspilt && !sym->remat && sym->usl.spillLoc && !sym->usl.spillLoc->allocreq)
    sym->usl.spillLoc->allocreq++;

  if (sym->isspilt)
    return;

  for(i = 0, completely_in_regs = TRUE; i < sym->nRegs; i++)
    if (!sym->regs[i])
      completely_in_regs = FALSE;
  if (completely_in_regs)
    return;

  stm8SpillThis (sym, FALSE);
}

void
stm8RegFix (eBBlock ** ebbs, int count)
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

void stm8_init_asmops (void);

/*-----------------------------------------------------------------*/
/* assignRegisters - assigns registers to each live range as need  */
/*-----------------------------------------------------------------*/
void
stm8_assignRegisters (ebbIndex *ebbi)
{
  eBBlock **ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  iCode *ic;

  stm8_init_asmops();

  transformPointerSet (ebbs, count);

  /* change assignments this will remove some
     live ranges reducing some register pressure */
  for (int i = 0; i < count; i++)
    packRegisters (ebbs[i]);

  /* liveranges probably changed by register packing
     so we compute them again */
  recomputeLiveRanges (ebbs, count, FALSE);

  if (options.dump_i_code)
    dumpEbbsToFileExt (DUMP_PACK, ebbi);

  /* first determine for each live range the number of
     registers & the type of registers required for each */
  regTypeNum ();

  /* Mark variables for assignment by the new allocator */
  serialRegMark (ebbs, count);

  stm8_extend_stack = stm8_call_stack_size > 255;

  /* Invoke optimal register allocator */
  ic = stm8_ralloc2_cc (ebbi);

  /* redo the offsets for stacked automatic variables */
  if (currFunc && !stm8_extend_stack && currFunc->stack + stm8_call_stack_size > 255)
    {
      _G.slocNum = 0;

      /* Mark variables for assignment by the new allocator */
      serialRegMark (ebbs, count);

      stm8_extend_stack = TRUE;

      /* Invoke optimal register allocator */
      ic = stm8_ralloc2_cc (ebbi);
    }

  if (options.dump_i_code)
    {
      dumpEbbsToFileExt (DUMP_RASSGN, ebbi);
      dumpLiveRanges (DUMP_LRANGE, liveRanges);
    }

  genSTM8Code (ic);

  _G.slocNum = 0;
}

