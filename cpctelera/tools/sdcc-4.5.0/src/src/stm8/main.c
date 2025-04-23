/*-------------------------------------------------------------------------
  main.c - STM8 specific definitions.

  Philipp Klaus Krause <pkk@spth.de> 2012-2013
  Valentin Dudouyt <valentin.dudouyt@gmail.com> 2013

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
#include "peep.h"

#define OPTION_MEDIUM_MODEL         "--model-medium"
#define OPTION_LARGE_MODEL          "--model-large"
#define OPTION_CODE_SEG        "--codeseg"
#define OPTION_CONST_SEG       "--constseg"
#define OPTION_ELF             "--out-fmt-elf"
#define OPTION_SDCCCALL        "--sdcccall"

extern DEBUGFILE dwarf2DebugFile;
extern int dwarf2FinalizeFile(FILE *);

static OPTION stm8_options[] = {
  {0, OPTION_MEDIUM_MODEL, NULL, "16-bit address space for both data and code (default)"},
  {0, OPTION_LARGE_MODEL, NULL, "16-bit address space for data, 24-bit for code"},
  {0, OPTION_CODE_SEG,        &options.code_seg, "<name> use this name for the code segment", CLAT_STRING},
  {0, OPTION_CONST_SEG,       &options.const_seg, "<name> use this name for the const segment", CLAT_STRING},
  {0, OPTION_ELF,             NULL, "Output executable in ELF format"},
  {0, OPTION_SDCCCALL,         &options.sdcccall, "Set ABI version for default calling convention", CLAT_INTEGER},
  {0, NULL}
};

enum
{
  P_CODESEG = 1,
  P_CONSTSEG,
};

static struct
{
  // Determine if we can put parameters in registers
  struct
  {
    int n;
    struct sym_link *ftype;
  } regparam;
}
_G;

static int
stm8_do_pragma (int id, const char *name, const char *cp)
{
  struct pragma_token_s token;
  int processed = 1, error = 0;

  init_pragma_token (&token);

  switch (id)
    {
      case P_CODESEG:
      case P_CONSTSEG:
        {
          char *segname;

          cp = get_pragma_token (cp, &token);
          if (token.type == TOKEN_EOL)
            {
              error = 1;
              break;
            }
          else
            segname = Safe_strdup (get_pragma_string (&token));

          cp = get_pragma_token (cp, &token);
          if (token.type != TOKEN_EOL)
            {
              Safe_free (segname);
              error = 1;
              break;
            }
          else
            {
              if (id == P_CODESEG)
                {
                  if (options.code_seg)
                    Safe_free (options.code_seg);
                  options.code_seg = segname;
                } 
              else
                {
                  if (options.const_seg)
                    Safe_free (options.const_seg);
                  options.const_seg = segname;
                }
            }
        }
        break;
      default:
        processed = 0;
        break;
    }

  if (error)
    werror (W_BAD_PRAGMA_ARGUMENTS, name);

  free_pragma_token (&token);
  return processed;
}

static struct pragma_s stm8_pragma_tbl[] = {
  {"codeseg", P_CODESEG, 0, stm8_do_pragma},
  {"constseg", P_CONSTSEG, 0, stm8_do_pragma},
  {NULL, 0, 0, NULL},
};

static int
stm8_process_pragma (const char *s)
{
  return process_pragma_tbl (stm8_pragma_tbl, s);
}

static char stm8_defaultRules[] = {
#include "peeph.rul"
  ""
};


static char *stm8_keywords[] = {
  "at",
  "critical",
  "interrupt",
  "trap",
  "naked",
  "raisonance",
  "iar",
  "cosmic",
  "z88dk_callee",
  NULL
};

static void
stm8_genAssemblerEnd (FILE *of)
{
  if (options.out_fmt == 'E' && options.debug)
    {
      dwarf2FinalizeFile (of);
    }
}

extern void stm8_init_asmops (void);

static void
stm8_init (void)
{
  asm_addTree (&asm_asxxxx_mapping);

  stm8_init_asmops ();
}

static void
stm8_reset_regparm (struct sym_link *ftype)
{
  _G.regparam.n = 0;
  _G.regparam.ftype = ftype;
}

static int
stm8_reg_parm (sym_link *l, bool reentrant)
{
  bool is_regarg = stm8IsRegArg (_G.regparam.ftype, ++_G.regparam.n, 0);

  return (is_regarg ? _G.regparam.n : 0);
}

static bool
stm8_parseOptions (int *pargc, char **argv, int *i)
{
  if (!strncmp (argv[*i], OPTION_ELF, sizeof (OPTION_ELF) - 1))
  {
    options.out_fmt = 'E';
    debugFile = &dwarf2DebugFile;
    return true;
  }
  return false;
}

static void
stm8_finaliseOptions (void)
{
  port->mem.default_local_map = data;
  port->mem.default_globl_map = data;

  if (options.model == MODEL_LARGE)
    {
      port->s.funcptr_size = 3;
      port->stack.call_overhead = 3;
      port->jumptableCost.maxCount = 0;
    }
}

static void
stm8_setDefaultOptions (void)
{
  options.nopeep = 0;
  options.stackAuto = 1;
  options.intlong_rent = 1;
  options.float_rent = 1;
  options.noRegParams = 0;
  options.data_loc = 0x0001; /* We can't use the byte at address zero in C, since NULL pointers have special meaning */
  options.code_loc = 0x8000;

  options.stack_loc = -1; /* Do not set the stack pointer in software- just use the device-specific reset value. */

  options.out_fmt = 'i';        /* Default output format is ihx */
}

