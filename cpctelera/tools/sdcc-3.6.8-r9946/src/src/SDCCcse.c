/*-------------------------------------------------------------------------
  SDCCcse.c - source file for Common Subexpressions and other utility

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
#include "dbuf_string.h"


/*-----------------------------------------------------------------*/
/* newCseDef - new cseDef                                          */
/*-----------------------------------------------------------------*/
cseDef *
newCseDef (operand * sym, iCode * ic)
{
  cseDef *cdp;
  memmap *map;

  assert (sym);
  cdp = Safe_alloc (sizeof (cseDef));

  cdp->sym = sym;
  cdp->diCode = ic;
  cdp->key = sym->key;
  cdp->ancestors = newBitVect(iCodeKey);
  cdp->fromGlobal = 0;
  cdp->fromAddrTaken = 0;

  if (ic->op != IF && ic->op != JUMPTABLE)
    {
      if (ic->op != ADDRESS_OF && IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)))
        {
          bitVectSetBit (cdp->ancestors, IC_LEFT (ic)->key);
          if (isOperandGlobal (IC_LEFT (ic)))
            {
              map = SPEC_OCLS (getSpec (operandType (IC_LEFT (ic))));
              if (map)
                cdp->fromGlobal |= (1 << map->ptrType);
            }
          cdp->fromAddrTaken |= OP_SYMBOL (IC_LEFT (ic))->addrtaken;
        }
      if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)))
        {
          bitVectSetBit (cdp->ancestors, IC_RIGHT (ic)->key);
          if (isOperandGlobal (IC_RIGHT (ic)))
            {
              map = SPEC_OCLS (getSpec (operandType (IC_RIGHT (ic))));
              if (map)
                cdp->fromGlobal |= (1 << map->ptrType);
            }
          cdp->fromAddrTaken |= OP_SYMBOL (IC_RIGHT (ic))->addrtaken;
        }
    }

  return cdp;
}

void
updateCseDefAncestors(cseDef *cdp, set * cseSet)
{
  cseDef *loop;
  set *sl;
  iCode *ic = cdp->diCode;

  if (ic->op!=IF && ic->op!=JUMPTABLE)
    {
      if (ic->op != ADDRESS_OF && IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)))
        {
          bitVectSetBit (cdp->ancestors, IC_LEFT (ic)->key);
          for (sl = cseSet; sl; sl = sl->next)
            {
              loop = sl->item;
              if (loop->sym->key == IC_LEFT (ic)->key)
                {
                  cdp->ancestors = bitVectUnion (cdp->ancestors, loop->ancestors);
                  cdp->fromGlobal |= loop->fromGlobal;
                  cdp->fromAddrTaken |= loop->fromAddrTaken;
                  break;
                }
            }
        }
      if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)))
        {
          bitVectSetBit (cdp->ancestors, IC_RIGHT (ic)->key);
          for (sl = cseSet; sl; sl = sl->next)
            {
              loop = sl->item;
              if (loop->sym->key == IC_RIGHT (ic)->key)
                {
                  cdp->ancestors = bitVectUnion (cdp->ancestors, loop->ancestors);
                  cdp->fromGlobal |= loop->fromGlobal;
                  cdp->fromAddrTaken |= loop->fromAddrTaken;
                  break;
                }
            }
        }
    }
}


/*-----------------------------------------------------------------*/
/* int isCseDefEqual - two definitions are equal                   */
/*-----------------------------------------------------------------*/
int
isCseDefEqual (void *vsrc, void *vdest)
{
  cseDef *src = vsrc;
  cseDef *dest = vdest;

  if (src == dest)
    return 1;

  return (src->key == dest->key &&
          src->diCode == dest->diCode);
}

/*-----------------------------------------------------------------*/
/* pcseDef - in the cseDef                                         */
/*-----------------------------------------------------------------*/
int
pcseDef (void *item, va_list ap)
{
  cseDef *cdp = item;
  iCodeTable *icTab;
  struct dbuf_s dbuf;

  (void) ap;

  if (!cdp->sym)
    fprintf (stdout, "**null op**");
  printOperand (cdp->sym, stdout);
  icTab = getTableEntry (cdp->diCode->op);
  dbuf_init (&dbuf, 1024);
  icTab->iCodePrint (&dbuf, cdp->diCode, icTab->printName);
  dbuf_write_and_destroy (&dbuf, stdout);
  return 1;
}

void ReplaceOpWithCheaperOp(operand **op, operand *cop) {
#ifdef RANGEHUNT
  printf ("ReplaceOpWithCheaperOp\n\t");
  printOperand (*op, stdout);
  printf ("\nwith\t");
  printOperand (cop, stdout);

  // if op is a register equivalent
  if (IS_ITEMP(cop) && IS_SYMOP((*op)) && OP_SYMBOL((*op))->isreqv) {
    operand **rop = &OP_SYMBOL((*op))->usl.spillLoc->reqv;
    if (isOperandEqual(*rop, *op)) {
      printf ("true");
      *rop=cop;
      OP_SYMBOL((*op))->isreqv=0;
      OP_SYMBOL(cop)->isreqv=1;
    } else {
      printf ("false");
    }
  }
  printf ("\n");
#endif
  *op=cop;
}

/*-----------------------------------------------------------------*/
/* replaceAllSymBySym - replaces all operands by operand in an     */
/*                      instruction chain                          */
/*-----------------------------------------------------------------*/
void
replaceAllSymBySym (iCode * ic, operand * from, operand * to, bitVect ** ndpset)
{
  iCode *lic;

#ifdef RANGEHUNT
  printf ("replaceAllSymBySym\n\t");
  printOperand (from, stdout);
  printf ("\nwith\t");
  printOperand (to, stdout);
  printf ("\n");
#endif
  for (lic = ic; lic; lic = lic->next)
    {
      int isaddr;

      /* do the special cases first */
      if (lic->op == IFX)
        {
          if (IS_SYMOP (to) &&
              IC_COND (lic)->key == from->key)
            {

              bitVectUnSetBit (OP_USES (from), lic->key);
              OP_USES(to)=bitVectSetBit (OP_USES (to), lic->key);
              isaddr = IC_COND (lic)->isaddr;
              IC_COND (lic) = operandFromOperand (to);
              IC_COND (lic)->isaddr = isaddr;

            }
          continue;
        }

      if (lic->op == JUMPTABLE)
        {
          if (IS_SYMOP (to) &&
              IC_JTCOND (lic)->key == from->key)
            {

              bitVectUnSetBit (OP_USES (from), lic->key);
              OP_USES(to)=bitVectSetBit (OP_USES (to), lic->key);
              isaddr = IC_COND (lic)->isaddr;
              IC_JTCOND (lic) = operandFromOperand (to);
              IC_JTCOND (lic)->isaddr = isaddr;

            }
          continue;
        }

      if (IS_SYMOP(to) &&
          IC_RESULT (lic) && IC_RESULT (lic)->key == from->key)
        {
          /* maintain du chains */
          if (POINTER_SET (lic))
            {
              bitVectUnSetBit (OP_USES (from), lic->key);
              OP_USES(to)=bitVectSetBit (OP_USES (to), lic->key);

              /* also check if the "from" was in the non-dominating
                 pointer sets and replace it with "to" in the bitVector */
              if (bitVectBitValue (*ndpset, from->key))
                {
                  bitVectUnSetBit (*ndpset, from->key);
                  bitVectSetBit (*ndpset, to->key);
                }

            }
          else
            {
              bitVectUnSetBit (OP_DEFS (from), lic->key);
              OP_DEFS(to)=bitVectSetBit (OP_DEFS (to), lic->key);
            }
          isaddr = IC_RESULT (lic)->isaddr;
          IC_RESULT (lic) = operandFromOperand (to);
          IC_RESULT (lic)->isaddr = isaddr;
        }

      if (IS_SYMOP (to) &&
          IC_RIGHT (lic) && IC_RIGHT (lic)->key == from->key)
        {
          bitVectUnSetBit (OP_USES (from), lic->key);
          OP_USES(to)=bitVectSetBit (OP_USES (to), lic->key);
          isaddr = IC_RIGHT (lic)->isaddr;
          IC_RIGHT (lic) = operandFromOperand (to);
          IC_RIGHT (lic)->isaddr = isaddr;
        }

      if (IS_SYMOP (to) &&
          IC_LEFT (lic) && IC_LEFT (lic)->key == from->key)
        {
          bitVectUnSetBit (OP_USES (from), lic->key);
          OP_USES(to)=bitVectSetBit (OP_USES (to), lic->key);
          isaddr = IC_LEFT (lic)->isaddr;
          IC_LEFT (lic) = operandFromOperand (to);
          IC_LEFT (lic)->isaddr = isaddr;
        }
    }
}

/*-----------------------------------------------------------------*/
/* iCodeKeyIs - if the icode keys match then return 1              */
/*-----------------------------------------------------------------*/
DEFSETFUNC (iCodeKeyIs)
{
  cseDef *cdp = item;
  V_ARG (int, key);

  if (cdp->diCode->key == key)
    return 1;
  else
    return 0;
}

/*-----------------------------------------------------------------*/
/* removeFromInExprs - removes an icode from inexpressions         */
/*-----------------------------------------------------------------*/
DEFSETFUNC (removeFromInExprs)
{
  eBBlock *ebp = item;
  V_ARG (iCode *, ic);
  V_ARG (operand *, from);
  V_ARG (operand *, to);
  V_ARG (eBBlock *, cbp);

  if (ebp->visited)
    return 0;

  ebp->visited = 1;
  deleteItemIf (&ebp->inExprs, iCodeKeyIs, ic->key);
  if (ebp != cbp && !bitVectBitValue (cbp->domVect, ebp->bbnum))
    replaceAllSymBySym (ebp->sch, from, to, &ebp->ndompset);

  applyToSet (ebp->succList, removeFromInExprs, ic, from, to, cbp);
  return 0;
}

/*-----------------------------------------------------------------*/
/* isGlobalInNearSpace - return TRUE if variable is a globalin data */
/*-----------------------------------------------------------------*/
static bool
isGlobalInNearSpace (operand * op)
{
  sym_link *type = getSpec (operandType (op));
  /* this is 8051 specific: optimization
     suggested by Jean-Louis VERN, with 8051s we have no
     advantage of putting variables in near space into
     registers */
  if (isOperandGlobal (op) && !IN_FARSPACE (SPEC_OCLS (type)) &&
      IN_DIRSPACE (SPEC_OCLS (type)))
    return TRUE;
  else
    return FALSE;
}

