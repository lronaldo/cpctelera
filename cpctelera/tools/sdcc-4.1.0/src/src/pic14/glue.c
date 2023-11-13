/*-------------------------------------------------------------------------

  glue.c - glues everything we have done together into one file.
                Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)

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

#include "glue.h"
#include "dbuf_string.h"

#include "device.h"
#include "gen.h"
#include "main.h"

#define DBG_MSG(fmt,...)
#define DBG_MEMMAP(lbl,map)
#define DBG_VALUE(lbl,val)
#define DBG_SYMBOL(lbl,sym)
#define DBG_TYPE(lbl,type)
#define DBG_ILIST(lbl,list)
#define DBG_REG(lbl,reg)
#define DBG_OPEN(mode,info)
#define DBG_CLOSE()
#define DBG_ENTRY()
#define DBG_EXIT()

/*
 * Imports from SDCCglue.c
 */

//extern set *publics;            /* public variables */
extern set *externs;            /* Variables that are declared as extern */
extern set *ccpStr;             /* char * const pointers with a string literal initializer */

extern int allocInfo;
extern symbol *mainf;
extern int noInit;              /* no initialization */

extern void initialComments (FILE *afile);
extern value *initPointer (initList *ilist, sym_link *toType, int showError);

/*
 * Imports from SDCCast.c
 */

//extern int noAlloc;
//extern struct dbuf_s *codeOutBuf;


set *pic14_localFunctions = NULL;
int pic14_hasInterrupt = 0;     // Indicates whether to emit interrupt handler or not

int pic14_stringInSet (const char *str, set **world, int autoAdd);


#ifdef WORDS_BIGENDIAN
#define _ENDIAN(x)  (3-x)
#else
#define _ENDIAN(x)  (x)
#endif

#define BYTE_IN_LONG(x,b) ((x>>(8*_ENDIAN(b)))&0xff)
#define IS_GLOBAL(sym)  ((sym)->level == 0)
#define IS_DEFINED_HERE(sym)    (!IS_EXTERN(sym->etype))

/* dbufs for initialized data (idata and code sections),
 * extern, and global declarations */
static struct dbuf_s *ivalBuf, *extBuf, *gloBuf, *gloDefBuf, *locBuf;

static set *emitted = NULL;
static set *declared = NULL;

static void showAllMemmaps (FILE *of); // XXX: emits initialized symbols

static void
addToPublics (symbol *sym)
{
  symbol *psym;

  for (psym = setFirstItem (publics); psym; psym = setNextItem (publics))
    if (!strcmp (sym->rname, psym->rname))
      {
        DBG_MSG ("symbol %s already in publics", sym->rname);
        return;
      }

  DBG_MSG ("symbol %s added to publics", sym->rname);
  addSetHead (&publics, sym);
}

static void
addToExterns (symbol *sym)
{
  symbol *esym;

  for (esym = setFirstItem (externs); esym; esym = setNextItem (externs))
    if (!strcmp (sym->rname, esym->rname))
      {
        DBG_MSG ("symbol %s already in externs", sym->rname);
        return;
      }

  DBG_MSG ("symbol %s added to externs", sym->rname);
  addSetHead (&externs, sym);
}

static void
emitPseudoStack (struct dbuf_s *oBuf, struct dbuf_s *oBufExt)
{
  int shared, low, high, size, i;
  PIC_device *pic = pic14_getPIC ();

  /* also emit STK symbols
   * XXX: This is ugly and fails as soon as devices start to get
   *      differently sized sharebanks, since STK12 will be
   *      required by larger devices but only up to STK03 might
   *      be defined using smaller devices. */
  shared = pic14_getSharedStack (&low, &high, &size);
  DBG_MSG ("emitPseudoStack shared %d low %d high %d size %d library %s enhanced %s",
           shared, low, high, size, pic14_options.isLibrarySource ? "yes" : "no", pic->isEnhancedCore ? "yes" : "no");
  if (!pic14_options.isLibrarySource)
    {
      dbuf_printf (oBuf, "\n");
      if (!pic->isEnhancedCore)
        {
          size -= 3;
          dbuf_printf (oBuf, "\tglobal PSAVE\n");
          dbuf_printf (oBuf, "\tglobal SSAVE\n");
          dbuf_printf (oBuf, "\tglobal WSAVE\n");
        }
      for (i = size - 1; i >= 0; i--)
        {
          dbuf_printf (oBuf, "\tglobal STK%02d\n", i);
        }                       // for i
      dbuf_printf (oBuf, "\n");

      // 16f84 has no SHAREBANK (in linkerscript) but memory aliased in two
      // banks, sigh...
      if (1 || !shared)
        {
          // for single banked devices: use normal, "banked" RAM
          dbuf_printf (oBuf, "sharebank udata_ovr 0x%04X\n", low);
        }
      else
        {
          // for devices with at least two banks, require a sharebank section
          dbuf_printf (oBuf, "sharebank udata_shr\n");
        }
      if (!pic->isEnhancedCore)
        {
          dbuf_printf (oBuf, "PSAVE\tres 1\n");
          dbuf_printf (oBuf, "SSAVE\tres 1\n");
          dbuf_printf (oBuf, "WSAVE\tres 1\n"); // WSAVE *must* be in sharebank (IRQ handlers)
        }
      /* fill rest of sharebank with stack STKxx .. STK00 */
      for (i = size - 1; i >= 0; i--)
        {
          dbuf_printf (oBuf, "STK%02d\tres 1\n", i);
        }                       // for i
    }
  else
    {
      /* declare STKxx as extern for all files
       * except the one containing main() */
      dbuf_printf (oBufExt, "\n");
      if (!pic->isEnhancedCore)
        {
          size -= 3;
          dbuf_printf (oBufExt, "\textern PSAVE\n");
          dbuf_printf (oBufExt, "\textern SSAVE\n");
          dbuf_printf (oBufExt, "\textern WSAVE\n");
        }
      for (i = size - 1; i >= 0; i--)
        {
          char buffer[128];
          SNPRINTF (&buffer[0], 127, "STK%02d", i);
          dbuf_printf (oBufExt, "\textern %s\n", &buffer[0]);
        }                       // for i
    }
  dbuf_printf (oBuf, "\n");
}

/*-----------------------------------------------------------------*/
/* pic14_emitDebugSym - emit label for debug symbol                */
/*-----------------------------------------------------------------*/
static void
pic14_emitDebugSym (struct dbuf_s *oBuf, symbol *sym)
{
  if (!options.debug)
    return;

//FIXME: commented out until valid gpasm debug directives are used.
  dbuf_printf (oBuf, "; ");

  if (sym->level && sym->localof)       /* symbol scope is local */
    {
      dbuf_printf (oBuf, "L%s_%s_", moduleName, sym->localof->name);
    }
  else if (IS_STATIC (sym->etype))      /* symbol scope is file */
    {
      dbuf_printf (oBuf, "F%s_", moduleName);
    }
  else                          /* symbol scope is global */
    {
      dbuf_printf (oBuf, "G_");
    }
  dbuf_printf (oBuf, "%s_%ld_%d", sym->name, sym->level, sym->block);
  if (SPEC_ABSA (sym->etype))
    dbuf_printf (oBuf, " == 0x%04x\n", SPEC_ADDR (sym->etype));
  else
    dbuf_printf (oBuf, " == .\n");
}

/*
 * Allocate space for uninitialized data.
 */

static void
pic14_allocateSpace (symbol *sym, int size)
{
  static unsigned sectionNr = 0;

  if (!pic14_stringInSet (sym->rname, &emitted, true))
    {
      DBG_MSG ("global definition UD udata: module %s section %u name %s size %d", moduleName, sectionNr, sym->rname, size);

      dbuf_printf (gloDefBuf, "UD_%s_%u\tudata\n", moduleName, sectionNr++);

      /* print extra debug info if required */
      pic14_emitDebugSym (gloDefBuf, sym);

      dbuf_printf (gloDefBuf, "%s\tres\t%d\n\n", sym->rname, size);
    }
}

/*
 * Says whether ival values must be written to code space (1) or data space (0).
 * Set by pic14_printIvalLabel. Used by all ival printing functions.
 */
static int in_code;

static void pic14_printIvalLabel (struct dbuf_s *oBuf, symbol *sym);
static void pic14_printLiteral (struct dbuf_s *oBuf, long lit, int size);
static void pic14_printIval (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf, bool check);

static hTab *absmap_ht = NULL;
static int absmap_min = -1, absmap_max = -1;

static void
addToAbsMap (symbol *sym)
{
  set *aliases;
  int addr;

  if (IS_DEFINED_HERE (sym) && SPEC_ABSA (sym->etype))
    {
      DBG_SYMBOL ("", sym);
      DBG_ENTRY ();
      addr = SPEC_ADDR (sym->etype);

      if (absmap_max == -1 || addr > absmap_max)
        absmap_max = addr;
      if (absmap_min == -1 || addr < absmap_min)
        absmap_min = addr;
      //fprintf (stderr, "%s: sym %s @ 0x%x\n", __FUNCTION__, sym->name, addr);
      aliases = hTabItemWithKey (absmap_ht, addr);
      if (aliases)
        {
          /* May not use addSetHead, as we cannot update the
           * list's head in the hastable `absmap_ht'. */
          addSet (&aliases, sym);
        }
      else
        {
          addSet (&aliases, sym);
          hTabAddItem (&absmap_ht, addr, aliases);
        }                       // if
      DBG_EXIT ();
    }                           // if
}

