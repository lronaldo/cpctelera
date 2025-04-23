/*-------------------------------------------------------------------------
  main.c - F8 specific definitions.

  Philipp Klaus Krause <pkk@spth.de> 2021

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

extern DEBUGFILE dwarf2DebugFile;
extern int dwarf2FinalizeFile(FILE *);

static OPTION f8_options[] = {
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
f8_do_pragma (int id, const char *name, const char *cp)
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

static struct pragma_s f8_pragma_tbl[] = {
  {"codeseg", P_CODESEG, 0, f8_do_pragma},
  {"constseg", P_CONSTSEG, 0, f8_do_pragma},
  {NULL, 0, 0, NULL},
};

static int
f8_process_pragma (const char *s)
{
  return process_pragma_tbl (f8_pragma_tbl, s);
}

static char f8_defaultRules[] = {
#include "peeph.rul"
  ""
};


static char *f8_keywords[] = {
  "at",
  "critical",
  "interrupt",
  "naked",
  0
};

static void
f8_genAssemblerEnd (FILE *of)
{
  if (options.out_fmt == 'E' && options.debug)
    {
      dwarf2FinalizeFile (of);
    }
}

extern void f8_init_asmops (void);

static void
f8_init (void)
{
  asm_addTree (&asm_asxxxx_mapping);

  f8_init_asmops ();
}

static void
f8_reset_regparm (struct sym_link *ftype)
{
  _G.regparam.n = 0;
  _G.regparam.ftype = ftype;
}

static int
f8_reg_parm (sym_link *l, bool reentrant)
{
  bool is_regarg = f8IsRegArg(_G.regparam.ftype, ++_G.regparam.n, 0);

  return (is_regarg ? _G.regparam.n : 0);
}

static bool
f8_parseOptions (int *pargc, char **argv, int *i)
{
  return false;
}

static void
f8_finaliseOptions (void)
{
  port->mem.default_local_map = data;
  port->mem.default_globl_map = data;
}

static void
f8_setDefaultOptions (void)
{
  options.nopeep = 0;
  options.stackAuto = 1;
  options.intlong_rent = 1;
  options.float_rent = 1;
  options.noRegParams = 0;

  options.data_loc = 0x2800; /* Assume a default of 6 KB of RAM for now. */
  options.code_loc = 0x4000;

  options.stack_loc = -1; /* Do not set the stack pointer in software- just use the device-specific reset value. */

  options.out_fmt = 'i';        /* Default output format is ihx */
}

static const char *
f8_getRegName (const struct reg_info *reg)
{
  if (reg)
    return reg->name;
  return "err";
}

static void
f8_genExtraArea (FILE *of, bool hasMain)
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
f8_genInitStartup (FILE *of)
{
  fprintf (of, "\tldw\ty, #0x%04x ; set stack pointer one above stack\n", options.stack_loc >= 0 ? (options.stack_loc + 1) : 0x4000);
  fprintf (of, "\tldw\tsp, y\n");

  /* Call external startup code */
  fprintf (of, "\tcall\t#___sdcc_external_startup\n");

  /* If external startup returned non-zero, skip init */
  fprintf (of, "\ttst\txl\n");
  fprintf (of, "\tjrz\t#__sdcc_init_data\n");
  fprintf (of, "\tjp\t#__sdcc_program_startup\n");

  /* Init static & global variables */
  fprintf (of, "__sdcc_init_data:\n");
  
  /* Zeroing memory (required by standard for static & global variables) */
  fprintf (of, "\tldw\tz, #l_DATA\n");
  fprintf (of, "\tjrz\t#00002$\n");
  fprintf (of, "\tclr\txl\n");
  fprintf (of, "00001$:\n");
  fprintf (of, "\tld (s_DATA - 1, z), xl\n");
  fprintf (of, "\taddw z, #-1\n");
  fprintf (of, "\tjrnz\t#00001$\n");
  fprintf (of, "00002$:\n");

  /* Copy l_INITIALIZER bytes from s_INITIALIZER to s_INITIALIZED */
  fprintf (of, "\tldw\tz, #l_INITIALIZER\n");
  fprintf (of, "\tjrz\t#00004$\n");
  fprintf (of, "00003$:\n");
  fprintf (of, "\tld\txl, (s_INITIALIZER - 1, z)\n");
  fprintf (of, "\tld\t(s_INITIALIZED - 1, z), xl\n");
  fprintf (of, "\taddw\tz, #-1\n");
  fprintf (of, "\tjrnz\t#00003$\n");
  fprintf (of, "00004$:\n");
}

