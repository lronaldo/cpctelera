/*----------------------------------------------------------------------
    SDCCval.c :- has routine to do all kinds of fun stuff with the
                value wrapper & with initialiser lists.

    Written By - Sandeep Dutta . sandeep.dutta@usa.net (1998)

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

#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include "newalloc.h"
#include "dbuf_string.h"

long cNestLevel;


/*-----------------------------------------------------------------*/
/* newValue - allocates and returns a new value                    */
/*-----------------------------------------------------------------*/
value *
newValue (void)
{
  value *val;

  val = Safe_alloc (sizeof (value));

  return val;
}

/*-----------------------------------------------------------------*/
/* newiList - new initializer list                                 */
/*-----------------------------------------------------------------*/
initList *
newiList (int type, void *ilist)
{
  initList *nilist;

  nilist = Safe_alloc (sizeof (initList));

  nilist->type = type;
  nilist->filename = lexFilename;
  nilist->lineno = lexLineno;
  nilist->designation = NULL;

  switch (type)
    {
    case INIT_NODE:
      nilist->init.node = (struct ast *) ilist;
      break;

    case INIT_DEEP:
      nilist->init.deep = (struct initList *) ilist;
      break;
    }

  return nilist;
}

/*------------------------------------------------------------------*/
/* revinit   - reverses the initial values for a value chain        */
/*------------------------------------------------------------------*/
initList *
revinit (initList * val)
{
  initList *prev, *curr, *next;

  if (!val)
    return NULL;

  prev = val;
  curr = val->next;

  while (curr)
    {
      next = curr->next;
      curr->next = prev;
      prev = curr;
      curr = next;
    }
  val->next = (void *) NULL;
  return prev;
}

bool
convertIListToConstList (initList * src, literalList ** lList, int size)
{
  int cnt = 0;
  initList *iLoop;
  literalList *head, *last, *newL;

  head = last = NULL;

  if (src && src->type != INIT_DEEP)
    {
      return FALSE;
    }

  iLoop = src ? src->init.deep : NULL;

  while (iLoop)
    {
      if (iLoop->designation != NULL)
        {
          return FALSE;
        }

      if (iLoop->type != INIT_NODE)
        {
          return FALSE;
        }

      if (!IS_AST_LIT_VALUE (decorateType (resolveSymbols (iLoop->init.node), RESULT_TYPE_NONE)))
        {
          return FALSE;
        }
      iLoop = iLoop->next;
      cnt++;
    }
  if (!size)
    {
      size = cnt;
    }

  /* We've now established that the initializer list contains only literal values. */

  iLoop = src ? src->init.deep : NULL;
  while (size--)
    {
      double val = iLoop ? AST_FLOAT_VALUE (iLoop->init.node) : 0;

      if (last && last->literalValue == val)
        {
          last->count++;
        }
      else
        {
          newL = Safe_alloc (sizeof (literalList));
          newL->literalValue = val;
          newL->count = 1;
          newL->next = NULL;

          if (last)
            {
              last->next = newL;
            }
          else
            {
              head = newL;
            }
          last = newL;
        }
      iLoop = iLoop ? iLoop->next : NULL;
    }

  if (!head)
    {
      return FALSE;
    }

  *lList = head;
  return TRUE;
}

literalList *
copyLiteralList (literalList * src)
{
  literalList *head, *prev, *newL;

  head = prev = NULL;

  while (src)
    {
      newL = Safe_alloc (sizeof (literalList));

      newL->literalValue = src->literalValue;
      newL->count = src->count;
      newL->next = NULL;

      if (prev)
        {
          prev->next = newL;
        }
      else
        {
          head = newL;
        }
      prev = newL;
      src = src->next;
    }

  return head;
}

/*------------------------------------------------------------------*/
/* copyIlist - copy initializer list                                */
/*------------------------------------------------------------------*/
initList *
copyIlist (initList * src)
{
  initList *dest = NULL;

  if (!src)
    return NULL;

  switch (src->type)
    {
    case INIT_DEEP:
      dest = newiList (INIT_DEEP, copyIlist (src->init.deep));
      break;
    case INIT_NODE:
      dest = newiList (INIT_NODE, copyAst (src->init.node));
      break;
    }

  if (src->designation)
    dest->designation = copyDesignation (src->designation);

  if (src->next)
    assert (dest != NULL);
  dest->next = copyIlist (src->next);

  return dest;
}

/*------------------------------------------------------------------*/
/* list2int - converts the first element of the list to value       */
/*------------------------------------------------------------------*/
double
list2int (initList * val)
{
  initList *i = val;

  assert (i->type != INIT_HOLE);

  if (i->type == INIT_DEEP)
    return list2int (val->init.deep);

  return floatFromVal (constExprValue (val->init.node, TRUE));
}

/*------------------------------------------------------------------*/
/* list2val - converts the first element of the list to value       */
/*------------------------------------------------------------------*/
value *
list2val (initList * val, int check)
{
  if (!val)
    return NULL;

  if(val->type == INIT_HOLE)
    return NULL;

  if (val->type == INIT_DEEP)
    return list2val (val->init.deep, check);

  if (val->type == INIT_NODE && val->init.node->opval.op == CAST)
    return constExprValue (val->init.node->right, check);

  return constExprValue (val->init.node, check);
}

/*------------------------------------------------------------------*/
/* list2expr - returns the first expression in the initializer list */
/*------------------------------------------------------------------*/
ast *
list2expr (initList * ilist)
{
  if (!ilist)
    return NULL;

  assert (ilist->type != INIT_HOLE);

  if (ilist->type == INIT_DEEP)
    return list2expr (ilist->init.deep);
  return ilist->init.node;
}

/*------------------------------------------------------------------*/
/* resolveIvalSym - resolve symbols in initial values               */
/*------------------------------------------------------------------*/
void
resolveIvalSym (initList *ilist, sym_link *type)
{
  int is_ptr = IS_PTR (type) || (IS_ARRAY(type) && IS_PTR(type->next));
  RESULT_TYPE resultType = getResultTypeFromType (getSpec (type));

  while (ilist)
    {
      if (ilist->type == INIT_NODE)
        {
          ilist->init.node = decorateType (resolveSymbols (ilist->init.node), is_ptr ? RESULT_TYPE_INT : resultType);
        }
      else if (ilist->type == INIT_DEEP)
        {
          resolveIvalSym (ilist->init.deep, type);
        }

      ilist = ilist->next;
    }
}

/*-----------------------------------------------------------------*/
/* newDesignation - new designation                                */
/*-----------------------------------------------------------------*/
designation *
newDesignation (int type, void *designator)
{
  designation *ndesignation;

  ndesignation = Safe_alloc (sizeof (designation));

  ndesignation->type = type;
  ndesignation->filename = lexFilename;
  ndesignation->lineno = lexLineno;

  switch (type)
    {
    case DESIGNATOR_STRUCT:
      ndesignation->designator.tag = (struct symbol *) designator;
      break;

    case DESIGNATOR_ARRAY:
      ndesignation->designator.elemno = * ((int *) designator);
      break;
    }

  return ndesignation;
}

/*------------------------------------------------------------------*/
/* revDesignation   - reverses the designation chain                */
/*------------------------------------------------------------------*/
designation *
revDesignation (designation * val)
{
  designation *prev, *curr, *next;

  if (!val)
    return NULL;

  prev = val;
  curr = val->next;

  while (curr)
    {
      next = curr->next;
      curr->next = prev;
      prev = curr;
      curr = next;
    }
  val->next = (void *) NULL;
  return prev;
}

/*------------------------------------------------------------------*/
/* copyDesignation - copy designation list                          */
/*------------------------------------------------------------------*/
designation *
copyDesignation (designation * src)
{
  designation *dest = NULL;

  if (!src)
    return NULL;

  switch (src->type)
    {
    case DESIGNATOR_STRUCT:
      dest = newDesignation (DESIGNATOR_STRUCT, copySymbol (src->designator.tag));
      break;
    case DESIGNATOR_ARRAY:
      dest = newDesignation (DESIGNATOR_ARRAY, &(src->designator.elemno) );
      break;
    }

  dest->lineno = src->lineno;
  dest->filename = src->filename;

  if (src->next)
    dest->next = copyDesignation (src->next);

  return dest;
}

/*------------------------------------------------------------------*/
/* moveNestedInit - rewrites an initList node with a nested         */
/*                  designator to remove one level of nesting.      */
/*------------------------------------------------------------------*/
static
void moveNestedInit(initList *deepParent, initList *src)
{
  initList *dst = NULL, **eol;

  /** Create new initList element */
  switch (src->type)
    {
    case INIT_NODE:
      dst = newiList(INIT_NODE, src->init.node);
      break;
    case INIT_DEEP:
      dst = newiList(INIT_DEEP, src->init.deep);
      break;
    }
  dst->filename = src->filename;
  dst->lineno = src->lineno;
  dst->designation = src->designation->next;

  /* add dst to end of deepParent */
  if (deepParent->type != INIT_DEEP)
    {
      werrorfl (deepParent->filename, deepParent->lineno,
                E_INIT_STRUCT, "<unknown>");
      return;
    }
  for (eol = &(deepParent->init.deep); *eol ; )
    eol = &((*eol)->next);
  *eol = dst;
}

/*-----------------------------------------------------------------*/
/* findStructField - find a specific field in a struct definition  */
/*-----------------------------------------------------------------*/
static int
findStructField (symbol *fields, symbol *target)
{
  int i;

  for (i=0 ; fields; fields = fields->next)
    {
      /* skip past unnamed bitfields */
      if (IS_BITFIELD (fields->type) && SPEC_BUNNAMED (fields->etype))
        continue;
      /* is this it? */
      if (strcmp(fields->name, target->name) == 0)
        return i;
      i++;
    }

  /* not found */
  werrorfl (target->fileDef, target->lineDef, E_NOT_MEMBER, target->name);
  return 0;
}

/*------------------------------------------------------------------*/
/* reorderIlist - expands an initializer list to match designated   */
/*                initializers.                                     */
/*------------------------------------------------------------------*/
initList *reorderIlist (sym_link * type, initList * ilist)
{
  initList *iloop, *nlist, **nlistArray;
  symbol *sflds;
  int size=0, idx;

  if (!IS_AGGREGATE (type))
    /* uninteresting: no designated initializers */
    return ilist;

  if (ilist && ilist->type == INIT_HOLE)
    /* ditto; just a uninitialized hole */
    return ilist;

  /* special case: check for string initializer */
  if (IS_ARRAY (type) && IS_CHAR (type->next) &&
      ilist && ilist->type == INIT_NODE)
    {
      ast *iast = ilist->init.node;
      value *v = (iast->type == EX_VALUE ? iast->opval.val : NULL);
      if (v && IS_ARRAY (v->type) && IS_CHAR (v->etype))
        {
          /* yep, it's a string; no changes needed here. */
          return ilist;
        }
    }

  if (ilist && ilist->type != INIT_DEEP)
    {
      werrorfl (ilist->filename, ilist->lineno, E_INIT_STRUCT, "<unknown>");
      return NULL;
    }

  /* okay, allocate enough space */
  if (IS_ARRAY (type))
    size = getNelements(type, ilist);
  else if (IS_STRUCT (type))
    {
      /* compute size from struct type. */
      size = 0;
      for (sflds = SPEC_STRUCT (type)->fields; sflds; sflds = sflds->next)
        {
          /* skip past unnamed bitfields */
          if (IS_BITFIELD (sflds->type) && SPEC_BUNNAMED (sflds->etype))
            continue;
          size++;
        }
    }
  nlistArray = Safe_calloc ( size, sizeof(initList *) );

  /* pull together all the initializers into an ordered list */
  iloop = ilist ? ilist->init.deep : NULL;
  for (idx = 0 ; iloop ; iloop = iloop->next, idx++)
    {
      if (iloop->designation)
        {
          assert (iloop->type != INIT_HOLE);

          if (IS_ARRAY (type))
            {
              if (iloop->designation->type == DESIGNATOR_ARRAY)
                idx = iloop->designation->designator.elemno;
              else
                werrorfl (iloop->filename, iloop->lineno, E_BAD_DESIGNATOR);
            }
          else if (IS_STRUCT (type))
            {
              if (iloop->designation->type == DESIGNATOR_STRUCT)
                idx = findStructField (SPEC_STRUCT (type)->fields,
                                       iloop->designation->designator.tag);
              else
                werrorfl (iloop->filename, iloop->lineno, E_BAD_DESIGNATOR);
            }
          else
            {
              assert (0);
            }

          if (iloop->designation->next)
            {
              if (idx >= size)
                continue;
              if (nlistArray[idx] == NULL)
                nlistArray[idx] = newiList(INIT_DEEP, NULL);
              moveNestedInit(nlistArray[idx], iloop);
              continue;
            }
        }

      /* overwrite any existing entry with iloop */
      if (iloop->type != INIT_HOLE)
        {
          if (idx >= size)
            continue;
          if (nlistArray[idx] != NULL)
            werrorfl (iloop->filename, iloop->lineno, W_DUPLICATE_INIT, idx);
          nlistArray[idx] = iloop;
        }
    }

  /* create new list from nlistArray/size */
  nlist = NULL;
  sflds = IS_STRUCT (type) ? SPEC_STRUCT (type)->fields : NULL;
  for ( idx=0; idx < size; idx++ )
    {
      initList *src = nlistArray[idx], *dst = NULL;
      if (!src || src->type==INIT_HOLE)
        {
          dst = newiList(INIT_HOLE, NULL);
          dst->filename = ilist->filename;
          dst->lineno = ilist->lineno;
        }
      else
        {
          switch (src->type)
            {
            case INIT_NODE:
              dst = newiList(INIT_NODE, src->init.node);
              break;
            case INIT_DEEP:
              dst = newiList(INIT_DEEP, src->init.deep);
              break;
            }
          dst->filename = src->filename;
          dst->lineno = src->lineno;
        }
      dst->next = nlist;
      nlist = dst;
      /* advance to next field which is not an unnamed bitfield */
      do
        {
          sflds = sflds ? sflds->next : NULL;
        }
      while (sflds &&
             IS_BITFIELD (sflds->type) && SPEC_BUNNAMED (sflds->etype));
    }

  nlist = newiList(INIT_DEEP, revinit (nlist));
  nlist->filename = ilist->filename;
  nlist->lineno = ilist->lineno;
  nlist->designation = ilist->designation;
  nlist->next = ilist->next;
  return nlist;
}

