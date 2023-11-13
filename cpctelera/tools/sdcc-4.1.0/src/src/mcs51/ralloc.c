/*------------------------------------------------------------------------

  SDCCralloc.c - source file for register allocation. (8051) specific

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
#include "ralloc.h"
#include "gen.h"
#include "dbuf_string.h"

/*-----------------------------------------------------------------*/
/* At this point we start getting processor specific although      */
/* some routines are non-processor specific & can be reused when   */
/* targetting other processors. The decision for this will have    */
/* to be made on a routine by routine basis                        */
/* routines used to pack registers are most definitely not reusable */
/* since the pack the registers depending strictly on the MCU      */
/*-----------------------------------------------------------------*/

extern void gen51Code (iCode *);
#define D(x)

/* Global data */
static struct
{
  bitVect *spiltSet;
  set *stackSpil;
  bitVect *regAssigned;
  bitVect *totRegAssigned;      /* final set of LRs that got into registers */
  short blockSpil;
  int slocNum;
  bitVect *funcrUsed;           /* registers used in a function */
  int stackExtend;
  int dataExtend;
  bitVect *allBitregs;          /* all bit registers */
  bitVect *allBankregs;         /* all bank registers */
}
_G;

/* Shared with gen.c */
int mcs51_ptrRegReq;            /* one byte pointer register required */

/* 8051 registers */
reg_info regs8051[] = {
  {REG_GPR, R7_IDX, REG_GPR, "r7", "ar7", "0", 7, 1},
  {REG_GPR, R6_IDX, REG_GPR, "r6", "ar6", "0", 6, 1},
  {REG_GPR, R5_IDX, REG_GPR, "r5", "ar5", "0", 5, 1},
  {REG_GPR, R4_IDX, REG_GPR, "r4", "ar4", "0", 4, 1},
  {REG_GPR, R3_IDX, REG_GPR, "r3", "ar3", "0", 3, 1},
  {REG_GPR, R2_IDX, REG_GPR, "r2", "ar2", "0", 2, 1},
  {REG_PTR, R1_IDX, REG_PTR, "r1", "ar1", "0", 1, 1},
  {REG_PTR, R0_IDX, REG_PTR, "r0", "ar0", "0", 0, 1},
  {REG_BIT, B0_IDX, REG_BIT, "b0", "b0", "bits", 0, 1},
  {REG_BIT, B1_IDX, REG_BIT, "b1", "b1", "bits", 1, 1},
  {REG_BIT, B2_IDX, REG_BIT, "b2", "b2", "bits", 2, 1},
  {REG_BIT, B3_IDX, REG_BIT, "b3", "b3", "bits", 3, 1},
  {REG_BIT, B4_IDX, REG_BIT, "b4", "b4", "bits", 4, 1},
  {REG_BIT, B5_IDX, REG_BIT, "b5", "b5", "bits", 5, 1},
  {REG_BIT, B6_IDX, REG_BIT, "b6", "b6", "bits", 6, 1},
  {REG_BIT, B7_IDX, REG_BIT, "b7", "b7", "bits", 7, 1},
  {REG_GPR, X8_IDX, REG_GPR, "x8", "x8", "xreg", 0, 1},
  {REG_GPR, X9_IDX, REG_GPR, "x9", "x9", "xreg", 1, 1},
  {REG_GPR, X10_IDX, REG_GPR, "x10", "x10", "xreg", 2, 1},
  {REG_GPR, X11_IDX, REG_GPR, "x11", "x11", "xreg", 3, 1},
  {REG_GPR, X12_IDX, REG_GPR, "x12", "x12", "xreg", 4, 1},
  {REG_CND, CND_IDX, REG_CND, "C", "not_psw", "0xd0", 0, 1},
  {0, DPL_IDX, 0, "dpl", "dpl", "0x82", 0, 0},
  {0, DPH_IDX, 0, "dph", "dph", "0x83", 0, 0},
  {0, B_IDX, 0, "b", "b", "0xf0", 0, 0},
  {0, A_IDX, 0, "a", "acc", "0xe0", 0, 0},
};

int mcs51_nRegs = 16;
static void spillThis (symbol *);
static void freeAllRegs ();

/*-----------------------------------------------------------------*/
/* allocReg - allocates register of given type                     */
/*-----------------------------------------------------------------*/
static reg_info *
allocReg (short type)
{
  int i;

  for (i = 0; i < mcs51_nRegs; i++)
    {
      /* if type is given as 0 then any
         free register will do */
      if (!type && regs8051[i].isFree)
        {
          regs8051[i].isFree = 0;
          if (currFunc)
            currFunc->regsUsed = bitVectSetBit (currFunc->regsUsed, i);
          return &regs8051[i];
        }
      /* otherwise look for specific type
         of register */
      if (regs8051[i].isFree && regs8051[i].type == type)
        {
          regs8051[i].isFree = 0;
          if (currFunc)
            currFunc->regsUsed = bitVectSetBit (currFunc->regsUsed, i);
          return &regs8051[i];
        }
    }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* allocThisReg - allocates a particular register (if free)        */
/*-----------------------------------------------------------------*/
static reg_info *
allocThisReg (reg_info *reg)
{
  if (!reg->isFree)
    return NULL;

  reg->isFree = 0;
  if (currFunc)
    currFunc->regsUsed = bitVectSetBit (currFunc->regsUsed, reg->rIdx);

  return reg;
}


/*-----------------------------------------------------------------*/
/* mcs51_regWithIdx - returns pointer to register with index number*/
/*-----------------------------------------------------------------*/
reg_info *
mcs51_regWithIdx (int idx)
{
  int i;

  for (i = 0; i < sizeof (regs8051) / sizeof (reg_info); i++)
    if (regs8051[i].rIdx == idx)
      return &regs8051[i];

  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "regWithIdx not found");
  exit (1);
}

/*-----------------------------------------------------------------*/
/* freeReg - frees a register                                      */
/*-----------------------------------------------------------------*/
static void
freeReg (reg_info *reg)
{
  if (!reg)
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "freeReg - Freeing NULL register");
      exit (1);
    }

  reg->isFree = 1;
}


/*-----------------------------------------------------------------*/
/* nFreeRegs - returns number of free registers                    */
/*-----------------------------------------------------------------*/
static int
nFreeRegs (int type)
{
  int i;
  int nfr = 0;

  for (i = 0; i < mcs51_nRegs; i++)
    if (regs8051[i].isFree && regs8051[i].type == type)
      nfr++;
  return nfr;
}

/*-----------------------------------------------------------------*/
/* nfreeRegsType - free registers with type                         */
/*-----------------------------------------------------------------*/
static int
nfreeRegsType (int type)
{
  int nfr;
  if (type == REG_PTR)
    {
      if ((nfr = nFreeRegs (type)) == 0)
        return nFreeRegs (REG_GPR);
    }

  return nFreeRegs (type);
}

/*-----------------------------------------------------------------*/
/* useReg - marks a register  as used                              */
/*-----------------------------------------------------------------*/
static void
useReg (reg_info *reg)
{
  reg->isFree = 0;
}

/*-----------------------------------------------------------------*/
/* computeSpillable - given a point find the spillable live ranges */
/*-----------------------------------------------------------------*/
static bitVect *
computeSpillable (iCode * ic)
{
  bitVect *spillable;

  /* spillable live ranges are those that are live at this
     point . the following categories need to be subtracted
     from this set.
     a) - those that are already spilt
     b) - if being used by this one
     c) - defined by this one */

  spillable = bitVectCopy (ic->rlive);
  spillable = bitVectCplAnd (spillable, _G.spiltSet);   /* those already spilt */
  spillable = bitVectCplAnd (spillable, ic->uses);      /* used in this one */
  bitVectUnSetBit (spillable, ic->defKey);
  spillable = bitVectIntersect (spillable, _G.regAssigned);
  return spillable;
}

/*-----------------------------------------------------------------*/
/* bitType - will return 1 if the symbol has type REG_BIT          */
/*-----------------------------------------------------------------*/
static int
bitType (symbol * sym, eBBlock * ebp, iCode * ic)
{
  return (sym->regType == REG_BIT ? 1 : 0);
}

/*-----------------------------------------------------------------*/
/* noSpilLoc - return true if a variable has no spil location      */
/*-----------------------------------------------------------------*/
static int
noSpilLoc (symbol * sym, eBBlock * ebp, iCode * ic)
{
  return (sym->usl.spillLoc ? 0 : 1);
}

/*-----------------------------------------------------------------*/
/* hasSpilLoc - will return 1 if the symbol has spil location      */
/*-----------------------------------------------------------------*/
static int
hasSpilLoc (symbol * sym, eBBlock * ebp, iCode * ic)
{
  return (sym->usl.spillLoc ? 1 : 0);
}

/*-----------------------------------------------------------------*/
/* directSpilLoc - will return 1 if the spillocation is in direct  */
/*-----------------------------------------------------------------*/
static int
directSpilLoc (symbol * sym, eBBlock * ebp, iCode * ic)
{
  if (sym->usl.spillLoc && (IN_DIRSPACE (SPEC_OCLS (sym->usl.spillLoc->etype))))
    return 1;
  else
    return 0;
}

/*-----------------------------------------------------------------*/
/* hasSpilLocnoUptr - will return 1 if the symbol has spil         */
/*                    location but is not used as a pointer        */
/*-----------------------------------------------------------------*/
static int
hasSpilLocnoUptr (symbol * sym, eBBlock * ebp, iCode * ic)
{
  return ((sym->usl.spillLoc && !sym->uptr) ? 1 : 0);
}

/*-----------------------------------------------------------------*/
/* rematable - will return 1 if the remat flag is set              */
/*-----------------------------------------------------------------*/
static int
rematable (symbol * sym, eBBlock * ebp, iCode * ic)
{
  return sym->remat;
}

/*-----------------------------------------------------------------*/
/* notUsedInRemaining - not used or defined in remain of the block */
/*-----------------------------------------------------------------*/
static int
notUsedInRemaining (symbol * sym, eBBlock * ebp, iCode * ic)
{
  return ((usedInRemaining (operandFromSymbol (sym), ic) ? 0 : 1) && allDefsOutOfRange (sym->defs, ebp->fSeq, ebp->lSeq));
}

/*-----------------------------------------------------------------*/
/* allLRs - return true for all                                    */
/*-----------------------------------------------------------------*/
static int
allLRs (symbol * sym, eBBlock * ebp, iCode * ic)
{
  return 1;
}

/*-----------------------------------------------------------------*/
/* liveRangesWith - applies function to a given set of live range  */
/*-----------------------------------------------------------------*/
static set *
liveRangesWith (bitVect * lrs, int (func) (symbol *, eBBlock *, iCode *), eBBlock * ebp, iCode * ic)
{
  set *rset = NULL;
  int i;

  if (!lrs || !lrs->size)
    return NULL;

  for (i = 1; i < lrs->size; i++)
    {
      symbol *sym;
      if (!bitVectBitValue (lrs, i))
        continue;

      /* if we don't find it in the live range
         hash table we are in serious trouble */
      if (!(sym = hTabItemWithKey (liveRanges, i)))
        {
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "liveRangesWith could not find liveRange");
          exit (1);
        }

      if (func (sym, ebp, ic) && bitVectBitValue (_G.regAssigned, sym->key))
        addSetHead (&rset, sym);
    }

  return rset;
}


