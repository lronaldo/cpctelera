/*-------------------------------------------------------------------------

  SDCCdflow.c - source file for data flow analysis and other utility
                routines related to data flow.

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

/*-----------------------------------------------------------------*/
/* ifKilledInBlock - will return 1 if the symbol is redefined in B */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifKilledInBlock)
{
  cseDef *cdp = item;
  V_ARG (eBBlock *, src);
  bitVect *outs;

  /* if this is a global variable and this block
     has a function call then delete it */
  if (isOperandGlobal (cdp->sym) && src->hasFcall)
    return 1;

  /* if this is pointer get then it will be killed
     if there is a pointer set for the same pointer
     in this block */
  if (POINTER_GET (cdp->diCode) &&
      bitVectBitValue (src->ptrsSet,
                       IC_LEFT (cdp->diCode)->key))
    return 1;

  /* if assignment to iTmep then if right is defined
     elsewhere kill this one */
  if (ASSIGNMENT (cdp->diCode) &&
      !POINTER_SET (cdp->diCode) &&
      IS_ITEMP (IC_RESULT (cdp->diCode)) &&
      IS_SYMOP (IC_RIGHT (cdp->diCode)) &&
      bitVectBitsInCommon (src->outDefs, OP_DEFS (IC_RIGHT (cdp->diCode))))
    return 1;

  /* if we find it in the defSet of this block */
  if (bitVectBitsInCommon (src->defSet, OP_DEFS (cdp->sym)))
    return 1;

  /* if in the outdef we find a definition other than this one */
  /* to do this we make a copy of the out definitions and turn */
  /* this one off then check if there are other definitions    */
  bitVectUnSetBit (outs = bitVectCopy (src->outDefs),
                   cdp->diCode->key);
  if (bitVectBitsInCommon (outs, OP_DEFS (cdp->sym)))
    {
      setToNull ((void *) &outs);
      return 1;
    }

  setToNull ((void *) &outs);

  /* if the operands of this one was changed in the block */
  /* then delete it */
  if (cdp->diCode &&
      ((IS_SYMOP (IC_LEFT (cdp->diCode)) &&
      bitVectBitsInCommon (src->defSet, OP_DEFS (IC_LEFT (cdp->diCode)))) ||
       (IS_SYMOP (IC_RIGHT (cdp->diCode)) &&
      bitVectBitsInCommon (src->defSet, OP_DEFS (IC_RIGHT (cdp->diCode))))))
    return 1;

  /* kill if cseBBlock() found a case we missed here */
  if (isinSetWith(src->killedExprs, cdp, isCseDefEqual))
    return 1;

  return 0;
}

/*-----------------------------------------------------------------*/
/* mergeInExprs - copy the in expression if it dominates           */
/*-----------------------------------------------------------------*/
DEFSETFUNC (mergeInExprs)
{
  eBBlock *ebp = item;
  V_ARG (eBBlock *, dest);
  V_ARG (int *, firstTime);

  dest->killedExprs = unionSets (dest->killedExprs, ebp->killedExprs, THROW_DEST);

  /* if in the dominator list then */
  if (bitVectBitValue (dest->domVect, ebp->bbnum) && dest != ebp)
    {
      /* if already present then intersect */
      if (!dest->inExprs && *firstTime)
        {
          dest->inExprs = setFromSet (ebp->outExprs);
          /* copy the pointer set from the dominator */
          dest->inPtrsSet = bitVectCopy (ebp->ptrsSet);
          dest->ndompset = bitVectCopy (ebp->ndompset);
        }
      else
        {
          dest->inExprs = intersectSets (dest->inExprs,
                                         ebp->outExprs,
                                         THROW_DEST);
          dest->inPtrsSet = bitVectUnion (dest->inPtrsSet, ebp->ptrsSet);
          dest->ndompset = bitVectUnion (dest->ndompset, ebp->ndompset);
        }
    }
  else
    {
      //if (dest != ebp)
      //  dest->inExprs = intersectSets (dest->inExprs, ebp->outExprs, THROW_DEST);

      /* delete only if killed in this block*/
      deleteItemIf (&dest->inExprs, ifKilledInBlock, ebp);
      /* union the ndompset with pointers set in this block */
      dest->ndompset = bitVectUnion (dest->ndompset, ebp->ptrsSet);
    }
  *firstTime = 0;

  return 0;
}


