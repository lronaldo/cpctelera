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

/*
 * Imports
 */
extern set *publics;
extern set *externs;
extern symbol *mainf;
extern struct dbuf_s *codeOutBuf;

extern void initialComments (FILE *afile);
extern operand *operandFromAst (ast *tree, int lvl);
extern value *initPointer (initList *ilist, sym_link *toType);


set *pic14_localFunctions = NULL;
int pic14_hasInterrupt = 0;             // Indicates whether to emit interrupt handler or not

int pic14_stringInSet(const char *str, set **world, int autoAdd);


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
static struct dbuf_s *ivalBuf, *extBuf, *gloBuf, *gloDefBuf;

static set *emitted = NULL;

static void showAllMemmaps (FILE *of); // XXX: emits initialized symbols

static void
emitPseudoStack(struct dbuf_s *oBuf, struct dbuf_s *oBufExt)
{
    int shared, low, high, size, i;

    /* also emit STK symbols
     * XXX: This is ugly and fails as soon as devices start to get
     *      differently sized sharebanks, since STK12 will be
     *      required by larger devices but only up to STK03 might
     *      be defined using smaller devices. */
    shared = pic14_getSharedStack(&low, &high, &size);
    if (!pic14_options.isLibrarySource)
    {
        dbuf_printf (oBuf, "\n");
        dbuf_printf (oBuf, "\tglobal PSAVE\n");
        dbuf_printf (oBuf, "\tglobal SSAVE\n");
        dbuf_printf (oBuf, "\tglobal WSAVE\n");
        for (i = size - 4; i >= 0; i--) {
            dbuf_printf (oBuf, "\tglobal STK%02d\n", i);
        } // for i
        dbuf_printf (oBuf, "\n");

        // 16f84 has no SHAREBANK (in linkerscript) but memory aliased in two
        // banks, sigh...
        if (1 || !shared) {
            // for single banked devices: use normal, "banked" RAM
            dbuf_printf (oBuf, "sharebank udata_ovr 0x%04X\n", low);
        } else {
            // for devices with at least two banks, require a sharebank section
            dbuf_printf (oBuf, "sharebank udata_shr\n");
        }
        dbuf_printf (oBuf, "PSAVE\tres 1\n");
        dbuf_printf (oBuf, "SSAVE\tres 1\n");
        dbuf_printf (oBuf, "WSAVE\tres 1\n"); // WSAVE *must* be in sharebank (IRQ handlers)
        /* fill rest of sharebank with stack STKxx .. STK00 */
        for (i = size - 4; i >= 0; i--) {
            dbuf_printf (oBuf, "STK%02d\tres 1\n", i);
        } // for i
    } else {
        /* declare STKxx as extern for all files
         * except the one containing main() */
        dbuf_printf (oBufExt, "\n");
        dbuf_printf (oBufExt, "\textern PSAVE\n");
        dbuf_printf (oBufExt, "\textern SSAVE\n");
        dbuf_printf (oBufExt, "\textern WSAVE\n");
        for (i = size - 4; i >= 0; i--) {
            char buffer[128];
            SNPRINTF(&buffer[0], 127, "STK%02d", i);
            dbuf_printf (oBufExt, "\textern %s\n", &buffer[0]);
            pic14_stringInSet(&buffer[0], &emitted, 1);
        } // for i
    }
    dbuf_printf (oBuf, "\n");
}

static int
emitIfNew(struct dbuf_s *oBuf, set **emitted, const char *fmt,
        const char *name)
{
    int wasPresent = pic14_stringInSet(name, emitted, 1);

    if (!wasPresent) {
        dbuf_printf (oBuf, fmt, name);
    } // if
    return (!wasPresent);
}

