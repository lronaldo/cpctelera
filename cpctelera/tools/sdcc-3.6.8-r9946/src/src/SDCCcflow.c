/*-------------------------------------------------------------------------

  SDCCcflow.c - source file for control flow analysis

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

static void computeDFOrdering (eBBlock *, int *);

/*-----------------------------------------------------------------*/
/* domSetFromVect - creates a domset from the vector               */
/*-----------------------------------------------------------------*/
static set *
domSetFromVect (ebbIndex *ebbi, bitVect * domVect)
{
  int i = 0;
  set *domSet = NULL;

  if (!domVect)
    return NULL;

  for (i = 0; i < domVect->size; i++)
    if (bitVectBitValue (domVect, i))
      addSet (&domSet, ebbi->bbOrder[i]);
  return domSet;
}


/*-----------------------------------------------------------------*/
/* addSuccessor - will add bb to succ also add it to the pred of   */
/*                the next one :                                   */
/*-----------------------------------------------------------------*/
static void 
addSuccessor (eBBlock * thisBlock, eBBlock * succ)
{
  /* check for boundary conditions */
  if (!thisBlock || !succ)
    return;

  /* add it to the succ of thisBlock */
  addSetIfnotP (&thisBlock->succList, succ);

  thisBlock->succVect =
    bitVectSetBit (thisBlock->succVect, succ->bbnum);
  /* add this edge to the list of edges */
  addSet (&graphEdges, newEdge (thisBlock, succ));

}

/*-----------------------------------------------------------------*/
/* eBBPredecessors - find the predecessors for each block          */
/*-----------------------------------------------------------------*/
static void 
eBBPredecessors (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  int i = 0, j;

  /* for each block do */
  for (i = 0; i < count; i++)
    {

      /* if there is no path to this then continue */
      if (ebbs[i]->noPath)
	continue;

      /* for each successor of this block if */
      /* it has depth first number > this block */
      /* then this block precedes the successor  */
      for (j = 0; j < ebbs[i]->succVect->size; j++)

	if (bitVectBitValue (ebbs[i]->succVect, j) &&
	    ebbs[j]->dfnum > ebbs[i]->dfnum)

	  addSet (&ebbs[j]->predList, ebbs[i]);
    }
}

/*-----------------------------------------------------------------*/
/* eBBSuccessors- find out the successors of all the nodes         */
/*-----------------------------------------------------------------*/
static void 
eBBSuccessors (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  int i = 0;

  /* for all the blocks do */
  for (; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath)
	continue;

      ebbs[i]->succVect = newBitVect (count);

      /* if the next on exists & this one does not */
      /* end in a GOTO or RETURN then the next is  */
      /* a natural successor of this. Note we have */
      /* consider eBBlocks with no instructions    */
      if (ebbs[i + 1])
	{

	  if (ebbs[i]->ech)
	    {
              bool foundNoReturn = FALSE;
              if (ebbs[i]->ech->op == CALL || ebbs[i]->ech->op == PCALL)
                {
                  sym_link *type = operandType (IC_LEFT (ebbs[i]->ech));
                  if (IS_FUNCPTR (type))
                    type = type->next;
                  if (type && FUNC_ISNORETURN (type))
                    foundNoReturn = TRUE;
                }
	      if (!foundNoReturn &&
                 ebbs[i]->ech->op != GOTO &&
		  ebbs[i]->ech->op != RETURN &&
		  ebbs[i]->ech->op != JUMPTABLE)
		{
		  int j = i + 1;

		  while (ebbs[j] && ebbs[j]->noPath)
		    j++;

		  addSuccessor (ebbs[i], ebbs[j]);	/* add it */
		}
	      else
		{
		  if (i && ebbs[i-1]->ech && ebbs[i-1]->ech->op==IFX) {
		    ebbs[i]->isConditionalExitFrom=ebbs[i-1];
		  }
		}
	    }			/* no instructions in the block */
	  /* could happen for dummy blocks */
	  else
	    addSuccessor (ebbs[i], ebbs[i + 1]);
	}

      /* go thru all the instructions: if we find a */
      /* goto or ifx or a return then we have a succ */
      if ((ic = ebbs[i]->ech))
	{
	  eBBlock *succ;

	  /* special case for jumptable */
	  if (ic->op == JUMPTABLE)
	    {
	      symbol *lbl;
	      for (lbl = setFirstItem (IC_JTLABELS (ic)); lbl;
		   lbl = setNextItem (IC_JTLABELS (ic)))
		addSuccessor (ebbs[i],
			      eBBWithEntryLabel (ebbi, lbl));
	    }
	  else
	    {

	      succ = NULL;
	      /* depending on the instruction operator */
	      switch (ic->op)
		{
		case GOTO:	/* goto has edge to label */
		  succ = eBBWithEntryLabel (ebbi, ic->label);

                  /* Sometimes a block has a GOTO added after the original */
                  /* final IFX (due to loop optimizations). If IFX found,  */
                  /* fall through to handle the IFX too. */
                  if (ic->prev && ic->prev->op == IFX)
                    {
                      if (succ)
                        addSuccessor (ebbs[i], succ); /* add the GOTO target */
                      ic = ic->prev;       /* get ready to handle IFX too. */
                    }
                  else
                    break;

		case IFX:	/* conditional jump */
		  /* if true label is present */
		  if (IC_TRUE (ic))
		    succ = eBBWithEntryLabel (ebbi, IC_TRUE (ic));
		  else
		    succ = eBBWithEntryLabel (ebbi, IC_FALSE (ic));
		  break;

		case RETURN:	/* block with return */
		  succ = eBBWithEntryLabel (ebbi, returnLabel);
		  break;
		}

	      /* if there is a successor add to the list */
	      /* if it is not already present in the list */
	      if (succ)
		addSuccessor (ebbs[i], succ);
	    }
	}
    }
}

