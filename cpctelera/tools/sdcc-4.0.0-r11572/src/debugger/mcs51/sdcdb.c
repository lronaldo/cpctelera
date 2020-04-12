/*-------------------------------------------------------------------------
  sdcdb.c - main source file for sdcdb debugger
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

#include "sdcdb.h"

char *ssdirl = DATADIR LIB_DIR_SUFFIX ":" DATADIR LIB_DIR_SUFFIX DIR_SEPARATOR_STRING "small" ;

#undef DATADIR
#include "symtab.h"
#include "simi.h"
#include "break.h"
#include "cmd.h"
#include "newalloc.h"
#if defined HAVE_LIBREADLINE && HAVE_LIBREADLINE != -1
#define HAVE_READLINE_COMPLETITION  1
#endif
#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif  /* HAVE_LIBREADLINE */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif defined _WIN32
#include <direct.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef SDCDB_DEBUG
int   sdcdbDebug = 0;
#endif

char *currModName = NULL;
cdbrecs *recsRoot = NULL;
set  *modules = NULL;    /* set of all modules */
set  *functions = NULL;  /* set of functions */
set  *symbols = NULL;    /* set of symbols */
set  *sfrsymbols = NULL; /* set of symbols of sfr or sbit */
int nStructs = 0 ;
structdef **structs = NULL; /* all structures */
int nLinkrecs = 0;
linkrec **linkrecs = NULL; /* all linkage editor records */
context *currCtxt = NULL;
short fullname = 0;
short showfull = 0;
char userinterrupt = 0;
char nointerrupt = 0;
char contsim = 0;
char *simArgs[40];
int nsimArgs = 0;
char model_str[20];
/* fake filename & lineno to make linker */
char *filename = NULL;
int lineno = 0;
int fatalError = 0;

static void commandLoop(FILE *cmdfile);
#ifdef HAVE_READLINE_COMPLETITION
char *completionCmdSource(const char *text, int state);
char *completionCmdFile(const char *text, int state);
char *completionCmdInfo(const char *text, int state);
char *completionCmdShow(const char *text, int state);
char *completionCmdListSymbols(const char *text, int state);
char *completionCmdPrintType(const char *text, int state);
char *completionCmdPrint(const char *text, int state);
char *completionCmdDelUserBp(const char *text, int state);
char *completionCmdUnDisplay(const char *text, int state);
char *completionCmdSetUserBp(const char *text, int state);
char *completionCmdSetOption(const char *text, int state);
#else
#define completionCmdSource NULL
#define completionCmdFile NULL
#define completionCmdInfo NULL
#define completionCmdShow NULL
#define completionCmdListSymbols NULL
#define completionCmdPrintType NULL
#define completionCmdPrint NULL
#define completionCmdDelUserBp NULL
#define completionCmdUnDisplay NULL
#define completionCmdSetUserBp NULL
#define completionCmdSetOption NULL
#endif /* HAVE_READLINE_COMPLETITION */

/* command table */
struct cmdtab
{
    const char *cmd;     /* command the user will enter */
    int (*cmdfunc)(char *,context *); /* function to execute when command is entered */
#ifdef HAVE_READLINE_COMPLETITION
    rl_compentry_func_t *completion_func;
#else
    void *dummy;
#endif  /* HAVE_READLINE_COMPLETITION */
    const char *htxt;    /* short help text */
} cmdTab[] = {
    /* NOTE:- the search is done from the top, so "break" should
       precede the synonym "b" */
    /* break point */
    { "break"    ,  cmdSetUserBp  , completionCmdSetUserBp,
      "{b}reak\t[LINE | FILE:LINE | FILE:FUNCTION | FUNCTION | *<address>]"
    },
    { "tbreak"   ,  cmdSetTmpUserBp , completionCmdSetUserBp/*same as "break"*/,
      "tbreak\t[LINE | FILE:LINE | FILE:FUNCTION | FUNCTION | *<address>]"
    },
    { "b"        ,  cmdSetUserBp  , completionCmdSetUserBp ,  NULL,},

    { "jump"   ,  cmdJump , NULL,
      "jump\tContinue program being debugged at specified line or address\n"
      "\t[LINE | FILE:LINE | *<address>]",
    },
    { "clear"    ,  cmdClrUserBp  , completionCmdSetUserBp/*same as "break"*/,
      "{cl}ear\t[LINE | FILE:LINE | FILE:FUNCTION | FUNCTION]"
    },
    { "cl"       ,  cmdClrUserBp  , completionCmdSetUserBp/*same as "break"*/ ,
      NULL
    },
    { "continue" ,  cmdContinue   ,  NULL,
      "{c}ontinue\tContinue program being debugged, after breakpoint."
    },
    { "condition" ,  cmdCondition   ,  completionCmdDelUserBp/*same as "delete"*/,
      "condition brkpoint_number expr\tSet condition for breakpoint."
    },
    { "ignore" ,  cmdIgnore  ,  completionCmdDelUserBp/*same as "delete"*/,
      "ignore brkpoint_number count\tSet ignore count for breakpoint."
    },
    { "commands" ,  cmdCommands  ,  completionCmdDelUserBp/*same as "delete"*/,
      "commands [brkpoint_number]\tSetting commands for breakpoint."
    },
    { "c"        ,  cmdContinue   , NULL ,
      NULL
    },
    { "disassemble",cmdDisasmF    ,  NULL,
      "disassemble [startaddr [endaddress]]\tdisassemble asm commands"
    },
    { "delete" ,  cmdDelUserBp  , completionCmdDelUserBp,
      "{d}elete n\tclears break point number n"
    },
    { "display"    ,  cmdDisplay     , completionCmdPrint/*same as "print"*/,
      "display [/<fmt>] [<variable>]\tprint value of given variable each time the program stops"
    },
    { "undisplay"  ,  cmdUnDisplay   , completionCmdUnDisplay,
      "undisplay [<variable>]\tdon't display this variable or all"
    },
    { "down"     ,  cmdDown      , NULL,
      "down\tSelect and print stack frame called by this one.\n"
      "\tAn argument says how many frames down to go."
    },
    {
      "up"       ,  cmdUp      , NULL,
      "up\tSelect and print stack frame that called this one.\n"
      "\tAn argument says how many frames up to go."
    },
    { "d"        ,  cmdDelUserBp  , completionCmdDelUserBp,
      NULL
    },
    { "info"     ,  cmdInfo       , completionCmdInfo,
      "info <break stack frame registers all-registers line source functions symbols variables>\t"
      "list all break points, call-stack, frame or register information"
    },
    { "listasm"  ,  cmdListAsm    , NULL,
      "listasm {la}\tlist assembler code for the current C line"
    },
    { "la"       ,  cmdListAsm    , NULL,
      NULL
    },
    { "ls"       ,  cmdListSymbols  , completionCmdListSymbols,
      "ls,lf,lm\tlist symbols,functions,modules"
    },
    { "lf"       ,  cmdListFunctions, completionCmdListSymbols,
      NULL
    },
    { "lm"       ,  cmdListModules  , completionCmdListSymbols,
      NULL
    },
    { "list"     ,  cmdListSrc    , completionCmdSetUserBp/*same as "break"*/,
      "{l}ist\t[LINE | FILE:LINE | FILE:FUNCTION | FUNCTION]"
    },
    { "l"        ,  cmdListSrc    , completionCmdSetUserBp/*same as "break"*/,
      NULL
    },
    { "show"     ,  cmdShow       , completionCmdShow,
      "show <copying warranty>\tcopying & distribution terms, warranty"
    },
    { "set"      ,  cmdSetOption  , completionCmdSetOption,
      "set <srcmode>\ttoggle between c/asm.\nset variable <var> = >value\tset variable to new value"
    },
    { "stepi"    ,  cmdStepi      , NULL,
      "stepi\tStep one instruction exactly."
    },
    { "step"     ,  cmdStep       , NULL,
      "{s}tep\tStep program until it reaches a different source line."
    },
    { "source"   ,  cmdSource      , completionCmdSource,
      "source <FILE>\tRead commands from a file named FILE."
    },
    { "s"        ,  cmdStep       , NULL,
      NULL
    },
    { "nexti"    ,  cmdNexti      , NULL,
      "nexti\tStep one instruction, but proceed through subroutine calls."
    },
    { "next"     ,  cmdNext       , NULL,
      "{n}ext\tStep program, proceeding through subroutine calls."
    },
    { "n"        ,  cmdNext       , NULL,
      NULL
    },
    { "run"      ,  cmdRun        , NULL,
      "{r}un\tStart debugged program. "
    },
    { "r"        ,  cmdRun        , NULL,
      NULL
    },
    { "ptype"    ,  cmdPrintType  , completionCmdPrintType,
      "{pt}ype <variable>\tprint type information of a variable"
    },
    { "pt"       ,  cmdPrintType  , NULL,
      NULL
    },
    { "print"    ,  cmdPrint      , completionCmdPrintType,
      "{p}rint <variable>\tprint value of given variable"
    },
    { "output"   ,  cmdOutput      , completionCmdPrint/*same as "print"*/,
      "output <variable>\tprint value of given variable without $ and newline"
    },
    { "p"        ,  cmdPrint      , completionCmdPrintType,
      NULL
    },
    { "file"     ,  cmdFile       , completionCmdFile,
      "file <filename>\tload symbolic information from <filename>"
    },
    { "frame"    ,  cmdFrame      , NULL,
      "{fr}ame\tprint information about the current Stack"
    },
    { "finish"   ,  cmdFinish     , NULL,
      "{fi}nish\texecute till return of current function"
    },
    { "fi"       ,  cmdFinish     , NULL,
      NULL
    },
    { "where"    ,  cmdWhere      , NULL,
      "where\tprint stack"
    },
    { "fr"       ,  cmdFrame      , NULL,
      NULL
    },
    { "f"        ,  cmdFrame      , NULL,
      NULL
    },
    { "x /i"     ,  cmdDisasm1    , NULL,
      "x\tdisassemble one asm command"
    },
    { "!"        ,  cmdSimulator  , NULL,
      "!<simulator command>\tsend a command directly to the simulator"
    },
    { "."        ,  cmdSimulator  , NULL,
      ".{cmd}\tswitch from simulator or debugger command mode"
    },
    { "help"     ,  cmdHelp       , NULL,
      "{h|?}elp\t[CMD_NAME | 0,1,2,3(help page)] (general help or specific help)"
    },
    { "?"        ,  cmdHelp       , NULL,
      NULL
    },
    { "h"        ,  cmdHelp       , NULL,
      NULL
    },
    { "quit"     ,  cmdQuit       , NULL,
      "{q}uit\t\"Watch me now. I'm going Down. My name is Bobby Brown\""
    },
    { "q"        ,  cmdQuit       , NULL,
      NULL
    }
};

