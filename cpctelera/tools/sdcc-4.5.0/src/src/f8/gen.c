/*-------------------------------------------------------------------------
  gen.c - code generator for F8.

  Copyright (C) 2021-2024, Philipp Klaus Krause krauseph@informatik.uni-freiburg.de)

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

#define UNIMPLEMENTED do {wassertl (regalloc_dry_run, "Unimplemented"); cost (150, 150);} while(0)

static bool regalloc_dry_run;
static unsigned int regalloc_dry_run_cost_bytes;
static float regalloc_dry_run_cost_cycles;
static float regalloc_dry_run_cycle_scale = 1.0f;

static struct
{
  short debugLine;
  struct
    {
      int pushed;
      int size;
      int param_offset;
    } stack;
  bool saved; // Saved the caller-save registers at call.
  int c; // Track state of carry bit. -1 for unknown.
}
G;

enum asminst
{
  A_ADC,
  A_ADCW,
  A_ADD,
  A_ADDW,
  A_AND,
  A_BOOL,
  A_BOOLW,
  A_CLR,
  A_CLRW,
  A_CP,
  A_CPW,
  A_DEC,
  A_DECW,
  A_INC,
  A_INCW,
  A_LD,
  A_LDW,
  A_MUL,
  A_NEGW,
  A_OR,
  A_ORW,
  A_RLC,
  A_RLCW,
  A_ROT,
  A_RRC,
  A_RRCW,
  A_SBC,
  A_SBCW,
  A_SLL,
  A_SLLW,
  A_SEX,
  A_SRA,
  A_SRAW,
  A_SRL,
  A_SRLW,
  A_SUB,
  A_SUBW,
  A_TST,
  A_TSTW,
  A_XCH,
  A_XOR,
  A_XORW,
};

static const char *asminstnames[] =
{
  "adc",
  "adcw",
  "add",
  "addw",
  "and",
  "bool",
  "boolw",
  "clr",
  "clrw",
  "cp",
  "cpw",
  "dec",
  "decw",
  "inc",
  "incw",
  "ld",
  "ldw",
  "mul",
  "negw",
  "or",
  "orw",
  "rlc",
  "rlcw",
  "rot",
  "rrc",
  "rrcw",
  "sbc",
  "sbcw",
  "sll",
  "sllw",
  "sex",
  "sra",
  "sraw",
  "srl",
  "srlw",
  "sub",
  "subw",
  "tst",
  "tstw",
  "xch",
  "xor",
  "xorw",
};

bool f8_regs_used_as_parms_in_calls_from_current_function[ZH_IDX + 1];
bool f8_regs_used_as_parms_in_pcalls_from_current_function[ZH_IDX + 1];

static struct asmop asmop_xl, asmop_xh, asmop_x, asmop_y, asmop_z, asmop_xy, asmop_xly, asmop_yx, asmop_zero, asmop_one, asmop_mone, asmop_f;
static struct asmop *const ASMOP_XL = &asmop_xl;
static struct asmop *const ASMOP_XH = &asmop_xh;
static struct asmop *const ASMOP_X = &asmop_x;
static struct asmop *const ASMOP_Y = &asmop_y;
static struct asmop *const ASMOP_Z = &asmop_z;
static struct asmop *const ASMOP_XY = &asmop_xy;
static struct asmop *const ASMOP_XLY = &asmop_xly;
static struct asmop *const ASMOP_YX = &asmop_yx;
static struct asmop *const ASMOP_ZERO = &asmop_zero;
static struct asmop *const ASMOP_ONE = &asmop_one;
static struct asmop *const ASMOP_MONE = &asmop_mone;
static struct asmop *const ASMOP_F = &asmop_f;

// Init aop as a an asmop for data in registers, as given by the -1-terminated array regidx.
static void
f8_init_reg_asmop(asmop *aop, const signed char *regidx)
{
  aop->type = AOP_REGSTK;
  aop->size = 0;
  memset (aop->regs, -1, sizeof(aop->regs));
  
  for(int i = 0; regidx[i] >= 0; i++)
    {
      aop->aopu.bytes[i].byteu.reg = f8_regs + regidx[i];
      aop->regs[regidx[i]] = i;
      aop->aopu.bytes[i].in_reg = true;
      aop->size++;
    }

  aop->valinfo.anything = true;
}

void
f8_init_asmops (void)
{
  f8_init_reg_asmop(&asmop_xl, (const signed char[]){XL_IDX, -1});
  f8_init_reg_asmop(&asmop_xh, (const signed char[]){XH_IDX, -1});
  f8_init_reg_asmop(&asmop_x, (const signed char[]){XL_IDX, XH_IDX, -1});
  f8_init_reg_asmop(&asmop_y, (const signed char[]){YL_IDX, YH_IDX, -1});
  f8_init_reg_asmop(&asmop_z, (const signed char[]){ZL_IDX, ZH_IDX, -1});
  f8_init_reg_asmop(&asmop_xy, (const signed char[]){YL_IDX, YH_IDX, XL_IDX, XH_IDX, -1});
  f8_init_reg_asmop(&asmop_yx, (const signed char[]){XL_IDX, XH_IDX, YL_IDX, YH_IDX, -1});
  f8_init_reg_asmop(&asmop_xly, (const signed char[]){YL_IDX, YH_IDX, XL_IDX, -1});

  asmop_zero.type = AOP_LIT;
  asmop_zero.size = 1;
  memset (asmop_zero.regs, -1, sizeof(asmop_zero.regs));
  asmop_zero.aopu.aop_lit = constVal ("0");
  asmop_zero.valinfo.anything = false;
  asmop_zero.valinfo.nothing = true;
  asmop_zero.valinfo.min = 0;
  asmop_zero.valinfo.max = 0;
  asmop_zero.valinfo.knownbitsmask = 0x0000000000000000ull;
  asmop_zero.valinfo.knownbits = 0;

  asmop_one.type = AOP_LIT;
  asmop_one.size = 1;
  memset (asmop_one.regs, -1, sizeof(asmop_one.regs));
  asmop_one.aopu.aop_lit = constVal ("1");
  asmop_one.valinfo.anything = true;

  asmop_mone.type = AOP_LIT;
  asmop_mone.size = 8; // Maximum size for asmop.
  memset (asmop_one.regs, -1, sizeof(asmop_mone.regs));
  asmop_mone.aopu.aop_lit = constVal ("-1");
  asmop_mone.valinfo.anything = true;

  f8_init_reg_asmop(&asmop_f, (const signed char[]){F_IDX, -1});
}

static void
cost(unsigned int bytes, float cycles)
{
  regalloc_dry_run_cost_bytes += bytes;
  regalloc_dry_run_cost_cycles += cycles * regalloc_dry_run_cycle_scale;
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
emitJP(const symbol *target, float probability)
{
  if (!regalloc_dry_run)
     emit2 ("jp", "#!tlabel", labelKey2num (target->key));
  cost (3, probability);
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
      if (aop->size <= offset && !b)
        continue;

      // Information from generalized constant propagation analysis
      if (!aop->valinfo.anything && (offset * 8 < sizeof (aop->valinfo.knownbitsmask) * CHAR_BIT) &&
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

/*-----------------------------------------------------------------*/
/* aopRS - asmop in register or on stack                           */
/*-----------------------------------------------------------------*/
static bool
aopRS (const asmop *aop)
{
  return (aop->type == AOP_REGSTK || aop->type == AOP_STK);
}

/*-----------------------------------------------------------------*/
/* aopInReg - asmop from offset in the register                    */
/*-----------------------------------------------------------------*/
static bool
aopInReg (const asmop *aop, int offset, short rIdx)
{
  if (aop->type != AOP_REGSTK)
    return (false);

  if (offset >= aop->size || offset < 0)
    return (false);

  if (rIdx == X_IDX)
    return (aopInReg (aop, offset, XL_IDX) && aopInReg (aop, offset + 1, XH_IDX));
  else if (rIdx == Y_IDX)
    return (aopInReg (aop, offset, YL_IDX) && aopInReg (aop, offset + 1, YH_IDX));
  else if (rIdx == Z_IDX)
    return (aopInReg (aop, offset, ZL_IDX) && aopInReg (aop, offset + 1, ZH_IDX));

  return (aop->aopu.bytes[offset].in_reg && aop->aopu.bytes[offset].byteu.reg->rIdx == rIdx);
}

/*-----------------------------------------------------------------*/
/* aopOnStack - asmop from offset on stack in consecutive memory   */
/*-----------------------------------------------------------------*/
static bool
aopOnStack (const asmop *aop, int offset, int size)
{
  int i;
  long int stk_base;

  if (size <= 0)
    return (true);

  if (!(aop->type == AOP_STK || aop->type == AOP_REGSTK))
    return (false);

  if (offset + size > aop->size)
    return (false);

  // Fully on stack?
  for (i = offset; i < offset + size; i++)
    if (i < 8 && aop->aopu.bytes[i].in_reg)
      return (false);

  // Consecutive?
  stk_base = aop->aopu.bytes[offset].byteu.stk;
  for (i = 1; i < size && i < 8; i++)
    if (!regalloc_dry_run && aop->aopu.bytes[offset + i].byteu.stk != stk_base + i) // Todo: Stack offsets might be unavailable during dry run (messes with addition costs, so we should have a mechanism to do it better).
      return (false);

  return (true);
}

/*-----------------------------------------------------------------*/
/* aopOnStack - asmop from offset on stack (excl. extended stack)  */
/*-----------------------------------------------------------------*/
static bool
aopOnStackNotExt (const asmop *aop, int offset, int size)
{
  if (size <= 0)
    return (true);
  return (aopOnStack (aop, offset, size) && (!size || aop->aopu.bytes[offset].byteu.stk + G.stack.pushed <= 255 || regalloc_dry_run)); // Todo: Stack offsets might be unavailable during dry run (messes with addition costs, so we should have a mechanism to do it better).
}

/*-----------------------------------------------------------------*/
/* aopSame - are two asmops in the same location?                  */
/*-----------------------------------------------------------------*/
static bool
aopSame (const asmop *aop1, int offset1, const asmop *aop2, int offset2, int size)
{
  for(; size; size--, offset1++, offset2++)
    {
      if (offset1 >= aop1->size || offset2 >= aop2->size)
        return (false);

      if (aopRS (aop1) && aopRS (aop2) && // Same register
        aop1->aopu.bytes[offset1].in_reg && aop2->aopu.bytes[offset2].in_reg &&
        aop1->aopu.bytes[offset1].byteu.reg == aop2->aopu.bytes[offset2].byteu.reg)
        continue;

      if (aopOnStack (aop1, offset1, 1) && aopOnStack (aop2, offset2, 1) && // Same stack location
        aop1->aopu.bytes[offset1].byteu.stk == aop2->aopu.bytes[offset2].byteu.stk)
        continue;

      if (aop1->type == AOP_LIT && aop2->type == AOP_LIT &&
        byteOfVal (aop1->aopu.aop_lit, offset1) == byteOfVal (aop2->aopu.aop_lit, offset2))
        continue;

      if (aop1->type == AOP_DIR && aop2->type == AOP_DIR &&
        offset1 == offset2 && !strcmp(aop1->aopu.aop_dir, aop2->aopu.aop_dir))
        return (true);

      return (false);
    }

  return (true);
}

/*-----------------------------------------------------------------*/
/* aopIsAcc8 - asmop at offset can be used as left 8-bit operand 
              to two-operand instruction                           */
/*-----------------------------------------------------------------*/
static bool
aopIsAcc8 (const asmop *aop, int offset)
{
  return (aopInReg (aop, offset, XL_IDX) || aopInReg (aop, offset, XH_IDX) ||
    aopInReg (aop, offset, YL_IDX) || aopInReg (aop, offset, YH_IDX) ||
    aopInReg (aop, offset, ZL_IDX) || aopInReg (aop, offset, ZH_IDX));
}

/*-----------------------------------------------------------------*/
/* aopIsOp8_2 - asmop at offset can be used as right 8-bit operand 
                to two-operand instruction                         */
/*-----------------------------------------------------------------*/
static bool
aopIsOp8_2 (const asmop *aop, int offset)
{
  return (aop->type == AOP_LIT || aop->type == AOP_DIR || aop->type == AOP_IMMD || offset >= aop->size ||
    aopOnStack (aop, offset, 1) ||
    aopInReg (aop, offset, XH_IDX) || aopInReg (aop, offset, YL_IDX) || aopInReg (aop, offset, YH_IDX) || aopInReg (aop, offset, ZL_IDX));
}

/*-----------------------------------------------------------------*/
/* aopIsOp8_1 - asmop at offset can be used at 8-bit operand to
                one-operand instruction                            */
/*-----------------------------------------------------------------*/
static bool
aopIsOp8_1 (const asmop *aop, int offset)
{
  return (aop->type == AOP_DIR ||
    aopOnStackNotExt (aop, offset, 1) ||
    aopRS (aop) && aop->aopu.bytes[offset].in_reg);
}

/*-----------------------------------------------------------------*/
/* aopAreOp8_2 - asmops at offsets can be used as 8-bit operand to
                 two-operand instruction                          */
/*-----------------------------------------------------------------*/
static bool
aopAre8_2 (const asmop *aop0, int offset0, const asmop *aop1, int offset1)
{
  return (aopIsAcc8 (aop0, offset0) && aopIsOp8_2 (aop1, offset1) ||
    aopInReg (aop1, offset1, XL_IDX) && aopIsOp8_2 (aop0, offset0) && aop0->type != AOP_LIT && aop0->type != AOP_IMMD);
}

/*-----------------------------------------------------------------*/
/* aopIsAcc16 - asmop at offset can be used as left 16-bit operand 
                to two-operand instruction                         */
/*-----------------------------------------------------------------*/
static bool
aopIsAcc16 (const asmop *aop, int offset)
{
  return (aopInReg (aop, offset, Y_IDX) || aopInReg (aop, offset, X_IDX) || aopInReg (aop, offset, Z_IDX));
}

/*-----------------------------------------------------------------*/
/* aopIsOp16_2 - asmop at offset can be used as 16-bit operand to
                 two-operand instruction                           */
/*-----------------------------------------------------------------*/
static bool
aopIsOp16_2 (const asmop *aop, int offset)
{
  return (aop->type == AOP_LIT || aop->type == AOP_IMMD || offset >= aop->size || aop->type == AOP_DIR && offset + 1 < aop->size ||
    aopOnStackNotExt (aop, offset, 2) ||
    aopInReg (aop, offset, X_IDX));
}

/*-----------------------------------------------------------------*/
/* aopIsOp16_1 - asmop at offset can be used at 16-bit operand to
                 one-operand instruction                           */
/*-----------------------------------------------------------------*/
static bool
aopIsOp16_1 (const asmop *aop, int offset)
{
  return (aop->type == AOP_DIR && offset + 1 < aop->size ||
    aopOnStack (aop, offset, 2) ||
    aopInReg (aop, offset, Y_IDX) || aopInReg (aop, offset, X_IDX) || aopInReg (aop, offset, Z_IDX));
}

/*-----------------------------------------------------------------*/
/* aopAreOp16_2 - asmops at offsets can be used as 16-bit operand to
                  two-operand instruction                          */
/*-----------------------------------------------------------------*/
static bool
aopAre16_2 (const asmop *aop0, int offset0, const asmop *aop1, int offset1)
{
  return (aopIsAcc16 (aop0, offset0) && (aopIsOp16_2 (aop1, offset1) || aopIsAcc16 (aop1, offset1)) &&
    !(aopInReg (aop0, offset0, Z_IDX) && aopInReg (aop1, offset1, X_IDX)) ||
    aopInReg (aop1, offset1, Y_IDX) && aopIsOp16_2 (aop0, offset0) && aop0->type != AOP_LIT && aop0->type != AOP_DIR);
}

/*-----------------------------------------------------------------*/
/* aopAreOpExt - asmops at offsets can be used as operands to
                 zeor/sign extension instruction                   */
/*-----------------------------------------------------------------*/
static bool
aopAreExt (const asmop *aop0, int offset0, const asmop *aop1, int offset1)
{
  return (
    aopInReg (aop0, offset0, Y_IDX) && (aopInReg (aop1, offset1, XL_IDX) || aopInReg (aop1, offset1, XH_IDX) || aopInReg (aop1, offset1, ZH_IDX)) ||
    aopInReg (aop0, offset0, X_IDX) && aopInReg (aop1, offset1, ZL_IDX) ||
    aopInReg (aop0, offset0, Z_IDX) && (aopInReg (aop1, offset1, YL_IDX) || aopInReg (aop1, offset1, YH_IDX)));
}

// Get aop at offset as 8-bit operand.
static const char *
aopGet(const asmop *aop, int offset)
{
  static char buffer[256];

  /* Don't really need the value during dry runs, so save some time. */
  if (regalloc_dry_run)
    return ("");

  if (aop->type == AOP_LIT)
    {
      SNPRINTF (buffer, sizeof(buffer), "#0x%02x", byteOfVal (aop->aopu.aop_lit, offset));
      return (buffer);
    }

  if (offset >= aop->size)
    return ("#0x00");

  if (aopRS (aop) && offset < 8 && aop->aopu.bytes[offset].in_reg)
    return (aop->aopu.bytes[offset].byteu.reg->name);

  if (aopRS (aop) && (offset >= 8 || !aop->aopu.bytes[offset].in_reg))
    {
      long int soffset;
      if (offset < 8)
        soffset = aop->aopu.bytes[offset].byteu.stk + G.stack.pushed;
      else
        soffset = aop->aopu.bytes[0].byteu.stk + offset + G.stack.pushed;

      wassert (soffset < (1 << 16) && soffset >= 0);

      if (soffset > 255)
        {
          long int eoffset;
          if (offset < 8)
            eoffset = (long int)(aop->aopu.bytes[offset].byteu.stk);
          else
            eoffset = (long int)(aop->aopu.bytes[0].byteu.stk) + offset;

          wassertl_bt (regalloc_dry_run || f8_extend_stack, "Extended stack access, but z not setup as frame pointer.");
          wassertl_bt (regalloc_dry_run || eoffset >= -G.stack.size && eoffset <= 0xffffl - G.stack.size, "Stack access out of extended stack range."); // Stack > 64K.

          SNPRINTF (buffer, sizeof(buffer), "(%u, z)", (unsigned)eoffset);
        }
      else
        SNPRINTF (buffer, sizeof(buffer), "(%u, sp)", (unsigned)soffset);
      return (buffer);
    }

  if (aop->type == AOP_IMMD)
    {
      if (offset == 0)
        SNPRINTF (buffer, sizeof(buffer), "#<(%s+%d)", aop->aopu.immd, aop->aopu.immd_off);
      else
        SNPRINTF (buffer, sizeof(buffer), "#((%s+%d) >> %d)", aop->aopu.immd, aop->aopu.immd_off, offset * 8);
      return (buffer);
    }

  if (aop->type == AOP_DIR)
    {
      SNPRINTF (buffer, sizeof(buffer), "%s+%d", aop->aopu.aop_dir, offset);
      return (buffer);
    }

  wassert_bt (0);
  return ("dummy");
}

// Get aop at offset as 16-bit operand.
static const char *
aopGet2(const asmop *aop, int offset)
{
  static char buffer[256];

  /* Don't really need the value during dry runs, so save some time. */
  if (regalloc_dry_run)
    return ("");

  if (aopInReg (aop, offset, X_IDX))
    return("x");
  if (aopInReg (aop, offset, Y_IDX))
    return("y");
  if (aopInReg (aop, offset, Z_IDX))
    return("z");

  if (aop->type != AOP_LIT && !aopOnStack (aop, offset, 2) && aop->type != AOP_IMMD && aop->type != AOP_DIR)
    fprintf (stderr, "Invalid aop for aopGet2. aop->type %d. aop->size %d.\n", aop->type, aop->size);
  wassert_bt (aop->type == AOP_LIT || aopOnStack (aop, offset, 2) || aop->type == AOP_IMMD || aop->type == AOP_DIR);

  if (aop->type == AOP_LIT)
    {
      SNPRINTF (buffer, sizeof(buffer), "#0x%02x%02x", byteOfVal (aop->aopu.aop_lit, offset + 1), byteOfVal (aop->aopu.aop_lit, offset));
      return (buffer);
    }
  else if (aop->type == AOP_IMMD)
    {
      if (offset)
        SNPRINTF (buffer, sizeof(buffer), "#((%s+%d) >> %d)", aop->aopu.immd, aop->aopu.immd_off, offset * 8);
      else
        SNPRINTF (buffer, sizeof(buffer), "#(%s+%d)", aop->aopu.immd, aop->aopu.immd_off);
      return (buffer);
    }

  return (aopGet (aop, offset));
}

// How many bytes would it take for the two-operand 8-bit instruction (including prefixes)? Also return the number of prefix bytes in *prefix.
// returns -1 if impossible.
static int
op2_bytes (int *prefixes, const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  int r0Idx = ((aopRS (op0) && op0->aopu.bytes[offset0].in_reg)) ? op0->aopu.bytes[offset0].byteu.reg->rIdx : -1;
  int r1Idx = ((aopRS (op1) && op1->aopu.bytes[offset1].in_reg)) ? op1->aopu.bytes[offset1].byteu.reg->rIdx : -1;

  *prefixes = 0;

  if (r0Idx == XL_IDX)
    if (r1Idx == XH_IDX || r1Idx == YL_IDX || r1Idx == YH_IDX || r1Idx == ZL_IDX)
      return 1;
    else if (op1->type == AOP_LIT || op1->type == AOP_IMMD ||
      offset1 >= op1->size ||
      op1->type == AOP_STK || op1->type == AOP_REGSTK && r1Idx == -1)
      return 2;
    else if (op1->type == AOP_DIR)
      return 3;
    else
      return -1;

  if (r0Idx == XH_IDX || r0Idx == YL_IDX || r0Idx == YH_IDX || r0Idx == ZL_IDX || r0Idx == ZH_IDX) // Try with alternate accumulator prefix.
    {
      int bytes = op2_bytes (prefixes, ASMOP_XL, 0, op1, offset1);
      if (bytes >= 0 && *prefixes == 0)
        {
          (*prefixes)++;
          return bytes + 1;
        }
    }
  if (r1Idx == XL_IDX || r1Idx == XH_IDX || r1Idx == YL_IDX || r1Idx == ZL_IDX) // Try with swap prefix
    {
      int bytes = op2_bytes (prefixes, op1, offset1, op0, offset0);
      if (bytes >= 0 && *prefixes == 0)
        {
          (*prefixes)++;
          return bytes + 1;
        }
    }

  return -1;
}

static void
op2_cost (const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  int prefixes;
  int bytes = op2_bytes (&prefixes, op0, offset0, op1, offset1);

  wassert_bt (bytes > 0);

  cost (bytes, prefixes + 1);
}

static void
op_cost (const asmop *op0, int offset0)
{
  if (op0->type == AOP_DIR)
    cost (3, 1);
  else if (aopOnStack (op0, offset0, 1))
    cost (2, 1);
  else if (aopInReg (op0, offset0, XL_IDX))
    cost (1, 1);
  else if (aopIsAcc8 (op0, offset0))
    cost (2, 2);
  else
    wassert (0);
}

// How many bytes would it take for the two-operand 16-bit instruction (including prefixes)? Also return the number of prefix bytes in *prefix.
// returns -1 if impossible.
static int
op2w_bytes (int *prefixes, const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  int r0Idx = ((aopRS (op0) && op0->aopu.bytes[offset0].in_reg)) ? op0->aopu.bytes[offset0].byteu.reg->rIdx : -1;
  int r1Idx = ((aopRS (op1) && op1->aopu.bytes[offset1].in_reg)) ? op1->aopu.bytes[offset1].byteu.reg->rIdx : -1;

  *prefixes = 0;

  if (r0Idx >= 0)
    {
      if (!op0->aopu.bytes[offset0 + 1].in_reg || op0->aopu.bytes[offset0 + 1].byteu.reg->rIdx != r0Idx + 1)
        return -1;
      switch (r0Idx)
        {
        case XL_IDX:
          r0Idx = X_IDX;
          break;
        case YL_IDX:
          r0Idx = Y_IDX;
          break;
        case ZL_IDX:
          r0Idx = Z_IDX;
          break;
        default:
          return -1;
        }
    }
  if (r1Idx >= 0)
    {
      if (!op1->aopu.bytes[offset1 + 1].in_reg || op1->aopu.bytes[offset1 + 1].byteu.reg->rIdx != r1Idx + 1)
        return -1;
      switch (r1Idx)
        {
        case XL_IDX:
          r1Idx = X_IDX;
          break;
        case YL_IDX:
          r1Idx = Y_IDX;
          break;
        case ZL_IDX:
          r1Idx = Z_IDX;
          break;
        default:
          return -1;
        }
    }

  if (r0Idx == Y_IDX)
    {
      if (r1Idx == X_IDX)
        return 1;
      else if (r1Idx == Z_IDX)
        {
          *prefixes = 1;
          return 2;
        }
      else if (op1->type == AOP_STK || op1->type == AOP_REGSTK && r1Idx == -1)
        return 2;
      else if (op1->type == AOP_LIT || op1->type == AOP_IMMD ||
        offset1 >= op1->size ||  op1->type == AOP_DIR)
        return 3;
      else
        return -1;
    }
  else if (r0Idx == Z_IDX && r1Idx == Y_IDX || r0Idx == X_IDX && r1Idx == Z_IDX)
    {
      *prefixes = 1;
      return 2;
    }

  if (r0Idx == X_IDX || r0Idx == Z_IDX) // Try with alternate accumulator prefix.
    {
      int bytes = op2w_bytes (prefixes, ASMOP_Y, 0, op1, offset1);
      if (bytes >= 0 && *prefixes == 0)
        {
          (*prefixes)++;
          return bytes + 1;
        }
    }    
  if (r1Idx == Y_IDX) // Try with swap prefix
    {
      int bytes = op2w_bytes (prefixes, op1, offset1, op0, offset0);
      if (bytes >= 0 && *prefixes == 0)
        {
          (*prefixes)++;
          return bytes + 1;
        }
    }

  return -1;
}

static void
op2w_cost (const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  int prefixes;
  int bytes = op2w_bytes (&prefixes, op0, offset0, op1, offset1);

  wassert_bt (bytes > 0);

  cost (bytes, prefixes + 1);
}

// How many bytes would it take for the two-operand 8-bit load (including prefixes)? Also return the number of prefix bytes in *prefix.
// returns -1 if impossible.
static int
ld_bytes (int *prefixes, const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  int r0Idx = ((aopRS (op0) && offset0 < 8 && op0->aopu.bytes[offset0].in_reg)) ? op0->aopu.bytes[offset0].byteu.reg->rIdx : -1;
  int r1Idx = ((aopRS (op1) && offset1 < 8 && op1->aopu.bytes[offset1].in_reg)) ? op1->aopu.bytes[offset1].byteu.reg->rIdx : -1;

  *prefixes = 0;

  if (r0Idx == XL_IDX)
    {
      if (r1Idx == XH_IDX || r1Idx == YL_IDX || r1Idx == YH_IDX || r1Idx == ZL_IDX || r1Idx == ZH_IDX)
        return 1;
      else if (op1->type == AOP_LIT || op1->type == AOP_IMMD ||
        offset1 >= op1->size ||
        op1->type == AOP_STK || op1->type == AOP_REGSTK && r1Idx == -1)
        return 2;
      else if (op1->type == AOP_DIR)
        return 3;
      else
        return -1;
    }
  else if (r1Idx == XL_IDX && op1->type == AOP_STK || op1->type == AOP_REGSTK && r0Idx == -1) // ld (n, sp), xl
    return 2;
  else if (r1Idx == XL_IDX && op1->type == AOP_DIR) // ld mm, xl
    return 3;

  if (r0Idx == XH_IDX || r0Idx == YL_IDX || r0Idx == YH_IDX || r0Idx == ZL_IDX || r0Idx == ZH_IDX) // Try with alternate accu prefix.
    {
      int bytes = ld_bytes (prefixes, ASMOP_XL, 0, op1, offset1);
      if (bytes >= 0 && *prefixes == 0)
        {
          (*prefixes)++;
          return bytes + 1;
        }
    }
  if (r1Idx == XH_IDX || r1Idx == YL_IDX || r1Idx == ZL_IDX) // Try with alternate accu prefix.
    {
      bool replace1 = (r1Idx == XH_IDX || r1Idx == YL_IDX || r1Idx == ZL_IDX);
      int bytes = ld_bytes (prefixes, op0, offset0, replace1 ? ASMOP_XL : op1, replace1 ? 0 : offset1);
      if (bytes >= 0 && *prefixes == 0)
        {
          (*prefixes)++;
          return bytes + 1;
        }
    }
  if (r1Idx == XL_IDX || r1Idx == XH_IDX || r1Idx == YL_IDX || r1Idx == ZL_IDX) // Try with swap prefix
    {
      int bytes = ld_bytes (prefixes, op1, offset1, op0, offset0);
      if (bytes >= 0 && *prefixes == 0)
        {
          (*prefixes)++;
          return bytes + 1;
        }
    }

  return -1;
}

static void
ld_cost (const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  int prefixes;
  int bytes = ld_bytes (&prefixes, op0, offset0, op1, offset1);

  wassert_bt (bytes > 0);

  // Multiple prefixes no longer supported in latest f8 draft.
  if(prefixes > 1)
    cost (100, 100);

  cost (bytes, prefixes + 1);
}

// How many bytes would it take for the two-operand 8-bit load (including prefixes)? Also return the number of prefix bytes in *prefix.
// returns -1 if impossible.
static int
ldw_bytes (int *prefixes, const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  int r0Idx = ((aopRS (op0) && offset0 < 8 && op0->aopu.bytes[offset0].in_reg)) ? op0->aopu.bytes[offset0].byteu.reg->rIdx : -1;
  int r1Idx = ((aopRS (op1) && offset1 < 8 && op1->aopu.bytes[offset1].in_reg)) ? op1->aopu.bytes[offset1].byteu.reg->rIdx : -1;

  *prefixes = 0;

  if (r0Idx >= 0)
    {
      if (!op0->aopu.bytes[offset0 + 1].in_reg || op0->aopu.bytes[offset0 + 1].byteu.reg->rIdx != r0Idx + 1)
        return -1;
      switch (r0Idx)
        {
        case XL_IDX:
          r0Idx = X_IDX;
          break;
        case YL_IDX:
          r0Idx = Y_IDX;
          break;
        case ZL_IDX:
          r0Idx = Z_IDX;
          break;
        default:
          return -1;
        }
    }
  if (r1Idx >= 0)
    {
      if (!op1->aopu.bytes[offset1 + 1].in_reg || op1->aopu.bytes[offset1 + 1].byteu.reg->rIdx != r1Idx + 1)
        return -1;
      switch (r1Idx)
        {
        case XL_IDX:
          r1Idx = X_IDX;
          break;
        case YL_IDX:
          r1Idx = Y_IDX;
          break;
        case ZL_IDX:
          r1Idx = Z_IDX;
          break;
        default:
          return -1;
        }
    }

  if (r0Idx == Y_IDX)
    if (r1Idx == X_IDX || r1Idx == Z_IDX)
      return 1;
    else if (op1->type == AOP_LIT && byteOfVal (op1->aopu.aop_lit, offset1 + 1) == ((byteOfVal (op1->aopu.aop_lit, offset1) & 0x80) ? 0xff : 0x00) || offset1 >= op1->size) // ldw y, #d
      return 2;
    else if (op1->type == AOP_LIT || op1->type == AOP_IMMD || // ldw y, #ii
      op1->type == AOP_DIR) // ldw y, mm
      return 3;
    else if (op1->type == AOP_STK || op1->type == AOP_REGSTK && r1Idx == -1) // ldw y, (n, sp)
      return 2;
    else
      return -1;
  else if (r1Idx == Y_IDX)
    if (r0Idx == X_IDX || r0Idx == Z_IDX)
      return 1;
    else if (op0->type == AOP_DIR) // ldw mm, y
      return 3;
    else if (op0->type == AOP_STK || op0->type == AOP_REGSTK && r0Idx == -1) // ldw (n, sp), y
      return 2;

  if(r0Idx == X_IDX && r1Idx == Z_IDX || r0Idx == Z_IDX && r1Idx == X_IDX)
    {
      *prefixes = 1;
      return 2;
    }

  if (r0Idx == X_IDX || r0Idx == Z_IDX || // Try with alternate accumulator prefix.
    r1Idx == X_IDX || r1Idx == Z_IDX)
    {
      bool replace0 = (r0Idx == X_IDX || r0Idx == Z_IDX);
      bool replace1 = (r1Idx == X_IDX || r1Idx == Z_IDX); // TODO: Each prefix can only replace one!
      int bytes = ldw_bytes (prefixes, replace0 ? ASMOP_Y : op0, replace0 ? 0 : offset0, replace1 ? ASMOP_Y : op1, replace1 ? 0 : offset1);
      if (bytes >= 0 && *prefixes == 0)
        {
          (*prefixes)++;
          return bytes + 1;
        }
    }

  return -1;
}

static void
ldw_cost (const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  int prefixes;
  int bytes = ldw_bytes (&prefixes, op0, offset0, op1, offset1);

  wassert_bt (bytes > 0);

  cost (bytes, prefixes + 1);
}

static void
opw_cost (const asmop *op0, int offset0)
{
  if (aopOnStack (op0, offset0, 2))
    {
      cost (2, 1);
      return;
    }
  else if (op0->type == AOP_DIR)
    {
      cost (3, 1);
      return;
    }

  int r0Idx = ((aopRS (op0) && op0->aopu.bytes[offset0].in_reg)) ? op0->aopu.bytes[offset0].byteu.reg->rIdx : -1;

  if (r0Idx < 0)
    wassert(0);

  if (!op0->aopu.bytes[offset0 + 1].in_reg || op0->aopu.bytes[offset0 + 1].byteu.reg->rIdx != r0Idx + 1)
    wassert(0);
  switch (r0Idx)
    {
    case XL_IDX:
      r0Idx = X_IDX;
      break;
    case YL_IDX:
      r0Idx = Y_IDX;
      break;
    case ZL_IDX:
      r0Idx = Z_IDX;
      break;
    default:
      wassert(0);
    }

  cost (1 + (r0Idx != Y_IDX), 1 + (r0Idx != Y_IDX));
}

static void
xch_cost (const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  if (aopRS (op0) && op0->aopu.bytes[offset0].in_reg && aopRS (op1) && op1->aopu.bytes[offset1].in_reg)
    if (op0->aopu.bytes[offset0].byteu.reg->rIdx == XL_IDX && op1->aopu.bytes[offset1].byteu.reg->rIdx == XH_IDX)
      opw_cost (ASMOP_X, 0);
    else if (op0->aopu.bytes[offset0].byteu.reg->rIdx == YL_IDX && op1->aopu.bytes[offset1].byteu.reg->rIdx == YH_IDX)
      opw_cost (ASMOP_Y, 0);
    else if (op0->aopu.bytes[offset0].byteu.reg->rIdx == ZL_IDX && op1->aopu.bytes[offset1].byteu.reg->rIdx == ZH_IDX)
      opw_cost (ASMOP_Z, 0);
    else
      wassert_bt (0);
  else if ((op1->type == AOP_STK || op1->type == AOP_REGSTK) && !op1->aopu.bytes[offset1].in_reg)
    cost (2, 1);
  else 
    wassert (0);
}

