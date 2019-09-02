/*-------------------------------------------------------------------------
  sdcdb.h - Header file used by ALL source files for the debugger
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

#ifndef  SDCDB_H
#define  SDCDB_H

#define SDCDB_VERSION	"0.9"

#define SDCDB_DEBUG

#ifdef SDCDB_DEBUG
// set D_x to 0 to turn off, 1 to turn on.
#define D_break  0x01
#define D_simi   0x02
#define D_sdcdb  0x04
#define D_symtab 0x08

extern int sdcdbDebug;

#define Dprintf(f, fs) {if (f & sdcdbDebug) printf fs ; }
#else
#define Dprintf(f, fs) { }
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#elif defined(_WIN32) && !defined(__MINGW32__)
# include "sdcc_vc.h"
#else
# include "sdccconf.h"
#endif
#include "src/SDCCset.h"
#include "src/SDCChasht.h"

#define TRUE 1
#define FALSE !TRUE

typedef short bool;

#ifndef max
#define max(a,b) (a > b ? a : b)
#endif

#ifndef min
#define min(a,b) (a < b ? a : b)
#endif

/*
 * #ifndef ALLOC
 * #define  ALLOC(x,sz) if (!(x = calloc(1, sz)))                       \
 *          {                                                           \
 *             fprintf(stderr,"sdcdb: out of memory\n");                \
 *             exit (1);                                                \
 *          }
 * #endif
 * #ifndef ALLOC_ATOMIC
 * #define ALLOC_ATOMIC(x,sz)   if (!(x = calloc(1, sz)))   \
 *          {                                               \
 *             fprintf(stderr,"sdcdb: out of memory\n");    \
 *             exit (1);                                    \
 *          }
 * #endif
 */

/* generalpurpose stack related macros */
#define  STACK_DCL(stack, type, size)                                      \
         typedef type t_##stack;                                           \
         t_##stack stack[size];                                            \
         t_##stack *p_##stack = stack - 1;                               \
         t_##stack *w_##stack;

/* define extern stack */
#define EXTERN_STACK_DCL(stack, type, size)                                \
        typedef type t_##stack;                                            \
        extern t_##stack stack[size];                                      \
        extern t_##stack *p_##stack;                                       \
        extern t_##stack *w_##stack;

#define STACK_EMPTY(stack)     ((p_##stack) < stack)
#define STACK_FULL(stack)      ((p_##stack) >= (stack +                   \
                               sizeof(stack) / sizeof(*stack) - 1)        )

#define STACK_PUSH_(stack, x)  (*++p_##stack = (x))
#define STACK_POP_(stack)      (*p_##stack--)

#define STACK_PUSH(stack, x)   (STACK_FULL(stack)                         \
                               ? (STACK_ERR(1, stack), *p_##stack)        \
                               : STACK_PUSH_(stack, x)                    )

#define STACK_POP(stack)       (STACK_EMPTY(stack)                        \
                               ? (STACK_ERR(-1, stack), *stack)           \
                               : STACK_POP_(stack)                        )

#define STACK_PEEK(stack)      (STACK_EMPTY(stack)                        \
                               ? (STACK_ERR(0, stack), *stack)            \
                               : *p_##stack                               )

#define STACK_PPEEK(stack)     (((p_##stack - 1) < stack)                 \
                               ? (STACK_ERR(0, stack), *stack)            \
                               : *(p_##stack - 1)                         )

#define STACK_ERR(o, stack)    (fprintf(stderr, "stack '%s' %s\n",        \
                                        #stack,                           \
                                        (o < 0)                           \
                                        ? "underflow"                     \
                                        : (o > 0)                         \
                                          ? "overflow"                    \
                                          : "empty")                      )

#define STACK_STARTWALK(stack) (w_##stack = p_##stack)

#define STACK_WALK(stack)      (w_##stack >= stack                        \
                               ? NULL : STACK_POP_(stack)                 )

#include "src/SDCCbitv.h"

enum {
    SYM_REC = 1,
    LNK_REC,
    FUNC_REC,
    STRUCT_REC,
    MOD_REC
};

enum {
    FMT_NON = 0,
    FMT_BIN = 1,
    FMT_OCT = 2,
    FMT_DEZ = 3,
    FMT_HEX = 4
};

enum { SRC_CMODE = 1, SRC_AMODE };