/*------------------------------------------------------------------*/
/* symbolVal - creates a value for a symbol                         */
/*------------------------------------------------------------------*/
value *
symbolVal (symbol * sym)
{
  value *val;

  if (!sym)
    return NULL;

  val = newValue ();
  val->sym = sym;

  if (sym->type)
    {
      val->type = sym->type;
      val->etype = getSpec (val->type);
    }

  if (*sym->rname)
    {
      SNPRINTF (val->name, sizeof (val->name), "%s", sym->rname);
    }
  else
    {
      SNPRINTF (val->name, sizeof (val->name), "_%s", sym->name);
    }

  return val;
}

/*--------------------------------------------------------------------*/
/* cheapestVal - try to reduce 'signed int' to 'char'                 */
/*--------------------------------------------------------------------*/
static value *
cheapestVal (value * val)
{
  /* only int can be reduced */
  if (!IS_INT (val->type))
    return val;

  /* long must not be changed */
  if (SPEC_LONG (val->type) || SPEC_LONGLONG (val->type))
    return val;

  /* unsigned must not be changed */
  if (SPEC_USIGN (val->type))
    return val;

  /* the only possible reduction is from signed int to (un)signed char,
     because it's automatically promoted back to signed int.

     a reduction from unsigned int to unsigned char is a bug,
     because an _unsigned_ char is promoted to _signed_ int! */
  if (SPEC_CVAL (val->type).v_int < -128 || SPEC_CVAL (val->type).v_int > 255)
    {
      /* not in the range of (un)signed char */
      return val;
    }

  SPEC_NOUN (val->type) = V_CHAR;

  /* 'unsigned char' promotes to 'signed int', so that we can
     reduce it the other way */
  if (SPEC_CVAL (val->type).v_int >= 0)
    {
      /* 'bool' promotes to 'signed int' too */
      if (SPEC_CVAL (val->type).v_int <= 1)
        {
          /* Do not use V_BIT here because in some contexts it also */
          /* implies a storage class. */
          SPEC_NOUN (val->type) = V_BOOL;
        }
      else
        {
          /* Boolean types are intrinsically unsigned, so only */
          /* set the USIGN flag for char types to avoid triggering */
          /* type checking errors/warnings. */
          SPEC_USIGN (val->type) = 1;
        }
    }

  return (val);
}

/*-----------------------------------------------------------------*/
/* double2ul - double to unsigned long conversion                  */
/*-----------------------------------------------------------------*/
unsigned long
double2ul (double val)
{
/*
 * See ISO/IEC 9899, chapter 6.3.1.4 Real floating and integer:
 * If the value of the integral part cannot be represented by the integer type, the behavior is undefined.
 * This shows up on Mac OS X i386 platform which useus SSE unit instead of the x87 FPU for floating-point operations
 */
/*
 * on Mac OS X ppc (long) 2147483648.0 equals to 2147483647, so we explicitely convert it to 0x80000000
 * on other known platforms (long) 2147483648.0 equals to -2147483648
 */
  return ((val) < 0) ? (((val) < -2147483647.0) ? 0x80000000UL : (unsigned long) -((long) -(val))) : (unsigned long) (val);
}

/*--------------------------------------------------------------------*/
/* checkConstantRange - check if constant fits in numeric range of    */
/* var type in comparisons and assignments                            */
/*--------------------------------------------------------------------*/
CCR_RESULT
checkConstantRange (sym_link * var, sym_link * lit, int op, bool exchangeLeftRight)
{
  sym_link *reType;
  TYPE_TARGET_LONGLONG litVal;
  TYPE_TARGET_ULONGLONG ulitVal;
  TYPE_TARGET_ULONGLONG signExtMask;
  TYPE_TARGET_ULONGLONG signMask;
  bool litValUnsigned;
  int varBits;

  litVal = ullFromLit (lit);
  ulitVal = (TYPE_TARGET_ULONGLONG) litVal;
  litValUnsigned = SPEC_USIGN (lit);
  varBits = bitsForType (var);
  signMask = 1ull << (varBits-1);
  signExtMask = varBits >= sizeof(TYPE_TARGET_ULONGLONG)*8 ? 0 : ~((1ull << varBits)-1);

#if 0
  printf("   ulitVal     = 0x%016lx\n", ulitVal);
  printf("   signExtMask = 0x%016lx\n", signExtMask);
  printf("   signMask    = 0x%016lx\n",signMask);
#endif

  //return CCR_OK; /* EEP - debug for long long */
  /* sanity checks */
  if (IS_FLOAT (var) || IS_FIXED (var))
    return CCR_OK;
  if (varBits < 1)
    return CCR_ALWAYS_FALSE;
  if (varBits > 64)
    return CCR_ALWAYS_TRUE;

  /* special: assignment */
  if (op == '=')
    {
      if (IS_BOOLEAN (var))
        return CCR_OK;

      if (1) // Though the else branch is dead, I still would like to keep it.
      //if (getenv ("SDCC_VERY_PEDANTIC"))
        {
          if (SPEC_USIGN (var))
            {
              if ((!litValUnsigned && litVal < 0) || (litVal & signExtMask) != 0)
                return CCR_OVL;
              return CCR_OK;
            }
          else
            {
              if (litValUnsigned)
                {
                  if ((ulitVal & (signExtMask | signMask)) == 0)
                    return CCR_OK;
                }
              else
                {
                  if ((litVal & (signExtMask | signMask)) == 0)
                    return CCR_OK;
                  if ((litVal & (signExtMask | signMask)) == (signExtMask | signMask))
                    return CCR_OK;
                }
              return CCR_OVL;
            }
        }
      else
        {
          /* ignore signedness, e.g. allow everything
             from -127...+255 for (unsigned) char */
          if ((litVal & signExtMask) == 0)
            return CCR_OK;
          if ((litVal & (signExtMask | signMask)) == (signExtMask | signMask))
            return CCR_OK;
          return CCR_OVL;
        }
    }

  if (exchangeLeftRight)
    switch (op)
      {
      case EQ_OP:
        break;
      case NE_OP:
        break;
      case '>':
        op = '<';
        break;
      case GE_OP:
        op = LE_OP;
        break;
      case '<':
        op = '>';
        break;
      case LE_OP:
        op = GE_OP;
        break;
      default:
        return CCR_ALWAYS_FALSE;
      }

  reType = computeType (var, lit, RESULT_TYPE_NONE, op);

  if (SPEC_USIGN (reType))
    {
      /* unsigned operation */
      int reBits = bitsForType (reType);
      TYPE_TARGET_ULONGLONG minValP, maxValP, minValM, maxValM;
      TYPE_TARGET_ULONGLONG opBitsMask = reBits >= sizeof(opBitsMask)*8 ? ~0ull : ((1ull << reBits)-1);

      if (IS_BOOL (var))
        {
          minValP = 0;
          maxValP = 1;
          minValM = 0;
          maxValM = 1;
        }
      else if (SPEC_USIGN (lit) && SPEC_USIGN (var))
        {
          /* both operands are unsigned, this is easy */
          minValP = 0;
          maxValP = ~signExtMask;
          /* there's only range, just copy it to 2nd set */
          minValM = minValP;
          maxValM = maxValP;
        }
      else if (SPEC_USIGN (var))
        {
          /* lit is casted from signed to unsigned, e.g.:
             unsigned u;
             u == (char) -17
             -> u == 0xffef'
           */
          minValP = 0;
          maxValP = ~signExtMask;
          /* there's only one range, just copy it to 2nd set */
          minValM = minValP;
          maxValM = maxValP;

          /* it's an unsigned operation */
          ulitVal &= opBitsMask;
        }
      else                      /* SPEC_USIGN (lit) */
        {
          /* var is casted from signed to unsigned, e.g.:
             signed char c;
             c == (unsigned) -17
             -> c == 0xffef'

             The possible values after casting var
             split up in two, nonconsecutive ranges:

             minValP =      0;  positive range: 0...127
             maxValP =   0x7f;
             minValM = 0xff80;  negative range: -128...-1
             maxValM = 0xffff;
           */

          /* positive range */
          minValP = 0;
          maxValP = ~(signExtMask | signMask);

          /* negative range */
          minValM = signExtMask | signMask;
          maxValM = (TYPE_TARGET_ULONGLONG)~0ull;     /* -1 */
          /* limit number of bits to size of return type */
          minValM &= opBitsMask;
          maxValM &= opBitsMask;
        }
#if 0
      printf("   reType      = ");
      printTypeChain (reType, NULL);
      printf("   ulitVal     = 0x%016lx\n", ulitVal);
      printf("   opBitsMask  = 0x%016lx\n", opBitsMask);
      printf("   maxValP     = 0x%016lx\n", maxValP);
      printf("   minValP     = 0x%016lx\n", minValP);
      printf("   maxValM     = 0x%016lx\n", maxValM);
      printf("   minValM     = 0x%016lx\n", minValM);
#endif

      switch (op)
        {
        case EQ_OP:            /* var == lit */
          if (ulitVal <= maxValP && ulitVal >= minValP)   /* 0 */
            return CCR_OK;
          if (ulitVal <= maxValM && ulitVal >= minValM)
            return CCR_OK;
          return CCR_ALWAYS_FALSE;
        case NE_OP:            /* var != lit */
          if (ulitVal <= maxValP && ulitVal >= minValP)   /* 0 */
            return CCR_OK;
          if (ulitVal <= maxValM && ulitVal >= minValM)
            return CCR_OK;
          return CCR_ALWAYS_TRUE;
        case '>':              /* var >  lit */
          if (ulitVal >= maxValM)
            return CCR_ALWAYS_FALSE;
          if (ulitVal < minValP) /* 0 */
            return CCR_ALWAYS_TRUE;
          return CCR_OK;
        case GE_OP:            /* var >= lit */
          if (ulitVal > maxValM)
            return CCR_ALWAYS_FALSE;
          if (ulitVal <= minValP)        /* 0 */
            return CCR_ALWAYS_TRUE;
          return CCR_OK;
        case '<':              /* var <  lit */
          if (ulitVal > maxValM)
            return CCR_ALWAYS_TRUE;
          if (ulitVal <= minValP)        /* 0 */
            return CCR_ALWAYS_FALSE;
          return CCR_OK;
        case LE_OP:            /* var <= lit */
          if (ulitVal >= maxValM)
            return CCR_ALWAYS_TRUE;
          if (ulitVal < minValP) /* 0 */
            return CCR_ALWAYS_FALSE;
          return CCR_OK;
        default:
          return CCR_ALWAYS_FALSE;
        }
    }
  else
    {
      /* signed operation */
      TYPE_TARGET_LONGLONG minVal, maxVal;

      if (IS_BOOL (var))
        {
          minVal = 0;
          maxVal = 1;
        }
      else if (SPEC_USIGN (var))
        {
          /* unsigned var, but signed operation. This happens
             when var is promoted to signed int.
             Set actual min/max values of var. */
          minVal = 0;
          maxVal = ~signExtMask;
        }
      else
        {
          /* signed var */
          minVal = signExtMask | signMask;
          maxVal = ~(signExtMask | signMask);
        }

      switch (op)
        {
        case EQ_OP:            /* var == lit */
          if (litVal > maxVal || litVal < minVal)
            return CCR_ALWAYS_FALSE;
          return CCR_OK;
        case NE_OP:            /* var != lit */
          if (litVal > maxVal || litVal < minVal)
            return CCR_ALWAYS_TRUE;
          return CCR_OK;
        case '>':              /* var >  lit */
          if (litVal >= maxVal)
            return CCR_ALWAYS_FALSE;
          if (litVal < minVal)
            return CCR_ALWAYS_TRUE;
          return CCR_OK;
        case GE_OP:            /* var >= lit */
          if (litVal > maxVal)
            return CCR_ALWAYS_FALSE;
          if (litVal <= minVal)
            return CCR_ALWAYS_TRUE;
          return CCR_OK;
        case '<':              /* var <  lit */
          if (litVal > maxVal)
            return CCR_ALWAYS_TRUE;
          if (litVal <= minVal)
            return CCR_ALWAYS_FALSE;
          return CCR_OK;
        case LE_OP:            /* var <= lit */
          if (litVal >= maxVal)
            return CCR_ALWAYS_TRUE;
          if (litVal < minVal)
            return CCR_ALWAYS_FALSE;
          return CCR_OK;
        default:
          return CCR_ALWAYS_FALSE;
        }
    }
}

