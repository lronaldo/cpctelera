/*-------------------------------------------------------------------------
  gen.c - code generator for Padauk.

  Copyright (C) 2018, Philipp Klaus Krause pkk@spth.de

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
-------------------------------------------------------------------------*/

#include "ralloc.h"
#include "gen.h"

/* Use the D macro for basic (unobtrusive) debugging messages */
#define D(x) do if (options.verboseAsm) { x; } while (0)

static bool regalloc_dry_run;
static unsigned int regalloc_dry_run_cost_words;
static float regalloc_dry_run_cost_cycles;
static unsigned int regalloc_dry_run_cycle_scale = 1;

static struct
{
  short debugLine;
  struct
    {
      int pushed;
      int size;
      int param_offset;
    } stack;
  bool saved;

  /* Track content of p */
  struct
   {
     AOP_TYPE type;
     const char *base;
     int offset;
   } p;
}
G;

static struct asmop asmop_a, asmop_p, asmop_pa, asmop_ap, asmop_zero, asmop_one, asmop_sp;
static struct asmop *const ASMOP_A = &asmop_a;
static struct asmop *const ASMOP_P = &asmop_p;
static struct asmop *const ASMOP_PA = &asmop_pa;
static struct asmop *const ASMOP_AP = &asmop_ap;
static struct asmop *const ASMOP_ZERO = &asmop_zero;
static struct asmop *const ASMOP_ONE = &asmop_one;
static struct asmop *const ASMOP_SP = &asmop_sp;

void
pdk_init_asmops (void)
{
  asmop_a.type = AOP_REG;
  asmop_a.size = 1;
  asmop_a.aopu.bytes[0].in_reg = true;
  asmop_a.aopu.bytes[0].byteu.reg = pdk_regs + A_IDX;

  asmop_p.type = AOP_REG;
  asmop_p.size = 1;
  asmop_p.aopu.bytes[0].in_reg = true;
  asmop_p.aopu.bytes[0].byteu.reg = pdk_regs + P_IDX;

  asmop_ap.type = AOP_REG;
  asmop_ap.size = 2;
  asmop_ap.aopu.bytes[0].in_reg = true;
  asmop_ap.aopu.bytes[0].byteu.reg = pdk_regs + A_IDX;
  asmop_ap.aopu.bytes[1].in_reg = true;
  asmop_ap.aopu.bytes[1].byteu.reg = pdk_regs + P_IDX;

  asmop_pa.type = AOP_REG;
  asmop_pa.size = 2;
  asmop_pa.aopu.bytes[0].in_reg = true;
  asmop_pa.aopu.bytes[0].byteu.reg = pdk_regs + P_IDX;
  asmop_pa.aopu.bytes[1].in_reg = true;
  asmop_pa.aopu.bytes[1].byteu.reg = pdk_regs + A_IDX;

  asmop_zero.type = AOP_LIT;
  asmop_zero.size = 1;
  asmop_zero.aopu.aop_lit = constVal ("0");

  asmop_one.type = AOP_LIT;
  asmop_one.size = 1;
  asmop_one.aopu.aop_lit = constVal ("1");

  asmop_sp.type = AOP_SFR;
  asmop_sp.aopu.aop_dir = "sp";
  asmop_sp.size = 1;
}

static void
emit2 (const char *inst, const char *fmt, ...)
{
  if (!regalloc_dry_run)
    {
      va_list ap;

      va_start (ap, fmt);
      va_emitcode (inst, fmt, ap);
      va_end (ap);
    }
}

static void
cost(unsigned int words, float cycles)
{
  regalloc_dry_run_cost_words += words;
  regalloc_dry_run_cost_cycles += cycles * regalloc_dry_run_cycle_scale;
}

static void
emitJP(const symbol *target, float probability)
{
  if (!regalloc_dry_run)
    emit2 ("goto", "%05d$", labelKey2num (target->key));
  cost (1, 2 * probability);
}

static bool
regDead (int idx, const iCode *ic)
{
  wassert (idx == A_IDX || idx == P_IDX);

  return (!bitVectBitValue (ic->rSurv, idx));
}

/*-----------------------------------------------------------------*/
/* aopInReg - asmop from offset in the register                    */
/*-----------------------------------------------------------------*/
static bool
aopInReg (const asmop *aop, int offset, short rIdx)
{
  if (aop->type != AOP_REG)
    return (false);

  return (aop->aopu.bytes[offset].in_reg && aop->aopu.bytes[offset].byteu.reg->rIdx == rIdx);
}

/*-----------------------------------------------------------------*/
/* aopSame - are two asmops in the same location?                  */
/*-----------------------------------------------------------------*/
static bool
aopSame (const asmop *aop1, int offset1, const asmop *aop2, int offset2, int size)
{
  for(; size; size--, offset1++, offset2++)
    {
      if (aop1->type == AOP_REG && aop2->type == AOP_REG && // Same register
        aop1->aopu.bytes[offset1].in_reg && aop2->aopu.bytes[offset2].in_reg &&
        aop1->aopu.bytes[offset1].byteu.reg == aop2->aopu.bytes[offset2].byteu.reg)
        continue;

      if (aop1->type == AOP_LIT && aop2->type == AOP_LIT &&
        byteOfVal (aop1->aopu.aop_lit, offset1) == byteOfVal (aop2->aopu.aop_lit, offset2))
        continue;

      if (aop1->type == AOP_DIR && aop2->type == AOP_DIR && aop1->aopu.immd_off + offset1 == aop2->aopu.immd_off + offset2 &&
        !strcmp(aop1->aopu.aop_dir, aop2->aopu.aop_dir))
        return (true);

      if (aop1->type == AOP_STK && aop2->type == AOP_STK && aop1->aopu.bytes[offset1].byteu.stk == aop2->aopu.bytes[offset2].byteu.stk)
        return (true);

      if (aop1->type == AOP_SFR && aop2->type == AOP_SFR && offset1 == offset2 &&
        aop1->aopu.aop_dir && aop2->aopu.aop_dir && !strcmp(aop1->aopu.aop_dir, aop2->aopu.aop_dir))
        return (true);

      return (false);
    }

  return (true);
}

/*-----------------------------------------------------------------*/
/* aopIsLitVal - asmop from offset is val                          */
/*-----------------------------------------------------------------*/
static bool
aopIsLitVal (const asmop *aop, int offset, int size, unsigned long long int val)
{
  wassert_bt (size <= sizeof (unsigned long long int)); // Make sure we are not testing outside of argument val.

  for(; size; size--, offset++)
    {
      unsigned char b = val & 0xff;
      val >>= 8;

      // Leading zeroes
      if ((aop->size <= offset || aop->type == AOP_STL && offset) && !b)
        continue;

      if (aop->type == AOP_IMMD && offset > (aop->aopu.code ? 1 : 0) && !b)
        continue;

      if (aop->size <= offset)
        return (false);

      if (aop->type != AOP_LIT)
        return (false);

      if (byteOfVal (aop->aopu.aop_lit, offset) != b)
        return (false);
    }

  return (true);
}

static const char *
aopGet(const asmop *aop, int offset)
{
  static char buffer[256];

  if (offset >= aop->size || aop->type == AOP_STL && offset)
    return ("#0x00");

  if (aop->type == AOP_LIT)
    {
      SNPRINTF (buffer, sizeof(buffer), "#0x%02x", byteOfVal (aop->aopu.aop_lit, offset));
      return (buffer);
    }

  if (aop->type == AOP_REG)
    return (aop->aopu.bytes[offset].byteu.reg->name);

  if (aop->type == AOP_IMMD)
    {
      if (offset == 0 && aop->aopu.code)
        SNPRINTF (buffer, sizeof(buffer), "#<(%s + %d)", aop->aopu.immd, aop->aopu.immd_off);
      else if (offset == 1 && aop->aopu.func)
        SNPRINTF (buffer, sizeof(buffer), "#>(%s + %d)", aop->aopu.immd, aop->aopu.immd_off);
      else if (offset == 1 && aop->aopu.code)
        SNPRINTF (buffer, sizeof(buffer), "#>(%s + 0x8000 + %d)", aop->aopu.immd, aop->aopu.immd_off);
      else if (offset == 0)
        SNPRINTF (buffer, sizeof(buffer), "#(%s + %d)", aop->aopu.immd, aop->aopu.immd_off);
      else
        SNPRINTF (buffer, sizeof(buffer), "#0", aop->aopu.immd, aop->aopu.immd_off);
      return (buffer);
    }

  if (aop->type == AOP_DIR)
    {
      SNPRINTF (buffer, sizeof(buffer), "%s+%d", aop->aopu.aop_dir, offset);
      return (buffer);
    }
  else if (aop->type == AOP_SFR)
    {
      wassert (aop->size == 1);
      SNPRINTF (buffer, sizeof(buffer), "%s", aop->aopu.aop_dir);
      return (buffer);
    }

  wassert_bt (0);
  return ("dummy");
}

/*-----------------------------------------------------------------*/
/* newAsmop - creates a new asmOp                                  */
/*-----------------------------------------------------------------*/
static asmop *
newAsmop (short type)
{
  asmop *aop;

  aop = Safe_calloc (1, sizeof (asmop));
  aop->type = type;

  return (aop);
}

/*-----------------------------------------------------------------*/
/* freeAsmop - free up the asmop given to an operand               */
/*----------------------------------------------------------------*/
static void
freeAsmop (operand *op)
{
  asmop *aop;

  wassert_bt (op);

  aop = op->aop;

  if (!aop)
    return;

  Safe_free (aop);

  op->aop = 0;
  if (IS_SYMOP (op) && SPIL_LOC (op))
    SPIL_LOC (op)->aop = 0;
}

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (const iCode *ic, symbol *sym)
{
  asmop *aop;

  wassert_bt (ic);
  wassert_bt (regalloc_dry_run || sym);
  wassert_bt (regalloc_dry_run || sym->etype);

  // Unlike some other backends we really free asmops; to avoid a double-free, we need to support multiple asmops for the same symbol.

  if (sym && IS_FUNC (sym->type))
    {
      aop = newAsmop (AOP_IMMD);
      aop->aopu.immd = sym->rname;
      aop->aopu.immd_off = 0;
      aop->aopu.code = IN_CODESPACE (SPEC_OCLS (sym->etype));
      aop->aopu.func = true;
      aop->size = getSize (sym->type);
    }
  /* Assign depending on the storage class */
  else if (sym && sym->onStack || sym && sym->iaccess)
    {
      aop = newAsmop (AOP_STK);
      aop->size = getSize (sym->type);
      int base = sym->stack + (sym->stack < 0 ? G.stack.param_offset : 0);
      for (int offset = 0; offset < aop->size; offset++)
        aop->aopu.bytes[offset].byteu.stk = base + offset;
    }
  /* sfr */
  else if (sym && IN_REGSP (SPEC_OCLS (sym->etype)))
    {
      wassertl (getSize (sym->type) <= 2, "Unimplemented support for wide (> 16 bit) I/O register");

      aop = newAsmop (AOP_SFR);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
    }
  else
    {
      aop = newAsmop (sym && IN_CODESPACE (SPEC_OCLS (sym->etype)) ? AOP_CODE : AOP_DIR);
      if (sym)
        {
          aop->aopu.aop_dir = sym->rname;
          aop->size = getSize (sym->type);
        }
    }

  return (aop);
}

/*-----------------------------------------------------------------*/
/* aopForRemat - rematerializes an object                          */
/*-----------------------------------------------------------------*/
static asmop *
aopForRemat (symbol *sym)
{
  iCode *ic = sym->rematiCode;
  asmop *aop;
  int val = 0;

  wassert_bt (ic);

  for (;;)
    {
      if (ic->op == '+')
        {
          if (isOperandLiteral (IC_RIGHT (ic)))
            {
              val += (int) operandLitValue (IC_RIGHT (ic));
              ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
            }
          else
            {
              val += (int) operandLitValue (IC_LEFT (ic));
              ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
            }
        }
      else if (ic->op == '-')
        {
          val -= (int) operandLitValue (IC_RIGHT (ic));
          ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
        }
      else if (ic->op == CAST)
        {
          ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
        }
      else if (ic->op == ADDRESS_OF)
        {
          val += (int) operandLitValue (IC_RIGHT (ic));
          break;
        }
      else
        wassert_bt (0);
    }

  if (OP_SYMBOL (IC_LEFT (ic))->onStack)
    {
      aop = newAsmop (AOP_STL);
      aop->aopu.stk_off = OP_SYMBOL (IC_LEFT (ic))->stack + (OP_SYMBOL (IC_LEFT (ic))->stack < 0 ? G.stack.param_offset : 0) + val;
    }
  else
    {
      aop = newAsmop (AOP_IMMD);
      aop->aopu.immd = OP_SYMBOL (IC_LEFT (ic))->rname;
      aop->aopu.immd_off = val;
      aop->aopu.code = IN_CODESPACE (SPEC_OCLS (OP_SYMBOL (IC_LEFT (ic))->etype));
    }

  aop->size = getSize (sym->type);

  return aop;
}

/*-----------------------------------------------------------------*/
/* aopOp - allocates an asmop for an operand  :                    */
/*-----------------------------------------------------------------*/
static void
aopOp (operand *op, const iCode *ic)
{
  wassert_bt (op);

  /* if already has an asmop */
  if (op->aop)
    return;

  /* if this a literal */
  if (IS_OP_LITERAL (op))
    {
      asmop *aop = newAsmop (AOP_LIT);
      aop->aopu.aop_lit = OP_VALUE (op);
      aop->size = getSize (operandType (op));
      op->aop = aop;
      return;
    }

  symbol *sym = OP_SYMBOL (op);

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aopForSym (ic, sym);
      return;
    }

  /* Rematerialize symbols where all bytes are spilt. */
  if (sym->remat && (sym->isspilt || regalloc_dry_run))
    {
      bool completely_spilt = TRUE;
      for (int i = 0; i < getSize (sym->type); i++)
        if (sym->regs[i])
          completely_spilt = FALSE;
      if (completely_spilt)
        {
          op->aop = aopForRemat (sym);
          return;
        }
    }

  /* if the type is a conditional */
  if (sym->regType == REG_CND)
    {
      asmop *aop = newAsmop (AOP_CND);
      op->aop = aop;
      sym->aop = sym->aop;
      return;
    }

  /* None of the above, which only leaves temporaries. */
  if ((sym->isspilt || sym->nRegs == 0) && !(regalloc_dry_run && (options.stackAuto || reentrant)))
    {
      sym->aop = op->aop = aopForSym (ic, sym->usl.spillLoc);
      op->aop->size = getSize (sym->type);
      return;
    }

  /* None of the above, which only leaves temporaries. */
  { 
    bool completely_in_regs = true;
    bool completely_spilt = true;
    asmop *aop = newAsmop (AOP_REGDIR);

    aop->size = getSize (operandType (op));
    op->aop = aop;

    for (int i = 0; i < aop->size; i++)
      {
        aop->aopu.bytes[i].in_reg = !!sym->regs[i];
        if (sym->regs[i])
          {
            completely_spilt = false;
            aop->aopu.bytes[i].byteu.reg = sym->regs[i];
            //aop->regs[sym->regs[i]->rIdx] = i;
          }
        else if (sym->isspilt && sym->usl.spillLoc || sym->nRegs && regalloc_dry_run)
          {
            completely_in_regs = false;

            if (!regalloc_dry_run)
              {
                /*aop->aopu.bytes[i].byteu.stk = (long int)(sym->usl.spillLoc->stack) + aop->size - i;

                if (sym->usl.spillLoc->stack + aop->size - (int)(i) <= -G.stack.pushed)
                  {
                    fprintf (stderr, "%s %d %d %d %d at ic %d\n", sym->name, (int)(sym->usl.spillLoc->stack), (int)(aop->size), (int)(i), (int)(G.stack.pushed), ic->key);
                    wassertl_bt (0, "Invalid stack offset.");
                  }*/
              }
            else
              {
                static long int old_base = -10;
                static const symbol *old_sym = 0;
                if (sym != old_sym)
                  {
                    old_base -= aop->size;
                    if (old_base < -100)
                      old_base = -10;
                    old_sym = sym;
                  }

                // aop->aopu.bytes[i].byteu.stk = old_base + aop->size - i;
              }
          }
        else // Dummy iTemp.
          {
            aop->type = AOP_DUMMY;
            return;
          }

        if (!completely_in_regs && (!currFunc || GcurMemmap == statsg))
          {
            if (!regalloc_dry_run)
              wassertl_bt (0, "Stack asmop outside of function.");
            cost (180, 180);
          }
      }

    if (completely_in_regs)
      aop->type = AOP_REG;
    else if (completely_spilt && !(options.stackAuto || reentrant))
      {
        aop->type = AOP_DIR;
        aop->aopu.immd = sym->rname;
      }
    else if (completely_spilt)
      aop->type = AOP_STK;
    else
      wassertl (0, "Unsupported partially spilt aop");
  }
}

static void
cheapMove (const asmop *result, int roffset, const asmop *source, int soffset, bool a_dead, bool p_dead, bool f_dead);

/*-----------------------------------------------------------------*/
/* pushAF - push af, adjusting stack tracking                      */
/*-----------------------------------------------------------------*/
static void pushAF (void)
{
  emit2 ("push", "af");
  cost (1, 1);
  G.stack.pushed += 2;
}

/*-----------------------------------------------------------------*/
/* popAF - pop af, adjusting stack tracking                      */
/*-----------------------------------------------------------------*/
static void popAF (void)
{
  emit2 ("pop", "af");
  cost (1, 1);
  G.stack.pushed -= 2;
}

/*-----------------------------------------------------------------*/
/* pushPF - push pf, adjusting stack tracking                      */
/*-----------------------------------------------------------------*/
static void pushPF (bool a_dead)
{
  if (!a_dead)
    {
      emit2 ("xch", "a, p");
      cost (1, 1);
      pushAF();
      emit2 ("xch", "a, p");
      cost (1, 1);
    }
  else
    {
      emit2 ("mov", "a, p");
      cost (1, 1);
      pushAF();
    }
}

/*-----------------------------------------------------------------*/
/* popPF - pop pf, adjusting stack tracking                      */
/*-----------------------------------------------------------------*/
static void popPF (bool a_dead)
{
  if (!a_dead)
    {
      emit2 ("xch", "a, p");
      cost (1, 1);
      popAF ();
      emit2 ("xch", "a, p");
      cost (1, 1);
    }
  else
    {
      popAF ();
      emit2 ("mov", "p, a");
      cost (1, 1);
    }
  G.p.type = AOP_INVALID;
}

/*-----------------------------------------------------------------*/
/* pointPStack - Make pseudo-register p point to stack location    */
/*-----------------------------------------------------------------*/
static void pointPStack (int s, bool a_dead, bool f_dead)
{
  // Try to adjust p when doing so is cheaper.
  if (G.p.type == AOP_STL)
    {
      if (G.p.offset == s)
        return;

      if (!f_dead)
        pushAF();

      if (abs(G.p.offset - s) < 3)
        {
          while (G.p.offset < s)
            {
              emit2 ("inc", "p");
              cost (1, 1);
              G.p.offset++;
            }

          while (G.p.offset > s)
            {
              emit2 ("dec", "p");
              cost (1, 1);
              G.p.offset--;
            }
       }
     else if (a_dead || !f_dead)
       {
         emit2 ("mov", "a, #%d", s - G.p.offset);
         emit2 ("add", "p, a");
         cost (2, 2);
         G.p.offset = s;
       }
     else
       {
         emit2 ("xch", "a, p");
         emit2 ("add", "a, #%d", s - G.p.offset);
         emit2 ("xch", "a, p");
         cost (3, 3);
         G.p.offset = s;
       }

     if (!f_dead)
        popAF();

      return;
    }

  if (!a_dead && f_dead)
    {
      int soffset = s - G.stack.pushed;
      emit2 ("xch", "a, p");
      emit2 ("mov", "a, sp");
      emit2 ("add", "a, #0x%02x", soffset & 0xff);
      emit2 ("xch", "a, p");
      cost (4, 4);
    }
  else
    {
      if (!(a_dead && f_dead))
        pushAF();

      int soffset = s - G.stack.pushed;
      cheapMove (ASMOP_A, 0, ASMOP_SP, 0, true, true, true);
      emit2 ("add", "a, #0x%02x", soffset & 0xff);
      cost (1, 1);
      cheapMove (ASMOP_P, 0, ASMOP_A, 0, true, true, true);

      if (!(a_dead && f_dead))
        popAF();
    }

  G.p.type = AOP_STL;
  G.p.offset = s;
}