static const char *
stm8_getRegName (const struct reg_info *reg)
{
  if (reg)
    return reg->name;
  return "err";
}

static void
stm8_genExtraArea (FILE *of, bool hasMain)
{
  fprintf (of, "\n; default segment ordering for linker\n");
  tfprintf (of, "\t!area\n", HOME_NAME);
  tfprintf (of, "\t!area\n", STATIC_NAME);
  tfprintf (of, "\t!area\n", port->mem.post_static_name);
  tfprintf (of, "\t!area\n", CONST_NAME);
  tfprintf (of, "\t!area\n", "INITIALIZER");
  tfprintf (of, "\t!area\n", CODE_NAME);
  fprintf (of, "\n");
}

static void
stm8_genInitStartup (FILE *of)
{
  if (options.stack_loc >= 0)
    {
      fprintf (of, "\tldw\tx, #0x%04x\n", options.stack_loc);
      fprintf (of, "\tldw\tsp, x\n");
    }

  /* Call external startup code */
  fprintf (of, options.model == MODEL_LARGE ? "\tcallf\t___sdcc_external_startup\n" : "\tcall\t___sdcc_external_startup\n");

  /* If external startup returned non-zero, skip init */
  fprintf (of, "\ttnz\ta\n");
  fprintf (of, "\tjreq\t__sdcc_init_data\n");
  fprintf (of, options.model == MODEL_LARGE ? "\tjpf\t__sdcc_program_startup\n" : "\tjp\t__sdcc_program_startup\n");

  /* Init static & global variables */
  fprintf (of, "__sdcc_init_data:\n");
  fprintf (of, "; stm8_genXINIT() start\n");

  /* Zeroing memory (required by standard for static & global variables) */
  fprintf (of, "\tldw x, #l_DATA\n");
  fprintf (of, "\tjreq\t00002$\n");
  fprintf (of, "00001$:\n");
  fprintf (of, "\tclr (s_DATA - 1, x)\n");
  fprintf (of, "\tdecw x\n");
  fprintf (of, "\tjrne\t00001$\n");
  fprintf (of, "00002$:\n");

  /* Copy l_INITIALIZER bytes from s_INITIALIZER to s_INITIALIZED */
  fprintf (of, "\tldw\tx, #l_INITIALIZER\n");
  fprintf (of, "\tjreq\t00004$\n");
  fprintf (of, "00003$:\n");
  fprintf (of, "\tld\ta, (s_INITIALIZER - 1, x)\n");
  fprintf (of, "\tld\t(s_INITIALIZED - 1, x), a\n");
  fprintf (of, "\tdecw\tx\n");
  fprintf (of, "\tjrne\t00003$\n");
  fprintf (of, "00004$:\n");
  fprintf (of, "; stm8_genXINIT() end\n");
}

#define STM8_INTERRUPTS_COUNT 30