#if 0
CCR_RESULT
checkConstantRange (sym_link * var, sym_link * lit, int op, bool exchangeLeftRight)
{
  CCR_RESULT result;

  printf ("checkConstantRange:\n");
  printf ("  var: ");
  printTypeChain (var, NULL);
  printf ("  lit = 0x%lx: ",ullFromLit (lit));
  printTypeChain (lit, NULL);
  printf ("  op: '");
  switch (op)
    {
    case '<': printf ("<"); break;
    case '=': printf ("="); break;
    case '>': printf (">"); break;
    case LE_OP: printf ("<="); break;
    case GE_OP: printf (">="); break;
    case EQ_OP: printf ("=="); break;
    case NE_OP: printf ("!="); break;
    default: printf ("%d",op);
    }
  printf("'\n");
  result = checkConstantRange2 (var, lit, op, exchangeLeftRight);
  switch (result)
    {
      case CCR_ALWAYS_TRUE: printf ("  CCR_ALWAYS_TRUE\n"); break;
      case CCR_ALWAYS_FALSE: printf ("  CCR_ALWAYS_FALSE\n"); break;
      case CCR_OK: printf ("  CCR_OK\n"); break;
      case CCR_OVL: printf ("  CCR_OVL\n"); break;
      default: printf("  CCR_%d\n",result);
    }
  return result;
}
#endif


/*-----------------------------------------------------------------*/
/* valueFromLit - creates a value from a literal                   */
/*-----------------------------------------------------------------*/
value *
valueFromLit (double lit)
{
  struct dbuf_s dbuf;
  value *ret;

  if ((((TYPE_TARGET_LONG) lit) - lit) == 0)
    {
      dbuf_init (&dbuf, 128);
      dbuf_printf (&dbuf, "%d", (TYPE_TARGET_LONG) lit);
      ret = constVal (dbuf_c_str (&dbuf));
      dbuf_destroy (&dbuf);
      return ret;
    }

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "%f", lit);
  ret = constFloatVal (dbuf_c_str (&dbuf));
  dbuf_destroy (&dbuf);
  return ret;
}

/*-----------------------------------------------------------------*/
/* constFloatVal - converts a FLOAT constant to value              */
/*-----------------------------------------------------------------*/
value *
constFloatVal (const char *s)
{
  value *val = newValue ();
  double sval;
  char *p;

  sval = strtod (s, &p);
  if (p == s)
    {
      werror (E_INVALID_FLOAT_CONST, s);
      return constCharVal (0);
    }

  val->type = val->etype = newLink (SPECIFIER);
  SPEC_NOUN (val->type) = V_FLOAT;
  SPEC_SCLS (val->type) = S_LITERAL;
  SPEC_CONST (val->type) = 1;
  SPEC_CVAL (val->type).v_float = sval;

  return val;
}

/*-----------------------------------------------------------------*/
/* constFixed16x16Val - converts a FIXED16X16 constant to value    */
/*-----------------------------------------------------------------*/
value *
constFixed16x16Val (const char *s)
{
  value *val = newValue ();
  double sval;
  char *p;

  sval = strtod (s, &p);
  if (p == s)
    {
      werror (E_INVALID_FLOAT_CONST, s);
      return constCharVal (0);
    }

  val->type = val->etype = newLink (SPECIFIER);
  SPEC_NOUN (val->type) = V_FLOAT;
  SPEC_SCLS (val->type) = S_LITERAL;
  SPEC_CONST (val->type) = 1;
  SPEC_CVAL (val->type).v_fixed16x16 = fixed16x16FromDouble (sval);

  return val;
}

/*-----------------------------------------------------------------*/
/* constVal - converts a constant into a cheap value type          */
/*-----------------------------------------------------------------*/
value *
constVal (const char *s)
{
  value *val = constIntVal (s);

  wassert (SPEC_NOUN (val->type) == V_INT);

  if (SPEC_LONGLONG (val->type))
    ;
  else if (SPEC_LONG (val->type))
    ;
  else if (SPEC_USIGN (val->type))
    {
      unsigned int i = SPEC_CVAL (val->type).v_uint;
      if (i < 256)
        SPEC_NOUN (val->type) = V_CHAR;
    }
  else
    {
      int i = SPEC_CVAL (val->type).v_int;
      if (i >= 0 && i < 256)
        {
          SPEC_NOUN (val->type) = V_CHAR;
          SPEC_USIGN (val->type) = TRUE;
          SPEC_CVAL (val->type).v_uint = i;
        }
      else if (i >= -128 && i < 128)
        {
          SPEC_NOUN (val->type) = V_CHAR;
        }
    }

  return val;
}

/*-----------------------------------------------------------------*/
/* constIntVal - converts an integer constant into correct type    */
/* See ISO C11, section 6.4.4.1 for the rules.                     */
/*-----------------------------------------------------------------*/
value *
constIntVal (const char *s)
{
  char *p, *p2;
  double dval;
  long long int llval;
  value *val = newValue ();
  bool decimal, u_suffix = FALSE, l_suffix = FALSE, ll_suffix = FALSE;

  val->type = val->etype = newLink (SPECIFIER);
  SPEC_SCLS (val->type) = S_LITERAL;
  SPEC_CONST (val->type) = 1;
  SPEC_USIGN (val->type) = 0;

  errno = 0;

  if (s[0] == '0')
    {
      if (s[1] == 'b' || s[1] == 'B')
        llval = strtoull (s + 2, &p, 2);
      else
        llval = strtoull (s, &p, 0);
      dval = (double)(unsigned long long int) llval;
      decimal = FALSE;
    }
  else
    {
      dval = strtod (s, &p);
      if (dval >= 0.0)
        llval = strtoull (s, &p, 0);
      else
        llval = strtoll (s, &p, 0);
      decimal = TRUE;
    }

  if (errno)
    {
      dval = 4294967295.0;
      werror (W_INVALID_INT_CONST, s, dval);
    }

  // Check suffixes
  if ((p2 = strchr (p, 'u')) || (p2 = strchr (p, 'U')))
    {
      u_suffix = TRUE;
      p2++;
      if (strchr (p2, 'u') || strchr (p2, 'U'))
        werror (E_INTEGERSUFFIX, p);
    }

  if ((p2 = strstr (p, "ll")) || (p2 = strstr (p, "LL")))
    {
      ll_suffix = TRUE;
      p2 += 2;
      if (strchr (p2, 'l') || strchr (p2, 'L'))
        werror (E_INTEGERSUFFIX, p);
    }
  else if ((p2 = strchr (p, 'l')) || (p2 = strchr (p, 'L')))
    {
      l_suffix = TRUE;
      p2++;
      if (strchr (p2, 'l') || strchr (p2, 'L'))
        werror (E_INTEGERSUFFIX, p);
    }

  SPEC_NOUN (val->type) = V_INT;

  if (u_suffix) // Choose first of unsigned int, unsigned long int, unsigned long long int that fits.
    {
      SPEC_USIGN (val->type) = 1;
      if (ll_suffix || dval > 0xffffffff)
        SPEC_LONGLONG (val->type) = 1;
      else if(l_suffix || dval > 0xffff)
        SPEC_LONG (val->type) = 1;
    }
  else
    {  
      if (decimal) // Choose first of int, long int, long long int that fits.
        {
          if (ll_suffix || dval > 0x7fffffff || dval < -0x80000000ll)
            {
              if (!options.std_c99) // C90 exception: Use unsigned long
                {
                  SPEC_USIGN (val->type) = 1;
                  SPEC_LONG (val->type) = 1;
                }
              else
                SPEC_LONGLONG (val->type) = 1;
            }
          else if(l_suffix || dval > 0x7fff || dval < -0x8000l)
            SPEC_LONG (val->type) = 1;
        }
      else // Choose first of int, unsigned int, long int, unsigned long int, long long int, unsigned long long int that fits.
       {
         if (dval > 0x7fffffffffffffff)
           {
             SPEC_USIGN (val->type) = 1;
             SPEC_LONGLONG (val->type) = 1;
           }
         else if (ll_suffix || dval > 0xffffffff || dval < -0x80000000ll)
           {
             SPEC_LONGLONG (val->type) = 1;
           }
         else if (dval > 0x7fffffff)
           {
             SPEC_USIGN (val->type) = 1;
             SPEC_LONG (val->type) = 1;
           }
         else if (l_suffix || dval > 0xffff || dval < -0x8000l)
           {
             SPEC_LONG (val->type) = 1;
           }
         else if (dval > 0x7fff)
           {
             SPEC_USIGN (val->type) = 1;
           }
       }
    }

  /* check for out of range */
  if (!SPEC_LONGLONG (val->type))
    {
      if (dval < -2147483648.0)
        {
          dval = -2147483648.0;
          werror (W_INVALID_INT_CONST, s, dval);
        }
      if (dval > 2147483648.0 && !SPEC_USIGN (val->type))
        {
          dval = 2147483647.0;
          werror (W_INVALID_INT_CONST, s, dval);
        }
      if (dval > 4294967295.0)
        {
          dval = 4294967295.0;
          werror (W_INVALID_INT_CONST, s, dval);
        }
    }

  if (SPEC_LONGLONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        {
          SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) llval;
        }
      else
        {
          SPEC_CVAL (val->type).v_longlong = (TYPE_TARGET_LONGLONG) llval;
        }
    }
  else if (SPEC_LONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        {
          SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) double2ul (dval);
        }
      else
        {
          SPEC_CVAL (val->type).v_long = (TYPE_TARGET_LONG) double2ul (dval);
        }
    }
  else
    {
      if (SPEC_USIGN (val->type))
        {
          SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) double2ul (dval);
        }
      else
        {
          SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) double2ul (dval);
        }
    }

  return val;
}

/*-----------------------------------------------------------------*/
/* constCharacterVal - converts a character constant to value      */
/*-----------------------------------------------------------------*/
value *
constCharacterVal (unsigned long v, char type)
{
  value *val = newValue ();     /* alloc space for value   */

  val->type = val->etype = newLink (SPECIFIER); /* create the specifier */
  SPEC_SCLS (val->type) = S_LITERAL;
  SPEC_CONST (val->type) = 1;

  switch (type)
    {
    case 0: // character constant
      SPEC_NOUN (val->type) = V_INT;
      SPEC_USIGN (val->type) = 0;
      SPEC_CVAL (val->type).v_int = options.signed_char ? (signed char) v : (unsigned char) v;
      break;
    case 'L': // wide character constant
      if (!options.std_c95)
        werror (E_WCHAR_CONST_C95);
      SPEC_NOUN (val->type) = V_INT;
      SPEC_USIGN (val->type) = 1;
      SPEC_LONG (val->etype) = 1;
      SPEC_CVAL (val->type).v_ulong = (TYPE_UDWORD) v;
      break;
    case 'u': // wide character constant
      if (!options.std_c11)
        werror (E_WCHAR_CONST_C11);
      SPEC_NOUN (val->type) = V_INT;
      SPEC_USIGN (val->type) = 1;
      SPEC_CVAL (val->type).v_uint = (TYPE_UWORD) v;
      break;
    case 'U': // wide character constant
      if (!options.std_c11)
        werror (E_WCHAR_CONST_C11);
      SPEC_NOUN (val->type) = V_INT;
      SPEC_USIGN (val->type) = 1;
      SPEC_LONG (val->etype) = 1;
      SPEC_CVAL (val->type).v_ulong = (TYPE_UDWORD) v;
      break;
    case '8':
      if (!options.std_c2x)
        werror (E_U8_CHAR_C2X);
      if (v >= 128)
        werror (E_U8_CHAR_INVALID);
      SPEC_NOUN (val->type) = V_CHAR;
      SPEC_USIGN (val->type) = 1;
      SPEC_CVAL (val->type).v_int = (unsigned char) v;
      break;
    default:
      wassert (0);
    }

  return val;
}

