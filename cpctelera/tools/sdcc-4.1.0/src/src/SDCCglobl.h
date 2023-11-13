/*-------------------------------------------------------------------------
  SDCCglobl.h - global macros etc required by all files

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net

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

#ifndef SDCCGLOBL_H
#define SDCCGLOBL_H

#include <memory.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <errno.h>

# ifndef __cplusplus
#  include <stdbool.h>
# endif

# ifndef TRUE
#   define TRUE     true
# endif
# ifndef FALSE
#   define FALSE    false
# endif

#include "SDCCset.h"


/*
 * Define host port dependant constants etc.
 */

#define UNIX_DIR_SEPARATOR_CHAR    '/'

#if defined(__BORLANDC__) || defined(_MSC_VER)
# define STRCASECMP   stricmp
# define STRNCASECMP  strnicmp
#else
# define STRCASECMP   strcasecmp
# define STRNCASECMP  strncasecmp
#endif

#if defined(__MSDOS__) || defined(_WIN32) || defined(__OS2__) || defined (__CYGWIN__)

# ifndef HAVE_DOS_BASED_FILE_SYSTEM
#   define HAVE_DOS_BASED_FILE_SYSTEM 1
# endif

# define IS_DIR_SEPARATOR(c)    ((c) == DIR_SEPARATOR_CHAR || (c) == UNIX_DIR_SEPARATOR_CHAR)
/* Note that IS_ABSOLUTE_PATH accepts d:foo as well, although it is
   only semi-absolute.  This is because the users of IS_ABSOLUTE_PATH
   want to know whether to prepend the current working directory to
   a file name, which should not be done with a name like d:foo.  */
# define IS_ABSOLUTE_PATH(f)    (IS_DIR_SEPARATOR((f)[0]) || (((f)[0]) && ((f)[1] == ':')))
# define FILENAME_CMP(s1, s2)   STRCASECMP(s1, s2)

#else  /* not DOSish */

# define IS_DIR_SEPARATOR(c)    ((c) == DIR_SEPARATOR_CHAR)
# define IS_ABSOLUTE_PATH(f)    (IS_DIR_SEPARATOR((f)[0]))
# define FILENAME_CMP(s1, s2)   strcmp(s1, s2)

#endif /* not DOSish */

#ifdef WIN32
# define NATIVE_WIN32           1
# ifndef __MINGW32__
#   define  PATH_MAX  _MAX_PATH
# endif
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#elif defined(_WIN32) && !defined(__MINGW32__)
# include "sdcc_vc.h"
#else
# include "sdccconf.h"
#endif

#include "SDCCerr.h"

#define SPACE ' '

#include <limits.h>             /* PATH_MAX                  */
#if !defined(PATH_MAX) || (PATH_MAX < 2048)
# undef PATH_MAX
# define PATH_MAX     2048      /* define a reasonable value */
#endif

#define MAX_REG_PARMS 1

/* C++ doesn't like min and max macros */
#ifndef __cplusplus
# ifndef max
#   define max(a,b)   (a > b ? a : b)
# endif
# ifndef min
#   define min(a,b)   (a < b ? a : b)
# endif
#endif /* __cplusplus */

#ifndef THROWS
# define THROWS
# define THROW_NONE  0
# define THROW_SRC   1
# define THROW_DEST  2
# define THROW_BOTH  3
#endif

/* sizes in bytes  */
#define BOOLSIZE      port->s.char_size
#define CHARSIZE      port->s.char_size
#define SHORTSIZE     port->s.short_size
#define INTSIZE       port->s.int_size
#define LONGSIZE      port->s.long_size
#define LONGLONGSIZE  port->s.longlong_size
#define NEARPTRSIZE   port->s.near_ptr_size
#define FARPTRSIZE    port->s.far_ptr_size
#define GPTRSIZE      port->s.ptr_size
#define FUNCPTRSIZE   port->s.funcptr_size
#define BFUNCPTRSIZE  port->s.banked_funcptr_size
#define BITSIZE       port->s.bit_size
#define FLOATSIZE     port->s.float_size