/*-----------------------------------------------------------------*/
/* findCheaperOp - cseBBlock support routine, will check to see if */
/*              we have a operand previously defined               */
/*-----------------------------------------------------------------*/
DEFSETFUNC (findCheaperOp)
{
  cseDef *cdp = item;
  V_ARG (operand *, cop);
  V_ARG (operand **, opp);
  V_ARG (int, checkSign);

  /* if we have already found it */
  if (*opp)
    return 1;

  /* not found it yet check if this is the one */
  /* and this is not the defining one          */
  if (cop->key && cop->key == cdp->key)
    {
      /* do a special check this will help in */
      /* constant propagation & dead code elim */
      /* for assignments only                 */
      if (cdp->diCode->op == '=')
        {
          /* if the result is volatile then return result */
          if (IS_OP_VOLATILE (IC_RESULT (cdp->diCode)))
            {
              *opp = IC_RESULT (cdp->diCode);
            }
          else
            {
              /* if this is a straight assignment and
                 left is a temp then prefer the temporary to the
                 true symbol */
              if (!POINTER_SET (cdp->diCode) &&
                  IS_ITEMP (IC_RESULT (cdp->diCode)) &&
                  IS_TRUE_SYMOP (IC_RIGHT (cdp->diCode)))
                {
                  *opp = IC_RESULT (cdp->diCode);
                }
              else
                {
                  /* if straight assignment and both
                     are temps then prefer the one that
                     will not need extra space to spil, also
                     take into consideration if right side
                     is an induction variable
                  */
                  if (!POINTER_SET (cdp->diCode) &&
                      IS_ITEMP (IC_RESULT (cdp->diCode)) &&
                      IS_ITEMP (IC_RIGHT (cdp->diCode)) &&
                      !OP_SYMBOL (IC_RIGHT (cdp->diCode))->isind &&
                      !OP_SYMBOL(IC_RIGHT (cdp->diCode))->isreqv &&
                      ((!SPIL_LOC (IC_RIGHT (cdp->diCode)) && SPIL_LOC (IC_RESULT (cdp->diCode))) ||
                       (SPIL_LOC (IC_RESULT (cdp->diCode)) && SPIL_LOC (IC_RESULT (cdp->diCode)) == SPIL_LOC (IC_RIGHT (cdp->diCode)))))
                    {
                      *opp = IC_RESULT (cdp->diCode);
                    }
                  else
                    {
                      *opp = IC_RIGHT (cdp->diCode);
                    }
                }
            }
        }
      else
        {
          *opp = IC_RESULT (cdp->diCode);
        }
    }

  /* if this is an assign to a temp. then check
     if the right side is this then return this */
  if (IS_TRUE_SYMOP (cop) &&
      cdp->diCode->op == '=' &&
      !POINTER_SET (cdp->diCode) &&
      cop->key == IC_RIGHT (cdp->diCode)->key &&
      !isGlobalInNearSpace (IC_RIGHT (cdp->diCode)) &&
      IS_ITEMP (IC_RESULT (cdp->diCode)))
    {
      *opp = IC_RESULT (cdp->diCode);
    }

  if ((*opp) &&
      (isOperandLiteral(*opp) || !checkSign ||
       (checkSign &&
        IS_SPEC(operandType (cop)) && IS_SPEC(operandType (*opp)) &&
        (SPEC_USIGN(operandType (cop))==SPEC_USIGN(operandType (*opp)) &&
         (SPEC_LONG(operandType (cop))==SPEC_LONG(operandType (*opp)))))))
    {
      if ((isGlobalInNearSpace (cop) && !isOperandLiteral (*opp)) ||
          isOperandVolatile (*opp, FALSE))
        {
          *opp = NULL;
          return 0;
        }

      if (cop->key == (*opp)->key)
        {
          *opp = NULL;
          return 0;
        }

      if ((*opp)->isaddr != cop->isaddr && IS_ITEMP (cop))
        {
          *opp = operandFromOperand (*opp);
          (*opp)->isaddr = cop->isaddr;
        }

      /* copy signedness to literal operands */
      if (IS_SPEC(operandType (cop)) && IS_SPEC(operandType (*opp))
          && isOperandLiteral(*opp)
          && SPEC_NOUN(operandType(*opp)) == SPEC_NOUN(operandType(cop))
          && SPEC_USIGN(operandType(*opp)) != SPEC_USIGN(operandType(cop)))
        {
          SPEC_USIGN(operandType(*opp)) = SPEC_USIGN(operandType(cop));
        }

      if (IS_SPEC(operandType (cop)) && IS_SPEC(operandType (*opp)) &&
          SPEC_NOUN(operandType(cop)) != SPEC_NOUN(operandType(*opp)))
        {
          // special case: we can make an unsigned char literal
          // into an int literal with no cost.
          if (isOperandLiteral(*opp) &&
              SPEC_NOUN(operandType(*opp)) == V_CHAR &&
              SPEC_NOUN(operandType(cop)) == V_INT)
            {
              *opp = operandFromOperand (*opp);
              SPEC_NOUN(operandType(*opp)) = V_INT;
            }
          else
            {
              // No clue...
              *opp = NULL;
              return 0;
            }
        }
      return 1;
    }
  *opp=NULL;
  return 0;
}

/*-----------------------------------------------------------------*/
/* findPointerSet - finds the right side of a pointer set op       */
/*-----------------------------------------------------------------*/
DEFSETFUNC (findPointerSet)
{
  cseDef *cdp = item;
  V_ARG (operand *, op);
  V_ARG (operand **, opp);
  V_ARG (operand *, rop);

  if (POINTER_SET (cdp->diCode) &&
      op->key &&
      IC_RESULT (cdp->diCode)->key == op->key &&
      !isOperandVolatile (IC_RESULT (cdp->diCode), TRUE) &&
      !isOperandVolatile (IC_RIGHT (cdp->diCode), TRUE) &&
      getSize (operandType (IC_RIGHT (cdp->diCode))) ==
      getSize (operandType (rop)))
    {
      if (IS_SPEC (operandType (IC_RIGHT (cdp->diCode))) &&
          SPEC_USIGN (operandType (IC_RIGHT (cdp->diCode))) !=
          SPEC_USIGN (operandType (rop)))
        {
          /* bug #1493710
            Reminder for Bernhard: check of signedness
            could be unnecessary together with 'checkSign', if
            signedness of operation is stored in ic */
          return 0;
        }
      *opp = IC_RIGHT (cdp->diCode);
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* findPrevIc - cseBBlock support function will return the iCode   */
/*              which matches the current one                      */
/*-----------------------------------------------------------------*/
DEFSETFUNC (findPrevIc)
{
  cseDef *cdp = item;
  V_ARG (iCode *, ic);
  V_ARG (iCode **, icp);

  /* if already found */
  if (*icp)
    return 1;

  /* if the iCodes are the same */
  if (isiCodeEqual (ic, cdp->diCode) &&
      isOperandEqual (cdp->sym, IC_RESULT (cdp->diCode)))
    {
      *icp = cdp->diCode;
      return 1;
    }

  /* if iCodes are not the same */
  /* see the operands maybe interchanged */
  if (ic->op == cdp->diCode->op &&
      IS_COMMUTATIVE (ic) &&
      isOperandEqual (IC_LEFT (ic), IC_RIGHT (cdp->diCode)) &&
      isOperandEqual (IC_RIGHT (ic), IC_LEFT (cdp->diCode)))
    {
      *icp = cdp->diCode;
      return 1;
    }

  return 0;
}

/*-------------------------------------------------------------------*/
/* ifAssignedFromGlobal - if definition is an assignment from global */
/*-------------------------------------------------------------------*/
DEFSETFUNC (ifAssignedFromGlobal)
{
  cseDef *cdp = item;
  iCode *dic=cdp->diCode;

  if (dic->op=='=' && isOperandGlobal(IC_RIGHT(dic))) {
    return 1;
  }
  return 0;
}

/*-------------------------------------------------------------------*/
/* ifFromGlobal - if definition is derived from global               */
/*-------------------------------------------------------------------*/
DEFSETFUNC (ifFromGlobal)
{
  cseDef *cdp = item;

  return cdp->fromGlobal;
}

/*-------------------------------------------------------------------*/
/* ifFromGlobalAliasableByPtr - if definition is derived from global */
/*   that may be aliasble by a particular pointer type               */
/*-------------------------------------------------------------------*/
DEFSETFUNC (ifFromGlobalAliasableByPtr)
{
  cseDef *cdp = item;
  V_ARG (DECLARATOR_TYPE, decl);

  if (decl == GPOINTER && cdp->fromGlobal)
    return 1;
  else if (cdp->fromGlobal & (1 << decl))
    return 1;
  else
    return 0;
}

/*-----------------------------------------------------------------*/
/* ifDefGlobal - if definition is global                           */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifDefGlobal)
{
  cseDef *cdp = item;

  return (isOperandGlobal (cdp->sym));
}

/*-----------------------------------------------------------------*/
/* ifDefGlobalAliasableByPtr - if definition is global             */
/*   and may be aliasble by a particular pointer type              */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifDefGlobalAliasableByPtr)
{
  cseDef *cdp = item;
  V_ARG (DECLARATOR_TYPE, decl);
  memmap *map;

  if (!isOperandGlobal (cdp->sym))
    return 0;
  if (decl == GPOINTER)
    return 1;
  map = SPEC_OCLS (getSpec (operandType (cdp->sym)));
  return (map->ptrType == decl);
}

/*-------------------------------------------------------------------*/
/* ifFromAddrTaken - if definition is derived from a symbol whose    */
/*                   address was taken                               */
/*-------------------------------------------------------------------*/
DEFSETFUNC (ifFromAddrTaken)
{
  cseDef *cdp = item;

  return cdp->fromAddrTaken;
}


/*-----------------------------------------------------------------*/
/* ifAnyGetPointer - if get pointer icode                          */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifAnyGetPointer)
{
  cseDef *cdp = item;

  if (cdp->diCode && POINTER_GET (cdp->diCode))
    return 1;
  return 0;
}

/*-----------------------------------------------------------------*/
/* ifAnyUnrestrictedGetPointer - if get pointer icode              */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifAnyUnrestrictedGetPointer)
{
  cseDef *cdp = item;
  V_ARG (DECLARATOR_TYPE, decl);

  if (cdp->diCode && POINTER_GET (cdp->diCode))
    {
      sym_link *ptype;
      ptype = operandType (IC_LEFT (cdp->diCode));
      if (!IS_PTR_RESTRICT (ptype))
        {
	  if (DCL_TYPE (ptype) == decl || IS_GENPTR (ptype))
            return 1;
	}
    }
  return 0;
}

/*-----------------------------------------------------------------*/
/* ifAnyUnrestrictedSetPointer - if set pointer icode              */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifAnyUnrestrictedSetPointer)
{
  cseDef *cdp = item;
  V_ARG (DECLARATOR_TYPE, decl);

  if (cdp->diCode && POINTER_SET (cdp->diCode))
    {
      sym_link *ptype;
      ptype = operandType (IC_RESULT (cdp->diCode));
      if (!IS_PTR_RESTRICT (ptype))
        {
	  if (DCL_TYPE (ptype) == decl || IS_GENPTR (ptype))
            return 1;
	}
    }
  return 0;
}

/*-----------------------------------------------------------------*/
/* ifOperandsHave - if any of the operand are the same as this     */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifOperandsHave)
{
  cseDef *cdp = item;
  V_ARG (operand *, op);

  if (bitVectBitValue(cdp->ancestors, op->key))
    return 1;

  if (IC_LEFT (cdp->diCode) &&
      IS_SYMOP (IC_LEFT (cdp->diCode)) &&
      IC_LEFT (cdp->diCode)->key == op->key)
    return 1;

  if (IC_RIGHT (cdp->diCode) &&
      IS_SYMOP (IC_RIGHT (cdp->diCode)) &&
      IC_RIGHT (cdp->diCode)->key == op->key)
    return 1;

  if (POINTER_SET (cdp->diCode) &&
      IS_SYMOP (IC_RESULT (cdp->diCode)) &&
      IC_RESULT (cdp->diCode)->key == op->key)
    return 1;

  /* or if any of the operands are volatile */
  if (IC_LEFT (cdp->diCode) &&
      IS_OP_VOLATILE (IC_LEFT (cdp->diCode)))
    return 1;

  if (IC_RIGHT (cdp->diCode) &&
      IS_OP_VOLATILE (IC_RIGHT (cdp->diCode)))
    return 1;


  if (IC_RESULT (cdp->diCode) &&
      IS_OP_VOLATILE (IC_RESULT (cdp->diCode)))
    return 1;

  return 0;
}