/*-----------------------------------------------------------------*/
/* mergeInDefs - merge in incoming definitions                     */
/*-----------------------------------------------------------------*/
DEFSETFUNC (mergeInDefs)
{
  eBBlock *ebp = item;
  V_ARG (eBBlock *, dest);
  V_ARG (int *, firstTime);

  /* the in definition is the union of the out */
  /* of all blocks that come to this block     */
  if (!dest->inDefs && *firstTime)
    dest->inDefs = bitVectCopy (ebp->outDefs);
  else
    dest->inDefs = bitVectUnion (dest->inDefs, ebp->outDefs);

  *firstTime = 0;

  return 0;
}


/*------------------------------------------------------------------*/
/* computeDataFlow - does computations for data flow accross blocks */
/*------------------------------------------------------------------*/
void
computeDataFlow (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->dfOrder;
  int count = ebbi->count;
  int i;
  int change;

  for (i = 0; i < count; i++)
    ebbs[i]->killedExprs = NULL;

  do
    {
      change = 0;

      /* for all blocks */
      for (i = 0; i < count; i++)
        {
          set *pred;
          set *oldOutExprs = NULL;
          set *oldKilledExprs = NULL;
          bitVect *oldOutDefs = NULL;
          int firstTime;
          eBBlock *pBlock;

          /* if this is the entry block then continue     */
          /* since entry block can never have any inExprs */
          if (ebbs[i]->noPath)
            continue;

          /* get blocks that can come to this block */
          pred = edgesTo (ebbs[i]);

          /* make a copy of the outExpressions and outDefs : to be */
          /* used for iteration   */
          if (optimize.global_cse)
            {
              oldOutExprs = setFromSet (ebbs[i]->outExprs);
              oldKilledExprs = setFromSet (ebbs[i]->killedExprs);
            }
          oldOutDefs = bitVectCopy (ebbs[i]->outDefs);
          setToNull ((void *) &ebbs[i]->inDefs);

          /* indefitions are easy just merge them by union */
          /* these are the definitions that can possibly   */
          /* reach this block                              */
          firstTime = 1;
          applyToSet (pred, mergeInDefs, ebbs[i], &firstTime);

          /* if none of the edges coming to this block */
          /* dominate this block then add the immediate dominator */
          /* of this block to the list of predecessors */
          for (pBlock = setFirstItem (pred); pBlock;
               pBlock = setNextItem (pred))
            {
              if (bitVectBitValue (ebbs[i]->domVect, pBlock->bbnum))
                break;
            }

          /* get the immediate dominator and put it there */
          if (!pBlock)
            {
              eBBlock *idom = immedDom (ebbi, ebbs[i]);
              if (idom)
                addSetHead (&pred, idom);
            }

          /* figure out the incoming expressions */
          /* this is a little more complex       */
          setToNull ((void *) &ebbs[i]->inExprs);
          if (optimize.global_cse)
            {
              firstTime = 1;
              applyToSet (pred, mergeInExprs, ebbs[i], &firstTime);
            }
          setToNull ((void *) &pred);

          /* do cse with computeOnly flag set to TRUE */
          /* this is by far the quickest way of computing */
          cseBBlock (ebbs[i], TRUE, ebbi);

          /* if it change we will need to iterate */
          if (optimize.global_cse)
            {
              change += !isSetsEqualWith (ebbs[i]->outExprs, oldOutExprs, isCseDefEqual);
              change += !isSetsEqualWith (ebbs[i]->killedExprs, oldKilledExprs, isCseDefEqual);
            }
          change += !bitVectEqual (ebbs[i]->outDefs, oldOutDefs);
        }
    }
  while (change);      /* iterate till no change */

  return;
}

