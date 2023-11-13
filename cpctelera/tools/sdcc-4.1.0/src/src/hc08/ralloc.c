/*------------------------------------------------------------------------

  SDCCralloc.c - source file for register allocation. 68HC08 specific

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

extern void genhc08Code (iCode *);

#define D(x)

// Build the old allocator. It can be used by command-line options
#define OLDRALLOC 1

/* Global data */
static struct
  {
    bitVect *spiltSet;
    set *stackSpil;
    bitVect *regAssigned;
    bitVect *totRegAssigned;    /* final set of LRs that got into registers */
    short blockSpil;
    int slocNum;
    bitVect *funcrUsed;         /* registers used in a function */
    int stackExtend;
    int dataExtend;
  }
_G;

/* Shared with gen.c */
int hc08_ptrRegReq;             /* one byte pointer register required */

/* 6808 registers */
reg_info regshc08[] =
{

  {REG_GPR, A_IDX,   "a",  HC08MASK_A,  NULL, 0, 1},
  {REG_GPR, X_IDX,   "x",  HC08MASK_X,  NULL, 0, 1},
  {REG_GPR, H_IDX,   "h",  HC08MASK_H,  NULL, 0, 1},
  {REG_PTR, HX_IDX,  "hx", HC08MASK_HX, NULL, 0, 1},
  {REG_GPR, XA_IDX,  "xa", HC08MASK_XA, NULL, 0, 1},

  {REG_CND, CND_IDX, "C",  0, NULL, 0, 1},
  {0,       SP_IDX,  "sp", 0, NULL, 0, 1},
};
int hc08_nRegs = 7;

reg_info *hc08_reg_a;
reg_info *hc08_reg_x;
reg_info *hc08_reg_h;
reg_info *hc08_reg_hx;
reg_info *hc08_reg_xa;
reg_info *hc08_reg_sp;

static void spillThis (symbol *);
static void freeAllRegs ();

/*-----------------------------------------------------------------*/
/* hc08_regWithIdx - returns pointer to register with index number */
/*-----------------------------------------------------------------*/
reg_info *
hc08_regWithIdx (int idx)
{
  int i;

  for (i = 0; i < hc08_nRegs; i++)
    if (regshc08[i].rIdx == idx)
      return &regshc08[i];

  werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
          "regWithIdx not found");
  exit (1);
}

/*-----------------------------------------------------------------*/
/* hc08_freeReg - frees a register                                      */
/*-----------------------------------------------------------------*/
void
hc08_freeReg (reg_info * reg)
{
  if (!reg)
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
              "hc08_freeReg - Freeing NULL register");
      exit (1);
    }

  reg->isFree = 1;

  switch (reg->rIdx)
    {
      case A_IDX:
        if (hc08_reg_x->isFree)
          hc08_reg_xa->isFree = 1;
        break;
      case X_IDX:
        if (hc08_reg_a->isFree)
          hc08_reg_xa->isFree = 1;
        if (hc08_reg_h->isFree)
          hc08_reg_hx->isFree = 1;
        break;
      case H_IDX:
        if (hc08_reg_x->isFree)
          hc08_reg_hx->isFree = 1;
        break;
      case HX_IDX:
        hc08_reg_h->isFree = 1;
        hc08_reg_x->isFree = 1;
        if (hc08_reg_a->isFree)
          hc08_reg_xa->isFree = 1;
        break;
      case XA_IDX:
        hc08_reg_x->isFree = 1;
        hc08_reg_a->isFree = 1;
        if (hc08_reg_h->isFree)
          hc08_reg_hx->isFree = 1;
        break;
      default:
        break;
    }
}


/*-----------------------------------------------------------------*/
/* hc08_useReg - marks a register  as used                         */
/*-----------------------------------------------------------------*/
void
hc08_useReg (reg_info * reg)
{
  reg->isFree = 0;

  switch (reg->rIdx)
    {
      case A_IDX:
        hc08_reg_xa->aop = NULL;
        hc08_reg_xa->isFree = 0;
        break;
      case X_IDX:
        hc08_reg_xa->aop = NULL;
        hc08_reg_xa->isFree = 0;
        hc08_reg_hx->aop = NULL;
        hc08_reg_hx->isFree = 0;
        break;
      case H_IDX:
        hc08_reg_hx->aop = NULL;
        hc08_reg_hx->isFree = 0;
        break;
      case HX_IDX:
        hc08_reg_h->aop = NULL;
        hc08_reg_h->isFree = 0;
        hc08_reg_x->aop = NULL;
        hc08_reg_x->isFree = 0;
        break;
      case XA_IDX:
        hc08_reg_x->aop = NULL;
        hc08_reg_x->isFree = 0;
        hc08_reg_a->aop = NULL;
        hc08_reg_a->isFree = 0;
        break;
      default:
        break;
    }
}

/*-----------------------------------------------------------------*/
/* hc08_dirtyReg - marks a register as dirty                       */
/*-----------------------------------------------------------------*/
void
hc08_dirtyReg (reg_info * reg, bool freereg)
{
  reg->aop = NULL;

  switch (reg->rIdx)
    {
      case A_IDX:
        hc08_reg_xa->aop = NULL;
	hc08_reg_xa->isLitConst = 0;
	hc08_reg_a->aop = NULL;
	hc08_reg_a->isLitConst = 0;
        break;
      case X_IDX:
        hc08_reg_xa->aop = NULL;
	hc08_reg_xa->isLitConst = 0;
        hc08_reg_hx->aop = NULL;
	hc08_reg_hx->isLitConst = 0;
	hc08_reg_x->aop = NULL;
	hc08_reg_x->isLitConst = 0;
        break;
      case H_IDX:
        hc08_reg_hx->aop = NULL;
	hc08_reg_hx->isLitConst = 0;
	hc08_reg_h->aop = NULL;
	hc08_reg_h->isLitConst = 0;
        break;
      case HX_IDX:
        hc08_reg_hx->aop = NULL;
	hc08_reg_hx->isLitConst = 0;
        hc08_reg_h->aop = NULL;
	hc08_reg_h->isLitConst = 0;
        hc08_reg_x->aop = NULL;
	hc08_reg_x->isLitConst = 0;
        break;
      case XA_IDX:
        hc08_reg_xa->aop = NULL;
	hc08_reg_xa->isLitConst = 0;
        hc08_reg_x->aop = NULL;
	hc08_reg_x->isLitConst = 0;
        hc08_reg_a->aop = NULL;
	hc08_reg_a->isLitConst = 0;
        break;
      default:
        break;
    }
  if (freereg)
    hc08_freeReg(reg);
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
  if (sym->isFree &&
      noOverLap (sym->usl.itmpStack, fsym) &&
      getSize (sym->type) >= getSize (fsym->type))
    {
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
  struct dbuf_s dbuf;
  int useXstack, model;

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
  SPEC_SCLS (sloc->etype) = S_DATA;
  SPEC_EXTR (sloc->etype) = 0;
  SPEC_STAT (sloc->etype) = 0;
  SPEC_VOLATILE(sloc->etype) = 0;
  SPEC_ABSA(sloc->etype) = 0;

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
          hc08_freeReg (sym->regs[i]);
          sym->regs[i] = NULL;
        }
    }

  if (sym->usl.spillLoc && !sym->remat)
    sym->usl.spillLoc->allocreq++;
  return;
}

