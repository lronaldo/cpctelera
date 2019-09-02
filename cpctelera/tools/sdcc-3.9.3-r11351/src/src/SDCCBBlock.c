/*-------------------------------------------------------------------------

  SDCCBBlock.c - routines to manipulate basic Blocks

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
#include "dbuf_string.h"

int eBBNum = 0;
set *graphEdges = NULL;         /* list of edges in this flow graph */

struct _dumpFiles dumpFiles[] = {
  {DUMP_RAW0, ".dumpraw0", NULL},
  {DUMP_RAW1, ".dumpraw1", NULL},
  {DUMP_CSE, ".dumpcse", NULL},
  {DUMP_DFLOW, ".dumpdflow", NULL},
  {DUMP_GCSE, ".dumpgcse", NULL},
  {DUMP_DEADCODE, ".dumpdeadcode", NULL},
  {DUMP_LOOP, ".dumploop", NULL},
  {DUMP_LOOPG, ".dumploopg", NULL},
  {DUMP_LOOPD, ".dumploopd", NULL},
  {DUMP_RANGE, ".dumprange", NULL},
  {DUMP_PACK, ".dumppack", NULL},
  {DUMP_RASSGN, ".dumprassgn", NULL},
  {DUMP_LRANGE, ".dumplrange", NULL},
  {DUMP_LOSPRE, ".dumplospre", NULL},
  {DUMP_CUSTOM, ".dumpcustom", NULL},
  {0, NULL, NULL}
};

/*-----------------------------------------------------------------*/
/* printEntryLabel - prints entry label of a ebblock               */
/*-----------------------------------------------------------------*/
DEFSETFUNC (printEntryLabel)
{
  eBBlock *bp = item;

  fprintf (stdout, " %-20s ", bp->entryLabel->name);
  return 0;
}

/*-----------------------------------------------------------------*/
/* neweBBlock - allocate & return a new extended basic block       */
/*-----------------------------------------------------------------*/
eBBlock *
neweBBlock ()
{
  eBBlock *ebb;

  ebb = Safe_alloc (sizeof (eBBlock));
  return ebb;
}

/*-----------------------------------------------------------------*/
/* newEdge - allocates & initialises an edge to given values       */
/*-----------------------------------------------------------------*/
edge *
newEdge (eBBlock * from, eBBlock * to)
{
  edge *ep;

  ep = Safe_alloc (sizeof (edge));

  ep->from = from;
  ep->to = to;
  return ep;
}

/*-----------------------------------------------------------------*/
/* createDumpFile - create the dump file                           */
/*-----------------------------------------------------------------*/
FILE *
createDumpFile (int id)
{
  struct _dumpFiles *dumpFilesPtr = dumpFiles;
  static int dumpIndex = 0;
  static char dumpIndexStr[32];

  while (dumpFilesPtr->id)
    {
      if (dumpFilesPtr->id == id)
        break;
      dumpFilesPtr++;
    }

  if (!dumpFilesPtr->id)
    {
      fprintf (stdout, "internal error: createDumpFile: unknown dump file.\n");
      exit (1);
    }

  sprintf (dumpIndexStr, ".%d", dumpIndex);
  dumpIndex++;

  if (!dumpFilesPtr->filePtr)
    {
      // not used before, create it
      struct dbuf_s dumpFileName;

      dbuf_init (&dumpFileName, PATH_MAX);
      dbuf_append_str (&dumpFileName, dstFileName);
#ifdef _DEBUG
      dbuf_append_str (&dumpFileName, dumpIndexStr);
#endif
      dbuf_append_str (&dumpFileName, dumpFilesPtr->ext);
      if (!(dumpFilesPtr->filePtr = fopen (dbuf_c_str (&dumpFileName), "w")))
        {
          werror (E_FILE_OPEN_ERR, dbuf_c_str (&dumpFileName));
          dbuf_destroy (&dumpFileName);
          exit (1);
        }
      dbuf_destroy (&dumpFileName);
    }

#if 0
  fprintf (dumpFilesPtr->filePtr, "Dump file index: %d\n", dumpIndex);
#endif

  return dumpFilesPtr->filePtr;
}

