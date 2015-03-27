/* This port is in development, UNSTABLE, DEVELOPERS ONLY! */

/* idea:
   R0-^-R2(R0L-R2H) used for scratch (byte, word, pointer)
   R3L-^-R6H used for bytes
   R6-v-R4 used for ptr
   R15/R6-v-R4 used for words
   
   R7 used for stack
*/

/*------------------------------------------------------------------------
  
  SDCCralloc.c - source file for register allocation. (xa51) specific
  
  Written By -  
  
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

extern void genXA51Code (iCode *);

#define D0(x) 
#define D1(x) x
#define D2(x) x

/* Global data */
static struct
{
  bitVect *spiltSet;
  set *stackSpil;
  bitVect *regAssigned;
  bitVect *totRegAssigned;    /* final set of LRs that got into registers */
  short blockSpil;
  int slocNum;
  bitVect *funcrUsed;		/* registers used in a function */
  int stackExtend;
}
_G;

/* xa51 registers */
regs regsXA51[]={
  // index size type name regMask isFree symbol
  {R0L_ID, 1, REG_SCR, "r0l", 0x0001, 1, NULL}, // r0l used for scratch
  {R0H_ID, 1, REG_SCR, "r0h", 0x0002, 1, NULL}, // r0h used for scratch
  {R1L_ID, 1, REG_SCR, "r1l", 0x0004, 1, NULL}, // r1l used for scratch
  {R1H_ID, 1, REG_SCR, "r1h", 0x0008, 1, NULL}, // r1h used for scratch
  {R2L_ID, 1, REG_PTR, "r2l", 0x0010, 1, NULL},
  {R2H_ID, 1, REG_PTR, "r2h", 0x0020, 1, NULL},
  {R3L_ID, 1, REG_PTR, "r3l", 0x0040, 1, NULL},
  {R3H_ID, 1, REG_PTR, "r3h", 0x0080, 1, NULL},
  {R4L_ID, 1, REG_PTR, "r4l", 0x0100, 1, NULL},
  {R4H_ID, 1, REG_PTR, "r4h", 0x0200, 1, NULL},
  {R5L_ID, 1, REG_PTR, "r5l", 0x0400, 1, NULL},
  {R5H_ID, 1, REG_PTR, "r5h", 0x0800, 1, NULL},
  {R6L_ID, 1, REG_PTR, "r6l", 0x1000, 1, NULL},
  {R6H_ID, 1, REG_PTR, "r6h", 0x2000, 1, NULL},
  {R7L_ID, 1, REG_STK, "r7l", 0x4000, 1, NULL}, // r7=SP
  {R7H_ID, 1, REG_STK, "r7h", 0x8000, 1, NULL}, // r7=SP

  {R0_ID, 2, REG_SCR, "r0",  0x0003, 1, NULL}, // r0 used for scratch
  {R1_ID, 2, REG_SCR, "r1",  0x000c, 1, NULL}, // r1 used for scratch
  {R2_ID, 2, REG_PTR, "r2",  0x0030, 1, NULL}, 
  {R3_ID, 2, REG_PTR, "r3",  0x00c0, 1, NULL},
  {R4_ID, 2, REG_PTR, "r4",  0x0300, 1, NULL}, 
  {R5_ID, 2, REG_PTR, "r5",  0x0c00, 1, NULL}, 
  {R6_ID, 2, REG_PTR, "r6",  0x3000, 1, NULL}, 
  {R7_ID, 2, REG_STK, "r7",  0xc000, 1, NULL}, // r7=SP
#if 0 // some derivates have even more! (only bit/word access no ptr use)
  {R8_ID, 2, REG_GPR, "r8",  0x010000, 1, NULL},
  {R9_ID, 2, REG_GPR, "r9",  0x020000, 1, NULL},
  {R10_ID, 2, REG_GPR, "r10", 0x040000, 1, NULL},
  {R11_ID, 2, REG_GPR, "r11", 0x080000, 1, NULL},
  {R12_ID, 2, REG_GPR, "r12", 0x100000, 1, NULL},
  {R13_ID, 2, REG_GPR, "r13", 0x200000, 1, NULL},
  {R14_ID, 2, REG_GPR, "r14", 0x400000, 1, NULL},
  {R15_ID, 2, REG_GPR, "r15", 0x800000, 1, NULL},
#endif
  {R0R1_ID, 4, REG_GPR, "(r0,r1)", 0x000f, 1, NULL},
  {R2R3_ID, 4, REG_GPR, "(r2,r3)", 0x00f0, 1, NULL},
  {R4R5_ID, 4, REG_GPR, "(r4,r5)", 0x0f00, 1, NULL},
  {R6R7_ID, 4, REG_GPR, "(r6,r7)", 0xf000, 1, NULL},
};

int xa51_nRegs=sizeof(regsXA51)/sizeof(regs);

// the currently in use registers
unsigned long xa51RegsInUse=0;

// this should be set with a command line switch
bool xa51HasGprRegs=0;

/*-----------------------------------------------------------------*/
/* xa51_regWithMask - returns pointer to register with mask        */
/*-----------------------------------------------------------------*/
regs *xa51_regWithMask (unsigned long mask) {
  int i;
  for (i=0; i<xa51_nRegs; i++) {
    if (regsXA51[i].regMask==mask) {
      return &regsXA51[i];
    }
  }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* checkRegsMask - check the consistancy of the regMask redundancy */
/*-----------------------------------------------------------------*/

void checkRegMask(const char *f) { // for debugging purposes only
  int i;
  unsigned long regMask=0;
  
  // rebuild the regmask
  for (i=0; i<xa51_nRegs; i++) {
    if (!regsXA51[i].isFree) {
      regMask |= regsXA51[i].regMask;
    }
  }
  
  // check it
  if (regMask != xa51RegsInUse) {
    fprintf (stderr, "error(%s): regMask inconsistent 0x%08lx != 0x%08lx\n",
	     f, regMask, xa51RegsInUse);
    regMask=regMask^xa51RegsInUse;
    fprintf (stderr, "%s used by %s\n", 
	     xa51_regWithMask(regMask)->name, 
	     xa51_regWithMask(regMask)->sym->name);
    
    exit(1);
    return;
  }
}

char *regTypeToStr(short type) {
  switch (type) 
    {
    case REG_PTR: return "ptr"; break; // pointer
    case REG_GPR: return "gpr"; break; // general purpose
    case REG_CND: return "cnd"; break; // condition (bit)
    case REG_STK: return "stk"; break; // stack
    case REG_SCR: return "scr"; break; // scratch
    default: return "???";
    }
}

/*-----------------------------------------------------------------*/
/* freeReg - frees a previous allocated register                   */
/*-----------------------------------------------------------------*/
static void freeReg (regs * reg, bool silent) {
  
  checkRegMask(__FUNCTION__);
  
  if (!reg) {
    werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
	    "freeReg - freeing NULL register");
    return;
  }
  
  if (!silent) {
    D0(fprintf (stderr, "freeReg: (%08lx) %s (%s) ", xa51RegsInUse, 
		reg->name, reg->sym->name));
  }
  
  if (reg->isFree || ((xa51RegsInUse&reg->regMask)!=reg->regMask)) {
    werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
	    "freeReg - freeing unused register(s)");
    exit (1);
  }
  xa51RegsInUse &= ~reg->regMask;
  reg->isFree = 1;
  reg->sym = NULL;
  if (!silent) D0(fprintf (stderr, "(%08lx)\n", xa51RegsInUse));
  
  checkRegMask(__FUNCTION__);
}