/*-----------------------------------------------------------------*/
/* leastUsedLR - given a set determines which is the least used    */
/*-----------------------------------------------------------------*/
static symbol *
leastUsedLR (set * sset)
{
  symbol *sym = NULL, *lsym = NULL;

  sym = lsym = setFirstItem (sset);

  if (!lsym)
    return NULL;

  for (; lsym; lsym = setNextItem (sset))
    {
      /* if usage is the same then prefer
         to spill the smaller of the two */
      if (lsym->used == sym->used)
        if (getSize (lsym->type) < getSize (sym->type))
          sym = lsym;

      /* if less usage */
      if (lsym->used < sym->used)
        sym = lsym;
    }

  setToNull ((void *) &sset);
  sym->blockSpil = 0;
  return sym;
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
      if (bitVectBitValue (sym->clashes, fsym->key))
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
     the size can be accomodated  */
  if (sym->isFree && noOverLap (sym->usl.itmpStack, fsym) && getSize (sym->type) >= getSize (fsym->type)
      && (IS_BIT (sym->type) == IS_BIT (fsym->type)))
    {
      *sloc = sym;
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* spillLRWithPtrReg :- will spil those live ranges which use PTR  */
/*-----------------------------------------------------------------*/
static void
spillLRWithPtrReg (symbol * forSym)
{
  symbol *lrsym;
  reg_info *r0, *r1;
  int k;

  if (!_G.regAssigned || bitVectIsZero (_G.regAssigned))
    return;

  r0 = mcs51_regWithIdx (R0_IDX);
  r1 = mcs51_regWithIdx (R1_IDX);

  /* for all live ranges */
  for (lrsym = hTabFirstItem (liveRanges, &k); lrsym; lrsym = hTabNextItem (liveRanges, &k))
    {
      int j;

      /* if no registers assigned to it or spilt */
      /* if it does not overlap this then
         no need to spill it */

      if (lrsym->isspilt || !lrsym->nRegs || (lrsym->liveTo < forSym->liveFrom))
        continue;

      /* go thru the registers : if it is either
         r0 or r1 then spill it */
      for (j = 0; j < lrsym->nRegs; j++)
        if (lrsym->regs[j] == r0 || lrsym->regs[j] == r1)
          {
            spillThis (lrsym);
            break;
          }
    }
}

/*-----------------------------------------------------------------*/
/* createStackSpil - create a location on the stack to spil        */
/*-----------------------------------------------------------------*/
static symbol *
createStackSpil (symbol * sym)
{
  symbol *sloc = NULL;
  int useXstack, model;

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

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "sloc%d", _G.slocNum++);

  sloc = newiTemp (dbuf_c_str (&dbuf));
  dbuf_destroy (&dbuf);

  /* set the type to the spilling symbol */
  sloc->type = copyLinkChain (sym->type);
  sloc->etype = getSpec (sloc->type);
  if (!IS_BIT (sloc->etype))
    {
      SPEC_SCLS (sloc->etype) = S_DATA;
    }
  else if (SPEC_SCLS (sloc->etype) == S_SBIT)
    {
      SPEC_SCLS (sloc->etype) = S_BIT;
    }
  SPEC_EXTR (sloc->etype) = 0;
  SPEC_STAT (sloc->etype) = 0;
  SPEC_VOLATILE (sloc->etype) = 0;
  SPEC_ABSA (sloc->etype) = 0;

  /* we don't allow it to be allocated
     onto the external stack since : so we
     temporarily turn it off ; we also
     turn off memory model to prevent
     the spil from going to the external storage
   */

  useXstack = options.useXstack;
  model = options.model;
/*     noOverlay = options.noOverlay; */
/*     options.noOverlay = 1; */
  options.model = options.useXstack = 0;

  allocLocal (sloc);

  options.useXstack = useXstack;
  options.model = model;
/*     options.noOverlay = noOverlay; */
  sloc->isref = 1;              /* to prevent compiler warning */

  /* if it is on the stack then update the stack */
  if (IN_STACK (sloc->etype))
    {
      currFunc->stack += getSize (sloc->type);
      _G.stackExtend += getSize (sloc->type);
    }
  else
    _G.dataExtend += getSize (sloc->type);

  /* add it to the _G.stackSpil set */
  addSetHead (&_G.stackSpil, sloc);
  sym->usl.spillLoc = sloc;
  sym->stackSpil = 1;

  /* add it to the set of itempStack set
     of the spill location */
  addSetHead (&sloc->usl.itmpStack, sym);
  return sym;
}

/*-----------------------------------------------------------------*/
/* isSpiltOnStack - returns true if the spil location is on stack  */
/*                  or otherwise needs a pointer register          */
/*-----------------------------------------------------------------*/
static bool
isSpiltOnStack (symbol * sym)
{
  sym_link *etype;

  if (!sym)
    return FALSE;

  if (!sym->isspilt)
    return FALSE;

/*     if (sym->_G.stackSpil) */
/*      return TRUE; */

  if (!sym->usl.spillLoc)
    return FALSE;

  if (sym->usl.spillLoc->onStack || sym->usl.spillLoc->iaccess)
    return TRUE;

  etype = getSpec (sym->usl.spillLoc->type);
  if (IN_STACK (etype))
    return TRUE;

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* spillThis - spils a specific operand                            */
/*-----------------------------------------------------------------*/
static void
spillThis (symbol * sym)
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

  bitVectUnSetBit (_G.regAssigned, sym->key);
  bitVectUnSetBit (_G.totRegAssigned, sym->key);

  for (i = 0; i < sym->nRegs; i++)
    {
      if (sym->regs[i])
        {
          freeReg (sym->regs[i]);
          sym->regs[i] = NULL;
        }
    }

  /* if spilt on stack then free up r0 & r1
     if they could have been assigned to some
     LIVE ranges */
  if (!mcs51_ptrRegReq && isSpiltOnStack (sym))
    {
      spillLRWithPtrReg (sym);
      mcs51_ptrRegReq++;
    }

  if (sym->usl.spillLoc && !sym->remat)
    sym->usl.spillLoc->allocreq++;
  return;
}

/*-----------------------------------------------------------------*/
/* selectSpil - select a iTemp to spil : rather a simple procedure */
/*-----------------------------------------------------------------*/
static symbol *
selectSpil (iCode * ic, eBBlock * ebp, symbol * forSym)
{
  bitVect *lrcs = NULL;
  set *selectS;
  symbol *sym;

  /* get the spillable live ranges */
  lrcs = computeSpillable (ic);

  /* remove incompatible registers */
  if ((forSym->regType == REG_PTR) || (forSym->regType == REG_GPR))
    {
      selectS = liveRangesWith (lrcs, bitType, ebp, ic);

      for (sym = setFirstItem (selectS); sym; sym = setNextItem (selectS))
        {
          bitVectUnSetBit (lrcs, sym->key);
        }
    }

  /* get all live ranges that are rematerializable */
  if ((selectS = liveRangesWith (lrcs, rematable, ebp, ic)))
    {
      /* return the least used of these */
      return leastUsedLR (selectS);
    }

  /* get live ranges with spillLocations in direct space */
  if ((selectS = liveRangesWith (lrcs, directSpilLoc, ebp, ic)))
    {
      sym = leastUsedLR (selectS);
      strncpyz (sym->rname,
                sym->usl.spillLoc->rname[0] ? sym->usl.spillLoc->rname : sym->usl.spillLoc->name, sizeof (sym->rname));
      sym->spildir = 1;
      /* mark it as allocation required */
      sym->usl.spillLoc->allocreq++;
      return sym;
    }

  /* if the symbol is local to the block then */
  if (forSym->liveTo < ebp->lSeq)
    {
      /* check if there are any live ranges allocated
         to registers that are not used in this block */
      if (!_G.blockSpil && (selectS = liveRangesWith (lrcs, notUsedInBlock, ebp, ic)))
        {
          sym = leastUsedLR (selectS);
          /* if this is not rematerializable */
          if (!sym->remat)
            {
              _G.blockSpil++;
              sym->blockSpil = 1;
            }
          return sym;
        }

      /* check if there are any live ranges that are
         not used in the remainder of the block */
      if (!_G.blockSpil && !isiCodeInFunctionCall (ic) && (selectS = liveRangesWith (lrcs, notUsedInRemaining, ebp, ic)))
        {
          sym = leastUsedLR (selectS);
          if (sym != forSym)
            {
              if (!sym->remat)
                {
                  sym->remainSpil = 1;
                  _G.blockSpil++;
                }
              return sym;
            }
        }
    }

  /* find live ranges with spillocation && not used as pointers */
  if ((selectS = liveRangesWith (lrcs, hasSpilLocnoUptr, ebp, ic)))
    {
      sym = leastUsedLR (selectS);
      /* mark this as allocation required */
      sym->usl.spillLoc->allocreq++;
      return sym;
    }

  /* find live ranges with spillocation */
  if ((selectS = liveRangesWith (lrcs, hasSpilLoc, ebp, ic)))
    {
      sym = leastUsedLR (selectS);
      sym->usl.spillLoc->allocreq++;
      return sym;
    }

  /* couldn't find then we need to create a spil
     location on the stack, for which one?
     the least used ofcourse */
  if ((selectS = liveRangesWith (lrcs, noSpilLoc, ebp, ic)))
    {
      /* return a created spil location */
      sym = createStackSpil (leastUsedLR (selectS));
      sym->usl.spillLoc->allocreq++;
      return sym;
    }

  /* this is an extreme situation we will spill
     this one : happens very rarely but it does happen */
  spillThis (forSym);
  return forSym;
}

/*-----------------------------------------------------------------*/
/* spilSomething - spil some variable & mark registers as free     */
/*-----------------------------------------------------------------*/
static bool
spilSomething (iCode * ic, eBBlock * ebp, symbol * forSym)
{
  symbol *ssym;
  int i;

  /* get something we can spil */
  ssym = selectSpil (ic, ebp, forSym);

  /* mark it as spilt */
  ssym->isspilt = ssym->spillA = 1;
  _G.spiltSet = bitVectSetBit (_G.spiltSet, ssym->key);

  /* mark it as not register assigned &
     take it away from the set */
  bitVectUnSetBit (_G.regAssigned, ssym->key);
  bitVectUnSetBit (_G.totRegAssigned, ssym->key);

  /* mark the registers as free */
  for (i = 0; i < ssym->nRegs; i++)
    if (ssym->regs[i])
      freeReg (ssym->regs[i]);

  /* if spilt on stack then free up r0 & r1
     if they could have been assigned to as gprs */
  if (!mcs51_ptrRegReq && isSpiltOnStack (ssym))
    {
      spillLRWithPtrReg (ssym);
      mcs51_ptrRegReq++;
    }

  /* if this was a block level spil then insert push & pop
     at the start & end of block respectively */
  if (ssym->blockSpil)
    {
      iCode *nic = newiCode (IPUSH, operandFromSymbol (ssym), NULL);
      /* add push to the start of the block */
      addiCodeToeBBlock (ebp, nic, (ebp->sch->op == LABEL ? ebp->sch->next : ebp->sch));
      nic = newiCode (IPOP, operandFromSymbol (ssym), NULL);
      /* add pop to the end of the block */
      addiCodeToeBBlock (ebp, nic, NULL);
    }

  /* if spilt because not used in the remainder of the
     block then add a push before this instruction and
     a pop at the end of the block */
  if (ssym->remainSpil)
    {
      iCode *nic = newiCode (IPUSH, operandFromSymbol (ssym), NULL);
      /* add push just before this instruction */
      addiCodeToeBBlock (ebp, nic, ic);

      nic = newiCode (IPOP, operandFromSymbol (ssym), NULL);
      /* add pop to the end of the block */
      addiCodeToeBBlock (ebp, nic, NULL);
    }

  if (ssym == forSym)
    return FALSE;
  else
    return TRUE;
}

/*-----------------------------------------------------------------*/
/* getRegPtr - will try for PTR if not a GPR type if not spil      */
/*-----------------------------------------------------------------*/
static reg_info *
getRegPtr (iCode * ic, eBBlock * ebp, symbol * sym)
{
  reg_info *reg;
  int j;

tryAgain:
  /* try for a ptr type */
  if ((reg = allocReg (REG_PTR)))
    return reg;

  /* try for gpr type */
  if ((reg = allocReg (REG_GPR)))
    return reg;

  /* we have to spil */
  if (!spilSomething (ic, ebp, sym))
    return NULL;

  /* make sure partially assigned registers aren't reused */
  for (j = 0; j <= sym->nRegs; j++)
    if (sym->regs[j])
      sym->regs[j]->isFree = 0;

  /* this looks like an infinite loop but
     in reality selectSpil will abort  */
  goto tryAgain;
}

/*-----------------------------------------------------------------*/
/* getRegGpr - will try for GPR if not spil                        */
/*-----------------------------------------------------------------*/
static reg_info *
getRegGpr (iCode * ic, eBBlock * ebp, symbol * sym)
{
  reg_info *reg;
  int j;

tryAgain:
  /* try for gpr type */
  if ((reg = allocReg (REG_GPR)))
    return reg;

  if (!mcs51_ptrRegReq)
    if ((reg = allocReg (REG_PTR)))
      return reg;

  /* we have to spil */
  if (!spilSomething (ic, ebp, sym))
    return NULL;

  /* make sure partially assigned registers aren't reused */
  for (j = 0; j <= sym->nRegs; j++)
    if (sym->regs[j])
      sym->regs[j]->isFree = 0;

  /* this looks like an infinite loop but
     in reality selectSpil will abort  */
  goto tryAgain;
}

/*-----------------------------------------------------------------*/
/* getRegBit - will try for Bit if not spill this                  */
/*-----------------------------------------------------------------*/
static reg_info *
getRegBit (symbol * sym)
{
  reg_info *reg;

  /* try for a bit type */
  if ((reg = allocReg (REG_BIT)))
    return reg;

  spillThis(sym);
  return 0;
}

/*-----------------------------------------------------------------*/
/* getRegPtrNoSpil - get it cannot be spilt                        */
/*-----------------------------------------------------------------*/
static reg_info *
getRegPtrNoSpil ()
{
  reg_info *reg;

  /* try for a ptr type */
  if ((reg = allocReg (REG_PTR)))
    return reg;

  /* try for gpr type */
  if ((reg = allocReg (REG_GPR)))
    return reg;

  assert (0);

  /* just to make the compiler happy */
  return 0;
}

/*-----------------------------------------------------------------*/
/* getRegGprNoSpil - get it cannot be spilt                        */
/*-----------------------------------------------------------------*/
static reg_info *
getRegGprNoSpil ()
{
  reg_info *reg;
  if ((reg = allocReg (REG_GPR)))
    return reg;

  if (!mcs51_ptrRegReq)
    if ((reg = allocReg (REG_PTR)))
      return reg;

  assert (0);

  /* just to make the compiler happy */
  return 0;
}

/*-----------------------------------------------------------------*/
/* getRegBitNoSpil - get it cannot be spilt                        */
/*-----------------------------------------------------------------*/
static reg_info *
getRegBitNoSpil ()
{
  reg_info *reg;

  /* try for a bit type */
  if ((reg = allocReg (REG_BIT)))
    return reg;

  /* try for gpr type */
  if ((reg = allocReg (REG_GPR)))
    return reg;

  assert (0);

  /* just to make the compiler happy */
  return 0;
}

/*-----------------------------------------------------------------*/
/* symHasReg - symbol has a given register                         */
/*-----------------------------------------------------------------*/
static bool
symHasReg (symbol * sym, reg_info * reg)
{
  int i;

  for (i = 0; i < sym->nRegs; i++)
    if (sym->regs[i] == reg)
      return TRUE;

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* updateRegUsage -  update the registers in use at the start of   */
/*                   this icode                                    */
/*-----------------------------------------------------------------*/
static void
updateRegUsage (iCode * ic)
{
  int reg;

  for (reg = 0; reg < mcs51_nRegs; reg++)
    {
      if (regs8051[reg].isFree)
        {
          ic->riu &= ~(1 << regs8051[reg].offset);
        }
      else
        {
          ic->riu |= (1 << regs8051[reg].offset);
          BitBankUsed |= (reg >= 8);
        }
    }
}

/*-----------------------------------------------------------------*/
/* deassignLRs - check the live to and if they have registers & are */
/*               not spilt then free up the registers              */
/*-----------------------------------------------------------------*/
static void
deassignLRs (iCode * ic, eBBlock * ebp)
{
  symbol *sym;
  int k;
  symbol *result;

  for (sym = hTabFirstItem (liveRanges, &k); sym; sym = hTabNextItem (liveRanges, &k))
    {
      symbol *psym = NULL;
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

      if (!bitVectBitValue (_G.regAssigned, sym->key))
        continue;

      /* special case check if this is an IFX &
         the privious one was a pop and the
         previous one was not spilt then keep track
         of the symbol */
      if (ic->op == IFX && ic->prev && ic->prev->op == IPOP && !ic->prev->parmPush && !OP_SYMBOL (IC_LEFT (ic->prev))->isspilt)
        psym = OP_SYMBOL (IC_LEFT (ic->prev));

      if (sym->nRegs)
        {
          int i = 0;

          bitVectUnSetBit (_G.regAssigned, sym->key);

          /* if the result of this one needs registers
             and does not have it then assign it right
             away */
          if (IC_RESULT (ic) &&
              !( SKIP_IC2 (ic)       ||                 /* not a special icode */
                 ic->op == JUMPTABLE ||
                 ic->op == IFX       ||
                 ic->op == IPUSH     ||
                 ic->op == IPOP      ||
                 ic->op == RETURN    ||
                 POINTER_SET (ic)      ) &&
              (result = OP_SYMBOL (IC_RESULT (ic))) &&  /* has a result */
              result->liveTo > ic->seq &&               /* and will live beyond this */
              result->liveTo <= ebp->lSeq &&            /* does not go beyond this block */
              result->liveFrom == ic->seq &&            /* does not start before here */
              result->regType == sym->regType &&        /* same register types */
              result->nRegs &&                          /* which needs registers */
              !result->isspilt &&                       /* and does not already have them */
              !result->remat &&
              !bitVectBitValue (_G.regAssigned, result->key) &&
              /* the number of free regs + number of regs in this LR
                 can accomodate the what result Needs */
              ((nfreeRegsType (result->regType) + sym->nRegs) >= result->nRegs))
            {
              for (i = 0; i < result->nRegs; i++)
                if (i < sym->nRegs)
                  result->regs[i] = sym->regs[i];
                else
                  result->regs[i] = getRegGpr (ic, ebp, result);

              _G.regAssigned = bitVectSetBit (_G.regAssigned, result->key);
              _G.totRegAssigned = bitVectSetBit (_G.totRegAssigned, result->key);
            }

          /* free the remaining */
          for (; i < sym->nRegs; i++)
            {
              if (psym)
                {
                  if (!symHasReg (psym, sym->regs[i]))
                    freeReg (sym->regs[i]);
                }
              else
                freeReg (sym->regs[i]);
            }
        }
    }
}


/*-----------------------------------------------------------------*/
/* reassignLR - reassign this to registers                         */
/*-----------------------------------------------------------------*/
static void
reassignLR (operand * op)
{
  symbol *sym = OP_SYMBOL (op);
  int i;

  /* not spilt any more */
  sym->isspilt = sym->spillA = sym->blockSpil = sym->remainSpil = 0;
  bitVectUnSetBit (_G.spiltSet, sym->key);

  _G.regAssigned = bitVectSetBit (_G.regAssigned, sym->key);
  _G.totRegAssigned = bitVectSetBit (_G.totRegAssigned, sym->key);

  _G.blockSpil--;

  for (i = 0; i < sym->nRegs; i++)
    sym->regs[i]->isFree = 0;
}

/*-----------------------------------------------------------------*/
/* willCauseSpill - determines if allocating will cause a spill    */
/*-----------------------------------------------------------------*/
static int
willCauseSpill (int nr, int rt)
{
  /* first check if there are any available registers
     of the type required */
  if (rt == REG_PTR)
    {
      /* special case for pointer type
         if pointer type not available then
         check for type gpr */
      if (nFreeRegs (rt) >= nr)
        return 0;
      if (nFreeRegs (REG_GPR) >= nr)
        return 0;
    }
  else if (rt == REG_BIT)
    {
      if (nFreeRegs (rt) >= nr)
        return 0;
    }
  else
    {
      if (mcs51_ptrRegReq)
        {
          if (nFreeRegs (rt) >= nr)
            return 0;
        }
      else
        {
          if (nFreeRegs (REG_PTR) + nFreeRegs (REG_GPR) >= nr)
            return 0;
        }
    }

  /* it will cause a spil */
  return 1;
}

/*-----------------------------------------------------------------*/
/* positionRegs - the allocator can allocate same registers to     */
/* result and operand, if this happens make sure they are in the   */
/* same position as the operand otherwise chaos results            */
/*-----------------------------------------------------------------*/
static int
positionRegs (symbol * result, symbol * opsym, int chOp)
{
  int count = min (result->nRegs, opsym->nRegs);
  int i, j = 0, shared = 0;
  int change = 0;

  /* if the result has been spilt then cannot share */
  if (opsym->isspilt)
    return 0;

  for (;;)
    {
      shared = 0;
      /* first make sure that they actually share */
      for (i = 0; i < count; i++)
        for (j = 0; j < count; j++)
          if (result->regs[i] == opsym->regs[j] && i != j)
            {
              shared = 1;
              goto xchgPositions;
            }
xchgPositions:
      if (shared)
        if (!chOp)
          {
            reg_info *tmp = result->regs[i];
            result->regs[i] = result->regs[j];
            result->regs[j] = tmp;
            change++;
          }
        else
          {
            reg_info *tmp = opsym->regs[i];
            opsym->regs[i] = opsym->regs[j];
            opsym->regs[j] = tmp;
            change++;
          }
      else
        return change;
    }
}

/*------------------------------------------------------------------*/
/* verifyRegsAssigned - make sure an iTemp is properly initialized; */
/* it should either have registers or have beed spilled. Otherwise, */
/* there was an uninitialized variable, so just spill this to get   */
/* the operand in a valid state.                                    */
/*------------------------------------------------------------------*/
static void
verifyRegsAssigned (operand * op, iCode * ic)
{
  symbol *sym;

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

  werrorfl (ic->filename, ic->lineno, W_LOCAL_NOINIT, sym->prereqv ? sym->prereqv->name : sym->name);
  spillThis (sym);
}


/*-----------------------------------------------------------------*/
/* serialRegAssign - serially allocate registers to the variables  */
/*-----------------------------------------------------------------*/
static void
serialRegAssign (eBBlock ** ebbs, int count)
{
  int i;

  /* for all blocks */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath && (ebbs[i]->entryLabel != entryLabel && ebbs[i]->entryLabel != returnLabel))
        continue;

      /* for all instructions do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          updateRegUsage (ic);

          /* if this is an ipop that means some live
             range will have to be assigned again */
          if (ic->op == IPOP)
            reassignLR (IC_LEFT (ic));

          /* if result is present && is a true symbol */
          if (IC_RESULT (ic) && ic->op != IFX && IS_TRUE_SYMOP (IC_RESULT (ic)))
            {
              OP_SYMBOL (IC_RESULT (ic))->allocreq++;
            }

          /* take away registers from live
             ranges that end at this instruction */
          deassignLRs (ic, ebbs[i]);

          /* some don't need registers */
          if (SKIP_IC2 (ic)       ||
              ic->op == JUMPTABLE ||
              ic->op == IFX       ||
              ic->op == IPUSH     ||
              ic->op == IPOP      ||
              (IC_RESULT (ic) && POINTER_SET (ic)))
            {
              continue;
            }

          /* now we need to allocate registers only for the result */
          if (IC_RESULT (ic))
            {
              symbol *sym = OP_SYMBOL (IC_RESULT (ic));
              bitVect *spillable;
              int willCS;
              int j;
              int ptrRegSet = 0;

              /* Make sure any spill location is definitely allocated */
              if (sym->isspilt && !sym->remat && sym->usl.spillLoc && !sym->usl.spillLoc->allocreq)
                {
                  sym->usl.spillLoc->allocreq++;
                }

              /* if it does not need or is spilt
                 or is already assigned to registers
                 or will not live beyond this instructions */
              if (!sym->nRegs || sym->isspilt || bitVectBitValue (_G.regAssigned, sym->key) || sym->liveTo <= ic->seq)
                {
                  continue;
                }

              /* do not try to spil bit registers as it won't work */
              if (sym->regType != REG_BIT)
                {
                  /* if some liverange has been spilt at the block level
                     and this one live beyond this block then spil this
                     to be safe */
                  if (_G.blockSpil && sym->liveTo > ebbs[i]->lSeq)
                    {
                      spillThis (sym);
                      continue;
                    }

                  willCS = willCauseSpill (sym->nRegs, sym->regType);
                  /* if this is a bit variable then don't use precious registers
                     along with expensive bit-to-char conversions but just spill
                     it */
                  if (willCS && SPEC_NOUN (sym->etype) == V_BIT)
                    {
                      spillThis (sym);
                      continue;
                    }

                  /* if trying to allocate this will cause
                     a spill and there is nothing to spill
                     or this one is rematerializable then
                     spill this one */
                  spillable = computeSpillable (ic);
                  if (sym->remat || (willCS && bitVectIsZero (spillable)))
                    {
                      spillThis (sym);
                      continue;
                    }

                  /* If the live range preceeds the point of definition
                     then ideally we must take into account registers that
                     have been allocated after sym->liveFrom but freed
                     before ic->seq. This is complicated, so spill this
                     symbol instead and let fillGaps handle the allocation. */
                  if (sym->liveFrom < ic->seq)
                    {
                      spillThis (sym);
                      continue;
                    }

                  /* if it has a spillocation & is used less than
                     all other live ranges then spill this */
                  if (willCS)
                    {
                      if (sym->usl.spillLoc)
                        {
                          symbol *leastUsed = leastUsedLR (liveRangesWith (spillable,
                                                                           allLRs, ebbs[i], ic));
                          if (leastUsed && leastUsed->used > sym->used)
                            {
                              spillThis (sym);
                              continue;
                            }
                        }
                      else
                        {
                          /* if none of the liveRanges have a spillLocation then better
                             to spill this one than anything else already assigned to registers */
                          if (liveRangesWith (spillable, noSpilLoc, ebbs[i], ic))
                            {
                              /* if this is local to this block then we might find a block spil */
                              if (!(sym->liveFrom >= ebbs[i]->fSeq && sym->liveTo <= ebbs[i]->lSeq))
                                {
                                  spillThis (sym);
                                  continue;
                                }
                            }
                        }
                    }
                }
              /* if we need ptr regs for the right side
                 then mark it */
              if (POINTER_GET (ic) && IS_SYMOP (IC_LEFT (ic))
                  && getSize (OP_SYMBOL (IC_LEFT (ic))->type) <= (unsigned int) NEARPTRSIZE)
                {
                  mcs51_ptrRegReq++;
                  ptrRegSet = 1;
                }
              if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)) && SPEC_OCLS (OP_SYMBOL (IC_LEFT (ic))->etype) == idata)
                {
                  mcs51_ptrRegReq++;
                  ptrRegSet = 1;
                }
              if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)) && SPEC_OCLS (OP_SYMBOL (IC_RIGHT (ic))->etype) == idata)
                {
                  mcs51_ptrRegReq++;
                  ptrRegSet = 1;
                }

              /* else we assign registers to it */
              _G.regAssigned = bitVectSetBit (_G.regAssigned, sym->key);
              _G.totRegAssigned = bitVectSetBit (_G.totRegAssigned, sym->key);

              for (j = 0; j < sym->nRegs; j++)
                {
                  sym->regs[j] = NULL;
                  if (sym->regType == REG_PTR)
                    sym->regs[j] = getRegPtr (ic, ebbs[i], sym);
                  else
                    {
                      if (ic->op == CAST && IS_SYMOP (IC_RIGHT (ic)))
                        {
                          symbol *right = OP_SYMBOL (IC_RIGHT (ic));

                          if (right->regs[j] && (right->regType != REG_BIT))
                            sym->regs[j] = allocThisReg (right->regs[j]);
                        }
                      if (!sym->regs[j])
                        {
                          if (sym->regType == REG_BIT) /* Prefer spilling over a GPR */
                            sym->regs[j] = getRegBit (sym);
                          else
                            sym->regs[j] = getRegGpr (ic, ebbs[i], sym);
                        }
                    }

                  /* if the allocation failed which means
                     this was spilt then break */
                  if (!sym->regs[j])
                    {
                      int i;
                      for (i = 0; i < sym->nRegs; i++)
                        sym->regs[i] = NULL;
                      break;
                    }
                }

              /* for debugging prefer to keep the sym in ascending
                 registers so sort them by address */
              if (sym->regs[0])
                {
                  for (j = 0; j < sym->nRegs - 1; j++)
                    {
                      int k;
                      for (k=j+1; k<sym->nRegs; k++)
                        {
                          if (sym->regs[j]->offset > sym->regs[k]->offset)
                            {
                              reg_info *tmp = sym->regs[j];
                              sym->regs[j] = sym->regs[k];
                              sym->regs[k] = tmp;
                            }
                        }
                    }
                }

              if (!POINTER_SET (ic) && !POINTER_GET (ic))
                {
                  /* if it shares registers with operands make sure
                     that they are in the same position */
                  if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)) && OP_SYMBOL (IC_LEFT (ic))->nRegs)
                    {
                      positionRegs (OP_SYMBOL (IC_RESULT (ic)), OP_SYMBOL (IC_LEFT (ic)), 0);
                    }
                  /* do the same for the right operand */
                  if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)) && OP_SYMBOL (IC_RIGHT (ic))->nRegs)
                    {
                      positionRegs (OP_SYMBOL (IC_RESULT (ic)), OP_SYMBOL (IC_RIGHT (ic)), 0);
                    }
                }

              if (ptrRegSet)
                {
                  mcs51_ptrRegReq--;
                  ptrRegSet = 0;
                }
            }
        }
    }

  /* Check for and fix any problems with uninitialized operands */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath && (ebbs[i]->entryLabel != entryLabel && ebbs[i]->entryLabel != returnLabel))
        {
          continue;
        }

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