/*-----------------------------------------------------------------*/
/* constCharVal - converts a character constant to value           */
/*-----------------------------------------------------------------*/
value *
constCharVal (unsigned char v)
{
  return constCharacterVal (v, 0);
}

/*-----------------------------------------------------------------*/
/* constBoolVal - converts a BOOL constant to value                */
/*-----------------------------------------------------------------*/
value *
constBoolVal (bool v)
{
  value *val = newValue ();     /* alloc space for value   */

  val->type = val->etype = newLink (SPECIFIER); /* create the specifier */
  SPEC_SCLS (val->type) = S_LITERAL;
  SPEC_CONST (val->type) = 1;

  SPEC_NOUN (val->type) = (bit) ? V_BIT : V_BOOL;

  SPEC_CVAL (val->type).v_uint = (unsigned int) v;

  return val;
}

// TODO: Move this function to SDCCutil?
static const TYPE_UDWORD *utf_32_from_utf_8 (size_t *utf_32_len, const char *utf_8, size_t utf_8_len)
{
  size_t allocated = 0;
  TYPE_UDWORD *utf_32 = 0;
  unsigned char first_byte;
  TYPE_UDWORD codepoint;
  size_t seqlen;

  for (*utf_32_len = 0; utf_8_len; (*utf_32_len)++)
    {
      if (allocated == *utf_32_len)
        {
          utf_32 = realloc (utf_32, sizeof(TYPE_UDWORD) * (*utf_32_len + 16));
          wassert (utf_32);
          allocated = *utf_32_len + 16;
        }

      first_byte = *utf_8;
      seqlen = 1;
      if (first_byte & 0x80)
        {
          while (first_byte & (0x80 >> seqlen))
            seqlen++;
          first_byte &= (0xff >> (seqlen + 1));
        }
      wassert (seqlen <= 6); // seqlen 5 and 6 are deprecated in current unicode standard, but for now, allow them.

      codepoint = first_byte;
      utf_8++;
      utf_8_len--;
      seqlen--;

      for(; seqlen; seqlen--)
        {
          codepoint <<= 6;
          codepoint |= (*utf_8 & 0x3f);
          utf_8++;
          utf_8_len--;
        }

      utf_32[*utf_32_len] = codepoint;
    }
  return (utf_32);
}

// TODO: Move this function to SDCCutil?
static const TYPE_UWORD *utf_16_from_utf_32 (size_t *utf_16_len, const TYPE_UDWORD *utf_32, size_t utf_32_len)
{
  size_t allocated = 0;
  TYPE_UWORD *utf_16 = 0;
  TYPE_UDWORD codepoint;

  for (*utf_16_len = 0; utf_32_len; utf_32_len--, utf_32++)
    {
      if (allocated <= *utf_16_len + 2)
        {
          utf_16 = realloc (utf_16, sizeof(TYPE_UWORD) * (*utf_16_len + 16));
          wassert (utf_16);
          allocated = *utf_16_len + 16;
        }

      codepoint = *utf_32;

      if (codepoint < 0xd7ff || codepoint >= 0xe000 && codepoint <= 0xffff) // Code in basic multilingual plane.
        {
          utf_16[(*utf_16_len)++] = codepoint;
          continue;
        }

      // Code point in supplementary plane.
      wassert (codepoint >= 0x100000 && codepoint <= 0x10ffff);
      codepoint -= 0x100000;

      utf_16[(*utf_16_len)++] = ((codepoint >> 10) & 0x3ff) + 0xd800;
      utf_16[(*utf_16_len)++] = (codepoint & 0x3ff) + 0xdc00;
    }

  return (utf_16);
}

/*------------------------------------------------------------------*/
/* strVal - converts a string constant to a value                   */
/*------------------------------------------------------------------*/
value *
strVal (const char *s)
{
  value *val;
  const char *utf_8;
  size_t utf_8_size;

  val = newValue ();

  /* get a declarator */
  val->type = newLink (DECLARATOR);
  DCL_TYPE (val->type) = ARRAY;
  val->type->next = val->etype = newLink (SPECIFIER);
  SPEC_SCLS (val->etype) = S_LITERAL;
  SPEC_CONST (val->etype) = 1;

  // Convert input string (mixed UTF-8 and UTF-32) to UTF-8 first (handling all escape sequences, etc).
  utf_8 = copyStr (s[0] == '"' ? s : s + 1, &utf_8_size);

  if (s[0] == '"') // UTF-8 string literal (any prefix u8 or L in the source would already have been stripped by earlier stages)
    {
      SPEC_NOUN (val->etype) = V_CHAR;
      SPEC_USIGN (val->etype) = !options.signed_char;
      val->etype->select.s.b_implicit_sign = true;
      SPEC_CVAL (val->etype).v_char = utf_8;
      DCL_ELEM (val->type) = utf_8_size;
    }
  else
    {
      size_t utf_32_size;
      // Convert to UTF-32 next, since converting UTF-32 to UTF-16 is easier than UTF-8 to UTF-16.
      const TYPE_UDWORD *utf_32 = utf_32_from_utf_8 (&utf_32_size, utf_8, utf_8_size);

      dbuf_free (utf_8);

      if (s[0] == 'U' || s[0] == 'L') // UTF-32 string literal
        {
          SPEC_NOUN (val->etype) = V_INT;
          SPEC_USIGN (val->etype) = 1;
          SPEC_LONG (val->etype) = 1;
          SPEC_CVAL (val->etype).v_char32 = utf_32;
          DCL_ELEM (val->type) = utf_32_size;
        }
      else if (s[0] == 'u') // UTF-16 string literal
        {
          size_t utf_16_size;
          const TYPE_UWORD *utf_16 = utf_16_from_utf_32 (&utf_16_size, utf_32, utf_32_size);

          SPEC_NOUN (val->etype) = V_INT;
          SPEC_USIGN (val->etype) = 1;
          SPEC_CVAL (val->etype).v_char16 = utf_16;
          DCL_ELEM (val->type) = utf_16_size;
        }
      else
        wassert (0);
    }

  return (val);
}

/*------------------------------------------------------------------*/
/* rawStrVal - converts a string to a value                         */
/*------------------------------------------------------------------*/
value *
rawStrVal (const char *s, size_t size)
{
  struct dbuf_s dbuf;
  value *val = newValue ();
  
  dbuf_init (&dbuf, size);
  wassert (dbuf_append (&dbuf, s, size));

  /* get a declarator */
  val->type = newLink (DECLARATOR);
  DCL_TYPE (val->type) = ARRAY;
  val->type->next = val->etype = newLink (SPECIFIER);
  SPEC_SCLS (val->etype) = S_LITERAL;
  SPEC_CONST (val->etype) = 1;

  SPEC_NOUN (val->etype) = V_CHAR;
  SPEC_USIGN (val->etype) = !options.signed_char;
  val->etype->select.s.b_implicit_sign = true;
  SPEC_CVAL (val->etype).v_char = dbuf_detach (&dbuf);
  DCL_ELEM (val->type) = size;

  return (val);
}

/*------------------------------------------------------------------*/
/* reverseValWithType - reverses value chain with type & etype      */
/*------------------------------------------------------------------*/
value *
reverseValWithType (value * val)
{
  sym_link *type;
  sym_link *etype;

  if (!val)
    return NULL;

  /* save the type * etype chains */
  type = val->type;
  etype = val->etype;

  /* set the current one 2b null */
  val->type = val->etype = NULL;
  val = reverseVal (val);

  /* restore type & etype */
  val->type = type;
  val->etype = etype;

  return val;
}

/*------------------------------------------------------------------*/
/* reverseVal - reverses the values for a value chain               */
/*------------------------------------------------------------------*/
value *
reverseVal (value * val)
{
  value *prev, *curr, *next;

  if (!val)
    return NULL;

  prev = val;
  curr = val->next;

  while (curr)
    {
      next = curr->next;
      curr->next = prev;
      prev = curr;
      curr = next;
    }
  val->next = (void *) NULL;
  return prev;
}

/*------------------------------------------------------------------*/
/* copyValueChain - will copy a chain of values                     */
/*------------------------------------------------------------------*/
value *
copyValueChain (value * src)
{
  value *dest;

  if (!src)
    return NULL;

  dest = copyValue (src);
  dest->next = copyValueChain (src->next);

  return dest;
}

/*------------------------------------------------------------------*/
/* copyValue - copies contents of a value to a fresh one            */
/*------------------------------------------------------------------*/
value *
copyValue (value * src)
{
  value *dest;

  dest = newValue ();
  dest->sym = copySymbol (src->sym);
  strncpyz (dest->name, src->name, SDCC_NAME_MAX);
  dest->type = (src->type ? copyLinkChain (src->type) : NULL);
  dest->etype = (src->type ? getSpec (dest->type) : NULL);

  return dest;
}

/*------------------------------------------------------------------*/
/* charVal - converts a character constant to a value               */
/*------------------------------------------------------------------*/
value *
charVal (const char *s)
{
  char type;

  if ((s[0] == 'L' || s[0] == 'u' || s[0] == 'U') && s[1] == '\'')
    type = *s++;
  else if (s[0] == 'u' && s[1] == '8' && s[2] == '\'')
    {
      if (s[4] != '\'')
        werror (E_U8_CHAR_INVALID);
      type = '8';
      s += 2;
    }
  else
    type = 0;

  s++; // Get rid of quotation.

  /* if \ then special processing */
  if (*s == '\\')
    {
      switch (*++s)             /* go beyond the backslash  */
        {
        case 'n':
          return constCharacterVal ('\n', type);
        case 't':
          return constCharacterVal ('\t', type);
        case 'v':
          return constCharacterVal ('\v', type);
        case 'b':
          return constCharacterVal ('\b', type);
        case 'r':
          return constCharacterVal ('\r', type);
        case 'f':
          return constCharacterVal ('\f', type);
        case 'a':
          return constCharacterVal ('\a', type);
        case '\\':
          return constCharacterVal ('\\', type);
        case '\?':
          return constCharacterVal ('\?', type);
        case '\'':
          return constCharacterVal ('\'', type);
        case '\"':
          return constCharacterVal ('\"', type);

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
          return constCharacterVal (octalEscape (&s), type);

        case 'x':
          return constCharacterVal (hexEscape (&s), type);

        case 'u':
          return constCharacterVal (universalEscape (&s, 4), type);

        case 'U':
          return constCharacterVal (universalEscape (&s, 8), type);

        default:
          return constCharacterVal (*s, type);
        }
    }
  else if (type) // Wide character constant
    {
      size_t ulen;
      const TYPE_UDWORD *ustr = utf_32_from_utf_8 (&ulen, s, strlen(s) - 1);
      value *val = constCharacterVal (*ustr, type);
      free ((void *)ustr);
      return (val);
    }
  else // Character constant that is not wide - compability with legacy encodings.
    return constCharacterVal (*s, 0);
}

/*------------------------------------------------------------------*/
/* valFromType - creates a value from type given                    */
/*------------------------------------------------------------------*/
value *
valFromType (sym_link * type)
{
  value *val = newValue ();
  val->type = copyLinkChain (type);
  val->etype = getSpec (val->type);
  return val;
}

/*------------------------------------------------------------------*/
/* floatFromVal - value to double float conversion                  */
/*------------------------------------------------------------------*/
double
floatFromVal (value * val)
{
  if (!val)
    return 0;

  if (val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
    {
      werror (E_CONST_EXPECTED, val->name);
      return 0;
    }

  /* if it is not a specifier then we can assume that */
  /* it will be an unsigned long                      */
  if (!IS_SPEC (val->type))
    return SPEC_CVAL (val->etype).v_ulong;

  if (SPEC_NOUN (val->etype) == V_FLOAT)
    return SPEC_CVAL (val->etype).v_float;

  if (SPEC_NOUN (val->etype) == V_FIXED16X16)
    return doubleFromFixed16x16 (SPEC_CVAL (val->etype).v_fixed16x16);

  if (SPEC_LONGLONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        return (double)SPEC_CVAL (val->etype).v_ulonglong;
      else
        return (double)SPEC_CVAL (val->etype).v_longlong;
    }

  if (SPEC_LONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        return SPEC_CVAL (val->etype).v_ulong;
      else
        return SPEC_CVAL (val->etype).v_long;
    }

  if (SPEC_NOUN (val->etype) == V_INT)
    {
      if (SPEC_USIGN (val->etype))
        return SPEC_CVAL (val->etype).v_uint;
      else
        return SPEC_CVAL (val->etype).v_int;
    }

  if (SPEC_NOUN (val->etype) == V_CHAR)
    {
      if (SPEC_USIGN (val->etype))
        return (unsigned char) SPEC_CVAL (val->etype).v_uint;
      else
        return (signed char) SPEC_CVAL (val->etype).v_int;
    }

  if (IS_BOOL (val->etype) || IS_BITVAR (val->etype))
    return SPEC_CVAL (val->etype).v_uint;

  if (SPEC_NOUN (val->etype) == V_VOID)
    return SPEC_CVAL (val->etype).v_ulong;

  if (SPEC_NOUN (val->etype) == V_STRUCT)
    return SPEC_CVAL (val->etype).v_ulong;

  /* we are lost ! */
  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "floatFromVal: unknown value");
  return 0;
}