/*-----------------------------------------------------------------*/
/* allocReg - allocates register of given size (byte, word, dword) */
/*            and type (ptr, gpr, cnd)                             */
/*-----------------------------------------------------------------*/
static bool allocReg (unsigned int size, int type, symbol *sym, 
		      int offset, bool silent) {
  int i;
  
  checkRegMask(__FUNCTION__);
  
  if (!silent) {
    D0(fprintf (stderr, "allocReg (%08lx) for %s size:%d, type:%s ", 
		xa51RegsInUse,
		sym->name,
		size, regTypeToStr(type)));
  }
  
  switch (size) 
    {
      // TODO: gaps could be filled for dwords too
    case 1:
      // let's see if we can fill a gap
      for (i=0; i<xa51_nRegs; i++) {
	if (regsXA51[i].size==2 && regsXA51[i].type==type) {
	  unsigned long mask=regsXA51[i].regMask & ~xa51RegsInUse;
	  if (mask && mask!=regsXA51[i].regMask) {
	    regs *reg=xa51_regWithMask(mask);
	    // found a gap
	    sym->regs[offset]=reg;
	    xa51RegsInUse |= mask;
	    reg->isFree=0; // redundant
	    reg->sym = sym;
	    if (!silent) {
	      D0(fprintf (stderr, "(using gap) %s\n", reg->name));
	    }
	    checkRegMask(__FUNCTION__);
	    return TRUE;
	  }
	}
      }
      // no we can't, fall through
    case 2:
      for (i=0; i<xa51_nRegs; i++) {
	if (regsXA51[i].size==size &&
	    regsXA51[i].type==type &&
	    (regsXA51[i].regMask & xa51RegsInUse)==0) {
	  xa51RegsInUse |= regsXA51[i].regMask;
	  regsXA51[i].isFree = 0; // redundant
	  regsXA51[i].sym = sym;
	  if (!silent) {
	    D0(fprintf (stderr, "%s\n", regsXA51[i].name));
	  }
	  sym->regs[offset]=&regsXA51[i];
	  checkRegMask(__FUNCTION__);
	  return TRUE;
	}
      }
      if (!silent) {
	D0(fprintf (stderr, "failed (%08lx)\n", xa51RegsInUse));
      }
      checkRegMask(__FUNCTION__);
      return FALSE;
      break;
    case 3:
      // this must be a generic pointer
      if (!silent) {
	D0(fprintf (stderr, "trying 1+2\n"));
      }
      // get the generic part (gpr regs can't be byte)
      if (allocReg (1, REG_PTR, sym, offset+1, silent)) {
	// get the pointer part
	if (allocReg (2, REG_PTR, sym, offset, silent)) {
	  checkRegMask(__FUNCTION__);
	  return TRUE;
	}
	freeReg(sym->regs[offset+1], silent);
	sym->regs[offset+1]=NULL;
      }
      checkRegMask(__FUNCTION__);
      return FALSE;
      break;
    case 4: // this is a dword
      if (!silent) {
	D0(fprintf (stderr, "trying 2+2\n"));
      }
      if ((xa51HasGprRegs && allocReg (2, REG_GPR, sym, offset, silent)) ||
	  allocReg (2, REG_PTR, sym, offset, silent)) {
	if ((xa51HasGprRegs && allocReg (2, REG_GPR, sym, offset+1, silent)) ||
	    allocReg (2, REG_PTR, sym, offset+1, silent)) {
	  checkRegMask(__FUNCTION__);
	  return TRUE;
	}
      }
      if (sym->regs[offset]) {
	freeReg(sym->regs[offset], FALSE);
	sym->regs[offset]=NULL;
      }
      checkRegMask(__FUNCTION__);
      return FALSE;
      break;
    }
  fprintf (stderr, "\nallocReg: cannot allocate reg of size %d\n", size);
  exit (1);
  return FALSE;
}

/*-------------------------------------------------------------------*/
/* freeAllRegs - frees all registers                                 */
/*-------------------------------------------------------------------*/
// just to be sure, this should not be needed
static void freeAllRegs (void) {
  char regsFreed[132];
  int i;
  int nfr = 0;
  
  checkRegMask(__FUNCTION__);
  
  regsFreed[0]=0;
  for (i=0; i<xa51_nRegs; i++) {
    if (!regsXA51[i].isFree) {
      strcat (regsFreed, regsXA51[i].name);
      strcat (regsFreed, " ");
      regsXA51[i].isFree=1;
      regsXA51[i].sym=NULL;
      nfr++;
    }
  }
  xa51RegsInUse=0;
  if (nfr) {
    fprintf (stderr, "freeAllRegisters: %d regs freed (%s)\n", nfr, regsFreed);
    exit (1);
  }
}

/*-----------------------------------------------------------------*/
/* computeSpillable - given a point find the spillable live ranges */
/*-----------------------------------------------------------------*/
static bitVect *computeSpillable (iCode * ic) {
  bitVect *spillable;
  
  /* spillable live ranges are those that are live at this 
     point . the following categories need to be subtracted
     from this set. 
     a) - those that are already spilt
     b) - if being used by this one
     c) - defined by this one */
  
  spillable = bitVectCopy (ic->rlive);
  spillable =
    bitVectCplAnd (spillable, _G.spiltSet);	/* those already spilt */
  spillable =
    bitVectCplAnd (spillable, ic->uses);	/* used in this one */
  bitVectUnSetBit (spillable, ic->defKey);      /* defined by this one */
  spillable = bitVectIntersect (spillable, _G.regAssigned);
  return spillable;
  
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
  return ((usedInRemaining (operandFromSymbol (sym), ic) ? 0 : 1) &&
	  allDefsOutOfRange (sym->defs, ebp->fSeq, ebp->lSeq));
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
liveRangesWith (bitVect * lrs, int (func) (symbol *, eBBlock *, iCode *),
		eBBlock * ebp, iCode * ic)
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
	  werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
		  "liveRangesWith could not find liveRange");
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
         the spill the smaller of the two */
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
  
  
  for (sym = setFirstItem (itmpStack); sym;
       sym = setNextItem (itmpStack))
    {
      if (bitVectBitValue(sym->clashes,fsym->key)) return 0;
    }
  
  return 1;
}