static void
pic14_constructAbsMap (struct dbuf_s *oBuf)
{
  int addr;

  /* now emit definitions for all absolute symbols */
  dbuf_printf (oBuf, "%s", iComments2);
  dbuf_printf (oBuf, "; absolute symbol definitions\n");
  dbuf_printf (oBuf, "%s", iComments2);
  /* and also for all initialized absolute symbols */
  dbuf_printf (ivalBuf, "%s", iComments2);
  dbuf_printf (ivalBuf, "; initialized absolute data\n");
  dbuf_printf (ivalBuf, "%s", iComments2);
  DBG_MSG ("pic14_constructAbsMap ---emit absolute symbols---");
  DBG_ENTRY ();
  for (addr = absmap_min; addr <= absmap_max; addr++)
    {
      set *aliases = hTabItemWithKey (absmap_ht, addr);
      if (aliases && elementsInSet (aliases))
        {
          symbol *sym, *isym = NULL;
          int size = 0, isize;
          DBG_MSG ("address 0x%04X has %d symbols", addr, elementsInSet (aliases));
          DBG_ENTRY ();
          /* Make sure there is no initialized value at this location! */
          for (sym = setFirstItem (aliases); sym; sym = setNextItem (aliases))
            {
              int ssize = getSize (sym->type) + sym->flexArrayLength;
              if (ssize > size)
                {
                  size = ssize;
                }
              if (sym->ival)
                {
                  if (isym)
                    {
                      /* We should check if both initializations are compatible */
                      DBG_MSG ("duplicated initialization for symbols %s and %s", isym->name, sym->name);
                      fprintf (stderr, "Duplicated initialization for symbols %s and %s", isym->name, sym->name);
                      /* Get the largest initialization */
                      if (isize >= ssize)
                        continue;
                    }
                  DBG_MSG ("initialized symbol %s size %d", sym->name, ssize);
                  isym = sym;
                  isize = ssize;
                }
            }                   // for

          if (isym)
            {
              /* emit the symbol label */
              pic14_printIvalLabel (ivalBuf, isym);
              /* emit the labels of the aliases */
              for (sym = setFirstItem (aliases); sym; sym = setNextItem (aliases))
                {
                  if (sym != isym)
                    {
                      DBG_MSG ("emit alias label %s", sym->rname);

                      /* print extra debug info if required */
                      pic14_emitDebugSym (ivalBuf, sym);

                      dbuf_printf (ivalBuf, "%s\n", sym->rname);
                      pic14_stringInSet (sym->rname, &emitted, true);
                    }
                }
              /* emit the value */
              resolveIvalSym (isym->ival, isym->type);
              pic14_printIval (isym, isym->type, isym->ival, ivalBuf, true);
              /* pad remaining bytes with zero */
              if ((size -= isize) > 0)
                pic14_printLiteral (ivalBuf, 0, size);

              /* if sym is a simple string and sym->ival is a string,
                 WE don't need it anymore */
              if (IS_ARRAY (isym->type) && IS_CHAR (isym->type->next) &&
                  IS_AST_SYM_VALUE (list2expr (isym->ival)) && list2val (isym->ival, true)->sym->isstrlit)
                {
                  freeStringSymbol (list2val (isym->ival, true)->sym);
                }

              DBG_EXIT ();
              continue;
            }

          DBG_MSG ("global definition UD_abs udata_ovr module %s address 0x%04X size %d", moduleName, addr, size);
          dbuf_printf (oBuf, "UD_abs_%s_%04x\tudata_ovr\t0x%04x\n", moduleName, addr, addr);
          for (sym = setFirstItem (aliases); sym; sym = setNextItem (aliases))
            {

              DBG_MSG ("adding symbol %s", sym->name);

              /* print extra debug info if required */
              pic14_emitDebugSym (oBuf, sym);

              /* emit STATUS as well as _STATUS, required for SFRs only */
              //dbuf_printf (oBuf, "%s\tres\t0\n", sym->name);
              dbuf_printf (oBuf, "%s\n", sym->rname);
              pic14_stringInSet (sym->rname, &emitted, true);
            }                   // for
          dbuf_printf (oBuf, "\tres\t%d\n", size);
          DBG_EXIT ();
        }                       // if
    }                           // for addr
  DBG_EXIT ();
}

/*-----------------------------------------------------------------*/
/* createInterruptVect - creates the interrupt vector              */
/*-----------------------------------------------------------------*/
static void
pic14createInterruptVect (struct dbuf_s *vBuf)
{
  mainf = newSymbol ("main", 0);
  mainf->block = 0;

  /* only if the main function exists */
  if (!(mainf = findSymWithLevel (SymbolTab, mainf)))
    {
      struct options *op = &options;
      if (!(op->cc_only || noAssemble))
        //      werror (E_NO_MAIN);
        fprintf (stderr, "WARNING: function 'main' undefined\n");
      return;
    }

  /* if the main is only a prototype ie. no body then do nothing */
  if (!IFFUNC_HASBODY (mainf->type))
    {
      /* if ! compile only then main function should be present */
      if (!(options.cc_only || noAssemble))
        //      werror (E_NO_MAIN);
        fprintf (stderr, "WARNING: function 'main' undefined\n");
      return;
    }

  dbuf_printf (vBuf, "%s", iComments2);
  dbuf_printf (vBuf, "; reset vector \n");
  dbuf_printf (vBuf, "%s", iComments2);
  // Lkr file should place section STARTUP at address 0x0, but does not ...
  dbuf_printf (vBuf, "STARTUP\t%s 0x%04X\n", CODE_NAME, options.code_loc);
  dbuf_printf (vBuf, "\tnop\n");        /* first location for used by incircuit debugger */
  dbuf_printf (vBuf, "\tpagesel __sdcc_gsinit_startup\n");
  dbuf_printf (vBuf, "\tgoto\t__sdcc_gsinit_startup\n");
  popGetExternal ("__sdcc_gsinit_startup", 0);
}


/*-----------------------------------------------------------------*/
/* initialComments - puts in some initial comments                 */
/*-----------------------------------------------------------------*/
static void
pic14initialComments (FILE * afile)
{
  initialComments (afile);
  fprintf (afile, "; PIC port for the 14-bit core\n");
  fprintf (afile, "%s", iComments2);
}

int
pic14_stringInSet (const char *str, set **world, int autoAdd)
{
  char *s;

  if (!str)
    return true;
  assert (world);

  for (s = setFirstItem (*world); s; s = setNextItem (*world))
    {
      /* found in set */
      if (0 == strcmp (s, str))
        {
          DBG_MSG ("symbol %s already emitted", str);
          return true;
        }
    }

  /* not found */
  if (autoAdd)
    {
      DBG_MSG ("symbol %s emitted", str);
      addSet (world, Safe_strdup (str));
    }
  else
    {
      DBG_MSG ("symbol %s not emitted", str);
    }
  return false;
}

static void
pic14printLocals (struct dbuf_s *oBuf)
{
  set *allregs[6] = { dynAllocRegs /*, dynStackRegs, dynProcessorRegs */ ,
    dynDirectRegs, dynDirectBitRegs /*, dynInternalRegs */
  };
  reg_info *reg;
  int i, is_first = true;
  static unsigned sectionNr = 0;
  const char *split_locals;

  split_locals = getenv ("SDCC_PIC14_SPLIT_LOCALS");

  DBG_MSG ("pic14printLocals ---start registers---");
  DBG_ENTRY ();
  /* emit all registers from all possible sets */
  for (i = 0; i < 6; i++)
    {
      if (allregs[i] == NULL)
        continue;

      for (reg = setFirstItem (allregs[i]); reg; reg = setNextItem (allregs[i]))
        {
          DBG_REG ("", reg);
          if (reg->isEmitted)
            continue;

          if (reg->wasUsed && !reg->isExtern)
            {
              if (!pic14_stringInSet (reg->name, &emitted, true))
                {
                  if (reg->isFixed)
                    {
                      if (reg->pc_type != PO_DIR)
                        {
                          // Should not happen, really...
                          assert (!"Compiler-assigned variables should not be pinned... This is a bug.");
                          dbuf_printf (oBuf, "UDL_abs_%s_%04x\tudata_ovr\t0x%04X\n%s\tres\t%d\n",
                                       moduleName, reg->address, reg->address, reg->name, reg->size);
                        }
                    }
                  else
                    {
                      if (split_locals != NULL)
                        {
                          // assign each local register into its own section
                          dbuf_printf (oBuf, "UDL_%s_%u\tudata\n%s\tres\t%d\n", moduleName, sectionNr++, reg->name, reg->size);
                        }
                      else
                        {
                          // group all local registers into a single section
                          // This should greatly improve BANKSEL generation...
                          if (is_first)
                            {
                              dbuf_printf (oBuf, "UDL_%s_%u\tudata\n", moduleName, sectionNr++);
                              is_first = false;
                            }
                          dbuf_printf (oBuf, "%s\tres\t%d\n", reg->name, reg->size);
                        }
                    }
                }
            }
          reg->isEmitted = true;
        }                       // for
    }                           // for
  DBG_EXIT ();
  DBG_MSG ("pic14printLocals ---end registers---");
}

/*-----------------------------------------------------------------*/
/* emitOverlay - will emit code for the overlay stuff              */
/*-----------------------------------------------------------------*/
static void
pic14emitOverlay (struct dbuf_s *aBuf)
{
  set *ovrset;

  /*  if (!elementsInSet (ovrSetSets)) */

  /* the hack below, fixes translates for devices which
   * only have udata_shr memory */
  dbuf_printf (aBuf, "%s\t%s\n", (elementsInSet (ovrSetSets) ? "" : ";"), port->mem.overlay_name);

  /* for each of the sets in the overlay segment do */
  for (ovrset = setFirstItem (ovrSetSets); ovrset; ovrset = setNextItem (ovrSetSets))
    {

      symbol *sym;

      if (elementsInSet (ovrset))
        {
          /* this dummy area is used to fool the assembler
             otherwise the assembler will append each of these
             declarations into one chunk and will not overlay
             sad but true */

          /* I don't think this applies to us. We are using gpasm.  CRF */

          dbuf_printf (aBuf, ";\t.area _DUMMY\n");
          /* output the area informtion */
          dbuf_printf (aBuf, ";\t.area\t%s\n", port->mem.overlay_name); /* MOF */
        }

      for (sym = setFirstItem (ovrset); sym; sym = setNextItem (ovrset))
        {

          /* if extern then do nothing */
          if (IS_EXTERN (sym->etype))
            continue;

          /* if allocation required check is needed
             then check if the symbol really requires
             allocation only for local variables */
          if (!IS_AGGREGATE (sym->type) && !(sym->_isparm && !IS_REGPARM (sym->etype)) && !sym->allocreq && sym->level)
            continue;

          /* if global variable & not static or extern
             and addPublics allowed then add it to the public set */
          if ((sym->_isparm && !IS_REGPARM (sym->etype)) && !IS_STATIC (sym->etype) && !IS_STATIC (sym->localof->etype))
            addToPublics (sym);

          /* if extern then do nothing or is a function
             then do nothing */
          if (IS_FUNC (sym->type))
            continue;

          /* if is has an absolute address then generate
             an equate for this no need to allocate space */
          if (SPEC_ABSA (sym->etype))
            {
              /* print extra debug info if required */
              pic14_emitDebugSym (aBuf, sym);

              dbuf_printf (aBuf, "%s\t=\t0x%04x\n", sym->rname, SPEC_ADDR (sym->etype));
            }
          else
            {
              int size = getSize (sym->type);

              if (size == 0)
                {
                  werrorfl (sym->fileDef, sym->lineDef, E_UNKNOWN_SIZE, sym->name);
                }

              /* print extra debug info if required */
              pic14_emitDebugSym (aBuf, sym);

              /* allocate space */
              dbuf_printf (aBuf, "%s:\n", sym->rname);
              dbuf_printf (aBuf, "\t.ds\t0x%04x\n", (unsigned int) getSize (sym->type) & 0xffff);
            }

        }
    }
}