/*-----------------------------------------------------------------*/
/* updateRegUsage -  update the registers in use at the start of   */
/*                   this icode                                    */
/*-----------------------------------------------------------------*/
static void
updateRegUsage (iCode * ic)
{
  int reg;

  // update the registers in use at the start of this icode
  for (reg=0; reg<hc08_nRegs; reg++)
    {
      if (regshc08[reg].isFree)
        {
          ic->riu &= ~(regshc08[reg].mask);
        }
      else
        {
          ic->riu |= (regshc08[reg].mask);
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

/*------------------------------------------------------------------*/
/* verifyRegsAssigned - make sure an iTemp is properly initialized; */
/* it should either have registers or have beed spilled. Otherwise, */
/* there was an uninitialized variable, so just spill this to get   */
/* the operand in a valid state.                                    */
/*------------------------------------------------------------------*/
static void
verifyRegsAssigned (operand *op, iCode * ic)
{
  symbol * sym;

  if (!op) return;
  if (!IS_ITEMP (op)) return;

  sym = OP_SYMBOL (op);
  if (sym->isspilt) return;
  if (!sym->nRegs) return;
  if (sym->regs[0]) return;

  /* Don't warn for new allocator, since this is not used by default (until Thoruop is implemented for spillocation compaction). */
  /*if (z80_opts.oldralloc)
    werrorfl (ic->filename, ic->lineno, W_LOCAL_NOINIT, sym->prereqv ? sym->prereqv->name : sym->name);*/

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

      if (ebbs[i]->noPath &&
          (ebbs[i]->entryLabel != entryLabel &&
           ebbs[i]->entryLabel != returnLabel))
        continue;

      /* for all instructions do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          updateRegUsage(ic);

          /* if this is an ipop that means some live
             range will have to be assigned again */
          if (ic->op == IPOP)
              reassignLR (IC_LEFT (ic));

          /* if result is present && is a true symbol */
          if (IC_RESULT (ic) && ic->op != IFX &&
              IS_TRUE_SYMOP (IC_RESULT (ic)))
            {
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
            {
              continue;
            }

          /* now we need to allocate registers only for the result */
          if (IC_RESULT (ic))
            {
              symbol *sym = OP_SYMBOL (IC_RESULT (ic));

              /* Make sure any spill location is definitely allocated */
              if (sym->isspilt && !sym->remat && sym->usl.spillLoc &&
                  !sym->usl.spillLoc->allocreq)
                {
                  sym->usl.spillLoc->allocreq++;
                }

              /* if it does not need or is spilt
                 or is already assigned to registers
                 or will not live beyond this instructions */
              if (!sym->nRegs ||
                  sym->isspilt ||
                  bitVectBitValue (_G.regAssigned, sym->key) ||
                  sym->liveTo <= ic->seq)
                {
                  continue;
                }

              spillThis (sym);
            }
        }
    }

  /* Check for and fix any problems with uninitialized operands */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath &&
          (ebbs[i]->entryLabel != entryLabel &&
           ebbs[i]->entryLabel != returnLabel))
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
/* rUmaskForOp :- returns register mask for an operand             */
/*-----------------------------------------------------------------*/
bitVect *
hc08_rUmaskForOp (operand * op)
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

  rumask = newBitVect (hc08_nRegs);

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
  bitVect *rmask = newBitVect (hc08_nRegs);

  /* do the special cases first */
  if (ic->op == IFX)
    {
      rmask = bitVectUnion (rmask, hc08_rUmaskForOp (IC_COND (ic)));
      goto ret;
    }

  /* for the jumptable */
  if (ic->op == JUMPTABLE)
    {
      rmask = bitVectUnion (rmask, hc08_rUmaskForOp (IC_JTCOND (ic)));
      goto ret;
    }

  /* of all other cases */
  if (IC_LEFT (ic))
    rmask = bitVectUnion (rmask, hc08_rUmaskForOp (IC_LEFT (ic)));

  if (IC_RIGHT (ic))
    rmask = bitVectUnion (rmask, hc08_rUmaskForOp (IC_RIGHT (ic)));

  if (IC_RESULT (ic))
    rmask = bitVectUnion (rmask, hc08_rUmaskForOp (IC_RESULT (ic)));

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
          ic->rSurv = newBitVect(port->num_regs);
          ic->rUsed = regsUsedIniCode (ic);
          _G.funcrUsed = bitVectUnion (_G.funcrUsed, ic->rUsed);

          /* now create the register mask for those
             registers that are in use : this is a
             super set of ic->rUsed */
          ic->rMask = newBitVect (hc08_nRegs + 1);

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
                  fprintf(stderr, "\tmissing live range: key=%d\n", j);
                  exit (0);
                }

              /* if no register assigned to it */
              if (!sym->nRegs || sym->isspilt)
                continue;

              /* for all the registers allocated to it */
              for (k = 0; k < sym->nRegs; k++)
                {
                  if (!sym->regs[k])
                    continue;
                  ic->rMask = bitVectSetBit (ic->rMask, sym->regs[k]->rIdx);
                  if (sym->liveTo != ic->key)
                    ic->rSurv = bitVectSetBit (ic->rSurv, sym->regs[k]->rIdx);
                }
            }
        }
    }
}