/*-----------------------------------------------------------------*/
/* closeDumpFiles - close possible opened dumpfiles                */
/*-----------------------------------------------------------------*/
void
closeDumpFiles ()
{
  struct _dumpFiles *dumpFilesPtr;

  for (dumpFilesPtr = dumpFiles; dumpFilesPtr->id; dumpFilesPtr++)
    {
      if (dumpFilesPtr->filePtr)
        {
          fclose (dumpFilesPtr->filePtr);
        }
    }
}

/*-----------------------------------------------------------------*/
/* dumpLiveRanges - dump liverange information into a file         */
/*-----------------------------------------------------------------*/
void
dumpLiveRanges (int id, hTab * liveRanges)
{
  FILE *file;
  symbol *sym;
  int k;

  if (id)
    {
      file = createDumpFile (id);
    }
  else
    {
      file = stdout;
    }

  if (currFunc)
    fprintf (file, "------------- Func %s -------------\n", currFunc->name);
  for (sym = hTabFirstItem (liveRanges, &k); sym; sym = hTabNextItem (liveRanges, &k))
    {

      fprintf (file, "%s [k%d lr%d:%d so:%d]{ re%d rm%d}",
               (sym->rname[0] ? sym->rname : sym->name),
               sym->key, sym->liveFrom, sym->liveTo, sym->stack, sym->isreqv, sym->remat);

      fprintf (file, "{");
      printTypeChain (sym->type, file);
      if (sym->usl.spillLoc)
        {
          fprintf (file, "}{ sir@ %s", sym->usl.spillLoc->rname);
        }
      fprintf (file, "} clashes with ");
      bitVectDebugOn (sym->clashes, file);
      fprintf (file, "\n");
    }

  fflush (file);
}