#define INITIAL_INLINEASM (4 * 1024)
#define DEFPOOLSTACK(type,size)     \
    type       *type##Pool        ; \
    type *type##FreeStack [size]  ; \
    int   type##StackPtr = 0      ;

#define PUSH(x,y)   x##FreeStack[x##StackPtr++] = y
#define PEEK(x)     x##FreeStack[x##StackPtr - 1]
#define POP(type)   type##FreeStack[--type##StackPtr]
/* #define POP(x)    (x##StackPtr ? x##FreeStack[--x##StackPtr] :       \
   (assert(x##StackPtr),0)) */
#ifdef UNIX
#define EMPTY(x)        (x##StackPtr <= 1 ? 1 : 0)
#else
#define EMPTY(x)        (x##StackPtr == 0 ? 1 : 0)
#endif


#define COPYTYPE(start,end,from)  (end = getSpec (start = from))


/* general purpose stack related macros */
#define STACK_DCL(stack, type, size)                                \
        typedef type t_##stack;                                     \
        t_##stack stack[size];                                      \
        t_##stack (*p_##stack) = stack - 1;

#define STACK_EMPTY(stack)     ((p_##stack) < stack)
#define STACK_FULL(stack)      ((p_##stack) >= (stack +             \
                                sizeof(stack) / sizeof(*stack) - 1) )

#define STACK_PUSH_(stack, x)  (*++p_##stack = (x))
#define STACK_POP_(stack)      (*p_##stack--)

#define STACK_PUSH(stack, x)   (STACK_FULL(stack)                   \
                               ? (STACK_ERR(1, stack), *p_##stack)  \
                               : STACK_PUSH_(stack,x)               )

#define STACK_POP(stack)       (STACK_EMPTY(stack)                  \
                               ? (STACK_ERR(-1, stack), *stack)     \
                               : STACK_POP_(stack)                  )

#define STACK_PEEK(stack)      (STACK_EMPTY(stack)                  \
                               ? (STACK_ERR(0, stack), *stack)      \
                               : *p_##stack                         )

#define STACK_ERR(o, stack)    (fatal(1, E_STACK_VIOLATION, #stack, \
                                       (o < 0)                      \
                                       ? "underflow"                \
                                       : (o > 0)                    \
                                         ? "overflow"               \
                                         : "empty"))

/* for semantically partitioned nest level values */
#define LEVEL_UNIT      65536
#define SUBLEVEL_UNIT   1

/* optimization options */
struct optimize
  {
    int global_cse;
    int ptrArithmetic;
    int label1;
    int label2;
    int label3;
    int label4;
    int loopInvariant;
    int loopInduction;
    int noLoopReverse;
    int codeSpeed;
    int codeSize;
    int lospre;
    int allow_unsafe_read;
    int noStdLibCall;
  };

/** Build model.
    Used in options.model.A bit each as port.supported_models is an OR
    of these.
*/
enum
  {
    NO_MODEL = 0,     /* no model applicable */
    MODEL_SMALL = 1,
    MODEL_COMPACT = 2,
    MODEL_MEDIUM = 4,
    MODEL_LARGE = 8,
    MODEL_FLAT24 = 16,
//  MODEL_PAGE0 = 32, /* disabled, was for the xa51 port */
    MODEL_HUGE = 64   /* for banked support */
  };

/* overlay segment name and the functions
   that belong to it. used by pragma overlay */
typedef struct {
    char *osname;       /* overlay segment name */
    int  nfuncs;        /* number of functions in this overlay */
    char *funcs[128];   /* function name that belong to this */
} olay;

enum
  {
    NO_DEPENDENCY_FILE_OPT = 0,
    SYSTEM_DEPENDENCY_FILE_OPT = 1,
    USER_DEPENDENCY_FILE_OPT = 2
  };

/* other command line options */
/*
 * cloneOptions function in SDCC.lex should be updated every time
 * a new set is added to the options structure!
 */