/*-----------------------------------------------------------------*/
/* ifDefSymIs - if a definition is found in the set                */
/*-----------------------------------------------------------------*/
int
ifDefSymIs (set * cseSet, operand * sym)
{
  cseDef *loop;
  set *sl;

  if (!sym || !IS_SYMOP (sym))
    return 0;
  for (sl = cseSet; sl; sl = sl->next)
    {
      loop = sl->item;
      if (loop->sym->key == sym->key)
        return 1;
    }
  return 0;
}


/*-----------------------------------------------------------------*/
/* ifDefSymIsX - will return 1 if the symbols match                */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifDefSymIsX)
{
  cseDef *cdp = item;
  V_ARG (operand *, op);
  int match;

  if (op && cdp->sym && op->key)
    match = cdp->sym->key == op->key;
  else
    match = (isOperandEqual (cdp->sym, op));
  #if 0
  if (match)
    printf("%s ",OP_SYMBOL(cdp->sym)->name);
  #endif
  return match;
}


/*-----------------------------------------------------------------*/
/* ifDiCodeIs - returns true if diCode is same                     */
/*-----------------------------------------------------------------*/
int
ifDiCodeIs (set * cseSet, iCode * ic)
{
  cseDef *loop;
  set *sl;

  if (!ic)
    return 0;

  for (sl = cseSet; sl; sl = sl->next)
    {
      loop = sl->item;
      if (loop->diCode == ic)
        return 1;
    }
  return 0;

}

/*-----------------------------------------------------------------*/
/* ifPointerGet - returns true if the icode is pointer get sym     */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifPointerGet)
{
  cseDef *cdp = item;
  V_ARG (operand *, op);
  iCode *dic = cdp->diCode;
  operand *left = IC_LEFT (cdp->diCode);

  if (POINTER_GET (dic) && op->key && left->key == op->key)
    return 1;

  return 0;
}

/*-----------------------------------------------------------------*/
/* ifPointerSet - returns true if the icode is pointer set sym     */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifPointerSet)
{
  cseDef *cdp = item;
  V_ARG (operand *, op);

  if (POINTER_SET (cdp->diCode) && op->key &&
      IC_RESULT (cdp->diCode)->key == op->key)
    return 1;

  return 0;
}

/*-----------------------------------------------------------------*/
/* ifDiCodeIsX - will return 1 if the symbols match                 */
/*-----------------------------------------------------------------*/
DEFSETFUNC (ifDiCodeIsX)
{
  cseDef *cdp = item;
  V_ARG (iCode *, ic);

  return cdp->diCode == ic;

}

/*-----------------------------------------------------------------*/
/* findBackwardDef - scan backwards to find definition of operand  */
/*-----------------------------------------------------------------*/
iCode *findBackwardDef(operand *op,iCode *ic)
{
    iCode *lic;

    for (lic = ic; lic ; lic = lic->prev) {
        if (IC_RESULT(lic) && isOperandEqual(op,IC_RESULT(lic)))
            return lic;
    }
    return NULL;
}