/*-----------------------------------------------------------------*/
/* regTypeNum - computes the type & number of registers required   */
/*-----------------------------------------------------------------*/
static void
regTypeNum (eBBlock *ebbs)
{
  symbol *sym;
  int k;

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
          if (sym->ruonly || sym->accuse)
            {
              if (IS_AGGREGATE (sym->type) || sym->isptr)
                sym->type = aggrToPtr (sym->type, FALSE);
              continue;
            }

          /* if not then we require registers */
          sym->nRegs = ((IS_AGGREGATE (sym->type) || sym->isptr) ?
                        getSize (sym->type = aggrToPtr (sym->type, FALSE)) :
                        getSize (sym->type));

          if (sym->nRegs > 8)
            {
              fprintf (stderr, "allocated more than 8 registers for type ");
              printTypeChain (sym->type, stderr);
              fprintf (stderr, "\n");
            }

          /* determine the type of register required */
          if (sym->nRegs == 1 && IS_PTR (sym->type) && sym->uptr)
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
/* freeAllRegs - mark all registers as free                        */
/*-----------------------------------------------------------------*/
static void
freeAllRegs ()
{
  int i;

  for (i = 0; i < hc08_nRegs; i++)
    {
      regshc08[i].isFree = 1;
      regshc08[i].aop = NULL;
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


#if 0
static void
packRegsForLiteral (iCode * ic)
{
  int k;
  iCode *uic;

  if (ic->op != '=')
    return;
  if (POINTER_SET (ic))
    return;
  if (!IS_LITERAL (getSpec (operandType (IC_RIGHT (ic)))))
    return;
  if (bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) > 1)
    return;

  for (k=0; k< OP_USES (IC_RESULT (ic))->size; k++)
    if (bitVectBitValue (OP_USES (IC_RESULT (ic)), k))
      {
        uic = hTabItemWithKey (iCodehTab, k);
        if (!uic) continue;

        if (uic->op != IFX && uic->op != JUMPTABLE)
          {
            if (IC_LEFT (uic) && IC_LEFT (uic)->key == IC_RESULT (ic)->key)
              ReplaceOpWithCheaperOp(&IC_LEFT(uic), IC_RIGHT(ic));
            if (IC_RIGHT (uic) && IC_RIGHT (uic)->key == IC_RESULT (ic)->key)
              ReplaceOpWithCheaperOp(&IC_RIGHT(uic), IC_RIGHT(ic));
            if (IC_RESULT (uic) && IC_RESULT (uic)->key == IC_RESULT (ic)->key)
              ReplaceOpWithCheaperOp(&IC_RESULT(uic), IC_RIGHT(ic));
          }
      }

}
#endif


/*-----------------------------------------------------------------*/
/* packRegsForAssign - register reduction for assignment           */
/*-----------------------------------------------------------------*/
static int
packRegsForAssign (iCode * ic, eBBlock * ebp)
{
  iCode *dic, *sic;

  if (!IS_ITEMP (IC_RIGHT (ic)) ||
      OP_SYMBOL (IC_RIGHT (ic))->isind ||
      OP_LIVETO (IC_RIGHT (ic)) > ic->seq)
    {
      return 0;
    }

  /* if the true symbol is defined in far space or on stack
     then we should not since this will increase register pressure */
#if 0
  if (isOperandInFarSpace(IC_RESULT(ic)) && !farSpacePackable(ic))
    {
      return 0;
    }
#endif

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
          if (!OP_SYMBOL (IC_RESULT (ic))->ismyparm
              && !OP_SYMBOL (IC_RESULT (ic))->islocal)
            {
              crossedCall = 1;
            }
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
              (IC_COND (dic)->key == IC_RESULT (ic)->key ||
               IC_COND (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }
        }
      else
        {
          if (IS_TRUE_SYMOP (IC_RESULT (dic)) &&
              IS_OP_VOLATILE (IC_RESULT (dic)))
            {
              dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_RESULT (dic)) &&
              IC_RESULT (dic)->key == IC_RIGHT (ic)->key)
            {
              if (POINTER_SET (dic))
                dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_RIGHT (dic)) &&
              (IC_RIGHT (dic)->key == IC_RESULT (ic)->key ||
               IC_RIGHT (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }

          if (IS_SYMOP (IC_LEFT (dic)) &&
              (IC_LEFT (dic)->key == IC_RESULT (ic)->key ||
               IC_LEFT (dic)->key == IC_RIGHT (ic)->key))
            {
              dic = NULL;
              break;
            }

          if (POINTER_SET (dic) &&
              IC_RESULT (dic)->key == IC_RESULT (ic)->key)
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
      if (dic->op == '=' &&
          !POINTER_SET (dic) &&
          IC_RESULT (dic)->key == op->key
          &&  IS_TRUE_SYMOP(IC_RIGHT(dic))
        )
        break;  /* found where this temp was defined */

      /* if we find an usage then we cannot delete it */
      if (IC_LEFT (dic) && IC_LEFT (dic)->key == op->key)
        return NULL;

      if (IC_RIGHT (dic) && IC_RIGHT (dic)->key == op->key)
        return NULL;

      if (POINTER_SET (dic) && IC_RESULT (dic)->key == op->key)
        return NULL;
    }

  if (!dic)
    return NULL;   /* didn't find any assignment to op */
  /* we are interested only if defined in far space */
  /* or in stack space in case of + & - */

  /* if assigned to a non-symbol then don't repack regs */
  if (!IS_SYMOP (IC_RIGHT (dic)))
    return NULL;

  /* if the symbol's address has been taken, there might be a */
  /* non-obvious assignment to it, and so we should not */
  if (OP_SYMBOL (IC_RIGHT (dic))->addrtaken)
    return NULL;

  /* if the symbol is volatile then we should not */
  if (isOperandVolatile (IC_RIGHT (dic), TRUE))
    return NULL;
  /* XXX TODO --- should we be passing FALSE to isOperandVolatile()?
     What does it mean for an iTemp to be volatile, anyway? Passing
     TRUE is more cautious but may prevent possible optimizations */

  /* if the symbol is in far space then we should not */
  /* if (isOperandInFarSpace (IC_RIGHT (dic)))
    return NULL; */


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
/* reassignAliasedSym - used by packRegsForSupport to replace      */
/*                      redundant iTemp with equivalent symbol     */
/*-----------------------------------------------------------------*/
static void
reassignAliasedSym (eBBlock *ebp, iCode *assignment, iCode *use, operand *op)
{
  iCode *ic;
  unsigned oldSymKey, newSymKey;

  oldSymKey = op->key;
  newSymKey = IC_RIGHT(assignment)->key;

  /* only track live ranges of compiler-generated temporaries */
  if (!IS_ITEMP(IC_RIGHT(assignment)))
    newSymKey = 0;

  /* update the live-value bitmaps */
  for (ic = assignment; ic != use; ic = ic->next) {
    bitVectUnSetBit (ic->rlive, oldSymKey);
    if (newSymKey != 0)
      ic->rlive = bitVectSetBit (ic->rlive, newSymKey);
  }

  /* update the sym of the used operand */
  OP_SYMBOL(op) = OP_SYMBOL(IC_RIGHT(assignment));
  op->key = OP_SYMBOL(op)->key;

  /* update the sym's liverange */
  if ( OP_LIVETO(op) < ic->seq )
    setToRange(op, ic->seq, FALSE);

  /* remove the assignment iCode now that its result is unused */
  remiCodeFromeBBlock (ebp, assignment);
  bitVectUnSetBit(OP_SYMBOL(IC_RESULT(assignment))->defs, assignment->key);
  hTabDeleteItem (&iCodehTab, assignment->key, assignment, DELETE_ITEM, NULL);
}


/*-----------------------------------------------------------------*/
/* packRegsForSupport :- reduce some registers for support calls   */
/*-----------------------------------------------------------------*/
static int
packRegsForSupport (iCode * ic, eBBlock * ebp)
{
  iCode *dic;
  int changes = 0;

  /* for the left & right operand :- look to see if the
     left was assigned a true symbol in far space in that
     case replace them */

  if (IS_ITEMP (IC_LEFT (ic)) &&
      OP_SYMBOL (IC_LEFT (ic))->liveTo <= ic->seq)
    {
      dic = findAssignToSym (IC_LEFT (ic), ic);

      if (dic)
        {
          /* found it we need to remove it from the block */
          reassignAliasedSym (ebp, dic, ic, IC_LEFT(ic));
          changes++;
        }
    }

  /* do the same for the right operand */
  if (IS_ITEMP (IC_RIGHT (ic)) &&
      OP_SYMBOL (IC_RIGHT (ic))->liveTo <= ic->seq)
    {
      iCode *dic = findAssignToSym (IC_RIGHT (ic), ic);

      if (dic)
        {
          /* found it we need to remove it from the block */
          reassignAliasedSym (ebp, dic, ic, IC_RIGHT(ic));
          changes++;
        }
    }

  return changes;
}


#if 0
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

  /* only up to 2 bytes */
  if (getSize (operandType (op)) > (fReturnSizeHC08 - 2))
    return NULL;

  return NULL;

  if (ic->op != SEND && //RETURN
      ic->op != SEND &&
      !POINTER_SET (ic) &&
      !POINTER_GET (ic))
    return NULL;

  if (ic->op == SEND && ic->argreg != 1)
    return NULL;

  /* this routine will mark the symbol as used in one
     instruction use only && if the definition is local
     (ie. within the basic block) && has only one definition &&
     that definition is either a return value from a
     function or does not contain any variables in
     far space */
  uses = bitVectCopy (OP_USES (op));
  bitVectUnSetBit (uses, ic->key);      /* take away this iCode */
  if (!bitVectIsZero (uses))    /* has other uses */
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
      if (getSize(OP_SYM_TYPE(IC_RESULT(dic))) > getSize(OP_SYM_TYPE(IC_RIGHT(dic))))
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
      if (ic->op != SEND && ic->op != RETURN &&
          !POINTER_SET(ic) && !POINTER_GET(ic))
        {
          OP_SYMBOL (op)->ruonly = 1;
          return dic;
        }
      dic = dic->next;
    }


//  /* otherwise check that the definition does
//     not contain any symbols in far space */
//  if (isOperandInFarSpace (IC_LEFT (dic)) ||
//      isOperandInFarSpace (IC_RIGHT (dic)) ||
//      IS_OP_RUONLY (IC_LEFT (ic)) ||
//      IS_OP_RUONLY (IC_RIGHT (ic)))
//    {
//      return NULL;
//    }

  /* if pointer set then make sure the pointer
     is one byte */
#if 0
  if (POINTER_SET (dic) &&
      !IS_DATA_PTR (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)))
    return NULL;

  if (POINTER_GET (dic) &&
      !IS_DATA_PTR (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)))
    return NULL;