int
f8_genIVT(struct dbuf_s * oBuf, symbol ** intTable, int intCount)
{
  dbuf_tprintf (oBuf, "\tjp #s_GSINIT ; reset\n");
  dbuf_tprintf (oBuf, "\tnop\n");
  if (interrupts[0])
    dbuf_printf (oBuf, "\tjp #%s ; interrupt handler\n", interrupts[0]->rname);
  else
    dbuf_printf (oBuf, "\treti ; no interrupt handler declared\n");
 
  return true;
}

/*----------------------------------------------------------------------*/
/* f8_dwarfRegNum - return the DWARF register number for a register.  */
/*----------------------------------------------------------------------*/
static int
f8_dwarfRegNum (const struct reg_info *reg)
{
  return reg->rIdx;
}

static bool
_hasNativeMulFor (iCode *ic, sym_link *left, sym_link *right)
{
  int result_size = IS_SYMOP (IC_RESULT (ic)) ? getSize (OP_SYM_TYPE (IC_RESULT(ic))) : 4;

  if (IS_BITINT (OP_SYM_TYPE (IC_RESULT(ic))) && SPEC_BITINTWIDTH (OP_SYM_TYPE (IC_RESULT(ic))) % 8)
    return false;

  if (ic->op != '*')
    return false;

  if (result_size == 1 || getSize (left) <= 1 && getSize (right) <= 1 && result_size == 2 && IS_UNSIGNED (left) && IS_UNSIGNED (right))
    return true;

  return false;
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
        if (size == 1)
          return (true);
        if (size == 2 && right % 8 <= 3)
          return (true);
        if ((size <= 2 || size == 4) && lbits == right * 2)
          return (true);
      }
      return (false);
    }

  return (false);
}

/** $1 is always the basename.
    $2 is always the output file.
    $3 varies
    $l is the list of extra options that should be there somewhere...
    MUST be terminated with a NULL.
*/
static const char *_linkCmd[] =
{
  "sdldf8", "-nf", "\"$1\"", NULL
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *f8AsmCmd[] =
{
  "sdasf8", "$l", "$3", "\"$1.asm\"", NULL
};

static const char *const _libs_f8[] = { "f8", NULL, };

PORT f8_port =
{
  TARGET_ID_F8,
  "f8",
  "F8",                         /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    true,                       /* We want f8_genIVT to be triggered */
    NO_MODEL,
    NO_MODEL,
    0,
  },
  {                             /* Assembler */
    f8AsmCmd,
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
    _libs_f8,                 /* libs */
  },
  {                             /* Peephole optimizer */
    f8_defaultRules,
    f8instructionSize,
    NULL,
    NULL,
    NULL,
    f8notUsed,
    f8canAssign,
    f8notUsedFrom,
    NULL,
    NULL,
    NULL,
  },
  /* Sizes: char, short, int, long, long long, ptr, fptr, gptr, bit, float, max */
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
  { f8_genExtraArea, NULL },
  1,                            /* default ABI revision */
  {                             /* stack information */
    -1,                         /* stack grows down */
     0,
     7,                         /* isr overhead */
     2,                         /* call overhead */
     0,
     2,
     0,                         /* sp points to next free stack location */
  },     
  { 
    -1,                         /* shifts never use support routines */
    false,                      /* don't use support routine for int x int -> long multiplication */
  },
  { f8_emitDebuggerSymbol,
    {
      f8_dwarfRegNum,
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
    {4, 5, 5},                  /* sizeofMatchJump[] todo: check!*/
    {4, 5, 5},                  /* sizeofRangeCompare[] todo:check!*/
    3,                          /* sizeofSubtract */
    5,                          /* sizeofDispatch - 1 byte for sllw y followed by 3 bytes for ldw y, (..., y) and 1 byte for jp (y) */
  },
  "_",
  f8_init,
  f8_parseOptions,
  f8_options,
  NULL,
  f8_finaliseOptions,
  f8_setDefaultOptions,
  f8_assignRegisters,
  f8_getRegName,
  0,
  NULL,
  f8_keywords,
  NULL,
  f8_genAssemblerEnd,
  f8_genIVT,
  0,                            /* no genXINIT code */
  f8_genInitStartup,            /* genInitStartup */
  f8_reset_regparm,
  f8_reg_parm,
  f8_process_pragma,            /* process_pragma */
  NULL,                         /* getMangledFunctionName */
  _hasNativeMulFor,             /* hasNativeMulFor */
  hasExtBitOp,                  /* hasExtBitOp */
  NULL,                         /* oclsExpense */
  TRUE,
  true,                         /* little endian */
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
  6,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};