static void
emit3cost (enum asminst inst, const asmop *op0, int offset0, const asmop *op1, int offset1)
{
  switch (inst)
  {
  case A_SBC:
  case A_SUB:
    wassertl_bt (op1 && op1->type != AOP_LIT && op1->type != AOP_IMMD, "Subtraction with constant right operand not available.");
  case A_ADC:
  case A_ADD:
  case A_AND:
  case A_CP:
  case A_OR:
  case A_XOR:
    op2_cost (op0, offset0, op1, offset1);
    break;
  case A_LD:
    ld_cost (op0, offset0, op1, offset1);
    break;
  case A_CLR:
  case A_DEC:
  case A_INC:
  case A_RLC:
  case A_RRC:
  case A_SLL:
  case A_SRL:
  case A_TST:
    op_cost (op0, offset0);
    break;
  case A_BOOL:
  case A_ROT:
  case A_SRA:
    if (aopInReg (op0, offset0, XL_IDX))
      cost (1, 1);
    else if (aopIsAcc8 (op0, offset0))
      cost (2, 1);
    else
      wassertl_bt (0, "Tried to get cost for invalid instruction");
    break;
  case A_XCH:
    xch_cost (op0, offset0, op1, offset1);
    break;
  case A_SBCW:
  case A_SUBW:
    wassertl_bt (!op1 || op1->type != AOP_LIT && op1->type != AOP_IMMD, "Subtraction with constant right operand not available.");
  case A_ADCW:
  case A_ADDW:
    if (inst == A_ADDW && op1 &&
      (op1->type == AOP_LIT && byteOfVal (op1->aopu.aop_lit, offset1 + 1) == ((byteOfVal (op1->aopu.aop_lit, offset1) & 0x80) ? 0xff : 0x00) || offset1 >= op1->size)) // addw y, #d
      {
        if (aopInReg (op0, offset0, Y_IDX))
          cost (1, 1);
        else if (aopInReg (op0, offset0, X_IDX) || aopInReg (op0, offset0, Z_IDX))
          cost (2, 1);
        else
          wassertl_bt (0, "Tried to get cost for invalid instruction");
        break;
      }
  case A_ORW:
  case A_XORW:
    if (op1)
      op2w_cost (op0, offset0, op1, offset1);
    else
      opw_cost (op0, offset0);
    break;
  case A_LDW:
    ldw_cost (op0, offset0, op1, offset1);
    break;
  case A_CLRW:
  case A_INCW:
  case A_TSTW:
    opw_cost (op0, offset0);
    break;
  case A_RLCW:
  case A_RRCW:
    if (aopOnStackNotExt (op0, offset0, 2))
      {
        cost (2, 1);
        return;
      }
  case A_BOOLW:
  case A_MUL:
  case A_NEGW:
  case A_SLLW:
  case A_SRAW:
  case A_SRLW:
    if (aopInReg (op0, offset0, Y_IDX))
      cost (1, 1);
    else if (aopInReg (op0, offset0, X_IDX) || aopInReg (op0, offset0, Z_IDX))
      cost (2, 1);
    else
      wassertl_bt (0, "Tried to get cost for invalid instruction");
    break;
  case A_CPW:
    wassert_bt (op1->type == AOP_LIT || op1->type == AOP_IMMD || offset1 >= op1->size);
    if (aopInReg (op0, offset0, Y_IDX))
      cost (3, 1);
    else if (aopInReg (op0, offset0, X_IDX) || aopInReg (op0, offset0, Z_IDX))
      cost (4, 1);
    else
      wassertl_bt (0, "Tried to get cost for invalid cpw instruction");
    break;
  case A_DECW:
    if (!aopOnStackNotExt (op0, offset0, 2))
      wassertl_bt (0, "Tried to get cost for invalid decw instruction");
    cost (2, 1);
    break;
  default:
    wassertl_bt (0, "Tried to get cost for unknown instruction");
  }
}

static void
emit3_o (const enum asminst inst, asmop *op0, int offset0, asmop *op1, int offset1)
{
  emit3cost (inst, op0, offset0, op1, offset1);
  if (regalloc_dry_run)
    return;

  bool wide = // Same order as in emit3cost above
    (inst == A_SBCW || inst == A_SUBW || inst == A_ADCW || inst == A_ADDW || inst == A_ORW || inst == A_XORW ||
    inst == A_LDW ||
    inst == A_CLRW || inst == A_DECW || inst == A_INCW || inst == A_TSTW ||
    inst == A_RLCW || inst == A_RRCW ||
    inst == A_BOOLW || inst == A_MUL || inst == A_NEGW || inst == A_SLLW || inst == A_SRAW || inst == A_SRLW ||
    inst == A_CPW);

  if (op1)
    {
      char *l = Safe_strdup (wide ? aopGet2 (op0, offset0) : aopGet (op0, offset0));
      emit2 (asminstnames[inst], "%s, %s", l, wide ? aopGet2 (op1, offset1) : aopGet (op1, offset1));
      Safe_free (l);
    }
  else
    emit2 (asminstnames[inst], "%s", wide ? aopGet2 (op0, offset0) : aopGet (op0, offset0));

  switch (inst)
    {
    case A_TST:
      G.c = 0;
      break;
    case A_TSTW:
      G.c = 1;
      break;
    case A_AND:
    case A_BOOL:
    case A_BOOLW:
    case A_CLR:
    case A_CLRW:
    case A_LD:
    case A_LDW:
    case A_OR:
    case A_ORW:
    case A_ROT:
    case A_SEX:
    case A_XCH:
    case A_XOR:
    case A_XORW:
      break;
    default:
      G.c = -1;
    }
}

static void
emit3 (enum asminst inst, asmop *op0, asmop *op1)
{
  emit3_o (inst, op0, 0, op1, 0);
}

// A variant of emit3_o that replaces the non-existing subtraction instructions with immediate operand by their addition equivalents. Used to be defined here, but has been moved further down, since some workarounds for assembler issues require the use of pop and push.
static void
emit3sub_o (enum asminst inst, asmop *op0, int offset0, asmop *op1, int offset1); // todo: allow to pass size, so instead of setting carry, we can just go for addition with +1 added to literal operand, when doing the full size in one instruction.

static void
emit3sub (enum asminst inst, asmop *op0, asmop *op1)
{
  emit3sub_o (inst, op0, 0, op1, 0);
}
             
