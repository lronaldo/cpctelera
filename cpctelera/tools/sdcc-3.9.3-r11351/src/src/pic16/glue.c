/*-------------------------------------------------------------------------
  glue.c - glues everything we have done together into one file.

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

#include "../common.h"
#include <time.h>
#include "ralloc.h"
#include "pcode.h"
#include "newalloc.h"
#include "gen.h"
#include "device.h"
#include "main.h"
#include "dbuf_string.h"
#include <string.h>


extern symbol *interrupts[256];
void pic16_printIval (symbol * sym, sym_link * type, initList * ilist, char ptype, void *p);
extern int noAlloc;
extern set *publics;
extern set *externs;
extern unsigned maxInterrupts;
extern symbol *mainf;
extern char *VersionString;
extern struct dbuf_s *codeOutBuf;
extern char *iComments1;
extern char *iComments2;
extern int has_xinst_config;

extern int initsfpnt;
extern unsigned long pFile_isize;

extern unsigned long pic16_countInstructions();
set *pic16_localFunctions = NULL;
set *rel_idataSymSet = NULL;
set *fix_idataSymSet = NULL;

extern void pic16_AnalyzeBanking (void);
extern void pic16_OptimizeJumps (void);
extern void pic16_OptimizeBanksel (void);
extern void pic16_InlinepCode (void);
extern void pic16_writeUsedRegs (FILE *);

extern void initialComments (FILE * afile);
extern void printPublics (FILE * afile);

void  pic16_pCodeInitRegisters (void);
pCodeOp *pic16_popCopyReg (pCodeOpReg *pc);
extern void pic16_pCodeConstString (char *name, const char *value, unsigned length);


/*-----------------------------------------------------------------*/
/* aopLiteral - string from a literal value                        */
/*-----------------------------------------------------------------*/
unsigned int pic16aopLiteral (value *val, int offset)
{
  union {
    float f;
    unsigned char c[4];
  } fl;

  /* if it is a float then it gets tricky */
  /* otherwise it is fairly simple */
  if (!(IS_FLOAT(val->type) || IS_FIXED(val->type))) {
    unsigned long v = ulFromVal (val);

    return ( (v >> (offset * 8)) & 0xff);
  }

  if(IS_FIXED16X16(val->type)) {
    unsigned long v = (unsigned long)fixed16x16FromDouble( floatFromVal( val ) );

    return ( (v >> (offset * 8)) & 0xff);
  }

  /* it is type float */
  fl.f = (float) floatFromVal(val);
#ifdef WORDS_BIGENDIAN
  return fl.c[3-offset];
#else
  return fl.c[offset];
#endif

}

iCode *tic;
symbol *nsym;
char tbuffer[512], *tbuf=tbuffer;


/*-----------------------------------------------------------------*/
/* emitRegularMap - emit code for maps with no special cases       */
/*-----------------------------------------------------------------*/
static void
pic16emitRegularMap (memmap * map, bool addPublics, bool arFlag)
{
  symbol *sym;
//  int i, size, bitvars = 0;;

//      fprintf(stderr, "%s:%d map name= %s\n", __FUNCTION__, __LINE__, map->sname);

        if(addPublics)
                dbuf_printf (&map->oBuf, ";\t.area\t%s\n", map->sname);
                /* print the area name */

        for (sym = setFirstItem (map->syms); sym; sym = setNextItem (map->syms)) {

#if 0
                fprintf(stderr, "%s\t%s: sym: %s\tused: %d\textern: %d\tstatic: %d\taggregate: %d\tregister: 0x%x\tfunction: %d\n",
                        __FUNCTION__,
                        map->sname, sym->name, sym->used, IS_EXTERN(sym->etype), IS_STATIC(sym->etype),
                        IS_AGGREGATE(sym->type), (SPEC_SCLS(sym->etype) == S_REGISTER), IS_FUNC(sym->type));
                printTypeChain( sym->type, stderr );
                fprintf(stderr, "\n");
#endif

                /* if extern then add to externs */
                if (IS_EXTERN (sym->etype)) {
                        /* reduce overhead while linking by not declaring
                         * extern unused external functions (usually declared
                         * in header files) */
                        if(IS_FUNC(sym->type) && !sym->used)continue;

                        /* make sure symbol is not in publics section */
                        if(!checkSym(publics, sym))
                                checkAddSym(&externs, sym);
                        continue;
                }

                /* if allocation required check is needed
                 * then check if the symbol really requires
                 * allocation only for local variables */
                 if (arFlag && !IS_AGGREGATE (sym->type) &&
                        !(sym->_isparm && !IS_REGPARM (sym->etype)) &&
                        !sym->allocreq && sym->level) {

//                      fprintf(stderr, "%s:%d special case, continuing...\n", __FILE__, __LINE__);

                        continue;
                }

                /* if global variable & not static or extern
                 * and addPublics allowed then add it to the public set */
                if ((sym->used) && (sym->level == 0 ||
                        (sym->_isparm && !IS_REGPARM (sym->etype))) &&
                        addPublics &&
                        !IS_STATIC (sym->etype) && !IS_FUNC(sym->type)) {

                        checkAddSym(&publics, sym);
                } else if (IS_AGGREGATE(sym->type) && sym->level == 0 && ! IS_STATIC(sym->etype)) {
                        /* If a global variable is not static and will be included in the published list. */

                        checkAddSym(&publics, sym);
                } else
                        /* new version */
                        if(IS_STATIC(sym->etype)
                                && !sym->ival) /* && !sym->level*/ {
                          reg_info *reg;
                          sectSym *ssym;
                          int found=0;

//                            debugf("adding symbol %s\n", sym->name);
#define SET_IMPLICIT    1

#if SET_IMPLICIT
                                if(IS_STRUCT(sym->type))
                                        sym->implicit = 1;
#endif

                                reg = pic16_allocDirReg( operandFromSymbol( sym ));

                                if(reg) {
                                  for(ssym=setFirstItem(sectSyms); ssym; ssym=setNextItem(sectSyms)) {
                                    if(!strcmp(ssym->name, reg->name))found=1;
                                  }

                                  if(!found)
                                    checkAddReg(&pic16_rel_udata, reg);
#if 0
                                  else
                                    debugf("Did find %s in pic16_rel_udata already. Check!\n", reg->name);
//                                  checkAddSym(&publics, sym);
#endif

                                }
                        }

                /* if extern then do nothing or is a function
                 * then do nothing */
                if (IS_FUNC (sym->type) && !IS_STATIC(sym->etype)) {
                        if(SPEC_OCLS(sym->etype) == code) {
//                              fprintf(stderr, "%s:%d: symbol added: %s\n", __FILE__, __LINE__, sym->rname);
                                checkAddSym(&publics, sym);
                        }
                        continue;
                }

                /* if is has an absolute address then generate
                an equate for this no need to allocate space */
                if (SPEC_ABSA (sym->etype)) {
//                              fprintf (stderr,"; %s == 0x%04x\t\treqv= %p nRegs= %d\n",
//                                      sym->name, SPEC_ADDR (sym->etype), sym->reqv, sym->regType);

                        dbuf_printf (&map->oBuf, "%s\tEQU\t0x%04x\n",
                                sym->rname,
                                SPEC_ADDR (sym->etype));

                        /* emit only if it is global */
                        if(sym->level == 0) {
                          reg_info *reg;

                                reg = pic16_dirregWithName( sym->name );
                                if(!reg) {
                                        /* here */
//                                      fprintf(stderr, "%s:%d: implicit add of symbol = %s\n",
//                                                      __FUNCTION__, __LINE__, sym->name);

                                        /* if IS_STRUCT is omitted the following
                                         * fixes structures but break char/int etc */
#if SET_IMPLICIT
                                        if(IS_STRUCT(sym->type))
                                                sym->implicit = 1;              // mark as implicit
#endif
                                        if(!sym->ival) {
                                                reg = pic16_allocDirReg( operandFromSymbol(sym) );
                                                if(reg) {
                                                        if(checkAddReg(&pic16_fix_udata, reg)) {
                                                                /* and add to globals list if not exist */
                                                                addSet(&publics, sym);
                                                        }
                                                }
                                        } else
                                                addSet(&publics, sym);
                                }
                        }
                } else {
                        if(!sym->used && (sym->level == 0)) {
                          reg_info *reg;

                                /* symbol not used, just declared probably, but its in
                                 * level 0, so we must declare it fine as global */

//                              fprintf(stderr, "EXTRA symbol declaration sym= %s\n", sym->name);

#if SET_IMPLICIT
                                if(IS_STRUCT(sym->type))
                                        sym->implicit = 1;              // mark as implicit
#endif
                                if(!sym->ival) {
                                        if(IS_AGGREGATE(sym->type)) {
                                                reg=pic16_allocRegByName(sym->rname, getSize( sym->type ), NULL);
                                        } else {
                                                reg = pic16_allocDirReg( operandFromSymbol( sym ) );
                                        }

                                        {
                                          sectSym *ssym;
                                          int found=0;

#if 0
                                                fprintf(stderr, "%s:%d sym->rname: %s reg: %p reg->name: %s\n", __FILE__, __LINE__,
                                                        sym->rname, reg, (reg?reg->name:"<<NULL>>"));
#endif

                                                if(reg) {
                                                  for(ssym=setFirstItem(sectSyms); ssym; ssym=setNextItem(sectSyms)) {
                                                    if(!strcmp(ssym->name, reg->name))found=1;
                                                  }

                                                  if(!found)
                                                    if(checkAddReg(&pic16_rel_udata, reg)) {
                                                      addSetHead(&publics, sym);
                                                    }
                                                }
                                        }



                                } else

                                        addSetHead(&publics, sym);
                        }

#if 0
                        /* allocate space */
                        /* If this is a bit variable, then allocate storage after 8 bits have been declared */
                        /* unlike the 8051, the pic does not have a separate bit area. So we emulate bit ram */
                        /* by grouping the bits together into groups of 8 and storing them in the normal ram. */
                        if (IS_BITVAR (sym->etype)) {
                                bitvars++;
                        } else {
                                dbuf_printf (map->oBuf, "\t%s\n", sym->rname);
                                if ((size = (unsigned int) getSize (sym->type) & 0xffff) > 1) {
                                        for (i = 1; i < size; i++)
                                                dbuf_printf (map->oBuf, "\t%s_%d\n", sym->rname, i);
                                }
                        }
                        dbuf_printf (map->oBuf, "\t.ds\t0x%04x\n", (unsigned int)getSize (sym->type) & 0xffff);
#endif
                }

                /* FIXME -- VR Fix the following, so that syms to be placed
                 * in the idata section and let linker decide about their fate */

                /* if it has an initial value then do it only if
                        it is a global variable */

                if (sym->ival
                  && ((sym->level == 0)
                      || IS_STATIC(sym->etype)) ) {
                  ast *ival = NULL;

#if 0
                        if(SPEC_OCLS(sym->etype)==data) {
                                fprintf(stderr, "%s: sym %s placed in data segment\n", map->sname, sym->name);
                        }

                        if(SPEC_OCLS(sym->etype)==code) {
                                fprintf(stderr, "%s: sym %s placed in code segment\n", map->sname, sym->name);
                        }
#endif

#if 0
                        fprintf(stderr, "'%s': sym '%s' has initial value SPEC_ABSA: %d, IS_AGGREGATE: %d\n",
                                map->sname, sym->name, SPEC_ABSA(sym->etype), IS_AGGREGATE(sym->type));
#endif

                        if (IS_AGGREGATE (sym->type)) {
                                if(SPEC_ABSA(sym->etype))
                                        addSet(&fix_idataSymSet, copySymbol(sym));
                                else
                                        addSet(&rel_idataSymSet, copySymbol(sym));
//                              ival = initAggregates (sym, sym->ival, NULL);
                        } else {
                                if(SPEC_ABSA(sym->etype))
                                        addSet(&fix_idataSymSet, copySymbol(sym));
                                else
                                        addSet(&rel_idataSymSet, copySymbol(sym));

//                                      ival = newNode ('=', newAst_VALUE(symbolVal (sym)),
//                                              decorateType (resolveSymbols (list2expr (sym->ival)), RESULT_TYPE_NONE));
                        }

                        if(ival) {
                                setAstFileLine (ival, sym->fileDef, sym->lineDef);
                                codeOutBuf = &statsg->oBuf;
                                GcurMemmap = statsg;
                                eBBlockFromiCode (iCodeFromAst (ival));
                                sym->ival = NULL;
                        }
                }
        }
}


