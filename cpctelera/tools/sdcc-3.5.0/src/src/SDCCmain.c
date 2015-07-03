/*-------------------------------------------------------------------------
  SDCCmain.c - main file

             Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1999)

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

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <libgen.h>
#endif

#include <signal.h>
#include "common.h"
#include <ctype.h>
#include "dbuf_string.h"
#include "SDCCerr.h"
#include "SDCCmacro.h"
#include "SDCCargs.h"

#ifdef _WIN32
#include <process.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

/* REMOVE ME!!! */
extern int yyparse (void);

FILE *srcFile;                  /* source file */
const char *fullSrcFileName;    /* full name for the source file; */
                                /* can be NULL while c1mode or linking without compiling */
const char *fullDstFileName;    /* full name for the output file; */
                                /* only given by -o, otherwise NULL */
const char *dstFileName;        /* destination file name without extension */
const char *moduleName;         /* module name is same as module name base, but with all */
                                /* non-alphanumeric characters replaced with underscore */
int currRegBank = 0;
int RegBankUsed[4] = { 1, 0, 0, 0 };    /*JCF: Reg Bank 0 used by default */

int BitBankUsed;                /* MB: overlayable bit bank */
struct optimize optimize;
struct options options;
int preProcOnly = 0;
int noAssemble = 0;
set *preArgvSet = NULL;         /* pre-processor arguments  */
set *asmOptionsSet = NULL;      /* set of assembler options */
set *linkOptionsSet = NULL;     /* set of linker options */
set *libFilesSet = NULL;
set *libPathsSet = NULL;
set *relFilesSet = NULL;
set *dataDirsSet = NULL;        /* list of data search directories */
set *includeDirsSet = NULL;     /* list of include search directories */
set *userIncDirsSet = NULL;     /* list of user include directories */
set *libDirsSet = NULL;         /* list of lib search directories */
bool regalloc_dry_run = FALSE;

static const char *dstPath = "";          /* path for the output files; */
                                          /* "" is equivalent with cwd */
static const char *moduleNameBase = NULL; /* module name base is source file without path and extension */
                                          /* can be NULL while linking without compiling */

/* uncomment JAMIN_DS390 to always override and use ds390 port
  for mcs51 work.  This is temporary, for compatibility testing. */
/* #define JAMIN_DS390 */
#ifdef JAMIN_DS390
static int ds390_jammed = 0;
#endif

/* Globally accessible scratch buffer for file names.
   TODO: replace them with local buffers */
char buffer[PATH_MAX * 2];

#define LENGTH(_a)      (sizeof (_a) / sizeof (*(_a)))

#define OPTION_HELP                 "--help"
#define OPTION_OUT_FMT_IHX          "--out-fmt-ihx"
#define OPTION_OUT_FMT_S19          "--out-fmt-s19"
#define OPTION_HUGE_MODEL           "--model-huge"
#define OPTION_LARGE_MODEL          "--model-large"
#define OPTION_MEDIUM_MODEL         "--model-medium"
#define OPTION_SMALL_MODEL          "--model-small"
#define OPTION_PEEP_FILE            "--peep-file"
#define OPTION_LIB_PATH             "--lib-path"
#define OPTION_CALLEE_SAVES         "--callee-saves"
#define OPTION_STACK_LOC            "--stack-loc"
#define OPTION_XSTACK_LOC           "--xstack-loc"
#define OPTION_DATA_LOC             "--data-loc"
#define OPTION_IDATA_LOC            "--idata-loc"
#define OPTION_XRAM_LOC             "--xram-loc"
#define OPTION_CODE_LOC             "--code-loc"
#define OPTION_IRAM_SIZE            "--iram-size"
#define OPTION_XRAM_SIZE            "--xram-size"
#define OPTION_CODE_SIZE            "--code-size"
#define OPTION_VERSION              "--version"
#define OPTION_NO_LABEL_OPT         "--nolabelopt"
#define OPTION_NO_LOOP_INV          "--noinvariant"
#define OPTION_NO_LOOP_IND          "--noinduction"
#define OPTION_LESS_PEDANTIC        "--less-pedantic"
#define OPTION_DISABLE_WARNING      "--disable-warning"
#define OPTION_WERROR               "--Werror"
#define OPTION_DEBUG                "--debug"
#define OPTION_NO_GCSE              "--nogcse"
#define OPTION_SHORT_IS_8BITS       "--short-is-8bits"
#define OPTION_NO_XINIT_OPT         "--no-xinit-opt"
#define OPTION_NO_CCODE_IN_ASM      "--no-c-code-in-asm"
#define OPTION_ICODE_IN_ASM         "--i-code-in-asm"
#define OPTION_PRINT_SEARCH_DIRS    "--print-search-dirs"
#define OPTION_MSVC_ERROR_STYLE     "--vc"
#define OPTION_USE_STDOUT           "--use-stdout"
#define OPTION_NO_PEEP_COMMENTS     "--no-peep-comments"
#define OPTION_VERBOSE_ASM          "--fverbose-asm"
#define OPTION_OPT_CODE_SPEED       "--opt-code-speed"
#define OPTION_OPT_CODE_SIZE        "--opt-code-size"
#define OPTION_STD_C89              "--std-c89"
#define OPTION_STD_C99              "--std-c99"
#define OPTION_STD_C11              "--std-c11"
#define OPTION_STD_SDCC89           "--std-sdcc89"
#define OPTION_STD_SDCC99           "--std-sdcc99"
#define OPTION_CODE_SEG             "--codeseg"
#define OPTION_CONST_SEG            "--constseg"
#define OPTION_DOLLARS_IN_IDENT     "--fdollars-in-identifiers"
#define OPTION_UNSIGNED_CHAR        "--funsigned-char"
#define OPTION_USE_NON_FREE         "--use-non-free"
#define OPTION_PEEP_RETURN          "--peep-return"
#define OPTION_NO_PEEP_RETURN       "--no-peep-return"
#define OPTION_NO_OPTSDCC_IN_ASM    "--no-optsdcc-in-asm"
#define OPTION_MAX_ALLOCS_PER_NODE  "--max-allocs-per-node"
#define OPTION_NO_LOSPRE            "--nolospre"
#define OPTION_ALLOW_UNSAFE_READ    "--allow-unsafe-read"
#define OPTION_DUMP_AST             "--dump-ast"
#define OPTION_DUMP_I_CODE          "--dump-i-code"
#define OPTION_DUMP_GRAPHS          "--dump-graphs"