/*-----------------------------------------------------------------*/
/*                         source line structure                   */
/*-----------------------------------------------------------------*/
typedef struct srcLine
{
    unsigned addr     ;
    short block, level; /* scope information */
    char     *src ;

} srcLine ;

/*-----------------------------------------------------------------*/
/*                     structure for cdb record                    */
/*-----------------------------------------------------------------*/
typedef struct  cdbrecs {
    char type ;               /* type of line */
    char *line;               /* contents of line */
    struct cdbrecs *next;     /* next in chain */
} cdbrecs ;

/*-----------------------------------------------------------------*/
/*                     module definition                           */
/*-----------------------------------------------------------------*/
typedef struct module {
    char *cfullname ;        /* full name Includeing path for the module */
    char *afullname;         /* fullname of assembly file */
    char *name ;             /* name of module */
    char *c_name;            /* c filename     */
    char *asm_name;          /* asm file name  */
    int   ncLines;           /* number of lines in this module */
    int   nasmLines;         /* # of lines in the assembler file */
    srcLine  **cLines;       /* actual source lines */
    srcLine  **asmLines;     /* actual assembler source lines*/
    set       *cfpoints;     /* set of double line execution points */
} module;

/*-----------------------------------------------------------------*/
/*            execution point definition                           */
/*-----------------------------------------------------------------*/
typedef struct exePoint
{
    unsigned addr  ;
    int      line  ;
    short    block , level ;
} exePoint ;

/*-----------------------------------------------------------------*/
/*                   definition for a function                     */
/*-----------------------------------------------------------------*/
typedef struct function {
    struct symbol  *sym     ;/* pointer to symbol for function */
    char           *modName ;/* module name */
    module         *mod     ;/* module for this function */
    int        entryline    ;/* first line in the function */
    int        aentryline   ;
    int        exitline     ;/* last line in the function  */
    int        aexitline    ;
    set       *cfpoints     ;/* set of all C execution points in func   */
    set       *afpoints     ;/* set of all ASM execution points in func */
    unsigned   int laddr    ;/* last executed address                   */
    int        lline        ;/* last executed linenumber                */
    unsigned   int stkaddr  ;/* stackpointer at beginning of function
                              * (not reentrant ! ) only actual */
} function ;

/*-----------------------------------------------------------------*/
/*                  link record defintion                          */
/*-----------------------------------------------------------------*/
typedef struct linkrec {
    char type;        /* type of linker rec */
    unsigned addr ;   /* address specified by the linker rec */
    char *name    ;   /* name specified by linker rec */
} linkrec;

/*-----------------------------------------------------------------*/
/*                       program context                           */
/*-----------------------------------------------------------------*/
typedef struct context {
    function *func;           /* current function we are in */
    char     *modName;        /* name of the module         */
    unsigned int addr ;       /* current pc                 */
    int      cline ;          /* current c line number      */
    int      asmline;         /* current asm line number    */
    int      block ;          /* current block number       */
    int      level ;          /* current level number       */
} context ;

/*-----------------------------------------------------------------*/
/*                     symbol display information                  */
/*-----------------------------------------------------------------*/
typedef struct _dsymbol
{
    char *name;
    int  dnum;
    int  fmt;
    char *rs;
} dsymbol;


extern cdbrecs *recsRoot ;
extern context *currCtxt ;
extern set *modules  ; /* set of modules   */
extern set *functions; /* set of functions */
extern set *symbols  ; /* set of symbols */
extern set *sfrsymbols;/* set of symbols of sfr or sbit */
extern set *dispsymbols; /* set of displayable symbols */


extern char *currModName ;
extern char userinterrupt ;
extern char nointerrupt ;
extern short showfull ;
extern int nStructs ;
extern struct structdef **structs ; /* all structures */
extern char *ssdirl; /* source directory search path */
void **resize (void **, int );
char  *alloccpy(char *,int );
char *gc_strdup(const char *s);
srcLine **loadFile (char *name, int *nlines);

extern short fullname;
extern int srcMode;
extern char contsim;
char *searchDirsFname (char *);
char *getNextCmdLine(void);
void setCmdLine(char *);
void stopCommandList(void);

char *argsToCmdLine(char **args, int nargs);

/* trimming functions */
extern char *trim_left(char *s);
extern char *trim_right(char *s);
extern char *trim(char *s);

#endif
