#include "common.h"


/*************************************************************
 *
 *
 *
 *
 *************************************************************/

int cdbOpenFile (const char *file);
int cdbCloseFile (void);
int cdbWriteFunction (symbol *pSym, iCode *ic);
int cdbWriteEndFunction (symbol *pSym, iCode *ic, int offset);
int cdbWriteLabel (symbol *pSym, iCode *ic);
int cdbWriteScope (iCode *ic);
int cdbWriteSymbol (symbol *pSym);
int cdbWriteType (structdef *sdef, int block, int inStruct, const char *tag);
int cdbWriteModule (const char *name);
int cdbWriteCLine (iCode *ic);
int cdbWriteALine (const char *module, int Line);
int cdbWriteFrameAddress (const char *variable, struct reg_info *reg, int offset);
int cdbWriteBasicSymbol (symbol *sym, int isStructSym, int isFunc);
void cdbTypeInfo (sym_link * type);


DEBUGFILE cdbDebugFile = 
  {
    &cdbOpenFile,
    &cdbCloseFile,
    &cdbWriteModule,
    &cdbWriteFunction,
    &cdbWriteEndFunction,
    &cdbWriteLabel,
    &cdbWriteScope,
    &cdbWriteSymbol,
    &cdbWriteType,
    &cdbWriteCLine,
    &cdbWriteALine,
    &cdbWriteFrameAddress
  };

FILE *cdbFilePtr = NULL;
const char *cdbModuleName = NULL;

/******************************************************************
 * spacesToUnderscores - replace all non alpha-numerics with
 * underscores
 *
 *
 *****************************************************************/

static char *
spacesToUnderscores (char *dest, const char *src, size_t len)
{
  unsigned int i;
  char *p;

  assert (dest != NULL);
  assert (src != NULL);
  assert (len > 0);

  --len;
  for (p = dest, i = 0; *src != '\0' && i < len; ++src, ++i) {
    *p++ = (isspace ((unsigned char)*src) || (*src == '-')) ? '_' : *src;
  }
  *p = '\0';

  return dest;
}