/*-----------------------------------------------------------------*/
/* algebraicOpts - does some algebraic optimizations               */
/*-----------------------------------------------------------------*/
static void
algebraicOpts (iCode * ic, eBBlock * ebp)
{
  /* we don't deal with the following iCodes
     here */
  if (ic->op == IFX ||
      ic->op == IPUSH ||
      ic->op == IPOP ||
      ic->op == CALL ||
      ic->op == PCALL ||
      ic->op == RETURN ||
      POINTER_GET (ic))
    return;

  /* if both operands present & ! IFX */
  /* then if they are both literal we */
  /* perform the operation right now  */
  if (IC_RESULT (ic) &&
      IC_RIGHT (ic) &&
      IC_LEFT (ic) &&
      IS_OP_LITERAL (IC_LEFT (ic)) &&
      IS_OP_LITERAL (IC_RIGHT (ic)))
    {
      IC_RIGHT (ic) = operandOperation (IC_LEFT (ic),
                                        IC_RIGHT (ic),
                                        ic->op,
                                        operandType (IC_RESULT (ic)));
      ic->op = '=';
      IC_LEFT (ic) = NULL;
      SET_RESULT_RIGHT (ic);
      return;
    }
  /* if not ifx & only one operand present */
  if (IC_RESULT (ic) &&
      IC_LEFT (ic) &&
      IS_OP_LITERAL (IC_LEFT (ic)) &&
      !IC_RIGHT (ic))
    {
      IC_RIGHT (ic) = operandOperation (IC_LEFT (ic),
                                        IC_RIGHT (ic),
                                        ic->op,
                                        operandType (IC_RESULT (ic)));
      ic->op = '=';
      IC_LEFT (ic) = NULL;
      SET_RESULT_RIGHT (ic);
      return;
    }

  /* a special case : or in short a kludgy solution will think
     about a better solution over a glass of wine someday */
  if (ic->op == GET_VALUE_AT_ADDRESS)
    {
      if (IS_ITEMP (IC_RESULT (ic)) &&
          IS_TRUE_SYMOP (IC_LEFT (ic)))
        {
          ic->op = '=';
          IC_RIGHT (ic) = operandFromOperand (IC_LEFT (ic));
          IC_RIGHT (ic)->isaddr = 0;
          IC_LEFT (ic) = NULL;
          IC_RESULT (ic) = operandFromOperand (IC_RESULT (ic));
          IC_RESULT (ic)->isaddr = 0;
          setOperandType (IC_RESULT (ic), operandType (IC_RIGHT (ic)));
          if (IS_DECL (operandType (IC_RESULT (ic))))
            {
              DCL_PTR_VOLATILE (operandType (IC_RESULT (ic))) = 0;
              DCL_PTR_ADDRSPACE (operandType (IC_RESULT (ic))) = 0;
            }
          return;
        }

      if (IS_ITEMP (IC_LEFT (ic)) &&
          IS_ITEMP (IC_RESULT (ic)) &&
          !IC_LEFT (ic)->isaddr)
        {
          ic->op = '=';
          IC_RIGHT (ic) = operandFromOperand (IC_LEFT (ic));
          IC_RIGHT (ic)->isaddr = 0;
          IC_RESULT (ic) = operandFromOperand (IC_RESULT (ic));
          IC_RESULT (ic)->isaddr = 0;
          IC_LEFT (ic) = NULL;
          return;
        }
    }

  /* depending on the operation */
  switch (ic->op)
    {
    case '+':
      /* if adding the same thing change to left shift by 1 */
      if (IC_LEFT (ic)->key == IC_RIGHT (ic)->key &&
          !IS_OP_VOLATILE (IC_LEFT (ic)) &&
          !(IS_FLOAT (operandType (IC_RESULT (ic)))
            || IS_FIXED(operandType (IC_RESULT (ic)))))
        {
          ic->op = LEFT_OP;
          IC_RIGHT (ic) = operandFromLit (1);
          return;
        }
      /* if addition then check if one of them is a zero */
      /* if yes turn it into assignment or cast */
      if (IS_OP_LITERAL (IC_LEFT (ic)) &&
          isEqualVal (OP_VALUE (IC_LEFT (ic)), 0))
        {
          int typematch;
          typematch = compareType (operandType (IC_RESULT (ic)),
                                   operandType (IC_RIGHT (ic)));
          if ((typematch<0) || (IS_TRUE_SYMOP (IC_RIGHT (ic))))
            {
              ic->op = CAST;
              IC_LEFT (ic) = operandFromLink (operandType (IC_RESULT (ic)));
            }
          else
            {
              ic->op = '=';
              IC_LEFT (ic) = NULL;
              if (typematch==0)
                {
                  /* for completely different types, preserve the source type */
                  IC_RIGHT (ic) = operandFromOperand (IC_RIGHT (ic));
                  setOperandType (IC_RIGHT (ic), operandType (IC_RESULT (ic)));
                }
            }
          SET_ISADDR (IC_RESULT (ic), 0);
          SET_ISADDR (IC_RIGHT (ic), 0);
          return;
        }
      if (IS_OP_LITERAL (IC_RIGHT (ic)) &&
          isEqualVal (OP_VALUE (IC_RIGHT (ic)), 0))
        {
          int typematch;
          typematch = compareType (operandType (IC_RESULT (ic)),
                                   operandType (IC_LEFT (ic)));
          if ((typematch<0) || (IS_TRUE_SYMOP (IC_LEFT (ic))))
            {
              ic->op = CAST;
              IC_RIGHT (ic) = IC_LEFT (ic);
              IC_LEFT (ic) = operandFromLink (operandType (IC_RESULT (ic)));
            }
          else
            {
              ic->op = '=';
              IC_RIGHT (ic) = IC_LEFT (ic);
              IC_LEFT (ic) = NULL;
              if (typematch==0)
                {
                  /* for completely different types, preserve the source type */
                  IC_RIGHT (ic) = operandFromOperand (IC_RIGHT (ic));
                  setOperandType (IC_RIGHT (ic), operandType (IC_RESULT (ic)));
                }
            }
          SET_ISADDR (IC_RIGHT (ic), 0);
          SET_ISADDR (IC_RESULT (ic), 0);
          return;
        }
      break;
    case '-':
      /* if subtracting the same thing then zero     */
      if (IC_LEFT (ic)->key == IC_RIGHT (ic)->key &&
        !IS_OP_VOLATILE (IC_LEFT (ic)))
        {printf("Sub. at %d\n", ic->key);
          ic->op = '=';
          IC_RIGHT (ic) = operandFromLit (0);
          IC_LEFT (ic) = NULL;
          IC_RESULT (ic) = operandFromOperand (IC_RESULT (ic));
          IC_RESULT (ic)->isaddr = 0;
          return;
        }

      /* if subtraction then check if one of the operand */
      /* is zero then depending on which operand change  */
      /* to assignment or unary minus                    */
      if (IS_OP_LITERAL (IC_RIGHT (ic)) &&
          isEqualVal (OP_VALUE (IC_RIGHT (ic)), 0))
        {
          /* right size zero change to assignment */
          ic->op = '=';
          IC_RIGHT (ic) = IC_LEFT (ic);
          IC_LEFT (ic) = NULL;
          SET_ISADDR (IC_RIGHT (ic), 0);
          SET_ISADDR (IC_RESULT (ic), 0);
          return;
        }
      if (IS_OP_LITERAL (IC_LEFT (ic)) &&
          isEqualVal (OP_VALUE (IC_LEFT (ic)), 0))
        {
          /* left zero turn into an unary minus */
          ic->op = UNARYMINUS;
          IC_LEFT (ic) = IC_RIGHT (ic);
          IC_RIGHT (ic) = NULL;
          return;
        }
      break;
      /* if multiplication then check if either of */
      /* them is zero then the result is zero      */
      /* if either of them is one then result is   */
      /* the other one                             */
    case '*':
      if (IS_OP_LITERAL (IC_LEFT (ic)))
        {
          if (isEqualVal (OP_VALUE (IC_LEFT (ic)), 0))
            {
              ic->op = '=';
              IC_RIGHT (ic) = IC_LEFT (ic);
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }
          if (isEqualVal (OP_VALUE (IC_LEFT (ic)), 1))
            {
              /* '*' can have two unsigned chars as operands */
              /* and an unsigned int as result.              */
              if (compareType (operandType (IC_RESULT (ic)),
                               operandType (IC_RIGHT (ic))) == 1)
                {
                  ic->op = '=';
                  IC_LEFT (ic) = NULL;
                  SET_RESULT_RIGHT (ic);
                }
              else
                {
                  ic->op = CAST;
                  IC_LEFT (ic) = operandFromOperand (IC_LEFT (ic));
                  IC_LEFT (ic)->type = TYPE;
                  IC_LEFT (ic)->isLiteral = 0;
                  setOperandType (IC_LEFT (ic), operandType (IC_RESULT (ic)));
                }
              return;
            }
          if (isEqualVal (OP_VALUE (IC_LEFT (ic)), -1))
            {
              /* convert -1 * x to -x */
              ic->op = UNARYMINUS;
              IC_LEFT (ic) = IC_RIGHT (ic);
              IC_RIGHT (ic) = NULL;
              return;
            }
        }

      if (IS_OP_LITERAL (IC_RIGHT (ic)))
        {
          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), 0))
            {
              ic->op = '=';
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }

          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), 1))
            {
              /* '*' can have two unsigned chars as operands */
              /* and an unsigned int as result.              */
              if (compareType (operandType (IC_RESULT (ic)),
                               operandType (IC_LEFT (ic))) == 1)
                {
                  ic->op = '=';
                  IC_RIGHT (ic) = IC_LEFT (ic);
                  IC_LEFT (ic) = NULL;
                  SET_RESULT_RIGHT (ic);
                }
              else
                {
                  operand *op;

                  ic->op = CAST;
                  op = IC_RIGHT (ic);
                  IC_RIGHT (ic) = IC_LEFT (ic);
                  IC_LEFT (ic) = operandFromOperand (op);
                  IC_LEFT (ic)->type = TYPE;
                  IC_LEFT (ic)->isLiteral = 0;
                  setOperandType (IC_LEFT (ic), operandType (IC_RESULT (ic)));
                }
              return;
            }
          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), -1))
            {
              /* '*' can have two unsigned chars as operands */
              /* and an unsigned int as result.              */
              if (IS_INTEGRAL (operandType (IC_LEFT (ic))))
                {
                  if ((getSize (operandType (IC_LEFT (ic))) < (unsigned int) INTSIZE) &&
                      (getSize (operandType (IC_LEFT (ic))) < getSize (operandType (IC_RESULT (ic)))))
                    {
                      operand * op;
                      iCode * newic;
                      /* Widen to int. */
                      op = operandFromOperand (IC_RESULT (ic));
                      op->type = TYPE;
                      setOperandType (op, INTTYPE);
                      newic = newiCode (CAST, op, IC_LEFT (ic));
                      IC_RESULT (newic) = newiTempOperand (INTTYPE, TRUE);
                      addiCodeToeBBlock (ebp, newic, ic);
                      IC_LEFT (ic) = IC_RESULT (newic);
                    }
                }
              /* convert x * -1 to -x */
              ic->op = UNARYMINUS;
              IC_RIGHT (ic) = NULL;
              return;
            }
        }
      break;
    case '/':
      /* if division by self then 1 */
      if (IC_LEFT (ic)->key == IC_RIGHT (ic)->key)
        {
          ic->op = '=';
          IC_RIGHT (ic) = operandFromLit (1);
          IC_LEFT (ic) = NULL;
          IC_RESULT (ic) = operandFromOperand (IC_RESULT (ic));
          IC_RESULT (ic)->isaddr = 0;
          return;
        }
      /* if this is a division then check if left is zero */
      /* then change it to an assignment */
      if (IS_OP_LITERAL (IC_LEFT (ic)) && isEqualVal (OP_VALUE (IC_LEFT (ic)), 0))
        {
          ic->op = '=';
          IC_RIGHT (ic) = IC_LEFT (ic);
          IC_LEFT (ic) = NULL;
          SET_RESULT_RIGHT (ic);
          return;
        }
      /* if this is a division then check if right */
      /* is one then change it to an assignment    */
      if (IS_OP_LITERAL (IC_RIGHT (ic)))
        {
          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), 1))
            {
              ic->op = '=';
              IC_RIGHT (ic) = IC_LEFT (ic);
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }
          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), -1))
            {
              /* '/' can have two unsigned chars as operands */
              /* and an unsigned int as result.              */
              if (IS_INTEGRAL (operandType (IC_LEFT (ic))))
                {
                  if ((getSize (operandType (IC_LEFT (ic))) < (unsigned int) INTSIZE) &&
                      (getSize (operandType (IC_LEFT (ic))) < getSize (operandType (IC_RESULT (ic)))))
                    {
                      operand * op;
                      iCode * newic;
                      /* Widen to int. */
                      op = operandFromOperand (IC_RESULT (ic));
                      op->type = TYPE;
                      setOperandType (op, INTTYPE);
                      newic = newiCode (CAST, op, IC_LEFT (ic));
                      IC_RESULT (newic) = newiTempOperand (INTTYPE, TRUE);
                      addiCodeToeBBlock (ebp, newic, ic);
                      IC_LEFT (ic) = IC_RESULT (newic);
                    }
                }
              /* convert x / -1 to -x */
              ic->op = UNARYMINUS;
              IC_RIGHT (ic) = NULL;
              return;
            }
        }
      break;
      /* if both are the same for an comparison operators */
    case EQ_OP:
    case LE_OP:
    case GE_OP:
      if (isOperandEqual (IC_LEFT (ic), IC_RIGHT (ic)) &&
        !IS_OP_VOLATILE (IC_LEFT (ic)))
        {
          ic->op = '=';
          IC_RIGHT (ic) = operandFromLit (1);
          IC_LEFT (ic) = NULL;
          SET_RESULT_RIGHT (ic);
        }
      break;
    case NE_OP:
    case '>':
    case '<':
      if (isOperandEqual (IC_LEFT (ic), IC_RIGHT (ic)) &&
        !IS_OP_VOLATILE (IC_LEFT (ic)))
        {
          ic->op = '=';
          IC_RIGHT (ic) = operandFromLit (0);
          IC_LEFT (ic) = NULL;
          SET_RESULT_RIGHT (ic);
        }
      break;
    case CAST:
        {
          sym_link *otype = operandType(IC_RIGHT(ic));
          sym_link *ctype = operandType(IC_LEFT(ic));
          /* if this is a cast of a literal value */
          if (IS_OP_LITERAL (IC_RIGHT (ic)))
            {
              double litval = operandLitValue (IC_RIGHT (ic));
              if (IS_GENPTR(ctype) && IS_PTR(otype))
                {
                  unsigned long gpVal = 0;
                  const char *name = IS_SYMOP (IC_RESULT (ic)) ? OP_SYMBOL (IC_RESULT (ic))->name : NULL;
                  if (IS_FUNCPTR(otype))
                    gpVal = pointerTypeToGPByte (DCL_TYPE (otype->next), NULL, name);
                  if (!IS_GENPTR(otype))
                    gpVal = pointerTypeToGPByte (DCL_TYPE (otype), NULL, name);
                  gpVal <<= ((GPTRSIZE - 1) * 8);
                  gpVal |= (unsigned long)litval;
                  litval = gpVal;
                }
              ic->op = '=';
              IC_RIGHT (ic) = operandFromValue (valCastLiteral (operandType (IC_LEFT (ic)), litval, litval));
              IC_LEFT (ic) = NULL;
              SET_ISADDR (IC_RESULT (ic), 0);
            }
          /* if casting to the same */
          if (compareType (operandType (IC_RESULT (ic)), operandType (IC_RIGHT (ic))) == 1)
            {
              ic->op = '=';
              IC_LEFT (ic) = NULL;
              SET_ISADDR (IC_RESULT (ic), 0);
            }
        }
      break;
    case '!':
      if (IS_OP_LITERAL (IC_LEFT (ic)))
        {
          ic->op = '=';
          IC_RIGHT (ic) =
            (isEqualVal (OP_VALUE (IC_LEFT (ic)), 0) ?
             operandFromLit (1) : operandFromLit (0));
          IC_LEFT (ic) = NULL;
          SET_ISADDR (IC_RESULT (ic), 0);
        }
      break;
    case BITWISEAND:
      /* if both operands are equal */
      /* if yes turn it into assignment */
      if (isOperandEqual (IC_LEFT (ic), IC_RIGHT (ic)))
        {
          if (IS_OP_VOLATILE (IC_LEFT (ic)))
            {
              iCode *newic = newiCode (DUMMY_READ_VOLATILE, NULL, IC_LEFT (ic));
              IC_RESULT (newic) = IC_LEFT (ic);
              newic->filename = ic->filename;
              newic->lineno = ic->lineno;
              addiCodeToeBBlock (ebp, newic, ic->next);
            }
          ic->op = '=';
          IC_LEFT (ic) = NULL;
          SET_RESULT_RIGHT (ic);
          return;
        }
      /* swap literal to right ic */
      if (IS_OP_LITERAL (IC_LEFT (ic)))
        {
          operand *op;

          op = IC_LEFT (ic);
          IC_LEFT (ic) = IC_RIGHT (ic);
          IC_RIGHT (ic) = op;
        }
      if (IS_OP_LITERAL (IC_RIGHT (ic)))
        {
          unsigned val;

          /* if BITWISEAND then check if one of them is a zero */
          /* if yes turn it into 0 assignment */
          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), 0))
            {
              if (IS_OP_VOLATILE (IC_LEFT (ic)))
                {
                  iCode *newic = newiCode (DUMMY_READ_VOLATILE, NULL, IC_LEFT (ic));
                  IC_RESULT (newic) = IC_LEFT (ic);
                  newic->filename = ic->filename;
                  newic->lineno = ic->lineno;
                  addiCodeToeBBlock (ebp, newic, ic->next);
                }
              ic->op = '=';
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }
          /* if BITWISEAND then check if one of them is 0xff... */
          /* if yes turn it into assignment */
          if (IS_BOOLEAN (operandType (IC_RIGHT (ic)))) /* Special handling since _Bool is stored in 8 bits */
            goto boolcase;
          switch (bitsForType (operandType (IC_RIGHT (ic))))
            {
            case 1:
            boolcase:
              val = 0x01;
              break;
            case 8:
              val = 0xff;
              break;
            case 16:
              val = 0xffff;
              break;
            case 32:
              val = 0xffffffff;
              break;
            default:
              return;
            }
          if (((unsigned) double2ul (operandLitValue (IC_RIGHT (ic))) & val) == val)
            {
              ic->op = '=';
              IC_RIGHT (ic) = IC_LEFT (ic);
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }
        }
      break;
    case '|':
      /* if both operands are equal */
      /* if yes turn it into assignment */
      if (isOperandEqual (IC_LEFT (ic), IC_RIGHT (ic)))
        {
          if (IS_OP_VOLATILE (IC_LEFT (ic)))
            {
              iCode *newic = newiCode (DUMMY_READ_VOLATILE, NULL, IC_LEFT (ic));
              IC_RESULT (newic) = IC_LEFT (ic);
              newic->filename = ic->filename;
              newic->lineno = ic->lineno;
              addiCodeToeBBlock (ebp, newic, ic->next);
            }
            ic->op = '=';
            IC_LEFT (ic) = NULL;
            SET_RESULT_RIGHT (ic);
            return;
        }
      /* swap literal to right ic */
      if (IS_OP_LITERAL (IC_LEFT (ic)))
        {
          operand *op;

          op = IC_LEFT (ic);
          IC_LEFT (ic) = IC_RIGHT (ic);
          IC_RIGHT (ic) = op;
        }
      if (IS_OP_LITERAL (IC_RIGHT (ic)))
        {
          unsigned val;

          /* if BITWISEOR then check if one of them is a zero */
          /* if yes turn it into assignment */
          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), 0))
            {
              ic->op = '=';
              IC_RIGHT (ic) = IC_LEFT (ic);
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }
          /* if BITWISEOR then check if one of them is 0xff... */
          /* if yes turn it into 0xff... assignment */
          switch (bitsForType (operandType (IC_RIGHT (ic))))
            {
              case 1:
                val = 0x01;
                break;
              case 8:
                val = 0xff;
                break;
              case 16:
                val = 0xffff;
                break;
              case 32:
                val = 0xffffffff;
                break;
              default:
                return;
            }
          if (((unsigned) double2ul (operandLitValue (IC_RIGHT (ic))) & val) == val)
            {
              if (IS_OP_VOLATILE (IC_LEFT (ic)))
                {
                  iCode *newic = newiCode (DUMMY_READ_VOLATILE, NULL, IC_LEFT (ic));
                  IC_RESULT (newic) = IC_LEFT (ic);
                  newic->filename = ic->filename;
                  newic->lineno = ic->lineno;
                  addiCodeToeBBlock (ebp, newic, ic->next);
                }
              ic->op = '=';
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }
        }
      break;
    case '^':
      /* if both operands are equal */
      /* if yes turn it into 0 assignment */
      if (isOperandEqual (IC_LEFT (ic), IC_RIGHT (ic)))
        {
          if (IS_OP_VOLATILE (IC_LEFT (ic)))
            {
              iCode *newic = newiCode (DUMMY_READ_VOLATILE, NULL, IC_LEFT (ic));
              IC_RESULT (newic) = IC_LEFT (ic);
              newic->filename = ic->filename;
              newic->lineno = ic->lineno;
              addiCodeToeBBlock (ebp, newic, ic->next);

              newic = newiCode (DUMMY_READ_VOLATILE, NULL, IC_LEFT (ic));
              IC_RESULT (newic) = IC_LEFT (ic);
              newic->filename = ic->filename;
              newic->lineno = ic->lineno;
              addiCodeToeBBlock (ebp, newic, ic->next);
            }
          ic->op = '=';
          IC_RIGHT (ic) = operandFromLit (0);
          IC_LEFT (ic) = NULL;
          SET_RESULT_RIGHT (ic);
          return;
        }
      /* swap literal to right ic */
      if (IS_OP_LITERAL (IC_LEFT (ic)))
        {
          operand *op;

          op = IC_LEFT (ic);
          IC_LEFT (ic) = IC_RIGHT (ic);
          IC_RIGHT (ic) = op;
        }
      /* if XOR then check if one of them is a zero */
      /* if yes turn it into assignment */
      if (IS_OP_LITERAL (IC_RIGHT (ic)))
        {
          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), 0))
            {
              ic->op = '=';
              IC_RIGHT (ic) = IC_LEFT (ic);
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }
        }
      /* if XOR then check if one of them is a zero or one */
      /* if yes turn it into assignment or invert */
      if (IS_OP_LITERAL (IC_RIGHT (ic)) &&
          IS_BOOLEAN (operandType (IC_LEFT (ic))) &&
          IS_BOOLEAN (operandType (IC_RESULT (ic)))
         )
        {
          if (isEqualVal (OP_VALUE (IC_RIGHT (ic)), 1))
            {
              ic->op = '!';
              IC_RIGHT (ic) = NULL;
              return;
            }
          else
            {
              ic->op = '=';
              IC_RIGHT (ic) = operandFromLit (1);
              IC_LEFT (ic) = NULL;
              SET_RESULT_RIGHT (ic);
              return;
            }
        }
      break;
    }

  return;
}