/*-----------------------------------------------------------------*/
/* moveStackStack - Move a block of memory on the stack.           */
/*-----------------------------------------------------------------*/
static void
moveStackStack (int d, int s, int size, bool a_dead)
{
  if (!a_dead)
    pushAF ();

  bool up = (d <= s);

  for (int i = up ? 0 : size - 1; up ? i < size : i >= 0; up ? i++ : i--)
    {
      pointPStack (s + i, true, true);
      emit2 ("idxm", "a, p");
      cost (1, 2);
      pointPStack (d + i, false, true);
      emit2 ("idxm", "p, a");
      cost (1, 2);
    }

  if (!a_dead)
    popAF();
}

/*-----------------------------------------------------------------*/
/* cheapMove - Copy a byte from one asmop to another               */
/*-----------------------------------------------------------------*/
static void
cheapMove (const asmop *result, int roffset, const asmop *source, int soffset, bool a_dead, bool p_dead, bool f_dead)
{
  bool dummy = (result->type == AOP_DUMMY || source->type == AOP_DUMMY);

  if (aopSame (result, roffset, source, soffset, 1))
    return;
  else if (source->type == AOP_STL)
    {
      if (soffset)
        {
          cheapMove (result, roffset, ASMOP_ZERO, 0, a_dead, p_dead, f_dead);
          return;
        }

      if (aopInReg (result, roffset, P_IDX))
        {
          pointPStack (source->aopu.stk_off, a_dead, f_dead);
          return;
        }

      if (aopInReg (result, roffset, A_IDX) && !f_dead)
        {
          cost (200, 200);
          wassert (regalloc_dry_run);
        }

      if (!a_dead || !f_dead)
        pushAF ();

      if (G.p.type == AOP_STL && source->aopu.stk_off == G.p.offset)
        {
          emit2 ("mov", "a, p");
          cost (1, 1);
        }
      else
        {
          emit2 ("mov", "a, sp");
          emit2 ("add", "a, #0x%02x", (source->aopu.stk_off - G.stack.pushed) & 0xff);
          cost (2, 2);
        }
      cheapMove (result, roffset, ASMOP_A, 0, true, p_dead, true);

      if (!a_dead || !f_dead)
        popAF ();
    }
  else if (!dummy && (result->type == AOP_DIR || aopInReg (result, roffset, P_IDX)) && aopIsLitVal (source, soffset, 1, 0))
    {
      emit2 ("clear", "%s", aopGet (result, roffset));
      cost (1, 1);
    }
  else if (source->type == AOP_CODE && aopInReg (result, roffset, A_IDX))
    {
      emit2 ("call", "%s+%d", source->aopu.aop_dir, soffset);
      cost (1, 4);
    }
  else if (source->type == AOP_STK && aopInReg (result, roffset, A_IDX) && !aopIsLitVal (source, soffset, 1, 0))
    {
      if (!p_dead)
        pushPF (true);
      pointPStack(source->aopu.bytes[soffset].byteu.stk, true, f_dead);
      emit2 ("idxm", "a, p");
      cost (1, 2);
      if (!p_dead)
        popPF (false);
    }
  else if (result->type == AOP_STK && aopInReg (source, soffset, A_IDX))
    {
      if (!p_dead)
        pushPF (false);
      pointPStack(result->aopu.bytes[roffset].byteu.stk, false, f_dead);
      emit2 ("idxm", "p, a");
      cost (1, 2);
      if (!p_dead)
        popPF (a_dead);
    }
  else if (aopInReg (result, roffset, A_IDX))
    {
      emit2 ("mov", "a, %s", aopGet (source, soffset));
      cost (1, 1);
    }
  else if (aopInReg (source, soffset, A_IDX))
    {
      emit2 ("mov", "%s, a", aopGet (result, roffset));
      cost (1, 1);
    }
  else if (result->type == AOP_STK && (source->type == AOP_DIR || source->type == AOP_IMMD || source->type == AOP_LIT || source->type == AOP_SFR) && a_dead && p_dead)
    {
      pointPStack(result->aopu.bytes[roffset].byteu.stk, true, f_dead);
      emit2 ("mov", "a, %s", aopGet (source, soffset));
      emit2 ("idxm", "p, a");
      cost (2, 3);
    }
  else if (result->type == AOP_STK && source->type == AOP_STK && !p_dead)
    {
      if (!a_dead)
        pushAF ();
      pushPF (true);
      cheapMove (ASMOP_A, 0, source, soffset, true, true, true);
      cheapMove (result, roffset, ASMOP_A, 0, true, true, true);
      popPF (true);
      if (!a_dead)
        popAF ();
    }
  else
    {
      if (!a_dead)
        pushAF ();
      cheapMove (ASMOP_A, 0, source, soffset, true, p_dead, f_dead || !a_dead);
      cheapMove (result, roffset, ASMOP_A, 0, true, p_dead, f_dead || !a_dead);
      if (!a_dead)
        popAF ();
    }

  if (aopInReg (result, roffset, P_IDX))
    G.p.type = AOP_INVALID;
}

/*--------------------------------------------------------------------------*/
/* adjustStack - Adjust the stack pointer by n bytes.                       */
/*--------------------------------------------------------------------------*/
static void
adjustStack (int n, bool a_free, bool p_free)
{
  wassertl_bt (!(n % 2), "Unsupported odd stack adjustment"); // The datasheets seem to require the stack pointer to be aligned to a 2-byte boundary. 

  if (n >= 0 && (!(p_free || a_free) || n <= 4))
    for (int i = 0; i < n; i += 2)
      {
        emit2 ("push", "af");
        cost (1, 1);
      }
  else if (!a_free && p_free)
    {
      emit2 ("xch", "a, p");
      emit2 ("mov", "a, sp");
      emit2 ("add", "a, #%d", n);
      emit2 ("mov", "sp, a");
      emit2 ("xch", "a, p");
      cost (5, 5);
    }
  else if (!a_free && !p_free && n < 0)
    {
      pushAF();
      cheapMove (ASMOP_A, 0, ASMOP_P, 0, true, true, true);
      pushAF();

      moveStackStack (G.stack.pushed + n - 4, G.stack.pushed - 4, 2, true);
      pointPStack (G.stack.pushed - 2, true, true);
      emit2 ("idxm", "a, p");
      emit2 ("mov", "p, a");
      cost (2, 3);

      emit2 ("mov", "a, sp");
      emit2 ("add", "a, #%d", n - 2);
      emit2 ("mov", "sp, a");
      cost (3, 3);
      G.stack.pushed -= 2;

      popAF();
    }
  else // Can't use pop af, since it might affect reserved flag bits.
    {
      wassert (a_free);
      emit2 ("mov", "a, sp");
      emit2 ("add", "a, #%d", n);
      emit2 ("mov", "sp, a");
      cost (3, 3);
    }

  G.stack.pushed += n;
}

/*-----------------------------------------------------------------*/
/* popP - pop p (preserving f), adjusting stack tracking           */
/*-----------------------------------------------------------------*/
static void popP (bool a_dead)
{
  pushAF ();
  pointPStack (G.stack.pushed - 3, false, true);
  emit2 ("idxm", "a, p");
  emit2 ("mov", "p, a");
  cost (2, 3);
  popAF ();
  adjustStack (-2, a_dead, false);
  G.p.type = AOP_INVALID;
}

static void
push (const asmop *op, int offset, int size)
{
  wassertl (!(size % 2) && (op->type == AOP_DIR || op->type == AOP_REG || op->type == AOP_LIT || op->type == AOP_IMMD || op->type == AOP_STK || op->type == AOP_STL), "Unimplemented push operand");

  int a_litval = -1;

  for (int i = 0; i < size; i+= 2)
    {
      if (aopInReg (op, i, P_IDX) && aopInReg (op, i + 1, A_IDX))
        {
          emit2 ("xch", "a, p");
          cost (1, 1);
          pushAF ();
          emit2 ("xch", "a, p");
          pointPStack (G.stack.pushed - 1, false, true);
          emit2 ("idxm", "p, a");
          cost (1, 1);
          continue;
        }

      if (a_litval < 0 || !aopIsLitVal (op, i, 1, a_litval))
        {
          cheapMove (ASMOP_A, 0, op, i, true, true, true);
          if (op->type == AOP_LIT)
            a_litval = byteOfVal (op->aopu.aop_lit, i);
        }

      pushAF ();

      if (aopInReg (op, i + 1, P_IDX))
        {
          emit2 ("xch", "a, p");
          cost (1, 1);
          pointPStack (G.stack.pushed - 1, false, true);
        }
      else if (a_litval >= 0 && aopIsLitVal (op, i + 1, 1, a_litval) && i > 1) // Only try to reuse a for i > 1, as otherwise pointPStack preserving a is too expensive.
        {
           pointPStack (G.stack.pushed - 1, false, true);
        }
      else if (op->type == AOP_STK)
        {
          cheapMove (ASMOP_A, 0, op, i + 1, true, true, true);
          pointPStack (G.stack.pushed - 1, false, true);
        }
      else
        {
          pointPStack (G.stack.pushed - 1, true, true);
          cheapMove (ASMOP_A, 0, op, i + 1, true, false, true);
        }
      emit2 ("idxm", "p, a");
      cost (1, 1);

      
      if (op->type == AOP_LIT)
        a_litval = byteOfVal (op->aopu.aop_lit, i + 1);
    }
}

/*-----------------------------------------------------------------*/
/* genMove_o - Copy part of one asmop to another                   */
/*-----------------------------------------------------------------*/
static void
genMove_o (asmop *result, int roffset, asmop *source, int soffset, int size, bool a_dead_global, bool p_dead_global)
{
  // Handle I/O first.
  wassert_bt ((result->type == AOP_SFR) + (source->type == AOP_SFR) <= 1);

  if (source->type == AOP_SFR)
    {
      wassert (source->size == 1 || source->size == 2);
      wassert (size >= source->size);

      if (source->size == 1)
        {
          cheapMove (result, roffset, source, soffset, a_dead_global, p_dead_global, true);
          genMove_o (result, roffset + 1, ASMOP_ZERO, 0, size - 1, a_dead_global, p_dead_global);
        }
#if 0 // TODO: Implement alignment requirements - ldt16 needs 16-bit-aligned operand
      else if (source->size == 2 && result->type == AOP_DIR && source->type == AOP_SFR)
        {
          emit2 ("ldt16", "%s", aopGet (result, roffset));
          cost (1, 1); // TODO: Really just 1 cycle? Other 16-bit-transfer instructions use 2.
        }
#endif
      else
        wassertl (0, "Unimplemented __sfr16 load");

      return;
    }
  else if (result->type == AOP_SFR)
    {
      wassert (result->size == 1 || result->size == 2);
      wassert (size == result->size);

      if (size == 1)
        cheapMove (result, roffset, source, soffset, a_dead_global, p_dead_global, true);
      else if (size == 2 && result->type == AOP_SFR && source->type == AOP_LIT && aopIsLitVal (source, 1, 1, 0x00))
          {
            if (!p_dead_global)
              pushPF (a_dead_global && !aopInReg (source, 0, A_IDX));
            cheapMove (ASMOP_P, 0, source, 0, true, true, true);
            emit2 ("stt16", "p");
            cost (1, 1); // TODO: Really just 1 cycle? Other 16-bit-transfer instructions use 2.
            if (!p_dead_global)
              popPF (a_dead_global);
          }
#if 0 // TODO: Implement alignment requirements - stt16 needs 16-bit-aligned operand
      else if (size == 2 && result->type == AOP_SFR && source->type == AOP_DIR)
          {
            emit2 ("stt16", "%s", aopGet (source, soffset));
            cost (1, 1); // TODO: Really just 1 cycle? Other 16-bit-transfer instructions use 2.
          }
#endif
      else if (regalloc_dry_run)
        cost (1000, 1000);
      else
        wassertl (0, "Unimplemented __sfr16 store");

      return;
    }

  wassert_bt (result->type == AOP_DIR || result->type == AOP_REG || result->type == AOP_STK);
  wassert_bt (source->type == AOP_LIT || source->type == AOP_IMMD || source->type == AOP_DIR || source->type == AOP_REG || source->type == AOP_STK || source->type == AOP_CODE || source->type == AOP_STL);

  if (size == 2 && aopInReg (result, roffset, P_IDX) && aopInReg (result, roffset + 1, A_IDX) && source->type == AOP_STK)
    {
      cheapMove (result, roffset + 1, source, soffset + 1, true, p_dead_global, true);
      cheapMove (result, roffset + 0, source, soffset + 0, false, p_dead_global, true);
      return;
    }
  else if (size == 2 && result->type == AOP_STK && aopInReg (source, soffset, A_IDX) && aopInReg (source, soffset + 1, P_IDX))
    {
      if (!p_dead_global)
        pushPF (false);
      pushAF ();
      emit2 ("xch", "a, p");
      cost (1, 1);
      cheapMove (result, roffset + 1, ASMOP_A, 0, false, true, true);
      popAF ();
      cheapMove (result, roffset + 0, ASMOP_A, 0, a_dead_global, true, true);
      if (!p_dead_global)
        popPF (a_dead_global);
      return;
    }
  else if (size == 2 && result->type == AOP_STK && aopInReg (source, soffset, P_IDX) && aopInReg (source, soffset + 1, A_IDX))
    {
      if (!p_dead_global)
        pushPF (false);
      pushAF ();
      emit2 ("xch", "a, p");
      cost (1, 1);
      cheapMove (result, roffset + 0, ASMOP_A, 0, false, true, true);
      popAF ();
      cheapMove (result, roffset + 1, ASMOP_A, 0, a_dead_global, true, true);
      if (!p_dead_global)
        popPF (a_dead_global);
      return;
    }
  else if (size == 2 && result->type == AOP_DIR && !a_dead_global && // Using xch cheaper than push / pop.
    aopInReg (source, soffset, A_IDX) && aopInReg (source, soffset + 1, P_IDX))
    {
      cheapMove (result, roffset + 0, ASMOP_A, 0, false, p_dead_global, true);
      emit2 ("xch", "a, p");
      cost (1, 1);
      cheapMove (result, roffset + 1, ASMOP_A, 0, p_dead_global, true, true);
      emit2 ("xch", "a, p");
      cost (1, 1);
      return;
    }
  else if (size == 2 && result->type == AOP_DIR && !a_dead_global && // Using xch cheaper than push / pop.
    aopInReg (source, soffset, P_IDX) && aopInReg (source, soffset + 1, A_IDX))
    {
      cheapMove (result, roffset + 1, ASMOP_A, 0, false, p_dead_global, true);
      emit2 ("xch", "a, p");
      cost (1, 1);
      cheapMove (result, roffset + 0, ASMOP_A, 0, p_dead_global, false, true);
      emit2 ("xch", "a, p");
      cost (1, 1);
      return;
    }
  else if (size == 2 && // Assign upper byte first to avoid overwriting a.
    (aopInReg (result, roffset, A_IDX) && aopInReg (result, roffset + 1, P_IDX) && source->type == AOP_DIR ||
    aopInReg (source, soffset, P_IDX) && aopInReg (source, soffset + 1, A_IDX) && result->type == AOP_DIR))
    {
      cheapMove (result, roffset + 1, source, soffset + 1, a_dead_global, p_dead_global, true);
      cheapMove (result, roffset + 0, source, soffset + 0, a_dead_global, p_dead_global, true);
      return;
    }
  else if (size == 2 &&
    (aopInReg (source, soffset, A_IDX) && aopInReg (source, soffset + 1, P_IDX) && aopInReg (result, roffset, P_IDX) && aopInReg (result, roffset + 1, A_IDX) ||
    aopInReg (source, soffset, P_IDX) && aopInReg (source, soffset + 1, A_IDX) && aopInReg (result, roffset, A_IDX) && aopInReg (result, roffset + 1, P_IDX)))
    {
      emit2 ("xch", "a, p");
      cost (1, 1);
      return;
    }
  else if (size >= 2 && result->type == AOP_DIR && source->type == AOP_DIR && !strcmp (result->aopu.aop_dir, source->aopu.aop_dir) && soffset < roffset)
    {
      if (!a_dead_global)
        pushAF ();
      if (soffset + 1 == roffset) // Use xch via a.
        {
          emit2 ("mov", "a, %s", aopGet (source, soffset));
          for (int i = 0; i < size - 1; i++)
            emit2 ("xch", "a, %s", aopGet (result, roffset + i));
          emit2 ("mov", "%s, a", aopGet (result, roffset + size - 1));
          cost (size + 1, size + 1);
        }
      else // Copy high-to-low to avoid overwriting of still-needed bytes.
        {
          for (int i = size - 1; i >= 0; i--)
            cheapMove (result, roffset + i, source, soffset + i, true, p_dead_global, true);
        }
      if (!a_dead_global)
        popAF ();
      return;
    }
  else if (size >= 2 && result->type == AOP_STK && source->type == AOP_STK && // Copy high-to-low to avoid overwriting of still-needed bytes.
    source->aopu.bytes[soffset].byteu.stk < result->aopu.bytes[roffset].byteu.stk && source->aopu.bytes[soffset].byteu.stk + size > result->aopu.bytes[roffset].byteu.stk)
    {
      if (!a_dead_global)
        pushAF ();
      for (int i = size - 1; i >= 0; i--)
        cheapMove (result, roffset + i, source, soffset + i, true, p_dead_global, true);
      if (!a_dead_global)
        popAF ();
      return;
    }
  else if (size >= 2 && result->type == AOP_DIR && source->type == AOP_DIR && !strcmp (result->aopu.aop_dir, source->aopu.aop_dir) && soffset > roffset && roffset + 1 == soffset && a_dead_global) // Use xch via a.
    {
      emit2 ("mov", "a, %s", aopGet (source, soffset + size - 1));
      for (int i = size - 1; i > 0; i--)
        emit2 ("xch", "a, %s", aopGet (result, roffset + i));
      emit2 ("mov", "%s, a", aopGet (result, roffset));
      cost (size + 1, size + 1);
      return;
    }
  else if (size >= 2 && result->type == AOP_STK && source->type == AOP_LIT) // Cache lit values in a.
    {
      int a_litval = -1;
      if (!a_dead_global)
        pushAF ();
      if (!p_dead_global)
        pushPF (true);
      for (unsigned int i = 0; i < size; i++)
        {
          pointPStack(result->aopu.bytes[roffset + i].byteu.stk, !i, true);
          if (a_litval != byteOfVal (source->aopu.aop_lit, soffset + i))
            {
              cheapMove (ASMOP_A, 0, source, soffset + i, true, false, true);
              a_litval = byteOfVal (source->aopu.aop_lit, soffset + i);
            }
          emit2 ("idxm", "p, a");
          cost (1, 2);
        }
      if (!p_dead_global)
        popPF (true);
      if (!a_dead_global)
        popAF ();
      return;
    }

  bool a_dead = a_dead_global && !aopInReg (source, soffset + 0, A_IDX) && !(aopInReg (source, soffset + 1, A_IDX) && size > 1);
  for (unsigned int i = 0; i < size; i++)
    {
      if (a_dead_global && aopInReg (source, soffset, A_IDX))
        a_dead = true;

      cheapMove (result, roffset + i, source, soffset + i, a_dead, p_dead_global, true);

      if (aopInReg (result, roffset + i, A_IDX))
        a_dead = false;
    }
}

/*-----------------------------------------------------------------*/
/* genMove - Copy the value from one asmop to another              */
/*-----------------------------------------------------------------*/
static void
genMove (asmop *result, asmop *source, bool a_dead, bool p_dead)
{
  genMove_o (result, 0, source, 0, result->size, a_dead, p_dead);
}

/*-----------------------------------------------------------------*/
/* isLiteralBit - test if lit == 2^n                               */
/*-----------------------------------------------------------------*/
static int
isLiteralBit (unsigned long lit)
{
  unsigned long pw[32] =
  {
    1l, 2l, 4l, 8l, 16l, 32l, 64l, 128l,
    0x100l, 0x200l, 0x400l, 0x800l,
    0x1000l, 0x2000l, 0x4000l, 0x8000l,
    0x10000l, 0x20000l, 0x40000l, 0x80000l,
    0x100000l, 0x200000l, 0x400000l, 0x800000l,
    0x1000000l, 0x2000000l, 0x4000000l, 0x8000000l,
    0x10000000l, 0x20000000l, 0x40000000l, 0x80000000l
  };
  int idx;

  for (idx = 0; idx < 32; idx++)
    if (lit == pw[idx])
      return idx;
  return -1;
}

