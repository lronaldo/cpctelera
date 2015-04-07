/* @file main.c
   xa51 specific general functions.
*/

#include "common.h"
#include "main.h"
#include "ralloc.h"
#include "gen.h"

static char _defaultRules[] =
{
#include "peeph.rul"
};

/* list of key words used by xa51 */
static char *_xa51_keywords[] =
{
  "at",
  "bit",
  "code",
  "critical",
  "data",
  "far",
  //"idata",
  "interrupt",
  "near",
  //"pdata",
  "reentrant",
  "sfr",
  "sbit",
  "using",
  "xdata",
  //"_data",
  //"_code",
  //"_generic",
  //"_near",
  //"_xdata",
  //"_pdata",
  //"_idata",
  "_naked",
  //"_overlay",
  NULL
};

/* rewinds declared in SDCCasm.c, function printCLine().
 * Currently commented out.
 *
 * extern int rewinds;
 */
void   _xa51_genAssemblerEnd (FILE * of)
{
  //fprintf (stderr, "Did %d rewind%c for c-line in asm comments\n", rewinds,
  //rewinds==1 ? '\0' : 's');
}

void xa51_assignRegisters (ebbIndex *);

static int regParmFlg = 0;      /* determine if we can register a parameter */

static void
_xa51_init (void)
{
  asm_addTree (&asm_xa_asm_mapping);
}

static void
_xa51_reset_regparm (void)
{
  regParmFlg = 0;
}

static int
_xa51_regparm (sym_link * l, bool reentrant)
{
  return 0; // for now
  /* for this processor it is simple
     can pass only the first parameter in a register */
  if (regParmFlg)
    return 0;

  regParmFlg = 1;
  return 1;
}

static bool
_xa51_parseOptions (int *pargc, char **argv, int *i)
{
  /* TODO: allow port-specific command line options to specify
   * segment names here.
   */
  return FALSE;
}

static void
_xa51_finaliseOptions (void)
{
  fprintf (stderr, "*** WARNING *** The XA51 port isn't yet complete\n");
  port->mem.default_local_map = istack;
  port->mem.default_globl_map = xdata;
  if (options.model!=MODEL_PAGE0) {
    fprintf (stderr, "-mxa51 only supports --model-page0\n");
    exit (1);
  }
}

static void
_xa51_setDefaultOptions (void)
{
  options.stackAuto=1;
  options.intlong_rent=1;
  options.float_rent=1;
  options.stack_loc=0x100;
  options.data_loc=0;
}

static const char *
_xa51_getRegName (struct regs *reg)
{
  if (reg)
    return reg->name;
  return "err";
}

/* Generate interrupt vector table. */
static int
_xa51_genIVT (struct dbuf_s * oBuf, symbol ** interrupts, int maxInterrupts)
{
  return TRUE;
}

/* Generate code to copy XINIT to XISEG */
static void _xa51_genXINIT (FILE * of) {
  fprintf (of, ";       _xa51_genXINIT() start\n");
  fprintf (of, "        mov     r0,#l_XINIT\n");
  fprintf (of, "        beq     00002$\n");
  fprintf (of, "        mov     r1,#s_XINIT\n");
  fprintf (of, "        mov     r2,#s_XISEG\n");
  fprintf (of, "00001$: movc    r3l,[r1+]\n");
  fprintf (of, "        mov     [r2+],r3l\n");
  fprintf (of, "        djnz    r0,00001$\n");
  fprintf (of, "00002$:\n");
  fprintf (of, ";       _xa51_genXINIT() end\n");
}

static void
_xa51_genAssemblerPreamble (FILE * of)
{
  symbol *mainExists=newSymbol("main", 0);
  mainExists->block=0;

  if ((mainExists=findSymWithLevel(SymbolTab, mainExists))) {
    fprintf (of, "\t.area GSINIT\t(CODE)\n");
    fprintf (of, "__interrupt_vect:\n");
    fprintf (of, "\t.dw\t0x8f00\n");
    fprintf (of, "\t.dw\t__sdcc_gsinit_startup\n");
    fprintf (of, "\n");
    fprintf (of, "__sdcc_gsinit_startup:\n");
    fprintf (of, ";\tmov.b\t_SCR,#0x01\t; page zero mode\n");
    fprintf (of, "\t.db 0x96,0x48,0x40,0x01\n");
    fprintf (of, "\tmov\tr7,#0x%04x\n", options.stack_loc);
    fprintf (of, "\tcall\t_external_startup\n");
    _xa51_genXINIT(of);
    fprintf (of, "\t.area CSEG\t(CODE)\n");
    fprintf (of, "\tcall\t_main\n");
    fprintf (of, "\treset\t;main should not return\n");
  }
}

/* dummy linker for now */
void xa_link(void) {
}