/*-----------------------------------------------------------------*/
/* pic16_initPointer - pointer initialization code massaging       */
/*-----------------------------------------------------------------*/
static value *
pic16_initPointer (initList * ilist, sym_link *toType)
{
  value *val;
  ast *expr;

  if (!ilist) {
      return valCastLiteral(toType, 0.0, 0);
  }

  expr = decorateType(resolveSymbols( list2expr (ilist) ), FALSE);
//  expr = list2expr( ilist );

  if (!expr)
    goto wrong;

  /* try it the old way first */
  if (expr->etype && (val = constExprValue (expr, FALSE)))
    return val;

  /* ( ptr + constant ) */
  if (IS_AST_OP (expr) &&
      (expr->opval.op == '+' || expr->opval.op == '-') &&
      IS_AST_SYM_VALUE (expr->left) &&
      (IS_ARRAY(expr->left->ftype) || IS_PTR(expr->left->ftype)) &&
      compareType(toType, expr->left->ftype) &&
      IS_AST_LIT_VALUE (expr->right)) {
    return valForCastAggr (expr->left, expr->left->ftype,
                           expr->right,
                           expr->opval.op);
  }

  /* (char *)&a */
  if (IS_AST_OP(expr) && expr->opval.op==CAST &&
      IS_AST_OP(expr->right) && expr->right->opval.op=='&') {
    if (compareType(toType, expr->left->ftype)!=1) {
      werror (W_INIT_WRONG);
      printFromToType(expr->left->ftype, toType);
    }
    // skip the cast ???
    expr=expr->right;
  }

  /* no then we have to do these cludgy checks */
  /* pointers can be initialized with address of
     a variable or address of an array element */
  if (IS_AST_OP (expr) && expr->opval.op == '&') {

    /* AST nodes for symbols of type struct or union may be missing type */
    /* due to the abuse of the sym->implicit flag, so get the type */
    /* directly from the symbol if it's missing in the node. */
    sym_link *etype = expr->left->etype;
    sym_link *ftype = expr->left->ftype;
    if (!etype && IS_AST_SYM_VALUE(expr->left))
      {
        etype = expr->left->opval.val->sym->etype;
        ftype = expr->left->opval.val->sym->type;
      }

    /* address of symbol */
    if (IS_AST_SYM_VALUE (expr->left) && etype) {
      val = AST_VALUE (expr->left);
      val->type = newLink (DECLARATOR);
      if(SPEC_SCLS (etype) == S_CODE) {
        DCL_TYPE (val->type) = CPOINTER;
        CodePtrPointsToConst (val->type);
      }
      else if (SPEC_SCLS (etype) == S_XDATA)
        DCL_TYPE (val->type) = FPOINTER;
      else if (SPEC_SCLS (etype) == S_XSTACK)
        DCL_TYPE (val->type) = PPOINTER;
      else if (SPEC_SCLS (etype) == S_IDATA)
        DCL_TYPE (val->type) = IPOINTER;
      else if (SPEC_SCLS (etype) == S_EEPROM)
        DCL_TYPE (val->type) = EEPPOINTER;
      else
        DCL_TYPE (val->type) = POINTER;

      val->type->next = ftype;
      val->etype = getSpec (val->type);
      return val;
    }

    /* if address of indexed array */
    if (IS_AST_OP (expr->left) && expr->left->opval.op == '[')
      return valForArray (expr->left);

    /* if address of structure element then
       case 1. a.b ; */
    if (IS_AST_OP (expr->left) &&
        expr->left->opval.op == '.') {
      return valForStructElem (expr->left->left,
                               expr->left->right);
    }

    /* case 2. (&a)->b ;
       (&some_struct)->element */
    if (IS_AST_OP (expr->left) &&
        expr->left->opval.op == PTR_OP &&
        IS_ADDRESS_OF_OP (expr->left->left)) {
      return valForStructElem (expr->left->left->left,
                               expr->left->right);
    }
  }
  /* case 3. (((char *) &a) +/- constant) */
  if (IS_AST_OP (expr) &&
      (expr->opval.op == '+' || expr->opval.op == '-') &&
      IS_AST_OP (expr->left) && expr->left->opval.op == CAST &&
      IS_AST_OP (expr->left->right) &&
      expr->left->right->opval.op == '&' &&
      IS_AST_LIT_VALUE (expr->right)) {

    return valForCastAggr (expr->left->right->left,
                           expr->left->left->opval.lnk,
                           expr->right, expr->opval.op);

  }
  /* case 4. (char *)(array type) */
  if (IS_CAST_OP(expr) && IS_AST_SYM_VALUE (expr->right) &&
      IS_ARRAY(expr->right->ftype)) {

    val = copyValue (AST_VALUE (expr->right));
    val->type = newLink (DECLARATOR);
    if (SPEC_SCLS (expr->right->etype) == S_CODE) {
      DCL_TYPE (val->type) = CPOINTER;
      CodePtrPointsToConst (val->type);
    }
    else if (SPEC_SCLS (expr->right->etype) == S_XDATA)
      DCL_TYPE (val->type) = FPOINTER;
    else if (SPEC_SCLS (expr->right->etype) == S_XSTACK)
      DCL_TYPE (val->type) = PPOINTER;
    else if (SPEC_SCLS (expr->right->etype) == S_IDATA)
      DCL_TYPE (val->type) = IPOINTER;
    else if (SPEC_SCLS (expr->right->etype) == S_EEPROM)
      DCL_TYPE (val->type) = EEPPOINTER;
    else
      DCL_TYPE (val->type) = POINTER;
    val->type->next = expr->right->ftype->next;
    val->etype = getSpec (val->type);
    return val;
  }

 wrong:
  if (expr)
    werrorfl (expr->filename, expr->lineno, E_INCOMPAT_PTYPES);
  else
    werror (E_INCOMPAT_PTYPES);
  return NULL;

}