/*-----------------------------------------------------------------*/
/* usedBetweenPoints - used between start & end                    */
/*-----------------------------------------------------------------*/
int
usedBetweenPoints (operand * op, iCode * start, iCode * end)
{
  iCode *lic = start;

  for (; lic != end; lic = lic->next)
    {
      /* if the operand is a parameter */
      /* then check for calls and return */
      /* true if there is a call       */
      if (IS_PARM (op) &&
          (lic->op == CALL || lic->op == PCALL))
        {
          value *args;
          if (lic->op == CALL)
            {
              args = FUNC_ARGS (OP_SYMBOL (IC_LEFT (lic))->type);
            }
          else
            {
              args = FUNC_ARGS (OP_SYMBOL (IC_LEFT (lic))->type->next);
            }
          if (isParameterToCall (args, op))
            return 1;
        }

      if (SKIP_IC2 (lic))
        continue;

      /* if ifx then check the condition */
      if (lic->op == IFX &&
          IC_COND (lic)->key == op->key)
        return 1;

      if (lic->op == JUMPTABLE &&
          IC_JTCOND (lic)->key == op->key)
        return 1;

      if (IC_RIGHT (lic) &&
          IC_RIGHT (lic)->key == op->key)
        return 1;

      if (IC_LEFT (lic) &&
          IC_LEFT (lic)->key == op->key)
        return 1;

      /* for a pointer assignment usage */
      if (POINTER_SET (lic) &&
          op->key == IC_RESULT (lic)->key)
        return 1;
      else if (IC_RESULT (lic) && op->key == IC_RESULT (lic)->key)
        return 0;
    }

  return 0;
}


/*------------------------------------------------------------------*/
/* usedInRemaining - returns point of usage for an operand if found */
/*------------------------------------------------------------------*/
iCode *
usedInRemaining (operand * op, iCode * ic)
{
  iCode *lic = ic;

  if (!IS_SYMOP (op))
    return 0;

  for (; lic; lic = lic->next)
    {
      /* if the operand is a parameter */
      /* then check for calls and return */
      /* true if there is a call       */
      /* if this is a global variable then
         return true */
      if (lic->op == CALL || lic->op == PCALL)
        {
          value *args;
          if (lic->op == CALL)
              args=FUNC_ARGS (operandType (IC_LEFT (lic)));
          else
            args=FUNC_ARGS (operandType (IC_LEFT (lic))->next);
          if ((IS_PARM (op) && isParameterToCall (args, op)) ||
              isOperandGlobal (op))
            return lic;
        }

      if (ic->op == SEND &&
          isOperandEqual (IC_LEFT (lic), op))
        return lic;

      if (SKIP_IC1 (lic))
        continue;

      /* if ifx then check the condition */
      if (lic->op == IFX && isOperandEqual (IC_COND (lic), op))
        return lic;

      if (lic->op == JUMPTABLE && isOperandEqual (IC_JTCOND (lic), op))
        return lic;

      if (IC_RIGHT (lic) && isOperandEqual (IC_RIGHT (lic), op))
        return lic;

      if (IC_LEFT (lic) &&
          isOperandEqual (IC_LEFT (lic), op))
        return lic;

      /* for a pointer assignment usage */
      if (POINTER_SET (lic) && isOperandEqual (op, IC_RESULT (lic)))
        return lic;
      else if (IC_RESULT (lic) && isOperandEqual (IC_RESULT (lic), op))
        return NULL;
    }

  return NULL;
}


/*-------------------------------------------------------------------*/
/* isDefAlive - will return true if definiton reaches a block & used */
/*-------------------------------------------------------------------*/
DEFSETFUNC (isDefAlive)
{
  eBBlock *ebp = item;
  V_ARG (iCode *, diCode);

  if (ebp->visited)
    return 0;

  ebp->visited = 1;

  /* if this definition is used in the block */
  if (bitVectBitValue (ebp->usesDefs, diCode->key))
    return 1;

  return applyToSet (ebp->succList, isDefAlive, diCode);
}