/*------------------------------------------------------------------*/
/* ulFromVal - value to unsigned long conversion                    */
/*------------------------------------------------------------------*/
unsigned long
ulFromVal (value *val)
{
  if (!val)
    return 0;

  if (val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
    {
      werror (E_CONST_EXPECTED, val->name);
      return 0;
    }

  /* if it is not a specifier then we can assume that */
  /* it will be an unsigned long                      */
  if (!IS_SPEC (val->type))
    return SPEC_CVAL (val->etype).v_ulong;

  if (SPEC_NOUN (val->etype) == V_FLOAT)
    return double2ul (SPEC_CVAL (val->etype).v_float);

  if (SPEC_NOUN (val->etype) == V_FIXED16X16)
    return double2ul (doubleFromFixed16x16 (SPEC_CVAL (val->etype).v_fixed16x16));

  if (SPEC_LONGLONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        return (unsigned long)SPEC_CVAL (val->etype).v_ulonglong;
      else
        return (unsigned long)SPEC_CVAL (val->etype).v_longlong;
    }

  if (SPEC_LONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        return SPEC_CVAL (val->etype).v_ulong;
      else
        return SPEC_CVAL (val->etype).v_long;
    }

  if (SPEC_NOUN (val->etype) == V_INT)
    {
      if (SPEC_USIGN (val->etype))
        return SPEC_CVAL (val->etype).v_uint;
      else
        return SPEC_CVAL (val->etype).v_int;
    }

  if (SPEC_NOUN (val->etype) == V_CHAR)
    {
      if (SPEC_USIGN (val->etype))
        return (unsigned char) SPEC_CVAL (val->etype).v_uint;
      else
        return (signed char) SPEC_CVAL (val->etype).v_int;
    }

  if (IS_BOOL (val->etype) || IS_BITVAR (val->etype))
    return SPEC_CVAL (val->etype).v_uint;

  if (SPEC_NOUN (val->etype) == V_VOID)
    return SPEC_CVAL (val->etype).v_ulong;

  if (SPEC_NOUN (val->etype) == V_STRUCT)
    return SPEC_CVAL (val->etype).v_ulong;

  /* we are lost ! */
  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "ulFromVal: unknown value");
  return 0;
}

/*------------------------------------------------------------------*/
/* byteOfVal - extract a byte of a value                            */
/*             offset = 0 (LSB) ... n-1 (MSB)                       */
/*             higher offsets of signed ints will be sign extended, */
/*             other types will be extended with zero padding       */
/*------------------------------------------------------------------*/
unsigned char
byteOfVal (value * val, int offset)
{
  unsigned char *p;
  int shift = 8*offset;

  wassert (offset >= 0);

  if (!val)
    return 0;

  if (val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
    {
      werror (E_CONST_EXPECTED, val->name);
      return 0;
    }
 
  /* if it is not a specifier then we can assume that */
  /* it will be an unsigned long                      */
  /* 2012-Apr-30 EEP - Why is this true?              */
  if (!IS_SPEC (val->type))
    return offset < 4 ? (SPEC_CVAL (val->etype).v_ulong >> shift) & 0xff : 0;

  if (SPEC_NOUN (val->etype) == V_FLOAT)
    {
      float f = (float)SPEC_CVAL (val->etype).v_float;

      if (offset > 3)
        return 0;
      p = (unsigned char *)&f;
#ifdef WORDS_BIGENDIAN
      p += 3 - offset;
#else
      p += offset;
#endif
      return *p;      
    }

  if (SPEC_NOUN (val->etype) == V_FIXED16X16)
    return offset < 4 ? (SPEC_CVAL (val->etype).v_fixed16x16 >> shift) & 0xff : 0;

  if (SPEC_LONGLONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        return offset < 8 ? (SPEC_CVAL (val->etype).v_ulonglong >> shift) & 0xff : 0;
      else
        return offset < 8 ? (SPEC_CVAL (val->etype).v_longlong >> shift) & 0xff : 
               (SPEC_CVAL (val->etype).v_longlong < 0 ? 0xff : 0);
    }

  if (SPEC_LONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        return offset < 4 ? (SPEC_CVAL (val->etype).v_ulong >> shift) & 0xff : 0;
      else
        return offset < 4 ? (SPEC_CVAL (val->etype).v_long >> shift) & 0xff : 
               (SPEC_CVAL (val->etype).v_long < 0 ? 0xff : 0);
    }

  if (SPEC_NOUN (val->etype) == V_INT)
    {
      if (SPEC_USIGN (val->etype))
        return offset < 2 ? (SPEC_CVAL (val->etype).v_uint >> shift) & 0xff : 0;
      else
        return offset < 2 ? (SPEC_CVAL (val->etype).v_int >> shift) & 0xff :
               (SPEC_CVAL (val->etype).v_int < 0 ? 0xff : 0);
    }

  if (SPEC_NOUN (val->etype) == V_CHAR)
    {
      if (SPEC_USIGN (val->etype))
        return offset < 1 ? SPEC_CVAL (val->etype).v_uint & 0xff : 0;
      else
        return offset < 1 ? SPEC_CVAL (val->etype).v_int & 0xff :
               (SPEC_CVAL (val->etype).v_int < 0 ? 0xff : 0);
    }

  if (IS_BOOL (val->etype) || IS_BITVAR (val->etype))
    return offset < 2 ? (SPEC_CVAL (val->etype).v_uint >> shift) & 0xff : 0;

  /* we are lost ! */
  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "byteOfVal: unknown value");
  return 0;
}

/*------------------------------------------------------------------*/
/* ullFromLit - literal to unsigned long long conversion            */
/*------------------------------------------------------------------*/
TYPE_TARGET_ULONGLONG
ullFromLit (sym_link * lit)
{
  sym_link * etype = getSpec(lit);

  if (!lit)
    return 0;

  if (etype && SPEC_SCLS (etype) != S_LITERAL)
    {
      werror (E_CONST_EXPECTED, "");
      return 0;
    }

  /* if it is not a specifier then we can assume that */
  /* it will be an unsigned long                      */
  if (!IS_SPEC (lit))
    return SPEC_CVAL (etype).v_ulong;

  if (SPEC_NOUN (etype) == V_FLOAT)
    return double2ul (SPEC_CVAL (etype).v_float); /* FIXME: this loses bits */

  if (SPEC_NOUN (etype) == V_FIXED16X16)
    return double2ul (doubleFromFixed16x16 (SPEC_CVAL (etype).v_fixed16x16)); /* FIXME: this loses bits */

  if (SPEC_LONGLONG (etype))
    {
      if (SPEC_USIGN (etype))
        return SPEC_CVAL (etype).v_ulonglong;
      else
        return SPEC_CVAL (etype).v_longlong;
    }

  if (SPEC_LONG (etype))
    {
      if (SPEC_USIGN (etype))
        return SPEC_CVAL (etype).v_ulong;
      else
        return SPEC_CVAL (etype).v_long;
    }

  if (SPEC_NOUN (etype) == V_INT)
    {
      if (SPEC_USIGN (etype))
        return SPEC_CVAL (etype).v_uint;
      else
        return SPEC_CVAL (etype).v_int;
    }

  if (SPEC_NOUN (etype) == V_CHAR)
    {
      if (SPEC_USIGN (etype))
        return (unsigned char) SPEC_CVAL (etype).v_uint;
      else
        return (signed char) SPEC_CVAL (etype).v_int;
    }

  if (IS_BOOL (etype) || IS_BITVAR (etype))
    return SPEC_CVAL (etype).v_uint;

  if (SPEC_NOUN (etype) == V_VOID)
    return SPEC_CVAL (etype).v_ulong;

  if (SPEC_NOUN (etype) == V_STRUCT)  /* ??? Why ??? EEP - 23 Nov 2012 */
    return SPEC_CVAL (etype).v_ulong;

  /* we are lost ! */
  werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "ullFromLit: unknown value");
  return 0;
}

/*------------------------------------------------------------------*/
/* ullFromVal - value to unsigned long long conversion              */
/*------------------------------------------------------------------*/
unsigned long long
ullFromVal (value * val)
{
  if (!val)
    return 0;

  if (val->etype && SPEC_SCLS (val->etype) != S_LITERAL)
    {
      werror (E_CONST_EXPECTED, val->name);
      return 0;
    }
  return (unsigned long long) ullFromLit (val->type);
}

/*------------------------------------------------------------------*/
/* csdOfVal - return 0 if the value can be represented as csd       */
/* topbit  - highest nonzero bit in csd                             */
/* nonzero - number of nonzero bits in csd                          */
/* csd_add - positive bits in csd                                   */
/* csd_sub - negative bits in csd                                   */
/*------------------------------------------------------------------*/
int csdOfVal (int *topbit, int *nonzero, unsigned long long *csd_add, unsigned long long *csd_sub, value *val)
{
  unsigned long long binary = ullFromVal (val);
  bool gamma, theta, a;
  int bit, next;

  *topbit = 0;
  *nonzero = 0;
  *csd_add = 0;
  *csd_sub = 0;

  for (a = 0, gamma = 0, bit = 0; bit < 61; bit++)
    {
       theta = a ^ (binary & 1);
       gamma = !gamma && theta;
       next = (1 - 2 * (bool)(binary & 2)) * gamma;
       if (next > 0)
         *csd_add |= (1ull << bit);
       else if (next < 0)
         *csd_sub |= (1ull << bit);
       if (next)
         {
           (*nonzero)++;
           *topbit = bit;
         }
       a = (binary & 1);
       binary >>= 1;
    }
  return((bool)binary);
}

/*------------------------------------------------------------------*/
/* isEqualVal - return 1 if value is equal to specified constant    */
/*------------------------------------------------------------------*/
int
isEqualVal (value * val, int k)
{
  if (IS_SPEC (val->type))
    {
      if (SPEC_NOUN (val->type) == V_FLOAT || SPEC_NOUN (val->type) == V_FIXED16X16)
        return floatFromVal (val) == k;
    }
  return ((TYPE_TARGET_LONGLONG) ullFromVal (val)) == k;
}


/*-----------------------------------------------------------------*/
/* doubleFromFixed16x16 - convert a fixed16x16 to double           */
/*-----------------------------------------------------------------*/
double
doubleFromFixed16x16 (TYPE_TARGET_ULONG value)
{
#if 0
  /* This version is incorrect negative values. */
  double tmp = 0, exp = 2;

  tmp = (value & 0xffff0000) >> 16;

  while (value)
    {
      value &= 0xffff;
      if (value & 0x8000)
        tmp += 1 / exp;
      exp *= 2;
      value <<= 1;
    }

  return (tmp);
#else
  return ((double) (value * 1.0) / (double) (1UL << 16));
#endif
}

TYPE_TARGET_ULONG
fixed16x16FromDouble (double value)
{
#if 0
  /* This version is incorrect negative values. */
  unsigned int tmp = 0, pos = 16;
  TYPE_TARGET_ULONG res;

  tmp = floor (value);
  res = tmp << 16;
  value -= tmp;

  tmp = 0;
  while (pos--)
    {
      value *= 2;
      if (value >= 1.0)
        tmp |= (1 << pos);
      value -= floor (value);
    }

  res |= tmp;

  return (res);
#else
  return double2ul (value * (double) (1UL << 16));
#endif
}

/*------------------------------------------------------------------*/
/* valUnaryPM - does the unary +/- operation on a constant          */
/*------------------------------------------------------------------*/
value *
valUnaryPM (value * val)
{
  /* depending on type */
  if (SPEC_NOUN (val->etype) == V_FLOAT)
    SPEC_CVAL (val->etype).v_float = -1.0 * SPEC_CVAL (val->etype).v_float;
  else if (SPEC_NOUN (val->etype) == V_FIXED16X16)
    SPEC_CVAL (val->etype).v_fixed16x16 = (TYPE_TARGET_ULONG) - ((long) SPEC_CVAL (val->etype).v_fixed16x16);
  else if (SPEC_LONGLONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_ulonglong = 0 - SPEC_CVAL (val->etype).v_ulonglong;
      else
        SPEC_CVAL (val->etype).v_longlong = -SPEC_CVAL (val->etype).v_longlong;
    }
  else if (SPEC_LONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_ulong = 0 - SPEC_CVAL (val->etype).v_ulong;
      else
        SPEC_CVAL (val->etype).v_long = -SPEC_CVAL (val->etype).v_long;
    }
  else
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_uint = 0 - SPEC_CVAL (val->etype).v_uint;
      else
        SPEC_CVAL (val->etype).v_int = -SPEC_CVAL (val->etype).v_int;

      if (SPEC_NOUN (val->etype) == V_CHAR)
        {
          /* promote to 'signed int', cheapestVal() might reduce it again */
          SPEC_USIGN (val->etype) = 0;
          SPEC_NOUN (val->etype) = V_INT;
        }
      return cheapestVal (val);
    }
  return val;
}