static const OPTION optionsTable[] = {
  {0,   NULL, NULL, "General options"},
  {0,   OPTION_HELP, NULL, "Display this help"},
  {'v', OPTION_VERSION, NULL, "Display sdcc's version"},
  {0,   "--verbose", &options.verbose, "Trace calls to the preprocessor, assembler, and linker"},
  {'V', NULL, &options.verboseExec, "Execute verbosely. Show sub commands as they are run"},
  {'d', NULL, NULL, NULL},
  {'D', NULL, NULL, "Define macro as in -Dmacro"},
  {'I', NULL, NULL, "Add to the include (*.h) path, as in -Ipath"},
  {'A', NULL, NULL, NULL},
  {'U', NULL, NULL, NULL},
  {'M', NULL, NULL, "Preprocessor option"},
  {'W', NULL, NULL, "Pass through options to the pre-processor (p), assembler (a) or linker (l)"},
  {'S', NULL, &noAssemble, "Compile only; do not assemble or link"},
  {'c', "--compile-only", &options.cc_only, "Compile and assemble, but do not link"},
  {'E', "--preprocessonly", &preProcOnly, "Preprocess only, do not compile"},
  {0,   "--c1mode", &options.c1mode, "Act in c1 mode.  The standard input is preprocessed code, the output is assembly code."},
  {'o', NULL, NULL, "Place the output into the given path resp. file"},
  {0,   OPTION_PRINT_SEARCH_DIRS, &options.printSearchDirs, "display the directories in the compiler's search path"},
  {0,   OPTION_MSVC_ERROR_STYLE, &options.vc_err_style, "messages are compatible with Micro$oft visual studio"},
  {0,   OPTION_USE_STDOUT, NULL, "send errors to stdout instead of stderr"},
  {0,   "--nostdlib", &options.nostdlib, "Do not include the standard library directory in the search path"},
  {0,   "--nostdinc", &options.nostdinc, "Do not include the standard include directory in the search path"},
  {0,   OPTION_LESS_PEDANTIC, NULL, "Disable some of the more pedantic warnings"},
  {0,   OPTION_DISABLE_WARNING, NULL, "<nnnn> Disable specific warning"},
  {0,   OPTION_WERROR, NULL, "Treat the warnings as errors"},
  {0,   OPTION_DEBUG, NULL, "Enable debugging symbol output"},
  {0,   "--cyclomatic", &options.cyclomatic, "Display complexity of compiled functions"},
  {0,   OPTION_STD_C89, NULL, "Use C89 standard (slightly incomplete)"},
  {0,   OPTION_STD_SDCC89, NULL, "Use C89 standard with SDCC extensions (default)"},
  {0,   OPTION_STD_C99, NULL, "Use C99 standard (incomplete)"},
  {0,   OPTION_STD_SDCC99, NULL, "Use C99 standard with SDCC extensions"},
  {0,   OPTION_STD_C11, NULL, "Use C11 standard (very incomplete)"},
  {0,   OPTION_DOLLARS_IN_IDENT, &options.dollars_in_ident, "Permit '$' as an identifier character"},
  {0,   OPTION_UNSIGNED_CHAR, &options.unsigned_char, "Make \"char\" unsigned by default"},
  {0,   OPTION_USE_NON_FREE, &options.use_non_free, "Search / include non-free licensed libraries and header files"},

  {0,   NULL, NULL, "Code generation options"},
  {'m', NULL, NULL, "Set the port to use e.g. -mz80."},
  {'p', NULL, NULL, "Select port specific processor e.g. -mpic14 -p16f84"},
  {0,   OPTION_SMALL_MODEL, NULL, "internal data space is used (default)"},
  {0,   OPTION_MEDIUM_MODEL, NULL, "external paged data space is used"},
  {0,   OPTION_LARGE_MODEL, NULL, "external data space is used"},
  {0,   OPTION_HUGE_MODEL, NULL, "functions are banked, data in external space"},
  {0,   "--stack-auto", &options.stackAuto, "Stack automatic variables"},
  {0,   "--xstack", &options.useXstack, "Use external stack"},
  {0,   "--int-long-reent", &options.intlong_rent, "Use reentrant calls on the int and long support functions"},
  {0,   "--float-reent", &options.float_rent, "Use reentrant calls on the float support functions"},
  {0,   "--xram-movc", &options.xram_movc, "Use movc instead of movx to read xram (xdata)"},
  {0,   OPTION_CALLEE_SAVES, &options.calleeSavesSet, "<func[,func,...]> Cause the called function to save registers instead of the caller", CLAT_SET},
  {0,   "--profile", &options.profile, "On supported ports, generate extra profiling information"},
  {0,   "--fomit-frame-pointer", &options.omitFramePtr, "Leave out the frame pointer."},
  {0,   "--all-callee-saves", &options.all_callee_saves, "callee will always save registers used"},
  {0,   "--stack-probe", &options.stack_probe, "insert call to function __stack_probe at each function prologue"},
  {0,   OPTION_NO_XINIT_OPT, &options.noXinitOpt, "don't memcpy initialized xram from code"},
  {0,   OPTION_NO_CCODE_IN_ASM, &options.noCcodeInAsm, "don't include c-code as comments in the asm file"},
  {0,   OPTION_NO_PEEP_COMMENTS, &options.noPeepComments, "don't include peephole optimizer comments"},
  {0,   OPTION_SHORT_IS_8BITS, NULL, "Make short 8 bits (for old times sake)"},
  {0,   OPTION_CODE_SEG, NULL, "<name> use this name for the code segment"},
  {0,   OPTION_CONST_SEG, NULL, "<name> use this name for the const segment"},

  {0,   NULL, NULL, "Optimization options"},
  {0,   "--nooverlay", &options.noOverlay, "Disable overlaying leaf function auto variables"},
  {0,   OPTION_NO_GCSE, NULL, "Disable the GCSE optimisation"},
  {0,   OPTION_NO_LABEL_OPT, NULL, "Disable label optimisation"},
  {0,   OPTION_NO_LOOP_INV, NULL, "Disable optimisation of invariants"},
  {0,   OPTION_NO_LOOP_IND, NULL, "Disable loop variable induction"},
  {0,   "--nojtbound", &optimize.noJTabBoundary, "Don't generate boundary check for jump tables"},
  {0,   "--noloopreverse", &optimize.noLoopReverse, "Disable the loop reverse optimisation"},
  {0,   "--no-peep", &options.nopeep, "Disable the peephole assembly file optimisation"},
  {0,   "--no-reg-params", &options.noRegParams, "On some ports, disable passing some parameters in registers"},
  {0,   "--peep-asm", &options.asmpeep, "Enable peephole optimization on inline assembly"},
  {0,   OPTION_PEEP_RETURN, NULL, "Enable peephole optimization for return instructions"},
  {0,   OPTION_NO_PEEP_RETURN, NULL, "Disable peephole optimization for return instructions"},
  {0,   OPTION_PEEP_FILE, &options.peep_file, "<file> use this extra peephole file", CLAT_STRING},
  {0,   OPTION_OPT_CODE_SPEED, NULL, "Optimize for code speed rather than size"},
  {0,   OPTION_OPT_CODE_SIZE, NULL, "Optimize for code size rather than speed"},
  {0,   OPTION_MAX_ALLOCS_PER_NODE, &options.max_allocs_per_node, "Maximum number of register assignments considered at each node of the tree decomposition", CLAT_INTEGER},
  {0,   OPTION_NO_LOSPRE, NULL, "Disable lospre"},
  {0,   OPTION_ALLOW_UNSAFE_READ, NULL, "Allow optimizations to read any memory location anytime"},

  {0,   NULL, NULL, "Internal debugging options"},
  {0,   OPTION_DUMP_AST, &options.dump_ast, "Dump front-end AST before generating i-code"},
  {0,   OPTION_DUMP_I_CODE, &options.dump_i_code, "Dump the i-code structure at all stages"},
  {0,   OPTION_DUMP_GRAPHS, &options.dump_graphs, "Dump graphs (control-flow, conflict, etc)"},
  {0,   OPTION_ICODE_IN_ASM, &options.iCodeInAsm, "Include i-code as comments in the asm file"},
  {0,   OPTION_VERBOSE_ASM, &options.verboseAsm, "Include code generator comments in the asm output"},

  {0,   NULL, NULL, "Linker options"},
  {'l', NULL, NULL, "Include the given library in the link"},
  {'L', NULL, NULL, "Add the next field to the library search path"},
  {0,   OPTION_LIB_PATH, &libPathsSet, "<path> use this path to search for libraries", CLAT_ADD_SET},
  {0,   OPTION_OUT_FMT_IHX, NULL, "Output in Intel hex format"},
  {0,   OPTION_OUT_FMT_S19, NULL, "Output in S19 hex format"},
  {0,   OPTION_XRAM_LOC, NULL, "<nnnn> External Ram start location", CLAT_INTEGER},
  {0,   OPTION_XRAM_SIZE, NULL, "<nnnn> External Ram size"},
  {0,   OPTION_IRAM_SIZE, &options.iram_size, "<nnnn> Internal Ram size", CLAT_INTEGER},
  {0,   OPTION_XSTACK_LOC, &options.xstack_loc, "<nnnn> External Stack start location", CLAT_INTEGER},
  {0,   OPTION_CODE_LOC, &options.code_loc, "<nnnn> Code Segment Location", CLAT_INTEGER},
  {0,   OPTION_CODE_SIZE, &options.code_size, "<nnnn> Code Segment size", CLAT_INTEGER},
  {0,   OPTION_STACK_LOC, &options.stack_loc, "<nnnn> Stack pointer initial value", CLAT_INTEGER},
  {0,   OPTION_DATA_LOC, &options.data_loc, "<nnnn> Direct data start location", CLAT_INTEGER},
  {0,   OPTION_IDATA_LOC, &options.idata_loc, NULL, CLAT_INTEGER},

  {0,   OPTION_NO_OPTSDCC_IN_ASM, &options.noOptsdccInAsm, "Do not emit .optsdcc in asm"},
  /* End of options */
  {0,   NULL}
};

/** Table of all unsupported options and help text to display when one
    is used.
*/
typedef struct
{
    /** shortOpt as in OPTIONS. */
  char shortOpt;
    /** longOpt as in OPTIONS. */
  const char *longOpt;
    /** Message to display inside W_UNSUPPORTED_OPT when this option
        is used. */
  const char *message;
} UNSUPPORTEDOPT;

static const UNSUPPORTEDOPT unsupportedOptTable[] = {
  {'X', NULL, "use --xstack-loc instead"},
  {'x', NULL, "use --xstack instead"},
  {'i', NULL, "use --idata-loc instead"},
  {'r', NULL, "use --xdata-loc instead"},
  {'s', NULL, "use --code-loc instead"},
  {'Y', NULL, "use -I instead"},
  {0, "--fommit-frame-pointer", "use --fomit-frame-pointer instead"},
};

/** List of all default constant macros.
 */
static const char *_baseValues[] = {
  "cpp", "sdcpp",
  "cppextraopts", "",
  /* Path seperator character */
  "sep", DIR_SEPARATOR_STRING,
  NULL
};

static const char *_preCmd = "{cpp} -nostdinc -Wall {cppstd}{cppextraopts} {fullsrcfilename} {cppoutfilename}";

PORT *port;

static PORT *_ports[] = {
#if !OPT_DISABLE_MCS51
  &mcs51_port,
#endif
#if !OPT_DISABLE_Z80
  &z80_port,
#endif
#if !OPT_DISABLE_Z180
  &z180_port,
#endif
#if !OPT_DISABLE_R2K
  &r2k_port,
#endif
#if !OPT_DISABLE_R3KA
  &r3ka_port,
#endif
#if !OPT_DISABLE_GBZ80
  &gbz80_port,
#endif
#if !OPT_DISABLE_TLCS90
  &tlcs90_port,
#endif
#if !OPT_DISABLE_AVR
  &avr_port,
#endif
#if !OPT_DISABLE_DS390
  &ds390_port,
#endif
#if !OPT_DISABLE_PIC16
  &pic16_port,
#endif
#if !OPT_DISABLE_PIC14
  &pic_port,
#endif
#if !OPT_DISABLE_TININative
  &tininative_port,
#endif
#if !OPT_DISABLE_XA51
  &xa51_port,
#endif
#if !OPT_DISABLE_DS400
  &ds400_port,
#endif
#if !OPT_DISABLE_HC08
  &hc08_port,
#endif
#if !OPT_DISABLE_S08
  &s08_port,
#endif
#if !OPT_DISABLE_STM8
  &stm8_port,
#endif
};

#define NUM_PORTS (sizeof(_ports)/sizeof(_ports[0]))

/** Sets the port to the one given by the command line option.
    @param    The name minus the option (eg 'mcs51')
    @return     0 on success.
*/
static void
_setPort (const char *name)
{
  int i;
  for (i = 0; i < NUM_PORTS; i++)
    {
      if (!strcmp (_ports[i]->target, name))
        {
          port = _ports[i];
          return;
        }
    }
  /* Error - didnt find */
  werror (E_UNKNOWN_TARGET, name);
  exit (EXIT_FAILURE);
}

/* Override the default processor with the one specified
 * on the command line */
static void
_setProcessor (char *_processor)
{
  port->processor = _processor;
}

static void
_validatePorts (void)
{
  int i;
  for (i = 0; i < NUM_PORTS; i++)
    {
      if (_ports[i]->magic != PORT_MAGIC)
        {
          /* Uncomment this line to debug which port is causing the problem
           * (the target name is close to the beginning of the port struct
           * and probably can be accessed just fine). */
          fprintf (stderr, "%s :", _ports[i]->target);
          wassertl (0, "Port definition structure is incomplete");
        }
    }
}


static char *
program_name (const char *path)
{
#ifdef _WIN32
  char fname[_MAX_FNAME];

  _splitpath (path, NULL, NULL, fname, NULL);
  return Safe_strdup (fname);
#else
  char *tmpPath = Safe_strdup (path);
  char *res = Safe_strdup (basename (tmpPath));

  Safe_free (tmpPath);
  return res;
#endif
}

