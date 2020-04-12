/*-------------------------------------------------------------------------
  SDCCglue.c - glues everything we have done together into one file.

  Copyright (C) 1998 Sandeep Dutta . sandeep.dutta@usa.net

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

#include "common.h"
#include <time.h>
#include "newalloc.h"
#include <fcntl.h>
#include <sys/stat.h>
#include "dbuf_string.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

symbol *interrupts[INTNO_MAX + 1];

void printIval (symbol *, sym_link *, initList *, struct dbuf_s *, bool check);
set *publics = NULL;            /* public variables */
set *externs = NULL;            /* Variables that are declared as extern */
set *strSym = NULL;             /* string initializers */
set *ccpStr = NULL;             /* char * const pointers with a string literal initializer */

unsigned maxInterrupts = 0;
int allocInfo = 1;
symbol *mainf;
int noInit = 0;                 /* no initialization */


char *
aopLiteralGptr (const char * name, value * val)
{
  unsigned long v = ulFromVal (val);
  struct dbuf_s dbuf;

  dbuf_init (&dbuf, 128);

  v >>= ((GPTRSIZE - 1) * 8);

  if (IS_FUNCPTR (val->type))
    dbuf_tprintf (&dbuf, "!immedbyte", v | pointerTypeToGPByte (DCL_TYPE (val->type->next), val->name, name));
  else if (IS_PTR (val->type) && !IS_GENPTR (val->type))
    dbuf_tprintf (&dbuf, "!immedbyte", pointerTypeToGPByte (DCL_TYPE (val->type), val->name, name));
  else
    dbuf_tprintf (&dbuf, "!immedbyte", (unsigned int) v & 0xff);

  return dbuf_detach_c_str (&dbuf);
}

const char *
aopLiteralLong (value *val, int offset, int size)
{
  unsigned long v;
  struct dbuf_s dbuf;

  if (!val)
    {
      // assuming we have been warned before
      val = constCharVal (0);
    }

  dbuf_init (&dbuf, 128);

  switch (size)
    {
    case 1:
      v = byteOfVal (val, offset);
      dbuf_tprintf (&dbuf, "!immedbyte", (unsigned int) v & 0xff);
      break;
    case 2:
      v = byteOfVal (val, offset+1);
      v = (v << 8) | byteOfVal (val, offset);
      dbuf_tprintf (&dbuf, "!immedword", (unsigned int) v & 0xffff);
      break;
    case 3:
      v = byteOfVal (val, offset+2);
      v = (v << 8) | byteOfVal (val, offset+1);
      v = (v << 8) | byteOfVal (val, offset);
      // we don't have a !immedword24 yet for ds390
      dbuf_printf (&dbuf, "#0x%06X", (unsigned int) v & 0xffffff);
      break;
    default:
      /* Hmm.  Too big for now. */
      assert (0);
    }
  return dbuf_detach_c_str (&dbuf);
}

/*-----------------------------------------------------------------*/
/* aopLiteral - string from a literal value                        */
/*-----------------------------------------------------------------*/
const char *
aopLiteral (value *val, int offset)
{
  return aopLiteralLong (val, offset, 1);
}

/*-----------------------------------------------------------------*/
/* emitDebugSym - emit label for debug symbol                      */
/*-----------------------------------------------------------------*/
static void
emitDebugSym (struct dbuf_s *oBuf, symbol * sym)
{
  if (sym->level && sym->localof)       /* symbol scope is local */
    {
      dbuf_printf (oBuf, "L%s.%s$", moduleName, sym->localof->name);
    }
  else if (IS_STATIC (sym->etype))      /* symbol scope is file */
    {
      dbuf_printf (oBuf, "F%s$", moduleName);
    }
  else                          /* symbol scope is global */
    {
      dbuf_printf (oBuf, "G$");
    }
  dbuf_printf (oBuf, "%s$%ld_%ld$%d", sym->name, sym->level / LEVEL_UNIT, sym->level % LEVEL_UNIT, sym->block);
}

/*-----------------------------------------------------------------*/
/* emitRegularMap - emit code for maps with no special cases       */
/*-----------------------------------------------------------------*/
static void
emitRegularMap (memmap *map, bool addPublics, bool arFlag)
{
  symbol *sym;
  ast *ival = NULL;

  if (!map)
    return;

  if (addPublics)
    {
      /* PENDING: special case here - should remove */
      if (!strcmp (map->sname, CODE_NAME))
        dbuf_tprintf (&map->oBuf, "\t!areacode\n", map->sname);
      else if (!strcmp (map->sname, DATA_NAME))
        {
          dbuf_tprintf (&map->oBuf, "\t!areadata\n", map->sname);
          if (options.data_seg && strcmp (DATA_NAME, options.data_seg))
            dbuf_tprintf (&map->oBuf, "\t!area\n", options.data_seg);
        }
      else if (!strcmp (map->sname, HOME_NAME))
        dbuf_tprintf (&map->oBuf, "\t!areahome\n", map->sname);
      else
        dbuf_tprintf (&map->oBuf, "\t!area\n", map->sname);

      if (map->regsp)
        dbuf_tprintf (&map->oBuf, "\t!org\n", 0);
    }

  for (sym = setFirstItem (map->syms); sym; sym = setNextItem (map->syms))
    {
      symbol *newSym = NULL;

      /* if allocation required check is needed
         then check if the symbol really requires
         allocation only for local variables */

      if (arFlag && !IS_AGGREGATE (sym->type) && !(sym->_isparm && !IS_REGPARM (sym->etype)) && !sym->allocreq && sym->level)
        continue;

      /* for bitvar locals and parameters */
      if (!arFlag && !sym->allocreq && sym->level && !SPEC_ABSA (sym->etype))
        {
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
          addSetHead (&publics, sym);
        }

      /* if extern then add it into the extern list */
      if (IS_EXTERN (sym->etype) && !sym->ival)
        {
          addSetHead (&externs, sym);
          continue;
        }

      /* if extern then do nothing or is a function
         then do nothing */
      if (IS_FUNC (sym->type) && !(sym->isitmp))
        continue;

      /* if it has an initial value then do it only if
         it is a global variable */
      if (sym->ival && sym->level == 0)
        {
          if ((SPEC_OCLS (sym->etype) == xidata || SPEC_OCLS (sym->etype) == initialized) && !SPEC_ABSA (sym->etype) && !SPEC_ADDRSPACE (sym->etype))
            {
              sym_link *t;
              ast *ival = NULL;
              symbol *tsym = copySymbol (sym);

              // check for constant
              if (IS_AGGREGATE (tsym->type))
                {
                  ival = initAggregates (tsym, tsym->ival, NULL);
                }
              else
                {
                  if (getNelements (tsym->type, tsym->ival) > 1)
                    {
                      werrorfl (tsym->fileDef, tsym->lineDef, W_EXCESS_INITIALIZERS, "scalar", tsym->name);
                    }
                  ival = newNode ('=', newAst_VALUE (symbolVal (tsym)),
                                  decorateType (resolveSymbols (list2expr (tsym->ival)), RESULT_TYPE_NONE));
                }
              if (ival)
                {
                  // set ival's lineno to where the symbol was defined
                  setAstFileLine (ival, filename = tsym->fileDef, lineno = tsym->lineDef);
                  // check if this is not a constant expression
                  if (!constExprTree (ival))
                    {
                      werrorfl (ival->filename, ival->lineno, E_CONST_EXPECTED, "found expression");
                      // but try to do it anyway
                    }
                }

              /* create a new "XINIT (CODE)" symbol, that will be emitted later
                 in the static seg */
              newSym = copySymbol (sym);
              SPEC_OCLS (newSym->etype) = (SPEC_OCLS (sym->etype) == xidata) ? xinit : initializer;
              SNPRINTF (newSym->name, sizeof (newSym->name), "__xinit_%s", sym->name);
              SNPRINTF (newSym->rname, sizeof (newSym->rname), "__xinit_%s", sym->rname);

              /* find the first non-array link */
              t = newSym->type;
              while (IS_ARRAY (t))
                t = t->next;
              if (IS_SPEC (t))
                SPEC_CONST (t) = 1;
              else
                DCL_PTR_CONST (t) = 1;
              SPEC_STAT (newSym->etype) = 1;

              strSym = NULL;
              ++noAlloc;
              resolveIvalSym (newSym->ival, newSym->type);
              --noAlloc;

              // add it to the "XINIT (CODE)" segment
              addSet ((SPEC_OCLS (sym->etype) == xidata) ? &xinit->syms : &initializer->syms, newSym);

              if (!SPEC_ABSA (sym->etype))
                {
                  struct dbuf_s tmpBuf;
                  symbol *ps = NULL;
                  set *tmpSym = NULL;

                  wassert(dbuf_init (&tmpBuf, 4096));
                  // before allocation we must parse the sym->ival tree
                  // but without actually generating initialization code
                  ++noAlloc;
                  resolveIvalSym (sym->ival, sym->type);
                  ++noInit;
                  printIval (sym, sym->type, sym->ival, &tmpBuf, TRUE);
                  --noInit;
                  --noAlloc;

                  // delete redundant __str_%d symbols (initializer for char arrays)
                  for (ps = setFirstItem (statsg->syms); ps; ps = setNextItem (statsg->syms))
                    if (!strstr (tmpBuf.buf, ps->name) && isinSet (strSym, ps))
                      addSet (&tmpSym, ps);
                  for (ps = setFirstItem (tmpSym); ps; ps = setNextItem (tmpSym))
                    deleteSetItem (&statsg->syms, ps);

                  deleteSet (&tmpSym);
                  dbuf_destroy (&tmpBuf);
                }

              if (strSym)
                deleteSet (&strSym);
            }
          else
            {
              if (IS_AGGREGATE (sym->type))
                {
                  ival = initAggregates (sym, sym->ival, NULL);
                }
              else
                {
                  if (getNelements (sym->type, sym->ival) > 1)
                    {
                      werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "scalar", sym->name);
                    }
                  ival = newNode ('=', newAst_VALUE (symbolVal (sym)),
                                  decorateType (resolveSymbols (list2expr (sym->ival)), RESULT_TYPE_NONE));
                }
              codeOutBuf = &statsg->oBuf;

              if (ival)
                {
                  // set ival's lineno to where the symbol was defined
                  setAstFileLine (ival, filename = sym->fileDef, lineno = sym->lineDef);
                  // check if this is not a constant expression
                  if (!constExprTree (ival))
                    {
                      werrorfl (ival->filename, ival->lineno, E_CONST_EXPECTED, "found expression");
                      // but try to do it anyway
                    }
                  allocInfo = 0;
                  if (!astErrors (ival))
                    eBBlockFromiCode (iCodeFromAst (ival));
                  allocInfo = 1;
                }
            }
        }

      /* if it has an absolute address then generate
         an equate for this no need to allocate space */
      if (SPEC_ABSA (sym->etype) && !sym->ival)
        {
          char *equ = "=";

          /* print extra debug info if required */
          if (options.debug)
            {
              emitDebugSym (&map->oBuf, sym);
              dbuf_printf (&map->oBuf, " == 0x%04x\n", SPEC_ADDR (sym->etype));
            }
          dbuf_printf (&map->oBuf, "%s\t%s\t0x%04x\n", sym->rname, equ, SPEC_ADDR (sym->etype));
        }
      else
        {
          int size = getSize (sym->type) + sym->flexArrayLength;
          if (size == 0)
            {
              werrorfl (sym->fileDef, sym->lineDef, E_UNKNOWN_SIZE, sym->name);
            }
          /* allocate space */
          if (SPEC_ABSA (sym->etype))
            {
              dbuf_tprintf (&map->oBuf, "\t!org\n", SPEC_ADDR (sym->etype));
            }
          /* print extra debug info if required */
          if (options.debug)
            {
              emitDebugSym (&map->oBuf, sym);
              dbuf_printf (&map->oBuf, "==.\n");
            }
          if (IS_STATIC (sym->etype) || sym->level)
            dbuf_tprintf (&map->oBuf, "!slabeldef\n", sym->rname);
          else
            dbuf_tprintf (&map->oBuf, "!labeldef\n", sym->rname);
          dbuf_tprintf (&map->oBuf, "\t!ds\n", (unsigned int) size & 0xffff);
        }

      sym->ival = NULL;
    }
}