/*-----------------------------------------------------------------*/
/* computeDominance - computes the dominance graph                 */
/* for algorithm look at Dragon book section 10.10, algo 10.16     */
/*-----------------------------------------------------------------*/
static void 
computeDominance (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  int i, j;

  /* now do the initialisation */
  /* D(n0) := { n0 } */
  ebbs[0]->domVect =
    bitVectSetBit (ebbs[0]->domVect = newBitVect (count), ebbs[0]->bbnum);


  /* for n in N - { n0 } do D(n) = N */
  for (i = 1; i < count; i++)
    {
      ebbs[i]->domVect = newBitVect (count);
      for (j = 0; j < count; j++)
	{
	  ebbs[i]->domVect =
	    bitVectSetBit (ebbs[i]->domVect, ebbs[j]->bbnum);
	}
    }

  /* end of initialisation */

  /* while changes to any D(n) occur do */
  /*   for n in N - { n0 } do           */
  /*       D(n) := { n } U  (intersection of D( all predecessors of n)) */
  while (1)
    {
      int change;

      change = 0;
      for (i = 1; i < count; i++)
	{
	  bitVect *cDomVect;
	  eBBlock *pred;

	  cDomVect = NULL;

	  /* get the intersection of the dominance of all predecessors */
	  for (pred = setFirstItem (ebbs[i]->predList),
	       cDomVect = (pred ? bitVectCopy (pred->domVect) : NULL);
	       pred;
	       pred = setNextItem (ebbs[i]->predList))
	    {
	      cDomVect = bitVectIntersect (cDomVect, pred->domVect);
	    }
	  if (!cDomVect)
	    cDomVect = newBitVect (count);
	  /* this node to the list */
	  cDomVect = bitVectSetBit (cDomVect, ebbs[i]->bbnum);


	  if (!bitVectEqual (cDomVect, ebbs[i]->domVect))
	    {
	      ebbs[i]->domVect = cDomVect;
	      change = 1;
	    }
	}

      /* if no change then exit */
      if (!change)
	break;
    }
  
}

/*-----------------------------------------------------------------*/
/* immedDom - returns the immediate dominator of a block           */
/*-----------------------------------------------------------------*/
eBBlock *
immedDom (ebbIndex * ebbi, eBBlock * ebp)
{
  /* first delete self from the list */
  set *iset = domSetFromVect (ebbi, ebp->domVect);
  eBBlock *loop;
  eBBlock *idom = NULL;

  deleteSetItem (&iset, ebp);
  /* then just return the one with the greatest */
  /* depthfirst number, this will be the immed dominator */
  if ((loop = setFirstItem (iset)))
    idom = loop;
  for (; loop; loop = setNextItem (iset))
    if (loop->dfnum > idom->dfnum)
      idom = loop;

  setToNull ((void *) &iset);
  return idom;

}

/*-----------------------------------------------------------------*/
/* DFOrdering - is visited then nothing else call DFOrdering this  */
/*-----------------------------------------------------------------*/
DEFSETFUNC (DFOrdering)
{
  eBBlock *ebbp = item;
  V_ARG (int *, count);

  if (ebbp->visited)
    return 0;

  computeDFOrdering (ebbp, count);	/* depthfirst */

  return 0;
}