static void
pic14_constructAbsMap (struct dbuf_s *oBuf, struct dbuf_s *gloBuf)
{
  memmap *maps[] = { data, sfr, NULL };
  int i;
  hTab *ht = NULL;
  symbol *sym;
  set *aliases;
  int addr, min=-1, max=-1;
  unsigned int size;

  for (i=0; maps[i] != NULL; i++)
  {
    for (sym = (symbol *)setFirstItem (maps[i]->syms);
        sym; sym = setNextItem (maps[i]->syms))
    {
      if (IS_DEFINED_HERE(sym) && SPEC_ABSA(sym->etype))
      {
        addr = SPEC_ADDR(sym->etype);

        /* handle CONFIG words here */
        if (IS_CONFIG_ADDRESS( addr ))
        {
          //fprintf( stderr, "%s: assignment to CONFIG@0x%x found\n", __FUNCTION__, addr );
          //fprintf( stderr, "ival: %p (0x%x)\n", sym->ival, (int)list2int( sym->ival ) );
          if (sym->ival) {
            pic14_assignConfigWordValue( addr, (int)list2int( sym->ival ) );
          } else {
            fprintf( stderr, "ERROR: Symbol %s, which is covering a __CONFIG word must be initialized!\n", sym->name );
          }
          continue;
        }

        if (max == -1 || addr > max) max = addr;
        if (min == -1 || addr < min) min = addr;
        //fprintf (stderr, "%s: sym %s @ 0x%x\n", __FUNCTION__, sym->name, addr);
        aliases = hTabItemWithKey (ht, addr);
        if (aliases) {
          /* May not use addSetHead, as we cannot update the
           * list's head in the hastable `ht'. */
          addSet (&aliases, sym);
#if 0
          fprintf( stderr, "%s: now %d aliases for %s @ 0x%x\n",
              __FUNCTION__, elementsInSet(aliases), sym->name, addr);
#endif
        } else {
          addSet (&aliases, sym);
          hTabAddItem (&ht, addr, aliases);
        } // if
      } // if
    } // for sym
  } // for i

  /* now emit definitions for all absolute symbols */
  dbuf_printf (oBuf, "%s", iComments2);
  dbuf_printf (oBuf, "; absolute symbol definitions\n");
  dbuf_printf (oBuf, "%s", iComments2);
  for (addr=min; addr <= max; addr++)
  {
    size = 1;
    aliases = hTabItemWithKey (ht, addr);
    if (aliases && elementsInSet(aliases)) {
      /* Make sure there is no initialized value at this location! */
      for (sym = setFirstItem(aliases); sym; sym = setNextItem(aliases)) {
          if (sym->ival) break;
      } // for
      if (sym) continue;

      dbuf_printf (oBuf, "UD_abs_%s_%x\tudata_ovr\t0x%04x\n",
          moduleName, addr, addr);
      for (sym = setFirstItem (aliases); sym;
          sym = setNextItem (aliases))
      {
        if (getSize(sym->type) > size) {
          size = getSize(sym->type);
        }

        /* initialized values are handled somewhere else */
        if (sym->ival) continue;

        /* emit STATUS as well as _STATUS, required for SFRs only */
        //dbuf_printf (oBuf, "%s\tres\t0\n", sym->name);
        dbuf_printf (oBuf, "%s\n", sym->rname);

        if (IS_GLOBAL(sym) && !IS_STATIC(sym->etype)) {
            //emitIfNew(gloBuf, &emitted, "\tglobal\t%s\n", sym->name);
            emitIfNew(gloBuf, &emitted, "\tglobal\t%s\n", sym->rname);
        } // if
      } // for
      dbuf_printf (oBuf, "\tres\t%d\n", size);
    } // if
  } // for i
}