/*------------------------------------------------------------------*/
/* valueComplement - complements a constant                         */
/*------------------------------------------------------------------*/
value *
valComplement (value * val)
{
  /* depending on type */
  if (SPEC_LONGLONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_ulonglong = ~SPEC_CVAL (val->etype).v_ulonglong;
      else
        SPEC_CVAL (val->etype).v_longlong = ~SPEC_CVAL (val->etype).v_longlong;
    }
  else if (SPEC_LONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_ulong = ~SPEC_CVAL (val->etype).v_ulong;
      else
        SPEC_CVAL (val->etype).v_long = ~SPEC_CVAL (val->etype).v_long;
    }
  else
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_uint = ~SPEC_CVAL (val->etype).v_uint;
      else
        SPEC_CVAL (val->etype).v_int = ~SPEC_CVAL (val->etype).v_int;

      if (SPEC_NOUN (val->etype) == V_CHAR)
        {
          /* promote to 'signed int', cheapestVal() might reduce it again */
          SPEC_USIGN (val->etype) = 0;
          SPEC_NOUN (val->etype) = V_INT;
        }
      return cheapestVal (val);
    }
  return val;
}

/*------------------------------------------------------------------*/
/* valueNot - complements a constant                                */
/*------------------------------------------------------------------*/
value *
valNot (value * val)
{
  /* depending on type */
  if (SPEC_LONGLONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_int = !SPEC_CVAL (val->etype).v_ulonglong;
      else
        SPEC_CVAL (val->etype).v_int = !SPEC_CVAL (val->etype).v_longlong;
    }
  else if (SPEC_LONG (val->etype))
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_int = !SPEC_CVAL (val->etype).v_ulong;
      else
        SPEC_CVAL (val->etype).v_int = !SPEC_CVAL (val->etype).v_long;
    }
  else
    {
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_int = !SPEC_CVAL (val->etype).v_uint;
      else
        SPEC_CVAL (val->etype).v_int = !SPEC_CVAL (val->etype).v_int;

    }
  /* ANSI: result type is int, value is 0 or 1 */
  /* sdcc will hold this in an 'unsigned char' */
  SPEC_USIGN (val->etype) = 1;
  SPEC_LONG (val->etype) = 0;
  SPEC_LONGLONG (val->type) = 0;
  SPEC_NOUN (val->etype) = V_CHAR;
  return val;
}

/*------------------------------------------------------------------*/
/* valMult - multiply constants                                     */
/*------------------------------------------------------------------*/
value *
valMult (value * lval, value * rval)
{
  value *val;

  /* create a new value */
  val = newValue ();
  val->type = val->etype = computeType (lval->etype, rval->etype, RESULT_TYPE_INT, '*');
  SPEC_SCLS (val->etype) = S_LITERAL;   /* will remain literal */

  if (IS_FLOAT (val->type))
    SPEC_CVAL (val->type).v_float = floatFromVal (lval) * floatFromVal (rval);
  else if (IS_FIXED16X16 (val->type))
    SPEC_CVAL (val->type).v_fixed16x16 = fixed16x16FromDouble (floatFromVal (lval) * floatFromVal (rval));
  /* signed and unsigned mul are the same, as long as the precision of the
     result isn't bigger than the precision of the operands. */
  else if (SPEC_LONGLONG (val->type))
    SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) ullFromVal (lval) * (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
  else if (SPEC_LONG (val->type))
    SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) * (TYPE_TARGET_ULONG) ulFromVal (rval);
  else if (SPEC_USIGN (val->type))      /* unsigned int */
    {
      TYPE_TARGET_ULONG ul = (TYPE_TARGET_UINT) ulFromVal (lval) * (TYPE_TARGET_UINT) ulFromVal (rval);

      SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) ul;
      if (ul != (TYPE_TARGET_UINT) ul)
        werror (W_INT_OVL);
    }
  else                          /* signed int */
    {
      TYPE_TARGET_LONG l = (TYPE_TARGET_INT) floatFromVal (lval) * (TYPE_TARGET_INT) floatFromVal (rval);

      SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) l;
      if (l != (TYPE_TARGET_INT) l)
        werror (W_INT_OVL);
    }
  return cheapestVal (val);
}

/*------------------------------------------------------------------*/
/* valDiv  - Divide   constants                                     */
/*------------------------------------------------------------------*/
value *
valDiv (value * lval, value * rval)
{
  value *val;

  if (isEqualVal (rval, 0) && !IS_FLOAT (computeType (lval->etype, rval->etype, RESULT_TYPE_INT, '/')))
    {
      werror (E_DIVIDE_BY_ZERO);
      return rval;
    }

  /* create a new value */
  val = newValue ();
  val->type = val->etype = computeType (lval->etype, rval->etype, RESULT_TYPE_INT, '/');
  SPEC_SCLS (val->etype) = S_LITERAL;   /* will remain literal */

  if (IS_FLOAT (val->type))
    SPEC_CVAL (val->type).v_float = floatFromVal (lval) / floatFromVal (rval);
  else if (IS_FIXED16X16 (val->type))
    SPEC_CVAL (val->type).v_fixed16x16 = fixed16x16FromDouble (floatFromVal (lval) / floatFromVal (rval));
  else if (SPEC_LONGLONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) ullFromVal (lval) / (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
      else
        SPEC_CVAL (val->type).v_longlong = (TYPE_TARGET_LONGLONG) ullFromVal (lval) / (TYPE_TARGET_LONGLONG) ullFromVal (rval);
    }
  else if (SPEC_LONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) / (TYPE_TARGET_ULONG) ulFromVal (rval);
      else
        SPEC_CVAL (val->type).v_long = (TYPE_TARGET_LONG) ulFromVal (lval) / (TYPE_TARGET_LONG) ulFromVal (rval);
    }
  else
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) ulFromVal (lval) / (TYPE_TARGET_UINT) ulFromVal (rval);
      else
        SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) ulFromVal (lval) / (TYPE_TARGET_INT) ulFromVal (rval);
    }
  return cheapestVal (val);
}

/*------------------------------------------------------------------*/
/* valMod  - Modulus  constants                                     */
/*------------------------------------------------------------------*/
value *
valMod (value * lval, value * rval)
{
  value *val;

  if (isEqualVal (rval, 0))
    {
      werror (E_DIVIDE_BY_ZERO);
      return rval;
    }

  /* create a new value */
  val = newValue ();
  val->type = val->etype = computeType (lval->etype, rval->etype, RESULT_TYPE_INT, '%');
  SPEC_SCLS (val->etype) = S_LITERAL;   /* will remain literal */

  if (SPEC_LONGLONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) ullFromVal (lval) % (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
      else
        SPEC_CVAL (val->type).v_longlong = (TYPE_TARGET_LONGLONG) ullFromVal (lval) % (TYPE_TARGET_LONGLONG) ullFromVal (rval);
    }
  else if (SPEC_LONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) % (TYPE_TARGET_ULONG) ulFromVal (rval);
      else
        SPEC_CVAL (val->type).v_long = (TYPE_TARGET_LONG) ulFromVal (lval) % (TYPE_TARGET_LONG) ulFromVal (rval);
    }
  else
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) ulFromVal (lval) % (TYPE_TARGET_UINT) ulFromVal (rval);
      else
        SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) ulFromVal (lval) % (TYPE_TARGET_INT) ulFromVal (rval);
    }
  return cheapestVal (val);
}

/*------------------------------------------------------------------*/
/* valPlus - Addition constants                                     */
/*------------------------------------------------------------------*/
value *
valPlus (value * lval, value * rval)
{
  value *val;

  /* create a new value */
  val = newValue ();
  val->type = computeType (lval->type, rval->type, RESULT_TYPE_INT, '+');
  val->etype = getSpec (val->type);
  SPEC_SCLS (val->etype) = S_LITERAL;   /* will remain literal */

  if (!IS_SPEC (val->type))
    SPEC_CVAL (val->etype).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) + (TYPE_TARGET_ULONG) ulFromVal (rval);
  else if (IS_FLOAT (val->type))
    SPEC_CVAL (val->type).v_float = floatFromVal (lval) + floatFromVal (rval);
  else if (IS_FIXED16X16 (val->type))
    SPEC_CVAL (val->type).v_fixed16x16 = fixed16x16FromDouble (floatFromVal (lval) + floatFromVal (rval));
  else if (SPEC_LONGLONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) ullFromVal (lval) + (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
      else
        SPEC_CVAL (val->type).v_longlong = (TYPE_TARGET_LONGLONG) ullFromVal (lval) + (TYPE_TARGET_LONGLONG) ullFromVal (rval);
    }
  else if (SPEC_LONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) + (TYPE_TARGET_ULONG) ulFromVal (rval);
      else
        SPEC_CVAL (val->type).v_long = (TYPE_TARGET_LONG) ulFromVal (lval) + (TYPE_TARGET_LONG) ulFromVal (rval);
    }
  else
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) ulFromVal (lval) + (TYPE_TARGET_UINT) ulFromVal (rval);
      else
        SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) ulFromVal (lval) + (TYPE_TARGET_INT) ulFromVal (rval);
    }
  return cheapestVal (val);
}

/*------------------------------------------------------------------*/
/* valMinus - Addition constants                                    */
/*------------------------------------------------------------------*/
value *
valMinus (value * lval, value * rval)
{
  value *val;

  /* create a new value */
  val = newValue ();
  val->type = computeType (lval->type, rval->type, RESULT_TYPE_INT, '-');
  val->etype = getSpec (val->type);
  SPEC_SCLS (val->etype) = S_LITERAL;   /* will remain literal */

  if (!IS_SPEC (val->type))
    SPEC_CVAL (val->etype).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) - (TYPE_TARGET_ULONG) ulFromVal (rval);
  else if (IS_FLOAT (val->type))
    SPEC_CVAL (val->type).v_float = floatFromVal (lval) - floatFromVal (rval);
  else if (IS_FIXED16X16 (val->type))
    SPEC_CVAL (val->type).v_fixed16x16 = fixed16x16FromDouble (floatFromVal (lval) - floatFromVal (rval));
  else if (SPEC_LONGLONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) ullFromVal (lval) - (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
      else
        SPEC_CVAL (val->type).v_longlong = (TYPE_TARGET_LONGLONG) ullFromVal (lval) - (TYPE_TARGET_LONGLONG) ullFromVal (rval);
    }
  else if (SPEC_LONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) - (TYPE_TARGET_ULONG) ulFromVal (rval);
      else
        SPEC_CVAL (val->type).v_long = (TYPE_TARGET_LONG) ulFromVal (lval) - (TYPE_TARGET_LONG) ulFromVal (rval);
    }
  else
    {
      if (SPEC_USIGN (val->type))
        SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) ulFromVal (lval) - (TYPE_TARGET_UINT) ulFromVal (rval);
      else
        SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) ulFromVal (lval) - (TYPE_TARGET_INT) ulFromVal (rval);
    }
  return cheapestVal (val);
}

