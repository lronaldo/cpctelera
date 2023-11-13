/*-------------------------------------------------------------------------

  SDCClrange.c - source file for live range computations

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
#include "limits.h"

int iCodeSeq = 0;
hTab *liveRanges = NULL;
hTab *iCodehTab = NULL;
hTab *iCodeSeqhTab = NULL;

/* all symbols, for which the previous definition is searched
   and warning is emitted if there's none. */
#define IS_AUTOSYM(op) (IS_ITEMP(op) || \
                        (IS_SYMOP(op) && IS_AUTO(OP_SYMBOL (op)) && !IS_PARM(op)))

/*-----------------------------------------------------------------*/
/* hashiCodeKeys - add all iCodes to the hash table                */
/*-----------------------------------------------------------------*/
void
hashiCodeKeys (eBBlock ** ebbs, int count)
{
  int i;

  for (i = 0; i < count; i++)
    {
      iCode *ic;
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        hTabAddItem (&iCodehTab, ic->key, ic);
    }
}

/*-----------------------------------------------------------------*/
/* sequenceiCode - creates a sequence number for the iCode & add   */
/*-----------------------------------------------------------------*/
static void
sequenceiCode (eBBlock ** ebbs, int count)
{
  int i;

  for (i = 0; i < count; i++)
    {

      iCode *ic;
      ebbs[i]->fSeq = iCodeSeq + 1;
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
	{
	  ic->seq = ++iCodeSeq;
	  ic->depth = ebbs[i]->depth;
	  //hTabAddItem (&iCodehTab, ic->key, ic);
	  hTabAddItem (&iCodeSeqhTab, ic->seq, ic);
	}
      ebbs[i]->lSeq = iCodeSeq;
    }
}

/*-----------------------------------------------------------------*/
/* setFromRange - sets the from range of a given operand           */
/*-----------------------------------------------------------------*/
#if 0
static void
setFromRange (operand * op, int from)
{
  /* only for compiler defined temporaries */
  if (!IS_ITEMP (op))
    return;

  hTabAddItemIfNotP (&liveRanges, op->key, OP_SYMBOL (op));

  if (op->isaddr)
    OP_SYMBOL (op)->isptr = 1;

  if (!OP_LIVEFROM (op) ||
      OP_LIVEFROM (op) > from)
    OP_LIVEFROM (op) = from;
}
#endif

/*-----------------------------------------------------------------*/
/* setToRange - set the range to for an operand                    */
/*-----------------------------------------------------------------*/
void
setToRange (operand * op, int to, bool check)
{
  /* only for compiler defined temps */
  if (!IS_ITEMP (op))
    return;

  OP_SYMBOL (op)->key = op->key;
  hTabAddItemIfNotP (&liveRanges, op->key, OP_SYMBOL (op));

  if (op->isaddr)
    OP_SYMBOL (op)->isptr = 1;

  if (check)
    if (!OP_LIVETO (op))
      OP_LIVETO (op) = to;
    else;
  else
    OP_LIVETO (op) = to;
}

/*-----------------------------------------------------------------*/
/* setFromRange - sets the from range of a given operand           */
/*-----------------------------------------------------------------*/
static void
setLiveFrom (symbol * sym, int from)
{
  if (!sym->liveFrom || sym->liveFrom > from)
    sym->liveFrom = from;
}

/*-----------------------------------------------------------------*/
/* setToRange - set the range to for an operand                    */
/*-----------------------------------------------------------------*/
static void
setLiveTo (symbol *sym, int to)
{
  if (!sym->liveTo || sym->liveTo < to)
    sym->liveTo = to;
}

/*-----------------------------------------------------------------*/
/* markLiveRanges - for each operand mark the liveFrom & liveTo    */
/*-----------------------------------------------------------------*/
static void
markLiveRanges (eBBlock **ebbs, int count)
{
  int i, key;
  symbol *sym;

  for (i = 0; i < count; i++)
    {
      iCode *ic;

      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
	  if (ic->op == CALL || ic->op == PCALL)
	    if (bitVectIsZero (OP_SYMBOL (IC_RESULT (ic))->uses))
              bitVectUnSetBit (ebbs[i]->defSet, ic->key);

	  /* for all iTemps alive at this iCode */
	  for (key = 1; key < ic->rlive->size; key++)
	    {
	      if (!bitVectBitValue(ic->rlive, key))
	        continue;

	      sym = hTabItemWithKey(liveRanges, key);
	      setLiveTo(sym, ic->seq);
	      setLiveFrom(sym, ic->seq);
	    }

	}
    }
}

/*-----------------------------------------------------------------*/
/* markAlive - marks the operand as alive between sic and eic      */
/*-----------------------------------------------------------------*/
static void
markAlive (iCode * sic, iCode * eic, int key)
{
  iCode *dic;

  for (dic = sic; dic != eic->next; dic = dic->next)
    {
      dic->rlive = bitVectSetBit (dic->rlive, key);
    }
}