/*-----------------------------------------------------------------*/
/* createInterruptVect - creates the interrupt vector              */
/*-----------------------------------------------------------------*/
static void
pic14createInterruptVect (struct dbuf_s * vBuf)
{
        mainf = newSymbol ("main", 0);
        mainf->block = 0;

        /* only if the main function exists */
        if (!(mainf = findSymWithLevel (SymbolTab, mainf)))
        {
                struct options *op = &options;
                if (!(op->cc_only || noAssemble))
                        //      werror (E_NO_MAIN);
                        fprintf(stderr,"WARNING: function 'main' undefined\n");
                return;
        }

        /* if the main is only a prototype ie. no body then do nothing */
        if (!IFFUNC_HASBODY(mainf->type))
        {
                /* if ! compile only then main function should be present */
                if (!(options.cc_only || noAssemble))
                        //      werror (E_NO_MAIN);
                        fprintf(stderr,"WARNING: function 'main' undefined\n");
                return;
        }

        dbuf_printf (vBuf, "%s", iComments2);
        dbuf_printf (vBuf, "; reset vector \n");
        dbuf_printf (vBuf, "%s", iComments2);
        // Lkr file should place section STARTUP at address 0x0, but does not ...
        dbuf_printf (vBuf, "STARTUP\t%s 0x0000\n", CODE_NAME);
        dbuf_printf (vBuf, "\tnop\n"); /* first location for used by incircuit debugger */
        dbuf_printf (vBuf, "\tpagesel __sdcc_gsinit_startup\n");
        dbuf_printf (vBuf, "\tgoto\t__sdcc_gsinit_startup\n");
        popGetExternal("__sdcc_gsinit_startup", 0);
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
pic14_stringInSet(const char *str, set **world, int autoAdd)
{
  char *s;

  if (!str) return 1;
  assert(world);

  for (s = setFirstItem(*world); s; s = setNextItem(*world))
  {
    /* found in set */
    if (0 == strcmp(s, str)) return 1;
  }

  /* not found */
  if (autoAdd) addSet(world, Safe_strdup(str));
  return 0;
}

static void
pic14printLocals (struct dbuf_s *oBuf)
{
    set *allregs[6] = { dynAllocRegs/*, dynStackRegs, dynProcessorRegs*/,
        dynDirectRegs, dynDirectBitRegs/*, dynInternalRegs */ };
    reg_info *reg;
    int i, is_first = 1;
    static unsigned sectionNr = 0;

    /* emit all registers from all possible sets */
    for (i = 0; i < 6; i++) {
        if (allregs[i] == NULL) continue;

        for (reg = setFirstItem(allregs[i]); reg; reg = setNextItem(allregs[i])) {
            if (reg->isEmitted) continue;

            if (reg->wasUsed && !reg->isExtern) {
                if (!pic14_stringInSet(reg->name, &emitted, 1)) {
                    if (reg->isFixed) {
                        // Should not happen, really...
                        assert ( !"Compiler-assigned variables should not be pinned... This is a bug." );
                        dbuf_printf(oBuf, "UDL_%s_%u\tudata\t0x%04X\n%s\tres\t%d\n",
                                moduleName, sectionNr++, reg->address, reg->name, reg->size);
                    } else {
                        if (getenv("SDCC_PIC14_SPLIT_LOCALS")) {
                            // assign each local register into its own section
                            dbuf_printf(oBuf, "UDL_%s_%u\tudata\n%s\tres\t%d\n",
                                    moduleName, sectionNr++, reg->name, reg->size);
                        } else {
                            // group all local registers into a single section
                            // This should greatly improve BANKSEL generation...
                            if (is_first) {
                                dbuf_printf(oBuf, "UDL_%s_%u\tudata\n", moduleName, sectionNr++);
                                is_first = 0;
                            }
                            dbuf_printf(oBuf, "%s\tres\t%d\n", reg->name, reg->size);
                        }
                    }
                }
            }
            reg->isEmitted = 1;
        } // for
    } // for
}

/*-----------------------------------------------------------------*/
/* emitOverlay - will emit code for the overlay stuff              */
/*-----------------------------------------------------------------*/
static void
pic14emitOverlay (struct dbuf_s * aBuf)
{
        set *ovrset;

        /*  if (!elementsInSet (ovrSetSets))*/

        /* the hack below, fixes translates for devices which
        * only have udata_shr memory */
        dbuf_printf (aBuf, "%s\t%s\n",
                (elementsInSet(ovrSetSets)?"":";"),
                port->mem.overlay_name);

        /* for each of the sets in the overlay segment do */
        for (ovrset = setFirstItem (ovrSetSets); ovrset;
        ovrset = setNextItem (ovrSetSets))
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
                        dbuf_printf (aBuf, ";\t.area\t%s\n", port->mem.overlay_name);   /* MOF */
                }

                for (sym = setFirstItem (ovrset); sym;
                sym = setNextItem (ovrset))
                {

                        /* if extern then do nothing */
                        if (IS_EXTERN (sym->etype))
                                continue;

                                /* if allocation required check is needed
                                then check if the symbol really requires
                        allocation only for local variables */
                        if (!IS_AGGREGATE (sym->type) &&
                                !(sym->_isparm && !IS_REGPARM (sym->etype))
                                && !sym->allocreq && sym->level)
                                continue;

                                /* if global variable & not static or extern
                        and addPublics allowed then add it to the public set */
                        if ((sym->_isparm && !IS_REGPARM (sym->etype))
                                && !IS_STATIC (sym->etype))
                                addSetHead (&publics, sym);

                                /* if extern then do nothing or is a function
                        then do nothing */
                        if (IS_FUNC (sym->type))
                                continue;

                        /* print extra debug info if required */
                        if (options.debug || sym->level == 0)
                        {
                                if (!sym->level)
                                {               /* global */
                                        if (IS_STATIC (sym->etype))
                                                dbuf_printf (aBuf, "F%s_", moduleName); /* scope is file */
                                        else
                                                dbuf_printf (aBuf, "G_");       /* scope is global */
                                }
                                else
                                        /* symbol is local */
                                        dbuf_printf (aBuf, "L%s_",
                                        (sym->localof ? sym->localof->name : "-null-"));
                                dbuf_printf (aBuf, "%s_%d_%d", sym->name, sym->level, sym->block);
                        }

                        /* if is has an absolute address then generate
                        an equate for this no need to allocate space */
                        if (SPEC_ABSA (sym->etype))
                        {

                                if (options.debug || sym->level == 0)
                                        dbuf_printf (aBuf, " == 0x%04x\n", SPEC_ADDR (sym->etype));

                                dbuf_printf (aBuf, "%s\t=\t0x%04x\n",
                                        sym->rname,
                                        SPEC_ADDR (sym->etype));
                        }
                        else
                        {
                                if (options.debug || sym->level == 0)
                                        dbuf_printf (aBuf, "==.\n");

                                /* allocate space */
                                dbuf_printf (aBuf, "%s:\n", sym->rname);
                                dbuf_printf (aBuf, "\t.ds\t0x%04x\n", (unsigned int) getSize (sym->type) & 0xffff);
                        }

                }
        }
}


static void
pic14_emitInterruptHandler (FILE * asmFile)
{
        if (pic14_hasInterrupt)
        {

                fprintf (asmFile, "%s", iComments2);
                fprintf (asmFile, "; interrupt and initialization code\n");
                fprintf (asmFile, "%s", iComments2);
                // Note - for mplink may have to enlarge section vectors in .lnk file
                // Note: Do NOT name this code_interrupt to avoid nameclashes with
                //       source files's code segment (interrupt.c -> code_interrupt)
                fprintf (asmFile, "c_interrupt\t%s\t0x4\n", CODE_NAME);

                /* interrupt service routine */
                fprintf (asmFile, "__sdcc_interrupt\n");
                copypCode(asmFile, 'I');
        }
}