/* search through the command line for the port */
static void
_findPort (int argc, char **argv)
{
  char *programName = program_name (*argv);
  int found = 0;
  int i;

  _validatePorts ();

  /* try to assign port by command line option */
  while (argc-- && !found)
    {
      if (!strncmp (*argv, "-m", 2))
        {
          _setPort (*argv + 2);
          found = 1;
        }
      argv++;
    }

  /* try to assign port by the name of the executable */
  for (i = 0; i < NUM_PORTS && !found; i++)
    {
      if (strstr (programName, _ports[i]->target))
        {
          _setPort (_ports[i]->target);
          found = 1;
        }
    }

  if (!found)
    {
      /* Use the first in the list as default */
      port = _ports[0];
    }

  Safe_free (programName);
}

/* search through the command line options for the processor */
static void
_findProcessor (int argc, char **argv)
{
  while (argc--)
    {
      if (!strncmp (*argv, "-p", 2))
        {
          _setProcessor (*argv + 2);
          return;
        }
      argv++;
    }

  /* no error if processor was not specified. */
}

/*-----------------------------------------------------------------*/
/* printVersionInfo - prints the version info        */
/*-----------------------------------------------------------------*/
void
printVersionInfo (FILE * stream)
{
  int i;

  fprintf (stream, "SDCC : ");
  for (i = 0; i < NUM_PORTS; i++)
    fprintf (stream, "%s%s", i == 0 ? "" : "/", _ports[i]->target);

  fprintf (stream, " " SDCC_VERSION_STR
#ifdef SDCC_SUB_VERSION_STR
           "/" SDCC_SUB_VERSION_STR
#endif
           " #%s (%s) (%s)\n", getBuildNumber (), getBuildDate (), getBuildEnvironment ());
  fprintf (stream, "published under GNU General Public License (GPL)\n");
}

static void
printOptions (const OPTION * optionsTable, FILE * stream)
{
  int i;
  for (i = 0; optionsTable[i].shortOpt != 0 || optionsTable[i].longOpt != NULL || optionsTable[i].help != NULL; i++)
    {
      if (!optionsTable[i].shortOpt && !optionsTable[i].longOpt && optionsTable[i].help)
        {
          fprintf (stream, "\n%s:\n", optionsTable[i].help);
        }
      else
        {
          fprintf (stream, "  %c%c  %-20s  %s\n",
                   optionsTable[i].shortOpt != 0 ? '-' : ' ',
                   optionsTable[i].shortOpt != 0 ? optionsTable[i].shortOpt : ' ',
                   optionsTable[i].longOpt != NULL ? optionsTable[i].longOpt : "",
                   optionsTable[i].help != NULL ? optionsTable[i].help : "");
        }
    }
}

/*-----------------------------------------------------------------*/
/* printUsage - prints command line syntax         */
/*-----------------------------------------------------------------*/
static void
printUsage (void)
{
  int i;
  FILE *stream = stderr;

  printVersionInfo (stream);
  fprintf (stream,
           "Usage : sdcc [options] filename\n"
           "Options :-\n");

  printOptions (optionsTable, stream);

  for (i = 0; i < NUM_PORTS; i++)
    {
      if (_ports[i]->poptions != NULL)
        {
          fprintf (stream, "\nSpecial options for the %s port:\n", _ports[i]->target);
          printOptions (_ports[i]->poptions, stream);
        }
    }
}

/*-----------------------------------------------------------------*/
/* setParseWithComma - separates string with comma to a set        */
/*-----------------------------------------------------------------*/
void
setParseWithComma (set ** dest, const char *src)
{
  const char *p, *end;
  struct dbuf_s dbuf;

  /* skip the initial white spaces */
  while (isspace ((unsigned char) *src))
    ++src;

  /* skip the trailing white spaces */
  end = &src[strlen (src) - 1];
  while (end >= src && isspace ((unsigned char) *end))
    --end;
  ++end;

  p = src;
  while (src < end)
    {
      dbuf_init (&dbuf, 16);

      while (p < end && ',' != *p)
        ++p;
      dbuf_append (&dbuf, src, p - src);
      addSet (dest, dbuf_detach_c_str (&dbuf));

      src = ++p;
    }
}

/*-----------------------------------------------------------------*/
/* setDefaultOptions - sets the default options                    */
/*-----------------------------------------------------------------*/
static void
setDefaultOptions (void)
{
  /* first the options part */
  options.stack_loc = 0;        /* stack pointer initialised to 0 */
  options.xstack_loc = 1;       /* xternal stack starts at 1 */
  options.code_loc = 0;         /* code starts at 0 */
  options.data_loc = 0;         /* JCF: By default let the linker locate data */
  options.xdata_loc = 1;        /* MB: Do not use address 0 by default as it equals NULL */
  options.idata_loc = 0;        /* MB: No need to limit idata to 0x80-0xFF */
  options.nopeep = 0;
  options.model = port->general.default_model;
  options.nostdlib = 0;
  options.nostdinc = 0;
  options.verbose = 0;
  options.shortis8bits = 0;
  options.std_sdcc = 1;         /* enable SDCC language extensions */
  options.std_c99 = 1;          /* default to C99, to enable inlining of standard functions */
  options.std_c11 = 0;          /* default to C99 until more C11 support */
  options.code_seg = CODE_NAME ? Safe_strdup (CODE_NAME) : NULL;        /* default to CSEG for generated code */
  options.const_seg = CONST_NAME ? Safe_strdup (CONST_NAME) : NULL;     /* default to CONST for generated code */
  options.stack10bit = 0;
  options.out_fmt = 0;
  options.dump_graphs = 0;

  /* now for the optimizations */
  /* turn on the everything */
  optimize.global_cse = 1;
  optimize.label1 = 1;
  optimize.label2 = 1;
  optimize.label3 = 1;
  optimize.label4 = 1;
  optimize.loopInvariant = 1;
  optimize.loopInduction = 1;
  options.max_allocs_per_node = 3000;
  optimize.lospre = 1;
  optimize.allow_unsafe_read = 0;

  /* now for the ports */
  port->setDefaultOptions ();
}

/*-----------------------------------------------------------------*/
/* processFile - determines the type of file from the extension    */
/*-----------------------------------------------------------------*/
static void
processFile (char *s)
{
  const char *extp;
  struct dbuf_s ext;
  struct dbuf_s path;

  dbuf_init (&ext, 128);
  dbuf_init (&path, PATH_MAX);

  /* get the file extension.
     If no '.' then we don't know what the file type is
     so give a warning and return */
  if (!dbuf_splitFile (s, &path, &ext))
    {
      werror (W_UNKNOWN_FEXT, s);

      dbuf_destroy (&ext);
      dbuf_destroy (&path);

      return;
    }

  /* otherwise depending on the file type */
  extp = dbuf_c_str (&ext);
  if (STRCASECMP (extp, ".c") == 0)
    {
      char *p, *m;

      dbuf_destroy (&ext);

      /* source file name : not if we already have a
         source file */
      if (fullSrcFileName)
        {
          werror (W_TOO_MANY_SRC, s);

          dbuf_destroy (&path);

          return;
        }

      /* the only source file */
      fullSrcFileName = s;
      if (!(srcFile = fopen (fullSrcFileName, "r")))
        {
          werror (E_FILE_OPEN_ERR, s);

          dbuf_destroy (&path);

          exit (EXIT_FAILURE);
        }

      /* get rid of any path information
         for the module name; */
      dbuf_init (&ext, 128);

      dbuf_splitPath (dbuf_c_str (&path), NULL, &ext);
      dbuf_destroy (&path);

      moduleNameBase = Safe_strdup (dbuf_c_str (&ext));
      m = dbuf_detach (&ext);

      for (p = m; *p; ++p)
        if (!isalnum ((unsigned char) *p))
          *p = '_';
      moduleName = m;
      return;
    }

  /* if the extension is type rel_ext
     additional object file will be passed to the linker */
  if (STRCASECMP (extp, port->linker.rel_ext) == 0)
    {
      dbuf_destroy (&ext);
      dbuf_destroy (&path);

      addSet (&relFilesSet, Safe_strdup (s));
      return;
    }

  /* if .lib or .LIB */
  if (STRCASECMP (extp, ".lib") == 0)
    {
      dbuf_destroy (&ext);
      dbuf_destroy (&path);

      addSet (&libFilesSet, Safe_strdup (s));
      return;
    }

  dbuf_destroy (&ext);
  dbuf_destroy (&path);

  werror (W_UNKNOWN_FEXT, s);
}

static void
_setModel (int model, const char *sz)
{
  if (port->general.supported_models & model)
    options.model = model;
  else
    werror (W_UNSUPPORTED_MODEL, sz, port->target);
}

/** Gets the string argument to this option.  If the option is '--opt'
    then for input of '--optxyz' or '--opt xyz' returns xyz.
*/
char *
getStringArg (const char *szStart, char **argv, int *pi, int argc)
{
  if (argv[*pi][strlen (szStart)])
    {
      return &argv[*pi][strlen (szStart)];
    }
  else
    {
      ++(*pi);
      if (*pi >= argc)
        {
          werror (E_ARGUMENT_MISSING, szStart);
          /* Die here rather than checking for errors later. */
          exit (EXIT_FAILURE);
        }
      else
        {
          return argv[*pi];
        }
    }
}

/** Gets the integer argument to this option using the same rules as
    getStringArg.
*/
int
getIntArg (const char *szStart, char **argv, int *pi, int argc)
{
  char *p;
  int val;
  char *str = getStringArg (szStart, argv, pi, argc);

  val = strtol (str, &p, 0);
  if (p == str || *p != '\0')
    {
      werror (E_BAD_INT_ARGUMENT, szStart);
      /* Die here rather than checking for errors later. */
      exit (EXIT_FAILURE);
    }
  return val;
}

static void
verifyShortOption (const char *opt)
{
  if (strlen (opt) != 2)
    {
      werror (W_EXCESS_SHORT_OPTIONS, opt);
    }
}