/*------------------------------------------------------------------*/
/* valShift - Shift left or right                                   */
/*------------------------------------------------------------------*/
value *
valShift (value * lval, value * rval, int lr)
{
  value *val;

  /* create a new value */
  val = newValue ();
  val->type = val->etype = computeType (lval->etype, NULL, RESULT_TYPE_INT, 'S');
  SPEC_SCLS (val->etype) = S_LITERAL;   /* will remain literal */

  if (getSize (val->type) * 8 <= (TYPE_TARGET_ULONG) ulFromVal (rval) &&
      /* left shift */
      (lr ||
       /* right shift and unsigned */
       (!lr && SPEC_USIGN (rval->type))) &&
       ((TYPE_TARGET_ULONG) ulFromVal (lval) != (TYPE_TARGET_ULONG) 0))
    {
      werror (W_SHIFT_CHANGED, (lr ? "left" : "right"));
    }

  if (SPEC_LONGLONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        {
          SPEC_CVAL (val->type).v_ulonglong = lr ?
            (TYPE_TARGET_ULONGLONG) ullFromVal (lval) << (TYPE_TARGET_ULONGLONG) ullFromVal (rval) :
            (TYPE_TARGET_ULONGLONG) ullFromVal (lval) >> (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
        }
      else
        {
          SPEC_CVAL (val->type).v_longlong = lr ?
            (TYPE_TARGET_LONGLONG) ullFromVal (lval) << (TYPE_TARGET_ULONGLONG) ullFromVal (rval) :
            (TYPE_TARGET_LONGLONG) ullFromVal (lval) >> (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
        }
    }
  else if (SPEC_LONG (val->type))
    {
      if (SPEC_USIGN (val->type))
        {
          SPEC_CVAL (val->type).v_ulong = lr ?
            (TYPE_TARGET_ULONG) ulFromVal (lval) << (TYPE_TARGET_ULONG) ulFromVal (rval) :
            (TYPE_TARGET_ULONG) ulFromVal (lval) >> (TYPE_TARGET_ULONG) ulFromVal (rval);
        }
      else
        {
          SPEC_CVAL (val->type).v_long = lr ?
            (TYPE_TARGET_LONG) ulFromVal (lval) << (TYPE_TARGET_ULONG) ulFromVal (rval) :
            (TYPE_TARGET_LONG) ulFromVal (lval) >> (TYPE_TARGET_ULONG) ulFromVal (rval);
        }
    }
  else
    {
      if (SPEC_USIGN (val->type))
        {
          SPEC_CVAL (val->type).v_uint = lr ?
            (TYPE_TARGET_UINT) ulFromVal (lval) << (TYPE_TARGET_ULONG) ulFromVal (rval) :
            (TYPE_TARGET_UINT) ulFromVal (lval) >> (TYPE_TARGET_ULONG) ulFromVal (rval);
        }
      else
        {
          SPEC_CVAL (val->type).v_int = lr ?
            (TYPE_TARGET_INT) ulFromVal (lval) << (TYPE_TARGET_ULONG) ulFromVal (rval) :
            (TYPE_TARGET_INT) ulFromVal (lval) >> (TYPE_TARGET_ULONG) ulFromVal (rval);
        }
    }
  return cheapestVal (val);
}

/*------------------------------------------------------------------*/
/* valCompare - Compares two literal                                */
/*------------------------------------------------------------------*/
value *
valCompare (value * lval, value * rval, int ctype)
{
  value *val;

  /* create a new value */
  val = newValue ();
  val->type = val->etype = newCharLink ();
  val->type->xclass = SPECIFIER;
  SPEC_NOUN (val->type) = V_CHAR;       /* type is char */
  SPEC_USIGN (val->type) = 1;
  SPEC_SCLS (val->type) = S_LITERAL;    /* will remain literal */

  switch (ctype)
    {
    /* FIXME: need to add long long support to inequalities */
    case '<':
      SPEC_CVAL (val->type).v_int = floatFromVal (lval) < floatFromVal (rval);
      break;

    case '>':
      SPEC_CVAL (val->type).v_int = floatFromVal (lval) > floatFromVal (rval);
      break;

    case LE_OP:
      SPEC_CVAL (val->type).v_int = floatFromVal (lval) <= floatFromVal (rval);
      break;

    case GE_OP:
      SPEC_CVAL (val->type).v_int = floatFromVal (lval) >= floatFromVal (rval);
      break;

    case EQ_OP:
      if (SPEC_NOUN (lval->type) == V_FLOAT || SPEC_NOUN (rval->type) == V_FLOAT)
        {
          SPEC_CVAL (val->type).v_int = floatFromVal (lval) == floatFromVal (rval);
        }
      else if (SPEC_NOUN (lval->type) == V_FIXED16X16 || SPEC_NOUN (rval->type) == V_FIXED16X16)
        {
          SPEC_CVAL (val->type).v_int = floatFromVal (lval) == floatFromVal (rval);
        }
      else
        {
          /* integrals: ignore signedness */
          TYPE_TARGET_ULONGLONG l, r;

          l = (TYPE_TARGET_ULONGLONG) ullFromVal (lval);
          r = (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
          /* In order to correctly compare 'signed int' and 'unsigned int' it's
             neccessary to strip them to 16 bit.
             Literals are reduced to their cheapest type, therefore left and
             right might have different types. It's neccessary to find a
             common type: int (used for char too) or long */
          if (!IS_LONGLONG (lval->etype) && !IS_LONGLONG (rval->etype))
            {
              r = (TYPE_TARGET_ULONG) r;
              l = (TYPE_TARGET_ULONG) l;
            }
          if (!IS_LONG (lval->etype) && !IS_LONG (rval->etype))
            {
              r = (TYPE_TARGET_UINT) r;
              l = (TYPE_TARGET_UINT) l;
            }
          SPEC_CVAL (val->type).v_int = l == r;
        }
      break;
    case NE_OP:
      if (SPEC_NOUN (lval->type) == V_FLOAT || SPEC_NOUN (rval->type) == V_FLOAT)
        {
          SPEC_CVAL (val->type).v_int = floatFromVal (lval) != floatFromVal (rval);
        }
      else if (SPEC_NOUN (lval->type) == V_FIXED16X16 || SPEC_NOUN (rval->type) == V_FIXED16X16)
        {
          SPEC_CVAL (val->type).v_int = floatFromVal (lval) != floatFromVal (rval);
        }
      else
        {
          /* integrals: ignore signedness */
          TYPE_TARGET_ULONGLONG l, r;

          l = (TYPE_TARGET_ULONGLONG) ullFromVal (lval);
          r = (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
          /* In order to correctly compare 'signed int' and 'unsigned int' it's
             neccessary to strip them to 16 bit.
             Literals are reduced to their cheapest type, therefore left and
             right might have different types. It's neccessary to find a
             common type: int (used for char too) or long */
          if (!IS_LONGLONG (lval->etype) && !IS_LONGLONG (rval->etype))
            {
              r = (TYPE_TARGET_ULONG) r;
              l = (TYPE_TARGET_ULONG) l;
            }
          if (!IS_LONG (lval->etype) && !IS_LONG (rval->etype))
            {
              r = (TYPE_TARGET_UINT) r;
              l = (TYPE_TARGET_UINT) l;
            }
          SPEC_CVAL (val->type).v_int = l != r;
        }
      break;

    }

  return val;
}

/*------------------------------------------------------------------*/
/* valBitwise - Bitwise operation                                   */
/*------------------------------------------------------------------*/
value *
valBitwise (value * lval, value * rval, int op)
{
  value *val;

  /* create a new value */
  val = newValue ();
  val->type = computeType (lval->etype, rval->etype, RESULT_TYPE_CHAR, op);
  val->etype = getSpec (val->type);
  SPEC_SCLS (val->etype) = S_LITERAL;

  switch (op)
    {
    case '&':
      if (SPEC_LONGLONG (val->type))
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) ullFromVal (lval) & (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
          else
            SPEC_CVAL (val->type).v_longlong = (TYPE_TARGET_LONGLONG) ullFromVal (lval) & (TYPE_TARGET_LONGLONG) ullFromVal (rval);
        }
      else if (SPEC_LONG (val->type))
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) & (TYPE_TARGET_ULONG) ulFromVal (rval);
          else
            SPEC_CVAL (val->type).v_long = (TYPE_TARGET_LONG) ulFromVal (lval) & (TYPE_TARGET_LONG) ulFromVal (rval);
        }
      else
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) ulFromVal (lval) & (TYPE_TARGET_UINT) ulFromVal (rval);
          else
            SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) ulFromVal (lval) & (TYPE_TARGET_INT) ulFromVal (rval);
        }
      break;

    case '|':
      if (SPEC_LONGLONG (val->type))
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) ullFromVal (lval) | (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
          else
            SPEC_CVAL (val->type).v_longlong = (TYPE_TARGET_LONGLONG) ullFromVal (lval) | (TYPE_TARGET_LONGLONG) ullFromVal (rval);
        }
      else if (SPEC_LONG (val->type))
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) | (TYPE_TARGET_ULONG) ulFromVal (rval);
          else
            SPEC_CVAL (val->type).v_long = (TYPE_TARGET_LONG) ulFromVal (lval) | (TYPE_TARGET_LONG) ulFromVal (rval);
        }
      else
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) ulFromVal (lval) | (TYPE_TARGET_UINT) ulFromVal (rval);
          else
            SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) ulFromVal (lval) | (TYPE_TARGET_INT) ulFromVal (rval);
        }

      break;

    case '^':
      if (SPEC_LONGLONG (val->type))
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_ulonglong = (TYPE_TARGET_ULONGLONG) ullFromVal (lval) ^ (TYPE_TARGET_ULONGLONG) ullFromVal (rval);
          else
            SPEC_CVAL (val->type).v_longlong = (TYPE_TARGET_LONGLONG) ullFromVal (lval) ^ (TYPE_TARGET_LONGLONG) ullFromVal (rval);
        }
      else if (SPEC_LONG (val->type))
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_ulong = (TYPE_TARGET_ULONG) ulFromVal (lval) ^ (TYPE_TARGET_ULONG) ulFromVal (rval);
          else
            SPEC_CVAL (val->type).v_long = (TYPE_TARGET_LONG) ulFromVal (lval) ^ (TYPE_TARGET_LONG) ulFromVal (rval);
        }
      else
        {
          if (SPEC_USIGN (val->type))
            SPEC_CVAL (val->type).v_uint = (TYPE_TARGET_UINT) ulFromVal (lval) ^ (TYPE_TARGET_UINT) ulFromVal (rval);
          else
            SPEC_CVAL (val->type).v_int = (TYPE_TARGET_INT) ulFromVal (lval) ^ (TYPE_TARGET_INT) ulFromVal (rval);
        }
      break;
    }

  return cheapestVal (val);
}

/*------------------------------------------------------------------*/
/* valAndOr   - Generates code for and / or operation               */
/*------------------------------------------------------------------*/
value *
valLogicAndOr (value * lval, value * rval, int op)
{
  value *val;

  /* create a new value */
  val = newValue ();
  val->type = val->etype = newCharLink ();
  val->type->xclass = SPECIFIER;
  SPEC_SCLS (val->type) = S_LITERAL;    /* will remain literal */
  SPEC_USIGN (val->type) = 1;

  switch (op)
    {
    case AND_OP:
      SPEC_CVAL (val->type).v_int = !isEqualVal (lval, 0) && !isEqualVal (rval, 0);
      break;

    case OR_OP:
      SPEC_CVAL (val->type).v_int = !isEqualVal (lval, 0) || !isEqualVal (rval, 0);
      break;
    }

  return val;
}

/*------------------------------------------------------------------*/
/* valCastLiteral - casts a literal value to another type           */
/*------------------------------------------------------------------*/
value *
valCastLiteral (sym_link * dtype, double fval, TYPE_TARGET_ULONGLONG llval)
{
  value *val;
  unsigned long l = double2ul (fval);

  if (!dtype)
    return NULL;

  val = newValue ();
  if (dtype)
    val->etype = getSpec (val->type = copyLinkChain (dtype));
  else
    {
      val->etype = val->type = newLink (SPECIFIER);
      SPEC_NOUN (val->etype) = V_VOID;
    }
  SPEC_SCLS (val->etype) = S_LITERAL;

  /* if it is not a specifier then we can assume that */
  /* it will be an unsigned long                      */
  if (!IS_SPEC (val->type))
    {
      SPEC_CVAL (val->etype).v_ulong = (TYPE_TARGET_ULONG) l;
      return val;
    }

  switch (SPEC_NOUN (val->etype))
    {
    case V_FLOAT:
      SPEC_CVAL (val->etype).v_float = fval;
      break;

    case V_FIXED16X16:
      SPEC_CVAL (val->etype).v_fixed16x16 = fixed16x16FromDouble (fval);
      break;

    case V_BOOL:
    case V_BIT:
    case V_SBIT:
      SPEC_CVAL (val->etype).v_uint = fval ? 1 : 0;
      break;

    case V_BITFIELD:
      l &= (0xffffffffu >> (32 - SPEC_BLEN (val->etype)));
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_uint = (TYPE_TARGET_UINT) l;
      else
        SPEC_CVAL (val->etype).v_int = (TYPE_TARGET_INT) l;
      break;

    case V_CHAR:
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_uint = (TYPE_TARGET_UCHAR) l;
      else
        SPEC_CVAL (val->etype).v_int = (TYPE_TARGET_CHAR) l;
      break;

    default:
      if (SPEC_LONGLONG (val->etype))
        {
          if (SPEC_USIGN (val->etype))
            SPEC_CVAL (val->etype).v_ulonglong = (TYPE_TARGET_ULONGLONG) llval;
          else
            SPEC_CVAL (val->etype).v_longlong = (TYPE_TARGET_LONGLONG) llval;
        }
      else if (SPEC_LONG (val->etype))
        {
          if (SPEC_USIGN (val->etype))
            SPEC_CVAL (val->etype).v_ulong = (TYPE_TARGET_ULONG) l;
          else
            SPEC_CVAL (val->etype).v_long = (TYPE_TARGET_LONG) l;
        }
      else
        {
          if (SPEC_USIGN (val->etype))
            SPEC_CVAL (val->etype).v_uint = (TYPE_TARGET_UINT) l;
          else
            SPEC_CVAL (val->etype).v_int = (TYPE_TARGET_INT) l;
        }
    }

  return val;
}