/*-----------------------------------------------------------------*/
/* glue - the final glue that hold the whole thing together        */
/*-----------------------------------------------------------------*/
void
picglue ()
{
        FILE *asmFile;
        struct dbuf_s ovrBuf;
        struct dbuf_s vBuf;

        dbuf_init(&ovrBuf, 4096);
        dbuf_init(&vBuf, 4096);

        pCodeInitRegisters();

        /* check for main() */
        mainf = newSymbol ("main", 0);
        mainf->block = 0;
        mainf = findSymWithLevel (SymbolTab, mainf);

        if (!mainf || !IFFUNC_HASBODY(mainf->type))
        {
                /* main missing -- import stack from main module */
                //fprintf (stderr, "main() missing -- assuming we are NOT the main module\n");
                pic14_options.isLibrarySource = 1;
        }

        /* At this point we've got all the code in the form of pCode structures */
        /* Now it needs to be rearranged into the order it should be placed in the */
        /* code space */

        movepBlock2Head('P');              // Last
        movepBlock2Head(code->dbName);
        movepBlock2Head('X');
        movepBlock2Head(statsg->dbName);   // First


        /* print the global struct definitions */
        if (options.debug)
                cdbStructBlock (0);

        /* do the overlay segments */
        pic14emitOverlay(&ovrBuf);

        /* PENDING: this isnt the best place but it will do */
        if (port->general.glue_up_main) {
                /* create the interrupt vector table */
                pic14createInterruptVect (&vBuf);
        }

        AnalyzepCode('*');

        ReuseReg(); // ReuseReg where call tree permits

        InlinepCode();

        AnalyzepCode('*');

        if (options.debug) pcode_test();


        /* now put it all together into the assembler file */
        /* create the assembler file name */

        if ((noAssemble || options.c1mode) && fullDstFileName)
        {
                sprintf (buffer, "%s", fullDstFileName);
        }
        else
        {
                sprintf (buffer, "%s", dstFileName);
                strcat (buffer, ".asm");
        }

        if (!(asmFile = fopen (buffer, "w"))) {
                werror (E_FILE_OPEN_ERR, buffer);
                exit (1);
        }

        /* prepare statistics */
        resetpCodeStatistics ();

        /* initial comments */
        pic14initialComments (asmFile);

        /* print module name */
        fprintf (asmFile, "%s\t.file\t\"%s\"\n",
            options.debug ? "" : ";", fullSrcFileName);

        /* Let the port generate any global directives, etc. */
        if (port->genAssemblerPreamble)
        {
                port->genAssemblerPreamble(asmFile);
        }

        /* Put all variables into a cblock */
        AnalyzeBanking();

        /* emit initialized data */
        showAllMemmaps(asmFile);

        /* print the locally defined variables in this module */
        writeUsedRegs(asmFile);

        /* create the overlay segments */
        fprintf (asmFile, "%s", iComments2);
        fprintf (asmFile, "; overlayable items in internal ram \n");
        fprintf (asmFile, "%s", iComments2);
        dbuf_write_and_destroy (&ovrBuf, asmFile);

        /* copy the interrupt vector table */
        if (mainf && IFFUNC_HASBODY(mainf->type))
          dbuf_write_and_destroy (&vBuf, asmFile);
        else
          dbuf_destroy(&vBuf);

        /* create interupt ventor handler */
        pic14_emitInterruptHandler (asmFile);

        /* copy over code */
        fprintf (asmFile, "%s", iComments2);
        fprintf (asmFile, "; code\n");
        fprintf (asmFile, "%s", iComments2);
        fprintf (asmFile, "code_%s\t%s\n", moduleName, port->mem.code_name);

        /* unknown */
        copypCode(asmFile, 'X');

        /* _main function */
        copypCode(asmFile, 'M');

        /* other functions */
        copypCode(asmFile, code->dbName);

        /* unknown */
        copypCode(asmFile, 'P');

        dumppCodeStatistics (asmFile);

        fprintf (asmFile,"\tend\n");

        fclose (asmFile);
        pic14_debugLogClose();
}

/*
 * Deal with initializers.
 */
#undef DEBUGprintf
#if 0
// debugging output
#define DEBUGprintf printf
#else
// be quiet
#define DEBUGprintf 1 ? (void)0 : (void)printf
#endif

