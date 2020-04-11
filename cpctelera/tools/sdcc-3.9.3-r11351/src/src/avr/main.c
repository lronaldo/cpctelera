/** @file main.c
    avr specific general functions.

    Note that mlh prepended _avr_ on the static functions.  Makes
    it easier to set a breakpoint using the debugger.
*/
#include "common.h"
#include "main.h"
#include "ralloc.h"
#include "gen.h"

static char _defaultRules[] = {
#include "peeph.rul"
};

/* list of key words used by msc51 */
static char *_avr_keywords[] = {
	"at",
	"code",
	"critical",
	"eeprom",
	"interrupt",
	"sfr",
	"xdata",
	"_code",
	"_eeprom",
	"_generic",
	"_xdata",
	"sram",
	"_sram",
	"flash",
	"_flash",
	NULL
};

static int regParmFlg = 0;	/* determine if we can register a parameter */

static void
_avr_init (void)
{
	asm_addTree (&asm_asxxxx_mapping);
}

static void
_avr_reset_regparm (void)
{
	regParmFlg = 0;
}

static int
_avr_regparm (sym_link * l, bool reentrant)
{
	/* the first eight bytes will be passed in
	   registers r16-r23. but we won't split variables
	   i.e. if not enough registers left to hold
	   the parameter then the whole parameter along
	   with rest of the parameters go onto the stack */
	if (regParmFlg < 8) {
		int size;
		if ((size = getSize (l)) > (8 - regParmFlg)) {
			/* all remaining go on stack */
			regParmFlg = 8;
			return 0;
		}
		regParmFlg += size;
		return 1;
	}

	return 0;
}

void avr_assignRegisters (ebbIndex *);

static bool
_avr_parseOptions (int *pargc, char **argv, int *i)
{
	/* TODO: allow port-specific command line options to specify
	 * segment names here.
	 */
	return FALSE;
}

static void
_avr_finaliseOptions (void)
{
	port->mem.default_local_map = port->mem.default_globl_map = xdata;
	/* change stack to be in far space */
	/* internal stack segment ;   
	   SFRSPACE       -   NO
	   FAR-SPACE      -   YES
	   PAGED          -   NO
	   DIRECT-ACCESS  -   NO
	   BIT-ACCESS     -   NO
	   CODE-ACESS     -   NO 
	   DEBUG-NAME     -   'B'
	   POINTER-TYPE   -   FPOINTER
	 */
	istack =
		allocMap (0, 1, 0, 0, 0, 0, options.stack_loc, ISTACK_NAME,
			  'B', FPOINTER);

	/* also change xdata to be direct space since we can use lds/sts */
	xdata->direct = 1;

}

static void
_avr_setDefaultOptions (void)
{
	options.stackAuto = 1;
}

static const char *
_avr_getRegName (struct regs *reg)
{
	if (reg)
		return reg->name;
	return "err";
}

static void
_avr_genAssemblerPreamble (FILE * of)
{

}

/* Generate interrupt vector table. */
static int
_avr_genIVT (struct dbuf_s * oBuf, symbol ** interrupts, int maxInterrupts)
{
  return TRUE;
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
static const char *_linkCmd[] = {
	"linkavr", "", "$1", NULL
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *_asmCmd[] = {
	"asavr", "$l" , "$3", "$1.s", NULL
};

/* Globals */
PORT avr_port = {
        TARGET_ID_AVR,
	"avr",
	"ATMEL AVR",		/* Target name */
	NULL,			/* processor */
	{
         glue,
	 TRUE,			/* Emit glue around main */
	 MODEL_LARGE | MODEL_SMALL,
	 MODEL_SMALL,
         NULL,                       /* model == target */
	},
	{
	 _asmCmd,
         NULL,
	 "-plosgff",		/* Options with debug */
	 "-plosgff",		/* Options without debug */
	 0,
	".s",
	 NULL,			/* no do_assemble */
	},
	{
	 _linkCmd,
         NULL,
	 NULL,
	 ".rel",
	 1},
	{
	 _defaultRules},
	{
	 /* Sizes: char, short, int, long, long long, ptr, fptr, gptr, bit, float, max */
	 1, 2, 2, 4, 8, 2, 2, 3, 1, 4, 4},
	
	 /* tags for generic pointers */
	{ 0x00, 0x40, 0x60, 0x80 },		/* far, near, xstack, code */
	
	{
	 "XSEG",
	 "STACK",
	 "CSEG",
	 "DSEG",
	 "ISEG",
	 NULL, //PSEG
	 "XSEG",
	 "BSEG",
	 "RSEG",
	 "GSINIT",
	 "OSEG",
	 "GSFINAL",
	 "HOME",
	 NULL, // initialized xdata
	 NULL, // a code copy of xiseg
	 "CONST   (CODE)",           // const_name - const data (code or not)
	 "CABS    (ABS,CODE)",       // cabs_name - const absolute data (code or not)
	 "XABS    (ABS,XDATA)",      // xabs_name - absolute xdata/pdata
	 "IABS    (ABS,DATA)",       // iabs_name - absolute idata/data
	 NULL,
	 NULL,
	 0,
	 },
        { NULL, NULL },
	{
	 -1, 1, 4, 1, 1, 0, 1},
	/* avr has an 8 bit mul */
	{
          1, -1, FALSE
        },
	{
          avr_emitDebuggerSymbol
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
	_avr_init,
	_avr_parseOptions,
	NULL,
	NULL,
	_avr_finaliseOptions,
	_avr_setDefaultOptions,
	avr_assignRegisters,
	_avr_getRegName,
	_avr_keywords,
	_avr_genAssemblerPreamble,
	NULL,				/* no genAssemblerEnd */
	_avr_genIVT,
	NULL, // _avr_genXINIT
	NULL,			/* genInitStartup */
	_avr_reset_regparm,
	_avr_regparm,
        NULL,
	NULL,
        NULL,
	hasExtBitOp,		/* hasExtBitOp */
	oclsExpense,		/* oclsExpense */
	FALSE,
	TRUE,			/* little endian */
	0,			/* leave lt */
	1,			/* transform gt ==> not le */
	0,			/* leave le */
	0,			/* leave ge */
	0,			/* leave !=  */
	0,			/* leave == */
	FALSE,                  /* No array initializer support. */
	0,                      /* no CSE cost estimation yet */
	NULL, 			/* no builtin functions */
	GPOINTER,		/* treat unqualified pointers as "generic" pointers */
	1,			/* reset labelKey to 1 */
	1,			/* globals & local static allowed */
	PORT_MAGIC
};