/*-----------------------------------------------------------------*/
/* isFree - will return 1 if the a free spil location is found     */
/*-----------------------------------------------------------------*/
static DEFSETFUNC (isFree) {
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
  if (sym->isFree &&
      noOverLap (sym->usl.itmpStack, fsym) &&
      /* TODO: this is a waste but causes to many problems 
	 getSize (sym->type) >= getSize (fsym->type)) {
      */
      getSize (sym->type) == getSize (fsym->type)) {
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
  char slocBuffer[30];
  
  D1(fprintf (stderr, "  createStackSpil for %s\n", sym->name));
  
  /* first go try and find a free one that is already 
     existing on the stack */
  if (applyToSet (_G.stackSpil, isFree, &sloc, sym))
    {
      /* found a free one : just update & return */
      sym->usl.spillLoc = sloc;
      sym->stackSpil = 1;
      sloc->isFree = 0;
      addSetHead (&sloc->usl.itmpStack, sym);
      D1(fprintf (stderr, "    using existing %s\n", sloc->name));
      return sym;
    }
  
  sprintf (slocBuffer, "sloc%d", _G.slocNum++);
  sloc = newiTemp (slocBuffer);
  
  /* set the type to the spilling symbol */
  sloc->type = copyLinkChain (sym->type);
  sloc->etype = getSpec (sloc->type);
  SPEC_SCLS (sloc->etype) = S_STACK;
  SPEC_EXTR (sloc->etype) = 0;
  SPEC_STAT (sloc->etype) = 0;
  SPEC_VOLATILE(sloc->etype) = 0;
  SPEC_ABSA(sloc->etype) = 0;
  
  allocLocal (sloc);
  
  sloc->isref = 1;		/* to prevent compiler warning */
  
  currFunc->stack += getSize (sloc->type);
  _G.stackExtend += getSize (sloc->type);
  
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
/* spillThis - spils a specific operand                            */
/*-----------------------------------------------------------------*/
static void
spillThis (symbol * sym)
{
  int i;
  
  D1(fprintf (stderr, "  spillThis: %s\n", sym->name));
  
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
	freeReg (sym->regs[i], FALSE);
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
  
  /* get all live ranges that are rematerizable */
  if ((selectS = liveRangesWith (lrcs, rematable, ebp, ic)))
    {
      /* return the least used of these */
      return leastUsedLR (selectS);
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
      if (!_G.blockSpil &&
          !isiCodeInFunctionCall (ic) &&       
          (selectS = liveRangesWith (lrcs, notUsedInRemaining, ebp, ic)))
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
/* spillSomething - spil some variable & mark registers as free    */
/*-----------------------------------------------------------------*/
static bool
spillSomething (iCode * ic, eBBlock * ebp, symbol * forSym)
{
  symbol *ssym;
  int i;
  
  /* get something we can spil */
  ssym = selectSpil (ic, ebp, forSym);
  
  D1(fprintf (stderr, "  spillSomething: spilling %s\n", ssym->name));
  
  /* mark it as spilt */
  ssym->isspilt = ssym->spillA = 1;
  _G.spiltSet = bitVectSetBit (_G.spiltSet, ssym->key);
  
  /* mark it as not register assigned &
     take it away from the set */
  //bitVectUnSetBit (_G.regAssigned, ssym->key);
  //bitVectUnSetBit (_G.totRegAssigned, ssym->key);
  
  /* mark the registers as free */
  for (i = 0; i < ssym->nRegs; i++) {
    if (ssym->regs[i]) {
      freeReg (ssym->regs[i], FALSE);
      // dont NULL ssym->regs[i], it might be used later
    }
  }
  
  /* if this was a block level spil then insert push & pop 
     at the start & end of block respectively */
  if (ssym->blockSpil)
    {
      iCode *nic = newiCode (IPUSH, operandFromSymbol (ssym), NULL);
      /* add push to the start of the block */
      addiCodeToeBBlock (ebp, nic, (ebp->sch->op == LABEL ?
				    ebp->sch->next : ebp->sch));
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
static bool getRegPtr (iCode * ic, eBBlock * ebp, symbol * sym, short offset) {
  
  D0(fprintf (stderr, "getRegPtr: %s ", sym->name));
  D0(printTypeChain(sym->type, stderr));
  D0(fprintf (stderr, "\n"));
  
  while (1) {
    /* this looks like an infinite loop but 
       in really selectSpil will abort  */
    
    /* try for a ptr type */
    if (allocReg (getSize(sym->type), REG_PTR, sym, offset, FALSE))
      return TRUE;
    
    /* try for gpr type */
    if (xa51HasGprRegs && allocReg (getSize(sym->type), 
				    REG_GPR, sym, offset, FALSE))
      return TRUE;
    
    /* we have to spil */
    if (!spillSomething (ic, ebp, sym))
      return FALSE;
    
  }
}

/*-----------------------------------------------------------------*/
/* getRegGpr - will try for GPR if not spil                        */
/*-----------------------------------------------------------------*/
static bool getRegGpr (iCode * ic, eBBlock * ebp, symbol * sym, short offset) {
  
  D0(fprintf (stderr, "getRegGpr: %s ", sym->name));
  D0(printTypeChain(sym->type, stderr));
  D0(fprintf (stderr, "\n"));
  
  while(1) {
    /* this looks like an infinite loop but 
       in really selectSpil will abort  */
    
    /* try for gpr type */
    if (xa51HasGprRegs && allocReg (getSize(sym->type), 
				    REG_GPR, sym, offset, FALSE))
      return TRUE;
    
    if (allocReg (getSize(sym->type), REG_PTR, sym, offset, FALSE))
      return TRUE;
    
    /* we have to spil */
    if (!spillSomething (ic, ebp, sym))
      return FALSE;
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
  
  for (sym = hTabFirstItem (liveRanges, &k); sym;
       sym = hTabNextItem (liveRanges, &k))
    {
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
      
      if (sym->nRegs) {
	int i;
	
	bitVectUnSetBit (_G.regAssigned, sym->key);
	
	/* free the regs */
	for (i=0; i < sym->nRegs; i++) {
	  freeReg (sym->regs[i], FALSE);
	}
      }
    }
}

/*-----------------------------------------------------------------*/
/* willCauseSpill - determines if allocating will cause a spill    */
/*-----------------------------------------------------------------*/
static bool willCauseSpill (symbol *sym) {
  int i;
  // do it the rude way
  if (allocReg (getSize(sym->type), sym->regType, sym, 0, TRUE) ||
      allocReg (getSize(sym->type), sym->regType==REG_PTR?REG_GPR:REG_PTR, 
		sym, 0, TRUE)) {
    // so we can, but we won't
    for (i=0; i<sym->nRegs; i++) {
      freeReg (sym->regs[i], TRUE);
      sym->regs[i]=NULL;
    }
    return FALSE;
  }
  D1(fprintf (stderr, "  %s will cause a spill\n", sym->name));
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* positionRegs - the allocator can allocate same registers to res- */
/* ult and operand, if this happens make sure they are in the same */
/* position as the operand otherwise chaos results                 */
/*-----------------------------------------------------------------*/
static int
positionRegs (symbol * result, symbol * opsym)
{
  int count = min (result->nRegs, opsym->nRegs);
  int i, j = 0, shared = 0;
  int changed = 0;
  
  /* if the result has been spilt then cannot share */
  if (opsym->isspilt)
    return 0;
 again:
  shared = 0;
  /* first make sure that they actually share */
  for (i = 0; i < count; i++)
    {
      for (j = 0; j < count; j++)
	{
	  if (result->regs[i] == opsym->regs[j] && i != j)
	    {
	      shared = 1;
	      goto xchgPositions;
	    }
	}
    }
 xchgPositions:
  if (shared)
    {
      regs *tmp = result->regs[i];
      result->regs[i] = result->regs[j];
      result->regs[j] = tmp;
      changed ++;
      D2(fprintf (stderr, "positionRegs: rearranged regs for %s and %s\n",
		  result->name, opsym->name));
      goto again;
    }
  return changed;
}

/*-----------------------------------------------------------------*/
/* serialRegAssign - serially allocate registers to the variables  */
/*-----------------------------------------------------------------*/
static void
serialRegAssign (eBBlock ** ebbs, int count)
{
  int i;
  
  /* for all blocks */
  for (i = 0; i < count; i++) {
    
    iCode *ic;
    
    if (ebbs[i]->noPath &&
	(ebbs[i]->entryLabel != entryLabel &&
	 ebbs[i]->entryLabel != returnLabel))
      continue;
    
    /* of all instructions do */
    for (ic = ebbs[i]->sch; ic; ic = ic->next) {
      
      /* if result is present && is a true symbol */
      if (IC_RESULT (ic) && ic->op != IFX &&
	  IS_TRUE_SYMOP (IC_RESULT (ic))) {
	OP_SYMBOL (IC_RESULT (ic))->allocreq++;
      }
      
      /* take away registers from live
	 ranges that end at this instruction */
      deassignLRs (ic, ebbs[i]);
      
      /* some don't need registers */
      if (SKIP_IC2 (ic) ||
	  ic->op == JUMPTABLE ||
	  ic->op == IFX ||
	  ic->op == IPUSH ||
	  ic->op == IPOP ||
	  (IC_RESULT (ic) && POINTER_SET (ic)))
	continue;
      
      /* now we need to allocate registers
	 only for the result */
      if (IC_RESULT (ic)) {
	symbol *sym = OP_SYMBOL (IC_RESULT (ic));
	bitVect *spillable;
	int willCS;
	
	/* Make sure any spill location is definately allocated */
	if (sym->isspilt && !sym->remat && sym->usl.spillLoc &&
	    !sym->usl.spillLoc->allocreq) {
	  sym->usl.spillLoc->allocreq++;
	}
	
        /* if it does not need or is spilt 
	   or is already assigned to registers
	   or will not live beyond this instructions */
	if (!sym->nRegs ||
	    sym->isspilt ||
	    bitVectBitValue (_G.regAssigned, sym->key) ||
	    sym->liveTo <= ic->seq)
	  continue;
	
	/* if some liverange has been spilt at the block level
	   and this one live beyond this block then spil this
	   to be safe */
	if (_G.blockSpil && sym->liveTo > ebbs[i]->lSeq) {
	  spillThis (sym);
	  continue;
	}
	/* if trying to allocate this will cause
	   a spill and there is nothing to spill 
	   or this one is rematerializable then
	   spill this one */
	willCS = willCauseSpill (sym);
	spillable = computeSpillable (ic);
	if (sym->remat || (willCS && bitVectIsZero (spillable))) {		      
	  spillThis (sym);
	  continue;		      
	}
	
	/* If the live range preceeds the point of definition 
	   then ideally we must take into account registers that 
	   have been allocated after sym->liveFrom but freed
	   before ic->seq. This is complicated, so spill this
	   symbol instead and let fillGaps handle the allocation. */
	if (sym->liveFrom < ic->seq) {
	    spillThis (sym);
	    continue;		      
	}
	
	/* if it has a spillocation & is used less than
	   all other live ranges then spill this */
	if (willCS) {
	  if (sym->usl.spillLoc) {
	    symbol *leastUsed = leastUsedLR (liveRangesWith (spillable,
							     allLRs, ebbs[i], ic));
	    if (leastUsed && leastUsed->used > sym->used) {
	      spillThis (sym);
	      continue;
	    }
	  } else {
	    /* if none of the liveRanges have a spillLocation then better
	       to spill this one than anything else already assigned to registers */
	    if (liveRangesWith(spillable,noSpilLoc,ebbs[i],ic)) {
	      /* if this is local to this block then we might find a block spil */
	      if (!(sym->liveFrom >= ebbs[i]->fSeq && sym->liveTo <= ebbs[i]->lSeq)) {
		spillThis (sym);
		continue;
	      }
	    }
	  }
	}
	
	/* else we assign registers to it */
	_G.regAssigned = bitVectSetBit (_G.regAssigned, sym->key);
	_G.totRegAssigned = bitVectSetBit (_G.totRegAssigned, sym->key);
	
	if (sym->regType == REG_PTR)
	  getRegPtr (ic, ebbs[i], sym, 0);
	else
	  getRegGpr (ic, ebbs[i], sym, 0);
	
	/* if it shares registers with operands make sure
	   that they are in the same position */
	if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)) &&
	    OP_SYMBOL (IC_LEFT (ic))->nRegs && ic->op != '=') {
	  positionRegs (OP_SYMBOL (IC_RESULT (ic)),
			OP_SYMBOL (IC_LEFT (ic)));
	}
	/* do the same for the right operand */
	if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)) &&
	    OP_SYMBOL (IC_RIGHT (ic))->nRegs) {
	  positionRegs (OP_SYMBOL (IC_RESULT (ic)),
			OP_SYMBOL (IC_RIGHT (ic)));
	}
      }
    }
  }
}

/*-----------------------------------------------------------------*/
/* rUmaskForOp :- returns register mask for an operand             */
/*-----------------------------------------------------------------*/
bitVect *xa51_rUmaskForOp (operand * op) {
  bitVect *rumask;
  symbol *sym;
  int j;
  
  /* only temporaries are assigned registers */
  if (!IS_ITEMP (op))
    return NULL;
  
  sym = OP_SYMBOL (op);
  
  /* if spilt or no registers assigned to it 
     then nothing */
  if (sym->isspilt || !sym->nRegs || !sym->regs[0])
    return NULL;
  
  rumask = newBitVect (xa51_nRegs);
  
  for (j = 0; j < sym->nRegs; j++) {
    rumask = bitVectSetBit (rumask,
			    sym->regs[j]->rIdx);
  }
  return rumask;
}

/*-----------------------------------------------------------------*/
/* regsUsedIniCode :- returns bit vector of registers used in iCode */
/*-----------------------------------------------------------------*/
static bitVect *
regsUsedIniCode (iCode * ic)
{
  bitVect *rmask = newBitVect (xa51_nRegs);
  
  /* do the special cases first */
  if (ic->op == IFX)
    {
      rmask = bitVectUnion (rmask,
			    xa51_rUmaskForOp (IC_COND (ic)));
      goto ret;
    }
  
  /* for the jumptable */
  if (ic->op == JUMPTABLE)
    {
      rmask = bitVectUnion (rmask,
			    xa51_rUmaskForOp (IC_JTCOND (ic)));
      
      goto ret;
    }
  
  /* of all other cases */
  if (IC_LEFT (ic))
    rmask = bitVectUnion (rmask,
			  xa51_rUmaskForOp (IC_LEFT (ic)));
  
  
  if (IC_RIGHT (ic))
    rmask = bitVectUnion (rmask,
			  xa51_rUmaskForOp (IC_RIGHT (ic)));
  
  if (IC_RESULT (ic))
    rmask = bitVectUnion (rmask,
			  xa51_rUmaskForOp (IC_RESULT (ic)));
  
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
      
      if (ebbs[i]->noPath &&
	  (ebbs[i]->entryLabel != entryLabel &&
	   ebbs[i]->entryLabel != returnLabel))
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
	  ic->rMask = newBitVect (xa51_nRegs + 1);
	  
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
		  werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
			  "createRegMask cannot find live range");
		  exit (0);
		}
	      
	      /* if no register assigned to it */
	      if (!sym->nRegs || sym->isspilt)
		continue;
	      
	      /* for all the registers allocated to it */
	      for (k = 0; k < sym->nRegs; k++)
		if (sym->regs[k])
		  ic->rMask =
		    bitVectSetBit (ic->rMask, sym->regs[k]->rIdx);
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
  char *s = buffer;
  iCode *ic = sym->rematiCode;
  
  while (1)
    {
      
      /* if plus or minus print the right hand side */
      if (ic->op == '+' || ic->op == '-')
	{
	  sprintf (s, "0x%04x %c ", (int) operandLitValue (IC_RIGHT (ic)),
		   ic->op);
	  s += strlen (s);
	  ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
	  continue;
	}
      
      /* cast then continue */
      if (IS_CAST_ICODE(ic)) {
	ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
	continue;
      }
      /* we reached the end */
      sprintf (s, "%s", OP_SYMBOL (IC_LEFT (ic))->rname);
      break;
    }
  
  return buffer;
}