static char *
parseIvalAst (ast *node, int *inCodeSpace) {
#define LEN 4096
    char *buffer = NULL;
    char *left, *right;

    if (IS_AST_VALUE(node)) {
        value *val = AST_VALUE(node);
        symbol *sym = IS_AST_SYM_VALUE(node) ? AST_SYMBOL(node) : NULL;
        if (inCodeSpace && val->type
                && (IS_FUNC(val->type) || IS_CODE(getSpec(val->type))))
        {
            *inCodeSpace = 1;
        }
        if (inCodeSpace && sym
                && (IS_FUNC(sym->type)
                    || IS_CODE(getSpec(sym->type))))
        {
            *inCodeSpace = 1;
        }

        DEBUGprintf ("%s: AST_VALUE\n", __FUNCTION__);
        if (IS_AST_LIT_VALUE(node)) {
            buffer = Safe_alloc(LEN);
            SNPRINTF(buffer, LEN, "0x%lx", AST_ULONG_VALUE (node));
        } else if (IS_AST_SYM_VALUE(node)) {
            assert ( AST_SYMBOL(node) );
            /*
            printf ("sym %s: ", AST_SYMBOL(node)->rname);
            printTypeChain(AST_SYMBOL(node)->type, stdout);
            printTypeChain(AST_SYMBOL(node)->etype, stdout);
            printf ("\n---sym %s: done\n", AST_SYMBOL(node)->rname);
            */
            buffer = Safe_strdup(AST_SYMBOL(node)->rname);
        } else {
            assert ( !"Invalid values type for initializers in AST." );
        }
    } else if (IS_AST_OP(node)) {
        DEBUGprintf ("%s: AST_OP\n", __FUNCTION__);
        switch (node->opval.op) {
        case CAST:
            assert (node->right);
            buffer = parseIvalAst(node->right, inCodeSpace);
            DEBUGprintf ("%s: %s\n", __FUNCTION__, buffer);
            break;
        case '&':
            assert ( node->left && !node->right );
            buffer = parseIvalAst(node->left, inCodeSpace);
            DEBUGprintf ("%s: %s\n", __FUNCTION__, buffer);
            break;
        case '+':
            assert (node->left && node->right );
            left = parseIvalAst(node->left, inCodeSpace);
            right = parseIvalAst(node->right, inCodeSpace);
            buffer = Safe_alloc(LEN);
            SNPRINTF(buffer, LEN, "(%s + %s)", left, right);
            DEBUGprintf ("%s: %s\n", __FUNCTION__, buffer);
            Safe_free(left);
            Safe_free(right);
            break;
        case '[':
            assert ( node->left && node->right );
            assert ( IS_AST_VALUE(node->left) && AST_VALUE(node->left)->sym );
            right = parseIvalAst(node->right, inCodeSpace);
            buffer = Safe_alloc(LEN);
            SNPRINTF(buffer, LEN, "(%s + %u * %s)",
                    AST_VALUE(node->left)->sym->rname, getSize(AST_VALUE(node->left)->type), right);
            Safe_free(right);
            DEBUGprintf ("%s: %s\n", __FUNCTION__, &buffer[0]);
            break;
        default:
            assert ( !"Unhandled operation in initializer." );
            break;
        }
    } else {
        assert ( !"Invalid construct in initializer." );
    }

    return (buffer);
}

/*
 * Emit the section preamble, absolute location (if any) and
 * symbol name(s) for intialized data.
 */
static int
emitIvalLabel(struct dbuf_s *oBuf, symbol *sym)
{
    char *segname;
    static int in_code = 0;
    static int sectionNr = 0;

    if (sym) {
        // code or data space?
        if (IS_CODE(getSpec(sym->type))) {
            segname = "code";
            in_code = 1;
        } else {
            segname = "idata";
            in_code  = 0;
        }
        dbuf_printf(oBuf, "\nID_%s_%d\t%s", moduleName, sectionNr++, segname);
        if (SPEC_ABSA(getSpec(sym->type))) {
            // specify address for absolute symbols
            dbuf_printf(oBuf, "\t0x%04X", SPEC_ADDR(getSpec(sym->type)));
        } // if
        dbuf_printf(oBuf, "\n%s\n", sym->rname);

        addSet(&emitted, sym->rname);
    }
    return (in_code);
}

/*
 * Actually emit the initial values in .asm format.
 */
static void
emitIvals(struct dbuf_s *oBuf, symbol *sym, initList *list, long lit, int size)
{
    int i;
    ast *node;
    operand *op;
    value *val = NULL;
    int inCodeSpace = 0;
    char *str = NULL;
    int in_code;

    assert (size <= sizeof(long));
    assert (!list || (list->type == INIT_NODE));
    node = list ? list->init.node : NULL;

    in_code = emitIvalLabel(oBuf, sym);
    if (!in_code)
        dbuf_printf (oBuf, "\tdb\t");

    if (!node) {
        for (i = 0; i < size; i++) {
            if (in_code) {
                dbuf_printf (oBuf, "\tretlw 0x%02x\n", (int)(lit & 0xff));
                // dbuf_printf (oBuf, "\tretlw 0x00\n"); // conflict from merge of sf-patch-2991122 ?
            } else {
                dbuf_printf (oBuf, "%s0x%02x", (i == 0) ? "" : ", ", (int)(lit & 0xff));
            }
            lit >>= 8;
        } // for
        if (!in_code)
            dbuf_printf (oBuf, "\n");
        return;
    } // if

    op = NULL;
    if (constExprTree(node) && (val = constExprValue(node, 0))) {
        op = operandFromValue(val);
        DEBUGprintf ("%s: constExpr ", __FUNCTION__);
    } else if (IS_AST_VALUE(node)) {
        op = operandFromAst(node, 0);
    } else if (IS_AST_OP(node)) {
        str = parseIvalAst(node, &inCodeSpace);
        DEBUGprintf("%s: AST_OP: %s\n", __FUNCTION__, str);
        op = NULL;
    } else {
        assert ( !"Unhandled construct in intializer." );
    }

    if (op) {
        aopOp(op, NULL, 1);
        assert(AOP(op));
        //printOperand(op, of);
    }

    for (i = 0; i < size; i++) {
        char *text;

        /*
         * FIXME: This is hacky and needs some more thought.
         */
        if (op && IS_SYMOP(op) && IS_FUNC(OP_SYM_TYPE(op))) {
            /* This branch is introduced to fix #1427663. */
            PCOI(AOP(op)->aopu.pcop)->offset+=i;
            text = get_op(AOP(op)->aopu.pcop, NULL, 0);
            PCOI(AOP(op)->aopu.pcop)->offset-=i;
        } else {
            text = op ? aopGet(AOP(op), i, 0, 0)
                : get_op(newpCodeOpImmd(str, i, 0, inCodeSpace, 0), NULL, 0);
        } // if
        if (in_code) {
            dbuf_printf (oBuf, "\tretlw %s\n", text);
        } else {
            dbuf_printf (oBuf, "%s%s", (i == 0) ? "" : ", ", text);
        }
    } // for
    if (!in_code)
        dbuf_printf (oBuf, "\n");
}