/*-----------------------------------------------------------------*/
/* findNextUseSym - finds the next use of the symbol and marks it  */
/*                  alive in between                               */
/*                  Return 0 iff there is no next use.             */
/*-----------------------------------------------------------------*/
static int
findNextUseSym (eBBlock *ebp, iCode *ic, symbol * sym)
{
  int retval = 0;
  iCode *uic;
  eBBlock *succ;

  hTabAddItemIfNotP (&liveRanges, sym->key, sym);

  if (!ic)
    goto check_successors;

  /* if we check a complete block and the symbol */
  /* is alive at the beginning of the block */
  /* and not defined in the first instructions */
  /* then a next use exists (return 1) */
  if ((ic == ebp->sch) && bitVectBitValue(ic->rlive, sym->key))
    {
      /* check if the first instruction is a def of this op */
      if (ic->op == JUMPTABLE || ic->op == IFX || SKIP_IC2(ic))
        return 1;

      if (IS_ITEMP(IC_RESULT(ic)))
        if (IC_RESULT(ic)->key == sym->key)
          return 0;

      return 1;
    }

  if (ebp->visited)
    return 0;

  if (ic == ebp->sch)
    ebp->visited = 1;

  /* for all remaining instructions in current block */
  for (uic = ic; uic; uic = uic->next)
    {

      if (SKIP_IC2(uic))
        continue;

      if (uic->op == JUMPTABLE)
        {
          if (IS_ITEMP(IC_JTCOND(uic)) && IC_JTCOND(uic)->key == sym->key)
            {
	      markAlive(ic, uic, sym->key);
	      return 1;
	    }
	   continue;
	}

      if (uic->op == IFX)
        {
          if (IS_ITEMP(IC_COND(uic)) && IC_COND(uic)->key == sym->key)
            {
	      markAlive(ic, uic, sym->key);
	      return 1;
	    }
	   continue;
	}

      if (IS_ITEMP (IC_LEFT (uic)))
        if (IC_LEFT (uic)->key == sym->key)
          {
	    markAlive(ic, uic, sym->key);
	    return 1;
	  }

      if (IS_ITEMP (IC_RIGHT (uic)))
        if (IC_RIGHT (uic)->key == sym->key)
	  {
	    markAlive (ic, uic, sym->key);
	    return 1;
	  }

      if (IS_ITEMP (IC_RESULT (uic)))
        if (IC_RESULT (uic)->key == sym->key)
	  {
	    if (POINTER_SET (uic))
	      {
	        markAlive (ic, uic, sym->key);
                return 1;
	      }
	    else
	      return 0;
	  }

    }

  /* check all successors */
check_successors:

  succ = setFirstItem (ebp->succList);
  for (; succ; succ = setNextItem (ebp->succList))
    {
      retval += findNextUseSym (succ, succ->sch, sym);
    }

  if (retval)
    {
      if (ic) markAlive (ic, ebp->ech, sym->key);
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* findNextUse - finds the next use of the operand and marks it    */
/*               alive in between                                  */
/*               Return 0 iff there is no next use.                */
/*-----------------------------------------------------------------*/
static int
findNextUse (eBBlock *ebp, iCode *ic, operand *op)
{
  if (op->isaddr)
    OP_SYMBOL (op)->isptr = 1;

  OP_SYMBOL (op)->key = op->key;

  return findNextUseSym (ebp, ic, OP_SYMBOL(op));
}

/*-----------------------------------------------------------------*/
/* unvisitBlocks - clears visited in all blocks                    */
/*-----------------------------------------------------------------*/
static void
unvisitBlocks (eBBlock ** ebbs, int count)
{
  int i;

  for (i = 0; i < count; i++)
    ebbs[i]->visited = 0;
}

/*------------------------------------------------------------------*/
/* markWholeLoop - mark the symbol 'key' alive in all blocks        */
/*                 included by the outermost loop                   */
/*------------------------------------------------------------------*/
static void
markWholeLoop (eBBlock *ebp, int key)
{
  eBBlock *ebpi;

  /* avoid endless loops */
  ebp->visited = 1;

  /* recurse through all predecessors */
  for (ebpi = setFirstItem (ebp->predList);
       ebpi;
       ebpi = setNextItem (ebp->predList))
    {
      if (ebpi->visited)
        continue;
      /* is the predecessor still in the loop? */
      if (ebpi->depth == 0)
        continue;
      markWholeLoop (ebpi, key);
    }

  /* recurse through all successors */
  for (ebpi = setFirstItem (ebp->succList);
       ebpi;
       ebpi = setNextItem (ebp->succList))
    {
      if (ebpi->visited)
        continue;
      if (ebpi->depth == 0)
        continue;
      markWholeLoop (ebpi, key);
    }

  markAlive (ebp->sch, ebp->ech, key);
}

/*------------------------------------------------------------------*/
/* findPrevUseSym - search for a previous definition of a symbol in */
/*                  - the previous icodes                           */
/*                  - all branches of predecessors                  */
/*------------------------------------------------------------------*/
static bool
findPrevUseSym  (eBBlock *ebp, iCode *ic, symbol * sym)
{
  eBBlock * pred;
  iCode * uic;

  if (ebp->visited)
    {
     /* already visited: this branch must have been succesfull, */
     /* because otherwise the search would have been aborted. */
      return TRUE;
    }
  ebp->visited = 1;

  /* search backward in the current block */
  for (uic = ic; uic; uic = uic->prev)
    {
      if (!POINTER_SET (uic) && IS_AUTOSYM (IC_RESULT (uic)))
        {
          if (IC_RESULT (uic)->key == sym->key)
            {
              /* Ok, found a definition */
              return TRUE;
            }
        }
      /* address taken from symbol? */
      if (uic->op == ADDRESS_OF && IS_AUTOSYM (IC_LEFT (uic)))
        {
          if (IC_LEFT (uic)->key == sym->key)
            {
              /* Ok, found a definition */
              return TRUE;
            }
        }
    }

  /* There's no definition in this bblock, */
  /* let's have a look at all predecessors. */
  pred = setFirstItem (ebp->predList);
  if (!pred)
    {
      /* no more predecessors and nothing found yet :-( */
      return FALSE;
    }
  for (; pred; pred = setNextItem (ebp->predList))
    {
      /* recurse into all predecessors */
      if (!findPrevUseSym (pred, pred->ech, sym))
        {
          /* found nothing: abort */
          return FALSE;
        }
    }

  /* Success! Went through all branches with no abort: */
  /* all branches end with a definition */
  return TRUE;
}

/*------------------------------------------------------------------*/
/* findPrevUse - search for a previous definition of an operand     */
/*                  If there's no definition let's:                 */
/*                  - emit a warning                                */
/*                  - fix the life range, if the symbol is used in  */
/*                    a loop                                        */
/*------------------------------------------------------------------*/
static int
findPrevUse (eBBlock *ebp, iCode *ic, operand *op,
             eBBlock ** ebbs, int count,
             bool emitWarnings)
{
  int change=0;

  unvisitBlocks (ebbs, count);

  if (op->isaddr)
    OP_SYMBOL (op)->isptr = 1;
  OP_SYMBOL (op)->key = op->key;

  /* There must be a definition in each branch of predecessors */
  if (!findPrevUseSym (ebp, ic->prev, OP_SYMBOL(op)))
    {
      /* computeLiveRanges() is called at least twice */
      if (emitWarnings)
        {
          if (IS_ITEMP (op))
            {
              if (OP_SYMBOL (op)->prereqv)
                {
                  iCode *newic, *ip;
                  value *val;
                  bitVect * dom = NULL;
                  bitVect * used;
                  int i, blocknum;
                  werrorfl (ic->filename, ic->lineno, W_LOCAL_NOINIT,
                            OP_SYMBOL (op)->prereqv->name);

                  /* iTemps must have a valid initial value, otherwise */
                  /* downstream algorithms will have problems. If      */
                  /* there's a problem with the user program such that */
                  /* something was left undefined, add an initializer  */
                  /* to the last common dominator before defs/uses.    */
                  /* First, find the common dominators of all defs/uses */
                  unvisitBlocks (ebbs, count);
                  used = newBitVect (count);
                  for (i=0; i<iCodeKey; i++)
                    {
                      if (bitVectBitValue (OP_USES (op), i) ||
                          bitVectBitValue (OP_DEFS (op), i))
                        {
                          bitVect * blockdomVect;
                          iCode * usedic;
                          usedic = hTabItemWithKey (iCodehTab, i);
                          if (!usedic)
                            continue;
                          if (ebbs[usedic->eBBlockNum]->visited)
                            continue;
                          ebbs[usedic->eBBlockNum]->visited = 1;
                          if (bitVectBitValue (OP_USES (op), i))
                            used = bitVectSetBit (used, usedic->eBBlockNum);
                          blockdomVect = ebbs[usedic->eBBlockNum]->domVect;
                          if (dom)
                            dom = bitVectInplaceIntersect (dom, blockdomVect);
                          else
                            dom = bitVectCopy (blockdomVect);
                        }
                    }
                  /* Find the common dominator with highest block num */
                  blocknum = 0;
                  for (i=0; i<dom->size; i++)
                    if (bitVectBitValue (dom, i) && !ebbs[i]->partOfLoop)
                      blocknum = i;
                  /* If there was a use in this block, set the insertion */
                  /* point near the beginning of the block, otherwise */
                  /* near the end */
                  if (bitVectBitValue (used, blocknum))
                    {
                      ip = ebbs[blocknum]->sch;
                      while (ip && (ip->op == LABEL || ip->op == FUNCTION || ip->op == RECEIVE))
                        ip = ip->next;
                    }
                  else
                    ip = NULL;
                  /* Finally, create initializer and insert it*/
                  val = valCastLiteral (operandType (op), 0.0, 0);
                  newic = newiCode ('=', NULL, operandFromValue (val));
                  IC_RESULT (newic) = operandFromOperand (op);
                  IC_RESULT (newic)->isaddr = 0;
                  OP_DEFS (IC_RESULT (newic)) = OP_DEFS (op) = bitVectSetBit (OP_DEFS (op), newic->key);
                  addiCodeToeBBlock (ebbs[blocknum], newic, ip);
                  newic->eBBlockNum = blocknum;
                  if (!ip && newic->prev)
                    {
                      newic->filename = newic->prev->filename;
                      newic->lineno = newic->prev->lineno;
                    }
                  ebbs[blocknum]->defSet = bitVectSetBit (ebbs[blocknum]->defSet, newic->key);
                  freeBitVect (used);
                  freeBitVect (dom);
                  change++;
                }
            }
          else
            {
              werrorfl (ic->filename, ic->lineno, W_LOCAL_NOINIT,
                        OP_SYMBOL (op)->name);
              OP_SYMBOL (op)->allocreq=1;
              OP_SYMBOL (op)->addrtaken=1; /* just to force allocation */
            }
        }
      /* is this block part of a loop? */
      if (IS_ITEMP (op) && ebp->depth != 0)
        {
          /* extend the life range to the outermost loop */
          unvisitBlocks(ebbs, count);
          markWholeLoop (ebp, op->key);
        }
    }
  return change;
}

/*-----------------------------------------------------------------*/
/* incUsed - increment a symbol's usage count                      */
/*-----------------------------------------------------------------*/
static void
incUsed (iCode *ic, operand *op)
{
  if (ic->depth)
    OP_SYMBOL (op)->used += (((unsigned int) 1 << ic->depth) + 1);
  else
    OP_SYMBOL (op)->used += 1;
}

/*-----------------------------------------------------------------*/
/* rliveClear - clears the rlive bitVectors                        */
/*-----------------------------------------------------------------*/
static void
rliveClear (eBBlock **ebbs, int count)
{
  int i;

  /* for all blocks do */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      /* for all instructions in this block do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
	      freeBitVect (ic->rlive);
	      ic->rlive = NULL;
	    }
    }
}

/*-----------------------------------------------------------------*/
/* rlivePoint - for each point compute the ranges that are alive   */
/* The live range is only stored for ITEMPs; the same code is used */
/* to find use of unitialized AUTOSYMs (an ITEMP is an AUTOSYM).   */
/* also, update funcUsesVolatile flag for current function         */
/*-----------------------------------------------------------------*/
static int
rlivePoint (eBBlock ** ebbs, int count, bool emitWarnings)
{
  int i, key;
  eBBlock *succ;
  bitVect *alive;
  int change = 0;

  bool uses_volatile = false;

  /* for all blocks do */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      /* for all instructions in this block do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          uses_volatile |= POINTER_GET (ic) && IS_VOLATILE (operandType (IC_LEFT(ic))->next) || IS_OP_VOLATILE (IC_LEFT(ic)) || IS_OP_VOLATILE (IC_RIGHT(ic));
          uses_volatile |= POINTER_SET (ic) && IS_VOLATILE (operandType (IC_RESULT(ic))->next) || IS_OP_VOLATILE (IC_RESULT(ic));

	  if (!ic->rlive)
	    ic->rlive = newBitVect (operandKey);

	  if (SKIP_IC2(ic))
	    continue;
      if (ebbs[i]->noPath) continue;
	  if (ic->op == JUMPTABLE && IS_SYMOP(IC_JTCOND(ic)))
	    {
	      incUsed (ic, IC_JTCOND(ic));

	      if (!IS_AUTOSYM(IC_JTCOND(ic)))
	        continue;

	      change += findPrevUse (ebbs[i], ic, IC_JTCOND(ic), ebbs, count, emitWarnings);
              if (IS_ITEMP(IC_JTCOND(ic)))
                {
                  unvisitBlocks(ebbs, count);
                  ic->rlive = bitVectSetBit (ic->rlive, IC_JTCOND(ic)->key);
                  findNextUse (ebbs[i], ic->next, IC_JTCOND(ic));
                }

	      continue;
	    }

	  if (ic->op == IFX && IS_SYMOP(IC_COND(ic)))
	    {
	      incUsed (ic, IC_COND(ic));

	      if (!IS_AUTOSYM(IC_COND(ic)))
	        continue;

	      change += findPrevUse (ebbs[i], ic, IC_COND(ic), ebbs, count, emitWarnings);
              if (IS_ITEMP(IC_COND(ic)))
                {
                  unvisitBlocks (ebbs, count);
                  ic->rlive = bitVectSetBit (ic->rlive, IC_COND(ic)->key);
                  findNextUse (ebbs[i], ic->next, IC_COND(ic));
                }

	      continue;
	    }

	  if (IS_SYMOP(IC_LEFT(ic)))
	    {
	      incUsed (ic, IC_LEFT(ic));
	      if (IS_AUTOSYM(IC_LEFT(ic)) &&
	          ic->op != ADDRESS_OF)
	        {
	          change += findPrevUse (ebbs[i], ic, IC_LEFT(ic), ebbs, count, emitWarnings);
                  if (IS_ITEMP(IC_LEFT(ic)))
                    {
                      unvisitBlocks(ebbs, count);
                      ic->rlive = bitVectSetBit (ic->rlive, IC_LEFT(ic)->key);
                      findNextUse (ebbs[i], ic->next, IC_LEFT(ic));

                      /* if this is a send extend the LR to the call */
                      if (ic->op == SEND)
                        {
                          iCode *lic;
                          for (lic = ic; lic; lic = lic->next)
                            {
                              if (lic->op == CALL || lic->op == PCALL)
                                {
                                  markAlive (ic, lic->prev, IC_LEFT (ic)->key);
                                  break;
                                }
                            }
                        }
                    }
		}
	    }

	  if (IS_SYMOP(IC_RIGHT(ic)))
	    {
	      incUsed (ic, IC_RIGHT(ic));
              if (IS_AUTOSYM(IC_RIGHT(ic)))
	        {
	          change += findPrevUse (ebbs[i], ic, IC_RIGHT(ic), ebbs, count, emitWarnings);
                  if (IS_ITEMP(IC_RIGHT(ic)))
                    {
                      unvisitBlocks(ebbs, count);
                      ic->rlive = bitVectSetBit (ic->rlive, IC_RIGHT(ic)->key);
                      findNextUse (ebbs[i], ic->next, IC_RIGHT(ic));
                    }
		}
	    }

	  if (POINTER_SET(ic) && IS_SYMOP(IC_RESULT(ic)))
	    incUsed (ic, IC_RESULT(ic));

          if (IS_AUTOSYM(IC_RESULT(ic)))
	    {
	      if (POINTER_SET(ic))
	        {
	          change += findPrevUse (ebbs[i], ic, IC_RESULT(ic), ebbs, count, emitWarnings);
		}
              if (IS_ITEMP(IC_RESULT(ic)))
                {
                  unvisitBlocks(ebbs, count);
                  ic->rlive = bitVectSetBit (ic->rlive, IC_RESULT(ic)->key);
                  findNextUse (ebbs[i], ic->next, IC_RESULT(ic));
                  /* findNextUse sometimes returns 0 here, which means that ic is
                     dead code. Something should be done about this dead code since
                     e.g. register allocation suffers. */
                }
	    }

	  if (!POINTER_SET(ic) && IC_RESULT(ic))
	    ic->defKey = IC_RESULT(ic)->key;

	}

      /* check all symbols that are alive in the last instruction */
      /* but are not alive in all successors */

      succ = setFirstItem (ebbs[i]->succList);
      if (!succ)
        continue;

      alive = succ->sch->rlive;
      while ((succ = setNextItem (ebbs[i]->succList)))
        {
	  if (succ->sch)
            alive = bitVectIntersect (alive, succ->sch->rlive);
	}

      if (ebbs[i]->ech)
        alive = bitVectCplAnd ( bitVectCopy (ebbs[i]->ech->rlive), alive);

      if(!alive)
        continue;
      for (key = 1; key < alive->size; key++)
        {
	  if (!bitVectBitValue (alive, key))
	    continue;

	  unvisitBlocks(ebbs, count);
	  findNextUseSym (ebbs[i], NULL, hTabItemWithKey (liveRanges, key));
	}

    }

  if(currFunc)
    currFunc->funcUsesVolatile = uses_volatile;
  return change;
}