/*-----------------------------------------------------------------*/
/* fillGaps - Try to fill in the Gaps left by Pass1                */
/*-----------------------------------------------------------------*/
static void
fillGaps (void)
{
  symbol *sym = NULL;
  int key = 0;
  int pass;
  iCode *ic = NULL;

  if (getenv ("DISABLE_FILL_GAPS"))
    return;

  /* look for liveranges that were spilt by the allocator */
  for (sym = hTabFirstItem (liveRanges, &key); sym; sym = hTabNextItem (liveRanges, &key))
    {
      int i;
      int pdone = 0;

      if (!sym->spillA || !sym->clashes || sym->remat)
        continue;

      /* if spilt in direct space the original rname is lost */
      if (sym->usl.spillLoc && (IN_DIRSPACE (SPEC_OCLS (sym->usl.spillLoc->etype))))
        continue;

      /* find the liveRanges this one clashes with, that are
         still assigned to registers & mark the registers as used */
      for (i = 0; i < sym->clashes->size; i++)
        {
          int k;
          symbol *clr;

          if (bitVectBitValue (sym->clashes, i) == 0 || /* those that clash with this */
              bitVectBitValue (_G.totRegAssigned, i) == 0)      /* and are still assigned to registers */
            continue;

          clr = hTabItemWithKey (liveRanges, i);
          assert (clr);

          /* mark these registers as used */
          for (k = 0; k < clr->nRegs; k++)
            useReg (clr->regs[k]);
        }

      if (willCauseSpill (sym->nRegs, sym->regType))
        {
          /* NOPE :( clear all registers & and continue */
          freeAllRegs ();
          continue;
        }

      ic = NULL;
      for (i = 0; i < sym->defs->size; i++)
        {
          if (bitVectBitValue (sym->defs, i))
            {
              if (!(ic = hTabItemWithKey (iCodehTab, i)))
                continue;
              if (ic->op == CAST)
                break;
            }
        }

      D (printf ("Attempting fillGaps on %s: [", sym->name));
      /* THERE IS HOPE !!!! */
      for (i = 0; i < sym->nRegs; i++)
        {
          if (sym->regType == REG_PTR)
            sym->regs[i] = getRegPtrNoSpil ();
          else if (sym->regType == REG_BIT)
            sym->regs[i] = getRegBitNoSpil ();
          else
            {
              sym->regs[i] = NULL;
              if (ic && ic->op == CAST && IS_SYMOP (IC_RIGHT (ic)))
                {
                  symbol *right = OP_SYMBOL (IC_RIGHT (ic));

                  if (right->regs[i] && right->regs[i]->type != REG_BIT)
                    sym->regs[i] = allocThisReg (right->regs[i]);
                }
              if (!sym->regs[i])
                sym->regs[i] = getRegGprNoSpil ();
            }
          D (printf ("%s ", sym->regs[i]->name));
        }
      D (printf ("]\n"));

      /* For all its definitions check if the registers
         allocated needs positioning NOTE: we can position
         only ONCE if more than One positioning required
         then give up.
         We may need to perform the checks twice; once to
         position the registers as needed, the second to
         verify any register repositioning is still
         compatible.
       */
      sym->isspilt = 0;
      for (pass = 0; pass < 2; pass++)
        {
          D (printf (" checking definitions\n"));
          for (i = 0; i < sym->defs->size; i++)
            {
              if (bitVectBitValue (sym->defs, i))
                {
                  if (!(ic = hTabItemWithKey (iCodehTab, i)))
                    continue;
                  D (printf ("  ic->seq = %d\n", ic->seq));
                  if (SKIP_IC (ic))
                    continue;
                  assert (isSymbolEqual (sym, OP_SYMBOL (IC_RESULT (ic))));     /* just making sure */
                  /* if left is assigned to registers */
                  if (IS_SYMOP (IC_LEFT (ic)))
                    {
                      D (printf ("   left = "));
                      D (printOperand (IC_LEFT (ic), NULL));
                    }
                  if (IS_SYMOP (IC_LEFT (ic)) && bitVectBitValue (_G.totRegAssigned, OP_SYMBOL (IC_LEFT (ic))->key))
                    {
                      pdone += (positionRegs (sym, OP_SYMBOL (IC_LEFT (ic)), 0) > 0);
                    }
                  if (IS_SYMOP (IC_RIGHT (ic)))
                    {
                      D (printf ("   right = "));
                      D (printOperand (IC_RIGHT (ic), NULL));
                    }
                  if (IS_SYMOP (IC_RIGHT (ic)) && bitVectBitValue (_G.totRegAssigned, OP_SYMBOL (IC_RIGHT (ic))->key))
                    {
                      pdone += (positionRegs (sym, OP_SYMBOL (IC_RIGHT (ic)), 0) > 0);
                    }
                  D (printf ("   pdone = %d\n", pdone));
                  if (pdone > 1)
                    break;
                }
            }
          D (printf (" checking uses\n"));
          for (i = 0; i < sym->uses->size; i++)
            {
              if (bitVectBitValue (sym->uses, i))
                {
                  iCode *ic;
                  if (!(ic = hTabItemWithKey (iCodehTab, i)))
                    continue;
                  D (printf ("  ic->seq = %d\n", ic->seq));
                  if (SKIP_IC (ic))
                    continue;
                  if (POINTER_SET (ic) || POINTER_GET (ic))
                    continue;

                  /* if result is assigned to registers */
                  if (IS_SYMOP (IC_RESULT (ic)))
                    {
                      D (printf ("   result = "));
                      D (printOperand (IC_RESULT (ic), NULL));
                    }
                  if (IS_SYMOP (IC_RESULT (ic)) && bitVectBitValue (_G.totRegAssigned, OP_SYMBOL (IC_RESULT (ic))->key))
                    {
                      pdone += (positionRegs (sym, OP_SYMBOL (IC_RESULT (ic)), 0) > 0);
                    }
                  D (printf ("   pdone = %d\n", pdone));
                  if (pdone > 1)
                    break;
                }
            }
          if (pdone == 0)
            break;              /* second pass only if regs repositioned */
          if (pdone > 1)
            break;
        }
      D (printf (" sym->regs = ["));
      for (i = 0; i < sym->nRegs; i++)
        D (printf ("%s ", sym->regs[i]->name));
      D (printf ("]\n"));
      /* had to position more than once GIVE UP */
      if (pdone > 1)
        {
          /* UNDO all the changes we made to try this */
          sym->isspilt = 1;
          for (i = 0; i < sym->nRegs; i++)
            {
              sym->regs[i] = NULL;
            }
          freeAllRegs ();
          D (printf
             ("Fill Gap gave up due to positioning for %s in function %s\n", sym->name, currFunc ? currFunc->name : "UNKNOWN"));
          continue;
        }
      D (printf ("FILLED GAP for %s in function %s\n", sym->name, currFunc ? currFunc->name : "UNKNOWN"));

      _G.totRegAssigned = bitVectSetBit (_G.totRegAssigned, sym->key);
      sym->isspilt = sym->spillA = 0;
      sym->usl.spillLoc->allocreq--;
      freeAllRegs ();
    }
}