/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbOpenFile (const char *file)
{
  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbOpenFile (%s)\n", file);

  return (cdbFilePtr = fopen(file, "w")) ? 1 : 0; 
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/
int
cdbCloseFile (void)
{
  if (getenv("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbCloseFile()\n");

  if(!cdbFilePtr) return 0;
 
  fclose(cdbFilePtr);
  cdbFilePtr = NULL;
  cdbModuleName = NULL;

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteFunction (symbol *pSym, iCode *ic)
{
  char debugSym[INITIAL_INLINEASM];
  
  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteFunction()\n");

  if (!cdbFilePtr) return 0;

  if (IS_STATIC (pSym->etype))
    sprintf (debugSym, "F%s$%s$0$0", moduleName, pSym->name);
  else
    sprintf (debugSym, "G$%s$0$0", pSym->name);
  emitDebuggerSymbol (debugSym);

  return cdbWriteBasicSymbol (pSym, FALSE, TRUE);
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteEndFunction (symbol *pSym, iCode *ic, int offset)
{
  char debugSym[INITIAL_INLINEASM];
  
  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteEndFunction()\n");

  if (!cdbFilePtr) return 0;

  if (ic)
    {
      sprintf (debugSym, "C$%s$%d$%d$%d",
               FileBaseName (ic->filename), pSym->lastLine,
               ic->level, ic->block);
      spacesToUnderscores (debugSym, debugSym, sizeof (debugSym));
      emitDebuggerSymbol (debugSym);
    }

  if (IS_STATIC (pSym->etype))
    sprintf (debugSym, "XF%s$%s$0$0", moduleName, pSym->name);
  else
    sprintf (debugSym, "XG$%s$0$0", pSym->name);
  emitDebuggerSymbol (debugSym);
    
  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteLabel (symbol *pSym, iCode *ic)
{
  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteLabel()\n");

  if (!cdbFilePtr) return 0;

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteScope (iCode *ic)
{
  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteScope()\n");

  if (!cdbFilePtr) return 0;

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteSymbol(symbol *pSym)
{
  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteSymbol()\n");

  if (!cdbFilePtr) return 0;

  return cdbWriteBasicSymbol(pSym, FALSE, FALSE);
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteType (structdef *sdef, int block, int inStruct, const char *tag)
{
  symbol *sym;

  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteType()\n");

  if (!cdbFilePtr) return 0;

  fprintf (cdbFilePtr, "T:");

  /* if block # then must have function scope */
  fprintf (cdbFilePtr, "F%s$", moduleName);

  fprintf (cdbFilePtr, "%s[", (tag ? tag : sdef->tag));

  for (sym = sdef->fields; sym; sym = sym->next)
    {
      fprintf (cdbFilePtr, "({%d}", sym->offset);
      cdbWriteBasicSymbol (sym, TRUE, FALSE);
      fprintf (cdbFilePtr, ")");
    }

  fprintf (cdbFilePtr, "]");

  if (!inStruct)
    fprintf (cdbFilePtr, "\n");

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteModule(const char *name)
{
  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteModule()\n");

  if (!cdbFilePtr) return 0;
  cdbModuleName = name;

  fprintf(cdbFilePtr, "M:%s\n", cdbModuleName);

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/
int
cdbWriteCLine (iCode *ic)
{
  char debugSym[INITIAL_INLINEASM];
  
  if (!cdbFilePtr) return 0;

  sprintf (debugSym, "C$%s$%d$%d$%d", 
           FileBaseName (ic->filename), ic->lineno,
           ic->level, ic->block);
  spacesToUnderscores (debugSym, debugSym, sizeof (debugSym));
  emitDebuggerSymbol (debugSym);

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteALine (const char *module, int Line)
{
  if (!cdbFilePtr) return 0;

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteFrameAddress (const char *variable, struct reg_info *reg, int offset)
{
  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteFrameAddress()\n");

  if (!cdbFilePtr) return 0;

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

int
cdbWriteBasicSymbol (symbol *sym, int isStructSym, int isFunc)
{
  memmap *map;
  symbol *sym2;

  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "cdbFile.c:cdbWriteBasicSymbol()\n");

  if (!cdbFilePtr) return 0;

  if (!sym) return 0;

  /* WRITE HEADER, Function or Symbol */
  if (isFunc)
    fprintf (cdbFilePtr, "F:");   
  else
    fprintf (cdbFilePtr, "S:");

  /* STRUCTS do not have scope info.. */

  if (!isStructSym)
    {
      if (sym->level && sym->localof)   /* symbol is local */
        {
          fprintf (cdbFilePtr, "L%s.%s$", moduleName, sym->localof->name);
        }
      else if (IS_STATIC (sym->etype))  /* scope is file */
        {
          fprintf (cdbFilePtr, "F%s$", moduleName);
        }
      else                              /* scope is global */
        {
          fprintf (cdbFilePtr, "G$");
        }
    }
  else
    {
      fprintf (cdbFilePtr, "S$");       /* scope is structure */
    }

  /* print the name, & mangled name */
  fprintf (cdbFilePtr, "%s$%d$%d(", sym->name, sym->level, sym->block);

  cdbTypeInfo (sym->type);

  fprintf (cdbFilePtr, "),");

  /* CHECK FOR REGISTER SYMBOL... */ 
  if (!sym->allocreq && sym->reqv)
    {
      int a;
      symbol *TempSym = OP_SYMBOL (sym->reqv);

      if (!TempSym->isspilt || TempSym->remat)
        {
          fprintf (cdbFilePtr, "R,0,0,[");

          for (a = 0; a < 4; a++)
            {
              if (TempSym->regs[a])
                {
                  fprintf (cdbFilePtr, "%s%s", port->getRegName(TempSym->regs[a]),
                           ((a < 3) && (TempSym->regs[a+1])) ? "," : "");
                }
            }

          fprintf (cdbFilePtr, "]");
          sym2 = NULL;
        }
      else
        {
          sym2 = TempSym->usl.spillLoc;
        }
    }
  else
    {
      sym2 = sym;
    }

  if (sym2)
    {
      /* print the address space */
      map = SPEC_OCLS (sym2->etype);

      fprintf (cdbFilePtr, "%c,%d,%d",
               (map ? map->dbName : 'Z'), sym2->onStack, SPEC_STAK (sym2->etype));
    }

  /* if assigned to registers then output register names */
  /* if this is a function then print
     if is it an interrupt routine & interrupt number
     and the register bank it is using */
  if (isFunc)
    fprintf (cdbFilePtr, ",%d,%d,%d", FUNC_ISISR (sym->type),
             FUNC_INTNO (sym->type), FUNC_REGBANK (sym->type));
  
/* alternate location to find this symbol @ : eg registers
     or spillocation */

  if (!isStructSym)
    fprintf (cdbFilePtr, "\n");

  return 1;
}

/******************************************************************
 *
 *
 *
 *
 *****************************************************************/

/*-----------------------------------------------------------------*/
/* cdbTypeInfo - print the type information for debugger           */
/*-----------------------------------------------------------------*/
void
cdbTypeInfo (sym_link * type)
{
  fprintf (cdbFilePtr, "{%d}", getSize (type));

  while (type)
    {
      if (IS_DECL (type))
        {
          switch (DCL_TYPE (type))
            {
            case FUNCTION: fprintf (cdbFilePtr, "DF,"); break;
            case GPOINTER: fprintf (cdbFilePtr, "DG,"); break;
            case CPOINTER: fprintf (cdbFilePtr, "DC,"); break;
            case FPOINTER: fprintf (cdbFilePtr, "DX,"); break;
            case POINTER:  fprintf (cdbFilePtr, "DD,"); break;
            case IPOINTER: fprintf (cdbFilePtr, "DI,"); break;
            case PPOINTER: fprintf (cdbFilePtr, "DP,"); break;
            case EEPPOINTER: fprintf (cdbFilePtr, "DA,"); break;
            case ARRAY: fprintf (cdbFilePtr, "DA%ud,", (unsigned int) DCL_ELEM (type)); break;
            default: break;
            }
        }
      else
        {
          switch (SPEC_NOUN (type))
            {
            case V_INT:
              if (IS_LONG (type))
                fprintf (cdbFilePtr, "SL");
              else
                fprintf (cdbFilePtr, "SI");
              break;

            case V_CHAR: fprintf (cdbFilePtr, "SC"); break;
            case V_VOID: fprintf (cdbFilePtr, "SV"); break;
            case V_FLOAT: fprintf (cdbFilePtr, "SF"); break;
            case V_FIXED16X16: fprintf(cdbFilePtr, "SQ"); break;
            case V_STRUCT: 
              fprintf (cdbFilePtr, "ST%s", SPEC_STRUCT (type)->tag); 
              break;

            case V_SBIT: fprintf (cdbFilePtr, "SX"); break;
            case V_BIT: 
            case V_BITFIELD: 
              fprintf (cdbFilePtr, "SB%d$%d", SPEC_BSTR (type), 
                       SPEC_BLEN (type));
              break;

            default:
              break;
            }
          fputs (":", cdbFilePtr);
          if (SPEC_USIGN (type))
            fputs ("U", cdbFilePtr);
          else
            fputs ("S", cdbFilePtr);
        }
      type = type->next;
    }
}