/*
 * For UNIONs, we first have to find the correct alternative to map the
 * initializer to. This function maps the structure of the initializer to
 * the UNION members recursively.
 * Returns the type of the first `fitting' member.
 */
static sym_link *
matchIvalToUnion (initList *list, sym_link *type, int size)
{
    symbol *sym;

    assert (type);

    if (IS_PTR(type) || IS_CHAR(type) || IS_INT(type) || IS_LONG(type)
            || IS_FLOAT(type))
    {
        if (!list || (list->type == INIT_NODE)) {
            DEBUGprintf ("OK, simple type\n");
            return (type);
        } else {
            DEBUGprintf ("ERROR, simple type\n");
            return (NULL);
        }
    } else if (IS_BITFIELD(type)) {
        if (!list || (list->type == INIT_NODE)) {
            DEBUGprintf ("OK, bitfield\n");
            return (type);
        } else {
            DEBUGprintf ("ERROR, bitfield\n");
            return (NULL);
        }
    } else if (IS_STRUCT(type) && SPEC_STRUCT(getSpec(type))->type == STRUCT) {
        if (!list || (list->type == INIT_DEEP)) {
            if (list) list = list->init.deep;
            sym = SPEC_STRUCT(type)->fields;
            while (sym) {
                DEBUGprintf ("Checking STRUCT member %s\n", sym->name);
                if (!matchIvalToUnion(list, sym->type, 0)) {
                    DEBUGprintf ("ERROR, STRUCT member %s\n", sym->name);
                    return (NULL);
                }
                if (list) list = list->next;
                sym = sym->next;
            } // while

            // excess initializers?
            if (list) {
                DEBUGprintf ("ERROR, excess initializers\n");
                return (NULL);
            }

            DEBUGprintf ("OK, struct\n");
            return (type);
        }
        return (NULL);
    } else if (IS_STRUCT(type) && SPEC_STRUCT(getSpec(type))->type == UNION) {
        if (!list || (list->type == INIT_DEEP)) {
            if (list) list = list->init.deep;
            sym = SPEC_STRUCT(type)->fields;
            while (sym) {
                while (list && list->type == INIT_HOLE) {
                  list = list->next;
                  sym = sym->next;
                }
                DEBUGprintf ("Checking UNION member %s.\n", sym->name);
                if (((IS_STRUCT(sym->type) || getSize(sym->type) == size))
                        && matchIvalToUnion(list, sym->type, size))
                {
                    DEBUGprintf ("Matched UNION member %s.\n", sym->name);
                    return (sym->type);
                }
                sym = sym->next;
            } // while
        } // if
        // no match found
        DEBUGprintf ("ERROR, no match found.\n");
        return (NULL);
    } else {
        assert ( !"Unhandled type in UNION." );
    }

    assert ( !"No match found in UNION for the given initializer structure." );
    return (NULL);
}

/*
 * Parse the type and its initializer and emit it (recursively).
 */
