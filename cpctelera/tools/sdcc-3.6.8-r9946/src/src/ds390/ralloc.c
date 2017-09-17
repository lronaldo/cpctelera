/*------------------------------------------------------------------------

  SDCCralloc.c - source file for register allocation. (DS80C390) specific

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
}
_G;

/* Shared with gen.c */
int ds390_ptrRegReq;            /* one byte pointer register required */

/* 8051 registers */
reg_info regs390[] = {

  {REG_GPR, R2_IDX, REG_GPR, "r2", "ar2", "0", 2, 1, 1},
  {REG_GPR, R3_IDX, REG_GPR, "r3", "ar3", "0", 3, 1, 1},
  {REG_GPR, R4_IDX, REG_GPR, "r4", "ar4", "0", 4, 1, 1},
  {REG_GPR, R5_IDX, REG_GPR, "r5", "ar5", "0", 5, 1, 1},
  {REG_GPR, R6_IDX, REG_GPR, "r6", "ar6", "0", 6, 1, 1},
  {REG_GPR, R7_IDX, REG_GPR, "r7", "ar7", "0", 7, 1, 1},
  {REG_PTR, R0_IDX, REG_PTR, "r0", "ar0", "0", 0, 1, 1},
  {REG_PTR, R1_IDX, REG_PTR, "r1", "ar1", "0", 1, 1, 1},
  {REG_GPR, DPL_IDX, REG_GPR, "dpl", "dpl", "dpl", 0, 0, 0},
  {REG_GPR, DPH_IDX, REG_GPR, "dph", "dph", "dph", 0, 0, 0},
  {REG_GPR, DPX_IDX, REG_GPR, "dpx", "dpx", "dpx", 0, 0, 0},
  {REG_GPR, B_IDX, REG_GPR, "b", "b", "b", 0, 0, 0},
  {REG_BIT, B0_IDX, REG_BIT, "b0", "b0", "bits", 0, 1, 0},
  {REG_BIT, B1_IDX, REG_BIT, "b1", "b1", "bits", 1, 1, 0},
  {REG_BIT, B2_IDX, REG_BIT, "b2", "b2", "bits", 2, 1, 0},
  {REG_BIT, B3_IDX, REG_BIT, "b3", "b3", "bits", 3, 1, 0},
  {REG_BIT, B4_IDX, REG_BIT, "b4", "b4", "bits", 4, 1, 0},
  {REG_BIT, B5_IDX, REG_BIT, "b5", "b5", "bits", 5, 1, 0},
  {REG_BIT, B6_IDX, REG_BIT, "b6", "b6", "bits", 6, 1, 0},
  {REG_BIT, B7_IDX, REG_BIT, "b7", "b7", "bits", 7, 1, 0},
  {REG_GPR, X8_IDX, REG_GPR, "x8", "x8", "xreg", 0, 0, 0},
  {REG_GPR, X9_IDX, REG_GPR, "x9", "x9", "xreg", 1, 0, 0},
  {REG_GPR, X10_IDX, REG_GPR, "x10", "x10", "xreg", 2, 0, 0},
  {REG_GPR, X11_IDX, REG_GPR, "x11", "x11", "xreg", 3, 0, 0},
  {REG_GPR, X12_IDX, REG_GPR, "x12", "x12", "xreg", 4, 0, 0},
  {REG_CND, CND_IDX, REG_GPR, "C", "psw", "xreg", 0, 0, 0},
  {0, DPL1_IDX, 0, "dpl1", "dpl1", "dpl1", 0, 0, 0},
  {0, DPH1_IDX, 0, "dph1", "dph1", "dph1", 0, 0, 0},
  {0, DPX1_IDX, 0, "dpx1", "dpx1", "dpx1", 0, 0, 0},
  {0, DPS_IDX, 0, "dps", "dps", "dps", 0, 0, 0},
  {0, A_IDX, 0, "a", "acc", "acc", 0, 0, 0},
  {0, AP_IDX, 0, "ap", "ap", "ap", 0, 0, 0},
};

int ds390_nRegs = 13;
int ds390_nBitRegs = 0;

static void spillThis (symbol *);
static void freeAllRegs ();
static iCode *packRegsDPTRuse (operand *);
static int packRegsDPTRnuse (operand *, unsigned);

