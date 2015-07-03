/*-------------------------------------------------------------------------

  SDCCptropt.c - source file for pointer arithmetic Optimizations

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

/*-----------------------------------------------------------------------*/
/* findPointerGetSet - find the pointer get or set for a operand         */
/*-----------------------------------------------------------------------*/
static iCode *
findPointerGetSet (iCode * sic, operand * op)
{
  iCode *ic = sic;

  for (; ic; ic = ic->next)
    {
      if ((POINTER_SET (ic) && isOperandEqual (op, IC_RESULT (ic))) || (POINTER_GET (ic) && isOperandEqual (op, IC_LEFT (ic))))
        return ic;

      /* if we find any other usage or definition of op null */
      if (IC_RESULT (ic) && isOperandEqual (IC_RESULT (ic), op))
        return NULL;

      if (IC_RIGHT (ic) && isOperandEqual (IC_RIGHT (ic), op))
        return NULL;

      if (IC_LEFT (ic) && isOperandEqual (IC_LEFT (ic), op))
        return NULL;

    }

  return NULL;
}

static int
pattern1 (iCode * sic)
{
  /* this is what we do. look for sequences like

     iTempX := _SOME_POINTER_;
     iTempY := _SOME_POINTER_ + nn ;   nn  = sizeof (pointed to object)      sic->next
     _SOME_POINTER_ := iTempY;                                               sic->next->next
     either
     iTempZ := @[iTempX];                                                    sic->next->next->next
     or
     *(iTempX) := ..something..                                              sic->next->next->next
     if we find this then transform this to
     iTempX := _SOME_POINTER_;
     either
     iTempZ := @[iTempX];
     or
     *(iTempX) := ..something..
     iTempY := _SOME_POINTER_ + nn ;   nn  = sizeof (pointed to object)
     _SOME_POINTER_ := iTempY; */

  /* sounds simple enough so let's start , here I use negative
     tests all the way to return if any test fails */
  iCode *pgs, *sh, *st;

  if (!(sic->next && sic->next->next && sic->next->next->next))
    return 0;
  if (sic->next->op != '+' && sic->next->op != '-')
    return 0;
  if (!(sic->next->next->op == '=' && !POINTER_SET (sic->next->next)))
    return 0;
  if (!isOperandEqual (IC_LEFT (sic->next), IC_RIGHT (sic)) || !IS_OP_LITERAL (IC_RIGHT (sic->next)))
    return 0;
  if (operandLitValue (IC_RIGHT (sic->next)) != getSize (operandType (IC_RIGHT (sic))->next))
    return 0;
  if (!isOperandEqual (IC_RESULT (sic->next->next), IC_RIGHT (sic)))
    return 0;
  if (!isOperandEqual (IC_RESULT (sic->next), IC_RIGHT (sic->next->next)))
    return 0;
  if (!(pgs = findPointerGetSet (sic->next->next, IC_RESULT (sic))))
    return 0;

  /* found the pattern .. now do the transformation */
  sh = sic->next;
  st = sic->next->next;

  /* take the two out of the chain */
  sic->next = st->next;
  st->next->prev = sic;

  /* and put them after the pointer get/set icode */
  if ((st->next = pgs->next))
    st->next->prev = st;
  pgs->next = sh;
  sh->prev = pgs;
  return 1;
}