static bool
tryHandleUnsupportedOpt (char **argv, int *pi)
{
  if (argv[*pi][0] == '-')
    {
      const char *longOpt = "";
      char shortOpt = -1;
      int i;

      if (argv[*pi][1] == '-')
        {
          /* Long option. */
          longOpt = argv[*pi];
        }
      else
        {
          shortOpt = argv[*pi][1];
        }
      for (i = 0; i < LENGTH (unsupportedOptTable); i++)
        {
          if (unsupportedOptTable[i].shortOpt == shortOpt ||
              (longOpt && unsupportedOptTable[i].longOpt && !strcmp (unsupportedOptTable[i].longOpt, longOpt)))
            {
              /* Found an unsupported opt. */
              struct dbuf_s dbuf;

              dbuf_init (&dbuf, 100);
              dbuf_printf (&dbuf, "%s%c%c", longOpt ? longOpt : "", shortOpt ? '-' : ' ', shortOpt ? shortOpt : ' ');
              werror (W_UNSUPP_OPTION, dbuf_detach_c_str (&dbuf), unsupportedOptTable[i].message);
              return 1;
            }
        }
      /* Didn't find in the table */
      return 0;
    }
  else
    {
      /* Not an option, so can't be unsupported :) */
      return 0;
    }
}

static bool
scanOptionsTable (const OPTION * optionsTable, char shortOpt, const char *longOpt, char **argv, int *pi, int argc)
{
  int i;

  for (i = 0; optionsTable[i].shortOpt != 0 || optionsTable[i].longOpt != NULL || optionsTable[i].help != NULL; i++)
    {
      if (optionsTable[i].shortOpt == shortOpt)
        {
          if (optionsTable[i].pparameter != NULL)
            {
              verifyShortOption (argv[*pi]);

              (*(int *) optionsTable[i].pparameter)++;

              return TRUE;
            }
        }
      else
        {
          size_t len = optionsTable[i].longOpt ? strlen (optionsTable[i].longOpt) : 0;

          if (longOpt &&
              (optionsTable[i].arg_type != CLAT_BOOLEAN ||
               (optionsTable[i].arg_type == CLAT_BOOLEAN && len == strlen (longOpt) && optionsTable[i].longOpt)) &&
              strncmp (optionsTable[i].longOpt, longOpt, len) == 0)
            {
              /* If it is a flag then we can handle it here */
              if (optionsTable[i].pparameter != NULL)
                {
                  switch (optionsTable[i].arg_type)
                    {
                    case CLAT_BOOLEAN:
                      (*(int *) optionsTable[i].pparameter)++;
                      break;

                    case CLAT_INTEGER:
                      *(int *) optionsTable[i].pparameter = getIntArg (optionsTable[i].longOpt, argv, pi, argc);
                      break;

                    case CLAT_STRING:
                      if (*(char **) optionsTable[i].pparameter)
                        Safe_free (*(char **) optionsTable[i].pparameter);
                      *(char **) optionsTable[i].pparameter =
                        Safe_strdup (getStringArg (optionsTable[i].longOpt, argv, pi, argc));
                      break;

                    case CLAT_SET:
                      if (*(set **) optionsTable[i].pparameter)
                        {
                          deleteSet ((set **) optionsTable[i].pparameter);
                        }
                      setParseWithComma ((set **) optionsTable[i].pparameter,
                                         getStringArg (optionsTable[i].longOpt, argv, pi, argc));
                      break;

                    case CLAT_ADD_SET:
                      addSet ((set **) optionsTable[i].pparameter,
                              Safe_strdup (getStringArg (optionsTable[i].longOpt, argv, pi, argc)));
                      break;
                    }
                  return TRUE;
                }
              else
                {
                  /* Not a flag.  Handled manually later. */
                  return FALSE;
                }
            }
        }
    }
  /* Didn't find in the table */
  return FALSE;
}

static bool
tryHandleSimpleOpt (char **argv, int *pi, int argc)
{
  if (argv[*pi][0] == '-')
    {
      const char *longOpt = "";
      char shortOpt = -1;

      if (argv[*pi][1] == '-')
        {
          /* Long option. */
          longOpt = argv[*pi];
        }
      else
        {
          shortOpt = argv[*pi][1];
        }

      if (scanOptionsTable (optionsTable, shortOpt, longOpt, argv, pi, argc))
        {
          return TRUE;
        }
      else if (port && port->poptions && scanOptionsTable (port->poptions, shortOpt, longOpt, argv, pi, argc))
        {
          return TRUE;
        }
      else
        {
          return FALSE;
        }
    }
  else
    {
      /* Not an option, so can't be handled. */
      return FALSE;
    }
}

