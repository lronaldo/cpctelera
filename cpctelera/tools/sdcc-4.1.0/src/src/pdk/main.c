/*-------------------------------------------------------------------------
  main.c - Padauk specific definitions.

  Philipp Klaus Krause <pkk@spth.de> 2012-2018

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
#include "dbuf_string.h"

#include "ralloc.h"
#include "gen.h"
#include "peep.h"

extern DEBUGFILE dwarf2DebugFile;
extern int dwarf2FinalizeFile(FILE *);

static char pdk_defaultRules[] = {
#include "peeph.rul"
  ""
};

static char *pdk_keywords[] = {
  "at",
  "code",
  "data",
  "interrupt",
  "naked",
  "near",
  "reentrant",
  "sfr",
  "sfr16",
  NULL
};

static void
pdk_genAssemblerStart (FILE *of)
{
  fprintf (of, "\n; default segment ordering in RAM for linker\n");
  tfprintf (of, "\t!area\n", DATA_NAME);
  tfprintf (of, "\t!area\n", OVERLAY_NAME);
  fprintf (of, "\n");
}

static void
pdk_genAssemblerEnd (FILE *of)
{
  if (options.out_fmt == 'E' && options.debug)
    dwarf2FinalizeFile (of);
}

int
pdk_genIVT(struct dbuf_s *oBuf, symbol **intTable, int intCount)
{
  dbuf_tprintf (oBuf, "\t.area\tHEADER (ABS)\n");
  dbuf_tprintf (oBuf, "\t.org\t 0x0020\n");
  if (interrupts[0])
    dbuf_tprintf (oBuf, "\tgoto\t%s\n", interrupts[0]->rname);
  else
    dbuf_tprintf (oBuf, "\treti\n");

  return (true);
}

static void
pdk_genInitStartup (FILE *of)
{
  fprintf (of, "\t.area\tPREG (ABS)\n");
  fprintf (of, "\t.org 0x00\n");
  fprintf (of, "p::\n");
  fprintf (of, "\t.ds 2\n");
  

  fprintf (of, "\t.area\tHEADER (ABS)\n"); // In the header we have 16 bytes. First should be nop.
  fprintf (of, "\t.org 0x0000\n");
  fprintf (of, "\tnop\n"); // First word is a jump to self-test routine at end of ROM on some new devices.

  // Zero upper byte of pseudo-register p to make p usable for pointers.
  fprintf (of, "\tclear\tp+1\n");

  // Initialize stack pointer
  if (options.stack_loc >= 0)
    {
      fprintf (of, "\tmov\ta, #0x%02x\n", options.stack_loc);
      fprintf (of, "\tmov\tsp, a\n");
    }
  else
    {
      fprintf (of, "\tmov\ta, #s_OSEG\n");
      fprintf (of, "\tadd\ta, #l_OSEG + 1\n");
      fprintf (of, "\tand\ta, #0xfe\n");
      fprintf (of, "\tmov\tsp, a\n");
    }

  fprintf (of, "\tcall\t__sdcc_external_startup\n");
  fprintf (of, "\tgoto\ts_GSINIT\n");

  tfprintf (of, "\t!area\n", STATIC_NAME);

  /* Init static & global variables */
  fprintf (of, "__sdcc_init_data:\n");

  /* Zeroing memory (required by standard for static & global variables) */
  fprintf (of, "\tmov\ta, #s_DATA\n");
  fprintf (of, "\tmov\tp, a\n");
  fprintf (of, "\tgoto\t00002$\n");
  fprintf (of, "00001$:\n");
  fprintf (of, "\tmov\ta, #0x00\n");
  fprintf (of, "\tidxm\tp, a\n");
  fprintf (of, "\tinc\tp\n");
  fprintf (of, "\tmov\ta, #s_DATA\n");
  fprintf (of, "00002$:\n");
  fprintf (of, "\tadd\ta, #l_DATA\n");
  fprintf (of, "\tceqsn\ta, p\n");
  fprintf (of, "\tgoto\t00001$\n");
}

static void
pdk_init (void)
{
  asm_addTree (&asm_asxxxx_smallpdk_mapping);
}

static void
pdk_reset_regparm (struct sym_link *funcType)
{
}

static int
pdk_reg_parm (sym_link *l, bool reentrant)
{
  return (0);
}