/*-----------------------------------------------------------------*/
/* regTypeNum - computes the type & number of registers required   */
/*-----------------------------------------------------------------*/
static void
regTypeNum (eBBlock *ebbs)
{
  symbol *sym;
  int k;
  iCode *ic;
  
  /* for each live range do */
  for (sym = hTabFirstItem (liveRanges, &k); sym;
       sym = hTabNextItem (liveRanges, &k))
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
#if 0 // not yet
	  if (sym->ruonly || sym->accuse)
	    {
	      if (IS_AGGREGATE (sym->type) || sym->isptr)
		sym->type = aggrToPtr (sym->type, FALSE);
	      continue;
	    }
#endif
	  
	  /* if the symbol has only one definition &
	     that definition is a get_pointer */
	  if (bitVectnBitsOn (sym->defs) == 1 &&
	      (ic = hTabItemWithKey (iCodehTab,
				     bitVectFirstBit (sym->defs))) &&
	      POINTER_GET (ic) &&
	      !sym->noSpilLoc &&
	      !IS_BITVAR (sym->etype))
	    {
	      /* and that pointer is remat in data space */
	      if (OP_SYMBOL (IC_LEFT (ic))->remat &&
		  !IS_CAST_ICODE(OP_SYMBOL (IC_LEFT (ic))->rematiCode) &&
		  DCL_TYPE (aggrToPtr (operandType(IC_LEFT(ic)), FALSE)) == POINTER)
		{
		  /* create a psuedo symbol & force a spil */
		  symbol *psym = newSymbol (rematStr (OP_SYMBOL (IC_LEFT (ic))), 1);
		  psym->type = sym->type;
		  psym->etype = sym->etype;
		  strcpy (psym->rname, psym->name);
		  sym->isspilt = 1;
		  sym->usl.spillLoc = psym;
#if 0 // an alternative fix for bug #480076
		  /* now this is a useless assignment to itself */
		  remiCodeFromeBBlock (ebbs, ic);
#else
		  /* now this really is an assignment to itself, make it so;
		     it will be optimized out later */
		  ic->op='=';
		  IC_RIGHT(ic)=IC_RESULT(ic);
		  IC_LEFT(ic)=NULL;
#endif
		  continue;
		}
	      
	      /* if in data space or idata space then try to
	         allocate pointer register */
	      
	    }
	  
	  /* if not then we require registers */
#if 0
	  sym->nRegs = ((IS_AGGREGATE (sym->type) || sym->isptr) ?
			getSize (sym->type = aggrToPtr (sym->type, FALSE)) :
			getSize (sym->type));
#else
	  {
	    int size=((IS_AGGREGATE (sym->type) || sym->isptr) ?
		      getSize (sym->type = aggrToPtr (sym->type, FALSE)) :
		      getSize (sym->type));
	    switch (size) 
	      {
	      case 1: // byte
	      case 2: // word or pointer
		sym->nRegs=1;
		break;
	      case 3: // generic pointer
		sym->nRegs=2;
		break;
	      case 4: // dword or float
		sym->nRegs=2;
		break;
	      default: 
		fprintf (stderr, "regTypeNum: unknown size\n");
		exit (1);
	      }
	  }
#endif
	  
	  if (sym->nRegs > 4)
	    {
	      fprintf (stderr, "allocated more than 4 or 0 registers for type ");
	      printTypeChain (sym->type, stderr);
	      fprintf (stderr, "\n");
	      exit (1);
	    }
	  
	  /* determine the type of register required */
	  if (IS_PTR (sym->type))
	    sym->regType = REG_PTR;
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
/* packRegsForAssign - register reduction for assignment           */
/*-----------------------------------------------------------------*/
static int
packRegsForAssign (iCode * ic, eBBlock * ebp)
{
  iCode *dic, *sic;
  
  if (!IS_ITEMP (IC_RIGHT (ic)) ||
      OP_LIVETO (IC_RIGHT (ic)) > ic->seq) {
    return 0;
  }

  /* find the definition of iTempNN scanning backwards */
  for (dic = ic->prev; dic; dic = dic->prev) {
    
    /* if there is a function call then don't pack it */
    if ((dic->op == CALL || dic->op == PCALL)) {
      dic = NULL;
      break;
    }
    
    if (SKIP_IC2 (dic))
      continue;
    
    if (IS_SYMOP (IC_RESULT (dic)) &&
	IC_RESULT (dic)->key == IC_RIGHT (ic)->key) {
      break;
    }
    
  }
  
  if (!dic)
    return 0;			/* did not find */

  /* found the definition */
  /* replace the result with the result of */
  /* this assignment and remove this assignment */
  bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
  IC_RESULT (dic) = IC_RESULT (ic);
  
  if (IS_ITEMP (IC_RESULT (dic)) && 
      OP_SYMBOL (IC_RESULT (dic))->liveFrom > dic->seq)
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
  bitVectUnSetBit(OP_SYMBOL(IC_RESULT(ic))->defs,ic->key);
  hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
  OP_DEFS(IC_RESULT (dic))=bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
  return 1;
  
}

/*-----------------------------------------------------------------*/
/* findAssignToSym : scanning backwards looks for first assig found */
/*-----------------------------------------------------------------*/
static iCode *
findAssignToSym (operand * op, iCode * ic)
{
  iCode *dic;
  
  for (dic = ic->prev; dic; dic = dic->prev)
    {
      
      /* if definition by assignment */
      if (dic->op == '=' &&
	  !POINTER_SET (dic) &&
	  IC_RESULT (dic)->key == op->key
	  /*          &&  IS_TRUE_SYMOP(IC_RIGHT(dic)) */
	  )
	{
	  
	  /* we are interested only if defined in far space */
	  /* or in stack space in case of + & - */
	  
	  /* if assigned to a non-symbol then return
	     FALSE */
	  if (!IS_SYMOP (IC_RIGHT (dic)))
	    return NULL;
	  
	  /* if the symbol is in far space then
	     we should not */
	  if (isOperandInFarSpace (IC_RIGHT (dic)))
	    return NULL;
	  
	  /* for + & - operations make sure that
	     if it is on the stack it is the same
	     as one of the three operands */
	  if ((ic->op == '+' || ic->op == '-') &&
	      OP_SYMBOL (IC_RIGHT (dic))->onStack)
	    {
	      
	      if (IC_RESULT (ic)->key != IC_RIGHT (dic)->key &&
		  IC_LEFT (ic)->key != IC_RIGHT (dic)->key &&
		  IC_RIGHT (ic)->key != IC_RIGHT (dic)->key)
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
	if (IC_RESULT (sic) &&
	    IC_RESULT (sic)->key == IC_RIGHT (dic)->key)
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
  iCode *dic, *sic;
  
  /* for the left & right operand :- look to see if the
     left was assigned a true symbol in far space in that
     case replace them */

  if (IS_ITEMP (IC_LEFT (ic)) &&
      OP_SYMBOL (IC_LEFT (ic))->liveTo <= ic->seq)
    {
      dic = findAssignToSym (IC_LEFT (ic), ic);
      
      if (!dic)
	goto right;
      
      /* found it we need to remove it from the
         block */
      for (sic = dic; sic != ic; sic = sic->next)
	bitVectUnSetBit (sic->rlive, IC_LEFT (ic)->key);
      
      OP_SYMBOL(IC_LEFT (ic))=OP_SYMBOL(IC_RIGHT (dic));
      IC_LEFT (ic)->key = OP_SYMBOL(IC_RIGHT (dic))->key;
      remiCodeFromeBBlock (ebp, dic);
      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
      hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
      change++;
    }
  
  /* do the same for the right operand */
 right:
  if (!change &&
      IS_ITEMP (IC_RIGHT (ic)) &&
      OP_SYMBOL (IC_RIGHT (ic))->liveTo <= ic->seq)
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
	bitVectUnSetBit (sic->rlive, IC_RIGHT (ic)->key);
      
      OP_SYMBOL (IC_RIGHT (ic)) = OP_SYMBOL (IC_RIGHT (dic));
      IC_RIGHT (ic)->key = OP_KEY (IC_RIGHT (dic));
      
      remiCodeFromeBBlock (ebp, dic);
      bitVectUnSetBit(OP_DEFS(IC_RESULT(dic)), dic->key);
      hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
      change++;
    }
  
  return change;
}


/*-----------------------------------------------------------------*/
/* packRegsForOneuse : - will reduce some registers for single Use */
/*-----------------------------------------------------------------*/
static iCode *
packRegsForOneuse (iCode * ic, operand * op, eBBlock * ebp)
{
  bitVect *uses;
  iCode *dic, *sic;

  /* if returning a literal then do nothing */
  if (!IS_SYMOP (op))
    return NULL;
  
  if (ic->op != RETURN &&
      ic->op != SEND &&
      !POINTER_SET (ic) &&
      !POINTER_GET (ic))
    return NULL;
  
  /* this routine will mark the a symbol as used in one 
     instruction use only && if the defintion is local 
     (ie. within the basic block) && has only one definition &&
     that definiion is either a return value from a 
     function or does not contain any variables in
     far space */
  uses = bitVectCopy (OP_USES (op));
  bitVectUnSetBit (uses, ic->key);	/* take away this iCode */
  if (!bitVectIsZero (uses))	/* has other uses */
    return NULL;
  
  /* if it has only one defintion */
  if (bitVectnBitsOn (OP_DEFS (op)) > 1)
    return NULL;		/* has more than one definition */
  
  /* get that definition */
  if (!(dic =
	hTabItemWithKey (iCodehTab,
			 bitVectFirstBit (OP_DEFS (op)))))
    return NULL;
  
#if 0
  /* if that only usage is a cast */
  if (dic->op == CAST) {
    /* to a bigger type */
    if (getSize(OP_SYM_TYPE(IC_RESULT(dic))) > 
	getSize(OP_SYM_TYPE(IC_RIGHT(dic)))) {
      /* than we can not, since we cannot predict the usage of b & acc */
      return NULL;
    }
  }
#endif
  
  /* found the definition now check if it is local */
  if (dic->seq < ebp->fSeq ||
      dic->seq > ebp->lSeq)
    return NULL;		/* non-local */
  
  /* now check if it is the return from
     a function call */
  if (dic->op == CALL || dic->op == PCALL)
    {
      if (ic->op != SEND && ic->op != RETURN &&
	  !POINTER_SET(ic) && !POINTER_GET(ic))
	{
	  OP_SYMBOL (op)->ruonly = 1;
	  return dic;
	}
      dic = dic->next;
    }
  
#if 0  
  /* otherwise check that the definition does
     not contain any symbols in far space */
  if (isOperandInFarSpace (IC_LEFT (dic)) ||
      isOperandInFarSpace (IC_RIGHT (dic)) ||
      IS_OP_RUONLY (IC_LEFT (ic)) ||
      IS_OP_RUONLY (IC_RIGHT (ic)))
    {
      return NULL;
    }
#endif

#if 0  
  /* if pointer set then make sure the pointer
     is one byte */
  if (POINTER_SET (dic) &&
      !IS_DATA_PTR (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)))
    return NULL;
  
  if (POINTER_GET (dic) &&
      !IS_DATA_PTR (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)))
    return NULL;
#endif
  
  sic = dic;
  
  /* also make sure the intervenening instructions
     don't have any thing in far space */
  for (dic = dic->next; dic && dic != ic && sic != ic; dic = dic->next)
    {
      
      /* if there is an intervening function call then no */
      if (dic->op == CALL || dic->op == PCALL)
	return NULL;

#if 0
      /* if pointer set then make sure the pointer
         is one byte */
      if (POINTER_SET (dic) &&
	  !IS_DATA_PTR (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)))
	return NULL;
      
      if (POINTER_GET (dic) &&
	  !IS_DATA_PTR (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)))
	return NULL;
#endif
      
      /* if address of & the result is remat the okay */
      if (dic->op == ADDRESS_OF &&
	  OP_SYMBOL (IC_RESULT (dic))->remat)
	continue;

#if 0      
      /* if operand has size of three or more & this
         operation is a '*','/' or '%' then 'b' may
         cause a problem */
      if ((dic->op == '%' || dic->op == '/' || dic->op == '*') &&
	  getSize (operandType (op)) >= 3)
	return NULL;
#endif
      
#if 0
      /* if left or right or result is in far space */
      if (isOperandInFarSpace (IC_LEFT (dic)) ||
	  isOperandInFarSpace (IC_RIGHT (dic)) ||
	  isOperandInFarSpace (IC_RESULT (dic)) ||
	  IS_OP_RUONLY (IC_LEFT (dic)) ||
	  IS_OP_RUONLY (IC_RIGHT (dic)) ||
	  IS_OP_RUONLY (IC_RESULT (dic)))
	{
	  return NULL;
	}
      /* if left or right or result is on stack */
      if (isOperandOnStack(IC_LEFT(dic)) ||
	  isOperandOnStack(IC_RIGHT(dic)) ||
	  isOperandOnStack(IC_RESULT(dic))) {
	return NULL;
      }
#endif
    }
  
  OP_SYMBOL (op)->ruonly = 1;
  fprintf (stderr, "%s is used only once in line %d.\n", 
	   OP_SYMBOL(op)->name, ic->lineno);
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
  if (IS_LITERAL(rtype) ||
      (IS_BITVAR (ltype) && IN_BITSPACE (SPEC_OCLS (ltype))))
    return TRUE;
  else
    return FALSE;
}