/*-----------------------------------------------------------------*/
/* computeClash - find out which live ranges collide with others   */
/*-----------------------------------------------------------------*/
static void
computeClash (eBBlock ** ebbs, int count)
{
  int i;

  /* for all blocks do */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      /* for every iCode do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
	{
	  symbol *sym1, *sym2;
	  int key1, key2;

	  /* for all iTemps alive at this iCode */
	  for (key1 = 1; key1 < ic->rlive->size; key1++)
	    {
	      if (!bitVectBitValue(ic->rlive, key1))
	        continue;

	      sym1 = hTabItemWithKey(liveRanges, key1);

	      if (!sym1->isitmp)
	        continue;

	      /* for all other iTemps alive at this iCode */
	      for (key2 = key1+1; key2 < ic->rlive->size; key2++)
	        {
		  if (!bitVectBitValue(ic->rlive, key2))
		    continue;

		  sym2 = hTabItemWithKey(liveRanges, key2);

		  if (!sym2->isitmp)
		    continue;

		  /* if the result and left or right is an iTemp */
		  /* than possibly the iTemps do not clash */
		  if ((ic->op != JUMPTABLE) && (ic->op != IFX) &&
		      IS_ITEMP(IC_RESULT(ic)) &&
		      (IS_ITEMP(IC_LEFT(ic)) || IS_ITEMP(IC_RIGHT(ic))))
		    {
		      if (OP_SYMBOL(IC_RESULT(ic))->key == key1
			  && sym1->liveFrom == ic->seq
			  && sym2->liveTo == ic->seq)
		        {
		          if (IS_SYMOP(IC_LEFT(ic)))
			    if (OP_SYMBOL(IC_LEFT(ic))->key == key2)
			      continue;
		          if (IS_SYMOP(IC_RIGHT(ic)))
			    if (OP_SYMBOL(IC_RIGHT(ic))->key == key2)
			      continue;
			}

		      if (OP_SYMBOL(IC_RESULT(ic))->key == key2
			  && sym2->liveFrom == ic->seq
			  && sym1->liveTo == ic->seq)
		        {
		          if (IS_SYMOP(IC_LEFT(ic)))
			    if (OP_SYMBOL(IC_LEFT(ic))->key == key1)
			      continue;
		          if (IS_SYMOP(IC_RIGHT(ic)))
			    if (OP_SYMBOL(IC_RIGHT(ic))->key == key1)
			      continue;
			}
		    }

		  /* the iTemps do clash. set the bits in clashes */
		  sym1->clashes = bitVectSetBit (sym1->clashes, key2);
		  sym2->clashes = bitVectSetBit (sym2->clashes, key1);

		  /* check if they share the same spill location */
		  /* what is this good for? */
	          if (SYM_SPIL_LOC(sym1) && SYM_SPIL_LOC(sym2) &&
		      SYM_SPIL_LOC(sym1) == SYM_SPIL_LOC(sym2))
		    {
		      if (sym1->reqv && !sym2->reqv) SYM_SPIL_LOC(sym2)=NULL;
		      else if (sym2->reqv && !sym1->reqv) SYM_SPIL_LOC(sym1)=NULL;
		      else if (sym1->used > sym2->used) SYM_SPIL_LOC(sym2)=NULL;
		      else SYM_SPIL_LOC(sym1)=NULL;
		    }
		}
	    }
	}
    }
}