static void
emitInitVal(struct dbuf_s *oBuf, symbol *topsym, sym_link *my_type, initList *list)
{
    symbol *sym;
    int size;
    long lit;
    unsigned char *str;

    /* Handle designated initializers */
    if (list)
      list = reorderIlist (my_type, list);

    /* If this is a hole, substitute an appropriate initializer. */
    if (list && list->type == INIT_HOLE)
      {
        if (IS_AGGREGATE (my_type))
          {
            list = newiList(INIT_DEEP, NULL); /* init w/ {} */
          }
        else
          {
            ast *ast = newAst_VALUE (constVal("0"));
            ast = decorateType (ast, RESULT_TYPE_NONE);
            list = newiList(INIT_NODE, ast);
          }
      }

    size = getSize(my_type);

    if (IS_PTR(my_type)) {
        DEBUGprintf ("(pointer, %d byte) 0x%x\n", size, list ? (unsigned int)list2int(list) : 0);
        emitIvals(oBuf, topsym, list, 0, size);
        return;
    }

    if (IS_ARRAY(my_type) && topsym && topsym->isstrlit) {
        str = (unsigned char *)SPEC_CVAL(topsym->etype).v_char;
        emitIvalLabel(oBuf, topsym);
        do {
            dbuf_printf (oBuf, "\tretlw 0x%02x ; '%c'\n", str[0], (str[0] >= 0x20 && str[0] < 128) ? str[0] : '.');
        } while (*(str++));
        return;
    }

    if (IS_ARRAY(my_type) && list && list->type == INIT_NODE) {
        fprintf (stderr, "Unhandled initialized symbol: %s\n", topsym->name);
        assert ( !"Initialized char-arrays are not yet supported, assign at runtime instead." );
        return;
    }

    if (IS_ARRAY(my_type)) {
        size_t i;

        DEBUGprintf ("(array, %d items, %ud byte) below\n", (unsigned int) DCL_ELEM(my_type), size);
        assert (!list || list->type == INIT_DEEP);
        if (list) list = list->init.deep;
        for (i = 0; i < DCL_ELEM(my_type); i++) {
            emitInitVal(oBuf, topsym, my_type->next, list);
            topsym = NULL;
            if (list) list = list->next;
        } // for i
        return;
    }

    if (IS_FLOAT(my_type)) {
        // float, 32 bit
        DEBUGprintf ("(float, %d byte) %lf\n", size, list ? list2int(list) : 0.0);
        emitIvals(oBuf, topsym, list, 0, size);
        return;
    }

    if (IS_CHAR(my_type) || IS_INT(my_type) || IS_LONG(my_type)) {
        // integral type, 8, 16, or 32 bit
        DEBUGprintf ("(integral, %d byte) 0x%lx/%ld\n", size, list ? (long)list2int(list) : 0, list ? (long)list2int(list) : 0);
        emitIvals(oBuf, topsym, list, 0, size);
        return;

    } else if (IS_STRUCT(my_type) && SPEC_STRUCT(my_type)->type == STRUCT) {
        // struct
        DEBUGprintf ("(struct, %d byte) handled below\n", size);
        assert (!list || (list->type == INIT_DEEP));

        // iterate over struct members and initList
        if (list) list = list->init.deep;
        sym = SPEC_STRUCT(my_type)->fields;
        while (sym) {
            long bitfield = 0;
            int len = 0;
            if (IS_BITFIELD(sym->type)) {
                while (sym && IS_BITFIELD(sym->type)) {
                    int bitoff = SPEC_BSTR(getSpec(sym->type)) + 8 * sym->offset;
                    assert (!list || ((list->type == INIT_NODE)
                                && IS_AST_LIT_VALUE(list->init.node)));
                    lit = (long) (list ? list2int(list) : 0);
                    DEBUGprintf ( "(bitfield member) %02lx (%d bit, starting at %d, bitfield %02lx)\n",
                            lit, SPEC_BLEN(getSpec(sym->type)),
                            bitoff, bitfield);
                    bitfield |= (lit & ((1ul << SPEC_BLEN(getSpec(sym->type))) - 1)) << bitoff;
                    len += SPEC_BLEN(getSpec(sym->type));

                    sym = sym->next;
                    if (list) list = list->next;
                } // while
                assert (len < sizeof (long) * 8); // did we overflow our initializer?!?
                len = (len + 7) & ~0x07; // round up to full bytes
                emitIvals(oBuf, topsym, NULL, bitfield, len / 8);
                topsym = NULL;
            } // if

            if (sym) {
                emitInitVal(oBuf, topsym, sym->type, list);
                topsym = NULL;
                sym = sym->next;
                if (list) list = list->next;
            } // if
        } // while
        if (list) {
            assert ( !"Excess initializers." );
        } // if
        return;

    } else if (IS_STRUCT(my_type) && SPEC_STRUCT(my_type)->type == UNION) {
        // union
        DEBUGprintf ("(union, %d byte) handled below\n", size);
        assert (list && list->type == INIT_DEEP);

        // iterate over union members and initList, try to map number and type of fields and initializers
        my_type = matchIvalToUnion(list, my_type, size);
        if (my_type) {
            emitInitVal(oBuf, topsym, my_type, list->init.deep);
            topsym = NULL;
            size -= getSize(my_type);
            if (size > 0) {
                // pad with (leading) zeros
                emitIvals(oBuf, NULL, NULL, 0, size);
            }
            return;
        } // if

        assert ( !"No UNION member matches the initializer structure.");
    } else if (IS_BITFIELD(my_type)) {
        assert ( !"bitfields should only occur in structs..." );

    } else {
        printf ("SPEC_NOUN: %d\n", SPEC_NOUN(my_type));
        assert( !"Unhandled initialized type.");
    }
}

/*
 * Emit a set of symbols.
 * type - 0: have symbol tell whether it is local, extern or global
 *        1: assume all symbols in set to be global
 *        2: assume all symbols in set to be extern
 */