/*-----------------------------------------------------------------*/
/* dumpEbbsToFileExt - write all the basic blocks to a file        */
/*-----------------------------------------------------------------*/
void
dumpEbbsToFileExt (int id, ebbIndex * ebbi)
{
  FILE *of;
  int i, d;
  eBBlock *bb;
  set *cseSet;
  eBBlock **ebbs = ebbi->dfOrder ? ebbi->dfOrder : ebbi->bbOrder;
  int count = ebbi->count;

  if (id)
    {
      of = createDumpFile (id);
    }
  else
    {
      of = stdout;
    }

  for (i = 0; i < count; i++)
    {
      fprintf (of, "\n----------------------------------------------------------------\n");
      fprintf (of, "Basic Block %s (df:%d bb:%d lvl:%ld:%ld): loopDepth=%d%s%s%s\n",
               ebbs[i]->entryLabel->name,
               ebbs[i]->dfnum, ebbs[i]->bbnum,
               ebbs[i]->entryLabel->level / LEVEL_UNIT, ebbs[i]->entryLabel->level % LEVEL_UNIT,
               ebbs[i]->depth,
               ebbs[i]->noPath ? " noPath" : "",
               ebbs[i]->partOfLoop ? " partOfLoop" : "", ebbs[i]->isLastInLoop ? " isLastInLoop" : "");

      // a --nolabelopt makes this more readable
      fprintf (of, "\nsuccessors: ");
      for (bb = setFirstItem (ebbs[i]->succList); bb; bb = setNextItem (ebbs[i]->succList))
        {
          fprintf (of, "%s ", bb->entryLabel->name);
        }
      fprintf (of, "\npredecessors: ");
      for (bb = setFirstItem (ebbs[i]->predList); bb; bb = setNextItem (ebbs[i]->predList))
        {
          fprintf (of, "%s ", bb->entryLabel->name);
        }
      fprintf (of, "\ndominators: ");
      for (d = 0; d < ebbs[i]->domVect->size; d++)
        {
          if (bitVectBitValue (ebbs[i]->domVect, d))
            {
              fprintf (of, "%s ", ebbi->bbOrder[d]->entryLabel->name);  //ebbs[d]->entryLabel->name);
            }
        }
      fprintf (of, "\n");

      fprintf (of, "\ndefines bitVector :");
      bitVectDebugOn (ebbs[i]->defSet, of);
      fprintf (of, "\nlocal defines bitVector :");
      bitVectDebugOn (ebbs[i]->ldefs, of);
      fprintf (of, "\npointers Set bitvector :");
      bitVectDebugOn (ebbs[i]->ptrsSet, of);
#if 0
      fprintf (of, "\nin coming definitions :");
      bitVectDebugOn (ebbs[i]->inDefs, of);
      fprintf (of, "\nout going definitions :");
      bitVectDebugOn (ebbs[i]->outDefs, of);
      fprintf (of, "\ndefines used :");
      bitVectDebugOn (ebbs[i]->usesDefs, of);
#endif

      if (ebbs[i]->isLastInLoop)
        {
          fprintf (of, "\nInductions Set bitvector :");
          bitVectDebugOn (ebbs[i]->linds, of);
        }

      fprintf (of, "\ninExprs:");
      for (cseSet = ebbs[i]->inExprs; cseSet; cseSet = cseSet->next)
        {
          cseDef *item = cseSet->item;
          fprintf (of, " %s(%d)", OP_SYMBOL (item->sym)->name, item->diCode->key);
          if (item->fromGlobal)
            fprintf (of, "g");
        }
      fprintf (of, "\noutExprs:");
      for (cseSet = ebbs[i]->outExprs; cseSet; cseSet = cseSet->next)
        {
          cseDef *item = cseSet->item;
          fprintf (of, " %s(%d)", OP_SYMBOL (item->sym)->name, item->diCode->key);
          if (item->fromGlobal)
            fprintf (of, "g");
        }
      fprintf (of, "\nkilledExprs:");
      for (cseSet = ebbs[i]->killedExprs; cseSet; cseSet = cseSet->next)
        {
          cseDef *item = cseSet->item;
          fprintf (of, " %s(%d)", OP_SYMBOL (item->sym)->name, item->diCode->key);
          if (item->fromGlobal)
            fprintf (of, "g");
        }

      fprintf (of, "\n----------------------------------------------------------------\n");
      printiCChain (ebbs[i]->sch, of);
    }
  fflush (of);
}

/*-----------------------------------------------------------------*/
/* iCode2eBBlock - converts a sequnce till label to a ebb          */
/*-----------------------------------------------------------------*/
eBBlock *
iCode2eBBlock (iCode * ic)
{
  iCode *loop;
  eBBlock *ebb = neweBBlock (); /* allocate an entry */

  /* put the first one unconditionally */
  ebb->sch = ic;
  ic->seq = 0;

  /* if this is a label then */
  if (ic->op == LABEL)
    ebb->entryLabel = ic->label;
  else
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      dbuf_printf (&dbuf, "_eBBlock%d", eBBNum++);
      ebb->entryLabel = newSymbol (dbuf_c_str (&dbuf), 1);
      dbuf_destroy (&dbuf);
      ebb->entryLabel->key = labelKey++;
    }

  if (ic && (ic->op == GOTO || ic->op == JUMPTABLE || ic->op == IFX))
    {
      ebb->ech = ebb->sch;
      return ebb;
    }

  /* if this is a function call */
  if (ic->op == CALL || ic->op == PCALL)
    {
      sym_link *type = operandType (IC_LEFT (ic));
      ebb->hasFcall = 1;
      if (currFunc)
        FUNC_HASFCALL (currFunc->type) = 1;
      if (IS_FUNCPTR (type))
        type = type->next;
      if (type && FUNC_ISNORETURN (type))
        {
          ebb->ech = ebb->sch;
          return ebb;
        }
    }

  if ((ic->next && ic->next->op == LABEL) || !ic->next)
    {
      ebb->ech = ebb->sch;
      return ebb;
    }

  /* loop thru till we find one with a label */
  for (loop = ic->next; loop; loop = loop->next)
    {
      loop->seq = 0;

      /* if this is the last one */
      if (!loop->next)
        break;
      /* if this is a function call */
      if (loop->op == CALL || loop->op == PCALL)
        {
          sym_link *type = operandType (IC_LEFT (loop));
          ebb->hasFcall = 1;
          if (currFunc)
            FUNC_HASFCALL (currFunc->type) = 1;
          if (IS_FUNCPTR (type))
            type = type->next;
          if (type && FUNC_ISNORETURN (type))
            break;
        }

      /* if the next one is a label */
      /* if this is a goto or ifx */
      if (loop->next->op == LABEL || loop->op == GOTO || loop->op == JUMPTABLE || loop->op == IFX)
        break;
    }

  /* mark the end of the chain */
  ebb->ech = loop;

  return ebb;
}