struct options
  {
    int model;                  /* see MODEL_* defines above */
    int stackAuto;              /* Stack Automatic  */
    int useXstack;              /* use Xternal Stack */
    int stack10bit;             /* use 10 bit stack (flat24 model only) */
    int dump_ast;               /* dump front-end tree before lowering to iCode */
    int dump_i_code;            /* dump iCode at various stages */
    int dump_graphs;            /* Dump graphs in .dot format (control-flow, conflict, etc) */
    int cc_only;                /* compile only flag              */
    int intlong_rent;           /* integer & long support routines reentrant */
    int float_rent;             /* floating point routines are reentrant */
    int out_fmt;                /* 0 = undefined, 'i' = intel Hex format, 's' = motorola S19 format, 'E' = elf format, 'Z' = gb format */
    int cyclomatic;             /* print cyclomatic information */
    int noOverlay;              /* don't overlay local variables & parameters */
    int xram_movc;              /* use movc instead of movx to read xram (mcs51) */
    int nopeep;                 /* no peep hole optimization */
    int asmpeep;                /* pass inline assembler thru peep hole */
    int peepReturn;             /* enable peephole optimization for return instructions */
    int debug;                  /* generate extra debug info */
    int c1mode;                 /* Act like c1 - no pre-proc, asm or link */
    char *peep_file;            /* additional rules for peep hole */
    int nostdlib;               /* Don't use standard lib files */
    int nostdinc;               /* Don't use standard include files */
    int noRegParams;            /* Disable passing some parameters in registers */
    int verbose;                /* Show what the compiler is doing */
    int lessPedantic;           /* disable some warnings */
    int profile;                /* Turn on extra profiling information */
    int omitFramePtr;           /* Turn off the frame pointer. */
    int useAccelerator;         /* use ds390 Arithmetic Accelerator */
    int noiv;                   /* do not generate irq vector table entries */
    int all_callee_saves;       /* callee saves for all functions */
    int stack_probe;            /* insert call to function __stack_probe */
    int tini_libid;             /* library ID for TINI */
    int protect_sp_update;      /* DS390 - will disable interrupts during ESP:SP updates */
    int parms_in_bank1;         /* DS390 - use reg bank1 to pass parameters */
    int stack_size;             /* MCS51/DS390 - Tells the linker to allocate this space for stack */
    int acall_ajmp;             /* MCS51 - Use acall/ajmp instead of lcall/ljmp */
    int no_ret_without_call;    /* MCS51 - Do not use ret independent of acall/lcall */
    int use_non_free;           /* Search / include non-free licensed libraries and header files */
    /* starting address of the segments */
    int xstack_loc;             /* initial location of external stack */
    int stack_loc;              /* initial value of internal stack pointer */
    int xdata_loc;              /* xternal ram starts at address */
    int data_loc;               /* interram start location       */
    int idata_loc;              /* indirect address space        */
    int code_loc;               /* code location start           */
    int iram_size;              /* internal ram size (used only for error checking) */
    int xram_size;              /* external ram size (used only for error checking) */
    bool xram_size_set;         /* since xram_size=0 is a possibility */
    int code_size;              /* code size (used only for error checking) */
    int verboseExec;            /* show what we are doing */
    int noXinitOpt;             /* don't optimize initialized xdata */
    int noCcodeInAsm;           /* hide c-code from asm */
    int iCodeInAsm;             /* show i-code in asm */
    int noPeepComments;         /* hide peephole optimizer comments */
    int verboseAsm;             /* include comments generated with gen.c */
    int printSearchDirs;        /* display the directories in the compiler's search path */
    int vc_err_style;           /* errors and warnings are compatible with Micro$oft visual studio */
    int use_stdout;             /* send errors to stdout instead of stderr */
    int no_std_crt0;            /* for the z80/gbz80 do not link default crt0.o*/
    int std_c95;                /* enable C95 keywords/constructs */
    int std_c99;                /* enable C99 keywords/constructs */
    int std_c11;                /* enable C11 keywords/constructs */
    int std_c2x;                /* enable C2X keywords/constructs */
    int std_sdcc;               /* enable SDCC extensions to C */
    int dollars_in_ident;       /* zero means dollar signs are punctuation */
    int signed_char;            /* use signed for char without signed/unsigned modifier */
    char *code_seg;             /* segment name to use instead of CSEG */
    char *const_seg;            /* segment name to use instead of CONST */
    char *data_seg;             /* segment name to use instead of DATA */
    int dependencyFileOpt;      /* write dependencies to given file */
    /* sets */
    set *calleeSavesSet;        /* list of functions using callee save */
    set *excludeRegsSet;        /* registers excluded from saving */
/*  set *olaysSet;               * not implemented yet: overlay segments used in #pragma OVERLAY */
    int max_allocs_per_node;    /* Maximum number of allocations / combinations considered at each node in the tree-decomposition based algorithms */
    bool noOptsdccInAsm;        /* Do not emit .optsdcc in asm */
    bool oldralloc;             /* Use old register allocator */
  };