int
stm8_genIVT(struct dbuf_s * oBuf, symbol ** intTable, int intCount)
{
  int i;
  dbuf_tprintf (oBuf, "\tint s_GSINIT ; reset\n");

  if(intCount > STM8_INTERRUPTS_COUNT)
    {
      werror(E_INT_BAD_INTNO, intCount - 1);
      intCount = STM8_INTERRUPTS_COUNT;
    }

  if (interrupts[INTNO_TRAP] || intCount)
    dbuf_printf (oBuf, "\tint %s ; trap\n", interrupts[INTNO_TRAP] ? interrupts[INTNO_TRAP]->rname : "0x000000");
    
  for (i = 0; i < intCount; i++)
    dbuf_printf (oBuf, "\tint %s ; int%d\n", interrupts[i] ? interrupts[i]->rname : "0x000000", i);

  return TRUE;
}

/*----------------------------------------------------------------------*/
/* stm8_dwarfRegNum - return the DWARF register number for a register.  */
/*----------------------------------------------------------------------*/
static int
stm8_dwarfRegNum (const struct reg_info *reg)
{
  return reg->rIdx;
}

static bool
_hasNativeMulFor (iCode *ic, sym_link *left, sym_link *right)
{
  int result_size = IS_SYMOP (IC_RESULT (ic)) ? getSize (OP_SYM_TYPE (IC_RESULT(ic))) : 4;
  sym_link *test = NULL;

  if (IS_BITINT (OP_SYM_TYPE (IC_RESULT(ic))) && SPEC_BITINTWIDTH (OP_SYM_TYPE (IC_RESULT(ic))) % 8)
    return false;

  if (IS_LITERAL (left))
    test = left;
  else if (IS_LITERAL (right))
    test = right;

  switch (ic->op)
    {
    case '/':
      if (getSize (left) <= 2 && getSize (right) <= 2 && IS_LITERAL (right) && isPowerOf2 (ulFromVal (valFromType (right))) && ulFromVal (valFromType (right)) <= 32) // Using arithmetic right-shift is worth it for small divisors only.
        return true;
    case '%':
      return (getSize (left) <= 2 && IS_UNSIGNED (left) && getSize (right) <= 2 && IS_UNSIGNED (right));
    case '*':
      {
        if (result_size == 1 || getSize (left) <= 1 && getSize (right) <= 1 && result_size == 2 && IS_UNSIGNED (left) && IS_UNSIGNED (right))
          return TRUE;

        if ((getSize (left) != 2 || getSize (right) != 2) || result_size != 2 || !test)
          return FALSE;

        unsigned long long add, sub;
        int topbit, nonzero;

        if (floatFromVal (valFromType (test)) < 0 || csdOfVal (&topbit, &nonzero, &add, &sub, valFromType (test), 0xffff))
          return false;

        int shifts = topbit;

        // If the leading digits of the cse are 1 0 -1 we can use 0 1 1 instead to reduce the number of shifts.
        if (topbit >= 2 && (add & (1ull << topbit)) && (sub & (1ull << (topbit - 2))))
          shifts--;

        wassert (nonzero);

        // Shifts are 1 byte, additions and subtractions are 3 bytes.
        if (shifts + 3 * (nonzero - 1) <= 9 - optimize.codeSize + 3 * optimize.codeSpeed)
          return TRUE;

        return FALSE;
      }
    default:
      return FALSE;
    }
}

/* Indicate which extended bit operations this port supports */
static bool
hasExtBitOp (int op, sym_link *left, int right)
{
  int size = getSize (left);

  switch (op)
    {
    case GETABIT:
    case GETBYTE:
    case GETWORD:
      return (true);
    case ROT:
      {
        unsigned int lbits = bitsForType (left);
        if (lbits % 8)
          return (false);
        if (size <= 1)
          return (true);
        if (size <= 2 && (right % lbits  == 1 || right % lbits == lbits - 1))
          return (true);
        if ((size <= 2 || size == 4) && lbits == right * 2)
          return (true);
      }
      return (false);
    }

  return (false);
}

static const char *
get_model (void)
{
  switch (options.model)
    {
    case MODEL_MEDIUM:
      return ("stm8");
      break;
    case MODEL_LARGE:
      return ("stm8-large");
      break;
    default:
      werror (W_UNKNOWN_MODEL, __FILE__, __LINE__);
      return "unknown";
    }
}