/*-----------------------------------------------------------------*/
/* initValPointer - pointer initialization code massaging          */
/*-----------------------------------------------------------------*/
value *
initValPointer (ast *expr)
{
  value *val;

  /* no then we have to do these kludgy checks */
  /* pointers can be initialized with address of
     a variable or address of an array element */
  if (IS_ADDRESS_OF_OP (expr))
    {
      /* address of symbol */
      if (IS_AST_SYM_VALUE (expr->left))
        {
          STORAGE_CLASS sclass = SPEC_SCLS (expr->left->etype);
          memmap *oclass = SPEC_OCLS (expr->left->etype);

          val = AST_VALUE (expr->left);
          val->type = newLink (DECLARATOR);
          if (sclass == S_CODE)
            {
              DCL_TYPE (val->type) = CPOINTER;
              CodePtrPointsToConst (val->type);
            }
          else if (oclass)
            DCL_TYPE (val->type) = oclass->ptrType;
          else if (sclass == S_XDATA)
            DCL_TYPE (val->type) = FPOINTER;
          else if (sclass == S_DATA)
            DCL_TYPE (val->type) = POINTER;
          else if (sclass == S_IDATA)
            DCL_TYPE (val->type) = IPOINTER;
          else if (sclass == S_PDATA)
            DCL_TYPE (val->type) = PPOINTER;
          else if (sclass == S_XSTACK)
            DCL_TYPE (val->type) = PPOINTER;
          else if (sclass == S_EEPROM)
            DCL_TYPE (val->type) = EEPPOINTER;
          else
            DCL_TYPE (val->type) = POINTER;
          val->type->next = expr->left->ftype;
          val->etype = getSpec (val->type);
          return val;
        }

      /* if address of indexed array */
      if (IS_ARRAY_OP (expr->left))
        return valForArray (expr->left);

      /* if address of structure element then
         case 1. a.b ; */
      if (IS_AST_OP (expr->left) && expr->left->opval.op == '.')
        {
          return valForStructElem (expr->left->left, expr->left->right);
        }

      /* case 2. (&a)->b ;
         (&some_struct)->element */
      if (IS_AST_OP (expr->left) && expr->left->opval.op == PTR_OP && IS_ADDRESS_OF_OP (expr->left->left))
        {
          return valForStructElem (expr->left->left->left, expr->left->right);
        }

      /* case 2.1. a->b */
      if (IS_AST_OP (expr->left) && expr->left->opval.op == PTR_OP)
        {
          return valForStructElem (expr->left->left, expr->left->right);
        }
    }

  /* case 3. (((char *) &a) +/- constant) */
  if (IS_AST_OP (expr) &&
      (expr->opval.op == '+' || expr->opval.op == '-') &&
      IS_CAST_OP (expr->left) && IS_ADDRESS_OF_OP (expr->left->right) && IS_AST_LIT_VALUE (expr->right))
    {
      return valForCastAggr (expr->left->right->left, expr->left->left->opval.lnk, expr->right, expr->opval.op);
    }

  /* case 4. (array type) */
  if (IS_AST_SYM_VALUE (expr) && IS_ARRAY (expr->ftype))
    {
      STORAGE_CLASS sclass = SPEC_SCLS (expr->etype);
      memmap *oclass = SPEC_OCLS (expr->etype);

      val = copyValue (AST_VALUE (expr));
      val->type = newLink (DECLARATOR);
      if (SPEC_SCLS (expr->etype) == S_CODE)
        {
          DCL_TYPE (val->type) = CPOINTER;
          CodePtrPointsToConst (val->type);
        }
      else if (oclass)
        DCL_TYPE (val->type) = oclass->ptrType;
      else if (sclass == S_XDATA)
        DCL_TYPE (val->type) = FPOINTER;
      else if (sclass == S_DATA)
        DCL_TYPE (val->type) = POINTER;
      else if (sclass == S_IDATA)
        DCL_TYPE (val->type) = IPOINTER;
      else if (sclass == S_PDATA)
        DCL_TYPE (val->type) = PPOINTER;
      else if (sclass == S_XSTACK)
        DCL_TYPE (val->type) = PPOINTER;
      else if (sclass == S_EEPROM)
        DCL_TYPE (val->type) = EEPPOINTER;
      else
        DCL_TYPE (val->type) = POINTER;
      val->type->next = expr->ftype->next;
      val->etype = getSpec (val->type);
      return val;
    }

  /* if structure element then
     case 5. a.b ; */
  if (IS_AST_OP (expr) && expr->opval.op == '.')
    {
      return valForStructElem (expr->left, expr->right);
    }

  /* case 6. a->b ;
     some_struct->element */
  if (IS_AST_OP (expr) && expr->opval.op == PTR_OP)
    {
      ast *t = expr->left;

      /*if (expr->left->left)
        if (expr->left->left->left)
          if (expr->left->left->opval.op == '[')
            t = expr->left->left;
          else
            t = expr->left->left->left;
        else
          t = expr->left->left;
      else
        t = expr->left;*/

      /* a more generic way of above code */
      while (t->left != NULL && t->opval.op != '[')
        t = t->left;

      return valForStructElem (t, expr->right); 
    }

  /* case 7. function name */
  if (IS_AST_SYM_VALUE (expr) && IS_FUNC (expr->ftype))
    {
      return AST_VALUE (expr);
    }

  return NULL;
}

/*-----------------------------------------------------------------*/
/* initPointer - pointer initialization code massaging             */
/*-----------------------------------------------------------------*/
value *
initPointer (initList *ilist, sym_link *toType, int showError)
{
  value *val;
  ast *expr;

  if (!ilist)
    {
      return valCastLiteral (toType, 0.0, 0);
    }

  expr = list2expr (ilist);

  if (!expr)
    {
      if (showError)
        werror (E_CONST_EXPECTED);
      return 0;
    }

  /* try it the old way first */
  if ((val = constExprValue (expr, FALSE)))
    return val;

  /* ( ptr + constant ) */
  if (IS_AST_OP (expr) &&
      (expr->opval.op == '+' || expr->opval.op == '-') &&
      IS_AST_SYM_VALUE (expr->left) &&
      (IS_ARRAY (expr->left->ftype) || IS_PTR (expr->left->ftype)) &&
      compareType (toType, expr->left->ftype) && IS_AST_LIT_VALUE (expr->right))
    {
      return valForCastAggr (expr->left, expr->left->ftype, expr->right, expr->opval.op);
    }

  /* (char *)(expr1) */
  if (IS_CAST_OP (expr))
    {
      if (compareType (toType, expr->left->ftype) == 0 && showError)
        {
          werror (W_INIT_WRONG);
          printFromToType (expr->left->ftype, toType);
        }
      val = initValPointer (expr->right);
      if (val)
        {
          DECLARATOR_TYPE dcl_type = DCL_TYPE (val->type);
          val->type = expr->left->ftype;
          val->etype = getSpec (val->type);
          if (IS_GENPTR (val->type))
            DCL_TYPE (val->type) = dcl_type;
        }
    }
  else
    {
      val = initValPointer (expr);
    }

  if (val)
    return val;

  if (showError)
    {
      if (expr)
        werrorfl (expr->filename, expr->lineno, E_CONST_EXPECTED);
      else
        werror (E_CONST_EXPECTED);
    }
  return 0;
}

/*-----------------------------------------------------------------*/
/* printChar - formats and prints a UTF-8 character string with DB */
/*-----------------------------------------------------------------*/
void
printChar (struct dbuf_s *oBuf, const char *s, int plen)
{
  int i;
  int pplen = 0;
  char buf[100];
  char *p = buf;

  if (TARGET_PDK_LIKE && !TARGET_IS_PDK16) // Assembler does not support .ascii
    {
      while (pplen < plen)
        {
          if (isprint((unsigned char) *s))
            dbuf_tprintf (oBuf, "\t!db !constbyte\t; %c\n", (unsigned char) *s, (unsigned char) *s);
          else
            dbuf_tprintf (oBuf, "\t!db !constbyte\n", (unsigned char) *s);

          s++;
          pplen++;
        }
      return;
    }

  while (pplen < plen)
    {
      i = 60;
      while (i && pplen < plen)
        {
          if (!isprint((unsigned char) *s) || *s == '\"' || *s == '\\')
            {
              *p = '\0';
              if (p != buf)
                dbuf_tprintf (oBuf, "\t!ascii\n", buf);
              dbuf_tprintf (oBuf, "\t!db !constbyte\n", (unsigned char) *s);
              p = buf;
              i = 60;
            }
          else
            {
              *p = *s;
              p++;
              i--;
            }
          s++;
          pplen++;
        }
      if (p != buf)
        {
          *p = '\0';
          dbuf_tprintf (oBuf, "\t!ascii\n", buf);
          p = buf;
          i = 60;
        }
    }
}