/*-----------------------------------------------------------------*/
/* allocReg - allocates register of given type                     */
/*-----------------------------------------------------------------*/
static reg_info *
allocReg (short type)
{
  int i;

  for (i = 0; i < ds390_nRegs; i++)
    {

      /* if type is given as 0 then any
         free register will do */
      if (!type && regs390[i].isFree)
        {
          regs390[i].isFree = 0;
          if (currFunc)
            currFunc->regsUsed = bitVectSetBit (currFunc->regsUsed, i);
          return &regs390[i];
        }
      /* otherwise look for specific type of register */
      if (regs390[i].isFree && regs390[i].type == type)
        {
          regs390[i].isFree = 0;
          if (currFunc)
            currFunc->regsUsed = bitVectSetBit (currFunc->regsUsed, i);
          return &regs390[i];
        }
    }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* ds390_regWithIdx - returns pointer to register with index number*/
/*-----------------------------------------------------------------*/
reg_info *
ds390_regWithIdx (int idx)
{
  int i;

  for (i = 0; i < sizeof (regs390) / sizeof (reg_info); i++)
    if (regs390[i].rIdx == idx)
      return &regs390[i];

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
/* useReg - marks a register  as used                              */
/*-----------------------------------------------------------------*/
static void
useReg (reg_info *reg)
{
  reg->isFree = 0;
}

/*-----------------------------------------------------------------*/
/* nFreeRegs - returns number of free registers                    */
/*-----------------------------------------------------------------*/
static int
nFreeRegs (int type)
{
  int i;
  int nfr = 0;

  for (i = 0; i < ds390_nRegs; i++)
    if (regs390[i].isFree && regs390[i].type == type)
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
/* isOperandInReg - returns true if operand is currently in regs   */
/*-----------------------------------------------------------------*/
static int
isOperandInReg (operand * op)
{
  if (!IS_SYMOP (op))
    return 0;
  if (OP_SYMBOL (op)->ruonly)
    return 1;
  if (OP_SYMBOL (op)->accuse)
    return 1;
  if (OP_SYMBOL (op)->dptr)
    return 1;
  return bitVectBitValue (_G.totRegAssigned, OP_SYMBOL (op)->key);
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
/* hasSpilLocnoUptr - will return 1 if the symbol has spil location */
/*                    but is not used as a pointer                 */
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

  r0 = ds390_regWithIdx (R0_IDX);
  r1 = ds390_regWithIdx (R1_IDX);

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
  int useXstack, model, noOverlay;
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
      if (options.model == MODEL_SMALL)
        {
          SPEC_SCLS (sloc->etype) = S_DATA;
        }
      else
        {
          SPEC_SCLS (sloc->etype) = S_XDATA;
        }
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
     and turn off overlaying
   */

  useXstack = options.useXstack;
  model = options.model;
  noOverlay = options.noOverlay;
  options.noOverlay = 1;

  /* options.model = options.useXstack = 0; */

  allocLocal (sloc);

  options.useXstack = useXstack;
  options.model = model;
  options.noOverlay = noOverlay;
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

  /* mark it has spilt & put it in the spilt set */
  sym->isspilt = sym->spillA = 1;
  _G.spiltSet = bitVectSetBit (_G.spiltSet, sym->key);

  bitVectUnSetBit (_G.regAssigned, sym->key);
  bitVectUnSetBit (_G.totRegAssigned, sym->key);

  for (i = 0; i < sym->nRegs; i++)

    if (sym->regs[i])
      {
        freeReg (sym->regs[i]);
        sym->regs[i] = NULL;
      }

  /* if spilt on stack then free up r0 & r1
     if they could have been assigned to some
     LIVE ranges */
  if (!ds390_ptrRegReq && isSpiltOnStack (sym) && !options.stack10bit)
    {
      ds390_ptrRegReq++;
      spillLRWithPtrReg (sym);
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

      /* check if there are any live ranges that not
         used in the remainder of the block */
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
     location on the stack , for which one? the least
     used ofcourse */
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
  if (!ds390_ptrRegReq && isSpiltOnStack (ssym) && !options.stack10bit)
    {
      ds390_ptrRegReq++;
      spillLRWithPtrReg (ssym);
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
     in really selectSpil will abort  */
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

  if (!ds390_ptrRegReq)
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
     in really selectSpil will abort  */
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

  spillThis (sym);
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

  if (!ds390_ptrRegReq)
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
symHasReg (symbol *sym, reg_info *reg)
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

  for (reg = 0; reg < ds390_nRegs; reg++)
    {
      if (regs390[reg].isFree)
        {
          ic->riu &= ~(1 << regs390[reg].offset);
        }
      else
        {
          ic->riu |= (1 << regs390[reg].offset);
          BitBankUsed |= (reg >= B0_IDX);
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
          if (IC_RESULT (ic) && !(SKIP_IC2 (ic) ||      /* not a special icode */
                                  ic->op == JUMPTABLE || ic->op == IFX || ic->op == IPUSH || ic->op == IPOP || ic->op == RETURN || POINTER_SET (ic)) && (result = OP_SYMBOL (IC_RESULT (ic))) &&        /* has a result */
              result->liveTo > ic->seq &&       /* and will live beyond this */
              result->liveTo <= ebp->lSeq &&    /* does not go beyond this block */
              result->liveFrom == ic->seq &&    /* does not start before here */
              result->regType == sym->regType &&        /* same register types */
              result->nRegs &&  /* which needs registers */
              !result->isspilt &&       /* and does not already have them */
              !result->remat && !bitVectBitValue (_G.regAssigned, result->key) &&
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
         if pointer type not avlb then
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
      if (ds390_ptrRegReq)
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
/* positionRegs - the allocator can allocate same registers to res- */
/* ult and operand, if this happens make sure they are in the same */
/* position as the operand otherwise chaos results                 */
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

/*-----------------------------------------------------------------*/
/* unusedLRS - returns a bitVector of liveranges not used in 'ebp' */
/*-----------------------------------------------------------------*/
bitVect *
unusedLRs (eBBlock * ebp)
{
  bitVect *ret = NULL;
  symbol *sym;
  int key;

  if (!ebp)
    return NULL;
  for (sym = hTabFirstItem (liveRanges, &key); sym; sym = hTabNextItem (liveRanges, &key))
    {

      if (notUsedInBlock (sym, ebp, NULL))
        {
          ret = bitVectSetBit (ret, sym->key);
        }
    }

  return ret;
}

/*-----------------------------------------------------------------*/
/* deassignUnsedLRs - if this baisc block ends in a return then    */
/*                    deassign symbols not used in this block      */
/*-----------------------------------------------------------------*/
bitVect *
deassignUnsedLRs (eBBlock * ebp)
{
  bitVect *unused = NULL;
  int i;

  switch (returnAtEnd (ebp))
    {
    case 2:                    /* successor block ends in a return */
      unused = unusedLRs ((eBBlock *) setFirstItem (ebp->succList));
      /* fall thru */
    case 1:                    /* this block ends in a return */
      unused = bitVectIntersect (unused, unusedLRs (ebp));
      break;
    }

  if (unused)
    {
      for (i = 0; i < unused->size; i++)
        {

          /* if unused  */
          if (bitVectBitValue (unused, i))
            {

              /* if assigned to registers */
              if (bitVectBitValue (_G.regAssigned, i))
                {
                  symbol *sym;
                  int j;

                  sym = hTabItemWithKey (liveRanges, i);
                  /* remove it from regassigned & mark the
                     register free */
                  bitVectUnSetBit (_G.regAssigned, i);
                  for (j = 0; j < sym->nRegs; j++)
                    freeReg (sym->regs[j]);
                }
              else
                {
                  /* not assigned to registers : remove from set */
                  bitVectUnSetBit (unused, i);
                }
            }
        }
    }
  return unused;
}

/*-----------------------------------------------------------------*/
/* reassignUnusedLRs - put registers to unused Live ranges         */
/*-----------------------------------------------------------------*/
void
reassignUnusedLRs (bitVect * unused)
{
  int i;
  if (!unused)
    return;

  for (i = 0; i < unused->size; i++)
    {
      /* if unused : means it was assigned to registers before */
      if (bitVectBitValue (unused, i))
        {
          symbol *sym;
          int j;

          /* put it back into reg set */
          bitVectSetBit (_G.regAssigned, i);

          sym = hTabItemWithKey (liveRanges, i);
          /* make registers busy */
          for (j = 0; j < sym->nRegs; j++)
            sym->regs[j]->isFree = 0;
        }
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
    {                           /* ebbs */

      iCode *ic;
      bitVect *unusedLRs = NULL;

      if (ebbs[i]->noPath && (ebbs[i]->entryLabel != entryLabel && ebbs[i]->entryLabel != returnLabel))
        continue;

      unusedLRs = deassignUnsedLRs (ebbs[i]);

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
            OP_SYMBOL (IC_RESULT (ic))->allocreq++;

          /* take away registers from live
             ranges that end at this instruction */
          deassignLRs (ic, ebbs[i]);

          /* some don't need registers */
          if (SKIP_IC2 (ic) ||
              ic->op == JUMPTABLE || ic->op == IFX || ic->op == IPUSH || ic->op == IPOP || (IC_RESULT (ic) && POINTER_SET (ic)))
            continue;

          /* now we need to allocate registers
             only for the result */
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
                continue;

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

              /* if we need ptr regs for the right side
                 then mark it */
              if (POINTER_GET (ic) && IS_SYMOP (IC_LEFT (ic)) && getSize (OP_SYMBOL (IC_LEFT (ic))->type) <= (unsigned) PTRSIZE)
                {
                  ds390_ptrRegReq++;
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
                  else if (sym->regType == REG_BIT)
                    sym->regs[j] = getRegBit (sym);
                  else
                    sym->regs[j] = getRegGpr (ic, ebbs[i], sym);

                  /* if the allocation failed which means
                     this was spilt then break */
                  if (!sym->regs[j])
                    break;
                }

              /* if it shares registers with operands make sure
                 that they are in the same position */
              if (!POINTER_SET (ic) && !POINTER_GET (ic))
                {
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
                  ds390_ptrRegReq--;
                  ptrRegSet = 0;
                }

            }
        }
      reassignUnusedLRs (unusedLRs);
    }

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

/*-----------------------------------------------------------------*/
/* fillGaps - Try to fill in the Gaps left by Pass1                */
/*-----------------------------------------------------------------*/
static void
fillGaps ()
{
  symbol *sym = NULL;
  int key = 0;
  int loop = 0, change;
  int pass;

  if (getenv ("DISABLE_FILL_GAPS"))
    return;

  /* First try to do DPTRuse once more since now we know what got into
     registers */

  while (loop++ < 10)
    {
      change = 0;

      for (sym = hTabFirstItem (liveRanges, &key); sym; sym = hTabNextItem (liveRanges, &key))
        {
          int size = getSize (sym->type);

          if (sym->liveFrom == sym->liveTo)
            continue;

          if (sym->uptr && sym->dptr == 0 && !sym->ruonly && size < 4 && size > 1)
            {

              if (packRegsDPTRuse (operandFromSymbol (sym)))
                {

                  /* if this was assigned to registers then */
                  if (bitVectBitValue (_G.totRegAssigned, sym->key))
                    {
                      /* take it out of the register assigned set */
                      bitVectUnSetBit (_G.totRegAssigned, sym->key);
                    }
                  else if (sym->usl.spillLoc)
                    {
                      sym->usl.spillLoc->allocreq--;
                      sym->usl.spillLoc = NULL;
                    }

                  sym->nRegs = 0;
                  sym->isspilt = sym->spillA = 0;
                  continue;
                }

              /* try assigning other dptrs */
              if (sym->dptr == 0 && packRegsDPTRnuse (operandFromSymbol (sym), 1) && !getenv ("DPTRnDISABLE"))
                {
                  /* if this was ssigned to registers then */
                  if (bitVectBitValue (_G.totRegAssigned, sym->key))
                    {
                      /* take it out of the register assigned set */
                      bitVectUnSetBit (_G.totRegAssigned, sym->key);
                    }
                  else if (sym->usl.spillLoc)
                    {
                      sym->usl.spillLoc->allocreq--;
                      sym->usl.spillLoc = NULL;
                    }
                  sym->nRegs = 0;
                  sym->isspilt = sym->spillA = 0;
                }
            }
        }

      /* look for liveranges that were spilt by the allocator */
      for (sym = hTabFirstItem (liveRanges, &key); sym; sym = hTabNextItem (liveRanges, &key))
        {

          int i;
          int pdone = 0;

          if (!sym->spillA || !sym->clashes || sym->remat)
            continue;
          if (!sym->uses || !sym->defs)
            continue;
          /* find the liveRanges this one clashes with, that are
             still assigned to registers & mark the registers as used */
          for (i = 0; i < sym->clashes->size; i++)
            {
              int k;
              symbol *clr;

              if (bitVectBitValue (sym->clashes, i) == 0 ||     /* those that clash with this */
                  bitVectBitValue (_G.totRegAssigned, i) == 0)  /* and are still assigned to registers */
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

          /* THERE IS HOPE !!!! */
          for (i = 0; i < sym->nRegs; i++)
            {
              if (sym->regType == REG_PTR)
                sym->regs[i] = getRegPtrNoSpil ();
              else if (sym->regType == REG_BIT)
                sym->regs[i] = getRegBitNoSpil ();
              else
                sym->regs[i] = getRegGprNoSpil ();
            }

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
              for (i = 0; i < sym->defs->size; i++)
                {
                  if (bitVectBitValue (sym->defs, i))
                    {
                      iCode *ic;
                      if (!(ic = hTabItemWithKey (iCodehTab, i)))
                        continue;
                      if (SKIP_IC (ic))
                        continue;
                      assert (isSymbolEqual (sym, OP_SYMBOL (IC_RESULT (ic)))); /* just making sure */
                      /* if left is assigned to registers */
                      if (IS_SYMOP (IC_LEFT (ic)) && bitVectBitValue (_G.totRegAssigned, OP_SYMBOL (IC_LEFT (ic))->key))
                        {
                          pdone += (positionRegs (sym, OP_SYMBOL (IC_LEFT (ic)), 0) > 0);
                        }
                      if (IS_SYMOP (IC_RIGHT (ic)) && bitVectBitValue (_G.totRegAssigned, OP_SYMBOL (IC_RIGHT (ic))->key))
                        {
                          pdone += (positionRegs (sym, OP_SYMBOL (IC_RIGHT (ic)), 0) > 0);
                        }
                      if (pdone > 1)
                        break;
                    }
                }
              for (i = 0; i < sym->uses->size; i++)
                {
                  if (bitVectBitValue (sym->uses, i))
                    {
                      iCode *ic;
                      if (!(ic = hTabItemWithKey (iCodehTab, i)))
                        continue;
                      if (SKIP_IC (ic))
                        continue;
                      if (POINTER_SET (ic) || POINTER_GET (ic))
                        continue;

                      /* if result is assigned to registers */
                      if (IS_SYMOP (IC_RESULT (ic)) && bitVectBitValue (_G.totRegAssigned, OP_SYMBOL (IC_RESULT (ic))->key))
                        {
                          pdone += (positionRegs (sym, OP_SYMBOL (IC_RESULT (ic)), 0) > 0);
                        }
                      if (pdone > 1)
                        break;
                    }
                }
              if (pdone == 0)
                break;          /* second pass only if regs repositioned */
              if (pdone > 1)
                break;
            }
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
              D (fprintf (stderr, "Fill Gap gave up due to positioning for "
                          "%s in function %s\n", sym->name, currFunc ? currFunc->name : "UNKNOWN"));
              continue;
            }
          D (fprintf (stderr, "FILLED GAP for %s in function %s\n", sym->name, currFunc ? currFunc->name : "UNKNOWN"));
          _G.totRegAssigned = bitVectSetBit (_G.totRegAssigned, sym->key);
          sym->isspilt = sym->spillA = 0;
          sym->usl.spillLoc->allocreq--;
          sym->usl.spillLoc = NULL;
          freeAllRegs ();
          change++;
        }
      if (!change)
        break;
    }
}

/*-----------------------------------------------------------------*/
/* findAllBitregs :- returns bit vector of all bit registers       */
/*-----------------------------------------------------------------*/
static bitVect *
findAllBitregs (void)
{
  bitVect *rmask = newBitVect (ds390_nRegs);
  int j;

  for (j = 0; j < ds390_nRegs; j++)
    {
      if (regs390[j].type == REG_BIT)
        rmask = bitVectSetBit (rmask, regs390[j].rIdx);
    }

  return rmask;
}

/*-----------------------------------------------------------------*/
/* ds390_allBitregs :- returns bit vector of all bit registers     */
/*-----------------------------------------------------------------*/
bitVect *
ds390_allBitregs (void)
{
  return _G.allBitregs;
}

/*-----------------------------------------------------------------*/
/* rUmaskForOp :- returns register mask for an operand             */
/*-----------------------------------------------------------------*/
bitVect *
ds390_rUmaskForOp (operand * op)
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

  rumask = newBitVect (ds390_nRegs);

  for (j = 0; j < sym->nRegs; j++)
    {
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
  bitVect *rmask = newBitVect (ds390_nRegs);

  /* do the special cases first */
  if (ic->op == IFX)
    {
      rmask = bitVectUnion (rmask, ds390_rUmaskForOp (IC_COND (ic)));
      goto ret;
    }

  /* for the jumptable */
  if (ic->op == JUMPTABLE)
    {
      rmask = bitVectUnion (rmask, ds390_rUmaskForOp (IC_JTCOND (ic)));

      goto ret;
    }

  /* of all other cases */
  if (IC_LEFT (ic))
    rmask = bitVectUnion (rmask, ds390_rUmaskForOp (IC_LEFT (ic)));


  if (IC_RIGHT (ic))
    rmask = bitVectUnion (rmask, ds390_rUmaskForOp (IC_RIGHT (ic)));

  if (IC_RESULT (ic))
    rmask = bitVectUnion (rmask, ds390_rUmaskForOp (IC_RESULT (ic)));

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
          ic->rMask = newBitVect (ds390_nRegs + 1);

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
#if 0
              /* special case for ruonly */
              if (sym->ruonly && sym->liveFrom != sym->liveTo)
                {
                  int size = getSize (sym->type);
                  int j = DPL_IDX;
                  for (k = 0; k < size; k++)
                    ic->rMask = bitVectSetBit (ic->rMask, j++);
                  continue;
                }
#endif
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
  static struct dbuf_s dbuf;

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

/*-----------------------------------------------------------------*/
/* regTypeNum - computes the type & number of registers required   */
/*-----------------------------------------------------------------*/
static void
regTypeNum ()
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
              continue;
            }

          /* if the symbol has only one definition &
             that definition is a get_pointer */
          if (bitVectnBitsOn (sym->defs) == 1 &&
              (ic = hTabItemWithKey (iCodehTab,
                                     bitVectFirstBit (sym->defs))) &&
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

          if (sym->nRegs > 4)
            {
              fprintf (stderr, "allocated more than 4 or 0 registers for type ");
              printTypeChain (sym->type, stderr);
              fprintf (stderr, "\n");
            }

          /* determine the type of register required */
          if (sym->nRegs == 1 && IS_PTR (sym->type) && sym->uptr)
            sym->regType = REG_PTR;
          else if (IS_BIT (sym->type))
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

  for (i = 0; i < ds390_nRegs; i++)
    regs390[i].isFree = 1;

  for (i = B0_IDX; i < ds390_nBitRegs; i++)
    regs390[i].isFree = 1;
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
#if 0
  if (isOperandInFarSpace (IC_RESULT (ic)))
    {
      if ((dic = farSpacePackable (ic)))
        goto pack;
      else
        return 0;
    }
#else
  if (isOperandInFarSpace (IC_RESULT (ic)) && !farSpacePackable (ic))
    {
      return 0;
    }
#endif

  /* find the definition of iTempNN scanning backwards if we find a
     a use of the true symbol in before we find the definition then
     we cannot */
  for (dic = ic->prev; dic; dic = dic->prev)
    {
      /* if there is a function call then don't pack it */
      if ((dic->op == CALL || dic->op == PCALL))
        {
          dic = NULL;
          break;
        }

      if (SKIP_IC2 (dic))
        continue;

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

      if (IS_SYMOP (IC_LEFT (dic)) && (IC_LEFT (dic)->key == IC_RESULT (ic)->key || IC_LEFT (dic)->key == IC_RIGHT (ic)->key))
        {
          dic = NULL;
          break;
        }

      if (POINTER_SET (dic) && IC_RESULT (dic)->key == IC_RESULT (ic)->key)
        {
          dic = NULL;
          break;
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
  /* replace the result with the result of */
  /* this assignment and remove this assignment */
  bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);

  IC_RESULT (dic) = IC_RESULT (ic);

  if (IS_ITEMP (IC_RESULT (dic)) && OP_SYMBOL (IC_RESULT (dic))->liveFrom > dic->seq)
    {
      OP_SYMBOL (IC_RESULT (dic))->liveFrom = dic->seq;
    }
  /* delete from liverange table also
     delete from all the points inbetween and the new
     one */
  for (sic = dic; sic != ic; sic = sic->next)
    {
      bitVectUnSetBit (sic->rlive, IC_RESULT (ic)->key);
      if (IS_ITEMP (IC_RESULT (dic)))
        bitVectSetBit (sic->rlive, IC_RESULT (dic)->key);
    }

  remiCodeFromeBBlock (ebp, ic);
  bitVectUnSetBit (OP_SYMBOL (IC_RESULT (ic))->defs, ic->key);
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
        {

          /* we are interested only if defined in far space */
          /* or in stack space in case of + & - */

          /* if assigned to a non-symbol then return
             FALSE */
          if (!IS_SYMOP (IC_RIGHT (dic)))
            return NULL;

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

          break;

        }

      /* if we find an usage then we cannot delete it */
      if (IC_LEFT (dic) && IC_LEFT (dic)->key == op->key)
        return NULL;

      if (IC_RIGHT (dic) && IC_RIGHT (dic)->key == op->key)
        return NULL;

      if (POINTER_SET (dic) && IC_RESULT (dic)->key == op->key)
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
/* packRegsForSupport :- reduce some registers for support calls   */
/*-----------------------------------------------------------------*/
static int
packRegsForSupport (iCode * ic, eBBlock * ebp)
{
  int change = 0;

  /* for the left & right operand :- look to see if the
     left was assigned a true symbol in far space in that
     case replace them */
  if (IS_ITEMP (IC_LEFT (ic)) && OP_SYMBOL (IC_LEFT (ic))->liveTo <= ic->seq)
    {
      iCode *dic = findAssignToSym (IC_LEFT (ic), ic);
      iCode *sic;

      if (!dic)
        goto right;

      /* found it we need to remove it from the
         block */
      for (sic = dic; sic != ic; sic = sic->next)
        {
          bitVectUnSetBit (sic->rlive, IC_LEFT (ic)->key);
          sic->rlive = bitVectSetBit (sic->rlive, IC_RIGHT (dic)->key);
        }

      wassert (IS_SYMOP (IC_LEFT (ic)));
      wassert (IS_SYMOP (IC_RIGHT (dic)));
      OP_SYMBOL (IC_LEFT (ic)) = OP_SYMBOL (IC_RIGHT (dic));
      OP_SYMBOL (IC_LEFT (ic))->liveTo = ic->seq;
      IC_LEFT (ic)->key = OP_KEY (IC_RIGHT (dic));
      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
      remiCodeFromeBBlock (ebp, dic);
      hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
      change++;
    }

  /* do the same for the right operand */
right:
  if (!change && IS_ITEMP (IC_RIGHT (ic)) && OP_SYMBOL (IC_RIGHT (ic))->liveTo <= ic->seq)
    {
      iCode *dic = findAssignToSym (IC_RIGHT (ic), ic);
      iCode *sic;

      if (!dic)
        return change;

      /* if this is a subtraction & the result
         is a true symbol in far space then don't pack */
      if (ic->op == '-' && IS_TRUE_SYMOP (IC_RESULT (dic)))
        {
          sym_link *etype = getSpec (operandType (IC_RESULT (dic)));
          if (IN_FARSPACE (SPEC_OCLS (etype)))
            return change;
        }
      /* found it we need to remove it from the
         block */
      for (sic = dic; sic != ic; sic = sic->next)
        {
          bitVectUnSetBit (sic->rlive, IC_RIGHT (ic)->key);
          sic->rlive = bitVectSetBit (sic->rlive, IC_RIGHT (dic)->key);
        }

      wassert (IS_SYMOP (IC_RIGHT (ic)));
      wassert (IS_SYMOP (IC_RIGHT (dic)));
      OP_SYMBOL (IC_RIGHT (ic)) = OP_SYMBOL (IC_RIGHT (dic));
      IC_RIGHT (ic)->key = OP_KEY (IC_RIGHT (dic));
      OP_SYMBOL (IC_RIGHT (ic))->liveTo = ic->seq;
      remiCodeFromeBBlock (ebp, dic);
      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
      hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
      change++;
    }

  return change;
}


/*-------------------------------------------------------------------*/
/* packRegsDPTRnuse - color live ranges that can go into extra DPTRS */
/*-------------------------------------------------------------------*/
static int
packRegsDPTRnuse (operand * op, unsigned dptr)
{
  symbol * opsym;
  int i, key;
  iCode *ic;

  if (!IS_SYMOP (op) || !IS_ITEMP (op))
    return 0;
  opsym = OP_SYMBOL (op);

  if (opsym->remat || opsym->ruonly || opsym->dptr)
    return 0;

  /* first check if any overlapping liverange has already been
     assigned to this DPTR */
  if (opsym->clashes)
    {
      for (i = 0; i < opsym->clashes->size; i++)
        {
          symbol *sym;
          if (bitVectBitValue (opsym->clashes, i))
            {
              sym = hTabItemWithKey (liveRanges, i);
              if (sym->dptr == dptr)
                return 0;
            }
        }
    }

  /* future for more dptrs */
  if (dptr > 1)
    {
      opsym->dptr = dptr;
      return 1;
    }

  /* DPTR1 is special since it is also used as a scratch by the backend.
     so we walk thru the entire live range of this operand and make sure
     DPTR1 will not be used by the backend. The logic here is to find out if
     more than one operand in an icode is in far space then we give up : we
     don't keep it live across functions for now
   */

  ic = hTabFirstItemWK (iCodeSeqhTab, opsym->liveFrom);
  for (; ic && ic->seq <= opsym->liveTo; ic = hTabNextItem (iCodeSeqhTab, &key))
    {
      int nfs = 0;

      if (ic->op == CALL || ic->op == PCALL)
        return 0;

      /* single operand icode are ok */
      if (ic->op == IFX || ic->op == IPUSH)
        continue;

      if (ic->op == SEND)
        {
          if (ic->argreg != 1)
            return 0;
          else
            continue;
        }
      /* four special cases first */
      if (POINTER_GET (ic) && !isOperandEqual (IC_LEFT (ic), op) &&     /* pointer get */
          !OP_SYMBOL (IC_LEFT (ic))->ruonly &&                          /* with result in far space */
          (isOperandInFarSpace (IC_RESULT (ic)) && !isOperandInReg (IC_RESULT (ic))))
        {
          return 0;
        }

      if (POINTER_GET (ic) && !isOperandEqual (IC_LEFT (ic), op) &&     /* pointer get */
          !OP_SYMBOL (IC_LEFT (ic))->ruonly &&                          /* with left in far space */
          (isOperandInFarSpace (IC_LEFT (ic)) && !isOperandInReg (IC_LEFT (ic))))
        {
          return 0;
        }

      if (POINTER_SET (ic) && !isOperandEqual (IC_RESULT (ic), op) &&   /* pointer set */
          !OP_SYMBOL (IC_RESULT (ic))->ruonly &&                        /* with right in far space */
          (isOperandInFarSpace (IC_RIGHT (ic)) && !isOperandInReg (IC_RIGHT (ic))))
        {
          return 0;
        }

      if (POINTER_SET (ic) && !isOperandEqual (IC_RESULT (ic), op) &&   /* pointer set */
          !OP_SYMBOL (IC_RESULT (ic))->ruonly &&                        /* with result in far space */
          (isOperandInFarSpace (IC_RESULT (ic)) && !isOperandInReg (IC_RESULT (ic))))
        {
          return 0;
        }

      if (IC_RESULT (ic) && IS_SYMOP (IC_RESULT (ic)) &&    /* if symbol operand */
          !isOperandEqual (IC_RESULT (ic), op) &&		    /* not the same as this */
          ((isOperandInFarSpace (IC_RESULT (ic)) ||         /* in farspace or */
            OP_SYMBOL (IC_RESULT (ic))->onStack) &&         /* on the stack   */
           !isOperandInReg (IC_RESULT (ic))))               /* and not in register */
        {
          nfs++;
        }
      /* same for left */
      if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)) &&        /* if symbol operand */
          !isOperandEqual (IC_LEFT (ic), op) &&             /* not the same as this */
          ((isOperandInFarSpace (IC_LEFT (ic)) ||           /* in farspace or */
            OP_SYMBOL (IC_LEFT (ic))->onStack) &&           /* on the stack   */
           !isOperandInReg (IC_LEFT (ic))))                 /* and not in register */
        {
          nfs++;
        }
      /* same for right */
      if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)) &&      /* if symbol operand */
          !isOperandEqual (IC_RIGHT (ic), op) &&            /* not the same as this */
          ((isOperandInFarSpace (IC_RIGHT (ic)) ||          /* in farspace or */
            OP_SYMBOL (IC_RIGHT (ic))->onStack) &&          /* on the stack   */
           !isOperandInReg (IC_RIGHT (ic))))                /* and not in register */
        {
          nfs++;
        }

      // Check that no other ops in this range have been assigned to dptr1.
      // I don't understand why this is not caught by the first check, above.
      // But it isn't always, see bug 769624.
      if (IC_RESULT (ic) && IS_SYMOP (IC_RESULT (ic)) && (OP_SYMBOL (IC_RESULT (ic))->dptr == 1))
        {
          //fprintf(stderr, "dptr1 already in use in live range # 1\n");
          return 0;
        }

      if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)) && (OP_SYMBOL (IC_LEFT (ic))->dptr == 1))
        {
          //fprintf(stderr, "dptr1 already in use in live range # 2\n");
          return 0;
        }

      if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)) && (OP_SYMBOL (IC_RIGHT (ic))->dptr == 1))
        {
          //fprintf(stderr, "dptr1 already in use in live range # 3\n");
          return 0;
        }

      if (nfs && IC_RESULT (ic) && IS_SYMOP (IC_RESULT (ic)) && OP_SYMBOL (IC_RESULT (ic))->ruonly)
        return 0;

      if (nfs > 1)
        return 0;
    }
  opsym->dptr = dptr;
  return 1;
}