/*-----------------------------------------------------------------*/
/* eBBWithEntryLabel - finds the basic block with the entry label  */
/*-----------------------------------------------------------------*/
eBBlock *
eBBWithEntryLabel (ebbIndex* ebbi, const symbol *eLabel)
{
  eBBlock **ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  int i;

  for (i = 0; i < count; i++)
    {
      if (isSymbolEqual (ebbs[i]->entryLabel, eLabel))
        return ebbs[i];
    }

  return NULL;
}


/*-----------------------------------------------------------------*/
/* ifFromIs - will return 1 if the from block matches this         */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifFromIs)
{
  edge *ep = item;
  V_ARG (eBBlock *, this);

  if (ep->from == this)
    return 1;

  return 0;
}


/*-----------------------------------------------------------------*/
/* edgesTo  - returns a set of edges with to == supplied value     */
/*-----------------------------------------------------------------*/
set *
edgesTo (eBBlock * to)
{
  set *result = NULL;
  edge *loop;

  for (loop = setFirstItem (graphEdges); loop; loop = setNextItem (graphEdges))
    if (loop->to == to && !loop->from->noPath)
      addSet (&result, loop->from);

  return result;
}


/*-----------------------------------------------------------------*/
/* addiCodeToeBBlock - will add an iCode to the end of a block     */
/*-----------------------------------------------------------------*/
void
addiCodeToeBBlock (eBBlock * ebp, iCode * ic, iCode * ip)
{
  ic->prev = ic->next = NULL;
  /* if the insert point is given */
  if (ip)
    {
      ic->filename = ip->filename;
      ic->lineno = ip->lineno;
      ic->eBBlockNum = ip->eBBlockNum;
      ic->prev = ip->prev;
      ip->prev = ic;
      ic->next = ip;
      if (!ic->prev)
        ebp->sch = ic;
      else
        ic->prev->next = ic;
      return;
    }

  /* if the block has no  instructions */
  if (ebp->ech == NULL)
    {
      ebp->sch = ebp->ech = ic;
      ic->next = NULL;
      return;
    }

  /* if the last instruction is a goto */
  /* we add it just before the goto    */
  if (ebp->ech->op == GOTO || ebp->ech->op == JUMPTABLE || ebp->ech->op == RETURN)
    {
      ic->filename = ebp->ech->filename;
      ic->lineno = ebp->ech->lineno;
      ic->prev = ebp->ech->prev;
      ebp->ech->prev = ic;
      ic->next = ebp->ech;
      if (!ic->prev)            /* was the last only on in the block */
        ebp->sch = ic;
      else
        ic->prev->next = ic;
      return;
    }

  /* if the last one was a ifx statement we check to see */
  /* if the condition was defined in the previous instruction */
  /* if this is true then we put it before the condition else */
  /* we put it before if, this is to reduce register pressure, */
  /* we don't have to hold  condition too long in a register  */

  /* loop induction sometimes appends a GOTO instruction, */
  /* it must be at the very end */
  if (ebp->ech->op == IFX && ic->op != GOTO)
    {
      iCode *ipoint;

/*  if ( !ebp->ech->prev )  */
/*      ipoint = ebp->ech ; */
/*  else  */
/*      if (!IC_RESULT(ebp->ech->prev)) */
/*    ipoint = ebp->ech ; */
/*      else */
/*    if (IC_COND(ebp->ech)->key == IC_RESULT(ebp->ech->prev)->key) */
/*        ipoint = ebp->ech->prev; */
/*    else */
/*        ipoint = ebp->ech ; */
      ipoint = ebp->ech;
      ic->filename = ipoint->filename;
      ic->lineno = ipoint->lineno;
      ic->prev = ipoint->prev;
      ipoint->prev = ic;
      ic->next = ipoint;
      if (!ic->prev)
        ebp->sch = ic;
      else
        ic->prev->next = ic;
      return;
    }

  /* will add it to the very end */
  ip = ebp->ech;
  ip->next = ic;
  ic->prev = ip;
  ic->next = NULL;
  ebp->ech = ic;

  return;
}