/*-----------------------------------------------------------------*/
/* allDefsOutOfRange - all definitions are out of a range          */
/*-----------------------------------------------------------------*/
bool
allDefsOutOfRange (bitVect * defs, int fseq, int toseq)
{
  int i;

  if (!defs)
    return TRUE;

  for (i = 0; i < defs->size; i++)
    {
      iCode *ic;

      if (bitVectBitValue (defs, i) &&
	  (ic = hTabItemWithKey (iCodehTab, i)) &&
	  (ic->seq >= fseq && ic->seq <= toseq))
	return FALSE;

    }

  return TRUE;
}

/*-----------------------------------------------------------------*/
/* notUsedInBlock - not used in this block                         */
/*-----------------------------------------------------------------*/
int
notUsedInBlock (symbol * sym, eBBlock * ebp, iCode *ic)
{
  return (!bitVectBitsInCommon (sym->defs, ebp->usesDefs) &&
	  allDefsOutOfRange (sym->defs, ebp->fSeq, ebp->lSeq) &&
	  allDefsOutOfRange (sym->uses, ebp->fSeq, ebp->lSeq));
}

/*-----------------------------------------------------------------*/
/* adjustIChain - correct the sch and ech pointers                 */
/*-----------------------------------------------------------------*/
void
adjustIChain (eBBlock ** ebbs, int count)
{
  int i;

  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath)
        continue;

      ic = ebbs[i]->sch;

      /* is there any code for this eBBlock? (e.g. ROM assignment) */
      if(!ic)continue;

      while (ic->prev)
        ic = ic->prev;
      ebbs[i]->sch = ic;

      ic = ebbs[i]->ech;
      while (ic->next)
        ic = ic->next;
      ebbs[i]->ech = ic;
    }
}