/*-----------------------------------------------------------------*/
/* parseCmdLine - parses the command line and sets the options     */
/*-----------------------------------------------------------------*/
static int
parseCmdLine (int argc, char **argv)
{
  int i;

  /* go thru all whole command line */
  for (i = 1; i < argc; i++)
    {
      if (i >= argc)
        break;

      /* check port specific options before general ones */
      if (port->parseOption (&argc, argv, &i) == TRUE)
        {
          continue;
        }

      if (tryHandleUnsupportedOpt (argv, &i) == TRUE)
        {
          continue;
        }

      if (tryHandleSimpleOpt (argv, &i, argc) == TRUE)
        {
          continue;
        }

      /* options */
      if (argv[i][0] == '-' && argv[i][1] == '-')
        {
          if (strcmp (argv[i], OPTION_USE_STDOUT) == 0)
            {
              if (options.use_stdout == 0)
                {
                  options.use_stdout = 1;
                  dup2 (STDOUT_FILENO, STDERR_FILENO);
                }
              continue;
            }
          if (strcmp (argv[i], OPTION_HELP) == 0)
            {
              printUsage ();
              exit (EXIT_SUCCESS);
            }

          if (strcmp (argv[i], OPTION_OUT_FMT_IHX) == 0)
            {
              options.out_fmt = 'i';
              continue;
            }

          if (strcmp (argv[i], OPTION_OUT_FMT_S19) == 0)
            {
              options.out_fmt = 's';
              continue;
            }

          if (strcmp (argv[i], OPTION_SMALL_MODEL) == 0)
            {
              _setModel (MODEL_SMALL, argv[i]);
              continue;
            }

          if (strcmp (argv[i], OPTION_MEDIUM_MODEL) == 0)
            {
              _setModel (MODEL_MEDIUM, argv[i]);
              continue;
            }

          if (strcmp (argv[i], OPTION_LARGE_MODEL) == 0)
            {
              _setModel (MODEL_LARGE, argv[i]);
              continue;
            }

          if (strcmp (argv[i], OPTION_HUGE_MODEL) == 0)
            {
              _setModel (MODEL_HUGE, argv[i]);
              continue;
            }

          if (strcmp (argv[i], OPTION_VERSION) == 0)
            {
              printVersionInfo (stdout);
              exit (EXIT_SUCCESS);
              continue;
            }

          if (strcmp (argv[i], OPTION_XRAM_LOC) == 0)
            {
              int val = getIntArg (OPTION_XRAM_LOC, argv, &i, argc);
              if (options.xdata_loc == options.xstack_loc)
                options.xstack_loc = val;
              options.xdata_loc = val;
              continue;
            }

          if (strcmp (argv[i], OPTION_XRAM_SIZE) == 0)
            {
              options.xram_size = getIntArg (OPTION_XRAM_SIZE, argv, &i, argc);
              options.xram_size_set = TRUE;
              continue;
            }

          if (strcmp (argv[i], OPTION_NO_GCSE) == 0)
            {
              optimize.global_cse = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_NO_LOOP_INV) == 0)
            {
              optimize.loopInvariant = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_NO_LABEL_OPT) == 0)
            {
              optimize.label2 = 0;
              optimize.label4 = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_NO_LOOP_IND) == 0)
            {
              optimize.loopInduction = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_OPT_CODE_SPEED) == 0)
            {
              optimize.codeSpeed = 1;
              optimize.codeSize = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_OPT_CODE_SIZE) == 0)
            {
              optimize.codeSpeed = 0;
              optimize.codeSize = 1;
              continue;
            }

          if (strcmp (argv[i], OPTION_NO_LOSPRE) == 0)
            {
              optimize.lospre = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_ALLOW_UNSAFE_READ) == 0)
            {
              optimize.allow_unsafe_read = 1;
              continue;
            }

          if (strcmp (argv[i], OPTION_LESS_PEDANTIC) == 0)
            {
              options.lessPedantic = 1;
              setErrorLogLevel (ERROR_LEVEL_WARNING);
              continue;
            }

          if (strcmp (argv[i], OPTION_DISABLE_WARNING) == 0)
            {
              int w = getIntArg (OPTION_DISABLE_WARNING, argv, &i, argc);
              setWarningDisabled (w);
              continue;
            }

          if (strcmp (argv[i], OPTION_WERROR) == 0)
            {
              setWError (1);
              addSet (&preArgvSet, Safe_strdup ("-Werror"));
              continue;
            }

          if (strcmp (argv[i], OPTION_SHORT_IS_8BITS) == 0)
            {
              printf ("Option %s is deprecated and will be removed in the future.\n", OPTION_SHORT_IS_8BITS);
              options.shortis8bits = 1;
              continue;
            }

          if (strcmp (argv[i], OPTION_STD_C89) == 0)
            {
              options.std_c99 = 0;
              options.std_sdcc = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_STD_C99) == 0)
            {
              options.std_c99 = 1;
              options.std_sdcc = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_STD_C11) == 0)
            {
              options.std_c99 = 1;
              options.std_c11 = 1;
              options.std_sdcc = 0;
              continue;
            }

          if (strcmp (argv[i], OPTION_STD_SDCC89) == 0)
            {
              options.std_c99 = 0;
              options.std_sdcc = 1;
              continue;
            }

          if (strcmp (argv[i], OPTION_STD_SDCC99) == 0)
            {
              options.std_c99 = 1;
              options.std_sdcc = 1;
              continue;
            }

          if (strcmp (argv[i], OPTION_CODE_SEG) == 0)
            {
              struct dbuf_s segname;

              dbuf_init (&segname, 16);
              dbuf_printf (&segname, "%-8s(CODE)", getStringArg (OPTION_CODE_SEG, argv, &i, argc));
              if (options.code_seg)
                Safe_free (options.code_seg);
              options.code_seg = dbuf_detach (&segname);
              continue;
            }

          if (strcmp (argv[i], OPTION_CONST_SEG) == 0)
            {
              struct dbuf_s segname;

              dbuf_init (&segname, 16);
              dbuf_printf (&segname, "%-8s(CODE)", getStringArg (OPTION_CONST_SEG, argv, &i, argc));
              if (options.const_seg)
                Safe_free (options.const_seg);
              options.const_seg = dbuf_detach (&segname);
              continue;
            }

          if (strcmp (argv[i], OPTION_PEEP_RETURN) == 0)
            {
              options.peepReturn = 1;
              continue;
            }

          if (strcmp (argv[i], OPTION_NO_PEEP_RETURN) == 0)
            {
              options.peepReturn = -1;
              continue;
            }

          if (strcmp (argv[i], OPTION_DEBUG) == 0)
            {
              if (options.peepReturn == 0)
                  options.peepReturn = -1;
              options.debug = 1;
              continue;
            }

          werror (W_UNKNOWN_OPTION, argv[i]);
          continue;
        }

      /* if preceded by  '-' then option */
      if (*argv[i] == '-')
        {
          switch (argv[i][1])
            {
            case 'h':
              verifyShortOption (argv[i]);

              printUsage ();
              exit (EXIT_SUCCESS);
              break;

            case 'm':
              /* Used to select the port. But this has already been done. */
              break;

            case 'p':
              /* Used to select the processor in port. But this has
               * already been done. */
              break;

            case 'c':
              verifyShortOption (argv[i]);

              options.cc_only = 1;
              break;

            case 'L':
              addSet (&libPathsSet, Safe_strdup (getStringArg ("-L", argv, &i, argc)));
              break;

            case 'l':
              addSet (&libFilesSet, Safe_strdup (getStringArg ("-l", argv, &i, argc)));
              break;

            case 'o':
              {
                char *outName = getStringArg ("-o", argv, &i, argc);
                size_t len = strlen (outName);

                /* point to last character */
                if (IS_DIR_SEPARATOR (outName[len - 1]))
                  {
                    /* only output path specified */
                    dstPath = Safe_strndup (outName, len - 1);
                    fullDstFileName = NULL;
                  }
                else
                  {
                    struct dbuf_s path;

                    dbuf_init (&path, PATH_MAX);
                    fullDstFileName = Safe_strdup (outName);

                    /* get rid of the "."-extension */
                    dbuf_splitFile (outName, &path, NULL);

                    dstFileName = dbuf_detach_c_str (&path);

                    dbuf_init (&path, PATH_MAX);
                    /* strip module name to get path */
                    if (dbuf_splitPath (dstFileName, &path, NULL))
                      dstPath = dbuf_detach_c_str (&path);
                    else
                      dbuf_destroy (&path);
                  }
                break;
              }

            case 'W':
              /* pre-processer options */
              if (argv[i][2] == 'p')
                {
                  setParseWithComma (&preArgvSet, getStringArg ("-Wp", argv, &i, argc));
                }
              /* linker options */
              else if (argv[i][2] == 'l')
                {
                  setParseWithComma (&linkOptionsSet, getStringArg ("-Wl", argv, &i, argc));
                }
              /* assembler options */
              else if (argv[i][2] == 'a')
                {
                  setParseWithComma (&asmOptionsSet, getStringArg ("-Wa", argv, &i, argc));
                }
              else
                {
                  werror (W_UNKNOWN_OPTION, argv[i]);
                }
              break;

            case 'v':
              verifyShortOption (argv[i]);

              printVersionInfo (stdout);
              exit (EXIT_SUCCESS);
              break;

              /* preprocessor options */
            case 'M':
              {
                preProcOnly = 1;
                if (argv[i][2] == 'M')
                  addSet (&preArgvSet, Safe_strdup ("-MM"));
                else
                  addSet (&preArgvSet, Safe_strdup ("-M"));
                break;
              }

            case 'd':
            case 'D':
            case 'I':
            case 'A':
            case 'U':
              {
                char sOpt = argv[i][1];
                char *rest, *s;
                struct dbuf_s dbuf;

                if (argv[i][2] == ' ' || argv[i][2] == '\0')
                  {
                    i++;
                    if (i >= argc)
                      {
                        /* No argument. */
                        werror (E_ARGUMENT_MISSING, argv[i - 1]);
                        break;
                      }
                    else
                      {
                        rest = argv[i];
                      }
                  }
                else
                  rest = &argv[i][2];

                if (sOpt == 'Y')
                  sOpt = 'I';

                s = shell_escape (rest);
                dbuf_init (&dbuf, 256);
                dbuf_printf (&dbuf, "-%c%s", sOpt, s);
                Safe_free (s);
                addSet (&preArgvSet, dbuf_detach_c_str (&dbuf));
                if (sOpt == 'I')
                  {
                    addSet (&includeDirsSet, Safe_strdup (rest));
                    addSet (&userIncDirsSet, Safe_strdup (rest));
                  }
              }
              break;

            default:
              werror (W_UNKNOWN_OPTION, argv[i]);
            }
          continue;
        }

      /* no option must be a filename */
      if (options.c1mode)
        {
          werror (W_NO_FILE_ARG_IN_C1, argv[i]);
        }
      else
        {
          processFile (argv[i]);
        }
    }

  /* some sanity checks in c1 mode */
  if (options.c1mode)
    {
      const char *s;

      if (fullSrcFileName)
        {
          fclose (srcFile);
          werror (W_NO_FILE_ARG_IN_C1, fullSrcFileName);
        }
      fullSrcFileName = NULL;
      for (s = setFirstItem (relFilesSet); s != NULL; s = setNextItem (relFilesSet))
        {
          werror (W_NO_FILE_ARG_IN_C1, s);
        }
      for (s = setFirstItem (libFilesSet); s != NULL; s = setNextItem (libFilesSet))
        {
          werror (W_NO_FILE_ARG_IN_C1, s);
        }
      deleteSet (&relFilesSet);
      deleteSet (&libFilesSet);

      if (options.cc_only || noAssemble || preProcOnly)
        {
          werror (W_ILLEGAL_OPT_COMBINATION);
        }
      options.cc_only = noAssemble = preProcOnly = 0;
      if (!dstFileName)
        {
          werror (E_NEED_OPT_O_IN_C1);
          exit (EXIT_FAILURE);
        }
      else
        {
          char *p, *m;

          m = Safe_strdup (dstFileName);
          for (p = m; *p; ++p)
            if (!isalnum ((unsigned char) *p))
              *p = '_';
          moduleName = m;
        }
    }
  /* if no dstFileName given with -o, we've to find one: */
  if (!dstFileName)
    {
      const char *s;

      /* use the modulename from the C-source */
      if (fullSrcFileName)
        {
          struct dbuf_s path;

          if (*dstPath != '\0')
            {
              dbuf_init (&path, 128);
              dbuf_makePath (&path, dstPath, moduleNameBase);
              dstFileName = dbuf_detach_c_str (&path);
            }
          else
            dstFileName = Safe_strdup (moduleNameBase);
        }
      /* use the modulename from the first object file */
      else if ((s = peekSet (relFilesSet)) != NULL)
        {
          struct dbuf_s file;

          dbuf_init (&file, 128);

          /* get rid of the "."-extension */
          dbuf_splitFile (s, &file, NULL);

          s = dbuf_detach_c_str (&file);

          dbuf_init (&file, PATH_MAX);

          dbuf_splitPath (s, NULL, &file);

          if (*dstPath != '\0')
            {
              struct dbuf_s path;

              dbuf_init (&path, PATH_MAX);
              dbuf_makePath (&path, dstPath, dbuf_detach_c_str (&file));
              dstFileName = dbuf_detach_c_str (&path);
            }
          else
            {
              dstFileName = dbuf_detach_c_str (&file);
            }
        }
      /* else no module given: help text is displayed */
    }

  /* set int, long and float reentrancy based on stack-auto */
  if (options.stackAuto)
    {
      options.intlong_rent++;
      options.float_rent++;
    }

  /* if debug option is set then open the cdbFile */
  if (options.debug && fullSrcFileName)
    {
      struct dbuf_s adbFile;

      dbuf_init (&adbFile, PATH_MAX);
      dbuf_append_str (&adbFile, dstFileName);
      dbuf_append_str (&adbFile, ".adb");

      if (debugFile->openFile (dbuf_c_str (&adbFile)))
        debugFile->writeModule (moduleName);
      else
        werror (E_FILE_OPEN_ERR, dbuf_c_str (&adbFile));

      dbuf_destroy (&adbFile);
    }
  MSVC_style (options.vc_err_style);

  return 0;
}

/*-----------------------------------------------------------------*/
/* finalizeOptions - finalize (post-process) options               */
/*-----------------------------------------------------------------*/
static void
finalizeOptions (void)
{
  /* no peephole comments if not verbose asm */
  if (!options.verboseAsm)
    options.noPeepComments = 1;
}

static const char *
getOutFmtExt (void)
{
  switch (options.out_fmt)
    {
    default:
    case 'i':
      return ".ihx";

    case 's':
      return ".s19";

    case 'E':
      return ".elf";
    }
}