/*-----------------------------------------------------------------*/
/* printPointerType - generates ival for pointer type              */
/*-----------------------------------------------------------------*/
static void
_pic16_printPointerType (const char *name, char ptype, void *p)
{
  char buf[256];

  SNPRINTF(buf, sizeof(buf), "LOW(%s)", name);
  pic16_emitDS (buf, ptype, p);
  SNPRINTF(buf, sizeof(buf), "HIGH(%s)", name);
  pic16_emitDS (buf, ptype, p);
}

/*-----------------------------------------------------------------*/
/* printPointerType - generates ival for pointer type              */
/*-----------------------------------------------------------------*/
static void
pic16_printPointerType (const char *name, char ptype, void *p)
{
  _pic16_printPointerType (name, ptype, p);
  //pic16_flushDB(ptype, p); /* breaks char* const arr[] = {&c, &c, &c}; */
}

/*-----------------------------------------------------------------*/
/* printGPointerType - generates ival for generic pointer type     */
/*-----------------------------------------------------------------*/
static void
pic16_printGPointerType (const char *iname, const unsigned int itype,
  char ptype, void *p)
{
  char buf[256];

  _pic16_printPointerType (iname, ptype, p);

  switch (itype)
    {
    case CPOINTER: /* fall through */
    case FUNCTION: /* fall through */
    case GPOINTER:
      /* GPTRs pointing to __data space should be reported as POINTERs */
      SNPRINTF(buf, sizeof(buf), "UPPER(%s)", iname);
      pic16_emitDS (buf, ptype, p);
      break;

    case POINTER:  /* fall through */
    case FPOINTER: /* fall through */
    case IPOINTER: /* fall through */
    case PPOINTER: /* __data space */
      SNPRINTF(buf, sizeof(buf), "0x%02x", GPTR_TAG_DATA);
      pic16_emitDS (buf, ptype, p);
      break;

    default:
      debugf ("itype = %d\n", itype );
      assert (0);
    }

    if (itype == GPOINTER) {
      fprintf(stderr, "%s: initialized generic pointer with unknown storage class assumes object in code space\n", __func__);
    }

  //pic16_flushDB(ptype, p); /* might break char* const arr[] = {...}; */
}


/* set to 0 to disable debug messages */
#define DEBUG_PRINTIVAL 0

/*-----------------------------------------------------------------*/
/* pic16_printIvalType - generates ival for int/char               */
/*-----------------------------------------------------------------*/
static void
pic16_printIvalType (symbol *sym, sym_link * type, initList * ilist, char ptype, void *p)
{
  value *val;
  int i;

//  fprintf(stderr, "%s for symbol %s\n",__FUNCTION__, sym->rname);

#if DEBUG_PRINTIVAL
  fprintf(stderr, "%s\n",__FUNCTION__);
#endif


  /* if initList is deep */
  if (ilist && ilist->type == INIT_DEEP)
    ilist = ilist->init.deep;

  if (!IS_AGGREGATE(sym->type) && getNelements(type, ilist)>1) {
    werror (W_EXCESS_INITIALIZERS, "scalar", sym->name, sym->lineDef);
  }

  if (!(val = list2val (ilist, TRUE))) {
    // assuming a warning has been thrown
    val = constCharVal (0);
  }

  if (val->type != type) {
    val = valCastLiteral(type, floatFromVal (val), (TYPE_TARGET_ULONGLONG) ullFromVal (val));
  }

  for (i = 0; i < (int)getSize (type); i++) {
    pic16_emitDB(pic16aopLiteral(val, i), ptype, p);
  } // for
}

/*--------------------------------------------------------------------*/
/* pic16_printIvalChar - generates initital value for character array */
/*--------------------------------------------------------------------*/
static int
pic16_printIvalChar (symbol *sym, sym_link * type, initList * ilist, const char *s, char ptype, void *p)
{
  value *val;
  int len;
  size_t remain, ilen;

  if(!p)
    return 0;

#if DEBUG_PRINTIVAL
  fprintf(stderr, "%s\n",__FUNCTION__);
#endif

  if(!s) {
    val = list2val (ilist, TRUE);

    /* if the value is a character string  */
    if(IS_ARRAY (val->type) && IS_CHAR (val->etype)) {
      /* length of initializer string (might contain \0, so do not use strlen) */
      ilen = DCL_ELEM(val->type);

#if 0
      /* This causes structflexarray.c to fail. */
      if(!DCL_ELEM (type))
        DCL_ELEM (type) = ilen;
#endif

      /* len is 0 if declartion equals initializer,
       * >0 if declaration greater than initializer
       * <0 if declaration less than initializer
       * Strategy: if >0 emit 0x00 for the rest of the length,
       * if <0 then emit only the length of declaration elements
       * and warn user
       */
      len = DCL_ELEM (type) - ilen;

//      fprintf(stderr, "%s:%d ilen = %i len = %i DCL_ELEM(type) = %i SPEC_CVAL-len = %i\n", __FILE__, __LINE__,
//        ilen, len, DCL_ELEM(type), strlen(SPEC_CVAL(val->etype).v_char));

      if(len >= 0) {
        /* emit initializer */
        for(remain=0; remain<ilen; remain++) {
          pic16_emitDB(SPEC_CVAL(val->etype).v_char[ remain ], ptype, p);
        } // for

        /* fill array with 0x00 */
        while(len--) {
          pic16_emitDB(0x00, ptype, p);
        } // while
      } else if (!DCL_ELEM (type)) {
        // flexible arrays: char str[] = "something"; */
        for(remain=0; remain<ilen; remain++) {
          pic16_emitDB(SPEC_CVAL(val->etype).v_char[ remain ], ptype, p);
        } // for
      } else {
        werror (W_EXCESS_INITIALIZERS, "array of chars", sym->name, sym->lineDef);
        for(remain=0; remain<DCL_ELEM (type); remain++) {
          pic16_emitDB(SPEC_CVAL(val->etype).v_char[ remain ], ptype, p);
        } // for
      } // if


//      if((remain = (DCL_ELEM (type) - strlen (SPEC_CVAL (val->etype).v_char) - 1)) > 0) {
//      }
      return 1;
    } else return 0;
  } else {
    for(remain=0; remain<strlen(s); remain++) {
        pic16_emitDB(s[remain], ptype, p);
    }
  }
  return 1;
}