/*-----------------------------------------------------------------*/
/* computeLiveRanges - computes the live ranges for variables      */
/*-----------------------------------------------------------------*/
void
computeLiveRanges (eBBlock **ebbs, int count, bool emitWarnings)
{
  int change;
  /* first look through all blocks and adjust the
     sch and ech pointers */
  adjustIChain (ebbs, count);

  /* sequence the code the live ranges are computed
     in terms of this sequence additionally the
     routine will also create a hashtable of instructions */
  do
    {
      iCodeSeq = 0;
      setToNull ((void *) &iCodehTab);
      iCodehTab = newHashTable (iCodeKey);
      hashiCodeKeys (ebbs, count);
      setToNull ((void *) &iCodeSeqhTab);
      iCodeSeqhTab = newHashTable (iCodeKey);
      sequenceiCode (ebbs, count);

      /* mark the ranges live for each point */
      setToNull ((void *) &liveRanges);
      change = rlivePoint (ebbs, count, emitWarnings);
      emitWarnings = FALSE;
    }
  while (change);

  /* mark the from & to live ranges for variables used */
  markLiveRanges (ebbs, count);

  /* compute which overlaps with what */
  computeClash(ebbs, count);
}

/*-----------------------------------------------------------------*/
/* recomputeLiveRanges - recomputes the live ranges for variables  */
/*-----------------------------------------------------------------*/
void
recomputeLiveRanges (eBBlock **ebbs, int count, bool emitWarnings)
{
  symbol * sym;
  int key;

  /* clear all rlive bitVectors */
  rliveClear (ebbs, count);

  sym = hTabFirstItem (liveRanges, &key);
  if (sym)
    {
      do {
        sym->used = 0;
        sym->liveFrom = 0;
        sym->liveTo = 0;
        freeBitVect (sym->clashes);
        sym->clashes = NULL;
      } while ( (sym = hTabNextItem (liveRanges, &key)));
    }

  /* do the LR computation again */
  computeLiveRanges (ebbs, count, emitWarnings);
}