/* forward definition for variables accessed globally */
extern int noAssemble;          /* no assembly, stop after code generation */
extern char *yytext;
extern char *lexFilename;       /* lex idea of current file name */
extern int lexLineno;           /* lex idea of line number of the current file */
extern const char *fullSrcFileName; /* full name for the source file; */
                                /* can be NULL while linking without compiling */
extern const char *fullDstFileName; /* full name for the output file; */
                                /* only given by -o, otherwise NULL */
extern const char *dstFileName; /* destination file name without extension */
extern const char *moduleName;  /* module name is source file without path and extension */
                                /* can be NULL while linking without compiling */
extern int seqPointNo;          /* current sequence point */
extern FILE *yyin;              /* */
extern FILE *asmFile;           /* assembly output file */
extern FILE *cdbFile;           /* debugger symbol file */
extern long NestLevel;          /* NestLevel                 SDCC.y */
extern int stackPtr;            /* stack pointer             SDCC.y */
extern int xstackPtr;           /* external stack pointer    SDCC.y */
extern int reentrant;           /* /X flag has been sent     SDCC.y */
extern char buffer[PATH_MAX * 2];/* general buffer           SDCCmain.c */
extern int currRegBank;         /* register bank being used  SDCCgens.c */
extern int RegBankUsed[4];      /* JCF: register banks used  SDCCmain.c */
extern int BitBankUsed;         /* MB: overlayable bit bank  SDCCmain.c */
extern struct symbol *currFunc; /* current function    SDCCgens.c */
extern long cNestLevel;         /* block nest level  SDCCval.c */
extern int blockNo;             /* maximum sequential block number */
extern int currBlockno;         /* sequential block number */
extern struct optimize optimize;
extern struct options options;
extern unsigned maxInterrupts;
extern int ignoreTypedefType;

/* Visible from SDCCmain.c */
extern set *preArgvSet;
extern set *relFilesSet;
extern set *libFilesSet;
extern set *libPathsSet;
extern set *libDirsSet;         /* list of lib search directories */

void setParseWithComma (set **, const char *);

/** An assert() macro that will go out through sdcc's error
    system.
*/
#define wassertl(a,s)   (void)((a) ? 0 : \
        (werror (E_INTERNAL_ERROR, __FILE__, __LINE__, s), 0))

#define wassert(a)    wassertl(a,"code generator internal error")

enum {
  DUMP_RAW0 = 1,
  DUMP_RAW1,
  DUMP_CSE,
  DUMP_DFLOW,
  DUMP_GCSE,
  DUMP_DEADCODE,
  DUMP_LOOP,
  DUMP_LOOPG,
  DUMP_LOOPD,
  DUMP_RANGE,
  DUMP_PACK,
  DUMP_RASSGN,
  DUMP_LRANGE,
  DUMP_LOSPRE,
  DUMP_CUSTOM /* For temporary dump points */
};

struct _dumpFiles {
  int id;
  char *ext;
  FILE *filePtr;
};

extern struct _dumpFiles dumpFiles[];

/* Define well known filenos if the system does not define them.  */
#ifndef STDIN_FILENO
# define STDIN_FILENO   0
#endif
#ifndef STDOUT_FILENO
# define STDOUT_FILENO  1
#endif
#ifndef STDERR_FILENO
# define STDERR_FILENO  2
#endif

#endif