#define OTHERS_PARM(s) (s->_isparm && !s->ismyparm)
/*-----------------------------------------------------------------*/
/* updateSpillLocation - keeps track of register spill location    */
/*-----------------------------------------------------------------*/
void
updateSpillLocation (iCode * ic, int induction)
{
  sym_link *setype;

  if (POINTER_SET (ic))
    return;

  if (ic->nosupdate)
    return;

#if 0
  /* for the form true_symbol := iTempNN */
  if (ASSIGN_ITEMP_TO_SYM (ic) &&
      !SPIL_LOC (IC_RIGHT (ic)))
    {
      setype = getSpec (operandType (IC_RESULT (ic)));

      if (!OP_SYMBOL(IC_RIGHT (ic))->noSpilLoc &&
          !IS_VOLATILE (setype) &&
          !IN_FARSPACE (SPEC_OCLS (setype)) &&
          !OTHERS_PARM (OP_SYMBOL (IC_RESULT (ic))))
        {
          wassert(IS_SYMOP(IC_RESULT (ic)));
          wassert(IS_SYMOP(IC_RIGHT (ic)));
          SPIL_LOC (IC_RIGHT (ic)) = IC_RESULT (ic)->operand.symOperand;
        }
    }
#endif

#if 0 /* this needs furthur investigation can save a lot of code */
  if (ASSIGN_SYM_TO_ITEMP(ic) &&
      !SPIL_LOC(IC_RESULT(ic)))
    {
      if (!OTHERS_PARM (OP_SYMBOL (IC_RIGHT (ic))))
          SPIL_LOC (IC_RESULT (ic)) = IC_RIGHT (ic)->operand.symOperand;
    }
#endif
  if (ASSIGN_ITEMP_TO_ITEMP (ic))
    {
      if (!SPIL_LOC (IC_RIGHT (ic)) &&
          !bitVectBitsInCommon (OP_DEFS (IC_RIGHT (ic)), OP_USES (IC_RESULT (ic))) &&
          OP_SYMBOL (IC_RESULT (ic))->isreqv)
        {
          setype = getSpec (operandType (IC_RESULT (ic)));

          if (!OP_SYMBOL(IC_RIGHT (ic))->noSpilLoc &&
              !IS_VOLATILE (setype) &&
              !IN_FARSPACE (SPEC_OCLS (setype)) &&
              !OTHERS_PARM (OP_SYMBOL (IC_RESULT (ic))))
            {
              SPIL_LOC (IC_RIGHT (ic)) = SPIL_LOC (IC_RESULT (ic));
              OP_SYMBOL (IC_RIGHT (ic))->prereqv = OP_SYMBOL (IC_RESULT (ic))->prereqv;
            }
        }
      /* special case for inductions */
      if (induction &&
          OP_SYMBOL(IC_RIGHT(ic))->isreqv &&
          !OP_SYMBOL(IC_RESULT (ic))->noSpilLoc &&
          !SPIL_LOC(IC_RESULT(ic)))
        {
          SPIL_LOC (IC_RESULT (ic)) = SPIL_LOC (IC_RIGHT (ic));
          OP_SYMBOL (IC_RESULT (ic))->prereqv = OP_SYMBOL (IC_RIGHT (ic))->prereqv;
        }
    }
}

/*-----------------------------------------------------------------*/
/* setUsesDef - sets the uses def bitvector for a given operand    */
/*-----------------------------------------------------------------*/
void
setUsesDefs (operand * op, bitVect * bdefs,
             bitVect * idefs, bitVect ** oud)
{
  /* compute the definitions alive at this point */
  bitVect *adefs = bitVectUnion (bdefs, idefs);

  /* of these definitions find the ones that are */
  /* for this operand */
  adefs = bitVectIntersect (adefs, OP_DEFS (op));

  /* these are the definitions that this operand can use */
  op->usesDefs = adefs;

  /* the out defs is an union */
  *oud = bitVectUnion (*oud, adefs);
}

/*-----------------------------------------------------------------*/
/* unsetDefsAndUses - clear this operation for the operands        */
/*-----------------------------------------------------------------*/
void
unsetDefsAndUses (iCode * ic)
{
  if (ic->op == JUMPTABLE)
    return;

  /* take away this definition from the def chain of the */
  /* result & take away from use set of the operands */
  if (ic->op != IFX)
    {
      /* turn off def set */
      if (IS_SYMOP (IC_RESULT (ic)))
        {
          if (!POINTER_SET (ic))
            bitVectUnSetBit (OP_DEFS (IC_RESULT (ic)), ic->key);
          else
            bitVectUnSetBit (OP_USES (IC_RESULT (ic)), ic->key);
        }
      /* turn off the useSet for the operands */
      if (IS_SYMOP (IC_LEFT (ic)))
        bitVectUnSetBit (OP_USES (IC_LEFT (ic)), ic->key);

      if (IS_SYMOP (IC_RIGHT (ic)))
        bitVectUnSetBit (OP_USES (IC_RIGHT (ic)), ic->key);
    }
  else
    /* must be ifx turn off the use */ if (IS_SYMOP (IC_COND (ic)))
    bitVectUnSetBit (OP_USES (IC_COND (ic)), ic->key);
}

