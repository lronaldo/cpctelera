/*----------------------------------------------------------------------
  SDCCval.h - value wrapper related header information
  Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1997)

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
#ifndef SDCCVAL_H
#define SDCCVAL_H

#include "SDCCsymt.h"


/* value wrapper */
typedef struct value
{
  char name[SDCC_NAME_MAX + 1]; /* operand accessing this value     */
  sym_link *type;               /* start of type chain              */
  sym_link *etype;              /* end of type chain                */
  symbol *sym;                  /* Original Symbol                  */
  struct value *next;           /* used in initializer list         */
  unsigned vArgs:1;             /* arg list ended with variable arg */
}
value;

typedef struct literalList
{
  double literalValue;
  unsigned count;
  struct literalList *next;
} literalList;

enum
{
  INIT_NODE,
  INIT_DEEP,
  INIT_HOLE
};

/* initializer lists use this structure */
typedef struct initList
{
  int type;
  int lineno;
  char *filename;
  struct designation *designation;
  union
  {
    struct ast *node;
    struct initList *deep;
  }
  init;

  struct initList *next;
}
initList;

enum
  {
    DESIGNATOR_STRUCT,
    DESIGNATOR_ARRAY
  };

/* designated initializers */
typedef struct designation
  {
    int type;
    int lineno;
    char *filename;
    union
      {
        struct symbol *tag;             /* tag part of structure          */
        int elemno;                     /* array element (constant expr)  */
      }
    designator;

    struct designation *next;           /* next part of nested designator */
  }
designation;

/* return values from checkConstantRange */
typedef enum
{
  CCR_OK,                       /* evaluate at runtime */
  CCR_OVL,
  CCR_ALWAYS_FALSE,
  CCR_ALWAYS_TRUE
}
CCR_RESULT;

#define  IS_VARG(x)             (x->vArgs)

/* forward definitions for the symbol table related functions */
value *newValue (void);
value *constVal (const char *);
value *constCharacterVal (unsigned long v, char type);
value *constCharVal (unsigned char v);
value *constBoolVal (bool v);
value *reverseVal (value *);
value *reverseValWithType (value *);
value *copyValue (value *);
value *copyValueChain (value *);
value *strVal (const char *);
value *charVal (const char *);
value *symbolVal (symbol *);
void printVal (value *);
double floatFromVal (value *);
unsigned long ulFromVal (value *);
unsigned long long ullFromVal (value *);

/* convert a fixed16x16 type to double */
double doubleFromFixed16x16 (TYPE_TARGET_ULONG value);

/* convert a double type to fixed16x16 */
TYPE_TARGET_ULONG fixed16x16FromDouble (double value);

CCR_RESULT checkConstantRange (sym_link * var, sym_link * lit, int op, bool exchangeOps);
value *array2Ptr (value *);
value *valUnaryPM (value *);
value *valComplement (value *);
value *valNot (value *);
value *valMult (value *, value *);
value *valDiv (value *, value *);
value *valMod (value *, value *);
value *valPlus (value *, value *);
value *valMinus (value *, value *);
value *valShift (value *, value *, int);
value *valCompare (value *, value *, int);
value *valBitwise (value *, value *, int);
value *valLogicAndOr (value *, value *, int);
value *valCastLiteral (sym_link *, double, TYPE_TARGET_ULONGLONG);
value *valueFromLit (double);
initList *newiList (int, void *);
initList *revinit (initList *);
initList *copyIlist (initList *);
double list2int (initList *);
value *list2val (initList *, int);
struct ast *list2expr (initList *);
void resolveIvalSym (initList *, sym_link *);
designation *newDesignation(int, void *);
designation *revDesignation (designation *);
designation *copyDesignation (designation *);
initList *reorderIlist (sym_link *, initList *);
value *valFromType (sym_link *);
value *constFloatVal (const char *);
value *constFixed16x16Val (const char *);
int getNelements (sym_link *, initList *);
value *valForArray (struct ast *);
value *valForStructElem (struct ast *, struct ast *);
value *valForCastAggr (struct ast *, sym_link *, struct ast *, int);
value *valForCastArr (struct ast *, sym_link *);
bool convertIListToConstList (initList * src, literalList ** lList, int size);
literalList *copyLiteralList (literalList * src);
unsigned long double2ul (double val);
unsigned char byteOfVal (value *, int);
int csdOfVal (int *topbit, int *nonzero, unsigned long long *csd_add, unsigned long long *csd_sub, value *val);
int isEqualVal (value *, int);
TYPE_TARGET_ULONGLONG ullFromLit (sym_link * lit);
value * valRecastLitVal (sym_link * dtype, value * val);

#endif