/*-----------------------------------------------------------------*/
/* packForPush - hueristics to reduce iCode for pushing            */
/*-----------------------------------------------------------------*/
static void
packForPush (iCode * ic, eBBlock * ebp)
{
  iCode *dic, *lic;
  bitVect *dbv;

  if (ic->op != IPUSH || !IS_ITEMP (IC_LEFT (ic)))
    return;
  
  /* must have only definition & one usage */
  if (bitVectnBitsOn (OP_DEFS (IC_LEFT (ic))) != 1 ||
      bitVectnBitsOn (OP_USES (IC_LEFT (ic))) != 1)
    return;
  
  /* find the definition */
  if (!(dic = hTabItemWithKey (iCodehTab,
			       bitVectFirstBit (OP_DEFS (IC_LEFT (ic))))))
    return;
  
  if (dic->op != '=' || POINTER_SET (dic))
    return;
  
  /* make sure the right side does not have any definitions
     inbetween */
  dbv = OP_DEFS(IC_RIGHT(dic));
  for (lic = ic; lic && lic != dic ; lic = lic->prev) {
    if (bitVectBitValue(dbv,lic->key)) 
      return ;
  }
  /* make sure they have the same type */
  {
    sym_link *itype=operandType(IC_LEFT(ic));
    sym_link *ditype=operandType(IC_RIGHT(dic));
    
    if (SPEC_USIGN(itype)!=SPEC_USIGN(ditype) ||
	SPEC_LONG(itype)!=SPEC_LONG(ditype))
      return;
  }
  /* extend the live range of replaced operand if needed */
  if (OP_SYMBOL(IC_RIGHT(dic))->liveTo < ic->seq) {
    OP_SYMBOL(IC_RIGHT(dic))->liveTo = ic->seq;
  }
  /* we now know that it has one & only one def & use
     and the that the definition is an assignment */
  IC_LEFT (ic) = IC_RIGHT (dic);
  
  remiCodeFromeBBlock (ebp, dic);
  bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
  hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
}