/*-----------------------------------------------------------------*/
/* ifxOptimize - changes ifx conditions if it can                  */
/*-----------------------------------------------------------------*/
void
ifxOptimize (iCode * ic, set * cseSet,
             int computeOnly,
             eBBlock * ebb, int *change,
             ebbIndex * ebbi)
{
  operand *pdop;
  symbol *label;

  /* if the condition can be replaced */
  if (!computeOnly)
    {
      pdop = NULL;
      applyToSetFTrue (cseSet, findCheaperOp, IC_COND (ic), &pdop, 0);
      if (pdop)
        {
          ReplaceOpWithCheaperOp(&IC_COND (ic), pdop);
          (*change)++;
        }
      else if(ic->prev &&  /* Remove unnecessary casts */
        (ic->prev->op == '=' || ic->prev->op == CAST || ic->prev->op == '!') && IS_ITEMP (IC_RESULT (ic->prev)) &&
        IC_RESULT (ic->prev)->key == IC_COND (ic)->key && bitVectnBitsOn (OP_USES (IC_RESULT (ic->prev))) <= 1)
        {
          sym_link *type = operandType (IC_RESULT (ic->prev));
          if (ic->prev->op != CAST || IS_BOOL (type) || bitsForType (operandType (IC_RIGHT (ic->prev))) < bitsForType (type))
          {
            if (!isOperandVolatile (ic->prev->op == '!' ? IC_LEFT (ic->prev) : IC_RIGHT (ic->prev), FALSE))
              {
                if (ic->prev->op =='!') /* Invert jump logic */
                  {
                    symbol *tmp = IC_TRUE (ic);
                    IC_TRUE (ic) = IC_FALSE (ic);
                    IC_FALSE (ic) = tmp;
                  }
                bitVectUnSetBit (OP_USES (IC_COND (ic)), ic->key);
                ReplaceOpWithCheaperOp(&IC_COND (ic), ic->prev->op == '!' ? IC_LEFT (ic->prev) : IC_RIGHT (ic->prev));
                (*change)++;
              }
/* There's an optimization opportunity here, but OP_USES doesn't seem to be */
/* initialized properly at this point. - EEP 2016-08-04 */
#if 0
            else if (bitVectnBitsOn (OP_USES(IC_COND (ic))) == 1)
              {
                /* We can replace the iTemp with the original volatile symbol */
                /* but we must make sure the volatile symbol is still accessed */
                /* only once. */
                bitVectUnSetBit (OP_USES (IC_COND (ic)), ic->key);
                ReplaceOpWithCheaperOp(&IC_COND (ic), IC_RIGHT (ic->prev));
                (*change)++;
                /* Make previous assignment an assignment to self. */
                /* killDeadCode() will eliminiate it. */
                IC_RIGHT (ic->prev) = IC_RESULT (ic->prev);
                IC_LEFT (ic->prev) = NULL;
                ic->prev->op = '=';
              }
#endif
          }
        }
    }

  /* if the conditional is a literal then */
  if (IS_OP_LITERAL (IC_COND (ic)))
    {
      if ((operandLitValue (IC_COND (ic)) != 0.0) && IC_TRUE (ic))
        {
          /* change to a goto */
          ic->op = GOTO;
          IC_LABEL (ic) = IC_TRUE (ic);
          (*change)++;
        }
      else
        {
          if (!operandLitValue (IC_COND (ic)) && IC_FALSE (ic))
            {
              ic->op = GOTO;
              IC_LABEL (ic) = IC_FALSE (ic);
              (*change)++;
            }
          else
            {
              /* then kill this if condition */
              remiCodeFromeBBlock (ebb, ic);
            }
        }

      /* now we need to recompute the control flow */
      /* since the control flow has changed        */
      /* this is very expensive but it does not happen */
      /* too often, if it does happen then the user pays */
      /* the price */
      computeControlFlow (ebbi);
      werrorfl (ic->filename, ic->lineno, W_CONTROL_FLOW);
      return;
    }

  /* if there is only one successor and that successor
     is the same one we are conditionally going to then
     we can remove this conditional statement */
  label = (IC_TRUE (ic) ? IC_TRUE (ic) : IC_FALSE (ic));
  if (elementsInSet (ebb->succList) == 1 &&
      isinSet (ebb->succList, eBBWithEntryLabel (ebbi, label)))
    {
      werrorfl (ic->filename, ic->lineno, W_CONTROL_FLOW);
      if (IS_OP_VOLATILE (IC_COND (ic)))
        {
          IC_RIGHT (ic) = IC_COND (ic);
          IC_LEFT (ic) = NULL;
          IC_RESULT (ic) = NULL;
          ic->op = DUMMY_READ_VOLATILE;
        }
      else
        {
          remiCodeFromeBBlock (ebb, ic);
          computeControlFlow (ebbi);
          return;
        }
    }

  /* if it remains an IFX then update the use Set */
  if (ic->op == IFX)
    {
      OP_USES(IC_COND (ic))=bitVectSetBit (OP_USES (IC_COND (ic)), ic->key);
      setUsesDefs (IC_COND (ic), ebb->defSet, ebb->outDefs, &ebb->usesDefs);
    }
  else if (ic->op == DUMMY_READ_VOLATILE)
    {
      OP_USES(IC_RIGHT (ic))=bitVectSetBit (OP_USES (IC_RIGHT (ic)), ic->key);
      setUsesDefs (IC_RIGHT (ic), ebb->defSet, ebb->outDefs, &ebb->usesDefs);
    }
  return;
}