/*-----------------------------------------------------------------*/
/* pic16_printIvalArray - generates code for array initialization        */
/*-----------------------------------------------------------------*/
static void
pic16_printIvalArray (symbol * sym, sym_link * type, initList * ilist,
                char ptype, void *p)
{
  initList *iloop;
  int lcnt = 0, size = 0;

  if(!p)
    return;


#if DEBUG_PRINTIVAL
  fprintf(stderr, "%s\n",__FUNCTION__);
#endif
  /* take care of the special   case  */
  /* array of characters can be init  */
  /* by a string                      */
  if (IS_CHAR (type->next) &&
      ilist && ilist->type == INIT_NODE) {
    if (!IS_LITERAL(list2val(ilist, TRUE)->etype)) {
      werror (W_INIT_WRONG);
      return;
    }

    if(pic16_printIvalChar (sym, type,
                       (ilist->type == INIT_DEEP ? ilist->init.deep : ilist),
                       SPEC_CVAL (sym->etype).v_char, ptype, p))
      return;
  }
  /* not the special case             */
  if (ilist && ilist->type != INIT_DEEP)
    {
      werror (E_INIT_STRUCT, sym->name);
      return;
    }

  iloop = (ilist ? ilist->init.deep : NULL);
  lcnt = DCL_ELEM (type);

  for (;;)
    {
      size++;
      pic16_printIval (sym, type->next, iloop, ptype, p);
      iloop = (iloop ? iloop->next : NULL);


      /* if not array limits given & we */
      /* are out of initialisers then   */
      if (!DCL_ELEM (type) && !iloop)
        break;

      /* no of elements given and we    */
      /* have generated for all of them */
      if (!--lcnt) {
        /* if initializers left */
        if (iloop) {
          werror (W_EXCESS_INITIALIZERS, "array", sym->name, sym->lineDef);
        }
        break;
      }
    }

#if 0
  /* This causes bug #1843745. */
  /* if we have not been given a size  */
  if (!DCL_ELEM (type))
    DCL_ELEM (type) = size;
#endif

  return;
}

/*-----------------------------------------------------------------*/
/* pic16_printIvalBitFields - generate initializer for bitfields   */
/*-----------------------------------------------------------------*/
static void
pic16_printIvalBitFields (symbol **sym, initList **ilist, char ptype, void *p)
{
  symbol *lsym = *sym;
  initList *lilist = *ilist;
  unsigned long ival = 0;
  unsigned size = 0;
  int bit_start = 0;
  unsigned i;


#if DEBUG_PRINTIVAL
  fprintf(stderr, "%s\n",__FUNCTION__);
#endif


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

          if (size)
            {
              if (bit_length > 8)
                size += (bit_length + 7) / 8;
            }
          else
            size = (bit_length + 7) / 8;

          ival |= (ulFromVal (val) & ((1ul << bit_length) - 1ul)) << bit_start;
          lilist = (lilist ? lilist->next : NULL);
        }
      bit_start += bit_length;
      lsym = lsym->next;
      if (lsym && IS_BITFIELD (lsym->type) && (0 == SPEC_BSTR (lsym->etype))) 
        {
          /* a new integer */
          break;
        }
    }

  for (i = 0; i < size; i++)
    pic16_emitDB (BYTE_IN_LONG (ival, i), ptype, p);

  *sym = lsym;
  *ilist = lilist;
}


/*-----------------------------------------------------------------*/
/* printIvalStruct - generates initial value for structures        */
/*-----------------------------------------------------------------*/
static void
pic16_printIvalStruct (symbol * sym, sym_link * type,
                 initList * ilist, char ptype, void *p)
{
  symbol *sflds;
  initList *iloop = NULL;


#if DEBUG_PRINTIVAL
  fprintf(stderr, "%s\n",__FUNCTION__);
#endif

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

  while (sflds)
    {
//      fprintf(stderr, "%s:%d sflds: %p\tiloop = %p\n", __FILE__, __LINE__, sflds, iloop);
      if (IS_BITFIELD (sflds->type))
        {
          pic16_printIvalBitFields (&sflds, &iloop, ptype, p);
        }
      else
        {
          pic16_printIval (sym, sflds->type, iloop, ptype, p);
          sflds = sflds->next;
          iloop = iloop ? iloop->next : NULL;
        }
    }
  if (iloop)
    werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "struct", sym->name);
}

/*-----------------------------------------------------------------*/
/* printIvalUnion - generates initial value for unions             */
/*-----------------------------------------------------------------*/
static void
pic16_printIvalUnion (symbol * sym, sym_link * type,
                 initList * ilist, char ptype, void *p)
{
  //symbol *sflds;
  initList *iloop = NULL;
  unsigned int size;
  symbol *sflds = NULL;

#if DEBUG_PRINTIVAL
  fprintf(stderr, "%s\n",__FUNCTION__);
#endif

  assert (type);

  sflds = SPEC_STRUCT (type)->fields;

  if (ilist) {
    if (ilist->type != INIT_DEEP) {
      werrorfl (sym->fileDef, sym->lineDef, E_INIT_STRUCT, sym->name);
      return;
    }

    iloop = ilist->init.deep;
  }

  size = SPEC_STRUCT(type)->size;
  sflds = SPEC_STRUCT(type)->fields;

  /* skip past holes, print value */
  while (iloop && iloop->type == INIT_HOLE)
    {
      iloop = iloop->next;
      sflds = sflds->next;
    }
  pic16_printIval (sym, sflds->type, iloop, ptype, p);

  /* if the first field is not the longest, fill with 0s */
  while (size > getSize (sflds->type)) {
      pic16_emitDB(0, ptype, p);
      size--;
  } // while
}

static int
pic16_isUnion( symbol *sym, sym_link *type )
{
  if (type && SPEC_STRUCT(type)->type == UNION) return 1;
  return 0;
}

/*--------------------------------------------------------------------------*/
/* pic16_printIvalCharPtr - generates initial values for character pointers */
/*--------------------------------------------------------------------------*/
static int
pic16_printIvalCharPtr (symbol * sym, sym_link * type, value * val, char ptype, void *p)
{
  int size = 0;
  int i;

  /* PENDING: this is _very_ mcs51 specific, including a magic
     number...
     It's also endin specific.

     VR - Attempting to port this function to pic16 port - 8-Jun-2004
   */


#if DEBUG_PRINTIVAL
  fprintf(stderr, "%s\n",__FUNCTION__);
#endif

  size = getSize (type);

  if (val->name && strlen (val->name))
    {
      if (size == 1)            /* This appears to be Z80 specific?? */
        {
          pic16_emitDS(val->name, ptype, p);
        }
      else if (size == 2)
        {
          pic16_printPointerType (val->name, ptype, p);
        }
      else if (size == 3)
        {
          int type;
          type = PTR_TYPE (SPEC_OCLS (val->etype));
          if (val->sym && val->sym->isstrlit) {
            // this is a literal string
            type = CPOINTER;
          }
          pic16_printGPointerType(val->name, type, ptype, p);
        }
      else
        {
          fprintf (stderr, "*** internal error: unknown size in "
                   "printIvalCharPtr.\n");
          assert(0);
        }
    }
  else
    {
      // these are literals assigned to pointers
      for (i = 0; i < size; i++)
        {
          pic16_emitDB(pic16aopLiteral(val, i), ptype, p);
        } // for
    }

  if (val->sym && val->sym->isstrlit) { // && !isinSet(statsg->syms, val->sym)) {
        if(ptype == 'p' && !isinSet(statsg->syms, val->sym))addSet (&statsg->syms, val->sym);
        else if(ptype == 'f' /*&& !isinSet(rel_idataSymSet, val->sym)*/)addSet(&rel_idataSymSet, val->sym);
  }

  return 1;
}