/** $1 is always the basename.
    $2 is always the output file.
    $3 varies
    $l is the list of extra options that should be there somewhere...
    $L is the list of extra options that should be passed on the command line...
    MUST be terminated with a NULL.
*/
static const char *_linkCmd[] =
{
  "sdldstm8", "-nf", "\"$1\"", "$L", NULL
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *stm8AsmCmd[] =
{
  "sdasstm8", "$l", "$3", "\"$1.asm\"", NULL
};

static const char *const _libs_stm8[] = { "stm8", NULL, };

PORT stm8_port =
{
  TARGET_ID_STM8,
  "stm8",
  "STM8",                       /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    TRUE,                       /* We want stm8_genIVT to be triggered */
    MODEL_MEDIUM | MODEL_LARGE,
    MODEL_MEDIUM,
    &get_model,                 /* model string used as library destination */
  },
  {                             /* Assembler */
    stm8AsmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _linkCmd,
    NULL,                       //LINKCMD,
    NULL,
    ".rel",
    1,
    NULL,                       /* crt */
    _libs_stm8,                 /* libs */
  },
  {                             /* Peephole optimizer */
    stm8_defaultRules,
    stm8instructionSize,
    NULL,
    NULL,
    NULL,
    stm8notUsed,
    stm8canAssign,
    stm8notUsedFrom,
    NULL,
    NULL,
    NULL,
  },
  /* Sizes */
  {
    1,                          /* char */
    2,                          /* short */
    2,                          /* int */
    4,                          /* long */
    8,                          /* long long */
    2,                          /* near ptr */
    2,                          /* far ptr */
    2,                          /* generic ptr */
    2,                          /* func ptr */
    3,                          /* banked func ptr */
    1,                          /* bit */
    4,                          /* float */
    64,                         /* bit-precise integer types up to _BitInt (64) */
  },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",                     /* code */
    "DATA",                     /* data */
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG (ABS)",               /* reg */
    "GSINIT",                   /* static initialization */
    NULL,                       /* overlay */
    "GSFINAL",                  /* gsfinal */
    "HOME",                     /* home */
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    "CONST",                    /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    "INITIALIZED",              /* name of segment for initialized variables */
    "INITIALIZER",              /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1,                          /* CODE  is read-only */
    false,                      // doesn't matter, as port has no __sfr anyway
    1                           /* No fancy alignments supported. */
  },
  { stm8_genExtraArea, NULL },
  1,                            /* default ABI revision */
  {                             /* stack information */
    -1,                         /* stack grows down */
     0,
     7,                         /* isr overhead */
     2,                         /* call overhead */
     0,
     2,
     1,                         /* sp points to next free stack location */
  },     
  { 
    -1,                         /* shifts never use support routines */
    true,                       /* use support routine for int x int -> long multiplication */
    true,                       /* use support routine for unsigned long x unsigned char -> unsigned long long multiplication */
  },
  { stm8_emitDebuggerSymbol,
    {
      stm8_dwarfRegNum,
      0,                        /* cfiSame */
      0,                        /* cfiUndef */
      4,                        /* addressSize */
      9,                        /* regNumRet */
      SP_IDX,                   /* regNumSP */
      0,                        /* regNumBP */
      2,                        /* offsetSP */
    },
  },
  {
    32767,                      /* maxCount */
    2,                          /* sizeofElement */
    {4, 5, 5},                  /* sizeofMatchJump[] - assuming operand in reg, inverse can be optimized away - would be much higher otherwise */
    {4, 5, 5},                  /* sizeofRangeCompare[] - same as above */
    3,                          /* sizeofSubtract - assuming 2 byte index, would be 2 otherwise */
    5,                          /* sizeofDispatch - 1 byte for sllw followed by 3 bytes for ldw x, (..., x) and 1 byte for jp (x) */
  },
  "_",
  stm8_init,
  stm8_parseOptions,
  stm8_options,
  NULL,
  stm8_finaliseOptions,
  stm8_setDefaultOptions,
  stm8_assignRegisters,
  stm8_getRegName,
  0,
  NULL,
  stm8_keywords,
  NULL,
  stm8_genAssemblerEnd,
  stm8_genIVT,
  0,                            /* no genXINIT code */
  stm8_genInitStartup,          /* genInitStartup */
  stm8_reset_regparm,
  stm8_reg_parm,
  stm8_process_pragma,          /* process_pragma */
  NULL,                         /* getMangledFunctionName */
  _hasNativeMulFor,             /* hasNativeMulFor */
  hasExtBitOp,                  /* hasExtBitOp */
  NULL,                         /* oclsExpense */
  TRUE,
  FALSE,                        /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  NULL,                         /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  5,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};