static void
pic14_emitInterruptHandler (FILE *asmFile)
{
  if (pic14_hasInterrupt)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; interrupt and initialization code\n");
      fprintf (asmFile, "%s", iComments2);
      // Note - for mplink may have to enlarge section vectors in .lnk file
      // Note: Do NOT name this code_interrupt to avoid nameclashes with
      //       source files's code segment (interrupt.c -> code_interrupt)
      fprintf (asmFile, "c_interrupt\t%s\t0x%04X\n", CODE_NAME, options.code_loc+4);

      /* interrupt service routine */
      fprintf (asmFile, "__sdcc_interrupt:\n");
      copypCode (asmFile, 'I');
    }
}

/*-----------------------------------------------------------------*/
/* glue - the final glue that hold the whole thing together        */
/*-----------------------------------------------------------------*/
void
picglue (void)
{
  FILE *asmFile;
  struct dbuf_s ovrBuf;
  struct dbuf_s vBuf;

  /* initialize the debugging system */
  DBG_OPEN (pic14_options.debug_glue, dbg_info);

  dbuf_init (&ovrBuf, 4096);
  dbuf_init (&vBuf, 4096);

  pCodeInitRegisters ();

  /* check for main() */
  mainf = newSymbol ("main", 0);
  mainf->block = 0;
  mainf = findSymWithLevel (SymbolTab, mainf);

  if (!mainf || !IFFUNC_HASBODY (mainf->type))
    {
      /* main missing -- import stack from main module */
      //fprintf (stderr, "main() missing -- assuming we are NOT the main module\n");
      pic14_options.isLibrarySource = true;
    }

  /* At this point we've got all the code in the form of pCode structures */
  /* Now it needs to be rearranged into the order it should be placed in the */
  /* code space */

  movepBlock2Head ('P');        // Last
  movepBlock2Head (code->dbName);
  movepBlock2Head ('X');
  movepBlock2Head (statsg->dbName);     // First


  /* print the global struct definitions */
  if (options.debug)
    cdbStructBlock (0);

  /* do the overlay segments */
  pic14emitOverlay (&ovrBuf);

  /* PENDING: this isnt the best place but it will do */
  if (port->general.glue_up_main)
    {
      /* create the interrupt vector table */
      pic14createInterruptVect (&vBuf);
    }

  AnalyzepCode ('*');

  ReuseReg ();                  // ReuseReg where call tree permits

  InlinepCode ();

  AnalyzepCode ('*');

  if (options.debug)
    pcode_test ();


  /* now put it all together into the assembler file */
  /* create the assembler file name */

  if ((noAssemble || options.c1mode) && fullDstFileName)
    {
      SNPRINTF (buffer, sizeof (buffer), "%s", fullDstFileName);
    }
  else
    {
      SNPRINTF (buffer, sizeof (buffer), "%s.asm", dstFileName);
    }

  if (!(asmFile = fopen (buffer, "w")))
    {
      werror (E_OUTPUT_FILE_OPEN_ERR, buffer, strerror (errno));
      exit (1);
    }

  /* prepare statistics */
  resetpCodeStatistics ();

  /* initial comments */
  pic14initialComments (asmFile);

  /* print module name */
  fprintf (asmFile, "%s\t.file\t\"%s\"\n", options.debug ? "" : ";", fullSrcFileName);

  /* Let the port generate any global directives, etc. */
  if (port->genAssemblerPreamble)
    {
      port->genAssemblerPreamble (asmFile);
    }

  /* Put all variables into a cblock */
  AnalyzeBanking ();

  /* emit initialized data */
  showAllMemmaps (asmFile);

  /* print the locally defined variables in this module */
  writeUsedRegs (asmFile);

  /* create the overlay segments */
  fprintf (asmFile, "%s", iComments2);
  fprintf (asmFile, "; overlayable items in internal ram \n");
  fprintf (asmFile, "%s", iComments2);
  dbuf_write_and_destroy (&ovrBuf, asmFile);

  /* copy the interrupt vector table */
  if (mainf && IFFUNC_HASBODY (mainf->type))
    dbuf_write_and_destroy (&vBuf, asmFile);
  else
    dbuf_destroy (&vBuf);

  /* create interupt vector handler */
  pic14_emitInterruptHandler (asmFile);

  /* copy over code */
  fprintf (asmFile, "%s", iComments2);
  fprintf (asmFile, "; code\n");
  fprintf (asmFile, "%s", iComments2);
  fprintf (asmFile, "code_%s\t%s\n", moduleName, port->mem.code_name);

  /* unknown */
  copypCode (asmFile, 'X');

  /* _main function */
  copypCode (asmFile, 'M');

  /* other functions */
  copypCode (asmFile, code->dbName);

  /* unknown */
  copypCode (asmFile, 'P');

  dumppCodeStatistics (asmFile);

  fprintf (asmFile, "\tend\n");

  fclose (asmFile);

  DBG_CLOSE ();
}

/*
 * Deal with initializers.
 */


/*
 * Emit the section preamble, absolute location (if any) and
 * symbol name(s) for intialized data.
 * Set in_code to the address space of the symbol.
 */
static void
pic14_printIvalLabel (struct dbuf_s *oBuf, symbol *sym)
{
  char *segname, *label;
  static unsigned sectionNr = 0;

  wassertl (sym != NULL, "NULL symbol in call to printIvalLabel");

  in_code = false;

  // code or data space?
  if (IS_CODE (sym->etype))
    {
      label = "IDC";
      segname = "code";
      in_code = true;
    }
  else
    {
      label = "IDD";
      segname = "idata";
      in_code = false;
    }
  if (SPEC_ABSA (sym->etype))
    {
      // specify address for absolute symbols
      int addr = SPEC_ADDR (sym->etype);
      DBG_MSG ("printIvalLabel %s %s module %s section %u address 0x%04X name %s",
               label, segname, moduleName, sectionNr, addr, sym->rname);
      dbuf_printf (oBuf, "\n%s_abs_%s_%04x\t%s\t0x%04x\n", label, moduleName, addr, segname, addr);
    }
  else
    {
      DBG_MSG ("printIvalLabel %s %s module %s section %u name %s", label, segname, moduleName, sectionNr, sym->rname);
      dbuf_printf (oBuf, "\n%s_%s_%u\t%s\n", label, moduleName, sectionNr++, segname);
    }

  /* print extra debug info if required */
  pic14_emitDebugSym (oBuf, sym);

  dbuf_printf (oBuf, "%s\n", sym->rname);
  pic14_stringInSet (sym->rname, &emitted, true);
}

/*
 * Emit a literal value of the specified size.
 * Also used for zero-padding.
 */

static void
pic14_printLiteral (struct dbuf_s *oBuf, long lit, int size)
{
  int i;

  DBG_MSG ("printLiteral %ld 0x%lx size %d", lit, lit, size);

  if (!in_code)
    dbuf_printf (oBuf, "\tdb\t");

  for (i = 0; i < size; i++)
    {
      if (in_code)
        {
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", (int) (lit & 0xff));
          // dbuf_printf (oBuf, "\tretlw 0x00\n"); // conflict from merge of sf-patch-2991122 ?
        }
      else
        {
          dbuf_printf (oBuf, "%s0x%02x", (i == 0) ? "" : ", ", (int) (lit & 0xff));
        }
      lit >>= 8;
    }                           // for
  if (!in_code)
    dbuf_printf (oBuf, "\n");
}

/*----------------------------------------------------------------*/
/* pic14_printChar - formats and prints a UTF-8 character string  */
/*----------------------------------------------------------------*/
static void
pic14_printChar (struct dbuf_s *oBuf, const char *s, int plen)
{
  for (; plen--; ++s)
    {
      unsigned char c = *s;
      char ch = isprint (c) ? c : '.';
      if (in_code)
        dbuf_printf (oBuf, "\tretlw 0x%02x ; '%c'\n", c, ch);
      else
        dbuf_printf (oBuf, "\tdb\t0x%02x ; '%c'\n", c, ch);
    }
}

/*-------------------------------------------------------------------*/
/* pic14_printChar16 - formats and prints a UTF-16 character string  */
/*-------------------------------------------------------------------*/
static void
pic14_printChar16 (struct dbuf_s *oBuf, const TYPE_TARGET_UINT *s, int plen)
{
  for (; plen--; ++s)
    {
      if (in_code)
        {
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", *s & 0xff);
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", (*s >> 8) & 0xff);
        }
      else
        {
          dbuf_printf (oBuf, "\tdb\t0x%02x, 0x%02x\n", (unsigned int) (*s & 0xff), (unsigned int) ((*s >> 8) & 0xff));
        }
    }
}