/*-----------------------------------------------------------------*/
/* computeDFOrdering - computes the depth first ordering of the    */
/*                     flowgraph                                   */
/*-----------------------------------------------------------------*/
static void 
computeDFOrdering (eBBlock * ebbp, int *count)
{

  ebbp->visited = 1;
  /* for each successor that is not visited */
  applyToSet (ebbp->succList, DFOrdering, count);

  /* set the depth first number */
  ebbp->dfnum = *count;
  *count -= 1;
}

/*-----------------------------------------------------------------*/
/* disconBBlock - removes all control flow links for a block       */
/*-----------------------------------------------------------------*/
void 
disconBBlock (eBBlock * ebp, ebbIndex * ebbi)
{
  /* mark this block as noPath & recompute control flow */
  ebp->noPath = 1;
  computeControlFlow (ebbi);
}

/*-----------------------------------------------------------------*/
/* markNoPath - marks those blocks which cannot be reached from top */
/*-----------------------------------------------------------------*/
static void 
markNoPath (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  int i;

  /* for all blocks if the visited flag is not set : then there */
  /* is no path from _entry to this block push them down in the */
  /* depth first order */
  for (i = 0; i < count; i++)
    if (!ebbs[i]->visited)
      ebbs[i]->noPath = 1;
}

/*-----------------------------------------------------------------*/
/* dfNumCompare - used by qsort to sort by dfNumber                */
/*-----------------------------------------------------------------*/
int 
dfNumCompare (const void *a, const void *b)
{
  const eBBlock *const *i = a;
  const eBBlock *const *j = b;

  if ((*i)->dfnum > (*j)->dfnum)
    return 1;

  if ((*i)->dfnum < (*j)->dfnum)
    return -1;

  return 0;
}

/*-----------------------------------------------------------------*/
/* computeControlFlow - does the control flow computation          */
/*-----------------------------------------------------------------*/
void 
computeControlFlow (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int dfCount = ebbi->count;
  int i;

  /* initialise some things */

  for (i = 0; i < ebbi->count; i++)
    {
      setToNull ((void *) &ebbs[i]->predList);
      setToNull ((void *) &ebbs[i]->domVect);
      setToNull ((void *) &ebbs[i]->succList);
      setToNull ((void *) &ebbs[i]->succVect);
      ebbs[i]->visited = 0;
      ebbs[i]->dfnum = 0;
    }

  setToNull ((void *) &graphEdges);
  /* this will put in the  */
  /* successor information for each blk */
  eBBSuccessors (ebbi);

  /* compute the depth first ordering */
  computeDFOrdering (ebbi->bbOrder[0], &dfCount);

  /* mark blocks with no paths to them */
  markNoPath (ebbi);

  /* with the depth first info in place */
  /* add the predecessors for the blocks */
  eBBPredecessors (ebbi);

  /* compute the dominance graph */
  computeDominance (ebbi);

  /* sort it by dfnumber */
  if (!ebbi->dfOrder)
    ebbi->dfOrder = Safe_alloc ((ebbi->count+1) * sizeof (eBBlock *));
  for (i = 0; i < (ebbi->count+1); i++)
    {
      ebbi->dfOrder[i] = ebbi->bbOrder[i];
    }
      
  qsort (ebbi->dfOrder, ebbi->count, sizeof (eBBlock *), dfNumCompare);

}

/*-----------------------------------------------------------------*/
/* returnAtEnd - returns 1 if Basic Block has a return at the end  */
/*               of it                                             */
/*-----------------------------------------------------------------*/
int returnAtEnd (eBBlock *ebp)
{
    /* case 1.
       This basic block ends in a return statment 
    */
    if (ebp->ech && ebp->ech->op == RETURN) return 1;

    /* case 2.
       This basic block has only one successor and that
       successor has only one return statement
    */
    if (elementsInSet(ebp->succList) == 1) {
	eBBlock *succ = setFirstItem(ebp->succList);
	/* could happen for dummy blocks */
	if (!succ->sch || !succ->ech) return 0;

	/* first iCode is a return */
	if (succ->sch->op == RETURN) return 2;

	/* or first iCode is a label & the next &
	   last icode is a return */
	if (succ->sch->op == LABEL && succ->sch->next == succ->ech &&
	    succ->ech->op == RETURN ) return 2;
    }

    return 0;
}