static int
pattern2 (iCode * sic)
{
  /* this is what we do. look for sequences like

     iTempX := _SOME_POINTER_;
     iTempY := _SOME_POINTER_ + nn ;   nn  = sizeof (pointed to object)      sic->next
     iTempK := iTempY;                                                       sic->next->next
     _SOME_POINTER_ := iTempK;                                               sic->next->next->next
     either
     iTempZ := @[iTempX];                                                    sic->next->next->next->next
     or
     *(iTempX) := ..something..                                              sic->next->next->next->next
     if we find this then transform this to
     iTempX := _SOME_POINTER_;
     either
     iTempZ := @[iTempX];
     or
     *(iTempX) := ..something..
     iTempY := _SOME_POINTER_ + nn ;   nn  = sizeof (pointed to object)
     iTempK := iTempY;
     _SOME_POINTER_ := iTempK; */

  /* sounds simple enough so let's start , here I use negative
     tests all the way to return if any test fails */
  iCode *pgs, *sh, *st;

  if (!(sic->next && sic->next->next && sic->next->next->next && sic->next->next->next->next))
    return 0;

  /* yes I can OR them together and make one large if... but I have
     simple mind and like to keep things simple & readable */
  if (!(sic->next->op == '+' || sic->next->op == '-'))
    return 0;
  if (!isOperandEqual (IC_RIGHT (sic), IC_LEFT (sic->next)))
    return 0;
  if (!IS_OP_LITERAL (IC_RIGHT (sic->next)))
    return 0;
  if (operandLitValue (IC_RIGHT (sic->next)) != getSize (operandType (IC_RIGHT (sic))->next))
    return 0;
  if (!IS_ASSIGN_ICODE (sic->next->next))
    return 0;
  if (!isOperandEqual (IC_RIGHT (sic->next->next), IC_RESULT (sic->next)))
    return 0;
  if (!IS_ASSIGN_ICODE (sic->next->next->next))
    return 0;
  if (!isOperandEqual (IC_RIGHT (sic->next->next->next), IC_RESULT (sic->next->next)))
    return 0;
  if (!isOperandEqual (IC_RESULT (sic->next->next->next), IC_LEFT (sic->next)))
    return 0;
  if (!(pgs = findPointerGetSet (sic->next->next->next, IC_RESULT (sic))))
    return 0;

  /* found the pattern .. now do the transformation */
  sh = sic->next;
  st = sic->next->next->next;

  /* take the three out of the chain */
  sic->next = st->next;
  st->next->prev = sic;

  /* and put them after the pointer get/set icode */
  if ((st->next = pgs->next))
    st->next->prev = st;
  pgs->next = sh;
  sh->prev = pgs;
  return 1;
}

/*-----------------------------------------------------------------------*/
/* ptrPostIncDecOpts - will do some pointer post increment optimizations */
/*                     this will help register allocation amongst others */
/*-----------------------------------------------------------------------*/
void
ptrPostIncDecOpt (iCode * sic)
{
  if (pattern1 (sic))
    return;
  pattern2 (sic);
}

/*-----------------------------------------------------------------------*/
/* addPattern1 - transform addition to pointer of variables              */
/*-----------------------------------------------------------------------*/
static int
addPattern1 (iCode * ic)
{
  iCode *dic;
  operand *tmp;
  /* transform :
     iTempAA = iTempBB + iTempCC
     iTempDD = iTempAA + CONST
     to
     iTempAA = iTempBB + CONST
     iTempDD = iTempAA + iTempCC
   */
  if (!isOperandLiteral (IC_RIGHT (ic)))
    return 0;
  if ((dic = findBackwardDef (IC_LEFT (ic), ic->prev)) == NULL)
    return 0;
  if (bitVectnBitsOn (OP_SYMBOL (IC_RESULT (dic))->uses) > 1)
    return 0;
  if (dic->op != '+')
    return 0;
  tmp = IC_RIGHT (ic);
  IC_RIGHT (ic) = IC_RIGHT (dic);
  IC_RIGHT (dic) = tmp;
  return 1;
}

/*-----------------------------------------------------------------------*/
/* ptrAddition - optimize pointer additions                              */
/*-----------------------------------------------------------------------*/
int
ptrAddition (iCode * sic)
{
  if (addPattern1 (sic))
    return 1;
  return 0;
}

/*--------------------------------------------------------------------*/
/* ptrBaseRematSym - find the base symbol of a remat. pointer         */
/*--------------------------------------------------------------------*/
symbol *
ptrBaseRematSym (symbol * ptrsym)
{
  iCode *ric;

  if (!ptrsym->remat)
    return NULL;

  ric = ptrsym->rematiCode;
  while (ric)
    {
      if (ric->op == '+' || ric->op == '-')
        ric = OP_SYMBOL (IC_LEFT (ric))->rematiCode;
      else if (IS_CAST_ICODE (ric))
        ric = OP_SYMBOL (IC_RIGHT (ric))->rematiCode;
      else
        break;
    }

  if (ric && IS_SYMOP (IC_LEFT (ric)))
    return OP_SYMBOL (IC_LEFT (ric));
  else
    return NULL;
}