/*-----------------------------------------------------------------*/
/* printChar16 - formats and prints a UTF-16 character string with DB*/
/*-----------------------------------------------------------------*/
void
printChar16 (struct dbuf_s *oBuf, const TYPE_TARGET_UINT *s, int plen)
{
  int pplen = 0;

  while (pplen < plen)
    {
      if (TARGET_PDK_LIKE && !TARGET_IS_PDK16)
        {
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", (*s >> 0) & 0xff);
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", (*s >> 8) & 0xff);
        }
      else if (port->little_endian)
        dbuf_printf (oBuf, "\t.byte %d,%d\n", (*s >> 0) & 0xff, (*s >> 8) & 0xff);
      else
        dbuf_printf (oBuf, "\t.byte %d,%d\n", (*s >> 8) & 0xff, (*s >> 0) & 0xff);

      s++;
      pplen++;
    }
  while (pplen < plen)
    {
      dbuf_tprintf (oBuf, "\t!db !constbyte\n", 0);
      pplen++;
    }
}

/*-----------------------------------------------------------------*/
/* printChar32 - formats and prints a UTF-32 character string with DB*/
/*-----------------------------------------------------------------*/
void
printChar32 (struct dbuf_s *oBuf, const TYPE_TARGET_ULONG *s, int plen)
{
  int pplen = 0;

  while (pplen < plen)
    {
      if (TARGET_PDK_LIKE && !TARGET_IS_PDK16)
        {
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", (*s >> 0) & 0xff);
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", (*s >> 8) & 0xff);
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", (*s >> 16) & 0xff);
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", (*s >> 24) & 0xff);
        }
      else if (port->little_endian)
        dbuf_printf (oBuf, "\t.byte %d,%d,%d,%d\n", (*s >> 0) & 0xff, (*s >> 8) & 0xff, (*s >> 16) & 0xff, (*s >> 24) & 0xff);
      else
        dbuf_printf (oBuf, "\t.byte %d,%d,%d,%d\n", (*s >> 24) & 0xff, (*s >> 16) & 0xff, (*s >> 8) & 0xff,(*s >> 0) & 0xff);

      s++;
      pplen++;
    }
  while (pplen < plen)
    {
      dbuf_tprintf (oBuf, "\t!db !constbyte\n", 0);
      pplen++;
    }
}

/*-----------------------------------------------------------------*/
/* return the generic pointer high byte for a given pointer type.  */
/*-----------------------------------------------------------------*/
int
pointerTypeToGPByte (const int p_type, const char *iname, const char *oname)
{
  switch (p_type)
    {
    case IPOINTER:
    case POINTER:
      return GPTYPE_NEAR;
    case GPOINTER:
      werror (W_USING_GENERIC_POINTER, iname ? iname : "<null>", oname ? oname : "<null>");
      return -1;
    case FPOINTER:
      return GPTYPE_FAR;
    case CPOINTER:
    case FUNCTION:
      return GPTYPE_CODE;
    case PPOINTER:
      return GPTYPE_XSTACK;
    default:
      fprintf (stderr, "*** internal error: unknown pointer type %d in GPByte.\n", p_type);
      exit (EXIT_FAILURE);
    }
  return -1;
}

/*-----------------------------------------------------------------*/
/* printIvalVal - generate ival according from value               */
/*-----------------------------------------------------------------*/
static void printIvalVal(struct dbuf_s *oBuf, value *val, int size, bool newline)
{
  if (size == 2 && port->use_dw_for_init)
    {
      dbuf_tprintf (oBuf, "\t!dws\n", aopLiteralLong (val, 0, 2));
      return;
    }

  const bool use_ret = TARGET_PDK_LIKE && !TARGET_IS_PDK16;

  if (!use_ret)
    dbuf_printf (oBuf, "\t.byte ");

  for (int i = 0; i < size; i++)
    {
      const char *inst;
      const char *byte = aopLiteral (val, port->little_endian ? i : size - 1 - i);
      if (use_ret)
        inst = i != size - 1 ? "\tret %s\n" : "\tret %s";
      else
        inst = i != size - 1 ? "%s, " : "%s";
      dbuf_printf (oBuf, inst, byte);
    }
  if (newline)
    dbuf_printf (oBuf, "\n");
}

/*-----------------------------------------------------------------*/
/* _printPointerType - generates ival for pointer type             */
/*-----------------------------------------------------------------*/
static void
_printPointerType (struct dbuf_s *oBuf, const char *name, int size)
{
  wassert (!TARGET_PDK_LIKE);

  if (size == 4)
    {
      if (port->little_endian)
        dbuf_printf (oBuf, "\t.byte %s, (%s >> 8), (%s >> 16), (%s >> 24)", name, name, name, name);
      else
        dbuf_printf (oBuf, "\t.byte (%s >> 24), (%s >> 16), (%s >> 8), %s", name, name, name, name);
    }
  else if (size == 3)
    {
      if (port->little_endian)
        dbuf_printf (oBuf, "\t.byte %s, (%s >> 8), (%s >> 16)", name, name, name);
      else
        dbuf_printf (oBuf, "\t.byte (%s >> 16), (%s >> 8), %s", name, name, name);
    }
  else
    {
      if (port->little_endian)
        dbuf_printf (oBuf, "\t.byte %s, (%s >> 8)", name, name);
      else
        dbuf_printf (oBuf, "\t.byte (%s >> 8), %s", name, name);
    }
}

/*-----------------------------------------------------------------*/
/* printPointerType - generates ival for pointer type              */
/*-----------------------------------------------------------------*/
static void
printPointerType (struct dbuf_s *oBuf, const char *name)
{
  _printPointerType (oBuf, name, (options.model == MODEL_FLAT24) ? 3 : 2);
  dbuf_printf (oBuf, "\n");
}

/*-----------------------------------------------------------------*/
/* printGPointerType - generates ival for generic pointer type     */
/*-----------------------------------------------------------------*/
static void
printGPointerType (struct dbuf_s *oBuf, const char *iname, const char *oname, int type)
{
  int byte = pointerTypeToGPByte (type, iname, oname);
  int size = (options.model == MODEL_FLAT24) ? 3 : 2;
  if (byte == -1)
    {
      _printPointerType (oBuf, iname, size + 1);
      dbuf_printf (oBuf, "\n");
    }
  else
    {
      _printPointerType (oBuf, iname, size);
      dbuf_printf (oBuf, ",#0x%02x\n", byte);
    }
}