/*-------------------------------------------------------------------*/
/* pic14_printChar32 - formats and prints a UTF-32 character string  */
/*-------------------------------------------------------------------*/
static void
pic14_printChar32 (struct dbuf_s *oBuf, const TYPE_TARGET_ULONG *s, int plen)
{
  for (; plen--; ++s)
    {
      if (in_code)
        {
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", *s & 0xff);
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", (*s >> 8) & 0xff);
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", (*s >> 16) & 0xff);
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", (*s >> 24) & 0xff);
        }
      else
        {
          dbuf_printf (oBuf, "\tdb\t0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                       (unsigned int) (*s & 0xff),
                       (unsigned int) ((*s >> 8) & 0xff),
                       (unsigned int) ((*s >> 16) & 0xff), (unsigned int) ((*s >> 24) & 0xff));
        }
    }
}

/*
 * Emit a label of the specified size and pointer type.
 */

static void
pic14_printLabel (struct dbuf_s *oBuf, const char *name, int size, int ptype)
{
  DBG_MSG ("printLabel name '%s' size %d ptype %d", name, size, ptype);
  wassertl (size >= 1 && size <= 4, "Unexpected size in printLabel");
  int tag = 0;

  if (size > 2)
    switch (ptype)
      {
      case POINTER:
      case FPOINTER:
      case GPOINTER:
        tag = GPTRTAG_DATA;
        break;
      case FUNCTION:
      case CPOINTER:
        tag = GPTRTAG_CODE;
        break;
      default:
        wassertl (0, "Unexpected pointer type in printLabel");
      }

  if (in_code)
    {
      if (size == 1)
        dbuf_printf (oBuf, "\tretlw %s\n", name);
      else
        dbuf_printf (oBuf, "\tretlw low(%s)\n", name);
      if (size > 1)
        dbuf_printf (oBuf, "\tretlw high(%s)\n", name);
      if (size > 2)
        dbuf_printf (oBuf, "\tretlw 0x%02x\n", tag);
      if (size > 3)
        dbuf_printf (oBuf, "\tretlw 0\n");
    }
  else
    {
      dbuf_printf (oBuf, "\tdb\t");
      if (size == 1)
        dbuf_printf (oBuf, "%s", name);
      else
        dbuf_printf (oBuf, "low(%s)", name);
      if (size > 1)
        dbuf_printf (oBuf, ", high(%s)", name);
      if (size > 2)
        dbuf_printf (oBuf, ", 0x%02x", tag);
      if (size > 3)
        dbuf_printf (oBuf, ", 0");
      dbuf_printf (oBuf, "\n");
    }
}

/*
 * Emit a label for int/char.
 */

static void
pic14_printTypeLabel (symbol *sym, sym_link *type, value *val, struct dbuf_s *oBuf)
{
  int size = getSize (type);
  DBG_MSG ("printTypeLabel size %d", size);

  switch (size)
    {
    case 1:
      pic14_printLabel (oBuf, val->name, 1, 0);
      break;
    case 2:
      pic14_printLabel (oBuf, val->name, 2, 0);
      break;
    case 4:
      pic14_printLabel (oBuf, val->name, 4, PTR_TYPE (SPEC_OCLS (val->sym->etype)));
      break;
    default:
      wassertl (0, "Attempting to initialize integer of non-handled size.");
    }
}

/*
 * Emit a label for a function pointer.
 */

static void
pic14_printFuncPtrLabel (sym_link *type, value *val, struct dbuf_s *oBuf)
{
  char *name;
  int size;

  /* now generate the name */
  if (!val->sym)
    name = val->name;
  else
    name = val->sym->rname;

  size = getSize (type);
  DBG_MSG ("printFuncPtrLabel name '%s' size %d", name, size);

  switch (size)
    {
    case 2:
      pic14_printLabel (oBuf, name, 2, 0);
      break;
    default:
      wassertl (0, "Invalid function pointer size.");
    }
}

/*
 * Emit a label for a character pointer.
 */

static void
pic14_printCharPtrLabel (symbol *sym, sym_link *type, value *val, struct dbuf_s *oBuf)
{
  int size = getSize (type);
  int ptype;
  DBG_MSG ("printCharPtrLabel size %d", size);

  switch (size)
    {
    case 2:
      pic14_printLabel (oBuf, val->name, 2, 0);
      break;
    case 3:
      if (IS_PTR (val->type))
        {
          ptype = DCL_TYPE (val->type);
        }
      else
        {
          ptype = PTR_TYPE (SPEC_OCLS (val->etype));
        }
      if (val->sym && val->sym->isstrlit)
        {
          // this is a literal string
          ptype = CPOINTER;
        }
      pic14_printLabel (oBuf, val->name, 3, ptype);
      break;
    default:
      wassertl (0, "Invalid char pointer size.");
    }
}

/*
 * Emit a label for a pointer.
 */

static void
pic14_printPtrLabel (symbol *sym, sym_link *type, value *val, struct dbuf_s *oBuf)
{
  int size = getSize (type);
  int ptype;
  DBG_MSG ("printPtrLabel size %d", size);

  switch (size)
    {
    case 2:
      pic14_printLabel (oBuf, val->name, 2, 0);
      break;
    case 3:
      ptype = IS_PTR (val->type) ? DCL_TYPE (val->type) : PTR_TYPE (SPEC_OCLS (val->etype));
      pic14_printLabel (oBuf, val->name, 3, ptype);
      break;
    default:
      wassertl (0, "Invalid pointer size.");
    }
}

/*
 * Emit a literal value of the specified type.
 */

static void
pic14_printValue (struct dbuf_s *oBuf, sym_link *type, value *val)
{
  int size = getSize (type);
  int tag = 0;
  char buf[100] = "";

  wassertl (size >= 1 && size <= 4, "Unexpected size in printValue");

  if (val)
    {
      if (IS_FLOAT (val->type))
        {
          SNPRINTF (buf, sizeof (buf), "\t; % e", floatFromVal (val));
        }
      else if (IS_INTEGRAL (val->type))
        {
          unsigned long ulVal = ulFromVal (val);
          switch (size)
            {
            case 1:
              if (isalnum ((int) ulVal))
                {
                  if (IS_UNSIGNED (val->type))
                    SNPRINTF (buf, sizeof (buf), "\t; %u\t'%c'", (unsigned int) ulVal, (int) ulVal);
                  else
                    SNPRINTF (buf, sizeof (buf), "\t; % d\t'%c'", (int) ulVal, (int) ulVal);
                  break;
                }
            case 2:
              if (IS_UNSIGNED (val->type))
                SNPRINTF (buf, sizeof (buf), "\t; %u", (unsigned int) ulVal);
              else
                SNPRINTF (buf, sizeof (buf), "\t; % d", (int) ulVal);
              break;
            case 4:
              if (IS_UNSIGNED (val->type))
                SNPRINTF (buf, sizeof (buf), "\t; %lu", (unsigned long) ulVal);
              else
                SNPRINTF (buf, sizeof (buf), "\t; % ld", (long) ulVal);
              break;
            }
        }
    }

  if (size == 3)
    {
      int ptype = -1;
      if (IS_FUNCPTR (val->type))
        ptype = DCL_TYPE (val->type->next);
      else if (IS_GENPTR (val->type))
        tag = byteOfVal (val, 2);
      else if (IS_PTR (val->type))
        ptype = DCL_TYPE (val->type);
      switch (ptype)
        {
        case -1:
          break;
        case POINTER:
        case FPOINTER:
          tag = GPTRTAG_DATA;
          break;
        case FUNCTION:
        case CPOINTER:
          tag = GPTRTAG_CODE;
          break;
        default:
          wassertl (0, "Unexpected pointer type in printValue");
        }
    }

  if (in_code)
    {
      dbuf_printf (oBuf, "\tretlw 0x%02x%s\n", val ? byteOfVal (val, 0) : 0, buf);
      if (size > 1)
        dbuf_printf (oBuf, "\tretlw 0x%02x\n", byteOfVal (val, 1));
      if (size > 2)
        if (size == 3)
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", tag);
        else
          dbuf_printf (oBuf, "\tretlw 0x%02x\n", byteOfVal (val, 2));
      if (size > 3)
        dbuf_printf (oBuf, "\tretlw 0x%02x\n", byteOfVal (val, 3));
    }
  else
    {
      dbuf_printf (oBuf, "\tdb\t");
      dbuf_printf (oBuf, "0x%02x", val ? byteOfVal (val, 0) : 0);
      if (size > 1)
        dbuf_printf (oBuf, ", 0x%02x", byteOfVal (val, 1));
      if (size > 2)
        if (size == 3)
          dbuf_printf (oBuf, ", 0x%02x", tag);
        else
          dbuf_printf (oBuf, ", 0x%02x", byteOfVal (val, 2));
      if (size > 3)
        dbuf_printf (oBuf, ", 0x%02x", byteOfVal (val, 3));
      dbuf_printf (oBuf, "%s\n", buf);
    }
}

/*
 * Emit a literal value for int/char.
 */

static void
pic14_printTypeLiteral (symbol *sym, sym_link *type, value *val, struct dbuf_s *oBuf)
{
  int size = getSize (type);
  DBG_MSG ("printTypeLiteral size %d", size);

  switch (size)
    {
    case 1:
    case 2:
    case 4:
      pic14_printValue (oBuf, type, val);
      break;
    default:
      wassertl (0, "Attempting to initialize integer of non-handled size.");
    }
}

/*
 * Emit a literal value for a character pointer.
 */

static void
pic14_printCharPtrLiteral (symbol *sym, sym_link *type, value *val, struct dbuf_s *oBuf)
{
  int size = getSize (type);
  DBG_MSG ("printCharPtrLiteral size %d", size);

  // these are literals assigned to pointers
  switch (size)
    {
    case 2:
      pic14_printValue (oBuf, type, val);
      break;
    case 3:
      if (IS_GENPTR (type) && GPTRSIZE > FARPTRSIZE && floatFromVal (val) != 0)
        {
          if (!IS_PTR (val->type) && !IS_FUNC (val->type))
            {
              // non-zero mcs51 generic pointer
              werrorfl (sym->fileDef, sym->lineDef, W_LITERAL_GENERIC);
            }
        }
      pic14_printValue (oBuf, type, val);
      break;
    default:
      wassertl (0, "Attempting to initialize character pointer of non-handled size.");
    }
}