/*-----------------------------------------------------------------*/
/* diCodeForSym - finds the definiting instruction for a symbol    */
/*-----------------------------------------------------------------*/
DEFSETFUNC (diCodeForSym)
{
  cseDef *cdp = item;
  V_ARG (operand *, sym);
  V_ARG (iCode **, dic);

  /* if already found */
  if (*dic)
    return 0;

  /* if not if this is the defining iCode */
  if (sym->key == cdp->key)
    {
      *dic = cdp->diCode;
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* constFold - does some constant folding                          */
/*-----------------------------------------------------------------*/
int
constFold (iCode * ic, set * cseSet)
{
  iCode *dic = NULL;
  iCode *ldic = NULL;
  /* this routine will change
     a = b + 10;
     c = a + 10;
     to
     c = b + 20; */

  /* deal with only + & - */
  if (ic->op != '+' &&
      ic->op != '-' &&
      ic->op != BITWISEAND)
    return 0;

  /* check if operation with a literal */
  if (!IS_OP_LITERAL (IC_RIGHT (ic)))
    return 0;

  /* check if we can find a definition for the
     left hand side */
  if (!(applyToSet (cseSet, diCodeForSym, IC_LEFT (ic), &dic)))
    return 0;

  if (ic->op == BITWISEAND) /* Optimize out bitwise and of comparion results */
    {
      /* check that this results in 0 or 1 only */
      if(dic->op != EQ_OP && dic->op != NE_OP && dic->op != LE_OP && dic->op != GE_OP && dic->op != '<' && dic->op != '>' && dic->op != '!')
        return 0;

      IC_RIGHT (ic) = (operandLitValueUll (IC_RIGHT (ic)) & 1) ? IC_LEFT (ic) : operandFromLit (0);

      ic->op = '=';
      IC_LEFT (ic) = 0;

      return 1;
    }

  /* check that this is also a +/-  */
  if (dic->op != '+' && dic->op != '-')
    return 0;

  /* with a literal */
  if (!IS_OP_LITERAL (IC_RIGHT (dic)))
    return 0;

  /* find the definition of the left operand
     of dic.then check if this defined with a
     get_pointer return 0 if the pointer size is
     less than 2 (MCS51 specific) */
  if (!(applyToSet (cseSet, diCodeForSym, IC_LEFT (dic), &ldic)))
    return 0;



  if (POINTER_GET (ldic) && getSize (operandType (IC_LEFT (ldic))) <= 1)
    return 0;

  /* it is if the operations are the same */
  /* the literal parts need to be added  */
  IC_LEFT (ic) = operandFromOperand (IC_LEFT (dic));
  if (ic->op == dic->op)
    IC_RIGHT (ic) = operandFromLit (operandLitValue (IC_RIGHT (ic)) +
                                    operandLitValue (IC_RIGHT (dic)));
  else
    IC_RIGHT (ic) = operandFromLit (operandLitValue (IC_RIGHT (ic)) -
                                    operandLitValue (IC_RIGHT (dic)));

  if (IS_ITEMP (IC_RESULT (ic)))
    {
      SPIL_LOC (IC_RESULT (ic)) = NULL;
      OP_SYMBOL(IC_RESULT (ic))->noSpilLoc = 1;
    }

  return 1;
}

/* Remove casts to bool from results of logical operations. */
static int
boolCast (iCode * ic, set * cseSet)
{
  iCode *dic = NULL;

  /* Only casts to booleans are optimized away. */
  if (ic->op != CAST || !IS_BOOL ( operandType (IC_RESULT (ic))))
    return 0;

  /* Find the definition for the right hand side. */
  if (!(applyToSet (cseSet, diCodeForSym, IC_RIGHT (ic), &dic)))
    return 0;

  /* Check that this is a logic op. */
  switch (dic->op)
    {
    case '!':
    case '<':
    case '>':
    case LE_OP:
    case GE_OP:
    case EQ_OP:
    case NE_OP:
    case AND_OP:
    case OR_OP:
    case GETHBIT:
    case GETABIT:
      break;
    case BITWISEAND:
      if (IS_BOOL (operandType (IC_LEFT (dic))) || IS_BOOL (operandType (IC_RIGHT (dic))))
        break;
      if (IS_OP_LITERAL (IC_RIGHT (dic)) && operandLitValue (IC_RIGHT (dic)) == 1)
        break;
    default:
      return 0;
    }

  /* Replace cast by assignment. */
  ic->op = '=';

  return 0;
}

/*-----------------------------------------------------------------*/
/* deleteGetPointers - called when a pointer is passed as parm     */
/* will delete from cseSet all get pointers computed from this     */
/* pointer. A simple ifOperandsHave is not good enough here        */
/*-----------------------------------------------------------------*/
static void
deleteGetPointers (set ** cseSet, set ** pss, operand * op, eBBlock * ebb)
{
  set *compItems = NULL;
  cseDef *cdp;
  operand *cop;
  int changes;

  /* easy return */
  if (!*cseSet && !*pss)
    return;

  addSet (&compItems, op);

  /* Recursively find all items computed from this operand .
     This done fairly simply go thru the list and find
     those that are computed by arthimetic with these
     ops */
  /* Also check for those computed from our computed
     list . This will take care of situations like
     iTemp1 = iTemp0 + 8;
     iTemp2 = iTemp1 + 8; */
  do
    {
      changes = 0;
      for (cdp = setFirstItem (*cseSet); cdp; cdp = setNextItem (*cseSet))
        {
          if (IS_ARITHMETIC_OP (cdp->diCode) || POINTER_GET(cdp->diCode))
            {
              if (isinSetWith (compItems, (void*)IC_LEFT (cdp->diCode),
                               (insetwithFunc)isOperandEqual) ||
                  isinSetWith (compItems, (void*)IC_RIGHT (cdp->diCode),
                               (insetwithFunc)isOperandEqual))
                {
                  if (!isinSetWith (compItems, (void*)IC_RESULT (cdp->diCode),
                                    (insetwithFunc)isOperandEqual))
                    {
                      addSet (&compItems, IC_RESULT (cdp->diCode));
                      changes++;
                    }
                }
            }
        }
    }
  while (changes);

  /* now for the computed items */
  for (cop = setFirstItem (compItems); cop; cop = setNextItem (compItems))
    {
      ebb->ptrsSet = bitVectSetBit (ebb->ptrsSet, cop->key);
      deleteItemIf (cseSet, ifPointerGet, cop);
      deleteItemIf (cseSet, ifDefSymIsX, cop);
      deleteItemIf (pss, ifPointerSet, cop);
    }
}

/*-----------------------------------------------------------------*/
/* delGetPointerSucc - delete get pointer from inExprs of succ with */
/*                     dfnum > supplied                            */
/*-----------------------------------------------------------------*/
DEFSETFUNC (delGetPointerSucc)
{
  eBBlock *ebp = item;
  V_ARG (operand *, op);
  V_ARG (int, dfnum);

  if (ebp->visited)
    return 0;

  ebp->visited = 1;
  if (ebp->dfnum > dfnum)
    {
      deleteItemIf (&ebp->inExprs, ifPointerGet, op);
    }

  return applyToSet (ebp->succList, delGetPointerSucc, op, dfnum);
}

/*-----------------------------------------------------------------*/
/* fixUpTypes - KLUGE HACK fixup a lowering problem                */
/*-----------------------------------------------------------------*/
static void
fixUpTypes (iCode * ic)
{
  sym_link *t1 = operandType (IC_LEFT (ic)), *t2;

  /* if (TARGET_IS_DS390) */
  if (options.model == MODEL_FLAT24)
    {
      /* hack-o-matic! */
      return;
    }

  /* for pointer_gets if the types of result & left r the
     same then change it type of result to next */
  if (IS_PTR (t1) &&
      compareType (t2 = operandType (IC_RESULT (ic)), t1) == 1)
    {
      setOperandType (IC_RESULT (ic), t2->next);
    }
}

/*-----------------------------------------------------------------*/
/* isSignedOp - will return 1 if sign is important to operation    */
/*-----------------------------------------------------------------*/
static int isSignedOp (iCode *ic)
{
    switch (ic->op) {
    case '!':
    case '~':
    case UNARYMINUS:
    case IPUSH:
    case IPOP:
    case CALL:
    case PCALL:
    case RETURN:
    case '+':
    case '-':
    case EQ_OP:
    case AND_OP:
    case OR_OP:
    case '^':
    case '|':
    case BITWISEAND:
    case INLINEASM:
    case LEFT_OP:
    case GET_VALUE_AT_ADDRESS:
    case '=':
    case IFX:
    case RECEIVE:
    case SEND:
        return 0;
    case '*':
    case '/':
    case '%':
    case '>':
    case '<':
    case LE_OP:
    case GE_OP:
    case NE_OP:
    case RRC:
    case RLC:
    case GETHBIT:
    case GETABIT:
    case GETBYTE:
    case GETWORD:
    case RIGHT_OP:
    case CAST:
    case ARRAYINIT:
        return 1;
    default:
        return 0;
    }
 }

#if 0
static void
dumpCseSet(set *cseSet)
{
  while (cseSet)
    {
      cseDef *item=cseSet->item;
      printf("->");
      printOperand (item->sym, NULL);
      printf("  ");
      piCode (item->diCode, NULL);
      cseSet = cseSet->next;
    }
}
#endif

/*-----------------------------------------------------------------*/
/* cseBBlock - common subexpression elimination for basic blocks   */
/*             this is the hackiest kludgiest routine in the whole */
/*             system. also the most important, since almost all   */
/*             data flow related information is computed by it     */
/*-----------------------------------------------------------------*/
int
cseBBlock (eBBlock * ebb, int computeOnly, ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  set *cseSet;
  iCode *ic;
  int change = 0;
  int i;
  set *ptrSetSet = NULL;
  cseDef *expr;
  int replaced;
  int recomputeDataFlow = 0;

  /* if this block is not reachable */
  if (ebb->noPath)
    return 0;

  /* set of common subexpressions */
  cseSet = setFromSet (ebb->inExprs);

  /* these will be computed by this routine */
  setToNull ((void *) &ebb->outDefs);
  setToNull ((void *) &ebb->defSet);
  setToNull ((void *) &ebb->usesDefs);
  setToNull ((void *) &ebb->ptrsSet);
  setToNull ((void *) &ebb->addrOf);
  setToNull ((void *) &ebb->ldefs);

  ebb->outDefs = bitVectCopy (ebb->inDefs);
  bitVectDefault = iCodeKey;
  ebb->defSet = newBitVect (iCodeKey);
  ebb->usesDefs = newBitVect (iCodeKey);

  /* for all the instructions in this block do */
  for (ic = ebb->sch; ic; ic = ic->next)
    {
      iCode *pdic;
      operand *pdop;
      iCode *defic;
      int checkSign ;

      ic->eBBlockNum = ebb->bbnum;

      if (SKIP_IC2 (ic))
        continue;

      /* if this is an assignment from true symbol
         to a temp then do pointer post inc/dec optimization */
      if (ic->op == '=' && !POINTER_SET (ic) &&
          IS_PTR (operandType (IC_RESULT (ic))))
        {
          ptrPostIncDecOpt (ic);
        }

      /* clear the def & use chains for the operands involved */
      /* in this operation since it can change due to opts    */
      unsetDefsAndUses (ic);

      if (ic->op == PCALL || ic->op == CALL || ic->op == RECEIVE)
        {
          /* add to defSet of the symbol */
          OP_DEFS (IC_RESULT (ic)) = bitVectSetBit (OP_DEFS (IC_RESULT (ic)), ic->key);
          /* add to the definition set of this block */
          ebb->defSet = bitVectSetBit (ebb->defSet, ic->key);
          ebb->ldefs = bitVectSetBit (ebb->ldefs, ic->key);
          ebb->outDefs = bitVectCplAnd (ebb->outDefs, OP_DEFS (IC_RESULT (ic)));
          setUsesDefs (IC_RESULT (ic), ebb->defSet, ebb->outDefs, &ebb->usesDefs);
          /* delete global variables from the cseSet
             since they can be modified by the function call */
          deleteItemIf (&cseSet, ifDefGlobal);

          /* and also iTemps derived from globals */
          deleteItemIf (&cseSet, ifFromGlobal);

          /* Delete iTemps derived from symbols whose address */
          /* has been taken */
          deleteItemIf (&cseSet, ifFromAddrTaken);

          /* delete all getpointer iCodes from cseSet, this should
             be done only for global arrays & pointers but at this
             point we don't know if globals, so to be safe do all */
          deleteItemIf (&cseSet, ifAnyGetPointer);

          /* can't cache pointer set/get operations across a call */
          deleteSet (&ptrSetSet);
        }

      /* for pcall & ipush we need to add to the useSet */
      if ((ic->op == PCALL ||
           ic->op == IPUSH ||
           ic->op == IPOP ||
           ic->op == SEND) &&
          IS_SYMOP (IC_LEFT (ic)))
        {
          /* check if they can be replaced */
          if (!computeOnly)
            {
              pdop = NULL;
              applyToSetFTrue (cseSet, findCheaperOp, IC_LEFT (ic), &pdop, 0);
              if (pdop)
                ReplaceOpWithCheaperOp(&IC_LEFT(ic), pdop);
            }
          /* the lookup could have changed it */
          if (IS_SYMOP (IC_LEFT (ic)))
            {
              OP_USES(IC_LEFT (ic))=
                bitVectSetBit (OP_USES (IC_LEFT (ic)), ic->key);
              setUsesDefs (IC_LEFT (ic), ebb->defSet,
                           ebb->outDefs, &ebb->usesDefs);
            }

          /* if we are sending a pointer as a parameter
             then kill all cse since the pointed to item
             might be changed in the function being called */
          if ((ic->op == IPUSH || ic->op == SEND) &&
              IS_PTR (operandType (IC_LEFT (ic))))
            {
              deleteGetPointers (&cseSet, &ptrSetSet, IC_LEFT (ic), ebb);
              ebb->ptrsSet = bitVectSetBit (ebb->ptrsSet, IC_LEFT (ic)->key);
              for (i = 0; i < count; ebbs[i++]->visited = 0);
              applyToSet (ebb->succList, delGetPointerSucc,
                          IC_LEFT (ic), ebb->dfnum);
            }
          continue;
        }

      /* if jumptable then mark the usage */
      if (ic->op == JUMPTABLE)
        {
          if (IS_SYMOP (IC_JTCOND (ic)))
            {
              OP_USES(IC_JTCOND (ic)) =
                bitVectSetBit (OP_USES (IC_JTCOND (ic)), ic->key);
              setUsesDefs (IC_JTCOND (ic), ebb->defSet,
                           ebb->outDefs, &ebb->usesDefs);
            }
          continue;
        }

      if (SKIP_IC (ic))
        continue;

      if (!computeOnly)
        {
          /* do some algebraic optimizations if possible */
          algebraicOpts (ic, ebb);
          while (constFold (ic, cseSet));
          while (boolCast (ic, cseSet));
        }

      /* small kludge */
      if (POINTER_GET (ic))
        {
          if (!IS_PTR (operandType (IC_LEFT (ic))))
            {
              setOperandType (IC_LEFT (ic),
                              aggrToPtr (operandType (IC_LEFT (ic)), FALSE));
              IC_LEFT (ic)->aggr2ptr = 0;
              fixUpTypes (ic);
            }
          else if (IC_LEFT (ic)->aggr2ptr == 1)
            {/* band aid for kludge */
              setOperandType (IC_LEFT (ic),
                              aggrToPtr (operandType (IC_LEFT (ic)), TRUE));
              IC_LEFT (ic)->aggr2ptr++;
              fixUpTypes (ic);
            }
        }

      if (POINTER_SET (ic))
        {
          if (!IS_PTR (operandType (IC_RESULT (ic))))
            {
              setOperandType (IC_RESULT (ic),
                              aggrToPtr (operandType (IC_RESULT (ic)), FALSE));
              IC_RESULT (ic)->aggr2ptr = 0;
            }
          else if (IC_RESULT (ic)->aggr2ptr == 1)
            {/* band aid for kludge */
              setOperandType (IC_RESULT (ic),
                              aggrToPtr (operandType (IC_RESULT (ic)), TRUE));
              IC_RESULT (ic)->aggr2ptr++;
            }
        }

      /* if this is a condition statement then */
      /* check if the condition can be replaced */
      if (ic->op == IFX)
        {
          ifxOptimize (ic, cseSet, computeOnly, ebb, &change, ebbi);
          continue;
        }

      /* if the assignment & result is a temp */
      /* see if we can replace it             */
      if (!computeOnly && ic->op == '=')
        {
          /* update the spill location for this */
          updateSpillLocation (ic, 0);

          if (POINTER_SET (ic) && IS_SYMOP (IC_RESULT (ic)) &&
              !(IS_BITFIELD (OP_SYMBOL (IC_RESULT (ic))->etype)))
            {
              pdop = NULL;
              applyToSetFTrue (cseSet, findCheaperOp, IC_RESULT (ic), &pdop, 0);
              if (pdop && !computeOnly && IS_ITEMP (pdop))
                {
                  ReplaceOpWithCheaperOp (&IC_RESULT(ic), pdop);
                  if (!IS_PTR (operandType (IC_RESULT (ic))))
                    {
                      setOperandType (IC_RESULT (ic),
                                      aggrToPtr (operandType (IC_RESULT (ic)), FALSE));
                    }
                }
            }
        }

      checkSign = isSignedOp(ic);
      replaced = 0;

      /* do the operand lookup i.e. for both the */
      /* right & left operand : check the cseSet */
      /* to see if they have been replaced if yes */
      /* then replace them with those from cseSet */
      /* left operand */
      /* and left is a symbol  */
      if (!computeOnly &&
          ic->op != ADDRESS_OF &&
          IS_SYMOP (IC_LEFT (ic)) &&
          !IS_BITFIELD (OP_SYM_ETYPE (IC_LEFT (ic))))
        {
          pdop = NULL;
          applyToSetFTrue (cseSet, findCheaperOp, IC_LEFT (ic), &pdop, checkSign);
          if (pdop)
            {
              if (POINTER_GET (ic))
                {
                  if (IS_ITEMP (pdop) || IS_OP_LITERAL (pdop))
                    {
                      /* some non dominating block does POINTER_SET with
                         this variable .. unsafe to remove any POINTER_GETs */
                      if (bitVectBitValue (ebb->ndompset, IC_LEFT (ic)->key))
                          ebb->ptrsSet = bitVectSetBit (ebb->ptrsSet, pdop->key);
                      ReplaceOpWithCheaperOp (&IC_LEFT (ic), pdop);
                      change = replaced = 1;
                    }
                  /* check if there is a pointer set
                     for the same pointer visible if yes
                     then change this into an assignment */
                  pdop = NULL;
                  if (applyToSetFTrue (cseSet, findPointerSet, IC_LEFT (ic), &pdop, IC_RESULT (ic)) &&
                      !bitVectBitValue (ebb->ptrsSet, pdop->key))
                    {
                      ic->op = '=';
                      IC_LEFT (ic) = NULL;
                      ReplaceOpWithCheaperOp (&IC_RIGHT (ic), pdop);
                      SET_ISADDR (IC_RESULT (ic), 0);
                      replaced = 1;
                    }
                }
              else
                {
                  ReplaceOpWithCheaperOp (&IC_LEFT (ic), pdop);
                  change = replaced = 1;
                }
            }
        }

      /* right operand */
      if (!computeOnly && IS_SYMOP (IC_RIGHT (ic)))
        {
          pdop = NULL;
          applyToSetFTrue (cseSet, findCheaperOp, IC_RIGHT (ic), &pdop, checkSign);
          if (pdop)
            {
              ReplaceOpWithCheaperOp (&IC_RIGHT (ic), pdop);
              change = replaced = 1;
            }
        }

      if (!computeOnly &&
          POINTER_SET (ic) &&
          IS_SYMOP (IC_RESULT (ic)) &&
          !IS_BITFIELD (OP_SYM_ETYPE (IC_RESULT (ic))))
        {
          pdop = NULL;
          applyToSetFTrue (cseSet, findCheaperOp, IC_RESULT (ic), &pdop, checkSign);
          if (pdop)
            {
              ReplaceOpWithCheaperOp (&IC_RESULT (ic), pdop);
              change = 1;
            }
        }

      /* if left or right changed then do algebraic */
      if (!computeOnly && change)
        {
          algebraicOpts (ic, ebb);
          while (constFold (ic, cseSet));
          while (boolCast (ic, cseSet));
        }

      /* if after all this it becomes an assignment to self
         then delete it and continue */
      if (ASSIGNMENT_TO_SELF (ic) && !isOperandVolatile (IC_RIGHT(ic), FALSE))
        {
          remiCodeFromeBBlock (ebb, ic);
          continue;
        }

      /* now we will check to see if the entire */
      /* operation has been performed before    */
      /* and is available                       */
      /* don't do assignments they will be killed */
      /* by dead code elimination if required  do */
      /* it only if result is a temporary         */
      pdic = NULL;
      if (!(POINTER_GET (ic) &&
            (IS_BITFIELD (OP_SYMBOL (IC_RESULT (ic))->etype) ||
             isOperandVolatile (IC_LEFT (ic), TRUE) || IS_VOLATILE (operandType (IC_LEFT (ic))->next) ||
             bitVectBitValue (ebb->ndompset, IC_LEFT (ic)->key))) &&
          !ASSIGNMENT (ic) &&
          IS_ITEMP (IC_RESULT (ic)) &&
          !computeOnly)
        {
          applyToSet (cseSet, findPrevIc, ic, &pdic);
          if (pdic && compareType (operandType (IC_RESULT (pdic)),
                                   operandType (IC_RESULT (ic))) != 1)
            {
              pdic = NULL;
            }
          if (pdic && port->cseOk && (*port->cseOk)(ic, pdic) == 0)
            {
              pdic = NULL;
            }
        }

      /* Alternate code */
      if (pdic && IS_ITEMP (IC_RESULT (ic)))
        {
          if (POINTER_GET (ic) && bitVectBitValue (ebb->ptrsSet, IC_LEFT (ic)->key))
            {
              /* Mmm, found an equivalent pointer get at a lower level.
                 This could be a loop however with the same pointer set
                 later on */
            }
          else
            {
              /* if previous definition found change this to an assignment */
              ic->op = '=';
              IC_LEFT (ic) = NULL;
              IC_RIGHT (ic) = operandFromOperand (IC_RESULT (pdic));
              SET_ISADDR (IC_RESULT (ic), 0);
              SET_ISADDR (IC_RIGHT (ic), 0);
            }
        }

      if (!(POINTER_SET (ic)) && IC_RESULT (ic))
        {
          cseDef *csed;
          deleteItemIf (&cseSet, ifDefSymIsX, IC_RESULT (ic));
          csed = newCseDef (IC_RESULT (ic), ic);
          updateCseDefAncestors (csed, cseSet);
          addSetHead (&cseSet, csed);
        }
      defic = ic;

      /* if assignment to a parameter which is not
         mine and type is a pointer then delete
         pointerGets to take care of aliasing */
      if (ASSIGNMENT (ic) &&
          IS_SYMOP (IC_RESULT (ic)) &&
          OTHERS_PARM (OP_SYMBOL (IC_RESULT (ic))) &&
          IS_PTR (operandType (IC_RESULT (ic))))
        {
          deleteGetPointers (&cseSet, &ptrSetSet, IC_RIGHT (ic), ebb);
          for (i = 0; i < count; ebbs[i++]->visited = 0);
          applyToSet (ebb->succList, delGetPointerSucc, IC_RIGHT (ic), ebb->dfnum);
          ebb->ptrsSet = bitVectSetBit (ebb->ptrsSet, IC_RIGHT (ic)->key);
        }

      /* if this is a pointerget then see if we can replace
         this with a previously assigned pointer value */
      if (POINTER_GET (ic) &&
          !(IS_BITFIELD (OP_SYMBOL (IC_RESULT (ic))->etype) ||
            isOperandVolatile (IC_LEFT (ic), TRUE)))
        {
          pdop = NULL;
          applyToSet (ptrSetSet, findPointerSet, IC_LEFT (ic), &pdop, IC_RESULT (ic));
          /* if we find it then locally replace all
             references to the result with what we assigned */
          if (pdop)
            {
              replaceAllSymBySym (ic->next, IC_RESULT (ic), pdop, &ebb->ndompset);
            }
        }

      /* delete from the cseSet anything that has */
      /* operands matching the result of this     */
      /* except in case of pointer access         */
      if (!(POINTER_SET (ic)) && IS_SYMOP (IC_RESULT (ic)))
        {
          deleteItemIf (&cseSet, ifOperandsHave, IC_RESULT (ic));
          deleteItemIf (&ptrSetSet, ifOperandsHave, IC_RESULT (ic));
          /* delete any previous definitions */
          ebb->defSet = bitVectCplAnd (ebb->defSet, OP_DEFS (IC_RESULT (ic)));

         /* Until pointer tracking is complete, by conservative and delete all */
         /* pointer accesses that might alias this symbol. */
         if (isOperandGlobal (IC_RESULT (ic)))
           {
             memmap *map = SPEC_OCLS (getSpec (operandType (IC_RESULT (ic))));
             deleteItemIf (&cseSet, ifAnyUnrestrictedGetPointer, map->ptrType);
             deleteItemIf (&ptrSetSet, ifAnyUnrestrictedSetPointer, map->ptrType);
           }
        }

      /* add the left & right to the defUse set */
      if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)))
        {
          OP_USES (IC_LEFT (ic)) =
            bitVectSetBit (OP_USES (IC_LEFT (ic)), ic->key);
          setUsesDefs (IC_LEFT (ic), ebb->defSet, ebb->outDefs, &ebb->usesDefs);
        }

      if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)))
        {
          OP_USES (IC_RIGHT (ic)) =
            bitVectSetBit (OP_USES (IC_RIGHT (ic)), ic->key);
          setUsesDefs (IC_RIGHT (ic), ebb->defSet, ebb->outDefs, &ebb->usesDefs);
        }

      /* for the result it is special case, put the result */
      /* in the defuseSet if it is a pointer or array access */
      if (POINTER_SET (defic) &&
		  (IS_SYMOP (IC_RESULT (ic)) || IS_OP_LITERAL (IC_RESULT (ic))))
        {
          sym_link *ptype = operandType (IC_RESULT (ic));

          if (IS_SYMOP (IC_RESULT (ic)))
            {
              OP_USES (IC_RESULT (ic)) =
                bitVectSetBit (OP_USES (IC_RESULT (ic)), ic->key);
              setUsesDefs (IC_RESULT (ic), ebb->defSet, ebb->outDefs, &ebb->usesDefs);
            }
          deleteItemIf (&cseSet, ifPointerGet, IC_RESULT (ic));
          ebb->ptrsSet = bitVectSetBit (ebb->ptrsSet, IC_RESULT (ic)->key);
          /* delete from inexpressions of all successors which
             have dfNum > than this block */
          for (i = 0; i < count; ebbs[i++]->visited = 0);
          applyToSet (ebb->succList, delGetPointerSucc, IC_RESULT (ic), ebb->dfnum);

          /* delete from cseSet all other pointer sets
             for this operand */
          deleteItemIf (&ptrSetSet, ifPointerSet, IC_RESULT (ic));
          /* add to the local pointerset set */
          addSetHead (&ptrSetSet, newCseDef (IC_RESULT (ic), ic));

          /* A write via a non-restrict pointer may modify a global */
          /* variable used by this function, so delete them */
          /* and any derived symbols from cseSet. */
          if (!IS_PTR_RESTRICT (ptype))
            {
              deleteItemIf (&cseSet, ifDefGlobalAliasableByPtr);
              deleteItemIf (&cseSet, ifFromGlobalAliasableByPtr, DCL_TYPE(ptype));
            }

          /* This could be made more specific for better optimization, but */
          /* for safety, delete anything this write may have modified. */
          deleteItemIf (&cseSet, ifFromAddrTaken);
          deleteItemIf (&cseSet, ifAnyGetPointer);
        }
      else
        {
          /* add the result to definition set */
          if (IS_SYMOP (IC_RESULT (ic)))
            {
              OP_DEFS(IC_RESULT (ic))=
                bitVectSetBit (OP_DEFS (IC_RESULT (ic)), ic->key);
              ebb->defSet = bitVectSetBit (ebb->defSet, ic->key);
              ebb->outDefs = bitVectCplAnd (ebb->outDefs, OP_DEFS (IC_RESULT (ic)));
              ebb->ldefs = bitVectSetBit (ebb->ldefs, ic->key);
            }
        }

      /* if this is an addressof instruction then */
      /* put the symbol in the address of list &  */
      /* delete it from the cseSet                */
      if (defic->op == ADDRESS_OF)
        {
          addSetHead (&ebb->addrOf, IC_LEFT (ic));
          deleteItemIf (&cseSet, ifDefSymIsX, IC_LEFT (ic));
        }

      /* If this was previously in the out expressions in the  */
      /* original form, it might need to be killed by another block */
      /* in the new form if we have replaced operands, so recompute */
      /* the data flow after we finish this block */
      if (replaced && ifDiCodeIs (ebb->outExprs, ic))
        recomputeDataFlow = 1;
    }

  for (expr=setFirstItem (ebb->inExprs); expr; expr=setNextItem (ebb->inExprs))
    if (!isinSetWith (cseSet, expr, isCseDefEqual) &&
        !isinSetWith (ebb->killedExprs, expr, isCseDefEqual))
      {
        addSetHead (&ebb->killedExprs, expr);
      }
  setToNull ((void *) &ebb->outExprs);
  ebb->outExprs = cseSet;
  ebb->outDefs = bitVectUnion (ebb->outDefs, ebb->defSet);
  ebb->ptrsSet = bitVectUnion (ebb->ptrsSet, ebb->inPtrsSet);

  if (recomputeDataFlow)
    computeDataFlow (ebbi);

  return change;
}

/*------------------------------------------------------------------*/
/* cseAllBlocks - will sequentially go thru & do cse for all blocks */
/*------------------------------------------------------------------*/
int
cseAllBlocks (ebbIndex * ebbi, int computeOnly)
{
  eBBlock ** ebbs = ebbi->dfOrder;
  int count = ebbi->count;
  int i;
  int change = 0;

  /* if optimization turned off */

  for (i = 0; i < count; i++)
    change += cseBBlock (ebbs[i], computeOnly, ebbi);

  return change;
}

