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

#define OPTION_CODE_SEG        "--codeseg"
#define OPTION_CONST_SEG       "--constseg"
#define OPTION_ELF             "--out-fmt-elf"

extern DEBUGFILE dwarf2DebugFile;
extern int dwarf2FinalizeFile(FILE *);

static OPTION stm8_options[] = {
  {0, OPTION_CODE_SEG,        &options.code_seg, "<name> use this name for the code segment", CLAT_STRING},
  {0, OPTION_CONST_SEG,       &options.const_seg, "<name> use this name for the const segment", CLAT_STRING},
  {0, OPTION_ELF,             NULL, "Output executable in ELF format"},
  {0, NULL}
};

enum
{
  P_CODESEG = 1,
  P_CONSTSEG,
};

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
  "smallc",
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

static void
stm8_init (void)
{
  // fprintf(stderr, "stm8_init\n");
  asm_addTree (&asm_asxxxx_mapping);
}


static void
stm8_reset_regparm (struct sym_link *funcType)
{
}

static int
stm8_reg_parm (sym_link * l, bool reentrant)
{
  return FALSE;
}

static bool
stm8_parseOptions (int *pargc, char **argv, int *i)
{
  if (!strcmp (argv[*i], "--out-fmt-elf"))
  {
    options.out_fmt = 'E';
    debugFile = &dwarf2DebugFile;
    return TRUE;
  }
  return FALSE;
}

static void
stm8_finaliseOptions (void)
{
  port->mem.default_local_map = data;
  port->mem.default_globl_map = data;
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

  options.out_fmt = 'i';        /* Default output format is ihx */
}

static const char *
stm8_getRegName (const struct reg_info *reg)
{
  if (reg)
    return reg->name;
  return "err";
}

void
stm8_genInitStartup(FILE * of)
{
  fprintf (of, "__sdcc_gs_init_startup:\n");

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
    dbuf_printf (oBuf, "\tint %s ; trap\n", interrupts[INTNO_TRAP] ? interrupts[INTNO_TRAP]->rname : "0x0000");
    
  for (i = 0; i < intCount; i++)
    dbuf_printf (oBuf, "\tint %s ; int%d\n", interrupts[i] ? interrupts[i]->rname : "0x0000", i);

  return TRUE;
}

/*----------------------------------------------------------------------*/
/* stm8_dwarfRegNum - return the DWARF register number for a register.  */
/*----------------------------------------------------------------------*/
static int
stm8_dwarfRegNum (const struct reg_info *reg)
{
  /* todo: return valid values */
  return -1;
}

static bool
_hasNativeMulFor (iCode *ic, sym_link *left, sym_link *right)
{
  int result_size = IS_SYMOP (IC_RESULT (ic)) ? getSize (OP_SYM_TYPE (IC_RESULT(ic))) : 4;
  sym_link *test = NULL;

  if (IS_LITERAL (left))
    test = left;
  else if (IS_LITERAL (right))
    test = right;

  switch (ic->op)
    {
    case '/':
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
        

        if (floatFromVal (valFromType (test)) < 0 || csdOfVal (&topbit, &nonzero, &add, &sub, valFromType (test)))
          return FALSE;

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
hasExtBitOp (int op, int size)
{
  return (op == GETABIT);
}

/** $1 is always the basename.
    $2 is always the output file.
    $3 varies
    $l is the list of extra options that should be there somewhere...
    MUST be terminated with a NULL.
*/
static const char *_linkCmd[] =
{
  "sdldstm8", "-nf", "\"$1\"", NULL
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
    NO_MODEL,
    NO_MODEL,
    NULL,                       /* model == target */
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
  },
  /* Sizes: char, short, int, long, long long, ptr, fptr, gptr, bit, float, max */
  { 1, 2, 2, 4, 8, 2, 2, 2, 1, 4, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },   /* far, near, xstack, code */
  {
    "XSEG",
    "STACK",
    "CODE",
    "DATA",
    NULL,                       /* idata */
    NULL,                       /* pdata */
    NULL,                       /* xdata */
    NULL,                       /* bit */
    "RSEG (ABS)",
    "GSINIT",                   /* static initialization */
    NULL,                       /* overlay */
    "GSFINAL",
    "HOME",
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    NULL,                       /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    NULL,                       /* iabs_name */
    "INITIALIZED",              /* name of segment for initialized variables */
    "INITIALIZER",              /* name of segment for copies of initialized variables in code space */
    NULL,
    NULL,
    1                           /* CODE  is read-only */
  },
  { NULL, NULL },
  { -1, 0, 7, 2, 0, 2, 1 },     /* stack information */
  { -1, TRUE },
  { stm8_emitDebuggerSymbol,
	{
      stm8_dwarfRegNum,
      NULL,
      NULL,
      4,                        /* addressSize */
      0,                        /* regNumRet */
      0,                        /* regNumSP */
      0,                        /* regNumBP */
      0,                        /* offsetSP */
    },
  },
  {
    32767,                      /* maxCount */
    2,                          /* sizeofElement */
    {4, 5, 5},                  /* sizeofMatchJump[] - assuming operand in reg, inverse can be optimized away - would be much higher otherwise */
    {4, 5, 5},                  /* sizeofRangeCompare[] - same as above */
    3,                          /* sizeofSubtract - assuming 2 byte index, would be 2 otherwise */
    5,                          /* sizeofDispatch - 1 byte for sllw followed by 3 bytes for ldw x, (..., X) and 2 byte for jp (x) */
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