/*-----------------------------------------------------------------*/
/* packRegisters - does some transformations to reduce register    */
/*                   pressure                                      */
/*-----------------------------------------------------------------*/
static void packRegisters (eBBlock * ebp) {
  iCode *ic;
  int change = 0;
  
  while (1) {
    change = 0;
    
    for (ic = ebp->sch; ic; ic = ic->next) {
      if (ic->op == '=')
	change += packRegsForAssign (ic, ebp);
    }
    
    if (!change)
      break;
  }
  
  for (ic = ebp->sch; ic; ic = ic->next)
    {
      /* if the condition of an if instruction
         is defined in the previous instruction and
	 this is the only usage then
         mark the itemp as a conditional */
      if ((IS_CONDITIONAL (ic) ||
	   (IS_BITWISE_OP(ic) && isBitwiseOptimizable (ic)))) {
	if (ic->next && ic->next->op == IFX &&
	    bitVectnBitsOn (OP_USES(IC_RESULT(ic)))==1 &&
	    isOperandEqual (IC_RESULT (ic), IC_COND (ic->next)) &&
	    OP_SYMBOL (IC_RESULT (ic))->liveTo <= ic->next->seq) {
	  OP_SYMBOL (IC_RESULT (ic))->regType = REG_CND;
	  continue;
	}
      }

#if 0
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
	  !IS_CAST_ICODE(OP_SYMBOL (IC_RIGHT (ic))->rematiCode) &&
	  bitVectnBitsOn (OP_SYMBOL (IC_RESULT (ic))->defs) <= 1)
	{
	  
	  OP_SYMBOL (IC_RESULT (ic))->remat =
	    OP_SYMBOL (IC_RIGHT (ic))->remat;
	  OP_SYMBOL (IC_RESULT (ic))->rematiCode =
	    OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
	}
      
      /* if cast to a generic pointer & the pointer being
	 cast is remat, then we can remat this cast as well */
      if (ic->op == CAST && 
	  IS_SYMOP(IC_RIGHT(ic)) &&
	  OP_SYMBOL(IC_RIGHT(ic))->remat ) {
	sym_link *to_type = operandType(IC_LEFT(ic));
	sym_link *from_type = operandType(IC_RIGHT(ic));
	if (IS_GENPTR(to_type) && IS_PTR(from_type)) {		      
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
	   IS_OP_LITERAL (IC_RIGHT (ic))) &&
	  OP_SYMBOL (IC_LEFT (ic))->remat &&
	  (!IS_SYMOP (IC_RIGHT (ic)) || !IS_CAST_ICODE(OP_SYMBOL (IC_RIGHT (ic))->rematiCode)) &&
	  bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1)
	{
	  OP_SYMBOL (IC_RESULT (ic))->remat = 1;
	  OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
	  OP_SYMBOL (IC_RESULT (ic))->usl.spillLoc = NULL;
	}
#endif
      
      /* mark the pointer usages */
      if (POINTER_SET (ic))
	OP_SYMBOL (IC_RESULT (ic))->uptr = 1;
      
      if (POINTER_GET (ic))
	OP_SYMBOL (IC_LEFT (ic))->uptr = 1;
      
      /* reduce for support function calls */
      if (ic->supportRtn || ic->op == '+' || ic->op == '-')
	packRegsForSupport (ic, ebp);
      
      /* some cases the redundant moves can
         can be eliminated for return statements */
      if (ic->op == RETURN || ic->op == SEND) {
	  packRegsForOneuse (ic, IC_LEFT (ic), ebp);
	}
      
      /* if pointer set & left has a size more than
         one and right is not in far space */
      if (POINTER_SET (ic) &&
	  !isOperandInFarSpace (IC_RIGHT (ic)) &&
	  !OP_SYMBOL (IC_RESULT (ic))->remat &&
	  !IS_OP_RUONLY (IC_RIGHT (ic)) &&
	  getSize (aggrToPtr (operandType (IC_RESULT (ic)), FALSE)) > 1)
	
	packRegsForOneuse (ic, IC_RESULT (ic), ebp);
      
      /* if pointer get */
      if (POINTER_GET (ic) &&
	  !isOperandInFarSpace (IC_RESULT (ic)) &&
	  !OP_SYMBOL (IC_LEFT (ic))->remat &&
	  !IS_OP_RUONLY (IC_RESULT (ic)) &&
	  getSize (aggrToPtr (operandType (IC_LEFT (ic)), FALSE)) > 1)
	
	packRegsForOneuse (ic, IC_LEFT (ic), ebp);
      
      
      /* if this is cast for intergral promotion then
         check if only use of  the definition of the 
         operand being casted/ if yes then replace
         the result of that arithmetic operation with 
         this result and get rid of the cast */
      if (ic->op == CAST)
	{
	  sym_link *fromType = operandType (IC_RIGHT (ic));
	  sym_link *toType = operandType (IC_LEFT (ic));
	  
	  if (IS_INTEGRAL (fromType) && IS_INTEGRAL (toType) &&
	      getSize (fromType) != getSize (toType) &&
	      SPEC_USIGN (fromType) == SPEC_USIGN (toType))
	    {
	      
	      iCode *dic = packRegsForOneuse (ic, IC_RIGHT (ic), ebp);
	      if (dic)
		{
		  if (IS_ARITHMETIC_OP (dic))
		    {		       
		      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
		      IC_RESULT (dic) = IC_RESULT (ic);
		      remiCodeFromeBBlock (ebp, ic);
		      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(ic))->defs,ic->key);
		      hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
		      OP_DEFS(IC_RESULT (dic))=bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
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
	      if (compareType (operandType (IC_RIGHT (ic)),
			       operandType (IC_LEFT (ic))) == 1)
		{
		  iCode *dic = packRegsForOneuse (ic, IC_RIGHT (ic), ebp);
		  if (dic)
		    {
		      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
		      IC_RESULT (dic) = IC_RESULT (ic);
		      remiCodeFromeBBlock (ebp, ic);
		      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(ic))->defs,ic->key);
		      hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
		      OP_DEFS(IC_RESULT (dic))=bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
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
	  packForPush (ic, ebp);
	}
    }
}