/*-----------------------------------------------------------------*/
/* linkEdit : - calls the linkage editor  with options             */
/*-----------------------------------------------------------------*/
static void
linkEdit (char **envp)
{
  FILE *lnkfile;
  int system_ret;
  const char *s;
  struct dbuf_s linkerScriptFileName;
  struct dbuf_s binFileName;
  char *buf, *tb;

  dbuf_init (&linkerScriptFileName, PATH_MAX);
  dbuf_init (&binFileName, PATH_MAX);

  if (port->linker.needLinkerScript)
    {
      char out_fmt = (options.out_fmt == 0) ? 'i' : options.out_fmt;

      if (NULL != fullDstFileName)
        {
          dbuf_append_str (&binFileName, fullDstFileName);
        }
      else
        {
          dbuf_append_str (&binFileName, dstFileName);
          dbuf_append_str (&binFileName, getOutFmtExt ());
        }

      /* first we need to create the <filename>.lk file */
      dbuf_printf (&linkerScriptFileName, "%s.lk", dstFileName);
      if (!(lnkfile = fopen (dbuf_c_str (&linkerScriptFileName), "w")))
        {
          werror (E_FILE_OPEN_ERR, dbuf_c_str (&linkerScriptFileName));
          exit (EXIT_FAILURE);
        }

      if (TARGET_Z80_LIKE)
        {
          fprintf (lnkfile, "-mjwx\n-%c %s\n", out_fmt, dbuf_c_str (&binFileName));
        }
      else                      /* For all the other ports which need linker script */
        {
          fprintf (lnkfile, "-muwx\n-%c %s\n", out_fmt, dbuf_c_str (&binFileName));
          if (TARGET_MCS51_LIKE)
            fprintf (lnkfile, "-M\n");
          if (!options.no_pack_iram)
            fprintf (lnkfile, "-Y\n");
          else
            werror (W_DEPRECATED_OPTION, "--no-pack-iram");
        }

      if (!TARGET_Z80_LIKE)   /* Not for the z80, gbz80 */
        {
          /* if iram size specified */
          if (options.iram_size)
            fprintf (lnkfile, "-I 0x%04x\n", options.iram_size);

          /* if stack size specified */
          if (options.stack_size)
            fprintf (lnkfile, "-S 0x%02x\n", options.stack_size);

          /* if xram size specified */
          if (options.xram_size_set)
            fprintf (lnkfile, "-X 0x%04x\n", options.xram_size);

          /* if code size specified */
          if (options.code_size)
            fprintf (lnkfile, "-C 0x%04x\n", options.code_size);
        }

      if (options.debug)
        fprintf (lnkfile, "-y\n");

#define WRITE_SEG_LOC(N, L) \
  if (N) \
  { \
    char *c, *segName; \
    segName = Safe_strdup (N); \
    c = strtok (segName, " \t"); \
    fprintf (lnkfile,"-b %s = 0x%04x\n", c, L); \
    if (segName) { Safe_free (segName); } \
  }

      if (!TARGET_Z80_LIKE)   /* Not for the z80, z180, gbz80 */
        {

          /* code segment start */
          WRITE_SEG_LOC (HOME_NAME, options.code_loc);

          /* data segment start. If zero, the linker chooses
             the best place for data */
          if (options.data_loc)
            {
              WRITE_SEG_LOC (DATA_NAME, options.data_loc);
            }

          /* xdata segment start. If zero, the linker chooses
             the best place for xdata */
          if (options.xdata_loc)
            {
              WRITE_SEG_LOC (XDATA_NAME, options.xdata_loc);
            }

          /* pdata/xstack segment start. If zero, the linker
             chooses the best place for them */
          if (options.xstack_loc)
            {
              WRITE_SEG_LOC (PDATA_NAME, options.xstack_loc);
            }

          /* indirect data */
          if (IDATA_NAME)
            {
              WRITE_SEG_LOC (IDATA_NAME, options.idata_loc);
            }

          /* bit segment start */
          WRITE_SEG_LOC (BIT_NAME, 0);

          /* stack start */
          if ((options.stack_loc) && (options.stack_loc < 0x100) && !TARGET_HC08_LIKE)
            {
              WRITE_SEG_LOC ("SSEG", options.stack_loc);
            }
        }
      else                      /* For the z80, z180, gbz80 */
        {
          WRITE_SEG_LOC ("_CODE", options.code_loc);
          WRITE_SEG_LOC ("_DATA", options.data_loc);
        }

      /* If the port has any special linker area declarations, get 'em */
      if (port->extraAreas.genExtraAreaLinkOptions)
        {
          port->extraAreas.genExtraAreaLinkOptions (lnkfile);
        }

      /* add the extra linker options */
      fputStrSet (lnkfile, linkOptionsSet);

      /* command line defined library paths if specified */
      for (s = setFirstItem (libPathsSet); s != NULL; s = setNextItem (libPathsSet))
        fprintf (lnkfile, "-k %s\n", s);

      /* standard library path */
      if (!options.nostdlib)
        {
          for (s = setFirstItem (libDirsSet); s != NULL; s = setNextItem (libDirsSet))
            fprintf (lnkfile, "-k %s\n", s);
        }

      /* command line defined library files if specified */
      for (s = setFirstItem (libFilesSet); s != NULL; s = setNextItem (libFilesSet))
        fprintf (lnkfile, "-l %s\n", s);

      /* standard library files */
      if (!options.nostdlib)
        {
          if (NULL != port->linker.libs)
            {
              const char *const *p;

              for (p = port->linker.libs; NULL != *p; ++p)
                {
                  fprintf (lnkfile, "-l %s\n", *p);
                }
            }
        }

      /* put in the object file, generated from the C cource */
      if (fullSrcFileName)
        {
          struct dbuf_s path;

          dbuf_init (&path, PATH_MAX);
          dbuf_printf (&path, "%s%s", dstFileName, port->linker.rel_ext);
          addSetHead (&relFilesSet, dbuf_detach (&path));
        }

      if (!options.no_std_crt0)
        {
          const char *const *p;
          set *crtSet = NULL;

          if (NULL != port->linker.crt)
            {
              struct dbuf_s crtpath;

              dbuf_init (&crtpath, PATH_MAX);

              for (p = port->linker.crt; NULL != *p; ++p)
                {
                  /* Try to find where C runtime files are ...
                     It is very important for this file to be first on the linking proccess
                     so the areas are set in the correct order, expecially _GSINIT */
                  for (s = setFirstItem (libDirsSet); s != NULL; s = setNextItem (libDirsSet))
                    {
                      dbuf_set_length (&crtpath, 0);
                      dbuf_printf (&crtpath, "%s%c%s", s, DIR_SEPARATOR_CHAR, *p);

                      if (!access (dbuf_c_str (&crtpath), 0))   /* Found it! */
                        {
                          /* append C runtime file to the crt list */
                          addSet (&crtSet, Safe_strdup (dbuf_c_str (&crtpath)));
                          break;
                        }
                    }
                  if (NULL == s)
                    {
                      /* not found in standard library directories, serch in user defined library paths */
                      /* TODO: sould be crt searched here at all? */
                      for (s = setFirstItem (libPathsSet); s != NULL; s = setNextItem (libPathsSet))
                        {
                          dbuf_set_length (&crtpath, 0);
                          dbuf_printf (&crtpath, "%s%c%s", s, DIR_SEPARATOR_CHAR, *p);

                          if (!access (dbuf_c_str (&crtpath), 0))       /* Found it! */
                            {
                              /* append C runtime file to the crt list */
                              addSet (&crtSet, Safe_strdup (dbuf_c_str (&crtpath)));
                              break;
                            }
                        }
                    }
                  if (NULL == s)
                    fprintf (stderr, "Warning: couldn't find %s\n", *p);
                }
              dbuf_destroy (&crtpath);
            }

          /* Merge crtSet and relFilesSet */
          mergeSets (&crtSet, relFilesSet);
          relFilesSet = crtSet;
        }

      /* put in all object files */
      fputStrSet (lnkfile, relFilesSet);

      fprintf (lnkfile, "\n-e\n");
      fclose (lnkfile);
    }                           /* if(port->linker.needLinkerScript) */

  if (port->linker.cmd)
    {
      /* shell_escape file names */
      char *b3 = shell_escape (dbuf_c_str (&linkerScriptFileName));
      char *bfn = shell_escape (dbuf_c_str (&binFileName));

      buf = buildCmdLine (port->linker.cmd, b3, bfn, NULL, linkOptionsSet);
      Safe_free (b3);
      Safe_free (bfn);
    }
  else
    {
      buf = buildMacros (port->linker.mcmd);
    }

  dbuf_destroy (&linkerScriptFileName);

  tb = setPrefixSuffix (buf);
  if (options.verbose)
    printf ("sdcc: %s\n", tb);
  system_ret = sdcc_system (tb);

  /* if the binary file name is defined,
     rename the linker output file name to binary file name */
  if (fullDstFileName)
    {
      struct dbuf_s lkrFileName;

      dbuf_init (&lkrFileName, PATH_MAX);
      dbuf_append_str (&lkrFileName, dstFileName);
      dbuf_append_str (&lkrFileName, getOutFmtExt ());

      if (FILENAME_CMP (dbuf_c_str (&binFileName), dbuf_c_str (&lkrFileName)))
        remove (dbuf_c_str (&binFileName));
      rename (dbuf_c_str (&lkrFileName), dbuf_c_str (&binFileName));

      dbuf_destroy (&lkrFileName);
    }

  dbuf_destroy (&binFileName);

  if (system_ret)
    exit (EXIT_FAILURE);
}

/*-----------------------------------------------------------------*/
/* assemble - spawns the assembler with arguments                  */
/*-----------------------------------------------------------------*/
static void
assemble (char **envp)
{
  struct dbuf_s asmName;

  if (port->assembler.do_assemble)
    {
      port->assembler.do_assemble (asmOptionsSet);
    }
  else
    {
      char *buf, *tb;

      /* build assembler output filename */
      dbuf_init (&asmName, PATH_MAX);

      /* -o option overrides default name? */
      if (options.cc_only && fullDstFileName)
        {
          dbuf_append_str (&asmName, fullDstFileName);
        }
      else
        {
          /* the assembled file gets the name of the first module */
          dbuf_printf (&asmName, "%s%s", dstFileName, port->linker.rel_ext);
        }

      if (port->assembler.cmd)
        {
          /* shell_escape file names */
          char *dfn = shell_escape (dstFileName);
          char *asmn = shell_escape (dbuf_c_str (&asmName));

          buf = buildCmdLine (port->assembler.cmd, dfn, asmn,
                    options.debug ? port->assembler.debug_opts : port->assembler.plain_opts, asmOptionsSet);
          Safe_free (dfn);
          Safe_free (asmn);
        }
      else
        {
          buf = buildMacros (port->assembler.mcmd);
        }

      dbuf_destroy (&asmName);
      tb = setPrefixSuffix (buf);

      if (options.verbose)
        printf ("sdcc: %s\n", tb);
      if (sdcc_system (tb))
        {
          Safe_free (buf);
          /* either system() or the assembler itself has reported an error */
          exit (EXIT_FAILURE);
        }
      Safe_free (buf);

      if (options.cc_only && fullDstFileName && TARGET_PIC_LIKE)
        {
          /* gpasm assembler doesn't properly handle the -o option:
             the file extension is replaced with .o,
             so sdcc have to rename the object file manually.
             This has been fixed in gpasm svn:
             http://sourceforge.net/tracker/?func=detail&aid=3018645&group_id=41924&atid=431665
             TODO: This code should be removed when the next gputils version
             after gpasm-0.13.7 beta will be released */
          struct dbuf_s outName;
          dbuf_init (&outName, PATH_MAX);
          dbuf_printf (&outName, "%s%s", dstFileName, port->linker.rel_ext);

          if (strcmp (dbuf_c_str (&outName), fullDstFileName))
            {
              /* file names are different: rename the genatated object file */
              remove (fullDstFileName);
              rename (dbuf_c_str (&outName), fullDstFileName);
            }
          dbuf_destroy (&outName);
        }
    }
}