/*-----------------------------------------------------------------*/
/* findAllBitregs :- returns bit vector of all bit registers       */
/*-----------------------------------------------------------------*/
static bitVect *
findAllBitregs (void)
{
  bitVect *rmask = newBitVect (mcs51_nRegs);
  int j;

  for (j = 0; j < mcs51_nRegs; j++)
    {
      if (regs8051[j].type == REG_BIT)
        rmask = bitVectSetBit (rmask, regs8051[j].rIdx);
    }

  return rmask;
}

/*-----------------------------------------------------------------*/
/* mcs51_allBitregs :- returns bit vector of all bit registers     */
/*-----------------------------------------------------------------*/
bitVect *
mcs51_allBitregs (void)
{
  return _G.allBitregs;
}

/*-----------------------------------------------------------------*/
/* findAllBankregs :- returns bit vector of all bank registers     */
/*-----------------------------------------------------------------*/
static bitVect *
findAllBankregs (void)
{
  bitVect *rmask = newBitVect (mcs51_nRegs);
  int j;

  for (j = 0; j < mcs51_nRegs; j++)
    {
      if ((regs8051[j].type == REG_GPR) || (regs8051[j].type == REG_PTR))
        rmask = bitVectSetBit (rmask, regs8051[j].rIdx);
    }

  return rmask;
}

/*-----------------------------------------------------------------*/
/* mcs51_allBankregs :- returns bit vector of all bank registers   */
/*-----------------------------------------------------------------*/
bitVect *
mcs51_allBankregs (void)
{
  return _G.allBankregs;
}

/*-----------------------------------------------------------------*/
/* rUmaskForOp :- returns register mask for an operand             */
/*-----------------------------------------------------------------*/
bitVect *
mcs51_rUmaskForOp (operand * op)
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

  rumask = newBitVect (mcs51_nRegs);

  for (j = 0; j < sym->nRegs; j++)
    {
      if (sym->regs[j])         /* EEP - debug */
        rumask = bitVectSetBit (rumask, sym->regs[j]->rIdx);
    }

  return rumask;
}

/*-----------------------------------------------------------------*/
/* regsUsedIniCode :- returns bit vector of registers used in iCode */
/*-----------------------------------------------------------------*/
static bitVect *
regsUsedIniCode (iCode * ic)
{
  bitVect *rmask = newBitVect (mcs51_nRegs);

  /* do the special cases first */
  if (ic->op == IFX)
    {
      rmask = bitVectUnion (rmask, mcs51_rUmaskForOp (IC_COND (ic)));
      goto ret;
    }

  /* for the jumptable */
  if (ic->op == JUMPTABLE)
    {
      rmask = bitVectUnion (rmask, mcs51_rUmaskForOp (IC_JTCOND (ic)));
      goto ret;
    }

  /* of all other cases */
  if (IC_LEFT (ic))
    rmask = bitVectUnion (rmask, mcs51_rUmaskForOp (IC_LEFT (ic)));

  if (IC_RIGHT (ic))
    rmask = bitVectUnion (rmask, mcs51_rUmaskForOp (IC_RIGHT (ic)));

  if (IC_RESULT (ic))
    rmask = bitVectUnion (rmask, mcs51_rUmaskForOp (IC_RESULT (ic)));

ret:
  return rmask;
}