/*-----------------------------------------------------------------*/
/* printIvalType - generates ival for int/char                     */
/*-----------------------------------------------------------------*/
void
printIvalType (symbol * sym, sym_link * type, initList * ilist, struct dbuf_s *oBuf)
{
  value *val;
  unsigned long ulVal = 0;

  /* if initList is deep */
  if (ilist && (ilist->type == INIT_DEEP))
    ilist = ilist->init.deep;

  if (!(val = list2val (ilist, FALSE)))
    {
      if (!!(val = initPointer (ilist, type, 0)))
        {
          int i, size = getSize (type), le = port->little_endian, top = (options.model == MODEL_FLAT24) ? 3 : 2;;         
          dbuf_printf (oBuf, "\t.byte ");
          for (i = (le ? 0 : size - 1); le ? (i < size) : (i > -1); i += (le ? 1 : -1))
            {
              if (i == 0)
			    if (val->name && strlen (val->name) > 0)
                  dbuf_printf (oBuf, "%s", val->name);
				else
                  dbuf_printf (oBuf, "#0x00");
              else if (0 < i && i < top)
			    if (val->name && strlen (val->name) > 0)
                  dbuf_printf (oBuf, "(%s >> %d)", val->name, i * 8);
				else
                  dbuf_printf (oBuf, "#0x00");				
              else
                dbuf_printf (oBuf, "#0x00");
              if (i == (le ? (size - 1) : 0))
                dbuf_printf (oBuf, "\n");
              else
                dbuf_printf (oBuf, ", ");
            }
          return;
        }
      else
        {
          werrorfl (ilist->filename, ilist->lineno, E_CONST_EXPECTED);
          val = constCharVal (0);
        }
    }

  if (val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
    {
      werrorfl (ilist->filename, ilist->lineno, E_CONST_EXPECTED);
      val = constCharVal (0);
    }

  /* check if the literal value is within bounds */
  if (checkConstantRange (type, val->etype, '=', FALSE) == CCR_OVL)
    {
      werror (W_LIT_OVERFLOW);
    }

  if (val->type != type)
    {
      val = valCastLiteral (type, floatFromVal (val), (TYPE_TARGET_ULONGLONG) ullFromVal (val));
    }

  if (IS_INTEGRAL (val->type))
    ulVal = ulFromVal (val);

  switch (getSize (type))
    {
    case 1:
      if (!val)
        dbuf_tprintf (oBuf, "\t!db !constbyte\n", 0);
      else
        {
          if (IS_UNSIGNED (val->type))
            {
              dbuf_tprintf (oBuf, "\t!dbs\t; %u", aopLiteral (val, 0), (unsigned int) ulVal);
            }
          else
            {
              dbuf_tprintf (oBuf, "\t!dbs\t; % d", aopLiteral (val, 0), (int) ulVal);
            }
          if (isalnum ((int) ulVal))
            dbuf_tprintf (oBuf, "\t'%c'\n", (int) ulVal);
          else
            dbuf_tprintf (oBuf, "\n");
        }
      break;

    case 2:
      if (port->use_dw_for_init)
        {
          printIvalVal (oBuf, val, 2, true);
          break;
        }
      printIvalVal (oBuf, val, 2, false);
      if (IS_UNSIGNED (val->type))
        dbuf_printf (oBuf, "\t; %u\n", (unsigned int) ulVal);
      else
        dbuf_printf (oBuf, "\t; % d\n", (int) ulVal);
      break;

    case 4:
      if (!val)
        {
          dbuf_tprintf (oBuf, "\t!dw !constword\n", 0);
          dbuf_tprintf (oBuf, "\t!dw !constword\n", 0);
        }
      else
        {
          printIvalVal (oBuf, val, 4, false);

          if (IS_FLOAT (val->type))
            {
              dbuf_printf (oBuf, "\t; % e\n", floatFromVal (val));
            }
          else
            {
              if (IS_UNSIGNED (val->type))
                dbuf_printf (oBuf, "\t; %u\n", (unsigned int) ulVal);
              else
                dbuf_printf (oBuf, "\t; % d\n", (int) ulVal);
            }
        }
      break;
    case 8:
      printIvalVal (oBuf, val, 8, true); // TODO: Print value as comment. Does dbuf_printf support long long even on MSVC?
      break;
    default:
      wassertl (0, "Attempting to initialize integer of non-handled size.");
    }
}

/*-----------------------------------------------------------------*/
/* printIvalBitFields - generate initializer for bitfields         */
/* return the number of bytes written                              */
/*-----------------------------------------------------------------*/
static unsigned int
printIvalBitFields (symbol ** sym, initList ** ilist, struct dbuf_s *oBuf)
{
  symbol *lsym = *sym;
  initList *lilist = *ilist;
  unsigned long ival = 0;
  unsigned size = 0;
  unsigned bit_start = 0;
  unsigned long int bytes_written = 0;

  while (lsym && IS_BITFIELD (lsym->type))
    {
      unsigned bit_length = SPEC_BLEN (lsym->etype);
      if (0 == bit_length)
        {
          /* bit-field structure member with a width of 0 */
          lsym = lsym->next;
          break;
        }
      else if (!SPEC_BUNNAMED (lsym->etype))
        {
          /* not an unnamed bit-field structure member */
          value *val = list2val (lilist, TRUE);

          if (val && val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
            {
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
          if (val && checkConstantRange (lsym->etype, val->etype, '=', FALSE) == CCR_OVL)
            {
              werror (W_LIT_OVERFLOW);
            }

          ival |= (ulFromVal (val) & ((1ul << bit_length) - 1ul)) << bit_start;
          lilist = lilist ? lilist->next : NULL;
        }
      bit_start += bit_length;
      lsym = lsym->next;
      if (lsym && IS_BITFIELD (lsym->type) && (0 == SPEC_BSTR (lsym->etype)))
        {
          /* a new integer */
          break;
        }
    }

  switch (size)
    {
    case 0:
      dbuf_tprintf (oBuf, "\n");
      break;

    case 1:
      dbuf_tprintf (oBuf, "\t!db !constbyte\n", ival);
      bytes_written++;
      break;

    case 2:
      if (TARGET_PDK_LIKE && !TARGET_IS_PDK16)
        {
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", (ival & 0xff));
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", ((ival >> 8) & 0xff));
        }
      else
        dbuf_tprintf (oBuf, "\t!db !constbyte, !constbyte\n", (ival & 0xff), (ival >> 8) & 0xff);
      bytes_written += 2;
      break;

    case 4:
      dbuf_tprintf (oBuf, "\t!db !constbyte, !constbyte, !constbyte, !constbyte\n",
                    (ival & 0xff), (ival >> 8) & 0xff, (ival >> 16) & 0xff, (ival >> 24) & 0xff);
      bytes_written += 4;
      break;
    default:
      wassert (0);
    }
  *sym = lsym;
  *ilist = lilist;

  return (bytes_written);
}

/*-----------------------------------------------------------------*/
/* printIvalStruct - generates initial value for structures        */
/*-----------------------------------------------------------------*/
static void
printIvalStruct (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf)
{
  symbol *sflds;
  initList *iloop = NULL;
  unsigned int skip_holes = 0;

  sflds = SPEC_STRUCT (type)->fields;

  if (ilist)
    {
      if (ilist->type != INIT_DEEP)
        {
          werrorfl (sym->fileDef, sym->lineDef, E_INIT_STRUCT, sym->name);
          return;
        }

      iloop = ilist->init.deep;
    }

  if (SPEC_STRUCT (type)->type == UNION)
    {
      int size;
      /* skip past holes, print value */
      while (iloop && iloop->type == INIT_HOLE)
        {
          iloop = iloop->next;
          sflds = sflds->next;
        }
      printIval (sym, sflds->type, iloop, oBuf, 1);
      /* pad out with zeros if necessary */
      size = getSize(type) - getSize(sflds->type);
      for ( ; size > 0 ; size-- )
        {
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", 0);
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
      while (sflds)
        {
          unsigned int oldoffset = sflds->offset;

          if (IS_BITFIELD (sflds->type))
            printIvalBitFields (&sflds, &iloop, oBuf);
          else
            {
              printIval (sym, sflds->type, iloop, oBuf, 1);
              sflds = sflds->next;
              iloop = iloop ? iloop->next : NULL;
            }

          // Handle members from anonymous unions. Just a hack to fix bug #2643.
          while (sflds && sflds->offset == oldoffset)
            {
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
    werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "struct", sym->name);
}

/*-----------------------------------------------------------------*/
/* printIvalChar - generates initital value for character array    */
/*-----------------------------------------------------------------*/
int
printIvalChar (symbol * sym, sym_link * type, initList * ilist, struct dbuf_s *oBuf, const char *s, bool check)
{
  value *val;
  size_t size = DCL_ELEM(type);
  char *p;
  size_t asz;

  if (!s)
    {
      val = list2val (ilist, TRUE);
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
              printChar (oBuf, p, size);
              free (p);
            }
          else
            printChar (oBuf, SPEC_CVAL (val->etype).v_char, size);

          return 1;
        }
      else
        return 0;
    }
  else
    printChar (oBuf, s, strlen (s) + 1);
  return 1;
}

static size_t strLen16(const TYPE_TARGET_UINT *s)
{
  size_t l = 0;
  while(*s++)
    l++;

  return l;
}

/*-----------------------------------------------------------------*/
/* printIvalChar16 - generates initital value for character array  */
/*-----------------------------------------------------------------*/
int
printIvalChar16 (symbol * sym, sym_link * type, initList * ilist, struct dbuf_s *oBuf, const TYPE_TARGET_UINT *s, bool check)
{
  value *val;
  size_t size = DCL_ELEM(type);
  TYPE_TARGET_UINT *p;
  size_t asz;

  if (!s)
    {
      val = list2val (ilist, TRUE);
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
              printChar16 (oBuf, p, size);
              free (p);
            }
          else
            printChar16 (oBuf, SPEC_CVAL (val->etype).v_char16, size);

          return 1;
        }
      else
        return 0;
    }
  else
    printChar16 (oBuf, s, strLen16 (s) + 1);
  return 1;
}

static size_t strLen32(const TYPE_TARGET_ULONG *s)
{
  size_t l = 0;
  while(*s++)
    l++;
  return l;
}

/*-----------------------------------------------------------------*/
/* printIvalChar32 - generates initital value for character array  */
/*-----------------------------------------------------------------*/
int
printIvalChar32 (symbol * sym, sym_link * type, initList * ilist, struct dbuf_s *oBuf, const TYPE_TARGET_ULONG *s, bool check)
{
  value *val;
  size_t size = DCL_ELEM(type);
  TYPE_TARGET_ULONG *p;
  size_t asz;

  if (!s)
    {
      val = list2val (ilist, TRUE);
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
              printChar32 (oBuf, p, size);
              free (p);
            }
          else
            printChar32 (oBuf, SPEC_CVAL (val->etype).v_char32, size);

          return 1;
        }
      else
        return 0;
    }
  else
    printChar32 (oBuf, s, strLen32 (s) + 1);
  return 1;
}

/*-----------------------------------------------------------------*/
/* printIvalArray - generates code for array initialization        */
/*-----------------------------------------------------------------*/
void
printIvalArray (symbol * sym, sym_link * type, initList * ilist, struct dbuf_s *oBuf, bool check)
{
  value *val;
  initList *iloop;
  unsigned int size = 0;

  if (ilist)
    {
      /* take care of the special   case  */
      /* array of characters can be init  */
      /* by a string                      */
      /* char *p = "abc";                 */
      if ((IS_CHAR (type->next) || IS_INT (type->next)) && ilist->type == INIT_NODE)
        {
          val = list2val (ilist, TRUE);
          if (!val)
            {
              werrorfl (ilist->filename, ilist->lineno, E_INIT_STRUCT, sym->name);
              return;
            }
          if (!IS_LITERAL (val->etype))
            {
              werrorfl (ilist->filename, ilist->lineno, E_CONST_EXPECTED);
              return;
            }
          if (IS_CHAR (type->next) && printIvalChar (sym, type, ilist, oBuf, SPEC_CVAL (sym->etype).v_char, check))
            return;
          if (IS_INT (type->next) && IS_UNSIGNED (type->next))
            {
              if (!IS_LONG (type->next) && printIvalChar16 (sym, type, ilist, oBuf, SPEC_CVAL (sym->etype).v_char16, check))
                return;
              if (IS_LONG (type->next) && printIvalChar32 (sym, type, ilist, oBuf, SPEC_CVAL (sym->etype).v_char32, check))
                return;
            }
        }
      /* char *p = {"abc"}; */
      if ((IS_CHAR (type->next) || IS_INT (type->next)) && ilist->type == INIT_DEEP && ilist->init.deep && ilist->init.deep->type == INIT_NODE)
        {
          val = list2val (ilist->init.deep, TRUE);
          if (!val)
            {
              werrorfl (ilist->init.deep->filename, ilist->init.deep->lineno, E_INIT_STRUCT, sym->name);
              return;
            }
          if (!IS_LITERAL (val->etype))
            {
              werrorfl (ilist->init.deep->filename, ilist->init.deep->lineno, E_CONST_EXPECTED);
              return;
            }
          if (IS_CHAR (type->next) && printIvalChar (sym, type, ilist->init.deep, oBuf, SPEC_CVAL (sym->etype).v_char, check))
            return;
          if (IS_INT (type->next) && IS_UNSIGNED (type->next))
            {
              if (!IS_LONG (type->next) && printIvalChar16 (sym, type, ilist->init.deep, oBuf, SPEC_CVAL (sym->etype).v_char16, check))
                return;
              if (IS_LONG (type->next) && printIvalChar32 (sym, type, ilist->init.deep, oBuf, SPEC_CVAL (sym->etype).v_char32, check))
                return;
            }
        }

      /* not the special case             */
      if (ilist->type != INIT_DEEP)
        {
          werrorfl (ilist->filename, ilist->lineno, E_INIT_STRUCT, sym->name);
          return;
        }

      for (iloop = ilist->init.deep; iloop; iloop = iloop->next)
        {
          if ((++size > DCL_ELEM (type)) && DCL_ELEM (type))
            {
              werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "array", sym->name);
              break;
            }
          printIval (sym, type->next, iloop, oBuf, TRUE);
        }
    }

  if (DCL_ELEM (type))
    {
      // pad with zeros if needed
      if (size < DCL_ELEM (type))
        {
          size = (DCL_ELEM (type) - size) * getSize (type->next);
          while (size--)
            {
              dbuf_tprintf (oBuf, "\t!db !constbyte\n", 0);
            }
        }
    }
  else
    {
      /* we have not been given a size, but now we know it */
      /* but first check, if it's a flexible array */
      if (IS_STRUCT (sym->type))
        sym->flexArrayLength = size * getSize (type->next);
      else
        DCL_ELEM (type) = size;
    }

  return;
}