/*-----------------------------------------------------------------*/
/* dump icode->rlive in all blocks                                 */
/*-----------------------------------------------------------------*/
#if 0
void
dumpIcRlive (eBBlock ** ebbs, int count)
{
  int i, j;
  iCode *ic;

  /* for all blocks do */
  for (i = 0; i < count; i++)
    {
      printf ("bb %d %s alive symbols:\n", i, ebbs[i]->entryLabel->name);
      /* for all instructions in this block do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          printf ("\tic->key %d\n", ic->key);

          if (!ic->rlive)
            continue;
          /* for all live Ranges alive at this point */
          for (j = 1; j < ic->rlive->size; j++)
            {
              symbol *sym;

              if (!bitVectBitValue (ic->rlive, j))
                continue;

              /* find the live range we are interested in */
              if ((sym = hTabItemWithKey (liveRanges, j)))
                printf ("\t\tsym->key %2d: %s\n", sym->key, sym->rname[0] ? sym->rname : sym->name);
            }
        }
    }
}
#endif

/*-----------------------------------------------------------------*/
/* Visit all iCodes reachable from ic                              */
/*-----------------------------------------------------------------*/
static void visit (set **visited, iCode *ic, const int key)
{
  symbol *lbl;

  while (ic && !isinSet (*visited, ic) && bitVectBitValue (ic->rlive, key))
    {
      addSet (visited, ic);

      switch (ic->op)
        {
        case GOTO:
          ic = hTabItemWithKey (labelDef, (IC_LABEL (ic))->key);
          break;
        case RETURN:
          ic = hTabItemWithKey (labelDef, returnLabel->key);
          break;
        case JUMPTABLE:
          for (lbl = setFirstItem (IC_JTLABELS (ic)); lbl; lbl = setNextItem (IC_JTLABELS (ic)))
            visit (visited, hTabItemWithKey (labelDef, lbl->key), key);
          break;
        case IFX:
          visit (visited, hTabItemWithKey (labelDef, (IC_TRUE(ic) ? IC_TRUE (ic) : IC_FALSE (ic))->key), key);
          ic = ic->next;
          break;
        default:
          ic = ic->next;
          if (!POINTER_SET (ic) && IC_RESULT (ic) && IS_SYMOP (IC_RESULT (ic)) && OP_SYMBOL_CONST (IC_RESULT (ic))->key == key)
            {
              addSet (visited, ic);
              return;
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* Split temporaries that have non-connected live ranges           */
/* Such temporaries can result from GCSE and losrpe,               */
/* And can confuse register allocation and rematerialization.      */
/*-----------------------------------------------------------------*/
int
separateLiveRanges (iCode *sic, ebbIndex *ebbi)
{
  set *candidates = 0;
  symbol *sym;
  int num_separated = 0;

  // printf("separateLiveRanges()\n");

  for (iCode *ic = sic; ic; ic = ic->next)
    {
      if (ic->op == IFX || ic->op == GOTO || ic->op == JUMPTABLE || !IC_RESULT (ic) || !IS_ITEMP (IC_RESULT (ic)) || bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) <= 1 || isinSet (candidates, OP_SYMBOL (IC_RESULT (ic))))
        continue;

      addSet (&candidates, OP_SYMBOL (IC_RESULT (ic)));
    }

  if (!candidates)
    return (0);

  for(sym = setFirstItem (candidates); sym; sym = setNextItem (candidates))
    {
      // printf("Looking at %s, %d definitions\n", sym->name, bitVectnBitsOn (sym->defs));

      set *defs = 0;
      set *uses = 0;
      bool skip_uses = false;

      for (int i = 0; i < sym->defs->size; i++)
        {
          if (bitVectBitValue (sym->defs, i))
            {
              iCode *dic;
              if(dic = hTabItemWithKey (iCodehTab, i))
                addSet (&defs, dic);
              else
                {
                  werror (W_INTERNAL_ERROR, __FILE__, __LINE__, "Definition not found");
                  return (num_separated);
                }
            }
          if (bitVectBitValue (sym->uses, i))
            {
              iCode *uic;
              if(uic = hTabItemWithKey (iCodehTab, i))
                addSet (&uses, uic);
              else
                skip_uses = true; // werror (W_INTERNAL_ERROR, __FILE__, __LINE__, "Use not found"); // return (num_separated); seems too harsh.
            }
        }
      do
        {
          set *visited = 0;
          set *newdefs = 0;
          int oldsize;

          wassert (defs);
          wassert (setFirstItem (defs));

          // printf("Looking at def at %d now\n", ((iCode *)(setFirstItem (defs)))->key);

          if (!bitVectBitValue (((iCode *)(setFirstItem (defs)))->rlive, sym->key))
            {
              werror (W_INTERNAL_ERROR, __FILE__, __LINE__, "Variable is not alive at one of its definitions");
              break;
            }
 
          visit (&visited, setFirstItem (defs), sym->key);
          addSet (&newdefs, setFirstItem (defs));

          do
            {
              oldsize = elementsInSet(visited);
              setFirstItem (defs);
              for(iCode *ic = setNextItem (defs); ic; ic = setNextItem (defs))
                {
                  // printf("Looking at other def at %d now\n", ic->key);
                  set *visited2 = 0;
                  set *intersection = 0;
                  visit (&visited2, ic, sym->key);
                  intersection = intersectSets (visited, visited2, THROW_NONE);
                  intersection = subtractFromSet (intersection, defs, THROW_DEST);
                  if (intersection)
                    {
                      visited = unionSets (visited, visited2, THROW_DEST);
                      addSet (&newdefs, ic);
                    }
                  deleteSet (&intersection);
                  deleteSet (&visited2);
                }
             }
          while (oldsize < elementsInSet(visited));

          defs = subtractFromSet (defs, newdefs, THROW_DEST);

          if (newdefs && defs)
            {
              operand *tmpop = newiTempOperand (operandType (IC_RESULT ((iCode *)(setFirstItem (newdefs)))), TRUE);

              // printf("Splitting %s from %s, using def at %d, op %d\n", OP_SYMBOL_CONST(tmpop)->name, sym->name, ((iCode *)(setFirstItem (newdefs)))->key, ((iCode *)(setFirstItem (newdefs)))->op);

              for (iCode *ic = setFirstItem (visited); ic; ic = setNextItem (visited))
                {
                  if (IC_LEFT (ic) && IS_ITEMP (IC_LEFT (ic)) && OP_SYMBOL (IC_LEFT (ic)) == sym)
                    IC_LEFT (ic) = operandFromOperand (tmpop);
                  if (IC_RIGHT (ic) && IS_ITEMP (IC_RIGHT (ic)) && OP_SYMBOL (IC_RIGHT (ic)) == sym)
                      IC_RIGHT (ic) = operandFromOperand (tmpop);
                  if (IC_RESULT (ic) && IS_ITEMP (IC_RESULT (ic)) && OP_SYMBOL (IC_RESULT (ic)) == sym && !POINTER_SET(ic) && ic->next && !isinSet (visited, ic->next))
                    continue;
                  if (IC_RESULT (ic) && IS_ITEMP (IC_RESULT (ic)) && OP_SYMBOL (IC_RESULT (ic)) == sym)
                    {
                      bool pset = POINTER_SET(ic);
                      IC_RESULT (ic) = operandFromOperand (tmpop);
                      if (pset)
                        IC_RESULT(ic)->isaddr = TRUE;
                      else
                        bitVectUnSetBit (sym->defs, ic->key);
                    }
                  bitVectUnSetBit (sym->uses, ic->key);

                  skip_uses = true;
                  num_separated++;
                }
            }
          else if (!skip_uses)
            {      
              set *undefined_uses = 0;
              undefined_uses = subtractFromSet (uses, visited, THROW_NONE);

              // Eliminate uses of undefined variables.
              for (iCode *ic = setFirstItem (undefined_uses); ic; ic = setNextItem (undefined_uses))
                {
                  iCode *prev = ic->prev;
                  iCode *next = ic->next;
                  if (prev && next)
                    {
                      prev->next = next;
                      next->prev = prev;
                    }

                  bitVectUnSetBit (sym->uses, ic->key);
                  if (IS_SYMOP (IC_RESULT (ic)))
                    bitVectUnSetBit (OP_DEFS (IC_RESULT (ic)), ic->key);
                }

              deleteSet (&undefined_uses);
            }

          deleteSet (&newdefs);
          deleteSet (&visited);
        }
      while (elementsInSet(defs) > 1);

      deleteSet (&defs);
      deleteSet (&uses);
    }

  deleteSet (&candidates);

  return (num_separated);
}

/*-----------------------------------------------------------------*/
/* Shorten live ranges by swapping order of operations             */
/*-----------------------------------------------------------------*/
int
shortenLiveRanges (iCode *sic, ebbIndex *ebbi)
{
  int change = 0;

  for (iCode *ic = sic; ic; ic = ic->next)
    {
      iCode *ifx = 0;

      iCode *pic = ic->prev;
      iCode *nic = ic->next;

      if (!pic || !nic)
        continue;

      if (ic->op == IFX || nic->op == IFX)
        continue;

      if (nic->op == IPUSH || nic->op == SEND || nic->op == RETURN)
        continue;

      if (pic->op != '=' || !IS_ITEMP (IC_RESULT (pic)) || bitVectnBitsOn (OP_DEFS (IC_RESULT (pic))) != 1)
        continue;

      if (IC_LEFT (nic) != IC_RESULT (pic) && IC_RIGHT (nic) != IC_RESULT (pic) || bitVectnBitsOn (OP_USES (IC_RESULT (pic))) != 1)
        continue;

      if (IS_OP_VOLATILE (IC_RIGHT (pic)) || IS_OP_VOLATILE (IC_LEFT (nic)) || IS_OP_VOLATILE (IC_RIGHT (nic)) || IS_OP_VOLATILE (IC_RESULT (nic)))
        continue;

      if (isOperandEqual (IC_RESULT (pic), IC_LEFT (ic)) || isOperandEqual (IC_RESULT (pic), IC_RIGHT (ic)))
        continue;

      if (isOperandEqual (IC_RESULT (ic), IC_LEFT (nic)) || isOperandEqual (IC_RESULT (ic), IC_RIGHT (nic)))
        continue;

      if ((POINTER_SET (nic) || isOperandGlobal (IC_RESULT (nic))) && (POINTER_GET (ic) || isOperandGlobal (IC_LEFT (ic)) || isOperandGlobal (IC_RIGHT (ic))) ||
        (POINTER_GET (nic) || isOperandGlobal (IC_LEFT (nic)) || isOperandGlobal (IC_RIGHT (nic))) && (POINTER_SET (ic) || POINTER_SET (nic) && isOperandGlobal (IC_RESULT (ic))))
        continue;

      if (isOperandGlobal (IC_RIGHT (pic)) && !TARGET_IS_STM8 && !TARGET_PDK_LIKE) // Might result in too many global operands per op for backend.
        continue;

      if (ifx = ifxForOp (IC_RESULT (nic), nic))
        {
          const symbol *starget = IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx);
          const iCode *itarget = eBBWithEntryLabel(ebbi, starget)->sch;

          if (nic->next != ifx || bitVectBitValue(itarget->rlive, IC_RESULT (ic)->key))
            continue;
        }

      if (IC_LEFT (nic) == IC_RESULT (pic))
        IC_LEFT (nic) = IC_RIGHT (pic);
      if (IC_RIGHT (nic) == IC_RESULT (pic))
        IC_RIGHT (nic) = IC_RIGHT (pic);
      bitVectUnSetBit (OP_USES (IC_RESULT (pic)), nic->key);
      if (IS_SYMOP (IC_RIGHT (pic)))
        bitVectSetBit (OP_USES (IC_RIGHT (pic)), nic->key);

      // Assignment to self will get optimized out later
      IC_LEFT (pic) = IC_RESULT (pic); 
      bitVectSetBit (OP_USES (IC_RESULT (pic)), pic->key);

      pic->next = nic;
      nic->prev = pic;
      ic->prev = nic;
      ic->next = nic->next;
      nic->next = ic;
      if (ic->next)
        ic->next->prev = ic;

      if (ifx) // Move calculation beyond ifx.
        {
          ifx->prev = ic->prev;
          ic->next = ifx->next;
          ifx->next = ic;
          ic->prev = ifx;

          ifx->prev->next = ifx;
          if (ic->next)
            ic->next->prev = ic;
        }

       change++;
    }

  return (change);
}