/*-----------------------------------------------------------------*/
/* preProcess - spawns the preprocessor with arguments             */
/*-----------------------------------------------------------------*/
static int
preProcess (char **envp)
{
  if (options.c1mode)
    {
      yyin = stdin;
    }
  else
    {
      const char *s;
      set *inclList = NULL;
      char *buf;

      if (NULL != port->linker.rel_ext)
        {
          struct dbuf_s dbuf;

          dbuf_init (&dbuf, 256);
          dbuf_printf (&dbuf, "-obj-ext=%s", port->linker.rel_ext);
          addSet (&preArgvSet, dbuf_detach_c_str (&dbuf));
        }

      /* if using dollar signs in identifiers */
      if (options.dollars_in_ident)
        addSet (&preArgvSet, Safe_strdup ("-fdollars-in-identifiers"));

      /* if using external stack define the macro */
      if (options.useXstack)
        addSet (&preArgvSet, Safe_strdup ("-D__SDCC_USE_XSTACK"));

      /* set the macro for stack autos  */
      if (options.stackAuto)
        addSet (&preArgvSet, Safe_strdup ("-D__SDCC_STACK_AUTO"));

      /* set the macro for stack autos  */
      if (options.stack10bit)
        addSet (&preArgvSet, Safe_strdup ("-D__SDCC_STACK_TENBIT"));

      /* set the macro for no overlay  */
      if (options.noOverlay)
        addSet (&preArgvSet, Safe_strdup ("-D__SDCC_NOOVERLAY"));

      /* set the macro for unsigned char  */
      if (options.unsigned_char)
        addSet (&preArgvSet, Safe_strdup ("-D__SDCC_CHAR_UNSIGNED"));

      /* set the macro for non-free  */
      if (options.use_non_free)
        addSet (&preArgvSet, Safe_strdup ("-D__SDCC_USE_NON_FREE"));

      /* set the macro for large model  */
      switch (options.model)
        {
        case MODEL_LARGE:
          addSet (&preArgvSet, Safe_strdup ("-D__SDCC_MODEL_LARGE"));
          break;

        case MODEL_SMALL:
          addSet (&preArgvSet, Safe_strdup ("-D__SDCC_MODEL_SMALL"));
          break;

        case MODEL_COMPACT:
          addSet (&preArgvSet, Safe_strdup ("-D__SDCC_MODEL_COMPACT"));
          break;

        case MODEL_MEDIUM:
          addSet (&preArgvSet, Safe_strdup ("-D__SDCC_MODEL_MEDIUM"));
          break;

        case MODEL_HUGE:
          addSet (&preArgvSet, Safe_strdup ("-D__SDCC_MODEL_HUGE"));
          break;

        case MODEL_FLAT24:
          addSet (&preArgvSet, Safe_strdup ("-D__SDCC_MODEL_FLAT24"));
          break;

        case NO_MODEL:
          break;

        default:
          werror (W_UNKNOWN_MODEL, __FILE__, __LINE__);
          break;
        }

      /* set macro corresponding to compiler option */
      if (options.intlong_rent)
        addSet (&preArgvSet, Safe_strdup ("-D__SDCC_INT_LONG_REENT"));

      /* set macro corresponding to compiler option */
      if (options.float_rent)
        addSet (&preArgvSet, Safe_strdup ("-D__SDCC_FLOAT_REENT"));

      if (options.all_callee_saves)
        addSet(&preArgvSet, Safe_strdup("-D__SDCC_ALL_CALLEE_SAVES"));

      /* add SDCC version number */
      {
        struct dbuf_s dbuf;

        dbuf_init (&dbuf, 20);
        dbuf_printf (&dbuf, "-D__SDCC=%d_%d_%d", SDCC_VERSION_HI, SDCC_VERSION_LO, SDCC_VERSION_P);
        addSet (&preArgvSet, dbuf_detach_c_str (&dbuf));
      }

      if (options.std_sdcc)
        {
          struct dbuf_s dbuf;

          dbuf_init (&dbuf, 20);
          dbuf_printf (&dbuf, "-DSDCC=%d%d%d", SDCC_VERSION_HI, SDCC_VERSION_LO, SDCC_VERSION_P);
          addSet (&preArgvSet, dbuf_detach_c_str (&dbuf));
        }

      /* add SDCC revision number */
      {
        struct dbuf_s dbuf;

        dbuf_init (&dbuf, 20);        
        dbuf_printf (&dbuf, "-D__SDCC_REVISION=%s", getBuildNumber ());
        addSet (&preArgvSet, dbuf_detach_c_str (&dbuf));
      }

      /* add port (processor information to processor */
      addSet (&preArgvSet, Safe_strdup ("-D__SDCC_{port}"));

      /* Optinal C features not (yet) supported by sdcc */
      addSet (&preArgvSet, Safe_strdup ("-D__STDC_NO_COMPLEX__"));
      addSet (&preArgvSet, Safe_strdup ("-D__STDC_NO_THREADS__"));
      addSet (&preArgvSet, Safe_strdup ("-D__STDC_NO_ATOMICS__"));
      addSet (&preArgvSet, Safe_strdup ("-D__STDC_NO_VLA__"));

      /* standard include path */
      if (!options.nostdinc)
        {
          inclList = processStrSet (includeDirsSet, "-isystem ", NULL, shell_escape);
          mergeSets (&preArgvSet, inclList);
        }

      setMainValue ("cppextraopts", (s = joinStrSet (preArgvSet)));
      Safe_free ((void *) s);
      if (inclList != NULL)
        deleteSet (&inclList);

      if (preProcOnly && fullDstFileName)
        {
          /* -E and -o given */
          char *s = shell_escape (fullDstFileName);

          setMainValue ("cppoutfilename", s);
          Safe_free (s);
        }
      else
        {
          /* Piping: set cppoutfilename to NULL, to avoid empty quotes */
          setMainValue ("cppoutfilename", NULL);
        }

      if (options.verbose)
        printf ("sdcc: Calling preprocessor...\n");
      buf = buildMacros (_preCmd);
      buf = setPrefixSuffix (buf);
      if (options.verbose)
        printf ("sdcc: %s\n", buf);

      if (preProcOnly)
        {
          if (sdcc_system (buf))
            {
              exit (EXIT_FAILURE);
            }

          exit (EXIT_SUCCESS);
        }

      yyin = sdcc_popen (buf);
      if (yyin == NULL)
        {
          perror ("Preproc file not found");
          exit (EXIT_FAILURE);
        }
    }

  return 0;
}

/* Set bin paths */
static void
setBinPaths (const char *argv0)
{
  const char *p;

  /*
   * Search logic:
   *
   * 1. - $SDCCDIR/PREFIX2BIN_DIR
   * 2. - path(argv[0])
   * 3. - $PATH
   */

  /* do it in reverse mode, so that addSetHead() can be used
     instead of slower addSet() */

  if ((p = getBinPath (argv0)) != NULL)
    addSetHead (&binPathSet, (void *) p);

  if ((p = getenv (SDCC_DIR_NAME)) != NULL)
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, PATH_MAX);
      dbuf_append_str (&dbuf, p);
      dbuf_append_str (&dbuf, PREFIX2BIN_DIR);
      addSetHead (&binPathSet, dbuf_detach_c_str (&dbuf));
    }
}

/* Set system include path */
static void
setIncludePath (void)
{
  /*
   * Search logic:
   *
   *  1. - $SDCC_INCLUDE/target
   *  2. - $SDCC_INCLUDE
   *  3. - $SDCC_HOME/PREFIX2DATA_DIR/INCLUDE_DIR_SUFFIX/target
   *  4. - path(argv[0])/BIN2DATA_DIR/INCLUDE_DIR_SUFFIX/target
   *  5. - DATADIR/INCLUDE_DIR_SUFFIX/target (only on *nix)
   *  6. - $SDCC_HOME/PREFIX2DATA_DIR/INCLUDE_DIR_SUFFIX
   *  7. - path(argv[0])/BIN2DATA_DIR/INCLUDE_DIR_SUFFIX
   *  8. - DATADIR/INCLUDE_DIR_SUFFIX (only on *nix)
   *  9. - $SDCC_HOME/PREFIX2DATA_DIR/NON_FREE_INCLUDE_DIR_SUFFIX/target
   * 10. - path(argv[0])/BIN2DATA_DIR/NON_FREE_INCLUDE_DIR_SUFFIX/target
   * 11. - DATADIR/NON_FREE_INCLUDE_DIR_SUFFIX/target (only on *nix)
   * 12. - $SDCC_HOME/PREFIX2DATA_DIR/NON_FREE_INCLUDE_DIR_SUFFIX
   * 13. - path(argv[0])/BIN2DATA_DIR/NON_FREE_INCLUDE_DIR_SUFFIX
   * 14. - DATADIR/NON_FREE_INCLUDE_DIR_SUFFIX (only on *nix)
   */

  if (!options.nostdinc)
    {
      char *p;
      set *tempSet;

      tempSet = processStrSet (dataDirsSet, NULL, INCLUDE_DIR_SUFFIX, NULL);
      includeDirsSet = processStrSet (tempSet, NULL, DIR_SEPARATOR_STRING, NULL);
      includeDirsSet = processStrSet (includeDirsSet, NULL, port->target, NULL);
      mergeSets (&includeDirsSet, tempSet);

      if (options.use_non_free)
        {
          set *tempSet1;

          tempSet = processStrSet (dataDirsSet, NULL, NON_FREE_INCLUDE_DIR_SUFFIX, NULL);
          tempSet1 = processStrSet (tempSet, NULL, DIR_SEPARATOR_STRING, NULL);
          tempSet1 = processStrSet (tempSet1, NULL, port->target, NULL);
          mergeSets (&tempSet1, tempSet);
          mergeSets (&includeDirsSet, tempSet1);
        }

      if ((p = getenv (SDCC_INCLUDE_NAME)) != NULL)
        {
          struct dbuf_s dbuf;

          dbuf_init (&dbuf, PATH_MAX);
          addSetHead (&includeDirsSet, p);
          dbuf_makePath (&dbuf, p, port->target);
          addSetHead (&includeDirsSet, dbuf_detach (&dbuf));
        }
    }
}