/*
 * Emit a literal value for a pointer.
 */

static void
pic14_printPtrLiteral (symbol *sym, sym_link *type, value *val, struct dbuf_s *oBuf)
{
  int size = getSize (type);
  DBG_MSG ("printPtrLiteral size %d", size);

  switch (size)
    {
    case 2:
    case 3:
      pic14_printValue (oBuf, type, val);
      break;
    default:
      wassertl (0, "Attempting to initialize pointer of non-handled size.");
    }
}

/*-----------------------------------------------------------------*/
/* pic14_printIvalType - generates ival for int/char               */
/*-----------------------------------------------------------------*/
static void
pic14_printIvalType (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf)
{
  value *val;
  DBG_MSG ("printIvalType entry");

  /* if initList is deep */
  if (ilist && (ilist->type == INIT_DEEP))
    ilist = ilist->init.deep;

  if (!(val = list2val (ilist, false)))
    {
      if (!!(val = initPointer (ilist, type, 0)))
        {
          if (val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
            {
              DBG_VALUE ("initPointer", val);
              pic14_printTypeLabel (sym, type, val, oBuf);
              return;
            }
        }
      else
        {
          DBG_MSG ("ERROR: constant expected");
          werrorfl (ilist->filename, ilist->lineno, E_CONST_EXPECTED);
          val = constCharVal (0);
        }
    }
  DBG_VALUE ("value", val);

  if (val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
    {
      DBG_MSG ("ERROR: constant expected");
      werrorfl (ilist->filename, ilist->lineno, E_CONST_EXPECTED);
      val = constCharVal (0);
    }

  /* check if the literal value is within bounds */
  if (checkConstantRange (type, val->etype, '=', false) == CCR_OVL)
    {
      DBG_MSG ("WARNING: literal overflow");
      werror (W_LIT_OVERFLOW);
    }

  if (val->type != type)
    {
      val = valCastLiteral (type, floatFromVal (val), (TYPE_TARGET_ULONGLONG) ullFromVal (val));
      DBG_VALUE ("casted value", val);
    }

  pic14_printTypeLiteral (sym, type, val, oBuf);
}

/*-----------------------------------------------------------------*/
/* pic14_printIvalBitFields - generate initializer for bitfields   */
/*-----------------------------------------------------------------*/
static void
pic14_printIvalBitFields (symbol **sym, initList **ilist, struct dbuf_s *oBuf)
{
  symbol *lsym = *sym;
  initList *lilist = *ilist;
  unsigned long ival = 0;
  unsigned size = 0;
  unsigned bit_start = 0;

  while (lsym && IS_BITFIELD (lsym->type))
    {
      unsigned bit_length = SPEC_BLEN (lsym->etype);
      if (0 == bit_length)
        {
          /* bit-field structure member with a width of 0 */
          DBG_MSG ("found zero-width bitfield %s: finish.", lsym->name);
          lsym = lsym->next;
          break;
        }
      else if (!SPEC_BUNNAMED (lsym->etype))
        {
          /* not an unnamed bit-field structure member */
          value *val = list2val (lilist, true);

          if (val && val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
            {
              DBG_MSG ("non-constant initializer for bitfield %s: initializing to 0.", lsym->name);
              werrorfl (lilist->filename, lilist->lineno, E_CONST_EXPECTED);
              val = constCharVal (0);
            }
          if (size)
            {
              if (bit_length > 8)
                size += (bit_length + 7) / 8;
            }
          else
            size = (bit_length + 7) / 8;

          /* check if the literal value is within bounds */
          if (val && checkConstantRange (lsym->etype, val->etype, '=', false) == CCR_OVL)
            {
              DBG_MSG ("initializer overflow for bitfield %s: ignoring.", lsym->name);
              werror (W_LIT_OVERFLOW);
            }

          ival |= (ulFromVal (val) & ((1ul << bit_length) - 1ul)) << bit_start;
          DBG_MSG ("(bitfield member) %s value 0x%02lx (%d bits, starting at %d) new value 0x%02lx",
                   lsym->name, ulFromVal (val), bit_length, bit_start, ival);
          lilist = lilist ? lilist->next : NULL;
        }
      bit_start += bit_length;
      lsym = lsym->next;
      if (lsym && IS_BITFIELD (lsym->type) && (0 == SPEC_BSTR (lsym->etype)))
        {
          /* a new integer */
          DBG_MSG ("found new bitfield %s: finish.", lsym->name);
          break;
        }
    }

  DBG_MSG ("bitfield member value %ld size %d", ival, size);
  switch (size)
    {
    case 0:
      break;
    case 1:
    case 2:
    case 4:
      pic14_printLiteral (oBuf, ival, size);
      break;
    default:
      wassertl (0, "Unexpected bitfield length.");
    }
  *sym = lsym;
  *ilist = lilist;
}

/*-----------------------------------------------------------------*/
/* pic14_printIvalStruct - generates initial value for structures  */
/*-----------------------------------------------------------------*/
static void
pic14_printIvalStruct (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf)
{
  symbol *sflds;
  initList *iloop = NULL;
  unsigned int skip_holes = 0;
  int size = getSize (type);
  DBG_MSG ("printIvalStruct entry");

  sflds = SPEC_STRUCT (type)->fields;

  if (ilist)
    {
      if (ilist->type != INIT_DEEP)
        {
          DBG_MSG ("ERROR: initialization needs curly braces");
          werrorfl (sym->fileDef, sym->lineDef, E_INIT_STRUCT, sym->name);
          return;
        }

      iloop = ilist->init.deep;
    }

  if (SPEC_STRUCT (type)->type == UNION)
    {
      DBG_MSG ("(union, %d byte) handled below", size);
      /* skip past holes, print value */
      while (iloop && iloop->type == INIT_HOLE)
        {
          iloop = iloop->next;
          sflds = sflds->next;
        }
      pic14_printIval (sym, sflds->type, iloop, oBuf, true);
      /* pad out with zeros if necessary */
      size -= getSize (sflds->type);
      if (size > 0)
        {
          DBG_MSG ("union padding %d bytes", size);
          pic14_printLiteral (oBuf, 0, size);
        }
      /* advance past holes to find out if there were excess initializers */
      do
        {
          iloop = iloop ? iloop->next : NULL;
          sflds = sflds->next;
        }
      while (iloop && iloop->type == INIT_HOLE);
    }
  else
    {
      DBG_MSG ("(struct, %d byte) handled below", size);
      while (sflds)
        {
          unsigned int oldoffset = sflds->offset;

          if (IS_BITFIELD (sflds->type))
            pic14_printIvalBitFields (&sflds, &iloop, oBuf);
          else
            {
              DBG_MSG ("struct member");
              pic14_printIval (sym, sflds->type, iloop, oBuf, true);
              sflds = sflds->next;
              iloop = iloop ? iloop->next : NULL;
            }

          // Handle members from anonymous unions. Just a hack to fix bug #2643.
          while (sflds && sflds->offset == oldoffset)
            {
              /* FIXME: this breaks bug-1981238 (initialNoPad) */
              DBG_MSG ("WARNING: skip holes disabled");
              break;

              sflds = sflds->next;
              skip_holes++;
            }
        }

      while (skip_holes && iloop && iloop->type == INIT_HOLE)
        {
          skip_holes--;
          iloop = iloop ? iloop->next : NULL;
        }
    }

  if (iloop)
    {
      DBG_ILIST ("excess struct initializer ", iloop);
      werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "struct", sym->name);
    }
}

/*-----------------------------------------------------------------------*/
/* pic14_printIvalChar - generates initital value for character array    */
/*-----------------------------------------------------------------------*/
static int
pic14_printIvalChar (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf, const char *s, bool check)
{
  value *val;
  size_t size = DCL_ELEM (type);
  char *p;
  size_t asz;

  if (!s)
    {
      val = list2val (ilist, true);
      /* if the value is a character string  */
      if (IS_ARRAY (val->type) && IS_CHAR (val->etype))
        {
          if (!size)
            {
              /* we have not been given a size, but now we know it */
              size = strlen (SPEC_CVAL (val->etype).v_char) + 1;
              /* but first check, if it's a flexible array */
              if (sym && IS_STRUCT (sym->type))
                sym->flexArrayLength = size;
              else
                DCL_ELEM (type) = size;
            }

          if (check && DCL_ELEM (val->type) > size)
            werror (W_EXCESS_INITIALIZERS, "array of chars", sym->name, sym->lineDef);

          if (size > (asz = DCL_ELEM (val->type)) && !!(p = malloc (size)))
            {
              memcpy (p, SPEC_CVAL (val->etype).v_char, asz);
              memset (p + asz, 0x00, size - asz);
              pic14_printChar (oBuf, p, size);
              free (p);
            }
          else
            pic14_printChar (oBuf, SPEC_CVAL (val->etype).v_char, size);

          return 1;
        }
      else
        return 0;
    }
  else
    pic14_printChar (oBuf, s, strlen (s) + 1);
  return 1;
}

static size_t
strLen16 (const TYPE_TARGET_UINT * s)
{
  size_t l = 0;
  while (*s++)
    l++;

  return l;
}

/*-----------------------------------------------------------------------*/
/* pic14_printIvalChar16 - generates initital value for character array  */
/*-----------------------------------------------------------------------*/
static int
pic14_printIvalChar16 (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf, const TYPE_TARGET_UINT *s,
                       bool check)
{
  value *val;
  size_t size = DCL_ELEM (type);
  TYPE_TARGET_UINT *p;
  size_t asz;

  if (!s)
    {
      val = list2val (ilist, true);
      /* if the value is a character string  */
      if (IS_ARRAY (val->type) && IS_INT (val->etype) && IS_UNSIGNED (val->etype) && !IS_LONG (val->etype))
        {
          if (!size)
            {
              /* we have not been given a size, but now we know it */
              size = strLen16 (SPEC_CVAL (val->etype).v_char16) + 1;
              /* but first check, if it's a flexible array */
              if (sym && IS_STRUCT (sym->type))
                sym->flexArrayLength = size;
              else
                DCL_ELEM (type) = size;
            }

          if (check && DCL_ELEM (val->type) > size)
            werror (W_EXCESS_INITIALIZERS, "array of chars", sym->name, sym->lineDef);

          if (size > (asz = DCL_ELEM (val->type)) && !!(p = malloc (size * 2)))
            {
              memcpy (p, SPEC_CVAL (val->etype).v_char16, asz * 2);
              memset (p + asz, 0x00, size * 2 - asz * 2);
              pic14_printChar16 (oBuf, p, size);
              free (p);
            }
          else
            pic14_printChar16 (oBuf, SPEC_CVAL (val->etype).v_char16, size);

          return 1;
        }
      else
        return 0;
    }
  else
    pic14_printChar16 (oBuf, s, strLen16 (s) + 1);
  return 1;
}

static size_t
strLen32 (const TYPE_TARGET_ULONG * s)
{
  size_t l = 0;
  while (*s++)
    l++;
  return l;
}

/*-----------------------------------------------------------------------*/
/* pic14_printIvalChar32 - generates initital value for character array  */
/*-----------------------------------------------------------------------*/
static int
pic14_printIvalChar32 (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf, const TYPE_TARGET_ULONG *s, bool check)
{
  value *val;
  size_t size = DCL_ELEM (type);
  TYPE_TARGET_ULONG *p;
  size_t asz;

  if (!s)
    {
      val = list2val (ilist, true);
      /* if the value is a character string  */
      if (IS_ARRAY (val->type) && IS_INT (val->etype) && IS_UNSIGNED (val->etype) && IS_LONG (val->etype))
        {
          if (!size)
            {
              /* we have not been given a size, but now we know it */
              size = strLen32 (SPEC_CVAL (val->etype).v_char32) + 1;
              /* but first check, if it's a flexible array */
              if (sym && IS_STRUCT (sym->type))
                sym->flexArrayLength = size;
              else
                DCL_ELEM (type) = size;
            }

          if (check && DCL_ELEM (val->type) > size)
            werror (W_EXCESS_INITIALIZERS, "array of chars", sym->name, sym->lineDef);

          if (size > (asz = DCL_ELEM (val->type)) && !!(p = malloc (size * 4)))
            {
              memcpy (p, SPEC_CVAL (val->etype).v_char32, asz * 4);
              memset (p + asz, 0x00, size * 4 - asz * 4);
              pic14_printChar32 (oBuf, p, size);
              free (p);
            }
          else
            pic14_printChar32 (oBuf, SPEC_CVAL (val->etype).v_char32, size);

          return 1;
        }
      else
        return 0;
    }
  else
    pic14_printChar32 (oBuf, s, strLen32 (s) + 1);
  return 1;
}

/*---------------------------------------------------------------------------------*/
/* pic14_printIvalCharArray - generates code for character array initialization    */
/* returns true if ilist is a (valid or invalid) character array, false otherwise  */
/*---------------------------------------------------------------------------------*/
static bool
pic14_printIvalCharArray (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf, bool check)
{
  value *val;

  if ((IS_CHAR (type->next) || (IS_INT (type->next) && IS_UNSIGNED (type->next))) && ilist && ilist->type == INIT_NODE)
    {
      DBG_MSG ("character array from list");
      val = list2val (ilist, true);
      if (!val)
        {
          DBG_MSG ("ERROR: initialization needs curly braces");
          werrorfl (ilist->filename, ilist->lineno, E_INIT_STRUCT, sym->name);
          return true;
        }
      if (!IS_LITERAL (val->etype))
        {
          DBG_MSG ("ERROR: initializer element is not a constant expression");
          werrorfl (ilist->filename, ilist->lineno, E_CONST_EXPECTED);
          return true;
        }
      if (IS_CHAR (type->next) && pic14_printIvalChar (sym, type, ilist, oBuf, SPEC_CVAL (sym->etype).v_char, check))
        {
          DBG_MSG ("UTF-8 character array");
          return true;
        }
      if (IS_INT (type->next) && IS_UNSIGNED (type->next))
        {
          if (!IS_LONG (type->next) && pic14_printIvalChar16 (sym, type, ilist, oBuf, SPEC_CVAL (sym->etype).v_char16, check))
            {
              DBG_MSG ("UTF-16 character array");
              return true;
            }
          if (IS_LONG (type->next) && pic14_printIvalChar32 (sym, type, ilist, oBuf, SPEC_CVAL (sym->etype).v_char32, check))
            {
              DBG_MSG ("UTF-32 character array");
              return true;
            }
        }
    }

  return false;
}

/*-----------------------------------------------------------------*/
/* pic14_printIvalArray - generates code for array initialization  */
/*-----------------------------------------------------------------*/
static void
pic14_printIvalArray (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf, bool check)
{
  initList *iloop;
  unsigned int size = 0;
  DBG_MSG ("printIvalArray entry");

  if (ilist)
    {
      /* take care of the special   case  */
      /* array of characters can be init  */
      /* by a string                      */

      /* char *p = "abc";                 */
      if (pic14_printIvalCharArray (sym, type, ilist, oBuf, check))
        return;

      /* char *p = {"abc"}; */
      if (ilist->type == INIT_DEEP && pic14_printIvalCharArray (sym, type, ilist->init.deep, oBuf, check))
        return;

      /* not the special case             */
      if (ilist->type != INIT_DEEP)
        {
          DBG_MSG ("ERROR: initialization needs curly braces");
          werrorfl (ilist->filename, ilist->lineno, E_INIT_STRUCT, sym->name);
          return;
        }

      for (iloop = ilist->init.deep; iloop; iloop = iloop->next)
        {
          if ((++size > DCL_ELEM (type)) && DCL_ELEM (type))
            {
              DBG_MSG ("ERROR: excess initializers");
              werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "array", sym->name);
              break;
            }
          pic14_printIval (sym, type->next, iloop, oBuf, true);
        }
    }

  if (DCL_ELEM (type))
    {
      // pad with zeros if needed
      if (size < DCL_ELEM (type))
        {
          size = (DCL_ELEM (type) - size) * getSize (type->next);
          DBG_MSG ("array padding %d bytes", size);
          pic14_printLiteral (oBuf, 0, size);
        }
    }
  else
    {
      /* we have not been given a size, but now we know it */
      /* but first check, if it's a flexible array */
      if (IS_STRUCT (sym->type))
        {
          sym->flexArrayLength = size * getSize (type->next);
          DBG_MSG ("flexArrayLength set to %d", sym->flexArrayLength);
        }
      else
        {
          DCL_ELEM (type) = size;
          DBG_MSG ("array size set to %d", size);
        }
    }
}

/*--------------------------------------------------------------------------*/
/* pic14_printIvalCharPtr - generates initial values for character pointers */
/*--------------------------------------------------------------------------*/
static int
pic14_printIvalCharPtr (symbol *sym, sym_link *type, value *val, struct dbuf_s *oBuf)
{
  char *p;
  DBG_MSG ("printIvalCharPtr entry");

  if (val && !!(p = (char *) malloc (strlen (val->name) + 1)))
    {
      strcpy (p, val->name);
      addSet (&ccpStr, p);
    }

  /* PENDING: this is _very_ mcs51 specific, including a magic
     number...
     It's also endian specific.
   */

  if (val->name && strlen (val->name))
    pic14_printCharPtrLabel (sym, type, val, oBuf);
  else
    pic14_printCharPtrLiteral (sym, type, val, oBuf);

#if 0
  if (!noInit && val->sym && val->sym->isstrlit && !isinSet (statsg->syms, val->sym))
    {
      addSet (&statsg->syms, val->sym);
    }
#endif
  return 1;
}

/*-----------------------------------------------------------------------*/
/* pic14_printIvalFuncPtr - generate initial value for function pointers */
/*-----------------------------------------------------------------------*/
static void
pic14_printIvalFuncPtr (sym_link *type, initList *ilist, struct dbuf_s *oBuf)
{
  value *val;
  DBG_MSG ("printIvalFuncPtr entry");

  if (ilist)
    val = list2val (ilist, true);
  else
    val = valCastLiteral (type, 0.0, 0);

  if (!val)
    {
      // an error has been thrown already
      val = constCharVal (0);
    }
  DBG_VALUE ("value", val);

  if (IS_LITERAL (val->etype))
    {
      if (compareType (type, val->type) == 0)
        {
          DBG_MSG ("ERROR:incompatible types");
          if (ilist)
            werrorfl (ilist->filename, ilist->lineno, E_INCOMPAT_TYPES);
          else
            werror (E_INCOMPAT_TYPES);
          printFromToType (val->type, type);
        }
      pic14_printIvalCharPtr (NULL, type, val, oBuf);
      return;
    }

  pic14_printFuncPtrLabel (type, val, oBuf);
}

/*-----------------------------------------------------------------*/
/* pic14_printIvalPtr - generates initial value for pointers       */
/*-----------------------------------------------------------------*/
static void
pic14_printIvalPtr (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf)
{
  value *val;
  DBG_MSG ("printIvalPtr entry");

  /* if deep then   */
  if (ilist && (ilist->type == INIT_DEEP))
    ilist = ilist->init.deep;

  /* function pointer     */
  if (IS_FUNC (type->next))
    {
      pic14_printIvalFuncPtr (type, ilist, oBuf);
      return;
    }

  if (!(val = initPointer (ilist, type, 1)))
    {
      DBG_MSG ("not a pointer value");
      return;
    }
  DBG_VALUE ("initPointer", val);

  /* if character pointer */
  if (IS_CHAR (type->next) || IS_INT (type->next) && IS_UNSIGNED (type->next))
    if (pic14_printIvalCharPtr (sym, type, val, oBuf))
      return;

  /* check the type      */
  if (compareType (type, val->type) == 0)
    {
      DBG_MSG ("WARNING: wrong initialization");
      wassert (ilist != NULL);
      werrorfl (ilist->filename, ilist->lineno, W_INIT_WRONG);
      printFromToType (val->type, type);
    }

  /* if val is literal */
  if (IS_LITERAL (val->etype))
    pic14_printPtrLiteral (sym, type, val, oBuf);
  else
    pic14_printPtrLabel (sym, type, val, oBuf);
}

/*-----------------------------------------------------------------*/
/* pic14_printIval - generates code for initial value              */
/*-----------------------------------------------------------------*/
static void
pic14_printIval (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf, bool check)
{
  DBG_MSG ("pic14_printIval symbol %p type %p ilist %p check %d", sym, type, ilist, check);
  DBG_ENTRY ();
  DBG_SYMBOL ("symbol", sym);
  DBG_TYPE ("type", type);
  DBG_ILIST ("ilist", ilist);

  /* Handle designated initializers */
  if (ilist && ilist->type == INIT_DEEP)
    {
      ilist = reorderIlist (type, ilist);
      DBG_ILIST ("ilist reordered", ilist);
    }

  /* If this is a hole, substitute an appropriate initializer. */
  if (ilist && ilist->type == INIT_HOLE)
    {
      if (IS_AGGREGATE (type))
        {
          ilist = newiList (INIT_DEEP, NULL);   /* init w/ {} */
        }
      else
        {
          ast *ast = newAst_VALUE (constVal ("0"));
          ast = decorateType (ast, RESULT_TYPE_NONE);
          ilist = newiList (INIT_NODE, ast);
        }
      DBG_ILIST ("ilist substituted", ilist);
    }

  /* if structure then    */
  if (IS_STRUCT (type))
    {
      pic14_printIvalStruct (sym, type, ilist, oBuf);
      DBG_EXIT ();
      return;
    }

  /* if this is an array  */
  if (IS_ARRAY (type))
    {
      pic14_printIvalArray (sym, type, ilist, oBuf, check);
      DBG_EXIT ();
      return;
    }

  if (ilist)
    {
      // not an aggregate, ilist must be a node
      if (ilist->type != INIT_NODE)
        {
          // or a 1-element list
          if (ilist->init.deep->next)
            {
              DBG_MSG ("WARNING: excess initializers");
              werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "scalar", sym->name);
            }
          else
            {
              ilist = ilist->init.deep;
            }
        }

      // and the type must match
      sym_link *itype = ilist->init.node->ftype;

      if (compareType (type, itype) == 0)
        {
          // special case for literal strings
          if (IS_ARRAY (itype) && IS_CHAR (getSpec (itype)) &&
              // which are really code pointers
              IS_CODEPTR (type))
            {
              // no sweat
            }
          else if (IS_CODEPTR (type) && IS_FUNC (type->next))   /* function pointer */
            {
              DBG_MSG ("ERROR: incompatible types");
              if (ilist)
                werrorfl (ilist->filename, ilist->lineno, E_INCOMPAT_TYPES);
              else
                werror (E_INCOMPAT_TYPES);
              printFromToType (itype, type->next);
            }
          else
            {
              DBG_MSG ("ERROR: assignment mismatch");
              werrorfl (ilist->filename, ilist->lineno, E_TYPE_MISMATCH, "assignment", " ");
              printFromToType (itype, type);
            }
        }
    }

  /* if this is a pointer */
  if (IS_PTR (type))
    {
      pic14_printIvalPtr (sym, type, ilist, oBuf);
      DBG_EXIT ();
      return;
    }

  /* if type is SPECIFIER */
  if (IS_SPEC (type))
    {
      pic14_printIvalType (sym, type, ilist, oBuf);
      DBG_EXIT ();
      return;
    }

  DBG_MSG ("ERROR: unhandled initialized type");
  wassertl (0, "Unhandled initialized type.");

  DBG_EXIT ();
}