/*-----------------------------------------------------------------*/
/* genNot - generates code for !                                   */
/*-----------------------------------------------------------------*/
static void
genNot (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);

  D (emit2 ("; genNot", ""));

  aopOp (left, ic);
  aopOp (result, ic);

  if (IS_BOOL (operandType (left)))
    {
      if ((aopInReg (left->aop, 0, P_IDX) || left->aop->type == AOP_DIR) && aopSame (result->aop, 0, left->aop, 0, 1))
        {
          emit2 ("mov", "a, #0x01");
          emit2 ("xor", "%s, a", aopGet (left->aop, 0));
          cost (2, 2);
        }
      else
        {
          cheapMove (ASMOP_A, 0, left->aop, 0, true, regDead (P_IDX, ic), true);
          emit2 ("xor", "a, #0x01");
          cost (1, 1);
          cheapMove (result->aop, 0, ASMOP_A, 0, true, regDead (P_IDX, ic), true);
        }
    }
  else
    {
      cheapMove (ASMOP_A, 0, left->aop, 0, true, regDead (P_IDX, ic), true);

      for (int i = 1; i < left->aop->size; i++)
        {
          if (left->aop->type == AOP_STK)
            {
              if (!regDead (P_IDX, ic))
                pushPF (true);
              cheapMove (ASMOP_P, 0, left->aop, i, false, true, true);
              emit2 ("or", "a, p");
              if (!regDead (P_IDX, ic))
                popPF (true);
            }
          else
            emit2 ("or", "a, %s", aopGet (left->aop, i));
          cost (1, 1);
        }
      emit2 ("sub", "a, #0x01");
      emit2 ("mov", "a, #0x00");
      emit2 ("slc", "a");

      cheapMove (result->aop, 0, ASMOP_A, 0, true, true, true);
    }

  genMove_o (result->aop, 1, ASMOP_ZERO, 0, result->aop->size - 1, true, true);

  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genCpl - generate code for complement                           */
/*-----------------------------------------------------------------*/
static void
genCpl (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);

  D (emit2 ("; genCpl", ""));

  aopOp (left, ic);
  aopOp (result, ic);

  int size = result->aop->size;

  if (result->aop->type == AOP_STK)
    {
      bool pushed_p = false;
      for (int i = 0; i < size; i++)
        {
          if (aopInReg (left->aop, P_IDX, i) && pushed_p)
            {
              popAF ();
              pushed_p = false;
            }
          else
            cheapMove (ASMOP_A, 0, left->aop, i, true, true, true);
          emit2 ("not", "a");
          if (aopInReg (left->aop, P_IDX, i + 1))
            {
              pushPF (false);
              pushed_p = true;
            }
          cheapMove (result->aop, i, ASMOP_A, 0, true, true, true);
        }
     }
  else
    {
      genMove (result->aop, left->aop, true, true);

      for (int i = 0; i < size; i++)
        {
          emit2 ("not", "%s", aopGet (result->aop, i));
          cost (1, 1);
        }
    }

  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genSub - generates code for subtraction                         */
/*-----------------------------------------------------------------*/
static void
genSub (const iCode *ic, asmop *result_aop, asmop *left_aop, asmop *right_aop)
{
  int size = result_aop->size;

  bool started = false;
  bool pushed_a = false;
  bool pushed_p = false;

  if (!regDead(P_IDX, ic) &&
    (left_aop->type == AOP_STK || right_aop->type == AOP_STK || result_aop->type == AOP_STK))
    {
      pushPF (regDead (A_IDX, ic) && left_aop->type != AOP_REG && right_aop->type != AOP_REG);
      pushed_p = true;
    }

  for (int i = 0; i < size; i++)
    {
      if (!started && right_aop->type == AOP_LIT && aopIsLitVal (right_aop, i, 1, 0x00))
        {
          cheapMove (result_aop, i, left_aop, i, regDead (A_IDX, ic), true, true);
          if (aopInReg (result_aop, i, A_IDX) && i + 1 < size)
            {
              pushAF ();
              pushed_a = true;
            }
          continue;
        }
      else if (!started && i + 1 == size && aopIsLitVal (left_aop, i, 1, 0x00) &&
        (right_aop->type == AOP_DIR || aopInReg (right_aop, i, P_IDX)) && aopSame (right_aop, i, result_aop, i, 1))
        {
          emit2 ("neg", "%s", aopGet (right_aop, i));
          cost (1, 1);
          started = true;
          continue;
        }
      else if ((TARGET_IS_PDK15 || TARGET_IS_PDK16) &&
        !started && i + 1 == size && aopInReg (left_aop, i, A_IDX) &&
        (right_aop->type == AOP_DIR || aopInReg (right_aop, i, P_IDX)) && aopSame (right_aop, i, result_aop, i, 1))
        {
          emit2 ("nadd", "%s, a", aopGet (right_aop, i));
          cost (1, 1);
          started = true;
          continue;
        }
      else if (!started && aopIsLitVal (right_aop, i, 1, 0x01) &&
        (left_aop->type == AOP_DIR || aopInReg (left_aop, i, P_IDX)) && aopSame (left_aop, i, result_aop, i, 1))
        {
          emit2 ("dec", "%s", aopGet (left_aop, i));
          cost (1, 1);
          started = true;
          continue;
        }
      else if (!started && i + 1 == size && aopIsLitVal (right_aop, i, 1, 0xff) &&
        (left_aop->type == AOP_DIR || aopInReg (left_aop, i, P_IDX)) && aopSame (left_aop, i, result_aop, i, 1))
        {
          emit2 ("inc", "%s", aopGet (left_aop, i));
          cost (1, 1);
          started = true;
          continue;
        }
      else if (started && (left_aop->type == AOP_DIR || left_aop->type == AOP_REGDIR || left_aop->type == AOP_REG) && aopIsLitVal (right_aop, i, 1, 0x00) && aopSame (left_aop, i, result_aop, i, 1))
        {
          emit2 ("subc", "%s", aopGet (left_aop, i));
          cost (1, 1);
          continue;
        }

      if (!(regDead (A_IDX, ic) || pushed_a))
        {
          pushAF();
          pushed_a = true;
        }

      if ((left_aop->type == AOP_DIR || aopInReg (left_aop, i, P_IDX)) && right_aop->type != AOP_STK && aopSame (left_aop, i, result_aop, i, 1))
        {
          cheapMove (ASMOP_A, 0, right_aop, i, true, !aopInReg (left_aop, i, P_IDX), !started);
          emit2 (started ? "subc" : "sub", "%s, a", aopGet (left_aop, i));
          cost (1, 1);
          started = true;
          continue;
        }
      else if (aopInReg (right_aop, P_IDX, i) && left_aop->type == AOP_STK)
        {
          cheapMove (ASMOP_A, 0, left_aop, i, true, false, !started);
          emit2 (started ? "subc" : "sub", "a, p");
          started = true;
          continue;
        }
      else if (!started && i + 1 == size && aopIsLitVal (left_aop, i, 1, 0x00))
        {
          cheapMove (ASMOP_A, 0, right_aop, i, true, true, true);
          emit2 ("neg", "a");
          cost (1, 1);
          started = true;
        }
      else if (!started && i + 1 == size && aopInReg (right_aop, i, A_IDX) &&
        (left_aop->type == AOP_LIT || left_aop->type == AOP_IMMD || left_aop->type == AOP_DIR || aopInReg (left_aop, i, P_IDX)))
        {
          if ((TARGET_IS_PDK15 || TARGET_IS_PDK16) && (left_aop->type == AOP_IMMD || aopInReg (left_aop, i, P_IDX)))
            {
              emit2 ("nadd", "a, %s", aopGet (left_aop, i));
              cost (1, 1);
            }
          else
            {
              emit2 ("neg", "a");
              emit2 ("add", "a, %s", aopGet (left_aop, i));
              cost (2, 2);
            }
          started = true;
        }
      else if (right_aop->type == AOP_STK || right_aop->type == AOP_STL && !i)
        {
          if (i + 1 < size && aopInReg (left_aop, i + 1, P_IDX))
            pushPF (!aopInReg (left_aop, i, A_IDX));
          cheapMove (ASMOP_A, 0, left_aop, i, true, true, !started);
          cheapMove (ASMOP_P, 0, right_aop, i, false, true, !started);
          emit2 (started ? "subc" : "sub", "a, p");
          cost (1, 1);
          if (i + 1 < size && aopInReg (left_aop, i + 1, P_IDX))
            popP (false);
          started = true;
        }
     else if (started && (right_aop->type == AOP_LIT || right_aop->type == AOP_IMMD) && !aopIsLitVal (right_aop, i, 1, 0x00) && i + 1 == size)
        {
          cheapMove (ASMOP_A, 0, left_aop, i, true, true, false);
          emit2 ("subc", "a");
          emit2 ("sub", "a, %s", aopGet (right_aop, i));
          cost (2, 2);
        }
      else if ((started && (right_aop->type == AOP_LIT || right_aop->type == AOP_IMMD) && !aopIsLitVal (right_aop, i, 1, 0x00) || aopInReg (right_aop, i, A_IDX)) &&
        regDead (A_IDX, ic) && regDead (P_IDX, ic) && !aopInReg(left_aop, P_IDX, i + 1) && !aopInReg(right_aop, P_IDX, i + 1))
        {
          cheapMove (ASMOP_P, 0, right_aop, i, !aopInReg (left_aop, i, A_IDX), true, !started);
          cheapMove (ASMOP_A, 0, left_aop, i, true, false, !started);
          emit2 (started ? "subc" : "sub", "a, p");
          cost (1, 1);
          started = true;
        }
      else if (aopInReg (right_aop, i, A_IDX))
        {
          if (i + 1 < size && aopInReg (right_aop, i + 1, P_IDX) || !regDead (P_IDX, ic) && !pushed_p)
            pushPF (false);
          if (aopInReg (left_aop, i, P_IDX))
            {
              emit2 ("xch", "a, p");
              cost (1, 1);
              G.p.type = AOP_INVALID;
            }
          else
            {
              cheapMove (ASMOP_P, 0, right_aop, i, true, !aopInReg (left_aop, i, P_IDX), !started);
              cheapMove (ASMOP_A, 0, left_aop, i, true, false, !started);
            }
          emit2 (started ? "subc" : "sub", "a, p");
          cost (1, 1);
          started = true;
          if (i + 1 < size && aopInReg (right_aop, i + 1, P_IDX) || !regDead (P_IDX, ic) && !pushed_p)
            {
              if (!pushed_a && !regDead (P_IDX, ic) && !pushed_p)
                pushed_p = true;
              else if (i + 1 < size)
                popP (false);
              else
                popPF (false);
            }
        }
      else
        {
          wassert (!aopInReg (right_aop, i, A_IDX));
          cheapMove (ASMOP_A, 0, left_aop, i, true, !aopInReg (right_aop, i, P_IDX), !started);
          if (started || !aopIsLitVal (right_aop, i, 1, 0x00))
            {
              if (started && aopIsLitVal (right_aop, i, 1, 0x00))
                emit2 ("subc", "a");
              else
                emit2 (started ? "subc" : "sub", "a, %s", aopGet (right_aop, i));
              cost (1, 1);
              started = true;
            }
        }
      if (i + 1 < size && aopInReg (result_aop, i, P_IDX) && (left_aop->type == AOP_STK || right_aop->type == AOP_STK))
        {
          if (!pushed_a)
            {
              pushPF (false);
              pushed_p = true;
            }
          else if (regalloc_dry_run)
            cost (1000, 1000);
          else
            wassertl (0, "Unimplemented p result in subtraction with stack operand");
        }
      if (i + 1 < size && result_aop->type == AOP_STK && (aopInReg (left_aop, i + 1, P_IDX) || aopInReg (right_aop, i + 1, P_IDX))) // Avoid overwriting still-needed p when storing onto stack.
        {
          emit2 ("xch", "a, p");
           G.p.type = AOP_INVALID;
          pushAF ();
          cheapMove (result_aop, i, ASMOP_P, 0, true, true, i + 1 == size);
          popAF ();
          emit2 ("xch", "a, p");
          G.p.type = AOP_INVALID;
          continue;
        }

      if (aopInReg (result_aop, i, A_IDX) && i + 1 < size)
        {
          pushAF ();
          pushed_a = true;
        }
      else
        cheapMove (result_aop, i, ASMOP_A, 0, true, true, i + 1 == size);
    }

  if (pushed_a)
    popAF ();

  if (pushed_p)
    popPF (regDead (A_IDX, ic) && result_aop->type != AOP_REG);
}

/*-----------------------------------------------------------------*/
/* genUminus - generates code for unary minus                      */
/*-----------------------------------------------------------------*/
static void
genUminusFloat (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);

  D (emit2 ("; genUminusFloat", ""));

  genMove_o (result->aop, 0, left->aop, 0, result->aop->size - 1, true, true);

  cheapMove (ASMOP_A, 0, left->aop, result->aop->size - 1, true, true, true);
  emit2 ("xor", "a, #0x80");
  cost (1, 1);
  cheapMove (result->aop, result->aop->size - 1, ASMOP_A, 0, true, true, true);

  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genUminus - generates code for unary minus                      */
/*-----------------------------------------------------------------*/
static void
genUminus (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);

  aopOp (left, ic);
  aopOp (result, ic);

  if (IS_FLOAT (operandType (left)))
    {
      genUminusFloat (ic);
      return;
    }

  D (emit2 ("; genUminus", ""));

  genSub (ic, result->aop, ASMOP_ZERO, left->aop);

  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genIpush - generate code for pushing this gets a little complex */
/*-----------------------------------------------------------------*/
static void
genIpush (const iCode *ic)
{
  operand *left = IC_LEFT (ic);

  aopOp (left, ic);

  D (emit2 ("; genIPush", ""));

  if (!ic->parmPush)
    wassertl (0, "Encountered an unsupported spill push.");

  wassertl (left->aop->size == 1 || !(left->aop->size % 2), "Unimplemented operand size for parameter push");

  if (left->aop->type == AOP_DUMMY)
    adjustStack (left->aop->size + (left->aop->size % 2), regDead (A_IDX, ic), regDead(P_IDX, ic));
  else if (left->aop->size == 1)
    {
      cheapMove (ASMOP_A, 0, left->aop, 0, true, regDead (P_IDX, ic), true);
      pushAF();
    }
  else
    push (left->aop, 0, left->aop->size);

  freeAsmop (IC_LEFT (ic));
}

/*-----------------------------------------------------------------*/
/* genCall - generates a call statement                            */
/*-----------------------------------------------------------------*/
static void
genCall (const iCode *ic)
{
  sym_link *dtype = operandType (IC_LEFT (ic));
  sym_link *etype = getSpec (dtype);
  sym_link *ftype = IS_FUNCPTR (dtype) ? dtype->next : dtype;
  bool tailjump = false;

  operand *left = IC_LEFT (ic);

  const bool bigreturn = (getSize (ftype->next) > 2) || IS_STRUCT (ftype->next);
  const bool SomethingReturned = (IS_ITEMP (IC_RESULT (ic)) &&
                       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir))
                       || IS_TRUE_SYMOP (IC_RESULT (ic));

  D (emit2 ("; genCall", ""));  

  aopOp (left, ic);
  if (SomethingReturned && !bigreturn)
    aopOp (IC_RESULT (ic), ic);

  if (bigreturn)
    {
      wassertl (IC_RESULT (ic), "Unused return value in call to function returning large type.");

      const symbol *rsym = OP_SYMBOL_CONST (IC_RESULT (ic));
      if (rsym->usl.spillLoc)
        rsym = rsym->usl.spillLoc;

      if (rsym->onStack || rsym->isspilt && regalloc_dry_run && (options.stackAuto || reentrant))
        {
          emit2 ("mov", "a, sp");
          emit2 ("add", "a, #0x%02x", (rsym->stack + (rsym->stack < 0 ? G.stack.param_offset : 0) - G.stack.pushed) & 0xff);
        }
      else
        {
          emit2 ("mov", "a, #%s", rsym->rname);
          cost (1, 1);
        }
      pushAF ();
    }
  // Check if we can do tail call optimization.
  else if (!(currFunc && IFFUNC_ISISR (currFunc->type)) &&
    (!SomethingReturned || IC_RESULT (ic)->aop->size == 1 && aopInReg (IC_RESULT (ic)->aop, 0, A_IDX) || IC_RESULT (ic)->aop->size == 2 && aopInReg (IC_RESULT (ic)->aop, 0, A_IDX) && aopInReg (IC_RESULT (ic)->aop, 1, P_IDX)) &&
    !ic->parmBytes && !ic->localEscapeAlive)
    {
      int limit = 16; // Avoid endless loops in the code putting us into an endless loop here.

      for (const iCode *nic = ic->next; nic && --limit;)
        {
          const symbol *targetlabel = 0;

          if (nic->op == LABEL)
            ;
          else if (nic->op == GOTO) // We dont have ebbi here, so we cant jsut use eBBWithEntryLabel (ebbi, ic->label). Search manually.
            targetlabel = IC_LABEL (nic);
          else if (nic->op == RETURN && (!IC_LEFT (nic) || SomethingReturned && IC_RESULT (ic)->key == IC_LEFT (nic)->key))
            targetlabel = returnLabel;
          else if (nic->op == ENDFUNCTION)
            {
              if (OP_SYMBOL (IC_LEFT (nic))->stack == 0)
                tailjump = true;
              break;
            }
          else
            break;

          if (targetlabel)
            {
              const iCode *nnic = 0;
              for (nnic = nic->next; nnic; nnic = nnic->next)
                if (nnic->op == LABEL && IC_LABEL (nnic)->key == targetlabel->key)
                  break;
              if (!nnic)
                for (nnic = nic->prev; nnic; nnic = nnic->prev)
                  if (nnic->op == LABEL && IC_LABEL (nnic)->key == targetlabel->key)
                    break;
              if (!nnic)
                break;

              nic = nnic;
            }
          else
            nic = nic->next;
        }
    }

  if (!regDead (A_IDX, ic) && ic->parmBytes || !regDead (P_IDX, ic))
    {
      cost (700, 700);
      wassertl (regalloc_dry_run, "Register saving across call not yet implemented");
    }

  if (!regDead (A_IDX, ic))
    pushAF ();

  bool jump = tailjump || !ic->parmBytes && IFFUNC_ISNORETURN (ftype);

  if (ic->op == PCALL)
    {
      // Push return address
      symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

      if (!jump)
        {
          if (!regalloc_dry_run)
            {
              emit2 ("mov", "a, #<(!tlabel)", labelKey2num (tlbl->key));
              emit2 ("push", "af");
              emit2 ("mov", "a, sp");
              emit2 ("mov", "p, a");
              emit2 ("dec", "p");
              emit2 ("mov", "a, #>(!tlabel)", labelKey2num (tlbl->key));
              emit2 ("idxm", "p, a");
              G.p.type = AOP_INVALID;
            }
          G.stack.pushed += 2;
          cost (7, 8);
        }

      // Jump to function
      push (left->aop, 0, 2);
      emit2 ("ret", "");
      G.stack.pushed -= 4 - jump * 2;
      cost (2, 1);

      if (!jump)
        emitLabel (tlbl);
    }
  else
    {
      if (IS_LITERAL (etype))
        emit2 (jump ? "goto" : "call", "0x%04X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
      else
        {
          emit2 (jump ? "goto" : "call", "%s",
                 (OP_SYMBOL (IC_LEFT (ic))->rname[0] ? OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name));
        }
      cost (1, 2);
    }
  G.p.type = AOP_INVALID;

  freeAsmop (left);

  if (!SomethingReturned || bigreturn)
    adjustStack (-ic->parmBytes - bigreturn * 2, true, true);
  else
    {
      genMove (IC_RESULT (ic)->aop, ASMOP_AP, true, true);

      adjustStack (-ic->parmBytes, !(aopInReg (IC_RESULT (ic)->aop, 0, A_IDX) || aopInReg (IC_RESULT (ic)->aop, 1, A_IDX)), !(aopInReg (IC_RESULT (ic)->aop, 0, P_IDX) || aopInReg (IC_RESULT (ic)->aop, 1, P_IDX)));

      freeAsmop (IC_RESULT (ic));
    }


  if (!regDead (A_IDX, ic))
    popAF ();
}