/*-----------------------------------------------------------------*/
/* trimming functions                                              */
/*-----------------------------------------------------------------*/
char *trim_left(char *s)
{
  while (isspace(*s))
      ++s;

  return s;
}

char *trim_right(char *s)
{
  char *p = &s[strlen(s) - 1];

  while (p >= s && isspace(*p))
      --p;
  *++p = '\0';

  return s;
}

char *trim(char *s)
{
  return trim_right(trim_left(s));
}

/*-----------------------------------------------------------------*/
/* gc_strdup - make a string duplicate garbage collector aware     */
/*-----------------------------------------------------------------*/
char *gc_strdup(const char *s)
{
  char *ret;
  ret = Safe_malloc(strlen(s)+1);
  strcpy(ret, s);
  return ret;
}

/*-----------------------------------------------------------------*/
/* alloccpy - allocate copy and return a new string                */
/*-----------------------------------------------------------------*/
char *alloccpy ( char *s, int size)
{
  char *d;

  if (!size)
      return NULL;

  d = Safe_malloc(size+1);
  memcpy(d,s,size);
  d[size] = '\0';

  return d;
}

/*-----------------------------------------------------------------*/
/* resize - resizes array of type with new size                    */
/*-----------------------------------------------------------------*/
void **resize (void **array, int newSize)
{
  void **vptr;

  if (array)
      vptr = Safe_realloc(array, newSize*(sizeof(void **)));
  else
      vptr = calloc(1, sizeof(void **));

  if (!vptr)
    {
      fprintf(stderr, "sdcdb: out of memory\n");
      exit(1);
    }

  return vptr;
}

/*-----------------------------------------------------------------*/
/* readCdb - reads the cdb files & puts the records into cdbLine   */
/*           linked list                                           */
/*-----------------------------------------------------------------*/
static int readCdb (FILE *file)
{
  cdbrecs *currl;
  char buffer[1024];
  char *bp;

  if (!(bp = fgets(buffer, sizeof(buffer), file)))
      return 0;

  currl = Safe_calloc(1, sizeof(cdbrecs));
  recsRoot = currl;

  while (1)
    {
      /* make sure this is a cdb record */
      if (strchr("STLFM",*bp) && *(bp+1) == ':')
        {
          /* depending on the record type */

          switch (*bp)
            {
              case 'S':
                /* symbol record */
                currl->type = SYM_REC;
                break;
              case 'T':
                currl->type = STRUCT_REC;
                break;
              case 'L':
                currl->type = LNK_REC;
                break;
              case 'F':
                currl->type = FUNC_REC;
                break;
              case 'M':
                currl->type = MOD_REC ;
                break;
            }

          bp += 2;
          currl->line = Safe_malloc(strlen(bp));
          strncpy(currl->line, bp, strlen(bp)-1);
          currl->line[strlen(bp)-1] = '\0';
        }

      if (!(bp = fgets(buffer, sizeof(buffer), file)))
          break;

      if (feof(file))
          break;

      currl->next = Safe_calloc(1, sizeof(cdbrecs));
      currl = currl->next;
    }

  return (recsRoot->line ? 1 : 0);
}