/*-----------------------------------------------------------------*/
/* printIvalFuncPtr - generate initial value for function pointers */
/*-----------------------------------------------------------------*/
void
printIvalFuncPtr (sym_link * type, initList * ilist, struct dbuf_s *oBuf)
{
  value *val;
  char *name;
  int size;

  if (ilist)
    val = list2val (ilist, TRUE);
  else
    val = valCastLiteral (type, 0.0, 0);

  if (!val)
    {
      // an error has been thrown already
      val = constCharVal (0);
    }

  if (IS_LITERAL (val->etype))
    {
      if (compareType (type, val->type) == 0)
        {
          if (ilist)
            werrorfl (ilist->filename, ilist->lineno, E_INCOMPAT_TYPES);
          else
            werror (E_INCOMPAT_TYPES);
          printFromToType (val->type, type);
        }
      printIvalCharPtr (NULL, type, val, oBuf);
      return;
    }

  /* now generate the name */
  if (!val->sym)
    name = val->name;
  else
    name = val->sym->rname;

  size = getSize (type);

  if (size == FUNCPTRSIZE)
    {
      if (TARGET_PDK_LIKE)
        {
          dbuf_printf (oBuf, "\tret #<%s\n", name);
          dbuf_printf (oBuf, "\tret #>%s\n", name);
        }
      else if (TARGET_IS_STM8 && FUNCPTRSIZE == 3)
        {
          _printPointerType (oBuf, name, size);
          dbuf_printf (oBuf, "\n");
        }
      else if (port->use_dw_for_init)
        {
          dbuf_tprintf (oBuf, "\t!dws\n", name);
        }
      else
        {
          printPointerType (oBuf, name);
        }
    }
  else if (size == BFUNCPTRSIZE)
    {
      _printPointerType (oBuf, name, size);
      dbuf_printf (oBuf, "\n");
    }
  else
    {
      wassertl (0, "Invalid function pointer size.");
    }

  return;
}

/*--------------------------------------------------------------------*/
/* printIvalCharPtr - generates initial values for character pointers */
/*--------------------------------------------------------------------*/
int
printIvalCharPtr (symbol * sym, sym_link * type, value * val, struct dbuf_s *oBuf)
{
  int size = 0;
  char *p;

  if (val && !!(p = (char *) malloc (strlen (val->name) + 1)))
    {
      strcpy (p, val->name);
      addSet (&ccpStr, p);
    }

  /* PENDING: this is _very_ mcs51 specific, including a magic
     number...
     It's also endian specific.
   */
  size = getSize (type);

  if (val->name && strlen (val->name))
    {
      if (size == 1)            /* This appears to be Z80 specific?? */
        {
          dbuf_tprintf (oBuf, "\t!dbs\n", val->name);
        }
      else if (size == FARPTRSIZE)
        {
          if (TARGET_PDK_LIKE && !TARGET_IS_PDK16)
            {
              dbuf_printf (oBuf, "\tret #<%s\n", val->name);
              dbuf_printf (oBuf, IN_CODESPACE (SPEC_OCLS (val->etype)) ? "\tret #>(%s + 0x8000)\n" : "\tret #0\n", val->name);
            }
          else if (port->use_dw_for_init)
            dbuf_tprintf (oBuf, "\t!dws\n", val->name);
          else
            printPointerType (oBuf, val->name);
        }
      else if (size == GPTRSIZE)
        {
          int type;
          if (IS_PTR (val->type))
            {
              type = DCL_TYPE (val->type);
            }
          else
            {
              type = PTR_TYPE (SPEC_OCLS (val->etype));
            }
          if (val->sym && val->sym->isstrlit)
            {
              // this is a literal string
              type = CPOINTER;
            }
          printGPointerType (oBuf, val->name, sym->name, type);
        }
      else
        {
          fprintf (stderr, "*** internal error: unknown size in " "printIvalCharPtr.\n");
        }
    }
  else
    {
      // these are literals assigned to pointers
      switch (size)
        {
        case 1:
          dbuf_tprintf (oBuf, "\t!dbs\n", aopLiteral (val, 0));
          break;
        case 2:
          if (TARGET_PDK_LIKE && !TARGET_IS_PDK16)
            {
              dbuf_tprintf (oBuf, "\tret %s\n", aopLiteral (val, 0));
              dbuf_tprintf (oBuf, "\tret %s\n", aopLiteral (val, 1));
            }
          else if (port->use_dw_for_init)
            dbuf_tprintf (oBuf, "\t!dws\n", aopLiteralLong (val, 0, size));
          else if (port->little_endian)
            dbuf_tprintf (oBuf, "\t.byte %s,%s\n", aopLiteral (val, 0), aopLiteral (val, 1));
          else
            dbuf_tprintf (oBuf, "\t.byte %s,%s\n", aopLiteral (val, 1), aopLiteral (val, 0));
          break;
        case 3:
          if (IS_GENPTR (type) && GPTRSIZE > FARPTRSIZE && floatFromVal (val) != 0)
            {
              if (!IS_PTR (val->type) && !IS_FUNC (val->type))
                {
                  // non-zero mcs51 generic pointer
                  werrorfl (sym->fileDef, sym->lineDef, W_LITERAL_GENERIC);
                }
              if (port->little_endian)
                dbuf_printf (oBuf, "\t.byte %s,%s,%s\n", aopLiteral (val, 0), aopLiteral (val, 1), aopLiteralGptr (sym->name, val));
              else
                dbuf_printf (oBuf, "\t.byte %s,%s,%s\n", aopLiteralGptr (sym->name, val), aopLiteral (val, 1), aopLiteral (val, 0));
            }
          else
            {
              if (port->little_endian)
                dbuf_printf (oBuf, "\t.byte %s,%s,%s\n", aopLiteral (val, 0), aopLiteral (val, 1), aopLiteral (val, 2));
              else
                dbuf_printf (oBuf, "\t.byte %s,%s,%s\n", aopLiteral (val, 2), aopLiteral (val, 1), aopLiteral (val, 0));
            }
          break;
        case 4:
          if (IS_GENPTR (type) && GPTRSIZE > FARPTRSIZE && floatFromVal (val) != 0)
            {
              if (!IS_PTR (val->type) && !IS_FUNC (val->type))
                {
                  // non-zero ds390 generic pointer
                  werrorfl (sym->fileDef, sym->lineDef, W_LITERAL_GENERIC);
                }
              if (port->little_endian)
                {
                  dbuf_printf (oBuf, "\t.byte %s,%s,%s", aopLiteral (val, 0), aopLiteral (val, 1), aopLiteral (val, 2));
                  if (IS_PTR (val->type) && !IS_GENPTR (val->type))
                    dbuf_tprintf (oBuf, ",!immedbyte\n", pointerTypeToGPByte (DCL_TYPE (val->type), val->name, sym->name));
                  else
                    dbuf_printf (oBuf, ",%s\n", aopLiteral (val, 3));
                }
              else
                {
                  if (IS_PTR (val->type) && !IS_GENPTR (val->type))
                    dbuf_tprintf (oBuf, "\t.byte !immedbyte\n", pointerTypeToGPByte (DCL_TYPE (val->type), val->name, sym->name));
                  else
                    dbuf_printf (oBuf, "\t.byte %s\n", aopLiteral (val, 3));
                  dbuf_printf (oBuf, ",%s,%s,%s", aopLiteral (val, 2), aopLiteral (val, 1), aopLiteral (val, 0));
                }
            }
          else
            {
              if (port->little_endian)
                {
                  dbuf_printf (oBuf, "\t.byte %s,%s,%s,%s\n",
                               aopLiteral (val, 0), aopLiteral (val, 1), aopLiteral (val, 2), aopLiteral (val, 3));
                }
              else
                {
                  dbuf_printf (oBuf, "\t.byte %s,%s,%s,%s\n",
                               aopLiteral (val, 3), aopLiteral (val, 2), aopLiteral (val, 1), aopLiteral (val, 0));
                }
            }
          break;
        default:
          assert (0);
        }
    }

  if (!noInit && val->sym && val->sym->isstrlit && !isinSet (statsg->syms, val->sym))
    {
      addSet (&statsg->syms, val->sym);
    }

  return 1;
}