/* Do CSE estimation */
static bool cseCostEstimation (iCode *ic, iCode *pdic)
{
    operand *result = IC_RESULT(ic);
    sym_link *result_type = operandType(result);

    /* if it is a pointer then return ok for now */
    if (IC_RESULT(ic) && IS_PTR(result_type)) return 1;

    /* if bitwise | add & subtract then no since xa51 is pretty good at it
       so we will cse only if they are local (i.e. both ic & pdic belong to
       the same basic block */
    if (IS_BITWISE_OP(ic) || ic->op == '+' || ic->op == '-') {
        /* then if they are the same Basic block then ok */
        if (ic->eBBlockNum == pdic->eBBlockNum) return 1;
        else return 0;
    }

    /* for others it is cheaper to do the cse */
    return 1;
}

/* Indicate which extended bit operations this port supports */
static bool
hasExtBitOp (int op, int size)
{
  if (op == RRC
      || op == RLC
      || op == GETHBIT
     )
    return TRUE;
  else
    return FALSE;
}

/* Indicate the expense of an access to an output storage class */
static int
oclsExpense (struct memmap *oclass)
{
  if (IN_FARSPACE(oclass))
    return 1;

  return 0;
}

/** $1 is always the basename.
    $2 is always the output file.
    $3 varies
    $l is the list of extra options that should be there somewhere...
    MUST be terminated with a NULL.
*/
static const char *_linkCmd[] =
{
  "xa_link", "", "$1", NULL
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *_asmCmd[] =
{
  "xa_rasm", "$l", "$3", "$2", "$1.asm", NULL
};

static const char * const _libs[] = { STD_XA51_LIB, NULL, };

/* Globals */
PORT xa51_port =
{
  TARGET_ID_XA51,
  "xa51",
  "MCU 80C51XA",                /* Target name */
  NULL,                         /* Processor name */
  {
    glue,
    FALSE,                      /* Emit glue around main */
    MODEL_PAGE0,
    MODEL_PAGE0,
    NULL,                       /* model == target */
  },
  {
    _asmCmd,
    NULL,
    "",                         /* Options with debug */
    "",                         /* Options without debug */
    0,
    ".asm",
    NULL                        /* no do_assemble function */
  },
  {                             /* Linker */
    _linkCmd,
    NULL,
    NULL,
    ".rel",
    1,
    NULL,                       /* crt */
    _libs,                      /* libs */
  },
  {
    _defaultRules
  },
  {
        /* Sizes: char, short, int, long, long long, ptr, fptr, gptr, bit, float, max */
    1, 2, 2, 4, 8, 2, 2, 3, 1, 4, 4
  },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },           /* far, near, xstack, code */
  {
    "XSEG    (XDATA)",
    "STACK   (XDATA)",
    "CSEG    (CODE)",
    "DSEG    (DATA)",
    NULL, //"ISEG    (DATA)",
    NULL, //"PSEG    (PAG,XDATA)",
    "XSEG    (XDATA)",
    "BSEG    (BIT)",
    NULL, //"RSEG    (DATA)",
    "GSINIT  (CODE)",
    NULL, //"OSEG    (OVR,XDATA)",
    "GSFINAL (CODE)",
    "HOME    (CODE)",
    "XISEG   (XDATA)", // initialized xdata
    "XINIT   (CODE)", // a code copy of xiseg
    "CONST   (CODE)",           // const_name - const data (code or not)
    "CABS    (ABS,CODE)",       // cabs_name - const absolute data (code or not)
    "XABS    (ABS,XDATA)",      // xabs_name - absolute xdata
    "IABS    (ABS,DATA)",       // iabs_name - absolute data
    NULL, // default local map
    NULL, // default global map
    1
  },
  { NULL, NULL },
  {
    -1, // stack grows down
    0, // bank overhead NUY
    4, // isr overhead, page zero mode
    2, // function call overhead, page zero mode
    0, // reentrant overhead NUY
    0 // banked overhead NUY
  },
    /* xa51 has an 16 bit mul */
  {
    2, -2
  },
  {
    xa51_emitDebuggerSymbol
  },
  {
    255/3,      /* maxCount */
    3,          /* sizeofElement */
    /* The rest of these costs are bogus. They approximate */
    /* the behavior of src/SDCCicode.c 1.207 and earlier.  */
    {4,4,4},    /* sizeofMatchJump[] */
    {0,0,0},    /* sizeofRangeCompare[] */
    0,          /* sizeofSubtract */
    3,          /* sizeofDispatch */
  },
  "_",
  _xa51_init,
  _xa51_parseOptions,
  NULL,
  NULL,
  _xa51_finaliseOptions,
  _xa51_setDefaultOptions,
  xa51_assignRegisters,
  _xa51_getRegName,
  _xa51_keywords,
  _xa51_genAssemblerPreamble,
  _xa51_genAssemblerEnd,
  _xa51_genIVT,
  _xa51_genXINIT,
  NULL,                         /* genInitStartup */
  _xa51_reset_regparm,
  _xa51_regparm,
  NULL, // process_pragma()
  NULL, // getMangledFunctionName()
  NULL, // hasNativeMulFor()
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  TRUE, // use_dw_for_init
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* No array initializer support. */
  cseCostEstimation,
  NULL,                         /* no builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local static allowed */
  0,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};