/*-----------------------------------------------------------------*/
/* createRegMask - for each instruction will determine the regsUsed */
/*-----------------------------------------------------------------*/
static void
createRegMask (eBBlock ** ebbs, int count)
{
  int i;

  /* for all blocks */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath && (ebbs[i]->entryLabel != entryLabel && ebbs[i]->entryLabel != returnLabel))
        continue;

      /* for all instructions */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          int j;

          if (SKIP_IC2 (ic) || !ic->rlive)
            continue;

          /* first mark the registers used in this
             instruction */
          ic->rUsed = regsUsedIniCode (ic);
          _G.funcrUsed = bitVectUnion (_G.funcrUsed, ic->rUsed);

          /* now create the register mask for those
             registers that are in use : this is a
             super set of ic->rUsed */
          ic->rMask = newBitVect (mcs51_nRegs + 1);

          /* for all live Ranges alive at this point */
          for (j = 1; j < ic->rlive->size; j++)
            {
              symbol *sym;
              int k;

              /* if not alive then continue */
              if (!bitVectBitValue (ic->rlive, j))
                continue;

              /* find the live range we are interested in */
              if (!(sym = hTabItemWithKey (liveRanges, j)))
                {
                  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "createRegMask cannot find live range");
                  fprintf (stderr, "\tmissing live range: key=%d\n", j);
                  exit (0);
                }

              /* if no register assigned to it */
              if (!sym->nRegs || sym->isspilt)
                continue;

              /* for all the registers allocated to it */
              for (k = 0; k < sym->nRegs; k++)
                if (sym->regs[k])
                  ic->rMask = bitVectSetBit (ic->rMask, sym->regs[k]->rIdx);
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* rematStr - returns the rematerialized string for a remat var    */
/*-----------------------------------------------------------------*/
static char *
rematStr (symbol * sym)
{
  iCode *ic = sym->rematiCode;
  int offset = 0;
  struct dbuf_s dbuf;

  while (1)
    {
      /* if plus adjust offset to right hand side */
      if (ic->op == '+')
        {
          offset += (int) operandLitValue (IC_RIGHT (ic));
          ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
          continue;
        }

      /* if minus adjust offset to right hand side */
      if (ic->op == '-')
        {
          offset -= (int) operandLitValue (IC_RIGHT (ic));
          ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
          continue;
        }

      /* cast then continue */
      if (IS_CAST_ICODE (ic))
        {
          ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
          continue;
        }
      /* we reached the end */
      break;
    }

  dbuf_init (&dbuf, 128);
  if (offset)
    {
      dbuf_printf (&dbuf, "(%s %c 0x%04x)", OP_SYMBOL (IC_LEFT (ic))->rname, offset >= 0 ? '+' : '-', abs (offset) & 0xffff);
    }
  else
    {
      dbuf_append_str (&dbuf, OP_SYMBOL (IC_LEFT (ic))->rname);
    }
  return dbuf_detach_c_str (&dbuf);
}

/*------------------------------------------------------------------*/
/* isBitVar - returns true if sym is a good candiate for allocation */
/*            to a bit                                              */
/*------------------------------------------------------------------*/
static bool isFlagVar (symbol *sym)
{
  if (IS_BIT (sym->type))
   return (true);

  if (!(IS_SPEC(sym->type) && SPEC_NOUN (sym->type) == V_BOOL && !sym->addrtaken))
    return (false);

  bitVect *defs = bitVectCopy (sym->defs);
  bitVect *uses = bitVectCopy (sym->uses);
  int key;
  unsigned int gooduses = 0;
  unsigned int baduses = 0;

  for (key = bitVectFirstBit (defs); key >= 0; key = bitVectFirstBit (defs))
    {
      bitVectUnSetBit (defs, key);

      iCode *ic = hTabItemWithKey (iCodehTab, key);

      if (ic->op == AND_OP || ic->op == OR_OP || ic->op == EQ_OP || ic->op == '<' || ic->op == '>' || ic->op == CAST || ic->op == '!')
        gooduses++;
      else if (ic->op == '=' &&
        (IS_OP_LITERAL (IC_RIGHT (ic)) || IS_SYMOP (IC_RIGHT (ic)) && IS_BIT (OP_SYMBOL (IC_RIGHT (ic))->type)))
        gooduses++;
      else
        baduses++;
    }

  for (key = bitVectFirstBit (uses); key >= 0; key = bitVectFirstBit (uses))
    {
      bitVectUnSetBit (uses, key);

      iCode *ic = hTabItemWithKey (iCodehTab, key);

      if (!ic) /* Shouldn't happen, but does */
        continue;

      if (ic->op == IFX || ic->op == '!' || ic->op == AND_OP || ic->op == OR_OP)
        gooduses++;
      else if (ic->op == BITWISEAND || ic->op == '|' || ic->op == '^')
        gooduses++;
      else if (ic->op == '=' && !POINTER_SET (ic) && IS_SYMOP (IC_RESULT (ic)) && IS_BIT (OP_SYMBOL (IC_RESULT (ic))->type))
        gooduses++;
      else
        baduses++;
    }

  freeBitVect (defs);
  freeBitVect (uses);

  return (gooduses >= baduses * 2);
}

/*-----------------------------------------------------------------*/
/* regTypeNum - computes the type & number of registers required   */
/*-----------------------------------------------------------------*/
static void
regTypeNum (eBBlock * ebbs)
{
  symbol *sym;
  int k;
  iCode *ic;

  /* for each live range do */
  for (sym = hTabFirstItem (liveRanges, &k); sym; sym = hTabNextItem (liveRanges, &k))
    {
      /* if used zero times then no registers needed */
      if ((sym->liveTo - sym->liveFrom) == 0)
        continue;

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
              else if (IS_BIT (sym->type))
                sym->regType = REG_CND;
              continue;
            }

          /* if the symbol has only one definition &
             that definition is a get_pointer */
          if (bitVectnBitsOn (sym->defs) == 1 &&
              (ic = hTabItemWithKey (iCodehTab, bitVectFirstBit (sym->defs))) &&
              POINTER_GET (ic) && !IS_BITVAR (sym->etype) && (aggrToPtrDclType (operandType (IC_LEFT (ic)), FALSE) == POINTER))
            {
              if (ptrPseudoSymSafe (sym, ic))
                {
                  char *s = rematStr (OP_SYMBOL (IC_LEFT (ic)));
                  ptrPseudoSymConvert (sym, ic, s);
                  Safe_free (s);
                  continue;
                }

              /* if in data space or idata space then try to
                 allocate pointer register */
            }

          /* if not then we require registers */
          sym->nRegs = ((IS_AGGREGATE (sym->type) || sym->isptr) ?
                        getSize (sym->type = aggrToPtr (sym->type, FALSE)) : getSize (sym->type));

          if (sym->nRegs > 8)
            {
              fprintf (stderr, "allocated more than 8 or 0 registers for type ");
              printTypeChain (sym->type, stderr);
              fprintf (stderr, "\n");
            }

          /* determine the type of register required */
          if (sym->nRegs == 1 && IS_PTR (sym->type) && sym->uptr)
            sym->regType = REG_PTR;
          else if (isFlagVar (sym))
            sym->regType = REG_BIT;
          else
            sym->regType = REG_GPR;
        }
      else
        /* for the first run we don't provide */
        /* registers for true symbols we will */
        /* see how things go                  */
        sym->nRegs = 0;
    }
}

/*-----------------------------------------------------------------*/
/* freeAllRegs - mark all registers as free                        */
/*-----------------------------------------------------------------*/
static void
freeAllRegs ()
{
  int i;

  for (i = 0; i < mcs51_nRegs; i++)
    regs8051[i].isFree = 1;
}

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
/* farSpacePackable - returns the packable icode for far variables */
/*-----------------------------------------------------------------*/
static iCode *
farSpacePackable (iCode * ic)
{
  iCode *dic;

  /* go thru till we find a definition for the
     symbol on the right */
  for (dic = ic->prev; dic; dic = dic->prev)
    {
      /* if the definition is a call then no */
      if ((dic->op == CALL || dic->op == PCALL) && IC_RESULT (dic)->key == IC_RIGHT (ic)->key)
        {
          return NULL;
        }

      /* if shift by unknown amount then not */
      if ((dic->op == LEFT_OP || dic->op == RIGHT_OP) && IC_RESULT (dic)->key == IC_RIGHT (ic)->key)
        return NULL;

      /* if pointer get and size > 1 */
      if (POINTER_GET (dic) && getSize (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)) > 1)
        return NULL;

      if (POINTER_SET (dic) && getSize (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)) > 1)
        return NULL;

      if (dic->op == IFX)
        {
          if (IC_COND (dic) && IS_TRUE_SYMOP (IC_COND (dic)) && isOperandInFarSpace (IC_COND (dic)))
            return NULL;
        }
      else if (dic->op == JUMPTABLE)
        {
          if (IC_JTCOND (dic) && IS_TRUE_SYMOP (IC_JTCOND (dic)) && isOperandInFarSpace (IC_JTCOND (dic)))
            return NULL;
        }
      else
        {
          /* if any tree is a true symbol in far space */
          if (IC_RESULT (dic) && IS_TRUE_SYMOP (IC_RESULT (dic)) && isOperandInFarSpace (IC_RESULT (dic)))
            return NULL;

          if (IC_RIGHT (dic) &&
              IS_TRUE_SYMOP (IC_RIGHT (dic)) &&
              isOperandInFarSpace (IC_RIGHT (dic)) && !isOperandEqual (IC_RIGHT (dic), IC_RESULT (ic)))
            return NULL;

          if (IC_LEFT (dic) &&
              IS_TRUE_SYMOP (IC_LEFT (dic)) &&
              isOperandInFarSpace (IC_LEFT (dic)) && !isOperandEqual (IC_LEFT (dic), IC_RESULT (ic)))
            return NULL;
        }

      if (isOperandEqual (IC_RIGHT (ic), IC_RESULT (dic)))
        {
          if ((dic->op == LEFT_OP || dic->op == RIGHT_OP || dic->op == '-') && IS_OP_LITERAL (IC_RIGHT (dic)))
            return NULL;
          else
            return dic;
        }
    }

  return NULL;
}

/*-----------------------------------------------------------------*/
/* packRegsForAssign - register reduction for assignment           */
/*-----------------------------------------------------------------*/
static int
packRegsForAssign (iCode * ic, eBBlock * ebp)
{
  iCode *dic, *sic;

  if (!IS_ITEMP (IC_RIGHT (ic)) || OP_SYMBOL (IC_RIGHT (ic))->isind || OP_LIVETO (IC_RIGHT (ic)) > ic->seq)
    {
      return 0;
    }

  /* if the true symbol is defined in far space or on stack
     then we should not since this will increase register pressure */
  if (isOperandInFarSpace (IC_RESULT (ic)) && !farSpacePackable (ic))
    {
      return 0;
    }

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
          if (!OP_SYMBOL (IC_RESULT (ic))->ismyparm && !OP_SYMBOL (IC_RESULT (ic))->islocal)
            {
              crossedCall = 1;
            }
        }

      if (dic->op == INLINEASM)
        {
          dic = NULL;
          break;
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
              if (POINTER_SET (dic))
                dic = NULL;
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

  /* if the result is on stack or iaccess then it must be
     the same atleast one of the operands */
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
      /* if definition by assignment */
      if (dic->op == '=' && !POINTER_SET (dic) && IC_RESULT (dic)->key == op->key
/*          &&  IS_TRUE_SYMOP(IC_RIGHT(dic)) */
        )
        break;                  /* found where this temp was defined */

      /* if we find an usage then we cannot delete it */

      if (dic->op == IFX)
        {
          if (IC_COND (dic) && IC_COND (dic)->key == op->key)
            return NULL;
        }
      else if (dic->op == JUMPTABLE)
        {
          if (IC_JTCOND (dic) && IC_JTCOND (dic)->key == op->key)
            return NULL;
        }
      else
        {
          if (IC_LEFT (dic) && IC_LEFT (dic)->key == op->key)
            return NULL;

          if (IC_RIGHT (dic) && IC_RIGHT (dic)->key == op->key)
            return NULL;

          if (POINTER_SET (dic) && IC_RESULT (dic)->key == op->key)
            return NULL;
        }
    }

  if (!dic)
    return NULL;                /* didn't find any assignment to op */

  /* we are interested only if defined in far space */
  /* or in stack space in case of + & - */

  /* if assigned to a non-symbol then don't repack regs */
  if (!IS_SYMOP (IC_RIGHT (dic)))
    return NULL;

  /* if the symbol is volatile then we should not */
  if (isOperandVolatile (IC_RIGHT (dic), TRUE))
    return NULL;
  /* XXX TODO --- should we be passing FALSE to isOperandVolatile()?
     What does it mean for an iTemp to be volatile, anyway? Passing
     TRUE is more cautious but may prevent possible optimizations */

  /* if the symbol is in far space then we should not */
  if (isOperandInFarSpace (IC_RIGHT (dic)))
    return NULL;

  /* for + & - operations make sure that
     if it is on the stack it is the same
     as one of the three operands */
  if ((ic->op == '+' || ic->op == '-') && OP_SYMBOL (IC_RIGHT (dic))->onStack)
    {
      if (IC_RESULT (ic)->key != IC_RIGHT (dic)->key &&
          IC_LEFT (ic)->key != IC_RIGHT (dic)->key && IC_RIGHT (ic)->key != IC_RIGHT (dic)->key)
        return NULL;
    }

  /* now make sure that the right side of dic
     is not defined between ic & dic */
  if (dic)
    {
      iCode *sic = dic->next;

      for (; sic != ic; sic = sic->next)
        if (IC_RESULT (sic) && IC_RESULT (sic)->key == IC_RIGHT (dic)->key)
          return NULL;
    }

  return dic;
}

