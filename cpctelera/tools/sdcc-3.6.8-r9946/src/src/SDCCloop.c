/*-------------------------------------------------------------------------

  SDCCloop.c - source file for loop detection & optimizations

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

#include "common.h"
#include "newalloc.h"

DEFSETFUNC (isDefAlive);

STACK_DCL (regionStack, eBBlock *, MAX_NEST_LEVEL * 10)

/*-----------------------------------------------------------------*/
/* newInduction - creates a new induction variable                 */
/*-----------------------------------------------------------------*/
static induction *
newInduction (operand * sym, unsigned int op, long constVal, iCode * ic, operand * asym)
{
  induction *ip;

  ip = Safe_alloc (sizeof (induction));

  ip->sym = sym;
  ip->asym = asym;
  ip->op = op;
  ip->cval = constVal;
  ip->ic = ic;
//updateSpillLocation(ic,1);
  return ip;
}

/*-----------------------------------------------------------------*/
/* newRegion - allocate & returns a loop structure                 */
/*-----------------------------------------------------------------*/
static region *
newRegion ()
{
  region *lp;

  lp = Safe_alloc (sizeof (region));

  return lp;
}


#if 0
/*-----------------------------------------------------------------*/
/* pinduction - prints induction                                   */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (pinduction)
{
  induction *ip = item;
  iCodeTable *icTab;

  fprintf (stdout, "\t");
  printOperand (ip->sym, stdout);
  icTab = getTableEntry (ip->ic->op);
  icTab->iCodePrint (stdout, ip->ic, icTab->printName);
  fprintf (stdout, " %04d\n", (int) ip->cval);
  return 0;
}

/*-----------------------------------------------------------------*/
/* pregion - prints loop information                                */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (pregion)
{
  region *lp = item;

  printf ("================\n");
  printf (" loop with entry -- > ");
  printEntryLabel (lp->entry, ap);
  printf ("\n");
  printf (" loop body --> ");
  applyToSet (lp->regBlocks, printEntryLabel);
  printf ("\n");
  printf (" loop exits --> ");
  applyToSet (lp->exits, printEntryLabel);
  printf ("\n");
  return 0;
}
#endif