static void
emitSymbolSet(set *s, int type)
{
    symbol *sym;
    initList *list;
    unsigned sectionNr = 0;

    for (sym = setFirstItem(s); sym; sym = setNextItem(s)) {
#if 0
        fprintf (stdout, ";    name %s, rname %s, level %d, block %d, key %d, local %d, ival %p, static %d, cdef %d, used %d\n",
                sym->name, sym->rname, sym->level, sym->block, sym->key, sym->islocal, sym->ival, IS_STATIC(sym->etype), sym->cdef, sym->used);
#endif

        if (sym->etype && SPEC_ABSA(sym->etype)
                && IS_CONFIG_ADDRESS(SPEC_ADDR(sym->etype))
                && sym->ival)
        {
            // handle config words
            pic14_assignConfigWordValue(SPEC_ADDR(sym->etype),
                    (int)list2int(sym->ival));
            pic14_stringInSet(sym->rname, &emitted, 1);
            continue;
        }

        if (sym->isstrlit) {
            // special case: string literals
            emitInitVal(ivalBuf, sym, sym->type, NULL);
            continue;
        }

        if (type != 0 || sym->cdef
                || (!IS_STATIC(sym->etype)
                    && IS_GLOBAL(sym)))
        {
            // bail out for ___fsadd and friends
            if (sym->cdef && !sym->used) continue;

            /* export or import non-static globals */
            if (!pic14_stringInSet(sym->rname, &emitted, 0)) {

                if (type == 2 || IS_EXTERN(sym->etype) || sym->cdef)
                {
                    /* do not add to emitted set, it might occur again! */
                    //if (!sym->used) continue;
                    // declare symbol
                    emitIfNew (extBuf, &emitted, "\textern\t%s\n", sym->rname);
                } else {
                    // declare symbol
                    emitIfNew (gloBuf, &emitted, "\tglobal\t%s\n", sym->rname);
                    if (!sym->ival && !IS_FUNC(sym->type)) {
                        // also define symbol
                        if (IS_ABSOLUTE(sym->etype)) {
                            // absolute location?
                            //dbuf_printf (gloDefBuf, "UD_%s_%u\tudata\t0x%04X\n", moduleName, sectionNr++, SPEC_ADDR(sym->etype));
                            // deferred to pic14_constructAbsMap
                        } else {
                            dbuf_printf (gloDefBuf, "UD_%s_%u\tudata\n", moduleName, sectionNr++);
                            dbuf_printf (gloDefBuf, "%s\tres\t%d\n\n", sym->rname, getSize(sym->type));
                        }
                    } // if
                } // if
                pic14_stringInSet(sym->rname, &emitted, 1);
            } // if
        } // if
        list = sym->ival;
        //if (list) showInitList(list, 0);
        if (list) {
            resolveIvalSym( list, sym->type );
            emitInitVal(ivalBuf, sym, sym->type, sym->ival);
            dbuf_printf (ivalBuf, "\n");
        }
    } // for sym
}

/*
 * Iterate over all memmaps and emit their contents (attributes, symbols).
 */
static void
showAllMemmaps(FILE *of)
{
    struct dbuf_s locBuf;
    memmap *maps[] = {
        xstack, istack, code, data, pdata, xdata, xidata, xinit,
        idata, bit, statsg, c_abs, x_abs, i_abs, d_abs,
        sfr, sfrbit, reg, generic, overlay, eeprom, home };
    memmap * map;
    int i;

    DEBUGprintf ("---begin memmaps---\n");
    if (!extBuf) extBuf = dbuf_new(1024);
    if (!gloBuf) gloBuf = dbuf_new(1024);
    if (!gloDefBuf) gloDefBuf = dbuf_new(1024);
    if (!ivalBuf) ivalBuf = dbuf_new(1024);
    dbuf_init(&locBuf, 1024);

    dbuf_printf (extBuf, "%s; external declarations\n%s", iComments2, iComments2);
    dbuf_printf (gloBuf, "%s; global declarations\n%s", iComments2, iComments2);
    dbuf_printf (gloDefBuf, "%s; global definitions\n%s", iComments2, iComments2);
    dbuf_printf (ivalBuf, "%s; initialized data\n%s", iComments2, iComments2);
    dbuf_printf (&locBuf, "%s; compiler-defined variables\n%s", iComments2, iComments2);

    for (i = 0; i < sizeof(maps) / sizeof (memmap *); i++) {
        map = maps[i];
        //DEBUGprintf ("memmap %i: %p\n", i, map);
        if (map) {
#if 0
            fprintf (stdout, ";  pageno %c, sname %s, dbName %c, ptrType %d, slbl %d, sloc %u, fmap %u, paged %u, direct %u, bitsp %u, codesp %u, regsp %u, syms %p\n",
                    map->pageno, map->sname, map->dbName, map->ptrType, map->slbl,
                    map->sloc, map->fmap, map->paged, map->direct, map->bitsp,
                    map->codesp, map->regsp, map->syms);
#endif
            emitSymbolSet(map->syms, 0);
        } // if (map)
    } // for i
    DEBUGprintf ("---end of memmaps---\n");

    emitSymbolSet(publics, 1);
    emitSymbolSet(externs, 2);

    emitPseudoStack(gloBuf, extBuf);
    pic14_constructAbsMap(gloDefBuf, gloBuf);
    pic14printLocals (&locBuf);
    pic14_emitConfigWord(of); // must be done after all the rest

    dbuf_write_and_destroy(extBuf, of);
    dbuf_write_and_destroy(gloBuf, of);
    dbuf_write_and_destroy(gloDefBuf, of);
    dbuf_write_and_destroy(&locBuf, of);
    dbuf_write_and_destroy(ivalBuf, of);

    extBuf = gloBuf = gloDefBuf = ivalBuf = NULL;
}