/*-----------------------------------------------------------------*/
/* printIvalPtr - generates initial value for pointers             */
/*-----------------------------------------------------------------*/
void
printIvalPtr (symbol *sym, sym_link *type, initList *ilist, struct dbuf_s *oBuf)
{
  value *val;
  int size;

  /* if deep then   */
  if (ilist && (ilist->type == INIT_DEEP))
    ilist = ilist->init.deep;

  /* function pointer     */
  if (IS_FUNC (type->next))
    {
      printIvalFuncPtr (type, ilist, oBuf);
      return;
    }

  if (!(val = initPointer (ilist, type, 1)))
    return;

  /* if character pointer */
  if (IS_CHAR (type->next) || IS_INT (type->next) && IS_UNSIGNED (type->next))
    if (printIvalCharPtr (sym, type, val, oBuf))
      return;

  /* check the type      */
  if (compareType (type, val->type) == 0)
    {
      assert (ilist != NULL);
      werrorfl (ilist->filename, ilist->lineno, W_INIT_WRONG);
      printFromToType (val->type, type);
    }

  /* if val is literal */
  if (IS_LITERAL (val->etype))
    {
      switch (getSize (type))
        {
        case 1:
          dbuf_tprintf (oBuf, "\t!db !constbyte\n", (unsigned int) ulFromVal (val) & 0xff);
          break;
        case 2:
          if (port->use_dw_for_init)
            dbuf_tprintf (oBuf, "\t!dws\n", aopLiteralLong (val, 0, 2));
          else if (port->little_endian)
            dbuf_tprintf (oBuf, "\t.byte %s,%s\n", aopLiteral (val, 0), aopLiteral (val, 1));
          else
            dbuf_tprintf (oBuf, "\t.byte %s,%s\n", aopLiteral (val, 1), aopLiteral (val, 0));
          break;
        case 3:
          dbuf_printf (oBuf, "; generic printIvalPtr\n");
          if (port->little_endian)
            dbuf_printf (oBuf, "\t.byte %s,%s", aopLiteral (val, 0), aopLiteral (val, 1));
          else
            dbuf_printf (oBuf, "\t.byte %s,%s", aopLiteral (val, 1), aopLiteral (val, 0));
          if (IS_GENPTR (val->type))
            dbuf_printf (oBuf, ",%s\n", aopLiteral (val, 2));
          else if (IS_PTR (val->type))
            dbuf_tprintf (oBuf, ",!immedbyte\n", pointerTypeToGPByte (DCL_TYPE (val->type), val->name, sym->name));
          else
            dbuf_printf (oBuf, ",%s\n", aopLiteral (val, 2));
          break;
        case 4:
          if (TARGET_IS_DS390 || TARGET_IS_DS400)
            dbuf_printf (oBuf, "\t.byte %s,%s,%s,%s\n", aopLiteral (val, 0), aopLiteral (val, 1), aopLiteral (val, 2), aopLiteral (val, 3));
          else
            wassertl(0, "Printing pointer of invalid size");
          break;
       default:
          wassertl(0, "Printing pointer of invalid size");
          break;
        }
      return;
    }

  size = getSize (type);

  if (TARGET_PDK_LIKE && size == 2)
    {
      if (IN_CODESPACE (SPEC_OCLS (val->etype)))
        {
          dbuf_printf (oBuf, "\tret #<%s\n", val->name);
          dbuf_printf (oBuf, "\tret #>(%s + 0x8000)\n", val->name);
        }
      else
        {
          dbuf_printf (oBuf, "\tret #%s\n", val->name);
          dbuf_printf (oBuf, "\tret #0\n");
        }
    }
  else if (size == 1)                /* Z80 specific?? */
    {
      dbuf_tprintf (oBuf, "\t!dbs\n", val->name);
    }
  else if (size == FARPTRSIZE)
    {
      if (port->use_dw_for_init)
        dbuf_tprintf (oBuf, "\t!dws\n", val->name);
      else
        printPointerType (oBuf, val->name);
    }
  else if (size == GPTRSIZE)
    {
      printGPointerType (oBuf, val->name, sym->name,
                         (IS_PTR (val->type) ? DCL_TYPE (val->type) : PTR_TYPE (SPEC_OCLS (val->etype))));
    }

  return;
}

/*-----------------------------------------------------------------*/
/* printIval - generates code for initial value                    */
/*-----------------------------------------------------------------*/
void
printIval (symbol * sym, sym_link * type, initList * ilist, struct dbuf_s *oBuf, bool check)
{
  sym_link *itype;

  /* Handle designated initializers */
  if (ilist && ilist->type==INIT_DEEP)
    ilist = reorderIlist (type, ilist);

  /* If this is a hole, substitute an appropriate initializer. */
  if (ilist && ilist->type == INIT_HOLE)
    {
      if (IS_AGGREGATE (type))
        {
          ilist = newiList(INIT_DEEP, NULL); /* init w/ {} */
        }
      else
        {
          ast *ast = newAst_VALUE (constVal("0"));
          ast = decorateType (ast, RESULT_TYPE_NONE);
          ilist = newiList(INIT_NODE, ast);
        }
    }

  /* if structure then    */
  if (IS_STRUCT (type))
    {
      printIvalStruct (sym, type, ilist, oBuf);
      return;
    }

  /* if this is an array  */
  if (IS_ARRAY (type))
    {
      printIvalArray (sym, type, ilist, oBuf, check);
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
              werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "scalar", sym->name);
            }
          else
            {
              ilist = ilist->init.deep;
            }
        }

      // and the type must match
      itype = ilist->init.node->ftype;

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
              if (ilist)
                werrorfl (ilist->filename, ilist->lineno, E_INCOMPAT_TYPES);
              else
                werror (E_INCOMPAT_TYPES);
              printFromToType (itype, type->next);
            }
          else
            {
              werrorfl (ilist->filename, ilist->lineno, E_TYPE_MISMATCH, "assignment", " ");
              printFromToType (itype, type);
            }
        }
    }

  /* if this is a pointer */
  if (IS_PTR (type))
    {
      printIvalPtr (sym, type, ilist, oBuf);
      return;
    }

  /* if type is SPECIFIER */
  if (IS_SPEC (type))
    {
      printIvalType (sym, type, ilist, oBuf);
      return;
    }
}