/*-----------------------------------------------------------------------*/
/* pic16_printIvalFuncPtr - generate initial value for function pointers */
/*-----------------------------------------------------------------------*/
static void
pic16_printIvalFuncPtr (sym_link * type, initList * ilist, char ptype, void *p)
{
  value *val;
  int dLvl = 0;


#if DEBUG_PRINTIVAL
  fprintf(stderr, "%s\n",__FUNCTION__);
#endif

  if (ilist)
    val = list2val (ilist, TRUE);
  else
    val = valCastLiteral(type, 0.0, 0);

  if (!val) {
    // an error has been thrown already
    val = constCharVal (0);
  }

  if (IS_LITERAL(val->etype)) {
    if (0 && compareType(type, val->etype) == 0) {
      werrorfl (ilist->filename, ilist->lineno, E_INCOMPAT_TYPES);
      printFromToType (val->type, type);
    }
    pic16_printIvalCharPtr (NULL, type, val, ptype, p);
    return;
  }

  /* check the types   */
  if ((dLvl = compareType (val->type, type->next)) <= 0)
    {
      pic16_emitDB(0x00, ptype, p);
      return;
    }

  /* now generate the name */
  if (!val->sym) {
      pic16_printGPointerType (val->name, CPOINTER /*DCL_TYPE(val->type)*/, ptype, p);
  } else {
      pic16_printGPointerType (val->sym->rname, CPOINTER /*DCL_TYPE(val->type)*/, ptype, p);

      if(IS_FUNC(val->sym->type) && !val->sym->used && !IS_STATIC(val->sym->etype)) {

        if(!checkSym(publics, val->sym))
          if(checkAddSym(&externs, val->sym) && (ptype == 'f')) {
            /* this has not been declared as extern
             * so declare it as a 'late extern' just after the symbol */
            fprintf((FILE *)p, ";\tdeclare symbol as extern\n");
            fprintf((FILE *)p, "\textern\t%s\n", val->sym->rname);
            fprintf((FILE *)p, ";\tcontinue variable declaration\n");
          }
      }
  }

  return;
}


/*-----------------------------------------------------------------*/
/* pic16_printIvalPtr - generates initial value for pointers       */
/*-----------------------------------------------------------------*/
static void
pic16_printIvalPtr (symbol * sym, sym_link * type, initList * ilist, char ptype, void *p)
{
  value *val;
  int size;
  int i;

#if 0
        fprintf(stderr, "%s:%d initialising pointer: %s size: %d\n", __FILE__, __LINE__,
                sym->rname, getSize(sym->type));
#endif

  /* if deep then   */
  if (ilist && (ilist->type == INIT_DEEP))
    ilist = ilist->init.deep;

  /* function pointer     */
  if (IS_FUNC (type->next))
    {
      pic16_printIvalFuncPtr (type, ilist, ptype, p);
      return;
    }

  if (!(val = pic16_initPointer (ilist, type)))
    return;

  /* if character pointer */
  if (IS_CHAR (type->next))
    if (pic16_printIvalCharPtr (sym, type, val, ptype, p))
      return;

  /* check the type      */
  if (compareType (type, val->type) == 0) {
    werrorfl (ilist->filename, ilist->lineno, W_INIT_WRONG);
    printFromToType (val->type, type);
  }

  size = getSize (type);

  /* if val is literal */
  if (IS_LITERAL (val->etype))
    {
      for (i = 0; i < size; i++)
        {
          pic16_emitDB(pic16aopLiteral(val, i), ptype, p);
        } // for
      return;
    }

  if (size == 1)                /* Z80 specific?? */
    {
      pic16_emitDS(val->name, ptype, p);
    }
  else if (size == 2)
    {
      pic16_printPointerType (val->name, ptype, p);
    }
  else if (size == 3)
    {
      int itype = 0;
      itype = PTR_TYPE (SPEC_OCLS (val->etype));
      pic16_printGPointerType (val->name, itype, ptype, p);
    }
  else
    {
      assert(0);
    }
}



/*-----------------------------------------------------------------*/
/* pic16_printIval - generates code for initial value                    */
/*-----------------------------------------------------------------*/
void pic16_printIval (symbol * sym, sym_link * type, initList * ilist, char ptype, void *p)
{
//  sym_link *itype;

  if (!p)
    return;

#if 0
        fprintf(stderr, "%s:%d generating init for %s\n", __FILE__, __LINE__, sym->name);
        fprintf(stderr, "%s: IS_STRUCT: %d  IS_ARRAY: %d  IS_PTR: %d  IS_SPEC: %d\n", sym->name,
                IS_STRUCT(type), IS_ARRAY(type), IS_PTR(type), IS_SPEC(type));
#endif

  /* Handle designated initializers */
  if (ilist)
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

  /* if structure then */
  if (IS_STRUCT (type))
    {
      if (pic16_isUnion(sym, type))
        {
          //fprintf(stderr,"%s union\n",__FUNCTION__);
          pic16_printIvalUnion (sym, type, ilist, ptype, p);
        } else {
          //fprintf(stderr,"%s struct\n",__FUNCTION__);
          pic16_printIvalStruct (sym, type, ilist, ptype, p);
        }
      return;
    }

  /* if this is an array */
  if (IS_ARRAY (type))
    {
//      fprintf(stderr,"%s array\n",__FUNCTION__);
      pic16_printIvalArray (sym, type, ilist, ptype, p);
      return;
    }

#if 0
  if (ilist)
    {
      // not an aggregate, ilist must be a node
      if (ilist->type!=INIT_NODE) {
          // or a 1-element list
        if (ilist->init.deep->next) {
          werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "scalar",
                  sym->name);
        } else {
          ilist=ilist->init.deep;
        }
      }

#if 0
      // and the type must match
      itype=ilist->init.node->ftype;

      if (compareType(type, itype)==0) {
        // special case for literal strings
        if (IS_ARRAY (itype) && IS_CHAR (getSpec(itype)) &&
            // which are really code pointers
            IS_PTR(type) && DCL_TYPE(type)==CPOINTER) {
          // no sweat
        } else {
//          werrorfl (ilist->filename, ilist->lineno, E_TYPE_MISMATCH, "assignment", " ");
//          printFromToType(itype, type);
        }
      }
#endif
    }
#endif

  /* if this is a pointer */
  if (IS_PTR (type))
    {
//      fprintf(stderr,"%s pointer\n",__FUNCTION__);
      pic16_printIvalPtr (sym, type, ilist, ptype, p);
      return;
    }


  /* if type is SPECIFIER */
  if (IS_SPEC (type))
    {
//      fprintf(stderr,"%s spec\n",__FUNCTION__);
      pic16_printIvalType (sym, type, ilist, ptype, p);
      return;
    }
}

static int
PIC16_IS_CONFIG_ADDRESS(int address)
{
  return ((address >= pic16->cwInfo.confAddrStart && address <= pic16->cwInfo.confAddrEnd));
}

static int
PIC16_IS_IDLOC_ADDRESS(int address)
{
   return ((address >= pic16->idInfo.idAddrStart && address <= pic16->idInfo.idAddrEnd));
}