/*-----------------------------------------------------------------*/
/* reassignAliasedSym - used by packRegsForSupport to replace      */
/*                      redundant iTemp with equivalent symbol     */
/*-----------------------------------------------------------------*/
static void
reassignAliasedSym (eBBlock * ebp, iCode * assignment, iCode * use, operand * op)
{
  iCode *ic;
  unsigned oldSymKey, newSymKey;

  oldSymKey = op->key;
  newSymKey = IC_RIGHT (assignment)->key;

  /* only track live ranges of compiler-generated temporaries */
  if (!IS_ITEMP (IC_RIGHT (assignment)))
    newSymKey = 0;

  /* update the live-value bitmaps */
  for (ic = assignment; ic != use; ic = ic->next)
    {
      bitVectUnSetBit (ic->rlive, oldSymKey);
      if (newSymKey != 0)
        ic->rlive = bitVectSetBit (ic->rlive, newSymKey);
    }

  /* update the sym of the used operand */
  OP_SYMBOL (op) = OP_SYMBOL (IC_RIGHT (assignment));
  op->key = OP_SYMBOL (op)->key;
  OP_SYMBOL (op)->accuse = 0;

  /* update the sym's liverange */
  if (OP_LIVETO (op) < ic->seq)
    setToRange (op, ic->seq, FALSE);

  /* remove the assignment iCode now that its result is unused */
  remiCodeFromeBBlock (ebp, assignment);
  bitVectUnSetBit (OP_SYMBOL (IC_RESULT (assignment))->defs, assignment->key);
  hTabDeleteItem (&iCodehTab, assignment->key, assignment, DELETE_ITEM, NULL);
}


/*-----------------------------------------------------------------*/
/* packRegsForSupport :- reduce some registers for support calls   */
/*-----------------------------------------------------------------*/
static int
packRegsForSupport (iCode * ic, eBBlock * ebp)
{
  iCode *dic;

  /* for the left & right operand :- look to see if the
     left was assigned a true symbol in far space in that
     case replace them */

  if (IS_ITEMP (IC_LEFT (ic)) && OP_SYMBOL (IC_LEFT (ic))->liveTo <= ic->seq)
    {
      dic = findAssignToSym (IC_LEFT (ic), ic);

      if (dic)
        {
          /* found it we need to remove it from the block */
          reassignAliasedSym (ebp, dic, ic, IC_LEFT (ic));
          return 1;
        }
    }

  /* do the same for the right operand */
  if (IS_ITEMP (IC_RIGHT (ic)) && OP_SYMBOL (IC_RIGHT (ic))->liveTo <= ic->seq)
    {
      iCode *dic = findAssignToSym (IC_RIGHT (ic), ic);

      if (dic)
        {
          /* if this is a subtraction & the result
             is a true symbol in far space then don't pack */
          if (ic->op == '-' && IS_TRUE_SYMOP (IC_RESULT (dic)))
            {
              sym_link *etype = getSpec (operandType (IC_RESULT (dic)));
              if (IN_FARSPACE (SPEC_OCLS (etype)))
                return 0;
            }
          /* found it we need to remove it from the block */
          reassignAliasedSym (ebp, dic, ic, IC_RIGHT (ic));

          return 1;
        }
    }

  return 0;
}


/*-----------------------------------------------------------------*/
/* packRegsForOneuse : - will reduce some registers for single Use */
/*-----------------------------------------------------------------*/
static iCode *
packRegsForOneuse (iCode * ic, operand * op, eBBlock * ebp)
{
  iCode *dic, *sic;
  sym_link *type;
  int usingCarry=0;

  /* if returning a literal then do nothing */
  if (!IS_ITEMP (op))
    return NULL;

  /* if rematerializable or already return use then do nothing */
  if (OP_SYMBOL (op)->remat || OP_SYMBOL (op)->ruonly)
    return NULL;

  /* only upto 2 bytes since we cannot predict
     the usage of b, & acc */
  type = operandType (op);
  if (getSize (type) > (fReturnSizeMCS51 - 2))
    return NULL;
  usingCarry = IS_BIT(type);

  if (ic->op != RETURN && ic->op != SEND && !POINTER_SET (ic) && !POINTER_GET (ic))
    return NULL;

  if (ic->op == SEND && ic->argreg != 1)
    return NULL;

  /* this routine will mark the symbol as used in one
     instruction use only && if the definition is local
     (ie. within the basic block) && has only one definition &&
     that definition is either a return value from a
     function or does not contain any variables in
     far space */
  if (bitVectnBitsOn (OP_USES (op)) > 1)
    return NULL;

  /* if it has only one definition */
  if (bitVectnBitsOn (OP_DEFS (op)) > 1)
    return NULL;                /* has more than one definition */

  /* get that definition */
  if (!(dic = hTabItemWithKey (iCodehTab, bitVectFirstBit (OP_DEFS (op)))))
    return NULL;

  /* if that only usage is a cast */
  if (dic->op == CAST)
    {
      /* to a bigger type */
      if (getSize (OP_SYM_TYPE (IC_RESULT (dic))) > getSize (OP_SYM_TYPE (IC_RIGHT (dic))))
        {
          /* then we can not, since we cannot predict the usage of b & acc */
          return NULL;
        }
    }

  /* found the definition now check if it is local */
  if (dic->seq < ebp->fSeq || dic->seq > ebp->lSeq)
    return NULL;                /* non-local */

  /* now check if it is the return from a function call */
  if (dic->op == CALL || dic->op == PCALL)
    {
      if (ic->op != SEND && ic->op != RETURN && !POINTER_SET (ic) && !POINTER_GET (ic))
        {
          OP_SYMBOL (op)->ruonly = 1;
          return dic;
        }
    }
  else
    {
      /* otherwise check that the definition does
         not contain any symbols in far space */
      if (isOperandInFarSpace (IC_LEFT (dic)) ||
          isOperandInFarSpace (IC_RIGHT (dic)) || IS_OP_RUONLY (IC_LEFT (ic)) || IS_OP_RUONLY (IC_RIGHT (ic)))
        {
          return NULL;
        }

      /* if pointer set then make sure the pointer is one byte */
      if (POINTER_SET (dic) && !IS_SMALL_PTR (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)))
        return NULL;

      if (POINTER_GET (dic) && !IS_SMALL_PTR (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)))
        return NULL;
    }

  /* Make sure no overlapping liverange is already assigned to DPTR */
  if (OP_SYMBOL (op)->clashes)
    {
      symbol *sym;
      int i;

      for (i = 0; i < OP_SYMBOL (op)->clashes->size; i++)
        {
          if (bitVectBitValue (OP_SYMBOL (op)->clashes, i))
            {
              sym = hTabItemWithKey (liveRanges, i);
              if (sym->ruonly)
                return NULL;
            }
        }
    }

  sic = dic;

  if (ic->op == SEND)
    {
      /* look for the call to extend following
         far space search to include all parameters.
         see bug 3004918 */
      for (; ic; ic = ic->next)
        if (ic->op == CALL || ic->op == PCALL)
          break;
      if (!ic)                  /* not found */
        return NULL;
    }

  if (ic->op == PCALL && !IS_SMALL_PTR(aggrToPtr(operandType(IC_LEFT(ic)), FALSE)))
    return NULL;

  /* make sure the intervening instructions
     don't have anything in far space */
  for (dic = dic->next; dic && dic != ic && sic != ic; dic = dic->next)
    {
      /* if there is an intervening function call then no */
      if (dic->op == CALL || dic->op == PCALL)
        return NULL;
      /* if pointer set then make sure the pointer
         is one byte */
      if (POINTER_SET (dic) && !IS_SMALL_PTR (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)))
        return NULL;

      if (POINTER_GET (dic) && !IS_SMALL_PTR (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)))
        return NULL;

      /* if address of & the result is remat then okay */
      if (dic->op == ADDRESS_OF && OP_SYMBOL (IC_RESULT (dic))->remat)
        continue;

      /* if operand has size of three or more & this
         operation is a '*','/' or '%' then 'b' may
         cause a problem */
      if ((dic->op == '%' || dic->op == '/' || dic->op == '*') && getSize (operandType (op)) >= 3)
        return NULL;

      /* if left or right or result is in far space */
      if (isOperandInFarSpace (IC_LEFT (dic)) ||
          isOperandInFarSpace (IC_RIGHT (dic)) ||
          isOperandInFarSpace (IC_RESULT (dic)) ||
          IS_OP_RUONLY (IC_LEFT (dic)) || IS_OP_RUONLY (IC_RIGHT (dic)) || IS_OP_RUONLY (IC_RESULT (dic)))
        {
          return NULL;
        }
      /* if left or right or result is on stack */
      if (isOperandOnStack (IC_LEFT (dic)) || isOperandOnStack (IC_RIGHT (dic)) || isOperandOnStack (IC_RESULT (dic)))
        {
          return NULL;
        }
      if (usingCarry)
        {
          if (isOperandInBitSpace (IC_LEFT (dic)) ||
              isOperandInBitSpace (IC_RIGHT (dic)) ||
              isOperandInBitSpace (IC_RESULT (dic)))
            {
              return NULL;
            }
          if (dic->op != SEND || dic->op != IPUSH || dic->op != '=')
            {
              return NULL;
            }
        }
    }

  OP_SYMBOL (op)->ruonly = 1;
  return sic;
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
  if (IS_LITERAL (rtype) || (IS_BITVAR (ltype) && IN_BITSPACE (SPEC_OCLS (ltype))))
    return TRUE;
  else
    return FALSE;
}

/*-----------------------------------------------------------------*/
/* isCommutativeOp - tests whether this op cares what order its    */
/*                   operands are in                               */
/*-----------------------------------------------------------------*/
bool
isCommutativeOp (unsigned int op)
{
  if (op == '+' || op == '*' || op == EQ_OP || op == '^' || op == '|' || op == BITWISEAND)
    return TRUE;
  else
    return FALSE;
}