/*-----------------------------------------------------------------*/
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void
genFunction (iCode *ic)
{
  const symbol *sym = OP_SYMBOL_CONST (IC_LEFT (ic));
  sym_link *ftype = operandType (IC_LEFT (ic));

  G.stack.pushed = 0;
  G.stack.param_offset = 0;

  /* create the function header */
  emit2 (";", "-----------------------------------------");
  emit2 (";", " function %s", sym->name);
  emit2 (";", "-----------------------------------------");

  D (emit2 (";", pdk_assignment_optimal ? "Register assignment is optimal." : "Register assignment might be sub-optimal."));

  emit2 ("", "%s:", sym->rname);
  if (!regalloc_dry_run)
    genLine.lineCurr->isLabel = 1;

  if (IFFUNC_ISNAKED(ftype))
    {
      emit2 (";", "naked function: no prologue.");
      return;
    }

  if (IFFUNC_ISISR (sym->type))
    {
      pushAF ();
      cheapMove (ASMOP_A, 0, ASMOP_P, 0, true, true, true);
      pushAF ();
    }

  G.stack.param_offset -= (getSize (ftype->next) > 2 || IS_STRUCT (ftype->next)) * 2; // Account for hidden parameter holding address of return value.

  G.p.type = AOP_INVALID;

  adjustStack (sym->stack, true, true);
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode *ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  int retsize = getSize (sym->type->next);

  D (emit2 ("; genEndFunction", ""));

  if (IFFUNC_ISNAKED(sym->type))
  {
      D (emit2 (";", "naked function: no epilogue."));
      if (options.debug && currFunc && !regalloc_dry_run)
        debugFile->writeEndFunction (currFunc, ic, 0);
      return;
  }

  /* adjust the stack for the function */
  if (sym->stack)
    adjustStack (-sym->stack, retsize == 0 || retsize > 2, retsize != 2);

  /* if debug then send end of function */
  if (options.debug && currFunc && !regalloc_dry_run)
    debugFile->writeEndFunction (currFunc, ic, 1);

  if (IFFUNC_ISISR (sym->type))
    {
      popAF ();
      cheapMove (ASMOP_P, 0, ASMOP_A, 0, true, true, true);
      popAF ();
      emit2 ("reti", "");
      cost (1, 2);
    }
  else
    {
      emit2 ("ret", "");
      cost (1, 1);
    }

  wassertl (!G.stack.pushed, "Unbalanced stack.");
}

/*-----------------------------------------------------------------*/
/* genReturn - generate code for return statement                  */
/*-----------------------------------------------------------------*/
static void
genReturn (const iCode *ic)
{
  operand *left = IC_LEFT (ic);

  D (emit2 ("; genReturn", ""));

  /* if we have no return value then
     just generate the "ret" */
  if (!left)
    goto jumpret;

  /* we have something to return then
     move the return value into place */
  aopOp (left, ic);

  wassertl (currFunc, "return iCode outside of function");

  if (left->aop->size > 2)
    {
      if (left->aop->type == AOP_STK)
        {
          for (int i = 0; i < left->aop->size; i++)
            {
              cheapMove (ASMOP_A, 0, left->aop, i, true, true, true);
              pushAF ();
              pointPStack (-4, true, true);
              emit2 ("idxm", "a, p");
              emit2 ("mov", "p, a");
              cost (2, 3);
              popAF ();
              for (int j = 0; j < i; j++)
                {
                  emit2 ("inc", "p");
                  cost (1, 1);
                }
              emit2 ("idxm", "p, a");
              cost (1, 2);
              G.p.type = AOP_INVALID;
            }
        }
      else
        {
          pointPStack (-4, true, true);
          emit2 ("idxm", "a, p");
          emit2 ("mov", "p, a");
          cost (2, 3);
          int a_litval = -1;
          for (int i = 0; i < left->aop->size; i++)
            {
              if (a_litval < 0 || !aopIsLitVal (left->aop, i, 1, a_litval))
                {
                  cheapMove (ASMOP_A, 0, left->aop, i, true, true, true);
                  if (left->aop->type == AOP_LIT)
                    a_litval = byteOfVal (left->aop->aopu.aop_lit, i);
                }
              emit2 ("idxm", "p, a");
              cost (1, 2);
              if (i + 1 < left->aop->size)
                {
                  emit2 ("inc", "p");
                  cost (1, 1);
                }
            }
        }
      goto end;
    }

  if (left->aop->size == 2 && aopInReg (left->aop, 0, P_IDX) && aopInReg (left->aop, 1, A_IDX))
    {
      emit2 ("xch", "a, p");
      cost(1, 1);
      goto end;
    }
  else if (left->aop->size == 2 && left->aop->type == AOP_STK)
    {
      genMove (ASMOP_AP, left->aop, true, true);
      goto end;
    }

  if (left->aop->size > 1)
    cheapMove (ASMOP_P, 0, left->aop, 1, true, true, true);
  if ((left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD) && !currFunc->stack)
    {
      emit2 ("ret", "%s", aopGet (left->aop, 0));
      cost (1, 2);
      freeAsmop (left);
      return;
    }
  cheapMove (ASMOP_A, 0, left->aop, 0, true, left->aop->size <= 1, true);

end:
  freeAsmop (left);  

jumpret:
  /* generate a jump to the return label
     if the next is not the return statement */
  if (!(ic->next && ic->next->op == LABEL && IC_LABEL (ic->next) == returnLabel))
    if (!currFunc->stack && !IFFUNC_ISISR (currFunc->type))
    {
      emit2 ("ret", "");
      cost (2, 1);
    }
    else
      emitJP(returnLabel, 1.0f);
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void
genLabel (const iCode *ic)
{
  D (emit2 ("; genLabel", ""));

  /* special case never generate */
  if (IC_LABEL (ic) == entryLabel)
    return;

  if (options.debug /*&& !regalloc_dry_run*/)
    debugFile->writeLabel (IC_LABEL (ic), ic);

  emitLabel (IC_LABEL (ic));

  G.p.type = AOP_INVALID;
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a jump                                      */
/*-----------------------------------------------------------------*/
static void
genGoto (const iCode *ic)
{
  D (emit2 ("; genGoto", ""));

  emitJP (IC_LABEL (ic), 1.0f);
}

/*-----------------------------------------------------------------*/
/* genPlus - generates code for addition                           */
/*-----------------------------------------------------------------*/
static void
genPlus (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genPlus", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  int size = result->aop->size;

  /* Swap if left is literal or right is in A. */
  if (left->aop->type == AOP_LIT || aopInReg (right->aop, 0, A_IDX) || right->aop->type == AOP_STL && !aopInReg (left->aop, 0, A_IDX) || right->aop->type == AOP_STK && !(aopInReg (left->aop, 0, A_IDX) || left->aop->type == AOP_STL))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  bool started = false;
  bool pushed_a = false;
  bool pushed_p = false;
  bool moved_to_a = false;

  if (!regDead(P_IDX, ic) &&
    (left->aop->type == AOP_STK || right->aop->type == AOP_STK || result->aop->type == AOP_STK))
    {
      pushPF (regDead (A_IDX, ic) && left->aop->type != AOP_REG && right->aop->type == AOP_REG);
      pushed_p = true;
    }

  bool p_dead = !aopInReg (left->aop, 0, P_IDX) && !aopInReg (left->aop, 1, P_IDX) && !aopInReg (right->aop, 0, P_IDX) && !aopInReg (right->aop, 1, P_IDX);

  for (int i = 0; i < size; i++)
    {
       if (aopInReg (left->aop, i, P_IDX) || aopInReg (right->aop, i, P_IDX))
         p_dead = true;

       if (!started && !moved_to_a && (left->aop->type == AOP_DIR || aopInReg (left->aop, i, P_IDX)) && aopIsLitVal (right->aop, i, 1, 0x01) && aopSame (left->aop, i, result->aop, i, 1))
        {
          emit2 ("inc", "%s", aopGet (left->aop, i));
          cost (1, 1);
          started = true;
          continue;
        }
       else if (!started && !moved_to_a && i + 1 == size && (left->aop->type == AOP_DIR || aopInReg (left->aop, i, P_IDX)) && aopIsLitVal (right->aop, i, 1, 0xff) && aopSame (left->aop, i, result->aop, i, 1))
        {
          emit2 ("dec", "%s", aopGet (left->aop, i));
          cost (1, 1);
          started = true;
          continue;
        }
      else if (started && !moved_to_a && (left->aop->type == AOP_DIR || aopInReg (left->aop, i, P_IDX)) && aopIsLitVal (right->aop, i, 1, 0x00) && aopSame (left->aop, i, result->aop, i, 1))
        {
          emit2 ("addc", "%s", aopGet (left->aop, i));
          cost (1, 1);
          continue;
        }
      else if (!started && !moved_to_a && right->aop->type == AOP_LIT && aopIsLitVal (right->aop, i, 1, 0x00))
        {
          cheapMove (result->aop, i, left->aop, i, regDead (A_IDX, ic), p_dead, true);
          if (aopInReg (result->aop, i, A_IDX) && i + 1 < size)
            {
              pushAF();
              pushed_a = true;
            }
          continue;
        }

      if (!(regDead (A_IDX, ic) || pushed_a))
        {
          pushAF();
          pushed_a = true;
        }

      if (!moved_to_a && (left->aop->type == AOP_DIR || aopInReg (left->aop, i, P_IDX)) && right->aop->type != AOP_STK && aopSame (left->aop, i, result->aop, i, 1))
        {
          cheapMove (ASMOP_A, 0, right->aop, i, true, p_dead && !aopInReg (left->aop, i, P_IDX), !started);
          emit2 (started ? "addc" : "add", "%s, a", aopGet (left->aop, i));
          cost (1, 1);
          started = true;
          continue;
        }
      else if (right->aop->type == AOP_STK || right->aop->type == AOP_STL && !i)
        {
          if (!regDead (P_IDX, ic) && !pushed_p)
            {
              if (pushed_a)
                {
                  cost (200, 200);
                  wassert (regalloc_dry_run);
                }
              pushPF (!moved_to_a && !aopInReg (left->aop, i, A_IDX));
              pushed_p = true;
            }
          if (!p_dead)
            pushPF (!moved_to_a && !aopInReg (left->aop, i, A_IDX));
          if (!moved_to_a)
            cheapMove (ASMOP_A, 0, left->aop, i, true, true, !started);
          cheapMove (ASMOP_P, 0, right->aop, i, false, true, !started);
          emit2 (started ? "addc" : "add", "a, p");
          cost (1, 1);
          started = true;
          if (!p_dead && i + 1 < size)
            popP (false);
          else if (!p_dead)
            popPF (false);
        }
      else if (!moved_to_a && aopInReg (left->aop, i, P_IDX))
        {
          cheapMove (ASMOP_A, 0, right->aop, i, true, false, !started);
          emit2 (started ? "addc" : "add", "a, p");
          cost (1, 1);
        }
      else if (started && (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD) && !aopIsLitVal (right->aop, i, 1, 0x00) && i + 1 == size)
        {
          if (!moved_to_a)
            cheapMove (ASMOP_A, 0, left->aop, i, true, p_dead, false);
          emit2 ("addc", "a");
          emit2 ("add", "a, %s", aopGet (right->aop, i));
          cost (2, 2);
        }
      else if (started && (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD) && !aopIsLitVal (right->aop, i, 1, 0x00))
        {
          if (!regDead (P_IDX, ic) && !pushed_p)
            {
              if (pushed_a)
                {
                  cost (200, 200);
                  wassert (regalloc_dry_run);
                }
              pushPF (!aopInReg (left->aop, i, A_IDX) && !aopInReg (right->aop, i, A_IDX) && !moved_to_a);
              pushed_p = true;
            }
          if (!p_dead)
            pushPF (!aopInReg (left->aop, i, A_IDX) && !aopInReg (right->aop, i, A_IDX) && !moved_to_a);
          cheapMove (ASMOP_P, 0, right->aop, i, !aopInReg (left->aop, i, A_IDX) && !moved_to_a, true, false);
          cheapMove (ASMOP_A, 0, left->aop, i, true, false, false);
          emit2 ("addc", "a, p");
          cost (1, 1);
          if (!p_dead && i + 1 < size)
            popP (false);
          else if (!p_dead)
            popPF (false);
        }
      else
        {
          if (!moved_to_a)
            cheapMove (ASMOP_A, 0, left->aop, i, true, p_dead && !aopInReg (right->aop, i, P_IDX), !started);
          if (started || !aopIsLitVal (right->aop, i, 1, 0x00))
            {
              if (started && aopIsLitVal (right->aop, i, 1, 0x00))
                emit2 ("addc", "a");
              else
                emit2 (started ? "addc" : "add", "a, %s", aopGet (right->aop, i));
              cost (1, 1);
              started = true;
            }
        }
      if (i + 1 < size && aopInReg (result->aop, i, P_IDX) && (left->aop->type == AOP_STK || right->aop->type == AOP_STK))
        {
          if (!pushed_a)
            {
              pushPF (true);
              pushed_p = true;
            }
          else if (regalloc_dry_run)
            cost (1000, 1000);
          else
            wassertl (0, "Unimplemented p result in addition with stack operand");
        }

      if (aopInReg (result->aop, i, A_IDX) && i + 1 < size)
        {
          pushAF();
          pushed_a = true;
        }
      else if (aopInReg (result->aop, i, P_IDX) && regDead (P_IDX, ic) && aopInReg (left->aop, i + 1, P_IDX))
        {
          emit2 ("xch", "a, p");
          cost (1, 1);
          moved_to_a = true;
        }
      else
        cheapMove (result->aop, i, ASMOP_A, 0, true, p_dead, i + 1 == size);
      if (aopInReg (result->aop, i, P_IDX))
        p_dead = false;
    }

  if (pushed_a)
    popAF();

  if (pushed_p)
    popPF (regDead (A_IDX, ic) && result->aop->type != AOP_REG);

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genMinus - generates code for minus                             */
/*-----------------------------------------------------------------*/
static void
genMinus (const iCode *ic, const iCode *ifx)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genMinus", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  if (ifx && ifx->generated)
    {
      wassert (IC_TRUE (ifx));
      wassert (left->aop->type == AOP_REG || left->aop->type == AOP_DIR);
      wassert (aopIsLitVal (right->aop, 0, 2, 1));

      emit2 ("dzsn", aopGet (left->aop, 0));
      cost (1, 1.8f);
      emitJP (IC_TRUE (ifx), 0.2f);

      for(int i = 1; i < left->aop->size; i++)
        {
          emit2 ("subc", aopGet (left->aop, i));
          emit2 ("t1sn", "f, z");
          cost (2, 2.8f);
          emitJP (IC_TRUE (ifx), 0.2f);
        }
    }
  else
    genSub (ic, result->aop, left->aop, right->aop);

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genMult - generates code for multiplication                     */
/*-----------------------------------------------------------------*/
static void
genMultLit (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  /* Swap left and right such that right is a literal */
  if (left->aop->type == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
    }

  wassert (result->aop && result->aop->size == 1 && right->aop && right->aop->type == AOP_LIT);

  cheapMove (ASMOP_A, 0, left->aop, 0, true, true, true);

  asmop *add_aop;
  if (aopInReg (left->aop, 0, P_IDX) || left->aop->type == AOP_DIR || left->aop->type == AOP_IMMD)
    add_aop = left->aop;
  else
    {
      add_aop = ASMOP_P;
      cheapMove (ASMOP_P, 0, left->aop, 0, false, true, true);
    }

  unsigned long long add, sub;
  int topbit, nonzero;

  value *bval = valueFromLit (ulFromVal (right->aop->aopu.aop_lit) & 0xff);
  wassert (!csdOfVal (&topbit, &nonzero, &add, &sub, bval));
  Safe_free (bval);

  // If the leading digits of the cse are 1 0 -1 we can use 0 1 1 instead to reduce the number of shifts.
  if (topbit >= 2 && (add & (1ull << topbit)) && (sub & (1ull << (topbit - 2))))
    {
      add = (add & ~(1u << topbit)) | (3u << (topbit - 2));
      sub &= ~(1u << (topbit - 1));
      topbit--;
    }

  for (int bit = topbit - 1; bit >= 0; bit--)
    {
      emit2 ("sl", "a");
      cost (1, 1);
      if ((add | sub) & (1ull << bit))
        {
          emit2 (add & (1ull << bit) ? "add" : "sub" , "a, %s", aopGet (add_aop, 0));
          cost (1, 1);
        }
    }

  cheapMove (result->aop, 0, ASMOP_A, 0, true, true, true);
}

/*-----------------------------------------------------------------*/
/* genMult - generates code for multiplication                     */
/*-----------------------------------------------------------------*/
static void
genMult (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genMult", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  if (left->aop->size >= 2 || right->aop->size >= 2 || result->aop->size > 2)
    wassertl (0, "Wide multiplication is to be handled via support function calls.");

  /* Swap left and right such that right is a literal */
  if (left->aop->type == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
    }

  if (right->aop->type == AOP_LIT && result->aop->size == 1)
    {
      genMultLit (ic);
      goto release;
    }

  wassert (0);

release:
  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genCmp :- greater or less than comparison                       */
/*-----------------------------------------------------------------*/
static void
genCmp (const iCode *ic, iCode *ifx)
{
  operand *left, *right, *result;

  D (emit2 ("; genCmp", ""));

  result = IC_RESULT (ic);
  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  int size = max (left->aop->size, right->aop->size);
  bool sign = false;
  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    sign = !(SPEC_USIGN (operandType (left)) | SPEC_USIGN (operandType (right)));

  // Non-destructive 1-byte unsigned comparison.
  if (!sign && size == 1 && ifx && aopInReg (left->aop, 0, A_IDX))
    {
      if (ic->op == '>' && right->aop->type == AOP_LIT)
        {
          wassert (!aopIsLitVal (right->aop, 0, 1, 0x00));
          
          if (IC_TRUE (ifx))
            {
              emit2 ("ceqsn", "a, #0x%02x", byteOfVal (right->aop->aopu.aop_lit, 0) + 1);
              emit2 ("t1sn", "f, c");
              cost (2, 2.5);
            }
          else
            {
              emit2 ("ceqsn", "a, #0x%02x", byteOfVal (right->aop->aopu.aop_lit, 0) + 1);
              emit2 ("nop", "");
              emit2 ("t0sn", "f, c");
              cost (3, 3.5);
            }
          emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.5f);
          goto release;
        }
      else if (ic->op == '<' && aopInReg (left->aop, 0, A_IDX) && (right->aop->type == AOP_LIT || right->aop->type == AOP_DIR || aopInReg (right->aop, 0, P_IDX)) && (IC_TRUE (ifx) || !regDead (A_IDX, ic)))
        {
          if (IC_FALSE (ifx))
            {
              emit2 ("ceqsn", "a, %s", aopGet (right->aop, 0));
              emit2 ("t1sn", "f, c");
              cost (2, 2.5);
            }
          else if ((TARGET_IS_PDK15 || TARGET_IS_PDK16) && (right->aop->type == AOP_DIR || aopInReg (right->aop, 0, P_IDX)))
            {
              emit2 ("comp", "a, %s", aopGet (right->aop, 0));
              emit2 ("t0sn", "f, c");
              cost (2, 2.5);
            }
          else
            {
              emit2 ("ceqsn", "a, %s", aopGet (right->aop, 0));
              emit2 ("nop", "");
              emit2 ("t0sn", "f, c");
              cost (3, 3.5);
            }
          emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.5f);
          goto release;
        }
    }

  if (ic->op == '>')
    {
      operand *t = right;
      right = left;
      left = t;  
    }

  bool started = false;
  for (int i = 0; i < size; i++)
    {
      if (!started && sign && aopIsLitVal (right->aop, i, 1, 0x00) && i + 1 == size)
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true, true, true);
          if (ifx)
            {
               if (IC_FALSE (ifx))
                 {
                   if (aopInReg (left->aop, i, A_IDX) && !regDead (A_IDX, ic))
                     {
                       emit2 ("ceqsn", "a, #0x80");
                       emit2 ("nop", "");
                       cost (2, 2);
                     }
                   else
                     {
                       emit2 ("sub", "a, #0x80");
                       cost (1, 1);
                     }
                   emit2 ("t0sn", "f, c");
                   cost (1, 1.5);
                 }
               else
                 {
                   emit2 ("ceqsn", "a, #0x80");
                   emit2 ("t1sn", "f, c");
                   cost (2, 2.5);
                 }

               emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.5f);
            }
          else
            {
              emit2 ("sl", "a");
              emit2 ("mov", "a, #0x00");
              emit2 ("slc", "a");
              cheapMove (result->aop, 0, ASMOP_A, 0, true, true, true);
            }
          goto release;
        }

      if (!started && aopIsLitVal (right->aop, i, 1, 0x00) && i + 1 < size)
        ;
      else if (started && aopIsLitVal (right->aop, i, 1, 0x00))
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true, true, !i);
          emit2 ("subc", "a");
          cost (1, 1);
        }
      else if (started && (right->aop->type == AOP_LIT && !aopIsLitVal (right->aop, i, 1, 0x00) || right->aop->type == AOP_IMMD)) // Work around lack of subc a, #nn.
        {
          if (left->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_A, 0, left->aop, i, true, true, !i);
              cheapMove (ASMOP_P, 0, right->aop, i, false, true, !i);
            }
          else
            {
              cheapMove (ASMOP_P, 0, right->aop, i, true, true, !i);
              cheapMove (ASMOP_A, 0, left->aop, i, true, false, !i);
            }
          emit2 ("subc", "a, p");
          cost (1, 1);
        }
      else if (right->aop->type == AOP_STK)
        {
          if (!regDead (P_IDX, ic) || aopInReg (left->aop, i + 1, P_IDX))
            pushPF (!aopInReg (left->aop, i, A_IDX));
          cheapMove (ASMOP_A, 0, left->aop, i, true, true, !i);
          cheapMove (ASMOP_P, 0, right->aop, i, false, true, !i);
          emit2 (started ? "subc" : "sub", "a, p");
          cost (1, 1);
          started = true;
          if (!regDead (P_IDX, ic) || aopInReg (left->aop, i + 1, P_IDX))
            {
              if (i + 1 == size)
                popPF (false);
              else
                popP (false);
            }
        }
      else if (!started && (!sign && ifx || i + 1 < size) &&
        (TARGET_IS_PDK15 || TARGET_IS_PDK16) &&
        aopInReg (right->aop, i, A_IDX) && (left->aop->type == AOP_IMMD || aopInReg (left->aop, i, P_IDX)))
        {
          emit2 ("comp", "%s, a", aopGet (left->aop, i));
          cost (1, 1);
        }
      else
        {
          if (aopInReg (right->aop, i, A_IDX))
            {
              cost (100, 100);
              wassert (regalloc_dry_run);
            }
          cheapMove (ASMOP_A, 0, left->aop, i, true, !aopInReg (right->aop, i, P_IDX), !i);
          emit2 (started ? "subc" : "sub", "a, %s", aopGet (right->aop, i));
          cost (1, 1);
          started = true;
        }
    }

  if (sign)
    {
      emit2 ("t0sn", "f, ov");
      emit2 ("xor", "a, #0x80");
      emit2 ("sl", "a");
      cost (3, 3);
    }

  if (ifx)
    {
      emit2 (IC_FALSE(ifx) ? "t1sn" : "t0sn", "f, c");
      cost (1, 1.5);
      emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.5f);
    }
  else
    {
      emit2 ("mov", "a, #0x00");
      emit2 ("slc", "a");
      cost (2, 2);
      cheapMove (result->aop, 0, ASMOP_A, 0, true, true, true);
    }