/*-----------------------------------------------------------------*/
/* packRegsDPTRuse : - will reduce some registers for single Use */
/*-----------------------------------------------------------------*/
static iCode *
packRegsDPTRuse (operand * op)
{
  /* go thru entire liveRange of this variable & check for
     other possible usage of DPTR, if we don't find it then
     assign this to DPTR (ruonly)
   */
  int i, key;
  symbol *sym;
  iCode *ic, *dic;
  sym_link *type;

  if (!IS_SYMOP (op) || !IS_ITEMP (op))
    return NULL;
  if (OP_SYMBOL (op)->remat || OP_SYMBOL (op)->ruonly)
    return NULL;

  /* first check if any overlapping liverange has already been
     assigned to DPTR */
  if (OP_SYMBOL (op)->clashes)
    {
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

  /* no then go thru this guys live range */
  dic = ic = hTabFirstItemWK (iCodeSeqhTab, OP_SYMBOL (op)->liveFrom);
  for (; ic && ic->seq <= OP_SYMBOL (op)->liveTo; ic = hTabNextItem (iCodeSeqhTab, &key))
    {
      if (SKIP_IC3 (ic))
        continue;

      /* if PCALL cannot be sure give up */
      if (ic->op == PCALL)
        return NULL;

      /* if SEND & not the first parameter then give up */
      if (ic->op == SEND && ic->argreg != 1 &&
          ((isOperandInFarSpace (IC_LEFT (ic)) && !isOperandInReg (IC_LEFT (ic))) || isOperandEqual (op, IC_LEFT (ic))))
        return NULL;

      /* if CALL then make sure it is VOID || return value not used
         or the return value is assigned to this one */
      if (ic->op == CALL)
        {
          if (OP_SYMBOL (IC_RESULT (ic))->liveTo == OP_SYMBOL (IC_RESULT (ic))->liveFrom)
            continue;
          type = operandType (IC_RESULT (ic));
          if (getSize (type) == 0 || isOperandEqual (op, IC_RESULT (ic)))
            continue;
          return NULL;
        }

      /* special case of add with a [remat] */
      if (ic->op == '+' &&
          IS_SYMOP (IC_LEFT (ic)) && OP_SYMBOL (IC_LEFT (ic))->remat &&
          isOperandInFarSpace (IC_RIGHT (ic)) && !isOperandInReg (IC_RIGHT (ic)))
        {
          return NULL;
        }

      /* special cases  */
      /* pointerGet */
      if (POINTER_GET (ic) && !isOperandEqual (IC_LEFT (ic), op) && getSize (operandType (IC_LEFT (ic))) > 1)
        return NULL;

      /* pointerSet */
      if (POINTER_SET (ic) && !isOperandEqual (IC_RESULT (ic), op) && getSize (operandType (IC_RESULT (ic))) > 1)
        return NULL;

      /* conditionals can destroy 'b' - make sure B wont
         be used in this one */
      if ((IS_CONDITIONAL (ic) || ic->op == '*' || ic->op == '/' ||
           ic->op == LEFT_OP || ic->op == RIGHT_OP) && getSize (operandType (op)) > 3)
        return NULL;

      /* if this is a cast to a bigger type */
      if (ic->op == CAST)
        {
          if (!IS_PTR (OP_SYM_TYPE (IC_RESULT (ic))) &&
              getSize (OP_SYM_TYPE (IC_RESULT (ic))) > getSize (OP_SYM_TYPE (IC_RIGHT (ic))))
            {
              return NULL;
            }
        }

      /* general case */
      if (IC_RESULT (ic) && IS_SYMOP (IC_RESULT (ic)) &&
          !isOperandEqual (IC_RESULT (ic), op) &&
          (((isOperandInFarSpace (IC_RESULT (ic)) || OP_SYMBOL (IC_RESULT (ic))->onStack) &&
            !isOperandInReg (IC_RESULT (ic))) || OP_SYMBOL (IC_RESULT (ic))->ruonly))
        return NULL;

      if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)) &&
          !isOperandEqual (IC_RIGHT (ic), op) &&
          (OP_SYMBOL (IC_RIGHT (ic))->liveTo >= ic->seq ||
           IS_TRUE_SYMOP (IC_RIGHT (ic)) ||
           OP_SYMBOL (IC_RIGHT (ic))->ruonly) &&
          ((isOperandInFarSpace (IC_RIGHT (ic)) || OP_SYMBOL (IC_RIGHT (ic))->onStack) && !isOperandInReg (IC_RIGHT (ic))))
        return NULL;

      if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)) &&
          !isOperandEqual (IC_LEFT (ic), op) &&
          (OP_SYMBOL (IC_LEFT (ic))->liveTo >= ic->seq ||
           IS_TRUE_SYMOP (IC_LEFT (ic)) ||
           OP_SYMBOL (IC_LEFT (ic))->ruonly) &&
          ((isOperandInFarSpace (IC_LEFT (ic)) || OP_SYMBOL (IC_LEFT (ic))->onStack) && !isOperandInReg (IC_LEFT (ic))))
        return NULL;

      if (IC_LEFT (ic) && IC_RIGHT (ic) &&
          IS_ITEMP (IC_LEFT (ic)) && IS_ITEMP (IC_RIGHT (ic)) &&
          (isOperandInFarSpace (IC_LEFT (ic)) && !isOperandInReg (IC_LEFT (ic))) &&
          (isOperandInFarSpace (IC_RIGHT (ic)) && !isOperandInReg (IC_RIGHT (ic))))
        return NULL;
    }
  OP_SYMBOL (op)->ruonly = 1;
  if (OP_SYMBOL (op)->usl.spillLoc)
    {
      if (OP_SYMBOL (op)->spillA)
        OP_SYMBOL (op)->usl.spillLoc->allocreq--;
      OP_SYMBOL (op)->usl.spillLoc = NULL;
    }
  return dic;
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
/* packRegsForAccUse - pack registers for acc use                  */
/*-----------------------------------------------------------------*/
static void
packRegsForAccUse (iCode * ic)
{
  iCode *uic;

  /* if this is an aggregate, e.g. a one byte char array */
  if (IS_AGGREGATE (operandType (IC_RESULT (ic))))
    {
      return;
    }

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

  /* if the usage is not is an assignment
     or an arithmetic / bitwise / shift operation then not */
  if (POINTER_SET (uic) && getSize (aggrToPtr (operandType (IC_RESULT (uic)), FALSE)) > 1)
    return;

  if (uic->op != '=' && !IS_ARITHMETIC_OP (uic) && !IS_BITWISE_OP (uic) && uic->op != LEFT_OP && uic->op != RIGHT_OP)
    return;

  /* if used in ^ operation then make sure right is not a
     literal */
  if (uic->op == '^' && isOperandLiteral (IC_RIGHT (uic)))
    return;

  /* if shift operation make sure right side is not a literal */
  if (uic->op == RIGHT_OP && (isOperandLiteral (IC_RIGHT (uic)) || getSize (operandType (IC_RESULT (uic))) > 1))
    return;

  if (uic->op == LEFT_OP && (isOperandLiteral (IC_RIGHT (uic)) || getSize (operandType (IC_RESULT (uic))) > 1))
    return;

  /* make sure that the result of this icode is not on the
     stack, since acc is used to compute stack offset */
  if (isOperandOnStack (IC_RESULT (uic)))
    return;

  /* if either one of them in far space then we cannot */
  if ((IS_TRUE_SYMOP (IC_LEFT (uic)) &&
       isOperandInFarSpace (IC_LEFT (uic))) || (IS_TRUE_SYMOP (IC_RIGHT (uic)) && isOperandInFarSpace (IC_RIGHT (uic))))
    return;

  /* if the usage has only one operand then we can */
  if (IC_LEFT (uic) == NULL || IC_RIGHT (uic) == NULL)
    goto accuse;

  /* make sure this is on the left side if not
     a '+' since '+' is commutative */
  if (ic->op != '+' && IC_LEFT (uic)->key != IC_RESULT (ic)->key)
    return;

  /* if the other one is not on stack then we can */
  if (IC_LEFT (uic)->key == IC_RESULT (ic)->key &&
      (IS_ITEMP (IC_RIGHT (uic)) || (IS_TRUE_SYMOP (IC_RIGHT (uic)) && !OP_SYMBOL (IC_RIGHT (uic))->onStack)))
    goto accuse;

  if (IC_RIGHT (uic)->key == IC_RESULT (ic)->key &&
      (IS_ITEMP (IC_LEFT (uic)) || (IS_TRUE_SYMOP (IC_LEFT (uic)) && !OP_SYMBOL (IC_LEFT (uic))->onStack)))
    goto accuse;

  return;

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

  if ((ic->op != IPUSH && ic->op != SEND) || !IS_ITEMP (IC_LEFT (ic)))
    return;

  /* must have only definition & one usage */
  if (bitVectnBitsOn (OP_DEFS (IC_LEFT (ic))) != 1 || bitVectnBitsOn (OP_USES (IC_LEFT (ic))) != 1)
    return;

  /* find the definition */
  if (!(dic = hTabItemWithKey (iCodehTab, bitVectFirstBit (OP_DEFS (IC_LEFT (ic))))))
    return;

  if (dic->op != '=' || POINTER_SET (dic))
    return;

  if (dic->eBBlockNum != ic->eBBlockNum)
    return;

  if (IS_OP_VOLATILE (IC_RIGHT (dic)))
    return;

  if ((IS_SYMOP (IC_RIGHT (dic)) && OP_SYMBOL (IC_RIGHT (dic))->addrtaken) || isOperandGlobal (IC_RIGHT (dic)))
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
  if (OP_SYMBOL (IC_RIGHT (dic))->liveTo < OP_SYMBOL (IC_LEFT (ic))->liveTo)
    {
      OP_SYMBOL (IC_RIGHT (dic))->liveTo = OP_SYMBOL (IC_LEFT (ic))->liveTo;
      OP_SYMBOL (IC_RIGHT (dic))->clashes =
        bitVectUnion (OP_SYMBOL (IC_RIGHT (dic))->clashes, OP_SYMBOL (IC_LEFT (ic))->clashes);
    }
  for (lic = ic; lic && lic != dic; lic = lic->prev)
    {
      bitVectUnSetBit (lic->rlive, IC_LEFT (ic)->key);
      if (IS_ITEMP (IC_RIGHT (dic)))
        bitVectSetBit (lic->rlive, IC_RIGHT (dic)->key);
    }
  if (IS_ITEMP (IC_RIGHT (dic)))
    OP_USES (IC_RIGHT (dic)) = bitVectSetBit (OP_USES (IC_RIGHT (dic)), ic->key);
  /* we now we know that it has one & only one def & use
     and the that the definition is an assignment */
  IC_LEFT (ic) = IC_RIGHT (dic);

  remiCodeFromeBBlock (ebp, dic);
  bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
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

  while (1)
    {
      change = 0;

      /* look for assignments of the form */
      /* iTempNN = TRueSym (someoperation) SomeOperand */
      /*       ....                       */
      /* TrueSym := iTempNN:1             */
      for (ic = ebp->sch; ic; ic = ic->next)
        {
          /* find assignment of the form TrueSym := iTempNN:1 */
          if (ic->op == '=' && !POINTER_SET (ic))
            change += packRegsForAssign (ic, ebp);
        }

      if (!change)
        break;
    }

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
          IS_ITEMP (IC_LEFT (ic)) &&
          IC_LEFT (ic) == IC_RESULT (ic->prev) && isOperandEqual (IC_RESULT (ic), IC_RIGHT (ic->prev)))
        {
          iCode *ic_prev = ic->prev;
          symbol *prev_result_sym = OP_SYMBOL (IC_RESULT (ic_prev));

          ReplaceOpWithCheaperOp (&IC_LEFT (ic), IC_RESULT (ic));
          if (IC_RESULT (ic_prev) != IC_RIGHT (ic))
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
          IS_TRUE_SYMOP (IC_LEFT (ic)) && bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 && !OP_SYMBOL (IC_LEFT (ic))->onStack)
        {

          OP_SYMBOL (IC_RESULT (ic))->remat = 1;
          OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
          OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;

        }

      /* if this is an itemp & used as a pointer
         & assigned to a literal then remat */
      if (IS_ASSIGN_ICODE (ic) &&
          IS_ITEMP (IC_RESULT (ic)) && bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 && isOperandLiteral (IC_RIGHT (ic)))
        {
          OP_SYMBOL (IC_RESULT (ic))->remat = 1;
          OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
          OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
        }

      /* if straight assignment then carry remat flag if
         this is the only definition */
      if (ic->op == '=' && !POINTER_SET (ic) && IS_SYMOP (IC_RIGHT (ic)) && OP_SYMBOL (IC_RIGHT (ic))->remat && !IS_CAST_ICODE (OP_SYMBOL (IC_RIGHT (ic))->rematiCode) && !isOperandGlobal (IC_RESULT (ic)) &&  /* due to bug 1618050 */
          bitVectnBitsOn (OP_SYMBOL (IC_RESULT (ic))->defs) <= 1)
        {
          OP_SYMBOL (IC_RESULT (ic))->remat = OP_SYMBOL (IC_RIGHT (ic))->remat;
          OP_SYMBOL (IC_RESULT (ic))->rematiCode = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
        }

      /* if cast to a generic pointer & the pointer being
         cast is remat, then we can remat this cast as well */
      if (ic->op == CAST && IS_SYMOP (IC_RIGHT (ic)) && !OP_SYMBOL (IC_RESULT (ic))->isreqv && OP_SYMBOL (IC_RIGHT (ic))->remat &&
          bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1)
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

      /* if this is a +/- operation with a rematerizable
         then mark this as rematerializable as well */
      if ((ic->op == '+' || ic->op == '-') &&
          (IS_SYMOP (IC_LEFT (ic)) &&
           IS_ITEMP (IC_RESULT (ic)) &&
           OP_SYMBOL (IC_LEFT (ic))->remat &&
           (!IS_SYMOP (IC_RIGHT (ic)) || !IS_CAST_ICODE (OP_SYMBOL (IC_RIGHT (ic))->rematiCode)) &&
           bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 && IS_OP_LITERAL (IC_RIGHT (ic))))
        {

          //int i = operandLitValue(IC_RIGHT(ic));
          OP_SYMBOL (IC_RESULT (ic))->remat = 1;
          OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
          OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
        }

      /* mark the pointer usages */
      if (POINTER_SET (ic) && IS_SYMOP (IC_RESULT (ic)))
        OP_SYMBOL (IC_RESULT (ic))->uptr = 1;

      if (POINTER_GET (ic) && IS_SYMOP (IC_LEFT (ic)))
        OP_SYMBOL (IC_LEFT (ic))->uptr = 1;

      if (ic->op == RETURN && IS_SYMOP (IC_LEFT (ic)))
        OP_SYMBOL (IC_LEFT (ic))->uptr = 1;

      if (ic->op == RECEIVE && ic->argreg == 1 && IS_SYMOP (IC_RESULT (ic)) && getSize (operandType (IC_RESULT (ic))) <= 3)
        OP_SYMBOL (IC_RESULT (ic))->uptr = 1;

      if (ic->op == SEND && ic->argreg == 1 &&
          IS_SYMOP (IC_LEFT (ic)) && getSize (aggrToPtr (operandType (IC_LEFT (ic)), FALSE)) <= 3)
        OP_SYMBOL (IC_LEFT (ic))->uptr = 1;

      if (!SKIP_IC2 (ic))
        {
          /* if we are using a symbol on the stack
             then we should say ds390_ptrRegReq */
          if (options.useXstack && ic->parmPush && (ic->op == IPUSH || ic->op == IPOP))
            ds390_ptrRegReq++;
          if (ic->op == IFX && IS_SYMOP (IC_COND (ic)))
            ds390_ptrRegReq += ((OP_SYMBOL (IC_COND (ic))->onStack ? !options.stack10bit : 0) +
                                OP_SYMBOL (IC_COND (ic))->iaccess + (SPEC_OCLS (OP_SYMBOL (IC_COND (ic))->etype) == idata));
          else if (ic->op == JUMPTABLE && IS_SYMOP (IC_JTCOND (ic)))
            ds390_ptrRegReq += ((OP_SYMBOL (IC_JTCOND (ic))->onStack ? !options.stack10bit : 0) +
                                OP_SYMBOL (IC_JTCOND (ic))->iaccess + (SPEC_OCLS (OP_SYMBOL (IC_JTCOND (ic))->etype) == idata));
          else
            {
              if (IS_SYMOP (IC_LEFT (ic)))
                ds390_ptrRegReq += ((OP_SYMBOL (IC_LEFT (ic))->onStack ? !options.stack10bit : 0) +
                                    OP_SYMBOL (IC_LEFT (ic))->iaccess + (SPEC_OCLS (OP_SYMBOL (IC_LEFT (ic))->etype) == idata));
              if (IS_SYMOP (IC_RIGHT (ic)))
                ds390_ptrRegReq += ((OP_SYMBOL (IC_RIGHT (ic))->onStack ? !options.stack10bit : 0) +
                                    OP_SYMBOL (IC_RIGHT (ic))->iaccess +
                                    (SPEC_OCLS (OP_SYMBOL (IC_RIGHT (ic))->etype) == idata));
              if (IS_SYMOP (IC_RESULT (ic)))
                ds390_ptrRegReq += ((OP_SYMBOL (IC_RESULT (ic))->onStack ? !options.stack10bit : 0) +
                                    OP_SYMBOL (IC_RESULT (ic))->iaccess +
                                    (SPEC_OCLS (OP_SYMBOL (IC_RESULT (ic))->etype) == idata));
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
#if 1
      /* reduce for support function calls */
      if (ic->supportRtn || ic->op == '+' || ic->op == '-')
        packRegsForSupport (ic, ebp);
#endif
      /* some cases the redundant moves can
         can be eliminated for return statements . Can be elminated for the first SEND */
      if ((ic->op == RETURN ||
           ((ic->op == SEND || ic->op == RECEIVE) && ic->argreg == 1)) && !isOperandInFarSpace (IC_LEFT (ic)) && !options.model)
        {

          packRegsDPTRuse (IC_LEFT (ic));
        }

      if (ic->op == CALL)
        {
          sym_link *ftype = operandType (IC_LEFT (ic));
          if (getSize (operandType (IC_RESULT (ic))) <= 4 && !IFFUNC_ISBUILTIN (ftype))
            {
              packRegsDPTRuse (IC_RESULT (ic));
            }
        }

      /* if pointer set & left has a size more than
         one and right is not in far space */
      if (POINTER_SET (ic) &&
          !isOperandInFarSpace (IC_RIGHT (ic)) &&
          IS_SYMOP (IC_RESULT (ic)) &&
          !OP_SYMBOL (IC_RESULT (ic))->remat &&
          !IS_OP_RUONLY (IC_RIGHT (ic)) && getSize (aggrToPtr (operandType (IC_RESULT (ic)), FALSE)) > 1)
        {

          packRegsDPTRuse (IC_RESULT (ic));
        }

      /* if pointer get */
      if (POINTER_GET (ic) &&
          !isOperandInFarSpace (IC_RESULT (ic)) &&
          IS_SYMOP (IC_LEFT (ic)) &&
          !OP_SYMBOL (IC_LEFT (ic))->remat &&
          !IS_OP_RUONLY (IC_RESULT (ic)) && getSize (aggrToPtr (operandType (IC_LEFT (ic)), FALSE)) > 1)
        {

          packRegsDPTRuse (IC_LEFT (ic));
        }

      /* if this is a cast for intergral promotion then
         check if it's the only use of  the definition of the
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

              iCode *dic = packRegsDPTRuse (IC_RIGHT (ic));
              if (dic)
                {
                  if (IS_ARITHMETIC_OP (dic))
                    {
                      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
                      IC_RESULT (dic) = IC_RESULT (ic);
                      remiCodeFromeBBlock (ebp, ic);
                      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (ic))->defs, ic->key);
                      hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
                      OP_DEFS (IC_RESULT (dic)) = bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
                      ic = ic->prev;
                    }
                  else
                    OP_SYMBOL (IC_RIGHT (ic))->ruonly = 0;
                }
            }
          else
            {

              /* if the type from and type to are the same
                 then if this is the only use then packit */
              if (compareType (operandType (IC_RIGHT (ic)), operandType (IC_LEFT (ic))) == 1)
                {
                  iCode *dic = packRegsDPTRuse (IC_RIGHT (ic));
                  if (dic)
                    {
                      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (ic))->defs, ic->key);
                      IC_RESULT (dic) = IC_RESULT (ic);
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
      if (ic->op == IPUSH || ic->op == SEND)
        {
          packForPush (ic, ebpp, blockno);
        }

      /* pack registers for accumulator use, when the
         result of an arithmetic or bit wise operation
         has only one use, that use is immediately following
         the defintion and the using iCode has only one
         operand or has two operands but one is literal &
         the result of that operation is not on stack then
         we can leave the result of this operation in acc:b
         combination */
      if ((IS_ARITHMETIC_OP (ic)
           || IS_CONDITIONAL (ic)
           || IS_BITWISE_OP (ic)
           || ic->op == LEFT_OP || ic->op == RIGHT_OP
           || (ic->op == ADDRESS_OF && isOperandOnStack (IC_LEFT (ic)))) &&
          IS_ITEMP (IC_RESULT (ic)) && getSize (operandType (IC_RESULT (ic))) <= 2)

        packRegsForAccUse (ic);
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
ds390_assignRegisters (ebbIndex * ebbi)
{
  eBBlock **ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  iCode *ic;
  int i;

  setToNull ((void *) &_G.funcrUsed);
  setToNull ((void *) &_G.regAssigned);
  setToNull ((void *) &_G.totRegAssigned);
  setToNull ((void *) &_G.funcrUsed);
  ds390_ptrRegReq = _G.stackExtend = _G.dataExtend = 0;
  if ((currFunc && IFFUNC_ISREENT (currFunc->type)) || options.stackAuto)
    {
      ds390_nBitRegs = 8;
    }
  else
    {
      ds390_nBitRegs = 0;
    }
  ds390_nRegs = 12 + ds390_nBitRegs;
  _G.allBitregs = findAllBitregs ();

  if (options.model != MODEL_FLAT24)
    options.stack10bit = 0;
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
  regTypeNum ();

  /* and serially allocate registers */
  serialRegAssign (ebbs, count);

  ds390_nRegs = 8;
  freeAllRegs ();
  fillGaps ();
  positionRegsReverse (ebbs, count);
  ds390_nRegs = 12 + ds390_nBitRegs;

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
    redoStackOffsets ();

  /* make sure r0 & r1 are flagged as used if they might be used */
  /* as pointers */
  if (currFunc && ds390_ptrRegReq)
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

  gen390Code (ic);

  /* free up any _G.stackSpil locations allocated */
  applyToSet (_G.stackSpil, deallocStackSpil);
  _G.slocNum = 0;
  setToNull ((void *) &_G.stackSpil);
  setToNull ((void *) &_G.spiltSet);
  /* mark all registers as free */
  ds390_nRegs = 8;
  freeAllRegs ();

  return;
}