/*-----------------------------------------------------------------*/
/* emitRegularMap - emit code for maps with no special cases       */
/*-----------------------------------------------------------------*/
static void
pic14_emitRegularMap (memmap *map, bool addPublics, bool arFlag)
{
  symbol *sym;

  if (!map)
    return;

  DBG_MEMMAP ("emitRegularMap", map);

  for (sym = setFirstItem (map->syms); sym; sym = setNextItem (map->syms))
    {
      DBG_SYMBOL ("symbol", sym);
      DBG_ENTRY ();

      if (SPEC_ABSA (sym->etype))
        {
          int addr = SPEC_ADDR (sym->etype);
  
          /* handle CONFIG words here */
          if (IS_CONFIG_ADDRESS (addr))
            {
              //fprintf( stderr, "%s: assignment to CONFIG@0x%x found\n", __FUNCTION__, addr );
              //fprintf( stderr, "ival: %p (0x%x)\n", sym->ival, (int)list2int( sym->ival ) );
              if (sym->ival)
                {
                  DBG_MSG ("config word addr 0x%04X value 0x%04X", addr, (int) list2int (sym->ival));
                  pic14_assignConfigWordValue (addr, (int) list2int (sym->ival));
                }
              else
                {
                  fprintf (stderr, "ERROR: Symbol %s, which is covering a __CONFIG word must be initialized!\n", sym->name);
                }
              DBG_EXIT ();
              continue;
            }
        }

      /* if allocation required check is needed
         then check if the symbol really requires
         allocation only for local variables */

      if (arFlag && !IS_AGGREGATE (sym->type) && !(sym->_isparm && !IS_REGPARM (sym->etype)) && !sym->allocreq && sym->level)
        {
          DBG_MSG ("skipped symbol arFlag set");
          DBG_EXIT ();
          continue;
        }

      /* for bitvar locals and parameters */
      if (!arFlag && !sym->allocreq && sym->level && !SPEC_ABSA (sym->etype))
        {
          DBG_MSG ("skipped symbol arFlag unset");
          DBG_EXIT ();
          continue;
        }

      /* if extern then add it into the extern list */
      if (IS_EXTERN (sym->etype) && !sym->ival)
        {
          if (sym->isref || sym->used)
            {
              addToExterns (sym);
            }
          else
            {
              DBG_MSG ("omitted unused extern");
            }
          DBG_EXIT ();
          continue;
        }

      /* if global variable & not static or extern
         and addPublics allowed then add it to the public set */
      if ((sym->level == 0 ||
           (sym->_isparm && !IS_REGPARM (sym->etype) && !IS_STATIC (sym->localof->etype))) &&
          addPublics &&
          !IS_STATIC (sym->etype) &&
          (IS_FUNC (sym->type) ? (sym->used || IFFUNC_HASBODY (sym->type)) : (!IS_EXTERN (sym->etype) || sym->ival)) &&
          !(IFFUNC_ISINLINE (sym->type) && !IS_STATIC (sym->etype) && !IS_EXTERN (sym->etype)))
        {
          addToPublics (sym);
        }

      /* if extern then do nothing or is a function
         then do nothing */
      if (IS_FUNC (sym->type) && !(sym->isitmp))
        {
          DBG_MSG ("skipped function");
          DBG_EXIT ();
          continue;
        }

      /* pic14 specific: static locals, uninitialized, in data space, that
       * require allocation but have not been allocated yet.
       * this happens if they are used only inside the ival.
       * allocate them now
       */
      if ((IS_STATIC (sym->etype) || sym->level != 0) && sym->allocreq && !sym->ival && !IS_CODE (sym->etype))
        {
          const char *name = sym->rname[0] ? sym->rname : sym->name;
          reg_info *reg = dirregWithName (name);
          DBG_MSG ("checking unallocated register %s", name);
          if (!reg)
            {
              DBG_MSG ("allocating register %s", name);
              allocDirReg (operandFromSymbol (sym));
              reg = dirregWithName (name);
            }
          if (reg && !reg->wasUsed)
            {
              DBG_MSG ("marking used register %s", name);
              reg->wasUsed = true;
            }
        }

      /* if it has an initial value then do it only if
         it is a global variable */
      if (sym->ival && (sym->level == 0 || IS_STATIC (sym->etype)) && !SPEC_ABSA (sym->etype))
        {
          DBG_MSG ("initializing");
          resolveIvalSym (sym->ival, sym->type);
          pic14_printIvalLabel (ivalBuf, sym);
          pic14_printIval (sym, sym->type, sym->ival, ivalBuf, true);
          dbuf_printf (ivalBuf, "\n");

          sym->ival = NULL;
        }

      /* if it has an absolute address then generate
         an equate for this no need to allocate space */
      else if (SPEC_ABSA (sym->etype) && !sym->ival)
        {
          DBG_MSG ("added to ABS not ival");
          addToAbsMap (sym);
        }
      else
        {
          int size = getSize (sym->type) + sym->flexArrayLength;
          if (size == 0)
            {
              DBG_MSG ("ERROR: zero size");
              werrorfl (sym->fileDef, sym->lineDef, E_UNKNOWN_SIZE, sym->name);
            }
          /* allocate space */
          if (SPEC_ABSA (sym->etype))
            {
              DBG_MSG ("added to ABS allocate");
              addToAbsMap (sym);
            }
          else if (!sym->islocal)
            {
              DBG_MSG ("allocate size %d", size);
              pic14_allocateSpace (sym, size);
            }
        }
      DBG_EXIT ();
    }
}