/*--------------------------------------------------------------------*/
/* ptrPseudoSymSafe - check to see if the conversion of the result of */
/*   a pointerGet of a rematerializable pointer to a pseudo symbol is */
/*   safe. Returns true if safe, or false if hazards were detected.   */
/*--------------------------------------------------------------------*/
int
ptrPseudoSymSafe (symbol * sym, iCode * dic)
{
  symbol *ptrsym;
  symbol *basesym;
  iCode *ric;
  iCode *ic;
  int ptrsymDclType;
  //int isGlobal;

  assert (POINTER_GET (dic));

  /* Can't if spills to this symbol are prohibited */
  if (sym->noSpilLoc)
    return 0;

  /* Get the pointer */
  if (!IS_SYMOP (IC_LEFT (dic)))
    return 0;
  ptrsym = OP_SYMBOL (IC_LEFT (dic));

  /* Must be a rematerializable pointer */
  if (!ptrsym->remat)
    return 0;

  /* The pointer type must be uncasted */
  if (IS_CAST_ICODE (ptrsym->rematiCode))
    return 0;

  /* The symbol's live range must not preceed its definition */
  if (dic->seq > sym->liveFrom)
    return 0;

  /* Ok, this is a good candidate for a pseudo symbol.      */
  /* However, we must check for two hazards:                */
  /*   1) The symbol's live range must not include a CALL   */
  /*      or PCALL iCode.                                   */
  /*   2) The symbol's live range must not include any      */
  /*      writes to the variable the pointer rematerializes */
  /*      within (to avoid aliasing problems)               */

  /* Find the base symbol the rematerialization is based on */
  ric = ptrsym->rematiCode;
  while (ric->op == '+' || ric->op == '-')
    ric = OP_SYMBOL (IC_LEFT (ric))->rematiCode;
  if (IS_CAST_ICODE (ric))
    return 0;
  basesym = OP_SYMBOL (IC_LEFT (ric));

  //isGlobal = !basesym->islocal && !basesym->ismyparm;
  ptrsymDclType = aggrToPtrDclType (ptrsym->type, FALSE);

  ic = dic->next;
  while (ic && ic->seq <= sym->liveTo)
    {
      if (!(SKIP_IC3 (ic) || ic->op == IFX))
        {
          /* Check for hazard #1 */
          if ((ic->op == CALL || ic->op == PCALL) /* && isGlobal */ )
            {
              if (ic->seq <= sym->liveTo)
                return 0;
            }
          /* Check for hazard #2 */
          else if (POINTER_SET (ic))
            {
              symbol *ptrsym2;

              if (!IS_SYMOP (IC_RESULT (ic)))
                return 0;

              ptrsym2 = OP_SYMBOL (IC_RESULT (ic));

              if (ptrsym2->remat)
                {
                  /* Must not be the same base symbol */
                  if (basesym == ptrBaseRematSym (ptrsym2))
                    return 0;
                }
              else
                {
                  int ptrsym2DclType = aggrToPtrDclType (ptrsym2->type, FALSE);

                  /* Pointer must have no memory space in common */
                  if (ptrsym2DclType == ptrsymDclType || ptrsym2DclType == GPOINTER || ptrsymDclType == GPOINTER)
                    return 0;
                }
            }
          else if (IC_RESULT (ic))
            {
              symbol *rsym;

              if (!IS_SYMOP (IC_RESULT (ic)))
                return 0;

              rsym = OP_SYMBOL (IC_RESULT (ic));

              /* Make sure there is no conflict with another pseudo symbol */
              if (rsym->psbase == basesym)
                return 0;
              if (rsym->isspilt && rsym->usl.spillLoc)
                rsym = rsym->usl.spillLoc;
              if (rsym->psbase == basesym)
                return 0;
            }
        }

      if (ic->seq == sym->liveTo)
        break;
      ic = ic->next;
    }

  /* If the live range went past the end of the defining basic */
  /* block, then a full analysis is too complicated to attempt */
  /* here. To be safe, we must assume the worst.               */
  if (!ic)
    return 0;

  /* Ok, looks safe */
  return 1;
}

/*--------------------------------------------------------------------*/
/* ptrPseudoSymConvert - convert the result of a pointerGet to a      */
/*   pseudo symbol. The pointer must be rematerializable.             */
/*--------------------------------------------------------------------*/
void
ptrPseudoSymConvert (symbol * sym, iCode * dic, const char *name)
{
  symbol *psym = newSymbol (name, 1);
  psym->psbase = ptrBaseRematSym (OP_SYMBOL (IC_LEFT (dic)));
  psym->type = sym->type;
  psym->etype = psym->psbase->etype;

  strcpy (psym->rname, psym->name);
  sym->isspilt = 1;
  sym->usl.spillLoc = psym;
#if 0                           // an alternative fix for bug #480076
  /* now this is a useless assignment to itself */
  remiCodeFromeBBlock (ebbs, dic);
#else
  /* now this really is an assignment to itself, make it so;
     it will be optimized out later */
  dic->op = '=';
  ReplaceOpWithCheaperOp (&IC_RIGHT (dic), IC_RESULT (dic));
  IC_LEFT (dic) = NULL;
#endif
}