#endif

  sic = dic;

  /* make sure the intervening instructions
     don't have anything in far space */
  for (dic = dic->next; dic && dic != ic && sic != ic; dic = dic->next)
    {
      /* if there is an intervening function call then no */
      if (dic->op == CALL || dic->op == PCALL)
        return NULL;
      /* if pointer set then make sure the pointer
         is one byte */
#if 0
      if (POINTER_SET (dic) &&
          !IS_DATA_PTR (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)))
        return NULL;

      if (POINTER_GET (dic) &&
          !IS_DATA_PTR (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)))
        return NULL;
#endif
      /* if address of & the result is remat then okay */
      if (dic->op == ADDRESS_OF &&
          OP_SYMBOL (IC_RESULT (dic))->remat)
        continue;

      /* if operand has size of three or more & this
         operation is a '*','/' or '%' then 'b' may
         cause a problem */
#if 0
      if ((dic->op == '%' || dic->op == '/' || dic->op == '*') &&
          getSize (operandType (op)) >= 3)
        return NULL;
#endif

      /* if left or right or result is in far space */
//      if (isOperandInFarSpace (IC_LEFT (dic)) ||
//        isOperandInFarSpace (IC_RIGHT (dic)) ||
//        isOperandInFarSpace (IC_RESULT (dic)) ||
//        IS_OP_RUONLY (IC_LEFT (dic)) ||
//        IS_OP_RUONLY (IC_RIGHT (dic)) ||
//        IS_OP_RUONLY (IC_RESULT (dic)))
//      {
//        return NULL;
//      }
//      /* if left or right or result is on stack */
//     if (isOperandOnStack(IC_LEFT(dic)) ||
//        isOperandOnStack(IC_RIGHT(dic)) ||
//        isOperandOnStack(IC_RESULT(dic))) {
//      return NULL;
//     }
    }

  OP_SYMBOL (op)->ruonly = 1;
  return sic;
}
#endif

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
/* isCommutativeOp - tests whether this op cares what order its    */
/*                   operands are in                               */
/*-----------------------------------------------------------------*/
bool isCommutativeOp2(unsigned int op)
{
  if (op == '+' || op == '*' || op == EQ_OP ||
      op == '^' || op == '|' || op == BITWISEAND)
    return TRUE;
  else
    return FALSE;
}