/*-----------------------------------------------------------------*/
/* emitStaticSeg - emitcode for the static segment                 */
/*-----------------------------------------------------------------*/
static void
pic14_emitStaticSeg (memmap *map, struct dbuf_s *oBuf)
{
  symbol *sym;
  set *tmpSet = NULL;

  DBG_MEMMAP ("emitStaticSeg", map);

  /* eliminate redundant __str_%d (generated in stringToSymbol(), SDCCast.c) */
  for (sym = setFirstItem (map->syms); sym; sym = setNextItem (map->syms))
    addSet (&tmpSet, sym);

  /* for all variables in this segment do */
  for (sym = setFirstItem (map->syms); sym; sym = setNextItem (map->syms))
    {
      DBG_SYMBOL ("symbol", sym);
      DBG_ENTRY ();

      if (SPEC_ABSA (sym->etype))
        {
          int addr = SPEC_ADDR (sym->etype);
  
          /* handle CONFIG words here */
          if (IS_CONFIG_ADDRESS (addr))
            {
              //fprintf( stderr, "%s: assignment to CONFIG@0x%x found\n", __FUNCTION__, addr );
              //fprintf( stderr, "ival: %p (0x%x)\n", sym->ival, (int)list2int( sym->ival ) );
              if (sym->ival)
                {
                  DBG_MSG ("config word addr 0x%04X value 0x%04X", addr, (int) list2int (sym->ival));
                  pic14_assignConfigWordValue (addr, (int) list2int (sym->ival));
                }
              else
                {
                  fprintf (stderr, "ERROR: Symbol %s, which is covering a __CONFIG word must be initialized!\n", sym->name);
                }
              DBG_EXIT ();
              continue;
            }
        }

      /* if it is "extern" then do nothing */
      if (IS_EXTERN (sym->etype) && !sym->ival)
        {
          if (sym->isref || sym->used)
            {
              addToExterns (sym);
            }
          else
            {
              DBG_MSG ("omitted unused extern");
            }
          DBG_EXIT ();
          continue;
        }

      /* eliminate redundant __str_%d (generated in stringToSymbol(), SDCCast.c) */
      if (!isinSet (tmpSet, sym))
        {
          const char *p;
          if (!ccpStr)
            {
              DBG_MSG ("skip ccpStr empty");
              DBG_EXIT ();
              continue;
            }
          for (p = setFirstItem (ccpStr); p; p = setNextItem (ccpStr))
            if (strcmp (p, sym->name) == 0)
              {
                DBG_MSG ("found in ccpStr");
                break;
              }
          if (!p)
            {
              DBG_MSG ("skip not found in ccpStr");
              DBG_EXIT ();
              continue;
            }
        }

      /* if it is not static add it to the public table */
      if (!IS_STATIC (sym->etype))
        {
          addToPublics (sym);
        }

      /* if it has an absolute address and no initializer */
      if (SPEC_ABSA (sym->etype) && !sym->ival)
        {
          DBG_MSG ("added to ABS not ival");
          addToAbsMap (sym);
        }
      else
        {
          int size = getSize (sym->type);

          if (size == 0)
            {
              DBG_MSG ("ERROR: zero size");
              werrorfl (sym->fileDef, sym->lineDef, E_UNKNOWN_SIZE, sym->name);
            }
          /* if it has an initial value */
          if (sym->ival)
            {
              if (SPEC_ABSA (sym->etype))
                {
                  DBG_MSG ("added to ABS ival");
                  addToAbsMap (sym);
                }
              else
                {
                  DBG_MSG ("initializing");
                  ++noAlloc;
                  resolveIvalSym (sym->ival, sym->type);
                  pic14_printIvalLabel (ivalBuf, sym);
                  pic14_printIval (sym, sym->type, sym->ival, ivalBuf, true);
                  --noAlloc;
                  /* if sym is a simple string and sym->ival is a string,
                     WE don't need it anymore */
                  if (IS_ARRAY (sym->type) && IS_CHAR (sym->type->next) &&
                      IS_AST_SYM_VALUE (list2expr (sym->ival)) && list2val (sym->ival, true)->sym->isstrlit)
                    {
                      DBG_MSG ("free string symbol");
                      freeStringSymbol (list2val (sym->ival, true)->sym);
                    }
                }
            }
          else
            {
              /* special case for character strings */
              if (IS_ARRAY (sym->type) &&
                  (IS_CHAR (sym->type->next) && SPEC_CVAL (sym->etype).v_char ||
                   IS_INT (sym->type->next) && !IS_LONG (sym->type->next) && SPEC_CVAL (sym->etype).v_char16 ||
                   IS_INT (sym->type->next) && IS_LONG (sym->type->next) && SPEC_CVAL (sym->etype).v_char32))
                {
                  pic14_printIvalLabel (ivalBuf, sym);
                  if (IS_CHAR (sym->type->next))
                    {
                      DBG_MSG ("array from string literal of %d UTF-8 characters", size);
                      pic14_printChar (ivalBuf, SPEC_CVAL (sym->etype).v_char, size);
                    }
                  else if (IS_INT (sym->type->next) && !IS_LONG (sym->type->next))
                    {
                      DBG_MSG ("array from string literal of %d UTF-16 characters", size);
                      pic14_printChar16 (ivalBuf, SPEC_CVAL (sym->etype).v_char16, size / 2);
                    }
                  else if (IS_INT (sym->type->next) && IS_LONG (sym->type->next))
                    {
                      DBG_MSG ("array from string literal of %d UTF-32 characters", size);
                      pic14_printChar32 (ivalBuf, SPEC_CVAL (sym->etype).v_char32, size / 4);
                    }
                  else
                    wassert (0);
                }
              else
                {
                  DBG_MSG ("allocate size %d", size);
                  pic14_allocateSpace (sym, size);
                }
            }
        }
      DBG_EXIT ();
    }

  if (tmpSet)
    deleteSet (&tmpSet);
  if (ccpStr)
    {
      char *p;
      for (p = setFirstItem (ccpStr); p; p = setNextItem (ccpStr))
        if (p)
          free (p);
      deleteSet (&ccpStr);
    }
}