release:
  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genCmpEQorNE - equal or not equal comparison                    */
/*-----------------------------------------------------------------*/
static void
genCmpEQorNE (const iCode *ic, iCode *ifx)
{
  operand *left, *right, *result;

  D (emit2 ("; genCmpEQorNE", ""));

  result = IC_RESULT (ic);
  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  symbol *lbl_ne = 0;
  symbol *endlbl = 0;

  if (ifx)
    {
      if ((ic->op == EQ_OP) ^ (bool)(IC_FALSE (ifx)))
        lbl_ne = regalloc_dry_run ? 0 : newiTempLabel (NULL);
      else
        lbl_ne = IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx);
    }
  else if (!regalloc_dry_run)
    {
      lbl_ne = newiTempLabel (NULL);
      endlbl = newiTempLabel (NULL);
    }

  int size = max (left->aop->size, right->aop->size);

  if (left->aop->type == AOP_LIT || aopInReg (right->aop, 0, A_IDX) || aopInReg (right->aop, 1, A_IDX) ||
     !aopInReg (left->aop, 0, A_IDX) && (right->aop->type == AOP_SFR || right->aop->type == AOP_STK || right->aop->type == AOP_CODE || right->aop->type == AOP_STL))
    {
      operand *temp = left;
      left = right;
      right = temp;
    }

  if (aopInReg (left->aop, 1, A_IDX) && (right->aop->type == AOP_LIT || right->aop->type == AOP_DIR || right->aop->type == AOP_IMMD))
    {
      wassert (regDead (A_IDX, ic));

      emit2 ("ceqsn", "a, %s", aopGet (right->aop, 1));
      cost (1, 1);
      emitJP (lbl_ne, 0.0f);
      cheapMove (ASMOP_A, 0, left->aop, 0, true, true, true);
      if (ifx && ((ic->op == EQ_OP) ^ (bool)(IC_FALSE(ifx))))
        {
          if (TARGET_IS_PDK13) // pdk13 does not have cneqsn
            {
              symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
              emit2 ("ceqsn", "a, %s", aopGet (right->aop, 0));
              emitJP (tlbl, 0.0f);
              cost (2, 3);
              emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.0f);
              emitLabel (tlbl);
            }
          else
            {
              emit2 ("cneqsn", "a, %s", aopGet (right->aop, 0));
              cost (1, 1);
              emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.0f);
            }
        }
      else
        {
          emit2 ("ceqsn", "a, %s", aopGet (right->aop, 0));
          cost (1, 1);
          emitJP (lbl_ne, 0.0f);
        }
    }
  else
    for (int i = 0; i < size; i++)
      {
        if (left->aop->type == AOP_STK && aopInReg (right->aop, i, P_IDX))
          {
            if (i + 1 < size && aopInReg (right->aop, i + 1, A_IDX))
              {
                cost (250, 250);
                wassert (regalloc_dry_run);
              }
            cheapMove (ASMOP_A, 0, right->aop, i, true, true, true);
            cheapMove (ASMOP_P, 0, left->aop, i, false, true, true);
          }
        else
          {
            if (i + 1 < size && aopInReg (left->aop, i + 1, A_IDX))
              {
                cost (250, 250);
                wassert (regalloc_dry_run);
              }
            cheapMove (ASMOP_A, 0, left->aop, i, true, true, true);
            if (right->aop->type == AOP_SFR || right->aop->type == AOP_STK || right->aop->type == AOP_CODE || right->aop->type == AOP_STL && !i)
              {
                if (!regDead (P_IDX, ic) || i + 1 < size && aopInReg(left->aop, i + 1, P_IDX))
                  {
                    cost (1000, 1000);
                    wassert (regalloc_dry_run);
                  }
                if (aopInReg (left->aop, i, P_IDX))
                  {
                    emit2 ("mov", "a, p");
                    cost (1, 1);    
                  }
                cheapMove (ASMOP_P, 0, right->aop, i, false, true, true);
              }
          }
  
        if (ifx && i + 1 == size && ((ic->op == EQ_OP) ^ (bool)(IC_FALSE(ifx))))
          {
            if (TARGET_IS_PDK13)
              {
                symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
                emit2 ("ceqsn", "a, %s", (right->aop->type == AOP_SFR || right->aop->type == AOP_STK || right->aop->type == AOP_CODE || right->aop->type == AOP_STL && !i) ? "p" : aopGet (right->aop, i));
                emitJP (tlbl, 0.0f);
                cost (2, 3);
                emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.0f);
                emitLabel (tlbl);
              }
            else
              {
                emit2 ("cneqsn", "a, %s", (right->aop->type == AOP_SFR || right->aop->type == AOP_STK || right->aop->type == AOP_CODE || right->aop->type == AOP_STL && !i) ? "p" : aopGet (right->aop, i));
                cost (1, 1);
                emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.0f);
              }
          }
        else
          {
            emit2 ("ceqsn", "a, %s", (right->aop->type == AOP_SFR || right->aop->type == AOP_STK || right->aop->type == AOP_CODE || right->aop->type == AOP_STL && !i) ? "p" : aopGet (right->aop, i));
            cost (1, 1);
            emitJP(lbl_ne, 0.0f);
          }
      }

  if (ifx) // Jump condition only.
    {
      if ((ic->op == EQ_OP) ^ (bool)(IC_FALSE(ifx)))
        {
          emitLabel (lbl_ne);
          G.p.type = AOP_INVALID;
        }
    }
  else // Needs result
    {
      if (!regDead (A_IDX, ic))
        pushAF();
      cheapMove (result->aop, 0, ic->op == EQ_OP ? ASMOP_ONE : ASMOP_ZERO, 0, true, true, true);
      emitJP(endlbl, 0.0f);
      emitLabel (lbl_ne);
      G.p.type = AOP_INVALID;
      if (!regDead (A_IDX, ic))
        {
          emit2 ("push", "af");
          cost (1, 1);
        }
      cheapMove (result->aop, 0, ic->op == NE_OP ? ASMOP_ONE : ASMOP_ZERO, 0, true, true, true);
      emitLabel (endlbl);
      G.p.type = AOP_INVALID;
      if (!regDead (A_IDX, ic))
        popAF();
    }

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

static void
genXorByte (const asmop *result_aop, const asmop *left_aop, const asmop *right_aop, int i, bool *pushed_a, bool a_dead, bool p_dead)
{
  if ((aopInReg (left_aop, i, A_IDX) || aopInReg (left_aop, i, P_IDX) || left_aop->type == AOP_DIR) &&
    aopIsLitVal (right_aop, i, 1, 0xff) && aopSame (result_aop, i, left_aop, i, 1))
    {
      emit2 ("not", "%s", aopGet (left_aop, i));
      cost (1, 1);
    }
  else if (aopIsLitVal (right_aop, i, 1, 0x00))
    {
      cheapMove (result_aop, i, left_aop, i, a_dead, p_dead, true);
    }
  else
    {
      if (!a_dead && !*pushed_a)
        {
          pushAF();
          *pushed_a = true;
        }

      if ((left_aop->type == AOP_DIR || aopInReg (left_aop, i, P_IDX)) && aopSame (left_aop, i, result_aop, i, 1))
        {
          cheapMove (ASMOP_A, 0, right_aop, i, true, p_dead, true);
          emit2 ("xor", "%s, a", aopGet (left_aop, i));
          cost (1, 1);
        }
      else if ((right_aop->type == AOP_DIR || aopInReg (right_aop, i, P_IDX)) && aopSame (right_aop, i, result_aop, i, 1))
        {
          cheapMove (ASMOP_A, 0, left_aop, i, true, true, true);
          emit2 ("xor", "%s, a", aopGet (right_aop, i));
          cost (1, 1);
        }
      else if (right_aop->type == AOP_STK || right_aop->type == AOP_STL)
        {
          if (!p_dead || aopInReg (left_aop, i, P_IDX))
            {
              cost (100, 100);
              wassert (regalloc_dry_run);
            }
          cheapMove (ASMOP_A, 0, left_aop, i, true, true, true);
          cheapMove (ASMOP_P, 0, right_aop, i, false, true, true);
          emit2 ("xor", "a, p");
          cost (1, 1);
          cheapMove (result_aop, i, ASMOP_A, 0, true, true, true);
        }
      else
        {
          cheapMove (ASMOP_A, 0, left_aop, i, true, p_dead, true);
          emit2 ("xor", "a, %s", aopGet (right_aop, i));
          cost (1, 1);
          cheapMove (result_aop, i, ASMOP_A, 0, true, p_dead, true);
        }
    }
}

/*-----------------------------------------------------------------*/
/* genXor - code for xor                                           */
/*-----------------------------------------------------------------*/
static void
genXor (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genXor", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  int size = result->aop->size;
  int skip_byte = -1;

  /* Swap if left is literal or right is in A. */
  if (left->aop->type == AOP_LIT || aopInReg (right->aop, 0, A_IDX) || aopInReg (right->aop, 1, A_IDX) || right->aop->type == AOP_STK && !aopInReg (left->aop, 0, A_IDX))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  bool a_free = regDead (A_IDX, ic);
  bool p_free = regDead (P_IDX, ic);
  bool pushed_a = false;

  for (int i = 0; i < size; i++)
    if (aopInReg (left->aop, i, A_IDX))
      {
        genXorByte (result->aop, left->aop, right->aop, i, &pushed_a, a_free, p_free);
        skip_byte = i;

        if (aopInReg (result->aop, i, A_IDX))
          a_free = false;
        if (aopInReg (result->aop, i, P_IDX))
          p_free = false;
      }

  for (int i = 0; i < size; i++)
    {
      if (i == skip_byte)
        continue;

      genXorByte (result->aop, left->aop, right->aop, i, &pushed_a, a_free, p_free);

      if (aopInReg (result->aop, i, A_IDX))
        a_free = false;
      if (aopInReg (result->aop, i, P_IDX))
        p_free = false;
    }

  if (pushed_a)
    popAF();

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genOr - code for or                                             */
/*-----------------------------------------------------------------*/
static void
genOr (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genOr", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  int size = result->aop->size;

  /* Swap if left is literal or right is in A. */
  if (left->aop->type == AOP_LIT || aopInReg (right->aop, 0, A_IDX)  || right->aop->type == AOP_STK && !aopInReg (left->aop, 0, A_IDX))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  bool p_free = regDead (P_IDX, ic) && !aopInReg (left->aop, 0, P_IDX) && !aopInReg (left->aop, 1, P_IDX) && !aopInReg (right->aop, 0, P_IDX) && !aopInReg (right->aop, 1, P_IDX);

  for (int i = 0; i < size; i++)
    {
      if (regDead (P_IDX, ic) && (aopInReg (left->aop, i, P_IDX) || aopInReg (right->aop, i, P_IDX)))
        p_free = true;

      int bit = right->aop->type == AOP_LIT ? isLiteralBit (byteOfVal (right->aop->aopu.aop_lit, i)) : -1;

      if (aopIsLitVal (right->aop, i, 1, 0x00))
        {
          cheapMove (result->aop, i, left->aop, i, true, p_free, true);
        }
      else if ((left->aop->type == AOP_SFR || aopInReg (left->aop, i, P_IDX)) && aopSame (left->aop, i, result->aop, i, 1) && bit >= 0)
        {
          emit2 ("set1", "%s, #%d", aopGet (left->aop, i), bit);
          cost (1, 1);
        }
      else if ((left->aop->type == AOP_DIR || aopInReg (left->aop, i, P_IDX) && right->aop->type != AOP_STK) && aopSame (left->aop, i, result->aop, i, 1))
        {
          cheapMove (ASMOP_A, 0, right->aop, i, true, p_free && !aopInReg (left->aop, i, P_IDX), true);
          emit2 ("or", "%s, a", aopGet (left->aop, i));
          cost (1, 1);
        }
      else if ((right->aop->type == AOP_DIR || aopInReg (right->aop, i, P_IDX) && left->aop->type != AOP_STK) && aopSame (right->aop, i, result->aop, i, 1))
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true, p_free && !aopInReg (right->aop, i, P_IDX), true);
          emit2 ("or", "%s, a", aopGet (right->aop, i));
          cost (1, 1);
        }
      else if (right->aop->type == AOP_STK)
        {
          if (!p_free)
            pushPF (!aopInReg (right->aop, i, A_IDX));
          cheapMove (ASMOP_A, 0, left->aop, i, true, p_free && !aopInReg (right->aop, i, P_IDX), true);
          cheapMove (ASMOP_P, 0, right->aop, i, false, true, true);
          emit2 ("or", "a, p");
          cost (1, 1);
          cheapMove (result->aop, i, ASMOP_A, 0, true, true, true);
          if (!p_free)
            popPF (!aopInReg (result->aop, i, A_IDX));
        }
      else
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true, p_free && !aopInReg (right->aop, i, P_IDX), true);
          emit2 ("or", "a, %s", aopGet (right->aop, i));
          cost (1, 1);
          cheapMove (result->aop, i, ASMOP_A, 0, true, p_free, true);
        }

      if (aopInReg (result->aop, i, P_IDX))
        p_free = false;
    }

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genAnd - code for and                                           */
/*-----------------------------------------------------------------*/
static void
genAnd (const iCode *ic, iCode *ifx)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genAnd", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  int size = result->aop->size;

  /* Swap if left is literal or right is in A. */
  if (left->aop->type == AOP_LIT || aopInReg (right->aop, 0, A_IDX) || right->aop->type == AOP_STK)
    {
      operand *t = right;
      right = left;
      left = t;
    }

  if (ifx && IC_FALSE (ifx) && result->aop->type == AOP_CND)
    {
      int i, j, nonzero;

      // Find the non-zero byte.
      if (right->aop->type != AOP_LIT)
        {
          wassert (right->aop->size == 1);
          i = 0;
          nonzero = 1;
        }
      else
        for (j = 0, nonzero = 0, i = 0; j < left->aop->size; j++)
          if (byteOfVal (right->aop->aopu.aop_lit, j))
            {
              i = j;
              nonzero++;
            }

      wassertl (nonzero <= 1, "Code generation for jump on bitwise and can handle at most one nonzero byte");

      int bit = right->aop->type == AOP_LIT ? isLiteralBit (byteOfVal (right->aop->aopu.aop_lit, i)) : - 1;

      if (aopInReg (left->aop, i, P_IDX) && bit >= 0)
        {
          emit2 (IC_FALSE  (ifx) ? "t1sn" : "t0sn", "p, #%d", bit);
          cost (1, 1.5);
        }
      else if (aopInReg (left->aop, i, P_IDX) && regDead (P_IDX, ic) &&
        (byteOfVal (right->aop->aopu.aop_lit, i) == 0x7f || byteOfVal (right->aop->aopu.aop_lit, i) == 0xfe))
        {
          emit2 (byteOfVal (right->aop->aopu.aop_lit, 0) == 0x7f ? "sl" : "sr", "p");
          emit2 (IC_FALSE  (ifx) ? "t0sn" : "t1sn", "f, z");
          cost (2, 2.5);
        }
      else
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true, true, true);
          if (!aopIsLitVal (right->aop, i, 1, 0xff))
            {
              emit2 ("and", "a, %s", aopGet (right->aop, i));
              cost (1, 1);
            }
          if (TARGET_IS_PDK13 && IC_FALSE (ic)) // pdk13 does not have cneqsn
            {
              symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (0));
              emit2 ("ceqsn", "a, #0x00");
              cost (1, 1.5);
              emitJP (tlbl, 0.5f);
              emitJP (IC_FALSE (ifx), 0.5f);
              emitLabel (tlbl);
              goto release;
            }
          else
            {
              emit2 (IC_FALSE  (ifx) ? "cneqsn" : "ceqsn", "a, #0x00");
              cost (1, 1.5);
            }
        }

      emitJP (IC_FALSE (ifx) ? IC_FALSE (ifx) : IC_TRUE (ifx), 0.5f);

      goto release;
    }
  else if (ifx && IC_TRUE (ifx) && result->aop->type == AOP_CND)
    {
      for (int i = 0; i < right->aop->size; i++)
        {
          int bit = right->aop->type == AOP_LIT ? isLiteralBit (byteOfVal (right->aop->aopu.aop_lit, i)) : - 1;

          if (aopIsLitVal (right->aop, i, 1, 0x00))
            continue;
          else if (aopInReg (left->aop, i, P_IDX) && bit >= 0)
            {
              emit2 ("t0sn", "p, #%d", bit);
              cost (1, 1.5);
            }
          else if (aopInReg (left->aop, i, P_IDX) && regDead (P_IDX, ic) &&
            (byteOfVal (right->aop->aopu.aop_lit, i) == 0x7f || byteOfVal (right->aop->aopu.aop_lit, i) == 0xfe))
            {
              emit2 (byteOfVal (right->aop->aopu.aop_lit, 0) == 0x7f ? "sl" : "sr", "p");
              emit2 ("t1sn", "f, z");
              cost (2, 2.5);
            }
          else if (left->aop->type == AOP_DIR && right->aop->type == AOP_LIT) // Try to combine multiple bytes with 0xff mask by or.
            {
              int j, k;
              for(j = i; j + 1 < right->aop->size && (aopIsLitVal (right->aop, j, 1, 0xff) || aopIsLitVal (right->aop, j, 1, 0x00)); j++);
              cheapMove (ASMOP_A, 0, left->aop, j, true, true, true);
              if (!(aopIsLitVal (right->aop, j, 1, 0xff) || aopIsLitVal (right->aop, j, 1, 0x00)))
                {
                  emit2 ("and", "a, %s", aopGet (right->aop, j));
                  cost (1, 1);
                }
              for (k = i; k < right->aop->size; k++)
                {
                  if (k == j || aopIsLitVal (right->aop, k, 1, 0x00))
                    continue;
                  if (!aopIsLitVal (right->aop, k, 1, 0xff))
                    break;
                  emit2 ("or", "a, %s", aopGet (left->aop, k));
                  cost (1, 1);
                }
              i = k - 1;
              emit2 ("ceqsn", "a, #0x00");
              cost (1, 1.5);
            }
          else
            {
              cheapMove (ASMOP_A, 0, left->aop, i, true, true, true);
              if (!aopIsLitVal (right->aop, i, 1, 0xff))
                {
                  emit2 ("and", "a, %s", aopGet (right->aop, i));
                  cost (1, 1);
                }
              emit2 ("ceqsn", "a, #0x00");
              cost (1, 1.5);
            }

          emitJP (IC_TRUE (ifx), 0.5f);
        }

      goto release;
    }

  bool p_dead = regDead (P_IDX, ic) && !aopInReg (left->aop, 0, P_IDX) && !aopInReg (left->aop, 1, P_IDX) && !aopInReg (right->aop, 0, P_IDX) && !aopInReg (right->aop, 1, P_IDX);

  for (int i = 0; i < size; i++)
    {
      if (regDead (P_IDX, ic) && (aopInReg (left->aop, i, P_IDX) || aopInReg (right->aop, i, P_IDX)))
        p_dead = true;

      if (aopInReg (right->aop, i, A_IDX))
        {
          operand *t = right;
          right = left;
          left = t;
        }

      int bit = right->aop->type == AOP_LIT ? isLiteralBit (~byteOfVal (right->aop->aopu.aop_lit, i) & 0xff) : -1;

      if (aopIsLitVal (right->aop, i, 1, 0xff))
        cheapMove (result->aop, i, left->aop, i, true, p_dead, true);
      else if (aopIsLitVal (right->aop, i, 1, 0x00))
        {
          int zend;
          for (zend = i; zend + 1 < size && aopIsLitVal (right->aop, zend + 1, 1, 0x00); zend++);
          genMove_o (result->aop, i, ASMOP_ZERO, 0, zend + 1 - i, true, p_dead);
          i = zend;
        }
      else if ((left->aop->type == AOP_SFR || aopInReg (left->aop, i, P_IDX)) && aopSame (left->aop, i, result->aop, i, 1) && bit >= 0)
        {
          emit2 ("set0", "%s, #%d", aopGet (left->aop, i), bit);
          cost (1, 1);
        }
      else if ((left->aop->type == AOP_DIR || aopInReg (left->aop, i, P_IDX) && right->aop->type != AOP_STK) && aopSame (left->aop, i, result->aop, i, 1))
        {
          cheapMove (ASMOP_A, 0, right->aop, i, true, p_dead && !aopInReg (left->aop, i, P_IDX), true);
          emit2 ("and", "%s, a", aopGet (left->aop, i));
          cost (1, 1);
        }
      else if ((right->aop->type == AOP_DIR || aopInReg (right->aop, i, P_IDX) && left->aop->type != AOP_STK) && aopSame (right->aop, i, result->aop, i, 1))
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true, p_dead && !aopInReg (right->aop, i, P_IDX), true);
          emit2 ("and", "%s, a", aopGet (right->aop, i));
          cost (1, 1);
        }
      else if (right->aop->type == AOP_STK)
        {
          if (!p_dead)
            pushPF (!aopInReg (left->aop, i, A_IDX));
          cheapMove (ASMOP_A, 0, left->aop, i, true, !aopInReg (right->aop, i, P_IDX), true);
          cheapMove (ASMOP_P, 0, right->aop, i, false, true, true);
          emit2 ("and", "a, p");
          cost (1, 1);
          cheapMove (result->aop, i, ASMOP_A, 0, true, true, true);
          if (!p_dead)
            popPF (!aopInReg (result->aop, i, A_IDX));
        }
      else
        {
          cheapMove (ASMOP_A, 0, left->aop, i, true, p_dead && !aopInReg (right->aop, i, P_IDX), true);
          emit2 ("and", "a, %s", aopGet (right->aop, i));
          cost (1, 1);
          cheapMove (result->aop, i, ASMOP_A, 0, true, p_dead, true);
        }

      if (aopInReg (result->aop, i, P_IDX))
        p_dead = false;
    }