static bool
regDead (int idx, const iCode *ic)
{
  if (idx == X_IDX)
    return (regDead (XL_IDX, ic) && regDead (XH_IDX, ic));
  if (idx == Y_IDX)
    return (regDead (YL_IDX, ic) && regDead (YH_IDX, ic));
  if (idx == Z_IDX)
    return (regDead (ZL_IDX, ic) && regDead (ZH_IDX, ic));

  if ((idx == ZL_IDX || idx == ZH_IDX) && f8_extend_stack)
    return false;

  return (!bitVectBitValue (ic->rSurv, idx));
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

  aop->regs[XL_IDX] = -1;
  aop->regs[XH_IDX] = -1;
  aop->regs[YL_IDX] = -1;
  aop->regs[YH_IDX] = -1;
  aop->regs[ZL_IDX] = -1;
  aop->regs[ZH_IDX] = -1;
  aop->regs[C_IDX] = -1;

  aop->valinfo.anything = true;

  return (aop);
}

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (const iCode *ic, symbol *sym)
{
  asmop *aop;

  wassert_bt (ic);
  wassert_bt (sym);
  wassert_bt (sym->etype);

  // Unlike other backends we really free asmops; to avoid a double-free, we need to support multiple asmops for the same symbol.

  if (IS_FUNC (sym->type))
    {
      aop = newAsmop (AOP_IMMD);
      aop->aopu.immd = sym->rname;
      aop->aopu.immd_off = 0;
      aop->size = getSize (sym->type);
    }
  /* Assign depending on the storage class */
  else if (sym->onStack || sym->iaccess)
    {
      int offset;
      long int base;

      aop = newAsmop (AOP_STK);
      aop->size = getSize (sym->type);

      base = sym->stack + (sym->stack > 0 ? G.stack.param_offset : 0);

      if (labs(base) > (1 << 15))
      {
        if (!regalloc_dry_run)
          werror (W_INVALID_STACK_LOCATION);
        base = 0;
      }

      for(offset = 0; offset < aop->size && offset < 8; offset++)
        aop->aopu.bytes[offset].byteu.stk = base + offset;
    }
  else
    {
      aop = newAsmop (AOP_DIR);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
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
  long val = 0;

  wassert_bt (ic);

  for (;;)
    {
      if (ic->op == '+')
        {
          if (isOperandLiteral (IC_RIGHT (ic)))
            {
              val += (long) operandLitValue (IC_RIGHT (ic));
              ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
            }
          else
            {
              val += (long) operandLitValue (IC_LEFT (ic));
              ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
            }
        }
      else if (ic->op == '-')
        {
          val -= (long) operandLitValue (IC_RIGHT (ic));
          ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
        }
      else if (IS_CAST_ICODE (ic))
        {
          ic = OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
        }
      else if (ic->op == ADDRESS_OF)
        {
          val += (long) operandLitValue (IC_RIGHT (ic));
          break;
        }
      else
        wassert_bt (0);
    }

  if (OP_SYMBOL (IC_LEFT (ic))->onStack)
    {
      aop = newAsmop (AOP_STL);
      long int base = (long)(OP_SYMBOL (IC_LEFT (ic))->stack) + (OP_SYMBOL (IC_LEFT (ic))->stack > 0 ? G.stack.param_offset : 0);
      aop->aopu.stk_off = base + val;
    }
  else
    {
      aop = newAsmop (AOP_IMMD);
      aop->aopu.immd = OP_SYMBOL (IC_LEFT (ic))->rname;
      aop->aopu.immd_off = val;
    }

  aop->size = getSize (sym->type);

  return aop;
}

/*-----------------------------------------------------------------*/
/* aopOp - allocates an asmop for an operand  :                    */
/*-----------------------------------------------------------------*/
static void
aopOp (operand *op, const iCode *ic, bool result)
{
  symbol *sym;

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
      aop->valinfo = getOperandValinfo (ic, op);
      op->aop = aop;
      return;
    }

  sym = OP_SYMBOL (op);

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aopForSym (ic, sym);
      if (!result)
        op->aop->valinfo = getOperandValinfo (ic, op);
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
          op->aop->valinfo = getOperandValinfo (ic, op);
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
  { 
    bool completely_in_regs = TRUE;
    bool completely_on_stack = TRUE;
    asmop *aop = newAsmop (AOP_REGSTK);

    aop->size = getSize (operandType (op));
    if (!result)
      aop->valinfo = getOperandValinfo (ic, op);
    op->aop = aop;

    for (int i = 0; i < aop->size; i++)
      {
        aop->aopu.bytes[i].in_reg = !!sym->regs[i];
        if (sym->regs[i])
          {
            completely_on_stack = FALSE;
            aop->aopu.bytes[i].byteu.reg = sym->regs[i];
            aop->regs[sym->regs[i]->rIdx] = i;
          }
        else if (sym->isspilt && sym->usl.spillLoc || sym->nRegs && regalloc_dry_run)
          {
            completely_in_regs = false;

            if (!regalloc_dry_run)
              {
                aop->aopu.bytes[i].byteu.stk = (long int)(sym->usl.spillLoc->stack) + (sym->usl.spillLoc->stack > 0 ? G.stack.param_offset : 0) + i;

                if (sym->usl.spillLoc->stack + i < -G.stack.pushed)
                  {
                    fprintf (stderr, "%s %ld %ld %ld %ld at ic %d\n", sym->name, (long)(sym->usl.spillLoc->stack), (long)(aop->size), (long)(i), (long)(G.stack.pushed), ic->key);
                    wassertl_bt (0, "Invalid stack offset.");
                  }
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

                aop->aopu.bytes[i].byteu.stk = old_base + i;
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

    if (completely_on_stack)
      aop->type = AOP_STK;

    return;
  }
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

/*--------------------------------------------------------------------------*/
/* updateCFA - update the debugger information to reflect the current       */
/*             connonical frame address relative to the stack pointer       */
/*--------------------------------------------------------------------------*/
static void
updateCFA (void)
{
  /* there is no frame unless there is a function */
  if (!currFunc)
    return;

  if (options.debug && !regalloc_dry_run)
    debugFile->writeFrameAddress (NULL, &f8_regs[SP_IDX], 1 + G.stack.param_offset + G.stack.pushed);
}

static void
spillReg (int rIdx)
{
  switch (rIdx)
    {
    case C_IDX:
      G.c = -1;
      break;
    }
}

static void
spillAllRegs (void)
{
  spillReg (C_IDX);
}

// Clear carry flag
static void
clrc (void)
{
  if (G.c != 0)
    emit3 (A_TST, ASMOP_XL, 0);
}

// Set carry flag
static void
setc (void)
{
  if (G.c != 1)
    emit3 (A_TSTW, ASMOP_Y, 0);
}

static void
push (const asmop *op, int offset, int size)
{
  if (size == 1 && aopInReg (op, offset, F_IDX))
    {
      emit2 ("push", "#0x00");
      emit2 ("xch", "f, (0, sp)");
      cost (3, 2);
    }
  else if (size == 1)
    {
      emit2 ("push", "%s", aopGet (op, offset));
      if (aopInReg (op, offset, XL_IDX) || aopInReg (op, offset, XH_IDX) || aopInReg (op, offset, YL_IDX) || aopInReg (op, offset, YH_IDX) || aopInReg (op, offset, ZL_IDX) || aopInReg (op, offset, ZH_IDX))
        cost (1, 1);
      else if (op->type == AOP_LIT || op->type == AOP_IMMD)
        cost (2, 1);
      else if (op->type == AOP_DIR)
        cost (3, 1);
      else if (aopOnStack (op, offset, 1))
        cost (2, 1);
      else
        wassertl_bt (0, "Invalid aop type for size 1 for push");
    }
  else if (size == 2)
    {
      emit2 ("pushw", aopGet2 (op, offset));
      if (aopInReg (op, offset, Y_IDX) || aopInReg (op, offset, X_IDX))
        cost (1, 1);
      else if (aopInReg (op, offset, Z_IDX))
        cost (2, 2);
      else if (op->type == AOP_LIT || op->type == AOP_IMMD || op->type == AOP_DIR)
        cost (1, 3);
      else if (aopOnStack (op, offset, 2))
        cost (1, 2);
      else
        wassertl_bt (0, "Invalid aop type for size 2 for pushw");
    }
  else
    wassertl_bt (0, "Invalid size for push/pushw");

  G.stack.pushed += size;
  updateCFA ();
}

static void
pop (const asmop *op, int offset, int size) // todo: xl_dead parameter for more efficient pop f, pop dir
{
  if (size == 1 && aopInReg (op, offset, F_IDX))
    {
      emit2 ("xch", "f, (0, sp)");
      emit2 ("addw", "sp, #1");
      cost (2, 3);
    }
  else if (size == 1 && op->type == AOP_DIR)
    {
      emit2 ("xch", "xl, (0, sp)");
      emit2 ("ld", "%s, xl", aopGet (op, offset));
      emit2 ("pop", "xl");
      cost (6, 3);
    }
  else if (size == 1)
    {
      emit2 ("pop", "%s", aopGet (op, offset));
      if (aopInReg (op, offset, XL_IDX))
        cost (1, 1);
      else if (aopInReg (op, offset, XH_IDX) || aopInReg (op, offset, YL_IDX) || aopInReg (op, offset, YH_IDX) || aopInReg (op, offset, ZL_IDX))
        cost (2, 2);
      else
        wassertl_bt (0, "Invalid aop type for size 1 for pop");
    }
  else if (size == 2)
    {
      emit2 ("popw", aopGet2 (op, offset));
      if (aopInReg (op, offset, Y_IDX))
        cost (1, 1);
      else if (aopInReg (op, offset, X_IDX) || aopInReg (op, offset, Z_IDX))
        cost (2, 2);
      else
        wassertl_bt (0, "Invalid aop type for size 2 for popw");
    }
  else
    wassertl_bt (0, "Invalid size for pop/popw");

  G.stack.pushed -= size;
  updateCFA ();
}

// A variant of emit3_o that replaces the non-existing subtraction instructions with immediate operand by their addition equivalents.
static void
emit3sub_o (enum asminst inst, asmop *op0, int offset0, asmop *op1, int offset1)
{
  unsigned int litword1 = 0;
  if (op1->type == AOP_LIT)
    litword1 = (byteOfVal (op1->aopu.aop_lit, offset1 + 1) << 8) | byteOfVal (op1->aopu.aop_lit, offset1);

  if (op1->type == AOP_LIT || op1->type == AOP_IMMD)
    switch (inst)
      {
      case A_SUB:
        if (op1->type == AOP_LIT && (~litword1 & 0xff) + 1 <= 0xff)
          {
            emit2 ("add", "%s, #0x%02x", aopGet (op0, offset0), (~litword1 & 0xff) + 1);
            cost (2 + !aopInReg (op0, offset0, XL_IDX), 1);
            spillReg (C_IDX);
            break;
          }
        setc ();
      case A_SBC:
        if (op1->type == AOP_LIT)
          {
            emit2 ("adc", "%s, #0x%02x", aopGet (op0, offset0), ~byteOfVal (op1->aopu.aop_lit, offset1) & 0xff);
            cost (2 + !aopInReg (op0, offset0, XL_IDX), 1);
            spillReg (C_IDX);
          }
        //else // todo: implement when supported by assembler
        //  emit2 ("adc", "%s, ~%s", aopGet (op0, offset0), aopGet (op1, offset1));
        else if (!aopInReg (op0, offset0, XH_IDX))
          {
            push (ASMOP_XH, 0, 1);
            emit3_o (A_LD, ASMOP_XH, 0, op1, offset1);
            emit3 (A_XOR, ASMOP_XH, ASMOP_MONE);
            emit2 ("adc", "%s, xh", aopGet (op0, offset0));
            cost (1 + !aopInReg (op0, offset0, XL_IDX), 1);
            spillReg (C_IDX);
            pop (ASMOP_XH, 0, 1);
          }
        else
          UNIMPLEMENTED;
        break;
      case A_SUBW:
        if (op1->type == AOP_LIT && (~litword1 & 0xffff) + 1 == 0x0001)
          {
            emit3_o (A_INCW, op0, offset0, 0, 0);
            break;
          }
        else if (op1->type == AOP_LIT && litword1 == 0x0000 && G.c == 0)
          {
            emit3_o (A_SBCW, op0, offset0, 0, 0);
            break;
          }
        else if (op1->type == AOP_LIT && (~litword1 & 0xffff) + 1 <= 0xffff)
          {
            emit2 ("addw", "%s, #0x%04x", aopGet2 (op0, offset0), (~litword1 & 0xffff) + 1);
            cost (2 + !aopInReg (op0, offset0, Y_IDX), 1 + !aopInReg (op0, offset0, Y_IDX));
            spillReg (C_IDX);
            break;
          }
        setc ();
      case A_SBCW:
        if (op1->type == AOP_LIT)
          {
            if (!litword1)
              emit3_o (A_SBCW, op0, offset0, 0, 0);
            else
              {
                emit2 ("adcw", "%s, #0x%04x", aopGet2 (op0, offset0), ~litword1 & 0xffff);
                cost (2 + !aopInReg (op0, offset0, Y_IDX), 1 + !aopInReg (op0, offset0, Y_IDX));
                spillReg (C_IDX);
              }
          }
        //else // todo: implement when supported by assembler
        //  emit2 ("adcw", "%s, ~%s", aopGet2 (op0, offset0), aopGet2 (op1, offset1));
        else if (!aopInReg (op0, offset0, X_IDX))
          {
            push (ASMOP_X, 0, 1);
            emit3_o (A_LDW, ASMOP_X, 0, op1, offset1);
            emit3_o (A_XOR, ASMOP_X, 0, ASMOP_MONE, 0);
            emit3_o (A_XOR, ASMOP_X, 1, ASMOP_MONE, 1);
            emit2 ("adcw", "%s, x", aopGet2 (op0, offset0));
            cost (1 + !aopInReg (op0, offset0, Y_IDX), 1);
            spillReg (C_IDX);
            pop (ASMOP_X, 0, 1);
          }
        else
          UNIMPLEMENTED;
        break;
      default:
        wassertl_bt (0, "Invalid instruction for emit3sub_o");
      }
  else
    emit3_o (inst, op0, offset0, op1, offset1);
}

static void
genMove_o (asmop *result, int roffset, asmop *source, int soffset, int size, bool xl_dead_global, bool xh_dead_global, bool y_dead_global, bool z_dead_global, bool c_dead);

/*-----------------------------------------------------------------*/
/* pointToSym - Store pointer to symbol into raop                  */
/*-----------------------------------------------------------------*/
static void
pointToSym (asmop *raop, const symbol *sym, long int offset, bool xl_dead, bool xh_dead, bool y_dead, bool z_dead)
{
  wassert (sym);
  if (sym->usl.spillLoc)
    sym = sym->usl.spillLoc;

  bool in_dest = !sym->onStack && aopIsAcc16 (raop, 0) ||
    aopInReg (raop, 0, Y_IDX) || aopIsAcc16 (raop, 0) && !y_dead;

  if (in_dest || y_dead)
    {
      struct asmop *aop = in_dest ? raop : ASMOP_Y;

      if (!sym->onStack)
        {
          wassert (sym->name);
          emit2 ("ldw", "%s, #%s+%ld", aopGet2 (aop, 0), sym->rname, offset);
          cost (3 + !aopInReg (aop, 0, Y_IDX), 1 + !aopInReg (aop, 0, Y_IDX));
        }
      else
        {
          long soffset = (long)(sym->stack) + G.stack.pushed + offset;
          
          emit2 ("ldw", "%s, sp", aopGet2 (aop, 0));
          cost (1 + !aopInReg (aop, 0, Y_IDX), 1 + !aopInReg (aop, 0, Y_IDX));
          if (soffset == 1)
            {
              emit3 (A_INCW, ASMOP_Y, 0);
              spillReg (C_IDX);
            }
          else if (soffset)
            {
              emit2 ("addw", "%s, #%ld", aopGet2 (aop, 0), soffset);
              cost (2 + !aopInReg (aop, 0, Y_IDX) + (labs(soffset) > 127), 1 + !aopInReg (aop, 0, Y_IDX));
              spillReg (C_IDX);
            }
        }
      genMove_o (raop, 0, aop, 0, 2, xl_dead, xh_dead, y_dead, z_dead, true);
    }
  else 
    UNIMPLEMENTED;
}

/*-----------------------------------------------------------------*/
/* genCopyStack - Copy the value - stack to stack only             */
/*-----------------------------------------------------------------*/
static void
genCopyStack (asmop *result, int roffset, asmop *source, int soffset, int n, bool *assigned, int *size, bool xl_free, bool xh_free, bool y_free, bool z_free, bool really_do_it_now)
{
  wassert (result->type == AOP_REGSTK || result->type == AOP_STK);
  wassert (source->type == AOP_REGSTK || source->type == AOP_STK);
  int rstk = -1, sstk = -1;
  for (int i = 0; i < n && i < 8; i++)
    {
      if (!result->aopu.bytes[roffset + i].in_reg)
        rstk = result->aopu.bytes[roffset + i].byteu.stk - i;
      if (!source->aopu.bytes[soffset + i].in_reg)
        sstk = source->aopu.bytes[soffset + i].byteu.stk - i;
    }
  
  bool copy_down = (sstk < rstk) && (sstk + n > rstk);

#if 0
  emit2 (";", "genCopyStack copy_down %d rstk %d sstk %d sstk + n %d", copy_down, rstk, sstk, sstk + n);
#endif
  if (copy_down)
    {
      for (int i = n - 1; i >= 0;)
        {
          if (assigned[i] || !aopOnStack (result, roffset + i, 1) || !aopOnStack (source, soffset + i, 1))
            {
              i--;
              continue;
            }
    
          // Same location.
          if (!assigned[i] &&
            result->aopu.bytes[roffset + i].byteu.stk == source->aopu.bytes[soffset + i].byteu.stk)
            {
              wassert_bt (*size >= 1);
    
              assigned[i] = true;
              (*size)--;
              i--;
              continue;
            }

          // Do not overwrite still-needed byte on stack.
          for (int j = 0; j < n; j++)
            if (!assigned[j] && aopOnStack (source, soffset + j, 1) && result->aopu.bytes[roffset + i].byteu.stk == source->aopu.bytes[soffset + j].byteu.stk)
              goto outer_continue_down;

          // Do two bytes at once, if possible.
          if (i > 0 && !assigned[i - 1] && aopOnStack (result, roffset + i - 1, 1) && aopOnStack (source, soffset + i - 1, 1) &&
            (y_free || z_free || (xl_free && xh_free))) // Prefer y, since it is cheaper. Using z or x is still cheaper than using xl twice below, though.
            {
              asmop *taop = y_free ? ASMOP_Y : z_free ? ASMOP_Z : ASMOP_X;
              emit3_o (A_LDW, taop, 0, source, soffset + i - 1);
              emit3_o (A_LDW, result, roffset + i - 1, taop, 0);
              assigned[i] = true;
              assigned[i - 1] = true;
              (*size) -= 2;
              i -= 2;
              continue;
            }
          else if (!xl_free && really_do_it_now && i > 0 && !assigned[i - 1] && aopOnStack (result, roffset + i - 1, 1) && aopOnStack (source, soffset + i - 1, 1))
            {
              if (!y_free)
                push (ASMOP_Y, 0, 2);
              emit3_o (A_LDW, ASMOP_Y, 0, source, soffset + i - 1);
              emit3_o (A_LDW, result, roffset + i - 1, ASMOP_Y, 0);
              if (!y_free)
                pop (ASMOP_Y, 0, 2);
              assigned[i] = true;
              assigned[i - 1] = true;
              (*size) -= 2;
              i -= 2;
              continue;
            }

          if (xl_free || really_do_it_now)
            {
              if (!xl_free)
                push (ASMOP_XL, 0, 1);
              emit3_o (A_LD, ASMOP_XL, 0, source, soffset + i);
              emit3_o (A_LD, result, roffset + i, ASMOP_XL, 0);
              if (!xl_free)
                pop (ASMOP_XL, 0, 1);
              assigned[i] = true;
              (*size)--;
            }
outer_continue_down:
          i--;
        }
    }
  else
    {
      for (int i = 0; i < n;)
        {
          if (assigned[i] || !aopOnStack (result, roffset + i, 1) || !aopOnStack (source, soffset + i, 1))
            {
              i++;
              continue;
            }
    
          // Same location.
          if (result->aopu.bytes[roffset + i].byteu.stk == source->aopu.bytes[soffset + i].byteu.stk)
            {
              wassert_bt (*size >= 1);
    
              assigned[i] = true;
              (*size)--;
              i++;
              continue;
            }

          // Do not overwrite still-needed byte on stack.
          for (int j = 0; j < n; j++)
            if (!assigned[j] && aopOnStack (source, soffset + j, 1) && result->aopu.bytes[roffset + i].byteu.stk == source->aopu.bytes[soffset + j].byteu.stk)
              goto outer_continue_up;

          // Do two bytes at once, if possible.
          if (i + 1 < n && !assigned[i + 1] && aopOnStack (result, roffset + i + 1, 1) && aopOnStack (source, soffset + i + 1, 1) &&
            (y_free || z_free || (xl_free && xh_free))) // Prefer y, since it is cheaper. Using x is still cheaper than using xl twice below, though.
            {
              asmop *taop = y_free ? ASMOP_Y : z_free ? ASMOP_Z : ASMOP_X;
              emit3_o (A_LDW, taop, 0, source, soffset + i);
              emit3_o (A_LDW, result, roffset + i, taop, 0);
              assigned[i] = true;
              assigned[i + 1] = true;
              (*size) -= 2;
              i += 2;
              continue;
            }
          else if (!xl_free && really_do_it_now && i + 1 < n && !assigned[i + 1] && aopOnStack (result, roffset + i + 1, 1) && aopOnStack (source, soffset + i + 1, 1))
            {
              if (!y_free)
                push (ASMOP_Y, 0, 2);
              emit3_o (A_LDW, ASMOP_Y, 0, source, soffset + i);
              emit3_o (A_LDW, result, roffset + i, ASMOP_Y, 0);
              if (!y_free)
                pop (ASMOP_Y, 0, 2);
              assigned[i] = true;
              assigned[i + 1] = true;
              (*size) -= 2;
              i += 2;
              continue;
            }

          if (xl_free || really_do_it_now)
            {
              if (!xl_free)
                push (ASMOP_XL, 0, 1);
              emit3_o (A_LD, ASMOP_XL, 0, source, soffset + i);
              emit3_o (A_LD, result, roffset + i, ASMOP_XL, 0);
              if (!xl_free)
                pop (ASMOP_XL, 0, 1);
              assigned[i] = true;
              (*size)--;
            }
outer_continue_up:
          i++;
        }
    }
}

/*-----------------------------------------------------------------*/
/* genCopy - Copy the value from one reg/stk asmop to another      */
/*-----------------------------------------------------------------*/
static void
genCopy (asmop *result, int roffset, asmop *source, int soffset, int sizex, bool xl_dead_global, bool xh_dead_global, bool y_dead_global, bool z_dead_global, bool c_dead)
{
  int n = (sizex < source->size - soffset) ? sizex : (source->size - soffset);
  bool assigned[8] = {false, false, false, false, false, false, false, false};
  bool xl_free, xh_free, y_free, z_free;

  wassert (aopRS (result) && aopRS (source));

  int size = n;
  int regsize = 0;
  for (int i = 0; roffset + i < 8 && soffset + i < 8 && i < n; i++)
    regsize += source->aopu.bytes[soffset + i].in_reg;

  // Do nothing for coalesced bytes.
  for (int i = 0; roffset + i < 8 && soffset + i < 8 && i < n; i++)
    if (result->aopu.bytes[roffset + i].in_reg && source->aopu.bytes[soffset + i].in_reg && result->aopu.bytes[roffset + i].byteu.reg == source->aopu.bytes[soffset + i].byteu.reg)
      {
        assigned[i] = true;
        regsize--;
        size--;
      }

  // Try to use zex.
  if (n == 1 && sizex == 2 && aopAreExt (result, roffset, source, soffset))
    {
      char *s = Safe_strdup (aopGet (source, soffset)); 
      emit2 ("zex", "%s, %s", aopGet2 (result, roffset), s);
      Safe_free (s);
      cost (1 + !aopInReg (result, roffset, Y_IDX), 1 + !aopInReg (result, roffset, Y_IDX));
      assigned[0] = true;
      assigned[1] = true;
      if (!aopInReg (result, roffset, X_IDX))
        {
          size--;
          regsize--;
        }
    }

  // Clear registers now that would be more expensive to clear later.
  if(n >= 1 && !assigned[n - 1] && sizex > n && !assigned[n] && aopInReg (result, roffset + n - 1, Y_IDX) && // We want to clear the high byte of y.
    size - regsize <= 1) // We won't need y for stack-to-stack copies.
    {
      const bool yl_free = source->regs[YL_IDX] < soffset || assigned[source->regs[YL_IDX] - soffset];
      const bool yh_free = source->regs[YH_IDX] < soffset || assigned[source->regs[YH_IDX] - soffset];
      const bool y_free = yl_free && yh_free;

      if (y_free)
        {
          emit3_o (A_CLRW, result, roffset + n - 1, 0, 0);
          assigned[n] = true;
        }
    }

  // Move everything from registers to the stack.
  for (int i = 0; i < n;)
    {
      if (aopOnStack (result, roffset + i, 1)) // Check that we don't overwrite a still-needed byte on the stack.
        for (int j = 0; j < n; j++)
          if (!assigned[j] && aopOnStack (source, soffset + j, 1) && result->aopu.bytes[roffset + i].byteu.stk == source->aopu.bytes[soffset + j].byteu.stk)
            {
              i++;
              goto outer_continue;
            }

      bool xl_free = xl_dead_global &&
        (source->regs[XL_IDX] < soffset || source->regs[XL_IDX] >= soffset + size || assigned[source->regs[XL_IDX] - soffset]) &&
        (result->regs[XL_IDX] < roffset || result->regs[XL_IDX] >= roffset + size || !assigned[result->regs[XL_IDX] - roffset]);

      if (i + 1 < n && aopOnStack (result, roffset + i, 2) &&
        (aopInReg (source, soffset + i, Y_IDX) || aopInReg (source, soffset + i, X_IDX) || aopInReg (source, soffset + i, Z_IDX)))
        {
          emit3_o (A_LDW, result, roffset + i, source, soffset + i);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          regsize -= 2;
          i += 2;
        }
      else if (aopOnStack (result, roffset + i, 1) &&
        (aopInReg (source, soffset + i, XL_IDX) || aopInReg (source, soffset + i, XH_IDX) || aopInReg (source, soffset + i, YL_IDX) || aopInReg (source, soffset + i, ZL_IDX)))
        {
          emit3_o (A_LD, result, roffset + i, source, soffset + i);
          assigned[i] = true;
          size--;
          regsize--;
          i++;
        }
      else if (aopOnStack (result, roffset + i, 1) && !aopOnStack (source, soffset + i, 1))
        {
          if (!xl_free)
            push (ASMOP_XL, 0, 1);
          emit3_o (A_LD, ASMOP_XL, 0, source, soffset + i);
          emit3_o (A_LD, result, roffset + i, ASMOP_XL, 0);
          if (!xl_free)
            pop (ASMOP_XL, 0, 1);
          assigned[i] = true;
          size--;
          regsize--;
          i++;
        }
      else // This byte is not a register-to-stack copy.
        i++;
outer_continue:
        ;
    }

  // Copy (stack-to-stack) what we can with whatever free regs we have.
  xl_free = xl_dead_global;
  xh_free = xh_dead_global;
  y_free = y_dead_global;
  z_free = z_dead_global;
  for (int i = 0; i < n; i++)
    {
      asmop *operand;
      int offset;

      if (!assigned[i])
        {
          operand = source;
          offset = soffset + i;
        }
      else
        {
          operand = result;
          offset = roffset + i;
        }

      if (aopInReg (operand, offset, XL_IDX))
        xl_free = false;
      else if (aopInReg (operand, offset, XH_IDX))
        xh_free = false;
      else if (aopInReg (operand, offset, YL_IDX) || aopInReg (operand, offset, YH_IDX))
        y_free = false;
      else if (aopInReg (operand, offset, ZL_IDX) || aopInReg (operand, offset, ZH_IDX))
        z_free = false;
    }
  genCopyStack (result, roffset, source, soffset, n, assigned, &size, xl_free, xh_free, y_free, z_free, false);

  // Now do the register shuffling.

  // Try to use xch yl, yh, etc.
  for (int b = XL_IDX; b <= ZL_IDX; b += 2)
    if (regsize >= 2)
      {
        int i;
        int ex[2] = {-1, -1};

        i = result->regs[b] - roffset;
        if (i > 0 && i < n && !assigned[i] && aopInReg (source, soffset + i, b + 1))
          ex[0] = i;
        i = result->regs[b + 1] - roffset;
        if (i > 0 && i < n && !assigned[i] && aopInReg (source, soffset + i, b))
          ex[1] = i;

        if (ex[0] >= 0 && ex[1] >= 0)
          {
            asmop *xchaop = b ? (b == 2 ? ASMOP_Y : ASMOP_Z) : ASMOP_X;
            emit3_o (A_XCH, xchaop, 0, xchaop, 1);
            assigned[ex[0]] = true;
            assigned[ex[1]] = true;
            regsize -= 2;
            size -= 2;
          }
      }

  // Try to use ldw y, x
  {
    const int il = result->regs[YL_IDX] - roffset;
    const int ih = result->regs[YH_IDX] - roffset;
    const bool assign_l = (il >= 0 && il < n && !assigned[il] && aopInReg (source, soffset + il, XL_IDX));
    const bool assign_h = (ih >= 0 && ih < n && !assigned[ih] && aopInReg (source, soffset + ih, XH_IDX));
    const bool yl_dead = y_dead_global && source->regs[YL_IDX] < soffset;
    if (source->regs[YL_IDX] < soffset && source->regs[YH_IDX] < soffset &&
      (assign_l && assign_h || yl_dead && il < 0 && assign_h))
    {
      emit3 (A_LDW, ASMOP_Y, ASMOP_X);
      if (assign_l)
        {
          assigned[il] = true;
          regsize--;
          size--;
        }
      if (assign_h)
        {
          assigned[ih] = true;
          regsize--;
          size--;
        }
    }
  }

  // Try to use ldw x, y
  {
    const int il = result->regs[XL_IDX] - roffset;
    const int ih = result->regs[XH_IDX] - roffset;
    const bool assign_l = (il >= 0 && il < n && !assigned[il] && aopInReg (source, soffset + il, YL_IDX));
    const bool assign_h = (ih >= 0 && ih < n && !assigned[ih] && aopInReg (source, soffset + ih, YH_IDX));
    const bool xl_dead = xl_dead_global && source->regs[XL_IDX] < soffset;
    if (source->regs[XL_IDX] < soffset && source->regs[XH_IDX] < soffset &&
      (assign_l && assign_h || xl_dead && il < 0 && assign_h))
    {
      emit3 (A_LDW, ASMOP_X, ASMOP_Y);
      if (assign_l)
        {
          assigned[il] = true;
          regsize--;
          size--;
        }
      if (assign_h)
        {
          assigned[ih] = true;
          regsize--;
          size--;
        }
    }
  }

  // Try to use ldw y, z
  {
    const int il = result->regs[YL_IDX] - roffset;
    const int ih = result->regs[YH_IDX] - roffset;
    const bool assign_l = (il >= 0 && il < n && !assigned[il] && aopInReg (source, soffset + il, ZL_IDX));
    const bool assign_h = (ih >= 0 && ih < n && !assigned[ih] && aopInReg (source, soffset + ih, ZH_IDX));
    const bool yl_dead = z_dead_global && source->regs[YL_IDX] < soffset;
    const bool yh_dead = z_dead_global && source->regs[YH_IDX] < soffset;
    if (source->regs[YL_IDX] < soffset && source->regs[YH_IDX] < soffset &&
      (assign_l && assign_h || yl_dead && il < 0 && assign_h || yh_dead && ih < 0 && assign_l))
    {
      emit3 (A_LDW, ASMOP_Y, ASMOP_Z);
      if (assign_l)
        {
          assigned[il] = true;
          regsize--;
          size--;
        }
      if (assign_h)
        {
          assigned[ih] = true;
          regsize--;
          size--;
        }
    }
  }

   // Try to use ldw z, y
  {
    const int il = result->regs[ZL_IDX] - roffset;
    const int ih = result->regs[ZH_IDX] - roffset;
    const bool assign_l = (il >= 0 && il < n && !assigned[il] && aopInReg (source, soffset + il, YL_IDX));
    const bool assign_h = (ih >= 0 && ih < n && !assigned[ih] && aopInReg (source, soffset + ih, YH_IDX));
    const bool zl_dead = z_dead_global && source->regs[ZL_IDX] < soffset;
    const bool zh_dead = z_dead_global && source->regs[ZH_IDX] < soffset;
    if (source->regs[ZL_IDX] < soffset && source->regs[ZH_IDX] < soffset &&
      (assign_l && assign_h || zl_dead && il < 0 && assign_h || zh_dead && ih < 0 && assign_l))
    {
      emit3 (A_LDW, ASMOP_Z, ASMOP_Y);
      if (assign_l)
        {
          assigned[il] = true;
          regsize--;
          size--;
        }
      if (assign_h)
        {
          assigned[ih] = true;
          regsize--;
          size--;
        }
    }
  }
  
  while (regsize)
    {
      int i;
      // Find lowest byte that can be assigned and needs to be assigned.
      for (i = 0; i < n; i++)
        {
          int j;

          if (assigned[i] || !source->aopu.bytes[soffset + i].in_reg || !result->aopu.bytes[roffset + i].in_reg)
            continue;

          for (j = 0; j < n; j++)
            {
              if (!source->aopu.bytes[soffset + j].in_reg || !result->aopu.bytes[roffset + i].in_reg)
                continue;
              if (!assigned[j] && i != j && result->aopu.bytes[roffset + i].byteu.reg == source->aopu.bytes[soffset + j].byteu.reg)
                goto skip_byte; // We can't write this one without overwriting the source.
            }

          break;                // Found byte that can be written safely.

skip_byte:
          ;
        }

      if (i < n) // We can safely assign a byte.
        {
          if (!aopInReg (result, roffset + i, YH_IDX) && !aopInReg (result, roffset + i, ZH_IDX) || // Assign via single ld if we can.
            !aopInReg (source, soffset + i, YH_IDX) && !aopInReg (source, soffset + i, ZH_IDX))
            emit3_o (A_LD, result, roffset + i, source, soffset + i);
          else // Go through xl instead.
            {
              bool xl_free = xl_dead_global &&
                (source->regs[XL_IDX] < soffset || source->regs[XL_IDX] >= soffset + n || assigned[source->regs[XL_IDX] - soffset]) &&
                (result->regs[XL_IDX] < roffset || result->regs[XL_IDX] >= roffset + n || !assigned[result->regs[XL_IDX] - roffset]);
              if (!xl_free)
                push (ASMOP_XL, 0, 1);
              emit3_o (A_LD, ASMOP_XL, 0, source, soffset + i);
              emit3_o (A_LD, result, roffset + i, ASMOP_XL, 0);
              if (!xl_free)
                pop (ASMOP_XL, 0, 1);
            }
          regsize--;
          size--;
          assigned[i] = true;
          continue;
        }

      // No byte can be assigned safely (i.e. the assignment is a permutation).
      UNIMPLEMENTED;
      return;
    }

  // Move everything from stack to registers.
  for (int i = 0; i < n;)
    {
      xl_free = xl_dead_global && (result->regs[XL_IDX] < roffset || result->regs[XL_IDX] >= roffset + source->size || !assigned[result->regs[XL_IDX] - roffset]);

      if (i + 1 < n && aopOnStack (source, soffset + i, 2) && aopIsAcc16 (result, roffset + i))
        {
          emit3_o (A_LDW, result, roffset + i, source, soffset + i);
          assigned[i] = true;
          assigned[i + 1] = true;
          size -= 2;
          i += 2;
        }
      else if (aopOnStack (source, soffset + i, 1) && aopIsAcc8 (result, roffset + i))
        {
          emit3_o (A_LD, result, roffset + i, source, soffset + i);
          assigned[i] = true;
          size--;
          i++;
        }
      else if (!aopOnStack (result, roffset + i, 1) && aopOnStack (source, soffset + i, 1))
        {
          if (!xl_free)
            push (ASMOP_XL, 0, 1);
          emit3_o (A_LD, ASMOP_XL, 0, source, soffset + i);
          emit3_o (A_LD, result, roffset + i, ASMOP_XL, 0);
          if (!xl_free)
            pop (ASMOP_XL, 0, 1);
          assigned[i] = true;
          size--;
          i++;
        }
      else // This byte is not a stack-to-register copy.
        i++;
    }

  // Copy (stack-to-stack) whatever is left.
  genCopyStack (result, roffset, source, soffset, n, assigned, &size, false, false, false, false, true);
  
  wassertl_bt (size >= 0, "genCopy() copied more than there is to be copied.");

  xl_free = xl_dead_global && (result->regs[XL_IDX] < roffset || result->regs[XL_IDX] >= roffset + source->size);
  xh_free = xh_dead_global && (result->regs[XH_IDX] < roffset || result->regs[XH_IDX] >= roffset + source->size);
  y_free = y_dead_global && (result->regs[YL_IDX] < roffset || result->regs[YL_IDX] >= roffset + source->size) && (result->regs[YH_IDX] < roffset || result->regs[YH_IDX] >= roffset + source->size);

  // Place leading zeroes.
  for (int i = source->size - soffset; i < sizex;)
    {
      if (assigned[i])
        {
          i++;
          continue;
        }
        
      int s = 1;
      for (int j = i + 1; j < sizex && !assigned[j]; j++, s++);

      genMove_o (result, roffset + i, ASMOP_ZERO, 0, s, xl_free, xh_free, y_free, false, c_dead);
      i += s;
    }

  if (size > 0)
    {
      if (!regalloc_dry_run)
        {
          wassertl_bt (0, "genCopy failed to completely copy operands.");
          fprintf (stderr, "%d bytes left.\n", size);
          fprintf (stderr, "left type %d source type %d\n", result->type, source->type);
          for (int i = 0; i < n ; i++)
            fprintf (stderr, "Byte %d, result in reg %d, source in reg %d. %s assigned.\n", i, (roffset + i < 8 && result->aopu.bytes[roffset + i].in_reg) ? result->aopu.bytes[roffset + i].byteu.reg->rIdx : -1, (soffset + i < 8 && source->aopu.bytes[soffset + i].in_reg) ? source->aopu.bytes[soffset + i].byteu.reg->rIdx : -1, assigned[i] ? "" : "not");
        }
      cost (180, 180);
    }
}

/*-----------------------------------------------------------------*/
/* genMove - Copy part of one asmop to another                     */
/*-----------------------------------------------------------------*/
static void
genMove_o (asmop *result, int roffset, asmop *source, int soffset, int size, bool xl_dead_global, bool xh_dead_global, bool y_dead_global, bool z_dead_global, bool c_dead)
{
  long val_y = -1;

  wassert_bt (result);
  wassertl_bt (result->type != AOP_LIT, "Trying to write to literal.");
  wassertl_bt (result->type != AOP_IMMD, "Trying to write to immediate.");
  wassertl_bt (roffset + size <= result->size, "Trying to write beyond end of operand");

#if 0
  D (emit2 (";", "genMove_o size %d, xl_dead_global %d xh_dead_global %d", size, xl_dead_global, xh_dead_global));
#endif

  if (aopRS (result) && aopRS (source))
    {
      genCopy (result, roffset, source, soffset, size, xl_dead_global, xh_dead_global, y_dead_global, z_dead_global, c_dead);
      return;
    }

  for (int i = 0; i < size;)
    {
      const bool xl_dead = xl_dead_global &&
        (!aopRS (result) || (result->regs[XL_IDX] >= (roffset + i) || result->regs[XL_IDX] < roffset)) &&
        (!aopRS (source) || source->regs[XL_IDX] <= i);
      const bool xh_dead = xh_dead_global &&
        (!aopRS (result) || (result->regs[XH_IDX] >= (roffset + i) || result->regs[XH_IDX] < roffset)) &&
        (!aopRS (source) || source->regs[XH_IDX] <= i);
      const bool x_dead = xl_dead && xh_dead;
      const bool y_dead = y_dead_global &&
        (!aopRS (result) || (result->regs[YL_IDX] >= (roffset + i) || result->regs[YL_IDX] < 0) && (result->regs[YH_IDX] >= (roffset + i) || result->regs[YH_IDX] < roffset)) &&
        (!aopRS (source) || source->regs[YL_IDX] <= i + 1 && source->regs[YH_IDX] <= i + 1);
      const bool z_dead = z_dead_global &&
        (!aopRS (result) || (result->regs[ZL_IDX] >= (roffset + i) || result->regs[ZL_IDX] < 0) && (result->regs[ZH_IDX] >= (roffset + i) || result->regs[ZH_IDX] < roffset)) &&
        (!aopRS (source) || source->regs[ZL_IDX] <= i + 1 && source->regs[ZH_IDX] <= i + 1);

      if (aopSame (result, roffset + i, source, soffset + i, 1))
        {
          i++;
          continue;
        }

      // Rematerialized stack location
      if (source->type == AOP_STL)
        {
          long stack_offset = (long)(source->aopu.stk_off) + G.stack.pushed;

          if (!y_dead && aopIsAcc16 (result, roffset + i) && !(soffset + i) && size >= 2)
            {
              emit2 ("ldw", "%s, sp", aopGet2 (result, roffset + i));
              cost (2, 1);
              if (stack_offset)
                {
                  if (!c_dead)
                    push (ASMOP_F, 0, 1);
                  emit2 ("addw", "%s, #%ld", aopGet2 (result, roffset + i), stack_offset);
                  cost (2 + (labs(stack_offset) > 127), 1);
                  if (!c_dead)
                    pop (ASMOP_F, 0, 1);
                  else
                    spillReg (C_IDX);
                }
              i += 2;
            }
          else if (y_dead || result->regs[YL_IDX] < 0 && result->regs[YH_IDX] < 0)
            {
              if (!y_dead)
                push (ASMOP_Y, 0, 2);
              stack_offset = (long)(source->aopu.stk_off) + G.stack.pushed; // Recalculate after push.
              emit2 ("ldw", "y, sp");
              cost (1, 1);
              if (stack_offset)
                {
                  if (!c_dead)
                    push (ASMOP_F, 0, 1);
                  emit2 ("addw", "y, #%ld", stack_offset);
                  cost (2 + (labs(stack_offset) > 127), 1);
                  if (!c_dead)
                    pop (ASMOP_F, 0, 1);
                  else
                    spillReg (C_IDX);
                }
              int lsize = size - i;
              genMove_o (result, roffset + i, ASMOP_Y, soffset + i, lsize, xl_dead, xh_dead, true, z_dead, c_dead);
              if (!y_dead)
                pop (ASMOP_Y, 0, 2);
              i += lsize;
            }
          else
            {
              UNIMPLEMENTED;
              i++;
            }

          continue;
        }
    
      if (i + 1 < size && (aopIsOp16_1 (result, roffset + i) || aopIsAcc16 (result, roffset + i)) && val_y >= 0 && aopIsLitVal (source, soffset + i, 2, val_y)) // Reuse cached value
        {
          if (!aopInReg (result, roffset + i,Y_IDX))
            emit3_o (A_LDW, result, roffset + i, ASMOP_Y, 0);
          i += 2;
          continue;
        }
      else if (i + 1 < size && aopIsOp16_1 (result, roffset + i) && aopIsLitVal (source, soffset + i, 2, 0x0000))
        {
          emit3_o (A_CLRW, result, roffset + i, 0, 0);
          if (aopInReg (result, roffset + i,Y_IDX))
            val_y = 0;
          i += 2;
          continue;
        }
      else if (i + 1 < size && aopIsAcc16 (result, roffset + i) &&
        (source->type == AOP_LIT || source->type == AOP_IMMD || (source->type == AOP_DIR && soffset + i + 1 < source->size) || aopOnStack (source, soffset + i, 2)) ||
        (result->type == AOP_DIR || aopOnStack (source, soffset + i, 2)) && i + 1 < size &&
        aopInReg (source, soffset + i, Y_IDX))
        {
          emit3_o (A_LDW, result, roffset + i, source, soffset + i);
          if (aopInReg (result, roffset + i, Y_IDX) && source->type == AOP_LIT)
            val_y = byteOfVal (source->aopu.aop_lit, soffset + i) + byteOfVal (source->aopu.aop_lit, soffset + i + 1) * 256;
          i += 2;
          continue;
        }
      else if (i + 1 < size && y_dead && aopIsLitVal (source, soffset + i, 2, 0x0000) && (aopInReg (result, roffset + i, YL_IDX) || aopInReg (result, roffset + i, YH_IDX)))
        {
          emit3 (A_CLRW, ASMOP_Y, 0);
          val_y = 0;
          i++;
          continue;
        }
      else if (i + 1 < size &&
        ((result->type == AOP_DIR || aopOnStack (result, roffset + i, 2)) && aopIsAcc16 (source, soffset + i) ||
        aopIsAcc16 (result, roffset + i) && (aopInReg (source, soffset + i, X_IDX) || aopInReg (source, soffset + i, Y_IDX))))
        {
          emit3_o (A_LDW, result, roffset + i, source, soffset + i);
          i += 2;
          continue;
        }
      else if (i + 1 < size && y_dead &&
        (result->type == AOP_DIR || aopOnStack (result, roffset + i, 2)) &&
        (source->type == AOP_DIR && soffset + i + 1 < source->size || source->type == AOP_LIT || source->type == AOP_IMMD || aopOnStack (source, soffset + i, 2)))
        {
          emit3_o (A_LDW, ASMOP_Y, 0, source, soffset + i);
          emit3_o (A_LDW, result, roffset + i, ASMOP_Y, 0);
          if (source->type == AOP_LIT)
            val_y = byteOfVal (source->aopu.aop_lit, soffset + i) + byteOfVal (source->aopu.aop_lit, soffset + i + 1) * 256;
          i += 2;
          continue;
        }
      else if (i + 1 < size && x_dead &&
        (result->type == AOP_DIR || aopOnStack (result, roffset + i, 2)) &&
        (source->type == AOP_DIR && soffset + i + 1 < source->size || source->type == AOP_LIT || source->type == AOP_IMMD || aopOnStack (source, soffset + i, 2)))
        {
          emit3_o (A_LDW, ASMOP_X, 0, source, soffset + i);
          emit3_o (A_LDW, result, roffset + i, ASMOP_X, 0);
          if (source->type == AOP_LIT)
            val_y = byteOfVal (source->aopu.aop_lit, soffset + i) + byteOfVal (source->aopu.aop_lit, soffset + i + 1) * 256;
          i += 2;
          continue;
        }
      else if (i + 1 < size && x_dead &&
        (result->type == AOP_DIR || aopOnStack (result, roffset + i, 2)) &&
        (source->type == AOP_DIR && soffset + i + 1 < source->size || source->type == AOP_LIT || source->type == AOP_IMMD || aopOnStack (source, soffset + i, 2)))
        {
          emit3_o (A_LDW, ASMOP_Z, 0, source, soffset + i);
          emit3_o (A_LDW, result, roffset + i, ASMOP_Z, 0);
          if (source->type == AOP_LIT)
            val_y = byteOfVal (source->aopu.aop_lit, soffset + i) + byteOfVal (source->aopu.aop_lit, soffset + i + 1) * 256;
          i += 2;
          continue;
        }
      else if (aopIsOp8_1 (result, roffset + i) && aopIsLitVal (source, soffset + i, 1, 0x00))
        {
          emit3_o (A_CLR, result, roffset + i, 0, 0);
          i++;
          continue;
        }
        
      bool via_xl =
        !aopInReg (result, roffset + i, XL_IDX) && !aopInReg (source, soffset + i, XL_IDX) &&
        !((aopInReg (result, roffset + i, XH_IDX) || aopInReg (result, roffset + i, YL_IDX) || aopInReg (result, roffset + i, ZL_IDX)) && (source->type == AOP_LIT || source->type == AOP_IMMD || source->type == AOP_DIR)) &&
        !(result->type == AOP_DIR && (aopInReg (source, soffset + i, XH_IDX) || aopInReg (source, soffset + i, YL_IDX) || aopInReg (source, soffset + i, ZL_IDX)));
      if (via_xl)
        {
          if (!xl_dead)
            push (ASMOP_XL, 0, 1);
          if (aopIsLitVal (source, soffset + i, 1, 0x00))
            emit3 (A_CLR, ASMOP_XL, 0);
          else  
            emit3_o (A_LD, ASMOP_XL, 0, source, soffset + i);
          emit3_o (A_LD, result, roffset + i, ASMOP_XL, 0);
          if (!xl_dead)
            pop (ASMOP_XL, 0, 1);
        }
      else
        emit3_o (A_LD, result, roffset + i, source, soffset + i);
      i++;
    }
}

/*-----------------------------------------------------------------*/
/* genMove - Copy the value from one asmop to another              */
/*-----------------------------------------------------------------*/
static void
genMove (asmop *result, asmop *source, bool xl_dead, bool xh_dead, bool y_dead, bool z_dead)
{
  wassert (result);
  genMove_o (result, 0, source, 0, result->size, xl_dead, xh_dead, y_dead, z_dead, true);
}

/*---------------------------------------------------------------------*/
/* f8_emitDebuggerSymbol - associate the current code location         */
/*   with a debugger symbol                                            */
/*---------------------------------------------------------------------*/
void
f8_emitDebuggerSymbol (const char *debugSym)
{
  G.debugLine = 1;
  emit2 ("", "%s ==.", debugSym);
  G.debugLine = 0;
}

// Get asmop for registers containing the return type of function
// Returns 0 if the function does not have a return value or it is not returned in registers.
static asmop *
aopRet (sym_link *ftype)
{
  wassert (IS_FUNC (ftype));

  int size = getSize (ftype->next);

  switch (size)
    {
    case 1:
      return (ASMOP_XL);
    case 2:
      return (ASMOP_Y);
    case 3:
      return (ASMOP_XLY);
    case 4:
      return (ASMOP_XY);
    default:
      return 0;
    }
}

// Get asmop for registers containing a parameter
// Returns 0 if the parameter is passed on the stack
static asmop *
aopArg (sym_link *ftype, int i)
{
  wassert (IS_FUNC (ftype));

  value *args = FUNC_ARGS(ftype);
  wassert (args);

  if (FUNC_HASVARARGS (ftype))
    return 0;

  int j;
  value *arg;
  for (j = 1, arg = args; j < i; j++, arg = arg->next)
    wassert (arg);

  if (IS_STRUCT (arg->type))
    return 0;

  if (i == 1 && getSize (arg->type) == 2)
    return ASMOP_Y;

  if (i == 1 && getSize (arg->type) == 1)
    return ASMOP_XL;

  return 0;
}

// Return true, iff ftype cleans up stack parameters.
static bool
isFuncCalleeStackCleanup (sym_link *ftype)
{
  wassert (IS_FUNC (ftype));

  return false;
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

      bool completely_spilt = true;
      for (unsigned int i = 0; i < getSize (sym->type); i++)
        if (sym->regs[i])
          completely_spilt = false;

      if (completely_spilt)
        return(true);
    }

  return (false);
}

/*--------------------------------------------------------------------------*/
/* adjustStack - Adjust the stack pointer by n bytes.                       */
/*--------------------------------------------------------------------------*/
static void
adjustStack (int n, bool xl_free, bool y_free)
{
  if (abs(n) > 513 && y_free)
   {
     emit2 ("ldw", "y, sp");
     emit2 ("addw", "y, #%d", n);
     emit2 ("ldw", "sp, y");
     cost (6, 3);
     spillReg (C_IDX);
     G.stack.pushed -= n;
     updateCFA ();
     return;
   }

  while (n)
    {
    	int m = n;
    	if (m < -128)
    	  m = -128;
    	else if (m > 127)
    	  m = 127;
    	
    	if (m == -1)
    	  {
    	    emit2 ("push", "xl");
    	    cost (1, 1);
    	  }
    	else if (m == -2)
    	  {
    	    emit2 ("pushw", "y");
    	    cost (1, 1);
    	  }
    	else if (m == 1 && xl_free)
    	  {
    	    emit2 ("pop", "xl");
    	    cost (1, 1);
    	  }
    	else if (m == 2 && y_free)
    	  {
    	    emit2 ("popw", "y");
    	    cost (1, 1);
    	  }
    	else
    	  {
    	    emit2 ("addw", "sp, #%d", m);
    	    cost (2, 1);
    	  }
    	n -= m;
    	G.stack.pushed -= m;
        updateCFA ();
    }
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

  aopOp (left, ic, false);
  aopOp (result, ic, true);

  if (left->aop->size == 1)
    {
      bool is_bool = !left->aop->valinfo.anything && left->aop->valinfo.min >= 0 && left->aop->valinfo.max <= 1;
      if (aopSame (result->aop, 0, left->aop, 0, 1) && aopIsAcc8 (result->aop, 0))
        {
          if (!is_bool)
            emit3 (A_BOOL, result->aop, 0);
          emit3 (A_XOR, result->aop, ASMOP_ONE);
          goto release;
        }

      if (!regDead (XL_IDX, ic))
        UNIMPLEMENTED;

      if (aopSame (result->aop, 0, left->aop, 0, 1) && is_bool && (result->aop->type == AOP_DIR || aopOnStack (result->aop, 0, 1)))
        {
          genMove (ASMOP_XL, ASMOP_ONE, true, false, false,false);
          emit3 (A_XOR, result->aop, ASMOP_XL);
          goto release;
        }
      else
        {
          genMove (ASMOP_XL, left->aop, true, false, regDead (Y_IDX, ic), regDead (Z_IDX, ic));
          if (!is_bool)
            emit3 (A_BOOL, ASMOP_XL, 0);
        }
    }
  else
    {
      if (!regDead (Y_IDX, ic) || left->aop->regs[YL_IDX] >= 2 || left->aop->regs[YH_IDX] >= 2)
        UNIMPLEMENTED;
      else
        {
          genMove (ASMOP_Y, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
          for (int i = 2; i < left->aop->size;)
            {
              if (i + 1 < left->aop->size && aopIsOp16_2 (left->aop, i))
                {
                  emit3_o (A_ORW, ASMOP_Y, 0, left->aop, i);
                  i += 2;
                }
              else if (aopIsOp8_2 (left->aop, i))
                {
                  emit3_o (A_OR, ASMOP_Y, 0, left->aop, i);
                  i++;
                }
              else
                {
                  UNIMPLEMENTED;
                  i++;
                }
            }
          emit3 (A_BOOLW, ASMOP_Y, 0);
        }
    }

  if (result->aop->size == 1)
    {
      if (!regDead (XL_IDX, ic))
        UNIMPLEMENTED;
      else
        {
          if (left->aop->size != 1)
            emit3 (A_LD, ASMOP_XL, ASMOP_Y);
          emit3 (A_XOR, ASMOP_XL, ASMOP_ONE);
          genMove (result->aop, ASMOP_XL, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
        }
    }
  else
    {
      if (!regDead (Y_IDX, ic))
        UNIMPLEMENTED;
      else
        {
          if (left->aop->size == 1)
            {
              emit2 ("zex", "y, xl");
              cost (1, 1);
            }
          emit3 (A_XOR, ASMOP_Y, ASMOP_ONE);
          genMove (result->aop, ASMOP_Y, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
        }
    }

release:
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genEor - code for bitwise exclusive or                          */
/*-----------------------------------------------------------------*/
static void
genEor (const iCode *ic, asmop *result_aop, asmop *left_aop, asmop *right_aop)
{
  int size = getSize (operandType (IC_RESULT (ic)));

  /* Prefer literal on right. */
  if (left_aop->type == AOP_LIT || right_aop->type != AOP_LIT && left_aop->type == AOP_IMMD) // todo: Swap in more cases when right in reg, left not. Swap individually per-byte.
    {
      asmop *t = right_aop;
      right_aop = left_aop;
      left_aop = t;
    }

  for (int i = 0; i < size;)
    {
       bool xl_free = regDead (XL_IDX, ic) && left_aop->regs[XL_IDX] <= i && right_aop->regs[XL_IDX] <= i && (result_aop->regs[XL_IDX] < 0 || result_aop->regs[XL_IDX] >= i);
       bool xh_free = regDead (XH_IDX, ic) && left_aop->regs[XH_IDX] <= i && right_aop->regs[XH_IDX] <= i && (result_aop->regs[XH_IDX] < 0 || result_aop->regs[XH_IDX] >= i);
       bool yl_free = regDead (YL_IDX, ic) && left_aop->regs[YL_IDX] <= i && right_aop->regs[YL_IDX] <= i && (result_aop->regs[YL_IDX] < 0 || result_aop->regs[YL_IDX] >= i);
       bool yh_free = regDead (YH_IDX, ic) && left_aop->regs[YH_IDX] <= i && right_aop->regs[YH_IDX] <= i && (result_aop->regs[YH_IDX] < 0 || result_aop->regs[YH_IDX] >= i);
       bool y_free = yl_free && yh_free;

       if (aopIsLitVal (right_aop, i, 1, 0x00))
         {
           int end;
           for(end = i; end < size && aopIsLitVal (right_aop, end, 1, 0x00); end++);
           genMove_o (result_aop, i, left_aop, i, end - i, xl_free, xh_free, false, false, true);
           i = end;
           continue;
         }
       else if (aopIsLitVal (left_aop, i, 1, 0x00))
         {
           int end;
           for(end = i; end < size && aopIsLitVal (left_aop, end, 1, 0x00); end++);
           genMove_o (result_aop, i, right_aop, i, end - i, xl_free, xh_free, false, false, true);
           i = end;
           continue;
         }

       if (i + 1 < size && aopAre16_2 (result_aop, i, right_aop, i) &&
         !(aopRS (right_aop) && aopRS (result_aop) && (result_aop->aopu.bytes[i].in_reg && right_aop->regs[result_aop->aopu.bytes[i].byteu.reg->rIdx] >= i || result_aop->aopu.bytes[i + 1].in_reg && right_aop->regs[result_aop->aopu.bytes[i + 1].byteu.reg->rIdx] >= i)))
         {
           genMove_o (result_aop, i, left_aop, i, 2, xl_free && right_aop->regs[XL_IDX] < i, xh_free && right_aop->regs[XH_IDX] < i, false, false, true);
           if ((aopInReg (result_aop, i, XL_IDX) || aopInReg (result_aop, i, ZL_IDX)) &&
             aopIsLitVal (right_aop, i + 1, 1, 0x00)) // Avoid xorw x/z, #ii, when xor xl/zl, #i will do.
             emit3_o (A_XOR, result_aop, i, right_aop, i);
           else
             emit3_o (A_XORW, result_aop, i, right_aop, i);
           i += 2;
           continue;
         }
       else if (i + 1 < size && aopAre16_2 (result_aop, i, left_aop, i) && !(aopInReg (result_aop, i, Z_IDX) && aopInReg (left_aop, i, X_IDX)) &&
         !(aopRS (left_aop) && aopRS (result_aop) && (result_aop->aopu.bytes[i].in_reg && left_aop->regs[result_aop->aopu.bytes[i].byteu.reg->rIdx] >= i || result_aop->aopu.bytes[i + 1].in_reg && left_aop->regs[result_aop->aopu.bytes[i + 1].byteu.reg->rIdx] >= i)))
         {
           genMove_o (result_aop, i, right_aop, i, 2, xl_free && left_aop->regs[XL_IDX] < i, xh_free && left_aop->regs[XH_IDX] < i, false, false, true);
           if ((aopInReg (result_aop, i, XL_IDX) || aopInReg (result_aop, i, ZL_IDX)) &&
             aopIsLitVal (left_aop, i + 1, 1, 0x00)) // Avoid xorw x/z, #ii, when xor xl/zl, #i will do.
             emit3_o (A_XOR, result_aop, i, left_aop, i);
           else
             emit3_o (A_XORW, result_aop, i, left_aop, i);
           i += 2;
           continue;
         }
       else if (i + 1 < size && y_free && aopOnStack (result_aop, i, 2) && (aopOnStack (left_aop, i, 2) || left_aop->type == AOP_DIR) && aopIsOp16_2 (right_aop, i))
         {
           genMove_o (ASMOP_Y, 0, left_aop, i, 2, xl_free && right_aop->regs[XL_IDX] < i, xh_free && right_aop->regs[XH_IDX] < i, true, false, true);
           emit3_o (A_XORW, ASMOP_Y, 0, right_aop, i);
           genMove_o (result_aop, i, ASMOP_Y, 0, 2, xl_free, xh_free, true, false, true);
           i += 2;
           continue;
         }


       if (aopSame (result_aop, i, left_aop, i, 1) && aopAre8_2 (left_aop, i, right_aop, i))
         {
           emit3_o (A_XOR, result_aop, i, right_aop, i);
           i++;
           continue;
         }
       else if (aopSame (result_aop, i, right_aop, i, 1) && aopAre8_2 (right_aop, i, left_aop, i))
         {
           emit3_o (A_XOR, result_aop, i, left_aop, i);
           i++;
           continue;
         }
       else if (i + 1 < size && !y_free && aopOnStack (result_aop, i, 2) && (aopOnStack (left_aop, i, 2) || left_aop->type == AOP_DIR) && aopIsOp16_2 (right_aop, i))
         {
           push (ASMOP_Y, 0, 2);
           genMove_o (ASMOP_Y, 0, left_aop, i, 2, xl_free && right_aop->regs[XL_IDX] < i, xh_free && right_aop->regs[XH_IDX] < i, true, false, true);
           emit3_o (A_XORW, ASMOP_Y, 0, right_aop, i);
           genMove_o (result_aop, i, ASMOP_Y, 0, 2, xl_free, xh_free, true, false, true);
           pop (ASMOP_Y, 0, 2);
           i += 2;
           continue;
         }

       if (!xl_free && aopInReg (result_aop, i, XL_IDX))
         UNIMPLEMENTED;

       if (!xl_free)
         push (ASMOP_XL, 0, 1);

       if (aopIsOp8_2 (right_aop, i))
         {
           genMove_o (ASMOP_XL, 0, left_aop, i, 1, true, xh_free, y_free, false, true);
           emit3_o (A_XOR, ASMOP_XL, 0, right_aop, i);
         }
       else if (aopIsOp8_2 (left_aop, i))
         {
           genMove_o (ASMOP_XL, 0, right_aop, i, 1, true, xh_free, y_free, false, true);
           emit3_o (A_XOR, ASMOP_XL, 0, left_aop, i);
         }
       else
         UNIMPLEMENTED;
       genMove_o (result_aop, i, ASMOP_XL, 0, 1, true, xh_free, y_free, false, true);
       if (!xl_free)
         pop (ASMOP_XL, 0, 1);

       i++;
    }
}
         
/*-----------------------------------------------------------------*/
/* genCpl - generates code for ~                                   */
/*-----------------------------------------------------------------*/
static void
genCpl (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);

  D (emit2 ("; genCpl", ""));

  aopOp (left, ic, false);
  aopOp (result, ic, true);

  genEor (ic, result->aop, left->aop, ASMOP_MONE);

  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genSub - generates code for subtraction                         */
/*-----------------------------------------------------------------*/
static void
genSub (const iCode *ic, asmop *result_aop, asmop *left_aop, asmop *right_aop)
{
  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  int size = result_aop->size;

  bool started = false;

  if (size == 2 && right_aop->type == AOP_STL) // Special case for rematerialized stack locations
    {
      if (!regDead (Y_IDX, ic) || !regDead (X_IDX, ic))
        UNIMPLEMENTED;

      genMove (ASMOP_X, left_aop, true, true, true, regDead (Z_IDX, ic));
      genMove (ASMOP_Y, right_aop, true, true, true, regDead (Z_IDX, ic));
      emit3 (A_SUBW, ASMOP_X, ASMOP_Y);
      genMove (result_aop, ASMOP_X, true, true, true, regDead (Z_IDX, ic));
      return;
    }

  for (int i = 0; i < size;)
    {
      bool maskedbyte = maskedtopbyte && (i + 1 == size);
      bool maskedword = maskedtopbyte && (i + 2 == size);

      bool xl_free = regDead (XL_IDX, ic) && left_aop->regs[XL_IDX] <= i && right_aop->regs[XL_IDX] <= i && (result_aop->regs[XL_IDX] < 0 || result_aop->regs[XL_IDX] >= i);
      bool xl_free2 = regDead (XL_IDX, ic) && left_aop->regs[XL_IDX] <= i + 1 && right_aop->regs[XL_IDX] <= i + 1 && (result_aop->regs[XL_IDX] < 0 || result_aop->regs[XL_IDX] >= i);
      //bool yl_free2 = regDead (YL_IDX, ic) && leftop->regs[YL_IDX] <= i + 1 && rightop->regs[YL_IDX] <= i + 1 && (result->aop->regs[YL_IDX] < 0 || result->aop->regs[YL_IDX] >= i);
      //bool yh_free2 = regDead (YH_IDX, ic) && leftop->regs[YH_IDX] <= i + 1 && rightop->regs[YH_IDX] <= i + 1 && (result->aop->regs[YH_IDX] < 0 || result->aop->regs[YH_IDX] >= i);
      //bool y_free2 = yl_free2 && yh_free2;

      if (!started && aopIsLitVal (right_aop, i, 1, 0x00)) // Skip lower bytes.
         {
           int end;
           for(end = i; end < size && aopIsLitVal (right_aop, end, 1, 0x00); end++);
           genMove_o (result_aop, i, left_aop, i, end - i, xl_free, false, false, false, true);
           i = end;
           continue;
         }

      if (i + 1 < size && !started && !maskedword && aopIsLitVal (left_aop, i, 2, 0) && aopIsAcc16 (result_aop, i)) // Use negw
        {
          genMove_o (result_aop, i, right_aop, i, 2, xl_free2, false, false, false, true);
          emit3_o (A_NEGW, result_aop, i, 0, 0);
          i += 2;
          started = true;
          continue;
        }
      else if (i + 1 < size && !started && !maskedword && aopSame (result_aop, i, left_aop, i, 2) && aopIsOp16_1 (left_aop, i) && // Use in-place incw
        aopIsLitVal (right_aop, i, 2, 0xffff))
        {
          emit3_o (A_INCW, left_aop, i, 0, 0);
          i += 2;
          started = true;
          continue;
        }
      else if (i + 1 < size && !started && !maskedword && aopSame (result_aop, i, left_aop, i, 2) && aopOnStackNotExt (left_aop, i, 2) && // Use in-place incw
        aopIsLitVal (right_aop, i, 2, 1))
        {
          emit3_o (A_DECW, left_aop, i, 0, 0);
          i += 2;
          started = true;
          continue;
        }
      else if (i + 1 < size && !started && !maskedword && aopInReg (result_aop, i, Y_IDX) && // Use incw y
        aopIsLitVal (right_aop, i, 2, 0xffff))
        {
          genMove (ASMOP_Y, left_aop, xl_free2, false, true, false);
          emit3 (A_INCW, ASMOP_Y, 0);
          i += 2;
          started = true;
          continue;
        }
      else if (i + 1 < size && started && !maskedword && aopSame (result_aop, i, left_aop, i, 2) && aopIsOp16_1 (left_aop, i) && // Use in-place adcw / sbcw
        (aopIsLitVal (right_aop, i, 2, 0) || aopIsLitVal (right_aop, i, 2, 0xffff)))
        {
          emit3_o (aopIsLitVal (right_aop, i, 2, 0) ? A_SBCW : A_ADCW, left_aop, i, 0, 0);
          i += 2;
          started = true;
          continue;
        }

      if (!started && !maskedword && aopSame (result_aop, i, left_aop, i, 1) && aopIsOp8_1 (left_aop, i) && // Use in-place inc / dec
         (aopIsLitVal (right_aop, i, 1, 1) || aopIsLitVal (right_aop, i, 1, 0xffff)) &&
         !aopAre16_2 (left_aop, i, right_aop, i)) // Fall throught o 16-bit operation below, if that is more efficient than two 8-bit ones.
         {
           emit3_o (aopIsLitVal (right_aop, i, 1, 1) ? A_DEC : A_INC, left_aop, i, 0, 0);
           i++;
           started = true;
           continue;
         }

      if (i + 1 < size && !maskedword && (aopIsOp16_2 (right_aop, i) || aopIsAcc16 (right_aop, i)) &&
        (aopInReg (left_aop, i, X_IDX) && regDead (X_IDX, ic) || aopInReg (left_aop, i, Y_IDX) && regDead (Y_IDX, ic) || aopInReg (left_aop, i, Z_IDX) && regDead (Z_IDX, ic) && !aopInReg (right_aop, i, X_IDX)))
        {
          if (aopIsLitVal (right_aop, i, 2, 0xffff))
            emit3_o (A_INCW, left_aop, i, 0, 0);
          else
            emit3sub_o (started ? A_SBCW : A_SUBW, left_aop, i, right_aop, i);
          genMove_o (result_aop, i, left_aop, i, 2, false, false, false, false, i + 2 == size);
          started = true;
          i += 2;
        }
      else if (i + 1 < size && !maskedword && (aopIsOp16_2 (right_aop, i) || aopIsAcc16 (right_aop, i)) &&
        (aopInReg (result_aop, i, X_IDX) && regDead (X_IDX, ic) && left_aop->regs[XL_IDX] <= i + 1 && left_aop->regs[XH_IDX] <= i + 1 && right_aop->regs[XL_IDX] < i && right_aop->regs[XH_IDX] < i ||
          aopInReg (result_aop, i, Y_IDX) && regDead (Y_IDX, ic) && left_aop->regs[YL_IDX] <= i + 1 && left_aop->regs[YH_IDX] <= i + 1 && right_aop->regs[YL_IDX] < i && right_aop->regs[YH_IDX] < i ||
          aopInReg (result_aop, i, Z_IDX) && regDead (Z_IDX, ic) && !aopInReg (right_aop, i, X_IDX) && left_aop->regs[ZL_IDX] <= i + 1 && left_aop->regs[ZH_IDX] <= i + 1 && right_aop->regs[ZL_IDX] < i && right_aop->regs[ZH_IDX] < i))
        {
          genMove_o (result_aop, i, left_aop, i, 2, false, false, false, false, !started);
          emit3sub_o (started ? A_SBCW : A_SUBW, result_aop, i, right_aop, i);
          started = true;
          i += 2;
        }
      else if (i + 1 < size && !maskedword && aopIsOp16_2 (right_aop, i) &&
        regDead (Y_IDX, ic) && left_aop->regs[YL_IDX] <= i + 1 && left_aop->regs[YH_IDX] <= i + 1 && right_aop->regs[YL_IDX] < i && right_aop->regs[YH_IDX] < i && result_aop->regs[YL_IDX] < 0 && result_aop->regs[YH_IDX] < 0 &&
        (aopOnStack (result_aop, i, 2) || result_aop->type == AOP_DIR))
        {
          genMove_o (ASMOP_Y, 0, left_aop, i, 2, false, false, true, false, !started);
          if (aopIsLitVal (right_aop, i, 2, 0xffff))
            emit3 (A_INCW, ASMOP_Y, 0);
          else
            emit3sub_o (started ? A_SBCW : A_SUBW, ASMOP_Y, 0, right_aop, i);
          genMove_o (result_aop, i, ASMOP_Y, 0, 2, false, false, true, false, i + 2 == size);
          started = true;
          i += 2;
        }
      else
        {
          if (aopIsOp8_2 (right_aop, i))
            {
              bool pushed_xl = false;
              asmop *lop;
              int lopoffset;
              if (aopSame (result_aop, i, left_aop, i, 1) && aopIsAcc8 (left_aop, i))
                {
                 lop = left_aop;
                 lopoffset = i;
                }
              else
                {
                  if (!xl_free)
                    {
                      push (ASMOP_XL, 0, 1);
                      pushed_xl = true;
                    }
                  lop = ASMOP_XL;
                  lopoffset = 0;
                  genMove_o (ASMOP_XL, 0, left_aop, i, 1, true, false, false, false, !started);
                }
              if (!started && (aopIsLitVal (right_aop, i, 1, 1) || aopIsLitVal (right_aop, i, 1, -1))) // Use inc / dec
                emit3 (aopIsLitVal (right_aop, i, 1, -11) ? A_INC : A_DEC, ASMOP_XL, 0);
              else
                emit3sub_o (started ? A_SBC : A_SUB, lop, lopoffset, right_aop, i);
              if (maskedbyte)
                {
                  emit2 ("and", "xl, #0x%02x", topbytemask);
                  cost (2, 1);
                }
              genMove_o (result_aop, i, lop, lopoffset, 1, true, false, false, false, i + 1 == size);
              if (pushed_xl)
                pop (ASMOP_XL, 0, 1);
            }
          else if (!xl_free)
            UNIMPLEMENTED;
          else if (aopIsOp8_1 (right_aop, i))
            {
              push (right_aop, i, 1);
              genMove_o (ASMOP_XL, 0, left_aop, i, 1, true, false, false, false, !started);
              emit2 (started ? "sbc" : "sub", "xl, (0, sp)");
              cost (2, 1);
              if (maskedbyte)
                {
                  emit2 ("and", "xl, #0x%02x", topbytemask);
                  cost (2, 1);
                }
              adjustStack (1, false, false);
              genMove_o (result_aop, i, ASMOP_XL, 0, 1, true, false, false, false, i + 1 == size);
            }
          else
            UNIMPLEMENTED;
          started = true;
          i++;
        }
    }
}

/*-----------------------------------------------------------------*/
/* genUminus - generates code for unary minus                      */
/*-----------------------------------------------------------------*/
static void
genUminus (const iCode *ic)
{
  operand *result;
  operand *left;

  D (emit2 ("; genUminus", ""));

  aopOp (ic->left, ic, false);
  aopOp (ic->result, ic, true);

  result = IC_RESULT (ic);
  left = IC_LEFT (ic);

  if (IS_FLOAT (operandType (ic->left)))
    {
      struct asmop topbit;
      topbit.type = AOP_LIT;
      topbit.size = 4;
      memset (topbit.regs, -1, sizeof(topbit.regs));
      topbit.aopu.aop_lit = constVal ("0x80000000");
      topbit.valinfo.anything = false;
      topbit.valinfo.min = topbit.valinfo.max = topbit.valinfo.knownbits = 0x80000000ull;
      topbit.valinfo.knownbitsmask =  ~0ull;

      genEor (ic, result->aop, left->aop, &topbit);
    }
  else
    genSub (ic, result->aop, ASMOP_ZERO, left->aop);

  freeAsmop (left);
  freeAsmop (result);
}

static void
saveRegsForCall (const iCode * ic)
{
  if (G.saved && !regalloc_dry_run)
    return;

  if (!regDead (X_IDX, ic))
    if (regDead (XH_IDX, ic))
      push (ASMOP_XL, 0, 1);
    else if (regDead (XL_IDX, ic))
      push (ASMOP_XH, 0, 1);
    else
      push (ASMOP_X, 0, 2);

  if (!regDead (Y_IDX, ic))
    push (ASMOP_Y, 0, 2);

  if (!regDead (Z_IDX, ic))
    push (ASMOP_Z, 0, 2);

  G.saved = true;
}

/*-----------------------------------------------------------------*/
/* genIpush - generate code for pushing this gets a little complex */
/*-----------------------------------------------------------------*/
static void
genIpush (const iCode * ic)
{
  operand *left = IC_LEFT (ic);
  iCode *walk;

  D (emit2 ("; genIPush", ""));

  if (!ic->parmPush)
    {
      wassertl (0, "Encountered an unsupported spill push.");
      return;
    }

  /* Caller saves, and this is the first iPush. */
  /* Scan ahead until we find the function that we are pushing parameters to.
     Count the number of addSets on the way to figure out what registers
     are used in the send set.
   */
  for (walk = ic->next; walk->op != CALL && walk->op != PCALL; walk = walk->next);
  if (!G.saved  && !regalloc_dry_run /* Cost is counted at CALL or PCALL instead */ )
    saveRegsForCall (walk);

  // Then do the push
  aopOp (left, ic, false);

  int size = left->aop->size;
  int y_val = -1;
  for (int i = size - 1; i >= 0;)
    {
      if (i > 0 && aopIsLitVal (left->aop, i - 1, 2, 0x0000) && regDead (Y_IDX, ic) &&
        (left->aop->regs[YL_IDX] < 0 || left->aop->regs[YL_IDX] >= i) && (left->aop->regs[YH_IDX] < 0 || left->aop->regs[YH_IDX] >= i))
        {
          if (y_val)
            emit3 (A_CLRW, ASMOP_Y, 0);
          y_val = 0;
          push (ASMOP_Y, 0, 2);
          i -= 2;
        }
      else if (i > 0 && (aopIsOp16_1 (left->aop, i - 1) || left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD))
        {
          push (left->aop, i - 1, 2);
          i -= 2;
        }
      else if (aopIsOp8_1 (left->aop, i) || left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD)
        {
          push (left->aop, i, 1);
          i--;
        }
      else if (i == 1 && size == 2 && regDead (Y_IDX, ic))
        {
          genMove (ASMOP_Y, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
          push (ASMOP_Y, 0, 2);
          i -= 2;
        }
      else
        {
          UNIMPLEMENTED;
          i--;
        }
    }

  freeAsmop (IC_LEFT (ic));
}

/*-----------------------------------------------------------------*/
/* genPointerPush - generate code for pushing                      */
/*-----------------------------------------------------------------*/
static void
genPointerPush (const iCode *ic)
{
  operand *left = IC_LEFT (ic);
  iCode *walk;

  D (emit2 ("; genPointerPush", ""));

  // Like in genIpush above.
  for (walk = ic->next; walk->op != CALL && walk->op != PCALL; walk = walk->next);
  if (!G.saved  && !regalloc_dry_run /* Cost is counted at CALL or PCALL instead */ )
    saveRegsForCall (walk);

  /* then do the push */
  aopOp (left, ic, false);

  wassertl (IC_RIGHT (ic), "IPUSH_VALUE_AT_ADDRESS without right operand");
  wassertl (IS_OP_LITERAL (IC_RIGHT (ic)), "IPUSH_VALUE_AT_ADDRESS with non-literal right operand");

  int size = getSize (operandType (left)->next);

  int offset = operandLitValue (ic->right);

  if (regDead (Z_IDX, ic) || aopInReg (left->aop, 0, Z_IDX))
    {
      //bool use_y = (regDead (Y_IDX, ic) && !aopInReg (IC_LEFT (ic)->aop, 0, Z_IDX) || aopInReg (IC_LEFT (ic)->aop, 0, Y_IDX)) && offset >= 0 && size - 1 + offset <= 255;
      genMove (ASMOP_Z, IC_LEFT (ic)->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      for(int i = size - 1; i >= 0;)
        {
          int o = i + offset;
          if (i > 0)
            {
              emit2 ("pushw", "(%d, z)", o - 1);
              cost (3, 1);
              G.stack.pushed += 2;
              updateCFA ();
              i -= 2;
            }
          else if (regDead (XL_IDX, ic))
            {
              emit2 ("ld", "xl, (%d, z)", o);
              cost (3, 1);
              push (ASMOP_XL, 0, 1);
              i -= 1;
            }
          else
            {
              emit2 ("pushw", "(%d, z)", o - 1);
              emit2 ("addw", "sp, #1");
              cost (5, 2);
              G.stack.pushed++;
              updateCFA ();
              i -= 1;
            }
        }
    }
  else if ((regDead (Y_IDX, ic) || aopInReg (left->aop, 0, Y_IDX)) && regDead (XL_IDX, ic) && offset + size - 1 <= 255)
    {
      genMove (ASMOP_Y, ic->left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      for(int i = size - 1; i >= 0;)
        {
          int o = i + offset;
          emit2 ("ld", "xl, (%d, y)", o);
          cost (2, 1);
          push (ASMOP_XL, 0, 1);
          i -= 1;
        }
    }
  else
    UNIMPLEMENTED;

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
  int prestackadjust = 0;
  bool tailjump = false;

  D (emit2 ("; genCall", ""));

  saveRegsForCall (ic);

  operand *left = IC_LEFT (ic);

  const bool bigreturn = (getSize (ftype->next) > 4) || IS_STRUCT (ftype->next);   // Return value of big type or returning struct or union.
  const bool SomethingReturned = (IS_ITEMP (IC_RESULT (ic)) &&
                       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir))
                       || IS_TRUE_SYMOP (IC_RESULT (ic));

  aopOp (left, ic, false);
  if (SomethingReturned && !bigreturn)
    aopOp (ic->result, ic, true);

  if (bigreturn)
    {
      wassertl (IC_RESULT (ic), "Unused return value in call to function returning large type.");

      aopOp (ic->result, ic, true);

      if (IC_RESULT (ic)->aop->type != AOP_STK)
        UNIMPLEMENTED;

      if (!f8IsParmInCall (ftype, "y") && (ic->op != PCALL || left->aop->regs[YL_IDX] < 0 && left->aop->regs[YH_IDX] < 0))
        {
          emit2 ("ldw", "y, sp");
          emit2 ("addw", "y, #%d", IC_RESULT (ic)->aop->aopu.bytes[0].byteu.stk + G.stack.pushed);
          cost (3 + (IC_RESULT (ic)->aop->aopu.bytes[getSize (ftype->next) - 1].byteu.stk + G.stack.pushed > 127), 2);
          spillReg (C_IDX);
          push (ASMOP_Y, 0, 2);
        }
      else if (!f8IsParmInCall (ftype, "x") && (ic->op != PCALL || left->aop->regs[XL_IDX] < 0 && left->aop->regs[XH_IDX] < 0))
        {
          emit2 ("ldw", "x, sp");
          emit2 ("addw", "x, #%d", IC_RESULT (ic)->aop->aopu.bytes[0].byteu.stk + G.stack.pushed);
          cost (5 + (IC_RESULT (ic)->aop->aopu.bytes[getSize (ftype->next) - 1].byteu.stk + G.stack.pushed > 127), 2);
          spillReg (C_IDX);
          push (ASMOP_X, 0, 2);
        }
      else if (!f8IsParmInCall (ftype, "z") && (ic->op != PCALL || left->aop->regs[ZL_IDX] < 0 && left->aop->regs[ZH_IDX] < 0 && !f8_extend_stack))
        {
          emit2 ("ldw", "z, sp");
          emit2 ("addw", "z, #%d", IC_RESULT (ic)->aop->aopu.bytes[0].byteu.stk + G.stack.pushed);
          cost (5 + (IC_RESULT (ic)->aop->aopu.bytes[getSize (ftype->next) - 1].byteu.stk + G.stack.pushed > 127), 2);
          spillReg (C_IDX);
          push (ASMOP_Z, 0, 2);
        }
      else
        UNIMPLEMENTED;

      freeAsmop (IC_RESULT (ic));
    }
  // Check if we can do tail call optimization.
  else if (currFunc && !IFFUNC_ISISR (currFunc->type) &&
    (!SomethingReturned ||
    aopInReg (IC_RESULT (ic)->aop, 0,
    aopRet (ftype)->aopu.bytes[0].byteu.reg->rIdx) &&
      (IC_RESULT (ic)->aop->size <= 1 || aopInReg (IC_RESULT (ic)->aop, 1, aopRet (ftype)->aopu.bytes[1].byteu.reg->rIdx)) &&
      (IC_RESULT (ic)->aop->size <= 2 || aopInReg (IC_RESULT (ic)->aop, 2, aopRet (ftype)->aopu.bytes[2].byteu.reg->rIdx)) &&
      (IC_RESULT (ic)->aop->size <= 3 || aopInReg (IC_RESULT (ic)->aop, 3, aopRet (ftype)->aopu.bytes[3].byteu.reg->rIdx)) &&
      IC_RESULT (ic)->aop->size <= 4) &&
    !ic->parmBytes && !bigreturn &&
    (!isFuncCalleeStackCleanup (currFunc->type) || !ic->parmEscapeAlive && !optimize.codeSize && ic->op == CALL) &&
    !ic->localEscapeAlive &&
    !(ic->op == PCALL && (left->aop->type == AOP_STK || left->aop->type == AOP_REGSTK))) // Avoid destroying the pointer that we need to call
    {
      int limit = 16; // Avoid endless loops in the code putting us into an endless loop here.

      if (isFuncCalleeStackCleanup (currFunc->type))
        {
           const bool caller_bigreturn = currFunc->type->next && (getSize (currFunc->type->next) > 4) || IS_STRUCT (currFunc->type->next);
           int caller_stackparmbytes = caller_bigreturn * 2;
           for (value *caller_arg = FUNC_ARGS(currFunc->type); caller_arg; caller_arg = caller_arg->next)
             {
               wassert (caller_arg->sym);
               if (!SPEC_REGPARM (caller_arg->etype))
                 caller_stackparmbytes += getSize (caller_arg->sym->type);
             }
           prestackadjust += caller_stackparmbytes;
        }

      for (const iCode *nic = ic->next; nic && --limit;)
        {
          const symbol *targetlabel = 0;

          if (nic->op == LABEL)
            ;
          else if (nic->op == GOTO) // We dont have ebbi here, so we can't just use eBBWithEntryLabel (ebbi, ic->label). Search manually.
            targetlabel = IC_LABEL (nic);
          else if (nic->op == RETURN && (!IC_LEFT (nic) || SomethingReturned && IC_RESULT (ic)->key == IC_LEFT (nic)->key))
            targetlabel = returnLabel;
          else if (nic->op == ENDFUNCTION)
            {
              if (OP_SYMBOL (IC_LEFT (nic))->stack <= (optimize.codeSize ? 250 : 510))
                if (!isFuncCalleeStackCleanup (currFunc->type) || prestackadjust <= 250)
                  {
                    prestackadjust += OP_SYMBOL (IC_LEFT (nic))->stack;
                    tailjump = true;
                    break;
                  }
              prestackadjust = 0;
              break;
            }
          else
            {
              prestackadjust = 0;
              break;
            }

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
                {
                  prestackadjust = 0;
                  tailjump = false;
                  break;
                }

              nic = nnic;
            }
          else
            nic = nic->next;
        }
    }

  const bool jump = tailjump || !ic->parmBytes && !bigreturn && IFFUNC_ISNORETURN (ftype);

  if (ic->op == PCALL)
    {
      bool xl_free = !f8IsParmInCall (ftype, "xl");
      bool xh_free = !f8IsParmInCall (ftype, "xh");
      bool x_free = !f8IsParmInCall (ftype, "x");
      bool y_free = !f8IsParmInCall (ftype, "y");
      bool z_free = !f8IsParmInCall (ftype, "z");
      adjustStack (prestackadjust, xl_free, y_free);
      if (y_free)
        {
          genMove (ASMOP_Y, left->aop, xl_free, xh_free, y_free, z_free);
          emit2 (jump ? "jp" : "call", "y");
          cost (1, 1);
        }
      else if (x_free)
        {
          genMove (ASMOP_X, left->aop, xl_free, xh_free, y_free, z_free);
          emit2 (jump ? "jp" : "call", "x");
          cost (2, 2);
        }
      else if (z_free)
        {
          genMove (ASMOP_Z, left->aop, xl_free, xh_free, y_free, z_free);
          emit2 (jump ? "jp" : "call", "z");
          cost (2, 2);
        }
      else
        UNIMPLEMENTED;
      spillAllRegs (); // Todo: Support __preserves_regs
    }
  else
    {
      if (prestackadjust && isFuncCalleeStackCleanup (currFunc->type) && !IFFUNC_ISNORETURN (ftype)) // Copy return value into correct location on stack for tail call optimization.
        {
          UNIMPLEMENTED;
        }

      adjustStack (prestackadjust, !f8IsParmInCall (ftype, "xl"), !f8IsParmInCall (ftype, "y"));

      if (IS_LITERAL (etype))
        emit2 (jump ? "jp" : "call", "#0x%04X", ulFromVal (OP_VALUE (left)));
      else
        emit2 (jump ? "jp" : "call", "#%s",
          (OP_SYMBOL (left)->rname[0] ? OP_SYMBOL (left)->rname : OP_SYMBOL (left)->name));
      cost (3, 1);
      spillAllRegs (); // Todo: Support __preserves_regs
    }

  freeAsmop (left);
  G.stack.pushed += prestackadjust;

  // Adjust the stack for parameters if required.
  if (ic->parmBytes || bigreturn)
    {
      const bool xl_free = !aopRet (ftype) || aopRet (ftype)->regs[XL_IDX] < 0;
      const bool y_free = !aopRet (ftype) || (aopRet (ftype)->regs[YL_IDX] < 0 && aopRet (ftype)->regs[YH_IDX] < 0);
      if (IFFUNC_ISNORETURN (ftype) || isFuncCalleeStackCleanup (ftype))
        {
          G.stack.pushed -= ic->parmBytes + bigreturn * 2;
          updateCFA ();
        }
      else
        adjustStack (ic->parmBytes + bigreturn * 2, xl_free, y_free);
    }


  const bool result_in_frameptr = f8_extend_stack && SomethingReturned && !bigreturn && (aopRet (ftype)->regs[ZL_IDX] >= 0 || aopRet (ftype)->regs[ZH_IDX] >= 0);

  asmop *result = IC_RESULT (ic)->aop;

  if (result_in_frameptr)
    {
      UNIMPLEMENTED;

      goto restore;
    }

  if (f8_extend_stack)
    pop (ASMOP_Z, 0, 2);

  /* if we need assign a result value */
  if (SomethingReturned && !bigreturn)
    {
      wassert (getSize (ftype->next) >= 1 && getSize (ftype->next) <= 4);
      genMove (result, aopRet (ftype), true, true, true, !f8_extend_stack);
    }

restore:
  if (SomethingReturned && !bigreturn)
    freeAsmop (IC_RESULT (ic));

  // Restore regs.
  if (!regDead (Z_IDX, ic) && !f8_extend_stack)
    if (regDead (ZH_IDX, ic))
      UNIMPLEMENTED;
    else if (regDead (ZL_IDX, ic))
      UNIMPLEMENTED;
    else
      pop (ASMOP_Z, 0, 2);

  if (!regDead (Y_IDX, ic))
    if (regDead (YH_IDX, ic))
      UNIMPLEMENTED;
    else if (regDead (YL_IDX, ic))
      UNIMPLEMENTED;
    else
      pop (ASMOP_Y, 0, 2);

  if (!regDead (X_IDX, ic))
    if (regDead (XH_IDX, ic))
      pop (ASMOP_XL, 0, 1);
    else if (regDead (XL_IDX, ic))
      pop (ASMOP_XH, 0, 1);
    else
      pop (ASMOP_X, 0, 2);

  G.saved = false;
}

/*---------------------------------------------------------------------*/
/* genCritical - mask interrupts until important block completes       */
/*---------------------------------------------------------------------*/

static void
genCritical (iCode *ic)
{
  push (ASMOP_X, 0, 2);
  push (ASMOP_Y, 0, 2);
  emit2 ("ldw", "y, #0x0010");
  cost (3, 1);
  emit3 (A_CLRW, ASMOP_X, 0);
  emit2 ("xchw", "x, (y)");
  cost (1, 1);
  pop (ASMOP_Y, 0, 2);
  emit2 ("xchw", "x, (0, sp)");
  cost (1, 1);
}

static void
genEndCritical (iCode *ic)
{
  emit2 ("xchw", "y, (0, sp)");
  emit2 ("ldw", "0x0010, y");
  cost (5, 2);
  pop (ASMOP_Y, 0, 2);
}

/*-----------------------------------------------------------------*/
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void
genFunction (iCode *ic)
{
  const symbol *sym = OP_SYMBOL_CONST (IC_LEFT (ic));
  sym_link *ftype = operandType (IC_LEFT (ic));
  bool bigreturn;

  G.stack.pushed = 0;
  G.stack.param_offset = 0;

  /* create the function header */
  emit2 (";", "-----------------------------------------");
  emit2 (";", " function %s", sym->name);
  emit2 (";", "-----------------------------------------");

  D (emit2 (";", f8_assignment_optimal ? "Register assignment is optimal." : "Register assignment might be sub-optimal."));
  D (emit2 (";", "Stack space usage: %d bytes.", sym->stack));

  emit2 ("", "%s:", sym->rname);
  genLine.lineCurr->isLabel = 1;

  if (IFFUNC_ISNAKED(ftype))
  {
      updateCFA(); //ensure function has at least 1 CFA record
      emit2(";", "naked function: no prologue.");
      return;
  }

  if (IFFUNC_ISISR (sym->type))
    {
      push (ASMOP_F, 0, 1);
      push (ASMOP_X, 0, 2);
      push (ASMOP_Y, 0, 2);
      push (ASMOP_Z, 0, 2);
    }

  if (IFFUNC_ISCRITICAL (ftype))
      genCritical (0);

  if (f8_extend_stack) // Setup for extended stack access.
    {
      G.stack.size = f8_call_stack_size + (sym->stack ? sym->stack : 0);
      D (emit2 (";", "Setup z as frame pointer."));
      emit2 ("ldw", "z, sp");
      cost (2, 1);
    }

  bigreturn = (getSize (ftype->next) > 4) || IS_STRUCT (ftype->next);
  G.stack.param_offset += bigreturn * 2;

  if (options.debug && !regalloc_dry_run)
    debugFile->writeFrameAddress (NULL, &f8_regs[SP_IDX], 1);

  /* adjust the stack for the function */
  {
    int fadjust = -sym->stack;
    adjustStack (fadjust, !f8IsParmInCall (sym->type, "xl"), !f8IsParmInCall (sym->type, "y"));
  }
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction (iCode *ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));

  D (emit2 ("; genEndFunction", ""));

  wassert (!regalloc_dry_run);

  if (IFFUNC_ISNAKED(sym->type))
  {
      D (emit2 (";", "naked function: no epilogue."));
      if (options.debug && currFunc && !regalloc_dry_run)
        debugFile->writeEndFunction (currFunc, ic, 0);
      return;
  }

  const bool bigreturn = (getSize (sym->type->next) > 4) || IS_STRUCT (sym->type->next);
  int stackparmbytes = bigreturn * 2;
  for (value *arg = FUNC_ARGS(sym->type); arg; arg = arg->next)
    {
      wassert (arg->sym);
      if (!SPEC_REGPARM (arg->etype))
        stackparmbytes += getSize (arg->sym->type);
    }

  int poststackadjust = isFuncCalleeStackCleanup (sym->type) ? stackparmbytes : 0;

  bool xl_free = !aopRet (sym->type) || aopRet (sym->type)->regs[XL_IDX] < 0;
  bool y_free = !aopRet (sym->type) || (aopRet (sym->type)->regs[YL_IDX] < 0 && aopRet (sym->type)->regs[YH_IDX] < 0);

  if (sym->stack)
    adjustStack (sym->stack, xl_free, y_free);

  if (poststackadjust)
    wassert (0);

  if (IFFUNC_ISCRITICAL (sym->type))
    genEndCritical (0);

  /* if debug then send end of function */
  if (options.debug && currFunc && !regalloc_dry_run)
    debugFile->writeEndFunction (currFunc, ic, 1);
        
  if (IFFUNC_ISISR (sym->type))
    {
      pop (ASMOP_Z, 0, 2);
      pop (ASMOP_Y, 0, 2);
      pop (ASMOP_X, 0, 2);
      pop (ASMOP_F, 0, 1);
      emit2 ("reti", "");
      cost (1, 1);
    }
  else
    {
      emit2 ("ret", "");
      cost (1, 1);
    }

  wassertl (!G.stack.pushed, "Unbalanced stack.");

  D (emit2 (";", "Total %s function size at codegen: %u bytes.", sym->name, (unsigned int)regalloc_dry_run_cost_bytes));
}

/*-----------------------------------------------------------------*/
/* genReturn - generate code for return statement                  */
/*-----------------------------------------------------------------*/
static void
genReturn (const iCode *ic)
{
  operand *left = IC_LEFT (ic);
  int size;

  D (emit2 ("; genReturn", ""));

  /* if we have no return value then
     just generate the "ret" */
  if (!IC_LEFT (ic))
    goto jumpret;

  /* we have something to return then
     move the return value into place */
  aopOp (left, ic, false);
  size = left->aop->size;

  if (IS_STRUCT (operandType (IC_LEFT (ic))))
    goto bigreturn;

  switch (size)
    {
    case 0:
      break;
    case 1:
    case 2:
    case 3:
    case 4:
      genMove (aopRet (currFunc->type), left->aop, true, true, true, true);
      break;
    default:
bigreturn:

      wassertl (size < 256, "Return not implemented for return value of this size.");

      for(int i = 0; i < size; i++)
        if (aopInReg (left->aop, i, YL_IDX) || aopInReg (left->aop, i, YH_IDX))
          UNIMPLEMENTED;

      unsigned int o = G.stack.pushed + 2;

      if (o <= 255)
        {
          emit2 ("ldw", "y, (0x%02x, sp)", o);
          cost (2, 1);
        }
      else
        UNIMPLEMENTED;

      // Clear xl first.
      for(int i = 0; i < size; i++)
        if (aopInReg (left->aop, i, XL_IDX))
          {
            emit2 ("ld", i ? "(%d, y), xl" : "(y), xl", i);
            cost (1 + (bool)i, 1);
            break;
          }

      for(int i = 0; i < size;)
        {
          if (/*i + 1 < size && aopInReg (left->aop, i, X_IDX)*/ false ||
            i + 1 < size && (aopOnStack (left->aop, i, 2) || left->aop->type == AOP_DIR) && regDead (X_IDX, ic) && left->aop->regs[XL_IDX] <= i + 1 && left->aop->regs[XH_IDX] <= i + 1)
            {
              genMove_o (ASMOP_X, 0, left->aop, i, 2, true, false, false, true, true);
              emit2 ("ldw", "(%d, y), x", i);
              cost (3, 1);
              i += 2;
            }
          else if (!aopInReg (left->aop, i, XL_IDX))
            {
              genMove_o (ASMOP_XL, 0, left->aop, i, 1, true, false, false, false, true);
              emit2 ("ld", i ? "(%d, y), xl" : "(y), xl", i);
              cost (1 + (bool)i, 1);
              i++;
            }
          else // xl, already stored earlier.
            i++;
        }
    }

  freeAsmop (left);

jumpret:
  /* generate a jump to the return label
     if the next is not the return statement */
  if (!(ic->next && ic->next->op == LABEL && IC_LABEL (ic->next) == returnLabel))
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

  if (options.debug && !regalloc_dry_run)
    debugFile->writeLabel (IC_LABEL (ic), ic);

  emitLabel (IC_LABEL (ic));
  spillAllRegs ();
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a jump                                      */
/*-----------------------------------------------------------------*/
static void
genGoto (const iCode *ic)
{
  D (emit2 ("; genGoto", ""));

  emitJP(IC_LABEL (ic), 1.0f);
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

  asmop *leftop;
  asmop *rightop;

  sym_link *resulttype = operandType (result);
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);

  D (emit2 ("; genPlus", ""));

  aopOp (ic->left, ic, false);
  aopOp (ic->right, ic, false);
  aopOp (ic->result, ic, true);

  int size = result->aop->size;

  /* Swap if left is literal. */
  if (left->aop->type == AOP_LIT || right->aop->type != AOP_LIT && left->aop->type == AOP_IMMD) // todo: Swap in more cases when right in reg, left not. Swap individually per-byte.
    {
      operand *t = right;
      right = left;
      left = t;
    }

  leftop = left->aop;
  rightop = right->aop;

  for (int i = 0, started = false; i < size;)
    {
      bool maskedbyte = maskedtopbyte && (i + 1 == size);
      bool maskedword = maskedtopbyte && (i + 2 == size);

      bool xl_free = regDead (XL_IDX, ic) && leftop->regs[XL_IDX] <= i && rightop->regs[XL_IDX] <= i && (result->aop->regs[XL_IDX] < 0 || result->aop->regs[XL_IDX] >= i);
      bool xl_free2 = regDead (XL_IDX, ic) && leftop->regs[XL_IDX] <= i + 1 && rightop->regs[XL_IDX] <= i + 1 && (result->aop->regs[XL_IDX] < 0 || result->aop->regs[XL_IDX] >= i);
      bool yl_free2 = regDead (YL_IDX, ic) && leftop->regs[YL_IDX] <= i + 1 && rightop->regs[YL_IDX] <= i + 1 && (result->aop->regs[YL_IDX] < 0 || result->aop->regs[YL_IDX] >= i);
      bool yh_free2 = regDead (YH_IDX, ic) && leftop->regs[YH_IDX] <= i + 1 && rightop->regs[YH_IDX] <= i + 1 && (result->aop->regs[YH_IDX] < 0 || result->aop->regs[YH_IDX] >= i);
      bool y_free2 = yl_free2 && yh_free2;

      // Special case for rematerializing sums
      if (!started && i == size - 2 && !maskedword && (leftop->type == AOP_IMMD && rightop->type == AOP_LIT) &&
        (aopInReg (result->aop, i, Y_IDX) || aopInReg (result->aop, i, X_IDX) || aopInReg (result->aop, i, Z_IDX) || y_free2 && aopOnStack (result->aop, i, 2)))
        {
          unsigned offset = byteOfVal (right->aop->aopu.aop_lit, i + 1) * 256 + byteOfVal (right->aop->aopu.aop_lit, i);
          asmop *taop = aopInReg (result->aop, i, X_IDX) ? ASMOP_X : (aopInReg (result->aop, i, Z_IDX) ? ASMOP_Z : ASMOP_Y);
          emit2 ("ldw", "%s, %s+%d", aopGet2 (taop, 0), aopGet2 (leftop, i), offset);
          cost (3 + !aopInReg (taop, 0, Y_IDX), 1);
          genMove_o (result->aop, i, taop, 0, 2, xl_free2, false, y_free2, false, true);
          i += 2;
          continue;
        }
      else if (!started && !i && i == size - 2 && !maskedword &&
        (leftop->type == AOP_STL && aopIsOp16_2 (rightop, i) || rightop->type == AOP_STL && aopIsOp16_2 (leftop, i)) &&
        (aopIsAcc16 (result->aop, i) && !(aopInReg (result->aop, i, Z_IDX) && (aopInReg (leftop, i, X_IDX) || aopInReg (rightop, i, X_IDX))) || regDead (Y_IDX, ic)))
        {
          struct asmop *taop = regDead (Y_IDX, ic) ? ASMOP_Y : result->aop; // We save 2 bytes by using y, which is usually worth the 1 byte extra ldw.
          struct asmop *stlop = leftop->type == AOP_STL ? leftop : rightop;
          struct asmop *otherop = leftop->type == AOP_STL ? rightop : leftop;
          if (aopIsAcc16 (result->aop, i) && otherop->regs[result->aop->aopu.bytes[0].byteu.reg->rIdx] < 0 && otherop->regs[result->aop->aopu.bytes[1].byteu.reg->rIdx] < 0 && !(aopInReg (result->aop, i, Z_IDX) && aopInReg (otherop, i, X_IDX)))
            taop = result->aop;
          if (aopRS (taop) && (taop->aopu.bytes[0].in_reg && otherop->regs[taop->aopu.bytes[0].byteu.reg->rIdx] >= 0 || taop->aopu.bytes[1].in_reg && otherop->regs[taop->aopu.bytes[1].byteu.reg->rIdx] >= 0))
            UNIMPLEMENTED;
          genMove (taop, stlop, regDead (XL_IDX, ic) && otherop->regs[XL_IDX] < 0, regDead (XH_IDX, ic) && otherop->regs[XH_IDX] < 0, false, false);
          cost (1 + !aopInReg (taop, 0, Y_IDX), 1);
          if (!aopIsLitVal (otherop, i, 2, 0x0000))
            {
              wassert (!aopInReg (taop, 0, Z_IDX) || !aopInReg (otherop, i, X_IDX));
              emit3_o (A_ADDW, taop, 0, otherop, i);
              started = true;
            }
          genMove_o (result->aop, i, taop, 0, 2, xl_free2, false, y_free2, false, true);
          i += 2;
          continue;
        } 

      if (!started && aopIsLitVal (rightop, i, 1, 0x00)) // Skip lower bytes.
         {
           genMove_o (result->aop, i, leftop, i, 1, xl_free, false, false, false, true);
           i++;
           continue;
         }

       if (i + 1 < size && !started && !maskedword && aopSame (result->aop, i, leftop, i, 2) && aopIsOp16_1 (leftop, i) && // Use in-place incw
         aopIsLitVal (rightop, i, 2, 1))
         {
           emit3_o (A_INCW, leftop, i, 0, 0);
           i += 2;
           started = true;
           continue;
         }
       else if (i + 1 < size && !started && !maskedword && aopInReg (result->aop, i, Y_IDX) && // Use incw y
         aopIsLitVal (rightop, i, 2, 1))
         {
           genMove_o (ASMOP_Y, 0, leftop, i, 2, xl_free2, false, true, false, true);
           emit3 (A_INCW , ASMOP_Y, 0);
           i += 2;
           started = true;
           continue;
         }
       else if (i + 1 < size && started && !maskedword && aopSame (result->aop, i, leftop, i, 2) && aopIsOp16_1 (leftop, i) && // Use in-place adcw / sbcw
         (aopIsLitVal (rightop, i, 2, 0) || aopIsLitVal (rightop, i, 2, -1)))
         {
           emit3_o (aopIsLitVal (rightop, i, 2, 0) ? A_ADCW : A_SBCW, leftop, i, 0, 0);
           i += 2;
           started = true;
           continue;
         }

       if (!started && !maskedbyte && aopSame (result->aop, i, leftop, i, 1) && aopIsOp8_1 (leftop, i) && // Use in-place inc / dec
         (aopIsLitVal (rightop, i, 1, 1) || aopIsLitVal (rightop, i, 1, -1)))
         {
           emit3_o (aopIsLitVal (rightop, i, 1, 1) ? A_INC : A_DEC, leftop, i, 0, 0);
           i++;
           started = true;
           continue;
         }

       if (i + 1 < size && !maskedword && aopSame (result->aop, i, leftop, i, 2) && aopAre16_2(leftop, i, rightop, i)) // Use addw / adcw
         {
           if (!started && aopIsLitVal (rightop, i, 2, 1))
             emit3_o (A_INCW, result->aop, i, 0, 0);
           else if (!started && aopOnStackNotExt (result->aop, i, 2) && aopIsLitVal (rightop, i, 2, -1))
             emit3_o (A_DECW, result->aop, i, 0, 0);
           else if (started && aopIsLitVal (rightop, i, 2, 0x0000))
             emit3_o (A_ADCW, result->aop, i, 0, 0);
           else
             emit3_o (started ? A_ADCW : A_ADDW, result->aop, i, rightop, i);
           i += 2;
           started = true;
           continue;
         }
       else if (i + 1 < size && !maskedword && aopSame (result->aop, i, rightop, i, 2) && aopAre16_2(rightop, i, leftop, i))
         {
           wassert (!aopInReg (result->aop, i, Z_IDX) || !aopInReg (leftop, i, X_IDX));
           emit3_o (started ? A_ADCW : A_ADDW, result->aop, i, leftop, i);
           i += 2;
           started = true;
           continue;
         }
       else if (aopSame (result->aop, i, leftop, i, 2) && aopOnStackNotExt (result->aop, i, 2) && y_free2)
         {
           genMove_o (ASMOP_Y, 0, rightop, i, 2, xl_free2, false, true, false, !started);
           emit3_o (started ? A_ADCW : A_ADDW, result->aop, i, ASMOP_Y, 0);
           i += 2;
           started = true;
           continue;
         }

       if (i + 1 < size && !maskedword && aopIsAcc16 (result->aop, i) && (aopOnStack (leftop, i, 2) || leftop->type == AOP_DIR) && // Use incw / adcw in destination
         (!started && aopIsLitVal (rightop, i, 2, 0x0001) || started && aopIsLitVal (rightop, i, 2, 0x0000)))
         {
           genMove_o (result->aop, i, leftop, i, 2, xl_free2 && rightop->regs[XL_IDX] < i, false, false, false, !started);
           if (!started && aopIsLitVal (rightop, i, 2, 1))
             emit3_o (A_INCW, result->aop, i, 0, 0);
           else if (started && aopIsLitVal (rightop, i, 2, 0x0000))
             emit3_o (A_ADCW, result->aop, i, 0, 0);
           else
             wassert (0);
           i += 2;
           started = true;
           continue;
         }
       else if (i + 1 < size && !maskedword && aopIsOp16_2 (rightop, i) && // Use addw / adcw in y
         (aopInReg (result->aop, i, Y_IDX) || y_free2 && (aopOnStack (leftop, i, 2) || leftop->type == AOP_DIR) && aopOnStack (result->aop, i, 2)))
         {
           genMove_o (ASMOP_Y, 0, leftop, i, 2, xl_free2 && rightop->regs[XL_IDX] < i, false, true, false, !started);
           if (!started && aopIsLitVal (rightop, i, 2, 0x0001))
             emit3 (A_INCW, ASMOP_Y, 0);
           else if (started && aopIsLitVal (rightop, i, 2, 0x0000))
             emit3 (A_ADCW, ASMOP_Y, 0);
           else
             emit3_o (started ? A_ADCW : A_ADDW, ASMOP_Y, 0, rightop, i);
           genMove_o (result->aop, i, ASMOP_Y, 0, 2, xl_free2, false, true, false, i + 2 == size);
           i += 2;
           started = true;
           continue;
         }
       else if (i + 1 < size && !maskedword && aopIsOp16_2 (leftop, i) &&
         (aopInReg (result->aop, i, Y_IDX) || y_free2 && (aopOnStack (rightop, i, 2) || rightop->type == AOP_DIR) && aopOnStack (result->aop, i, 2)))
         {
           genMove_o (ASMOP_Y, 0, rightop, i, 2, xl_free2 && leftop->regs[XL_IDX] < i, false, true, false, !started);
           if (started && aopIsLitVal (leftop, i, 2, 0x0000))
             emit3 (A_ADCW, ASMOP_Y, 0);
           else
             emit3_o (started ? A_ADCW : A_ADDW, ASMOP_Y, 0, leftop, i);
           genMove_o (result->aop, i, ASMOP_Y, 0, 2, xl_free2, false, true, false, i + 2 == size);
           i += 2;
           started = true;
           continue;
         }
       else if (i + 1 < size && !maskedword && aopIsOp16_2 (rightop, i) &&
         (aopInReg (leftop, i, X_IDX) && regDead (X_IDX, ic) || aopInReg (leftop, i, Z_IDX) && regDead (Z_IDX, ic) && !aopInReg (rightop, i, X_IDX)) &&
         (aopOnStack (result->aop, i, 2) || result->aop->type == AOP_DIR))
         {
           wassert (!aopInReg (leftop, i, Z_IDX) || !aopInReg (rightop, i, X_IDX));
           if (started && aopIsLitVal (leftop, i, 2, 0x0000))
             emit3_o (A_ADCW, left->aop, i, 0, 0);
           else
             emit3_o (started ? A_ADCW : A_ADDW, leftop, i, rightop, i);
           genMove_o (result->aop, i, leftop, i, 2, true, true, y_free2, false, i + 2 == size);
           i += 2;
           started = true;
           continue;
         }
       else if (!maskedbyte && aopSame (result->aop, i, leftop, i, 1) && aopAre8_2 (leftop, i, rightop, i))
         {
            if (!started && (aopIsLitVal (rightop, i, 1, 1) || aopIsLitVal (rightop, i, 1, -1))) // Use inc / dec
             emit3_o (aopIsLitVal (rightop, i, 1, 1) ? A_INC : A_DEC, leftop, i, 0, 0);
           else
             emit3_o (started ? A_ADC : A_ADD, leftop, i, rightop, i);
           i++;
           started = true;
           continue;
         }
       else if (xl_free && aopInReg (rightop, i, XL_IDX) && aopIsOp8_2 (leftop, i))
         {
           if (!started && (aopIsLitVal (rightop, i, 1, 1) || aopIsLitVal (rightop, i, 1, -1))) // Use inc / dec
             emit3 (aopIsLitVal (rightop, i, 1, 1) ? A_INC : A_DEC, ASMOP_XL, 0);
           else
             emit3_o (started ? A_ADC : A_ADD, ASMOP_XL, 0, leftop, i);
           if (maskedbyte)
            {
              emit2 ("and", "xl, #0x%02x", topbytemask);
              cost (2, 1);
            }
           genMove_o (result->aop, i, ASMOP_XL, 0, true, false, false, false, false, i + 1 == size);
           i++;
           started = true;
           continue;
         }

       if (!xl_free)
         push (ASMOP_XL, 0, 1);
       if (aopInReg (rightop, i, XL_IDX))
         UNIMPLEMENTED;

       bool y_dead = regDead (Y_IDX, ic) &&
         leftop->regs[YL_IDX] <= i && leftop->regs[YH_IDX] <= i && rightop->regs[YL_IDX] < i && rightop->regs[YH_IDX] < i &&
         (result->aop->regs[YL_IDX] < 0 || result->aop->regs[YL_IDX] >= i) && (result->aop->regs[YH_IDX] < 0 || result->aop->regs[YH_IDX] >= i);
       genMove_o (ASMOP_XL, 0, leftop, i, 1, true, false, y_dead, false, !started);
       if (aopIsOp8_2 (rightop, i))
         {
           if (!started && (aopIsLitVal (rightop, i, 1, 1) || aopIsLitVal (rightop, i, 1, -1))) // Use inc / dec
             emit3 (aopIsLitVal (rightop, i, 1, 1) ? A_INC : A_DEC, ASMOP_XL, 0);
           else
             emit3_o (started ? A_ADC : A_ADD, ASMOP_XL, 0, rightop, i);
           if (maskedbyte)
            {
              emit2 ("and", "xl, #0x%02x", topbytemask);
              cost (2, 1);
            }
         }
       else
         UNIMPLEMENTED;
       genMove_o (result->aop, i, ASMOP_XL, 0, true, false, false, false, false, i + 1 == size);
       if (!xl_free)
         pop (ASMOP_XL, 0, 1);
       i++;
       started = true;
    }

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genMinus - generates code for minus                             */
/*-----------------------------------------------------------------*/
static void
genMinus (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genMinus", ""));

  aopOp (ic->left, ic, false);
  aopOp (ic->right, ic, false);
  aopOp (ic->result, ic, true);

  genSub (ic, result->aop, left->aop, right->aop);

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
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

  aopOp (ic->left, ic, false);
  aopOp (ic->right, ic, false);
  aopOp (ic->result, ic, true);

  if (left->aop->size > 1 || right->aop->size > 1 || result->aop->size > 2)
    wassertl (0, "Large multiplication is handled through support function calls.");

  int size = result->aop->size;
  asmop *mulop = 0;

  if (size == 2 && aopInReg (result->aop, 0, X_IDX))
    mulop = ASMOP_X;
  else if (size == 2 && aopInReg (result->aop, 0, Z_IDX))
    mulop = ASMOP_Z;
  else if (regDead (Y_IDX, ic))
    mulop = ASMOP_Y;

  if (mulop)
    {
      if (aopInReg (left->aop, 0, mulop->aopu.bytes[1].byteu.reg->rIdx) || aopInReg (right->aop, 0, mulop->aopu.bytes[0].byteu.reg->rIdx))
        {
          genMove_o (mulop, 1, left->aop, 0, 1, regDead (XL_IDX, ic) && !aopInReg (right->aop, 0, XL_IDX), false, false, false, true);
          genMove_o (mulop, 0, right->aop, 0, 1, regDead (XL_IDX, ic) && !aopInReg (mulop, 1, XL_IDX), false, false, false, true);
        }
      else
        {
          genMove_o (mulop, 1, right->aop, 0, 1, regDead (XL_IDX, ic) && !aopInReg (left->aop, 0, XL_IDX), false, false, false, true);
          genMove_o (mulop, 0, left->aop, 0, 1, regDead (XL_IDX, ic) && !aopInReg (mulop, 1, XL_IDX), false, false, false, true);
        }
      emit3 (A_MUL, mulop, 0);
      genMove (result->aop, mulop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
    }
  else
    UNIMPLEMENTED;

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*------------------------------------------------------------------*/
/* genCmp :- greater or less than (and maybe with equal) comparison */
/*------------------------------------------------------------------*/
static void
genCmp (const iCode *ic, iCode *ifx)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  
  D (emit2 ("; genCmp", ""));

  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  bool pushed_xl = false;
  int size = max (left->aop->size, right->aop->size);
  bool sign = false;
  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    sign = !(SPEC_USIGN (operandType (left)) | SPEC_USIGN (operandType (right)));

  // To not swap operands if doing so complicates the code.
  if (ic->op == '>' && ifx && !sign &&
    (size == 1 && aopAre8_2 (left->aop, 0, right->aop, 0) ||
     size == 2 && (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD) && aopIsAcc16 (left->aop, 0)))
    {
      if (size == 1)
        emit3 (A_CP, left->aop, right->aop);
      else
        emit3 (A_CPW, left->aop, right->aop);
      symbol *tlbl = 0;
      if (!regalloc_dry_run)
        {
          tlbl = newiTempLabel (0);
          emit2 (IC_TRUE (ifx) ? "jrle" : "jrgt", "#!tlabel", labelKey2num (tlbl->key));
        }
      cost (2, 1);
      emitJP (IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
      emitLabel (tlbl);
      if (IC_TRUE (ifx))
        G.c = 1;
      goto release;
    }

  if (ic->op == '>')
    {
      operand *t = right;
      right = left;
      left = t;  
    }

  if (ifx && right->aop->type == AOP_LIT && sign && aopIsLitVal (right->aop, 0, size, 0) && (aopIsOp8_1 (left->aop, size - 1) || size >= 2 && aopIsOp16_1 (left->aop, size - 2))) // Use tst(w)
    {
      if (aopIsOp8_1 (left->aop, size - 1))
        {
          if (aopInReg (left->aop, size - 1, YH_IDX)) // tstw y is cheaper than tst yh.
            emit3 (A_TSTW, ASMOP_Y, 0);
          else
            emit3_o (A_TST, left->aop, size - 1, 0, 0);
        }
      else
        emit3_o (A_TSTW, left->aop, size - 2, 0, 0);
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      if (!regalloc_dry_run)
        emit2 (IC_TRUE (ifx) ? "jrnn" : "jrn", "#!tlabel", labelKey2num (tlbl->key));
      cost (2, 1);
      emitJP (IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
      emitLabel (tlbl);
      goto release;
    }
  else if (!ifx && right->aop->type == AOP_LIT && sign && aopIsLitVal (right->aop, 0, size, 0))
    {
      if (aopRS (left->aop) && left->aop->aopu.bytes[size - 1].in_reg && regDead (left->aop->aopu.bytes[size - 1].byteu.reg->rIdx, ic) && !aopInReg (left->aop, size - 1, YH_IDX))
        emit3_o (A_SLL, left->aop, size - 1, 0, 0);
      else
        {
          if (!regDead (XL_IDX, ic))
            {
              push (ASMOP_XL, 0, 1);
              pushed_xl = true;
            }
          genMove_o (ASMOP_XL, 0, left->aop, size - 1, 1, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);
          emit2 ("and", "xl, #0x80");
          cost (2, 1);
          emit3 (A_BOOL, ASMOP_XL, 0);
          goto return_xl;
        }
      goto return_c;
    }
  else if (!sign && size == 1 && aopAre8_2 (left->aop, 0, right->aop, 0))
    emit3 (A_CP, left->aop, right->aop);
  else if (!sign && size == 2 && (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD) && aopIsAcc16 (left->aop, 0))
    emit3 (A_CPW, left->aop, right->aop);
  else if (ifx && // Use inverse jump condition.
    (size == 1 && aopAre8_2 (right->aop, 0, left->aop, 0) || size == 2 && aopIsAcc16 (right->aop, 0) && (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD)))
    {
      emit3 ((size == 1) ? A_CP : A_CPW, right->aop, left->aop);
      symbol *tlbl = 0;
      if (!regalloc_dry_run)
        {
          tlbl = newiTempLabel (0);

          if (!sign)
            emit2 (IC_TRUE (ifx) ? "jrle" : "jrgt", "#!tlabel", labelKey2num (tlbl->key));
          else
            emit2 (IC_TRUE (ifx) ? "jrsle" : "jrsgt", "#!tlabel", labelKey2num (tlbl->key));
        }
      cost (2, 1);
      emitJP (IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
      emitLabel (tlbl);
      goto release;
    }
  else
    {
      bool started = false;

      for (int i = 0; i < size;)
        {
          if (!sign && i + 2 < size && !started && aopIsLitVal (right->aop, i, 2, 0x0000))
            {
              i += 2;
            }
          else if (((!sign || ifx) && i + 1 < size || i + 2 < size) && !started && // Try to use cpw
            (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD) &&
            (aopInReg (left->aop, i, X_IDX) || aopInReg (left->aop, i, Y_IDX) || aopInReg (left->aop, i, Z_IDX)))
            {
              emit3_o (A_CPW, left->aop, i, right->aop, i);
              started = true;
              i += 2;
            }
          else if (((!sign || ifx) && i + 1 < size || i + 2 < size) && !started && // Try to use negw
            aopIsAcc16 (right->aop, 0) && aopIsLitVal (left->aop, i, 2, 0x0000) && regDead (right->aop->aopu.bytes[i].byteu.reg->rIdx, ic) && regDead (right->aop->aopu.bytes[i + 1].byteu.reg->rIdx, ic))
            {
              emit3_o (A_NEGW, right->aop, i, 0, 0);
              started = true;
              i += 2;
            }
          else if (((!sign || ifx) && i + 1 < size || i + 2 < size) && !started && // Try to use cpw
            (regDead (Y_IDX, ic) && aopInReg (left->aop, i, Y_IDX) && aopInReg (left->aop, i, X_IDX) ||
            regDead (X_IDX, ic) && aopInReg (left->aop, i, X_IDX) && aopInReg (left->aop, i, Y_IDX)))
            {
              emit3sub_o (started ? A_SBCW : A_SUBW, left->aop, i, right->aop, i);
              started = true;
              i += 2;
            }
          else if (((!sign || ifx) && i + 1 < size || i + 2 < size) &&
            aopIsOp16_2 (right->aop, i) &&
            regDead (Y_IDX, ic) && (aopInReg (left->aop, i, Y_IDX) || left->aop->type == AOP_LIT || left->aop->type == AOP_DIR || aopOnStack (left->aop, i, 2)) && right->aop->regs[YL_IDX] < i && right->aop->regs[YH_IDX] < i && left->aop->regs[YL_IDX] <= i + 1 && left->aop->regs[YH_IDX] <= i + 1)
            {
              genMove_o (ASMOP_Y, 0, left->aop, i, 2, false, true, false, false, !started);
              if (right->aop->type == AOP_LIT)
                {
                  if (!started && aopIsLitVal (right->aop, i, 2, 0xffff))
                    emit3 (A_INCW, ASMOP_Y, 0);
                  else if (started && (aopIsLitVal (right->aop, i, 2, 0x0000) || aopIsLitVal (right->aop, i, 2, 0xffff)))
                    emit3 (aopIsLitVal (right->aop, i, 2, 0x0000) ? A_SBCW : A_ADCW, ASMOP_Y, 0);
                  else if (!started)
                    {
                      emit2 ("cpw", "y, #0x%04x", byteOfVal (right->aop->aopu.aop_lit, i) + byteOfVal (right->aop->aopu.aop_lit, i + 1) * 256);
                      cost (3, 1);
                      spillReg (C_IDX);
                    }
                  else
                    emit3sub_o (A_SBCW, ASMOP_Y, 0, right->aop, i);
                }
              else
                emit3sub_o (started ? A_SBCW : A_SUBW, ASMOP_Y, 0, right->aop, i);
              started = true;
              i += 2;
            }
          else if (((!sign || ifx) && i + 1 < size || i + 2 < size) &&
            aopIsOp16_2 (right->aop, i) &&
            (regDead (X_IDX, ic) && aopInReg (left->aop, i, X_IDX) || regDead (Z_IDX, ic) && aopInReg (left->aop, i, Z_IDX) && !aopInReg (right->aop, i, X_IDX)))
            {
              if (right->aop->type == AOP_LIT)
                {
                  if (!started && aopIsLitVal (right->aop, i, 2, 0xffff))
                    emit3_o (A_INCW, left->aop, i, 0, 0);
                  else if (started && (aopIsLitVal (right->aop, i, 2, 0x0000) || aopIsLitVal (right->aop, i, 2, 0xffff)))
                    emit3_o (aopIsLitVal (right->aop, i, 2, 0x0000) ? A_SBCW : A_ADCW, left->aop, i, 0, 0);
                  else
                    emit3sub_o (started ? A_SBCW : A_SUBW, left->aop, i, right->aop, i);
                }
              else
                emit3sub_o (started ? A_SBCW : A_SUBW, left->aop, i, right->aop, i);
              started = true;
              i += 2;
            }
          else if (((!sign || ifx) && i + 1 < size || i + 2 < size) &&
            (aopOnStack (left->aop, i, 2) || left->aop->type == AOP_DIR || left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD || aopInReg (left->aop, i, Y_IDX) || aopInReg (left->aop, i, X_IDX)) &&
            aopIsOp16_2 (right->aop, i) &&
            (regDead (Y_IDX, ic) && left->aop->regs[YL_IDX] <= i && left->aop->regs[YH_IDX] <= i && right->aop->regs[YL_IDX] < i && right->aop->regs[YH_IDX] < i ||
            regDead (X_IDX, ic) && left->aop->regs[XL_IDX] <= i && left->aop->regs[XH_IDX] <= i && right->aop->regs[XL_IDX] < i && right->aop->regs[XH_IDX] < i ||
            regDead (Z_IDX, ic) && left->aop->regs[ZL_IDX] <= i && left->aop->regs[ZH_IDX] <= i && right->aop->regs[ZL_IDX] < i && right->aop->regs[ZH_IDX] < i && !aopInReg (right->aop, i, X_IDX)))
            {
              asmop *laop =
                (regDead (Y_IDX, ic) && left->aop->regs[YL_IDX] <= i && left->aop->regs[YH_IDX] <= i && right->aop->regs[YL_IDX] <= i && right->aop->regs[YH_IDX] <= i) ? ASMOP_Y :
                ((regDead (X_IDX, ic) && left->aop->regs[XL_IDX] <= i && left->aop->regs[XH_IDX] <= i && right->aop->regs[XL_IDX] <= i && right->aop->regs[XH_IDX] <= i) ? ASMOP_X : ASMOP_Z); 
              genMove_o (laop, 0, left->aop, i, 2, false, true, false, false, !started);
              if (right->aop->type == AOP_LIT)
                {
                  if (!started && aopIsLitVal (right->aop, i, 2, 0xffff))
                    emit3 (A_INCW, laop, 0);
                  else if (started && (aopIsLitVal (right->aop, i, 2, 0x0000) || aopIsLitVal (right->aop, i, 2, 0xffff)))
                    emit3 (aopIsLitVal (right->aop, i, 2, 0x0000) ? A_SBCW : A_ADCW, laop, 0);
                  else
                    emit3sub_o (started ? A_SBCW : A_SUBW, laop, 0, right->aop, i);
                }
              else
                emit3sub_o (started ? A_SBCW : A_SUBW, laop, 0, right->aop, i);
              started = true;
              i += 2;
            }
          else if ((!sign || ifx) && !started && aopAre8_2 (left->aop, i, right->aop, i))
            {
              emit3_o (A_CP, left->aop, i, right->aop, i);
              started = true;
              i++;
            }
          else
            {
              if (!regDead (XL_IDX, ic) && !pushed_xl || left->aop->regs[XL_IDX] > i || right->aop->regs[XL_IDX] > i)
                {
                  push (ASMOP_XL, 0, 1);
                  pushed_xl = true;
                }

              if (!aopIsOp8_2 (right->aop, i))
                UNIMPLEMENTED;
              else
                {
                  genMove_o (ASMOP_XL, 0, left->aop, i, 1, true, false, false, false, !started);
                  emit3sub_o (started ? A_SBC : A_SUB, ASMOP_XL, 0, right->aop, i);
                }
              
              if (pushed_xl && (left->aop->regs[XL_IDX] > i || right->aop->regs[XL_IDX] > i)) // Restore xl here early if we will need it again soon.
                {
                  pop (ASMOP_XL, 0, 1);
                  pushed_xl = false;
                }
              started = true;
              i++;
            }
        }
    }

  if (ifx)
    {
      if (pushed_xl)
        pop (ASMOP_XL, 0, 1);
      symbol *tlbl = 0;
      if (!regalloc_dry_run)
        {
          tlbl = newiTempLabel (0);
          if (!sign)
            emit2 (IC_TRUE (ifx) ? "jrc" : "jrnc", "#!tlabel", labelKey2num (tlbl->key));
          else
            emit2 (IC_TRUE (ifx) ? "jrsge" : "jrslt", "#!tlabel", labelKey2num (tlbl->key));
        }
      cost (2, 1);
      emitJP (IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
      emitLabel (tlbl);
      if (!sign)
        G.c = (bool)(IC_TRUE (ifx));
      goto release;
    }

  if (sign)
    {
      if (!regalloc_dry_run)
        {
          symbol *tlbl = newiTempLabel (0);
          emit2 ("jrno", "#!tlabel", labelKey2num (tlbl->key));
          emit2 ("xor", "xl, #0x80");
          cost (4, 2);
          emitLabel (tlbl);
        }
      cost (2, 1);
      emit3 (A_SLL, ASMOP_XL, 0);
    }
  else
    {
      emit3 (A_RLC, ASMOP_XL, 0);
      emit3 (A_XOR, ASMOP_XL, ASMOP_ONE);
      if (regDead (XL_IDX, ic) || pushed_xl)
        {
          emit3 (A_AND, ASMOP_XL, ASMOP_ONE);
          goto return_xl;
        }
      emit3 (A_RRC, ASMOP_XL, 0);
    }

return_c:
  if (!regDead (XL_IDX, ic) && !pushed_xl)
    {
      push (ASMOP_XL, 0, 1);
      pushed_xl = true;
    }
  emit3 (A_CLR, ASMOP_XL, 0);
  emit3 (A_RLC, ASMOP_XL, 0);
  G.c = 0;
return_xl:
  genMove (result->aop, ASMOP_XL, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      
  if (pushed_xl)
    pop (ASMOP_XL, 0, 1);

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
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  symbol *tlbl_NE = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  symbol *tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

  D (emit2 ("; genCmpEQorNE", ""));

  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  int size = max (left->aop->size, right->aop->size);

  /* Prefer literal operand on right */
  if (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD ||
    right->aop->type != AOP_LIT && right->aop->type != AOP_IMMD && left->aop->type == AOP_DIR)
    {
      operand *temp = left;
      left = right;
      right = temp;
    }

  if (!ifx && size == 1 && aopIsOp8_2 (right->aop, 0) &&
    result->aop->size == 1 && aopInReg (result->aop, 0, XL_IDX))
    {
      genMove (ASMOP_XL, left->aop, true, false, false, false);
      emit3sub (A_SUB, ASMOP_XL, right->aop);
      emit3 (A_BOOL, ASMOP_XL, 0);
      if (ic->op == EQ_OP)
        emit3 (A_XOR, ASMOP_XL, ASMOP_ONE);
      goto release;
    }
  else if (!ifx && size == 2 && aopIsOp16_2 (right->aop, 0) &&
    (result->aop->size == 2 && aopInReg (result->aop, 0, Y_IDX) || result->aop->size == 1 && aopInReg (result->aop, 0, YL_IDX) && regDead (YH_IDX, ic)))
    {
      bool xl_dead = regDead (XL_IDX, ic) && right->aop->regs[XL_IDX] < 0;
      genMove (ASMOP_Y, left->aop, xl_dead, false, true, false);
      emit3sub (A_SUBW, ASMOP_Y, right->aop);
      if (ic->op == NE_OP)
        emit3 (A_BOOLW, ASMOP_Y, 0);
      else
        {
          emit3 (A_CLRW, ASMOP_Y, 0);
          if (tlbl)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl->key));
          emit3 (A_INCW, ASMOP_Y, 0);
          emitLabel (tlbl);
        }
      goto release;
    }

  for (int i = 0; i < size;)
    {
      bool xl_dead = regDead (XL_IDX, ic) && left->aop->regs[XL_IDX] <= i && right->aop->regs[XL_IDX] <= i;
      bool xh_dead = regDead (XH_IDX, ic) && left->aop->regs[XH_IDX] <= i && right->aop->regs[XH_IDX] <= i;
      bool x_dead2 = regDead (X_IDX, ic) && left->aop->regs[XL_IDX] <= i + 1 && left->aop->regs[XH_IDX] <= i + 1 && right->aop->regs[XL_IDX] <= i + 1 && right->aop->regs[XH_IDX] <= i + 1;
      bool y_dead2 = regDead (Y_IDX, ic) && left->aop->regs[YL_IDX] <= i + 1 && left->aop->regs[YH_IDX] <= i + 1 && right->aop->regs[YL_IDX] <= i + 1 && right->aop->regs[YH_IDX] <= i + 1;

      if (i + 1 < size && aopIsOp16_1 (left->aop, i) && aopIsLitVal (right->aop, i, 2, 0x0000)) // Use tstw
        {
          emit3_o (A_TSTW, left->aop, i, 0, 0);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (i + 1 < size && aopInReg (left->aop, i, Y_IDX) && regDead (Y_IDX, ic) && aopIsLitVal (right->aop, i, 2, 0xffff)) // Use incw
        {
          emit3_o (A_INCW, left->aop, i, 0, 0);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (i + 1 < size && aopIsAcc16 (left->aop, i) &&
        (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD))
        {
          emit3_o (A_CPW, left->aop, i, right->aop, i);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (i + 1 < size && aopInReg (left->aop, i, X_IDX) && x_dead2 && aopOnStackNotExt (right->aop, i, 2))
        {
          emit3sub_o (A_SUBW, ASMOP_X, 0, right->aop, i);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (i + 1 < size && aopIsOp16_2 (right->aop, i) && y_dead2)
        {
          genMove_o (ASMOP_Y, 0, left->aop, i, 2, xl_dead && right->aop->regs[XL_IDX] < i, xh_dead && right->aop->regs[XH_IDX] < i, true, false, true);
          emit3sub_o (A_SUBW, ASMOP_Y, 0, right->aop, i);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (i + 1 < size && aopIsOp16_2 (left->aop, i) && y_dead2)
        {
          genMove_o (ASMOP_Y, 0, right->aop, i, 2, xl_dead && left->aop->regs[XL_IDX] < i, xh_dead && left->aop->regs[XH_IDX] < i, true, false, true);
          emit3sub_o (A_SUBW, ASMOP_Y, 0, left->aop, i);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (i + 1 < size && aopInReg (left->aop, i, Y_IDX) && aopIsOp16_2 (right->aop, i))
        {
          push (ASMOP_Y, 0, 2);
          emit3sub_o (A_SUBW, ASMOP_Y, 0, right->aop, i);
          pop (ASMOP_Y, 0, 2);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (i + 1 < size && aopInReg (right->aop, i, Y_IDX) && aopIsOp16_2 (left->aop, i))
        {
          push (ASMOP_Y, 0, 2);
          emit3sub_o (A_SUBW, ASMOP_Y, 0, left->aop, i);
          pop (ASMOP_Y, 0, 2);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (i + 1 < size && (aopOnStack (right->aop, i, 2) || right->aop->type == AOP_DIR) && aopIsOp16_2 (right->aop, i) && x_dead2)
        {
          genMove_o (ASMOP_X, 0, left->aop, i, 2, xl_dead && right->aop->regs[XL_IDX] < i, xh_dead && right->aop->regs[XH_IDX] < i, true, false, true);
          emit3sub_o (A_SUBW, ASMOP_X, 0, right->aop, i);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else if (!xl_dead && i + 1 < size && aopIsOp16_2 (right->aop, i))
        {
          push (ASMOP_Y, 0, 2);
          genMove_o (ASMOP_Y, 0, left->aop, i, 2, false, xh_dead && right->aop->regs[XH_IDX] < i, true, false, true);
          emit3sub_o (A_SUBW, ASMOP_Y, 0, right->aop, i);
          pop (ASMOP_Y, 0, 2);
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i += 2;
        }
      else
        {
          if (aopAre8_2 (left->aop, i, right->aop, i) &&
            !(aopInReg (right->aop, i, XL_IDX))) // Fall through to swapped operands below, if that is more efficient.
            emit3_o (A_CP, left->aop, i, right->aop, i);
          else if (aopAre8_2 (right->aop, i, left->aop, i))
            emit3_o (A_CP, right->aop, i, left->aop, i);
          else if (xl_dead && xh_dead && !aopIsOp8_2 (right->aop, i) && left->aop->regs[XH_IDX] < i)
            {
              genMove_o (ASMOP_XH, 0, right->aop, i, 1, true, true, false, false, true);
              genMove_o (ASMOP_XL, 0, left->aop, i, 1, true, false, false, false, true);
              emit3 (A_CP, ASMOP_XL, ASMOP_XH);
            }
          else if (!aopIsOp8_2 (right->aop, i))
            UNIMPLEMENTED;
          else
            {
              if (!xl_dead)
                push (ASMOP_XL, 0, 1);
              genMove_o (ASMOP_XL, 0, left->aop, i, 1, true, false, false, false, true);
              emit3_o (A_CP, ASMOP_XL, 0, right->aop, i);
              if (!xl_dead)
                pop (ASMOP_XL, 0, 1);
            }
          if (tlbl_NE)
            emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl_NE->key));
          i++;
        }
    }

  if (ifx)
    {
      if ((bool)IC_TRUE (ifx) ^ (ic->op == NE_OP))
        {
          emitJP(IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
          emitLabel (tlbl_NE);
        }
      else
        {
          emitJP(tlbl, 0.5f);
          emitLabel (tlbl_NE);
          emitJP(IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
          emitLabel (tlbl);
        }
    }
  else
    {
      genMove (result->aop, ic->op == EQ_OP ? ASMOP_ONE : ASMOP_ZERO, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      emitJP(tlbl, 0.5);
      emitLabel (tlbl_NE);
      genMove (result->aop, ic->op == NE_OP ? ASMOP_ONE : ASMOP_ZERO, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      emitLabel (tlbl);
    }

release:    
  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genXor - code for xor                                           */
/*-----------------------------------------------------------------*/
static void
genXor (const iCode *ic)
{
  operand *left, *right, *result;

  D (emit2 ("; genXor", ""));

  aopOp ((left = ic->left), ic, false);
  aopOp ((right = ic->right), ic, false);
  aopOp ((result = ic->result), ic, true);
  
  genEor (ic, result->aop, left->aop, right->aop);

  freeAsmop (left);
  freeAsmop (right);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genOr - code for and                                            */
/*-----------------------------------------------------------------*/
static void
genOr (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  D (emit2 ("; genOr", ""));

  aopOp (ic->left, ic, false);
  aopOp (ic->right, ic, false);
  aopOp (ic->result, ic, true);

  int size = getSize (operandType (result));

  /* Prefer literal on right. */
  if (left->aop->type == AOP_LIT || right->aop->type != AOP_LIT && left->aop->type == AOP_IMMD) // todo: Swap in more cases when right in reg, left not. Swap individually per-byte.
    {
      operand *t = right;
      right = left;
      left = t;
    }

  for (int i = 0; i < size;)
    {
       bool xl_free = regDead (XL_IDX, ic) && left->aop->regs[XL_IDX] <= i && right->aop->regs[XL_IDX] <= i && (result->aop->regs[XL_IDX] < 0 || result->aop->regs[XL_IDX] >= i);
       bool xh_free = regDead (XH_IDX, ic) && left->aop->regs[XH_IDX] <= i && right->aop->regs[XH_IDX] <= i && (result->aop->regs[XH_IDX] < 0 || result->aop->regs[XH_IDX] >= i);
       bool yl_free = regDead (YL_IDX, ic) && left->aop->regs[YL_IDX] <= i && right->aop->regs[YL_IDX] <= i && (result->aop->regs[YL_IDX] < 0 || result->aop->regs[YL_IDX] >= i);
       bool yh_free = regDead (YH_IDX, ic) && left->aop->regs[YH_IDX] <= i && right->aop->regs[YH_IDX] <= i && (result->aop->regs[YH_IDX] < 0 || result->aop->regs[YH_IDX] >= i);
       bool y_free = yl_free && yh_free;

       if (aopIsLitVal (right->aop, i, 1, 0x00) || aopIsLitVal (right->aop, i, 1, 0xff))
         {
           unsigned int bytelit = aopIsLitVal (right->aop, i, 1, 0x00) ? 0x00 : 0xff;
           int end;
           for(end = i; end < size && aopIsLitVal (right->aop, end, 1, bytelit); end++);
           genMove_o (result->aop, i, bytelit == 0xff ? ASMOP_MONE : left->aop, i, end - i, xl_free, xh_free, y_free, false, true);
           i = end;
           continue;
         }
       else if (aopIsLitVal (left->aop, i, 1, 0x00) || aopIsLitVal (left->aop, i, 1, 0xff))
         {
           unsigned int bytelit = aopIsLitVal (left->aop, i, 1, 0x00) ? 0x00 : 0xff;
           int end;
           for(end = i; end < size && aopIsLitVal (left->aop, end, 1, bytelit); end++);
           genMove_o (result->aop, i, bytelit == 0xff ? ASMOP_MONE : right->aop, i, end - i, xl_free, xh_free, y_free, false, true);
           i = end;
           continue;
         }

       if (i + 1 < size && aopAre16_2 (result->aop, i, right->aop, i) &&
         !(aopRS (right->aop) && aopRS (result->aop) && (result->aop->aopu.bytes[i].in_reg && right->aop->regs[result->aop->aopu.bytes[i].byteu.reg->rIdx] >= i || result->aop->aopu.bytes[i + 1].in_reg && right->aop->regs[result->aop->aopu.bytes[i + 1].byteu.reg->rIdx] >= i)))
         {
           genMove_o (result->aop, i, left->aop, i, 2, xl_free && right->aop->regs[XL_IDX] < i, xh_free && right->aop->regs[XH_IDX] < i, false, false, true);
           if ((aopInReg (result->aop, i, XL_IDX) || aopInReg (result->aop, i, ZL_IDX)) &&
             aopIsLitVal (right->aop, i + 1, 1, 0x00)) // Avoid orw x/z, #ii, when or xl/zl, #i will do.
             emit3_o (A_OR, result->aop, i, right->aop, i);
           else
             emit3_o (A_ORW, result->aop, i, right->aop, i);
           i += 2;
           continue;
         }
       else if (i + 1 < size && aopAre16_2 (result->aop, i, left->aop, i) && !(aopInReg (result->aop, i, Z_IDX) && aopInReg (left->aop, i, X_IDX)) &&
         !(aopRS (left->aop) && aopRS (result->aop) && (result->aop->aopu.bytes[i].in_reg && left->aop->regs[result->aop->aopu.bytes[i].byteu.reg->rIdx] >= i || result->aop->aopu.bytes[i + 1].in_reg && left->aop->regs[result->aop->aopu.bytes[i + 1].byteu.reg->rIdx] >= i)))
         {
           genMove_o (result->aop, i, right->aop, i, 2, xl_free && left->aop->regs[XL_IDX] < i, xh_free && left->aop->regs[XH_IDX] < i, false, false, true);
           if ((aopInReg (result->aop, i, XL_IDX) || aopInReg (result->aop, i, ZL_IDX)) &&
             aopIsLitVal (left->aop, i + 1, 1, 0x00)) // Avoid orw x/z, #ii, when or xl/zl, #i will do.
             emit3_o (A_OR, result->aop, i, left->aop, i);
           else
             emit3_o (A_ORW, result->aop, i, left->aop, i);
           i += 2;
           continue;
         }
       else if (i + 1 < size && (aopIsAcc8 (left->aop, i) || aopOnStack (left->aop, i, 2) || left->aop->type == AOP_DIR) && aopSame (result->aop, i, left->aop, i, 2) && aopIsLitVal (right->aop, i, 2, 0x0001) &&
         !left->aop->valinfo.anything && (left->aop->valinfo.knownbitsmask & (1ull << i * 8)) && !(left->aop->valinfo.knownbits & (1ull << i * 8)))
         {
           if (aopInReg (left->aop, i, YL_IDX))
             emit3 (A_INCW, ASMOP_Y, 0);
           else
             emit3_o (A_INC, result->aop, i, 0, 0);
           i += 2;
           continue;
         }
       else if (i + 1 < size && y_free && (aopOnStack (left->aop, i, 2) || left->aop->type == AOP_DIR) && aopSame (result->aop, i, left->aop, i, 2))
         {
           genMove_o (ASMOP_Y, 0, right->aop, i, 2, xl_free, xh_free, true, false, true);
           emit3_o (A_ORW, result->aop, i, ASMOP_Y, 0);
           i += 2;
           continue;
         }
       else if (i + 1 < size && y_free && (aopOnStack (right->aop, i, 2) || right->aop->type == AOP_DIR) && aopSame (result->aop, i, right->aop, i, 2))
         {
           genMove_o (ASMOP_Y, 0, left->aop, i, 2, xl_free, xh_free, true, false, true);
           emit3_o (A_ORW, result->aop, i, ASMOP_Y, 0);
           i += 2;
           continue;
         }
       else if (i + 1 < size && y_free && aopOnStack (result->aop, i, 2) && (aopOnStack (left->aop, i, 2) || left->aop->type == AOP_DIR) && aopIsOp16_2 (right->aop, i))
         {
           genMove_o (ASMOP_Y, 0, left->aop, i, 2, xl_free && right->aop->regs[XL_IDX] < i, xh_free && right->aop->regs[XH_IDX] < i, true, false, true);
           emit3_o (A_ORW, ASMOP_Y, 0, right->aop, i);
           genMove_o (result->aop, i, ASMOP_Y, 0, 2, xl_free, xh_free, true, false, true);
           i += 2;
           continue;
         }
       else if (aopSame (result->aop, i, left->aop, i, 1) && aopAre8_2 (left->aop, i, right->aop, i))
         {
           emit3_o (A_OR, result->aop, i, right->aop, i);
           i++;
           continue;
         }
       else if (aopSame (result->aop, i, right->aop, i, 1) && aopAre8_2 (right->aop, i, left->aop, i))
         {
           emit3_o (A_OR, result->aop, i, left->aop, i);
           i++;
           continue;
         }
       else if (i + 1 < size && !y_free && aopOnStack (result->aop, i, 2) && (aopOnStack (left->aop, i, 2) || left->aop->type == AOP_DIR) && aopIsOp16_2 (right->aop, i) &&
         !(aopSame (result->aop, i, left->aop, i, 1) && (aopIsLitVal (left->aop, i + 1, 1, 0xff) || aopIsLitVal (right->aop, i + 1, 1, 0xff)))) // Avoid this path that saves and restores y if using xl is cheaper.
         {
           push (ASMOP_Y, 0, 2);
           genMove_o (ASMOP_Y, 0, left->aop, i, 2, xl_free && right->aop->regs[XL_IDX] < i, xh_free && right->aop->regs[XH_IDX] < i, true, false, true);
           emit3_o (A_ORW, ASMOP_Y, 0, right->aop, i);
           genMove_o (result->aop, i, ASMOP_Y, 0, 2, xl_free, xh_free, true, false, true);
           pop (ASMOP_Y, 0, 2);
           i += 2;
           continue;
         }

       if (!xl_free)
         push (ASMOP_XL, 0, 1);
       
       if (aopIsOp8_2 (right->aop, i))
         {
           genMove_o (ASMOP_XL, 0, left->aop, i, 1, true, xh_free, y_free, false, true);
           emit3_o (A_OR, ASMOP_XL, 0, right->aop, i);
         }
       else if (aopIsOp8_2 (left->aop, i))
         {
           genMove_o (ASMOP_XL, 0, right->aop, i, 1, true, xh_free, y_free, false, true);
           emit3_o (A_OR, ASMOP_XL, 0, left->aop, i);
         }
       else
         UNIMPLEMENTED;

       genMove_o (result->aop, i, ASMOP_XL, 0, 1, true, xh_free, y_free, false, true);
       
       if (!xl_free)
         pop (ASMOP_XL, 0, 1);

       i++;
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

  aopOp (ic->left, ic, false);
  aopOp (ic->right, ic, false);
  aopOp (ic->result, ic, true);

  int size = getSize (operandType (result));

  /* Prefer literal on right. */
  if (left->aop->type == AOP_LIT || right->aop->type != AOP_LIT && left->aop->type == AOP_IMMD) // todo: Swap in more cases when right in reg, left not. Swap individually per-byte.
    {
      operand *t = right;
      right = left;
      left = t;
    }

  if (ifx && result->aop->type == AOP_CND)
    {
      int i, j;
      int nonzero;
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);

      wassertl (right->aop->type == AOP_LIT, "Code generation for bitwise and can only jump on literal operand");
      // Find the non-zero byte.
      for (j = 0, nonzero = 0, i = -1; j < size; j++)
        if (byteOfVal (right->aop->aopu.aop_lit, j))
          {
            i = j;
            nonzero++;
          }

      wassertl (nonzero <= 1, "Code generation for bitwise and can handle at most one nonzero byte");

      if (!nonzero)
        goto release;

      if (byteOfVal (right->aop->aopu.aop_lit, i) == 0x80 && // Use tstw
        aopInReg (left->aop, i, YH_IDX))
        {
          emit3 (A_TSTW, ASMOP_Y, 0);
          if (!regalloc_dry_run)
            emit2 (IC_TRUE (ifx) ? "jrnn" : "jrn", "!tlabel", labelKey2num (tlbl->key));
          cost (2, 1);
        }
      else if ((byteOfVal (right->aop->aopu.aop_lit, i) == 0x80 || byteOfVal (right->aop->aopu.aop_lit, i) == 0xff) && // Use tst
        (aopIsOp8_1 (left->aop, i) || aopIsAcc8 (left->aop, i)))
        {
          emit3_o (A_TST, left->aop, i, 0, 0);
          if (!regalloc_dry_run)
            {
              if (byteOfVal (right->aop->aopu.aop_lit, i) == 0x80)
                emit2 (IC_TRUE (ifx) ? "jrnn" : "jrn", "#!tlabel", labelKey2num (tlbl->key));
              else // 0xff
                emit2 (IC_TRUE (ifx) ? "jrz" : "jrnz", "#!tlabel", labelKey2num (tlbl->key));
            }
          cost (2, 1);
        }
      else if (byteOfVal (right->aop->aopu.aop_lit, i) == 0x01 && aopInReg (left->aop, i, YL_IDX) && regDead (Y_IDX, ic)) // Use srlw
        {
          emit3 (A_SRLW, ASMOP_Y, 0);
          if (!regalloc_dry_run)
            emit2 (IC_TRUE (ifx) ? "jrnc" : "jrc", "#!tlabel", labelKey2num (tlbl->key));
          cost (2, 1);
        }
      else if (byteOfVal (right->aop->aopu.aop_lit, i) == 0x01 && aopIsAcc8 (left->aop, i) && regDead (left->aop->aopu.bytes[i].byteu.reg->rIdx, ic)) // Use srl
        {
          emit3_o (A_SRL, left->aop, i, 0, 0);
          if (!regalloc_dry_run)
            emit2 (IC_TRUE (ifx) ? "jrnc" : "jrc", "#!tlabel", labelKey2num (tlbl->key));
          cost (2, 1);
        }
      else // and in xl.
        {
          if (!regDead (XL_IDX, ic))
            push (ASMOP_XL, 0, 1);
          genMove_o (ASMOP_XL, 0, left->aop, i, 1, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);
          emit3_o (A_AND, ASMOP_XL, 0, right->aop, i);
          if (!regDead (XL_IDX, ic))
            pop (ASMOP_XL, 0, 1);
          if (!regalloc_dry_run)
            emit2 (IC_TRUE (ifx) ? "jrz" : "jrnz", "#!tlabel", labelKey2num (tlbl->key));
          cost (2, 1);
        }

      emitJP(IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
      emitLabel (tlbl);
      goto release;
    }

  for (int i = 0; i < size;)
    {
       const bool xl_free = regDead (XL_IDX, ic) && left->aop->regs[XL_IDX] <= i && right->aop->regs[XL_IDX] <= i && (result->aop->regs[XL_IDX] < 0 || result->aop->regs[XL_IDX] >= i);
       const bool xh_free = regDead (XH_IDX, ic) && left->aop->regs[XH_IDX] <= i && right->aop->regs[XH_IDX] <= i && (result->aop->regs[XH_IDX] < 0 || result->aop->regs[XH_IDX] >= i);
       const bool yl_free = regDead (YL_IDX, ic) && left->aop->regs[YL_IDX] <= i && right->aop->regs[YL_IDX] <= i && (result->aop->regs[YL_IDX] < 0 || result->aop->regs[YL_IDX] >= i);
       const bool yh_free = regDead (YH_IDX, ic) && left->aop->regs[YH_IDX] <= i && right->aop->regs[YH_IDX] <= i && (result->aop->regs[YH_IDX] < 0 || result->aop->regs[YH_IDX] >= i);
       const bool y_free = yl_free && yh_free;

       if (aopIsLitVal (right->aop, i, 1, 0x00) || aopIsLitVal (right->aop, i, 1, 0xff))
         {
           unsigned int bytelit = aopIsLitVal (right->aop, i, 1, 0x00) ? 0x00 : 0xff;
           int end;
           for(end = i; end < size && aopIsLitVal (right->aop, end, 1, bytelit); end++);
           genMove_o (result->aop, i, bytelit == 0x00 ? ASMOP_ZERO : left->aop, i, end - i, xl_free, xh_free, y_free, false, true);
           i = end;
           continue;
         }
       else if (aopIsLitVal (left->aop, i, 1, 0x00) || aopIsLitVal (left->aop, i, 1, 0xff))
         {
           unsigned int bytelit = aopIsLitVal (left->aop, i, 1, 0x00) ? 0x00 : 0xff;
           int end;
           for (end = i; end < size && aopIsLitVal (left->aop, end, 1, bytelit); end++);
           genMove_o (result->aop, i, bytelit == 0x00 ? ASMOP_ZERO : right->aop, i, end - i, xl_free, xh_free, y_free, false, true);
           i = end;
           continue;
         }

       if (aopSame (result->aop, i, left->aop, i, 1) && aopAre8_2 (left->aop, i, right->aop, i))
         {
           emit3_o (A_AND, result->aop, i, right->aop, i);
           i++;
           continue;
         }
       else if (aopSame (result->aop, i, right->aop, i, 1) && aopAre8_2 (right->aop, i, left->aop, i))
         {
           emit3_o (A_AND, result->aop, i, left->aop, i);
           i++;
           continue;
         }

       if (!xl_free)
         UNIMPLEMENTED;

       if (aopIsOp8_2 (right->aop, i))
         {
           genMove_o (ASMOP_XL, 0, left->aop, i, 1, true, xh_free, y_free, false, true);
           emit3_o (A_AND, ASMOP_XL, 0, right->aop, i);
         }
       else if (aopIsOp8_2 (left->aop, i))
         {
           genMove_o (ASMOP_XL, 0, right->aop, i, 1, true, xh_free, y_free, false, true);
           emit3_o (A_AND, ASMOP_XL, 0, left->aop, i);
         }
       else
         UNIMPLEMENTED;

       if (i + 1 < size && aopIsLitVal (right->aop, i + 1, 1, 0xff) && aopInReg (left->aop, i + 1, XH_IDX))
         {
           genMove_o (result->aop, i, ASMOP_X, 0, 2, true, true, y_free, false, true);
           i += 2;
         }
       else
         {
           genMove_o (result->aop, i, ASMOP_XL, 0, 1, true, xh_free, y_free, false, true);
           i++;
         }
    }

release:
  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*------------------------------------------------------------------*/
/* genGetABit - get a bit                                           */
/*------------------------------------------------------------------*/
static void
genGetABit (const iCode *ic, iCode *ifx)
{
  operand *left, *right, *result;
  int shCount;
  bool pushed_xl = false;

  D (emit2 ("; genGetABit", ""));

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic, false);
  aopOp (left, ic, false);
  aopOp (result, ic, true);

  shCount = (int) ulFromVal ((right->aop)->aopu.aop_lit);

  if (ifx && result->aop->type == AOP_CND) // Use tst(w)
    {
      wassert (shCount % 8 == 7);

      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (NULL);

      if (aopInReg (left->aop, shCount / 8, YH_IDX))
        emit3 (A_TSTW, ASMOP_Y, 0);
      else if (aopIsOp8_1 (left->aop, shCount / 8))
        emit3_o (A_TST, left->aop, shCount / 8, 0, 0);

      if (!regalloc_dry_run)
        emit2 (IC_TRUE (ifx) ? "jrnn" : "jrn", "#!tlabel", labelKey2num (tlbl->key));
      cost (2, 1);

      emitJP (IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 1.0f);
      emitLabel (tlbl);

      goto release;
    }

  if (!regDead (XL_IDX, ic))
    {
      push (ASMOP_XL, 0, 1);
      pushed_xl = true;
    }

  if (aopInReg (left->aop, shCount / 8, ZL_IDX) && (shCount % 8 || !regDead (XL_IDX, ic)) &&
    (result->aop->size == 1 && aopInReg (result->aop, 0, ZL_IDX) && regDead (ZH_IDX, ic) ||
    result->aop->size == 2 && aopInReg (result->aop, 0, Z_IDX)))
    {
      emit3_o (A_CLR, ASMOP_Z, 1, 0, 0);
      emit2 ("and", "zl, #0x%02x", 1 << (shCount % 8));
      cost (3, 2);
      emit3 (A_BOOLW, ASMOP_Z, 0);
      goto release;
    }
  else if ((shCount % 8) == 7 && aopInReg (left->aop, shCount / 8, YH_IDX) && regDead (Y_IDX, ic) &&
    (result->aop->size == 1 && aopInReg (result->aop, 0, YL_IDX) && regDead (YH_IDX, ic) ||
    result->aop->size == 2 && aopInReg (result->aop, 0, Y_IDX)))
    {
      emit3 (A_SLLW, ASMOP_Y, 0);
      emit3 (A_CLRW, ASMOP_Y, 0);
      emit3 (A_RLCW, ASMOP_Y, 0);
      G.c = 0;
      goto release;
    }
  else if ((shCount % 8) == 7 && aopInReg (left->aop, shCount / 8, YH_IDX) && regDead (Y_IDX, ic))
    {
      emit3 (A_SLLW, ASMOP_Y, 0);
      emit3 (A_CLR, ASMOP_XL, 0);
      emit3 (A_RLC, ASMOP_XL, 0);
      G.c = 0;
      goto store_xl;
    }

  genMove_o (ASMOP_XL, 0, left->aop, shCount / 8, 1, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);
  shCount %= 8;

  emit2 ("and", "xl, #0x%02x", 1 << shCount);
  cost (2, 1);
  if (shCount)
    emit3 (A_BOOL, ASMOP_XL, 0);

store_xl:
  genMove (result->aop, ASMOP_XL, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));

release:
  if (pushed_xl)
    pop (ASMOP_XL, 0, 1);

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genGetByte - generates code to get a single byte                */
/*-----------------------------------------------------------------*/
static void
genGetByte (const iCode *ic)
{
  operand *left, *right, *result;
  int offset;

  D (emit2 ("; genGetByte", ""));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  offset = (int) ulFromVal (right->aop->aopu.aop_lit) / 8;
  genMove_o (result->aop, 0, left->aop, offset, 1, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);

  freeAsmop (result);
  freeAsmop (right);
  freeAsmop (left);
}

/*-----------------------------------------------------------------*/
/* genGetWord - generates code to get a 16-bit word                */
/*-----------------------------------------------------------------*/
static void
genGetWord (const iCode *ic)
{
  operand *left, *right, *result;
  int offset;

  D (emit2 ("; genGetWord", ""));

  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);
  result = IC_RESULT (ic);
  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  offset = (int) ulFromVal (right->aop->aopu.aop_lit) / 8;
  genMove_o (result->aop, 0, left->aop, offset, 2, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);

  freeAsmop (result);
  freeAsmop (right);
  freeAsmop (left);
}

/*-----------------------------------------------------------------*/
/* emitLeftShift - shifts asmop left by 1                          */
/*-----------------------------------------------------------------*/
static void emitLeftShift (asmop *aop, int offset, int size, bool rlc, bool xl_dead, bool *xl_pushed, int topbit)
{
  wassert (offset >= 0);

#if 0
  emit2(";", "emitLeftShift G.c %d topbit %d", G.c, topbit);
#endif

  for (int i = 0; i < size;)
    {
      int ri = i + offset;
      if (i + 1 < size && (aopIsAcc16 (aop, ri) || aopOnStackNotExt (aop, ri, 2)))
        {
          if (!i && !rlc && aopOnStackNotExt (aop, ri, 2)) // There is no on-stack sllw. Emulate it.
            {
              clrc ();
              emit3_o (A_RLCW, aop, ri, 0, 0);
            }
          else
            emit3_o ((i || rlc) ? A_RLCW : A_SLLW, aop, ri, 0, 0);
          i += 2;
        }
      else if (aopIsOp8_1 (aop, ri))
        {
          emit3_o ((i || rlc) ? A_RLC : A_SLL, aop, ri, 0, 0);
          i++;
        }
      else if (aop->regs[XL_IDX] < 0)
        {
          if (!xl_dead)
            {
              push (ASMOP_XL, 0, 1);
              *xl_pushed = true;
              xl_dead = true;
            }
          genMove_o (ASMOP_XL, 0, aop, ri, 1, true, false, false, false, !(i || rlc));
          emit3 ((i || rlc) ? A_RLC : A_SLL, ASMOP_XL, 0);
          genMove_o (aop, ri, ASMOP_XL, 0, 1, true, false, false, false, false);
          i++;
        }
      else
        {
          push (ASMOP_XL, 0, 1);
          genMove_o (ASMOP_XL, 0, aop, ri, 1, true, false, false, false, !(i || rlc));
          emit3 ((i || rlc) ? A_RLC : A_SLL, ASMOP_XL, 0);
          genMove_o (aop, ri, ASMOP_XL, 0, 1, true, false, false, false, false);
          pop (ASMOP_XL, 0, 1);
          i++;
        }
    }
  G.c = topbit;
}

/*-----------------------------------------------------------------*/
/* emitRightShift - shifts asmop right by 1                          */
/*-----------------------------------------------------------------*/
static void emitRightShift (asmop *aop, int offset, int size, bool rrc, bool sign, bool xl_dead, bool *xl_pushed)
{
  wassert (offset >= 0);

  for (int i = size - 1; i >= 0;)
    {
      int ri = i + offset;
      if (i > 0 && (aopIsAcc16 (aop, ri - 1) || (i != size - 1 || rrc || !sign) && aopOnStackNotExt (aop, ri - 1, 2)))
        {
          if (i == size - 1 && !rrc && aopOnStackNotExt (aop, ri - 1, 2)) // There is no on-stack srlw. Emulate it.
            {
              clrc ();
              emit3_o (A_RRCW, aop, ri - 1, 0, 0);
            }
          else
            emit3_o ((i != size - 1 || rrc) ? A_RRCW : (sign ? A_SRAW : A_SRLW), aop, ri - 1, 0, 0);
          i -= 2;
        }
      else if (i == size - 1 && sign) // sra needs special handling since it only supports few operands.
        {
          if (aopInReg (aop, ri, XL_IDX) || aopInReg (aop, ri, XH_IDX) || aopInReg (aop, ri, YL_IDX) || aopInReg (aop, ri, ZL_IDX))
            emit3_o (A_SRA, aop, ri, 0, 0);
          else if (aopOnStackNotExt (aop, ri, 1) && (!xl_dead || aop->regs[XL_IDX] >= 0))
            {
              emit3_o (A_XCH, ASMOP_XL, 0, aop, ri);
              emit3 (A_SRA, ASMOP_XL, 0);
              emit3_o (A_XCH, ASMOP_XL, 0, aop, ri);
            }
          else if (aop->regs[XL_IDX] < 0)
            {
              if (!xl_dead)
                {
                  if (*xl_pushed)
                    UNIMPLEMENTED;
                  push (ASMOP_XL, 0, 1);
                  *xl_pushed = true;
                  xl_dead = true;
                }
              genMove_o (ASMOP_XL, 0, aop, ri, 1, true, false, false, false, true);
              emit3 (A_SRA, ASMOP_XL, 0);
              genMove_o (aop, ri, ASMOP_XL, 0, 1, true, false, false, false, false);
            }
          else
            UNIMPLEMENTED;
          i--;
        }
      else if (aopIsOp8_1 (aop, ri) && (i != size - 1 || rrc || !sign))
        {
          emit3_o ((i != size - 1 || rrc) ? A_RRC : A_SRL, aop, ri, 0, 0);
          i--;
        }
      else if (aop->regs[XL_IDX] < 0)
        {
          if (!xl_dead)
            {
              push (ASMOP_XL, 0, 1);
              *xl_pushed = true;
              xl_dead = true;
            }
          genMove_o (ASMOP_XL, 0, aop, ri, 1, true, false, false, false, false);
          emit3 ((i != size - 1 || rrc) ? A_RRC : A_SRL, ASMOP_XL, 0);
          genMove_o (aop, ri, ASMOP_XL, 0, 1, true, false, false, false, false);
          i--;
        }
      else
        {
          push (ASMOP_XL, 0, 1);
          genMove_o (ASMOP_XL, 0, aop, ri, 1, true, false, false, false, false);
          emit3 ((i != size - 1 || rrc) ? A_RRC : A_SRL, ASMOP_XL, 0);
          genMove_o (aop, ri, ASMOP_XL, 0, 1, true, false, false, false, false);
          pop (ASMOP_XL, 0, 1);
          i--;
        }
    }
}

/*------------------------------------------------------------------*/
/* init_shiftop - find a good place to shift in                     */
/*------------------------------------------------------------------*/
static void 
init_shiftop(asmop *shiftop, const asmop *result, const asmop *left, const asmop *right, const iCode *ic)
{
  int i;
  const int size = result->size;
  // unsigned int shCount = right->type == AOP_LIT ? ulFromVal (right->aopu.aop_lit) : 0;

  shiftop->type = AOP_REGSTK;
  shiftop->size = size;
  memset (shiftop->regs, -1, sizeof(shiftop->regs));
  shiftop->valinfo.anything = true;

  for (i = 0; i < size;)
    {
      bool same_2_stack = aopOnStack (left, 0, 2) && aopOnStack (result, 0, 2) && left->aopu.bytes[i].byteu.stk == result->aopu.bytes[i].byteu.stk;
      // bool same_1_stack = aopOnStack (left, 0, 1) && aopOnStack (result, 0, 1) && left->aopu.bytes[i].byteu.stk == result->aopu.bytes[i].byteu.stk;

      if (aopInReg (left, i, Y_IDX) && regDead (Y_IDX, ic) && result->regs[YL_IDX] == -1 && result->regs[YH_IDX] == -1)
        {
          shiftop->aopu.bytes[i] = left->aopu.bytes[i];
          shiftop->aopu.bytes[i + 1] = left->aopu.bytes[i + 1];
          shiftop->regs[YL_IDX] = i;
          shiftop->regs[YH_IDX] = i + 1;
          i += 2;
        }
      else if (size == 2 && (aopOnStack (left, i, 2) || left->type == AOP_LIT) && aopOnStack (result, i, 2) && !same_2_stack && regDead (Y_IDX, ic) &&
        shiftop->regs[YL_IDX] == -1 && shiftop->regs[YH_IDX] == -1 &&
        left->regs[YL_IDX] == -1 && left->regs[YH_IDX] == -1 && result->regs[YL_IDX] == -1 && result->regs[YH_IDX] == -1)
        {
          shiftop->aopu.bytes[i] = ASMOP_Y->aopu.bytes[0];
          shiftop->aopu.bytes[i + 1] = ASMOP_Y->aopu.bytes[1];
          shiftop->regs[YL_IDX] = i;
          shiftop->regs[YH_IDX] = i + 1;
          i += 2;
        }
      else if (result->type != AOP_REGSTK)
        {
          memcpy (shiftop, result, sizeof(*shiftop));
          return;
        }
      else
        {
          shiftop->aopu.bytes[i] = result->aopu.bytes[i];
          if (result->type == AOP_REGSTK && result->aopu.bytes[i].in_reg)
            shiftop->regs[result->aopu.bytes[i].byteu.reg->rIdx] = i;
          i++;
        }
    }
}

/*-----------------------------------------------------------------*/
/* genRot - generates code for rotation                            */
/*-----------------------------------------------------------------*/
static void
genRot (const iCode *ic)
{
  operand *right = IC_RIGHT (ic);
  operand *left = IC_LEFT (ic);
  operand *result = IC_RESULT (ic);

  D (emit2 ("; genRot", ""));

  wassert (!(bitsForType (operandType (left)) % 8));
  wassert (IS_OP_LITERAL (right));

  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  int size = result->aop->size;
  int s = operandLitValueUll (right) % bitsForType (operandType (left));

  if (size == 1)
    {
      bool pushed_xl = false;
      asmop *rotaop = ASMOP_XL;
      if (aopSame (result->aop, 0, left->aop, 0, 1) && aopIsAcc8 (left->aop, 0) ||
        aopInReg (left->aop, 0, XL_IDX) && regDead (XL_IDX, ic))
        rotaop = left->aop;
      else if (aopIsAcc8 (result->aop, 0))
        rotaop = result->aop;
      if (!regDead (XL_IDX, ic) && aopInReg (rotaop, 0, XL_IDX))
        {
          push (ASMOP_XL, 0, 1);
          pushed_xl = true;
        }
      genMove (rotaop, left->aop, regDead (XL_IDX, ic) || pushed_xl, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      emit2 ("rot", "%s, #%d", aopGet(rotaop, 0), s);
      genMove (result->aop, rotaop, regDead (XL_IDX, ic) || pushed_xl, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      if (pushed_xl)
        pop (ASMOP_XL, 0, 1);
    }
  else if (size == 2)
    {
      asmop *rotaop = ASMOP_Y;
      if (aopSame (result->aop, 0, left->aop, 0, 2) && aopIsAcc16 (left->aop, 0))
        rotaop = left->aop;
      else if (regDead (Y_IDX, ic))
        rotaop = ASMOP_Y;
      else
        UNIMPLEMENTED;
      genMove (rotaop, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      while (s)
        if (s >= 8 && s <= 9)
          {
            emit3_o (A_XCH, rotaop, 0, rotaop, 1);
            s -= 8;
          }
        else
          {
            emit3 (A_SLLW, rotaop, 0);
            emit3 (A_ADCW, rotaop, 0);
            s--;
          }
      genMove (result->aop, rotaop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
    }
  else if (size == 4)
    {
      if (s != 16)
        UNIMPLEMENTED;
      asmop rotaop;
      signed char idxarray[5];
      if (result->aop->type == AOP_REGSTK &&
        result->aop->aopu.bytes[0].in_reg && result->aop->aopu.bytes[1].in_reg && result->aop->aopu.bytes[2].in_reg && result->aop->aopu.bytes[3].in_reg)
        {
          idxarray[0] = result->aop->aopu.bytes[2].byteu.reg->rIdx;
          idxarray[1] = result->aop->aopu.bytes[3].byteu.reg->rIdx;
          idxarray[2] = result->aop->aopu.bytes[0].byteu.reg->rIdx;
          idxarray[3] = result->aop->aopu.bytes[1].byteu.reg->rIdx;
          idxarray[4] = -1;
          f8_init_reg_asmop (&rotaop, idxarray);
          genMove (&rotaop, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
        }
      else if (left->aop->type == AOP_REGSTK &&
        left->aop->aopu.bytes[0].in_reg && left->aop->aopu.bytes[1].in_reg && left->aop->aopu.bytes[2].in_reg && left->aop->aopu.bytes[3].in_reg)
        {
          idxarray[0] = left->aop->aopu.bytes[2].byteu.reg->rIdx;
          idxarray[1] = left->aop->aopu.bytes[3].byteu.reg->rIdx;
          idxarray[2] = left->aop->aopu.bytes[0].byteu.reg->rIdx;
          idxarray[3] = left->aop->aopu.bytes[1].byteu.reg->rIdx;
          idxarray[4] = -1;
          f8_init_reg_asmop (&rotaop, idxarray);
          genMove (result->aop, &rotaop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
        }
      else
        {
          if (!regDead (X_IDX, ic))
            push (ASMOP_X, 0, 2);
          if (!regDead (Y_IDX, ic))
            push (ASMOP_Y, 0, 2);

          genMove (ASMOP_XY, left->aop, true, true, true, regDead (Z_IDX, ic));
          genMove (result->aop, ASMOP_YX, true, true, true, regDead (Z_IDX, ic));
 
          if (!regDead (X_IDX, ic) && (result->aop->regs[XL_IDX] >= 0 || result->aop->regs[XH_IDX] >= 0) ||
            !regDead (Y_IDX, ic) && (result->aop->regs[YL_IDX] >= 0 || result->aop->regs[YH_IDX] >= 0))
            {
              cost (300, 300);
              wassertl (regalloc_dry_run, "Swap result partially in non-dead x/y not implemented.");
            }

          if (!regDead (Y_IDX, ic))
            pop (ASMOP_Y, 0, 2);
          if (!regDead (X_IDX, ic))
            pop (ASMOP_X, 0, 2);
        }
    }
  else
    UNIMPLEMENTED;

  freeAsmop (left);
  freeAsmop (result);
  freeAsmop (right);
}

/*-----------------------------------------------------------------*/
/* genLeftShift - generates code for left shift                    */
/*-----------------------------------------------------------------*/
static void
genLeftShift (const iCode *ic)
{
  operand *right = IC_RIGHT (ic);
  operand *left = IC_LEFT (ic);
  operand *result = IC_RESULT (ic);

  D (emit2 ("; genLeftShift", ""));

  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  struct asmop shiftop_impl;
  struct asmop *shiftop = result->aop;
  struct valinfo v = left->aop->valinfo;

  int size = result->aop->size;
  bool premoved_count = false;

  // Avoid overwriting shift count on stack when moving to shiftop.
  if (aopOnStack (right->aop, 0, 1) && aopRS (shiftop))
    for (int i = 0; i < left->aop->size; i++)
      if (aopOnStack (shiftop, i, 1) && shiftop->aopu.bytes[i].byteu.stk == right->aop->aopu.bytes[0].byteu.stk)
        {
          genMove (ASMOP_XL, right->aop, true, regDead (XH_IDX, ic) && left->aop->regs[XH_IDX] < 0, false, false);
          premoved_count = true;
          if (left->aop->regs[XL_IDX] >= 0 || shiftop->regs[XL_IDX] >= 0)
            UNIMPLEMENTED;
          break;
        }

  if (right->aop->type == AOP_LIT)
    {
      int shCount = ulFromVal (right->aop->aopu.aop_lit);
      int offset = 0;
      bool rrc = false;
      if (shCount > (size * 8))
        shCount = size * 8;

      if (shCount <= 6)
        {
          shiftop = &shiftop_impl;
          init_shiftop (shiftop, result->aop, left->aop, right->aop, ic);
        }

      if (shCount % 8 == 7 && aopIsOp8_1 (left->aop, size - (shCount + 1) / 8) &&
        aopRS (left->aop) && left->aop->aopu.bytes[size - (shCount + 1) / 8].in_reg && regDead (left->aop->aopu.bytes[size - (shCount + 1) / 8].byteu.reg->rIdx, ic))
        {
          emit3_o (A_SRL, left->aop, size - (shCount + 1) / 8, 0, 0); // Move the one bit we still need into carry.
          genMove_o (shiftop, (shCount + 1) / 8, left->aop, 0, size - (shCount + 1) / 8, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);
          genMove_o (shiftop, 0, ASMOP_ZERO, 0, (shCount + 1) / 8, regDead (XL_IDX, ic) && shiftop->regs[XL_IDX] < 0, regDead (XH_IDX, ic) && shiftop->regs[XL_IDX] < 0, false, false, true);
          rrc = true;
        }
      else
        {
          genMove_o (shiftop, shCount / 8, left->aop, 0, size - shCount / 8, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);
          genMove_o (shiftop, 0, ASMOP_ZERO, 0, shCount / 8, regDead (XL_IDX, ic) && shiftop->regs[XL_IDX] < 0, regDead (XH_IDX, ic) && shiftop->regs[XL_IDX] < 0, false, false, true);
        }
      
      if (shCount >= 16 && // Skip known 0 word when shifting.
        (aopOnStack (shiftop, 0, 2) || aopInReg (shiftop, 0, Y_IDX) || aopInReg (shiftop, 0, X_IDX) || aopInReg (shiftop, 0, Z_IDX) || shiftop->type == AOP_DIR))
        {
          offset += 2;
        }

      if (shCount > 8)
        v.anything = true;
      shCount %= 8;

      bool xl_pushed = false;
      bool xl_free = regDead (XL_IDX, ic) && shiftop->regs[XL_IDX] < 0;
      if (rrc)
        emitRightShift (shiftop, offset, size - offset, true, false, xl_free, &xl_pushed);
      else if (size - offset == 2 && aopInReg (shiftop, offset, Y_IDX) && xl_free && shCount >= 4)
        {
          emit2 ("ld", "xl, #%d", shCount);
          emit2 ("sllw", "y, xl");
          cost (2, 4);
          spillReg (C_IDX);
        }
      else
        for (int c = 0; c < shCount; c++)
          {
            int topbit = -1;
            if (!v.anything)
              {
                unsigned long long topbitmask = 1ull << (size * 8 - 1);
                if (v.knownbitsmask & topbitmask)
                  topbit = (bool)(v.knownbits & topbitmask);
                v.knownbitsmask <<= 1;
                v.knownbits <<= 1;
              }
            emitLeftShift (shiftop, offset, size - offset, false, xl_free, &xl_pushed, topbit);
            xl_free |= xl_pushed;
          }
      if (xl_pushed)
        pop (ASMOP_XL, 0, 1);
    }
  else if (size == 2 && regDead (Y_IDX, ic) && aopOnStack (result->aop, 0, 1) && aopOnStack (left->aop, 0, 1) &&
    (premoved_count || aopInReg (right->aop, 0, XL_IDX) || regDead (XL_IDX, ic) && aopOnStack (right->aop, 0, 1)))
    {
      genMove (ASMOP_Y, left->aop, false, false, true, regDead (Z_IDX, ic) && right->aop->regs[ZL_IDX] < 0 && right->aop->regs[ZH_IDX] < 0);
      if (!premoved_count)
        genMove (ASMOP_XL, right->aop, true, regDead (XH_IDX, ic), false, regDead (Z_IDX, ic));
      emit2 ("sllw", "y, xl");
      cost (2, 1);
      genMove (result->aop, ASMOP_Y, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
    }
  else if (regDead (XL_IDX, ic) && shiftop->regs[XL_IDX] < 0)
    {
      bool xl_dead = regDead (XL_IDX, ic) && (premoved_count ? false : (right->aop->regs[XL_IDX] < 0));
      bool xh_dead = regDead (XH_IDX, ic) && (right->aop->regs[XH_IDX] < 0 || premoved_count);
      bool yl_dead = regDead (YL_IDX, ic) && (right->aop->regs[YL_IDX] < 0 || premoved_count);
      bool yh_dead = regDead (YH_IDX, ic) && (right->aop->regs[YH_IDX] < 0 || premoved_count);
      bool y_dead = yl_dead && yh_dead;

      genMove (shiftop, left->aop, xl_dead, xh_dead, y_dead, false);

      symbol *tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (0));
      symbol *tlbl2 = (regalloc_dry_run ? 0 : newiTempLabel (0));

      if (!premoved_count && aopRS (right->aop) && right->aop->aopu.bytes[0].in_reg && shiftop->regs[right->aop->aopu.bytes[0].byteu.reg->rIdx] >= 0 || shiftop->regs[XL_IDX] >= 0) // Right register operand overwritten by result
        UNIMPLEMENTED;
      if (!premoved_count)
        genMove (ASMOP_XL, right->aop, true, regDead (XH_IDX, ic) && shiftop->regs[XH_IDX] < 0, false, false);

      if (size == 2 && aopInReg (shiftop, 0, Y_IDX) || size == 1 && aopInReg (shiftop, 0, YL_IDX) && regDead (YH_IDX, ic))
        {
          emit2 ("sllw", "y, xl");
          cost (2, 1);
          spillReg (C_IDX);
          goto shifted;
        }

      if (!aopIsLitVal (right->aop, 0, 1, 0))
        {
          emit3 (A_TST, ASMOP_XL, 0);
          if (tlbl2)
            emit2 ("jrz", "#!tlabel", labelKey2num (tlbl2->key));
          cost (2, 1);
        }
        
      emitLabel (tlbl1);
      spillReg (C_IDX);

      bool xl_pushed = false;
      emitLeftShift (shiftop, 0, size, false, false, &xl_pushed, -1);
      if (xl_pushed)
        pop (ASMOP_XL, 0, 1);

      emit3 (A_DEC, ASMOP_XL, 0);
      if (tlbl1)
        emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl1->key));
      cost (2, 1);
      emitLabel (tlbl2);
      spillReg (C_IDX);
    }
  else
    UNIMPLEMENTED;

shifted:
  genMove (result->aop, shiftop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));

  sym_link *resulttype = operandType (IC_RESULT (ic));
  unsigned topbytemask = (IS_BITINT (resulttype) && SPEC_USIGN (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;
  bool maskedtopbyte = (topbytemask != 0xff);
  if (maskedtopbyte)
    {
      if (aopIsAcc8 (result->aop, result->aop->size - 1))
        {
          emit2 ("and", "%s, #0x%02x", aopGet (result->aop, result->aop->size - 1), topbytemask);
          cost (2 + !aopInReg (result->aop, result->aop->size - 1, XL_IDX), 1);
        }
      else if (topbytemask == 0x7f && aopOnStackNotExt (result->aop, result->aop->size - 1, 1))
        {
          emit3_o (A_SLL, result->aop, result->aop->size - 1, 0, 0);
          emit3_o (A_SRL, result->aop, result->aop->size - 1, 0, 0);
        }
      else
        {
          bool pushed_xl = false;
          if (!regDead (XL_IDX, ic) || result->aop->regs[XL_IDX] >= 0 && result->aop->regs[XL_IDX] != result->aop->size - 1)
            {
              push (ASMOP_XL, 0, 1);
              pushed_xl = true;
            }
          if (aopOnStack (result->aop, result->aop->size - 1, 1) || result->aop->type == AOP_DIR)
            {
              emit2 ("ld", "xl, #0x%02x", topbytemask);
              cost (2, 1);
              emit3_o (A_AND, result->aop, result->aop->size - 1, ASMOP_XL, 0);
            }
          else
            {
              genMove_o (ASMOP_XL, 0, result->aop, result->aop->size - 1, 1, true, false, false, false, true);
              emit2 ("and", "xl, #0x%02x", topbytemask);
              cost (2, 1);
              genMove_o (result->aop, result->aop->size - 1, ASMOP_XL, 0, 1, true, false, false, false, true);
            }
          if (pushed_xl)
            pop (ASMOP_XL, 0, 1);
        }
    }

  freeAsmop (left);
  freeAsmop (result);
  freeAsmop (right);
}

/*-----------------------------------------------------------------*/
/* genRightShift - generates code for right shift                  */
/*-----------------------------------------------------------------*/
static void
genRightShift (const iCode *ic)
{
  operand *right = IC_RIGHT (ic);
  operand *left = IC_LEFT (ic);
  operand *result = IC_RESULT (ic);

  D (emit2 ("; genRightShift", ""));

  aopOp (left, ic, false);
  aopOp (right, ic, false);
  aopOp (result, ic, true);

  struct asmop shiftop_impl;
  struct asmop *shiftop = result->aop;

  bool sign =  !SPEC_USIGN (getSpec (operandType (left)));
  int size = result->aop->size;
  bool premoved_count = false;

  // Avoid overwriting shift count on stack when moving to shiftop.
  if (aopOnStack (right->aop, 0, 1) && aopRS (shiftop))
    for (int i = 0; i < left->aop->size; i++)
      if (aopOnStack (shiftop, i, 1) && shiftop->aopu.bytes[i].byteu.stk == right->aop->aopu.bytes[0].byteu.stk)
        {
          genMove (ASMOP_XL, right->aop, true, regDead (XH_IDX, ic) && left->aop->regs[XH_IDX] < 0, false, false);
          premoved_count = true;
          if (left->aop->regs[XL_IDX] >= 0 || shiftop->regs[XL_IDX] >= 0)
            UNIMPLEMENTED;
          break;
        }

  if (right->aop->type == AOP_LIT)
    {
      int shCount = ulFromVal (right->aop->aopu.aop_lit);
      bool rlc = false;
      if (shCount > (size * 8))
        shCount = size * 8;

      if (sign && shCount > 4 && size > 2) // todo: better estimate, that also takes optimization goals into account.
        goto makeloop;

      if (shCount <= 6)
        {
          shiftop = &shiftop_impl;
          init_shiftop (shiftop, result->aop, left->aop, right->aop, ic);
        }

      if (!sign)
        {
          if (shCount % 8 == 7 && !sign && aopIsOp8_1 (left->aop, shCount / 8) &&
            aopRS (left->aop) && left->aop->aopu.bytes[size - (shCount + 1) / 8].in_reg && regDead (left->aop->aopu.bytes[size - (shCount + 1) / 8].byteu.reg->rIdx, ic))
            {
              emit3_o (A_SLL, left->aop, shCount / 8, 0, 0); // Move the one bit we still need into carry.
              genMove_o (shiftop, 0, left->aop, (shCount + 1) / 8, size - (shCount + 1) / 8, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), false);
              genMove_o (shiftop, size - (shCount + 1) / 8, ASMOP_ZERO, 0, (shCount + 1) / 8, regDead (XL_IDX, ic) && shiftop->regs[XL_IDX] < 0, regDead (XH_IDX, ic) && shiftop->regs[XL_IDX] < 0, false, false, false);
              rlc = true;
            }
          else
            {
              genMove_o (shiftop, 0, left->aop, shCount / 8, size - shCount / 8, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);
              genMove_o (shiftop, size - shCount / 8, ASMOP_ZERO, 0, shCount / 8, regDead (XL_IDX, ic) && shiftop->regs[XL_IDX] < 0, regDead (XH_IDX, ic) && shiftop->regs[XL_IDX] < 0, false, false, true);
            }
          if (shCount >= 16 && // Skip known 0 word when shifting.
            (aopOnStack (shiftop, size - 2, 2) || aopInReg (shiftop, size - 2, Y_IDX) || aopInReg (shiftop, size - 2, X_IDX) || aopInReg (shiftop, size - 2, Z_IDX) || shiftop->type == AOP_DIR))
            size -= 2;
          shCount %= 8;
        }
      else
        genMove (shiftop, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));

      if(shCount == 7 && !sign && size == 4 && aopInReg (shiftop, 0, Y_IDX) && (aopInReg (shiftop, 2, X_IDX) || aopInReg (shiftop, 2, Z_IDX)))
        {
          emit3_o (A_SLL, ASMOP_Y, 0, 0, 0);
          emit3_o (A_XCH, ASMOP_Y, 0, ASMOP_Y, 1);
          emit3_o (A_LD, ASMOP_Y, 1, shiftop, 2);
          emit3_o (A_LD, shiftop, 2, shiftop, 3);
          emit3_o (A_CLR, shiftop, 3, 0, 0);
          rlc = true;
        }

      bool xl_pushed = false;
      bool xl_free = regDead (XL_IDX, ic);
      if (rlc)
        emitLeftShift (shiftop, 0, size, true, xl_free, &xl_pushed, -1);
      else if (size == 1 && !sign && shCount >= 4 && aopIsAcc8 (shiftop, 0))
        {
          emit2 ("rot", "%s, #%d", aopGet (shiftop, 0), 8 - shCount);
          emit2 ("and", "%s, #0x%02x", aopGet (shiftop, 0), 0xff >> shCount);
          cost (4 + 2 * !aopInReg (shiftop, 0, XL_IDX), 2);
        }
      else
        for (int c = 0; c < shCount; c++)
          {
            xl_free |= xl_pushed;
            emitRightShift (shiftop, 0, size, false, sign, xl_free, &xl_pushed);
          }
      if (xl_pushed)
        pop (ASMOP_XL, 0, 1);
    }
  else
    {
makeloop:
      ;
      bool xl_dead = regDead (XL_IDX, ic) && (premoved_count ? false : (right->aop->regs[XL_IDX] < 0));
      bool xh_dead = regDead (XH_IDX, ic) && (right->aop->regs[XH_IDX] < 0 || premoved_count);
      bool yl_dead = regDead (YL_IDX, ic) && (right->aop->regs[YL_IDX] < 0 || premoved_count);
      bool yh_dead = regDead (YH_IDX, ic) && (right->aop->regs[YH_IDX] < 0 || premoved_count);
      bool y_dead = yl_dead && yh_dead;

      symbol *tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (0));
      symbol *tlbl2 = (regalloc_dry_run ? 0 : newiTempLabel (0));

      bool xl_pushed = false;

      genMove (shiftop, left->aop, xl_dead, xh_dead, y_dead, false);

      if (!regDead (XL_IDX, ic))
        {
          push (ASMOP_XL, 0, 1);
          xl_pushed = true;
        }
      if (!premoved_count && aopRS (right->aop) && right->aop->aopu.bytes[0].in_reg && shiftop->regs[right->aop->aopu.bytes[0].byteu.reg->rIdx] >= 0 || shiftop->regs[XL_IDX] >= 0) // Right register operand overwritten by result
        UNIMPLEMENTED;
      if (!premoved_count)
        genMove (ASMOP_XL, right->aop, true, regDead (XH_IDX, ic) && shiftop->regs[XH_IDX] < 0, false, false);

      if (!aopIsLitVal (right->aop, 0, 1, 0))
        {
          emit3 (A_TST, ASMOP_XL, 0);
          if (tlbl2)
            emit2 ("jrz", "#!tlabel", labelKey2num (tlbl2->key));
          cost (2, 1);
        }
        
      emitLabel (tlbl1);
      spillReg (C_IDX);

      emitRightShift (shiftop, 0, size, false, sign, false, &xl_pushed);
      if (xl_pushed && regDead (XL_IDX, ic))
        pop (ASMOP_XL, 0, 1);

      emit3 (A_DEC, ASMOP_XL, 0);
      if (tlbl1)
        emit2 ("jrnz", "#!tlabel", labelKey2num (tlbl1->key));
      cost (2, 1);
      emitLabel (tlbl2);
      spillReg (C_IDX);
      if (xl_pushed && !regDead (XL_IDX, ic))
        pop (ASMOP_XL, 0, 1);
    }

  genMove (result->aop, shiftop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));

  freeAsmop (left);
  freeAsmop (result);
  freeAsmop (right);
}

/*------------------------------------------------------------------*/
/* init_stackop - initalize asmop for stack location                */
/*------------------------------------------------------------------*/
static void 
init_stackop (asmop *stackop, int size, long int stk_off)
{
  stackop->size = size;
  memset (stackop->regs, -1, sizeof (stackop->regs));

  if (labs(stk_off) > (1 << 15))
  {
    if (!regalloc_dry_run)
      werror (W_INVALID_STACK_LOCATION);
    stk_off = 0;
  }

  for (int i = 0; i < size; i++)
    {
      stackop->aopu.bytes[i].in_reg = false;
      stackop->aopu.bytes[i].byteu.stk = stk_off + i;
    }

  stackop->type = AOP_STK;
  stackop->valinfo.anything = true;
}

// Shifting, masking, and sign-extending of top bit-field byte.
static void
handle_bitfield_topbyte_in_xl (int blen, int bstr, bool sign_extend, bool xh_dead)
{
  // Shift
  if (blen < 8) 
    {
      if (bstr <= 1)
        while (bstr--)
          emit3 (A_SRL, ASMOP_XL, 0);
      else
        {
          emit2 ("rot", "xl, #%d", 8 - bstr);
          cost (2, 1);
        }
    }

  if (blen == 8)
    return;

  // Mask
  emit2 ("and", "xl, #0x%02x", 0xff >> (8 - blen));
  cost (2, 1);

  if (!sign_extend)
    return;

  // Sign-extend
  symbol *const tlbl = (regalloc_dry_run ? 0 : newiTempLabel (0));
  if (blen == 1) // The and above already set the z flag.
    {
      if (tlbl)
        emit2 ("jrz", "#!tlabel", labelKey2num (tlbl->key));
      cost (2, 0);
    }
  else if (blen != 1) 
    {
      emit2 ("cp", "xl, #0x%02x", 0x80 >> (8 - blen));
      cost (2, 1);
      if (tlbl)
        emit2 ("jrnc", "#!tlabel", labelKey2num (tlbl->key));
      cost (2, 0);
    }
  emit2 ("or", "xl, #0x%02x", (0xff00 >> (8 - blen)) & 0xff);
  cost (2, 1);
  emitLabel (tlbl);
}

/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for read via pointer              */
/*-----------------------------------------------------------------*/
static void
genPointerGet (const iCode *ic, iCode *ifx)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  bool use_z = false;
  bool blockmove = false;
  int i = 0;

  bool bit_field = IS_BITVAR (operandType (result));
  int blen = bit_field ? SPEC_BLEN (getSpec (operandType (result))) : 0;
  int bstr = bit_field ? SPEC_BSTR (getSpec (operandType (result))) : 0;
  
  D (emit2 ("; genPointerGet", ""));

  aopOp (ic->left, ic, false);
  aopOp (ic->right, ic, false);
  aopOp (ic->result, ic, true);

  if (result->aop->type == AOP_DUMMY)
    D (emit2 ("; Dummy read", ""));

  wassertl (right, "GET_VALUE_AT_ADDRESS without right operand");
  wassertl (IS_OP_LITERAL (IC_RIGHT (ic)), "GET_VALUE_AT_ADDRESS with non-literal right operand");

  int size = result->aop->size;
  long int offset = operandLitValueUll (IC_RIGHT(ic));

  bool y_dead = regDead (Y_IDX, ic) && right->aop->regs[YL_IDX] < 0 && right->aop->regs[YH_IDX] < 0;

  if (ifx && result->aop->type == AOP_CND && (!bit_field || blen == 8))
    {
      wassert (size <= 2 || bit_field);
      bool wide = (size > 1 && !bit_field);
      if (left->aop->type == AOP_LIT)
        {
          emit2 (wide ? "tstw" : "tst", offset ? "0x%02x%02x+%d" : "0x%02x%02x", byteOfVal (left->aop->aopu.aop_lit, 1), byteOfVal (left->aop->aopu.aop_lit, 0), (int)(offset));
          cost (3 + wide ? !aopInReg (result->aop, 0, Y_IDX) : !aopInReg (result->aop, 0, XL_IDX), 1);
        }
      else if (left->aop->type == AOP_IMMD)
        {
          emit2 (wide ? "tstw" : "tst", offset ? "%s+%d" : "%s+%d", left->aop->aopu.immd, (int)(left->aop->aopu.immd_off + offset));
          cost (3 + wide ? !aopInReg (result->aop, 0, Y_IDX) : !aopInReg (result->aop, 0, XL_IDX), 1);
        }
      else if (wide && left->aop->type == AOP_STL)
        {
          struct asmop stackop_impl;
          init_stackop (&stackop_impl, size, left->aop->aopu.stk_off + offset);
          emit3 (A_TSTW, &stackop_impl, 0);
        }
      else if (wide && aopInReg (left->aop, 0, Z_IDX))
        {
          emit2 ("tstw", "(%u, z)", (unsigned int)offset);
          cost (3, 1);
        }
      else if (wide && (y_dead || aopInReg (left->aop, 0, Y_IDX)))
        {
          genMove (ASMOP_Y, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
          if (!offset)
            {
              emit2 ("ldw", "y, (y)");
              cost (1, 1);
            }
          else
            {
              emit2 ("ldw", "y, (%u, y)", (unsigned int)offset);
              cost (2 + (offset > 255), 1);
            }
        }
      else if (!wide && (y_dead || aopInReg (left->aop, 0, Y_IDX)) && offset <= 255)
        {
          genMove (ASMOP_Y, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
          emit2 ("tst", "(%u, y)", (unsigned int)offset);
        }
      else if (wide && regDead (Z_IDX, ic))
        {
          genMove (ASMOP_Z, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), y_dead, true);
          emit2 ("tstw", "(%u, z)", (unsigned int)offset);
          cost (3, 1);
        }
      else
        UNIMPLEMENTED;
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      if (!regalloc_dry_run)
        emit2 (IC_TRUE (ifx) ? "jrz" : "jrnz", "#!tlabel", labelKey2num (tlbl->key));
      cost (2, 1);
      emitJP(IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
      emitLabel (tlbl);
      goto release;
    }
  else if (ifx && result->aop->type == AOP_CND && bit_field)
    {
      wassert (blen <= 8);
      if (!regDead (XL_IDX, ic))
        UNIMPLEMENTED;
      else if (left->aop->type == AOP_LIT)
        emit2 ("ld", offset ? "xl, 0x%02x%02x+%d" : "xl, 0x%02x%02x", byteOfVal (left->aop->aopu.aop_lit, 1), byteOfVal (left->aop->aopu.aop_lit, 0), (int)(offset));
      else if (left->aop->type == AOP_IMMD)
        emit2 ("ld", offset ? "xl, %s+%d" : "xl, %s+%d", left->aop->aopu.immd, (int)(left->aop->aopu.immd_off + offset));
      else if (aopInReg (left->aop, 0, Z_IDX))
        {
          emit2 ("ld", "xl, (%u, z)", (unsigned int)offset);
          cost (3, 1);
        }
      else if ((y_dead || aopInReg (left->aop, 0, Y_IDX)) && offset <= 255)
        {
          genMove (ASMOP_Y, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
          if (!offset)
            {
              emit2 ("ld", "xl, (y)");
              cost (1, 1);
            }
          else
            {
              emit2 ("ld", "xl, (%u, y)", (unsigned int)offset);
              cost (2, 1);
            }
        }
      else if (regDead (Z_IDX, ic))
        {
          genMove (ASMOP_Z, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), y_dead, true);
          emit2 ("ld", "xl, (%u, z)", (unsigned int)offset);
          cost (3, 1);
        }
      else
        UNIMPLEMENTED;
      emit2 ("and", "xl, #0x%02x", 0xff >> (8 - blen) << bstr);
      cost (2, 1);
      symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
      if (!regalloc_dry_run)
        emit2 (IC_TRUE (ifx) ? "jrz" : "jrnz", "#!tlabel", labelKey2num (tlbl->key));
      cost (2, 1);
      emitJP(IC_TRUE (ifx) ? IC_TRUE (ifx) : IC_FALSE (ifx), 0.5f);
      emitLabel (tlbl);
      goto release;
    }
  else if (!bit_field && (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD) && result->aop->type != AOP_DUMMY &&
    (size == 1 && aopIsAcc8 (result->aop, 0) || size == 2 && aopIsAcc16 (result->aop, 0)))
    {
      bool wide = size > 1;
      if (left->aop->type == AOP_LIT)
        emit2 (wide ? "ldw" : "ld", offset ? "%s, 0x%02x%02x+%d" : "%s, 0x%02x%02x", wide ? aopGet2 (result->aop, 0) : aopGet (result->aop, 0), byteOfVal (left->aop->aopu.aop_lit, 1), byteOfVal (left->aop->aopu.aop_lit, 0), (int)(offset));
      else
        emit2 (wide ? "ldw" : "ld", offset ? "%s, %s+%d" : "%s, %s+%d", wide ? aopGet2 (result->aop, 0) : aopGet (result->aop, 0), left->aop->aopu.immd, (int)(left->aop->aopu.immd_off + offset));
      cost (3 + wide ? !aopInReg (result->aop, 0, Y_IDX) : !aopInReg (result->aop, 0, XL_IDX), 1);
      goto release;
    }
  else if (!bit_field && left->aop->type == AOP_STL && result->aop->type != AOP_DUMMY)
    {
      struct asmop stackop_impl;
      init_stackop (&stackop_impl, result->aop->size, left->aop->aopu.stk_off + offset);
      genMove (result->aop, &stackop_impl, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      goto release;
    }
  else if (bit_field && (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD) && result->aop->type != AOP_DUMMY && blen <= 8 && aopInReg (result->aop, 0, XL_IDX))
    {
      if (left->aop->type == AOP_LIT)
        emit2("ld", offset ? "%s, 0x%02x%02x+%d" : "%s, 0x%02x%02x", aopGet (result->aop, 0), byteOfVal (left->aop->aopu.aop_lit, 1), byteOfVal (left->aop->aopu.aop_lit, 0), (unsigned int)offset);
      else
        emit2("ld", offset ? "%s, %s+%d" : "%s, %s+%d", aopGet (result->aop, 0), left->aop->aopu.immd, (int)(left->aop->aopu.immd_off + offset));
      cost (3, 1);
      handle_bitfield_topbyte_in_xl (blen, bstr, !SPEC_USIGN (getSpec (operandType (result))), regDead (XH_IDX, ic) && result->aop->regs[XH_IDX] < 0);
      i = 1;
      goto extend_bitfield;
    }
  else if (!bit_field && size == 2 && !offset && aopIsAcc16 (result->aop, 0) && aopIsAcc16 (left->aop, 0) &&
    !(aopInReg (result->aop, 0, Y_IDX) && aopInReg (left->aop, 0, X_IDX) || aopInReg (result->aop, 0, X_IDX) && aopInReg (left->aop, 0, Z_IDX)))
    {
      if (!regalloc_dry_run) // Save time of memory (de)allocation.
        {
          char *laopstr = Safe_strdup (aopGet2 (left->aop, 0));
          emit2 ("ldw", "%s, (%s)", aopGet2 (result->aop, 0), laopstr);
          Safe_free (laopstr);
        }
      cost (((aopInReg (result->aop, 0, X_IDX) || aopInReg (result->aop, 0, Y_IDX)) && aopInReg (left->aop, 0, Y_IDX)) ? 1 : 2, 1);
      goto release;
    }

  if (!bit_field && size >= 4 + (bool)offset && (aopOnStack (result->aop, 0, size) || result->aop->type == AOP_DIR) &&
    regDead (Y_IDX, ic) && regDead (Z_IDX, ic))
    {
      genMove (ASMOP_Z, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, true);
      if (offset)
        {
          emit2 ("addw", "z, #%ld", offset);
          cost (3, 1);
          spillReg (C_IDX);
          offset = 0;
        }
      use_z = true;
      blockmove = true;
    }
  else if (aopInReg (left->aop, 0, Z_IDX) && !bit_field && size <= 2 && (aopInReg (result->aop, size - 2, Z_IDX) || result->aop->regs[ZL_IDX] < 0 && result->aop->regs[ZH_IDX] < 0))
    use_z = true;
  else if (aopInReg (left->aop, 0, Y_IDX) && (unsigned)offset + size - 1 <= 255 && (size >= 2 && aopInReg (result->aop, size - 2, Y_IDX) || result->aop->regs[YL_IDX] < 0 && result->aop->regs[YH_IDX] < 0))
    ;
  else if (y_dead && (size >= 2 && aopInReg (result->aop, size - 2, Y_IDX) ||
                      size >= 4 && !bit_field && aopInReg (result->aop, size - 4, Y_IDX) && aopIsAcc16 (result->aop, size - 2) ||
                      result->aop->regs[YL_IDX] < 0 && result->aop->regs[YH_IDX] < 0 ))
    {
      if (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD)
        {
          emit2 ("ldw", "y, %s+%d", aopGet2 (left->aop, 0), offset);
          cost (3, 1);
          offset = 0;
        }
      else
        genMove (ASMOP_Y, left->aop, false, false, y_dead, false);
      if ((unsigned)offset + size - 1 >= 255)
        {
          emit2 ("addw", "y, #%ld", offset);
          cost (2 + (offset > 127), 1);
          spillReg (C_IDX);
          offset = 0;
        }
    }
  else
    UNIMPLEMENTED;

  if (blockmove)
    {
      wassert (!bit_field && !offset && use_z);
      pointToSym (ASMOP_Y, OP_SYMBOL_CONST (result), 0,  regDead (XL_IDX, ic), regDead (XH_IDX, ic), false, regDead (Z_IDX, ic));
      for(int i = 0; i < size;)
        if (i + 1 < size)
          {
            emit2 ("ldwi", "(%d, y), (z)", i);
            cost (2, 1);
            i += 2;
          }
        else
          {
            emit2 ("ldi", "(%d, y), (z)", i);
            cost (2, 1);
            i++;
          }
      goto release;
    }

  for (i = 0; !bit_field ? i < size : blen > 0; i++, blen -= 8)
    {
      bool xl_dead = regDead (XL_IDX, ic) && (result->aop->regs[XL_IDX] < 0 || result->aop->regs[XL_IDX] >= i);
      bool xh_dead = regDead (XH_IDX, ic) && (result->aop->regs[XH_IDX] < 0 || result->aop->regs[XH_IDX] >= i);
      bool x_dead = xl_dead && xh_dead;

      if (i + 4 == size && !use_z && !bit_field &&
        aopInReg (result->aop, i, Y_IDX) && (aopInReg (result->aop, i + 2, X_IDX) || aopInReg (result->aop, i + 2, Z_IDX)))
        {
          emit2 ("ldw", "%s, (%d, y)", aopGet2 (result->aop, i + 2), offset + i + 2);
          emit2 ("ldw", (offset + i) ? "y, (%d, y)" : "y, (y)", offset + i);
          cost (4 + (bool)(offset + i), 2);
          i += 4;
          continue;
        }
      if ((!bit_field && i + 2 <= size || blen >= 16) &&
        (aopInReg (result->aop, i, Y_IDX) || aopInReg (result->aop, i, X_IDX) || aopInReg (result->aop, i, Z_IDX) ||
          (i + 2 == size && regDead (Y_IDX, ic) || x_dead) && (aopOnStack (result->aop, i, 2) || result->aop->type == AOP_DIR)))
        {
          asmop *taop = ASMOP_Y;
          if ((aopOnStack (result->aop, i, 2) || result->aop->type == AOP_DIR) && x_dead &&
            !(use_z && regDead (Y_IDX, ic)) && !(i + 2 == size && regDead (Y_IDX, ic)))
            taop = ASMOP_X;
          else if (aopInReg (result->aop, i, X_IDX))
            taop = ASMOP_X;
          else if (aopInReg (result->aop, i, Z_IDX))
            taop = ASMOP_Z;

          if (!(offset + i) && !use_z && aopIsAcc16 (taop, i))
            {
              emit2 ("ldw", "%s, (y)", aopGet2 (taop, 0));
              cost (1 + aopInReg (taop, i, Z_IDX), 1);
            }
          else
            {
              emit2 ("ldw", use_z ? "%s, (%d, z)" : "%s, (%d, y)", aopGet2 (taop, 0), (int)(offset + i));
              cost (2 + (use_z || offset + i > 255) + !aopInReg (taop, 0, Y_IDX), 1);
            }
          genMove_o (result->aop, i, taop, 0, 2, xl_dead, false, true, false, true);
          i++, blen -= 8;
          continue;
        }

      if (result->aop->regs[use_z ? ZL_IDX : YL_IDX] > 0 && result->aop->regs[use_z ? ZL_IDX : YL_IDX] < i || result->aop->regs[use_z ? ZH_IDX : YH_IDX] > 0 && result->aop->regs[use_z ? ZH_IDX : YH_IDX] < i)
        UNIMPLEMENTED;

      if ((!bit_field || blen >= 8) &&
        (aopIsAcc8 (result->aop, i)))
        {
          if (i + 1 < size &&
            (!use_z && (aopInReg (result->aop, i, YL_IDX) || aopInReg (result->aop, i, YH_IDX)) ||
            use_z && (aopInReg (result->aop, i, ZL_IDX) || aopInReg (result->aop, i, ZH_IDX))))
            UNIMPLEMENTED;
            
          if (!(offset + i) && !use_z && (aopInReg (result->aop, i, XL_IDX) || aopInReg (result->aop, i, XH_IDX) || aopInReg (result->aop, i, ZH_IDX)))
            {
              emit2 ("ld", "%s, (y)", aopGet (result->aop, i));
              cost (2, 1);
            }
          else
            {
              emit2 ("ld", use_z ? "%s, (%d, z)" : "%s, (%d, y)", aopGet (result->aop, i), (int)(offset + i));
              cost (3 + (use_z || offset + i > 255), 1);
            }
          continue;
        }

      if (!xl_dead)
        UNIMPLEMENTED;

      if (!(offset + i) && !use_z)
        {
          emit2 ("ld", "xl, (y)");
          cost (1, 1);
        }
      else
        {
          emit2 ("ld", use_z ? "xl, (%d, z)" : "xl, (%d, y)", (int)(offset + i));
          cost (2 + (use_z || offset + i > 255), 1);
        }

      if (bit_field && blen <= 8) // Sign extension for partial byte of signed bit-field
        handle_bitfield_topbyte_in_xl (blen, bstr, !SPEC_USIGN (getSpec (operandType (result))), regDead (XH_IDX, ic) && result->aop->regs[XH_IDX] < 0);

      if (result->aop->type == AOP_DUMMY) // Pointer dereference where the result is ignored, but wasn't optimized out (typically due to use of volatile).
        continue;
      if ((!bit_field ? i + 1 < size : blen - 8 > 0) && (aopInReg (result->aop, i, use_z ? ZL_IDX : YL_IDX) || aopInReg (result->aop, i, use_z ? ZH_IDX : YH_IDX)))
        UNIMPLEMENTED;
      genMove_o (result->aop, i, ASMOP_XL, 0, 1, true, false, false, false, true);
    }

extend_bitfield:
  if (bit_field && i < size)
    {
      if (SPEC_USIGN (getSpec (operandType (result))))
        genMove_o (result->aop, i, ASMOP_ZERO, 0, i, regDead (XL_IDX, ic) && result->aop->regs[XL_IDX] < 0, regDead (XH_IDX, ic) && result->aop->regs[XH_IDX] < 0, false, false, true);
      else
        wassertl (0, "Unimplemented multibyte sign extension for bit-field.");
    }
release:
  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genPointerSet - generate code for write via pointer             */
/*-----------------------------------------------------------------*/
static void
genPointerSet (const iCode *ic)
{
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);

  bool bit_field = IS_BITVAR (operandType (left)->next);
  int blen = bit_field ? (SPEC_BLEN (getSpec (operandType (IS_BITVAR (getSpec (operandType (right))) ? right : left)))) : 0;
  int bstr = bit_field ? (SPEC_BSTR (getSpec (operandType (IS_BITVAR (getSpec (operandType (right))) ? right : left)))) : 0;

  bool pushed_y = false;
  bool pushed_xl = false;

  D (emit2 ("; genPointerSet", ""));

  aopOp (left, ic, false);
  aopOp (right, ic, false);

  int size = right->aop->size;

  bool y_dead = regDead (Y_IDX, ic) && right->aop->regs[YL_IDX] < 0 && right->aop->regs[YH_IDX] < 0;

  if (!bit_field && (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD) && size <= 2 && aopIsLitVal (right->aop, 0, size, 0x0000)) // Use clr / clrw mm.
    {
      bool wide = size > 1;
      if (left->aop->type == AOP_LIT)
        emit2 (wide ? "clrw" : "clr", "0x%02x%02x", byteOfVal (left->aop->aopu.aop_lit, 1), byteOfVal (left->aop->aopu.aop_lit, 0));
      else
        emit2 (wide ? "clrw" : "clr", "%s+%d", left->aop->aopu.immd, left->aop->aopu.immd_off);
      cost (3, 1);
      goto release;
    } 
  else if (!bit_field && (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD) &&
    (size == 1 && (regDead (XL_IDX, ic) || aopInReg (right->aop, 0, XL_IDX)) || size == 2 && (y_dead || aopInReg (right->aop, 0, Y_IDX))))
    {
      bool wide = size > 1;
      genMove (wide ? ASMOP_Y : ASMOP_XL, right->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), y_dead, regDead (Z_IDX, ic));
      if (left->aop->type == AOP_LIT)
        emit2 (wide ? "ldw" : "ld", "0x%02x%02x, %s", byteOfVal (left->aop->aopu.aop_lit, 1), byteOfVal (left->aop->aopu.aop_lit, 0), wide ? "y" : "xl");
      else
        emit2 (wide ? "ldw" : "ld", "%s+%d, %s", left->aop->aopu.immd, left->aop->aopu.immd_off, wide ? "y" : "xl");
      cost (3, 1);
      goto release;
    }
  else if (!bit_field && left->aop->type == AOP_STL)
    {
      struct asmop stackop_impl;
      init_stackop (&stackop_impl, size, left->aop->aopu.stk_off);
      genMove(&stackop_impl, right->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      goto release;
    }
  else if (!bit_field && size == 2 && aopOnStackNotExt (left->aop, 0, 2) &&
    (!regalloc_dry_run || !f8_extend_stack) && // Avoid getting into a situation where stack allocation makes code generation impossible
    aopIsAcc16 (right->aop, 0))
    {
      unsigned int soffset = left->aop->aopu.bytes[0].byteu.stk + G.stack.pushed;
      emit2 ("ldw", "((%u, sp)), %s", soffset, aopGet2 (right->aop, 0));
      cost (2 + !aopInReg (right->aop, 0, Y_IDX), 1);
      goto release;
    }
  else if (!bit_field && size == 2 && aopOnStackNotExt (left->aop, 0, 2) &&
    (!regalloc_dry_run || !f8_extend_stack) && // Avoid getting into a situation where stack allocation makes code generation impossible
    (right->aop->type == AOP_DIR || right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD || aopOnStack (right->aop, 0, 2)) &&
    (regDead (Y_IDX, ic) || regDead (X_IDX, ic)))
    {
      unsigned int soffset = left->aop->aopu.bytes[0].byteu.stk + G.stack.pushed;
      bool use_y = regDead (Y_IDX, ic);
      genMove (use_y ? ASMOP_Y : ASMOP_X, right->aop, true, true, regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      emit2 ("ldw", "((%u, sp)), %s", soffset, use_y ? "y" : "x");
      cost (2 + !use_y, 1);
      goto release;
    }

  if (!bit_field && size == 2 && aopInReg (left->aop, 0, Z_IDX) && aopIsAcc16 (right->aop, 0))
    {
      emit2 ("ldw", "(0, z), %s", aopGet2 (right->aop, 0));
      cost (3 + !aopInReg (right->aop, 0, Y_IDX), 1);
      goto release;
    }
  else if (!bit_field && size == 1 && aopInReg (left->aop, 0, Z_IDX) && aopIsAcc8 (right->aop, 0))
    {
      emit2 ("ld", "(0, z), %s", aopGet (right->aop, 0));
      cost (3 + !aopInReg (right->aop, 0, XL_IDX), 1);
      goto release;
    }
  else if (!bit_field && size >= 4 && (regDead (Y_IDX, ic) || aopInReg (left->aop, 0, Y_IDX)) && regDead (Z_IDX, ic) &&
    (aopOnStack(right->aop, 0, size) || right->aop->type == AOP_DIR))
    {
      genMove (ASMOP_Y, left->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, true);
      pointToSym (ASMOP_Z, OP_SYMBOL_CONST (right), 0,  regDead (XL_IDX, ic), regDead (XH_IDX, ic), false, true);
      for(int i = 0; i < size;)
        if (i + 1 < size)
          {
            emit2 ("ldwi", "(%i, y), (z)", i);
            cost (2, 1);
            i += 2;
          }
        else
          {
            emit2 ("ldi", "(%i, y), (z)", i);
            cost (2, 1);
            i++;
          }
      goto release;
    }
  else if ((left->aop->type == AOP_IMMD || left->aop->type == AOP_LIT) && regDead (XL_IDX, ic) &&
    bit_field && blen <= 8 && right->aop->type == AOP_LIT && (byteOfVal (right->aop->aopu.aop_lit, 0) == 0x00 || byteOfVal (right->aop->aopu.aop_lit, 0) == (0xff >> (8 - blen))))
    {
      unsigned char bval = (byteOfVal (right->aop->aopu.aop_lit, 0) << bstr) & ((0xff >> (8 - blen)) << bstr);
      if (byteOfVal (right->aop->aopu.aop_lit, 0) == 0x00)
        {
          emit2 ("ld", "xl, #0x%02x", ~((0xff >> (8 - blen)) << bstr) & 0xff);
          if (left->aop->type == AOP_LIT)
            emit2 ("and", "0x%02x%02x, xl", byteOfVal (left->aop->aopu.aop_lit, 1), byteOfVal (left->aop->aopu.aop_lit, 0));
          else
            emit2 ("and", "%s+%d, xl", left->aop->aopu.immd, left->aop->aopu.immd_off);
        }
      else
        {
          emit2 ("ld", "xl, #0x%02x", bval);
          if (left->aop->type == AOP_LIT)
            emit2 ("or", "0x%02x%02x, xl", byteOfVal (left->aop->aopu.aop_lit, 1), byteOfVal (left->aop->aopu.aop_lit, 0));
          else
            emit2 ("or", "%s+%d, xl", left->aop->aopu.immd, left->aop->aopu.immd_off);
        }
      cost (5, 2);
      goto release;
    }

  if (aopInReg (left->aop, 0, Y_IDX))
    ;
  else
    {
      if (!y_dead && (right->aop->regs[YL_IDX] >= 0 || right->aop->regs[YH_IDX] >= 0))
        UNIMPLEMENTED;

      if (!y_dead)
        {
          push (ASMOP_Y, 0, 2);
          pushed_y = true;
        }

       genMove (ASMOP_Y, left->aop, false, false, true, false);
    }

  for (int i = 0; !bit_field ? i < size : blen > 0; i++, blen -= 8)
    {
      bool xl_dead = regDead (XL_IDX, ic) && (right->aop->regs[XL_IDX] <= i);
      bool xl_dead2 = regDead (XL_IDX, ic) && (right->aop->regs[XL_IDX] <= i + 1);
      bool xh_dead2 = regDead (XH_IDX, ic) && (right->aop->regs[XH_IDX] <= i + 1);
      bool x_dead2 = xl_dead2 && xh_dead2;

      if ((!bit_field || blen >= 16) && i + 1 < size &&
        (aopInReg (right->aop, i, X_IDX) || x_dead2 && (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD || right->aop->type == AOP_DIR || aopOnStack (right->aop, i, 2))))
        {
          genMove_o (ASMOP_X, 0, right->aop, i, 2, true, true, false, false, true);
          emit2 ("ldw", i ? "(%d, y), x" : "(y), x", i);
          cost (1 + (bool)i, 1);
          i++;
          blen -= 8;
          continue;
        }
      else if ((!bit_field || blen >= 16) && i + 1 < size && aopInReg (right->aop, i, Z_IDX))
        {
          emit2 ("ldw", i ? "(%d, y), z" : "(y), z", i);
          cost (2 + (bool)i, 2);
          i++;
          blen -= 8;
          continue;
        }
      else if ((!bit_field || blen >= 8) &&
        (aopInReg (right->aop, i, XL_IDX) || aopInReg (right->aop, i, XH_IDX) || aopInReg (right->aop, i, ZL_IDX) || aopInReg (right->aop, i, ZH_IDX)))
        {
          if (!i)
            {
              emit2 ("ld", "(y), %s", aopGet (right->aop, i));
              cost (1 + !aopInReg (right->aop, i, XL_IDX), 1);
            }
          else
            {
              emit2 ("ld", "(%d, y), %s", i, aopGet (right->aop, i));
              cost (2 + !aopInReg (right->aop, i, XL_IDX), 1);
            }
          continue;
        }
      else if ((!bit_field || blen >= 8) && aopIsLitVal (right->aop, i, 1, 0x00))
        {
          emit2 ("clr", "(%d, y)", i);
          cost (2, 1);
          continue;
        }

      if (bit_field && blen < 8 && right->aop->type == AOP_LIT) // We can save a lot of shifting and masking using the known literal value
        {
          unsigned char bval = (byteOfVal (right->aop->aopu.aop_lit, i) << bstr) & ((0xff >> (8 - blen)) << bstr);

          if (!xl_dead)
            {
              push (ASMOP_XL, 0, 1);
              pushed_xl = true;
            }

          if (!i)
            {
              emit2 ("ld", "xl, (y)", i);
              cost (1, 1);
            }
          else
            {
              emit2 ("ld", "xl, (%d, y)", i);
              cost (2, 1);
            }
                
          if (((~((0xff >> (8 - blen)) << bstr) & 0xff) | bval) != 0xff)
            {
              emit2 ("and", "xl, #0x%02x", ~((0xff >> (8 - blen)) << bstr) & 0xff);
              cost (2, 1);
            }

          if (bval)
            {
              emit2 ("or", "xl, #0x%02x", bval);
              cost (2, 1);
            }

          goto store;
        }

      if (!xl_dead)
        {
          push (ASMOP_XL, 0, 1);
          pushed_xl = true;
        }
            
      genMove_o (ASMOP_XL, 0, right->aop, i, 1, true, false, false, false, true);

      if (!i && bit_field && blen < 8)
        {
          if (bstr <= 1)
            for (int j = 0; j < bstr; j++)
              emit3 (A_SLL, ASMOP_XL, 0);
          else
            {
              emit2 ("rot", "xl, #%d", bstr);
              cost (2, 1);
            }
          emit2 ("msk", "(y), xl, #0x%02x", (0xff >> (8 - blen)) << bstr);
          cost (2, 1);
          goto pop;
        }
      else if (bit_field && blen < 8)
        {
          if (!regDead (XH_IDX, ic))
            UNIMPLEMENTED;
          if (bstr <= 1)
            for (int j = 0; j < bstr; j++)
              emit3 (A_SLL, ASMOP_XL, 0);
          else
            {
              emit2 ("rot", "xl, #%d", bstr);
              cost (2, 1);
            }
          emit2 ("and", "xl, #0x%02x", (0xff >> (8 - blen)) << bstr);
          cost (2, 1);
          if (!i)
            {
              emit2 ("ld", "xh, (y)", i);
              cost (2, 1);
            }
          else
            {
              emit2 ("ld", "xh, (%d, y)", i);
              cost (3, 1);
            }
          emit2 ("and", "xh, #0x%02x", ~((0xff >> (8 - blen)) << bstr) & 0xff);
          cost (3, 1);
          emit2 ("or", "xl, xh");
          cost (1, 1);
        }
store:
      if (!i)
        {
          emit2 ("ld", "(y), xl");
          cost (1, 1);
        }
      else
        {
          emit2 ("ld", "(%d, y), xl", i);
          cost (2, 1);
        }
pop:
      if (pushed_xl)
        {
          pop (ASMOP_XL, 0, 1);
          pushed_xl = false;
        }
    }

release:
  if (pushed_y)
    pop (ASMOP_Y, 0, 2);

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

  aopOp (right, ic, false);
  aopOp (result, ic, true);

  wassert (result->aop->type != AOP_DUMMY || right->aop->type != AOP_DUMMY);

  if (right->aop->type == AOP_DUMMY)
    {
      D (emit2 ("; Dummy write", ""));
      wassert (0);
    }
  else if (result->aop->type == AOP_DUMMY)
    {
      D (emit2 ("; Dummy read", ""));
      wassert (0);
    }
  else
    genMove (result->aop, right->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));

  freeAsmop (right);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genIfx - generate code for conditional jump                     */
/*-----------------------------------------------------------------*/
static void
genIfx (const iCode *ic)
{
  operand *const cond = IC_COND (ic);
  sym_link *type = operandType (cond);
  symbol *const tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  aopOp (cond, ic, false);
  bool z_current = false;

  D (emit2 ("; genIfx", ""));

  int size = cond->aop->size;

  if (cond->aop->type == AOP_IMMD || cond->aop->type == AOP_STL) // An AOP_IMMD or AOP_STL points to something valid, so it is not a null pointer.
    {
      if (IC_TRUE (ic))
        emitJP (IC_TRUE (ic), 1.0f);
      goto release;
    }

  if (cond->aop->regs[XL_IDX] > 0)
    UNIMPLEMENTED;

  if (size == 1 && !IS_FLOAT(type) && aopIsOp8_1 (cond->aop, 0))
    {
      emit3 (A_TST, cond->aop, 0);
      goto jump;
    }

  if (size == 2 && !IS_FLOAT(type) &&
    (aopInReg (cond->aop, 0, Y_IDX) || cond->aop->type == AOP_DIR || aopOnStack (cond->aop, 0, 2) || aopInReg (cond->aop, 0, Z_IDX) ||
      (aopInReg (cond->aop, 0, X_IDX) && !regDead (XL_IDX, ic))))
    {
      emit3 (A_TSTW, cond->aop, 0);
      goto jump;
    }

  if (size == 4 && !IS_FLOAT(type) && 
    (aopInReg (cond->aop, 0, Y_IDX) && aopIsOp16_2 (cond->aop, 2) || aopInReg (cond->aop, 2, Y_IDX) && aopIsOp16_2 (cond->aop, 0)))
    {
      if (!regDead (Y_IDX, ic))
        push (ASMOP_Y, 0, 2);
      emit3_o (A_ORW, ASMOP_Y, 0, cond->aop, aopInReg (cond->aop, 0, Y_IDX) ? 2 : 0);
      if (!regDead (Y_IDX, ic))
        pop (ASMOP_Y, 0, 2);
      goto jump;
    }
  else if (size >= 4 && !(size % 2) && !IS_FLOAT(type) && regDead (Y_IDX, ic) &&
    aopOnStack (cond->aop, 0, 2) && aopIsOp16_2 (cond->aop, 2) && cond->aop->regs[XL_IDX] < 2 && cond->aop->regs[XH_IDX] < 2 && aopOnStackNotExt (cond->aop, 4, size - 4))
    {
      genMove_o (ASMOP_Y, 0, cond->aop, 0, 2, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, false, true);
      for (int i = 2; i < size; i += 2)
        emit3_o (A_ORW, ASMOP_Y, 0, cond->aop, i);
      goto jump;
    }

  if (!regDead (XL_IDX, ic))
    push (ASMOP_XL, 0, 1);

  int skipbyte;

  if (IS_FLOAT (type))
    {
      genMove_o (ASMOP_XL, 0, cond->aop, size - 1, 1, true, regDead (XH_IDX, ic) && cond->aop->regs[XH_IDX] <= 0, regDead (Y_IDX, ic) && cond->aop->regs[YL_IDX] <= 0 && cond->aop->regs[YH_IDX] <= 0, false, true);
      emit3 (A_SLL, ASMOP_XL, 0);
      skipbyte = size - 1;
    }
  else
    {
      genMove (ASMOP_XL, cond->aop, true, regDead (XH_IDX, ic) && cond->aop->regs[XH_IDX] <= 0, regDead (Y_IDX, ic) && cond->aop->regs[YL_IDX] <= 0 && cond->aop->regs[YH_IDX] <= 0, false);
      skipbyte = 0;
    }

  for (int i = 0; i < size;)
    {
      if (i == skipbyte)
        ;
      else if (aopIsOp8_2 (cond->aop, i))
        {
          emit3_o (A_OR, ASMOP_XL, 0, cond->aop, i);
          z_current = true;
        }
      else if (aopInReg (cond->aop, i, ZH_IDX))
        {
          push (ASMOP_Z, 1, 1);
          emit2 ("or", "xl, (0, sp)");
          cost (2, 1);
          adjustStack (1, false, false);
          z_current = true;
        }
      else
        UNIMPLEMENTED;
      i++;
    }

  if (!z_current)
    emit3 (A_TST, ASMOP_XL, 0);

  if (!regDead (XL_IDX, ic))
    pop (ASMOP_XL, 0, 1);

jump:
  if (tlbl)
    emit2 (IC_FALSE (ic) ? "jrnz" : "jrz", "#!tlabel", labelKey2num (tlbl->key));
  emitJP (IC_TRUE (ic) ? IC_TRUE (ic) : IC_FALSE (ic), 0.0f);
  emitLabel (tlbl);
  // todo : cost.

release:
  freeAsmop (cond);
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void
genAddrOf (const iCode *ic)
{
  const symbol *sym;
  operand *result, *left, *right;

  D (emit2 ("; genAddrOf", ""));

  result = IC_RESULT (ic);
  left = IC_LEFT (ic);
  right = IC_RIGHT (ic);

  wassert (result);
  wassert (left);
  wassert (IS_TRUE_SYMOP (left));
  wassert (right && IS_OP_LITERAL (IC_RIGHT (ic)));

  sym = OP_SYMBOL_CONST (left);

  aopOp (result, ic, true);

  pointToSym (result->aop, sym, (long)(operandLitValue (right)),  regDead (XL_IDX, ic),  regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));

  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genJumpTab - generate code for jump table                       */
/*-----------------------------------------------------------------*/
static void
genJumpTab (const iCode *ic)
{
  symbol *jtab = regalloc_dry_run ? 0 : newiTempLabel (0);
  operand *cond;

  D (emit2 ("; genJumpTab", ""));

  cond = IC_JTCOND (ic);

  aopOp (cond, ic, false);

  if (!regDead (Y_IDX, ic))
    {
      wassertl (regalloc_dry_run, "Need free Y for jump table.");
      cost (180, 180);
    }

  genMove (ASMOP_Y, cond->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));

  emit3 (A_SLLW, ASMOP_Y, 0);

  if (!regalloc_dry_run)
    {
      //emit2 ("ldw", "y, (#!tlabel, y)", labelKey2num (jtab->key)); todo: make this work to save a byte and a cycle!
      emit2 ("addw", "y, #!tlabel", labelKey2num (jtab->key));
      emit2 ("ldw", "y, (y)");
      emit2 ("jp", "y");
    }
  cost (4, 3);

  emitLabel (jtab);
  for (jtab = setFirstItem (IC_JTLABELS (ic)); jtab; jtab = setNextItem (IC_JTLABELS (ic)))
    {
      if (!regalloc_dry_run)
        emit2 (".dw", "#!tlabel", labelKey2num (jtab->key));
      cost (2, 0);
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
  sym_link *resulttype, *righttype;

  D (emit2 ("; genCast", ""));

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);
  resulttype = operandType (result);
  righttype = operandType (right);

  unsigned topbytemask = (IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8)) ?
    (0xff >> (8 - SPEC_BITINTWIDTH (resulttype) % 8)) : 0xff;

  // Cast to _BitInt can require mask of top byte.
  if (IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8) && bitsForType (resulttype) < bitsForType (righttype))
    {
      aopOp (right, ic, false);
      aopOp (result, ic, true);

      if (!regDead (XL_IDX, ic) || result->aop->regs[XL_IDX] >= 0 && result->aop->regs[XL_IDX] != result->aop->size - 1)
        UNIMPLEMENTED;
      genMove (result->aop, right->aop, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      genMove_o (ASMOP_XL, 0, result->aop, result->aop->size - 1, 1, true, false, false, false, true);
      emit2 ("and", "xl, #0x%02x", topbytemask);
      cost (2, 1);
      if (!SPEC_USIGN (resulttype)) // Sign-extend
        {
          unsigned testmask = 1u << (SPEC_BITINTWIDTH (resulttype) % 8 - 1);
          symbol *tlbl = regalloc_dry_run ? 0 : newiTempLabel (0);
          if (testmask != topbytemask)
            {
              push (ASMOP_XL, 0, 1);
              emit2 ("and", "xl, #0x%02x", testmask);
              pop (ASMOP_XL, 0, 1);
            }
          if (!regalloc_dry_run)
            emit2 ("jrz", "#!tlabel", labelKey2num (tlbl->key));
          emit2 ("or", "xl, #0x%02x", ~topbytemask & 0xff);
          cost (6, 2.5);
          emitLabel (tlbl);
        }
      genMove_o (result->aop, result->aop->size - 1, ASMOP_XL, 0, 1, true, false, false, false, true);

      goto release;
    }

  if ((getSize (resulttype) <= getSize (righttype) || !IS_SPEC (righttype) || (SPEC_USIGN (righttype) || IS_BOOL (righttype))) &&
    (!IS_BOOL (resulttype) || IS_BOOL (righttype)))
    {
      genAssign (ic);
      return;
    }

  aopOp (right, ic, false);
  aopOp (result, ic, true);

  if (IS_BOOL (resulttype))
    {
      wassert (result->aop->size == 1);

      if (right->aop->size == 2 && regDead (Y_IDX, ic) && aopInReg (result->aop, 0, YL_IDX))
        {
          genMove (ASMOP_Y, right->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
          emit2 ("boolw", "y");
          cost (1, 1);
        }
      else if (right->aop->size == 2 && regDead (X_IDX, ic))
        {
          genMove (ASMOP_X, right->aop, true, true, regDead (Y_IDX, ic), regDead (Z_IDX, ic));
          emit2 ("boolw", "x");
          cost (2, 1);
          genMove (result->aop, ASMOP_XL, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
        }
      else if (right->aop->size > 2 && right->aop->size % 2 == 0 && regDead (Y_IDX, ic) && (aopOnStack (right->aop, 2, right->aop->size - 2) || right->aop->type == AOP_DIR))
        {
          genMove (ASMOP_Y, right->aop, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
          for (int i = 2; i < right->aop->size; i += 2)
            emit3_o (A_ORW, ASMOP_Y, 0, right->aop, i);
          emit2 ("boolw", "y");
          cost (1, 1);
          genMove (result->aop, ASMOP_Y, regDead (XL_IDX, ic), regDead (XH_IDX, ic), true, regDead (Z_IDX, ic));
        }
      else if (right->aop->size == 1)
        {
          if (!regDead (XL_IDX, ic))
            push (ASMOP_XL, 0, 1);
          genMove (ASMOP_XL, right->aop, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
          emit2 ("bool", "xl");
          cost (1, 1);
          genMove (result->aop, ASMOP_XL, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
          if (!regDead (XL_IDX, ic))
            pop (ASMOP_XL, 0, 1);
        }
      else
        UNIMPLEMENTED;
    }
  // Signed cast
  else if (result->aop->size == 2 && (topbytemask == 0xff || !SPEC_USIGN (resulttype)) &&
    (aopInReg (result->aop, 0, Y_IDX) /*|| aopInReg (result->aop, 0, X_IDX) || aopInReg (result->aop, 0, Z_IDX)*/)) // todo: reenable with fixed right operand!
    {
      if (!regDead (XL_IDX, ic) && !aopInReg (right->aop, 0, XL_IDX))
        push (ASMOP_XL, 0, 1);
      genMove (ASMOP_XL, right->aop, true, regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic));
      emit2 ("sex", "%s, xl", aopGet2 (result->aop, 0));
      cost (1 + !aopInReg (result->aop, 0, Y_IDX), 1 + !aopInReg (result->aop, 0, Y_IDX));
      if (!regDead (XL_IDX, ic) && !aopInReg (right->aop, 0, XL_IDX))
        pop (ASMOP_XL, 0, 1);
    }
  else 
    {
      bool masktopbyte = IS_BITINT (resulttype) && (SPEC_BITINTWIDTH (resulttype) % 8) && SPEC_USIGN (resulttype);

      genMove_o (result->aop, 0, right->aop, 0, right->aop->size, regDead (XL_IDX, ic), regDead (XH_IDX, ic), regDead (Y_IDX, ic), regDead (Z_IDX, ic), true);

      if (!regDead (Y_IDX, ic) ||
        result->aop->regs[YL_IDX] >= 0 && result->aop->regs[YL_IDX] != right->aop->size - 1 ||
        result->aop->regs[YH_IDX] >= 0 && result->aop->regs[YH_IDX] < right->aop->size)
        UNIMPLEMENTED;
      if (!regDead (XL_IDX, ic) ||
        result->aop->regs[XL_IDX] >= 0 && result->aop->regs[XL_IDX] < right->aop->size)
        UNIMPLEMENTED;

      if (aopInReg (result->aop, right->aop->size - 1, ZH_IDX))
        {
          emit2 ("sex", "y, zh");
          cost (2, 2);
        }
      else
        {
          genMove_o (ASMOP_XL, 0, result->aop, right->aop->size - 1, 1, true, false, true, false, true);
          emit2 ("sex", "y, xl");
          cost (1, 1);
        }

      if (!masktopbyte && right->aop->size + 1 == result->aop->size)
        genMove_o (result->aop, right->aop->size, ASMOP_Y, 1, 1, true, false, true, false, true);
      else if (!masktopbyte && right->aop->size + 2 == result->aop->size && result->aop->regs[YL_IDX] < 0 &&
        (aopInReg (result->aop, right->aop->size, Y_IDX) || aopOnStack (result->aop, right->aop->size, 2) || result->aop->type == AOP_DIR))
        {
          emit3_o (A_LD, ASMOP_Y, 0, ASMOP_Y, 1);
          genMove_o (result->aop, right->aop->size, ASMOP_Y, 0, 2, true, false, true, false, true);
        }
      else
        {
          emit3_o (A_LD, ASMOP_X, 0, ASMOP_Y, 1);

          for (int i = right->aop->size; i < result->aop->size; i++)
            {
              bool pushed_xl = false;
              if (masktopbyte && i + 1 == result->aop->size)
                {
                  if (result->aop->regs[XL_IDX] >= 0 && result->aop->regs[XL_IDX] < i)
                    {
                      push (ASMOP_XL, 0, 1);
                      pushed_xl = true;
                    }
                  emit2 ("and", "xl, #0x%02x", topbytemask);
                  cost (2, 1);
                }
              genMove_o (result->aop, i, ASMOP_XL, 0, 1, false, false, false, false, true);
              if (pushed_xl)
                pop (ASMOP_XL, 0, 1);
            }
        }
    }

release:
  freeAsmop (right);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genReceive - generate code for receiving a register parameter.  */
/*-----------------------------------------------------------------*/
static void
genReceive (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  aopOp (result, ic, true);

  D (emit2 ("; genReceive", ""));

  wassert (currFunc);
  wassert (ic->argreg);
  wassert (aopArg (currFunc->type, ic->argreg));

  bool dead_regs[ZH_IDX + 1];
  
  for (int i = 0; i <= ZH_IDX; i++)
    dead_regs[i] = regDead (i, ic);

  for(iCode *nic = ic->next; nic && nic->op == RECEIVE; nic = nic->next)
    {
      asmop *narg = aopArg (currFunc->type, nic->argreg);
      wassert (narg);
      for (int i = 0; i < narg->size; i++)
        dead_regs[narg->aopu.bytes[i].byteu.reg->rIdx] = false;
    }
    
  if (result->aop->type == AOP_REGSTK)
    for (int i = 0; i < result->aop->size; i++)
      if (aopRS (result->aop) && result->aop->aopu.bytes[i].in_reg && !dead_regs[result->aop->aopu.bytes[i].byteu.reg->rIdx])
        UNIMPLEMENTED;

  genMove (result->aop, aopArg (currFunc->type, ic->argreg), dead_regs[XL_IDX], dead_regs[XH_IDX], dead_regs[YL_IDX] && dead_regs[YH_IDX], dead_regs[ZL_IDX] && dead_regs[ZH_IDX]);

  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genSend - generate code for sending a register parameter.       */
/*-----------------------------------------------------------------*/
static void
genSend (const iCode *ic)
{
  D (emit2 ("; genSend", ""));

  aopOp (ic->left, ic, false);

  /* Caller saves, and this is the first iPush. */
  /* Scan ahead until we find the function that we are pushing parameters to.
     Count the number of addSets on the way to figure out what registers
     are used in the send set.
   */
  const iCode *walk;
  for (walk = ic->next; walk; walk = walk->next)
    {
      if (walk->op == CALL || walk->op == PCALL)
        break;
    }

  if (!G.saved && !regalloc_dry_run) // Cost is counted at CALL or PCALL instead
    saveRegsForCall (walk);

  sym_link *ftype = IS_FUNCPTR (operandType (IC_LEFT (walk))) ? operandType (IC_LEFT (walk))->next : operandType (IC_LEFT (walk));
  asmop *argreg = aopArg (ftype, ic->argreg);

  wassert (argreg);

  // The register argument shall not overwrite a still-needed (i.e. as further parameter or function for the call) value.
  for (int i = 0; i < argreg->size; i++)
    if (!regDead (argreg->aopu.bytes[i].byteu.reg->rIdx, ic))
      for (iCode *walk2 = ic->next; walk2; walk2 = walk2->next)
        {
          if (walk2->op != CALL && IC_LEFT (walk2) && !IS_OP_LITERAL (IC_LEFT (walk2)))
            UNIMPLEMENTED;

          if (walk2->op == CALL || walk2->op == PCALL)
            break;
        }

  bool xl_dead = regDead (XL_IDX, ic);
  bool xh_dead = regDead (XH_IDX, ic);
  bool y_dead = regDead (Y_IDX, ic);
  bool z_dead = regDead (Z_IDX, ic);
  
  for (iCode *walk2 = ic->prev; walk2 && walk2->op == SEND; walk2 = walk2->prev)
    {
      asmop *warg = aopArg (ftype, walk2->argreg);
      wassert (warg);
      xl_dead &= (warg->regs[XL_IDX] < 0);
      xh_dead &= (warg->regs[XH_IDX] < 0);
      y_dead &= (warg->regs[YL_IDX] < 0 && warg->regs[YH_IDX] < 0);
      z_dead &= (warg->regs[ZL_IDX] < 0 && warg->regs[ZH_IDX] < 0);
    }
    
  genMove (argreg, IC_LEFT (ic)->aop, xl_dead, xh_dead, y_dead, z_dead);

  for (int i = 0; i < argreg->size; i++)
    if (!regalloc_dry_run)
       ((walk->op == PCALL) ? f8_regs_used_as_parms_in_pcalls_from_current_function : f8_regs_used_as_parms_in_calls_from_current_function)[argreg->aopu.bytes[i].byteu.reg->rIdx] = true;

  freeAsmop (IC_LEFT (ic));
}

/*-----------------------------------------------------------------*/
/* genDummyRead - generate code for dummy read of volatiles.       */
/*-----------------------------------------------------------------*/
static void
genDummyRead (const iCode *ic)
{
  operand *op;
  int i;

  if ((op = IC_RIGHT (ic)) && IS_SYMOP (op))
    {
      aopOp (op, ic, false);

      D (emit2 ("; genDummyRead", ""));

      for (i = 0; i < op->aop->size; i++)
        {
          if (i + 1 < op->aop->size && aopIsOp16_1 (op->aop, i))
            {
              emit3_o (A_TSTW, op->aop, i, 0, 0);
              i++;
            }
          else if (aopIsOp8_1 (op->aop, i))
            emit3_o (A_TST, op->aop, i, 0, 0);
          else
            UNIMPLEMENTED;
        }

      freeAsmop (op);
    }

  if ((op = IC_LEFT (ic)) && IS_SYMOP (op))
    {
      aopOp (op, ic, false);

      D (emit2 ("; genDummyRead", ""));

      for (i = 0; i < op->aop->size; i++)
        {
          if (i + 1 < op->aop->size && aopIsOp16_1 (op->aop, i))
            {
              emit3_o (A_TSTW, op->aop, i, 0, 0);
              i++;
            }
          else if (aopIsOp8_1 (op->aop, i))
            emit3_o (A_TST, op->aop, i, 0, 0);
          else
            UNIMPLEMENTED;
        }

      freeAsmop (op);
    }
}

/*---------------------------------------------------------------------*/
/* genF8Code - generate code for F8 for a single iCode instruction     */
/*---------------------------------------------------------------------*/
static void
genF8iCode (iCode *ic)
{
  genLine.lineElement.ic = ic;

#if 0
  if (!regalloc_dry_run)
    printf ("ic %d op %d stack pushed %d\n", ic->key, ic->op, G.stack.pushed);
#endif

  if (resultRemat (ic))
    {
      if (!regalloc_dry_run)
        D (emit2 ("; skipping iCode since result will be rematerialized", ""));
      return;
    }

  if (ic->generated)
    {
      if (!regalloc_dry_run)
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

   case IPUSH_VALUE_AT_ADDRESS:
      genPointerPush (ic);
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
      genMinus (ic);
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

    case GETABIT:
      genGetABit (ic, ifxForOp (IC_RESULT (ic), ic));
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
      genPointerGet (ic, ifxForOp (IC_RESULT (ic), ic));
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
      genReceive (ic);
      break;
      
    case SEND:
      genSend (ic);
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
    }
}

float
dryF8iCode (iCode *ic)
{
  regalloc_dry_run = true;
  regalloc_dry_run_cost_bytes = 0;
  regalloc_dry_run_cost_cycles = 0;

  spillAllRegs ();

  initGenLineElement ();

  genF8iCode (ic);

  destroy_line_list ();

  wassert (regalloc_dry_run);

  const unsigned int byte_cost_weight = 2 << (optimize.codeSize * 3 + !optimize.codeSpeed * 3);

  return ((float)regalloc_dry_run_cost_bytes * byte_cost_weight + (float)regalloc_dry_run_cost_cycles * ic->count);
}

/*---------------------------------------------------------------------*/
/* genF8Code - generate code for F8 for a block of intructions         */
/*---------------------------------------------------------------------*/
void
genF8Code (iCode *lic)
{
  iCode *ic;
  int clevel = 0;
  int cblock = 0;  
  int cln = 0;
  regalloc_dry_run = false;

  /* if debug information required */
  if (options.debug && currFunc && !regalloc_dry_run)
    debugFile->writeFunction (currFunc, lic);

  if (options.debug && !regalloc_dry_run)
    debugFile->writeFrameAddress (NULL, NULL, 0); /* have no idea where frame is now */

  memset(f8_regs_used_as_parms_in_calls_from_current_function, 0, sizeof(bool) * (ZH_IDX + 1));
  memset(f8_regs_used_as_parms_in_pcalls_from_current_function, 0, sizeof(bool) * (ZH_IDX + 1));

  spillAllRegs ();

  for (ic = lic; ic; ic = ic->next)
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

      regalloc_dry_run_cost_bytes = 0;
      regalloc_dry_run_cost_cycles = 0;

      if (options.iCodeInAsm)
        {
          const char *iLine = printILine (ic);
          emit2 ("; ic:", "%d: %s", ic->key, iLine);
          dbuf_free (iLine);
        }
#if 0
      D (emit2 (";", "count: %f, G.c %d", ic->count, G.c));
#endif
      genF8iCode(ic);

#if 0
      fprintf (stderr, "ic %d: (op %d) stack %d\n", ic->key, ic->op, G.stack.pushed);
#endif

#if 1
      D (emit2 (";", "Cost for generated ic %d : (%d, %f)", ic->key, regalloc_dry_run_cost_bytes, regalloc_dry_run_cost_cycles));
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

  /* destroy the line list */
  destroy_line_list ();
}

bool
f8IsReturned (const char *what)
{
  if (!strcmp(what, "x"))
    return (f8IsReturned ("xl") || f8IsReturned ("xh"));
  else if (!strcmp(what, "y"))
    return (f8IsReturned ("yl") || f8IsReturned ("yh"));
  else if (!strcmp(what, "z"))
    return (f8IsReturned ("zl") || f8IsReturned ("zh"));

  const asmop *retaop = aopRet (currFunc->type);

  if (!retaop)
    return false;
  for (int i = 0; i < retaop->size; i++)
    if (!strcmp(retaop->aopu.bytes[i].byteu.reg->name, what))
      return true;
  return false;
}

// Check if what is part of the ith argument (counting from 1) to a function of type ftype.
// If what is 0, just check if hte ith argument is in registers.
bool
f8IsRegArg (struct sym_link *ftype, int i, const char *what)
{
  if (what && !strcmp(what, "x"))
    return (f8IsRegArg (ftype, i, "xl") || f8IsRegArg (ftype, i, "xh"));
  else if (what && !strcmp(what, "y"))
    return (f8IsRegArg (ftype, i, "yl") || f8IsRegArg (ftype, i, "yh"));
  else if (what && !strcmp(what, "z"))
    return (f8IsRegArg (ftype, i, "zl") || f8IsRegArg (ftype, i, "zh"));
 
  const asmop *argaop = aopArg (ftype, i);

  if (!argaop)
    return false;
    
  if (!what)
    return true;

  for (int i = 0; i < argaop->size; i++)
    if (!strcmp(argaop->aopu.bytes[i].byteu.reg->name, what))
      return true;

  return false; 
}

bool
f8IsParmInCall (sym_link *ftype, const char *what)
{
  const value *args;
  int i;

  for (i = 1, args = FUNC_ARGS (ftype); args; args = args->next, i++)
    if (f8IsRegArg(ftype, i, what))
      return true;
  return false;
}