/*-------------------------------------------------------------------*/
/* valRecastLitVal - changes type of a literal value to another type */
/*-------------------------------------------------------------------*/
value *
valRecastLitVal (sym_link * dtype, value * val)
{
  sym_link * otype = val->type;
  double fval;
  TYPE_TARGET_ULONGLONG ull;

  if (IS_SPEC (otype) && (SPEC_NOUN (otype) == V_FIXED16X16 || SPEC_NOUN (otype) == V_FLOAT))
    {
      fval = floatFromVal (val);
      ull = (TYPE_TARGET_ULONGLONG)fval;
    }
  else
    {
      ull = (TYPE_TARGET_ULONGLONG) ullFromVal (val);
      fval = (double)ull;
    }

  if (dtype)
    val->etype = getSpec (val->type = copyLinkChain (dtype));
  else
    {
      val->etype = val->type = newLink (SPECIFIER);
      SPEC_NOUN (val->etype) = V_VOID;
    }
  SPEC_SCLS (val->etype) = S_LITERAL;

  /* if it is not a specifier then we can assume that */
  /* it will be an unsigned long                      */
  if (!IS_SPEC (val->type))
    {
      SPEC_CVAL (val->etype).v_ulong = (TYPE_TARGET_ULONG) ull;
      return val;
    }

  switch (SPEC_NOUN (val->etype))
    {
    case V_FLOAT:
      SPEC_CVAL (val->etype).v_float = fval;
      break;

    case V_FIXED16X16:
      SPEC_CVAL (val->etype).v_fixed16x16 = fixed16x16FromDouble (fval);
      break;

    case V_BOOL:
    case V_BIT:
    case V_SBIT:
      SPEC_CVAL (val->etype).v_uint = fval ? 1 : 0;
      break;

    case V_BITFIELD:
      ull &= (0xffffffffu >> (32 - SPEC_BLEN (val->etype)));
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_uint = (TYPE_TARGET_UINT) ull;
      else
        SPEC_CVAL (val->etype).v_int = (TYPE_TARGET_INT) ull;
      break;

    case V_CHAR:
      if (SPEC_USIGN (val->etype))
        SPEC_CVAL (val->etype).v_uint = (TYPE_TARGET_UCHAR) ull;
      else
        SPEC_CVAL (val->etype).v_int = (TYPE_TARGET_CHAR) ull;
      break;

    default:
      if (SPEC_LONGLONG (val->etype))
        {
          if (SPEC_USIGN (val->etype))
            SPEC_CVAL (val->etype).v_ulonglong = (TYPE_TARGET_ULONGLONG) ull;
          else
            SPEC_CVAL (val->etype).v_longlong = (TYPE_TARGET_LONGLONG) ull;
        }
      else if (SPEC_LONG (val->etype))
        {
          if (SPEC_USIGN (val->etype))
            SPEC_CVAL (val->etype).v_ulong = (TYPE_TARGET_ULONG) ull;
          else
            SPEC_CVAL (val->etype).v_long = (TYPE_TARGET_LONG) ull;
        }
      else
        {
          if (SPEC_USIGN (val->etype))
            SPEC_CVAL (val->etype).v_uint = (TYPE_TARGET_UINT) ull;
          else
            SPEC_CVAL (val->etype).v_int = (TYPE_TARGET_INT) ull;
        }
    }

  return val;
}

/*------------------------------------------------------------------*/
/* getNelements - determines # of elements from init list           */
/*------------------------------------------------------------------*/
int
getNelements (sym_link * type, initList * ilist)
{
  int i, size;

  if (!ilist)
    return 0;

  if (ilist->type == INIT_DEEP)
    ilist = ilist->init.deep;

  /* if type is a character array and there is only one
     (string) initialiser then get the length of the string */
  if (IS_ARRAY (type) && (IS_CHAR (type->next) || IS_INT (type->next) && IS_UNSIGNED (type->next)) && !ilist->next)
    {
      ast *iast = ilist->init.node;
      value *v = (iast->type == EX_VALUE ? iast->opval.val : NULL);

      if (v && IS_ARRAY (v->type) && (IS_CHAR (v->etype) || IS_INT (v->etype) && IS_UNSIGNED (v->etype) && IS_LONG (type->next) == IS_LONG (v->etype)))
        /* yep, it's a string */
        {
          return DCL_ELEM (v->type);
        }
    }

  size = 0;
  i = 0;
  while (ilist)
    {
      if (ilist->designation)
        {
          if (ilist->designation->type != DESIGNATOR_ARRAY)
            {
              // structure designator for array, boo.
              werrorfl (ilist->filename, ilist->lineno, E_BAD_DESIGNATOR);
            }
          else
            {
              i = ilist->designation->designator.elemno;
            }
        }
      if (size <= i)
        {
          size = i + 1; /* array size is one larger than array init element */
        }
      i++;
      ilist = ilist->next;
    }
  return size;
}

/*-----------------------------------------------------------------*/
/* valForArray - returns a value with name of array index          */
/*-----------------------------------------------------------------*/
value *
valForArray (ast * arrExpr)
{
  value *val, *lval = NULL;
  int size = getSize (arrExpr->left->ftype->next);

  /* if the right or left is an array
     resolve it first */
  if (IS_AST_OP (arrExpr->left))
    {
      if (arrExpr->left->opval.op == '[')
        lval = valForArray (arrExpr->left);
      else if (arrExpr->left->opval.op == '.')
        lval = valForStructElem (arrExpr->left->left, arrExpr->left->right);
      else if (arrExpr->left->opval.op == PTR_OP)
        {
          if (IS_ADDRESS_OF_OP (arrExpr->left->left))
            lval = valForStructElem (arrExpr->left->left->left, arrExpr->left->right);
          else if (IS_AST_VALUE (arrExpr->left->left) && IS_PTR (arrExpr->left->left->ftype))
            lval = valForStructElem (arrExpr->left->left, arrExpr->left->right);
        }
      else
        return NULL;
    }
  else if (!IS_AST_SYM_VALUE (arrExpr->left))
    return NULL;

  if (!IS_AST_LIT_VALUE (arrExpr->right))
    return NULL;

  val = newValue ();
  val->type = newLink (DECLARATOR);
  if (IS_AST_LIT_VALUE (arrExpr->left) && IS_PTR (arrExpr->left->ftype))
    {
      SNPRINTF (val->name, sizeof (val->name), "0x%X",
                AST_ULONG_VALUE (arrExpr->left) + AST_ULONG_VALUE (arrExpr->right) * size);
      memcpy (val->type, arrExpr->left->ftype, sizeof (sym_link));
    }
  else if (lval)
    {
      SNPRINTF (val->name, sizeof (val->name), "(%s + %d)", lval->name, AST_ULONG_VALUE (arrExpr->right) * size);
      memcpy (val->type, lval->type, sizeof (sym_link));
    }
  else
    {
      SNPRINTF (val->name, sizeof (val->name), "(%s + %d)",
                AST_SYMBOL (arrExpr->left)->rname, AST_ULONG_VALUE (arrExpr->right) * size);
      if (SPEC_SCLS (arrExpr->left->etype) == S_CODE)
        DCL_TYPE (val->type) = CPOINTER;
      else if (SPEC_SCLS (arrExpr->left->etype) == S_XDATA)
        DCL_TYPE (val->type) = FPOINTER;
      else if (SPEC_SCLS (arrExpr->left->etype) == S_XSTACK)
        DCL_TYPE (val->type) = PPOINTER;
      else if (SPEC_SCLS (arrExpr->left->etype) == S_IDATA)
        DCL_TYPE (val->type) = IPOINTER;
      else if (SPEC_SCLS (arrExpr->left->etype) == S_EEPROM)
        DCL_TYPE (val->type) = EEPPOINTER;
      else
        DCL_TYPE (val->type) = POINTER;
    }

  val->type->next = arrExpr->left->ftype->next;
  val->etype = getSpec (val->type);
  return val;
}

/*-----------------------------------------------------------------*/
/* valForStructElem - returns value with name of struct element    */
/*-----------------------------------------------------------------*/
value *
valForStructElem (ast * structT, ast * elemT)
{
  value *val, *lval = NULL;
  symbol *sym;
  int idxoff = 0;
  ast *sast = NULL;

  /* left could be further derefed */
  if (IS_AST_OP (structT))
    {
      if (structT->opval.op == '[')
        lval = valForArray (structT);
      else if (structT->opval.op == '+')
        {
          if (IS_AST_LIT_VALUE (structT->right) && !IS_AST_OP (structT->left))
            {
              idxoff = (int) (AST_ULONG_VALUE (structT->right) * getSize (structT->left->ftype->next));
              sast = structT->left;
            }
          else if (IS_AST_LIT_VALUE (structT->left) && !IS_AST_OP (structT->right))
            {
              idxoff = (int) (AST_ULONG_VALUE (structT->left) * getSize (structT->right->ftype->next));
              sast = structT->right;
            }
          else
            return NULL;
        }
      else if (structT->opval.op == '-')
        {
          if (IS_AST_LIT_VALUE (structT->right) && !IS_AST_OP (structT->left))
            {
              idxoff = 0 - (int) (AST_ULONG_VALUE (structT->right) * getSize (structT->left->ftype->next));
              sast = structT->left;
            }
          else
            return NULL;
        }
      else if (structT->opval.op == '.')
        lval = valForStructElem (structT->left, structT->right);
      else if (structT->opval.op == PTR_OP)
        {
          if (IS_ADDRESS_OF_OP (structT->left))
            lval = valForStructElem (structT->left->left, structT->right);
          else if (IS_AST_VALUE (structT->left) && IS_PTR (structT->left->ftype))
            lval = valForStructElem (structT->left, structT->right);
        }
      else
        return NULL;
    }

  if (!IS_AST_SYM_VALUE (elemT))
    return NULL;

  if (!structT || !IS_STRUCT (structT->etype))
    return NULL;

  if ((sym = getStructElement (SPEC_STRUCT (structT->etype), AST_SYMBOL (elemT))) == NULL)
    {
      return NULL;
    }

  val = newValue ();
  val->type = newLink (DECLARATOR);
  if (IS_AST_LIT_VALUE (structT) && IS_PTR (structT->ftype))
    {
      SNPRINTF (val->name, sizeof (val->name), "0x%X", AST_ULONG_VALUE (structT) + (int) sym->offset);
      memcpy (val->type, structT->ftype, sizeof (sym_link));
    }
  else if (lval)
    {
      SNPRINTF (val->name, sizeof (val->name), "(%s + %d)", lval->name, (int) sym->offset);
      memcpy (val->type, lval->type, sizeof (sym_link));
    }
  else
    {
      if (sast)
        SNPRINTF (val->name, sizeof (val->name), "(%s + (%d))", AST_SYMBOL (sast)->rname, ((int) sym->offset) + idxoff);
      else
        SNPRINTF (val->name, sizeof (val->name), "(%s + %d)", AST_SYMBOL (structT)->rname, (int) sym->offset);

      if (SPEC_SCLS (structT->etype) == S_CODE)
        DCL_TYPE (val->type) = CPOINTER;
      else if (SPEC_SCLS (structT->etype) == S_XDATA)
        DCL_TYPE (val->type) = FPOINTER;
      else if (SPEC_SCLS (structT->etype) == S_XSTACK)
        DCL_TYPE (val->type) = PPOINTER;
      else if (SPEC_SCLS (structT->etype) == S_IDATA)
        DCL_TYPE (val->type) = IPOINTER;
      else if (SPEC_SCLS (structT->etype) == S_EEPROM)
        DCL_TYPE (val->type) = EEPPOINTER;
      else
        DCL_TYPE (val->type) = POINTER;
    }

  val->type->next = sym->type;
  val->etype = getSpec (val->type);
  return val;
}

/*-----------------------------------------------------------------*/
/* valForCastAggr - will return value for a cast of an aggregate   */
/*                  plus minus a constant                          */
/*-----------------------------------------------------------------*/
value *
valForCastAggr (ast * aexpr, sym_link * type, ast * cnst, int op)
{
  value *val;

  if (!IS_AST_SYM_VALUE (aexpr))
    return NULL;
  if (!IS_AST_LIT_VALUE (cnst))
    return NULL;

  val = newValue ();

  SNPRINTF (val->name, sizeof (val->name), "(%s %c %d)",
            AST_SYMBOL (aexpr)->rname, op, getSize (type->next) * AST_ULONG_VALUE (cnst));

  val->type = type;
  val->etype = getSpec (val->type);
  return val;
}

/*-----------------------------------------------------------------*/
/* valForCastArr - will return value for a cast of an aggregate    */
/*                 with no constant                                */
/*-----------------------------------------------------------------*/
value *
valForCastArr (ast * aexpr, sym_link * type)
{
  value *val;

  if (!IS_AST_SYM_VALUE (aexpr))
    return NULL;

  val = newValue ();

  SNPRINTF (val->name, sizeof (val->name), "(%s)", AST_SYMBOL (aexpr)->rname);

  val->type = type;
  val->etype = getSpec (val->type);
  return val;
}