release:
  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genLeftShift - generates code for right shifting               */
/*-----------------------------------------------------------------*/
static void
genLeftShift (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genLeftShift", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  int size = result->aop->size;

  bool pushed_p = false;
  bool p_dead = regDead (P_IDX, ic);

  if (result->aop->type == AOP_STK && !p_dead)
    {
      pushPF (regDead (A_IDX, ic) && !aopInReg (left->aop, 0, A_IDX) && !aopInReg (right->aop, 0, A_IDX));
      pushed_p = true;
      p_dead = true;
    }

  if (right->aop->type == AOP_LIT)
    {
      int shCount = ulFromVal (right->aop->aopu.aop_lit);
      int offset = shCount / 8;

      genMove_o (result->aop, offset, left->aop, 0, result->aop->size - shCount / 8, regDead (A_IDX, ic), p_dead);
      genMove_o (result->aop, 0, ASMOP_ZERO, 0, offset, regDead (A_IDX, ic), p_dead);
      shCount %= 8;

      if (!shCount)
        goto release;

      bool loop = (shCount > 2 + ((size - offset) <= 1) * 2 + optimize.codeSpeed) && !(size == 1 && aopInReg (result->aop, 0, A_IDX)) &&
        regDead (A_IDX, ic) && !aopInReg (result->aop, 0, A_IDX) && !aopInReg (result->aop, 1, A_IDX);
      symbol *tlbl = (!loop || regalloc_dry_run) ? 0 : newiTempLabel (0);

      if (loop)
        {
          emit2 ("mov", "a, #%d", shCount);
          cost (1, 1);
          emitLabel (tlbl);
          G.p.type = AOP_INVALID;
          regalloc_dry_run_cycle_scale = shCount;
          shCount = 1;
        }

      if (result->aop->type == AOP_STK && (!regDead (A_IDX, ic) || loop))
        pushAF ();

      while (shCount)
        {
          if (shCount == 7 && (size - offset) == 1 && (result->aop->type == AOP_REG || result->aop->type == AOP_DIR))
            {
              emit2 ("sr", "%s", aopGet (result->aop, offset));
              if (aopInReg (result->aop, 0, A_IDX))
                emit2("mov", "a, #0x00");
              else
                emit2 ("clear", "%s", aopGet (result->aop, offset));
              emit2 ("src", "%s", aopGet (result->aop, offset));
              shCount = 0;
              continue;
            }
          else if (shCount >= 4 && (size - offset) == 1 && aopInReg (result->aop, offset, A_IDX))
            {
              emit2 ("swap", "a");
              emit2 ("and", "a, #0xf0");
              cost (2, 2);
              shCount -= 4;
              continue;
            }
          else if ((size - offset) == 1 && result->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_A, 0, result->aop, offset, true, p_dead, true);
              if (shCount >= 4)
                {
                  emit2 ("swap", "a");
                  emit2 ("and", "a, #0xf0");
                  cost (2, 2);
                  shCount -= 4;
                }
              for (;shCount; shCount--)
                {
                  emit2 ("sl", "a");
                  cost (1, 1);
                }
              cheapMove (result->aop, offset, ASMOP_A, 0, true, p_dead, true);
              continue;
            }

          for (int i = offset; i < size; i++)
            {
              if (result->aop->type == AOP_STK)
                {
                  cheapMove (ASMOP_A, 0, result->aop, i, true, p_dead, i <= offset);
                  emit2 ((i > offset) ? "slc" : "sl", "a");
                  cost (1, 1);
                  cheapMove (result->aop, i, ASMOP_A, 0, true, p_dead, i + 1 != size);
                }
              else
                {
                  emit2 ((i > offset) ? "slc" : "sl", "%s", aopGet (result->aop, i));
                  cost (1, 1);
                }
            }
          shCount--;
        }

      if (result->aop->type == AOP_STK && (!regDead (A_IDX, ic) || loop))
        popAF ();

      if (loop)
        {
          emit2 ("dzsn", "a");
          if (!regalloc_dry_run)
            emit2 ("goto", "!tlabel", labelKey2num (tlbl->key));
          cost (2, 2);
        }

      regalloc_dry_run_cycle_scale = 1;
    }
  else
    {
      bool pushed_counter = false;

      // Avoid overwriting counter by result.
      if (right->aop->type == AOP_STK && result->aop->type == AOP_STK && right->aop->aopu.stk_off >= result->aop->aopu.stk_off && right->aop->aopu.stk_off < result->aop->aopu.stk_off + result->aop->size)
        {
          cheapMove (ASMOP_A, 0, right->aop, 0, true, true, true);
          pushAF ();
          pushed_counter = true;
        }
      genMove (result->aop, left->aop, !aopInReg (right->aop, 0, A_IDX), p_dead && !aopInReg (right->aop, 0, P_IDX));

      symbol *tlbl1 = regalloc_dry_run ? 0 : newiTempLabel (0);
      symbol *tlbl2 = regalloc_dry_run ? 0 : newiTempLabel (0);

      if (pushed_counter)
        popAF ();
      else
        cheapMove (ASMOP_A, 0, right->aop, 0, true, true, true);
      emitLabel (tlbl1);
      G.p.type = AOP_INVALID;
      emit2 ("sub", "a, #1");
      emit2 ("t0sn", "f, c");
      if (!regalloc_dry_run)
        emit2 ("goto", "!tlabel", labelKey2num (tlbl2->key));
      cost (3, 3);
    
      for(int i = 0; i < size; i++)
        {
          if (result->aop->type == AOP_STK && size == 1)
            {
              pushAF ();
              cheapMove (ASMOP_A, 0, result->aop, i, true, p_dead, true);
              emit2("sl", "a");
              cost (1, 1);
              cheapMove (result->aop, i, ASMOP_A, 0, true, p_dead, true);
              popAF ();
            }
          else if (result->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_P, 0, result->aop, i, false, p_dead, !i);
              emit2(i ? "slc" : "sl", "p");
              cost (1, 1);
              cheapMove (result->aop, i, ASMOP_P, 0, false, p_dead, i + 1 == size);
            }
          else
            {
              emit2(i ? "slc" : "sl", "%s", aopGet (result->aop, i));
              cost (1, 1);
            }
        }
    
      if (!regalloc_dry_run)
        emit2 ("goto", "!tlabel", labelKey2num (tlbl1->key));
      cost (1, 1);

      emitLabel (tlbl2);
      G.p.type = AOP_INVALID;
    }

release:

  if (pushed_p)
    popPF (regDead (A_IDX, ic));

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genRightShift - generates code for right shifting               */
/*-----------------------------------------------------------------*/
static void
genRightShift (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genRightShift", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  bool pushed_a = false;
  bool pushed_p = false;

  int size = result->aop->size;

  bool p_dead = regDead (P_IDX, ic);

  if (result->aop->type == AOP_STK && !p_dead)
    {
      pushPF (regDead (A_IDX, ic) && !aopInReg (left->aop, 0, A_IDX) && !aopInReg (right->aop, 0, A_IDX));
      pushed_p = true;
      p_dead = true;
    }

  if (right->aop->type == AOP_LIT)
    {
      int shCount = ulFromVal (right->aop->aopu.aop_lit);

      if (SPEC_USIGN (getSpec (operandType (left))))
        {
          genMove_o (result->aop, 0, left->aop, shCount / 8, result->aop->size, regDead (A_IDX, ic), true);
          size -= shCount / 8;
          shCount %= 8;
        }
      else
        genMove (result->aop, left->aop, !aopInReg (right->aop, 0, A_IDX), true);

      if (!shCount)
        goto release;

      bool loop = (shCount > 2 + (size == 1) * 2 + optimize.codeSpeed) && !(size == 1 && aopInReg (result->aop, 0, A_IDX)) &&
        regDead (A_IDX, ic) && !aopInReg (result->aop, 0, A_IDX) && !aopInReg (result->aop, 1, A_IDX);

      symbol *tlbl = (!loop || regalloc_dry_run) ? 0 : newiTempLabel (0);

      if (loop)
        {
          emit2 ("mov", "a, #%d", shCount);
          cost (1, 1);
          emitLabel (tlbl);
          G.p.type = AOP_INVALID;
          regalloc_dry_run_cycle_scale = shCount;
          shCount = 1;
        }

      while (shCount)
        {
          if (SPEC_USIGN (getSpec (operandType (left))) && shCount == 7 && size == 1 && (result->aop->type == AOP_REG || result->aop->type == AOP_DIR))
            {
              emit2 ("sl", "%s", aopGet (result->aop, 0));
              if (aopInReg (result->aop, 0, A_IDX))
                emit2("mov", "a, #0x00");
              else
                emit2 ("clear", "%s", aopGet (result->aop, 0));
              emit2 ("slc", "%s", aopGet (result->aop, 0));
              shCount = 0;
              continue;
            }
          else if (SPEC_USIGN (getSpec (operandType (left))) && shCount >= 4 && size == 1 && aopInReg (result->aop, 0, A_IDX))
            {
              emit2 ("swap", "a");
              emit2 ("and", "a, #0x0f");
              cost (2, 2);
              shCount -= 4;
              continue;
            }

          if ((!SPEC_USIGN (getSpec (operandType (left))) || result->aop->type == AOP_STK) && (loop || !regDead (A_IDX, ic)))
            {
              pushAF();
              pushed_a = true;
            }

          if (SPEC_USIGN (getSpec (operandType (left))) && size == 1 && result->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_A, 0, result->aop, 0, true, p_dead, true);
              if (shCount >= 4)
                {
                  emit2 ("swap", "a");
                  emit2 ("and", "a, #0x0f");
                  cost (2, 2);
                  shCount -= 4;
                }
              for (;shCount; shCount--)
                {
                  emit2 ("sr", "a");
                  cost (1, 1);
                }
              cheapMove (result->aop, 0, ASMOP_A, 0, true, p_dead, true);
              continue;
            }

          // Padauk has no arithmetic right shift instructions.
          // So we need this emulation sequence here.
          if (!SPEC_USIGN (getSpec (operandType (left))))
            {
              if (aopInReg (result->aop, size - 1, A_IDX) &&
                regDead (P_IDX, ic) && (size == 1 || !aopInReg (result->aop, 0, P_IDX)))
                {
                  cheapMove (ASMOP_P, 0, ASMOP_A, 0, true, true, true);
                  emit2 ("sl", "p");
                  emit2 ("src", "a");
                  cost (2, 2);
                }
              else if (aopInReg (result->aop, size - 1, A_IDX))
                {
                   emit2 ("sl", "a");
                   emit2 ("t0sn", "f, c");
                   emit2 ("or", "a, #0x01", aopGet (result->aop, size - 1));
                   emit2 ("src", "a");
                   emit2 ("src", "a");
                   cost (5, 5);
                }
              else
                {
                  if (size > 1 && aopInReg (result->aop, 0, A_IDX) || !regDead (A_IDX, ic))
                    {
                      wassert (regalloc_dry_run);
                      cost (500, 500);
                    }
                   cheapMove (ASMOP_A, 0, result->aop, size - 1, true, true, true);
                   emit2 ("sl", "a");
                   cost (1, 1);
                   if (result->aop->type == AOP_STK)
                     {
                       cheapMove (ASMOP_A, 0, result->aop, size - 1, true, true, false);
                       emit2 ("src", "a");
                       cheapMove (result->aop, size - 1, ASMOP_A, 0, true, true, size > 1);
                     }
                   else
                     {
                       emit2 ("src", aopGet (result->aop, size - 1));
                       cost (1, 1);
                     }
                }
            }
          else if (result->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_A, 0, result->aop, size - 1, true, true, true);
              emit2("sr", "a");
              cost (1, 1);
              cheapMove (result->aop, size - 1, ASMOP_A, 0, true, true, size == 1);
            }
          else
            {
              emit2("sr", aopGet (result->aop, size - 1));
              cost (1, 1);
            }
        
          for(int i = size - 2; i >= 0; i--)
            {
              if (pushed_a && aopInReg (result->aop, i, A_IDX))
                {
                  wassert (regalloc_dry_run);
                  cost (500, 500);
                }
              if (result->aop->type == AOP_STK)
                {
                  cheapMove (ASMOP_A, 0, result->aop, i, true, true, false);
                  emit2("src", "a");
                  cost (1, 1);
                  cheapMove (result->aop, i, ASMOP_A, 0, true, true, i > 1);
                }
              else
                {
                  emit2 ("src", "%s", aopGet (result->aop, i));
                  cost (1, 1);
                }
            }

          shCount--;
        }

      if (loop)
        {
          if (pushed_a)
            {
              popAF();
              pushed_a = false;
            }
          emit2 ("dzsn", "a");
          if (!regalloc_dry_run)
            emit2 ("goto", "!tlabel", labelKey2num (tlbl->key));
          cost (2, 2);
        }

      regalloc_dry_run_cycle_scale = 1;
    }
  else
    {
      bool pushed_counter = false;

      // Avoid overwriting counter by result.
      if (right->aop->type == AOP_STK && result->aop->type == AOP_STK && right->aop->aopu.stk_off >= result->aop->aopu.stk_off && right->aop->aopu.stk_off < result->aop->aopu.stk_off + result->aop->size)
        {
          cheapMove (ASMOP_A, 0, right->aop, 0, true, true, true);
          pushAF ();
          pushed_counter = true;
        }

      genMove (result->aop, left->aop, !aopInReg (right->aop, 0, A_IDX), !aopInReg (right->aop, 0, P_IDX));

      symbol *tlbl1 = regalloc_dry_run ? 0 : newiTempLabel (0);
      symbol *tlbl2 = regalloc_dry_run ? 0 : newiTempLabel (0);

      if (pushed_counter)
        popAF ();
      else
        cheapMove (ASMOP_A, 0, right->aop, 0, true, p_dead && !aopInReg (result->aop, 0, P_IDX), true);
      emitLabel (tlbl1);
      G.p.type = AOP_INVALID;

      emit2 ("sub", "a, #1");
      emit2 ("t0sn", "f, c");
      if (!regalloc_dry_run)
        emit2 ("goto", "!tlabel", labelKey2num (tlbl2->key));
      cost (3, 3);

      if (!SPEC_USIGN (getSpec (operandType (left))) || result->aop->type == AOP_STK)
        {
          pushAF();
          pushed_a = true;
        }

      // Padauk has no arithmetic right shift instructions.
      // So we need this emulation sequence here.
      if (!SPEC_USIGN (getSpec (operandType (left))))
        {
          if (result->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_P, 0, result->aop, size - 1, true, true, true);
              emit2 ("mov", "a, #0x01");
              emit2 ("sl", "p");
              emit2 ("t0sn", "f, c");
              emit2 ("or", "p, a");
              emit2 ("src", "p");
              emit2 ("src", "p");
              cheapMove (result->aop, size - 1, ASMOP_P, 0, true, true, size == 1);
            }
          else
            {
              emit2 ("mov", "a, #0x01");
              emit2 ("sl", aopGet (result->aop, size - 1));
              emit2 ("t0sn", "f, c");
              emit2 ("or", "%s, a", aopGet (result->aop, size - 1));
              emit2 ("src", aopGet (result->aop, size - 1));
              emit2 ("src", aopGet (result->aop, size - 1));
              cost (6, 6);
            }
        }
      else
        {
          if (result->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_A, 0, result->aop, size - 1, true, true, true);
              emit2("sr", "a");
              cost (1, 1);
              cheapMove (result->aop, size - 1, ASMOP_A, 0, true, true, size == 1);
            }
          else
            {
              emit2("sr", aopGet (result->aop, size - 1));
              cost (1, 1);
            }
        }
    
      for(int i = size - 2; i >= 0; i--)
        {
          if (result->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_A, 0, result->aop, i, true, true, false);
              emit2 ("src", "a");
              cheapMove (result->aop, i, ASMOP_A, 0, true, true, i == 1);
            }
          else
            {
              emit2 ("src", "%s", aopGet (result->aop, i));
              cost (1, 1);
            }
        }

      if (!SPEC_USIGN (getSpec (operandType (left))) || result->aop->type == AOP_STK)
        {
          popAF();
          pushed_a = false;
        }
    
      if (!regalloc_dry_run)
        emit2 ("goto", "!tlabel", labelKey2num (tlbl1->key));
      cost (1, 1);

      emitLabel (tlbl2);
      G.p.type = AOP_INVALID;
    }

  if (pushed_a)
    popAF();

  if (pushed_p)
    popPF (regDead (A_IDX, ic));