static bool
pdk_parseOptions (int *pargc, char **argv, int *i)
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
pdk_finaliseOptions (void)
{
  port->mem.default_local_map = data;
  port->mem.default_globl_map = data;
}

static void
pdk_setDefaultOptions (void)
{
  options.out_fmt = 'i';        /* Default output format is ihx */
  options.data_loc = 0x02;      /* First two bytes of RAM are used for the pseudo-register p */
  options.code_loc = 0x0022;
  options.stack_loc = -1;
}

static const char *
pdk_getRegName (const struct reg_info *reg)
{
  if (reg)
    return reg->name;
  return "err";
}

static bool
_hasNativeMulFor (iCode *ic, sym_link *left, sym_link *right)
{
  int result_size = IS_SYMOP (IC_RESULT(ic)) ? getSize (OP_SYM_TYPE (IC_RESULT(ic))) : 4;

  if (ic->op != '*')
    return (false);

  return ((IS_LITERAL (left) || IS_LITERAL (right)) && result_size == 1);
}

/* Indicate which extended bit operations this backend supports */
static bool
hasExtBitOp (int op, int size)
{
  return (false);
}

static const char *
get_model (void)
{
    return(options.stackAuto ? "pdk15-stack-auto" : "pdk15");
}

/** $1 is always the basename.
    $2 is always the output file.
    $3 varies
    $l is the list of extra options that should be there somewhere...
    MUST be terminated with a NULL.
*/
static const char *_linkCmd[] =
{
  "sdldpdk", "-nf", "\"$1\"", NULL
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *pdk13AsmCmd[] =
{
  "sdaspdk13", "$l", "$3", "\"$1.asm\"", NULL
};

static const char *const _libs_pdk13[] = { "pdk13", NULL, };

PORT pdk13_port =
{
  TARGET_ID_PDK13,
  "pdk13",
  "PDK13",                       /* Target name */
  0,                             /* Processor name */
  {
    glue,
    true,
    NO_MODEL,
    NO_MODEL,
    0,                          /* model == target */
  },
  {                             /* Assembler */
    pdk13AsmCmd,
    0,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _linkCmd,
    0,                          //LINKCMD,
    0,
    ".rel",
    1,
    0,                          /* crt */
    _libs_pdk13,                /* libs */
  },
  {                             /* Peephole optimizer */
    pdk_defaultRules,
    pdkinstructionSize,
    0,
    0,
    0,
    pdknotUsed,
    0,
    pdknotUsedFrom,
    0,
    0,
    0,
  },
  /* Sizes: char, short, int, long, long long, ptr, fptr, gptr, bit, float, max */
  {
    1,                          /* char */
    2,                          /* short */
    2,                          /* int */
    4,                          /* long */
    8,                          /* long long */
    1,                          /* near ptr */
    2,                          /* far ptr */
    2,                          /* generic ptr */
    2,                          /* func ptr */
    0,                          /* banked func ptr */
    1,                          /* bit */
    4,                          /* float */
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
    "OSEG (OVR,DATA)",          /* overlay */
    "GSFINAL",                  /* gsfinal */
    "HOME",                     /* home */
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    "CONST",                    /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    0,                          /* iabs_name */
    0,                          /* name of segment for initialized variables */
    0,                          /* name of segment for copies of initialized variables in code space */
    0,
    0,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
  },
  { 0, 0 },
  {                             /* stack information */
     +1,                        /* direction: stack grows up */
     0,
     7,                         /* isr overhead */
     2,                         /* call overhead */
     0,
     2,
     1,                         /* sp points to next free stack location */
  },     
  { -1, false },                /* no int x int -> long multiplication support routine. */
  { pdk_emitDebuggerSymbol,
    {
      0,
      0,                        /* cfiSame */
      0,                        /* cfiUndef */
      0,                        /* addressSize */
      0,                        /* regNumRet */
      0,                        /* regNumSP */
      0,                        /* regNumBP */
      0,                        /* offsetSP */
    },
  },
  {
    256,                        /* maxCount */
    1,                          /* sizeofElement */
    {2, 0, 0},                  /* sizeofMatchJump[] - the 0s here ensure that we only generate jump tables for 8-bit operands, which is all the backend can handle */
    {4, 0, 0},                  /* sizeofRangeCompare[] */
    1,                          /* sizeofSubtract */
    2,                          /* sizeofDispatch */
  },
  "_",
  pdk_init,
  pdk_parseOptions,
  0,
  0,
  pdk_finaliseOptions,           /* finaliseOptions */
  pdk_setDefaultOptions,         /* setDefaultOptions */
  pdk_assignRegisters,
  pdk_getRegName,
  0,
  0,
  pdk_keywords,
  pdk_genAssemblerStart,
  pdk_genAssemblerEnd,
  pdk_genIVT,
  0,                            /* no genXINIT code */
  pdk_genInitStartup,           /* genInitStartup */
  pdk_reset_regparm,
  pdk_reg_parm,
  0,                            /* process_pragma */
  0,                            /* getMangledFunctionName */
  _hasNativeMulFor,             /* hasNativeMulFor */
  hasExtBitOp,                  /* hasExtBitOp */
  0,                            /* oclsExpense */
  false,                        /* data is represented in ROM using ret k instructions */
  true,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  false,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  0,                            /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  2,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *pdk14AsmCmd[] =
{
  "sdaspdk14", "$l", "$3", "\"$1.asm\"", NULL
};

static const char *const _libs_pdk14[] = { "pdk14", NULL, };

PORT pdk14_port =
{
  TARGET_ID_PDK14,
  "pdk14",
  "PDK14",                       /* Target name */
  0,                             /* Processor name */
  {
    glue,
    true,
    NO_MODEL,
    NO_MODEL,
    0,                          /* model == target */
  },
  {                             /* Assembler */
    pdk14AsmCmd,
    0,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _linkCmd,
    0,                          //LINKCMD,
    0,
    ".rel",
    1,
    0,                          /* crt */
    _libs_pdk14,                /* libs */
  },
  {                             /* Peephole optimizer */
    pdk_defaultRules,
    pdkinstructionSize,
    0,
    0,
    0,
    pdknotUsed,
    0,
    pdknotUsedFrom,
    0,
  },
  /* Sizes: char, short, int, long, long long, ptr, fptr, gptr, bit, float, max */
  {
    1,                          /* char */
    2,                          /* short */
    2,                          /* int */
    4,                          /* long */
    8,                          /* long long */
    1,                          /* near ptr */
    2,                          /* far ptr */
    2,                          /* generic ptr */
    2,                          /* func ptr */
    0,                          /* banked func ptr */
    1,                          /* bit */
    4,                          /* float */
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
    "OSEG (OVR,DATA)",          /* overlay */
    "GSFINAL",                  /* gsfinal */
    "HOME",                     /* home */
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    "CONST",                    /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    0,                          /* iabs_name */
    0,                          /* name of segment for initialized variables */
    0,                          /* name of segment for copies of initialized variables in code space */
    0,
    0,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
  },
  { 0, 0 },
  {                             /* stack information */
     +1,                        /* direction: stack grows up */
     0,
     7,                         /* isr overhead */
     2,                         /* call overhead */
     0,
     2,
     1,                         /* sp points to next free stack location */
  },     
  { -1, false },                /* no int x int -> long multiplication support routine. */
  { pdk_emitDebuggerSymbol,
    {
      0,
      0,                        /* cfiSame */
      0,                        /* cfiUndef */
      0,                        /* addressSize */
      0,                        /* regNumRet */
      0,                        /* regNumSP */
      0,                        /* regNumBP */
      0,                        /* offsetSP */
    },
  },
  {
    256,                        /* maxCount */
    1,                          /* sizeofElement */
    {2, 0, 0},                  /* sizeofMatchJump[] - the 0s here ensure that we only generate jump tables for 8-bit operands, which is all the backend can handle */
    {4, 0, 0},                  /* sizeofRangeCompare[] */
    1,                          /* sizeofSubtract */
    2,                          /* sizeofDispatch */
  },
  "_",
  pdk_init,
  pdk_parseOptions,
  0,
  0,
  pdk_finaliseOptions,           /* finaliseOptions */
  pdk_setDefaultOptions,         /* setDefaultOptions */
  pdk_assignRegisters,
  pdk_getRegName,
  0,
  0,
  pdk_keywords,
  pdk_genAssemblerStart,
  pdk_genAssemblerEnd,
  pdk_genIVT,
  0,                            /* no genXINIT code */
  pdk_genInitStartup,           /* genInitStartup */
  pdk_reset_regparm,
  pdk_reg_parm,
  0,                            /* process_pragma */
  0,                            /* getMangledFunctionName */
  _hasNativeMulFor,             /* hasNativeMulFor */
  hasExtBitOp,                  /* hasExtBitOp */
  0,                            /* oclsExpense */
  false,                        /* data is represented in ROM using ret k instructions */
  true,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  false,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  0,                            /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  2,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *pdk15AsmCmd[] =
{
  "sdaspdk15", "$l", "$3", "\"$1.asm\"", NULL
};

static const char *const _libs_pdk15[] = { "pdk15", NULL, };

PORT pdk15_port =
{
  TARGET_ID_PDK15,
  "pdk15",
  "PDK15",                       /* Target name */
  0,                             /* Processor name */
  {
    glue,
    true,
    NO_MODEL,
    NO_MODEL,
    &get_model,
  },
  {                             /* Assembler */
    pdk15AsmCmd,
    0,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm"
  },
  {                             /* Linker */
    _linkCmd,
    0,                          //LINKCMD,
    0,
    ".rel",
    1,
    0,                          /* crt */
    _libs_pdk15,                /* libs */
  },
  {                             /* Peephole optimizer */
    pdk_defaultRules,
    pdkinstructionSize,
    0,
    0,
    0,
    pdknotUsed,
    0,
    pdknotUsedFrom,
    0,
  },
  /* Sizes: char, short, int, long, long long, ptr, fptr, gptr, bit, float, max */
  {
    1,                          /* char */
    2,                          /* short */
    2,                          /* int */
    4,                          /* long */
    8,                          /* long long */
    1,                          /* near ptr */
    2,                          /* far ptr */
    2,                          /* generic ptr */
    2,                          /* func ptr */
    0,                          /* banked func ptr */
    1,                          /* bit */
    4,                          /* float */
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
    "OSEG (OVR,DATA)",          /* overlay */
    "GSFINAL",                  /* gsfinal */
    "HOME",                     /* home */
    NULL,                       /* xidata */
    NULL,                       /* xinit */
    "CONST",                    /* const_name */
    "CABS (ABS)",               /* cabs_name */
    "DABS (ABS)",               /* xabs_name */
    0,                          /* iabs_name */
    0,                          /* name of segment for initialized variables */
    0,                          /* name of segment for copies of initialized variables in code space */
    0,
    0,
    1,                          /* CODE  is read-only */
    1                           /* No fancy alignments supported. */
  },
  { 0, 0 },
  {                             /* stack information */
     +1,                        /* direction: stack grows up */
     0,
     7,                         /* isr overhead */
     2,                         /* call overhead */
     0,
     2,
     1,                         /* sp points to next free stack location */
  },     
  { -1, false },                /* no int x int -> long multiplication support routine. */
  { pdk_emitDebuggerSymbol,
    {
      0,
      0,                        /* cfiSame */
      0,                        /* cfiUndef */
      0,                        /* addressSize */
      0,                        /* regNumRet */
      0,                        /* regNumSP */
      0,                        /* regNumBP */
      0,                        /* offsetSP */
    },
  },
  {
    256,                        /* maxCount */
    1,                          /* sizeofElement */
    {2, 0, 0},                  /* sizeofMatchJump[] - the 0s here ensure that we only generate jump tables for 8-bit operands, which is all the backend can handle */
    {4, 0, 0},                  /* sizeofRangeCompare[] */
    1,                          /* sizeofSubtract */
    2,                          /* sizeofDispatch */
  },
  "_",
  pdk_init,
  pdk_parseOptions,
  0,
  0,
  pdk_finaliseOptions,           /* finaliseOptions */
  pdk_setDefaultOptions,         /* setDefaultOptions */
  pdk_assignRegisters,
  pdk_getRegName,
  0,
  0,
  pdk_keywords,
  pdk_genAssemblerStart,
  pdk_genAssemblerEnd,
  pdk_genIVT,
  0,                            /* no genXINIT code */
  pdk_genInitStartup,           /* genInitStartup */
  pdk_reset_regparm,
  pdk_reg_parm,
  0,                            /* process_pragma */
  0,                            /* getMangledFunctionName */
  _hasNativeMulFor,             /* hasNativeMulFor */
  hasExtBitOp,                  /* hasExtBitOp */
  0,                            /* oclsExpense */
  false,                        /* data is represented in ROM using ret k instructions */
  true,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  false,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  0,                            /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  2,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

