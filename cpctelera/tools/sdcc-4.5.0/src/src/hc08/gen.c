/*-------------------------------------------------------------------------
  gen.c - source file for code generation for the 68HC08

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Bug Fixes - Wojciech Stryjewski  wstryj1@tiger.lsu.edu (1999 v2.1.9a)
  Hacked for the 68HC08:
  Copyright (C) 2003, Erik Petrich
  Copyright (c) 2023, Philipp Klaus Krause philipp@colecovision.eu

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

/* Use the D macro for basic (unobtrusive) debugging messages */
#define D(x) do if (options.verboseAsm) { x; } while (0)
/* Use the DD macro for detailed debugging messages */
#define DD(x)
//#define DD(x) x

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "hc08.h"
#include "ralloc.h"
#include "gen.h"
#include "dbuf_string.h"

extern int allocInfo;
static int pushReg (reg_info * reg, bool freereg);
static void pullReg (reg_info * reg);
static void transferAopAop (asmop * srcaop, int srcofs, asmop * dstaop, int dstofs);
static void adjustStack (int n);

static char *zero = "#0x00";
static char *one = "#0x01";

unsigned fReturnSizeHC08 = 4;   /* shared with ralloc.c */


static struct
{
  short hxPushed;
  short accInUse;
  short nRegsSaved;
  int stackOfs;
  int stackPushes;
  short regsinuse;
  set *sendSet;
  int tsxStackPushes;
}
_G;

static asmop *hc08_aop_pass[8];
static asmop tsxaop;

extern int hc08_ptrRegReq;
extern int hc08_nRegs;
extern struct dbuf_s *codeOutBuf;
static bool operandsEqu (operand * op1, operand * op2);
static void loadRegFromConst (reg_info * reg, int c);
static asmop *newAsmop (short type);
static const char *aopAdrStr (asmop * aop, int loffset, bool bit16);
static void updateiTempRegisterUse (operand * op);
#define RESULTONSTACK(x) \
                         (IC_RESULT(x) && IC_RESULT(x)->aop && \
                         IC_RESULT(x)->aop->type == AOP_STK )
#define IS_AOP_HX(x) ((x)->regmask == HC08MASK_HX)
#define IS_AOP_XA(x) ((x)->regmask == HC08MASK_XA)
#define IS_AOP_AX(x) ((x)->regmask == HC08MASK_AX)
#define IS_AOP_A(x) ((x)->regmask == HC08MASK_A)
#define IS_AOP_X(x) ((x)->regmask == HC08MASK_X)
#define IS_AOP_H(x) ((x)->regmask == HC08MASK_H)
#define IS_AOP_WITH_A(x) (((x)->regmask & HC08MASK_A) != 0)
#define IS_AOP_WITH_X(x) (((x)->regmask & HC08MASK_X) != 0)
#define IS_AOP_WITH_H(x) (((x)->regmask & HC08MASK_H) != 0)

#define IS_AOPOFS_A(x,o) (((x)->type == AOP_REG) && ((x)->aopu.aop_reg[o]->mask == HC08MASK_A))
#define IS_AOPOFS_X(x,o) (((x)->type == AOP_REG) && ((x)->aopu.aop_reg[o]->mask == HC08MASK_X))
#define IS_AOPOFS_H(x,o) (((x)->type == AOP_REG) && ((x)->aopu.aop_reg[o]->mask == HC08MASK_H))


#define LSB     0
#define MSB16   1
#define MSB24   2
#define MSB32   3

#define AOP(op) op->aop
#define AOP_TYPE(op) AOP(op)->type
#define AOP_SIZE(op) AOP(op)->size
#define AOP_OP(aop) aop->op

static bool regalloc_dry_run;
static unsigned int regalloc_dry_run_cost;

static void
emitBranch (char *branchop, symbol * tlbl)
{
  if (!regalloc_dry_run)
    emitcode (branchop, "%05d$", labelKey2num (tlbl->key));
  regalloc_dry_run_cost += (!strcmp(branchop, "jmp") || !strcmp(branchop, "brclr") || !strcmp(branchop, "brset") ? 3 : 2);
}

/*-----------------------------------------------------------------*/
/* hc08_emitDebuggerSymbol - associate the current code location   */
/*   with a debugger symbol                                        */
/*-----------------------------------------------------------------*/
void
hc08_emitDebuggerSymbol (const char *debugSym)
{
  genLine.lineElement.isDebug = 1;
  emitcode ("", "%s ==.", debugSym);
  genLine.lineElement.isDebug = 0;
}

/*--------------------------------------------------------------------------*/
/* transferRegReg - Transfer from register(s) sreg to register(s) dreg. If  */
/*                  freesrc is true, sreg is marked free and available for  */
/*                  reuse. sreg and dreg must be of equal size              */
/*--------------------------------------------------------------------------*/
static void
transferRegReg (reg_info *sreg, reg_info *dreg, bool freesrc)
{
  int srcidx;
  int dstidx;
  char error = 0;

  /* Nothing to do if no destination. */
  if (!dreg)
    return;

  /* But it's definitely an error if there's no source. */
  if (!sreg)
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "NULL sreg in transferRegReg");
      return;
    }

  DD (emitcode ("", "; transferRegReg(%s,%s)", sreg->name, dreg->name));

  srcidx = sreg->rIdx;
  dstidx = dreg->rIdx;

  if (srcidx == dstidx)
    {
      hc08_useReg (dreg);
      return;
    }

  switch (dstidx)
    {
    case A_IDX:
      switch (srcidx)
        {
        case H_IDX:            /* H to A */
          pushReg (hc08_reg_h, false);
          pullReg (hc08_reg_a);
          break;
        case X_IDX:            /* X to A */
          emitcode ("txa", "");
          regalloc_dry_run_cost++;
          break;
        default:
          error = 1;
        }
      break;
    case H_IDX:
      switch (srcidx)
        {
        case A_IDX:            /* A to H */
          pushReg (hc08_reg_a, false);
          pullReg (hc08_reg_h);
          break;
        case X_IDX:            /* X to H */
          pushReg (hc08_reg_x, false);
          pullReg (hc08_reg_h);
          break;
        default:
          error = 1;
        }
      break;
    case X_IDX:
      switch (srcidx)
        {
        case A_IDX:            /* A to X */
          emitcode ("tax", "");
          regalloc_dry_run_cost++;
          break;
        case H_IDX:            /* H to X */
          pushReg (hc08_reg_h, false);
          pullReg (hc08_reg_x);
          break;
        default:
          error = 1;
        }
      break;
    case HX_IDX:
      switch (srcidx)
        {
        case XA_IDX:           /* XA to HX */
          pushReg (hc08_reg_x, false);
          pullReg (hc08_reg_h);
          emitcode ("tax", "");
          regalloc_dry_run_cost++;
          break;
        default:
          error = 1;
        }
      break;
    case XA_IDX:
      switch (srcidx)
        {
        case HX_IDX:           /* HX to XA */
          emitcode ("txa", "");
          regalloc_dry_run_cost++;
          pushReg (hc08_reg_h, false);
          pullReg (hc08_reg_x);
          break;
        default:
          error = 1;
        }
      break;
    default:
      error = 1;
    }

  wassertl (!error, "bad combo in transferRegReg");

  if (freesrc)
    hc08_freeReg (sreg);

  dreg->aop = sreg->aop;
  dreg->aopofs = sreg->aopofs;
  dreg->isFree = false;
  hc08_dirtyReg (dreg, false);
  hc08_useReg (dreg);
}

/*--------------------------------------------------------------------------*/
/* updateCFA - update the debugger information to reflect the current       */
/*             canonical frame address relative to the stack pointer        */
/*--------------------------------------------------------------------------*/
static void
updateCFA (void)
{
  /* there is no frame unless there is a function */
  if (!currFunc)
    return;

  if (options.debug && !regalloc_dry_run)
    debugFile->writeFrameAddress (NULL, hc08_reg_sp, 1 + _G.stackOfs + _G.stackPushes);
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
      if (aop->size <= offset && !b && aop->type != AOP_LIT)
        continue;

      // Information from generalized constant propagation analysis
      if (!aop->valinfo.anything &&
        ((aop->valinfo.knownbitsmask >> (offset * 8)) & 0xff) == 0xff &&
        ((aop->valinfo.knownbits >> (offset * 8)) & 0xff) == b)
        continue;

      if (aop->type != AOP_LIT)
        return (false);

      if (byteOfVal (aop->aopu.aop_lit, offset) != b)
        return (false);
    }

  return (true);
}

/*--------------------------------------------------------------------------*/
/* pushReg - Push register reg onto the stack. If freereg is true, reg is   */
/*           marked free and available for reuse.                           */
/*--------------------------------------------------------------------------*/
static int
pushReg (reg_info * reg, bool freereg)
{
  int regidx = reg->rIdx;

  switch (regidx)
    {
    case A_IDX:
      emitcode ("psha", "");
      regalloc_dry_run_cost++;
      _G.stackPushes++;
      updateCFA ();
      break;
    case X_IDX:
      emitcode ("pshx", "");
      regalloc_dry_run_cost++;
      _G.stackPushes++;
      updateCFA ();
      break;
    case H_IDX:
      emitcode ("pshh", "");
      regalloc_dry_run_cost++;
      _G.stackPushes++;
      updateCFA ();
      break;
    case HX_IDX:
      emitcode ("pshx", "");
      regalloc_dry_run_cost++;
      _G.stackPushes++;
      updateCFA ();
      emitcode ("pshh", "");
      regalloc_dry_run_cost++;
      _G.stackPushes++;
      updateCFA ();
      break;
    case XA_IDX:
      emitcode ("psha", "");
      regalloc_dry_run_cost++;
      updateCFA ();
      _G.stackPushes++;
      emitcode ("pshx", "");
      regalloc_dry_run_cost++;
      updateCFA ();
      _G.stackPushes++;
      break;
    default:
      break;
    }
  if (freereg)
    hc08_freeReg (reg);
  return -_G.stackOfs - _G.stackPushes;
}

/*--------------------------------------------------------------------------*/
/* pullReg - Pull register reg off the stack.                               */
/*--------------------------------------------------------------------------*/
static void
pullReg (reg_info * reg)
{
  int regidx = reg->rIdx;

  switch (regidx)
    {
    case A_IDX:
      emitcode ("pula", "");
      regalloc_dry_run_cost++;
      _G.stackPushes--;
      updateCFA ();
      break;
    case X_IDX:
      emitcode ("pulx", "");
      regalloc_dry_run_cost++;
      _G.stackPushes--;
      updateCFA ();
      break;
    case H_IDX:
      emitcode ("pulh", "");
      regalloc_dry_run_cost++;
      _G.stackPushes--;
      updateCFA ();
      break;
    case HX_IDX:
      emitcode ("pulh", "");
      regalloc_dry_run_cost++;
      _G.stackPushes--;
      updateCFA ();
      emitcode ("pulx", "");
      regalloc_dry_run_cost++;
      _G.stackPushes--;
      updateCFA ();
      break;
    case XA_IDX:
      emitcode ("pulx", "");
      regalloc_dry_run_cost++;
      _G.stackPushes--;
      updateCFA ();
      emitcode ("pula", "");
      regalloc_dry_run_cost++;
      _G.stackPushes--;
      updateCFA ();
      break;
    default:
      break;
    }
  hc08_useReg (reg);
  hc08_dirtyReg (reg, false);
}

/*--------------------------------------------------------------------------*/
/* pullNull - Discard n bytes off the top of the stack                      */
/*--------------------------------------------------------------------------*/
static void
pullNull (int n)
{
  wassert (n >= 0);
  adjustStack (n);
}

/*--------------------------------------------------------------------------*/
/* pushConst - Push a constant byte value onto stack                        */
/*--------------------------------------------------------------------------*/
static void
pushConst (int c)
{
  if (hc08_reg_a->isFree)
    {
      loadRegFromConst (hc08_reg_a, c);
      pushReg (hc08_reg_a, true);
    }
  else if (hc08_reg_x->isFree)
    {
      loadRegFromConst (hc08_reg_x, c);
      pushReg (hc08_reg_x, true);
    }
  else if (hc08_reg_h->isFree && !c)
    {
      loadRegFromConst (hc08_reg_h, c);
      pushReg (hc08_reg_h, true);
    }
  else if (!c)
    {
      adjustStack (-1);
      emitcode ("clr", "1,s");
      regalloc_dry_run_cost += 3;
    }
  else
    {
      pushReg (hc08_reg_a, false);
      pushReg (hc08_reg_a, false);
      loadRegFromConst (hc08_reg_a, c);
      emitcode ("sta", "2,s");
      regalloc_dry_run_cost += 3;
      pullReg (hc08_reg_a);
    }
}

/*--------------------------------------------------------------------------*/
/* pushRegIfUsed - Push register reg if marked in use. Returns true if the  */
/*                 push was performed, false otherwise.                     */
/*--------------------------------------------------------------------------*/
static bool
pushRegIfUsed (reg_info *reg)
{
  if (!reg->isFree)
    {
      pushReg (reg, true);
      return true;
    }
  else
    return false;
}

/*--------------------------------------------------------------------------*/
/* pushRegIfSurv - Push register reg if marked surviving. Returns true if   */
/*                 the push was performed, false otherwise.                 */
/*--------------------------------------------------------------------------*/
static bool
pushRegIfSurv (reg_info *reg)
{
  if (!reg->isDead)
    {
      pushReg (reg, true);
      return true;
    }
  else
    return false;
}

/*--------------------------------------------------------------------------*/
/* pullOrFreeReg - If needpull is true, register reg is pulled from the     */
/*                 stack. Otherwise register reg is marked as free.         */
/*--------------------------------------------------------------------------*/
static void
pullOrFreeReg (reg_info * reg, bool needpull)
{
  if (needpull)
    pullReg (reg);
  else
    hc08_freeReg (reg);
}

/*--------------------------------------------------------------------------*/
/* adjustStack - Adjust the stack pointer by n bytes.                       */
/*--------------------------------------------------------------------------*/
static void
adjustStack (int n)
{
  while (n)
    {
      if (n > 127)
        {
          emitcode ("ais", "#127");
          regalloc_dry_run_cost += 2;
          n -= 127;
          _G.stackPushes -= 127;
          updateCFA ();
        }
      else if (n < -128)
        {
          emitcode ("ais", "#-128");
          regalloc_dry_run_cost += 2;
          n += 128;
          _G.stackPushes += 128;
          updateCFA ();
        }
      else
        {
          if (n == -1)
            {
              emitcode ("pshh", "");      /* 1 byte,  2 cycles */
              regalloc_dry_run_cost++;
            }
          else if (n == 1 && optimize.codeSize && hc08_reg_h->isFree)
            {
              emitcode ("pulh", "");      /* 1 byte,  3 cycles */
              regalloc_dry_run_cost++;
            }
          else
            {
              emitcode ("ais", "#%d", n); /* 2 bytes, 2 cycles */
              regalloc_dry_run_cost += 2;
            }
          _G.stackPushes -= n;
          n = 0;
          updateCFA ();
        }
    }
}

#if DD(1) -1 == 0
/*--------------------------------------------------------------------------*/
/* aopName - Return a string with debugging information about an asmop.     */
/*--------------------------------------------------------------------------*/
static char *
aopName (asmop * aop)
{
  static char buffer[256];
  char *buf = buffer;

  if (!aop)
    return "(asmop*)NULL";

  switch (aop->type)
    {
    case AOP_IMMD:
      sprintf (buf, "IMMD(%s)", aop->aopu.aop_immd);
      return buf;
    case AOP_LIT:
      sprintf (buf, "LIT(%s)", aopLiteral (aop->aopu.aop_lit, 0));
      return buf;
    case AOP_DIR:
      sprintf (buf, "DIR(%s)", aop->aopu.aop_dir);
      return buf;
    case AOP_EXT:
      sprintf (buf, "EXT(%s)", aop->aopu.aop_dir);
      return buf;
    case AOP_SOF:
      sprintf (buf, "SOF(%s)", OP_SYMBOL (aop->op)->name);
      return buf;
    case AOP_REG:
      sprintf (buf, "REG(%s,%s,%s,%s)",
               aop->aopu.aop_reg[3] ? aop->aopu.aop_reg[3]->name : "-",
               aop->aopu.aop_reg[2] ? aop->aopu.aop_reg[2]->name : "-",
               aop->aopu.aop_reg[1] ? aop->aopu.aop_reg[1]->name : "-",
               aop->aopu.aop_reg[0] ? aop->aopu.aop_reg[0]->name : "-");
      return buf;
    case AOP_STK:
      return "STK";
    default:
      sprintf (buf, "?%d", aop->type);
      return buf;
    }

  return "?";
}
#endif

/*--------------------------------------------------------------------------*/
/* loadRegFromAop - Load register reg from logical offset loffset of aop.   */
/*                  For multi-byte registers, loffset is of the lsb reg.    */
/*--------------------------------------------------------------------------*/
static void
loadRegFromAop (reg_info * reg, asmop * aop, int loffset)
{
  int regidx = reg->rIdx;

  if (aop->stacked && aop->stk_aop[loffset])
    {
      loadRegFromAop (reg, aop->stk_aop[loffset], 0);
      return;
    }

  DD (emitcode ("", ";     loadRegFromAop (%s, %s, %d)", reg->name, aopName (aop), loffset));

#if 0
  /* If operand is volatile, we cannot optimize. */
  if (!aop->op || isOperandVolatile (aop->op, false))
    goto forceload;


  /* If this register already has this offset of the operand
     then we need only mark it as in use. */
  if (reg->aop && reg->aop->op && aop->op && operandsEqu (reg->aop->op, aop->op) && (reg->aopofs == loffset))
    {
      hc08_useReg (reg);
      DD (emitcode ("", "; already had correct value for %s", reg->name));
      return;
    }

  /* TODO: check to see if we can transfer from another register */

  if (hc08_reg_h->aop && hc08_reg_h->aop->op && aop->op
      && operandsEqu (hc08_reg_h->aop->op, aop->op) && (hc08_reg_h->aopofs == loffset))
    {
      DD (emitcode ("", "; found correct value for %s in h", reg->name));
      transferRegReg (hc08_reg_h, reg, false);
      hc08_useReg (reg);
      return;
    }


  if (hc08_reg_x->aop && hc08_reg_x->aop->op && aop->op
      && operandsEqu (hc08_reg_x->aop->op, aop->op) && (hc08_reg_x->aopofs == loffset))
    {
      DD (emitcode ("", "; found correct value for %s in x", reg->name));
      transferRegReg (hc08_reg_x, reg, false);
      hc08_useReg (reg);
      return;
    }

  if (hc08_reg_a->aop && hc08_reg_a->aop->op && aop->op
      && operandsEqu (hc08_reg_a->aop->op, aop->op) && (hc08_reg_a->aopofs == loffset))
    {
      DD (emitcode ("", "; found correct value for %s in a", reg->name));
      transferRegReg (hc08_reg_a, reg, false);
      hc08_useReg (reg);
      return;
    }

forceload:
#endif

  switch (regidx)
    {
    case A_IDX:
      if (aop->type == AOP_REG)
        {
          if (loffset < aop->size)
            transferRegReg (aop->aopu.aop_reg[loffset], reg, false);
          else
            loadRegFromConst (reg, 0); /* TODO: handle sign extension */
        }
      else
        {
          if (aop->type == AOP_LIT)
            {
              loadRegFromConst (reg, byteOfVal (aop->aopu.aop_lit, loffset));
            }
          else
            {
              const char *l = aopAdrStr (aop, loffset, false);
              emitcode ("lda", "%s", l);
              regalloc_dry_run_cost += ((aop->type == AOP_DIR || aop->type == AOP_IMMD || aop->type == AOP_LIT) ? 2 : 3);
              hc08_dirtyReg (reg, false);
            }
        }
      break;
    case X_IDX:
      if (aop->type == AOP_REG)
        {
          if (loffset < aop->size)
            transferRegReg (aop->aopu.aop_reg[loffset], reg, false);
          else
            loadRegFromConst (reg, 0); /* TODO: handle sign extension */
        }
      else
        {
          if (aop->type == AOP_LIT)
            {
              loadRegFromConst (reg, byteOfVal (aop->aopu.aop_lit, loffset));
            }
          else
            {
              const char *l = aopAdrStr (aop, loffset, false);
              emitcode ("ldx", "%s", l);
              regalloc_dry_run_cost += ((aop->type == AOP_DIR || aop->type == AOP_IMMD || aop->type == AOP_LIT) ? 2 : 3);
              hc08_dirtyReg (reg, false);
            }
        }
      break;
    case H_IDX:
       if (aop->type == AOP_LIT)
         {
           loadRegFromConst (reg, byteOfVal (aop->aopu.aop_lit, loffset));
           break;
         }
       if (aop->type == AOP_SOF && !(_G.stackOfs + _G.stackPushes + aop->aopu.aop_stk + aop->size - loffset - 1))
        {
          pullReg (hc08_reg_h);
          pushReg (hc08_reg_h, false);
          break;
        }
      if (aop->type == AOP_REG && loffset < aop->size)
        transferRegReg (aop->aopu.aop_reg[loffset], hc08_reg_h, true);
      else if (!(aop->op && isOperandVolatile (aop->op, false)) &&  (loffset - 1 >= 0 || aop->type == AOP_LIT) && (aop->type == AOP_LIT || aop->type == AOP_IMMD || IS_S08 && aop->type == AOP_EXT)) /* TODO: Allow negative loffset - 1 */
        {
          bool pushedx = false;
          if (!hc08_reg_x->isFree)
            {
              pushReg (hc08_reg_x, true);
              pushedx = true;
            }
          loadRegFromAop (hc08_reg_hx, aop, loffset - 1);
          pullOrFreeReg (hc08_reg_x, pushedx);
        }
      else if (hc08_reg_a->isFree)
        {
          loadRegFromAop (hc08_reg_a, aop, loffset);
          transferRegReg (hc08_reg_a, hc08_reg_h, true);
        }
      else if (hc08_reg_x->isFree)
        {
          loadRegFromAop (hc08_reg_x, aop, loffset);
          transferRegReg (hc08_reg_x, hc08_reg_h, true);
        }
      else
        {
          pushReg (hc08_reg_a, true);
          loadRegFromAop (hc08_reg_a, aop, loffset);
          transferRegReg (hc08_reg_a, hc08_reg_h, true);
          pullReg (hc08_reg_a);
        }
      break;
    case HX_IDX:
       if (aop->type == AOP_LIT)
         {
           loadRegFromConst (reg, byteOfVal (aop->aopu.aop_lit, loffset) |
                                  (byteOfVal (aop->aopu.aop_lit, loffset+1) << 8));
           break;
         }
      if (aop->type == AOP_SOF)
        {
          int offset = (_G.stackOfs + _G.stackPushes + aop->aopu.aop_stk + aop->size - loffset - 1);
          if (IS_S08 && offset >= 0 && offset <= 0xff)
            {
              emitcode ("ldhx", "%s", aopAdrStr (aop, loffset, true));
              regalloc_dry_run_cost += 2;
              hc08_dirtyReg (reg, false);
              break;
            }
          else if (offset == 1)
            {
              pullReg (hc08_reg_h);
              pullReg (hc08_reg_x);
              pushReg (hc08_reg_x, false);
              pushReg (hc08_reg_h, false);
              break;
            }
        }
      if (IS_AOP_HX (aop))
        break;
      else if (IS_AOP_XA (aop))
        transferRegReg (hc08_reg_xa, hc08_reg_hx, false);
      else if (aop->type == AOP_DIR || IS_S08 && aop->type == AOP_EXT)
        {
          if (aop->size >= (loffset + 2))
            {
              emitcode ("ldhx", "%s", aopAdrStr (aop, loffset, true));
              regalloc_dry_run_cost += (aop->type == AOP_DIR ? 2 : 3);
              hc08_dirtyReg (reg, false);
            }
          else 
            {
              loadRegFromConst (hc08_reg_h, 0);
              loadRegFromAop (hc08_reg_x, aop, loffset);
            }
        }
      else if ((aop->type == AOP_LIT) || (aop->type == AOP_IMMD))
        {
          emitcode ("ldhx", "%s", aopAdrStr (aop, loffset, true));
          regalloc_dry_run_cost += 3;
          hc08_dirtyReg (reg, false);
        }
      else
        {
          loadRegFromAop (hc08_reg_h, aop, loffset + 1);
          loadRegFromAop (hc08_reg_x, aop, loffset);
        }
      break;
    case XA_IDX:
      if (IS_AOP_XA (aop))
        break;
      else if (IS_AOP_HX (aop))
        transferRegReg (hc08_reg_hx, hc08_reg_xa, false);
      else if (IS_AOP_AX (aop))
        {
          pushReg (hc08_reg_a, false);
          transferRegReg (hc08_reg_x, hc08_reg_a, false);
          pullReg (hc08_reg_x);
        }
      else
        {
          loadRegFromAop (hc08_reg_a, aop, loffset);
          loadRegFromAop (hc08_reg_x, aop, loffset + 1);
        }
      break;
    }

  hc08_useReg (reg);
}

/*--------------------------------------------------------------------------*/
/* loadRegHXAfromAop - Load registers A, H, and X from aops.                */
/*                     This takes care of the tricky cases where the        */
/*                     sources may be registers that overlap with the       */
/*                     destination registers. An aop may be NULL if the     */
/*                     corresponding register does not need to be loaded.   */
/*--------------------------------------------------------------------------*/
static void
loadRegHXAfromAop(asmop * aopH, int ofsH, asmop * aopX, int ofsX, asmop * aopA, int ofsA)
{
  /* There are three cases where pairs of registers need to be swapped */
  if (aopA && aopX && IS_AOPOFS_X (aopA, ofsA) && IS_AOPOFS_A (aopX, ofsX))
    {
      /* Swap A and X, load H */
      pushReg (hc08_reg_a, true);
      if (aopH)
        loadRegFromAop (hc08_reg_h, aopH, ofsH);
      transferRegReg (hc08_reg_x, hc08_reg_a, false);
      pullReg (hc08_reg_x);
      return;
    }
  if (aopA && aopH && IS_AOPOFS_H (aopA, ofsA) && IS_AOPOFS_A (aopH, ofsH))
    {
      /* Swap A and H, load X */
      pushReg (hc08_reg_a, true);
      if (aopX)
        loadRegFromAop (hc08_reg_x, aopX, ofsX);
      transferRegReg (hc08_reg_h, hc08_reg_a, false);
      pullReg (hc08_reg_h);
      return;
    }
  if (aopX && aopH && IS_AOPOFS_H (aopX, ofsX) && IS_AOPOFS_X (aopH, ofsH))
    {
      /* Swap X and H, load A */
      pushReg (hc08_reg_x, true);
      if (aopA)
        loadRegFromAop (hc08_reg_a, aopA, ofsA);
      transferRegReg (hc08_reg_h, hc08_reg_x, false);
      pullReg (hc08_reg_h);
      return;
    }

  /* There are two cases where the registers need to rotate */
  if (aopA && aopH && aopX && IS_AOPOFS_A (aopH, ofsH) && IS_AOPOFS_H (aopX, ofsX) && IS_AOPOFS_X (aopA, ofsA))
    {
      /* Rotate A->H->X->A */
      pushReg (hc08_reg_a, false);
      transferRegReg (hc08_reg_x, hc08_reg_a, false);
      transferRegReg (hc08_reg_h, hc08_reg_x, false);
      pullReg (hc08_reg_h);
      return;
    }
  if (aopA && aopH && aopX && IS_AOPOFS_A (aopX, ofsX) && IS_AOPOFS_X (aopH, ofsH) && IS_AOPOFS_H (aopA, ofsA))
    {
      /* Rotate A->X->H->A */
      pushReg (hc08_reg_a, false);
      transferRegReg (hc08_reg_h, hc08_reg_a, false);
      transferRegReg (hc08_reg_x, hc08_reg_h, false);
      pullReg (hc08_reg_x);
      return;
    }

  /* At this point there can be at most 1 overlapping register source. */
  if (aopX && (aopX->type == AOP_REG) && !IS_AOP_WITH_X (aopX))
    {
      loadRegFromAop (hc08_reg_x, aopX, ofsX);
      if (aopA)
        hc08_freeReg (hc08_reg_a); /* in case it's needed to load H */
      if (aopH)
        loadRegFromAop (hc08_reg_h, aopH, ofsH);
      if (aopA)
        loadRegFromAop (hc08_reg_a, aopA, ofsA);
      return;
    }
  if (aopH && (aopH->type == AOP_REG) && !IS_AOP_WITH_H (aopH))
    {
      if (aopA)
        hc08_freeReg (hc08_reg_a); /* in case it's needed to load H */
      if (aopX)
        hc08_freeReg (hc08_reg_x); /* in case it's needed to load H */
      loadRegFromAop (hc08_reg_h, aopH, ofsH);
      if (aopX)
        loadRegFromAop (hc08_reg_x, aopX, ofsX);
      if (aopA)
        loadRegFromAop (hc08_reg_a, aopA, ofsA);
      return;
    }
    
  /* Either A needs to be loaded first or the order no longer matters */
  if (aopA)
    loadRegFromAop (hc08_reg_a, aopA, ofsA);
  if (aopX)
    hc08_freeReg (hc08_reg_x); /* in case it's needed to load H */
  if (aopH && aopX && (aopH == aopX) && (ofsH == (ofsX+1)))
    loadRegFromAop (hc08_reg_hx, aopX, ofsX);
  else
    {
      if (aopH)
        loadRegFromAop (hc08_reg_h, aopH, ofsH);
      if (aopX)
        loadRegFromAop (hc08_reg_x, aopX, ofsX);
    }
}

/*--------------------------------------------------------------------------*/
/* forceStackedAop - Reserve space on the stack for asmop aop; when         */
/*                   freeAsmop is called with aop, the stacked data will    */
/*                   be copied to the original aop location.                */
/*--------------------------------------------------------------------------*/
static asmop *
forceStackedAop (asmop * aop, bool copyOrig)
{
  reg_info *reg;
  int loffset;
  asmop *newaop = newAsmop (aop->type);
  memcpy (newaop, aop, sizeof (*newaop));

  DD (emitcode ("", "; forcedStackAop %s", aopName (aop)));

  if (copyOrig && hc08_reg_a->isFree)
    reg = hc08_reg_a;
  else if (copyOrig && hc08_reg_x->isFree)
    reg = hc08_reg_x;
  else
    reg = NULL;

  for (loffset = 0; loffset < newaop->size; loffset++)
    {
      asmop *aopsof = newAsmop (AOP_SOF);
      aopsof->size = 1;
      if (copyOrig && reg)
        {
          loadRegFromAop (reg, aop, loffset);
          aopsof->aopu.aop_stk = pushReg (reg, false);
        }
      else
        {
          aopsof->aopu.aop_stk = pushReg (hc08_reg_a, false);
        }
      aopsof->op = aop->op;
      newaop->stk_aop[loffset] = aopsof;
    }
  newaop->stacked = 1;

  if (!reg && copyOrig)
    {
      for (loffset = 0; loffset < newaop->size; loffset++)
        {
          transferAopAop (aop, loffset, newaop, loffset);
        }
    }

  return newaop;
}


/*--------------------------------------------------------------------------*/
/* storeRegToAop - Store register reg to logical offset loffset of aop.     */
/*                 For multi-byte registers, loffset is of the lsb reg.     */
/*--------------------------------------------------------------------------*/
static void
storeRegToAop (reg_info *reg, asmop * aop, int loffset)
{
  int regidx = reg->rIdx;

  DD (emitcode ("", ";     storeRegToAop (%s, %s, %d), stacked=%d",
                reg->name, aopName (aop), loffset, aop->stacked));

  if ((reg->rIdx == HX_IDX) && aop->stacked && (aop->stk_aop[loffset] || aop->stk_aop[loffset + 1]))
    {
      storeRegToAop (hc08_reg_h, aop, loffset + 1);
      storeRegToAop (hc08_reg_x, aop, loffset);
      return;
    }

  if ((reg->rIdx == XA_IDX) && aop->stacked && (aop->stk_aop[loffset] || aop->stk_aop[loffset + 1]))
    {
      storeRegToAop (hc08_reg_x, aop, loffset + 1);
      storeRegToAop (hc08_reg_a, aop, loffset);
      return;
    }

  if (aop->stacked && aop->stk_aop[loffset])
    {
      storeRegToAop (reg, aop->stk_aop[loffset], 0);
      return;
    }

  if (aop->type == AOP_DUMMY)
    return;

  if (aop->type == AOP_CRY)     /* This can only happen if IFX was optimized */
    return;                     /* away, so just toss the result */

  switch (regidx)
    {
    case A_IDX:
      if ((aop->type == AOP_REG) && (loffset < aop->size))
        transferRegReg (reg, aop->aopu.aop_reg[loffset], false);
      else
        {
          emitcode ("sta", "%s", aopAdrStr (aop, loffset, false));
          regalloc_dry_run_cost += ((aop->type == AOP_DIR || aop->type == AOP_IMMD) ? 2 :3);
        }
      break;
    case X_IDX:
      if ((aop->type == AOP_REG) && (loffset < aop->size))
        transferRegReg (reg, aop->aopu.aop_reg[loffset], false);
      else
        {
          emitcode ("stx", "%s", aopAdrStr (aop, loffset, false));
          regalloc_dry_run_cost += ((aop->type == AOP_DIR || aop->type == AOP_IMMD) ? 2 :3);
        }
      break;
    case H_IDX:
      if ((aop->type == AOP_REG) && (loffset < aop->size))
        transferRegReg (reg, aop->aopu.aop_reg[loffset], false);
      else if (hc08_reg_a->isFree)
        {
          transferRegReg (hc08_reg_h, hc08_reg_a, false);
          storeRegToAop (hc08_reg_a, aop, loffset);
          hc08_freeReg (hc08_reg_a);
        }
      else if (hc08_reg_x->isFree)
        {
          transferRegReg (hc08_reg_h, hc08_reg_x, false);
          storeRegToAop (hc08_reg_x, aop, loffset);
          hc08_freeReg (hc08_reg_x);
        }
      else
        {
          pushReg (hc08_reg_a, true);
          transferRegReg (hc08_reg_h, hc08_reg_a, false);
          storeRegToAop (hc08_reg_a, aop, loffset);
          pullReg (hc08_reg_a);
        }
      break;
    case HX_IDX:
      if (aop->type == AOP_SOF)
        {
          int offset = (_G.stackOfs + _G.stackPushes + aop->aopu.aop_stk + aop->size - loffset - 1);
          if (IS_S08 && offset >= 0 && offset <= 0xff)
            {
              emitcode ("sthx", "%s", aopAdrStr (aop, loffset, true));
              regalloc_dry_run_cost += 2;
              break;
            }
        }
      if (aop->type == AOP_DIR || IS_S08 && aop->type == AOP_EXT)
        {
          emitcode ("sthx", "%s", aopAdrStr (aop, loffset, true));
          regalloc_dry_run_cost += (aop->type == AOP_DIR ? 2 : 3);;
        }
      else if (IS_AOP_XA (aop))
        transferRegReg (reg, hc08_reg_xa, false);
      else if (IS_AOP_HX (aop))
        break;
      else if (hc08_reg_a->isFree)
        {
          bool needpula;
          needpula = pushRegIfUsed (hc08_reg_a);
          transferRegReg (hc08_reg_h, hc08_reg_a, false);
          storeRegToAop (hc08_reg_a, aop, loffset + 1);
          storeRegToAop (hc08_reg_x, aop, loffset);
          pullOrFreeReg (hc08_reg_a, needpula);
        }
      else
        {
          bool needpulx;
          storeRegToAop (hc08_reg_x, aop, loffset);
          needpulx = pushRegIfUsed (hc08_reg_x);
          transferRegReg (hc08_reg_h, hc08_reg_x, false);
          storeRegToAop (hc08_reg_x, aop, loffset + 1);
          pullOrFreeReg (hc08_reg_x, needpulx);
        }
      break;
    case XA_IDX:
      if (IS_AOP_HX (aop))
        transferRegReg (reg, hc08_reg_hx, false);
      else if (IS_AOP_XA (aop))
        break;
      else if (IS_AOP_AX (aop))
        {
          pushReg (hc08_reg_a, false);
          transferRegReg (hc08_reg_x, hc08_reg_a, false);
          pullReg (hc08_reg_x);
        }
      else
        {
          storeRegToAop (hc08_reg_a, aop, loffset);
          storeRegToAop (hc08_reg_x, aop, loffset + 1);
        }
      break;
    default:
      wassert (0);
    }

  /* Disable the register tracking for now */
#if 0
  //if (!reg->aop || (reg->aop && (reg->aop != aop)))
  {
    //if (reg->aop!=aop)
    for (otheridx = 0; otheridx < hc08_nRegs; otheridx++)
      {
        otherreg = hc08_regWithIdx (otheridx);
        if (otherreg && otherreg->aop
            && otherreg->aop->op && aop->op && operandsEqu (otherreg->aop->op, aop->op) && (otherreg->aopofs == loffset))
          {
            DD (emitcode ("", "; marking %s stale", otherreg->name));
            otherreg->aop = NULL;
          }
      }
    if ((!hc08_reg_x->aop || !hc08_reg_h->aop) && hc08_reg_hx->aop)
      {
        hc08_reg_hx->aop = NULL;
        DD (emitcode ("", "; marking hx stale"));
      }
    if ((!hc08_reg_x->aop || !hc08_reg_a->aop) && hc08_reg_xa->aop)
      {
        hc08_reg_xa->aop = NULL;
        DD (emitcode ("", "; marking xa stale"));
      }

    reg->aop = aop;
    reg->aopofs = loffset;
  }
#endif
}

/*--------------------------------------------------------------------------*/
/* loadRegFromConst - Load register reg from constant c.                    */
/*--------------------------------------------------------------------------*/
static void
loadRegFromConst (reg_info * reg, int c)
{
  switch (reg->rIdx)
    {
    case A_IDX:
      c &= 0xff;
      if (reg->isLitConst)
        {
          if (reg->litConst == c)
            break;
          if (((reg->litConst + 1) & 0xff) == c)
            {
              emitcode ("inca", "");
              regalloc_dry_run_cost++;
              break;
            }
          if (((reg->litConst - 1) & 0xff) == c)
            {
              emitcode ("deca", "");
              regalloc_dry_run_cost++;
              break;
            }
        }

      if (hc08_reg_x->isLitConst && hc08_reg_x->litConst == c)
        transferRegReg (hc08_reg_x, reg, false);
      else if (!c)
        {
          emitcode ("clra", "");
          regalloc_dry_run_cost++;
        }
      else
        {
          emitcode ("lda", "!immedbyte", c);
          regalloc_dry_run_cost += 2;
        }
      break;
    case X_IDX:
      c &= 0xff;
      if (reg->isLitConst)
        {
          if (reg->litConst == c)
            break;
          if (((reg->litConst + 1) & 0xff) == c)
            {
              emitcode ("incx", "");
              regalloc_dry_run_cost++;
              break;
            }
          if (((reg->litConst - 1) & 0xff) == c)
            {
              emitcode ("decx", "");
              regalloc_dry_run_cost++;
              break;
            }
        }

      if (hc08_reg_a->isLitConst && hc08_reg_a->litConst == c)
        transferRegReg (hc08_reg_a, reg, false);
      else if (!c)
        {
          emitcode ("clrx", "");
          regalloc_dry_run_cost++;
        }
      else
        {
          emitcode ("ldx", "!immedbyte", c);
          regalloc_dry_run_cost += 2;
        }
      break;
    case H_IDX:
      c &= 0xff;
      if (reg->isLitConst && reg->litConst == c)
	break;

      if (!c)
        {
          emitcode ("clrh", "");
          regalloc_dry_run_cost++;
        }
      else if (hc08_reg_a->isLitConst && hc08_reg_a->litConst == c)
        transferRegReg (hc08_reg_a, reg, false);
      else if (hc08_reg_x->isLitConst && hc08_reg_x->litConst == c)
        transferRegReg (hc08_reg_x, reg, false);
      else if (hc08_reg_a->isFree)
        {
          loadRegFromConst (hc08_reg_a, c);
          transferRegReg (hc08_reg_a, hc08_reg_h, true);
        }
      else if (hc08_reg_x->isFree)
        {
          loadRegFromConst (hc08_reg_hx, c << 8);
        }
      else
        {
          pushReg (hc08_reg_x, true);
          loadRegFromConst (hc08_reg_hx, c << 8);
          pullReg (hc08_reg_x);
        }
      break;
    case HX_IDX:
      c &= 0xffff;
      if (reg->isLitConst)
        {
          int delta = (c - reg->litConst) & 0xffff;
          if (delta & 0x8000)
            delta = -0x8000 + (delta & 0x7fff);
          if (reg->litConst == c)
            break;
          if ((reg->litConst & 0xff) == c)
            {
              loadRegFromConst (hc08_reg_h, 0);
              break;
            }
          if ((reg->litConst & 0xff00) == (c & 0xff00))
            {
              loadRegFromConst (hc08_reg_x, c & 0xff);
              break;
            }
          if ((delta <= 127) && (delta >= -128))
            {
              emitcode ("aix","#%d", delta);
              regalloc_dry_run_cost += 2;
              break;
            }
        }
      if (!c)
        {
          loadRegFromConst (hc08_reg_h, 0);
          loadRegFromConst (hc08_reg_x, 0);
          break;
        }
      emitcode ("ldhx", "!immedword", c);
      regalloc_dry_run_cost += 3;
      break;
    case XA_IDX:
      c &= 0xffff;
      if (reg->isLitConst && reg->litConst == c)
	break;
      loadRegFromConst (hc08_reg_a, c);
      loadRegFromConst (hc08_reg_x, c >> 8);
      break;
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "Bad rIdx in loadRegFromConst");
      return;
    }

  hc08_dirtyReg (reg, false);
  reg->isLitConst = 1;
  reg->litConst = c;
  if (reg->rIdx == HX_IDX)
    {
      hc08_reg_x->isLitConst = 1;
      hc08_reg_x->litConst = c & 0xff;
      hc08_reg_h->isLitConst = 1;
      hc08_reg_h->litConst = (c >> 8) & 0xff;
    }
  hc08_useReg (reg);
}

/*--------------------------------------------------------------------------*/
/* loadRegFromImm - Load register reg from immediate value c.               */
/*--------------------------------------------------------------------------*/
static void
loadRegFromImm (reg_info * reg, char * c)
{
  if (*c == '#')
    c++;
  switch (reg->rIdx)
    {
    case A_IDX:
      emitcode ("lda", "#%s", c);
      regalloc_dry_run_cost += 2;
      break;
    case X_IDX:
      emitcode ("ldx", "#%s", c);
      regalloc_dry_run_cost += 2;
      break;
    case H_IDX:
      if (hc08_reg_a->isFree)
        {
          loadRegFromImm (hc08_reg_a, c);
          transferRegReg (hc08_reg_a, hc08_reg_h, true);
        }
      else if (hc08_reg_x->isFree)
        {
          loadRegFromImm (hc08_reg_x, c);
          transferRegReg (hc08_reg_x, hc08_reg_h, true);
        }
      else
        {
          pushReg (hc08_reg_a, true);
          loadRegFromImm (hc08_reg_a, c);
          transferRegReg (hc08_reg_a, hc08_reg_h, true);
          pullReg (hc08_reg_a);
        }
      break;
    case HX_IDX:
      emitcode ("ldhx", "#%s", c);
      regalloc_dry_run_cost += 3;
      break;
    case XA_IDX:
      emitcode ("lda", "#%s", c);
      emitcode ("ldx", "#%s >> 8", c);
      regalloc_dry_run_cost += 4;
      break;
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "Bad rIdx in loadRegFromConst");
      return;
    }
  hc08_dirtyReg (reg, false);
  hc08_useReg (reg);
}

/*--------------------------------------------------------------------------*/
/* storeConstToAop- Store constant c to logical offset loffset of asmop aop.*/
/*--------------------------------------------------------------------------*/
static void
storeConstToAop (int c, asmop * aop, int loffset)
{
  if (aop->stacked && aop->stk_aop[loffset])
    {
      storeConstToAop (c, aop->stk_aop[loffset], 0);
      return;
    }

  /* If the value needed is already in A or X, just store it */
  if (hc08_reg_a->isLitConst && hc08_reg_a->litConst == c)
    {
      storeRegToAop (hc08_reg_a, aop, loffset);
      return;
    }
  if (hc08_reg_x->isLitConst && hc08_reg_x->litConst == c)
    {
      storeRegToAop (hc08_reg_x, aop, loffset);
      return;
    }

  switch (aop->type)
    {
    case AOP_DIR:
      /* clr operates with read-modify-write cycles, so don't use if the */
      /* destination is volatile to avoid the read side-effect. */
      if (!c && !(aop->op && isOperandVolatile (aop->op, false)) && optimize.codeSize)
        {
          /* clr dst : 2 bytes, 5 cycles */
          emitcode ("clr", "%s", aopAdrStr (aop, loffset, false));
          regalloc_dry_run_cost += 2;
        }
      else
        {
          /* mov #0,dst : 3 bytes, 4 cycles */
          emitcode ("mov", "!immedbyte,%s", c, aopAdrStr (aop, loffset, false));
          regalloc_dry_run_cost += 3;
        }
      break;
    case AOP_REG:
      if (loffset > (aop->size - 1))
        break;
      loadRegFromConst (aop->aopu.aop_reg[loffset], c);
      break;
    case AOP_DUMMY:
      break;
    default:
      if (hc08_reg_a->isFree)
        {
          loadRegFromConst (hc08_reg_a, c);
          storeRegToAop (hc08_reg_a, aop, loffset);
          hc08_freeReg (hc08_reg_a);
        }
      else if (hc08_reg_x->isFree)
        {
          loadRegFromConst (hc08_reg_x, c);
          storeRegToAop (hc08_reg_x, aop, loffset);
          hc08_freeReg (hc08_reg_x);
        }
      else
        {
          pushReg (hc08_reg_a, true);
          loadRegFromConst (hc08_reg_a, c);
          storeRegToAop (hc08_reg_a, aop, loffset);
          pullReg (hc08_reg_a);
        }
    }
}

/*--------------------------------------------------------------------------*/
/* storeImmToAop- Store immediate value c to logical offset loffset of asmop aop.*/
/*--------------------------------------------------------------------------*/
static void
storeImmToAop (char *c, asmop * aop, int loffset)
{
  if (aop->stacked && aop->stk_aop[loffset])
    {
      storeImmToAop (c, aop->stk_aop[loffset], 0);
      return;
    }

  switch (aop->type)
    {
    case AOP_DIR:
      /* clr operates with read-modify-write cycles, so don't use if the */
      /* destination is volatile to avoid the read side-effect. */
      if (!strcmp (c, zero) && !(aop->op && isOperandVolatile (aop->op, false)) && optimize.codeSize)
        {
          /* clr dst : 2 bytes, 5 cycles */
          emitcode ("clr", "%s", aopAdrStr (aop, loffset, false));
          regalloc_dry_run_cost += 2;
        }
      else
        {
          /* mov #0,dst : 3 bytes, 4 cycles */
          emitcode ("mov", "%s,%s", c, aopAdrStr (aop, loffset, false));
          regalloc_dry_run_cost += 3;
        }
      break;
    case AOP_REG:
      if (loffset > (aop->size - 1))
        break;
      loadRegFromImm (aop->aopu.aop_reg[loffset], c);
      break;
    case AOP_DUMMY:
      break;
    default:
      if (hc08_reg_a->isFree)
        {
          loadRegFromImm (hc08_reg_a, c);
          storeRegToAop (hc08_reg_a, aop, loffset);
          hc08_freeReg (hc08_reg_a);
        }
      else if (hc08_reg_x->isFree)
        {
          loadRegFromImm (hc08_reg_x, c);
          storeRegToAop (hc08_reg_x, aop, loffset);
          hc08_freeReg (hc08_reg_x);
        }
      else
        {
          pushReg (hc08_reg_a, true);
          loadRegFromImm (hc08_reg_a, c);
          storeRegToAop (hc08_reg_a, aop, loffset);
          pullReg (hc08_reg_a);
        }
    }
}


/*--------------------------------------------------------------------------*/
/* storeRegSignToUpperAop - If isSigned is true, the sign bit of register   */
/*                          reg is extended to fill logical offsets loffset */
/*                          and above of asmop aop. Otherwise, logical      */
/*                          offsets loffset and above of asmop aop are      */
/*                          zeroed. reg must be an 8-bit register.          */
/*--------------------------------------------------------------------------*/
static void
storeRegSignToUpperAop (reg_info * reg, asmop * aop, int loffset, bool isSigned)
{
//  int regidx = reg->rIdx;
  int size = aop->size;

  if (size <= loffset)
    return;

  if (!isSigned)
    {
      /* Unsigned case */
      while (loffset < size)
        storeConstToAop (0, aop, loffset++);
    }
  else
    {
      /* Signed case */
      transferRegReg (reg, hc08_reg_a, false);
      emitcode ("rola", "");
      emitcode ("clra", "");
      emitcode ("sbc", "#0");
      regalloc_dry_run_cost += 4;
      hc08_useReg (hc08_reg_a);
      while (loffset < size)
        storeRegToAop (hc08_reg_a, aop, loffset++);
      hc08_freeReg (hc08_reg_a);
    }
}

/*--------------------------------------------------------------------------*/
/* storeRegToFullAop - Store register reg to asmop aop with appropriate     */
/*                     padding and/or truncation as needed. If isSigned is  */
/*                     true, sign extension will take place in the padding. */
/*--------------------------------------------------------------------------*/
static void
storeRegToFullAop (reg_info *reg, asmop *aop, bool isSigned)
{
  int regidx = reg->rIdx;
  int size = aop->size;

  switch (regidx)
    {
    case A_IDX:
    case X_IDX:
    case H_IDX:
      storeRegToAop (reg, aop, 0);
      if (size > 1 && isSigned && aop->type == AOP_REG && aop->aopu.aop_reg[0]->rIdx == A_IDX)
        pushReg (hc08_reg_a, true);
      storeRegSignToUpperAop (reg, aop, 1, isSigned);
      if (size > 1 && isSigned && aop->type == AOP_REG && aop->aopu.aop_reg[0]->rIdx == A_IDX)
        pullReg (hc08_reg_a);
      break;
    case HX_IDX:
      if (size == 1)
        {
          storeRegToAop (hc08_reg_x, aop, 0);
        }
      else
        {
          storeRegToAop (reg, aop, 0);
          storeRegSignToUpperAop (hc08_reg_h, aop, 2, isSigned);
        }
      break;
    case XA_IDX:
      if (size == 1)
        {
          storeRegToAop (hc08_reg_a, aop, 0);
        }
      else
        {
          storeRegToAop (reg, aop, 0);
          storeRegSignToUpperAop (hc08_reg_x, aop, 2, isSigned);
        }
      break;
    default:
      wassert (0);
    }
}

/*--------------------------------------------------------------------------*/
/* transferAopAop - Transfer the value at logical offset srcofs of asmop    */
/*                  srcaop to logical offset dstofs of asmop dstaop.        */
/*--------------------------------------------------------------------------*/
static void
transferAopAop (asmop *srcaop, int srcofs, asmop *dstaop, int dstofs)
{
  bool needpula = false;
  reg_info *reg = NULL;
  bool keepreg = false;
  bool afree;

  wassert (srcaop && dstaop);

  /* ignore transfers at the same byte, unless its volatile */
  if (srcaop->op && !isOperandVolatile (srcaop->op, false)
      && dstaop->op && !isOperandVolatile (dstaop->op, false)
      && operandsEqu (srcaop->op, dstaop->op) && srcofs == dstofs && dstaop->type == srcaop->type)
    return;

  if (srcaop->stacked && srcaop->stk_aop[srcofs])
    {
      transferAopAop (srcaop->stk_aop[srcofs], 0, dstaop, dstofs);
      return;
    }

  if (dstaop->stacked && dstaop->stk_aop[srcofs])
    {
      transferAopAop (srcaop, srcofs, dstaop->stk_aop[dstofs], 0);
      return;
    }

//  DD(emitcode ("", "; transferAopAop (%s, %d, %s, %d)",
//            aopName (srcaop), srcofs, aopName (dstaop), dstofs));
//  DD(emitcode ("", "; srcaop->type = %d", srcaop->type));
//  DD(emitcode ("", "; dstaop->type = %d", dstaop->type));

  if (dstofs >= dstaop->size)
    return;

  if (srcaop->type == AOP_LIT)
    {
      storeConstToAop (byteOfVal (srcaop->aopu.aop_lit, srcofs), dstaop, dstofs);
      return;
    }
    
  if ((dstaop->type == AOP_DIR) && (srcaop->type == AOP_DIR))
    {
      const char *src = aopAdrStr (srcaop, srcofs, false);
      /* mov src,dst : 3 bytes, 5 cycles */
      emitcode ("mov", "%s,%s", src, aopAdrStr (dstaop, dstofs, false));
      regalloc_dry_run_cost += 3;
      return;
    }

  if (dstaop->type == AOP_REG)
    {
      reg = dstaop->aopu.aop_reg[dstofs];
      keepreg = true;
    }
  else if ((srcaop->type == AOP_REG) && (srcaop->aopu.aop_reg[srcofs]))
    {
      reg = srcaop->aopu.aop_reg[srcofs];
      keepreg = true;
    }

  afree = hc08_reg_a->isFree;

  if (!reg)
    {
      if (hc08_reg_a->isFree)
        reg = hc08_reg_a;
      else if (hc08_reg_x->isFree)
        reg = hc08_reg_x;
      else
        {
          pushReg (hc08_reg_a, true);
          needpula = true;
          reg = hc08_reg_a;
        }
    }

  loadRegFromAop (reg, srcaop, srcofs);
  storeRegToAop (reg, dstaop, dstofs);

  if (!keepreg)
    pullOrFreeReg (hc08_reg_a, needpula);

  hc08_reg_a->isFree = afree;
}


/*--------------------------------------------------------------------------*/
/* accopWithMisc - Emit accumulator modifying instruction accop with the    */
/*                 parameter param.                                         */
/*--------------------------------------------------------------------------*/
static void
accopWithMisc (char *accop, char *param)
{
  emitcode (accop, "%s", param);
  regalloc_dry_run_cost += ((!param[0] || !strcmp(param, ",x")) ? 1 : ((param[0]=='#' || param[0]=='*') ? 2 : 3));
  if (strcmp (accop, "bit") && strcmp (accop, "cmp") && strcmp (accop, "cpx"))
    hc08_dirtyReg (hc08_reg_a, false);
}

/*--------------------------------------------------------------------------*/
/* accopWithAop - Emit accumulator modifying instruction accop with the     */
/*                byte at logical offset loffset of asmop aop.              */
/*                Supports: adc, add, and, bit, cmp, eor, ora, sbc, sub     */
/*--------------------------------------------------------------------------*/
static void
accopWithAop (char *accop, asmop *aop, int loffset)
{
  if (aop->stacked && aop->stk_aop[loffset])
    {
      accopWithAop (accop, aop->stk_aop[loffset], 0);
      return;
    }

  if (aop->type == AOP_DUMMY)
    return;

  if (aop->type == AOP_REG)
    {
  if (loffset < aop->size)
    {
      pushReg (aop->aopu.aop_reg[loffset], false);
      emitcode (accop, "1,s");
      regalloc_dry_run_cost += 3;
      pullNull (1);
    } else {
      emitcode (accop, "#0");
      regalloc_dry_run_cost += 2;
    }
    }
  else
    {
      emitcode (accop, "%s", aopAdrStr (aop, loffset, false));
      if (aop->type == AOP_DIR || aop->type == AOP_LIT)
        regalloc_dry_run_cost +=2;
      else
        regalloc_dry_run_cost += 3;
    }

  if (strcmp (accop, "bit") && strcmp (accop, "cmp") && strcmp (accop, "cpx"))
    hc08_dirtyReg (hc08_reg_a, false);
}


/*--------------------------------------------------------------------------*/
/* rmwWithReg - Emit read/modify/write instruction rmwop with register reg. */
/*              byte at logical offset loffset of asmop aop. Register reg   */
/*              must be 8-bit.                                              */
/*              Supports: com, dec, inc, lsl, lsr, neg, rol, ror            */
/*--------------------------------------------------------------------------*/
static void
rmwWithReg (char *rmwop, reg_info * reg)
{
  char rmwbuf[10];
  char *rmwaop = rmwbuf;

  if (reg->rIdx == A_IDX)
    {
      sprintf (rmwaop, "%sa", rmwop);
      emitcode (rmwaop, "");
      regalloc_dry_run_cost++;
      hc08_dirtyReg (hc08_reg_a, false);
    }
  else if (reg->rIdx == X_IDX)
    {
      sprintf (rmwaop, "%sx", rmwop);
      emitcode (rmwaop, "");
      regalloc_dry_run_cost++;
      hc08_dirtyReg (hc08_reg_x, false);
    }
  else
    {
      pushReg (reg, false);
      emitcode (rmwop, "1,s");
      regalloc_dry_run_cost += 3;
      pullReg (reg);
      hc08_dirtyReg (reg, false);
    }
}

/*--------------------------------------------------------------------------*/
/* rmwWithAop - Emit read/modify/write instruction rmwop with the byte at   */
/*                logical offset loffset of asmop aop.                      */
/*                Supports: com, dec, inc, lsl, lsr, neg, rol, ror, tst     */
/*--------------------------------------------------------------------------*/
static void
rmwWithAop (char *rmwop, asmop * aop, int loffset)
{
  bool needpull = false;
  reg_info * reg;

  if (aop->stacked && aop->stk_aop[loffset])
    {
      rmwWithAop (rmwop, aop->stk_aop[loffset], 0);
      return;
    }

  /* If we need a register: */
  /*   use A if it's free,  */
  /*   otherwise use X if it's free */
  /*   otherwise use A (and preserve original value via the stack) */
  if (!hc08_reg_a->isFree && hc08_reg_x->isFree)
    reg = hc08_reg_x;
  else
    reg = hc08_reg_a;

  switch (aop->type)
    {
    case AOP_REG:
      rmwWithReg (rmwop, aop->aopu.aop_reg[loffset]);
      break;
    case AOP_EXT:
      needpull = pushRegIfUsed (reg);
      loadRegFromAop (reg, aop, loffset);
      rmwWithReg (rmwop, reg);
      if (strcmp ("tst", rmwop))
        storeRegToAop (reg, aop, loffset);
      pullOrFreeReg (reg, needpull);
      break;
    case AOP_DUMMY:
      break;
    case AOP_SOF:
      {
        int offset = aop->size - 1 - loffset;
        offset += _G.stackOfs + _G.stackPushes + aop->aopu.aop_stk + 1;
        if ((offset > 0xff) || (offset < 0))
          {
            /* Unfortunately, the rmw class of instructions only support a */
            /* single byte stack pointer offset and we need two. */
            needpull = pushRegIfUsed (reg);
            loadRegFromAop (reg, aop, loffset);
            rmwWithReg (rmwop, reg);
            if (strcmp ("tst", rmwop))
              storeRegToAop (reg, aop, loffset);
            pullOrFreeReg (reg, needpull);
            break;
          }
        /* If the offset is small enough, fall through to default case */
      }
    default:
      emitcode (rmwop, "%s", aopAdrStr (aop, loffset, false));
      regalloc_dry_run_cost += ((aop->type == AOP_DIR || aop->type == AOP_IMMD) ? 2 : 3);
    }

}

/*--------------------------------------------------------------------------*/
/* loadRegIndexed - Load a register using indexed addressing mode.          */
/*                  NOTE: offset is physical (not logical)                  */
/*--------------------------------------------------------------------------*/
static void
loadRegIndexed (reg_info * reg, int offset, char * rematOfs)
{
  bool needpula = false;

  /* The rematerialized offset may have a "#" prefix; skip over it */
  if (rematOfs && rematOfs[0] == '#')
    rematOfs++;
  if (rematOfs && !rematOfs[0])
    rematOfs = NULL;

  /* force offset to signed 16-bit range */
  offset &= 0xffff;
  if (offset & 0x8000)
    offset = 0x10000 - offset;

  switch (reg->rIdx)
    {
    case A_IDX:
      if (rematOfs)
        {
          if (!offset)
            emitcode ("lda", "(%s),x", rematOfs);
          else
            emitcode ("lda", "(%s+%d),x", rematOfs, offset);
          regalloc_dry_run_cost += 3;
        }
      else if (offset)
        {
          emitcode ("lda", "%d,x", offset);
          if (offset > 1 && offset <= 0xff)
            regalloc_dry_run_cost += 2;
          else
            regalloc_dry_run_cost += 3;
        }
      else
        {
          emitcode ("lda", ",x");
          regalloc_dry_run_cost++;
        }
      hc08_dirtyReg (reg, false);
      break;
    case X_IDX:
      if (rematOfs)
        {
          if (!offset)
            emitcode ("ldx", "(%s),x", rematOfs);
          else
            emitcode ("ldx", "(%s+%d),x", rematOfs, offset);
          regalloc_dry_run_cost += 3;
        }
      else if (offset)
        {
          emitcode ("ldx", "%d,x", offset);
          if (offset > 1 && offset <= 0xff)
            regalloc_dry_run_cost += 2;
          else
            regalloc_dry_run_cost += 3;
        }
      else
        {
          emitcode ("ldx", ",x");
          regalloc_dry_run_cost++;
        }
      hc08_dirtyReg (reg, false);
      break;
    case H_IDX:
      needpula = pushRegIfUsed (hc08_reg_a);
      loadRegIndexed (hc08_reg_a, offset, rematOfs);
      transferRegReg (hc08_reg_a, hc08_reg_h, true);
      pullOrFreeReg (hc08_reg_a, needpula);
      break;
    case HX_IDX:
      if (!IS_S08)
        {
          needpula = pushRegIfUsed (hc08_reg_a);
          loadRegIndexed (hc08_reg_a, offset, rematOfs);
          pushReg (hc08_reg_a, true);
          loadRegIndexed (hc08_reg_x, offset+1, rematOfs);
          pullReg (hc08_reg_h);
          pullOrFreeReg (hc08_reg_a, needpula);
          break;
        }
      else if (rematOfs)
        {
          if (!offset)
            emitcode ("ldhx", "(%s),x", rematOfs);
          else
            emitcode ("ldhx", "(%s+%d),x", rematOfs, offset);
          regalloc_dry_run_cost += 4;
        }
      else if (offset)
        {
          emitcode ("ldhx", "%d,x", offset);
          if (offset > 1 && offset <= 0xff)
            regalloc_dry_run_cost += 3;
          else
            regalloc_dry_run_cost += 4;
        }
      else
        {
          emitcode ("ldhx", ",x");
          regalloc_dry_run_cost += 2;
        }
      hc08_dirtyReg (reg, false);
      break;
    case XA_IDX:
      loadRegIndexed (hc08_reg_a, offset+1, rematOfs);
      loadRegIndexed (hc08_reg_x, offset, rematOfs);
      break;
    default:
      wassert (0);
    }
}

/*--------------------------------------------------------------------------*/
/* storeRegIndexed - Store a register using indexed addressing mode.        */
/*                   NOTE: offset is physical (not logical)                 */
/*--------------------------------------------------------------------------*/
static void
storeRegIndexed (reg_info * reg, int offset, char * rematOfs)
{
  bool needpula = false;

  /* The rematerialized offset may have a "#" prefix; skip over it */
  if (rematOfs && rematOfs[0] == '#')
    rematOfs++;
  if (rematOfs && !rematOfs[0])
    rematOfs = NULL;

  /* force offset to signed 16-bit range */
  offset &= 0xffff;
  if (offset & 0x8000)
    offset = offset - 0x10000;

  switch (reg->rIdx)
    {
    case A_IDX:
      if (rematOfs)
        {
          if (!offset)
            emitcode ("sta", "(%s),x", rematOfs);
          else
            emitcode ("sta", "(%s+%d),x", rematOfs, offset);
          regalloc_dry_run_cost += 3;
        }
      else if (offset)
        {
          emitcode ("sta", "%d,x", offset);
          if (offset > 1 && offset <= 0xff)
            regalloc_dry_run_cost += 2;
          else
            regalloc_dry_run_cost += 3;
        }
      else
        {
          emitcode ("sta", ",x");
          regalloc_dry_run_cost++;
        }
      break;
    case X_IDX:
      if (rematOfs)
        {
          if (!offset)
            emitcode ("stx", "(%s),x", rematOfs);
          else
            emitcode ("stx", "(%s+%d),x", rematOfs, offset);
          regalloc_dry_run_cost += 3;
        }
      else if (offset)
        {
          emitcode ("stx", "%d,x", offset);
          if (offset > 1 && offset <= 0xff)
            regalloc_dry_run_cost += 2;
          else
            regalloc_dry_run_cost += 3;
        }
      else
        {
          emitcode ("stx", ",x");
          regalloc_dry_run_cost++;
        }
      break;
    case H_IDX:
      needpula = pushRegIfUsed (hc08_reg_a);
      transferRegReg (hc08_reg_h, hc08_reg_a, true);
      storeRegIndexed (hc08_reg_a, offset, rematOfs);
      pullOrFreeReg (hc08_reg_a, needpula);
      break;
    case HX_IDX:
      storeRegIndexed (hc08_reg_h, offset, rematOfs);
      storeRegIndexed (hc08_reg_x, offset+1, rematOfs);
      break;
    case XA_IDX:
      /* This case probably won't happen, but it's easy to implement */
      storeRegIndexed (hc08_reg_x, offset, rematOfs);
      storeRegIndexed (hc08_reg_a, offset+1, rematOfs);
      break;
    default:
      wassert (0);
    }
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
  aop->op = NULL;
  aop->valinfo.anything = true;
  return aop;
}


/*-----------------------------------------------------------------*/
/* operandConflictsWithHX - true if operand in h and/or x register */
/*-----------------------------------------------------------------*/
static bool
operandConflictsWithHX (operand *op)
{
  symbol *sym;
  int i;

  if (IS_ITEMP (op))
    {
      sym = OP_SYMBOL (op);
      if (!sym->isspilt)
        {
          for(i = 0; i < sym->nRegs; i++)
            if (sym->regs[i] == hc08_reg_h || sym->regs[i] == hc08_reg_x)
              return true;
        }
    }

  return false;
}

/*-----------------------------------------------------------------*/
/* operandOnStack - returns True if operand is on the stack        */
/*-----------------------------------------------------------------*/
static bool
operandOnStack(operand *op)
{
  symbol *sym;

  if (!op || !IS_SYMOP (op))
    return false;
  sym = OP_SYMBOL (op);
  if (!sym->isspilt && sym->onStack)
    return true;
  if (sym->isspilt)
    {
      sym = sym->usl.spillLoc;
      if (sym && sym->onStack)
        return true;
    }
  return false;
}

/*-----------------------------------------------------------------*/
/* tsxUseful - returns True if tsx could help at least two         */
/*             anticipated stack references                        */
/*-----------------------------------------------------------------*/
static bool
tsxUseful(iCode *ic)
{
  int uses = 0;

  if (ic->op == CALL)
    {
      if (IC_RESULT (ic) && operandSize (IC_RESULT (ic)) < 2 && operandOnStack (IC_RESULT (ic)))
        {
          uses++;
          ic = ic->next;
        }
    }

  while (ic && uses < 2)
    {
      if (ic->op == IFX)
        {
          if (operandOnStack (IC_COND (ic)))
            uses += operandSize(IC_COND (ic));
          break;
        }
      else if (ic->op == JUMPTABLE)
        {
          if (operandOnStack (IC_JTCOND (ic)))
            uses++;
          break;
        }
      else if (ic->op == ADDRESS_OF)
        {
          if (operandOnStack (IC_RIGHT (ic)))
            break;
        }
      else if (ic->op == LABEL || ic->op == GOTO || ic->op == CALL || ic->op == PCALL)
        break;
      else if (POINTER_SET (ic) || POINTER_GET (ic))
        break;
      else
        {
          if (operandConflictsWithHX (IC_RESULT (ic)))
            break;
          if (operandOnStack (IC_LEFT (ic)))
            uses += operandSize (IC_LEFT (ic));
          if (operandOnStack (IC_RIGHT (ic)))
            uses += operandSize (IC_RIGHT (ic));
          if (operandOnStack (IC_RESULT (ic)))
            uses += operandSize (IC_RESULT (ic));
        }

      ic = ic->next;
    }

  return uses>=2;
}


/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (iCode * ic, symbol * sym, bool result)
{
  asmop *aop;
  memmap *space;

  wassertl (ic != NULL, "Got a null iCode");
  wassertl (sym != NULL, "Got a null symbol");

//  printf("in aopForSym for symbol %s\n", sym->name);

  space = SPEC_OCLS (sym->etype);

  /* if already has one */
  if (sym->aop)
    {
      return sym->aop;
    }

  /* special case for a function */
  if (IS_FUNC (sym->type))
    {
      sym->aop = aop = newAsmop (AOP_IMMD);
      aop->aopu.aop_immd = Safe_calloc (1, strlen (sym->rname) + 1);
      strcpy (aop->aopu.aop_immd, sym->rname);
      aop->size = FARPTRSIZE;
      return aop;
    }

  /* if it is on the stack */
  if (sym->onStack)
    {
      sym->aop = aop = newAsmop (AOP_SOF);
//      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
      aop->aopu.aop_stk = sym->stack;

      if (!regalloc_dry_run && hc08_reg_hx->isFree && hc08_reg_hx->aop != &tsxaop)
        {
          if (!hc08_reg_h->isDead || !hc08_reg_x->isDead)
            return aop;
          if (ic->op == IFX && operandConflictsWithHX (IC_COND (ic)))
            return aop;
          else if (ic->op == JUMPTABLE && operandConflictsWithHX (IC_JTCOND (ic)))
            return aop;
          else
            {
              /* If this is a pointer gen/set, then hx is definitely in use */
              if (POINTER_SET (ic) || POINTER_GET (ic))
                return aop;
              if (ic->op == ADDRESS_OF)
                return aop;
              if (operandConflictsWithHX (IC_LEFT (ic)))
                return aop;
              if (operandConflictsWithHX (IC_RIGHT (ic)))
                return aop;
            }
          /* It's safe to use tsx here. tsx costs 1 byte and 2 cycles but */
          /* can save us 1 byte and 1 cycle for each time we can use x    */
          /* instead of sp. For a single use, we break even on bytes, but */
          /* lose a cycle. Make sure there are at least two uses.         */
          if (!tsxUseful (ic))
            return aop;
          emitcode ("tsx", "");
          hc08_dirtyReg (hc08_reg_hx, false);
          hc08_reg_hx->aop = &tsxaop;
          _G.tsxStackPushes = _G.stackPushes;
        }
      return aop;
    }

  /* if it is in direct space */
  if (IN_DIRSPACE (space))
    {
      sym->aop = aop = newAsmop (AOP_DIR);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
      return aop;
    }

  /* default to far space */
  sym->aop = aop = newAsmop (AOP_EXT);
  aop->aopu.aop_dir = sym->rname;
  aop->size = getSize (sym->type);
  return aop;
}

/*-----------------------------------------------------------------*/
/* aopForRemat - rematerializes an object                          */
/*-----------------------------------------------------------------*/
static asmop *
aopForRemat (symbol * sym)
{
  iCode *ic = sym->rematiCode;
  asmop *aop = NULL;
  int val = 0;

  if (!ic)
    {
      fprintf (stderr, "Symbol %s to be rematerialized, but has no rematiCode.\n", sym->name);
      wassert (0);
    }

  for (;;)
    {
      if (ic->op == '+')
        val += (int) operandLitValue (IC_RIGHT (ic));
      else if (ic->op == '-')
        val -= (int) operandLitValue (IC_RIGHT (ic));
      else if (IS_CAST_ICODE (ic))
        {
          ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
          continue;
        }
      else
        break;

      ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
    }

  if (ic->op == ADDRESS_OF)
    {
      if (val)
        {
          SNPRINTF (buffer, sizeof (buffer),
                    "(%s %c 0x%04x)", OP_SYMBOL (IC_LEFT (ic))->rname, val >= 0 ? '+' : '-', abs (val) & 0xffff);
        }
      else
        {
          strncpyz (buffer, OP_SYMBOL (IC_LEFT (ic))->rname, sizeof (buffer));
        }

      aop = newAsmop (AOP_IMMD);
      aop->aopu.aop_immd = Safe_strdup (buffer);
    }
  else if (ic->op == '=')
    {
      val += (int) operandLitValue (IC_RIGHT (ic));
      val &= 0xffff;
      SNPRINTF (buffer, sizeof (buffer), "0x%04x", val);
      aop = newAsmop (AOP_LIT);
      aop->aopu.aop_lit = constVal (buffer);
    }
  else
    {
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "unexpected rematerialization");
    }

  return aop;
}

/*-----------------------------------------------------------------*/
/* regsInCommon - two operands have some registers in common       */
/*-----------------------------------------------------------------*/
static bool
regsInCommon (operand * op1, operand * op2)
{
  symbol *sym1, *sym2;
  int i;

  /* if they have registers in common */
  if (!IS_SYMOP (op1) || !IS_SYMOP (op2))
    return false;

  sym1 = OP_SYMBOL (op1);
  sym2 = OP_SYMBOL (op2);

  if (sym1->nRegs == 0 || sym2->nRegs == 0)
    return false;

  for (i = 0; i < sym1->nRegs; i++)
    {
      int j;
      if (!sym1->regs[i])
        continue;

      for (j = 0; j < sym2->nRegs; j++)
        {
          if (!sym2->regs[j])
            continue;

          if (sym2->regs[j] == sym1->regs[i])
            return true;
        }
    }

  return false;
}

/*-----------------------------------------------------------------*/
/* operandsEqu - equivalent                                        */
/*-----------------------------------------------------------------*/
static bool
operandsEqu (operand *op1, operand *op2)
{
  symbol *sym1, *sym2;

  /* if they not symbols */
  if (!IS_SYMOP (op1) || !IS_SYMOP (op2))
    return false;

  sym1 = OP_SYMBOL (op1);
  sym2 = OP_SYMBOL (op2);

  /* if both are itemps & one is spilt
     and the other is not then false */
  if (IS_ITEMP (op1) && IS_ITEMP (op2) && sym1->isspilt != sym2->isspilt)
    return false;

  /* if they are the same */
  if (sym1 == sym2)
    return true;

  /* if they have the same rname */
  if (sym1->rname[0] && sym2->rname[0] && strcmp (sym1->rname, sym2->rname) == 0)
    return true;

  /* if left is a tmp & right is not */
  if (IS_ITEMP (op1) && !IS_ITEMP (op2) && sym1->isspilt && (sym1->usl.spillLoc == sym2))
    return true;

  if (IS_ITEMP (op2) && !IS_ITEMP (op1) && sym2->isspilt && sym1->level > 0 && (sym2->usl.spillLoc == sym1))
    return true;

  return false;
}

/*-----------------------------------------------------------------*/
/* sameRegs - two asmops have the same registers                   */
/*-----------------------------------------------------------------*/
static bool
sameRegs (asmop *aop1, asmop *aop2)
{
  int i;

  if (aop1 == aop2)
    return true;

//  if (aop1->size != aop2->size)
//    return false;

  if (aop1->type == aop2->type && aop1->size == aop2->size)
    {
      switch (aop1->type)
        {
        case AOP_REG:
          for (i = 0; i < aop1->size; i++)
            if (aop1->aopu.aop_reg[i] != aop2->aopu.aop_reg[i])
              return false;
          return true;
        case AOP_SOF:
          return (aop1->aopu.aop_stk == aop2->aopu.aop_stk);
        case AOP_DIR:
          if (regalloc_dry_run)
            return false;
        case AOP_EXT:
          return (!strcmp (aop1->aopu.aop_dir, aop2->aopu.aop_dir));
        default:
          break;
        }
    }

  return false;
}

/*-----------------------------------------------------------------*/
/* aopOp - allocates an asmop for an operand  :                    */
/*-----------------------------------------------------------------*/
static void
aopOp (operand *op, iCode * ic, bool result)
{
  asmop *aop = NULL;
  symbol *sym;
  int i;

  if (!op)
    return;

  // Is this a pointer set result?
  //
  if ((op == IC_RESULT (ic)) && POINTER_SET (ic))
    {
    }

//  printf("checking literal\n");
  /* if this a literal */
  if (IS_OP_LITERAL (op))
    {
      op->aop = aop = newAsmop (AOP_LIT);
      aop->aopu.aop_lit = OP_VALUE (op);
      aop->size = getSize (operandType (op));
      aop->op = op;
      if (!result)
        aop->valinfo = getOperandValinfo (ic, op);
      return;
    }

//  printf("checking pre-existing\n");
  /* if already has a asmop then continue */
  if (op->aop)
    {
      op->aop->op = op;
      return;
    }

//  printf("checking underlying sym\n");
  /* if the underlying symbol has a aop */
  if (IS_SYMOP (op) && OP_SYMBOL (op)->aop)
    {
      op->aop = aop = Safe_calloc (1, sizeof (*aop));
      memcpy (aop, OP_SYMBOL (op)->aop, sizeof (*aop));
      //op->aop = aop = OP_SYMBOL (op)->aop;
      aop->size = getSize (operandType (op));
      //printf ("reusing underlying symbol %s\n",OP_SYMBOL (op)->name);
      //printf (" with size = %d\n", aop->size);

      aop->op = op;
      return;
    }

//  printf("checking true sym\n");
  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aop = aopForSym (ic, OP_SYMBOL (op), result);
      aop->op = op;
      if (!result)
        aop->valinfo = getOperandValinfo (ic, op);
      //printf ("new symbol %s\n", OP_SYMBOL (op)->name);
      //printf (" with size = %d\n", aop->size);
      return;
    }

  /* this is a temporary : this has
     only five choices :
     a) register
     b) spillocation
     c) rematerialize
     d) conditional
     e) can be a return use only */

  if (!IS_SYMOP (op))
    piCode (ic, NULL);
  sym = OP_SYMBOL (op);

//  printf("checking conditional\n");
  /* if the type is a conditional */
  if (sym->regType == REG_CND)
    {
      sym->aop = op->aop = aop = newAsmop (AOP_CRY);
      aop->size = 0;
      aop->op = op;
      if (!result)
        aop->valinfo = getOperandValinfo (ic, op);
      return;
    }

//  printf("checking spilt\n");
  /* if it is spilt then two situations
     a) is rematerialize
     b) has a spill location */
  if (sym->isspilt || sym->nRegs == 0)
    {
//      printf("checking remat\n");
      /* rematerialize it NOW */
      if (sym->remat)
        {
          sym->aop = op->aop = aop = aopForRemat (sym);
          aop->size = getSize (sym->type);
          aop->op = op;
          if (!result)
            aop->valinfo = getOperandValinfo (ic, op);
          return;
        }

       wassertl (!sym->ruonly, "sym->ruonly not supported");

       if (regalloc_dry_run)     // Todo: Handle dummy iTemp correctly
        {
          if (options.stackAuto || (currFunc && IFFUNC_ISREENT (currFunc->type)))
            {
              sym->aop = op->aop = aop = newAsmop (AOP_SOF);
              aop->aopu.aop_stk = 8; /* bogus stack offset, high enough to prevent optimization */
            }
          else
            sym->aop = op->aop = aop = newAsmop (AOP_DIR);
          aop->size = getSize (sym->type);
          aop->op = op;
          if (!result)
            aop->valinfo = getOperandValinfo (ic, op);
          return;
        }

      /* else spill location  */
      if (sym->isspilt && sym->usl.spillLoc || regalloc_dry_run)
        {
          asmop *oldAsmOp = NULL;

          if (sym->usl.spillLoc->aop && sym->usl.spillLoc->aop->size != getSize (sym->type))
            {
              /* force a new aop if sizes differ */
              oldAsmOp = sym->usl.spillLoc->aop;
              sym->usl.spillLoc->aop = NULL;
              //printf ("forcing new aop\n");
            }
          sym->aop = op->aop = aop = aopForSym (ic, sym->usl.spillLoc, result);
          if (sym->usl.spillLoc->aop->size != getSize (sym->type))
            {
              /* Don't reuse the new aop, go with the last one */
              sym->usl.spillLoc->aop = oldAsmOp;
            }
          aop->size = getSize (sym->type);
          aop->op = op;
          //printf ("spill symbol %s\n", OP_SYMBOL (op)->name);
          //printf (" with size = %d\n", aop->size);
          if (!result)
            aop->valinfo = getOperandValinfo (ic, op);
          return;
        }

      /* else must be a dummy iTemp */
      sym->aop = op->aop = aop = newAsmop (AOP_DUMMY);
      aop->size = getSize (sym->type);
      aop->op = op;
      if (!result)
        aop->valinfo = getOperandValinfo (ic, op);
      return;
    }

//  printf("assuming register\n");
  /* must be in a register */
  wassert (sym->nRegs);
  sym->aop = op->aop = aop = newAsmop (AOP_REG);
  aop->size = sym->nRegs;
  for (i = 0; i < sym->nRegs; i++)
    {
       wassert (sym->regs[i] >= regshc08 && sym->regs[i] < regshc08 + 3);
       wassertl (sym->regs[i], "Symbol in register, but no register assigned.");
       aop->aopu.aop_reg[i] = sym->regs[i];
       aop->regmask |= sym->regs[i]->mask;
    }
  if ((sym->nRegs > 1) && (sym->regs[0]->mask > sym->regs[1]->mask))
    aop->regmask |= HC08MASK_REV;
  aop->op = op;
  if (!result)
    aop->valinfo = getOperandValinfo (ic, op);
}

/*-----------------------------------------------------------------*/
/* freeAsmop - free up the asmop given to an operand               */
/*-----------------------------------------------------------------*/
static void
freeAsmop (operand * op, asmop * aaop, iCode * ic, bool pop)
{
  asmop *aop;

  if (!op)
    aop = aaop;
  else
    aop = op->aop;

  if (!aop)
    return;

  if (aop->freed)
    goto dealloc;

  aop->freed = 1;

  if (aop->stacked)
    {
      int stackAdjust;
      int loffset;

      DD (emitcode ("", "; freeAsmop restoring stacked %s", aopName (aop)));
      aop->stacked = 0;
      stackAdjust = 0;
      for (loffset = 0; loffset < aop->size; loffset++)
        if (aop->stk_aop[loffset])
          {
            transferAopAop (aop->stk_aop[loffset], 0, aop, loffset);
            stackAdjust++;
          }
      pullNull (stackAdjust);
    }

dealloc:
  /* all other cases just dealloc */
  if (op)
    {
      op->aop = NULL;
      if (IS_SYMOP (op))
        {
          OP_SYMBOL (op)->aop = NULL;
          /* if the symbol has a spill */
          if (SPIL_LOC (op))
            SPIL_LOC (op)->aop = NULL;
        }
    }
}


/*-----------------------------------------------------------------*/
/* aopDerefAop - treating the aop parameter as a pointer, return   */
/*               an asmop for the object it references             */
/*-----------------------------------------------------------------*/
asmop *
aopDerefAop (asmop * aop, int offset)
{
  int adr;
  asmop *newaop = NULL;
  sym_link *type, *etype;
  int p_type;
  struct dbuf_s dbuf;

  DD (emitcode ("", ";     aopDerefAop(%s)", aopName (aop)));
  if (aop->op)
    {

      type = operandType (aop->op);
      etype = getSpec (type);
      /* if op is of type of pointer then it is simple */
      if (IS_PTR (type) && !IS_FUNC (type->next))
        p_type = DCL_TYPE (type);
      else
        {
          /* we have to go by the storage class */
          p_type = PTR_TYPE (SPEC_OCLS (etype));
        }
    }
  else
    p_type = UPOINTER;

  switch (aop->type)
    {
    case AOP_IMMD:
      if (p_type == POINTER)
        newaop = newAsmop (AOP_DIR);
      else
        newaop = newAsmop (AOP_EXT);
      if (!offset)
        newaop->aopu.aop_dir = aop->aopu.aop_immd;
      else
        {
          dbuf_init (&dbuf, 64);
          dbuf_printf (&dbuf, "(%s+%d)", aop->aopu.aop_immd, offset);
          newaop->aopu.aop_dir = dbuf_detach_c_str (&dbuf);
        }
      break;
    case AOP_LIT:
      adr = (int) ulFromVal (aop->aopu.aop_lit);
      if (p_type == POINTER)
        adr &= 0xff;
      adr = (adr + offset) & 0xffff;
      dbuf_init (&dbuf, 64);

      if (adr < 0x100)
        {
          newaop = newAsmop (AOP_DIR);
          dbuf_printf (&dbuf, "0x%02x", adr);
        }
      else
        {
          newaop = newAsmop (AOP_EXT);
          dbuf_printf (&dbuf, "0x%04x", adr);
        }
      newaop->aopu.aop_dir = dbuf_detach_c_str (&dbuf);
      break;
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "unsupported asmop");
      return NULL;
    }

  return newaop;
}


/*-----------------------------------------------------------------*/
/* aopOpExtToIdx - attempt to convert AOP_EXT to AOP_IDX           */
/*-----------------------------------------------------------------*/
static void
aopOpExtToIdx(asmop * result, asmop *left, asmop *right)
{
  int accesses=0;
  int resultAccesses=0;
  int leftAccesses=0;
  int rightAccesses=0;
  asmop * winner;
  int winnerAccesses;

  if (!hc08_reg_x->isFree || !hc08_reg_h->isFree)
    return;

  /* Need to replace at least two extended mode accesses with indexed */
  /* to break even with the extra cost of loading HX. Do a quick check */
  /* to see if anything is using extended mode at all. */
  if (result && result->type == AOP_EXT)
    accesses += result->size;
  if (left && left->type == AOP_EXT)
    accesses += left->size;
  if (right && right->type == AOP_EXT)
    accesses += right->size;
  if (accesses<2)
    return;
  
  /* If an operand is already using or going to H or X then we cannot */
  /* use indexed addressing mode at the same time. */
  if (result && (IS_AOP_WITH_H (result) || IS_AOP_WITH_X (result)))
    return;
  if (left && (IS_AOP_WITH_H (left) || IS_AOP_WITH_X (left)))
    return;
  if (right && (IS_AOP_WITH_H (right) || IS_AOP_WITH_X (right)))
    return;

  /* Decide which is the best asmop to make indexed. */
  if (result && result->type == AOP_EXT)
    {
      resultAccesses = result->size;
      if (result->op && left && left->op && result->op->key == left->op->key)
        resultAccesses += result->size;
      if (result->op && right && right->op && result->op->key == right->op->key)
        resultAccesses += result->size;
    }
  if (left && left->type == AOP_EXT)
    {
      leftAccesses = left->size;
      if (left->op && right && right->op && left->op->key == right->op->key)
        leftAccesses += left->size;
    }
  if (right && right->type == AOP_EXT)
    {
      rightAccesses = right->size;
    }

  winner = result; winnerAccesses = resultAccesses;
  if (leftAccesses > winnerAccesses)
    {
      winnerAccesses = leftAccesses;
      winner = left;
    }
  if (rightAccesses > winnerAccesses)
    {
      winnerAccesses = rightAccesses;
      winner = right;
    }

  /* Make sure there were enough accesses of a single variable to be worthwhile. */ 
  if (winnerAccesses < 2)
    return;

  if (winner->op && result && result->op && winner->op->key == result->op->key)
    result->type = AOP_IDX;
  if (winner->op && left && left->op && winner->op->key == left->op->key)
    left->type = AOP_IDX;
  if (winner->op && right && right->op && winner->op->key == right->op->key)
    right->type = AOP_IDX;
  loadRegFromImm (hc08_reg_hx, winner->aopu.aop_dir);
}


/*-----------------------------------------------------------------*/
/* aopAdrStr - for referencing the address of the aop              */
/*-----------------------------------------------------------------*/
/* loffset seems to have a weird meaning here. It seems to be nonzero in some places where one would expect an offset to be zero */
static const char *
aopAdrStr (asmop * aop, int loffset, bool bit16)
{
  char *s = buffer;
  char *rs;
  int offset = aop->size - 1 - loffset - (bit16 ? 1 : 0);
  int xofs;

  /* offset is greater than
     size then zero */
  if (loffset > (aop->size - 1) && aop->type != AOP_LIT)
    return zero;
  
  /* depending on type */
  switch (aop->type)
    {
    case AOP_DUMMY:
      return zero;

    case AOP_IMMD:
      if (loffset)
        {
          if (loffset > 1)
            sprintf (s, "#(%s >> %d)", aop->aopu.aop_immd, loffset * 8);
          else
            sprintf (s, "#>%s", aop->aopu.aop_immd);
        }
      else
        sprintf (s, "#%s", aop->aopu.aop_immd);
      rs = Safe_calloc (1, strlen (s) + 1);
      strcpy (rs, s);
      return rs;

    case AOP_DIR:
      if (regalloc_dry_run)
        return "*dry";
      if (offset)
        sprintf (s, "*(%s + %d)", aop->aopu.aop_dir, offset);
      else
        sprintf (s, "*%s", aop->aopu.aop_dir);
      rs = Safe_calloc (1, strlen (s) + 1);
      strcpy (rs, s);
      return rs;

    case AOP_EXT:
      if (regalloc_dry_run)
        return "dry";
      if (offset)
        sprintf (s, "(%s + %d)", aop->aopu.aop_dir, offset);
      else
        sprintf (s, "%s", aop->aopu.aop_dir);
      rs = Safe_calloc (1, strlen (s) + 1);
      strcpy (rs, s);
      return rs;

    case AOP_REG:
      return aop->aopu.aop_reg[loffset]->name;

    case AOP_LIT:
      if (bit16)
        return aopLiteralLong (aop->aopu.aop_lit, loffset, 2);
      else
        return aopLiteral (aop->aopu.aop_lit, loffset);

    case AOP_SOF:
      if (!regalloc_dry_run && hc08_reg_hx->aop == &tsxaop)
        {
          xofs = _G.stackOfs + _G.tsxStackPushes + aop->aopu.aop_stk + offset;
          if (xofs)
            sprintf (s, "%d,x", xofs);
          else
            sprintf (s, ",x");
        }
      else
        sprintf (s, "%d,s", _G.stackOfs + _G.stackPushes + aop->aopu.aop_stk + offset + 1);
      rs = Safe_calloc (1, strlen (s) + 1);
      strcpy (rs, s);
      return rs;
    case AOP_IDX:
      xofs = offset; /* For now, assume hx points to the base address of operand */
      if (xofs)
        {
          if (regalloc_dry_run) /* Don't worry about the exact offset during the dry run */
            return "1,x";
          sprintf (s, "%d,x", xofs);
          rs = Safe_calloc (1, strlen (s) + 1);
          strcpy (rs, s);
          return rs;
        }
      else
        return ",x";
    default:
      break;
    }

  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "aopAdrStr got unsupported aop->type");
  exit (1);
}


/*-----------------------------------------------------------------*/
/* getDataSize - get the operand data size                         */
/*-----------------------------------------------------------------*/
static int
getDataSize (operand *op)
{
  int size;
  size = AOP_SIZE (op);
  return size;
}


/*-----------------------------------------------------------------*/
/* asmopToBool - Emit code to convert an asmop to a boolean.       */
/*               Result left in A (0=false, 1=true) if ResultInA,  */
/*               otherwise result left in Z flag (1=false, 0=true) */
/*-----------------------------------------------------------------*/
static void
asmopToBool (asmop *aop, bool resultInA)
{
  bool isFloat; 
  symbol *tlbl, *tlbl1;
  int size = aop->size;
  bool needpula = false;
  bool flagsonly = true;
  int offset = size - 1;
  sym_link *type;

  wassert (aop);
  type = operandType (AOP_OP (aop));
  isFloat = IS_FLOAT (type);

  if (resultInA)
    hc08_freeReg (hc08_reg_a);

  if (IS_BOOL (type))
    {
      if (resultInA)
        loadRegFromAop (hc08_reg_a, aop, 0);
      else
        rmwWithAop ("tst", aop, 0);
      return;
    }

  if (resultInA && size == 1)
    {
      loadRegFromAop (hc08_reg_a, aop, 0);
      rmwWithReg ("neg", hc08_reg_a);
      loadRegFromConst (hc08_reg_a, 0);
      rmwWithReg ("rol", hc08_reg_a);
      return;
    }

  switch (aop->type)
    {
    case AOP_REG:
      if (IS_AOP_A (aop))
        {
          emitcode ("tsta", "");
          regalloc_dry_run_cost++;
          flagsonly = false;
        }
      else if (IS_AOP_X (aop))
        {
          emitcode ("tstx", "");
          regalloc_dry_run_cost++;
        }
      else if (IS_AOP_H (aop))
        {
          if (hc08_reg_a->isFree)
            {
              transferRegReg (hc08_reg_h, hc08_reg_a, false);
              emitcode ("tsta", "");
              regalloc_dry_run_cost++;
              flagsonly = false;
              hc08_freeReg (hc08_reg_a);
            }
          else if (hc08_reg_x->isFree)
            {
              transferRegReg (hc08_reg_h, hc08_reg_x, false);
              emitcode ("tstx", "");
              regalloc_dry_run_cost++;
              hc08_freeReg (hc08_reg_x);
            }
          else
            {
              emitcode ("pshh", "");
              emitcode ("tst", "1,s");
              regalloc_dry_run_cost += 4;
              if (IS_S08 && optimize.codeSpeed)
                {
                  emitcode ("ais", "#1"); // 2 Bytes, 2 cycles.
                  regalloc_dry_run_cost += 2;
                }
              else
                {
                  emitcode ("pulh", ""); // 1 Byte, 2 (hc08) / 3 (s08) cycles.
                  regalloc_dry_run_cost += 1;
                }
              
            }
        }
      else if (IS_AOP_HX (aop))
        {
          emitcode ("cphx", zero);
          regalloc_dry_run_cost += 3;
        }
      else if (IS_AOP_XA (aop) || IS_AOP_AX (aop))
        {
          symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emitcode ("tsta", "");
          if (!regalloc_dry_run)
            emitcode ("bne", "%05d$", labelKey2num (tlbl->key));
          emitcode ("tstx", "");
          regalloc_dry_run_cost += 4;
          if (!regalloc_dry_run)
            emitLabel (tlbl);
        }
      else
        {
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "Bad rIdx in asmopToBool");
          return;
        }
      break;
    case AOP_EXT:
      if (!resultInA && (size == 1) && !IS_AOP_A (aop) && !hc08_reg_a->isFree && hc08_reg_x->isFree)
        {
          loadRegFromAop (hc08_reg_x, aop, 0);
          break;
        }
      if (resultInA)
        needpula = false;
      else
        needpula = pushRegIfUsed (hc08_reg_a);
      loadRegFromAop (hc08_reg_a, aop, offset--);
      if (isFloat)
        {
          emitcode ("and", "#0x7F");      //clear sign bit
          regalloc_dry_run_cost += 2;
        }
      while (--size)
        accopWithAop ("ora", aop, offset--);
      if (needpula)
        pullReg (hc08_reg_a);
      else
        {
          hc08_freeReg (hc08_reg_a);
          flagsonly = false;
        }
      break;
    case AOP_LIT:
      /* Higher levels should optimize this case away but let's be safe */
      if (ulFromVal (aop->aopu.aop_lit))
        loadRegFromConst (hc08_reg_a, 1);
      else
        loadRegFromConst (hc08_reg_a, 0);
      hc08_freeReg (hc08_reg_a);
      break;
    default:
      if (size == 1)
        {
          if (resultInA)
            {
              loadRegFromAop (hc08_reg_a, aop, 0);
              hc08_freeReg (hc08_reg_a);
              flagsonly = false;
            }
          else
            {
              emitcode ("tst", "%s", aopAdrStr (aop, 0, false));
              regalloc_dry_run_cost += ((aop->type == AOP_DIR || aop->type == AOP_IMMD) ? 2 : 3);
            }
          break;
        }
      else if (size == 2)
        {
          if (hc08_reg_a->isFree)
            {
              loadRegFromAop (hc08_reg_a, aop, 0);
              accopWithAop ("ora", aop, 1);
              hc08_freeReg (hc08_reg_a);
              flagsonly = false;
            }
          else
            {
              tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
              emitcode ("tst", "%s", aopAdrStr (aop, 0, false));
              if (!regalloc_dry_run)
                emitcode ("bne", "%05d$", labelKey2num (tlbl->key));
              emitcode ("tst", "%s", aopAdrStr (aop, 1, false));
              regalloc_dry_run_cost += 4;
              if (!regalloc_dry_run)
                emitLabel (tlbl);
              break;
            }
        }
      else
        {
          needpula = pushRegIfUsed (hc08_reg_a);
          loadRegFromAop (hc08_reg_a, aop, offset--);
          if (isFloat)
            {
              emitcode ("and", "#0x7F");
              regalloc_dry_run_cost += 2;
            }
          while (--size)
            accopWithAop ("ora", aop, offset--);
          if (needpula)
            pullReg (hc08_reg_a);
          else
            {
              hc08_freeReg (hc08_reg_a);
              flagsonly = false;
            }
        }
    }

  if (resultInA)
    {
      tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

      if (flagsonly)
        {
          tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emitBranch ("bne", tlbl1);
          loadRegFromConst (hc08_reg_a, 0);
          emitBranch ("bra", tlbl);
          if (!regalloc_dry_run)
            emitLabel (tlbl1);
          hc08_dirtyReg (hc08_reg_a, false);
          loadRegFromConst (hc08_reg_a, 1);
        }
      else
        {
          emitBranch ("beq", tlbl);
          loadRegFromConst (hc08_reg_a, 1);
        }
      if (!regalloc_dry_run)
        emitLabel (tlbl);
      hc08_dirtyReg (hc08_reg_a, false);
      hc08_useReg (hc08_reg_a);
    }
}

/*-----------------------------------------------------------------*/
/* genCopy - Copy the value from one operand to another            */
/*           The caller is responsible for aopOp and freeAsmop     */
/*-----------------------------------------------------------------*/
static void
genCopy (operand *result, operand *source)
{
  int size = AOP_SIZE (result);
  int srcsize = AOP_SIZE (source);
  int offset = 0;

  /* if they are the same and not volatile */
  if (operandsEqu (result, source) && !isOperandVolatile (result, false) &&
      !isOperandVolatile (source, false))
    return;

  /* The source and destinations may be different size due to optimizations. */
  /* This is not a cast, so there is no need to worry about sign extension. */
  /* When this happens, it is usually just 1 byte source to 2 byte dest, so */
  /* nothing significant to optimize. */
  if (srcsize < size)
    {
      size -= srcsize;
      while (srcsize)
        {
          transferAopAop (AOP (source), offset, AOP (result), offset);
          offset++;
          srcsize--;
        }
      while (size)
        {
          storeConstToAop (0, AOP (result), offset);
          offset++;
          size--;
        }

      return;
    }

  /* if they are the same registers */
  if (sameRegs (AOP (source), AOP (result)))
    return;

  if (IS_AOP_HX (AOP (result)) && srcsize == 2)
    {
      loadRegFromAop (hc08_reg_hx, AOP (source), 0);
      return;
    }
  if (IS_AOP_HX (AOP (source)) && size == 2)
    {
      storeRegToAop (hc08_reg_hx, AOP (result), 0);
      return;
    }

  /* If the result and right are 2 bytes and both in registers, we have to be careful */
  /* to make sure the registers are not overwritten prematurely. */
  if (AOP_SIZE (result) == 2 && AOP (result)->type == AOP_REG && AOP (source)->type == AOP_REG)
    {
      if (AOP (result)->aopu.aop_reg[0] == AOP (source)->aopu.aop_reg[1] &&
          AOP (result)->aopu.aop_reg[1] == AOP (source)->aopu.aop_reg[0])
        {
          pushReg (AOP (source)->aopu.aop_reg[1], true);
          transferAopAop (AOP (source), 0, AOP (result), 0);
          pullReg (AOP (result)->aopu.aop_reg[1]);
        }
      else if (AOP (result)->aopu.aop_reg[0] == AOP (source)->aopu.aop_reg[1])
        {
          transferAopAop (AOP (source), 1, AOP (result), 1);
          transferAopAop (AOP (source), 0, AOP (result), 0);
        }
      else
        {
          transferAopAop (AOP (source), 0, AOP (result), 0);
          transferAopAop (AOP (source), 1, AOP (result), 1);
        }
      return;
    }

   if (IS_HC08 && (size > 2) && size == srcsize /* todo: load adjusted value into hx when srcsize > size to generate better code */)
     aopOpExtToIdx (AOP (result), NULL, AOP (source));

  /* general case */
  bool need_lsb_to_msb_order = false; // Copy in msb to lsb order, if possible, since some multi-byte hardware registers expect this order.
  if ((result->aop->type == AOP_DIR || result->aop->type == AOP_SOF) && // Avoid overwriting still-needed value.
    result->aop->type == source->aop->type)
   {
     bool overlap = false;
     bool result_at_lower_address = false;
     if (result->aop->type == AOP_DIR)
       {
         symbol *rsym = OP_SYMBOL (result);
         symbol *ssym = OP_SYMBOL (source);
         if(rsym && ssym && !strcmp (rsym->rname, ssym->rname))
           {
             overlap = true;
             result_at_lower_address = (result->aop->size < source->aop->size);
           }
       }
     else if (result->aop->type == AOP_SOF)
       {
         // todo.
       }
     else
       wassert (0);
     need_lsb_to_msb_order = (overlap && result_at_lower_address);
   }
  if (need_lsb_to_msb_order)
    {
      wassert (!IS_OP_VOLATILE (result) && !IS_OP_VOLATILE (source));
      offset = 0;
      while (size)
        {
          if (size >= 2 && hc08_reg_h->isDead && hc08_reg_x->isDead &&
            (AOP_TYPE (source) == AOP_IMMD || AOP_TYPE (source) == AOP_LIT || IS_S08 && AOP_TYPE (source) == AOP_EXT) &&
            (AOP_TYPE (result) == AOP_DIR || IS_S08 && AOP_TYPE (result) == AOP_EXT))
            {
              loadRegFromAop (hc08_reg_hx, AOP (source), offset);
              storeRegToAop (hc08_reg_hx, AOP (result), offset);
              hc08_freeReg (hc08_reg_hx);
              offset += 2;
              size -= 2;
            }
          else
            {
              if (AOP_TYPE (source) == AOP_IDX && AOP_TYPE (result) == AOP_DIR)
                {
                  emitcode ("mov", ",x+,%s", aopAdrStr (AOP (result), offset, false));
                  regalloc_dry_run_cost += 2;
                }
              else if (AOP_TYPE (source) == AOP_DIR && AOP_TYPE (result) == AOP_IDX)
                {
                  emitcode ("mov", "%s,x+", aopAdrStr (AOP (source), offset, false));
                  regalloc_dry_run_cost += 2;
                }
              else
                transferAopAop (AOP (source), offset, AOP (result), offset);
              offset++;
              size--;
            }
        }
    }
  else
    {
      offset = size - 1;
      while (size)
        {
          if (size >= 2 && hc08_reg_h->isDead && hc08_reg_x->isDead  &&
            (AOP_TYPE (source) == AOP_IMMD || AOP_TYPE (source) == AOP_LIT ||IS_S08 && AOP_TYPE (source) == AOP_EXT) &&
            (AOP_TYPE (result) == AOP_DIR || IS_S08 && AOP_TYPE (result) == AOP_EXT))
            {
              loadRegFromAop (hc08_reg_hx, AOP (source), offset - 1);
              storeRegToAop (hc08_reg_hx, AOP (result), offset - 1);
              offset -= 2;
              size -= 2;
            }
          else
            {
              if (AOP_TYPE (source) == AOP_IDX && AOP_TYPE (result) == AOP_DIR)
                {
                  emitcode ("mov", ",x+,%s", aopAdrStr (AOP (result), offset, false));
                  regalloc_dry_run_cost += 2;
                }
              else if (AOP_TYPE (source) == AOP_DIR && AOP_TYPE (result) == AOP_IDX)
                {
                  emitcode ("mov", "%s,x+", aopAdrStr (AOP (source), offset, false));
                  regalloc_dry_run_cost += 2;
                }
              else
                transferAopAop (AOP (source), offset, AOP (result), offset);
              offset--;
              size--;
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* genNot - generate code for ! operation                          */
/*-----------------------------------------------------------------*/
static void
genNot (iCode * ic)
{
  bool needpulla;

  D (emitcode (";     genNot", ""));

  /* assign asmOps to operand & result */
  aopOp (IC_LEFT (ic), ic, false);
  aopOp (IC_RESULT (ic), ic, true);
  needpulla = pushRegIfSurv (hc08_reg_a);
  asmopToBool (AOP (IC_LEFT (ic)), true);

  emitcode ("eor", one);
  regalloc_dry_run_cost += 2;
  storeRegToFullAop (hc08_reg_a, AOP (IC_RESULT (ic)), false);
  pullOrFreeReg (hc08_reg_a, needpulla);

  freeAsmop (IC_RESULT (ic), NULL, ic, true);
  freeAsmop (IC_LEFT (ic), NULL, ic, true);
}


/*-----------------------------------------------------------------*/
/* genCpl - generate code for complement                           */
/*-----------------------------------------------------------------*/
static void
genCpl (iCode * ic)
{
  int offset = 0;
  int size;
  reg_info *reg;
  bool needpullreg;

  D (emitcode (";     genCpl", ""));

  /* assign asmOps to operand & result */
  aopOp (IC_LEFT (ic), ic, false);
  aopOp (IC_RESULT (ic), ic, true);
  size = AOP_SIZE (IC_RESULT (ic));

  if(AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP_TYPE (IC_RESULT (ic)) == AOP_REG &&
    AOP (IC_RESULT (ic))->aopu.aop_reg[0] == AOP (IC_LEFT (ic))->aopu.aop_reg[0] &&
    (size < 2 || AOP (IC_RESULT (ic))->aopu.aop_reg[1] == AOP (IC_LEFT (ic))->aopu.aop_reg[1]))
    {
      while (size--)
        rmwWithReg ("com", AOP (IC_RESULT (ic))->aopu.aop_reg[offset++]);
      goto release;
    }

  if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG && AOP_TYPE (IC_LEFT (ic)) == AOP_REG)
    {
      while (size--)
        {
          if ((reg = AOP (IC_RESULT (ic))->aopu.aop_reg[offset]) != hc08_reg_h)
            {
              transferAopAop (AOP (IC_LEFT (ic)), offset, AOP (IC_RESULT (ic)), offset);
              rmwWithReg ("com", reg);
            }
          else
            {
              if ((reg = AOP (IC_LEFT (ic))->aopu.aop_reg[offset]) == hc08_reg_h)
                reg = hc08_reg_a->isDead ? hc08_reg_a : hc08_reg_x;
              needpullreg = pushRegIfSurv (reg);
              loadRegFromAop (reg, AOP (IC_LEFT (ic)), offset);
              rmwWithReg ("com", reg);
              storeRegToAop (reg, AOP (IC_RESULT (ic)), offset);
              if (needpullreg)
                pullReg (reg);
            }
          offset++;
        }
      goto release;
    }

  reg = (hc08_reg_a->isDead && !(AOP_TYPE (IC_RESULT (ic)) == AOP_REG && AOP (IC_RESULT (ic))->aopu.aop_reg[0] == hc08_reg_a) ? hc08_reg_a : hc08_reg_x);

  needpullreg = pushRegIfSurv (reg);
  while (size--)
    {
      bool needpullreg2 = (!size && AOP_TYPE (IC_RESULT (ic)) == AOP_REG && AOP (IC_RESULT (ic))->aopu.aop_reg[0] == reg || size && AOP_TYPE (IC_RESULT (ic)) == AOP_REG && AOP (IC_RESULT (ic))->aopu.aop_reg[1] == reg);
      if (needpullreg2)
        pushReg (reg, true);
      loadRegFromAop (reg, AOP (IC_LEFT (ic)), offset);
      rmwWithReg ("com", reg);
      hc08_useReg (reg);
      storeRegToAop (reg, AOP (IC_RESULT (ic)), offset);
      hc08_freeReg (reg);
      if (needpullreg2)
        pullReg (reg);
      offset++;
    }
  pullOrFreeReg (reg, needpullreg);

  /* release the aops */
release:
  freeAsmop (IC_RESULT (ic), NULL, ic, true);
  freeAsmop (IC_LEFT (ic), NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genUminusFloat - unary minus for floating points                */
/*-----------------------------------------------------------------*/
static void
genUminusFloat (operand * op, operand * result)
{
  int size, offset = 0;
  bool needpula;

  D (emitcode (";     genUminusFloat", ""));

  /* for this we just copy and then flip the bit */

  size = AOP_SIZE (op) - 1;

  while (size--)
    {
      transferAopAop (AOP (op), offset, AOP (result), offset);
      offset++;
    }

  needpula = pushRegIfSurv (hc08_reg_a);
  loadRegFromAop (hc08_reg_a, AOP (op), offset);
  emitcode ("eor", "#0x80");
  regalloc_dry_run_cost += 2;
  hc08_useReg (hc08_reg_a);
  storeRegToAop (hc08_reg_a, AOP (result), offset);
  pullOrFreeReg (hc08_reg_a, needpula);
}

/*-----------------------------------------------------------------*/
/* genUminus - unary minus code generation                         */
/*-----------------------------------------------------------------*/
static void
genUminus (iCode * ic)
{
  int offset, size;
  sym_link *optype;
  char *sub;
  bool needpula;
  asmop *result;

  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  D (emitcode (";     genUminus", ""));

  /* assign asmops */
  aopOp (IC_LEFT (ic), ic, false);
  aopOp (IC_RESULT (ic), ic, true);

  optype = operandType (IC_LEFT (ic));

  /* if float then do float stuff */
  if (IS_FLOAT (optype))
    {
      genUminusFloat (IC_LEFT (ic), IC_RESULT (ic));
      goto release;
    }

  /* otherwise subtract from zero */
  size = AOP_SIZE (IC_LEFT (ic));
  offset = 0;

  if (size == 1)
    {
      needpula = pushRegIfSurv (hc08_reg_a);
      loadRegFromAop (hc08_reg_a, AOP (IC_LEFT (ic)), 0);
      rmwWithReg ("neg", hc08_reg_a);
      if (maskedtopbyte)
        {
          emitcode ("and", "#0x%02x", topbytemask);
          regalloc_dry_run_cost += 2;
        }
      hc08_freeReg (hc08_reg_a);
      storeRegToFullAop (hc08_reg_a, AOP (IC_RESULT (ic)), SPEC_USIGN (operandType (IC_LEFT (ic))));
      pullOrFreeReg (hc08_reg_a, needpula);
      goto release;
    }

  /* If either left or result are in registers, handle this carefully to     */
  /* avoid prematurely overwriting register values. The 1 byte case was      */
  /* handled above and there aren't enough registers to handle 4 byte values */
  /* so this case only needs to deal with 2 byte values. */
  if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG || AOP_TYPE (IC_LEFT (ic)) == AOP_REG)
    {
      reg_info *result0 = NULL;
      reg_info *left0 = NULL;
      reg_info *left1 = NULL;
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG)
        {
          left0 = AOP (IC_LEFT (ic))->aopu.aop_reg[0];
          left1 = AOP (IC_LEFT (ic))->aopu.aop_reg[1];
        }
      if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG)
        {
          result0 = AOP (IC_RESULT (ic))->aopu.aop_reg[0];
        }
      needpula = pushRegIfSurv (hc08_reg_a);
      if (left1 == hc08_reg_a)
        pushReg (left1, true);

      if (left0 == hc08_reg_a)
        rmwWithReg ("neg", hc08_reg_a);
      else
        {
          loadRegFromConst (hc08_reg_a, 0);
          accopWithAop ("sub", AOP (IC_LEFT (ic)), 0);
        }
      if (result0 == hc08_reg_a || (result0 && result0 == left1))
        pushReg (hc08_reg_a, true);
      else
        storeRegToAop (hc08_reg_a, AOP (IC_RESULT (ic)), 0);
      loadRegFromConst (hc08_reg_a, 0);
      if (left1 == hc08_reg_a)
        {
          emitcode ("sbc","%d,s", (result0 == hc08_reg_a || (result0 && result0 == left1)) ? 2 : 1);
          regalloc_dry_run_cost += 3;
          hc08_dirtyReg (hc08_reg_a, false);
        }
      else
        {
          accopWithAop ("sbc", AOP (IC_LEFT (ic)), 1);
        }
      storeRegToAop (hc08_reg_a, AOP (IC_RESULT (ic)), 1);
      if (result0 == hc08_reg_a || (result0 && result0 == left1))
        pullReg (result0);
      if (left1 == hc08_reg_a)
        pullNull (1);
      pullOrFreeReg (hc08_reg_a, needpula);
      goto release;
    }

  result = AOP (IC_RESULT (ic));

  needpula = pushRegIfSurv (hc08_reg_a);
  sub = "sub";
  while (size--)
    {
      loadRegFromConst (hc08_reg_a, 0);
      accopWithAop (sub, AOP (IC_LEFT (ic)), offset);
      storeRegToAop (hc08_reg_a, result, offset++);
      sub = "sbc";
    }
  storeRegSignToUpperAop (hc08_reg_a, result, offset, SPEC_USIGN (operandType (IC_LEFT (ic))));
  pullOrFreeReg (hc08_reg_a, needpula);

release:
  /* release the aops */
  freeAsmop (IC_RESULT (ic), NULL, ic, true);
  freeAsmop (IC_LEFT (ic), NULL, ic, false);
}

/*-----------------------------------------------------------------*/
/* saveRegisters - will look for a call and save the registers     */
/*-----------------------------------------------------------------*/
static void
saveRegisters (iCode *lic)
{
  int i;
  iCode *ic;

  /* look for call */
  for (ic = lic; ic; ic = ic->next)
    if (ic->op == CALL || ic->op == PCALL)
      break;

  if (!ic)
    {
      fprintf (stderr, "found parameter push with no function call\n");
      return;
    }

  /* if the registers have been saved already or don't need to be then
     do nothing */
  if (ic->regsSaved)
    return;
  if (IS_SYMOP (IC_LEFT (ic)) &&
      (IFFUNC_CALLEESAVES (OP_SYMBOL (IC_LEFT (ic))->type) || IFFUNC_ISNAKED (OP_SYM_TYPE (IC_LEFT (ic)))))
    return;

  if (!regalloc_dry_run)
    ic->regsSaved = 1;
  for (i = A_IDX; i <= H_IDX; i++)
    {
      if (bitVectBitValue (ic->rSurv, i))
        pushReg (hc08_regWithIdx (i), false);
    }
}

/*-----------------------------------------------------------------*/
/* unsaveRegisters - pop the pushed registers                      */
/*-----------------------------------------------------------------*/
static void
unsaveRegisters (iCode *ic)
{
  int i;

  for (i = H_IDX; i >= A_IDX; i--)
    {
      if (bitVectBitValue (ic->rSurv, i))
        pullReg (hc08_regWithIdx (i));
    }

}


/*-----------------------------------------------------------------*/
/* pushSide -                                                      */
/*-----------------------------------------------------------------*/
static void
pushSide (operand *oper, int size, iCode *ic)
{
  int offset = 0;
  bool xIsFree = hc08_reg_x->isFree;

  hc08_useReg (hc08_reg_x);
  aopOp (oper, ic, false);

  if (AOP_TYPE (oper) == AOP_REG)
    {
      /* The operand is in registers; we can push them directly */
      while (size--)
        {
          pushReg (AOP (oper)->aopu.aop_reg[offset++], true);
        }
    }
  else if (hc08_reg_a->isFree)
    {
      /* A is free, so piecewise load operand into a and push A */
      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (oper), offset++);
          pushReg (hc08_reg_a, true);
        }
    }
  else
    {
      /* A is not free. Adjust stack, preserve A, copy operand */
      /* into position on stack (using A), and restore original A */
      adjustStack (-size);
      pushReg (hc08_reg_a, true);
      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (oper), offset++);
          emitcode ("sta", "%d,s", 2+size);
          regalloc_dry_run_cost += 3;
        }
      pullReg (hc08_reg_a);
    }

  freeAsmop (oper, NULL, ic, true);
  if (xIsFree)
    hc08_freeReg (hc08_reg_x);
}

/*-----------------------------------------------------------------*/
/* assignResultValue -                                             */
/*-----------------------------------------------------------------*/
static void
assignResultValue (operand * oper)
{
  int size = AOP_SIZE (oper);
  int offset = 0;
  bool delayed_x = false;
  while (size--)
    {
      if (!offset && AOP_TYPE (oper) == AOP_REG && AOP_SIZE (oper) > 1 && AOP (oper)->aopu.aop_reg[0]->rIdx == X_IDX)
        {
          pushReg (hc08_reg_a, true);
          delayed_x = true;
        }
      else
        transferAopAop (hc08_aop_pass[offset], 0, AOP (oper), offset);
      if (hc08_aop_pass[offset]->type == AOP_REG)
        hc08_freeReg (hc08_aop_pass[offset]->aopu.aop_reg[0]);
      offset++;
    }
  if (delayed_x)
    pullReg (hc08_reg_x);
}

/*-----------------------------------------------------------------*/
/* genIpush - generate code for pushing this gets a little complex */
/*-----------------------------------------------------------------*/
static void
genIpush (iCode * ic)
{
  int size, offset = 0;

  D (emitcode (";", "genIpush"));

  /* if this is not a parm push : ie. it is spill push
     and spill push is always done on the local stack */
  if (!ic->parmPush)
    {
      /* and the item is spilt then do nothing */
      if (OP_SYMBOL (IC_LEFT (ic))->isspilt)
        return;

      aopOp (IC_LEFT (ic), ic, false);
      size = AOP_SIZE (IC_LEFT (ic));
      /* push it on the stack */
      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (IC_LEFT (ic)), offset++);
          pushReg (hc08_reg_a, true);
        }
      return;
    }

  /* this is a parameter push: in this case we call
     the routine to find the call and save those
     registers that need to be saved */
  if (!regalloc_dry_run) /* Cost for saving registers is counted at CALL or PCALL */
    saveRegisters (ic);

  /* then do the push */
  aopOp (IC_LEFT (ic), ic, false);

  // pushSide(IC_LEFT(ic), AOP_SIZE(IC_LEFT(ic)));
  size = AOP_SIZE (IC_LEFT (ic));

//  l = aopGet (AOP (IC_LEFT (ic)), 0, false, true);
  if (AOP_TYPE (IC_LEFT (ic)) == AOP_IMMD || AOP_TYPE (IC_LEFT (ic)) == AOP_LIT ||IS_AOP_HX (AOP (IC_LEFT (ic))))
    {
      if ((size == 2) && hc08_reg_hx->isDead || IS_AOP_HX (AOP (IC_LEFT (ic))))
        {
          loadRegFromAop (hc08_reg_hx, AOP (IC_LEFT (ic)), 0);
          pushReg (hc08_reg_hx, true);
          goto release;
        }
    }

  if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG)
    {
      while (size--)
        pushReg (AOP (IC_LEFT (ic))->aopu.aop_reg[offset++], true);
      goto release;
    }

  while (size--)
    {
//      printf("loading %d\n", offset);
      loadRegFromAop (hc08_reg_a, AOP (IC_LEFT (ic)), offset++);
//      printf("pushing \n");
      pushReg (hc08_reg_a, true);
    }

release:
  freeAsmop (IC_LEFT (ic), NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genPointerPush - generate code for pushing                      */
/*-----------------------------------------------------------------*/
static void
genPointerPush (iCode *ic)
{
  operand *left = IC_LEFT (ic);

  D (emitcode (";     genPointerPush", ""));

  aopOp (left, ic, false);

  wassertl (IC_RIGHT (ic), "IPUSH_VALUE_AT_ADDRESS without right operand");
  wassertl (IS_OP_LITERAL (IC_RIGHT (ic)), "IPUSH_VALUE_AT_ADDRESS with non-literal right operand");
  wassertl (!operandLitValue (IC_RIGHT(ic)), "IPUSH_VALUE_AT_ADDRESS with non-zero right operand");

  loadRegFromAop (hc08_reg_hx, left->aop, 0);
  /* so hx now contains the address */

  int size = getSize (operandType (IC_LEFT (ic))->next);
  while (size--)
    {
      loadRegIndexed (hc08_reg_a, size, 0);
      pushReg (hc08_reg_a, true);
    }

  freeAsmop (IC_LEFT (ic), NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genIpop - recover the registers: can happen only for spilling   */
/*-----------------------------------------------------------------*/
static void
genIpop (iCode * ic)
{
  int size, offset;

  D (emitcode (";", "genIpop"));

  /* if the temp was not pushed then */
  if (OP_SYMBOL (IC_LEFT (ic))->isspilt)
    return;

  aopOp (IC_LEFT (ic), ic, false);
  size = AOP_SIZE (IC_LEFT (ic));
  offset = size - 1;
  while (size--)
    {
      pullReg (hc08_reg_a);
      storeRegToAop (hc08_reg_a, AOP (IC_LEFT (ic)), offset--);
    }

  freeAsmop (IC_LEFT (ic), NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genSend - gen code for SEND                                     */
/*-----------------------------------------------------------------*/
static void
genSend (set *sendSet)
{
  iCode *send1;
  iCode *send2;

  D (emitcode (";", "genSend"));

  /* case 1: single parameter in A
   * case 2: single parameter in XA
   * case 3: first parameter in A, second parameter in X
   */
  send1 = setFirstItem (sendSet);
  send2 = setNextItem (sendSet);

  if (!send2)
    {
      int size;
      /* case 1 or 2, this is fairly easy */
      aopOp (IC_LEFT (send1), send1, false);
      size = AOP_SIZE (IC_LEFT (send1));
      wassert (size <= 2);
      if (size == 1)
        {
          loadRegFromAop (send1->argreg == 2 ? hc08_reg_x : hc08_reg_a, AOP (IC_LEFT (send1)), 0);
        }
      else if (AOP (IC_LEFT (send1))->type == AOP_REG)
        loadRegFromAop (hc08_reg_xa, AOP (IC_LEFT (send1)), 0);
      else if (isOperandVolatile (IC_LEFT (send1), false))
        {
          /* use msb to lsb order for volatile operands */
          loadRegFromAop (hc08_reg_x, AOP (IC_LEFT (send1)), 1);
          loadRegFromAop (hc08_reg_a, AOP (IC_LEFT (send1)), 0);
        }
      else
        {
          /* otherwise perfer to load x last (lsb to msb order) */
          loadRegFromAop (hc08_reg_a, AOP (IC_LEFT (send1)), 0);
          loadRegFromAop (hc08_reg_x, AOP (IC_LEFT (send1)), 1);
        }
      freeAsmop (IC_LEFT (send1), NULL, send1, true);
    }
  else
    {
      /* case 3 */
      /* make sure send1 is the first argument and swap with send2 if not */
      if (send1->argreg > send2->argreg)
        {
          iCode * sic = send1;
          send1 = send2;
          send2 = sic;
        }
      aopOp (IC_LEFT (send1), send1, false);
      aopOp (IC_LEFT (send2), send2, false);
      if (IS_AOP_X (AOP (IC_LEFT (send1))) && IS_AOP_A (AOP (IC_LEFT (send2))))
        {
          /* If the parameters' register assignment is exactly backwards */
          /* from what is needed, then swap the registers. */
          pushReg (hc08_reg_a, false);
          transferRegReg (hc08_reg_x, hc08_reg_a, false);
          pullReg (hc08_reg_x);
        }
      else if (IS_AOP_A (AOP (IC_LEFT (send2))))
        {
          loadRegFromAop (hc08_reg_x, AOP (IC_LEFT (send2)), 0);
          loadRegFromAop (hc08_reg_a, AOP (IC_LEFT (send1)), 0);
        }
      else
        {
          loadRegFromAop (hc08_reg_a, AOP (IC_LEFT (send1)), 0);
          loadRegFromAop (hc08_reg_x, AOP (IC_LEFT (send2)), 0);
        }
      freeAsmop (IC_LEFT (send2), NULL, send2, true);
      freeAsmop (IC_LEFT (send1), NULL, send1, true);
    }
}

/*-----------------------------------------------------------------*/
/* pushbigreturn - emit code to push hidden pointer for struct return */
/*-----------------------------------------------------------------*/
static void
pushbigreturn (operand *result)
{
  wassert (result);

  symbol *sym = OP_SYMBOL (result);
  wassert (sym);

  if (sym->onStack)
    {
      /* if it has an offset then we need to compute it */
      int offset = _G.stackOfs + _G.stackPushes + sym->stack;
      hc08_useReg (hc08_reg_hx);
      emitcode ("tsx", "");
      hc08_dirtyReg (hc08_reg_hx, false);
      regalloc_dry_run_cost++;
      while (offset > 127)
        {
          emitcode ("aix", "#127");
          regalloc_dry_run_cost += 2;
          offset -= 127;
        }
      while (offset < -128)
        {
          emitcode ("aix", "#-128");
          regalloc_dry_run_cost += 2;
          offset += 128;
        }
      if (offset)
        {
          emitcode ("aix", "#%d", offset);
          regalloc_dry_run_cost += 2;
        }
    }
  else
    loadRegFromImm (hc08_reg_hx, sym->rname);

  pushReg (hc08_reg_hx, true);
}

/*-----------------------------------------------------------------*/
/* genCall - generates a call statement                            */
/*-----------------------------------------------------------------*/
static void
genCall (iCode * ic)
{
  sym_link *dtype;
  sym_link *etype;
//  bool restoreBank = false;
//  bool swapBanks = false;

  D (emitcode (";", "genCall"));

  /* if caller saves & we have not saved then */
  if (!ic->regsSaved)
    saveRegisters (ic);

  dtype = operandType (IC_LEFT (ic));
  etype = getSpec (dtype);
  const bool bigreturn = IS_STRUCT (dtype->next);

  /* if send set is not empty then assign */
  if (_G.sendSet && !regalloc_dry_run)
    {
      if (IFFUNC_ISREENT (dtype))       /* need to reverse the send set */
        {
          genSend (reverseSet (_G.sendSet));
        }
      else
        {
          genSend (_G.sendSet);
        }
      _G.sendSet = NULL;
    }

  // Pass pointer for storing return value
  if (bigreturn)
    pushbigreturn (IC_RESULT (ic));

  /* make the call */
  if (IS_LITERAL (etype))
    {
      emitcode ("jsr", "0x%04X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
      regalloc_dry_run_cost += 3;
    }
  else
    {
      bool jump = (!ic->parmBytes && IFFUNC_ISNORETURN (OP_SYMBOL (IC_LEFT (ic))->type));

      emitcode (jump ? "jmp" : "jsr", "%s", (OP_SYMBOL (IC_LEFT (ic))->rname[0] ?
                              OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name));
      regalloc_dry_run_cost += 3;
    }

  hc08_dirtyReg (hc08_reg_a, false);
  hc08_dirtyReg (hc08_reg_hx, false);

  // Adjust stack pointer for the hidden pointer parameter.
  if (bigreturn)
    adjustStack (2);

  /* if we need assign a result value */
  else if ((IS_ITEMP (IC_RESULT (ic)) &&
       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {
      hc08_useReg (hc08_reg_a);
      if (operandSize (IC_RESULT (ic)) > 1)
        hc08_useReg (hc08_reg_x);
      _G.accInUse++;
      aopOp (IC_RESULT (ic), ic, false);
      _G.accInUse--;

      assignResultValue (IC_RESULT (ic));

      freeAsmop (IC_RESULT (ic), NULL, ic, true);
    }

  /* adjust the stack for parameters if required */
  if (ic->parmBytes)
    {
      pullNull (ic->parmBytes);
    }

  /* if we had saved some registers then unsave them */
  if (ic->regsSaved && !IFFUNC_CALLEESAVES (dtype))
    unsaveRegisters (ic);
}

/*-----------------------------------------------------------------*/
/* genPcall - generates a call by pointer statement                */
/*-----------------------------------------------------------------*/
static void
genPcall (iCode * ic)
{
  sym_link *dtype;
  sym_link *etype;
  symbol *rlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  iCode * sendic;

  D (emitcode (";", "genPcall"));

  dtype = operandType (IC_LEFT (ic))->next;
  etype = getSpec (dtype);
  /* if caller saves & we have not saved then */
  if (!ic->regsSaved)
    saveRegisters (ic);

  /* Go through the send set and mark any registers used by iTemps as */
  /* in use so we don't clobber them while setting up the return address */
  for (sendic = setFirstItem (_G.sendSet); sendic; sendic = setNextItem (_G.sendSet))
    {
      updateiTempRegisterUse (IC_LEFT (sendic));
    }
  
  if (!IS_LITERAL (etype))
    {
      /* push the return address on to the stack */
      emitBranch ("bsr", tlbl);
      emitBranch ("bra", rlbl);
      if (!regalloc_dry_run)
        emitLabel (tlbl);
      _G.stackPushes += 2;          /* account for the bsr return address now on stack */
      updateCFA ();

      /* now push the function address */
      pushSide (IC_LEFT (ic), FARPTRSIZE, ic);
    }

  /* if send set is not empty then assign */
  if (_G.sendSet && !regalloc_dry_run)
    {
      genSend (reverseSet (_G.sendSet));
      _G.sendSet = NULL;
    }

  /* make the call */
  if (!IS_LITERAL (etype))
    {
      emitcode ("rts", "");
      regalloc_dry_run_cost++;

      if (!regalloc_dry_run)
        emitLabel (rlbl);
      _G.stackPushes -= 4;          /* account for rts here & in called function */
      updateCFA ();
    }
  else
    {
      emitcode ("jsr", "0x%04X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
      regalloc_dry_run_cost += 3;
    }

  hc08_dirtyReg (hc08_reg_a, false);
  hc08_dirtyReg (hc08_reg_hx, false);

  /* if we need assign a result value */
  if ((IS_ITEMP (IC_RESULT (ic)) &&
       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir)) || IS_TRUE_SYMOP (IC_RESULT (ic)))
    {
      hc08_useReg (hc08_reg_a);
      if (operandSize (IC_RESULT (ic)) > 1)
        hc08_useReg (hc08_reg_x);
      _G.accInUse++;
      aopOp (IC_RESULT (ic), ic, false);
      _G.accInUse--;

      assignResultValue (IC_RESULT (ic));

      freeAsmop (IC_RESULT (ic), NULL, ic, true);
    }

  /* adjust the stack for parameters if required */
  if (ic->parmBytes)
    {
      pullNull (ic->parmBytes);
    }

  /* if we had saved some registers then unsave them */
  if (ic->regsSaved && !IFFUNC_CALLEESAVES (dtype))
    unsaveRegisters (ic);
}

/*-----------------------------------------------------------------*/
/* resultRemat - result  is rematerializable                       */
/*-----------------------------------------------------------------*/
static int
resultRemat (iCode * ic)
{
  if (SKIP_IC (ic) || ic->op == IFX)
    return 0;

  if (IC_RESULT (ic) && IS_ITEMP (IC_RESULT (ic)))
    {
      symbol *sym = OP_SYMBOL (IC_RESULT (ic));
      if (sym->remat && !POINTER_SET (ic))
        return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* inExcludeList - return 1 if the string is in exclude Reg list   */
/*-----------------------------------------------------------------*/
static int
regsCmp (void *p1, void *p2)
{
  return (STRCASECMP ((char *) p1, (char *) (p2)) == 0);
}

static bool
inExcludeList (char *s)
{
  const char *p = setFirstItem (options.excludeRegsSet);

  if (p == NULL || STRCASECMP (p, "none") == 0)
    return false;


  return isinSetWith (options.excludeRegsSet, s, regsCmp);
}

/*-----------------------------------------------------------------*/
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void
genFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  sym_link *ftype;
  iCode *ric = (ic->next && ic->next->op == RECEIVE) ? ic->next : NULL;
  int stackAdjust = sym->stack;
  int accIsFree = sym->recvSize == 0;

  _G.nRegsSaved = 0;
  _G.stackPushes = 0;
  /* create the function header */
  emitcode (";", "-----------------------------------------");
  emitcode (";", " function %s", sym->name);
  emitcode (";", "-----------------------------------------");
  emitcode (";", hc08_assignment_optimal ? "Register assignment is optimal." : "Register assignment might be sub-optimal.");
  emitcode (";", "Stack space usage: %d bytes.", sym->stack);

  emitcode ("", "%s:", sym->rname);
  genLine.lineCurr->isLabel = 1;
  ftype = operandType (IC_LEFT (ic));

  _G.stackOfs = 0;
  _G.stackPushes = 0;
  if (options.debug && !regalloc_dry_run)
    debugFile->writeFrameAddress (NULL, hc08_reg_sp, 0);

  if (IFFUNC_ISNAKED (ftype))
    {
      emitcode (";", "naked function: no prologue.");
      return;
    }

  /* if this is an interrupt service routine then
     save h  */
  if (IFFUNC_ISISR (sym->type))
    {
      if (!inExcludeList ("h"))
        pushReg (hc08_reg_h, false);
    }

  /* For some cases it is worthwhile to perform a RECEIVE iCode */
  /* before setting up the stack frame completely. */
  while (ric && ric->next && ric->next->op == RECEIVE)
    ric = ric->next;
  while (ric && IC_RESULT (ric))
    {
      symbol *rsym = OP_SYMBOL (IC_RESULT (ric));
      int rsymSize = rsym ? getSize (rsym->type) : 0;

      if (rsym->isitmp)
        {
          if (rsym && rsym->regType == REG_CND)
            rsym = NULL;
          if (rsym && (/*rsym->accuse ||*/ rsym->ruonly))
            rsym = NULL;
          if (rsym && (rsym->isspilt || rsym->nRegs == 0) && rsym->usl.spillLoc)
            rsym = rsym->usl.spillLoc;
        }

      /* If the RECEIVE operand immediately spills to the first entry on the  */
      /* stack, we can push it directly rather than use an sp relative store. */
      if (rsym && rsym->onStack && rsym->stack == -_G.stackPushes - rsymSize)
        {
          int ofs;

          genLine.lineElement.ic = ric;
          D (emitcode (";     genReceive", ""));
          for (ofs = 0; ofs < rsymSize; ofs++)
            {
              reg_info *reg = hc08_aop_pass[ofs + (ric->argreg - 1)]->aopu.aop_reg[0];
              pushReg (reg, true);
              if (reg->rIdx == A_IDX)
                accIsFree = 1;
              stackAdjust--;
            }
          genLine.lineElement.ic = ic;
          ric->generated = 1;
        }
      ric = (ric->prev && ric->prev->op == RECEIVE) ? ric->prev : NULL;
    }

  /* adjust the stack for the function */
  if (stackAdjust)
    {
      adjustStack (-stackAdjust);
    }
  _G.stackOfs = sym->stack;
  _G.stackPushes = 0;

  /* if critical function then turn interrupts off */
  if (IFFUNC_ISCRITICAL (ftype))
    {
      if (!accIsFree)
        {
          /* Function was passed parameters, so make sure A is preserved */
          pushReg (hc08_reg_a, false);
          pushReg (hc08_reg_a, false);
          emitcode ("tpa", "");
          emitcode ("sta", "2,s");
          emitcode ("sei", "");
          regalloc_dry_run_cost += 5;
          pullReg (hc08_reg_a);
        }
      else
        {
          /* No passed parameters, so A can be freely modified */
          emitcode ("tpa", "");
          regalloc_dry_run_cost++;
          pushReg (hc08_reg_a, true);
          emitcode ("sei", "");
          regalloc_dry_run_cost++;
        }
    }
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));

  if (IFFUNC_ISNAKED (sym->type))
    {
      emitcode (";", "naked function: no epilogue.");
      if (options.debug && currFunc && !regalloc_dry_run)
        debugFile->writeEndFunction (currFunc, ic, 0);
      return;
    }

  if (IFFUNC_ISCRITICAL (sym->type))
    {
      if (!IS_VOID (sym->type->next))
        {
          /* Function has return value, so make sure A is preserved */
          pushReg (hc08_reg_a, false);
          emitcode ("lda", "2,s");
          emitcode ("tap", "");
          regalloc_dry_run_cost += 4;
          pullReg (hc08_reg_a);
          pullNull (1);
        }
      else
        {
          /* Function returns void, so A can be freely modified */
          pullReg (hc08_reg_a);
          emitcode ("tap", "");
          regalloc_dry_run_cost++;
        }
    }

  if (IFFUNC_ISREENT (sym->type) || options.stackAuto)
    {
    }

  if (sym->stack)
    {
      _G.stackPushes += sym->stack;
      adjustStack (sym->stack);
    }


  if ((IFFUNC_ISREENT (sym->type) || options.stackAuto))
    {
    }

  if (IFFUNC_ISISR (sym->type))
    {

      if (!inExcludeList ("h"))
        pullReg (hc08_reg_h);


      /* if debug then send end of function */
      if (options.debug && currFunc && !regalloc_dry_run)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      emitcode ("rti", "");
      regalloc_dry_run_cost++;
    }
  else
    {
      if (IFFUNC_CALLEESAVES (sym->type))
        {
          int i;

          /* if any registers used */
          if (sym->regsUsed)
            {
              /* save the registers used */
              for (i = sym->regsUsed->size; i >= 0; i--)
                {
                  if (bitVectBitValue (sym->regsUsed, i) || (hc08_ptrRegReq && (i == HX_IDX || i == HX_IDX)))
                    emitcode ("pop", "%s", hc08_regWithIdx (i)->name); /* Todo: Cost. Can't find this instruction in manual! */
                }
            }
        }

      /* if debug then send end of function */
      if (options.debug && currFunc && !regalloc_dry_run)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      emitcode ("rts", "");
      regalloc_dry_run_cost++;
    }
}

/*-----------------------------------------------------------------*/
/* genRet - generate code for return statement                     */
/*-----------------------------------------------------------------*/
static void
genRet (iCode * ic)
{
  int size, offset = 0;
//  int pushed = 0;
  bool delayed_x = false;

  D (emitcode (";     genRet", ""));

  /* if we have no return value then
     just generate the "ret" */
  if (!IC_LEFT (ic))
    goto jumpret;

  /* we have something to return then
     move the return value into place */
  aopOp (IC_LEFT (ic), ic, false);
  size = AOP_SIZE (IC_LEFT (ic));
  const bool bigreturn = IS_STRUCT (operandType (IC_LEFT (ic)));

  if (bigreturn) // todo: implement!
    {
      if (!regalloc_dry_run)
        werror ( E_FUNC_AGGR);
      goto jumpret;
    }

  if (AOP_TYPE (IC_LEFT (ic)) == AOP_LIT)
    {
      /* If returning a literal, we can load the bytes of the return value */
      /* in any order. By loading A and X first, any other bytes that match */
      /* can use the shorter sta and stx instructions. */
      offset = 0;
      while (size--)
        {
          transferAopAop (AOP (IC_LEFT (ic)), offset, hc08_aop_pass[offset], 0);
          offset++;
        }
    }
  else
    {
      /* Take care when swapping a and x */
      if (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && size > 1 && AOP (IC_LEFT (ic))->aopu.aop_reg[0]->rIdx == X_IDX)
        {
          delayed_x = true;
          pushReg (hc08_reg_x, true);
        }

      offset = size - 1;
      while (size--)
        {
          if (!(delayed_x && !offset))
            transferAopAop (AOP (IC_LEFT (ic)), offset, hc08_aop_pass[offset], 0);
          offset--;
        }

      if (delayed_x)
        pullReg (hc08_reg_a);
    }

  freeAsmop (IC_LEFT (ic), NULL, ic, true);

jumpret:
  /* generate a jump to the return label
     if the next is not the return statement */
  if (!(ic->next && ic->next->op == LABEL && IC_LABEL (ic->next) == returnLabel))
    {
      emitcode ("jmp", "%05d$", labelKey2num (returnLabel->key));
      regalloc_dry_run_cost += 3;
    }
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void
genLabel (iCode * ic)
{
  int i;
  reg_info *reg;

  /* For the high level labels we cannot depend on any */
  /* register's contents. Amnesia time.                */
  for (i = A_IDX; i <= XA_IDX; i++)
    {
      reg = hc08_regWithIdx (i);
      if (reg)
        {
          reg->aop = NULL;
          reg->isLitConst = 0;
        }
    }

  /* special case never generate */
  if (IC_LABEL (ic) == entryLabel)
    return;

  if (options.debug && !regalloc_dry_run)
    debugFile->writeLabel (IC_LABEL (ic), ic);

  emitLabel (IC_LABEL (ic));

}

/*-----------------------------------------------------------------*/
/* genGoto - generates a jmp                                      */
/*-----------------------------------------------------------------*/
static void
genGoto (iCode * ic)
{
  emitcode ("jmp", "%05d$", labelKey2num (IC_LABEL (ic)->key));
  regalloc_dry_run_cost += 3;
}


/*-----------------------------------------------------------------*/
/* genPlusIncr :- does addition with increment if possible         */
/*-----------------------------------------------------------------*/
static bool
genPlusIncr (iCode * ic)
{
  int icount;
  operand *left;
  operand *result;
  bool needpulx;
  bool needpulh;
  bool needpula;
  unsigned int size = getDataSize (IC_RESULT (ic));
  unsigned int offset;
  symbol *tlbl = NULL;

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  /* will try to generate an increment */
  /* if the right side is not a literal
     we cannot */
  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    return false;

  icount = (unsigned int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);

  DD (emitcode ("", "; IS_AOP_HX = %d", IS_AOP_HX (AOP (left))));

  if ((IS_AOP_HX (AOP (left)) || IS_AOP_HX (AOP (result)) ||
    ((AOP_TYPE (left) == AOP_DIR || IS_S08 && AOP_TYPE (left) == AOP_EXT) && (AOP_TYPE (result) == AOP_DIR || IS_S08 && AOP_TYPE (result) == AOP_EXT))) &&
    (icount >= -128) && (icount <= 127) && (size == 2))
    {
      needpulx = pushRegIfSurv (hc08_reg_x);
      needpulh = pushRegIfSurv (hc08_reg_h);
      loadRegFromAop (hc08_reg_hx, AOP (left), 0);
      emitcode ("aix", "#%d", icount);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_hx, false);
      storeRegToAop (hc08_reg_hx, AOP (result), 0);
      pullOrFreeReg (hc08_reg_h, needpulh);
      pullOrFreeReg (hc08_reg_x, needpulx);
      return true;
    }
  if (size == 1 && (IS_AOP_X (AOP (result)) && (!IS_AOP_A (AOP (left)) || hc08_reg_h->isDead) || IS_AOP_X (AOP (left)) && !IS_AOP_A (AOP (result)) && hc08_reg_x->isDead && hc08_reg_h->isDead))
    {
      icount = (int)(signed char) icount; /* truncate to a signed single byte value */
      needpulh = pushRegIfSurv (hc08_reg_h);
      loadRegFromAop (hc08_reg_x, AOP (left), 0);
      emitcode ("aix", "#%d", icount);
      regalloc_dry_run_cost += 2;
      storeRegToAop (hc08_reg_x, AOP (result), 0);
      pullOrFreeReg (hc08_reg_h, needpulh);
      return true;
    }

  DD (emitcode ("", "; icount = %d, sameRegs=%d", icount, sameRegs (AOP (left), AOP (result))));

  if ((icount > 255) || (icount < 0))
    return false;

  if (!sameRegs (AOP (left), AOP (result)))
    return false;

  D (emitcode (";     genPlusIncr", ""));

  aopOpExtToIdx (AOP (result), AOP (left), NULL);

  if (size > 1)
    tlbl = regalloc_dry_run ? 0 : newiTempLabel (NULL);

  if (icount == 1)
    {
      needpula = false;
      rmwWithAop ("inc", AOP (result), 0);
      if (1 < size)
        emitBranch ("bne", tlbl);
    }
  else
    {
      if (!IS_AOP_A (AOP (result)) && !IS_AOP_XA (AOP (result)))
        needpula = pushRegIfUsed (hc08_reg_a);
      else
        needpula = false;
      loadRegFromAop (hc08_reg_a, AOP (result), 0);
      accopWithAop ("add", AOP (IC_RIGHT (ic)), 0);
      hc08_useReg (hc08_reg_a);
      storeRegToAop (hc08_reg_a, AOP (result), 0);
      hc08_freeReg (hc08_reg_a);
      if (1 < size)
        emitBranch ("bcc", tlbl);
    }
  for (offset = 1; offset < size; offset++)
    {
      rmwWithAop ("inc", AOP (result), offset);
      if ((offset + 1) < size)
        emitBranch ("bne", tlbl);
    }

  if (size > 1 && !regalloc_dry_run)
    emitLabel (tlbl);

  pullOrFreeReg (hc08_reg_a, needpula);

  return true;
}



/*-----------------------------------------------------------------*/
/* genPlus - generates code for addition                           */
/*-----------------------------------------------------------------*/
static void
genPlus (iCode *ic)
{
  int size, offset = 0;
  char *add;
  asmop *leftOp, *rightOp;
  bool needpulla;
  bool earlystore = false;
  bool delayedstore = false;
  bool mayskip = true;
  bool skip = false;
  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  /* special cases :- */

  D (emitcode (";     genPlus", ""));

  aopOp (IC_LEFT (ic), ic, false);
  aopOp (IC_RIGHT (ic), ic, false);
  aopOp (IC_RESULT (ic), ic, true);

  /* we want registers on the left and literals on the right */
  if ((AOP_TYPE (IC_LEFT (ic)) == AOP_LIT) || (AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && !IS_AOP_WITH_A (AOP (IC_LEFT (ic)))))
    {
      operand *t = IC_RIGHT (ic);
      IC_RIGHT (ic) = IC_LEFT (ic);
      IC_LEFT (ic) = t;
    }

  /* if I can do an increment instead
     of add then GOOD for ME */
  if (!maskedtopbyte && genPlusIncr (ic))
    goto release;

  DD (emitcode ("", ";  left size = %d", getDataSize (IC_LEFT (ic))));
  DD (emitcode ("", ";  right size = %d", getDataSize (IC_RIGHT (ic))));
  DD (emitcode ("", ";  result size = %d", getDataSize (IC_RESULT (ic))));

  aopOpExtToIdx (AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic)), AOP (IC_RIGHT (ic)));

  size = getDataSize (IC_RESULT (ic));

  leftOp = AOP (IC_LEFT (ic));
  rightOp = AOP (IC_RIGHT (ic));
  add = "add";

  offset = 0;
  needpulla = pushRegIfSurv (hc08_reg_a);

  if(size > 1 && IS_AOP_AX (AOP (IC_LEFT (ic))))
    {
      earlystore = true;
      pushReg (hc08_reg_a, true);
    }

  while (size--)
    {
      if (earlystore && offset == 1)
        pullReg (hc08_reg_a);
      else
        loadRegFromAop (hc08_reg_a, leftOp, offset);
      if (!mayskip || !aopIsLitVal (rightOp, offset, 1, 0x00))
        {
          accopWithAop (add, rightOp, offset);
          if (!size && maskedtopbyte)
            {
              emitcode ("and", "#0x%02x", topbytemask);
              regalloc_dry_run_cost += 2;
            }
          mayskip = false;
          skip = false;
        }
      else
        skip = true;
      if (size && AOP_TYPE (IC_RESULT (ic)) == AOP_REG && AOP (IC_RESULT (ic))->aopu.aop_reg[offset]->rIdx == A_IDX)
        {
          pushReg (hc08_reg_a, true);
          delayedstore = true;
        }
      else
        storeRegToAop (hc08_reg_a, AOP (IC_RESULT (ic)), offset);
      offset++;
      hc08_freeReg (hc08_reg_a);
      if (!skip)
        add = "adc";              /* further adds must propagate carry */
    }
 if (delayedstore)
   pullReg (hc08_reg_a);
 pullOrFreeReg (hc08_reg_a, needpulla);

 wassert (!earlystore || !delayedstore);

release:
  freeAsmop (IC_RESULT (ic), NULL, ic, true);
  freeAsmop (IC_RIGHT (ic), NULL, ic, true);
  freeAsmop (IC_LEFT (ic), NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genMinusDec :- does subtraction with decrement if possible      */
/*-----------------------------------------------------------------*/
static bool
genMinusDec (iCode * ic)
{
  int icount;
  operand *left;
  operand *result;
  bool needpulx;
  bool needpulh;
  unsigned int size = getDataSize (IC_RESULT (ic));
//  int offset;
//  symbol *tlbl;

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  /* will try to generate an increment */
  /* if the right side is not a literal
     we cannot */
  if (AOP_TYPE (IC_RIGHT (ic)) != AOP_LIT)
    return false;

  icount = (unsigned int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);
  if ((IS_AOP_HX (AOP (left)) || IS_AOP_HX (AOP (result)) ||
    ((AOP_TYPE (left) == AOP_DIR || IS_S08 && AOP_TYPE (left) == AOP_EXT) && (AOP_TYPE (result) == AOP_DIR || IS_S08 && AOP_TYPE (result) == AOP_EXT))) &&
    (icount >= -127) && (icount <= 128) && (size == 2))
    {
      needpulx = pushRegIfSurv (hc08_reg_x);
      needpulh = pushRegIfSurv (hc08_reg_h);

      loadRegFromAop (hc08_reg_hx, AOP (left), 0);
      emitcode ("aix", "#%d", -(int) icount);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_hx, false);
      storeRegToAop (hc08_reg_hx, AOP (result), 0);
      pullOrFreeReg (hc08_reg_h, needpulh);
      pullOrFreeReg (hc08_reg_x, needpulx);
      return true;
    }

  if ((icount > 1) || (icount < 0))
    return false;

  if (!sameRegs (AOP (left), AOP (result)))
    return false;

  if (size != 1)
    return false;

  D (emitcode (";     genMinusDec", ""));

  aopOpExtToIdx (AOP (result), AOP (left), NULL);

  rmwWithAop ("dec", AOP (result), 0);

  return true;
}

/*-----------------------------------------------------------------*/
/* addSign - complete with sign                                    */
/*-----------------------------------------------------------------*/
static void
addSign (operand * result, int offset, int sign)
{
  int size = (getDataSize (result) - offset);
  if (size > 0)
    {
      if (sign)
        {
          emitcode ("rola", "");
          emitcode ("clra", "");
          emitcode ("sbc", zero);
          hc08_dirtyReg (hc08_reg_a, false);
          regalloc_dry_run_cost += 4;
          while (size--)
            storeRegToAop (hc08_reg_a, AOP (result), offset++);
        }
      else
        while (size--)
          storeConstToAop (0, AOP (result), offset++);
    }
}


/*-----------------------------------------------------------------*/
/* genMinus - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
static void
genMinus (iCode * ic)
{
  char *sub;
  int size, offset = 0;
  bool needpulla;
  bool earlystore = false;
  bool delayedstore = false;

  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  asmop *leftOp, *rightOp;

  D (emitcode (";     genMinus", ""));

  aopOp (IC_LEFT (ic), ic, false);
  aopOp (IC_RIGHT (ic), ic, false);
  aopOp (IC_RESULT (ic), ic, true);

  /* special cases :- */
  /* if I can do an decrement instead
     of subtract then GOOD for ME */
  if (!maskedtopbyte && genMinusDec (ic))
    goto release;

  aopOpExtToIdx (AOP (IC_RESULT (ic)), AOP (IC_LEFT (ic)), AOP (IC_RIGHT (ic)));

  size = getDataSize (IC_RESULT (ic));


  leftOp = AOP (IC_LEFT (ic));
  rightOp = AOP (IC_RIGHT (ic));
  sub = "sub";
  offset = 0;

  needpulla = pushRegIfSurv (hc08_reg_a);

  if (IS_AOP_A (rightOp))
    {
      loadRegFromAop (hc08_reg_a, rightOp, offset);
      accopWithAop (sub, leftOp, offset);
      accopWithMisc ("nega", "");
      if (maskedtopbyte)
        {
          emitcode ("and", "#0x%02x", topbytemask);
          regalloc_dry_run_cost += 2;
        }
      storeRegToAop (hc08_reg_a, AOP (IC_RESULT (ic)), offset);
      pullOrFreeReg (hc08_reg_a, needpulla);
      goto release;
    }

  if (size > 1 && (IS_AOP_AX (AOP (IC_LEFT (ic))) || IS_AOP_AX (AOP (IC_RIGHT (ic)))))
    {
      earlystore = true;
      pushReg (hc08_reg_a, true);
    }

  while (size--)
    {
      if (earlystore &&
        (AOP_TYPE (IC_LEFT (ic)) == AOP_REG && AOP (IC_LEFT (ic))->aopu.aop_reg[offset]->rIdx == A_IDX ||
        AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[offset]->rIdx == A_IDX))
        pullReg (hc08_reg_a);
      if (AOP_TYPE (IC_RIGHT (ic)) == AOP_REG && AOP (IC_RIGHT (ic))->aopu.aop_reg[offset]->rIdx == A_IDX)
        {
          pushReg (hc08_reg_a, true);
          loadRegFromAop (hc08_reg_a, leftOp, offset);
          emitcode (sub, "1, s");
          hc08_dirtyReg (hc08_reg_a, false);
          regalloc_dry_run_cost += 3;
          pullNull (1);
        }
      else
        {
          loadRegFromAop (hc08_reg_a, leftOp, offset);
          accopWithAop (sub, rightOp, offset);
        }
      if (!size && maskedtopbyte)
        {
          emitcode ("and", "#0x%02x", topbytemask);
          regalloc_dry_run_cost += 2;
        }
        
      if (size && AOP_TYPE (IC_RESULT (ic)) == AOP_REG && AOP (IC_RESULT (ic))->aopu.aop_reg[offset]->rIdx == A_IDX)
        {
          pushReg (hc08_reg_a, true);
          delayedstore = true;
        }
      else
        storeRegToAop (hc08_reg_a, AOP (IC_RESULT (ic)), offset);
      offset++;
      sub = "sbc";
    }
  if(delayedstore)
    pullReg (hc08_reg_a);
  pullOrFreeReg (hc08_reg_a, needpulla);

  wassert (!earlystore || !delayedstore);

release:
  freeAsmop (IC_LEFT (ic), NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (IC_RIGHT (ic), NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (IC_RESULT (ic), NULL, ic, true);
}



/*-----------------------------------------------------------------*/
/* genMultOneByte : 8*8=8/16 bit multiplication                    */
/*-----------------------------------------------------------------*/
static void
genMultOneByte (operand * left, operand * right, operand * result)
{
  /* sym_link *opetype = operandType (result); */
  symbol *tlbl1, *tlbl2, *tlbl3, *tlbl4;
  int size = AOP_SIZE (result);
  bool negLiteral = false;
  bool lUnsigned, rUnsigned;
  bool needpulla, needpullx;

  D (emitcode (";     genMultOneByte", ""));

  if (size < 1 || size > 2)
    {
      // this should never happen
      fprintf (stderr, "size!=1||2 (%d) in %s at line:%d \n", AOP_SIZE (result), __FILE__, lineno);
      exit (1);
    }

  /* (if two literals: the value is computed before) */
  /* if one literal, literal on the right */
  if (AOP_TYPE (left) == AOP_LIT)
    {
      operand *t = right;
      right = left;
      left = t;
    }
  /* if an operand is in A, make sure it is on the left */
  if (IS_AOP_A (AOP (right)))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  needpulla = pushRegIfSurv (hc08_reg_a);
  needpullx = pushRegIfSurv (hc08_reg_x);

  lUnsigned = SPEC_USIGN (getSpec (operandType (left)));
  rUnsigned = SPEC_USIGN (getSpec (operandType (right)));

  /* lUnsigned  rUnsigned  negLiteral  negate     case */
  /* false      false      false       odd        3    */
  /* false      false      true        even       3    */
  /* false      true       false       odd        3    */
  /* false      true       true        impossible      */
  /* true       false      false       odd        3    */
  /* true       false      true        always     2    */
  /* true       true       false       never      1    */
  /* true       true       true        impossible      */

  /* case 1 */
  if (size == 1 || (lUnsigned && rUnsigned))
    {
      // just an unsigned 8*8=8/16 multiply
      //DD(emitcode (";","unsigned"));

      loadRegFromAop (hc08_reg_a, AOP (left), 0);
      loadRegFromAop (hc08_reg_x, AOP (right), 0);
      emitcode ("mul", "");
      regalloc_dry_run_cost++;
      hc08_dirtyReg (hc08_reg_xa, false);
      storeRegToFullAop (hc08_reg_xa, AOP (result), true);
      hc08_freeReg (hc08_reg_xa);
      pullOrFreeReg (hc08_reg_x, needpullx);
      pullOrFreeReg (hc08_reg_a, needpulla);

      return;
    }

  // we have to do a signed multiply

  /* case 2 */
  /* left unsigned, right signed literal -- literal determines sign handling */
  if (AOP_TYPE (right) == AOP_LIT && lUnsigned && !rUnsigned)
    {
      signed char val = (signed char) ulFromVal (AOP (right)->aopu.aop_lit);

      loadRegFromAop (hc08_reg_a, AOP (left), 0);
      emitcode ("ldx", "#0x%02x", val < 0 ? -val : val);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_x, false);

      emitcode ("mul", "");
      regalloc_dry_run_cost++;
      hc08_dirtyReg (hc08_reg_xa, false);

      if (val < 0)
        {
          rmwWithReg ("neg", hc08_reg_a);
          tlbl4 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emitBranch ("bcc", tlbl4);
          rmwWithReg ("inc", hc08_reg_x);
          if (!regalloc_dry_run)
            emitLabel (tlbl4);
          rmwWithReg ("neg", hc08_reg_x);
        }

      storeRegToFullAop (hc08_reg_xa, AOP (result), true);
      hc08_freeReg (hc08_reg_xa);
      pullOrFreeReg (hc08_reg_x, needpullx);
      pullOrFreeReg (hc08_reg_a, needpulla);
      return;
    }


  /* case 3 */
  adjustStack (-1);
  emitcode ("clr", "1,s");
  regalloc_dry_run_cost += 3;

  loadRegFromAop (hc08_reg_a, AOP (left), 0);
  if (!lUnsigned)
    {
      tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
      emitcode ("tsta", "");
      regalloc_dry_run_cost++;
      emitBranch ("bpl", tlbl1);
      emitcode ("inc", "1,s");
      regalloc_dry_run_cost += 3;
      rmwWithReg ("neg", hc08_reg_a);
      regalloc_dry_run_cost++;
      if (!regalloc_dry_run)
        emitLabel (tlbl1);
    }

  if (AOP_TYPE (right) == AOP_LIT && !rUnsigned)
    {
      signed char val = (signed char) ulFromVal (AOP (right)->aopu.aop_lit);
      /* AND literal negative */
      if (val < 0)
        {
          emitcode ("ldx", "#0x%02x", -val);
          regalloc_dry_run_cost += 2;
          negLiteral = true;
        }
      else
        {
          emitcode ("ldx", "#0x%02x", val);
          regalloc_dry_run_cost += 2;
        }
      hc08_dirtyReg (hc08_reg_x, false);
      hc08_useReg (hc08_reg_x);
    }
  else
    {
      loadRegFromAop (hc08_reg_x, AOP (right), 0);
      if (!rUnsigned)
        {
          tlbl2 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emitcode ("tstx", "");
          regalloc_dry_run_cost++;
          emitBranch ("bpl", tlbl2);
          emitcode ("inc", "1,s");
          regalloc_dry_run_cost += 3;
          rmwWithReg ("neg", hc08_reg_x);
          regalloc_dry_run_cost++;
          if (!regalloc_dry_run)
            emitLabel (tlbl2);
        }
    }

  emitcode ("mul", "");
  regalloc_dry_run_cost++;
  hc08_dirtyReg (hc08_reg_xa, false);

  tlbl3 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  emitcode ("dec", "1,s");
  regalloc_dry_run_cost += 3;
  if (!lUnsigned && !rUnsigned && negLiteral)
    emitBranch ("beq", tlbl3);
  else
    emitBranch ("bne", tlbl3);

  rmwWithReg ("neg", hc08_reg_a);
  tlbl4 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  emitBranch ("bcc", tlbl4);
  rmwWithReg ("inc", hc08_reg_x);
  if (!regalloc_dry_run)
    emitLabel (tlbl4);
  rmwWithReg ("neg", hc08_reg_x);

  if (!regalloc_dry_run)
    emitLabel (tlbl3);
  adjustStack (1);
  storeRegToFullAop (hc08_reg_xa, AOP (result), true);
  
  pullOrFreeReg (hc08_reg_x, needpullx);
  pullOrFreeReg (hc08_reg_a, needpulla);
}

/*-----------------------------------------------------------------*/
/* genMult - generates code for multiplication                     */
/*-----------------------------------------------------------------*/
static void
genMult (iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  D (emitcode (";     genMult", ""));

  /* assign the amsops */
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  /* special cases first */
  /* if both are of size == 1 */
//  if (getSize(operandType(left)) == 1 &&
//      getSize(operandType(right)) == 1)
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1)
    {
      genMultOneByte (left, right, result);
      goto release;
    }

  /* should have been converted to function call */
  fprintf (stderr, "left: %d right: %d\n", getSize (OP_SYMBOL (left)->type), getSize (OP_SYMBOL (right)->type));
  fprintf (stderr, "left: %d right: %d\n", AOP_SIZE (left), AOP_SIZE (right));
  assert (0);

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genDivOneByte : 8 bit division                                  */
/*-----------------------------------------------------------------*/
static void
genDivOneByte (operand * left, operand * right, operand * result)
{
  symbol *tlbl1, *tlbl2, *tlbl3;
  int size;
  bool lUnsigned, rUnsigned;
  bool runtimeSign, compiletimeSign;
  bool needpulla, needpullh;
  bool needpullx = false;
  bool preload_a = false;

  lUnsigned = SPEC_USIGN (getSpec (operandType (left)));
  rUnsigned = SPEC_USIGN (getSpec (operandType (right)));

  D (emitcode (";     genDivOneByte", ""));

  needpulla = pushRegIfSurv (hc08_reg_a);
  needpullh = pushRegIfSurv (hc08_reg_h);
  needpullx = pushRegIfSurv (hc08_reg_x);

  /* If both left and right are in registers and backwards from what we need, */
  /* the swap the operands and the registers. */
  if (IS_AOP_A (AOP (right)) && IS_AOP_X (AOP (left)))
    {
      operand * t;
      t = left;
      left = right;
      right = t;
      pushReg (hc08_reg_a, false);
      transferRegReg (hc08_reg_x, hc08_reg_a, false);
      pullReg (hc08_reg_x);
    }

  size = AOP_SIZE (result);
  /* signed or unsigned */
  if (lUnsigned && rUnsigned)
    {
      /* unsigned is easy */
      if (IS_AOP_A (AOP (right)))
        {
          loadRegFromAop (hc08_reg_x, AOP (right), 0);
          loadRegFromAop (hc08_reg_a, AOP (left), 0);
        }
      else
        {
          loadRegFromAop (hc08_reg_a, AOP (left), 0);
          loadRegFromAop (hc08_reg_x, AOP (right), 0);
        }
      loadRegFromConst (hc08_reg_h, 0);
      emitcode ("div", "");
      regalloc_dry_run_cost++;
      hc08_dirtyReg (hc08_reg_a, false);
      hc08_dirtyReg (hc08_reg_h, false);
      storeRegToFullAop (hc08_reg_a, AOP (result), false);
      pullOrFreeReg (hc08_reg_x, needpullx);
      pullOrFreeReg (hc08_reg_h, needpullh);
      pullOrFreeReg (hc08_reg_a, needpulla);
      return;
    }

  /* signed is a little bit more difficult */

  /* now sign adjust for both left & right */

  /* let's see what's needed: */
  /* apply negative sign during runtime */
  runtimeSign = false;
  /* negative sign from literals */
  compiletimeSign = false;

  if (!lUnsigned)
    {
      if (AOP_TYPE (left) == AOP_LIT)
        {
          /* signed literal */
          signed char val = (char) ulFromVal (AOP (left)->aopu.aop_lit);
          if (val < 0)
            compiletimeSign = true;
        }
      else
        /* signed but not literal */
        runtimeSign = true;
    }

  if (!rUnsigned)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* signed literal */
          signed char val = (char) ulFromVal (AOP (right)->aopu.aop_lit);
          if (val < 0)
            compiletimeSign ^= true;
        }
      else
        /* signed but not literal */
        runtimeSign = true;
    }

  /* initialize the runtime sign */
  if (runtimeSign)
    {
      if (compiletimeSign)
        pushConst (1);    /* set sign flag */
      else
        pushConst (0);    /* reset sign flag */
    }

  if (IS_AOP_X (AOP (left)))
    {
      loadRegFromAop (hc08_reg_a, AOP (left), 0);
      preload_a = true;
    }

  /* save the signs of the operands */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (right)->aopu.aop_lit);

      if (!rUnsigned && val < 0)
        {
          emitcode ("ldx", "#0x%02x", -val);
          regalloc_dry_run_cost += 2;
        }
      else
        {
          emitcode ("ldx", "#0x%02x", (unsigned char) val);
          regalloc_dry_run_cost += 2;
        }
      hc08_dirtyReg (hc08_reg_x, false);
      hc08_useReg (hc08_reg_x);
    }
  else                          /* ! literal */
    {
      loadRegFromAop (hc08_reg_x, AOP (right), 0);
      if (!rUnsigned)
        {
          tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emitcode ("tstx", "");
          regalloc_dry_run_cost++;
          emitBranch ("bpl", tlbl1);
          emitcode ("inc", "1,s");
          regalloc_dry_run_cost += 3;
          rmwWithReg ("neg", hc08_reg_x);
          if (!regalloc_dry_run)
            emitLabel (tlbl1);
        }
    }

  if (AOP_TYPE (left) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (left)->aopu.aop_lit);

      if (!lUnsigned && val < 0)
        {
          emitcode ("lda", "#0x%02x", -val);
          regalloc_dry_run_cost += 2;
        }
      else
        {
          emitcode ("lda", "#0x%02x", (unsigned char) val);
          regalloc_dry_run_cost += 2;
        }
      hc08_dirtyReg (hc08_reg_a, false);
      hc08_useReg (hc08_reg_a);
    }
  else                          /* ! literal */
    {
      if (!preload_a)
        loadRegFromAop (hc08_reg_a, AOP (left), 0);
      if (!lUnsigned)
        {
          tlbl2 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emitcode ("tsta", "");
          regalloc_dry_run_cost++;
          emitBranch ("bpl", tlbl2);
          emitcode ("inc", "1,s");
          regalloc_dry_run_cost += 3;
          rmwWithReg ("neg", hc08_reg_a);
          if (!regalloc_dry_run)
            emitLabel (tlbl2);
        }
    }

  loadRegFromConst (hc08_reg_h, 0);
  emitcode ("div", "");
  regalloc_dry_run_cost++;
  hc08_dirtyReg (hc08_reg_x, false);
  hc08_dirtyReg (hc08_reg_a, false);
  hc08_dirtyReg (hc08_reg_h, false);

  if (runtimeSign || compiletimeSign)
    {
      if (runtimeSign)
        {
          tlbl3 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          pullReg (hc08_reg_x);
          rmwWithReg ("lsr", hc08_reg_x);
          if (size > 1)
            loadRegFromConst (hc08_reg_x, 0);
          emitBranch ("bcc", tlbl3);
          rmwWithReg ("neg", hc08_reg_a);
          if (size > 1)
            rmwWithReg ("dec", hc08_reg_x);
          if (!regalloc_dry_run)
            emitLabel (tlbl3);
          /* signed result now in A or XA */
          if (size == 1)
            storeRegToAop (hc08_reg_a, AOP (result), 0);
          else
            storeRegToAop (hc08_reg_xa, AOP (result), 0);
        }
      else /* must be compiletimeSign */
        {
          hc08_freeReg (hc08_reg_x); /* in case we need a free reg for the 0xff */
          rmwWithReg ("neg", hc08_reg_a);
          storeRegToAop (hc08_reg_a, AOP (result), 0);
          if (size > 1)
            storeConstToAop (0xff, AOP (result), 1);
        }
    }
  else
    {
      storeRegToFullAop (hc08_reg_a, AOP (result), false);
    }

  pullOrFreeReg (hc08_reg_x, needpullx);
  pullOrFreeReg (hc08_reg_h, needpullh);
  pullOrFreeReg (hc08_reg_a, needpulla);
}

/*-----------------------------------------------------------------*/
/* genDiv - generates code for division                            */
/*-----------------------------------------------------------------*/
static void
genDiv (iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  D (emitcode (";     genDiv", ""));

  /* assign the amsops */
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  /* special cases first */
  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1 && AOP_SIZE (result) <= 2)
    {
      genDivOneByte (left, right, result);
      goto release;
    }

  /* should have been converted to function call */
  assert (0);
release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genModOneByte : 8 bit modulus                                   */
/*-----------------------------------------------------------------*/
static void
genModOneByte (operand * left, operand * right, operand * result)
{
  symbol *tlbl1, *tlbl2, *tlbl3;
  int size;
  bool lUnsigned, rUnsigned;
  bool runtimeSign, compiletimeSign;
  bool needpulla, needpullh;
  bool needpullx = false;
  bool preload_a = false;

  lUnsigned = SPEC_USIGN (getSpec (operandType (left)));
  rUnsigned = SPEC_USIGN (getSpec (operandType (right)));

  D (emitcode (";     genModOneByte", ""));

  size = AOP_SIZE (result);

  needpulla = pushRegIfSurv (hc08_reg_a);
  needpullh = pushRegIfSurv (hc08_reg_h);
  needpullx = pushRegIfSurv (hc08_reg_x);

  /* If both left and right are in registers and backwards from what we need, */
  /* the swap the operands and the registers. */
  if (IS_AOP_A (AOP (right)) && IS_AOP_X (AOP (left)))
    {
      operand * t;
      t = left;
      left = right;
      right = t;
      pushReg (hc08_reg_a, false);
      transferRegReg (hc08_reg_x, hc08_reg_a, false);
      pullReg (hc08_reg_x);
    }

  if (lUnsigned && rUnsigned)
    {
      /* unsigned is easy */
      if (IS_AOP_A (AOP (right)))
        {
          loadRegFromAop (hc08_reg_x, AOP (right), 0);
          loadRegFromAop (hc08_reg_a, AOP (left), 0);
        }
      else
        {
          loadRegFromAop (hc08_reg_a, AOP (left), 0);
          loadRegFromAop (hc08_reg_x, AOP (right), 0);
        }
      loadRegFromConst (hc08_reg_h, 0);
      emitcode ("div", "");
      regalloc_dry_run_cost++;
      hc08_freeReg (hc08_reg_x);
      hc08_dirtyReg (hc08_reg_a, true);
      hc08_dirtyReg (hc08_reg_h, false);
      storeRegToFullAop (hc08_reg_h, AOP (result), false);
      pullOrFreeReg (hc08_reg_x, needpullx);
      pullOrFreeReg (hc08_reg_h, needpullh);
      pullOrFreeReg (hc08_reg_a, needpulla);
      return;
    }

  /* signed is a little bit more difficult */
  if (IS_AOP_X (AOP (left)))
    {
      loadRegFromAop (hc08_reg_a, AOP (left), 0);
      preload_a = true;
    }

  if (AOP_TYPE (right) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (right)->aopu.aop_lit);

      if (!rUnsigned && val < 0)
        {
          emitcode ("ldx", "#0x%02x", -val);
          regalloc_dry_run_cost += 2;
        }
      else
        {
          emitcode ("ldx", "#0x%02x", (unsigned char) val);
          regalloc_dry_run_cost += 2;
        }
      hc08_dirtyReg (hc08_reg_x, false);
      hc08_useReg (hc08_reg_x);
    }
  else                          /* ! literal */
    {
      loadRegFromAop (hc08_reg_x, AOP (right), 0);
      if (!rUnsigned)
        {
          tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emitcode ("tstx", "");
          regalloc_dry_run_cost++;
          emitBranch ("bpl", tlbl1);
          rmwWithReg ("neg", hc08_reg_x);
          if (!regalloc_dry_run)
            emitLabel (tlbl1);
        }
    }

  /* let's see what's needed: */
  /* apply negative sign during runtime */
  runtimeSign = false;
  /* negative sign from literals */
  compiletimeSign = false;

  /* sign adjust left side */
  if (AOP_TYPE (left) == AOP_LIT)
    {
      signed char val = (char) ulFromVal (AOP (left)->aopu.aop_lit);

      if (!lUnsigned && val < 0)
        {
          compiletimeSign = true;       /* set sign flag */
          emitcode ("lda", "#0x%02x", -val);
          regalloc_dry_run_cost += 2;
        }
      else
        {
          emitcode ("lda", "#0x%02x", (unsigned char) val);
          regalloc_dry_run_cost += 2;
        }
      hc08_dirtyReg (hc08_reg_a, false);
      hc08_useReg (hc08_reg_a);
    }
  else                          /* ! literal */
    {
      if (lUnsigned)
        {
          if (!preload_a)
            loadRegFromAop (hc08_reg_a, AOP (left), 0);
        }
      else
        {
          runtimeSign = true;
          pushConst (0);

          if (!preload_a)
            loadRegFromAop (hc08_reg_a, AOP (left), 0);
          tlbl2 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          emitcode ("tsta", "");
          regalloc_dry_run_cost++;
          emitBranch ("bpl", tlbl2);
          emitcode ("inc", "1,s");
          regalloc_dry_run_cost += 3;
          rmwWithReg ("neg", hc08_reg_a);
          if (!regalloc_dry_run)
            emitLabel (tlbl2);
        }
    }

  loadRegFromConst (hc08_reg_h, 0);
  emitcode ("div", "");
  regalloc_dry_run_cost++;
  hc08_freeReg (hc08_reg_x);
  hc08_dirtyReg (hc08_reg_a, true);
  hc08_dirtyReg (hc08_reg_h, false);

  if (runtimeSign || compiletimeSign)
    {
      transferRegReg (hc08_reg_h, hc08_reg_a, true);
      if (runtimeSign)
        {
          tlbl3 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          pullReg (hc08_reg_x);
          rmwWithReg ("lsr", hc08_reg_x);
          if (size > 1)
            loadRegFromConst (hc08_reg_x, 0);
          emitBranch ("bcc", tlbl3);
          rmwWithReg ("neg", hc08_reg_a);
          if (size > 1)
            rmwWithReg ("dec", hc08_reg_x);
          if (!regalloc_dry_run)
            emitLabel (tlbl3);
          /* signed result now in A or XA */
          if (size == 1)
            storeRegToAop (hc08_reg_a, AOP (result), 0);
          else
            storeRegToAop (hc08_reg_xa, AOP (result), 0);
        }
      else /* must be compiletimeSign */
        {
          hc08_freeReg (hc08_reg_x); /* in case we need a free reg for the 0xff */
          rmwWithReg ("neg", hc08_reg_a);
          storeRegToAop (hc08_reg_a, AOP (result), 0);
          if (size > 1)
            storeConstToAop (0xff, AOP (result), 1);
        }
    }
  else
    {
      storeRegToFullAop (hc08_reg_h, AOP (result), false);
    }

  pullOrFreeReg (hc08_reg_x, needpullx);
  pullOrFreeReg (hc08_reg_h, needpullh);
  pullOrFreeReg (hc08_reg_a, needpulla);
}

/*-----------------------------------------------------------------*/
/* genMod - generates code for division                            */
/*-----------------------------------------------------------------*/
static void
genMod (iCode * ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  D (emitcode (";     genMod", ""));

  /* assign the amsops */
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  /* special cases first */
  /* if both are of size == 1 */
  if (AOP_SIZE (left) == 1 && AOP_SIZE (right) == 1 && AOP_SIZE (result) <= 2)
    {
      genModOneByte (left, right, result);
      goto release;
    }

  /* should have been converted to function call */
  assert (0);

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genIfxJump :- will create a jump depending on the ifx           */
/*-----------------------------------------------------------------*/
static void
genIfxJump (iCode * ic, char *jval)
{
  symbol *jlbl;
  symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  char *inst;

  D (emitcode (";     genIfxJump", ""));

  /* if true label then we jump if condition
     supplied is true */
  if (IC_TRUE (ic))
    {
      jlbl = IC_TRUE (ic);
      if (!strcmp (jval, "a"))
        inst = "beq";
      else if (!strcmp (jval, "c"))
        inst = "bcc";
      else if (!strcmp (jval, "n"))
        inst = "bpl";
      else
        inst = "bge";
    }
  else
    {
      /* false label is present */
      jlbl = IC_FALSE (ic);
      if (!strcmp (jval, "a"))
        inst = "bne";
      else if (!strcmp (jval, "c"))
        inst = "bcs";
      else if (!strcmp (jval, "n"))
        inst = "bmi";
      else
        inst = "blt";
    }
  emitBranch (inst, tlbl);
  emitBranch ("jmp", jlbl);
  if (!regalloc_dry_run)
    emitLabel (tlbl);

  /* mark the icode as generated */
  ic->generated = 1;
}


/*-----------------------------------------------------------------*/
/* exchangedCmp : returns the opcode need if the two operands are  */
/*                exchanged in a comparison                        */
/*-----------------------------------------------------------------*/
static int
exchangedCmp (int opcode)
{
  switch (opcode)
    {
    case '<':
      return '>';
    case '>':
      return '<';
    case LE_OP:
      return GE_OP;
    case GE_OP:
      return LE_OP;
    case NE_OP:
      return NE_OP;
    case EQ_OP:
      return EQ_OP;
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "opcode not a comparison");
    }
  return EQ_OP;                 /* shouldn't happen, but need to return something */
}

/*------------------------------------------------------------------*/
/* negatedCmp : returns the equivalent opcode for when a comparison */
/*              if not true                                         */
/*------------------------------------------------------------------*/
static int
negatedCmp (int opcode)
{
  switch (opcode)
    {
    case '<':
      return GE_OP;
    case '>':
      return LE_OP;
    case LE_OP:
      return '>';
    case GE_OP:
      return '<';
    case NE_OP:
      return EQ_OP;
    case EQ_OP:
      return NE_OP;
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "opcode not a comparison");
    }
  return EQ_OP;                 /* shouldn't happen, but need to return something */
}

/*------------------------------------------------------------------*/
/* nameCmp : helper function for human readable debug output        */
/*------------------------------------------------------------------*/
static char *
nameCmp (int opcode)
{
  switch (opcode)
    {
    case '<':
      return "<";
    case '>':
      return ">";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case NE_OP:
      return "!=";
    case EQ_OP:
      return "==";
    default:
      return "invalid";
    }
}

/*------------------------------------------------------------------*/
/* branchInstCmp : returns the conditional branch instruction that  */
/*                 will branch if the comparison is true            */
/*------------------------------------------------------------------*/
static char *
branchInstCmp (int opcode, int sign)
{
  switch (opcode)
    {
    case '<':
      if (sign)
        return "blt";
      else
        return "bcs";           /* same as blo */
    case '>':
      if (sign)
        return "bgt";
      else
        return "bhi";
    case LE_OP:
      if (sign)
        return "ble";
      else
        return "bls";
    case GE_OP:
      if (sign)
        return "bge";
      else
        return "bcc";           /* same as bhs */
    case NE_OP:
      return "bne";
    case EQ_OP:
      return "beq";
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "opcode not a comparison");
    }
  return "brn";
}


/*------------------------------------------------------------------*/
/* genCmp :- greater or less than (and maybe with equal) comparison */
/*------------------------------------------------------------------*/
static void
genCmp (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  sym_link *letype, *retype;
  int sign, opcode;
  int size, offset = 0;
  unsigned long long lit = 0ull;
  char *sub;
  symbol *jlbl = NULL;
  bool needpulla = false;

  opcode = ic->op;

  D (emitcode (";     genCmp", "(%s)", nameCmp (opcode)));

  result = IC_RESULT (ic);
  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);

  sign = 0;
  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    {
      letype = getSpec (operandType (left));
      retype = getSpec (operandType (right));
      sign = !(SPEC_USIGN (letype) | SPEC_USIGN (retype));
    }

  /* assign the amsops */
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  /* need register operand on left, prefer literal operand on right */
  if ((AOP_TYPE (right) == AOP_REG) || AOP_TYPE (left) == AOP_LIT)
    {
      operand *temp = left;
      left = right;
      right = temp;
      opcode = exchangedCmp (opcode);
    }

  if (ifx)
    {
      if (IC_TRUE (ifx))
        {
          jlbl = IC_TRUE (ifx);
          opcode = negatedCmp (opcode);
        }
      else
        {
          /* false label is present */
          jlbl = IC_FALSE (ifx);
        }
    }

  size = max (AOP_SIZE (left), AOP_SIZE (right));

  if (size == 1 && IS_AOP_X (AOP (left)))
    {
      accopWithAop ("cpx", AOP (right), offset);
    }
  else if (size == 1 && IS_AOP_A (AOP (left)))
    {
      accopWithAop ("cmp", AOP (right), offset);
    }
  else if ((size == 2)
      && ((AOP_TYPE (left) == AOP_DIR || IS_AOP_HX (AOP (left))) && (AOP_SIZE (left) == 2))
      && ((AOP_TYPE (right) == AOP_LIT) || ((AOP_TYPE (right) == AOP_DIR || IS_S08 && AOP_TYPE (right) == AOP_EXT) && (AOP_SIZE (right) == 2))) && (hc08_reg_h->isDead && hc08_reg_x->isDead || IS_AOP_HX (AOP (left))))
    {
      loadRegFromAop (hc08_reg_hx, AOP (left), 0);
      emitcode ("cphx", "%s", aopAdrStr (AOP (right), 0, true));
      regalloc_dry_run_cost += (AOP_TYPE (right) == AOP_DIR ? 2 : 3);
      hc08_freeReg (hc08_reg_hx);
    }
  else
    {
      offset = 0;
      if (size == 1)
        sub = "cmp";
      else
        {
          sub = "sub";

          /* These conditions depend on the Z flag bit, but Z is */
          /* only valid for the last byte of the comparison, not */
          /* the whole value. So exchange the operands to get a  */
          /* comparison that doesn't depend on Z. (This is safe  */
          /* to do here since ralloc won't assign multi-byte     */
          /* operands to registers for comparisons)              */
          if ((opcode == '>') || (opcode == LE_OP))
            {
              operand *temp = left;
              left = right;
              right = temp;
              opcode = exchangedCmp (opcode);
            }

          if ((AOP_TYPE (right) == AOP_LIT) && !isOperandVolatile (left, false))
            {
              lit = ullFromVal (AOP (right)->aopu.aop_lit);
              while ((size > 1) && (((lit >> (8 * offset)) & 0xff) == 0))
                {
                  offset++;
                  size--;
                }
            }
        }
      needpulla = pushRegIfSurv (hc08_reg_a);
      while (size--)
        {
          if (AOP_TYPE (right) == AOP_REG && AOP(right)->aopu.aop_reg[offset]->rIdx == A_IDX)
            {
              pushReg (hc08_reg_a, true);
              loadRegFromAop (hc08_reg_a, AOP (left), offset);
              emitcode (sub, "1, s");
              regalloc_dry_run_cost += 3;
              pullReg (hc08_reg_a);
            }
          else
            {
              loadRegFromAop (hc08_reg_a, AOP (left), offset);
              accopWithAop (sub, AOP (right), offset);
            }
          hc08_freeReg (hc08_reg_a);
          offset++;
          sub = "sbc";
        }
    }
  freeAsmop (right, NULL, ic, false);
  freeAsmop (left, NULL, ic, false);

  if (ifx)
    {
      symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
      char *inst;

      pullOrFreeReg (hc08_reg_a, needpulla);

      freeAsmop (result, NULL, ic, true);

      inst = branchInstCmp (opcode, sign);
      emitBranch (inst, tlbl);
      emitBranch ("jmp", jlbl);
      if (!regalloc_dry_run)
        emitLabel (tlbl);

      /* mark the icode as generated */
      ifx->generated = 1;
    }
  else
    {
      symbol *tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
      symbol *tlbl2 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

      if (!needpulla)
        needpulla = pushRegIfSurv (hc08_reg_a);

      emitBranch (branchInstCmp (opcode, sign), tlbl1);
      loadRegFromConst (hc08_reg_a, 0);
      emitBranch ("bra", tlbl2);
      if (!regalloc_dry_run)
        emitLabel (tlbl1);
      hc08_dirtyReg (hc08_reg_a, false);
      loadRegFromConst (hc08_reg_a, 1);
      if (!regalloc_dry_run)
        emitLabel (tlbl2);
      hc08_dirtyReg (hc08_reg_a, false);
      storeRegToFullAop (hc08_reg_a, AOP (result), false);
      pullOrFreeReg (hc08_reg_a, needpulla);
      freeAsmop (result, NULL, ic, true);
    }
}

/*-----------------------------------------------------------------*/
/* genCmpEQorNE - equal or not equal comparison                    */
/*-----------------------------------------------------------------*/
static void
genCmpEQorNE (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int opcode;
  int size, offset = 0;
  symbol *jlbl = NULL;
  symbol *tlbl_NE = NULL;
  symbol *tlbl_EQ = NULL;
  bool needpulla = false;

  opcode = ic->op;

  D (emitcode (";     genCmpEQorNE", "(%s)", nameCmp (opcode)));

  result = IC_RESULT (ic);
  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);

  /* assign the amsops */
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  /* need register operand on left, prefer literal operand on right */
  if ((AOP_TYPE (right) == AOP_REG) || AOP_TYPE (left) == AOP_LIT)
    {
      operand *temp = left;
      left = right;
      right = temp;
      opcode = exchangedCmp (opcode);
    }

  if (ifx)
    {
      if (IC_TRUE (ifx))
        {
          jlbl = IC_TRUE (ifx);
          opcode = negatedCmp (opcode);
        }
      else
        {
          /* false label is present */
          jlbl = IC_FALSE (ifx);
        }
    }

  size = max (AOP_SIZE (left), AOP_SIZE (right));

  if ((size == 2)
      && ((AOP_TYPE (left) == AOP_DIR || IS_AOP_HX (AOP (left))) && (AOP_SIZE (left) == 2))
      && ((AOP_TYPE (right) == AOP_LIT) || ((AOP_TYPE (right) == AOP_DIR || IS_S08 && AOP_TYPE (right) == AOP_EXT) && (AOP_SIZE (right) == 2))) && hc08_reg_h->isDead && hc08_reg_x->isDead)
    {
      loadRegFromAop (hc08_reg_hx, AOP (left), 0);
      emitcode ("cphx", "%s", aopAdrStr (AOP (right), 0, true));
      regalloc_dry_run_cost += (AOP_TYPE (right) == AOP_DIR ? 2 : 3);
      hc08_freeReg (hc08_reg_hx);
    }
  else
    {
      offset = 0;
      while (size--)
        {
          if (AOP_TYPE (left) == AOP_REG && AOP (left)->aopu.aop_reg[offset]->rIdx == X_IDX)
            {
              if (aopIsLitVal (right->aop, offset, 1, 0x00))
                {
                  emitcode ("tstx", "");
                  regalloc_dry_run_cost++;
                }
              else
                accopWithAop ("cpx", AOP (right), offset);
            }
          else
            {
              if (!(AOP_TYPE (left) == AOP_REG && AOP (left)->aopu.aop_reg[offset]->rIdx == A_IDX))
                {
                  needpulla = pushRegIfSurv (hc08_reg_a);
                  loadRegFromAop (hc08_reg_a, AOP (left), offset);
                }
              if (aopIsLitVal (right->aop, offset, 1, 0x00))
                {
                  emitcode ("tsta", "");
                  regalloc_dry_run_cost++;
                }
              else
                accopWithAop ("cmp", AOP (right), offset);
              if (!(AOP_TYPE (left) == AOP_REG && AOP (left)->aopu.aop_reg[offset]->rIdx == A_IDX))
                pullOrFreeReg (hc08_reg_a, needpulla);
              needpulla = false;
            }
          if (size)
            {
              if (!needpulla && !ifx)
                needpulla = pushRegIfSurv (hc08_reg_a);
              if (!tlbl_NE && !regalloc_dry_run)
                tlbl_NE = newiTempLabel (NULL);
              emitBranch ("bne", tlbl_NE);
              pullOrFreeReg (hc08_reg_a, needpulla);
              needpulla = false;
            }
          offset++;
          }
    }
  freeAsmop (right, NULL, ic, false);
  freeAsmop (left, NULL, ic, false);

  if (ifx)
    {
      freeAsmop (result, NULL, ic, true);

      if (opcode == EQ_OP)
        {
          if (!tlbl_EQ && !regalloc_dry_run)
            tlbl_EQ = newiTempLabel (NULL);
          emitBranch ("beq", tlbl_EQ);
          if (tlbl_NE)
            emitLabel (tlbl_NE);
          emitBranch ("jmp", jlbl);
          if (!regalloc_dry_run)
            emitLabel (tlbl_EQ);
        }
      else
        {
          if (!tlbl_NE && !regalloc_dry_run)
            tlbl_NE = newiTempLabel (NULL);
          emitBranch ("bne", tlbl_NE);
          emitBranch ("jmp", jlbl);
          if (!regalloc_dry_run)
            emitLabel (tlbl_NE);
        }

      /* mark the icode as generated */
      ifx->generated = 1;
    }
  else
    {
      symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

      if (!needpulla)
        needpulla = pushRegIfSurv (hc08_reg_a);
      if (opcode == EQ_OP)
        {
          if (!tlbl_EQ && !regalloc_dry_run)
            tlbl_EQ = newiTempLabel (NULL);
          emitBranch ("beq", tlbl_EQ);
          if (tlbl_NE)
            emitLabel (tlbl_NE);
          hc08_dirtyReg (hc08_reg_a, false);
          loadRegFromConst (hc08_reg_a, 0);
          emitBranch ("bra", tlbl);
          if (!regalloc_dry_run)
            emitLabel (tlbl_EQ);
          hc08_dirtyReg (hc08_reg_a, false);
          loadRegFromConst (hc08_reg_a, 1);
        }
      else
        {
          if (!tlbl_NE && !regalloc_dry_run)
            tlbl_NE = newiTempLabel (NULL);
          emitBranch ("bne", tlbl_NE);
          loadRegFromConst (hc08_reg_a, 0);
          emitBranch ("bra", tlbl);
          if (!regalloc_dry_run)
            emitLabel (tlbl_NE);
          hc08_dirtyReg (hc08_reg_a, false);
          loadRegFromConst (hc08_reg_a, 1);
        }

      if (!regalloc_dry_run)
        emitLabel (tlbl);
      hc08_dirtyReg (hc08_reg_a, false);
      storeRegToFullAop (hc08_reg_a, AOP (result), false);
      pullOrFreeReg (hc08_reg_a, needpulla);
      freeAsmop (result, NULL, ic, true);
    }
}

/*-----------------------------------------------------------------*/
/* hasInchc08 - operand is incremented before any other use        */
/*-----------------------------------------------------------------*/
iCode *
hasInchc08 (operand *op, const iCode *ic, int osize)
{
  sym_link *type = operandType (op);
  sym_link *retype = getSpec (type);
  iCode *lic = ic->next;
  int isize;

  /* this could from a cast, e.g.: "(char xdata *) 0x7654;" */
  if (!IS_SYMOP (op))
    return NULL;

  if (IS_BITVAR (retype) || !IS_PTR (type))
    return NULL;
  if (IS_AGGREGATE (type->next))
    return NULL;
  if (osize != (isize = getSize (type->next)))
    return NULL;
  
  while (lic)
    {
      /* if operand of the form op = op + <sizeof *op> */
      if (lic->op == '+' && isOperandEqual (IC_LEFT (lic), op) &&
          isOperandEqual (IC_RESULT (lic), op) &&
          isOperandLiteral (IC_RIGHT (lic)) && operandLitValue (IC_RIGHT (lic)) == isize)
        {
          return lic;
        }
      /* if the operand used or deffed */
      if (bitVectBitValue (OP_USES (op), lic->key) || lic->defKey == op->key)
        {
          return NULL;
        }
      /* if GOTO or IFX */
      if (lic->op == IFX || lic->op == GOTO || lic->op == LABEL)
        break;
      lic = lic->next;
    }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* genAndOp - for && operation                                     */
/*-----------------------------------------------------------------*/
static void
genAndOp (iCode * ic)
{
  operand *left, *right, *result;
  symbol *tlbl, *tlbl0;
  bool needpulla;

  D (emitcode (";     genAndOp", ""));

  /* note here that && operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  aopOp ((left = IC_LEFT (ic)), ic, false);
  aopOp ((right = IC_RIGHT (ic)), ic, false);
  aopOp ((result = IC_RESULT (ic)), ic, false);

  tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  tlbl0 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

  needpulla = pushRegIfSurv (hc08_reg_a);
  asmopToBool (AOP (left), false);
  emitBranch ("beq", tlbl0);
  asmopToBool (AOP (right), false);
  emitBranch ("beq", tlbl0);
  loadRegFromConst (hc08_reg_a, 1);
  emitBranch ("bra", tlbl);
  if (!regalloc_dry_run)
    emitLabel (tlbl0);
  hc08_dirtyReg (hc08_reg_a, false);
  loadRegFromConst (hc08_reg_a, 0);
  if (!regalloc_dry_run)
    emitLabel (tlbl);
  hc08_dirtyReg (hc08_reg_a, false);

  hc08_useReg (hc08_reg_a);
  hc08_freeReg (hc08_reg_a);

  storeRegToFullAop (hc08_reg_a, AOP (result), false);
  pullOrFreeReg(hc08_reg_a, needpulla);

  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (result, NULL, ic, true);
}


/*-----------------------------------------------------------------*/
/* genOrOp - for || operation                                      */
/*-----------------------------------------------------------------*/
static void
genOrOp (iCode * ic)
{
  operand *left, *right, *result;
  symbol *tlbl, *tlbl0;
  bool needpulla;

  D (emitcode (";     genOrOp", ""));

  /* note here that || operations that are in an
     if statement are taken away by backPatchLabels
     only those used in arthmetic operations remain */
  aopOp ((left = IC_LEFT (ic)), ic, false);
  aopOp ((right = IC_RIGHT (ic)), ic, false);
  aopOp ((result = IC_RESULT (ic)), ic, false);

  tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  tlbl0 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

  needpulla = pushRegIfSurv (hc08_reg_a);
  asmopToBool (AOP (left), false);
  emitBranch ("bne", tlbl0);
  asmopToBool (AOP (right), false);
  emitBranch ("bne", tlbl0);
  loadRegFromConst (hc08_reg_a, 0);
  emitBranch ("bra", tlbl);
  if (!regalloc_dry_run)
    emitLabel (tlbl0);
  hc08_dirtyReg (hc08_reg_a, false);
  loadRegFromConst (hc08_reg_a, 1);
  if (!regalloc_dry_run)
    emitLabel (tlbl);
  hc08_dirtyReg (hc08_reg_a, false);

  hc08_useReg (hc08_reg_a);
  hc08_freeReg (hc08_reg_a);

  storeRegToFullAop (hc08_reg_a, AOP (result), false);
  pullOrFreeReg(hc08_reg_a, needpulla);

  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* isLiteralBit - test if lit == 2^n                               */
/*-----------------------------------------------------------------*/
static int
isLiteralBit (unsigned long lit)
{
  unsigned long pw[32] =
  {
    1L, 2L, 4L, 8L, 16L, 32L, 64L, 128L,
    0x100L, 0x200L, 0x400L, 0x800L,
    0x1000L, 0x2000L, 0x4000L, 0x8000L,
    0x10000L, 0x20000L, 0x40000L, 0x80000L,
    0x100000L, 0x200000L, 0x400000L, 0x800000L,
    0x1000000L, 0x2000000L, 0x4000000L, 0x8000000L,
    0x10000000L, 0x20000000L, 0x40000000L, 0x80000000L
  };
  int idx;

  for (idx = 0; idx < 32; idx++)
    if (lit == pw[idx])
      return idx + 1;
  return 0;
}

/*-----------------------------------------------------------------*/
/* maskByte - apply bit mask to byte of operand                    */
/*-----------------------------------------------------------------*/
void maskByte (operand *op, int offset, unsigned mask)
{
  mask &= 0xff;
  if (mask == 0xff)
    return;
  wassert (offset < op->aop->size);
  bool in_a = (op->aop->type == AOP_REG && op->aop->aopu.aop_reg[offset]->rIdx == A_IDX);
  bool in_x = (op->aop->type == AOP_REG && op->aop->aopu.aop_reg[offset]->rIdx == X_IDX);
  if (in_a)
    {
      emitcode ("and", "#0x%02x", mask);
      regalloc_dry_run_cost += 2;
    }
  else if (in_x && mask == 0x7f)
    {
      emitcode ("lslx", "");
      emitcode ("lsrx", "");
      regalloc_dry_run_cost += 2;
    }
  else if (op->aop->type == AOP_DIR && isLiteralBit (~mask & 0xff))
    {
      int bitpos = isLiteralBit (~mask & 0xff) - 1;
      emitcode ("bclr", "#%d,%s", bitpos & 7, aopAdrStr (op->aop, offset, false));
      regalloc_dry_run_cost += 3;
    }
  else
    {
      bool needpula = pushRegIfUsed (hc08_reg_a);
      loadRegFromAop (hc08_reg_a, op->aop, offset);
      emitcode ("and", "#0x%02x", mask);
      regalloc_dry_run_cost += 2;
      hc08_useReg (hc08_reg_a);
      storeRegToAop (hc08_reg_a, op->aop, offset);
      pullOrFreeReg (hc08_reg_a, needpula);
    }
}

/*-----------------------------------------------------------------*/
/* genAnd  - code for and                                          */
/*-----------------------------------------------------------------*/
static void
genAnd (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0ll;
  unsigned long long litinv;
  int bitpos = -1;
  unsigned char bytemask;
  bool needpulla = false;
  bool earlystore = false;

  D (emitcode (";     genAnd", ""));

  aopOp ((left = IC_LEFT (ic)), ic, false);
  aopOp ((right = IC_RIGHT (ic)), ic, false);
  aopOp ((result = IC_RESULT (ic)), ic, true);

#ifdef DEBUG_TYPE
  DD (emitcode ("", "; Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right)));
  DD (emitcode ("", "; Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right)));
#endif

  /* if left is a literal & right is not then exchange them */
  if (AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT)
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if right is accumulator & left is not then exchange them */
  if (AOP_TYPE (right) == AOP_REG && ! IS_AOP_WITH_A (AOP (left)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  size = (AOP_SIZE (left) >= AOP_SIZE (right)) ? AOP_SIZE (left) : AOP_SIZE (right);

  if (AOP_TYPE (right) == AOP_LIT)
    {
      lit = ullFromVal (AOP (right)->aopu.aop_lit);
      if (size == 1)
        lit &= 0xff;
      else if (size == 2)
        lit &= 0xffff;
      else if (size == 4)
        lit &= 0xffffffff;
      else if (size == 8)
        lit &= 0xffffffffffffffff;
      bitpos = isLiteralBit (lit) - 1;
    }

  if (ifx && AOP_TYPE (result) == AOP_CRY && AOP_TYPE (right) == AOP_LIT && AOP_TYPE (left) == AOP_DIR && bitpos >= 0)
    {
      symbol *tlbl = NULL;
      if (!regalloc_dry_run)
        tlbl = newiTempLabel (NULL);
      if (IC_TRUE (ifx))
        {
          if (!regalloc_dry_run)
            emitcode ("brclr", "#%d,%s,%05d$", bitpos & 7, aopAdrStr (AOP (left), bitpos >> 3, false), labelKey2num ((tlbl->key)));
          regalloc_dry_run_cost += 3;
          emitBranch ("jmp", IC_TRUE (ifx));
          if (!regalloc_dry_run)
            emitLabel (tlbl);
          if (IC_FALSE (ifx))
            emitBranch ("jmp", IC_FALSE (ifx));
        }
      else
        {
          if (!regalloc_dry_run)
            emitcode ("brset", "#%d,%s,%05d$", bitpos & 7, aopAdrStr (AOP (left), bitpos >> 3, false), labelKey2num ((tlbl->key)));
          regalloc_dry_run_cost += 3;
          emitBranch ("jmp", IC_FALSE (ifx));
          if (!regalloc_dry_run)
            emitLabel (tlbl);
        }
      ifx->generated = true;
      goto release;
    }

  if (AOP_TYPE (result) == AOP_CRY && size > 1 && (isOperandVolatile (left, false) || isOperandVolatile (right, false)))
    {
      needpulla = pushRegIfSurv (hc08_reg_a);

      /* this generates ugly code, but meets volatility requirements */
      loadRegFromConst (hc08_reg_a, 0);
      pushReg (hc08_reg_a, true);

      offset = 0;
      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (left), offset);
          accopWithAop ("and", AOP (right), offset);
          emitcode ("ora", "1,s");
          emitcode ("sta", "1,s");
          regalloc_dry_run_cost += 6;
          offset++;
        }

      pullReg (hc08_reg_a);
      emitcode ("tsta", "");
      regalloc_dry_run_cost++;

      pullOrFreeReg (hc08_reg_a, needpulla);

      genIfxJump (ifx, "a");

      goto release;
    }

  if (AOP_TYPE (result) == AOP_CRY && AOP_TYPE (right) == AOP_LIT)
    {
      if (bitpos >= 0 && (bitpos & 7) == 7)
        {
          rmwWithAop ("tst", AOP (left), bitpos >> 3);
          genIfxJump (ifx, "n");
          goto release;
        }
    }

  if (AOP_TYPE (result) == AOP_CRY && size == 1 && (IS_AOP_A (AOP (left)) || IS_AOP_A (AOP (right))))
    {
      if (IS_AOP_A (AOP (left)))
        accopWithAop ("bit", AOP (right), 0);
      else
        accopWithAop ("bit", AOP (left), 0);
      genIfxJump (ifx, "a");
      goto release;
    }

  if (AOP_TYPE (result) == AOP_CRY)
    {
      symbol *tlbl = NULL;

      needpulla = pushRegIfSurv (hc08_reg_a);

      offset = 0;
      while (size--)
        {
          bytemask = (lit >> (offset * 8)) & 0xff;

          if (AOP_TYPE (right) == AOP_LIT && bytemask == 0)
            {
              /* do nothing */
            }
          else if (AOP_TYPE (right) == AOP_LIT && bytemask == 0xff)
            {
              rmwWithAop ("tst", AOP (left), offset);
              if (size)
                {
                  if (!tlbl && !regalloc_dry_run)
                    tlbl = newiTempLabel (NULL);
                  emitBranch ("bne", tlbl);
                }
            }
          else
            {
              loadRegFromAop (hc08_reg_a, AOP (left), offset);
              accopWithAop ("bit", AOP (right), offset);
              hc08_freeReg (hc08_reg_a);
              if (size)
                {
                  if (!tlbl && !regalloc_dry_run)
                    tlbl = newiTempLabel (NULL);
                  emitBranch ("bne", tlbl);
                }
            }
          offset++;
        }
      if (tlbl)
        emitLabel (tlbl);

      pullOrFreeReg (hc08_reg_a, needpulla);

      if (ifx)
        genIfxJump (ifx, "a");
      goto release;
    }

  size = AOP_SIZE (result);

  if (AOP_TYPE (right) == AOP_LIT)
    {
      litinv = (~lit) & ((0xffffffffffffffffull) >> (8 * (8 - size)));
      if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))) && (AOP_TYPE (left) == AOP_DIR) && isLiteralBit (litinv))
        {
          bitpos = isLiteralBit (litinv) - 1;
          emitcode ("bclr", "#%d,%s", bitpos & 7, aopAdrStr (AOP (left), bitpos >> 3, false));
          regalloc_dry_run_cost += 2;
          goto release;
        }
    }

  needpulla = pushRegIfSurv (hc08_reg_a);

  offset = 0;
  if (size >= 2 && IS_AOP_AX (AOP (left)))
    {
      pushReg (hc08_reg_a, true);
      earlystore = true;
    }
  while (size--)
    {
      if (earlystore && offset == 1)
        pullReg (hc08_reg_a);
      if (aopIsLitVal (left->aop, offset, 1, 0x00) || aopIsLitVal (right->aop, offset, 1, 0x00))
        {
          if (isOperandVolatile (left, false))
            {
              loadRegFromAop (hc08_reg_a, AOP (left), offset);
              hc08_freeReg (hc08_reg_a);
            }
          if (isOperandVolatile (right, false))
            {
              loadRegFromAop (hc08_reg_a, AOP (left), offset);
              hc08_freeReg (hc08_reg_a);
            }
          storeConstToAop (0, AOP (result), offset);
        }
      else if (aopIsLitVal (right->aop, offset, 1, 0xff) && !isOperandVolatile (left, false))
        transferAopAop (left->aop, offset, result->aop, offset);
      else if (aopIsLitVal (left->aop, offset, 1, 0xff) && !isOperandVolatile (right, false))
        transferAopAop (right->aop, offset, result->aop, offset);
      else
        {
          loadRegFromAop (hc08_reg_a, AOP (left), offset);
          accopWithAop ("and", AOP (right), offset);
          storeRegToAop (hc08_reg_a, AOP (result), offset);
          hc08_freeReg (hc08_reg_a);
        }
      if (AOP_TYPE (result) == AOP_REG && size && AOP (result)->aopu.aop_reg[offset]->rIdx == A_IDX)
        {
          pushReg (hc08_reg_a, true);
          needpulla = true;
        }
      offset++;
    }

  pullOrFreeReg (hc08_reg_a, needpulla);

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genOr  - code for or                                            */
/*-----------------------------------------------------------------*/
static void
genOr (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  unsigned long long lit = 0ull;
  unsigned char bytemask;
  bool needpulla = false;
  bool earlystore = false;

  D (emitcode (";     genOr", ""));

  aopOp ((left = IC_LEFT (ic)), ic, false);
  aopOp ((right = IC_RIGHT (ic)), ic, false);
  aopOp ((result = IC_RESULT (ic)), ic, true);

#ifdef DEBUG_TYPE
  DD (emitcode ("", "; Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right)));
  DD (emitcode ("", "; Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right)));
#endif

  /* if left is a literal & right is not then exchange them */
  if (AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT)
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if left is accumulator & right is not then exchange them */
  if (AOP_TYPE (right) == AOP_REG && !IS_AOP_WITH_A (AOP (left)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  if (AOP_TYPE (right) == AOP_LIT)
    lit = ullFromVal (AOP (right)->aopu.aop_lit);

  size = (AOP_SIZE (left) >= AOP_SIZE (right)) ? AOP_SIZE (left) : AOP_SIZE (right);

  if (AOP_TYPE (result) == AOP_CRY && size > 1 && (isOperandVolatile (left, false) || isOperandVolatile (right, false)))
    {
      needpulla = pushRegIfSurv (hc08_reg_a);

      /* this generates ugly code, but meets volatility requirements */
      loadRegFromConst (hc08_reg_a, 0);
      pushReg (hc08_reg_a, true);

      offset = 0;
      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (left), offset);
          accopWithAop ("ora", AOP (right), offset);
          emitcode ("ora", "1,s");
          emitcode ("sta", "1,s");
          regalloc_dry_run_cost += 6;
          offset++;
        }

      pullReg (hc08_reg_a);
      emitcode ("tsta", "");
      regalloc_dry_run_cost++;

      pullOrFreeReg (hc08_reg_a, needpulla);

      genIfxJump (ifx, "a");

      goto release;
    }

  if (AOP_TYPE (result) == AOP_CRY)
    {
      symbol *tlbl = NULL;

      needpulla = pushRegIfSurv (hc08_reg_a);

      offset = 0;
      while (size--)
        {
          bytemask = (lit >> (offset * 8)) & 0xff;

          if (AOP_TYPE (right) == AOP_LIT && bytemask == 0x00)
            {
              rmwWithAop ("tst", AOP (left), offset);
              if (size)
                {
                  if (!tlbl && !regalloc_dry_run)
                    tlbl = newiTempLabel (NULL);
                  emitBranch ("bne", tlbl);
                }
            }
          else
            {
              loadRegFromAop (hc08_reg_a, AOP (left), offset);
              accopWithAop ("ora", AOP (right), offset);
              hc08_freeReg (hc08_reg_a);
              if (size)
                {
                  if (!tlbl && !regalloc_dry_run)
                    tlbl = newiTempLabel (NULL);
                  emitBranch ("bne", tlbl);
                }
            }
          offset++;
        }
      if (tlbl)
        emitLabel (tlbl);

      pullOrFreeReg (hc08_reg_a, needpulla);

      if (ifx)
        genIfxJump (ifx, "a");

      goto release;
    }

  if (AOP_TYPE (right) == AOP_LIT)
    lit = ullFromVal (AOP (right)->aopu.aop_lit);

  size = AOP_SIZE (result);

  if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))) &&
      (AOP_TYPE (right) == AOP_LIT) && isLiteralBit (lit) && (AOP_TYPE (left) == AOP_DIR))
    {
      int bitpos = isLiteralBit (lit) - 1;
      emitcode ("bset", "#%d,%s", bitpos & 7, aopAdrStr (AOP (left), bitpos >> 3, false));
      regalloc_dry_run_cost += 2;
      goto release;
    }

  needpulla = pushRegIfSurv (hc08_reg_a);

  offset = 0;
  if (size >= 2 && IS_AOP_AX (AOP (left)))
    {
      pushReg (hc08_reg_a, true);
      earlystore = true;
    }
  while (size--)
    {
      if (earlystore && offset == 1)
        pullReg (hc08_reg_a);
      if (aopIsLitVal (right->aop, offset, 1, 0xff))
        {
          if (isOperandVolatile (left, false))
            {
              loadRegFromAop (hc08_reg_a, AOP (left), offset);
              hc08_freeReg (hc08_reg_a);
            }
          transferAopAop (AOP (right), offset, AOP (result), offset);
        }
      else if (aopIsLitVal (right->aop, offset, 1, 0x00))
        transferAopAop (left->aop, offset, result->aop, offset);
      else if (aopIsLitVal (left->aop, offset, 1, 0x00))
        transferAopAop (right->aop, offset, result->aop, offset);
      else
        {
          loadRegFromAop (hc08_reg_a, AOP (left), offset);
          accopWithAop ("ora", AOP (right), offset);
          storeRegToAop (hc08_reg_a, AOP (result), offset);
          hc08_freeReg (hc08_reg_a);
        }
      if (AOP_TYPE (result) == AOP_REG && size && AOP (result)->aopu.aop_reg[offset]->rIdx == A_IDX)
        {
          pushReg (hc08_reg_a, true);
          needpulla = true;
        }
      offset++;
    }

  pullOrFreeReg (hc08_reg_a, needpulla);

release:
  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genXor - code for Exclusive or                                  */
/*-----------------------------------------------------------------*/
static void
genXor (iCode * ic, iCode * ifx)
{
  operand *left, *right, *result;
  int size, offset = 0;
  bool needpulla = false;
  bool earlystore = false;

  D (emitcode (";     genXor", ""));

  aopOp ((left = IC_LEFT (ic)), ic, false);
  aopOp ((right = IC_RIGHT (ic)), ic, false);
  aopOp ((result = IC_RESULT (ic)), ic, true);

#ifdef DEBUG_TYPE
  DD (emitcode ("", "; Type res[%d] = l[%d]&r[%d]", AOP_TYPE (result), AOP_TYPE (left), AOP_TYPE (right)));
  DD (emitcode ("", "; Size res[%d] = l[%d]&r[%d]", AOP_SIZE (result), AOP_SIZE (left), AOP_SIZE (right)));
#endif

  /* if left is a literal & right is not ||
     if left needs acc & right does not */
  if (AOP_TYPE (left) == AOP_LIT && AOP_TYPE (right) != AOP_LIT)
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  /* if left is accumulator & right is not then exchange them */
  if (AOP_TYPE (right) == AOP_REG && !IS_AOP_WITH_A (AOP (left)))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  needpulla = pushRegIfSurv (hc08_reg_a);

  if (AOP_TYPE (result) == AOP_CRY)
    {
      symbol *tlbl;

      tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
      size = (AOP_SIZE (left) >= AOP_SIZE (right)) ? AOP_SIZE (left) : AOP_SIZE (right);
      offset = 0;
      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (left), offset);
          if (AOP_TYPE (right) == AOP_LIT && ((ullFromVal (AOP (right)->aopu.aop_lit) >> (offset * 8)) & 0xff) == 0)
            {
              emitcode ("tsta", "");
              regalloc_dry_run_cost++;
            }
          else
            accopWithAop ("eor", AOP (right), offset);

          hc08_freeReg (hc08_reg_a);
          if (size)
            emitBranch ("bne", tlbl);
          else
            {
              /*
               * I think this is all broken here, (see simulation mismatch in bug1875933.c)
               *   multiple calls to emitLabel() ?!
               * and we can't genIfxJump, if there is none
               */
              if (!regalloc_dry_run)
                emitLabel (tlbl);
              pullOrFreeReg (hc08_reg_a, needpulla);
              if (ifx)
                genIfxJump (ifx, "a");
            }
          offset++;
        }
      goto release;
    }

  size = AOP_SIZE (result);
  offset = 0;
  if (size >= 2 && IS_AOP_AX (AOP (left)))
    {
      pushReg (hc08_reg_a, true);
      earlystore = true;
    }
  while (size--)
    {
      if (earlystore && offset == 1)
        pullReg (hc08_reg_a);
      loadRegFromAop (hc08_reg_a, AOP (left), offset);
      if (!aopIsLitVal (right->aop, offset, 1, 0x00))
        accopWithAop ("eor", right->aop, offset);
      storeRegToAop (hc08_reg_a, AOP (result), offset);
      if (AOP_TYPE (result) == AOP_REG && size && AOP (result)->aopu.aop_reg[offset]->rIdx == A_IDX)
        {
          pushReg (hc08_reg_a, true);
          needpulla = true;
        }
      hc08_freeReg (hc08_reg_a);
      offset++;
    }

  pullOrFreeReg (hc08_reg_a, needpulla);

release:

  freeAsmop (left, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (right, NULL, ic, (RESULTONSTACK (ic) ? false : true));
  freeAsmop (result, NULL, ic, true);
}

static const char *
expand_symbols (iCode * ic, const char *inlin)
{
  const char *begin = NULL, *p = inlin;
  bool inIdent = false;
  struct dbuf_s dbuf;

  dbuf_init (&dbuf, 128);

  while (*p)
    {
      if (inIdent)
        {
          if ('_' == *p || isalnum (*p))
            /* in the middle of identifier */
            ++p;
          else
            {
              /* end of identifier */
              symbol *sym, *tempsym;
              char *symname = Safe_strndup (p + 1, p - begin - 1);

              inIdent = 0;

              tempsym = newSymbol (symname, ic->level);
              tempsym->block = ic->block;
              sym = (symbol *) findSymWithLevel (SymbolTab, tempsym);
              if (!sym)
                {
                  dbuf_append (&dbuf, begin, p - begin);
                }
              else
                {
                  asmop *aop = aopForSym (ic, sym, false);
                  const char *l = aopAdrStr (aop, aop->size - 1, true);

                  if ('#' == *l)
                    l++;
                  sym->isref = 1;
                  if (sym->level && !sym->allocreq && !sym->ismyparm)
                    {
                      werror (E_ID_UNDEF, sym->name);
                      werror (W_CONTINUE,
                              "  Add 'volatile' to the variable declaration so that it\n"
                              "  can be referenced within inline assembly");
                    }
                  dbuf_append_str (&dbuf, l);
                }
              Safe_free (symname);
              begin = p++;
            }
        }
      else if ('_' == *p)
        {
          /* begin of identifier */
          inIdent = true;
          if (begin)
            dbuf_append (&dbuf, begin, p - begin);
          begin = p++;
        }
      else
        {
          if (!begin)
            begin = p;
          p++;
        }
    }

  if (begin)
    dbuf_append (&dbuf, begin, p - begin);

  return dbuf_detach_c_str (&dbuf);
}

/*-----------------------------------------------------------------*/
/* genInline - write the inline code out                           */
/*-----------------------------------------------------------------*/
static void
hc08_genInline (iCode * ic)
{
  char *buf, *bp, *begin;
  const char *expanded;
  bool inComment = false;

  D (emitcode (";", "genInline"));

  genLine.lineElement.isInline += (!options.asmpeep);

  buf = bp = begin = Safe_strdup (IC_INLINE (ic));

  /* emit each line as a code */
  while (*bp)
    {
      switch (*bp)
        {
        case ';':
          inComment = true;
          ++bp;
          break;

        case '\x87':
        case '\n':
          inComment = false;
          *bp++ = '\0';
          expanded = expand_symbols (ic, begin);
          emitcode (expanded, NULL);
          dbuf_free (expanded);
          begin = bp;
          break;

        default:
          /* Add \n for labels, not dirs such as c:\mydir */
          if (!inComment && (*bp == ':') && (isspace ((unsigned char) bp[1])))
            {
              ++bp;
              *bp = '\0';
              ++bp;
              emitcode (begin, NULL);
              begin = bp;
            }
          else
            ++bp;
          break;
        }
    }
  if (begin != bp)
    {
      expanded = expand_symbols (ic, begin);
      emitcode (expanded, NULL);
      dbuf_free (expanded);
    }

  Safe_free (buf);

  /* consumed; we can free it here */
  dbuf_free (IC_INLINE (ic));

  genLine.lineElement.isInline -= (!options.asmpeep);
}

/*-----------------------------------------------------------------*/
/* genRRC - rotate right with carry                                */
/*-----------------------------------------------------------------*/
static void
genRRC (iCode * ic)
{
  operand *left, *result;
  int size, offset = 0;
  bool needpula = false;
  bool resultInA = false;
  char *shift;

  D (emitcode (";     genRRC", ""));

  /* rotate right with carry */
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false);
  aopOp (result, ic, false);

  if ((AOP_TYPE (result) == AOP_REG) && (AOP (result)->aopu.aop_reg[0]->rIdx == A_IDX))
    resultInA = true;

  size = AOP_SIZE (result);
  offset = size - 1;

  shift = "lsr";
  if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
    {
      while (size--)
        {
          rmwWithAop (shift, AOP (result), offset--);
          shift = "ror";
        }
    }
  else
    {
      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (left), offset);
          rmwWithReg (shift, hc08_reg_a);
          storeRegToAop (hc08_reg_a, AOP (result), offset--);
          hc08_freeReg (hc08_reg_a);
          shift = "ror";
        }
    }

  if ((!hc08_reg_a->isFree) || resultInA)
    {
      pushReg (hc08_reg_a, true);
      needpula = true;
    }

  /* now we need to put the carry into the
     highest order byte of the result */
  offset = AOP_SIZE (result) - 1;
  emitcode ("clra", "");
  emitcode ("rora", "");
  regalloc_dry_run_cost += 2;
  hc08_dirtyReg (hc08_reg_a, false);
  if (resultInA)
    {
      emitcode ("ora", "1,s");
      pullNull (1);
      regalloc_dry_run_cost += 3;
      hc08_dirtyReg (hc08_reg_a, false);
      needpula = false;
    }
  else
    accopWithAop ("ora", AOP (result), offset);
  storeRegToAop (hc08_reg_a, AOP (result), offset);

  pullOrFreeReg (hc08_reg_a, needpula);

  freeAsmop (left, NULL, ic, true);
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genRLC - generate code for rotate left with carry               */
/*-----------------------------------------------------------------*/
static void
genRLC (iCode * ic)
{
  operand *left, *result;
  int size, offset = 0;
  char *shift;
  bool resultInA = false;
  bool needpula = false;

  D (emitcode (";     genRLC", ""));

  /* rotate right with carry */
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false);
  aopOp (result, ic, false);

  if ((AOP_TYPE (result) == AOP_REG) && (AOP (result)->aopu.aop_reg[0]->rIdx == A_IDX))
    resultInA = true;

  size = AOP_SIZE (result);
  offset = 0;

  shift = "lsl";
  if (sameRegs (AOP (IC_LEFT (ic)), AOP (IC_RESULT (ic))))
    {
      while (size--)
        {
          rmwWithAop (shift, AOP (result), offset++);
          shift = "rol";
        }
    }
  else
    {
      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (left), offset);
          rmwWithReg (shift, hc08_reg_a);
          storeRegToAop (hc08_reg_a, AOP (result), offset++);
          hc08_freeReg (hc08_reg_a);
          shift = "rol";
        }
    }

  /* now we need to put the carry into the
     lowest order byte of the result */
  if (resultInA)
    {
      emitcode ("adc", "#0x00");
      regalloc_dry_run_cost += 2;
    }
  else
    {
      if (!hc08_reg_a->isFree)
        {
          pushReg (hc08_reg_a, true);
          needpula = true;
        }
      offset = 0;
      emitcode ("clra", "");
      emitcode ("rola", "");
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_a, false);
      accopWithAop ("ora", AOP (result), offset);
      storeRegToAop (hc08_reg_a, AOP (result), offset);
    }

  pullOrFreeReg (hc08_reg_a, needpula);

  freeAsmop (left, NULL, ic, true);
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genGetAbit - generates code get a single bit                    */
/*-----------------------------------------------------------------*/
static void
genGetAbit (iCode * ic)
{
  operand *left, *right, *result;
  int shCount;
  bool needpulla;

  D (emitcode (";     genGetAbit", ""));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, false);

  shCount = (int) ulFromVal (AOP (IC_RIGHT (ic))->aopu.aop_lit);

  needpulla = pushRegIfSurv (hc08_reg_a);

  /* get the needed byte into a */
  loadRegFromAop (hc08_reg_a, AOP (left), shCount / 8);
  shCount %= 8;
  if (AOP_TYPE (result) == AOP_CRY)
    {
      emitcode ("and", "#0x%02x", 1 << shCount);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_a, false);
    }
  else
    {
      switch (shCount)
        {
        case 3:
          emitcode ("lsra", "");
          regalloc_dry_run_cost++;
          //fallthrough
        case 2:
          emitcode ("lsra", "");
          regalloc_dry_run_cost++;
          //fallthrough
        case 1:
          emitcode ("lsra", "");
          regalloc_dry_run_cost++;
          //fallthrough
        case 0:
          emitcode ("and", "#0x01");
          regalloc_dry_run_cost += 2;
          break;
        case 4:
          emitcode ("nsa", "");
          emitcode ("and", "#0x01");
          regalloc_dry_run_cost += 3;
          break;
        case 5:
          emitcode ("rola", "");
          regalloc_dry_run_cost++;
          //fallthrough
        case 6:
          emitcode ("rola", "");
          regalloc_dry_run_cost++;
          //fallthrough
        case 7:
          emitcode ("rola", "");
          emitcode ("clra", "");
          emitcode ("rola", "");
          regalloc_dry_run_cost += 3;
          break;
        }
      hc08_dirtyReg (hc08_reg_a, false);
      storeRegToFullAop (hc08_reg_a, AOP (result), false);
    }
  pullOrFreeReg (hc08_reg_a, needpulla);

  freeAsmop (result, NULL, ic, true);
  freeAsmop (right, NULL, ic, true);
  freeAsmop (left, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genGetByte - generates code get a single byte                   */
/*-----------------------------------------------------------------*/
static void
genGetByte (iCode * ic)
{
  operand *left, *right, *result;
  int offset;

  D (emitcode (";", "genGetByte"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, false);

  offset = (int) ulFromVal (AOP (right)->aopu.aop_lit) / 8;
  transferAopAop (AOP (left), offset, AOP (result), 0);

  freeAsmop (result, NULL, ic, true);
  freeAsmop (right, NULL, ic, true);
  freeAsmop (left, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genGetWord - generates code get two bytes                       */
/*-----------------------------------------------------------------*/
static void
genGetWord (iCode * ic)
{
  operand *left, *right, *result;
  int offset;

  D (emitcode (";", "genGetWord"));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, false);

  offset = (int) ulFromVal (AOP (right)->aopu.aop_lit) / 8;
  transferAopAop (AOP (left), offset + 1, AOP (result), 1);
  transferAopAop (AOP (left), offset, AOP (result), 0);

  freeAsmop (result, NULL, ic, true);
  freeAsmop (right, NULL, ic, true);
  freeAsmop (left, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genSwap - generates code to swap nibbles or bytes               */
/*-----------------------------------------------------------------*/
static void
genSwap (iCode * ic)
{
  operand *left, *result;
  bool needpulla;

  D (emitcode (";     genSwap", ""));

  left = IC_LEFT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false);
  aopOp (result, ic, false);

  switch (AOP_SIZE (left))
    {
    case 2:                    /* swap bytes in a word */
      if (IS_AOP_XA (AOP (left)) && IS_AOP_AX (AOP (result)) ||
        IS_AOP_AX (AOP (left)) && IS_AOP_XA (AOP (result)))
        break;
      if (AOP_TYPE (result) == AOP_REG && AOP_TYPE (left) == AOP_REG)
        {
          if (AOP (result)->aopu.aop_reg[1] != AOP (left)->aopu.aop_reg[0])
            pushReg (AOP (left)->aopu.aop_reg[0], true);
          storeRegToAop (AOP (left)->aopu.aop_reg[1], AOP (result), 0);
          if (AOP (result)->aopu.aop_reg[1] != AOP (left)->aopu.aop_reg[0])
            pullReg (AOP (result)->aopu.aop_reg[1]);
        }
      else if (operandsEqu (left, result) || sameRegs (AOP (left), AOP (result)))
        {
          needpulla = pushRegIfSurv (hc08_reg_a);
          loadRegFromAop (hc08_reg_a, AOP (left), 0);
          hc08_useReg (hc08_reg_a);
          transferAopAop (AOP (left), 1, AOP (result), 0);
          storeRegToAop (hc08_reg_a, AOP (result), 1);
          pullOrFreeReg (hc08_reg_a, needpulla);
        }
      else
        {
          transferAopAop (AOP (left), 0, AOP (result), 1);
          transferAopAop (AOP (left), 1, AOP (result), 0);
        }
      break;
    default:
      wassertl (false, "unsupported SWAP operand size");
    }

  freeAsmop (left, NULL, ic, true);
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* AccRol - rotate left accumulator by known count                 */
/*-----------------------------------------------------------------*/
static void
AccRol (int shCount)
{
  shCount &= 0x0007;            // shCount : 0..7

  switch (shCount)
    {
    case 0:
      break;
    case 1:
      emitcode ("lsla", "");    /* 1 cycle */
      emitcode ("adc", "#0x00");/* 2 cycles */
      regalloc_dry_run_cost += 3;
      break;
    case 2:
      emitcode ("lsla", "");    /* 1 cycle */
      emitcode ("adc", "#0x00");/* 2 cycles */
      emitcode ("lsla", "");    /* 1 cycle */
      emitcode ("adc", "#0x00");/* 2 cycles */
      regalloc_dry_run_cost += 6;
      break;
    case 3:
      emitcode ("nsa", "");     /* 3 cycles */
      regalloc_dry_run_cost++;
      goto ror;
    case 4:
      emitcode ("nsa", "");     /* 3 cycles */
      regalloc_dry_run_cost++;
      break;
    case 5:
      emitcode ("nsa", "");     /* 3 cycles */
      emitcode ("lsla", "");    /* 1 cycle */
      emitcode ("adc", "#0x00");/* 2 cycles */
      regalloc_dry_run_cost += 4;
      break;
    case 6:
      emitcode ("nsa", "");     /* 3 cycles */
      emitcode ("lsla", "");    /* 1 cycle */
      emitcode ("adc", "#0x00");/* 2 cycles */
      emitcode ("lsla", "");    /* 1 cycle */
      emitcode ("adc", "#0x00");/* 2 cycles */
      regalloc_dry_run_cost += 7;
      break;
    case 7:
ror:
      emitcode ("lsra", "");     /* 1 cycle */
      emitcode ("psha", "");     /* 2 cycles */
      emitcode ("clra", "");     /* 1 cycle */
      emitcode ("rora", "");     /* 1 cycles */
      emitcode ("ora", "1, s"); /* 1 cycles */
      regalloc_dry_run_cost += 6;
      if (hc08_reg_h->isFree && optimize.codeSize)
        {
          emitcode ("pulh", "");      /* 1 byte,  3 cycles */
          regalloc_dry_run_cost++;
        }
      else
        {
          emitcode ("ais", "#1"); /* 2 bytes, 2 cycles */
          regalloc_dry_run_cost += 2;
        }
      break;
    }
  hc08_dirtyReg (hc08_reg_a, false);
}

/*-----------------------------------------------------------------*/
/* AccLsh - left shift accumulator by known count                  */
/*-----------------------------------------------------------------*/
static void
AccLsh (int shCount)
{
  int i;

  shCount &= 0x0007;            // shCount : 0..7

  /* Shift counts of 4 and 5 are currently optimized for code size.        */
  /* Falling through to the unrolled loop would be optimal for code speed. */
  /* For shift counts of 6 and 7, the unrolled loop is never optimal.      */
  switch (shCount)
    {
    case 4:
      if (optimize.codeSpeed)
        break;
      accopWithMisc ("nsa", "");
      accopWithMisc ("and", "#0xf0");
      /* total: 5 cycles, 3 bytes */
      return;
    case 5:
      if (optimize.codeSpeed)
        break;
      accopWithMisc ("nsa", "");
      accopWithMisc ("and", "#0xf0");
      accopWithMisc ("lsla", "");
      /* total: 6 cycles, 4 bytes */
      return;
    case 6:
      accopWithMisc ("rora", "");
      accopWithMisc ("rora", "");
      accopWithMisc ("rora", "");
      accopWithMisc ("and", "#0xc0");
      /* total: 5 cycles, 5 bytes */
      return;
    case 7:
      accopWithMisc ("rora", "");
      accopWithMisc ("clra", "");
      accopWithMisc ("rora", "");
      /* total: 3 cycles, 3 bytes */
      return;
    }

  /* lsla is only 1 cycle and byte, so an unrolled loop is often  */
  /* the fastest (shCount<6) and shortest (shCount<4).            */
  for (i = 0; i < shCount; i++)
    accopWithMisc ("lsla", "");
}


/*-----------------------------------------------------------------*/
/* AccSRsh - signed right shift accumulator by known count         */
/*-----------------------------------------------------------------*/
static void
AccSRsh (int shCount)
{
  int i;

  shCount &= 0x0007;            // shCount : 0..7

  if (shCount == 7)
    {
      accopWithMisc ("rola", "");
      accopWithMisc ("clra", "");
      accopWithMisc ("sbc", zero);
      /* total: 4 cycles, 4 bytes */
      return;
    }

  for (i = 0; i < shCount; i++)
    accopWithMisc ("asra", "");
}

/*-----------------------------------------------------------------*/
/* AccRsh - right shift accumulator by known count                 */
/*-----------------------------------------------------------------*/
static void
AccRsh (int shCount, bool sign)
{
  int i;

  if (sign)
    {
      AccSRsh (shCount);
      return;
    }

  shCount &= 0x0007;            // shCount : 0..7

  /* Shift counts of 4 and 5 are currently optimized for code size.        */
  /* Falling through to the unrolled loop would be optimal for code speed. */
  /* For shift counts of 6 and 7, the unrolled loop is never optimal.      */
  switch (shCount)
    {
    case 4:
      if (optimize.codeSpeed)
        break;
      accopWithMisc ("nsa", "");
      accopWithMisc ("and", "#0x0f");
      /* total: 5 cycles, 3 bytes */
      return;
    case 5:
      if (optimize.codeSpeed)
        break;
      accopWithMisc ("nsa", "");
      accopWithMisc ("and", "#0x0f");
      accopWithMisc ("lsra", "");
      /* total: 6 cycles, 4 bytes */
      return;
    case 6:
      accopWithMisc ("rola", "");
      accopWithMisc ("rola", "");
      accopWithMisc ("rola", "");
      accopWithMisc ("and", "#0x03");
      /* total: 5 cycles, 5 bytes */
      return;
    case 7:
      accopWithMisc ("rola", "");
      accopWithMisc ("clra", "");
      accopWithMisc ("rola", "");
      /* total: 3 cycles, 3 bytes */
      return;
    }

  /* lsra is only 1 cycle and byte, so an unrolled loop is often  */
  /* the fastest (shCount<6) and shortest (shCount<4).            */
  for (i = 0; i < shCount; i++)
    accopWithMisc ("lsra", "");
}


/*-----------------------------------------------------------------*/
/* XAccLsh - left shift register pair XA by known count            */
/*-----------------------------------------------------------------*/
static void
XAccLsh (int shCount)
{
  int i;

  shCount &= 0x000f;            // shCount : 0..15

  if (shCount >= 8)
    {
      AccLsh (shCount - 8);
      transferRegReg (hc08_reg_a, hc08_reg_x, false);
      loadRegFromConst (hc08_reg_a, 0);
      return;
    }

  /* if we can beat 2n cycles or bytes for some special case, do it here */
  switch (shCount)
    {
    case 7:
      /*          bytes  cycles     reg x      reg a   carry
       **                          abcd efgh  ijkl mnop   ?
       **   lsrx       1  1        0abc defg  ijkl mnop   h
       **   rora       1  1        0abc defg  hijk lmno   p
       **   tax        1  1        hijk lmno  hijk lmno   p
       **   clra       1  1        hijk lmno  0000 0000   p
       **   rora       1  1        hijk lmno  p000 0000   0
       ** total: 5 cycles, 5 bytes (beats 14 cycles, 14 bytes)
       */
      rmwWithReg ("lsr", hc08_reg_x);
      rmwWithReg ("ror", hc08_reg_a);
      transferRegReg (hc08_reg_a, hc08_reg_x, false);
      loadRegFromConst (hc08_reg_a, 0);
      rmwWithReg ("ror", hc08_reg_a);
      return;

    default:
      ;
    }

  /* lsla/rolx is only 2 cycles and bytes, so an unrolled loop is often  */
  /* the fastest and shortest.                                           */
  for (i = 0; i < shCount; i++)
    {
      rmwWithReg ("lsl", hc08_reg_a);
      rmwWithReg ("rol", hc08_reg_x);
    }
}

/*-----------------------------------------------------------------*/
/* XAccSRsh - signed right shift register pair XA by known count   */
/*-----------------------------------------------------------------*/
static void
XAccSRsh (int shCount)
{
  int i;

  shCount &= 0x000f;            // shCount : 0..7

  /* if we can beat 2n cycles or bytes for some special case, do it here */
  switch (shCount)
    {
    case 15:
      /*          bytes  cycles     reg x      reg a   carry
       **                          abcd efgh  ijkl mnop   ?
       **   lslx       1  1        bcde fgh0  ijkl mnop   a
       **   clra       1  1        bcde fgh0  0000 0000   a
       **   rola       1  1        bcde fgh0  0000 000a   0
       **   nega       1  1        bcde fgh0  aaaa aaaa   a
       **   tax        1  1        aaaa aaaa  aaaa aaaa   a
       ** total: 5 cycles, 5 bytes
       */
      rmwWithReg ("lsl", hc08_reg_x);
      loadRegFromConst (hc08_reg_a, 0);
      rmwWithReg ("rol", hc08_reg_a);
      rmwWithReg ("neg", hc08_reg_a);
      transferRegReg (hc08_reg_a, hc08_reg_x, false);
      return;

    case 14:
    case 13:
    case 12:
    case 11:
    case 10:
    case 9:
    case 8:
      /*          bytes  cycles     reg x      reg a   carry
       **                          abcd efgh  ijkl mnop   ?
       **   txa        1  1        abcd efgh  abcd efgh   ?
       **   (AccSRsh) <8 <8        abcd efgh  LSBresult   ?
       **   lsla       1  1        abcd efgh  ???? ????   a
       **   clrx       1  1        0000 0000  ???? ????   a
       **   rolx       1  1        0000 000a  ???? ????   0
       **   negx       1  1        aaaa aaaa  ???? ????   a
       **   rora       1  1        aaaa aaaa  LSBresult   0
       ** total: n-2 cycles, n-2 bytes (beats 2n cycles, 2n bytes (for n>=8))
       */
      transferRegReg (hc08_reg_x, hc08_reg_a, false);
      AccSRsh (shCount - 8);
      rmwWithReg ("lsl", hc08_reg_a);
      loadRegFromConst (hc08_reg_x, 0);
      rmwWithReg ("rol", hc08_reg_x);
      rmwWithReg ("neg", hc08_reg_x);
      rmwWithReg ("ror", hc08_reg_a);
      return;

    default:
      ;
    }

  /* asrx/rora is only 2 cycles and bytes, so an unrolled loop is often  */
  /* the fastest and shortest.                                           */
  for (i = 0; i < shCount; i++)
    {
      rmwWithReg ("asr", hc08_reg_x);
      rmwWithReg ("ror", hc08_reg_a);
    }
}

/*-----------------------------------------------------------------*/
/* XAccRsh - right shift register pair XA by known count           */
/*-----------------------------------------------------------------*/
static void
XAccRsh (int shCount, bool sign)
{
  int i;

  if (sign)
    {
      XAccSRsh (shCount);
      return;
    }

  shCount &= 0x000f;            // shCount : 0..f

  /* if we can beat 2n cycles or bytes for some special case, do it here */
  switch (shCount)
    {
    case 15:
      /*          bytes  cycles     reg x      reg a   carry
       **                          abcd efgh  ijkl mnop   ?
       **   clra       1  1        abcd efgh  0000 0000   a
       **   lslx       1  1        bcde fgh0  0000 0000   a
       **   rola       1  1        bcde fgh0  0000 000a   0
       **   clrx       1  1        0000 0000  0000 000a   0
       ** total: 4 cycles, 4 bytes
       */
      loadRegFromConst (hc08_reg_x, 0);
      rmwWithReg ("lsl", hc08_reg_x);
      rmwWithReg ("rol", hc08_reg_a);
      loadRegFromConst (hc08_reg_a, 0);
      return;

    case 14:
      /*          bytes  cycles     reg x      reg a   carry
       **                          abcd efgh  ijkl mnop   ?
       **   clra       1  1        abcd efgh  0000 0000   a
       **   lslx       1  1        bcde fgh0  0000 0000   a
       **   rola       1  1        bcde fgh0  0000 000a   0
       **   lslx       1  1        cdef gh00  0000 000a   b
       **   rola       1  1        cdef gh00  0000 00ab   0
       **   clrx       1  1        0000 0000  0000 00ab   0
       ** total: 6 cycles, 6 bytes
       */
      loadRegFromConst (hc08_reg_x, 0);
      rmwWithReg ("lsl", hc08_reg_x);
      rmwWithReg ("rol", hc08_reg_a);
      rmwWithReg ("lsl", hc08_reg_x);
      rmwWithReg ("rol", hc08_reg_a);
      loadRegFromConst (hc08_reg_a, 0);
      return;

    case 13:
    case 12:
    case 11:
    case 10:
    case 9:
    case 8:
      transferRegReg (hc08_reg_x, hc08_reg_a, false);
      AccRsh (shCount - 8, false);
      loadRegFromConst (hc08_reg_x, 0);
      return;

    case 7:
      /*          bytes  cycles     reg x      reg a   carry
       **                          abcd efgh  ijkl mnop   ?
       **   lsla       1  1        abcd efgh  jklm nop0   i
       **   txa        1  1        abcd efgh  abcd efgh   i
       **   rola       1  1        abcd efgh  bcde fghi   a
       **   clrx       1  1        0000 0000  bcde fghi   a
       **   rolx       1  1        0000 000a  bcde fghi   0
       ** total: 5 cycles, 5 bytes (beats 14 cycles, 14 bytes)
       */
      rmwWithReg ("lsl", hc08_reg_a);
      transferRegReg (hc08_reg_x, hc08_reg_a, false);
      rmwWithReg ("rol", hc08_reg_a);
      loadRegFromConst (hc08_reg_x, 0);
      rmwWithReg ("rol", hc08_reg_x);
      return;
    case 6:
      /*          bytes  cycles     reg x      reg a   carry
       **                          abcd efgh  ijkl mnop   ?
       **   lsla       1  1        abcd efgh  jklm nop0   i
       **   rolx       1  1        bcde fghi  jklm nop0   a
       **   rola       1  1        bcde fghi  klmn op0a   j
       **   rolx       1  1        cdef ghij  klmn op0a   b
       **   rola       1  1        cdef ghij  lmno p0ab   k
       **   and #3     2  2        cdef ghij  0000 00ab   k
       **   psha       1  2        cdef ghij  0000 00ab   k
       **   txa        1  1        cdef ghij  cdef ghij   k
       **   pula       1  2        0000 00ab  cdef ghij   k
       ** total: 12 cycles, 10 bytes (beats 12 bytes)
       */
    default:
      ;
    }

  /* lsrx/rora is only 2 cycles and bytes, so an unrolled loop is often  */
  /* the fastest and shortest.                                           */
  for (i = 0; i < shCount; i++)
    {
      rmwWithReg ("lsr", hc08_reg_x);
      rmwWithReg ("ror", hc08_reg_a);
    }

}


/*-----------------------------------------------------------------*/
/* shiftL1Left2Result - shift left one byte from left to result    */
/*-----------------------------------------------------------------*/
static void
shiftL1Left2Result (operand *left, int offl, operand *result, int offr, int shCount)
{
  sym_link *resulttype = operandType (result);
  unsigned bytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedbyte = (bytemask != 0xff);

  bool needpulla = pushRegIfSurv (hc08_reg_a);
  loadRegFromAop (hc08_reg_a, AOP (left), offl);
  AccLsh (shCount); // Shift left accumulator.
  if (maskedbyte)
    {
      emitcode ("and", "#0x%02x", bytemask);
      regalloc_dry_run_cost += 2;
    }
  storeRegToAop (hc08_reg_a, AOP (result), offr);
  pullOrFreeReg (hc08_reg_a, needpulla);
}

/*-----------------------------------------------------------------*/
/* movLeft2Result - move byte from left to result                  */
/*-----------------------------------------------------------------*/
static void
movLeft2Result (operand * left, int offl, operand * result, int offr, int sign)
{
  if (!sameRegs (AOP (left), AOP (result)) || (offl != offr))
    {
      transferAopAop (AOP (left), offl, AOP (result), offr);
    }
}


/*-----------------------------------------------------------------*/
/* shiftL2Left2Result - shift left two bytes from left to result   */
/*-----------------------------------------------------------------*/
static void
shiftL2Left2Result (operand *left, int offl, operand *result, int offr, int shCount)
{
  int i;
  bool needpula;
  bool needpulx;

  if (!IS_AOP_XA (AOP (left)) && !IS_AOP_A (AOP (left)))
    needpula = pushRegIfUsed (hc08_reg_a);
  else
    needpula = false;
  if (!IS_AOP_XA (AOP (left)))
    needpulx = pushRegIfUsed (hc08_reg_x);
  else
    needpulx = false;

  loadRegFromAop (hc08_reg_xa, AOP (left), offl);

  switch (shCount)
    {
    case 7:
      rmwWithReg ("lsr", hc08_reg_x);
      rmwWithReg ("ror", hc08_reg_a);
      transferRegReg (hc08_reg_a, hc08_reg_x, false);
      rmwWithReg ("clr", hc08_reg_a);
      rmwWithReg ("ror", hc08_reg_a);
      break;
    default:
      for (i = 0; i < shCount; i++)
        {
          rmwWithReg ("lsl", hc08_reg_a);
          rmwWithReg ("rol", hc08_reg_x);
        }
    }
  storeRegToAop (hc08_reg_xa, AOP (result), offr);

  pullOrFreeReg (hc08_reg_x, needpulx);
  pullOrFreeReg (hc08_reg_a, needpula);
}



/*-----------------------------------------------------------------*/
/* shiftRLeftOrResult - shift right one byte from left,or to result */
/*-----------------------------------------------------------------*/
static void
shiftRLeftOrResult (operand * left, int offl, operand * result, int offr, int shCount)
{
  bool needpula;

  if (!IS_AOP_XA (AOP (left)) && !IS_AOP_A (AOP (left)))
    needpula = pushRegIfUsed (hc08_reg_a);
  else
    needpula = false;

  loadRegFromAop (hc08_reg_a, AOP (left), offl);
  /* shift left accumulator */
  AccRsh (shCount, false);
  /* or with result */
  accopWithAop ("ora", AOP (result), offr);
  /* back to result */
  storeRegToAop (hc08_reg_a, AOP (result), offr);

  pullOrFreeReg (hc08_reg_a, needpula);
}

/*-----------------------------------------------------------------*/
/* genlshOne - left shift a one byte quantity by known count       */
/*-----------------------------------------------------------------*/
static void
genlshOne (operand * result, operand * left, int shCount)
{
  D (emitcode (";     genlshOne", ""));

  shiftL1Left2Result (left, LSB, result, LSB, shCount);
}

/*-----------------------------------------------------------------*/
/* genlshTwo - left shift two bytes by known amount != 0           */
/*-----------------------------------------------------------------*/
static void
genlshTwo (operand *result, operand *left, int shCount)
{
  int size;
  bool needpulla, needpullx;

  sym_link *resulttype = operandType (result);
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  D (emitcode (";     genlshTwo", ""));

  size = getDataSize (result);

  /* if shCount >= 8 */
  if (shCount >= 8)
    {
      shCount -= 8;

      needpulla = pushRegIfSurv (hc08_reg_a);
      if (size > 1)
        {
          loadRegFromAop (hc08_reg_a, AOP (left), 0);
          AccLsh (shCount);
          if (maskedtopbyte)
            {
              emitcode ("and", "#0x%02x", topbytemask);
              regalloc_dry_run_cost += 2;
            }
          storeRegToAop (hc08_reg_a, AOP (result), 1);
        }
      storeConstToAop (0, AOP (result), LSB);
      pullOrFreeReg (hc08_reg_a, needpulla);
    }

  /*  1 <= shCount <= 7 */
  else
    {
      needpulla = pushRegIfSurv (hc08_reg_a);
      needpullx = pushRegIfSurv (hc08_reg_x);
      loadRegFromAop (hc08_reg_xa, AOP (left), 0);
      XAccLsh (shCount);
      if (maskedtopbyte)
        {
          if (topbytemask == 0x7f)
            {
              emitcode ("lslx", "");
              emitcode ("lsrx", "");
              regalloc_dry_run_cost += 2;
            }
          else
            {
              emitcode ("psha", "");
              emitcode ("txa", "");
              emitcode ("and", "#0x%02x", topbytemask);
              emitcode ("tax", "");
              emitcode ("pula", "");
              regalloc_dry_run_cost += 6;
            }
        }
      storeRegToFullAop (hc08_reg_xa, AOP (result), 0);
      pullOrFreeReg (hc08_reg_x, needpullx);
      pullOrFreeReg (hc08_reg_a, needpulla);
    }
}

/*-----------------------------------------------------------------*/
/* shiftLLong - shift left one long from left to result            */
/* offr = LSB or MSB16                                             */
/*-----------------------------------------------------------------*/
static void
shiftLLong (operand * left, operand * result, int offr)
{
//  char *l;
//  int size = AOP_SIZE (result);

  bool needpula = false;
  bool needpulx = false;

  needpula = pushRegIfUsed (hc08_reg_a);
  needpulx = pushRegIfUsed (hc08_reg_x);

  loadRegFromAop (hc08_reg_xa, AOP (left), LSB);
  rmwWithReg ("lsl", hc08_reg_a);
  rmwWithReg ("rol", hc08_reg_x);

  if (offr == LSB)
    {
      storeRegToAop (hc08_reg_xa, AOP (result), offr);
      loadRegFromAop (hc08_reg_xa, AOP (left), MSB24);
      rmwWithReg ("rol", hc08_reg_a);
      rmwWithReg ("rol", hc08_reg_x);
      storeRegToAop (hc08_reg_xa, AOP (result), offr + 2);
    }
  else if (offr == MSB16)
    {
      storeRegToAop (hc08_reg_a, AOP (result), offr);
      loadRegFromAop (hc08_reg_a, AOP (left), MSB24);
      storeRegToAop (hc08_reg_x, AOP (result), offr + 1);
      rmwWithReg ("rol", hc08_reg_a);
      storeRegToAop (hc08_reg_a, AOP (result), offr + 2);
      storeConstToAop (0, AOP (result), 0);
    }

  pullOrFreeReg (hc08_reg_x, needpulx);
  pullOrFreeReg (hc08_reg_a, needpula);
}

/*-----------------------------------------------------------------*/
/* genlshFour - shift four byte by a known amount != 0             */
/*-----------------------------------------------------------------*/
static void
genlshFour (operand * result, operand * left, int shCount)
{
  int size;

  sym_link *resulttype = operandType (result);
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  D (emitcode (";     genlshFour", ""));

  size = AOP_SIZE (result);

  /* TODO: deal with the &result == &left case */

  /* if shifting more that 3 bytes */
  if (shCount >= 24)
    {
      shCount -= 24;
      if (shCount)
        /* lowest order of left goes to the highest
           order of the destination */
        shiftL1Left2Result (left, LSB, result, MSB32, shCount);
      else
        movLeft2Result (left, LSB, result, MSB32, 0);
      storeConstToAop (0, AOP (result), LSB);
      storeConstToAop (0, AOP (result), MSB16);
      storeConstToAop (0, AOP (result), MSB24);
      return;
    }

  /* more than two bytes */
  else if (shCount >= 16)
    {
      /* lower order two bytes goes to higher order two bytes */
      shCount -= 16;
      /* if some more remaining */
      if (shCount)
        shiftL2Left2Result (left, LSB, result, MSB24, shCount);
      else
        {
          movLeft2Result (left, MSB16, result, MSB32, 0);
          movLeft2Result (left, LSB, result, MSB24, 0);
        }
      storeConstToAop (0, AOP (result), LSB);
      storeConstToAop (0, AOP (result), MSB16);
      return;
    }

  /* if more than 1 byte */
  else if (shCount >= 8)
    {
      /* lower order three bytes goes to higher order  three bytes */
      shCount -= 8;
      if (size == 2)
        {
          if (shCount)
            shiftL1Left2Result (left, LSB, result, MSB16, shCount);
          else
            movLeft2Result (left, LSB, result, MSB16, 0);
        }
      else
        {
          /* size = 4 */
          if (shCount == 0)
            {
              movLeft2Result (left, MSB24, result, MSB32, 0);
              movLeft2Result (left, MSB16, result, MSB24, 0);
              movLeft2Result (left, LSB, result, MSB16, 0);
              storeConstToAop (0, AOP (result), LSB);
            }
          else if (shCount == 1)
            shiftLLong (left, result, MSB16);
          else
            {
              shiftL2Left2Result (left, MSB16, result, MSB24, shCount);
              shiftL1Left2Result (left, LSB, result, MSB16, shCount);
              shiftRLeftOrResult (left, LSB, result, MSB24, 8 - shCount);
              storeConstToAop (0, AOP (result), LSB);
            }
        }
    }

  /* 1 <= shCount <= 2 */
  else if (shCount <= 2)
    {
      shiftLLong (left, result, LSB);
      if (shCount == 2)
        shiftLLong (result, result, LSB);
      if (maskedtopbyte)
        maskByte (result, 3, topbytemask);
    }
  /* 3 <= shCount <= 7, optimize */
  else
    {
      shiftL2Left2Result (left, MSB24, result, MSB24, shCount);
      shiftRLeftOrResult (left, MSB16, result, MSB24, 8 - shCount);
      shiftL2Left2Result (left, LSB, result, LSB, shCount);
      if (maskedtopbyte)
        maskByte (result, 3, topbytemask);
    }
}

/*-----------------------------------------------------------------*/
/* genRot1 - generates code for rotation of 8-bit values           */
/*-----------------------------------------------------------------*/
static void
genRot1 (iCode *ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);

  aopOp (left, ic, false);
  aopOp (result, ic, false);

  wassert (bitsForType (operandType (left)) == 8);
  wassert (IS_OP_LITERAL (right));

  int s = operandLitValueUll (right) % 8;

  bool needpulla = pushRegIfSurv (hc08_reg_a);
  loadRegFromAop (hc08_reg_a, left->aop, 0);
  AccRol (s);
  storeRegToAop (hc08_reg_a, result->aop, 0);
  pullOrFreeReg (hc08_reg_a, needpulla);

  freeAsmop (result, NULL, ic, true);
  freeAsmop (left, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genRot - generates code for rotation                            */
/*-----------------------------------------------------------------*/
static void
genRot (iCode *ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  unsigned int lbits = bitsForType (operandType (left));
  if (lbits == 8)
    genRot1 (ic);
  else if (IS_OP_LITERAL (right) && operandLitValueUll (right) % lbits == 1)
    genRLC (ic);
  else if (IS_OP_LITERAL (right) && operandLitValueUll (right) % lbits ==  lbits - 1)
    genRRC (ic);
  else if (IS_OP_LITERAL (right) && operandLitValueUll (right) %lbits == lbits / 2)
    genSwap (ic);
  else
    wassertl (0, "Unsupported rotation.");
}

/*-----------------------------------------------------------------*/
/* genLeftShiftLiteral - left shifting by known count              */
/*-----------------------------------------------------------------*/
static void
genLeftShiftLiteral (operand * left, operand * right, operand * result, iCode * ic)
{
  int shCount = (int) ulFromVal (AOP (right)->aopu.aop_lit);
  int size;

  D (emitcode (";     genLeftShiftLiteral", ""));

  freeAsmop (right, NULL, ic, true);

  aopOp (left, ic, false);
  aopOp (result, ic, false);

//  size = getSize (operandType (result));
  size = AOP_SIZE (result);

#if VIEW_SIZE
  DD (emitcode ("; shift left ", "result %d, left %d", size, AOP_SIZE (left)));
#endif

  if (shCount == 0)
    {
      genCopy (result, left);
    }
  else if (shCount >= (size * 8))
    {
      while (size--)
        storeConstToAop (0, AOP (result), size);
    }
  else
    {
      switch (size)
        {
        case 1:
          genlshOne (result, left, shCount);
          break;

        case 2:
          genlshTwo (result, left, shCount);
          break;

        case 4:
          genlshFour (result, left, shCount);
          break;

        default:
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "*** ack! mystery literal shift!\n");
          fprintf (stderr, "Shift by %d\n", size);
          break;
        }
    }
  freeAsmop (left, NULL, ic, true);
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genLeftShift - generates code for left shifting                 */
/*-----------------------------------------------------------------*/
static void
genLeftShift (iCode *ic)
{
  operand *left, *right, *result;
  int size, offset;
  symbol *tlbl, *tlbl1;
  char *shift;
  asmop *aopResult;
  bool needpullcountreg;
  reg_info *countreg = NULL;

  D (emitcode (";     genLeftShift", ""));

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  sym_link *resulttype = operandType (result);
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  aopOp (right, ic, false);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (AOP_TYPE (right) == AOP_LIT &&
    (getSize (operandType (result)) == 1 || getSize (operandType (result)) == 2 || getSize (operandType (result)) == 4))
    {
      genLeftShiftLiteral (left, right, result, ic);
      return;
    }

  /* shift count is unknown then we have to form
     a loop get the loop count in X : Note: we take
     only the lower order byte since shifting
     more that 32 bits make no sense anyway, ( the
     largest size of an object can be only 32 bits ) */

  aopOp (result, ic, false);
  aopOp (left, ic, false);
  aopResult = AOP (result);

  if (sameRegs (AOP (right), AOP (result)) || regsInCommon (right, result) || IS_AOP_XA (AOP (result)) || isOperandVolatile (result, false))
    aopResult = forceStackedAop (AOP (result), sameRegs (AOP (left), AOP (result)));

  /* now move the left to the result if they are not the
     same */
  if (IS_AOP_HX (aopResult) && !aopResult->stacked)
    loadRegFromAop (hc08_reg_hx, AOP (left), 0);
  else if (IS_AOP_AX (AOP (result)) && IS_AOP_XA (AOP (left)) || IS_AOP_XA (AOP (result)) && IS_AOP_AX (AOP (left)))
    {
      pushReg (hc08_reg_x, true);
      emitcode("tax", "");
      regalloc_dry_run_cost++;
      pullReg (hc08_reg_a);
    }
  else if (!sameRegs (left->aop, aopResult))
    {
      int size = AOP_SIZE (result);
      offset = 0;
      while (size--)
        {
          transferAopAop (AOP (left), offset, aopResult, offset);
          offset++;
        }
    }
  freeAsmop (left, NULL, ic, true);
  AOP (result) = aopResult;

  tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  size = AOP_SIZE (result);
  offset = 0;
  tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

  if (hc08_reg_x->isDead && !IS_AOP_X (AOP (result)) && !IS_AOP_XA (AOP (result)) && !IS_AOP_AX (AOP (result)) && !IS_AOP_HX (AOP (result)))
    countreg = hc08_reg_x;
  else if (hc08_reg_a->isDead && !IS_AOP_A (AOP (result)) && !IS_AOP_XA (AOP (result)) && !IS_AOP_AX (AOP (result)))
    countreg = hc08_reg_a;
  else if (!IS_AOP_X (AOP (result)) && !IS_AOP_XA (AOP (result)) && !IS_AOP_AX (AOP (result)) && !IS_AOP_HX (AOP (result)))
    countreg = hc08_reg_x;
  else if(!IS_AOP_A (AOP (result)) && !IS_AOP_XA (AOP (result)) && !IS_AOP_AX (AOP (result)))
    countreg = hc08_reg_a;
  needpullcountreg = (countreg && pushRegIfSurv (countreg));
  if(countreg)
    {
      countreg->isFree = false;
      loadRegFromAop (countreg, AOP (right), 0);
    }
  else
    {
      pushReg (hc08_reg_a, false);
      pushReg (hc08_reg_a, true);
      loadRegFromAop (hc08_reg_a, AOP (right), 0);
      emitcode ("sta", "2, s");
      regalloc_dry_run_cost += 3;
      pullReg (hc08_reg_a);
    }
  emitcode (countreg == hc08_reg_a ? "tsta" : (countreg ? "tstx" : "tst 1, s"), "");
  regalloc_dry_run_cost += (countreg ? 1 : 3);

  if (right->aop->type != AOP_LIT || !ulFromVal (right->aop->aopu.aop_lit))
    emitBranch ("beq", tlbl1);
  if (!regalloc_dry_run)
    emitLabel (tlbl);

  shift = "lsl";
  for (offset = 0; offset < size; offset++)
    {
      rmwWithAop (shift, AOP (result), offset);
      shift = "rol";
    }
  if (!regalloc_dry_run)
    emitcode (countreg == hc08_reg_a ? "dbnza" : (countreg ? "dbnzx" : "dbnz 1, s"), "%05d$", labelKey2num (tlbl->key));
  regalloc_dry_run_cost += (countreg ? 2 : 4);

  if (!regalloc_dry_run)
    emitLabel (tlbl1);

  // After loop, countreg is 0
  if (countreg)
    {
      countreg->isLitConst = 1;
      countreg->litConst = 0;
    }

  if (!countreg)
    pullNull (1);
  else
    pullOrFreeReg (countreg, needpullcountreg);

  if (maskedtopbyte)
    {
      bool in_a = (result->aop->type == AOP_REG && result->aop->aopu.aop_reg[size - 1]->rIdx == A_IDX);
      bool needpull = false;
      if (!in_a)
        {
          needpull = pushRegIfUsed (hc08_reg_a);
          loadRegFromAop (hc08_reg_a, result->aop, size - 1);
        }
      emitcode ("and", "#0x%02x", topbytemask);
      regalloc_dry_run_cost += 2;
      if (!in_a)
        {
          storeRegToAop (hc08_reg_a, result->aop, size - 1);
          pullOrFreeReg (hc08_reg_a, needpull);
        }
    }

  freeAsmop (result, NULL, ic, true);
  freeAsmop (right, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genrshOne - right shift a one byte quantity by known count      */
/*-----------------------------------------------------------------*/
static void
genrshOne (operand * result, operand * left, int shCount, int sign)
{
  bool needpulla;
  D (emitcode (";     genrshOne", ""));
  needpulla = pushRegIfSurv (hc08_reg_a);
  loadRegFromAop (hc08_reg_a, AOP (left), 0);
  AccRsh (shCount, sign);
  storeRegToFullAop (hc08_reg_a, AOP (result), sign);
  pullOrFreeReg (hc08_reg_a, needpulla);
}

/*-----------------------------------------------------------------*/
/* genrshTwo - right shift two bytes by known amount != 0          */
/*-----------------------------------------------------------------*/
static void
genrshTwo (operand * result, operand * left, int shCount, int sign)
{
  bool needpulla, needpullx;
  D (emitcode (";     genrshTwo", ""));

  /* if shCount >= 8 */
  if (shCount >= 8)
    {
      if (shCount != 8 || sign)
        {
          needpulla = pushRegIfSurv (hc08_reg_a);
          loadRegFromAop (hc08_reg_a, AOP (left), 1);
          AccRsh (shCount - 8, sign);
          storeRegToFullAop (hc08_reg_a, AOP (result), sign);
          pullOrFreeReg (hc08_reg_a, needpulla);
        }
      else
        {
          transferAopAop (AOP (left), 1, AOP (result), 0);
          storeConstToAop (0, AOP (result), 1);
        }
    }

  /*  1 <= shCount <= 7 */
  else
    {
      needpulla = pushRegIfSurv (hc08_reg_a);
      needpullx = pushRegIfSurv (hc08_reg_x);
      loadRegFromAop (hc08_reg_xa, AOP (left), 0);
      XAccRsh (shCount, sign);
      storeRegToAop (hc08_reg_xa, AOP (result), 0);
      pullOrFreeReg (hc08_reg_x, needpullx);
      pullOrFreeReg (hc08_reg_a, needpulla);
    }
}

/*-----------------------------------------------------------------*/
/* shiftRLong - shift right one long from left to result           */
/* offl = LSB or MSB16                                             */
/*-----------------------------------------------------------------*/
static void
shiftRLong (operand * left, int offl, operand * result, int sign)
{
  bool needpula = pushRegIfSurv (hc08_reg_a);
  bool needpulx = pushRegIfSurv (hc08_reg_x);

  if (offl == LSB)
    {
      loadRegFromAop (hc08_reg_xa, AOP (left), MSB24);
      if (sign)
        rmwWithReg ("asr", hc08_reg_x);
      else
        rmwWithReg ("lsr", hc08_reg_x);
      rmwWithReg ("ror", hc08_reg_a);
      storeRegToAop (hc08_reg_xa, AOP (result), MSB24);
      loadRegFromAop (hc08_reg_xa, AOP (left), LSB);
    }
  else if (offl == MSB16)
    {
      loadRegFromAop (hc08_reg_a, AOP (left), MSB32);
      if (sign)
        rmwWithReg ("asr", hc08_reg_a);
      else
        rmwWithReg ("lsr", hc08_reg_a);
      loadRegFromAop (hc08_reg_x, AOP (left), MSB24);
      storeRegToAop (hc08_reg_a, AOP (result), MSB24);
      loadRegFromAop (hc08_reg_a, AOP (left), MSB16);
    }

  rmwWithReg ("ror", hc08_reg_x);
  rmwWithReg ("ror", hc08_reg_a);
  storeRegToAop (hc08_reg_xa, AOP (result), LSB);

  if (offl == MSB16)
    {
      if (sign)
        {
          loadRegFromAop (hc08_reg_a, AOP (left), MSB24);
          storeRegSignToUpperAop (hc08_reg_a, AOP (result), MSB32, sign);
        }
      else
        {
          storeConstToAop (0, AOP (result), MSB32);
        }
    }

  pullOrFreeReg (hc08_reg_x, needpulx);
  pullOrFreeReg (hc08_reg_a, needpula);
}

/*-----------------------------------------------------------------*/
/* genrshFour - shift four byte by a known amount != 0             */
/*-----------------------------------------------------------------*/
static void
genrshFour (operand * result, operand * left, int shCount, int sign)
{
  bool needpulla = false;
  bool needpullx = false;

  /* TODO: handle cases where left == result */

  D (emitcode (";     genrshFour", ""));

  /* if shifting more that 3 bytes */
  if (shCount >= 24)
    {
      needpulla = pushRegIfSurv (hc08_reg_a);
      loadRegFromAop (hc08_reg_a, AOP (left), 3);
      AccRsh (shCount - 24, sign);
      storeRegToFullAop (hc08_reg_a, AOP (result), sign);
    }
  else if (shCount >= 16)
    {
      needpulla = pushRegIfSurv (hc08_reg_a);
      needpullx = pushRegIfSurv (hc08_reg_x);
      loadRegFromAop (hc08_reg_xa, AOP (left), 2);
      XAccRsh (shCount - 16, sign);
      storeRegToFullAop (hc08_reg_xa, AOP (result), sign);
    }
  else if (shCount >= 8)
    {
      if (shCount == 1)
        {
          shiftRLong (left, MSB16, result, sign);
          return;
        }
      else if (shCount == 8)
        {
          needpulla = pushRegIfSurv (hc08_reg_a);
          transferAopAop (AOP (left), 1, AOP (result), 0);
          transferAopAop (AOP (left), 2, AOP (result), 1);
          loadRegFromAop (hc08_reg_a, AOP (left), 3);
          storeRegToAop (hc08_reg_a, AOP (result), 2);
          storeRegSignToUpperAop (hc08_reg_a, AOP (result), 3, sign);
        }
      else if (shCount == 9)
        {
          shiftRLong (left, MSB16, result, sign);
          return;
        }
      else
        {
          needpulla = pushRegIfSurv (hc08_reg_a);
          needpullx = pushRegIfSurv (hc08_reg_x);
          loadRegFromAop (hc08_reg_xa, AOP (left), 1);
          XAccRsh (shCount - 8, false);
          storeRegToAop (hc08_reg_xa, AOP (result), 0);
          loadRegFromAop (hc08_reg_x, AOP (left), 3);
          loadRegFromConst (hc08_reg_a, 0);
          XAccRsh (shCount - 8, sign);
          accopWithAop ("ora", AOP (result), 1);
          storeRegToAop (hc08_reg_xa, AOP (result), 1);
          storeRegSignToUpperAop (hc08_reg_x, AOP (result), 3, sign);
        }
    }
  else
    {
      /* 1 <= shCount <= 7 */
      if (shCount == 1)
        {
          shiftRLong (left, LSB, result, sign);
          return;
        }
      else
        {
          needpulla = pushRegIfSurv (hc08_reg_a);
          needpullx = pushRegIfSurv (hc08_reg_x);
          loadRegFromAop (hc08_reg_xa, AOP (left), 0);
          XAccRsh (shCount, false);
          storeRegToAop (hc08_reg_xa, AOP (result), 0);
          loadRegFromAop (hc08_reg_a, AOP (left), 2);
          AccLsh (8 - shCount);
          accopWithAop ("ora", AOP (result), 1);
          storeRegToAop (hc08_reg_a, AOP (result), 1);
          loadRegFromAop (hc08_reg_xa, AOP (left), 2);
          XAccRsh (shCount, sign);
          storeRegToAop (hc08_reg_xa, AOP (result), 2);
        }
    }
  pullOrFreeReg (hc08_reg_x, needpullx);
  pullOrFreeReg (hc08_reg_a, needpulla);
}

/*-----------------------------------------------------------------*/
/* genRightShiftLiteral - right shifting by known count            */
/*-----------------------------------------------------------------*/
static void
genRightShiftLiteral (operand * left, operand * right, operand * result, iCode * ic, int sign)
{
  int shCount = (int) ulFromVal (AOP (right)->aopu.aop_lit);
  int size;

  D (emitcode (";     genRightShiftLiteral", ""));

  freeAsmop (right, NULL, ic, true);

  aopOp (left, ic, false);
  aopOp (result, ic, false);

#if VIEW_SIZE
  DD (emitcode ("; shift right ", "result %d, left %d", AOP_SIZE (result), AOP_SIZE (left)));
#endif

  size = getDataSize (left);
  /* test the LEFT size !!! */

  /* I suppose that the left size >= result size */
  if (shCount == 0)
    {
      genCopy (result, left);
    }
  else if (shCount >= (size * 8))
    {
      bool needpulla = pushRegIfSurv (hc08_reg_a);
      if (sign)
        {
          
          /* get sign in acc.7 */
          loadRegFromAop (hc08_reg_a, AOP (left), size - 1);
        }
      addSign (result, LSB, sign);
      pullOrFreeReg (hc08_reg_a, needpulla);
    }
  else
    {
      switch (size)
        {
        case 1:
          genrshOne (result, left, shCount, sign);
          break;

        case 2:
          genrshTwo (result, left, shCount, sign);
          break;

        case 4:
          genrshFour (result, left, shCount, sign);
          break;
        default:
          wassertl (0, "Invalid operand size in right shift.");
          break;
        }
    }
  freeAsmop (left, NULL, ic, true);
  freeAsmop (result, NULL, ic, true);
}


/*-----------------------------------------------------------------*/
/* genRightShift - generate code for right shifting                */
/*-----------------------------------------------------------------*/
static void
genRightShift (iCode * ic)
{
  operand *right, *left, *result;
  int size, offset;
  symbol *tlbl, *tlbl1;
  char *shift;
  bool sign;
  asmop *aopResult;
  bool needpullcountreg;
  reg_info *countreg = NULL;

  D (emitcode (";     genRightShift", ""));

  /* signed & unsigned types are treated the same : i.e. the
     signed is NOT propagated inwards : quoting from the
     ANSI - standard : "for E1 >> E2, is equivalent to division
     by 2**E2 if unsigned or if it has a non-negative value,
     otherwise the result is implementation defined ", MY definition
     is that the sign does not get propagated */

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  /* if signed then we do it the hard way preserve the
     sign bit moving it inwards */
  sign = !SPEC_USIGN (getSpec (operandType (left)));

  aopOp (right, ic, false);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (AOP_TYPE (right) == AOP_LIT &&
    (getSize (operandType (result)) == 1 || getSize (operandType (result)) == 2 || getSize (operandType (result)) == 4))
    {
      genRightShiftLiteral (left, right, result, ic, sign);
      return;
    }

  /* shift count is unknown then we have to form
     a loop get the loop count in X : Note: we take
     only the lower order byte since shifting
     more that 32 bits make no sense anyway, ( the
     largest size of an object can be only 32 bits ) */

  aopOp (result, ic, false);
  aopOp (left, ic, false);
  aopResult = AOP (result);

  if (sameRegs (AOP (right), AOP (result)) || regsInCommon (right, result) || IS_AOP_XA (AOP (result)) || isOperandVolatile (result, false))
    aopResult = forceStackedAop (AOP (result), sameRegs (AOP (left), AOP (result)));

  /* now move the left to the result if they are not the
     same */
  if (IS_AOP_HX (aopResult) && !aopResult->stacked)
    loadRegFromAop (hc08_reg_hx, AOP (left), 0);
  else if (IS_AOP_AX (AOP (result)) && IS_AOP_XA (AOP (left)) || IS_AOP_XA (AOP (result)) && IS_AOP_AX (AOP (left)))
    {
      pushReg (hc08_reg_x, true);
      emitcode("tax", "");
      regalloc_dry_run_cost++;
      pullReg (hc08_reg_a);
    }
  else if (!sameRegs (left->aop, aopResult))
    {
      int size = AOP_SIZE (result);
      offset = 0;
      while (size--)
        {
          transferAopAop (AOP (left), offset, aopResult, offset);
          offset++;
        }
    }
  freeAsmop (left, NULL, ic, true);
  AOP (result) = aopResult;

  tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  size = AOP_SIZE (result);
  offset = 0;
  tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

  if (hc08_reg_x->isDead && !IS_AOP_X (AOP (result)) && !IS_AOP_XA (AOP (result)) && !IS_AOP_AX (AOP (result)) && !IS_AOP_HX (AOP (result)))
    countreg = hc08_reg_x;
  else if (hc08_reg_a->isDead && !IS_AOP_A (AOP (result)) && !IS_AOP_XA (AOP (result)) && !IS_AOP_AX (AOP (result)))
    countreg = hc08_reg_a;
  else if (!IS_AOP_X (AOP (result)) && !IS_AOP_XA (AOP (result)) && !IS_AOP_AX (AOP (result)) && !IS_AOP_HX (AOP (result)))
    countreg = hc08_reg_x;
  else if(!IS_AOP_A (AOP (result)) && !IS_AOP_XA (AOP (result)) && !IS_AOP_AX (AOP (result)))
    countreg = hc08_reg_a;
  needpullcountreg = (countreg && pushRegIfSurv (countreg));
  wassert (right->aop); // This can fail is left and right are the same, resulting in a segfault later (bug #3598).
  if(countreg)
    {
      countreg->isFree = false;
      loadRegFromAop (countreg, AOP (right), 0);
    }
  else
    {
      pushReg (hc08_reg_a, false);
      pushReg (hc08_reg_a, true);
      loadRegFromAop (hc08_reg_a, AOP (right), 0);
      emitcode ("sta", "2, s");
      regalloc_dry_run_cost += 3;
      pullReg (hc08_reg_a);
    }
  emitcode (countreg == hc08_reg_a ? "tsta" : (countreg ? "tstx" : "tst 1, s"), "");
  regalloc_dry_run_cost += (countreg ? 1 : 3);

  if (right->aop->type != AOP_LIT || !ulFromVal (right->aop->aopu.aop_lit))
    emitBranch ("beq", tlbl1);
  if (!regalloc_dry_run)
    emitLabel (tlbl);

  shift = sign ? "asr" : "lsr";
  for (offset = size - 1; offset >= 0; offset--)
    {
      rmwWithAop (shift, AOP (result), offset);
      shift = "ror";
    }

  if (!regalloc_dry_run)
    emitcode (countreg == hc08_reg_a ? "dbnza" : (countreg ? "dbnzx" : "dbnz 1, s"), "%05d$", labelKey2num (tlbl->key));
  regalloc_dry_run_cost += (countreg ? 2 : 4);

  if (!regalloc_dry_run)
    emitLabel (tlbl1);

  // After loop, countreg is 0
  if (countreg)
    {
      countreg->isLitConst = 1;
      countreg->litConst = 0;
    }

  if (!countreg)
    pullNull (1);
  else
    pullOrFreeReg (countreg, needpullcountreg);

  freeAsmop (result, NULL, ic, true);
  freeAsmop (right, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* decodePointerOffset - decode a pointer offset operand into a    */
/*                    literal offset and a rematerializable offset */
/*-----------------------------------------------------------------*/
static void
decodePointerOffset (operand * opOffset, int * litOffset, char ** rematOffset)
{
  *litOffset = 0;
  *rematOffset = NULL;

  if (!opOffset)
    return;

  if (IS_OP_LITERAL (opOffset))
    {
      *litOffset = (int)operandLitValue (opOffset);
    }
  else if (IS_ITEMP (opOffset) && OP_SYMBOL (opOffset)->remat)
    {
      asmop * aop = aopForRemat (OP_SYMBOL (opOffset));

      if (aop->type == AOP_LIT)
        *litOffset = (int) floatFromVal (aop->aopu.aop_lit);
      else if (aop->type == AOP_IMMD)
        *rematOffset = aop->aopu.aop_immd;
    }
  else
    wassertl (0, "Pointer get/set with non-constant offset");
}


/*-----------------------------------------------------------------*/
/* genUnpackBits - generates code for unpacking bits               */
/*-----------------------------------------------------------------*/
static void
genUnpackBits (operand * result, operand * left, operand * right, iCode * ifx)
{
  int offset = 0;               /* result byte offset */
  int rsize;                    /* result size */
  int rlen = 0;                 /* remaining bitfield length */
  sym_link *etype;              /* bitfield type information */
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */
  bool needpulla = false;
  bool needpullh = false;
  bool needpullx = false;
  int litOffset = 0;
  char * rematOffset = NULL;

  D (emitcode (";     genUnpackBits", ""));

  decodePointerOffset (right, &litOffset, &rematOffset);
  etype = getSpec (operandType (result));
  rsize = getSize (operandType (result));
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  needpulla = pushRegIfSurv (hc08_reg_a);

  if (!IS_AOP_HX (AOP (left)))
    {
      needpullx = pushRegIfSurv (hc08_reg_x);
      needpullh = pushRegIfSurv (hc08_reg_h);
    }

  /* if the operand is already in hx
     then we do nothing else we move the value to hx */
  loadRegFromAop (hc08_reg_hx, AOP (left), 0);
  /* so hx now contains the address */

  if (ifx && blen <= 8)
    {
      loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
      if (blen < 8)
        {
          emitcode ("and", "#0x%02x", (((unsigned char) - 1) >> (8 - blen)) << bstr);
          regalloc_dry_run_cost += 2;
        }
      pullOrFreeReg (hc08_reg_h, needpullh);
      pullOrFreeReg (hc08_reg_x, needpullx);
      pullOrFreeReg (hc08_reg_a, needpulla);
      genIfxJump (ifx, "a");
      return;
    }
  wassert (!ifx);

  /* If the bitfield length is less than a byte */
  if (blen < 8)
    {
      loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
      AccRsh (bstr, false);
      emitcode ("and", "#0x%02x", ((unsigned char) - 1) >> (8 - blen));
      regalloc_dry_run_cost += 2;
      if (!SPEC_USIGN (etype))
        {
          /* signed bitfield */
          symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

          emitcode ("bit", "#0x%02x", 1 << (blen - 1));
          if (!regalloc_dry_run)
            emitcode ("beq", "%05d$", labelKey2num (tlbl->key));
          emitcode ("ora", "#0x%02x", (unsigned char) (0xff << blen));
          regalloc_dry_run_cost += 6;
          if (!regalloc_dry_run)
            emitLabel (tlbl);
        }
      storeRegToAop (hc08_reg_a, AOP (result), offset++);
      goto finish;
    }

  /* Bit field did not fit in a byte. Copy all
     but the partial byte at the end.  */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
      if (rlen > 8 && AOP_TYPE (result) == AOP_REG)
        pushReg (hc08_reg_a, true);
      else
        storeRegToAop (hc08_reg_a, AOP (result), offset);
      offset++;
      if (rlen > 8)
        {
          litOffset++;
        }
    }

  /* Handle the partial byte at the end */
  if (rlen)
    {
      loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
      emitcode ("and", "#0x%02x", ((unsigned char) - 1) >> (8 - rlen));
      regalloc_dry_run_cost += 3;
      if (!SPEC_USIGN (etype))
        {
          /* signed bitfield */
          symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

          emitcode ("bit", "#0x%02x", 1 << (rlen - 1));
          if (!regalloc_dry_run)
            emitcode ("beq", "%05d$", labelKey2num (tlbl->key));
          emitcode ("ora", "#0x%02x", (unsigned char) (0xff << rlen));
         regalloc_dry_run_cost += 6;
         if (!regalloc_dry_run)
            emitLabel (tlbl);
        }
      storeRegToAop (hc08_reg_a, AOP (result), offset++);
    }
  if (blen > 8 && AOP_TYPE (result) == AOP_REG)
    {
      pullReg (AOP (result)->aopu.aop_reg[0]);
    }

finish:
  if (offset < rsize)
    {
      rsize -= offset;
      if (SPEC_USIGN (etype))
        {
          while (rsize--)
            storeConstToAop (0, AOP (result), offset++);
        }
      else
        {
          /* signed bitfield: sign extension with 0x00 or 0xff */
          emitcode ("rola", "");
          emitcode ("clra", "");
          emitcode ("sbc", zero);
          regalloc_dry_run_cost += 4;

          while (rsize--)
            storeRegToAop (hc08_reg_a, AOP (result), offset++);
        }
    }
  pullOrFreeReg (hc08_reg_h, needpullh);
  pullOrFreeReg (hc08_reg_x, needpullx);
  pullOrFreeReg (hc08_reg_a, needpulla);
}


/*-----------------------------------------------------------------*/
/* genUnpackBitsImmed - generates code for unpacking bits          */
/*-----------------------------------------------------------------*/
static void
genUnpackBitsImmed (operand * left, operand *right, operand * result, iCode * ic, iCode * ifx)
{
  int size;
  int offset = 0;               /* result byte offset */
  int litOffset = 0;
  char * rematOffset = NULL;
  int rsize;                    /* result size */
  int rlen = 0;                 /* remaining bitfield length */
  sym_link *etype;              /* bitfield type information */
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */
  asmop *derefaop;
  bool delayed_a = false;
  bool assigned_a = false;
  bool needpulla = false;

  D (emitcode (";     genUnpackBitsImmed", ""));

  decodePointerOffset (right, &litOffset, &rematOffset);
  wassert (rematOffset==NULL);

  aopOp (result, ic, true);
  size = AOP_SIZE (result);

  derefaop = aopDerefAop (AOP (left), litOffset);
  freeAsmop (left, NULL, ic, true);
  derefaop->size = size;

  etype = getSpec (operandType (result));
  rsize = getSize (operandType (result));
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  needpulla = pushRegIfSurv (hc08_reg_a);

  /* if the bitfield is a single bit in the direct page */
  if (blen == 1 && derefaop->type == AOP_DIR)
    {
      if (!ifx && bstr)
        {
          symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

          loadRegFromConst (hc08_reg_a, 0);
          if (!regalloc_dry_run)
            emitcode ("brclr", "#%d,%s,%05d$", bstr, aopAdrStr (derefaop, 0, false), labelKey2num ((tlbl->key)));
          regalloc_dry_run_cost += 3;
          if (SPEC_USIGN (etype))
            rmwWithReg ("inc", hc08_reg_a);
          else
            rmwWithReg ("dec", hc08_reg_a);
          if (!regalloc_dry_run)
            emitLabel (tlbl);
          storeRegToAop (hc08_reg_a, AOP (result), offset);
          if (AOP_TYPE (result) == AOP_REG && AOP(result)->aopu.aop_reg[offset]->rIdx == A_IDX)
            assigned_a = true;
          hc08_freeReg (hc08_reg_a);
          offset++;
          goto finish;
        }
      else if (ifx)
        {
          symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          symbol *jlbl;
          char *inst;

          if (IC_TRUE (ifx))
            {
              jlbl = IC_TRUE (ifx);
              inst = "brclr";
            }
          else
            {
              jlbl = IC_FALSE (ifx);
              inst = "brset";
            }
          if (!regalloc_dry_run)
            emitcode (inst, "#%d,%s,%05d$", bstr, aopAdrStr (derefaop, 0, false), labelKey2num ((tlbl->key)));
          regalloc_dry_run_cost += 3;
          emitBranch ("jmp", jlbl);
          if (!regalloc_dry_run)
            emitLabel (tlbl);
          ifx->generated = 1;
          offset++;
          goto finish;
        }
    }

  /* If the bitfield length is less than a byte */
  if (blen < 8)
    {
      loadRegFromAop (hc08_reg_a, derefaop, 0);
      if (!ifx)
        {
          AccRsh (bstr, false);
          emitcode ("and", "#0x%02x", ((unsigned char) - 1) >> (8 - blen));
          regalloc_dry_run_cost += 2;
          hc08_dirtyReg (hc08_reg_a, false);
          if (!SPEC_USIGN (etype))
            {
              /* signed bitfield */
              symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

              emitcode ("bit", "#0x%02x", 1 << (blen - 1));
              if (!regalloc_dry_run)
                emitcode ("beq", "%05d$", labelKey2num (tlbl->key));
              emitcode ("ora", "#0x%02x", (unsigned char) (0xff << blen));
              regalloc_dry_run_cost += 6;
              if (!regalloc_dry_run)
                emitLabel (tlbl);
            }
          storeRegToAop (hc08_reg_a, AOP (result), offset);
          if (AOP_TYPE (result) == AOP_REG && AOP(result)->aopu.aop_reg[offset]->rIdx == A_IDX)
            assigned_a = true;
        }
      else
        {
          emitcode ("and", "#0x%02x", (((unsigned char) - 1) >> (8 - blen)) << bstr);
          regalloc_dry_run_cost += 2;
          hc08_dirtyReg (hc08_reg_a, false);
        }
      offset++;
      goto finish;
    }

  /* Bit field did not fit in a byte. Copy all
     but the partial byte at the end.  */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      if (assigned_a && !delayed_a)
        {
          pushReg (hc08_reg_a, true);
          delayed_a = true;
        }
      loadRegFromAop (hc08_reg_a, derefaop, size - offset - 1);
      if (!ifx)
        {
          storeRegToAop (hc08_reg_a, AOP (result), offset);
          if (AOP_TYPE (result) == AOP_REG && AOP(result)->aopu.aop_reg[offset]->rIdx == A_IDX)
            assigned_a = true;
        }
      else
        {
          emitcode ("tsta", "");
          regalloc_dry_run_cost++;
        }
      offset++;
    }

  /* Handle the partial byte at the end */
  if (rlen)
    {
      if (assigned_a && !delayed_a)
        {
          pushReg (hc08_reg_a, true);
          delayed_a = true;
        }
      loadRegFromAop (hc08_reg_a, derefaop, size - offset - 1);
      emitcode ("and", "#0x%02x", ((unsigned char) - 1) >> (8 - rlen));
      regalloc_dry_run_cost += 2;
      if (!SPEC_USIGN (etype))
        {
          /* signed bitfield */
          symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

          emitcode ("bit", "#0x%02x", 1 << (rlen - 1));
          if (!regalloc_dry_run)
            emitcode ("beq", "%05d$", labelKey2num (tlbl->key));
          emitcode ("ora", "#0x%02x", (unsigned char) (0xff << rlen));
          regalloc_dry_run_cost += 6;
          if (!regalloc_dry_run)
            emitLabel (tlbl);
        }
      storeRegToAop (hc08_reg_a, AOP (result), offset);
      if (AOP_TYPE (result) == AOP_REG && AOP(result)->aopu.aop_reg[offset]->rIdx == A_IDX)
        assigned_a = true;
      offset++;
    }

finish:
  if (offset < rsize)
    {
      rsize -= offset;
      if (SPEC_USIGN (etype))
        {
          while (rsize--)
            storeConstToAop (0, AOP (result), offset++);
        }
      else
        {
          if (assigned_a && !delayed_a)
            {
              pushReg (hc08_reg_a, true);
              delayed_a = true;
            }

          /* signed bitfield: sign extension with 0x00 or 0xff */
          emitcode ("rola", "");
          emitcode ("clra", "");
          emitcode ("sbc", zero);
          regalloc_dry_run_cost += 4;

          while (rsize--)
            storeRegToAop (hc08_reg_a, AOP (result), offset++);
        }
    }

  freeAsmop (NULL, derefaop, ic, true);
  freeAsmop (result, NULL, ic, true);

  if (ifx && !ifx->generated)
    {
      genIfxJump (ifx, "a");
    }
  if (delayed_a)
    pullReg (hc08_reg_a);

  pullOrFreeReg (hc08_reg_a, needpulla);
}


/*-----------------------------------------------------------------*/
/* genDataPointerGet - generates code when ptr offset is known     */
/*-----------------------------------------------------------------*/
static void
genDataPointerGet (operand * left, operand * right, operand * result, iCode * ic, iCode * ifx)
{
  int size;
  int litOffset = 0;
  char * rematOffset = NULL;
  asmop *derefaop;
  bool needpulla = false;

  D (emitcode (";     genDataPointerGet", ""));

  decodePointerOffset (right, &litOffset, &rematOffset);
  wassert (rematOffset==NULL);

  aopOp (result, ic, true);
  size = AOP_SIZE (result);

  derefaop = aopDerefAop (AOP (left), litOffset);
  freeAsmop (left, NULL, ic, true);
  derefaop->size = size;
  
  if (ifx)
    needpulla = pushRegIfSurv (hc08_reg_a);

  if (IS_AOP_HX (AOP (result)))
    loadRegFromAop (hc08_reg_hx, derefaop, 0);
  else
    while (size--)
      {
        if (!ifx)
          transferAopAop (derefaop, size, AOP (result), size);
        else
          loadRegFromAop (hc08_reg_a, derefaop, size);
      }

  freeAsmop (NULL, derefaop, ic, true);
  freeAsmop (result, NULL, ic, true);

  pullOrFreeReg (hc08_reg_a, needpulla);
  if (ifx && !ifx->generated)
    {
      genIfxJump (ifx, "a");
    }
}


/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void
genPointerGet (iCode * ic, iCode * pi, iCode * ifx)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);
  int size, offset, xoffset;
  int litOffset = 0;
  char * rematOffset = NULL;
  sym_link *retype = getSpec (operandType (result));
  bool needpulla = false;
  bool needpullh = false;
  bool needpullx = false;
  bool vol = false;

  D (emitcode (";     genPointerGet", ""));

  if ((size = getSize (operandType (result))) > 1)
    ifx = NULL;

  aopOp (left, ic, false);

  /* if left is rematerialisable */
  if (AOP_TYPE (left) == AOP_IMMD || AOP_TYPE (left) == AOP_LIT)
    {
      /* if result is not bit variable type */
      if (!IS_BITVAR (retype))
        {
          genDataPointerGet (left, right, result, ic, ifx);
          return;
        }
      else
        {
          genUnpackBitsImmed (left, right, result, ic, ifx);
          return;
        }
    }

  aopOp (result, ic, false);

  /* if bit then unpack */
  if (IS_BITVAR (retype))
    {
      /* hasInchc08() will be false for bitfields, so no need */
      /* to consider post-increment in this case. */
      genUnpackBits (result, left, right, ifx);
      goto release;
    }

  /* This was marking the result registers dead, but I think */
  /* it should be marking the pointer (left op) registers dead. */
  /* EEP - 5 Jan 2013 */
  if (AOP_TYPE (left) == AOP_REG && pi)
    { 
      int i;
      for (i = 0; i < AOP_SIZE (left); i++)
        AOP (left)->aopu.aop_reg[i]->isDead = true;
    }

  if (!IS_AOP_HX (AOP (left)))
    {
      needpullx = pushRegIfSurv (hc08_reg_x);
      needpullh = pushRegIfSurv (hc08_reg_h);
    }

  /* if the operand is already in hx
     then we do nothing else we move the value to hx */
  loadRegFromAop (hc08_reg_hx, AOP (left), 0);
  /* so hx now contains the address */

  decodePointerOffset (right, &litOffset, &rematOffset);

  if (AOP_TYPE (result) == AOP_REG)
    {
      if (pi)
        aopOp (IC_RESULT (pi), pi, false);

      if (AOP_SIZE (result) == 1)
        {
          if (!pi)
            {
              if (IS_AOP_H (AOP (result)))
                hc08_freeReg (hc08_reg_x);
              loadRegIndexed (AOP (result)->aopu.aop_reg[0], litOffset, rematOffset);
            }
          else
            {
              needpulla = pushRegIfSurv (hc08_reg_a);
              loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
              hc08_useReg (hc08_reg_a);
              emitcode ("aix", "#%d", size);
              regalloc_dry_run_cost += 2;
              hc08_dirtyReg (hc08_reg_hx, false);
              hc08_freeReg (hc08_reg_x);
              storeRegToAop (hc08_reg_hx, AOP (IC_RESULT (pi)), 0);
              pi->generated = 1;
              storeRegToAop (hc08_reg_a, AOP (IC_RESULT (ic)), 0);
            }
        }
      else
        {
          if (pi || (IS_AOP_XA (AOP (IC_RESULT (ic))) && vol))
            {
              loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
              pushReg (hc08_reg_a, false);
              loadRegIndexed (hc08_reg_a, litOffset+1, rematOffset);
              hc08_useReg (hc08_reg_a);
              if (pi)
                {
                  emitcode ("aix", "#%d", size);
                  regalloc_dry_run_cost += 2;
                  hc08_dirtyReg (hc08_reg_hx, false);
                  hc08_freeReg (hc08_reg_x);
                  storeRegToAop (hc08_reg_hx, AOP (IC_RESULT (pi)), 0);
                  pi->generated = 1;
                }
              storeRegToAop (hc08_reg_a, AOP (IC_RESULT (ic)), 0);
              pullReg (AOP (IC_RESULT (ic))->aopu.aop_reg[1]);
            }
          else
            {
              if (IS_AOP_HX (AOP (IC_RESULT (ic))))
                {
                  hc08_freeReg (hc08_reg_x);
                  loadRegIndexed (hc08_reg_hx, litOffset, rematOffset);
                }
              else if (IS_AOP_XA (AOP (IC_RESULT (ic))))
                {
                  loadRegIndexed (hc08_reg_a, litOffset+1, rematOffset);
                  loadRegIndexed (hc08_reg_x, litOffset, rematOffset);
                }
              else if (IS_AOP_AX (AOP (IC_RESULT (ic))))
                {
                  loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
                  loadRegIndexed (hc08_reg_x, litOffset+1, rematOffset);
                }
            }
        }

      if (pi)
        {
          freeAsmop (IC_RESULT (pi), NULL, pi, true);
        }
    }
  else if (!pi && !ifx && size == 2 && IS_S08 && hc08_reg_x->isDead && hc08_reg_h->isDead && AOP_TYPE (result) == AOP_EXT) /* Todo: Use this for bigger sizes, too */
    {
      loadRegIndexed (hc08_reg_hx, litOffset, rematOffset);
      storeRegToAop (hc08_reg_hx, AOP (result), 0);
    }
  else if (!pi && !ifx && size == 1 && hc08_reg_x->isDead && !hc08_reg_a->isDead)
    {
      loadRegIndexed (hc08_reg_x, litOffset, rematOffset);
      storeRegToAop (hc08_reg_x, AOP (result), 0);
    }
  else
    {
      offset = size - 1;
      needpulla = pushRegIfSurv (hc08_reg_a);

      while (size--)
        {
          xoffset = litOffset + (AOP_SIZE (result) - offset - 1);
          loadRegIndexed (hc08_reg_a, xoffset, rematOffset);
          if (!ifx)
            storeRegToAop (hc08_reg_a, AOP (result), offset);
          offset--;
        }
    }

release:
  size = AOP_SIZE (result);
  freeAsmop (left, NULL, ic, true);
  freeAsmop (result, NULL, ic, true);

  if (pi && !pi->generated)
    {
      emitcode ("aix", "#%d", size);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_hx, false);
      aopOp (IC_RESULT (pi), pi, false);
      storeRegToAop (hc08_reg_hx, AOP (IC_RESULT (pi)), 0);
      if (ifx && AOP_TYPE (IC_RESULT (pi)) != AOP_REG)
        {
          /* Ensure Z/NZ flag is correct since storeRegToAop() */
          /* disrupts the flag bits when storing to memory. */
          emitcode ("tsta", "");
          regalloc_dry_run_cost++;
        }
      freeAsmop (IC_RESULT (pi), NULL, pi, true);
      pi->generated = 1;
    }

  pullOrFreeReg (hc08_reg_a, needpulla);
  pullOrFreeReg (hc08_reg_h, needpullh);
  pullOrFreeReg (hc08_reg_x, needpullx);

  if (ifx && !ifx->generated)
    {
      genIfxJump (ifx, "a");
    }
}

/*-----------------------------------------------------------------*/
/* genPackBits - generates code for packed bit storage             */
/*-----------------------------------------------------------------*/
static void
genPackBits (operand * result, operand * left, sym_link * etype, operand * right)
{
  int offset = 0;               /* source byte offset */
  int rlen = 0;                 /* remaining bitfield length */
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */
  int litval;                   /* source literal value (if AOP_LIT) */
  unsigned char mask;           /* bitmask within current byte */
  int litOffset = 0;
  char *rematOffset = NULL;
  bool needpulla;

  D (emitcode (";     genPackBits", ""));

  decodePointerOffset (left, &litOffset, &rematOffset);
  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  needpulla = pushRegIfSurv (hc08_reg_a);
  if (AOP_TYPE (right) == AOP_REG)
    {
      /* Not optimal, but works for any register sources. */
      /* Just push the source values onto the stack and   */
      /* pull them off any needed. Better optimzed would  */
      /* be to do some of the shifting/masking now and    */
      /* push the intermediate result. */
      if (blen > 8)
        pushReg (AOP (right)->aopu.aop_reg[1], true);
      pushReg (AOP (right)->aopu.aop_reg[0], true);
    }
  loadRegFromAop (hc08_reg_hx, AOP (result), 0);

  /* If the bitfield length is less than a byte */
  if (blen < 8)
    {
      mask = ((unsigned char) (0xFF << (blen + bstr)) | (unsigned char) (0xFF >> (8 - bstr)));

      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with a bitfield length <8 and literal source
           */
          litval = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          litval <<= bstr;
          litval &= (~mask) & 0xff;

          loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
          if ((mask | litval) != 0xff)
            {
              emitcode ("and", "#0x%02x", mask);
              regalloc_dry_run_cost += 2;
            }
          if (litval)
            {
              emitcode ("ora", "#0x%02x", litval);
              regalloc_dry_run_cost += 2;
            }
          hc08_dirtyReg (hc08_reg_a, false);
          storeRegIndexed (hc08_reg_a, litOffset, rematOffset);

          pullOrFreeReg (hc08_reg_a, needpulla);
          return;
        }

      /* Case with a bitfield length < 8 and arbitrary source
       */
      if (AOP_TYPE (right) == AOP_REG)
        pullReg (hc08_reg_a);
      else
        loadRegFromAop (hc08_reg_a, AOP (right), 0);
      /* shift and mask source value */
      AccLsh (bstr);
      emitcode ("and", "#0x%02x", (~mask) & 0xff);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_a, false);
      pushReg (hc08_reg_a, true);

      loadRegIndexed (hc08_reg_a, litOffset, rematOffset);
      emitcode ("and", "#0x%02x", mask);
      emitcode ("ora", "1,s");
      regalloc_dry_run_cost += 5;
      storeRegIndexed (hc08_reg_a, litOffset, rematOffset);
      pullReg (hc08_reg_a);

      pullOrFreeReg (hc08_reg_a, needpulla);
      return;
    }

  /* Bit length is greater than 7 bits. In this case, copy  */
  /* all except the partial byte at the end                 */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      if (!litOffset && !rematOffset && AOP (right)->type == AOP_DIR)
        {
          emitcode ("mov", "%s,x+", aopAdrStr (AOP (right), offset, false));
          regalloc_dry_run_cost += 2;
          litOffset--;
        }
      else
        {
          if (AOP_TYPE (right) == AOP_REG)
            pullReg (hc08_reg_a);
          else
            loadRegFromAop (hc08_reg_a, AOP (right), offset);
          storeRegIndexed (hc08_reg_a, litOffset+offset, rematOffset);
        }
      offset++;
    }

  /* If there was a partial byte at the end */
  if (rlen)
    {
      mask = (((unsigned char) - 1 << rlen) & 0xff);

      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with partial byte and literal source
           */
          litval = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          litval >>= (blen - rlen);
          litval &= (~mask) & 0xff;
          loadRegIndexed (hc08_reg_a, litOffset+offset, rematOffset);
          if ((mask | litval) != 0xff)
            {
              emitcode ("and", "#0x%02x", mask);
              regalloc_dry_run_cost += 2;
            }
          if (litval)
            {
              emitcode ("ora", "#0x%02x", litval);
              regalloc_dry_run_cost += 2;
            }
          hc08_dirtyReg (hc08_reg_a, false);
          storeRegIndexed (hc08_reg_a, litOffset+offset, rematOffset);
          pullOrFreeReg (hc08_reg_a, needpulla);
          return;
        }

      /* Case with partial byte and arbitrary source
       */
      if (AOP_TYPE (right) == AOP_REG)
        pullReg (hc08_reg_a);
      else
        loadRegFromAop (hc08_reg_a, AOP (right), offset);
      emitcode ("and", "#0x%02x", (~mask) & 0xff);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_a, false);
      pushReg (hc08_reg_a, true);

      loadRegIndexed(hc08_reg_a, litOffset+offset, rematOffset);
      emitcode ("and", "#0x%02x", mask);
      emitcode ("ora", "1,s");
      regalloc_dry_run_cost += 5;
      storeRegIndexed (hc08_reg_a, litOffset+offset, rematOffset);
      pullReg (hc08_reg_a);
    }

  pullOrFreeReg (hc08_reg_a, needpulla);
}

/*-----------------------------------------------------------------*/
/* genPackBitsImmed - generates code for packed bit storage        */
/*-----------------------------------------------------------------*/
static void
genPackBitsImmed (operand * result, operand * left, sym_link * etype, operand * right, iCode * ic)
{
  asmop *derefaop;
  int size;
  int offset = 0;               /* source byte offset */
  int rlen = 0;                 /* remaining bitfield length */
  unsigned blen;                /* bitfield length */
  unsigned bstr;                /* bitfield starting bit within byte */
  unsigned long long int litval;/* source literal value (if AOP_LIT) */
  unsigned char mask;           /* bitmask within current byte */
  bool needpulla;
  int litOffset = 0;
  char *rematOffset = NULL;

  D (emitcode (";     genPackBitsImmed", ""));

  blen = SPEC_BLEN (etype);
  bstr = SPEC_BSTR (etype);

  aopOp (right, ic, false);
  size = AOP_SIZE (right);
  decodePointerOffset (left, &litOffset, &rematOffset);
  wassert (!rematOffset);

  derefaop = aopDerefAop (AOP (result), litOffset);
  freeAsmop (result, NULL, ic, true);
  derefaop->size = size;

  /* if the bitfield is a single bit in the direct page */
  if (blen == 1 && derefaop->type == AOP_DIR)
    {
      if (AOP_TYPE (right) == AOP_LIT)
        {
          litval = ullFromVal (AOP (right)->aopu.aop_lit);

          emitcode ((litval & 1) ? "bset" : "bclr", "#%d,%s", bstr, aopAdrStr (derefaop, 0, false));
          regalloc_dry_run_cost += 2;
        }
      else
        {
          symbol *tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
          symbol *tlbl2 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

          needpulla = pushRegIfSurv (hc08_reg_a);
          loadRegFromAop (hc08_reg_a, AOP (right), 0);
          emitcode ("lsra", "");
          regalloc_dry_run_cost++;
          emitBranch ("bcs", tlbl1);
          emitcode ("bclr", "#%d,%s", bstr, aopAdrStr (derefaop, 0, false));
          regalloc_dry_run_cost += 2;
          emitBranch ("bra", tlbl2);
          if (!regalloc_dry_run)
            emitLabel (tlbl1);
          emitcode ("bset", "#%d,%s", bstr, aopAdrStr (derefaop, 0, false));
          regalloc_dry_run_cost += 2;
          if (!regalloc_dry_run)
            emitLabel (tlbl2);
          pullOrFreeReg (hc08_reg_a, needpulla);
        }
      goto release;
    }

  /* If the bitfield length is less than a byte */
  if (blen < 8)
    {
      mask = ((unsigned char) (0xFF << (blen + bstr)) | (unsigned char) (0xFF >> (8 - bstr)));

      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with a bitfield length <8 and literal source
           */
          litval = ullFromVal (AOP (right)->aopu.aop_lit);
          litval <<= bstr;
          litval &= (~mask) & 0xff;

          needpulla = pushRegIfSurv (hc08_reg_a);
          loadRegFromAop (hc08_reg_a, derefaop, 0);
          if ((mask | litval) != 0xff)
            {
              emitcode ("and", "#0x%02x", mask);
              regalloc_dry_run_cost += 2;
            }
          if (litval)
            {
              emitcode ("ora", "#0x%02llx", litval);
              regalloc_dry_run_cost += 2;
            }
          hc08_dirtyReg (hc08_reg_a, false);
          storeRegToAop (hc08_reg_a, derefaop, 0);

          pullOrFreeReg (hc08_reg_a, needpulla);
          goto release;
        }

      /* Case with a bitfield length < 8 and arbitrary source
       */
      needpulla = pushRegIfSurv (hc08_reg_a);
      loadRegFromAop (hc08_reg_a, AOP (right), 0);
      /* shift and mask source value */
      AccLsh (bstr);
      emitcode ("and", "#0x%02x", (~mask) & 0xff);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_a, false);
      pushReg (hc08_reg_a, true);

      loadRegFromAop (hc08_reg_a, derefaop, 0);
      emitcode ("and", "#0x%02x", mask);
      emitcode ("ora", "1,s");
      regalloc_dry_run_cost += 5;
      storeRegToAop (hc08_reg_a, derefaop, 0);
      pullReg (hc08_reg_a);

      pullOrFreeReg (hc08_reg_a, needpulla);
      goto release;
    }

  /* Bit length is greater than 7 bits. In this case, copy  */
  /* all except the partial byte at the end                 */
  for (rlen = blen; rlen >= 8; rlen -= 8)
    {
      transferAopAop (AOP (right), offset, derefaop, size - offset - 1);
      offset++;
    }

  /* If there was a partial byte at the end */
  if (rlen)
    {
      mask = (((unsigned char) - 1 << rlen) & 0xff);

      if (AOP_TYPE (right) == AOP_LIT)
        {
          /* Case with partial byte and literal source
           */
          litval = (int) ulFromVal (AOP (right)->aopu.aop_lit);
          litval >>= (blen - rlen);
          litval &= (~mask) & 0xff;
          needpulla = pushRegIfSurv (hc08_reg_a);
          loadRegFromAop (hc08_reg_a, derefaop, size - offset - 1);
          if ((mask | litval) != 0xff)
            {
              emitcode ("and", "#0x%02x", mask);
              regalloc_dry_run_cost += 2;
            }
          if (litval)
            {
              emitcode ("ora", "#0x%02llx", litval);
              regalloc_dry_run_cost += 2;
            }
          hc08_dirtyReg (hc08_reg_a, false);
          storeRegToAop (hc08_reg_a, derefaop, size - offset - 1);
          hc08_dirtyReg (hc08_reg_a, false);
          pullOrFreeReg (hc08_reg_a, needpulla);
          goto release;
        }

      /* Case with partial byte and arbitrary source
       */
      needpulla = pushRegIfSurv (hc08_reg_a);
      loadRegFromAop (hc08_reg_a, AOP (right), offset);
      emitcode ("and", "#0x%02x", (~mask) & 0xff);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_a, false);
      pushReg (hc08_reg_a, true);

      loadRegFromAop (hc08_reg_a, derefaop, size - offset - 1);
      emitcode ("and", "#0x%02x", mask);
      emitcode ("ora", "1,s");
      regalloc_dry_run_cost += 5;
      storeRegToAop (hc08_reg_a, derefaop, size - offset - 1);
      pullReg (hc08_reg_a);
      pullOrFreeReg (hc08_reg_a, needpulla);
    }

  hc08_freeReg (hc08_reg_a);

release:
  freeAsmop (right, NULL, ic, true);
  freeAsmop (NULL, derefaop, ic, true);
}

/*-----------------------------------------------------------------*/
/* genDataPointerSet - remat pointer to data space                 */
/*-----------------------------------------------------------------*/
static void
genDataPointerSet (operand * left, operand * right, operand * result, iCode * ic)
{
  int size;
  asmop *derefaop;
  int litOffset = 0;
  char *rematOffset = NULL;

  D (emitcode (";     genDataPointerSet", ""));

  aopOp (right, ic, false);
  size = AOP_SIZE (right);
  decodePointerOffset (left, &litOffset, &rematOffset);
  wassert (!rematOffset);

  derefaop = aopDerefAop (AOP (result), litOffset);
  freeAsmop (result, NULL, ic, true);
  derefaop->size = size;

  if (IS_AOP_HX (AOP (right)))
    {
      storeRegToAop (hc08_reg_hx, derefaop, 0);
    }
  else
    {
      while (size--)
        {
          transferAopAop (AOP (right), size, derefaop, size);
        }
    }

  freeAsmop (right, NULL, ic, true);
  freeAsmop (NULL, derefaop, ic, true);
}


/*-----------------------------------------------------------------*/
/* genPointerSet - stores the value into a pointer location        */
/*-----------------------------------------------------------------*/
static void
genPointerSet (iCode * ic, iCode * pi)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  operand *result = IC_RESULT (ic);
  int size, offset;
  bool needpulla = false;
  bool needpullx = false;
  bool needpullh = false;
  bool vol = false;
  int litOffset = 0;
  char *rematOffset = NULL;
  wassert (operandType (result)->next);
  bool bit_field = IS_BITVAR (operandType (result)->next);

  D (emitcode (";     genPointerSet", ""));

  aopOp (result, ic, false);

  /* if the result is rematerializable */
  if (AOP_TYPE (result) == AOP_IMMD || AOP_TYPE (result) == AOP_LIT)
    {
      if (!bit_field)
        genDataPointerSet (left, right, result, ic);
      else
        genPackBitsImmed (result, left, operandType (result)->next, right, ic);
      return;
    }
  if (AOP_TYPE (result) == AOP_REG && pi)
    { 
      int i;
      for (i = 0; i < AOP_SIZE (result); i++)
        AOP (result)->aopu.aop_reg[i]->isDead = true;
    }

  needpullx = pushRegIfSurv (hc08_reg_x);
  needpullh = pushRegIfSurv (hc08_reg_h);

  aopOp (right, ic, false);
  size = AOP_SIZE (right);

  /* if bit-field then pack */
  if (bit_field)
    {
      genPackBits (result, left, operandType (result)->next, right);
    }
  else if (AOP_TYPE (right) == AOP_REG)
    {
      decodePointerOffset (left, &litOffset, &rematOffset);
      if (size == 1)
        {
          if (!IS_AOP_A (AOP (right)))
            needpulla = pushRegIfSurv (hc08_reg_a);
          loadRegHXAfromAop(AOP (result), 1, AOP (result), 0, AOP (right), 0 );
          storeRegIndexed (hc08_reg_a, litOffset, rematOffset);
        }
      else if (IS_AOP_XA (AOP (right)) || IS_AOP_HX (AOP (right)))
        {
          if (vol)
            {
              if (AOP (right)->aopu.aop_reg[0]->rIdx != A_IDX)
                needpulla = pushRegIfSurv (hc08_reg_a);
              pushReg (AOP (right)->aopu.aop_reg[0], true);
              pushReg (AOP (right)->aopu.aop_reg[1], true);
              loadRegFromAop (hc08_reg_hx, AOP (result), 0);
              pullReg (hc08_reg_a);
              storeRegIndexed (hc08_reg_a, litOffset, rematOffset);
              pullReg (hc08_reg_a);
              storeRegIndexed (hc08_reg_a, litOffset+1, rematOffset);
            }
          else
            {
              needpulla = pushRegIfSurv (hc08_reg_a);
              loadRegFromAop (hc08_reg_a, AOP (right), 0);
              pushReg (AOP (right)->aopu.aop_reg[1], true);
              loadRegFromAop (hc08_reg_hx, AOP (result), 0);
              storeRegIndexed (hc08_reg_a, litOffset+1, rematOffset);
              pullReg (hc08_reg_a);
              storeRegIndexed (hc08_reg_a, litOffset, rematOffset);
              hc08_freeReg (hc08_reg_a);
            }
        }
      else if (IS_AOP_AX (AOP (right)))
        {
          needpulla = pushRegIfSurv (hc08_reg_a);
          pushReg (hc08_reg_x, true);
          loadRegFromAop (hc08_reg_hx, AOP (result), 0);
          storeRegIndexed (hc08_reg_a, litOffset, rematOffset);
          pullReg (hc08_reg_a); /* original X value */
          storeRegIndexed (hc08_reg_a, litOffset+1, rematOffset);
          hc08_freeReg (hc08_reg_a);
        }
      else
        {
          wassertl (0, "bad source regs in genPointerSet()");
        }
    }
  else
    {
      decodePointerOffset (left, &litOffset, &rematOffset);
      needpulla = pushRegIfSurv (hc08_reg_a);
      loadRegFromAop (hc08_reg_hx, AOP (result), 0);
      offset = size;

      while (offset--)
        {
          loadRegFromAop (hc08_reg_a, AOP (right), offset);
          storeRegIndexed (hc08_reg_a, litOffset + size - offset - 1, rematOffset);
          hc08_freeReg (hc08_reg_a);
        }
    }

  freeAsmop (result, NULL, ic, true);
  freeAsmop (right, NULL, ic, true);

  if (pi)
    {
      aopOp (IC_RESULT (pi), pi, false);
      emitcode ("aix", "#%d", size);
      regalloc_dry_run_cost += 2;
      hc08_dirtyReg (hc08_reg_hx, false);
      storeRegToAop (hc08_reg_hx, AOP (IC_RESULT (pi)), 0);
      freeAsmop (IC_RESULT (pi), NULL, pi, true);
      pi->generated = 1;
    }

  pullOrFreeReg (hc08_reg_a, needpulla);
  pullOrFreeReg (hc08_reg_h, needpullh);
  pullOrFreeReg (hc08_reg_x, needpullx);
}

/*-----------------------------------------------------------------*/
/* genIfx - generate code for Ifx statement                        */
/*-----------------------------------------------------------------*/
static void
genIfx (iCode * ic, iCode * popIc)
{
  operand *cond = IC_COND (ic);
  
  D (emitcode (";     genIfx", ""));

  aopOp (cond, ic, false);

  /* If the condition is a literal, we can just do an unconditional */
  /* branch or no branch */
  if (AOP_TYPE (cond) == AOP_LIT)
    {
      unsigned long long lit = ullFromVal (AOP (cond)->aopu.aop_lit);
      freeAsmop (cond, NULL, ic, true);

      /* if there was something to be popped then do it */
      if (popIc)
        genIpop (popIc);
      if (lit)
        {
          if (IC_TRUE (ic))
            emitBranch ("jmp", IC_TRUE (ic));
        }
      else
        {
          if (IC_FALSE (ic))
            emitBranch ("jmp", IC_FALSE (ic));
        }
      ic->generated = 1;
      return;
    }

  /* evaluate the operand */
  if (AOP_TYPE (cond) != AOP_CRY)
    asmopToBool (AOP (cond), false);
  /* the result is now in the z flag bit */
  freeAsmop (cond, NULL, ic, true);

  /* if there was something to be popped then do it */
  if (popIc)
    genIpop (popIc);

  genIfxJump (ic, "a");

  ic->generated = 1;
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void
genAddrOf (iCode * ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));
  asmop *aopr;
  int size, offset;
  bool needpullx, needpullh;
  struct dbuf_s dbuf;
  
  D (emitcode (";     genAddrOf", ""));

  aopOp (IC_RESULT (ic), ic, false);
  aopr = AOP (IC_RESULT (ic));

  /* if the operand is on the stack then we
     need to get the stack offset of this
     variable */
  if (sym->onStack)
    {
      needpullx = pushRegIfSurv (hc08_reg_x);
      needpullh = pushRegIfSurv (hc08_reg_h);
      /* if it has an offset then we need to compute it */
      offset = _G.stackOfs + _G.stackPushes + sym->stack;
      hc08_useReg (hc08_reg_hx);
      emitcode ("tsx", "");
      hc08_dirtyReg (hc08_reg_hx, false);
      regalloc_dry_run_cost++;
      while (offset > 127)
        {
          emitcode ("aix", "#127");
          regalloc_dry_run_cost += 2;
          offset -= 127;
        }
      while (offset < -128)
        {
          emitcode ("aix", "#-128");
          regalloc_dry_run_cost += 2;
          offset += 128;
        }
      if (offset)
        {
          emitcode ("aix", "#%d", offset);
          regalloc_dry_run_cost += 2;
        }
      storeRegToFullAop (hc08_reg_hx, AOP (IC_RESULT (ic)), false);
      pullOrFreeReg (hc08_reg_h, needpullh);
      pullOrFreeReg (hc08_reg_x, needpullx);
      goto release;
    }

  if (IS_AOP_HX (aopr) || aopr->type == AOP_DIR ||
      (IS_S08 && aopr->type != AOP_REG))
    {
      needpullx = pushRegIfSurv (hc08_reg_x);
      needpullh = pushRegIfSurv (hc08_reg_h);
      loadRegFromImm (hc08_reg_hx, sym->rname);
      storeRegToFullAop (hc08_reg_hx, AOP (IC_RESULT (ic)), false);
      pullOrFreeReg (hc08_reg_h, needpullh);
      pullOrFreeReg (hc08_reg_x, needpullx);
      goto release;
    }
  
  /* object not on stack then we need the name */
  size = AOP_SIZE (IC_RESULT (ic));
  offset = 0;

  while (size--)
    {
      dbuf_init (&dbuf, 64);
      switch (offset)
        {
        case 0:
          dbuf_printf (&dbuf, "#%s", sym->rname);
          break;
        case 1:
          dbuf_printf (&dbuf, "#>%s", sym->rname);
          break;
        default:
          dbuf_printf (&dbuf, "#0");
        }
      storeImmToAop (dbuf_detach_c_str (&dbuf), AOP (IC_RESULT (ic)), offset++);
    }

release:
  freeAsmop (IC_RESULT (ic), NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genAssignLit - Try to generate code for literal assignment.     */
/*                result and right should already be asmOped       */
/*-----------------------------------------------------------------*/
static bool
genAssignLit (operand * result, operand * right)
{
  char assigned[8];
  unsigned char value[sizeof(assigned)];
  char dup[sizeof(assigned)];
  int size;
  int offset,offset2;
  int dups,multiples;
  bool needpula = false;
  bool canUseHX = true;
  int remaining;

  /* Make sure this is a literal assignment */
  if (AOP_TYPE (right) != AOP_LIT)
    return false;

  /* The general case already handles register assignment well */
  if (AOP_TYPE (result) == AOP_REG)
    return false;

  /* Some hardware registers require MSB to LSB assignment order */
  /* so don't optimize the assignment order if volatile */
  if (isOperandVolatile (result, false))
    return false;

  /* Make sure the assignment is not larger than we can handle */
  size = AOP_SIZE (result);
  if (size > sizeof(assigned))
    return false;

  for (offset=0; offset<size; offset++)
    {
      assigned[offset] = 0;
      dup[offset] = 0;
      value[offset] = byteOfVal (AOP (right)->aopu.aop_lit, offset);
    }

  if ((AOP_TYPE (result) != AOP_DIR ) && IS_HC08)
    canUseHX = false;
    
  if (canUseHX)
    {
      /* Assign words that are already in HX */
      for (offset=size-2; offset>=0; offset -= 2)
        {
          if (assigned[offset] || assigned[offset+1])
            continue;
          if (hc08_reg_hx->isLitConst && hc08_reg_hx->litConst == ((value[offset+1] << 8) + value[offset]))
            {
              storeRegToAop (hc08_reg_hx, AOP (result), offset);
              assigned[offset] = 1;
              assigned[offset+1] = 1;
            }
        }
    }

  if (!(hc08_reg_h->isDead && hc08_reg_x->isDead))
    canUseHX = false;

  if (canUseHX && (size>=2))
    {
      /* Assign whatever reamains to be assigned */
      for (offset=size-2; offset>=0; offset -= 2)
        {
          if (assigned[offset] && assigned[offset+1])
            continue;
          loadRegFromConst (hc08_reg_hx, (value[offset+1] << 8) + value[offset]);
          storeRegToAop (hc08_reg_hx, AOP (result), offset);
          assigned[offset] = 1;
          assigned[offset+1] = 1;
        }
    }

  remaining = size;
  for (offset=0; offset<size; offset++)
    remaining -= assigned[offset];
  if (!remaining)
    return true;

  if (remaining > 2)
    aopOpExtToIdx (AOP (result), NULL, NULL);
  
  /* Assign bytes that are already in A and/or X */
  for (offset=size-1; offset>=0; offset--)
    {
      if (assigned[offset])
        continue;
      if ((hc08_reg_a->isLitConst && hc08_reg_a->litConst == value[offset]) ||
          (hc08_reg_x->isLitConst && hc08_reg_x->litConst == value[offset]))
        {
          storeConstToAop (value[offset], AOP (result), offset);
          assigned[offset] = 1;
        }
    }

  /* Consider bytes that appear multiple times */
  multiples = 0;
  for (offset=size-1; offset>=0; offset--)
    {
      if (assigned[offset] || dup[offset])
        continue;
      dups = 0;
      for (offset2=offset-1; offset2>=0; offset2--)
        if (value[offset2] == value[offset])
          {
            dup[offset] = 1;
            dups++;
          }
      if (dups)
        multiples += (dups+1);
    }

  /* Assign bytes that appear multiple times if the register cost */
  /* isn't too high. */
  if (multiples > 2 || (multiples && !hc08_reg_a->isDead))
    {
      needpula = pushRegIfSurv (hc08_reg_a);
      for (offset=size-1; offset>=0; offset--)
        {
          if (assigned[offset])
            continue;
          if (dup[offset])
            {
              loadRegFromConst (hc08_reg_a, value[offset]);
              for (offset2=offset; offset2>=0; offset2--)
                if (!assigned[offset2] && value[offset2] == value[offset])
                  {
                    storeRegToAop (hc08_reg_a, AOP (result), offset2);
                    assigned[offset2] = 1;
                  }
              hc08_freeReg (hc08_reg_a);
            }
        }
    }
    
  /* Assign whatever remains to be assigned */
  for (offset=size-1; offset>=0; offset--)
    {
      if (assigned[offset])
        continue;
      storeConstToAop (value[offset], AOP (result), offset);
    }

  if (needpula)
    pullReg (hc08_reg_a);
  return true;
}


/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void
genAssign (iCode * ic)
{
  operand *result, *right;

  D (emitcode (";     genAssign", ""));

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);
  
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  if (!genAssignLit (result, right))
    genCopy (result, right);

  freeAsmop (right, NULL, ic, true);
  freeAsmop (result, NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genJumpTab - generates code for jump table                       */
/*-----------------------------------------------------------------*/
static void
genJumpTab (iCode * ic)
{
  symbol *jtab;
  symbol *jtablo = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  symbol *jtabhi = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

  D (emitcode (";     genJumpTab", ""));

  aopOp (IC_JTCOND (ic), ic, false);

  if (hc08_reg_x->isFree && hc08_reg_h->isFree)
    {
      /* get the condition into x */
      loadRegFromAop (hc08_reg_x, AOP (IC_JTCOND (ic)), 0);
      freeAsmop (IC_JTCOND (ic), NULL, ic, true);
      loadRegFromConst (hc08_reg_h, 0);

      if (!regalloc_dry_run)
        {
          emitcode ("lda", "%05d$,x", labelKey2num (jtabhi->key));
          emitcode ("ldx", "%05d$,x", labelKey2num (jtablo->key));
        }
      regalloc_dry_run_cost += 6;
      transferRegReg (hc08_reg_a, hc08_reg_h, true);
      emitcode ("jmp", ",x");
      regalloc_dry_run_cost++;

      hc08_dirtyReg (hc08_reg_a, true);
      hc08_dirtyReg (hc08_reg_hx, true);
    }
  else
    {
      adjustStack (-2);
      pushReg (hc08_reg_hx, true);

      /* get the condition into x */
      loadRegFromAop (hc08_reg_x, AOP (IC_JTCOND (ic)), 0);
      freeAsmop (IC_JTCOND (ic), NULL, ic, true);
      loadRegFromConst (hc08_reg_h, 0);

      if (!regalloc_dry_run)
        emitcode ("lda", "%05d$,x", labelKey2num (jtabhi->key));
      emitcode ("sta", "3,s");
      if (!regalloc_dry_run)
        emitcode ("lda", "%05d$,x", labelKey2num (jtablo->key));
      emitcode ("sta", "4,s");
      regalloc_dry_run_cost += 12;

      pullReg (hc08_reg_hx);
      emitcode ("rts", "");
      regalloc_dry_run_cost++;
      _G.stackPushes -= 2;
      updateCFA ();
    }

  /* now generate the jump labels */
  if (!regalloc_dry_run)
    emitLabel (jtablo);
  for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
    {
      emitcode (".db", "%05d$", labelKey2num (jtab->key));
      regalloc_dry_run_cost++;
    }
  if (!regalloc_dry_run)
    emitLabel (jtabhi);
  for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
    {
      emitcode (".db", ">%05d$", labelKey2num (jtab->key));
      regalloc_dry_run_cost++;
    }
}

/*-----------------------------------------------------------------*/
/* genCast - gen code for casting                                  */
/*-----------------------------------------------------------------*/
static void
genCast (iCode * ic)
{
  operand *result = IC_RESULT (ic);
  operand *right = IC_RIGHT (ic);
  sym_link *resulttype = operandType (result);
  sym_link *righttype = operandType (right);
  int size, offset;
  bool signExtend;
  bool save_a;

  D (emitcode (";     genCast", ""));

  /* if they are equivalent then do nothing */
  if (operandsEqu (IC_RESULT (ic), IC_RIGHT (ic)))
    return;

  unsigned topbytemask = (IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;

  aopOp (right, ic, false);
  aopOp (result, ic, false);

  // Cast to _BitInt can require mask of top byte.
  if (IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8) && bitsForType (resulttype) < bitsForType (righttype))
    {
      save_a = false;
      genCopy (result, right);
      if (result->aop->type != AOP_REG || result->aop->aopu.aop_reg[result->aop->size - 1] != hc08_reg_a)
        {
          save_a = result->aop->aopu.aop_reg[0] == hc08_reg_a || !hc08_reg_a->isDead;
          if (save_a)
            pushReg(hc08_reg_a, false);
          loadRegFromAop (hc08_reg_a, result->aop, result->aop->size - 1);
        }
      emitcode ("and", "#0x%02x", topbytemask);
      regalloc_dry_run_cost += 2;
      if (!SPEC_USIGN (resulttype)) // Sign-extend
        {
          symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
          emitcode ("bit", "#0x%02x", 1u << (SPEC_BITINTWIDTH (resulttype) % 8 - 1));
          if (!regalloc_dry_run)
            emitcode ("beq", "!tlabel", labelKey2num (tlbl->key));
          emitcode ("ora", "#0x%02x", ~topbytemask & 0xff);
          regalloc_dry_run_cost += 6;
          emitLabel (tlbl);
        }
      storeRegToAop (hc08_reg_a, result->aop, result->aop->size - 1);
      if (save_a)
        pullReg (hc08_reg_a);
      goto release;
    }

  if (IS_BOOL (resulttype))
    {
      bool needpulla = pushRegIfSurv (hc08_reg_a);
      asmopToBool (AOP (right), true);
      storeRegToAop (hc08_reg_a, AOP (result), 0);
      pullOrFreeReg (hc08_reg_a, needpulla);
      goto release;
    }

  /* If the result is 1 byte, then just copy the one byte; there is */
  /* nothing special required. */
  if (result->aop->size <= right->aop->size)
    {
      wassert (!IS_BITINT (resulttype) || !(SPEC_BITINTWIDTH (resulttype) % 8));
      genCopy (result, right);
      goto release;
    }

  signExtend = AOP_SIZE (result) > AOP_SIZE (right) && !IS_BOOL (righttype) && IS_SPEC (righttype) && !SPEC_USIGN (righttype);
  bool masktopbyte = IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8) && SPEC_USIGN (resulttype);

  /* If the result is 2 bytes and in registers, we have to be careful */
  /* to make sure the registers are not overwritten prematurely. */
  if (AOP_SIZE (result) == 2 && AOP (result)->type == AOP_REG)
    {
      if (IS_AOP_HX (AOP (result)) && AOP_SIZE (right) == 2 && !IS_BITINT (resulttype))
        {
          loadRegFromAop (hc08_reg_hx, AOP (right), 0);
          goto release;
        }

      if (AOP_SIZE (right) == 1)
        {
          transferAopAop (AOP (right), 0, AOP (result), 0);
          if (!signExtend)
            storeConstToAop (0, AOP (result), 1);
          else
            {
              save_a = (AOP (result)->aopu.aop_reg[0] == hc08_reg_a || !hc08_reg_a->isDead);

              /* we need to extend the sign :{ */
              if (save_a)
                pushReg(hc08_reg_a, false);
              if (AOP (result)->aopu.aop_reg[0] != hc08_reg_a)
                loadRegFromAop (hc08_reg_a, AOP (right), 0);
              accopWithMisc ("rola", "");
              accopWithMisc ("clra", "");
              accopWithMisc ("sbc", zero);
              regalloc_dry_run_cost += 4;
              if (masktopbyte)
                {
                  emitcode ("and", "#0x%02x", topbytemask);
                  regalloc_dry_run_cost++;
                }
              storeRegToAop (hc08_reg_a, AOP (result), 1);
              if (save_a)
                pullReg(hc08_reg_a);
            }
          goto release;
        }

      wassert (0); // Should have been covered by logic earlier in this function,
    }

  wassert (AOP (result)->type != AOP_REG);

  save_a = !hc08_reg_a->isDead && (signExtend || topbytemask != 0xff);
  if (save_a)
    pushReg(hc08_reg_a, true);

  offset = 0;
  size = AOP_SIZE (right);
  if (AOP_SIZE (result) < size)
    size = AOP_SIZE (result);
  while (size)
    {
      if (size == 1 && signExtend)
        {
          loadRegFromAop (hc08_reg_a, AOP (right), offset);
          storeRegToAop (hc08_reg_a, AOP (result), offset);
          offset++;
          size--;
        }
      else if ((size > 2 || size >= 2 && !signExtend) && hc08_reg_h->isDead && hc08_reg_x->isDead && 
        (AOP_TYPE (right) == AOP_IMMD || IS_S08 && AOP_TYPE (right) == AOP_EXT) &&
        (AOP_TYPE (result) == AOP_DIR || IS_S08 && AOP_TYPE (result) == AOP_EXT))
        {
          loadRegFromAop (hc08_reg_hx, AOP (right), offset);
          storeRegToAop (hc08_reg_hx, AOP (result), offset);
          offset += 2;
          size -= 2;
        }
      else
        {
          transferAopAop (AOP (right), offset, AOP (result), offset);
          offset++;
          size--;
        }
    }

  size = AOP_SIZE (result) - offset;
  if (size && !signExtend)
    {
      while (size--)
        storeConstToAop (0, AOP (result), offset++);
    }
  else if (size)
    {
      accopWithMisc ("rola", "");
      accopWithMisc ("clra", "");
      accopWithMisc ("sbc", zero);
      regalloc_dry_run_cost += 4;
      while (size--)
        {
          if (!size && masktopbyte)
            {
              emitcode ("and", "#0x%02x", topbytemask);
              regalloc_dry_run_cost += 2;
            }
          storeRegToAop (hc08_reg_a, AOP (result), offset++);
        }
    }

  if (save_a)
    pullReg(hc08_reg_a);

  /* we are done hurray !!!! */

release:
  freeAsmop (right, NULL, ic, true);
  freeAsmop (result, NULL, ic, true);

}

/*-----------------------------------------------------------------*/
/* genDjnz - generate decrement & jump if not zero instruction      */
/*-----------------------------------------------------------------*/
static int
genDjnz (iCode * ic, iCode * ifx)
{
  if (!ifx)
    return 0;

  D (emitcode (";     genDjnz", ""));

  /* if the minus is not of the form
     a = a - 1 */
  if (!isOperandEqual (IC_RESULT (ic), IC_LEFT (ic)) || !IS_OP_LITERAL (IC_RIGHT (ic)))
    return 0;

  if (operandLitValue (IC_RIGHT (ic)) != 1)
    return 0;

  /* if the size of this greater than one then no
     saving, unless it's already in HX  */
  aopOp (IC_RESULT (ic), ic, false);
  if (AOP_SIZE (IC_RESULT (ic)) > 1 && !IS_AOP_HX (AOP (IC_RESULT (ic))))
    {
      freeAsmop (IC_RESULT (ic), NULL, ic, true);
      return 0;
    }

  /* Trying to use dbnz directly requires some convoluted branch/jumps */
  /* to handle the cases where the target is far away. The peepholer   */
  /* is left to clean it up for the simple cases. However, this leaves */
  /* a needlessly high dry run cost. So we do not use dbnz and instead */
  /* generate simpler (and less constly) code that the peepholer can   */
  /* easily transform to dbnz if the target is close enough. Thus the  */
  /* register allocator gets a better idea of the true cost. */
  if (IS_AOP_HX (AOP (IC_RESULT (ic))))
    {
      emitcode ("aix", "#-1");
      hc08_dirtyReg (hc08_reg_hx, false);
      emitcode ("cphx", "#0");
      regalloc_dry_run_cost += 5;
    }
  else
    rmwWithAop ("dec", AOP (IC_RESULT (ic)), 0);
  genIfxJump (ifx, "a");
  freeAsmop (IC_RESULT (ic), NULL, ic, true);
  return 1;
}

/*-----------------------------------------------------------------*/
/* genReceive - generate code for a receive iCode                  */
/*-----------------------------------------------------------------*/
static void
genReceive (iCode * ic)
{
  int size;
  int offset;
  bool delayed_x = false;

  D (emitcode (";", "genReceive"));

  aopOp (IC_RESULT (ic), ic, false);
  size = AOP_SIZE (IC_RESULT (ic));
  offset = 0;

  if (ic->argreg && IS_AOP_HX (AOP (IC_RESULT (ic))) && (offset + (ic->argreg - 1)) == 0)
    {
      pushReg (hc08_reg_x, true);
      emitcode ("tax", "");
      regalloc_dry_run_cost++;
      pullReg (hc08_reg_h);
    }
  else if (ic->argreg)
    {
      while (size--)
        {
          if (AOP_TYPE (IC_RESULT (ic)) == AOP_REG && !(offset + (ic->argreg - 1)) && AOP (IC_RESULT (ic))->aopu.aop_reg[0]->rIdx == X_IDX && size)
            {
              pushReg (hc08_reg_a, true);
              delayed_x = true;
            }
          else
            transferAopAop (hc08_aop_pass[offset + (ic->argreg - 1)], 0, AOP (IC_RESULT (ic)), offset);
          if (hc08_aop_pass[offset]->type == AOP_REG)
            hc08_freeReg (hc08_aop_pass[offset]->aopu.aop_reg[0]);
          offset++;
        }
    }

  if (delayed_x)
    pullReg (hc08_reg_x);

  freeAsmop (IC_RESULT (ic), NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genDummyRead - generate code for dummy read of volatiles        */
/*-----------------------------------------------------------------*/
static void
genDummyRead (iCode * ic)
{
  operand *op;
  int size, offset;
  bool needpulla;

  D (emitcode (";     genDummyRead", ""));

  op = IC_RIGHT (ic);

  needpulla = pushRegIfSurv (hc08_reg_a);
  if (op && IS_SYMOP (op))
    {

      aopOp (op, ic, false);

      size = AOP_SIZE (op);
      offset = size - 1;

      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (op), offset);
          hc08_freeReg (hc08_reg_a);
          offset--;
        }

      freeAsmop (op, NULL, ic, true);
    }
  op = IC_LEFT (ic);
  if (op && IS_SYMOP (op))
    {

      aopOp (op, ic, false);

      size = AOP_SIZE (op);
      offset = size - 1;

      while (size--)
        {
          loadRegFromAop (hc08_reg_a, AOP (op), offset);
          hc08_freeReg (hc08_reg_a);
          offset--;
        }

      freeAsmop (op, NULL, ic, true);
    }
  pullOrFreeReg (hc08_reg_a, needpulla);
}

/*-----------------------------------------------------------------*/
/* genCritical - generate code for start of a critical sequence    */
/*-----------------------------------------------------------------*/
static void
genCritical (iCode * ic)
{
  D (emitcode (";     genCritical", ""));

  if (IC_RESULT (ic))
    aopOp (IC_RESULT (ic), ic, true);

  emitcode ("tpa", "");
  regalloc_dry_run_cost++;
  hc08_dirtyReg (hc08_reg_a, false);
  emitcode ("sei", "");
  regalloc_dry_run_cost++;

  if (IC_RESULT (ic))
    storeRegToAop (hc08_reg_a, AOP (IC_RESULT (ic)), 0);
  else
    pushReg (hc08_reg_a, false);

  hc08_freeReg (hc08_reg_a);
  if (IC_RESULT (ic))
    freeAsmop (IC_RESULT (ic), NULL, ic, true);
}

/*-----------------------------------------------------------------*/
/* genEndCritical - generate code for end of a critical sequence   */
/*-----------------------------------------------------------------*/
static void
genEndCritical (iCode * ic)
{
  D (emitcode (";     genEndCritical", ""));

  if (IC_RIGHT (ic))
    {
      aopOp (IC_RIGHT (ic), ic, false);
      loadRegFromAop (hc08_reg_a, AOP (IC_RIGHT (ic)), 0);
      emitcode ("tap", "");
      regalloc_dry_run_cost++;
      hc08_freeReg (hc08_reg_a);
      freeAsmop (IC_RIGHT (ic), NULL, ic, true);
    }
  else
    {
      pullReg (hc08_reg_a);
      emitcode ("tap", "");
      regalloc_dry_run_cost++;
    }
}

static void
updateiTempRegisterUse (operand * op)
{
  symbol *sym;

  if (IS_ITEMP (op))
    {
      sym = OP_SYMBOL (op);
      if (!sym->isspilt)
        {
	  /* If only used by IFX, there might not be any register assigned */
          int i;
          for(i = 0; i < sym->nRegs; i++)
            if (sym->regs[i])
              hc08_useReg (sym->regs[i]);
        }
    }
}

/*---------------------------------------------------------------------------------------*/
/* genhc08iode - generate code for HC08 based controllers for a single iCode instruction */
/*---------------------------------------------------------------------------------------*/
static void
genhc08iCode (iCode *ic)
{
  /* if the result is marked as
     spilt and rematerializable or code for
     this has already been generated then
     do nothing */
  if (resultRemat (ic) || ic->generated)
    return;

  {
    int i;
    reg_info *reg;

    initGenLineElement ();
    genLine.lineElement.ic = ic;

    for (i = A_IDX; i <= XA_IDX; i++)
      {
        reg = hc08_regWithIdx (i);
        //if (reg->aop)
        //  emitcode ("", "; %s = %s offset %d", reg->name, aopName (reg->aop), reg->aopofs);
        reg->isFree = true;
        if (regalloc_dry_run)
          reg->isLitConst = 0;
      }

    if (ic->op == IFX)
      updateiTempRegisterUse (IC_COND (ic));
    else if (ic->op == JUMPTABLE)
      updateiTempRegisterUse (IC_JTCOND (ic));
    else if (ic->op == RECEIVE)
      {
        hc08_useReg (hc08_reg_a);
        hc08_useReg (hc08_reg_x); // TODO: x really is free if function only receives 1 byte
      }
    else
      {
        if (POINTER_SET (ic))
          updateiTempRegisterUse (IC_RESULT (ic));
        updateiTempRegisterUse (IC_LEFT (ic));
        updateiTempRegisterUse (IC_RIGHT (ic));
      }

    for (i = A_IDX; i <= H_IDX; i++)
      {
        if (bitVectBitValue (ic->rSurv, i))
          {
            hc08_regWithIdx (i)->isDead = false;
            hc08_regWithIdx (i)->isFree = false;
          }
        else
          hc08_regWithIdx (i)->isDead = true;
      }
  }

  /* depending on the operation */
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

    case IPUSH_VALUE_AT_ADDRESS:
          genPointerPush (ic);
          break;
  
    case IPOP:
      /* IPOP happens only when trying to restore a
         spilt live range, if there is an ifx statement
         following this pop then the if statement might
         be using some of the registers being popped which
         would destroy the contents of the register so
         we need to check for this condition and handle it */
      if (ic->next && ic->next->op == IFX && regsInCommon (IC_LEFT (ic), IC_COND (ic->next)))
        genIfx (ic->next, ic);
      else
        genIpop (ic);
      break;

    case CALL:
      genCall (ic);
      break;

    case PCALL:
      genPcall (ic);
      break;

    case FUNCTION:
      genFunction (ic);
      break;

    case ENDFUNCTION:
      genEndFunction (ic);
      break;

    case RETURN:
      genRet (ic);
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
      if (!genDjnz (ic, ifxForOp (IC_RESULT (ic), ic)))
        genMinus (ic);
      break;

    case '*':
      genMult (ic);
      break;

    case '/':
      genDiv (ic);
      break;

    case '%':
      genMod (ic);
      break;

    case '>':
    case '<':
    case LE_OP:
    case GE_OP:
      genCmp (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case NE_OP:
    case EQ_OP:
      genCmpEQorNE (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case AND_OP:
      genAndOp (ic);
      break;

    case OR_OP:
      genOrOp (ic);
      break;

    case '^':
      genXor (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case '|':
      genOr (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case BITWISEAND:
      genAnd (ic, ifxForOp (IC_RESULT (ic), ic));
      break;

    case INLINEASM:
      hc08_genInline (ic);
      break;

    case GETABIT:
      genGetAbit (ic);
      break;

    case GETBYTE:
      genGetByte (ic);
      break;

    case GETWORD:
      genGetWord (ic);
      break;

    case ROT:
      genRot (ic);
      break;

    case LEFT_OP:
      genLeftShift (ic);
      break;

    case RIGHT_OP:
      genRightShift (ic);
      break;

    case GET_VALUE_AT_ADDRESS:
      genPointerGet (ic, hasInchc08 (IC_LEFT (ic), ic, getSize (operandType (IC_RESULT (ic)))), ifxForOp (IC_RESULT (ic), ic));
      break;

    case '=':
      if (POINTER_SET (ic))
        genPointerSet (ic, hasInchc08 (IC_RESULT (ic), ic, getSize (operandType (IC_RIGHT (ic)))));
      else
        genAssign (ic);
      break;

    case IFX:
      genIfx (ic, NULL);
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
      genReceive (ic);
      break;

    case SEND:
      if (!regalloc_dry_run)
        addSet (&_G.sendSet, ic);
      else
        {
          set * sendSet = NULL;
          addSet (&sendSet, ic);
          genSend (sendSet);
          deleteSet (&sendSet);
        }
      break;

    case DUMMY_READ_VOLATILE:
      genDummyRead (ic);
      break;

    case CRITICAL:
      genCritical (ic);
      break;

    case ENDCRITICAL:
      genEndCritical (ic);
      break;

    default:
      wassertl (0, "Unknown iCode");
      fprintf (stderr, "ic->op: %d\n", ic->op);
    }
}

static void
init_aop_pass(void)
{
  if (hc08_aop_pass[0])
    return;

  hc08_aop_pass[0] = newAsmop (AOP_REG);
  hc08_aop_pass[0]->size = 1;
  hc08_aop_pass[0]->aopu.aop_reg[0] = hc08_reg_a;
  hc08_aop_pass[1] = newAsmop (AOP_REG);
  hc08_aop_pass[1]->size = 1;
  hc08_aop_pass[1]->aopu.aop_reg[0] = hc08_reg_x;
  hc08_aop_pass[2] = newAsmop (AOP_DIR);
  hc08_aop_pass[2]->size = 1;
  hc08_aop_pass[2]->aopu.aop_dir = "___SDCC_hc08_ret2";
  hc08_aop_pass[3] = newAsmop (AOP_DIR);
  hc08_aop_pass[3]->size = 1;
  hc08_aop_pass[3]->aopu.aop_dir = "___SDCC_hc08_ret3";
  hc08_aop_pass[4] = newAsmop (AOP_DIR);
  hc08_aop_pass[4]->size = 1;
  hc08_aop_pass[4]->aopu.aop_dir = "___SDCC_hc08_ret4";
  hc08_aop_pass[5] = newAsmop (AOP_DIR);
  hc08_aop_pass[5]->size = 1;
  hc08_aop_pass[5]->aopu.aop_dir = "___SDCC_hc08_ret5";
  hc08_aop_pass[6] = newAsmop (AOP_DIR);
  hc08_aop_pass[6]->size = 1;
  hc08_aop_pass[6]->aopu.aop_dir = "___SDCC_hc08_ret6";
  hc08_aop_pass[7] = newAsmop (AOP_DIR);
  hc08_aop_pass[7]->size = 1;
  hc08_aop_pass[7]->aopu.aop_dir = "___SDCC_hc08_ret7";
}

float
dryhc08iCode (iCode *ic)
{
  regalloc_dry_run = true;
  regalloc_dry_run_cost = 0;

  init_aop_pass();
  
  genhc08iCode (ic);

  destroy_line_list ();
  /*freeTrace (&_G.trace.aops);*/

  return (regalloc_dry_run_cost);
}

/*-----------------------------------------------------------------*/
/* genhc08Code - generate code for HC08 based controllers          */
/*-----------------------------------------------------------------*/
void
genhc08Code (iCode *lic)
{
  iCode *ic;
  int cln = 0;
  int clevel = 0;
  int cblock = 0;

  regalloc_dry_run = false;

  hc08_dirtyReg (hc08_reg_a, false);
  hc08_dirtyReg (hc08_reg_h, false);
  hc08_dirtyReg (hc08_reg_x, false);

  /* print the allocation information */
  if (allocInfo && currFunc)
    printAllocInfo (currFunc, codeOutBuf);
  /* if debug information required */
  if (options.debug && currFunc && !regalloc_dry_run)
    {
      debugFile->writeFunction (currFunc, lic);
    }

  if (options.debug && !regalloc_dry_run)
    debugFile->writeFrameAddress (NULL, NULL, 0); /* have no idea where frame is now */

  init_aop_pass();

  for (ic = lic; ic; ic = ic->next)
    ic->generated = false;

  for (ic = lic; ic; ic = ic->next)
    {
      initGenLineElement ();

      genLine.lineElement.ic = ic;

      if (ic->level != clevel || ic->block != cblock)
        {
          if (options.debug)
            {
              debugFile->writeScope (ic);
            }
          clevel = ic->level;
          cblock = ic->block;
        }

      if (ic->lineno && cln != ic->lineno)
        {
          if (options.debug)
            {
              debugFile->writeCLine (ic);
            }
          if (!options.noCcodeInAsm)
            {
              emitcode ("", ";%s:%d: %s", ic->filename, ic->lineno, printCLine (ic->filename, ic->lineno));
            }
          cln = ic->lineno;
        }
      if (options.iCodeInAsm)
        {
          char regsSurv[4];
          const char *iLine;

          regsSurv[0] = (bitVectBitValue (ic->rSurv, A_IDX)) ? 'a' : '-';
          regsSurv[1] = (bitVectBitValue (ic->rSurv, H_IDX)) ? 'h' : '-';
          regsSurv[2] = (bitVectBitValue (ic->rSurv, X_IDX)) ? 'x' : '-';
          regsSurv[3] = 0;
          iLine = printILine (ic);
          emitcode ("", "; [%s] ic:%d: %s", regsSurv, ic->key, iLine);
          dbuf_free (iLine);
        }

      regalloc_dry_run_cost = 0;
      genhc08iCode(ic);
      //if (options.verboseAsm)
      //  emitcode (";", "iCode %d (key %d) total cost: %d\n", ic->seq, ic->key, (int) regalloc_dry_run_cost);

      if (!hc08_reg_a->isFree)
        DD (emitcode ("", "; forgot to free a"));
      if (!hc08_reg_x->isFree)
        DD (emitcode ("", "; forgot to free x"));
      if (!hc08_reg_h->isFree)
        DD (emitcode ("", "; forgot to free h"));
      if (!hc08_reg_hx->isFree)
        DD (emitcode ("", "; forgot to free hx"));
      if (!hc08_reg_xa->isFree)
        DD (emitcode ("", "; forgot to free xa"));
    }

  if (options.debug)
    debugFile->writeFrameAddress (NULL, NULL, 0); /* have no idea where frame is now */


  /* now we are ready to call the
     peep hole optimizer */
  if (!options.nopeep)
    peepHole (&genLine.lineHead);

  /* now do the actual printing */
  printLine (genLine.lineHead, codeOutBuf);

  /* destroy the line list */
  destroy_line_list ();
}