release:
  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}


/*-----------------------------------------------------------------*/
/* getBitFieldByte - process partial byte of bit-field             */
/*-----------------------------------------------------------------*/
static void getBitFieldByte (int len, int str, bool sex)
{
  wassert (len >= 0 && len < 8);

  bool mask = len + str != 8;

  // Shift
  if (len == 1 && str == 7)
    {
      emit2 ("sl", "a");
      emit2 ("slc", "a");
      str = 0;
      mask = true;
    }
  if (str >= 4)
    {
      emit2 ("swap", "a");
      cost (1, 1);
      str -= 4;
      mask = true;
    }
  while (str--)
    {
      emit2 ("sr", "a");
      cost (1, 1);
    }

  // Mask
  if (mask)
    {
      emit2 ("and", "a, #0x%02x", 0xff >> (8 - len));
      cost (2, 1);
    }

  // Sign-extend
  if (sex)
    {
      symbol *const tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      emit2 ("ceqsn", "a, #0x%02x", 0x80 >> (8 - len));
      emit2 ("nop", "");
      emit2 ("t0sn", "f, c");
      if (tlbl)
        emit2 ("goto", "!tlabel", labelKey2num (tlbl->key));
      emit2 ("or", "a, #0x%02x", (0xff00 >> (8 - len)) & 0xff);
      cost (5, 5);
      emitLabel (tlbl);
    }
}