/*
 * Declare a public/extern symbol if not already done.
 */

static void
pic14_declare (struct dbuf_s *oBuf, const char *type, symbol *sym)
{
  char *str = sym->rname;
  char *s;

  for (s = setFirstItem (declared); s; s = setNextItem (declared))
    {
      /* found in set */
      if (0 == strcmp (s, str))
        {
          DBG_MSG ("symbol %s already declared", str);
          return;
        }
    }

  /* not found */
  DBG_MSG ("symbol %s declared %s", str, type);
  addSet (&declared, Safe_strdup (str));
  dbuf_printf (oBuf, "\t%s\t%s\n", type, str);
}

/*-----------------------------------------------------------------*/
/* printPublics - generates .global for publics                    */
/*-----------------------------------------------------------------*/
static void
pic14_printPublics (struct dbuf_s *gloBuf)
{
  symbol *sym;

  DBG_MSG ("public symbols: %d symbols", elementsInSet (publics));
  for (sym = setFirstItem (publics); sym; sym = setNextItem (publics))
    pic14_declare (gloBuf, "global", sym);
}

/*-----------------------------------------------------------------*/
/* printExterns - generates .global for externs                    */
/*-----------------------------------------------------------------*/
static void
pic14_printExterns (struct dbuf_s *extBuf)
{
  symbol *sym;

  DBG_MSG ("extern symbols: %d symbols", elementsInSet (externs));
  for (sym = setFirstItem (externs); sym; sym = setNextItem (externs))
    pic14_declare (extBuf, "extern", sym);
}

/*
 * Iterate over all memmaps and emit their contents (attributes, symbols).
 */
static void
showAllMemmaps (FILE *of)
{
  // trying to keep things as similar as possible to SDCCglue.c
  namedspacemap *nm;
  int publicsfr = true;

  DBG_MSG ("---begin memmaps---");
  if (!extBuf)
    extBuf = dbuf_new (1024);
  if (!gloBuf)
    gloBuf = dbuf_new (1024);
  if (!gloDefBuf)
    gloDefBuf = dbuf_new (1024);
  if (!ivalBuf)
    ivalBuf = dbuf_new (1024);
  if (!locBuf)
    locBuf = dbuf_new (1024);

  dbuf_printf (extBuf, "%s; external declarations\n%s", iComments2, iComments2);
  dbuf_printf (gloBuf, "%s; global declarations\n%s", iComments2, iComments2);
  dbuf_printf (gloDefBuf, "%s; global definitions\n%s", iComments2, iComments2);
  dbuf_printf (ivalBuf, "%s; initialized data\n%s", iComments2, iComments2);
  dbuf_printf (locBuf, "%s; compiler-defined variables\n%s", iComments2, iComments2);

//    inInitMode++;
  /* no special considerations for the following
     data, idata & bit & xdata */
  pic14_emitRegularMap (data, true, true);
  pic14_emitRegularMap (initialized, true, true);
  for (nm = namedspacemaps; nm; nm = nm->next)
    {
      DBG_MSG ("named space map %p sname '%s' dbName '%c' is_const %u: %d symbols",
               nm, nm->map->sname, nm->map->dbName, nm->is_const, nm->map->syms ? elementsInSet (nm->map->syms) : 0);
      if (nm->is_const)
        pic14_emitStaticSeg (nm->map, &nm->map->oBuf);
      else
        pic14_emitRegularMap (nm->map, true, true);
    }
  pic14_emitRegularMap (idata, true, true);
  pic14_emitRegularMap (d_abs, true, true);
  pic14_emitRegularMap (i_abs, true, true);
  pic14_emitRegularMap (bit, true, true);
  pic14_emitRegularMap (pdata, true, true);
  pic14_emitRegularMap (xdata, true, true);
  pic14_emitRegularMap (x_abs, true, true);
  if (port->genXINIT)
    pic14_emitRegularMap (xidata, true, true);
  pic14_emitRegularMap (sfr, publicsfr, false);
  pic14_emitRegularMap (sfrbit, publicsfr, false);
  pic14_emitRegularMap (home, true, false);

  pic14_emitStaticSeg (statsg, &code->oBuf);

  if (port->genXINIT)
    pic14_emitStaticSeg (xinit, &code->oBuf);
  if (initializer)
    pic14_emitStaticSeg (initializer, &code->oBuf);
  pic14_emitStaticSeg (c_abs, &code->oBuf);

  pic14_constructAbsMap (gloDefBuf);

  // must be the last one, so externs used in ivals get marked as isref
  pic14_emitRegularMap (code, true, false);
//    inInitMode--;

  pic14_printPublics (gloBuf);
  pic14_printExterns (extBuf);

  emitPseudoStack (gloBuf, extBuf);
  pic14printLocals (locBuf);
  pic14_emitConfigWord (of);    // must be done after all the rest

  dbuf_write_and_destroy (extBuf, of);
  dbuf_write_and_destroy (gloBuf, of);
  dbuf_write_and_destroy (gloDefBuf, of);
  dbuf_write_and_destroy (locBuf, of);
  dbuf_write_and_destroy (ivalBuf, of);

  extBuf = gloBuf = gloDefBuf = ivalBuf = locBuf = NULL;
}