/*-----------------------------------------------------------------*/
/* remiCodeFromeBBlock - remove an iCode from BBlock               */
/*-----------------------------------------------------------------*/
void
remiCodeFromeBBlock (eBBlock * ebb, iCode * ic)
{
  wassert (ic->seq >= ebb->fSeq && ic->seq <= ebb->lSeq);
  if (ic->prev)
    ic->prev->next = ic->next;
  else
    ebb->sch = ic->next;

  if (ic->next)
    ic->next->prev = ic->prev;
  else
    ebb->ech = ic->prev;
}

/*-----------------------------------------------------------------*/
/* iCodeBreakDown : breakDown iCode chain to blocks                */
/*-----------------------------------------------------------------*/
ebbIndex *
iCodeBreakDown (iCode * ic)
{
  eBBlock **ebbs = NULL;
  iCode *loop = ic;
  ebbIndex *ebbi;

  ebbi = Safe_alloc (sizeof (ebbIndex));
  ebbi->count = 0;
  ebbi->dfOrder = NULL;         /* no depth first order information yet */

  /* allocate for the first entry */

  ebbs = Safe_alloc (sizeof (eBBlock *));
  ebbi->bbOrder = ebbs;

  while (loop)
    {

      /* convert 2 block */
      eBBlock *ebb = iCode2eBBlock (loop);
      loop = ebb->ech->next;

      ebb->ech->next = NULL;    /* mark the end of this chain */
      if (loop)
        loop->prev = NULL;
      ebb->bbnum = ebbi->count; /* save this block number     */
      /* put it in the array */
      ebbs[(ebbi->count)++] = ebb;

      /* allocate for the next one. Remember to clear the new */
      /*  pointer at the end, that was created by realloc. */

      ebbs = Safe_realloc (ebbs, (ebbi->count + 1) * sizeof (eBBlock *));
      ebbi->bbOrder = ebbs;

      ebbs[ebbi->count] = 0;

      /* if this one ends in a goto or a conditional */
      /* branch then check if the block it is going  */
      /* to already exists, if yes then this could   */
      /* be a loop, add a preheader to the block it  */
      /* goes to  if it does not already have one    */
      if (ebbs[(ebbi->count) - 1]->ech && (ebbs[(ebbi->count) - 1]->ech->op == GOTO || ebbs[(ebbi->count) - 1]->ech->op == IFX))
        {

          symbol *label;
          eBBlock *destBlock;

          if (ebbs[(ebbi->count) - 1]->ech->op == GOTO)
            label = IC_LABEL (ebbs[(ebbi->count) - 1]->ech);
          else if (!(label = IC_TRUE (ebbs[(ebbi->count) - 1]->ech)))
            label = IC_FALSE (ebbs[(ebbi->count) - 1]->ech);

          if ((destBlock = eBBWithEntryLabel (ebbi, label)) &&
              destBlock->preHeader == NULL && otherPathsPresent (ebbs, destBlock))
            {

              symbol *preHeaderLabel = newiTempLoopHeaderLabel (1);
              int i, j;
              eBBlock *pBlock;

              /* go thru all block replacing the entryLabel with new label */
              /* till we reach the block , then we insert a new ebblock    */
              for (i = 0; i < (ebbi->count); i++)
                {
                  if (ebbs[i] == destBlock)
                    break;
                  replaceLabel (ebbs[i], label, preHeaderLabel);
                }

              (ebbi->count)++;

              /* if we have stopped at the block , allocate for an extra one */

              ebbs = Safe_realloc (ebbs, (ebbi->count + 1) * sizeof (eBBlock *));
              ebbi->bbOrder = ebbs;

              ebbs[ebbi->count] = 0;

              /* then move the block down one count */
              pBlock = ebbs[j = i];
              for (i += 1; i < (ebbi->count); i++)
                {
                  eBBlock *xBlock;

                  xBlock = ebbs[i];
                  ebbs[i] = pBlock;
                  ebbs[i]->bbnum = i;
                  pBlock = xBlock;
                }

              destBlock->preHeader = ebbs[j] = neweBBlock ();
              ebbs[j]->bbnum = j;
              ebbs[j]->entryLabel = preHeaderLabel;
              ebbs[j]->sch = ebbs[j]->ech = newiCodeLabelGoto (LABEL, preHeaderLabel);
              ebbs[j]->sch->filename = destBlock->sch->filename;
              ebbs[j]->sch->lineno = destBlock->sch->lineno;
            }
        }
    }

  /* mark the end */
  ebbs[ebbi->count] = NULL;

  return ebbi;
}