/*-----------------------------------------------------------------*/
/* emitStaticSeg - emitcode for the static segment                 */
/*-----------------------------------------------------------------*/
void
emitStaticSeg (memmap *map, struct dbuf_s *oBuf)
{
  symbol *sym;
  set *tmpSet = NULL;

  /* fprintf(out, "\t.area\t%s\n", map->sname); */

  /* eliminate redundant __str_%d (generated in stringToSymbol(), SDCCast.c) */
  for (sym = setFirstItem (map->syms); sym; sym = setNextItem (map->syms))
    addSet (&tmpSet, sym);

  /* for all variables in this segment do */
  for (sym = setFirstItem (map->syms); sym; sym = setNextItem (map->syms))
    {
      /* if it is "extern" then do nothing */
      if (IS_EXTERN (sym->etype) && !sym->ival)
        continue;

      /* eliminate redundant __str_%d (generated in stringToSymbol(), SDCCast.c) */
      if (!isinSet (tmpSet, sym))
        {
          const char *p;
          if (!ccpStr)
            continue;
          for (p = setFirstItem (ccpStr); p; p = setNextItem (ccpStr))
            if (strcmp (p, sym->name) == 0)
              break;
          if (!p)
            continue;
        }

      /* if it is not static add it to the public table */
      if (!IS_STATIC (sym->etype))
        {
          addSetHead (&publics, sym);
        }

      /* if it has an absolute address and no initializer */
      if (SPEC_ABSA (sym->etype) && !sym->ival)
        {
          if (options.debug)
            {
              emitDebugSym (oBuf, sym);
              dbuf_printf (oBuf, " == 0x%04x\n", SPEC_ADDR (sym->etype));
            }
          dbuf_printf (oBuf, "%s\t=\t0x%04x\n", sym->rname, SPEC_ADDR (sym->etype));
        }
      else
        {
          int size = getSize (sym->type);

          if (size == 0)
            {
              werrorfl (sym->fileDef, sym->lineDef, E_UNKNOWN_SIZE, sym->name);
            }
          /* if it has an initial value */
          if (sym->ival)
            {
              if (SPEC_ABSA (sym->etype))
                {
                  dbuf_tprintf (oBuf, "\t!org\n", SPEC_ADDR (sym->etype));
                }
              if (options.debug)
                {
                  emitDebugSym (oBuf, sym);
                  dbuf_printf (oBuf, " == .\n");
                }
              dbuf_printf (oBuf, "%s:\n", sym->rname);
              ++noAlloc;
              resolveIvalSym (sym->ival, sym->type);
              printIval (sym, sym->type, sym->ival, oBuf, (map != xinit && map != initializer));
              --noAlloc;
              /* if sym is a simple string and sym->ival is a string,
                 WE don't need it anymore */
              if (IS_ARRAY (sym->type) && IS_CHAR (sym->type->next) &&
                  IS_AST_SYM_VALUE (list2expr (sym->ival)) && list2val (sym->ival, TRUE)->sym->isstrlit)
                {
                  freeStringSymbol (list2val (sym->ival, TRUE)->sym);
                }
            }
          else
            {
              /* allocate space */
              if (options.debug)
                {
                  emitDebugSym (oBuf, sym);
                  dbuf_printf (oBuf, " == .\n");
                }
              /* special case for character strings */
              if (IS_ARRAY (sym->type) &&
                (IS_CHAR (sym->type->next) && SPEC_CVAL (sym->etype).v_char ||
                 IS_INT (sym->type->next) && !IS_LONG (sym->type->next) && SPEC_CVAL (sym->etype).v_char16 ||
                 IS_INT (sym->type->next) && IS_LONG (sym->type->next) && SPEC_CVAL (sym->etype).v_char32))
                {
                  if (options.const_seg)
                    dbuf_tprintf(&code->oBuf, "\t!area\n", options.const_seg);
                  dbuf_printf (oBuf, "%s:\n", sym->rname);
                  if (IS_CHAR (sym->type->next))
                    printChar (oBuf, SPEC_CVAL (sym->etype).v_char, size);
                  else if (IS_INT (sym->type->next) && !IS_LONG (sym->type->next))
                    printChar16 (oBuf, SPEC_CVAL (sym->etype).v_char16, size / 2);
                  else if (IS_INT (sym->type->next) && IS_LONG (sym->type->next))
                    printChar32 (oBuf, SPEC_CVAL (sym->etype).v_char32, size / 4);
                  else
                    wassert (0);
                  if (options.const_seg)
                    dbuf_tprintf(oBuf, "\t!areacode\n", options.code_seg);
                }
              else
                {
                  dbuf_printf (oBuf, "%s:\n", sym->rname);
                  dbuf_tprintf (oBuf, "\t!ds\n", (unsigned int) size & 0xffff);
                }
            }
        }
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

/*-----------------------------------------------------------------*/
/* emitMaps - emits the code for the data portion the code         */
/*-----------------------------------------------------------------*/
void
emitMaps (void)
{
  namedspacemap *nm;
  int publicsfr = TARGET_IS_MCS51 || TARGET_PDK_LIKE;      /* Ideally, this should be true for all  */
                                                           /* ports but let's be conservative - EEP */

  inInitMode++;
  /* no special considerations for the following
     data, idata & bit & xdata */
  emitRegularMap (data, TRUE, TRUE);
  emitRegularMap (initialized, TRUE, TRUE);
  for (nm = namedspacemaps; nm; nm = nm->next)
    if (nm->is_const)
      {
        dbuf_tprintf (&nm->map->oBuf, "\t!areacode\n", nm->map->sname);
        emitStaticSeg (nm->map, &nm->map->oBuf);
      }
    else
      emitRegularMap (nm->map, TRUE, TRUE);
  emitRegularMap (idata, TRUE, TRUE);
  emitRegularMap (d_abs, TRUE, TRUE);
  emitRegularMap (i_abs, TRUE, TRUE);
  emitRegularMap (bit, TRUE, TRUE);
  emitRegularMap (pdata, TRUE, TRUE);
  emitRegularMap (xdata, TRUE, TRUE);
  emitRegularMap (x_abs, TRUE, TRUE);
  if (port->genXINIT)
    {
      emitRegularMap (xidata, TRUE, TRUE);
    }
  emitRegularMap (sfr, publicsfr, FALSE);
  emitRegularMap (sfrbit, publicsfr, FALSE);
  emitRegularMap (home, TRUE, FALSE);
  emitRegularMap (code, TRUE, FALSE);

  if (options.const_seg)
    dbuf_tprintf (&code->oBuf, "\t!area\n", options.const_seg);
  emitStaticSeg (statsg, &code->oBuf);

  if (port->genXINIT)
    {
      dbuf_tprintf (&code->oBuf, "\t!area\n", xinit->sname);
      emitStaticSeg (xinit, &code->oBuf);
    }
  if (initializer)
    {
      dbuf_tprintf (&code->oBuf, "\t!area\n", initializer->sname);
      emitStaticSeg (initializer, &code->oBuf);
    }
  dbuf_tprintf (&code->oBuf, "\t!area\n", c_abs->sname);
  emitStaticSeg (c_abs, &code->oBuf);
  inInitMode--;
}

/*-----------------------------------------------------------------*/
/* flushStatics - flush all currently defined statics out to file  */
/*  and delete.  Temporary function                                */
/*-----------------------------------------------------------------*/
void
flushStatics (void)
{
  emitStaticSeg (statsg, codeOutBuf);
  statsg->syms = NULL;
}

/*-----------------------------------------------------------------*/
/* createInterruptVect - creates the interrupt vector              */
/*-----------------------------------------------------------------*/
void
createInterruptVect (struct dbuf_s *vBuf)
{
  mainf = newSymbol ("main", 0);
  mainf->block = 0;

  /* only if the main function exists */
  if (!(mainf = findSymWithLevel (SymbolTab, mainf)))
    {
      if (!options.cc_only && !noAssemble && !options.c1mode)
        werror (E_NO_MAIN);
      return;
    }

  /* if the main is only a prototype ie. no body then do nothing */
  if (!IFFUNC_HASBODY (mainf->type))
    {
      /* if ! compile only then main function should be present */
      if (!options.cc_only && !noAssemble)
        werror (E_NO_MAIN);
      return;
    }

  dbuf_tprintf (vBuf, "\t!areacode\n", HOME_NAME);
  dbuf_printf (vBuf, "__interrupt_vect:\n");


  if (!port->genIVT || !(port->genIVT (vBuf, interrupts, maxInterrupts)))
    {
      /* There's no such thing as a "generic" interrupt table header. */
      wassert (0);
    }
}

char *iComments1 = {
  ";--------------------------------------------------------\n"
  "; File Created by SDCC : free open source ANSI-C Compiler\n"
};

char *iComments2 = {
  ";--------------------------------------------------------\n"
};


/*-----------------------------------------------------------------*/
/* initialComments - puts in some initial comments                 */
/*-----------------------------------------------------------------*/
void
initialComments (FILE * afile)
{
  fprintf (afile, "%s", iComments1);
  fprintf (afile, "; Version " SDCC_VERSION_STR " #%s (%s)\n", getBuildNumber (), getBuildEnvironment ());
  fprintf (afile, "%s", iComments2);
}

/*-----------------------------------------------------------------*/
/* printPublics - generates .global for publics                    */
/*-----------------------------------------------------------------*/
void
printPublics (FILE * afile)
{
  symbol *sym;

  fprintf (afile, "%s", iComments2);
  fprintf (afile, "; Public variables in this module\n");
  fprintf (afile, "%s", iComments2);

  for (sym = setFirstItem (publics); sym; sym = setNextItem (publics))
    tfprintf (afile, "\t!global\n", sym->rname);
}

/*-----------------------------------------------------------------*/
/* printExterns - generates .global for externs                    */
/*-----------------------------------------------------------------*/
void
printExterns (FILE * afile)
{
  symbol *sym;

  fprintf (afile, "%s", iComments2);
  fprintf (afile, "; Externals used\n");
  fprintf (afile, "%s", iComments2);

  for (sym = setFirstItem (externs); sym; sym = setNextItem (externs))
    tfprintf (afile, "\t!extern\n", sym->rname);
}

/*-----------------------------------------------------------------*/
/* emitOverlay - will emit code for the overlay stuff              */
/*-----------------------------------------------------------------*/
static void
emitOverlay (struct dbuf_s *aBuf)
{
  set *ovrset;

  /* for each of the sets in the overlay segment do */
  for (ovrset = setFirstItem (ovrSetSets); ovrset; ovrset = setNextItem (ovrSetSets))
    {
      symbol *sym;

      if (elementsInSet (ovrset))
        {
          /* output the area information */
          dbuf_printf (aBuf, "\t.area\t%s\n", port->mem.overlay_name);  /* MOF */
        }

      for (sym = setFirstItem (ovrset); sym; sym = setNextItem (ovrset))
        {
          /* if extern then it is in the publics table: do nothing */
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
            {
              addSetHead (&publics, sym);
            }

          /* if extern then do nothing or is a function
             then do nothing */
          if (IS_FUNC (sym->type))
            continue;

          /* if is has an absolute address then generate
             an equate for this no need to allocate space */
          if (SPEC_ABSA (sym->etype))
            {
              /* print extra debug info if required */
              if (options.debug)
                {
                  emitDebugSym (aBuf, sym);
                  dbuf_printf (aBuf, " == 0x%04x\n", SPEC_ADDR (sym->etype));
                }
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
              if (options.debug)
                {
                  emitDebugSym (aBuf, sym);
                  dbuf_printf (aBuf, "==.\n");
                }

              /* allocate space */
              dbuf_tprintf (aBuf, "!slabeldef\n", sym->rname);
              dbuf_tprintf (aBuf, "\t!ds\n", (unsigned int) getSize (sym->type) & 0xffff);
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* glue - the final glue that hold the whole thing together        */
/*-----------------------------------------------------------------*/
void
glue (void)
{
  struct dbuf_s vBuf;
  struct dbuf_s ovrBuf;
  struct dbuf_s asmFileName;
  FILE *asmFile;
  int mcs51_like;
  namedspacemap *nm;

  dbuf_init (&vBuf, 4096);
  dbuf_init (&ovrBuf, 4096);

  mcs51_like = (port->general.glue_up_main &&
                (TARGET_IS_MCS51 || TARGET_IS_DS390 || TARGET_IS_DS400));

  /* print the global struct definitions */
  if (options.debug)
    cdbStructBlock (0);

  /* PENDING: this isn't the best place but it will do */
  if (port->general.glue_up_main)
    {
      /* create the interrupt vector table */
      createInterruptVect (&vBuf);
    }

  /* emit code for the all the variables declared */
  emitMaps ();
  /* do the overlay segments */
  emitOverlay (&ovrBuf);

  outputDebugSymbols ();

  /* now put it all together into the assembler file */
  /* create the assembler file name */

  /* -o option overrides default name? */
  dbuf_init (&asmFileName, PATH_MAX);
  if ((noAssemble || options.c1mode) && fullDstFileName)
    {
      dbuf_append_str (&asmFileName, fullDstFileName);
    }
  else
    {
      dbuf_append_str (&asmFileName, dstFileName);
      dbuf_append_str (&asmFileName, port->assembler.file_ext);
    }

  if (!(asmFile = fopen (dbuf_c_str (&asmFileName), "w")))
    {
      werror (E_FILE_OPEN_ERR, dbuf_c_str (&asmFileName));
      dbuf_destroy (&asmFileName);
      exit (EXIT_FAILURE);
    }
  dbuf_destroy (&asmFileName);

  /* initial comments */
  initialComments (asmFile);

  if (TARGET_IS_S08)
    fprintf (asmFile, "\t.cs08\n");
  else if (TARGET_IS_Z180)
    fprintf (asmFile, "\t.hd64\n");
  else if (TARGET_IS_R3KA)
    fprintf (asmFile, "\t.r3k\n");
  else if (TARGET_IS_EZ80_Z80)
    fprintf (asmFile, "\t.ez80\n");

  /* print module name */
  tfprintf (asmFile, "\t!module\n", moduleName);
  if (mcs51_like)
    {
      if(!options.noOptsdccInAsm)
        fprintf (asmFile, "\t.optsdcc -m%s", port->target);

      switch (options.model)
        {
        case MODEL_SMALL:
          fprintf (asmFile, " --model-small");
          break;
        case MODEL_COMPACT:
          fprintf (asmFile, " --model-compact");
          break;
        case MODEL_MEDIUM:
          fprintf (asmFile, " --model-medium");
          break;
        case MODEL_LARGE:
          fprintf (asmFile, " --model-large");
          break;
        case MODEL_FLAT24:
          fprintf (asmFile, " --model-flat24");
          break;
        case MODEL_HUGE:
          fprintf (asmFile, " --model-huge");
          break;
        default:
          break;
        }
      /*if(options.stackAuto)      fprintf (asmFile, " --stack-auto"); */
      if (options.useXstack)
        fprintf (asmFile, " --xstack");
      /*if(options.intlong_rent)   fprintf (asmFile, " --int-long-rent"); */
      /*if(options.float_rent)     fprintf (asmFile, " --float-rent"); */
      if (options.noRegParams)
        fprintf (asmFile, " --no-reg-params");
      if (options.parms_in_bank1)
        fprintf (asmFile, " --parms-in-bank1");
      if (options.all_callee_saves)
        fprintf (asmFile, " --all-callee-saves");
      fprintf (asmFile, "\n");
    }
  else if (!TARGET_PIC_LIKE && !options.noOptsdccInAsm)
    {
      fprintf (asmFile, "\t.optsdcc -m%s\n", port->target);
    }

  tfprintf (asmFile, "\t!fileprelude\n");

  /* Let the port generate any global directives, etc. */
  if (port->genAssemblerPreamble)
    {
      port->genAssemblerPreamble (asmFile);
    }

  /* print the global variables in this module */
  printPublics (asmFile);
  if (port->assembler.externGlobal)
    printExterns (asmFile);

  if ((mcs51_like) || (TARGET_IS_Z80 || TARGET_IS_GBZ80 || TARGET_IS_Z180 || TARGET_IS_RABBIT || TARGET_IS_EZ80_Z80) || TARGET_PDK_LIKE)  /*.p.t.20030924 need to output SFR table for Z80 as well */
    {
      /* copy the sfr segment */
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; special function registers\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&sfr->oBuf, asmFile);
    }

  if (mcs51_like)
    {
      /* copy the sbit segment */
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; special function bits\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&sfrbit->oBuf, asmFile);

      /*JCF: Create the areas for the register banks */
      if (RegBankUsed[0] || RegBankUsed[1] || RegBankUsed[2] || RegBankUsed[3])
        {
          fprintf (asmFile, "%s", iComments2);
          fprintf (asmFile, "; overlayable register banks\n");
          fprintf (asmFile, "%s", iComments2);
          if (RegBankUsed[0])
            fprintf (asmFile, "\t.area REG_BANK_0\t(REL,OVR,DATA)\n\t.ds 8\n");
          if (RegBankUsed[1] || options.parms_in_bank1)
            fprintf (asmFile, "\t.area REG_BANK_1\t(REL,OVR,DATA)\n\t.ds 8\n");
          if (RegBankUsed[2])
            fprintf (asmFile, "\t.area REG_BANK_2\t(REL,OVR,DATA)\n\t.ds 8\n");
          if (RegBankUsed[3])
            fprintf (asmFile, "\t.area REG_BANK_3\t(REL,OVR,DATA)\n\t.ds 8\n");
        }
      if (BitBankUsed)
        {
          fprintf (asmFile, "%s", iComments2);
          fprintf (asmFile, "; overlayable bit register bank\n");
          fprintf (asmFile, "%s", iComments2);
          fprintf (asmFile, "\t.area BIT_BANK\t(REL,OVR,DATA)\n");
          fprintf (asmFile, "bits:\n\t.ds 1\n");
          fprintf (asmFile, "\tb0 = bits[0]\n");
          fprintf (asmFile, "\tb1 = bits[1]\n");
          fprintf (asmFile, "\tb2 = bits[2]\n");
          fprintf (asmFile, "\tb3 = bits[3]\n");
          fprintf (asmFile, "\tb4 = bits[4]\n");
          fprintf (asmFile, "\tb5 = bits[5]\n");
          fprintf (asmFile, "\tb6 = bits[6]\n");
          fprintf (asmFile, "\tb7 = bits[7]\n");
        }
    }

  /* copy the data segment */
  fprintf (asmFile, "%s", iComments2);
  fprintf (asmFile, ";%s ram data\n", mcs51_like ? " internal" : "");
  fprintf (asmFile, "%s", iComments2);
  dbuf_write_and_destroy (&data->oBuf, asmFile);

  /* copy the initialized segment */
  if (initialized)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, ";%s ram data\n", mcs51_like ? " internal" : "");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&initialized->oBuf, asmFile);
    }

  /* copy segments for named address spaces */
  for (nm = namedspacemaps; nm; nm = nm->next)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; %s %s data\n", nm->name, nm->is_const ? "rom" : "ram");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&nm->map->oBuf, asmFile);
    }

  /* create the overlay segments */
  if (overlay)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; overlayable items in%s ram \n", mcs51_like ? " internal" : "");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&ovrBuf, asmFile);
    }

  /* create the stack segment MOF */
  if (mainf && IFFUNC_HASBODY (mainf->type))
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; Stack segment in internal ram \n");
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "\t.area\tSSEG\n" "__start__stack:\n\t.ds\t1\n\n");
    }

  /* create the idata segment */
  if (idata)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; indirectly addressable internal ram data\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&idata->oBuf, asmFile);
    }

  /* create the absolute idata/data segment */
  if (d_abs || i_abs)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; absolute%s ram data\n", mcs51_like ? " internal" : "");
      fprintf (asmFile, "%s", iComments2);
      if (d_abs)
        dbuf_write_and_destroy (&d_abs->oBuf, asmFile);
      if (i_abs)
        dbuf_write_and_destroy (&i_abs->oBuf, asmFile);
    }

  /* copy the bit segment */
  if (bit)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; bit data\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&bit->oBuf, asmFile);
    }

  /* copy paged external ram data */
  if (pdata)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; paged external ram data\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&pdata->oBuf, asmFile);
    }

  /* if external stack then reserve space for it */
  if (mainf && IFFUNC_HASBODY (mainf->type) && options.useXstack)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; external stack \n");
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "\t.area XSTK (PAG,XDATA)\n" "__start__xstack:\n\t.ds\t1\n\n");
    }

  /* copy external ram data */
  if (xdata && mcs51_like)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; external ram data\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&xdata->oBuf, asmFile);
    }

  /* create the absolute xdata segment */
  if (x_abs)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; absolute external ram data\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&x_abs->oBuf, asmFile);
    }

  /* copy external initialized ram data */
  if (xidata)
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; external initialized ram data\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&xidata->oBuf, asmFile);
    }

  /* If the port wants to generate any extra areas, let it do so. */
  if (port->extraAreas.genExtraAreaDeclaration)
    {
      port->extraAreas.genExtraAreaDeclaration (asmFile, mainf && IFFUNC_HASBODY (mainf->type));
    }

  /* copy the interrupt vector table */
  if (mainf && IFFUNC_HASBODY (mainf->type))
    {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; interrupt vector \n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&vBuf, asmFile);
    }

  /* copy global & static initialisations */
  fprintf (asmFile, "%s", iComments2);
  fprintf (asmFile, "; global & static initialisations\n");
  fprintf (asmFile, "%s", iComments2);

  /* Everywhere we generate a reference to the static_name area,
   * (which is currently only here), we immediately follow it with a
   * definition of the post_static_name area. This guarantees that
   * the post_static_name area will immediately follow the static_name
   * area.
   */
  tfprintf (asmFile, "\t!area\n", port->mem.home_name);
  tfprintf (asmFile, "\t!area\n", port->mem.static_name);       /* MOF */
  tfprintf (asmFile, "\t!area\n", port->mem.post_static_name);
  tfprintf (asmFile, "\t!area\n", port->mem.static_name);

  if (mainf && IFFUNC_HASBODY (mainf->type))
    {
      if (port->genInitStartup)
        {
          port->genInitStartup (asmFile);
        }
      else
        {
          assert (0);
        }
    }
  dbuf_write_and_destroy (&statsg->oBuf, asmFile);

  /* STM8 / PDK14 note: there are no such instructions supported.
     Also, we don't need this logic as well. */
  if (port->general.glue_up_main && mainf && IFFUNC_HASBODY (mainf->type))
    {
      /* This code is generated in the post-static area.
       * This area is guaranteed to follow the static area
       * by the ugly shucking and jiving about 20 lines ago.
       */
      tfprintf (asmFile, "\t!area\n", port->mem.post_static_name);
      if(TARGET_IS_STM8)
        fprintf (asmFile, "\tjp\t__sdcc_program_startup\n");
      else if(TARGET_PDK_LIKE)
        fprintf (asmFile, "\tgoto\t__sdcc_program_startup\n");
      else
        fprintf (asmFile, "\t%cjmp\t__sdcc_program_startup\n", options.acall_ajmp ? 'a' : 'l');
    }

  fprintf (asmFile, "%s" "; Home\n" "%s", iComments2, iComments2);
  tfprintf (asmFile, "\t!areahome\n", HOME_NAME);
  dbuf_write_and_destroy (&home->oBuf, asmFile);

  if (mainf && IFFUNC_HASBODY (mainf->type))
    {
      /* STM8 note: there is no need to call main().
         Instead of that, it's address is specified in the 
         interrupts table and always equals to 0x8080.
       */

      /* entry point @ start of HOME */
      fprintf (asmFile, "__sdcc_program_startup:\n");

      /* put in jump or call to main */
      if(TARGET_IS_STM8)
        fprintf (asmFile, options.model == MODEL_LARGE ? "\tjpf\t_main\n" : "\tjp\t_main\n");
      else if(TARGET_PDK_LIKE)
        fprintf (asmFile, "\tgoto\t_main\n");
      else
        fprintf (asmFile, "\t%cjmp\t_main\n", options.acall_ajmp ? 'a' : 'l');        /* needed? */
      fprintf (asmFile, ";\treturn from main will return to caller\n");
    }
  /* copy over code */
  fprintf (asmFile, "%s", iComments2);
  fprintf (asmFile, "; code\n");
  fprintf (asmFile, "%s", iComments2);
  tfprintf (asmFile, "\t!areacode\n", options.code_seg);
  dbuf_write_and_destroy (&code->oBuf, asmFile);

  if (port->genAssemblerEnd)
    {
      port->genAssemblerEnd (asmFile);
    }
  fclose (asmFile);
}

/* will return 1 if the string is a part
   of a target specific keyword */
int
isTargetKeyword (const char *s)
{
  int i;

  if (port->keywords == NULL)
    return 0;

  if (s[0] == '_' && s[1] == '_')
    {
      /* Keywords in the port's array have either 0 or 1 underscore, */
      /* so skip over the appropriate number of chars when comparing */
      for (i = 0 ; port->keywords[i] ; i++ )
        {
          if (port->keywords[i][0] == '_' &&
              strcmp(port->keywords[i],s+1) == 0)
            return 1;
          else if (strcmp(port->keywords[i],s+2) == 0)
            return 1;
        }
    }
  else
    {
      for (i = 0 ; port->keywords[i] ; i++ )
        {
          if (strcmp(port->keywords[i],s) == 0)
            return 1;
        }
    }

  return 0;
}