/*-----------------------------------------------------------------*/
/* operandUsesAcc - determines whether the code generated for this */
/*                  operand will have to use the accumulator       */
/*-----------------------------------------------------------------*/
bool
operandUsesAcc (operand * op, bool allowBitspace)
{
  if (!op)
    return FALSE;

  if (IS_SYMOP (op))
    {
      symbol *sym = OP_SYMBOL (op);
      memmap *symspace;

      if (sym->accuse)
        return TRUE;            /* duh! */

      if (IN_STACK (sym->etype) || sym->onStack || (SPIL_LOC (op) && SPIL_LOC (op)->onStack))
        return TRUE;            /* acc is used to calc stack offset */

      if (IS_ITEMP (op))
        {
          if (SPIL_LOC (op))
            {
              sym = SPIL_LOC (op);      /* if spilled, look at spill location */
            }
          else
            {
              return FALSE;     /* more checks? */
            }
        }

      symspace = SPEC_OCLS (sym->etype);

      if (sym->iaccess && symspace->paged)
        return TRUE;            /* must fetch paged indirect sym via accumulator */

      if (!allowBitspace && IN_BITSPACE (symspace))
        return TRUE;            /* fetching bit vars uses the accumulator */

      if (IN_FARSPACE (symspace) || IN_CODESPACE (symspace))
        return TRUE;            /* fetched via accumulator and dptr */
    }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* packRegsForAccUse - pack registers for acc use                  */
/*-----------------------------------------------------------------*/
static void
packRegsForAccUse (iCode * ic)
{
  iCode *uic;

  /* if this is an aggregate, e.g. a one byte char array */
  if (IS_AGGREGATE (operandType (IC_RESULT (ic))))
    return;

  /* if we are calling a reentrant function that has stack parameters */
  if (ic->op == CALL && IFFUNC_ISREENT (operandType (IC_LEFT (ic))) && FUNC_HASSTACKPARM (operandType (IC_LEFT (ic))))
    return;

  if (ic->op == PCALL &&
      IFFUNC_ISREENT (operandType (IC_LEFT (ic))->next) && FUNC_HASSTACKPARM (operandType (IC_LEFT (ic))->next))
    return;

  /* if + or - then it has to be one byte result */
  if ((ic->op == '+' || ic->op == '-') && getSize (operandType (IC_RESULT (ic))) > 1)
    return;

  /* if shift operation make sure right side is not a literal */
  if (ic->op == RIGHT_OP && (isOperandLiteral (IC_RIGHT (ic)) || getSize (operandType (IC_RESULT (ic))) > 1))
    return;

  if (ic->op == LEFT_OP && (isOperandLiteral (IC_RIGHT (ic)) || getSize (operandType (IC_RESULT (ic))) > 1))
    return;

  if (IS_BITWISE_OP (ic) && getSize (operandType (IC_RESULT (ic))) > 1)
    return;

  /* has only one definition */
  if (bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) > 1)
    return;

  /* has only one use */
  if (bitVectnBitsOn (OP_USES (IC_RESULT (ic))) > 1)
    return;

  /* and the usage immediately follows this iCode */
  if (!(uic = hTabItemWithKey (iCodehTab, bitVectFirstBit (OP_USES (IC_RESULT (ic))))))
    return;

  if (ic->next != uic)
    return;

  /* if it is a conditional branch then we definitely can */
  if (uic->op == IFX)
    goto accuse;

  if (uic->op == JUMPTABLE)
    return;

  if (POINTER_SET (uic) && getSize (aggrToPtr (operandType (IC_RESULT (uic)), FALSE)) > 1)
    return;

  /* if the usage is not an assignment
     or an arithmetic / bitwise / shift operation then not */
  if (uic->op != '=' && !IS_ARITHMETIC_OP (uic) && !IS_BITWISE_OP (uic) && uic->op != LEFT_OP && uic->op != RIGHT_OP)
    return;

  /* if shift operation make sure right side is not a literal */
  /* WIML: Why is this? */
  if (uic->op == RIGHT_OP && (isOperandLiteral (IC_RIGHT (uic)) || getSize (operandType (IC_RESULT (uic))) > 1))
    return;
  if (uic->op == LEFT_OP && (isOperandLiteral (IC_RIGHT (uic)) || getSize (operandType (IC_RESULT (uic))) > 1))
    return;

  /* make sure that the result of this icode is not on the
     stack, since acc is used to compute stack offset */
#if 0
  if (IS_TRUE_SYMOP (IC_RESULT (uic)) && OP_SYMBOL (IC_RESULT (uic))->onStack)
    return;
#else
  if (isOperandOnStack (IC_RESULT (uic)))
    return;
#endif

  /* if the usage has only one operand then we can */
  if (IC_LEFT (uic) == NULL || IC_RIGHT (uic) == NULL)
    goto accuse;

  /* if the other operand uses the accumulator then we cannot */
  if ((IC_LEFT (uic)->key == IC_RESULT (ic)->key &&
       operandUsesAcc (IC_RIGHT (uic), IS_BIT (operandType (IC_LEFT (uic))))) ||
      (IC_RIGHT (uic)->key == IC_RESULT (ic)->key && operandUsesAcc (IC_LEFT (uic), IS_BIT (operandType (IC_RIGHT (uic))))))
    return;

  /* make sure this is on the left side if not commutative */
  /* except for '-', which has been written to be able to
     handle reversed operands */
  if (!(isCommutativeOp (ic->op) || ic->op == '-') && IC_LEFT (uic)->key != IC_RESULT (ic)->key)
    return;

  /* Sign handling will overwrite a */
  if (uic->op == '*' && getSize (operandType (IC_RESULT (uic))) > 1 &&
    (!SPEC_USIGN (getSpec (operandType (IC_LEFT (uic)))) || !SPEC_USIGN (getSpec (operandType (IC_RIGHT (uic))))))
    return;
  if ((uic->op == '/' || uic->op == '%') &&
    (!SPEC_USIGN (getSpec (operandType (IC_LEFT (uic)))) || !SPEC_USIGN (getSpec (operandType (IC_RIGHT (uic))))))
    return;

#if 0
  // this is too dangerous and need further restrictions
  // see bug #447547

  /* if one of them is a literal then we can */
  if ((IC_LEFT (uic) && IS_OP_LITERAL (IC_LEFT (uic))) || (IC_RIGHT (uic) && IS_OP_LITERAL (IC_RIGHT (uic))))
    {
      OP_SYMBOL (IC_RESULT (ic))->accuse = 1;
      return;
    }
#endif

accuse:
  OP_SYMBOL (IC_RESULT (ic))->accuse = 1;
}

/*-----------------------------------------------------------------*/
/* packForPush - heuristics to reduce iCode for pushing            */
/*-----------------------------------------------------------------*/
static void
packForPush (iCode * ic, eBBlock ** ebpp, int blockno)
{
  iCode *dic, *lic;
  bitVect *dbv;
  struct eBBlock *ebp = ebpp[blockno];
  int disallowHiddenAssignment = 0;

  if (ic->op != IPUSH || !IS_ITEMP (IC_LEFT (ic)))
    return;

  /* must have only definition & one usage */
  if (bitVectnBitsOn (OP_DEFS (IC_LEFT (ic))) != 1 || bitVectnBitsOn (OP_USES (IC_LEFT (ic))) != 1)
    return;

  /* The changes in SDCCopt.c #7741 should correct the use info, making */
  /* this extra test redundant. */
  if (ic->parmPush)
    {// find Send or other Push for this func call
      for (lic = ic->next; lic && lic->op != CALL; lic = lic->next)
        {
          if ((lic->op == IPUSH || lic->op == SEND) && IS_ITEMP (IC_LEFT (lic)))
            {// and check parameter is not passed again
              symbol * parm = OP_SYMBOL (IC_LEFT (ic));
              symbol * other = OP_SYMBOL (IC_LEFT (lic));
              if (other == parm)
                return;
            }
        }
    }

  /* find the definition */
  if (!(dic = hTabItemWithKey (iCodehTab, bitVectFirstBit (OP_DEFS (IC_LEFT (ic))))))
    return;

  if (dic->op != '=' || POINTER_SET (dic))
    return;

  if (dic->seq < ebp->fSeq)     // Evelyn did this
    {
      int i;
      for (i = 0; i < blockno; i++)
        {
          if (dic->seq >= ebpp[i]->fSeq && dic->seq <= ebpp[i]->lSeq)
            {
              ebp = ebpp[i];
              break;
            }
        }
      wassert (i != blockno);   // no way to recover from here
    }

  if (IS_SYMOP (IC_RIGHT (dic)))
    {
      if (IC_RIGHT (dic)->isvolatile)
        return;

      if (OP_SYMBOL (IC_RIGHT (dic))->addrtaken || isOperandGlobal (IC_RIGHT (dic)))
        disallowHiddenAssignment = 1;

      /* make sure the right side does not have any definitions
         inbetween */
      dbv = OP_DEFS (IC_RIGHT (dic));
      for (lic = ic; lic && lic != dic; lic = lic->prev)
        {
          if (bitVectBitValue (dbv, lic->key))
            return;
          if (disallowHiddenAssignment && (lic->op == CALL || lic->op == PCALL || POINTER_SET (lic)))
            return;
        }
      /* make sure they have the same type */
      if (IS_SPEC (operandType (IC_LEFT (ic))))
        {
          sym_link *itype = operandType (IC_LEFT (ic));
          sym_link *ditype = operandType (IC_RIGHT (dic));

          if (SPEC_USIGN (itype) != SPEC_USIGN (ditype) || SPEC_LONG (itype) != SPEC_LONG (ditype))
            return;
        }
      /* extend the live range of replaced operand if needed */
      if (OP_SYMBOL (IC_RIGHT (dic))->liveTo < ic->seq)
        {
          OP_SYMBOL (IC_RIGHT (dic))->liveTo = ic->seq;
        }
      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
    }
  if (IS_ITEMP (IC_RIGHT (dic)))
    OP_USES (IC_RIGHT (dic)) = bitVectSetBit (OP_USES (IC_RIGHT (dic)), ic->key);

  /* now we know that it has one & only one def & use
     and the that the definition is an assignment */
  ReplaceOpWithCheaperOp (&IC_LEFT (ic), IC_RIGHT (dic));
  remiCodeFromeBBlock (ebp, dic);
  hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
}