/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void
genPointerGet (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genPointerGet", ""));

  aopOp (left, ic);
  aopOp (right, ic);
  aopOp (result, ic);

  wassertl (right, "GET_VALUE_AT_ADDRESS without right operand");
  wassertl (IS_OP_LITERAL (right), "GET_VALUE_AT_ADDRESS with non-literal right operand");

  bool pushed_a = false;
  bool pushed_p = false;

  bool bit_field = IS_BITVAR (getSpec (operandType (result)));
  int size = result->aop->size;
  int blen, bstr;
  blen = bit_field ? (SPEC_BLEN (getSpec (operandType (IS_BITVAR (getSpec (operandType (right))) ? right : left)))) : 0;
  bstr = bit_field ? (SPEC_BSTR (getSpec (operandType (IS_BITVAR (getSpec (operandType (right))) ? right : left)))) : 0;

  sym_link *type = operandType (left);
  int ptype = (IS_PTR (type) && !IS_FUNC (type->next)) ? DCL_TYPE (type) : PTR_TYPE (SPEC_OCLS (getSpec (type)));

  if (left->aop->type == AOP_IMMD && ptype == GPOINTER && IS_SYMOP (left) && OP_SYMBOL (left)->remat)
    ptype = left->aop->aopu.code ? CPOINTER : POINTER;

  wassertl (aopIsLitVal (right->aop, 0, 2, 0x0000), "Unimplemented nonzero right operand in pointer read");

  if (left->aop->type == AOP_IMMD && (ptype == POINTER || ptype == CPOINTER))
    {
      for (int i = 0; !bit_field ? i < size : blen > 0; i++, blen -= 8)
        {
          if (!regDead (A_IDX, ic) && !pushed_a)
            {
              pushAF();
              pushed_a = true;
            }

          if (ptype == POINTER)
            {
              emit2 ("mov", "a, %s+%d", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
              cost (1, 1);
            }
          else
            {
              emit2 ("call", "%s+%d", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
              cost (1, 4);
            }

          if (bit_field && blen < 8)
            getBitFieldByte (blen, bstr, !SPEC_USIGN (getSpec (operandType (result))));

          if (aopInReg (result->aop, i, A_IDX) && (!bit_field ? i + 1 < size : blen - 8 > 0))
            {
              wassert (!pushed_a);
              pushAF();
              pushed_a = true;
            }
          else
            cheapMove (result->aop, i, ASMOP_A, 0, true, true, true);
        }
    }
#if 0 // TODO: Implement alignment requirements - ldtabl needs 16-bit-aligned operand
  else if (TARGET_IS_PDK15 && ptype == CPOINTER && left->aop->type == AOP_DIR && size == 1) // pdk15 has ldtabl for efficient read from code space via a 12-bit address (top nibble of 16-bit value is ignored).
    {
      emit2 ("ldtabl", "a, %s", aopGet (left->aop, 0));
      cost (1, 2);

      if (bit_field && blen < 8)
        getBitFieldByte (blen, bstr, !SPEC_USIGN (getSpec (operandType (result))));

      cheapMove (result->aop, 0, ASMOP_A, 0, true, true);
      goto release;
    }
#endif
  else if (ptype == POINTER) // Try to use efficient idxm when we know the source is in RAM.
    {
      const asmop *ptr_aop = (left->aop->type == AOP_DIR && TARGET_IS_PDK16) ? left->aop : ASMOP_P;

      if (!regDead (A_IDX, ic))
        {
          pushAF ();
          pushed_a = true;
        }

      cheapMove (ptr_aop, 0, left->aop, 0, true, true, true);

      for (int i = 0; !bit_field ? i < size : blen > 0; i++, blen -= 8)
        {
          if (i != 0 && aopInReg (ptr_aop, 0, P_IDX) && aopInReg (result->aop, i - 1, P_IDX)) // Would have been overwritten on previous byte.
            {
              cost (500, 500);
              wassert (regalloc_dry_run);
            }

          emit2 ("idxm", "a, %s", aopGet (ptr_aop, 0));
          cost (1, 2);

          if (bit_field && blen < 8)
            getBitFieldByte (blen, bstr, !SPEC_USIGN (getSpec (operandType (result))));

          if (aopInReg (result->aop, i, A_IDX) && (!bit_field ? i + 1 < size : blen - 8 > 0))
            {
              pushAF();
              pushed_a = true;
            }
          else
            {
              cheapMove (result->aop, i, ASMOP_A, 0, true, i + 1 == size, true);
            }

          if (i + 1 != size)
            {
              emit2 ("inc", "%s", aopGet (ptr_aop, 0));
              if (aopInReg (ptr_aop, 0, P_IDX))
                G.p.type = AOP_INVALID;
              cost (1, 1);
            }
        }
      if (ptr_aop == left->aop && !(aopInReg (left->aop, 0, P_IDX) && regDead (P_IDX, ic)))
        for (int i = 1; i < size; i++)
          {
            emit2 ("dec", "%s", aopGet (ptr_aop, 0));
            if (aopInReg (ptr_aop, 0, P_IDX))
              G.p.type = AOP_INVALID;
            cost (1, 1);
          }
      goto release;
    }
  else // Generic, but also inefficient.
    {
      if (!regDead (A_IDX, ic))
        {
          pushAF ();
          pushed_a = true;
        }
      if (!regDead (P_IDX, ic))
        {
          pushPF (!aopInReg (left->aop, 0, A_IDX) && !aopInReg (left->aop, 1, A_IDX));
          pushed_p = true;
        }

      for (int i = 0; !bit_field ? i < size : blen > 0; i++, blen -= 8)
        {
          if (i != 0 && (aopInReg (left->aop, 0, A_IDX) || aopInReg (left->aop, 1, A_IDX) || aopInReg (result->aop, i - 1, P_IDX))) // Would have been overwritten on previous byte.
            {
              cost (500, 500);
              wassert (regalloc_dry_run);
            }

          genMove (ASMOP_PA, left->aop, true, true);
          if (i > 2)
            {
              emit2 ("xch", "a, p");
              emit2 ("add", "a, #%d", i);
              emit2 ("xch", "a, p");
              emit2 ("addc", "a");
              cost (4, 4);
            }
          else
            for (int j = 0; j < i; j++)
              {
                emit2 ("inc", "p");
                emit2 ("addc", "a");
                cost (2, 2);
              }

          if (i + 1 < size && !bit_field)
            {
              emit2 ("call", "__gptrget2");
              cost (1, (ptype == CPOINTER) ? 32 : 13);
              G.p.type = AOP_INVALID;
              genMove_o (result->aop, i, ASMOP_AP, 0, 2, true, true);
              i++;
              continue;
            }

          emit2 ("call", "__gptrget");
          cost (1, (ptype == CPOINTER) ? 16 : 8);
          G.p.type = AOP_INVALID;

          if (bit_field && blen < 8)
            getBitFieldByte (blen, bstr, !SPEC_USIGN (getSpec (operandType (result))));

          if (aopInReg (result->aop, i, P_IDX) && (!bit_field ? i + 1 < size : blen - 8 > 0))
            {
              wassert (regalloc_dry_run);
              cost (200, 200);
            }
          else if (aopInReg (result->aop, i, A_IDX) && (!bit_field ? i + 1 < size : blen - 8 > 0))
            {
              pushAF();
              pushed_a = true;
            }
          else
            {
              cheapMove (result->aop, i, ASMOP_A, 0, true, true, true);
            }
        }
      goto release;
    }

release:
  if (pushed_p)
    popPF (!aopInReg (result->aop, 0, A_IDX) && !aopInReg (result->aop, 1, A_IDX));
  if (pushed_a)
    popAF ();

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genPointerSet - stores the value into a pointer location        */
/*-----------------------------------------------------------------*/
static void
genPointerSet (iCode *ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genPointerSet", ""));

  aopOp (left, ic);
  aopOp (right, ic);

  bool pushed_p = false;
  bool pushed_a = false;

  bool bit_field = IS_BITVAR (getSpec (operandType (right))) || IS_BITVAR (getSpec (operandType (left)));
  int size = right->aop->size;

  int blen, bstr;
  blen = bit_field ? (SPEC_BLEN (getSpec (operandType (IS_BITVAR (getSpec (operandType (right))) ? right : left)))) : 0;
  bstr = bit_field ? (SPEC_BSTR (getSpec (operandType (IS_BITVAR (getSpec (operandType (right))) ? right : left)))) : 0;

#if 0 // TODO: Implement alignment requirements - idxm needs 16-bit-aligned operand
  if (left->aop->type == AOP_DIR && TARGET_IS_PDK16 && !bit_field)
    {
      for (int i = 0; i < size; i++)
        {
          cheapMove (ASMOP_A, 0, right->aop, i, true, true);
          emit2 ("idxm", "%s, a", aopGet (left->aop, 0));
          cost (1, 2);
          if (i + 1 != size)
            {
              emit2 ("inc", "%s", aopGet (left->aop, 0));
              cost (1, 1);
            }
        }
      for (int i = 1; i < size; i++)
        {
          emit2 ("dec", "%s", aopGet (left->aop, 0));
          cost (1, 1);
        }
    }
  else
#endif
  if (right->aop->type == AOP_STK && !bit_field && aopInReg (left->aop, 0, A_IDX))
    {
      if (!regDead (P_IDX, ic))
        {
          pushPF (false);
          pushed_p = true;
        }
      for (int i = 0; i < size; i++)
        {
          cheapMove (ASMOP_P, 0, right->aop, i, false, true, true);
          emit2 ("xch", "a, p");
          emit2 ("idxm", "p, a");
          cost (2, 3);

          if (i + 1 != size)
            {
              emit2 ("inc", "p");
              emit2 ("xch", "a, p");
              cost (2, 2);
              if (G.p.type == AOP_STL)
                G.p.offset++;
              else
                G.p.type = AOP_INVALID;
            }
        }
    }
  if (right->aop->type == AOP_STK && !bit_field && left->aop->type != AOP_IMMD)
    {
      if (!regDead (A_IDX, ic))
        {
          pushAF ();
          pushed_a = true;
        }
      if (!regDead (P_IDX, ic))
        {
          pushPF (!aopInReg (left->aop, 0, A_IDX) && !aopInReg (right->aop, 0, A_IDX));
          pushed_p = true;
        }

      for (int i = 0; i < size; i++)
        {
          cheapMove (ASMOP_A, 0, right->aop, i, true, !aopInReg (left->aop, 0, P_IDX), true);
          cheapMove (ASMOP_P, 0, left->aop, 0, false, true, true);

          if (!aopInReg (left->aop, 0, P_IDX))
            {
              if (i > 3)
                {
                  emit2 ("xch", "a, p");
                  emit2 ("add", "a, #%d", i);
                  emit2 ("xch", "a, p");
                  cost (3, 3);
                }
              else
                for (int j = 0; j < i; j++)
                  {
                    emit2 ("inc", "p");
                    cost (1, 1);
                    if (G.p.type == AOP_STL)
                      G.p.offset++;
                    else
                      G.p.type = AOP_INVALID;
                  }
            }
          else if (i)
            {
              emit2 ("inc", "p");
              cost (1, 1);
              if (G.p.type == AOP_STL)
                G.p.offset++;
              else
                G.p.type = AOP_INVALID;
            }

          emit2 ("idxm", "p, a");
          cost (1, 2);
        }

      if (aopInReg (left->aop, 0, P_IDX) && !regDead (P_IDX, ic))
        for (int i = 1; i < size; i++)
          {
            emit2 ("dec", "p");
            cost (1, 1);
            G.p.type = AOP_INVALID;
          }
    }
  else if (aopInReg (right->aop, 0, P_IDX) && aopInReg (right->aop, 1, A_IDX) && left->aop->type == AOP_IMMD)
    {
      emit2 ("mov", "%s+%d, a", left->aop->aopu.immd, left->aop->aopu.immd_off + 1);
      cost (1, 1);
      if (regDead (A_IDX, ic))
        {
          emit2 ("mov", "a, p");
          emit2 ("mov", "%s+%d, a", left->aop->aopu.immd, left->aop->aopu.immd_off);
          cost (2, 2);
        }
      else
        {
          emit2 ("xch", "a, p");
          emit2 ("mov", "%s+%d, a", left->aop->aopu.immd, left->aop->aopu.immd_off);
          emit2 ("xch", "a, p");
          cost (3, 3);
        }
    }
  else
    {
      const asmop *ptr_aop;
      bool swapped = false;

      if (left->aop->type == AOP_IMMD)
          ptr_aop = 0;
#if 0 // TODO: Implement alignment requirements - idxm needs 16-bit-aligned operand
      else if (left->aop->type == AOP_DIR && size == 1 || aopInReg (left->aop, 0, P_IDX))
        ptr_aop = left->aop;
#endif
      else if (aopInReg (right->aop, 0, P_IDX) && regDead (P_IDX, ic))
        {
          if (!regDead (A_IDX, ic))
            {
              pushAF ();
              pushed_a = true;
            }
          if (aopInReg (right->aop, 1, A_IDX))
            {
              cost (1000, 1000);
              wassert (regalloc_dry_run);
            }
          ptr_aop = ASMOP_P;
          cheapMove (ASMOP_A, 0, left->aop, 0, true, false, true);
          emit2 ("xch", "a, p");
          cost (1, 1);
          G.p.type = AOP_INVALID;
          swapped = true;
        }
      else
        {
          if (!regDead (A_IDX, ic))
            {
              pushAF ();
              pushed_a = true;
            }
          ptr_aop = ASMOP_P;
          cheapMove (ptr_aop, 0, left->aop, 0, !aopInReg (right->aop, 0, A_IDX), true, true);
          if (!regDead (P_IDX, ic) || aopInReg (right->aop, 0, P_IDX) || aopInReg (right->aop, 1, P_IDX))
            {
              cost (1000, 1000);
              wassert (regalloc_dry_run);     
            }
        }
      
      for (int i = 0; !bit_field ? i < size : blen > 0; i++, blen -= 8)
        {
          if (!ptr_aop && aopIsLitVal (right->aop, i, 1, 0) && !(bit_field || blen >= 8))
            {
              emit2 ("clear", "%s+%d", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
              cost (1, 1);
              continue;
            }

          if (bit_field && blen < 8)
            {
              if (!regDead (A_IDX, ic) && !pushed_a)
                {
                  pushAF ();
                  pushed_a = true;
                }

              if (right->aop->type == AOP_LIT)
                {
                  unsigned char mval = ~((0xff >> (8 - blen)) << bstr) & 0xff;
                  unsigned char bval = (byteOfVal (right->aop->aopu.aop_lit, i) << bstr) & ((0xff >> (8 - blen)) << bstr);

                  if (!ptr_aop && (byteOfVal (right->aop->aopu.aop_lit, i) << bstr) == ((0xff >> (8 - blen)) << bstr))
                    {
                      emit2 ("mov", "a, #0x%02x", bval);
                      emit2 ("or", "%s+%d, a", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
                      cost (2, 2);
                      continue;
                    }
                  else if (!ptr_aop && !bval)
                    {
                      emit2 ("mov", "a, #0x%02x", mval);
                      emit2 ("and", "%s+%d, a", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
                      cost (2, 2);
                      continue;
                    }

                  if (!ptr_aop)
                    {
                      emit2 ("mov", "a, %s+%d", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
                      cost (1, 1);
                    }
                  else
                    {
                      emit2 ("idxm", "a, %s", aopGet (ptr_aop, 0));
                      cost (1, 2);
                    }

                  if ((byteOfVal (right->aop->aopu.aop_lit, i) << bstr) != ((0xff >> (8 - blen)) << bstr))
                    {
                      emit2 ("and", "a, #0x%02x", mval);
                      cost (1, 1);
                    }

                  if (bval)
                    {
                      emit2 ("or", "a, #0x%02x", bval);
                      cost (1, 1);
                    }
                }
              else if (bit_field && blen == 1)
                {
                  cheapMove (ASMOP_A, 0, right->aop, i, true, !ptr_aop || !aopInReg (ptr_aop, 0, P_IDX), true);
                  emit2 ("sr", "a");
                  cost (1, 1);
                  if (!ptr_aop)
                    {
                      emit2 ("mov", "a, %s+%d", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
                      cost (1, 1);
                    }
                  else
                    {
                      emit2 ("idxm", "a, %s", aopGet (ptr_aop, 0));
                      cost (1, 2);
                    }
                  emit2 ("and", "a, #0x%02x", ~((0xff >> (8 - blen)) << bstr) & 0xff);
                  emit2 ("t0sn", "f, c");
                  emit2 ("or", "a, #0x%02x", 1 << bstr);
                  cost (3, 3);
                }
              else
                {
                  if (aopInReg (right->aop, i, A_IDX))
                    {
                      cost (100, 100);
                      wassert (regalloc_dry_run);
                    }
                  if (!ptr_aop)
                    {
                      emit2 ("mov", "a, %s+%d", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
                      cost (1, 1);
                    }
                  else
                    {
                      emit2 ("idxm", "a, %s", aopGet (ptr_aop, 0));
                      cost (1, 2);
                    }
                  emit2 ("and", "a, #0x%02x", ~((0xff >> (8 - blen)) << bstr) & 0xff);
                  cost (1, 1);

                  emit2 ("xch", "a, p");
                  cost (1, 1);
                  if (!regDead (P_IDX, ic) && !pushed_p)
                    {
                      pushAF ();
                      pushed_p = true;
                    }
                  if (ptr_aop && aopInReg (ptr_aop, 0, P_IDX))
                    pushAF ();
                  if (!aopInReg (right->aop, i, P_IDX)) // xch above would already have brought it into a.
                    cheapMove (ASMOP_A, 0, right->aop, i, true, false, true);
                  if (bstr >= 4)
                    {
                      emit2 ("swap", "a");
                      cost (1, 1);
                    }
                  for (int j = (bstr >= 4 ? 4 : 0); j < bstr; j++)
                    {
                      emit2 ("sl", "a");
                      cost (1, 1);
                    }
                  emit2 ("and", "a, #0x%02x", (0xff >> (8 - blen)) << bstr);
                  emit2 ("or", "a, p");
                  cost (2, 2);

                  if (ptr_aop && aopInReg (ptr_aop, 0, P_IDX))
                    {
                      emit2 ("mov", "p, a");
                      cost (1, 1);
                      popAF ();
                      emit2 ("xch", "a, p");
                      cost (1, 1);
                    }
                }
            }
          else if (!swapped)
            {
              if (!aopInReg (right->aop, i, A_IDX) && !regDead (A_IDX, ic) && !pushed_a)
                {
                  pushAF ();
                  pushed_a = true;
                }
              cheapMove (ASMOP_A, 0, right->aop, i, true, regDead (P_IDX, ic) || pushed_p, true);
            }

          if (!ptr_aop)
            {
              emit2 ("mov", "%s+%d, a", left->aop->aopu.immd, left->aop->aopu.immd_off + i);
              cost (1, 1);
            }
          else
            {
              emit2 ("idxm", "%s, a", aopGet (ptr_aop, 0));
              cost (1, 2);
            }

          if (i + 1 != size && ptr_aop)
            {
              emit2 ("inc", "%s", aopGet (ptr_aop, 0));
              if (G.p.type == AOP_STL)
                G.p.offset++;
              else
                G.p.type = AOP_INVALID;
              cost (1, 1);
            }
        }
    }

  if (pushed_p)
    popPF (true);
  if (pushed_a)
    popAF ();

  freeAsmop (right);
  freeAsmop (left);
}

/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void
genAssign (const iCode *ic)
{
  operand *result, *right;

  D (emit2 ("; genAssign", ""));

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);

  aopOp (right, ic);
  aopOp (result, ic);

  wassert (result->aop->type != AOP_DUMMY || right->aop->type != AOP_DUMMY);

  if (right->aop->type == AOP_DUMMY)
    {
      int i;
      D (emit2 ("; Dummy write", ""));
      for (i = 0; i < result->aop->size; i++)
        cheapMove (result->aop, i, ASMOP_A, 0, regDead (A_IDX, ic), regDead (P_IDX, ic), true);
    }
  else if (result->aop->type == AOP_DUMMY)
    {
      wassert (0);
#if 0
      int i;
      D (emit2 ("; Dummy read", ""));

      if (!regDead(A_IDX, ic) && right->aop->type == AOP_DIR)
        for (i = 0; i < right->aop->size; i++)
          emit3_o (A_TNZ, right->aop, i, 0, 0);
      else
        {
          if (!regDead(A_IDX, ic))
            push (ASMOP_A, 0, 1);
          for (i = 0; i < right->aop->size; i++)
            cheapMove (ASMOP_A, 0, right->aop, i, FALSE);
          if (!regDead(A_IDX, ic))
            pop (ASMOP_A, 0, 1);
        }
#endif
    }
  else
    genMove(result->aop, right->aop, regDead (A_IDX, ic), regDead (P_IDX, ic));

  freeAsmop (right);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genIfx - generate code for Ifx statement                        */
/*-----------------------------------------------------------------*/
static void
genIfx (const iCode *ic)
{
  operand *const cond = IC_COND (ic);

  D (emit2 ("; genIfx", ""));

  aopOp (cond, ic);

  if(cond->aop->type == AOP_STK && !regDead (P_IDX, ic))
    {
      cost (200, 200);
      wassert (regalloc_dry_run);
    }

  if ((aopInReg (cond->aop, 0, A_IDX) && aopInReg (cond->aop, 1, P_IDX) ||
    aopInReg (cond->aop, 0, P_IDX) && aopInReg (cond->aop, 1, A_IDX)) &&
    !regDead (A_IDX, ic))
    {
      if (IC_TRUE (ic))
        {
          emit2 ("ceqsn", "a, #0");
          cost (1, 1.5f);
          emitJP (IC_TRUE (ic), 0.5f);
          emit2 ("ceqsn", "a, p");
          cost (1, 0.75f);
          emitJP (IC_TRUE (ic), 0.375f);
          goto release;
        }
      else if (!TARGET_IS_PDK13)
        {
          symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emit2 ("ceqsn", "a, #0");
          if (!regalloc_dry_run)
            emit2 ("goto", "#!tlabel", labelKey2num (tlbl->key));
          emit2 ("cneqsn", "a, p");
          cost (3, 3.25f);
          emitJP (IC_FALSE (ic), 0.375f);
          emitLabel (tlbl);
          goto release;
        }
    }

  int skip_byte;
  if (IS_FLOAT (operandType (cond))) // Clear sign bit for float.
    {
      cheapMove (ASMOP_A, 0, cond->aop, cond->aop->size - 1, true, true, true);
      emit2 ("and", "a, #0x7f");
      cost (1, 1);
      skip_byte = cond->aop->size - 1;
    }
  else if (aopInReg (cond->aop, 1, A_IDX))
    {
      skip_byte = 1;
    }
  else
    {
      cheapMove (ASMOP_A, 0, cond->aop, 0, true, true, true);
      skip_byte = 0;
    }

  for (int i = 0; i < cond->aop->size; i++)
    {
      if (i == skip_byte)
        continue;

      if (cond->aop->type == AOP_STK)
        {
          cheapMove (ASMOP_P, 0, cond->aop, i, false, true, true);
          emit2 ("or", "a, p");
          cost (1, 1);
        }
      else
        {
          emit2 ("or", "a, %s", aopGet (cond->aop, i));
          cost (1, 1);
        }
    }

  if (TARGET_IS_PDK13 && IC_FALSE (ic)) // pdk13 does not have cneqsn.
    {
      symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
      emit2 ("ceqsn", "a, #0x00");
      emitJP (tlbl, 0.0f);
      cost (2, 3);
      emitJP (IC_FALSE (ic), 0.0f);
      emitLabel (tlbl);
    }
  else
    {
      emit2 (IC_FALSE (ic) ? "cneqsn" : "ceqsn", "a, #0x00");
      cost (1, 1); 
      emitJP (IC_FALSE (ic) ? IC_FALSE (ic) : IC_TRUE (ic), 0.0f);
    }

release:
  freeAsmop (cond);
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void
genAddrOf (const iCode *ic)
{
  operand *result, *left, *right;

  D (emit2 ("; genAddrOf", ""));

  result = IC_RESULT (ic);
  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);

  wassert (result);
  wassert (left);
  wassert (IS_TRUE_SYMOP (left));
  wassert (right && IS_OP_LITERAL (IC_RIGHT (ic)));

  const symbol *sym = OP_SYMBOL_CONST (left);
  wassert (sym);

  aopOp (result, ic);

  int size = result->aop->size;

  wassert (size == 1 || size == 2);
  if (sym->onStack)
    {
      int s = sym->stack + (sym->stack < 0 ? G.stack.param_offset : 0) + operandLitValue (right);

      if (G.p.type == AOP_STL && s == G.p.offset)
        cheapMove (result->aop, 0, ASMOP_P, 0, true, true, true);
      else
        {
          cheapMove (ASMOP_A, 0, ASMOP_SP, 0, true, true, true);
          emit2 ("add", "a, #0x%02x", (s - G.stack.pushed) & 0xff);
          cost (1, 1);
          cheapMove (result->aop, 0, ASMOP_A, 0, true, true, true);
        }

      if (size == 2)
        cheapMove (result->aop, 1, ASMOP_ZERO, 0, true, true, true);
    }
  else if (PTR_TYPE (SPEC_OCLS (getSpec (operandType (IC_LEFT (ic))))) == CPOINTER) // In ROM
    {
      wassert (size == 2);

      emit2 ("mov", "a, #<(%s + %d)", sym->rname, (int)operandLitValue (right));
      cost (1, 1);
      cheapMove (result->aop, 0, ASMOP_A, 0, true, true, true);
      emit2 ("mov", "a, #>(%s + 0x8000 + %d)", sym->rname, (int)operandLitValue (right));
      cheapMove (result->aop, 1, ASMOP_A, 0, true, true, true);
    }
  else // In RAM
    {
      emit2 ("mov", "a, #(%s + %d)", sym->rname, (int)operandLitValue (right));
      cost (1, 1);
      cheapMove (result->aop, 0, ASMOP_A, 0, true, true, true);
      if (size == 2)
        cheapMove (result->aop, 1, ASMOP_ZERO, 0, true, true, true);
    }

  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genJumpTab - generate code for jump table                       */
/*-----------------------------------------------------------------*/
static void
genJumpTab (const iCode *ic)
{
  operand *cond;

  D (emit2 ("; genJumpTab", ""));

  cond = IC_JTCOND (ic);

  aopOp (cond, ic);

  wassertl (cond->aop->size == 1, "Jump table not implemented for operands wider than 1 byte.");

  cheapMove (ASMOP_A, 0, cond->aop, 0, true, true, true);

  emit2 ("add", "a, #0x01");
  emit2 ("pcadd", "a");
  cost (2, 3);

  for (symbol *jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
    {
      if (!regalloc_dry_run)
        emit2 ("goto", "#!tlabel", labelKey2num (jtab->key));
      cost (1, 0);
    }

  freeAsmop (cond);
}

/*-----------------------------------------------------------------*/
/* genCast - generate code for cast                                */
/*-----------------------------------------------------------------*/
static void
genCast (const iCode *ic)
{
  operand *result, *right;
  int offset;
  sym_link *resulttype, *righttype;

  D (emit2 ("; genCast", ""));

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);
  resulttype = operandType (result);
  righttype = operandType (right);

  bool pushed_a = false;

  if ((getSize (resulttype) <= getSize (righttype) || !IS_SPEC (righttype) || (SPEC_USIGN (righttype) || IS_BOOLEAN (righttype))) &&
    (!IS_BOOLEAN (resulttype) || IS_BOOLEAN (righttype)))
    {
      genAssign (ic);
      return;
    }

  aopOp (right, ic);
  aopOp (result, ic);

  if (!regDead (A_IDX, ic))
    {
      pushAF ();
      pushed_a = true;
    }

  if (IS_BOOL (resulttype))
    {
      int size = right->aop->size;
      int skipbyte;

      if (aopInReg (right->aop, 1, A_IDX))
        skipbyte = 1;
      else
        {
          cheapMove (ASMOP_A, 0, right->aop, 0, true, true, true);
          skipbyte = 0;
        }

      for (offset = 0; offset < size; offset++)
        {
          if (offset == skipbyte)
            continue;
          if (right->aop->type == AOP_STK)
            {
              cheapMove (ASMOP_P, 0, right->aop, offset, false, true, true);
              emit2 ("or", "a, p");
            }
          else
            emit2 ("or", "a, %s", aopGet (right->aop, offset));
          cost (1, 1);
        }

      emit2 ("ceqsn", "a, #0x00");
      emit2 ("mov", "a, #0x01");
      if (!regalloc_dry_run) // Dummy label as target for ceqsn to prevent peephole optimizer from optimizing out mov.
        {
          symbol *tlbl = newiTempLabel (0);
          emitLabel (tlbl);
        }
      cost (2, 2);

      cheapMove (result->aop, 0, ASMOP_A, 0, true, true, true);
    }
  else // Cast to signed type
    {
      genMove_o (result->aop, 0, right->aop, 0, right->aop->size, regDead (A_IDX, ic), regDead (P_IDX, ic));

      int size = result->aop->size - right->aop->size;
      offset = right->aop->size;

      if (!regDead (A_IDX, ic) || aopInReg (result->aop, 0, A_IDX))
        pushAF ();

      cheapMove (ASMOP_A, 0, result->aop, right->aop->size - 1, true, regDead (P_IDX, ic), true);
      emit2 ("sl", "a");
      emit2 ("mov", "a, #0x00");
      emit2 ("subc", "a");
      cost (3, 3);

      while (size--)
        cheapMove (result->aop, offset++, ASMOP_A, 0, true, regDead (P_IDX, ic), true);

      if (!regDead (A_IDX, ic) || aopInReg (result->aop, 0, A_IDX))
        popAF ();
    }

  if (pushed_a)
    popAF ();

  freeAsmop (right);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genDummyRead - generate code for dummy read of volatiles        */
/*-----------------------------------------------------------------*/
static void
genDummyRead (const iCode *ic)
{
  operand *op;

  if ((op = IC_LEFT (ic)) && IS_SYMOP (op))
    ;
  else if ((op = IC_RIGHT (ic)) && IS_SYMOP (op))
    ;
  else
    return;

  aopOp (op, ic);

  if (!regDead(A_IDX, ic) && op->aop->type == AOP_DIR && (op->aop->size <= 2 || TARGET_IS_PDK15 || TARGET_IS_PDK16))
    for (int i = 0; i < op->aop->size; i++)  
      if (TARGET_IS_PDK15 || TARGET_IS_PDK16)
        {
          emit2 ("comp", "a, %s", aopGet (op->aop, i));
          cost (1, 1);
        }
      else
        {
          emit2 ("ceqsn", "a, %s", aopGet (op->aop, i));
          emit2 ("nop", "");
          cost (2, 2);
        }
  else
    {
      if (!regDead(A_IDX, ic))
        {
          emit2 ("push", "af");
          cost (1, 1);
        }

      for (int i = 0; i < op->aop->size; i++)
        cheapMove (ASMOP_A, 0, op->aop, i, true, true, true);

      if (!regDead(A_IDX, ic))
        {
          emit2 ("pop", "af");
          cost (1, 1);
        }  
    }

  freeAsmop (op);
}

/*-----------------------------------------------------------------*/
/* resultRemat - result is to be rematerialized                    */
/*-----------------------------------------------------------------*/
static bool
resultRemat (const iCode *ic)
{
  if (SKIP_IC (ic) || ic->op == IFX)
    return 0;

  if (IC_RESULT (ic) && IS_ITEMP (IC_RESULT (ic)))
    {
      const symbol *sym = OP_SYMBOL_CONST (IC_RESULT (ic));

      if (!sym->remat)
        return(false);

      bool completely_spilt = TRUE;
      for (unsigned int i = 0; i < getSize (sym->type); i++)
        if (sym->regs[i])
          completely_spilt = FALSE;

      if (completely_spilt)
        return(true);
    }

  return (false);
}

/*---------------------------------------------------------------------*/
/* genSTM8Code - generate code for STM8 for a single iCode instruction */
/*---------------------------------------------------------------------*/
static void
genPdkiCode (iCode *ic)
{
  genLine.lineElement.ic = ic;

  if (resultRemat (ic))
    {
      if (!regalloc_dry_run)
        D (emit2 ("; skipping iCode since result will be rematerialized", ""));
      return;
    }

  if (ic->generated)
    {
      //if (!regalloc_dry_run)
        D (emit2 ("; skipping generated iCode", ""));
      return;
    }

  switch (ic->op)
    {
    case '!':
      genNot (ic);
      break;

    case '~':
      genCpl (ic);
      break;

    case UNARYMINUS:
      genUminus (ic);
      break;

    case IPUSH:
      genIpush (ic);
      break;

    case IPOP:
      wassertl (0, "Unimplemented iCode");
      break;

    case CALL:
    case PCALL:
      genCall (ic);
      break;

    case FUNCTION:
      genFunction (ic);
      break;

    case ENDFUNCTION:
      genEndFunction (ic);
      break;

   case RETURN:
      genReturn (ic);
      break;

    case LABEL:
      genLabel (ic);
      break;

    case GOTO:
      genGoto (ic);
      break;

    case '+':
      genPlus (ic);
      break;

    case '-':
      genMinus (ic, ic->next && ic->next->op == IFX ? ic->next : 0);
      break;

    case '*':
      genMult (ic);
      break;

    case '/':
    case '%':
      wassertl (0, "Unimplemented iCode");
      break;

    case '>':
    case '<':
      genCmp (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case LE_OP:
    case GE_OP:
      wassertl (0, "Unimplemented iCode");
      break;

    case NE_OP:
    case EQ_OP:
      genCmpEQorNE (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case AND_OP:
    case OR_OP:
      wassertl (0, "Unimplemented iCode");
      break;

    case '^':
      genXor (ic);
      break;

    case '|':
      genOr (ic);
      break;

    case BITWISEAND:
      genAnd (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case INLINEASM:
      genInline (ic);
      break;

    case RRC:
    case RLC:
      wassertl (0, "Unimplemented iCode");
      break;

    case GETABIT:
      wassertl (0, "Unimplemented iCode");
      break;

    case LEFT_OP:
      genLeftShift (ic);
      break;

    case RIGHT_OP:
      genRightShift (ic);
      break;

    case GET_VALUE_AT_ADDRESS:
      genPointerGet (ic);
      break;

    case SET_VALUE_AT_ADDRESS:
      genPointerSet (ic);
      break;

    case '=':
      wassert (!POINTER_SET (ic));
      genAssign (ic);
      break;

    case IFX:
      genIfx (ic);
      break;

    case ADDRESS_OF:
      genAddrOf (ic);
      break;

    case JUMPTABLE:
      genJumpTab (ic);
      break;

    case CAST:
      genCast (ic);
      break;

    case RECEIVE:
    case SEND:
      wassertl (0, "Unimplemented iCode");
      break;

    case DUMMY_READ_VOLATILE:
      genDummyRead (ic);
      break;

    case CRITICAL:
      wassertl (0, "Unimplemented iCode: Critical section");
      break;

    case ENDCRITICAL:
      wassertl (0, "Unimplemented iCode: Critical section");
      break;

    default:
      fprintf (stderr, "iCode op %d:\n", ic->op);
      wassertl (0, "Unknown iCode");
    }
}

float
dryPdkiCode (iCode *ic)
{
  regalloc_dry_run = true;
  regalloc_dry_run_cost_words = 0;
  regalloc_dry_run_cost_cycles = 0;

  initGenLineElement ();

  genPdkiCode (ic);

  G.p.type = AOP_INVALID;

  destroy_line_list ();

  wassert (regalloc_dry_run);

  const unsigned int word_cost_weight = 2 << (optimize.codeSize * 3 + !optimize.codeSpeed * 3);

  return (regalloc_dry_run_cost_words * word_cost_weight + regalloc_dry_run_cost_cycles * ic->count);
}

/*---------------------------------------------------------------------*/
/* genPdkCode - generate code for Padauk for a block of intructions    */
/*---------------------------------------------------------------------*/
void
genPdkCode (iCode *lic)
{
  int clevel = 0;
  int cblock = 0;  
  int cln = 0;
  regalloc_dry_run = false;

  for (iCode *ic = lic; ic; ic = ic->next)
    {
      initGenLineElement ();

      genLine.lineElement.ic = ic;

      if (ic->level != clevel || ic->block != cblock)
        {
          if (options.debug)
            debugFile->writeScope (ic);
          clevel = ic->level;
          cblock = ic->block;
        }

      if (ic->lineno && cln != ic->lineno)
        {
          if (options.debug)
            debugFile->writeCLine (ic);

          if (!options.noCcodeInAsm)
            emit2 (";", "%s: %d: %s", ic->filename, ic->lineno, printCLine (ic->filename, ic->lineno));
          cln = ic->lineno;
        }

      regalloc_dry_run_cost_words = 0;
      regalloc_dry_run_cost_cycles = 0;

      if (options.iCodeInAsm)
        {
          const char *iLine = printILine (ic);
          emit2 ("; ic:", "%d: %s", ic->key, iLine);
          dbuf_free (iLine);
        }

      genPdkiCode(ic);

#if 0
      D (emit2 (";", "Cost for generated ic %d : (%d, %f)", ic->key, regalloc_dry_run_cost_words, regalloc_dry_run_cost_cycles));
#endif
    }

  if (options.debug)
    debugFile->writeFrameAddress (NULL, NULL, 0); /* have no idea where frame is now */

  /* now we are ready to call the
     peephole optimizer */
  if (!options.nopeep)
    peepHole (&genLine.lineHead);

  /* now do the actual printing */
  printLine (genLine.lineHead, codeOutBuf);

  G.p.type = AOP_INVALID;

  /* destroy the line list */
  destroy_line_list ();
}