/* Set system lib path */
static void
setLibPath (void)
{
  /*
   * Search logic:
   *
   * 1. - $SDCC_LIB/<model>
   * 2. - $SDCC_LIB
   * 3. - $SDCC_HOME/PREFIX2DATA_DIR/LIB_DIR_SUFFIX/<model>
   * 4. - path(argv[0])/BIN2DATA_DIR/LIB_DIR_SUFFIX/<model>
   * 5. - DATADIR/LIB_DIR_SUFFIX/<model> (only on *nix)
   * 6. - $SDCC_HOME/PREFIX2DATA_DIR/NON_FREE_LIB_DIR_SUFFIX/<model>
   * 7. - path(argv[0])/BIN2DATA_DIR/NON_FREE_LIB_DIR_SUFFIX/<model>
   * 8. - DATADIR/NON_FREE_LIB_DIR_SUFFIX/<model> (only on *nix)
   */

  if (!options.nostdlib)
    {
      char *p;
      const char *targetname;

      struct dbuf_s dbuf;

      dbuf_init (&dbuf, PATH_MAX);

      targetname = port->target;

      dbuf_makePath (&dbuf, LIB_DIR_SUFFIX, port->general.get_model ? port->general.get_model () : targetname);
      libDirsSet = processStrSet (dataDirsSet, NULL, dbuf_c_str (&dbuf), NULL);

      if (options.use_non_free)
        {
          dbuf_set_length (&dbuf, 0);
          dbuf_makePath (&dbuf, NON_FREE_LIB_DIR_SUFFIX, port->general.get_model ? port->general.get_model () : targetname);
          mergeSets (&libDirsSet, processStrSet (dataDirsSet, NULL, dbuf_c_str (&dbuf), NULL));
        }

      if ((p = getenv (SDCC_LIB_NAME)) != NULL)
        {
          addSetHead (&libDirsSet, Safe_strdup (p));

          dbuf_set_length (&dbuf, 0);
          dbuf_makePath (&dbuf, p, port->general.get_model ? port->general.get_model () : targetname);
          addSetHead (&libDirsSet, dbuf_detach (&dbuf));
        }
      else
        dbuf_destroy (&dbuf);
    }
}

/* Set data path */
static void
setDataPaths (const char *argv0)
{
  const char *p;

  /*
   * Search logic:
   *
   * 1. - $SDCC_HOME/PREFIX2DATA_DIR
   * 2. - path(argv[0])/BIN2DATA_DIR
   * 3. - DATADIR (only on *nix)
   */

  if ((p = getenv (SDCC_DIR_NAME)) != NULL)
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, PATH_MAX);
      dbuf_append_str (&dbuf, p);
      dbuf_append_str (&dbuf, PREFIX2DATA_DIR);
      addSetHead (&dataDirsSet, dbuf_detach_c_str (&dbuf));
    }

  if ((p = getBinPath (argv0)) != NULL)
    {
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, PATH_MAX);
      dbuf_append_str (&dbuf, p);
      dbuf_append_str (&dbuf, BIN2DATA_DIR);
      addSetHead (&dataDirsSet, dbuf_detach_c_str (&dbuf));
    }

#ifdef _WIN32
  if (peekSet (dataDirsSet) == NULL)
    {
      /* this should never happen... */
      wassertl (0, "Can't get binary path");
    }
#else
  addSet (&dataDirsSet, Safe_strdup (DATADIR));
#endif

  setIncludePath ();
  setLibPath ();
}

static void
initValues (void)
{
  char *s;

  populateMainValues (_baseValues);
  setMainValue ("port", port->target);
  setMainValue ("objext", port->linker.rel_ext);
  setMainValue ("asmext", port->assembler.file_ext);

  setMainValue ("dstfilename", dstFileName);

  s = fullSrcFileName ? shell_escape (fullSrcFileName) : "fullsrcfilename";
  setMainValue ("fullsrcfilename", s);
  if (fullSrcFileName)
    Safe_free (s);

  if (options.cc_only && fullDstFileName)
    /* compile + assemble and -o given: -o specifies name of object file */
    {
      setMainValue ("objdstfilename", fullDstFileName);
    }
  else
    {
      setMainValue ("objdstfilename", "{stdobjdstfilename}");
    }
  if (fullDstFileName)
    /* if we're linking, -o gives the final file name */
    {
      setMainValue ("linkdstfilename", fullDstFileName);
    }
  else
    {
      setMainValue ("linkdstfilename", "{stdlinkdstfilename}");
    }

  /*
   * Make sure the preprocessor is called with the "-std" option
   * corresponding to the --std used to start sdcc
   */
  if (!options.std_sdcc)
    setMainValue ("cppstd", options.std_c11 ? "-std=c11 " : (options.std_c99 ? "-std=c99 " : "-std=c89 "));
}

static void
doPrintSearchDirs (void)
{
  const char *s;

  printf ("programs:\n");
  fputStrSet (stdout, binPathSet);

  printf ("datadir:\n");
  fputStrSet (stdout, dataDirsSet);

  printf ("includedir:\n");
  fputStrSet (stdout, includeDirsSet);

  printf ("libdir:\n");
  for (s = setFirstItem (libDirsSet); s != NULL; s = setNextItem (libDirsSet))
    fprintf (stdout, "%s\n", s);

  printf ("libpath:\n");
  fputStrSet (stdout, libPathsSet);
}


static void
sig_handler (int signal)
{
  char *sig_string;

  switch (signal)
    {
    case SIGABRT:
      sig_string = "SIGABRT";
      break;

    case SIGTERM:
      sig_string = "SIGTERM";
      break;

    case SIGINT:
      sig_string = "SIGINT";
      break;

    case SIGSEGV:
      sig_string = "SIGSEGV";
      break;

    default:
      sig_string = "Unknown?";
      break;
    }
  fprintf (stderr, "Caught signal %d: %s\n", signal, sig_string);
  exit (EXIT_FAILURE);
}

/*
 * main routine
 * initialises and calls the parser
 */

int
main (int argc, char **argv, char **envp)
{
  getPrefixSuffix (argv[0]);

  /* turn all optimizations off by default */
  memset (&optimize, 0, sizeof (struct optimize));

  if (NUM_PORTS == 0)
    {
      fprintf (stderr, "Build error: no ports are enabled.\n");
      exit (EXIT_FAILURE);
    }

  /* install signal handler;
     it's only purpose is to call exit() to remove temp files */
  if (!getenv ("SDCC_LEAVE_SIGNALS"))
    {
      signal (SIGABRT, sig_handler);
      signal (SIGTERM, sig_handler);
      signal (SIGINT, sig_handler);
      signal (SIGSEGV, sig_handler);
    }

  /* Before parsing the command line options, do a
   * search for the port and processor and initialize
   * them if they're found. (We can't gurantee that these
   * will be the first options specified).
   */

  _findPort (argc, argv);

#ifdef JAMIN_DS390
  if (strcmp (port->target, "mcs51") == 0)
    {
      printf ("DS390 jammed in A\n");
      _setPort ("ds390");
      ds390_jammed = 1;
    }
#endif

  _findProcessor (argc, argv);

  /* Initalise the port. */
  if (port->init)
    port->init ();

  setDefaultOptions ();
#ifdef JAMIN_DS390
  if (ds390_jammed)
    {
      options.model = MODEL_SMALL;
      options.stack10bit = 0;
    }
#endif

  parseCmdLine (argc, argv);

  if (options.verbose && NULL != port->processor)
    printf ("Processor: %s\n", port->processor);

  initValues ();

  setBinPaths (argv[0]);
  setDataPaths (argv[0]);

  if (port->initPaths)
    port->initPaths ();

  if (options.printSearchDirs)
    doPrintSearchDirs ();

  /* if no input then printUsage & exit */
  if (!options.c1mode && !fullSrcFileName && peekSet (relFilesSet) == NULL)
    {
      if (options.printSearchDirs)
        exit (EXIT_SUCCESS);
      printUsage ();
      exit (EXIT_FAILURE);
    }

  /* initMem() is expensive, but
     initMem() must called before port->finaliseOptions ().
     And the z80 port needs port->finaliseOptions(),
     even if we're only linking. */
  initMem ();

  /* finalize target specific options */
  port->finaliseOptions ();

  /* finalize common options */
  finalizeOptions ();

  if (fullSrcFileName || options.c1mode)
    {
      preProcess (envp);

      initSymt ();
      initiCode ();
      initCSupport ();
      initBuiltIns ();
      initPeepHole ();

      if (options.verbose)
        printf ("sdcc: Generating code...\n");

      yyparse ();

      if (!options.c1mode)
        if (sdcc_pclose (yyin))
          fatalError = 1;

      if (fatalError)
        exit (EXIT_FAILURE);

      if (port->general.do_glue != NULL)
        (*port->general.do_glue) ();
      else
        {
          /* this shouldn't happen */
          assert (FALSE);
          /* in case of NDEBUG */
          glue ();
        }

      if (fatalError)
        exit (EXIT_FAILURE);

      if (!options.c1mode && !noAssemble)
        {
          if (options.verbose)
            printf ("sdcc: Calling assembler...\n");
          assemble (envp);
        }
    }
  closeDumpFiles ();

  if (options.debug && debugFile)
    debugFile->closeFile ();

  if (!options.cc_only && !fatalError && !noAssemble && !options.c1mode && (fullSrcFileName || peekSet (relFilesSet) != NULL))
    {
      if (options.verbose)
        printf ("sdcc: Calling linker...\n");

      if (port->linker.do_link)
        port->linker.do_link ();
      else
        linkEdit (envp);
    }

  return 0;
}