/*-----------------------------------------------------------------*/
/* packRegisters - does some transformations to reduce register    */
/*                   pressure                                      */
/*-----------------------------------------------------------------*/
static void
packRegisters (eBBlock ** ebpp, int blockno)
{
  iCode *ic;
  int change = 0;
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
      /* Fix for bug #979599:   */
      /* P0 &= ~1;              */

      /* Look for two subsequent iCodes with */
      /*   iTemp := _c;         */
      /*   _c = iTemp & op;     */
      /* and replace them by    */
      /*   iTemp := _c;         */
      /*   _c = _c & op;        */
      if ((ic->op == BITWISEAND || ic->op == '|' || ic->op == '^') &&
          ic->prev &&
          ic->prev->op == '=' &&
          (IS_ITEMP (IC_LEFT (ic)) && isOperandEqual (IC_LEFT (ic), IC_RESULT (ic->prev)) && isOperandEqual (IC_RESULT (ic), IC_RIGHT (ic->prev)) ||
           IS_ITEMP (IC_RIGHT (ic)) && isOperandEqual (IC_RIGHT (ic), IC_RESULT (ic->prev)) && isOperandEqual (IC_RESULT (ic), IC_RIGHT (ic->prev))))
        {
          bool left = IS_ITEMP (IC_LEFT (ic)) && isOperandEqual (IC_LEFT (ic), IC_RESULT (ic->prev)) && isOperandEqual (IC_RESULT (ic), IC_RIGHT (ic->prev));

          iCode *ic_prev = ic->prev;
          symbol *prev_result_sym = OP_SYMBOL (IC_RESULT (ic_prev));

          ReplaceOpWithCheaperOp (left ? &IC_LEFT (ic) : &IC_RIGHT (ic), IC_RESULT (ic));
          if (IC_RESULT (ic_prev) != (left ? IC_RIGHT (ic) : IC_LEFT (ic)))
            {
              bitVectUnSetBit (OP_USES (IC_RESULT (ic_prev)), ic->key);
              if (              /*IS_ITEMP (IC_RESULT (ic_prev)) && */
                   prev_result_sym->liveTo == ic->seq)
                {
                  prev_result_sym->liveTo = ic_prev->seq;
                }
            }
          bitVectSetBit (OP_USES (IC_RESULT (ic)), ic->key);

          bitVectSetBit (ic->rlive, IC_RESULT (ic)->key);

          if (bitVectIsZero (OP_USES (IC_RESULT (ic_prev))))
            {
              bitVectUnSetBit (ic->rlive, IC_RESULT (ic)->key);
              bitVectUnSetBit (OP_DEFS (IC_RESULT (ic_prev)), ic_prev->key);
              remiCodeFromeBBlock (ebp, ic_prev);
              hTabDeleteItem (&iCodehTab, ic_prev->key, ic_prev, DELETE_ITEM, NULL);
            }
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

      /* if straight assignment then carry remat flag if
         this is the only definition */
      if (ic->op == '=' &&
          !POINTER_SET (ic) &&
          IS_SYMOP (IC_RIGHT (ic)) &&
          OP_SYMBOL (IC_RIGHT (ic))->remat &&
          !IS_CAST_ICODE (OP_SYMBOL (IC_RIGHT (ic))->rematiCode) &&
          !isOperandGlobal (IC_RESULT (ic)) &&  /* due to bug 1618050 */
          bitVectnBitsOn (OP_SYMBOL (IC_RESULT (ic))->defs) <= 1 &&
          !OP_SYMBOL (IC_RESULT (ic))->addrtaken)
        {
          OP_SYMBOL (IC_RESULT (ic))->remat = OP_SYMBOL (IC_RIGHT (ic))->remat;
          OP_SYMBOL (IC_RESULT (ic))->rematiCode = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
        }

      /* if cast to a generic pointer & the pointer being
         cast is remat, then we can remat this cast as well */
      if (ic->op == CAST &&
          IS_SYMOP (IC_RIGHT (ic)) &&
          OP_SYMBOL (IC_RIGHT (ic))->remat &&
          bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 &&
          !OP_SYMBOL (IC_RESULT (ic))->addrtaken)
        {
          sym_link *to_type = operandType (IC_LEFT (ic));
          sym_link *from_type = operandType (IC_RIGHT (ic));
          if (IS_PTR (to_type) && IS_PTR (from_type))
            {
              OP_SYMBOL (IC_RESULT (ic))->remat = 1;
              OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
              OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
            }
        }

      /* if this is a +/- operation with a rematerializable
         then mark this as rematerializable as well */
      if ((ic->op == '+' || ic->op == '-') &&
          IS_SYMOP (IC_LEFT (ic)) &&
          IS_ITEMP (IC_RESULT (ic)) &&
          IS_OP_LITERAL (IC_RIGHT (ic)) &&
          OP_SYMBOL (IC_LEFT (ic))->remat &&
          (!IS_SYMOP (IC_RIGHT (ic)) || !IS_CAST_ICODE (OP_SYMBOL (IC_RIGHT (ic))->rematiCode)) &&
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

      if (!SKIP_IC2 (ic))
        {
          /* if we are using a symbol on the stack
             then we should say mcs51_ptrRegReq */
          if (options.useXstack && ic->parmPush && (ic->op == IPUSH || ic->op == IPOP))
            mcs51_ptrRegReq++;
          if (ic->op == IFX && IS_SYMOP (IC_COND (ic)))
            mcs51_ptrRegReq += ((OP_SYMBOL (IC_COND (ic))->onStack ||
                                 OP_SYMBOL (IC_COND (ic))->iaccess ||
                                 SPEC_OCLS (OP_SYMBOL (IC_COND (ic))->etype) == idata) ? 1 : 0);
          else if (ic->op == JUMPTABLE && IS_SYMOP (IC_JTCOND (ic)))
            mcs51_ptrRegReq += ((OP_SYMBOL (IC_JTCOND (ic))->onStack ||
                                 OP_SYMBOL (IC_JTCOND (ic))->iaccess ||
                                 SPEC_OCLS (OP_SYMBOL (IC_JTCOND (ic))->etype) == idata) ? 1 : 0);
          else
            {
              if (IS_SYMOP (IC_LEFT (ic)))
                mcs51_ptrRegReq += ((OP_SYMBOL (IC_LEFT (ic))->onStack ||
                                     OP_SYMBOL (IC_LEFT (ic))->iaccess ||
                                     SPEC_OCLS (OP_SYMBOL (IC_LEFT (ic))->etype) == idata) ? 1 : 0);
              if (IS_SYMOP (IC_RIGHT (ic)))
                mcs51_ptrRegReq += ((OP_SYMBOL (IC_RIGHT (ic))->onStack ||
                                     OP_SYMBOL (IC_RIGHT (ic))->iaccess ||
                                     SPEC_OCLS (OP_SYMBOL (IC_RIGHT (ic))->etype) == idata) ? 1 : 0);
              if (IS_SYMOP (IC_RESULT (ic)))
                mcs51_ptrRegReq += ((OP_SYMBOL (IC_RESULT (ic))->onStack ||
                                     OP_SYMBOL (IC_RESULT (ic))->iaccess ||
                                     SPEC_OCLS (OP_SYMBOL (IC_RESULT (ic))->etype) == idata) ? 1 : 0);
              if (POINTER_GET (ic) && IS_SYMOP (IC_LEFT (ic))
                  && getSize (OP_SYMBOL (IC_LEFT (ic))->type) <= (unsigned int) NEARPTRSIZE)
                mcs51_ptrRegReq++;
              if (POINTER_SET (ic) && IS_SYMOP (IC_RESULT (ic))
                  && getSize (OP_SYMBOL (IC_RESULT (ic))->type) <= (unsigned int) NEARPTRSIZE)
                mcs51_ptrRegReq++;
            }
        }

      /* if the condition of an if instruction
         is defined in the previous instruction and
         this is the only usage then
         mark the itemp as a conditional */
      if ((IS_CONDITIONAL (ic) ||
           (IS_BITWISE_OP (ic) && isBitwiseOptimizable (ic))) &&
          ic->next && ic->next->op == IFX &&
          bitVectnBitsOn (OP_USES (IC_RESULT (ic))) == 1 &&
          isOperandEqual (IC_RESULT (ic), IC_COND (ic->next)) && OP_SYMBOL (IC_RESULT (ic))->liveTo <= ic->next->seq)
        {
          OP_SYMBOL (IC_RESULT (ic))->regType = REG_CND;
          continue;
        }

      /* if the condition of an if instruction
         is defined in the previous GET_POINTER instruction and
         this is the only usage then
         mark the itemp as accumulator use */
      if ((POINTER_GET (ic) && getSize (operandType (IC_RESULT (ic))) <= 1) &&
          ic->next && ic->next->op == IFX &&
          bitVectnBitsOn (OP_USES (IC_RESULT (ic))) == 1 &&
          isOperandEqual (IC_RESULT (ic), IC_COND (ic->next)) && OP_SYMBOL (IC_RESULT (ic))->liveTo <= ic->next->seq)
        {
          OP_SYMBOL (IC_RESULT (ic))->accuse = 1;
          continue;
        }

      /* reduce for support function calls */
      if (ic->supportRtn || ic->op == '+' || ic->op == '-')
        packRegsForSupport (ic, ebp);

      /* some cases the redundant moves can
         can be eliminated for return statements */
      if ((ic->op == RETURN || (ic->op == SEND && ic->argreg == 1)) &&
          !isOperandInFarSpace (IC_LEFT (ic)) && (options.model == MODEL_SMALL || options.model == MODEL_MEDIUM))
        {
          packRegsForOneuse (ic, IC_LEFT (ic), ebp);
        }

      /* if pointer set & left has a size more than
         one and right is not in far space */
      if (POINTER_SET (ic) &&
          IS_SYMOP (IC_RESULT (ic)) &&
          !isOperandInFarSpace (IC_RIGHT (ic)) &&
          !OP_SYMBOL (IC_RESULT (ic))->remat &&
          !IS_OP_RUONLY (IC_RIGHT (ic)) && getSize (aggrToPtr (operandType (IC_RESULT (ic)), FALSE)) > 1)
        {
          packRegsForOneuse (ic, IC_RESULT (ic), ebp);
        }

      /* if pointer get */
      if (POINTER_GET (ic) &&
          IS_SYMOP (IC_LEFT (ic)) &&
          !isOperandInFarSpace (IC_RESULT (ic)) &&
          !OP_SYMBOL (IC_LEFT (ic))->remat &&
          !IS_OP_RUONLY (IC_RESULT (ic)) &&
          getSize (aggrToPtr (operandType (IC_LEFT (ic)), FALSE)) > 1)
        {
          packRegsForOneuse (ic, IC_LEFT (ic), ebp);
        }

      /* if this is a cast for integral promotion then
         check if it's the only use of the definition of the
         operand being casted/ if yes then replace
         the result of that arithmetic operation with
         this result and get rid of the cast */
      if (ic->op == CAST)
        {
          sym_link *fromType = operandType (IC_RIGHT (ic));
          sym_link *toType = operandType (IC_LEFT (ic));

          if (IS_INTEGRAL (fromType) && IS_INTEGRAL (toType) &&
              getSize (fromType) != getSize (toType) && SPEC_USIGN (fromType) == SPEC_USIGN (toType))
            {
              iCode *dic = packRegsForOneuse (ic, IC_RIGHT (ic), ebp);
              if (dic)
                {
                  if (IS_ARITHMETIC_OP (dic))
                    {
                      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
                      ReplaceOpWithCheaperOp (&IC_RESULT (dic), IC_RESULT (ic));
                      remiCodeFromeBBlock (ebp, ic);
                      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (ic))->defs, ic->key);
                      hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
                      OP_DEFS (IC_RESULT (dic)) = bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
                      ic = ic->prev;
                    }
                  else
                    {
                      OP_SYMBOL (IC_RIGHT (ic))->ruonly = 0;
                    }
                }
            }
          else
            {
              /* if the type from and type to are the same
                 then if this is the only use then pack it */
              if (compareType (operandType (IC_RIGHT (ic)), operandType (IC_LEFT (ic))) == 1)
                {
                  iCode *dic = packRegsForOneuse (ic, IC_RIGHT (ic), ebp);
                  if (dic)
                    {
                      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
                      ReplaceOpWithCheaperOp (&IC_RESULT (dic), IC_RESULT (ic));
                      remiCodeFromeBBlock (ebp, ic);
                      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (ic))->defs, ic->key);
                      hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
                      OP_DEFS (IC_RESULT (dic)) = bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
                      ic = ic->prev;
                    }
                }
            }
        }

      /* pack for PUSH
         iTempNN := (some variable in farspace) V1
         push iTempNN ;
         -------------
         push V1
       */
      if (ic->op == IPUSH)
        {
          packForPush (ic, ebpp, blockno);
        }

      /* pack registers for accumulator use, when the
         result of an arithmetic or bit wise operation
         has only one use, that use is immediately following
         the definition and the using iCode has only one
         operand or has two operands but one is literal &
         the result of that operation is not on stack then
         we can leave the result of this operation in acc:b
         combination */
      if ((IS_ARITHMETIC_OP (ic)
           || IS_CONDITIONAL (ic)
           || IS_BITWISE_OP (ic)
           || ic->op == LEFT_OP || ic->op == RIGHT_OP || ic->op == CALL
           || ic->op == '=' && !POINTER_SET(ic) && getSize (operandType (IC_RESULT (ic))) < 2
           || (ic->op == ADDRESS_OF && isOperandOnStack (IC_LEFT (ic)))) &&
          IS_ITEMP (IC_RESULT (ic)) && getSize (operandType (IC_RESULT (ic))) <= 2)
        {
          packRegsForAccUse (ic);
        }
    }
}

/*------------------------------------------------------------------------*/
/* positionRegsReverse - positioning registers from end to begin to avoid */
/* conflict among result, left and right operands in some extrem cases    */
/*------------------------------------------------------------------------*/
static void
positionRegsReverse (eBBlock ** ebbs, int count)
{
  int i;
  iCode *ic;

  for (i = count - 1; i >= 0; i--)
    for (ic = ebbs[i]->ech; ic; ic = ic->prev)
      {
        if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)) && OP_SYMBOL (IC_LEFT (ic))->nRegs &&
            IC_RESULT (ic) && IS_SYMOP (IC_RESULT (ic)) && OP_SYMBOL (IC_RESULT (ic))->nRegs)
          {
            positionRegs (OP_SYMBOL (IC_RESULT (ic)), OP_SYMBOL (IC_LEFT (ic)), 1);    
          }
        if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)) && OP_SYMBOL (IC_RIGHT (ic))->nRegs &&
            IC_RESULT (ic) && IS_SYMOP (IC_RESULT (ic)) && OP_SYMBOL (IC_RESULT (ic))->nRegs)
          {
            positionRegs (OP_SYMBOL (IC_RESULT (ic)), OP_SYMBOL (IC_RIGHT (ic)), 1);    
          }
      }
}

/*-----------------------------------------------------------------*/
/* assignRegisters - assigns registers to each live range as need  */
/*-----------------------------------------------------------------*/
void
mcs51_assignRegisters (ebbIndex * ebbi)
{
  eBBlock **ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  iCode *ic;
  int i;

  setToNull ((void *) &_G.funcrUsed);
  setToNull ((void *) &_G.regAssigned);
  setToNull ((void *) &_G.totRegAssigned);
  mcs51_ptrRegReq = _G.stackExtend = _G.dataExtend = 0;
  if ((currFunc && IFFUNC_ISREENT (currFunc->type)) || options.stackAuto)
    {
      mcs51_nRegs = 16;
    }
  else
    {
      mcs51_nRegs = 8;
    }
  _G.allBitregs = findAllBitregs ();
  _G.allBankregs = findAllBankregs ();

  /* change assignments this will remove some
     live ranges reducing some register pressure */

  for (i = 0; i < count; i++)
    packRegisters (ebbs, i);

  /* liveranges probably changed by register packing
     so we compute them again */
  recomputeLiveRanges (ebbs, count, FALSE);

  if (options.dump_i_code)
    dumpEbbsToFileExt (DUMP_PACK, ebbi);

  /* first determine for each live range the number of
     registers & the type of registers required for each */
  regTypeNum (*ebbs);

  /* and serially allocate registers */
  serialRegAssign (ebbs, count);

  freeAllRegs ();
  //setToNull ((void *) &_G.regAssigned);
  //setToNull ((void *) &_G.totRegAssigned);
  fillGaps ();

  /* do positionRegs() for all ICs from end to begin */
  positionRegsReverse (ebbs, count);

  /* if stack was extended then tell the user */
  if (_G.stackExtend)
    {
/*      werror(W_TOOMANY_SPILS,"stack", */
/*             _G.stackExtend,currFunc->name,""); */
      _G.stackExtend = 0;
    }

  if (_G.dataExtend)
    {
/*      werror(W_TOOMANY_SPILS,"data space", */
/*             _G.dataExtend,currFunc->name,""); */
      _G.dataExtend = 0;
    }

  /* after that create the register mask
     for each of the instruction */
  createRegMask (ebbs, count);

  /* redo that offsets for stacked automatic variables */
  if (currFunc)
    {
      redoStackOffsets ();
    }

  /* make sure r0 & r1 are flagged as used if they might be used */
  /* as pointers */
  if (currFunc && mcs51_ptrRegReq)
    {
      currFunc->regsUsed = bitVectSetBit (currFunc->regsUsed, R0_IDX);
      currFunc->regsUsed = bitVectSetBit (currFunc->regsUsed, R1_IDX);
    }

  if (options.dump_i_code)
    {
      dumpEbbsToFileExt (DUMP_RASSGN, ebbi);
      dumpLiveRanges (DUMP_LRANGE, liveRanges);
    }

  /* do the overlaysegment stuff SDCCmem.c */
  doOverlays (ebbs, count);

  /* now get back the chain */
  ic = iCodeLabelOptimize (iCodeFromeBBlock (ebbs, count));

  gen51Code (ic);

  /* free up any _G.stackSpil locations allocated */
  applyToSet (_G.stackSpil, deallocStackSpil);
  _G.slocNum = 0;
  setToNull ((void *) &_G.stackSpil);
  setToNull ((void *) &_G.spiltSet);
  /* mark all registers as free */
  freeAllRegs ();

  return;
}