/*-----------------------------------------------------------------*/
/* emitStaticSeg - emitcode for the static segment                 */
/*-----------------------------------------------------------------*/
static void
pic16emitStaticSeg (memmap * map)
{
  symbol *sym;
  static int didcode = 0;

  //fprintf(stderr, "%s\n",__FUNCTION__);

  pic16_initDB ();

  /* for all variables in this segment do */
  for (sym = setFirstItem (map->syms); sym; sym = setNextItem (map->syms))
    {
      int size = getSize (sym->type);

#if 0
      fprintf (stderr, "%s\t%s: sym: %s\tused: %d\tSPEC_ABSA: %d\tSPEC_AGGREGATE: %d\tCODE: %d\n\
CODESPACE: %d\tCONST: %d\tPTRCONST: %d\tSPEC_CONST: %d\n", __FUNCTION__, map->sname, sym->name, sym->used, SPEC_ABSA (sym->etype), IS_AGGREGATE (sym->type), IS_CODE (sym->etype), IN_CODESPACE (SPEC_OCLS (sym->etype)), IS_CONSTANT (sym->etype), IS_PTR_CONST (sym->etype), SPEC_CONST (sym->etype));
      printTypeChain (sym->type, stderr);
      fprintf (stderr, "\n");
#endif

      if (SPEC_ABSA (sym->etype) && PIC16_IS_CONFIG_ADDRESS (SPEC_ADDR (sym->etype)))
        {
          pic16_assignConfigWordValue (SPEC_ADDR (sym->etype), (int) ulFromVal (list2val (sym->ival, TRUE)));
          continue;
        }

      if (SPEC_ABSA (sym->etype) && PIC16_IS_IDLOC_ADDRESS (SPEC_ADDR (sym->etype)))
        {
          pic16_assignIdByteValue (SPEC_ADDR (sym->etype), (char) ulFromVal (list2val (sym->ival, TRUE)));
          continue;
        }

      /* if it is "extern" then do nothing */
      if (IS_EXTERN (sym->etype) /* && !SPEC_ABSA(sym->etype) */ )
        {
          checkAddSym (&externs, sym);
          continue;
        }

      /* bail out on incomplete types */
      if (0 == size)
        {
          werrorfl (sym->fileDef, sym->lineDef, E_UNKNOWN_SIZE, sym->name);
        }

      /* if it is not static add it to the public
         table */
      if (!IS_STATIC (sym->etype))
        {
          /* do not emit if it is a config word declaration */
          checkAddSym (&publics, sym);
        }

      /* print extra debug info if required */
      if (options.debug || sym->level == 0)
        {
          /* NOTE to me - cdbFile may be null in which case,
           * the sym name will be printed to stdout. oh well */
          debugFile->writeSymbol (sym);
        }

      /* if it has an absolute address */
      if (SPEC_ABSA (sym->etype))
        {
//        fprintf(stderr, "%s:%d spec_absa is true for symbol: %s\n",
//                __FILE__, __LINE__, sym->name);

          if (!sym->ival && IS_ARRAY (sym->type) && IS_CHAR (sym->type->next) && SPEC_CVAL (sym->etype).v_char)
            {
              /* symbol has absolute address but no initial value */
              /* special case for character strings */

//            fprintf(stderr, "%s:%d printing code string from %s\n", __FILE__, __LINE__, sym->rname);

              pic16_pCodeConstString (sym->rname, SPEC_CVAL (sym->etype).v_char, getSize (sym->type));
            }
          else
            {
              pBlock *pb;
              symbol *asym;
              absSym *abSym;
              pCode *pcf;

              /* symbol has absolute address and initial value */
              ++noAlloc;
              resolveIvalSym (sym->ival, sym->type);
              asym = newSymbol (sym->rname, 0);
              abSym = Safe_alloc(sizeof (absSym));
              strncpy(abSym->name, sym->rname, sizeof(abSym->name) - 1);
              abSym->name[sizeof(abSym->name) - 1] = '\0';
              abSym->address = SPEC_ADDR (sym->etype);
              addSet (&absSymSet, abSym);

              pb = pic16_newpCodeChain (NULL, 'A', pic16_newpCodeCharP ("; Starting pCode block for absolute Ival"));
              pic16_addpBlock (pb);

              pcf = pic16_newpCodeFunction (moduleName, asym->name);
              PCF (pcf)->absblock = TRUE;

              pic16_addpCode2pBlock (pb, pcf);
              pic16_addpCode2pBlock (pb, pic16_newpCodeLabel (sym->rname, -1));
              //fprintf(stderr, "%s:%d [1] generating init for label: %s\n", __FILE__, __LINE__, sym->rname);
              /* if it has an initial value */
              if (sym->ival)
                {
                  pic16_printIval (sym, sym->type, sym->ival, 'p', (void *) pb);
                  pic16_flushDB ('p', (void *) pb);
                }

              pic16_addpCode2pBlock (pb, pic16_newpCodeFunction (NULL, NULL));
              --noAlloc;
            }
        }
      else
        {
//        fprintf(stderr, "%s:%d spec_absa is false for symbol: %s\n",
//               __FILE__, __LINE__, sym->name);


          /* if it has an initial value */
          if (sym->ival)
            {
              pBlock *pb;

              /* symbol doesn't have absolute address but has initial value */
              dbuf_printf (&code->oBuf, "%s:\n", sym->rname);
              ++noAlloc;
              resolveIvalSym (sym->ival, sym->type);

              pb = pic16_newpCodeChain (NULL, 'P', pic16_newpCodeCharP ("; Starting pCode block for Ival"));
              pic16_addpBlock (pb);

              if (!didcode)
                {
                  /* make sure that 'code' directive is emitted before, once */
                  pic16_addpCode2pBlock (pb, pic16_newpCodeAsmDir ("code", NULL));

                  ++didcode;
                }

              pic16_addpCode2pBlock (pb, pic16_newpCodeLabel (sym->rname, -1));
              //fprintf(stderr, "%s:%d [2] generating init for label: %s\n", __FILE__, __LINE__, sym->rname);
              pic16_printIval (sym, sym->type, sym->ival, 'p', (void *) pb);
              pic16_flushDB ('p', (void *) pb);
              --noAlloc;
            }
          else
            {

              /* symbol doesn't have absolute address and no initial value */
              /* allocate space */
//            fprintf(stderr, "%s:%d [3] generating init for label: %s\n", __FILE__, __LINE__, sym->rname);
              dbuf_printf (&code->oBuf, "%s:\n", sym->rname);
              /* special case for character strings */
              if (IS_ARRAY (sym->type) && IS_CHAR (sym->type->next) && SPEC_CVAL (sym->etype).v_char)
                {

//                fprintf(stderr, "%s:%d printing code string for %s\n", __FILE__, __LINE__, sym->rname);

                  pic16_pCodeConstString (sym->rname, SPEC_CVAL (sym->etype).v_char, getSize (sym->type));
                }
              else
                {
                  assert (0);
                }
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* pic16_emitConfigRegs - emits the configuration registers              */
/*-----------------------------------------------------------------*/
void pic16_emitConfigRegs (FILE *of)
{
  int i;

  if (pic16_config_options)
    {
      pic16_config_options_t *p;

      /* check if mixing config directives */
      for (i = 0; i <= (pic16->cwInfo.confAddrEnd - pic16->cwInfo.confAddrStart); ++i)
        if (pic16->cwInfo.crInfo[i].emit)
          {
            werror (E_MIXING_CONFIG);
            break;
          }

      /* emit new config directives */
      for (p = pic16_config_options; p; p = p->next)
        fprintf (of, "\t%s\n", p->config_str);
    }

  /* emit old __config directives */
  for (i = 0; i <= (pic16->cwInfo.confAddrEnd - pic16->cwInfo.confAddrStart); ++i)
    if (pic16->cwInfo.crInfo[i].emit)        //mask != -1)
      fprintf (of, "\t__config 0x%06x, 0x%02x\n",
      pic16->cwInfo.confAddrStart + i,
      (unsigned char) pic16->cwInfo.crInfo[i].value);
}

void pic16_emitIDRegs (FILE *of)
{
  int i;

  for (i=0; i <= (pic16->idInfo.idAddrEnd - pic16->idInfo.idAddrStart); i++)
    if (pic16->idInfo.irInfo[i].emit)
      fprintf (of, "\t__idlocs 0x%06x, 0x%02x\n",
      pic16->idInfo.idAddrStart + i,
      (unsigned char) pic16->idInfo.irInfo[i].value);
}


static void
pic16emitMaps ()
{
  /* no special considerations for the following
     data, idata & bit & xdata */
  pic16emitRegularMap (data, TRUE, TRUE);
  pic16emitRegularMap (idata, TRUE, TRUE);
  pic16emitRegularMap (bit, TRUE, FALSE);
  pic16emitRegularMap (xdata, TRUE, TRUE);
  pic16emitRegularMap (sfr, FALSE, FALSE);
  pic16emitRegularMap (sfrbit, FALSE, FALSE);
  pic16emitRegularMap (code, TRUE, FALSE);
  pic16emitStaticSeg (statsg);
  pic16emitStaticSeg (c_abs);
}

/*-----------------------------------------------------------------*/
/* createInterruptVect - creates the interrupt vector              */
/*-----------------------------------------------------------------*/
static void
pic16createInterruptVect (struct dbuf_s * vBuf)
{
        /* if the main is only a prototype ie. no body then do nothing */
#if 0
        if (!IFFUNC_HASBODY(mainf->type)) {
                /* if ! compile only then main function should be present */
                if (!options.cc_only)
                        werror (E_NO_MAIN);
                return;
        }
#endif
#if 0
        if((!pic16_options.omit_ivt) || (pic16_options.omit_ivt && pic16_options.leave_reset)) {
                dbuf_printf (vBuf, ";\t.area\t%s\n", CODE_NAME);
                dbuf_printf (vBuf, ".intvecs\tcode\t0x%06x\n", pic16_options.ivt_loc);

                /* this is an overkill since WE are the port,
                 * and we know if we have a genIVT function! */
                if(port->genIVT) {
                        port->genIVT(vFile, interrupts, maxInterrupts);
                }
        }
#endif

}


/*-----------------------------------------------------------------*/
/* pic16initialComments - puts in some initial comments            */
/*-----------------------------------------------------------------*/
static void
pic16initialComments (FILE * afile)
{
    initialComments (afile);
    fprintf (afile, "; PIC16 port for the Microchip 16-bit core micros\n");
    if (pic16_options.xinst) {
        fprintf (afile, "; * Extended Instruction Set\n");
    } // if

    if (pic16_mplab_comp) {
        fprintf(afile, "; * MPLAB/MPASM/MPASMWIN/MPLINK compatibility mode enabled\n");
    } // if
    fprintf (afile, "%s", iComments2);

    if (options.debug) {
        fprintf (afile, "\n\t.ident \"SDCC version %s #%s [pic16 port]%s\"\n",
                SDCC_VERSION_STR, getBuildNumber(), (!pic16_options.xinst?"":" {extended}") );
    } // if
}

int
pic16_stringInSet(const char *str, set **world, int autoAdd)
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

static int
pic16_emitSymbolIfNew(FILE *file, const char *fmt, const char *sym, int checkLocals)
{
  static set *emitted = NULL;

  if (!pic16_stringInSet(sym, &emitted, 1)) {
    /* sym was not in emittedSymbols */
    if (!checkLocals || !pic16_stringInSet(sym, &pic16_localFunctions, 0)) {
      /* sym is not a locally defined function---avoid bug #1443651 */
      fprintf( file, fmt, sym );
      return 0;
    }
  }
  return 1;
}

/*-----------------------------------------------------------------*/
/* printPublics - generates global declarations for publics        */
/*-----------------------------------------------------------------*/
static void
pic16printPublics (FILE *afile)
{
  symbol *sym;

        fprintf (afile, "\n%s", iComments2);
        fprintf (afile, "; public variables in this module\n");
        fprintf (afile, "%s", iComments2);

        for(sym = setFirstItem (publics); sym; sym = setNextItem (publics))
          /* sanity check */
          if(!IS_STATIC(sym->etype))
                pic16_emitSymbolIfNew(afile, "\tglobal\t%s\n", sym->rname, 0);
}

/*-----------------------------------------------------------------*/
/* printExterns - generates extern declarations for externs        */
/*-----------------------------------------------------------------*/
static void
pic16_printExterns(FILE *afile)
{
  symbol *sym;

        /* print nothing if no externs to declare */
        if(!elementsInSet(externs) && !elementsInSet(pic16_builtin_functions))
                return;

        fprintf(afile, "\n%s", iComments2);
        fprintf(afile, "; extern variables in this module\n");
        fprintf(afile, "%s", iComments2);

        for(sym = setFirstItem(externs); sym; sym = setNextItem(externs))
                pic16_emitSymbolIfNew(afile, "\textern\t%s\n", sym->rname, 1);

        for(sym = setFirstItem(pic16_builtin_functions); sym; sym = setNextItem(pic16_builtin_functions))
                pic16_emitSymbolIfNew(afile, "\textern\t_%s\n", sym->name, 1);
}

/*-----------------------------------------------------------------*/
/* emitOverlay - will emit code for the overlay stuff              */
/*-----------------------------------------------------------------*/
static void
pic16emitOverlay (struct dbuf_s *aBuf)
{
  set *ovrset;

  if (!elementsInSet (ovrSetSets))
    dbuf_printf (aBuf, ";\t.area\t%s\n", port->mem.overlay_name);

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
          dbuf_printf (aBuf, ";\t.area _DUMMY\n");
          /* output the area informtion */
          dbuf_printf (aBuf, ";\t.area\t%s\n", port->mem.overlay_name); /* MOF */
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
              && !IS_STATIC (sym->etype)) {
//            fprintf(stderr, "%s:%d %s accessed\n", __FILE__, __LINE__, __FUNCTION__);
              checkAddSym(&publics, sym);
//          addSetHead (&publics, sym);
          }

          /* if extern then do nothing or is a function
             then do nothing */
          if (IS_FUNC (sym->type))
            continue;


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
emitStatistics(FILE *asmFile)
{
  unsigned long isize, udsize, ramsize;
  statistics.isize = pic16_countInstructions();
  isize = (((long)statistics.isize) >= 0) ? statistics.isize : 0;
  udsize = (((long)statistics.udsize) >= 0) ? statistics.udsize : 0;
  ramsize = pic16 ? pic16->RAMsize : 0x200;
  ramsize -= 256; /* ignore access bank and SFRs */
  if (ramsize == 0) ramsize = 64; /* prevent division by zero (below) */

  fprintf (asmFile, "\n\n; Statistics:\n");
  fprintf (asmFile, "; code size:\t%5ld (0x%04lx) bytes (%5.2f%%)\n;           \t%5ld (0x%04lx) words\n",
    isize, isize, (isize*100.0)/(128UL << 10),
    isize>>1, isize>>1);
  fprintf (asmFile, "; udata size:\t%5ld (0x%04lx) bytes (%5.2f%%)\n",
    udsize, udsize, (udsize*100.0) / (1.0 * ramsize));
  fprintf (asmFile, "; access size:\t%5ld (0x%04lx) bytes\n",
    statistics.intsize, statistics.intsize);

  fprintf (asmFile, "\n\n");
}



/*-----------------------------------------------------------------*/
/* glue - the final glue that hold the whole thing together        */
/*-----------------------------------------------------------------*/
void
pic16glue ()
{
  FILE *asmFile;
  struct dbuf_s ovrBuf;
  struct dbuf_s vBuf;

    dbuf_init(&ovrBuf, 4096);
    dbuf_init(&vBuf, 4096);

    mainf = newSymbol ("main", 0);
    mainf->block = 0;

    mainf = findSymWithLevel(SymbolTab, mainf);

    if (mainf && IFFUNC_HASBODY(mainf->type) && pic16->xinst && !has_xinst_config)
      {
        fprintf(stderr, "WARNING: The target device seems to support XINST and no #pragma config XINST=OFF was found.\n");
        fprintf(stderr, "         The code generated by SDCC does probably not work when XINST is enabled (possibly by default).\n");
        fprintf(stderr, "         Please make sure to disable XINST.\n");
        fprintf(stderr, "         (If the target does not actually support XINST, please report this as a bug in SDCC.)\n");
      } // if

    pic16_pCodeInitRegisters();

    if(pic16_options.no_crt && mainf && IFFUNC_HASBODY(mainf->type)) {
      pBlock *pb = pic16_newpCodeChain(NULL,'X',pic16_newpCodeCharP("; Starting pCode block"));

        pic16_addpBlock(pb);

        /* entry point @ start of CSEG */
        pic16_addpCode2pBlock(pb,pic16_newpCodeLabel("__sdcc_program_startup",-1));

        if(initsfpnt) {
          pic16_addpCode2pBlock(pb, pic16_newpCode(POC_LFSR,
                  pic16_popGetLit2(1, pic16_newpCodeOpRegFromStr("_stack_end"))));
          pic16_addpCode2pBlock(pb, pic16_newpCode(POC_LFSR,
                  pic16_popGetLit2(2, pic16_newpCodeOpRegFromStr("_stack_end"))));
        }

        /* put in the call to main */
        pic16_addpCode2pBlock(pb,pic16_newpCode(POC_CALL,pic16_newpCodeOp("_main",PO_STR)));

        
        pic16_addpCode2pBlock(pb,pic16_newpCodeCharP(";\treturn from main will return to caller\n"));
        pic16_addpCode2pBlock(pb,pic16_newpCode(POC_RETURN,NULL));
    }

    /* At this point we've got all the code in the form of pCode structures */
    /* Now it needs to be rearranged into the order it should be placed in the */
    /* code space */

    pic16_movepBlock2Head('P');              // Last
    pic16_movepBlock2Head(code->dbName);
    pic16_movepBlock2Head('X');
    pic16_movepBlock2Head(statsg->dbName);   // First

    /* print the global struct definitions */

    /* PENDING: this isnt the best place but it will do */
    if (port->general.glue_up_main) {
      /* create the interrupt vector table */
      pic16createInterruptVect (&vBuf);
    }

    /* emit code for the all the variables declared */
    pic16emitMaps ();

    /* do the overlay segments */
    pic16emitOverlay(&ovrBuf);
    pic16_AnalyzepCode('*');

#if 1
    if(pic16_options.dumpcalltree) {
        FILE *cFile;

        SNPRINTF(buffer, sizeof(buffer), "%s.calltree", dstFileName);
        cFile = fopen(buffer, "w");
        pic16_printCallTree( cFile );
        fclose(cFile);
    }
#endif

    pic16_InlinepCode();
    pic16_AnalyzepCode('*');

    if(pic16_debug_verbose)
      pic16_pcode_test();

    /* now put it all together into the assembler file */
    /* create the assembler file name */
    if((noAssemble || options.c1mode)  && fullDstFileName) {
      SNPRINTF(buffer, sizeof(buffer), "%s", fullDstFileName);
    } else {
      SNPRINTF(buffer, sizeof(buffer), "%s.asm", dstFileName);
    }

    if(!(asmFile = fopen (buffer, "w"))) {
      werror (E_FILE_OPEN_ERR, buffer);
      exit (1);
    }

    /* initial comments */
    pic16initialComments (asmFile);

    /* print module name */
    if(options.debug)
      fprintf(asmFile, "\t.file\t\"%s\"\n", fullSrcFileName);

    /* Let the port generate any global directives, etc. */
    if(port->genAssemblerPreamble) {
      port->genAssemblerPreamble(asmFile);
    }

    /* Put all variables into a cblock */
    pic16_AnalyzeBanking();

#if 0
    if(pic16_options.opt_flags & OF_LR_SUPPORT) {
      pic16_OptimizeLocalRegs();
    }
#endif

    /* remove redundant BANKSELs -- added by RN 2005-01-17 */
    if(pic16_options.opt_banksel > 1) {
      pic16_OptimizeBanksel();
    }

    /* turn GOTOs into BRAs -- added by RN 2004-11-16 */
    if(!(pic16_options.opt_flags & OF_NO_OPTIMIZE_GOTO)) {
      pic16_OptimizeJumps();
    }

    /* print the global variables in this module */
    pic16printPublics (asmFile);

    /* print the extern variables to this module */
    pic16_printExterns(asmFile);

    pic16_writeUsedRegs(asmFile);

#if 0
    /* no xdata in pic */
    /* if external stack then reserve space of it */
    if(mainf && IFFUNC_HASBODY(mainf->type) && options.useXstack ) {
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile, "; external stack\n");
      fprintf (asmFile, "%s", iComments2);
      fprintf (asmFile,";\t.area XSEG (XDATA)\n"); /* MOF */
      fprintf (asmFile,";\t.ds 256\n");
    }
#endif

#if 0
    /* no xdata in pic */
    /* copy xtern ram data */
    fprintf (asmFile, "%s", iComments2);
    fprintf (asmFile, "; external ram data\n");
    fprintf (asmFile, "%s", iComments2);
    dbuf_write_and_destroy (&xdata->oBuf, asmFile);
#endif

#if 0
    /* copy the bit segment */
    fprintf (asmFile, "%s", iComments2);
    fprintf (asmFile, "; bit data\n");
    fprintf (asmFile, "%s", iComments2);
    dbuf_write_and_destroy (&bit->oBuf, asmFile);
#endif

    /* copy the interrupt vector table */
    if(mainf && IFFUNC_HASBODY(mainf->type)) {
      fprintf (asmFile, "\n%s", iComments2);
      fprintf (asmFile, "; interrupt vector\n");
      fprintf (asmFile, "%s", iComments2);
      dbuf_write_and_destroy (&vBuf, asmFile);
    }

    /* copy global & static initialisations */
    fprintf (asmFile, "\n%s", iComments2);
    fprintf (asmFile, "; global & static initialisations\n");
    fprintf (asmFile, "%s", iComments2);

    if(pic16_debug_verbose)
      fprintf(asmFile, "; A code from now on!\n");

    pic16_copypCode(asmFile, 'A');

    if(pic16_options.no_crt) {
      if(mainf && IFFUNC_HASBODY(mainf->type)) {
        fprintf(asmFile, "\tcode\n");
        fprintf(asmFile,"__sdcc_gsinit_startup:\n");
      }
    }

//    dbuf_write_and_destroy (&code->oBuf, stderr);

    fprintf(asmFile, "; I code from now on!\n");
    pic16_copypCode(asmFile, 'I');

    if(pic16_debug_verbose)
      fprintf(asmFile, "; dbName from now on!\n");

    pic16_copypCode(asmFile, statsg->dbName);

    if(pic16_options.no_crt) {
      if (port->general.glue_up_main && mainf && IFFUNC_HASBODY(mainf->type)) {
        fprintf (asmFile,"\tgoto\t__sdcc_program_startup\n");
      }
    }

    if(pic16_debug_verbose)
      fprintf(asmFile, "; X code from now on!\n");

    pic16_copypCode(asmFile, 'X');

    if(pic16_debug_verbose)
      fprintf(asmFile, "; M code from now on!\n");

    pic16_copypCode(asmFile, 'M');

    pic16_copypCode(asmFile, code->dbName);

    pic16_copypCode(asmFile, 'P');

    emitStatistics(asmFile);

    fprintf (asmFile,"\tend\n");
    fclose (asmFile);
}