/*-----------------------------------------------------------------*/
/* operandUsesAcc2 - determines whether the code generated for this */
/*                  operand will have to use the accumulator       */
/*-----------------------------------------------------------------*/
bool operandUsesAcc2(operand *op)
{
  if (!op)
    return FALSE;

  if (IS_SYMOP(op)) {
    symbol *sym = OP_SYMBOL(op);
    memmap *symspace;

    if (sym->accuse)
      return TRUE;  /* duh! */

    if (IS_ITEMP(op))
      {
        if (SPIL_LOC(op)) {
          sym = SPIL_LOC(op);  /* if spilled, look at spill location */
        } else {
          return FALSE;  /* more checks? */
        }
      }

    symspace = SPEC_OCLS(sym->etype);

    if (IN_BITSPACE(symspace))
      return TRUE;  /* fetching bit vars uses the accumulator */

    if (IN_FARSPACE(symspace) || IN_CODESPACE(symspace))
      return TRUE;  /* fetched via accumulator and dptr */
  }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* canDefAccResult - return 1 if the iCode can generate a result   */
/*                   in A or XA                                    */
/*-----------------------------------------------------------------*/
static int
canDefAccResult (iCode * ic)
{
  int size;

  if (ic->op == IFX || ic->op == JUMPTABLE)     /* these iCodes have no result */
    return 0;

  if (POINTER_SET (ic))
    return 0;

  if (!IC_RESULT (ic))
    return 0;

  if (!IS_ITEMP (IC_RESULT (ic)))
    return 0;

  /* I don't think an iTemp can be an aggregate, but just in case */
  if (IS_AGGREGATE(operandType(IC_RESULT(ic))))
    return 0;

  size = getSize (operandType (IC_RESULT (ic)));

  if (size == 1)
    {
      /* All 1 byte operations should safely generate an accumulator result */
      return 1;
    }
  else if (size == 2)
    {
      switch (ic->op)
        {
        case LEFT_OP:
        case RIGHT_OP:
          return isOperandLiteral (IC_RIGHT (ic))
                  && SPEC_USIGN (operandType (IC_RESULT (ic)));
        case CALL:
        case PCALL:
        case '*':
        case RECEIVE:
        case '=': /* assignment, since POINTER_SET is already ruled out */
          return 1;

        default:
          return 0;
        }
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* canUseAccOperand - return 1 if the iCode can use the operand    */
/*                    when passed in A or XA                       */
/*-----------------------------------------------------------------*/
static int
canUseAccOperand (iCode * ic, operand * op)
{
  int size;

  if (ic->op == IFX)
    {
      if (isOperandEqual (op, IC_COND (ic)))
        return 1;
      else
        return 0;
    }

  if (ic->op == JUMPTABLE)
    {
      if (isOperandEqual (op, IC_JTCOND (ic)))
        return 1;
      else
        return 0;
    }

  if (POINTER_SET (ic) && isOperandEqual (op, IC_RESULT (ic)))
    return 1;

  if (!isOperandEqual (op, IC_LEFT (ic)) && !isOperandEqual (op, IC_RIGHT (ic)))
    return 0;

  /* Generation of SEND is deferred until CALL; not safe */
  /* if there are intermediate iCodes */
  if (ic->op == SEND && ic->next && ic->next->op != CALL)
    return 0;

  size = getSize (operandType (op));
  if (size == 1)
    {
      /* All 1 byte operations should safely use an accumulator operand */
      return 1;
    }
  else if (size == 2)
    {
      switch (ic->op)
        {
        case LEFT_OP:
        case RIGHT_OP:
          return isOperandLiteral (IC_RIGHT (ic));
        case SEND:
          return 1;
        default:
          return 0;
        }
    }

  return 0;
}


/*-----------------------------------------------------------------*/
/* packRegsForAccUse - pack registers for acc use                  */
/*-----------------------------------------------------------------*/
static int
packRegsForAccUse (iCode * ic)
{
  iCode * uic;
  operand * op;

  if (!canDefAccResult (ic))
    return 0;

  op = IC_RESULT (ic);

  /* has only one definition */
  if (bitVectnBitsOn (OP_DEFS (op)) > 1)
    return 0;

  /* has only one use */
  if (bitVectnBitsOn (OP_USES (op)) > 1)
    return 0;

  uic = ic->next;
  if (!uic)
    return 0;

  if (!canUseAccOperand (uic, op))
    return 0;

  #if 0
  if ((POINTER_GET(uic))
      || (ic->op == ADDRESS_OF && uic->op == '+' && IS_OP_LITERAL (IC_RIGHT (uic))))
    {
      OP_SYMBOL (IC_RESULT (ic))->accuse = ACCUSE_HX;
      return;
    }
  #endif

  OP_SYMBOL (IC_RESULT (ic))->accuse = ACCUSE_XA;
  return 1;
}

/*-----------------------------------------------------------------*/
/* packForPush - heuristics to reduce iCode for pushing            */
/*-----------------------------------------------------------------*/
static void
packForPush (iCode * ic, eBBlock ** ebpp, int count)
{
  iCode *dic, *lic;
  bitVect *dbv;
  int disallowHiddenAssignment = 0;
  eBBlock * ebp = ebpp[ic->eBBlockNum];

  if ((ic->op != IPUSH && ic->op != SEND) || !IS_ITEMP (IC_LEFT (ic)))
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

  if (dic->seq < ebp->fSeq || dic->seq > ebp->lSeq) // Evelyn did this
    {
      int i;
      for (i=0; i<count; i++)
        {
          if (dic->seq >= ebpp[i]->fSeq && dic->seq <= ebpp[i]->lSeq)
            {
              ebp=ebpp[i];
              break;
            }
        }
      if (i==count) // Abort if we can't find the definition's block
        return;
    }

  if (IS_SYMOP(IC_RIGHT(dic)))
    {
      if (IC_RIGHT (dic)->isvolatile)
        return;

      if (OP_SYMBOL (IC_RIGHT (dic))->addrtaken || isOperandGlobal (IC_RIGHT (dic)))
        disallowHiddenAssignment = 1;

      /* make sure the right side does not have any definitions
         inbetween */
      dbv = OP_DEFS(IC_RIGHT(dic));
      for (lic = ic; lic && lic != dic ; lic = lic->prev)
        {
          if (bitVectBitValue(dbv,lic->key))
            return ;
          if (disallowHiddenAssignment && (lic->op == CALL || lic->op == PCALL || POINTER_SET (lic)))
            return;
        }
      /* make sure they have the same type */
      if (IS_SPEC(operandType(IC_LEFT(ic))))
        {
          sym_link *itype=operandType(IC_LEFT(ic));
          sym_link *ditype=operandType(IC_RIGHT(dic));

          if (SPEC_USIGN(itype)!=SPEC_USIGN(ditype) ||
              SPEC_LONG(itype)!=SPEC_LONG(ditype))
            return;
        }
      /* extend the live range of replaced operand if needed */
      if (OP_SYMBOL(IC_RIGHT(dic))->liveTo < ic->seq)
        {
          OP_SYMBOL(IC_RIGHT(dic))->liveTo = ic->seq;
        }
      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
    }
  if (IS_ITEMP (IC_RIGHT (dic)))
    OP_USES (IC_RIGHT (dic)) = bitVectSetBit (OP_USES (IC_RIGHT (dic)), ic->key);

  /* we now we know that it has one & only one def & use
     and the that the definition is an assignment */
  ReplaceOpWithCheaperOp(&IC_LEFT (ic), IC_RIGHT (dic));
  remiCodeFromeBBlock (ebp, dic);
  hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
}

/*------------------------------------------------------------------*/
/* moveSendToCall - move SEND to immediately precede its CALL/PCALL */
/*------------------------------------------------------------------*/
static iCode *
moveSendToCall (iCode *sic, eBBlock *ebp)
{
  iCode * prev = sic->prev;
  iCode * sic2 = NULL;
  iCode * cic;
  
  /* Go find the CALL/PCALL */
  cic = sic;
  while (cic && cic->op != CALL && cic->op != PCALL)
    cic = cic->next;
  if (!cic)
    return sic;

  /* Is there a second SEND? If so, we'll need to move it too. */
  if (sic->next->op == SEND)
    sic2 = sic->next;
  
  /* relocate the SEND(s) */
  remiCodeFromeBBlock (ebp, sic);
  addiCodeToeBBlock (ebp, sic, cic);
  if (sic2)
    {
      remiCodeFromeBBlock (ebp, sic2);
      addiCodeToeBBlock (ebp, sic2, cic);
    }

  /* Return the iCode to continue processing at. */
  if (prev)
    return prev->next;
  else
    return ebp->sch;
}


/*---------------------------------------------------------------------*/
/* packPointerOp - see if we can move an offset from addition iCode    */
/*                 to the pointer iCode to used indexed addr mode      */
/* The z80-related ports do a similar thing in SDCCopt.c, offsetFold() */
/*---------------------------------------------------------------------*/
static void
packPointerOp (iCode * ic, eBBlock ** ebpp)
{
  operand * pointer;
  operand * offsetOp;
  operand * nonOffsetOp;
  iCode * dic;
  iCode * uic;
  int key;

  if (POINTER_SET (ic))
    {
      pointer = IC_RESULT (ic);
      offsetOp = IC_LEFT (ic);
    }
  else if (POINTER_GET (ic))
    {
      pointer = IC_LEFT (ic);
      offsetOp = IC_RIGHT (ic);
    }
  else
    return;

  if (!IS_ITEMP (pointer))
    return;

  /* If the pointer is rematerializable, it's already fully optimized */
  if (OP_SYMBOL (pointer)->remat)
    return;

  if (offsetOp && IS_OP_LITERAL (offsetOp) && operandLitValue (offsetOp) != 0)
    return;
  if (offsetOp && IS_SYMOP (offsetOp))
    return;

  /* There must be only one definition */
  if (bitVectnBitsOn (OP_DEFS (pointer)) != 1)
    return;
  /* find the definition */
  if (!(dic = hTabItemWithKey (iCodehTab, bitVectFirstBit (OP_DEFS (pointer)))))
    return;

  if (dic->op == '+' && (IS_OP_LITERAL (IC_RIGHT (dic)) ||
                        (IS_ITEMP (IC_RIGHT (dic)) && OP_SYMBOL (IC_RIGHT (dic))->remat)))
    {
      nonOffsetOp = IC_LEFT (dic);
      offsetOp = IC_RIGHT (dic);
    }
  else if (dic->op == '+' && IS_ITEMP (IC_LEFT (dic)) && OP_SYMBOL (IC_LEFT (dic))->remat)
    {
      nonOffsetOp = IC_RIGHT (dic);
      offsetOp = IC_LEFT (dic);
    }
  else
    return;


  /* Now check all of the uses to make sure they are all get/set pointer */
  /* and don't already have a non-zero offset operand */
  for (key=0; key<OP_USES (pointer)->size; key++)
    {
      if (bitVectBitValue (OP_USES (pointer), key))
        {
          uic = hTabItemWithKey (iCodehTab, key);
          if (POINTER_GET (uic))
            {
              if (IC_RIGHT (uic) && IS_OP_LITERAL (IC_RIGHT (uic)) && operandLitValue (IC_RIGHT (uic)) != 0)
                return;
              if (IC_RIGHT (uic) && IS_SYMOP (IC_RIGHT (uic)))
                return;
            }
          else if (POINTER_SET (uic))
            {
              if (IC_LEFT (uic) && IS_OP_LITERAL (IC_LEFT (uic)) && operandLitValue (IC_LEFT (uic)) != 0)
                return;
              if (IC_LEFT (uic) && IS_SYMOP (IC_LEFT (uic)))
                return;
            }
          else
            return;
        }
    }

  /* Everything checks out. Move the literal or rematerializable offset */
  /* to the pointer get/set iCodes */
  for (key=0; key<OP_USES (pointer)->size; key++)
    {
      if (bitVectBitValue (OP_USES (pointer), key))
        {
          uic = hTabItemWithKey (iCodehTab, key);
          if (POINTER_GET (uic))
            {
              IC_RIGHT (uic) = offsetOp;
              if (IS_SYMOP (offsetOp))
                OP_USES (offsetOp) = bitVectSetBit (OP_USES (offsetOp), key);
            }
          else if (POINTER_SET (uic))
            {
              IC_LEFT (uic) = offsetOp;
              if (IS_SYMOP (offsetOp))
                OP_USES (offsetOp) = bitVectSetBit (OP_USES (offsetOp), key);
            }
          else
            return;
        }
    }

  /* Put the remaining operand on the right and convert to assignment     */
  if (IS_SYMOP (offsetOp))
    bitVectUnSetBit (OP_USES (offsetOp), dic->key);
  IC_RIGHT (dic) = nonOffsetOp;
  IC_LEFT (dic) = NULL;
  SET_ISADDR (IC_RESULT (dic), 0);
  dic->op = '=';
}

/*-----------------------------------------------------------------*/
/* packRegisters - does some transformations to reduce register    */
/*                   pressure                                      */
/*-----------------------------------------------------------------*/
static void
packRegisters (eBBlock ** ebpp, int count)
{
  iCode *ic;
  int change = 0;
  int blockno;

  for (blockno=0; blockno<count; blockno++)
    {
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
          //packRegsForLiteral (ic);
      
          /* move SEND to immediately precede its CALL/PCALL */
          if (ic->op == SEND && ic->next &&
              ic->next->op != CALL && ic->next->op != PCALL)
            {
              ic = moveSendToCall (ic, ebp);
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
              bitVectnBitsOn (OP_SYMBOL (IC_RESULT (ic))->defs) <= 1 &&
              !OP_SYMBOL (IC_RESULT (ic))->_isparm &&
              !OP_SYMBOL (IC_RESULT (ic))->addrtaken &&
              !isOperandGlobal (IC_RESULT (ic)))
            {
              OP_SYMBOL (IC_RESULT (ic))->remat = OP_SYMBOL (IC_RIGHT (ic))->remat;
              OP_SYMBOL (IC_RESULT (ic))->rematiCode = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
            }

          /* if cast to a generic pointer & the pointer being
             cast is remat, then we can remat this cast as well */
          if (ic->op == CAST &&
              IS_SYMOP(IC_RIGHT(ic)) &&
              OP_SYMBOL(IC_RIGHT(ic))->remat &&
              bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 &&
              !OP_SYMBOL (IC_RESULT (ic))->_isparm &&
              !OP_SYMBOL (IC_RESULT (ic))->addrtaken &&
              !isOperandGlobal (IC_RESULT (ic)))
            {
              sym_link *to_type = operandType(IC_LEFT(ic));
              sym_link *from_type = operandType(IC_RIGHT(ic));
              if (IS_PTR(to_type) && IS_PTR(from_type))
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
               IS_OP_LITERAL (IC_RIGHT (ic))) &&
               OP_SYMBOL (IC_LEFT (ic))->remat &&
              (!IS_SYMOP (IC_RIGHT (ic)) || !IS_CAST_ICODE(OP_SYMBOL (IC_RIGHT (ic))->rematiCode)) &&
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

          /* reduce for support function calls */
          if (ic->supportRtn || (ic->op != IFX && ic->op != JUMPTABLE))
            packRegsForSupport (ic, ebp);

          /* if the condition of an if instruction
             is defined in the previous instruction and
             this is the only usage then
             mark the itemp as a conditional */
          if ((IS_CONDITIONAL (ic) ||
               (IS_BITWISE_OP(ic) && isBitwiseOptimizable (ic))) &&
              ic->next && ic->next->op == IFX &&
              bitVectnBitsOn (OP_USES(IC_RESULT(ic)))==1 &&
              isOperandEqual (IC_RESULT (ic), IC_COND (ic->next)) &&
              OP_SYMBOL (IC_RESULT (ic))->liveTo <= ic->next->seq)
            {
              OP_SYMBOL (IC_RESULT (ic))->regType = REG_CND;
              continue;
            }

          /* pack for PUSH
             iTempNN := (some variable in farspace) V1
             push iTempNN ;
             -------------
             push V1
           */
          if (ic->op == IPUSH || ic->op == SEND)
            {
              packForPush (ic, ebpp, count);
            }

          if (POINTER_SET (ic) || POINTER_GET (ic))
            packPointerOp (ic, ebpp);

          if (options.oldralloc)
            packRegsForAccUse (ic);
        }
    }
}

static void
RegFix (eBBlock ** ebbs, int count)
{
  int i;

  /* Check for and fix any problems with uninitialized operands */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath && (ebbs[i]->entryLabel != entryLabel && ebbs[i]->entryLabel != returnLabel))
        continue;

      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          deassignLRs (ic, ebbs[i]);

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

static void
replaceAccuseOperand (operand * op)
{
  symbol * sym;

  if (!IS_ITEMP (op))
    return;

  sym = OP_SYMBOL (op);
  if (sym->remat || !sym->accuse)
    return;

  sym->nRegs = getSize (operandType (op));
  wassert ((sym->accuse == ACCUSE_XA) || (sym->accuse == ACCUSE_HX));
  wassert (sym->nRegs <= 2);

  if (sym->accuse == ACCUSE_XA)
    {
      sym->regs[0] = hc08_reg_a;
      if (sym->nRegs > 1)
        sym->regs[1] = hc08_reg_x;
    }
  else /* must be sym->accuse == ACCUSE_HX */
    {
      sym->regs[0] = hc08_reg_x;
      if (sym->nRegs > 1)
        sym->regs[1] = hc08_reg_h;
    }  
  sym->accuse = 0;
}


/* Replace all uses of the sym->accuse flag with initialized */
/* sym->regs[] and sym->nRegs so the current code generator  */
/* can work with the old register allocator. */
static void
replaceAccuse (eBBlock ** ebbs, int count)
{
  int i;

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
              replaceAccuseOperand (IC_COND (ic));
              continue;
            }

          if (ic->op == JUMPTABLE)
            {
              replaceAccuseOperand (IC_JTCOND (ic));
              continue;
            }

          replaceAccuseOperand (IC_RESULT (ic));
          replaceAccuseOperand (IC_LEFT (ic));
          replaceAccuseOperand (IC_RIGHT (ic));
        }
    }
}