/*-----------------------------------------------------------------*/
/* backEdges - returns a list of back edges                        */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (backEdges)
{
  edge *ep = item;
  V_ARG (set **, bEdges);

  /* if this is a back edge ; to determine this we check */
  /* to see if the 'to' is in the dominator list of the  */
  /* 'from' if yes then this is a back edge              */
  if (bitVectBitValue (ep->from->domVect, ep->to->bbnum))
    {
      addSetHead (bEdges, ep);
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* intersectLoopSucc - returns intersection of loop Successors     */
/*-----------------------------------------------------------------*/
static bitVect *
intersectLoopSucc (set * lexits, eBBlock ** ebbs)
{
  bitVect *succVect = NULL;
  eBBlock *exit = setFirstItem (lexits);

  if (!exit)
    return NULL;

  succVect = bitVectCopy (exit->succVect);

  for (exit = setNextItem (lexits); exit; exit = setNextItem (lexits))
    {
      succVect = bitVectIntersect (succVect, exit->succVect);
    }

  return succVect;
}

/*-----------------------------------------------------------------*/
/* loopInsert will insert a block into the loop set                */
/*-----------------------------------------------------------------*/
static void
loopInsert (set ** regionSet, eBBlock * block)
{
  if (!isinSet (*regionSet, block))
    {
      addSetHead (regionSet, block);
      STACK_PUSH (regionStack, block);
    }
}

/*-----------------------------------------------------------------*/
/* insertIntoLoop - insert item into loop                          */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (insertIntoLoop)
{
  eBBlock *ebp = item;
  V_ARG (set **, regionSet);

  loopInsert (regionSet, ebp);
  return 0;
}

/*-----------------------------------------------------------------*/
/* isNotInBlocks - will return 1 if not is blocks                  */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (isNotInBlocks)
{
  eBBlock *ebp = item;
  V_ARG (set *, blocks);

  if (!isinSet (blocks, ebp))
    return 1;

  return 0;
}

#if 0
/*-----------------------------------------------------------------*/
/* hasIncomingDefs - has definitions coming into the loop. i.e.    */
/* check to see if the preheaders outDefs has any definitions      */
/*-----------------------------------------------------------------*/
static int
hasIncomingDefs (region * lreg, operand * op)
{
  eBBlock *preHdr = lreg->entry->preHeader;

  if (preHdr && bitVectBitsInCommon (preHdr->outDefs, OP_DEFS (op)))
    return 1;
  return 0;
}

/*-----------------------------------------------------------------*/
/* findLoopEndSeq - will return the sequence number of the last    */
/* iCode with the maximum dfNumber in the region                   */
/*-----------------------------------------------------------------*/
static int
findLoopEndSeq (region * lreg)
{
  eBBlock *block;
  eBBlock *lblock;

  for (block = lblock = setFirstItem (lreg->regBlocks); block; block = setNextItem (lreg->regBlocks))
    {
      if (block != lblock && block->lSeq > lblock->lSeq)
        lblock = block;
    }

  return lblock->lSeq;
}
#endif

/*-----------------------------------------------------------------*/
/* addToExitsMarkDepth - will add the the exitSet all blocks that  */
/* have exits, will also update the depth field in the blocks      */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (addToExitsMarkDepth)
{
  eBBlock *ebp = item;
  V_ARG (set *, loopBlocks);
  V_ARG (set **, exits);
  V_ARG (int, depth);
  V_ARG (region *, lr);

  /* mark the loop depth of this block */
  //if (!ebp->depth)
  if (ebp->depth < depth)
    ebp->depth = depth;

  /* NOTE: here we will update only the inner most loop
     that it is a part of */
  if (!ebp->partOfLoop)
    ebp->partOfLoop = lr;

  /* if any of the successors go out of the loop then */
  /* we add this one to the exits */
  if (applyToSet (ebp->succList, isNotInBlocks, loopBlocks))
    {
      addSetHead (exits, ebp);
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* createLoop - will create a set of region                        */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (createLoop)
{
  edge *ep = item;
  V_ARG (set **, allRegion);
  region *aloop = newRegion ();
  eBBlock *block;

  /* make sure regionStack is empty */
  while (!STACK_EMPTY (regionStack))
    STACK_POP (regionStack);

  /* add the entryBlock */
  addSet (&aloop->regBlocks, ep->to);
  loopInsert (&aloop->regBlocks, ep->from);

  while (!STACK_EMPTY (regionStack))
    {
      block = STACK_POP (regionStack);
      /* if block != entry */
      if (block != ep->to)
        applyToSet (block->predList, insertIntoLoop, &aloop->regBlocks);
    }

  aloop->entry = ep->to;

  /* now add it to the set */
  addSetHead (allRegion, aloop);
  return 0;
}

/*-----------------------------------------------------------------*/
/* dominatedBy - will return 1 if item is dominated by block       */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (dominatedBy)
{
  eBBlock *ebp = item;
  V_ARG (eBBlock *, block);

  return bitVectBitValue (ebp->domVect, block->bbnum);
}

/*-----------------------------------------------------------------*/
/* addDefInExprs - adds an expression into the inexpressions       */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (addDefInExprs)
{
  eBBlock *ebp = item;
  V_ARG (cseDef *, cdp);
  V_ARG (ebbIndex *, ebbi);

  addSetHead (&ebp->inExprs, cdp);
  cseBBlock (ebp, optimize.global_cse, ebbi);
  return 0;
}

/*-----------------------------------------------------------------*/
/* assignmentsToSym - for a set of blocks determine # time assigned */
/*-----------------------------------------------------------------*/
static int
assignmentsToSym (set * sset, operand * sym)
{
  eBBlock *ebp;
  int assigns = 0;
  set *blocks = setFromSet (sset);

  for (ebp = setFirstItem (blocks); ebp; ebp = setNextItem (blocks))
    {
      /* get all the definitions for this symbol
         in this block */
      bitVect *defs = bitVectIntersect (ebp->ldefs, OP_DEFS (sym));
      assigns += bitVectnBitsOn (defs);
      setToNull ((void *) &defs);
    }

  return assigns;
}


/*-----------------------------------------------------------------*/
/* isOperandInvariant - determines if an operand is an invariant   */
/*-----------------------------------------------------------------*/
static int
isOperandInvariant (operand * op, region * theLoop, set * lInvars)
{
  int opin = 0;
  /* operand is an invariant if it is a                */
  /*       a. constants .                              */
  /*       b. that have defintions reaching loop entry */
  /*       c. that are already defined as invariant    */
  /*       d. has no assignments in the loop           */
  if (op)
    {
      if (IS_OP_LITERAL (op))
        opin = 1;
      else if (IS_SYMOP (op) && OP_SYMBOL (op)->addrtaken)
        opin = 0;
      else if (ifDefSymIs (theLoop->entry->inExprs, op))
        opin = 1;
      else if (ifDefSymIs (lInvars, op))
        opin = 1;
      else if (IS_SYMOP (op) && !IS_OP_GLOBAL (op) && !IS_OP_VOLATILE (op) && assignmentsToSym (theLoop->regBlocks, op) == 0)
        opin = 1;
    }
  else
    opin++;

  return opin;
}

/*-----------------------------------------------------------------*/
/* pointerAssigned - will return 1 if pointer set found            */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (pointerAssigned)
{
  eBBlock *ebp = item;
  V_ARG (operand *, op);

  if (ebp->hasFcall)
    return 1;

  if (bitVectBitValue (ebp->ptrsSet, op->key))
    return 1;

  /* Unfortunately, one of the other pointer set operations  */
  /* may be using an alias of this operand, and the above    */
  /* test would miss it. To be thorough, some aliasing       */
  /* analysis should be done here. In the meantime, be       */
  /* conservative and assume any other pointer set operation */
  /* is dangerous                                            */
  if (!bitVectIsZero (ebp->ptrsSet))
    return 1;

  return 0;
}

/*-----------------------------------------------------------------*/
/* hasNonPtrUse - returns true if operand has non pointer usage    */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (hasNonPtrUse)
{
  eBBlock *ebp = item;
  V_ARG (operand *, op);
  iCode *ic = usedInRemaining (op, ebp->sch);

  if (ic && !POINTER_SET (ic) && !POINTER_GET (ic))
    return 1;

  return 0;

}

/*-----------------------------------------------------------------*/
/* loopInvariants - takes loop invariants out of region            */
/*-----------------------------------------------------------------*/
static int
loopInvariants (region * theLoop, ebbIndex * ebbi)
{
  eBBlock **ebbs = ebbi->dfOrder;
  int count = ebbi->count;
  eBBlock *lBlock;
  set *lInvars = NULL;

  int change = 0;
  int fCallsInBlock;

  /* if the preHeader does not exist then do nothing */
  /* or no exits then do nothing ( have to think about this situation */
  if (theLoop->entry->preHeader == NULL || theLoop->exits == NULL)
    return 0;

  /* we will do the elimination for those blocks       */
  /* in the loop that dominate all exits from the loop */
  for (lBlock = setFirstItem (theLoop->regBlocks); lBlock; lBlock = setNextItem (theLoop->regBlocks))
    {
      iCode *ic;
      int domsAllExits;
      int i;

      /* mark the dominates all exits flag */
      domsAllExits = (applyToSet (theLoop->exits, dominatedBy, lBlock) == elementsInSet (theLoop->exits));

      /* find out if we have a function call in this block */
      for (ic = lBlock->sch, fCallsInBlock = 0; ic; ic = ic->next)
        {
          if (SKIP_IC (ic))
            {
              fCallsInBlock++;
            }
        }

      /* now we go thru the instructions of this block and */
      /* collect those instructions with invariant operands */
      for (ic = lBlock->sch; ic; ic = ic->next)
        {
          int lin, rin;
          cseDef *ivar;

          /* TODO this is only needed if the call is between
             here and the definition, but I am too lazy to do that now */

          /* if there are function calls in this block */
          if (fCallsInBlock)
            {
              /* if this is a pointer get */
              if (POINTER_GET (ic))
                {
                  continue;
                }

              /* if this is an assignment from a global */
              if (ic->op == '=' && isOperandGlobal (IC_RIGHT (ic)))
                {
                  continue;
                }

              /* Bug 1717943,
               * if this is an assignment to a global */
              if (ic->op == '=' && isOperandGlobal (IC_RESULT (ic)))
                {
                  continue;
                }
            }

          if (SKIP_IC (ic) || POINTER_SET (ic) || ic->op == IFX)
            continue;

          /* iTemp assignment from a literal may be invariant, but it
             will needlessly increase register pressure if the
             iCode(s) that use this iTemp are not also invariant */
          if (ic->op == '=' && IS_ITEMP (IC_RESULT (ic)) && IS_OP_LITERAL (IC_RIGHT (ic)))
            continue;

          /* if result is volatile then skip */
          if (IC_RESULT (ic) && (isOperandVolatile (IC_RESULT (ic), TRUE) || IS_OP_PARM (IC_RESULT (ic))))
            continue;

          /* if result depends on a volatile then skip */
          if ((IC_LEFT (ic) && isOperandVolatile (IC_LEFT (ic), TRUE)) ||
              (IC_RIGHT (ic) && isOperandVolatile (IC_RIGHT (ic), TRUE)))
            continue;

          lin = rin = 0;

          /* special case */
          /* if address of then it is an invariant */
          if (ic->op == ADDRESS_OF && IS_SYMOP (IC_LEFT (ic)) && IS_AGGREGATE (operandType (IC_LEFT (ic))))
            {
              lin++;
            }
          else
            {
              /* check if left operand is an invariant */
              if ((lin = isOperandInvariant (IC_LEFT (ic), theLoop, lInvars)) &&
                  /* if this is a pointer get then make sure
                     that the pointer set does not exist in
                     any of the blocks */
                  POINTER_GET (ic) && applyToSet (theLoop->regBlocks, pointerAssigned, IC_LEFT (ic)))
                {
                  lin = 0;
                }
            }

          /* do the same for right */
          rin = isOperandInvariant (IC_RIGHT (ic), theLoop, lInvars);

          /* if this is a POINTER_GET then special case, make sure all
             usages within the loop are POINTER_GET any other usage
             would mean that this is not an invariant , since the pointer
             could then be passed as a parameter */
          if (POINTER_GET (ic) && applyToSet (theLoop->regBlocks, hasNonPtrUse, IC_LEFT (ic)))
            continue;


          /* if both the left & right are invariants : then check that */
          /* this definition exists in the out definition of all the   */
          /* blocks, this will ensure that this is not assigned any    */
          /* other value in the loop, and not used in this block       */
          /* prior to this definition which means only this definition */
          /* is used in this loop                                      */
          if (lin && rin && IC_RESULT (ic))
            {
              eBBlock *sBlock;
              set *lSet = setFromSet (theLoop->regBlocks);

              /* if this block does not dominate all exits */
              /* make sure this defintion is not used anywhere else */
              if (!domsAllExits)
                {
                  if (isOperandGlobal (IC_RESULT (ic)))
                    continue;
                  /* for successors for all exits */
                  for (sBlock = setFirstItem (theLoop->exits); sBlock; sBlock = setNextItem (theLoop->exits))
                    {
                      for (i = 0; i < count; ebbs[i++]->visited = 0);
                      lBlock->visited = 1;
                      if (applyToSet (sBlock->succList, isDefAlive, ic))
                        break;
                    }

                  /* we have found usage */
                  if (sBlock)
                    continue;
                }

              /* now make sure this is the only definition */
              for (sBlock = setFirstItem (lSet); sBlock; sBlock = setNextItem (lSet))
                {
                  iCode *ic2;
                  int used;
                  int defDominates;

                  /* if this is the block make sure the definition */
                  /* reaches the end of the block */
                  if (sBlock == lBlock)
                    {
                      if (!ifDiCodeIs (sBlock->outExprs, ic))
                        break;
                    }
                  else if (bitVectBitsInCommon (sBlock->defSet, OP_DEFS (IC_RESULT (ic))))
                    break;

                  /* Check that this definition is not assigned */
                  /* any other value in this block. Also check */
                  /* that any usage in the block is dominated by */
                  /* by this definition. */
                  defDominates = bitVectBitValue (sBlock->domVect, lBlock->bbnum);
                  used = 0;
                  for (ic2 = sBlock->sch; ic2; ic2 = ic2->next)
                    {
                      if (ic2->op == IFX)
                        {
                          if (isOperandEqual (IC_RESULT (ic), IC_COND (ic2)))
                            used = 1;
                        }
                      else if (ic2->op == JUMPTABLE)
                        {
                          if (isOperandEqual (IC_RESULT (ic), IC_JTCOND (ic2)))
                            used = 1;
                        }
                      else
                        {
                          if (IC_LEFT (ic2) && isOperandEqual (IC_RESULT (ic), IC_LEFT (ic2)))
                            used = 1;
                          if (IC_RIGHT (ic2) && isOperandEqual (IC_RESULT (ic), IC_RIGHT (ic2)))
                            used = 1;
                          if ((ic != ic2) && (isOperandEqual (IC_RESULT (ic), IC_RESULT (ic2))))
                            break;
                          /* If used before this definition, might not be invariant */
                          if ((ic == ic2) && used)
                            break;
                        }
                      if (used && !defDominates)
                        break;
                    }
                  if (ic2)      /* found another definition or a usage before the definition */
                    break;
                }

              if (sBlock)
                continue;       /* another definition present in the block */

              /* now check if it exists in the in of this block */
              /* if not then it was killed before this instruction */
              if (!bitVectBitValue (lBlock->inDefs, ic->key))
                continue;

              /* now we know it is a true invariant */
              /* remove it from the insts chain & put */
              /* in the invariant set                */

              OP_SYMBOL (IC_RESULT (ic))->isinvariant = 1;
              SPIL_LOC (IC_RESULT (ic)) = NULL;
              remiCodeFromeBBlock (lBlock, ic);

              /* maintain the data flow */
              /* this means removing from definition from the */
              /* defset of this block and adding it to the    */
              /* inexpressions of all blocks within the loop  */
              bitVectUnSetBit (lBlock->defSet, ic->key);
              bitVectUnSetBit (lBlock->ldefs, ic->key);
              ivar = newCseDef (IC_RESULT (ic), ic);
              applyToSet (theLoop->regBlocks, addDefInExprs, ivar, ebbi);
              addSet (&lInvars, ivar);
            }
        }
    }                           /* for all loop blocks */

  /* if we have some invariants then */
  if (lInvars)
    {
      eBBlock *preHdr = theLoop->entry->preHeader;
      iCode *icFirst = NULL, *icLast = NULL;
      cseDef *cdp;

      /* create an iCode chain from it */
      for (cdp = setFirstItem (lInvars); cdp; cdp = setNextItem (lInvars))
        {
          /* maintain data flow .. add it to the */
          /* ldefs defSet & outExprs of the preheader  */
          preHdr->defSet = bitVectSetBit (preHdr->defSet, cdp->diCode->key);
          preHdr->ldefs = bitVectSetBit (preHdr->ldefs, cdp->diCode->key);
          cdp->diCode->filename = preHdr->ech->filename;
          cdp->diCode->lineno = preHdr->ech->lineno;
          addSetHead (&preHdr->outExprs, cdp);

          if (!icFirst)
            icFirst = cdp->diCode;
          if (icLast)
            {
              icLast->next = cdp->diCode;
              cdp->diCode->prev = icLast;
              icLast = cdp->diCode;
            }
          else
            icLast = cdp->diCode;
          change++;
        }

      /* add the instruction chain to the end of the
         preheader for this loop, preheaders will always
         have atleast a label */
      preHdr->ech->next = icFirst;
      icFirst->prev = preHdr->ech;
      preHdr->ech = icLast;
      icLast->next = NULL;
    }
  return change;
}

/*-----------------------------------------------------------------*/
/* addressTaken - returns true if the symbol is found in the addrof */
/*-----------------------------------------------------------------*/
static int
addressTaken (set * sset, operand * sym)
{
  set *loop;
  eBBlock *ebp;
  set *loop2;

  for (loop = sset; loop; loop = loop->next)
    {
      ebp = loop->item;
      loop2 = ebp->addrOf;
      while (loop2)
        {
          if (isOperandEqual ((operand *) loop2->item, sym))
            return 1;
          loop2 = loop2->next;
        }
    }

  return 0;
}


/*-----------------------------------------------------------------*/
/* findInduction :- returns 1 & the item if the induction is found */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (findInduction)
{
  induction *ip = item;
  V_ARG (operand *, sym);
  V_ARG (induction **, ipp);

  if (isOperandEqual (ip->sym, sym))
    {
      *ipp = ip;
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* findDefInRegion - finds the definition within the region        */
/*-----------------------------------------------------------------*/
static iCode *
findDefInRegion (set * regBlocks, operand * defOp, eBBlock ** owner)
{
  eBBlock *lBlock;

  /* for all blocks in the region */
  for (lBlock = setFirstItem (regBlocks); lBlock; lBlock = setNextItem (regBlocks))
    {

      /* if a definition for this exists */
      if (bitVectBitsInCommon (lBlock->defSet, OP_DEFS (defOp)))
        {
          iCode *ic;

          /* go thru the instruction chain to find it */
          for (ic = lBlock->sch; ic; ic = ic->next)
            if (bitVectBitValue (OP_DEFS (defOp), ic->key))
              {
                if (owner)
                  *owner = lBlock;
                return ic;
              }
        }
    }

  return NULL;
}

/*-----------------------------------------------------------------*/
/* addPostLoopBlock - add a ebblock before the successors of the   */
/*                    loop exits                                   */
/*-----------------------------------------------------------------*/
static void
addPostLoopBlock (region * loopReg, ebbIndex * ebbi, iCode * ic)
{
  bitVect *loopSuccs;
  int i;

  /* if the number of exits is greater than one then
     we use another trick: we will create an intersection
     of succesors of the exits, then take those that are not
     part of the loop and have dfNumber greater loop entry (eblock),
     insert a new predecessor postLoopBlk before them and add
     a copy of ic in the new block. The postLoopBlk in between
     is necessary, because the old successors of the loop exits can
     be successors of other blocks too: see bug-136564. */

  /* loopSuccs now contains intersection
     of all the loops successors */
  loopSuccs = intersectLoopSucc (loopReg->exits, ebbi->dfOrder);
  if (!loopSuccs)
    return;

  for (i = 0; i < loopSuccs->size; i++)
    {
      eBBlock *eblock = NULL;
      eBBlock *postLoopBlk;
      iCode *newic;
      int j;

      if (!bitVectBitValue (loopSuccs, i))
        continue;

      /* Need to search for bbnum == i since ebbi->dfOrder is
         sorted by dfnum; a direct index won't do.  */
      for (j = 0; j < ebbi->count; j++)
        if (ebbi->dfOrder[j]->bbnum == i)
          {
            eblock = ebbi->dfOrder[j];
            break;
          }
      assert (eblock);

      /* if the successor does not belong to the loop
         and will be executed after the loop : then
         add a definition to the block */
      if (isinSet (loopReg->regBlocks, eblock))
        continue;
      if (eblock->dfnum <= loopReg->entry->dfnum)
        continue;

      /* look for an existing loopExitBlock */
      if (strncmp (LOOPEXITLBL, eblock->entryLabel->name, sizeof (LOOPEXITLBL) - 1) == 0)
        {
          /* reuse the existing one */
          postLoopBlk = eblock;
        }
      else
        {
          /* create and insert a new eBBlock.
             Damn, that's messy ... */
          int i;
          set *oldPredList;
          eBBlock *ebpi;

          postLoopBlk = neweBBlock ();

          /* create a loopExit-label */
          postLoopBlk->entryLabel = newiTempLoopHeaderLabel (0);

          /* increase bbnum for all blocks after
             (and including) eblock */
          for (i = 0; i < ebbi->count; i++)
            {
              if (ebbi->bbOrder[i]->bbnum >= eblock->bbnum)
                ++ebbi->bbOrder[i]->bbnum;
            }
          /* insert postLoopBlk before bbnum */
          postLoopBlk->bbnum = eblock->bbnum - 1;

          ++ebbi->count;

          /* these arrays need one more block, which ... */
          ebbi->bbOrder = Safe_realloc (ebbi->bbOrder, (ebbi->count + 1) * sizeof (eBBlock *));
          /* ... must be initialized with 0 */
          ebbi->bbOrder[ebbi->count] = NULL;
          /* move the blocks up ... */
          memmove (&ebbi->bbOrder[postLoopBlk->bbnum + 1],
                   &ebbi->bbOrder[postLoopBlk->bbnum], (ebbi->count - postLoopBlk->bbnum - 1) * sizeof (eBBlock *));
          /* ... and insert postLoopBlk */
          ebbi->bbOrder[postLoopBlk->bbnum] = postLoopBlk;

          /* just add postLoopBlk at the end of dfOrder,
             computeControlFlow() will compute the new dfnum */
          ebbi->dfOrder = Safe_realloc (ebbi->dfOrder, (ebbi->count + 1) * sizeof (eBBlock *));
          ebbi->dfOrder[ebbi->count] = NULL;
          ebbi->dfOrder[ebbi->count - 1] = postLoopBlk;

          /* copy loop information from eblock to postLoopBlk */
          if (eblock->partOfLoop)
            {
              postLoopBlk->partOfLoop = eblock->partOfLoop;
              /* add postLoopBlk to loop region */
              addSetHead (&postLoopBlk->partOfLoop->regBlocks, postLoopBlk);
            }

          /* go through loop exits and replace the old exit block eblock
             with the new postLoopBlk */
          for (ebpi = setFirstItem (loopReg->exits); ebpi; ebpi = setNextItem (loopReg->exits))
            {
              /* replace old label with new label */
              replaceLabel (ebpi, eblock->entryLabel, postLoopBlk->entryLabel);
            }

          /* now eblock is replaced by postLoopBlk.
             It's possible, that eblock has an immediate predecessor
             (with ebpi->bbnum + 1 == eblock->bbnum), which isn't part of the
             loop. This predecessor must stay predecessor of eblock, not of
             postLoopBlk. But now postLoopBlk is in between, therefore a GOTO
             from this predecessor to eblock must be appended.
             Example: bug-136564.c */

          /* take all predecessors and subtract the loop exits */
          oldPredList = subtractFromSet (eblock->predList, loopReg->exits, THROW_NONE);
          for (ebpi = setFirstItem (oldPredList); ebpi; ebpi = setNextItem (oldPredList))
            {
              /* Is it an immediate predecessor (without GOTO)?
                 All other predecessors end with a
                 GOTO, IF, IFX or JUMPTABLE: nothing to to do */
              if (ebpi->bbnum + 1 == postLoopBlk->bbnum)
                {
                  /* insert goto to old predecessor of eblock */
                  newic = newiCodeLabelGoto (GOTO, eblock->entryLabel);
                  addiCodeToeBBlock (ebpi, newic, NULL);
                  /* Make sure the GOTO has a target */
                  if (eblock->sch->op != LABEL)
                    {
                      newic = newiCodeLabelGoto (LABEL, eblock->entryLabel);
                      addiCodeToeBBlock (eblock, newic, eblock->sch);
                    }
                  break;        /* got it, only one is possible */
                }
            }

          /* set the label */
          postLoopBlk->sch = postLoopBlk->ech = newiCodeLabelGoto (LABEL, postLoopBlk->entryLabel);
        }

      /* create the definition in postLoopBlk */
      newic = newiCode ('=', NULL, operandFromOperand (IC_RIGHT (ic)));
      IC_RESULT (newic) = operandFromOperand (IC_RESULT (ic));
      /* maintain data flow */
      OP_DEFS (IC_RESULT (newic)) = bitVectSetBit (OP_DEFS (IC_RESULT (newic)), newic->key);
      OP_USES (IC_RIGHT (newic)) = bitVectSetBit (OP_USES (IC_RIGHT (newic)), newic->key);

      /* and add it */
      addiCodeToeBBlock (postLoopBlk, newic, NULL);
      postLoopBlk->sch->filename = postLoopBlk->ech->filename = eblock->sch->filename;
      postLoopBlk->sch->lineno = postLoopBlk->ech->lineno = eblock->sch->lineno;

      /* outDefs is needed by computeControlFlow(), anything
         else will be set up by computeControlFlow() */
      postLoopBlk->outDefs = bitVectSetBit (postLoopBlk->outDefs, newic->key);
    }                           /* for (i = 0; i < loopSuccs->size; i++) */

  /* the postLoopBlk and the induction significantly
     changed the control flow, recompute it */
  computeControlFlow (ebbi);
}

/*-----------------------------------------------------------------*/
/* basicInduction - finds the basic induction variables in a loop  */
/*-----------------------------------------------------------------*/
static set *
basicInduction (region * loopReg, ebbIndex * ebbi)
{
  eBBlock *lBlock;
  set *indVars = NULL;

  /* i.e. all assignments of the form a := a +/- const */
  /* for all blocks within the loop do */
  for (lBlock = setFirstItem (loopReg->regBlocks); lBlock; lBlock = setNextItem (loopReg->regBlocks))
    {

      iCode *ic, *dic;

      /* for all instructions in the blocks do */
      for (ic = lBlock->sch; ic; ic = ic->next)
        {
          operand *aSym;
          long litValue;
          induction *ip;
          iCode *indIc;
          eBBlock *owner = NULL;
          int nexits;
          sym_link *optype;

          /* To find an induction variable, we need to */
          /* find within the loop three iCodes in this */
          /* general form:                             */
          /*                                           */
          /* ddic: iTempB    := symbolVar              */
          /* dic:  iTempA    := iTempB + lit           */
          /*    or iTempA    := iTempB - lit           */
          /*    or iTempA    := lit + iTempB           */
          /* ic:   symbolVar := iTempA                 */
          /*                                           */
          /* (symbolVar may also be an iTemp if it is  */
          /*  register equivalent)                     */

          /* look for assignments of the form */
          /*   symbolVar := iTempNN */
          if (ic->op != '=')
            continue;

          if (!IS_SYMOP (IC_RESULT (ic)))
            continue;

          if (!IS_TRUE_SYMOP (IC_RESULT (ic)) && !OP_SYMBOL (IC_RESULT (ic))->isreqv)
            continue;

          if (isOperandGlobal (IC_RESULT (ic)))
            continue;

          if (!IS_ITEMP (IC_RIGHT (ic)))
            continue;

          /* if it has multiple assignments within the loop then skip */
          if (assignmentsToSym (loopReg->regBlocks, IC_RESULT (ic)) > 1)
            continue;

          /* if the address of this was taken inside the loop then continue */
          if (addressTaken (loopReg->regBlocks, IC_RESULT (ic)))
            continue;

          /* Only consider variables with integral type. */
          /* (2004/12/06 - EEP - ds390 fails regression tests unless */
          /* pointers are also considered for induction (due to some */
          /* register alloctaion bugs). Remove !IS_PTR clause when */
          /* that gets fixed) */
          optype = operandType (IC_RIGHT (ic));
          if (!IS_INTEGRAL (optype) && !IS_PTR (optype))
            continue;

          /* find the definition for the result in the block */
          if (!(dic = findDefInRegion (setFromSet (loopReg->regBlocks), IC_RIGHT (ic), &owner)))
            continue;

          /* if not +/- continue */
          if (dic->op != '+' && dic->op != '-')
            continue;

          /* make sure definition is of the form  a +/- c */
          if (!IS_OP_LITERAL (IC_LEFT (dic)) && !IS_OP_LITERAL (IC_RIGHT (dic)))
            continue;

          /* make sure the definition found is the only one */
          if (assignmentsToSym (loopReg->regBlocks, IC_RIGHT (ic)) > 1)
            continue;

          if (IS_OP_LITERAL (IC_RIGHT (dic)))
            {
              aSym = IC_LEFT (dic);
              litValue = (long) operandLitValue (IC_RIGHT (dic));
            }
          else
            {
              /* For minus, the literal must not be on the left side. */
              /* (Actually, this case could be handled, but it's probably */
              /* not worth the extra code) */
              if (dic->op == '-')
                continue;
              aSym = IC_RIGHT (dic);
              litValue = (long) operandLitValue (IC_LEFT (dic));
            }

          if (!isOperandEqual (IC_RESULT (ic), aSym) && !isOperandEqual (IC_RIGHT (ic), aSym))
            {
              iCode *ddic;
              /* find the definition for this and check */
              if (!(ddic = findDefInRegion (setFromSet (loopReg->regBlocks), aSym, &owner)))
                continue;

              if (ddic->op != '=')
                continue;

              if (!isOperandEqual (IC_RESULT (ddic), aSym) || !isOperandEqual (IC_RIGHT (ddic), IC_RESULT (ic)))
                continue;
            }

          /* if the right hand side has more than one usage then
             don't make it an induction (will have to think some more) */
          if (bitVectnBitsOn (OP_USES (IC_RIGHT (ic))) > 1)
            continue;

          /* if the definition is volatile then it cannot be
             an induction object */
          if (isOperandVolatile (IC_RIGHT (ic), FALSE) || isOperandVolatile (IC_RESULT (ic), FALSE))
            continue;

          /* whew !! that was a lot of work to find the definition */
          /* create an induction object */
          indIc = newiCode ('=', NULL, IC_RESULT (ic));
          indIc->filename = ic->filename;
          indIc->lineno = ic->lineno;
          IC_RESULT (indIc) = operandFromOperand (IC_RIGHT (ic));
          IC_RESULT (indIc)->isaddr = 0;
          OP_SYMBOL (IC_RESULT (indIc))->isind = 1;
          ip = newInduction (IC_RIGHT (ic), dic->op, litValue, indIc, NULL);

          /* replace the inducted variable by the iTemp */
          replaceSymBySym (loopReg->regBlocks, IC_RESULT (ic), IC_RIGHT (ic));
          /* ic should now be moved to the exit block(s) */

          nexits = elementsInSet (loopReg->exits);
          if (nexits == 0)
            {
              /* this is a endless loop without exits: there's
                 no need to restore the inducted variable */
              iCode *saveic = ic->prev;

              /* ic will be removed by killDeadCode() too,
                 but it's a nice to see a clean dumploop now. */
              remiCodeFromeBBlock (lBlock, ic);
              /* clear the definition */
              bitVectUnSetBit (lBlock->defSet, ic->key);
              ic = saveic;
            }
          else
            addPostLoopBlock (loopReg, ebbi, ic);
          addSet (&indVars, ip);
        }                       /* for ic */
    }                           /* end of all blocks for basic induction variables */

  return indVars;
}

/*-----------------------------------------------------------------*/
/* loopInduction - remove induction variables from a loop          */
/*-----------------------------------------------------------------*/
static int
loopInduction (region * loopReg, ebbIndex * ebbi)
{
  int change = 0;
  eBBlock *lBlock, *owner, *lastBlock = NULL;
  set *indVars = NULL;
  set *basicInd = NULL;

  if (loopReg->entry->preHeader == NULL)
    return 0;

  /* we first determine the basic Induction variables */
  basicInd = setFromSet (indVars = basicInduction (loopReg, ebbi));

  /* find other induction variables : by other we mean definitions of */
  /* the form x := y (* | / ) <constant> .. we will move this one to  */
  /* beginning of the loop and reduce strength i.e. replace with +/-  */
  /* these expensive expressions: OH! and y must be induction too     */
  for (lBlock = setFirstItem (loopReg->regBlocks), lastBlock = lBlock;
       lBlock && indVars; lBlock = setNextItem (loopReg->regBlocks))
    {
      iCode *ic, *indIc, *dic;
      induction *ip;

      /* last block is the one with the highest block
         number */
      if (lastBlock->bbnum < lBlock->bbnum)
        lastBlock = lBlock;

      for (ic = lBlock->sch; ic; ic = ic->next)
        {
          operand *aSym;
          operand *litSym;
          long litVal;

          /* consider only * & / */
          if (ic->op != '*' && ic->op != '/')
            continue;

          /* only consider variables with integral type */
          if (!IS_INTEGRAL (operandType (IC_RESULT (ic))))
            continue;

          /* if the result has more definitions then */
          if (assignmentsToSym (loopReg->regBlocks, IC_RESULT (ic)) > 1)
            continue;

          /* check if the operands are what we want */
          /* i.e. one of them an symbol the other a literal */
          if (!((IS_SYMOP (IC_LEFT (ic)) && IS_OP_LITERAL (IC_RIGHT (ic))) ||
                (IS_OP_LITERAL (IC_LEFT (ic)) && IS_SYMOP (IC_RIGHT (ic)))))
            continue;

          if (IS_SYMOP (IC_LEFT (ic)))
            {
              aSym = IC_LEFT (ic);
              litSym = IC_RIGHT (ic);
            }
          else
            {
              /* For division, the literal must not be on the left side */
              if (ic->op == '/')
                continue;
              aSym = IC_RIGHT (ic);
              litSym = IC_LEFT (ic);
            }
          litVal = (long) operandLitValue (litSym);

          ip = NULL;
          /* check if this is an induction variable */
          if (!applyToSetFTrue (basicInd, findInduction, aSym, &ip))
            continue;

          /* ask port for size not worth if native instruction
             exist for multiply & divide */
          if (port->hasNativeMulFor (ic, operandType (IC_LEFT (ic)), operandType (IC_RIGHT (ic))))
            continue;

          /* if this is a division then the remainder should be zero
             for it to be inducted */
          if (ic->op == '/' && (ip->cval % litVal))
            continue;

          /* create the iCode to be placed in the loop header */
          /* and create the induction object */

          /* create an instruction */
          /* this will be put on the loop header */
          indIc = newiCode (ic->op, operandFromOperand (aSym), operandFromOperand (litSym));
          indIc->filename = ic->filename;
          indIc->lineno = ic->lineno;
          IC_RESULT (indIc) = operandFromOperand (IC_RESULT (ic));
          OP_SYMBOL (IC_RESULT (indIc))->isind = 1;

          /* keep track of the inductions */
          litVal = (ic->op == '*' ? (litVal * ip->cval) : (ip->cval / litVal));

          addSet (&indVars, newInduction (IC_RESULT (ic), ip->op, litVal, indIc, NULL));

          /* Replace mul/div with assignment to self; killDeadCode() will */
          /* clean this up since we can't use remiCodeFromeBBlock() here. */
          ic->op = '=';
          IC_LEFT (ic) = NULL;
          IC_RIGHT (ic) = IC_RESULT (ic);

          /* Insert an update of the induction variable just before */
          /* the update of the basic induction variable. */
          indIc = newiCode (ip->op, operandFromOperand (IC_RESULT (ic)), operandFromLit (litVal));
          IC_RESULT (indIc) = operandFromOperand (IC_RESULT (ic));
          owner = NULL;
          dic = findDefInRegion (setFromSet (loopReg->regBlocks), aSym, &owner);
          assert (dic);
          addiCodeToeBBlock (owner, indIc, dic);
        }
    }

  /* if we have some induction variables then */
  if (indVars)
    {
      eBBlock *preHdr = loopReg->entry->preHeader;
      iCode *icFirst = NULL, *icLast = NULL;
      induction *ip;
      bitVect *indVect = NULL;

      /* create an iCode chain from it */
      for (ip = setFirstItem (indVars); ip; ip = setNextItem (indVars))
        {
          indVect = bitVectSetBit (indVect, ip->ic->key);
          ip->ic->filename = preHdr->ech->filename;
          ip->ic->lineno = preHdr->ech->lineno;
          if (!icFirst)
            icFirst = ip->ic;
          if (icLast)
            {
              icLast->next = ip->ic;
              ip->ic->prev = icLast;
              icLast = ip->ic;
            }
          else
            icLast = ip->ic;
          change++;
        }

      /* add the instruction chain to the end of the */
      /* preheader for this loop                     */
      preHdr->ech->next = icFirst;
      icFirst->prev = preHdr->ech;
      preHdr->ech = icLast;
      icLast->next = NULL;

      /* add the induction variable vector to the last
         block in the loop */
      lastBlock->isLastInLoop = 1;
      lastBlock->linds = bitVectUnion (lastBlock->linds, indVect);
    }

  setToNull ((void *) &indVars);
  return change;
}

/*-----------------------------------------------------------------*/
/* mergeRegions - will merge region with same entry point           */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (mergeRegions)
{
  region *theLoop = item;
  V_ARG (set *, allRegion);
  region *lp;

  /* if this has already been merged then do nothing */
  if (theLoop->merged)
    return 0;

  /* go thru all the region and check if any of them have the */
  /* entryPoint as the Loop                                  */
  for (lp = setFirstItem (allRegion); lp; lp = setNextItem (allRegion))
    {

      if (lp == theLoop)
        continue;

      if (lp->entry == theLoop->entry)
        {
          theLoop->regBlocks = unionSets (theLoop->regBlocks, lp->regBlocks, THROW_DEST);
          lp->merged = 1;
        }
    }

  return 1;
}

/*-----------------------------------------------------------------*/
/* ifMerged - return 1 if the merge flag is 1                      */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (ifMerged)
{
  region *lp = item;

  return lp->merged;
}

/*-----------------------------------------------------------------*/
/* mergeInnerLoops - will merge into body when entry is present    */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (mergeInnerLoops)
{
  region *theLoop = item;
  V_ARG (set *, allRegion);
  V_ARG (int *, maxDepth);
  region *lp;

  /* check if the entry point is present in the body of any */
  /* loop then put the body of this loop into the outer loop */
  for (lp = setFirstItem (allRegion); lp; lp = setNextItem (allRegion))
    {

      if (lp == theLoop)
        continue;

      if (isinSet (lp->regBlocks, theLoop->entry))
        {
          lp->containsLoops += theLoop->containsLoops + 1;
          if (lp->containsLoops > (*maxDepth))
            *maxDepth = lp->containsLoops;

          lp->regBlocks = unionSets (lp->regBlocks, theLoop->regBlocks, THROW_DEST);
        }
    }

  return 1;
}


/*-----------------------------------------------------------------*/
/* createLoopRegions - will detect and create a set of natural loops */
/*-----------------------------------------------------------------*/
hTab *
createLoopRegions (ebbIndex * ebbi)
{
  set *allRegion = NULL;        /* set of all loops */
  hTab *orderedLoops = NULL;
  set *bEdges = NULL;
  int maxDepth = 0;
  region *lp;

  /* get all the back edges in the graph */
  if (!applyToSet (graphEdges, backEdges, &bEdges))
    return 0;                   /* found no loops */

  /* for each of these back edges get the blocks that */
  /* constitute the loops                             */
  applyToSet (bEdges, createLoop, &allRegion);

  /* now we will create regions from these loops               */
  /* loops with the same entry points are considered to be the */
  /* same loop & they are merged. If the entry point of a loop */
  /* is found in the body of another loop then , all the blocks */
  /* in that loop are added to the loops containing the header */
  applyToSet (allRegion, mergeRegions, allRegion);

  /* delete those already merged */
  deleteItemIf (&allRegion, ifMerged);

  applyToSet (allRegion, mergeInnerLoops, allRegion, &maxDepth);
  maxDepth++;

  /* now create all the exits .. also */
  /* create an ordered set of loops   */
  /* i.e. we process loops in the inner to outer order */
  for (lp = setFirstItem (allRegion); lp; lp = setNextItem (allRegion))
    {
      applyToSet (lp->regBlocks, addToExitsMarkDepth, lp->regBlocks, &lp->exits, (maxDepth - lp->containsLoops), lp);

      hTabAddItem (&orderedLoops, lp->containsLoops, lp);

    }
  return orderedLoops;
}

/*-----------------------------------------------------------------*/
/* loopOptimizations - identify region & remove invariants & ind   */
/*-----------------------------------------------------------------*/
int
loopOptimizations (hTab * orderedLoops, ebbIndex * ebbi)
{
  region *lp;
  int change = 0;
  int k;

  /* if no loop optimizations requested */
  if (!optimize.loopInvariant && !optimize.loopInduction)
    return 0;

  /* now we process the loops inner to outer order */
  /* this is essential to maintain data flow information */
  /* the other choice is an ugly iteration for the depth */
  /* of the loops would hate that */
  for (lp = hTabFirstItem (orderedLoops, &k); lp; lp = hTabNextItem (orderedLoops, &k))
    {

      if (optimize.loopInvariant)
        change += loopInvariants (lp, ebbi);

      if (optimize.loopInduction)
        change += loopInduction (lp, ebbi);
    }

  return change;
}