/*-----------------------------------------------------------------*/
/* searchDirsFname - search directory list & return the filename   */
/*-----------------------------------------------------------------*/
char *searchDirsFname (char *fname)
{
  char *dirs , *sdirs;
  FILE *rfile = NULL;
  char buffer[128];

  /* first try the current directory */
  if ((rfile = fopen(fname, "r")))
    {
      fclose(rfile);
      return strdup(fname) ;
    }

  if (!ssdirl)
      return strdup(fname);

  /* make a copy of the source directories */
  dirs = sdirs = strdup(ssdirl);

  /* assume that the separator is ':'
     and try for each directory in the search list */
  dirs = strtok(dirs, ":");
  while (dirs)
    {
      if (dirs[strlen(dirs)] == '/')
          sprintf(buffer, "%s%s", dirs, fname);
      else
          sprintf(buffer, "%s/%s", dirs, fname);
      if ((rfile = fopen(buffer, "r")))
          break;
      dirs = strtok(NULL, ":");
    }

  free(sdirs);
  if (rfile)
    {
      fclose(rfile);
      return strdup(buffer);
    }
  else //not found
    {
      char *p, *found;

//      sprintf(buffer, "%s", fname);
      p = fname;
      while (NULL != (p = strchr(p, '_')))
        {
          *p = '.'; // try again with '_' replaced by '.'
          if (NULL != (found = searchDirsFname(fname)))
            return found;
          *p = '_'; // not found, restore '_' and try next '_'
        }
    }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* searchDirsFopen - go thru list of directories for filename given*/
/*-----------------------------------------------------------------*/
FILE *searchDirsFopen(char *fname)
{
  char *dirs , *sdirs;
  FILE *rfile = NULL;
  char buffer[128];

  /* first try the current directory */
  if ((rfile = fopen(fname, "r")))
      return rfile;

  if (!ssdirl)
      return NULL;

  /* make a copy of the source directories */
  dirs = sdirs = strdup(ssdirl);

  /* assume that the separator is ':'
     and try for each directory in the search list */
  dirs = strtok(dirs, ":");
  while (dirs)
    {
      sprintf(buffer, "%s/%s", dirs, fname);
      if ((rfile = fopen(buffer, "r")))
          break;
      dirs = strtok(NULL, ":");
    }

  free(sdirs);
  return rfile ;
}

/*-----------------------------------------------------------------*/
/* loadFile - loads a file into module buffer                      */
/*-----------------------------------------------------------------*/
srcLine **loadFile (char *name, int *nlines)
{
  FILE *mfile;
  char buffer[512];
  char *bp;
  srcLine **slines = NULL;

  if (!(mfile = searchDirsFopen(name)))
    {
      fprintf(stderr, "sdcdb: cannot open module %s -- use '--directory=<source directory> option\n", name);
      return NULL;
    }

  while ((bp = fgets(buffer, sizeof(buffer), mfile)))
    {
      (*nlines)++;

      slines = (srcLine **)resize((void **)slines, *nlines);

      slines[(*nlines)-1] = Safe_calloc(1, sizeof(srcLine));
      slines[(*nlines)-1]->src = alloccpy(bp, strlen(bp));
      slines[(*nlines)-1]->addr = INT_MAX;
    }

  fclose(mfile);
  return slines;
}


/*-----------------------------------------------------------------*/
/* loadModules - reads the source files into module structure      */
/*-----------------------------------------------------------------*/
static void loadModules (void)
{
  cdbrecs *loop;
  module *currMod;
  char *rs;

  /* go thru the records & find out the module
     records & load the modules specified */
  for ( loop = recsRoot ; loop ; loop = loop->next )
    {
      switch (loop->type)
        {
          /* for module records do */
          case MOD_REC:
            currMod = parseModule(loop->line, TRUE);
            currModName = currMod->name ;

            /* search the c source file and load it into buffer */
            currMod->cfullname = searchDirsFname(currMod->c_name);
            currMod->cLines = loadFile (currMod->c_name, &currMod->ncLines);

            /* do the same for the assembler file */
            currMod->afullname = searchDirsFname(currMod->asm_name);
            currMod->asmLines = loadFile (currMod->asm_name, &currMod->nasmLines);
            break;

          /* if this is a function record */
          case FUNC_REC:
            parseFunc(loop->line);
            break;

          /* if this is a structure record */
          case STRUCT_REC:
            parseStruct(loop->line);
            break;

          /* if symbol then parse the symbol */
          case  SYM_REC:
            parseSymbol(loop->line, &rs, 2);
            break;

          case LNK_REC:
            parseLnkRec(loop->line);
            break;
        }
    }
}

/*-----------------------------------------------------------------*/
/* generate extra sets of sfr and sbit symbols                     */
/*-----------------------------------------------------------------*/
static void specialFunctionRegs (void)
{
  symbol *sym;
  for (sym = setFirstItem(symbols); sym; sym = setNextItem(symbols))
    {
      if ( sym->addrspace == 'I' || sym->addrspace == 'J')
        {
          addSet(&sfrsymbols, sym);
        }
    }
}
/*-----------------------------------------------------------------*/
/* functionPoints - determine the execution points within a func   */
/*-----------------------------------------------------------------*/
static void functionPoints (void)
{
  function *func;
  symbol *sym;
  exePoint *ep;

  // add _main dummy for runtime env
  if ((func = needExtraMainFunction()))
    {
      function *func1;

      /* alloc new _main function */
      func1 = Safe_calloc(1, sizeof(function));
      *func1 = *func;
      func1->sym = Safe_calloc(1, sizeof(symbol));
      *func1->sym = *func->sym;
      func1->sym->name  = alloccpy("_main", 5);
      func1->sym->rname = alloccpy("G$_main$0$", 10);
      /* TODO must be set by symbol information */
      func1->sym->addr  = 0;
      func1->sym->eaddr = 0x2f;
      addSet(&functions, func1);
    }

  /* for all functions do */
  for ( func = setFirstItem(functions); func; func = setNextItem(functions))
    {
      int j ;
      module *mod;

      sym = func->sym;

      Dprintf(D_sdcdb, ("sdcdb: func '%s' has entry '0x%x' exit '0x%x'\n",
                        func->sym->name,
                        func->sym->addr,
                        func->sym->eaddr));

      if (!func->sym->addr && !func->sym->eaddr)
          continue;

      /* for all source lines in the module find
         the ones with address >= start and <= end
         and put them in the point */
      mod = NULL ;
      if (! applyToSet(modules, moduleWithName, func->modName, &mod))
          continue;
      func->mod = mod;
      func->entryline= INT_MAX-2;
      func->exitline = 0;
      func->aentryline = INT_MAX-2;
      func->aexitline = 0;

      /* do it for the C Lines first */
      for ( j = 0 ; j < mod->ncLines ; j++ )
        {
          if (mod->cLines[j]->addr < INT_MAX &&
              mod->cLines[j]->addr >= sym->addr &&
              mod->cLines[j]->addr <= sym->eaddr )
            {
              /* add it to the execution point */
              if (func->entryline > j)
                  func->entryline = j;

              if (func->exitline < j)
                  func->exitline = j;

              ep = Safe_calloc(1, sizeof(exePoint));
              ep->addr =  mod->cLines[j]->addr ;
              ep->line = j;
              ep->block= mod->cLines[j]->block;
              ep->level= mod->cLines[j]->level;
              addSet(&func->cfpoints, ep);
            }
        }
      /* check double line execution points of module */
      for (ep = setFirstItem(mod->cfpoints); ep; ep = setNextItem(mod->cfpoints))
        {
          if (ep->addr >= sym->addr && ep->addr <= sym->eaddr )
            {
              addSet(&func->cfpoints, ep);
            }
        }
      /* do the same for asm execution points */
      for ( j = 0 ; j < mod->nasmLines ; j++ )
        {
          if (mod->asmLines[j]->addr < INT_MAX &&
              mod->asmLines[j]->addr >= sym->addr &&
              mod->asmLines[j]->addr <= sym->eaddr )
            {
              exePoint *ep ;
              /* add it to the execution point */
              if (func->aentryline > j)
                  func->aentryline = j;

              if (func->aexitline < j)
                  func->aexitline = j;

              /* add it to the execution point */
              ep = Safe_calloc(1, sizeof(exePoint));
              ep->addr =  mod->asmLines[j]->addr;
              ep->line = j;
              addSet(&func->afpoints, ep);
            }
        }
      if ( func->entryline == INT_MAX-2 )
          func->entryline = 0;
      if ( func->aentryline == INT_MAX-2 )
          func->aentryline = 0;

#ifdef SDCDB_DEBUG
      if (!( D_sdcdb & sdcdbDebug))
          continue;

      Dprintf(D_sdcdb, ("sdcdb: function '%s' has the following C exePoints\n",
                        func->sym->name));
      {
        exePoint *ep;

        for (ep = setFirstItem(func->cfpoints); ep; ep = setNextItem(func->cfpoints))
          {
            Dprintf(D_sdcdb, ("sdcdb: {0x%x,%d} %s",
                              ep->addr, ep->line+1, mod->cLines[ep->line]->src));
          }

        Dprintf(D_sdcdb, ("sdcdb:  and the following ASM exePoints\n"));
        for (ep = setFirstItem(func->afpoints); ep; ep = setNextItem(func->afpoints))
          {
            Dprintf (D_sdcdb, ("sdcdb: {0x%x,%d} %s",
                               ep->addr, ep->line+1, mod->asmLines[ep->line]->src));
          }
      }
#endif
    }
}


/*-----------------------------------------------------------------*/
/* setEntryExitBP - set the entry & exit Break Points for functions*/
/*-----------------------------------------------------------------*/
DEFSETFUNC(setEntryExitBP)
{
  function *func = item;

  if (func->sym && func->sym->addr && func->sym->eaddr)
    {
      /* set the entry break point */
      setBreakPoint (func->sym->addr, CODE, FENTRY,
                     fentryCB, func->mod->c_name, func->entryline);

      /* set the exit break point */
      setBreakPoint (func->sym->eaddr, CODE, FEXIT,
                     fexitCB, func->mod->c_name, func->exitline);
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* cmdFile - load file into the debugger                           */
/*-----------------------------------------------------------------*/
int cmdFile (char *s,context *cctxt)
{
  FILE *cdbFile;
  char buffer[128];
  char *bp;

  s = trim_left(s);

  if (!*s)
    {
      fprintf(stdout, "No exec file now.\nNo symbol file now.\n");
      return 0;
    }

  sprintf(buffer, "%s.cdb", s);
  /* try creating the cdbfile */
  if (!(cdbFile = searchDirsFopen(buffer)))
    {
      fprintf(stdout, "Cannot open file\"%s\", no symbolic information loaded\n", buffer);
      // return 0;
    }

  /* allocate for context */
  currCtxt = Safe_calloc(1, sizeof(context));

  if (cdbFile)
    {
      /* read in the debug information */
      if (!readCdb (cdbFile))
        {
          fprintf(stdout,"No symbolic information found in file %s.cdb\n",s);
          //return 0;
        }
    }

  /* parse and load the modules required */
  loadModules();

  /* determine the execution points for this module */
  functionPoints();

  /* extract known special function registers */
  specialFunctionRegs();

  /* start the simulator & setup connection to it */
#ifdef _WIN32
  if (INVALID_SOCKET == sock)
#else
  if ( sock == -1 )
#endif
      openSimulator((char **)simArgs, nsimArgs);
  fprintf(stdout, "%s", simResponse());
  /* now send the filename to be loaded to the simulator */
  sprintf(buffer, "%s.ihx", s);
  bp = searchDirsFname(buffer);
  simLoadFile(bp);
  free(bp);

  /* set the break points
     required by the debugger . i.e. the function entry
     and function exit break points */
  applyToSet(functions, setEntryExitBP);

  setMainContext();
  return 0;
}

/*-----------------------------------------------------------------*/
/* cmdSource - read commands from file                             */
/*-----------------------------------------------------------------*/
int cmdSource (char *s, context *cctxt)
{
  FILE *cmdfile;

  s = trim(s);

  if (!( cmdfile = searchDirsFopen(s)))
    {
      fprintf(stderr,"commandfile '%s' not found\n",s);
      return 0;
    }
  commandLoop( cmdfile );
  fclose( cmdfile );
  return 0;
}

/*-----------------------------------------------------------------*/
/* cmdHelp - help command                                          */
/*-----------------------------------------------------------------*/
#define TEXT_OFFSET     24

static void printHelpLine(const char *htxt, int offs)
{
  static char *spaces = NULL;
  const char *p;
  int state = 0;

  if (NULL == spaces)
    {
      spaces = Safe_malloc(TEXT_OFFSET + 1);
      memset(spaces, ' ', TEXT_OFFSET);
      spaces[TEXT_OFFSET] = '\0';
    }

  p = htxt;

  do
    {
      const char *ps = p;
      int len;
      while (*p && *p != '\t' &&  *p != '\n')
          ++p;
      len = p - ps;

      if (state == 0)
        {
          printf("%.*s%.*s", offs, spaces, len, ps); /* command text */

          if (len >= TEXT_OFFSET - offs)
              printf("\n%s", spaces);
          else
              printf("%.*s", TEXT_OFFSET - offs - len, spaces);
        }
      else
        {
          printf("%.*s\n", len, ps);  /* help text */
        }
      state = *p == '\t';
    }
  while (*p++);
}

int cmdHelp (char *s, context *cctxt)
{
  int i ;
  int endline = 999;
  int startline = 0;

  s = trim_left(s);

  if (isdigit(*s))
    {
      endline = ((*s - '0') * 20) + 20;
      if (endline > 0)
          startline = endline - 20;
    }
  else if (*s)
    {
      for (i = 0 ; i < (sizeof(cmdTab)/sizeof(struct cmdtab)) ; i++)
        {
          if ((cmdTab[i].htxt) && !strcmp(cmdTab[i].cmd,s))
            {
              printHelpLine(cmdTab[i].htxt, 0);
              break;
            }
        }
      return 0;
    }

  for (i = 0 ; i < (sizeof(cmdTab)/sizeof(struct cmdtab)) ; i++)
    {
      /* command string matches */

      if ((cmdTab[i].htxt) && (i >= startline))
          printHelpLine(cmdTab[i].htxt, 0);
      if (i == endline)
          break;
    }

  return 0;
}

#define MAX_CMD_LEN 512
static char cmdbuff[MAX_CMD_LEN];
static int sim_cmd_mode = 0;

/*-----------------------------------------------------------------
 interpretCmd - interpret and do the command.  Return 0 to continue,
   return 1 to exit program.
|-----------------------------------------------------------------*/
int interpretCmd (char *s)
{
  static char *pcmd = NULL;
  int i ;
  int rv = 0 ;

  /* if nothing & previous command exists then
     execute the previous command again */
  if (*s == '\n' && pcmd)
      strcpy(s,pcmd);

  /* if previous command exists & is different
     from the current command then copy it */
  if (pcmd)
    {
      if (strcmp(pcmd,s))
        {
          free(pcmd);
          pcmd = strdup(s);
        }
    }
  else
    {
      pcmd = strdup(s);
    }

  /* trim trailing blanks */
  s = trim_right(s);

  if (sim_cmd_mode)
    {
      if (strcmp(s,".") == 0)
        {
          sim_cmd_mode = 0;
          return 0;
        }
      else if (s[0] == '.')
        {
          /* kill the preceeding '.' and pass on as SDCDB command */
          char *s1 = s+1;
          char *s2 = s;
          while (*s1 != 0)
              *s2++ = *s1++;
          *s2 = 0;
        }
      else
        {
          cmdSimulator (s, currCtxt);
          return 0;
        }
    }
  else
    {
      if (strcmp(s,".") ==0)
        {
          sim_cmd_mode = 1;
          return 0;
        }
    }

  for (i = 0 ; i < (sizeof(cmdTab)/sizeof(struct cmdtab)) ; i++)
    {
      /* command string matches */
      if (strncmp(s,cmdTab[i].cmd,strlen(cmdTab[i].cmd)) == 0)
        {
          if (!cmdTab[i].cmdfunc)
              return 1;

          rv = (*cmdTab[i].cmdfunc)(s + strlen(cmdTab[i].cmd),currCtxt);

          /* if full name then give the file name & position */
          if (fullname && showfull && currCtxt && currCtxt->func)
            {
              showfull = 0;
              if (srcMode == SRC_CMODE)
                  fprintf(stdout,"\032\032%s:%d:1:beg:0x%08x\n",
                          currCtxt->func->mod->cfullname,
                          currCtxt->cline+1,currCtxt->addr);
              else
                  fprintf(stdout,"\032\032%s:%d:1:beg:0x%08x\n",
                          currCtxt->func->mod->afullname,
                          currCtxt->asmline,currCtxt->addr);
              displayAll(currCtxt);
            }
          goto ret;
        }
    }
  fprintf(stdout,"Undefined command: \"%s\".  Try \"help\".\n",s);

ret:
  return rv;
}

static FILE *actualcmdfile=NULL ;
static char *actualcmds=NULL;
static int   stopcmdlist;
/*-----------------------------------------------------------------*/
/* getNextCmdLine get additional lines used by special commands    */
/*-----------------------------------------------------------------*/
char *getNextCmdLine(void)
{
  //fprintf(stderr,"getNextCmdLine() actualcmdfile=%p\n",actualcmdfile);
  if (!actualcmdfile)
      return NULL;
  fprintf(stdout,">");
  fflush(stdout);
  if (fgets(cmdbuff,sizeof(cmdbuff),actualcmdfile) == NULL)
    {
      // fprintf(stderr,"getNextCmdLine() returns null\n");
      return NULL;
    }
  //fprintf(stderr,"getNextCmdLine() returns: %s",cmdbuff);
  return cmdbuff;
}

void setCmdLine( char *cmds )
{
  actualcmds = cmds;
}

void stopCommandList()
{
  stopcmdlist = 1;
}

#ifdef HAVE_READLINE_COMPLETITION
// helper function for doing readline completion.
// input: toknum=index of token to find (0=first token)
// output: *start=first character index of the token,
//                or the index of '\0'
//         *end=first blank character right after the token,
//                or the index of '\0'
// return value: 0=token not found, 1=token found
int completionHelper_GetTokenNumber(int toknum, int *start, int *end)
{
  int tok_index;
  const char *p = rl_line_buffer;

  tok_index = 0;
  *start = *end = 0;
  while (p[*end] != 0)
    {
      // start = skip blanks from end
      *start = *end;
      while (p[*start] && isspace( p[*start] ))
          (*start)++;

      // end = skip non-blanks from start
      *end = *start;
      while (p[*end] && !isspace( p[*end] ))
          (*end)++;

      if (tok_index == toknum)
          return 1;   // found

      tok_index++;
    }

  return 0;   // not found
}

// helper function for doing readline completion.
// returns the token number that we were asked to complete.
// 0=first token (command name), 1=second token...
int completionHelper_GetCurrTokenNumber()
{
  int toknum, start, end;

  toknum = start = end = 0;
  while (1)
    {
      if (!completionHelper_GetTokenNumber(toknum, &start, &end))
          return toknum;

      if (rl_point <= end)
          return toknum;

      toknum++;
    }
}

// exapmle for vallist on entry:
//          "copying\0warranty\0";
char *completionCompleteFromStrList(const char *text, int state, char *vallist)
{
  static char *ptr;
  int len;

  if (state == 0)
      ptr = vallist;
  else
      ptr += strlen(ptr)+1;

  len = strlen(text);
  while (*ptr)
    {
      if ( (len < strlen(ptr)) && !strncmp(text, ptr, len) )
          return strdup(ptr);

      ptr += strlen(ptr)+1;
    }

  return NULL;
}

// readline library completion function.
// completes from the list of all sdcdb command.
char *completionCommandsList(const char *text, int state)
{
  static int i = 0;

  if (state == 0) // new completion?
    {   // yes, only complete if this is the first token on the line.
      int ok = 0; // try to complete this request?
      char *p = rl_line_buffer;

      // skip blanks
      while (p && isspace(*p))
        {
          if (p-rl_line_buffer == rl_point)
              ok = 1;
          p++;
        }

      while (p && !isspace(*p))
        {
          if (p-rl_line_buffer == rl_point)
              ok = 1;
          p++;
        }

      if (p-rl_line_buffer == rl_point)
          ok = 1;

      if ( !ok )
          return NULL; // no more completions

      i = 0;  // ok, gonna complete. initialize static variable.
    }
  else
    {
      i++;
    }

  for (; i < (sizeof(cmdTab)/sizeof(struct cmdtab)) ; i++)
    {
      int len = strlen(text);
      if (len <= strlen(cmdTab[i].cmd))
        {
          if (strncmp(text,cmdTab[i].cmd,len) == 0)
              return strdup(cmdTab[i].cmd);
        }
    }

  return NULL; // no more completions
}

// readline library completion function.
// completes from the list of symbols.
char *completionSymbolName(const char *text, int state)
{
  static symbol *sy;

  if (state == 0) // new completion?
      sy = setFirstItem(symbols); // yes
  else
    sy = setNextItem(symbols);

  for (; sy != NULL; )
    {
      int len = strlen(text);
      if (len <= strlen(sy->name))
        {
          if (strncmp(text,sy->name,len) == 0)
              return strdup(sy->name);
        }

      sy = setNextItem(symbols);
    }
  return NULL;
}

// readline library completion function.
// completes from the list known functions.
// module_flag - if false, ignore function module name
//               if true, compare against module_name:fnction_name
char *completionFunctionName(const char *text, int state, int module_flag)
{
  static function *f;

  if (state == 0) // new completion?
      f = setFirstItem(functions); // yes
  else
    f = setNextItem(functions);

  for (; f != NULL; )
    {
      int text_len = strlen(text);

      if (!module_flag)
        {
          if (text_len <= strlen(f->sym->name) &&
              !strncmp(text,f->sym->name,text_len))
            {
              return strdup(f->sym->name);
            }
        }
      else
        {
          int modname_len = strlen(f->mod->c_name);
          int funcname_len = strlen(f->sym->name);
          char *functext = malloc(modname_len+funcname_len+2);
          //assert(functext);
          strcpy(functext,f->mod->c_name);
          strcat(functext,":");
          strcat(functext,f->sym->name);
          if (text_len <= strlen(functext) &&
              !strncmp(text,functext,text_len))
            {
              return functext;
            }
          else
            {
              free(functext);
            }
        }
      f = setNextItem(functions);
    }
  return NULL;
}

// readline library completion function.
// completes from the list known modules.
char *completionModuleName(const char *text, int state)
{
  static module *m;

  if (state == 0) // new completion?
      m = setFirstItem(modules); // yes
  else
      m = setNextItem(modules);

  for (; m != NULL; )
    {
      int len = strlen(text);
      if ( (len <= strlen(m->c_name)) &&
           !strncmp(text,m->c_name,len) )
        {
          return strdup(m->c_name);
        }

      if ( (len <= strlen(m->asm_name)) &&
           (strncmp(text,m->asm_name,len) == 0) )
        {
          return strdup(m->asm_name);
        }

      m = setNextItem(modules);
    }
  return NULL;
}

// readline completion function for "file" command
char *completionCmdFile(const char *text, int state)
{
  if (state == 0)
    {
      if (completionHelper_GetCurrTokenNumber() != 1)
          return NULL;
    }

  // we use filename_completion_function() from the readline library.
  return rl_filename_completion_function(text, state);
}

// readline completion function for "source" command
char *completionCmdSource(const char *text, int state)
{
  return completionCmdFile(text, state);
}

// readline completion function for "info" command
char *completionCmdInfo(const char *text, int state)
{
  if (state == 0)
    {
      if (completionHelper_GetCurrTokenNumber() != 1)
          return NULL;
    }

  return completionCompleteFromStrList(text, state,
          "break\0stack\0frame\0registers\0all-registers\0"
          "line\0source\0functions\0symbols\0variables\0");
}

// readline completion function for "show" command
char *completionCmdShow(const char *text, int state)
{
  if (state == 0)
    {
      if (completionHelper_GetCurrTokenNumber() != 1)
          return NULL;
    }
  return completionCompleteFromStrList(text, state, "copying\0warranty\0");
}

// readline completion function for "la" command
char *completionCmdListSymbols(const char *text, int state)
{
  if (state == 0)
    {
      if (completionHelper_GetCurrTokenNumber() != 1)
          return NULL;
    }
  return completionCompleteFromStrList(text, state, "v1\0v2\0");
}

char *completionCmdPrintType(const char *text, int state)
{
  if (state == 0)
    {
      if (completionHelper_GetCurrTokenNumber() != 1)
          return NULL;
    }
  return completionSymbolName(text, state);
}

char *completionCmdPrint(const char *text, int state)
{
  if (state == 0)
    {
      int i = completionHelper_GetCurrTokenNumber();
      if (i != 1 && i != 2)
          return NULL;
    }
  return completionSymbolName(text, state);
}

char *completionCmdDelUserBp(const char *text, int state)
{
  static breakp *bp;
  static int k;

  if (state == 0)
    {
      if (completionHelper_GetCurrTokenNumber() != 1)
          return NULL;

      if (!userBpPresent)
          return NULL;

      bp = hTabFirstItem(bptable,&k);
    }
  else
    {
      bp = hTabNextItem(bptable,&k);
    }

  for ( ; bp ; bp = hTabNextItem(bptable,&k))
    {
      if (bp->bpType == USER || bp->bpType == TMPUSER)
        {
          char buff[20];
          sprintf(buff, "%d", bp->bpnum);
          return strdup(buff);
        }
    }

  return NULL;
}

// readline completion function for "undisplay" command
char *completionCmdUnDisplay(const char *text, int state)
{
  static dsymbol *dsym;

  if (state == 0)
    {
      if (completionHelper_GetCurrTokenNumber() != 1)
          return NULL;
      dsym = setFirstItem(dispsymbols);
    }

  if (dsym)
    {
      char buff[30];
      sprintf(buff, "%d", dsym->dnum);
      dsym = setNextItem(dispsymbols);
      return strdup(buff);
    }
  return NULL;
}

char *completionCmdSetUserBp(const char *text, int state)
{
  static int internal_state; // 0=calling completionFunctionName(text, state, 0)
                               // 1=calling completionFunctionName(text, 1, 1)
  if (state == 0)
    {
      if (completionHelper_GetCurrTokenNumber() != 1)
          return NULL;

      internal_state = 0;
    }
  if (internal_state == 0)
    {
      char *p = completionFunctionName(text, state, 0);
      if (p)
          return p;
      internal_state = 1;
      return completionFunctionName(text, 0, 1);
    }
  else
    {
      return completionFunctionName(text, 1, 1);
    }
}

char *completionCmdSetOption(const char *text, int state)
{
  static int currtok;

  if (state == 0)
    {
      int start,end;

      currtok = completionHelper_GetCurrTokenNumber();

      if (currtok == 2 || currtok == 3)
        {
          // make sure token 1 == "variable"
          completionHelper_GetTokenNumber(1, &start, &end);
          if (end - start != 8 ||
              strncmp(rl_line_buffer+start,"variable",8))
            {
              return NULL;
            }
        }
      else if (currtok != 1)
        {
          return NULL;
        }
    }

  switch (currtok)
    {
      case 1:
        return completionCompleteFromStrList(text, state,
#ifdef SDCDB_DEBUG
                "debug\0"
#endif
                "srcmode\0listsize\0variable\0");
      case 2:
        return completionSymbolName(text, state);

      case 3:
        return completionCompleteFromStrList(text, state, "=\0");
    }
  return NULL;
}

// our main readline completion function
// calls the other completion functions as needed.
char *completionMain(const char *text, int state)
{
  static rl_compentry_func_t *compl_func;
  int i, start, end, len;

  if (state == 0) // new completion?
    {
      compl_func = NULL;

      if (completionHelper_GetCurrTokenNumber() == 0)
        {
          compl_func = &completionCommandsList;
        }
      else
        { // not completing first token, find the right completion
          // function according to the first token the user typed.
          completionHelper_GetTokenNumber(0, &start, &end);
          len = end-start;

          for (i=0; i < (sizeof(cmdTab)/sizeof(struct cmdtab)) ; i++)
            {
              if (!strncmp(rl_line_buffer+start,cmdTab[i].cmd,len) &&
                  cmdTab[i].cmd[len] == '\0')
                {
                  compl_func = cmdTab[i].completion_func;
                  break;
                }
            }
        }
      if (!compl_func)
          return NULL;
    }

  return (*compl_func)(text,state);
}
#endif  /* HAVE_READLINE_COMPLETITION */

/*-----------------------------------------------------------------*/
/* commandLoop - the main command loop or loop over command file   */
/*-----------------------------------------------------------------*/
static void commandLoop(FILE *cmdfile)
{
  char *line, save_ch, *s;
#ifdef HAVE_LIBREADLINE
  char *line_read;

  FILE *old_rl_instream, *old_rl_outstream;
  actualcmdfile = cmdfile;

#ifdef HAVE_READLINE_COMPLETITION
  rl_completion_entry_function = completionMain;
#endif  /* HAVE_READLINE_COMPLETITION */
  rl_readline_name = "sdcdb"; // Allow conditional parsing of the ~/.inputrc file.

  // save readline's input/output streams
  // this is done to support nested calls to commandLoop()
  // i wonder if it works...
  old_rl_instream = rl_instream;
  old_rl_outstream = rl_outstream;

  // set new streams for readline
  if ( cmdfile == stdin )
      rl_instream = rl_outstream = NULL;  // use stdin/stdout pair
  else
      rl_instream = rl_outstream = cmdfile;

  while (1)
    {
      if ( cmdfile == stdin )
        {
          if (sim_cmd_mode)
            line_read = (char*)readline ("(sim) ");
          else
            line_read = (char*)readline ("(sdcdb) ");
        }
      else
        line_read = (char*)readline ("");

      if (line_read)
        {
          /* If the line has any text in it,
             save it on the history. */
          if (line_read && *line_read)
              add_history (line_read);

           // FIX: readline returns malloced string.
           //   should check the source to verify it can be used
           //    directly. for now - just copy it to cmdbuff.
           strcpy(cmdbuff,line_read);
#if defined(_WIN32) || defined(HAVE_RL_FREE)
            rl_free(line_read);
#else
            free(line_read);
#endif
            line_read = NULL;
        }
      else
        {
          break;  // EOF
        }
#else  /* HAVE_LIBREADLINE */
  actualcmdfile = cmdfile;

  while (1)
    {
      if ( cmdfile == stdin )
        {
          if (sim_cmd_mode)
              printf("(sim) ");
          else
              fprintf(stdout,"(sdcdb) ");
          fflush(stdout);
        }
      //fprintf(stderr,"commandLoop actualcmdfile=%p cmdfile=%p\n",
      //        actualcmdfile,cmdfile);
      if (fgets(cmdbuff,sizeof(cmdbuff),cmdfile) == NULL)
          break;
#endif  /* HAVE_LIBREADLINE */

      if (interpretCmd(cmdbuff))
          break;

      while ( actualcmds )
        {
          strcpy(cmdbuff,actualcmds);
          actualcmds = NULL;
          stopcmdlist= 0;
          for ( line = cmdbuff; *line ; line = s )
            {
              if ( (s=strchr(line ,'\n')))
                {
                  save_ch = *++s;
                  *s = '\0';
                }
              else
                {
                  s += strlen( line );
                  save_ch = '\0';
                }
              if (interpretCmd( line ))
                {
                  *s = save_ch;
                  break;
                }
              *s = save_ch;
              if ( stopcmdlist )
                  break;
            }
        }
    }
#ifdef HAVE_LIBREADLINE
  // restore readline's input/output streams
  rl_instream = old_rl_instream;
  rl_outstream = old_rl_outstream;
#endif  /* HAVE_LIBREADLINE */
}

/*-----------------------------------------------------------------*/
/* printVersionInfo - print the version information                */
/*-----------------------------------------------------------------*/
static void printVersionInfo(void)
{
  fprintf(stdout,
          "SDCDB is free software and you are welcome to distribute copies of it\n"
          "under certain conditions; type \"show copying\" to see the conditions.\n"
          "There is absolutely no warranty for SDCDB; type \"show warranty\" for details.\n"
          "SDCDB " SDCDB_VERSION ". Copyright (C) 1999 Sandeep Dutta (sandeep.dutta@usa.net)\n");
}

/*-----------------------------------------------------------------*/
/* printHelp - print help                                          */
/*-----------------------------------------------------------------*/
static void printHelp(void)
{
  fprintf(stdout, "Type ? for help\n");
}

/*-----------------------------------------------------------------*/
/* escapeQuotes - escape double quotes                             */
/*-----------------------------------------------------------------*/
#define CHUNK  256

static const char *escapeQuotes(const char *arg)
{
#define extend(n)  do { if ((size_t)(ps - str + (n)) > strLen) str = Safe_realloc (str, strLen += CHUNK); } while (0)

  static char *str = NULL;
  static size_t strLen = 0;
  char *ps;
  const char *pa;

  if (NULL == str)
    {
      strLen = CHUNK;
      str = Safe_malloc (strLen);
    }

  for (ps = str, pa = arg; '\0' != *pa; ++pa)
    {
      if ('"' == *pa)
        {
          extend (2);
          *ps++ = '\\';       /* excape the quote */
          *ps++ = *pa;
        }
      else
        {
          extend (1);
          *ps++ = *pa;
        }
    }
  extend (1);
  *ps = '\0';

  return str;
}

/*-----------------------------------------------------------------*/
/* argsToCmdLine - concatenate arguments ti command line           */
/*-----------------------------------------------------------------*/
char *argsToCmdLine(char **args, int nargs)
{
  static char *cmd = NULL;
  static size_t cmdLen = 0;
  int i;
  size_t cmdPos = 0;

  if (NULL == cmd)
    {
      cmd = Safe_malloc(CHUNK);
      cmdLen = CHUNK;
    }

  for (i = 0; i < nargs; ++i)
    {
      size_t argLen;
      size_t allocLen = 0;
      int quote = 0;
      const char *arg;

      if (0 < i)
          ++allocLen;                 /* space for space character */

      if (NULL != strchr(args[i], ' '))
        {
          quote = 1;
          allocLen += 2;              /* space for inital and final quote */
          arg = escapeQuotes(args[i]);
        }
      else
        {
          arg = args[i];
        }

      argLen = strlen(arg);
      allocLen += argLen;             /* space for argument */

      /* extend the buffer */
      if (cmdPos + allocLen >= cmdLen)
        {
          do
            {
              cmdLen += cmdLen;
            }
          while (cmdPos + allocLen >= cmdLen);
          cmd = Safe_realloc(cmd, cmdLen);
        }

      if (0 < i)
        {
          cmd[cmdPos++] = ' ';        /* append space character */
        }

      if (quote)
        {
          cmd[cmdPos++] = '"';        /* append initial quote */
        }

      memcpy(&cmd[cmdPos], arg, argLen); /* append argument */
      cmdPos += argLen;

      if (quote)
          cmd[cmdPos++] = '"';        /* append final quote */
    }

  cmd[cmdPos] = '\0';

  return cmd;
}

static void usage(void)
{
  const char *args =
        "-{h|?}, --help\tDisplay this help\n"
        "-v\tVerbose: show the simulator invocation commald line\n"
        "--directory=<dir>\tSearch modules in <dir> directory\n"
        "-fullname\tGive the file name & position\n"
        "-cd=<dir>, -cd <dir>\tChange directory to <dir>\n"
#ifdef SDCDB_DEBUG
        "-d=<msk>\tSet debugging to <mask>\n"
#endif
        "-contsim\tContinuous simulation\n"
        "-q\tIgnored\n"
        "-m<model>\tModel string: avr, xa, z80\n"
        "-z\tAll remaining options are for simulator";

  const char *simArgs =
        "-t <cpu>, -cpu <cpu>\tCpu type\n"
        "-frequency <frequency>, -X <frequency>\tXTAL Frequency\n"
        "-{s|S} <serial_port>\tSerial port\n"
        "-k\tNetwork serial port";

  printf("usage: sdcdb [args] [simulator args] [filename]\n"
         "args:\n");
  printHelpLine(args, 2);
  printf("simulator args:\n");
  printHelpLine(simArgs, 2);
  putchar('\n');
  printVersionInfo();
}

/*-----------------------------------------------------------------*/
/* parseCmdLine - parse the commandline arguments                  */
/*-----------------------------------------------------------------*/
static void parseCmdLine (int argc, char **argv)
{
  int i;
  char *filename = NULL;
  int passon_args_flag = 0;  /* if true, pass on args to simulator */
  int verbose = 0;

  Dprintf(D_sdcdb, ("sdcdb: parseCmdLine\n"));
  contsim = 0;

  for (i = 1; i < argc ; i++)
    {
      if (passon_args_flag) /* if true, pass on args to simulator */
        {
          simArgs[nsimArgs++] = strdup(argv[i]);
          continue;
        }

      /* if this is an option */
      if (argv[i][0] == '-')
        {
          /* display usage */
          if (strcmp(argv[i], "-h") == 0 ||
              strcmp(argv[i], "-?") == 0 ||
              strcmp(argv[i], "--help") == 0)
            {
              usage();
              exit(0);
            }

          /* verbose */
          if (strcmp(argv[i], "-v") == 0)
            {
              verbose = 1;
              continue;
            }

          /* if directory then mark directory */
          if (strncmp(argv[i], "--directory=", 12) == 0)
            {
              if (!ssdirl)
                {
                  ssdirl = &argv[i][12];
                }
              else
                {
                  char *p = Safe_malloc(strlen(ssdirl)+strlen(&argv[i][12])+2);
                  strcat(strcat(strcpy(p,&argv[i][12]),":"),ssdirl);
                  ssdirl = p;
                }
              continue;
            }

          if (strcmp(argv[i], "-fullname") == 0)
            {
              fullname = TRUE;
              continue;
            }

          if (strncmp(argv[i], "-cd=", 4) == 0)
            {
              if (0 > chdir(&argv[i][4]))
                {
                  fprintf(stderr, "can't change directory to %s\n", &argv[i][4]);
                  exit(1);
                }
              continue;
            }

          if (strcmp(argv[i], "-cd") == 0)
            {
              i++;
              if (0 > chdir(argv[i]))
                {
                  fprintf(stderr, "can't change directory to %s\n", argv[i]);
                  exit(1);
                }
              continue;
            }

#ifdef SDCDB_DEBUG
          if (strncmp(argv[i], "-d=", 3) == 0)
            {
              sdcdbDebug = strtol(&argv[i][3],0,0);
              continue;
            }
#endif
          if (strcmp(argv[i], "-contsim") == 0)
            {
              contsim=1;
              continue;
            }
          if (strcmp(argv[i], "-q") == 0)
            {
              continue;
            }

          /* model string */
          if (strncmp(argv[i],"-m", 2) == 0)
            {
              strncpy(model_str, &argv[i][2], 15);
              if (strcmp(model_str, "avr") == 0)
                  simArgs[0] = "savr";
              else if (strcmp(model_str, "xa") == 0)
                  simArgs[0] = "sxa";
              else if (strcmp(model_str, "z80") == 0)
                  simArgs[0] = "sz80";
              continue;
            }

          /* -z all remaining options are for simulator */
          if (strcmp(argv[i], "-z") == 0)
            {
              passon_args_flag = 1;
              continue;
            }

          /* the simulator arguments */

          /* cpu */
          if (strcmp(argv[i], "-t") == 0 ||
              strcmp(argv[i], "-cpu") == 0)
            {
              simArgs[nsimArgs++] = "-t";
              simArgs[nsimArgs++] = strdup(argv[++i]);
              continue;
            }

          /* XTAL Frequency */
          if (strcmp(argv[i], "-X") == 0 ||
              strcmp(argv[i], "-frequency") == 0)
            {
              simArgs[nsimArgs++] = "-X";
              simArgs[nsimArgs++] = strdup(argv[++i]);
              continue;
            }

          /* serial port */
          if ((strcmp(argv[i], "-S") == 0) ||
              (strcmp(argv[i], "-s") == 0))
            {
              simArgs[nsimArgs++] = strdup(argv[i]);
              simArgs[nsimArgs++] = strdup(argv[++i]);
              continue;
            }

          /* network serial port */
          if ((strcmp(argv[i], "-k") == 0))
            {
              simArgs[nsimArgs++] = strdup(argv[i]);
              simArgs[nsimArgs++] = strdup(argv[++i]);
              continue;
            }

          fprintf(stderr,"unknown option %s --- ignored\n", argv[i]);
        }
      else
        {
          FILE* file;

          /* must be file name */
          if (filename)
            {
              fprintf(stderr,
                      "too many filenames .. parameter '%s' ignored\n",
                      argv[i]);
              continue ;
            }

          file = fopen(argv[i], "r");
          if (file)
            {
              /* file exists: strip the cdb or ihx extension */
              char *p = strrchr(argv[i], '.');

              fclose(file);
              if (NULL != p &&
                  (0 == strcmp(p, ".cdb") || 0 == strcmp(p, ".ihx")))
                {
                  *p = '\0';
                }
            }
          filename = argv[i];
        }
    }

  if (filename)
      cmdFile(filename,NULL);

  if (verbose)
      printf("+ %s\n", argsToCmdLine(simArgs, nsimArgs));
}

/*-----------------------------------------------------------------*/
/* setsignals -  catch some signals                                */
/*-----------------------------------------------------------------*/
#include <signal.h>
static void
bad_signal(int sig)
{
  if ( simactive )
      closeSimulator();
  exit(1);
}

static void
sigintr(int sig)
{
  /* may be interrupt from user: stop debugger and also simulator */
  userinterrupt = 1;
  if ( !nointerrupt )
      sendSim("stop\n");
}

#ifndef _WIN32
/* the only child can be the simulator */
static void sigchld(int sig)
{
  /* the only child can be the simulator */
  int status;
  wait ( &status );
  simactive = 0;
}
#endif

static void
setsignals()
{
  signal(SIGINT , sigintr );
  signal(SIGABRT, bad_signal);
  signal(SIGTERM, bad_signal);

#ifndef _WIN32
  signal(SIGHUP , SIG_IGN);
  signal(SIGCONT, SIG_IGN);
  signal(SIGCHLD, sigchld );

  signal(SIGALRM, bad_signal);
  //signal(SIGFPE,  bad_signal);
  //signal(SIGILL,  bad_signal);
  signal(SIGPIPE, bad_signal);
  signal(SIGQUIT, bad_signal);
  //signal(SIGSEGV, bad_signal);
#endif
}

/*-----------------------------------------------------------------*/
/* main -                                                          */
/*-----------------------------------------------------------------*/

int main ( int argc, char **argv)
{
  simArgs[nsimArgs++] = "s51";
  simArgs[nsimArgs++] = "-P";
  simArgs[nsimArgs++] = "-r";
  simArgs[nsimArgs++] = "9756";

  /* parse command line */
  parseCmdLine(argc, argv);

  printVersionInfo();
  printHelp();
  printf("WARNING: SDCDB is EXPERIMENTAL.\n");

  setsignals();

  commandLoop(stdin);

  return 0;
}