/*-----------------------------------------------------------------*/
/* replaceSymBySym : - replace operand by operand in blocks        */
/*                     replaces only left & right in blocks        */
/*-----------------------------------------------------------------*/
void
replaceSymBySym (set * sset, operand * src, operand * dest)
{
  set *loop;
  eBBlock *rBlock;

  /* for all blocks in the set do */
  for (loop = sset; loop; loop = loop->next)
    {
      iCode *ic;

      rBlock = loop->item;
      /* for all instructions in this block do */
      for (ic = rBlock->sch; ic; ic = ic->next)
        {

          /* if we find usage */
          if (ic->op == IFX && isOperandEqual (src, IC_COND (ic)))
            {
              bitVectUnSetBit (OP_USES (IC_COND (ic)), ic->key);
              IC_COND (ic) = operandFromOperand (dest);
              OP_USES (dest) = bitVectSetBit (OP_USES (dest), ic->key);
              continue;
            }

          if (isOperandEqual (IC_RIGHT (ic), src))
            {
              bitVectUnSetBit (OP_USES (IC_RIGHT (ic)), ic->key);
              IC_RIGHT (ic) = operandFromOperand (dest);
              IC_RIGHT (ic)->isaddr = 0;
              OP_USES (dest) = bitVectSetBit (OP_USES (dest), ic->key);
            }

          if (isOperandEqual (IC_LEFT (ic), src))
            {
              bitVectUnSetBit (OP_USES (IC_LEFT (ic)), ic->key);
              if (POINTER_GET (ic) && IS_ITEMP (dest))
                {
                  IC_LEFT (ic) = operandFromOperand (dest);
                  IC_LEFT (ic)->isaddr = 1;
                }
              else
                {
                  IC_LEFT (ic) = operandFromOperand (dest);
                  IC_LEFT (ic)->isaddr = 0;
                }
              OP_USES (dest) = bitVectSetBit (OP_USES (dest), ic->key);
            }

          /* special case for pointer sets */
          if (POINTER_SET (ic) && isOperandEqual (IC_RESULT (ic), src))
            {
              bitVectUnSetBit (OP_USES (IC_RESULT (ic)), ic->key);
              IC_RESULT (ic) = operandFromOperand (dest);
              IC_RESULT (ic)->isaddr = 1;
              OP_USES (dest) = bitVectSetBit (OP_USES (dest), ic->key);
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* replaceLabel - replace reference to one label by another        */
/*-----------------------------------------------------------------*/
void
replaceLabel (eBBlock *ebp, symbol *fromLbl, symbol *toLbl)
{
  iCode *ic;

  if (!ebp)
    return;

  for (ic = ebp->sch; ic; ic = ic->next)
    {
      switch (ic->op)
        {

        case GOTO:
          if (isSymbolEqual (IC_LABEL (ic), fromLbl))
            IC_LABEL (ic) = toLbl;
          break;

        case IFX:
          if (IC_TRUE (ic) && isSymbolEqual (IC_TRUE (ic), fromLbl))
            IC_TRUE (ic) = toLbl;
          else if (isSymbolEqual (IC_FALSE (ic), fromLbl))
            IC_FALSE (ic) = toLbl;
          break;

        case JUMPTABLE:
          replaceSetItem (IC_JTLABELS (ic), fromLbl, toLbl);
        }
    }

  return;

}


/*-----------------------------------------------------------------*/
/* iCodeFromeBBlock - convert basic block to iCode chain           */
/*-----------------------------------------------------------------*/
iCode *
iCodeFromeBBlock (eBBlock ** ebbs, int count)
{
  int i = 1;
  iCode *ric = ebbs[0]->sch;
  iCode *lic = ebbs[0]->ech;

  for (; i < count; i++)
    {
      if (ebbs[i]->sch == NULL)
        continue;

      if (ebbs[i]->noPath && optimize.label4 && (ebbs[i]->entryLabel != entryLabel && ebbs[i]->entryLabel != returnLabel))
        {
          iCode *ic = NULL;
          bool foundNonlabel = 0;
          ic = ebbs[i]->sch;
          do
            {
              if (ic->op != LABEL)
                {
                  foundNonlabel = 1;
                  break;
                }
              if (ic == ebbs[i]->ech)
                break;
              ic = ic->next;
            }
          while (ic);
          if (foundNonlabel && ic)
            {
              werrorfl (ic->filename, ic->lineno, W_CODE_UNREACH);
              continue;
            }
        }

      lic->next = ebbs[i]->sch;
      lic->next->prev = lic;
      lic = ebbs[i]->ech;

    }
  
  return ric;
}

/*-----------------------------------------------------------------*/
/* otherPathsPresent - determines if there is a path from _entry   */
/*      to this block in a half constructed set of blocks          */
/*-----------------------------------------------------------------*/
int
otherPathsPresent (eBBlock ** ebbs, eBBlock * this)
{
  int i;

  /* for all blocks preceding this block */
  for (i = 0; i < this->bbnum; i++)
    {
      iCode *ic;

      /* if there is a reference to the entry label of this block */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          switch (ic->op)
            {
            case GOTO:
              if (IC_LABEL (ic)->key == this->entryLabel->key)
                return 1;
              break;

            case IFX:
              if (IC_TRUE (ic))
                {
                  if (IC_TRUE (ic)->key == this->entryLabel->key)
                    return 1;
                }
              else if (IC_FALSE (ic)->key == this->entryLabel->key)
                return 1;
              break;
            }
        }
    }

  /* comes here means we have not found it yet */
  /* in this case check if the previous block  */
  /* ends in a goto if it does then we have no */
  /* path else we have a path                  */
  if (this->bbnum && ebbs[this->bbnum - 1]->ech && ebbs[this->bbnum - 1]->ech->op == GOTO)
    return 0;
  else
    return 1;
}


/*-----------------------------------------------------------------*/
/* freeBBlockData - Deallocate data structures associated with     */
/*      the current blocks. They will all be recomputed if the     */
/*      iCode chain is divided into blocks again later.            */
/*-----------------------------------------------------------------*/
void
freeeBBlockData(ebbIndex * ebbi)
{
  int i;
  eBBlock ** ebbs = ebbi->bbOrder;
  
  for (i=0; i < ebbi->count; i++)
    {
      deleteSet (&ebbs[i]->succList);
      deleteSet (&ebbs[i]->predList);
      freeBitVect (ebbs[i]->succVect);
      freeBitVect (ebbs[i]->domVect);
      
      freeCSEdata(ebbs[i]);
    }
}