/*-----------------------------------------------------------------*/
/* assignRegisters - assigns registers to each live range as need  */
/*-----------------------------------------------------------------*/
void
xa51_assignRegisters (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  iCode *ic;
  int i;
  
  setToNull ((void *) &_G.funcrUsed);
  setToNull ((void *) &_G.totRegAssigned);
  _G.stackExtend = 0;
  
  /* change assignments this will remove some
     live ranges reducing some register pressure */
  for (i = 0; i < count; i++)
    packRegisters (ebbs[i]);

  /* liveranges probably changed by register packing
     so we compute them again */
  recomputeLiveRanges (ebbs, count);

  if (options.dump_i_code)
    dumpEbbsToFileExt (DUMP_PACK, ebbi);
  
  /* first determine for each live range the number of 
     registers & the type of registers required for each */
  regTypeNum (*ebbs);
  
  /* and serially allocate registers */
  serialRegAssign (ebbs, count);
  
  freeAllRegs ();
  
  /* if stack was extended then tell the user */
  if (_G.stackExtend)
    {
      werror(I_EXTENDED_STACK_SPILS,
	     _G.stackExtend,currFunc->name,"");
      _G.stackExtend = 0;
    }
  
  /* after that create the register mask
     for each of the instruction */
  createRegMask (ebbs, count);
  
  /* redo that offsets for stacked automatic variables */
  redoStackOffsets ();
  
  if (options.dump_i_code)
    {
      dumpEbbsToFileExt (DUMP_RASSGN, ebbi);
      dumpLiveRanges (DUMP_LRANGE, liveRanges);
    }
  
  /* do the overlaysegment stuff SDCCmem.c */
  doOverlays (ebbs, count);
  
  /* now get back the chain */
  ic = iCodeLabelOptimize (iCodeFromeBBlock (ebbs, count));
  
  genXA51Code (ic);
  
  /* free up any _G.stackSpil locations allocated */
  applyToSet (_G.stackSpil, deallocStackSpil);
  _G.slocNum = 0;
  setToNull ((void *) &_G.stackSpil);
  setToNull ((void *) &_G.spiltSet);
  /* mark all registers as free */
  freeAllRegs ();
  
  return;
}