#ifdef OLDRALLOC
/*-----------------------------------------------------------------*/
/* Old, obsolete register allocator                                */
/*-----------------------------------------------------------------*/
void
hc08_oldralloc (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  iCode *ic;

  setToNull ((void *) &_G.funcrUsed);
  setToNull ((void *) &_G.regAssigned);
  setToNull ((void *) &_G.totRegAssigned);
  hc08_ptrRegReq = _G.stackExtend = _G.dataExtend = 0;
  hc08_nRegs = 7;
  hc08_reg_a = hc08_regWithIdx(A_IDX);
  hc08_reg_x = hc08_regWithIdx(X_IDX);
  hc08_reg_h = hc08_regWithIdx(H_IDX);
  hc08_reg_hx = hc08_regWithIdx(HX_IDX);
  hc08_reg_xa = hc08_regWithIdx(XA_IDX);
  hc08_reg_sp = hc08_regWithIdx(SP_IDX);
  hc08_nRegs = 5;

  /* change assignments this will remove some
     live ranges reducing some register pressure */

  packRegisters (ebbs, count);

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
#if 0
  fillGaps();
#endif

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

  /* Convert the old sym->accuse flag into normal register assignments */
  replaceAccuse (ebbs, count);

  /* redo that offsets for stacked automatic variables */
  if (currFunc)
    {
      redoStackOffsets ();
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

  genhc08Code (ic);

  /* free up any _G.stackSpil locations allocated */
  applyToSet (_G.stackSpil, deallocStackSpil);
  _G.slocNum = 0;
  setToNull ((void *) &_G.stackSpil);
  setToNull ((void *) &_G.spiltSet);
  /* mark all registers as free */
  freeAllRegs ();

  return;
}
#endif

/** Serially allocate registers to the variables.
    This was the main register allocation function.  It is called after
    packing.
    In the new register allocator it only serves to mark variables for the new register allocator.
 */
static void
serialRegMark (eBBlock ** ebbs, int count)
{
  int i;
  short int max_alloc_bytes = SHRT_MAX; // Byte limit. Set this to a low value to pass only few variables to the register allocator. This can be useful for debugging.

  /* for all blocks */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath &&
          (ebbs[i]->entryLabel != entryLabel &&
           ebbs[i]->entryLabel != returnLabel))
        continue;

      /* for all instructions do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {
          updateRegUsage(ic);

          /* if this is an ipop that means some live
             range will have to be assigned again */
          if (ic->op == IPOP)
              reassignLR (IC_LEFT (ic));

          /* if result is present && is a true symbol */
          if (IC_RESULT (ic) && ic->op != IFX &&
              IS_TRUE_SYMOP (IC_RESULT (ic)))
            {
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
            {
              continue;
            }

          /* now we need to allocate registers only for the result */
          if (IC_RESULT (ic))
            {
              symbol *sym = OP_SYMBOL (IC_RESULT (ic));

              /* Make sure any spill location is definitely allocated */
              if (sym->isspilt && !sym->remat && sym->usl.spillLoc &&
                  !sym->usl.spillLoc->allocreq)
                {
                  sym->usl.spillLoc->allocreq++;
                }

              /* if it does not need or is spilt
                 or is already assigned to registers
                 or will not live beyond this instructions */
              if (!sym->nRegs ||
                  sym->isspilt ||
                  bitVectBitValue (_G.regAssigned, sym->key) ||
                  sym->liveTo <= ic->seq)
                {
                  continue;
                }

              /* if some liverange has been spilt at the block level
                 and this one live beyond this block then spil this
                 to be safe */
              if (_G.blockSpil && sym->liveTo > ebbs[i]->lSeq)
                {
                  spillThis (sym);
                  continue;
                }

              if (sym->remat)
                {
                  spillThis (sym);
                  continue;
                }

              if (max_alloc_bytes >= sym->nRegs)
                {
                  sym->for_newralloc = 1;
                  max_alloc_bytes -= sym->nRegs;
                }
              else if (!sym->for_newralloc)
                {
                  spillThis (sym);
                  printf ("Spilt %s due to byte limit.\n", sym->name);
                }
            }
        }
    }
}
/*-----------------------------------------------------------------*/
/* New register allocator                                          */
/*-----------------------------------------------------------------*/
void
hc08_ralloc (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  iCode *ic;

  setToNull ((void *) &_G.funcrUsed);
  setToNull ((void *) &_G.regAssigned);
  setToNull ((void *) &_G.totRegAssigned);
  hc08_ptrRegReq = _G.stackExtend = _G.dataExtend = 0;
  hc08_nRegs = 7;
  hc08_reg_a = hc08_regWithIdx(A_IDX);
  hc08_reg_x = hc08_regWithIdx(X_IDX);
  hc08_reg_h = hc08_regWithIdx(H_IDX);
  hc08_reg_hx = hc08_regWithIdx(HX_IDX);
  hc08_reg_xa = hc08_regWithIdx(XA_IDX);
  hc08_reg_sp = hc08_regWithIdx(SP_IDX);
  hc08_nRegs = 5;

  /* change assignments this will remove some
     live ranges reducing some register pressure */

  packRegisters (ebbs, count);

  /* liveranges probably changed by register packing
     so we compute them again */
  recomputeLiveRanges (ebbs, count, FALSE);

  if (options.dump_i_code)
    dumpEbbsToFileExt (DUMP_PACK, ebbi);

  /* first determine for each live range the number of
     registers & the type of registers required for each */
  regTypeNum (*ebbs);

  /* and serially allocate registers */
  serialRegMark (ebbs, count);

  /* The new register allocator invokes its magic */
  ic = hc08_ralloc2_cc (ebbi);

  RegFix (ebbs, count);

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

  /* redo that offsets for stacked automatic variables */
  if (currFunc)
    {
      redoStackOffsets ();
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

  genhc08Code (ic);

  /* free up any _G.stackSpil locations allocated */
  applyToSet (_G.stackSpil, deallocStackSpil);
  _G.slocNum = 0;
  setToNull ((void *) &_G.stackSpil);
  setToNull ((void *) &_G.spiltSet);
  /* mark all registers as free */
  freeAllRegs ();

  return;
}

/*-----------------------------------------------------------------*/
/* assignRegisters - assigns registers to each live range as need  */
/*-----------------------------------------------------------------*/
void
hc08_assignRegisters (ebbIndex * ebbi)
{
#ifdef OLDRALLOC
  if (options.oldralloc)
    hc08_oldralloc (ebbi);
  else
#endif
    hc08_ralloc (ebbi);
}

