/*-------------------------------------------------------------------------
  SDCCast.c - source file for parser support & all ast related routines

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

#define DEBUG_CF(x)             /* puts(x); */

#include <stdint.h>

#include "common.h"
#include "dbuf_string.h"
#include "SDCCbtree.h"
#include "SDCCset.h"

int currLineno = 0;
set *astList = NULL;
set *operKeyReset = NULL;
ast *staticAutos = NULL;
int labelKey = 1;
static struct
{
  int count;                    /* number of inline functions inserted */
  symbol *retsym;               /* variable for inlined function return value */
  symbol *retlab;               /* label ending inlined function (virtual return) */
} inlineState;

#define LRVAL(x) x->left->rvalue
#define RRVAL(x) x->right->rvalue
#define TRVAL(x) x->rvalue
#define LLVAL(x) x->left->lvalue
#define RLVAL(x) x->right->lvalue
#define TLVAL(x) x->lvalue
#define RTYPE(x) x->right->ftype
#define RETYPE(x) x->right->etype
#define LTYPE(x) x->left->ftype
#define LETYPE(x) x->left->etype
#define TTYPE(x) x->ftype
#define TETYPE(x) x->etype
#define ALLOCATE 1
#define DEALLOCATE 2

#define IS_AND(ex) (ex->type == EX_OP && ex->opval.op == AND_OP )
#define IS_OR(ex)  (ex->type == EX_OP && ex->opval.op == OR_OP )
#define IS_NOT(ex) (ex->type == EX_OP && ex->opval.op == '!' )
#define IS_ANDORNOT(ex) (IS_AND(ex) || IS_OR(ex) || IS_NOT(ex))
#define IS_IFX(ex) (ex->type == EX_OP && ex->opval.op == IFX )
#define IS_LT(ex)  (ex->type == EX_OP && ex->opval.op == '<' )
#define IS_GT(ex)  (ex->type == EX_OP && ex->opval.op == '>')

int noLineno = 0;
int noAlloc = 0;
symbol *currFunc = NULL;
static ast *createIval (ast *, sym_link *, initList *, ast *, ast *, int);
static ast *createIvalCharPtr (ast *, sym_link *, ast *, ast *);
static ast *optimizeCompare (ast *);
ast *optimizeRRCRLC (ast *);
ast *optimizeSWAP (ast *);
ast *optimizeGetHbit (ast *, RESULT_TYPE);
ast *optimizeGetAbit (ast *, RESULT_TYPE);
ast *optimizeGetByte (ast *, RESULT_TYPE);
ast *optimizeGetWord (ast *, RESULT_TYPE);
static ast *backPatchLabels (ast *, symbol *, symbol *);
static void copyAstLoc (ast *, ast *);
void PA (ast * t);
int inInitMode = 0;
memmap *GcurMemmap = NULL;      /* points to the memmap that's currently active */
struct dbuf_s *codeOutBuf;
extern int inCriticalFunction;
extern int inCriticalBlock;

int
ptt (ast * tree)
{
  printTypeChain (tree->ftype, stdout);
  return 0;
}

/*-----------------------------------------------------------------*/
/* newAst - creates a fresh node for an expression tree            */
/*-----------------------------------------------------------------*/
static ast *
newAst_ (unsigned type)
{
  ast *ex;

  ex = Safe_alloc (sizeof (ast));

  ex->type = type;
  ex->lineno = (noLineno ? 0 : lexLineno);
  ex->filename = lexFilename;
  ex->level = NestLevel;
  ex->block = currBlockno;
  ex->initMode = inInitMode;
  ex->seqPoint = seqPointNo;
  return ex;
}

ast *
newAst_VALUE (value * val)
{
  ast *ex = newAst_ (EX_VALUE);
  ex->opval.val = val;
  return ex;
}

ast *
newAst_OP (unsigned op)
{
  ast *ex = newAst_ (EX_OP);
  ex->opval.op = op;
  return ex;
}

ast *
newAst_LINK (sym_link * val)
{
  ast *ex = newAst_ (EX_LINK);
  ex->opval.lnk = val;
  return ex;
}

/*-----------------------------------------------------------------*/
/* newNode - creates a new node                                    */
/*-----------------------------------------------------------------*/
ast *
newNode (long op, ast * left, ast * right)
{
  ast *ex;

  ex = newAst_OP (op);
  ex->left = left;
  ex->right = right;

  return ex;
}

/*-----------------------------------------------------------------*/
/* newIfxNode - creates a new Ifx Node                             */
/*-----------------------------------------------------------------*/
ast *
newIfxNode (ast * condAst, symbol * trueLabel, symbol * falseLabel)
{
  ast *ifxNode;

  /* if this is a literal then we already know the result */
  if (condAst->etype && IS_LITERAL (condAst->etype) && IS_AST_VALUE (condAst))
    {
      /* then depending on the expression value */
      if (floatFromVal (condAst->opval.val))
        ifxNode = newNode (GOTO, newAst_VALUE (symbolVal (trueLabel)), NULL);
      else
        ifxNode = newNode (GOTO, newAst_VALUE (symbolVal (falseLabel)), NULL);
    }
  else
    {
      ifxNode = newNode (IFX, condAst, NULL);
      ifxNode->trueLabel = trueLabel;
      ifxNode->falseLabel = falseLabel;
    }

  return ifxNode;
}

/*-----------------------------------------------------------------*/
/* copyAstValues - copies value portion of ast if needed           */
/*-----------------------------------------------------------------*/
void
copyAstValues (ast * dest, ast * src)
{
  switch (src->opval.op)
    {
    case BLOCK:
      dest->values.sym = copySymbolChain (src->values.sym);
      break;

    case SWITCH:
      dest->values.switchVals.swVals = copyValueChain (src->values.switchVals.swVals);
      dest->values.switchVals.swDefault = src->values.switchVals.swDefault;
      dest->values.switchVals.swNum = src->values.switchVals.swNum;
      dest->values.switchVals.swSuffix = src->values.switchVals.swSuffix;
      break;

    case INLINEASM:
      dest->values.inlineasm = Safe_strdup (src->values.inlineasm);
      break;

    case ARRAYINIT:
      dest->values.constlist = copyLiteralList (src->values.constlist);
      break;

    case FOR:
      AST_FOR (dest, trueLabel) = copySymbol (AST_FOR (src, trueLabel));
      AST_FOR (dest, continueLabel) = copySymbol (AST_FOR (src, continueLabel));
      AST_FOR (dest, falseLabel) = copySymbol (AST_FOR (src, falseLabel));
      AST_FOR (dest, condLabel) = copySymbol (AST_FOR (src, condLabel));
      AST_FOR (dest, initExpr) = copyAst (AST_FOR (src, initExpr));
      AST_FOR (dest, condExpr) = copyAst (AST_FOR (src, condExpr));
      AST_FOR (dest, loopExpr) = copyAst (AST_FOR (src, loopExpr));
      break;

    case CAST:
      dest->values.cast.literalFromCast = src->values.cast.literalFromCast;
      dest->values.cast.removedCast = src->values.cast.removedCast;
      dest->values.cast.implicitCast = src->values.cast.implicitCast;
    }
}

/*-----------------------------------------------------------------*/
/* copyAst - makes a copy of a given astession                     */
/*-----------------------------------------------------------------*/
ast *
copyAst (ast * src)
{
  ast *dest = NULL;
  static long level = -1;
  static set *pset;

  if (!src)
    return NULL;

  if (++level == 0)
    if ((pset = newSet ()) == NULL)
      goto exit_1;

  /* check if it is a infinate recursive call */
  if (isinSet (pset, src))
    goto exit_1;
  else
    addSet (&pset, src);

  dest = Safe_alloc (sizeof (ast));
  dest->type = src->type;
  dest->filename = src->filename;
  dest->lineno = src->lineno;
  dest->level = src->level;
  dest->funcName = src->funcName;
  dest->reversed = src->reversed;

  if (src->ftype)
    dest->etype = getSpec (dest->ftype = copyLinkChain (src->ftype));

  /* if this is a leaf */
  /* if value */
  if (src->type == EX_VALUE)
    {
      dest->opval.val = copyValue (src->opval.val);
      goto exit;
    }

  /* if link */
  if (src->type == EX_LINK)
    {
      dest->opval.lnk = copyLinkChain (src->opval.lnk);
      goto exit;
    }

  dest->opval.op = src->opval.op;

  /* if this is a node that has special values */
  copyAstValues (dest, src);

  dest->trueLabel = copySymbol (src->trueLabel);
  dest->falseLabel = copySymbol (src->falseLabel);
  dest->left = copyAst (src->left);
  dest->right = copyAst (src->right);

exit:
  if (pset != NULL && isinSet (pset, src))
    deleteSetItem (&pset, src);
exit_1:
  if (level-- == 0)
    if (pset != NULL)
      deleteSet (&pset);
  return dest;
}

#if 0
/*-----------------------------------------------------------------*/
/* removeIncDecOps: remove for side effects in *_ASSIGN's          */
/*                  "*s++ += 3" -> "*s++ = *s++ + 3"               */
/*-----------------------------------------------------------------*/
ast *
removeIncDecOps (ast * tree)
{

  // traverse the tree and remove inc/dec ops

  if (!tree)
    return NULL;

  if (tree->type == EX_OP && (tree->opval.op == INC_OP || tree->opval.op == DEC_OP))
    {
      if (tree->left)
        tree = tree->left;
      else
        tree = tree->right;
    }

  tree->left = removeIncDecOps (tree->left);
  tree->right = removeIncDecOps (tree->right);

  return tree;
}

/*-----------------------------------------------------------------*/
/* removePreIncDecOps: remove for side effects in *_ASSIGN's       */
/*                  "*++s += 3" -> "*++s = *++s + 3"               */
/*-----------------------------------------------------------------*/
ast *
removePreIncDecOps (ast * tree)
{

  // traverse the tree and remove pre-inc/dec ops

  if (!tree)
    return NULL;

  if (tree->type == EX_OP && (tree->opval.op == INC_OP || tree->opval.op == DEC_OP))
    {
      if (tree->right)
        tree = tree->right;
    }

  tree->left = removePreIncDecOps (tree->left);
  tree->right = removePreIncDecOps (tree->right);

  return tree;
}

/*-----------------------------------------------------------------*/
/* removePostIncDecOps: remove for side effects in *_ASSIGN's      */
/*                  "*s++ += 3" -> "*s++ = *s++ + 3"               */
/*-----------------------------------------------------------------*/
ast *
removePostIncDecOps (ast * tree)
{

  // traverse the tree and remove pre-inc/dec ops

  if (!tree)
    return NULL;

  if (tree->type == EX_OP && (tree->opval.op == INC_OP || tree->opval.op == DEC_OP))
    {
      if (tree->left)
        tree = tree->left;
    }

  tree->left = removePostIncDecOps (tree->left);
  tree->right = removePostIncDecOps (tree->right);

  return tree;
}
#endif

/*-----------------------------------------------------------------*/
/* replaceAstWithTemporary: Replace the AST pointed to by the arg  */
/*            with a reference to a new temporary variable. Returns*/
/*            an AST which assigns the original value to the       */
/*            temporary.                                           */
/*-----------------------------------------------------------------*/
static ast *
replaceAstWithTemporary (ast ** treeptr)
{
  symbol *sym = newSymbol (genSymName (NestLevel), NestLevel);
  ast *tempvar;

  /* Tell gatherImplicitVariables() to automatically give the
     symbol the correct type */
  sym->infertype = 1;
  sym->type = NULL;
  sym->etype = NULL;

  tempvar = newNode ('=', newAst_VALUE (symbolVal (sym)), *treeptr);
  *treeptr = newAst_VALUE (symbolVal (sym));

  addSymChain (&sym);

  return tempvar;
}

/*-----------------------------------------------------------------*/
/* createRMW: Create a read-modify-write expression, using a       */
/*            temporary variable if necessary to avoid duplicating */
/*            any side effects, for use in e.g.                    */
/*               foo()->count += 5;      becomes                   */
/*               tmp = foo(); tmp->count = tmp->count + 5;         */
/*-----------------------------------------------------------------*/
ast *
createRMW (ast * target, unsigned op, ast * operand)
{
  ast *readval, *writeval;
  ast *tempvar1 = NULL;
  ast *tempvar2 = NULL;
  ast *result;

  if (!target || !operand)
    {
      return NULL;
    }

  /* we need to create two copies of target: one to read from and
     one to write to. but we need to do this without duplicating
     any side effects that may be contained in the tree. */

  if (IS_AST_OP (target))
    {
      /* if this is a dereference, put the referenced item in the temporary */
      if (IS_DEREF_OP (target))
        {
          /* create a new temporary containing the item being dereferenced */
          if (hasSEFcalls (target->left))
            tempvar1 = replaceAstWithTemporary (&(target->left));
        }
      else if (target->opval.op == '[')
        {
          /* Array access is similar, but we have to avoid side effects in
             both values [WIML: Why not transform a[b] to *(a+b) in parser?] */
          if (hasSEFcalls (target->left))
            tempvar1 = replaceAstWithTemporary (&(target->left));
          if (hasSEFcalls (target->right))
            tempvar2 = replaceAstWithTemporary (&(target->right));
        }
      else if ((target->opval.op == INC_OP) || (target->opval.op == DEC_OP))
        {
          /* illegal pre/post-increment/decrement */
          werrorfl (target->filename, target->lineno, E_LVALUE_REQUIRED, "=");
        }
      else
        {
          /* we would have to handle '.', but it is not generated any more */
          wassertl (target->opval.op != '.', "obsolete opcode in tree");

          /* no other kinds of ASTs are lvalues and can contain side effects */
        }
    }

  readval = target;
  writeval = copyAst (target);

  result = newNode ('=', writeval, newNode (op, readval, operand));
  if (tempvar2)
    result = newNode (',', tempvar2, result);
  if (tempvar1)
    result = newNode (',', tempvar1, result);

  return result;
}

/*-----------------------------------------------------------------*/
/* hasSEFcalls - returns TRUE if tree has a function call,         */
/*               inc/decrement, or other side effect               */
/*-----------------------------------------------------------------*/
bool
hasSEFcalls (ast * tree)
{
  if (!tree)
    return FALSE;

  if (tree->type == EX_OP &&
      (tree->opval.op == CALL ||
       tree->opval.op == PCALL || tree->opval.op == '=' || tree->opval.op == INC_OP || tree->opval.op == DEC_OP))
    return TRUE;

  if (astHasVolatile(tree))
    return TRUE;

  return (hasSEFcalls (tree->left) | hasSEFcalls (tree->right));
}

/*-----------------------------------------------------------------*/
/* isAstEqual - compares two asts & returns 1 if they are equal    */
/*-----------------------------------------------------------------*/
static int
isAstEqual (ast * t1, ast * t2)
{
  if (!t1 && !t2)
    return 1;

  if (!t1 || !t2)
    return 0;

  /* match type */
  if (t1->type != t2->type)
    return 0;

  switch (t1->type)
    {
    case EX_OP:
      if (t1->opval.op != t2->opval.op)
        return 0;
      return (isAstEqual (t1->left, t2->left) && isAstEqual (t1->right, t2->right));
      break;

    case EX_VALUE:
      if (t1->opval.val->sym)
        {
          if (!t2->opval.val->sym)
            return 0;
          else
            return isSymbolEqual (t1->opval.val->sym, t2->opval.val->sym);
        }
      else
        {
          if (t2->opval.val->sym)
            return 0;
          else
            return (floatFromVal (t1->opval.val) == floatFromVal (t2->opval.val));
        }
      break;

      /* only compare these two types */
    default:
      return 0;
    }
}

/*-----------------------------------------------------------------*/
/* resolveSymbols - resolve symbols from the symbol table          */
/*-----------------------------------------------------------------*/
ast *
resolveSymbols (ast * tree)
{
  /* walk the entire tree and check for values */
  /* with symbols if we find one then replace  */
  /* symbol with that from the symbol table    */

  if (tree == NULL)
    return tree;

#if 0
  /* print the line          */
  /* if not block & function */
  if (tree->type == EX_OP && (tree->opval.op != FUNCTION && tree->opval.op != BLOCK && tree->opval.op != NULLOP))
    {
      filename = tree->filename;
      lineno = tree->lineno;
    }
#endif

  /* make sure we resolve the true & false labels for ifx */
  if (tree->type == EX_OP && tree->opval.op == IFX)
    {
      symbol *csym;

      if (tree->trueLabel)
        {
          if ((csym = findSym (LabelTab, tree->trueLabel, tree->trueLabel->name)))
            tree->trueLabel = csym;
          else
            werrorfl (tree->filename, tree->lineno, E_LABEL_UNDEF, tree->trueLabel->name);
        }

      if (tree->falseLabel)
        {
          if ((csym = findSym (LabelTab, tree->falseLabel, tree->falseLabel->name)))
            tree->falseLabel = csym;
          else
            werrorfl (tree->filename, tree->lineno, E_LABEL_UNDEF, tree->falseLabel->name);
        }
    }

  if (tree->type == EX_OP && tree->opval.op == FOR)
    {
      symbol *csym;

      if (AST_FOR (tree, trueLabel))
        {
          if ((csym = findSym (LabelTab, AST_FOR (tree, trueLabel), AST_FOR (tree, trueLabel)->name)))
            AST_FOR (tree, trueLabel) = csym;
        }
      if (AST_FOR (tree, falseLabel))
        {
          if ((csym = findSym (LabelTab, AST_FOR (tree, falseLabel), AST_FOR (tree, falseLabel)->name)))
            AST_FOR (tree, falseLabel) = csym;
        }
      if (AST_FOR (tree, continueLabel))
        {
          if ((csym = findSym (LabelTab, AST_FOR (tree, continueLabel), AST_FOR (tree, continueLabel)->name)))
            AST_FOR (tree, continueLabel) = csym;
        }
      if (AST_FOR (tree, condLabel))
        {
          if ((csym = findSym (LabelTab, AST_FOR (tree, condLabel), AST_FOR (tree, condLabel)->name)))
            AST_FOR (tree, condLabel) = csym;
        }
      AST_FOR (tree, initExpr) = resolveSymbols (AST_FOR (tree, initExpr));
      AST_FOR (tree, condExpr) = resolveSymbols (AST_FOR (tree, condExpr));
      AST_FOR (tree, loopExpr) = resolveSymbols (AST_FOR (tree, loopExpr));
    }

  /* if this is a label resolve it from the labelTab */
  if (IS_AST_VALUE (tree) && tree->opval.val->sym && tree->opval.val->sym->islbl)
    {
      symbol *csym = findSym (LabelTab, tree->opval.val->sym,
                              tree->opval.val->sym->name);

      if (!csym)
        werrorfl (tree->filename, tree->lineno, E_LABEL_UNDEF, tree->opval.val->sym->name);
      else
        tree->opval.val->sym = csym;

      goto resolveChildren;
    }

  /* do only for leafs */
  if (IS_AST_VALUE (tree) && tree->opval.val->sym && !tree->opval.val->sym->implicit)
    {
      symbol *csym = findSymWithLevel (SymbolTab, tree->opval.val->sym);

      /* if found in the symbol table & they are not the same */
      if (csym && tree->opval.val->sym != csym)
        {
          tree->opval.val->sym = csym;
          tree->opval.val->type = csym->type;
          tree->opval.val->etype = csym->etype;
          if (*csym->rname)
            {
              SNPRINTF (tree->opval.val->name, sizeof (tree->opval.val->name), "%s", csym->rname);
            }
          else
            {
              SNPRINTF (tree->opval.val->name, sizeof (tree->opval.val->name), "_%s", csym->name);
            }
        }

      /* if not found in the symbol table */
      /* mark it as undefined & assume it */
      /* is an integer in data space      */
      if (!csym && !tree->opval.val->sym->implicit && !tree->opval.val->type)
        {
          /* if this is a function name then */
          /* mark it as returning an int     */
          if (tree->funcName)
            {
              tree->opval.val->sym->type = newLink (DECLARATOR);
              DCL_TYPE (tree->opval.val->sym->type) = FUNCTION;
              tree->opval.val->sym->type->next = tree->opval.val->sym->etype = newIntLink ();
              tree->opval.val->etype = tree->opval.val->etype;
              tree->opval.val->type = tree->opval.val->sym->type;
              werrorfl (tree->filename, tree->lineno, W_IMPLICIT_FUNC, tree->opval.val->sym->name);
              //tree->opval.val->sym->undefined = 1;
              allocVariables (tree->opval.val->sym);
            }
          else
            {
              tree->opval.val->sym->undefined = 1;
              tree->opval.val->type = tree->opval.val->etype = newIntLink ();
              tree->opval.val->sym->type = tree->opval.val->sym->etype = newIntLink ();
            }
        }
    }

    /* If entering a block with symbols defined, mark the symbols in-scope */
    /* before continuing down the tree, and mark them out-of-scope again   */
    /* on the way back up */ 
    if (tree->type == EX_OP && tree->opval.op == BLOCK && tree->values.sym)
      {
        symbol * sym = tree->values.sym;
        while (sym)
          {
            sym->isinscope = 1;
            sym = sym->next;
          }
        resolveSymbols (tree->left);
        resolveSymbols (tree->right);
        sym = tree->values.sym;
        while (sym)
          {
            sym->isinscope = 0;
            sym = sym->next;
          }
        return tree;
      }
      
resolveChildren:
  resolveSymbols (tree->left);
  resolveSymbols (tree->right);

  return tree;
}

/*------------------------------------------------------------------------*/
/* setAstFileLine - walks a ast tree & sets the file name and line number */
/*------------------------------------------------------------------------*/
int
setAstFileLine (ast * tree, char *filename, int lineno)
{
  if (!tree)
    return 0;

  tree->filename = filename;
  tree->lineno = lineno;
  setAstFileLine (tree->left, filename, lineno);
  setAstFileLine (tree->right, filename, lineno);
  return 0;
}

/*-----------------------------------------------------------------*/
/* funcOfType :- function of type with name                        */
/*-----------------------------------------------------------------*/
symbol *
funcOfType (const char *name, sym_link * type, sym_link * argType, int nArgs, int rent)
{
  symbol *sym;
  /* create the symbol */
  sym = newSymbol (name, 0);

  /* setup return value */
  sym->type = newLink (DECLARATOR);
  DCL_TYPE (sym->type) = FUNCTION;
  sym->type->next = copyLinkChain (type);
  sym->etype = getSpec (sym->type);
  FUNC_ISREENT (sym->type) = rent ? 1 : 0;
  FUNC_NONBANKED (sym->type) = 1;

  /* if arguments required */
  if (nArgs)
    {
      value *args;
      args = FUNC_ARGS (sym->type) = newValue ();

      while (nArgs--)
        {
          args->type = copyLinkChain (argType);
          args->etype = getSpec (args->type);
          SPEC_EXTR (args->etype) = 1;
          if (!nArgs)
            break;
          args = args->next = newValue ();
        }
    }

  /* save it */
  addSymChain (&sym);
  sym->cdef = 1;
  allocVariables (sym);
  return sym;
}

/*-----------------------------------------------------------------*/
/* funcOfTypeVarg :- function of type with name and argtype        */
/*-----------------------------------------------------------------*/
symbol *
funcOfTypeVarg (const char *name, const char *rtype, int nArgs, const char **atypes)
{
  symbol *sym;
  int i;
  /* create the symbol */
  sym = newSymbol (name, 0);

  /* setup return value */
  sym->type = newLink (DECLARATOR);
  DCL_TYPE (sym->type) = FUNCTION;
  sym->type->next = typeFromStr (rtype);
  sym->etype = getSpec (sym->type);

  /* if arguments required */
  if (nArgs)
    {
      value *args;
      args = FUNC_ARGS (sym->type) = newValue ();

      for (i = 0; i < nArgs; i++)
        {
          args->type = typeFromStr (atypes[i]);
          args->etype = getSpec (args->type);
          SPEC_EXTR (args->etype) = 1;
          if ((i + 1) == nArgs)
            break;
          args = args->next = newValue ();
        }
    }

  /* save it */
  addSymChain (&sym);
  sym->cdef = 1;
  allocVariables (sym);
  return sym;
}

/*-----------------------------------------------------------------*/
/* reverseParms - will reverse a parameter tree                    */
/*-----------------------------------------------------------------*/
static void
reverseParms (ast * ptree, int r)
{
  ast *ttree;
  if (!ptree)
    return;

  /* top down if we find a nonParm tree then quit */
  if (ptree->type == EX_OP && ptree->opval.op == PARAM && ptree->reversed != r)
    {
      /* The various functions expect the parameter tree to be right heavy. */
      /* Rotate the tree to be left heavy so that after reversal it is */
      /* right heavy again. */
      while ((ttree = ptree->right) && ttree->type == EX_OP && ttree->opval.op == PARAM)
        {
          ptree->right = ttree->right;
          ttree->right = ttree->left;
          ttree->left = ptree->left;
          ptree->left = ttree;
        }

      /* Now reverse */
      ttree = ptree->left;
      ptree->left = ptree->right;
      ptree->right = ttree;
      ptree->reversed = r;
      reverseParms (ptree->left, r);
      reverseParms (ptree->right, r);
    }

  return;
}

/*-----------------------------------------------------------------*/
/* processParms  - makes sure the parameters are okay and do some  */
/*                 processing with them                            */
/*-----------------------------------------------------------------*/
static int
processParms (ast * func, value * defParm, ast ** actParm, int *parmNumber,     /* unused, although updated */
              bool rightmost)
{
  RESULT_TYPE resultType;
  sym_link *functype;

  /* if none of them exist */
  if (!defParm && !*actParm)
    return 0;

  if (defParm)
    {
      if (getenv ("DEBUG_SANITY"))
        {
          fprintf (stderr, "processParms: %s ", defParm->name);
        }
      /* make sure the type is complete and sane */
      checkTypeSanity (defParm->etype, defParm->name);
    }

  if (IS_FUNCPTR (func->ftype))
    functype = func->ftype->next;
  else
    functype = func->ftype;

  /* if defined parameters ended but actual parameters */
  /* exist and this is not defined as a variable arg   */
  if (!defParm && *actParm && !IFFUNC_HASVARARGS (functype))
    {
      werror (E_TOO_MANY_PARMS);
      return 1;
    }

  /* if defined parameters present but no actual parameters */
  if (defParm && !*actParm)
    {
      werror (E_TOO_FEW_PARMS);
      return 1;
    }

  /* if this is a PARAM node then match left & right */
  if (IS_AST_PARAM (*actParm))
    {
      (*actParm)->decorated = 1;
      if ((*actParm)->reversed)
        {
          return (processParms (func, defParm, &(*actParm)->right, parmNumber, FALSE) ||
                 processParms (func, defParm ? defParm->next : NULL, &(*actParm)->left, parmNumber, rightmost));
        }
      else
        {
          return (processParms (func, defParm, &(*actParm)->left, parmNumber, FALSE) ||
                 processParms (func, defParm ? defParm->next : NULL, &(*actParm)->right, parmNumber, rightmost));
        }
    }
  else if (defParm)             /* not vararg */
    {
      /* If we have found a value node by following only right-hand links,
       * then we know that there are no more values after us.
       *
       * Therefore, if there are more defined parameters, the caller didn't
       * supply enough.
       */
      if (rightmost && defParm->next)
        {
          werror (E_TOO_FEW_PARMS);
          return 1;
        }
    }

  /* decorate parameter */
  resultType = defParm ? getResultTypeFromType (defParm->type) : RESULT_TYPE_NONE;
  *actParm = decorateType (*actParm, resultType);

  if (IS_VOID ((*actParm)->ftype))
    {
      werror (E_VOID_VALUE_USED);
      return 1;
    }

  /* If this is a varargs function... */
  if (!defParm && *actParm && IFFUNC_HASVARARGS (functype))
    {
      ast *newType = NULL;
      sym_link *ftype;

      /* don't perform integer promotion of explicitly typecasted variable arguments
       * if sdcc extensions are enabled */
      if (options.std_sdcc && !TARGET_PDK_LIKE &&
          (IS_CAST_OP (*actParm) ||
           (IS_AST_SYM_VALUE (*actParm) && AST_VALUES (*actParm, cast.removedCast)) ||
           (IS_AST_LIT_VALUE (*actParm) && AST_VALUES (*actParm, cast.literalFromCast))))
        {
          /* Parameter was explicitly typecast; don't touch it. */
          return 0;
        }

      ftype = (*actParm)->ftype;

      resultType = RESULT_TYPE_NONE;

      /* If it's a char, upcast to int. */
      if (IS_INTEGRAL (ftype) && (getSize (ftype) < (unsigned) INTSIZE))
        {
          newType = newAst_LINK (INTTYPE);
        }

      if (IS_PTR (ftype) && !IS_GENPTR (ftype))
        {
          newType = newAst_LINK (copyLinkChain (ftype));
          DCL_TYPE (newType->opval.lnk) = port->unqualified_pointer;
          resultType = RESULT_TYPE_GPTR;
        }

      if (IS_STRUCT (ftype))
        {
          werrorfl ((*actParm)->filename, (*actParm)->lineno, E_STRUCT_AS_ARG, (*actParm)->opval.val->name);
          return 1;
        }

      if (IS_ARRAY (ftype))
        {
          newType = newAst_LINK (copyLinkChain (ftype));
          DCL_TYPE (newType->opval.lnk) = port->unqualified_pointer;
          resultType = RESULT_TYPE_GPTR;
        }

      if (newType)
        {
          /* cast required; change this op to a cast. */
          (*actParm)->decorated = 0;
          *actParm = newNode (CAST, newType, *actParm);
          (*actParm)->filename = (*actParm)->right->filename;
          (*actParm)->lineno = (*actParm)->right->lineno;
          AST_VALUES (*actParm, cast.implicitCast) = 1;

          *actParm = decorateType (*actParm, resultType);
        }
      return 0;
    }                           /* vararg */

  /* if defined parameters ended but actual has not & */
  /* reentrant */
  if (!defParm && *actParm && (options.stackAuto || IFFUNC_ISREENT (functype)))
    {
      return 0;
    }
  resolveSymbols (*actParm);

  /* the parameter type must be at least castable */
  if (compareType (defParm->type, (*actParm)->ftype) == 0)
    {
      werror (E_INCOMPAT_TYPES);
      printFromToType ((*actParm)->ftype, defParm->type);
      return 1;
    }

  /* if the parameter is castable then add the cast */
  if ((IS_ARRAY((*actParm)->ftype) && IS_PTR(defParm->type)) ||
      (compareType (defParm->type, (*actParm)->ftype) == -1))
    {
      ast *pTree;

      resultType = getResultTypeFromType (defParm->type);
      pTree = resolveSymbols (copyAst (*actParm));

      /* now change the current one to a cast */
      *actParm = newNode (CAST, newAst_LINK (defParm->type), pTree);
      (*actParm)->filename = (*actParm)->right->filename;
      (*actParm)->lineno = (*actParm)->right->lineno;
      AST_VALUES (*actParm, cast.implicitCast) = 1;
      *actParm = decorateType (*actParm, IS_GENPTR (defParm->type) ? RESULT_TYPE_GPTR : resultType);
    }

  /* make a copy and change the regparm type to the defined parm */
  (*actParm)->etype = getSpec ((*actParm)->ftype = copyLinkChain ((*actParm)->ftype));
  SPEC_REGPARM ((*actParm)->etype) = SPEC_REGPARM (defParm->etype);
  SPEC_ARGREG ((*actParm)->etype) = SPEC_ARGREG (defParm->etype);

  /* if the function is being called via a pointer &  */
  /* this parameter is not passed in registers        */
  /* then the function must be defined reentrant      */
  if (IS_FUNCPTR (func->ftype) && !SPEC_REGPARM ((*actParm)->etype) && !IFFUNC_ISREENT (functype) && !options.stackAuto)
    {
      werror (E_NONRENT_ARGS);
      fatalError++;
      return 1;
    }

  (*parmNumber)++;
  return 0;
}

/*-----------------------------------------------------------------*/
/* createIvalType - generates ival for basic types                 */
/*-----------------------------------------------------------------*/
static ast *
createIvalType (ast * sym, sym_link * type, initList * ilist)
{
  ast *iExpr;

  /* if initList is deep */
  if (ilist && ilist->type == INIT_DEEP)
    ilist = ilist->init.deep;

  if (ilist)
    iExpr = decorateType (resolveSymbols (list2expr (ilist)), RESULT_TYPE_NONE);
  else
    iExpr = newAst_VALUE (valueFromLit (0));
  return decorateType (newNode ('=', sym, iExpr), RESULT_TYPE_NONE);
}

/*------------------------------------------------------------------*/
/* moveNestedInit - rewrites an initList node with a nested         */
/*                  designator to remove one level of nesting.      */
/*------------------------------------------------------------------*/
static initList *
moveNestedInit (initList *src)
{
  initList *dst;

  /** Create new initList element */
  switch (src->type)
    {
    case INIT_NODE:
      dst = newiList(INIT_NODE, src->init.node);
      break;
    case INIT_DEEP:
      dst = newiList(INIT_DEEP, src->init.deep);
      break;
    default:
      return NULL;
    }
  dst->filename = src->filename;
  dst->lineno = src->lineno;
  /* remove one level of nesting from the designation */
  dst->designation = src->designation->next;

  dst = newiList(INIT_DEEP, dst);
  dst->filename = src->filename;
  dst->lineno = src->lineno;
  dst->next = src->next;
  return dst;
}

/*-----------------------------------------------------------------*/
/* findStructField - find a specific field in a struct definition  */
/*-----------------------------------------------------------------*/
static symbol *
findStructField (symbol *fields, symbol *target)
{
  for ( ; fields; fields = fields->next)
    {
      if (strcmp(fields->name, target->name) == 0)
        return fields;
    }
  return NULL; /* not found */
}

static int aggregateIsAutoVar = 0;

/*-----------------------------------------------------------------*/
/* createIvalStruct - generates initial value for structures       */
/*-----------------------------------------------------------------*/
static ast *
createIvalStruct (ast * sym, sym_link * type, initList * ilist, ast * rootValue)
{
  ast *rast = NULL;
  ast *lAst;
  symbol *sflds, *old_sflds, *ps;
  initList *iloop;
  sym_link *etype = getSpec (type);

  if (ilist && ilist->type != INIT_DEEP)
    {
      werror (E_INIT_STRUCT, "");
      return NULL;
    }

  iloop = ilist ? ilist->init.deep : NULL;

  for (sflds = SPEC_STRUCT (type)->fields; ; sflds = sflds->next)
    {
      /* skip past unnamed bitfields */
      if (sflds && IS_BITFIELD (sflds->type) && SPEC_BUNNAMED (sflds->etype))
        continue;

      old_sflds = sflds;
      /* designated initializer? */
      if (iloop && iloop->designation)
        {
          if (iloop->designation->type != DESIGNATOR_STRUCT)
            {
              werrorfl (iloop->filename, iloop->lineno, E_BAD_DESIGNATOR);
            }
          else /* find this designated element */
            {
              sflds = findStructField(SPEC_STRUCT (type)->fields,
                                      iloop->designation->designator.tag);
              if (sflds)
                {
                  if (iloop->designation->next)
                    {
                      iloop = moveNestedInit(iloop);
                    }
                }
              else
                {
                  werrorfl (iloop->filename, iloop->lineno, E_NOT_MEMBER,
                            iloop->designation->designator.tag->name);
                  sflds = SPEC_STRUCT (type)->fields; /* fixup */
                }
            }
        }

      /* if we have come to end */
      if (!sflds)
        break;
      if (!iloop && (!AST_SYMBOL (rootValue)->islocal || SPEC_STAT (etype)))
        break;

      if (aggregateIsAutoVar && (SPEC_STRUCT (type)->type != UNION))
        for (ps = old_sflds; ps != sflds && ps != NULL; ps = ps->next)
          {
            ps->implicit = 1;
            lAst = newNode (PTR_OP, newNode ('&', sym, NULL), newAst_VALUE (symbolVal (ps)));
            lAst = decorateType (resolveSymbols (lAst), RESULT_TYPE_NONE);
            rast = decorateType (resolveSymbols (createIval (lAst, ps->type, NULL, rast, rootValue, 1)), RESULT_TYPE_NONE);          
          }

      /* initialize this field */
      sflds->implicit = 1;
      lAst = newNode (PTR_OP, newNode ('&', sym, NULL), newAst_VALUE (symbolVal (sflds)));
      lAst = decorateType (resolveSymbols (lAst), RESULT_TYPE_NONE);
      rast = decorateType (resolveSymbols (createIval (lAst, sflds->type, iloop, rast, rootValue, 1)), RESULT_TYPE_NONE);
      iloop = iloop ? iloop->next : NULL;

      /* Unions can only initialize a single field */
      if (SPEC_STRUCT (type)->type == UNION)
        break;
    }

  if (iloop)
    {
      if (IS_AST_VALUE (sym))
        werrorfl (sym->opval.val->sym->fileDef, sym->opval.val->sym->lineDef,
                  W_EXCESS_INITIALIZERS, "struct", sym->opval.val->sym->name);
      else
        werrorfl (sym->filename, sym->lineno, E_INIT_COUNT);
    }

  return rast;
}

/*-----------------------------------------------------------------*/
/* createIvalArray - generates code for array initialization       */
/*-----------------------------------------------------------------*/
static ast *
createIvalArray (ast * sym, sym_link * type, initList * ilist, ast * rootValue)
{
  ast *rast = NULL;
  initList *iloop;
  int lcnt = 0, size = 0, idx = 0;
  literalList *literalL;
  sym_link *etype = getSpec (type);

  /* take care of the special   case  */
  /* array of characters can be init  */
  /* by a string                      */
  /* char *p = "abc";                 */
  if ((IS_CHAR (type->next) || IS_INT (type->next) && IS_UNSIGNED (type->next)) &&
      ilist && ilist->type == INIT_NODE)
    if ((rast = createIvalCharPtr (sym,
                                   type,
                                   decorateType (resolveSymbols (list2expr (ilist)), RESULT_TYPE_NONE),
                                   rootValue)))
      return decorateType (resolveSymbols (rast), RESULT_TYPE_NONE);
  /* char *p = {"abc"}; */
  if ((IS_CHAR (type->next) || IS_INT (type->next) && IS_UNSIGNED (type->next)) &&
      ilist && ilist->type == INIT_DEEP && ilist->init.deep && ilist->init.deep->type == INIT_NODE)
    if ((rast = createIvalCharPtr (sym,
                                   type,
                                   decorateType (resolveSymbols (list2expr (ilist->init.deep)), RESULT_TYPE_NONE),
                                   rootValue)))
      return decorateType (resolveSymbols (rast), RESULT_TYPE_NONE);

  /* not the special case */
  if (ilist && ilist->type != INIT_DEEP)
    {
      werror (E_INIT_STRUCT, "");
      return NULL;
    }

  iloop = ilist ? ilist->init.deep : NULL;
  lcnt = DCL_ELEM (type);

  if (!iloop && (!lcnt || !DCL_ELEM (type) || !AST_SYMBOL (rootValue)->islocal || SPEC_STAT (etype)))
    {
      return NULL;
    }

  if (port->arrayInitializerSuppported && convertIListToConstList (reorderIlist(type,ilist), &literalL, lcnt))
    {
      ast *aSym;

      aSym = decorateType (resolveSymbols (sym), RESULT_TYPE_NONE);

      rast = newNode (ARRAYINIT, aSym, NULL);
      rast->values.constlist = literalL;

      // Make sure size is set to length of initializer list.
      size = getNelements (type, ilist);

      if (lcnt && size > lcnt)
        {
          // Array size was specified, and we have more initializers than needed.
          werrorfl (sym->opval.val->sym->fileDef, sym->opval.val->sym->lineDef,
                    W_EXCESS_INITIALIZERS, "array", sym->opval.val->sym->name);
        }
    }
  else
    {
      for (;;)
        {
          ast *aSym;
          if (!iloop && ((lcnt && size >= lcnt) || !DCL_ELEM (type) || !AST_SYMBOL (rootValue)->islocal))
            {
              break;
            }

          if (iloop && iloop->designation)
            {
              if (iloop->designation->type != DESIGNATOR_ARRAY)
                {
                  werrorfl (iloop->filename, iloop->lineno, E_BAD_DESIGNATOR);
                }
              else
                {
                  idx = iloop->designation->designator.elemno;
                  if (iloop->designation->next)
                    {
                      iloop = moveNestedInit(iloop);
                    }
                }
            }
          /* track array size based on the initializers seen */
          if (size <= idx)
            {
              size = idx + 1;
            }
          /* too many initializers? */
          if (iloop && (lcnt && size > lcnt))
            {
              // is this a better way? at least it won't crash
              char *name = (IS_AST_SYM_VALUE (sym)) ? AST_SYMBOL (sym)->name : "";
              werrorfl (iloop->filename, iloop->lineno, W_EXCESS_INITIALIZERS, "array", name);

              break;
            }

          aSym = newNode ('[', sym, newAst_VALUE (valueFromLit ((float) (idx))));
          aSym = decorateType (resolveSymbols (aSym), RESULT_TYPE_NONE);
          rast = createIval (aSym, type->next, iloop, rast, rootValue, 0);
          idx++;
          iloop = (iloop ? iloop->next : NULL);
        }
    }

  /* if we have not been given a size  */
  if (!DCL_ELEM (type))
    {
      /* check, if it's a flexible array */
      if (IS_STRUCT (AST_VALUE (rootValue)->type))
        AST_SYMBOL (rootValue)->flexArrayLength = size * getSize (type->next);
      else
        DCL_ELEM (type) = size;
    }

  return decorateType (resolveSymbols (rast), RESULT_TYPE_NONE);
}

/*-----------------------------------------------------------------*/
/* createIvalCharPtr - generates initial values for char pointers  */
/*-----------------------------------------------------------------*/
static ast *
createIvalCharPtr (ast * sym, sym_link * type, ast * iexpr, ast * rootVal)
{
  ast *rast = NULL;
  unsigned size = 0;

  /* if this is a pointer & right is a literal array then */
  /* just assignment will do                              */
  if (IS_PTR (type) && ((IS_LITERAL (iexpr->etype) || (IS_SPEC (iexpr->etype) && SPEC_SCLS (iexpr->etype) == S_CODE)) && IS_ARRAY (iexpr->ftype)))
    return newNode ('=', sym, iexpr);

  /* left side is an array so we have to assign each element */
  if (!iexpr)
    {
      /* for each character generate an assignment */
      /* to the array element */
      unsigned int i = 0;
      unsigned int symsize = DCL_ELEM (type);

      if (!AST_SYMBOL (rootVal)->islocal || SPEC_STAT (getSpec (type)))
        return NULL;

      for (i = 0; i < symsize; ++i)
        {
          rast = newNode (NULLOP,
                          rast,
                          newNode ('=',
                                   newNode ('[', sym,
                                            newAst_VALUE (valueFromLit ((float) i))), newAst_VALUE (valueFromLit (0))));
        }

      return decorateType (resolveSymbols (rast), RESULT_TYPE_NONE);
    }

  if ((IS_LITERAL (iexpr->etype) || (IS_SPEC (iexpr->etype) && SPEC_SCLS (iexpr->etype) == S_CODE)) && IS_ARRAY (iexpr->ftype))
    {
      /* for each character generate an assignment */
      /* to the array element */
      unsigned int i = 0;
      unsigned int symsize = DCL_ELEM (type);

      size = DCL_ELEM (iexpr->ftype);
      if (symsize && size > symsize)
        {
          if (size > symsize)
            {
              char *name = (IS_AST_SYM_VALUE (sym)) ? AST_SYMBOL (sym)->name : "";

              TYPE_TARGET_ULONG c;
              if (IS_CHAR (type->next))
                c = SPEC_CVAL (iexpr->etype).v_char[symsize];
              else if (!IS_LONG (type->next))
                c = SPEC_CVAL (iexpr->etype).v_char16[symsize];
              else
                c = SPEC_CVAL (iexpr->etype).v_char32[symsize];
              if (options.std_c99 && c == '\0' && size == symsize + 1)
                { 
                  if (!options.lessPedantic)
                    werrorfl (iexpr->filename, iexpr->lineno, W_STRING_CANNOT_BE_TERMINATED, name);
                }
              else
                werrorfl (iexpr->filename, iexpr->lineno, W_EXCESS_INITIALIZERS, "string", name);
            }
          size = symsize;
        }

      for (i = 0; i < size; i++)
        {
          TYPE_TARGET_ULONG c;
          if (IS_CHAR (type->next))
            c = SPEC_CVAL (iexpr->etype).v_char[i];
          else if (!IS_LONG (type->next))
            c = SPEC_CVAL (iexpr->etype).v_char16[i];
          else
            c = SPEC_CVAL (iexpr->etype).v_char32[i];
          rast = newNode (NULLOP,
                          rast,
                          newNode ('=',
                                   newNode ('[', sym,
                                            newAst_VALUE (valueFromLit ((float) i))), newAst_VALUE (valueFromLit (c))));
        }

      /* assign zero to others */
      for (i = size; i < symsize; i++)
        {
          rast = newNode (NULLOP,
                          rast,
                          newNode ('=',
                                   newNode ('[', sym,
                                            newAst_VALUE (valueFromLit ((float) i))), newAst_VALUE (valueFromLit (0))));
        }

      // now WE don't need iexpr's symbol anymore
      freeStringSymbol (AST_SYMBOL (iexpr));

      /* if we have not been given a size  */
      if (!DCL_ELEM (type))
        {
          /* check, if it's a flexible array */
          if (IS_STRUCT (AST_VALUE (rootVal)->type))
            AST_SYMBOL (rootVal)->flexArrayLength = size * getSize (type->next);
          else
            DCL_ELEM (type) = size;
        }

      return decorateType (resolveSymbols (rast), RESULT_TYPE_NONE);
    }

  return NULL;
}

/*-----------------------------------------------------------------*/
/* createIvalPtr - generates initial value for pointers            */
/*-----------------------------------------------------------------*/
static ast *
createIvalPtr (ast * sym, sym_link * type, initList * ilist, ast * rootVal)
{
  ast *rast;
  ast *iexpr;

  /* if deep then   */
  if (ilist && ilist->type == INIT_DEEP)
    ilist = ilist->init.deep;

  if (ilist)
    iexpr = decorateType (resolveSymbols (list2expr (ilist)), RESULT_TYPE_NONE);
  else
    iexpr = newAst_VALUE (valueFromLit (0));

  /* if character pointer */
  if (IS_CHAR (type->next) || IS_INT (type->next) && IS_UNSIGNED (type->next))
    if ((rast = createIvalCharPtr (sym, type, iexpr, rootVal)))
      return rast;

  return newNode ('=', sym, iexpr);
}

/*-----------------------------------------------------------------*/
/* createIval - generates code for initial value                   */
/*-----------------------------------------------------------------*/
static ast *
createIval (ast * sym, sym_link * type, initList * ilist, ast * wid, ast * rootValue, int omitStatic)
{
  ast *rast = NULL;

  if (!ilist && (!AST_SYMBOL (rootValue)->islocal || (omitStatic && SPEC_STAT (getSpec (type)))))
    return NULL;

  /* if structure then    */
  if (IS_STRUCT (type))
    rast = createIvalStruct (sym, type, ilist, rootValue);
  else
    /* if this is a pointer */
  if (IS_PTR (type))
    rast = createIvalPtr (sym, type, ilist, rootValue);
  else
    /* if this is an array   */
  if (IS_ARRAY (type))
    rast = createIvalArray (sym, type, ilist, rootValue);
  else
    /* if type is SPECIFIER */
  if (IS_SPEC (type))
    rast = createIvalType (sym, type, ilist);

  if (wid)
    return decorateType (resolveSymbols (newNode (NULLOP, wid, rast)), RESULT_TYPE_NONE);
  else
    return decorateType (resolveSymbols (rast), RESULT_TYPE_NONE);
}

/*-----------------------------------------------------------------*/
/* initAggregates - initialises aggregate variables with initv     */
/*-----------------------------------------------------------------*/
ast *
initAggregates (symbol *sym, initList *ival, ast *wid)
{
  ast *newAst = newAst_VALUE (symbolVal (sym));
  return createIval (newAst, sym->type, ival, wid, newAst, 1);
}

/*-----------------------------------------------------------------*/
/* gatherAutoInit - creates assignment expressions for initial     */
/*                  values                                         */
/*-----------------------------------------------------------------*/
static ast *
gatherAutoInit (symbol * autoChain)
{
  ast *init = NULL;
  ast *work;
  symbol *sym;

  inInitMode = 1;
  for (sym = autoChain; sym; sym = sym->next)
    {
      /* resolve the symbols in the ival */
      if (sym->ival)
        resolveIvalSym (sym->ival, sym->type);

#if 1
      /* if we are PIC14 or PIC16 port,
       * and this is a static,
       * and have initial value,
       * and not S_CODE, don't emit in gs segment,
       * but allow glue.c:pic16emitRegularMap to put symbol
       * in idata section */
      if (TARGET_PIC_LIKE && IS_STATIC (sym->etype) && sym->ival && SPEC_SCLS (sym->etype) != S_CODE)
        {
          SPEC_SCLS (sym->etype) = S_DATA;
          continue;
        }
#endif

      /* if this is a static variable & has an */
      /* initial value the code needs to be lifted */
      /* here to the main portion since they can be */
      /* initialised only once at the start    */
      if (IS_STATIC (sym->etype) && sym->ival && SPEC_SCLS (sym->etype) != S_CODE)
        {
          symbol *newSym;

          /* insert the symbol into the symbol table */
          /* with level = 0 & name = rname       */
          newSym = copySymbol (sym);
          addSym (SymbolTab, newSym, newSym->rname, 0, 0, 1);

          /* now lift the code to main */
          if (IS_AGGREGATE (sym->type))
            {
              work = initAggregates (sym, sym->ival, NULL);
            }
          else
            {
              if (getNelements (sym->type, sym->ival) > 1)
                {
                  werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "scalar", sym->name);
                }
              work = newNode ('=', newAst_VALUE (symbolVal (newSym)), list2expr (sym->ival));
            }

          setAstFileLine (work, sym->fileDef, sym->lineDef);

          sym->ival = NULL;
          if (staticAutos)
            staticAutos = newNode (NULLOP, staticAutos, work);
          else
            staticAutos = work;

          continue;
        }

      /* if there is an initial value */
      if (sym->ival && SPEC_SCLS (sym->etype) != S_CODE)
        {
          initList *ilist = sym->ival;

          while (ilist->type == INIT_DEEP)
            {
              ilist = ilist->init.deep;
            }

          /* update lineno for error msg */
          filename = sym->fileDef;
          lineno = sym->lineDef;
          setAstFileLine (ilist->init.node, sym->fileDef, sym->lineDef);

          if (IS_AGGREGATE (sym->type))
            {
              aggregateIsAutoVar = 1;
              work = initAggregates (sym, sym->ival, NULL);
              aggregateIsAutoVar = 0;
            }
          else
            {
              if (getNelements (sym->type, sym->ival) > 1)
                {
                  werrorfl (sym->fileDef, sym->lineDef, W_EXCESS_INITIALIZERS, "scalar", sym->name);
                }
              work = newNode ('=', newAst_VALUE (symbolVal (sym)), list2expr (sym->ival));
            }

          // just to be sure
          setAstFileLine (work, sym->fileDef, sym->lineDef);

          sym->ival = NULL;
          if (init)
            init = newNode (NULLOP, init, work);
          else
            init = work;
        }
    }
  inInitMode = 0;
  return init;
}

/*-----------------------------------------------------------------*/
/* freeStringSymbol - delete a literal string if no more usage     */
/*-----------------------------------------------------------------*/
void
freeStringSymbol (symbol *sym)
{
  /* make sure this is a literal string */
  assert (sym->isstrlit);
  if (--sym->isstrlit == 0)
    {                           // lower the usage count
      memmap *segment = SPEC_OCLS (sym->etype);
      if (segment)
        {
          deleteSetItem (&segment->syms, sym);
        }
    }
}

/*-----------------------------------------------------------------*/
/* stringToSymbol - creates a symbol from a literal string         */
/*-----------------------------------------------------------------*/
value *
stringToSymbol (value *val)
{
  struct dbuf_s dbuf;
  static int charLbl = 0;
  symbol *sym;
  set *sp;
  unsigned int size;

  // have we heard this before?
  for (sp = statsg->syms; sp; sp = sp->next)
    {
      sym = sp->item;
      size = getSize (sym->type);
      if (sym->isstrlit && size == getSize (val->type) &&
          !memcmp (SPEC_CVAL (sym->etype).v_char, SPEC_CVAL (val->etype).v_char, size))
        {
          // yes, this is old news. Don't publish it again.
          sym->isstrlit++;      // but raise the usage count
          return symbolVal (sym);
        }
    }

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "__str_%d", charLbl++);
  sym = newSymbol (dbuf_c_str (&dbuf), 0);      /* make it @ level 0 */
  strncpyz (sym->rname, dbuf_c_str (&dbuf), SDCC_NAME_MAX);
  dbuf_destroy (&dbuf);

  /* copy the type from the value passed */
  sym->type = copyLinkChain (val->type);
  sym->etype = getSpec (sym->type);
  /* change to storage class & output class */
  SPEC_SCLS (sym->etype) = S_CODE;
  SPEC_CVAL (sym->etype).v_char = SPEC_CVAL (val->etype).v_char;
  SPEC_STAT (sym->etype) = 1;
  /* make the level & block = 0 */
  sym->block = sym->level = 0;
  sym->isstrlit = 1;
  /* create an ival */
  sym->ival = newiList (INIT_NODE, newAst_VALUE (val));
  if (noAlloc == 0)
    {
      /* allocate it */
      addSymChain (&sym);
      allocVariables (sym);
    }
  else
    {
      defaultOClass (sym);
      addSet (&strSym, sym);
      addSet (&statsg->syms, sym);
    }
  sym->ival = NULL;
  return symbolVal (sym);
}

/*-----------------------------------------------------------------*/
/* processBlockVars - will go thru the ast looking for block if    */
/*                    a block is found then will allocate the syms */
/*                    will also gather the auto inits present      */
/*-----------------------------------------------------------------*/
ast *
processBlockVars (ast * tree, int *stack, int action)
{
  if (!tree)
    return NULL;

  /* if this is a block */
  if (tree->type == EX_OP && tree->opval.op == BLOCK)
    {
      ast *autoInit;
      symbol * sym = tree->values.sym;

      while (sym)
        {
          sym->isinscope = 1;
          sym = sym->next;
        }

      if (action == ALLOCATE)
        {
          *stack += allocVariables (tree->values.sym);
          autoInit = gatherAutoInit (tree->values.sym);

          /* if there are auto inits then do them */
          if (autoInit)
            tree->left = newNode (NULLOP, autoInit, tree->left);
        }
      else                      /* action is deallocate */
        {
          deallocLocal (tree->values.sym);
        }
    }

  if (IS_FOR_STMT (tree))
    {
      processBlockVars (AST_FOR (tree, initExpr), stack, action);
      processBlockVars (AST_FOR (tree, condExpr), stack, action);
      processBlockVars (AST_FOR (tree, loopExpr), stack, action);
    }
  processBlockVars (tree->left, stack, action);
  processBlockVars (tree->right, stack, action);

  /* if this is a block */
  if (tree->type == EX_OP && tree->opval.op == BLOCK)
    {
      symbol * sym = tree->values.sym;

      while (sym)
        {
          sym->isinscope = 0;
          sym = sym->next;
        }
    }
    
  return tree;
}

/*-------------------------------------------------------------*/
/* constExprTree - returns TRUE if this tree is a constant     */
/*                 expression                                  */
/*-------------------------------------------------------------*/
bool
constExprTree (ast * cexpr)
{
  if (!cexpr)
    {
      return TRUE;
    }

  cexpr = decorateType (resolveSymbols (cexpr), RESULT_TYPE_NONE);

  switch (cexpr->type)
    {
    case EX_VALUE:
      if (IS_AST_LIT_VALUE (cexpr))
        {
          // this is a literal
          return TRUE;
        }
      if (IS_AST_SYM_VALUE (cexpr) && IS_FUNC (AST_SYMBOL (cexpr)->type))
        {
          // a function's address will never change
          return TRUE;
        }
      if (IS_AST_SYM_VALUE (cexpr) && IS_ARRAY (AST_SYMBOL (cexpr)->type))
        {
          // an array's address will never change
          return TRUE;
        }
      if (IS_AST_SYM_VALUE (cexpr) && !AST_SYMBOL (cexpr)->etype)
        {
          // the offset of a struct field will never change
          return TRUE;
        }
#if 0
      if (IS_AST_SYM_VALUE (cexpr) && IN_CODESPACE (SPEC_OCLS (AST_SYMBOL (cexpr)->etype)))
        {
          // a symbol in code space will never change
          // This is only for the 'char *s="hallo"' case and will have to leave
          //printf(" code space symbol");
          return TRUE;
        }
#endif
      return FALSE;
    case EX_LINK:
      wassertl (0, "unexpected link in expression tree");
      return FALSE;
    case EX_OP:
      if (cexpr->opval.op == ARRAYINIT)
        {
          // this is a list of literals
          return TRUE;
        }
      if (cexpr->opval.op == '=')
        {
          return constExprTree (cexpr->right);
        }
      if (cexpr->opval.op == CAST)
        {
          // cast ignored, maybe we should throw a warning here?
          return constExprTree (cexpr->right);
        }
      if (cexpr->opval.op == '&')
        {
          return TRUE;
        }
      if (cexpr->opval.op == CALL || cexpr->opval.op == PCALL)
        {
          return FALSE;
        }
      if (constExprTree (cexpr->left) && constExprTree (cexpr->right))
        {
          return TRUE;
        }
      return FALSE;
    case EX_OPERAND:
      return IS_CONSTANT (operandType (cexpr->opval.oprnd));
    }
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* constExprValue - returns the value of a constant expression     */
/*                  or NULL if it is not a constant expression     */
/*-----------------------------------------------------------------*/
value *
constExprValue (ast * cexpr, int check)
{
  cexpr = decorateType (resolveSymbols (cexpr), RESULT_TYPE_NONE);

  /* if this is not a constant then */
  if (!IS_LITERAL (cexpr->ftype))
    {
      /* then check if this is a literal array
         in code segment */
      if (SPEC_SCLS (cexpr->etype) == S_CODE && SPEC_CVAL (cexpr->etype).v_char && IS_ARRAY (cexpr->ftype))
        {
          value *val = valFromType (cexpr->ftype);
          SPEC_SCLS (val->etype) = S_LITERAL;
          val->sym = cexpr->opval.val->sym;
          val->sym->type = copyLinkChain (cexpr->ftype);
          val->sym->etype = getSpec (val->sym->type);
          strncpyz (val->name, cexpr->opval.val->sym->rname, SDCC_NAME_MAX);
          return val;
        }

      /* if we are casting a literal value then */
      if (IS_CAST_OP (cexpr) && IS_LITERAL (cexpr->right->ftype))
        {
          return valCastLiteral (cexpr->ftype, floatFromVal (cexpr->right->opval.val), (TYPE_TARGET_ULONGLONG) ullFromVal (cexpr->right->opval.val));
        }

      if (IS_AST_VALUE (cexpr))
        {
          return cexpr->opval.val;
        }

      if (check)
        werrorfl (cexpr->filename, cexpr->lineno, E_CONST_EXPECTED, "found expression");

      return NULL;
    }

  /* return the value */
  if (IS_AST_VALUE (cexpr))
    {
      return cexpr->opval.val;
    }
  return NULL;
}

/*-----------------------------------------------------------------*/
/* isLabelInAst - will return true if a given label is found       */
/*-----------------------------------------------------------------*/
bool
isLabelInAst (symbol * label, ast * tree)
{
  if (!tree || IS_AST_VALUE (tree) || IS_AST_LINK (tree))
    return FALSE;

  if (IS_AST_OP (tree) && tree->opval.op == LABEL && isSymbolEqual (AST_SYMBOL (tree->left), label))
    return TRUE;

  return isLabelInAst (label, tree->right) && isLabelInAst (label, tree->left);
}

/*-----------------------------------------------------------------*/
/* isLoopCountable - return true if the loop count can be          */
/* determined at compile time .                                    */
/*-----------------------------------------------------------------*/
static bool
isLoopCountable (ast * initExpr, ast * condExpr, ast * loopExpr, symbol ** sym, ast ** init, ast ** end)
{
  /* the loop is considered countable if the following
     conditions are true :-

     a) initExpr :- <sym> = <const>
     b) condExpr :- <sym> < <const1>
     c) loopExpr :- <sym> ++
   */

  /* first check the initExpr */
  if (IS_AST_OP (initExpr) && initExpr->opval.op == '=' &&      /* is assignment */
      IS_AST_SYM_VALUE (initExpr->left))
    {                           /* left is a symbol */

      *sym = AST_SYMBOL (initExpr->left);
      *init = initExpr->right;
    }
  else
    return FALSE;

  /* don't reverse loop with volatile counter */
  if (IS_VOLATILE ((*sym)->type))
    return FALSE;

  /* for now the symbol has to be of
     integral type */
  if (!IS_INTEGRAL ((*sym)->type))
    return FALSE;

  /* now check condExpr */
  if (IS_AST_OP (condExpr))
    {
      switch (condExpr->opval.op)
        {
        case '<':
          if (IS_AST_SYM_VALUE (condExpr->left) &&
              isSymbolEqual (*sym, AST_SYMBOL (condExpr->left)) && IS_AST_LIT_VALUE (condExpr->right))
            {
              *end = condExpr->right;
              break;
            }
          return FALSE;

        case '!':
          if (IS_AST_OP (condExpr->left) &&
              condExpr->left->opval.op == '>' &&
              IS_AST_LIT_VALUE (condExpr->left->right) &&
              IS_AST_SYM_VALUE (condExpr->left->left) && isSymbolEqual (*sym, AST_SYMBOL (condExpr->left->left)))
            {

              *end = newNode ('+', condExpr->left->right, newAst_VALUE (constVal ("1")));
              break;
            }
          return FALSE;

        default:
          return FALSE;
        }
    }
  else
    return FALSE;

  /* check loop expression is of the form <sym>++ */
  if (!IS_AST_OP (loopExpr))
    return FALSE;

  /* check if <sym> ++ */
  if (loopExpr->opval.op == INC_OP)
    {
      if (loopExpr->left)
        {
          /* pre */
          if (IS_AST_SYM_VALUE (loopExpr->left) && isSymbolEqual (*sym, AST_SYMBOL (loopExpr->left)))
            return TRUE;
        }
      else
        {
          /* post */
          if (IS_AST_SYM_VALUE (loopExpr->right) && isSymbolEqual (*sym, AST_SYMBOL (loopExpr->right)))
            return TRUE;
        }
    }
  else
    {
      /* check for += */
      if (loopExpr->opval.op == ADD_ASSIGN)     /* seems to never happen, createRMW() absorbed */
        {
          wassertl (0, "obsolete opcode in tree");

          if (IS_AST_SYM_VALUE (loopExpr->left) &&
              isSymbolEqual (*sym, AST_SYMBOL (loopExpr->left)) &&
              IS_AST_LIT_VALUE (loopExpr->right) && AST_ULONG_VALUE (loopExpr->right) == 1)
            return TRUE;
        }
    }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* astHasVolatile - returns true if ast contains any volatile      */
/*-----------------------------------------------------------------*/
bool
astHasVolatile (ast *tree)
{
  if (!tree)
    return FALSE;

  if (TETYPE (tree) && IS_VOLATILE (TETYPE (tree)))
    return TRUE;

  if (IS_AST_OP (tree))
    return astHasVolatile (tree->left) || astHasVolatile (tree->right);
  else
    return FALSE;
}

/*-----------------------------------------------------------------*/
/* astHasPointer - return true if the ast contains any ptr variable */
/*-----------------------------------------------------------------*/
bool
astHasPointer (ast * tree)
{
  if (!tree)
    return FALSE;

  if (IS_AST_LINK (tree))
    return TRUE;

  /* if we hit an array expression then check
     only the left side */
  if (IS_AST_OP (tree) && tree->opval.op == '[')
    return astHasPointer (tree->left);

  if (IS_AST_VALUE (tree))
    return IS_PTR (tree->ftype) || IS_ARRAY (tree->ftype);

  return astHasPointer (tree->left) || astHasPointer (tree->right);
}

/*-----------------------------------------------------------------*/
/* astHasSymbol - return true if the ast has the given symbol      */
/*-----------------------------------------------------------------*/
bool
astHasSymbol (ast * tree, symbol * sym)
{
  if (!tree || IS_AST_LINK (tree))
    return FALSE;

  if (IS_AST_VALUE (tree))
    {
      if (IS_AST_SYM_VALUE (tree))
        return isSymbolEqual (AST_SYMBOL (tree), sym);
      else
        return FALSE;
    }

  return astHasSymbol (tree->left, sym) || astHasSymbol (tree->right, sym);
}

/*-----------------------------------------------------------------*/
/* astHasDeref - return true if the ast has an indirect access     */
/*-----------------------------------------------------------------*/
static bool
astHasDeref (ast * tree)
{
  if (!tree || IS_AST_LINK (tree) || IS_AST_VALUE (tree))
    return FALSE;

  if (tree->opval.op == '*' && tree->right == NULL)
    return TRUE;

  return astHasDeref (tree->left) || astHasDeref (tree->right);
}

/*-----------------------------------------------------------------*/
/* isConformingBody - the loop body has to conform to a set of     */
/* rules for the loop to be considered reversible read on for rules*/
/*-----------------------------------------------------------------*/
bool
isConformingBody (ast * pbody, symbol * sym, ast * body)
{
  /* we are going to do a pre-order traversal of the
     tree && check for the following conditions. (essentially
     a set of very shallow tests )
     a) the sym passed does not participate in any arithmetic operation
     b) There are no function calls
     c) all jumps are within the body
     d) address of loop control variable not taken
     e) if an assignment has a pointer on the left hand side make sure
     right does not have loop control variable
   */

  /* if we reach the end or a leaf then true */
  if (!pbody || IS_AST_LINK (pbody) || IS_AST_VALUE (pbody))
    return TRUE;

  /* if anything else is "volatile" */
  if (IS_VOLATILE (TETYPE (pbody)))
    return FALSE;

  /* we will walk the body in a pre-order traversal for
     efficiency sake */
  switch (pbody->opval.op)
    {
/*------------------------------------------------------------------*/
    case '[':
      /* if the loopvar is used as an index */
      /* array op is commutative -- must check both left & right */
      if (astHasSymbol (pbody->right, sym) || astHasSymbol (pbody->left, sym))
        {
          return FALSE;
        }
      return isConformingBody (pbody->right, sym, body) && isConformingBody (pbody->left, sym, body);

/*------------------------------------------------------------------*/
    case PTR_OP:
      /* '->' right: is a symbol
         left: check if the loopvar is used as an index */
      if (astHasSymbol (pbody->left, sym))
        return FALSE;
      return isConformingBody (pbody->left, sym, body);

    case '.':
      wassertl (0, "obsolete opcode in tree");
      break;

/*------------------------------------------------------------------*/
    case INC_OP:
    case DEC_OP:

      if (astHasSymbol (pbody->right, sym) || astHasSymbol (pbody->left, sym))
        return FALSE;

      return isConformingBody (pbody->right, sym, body) && isConformingBody (pbody->left, sym, body);

/*------------------------------------------------------------------*/

    case '*':                  /* can be unary  : if right is null then unary operation */
    case '+':
    case '-':
    case '&':

      /* if right is NULL then unary operation  */
/*------------------------------------------------------------------*/
      /*----------------------------*/
      /*  address of                */
      /*----------------------------*/
      if (!pbody->right)
        {
          if (IS_AST_SYM_VALUE (pbody->left) && isSymbolEqual (AST_SYMBOL (pbody->left), sym))
            return FALSE;
          else
            return isConformingBody (pbody->left, sym, body);
        }
      else
        {
          if (astHasSymbol (pbody->left, sym) || astHasSymbol (pbody->right, sym))
            return FALSE;
        }

/*------------------------------------------------------------------*/
    case '|':
    case '^':
    case '/':
    case '%':
    case LEFT_OP:
    case RIGHT_OP:
    case GETABIT:
    case GETBYTE:
    case GETWORD:

      if (IS_AST_SYM_VALUE (pbody->left) && isSymbolEqual (AST_SYMBOL (pbody->left), sym))
        return FALSE;

      if (IS_AST_SYM_VALUE (pbody->right) && isSymbolEqual (AST_SYMBOL (pbody->right), sym))
        return FALSE;

      return isConformingBody (pbody->left, sym, body) && isConformingBody (pbody->right, sym, body);

    case '~':
    case '!':
    case RRC:
    case RLC:
    case GETHBIT:
    case SWAP:
      if (IS_AST_SYM_VALUE (pbody->left) && isSymbolEqual (AST_SYMBOL (pbody->left), sym))
        return FALSE;
      return isConformingBody (pbody->left, sym, body);

/*------------------------------------------------------------------*/

    case AND_OP:
    case OR_OP:
    case '>':
    case '<':
    case LE_OP:
    case GE_OP:
    case EQ_OP:
    case NE_OP:
    case '?':
    case ':':
    case SIZEOF:               /* evaluate without code generation */

      if (IS_AST_SYM_VALUE (pbody->left) && isSymbolEqual (AST_SYMBOL (pbody->left), sym))
        return FALSE;

      if (IS_AST_SYM_VALUE (pbody->right) && isSymbolEqual (AST_SYMBOL (pbody->right), sym))
        return FALSE;

      return isConformingBody (pbody->left, sym, body) && isConformingBody (pbody->right, sym, body);

/*------------------------------------------------------------------*/
    case '=':

      /* if left has a pointer & right has loop
         control variable then we cannot */
      if (astHasPointer (pbody->left) && astHasSymbol (pbody->right, sym))
        return FALSE;

      if (astHasVolatile (pbody->left))
        return FALSE;

      if (IS_AST_SYM_VALUE (pbody->left))
        {
          // if the loopvar has an assignment
          if (isSymbolEqual (AST_SYMBOL (pbody->left), sym))
            return FALSE;
          // if the loopvar is used in another (maybe conditional) block
          if (astHasSymbol (pbody->right, sym) && (pbody->level >= body->level))
            {
              return FALSE;
            }
        }

      if (astHasDeref (pbody->right))
        return FALSE;

      return isConformingBody (pbody->left, sym, body) && isConformingBody (pbody->right, sym, body);

    case MUL_ASSIGN:
    case DIV_ASSIGN:
    case AND_ASSIGN:
    case OR_ASSIGN:
    case XOR_ASSIGN:
    case RIGHT_ASSIGN:
    case LEFT_ASSIGN:
    case SUB_ASSIGN:
    case ADD_ASSIGN:
      assert ("Parser should not have generated this\n");

/*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      comma operator        */
      /*----------------------------*/
    case ',':
      return isConformingBody (pbody->left, sym, body) && isConformingBody (pbody->right, sym, body);

/*------------------------------------------------------------------*/
      /*----------------------------*/
      /*       function call        */
      /*----------------------------*/
    case CALL:
      /* if local & no parameters &
         not used to find the function then ok */
      if (sym->level && !pbody->right && !astHasSymbol (pbody->left, sym))
        {
          return TRUE;
        }
      return FALSE;

/*------------------------------------------------------------------*/
      /*----------------------------*/
      /*     return statement       */
      /*----------------------------*/
    case RETURN:
      return FALSE;

    case GOTO:
      if (isLabelInAst (AST_SYMBOL (pbody->left), body))
        return TRUE;
      else
        return FALSE;

    case SWITCH:
      if (astHasSymbol (pbody->left, sym))
        return FALSE;
      break;

    default:
      break;
    }

  return isConformingBody (pbody->left, sym, body) && isConformingBody (pbody->right, sym, body);
}

/*-----------------------------------------------------------------*/
/* isLoopReversible - takes a for loop as input && returns true    */
/* if the for loop is reversible. If yes will set the value of     */
/* the loop control var & init value & termination value           */
/*-----------------------------------------------------------------*/
static bool
isLoopReversible (ast * loop, symbol ** loopCntrl, ast ** init, ast ** end)
{
  /* if option says don't do it then don't */
  if (optimize.noLoopReverse)
    return 0;
  /* there are several tests to determine this */

  /* for loop has to be of the form
     for ( <sym> = <const1> ;
     [<sym> < <const2>]  ;
     [<sym>++] | [<sym> += 1] | [<sym> = <sym> + 1] )
     forBody */
  if (!isLoopCountable (AST_FOR (loop, initExpr), AST_FOR (loop, condExpr), AST_FOR (loop, loopExpr), loopCntrl, init, end))
    return 0;

  /* now do some serious checking on the body of the loop
   */

  return isConformingBody (loop->left, *loopCntrl, loop->left);
}

/*-----------------------------------------------------------------*/
/* replLoopSym - replace the loop sym by loop sym -1               */
/*-----------------------------------------------------------------*/
static void
replLoopSym (ast * body, symbol * sym)
{
  /* reached end */
  if (!body || IS_AST_LINK (body))
    return;

  if (IS_AST_SYM_VALUE (body))
    {
      if (isSymbolEqual (AST_SYMBOL (body), sym))
        {

          body->type = EX_OP;
          body->opval.op = '-';
          body->left = newAst_VALUE (symbolVal (sym));
          body->right = newAst_VALUE (constVal ("1"));
        }
      return;
    }

  replLoopSym (body->left, sym);
  replLoopSym (body->right, sym);
}

/*-----------------------------------------------------------------*/
/* reverseLoop - do the actual loop reversal                       */
/*-----------------------------------------------------------------*/
ast *
reverseLoop (ast * loop, symbol * sym, ast * init, ast * end)
{
  ast *rloop;

  /* create the following tree
     <sym> = loopCount ;
     for_continue:
     forbody
     <sym> -= 1;
     if (sym) goto for_continue ;
     <sym> = end */

  /* put it together piece by piece */
  rloop = newNode (NULLOP,
                   createIf (newAst_VALUE (symbolVal (sym)),
                             newNode (GOTO,
                                      newAst_VALUE (symbolVal (AST_FOR (loop, continueLabel))),
                                      NULL), NULL), newNode ('=', newAst_VALUE (symbolVal (sym)), end));

  replLoopSym (loop->left, sym);
  setAstFileLine (rloop, init->filename, init->lineno);

  rloop = newNode (NULLOP,
                   newNode ('=',
                            newAst_VALUE (symbolVal (sym)),
                            newNode ('-', end, init)),
                   createLabel (AST_FOR (loop, continueLabel),
                                newNode (NULLOP,
                                         loop->left,
                                         newNode (NULLOP,
                                                  newNode (SUB_ASSIGN,
                                                           newAst_VALUE (symbolVal (sym)),
                                                           newAst_VALUE (constVal ("1"))), rloop))));

  rloop->lineno = init->lineno;
  return decorateType (rloop, RESULT_TYPE_NONE);
}

/*-----------------------------------------------------------------*/
/* replLoopSymByVal - replace the loop sym by a value              */
/*-----------------------------------------------------------------*/
static int
replLoopSymByVal (ast * body, symbol * sym, value * val)
{
  int changed;
  /* reached end */
  if (!body || IS_AST_LINK (body))
    return 0;

  if (IS_AST_SYM_VALUE (body))
    {
      if (isSymbolEqual (AST_SYMBOL (body), sym))
        {

          body->type = EX_VALUE;
          AST_VALUE (body) = copyValue (val);
          body->decorated = 0;
          return 1;
        }
      return 0;
    }

  changed = replLoopSymByVal (body->left, sym, val);
  changed |= replLoopSymByVal (body->right, sym, val);
  if (changed)
    body->decorated = 0;
  return changed;
}

/*-----------------------------------------------------------------*/
/* isInitiallyTrue - check if a for loop's condition is true for   */
/*                   the initial iteration                         */
/*-----------------------------------------------------------------*/
static bool
isInitiallyTrue (ast *initExpr, ast * condExpr)
{
  symbol * sym;
  ast * init;

  if (!condExpr)
    return TRUE;
  if (!initExpr)
    return FALSE;
  
  /* first check the initExpr */
  if (IS_AST_OP (initExpr) && initExpr->opval.op == '=' &&      /* is assignment */
      IS_AST_SYM_VALUE (initExpr->left))
    {                           /* left is a symbol */

      sym = AST_SYMBOL (initExpr->left);
      init = initExpr->right;
    }
  else
    return FALSE;

  /* don't defer condition test if volatile */
  if (IS_VOLATILE ((sym)->type))
    return FALSE;

  if (!IS_AST_LIT_VALUE (init))
    return FALSE;

  /* Cannot move the condition if the condition has side-effects */
  if (hasSEFcalls (condExpr))
    return FALSE;

  /* Cast the initial value to the type of the loop symbol so that  */
  /* we have the actual value that we would read out of that symbol */
  /* rather than just the value assigned to it. */
  initExpr = copyAst (initExpr);
  initExpr->opval.op = CAST;
  initExpr->left = newAst_LINK (LTYPE (initExpr));
  initExpr->decorated = 0;
  decorateType (initExpr, RESULT_TYPE_NONE);
  if (!IS_AST_LIT_VALUE (initExpr))
    return FALSE;

  /* Replace the symbol with its initial value and see if the condition */
  /* simplifies to a non-zero (TRUE) literal value */      
  condExpr = copyAst (condExpr);
  if (replLoopSymByVal (condExpr, sym, AST_VALUE (initExpr)))
    {
      int original;

      /* We are speculating that the condition is always true */
      /* or false for the first loop iteration; no need to    */
      /* trigger a warning that may not apply to the original */
      /* source code. */
      original = setWarningDisabledState (W_COMP_RANGE, TRUE);
      condExpr = decorateType (condExpr, RESULT_TYPE_NONE);
      setWarningDisabledState (W_COMP_RANGE, original);
    }
  if (!IS_AST_LIT_VALUE (condExpr))
    return FALSE;
  return !isEqualVal (AST_VALUE (condExpr), 0);
}

/*-----------------------------------------------------------------*/
/* createDoFor - creates parse tree for 'for' statement            */
/*                                                                 */
/* When we know that the condition is always true for the initial  */
/* iteration, we can build a more optimal tree by testing the      */
/* condition at the end of the loop (like do-while).               */
/*                                                                 */
/*        initExpr                                                 */
/*   _forbody_n:                                                   */
/*        statements                                               */
/*   _forcontinue_n:                                               */
/*        loopExpr                                                 */
/*        condExpr  +-> trueLabel -> _forbody_n                    */
/*                  |                                              */
/*                  +-> falseLabel-> _forbreak_n                   */
/*   _forbreak_n:                                                  */
/*-----------------------------------------------------------------*/
ast *
createDoFor (symbol * trueLabel, symbol * continueLabel, symbol * falseLabel,
             symbol * condLabel, ast * initExpr, ast * condExpr,
             ast * loopExpr, ast * forBody, ast * continueLabelAst)
{
  ast *forTree;

  if (condExpr)
    {
      condExpr = backPatchLabels (condExpr, trueLabel, falseLabel);
      if (condExpr && !IS_IFX (condExpr))
        condExpr = newIfxNode (condExpr, trueLabel, falseLabel);
    }
  else /* if no condition specified, it is considered always TRUE */
    condExpr = newNode (GOTO, newAst_VALUE (symbolVal (trueLabel)), NULL);

  /* attach body label to body */
  forBody = createLabel (trueLabel, forBody);

  /* attach continue to forLoop expression and condition */
  loopExpr = newNode (NULLOP, loopExpr, condExpr);
  if (continueLabelAst)
    {
      continueLabelAst->right = loopExpr;
      loopExpr = continueLabelAst;
    }
  else
    loopExpr = createLabel (continueLabel, loopExpr);
   
  /* now start putting them together */
  forTree = newNode (NULLOP, initExpr, forBody);
  forTree = newNode (NULLOP, forTree, loopExpr);

  /* the break label is already in the tree as a sibling */
  /* to the original FOR node this tree is replacing */
  return forTree;
}

/*-----------------------------------------------------------------*/
/* searchLitOp - search tree (*ops only) for an ast with literal */
/*-----------------------------------------------------------------*/
static ast *
searchLitOp (ast * tree, ast ** parent, const char *ops)
{
  ast *ret;

  if (tree && optimize.global_cse)
    {
      /* is there a literal operand? */
      if (tree->right &&
          IS_AST_OP (tree->right) &&
          tree->right->right && (tree->right->opval.op == (unsigned) ops[0] || tree->right->opval.op == (unsigned) ops[1]))
        {
          if (IS_LITERAL (RTYPE (tree->right)) != IS_LITERAL (LTYPE (tree->right)))
            {
              tree->right->decorated = 0;
              tree->decorated = 0;
              *parent = tree;
              return tree->right;
            }
          ret = searchLitOp (tree->right, parent, ops);
          if (ret)
            return ret;
        }
      if (tree->left &&
          IS_AST_OP (tree->left) &&
          tree->left->right && (tree->left->opval.op == (unsigned) ops[0] || tree->left->opval.op == (unsigned) ops[1]))
        {
          if (IS_LITERAL (RTYPE (tree->left)) != IS_LITERAL (LTYPE (tree->left)))
            {
              tree->left->decorated = 0;
              tree->decorated = 0;
              *parent = tree;
              return tree->left;
            }
          ret = searchLitOp (tree->left, parent, ops);
          if (ret)
            return ret;
        }
    }
  return NULL;
}

const char *
getResultTypeName (RESULT_TYPE resultType)
{
  switch (resultType)
  {
    case RESULT_TYPE_NONE: return "RESULT_TYPE_NONE";
    case RESULT_TYPE_BOOL: return "RESULT_TYPE_BOOL";
    case RESULT_TYPE_CHAR: return "RESULT_TYPE_CHAR";
    case RESULT_TYPE_INT: return "RESULT_TYPE_INT";
    case RESULT_TYPE_OTHER: return "RESULT_TYPE_OTHER";
    case RESULT_TYPE_IFX: return "RESULT_TYPE_IFX";
    case RESULT_TYPE_GPTR: return "RESULT_TYPE_GPTR";
  }
  return "invalid result type";
}

/*-----------------------------------------------------------------*/
/* getResultFromType                                               */
/*-----------------------------------------------------------------*/
RESULT_TYPE
getResultTypeFromType (sym_link * type)
{
  /* type = getSpec (type); */
  if (IS_BOOLEAN (type))
    return RESULT_TYPE_BOOL;
  if (IS_BITFIELD (type))
    {
      unsigned blen = SPEC_BLEN (type);

/*    BOOL and single bit BITFIELD are not interchangeable!
 *    There must be a cast to do this safely, in which case
 *    the previous IS_BOOLEAN test will handle it. 
      
      if (blen <= 1)
        return RESULT_TYPE_BOOL;
*/
      if (blen <= 8)
        return RESULT_TYPE_CHAR;
      return RESULT_TYPE_INT;
    }
  if (IS_CHAR (type))
    return RESULT_TYPE_CHAR;
  if (IS_INT (type) && !IS_LONG (type) && !IS_LONGLONG (type))
    return RESULT_TYPE_INT;
  return RESULT_TYPE_OTHER;
}

/*-----------------------------------------------------------------*/
/* addCast - adds casts to a type specified by RESULT_TYPE         */
/*-----------------------------------------------------------------*/
static ast *
addCast (ast * tree, RESULT_TYPE resultType, bool promote)
{
  sym_link *newLink;
  bool upCasted = FALSE;

  switch (resultType)
    {
    case RESULT_TYPE_NONE:
      /* if thing smaller than int must be promoted to int */
      if (!promote || getSize (tree->etype) >= INTSIZE)
        /* promotion not necessary or already an int */
        return tree;
      /* char and bits: promote to int */
      newLink = newIntLink ();
      upCasted = TRUE;
      break;
    case RESULT_TYPE_BOOL:
      if (!promote ||
          /* already an int */
          bitsForType (tree->etype) >= 16 ||
          /* bit to bit operation: don't promote, the code generators
             hopefully know everything about promotion rules */
          bitsForType (tree->etype) == 1)
        return tree;
      newLink = newIntLink ();
      upCasted = TRUE;
      break;
    case RESULT_TYPE_CHAR:
      if (IS_CHAR (tree->etype) || IS_FLOAT (tree->etype) || IS_FIXED (tree->etype))
        return tree;
      newLink = newCharLink ();
      break;
    case RESULT_TYPE_INT:
    case RESULT_TYPE_GPTR:
#if 0
      if (getSize (tree->etype) > INTSIZE)
        {
          /* warn ("Loosing significant digits"); */
          return;
        }
#endif
      /* char: promote to int */
      if (!promote || getSize (tree->etype) >= INTSIZE)
        return tree;
      newLink = newIntLink ();
      upCasted = TRUE;
      break;
    case RESULT_TYPE_IFX:
    case RESULT_TYPE_OTHER:
      if (!promote ||
          /* return type is ifx, long, float: promote char to int */
          getSize (tree->etype) >= INTSIZE)
        return tree;
      newLink = newIntLink ();
      upCasted = TRUE;
      break;
    default:
      return tree;
    }
  tree->decorated = 0;
  tree = newNode (CAST, newAst_LINK (newLink), tree);
  tree->filename = tree->right->filename;
  tree->lineno = tree->right->lineno;
  /* keep unsigned type during cast to smaller type,
     but not when promoting from char to int */
  if (!upCasted)
    SPEC_USIGN (tree->left->opval.lnk) = IS_UNSIGNED (tree->right->etype) ? 1 : 0;
  return decorateType (tree, resultType);
}

/*-----------------------------------------------------------------*/
/* resultTypePropagate - decides if resultType can be propagated   */
/*-----------------------------------------------------------------*/
static RESULT_TYPE
resultTypePropagate (ast *tree, RESULT_TYPE resultType)
{
  switch (tree->opval.op)
    {
    case AND_OP:
    case OR_OP:
    case '!':
      /* Logical operators should always propagate to boolean */
      return RESULT_TYPE_BOOL;
    case '=':
    case '?':
    case ':':
    case '|':
    case '^':
    case '~':
    case LEFT_OP:
    case LABEL:
    case GETHBIT:
    case GETABIT:
    case GETBYTE:
    case GETWORD:
      return resultType;
    case '*':
    case '+':
    case '-':
      if ((IS_AST_VALUE (tree->left) && !IS_INTEGRAL (tree->left->opval.val->etype)) ||
          (IS_AST_VALUE (tree->right) && !IS_INTEGRAL (tree->right->opval.val->etype)))
        return RESULT_TYPE_NONE;
      return resultType;
    case '&':
      if (!tree->right)
        /* can be unary */
        return RESULT_TYPE_NONE;
      else
        return resultType;
    case IFX:
      return RESULT_TYPE_IFX;
    default:
      return RESULT_TYPE_NONE;
    }
}

/*-----------------------------------------------------------------*/
/* getLeftResultType - gets type from left branch for propagation  */
/*-----------------------------------------------------------------*/
static RESULT_TYPE
getLeftResultType (ast * tree, RESULT_TYPE resultType)
{
  switch (tree->opval.op)
    {
    case '=':
    case CAST:
      if (IS_GENPTR (LTYPE (tree)))
        return RESULT_TYPE_GPTR;
      else if (IS_PTR (LTYPE (tree)))
        return RESULT_TYPE_NONE;
      else
        return getResultTypeFromType (LETYPE (tree));
    case RETURN:
      if (IS_PTR (currFunc->type->next))
        return RESULT_TYPE_NONE;
      else
        return getResultTypeFromType (currFunc->type->next);
    case '[':
      if (!IS_ARRAY (LTYPE (tree)))
        return resultType;
      if (DCL_ELEM (LTYPE (tree)) > 0 && getSize (tree->left->ftype) < 128)
        return RESULT_TYPE_CHAR; // TODO: Instead of doing this optimization here, do it later on the iCode (where it probably could use a more efficient unsigned char).
      return resultType;
    default:
      return resultType;
    }
}

/*------------------------------------------------------------------*/
/* gatherImplicitVariables: assigns correct type information to     */
/*            symbols and values created by replaceAstWithTemporary */
/*            and adds the symbols to the declarations list of the  */
/*            innermost block that contains them                    */
/*------------------------------------------------------------------*/
void
gatherImplicitVariables (ast * tree, ast * block)
{
  if (!tree)
    return;

  if (tree->type == EX_OP && tree->opval.op == BLOCK)
    {
      /* keep track of containing scope */
      block = tree;
    }
  if (tree->type == EX_OP && tree->opval.op == '=' && tree->left->type == EX_VALUE && tree->left->opval.val->sym)
    {
      symbol *assignee = tree->left->opval.val->sym;

      /* special case for assignment to compiler-generated temporary variable:
         compute type of RHS, and set the symbol's type to match */
      if (assignee->type == NULL && assignee->infertype)
        {
          ast *dtr = decorateType (resolveSymbols (tree->right), RESULT_TYPE_NONE);

          if (dtr != tree->right)
            tree->right = dtr;

          assignee->type = copyLinkChain (TTYPE (dtr));
          assignee->etype = getSpec (assignee->type);
          SPEC_ADDRSPACE (assignee->etype) = 0;
          SPEC_SCLS (assignee->etype) = S_AUTO;
          SPEC_OCLS (assignee->etype) = NULL;
          SPEC_EXTR (assignee->etype) = 0;
          SPEC_STAT (assignee->etype) = 0;
          SPEC_VOLATILE (assignee->etype) = 0;
          SPEC_ABSA (assignee->etype) = 0;
          SPEC_CONST (assignee->etype) = 0;

          wassertl (block != NULL, "implicit variable not contained in block");
          wassert (assignee->next == NULL);
          if (block != NULL)
            {
              symbol **decl = &(block->values.sym);

              while (*decl)
                {
                  wassert (*decl != assignee);  /* should not already be in list */
                  decl = &((*decl)->next);
                }

              *decl = assignee;
            }
        }
    }
  if (tree->type == EX_VALUE && !(IS_LITERAL (tree->opval.val->etype)) &&
      tree->opval.val->type == NULL && tree->opval.val->sym && tree->opval.val->sym->infertype)
    {
      /* fixup type of value for compiler-inferred temporary var */
      tree->opval.val->type = tree->opval.val->sym->type;
      tree->opval.val->etype = tree->opval.val->sym->etype;
    }

  gatherImplicitVariables (tree->left, block);
  gatherImplicitVariables (tree->right, block);
}

/*-----------------------------------------------------------------*/
/* CodePtrPointsToConst - if code memory is read-only, then        */
/*   pointers to code memory implicitly point to constants.        */
/*   Make this explicit.                                           */
/*-----------------------------------------------------------------*/
void
CodePtrPointsToConst (sym_link * t)
{
  if (port->mem.code_ro)
    {
      while (t && t->next)
        {
          if (IS_CODEPTR (t))
            {
              sym_link *t2 = t;
              /* find the first non-array link */
              while (IS_ARRAY (t2->next))
                t2 = t2->next;
              if (IS_SPEC (t2->next))
                SPEC_CONST (t2->next) = 1;
              else
                DCL_PTR_CONST (t2->next) = 1;
            }
          t = t->next;
        }
    }
}

/*-----------------------------------------------------------------*/
/* checkPtrCast - if casting to/from pointers, do some checking    */
/*-----------------------------------------------------------------*/
void
checkPtrCast (sym_link *newType, sym_link *orgType, bool implicit, bool orgIsNullPtrConstant)
{
  int errors = 0;
  
  if (IS_ARRAY (orgType))
    {
      value *val;
      val = aggregateToPointer (valFromType (orgType));
      orgType = val->type;
      Safe_free (val);
    }

  if (IS_PTR (newType))         // to a pointer
    {
      if (!IS_PTR (orgType) && !IS_FUNC (orgType) && !IS_AGGREGATE (orgType))   // from a non pointer
        {
          if (IS_INTEGRAL (orgType))
            {
              // maybe this is NULL, then it's ok.
              if (!(IS_LITERAL (orgType) && (SPEC_CVAL (orgType).v_ulong == 0)))
                {
                  if (GPTRSIZE > FARPTRSIZE && IS_GENPTR (newType) && !IS_FUNCPTR (newType))
                    {
                      // no way to set the storage
                      if (IS_LITERAL (orgType))
                        {
                          errors += werror (W_LITERAL_GENERIC);
                        }
                      else
                        {
                          errors += werror (W_NONPTR2_GENPTR);
                        }
                    }
                  else if (implicit)
                    {
                      errors += werror (W_INTEGRAL2PTR_NOCAST);
                    }
                }
            }
          else
            {
              // shouldn't do that with float, array or structure unless to void
              if (!IS_VOID (getSpec (newType)) && !(IS_CODEPTR (newType) && IS_FUNC (newType->next) && IS_FUNC (orgType)))
                {
                  errors += werror (E_INCOMPAT_TYPES);
                }
            }
        }
      else                      // from a pointer to a pointer
        {
          if (implicit && getAddrspace (newType->next) != getAddrspace (orgType->next))
            {
              errors += werror (E_INCOMPAT_PTYPES);
            }
          else if (IS_GENPTR (newType) && IS_VOID (newType->next)) // cast to void* is always allowed
            {
              if (IS_FUNCPTR (orgType)) 
                errors += werror (FUNCPTRSIZE > GPTRSIZE ? E_INCOMPAT_PTYPES : W_INCOMPAT_PTYPES);
            }
          else if (IS_GENPTR (orgType) && IS_VOID (orgType->next)) // cast from void* is always allowed - as long as we cast to a pointer to an object type
            {
              if (IS_FUNCPTR (newType) && !orgIsNullPtrConstant) // cast to pointer to function is only allowed for null pointer constants
                errors += werror (W_INCOMPAT_PTYPES);
            }
          else if (GPTRSIZE > FARPTRSIZE /*!TARGET_IS_Z80 && !TARGET_IS_GBZ80 */ )
            {
              // if not a pointer to a function
              if (!(IS_CODEPTR (newType) && IS_FUNC (newType->next) && IS_FUNC (orgType)))
                {
                  if (implicit) // if not to generic, they have to match
                    {
                      if (!IS_GENPTR (newType) &&
                          !((DCL_TYPE (orgType) == DCL_TYPE (newType)) ||
                            ((DCL_TYPE (orgType) == POINTER) && (DCL_TYPE (newType) == IPOINTER))))
                        {
                          errors += werror (E_INCOMPAT_PTYPES);
                        }
                    }
                }
            }
        }
    }
  else                          // to a non pointer
    {
      if (IS_PTR (orgType))     // from a pointer
        {
          if (implicit)         // sneaky
            {
              if (IS_INTEGRAL (newType))
                {
                  errors += werror (W_PTR2INTEGRAL_NOCAST);
                }
              else              // shouldn't do that with float, array or structure
                {
                  errors += werror (E_INCOMPAT_TYPES);
                }
            }
        }
    }
  if (errors)
    {
      printFromToType (orgType, newType);
    }
}

/*-----------------------------*/
/* check if div or mod by zero */
/*-----------------------------*/
static void
checkZero (value *val)
{
  if (!val)
    return;

  if (IS_FLOAT (val->type) || IS_FIXED16X16 (val->type))
    {
      if (floatFromVal (val) == 0.0)
        werror (E_DIVIDE_BY_ZERO);
    }
  else if (SPEC_LONGLONG (val->type))
    {
      if (ullFromVal (val) == 0LL)
        werror (E_DIVIDE_BY_ZERO);
    }
  else if (ulFromVal (val) == 0L)
    {
      werror (E_DIVIDE_BY_ZERO);
    }
}

/*--------------------------------------------------------*/
/* return true if subtree <search> is somewhere in <tree> */
/*--------------------------------------------------------*/
static bool
isAstInAst (ast *tree, ast *search)
{
  if (tree == search)
    return TRUE;
  if (!tree)
    return FALSE;
  if (tree->type == EX_OP)
    {
      if (isAstInAst (tree->left, search))
        return TRUE;
      if (isAstInAst (tree->right, search))
        return TRUE;
    }
  return FALSE;
}

/*---------------------------------------*/
/* make a simple copy of single ast node */
/*---------------------------------------*/
static ast *
copyAstNode (ast *tree)
{
  ast * newtree;

  newtree = newAst_(tree->type);
  newtree->lineno = tree->lineno;
  newtree->filename = tree->filename;
  newtree->level = tree->level;
  newtree->block = tree->block;
  newtree->initMode = tree->initMode;
  newtree->seqPoint = tree->seqPoint;
  newtree->funcName = tree->funcName;
  newtree->reversed = tree->reversed;
  newtree->decorated = tree->decorated;

  if (tree->ftype)
    newtree->etype = getSpec (newtree->ftype = copyLinkChain (tree->ftype));

  if (tree->type == EX_VALUE)
    {
      newtree->opval.val = tree->opval.val;
    }
  else if (tree->type == EX_LINK)
    {
      newtree->opval.lnk = tree->opval.lnk;
    }
  else
    {
      newtree->opval.op = tree->opval.op;
      copyAstValues (newtree, tree);
      newtree->left = tree->left;
      newtree->right = tree->right;

      newtree->trueLabel = tree->trueLabel;
      newtree->falseLabel = tree->falseLabel;
    }

  return newtree;
}

/*-------------------------------------------------*/
/* extract the ast subtrees that have side-effects */
/*-------------------------------------------------*/
static void
rewriteAstGatherSideEffects (ast *tree, set ** sideEffects)
{
  if (!tree)
    return;

  if (tree->type == EX_OP)
    {
      switch (tree->opval.op)
        {
          case '=':
          case INC_OP:
          case DEC_OP:
          case CALL:
          case PCALL:
            addSetHead (sideEffects, tree);
            break;
          default:
            if (tree->left)
              rewriteAstGatherSideEffects (tree->left, sideEffects);
            if (tree->right)
              rewriteAstGatherSideEffects (tree->right, sideEffects);
            break;
        }
      return;
    }
  if (TETYPE (tree) && IS_VOLATILE (TETYPE (tree)))
    {
      addSetHead (sideEffects, tree);
      return;
    }
}

/*-------------------------------------------------------------------*/
/* after rewriting a node, rejoin the portions of any old left/right */
/* subtree that had side effects with a comma operator               */
/*-------------------------------------------------------------------*/
static void
rewriteAstJoinSideEffects (ast *tree, ast *oLeft, ast *oRight)
{
  set * sideEffects = NULL;
  ast * sefTree;

  /* If the old left or right subtree has been orphaned, */
  /* gather any side-effects in it */
  if (oLeft && !isAstInAst (tree, oLeft))
    rewriteAstGatherSideEffects (oLeft, &sideEffects);
  if (oRight && !isAstInAst (tree, oRight))
    rewriteAstGatherSideEffects (oRight, &sideEffects);

  /* Join any side-effects found */
  for (sefTree = setFirstItem (sideEffects); sefTree; sefTree = setNextItem (sideEffects))
    {
      tree->right = copyAstNode (tree);
      tree->left = sefTree;
      tree->type = EX_OP;
      tree->opval.op = ',';
    }

  deleteSet (&sideEffects);
}

static void
optStdLibCall (ast *tree, RESULT_TYPE resulttype)
{
  ast *parms = tree->right;
  ast *func = tree->left;

  if (!TARGET_IS_STM8 && !TARGET_Z80_LIKE && !TARGET_PDK_LIKE) // Regression test gcc-torture-execute-20121108-1.c fails to build for hc08 and mcs51 (without --stack-auto)
    return;

  if (!IS_FUNC (func->ftype) || IS_LITERAL (func->ftype) || func->type != EX_VALUE || !func->opval.val->sym)
    return;

  const char *funcname = func->opval.val->sym->name;

  unsigned int nparms = 0;
  ast *parm;
  for (parm = parms; parm && parm->type == EX_OP && parm->opval.op == PARAM; parm = parm->right)
    if (parm->left)
      nparms++;
  if (parm)
    nparms++;

  // Optimize printf() to puts().
  if (!strcmp(funcname, "printf") && nparms == 1 && resulttype == RESULT_TYPE_NONE)
    {
      ast *parm = parms;

      if (parm->type == EX_OP && parm->opval.op == CAST)
        parm = parm->right;

      if (parm->type != EX_VALUE || !IS_ARRAY (parm->opval.val->type) || !parm->opval.val->sym)
        return;

      size_t strlength = DCL_ELEM (parm->opval.val->type);
      symbol *strsym = parm->opval.val->sym;
      sym_link *strlink = strsym->etype;

      if (strsym->isstrlit != 1 || !strlink || !IS_SPEC(strlink) || SPEC_NOUN (strlink) != V_CHAR)
        return;

      for (size_t i = 0; i < strlength; i++)
        if (SPEC_CVAL (strlink).v_char[i] == '%')
          return;
      if(strlength < 2 || SPEC_CVAL (strlink).v_char[strlength - 2] != '\n')
        return;

      symbol *puts_sym = findSym (SymbolTab, NULL, "puts");

      if(!puts_sym)
        return;

      // Do the equivalent of
      // DCL_ELEM (strsym->type)--;
      // but in a way that works better with the reuse of string symbols
      {
        struct dbuf_s dbuf;
        dbuf_init (&dbuf, strlength - 1);
	wassert (dbuf_append (&dbuf, SPEC_CVAL (strlink).v_char, strlength - 1));
        ((char *)(dbuf_get_buf (&dbuf)))[strlength - 2] = 0;

        parm->opval.val = stringToSymbol (rawStrVal (dbuf_get_buf (&dbuf), strlength - 1));
	dbuf_destroy (&dbuf);

        freeStringSymbol (strsym);
      }

      func->opval.val->sym = puts_sym;
    }
  // Optimize strcpy() to memcpy().
  else if (!strcmp(funcname, "strcpy") && nparms == 2)
    {
      ast *parm = parms->right;

      if (parm->type == EX_OP && parm->opval.op == CAST)
        parm = parm->right;

      if (parm->type != EX_VALUE || !IS_ARRAY (parm->opval.val->type) || !parm->opval.val->sym)
        return;

      size_t strlength = DCL_ELEM (parm->opval.val->type);
      symbol *strsym = parm->opval.val->sym;
      sym_link *strlink = strsym->etype;

      if (!strsym->isstrlit || !strlink || !IS_SPEC(strlink) || SPEC_NOUN (strlink) != V_CHAR)
        return;

      for (size_t i = 0; i < strlength; i++)
        if (!SPEC_CVAL (strlink).v_char[i])
          {
            strlength = i + 1;
            break;
          }

      size_t minlength; // Minimum string length for replacement.
      if (TARGET_IS_STM8)
        minlength = optimize.codeSize ? SIZE_MAX : 12;
      else // TODO:Check for other targets when memcpy() is a better choice than strcpy;
        minlength = SIZE_MAX;

      if (strlength < minlength)
        return;

      symbol *memcpy_sym = findSym (SymbolTab, NULL, "memcpy");

      if(!memcpy_sym)
        return;

      ast *lengthparm = newAst_VALUE (valCastLiteral (newIntLink(), strlength, strlength));
      decorateType (lengthparm, RESULT_TYPE_NONE);
      ast *node = newAst_OP (PARAM);
      node->left = parm;
      node->right = lengthparm;
      node->decorated = 1;
      parms->right = node;
      func->opval.val->sym = memcpy_sym;
    }
}

/*--------------------------------------------------------------*/
/* rewrite ast node as an operator node and rejoin any orphaned */
/* side-effects with a comma operator                           */
/*--------------------------------------------------------------*/
static void
rewriteAstNodeOp (ast *tree, int op, ast *left, ast *right)
{
  ast *oLeft = (tree->type == EX_OP) ? tree->left : NULL;
  ast *oRight = (tree->type == EX_OP) ? tree->right : NULL;

  tree->type = EX_OP;
  tree->opval.op = op;
  tree->left = left;
  tree->right = right;
  tree->decorated = 0;
  
  rewriteAstJoinSideEffects (tree, oLeft, oRight);
}

/*----------------------------------------------------------*/
/* rewrite ast node as a value node and rejoin any orphaned */
/* side-effects with a comma operator                       */
/*----------------------------------------------------------*/
static void
rewriteAstNodeVal (ast *tree, value *val)
{
  ast *oLeft = (tree->type == EX_OP) ? tree->left : NULL;
  ast *oRight = (tree->type == EX_OP) ? tree->right : NULL;

  tree->type = EX_VALUE;
  tree->opval.val = val;
  tree->left = NULL;
  tree->right = NULL;
  TETYPE (tree) = getSpec (TTYPE (tree) = tree->opval.val->type);
  tree->decorated = 0;
  
  rewriteAstJoinSideEffects (tree, oLeft, oRight);
}

/*-----------------------------------------------------------*/
/* rewrite struct assignment "a = b" to something similar to */
/* "__builtin_memcpy (&a, &b, sizeof (a)), a"   or, if a has */
/* side effects, "*(__builtin_memcpy (&a, &b, sizeof (a)))"  */
/*-----------------------------------------------------------*/
ast *
rewriteStructAssignment (ast *tree)
{
  /* prepare pointer to destination */
  ast *dest = newNode ('&', tree->left, NULL);
  copyAstLoc (dest, tree->left);

  /* prepare remaining arguments */
  ast *src = newNode ('&', tree->right, NULL);
  copyAstLoc (src, tree->right);
  ast *size = newNode (SIZEOF, NULL, tree->left);
  copyAstLoc (size, tree->left);
  ast *srcsize = newNode (PARAM, src, size);
  copyAstLoc (srcsize, tree);
  ast *params = newNode (PARAM, dest, srcsize);
  copyAstLoc (params, tree);

  /* create call to the appropriate memcpy function */
  ast *memcpy_ast = newAst_VALUE (symbolVal (memcpy_builtin));
  copyAstLoc (memcpy_ast, tree);
  ast *call = newNode (CALL, memcpy_ast, params);
  copyAstLoc (call, tree);

  /* assemble the result expression depending on side effects */
  ast *newTree;
  if (hasSEFcalls (dest))
    {
      /* memcpy returns the dest pointer -> dereference it */
      newTree = newNode ('*', call, NULL);
    }
  else
    {
      /* no side effects -> dereference dest pointer itself */
      ast *destderef = newNode ('*', dest, NULL);
      copyAstLoc (destderef, dest);
      newTree = newNode (',', call, destderef);
    }

  /* copy source location and return decorated result */
  copyAstLoc (newTree, tree);
  return decorateType (newTree, RESULT_TYPE_OTHER);
}

/*--------------------------------------------------------------------*/
/* decorateType - compute type for this tree, also does type checking.*/
/* This is done bottom up, since type has to flow upwards.            */
/* resultType flows top-down and forces e.g. char-arithmetic, if the  */
/* result is a char and the operand(s) are int's.                     */
/* It also does constant folding, and parameter checking.             */
/*--------------------------------------------------------------------*/
ast *
decorateType (ast *tree, RESULT_TYPE resultType)
{
  int parmNumber;
  sym_link *p;
  RESULT_TYPE resultTypeProp;

  if (!tree)
    return tree;

  /* if already has type then do nothing */
  if (tree->decorated)
    return tree;

  tree->decorated = 1;

#if 0
  /* print the line          */
  /* if not block & function */
  if (tree->type == EX_OP && (tree->opval.op != FUNCTION && tree->opval.op != BLOCK && tree->opval.op != NULLOP))
    {
      filename = tree->filename;
      lineno = tree->lineno;
    }
#endif

  /* if any child is an error | this one is an error do nothing */
  if (tree->isError || (tree->left && tree->left->isError) || (tree->right && tree->right->isError))
    return tree;

/*------------------------------------------------------------------*/
/*----------------------------*/
/*   leaf has been reached    */
/*----------------------------*/
  filename = tree->filename;
  lineno = tree->lineno;
  /* if this is of type value */
  /* just get the type        */
  if (tree->type == EX_VALUE)
    {
      if (IS_LITERAL (tree->opval.val->etype))
        {
          /* if this is a character array then declare it */
          if (IS_ARRAY (tree->opval.val->type))
            tree->opval.val = stringToSymbol (tree->opval.val);

          /* otherwise just copy the type information */
          COPYTYPE (TTYPE (tree), TETYPE (tree), tree->opval.val->type);
          return tree;
        }

      if (tree->opval.val->sym)
        {
          /* if the undefined flag is set then give error message */
          if (tree->opval.val->sym->undefined)
            {
              werrorfl (tree->filename, tree->lineno, E_ID_UNDEF, tree->opval.val->sym->name);
              /* assume int */
              TTYPE (tree) = TETYPE (tree) =
                tree->opval.val->type = tree->opval.val->sym->type =
                tree->opval.val->etype = tree->opval.val->sym->etype = copyLinkChain (INTTYPE);
            }
          else if (tree->opval.val->sym->implicit)
            {
              /* if implicit i.e. struct/union member then no type */
              TTYPE (tree) = TETYPE (tree) = NULL;
            }
          else
            {
              /* copy the type from the value into the ast */
              COPYTYPE (TTYPE (tree), TETYPE (tree), tree->opval.val->type);

              /* and mark the symbol as referenced */
              tree->opval.val->sym->isref = 1;
            }
        }
      else
        {
          /* unreached: all values are literals or symbols */
          wassert (0);
        }

      return tree;
    }

  /* if type link for the case of cast */
  if (tree->type == EX_LINK)
    {
      COPYTYPE (TTYPE (tree), TETYPE (tree), tree->opval.lnk);
      return tree;
    }

  {
    ast *dtl, *dtr;

#if 0
    if (tree->opval.op == NULLOP || tree->opval.op == BLOCK)
      {
        if (tree->left && tree->left->type == EX_OPERAND
            && (tree->left->opval.op == INC_OP || tree->left->opval.op == DEC_OP) && tree->left->left)
          {
            tree->left->right = tree->left->left;
            tree->left->left = NULL;
          }
        if (tree->right && tree->right->type == EX_OPERAND
            && (tree->right->opval.op == INC_OP || tree->right->opval.op == DEC_OP) && tree->right->left)
          {
            tree->right->right = tree->right->left;
            tree->right->left = NULL;
          }
      }
#endif

    /* Before decorating the left branch we've to decide in dependence
       upon tree->opval.op, if resultType can be propagated */
    resultTypeProp = resultTypePropagate (tree, resultType);

    if ((tree->opval.op == '?') && (resultTypeProp != RESULT_TYPE_BOOL))
      dtl = decorateType (tree->left, RESULT_TYPE_IFX);
    else
      dtl = decorateType (tree->left, resultTypeProp);

    /* if an array node, we may need to swap branches */
    if (tree->opval.op == '[')
      {
        /* determine which is the array & which the index */
        if ((IS_ARRAY (RTYPE (tree)) || IS_PTR (RTYPE (tree))) && IS_INTEGRAL (LTYPE (tree)))
          {
            ast *tempTree = tree->left;
            tree->left = tree->right;
            tree->right = tempTree;
          }
      }

    /* After decorating the left branch there's type information available
       in tree->left->?type. If the op is e.g. '=' we extract the type
       information from there and propagate it to the right branch. */
    resultTypeProp = getLeftResultType (tree, resultTypeProp);

    switch (tree->opval.op)
      {
      case '?':
        /* delay right side for '?' operator since conditional macro
           expansions might rely on this */
        dtr = tree->right;
        break;
      case CALL:
        /* decorate right side for CALL (parameter list) in processParms();
           there is resultType available */
        dtr = tree->right;
        break;
      case SIZEOF:
        /* don't allocate string if it is a sizeof argument */
        ++noAlloc;
        dtr = decorateType (tree->right, resultTypeProp);
        --noAlloc;
        break;
      default:
        dtr = decorateType (tree->right, resultTypeProp);
        break;
      }

    /* this is to take care of situations
       when the tree gets rewritten */
    if (dtl != tree->left)
      tree->left = dtl;
    if (dtr != tree->right)
      tree->right = dtr;
    if ((dtl && dtl->isError) || (dtr && dtr->isError))
      return tree;
  }

  /* depending on type of operator do */

  switch (tree->opval.op)
    {
      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*        array node          */
      /*----------------------------*/
    case '[':
      /* Swap trees if right side is array or a pointer */
      if (IS_ARRAY (RTYPE (tree)) || IS_PTR (RTYPE (tree)))
        {
          ast *tTree = tree->left;
          tree->left = tree->right;
          tree->right = tTree;
        }

      /* check if this is an array or a pointer */
      if ((!IS_ARRAY (LTYPE (tree))) && (!IS_PTR (LTYPE (tree))))
        {
          werrorfl (tree->filename, tree->lineno, E_NEED_ARRAY_PTR, "[]");
          goto errorTreeReturn;
        }

      /* check the type of the idx */
      if (!IS_INTEGRAL (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_IDX_NOT_INT);
          goto errorTreeReturn;
        }

      /* if the left is an rvalue then error */
      if (LRVAL (tree))
        {
          werrorfl (tree->filename, tree->lineno, E_LVALUE_REQUIRED, "array access");
          goto errorTreeReturn;
        }

      if (IS_LITERAL (RTYPE (tree)))
        {
          int arrayIndex = (int) ulFromVal (valFromType (RETYPE (tree)));
          int arraySize = DCL_ELEM (LTYPE (tree));
          if (arraySize && arrayIndex >= arraySize)
            {
              werrorfl (tree->filename, tree->lineno, W_IDX_OUT_OF_BOUNDS, arrayIndex, arraySize);
            }
        }

      RRVAL (tree) = 1;
      TTYPE (tree) = copyLinkChain (LTYPE (tree)->next);
      TETYPE (tree) = getSpec (TTYPE (tree));

      if (IS_PTR (LTYPE (tree)) /* && !IS_LITERAL (TETYPE (tree)) caused bug #2850 */)
        {
          SPEC_SCLS (TETYPE (tree)) = sclsFromPtr (LTYPE (tree));
        }

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      struct/union          */
      /*----------------------------*/
    case '.':
      /* if this is not a structure */
      if (!IS_STRUCT (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_STRUCT_UNION, ".");
          goto errorTreeReturn;
        }
      TTYPE (tree) = structElemType (LTYPE (tree), (tree->right->type == EX_VALUE ? tree->right->opval.val : NULL));
      TETYPE (tree) = getSpec (TTYPE (tree));
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*    struct/union pointer    */
      /*----------------------------*/
    case PTR_OP:
      /* if not pointer to a structure */
      if (!IS_PTR (LTYPE (tree)) && !IS_ARRAY (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_PTR_REQD);
          goto errorTreeReturn;
        }

      if (!IS_STRUCT (LTYPE (tree)->next))
        {
          werrorfl (tree->filename, tree->lineno, E_STRUCT_UNION, "->");
          goto errorTreeReturn;
        }

      TTYPE (tree) = structElemType (LTYPE (tree)->next, (tree->right->type == EX_VALUE ? tree->right->opval.val : NULL));
      TETYPE (tree) = getSpec (TTYPE (tree));

      /* adjust the storage class */
      if (DCL_TYPE (LTYPE (tree)) != ARRAY)
        {
          setOClass (LTYPE (tree), TETYPE (tree));
          SPEC_SCLS (TETYPE (tree)) = sclsFromPtr (LTYPE (tree));
          SPEC_ADDRSPACE (TETYPE (tree)) = DCL_PTR_ADDRSPACE (LTYPE (tree));
        }
      /* This breaks with extern declarations, bit-fields, and perhaps other */
      /* cases (gcse). Let's leave this optimization disabled for now and   */
      /* ponder if there's a safe way to do this. -- EEP                    */
#if 0
      if (IS_ADDRESS_OF_OP (tree->left) && IS_AST_SYM_VALUE (tree->left->left)
          && SPEC_ABSA (AST_SYMBOL (tree->left->left)->etype))
        {
          /* If defined    struct type at addr var
             then rewrite  (&struct var)->member
             as            temp
             and define    membertype at (addr+offsetof(struct var,member)) temp
           */
          symbol *sym;
          symbol *element = getStructElement (SPEC_STRUCT (LETYPE (tree)),
                                              AST_SYMBOL (tree->right));

          sym = newSymbol (genSymName (0), 0);
          sym->type = TTYPE (tree);
          sym->etype = getSpec (sym->type);
          sym->lineDef = tree->lineno;
          sym->cdef = 1;
          sym->isref = 1;
          SPEC_STAT (sym->etype) = 1;
          SPEC_ADDR (sym->etype) = SPEC_ADDR (AST_SYMBOL (tree->left->left)->etype) + element->offset;
          SPEC_ABSA (sym->etype) = 1;
          addSym (SymbolTab, sym, sym->name, 0, 0, 0);
          allocGlobal (sym);

          AST_VALUE (tree) = symbolVal (sym);
          TLVAL (tree) = 1;
          TRVAL (tree) = 0;
          tree->type = EX_VALUE;
          tree->left = NULL;
          tree->right = NULL;
        }
#endif

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*  ++/-- operation           */
      /*----------------------------*/
    case INC_OP:
    case DEC_OP:
      {
        sym_link *ltc = (tree->right ? RTYPE (tree) : LTYPE (tree));
        COPYTYPE (TTYPE (tree), TETYPE (tree), ltc);
        if (!tree->initMode && IS_CONSTANT (TTYPE (tree)))
          werrorfl (tree->filename, tree->lineno, E_CODE_WRITE, tree->opval.op == INC_OP ? "++" : "--");

        if (tree->right)
          RLVAL (tree) = 1;
        else
          LLVAL (tree) = 1;
        return tree;
      }

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*  bitwise and               */
      /*----------------------------*/
    case '&':                  /* can be unary */
      /* if right is NULL then unary operation */
      if (tree->right)          /* not a unary operation */
        {
          ast *otree;

          if (!IS_INTEGRAL (LTYPE (tree)) || !IS_INTEGRAL (RTYPE (tree)))
            {
              werrorfl (tree->filename, tree->lineno, E_BITWISE_OP);
              werrorfl (tree->filename, tree->lineno, W_CONTINUE, "left & right types are ");
              printTypeChain (LTYPE (tree), stderr);
              fprintf (stderr, ",");
              printTypeChain (RTYPE (tree), stderr);
              fprintf (stderr, "\n");
              goto errorTreeReturn;
            }

          /* if they are both literal */
          if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
            {
              tree->type = EX_VALUE;
              tree->opval.val = valBitwise (valFromType (LETYPE (tree)), valFromType (RETYPE (tree)), '&');

              tree->right = tree->left = NULL;
              TETYPE (tree) = tree->opval.val->etype;
              TTYPE (tree) = tree->opval.val->type;
              return tree;
            }

          /* if left is a literal exchange left & right */
          if (IS_LITERAL (LTYPE (tree)))
            {
              ast *tTree = tree->left;
              tree->left = tree->right;
              tree->right = tTree;
            }

          /* if right is a literal and */
          /* we can find a 2nd literal in an and-tree then */
          /* rearrange the tree */
          if (IS_LITERAL (RTYPE (tree)))
            {
              ast *parent;
              ast *litTree = searchLitOp (tree, &parent, "&");
              if (litTree)
                {
                  DEBUG_CF ("&") ast *tTree = litTree->left;
                  litTree->left = tree->right;
                  tree->right = tTree;
                  /* both operands in litTree are literal now */
                  decorateType (parent, resultType);
                }
            }

          /* if ANDing boolean with literal then reduce literal LSB to boolean */
          if (IS_LITERAL (RTYPE (tree)) && IS_BOOLEAN (LTYPE (tree)))
            {
              unsigned long litval = AST_ULONG_VALUE (tree->right);
              tree->right = decorateType (newAst_VALUE (constBoolVal (litval & 1)), resultType);
            }

          /* see if this is a GETHBIT operation if yes
             then return that */
          otree = optimizeGetHbit (tree, resultType);
          if (otree != tree)
            return decorateType (otree, RESULT_TYPE_NONE);

          /* see if this is a GETABIT operation if yes
             then return that */
          otree = optimizeGetAbit (tree, resultType);
          if (otree != tree)
            return decorateType (otree, RESULT_TYPE_NONE);

          /* see if this is a GETBYTE operation if yes
             then return that */
          otree = optimizeGetByte (tree, resultType);
          if (otree != tree)
            return decorateType (otree, RESULT_TYPE_NONE);

          /* see if this is a GETWORD operation if yes
             then return that */
          otree = optimizeGetWord (tree, resultType);
          if (otree != tree)
            return decorateType (otree, RESULT_TYPE_NONE);

          /* if right is a literal and has the same size with left, 
             then also sync their signess to avoid unecessary cast */
          if (IS_LITERAL (RTYPE (tree)) && getSize (RTYPE (tree)) == getSize (LTYPE (tree)))
            SPEC_USIGN (RTYPE (tree)) = SPEC_USIGN (LTYPE (tree));

          LRVAL (tree) = RRVAL (tree) = 1;

          TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), resultType, tree->opval.op);
          TETYPE (tree) = getSpec (TTYPE (tree));

          return tree;
        }

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*  address of                */
      /*----------------------------*/
      if (IS_FUNC (LTYPE (tree)))
        {
          // this ought to be ignored
          return (tree->left);
        }

      /* if bit field then error */
      if (IS_BITFIELD (tree->left->etype) || (IS_BITVAR (tree->left->etype) && TARGET_MCS51_LIKE))
        {
          werrorfl (tree->filename, tree->lineno, E_ILLEGAL_ADDR, "address of bit variable");
          goto errorTreeReturn;
        }

      if (LETYPE (tree) && SPEC_SCLS (tree->left->etype) == S_REGISTER)
        {
          werrorfl (tree->filename, tree->lineno, E_ILLEGAL_ADDR, "address of register variable");
          goto errorTreeReturn;
        }

      if (IS_LITERAL (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_ILLEGAL_ADDR, "address of literal");
          goto errorTreeReturn;
        }

      if (LRVAL (tree))
        {
          werrorfl (tree->filename, tree->lineno, E_LVALUE_REQUIRED, "address of");
          goto errorTreeReturn;
        }

      p = newLink (DECLARATOR);
      if (!LETYPE (tree))
        DCL_TYPE (p) = POINTER;
      else if (SPEC_SCLS (LETYPE (tree)) == S_CODE)
        DCL_TYPE (p) = CPOINTER;
      else if (SPEC_SCLS (LETYPE (tree)) == S_XDATA)
        DCL_TYPE (p) = FPOINTER;
      else if (SPEC_SCLS (LETYPE (tree)) == S_XSTACK)
        DCL_TYPE (p) = PPOINTER;
      else if (SPEC_SCLS (LETYPE (tree)) == S_IDATA)
        DCL_TYPE (p) = IPOINTER;
      else if (SPEC_SCLS (LETYPE (tree)) == S_EEPROM)
        DCL_TYPE (p) = EEPPOINTER;
      else if (SPEC_OCLS (LETYPE (tree)))
        DCL_TYPE (p) = PTR_TYPE (SPEC_OCLS (LETYPE (tree)));
      else
        DCL_TYPE (p) = POINTER;

      if (IS_AST_SYM_VALUE (tree->left))
        {
          AST_SYMBOL (tree->left)->addrtaken = 1;
          AST_SYMBOL (tree->left)->allocreq = 1;
        }

      p->next = LTYPE (tree);
      TTYPE (tree) = p;
      TETYPE (tree) = getSpec (TTYPE (tree));
      LLVAL (tree) = 1;
      TLVAL (tree) = 1;

#if 0
      if (IS_AST_OP (tree->left) && tree->left->opval.op == PTR_OP
          && IS_AST_VALUE (tree->left->left) && !IS_AST_SYM_VALUE (tree->left->left))
        {
          symbol *element = getStructElement (SPEC_STRUCT (LETYPE (tree->left)),
                                              AST_SYMBOL (tree->left->right));
          AST_VALUE (tree) = valPlus (AST_VALUE (tree->left->left), valueFromLit (element->offset));
          tree->left = NULL;
          tree->right = NULL;
          tree->type = EX_VALUE;
          tree->values.cast.literalFromCast = 1;
        }
#endif

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*  bitwise or                */
      /*----------------------------*/
    case '|':
      /* if the rewrite succeeds then don't go any further */
      {
        ast *wtree = optimizeRRCRLC (tree);
        if (wtree != tree)
          return decorateType (wtree, RESULT_TYPE_NONE);

        wtree = optimizeSWAP (tree);
        if (wtree != tree)
          return decorateType (wtree, RESULT_TYPE_NONE);
      }

      /* if left is a literal exchange left & right */
      if (IS_LITERAL (LTYPE (tree)))
        {
          ast *tTree = tree->left;
          tree->left = tree->right;
          tree->right = tTree;
        }

      /* if right is a literal and */
      /* we can find a 2nd literal in an or-tree then */
      /* rearrange the tree */
      if (IS_LITERAL (RTYPE (tree)))
        {
          ast *parent;
          ast *litTree = searchLitOp (tree, &parent, "|");
          if (litTree)
            {
              DEBUG_CF ("|") ast *tTree = litTree->left;
              litTree->left = tree->right;
              tree->right = tTree;
              /* both operands in tTree are literal now */
              decorateType (parent, resultType);
            }
        }

      /* if ORing boolean with literal then reduce literal to boolean */
      if (IS_LITERAL (RTYPE (tree)) &&
          IS_BOOLEAN (LTYPE (tree)) &&
          IS_INTEGRAL (RTYPE (tree)) &&
          resultType == RESULT_TYPE_BOOL)
        {
          unsigned long litval = AST_ULONG_VALUE (tree->right);
          tree->right = decorateType (newAst_VALUE (constBoolVal (litval != 0)), resultType);
        }

      /* fall through */

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*  bitwise xor               */
      /*----------------------------*/
    case '^':
      if (!IS_INTEGRAL (LTYPE (tree)) || !IS_INTEGRAL (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_BITWISE_OP);
          werrorfl (tree->filename, tree->lineno, W_CONTINUE, "left & right types are ");
          printTypeChain (LTYPE (tree), stderr);
          fprintf (stderr, ",");
          printTypeChain (RTYPE (tree), stderr);
          fprintf (stderr, "\n");
          goto errorTreeReturn;
        }

      /* if they are both literal then rewrite the tree */
      if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
        {
          rewriteAstNodeVal (tree, valBitwise (valFromType (LETYPE (tree)), valFromType (RETYPE (tree)), tree->opval.op));
          return decorateType (tree, resultType);
        }

      /* if left is a literal exchange left & right */
      if (IS_LITERAL (LTYPE (tree)))
        {
          ast *tTree = tree->left;
          tree->left = tree->right;
          tree->right = tTree;
        }

      /* if right is a literal and */
      /* we can find a 2nd literal in a xor-tree then */
      /* rearrange the tree */
      if (IS_LITERAL (RTYPE (tree)) && tree->opval.op == '^')   /* the same source is used by 'bitwise or' */
        {
          ast *parent;
          ast *litTree = searchLitOp (tree, &parent, "^");
          if (litTree)
            {
              DEBUG_CF ("^") ast *tTree = litTree->left;
              litTree->left = tree->right;
              tree->right = tTree;
              /* both operands in litTree are literal now */
              decorateType (parent, resultType);
            }
        }

      /* if XORing boolean with literal then reduce literal to boolean */
      if (IS_LITERAL (RTYPE (tree)) &&
          IS_BOOLEAN (LTYPE (tree)) &&
          IS_INTEGRAL (RTYPE (tree)) &&
          resultType == RESULT_TYPE_BOOL &&
          tree->opval.op == '^')   /* the same source is used by 'bitwise or' */
        {
          unsigned long litval = AST_ULONG_VALUE (tree->right);
          if (litval == 0 || litval == 1)
            {
              tree->right = decorateType (newAst_VALUE (constBoolVal (litval != 0)), resultType);
            }
          else
            {
              tree->opval.op = '|';
              tree->right = newAst_VALUE (constBoolVal (1));
              tree->decorated = 0;
              return decorateType (tree, resultType);
            }
        }

      /* if right is a literal and has the same size with left, 
         then also sync their signess to avoid unecessary cast */
      if (IS_LITERAL (RTYPE (tree)) && getSize (RTYPE (tree)) == getSize (LTYPE (tree)))
        SPEC_USIGN (RTYPE (tree)) = SPEC_USIGN (LTYPE (tree));

      LRVAL (tree) = RRVAL (tree) = 1;

      TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), resultType, tree->opval.op);
      TETYPE (tree) = getSpec (TTYPE (tree));

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*  division                  */
      /*----------------------------*/
    case '/':
      if (!IS_ARITHMETIC (LTYPE (tree)) || !IS_ARITHMETIC (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_INVALID_OP, "divide");
          goto errorTreeReturn;
        }
      /* check if div by zero */
      if (IS_LITERAL (RTYPE (tree)) && !IS_LITERAL (LTYPE (tree)))
        checkZero (valFromType (RETYPE (tree)));
      /* if they are both literal then */
      /* rewrite the tree */
      if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
        {
          rewriteAstNodeVal (tree, valDiv (valFromType (LETYPE (tree)), valFromType (RETYPE (tree))));
          return decorateType (tree, resultType);
        }

      LRVAL (tree) = RRVAL (tree) = 1;

      TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), resultType, tree->opval.op));

      /* if right is a literal and */
      /* left is also a division by a literal then */
      /* rearrange the tree */
#if 0
      /* This converts (a/b)/c into a/(b*c)/1, where b and c are literals. */
      /* Algebraically, this is fine, but may fail as an optimization if  */
      /* b*c overflows or causes the expression to have a different resultant */
      /* type. I don't think it's worth the effort to sort out the cases of */
      /* when this is safe or not safe, so I am just going to leave this */
      /* disabled. -- EEP -- 15 Nov 2012 */
      if (IS_LITERAL (RTYPE (tree))
          /* avoid infinite loop */
          && (TYPE_TARGET_ULONG) ulFromVal (tree->right->opval.val) != 1)
        {
          ast *parent;
          ast *litTree = searchLitOp (tree, &parent, "/");
          if (litTree)
            {
              if (IS_LITERAL (RTYPE (litTree)))
                {
                  /* foo_div */
                  DEBUG_CF ("div r") litTree->right = newNode ('*', litTree->right, copyAst (tree->right));
                  litTree->right->filename = tree->filename;
                  litTree->right->lineno = tree->lineno;

                  tree->right->opval.val = constCharVal (1);
                  decorateType (parent, resultType);
                }
              else
                {
                  /* litTree->left is literal: no gcse possible.
                     We can't call decorateType(parent, RESULT_TYPE_NONE), because
                     this would cause an infinit loop. */
                  parent->decorated = 1;
                  decorateType (litTree, resultType);
                }
            }
        }
#endif

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*            modulus         */
      /*----------------------------*/
    case '%':
      if (!IS_INTEGRAL (LTYPE (tree)) || !IS_INTEGRAL (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_BITWISE_OP);
          werrorfl (tree->filename, tree->lineno, W_CONTINUE, "left & right types are ");
          printTypeChain (LTYPE (tree), stderr);
          fprintf (stderr, ",");
          printTypeChain (RTYPE (tree), stderr);
          fprintf (stderr, "\n");
          goto errorTreeReturn;
        }
      /* check if div by zero */
      if (IS_LITERAL (RTYPE (tree)) && !IS_LITERAL (LTYPE (tree)))
        checkZero (valFromType (RETYPE (tree)));
      /* if they are both literal then */
      /* rewrite the tree */
      if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
        {
          rewriteAstNodeVal (tree, valMod (valFromType (LETYPE (tree)), valFromType (RETYPE (tree))));
          return decorateType (tree, resultType);
        }
      LRVAL (tree) = RRVAL (tree) = 1;
      TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), resultType, tree->opval.op));
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*  address dereference       */
      /*----------------------------*/
    case '*':                  /* can be unary  : if right is null then unary operation */
      if (!tree->right)
        {
          if (!IS_PTR (LTYPE (tree)) && !IS_ARRAY (LTYPE (tree)))
            {
              werrorfl (tree->filename, tree->lineno, E_PTR_REQD);
              goto errorTreeReturn;
            }

          if (LRVAL (tree))
            {
              werrorfl (tree->filename, tree->lineno, E_LVALUE_REQUIRED, "pointer deref");
              goto errorTreeReturn;
            }
          if (IS_ADDRESS_OF_OP (tree->left))
            {
              /* replace *&obj with obj */
              return tree->left->left;
            }
          TTYPE (tree) = copyLinkChain (LTYPE (tree)->next);
          TETYPE (tree) = getSpec (TTYPE (tree));
          /* adjust the storage class */
          if (DCL_TYPE (tree->left->ftype) != ARRAY && DCL_TYPE (tree->left->ftype) != FUNCTION)
            SPEC_SCLS (TETYPE (tree)) = sclsFromPtr (tree->left->ftype);

          return tree;
        }

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      multiplication        */
      /*----------------------------*/
      if (!IS_ARITHMETIC (LTYPE (tree)) || !IS_ARITHMETIC (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_INVALID_OP, "multiplication");
          goto errorTreeReturn;
        }

      /* if they are both literal then */
      /* rewrite the tree */
      if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
        {
          rewriteAstNodeVal (tree, valMult (valFromType (LETYPE (tree)), valFromType (RETYPE (tree))));
          return decorateType (tree, resultType);
        }

      /* if left is a literal exchange left & right */
      if (IS_LITERAL (LTYPE (tree)))
        {
          ast *tTree = tree->left;
          tree->left = tree->right;
          tree->right = tTree;
        }

      /* if right is a literal and */
      /* we can find a 2nd literal in a mul-tree then */
      /* rearrange the tree */
      if (IS_LITERAL (RTYPE (tree)))
        {
          ast *parent;
          ast *litTree = searchLitOp (tree, &parent, "*");
          if (litTree)
            {
              DEBUG_CF ("mul") ast *tTree = litTree->left;
              litTree->left = tree->right;
              tree->right = tTree;
              /* both operands in litTree are literal now */
              decorateType (parent, resultType);
            }
        }

      LRVAL (tree) = RRVAL (tree) = 1;

      { // cast happen only if both left and right can be casted to result type
        ast *l = addCast (tree->left, resultTypeProp, FALSE);
        ast *r = addCast (tree->right, resultTypeProp, FALSE);
        if (l != tree->left && r != tree->right)
          {
            tree->left = l;
            tree->right = r;
          }
      }
      TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), resultType, tree->opval.op));

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*    unary '+' operator      */
      /*----------------------------*/
    case '+':
      /* if unary plus */
      if (!tree->right)
        {
          if (!IS_ARITHMETIC (LTYPE (tree)))
            {
              werrorfl (tree->filename, tree->lineno, E_UNARY_OP, '+');
              goto errorTreeReturn;
            }

          /* if left is a literal then do it */
          if (IS_LITERAL (LTYPE (tree)))
            {
              tree->type = EX_VALUE;
              tree->opval.val = valFromType (LETYPE (tree));
              tree->left = NULL;
              TETYPE (tree) = TTYPE (tree) = tree->opval.val->type;
              return tree;
            }
          LRVAL (tree) = 1;
          COPYTYPE (TTYPE (tree), TETYPE (tree), LTYPE (tree));
          return tree;
        }

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      addition              */
      /*----------------------------*/

      /* this is not a unary operation */
      /* if both pointers then problem */
      if ((IS_PTR (LTYPE (tree)) || IS_ARRAY (LTYPE (tree))) && (IS_PTR (RTYPE (tree)) || IS_ARRAY (RTYPE (tree))))
        {
          werrorfl (tree->filename, tree->lineno, E_PTR_PLUS_PTR);
          goto errorTreeReturn;
        }

      if (!IS_ARITHMETIC (LTYPE (tree)) && !IS_PTR (LTYPE (tree)) && !IS_ARRAY (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "+");
          goto errorTreeReturn;
        }

      if (!IS_ARITHMETIC (RTYPE (tree)) && !IS_PTR (RTYPE (tree)) && !IS_ARRAY (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "+");
          goto errorTreeReturn;
        }
      /* if they are both literal then */
      /* rewrite the tree */
      if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
        {
          tree->left = addCast (tree->left, resultTypeProp, TRUE);
          tree->right = addCast (tree->right, resultTypeProp, TRUE);
          rewriteAstNodeVal (tree, valPlus (valFromType (LETYPE (tree)), valFromType (RETYPE (tree))));
          return decorateType (tree, resultType);
        }

      /* if the right is a pointer or left is a literal
         xchange left & right */
      if (IS_ARRAY (RTYPE (tree)) || IS_PTR (RTYPE (tree)) || IS_LITERAL (LTYPE (tree)))
        {
          ast *tTree = tree->left;
          tree->left = tree->right;
          tree->right = tTree;
        }

      /* if right is a literal and */
      /* left is also an addition/subtraction with a literal then */
      /* rearrange the tree */
      if (IS_LITERAL (RTYPE (tree)))
        {
          ast *litTree, *parent;
          litTree = searchLitOp (tree, &parent, "+-");
          if (litTree)
            {
              if (litTree->opval.op == '+')
                {
                  /* foo_aa */
                  DEBUG_CF ("+ 1 AA") ast *tTree = litTree->left;
                  litTree->left = tree->right;
                  tree->right = tree->left;
                  tree->left = tTree;
                }
              else if (litTree->opval.op == '-')
                {
                  if (IS_LITERAL (RTYPE (litTree)))
                    {
                      DEBUG_CF ("+ 2 ASR")
                        /* foo_asr */
                      ast *tTree = litTree->left;
                      litTree->left = tree->right;
                      tree->right = tTree;
                    }
                  else
                    {
                      DEBUG_CF ("+ 3 ASL")
                        /* foo_asl */
                      ast *tTree = litTree->right;
                      litTree->right = tree->right;
                      tree->right = tTree;
                      litTree->opval.op = '+';
                      tree->opval.op = '-';
                    }
                }
              decorateType (parent, resultType);
            }
        }

      LRVAL (tree) = RRVAL (tree) = 1;

      /* if the left is a pointer */
      if (IS_PTR (LTYPE (tree)) || IS_AGGREGATE (LTYPE (tree)))
        TETYPE (tree) = getSpec (TTYPE (tree) = LTYPE (tree));
      else
        {
          tree->left = addCast (tree->left, resultTypeProp, TRUE);
          tree->right = addCast (tree->right, resultTypeProp, TRUE);
          TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), resultType, tree->opval.op));
        }
      if (IS_LITERAL (TETYPE (tree)))
        {
          if (TTYPE (tree) == LTYPE (tree))
            TETYPE (tree) = getSpec (TTYPE (tree) = copyLinkChain (TTYPE (tree)));
          SPEC_SCLS (TETYPE (tree)) = 0;
        }

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      unary '-'             */
      /*----------------------------*/
    case '-':                  /* can be unary   */
      /* if right is null then unary */
      if (!tree->right)
        {
          if (!IS_ARITHMETIC (LTYPE (tree)))
            {
              werrorfl (tree->filename, tree->lineno, E_UNARY_OP, tree->opval.op);
              goto errorTreeReturn;
            }

          /* if left is a literal then do it */
          if (IS_LITERAL (LTYPE (tree)))
            {
              tree->type = EX_VALUE;
              tree->opval.val = valUnaryPM (valFromType (LETYPE (tree)));
              tree->left = NULL;
              TETYPE (tree) = TTYPE (tree) = tree->opval.val->type;
              return tree;
            }
          tree->left = addCast (tree->left, resultTypeProp, TRUE);
          TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), NULL, resultType, tree->opval.op));
          LRVAL (tree) = 1;
          return tree;
        }

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*    subtraction             */
      /*----------------------------*/

      if (!(IS_PTR (LTYPE (tree)) || IS_ARRAY (LTYPE (tree)) || IS_ARITHMETIC (LTYPE (tree))))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "-");
          goto errorTreeReturn;
        }

      if (!(IS_PTR (RTYPE (tree)) || IS_ARRAY (RTYPE (tree)) || IS_ARITHMETIC (RTYPE (tree))))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "-");
          goto errorTreeReturn;
        }

      if ((IS_PTR (LTYPE (tree)) || IS_ARRAY (LTYPE (tree))) &&
          !(IS_PTR (RTYPE (tree)) || IS_ARRAY (RTYPE (tree)) || IS_INTEGRAL (RTYPE (tree))))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "-");
          goto errorTreeReturn;
        }

      /* if they are both literal then */
      /* rewrite the tree */
      if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
        {
          tree->left = addCast (tree->left, resultTypeProp, TRUE);
          tree->right = addCast (tree->right, resultTypeProp, TRUE);
          rewriteAstNodeVal (tree, valMinus (valFromType (LETYPE (tree)), valFromType (RETYPE (tree))));
          return decorateType (tree, resultType);
       }

      /* if the left & right are equal then zero */
      if (!hasSEFcalls(tree->left) && !hasSEFcalls(tree->right) &&
        isAstEqual (tree->left, tree->right))
        {
          tree->type = EX_VALUE;
          tree->left = tree->right = NULL;
          tree->opval.val = constVal ("0");
          TETYPE (tree) = TTYPE (tree) = tree->opval.val->type;
          return tree;
        }

      /* if both of them are pointers or arrays then */
      /* the result is going to be an integer        */
      if ((IS_ARRAY (LTYPE (tree)) || IS_PTR (LTYPE (tree))) && (IS_ARRAY (RTYPE (tree)) || IS_PTR (RTYPE (tree))))
        TETYPE (tree) = TTYPE (tree) = newIntLink ();
      else
        /* if only the left is a pointer */
        /* then result is a pointer      */
      if (IS_PTR (LTYPE (tree)) || IS_ARRAY (LTYPE (tree)))
        TETYPE (tree) = getSpec (TTYPE (tree) = LTYPE (tree));
      else
        {
          tree->left = addCast (tree->left, resultTypeProp, TRUE);
          tree->right = addCast (tree->right, resultTypeProp, TRUE);

          TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), resultType, tree->opval.op));
        }
      if (IS_LITERAL (TETYPE (tree)))
        {
          if (TTYPE (tree) == LTYPE (tree))
            TETYPE (tree) = getSpec (TTYPE (tree) = copyLinkChain (TTYPE (tree)));
          SPEC_SCLS (TETYPE (tree)) = 0;
        }

      LRVAL (tree) = RRVAL (tree) = 1;

      /* if right is a literal and */
      /* left is also an addition/subtraction with a literal then */
      /* rearrange the tree */
      if (IS_LITERAL (RTYPE (tree))
          /* avoid infinite loop */
          && (TYPE_TARGET_ULONG) ulFromVal (tree->right->opval.val) != 0)
        {
          ast *litTree, *litParent;
          litTree = searchLitOp (tree, &litParent, "+-");
          if (litTree)
            {
              if (litTree->opval.op == '+')
                {
                  /* foo_sa */
                  DEBUG_CF ("- 1 SA") ast *tTree = litTree->left;
                  litTree->left = litTree->right;
                  litTree->right = tree->right;
                  tree->right = tTree;
                  tree->opval.op = '+';
                  litTree->opval.op = '-';
                }
              else if (litTree->opval.op == '-')
                {
                  if (IS_LITERAL (RTYPE (litTree)))
                    {
                      /* foo_ssr */
                      DEBUG_CF ("- 2 SSR") ast *tTree = litTree->left;
                      litTree->left = tree->right;
                      tree->right = litParent->left;
                      litParent->left = tTree;
                      litTree->opval.op = '+';

                      tree->decorated = 0;
                      decorateType (tree, resultType);
                    }
                  else
                    {
                      /* foo_ssl */
                      DEBUG_CF ("- 3 SSL") ast *tTree = litTree->right;
                      litTree->right = tree->right;
                      tree->right = tTree;
                    }
                }
              decorateType (litParent, resultType);
            }
        }
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*    complement              */
      /*----------------------------*/
    case '~':
      /* can be only integral type */
      if (!IS_INTEGRAL (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_UNARY_OP, tree->opval.op);
          goto errorTreeReturn;
        }

      /* if left is a literal then do it */
      if (IS_LITERAL (LTYPE (tree)))
        {
          tree->type = EX_VALUE;
          tree->opval.val = valComplement (valFromType (LETYPE (tree)));
          tree->left = NULL;
          TETYPE (tree) = TTYPE (tree) = tree->opval.val->type;
          return addCast (tree, resultTypeProp, TRUE);
        }

      if (resultType == RESULT_TYPE_BOOL && IS_UNSIGNED (tree->left->etype) && getSize (tree->left->etype) < INTSIZE)
        {
          /* promotion rules are responsible for this strange result:
             bit -> int -> ~int -> bit
             uchar -> int -> ~int -> bit
           */
          werrorfl (tree->filename, tree->lineno, W_COMPLEMENT);

          /* optimize bit-result, even if we optimize a buggy source */
          tree->type = EX_VALUE;
          tree->opval.val = constBoolVal (1);
        }
      else
        tree->left = addCast (tree->left, resultTypeProp, TRUE);
      LRVAL (tree) = 1;
      COPYTYPE (TTYPE (tree), TETYPE (tree), LTYPE (tree));
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*           not              */
      /*----------------------------*/
    case '!':
      /* can be pointer */
      if (!IS_ARITHMETIC (LTYPE (tree)) && !IS_PTR (LTYPE (tree)) && !IS_FUNC (LTYPE (tree)) && !IS_ARRAY (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_UNARY_OP, tree->opval.op);
          goto errorTreeReturn;
        }

      /* if left is another '!' */
#if 0 /* Disabled optimization due to bugs #2548, #2551. */
      if (IS_AST_NOT_OPER (tree->left))
        {
          if (resultType == RESULT_TYPE_IFX || resultType == RESULT_TYPE_BOOL))
            {
              /* replace double '!!X' by 'X' */
              return tree->left->left;
            }

          /* remove double '!!X' by 'X ? 1 : 0' */ /* TODO: Casts to _Bools tend to result in far more efficient code than '?' */
          tree->opval.op = '?';
          tree->left = tree->left->left;
          tree->right = newNode (':', newAst_VALUE (constBoolVal (1)), newAst_VALUE (constBoolVal (0)));
          tree->right->filename = tree->filename;
          tree->right->lineno = tree->lineno;
          tree->decorated = 0;
          return decorateType (tree, resultType);
        }
#endif

      /* if left is a literal then do it */
      if (IS_LITERAL (LTYPE (tree)))
        {
          rewriteAstNodeVal (tree, valNot (valFromType (LETYPE (tree))));
          return decorateType (tree, resultType);
        }
      LRVAL (tree) = 1;
      TTYPE (tree) = TETYPE (tree) = (resultTypeProp == RESULT_TYPE_BOOL) ? newBoolLink () : newCharLink ();
      if (IS_BOOLEAN (LTYPE (tree)))
        SPEC_USIGN (TTYPE (tree)) = SPEC_USIGN (LTYPE (tree));
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*           shift            */
      /*----------------------------*/
    case RRC:
    case RLC:
    case SWAP:
      TTYPE (tree) = LTYPE (tree);
      TETYPE (tree) = LETYPE (tree);
      return tree;

    case GETHBIT:
    case GETABIT:
      TTYPE (tree) = TETYPE (tree) = (resultTypeProp == RESULT_TYPE_BOOL) ? newBoolLink () : newCharLink ();
      return tree;

    case GETBYTE:
      TTYPE (tree) = TETYPE (tree) = newCharLink ();
      return tree;

    case GETWORD:
      TTYPE (tree) = TETYPE (tree) = newIntLink ();
      return tree;

    case LEFT_OP:
    case RIGHT_OP:
      if (!IS_INTEGRAL (LTYPE (tree)) || !IS_INTEGRAL (tree->left->etype))
        {
          werrorfl (tree->filename, tree->lineno, E_SHIFT_OP_INVALID);
          werrorfl (tree->filename, tree->lineno, W_CONTINUE, "left & right types are ");
          printTypeChain (LTYPE (tree), stderr);
          fprintf (stderr, ",");
          printTypeChain (RTYPE (tree), stderr);
          fprintf (stderr, "\n");
          goto errorTreeReturn;
        }

      /* make smaller type only if it's a LEFT_OP */
      if (tree->opval.op == LEFT_OP)
        tree->left = addCast (tree->left, resultTypeProp, TRUE);

      /* if they are both literal then */
      /* rewrite the tree */
      if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
        {
          rewriteAstNodeVal (tree, valShift (valFromType (LETYPE (tree)), valFromType (RETYPE (tree)), (tree->opval.op == LEFT_OP ? 1 : 0)));
          return decorateType (tree, resultType);
        }

      /* see if this is a GETBYTE operation if yes
         then return that */
      {
        ast *otree = optimizeGetByte (tree, resultType);

        if (otree != tree)
          return decorateType (otree, RESULT_TYPE_NONE);
      }

      /* see if this is a GETWORD operation if yes
         then return that */
      {
        ast *otree = optimizeGetWord (tree, resultType);

        if (otree != tree)
          return decorateType (otree, RESULT_TYPE_NONE);
      }

      LRVAL (tree) = RRVAL (tree) = 1;
      if (tree->opval.op == LEFT_OP)
        {
          TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), NULL, resultType, tree->opval.op));
        }
      else                      /* RIGHT_OP */
        {
          /* no promotion necessary */
          TTYPE (tree) = TETYPE (tree) = copyLinkChain (LTYPE (tree));
          if (IS_LITERAL (TTYPE (tree)))
            SPEC_SCLS (TTYPE (tree)) &= ~S_LITERAL;
        }

      /* if only the right side is a literal & we are
         shifting more than size of the left operand then zero */
      if (IS_LITERAL (RTYPE (tree)) &&
          ((TYPE_TARGET_ULONG) ulFromVal (valFromType (RETYPE (tree)))) >= (getSize (TETYPE (tree)) * 8))
        {
          if (tree->opval.op == LEFT_OP || (tree->opval.op == RIGHT_OP && SPEC_USIGN (LETYPE (tree))))
            {
              werrorfl (tree->filename, tree->lineno, W_SHIFT_CHANGED, (tree->opval.op == LEFT_OP ? "left" : "right"));
              /* Change shift op to comma op and replace the right operand with 0. */
              /* This preserves the left operand in case there were side-effects. */
              tree->opval.op = ',';
              tree->right->opval.val = constVal ("0");
              TETYPE (tree) = TTYPE (tree) = tree->right->opval.val->type;
              return tree;
            }
        }

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*         casting            */
      /*----------------------------*/
    case CAST:                 /* change the type   */
      /* cannot cast to an aggregate type */
      if (IS_AGGREGATE (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_CAST_ILLEGAL);
          goto errorTreeReturn;
        }

      /* make sure the type is complete and sane */
      if ((resultType == RESULT_TYPE_GPTR) && IS_FUNCPTR (LTYPE (tree)))
        changePointer (LTYPE (tree)->next);
      else
        changePointer (LTYPE (tree));
      checkTypeSanity (LETYPE (tree), "(cast)");

      /* if 'from' and 'to' are the same remove the superfluous cast,
       * this helps other optimizations */
      if (compareTypeExact (LTYPE (tree), RTYPE (tree), -1) == 1)
        {
          /* mark that the explicit cast has been removed,
           * for proper processing (no integer promotion) of explicitly typecasted variable arguments */
          tree->right->values.cast.removedCast = 1;
          return tree->right;
        }

      /* If code memory is read only, then pointers to code memory */
      /* implicitly point to constants -- make this explicit       */
      CodePtrPointsToConst (LTYPE (tree));

#if 0
      /* if the right is a literal replace the tree */
      if (IS_LITERAL (RETYPE (tree)))
        {
          if (!IS_PTR (LTYPE (tree)))
            {
              tree->type = EX_VALUE;
              tree->opval.val = valCastLiteral (LTYPE (tree), floatFromVal (valFromType (RETYPE (tree))));
              tree->left = NULL;
              tree->right = NULL;
              TTYPE (tree) = tree->opval.val->type;
              tree->values.cast.literalFromCast = 1;
            }
          else if (IS_GENPTR (LTYPE (tree)) && !IS_PTR (RTYPE (tree)) && ((int) ulFromVal (valFromType (RETYPE (tree)))) != 0)  /* special case of NULL */
            {
              sym_link *rest = LTYPE (tree)->next;
              werrorfl (tree->filename, tree->lineno, W_LITERAL_GENERIC);
              TTYPE (tree) = newLink (DECLARATOR);
              DCL_TYPE (TTYPE (tree)) = FPOINTER;
              TTYPE (tree)->next = rest;
              tree->left->opval.lnk = TTYPE (tree);
              LRVAL (tree) = 1;
            }
          else
            {
              TTYPE (tree) = LTYPE (tree);
              LRVAL (tree) = 1;
            }
        }
      else
        {
          TTYPE (tree) = LTYPE (tree);
          LRVAL (tree) = 1;
        }
#else
#if 0                           // this is already checked, now this could be explicit
      /* if pointer to struct then check names */
      if (IS_PTR (LTYPE (tree)) && IS_STRUCT (LTYPE (tree)->next) &&
          IS_PTR (RTYPE (tree)) && IS_STRUCT (RTYPE (tree)->next) &&
          strcmp (SPEC_STRUCT (LETYPE (tree))->tag, SPEC_STRUCT (RETYPE (tree))->tag))
        {
          werrorfl (tree->filename, tree->lineno, W_CAST_STRUCT_PTR, SPEC_STRUCT (RETYPE (tree))->tag,
                    SPEC_STRUCT (LETYPE (tree))->tag);
        }
#endif
#if 0                           // disabled to fix bug 2941749
      if (IS_ADDRESS_OF_OP (tree->right)
          && IS_AST_SYM_VALUE (tree->right->left) && SPEC_ABSA (AST_SYMBOL (tree->right->left)->etype))
        {
          symbol *sym = AST_SYMBOL (tree->right->left);
          unsigned int gptype = 0;
          unsigned int addr = SPEC_ADDR (sym->etype);

          if (IS_GENPTR (LTYPE (tree)) && ((GPTRSIZE > FARPTRSIZE) || TARGET_IS_PIC16))
            {
              switch (SPEC_SCLS (sym->etype))
                {
                case S_CODE:
                  gptype = GPTYPE_CODE;
                  break;
                case S_XDATA:
                  gptype = GPTYPE_FAR;
                  break;
                case S_DATA:
                case S_IDATA:
                  gptype = GPTYPE_NEAR;
                  break;
                case S_PDATA:
                  gptype = GPTYPE_XSTACK;
                  break;
                default:
                  gptype = 0;
                  if (TARGET_IS_PIC16 && (SPEC_SCLS (sym->etype) == S_FIXED))
                    gptype = GPTYPE_NEAR;
                }
              addr |= gptype << (8 * (GPTRSIZE - 1));
            }

          tree->type = EX_VALUE;
          tree->opval.val = valCastLiteral (LTYPE (tree), addr);
          TTYPE (tree) = tree->opval.val->type;
          TETYPE (tree) = getSpec (TTYPE (tree));
          tree->left = NULL;
          tree->right = NULL;
          tree->values.cast.literalFromCast = 1;
          return tree;
        }
#endif

      /* if the right is a literal replace the tree */
      if (IS_LITERAL (RETYPE (tree)))
        {
#if 0
          if (IS_PTR (LTYPE (tree)) && !IS_GENPTR (LTYPE (tree)))
            {
              /* rewrite      (type *)litaddr
                 as           &temp
                 and define   type at litaddr temp
                 (but only if type's storage class is not generic)
               */
              ast *newTree = newNode ('&', NULL, NULL);
              symbol *sym;

              TTYPE (newTree) = LTYPE (tree);
              TETYPE (newTree) = getSpec (LTYPE (tree));

              /* define a global symbol at the casted address */
              sym = newSymbol (genSymName (0), 0);
              sym->type = LTYPE (tree)->next;
              if (!sym->type)
                sym->type = newLink (V_VOID);
              sym->etype = getSpec (sym->type);
              SPEC_SCLS (sym->etype) = sclsFromPtr (LTYPE (tree));
              sym->lineDef = tree->lineno;
              sym->cdef = 1;
              sym->isref = 1;
              SPEC_STAT (sym->etype) = 1;
              SPEC_ADDR (sym->etype) = floatFromVal (valFromType (RTYPE (tree)));
              SPEC_ABSA (sym->etype) = 1;
              addSym (SymbolTab, sym, sym->name, 0, 0, 0);
              allocGlobal (sym);

              newTree->left = newAst_VALUE (symbolVal (sym));
              newTree->left->filename = tree->filename;
              newTree->left->lineno = tree->lineno;
              LTYPE (newTree) = sym->type;
              LETYPE (newTree) = sym->etype;
              LLVAL (newTree) = 1;
              LRVAL (newTree) = 0;
              TLVAL (newTree) = 1;
              return newTree;
            }
#endif
          if (!IS_PTR (LTYPE (tree)))
            {
              tree->type = EX_VALUE;
              tree->opval.val = valCastLiteral (LTYPE (tree), floatFromVal (valFromType (RTYPE (tree))), (TYPE_TARGET_ULONGLONG) ullFromVal (valFromType (RTYPE (tree))));
              TTYPE (tree) = tree->opval.val->type;
              tree->left = NULL;
              tree->right = NULL;
              tree->values.cast.literalFromCast = 1;
              TETYPE (tree) = getSpec (TTYPE (tree));
              return tree;
            }
          else
            {
              unsigned long long gpVal = 0;
              int size = getSize(LTYPE (tree));
              unsigned long long mask = (size >= sizeof(long long)) ? 0xffffffffffffffffull : (1ull << (size * 8)) - 1;
              unsigned long long pVal = ullFromVal (valFromType (RTYPE (tree))) & mask;

              /* if casting literal specific pointer to generic pointer */
              if (IS_GENPTR (LTYPE (tree)) && IS_PTR (RTYPE (tree)) && !IS_GENPTR (RTYPE (tree)))
                {
                  if (resultType != RESULT_TYPE_GPTR)
                    {
                      DCL_TYPE (LTYPE (tree)) = DCL_TYPE (RTYPE (tree));
                    }
                  else
                    {
                      gpVal = pointerTypeToGPByte (DCL_TYPE (RTYPE (tree)), NULL, NULL);
                      gpVal <<= getSize (RTYPE (tree)) * 8;
                      gpVal &= mask;
                    }
                }
              checkPtrCast (LTYPE (tree), RTYPE (tree), tree->values.cast.implicitCast, !ullFromVal (valFromType (RTYPE (tree))));
              LRVAL (tree) = 1;
              tree->type = EX_VALUE;
              tree->opval.val = valCastLiteral (LTYPE (tree), gpVal | pVal, gpVal | pVal);
              TTYPE (tree) = tree->opval.val->type;
              tree->left = NULL;
              tree->right = NULL;
              tree->values.cast.literalFromCast = 1;
              TETYPE (tree) = getSpec (TTYPE (tree));
              return tree;
            }
        }
      checkPtrCast (LTYPE (tree), RTYPE (tree), tree->values.cast.implicitCast, FALSE);
      if (IS_GENPTR (LTYPE (tree)) && (resultType != RESULT_TYPE_GPTR))
        {
          if (IS_PTR (RTYPE (tree)) && !IS_GENPTR (RTYPE (tree)))
            DCL_TYPE (LTYPE (tree)) = DCL_TYPE (RTYPE (tree));
          if (IS_ARRAY (RTYPE (tree)) && SPEC_OCLS (RETYPE (tree)))
            DCL_TYPE (LTYPE (tree)) = PTR_TYPE (SPEC_OCLS (RETYPE (tree)));
        }
      TTYPE (tree) = LTYPE (tree);
      LRVAL (tree) = 1;

#endif
      TETYPE (tree) = getSpec (TTYPE (tree));

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*       logical &&, ||       */
      /*----------------------------*/
    case AND_OP:
    case OR_OP:
      /* each must be arithmetic type or be a pointer */
      if (!IS_PTR (LTYPE (tree)) && !IS_ARRAY (LTYPE (tree)) && !IS_INTEGRAL (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_COMPARE_OP);
          goto errorTreeReturn;
        }

      if (!IS_PTR (RTYPE (tree)) && !IS_ARRAY (RTYPE (tree)) && !IS_INTEGRAL (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_COMPARE_OP);
          goto errorTreeReturn;
        }
      /* if they are both literal then */
      /* rewrite the tree */
      if (IS_LITERAL (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
        {
          rewriteAstNodeVal (tree, valLogicAndOr (valFromType (LETYPE (tree)), valFromType (RETYPE (tree)), tree->opval.op));
          return decorateType (tree, resultType);
        }
      LRVAL (tree) = RRVAL (tree) = 1;
      TTYPE (tree) = TETYPE (tree) = (resultTypeProp == RESULT_TYPE_BOOL) ? newBoolLink () : newCharLink ();
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*     comparison operators   */
      /*----------------------------*/
    case '>':
    case '<':
    case LE_OP:
    case GE_OP:
    case EQ_OP:
    case NE_OP:
      {
        ast *lt = optimizeCompare (tree);

        if (tree != lt)
          return lt;
      }

      /* C does not allow comparison of struct or union. */
      if (IS_STRUCT (LTYPE (tree)) || IS_STRUCT (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_COMPARE_OP);
          goto errorTreeReturn;
        }

      /* if they are pointers they must be castable */
      else if (IS_PTR (LTYPE (tree)) && IS_PTR (RTYPE (tree)))
        {
          if (tree->opval.op == EQ_OP && !IS_GENPTR (LTYPE (tree)) && IS_GENPTR (RTYPE (tree)))
            {
              // we cannot cast a gptr to a !gptr: switch the leaves
              struct ast *s = tree->left;
              tree->left = tree->right;
              tree->right = s;
            }
          if (compareType (LTYPE (tree), RTYPE (tree)) == 0)
            {
              werrorfl (tree->filename, tree->lineno, E_INCOMPAT_TYPES);
              fprintf (stderr, "comparing type ");
              printTypeChain (LTYPE (tree), stderr);
              fprintf (stderr, " to type ");
              printTypeChain (RTYPE (tree), stderr);
              fprintf (stderr, "\n");
              goto errorTreeReturn;
            }
        }
      /* else they should be promotable to one another */
      else
        {
          if (!((IS_PTR (LTYPE (tree)) && IS_LITERAL (RTYPE (tree)))
                || (IS_PTR (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)))
                || ((tree->opval.op == EQ_OP || tree->opval.op == NE_OP)
                    && ((IS_FUNC (LTYPE (tree)) && IS_LITERAL (RTYPE (tree)) && !ullFromVal (valFromType (RTYPE (tree))))
                        || (IS_FUNC (RTYPE (tree)) && IS_LITERAL (LTYPE (tree)) && !ullFromVal (valFromType (LTYPE (tree))))))))

            if (compareType (LTYPE (tree), RTYPE (tree)) == 0)
              {
                if (compareType (RTYPE (tree), LTYPE (tree)) != 0)
                  {
                    struct ast *s = tree->left;
                    tree->left = tree->right;
                    tree->right = s;
                    if (tree->opval.op == '>')
                      tree->opval.op = '<';
                    else if (tree->opval.op == '>')
                      tree->opval.op = '<';
                    else if (tree->opval.op == LE_OP)
                      tree->opval.op = GE_OP;
                    else if (tree->opval.op == GE_OP)
                      tree->opval.op = LE_OP;
                  }
                else
                  {
                    werrorfl (tree->filename, tree->lineno, E_INCOMPAT_TYPES);
                    fprintf (stderr, "comparing type ");
                    printTypeChain (LTYPE (tree), stderr);
                    fprintf (stderr, " to type ");
                    printTypeChain (RTYPE (tree), stderr);
                    fprintf (stderr, "\n");
                    goto errorTreeReturn;
                  }
              }
        }

      {
        CCR_RESULT ccr_result = CCR_OK;
        ast * newResult;

        /* if left is integral and right is literal
           then check constant range */
        if (IS_INTEGRAL (LTYPE (tree)) && !IS_LITERAL (LTYPE (tree)) && IS_LITERAL (RTYPE (tree)))
          ccr_result = checkConstantRange (LTYPE (tree), RTYPE (tree), tree->opval.op, FALSE);
        if (ccr_result == CCR_OK && IS_INTEGRAL (RTYPE (tree)) && !IS_LITERAL (RTYPE (tree))  && IS_LITERAL (LTYPE (tree)))
          ccr_result = checkConstantRange (RTYPE (tree), LTYPE (tree), tree->opval.op, TRUE);
        switch (ccr_result)
          {
          case CCR_ALWAYS_TRUE:
          case CCR_ALWAYS_FALSE:
            werrorfl (tree->filename, tree->lineno, W_COMP_RANGE, ccr_result == CCR_ALWAYS_TRUE ? "true" : "false");
            newResult = newAst_VALUE (constBoolVal ((unsigned char) (ccr_result == CCR_ALWAYS_TRUE)));
            /* If there are side effects, join the non-literal side */
            /* to the boolean result with a comma operator */
            if (hasSEFcalls (tree))
              {
                if (!IS_LITERAL (LTYPE (tree)))
                  newResult = newNode (',', tree->left, newResult);
                else
                  newResult = newNode (',', tree->right, newResult);
              }
            return decorateType (newResult, resultType);
          case CCR_OK:
          default:
            break;
          }
      }

      /* if (unsigned value) > 0 then '(unsigned value) ? 1 : 0' */
      if (tree->opval.op == '>' &&
          SPEC_USIGN (LETYPE (tree)) && IS_LITERAL (RTYPE (tree)) && ((int) ulFromVal (valFromType (RETYPE (tree)))) == 0)
        {
          if ((resultType == RESULT_TYPE_IFX) || (resultType == RESULT_TYPE_BOOL))
            {
              /* the parent is an ifx: */
              /* if (unsigned value) */
              return tree->left;
            }

          /* (unsigned value) ? 1 : 0 */ /* TODO: Casts to _Bools tend to result in far more efficient code than '?' */
          tree->opval.op = '?';
          tree->right = newNode (':', newAst_VALUE (constBoolVal (1)), tree->right);    /* val 0 */
          tree->right->filename = tree->filename;
          tree->right->lineno = tree->lineno;
          tree->right->left->filename = tree->filename;
          tree->right->left->lineno = tree->lineno;
          tree->decorated = 0;
          return decorateType (tree, resultType);
        }

      /* 'ifx (0 == op)' -> 'ifx (!(op))' */
      if (IS_LITERAL (LETYPE (tree)) &&
          floatFromVal (valFromType (LTYPE (tree))) == 0 &&
          tree->opval.op == EQ_OP && (resultType == RESULT_TYPE_IFX || resultType == RESULT_TYPE_BOOL))
        {
          rewriteAstNodeOp (tree, '!', tree->right, NULL);
          return decorateType (tree, resultType);
        }

      /* 'ifx (op == 0)' -> 'ifx (!(op))' */
      if (IS_LITERAL (RETYPE (tree)) &&
          floatFromVal (valFromType (RTYPE (tree))) == 0 &&
          tree->opval.op == EQ_OP && (resultType == RESULT_TYPE_IFX || resultType == RESULT_TYPE_BOOL))
        {
          rewriteAstNodeOp (tree, '!', tree->left, NULL);
          return decorateType (tree, resultType);
        }

      /* 'ifx (op == 1)' -> 'ifx (op)' for bool */
      if (IS_LITERAL (RETYPE (tree)) &&
          floatFromVal (valFromType (RTYPE (tree))) == 1 && IS_BOOLEAN (LETYPE (tree)) &&
          tree->opval.op == EQ_OP && (resultType == RESULT_TYPE_IFX || resultType == RESULT_TYPE_BOOL))
        {
          tree = tree->left;
          return decorateType (tree, resultType);
        }

      /* if they are both literal then */
      /* rewrite the tree */
      if (IS_LITERAL (RETYPE (tree)) && IS_LITERAL (LETYPE (tree)))
        {
          rewriteAstNodeVal (tree, valCompare (valFromType (LETYPE (tree)), valFromType (RETYPE (tree)), tree->opval.op));
          return decorateType (tree, resultType);
        }

      /* if one is 'signed char ' and the other one is 'unsigned char' */
      /* it's necessary to promote to int */
      if (IS_CHAR (RTYPE (tree)) && IS_CHAR (LTYPE (tree)) && (IS_UNSIGNED (RTYPE (tree)) != IS_UNSIGNED (LTYPE (tree))))
        {
          /* Small literal integers are 'optimized' to 'unsigned char' but chars in single quotes are 'char'.
             Try to figure out, if it's possible to do without a cast to integer */

          /* is right a small literal char? */
          if (IS_LITERAL (RTYPE (tree)))
            {
              int val = (int) ulFromVal (valFromType (RETYPE (tree)));
              /* the overlapping value range of a '(un)signed char' is 0...127;
                 if 0 <= the actual value < 128 it can be changed to (un)signed */
              if (val >= 0 && val < 128)
                {
                  /* now we've got 2 '(un)signed char'! */
                  SPEC_USIGN (RETYPE (tree)) = SPEC_USIGN (LETYPE (tree));
                }
            }
          /* same test for the left operand: */
          else if (IS_LITERAL (LTYPE (tree)))
            {
              int val = (int) ulFromVal (valFromType (LETYPE (tree)));
              if (val >= 0 && val < 128)
                {
                  SPEC_USIGN (LETYPE (tree)) = SPEC_USIGN (RETYPE (tree));
                }
            }
          else
            {
              werrorfl (tree->filename, tree->lineno, W_CMP_SU_CHAR);
              tree->left = addCast (tree->left, RESULT_TYPE_INT, TRUE);
              tree->right = addCast (tree->right, RESULT_TYPE_INT, TRUE);
            }
        }

      LRVAL (tree) = RRVAL (tree) = 1;
      TTYPE (tree) = TETYPE (tree) = (resultType == RESULT_TYPE_BOOL) ? newBoolLink () : newCharLink ();

      /* condition transformations */
      {
        unsigned transformedOp = 0;

        switch (tree->opval.op)
          {
          case '<':            /* transform (a < b)  to !(a >= b)  */
            if (port->lt_nge)
              transformedOp = GE_OP;
            break;
          case '>':            /* transform (a > b)  to !(a <= b)  */
            if (port->gt_nle)
              transformedOp = LE_OP;
            break;
          case LE_OP:          /* transform (a <= b) to !(a > b)   */
            if (port->le_ngt)
              transformedOp = '>';
            break;
          case GE_OP:          /* transform (a >= b) to !(a < b)   */
            if (port->ge_nlt)
              transformedOp = '<';
            break;
          case NE_OP:          /* transform (a != b) to !(a == b)   */
            if (port->ne_neq)
              transformedOp = EQ_OP;
            break;
          case EQ_OP:          /* transform (a == b) to !(a != b)   */
            if (port->eq_nne)
              transformedOp = NE_OP;
            break;
          default:
            break;
          }
        if (transformedOp)
          {
            tree->opval.op = transformedOp;
            tree->decorated = 0;
            tree = newNode ('!', tree, NULL);
            tree->filename = tree->left->filename;
            tree->lineno = tree->left->lineno;
            return decorateType (tree, resultType);
          }
      }

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*             sizeof         */
      /*----------------------------*/
    case SIZEOF:               /* evaluate without code generation */
      {
        /* change the type to a integer */
        struct dbuf_s dbuf;
        int size = getSize (tree->right->ftype);

        dbuf_init (&dbuf, 128);
        dbuf_printf (&dbuf, "%d", size);
        if (!size && !IS_VOID (tree->right->ftype))
          werrorfl (tree->filename, tree->lineno, E_SIZEOF_INCOMPLETE_TYPE);
        tree->type = EX_VALUE;
        tree->opval.val = constVal (dbuf_c_str (&dbuf));
        dbuf_destroy (&dbuf);
        tree->right = tree->left = NULL;
        TETYPE (tree) = getSpec (TTYPE (tree) = tree->opval.val->type);

        return tree;
      }

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*             typeof         */
      /*----------------------------*/
    case TYPEOF:
      /* return typeof enum value */
      tree->type = EX_VALUE;
      {
        int typeofv = 0;
        struct dbuf_s dbuf;

        if (IS_SPEC (tree->right->ftype))
          {
            switch (SPEC_NOUN (tree->right->ftype))
              {
              case V_INT:
                if (SPEC_LONG (tree->right->ftype))
                  typeofv = TYPEOF_LONG;
                else
                  typeofv = TYPEOF_INT;
                break;
              case V_FLOAT:
                typeofv = TYPEOF_FLOAT;
                break;
              case V_FIXED16X16:
                typeofv = TYPEOF_FIXED16X16;
                break;
              case V_BOOL:
                typeofv = TYPEOF_BOOL;
                break;
              case V_CHAR:
                typeofv = TYPEOF_CHAR;
                break;
              case V_VOID:
                typeofv = TYPEOF_VOID;
                break;
              case V_STRUCT:
                typeofv = TYPEOF_STRUCT;
                break;
              case V_BITFIELD:
                typeofv = TYPEOF_BITFIELD;
                break;
              case V_BIT:
                typeofv = TYPEOF_BIT;
                break;
              case V_SBIT:
                typeofv = TYPEOF_SBIT;
                break;
              default:
                break;
              }
          }
        else
          {
            switch (DCL_TYPE (tree->right->ftype))
              {
              case POINTER:
                typeofv = TYPEOF_POINTER;
                break;
              case FPOINTER:
                typeofv = TYPEOF_FPOINTER;
                break;
              case CPOINTER:
                typeofv = TYPEOF_CPOINTER;
                break;
              case GPOINTER:
                typeofv = TYPEOF_GPOINTER;
                break;
              case PPOINTER:
                typeofv = TYPEOF_PPOINTER;
                break;
              case IPOINTER:
                typeofv = TYPEOF_IPOINTER;
                break;
              case ARRAY:
                typeofv = TYPEOF_ARRAY;
                break;
              case FUNCTION:
                typeofv = TYPEOF_FUNCTION;
                break;
              default:
                break;
              }
          }
        dbuf_init (&dbuf, 128);
        dbuf_printf (&dbuf, "%d", typeofv);
        tree->opval.val = constVal (dbuf_c_str (&dbuf));
        dbuf_destroy (&dbuf);
        tree->right = tree->left = NULL;
        TETYPE (tree) = getSpec (TTYPE (tree) = tree->opval.val->type);
      }
      return tree;
      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /* conditional operator  '?'  */
      /*----------------------------*/
    case '?':
      /* the type is value of the colon operator (on the right) */
      assert (IS_COLON_OP (tree->right));

      /* If already known then replace the tree : optimizer will do it
         but faster to do it here. If done before decorating tree->right
         this can save generating unused const strings. */
      if (IS_LITERAL (LTYPE (tree)))
        {
          ast * heir;
          
          ++noAlloc;
          tree->right = decorateType (tree->right, resultTypeProp);
          --noAlloc;

          if (((int) ulFromVal (valFromType (LETYPE (tree)))) != 0)
            heir = tree->right->left;
          else
            heir = tree->right->right;
            
          heir = decorateType (heir, resultTypeProp);
          if (IS_LITERAL (TETYPE (heir)))
            TTYPE (heir) = valRecastLitVal (TTYPE (tree->right), valFromType (TETYPE (heir)))->type;
          else
            TTYPE (heir) = TTYPE (tree->right);
          TETYPE (heir) = getSpec (TTYPE (heir));
          return heir;
        }

      tree->right = decorateType (tree->right, resultTypeProp);

      if (IS_AST_LIT_VALUE (tree->right->left) && IS_AST_LIT_VALUE (tree->right->right) &&
          ((resultType == RESULT_TYPE_IFX) || (resultType == RESULT_TYPE_BOOL)))
        {
          double valTrue = AST_FLOAT_VALUE (tree->right->left);
          double valFalse = AST_FLOAT_VALUE (tree->right->right);

          if ((valTrue != 0) && (valFalse == 0))
            {
              /* assign cond to result */
              tree->left->decorated = 0;
              return decorateType (tree->left, resultTypeProp);
            }
          else if ((valTrue == 0) && (valFalse != 0))
            {
              /* assign !cond to result */
              tree->opval.op = '!';
              tree->decorated = 0;
              tree->right = NULL;
              return decorateType (tree, resultTypeProp);
            }
          else
            {
              /* they have the same boolean value, make them equal */
              tree->right->left = tree->right->right;
            }
        }

      /* if they are equal then replace the tree */
      if (isAstEqual (tree->right->left, tree->right->right))
        {
          return tree->right->left;
        }

      TTYPE (tree) = RTYPE (tree);
      TETYPE (tree) = getSpec (TTYPE (tree));
      return tree;

    case GENERIC:
      {
        sym_link *type = tree->left->ftype;
        ast *assoc_list;
        ast *default_expr = 0;
        ast *found_expr = 0;

        for(assoc_list = tree->right; assoc_list; assoc_list = assoc_list->left)
          {
            ast *const assoc = assoc_list->right;
            if (!assoc->left)
              {
                if (default_expr)
                  {
                    werror (E_MULTIPLE_DEFAULT_IN_GENERIC);
                    goto errorTreeReturn;
                  }
                default_expr = assoc->right;
              }
            else
              {
                sym_link *assoc_type;
                wassert (IS_AST_LINK (assoc->left));
                assoc_type = assoc->left->opval.lnk;
                checkTypeSanity (assoc_type, "_Generic");

                if (compareType (type, assoc->left->opval.lnk) > 0 && !(SPEC_NOUN (type) == V_CHAR && type->select.s.b_implicit_sign != assoc->left->opval.lnk->select.s.b_implicit_sign))
                  {
                    if (found_expr)
                      {
                        werror (E_MULTIPLE_MATCHES_IN_GENERIC);
                        goto errorTreeReturn;
                      }
                    found_expr = assoc->right;
                  }
              }
          }
        if (!found_expr)
          found_expr = default_expr;

        if (!found_expr)
          {
            werror (E_NO_MATCH_IN_GENERIC);
            goto errorTreeReturn;
          }
        
        tree = found_expr;
      }
      return tree;

    case GENERIC_ASSOC_LIST:
      return tree;

    case GENERIC_ASSOCIATION:
      return tree;

    case ':':
      if ((compareType (LTYPE (tree), RTYPE (tree)) == 0) && (compareType (RTYPE (tree), LTYPE (tree)) == 0))
        {
          if (IS_PTR (LTYPE (tree)) && !IS_GENPTR (LTYPE (tree)))
            DCL_TYPE (LTYPE(tree)) = GPOINTER;
          if (IS_PTR (RTYPE (tree)) && !IS_GENPTR (RTYPE (tree)))
            DCL_TYPE (RTYPE(tree)) = GPOINTER;
        }

      if ((compareType (LTYPE (tree), RTYPE (tree)) == 0) &&
        (compareType (RTYPE (tree), LTYPE (tree)) == 0) &&
        !(IS_ARRAY(LTYPE (tree)) && IS_INTEGRAL(RTYPE (tree))) &&
        !(IS_ARRAY(RTYPE (tree)) && IS_INTEGRAL(LTYPE (tree))))
        {
          werrorfl (tree->filename, tree->lineno, E_TYPE_MISMATCH, "conditional operator", " ");
          printFromToType (RTYPE (tree), LTYPE (tree));
        }

      TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), resultType, tree->opval.op);
      TETYPE (tree) = getSpec (TTYPE (tree));

      return tree;

#if 0                           // assignment operators are converted by the parser
      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*    assignment operators    */
      /*----------------------------*/
    case MUL_ASSIGN:
    case DIV_ASSIGN:
      /* for these it must be both must be integral */
      if (!IS_ARITHMETIC (LTYPE (tree)) || !IS_ARITHMETIC (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_OPS_INTEGRAL);
          goto errorTreeReturn;
        }
      RRVAL (tree) = 1;
      TETYPE (tree) = getSpec (TTYPE (tree) = LTYPE (tree));

      if (!tree->initMode && IS_CONSTANT (LTYPE (tree)))
        werrorfl (tree->filename, tree->lineno, E_CODE_WRITE, tree->opval.op == MUL_ASSIGN ? "*=" : "/=");

      if (LRVAL (tree))
        {
          werrorfl (tree->filename, tree->lineno, E_LVALUE_REQUIRED, tree->opval.op == MUL_ASSIGN ? "*=" : "/=");
          goto errorTreeReturn;
        }
      LLVAL (tree) = 1;

      return tree;

    case AND_ASSIGN:
    case OR_ASSIGN:
    case XOR_ASSIGN:
    case RIGHT_ASSIGN:
    case LEFT_ASSIGN:
      /* for these it must be both must be integral */
      if (!IS_INTEGRAL (LTYPE (tree)) || !IS_INTEGRAL (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_OPS_INTEGRAL);
          goto errorTreeReturn;
        }
      RRVAL (tree) = 1;
      TETYPE (tree) = getSpec (TTYPE (tree) = LTYPE (tree));

      if (!tree->initMode && IS_CONSTANT (LETYPE (tree)))
        werrorfl (tree->filename, tree->lineno, E_CODE_WRITE, "&= or |= or ^= or >>= or <<=");

      if (LRVAL (tree))
        {
          werrorfl (tree->filename, tree->lineno, E_LVALUE_REQUIRED, "&= or |= or ^= or >>= or <<=");
          goto errorTreeReturn;
        }
      LLVAL (tree) = 1;

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*    -= operator             */
      /*----------------------------*/
    case SUB_ASSIGN:
      if (!(IS_PTR (LTYPE (tree)) || IS_ARITHMETIC (LTYPE (tree))))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "-=");
          goto errorTreeReturn;
        }

      if (!(IS_PTR (RTYPE (tree)) || IS_ARITHMETIC (RTYPE (tree))))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "-=");
          goto errorTreeReturn;
        }
      RRVAL (tree) = 1;
      TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), RESULT_TYPE_NOPROM, tree->opval.op));

      if (!tree->initMode && IS_CONSTANT (LETYPE (tree)))
        werrorfl (tree->filename, tree->lineno, E_CODE_WRITE, "-=");

      if (LRVAL (tree))
        {
          werrorfl (tree->filename, tree->lineno, E_LVALUE_REQUIRED, "-=");
          goto errorTreeReturn;
        }
      LLVAL (tree) = 1;

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*          += operator       */
      /*----------------------------*/
    case ADD_ASSIGN:
      /* this is not a unary operation */
      /* if both pointers then problem */
      if (IS_PTR (LTYPE (tree)) && IS_PTR (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_PTR_PLUS_PTR);
          goto errorTreeReturn;
        }

      if (!IS_ARITHMETIC (LTYPE (tree)) && !IS_PTR (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "+=");
          goto errorTreeReturn;
        }

      if (!IS_ARITHMETIC (RTYPE (tree)) && !IS_PTR (RTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_PLUS_INVALID, "+=");
          goto errorTreeReturn;
        }
      RRVAL (tree) = 1;
      TETYPE (tree) = getSpec (TTYPE (tree) = computeType (LTYPE (tree), RTYPE (tree), RESULT_TYPE_NOPROM, tree->opval.op));

      if (!tree->initMode && IS_CONSTANT (LETYPE (tree)))
        werrorfl (tree->filename, tree->lineno, E_CODE_WRITE, "+=");

      if (LRVAL (tree))
        {
          werrorfl (tree->filename, tree->lineno, E_LVALUE_REQUIRED, "+=");
          goto errorTreeReturn;
        }

      tree->right = decorateType (newNode ('+', copyAst (tree->left), tree->right), RESULT_TYPE_NONE);
      tree->opval.op = '=';

      return tree;
#endif

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      straight assignemnt   */
      /*----------------------------*/
    case '=':
      /* cannot be an array */
      if (IS_ARRAY (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_ARRAY_ASSIGN);
          goto errorTreeReturn;
        }

      /* they should either match or be castable */
      if (compareType (LTYPE (tree), RTYPE (tree)) == 0)
        {
          if (IS_CODEPTR (LTYPE (tree)) && IS_FUNC (LTYPE (tree)->next))        /* function pointer */
            {
              werrorfl (tree->filename, tree->lineno, E_INCOMPAT_TYPES);
              printFromToType (RTYPE (tree), LTYPE (tree)->next);
            }
          else
            {
              int lineno = tree->lineno ? tree->lineno : tree->left->lineno ? tree->left->lineno : tree->right->lineno;
              werrorfl (tree->filename, lineno, E_TYPE_MISMATCH, "assignment", " ");
              printFromToType (RTYPE (tree), LTYPE (tree));
            }
        }

      /* if the left side of the tree is of type void
         then report error */
      if (IS_VOID (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_CAST_ZERO);
          printFromToType (RTYPE (tree), LTYPE (tree));
        }

      TETYPE (tree) = getSpec (TTYPE (tree) = LTYPE (tree));
      if (IS_STRUCT (LTYPE (tree)))
        tree = rewriteStructAssignment (tree);
      else
        {
          RRVAL (tree) = 1;
          LLVAL (tree) = 1;
        }
      if (!tree->initMode)
        {
          if (IS_CONSTANT (LTYPE (tree)))
            werrorfl (tree->filename, tree->lineno, E_CODE_WRITE, "=");
        }
      if (tree->initMode && SPEC_STAT (getSpec (LTYPE (tree))) && !constExprTree (tree->right))
        werrorfl (tree->filename, tree->lineno, E_CONST_EXPECTED, "=");
      if (LRVAL (tree))
        {
          werrorfl (tree->filename, tree->lineno, E_LVALUE_REQUIRED, "=");
          goto errorTreeReturn;
        }

      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      comma operator        */
      /*----------------------------*/
    case ',':
      TETYPE (tree) = getSpec (TTYPE (tree) = RTYPE (tree));
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*       function call        */
      /*----------------------------*/
    case CALL:
      if (IFFUNC_ISCRITICAL (LTYPE(tree)) && (inCriticalFunction || inCriticalBlock))
        werror (E_INVALID_CRITICAL);

      /* undo any explicit pointer dereference; PCALL will handle it instead */
      if (IS_FUNC (LTYPE (tree)) && tree->left->type == EX_OP)
        {
          if (tree->left->opval.op == '*' && !tree->left->right)
            tree->left = tree->left->left;
        }

      /* require a function or pointer to function */
      if (!IS_FUNC (LTYPE (tree)) && !IS_FUNCPTR (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_FUNCTION_EXPECTED);
          goto errorTreeReturn;
        }

      /* if there are parms, make sure that
         parms are decorate / process / reverse only once */
      if (!tree->right || !tree->right->decorated)
        {
          sym_link *functype;
          parmNumber = 1;

          if (IS_FUNCPTR (LTYPE (tree)))
            {
              functype = LTYPE (tree)->next;
              processFuncPtrArgs (functype);
            }
          else
            functype = LTYPE (tree);

          if (tree->right && tree->right->reversed)
            reverseParms (tree->right, 0);

          if (processParms (tree->left, FUNC_ARGS (functype), &tree->right, &parmNumber, TRUE))
            goto errorTreeReturn;

          if (!optimize.noStdLibCall)
            optStdLibCall (tree, resultType);

          if ((options.stackAuto || IFFUNC_ISREENT (functype)) && !IFFUNC_ISBUILTIN (functype))
            reverseParms (tree->right, 1);

          TTYPE (tree) = copyLinkChain(functype->next);
          TETYPE (tree) = getSpec (TTYPE (tree));
          SPEC_SCLS (TETYPE (tree)) = S_FIXED;
        }
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*     return statement       */
      /*----------------------------*/
    case RETURN:
      if (tree->right)
        {
          int typecompat;

          if (IS_VOID (currFunc->type->next) && tree->right)
            {
              if (!IS_VOID (RTYPE (tree)) || !options.std_sdcc)
                {
                  werrorfl (tree->filename, tree->lineno, E_FUNC_VOID);
                  goto errorTreeReturn;
                }
            }

          typecompat = compareType (currFunc->type->next, RTYPE (tree));

          /* if there is going to be a casting required then add it */
          if (typecompat == -1)
            {
              tree->right = newNode (CAST,
                                     newAst_LINK (copyLinkChain (currFunc->type->next)),
                                     tree->right);
              tree->right->values.cast.implicitCast = 1;
              tree->right->lineno = tree->right->left->lineno = tree->lineno;
              tree->right->filename = tree->right->left->filename = tree->filename;
              tree->right = decorateType (tree->right, IS_GENPTR (currFunc->type->next) ? RESULT_TYPE_GPTR : RESULT_TYPE_NONE);
            }
          else if (!typecompat)
            {
              werrorfl (tree->filename, tree->lineno, W_RETURN_MISMATCH);
              printFromToType (RTYPE (tree), currFunc->type->next);
            }

          RRVAL (tree) = 1;
        }
      else /* no return value specified */
        {
          if (!IS_VOID (currFunc->type->next) && tree->right == NULL)
            {
              werrorfl (tree->filename, tree->lineno, W_VOID_FUNC, currFunc->name);
              /* We will return an undefined value */
            }

          TTYPE (tree) = TETYPE (tree) = NULL;
        }
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*     switch statement       */
      /*----------------------------*/
    case SWITCH:
      /* the switch value must be an integer */
      if (!IS_INTEGRAL (LTYPE (tree)))
        {
          werrorfl (tree->filename, tree->lineno, E_SWITCH_NON_INTEGER);
          goto errorTreeReturn;
        }
      LRVAL (tree) = 1;
      TTYPE (tree) = TETYPE (tree) = NULL;
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /* ifx Statement              */
      /*----------------------------*/
    case IFX:
      tree->left = backPatchLabels (tree->left, tree->trueLabel, tree->falseLabel);
      TTYPE (tree) = TETYPE (tree) = NULL;
      return tree;

      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /* for Statement              */
      /*----------------------------*/
    case FOR:

      AST_FOR (tree, initExpr) = decorateType (resolveSymbols (AST_FOR (tree, initExpr)), RESULT_TYPE_NONE);
      AST_FOR (tree, condExpr) = decorateType (resolveSymbols (AST_FOR (tree, condExpr)), RESULT_TYPE_NONE);
      AST_FOR (tree, loopExpr) = decorateType (resolveSymbols (AST_FOR (tree, loopExpr)), RESULT_TYPE_NONE);

      /* if the for loop is reversible then
         reverse it otherwise do what we normally
         do */
      {
        symbol *sym;
        ast *init, *end;

        if (!AST_FOR (tree, continueLabel)->isref &&
            !AST_FOR (tree, falseLabel)->isref &&
            isLoopReversible (tree, &sym, &init, &end))
          return reverseLoop (tree, sym, init, end);
        else if (isInitiallyTrue (AST_FOR (tree, initExpr), AST_FOR (tree, condExpr)))
          {
            tree = createDoFor (AST_FOR (tree, trueLabel),
                                AST_FOR (tree, continueLabel),
                                AST_FOR (tree, falseLabel),
                                AST_FOR (tree, condLabel),
                                AST_FOR (tree, initExpr),
                                AST_FOR (tree, condExpr),
                                AST_FOR (tree, loopExpr),
                                tree->left,
                                tree->right);
            return decorateType (tree, RESULT_TYPE_NONE);
          }
        else
          {
            tree = createFor (AST_FOR (tree, trueLabel),
                              AST_FOR (tree, continueLabel),
                              AST_FOR (tree, falseLabel),
                              AST_FOR (tree, condLabel),
                              AST_FOR (tree, initExpr),
                              AST_FOR (tree, condExpr),
                              AST_FOR (tree, loopExpr),
                              tree->left,
                              tree->right);
            return decorateType (tree, RESULT_TYPE_NONE);
          }
      }
    case PARAM:
      werrorfl (tree->filename, tree->lineno, E_INTERNAL_ERROR, __FILE__, __LINE__, "node PARAM shouldn't be processed here");
      /* but in processParms() */
      return tree;
    case INLINEASM:
      formatInlineAsm (tree->values.inlineasm);
      TTYPE (tree) = TETYPE (tree) = NULL;
      return tree;
    default:
      TTYPE (tree) = TETYPE (tree) = NULL;
      return tree;
    }

  /* some error found this tree will be killed */
errorTreeReturn:
  TTYPE (tree) = TETYPE (tree) = newCharLink ();
  tree->opval.op = NULLOP;
  tree->isError = 1;

  return tree;
}

/*-----------------------------------------------------------------*/
/* sizeofOp - processes size of operation                          */
/*-----------------------------------------------------------------*/
value *
sizeofOp (sym_link *type)
{
  struct dbuf_s dbuf;
  value *val;
  int size;

  /* make sure the type is complete and sane */
  checkTypeSanity (type, "(sizeof)");

  /* get the size and convert it to character  */
  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "%d", size = getSize (type));
  if (!size && !IS_VOID (type))
    werror (E_SIZEOF_INCOMPLETE_TYPE);

  /* now convert into value  */
  val = constVal (dbuf_c_str (&dbuf));
  dbuf_destroy (&dbuf);
  return val;
}

/*-----------------------------------------------------------------*/
/* sizeofOp - processes alignment of operation                     */
/*-----------------------------------------------------------------*/
value *
alignofOp (sym_link *type)
{
  value *val;

  /* make sure the type is complete and sane */
  checkTypeSanity (type, "(_Alignof)");

  val = constVal ("1");

  return val;
}

/*-----------------------------------------------------------------*/
/* backPatchLabels - change and or not operators to flow control    */
/*-----------------------------------------------------------------*/
static ast *
backPatchLabels (ast * tree, symbol * trueLabel, symbol * falseLabel)
{
  if (!tree)
    return NULL;

  /* while-loops insert a label between the IFX and the condition,
     therefore look behind the label too */
  if (tree->opval.op == LABEL && tree->right && IS_ANDORNOT (tree->right))
    {
      tree->right = backPatchLabels (tree->right, trueLabel, falseLabel);
      return tree;
    }

  if (!(IS_ANDORNOT (tree)))
    return tree;

  /* if this an and */
  if (IS_AND (tree))
    {
      static int localLbl = 0;
      symbol *localLabel;
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      dbuf_printf (&dbuf, "_andif_%d", localLbl++);
      localLabel = newSymbol (dbuf_c_str (&dbuf), NestLevel);
      dbuf_destroy (&dbuf);

      tree->left = backPatchLabels (tree->left, localLabel, falseLabel);

      /* if left is already a IFX then just change the if true label in that */
      if (!IS_IFX (tree->left))
        tree->left = newIfxNode (tree->left, localLabel, falseLabel);

      tree->right = backPatchLabels (tree->right, trueLabel, falseLabel);
      /* right is a IFX then just join */
      if (IS_IFX (tree->right))
        return newNode (NULLOP, tree->left, createLabel (localLabel, tree->right));

      tree->right = newIfxNode (tree->right, trueLabel, falseLabel);
      tree->right = createLabel (localLabel, tree->right);

      return newNode (NULLOP, tree->left, tree->right);
    }

  /* if this is an or operation */
  if (IS_OR (tree))
    {
      static int localLbl = 0;
      symbol *localLabel;
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);
      dbuf_printf (&dbuf, "_orif_%d", localLbl++);
      localLabel = newSymbol (dbuf_c_str (&dbuf), NestLevel);
      dbuf_destroy (&dbuf);

      tree->left = backPatchLabels (tree->left, trueLabel, localLabel);

      /* if left is already a IFX then just change the if true label in that */
      if (!IS_IFX (tree->left))
        tree->left = newIfxNode (tree->left, trueLabel, localLabel);

      tree->right = backPatchLabels (tree->right, trueLabel, falseLabel);
      /* right is a IFX then just join */
      if (IS_IFX (tree->right))
        return newNode (NULLOP, tree->left, createLabel (localLabel, tree->right));

      tree->right = newIfxNode (tree->right, trueLabel, falseLabel);
      tree->right = createLabel (localLabel, tree->right);

      return newNode (NULLOP, tree->left, tree->right);
    }

  /* change not */
  if (IS_NOT (tree))
    {
      /* call with exchanged labels */
      tree->left = backPatchLabels (tree->left, falseLabel, trueLabel);

      /* if left isn't already a IFX */
      if (!IS_IFX (tree->left))
        {
          tree->left = newNode (IFX, tree->left, NULL);
          tree->left->trueLabel = falseLabel;
          tree->left->falseLabel = trueLabel;
        }
      return tree->left;
    }

  if (IS_IFX (tree))
    {
      tree->trueLabel = trueLabel;
      tree->falseLabel = falseLabel;
    }

  return tree;
}

/*-----------------------------------------------------------------*/
/* createBlock - create expression tree for block                  */
/*-----------------------------------------------------------------*/
ast *
createBlock (symbol * decl, ast * body)
{
  ast *ex;

  /* if the block has nothing */
  if (!body && !decl)
    return NULL;

  ex = newNode (BLOCK, NULL, body);
  ex->values.sym = decl;

  ex->level += LEVEL_UNIT;
  ex->filename = NULL;
  ex->lineno = 0;
  if (body)
    ex->block = body->block;
  return ex;
}

/*-----------------------------------------------------------------*/
/* createLabel - creates the expression tree for labels            */
/*-----------------------------------------------------------------*/
ast *
createLabel (symbol * label, ast * stmnt)
{
  symbol *csym;
  ast *rValue;

  /* must create fresh symbol if the symbol name  */
  /* exists in the symbol table, since there can  */
  /* be a variable with the same name as the labl */
  if ((csym = findSym (SymbolTab, NULL, label->name)) && (csym->level == label->level))
    label = newSymbol (label->name, label->level);

  /* put the label in the LabelSymbol table    */
  /* but first check if a label of the same    */
  /* name exists                               */
  if ((csym = findSym (LabelTab, NULL, label->name)))
    werror (E_DUPLICATE_LABEL, label->name);
  else
    addSym (LabelTab, label, label->name, label->level, 0, 0);

  label->isitmp = 1;
  label->islbl = 1;
  label->key = labelKey++;
  rValue = newNode (LABEL, newAst_VALUE (symbolVal (label)), stmnt);
  rValue->filename = NULL;
  rValue->lineno = 0;

  return rValue;
}

/*-----------------------------------------------------------------*/
/* createCase - generates the parsetree for a case statement       */
/*-----------------------------------------------------------------*/
ast *
createCase (ast * swStat, ast * caseVal, ast * stmnt)
{
  struct dbuf_s dbuf;
  ast *rexpr;
  value *val;

  /* if the switch statement does not exist */
  /* then case is out of context            */
  if (!swStat)
    {
      werrorfl (caseVal->filename, caseVal->lineno, E_CASE_CONTEXT);
      return NULL;
    }

  caseVal = decorateType (resolveSymbols (caseVal), RESULT_TYPE_NONE);
  /* if not a constant then error  */
  if (!IS_LITERAL (caseVal->ftype))
    {
      werrorfl (caseVal->filename, caseVal->lineno, E_CASE_CONSTANT);
      return NULL;
    }

  /* if not a integer than error */
  if (!IS_INTEGRAL (caseVal->ftype))
    {
      werrorfl (caseVal->filename, caseVal->lineno, E_CASE_NON_INTEGER);
      return NULL;
    }

  /* find the end of the switch values chain   */
  if (!(val = swStat->values.switchVals.swVals))
    swStat->values.switchVals.swVals = caseVal->opval.val;
  else
    {
      /* also order the cases according to value */
      value *pval = NULL;
      int cVal = (int) ulFromVal (caseVal->opval.val);
      while (val && (int) ulFromVal (val) < cVal)
        {
          pval = val;
          val = val->next;
        }

      /* if we reached the end then */
      if (!val)
        {
          pval->next = caseVal->opval.val;
        }
      else if ((int) ulFromVal (val) == cVal)
        {
          werrorfl (caseVal->filename, caseVal->lineno, E_DUPLICATE_LABEL, "case");
          return NULL;
        }
      else
        {
          /* we found a value greater than */
          /* the current value we must add this */
          /* before the value */
          caseVal->opval.val->next = val;

          /* if this was the first in chain */
          if (swStat->values.switchVals.swVals == val)
            swStat->values.switchVals.swVals = caseVal->opval.val;
          else
            pval->next = caseVal->opval.val;
        }

    }

  /* create the case label   */
  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "_case_%d_%d", swStat->values.switchVals.swNum, (int) ulFromVal (caseVal->opval.val));

  rexpr = createLabel (newSymbol (dbuf_c_str (&dbuf), 0), stmnt);
  dbuf_destroy (&dbuf);
  rexpr->filename = 0;
  rexpr->lineno = 0;
  return rexpr;
}

/*-----------------------------------------------------------------*/
/* createDefault - creates the parse tree for the default statement */
/*-----------------------------------------------------------------*/
ast *
createDefault (ast * swStat, ast * defaultVal, ast * stmnt)
{
  struct dbuf_s dbuf;
  ast *ret;

  /* if the switch statement does not exist */
  /* then case is out of context            */
  if (!swStat)
    {
      werrorfl (defaultVal->filename, defaultVal->lineno, E_CASE_CONTEXT);
      return NULL;
    }

  if (swStat->values.switchVals.swDefault)
    {
      werrorfl (defaultVal->filename, defaultVal->lineno, E_DUPLICATE_LABEL, "default");
      return NULL;
    }

  /* turn on the default flag   */
  swStat->values.switchVals.swDefault = 1;

  /* create the label  */
  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "_default_%d", swStat->values.switchVals.swNum);
  ret = createLabel (newSymbol (dbuf_c_str (&dbuf), 0), stmnt);
  dbuf_destroy (&dbuf);
  return ret;
}

/*-----------------------------------------------------------------*/
/* createIf - creates the parsetree for the if statement           */
/*-----------------------------------------------------------------*/
ast *
createIf (ast * condAst, ast * ifBody, ast * elseBody)
{
  static int Lblnum = 0;
  ast *ifTree;
  symbol *ifTrue, *ifFalse, *ifEnd;
  struct dbuf_s dbuf;

  /* if neither exists */
  if (!elseBody && !ifBody)
    {
      // if there are no side effects (i++, j() etc)
      if (!hasSEFcalls (condAst))
        {
          return condAst;
        }
    }

  /* create the labels */
  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "__iffalse_%d", Lblnum);
  ifFalse = newSymbol (dbuf_c_str (&dbuf), NestLevel);
  dbuf_destroy (&dbuf);
  /* if no else body then end == false */
  if (!elseBody)
    {
      ifEnd = ifFalse;
    }
  else
    {
      dbuf_init (&dbuf, 128);
      dbuf_printf (&dbuf, "__ifend_%d", Lblnum);
      ifEnd = newSymbol (dbuf_c_str (&dbuf), NestLevel);
      dbuf_destroy (&dbuf);
    }

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "__iftrue_%d", Lblnum);
  ifTrue = newSymbol (dbuf_c_str (&dbuf), NestLevel);
  dbuf_destroy (&dbuf);

  Lblnum++;

  /* attach the ifTrue label to the top of it body */
  ifBody = createLabel (ifTrue, ifBody);
  /* attach a goto end to the ifBody if else is present */
  if (elseBody)
    {
      ifBody = newNode (NULLOP, ifBody, newNode (GOTO, newAst_VALUE (symbolVal (ifEnd)), NULL));
      /* put the elseLabel on the else body */
      elseBody = createLabel (ifFalse, elseBody);
      /* out the end at the end of the body */
      elseBody = newNode (NULLOP, elseBody, createLabel (ifEnd, NULL));
    }
  else
    {
      ifBody = newNode (NULLOP, ifBody, createLabel (ifFalse, NULL));
    }
  condAst = backPatchLabels (condAst, ifTrue, ifFalse);
  if (IS_IFX (condAst))
    ifTree = condAst;
  else
    ifTree = newIfxNode (condAst, ifTrue, ifFalse);

  return newNode (NULLOP, ifTree, newNode (NULLOP, ifBody, elseBody));

}

/*-----------------------------------------------------------------*/
/* createDo - creates parse tree for do                            */
/*        _dobody_n:                                               */
/*            statements                                           */
/*        _docontinue_n:                                           */
/*            condition_expression +-> trueLabel -> _dobody_n      */
/*                                 |                               */
/*                                 +-> falseLabel-> _dobreak_n     */
/*        _dobreak_n:                                              */
/*-----------------------------------------------------------------*/
ast *
createDo (symbol * trueLabel, symbol * continueLabel, symbol * falseLabel, ast * condAst, ast * doBody)
{
  ast *doTree;

  /* if the body does not exist then it is simple */
  if (!doBody)
    {
      condAst = backPatchLabels (condAst, continueLabel, falseLabel);
      if (condAst && !IS_IFX (condAst))
        {
          condAst = newNode (IFX, condAst, NULL);
          condAst->trueLabel = continueLabel;
          condAst->falseLabel = NULL;
        }

      doTree = createLabel (continueLabel, condAst);
      doTree = newNode (NULLOP, doTree, createLabel (falseLabel, NULL));
      return doTree;
    }

  /* otherwise we have a body */
  condAst = backPatchLabels (condAst, trueLabel, falseLabel);

  /* attach the body label to the top */
  doBody = createLabel (trueLabel, doBody);
  /* attach the continue label to end of body */
  doBody = newNode (NULLOP, doBody, createLabel (continueLabel, NULL));

  /* now put the break label at the end */
  if (IS_IFX (condAst))
    doTree = condAst;
  else
    doTree = newIfxNode (condAst, trueLabel, falseLabel);

  doTree = newNode (NULLOP, doTree, createLabel (falseLabel, NULL));

  /* putting it together */
  return newNode (NULLOP, doBody, doTree);
}

/*-----------------------------------------------------------------*/
/* createFor - creates parse tree for 'for' statement              */
/*        initExpr                                                 */
/*   _forcond_n:                                                   */
/*        condExpr  +-> trueLabel -> _forbody_n                    */
/*                  |                                              */
/*                  +-> falseLabel-> _forbreak_n                   */
/*   _forbody_n:                                                   */
/*        statements                                               */
/*   _forcontinue_n:                                               */
/*        loopExpr                                                 */
/*        goto _forcond_n ;                                        */
/*   _forbreak_n:                                                  */
/*-----------------------------------------------------------------*/
ast *
createFor (symbol * trueLabel, symbol * continueLabel, symbol * falseLabel,
           symbol * condLabel, ast * initExpr, ast * condExpr, ast * loopExpr,
           ast * forBody, ast * continueLabelAst)
{
  ast *forTree;

  /* vanilla for statement */
  condExpr = backPatchLabels (condExpr, trueLabel, falseLabel);

  if (condExpr && !IS_IFX (condExpr))
    condExpr = newIfxNode (condExpr, trueLabel, falseLabel);

  /* attach body label to body */
  forBody = createLabel (trueLabel, forBody);

  /* attach condition label to condition */
  condExpr = createLabel (condLabel, condExpr);
  
  /* attach continue to forLoop expression & attach */
  /* goto the forcond @ and of loopExpression       */
  loopExpr = newNode (NULLOP, loopExpr, newNode (GOTO, newAst_VALUE (symbolVal (condLabel)), NULL));
  if (continueLabelAst)
    {
      continueLabelAst->right = loopExpr;
      loopExpr = continueLabelAst;
    }
  else
    loopExpr = createLabel (continueLabel, loopExpr);

  /* now start putting them together */
  forTree = newNode (NULLOP, initExpr, condExpr);
  forTree = newNode (NULLOP, forTree, forBody);
  forTree = newNode (NULLOP, forTree, loopExpr);
  
  /* the break label is already in the tree as a sibling */
  /* to the original FOR node this tree is replacing */
  return forTree;
}

/*-----------------------------------------------------------------*/
/* createWhile - creates parse tree for while statement            */
/*               the while statement will be created as follows    */
/*                                                                 */
/*      _while_continue_n:                                         */
/*            condition_expression +-> trueLabel -> _while_boby_n  */
/*                                 |                               */
/*                                 +-> falseLabel -> _while_break_n */
/*      _while_body_n:                                             */
/*            statements                                           */
/*            goto _while_continue_n                               */
/*      _while_break_n:                                            */
/*-----------------------------------------------------------------*/
ast *
createWhile (symbol * trueLabel, symbol * continueLabel, symbol * falseLabel, ast * condExpr, ast * whileBody)
{
  ast *whileTree;

  /* put the continue label */
  condExpr = backPatchLabels (condExpr, trueLabel, falseLabel);
  if (condExpr && !IS_IFX (condExpr))
    {
      condExpr = newNode (IFX, condExpr, NULL);
      /* put the true & false labels in place */
      condExpr->trueLabel = trueLabel;
      condExpr->falseLabel = falseLabel;
    }
  whileTree = createLabel (continueLabel, condExpr);
  whileTree->filename = NULL;
  whileTree->lineno = 0;

  /* put the body label in front of the body */
  whileBody = createLabel (trueLabel, whileBody);
  whileBody->filename = NULL;
  whileBody->lineno = 0;
  /* put a jump to continue at the end of the body */
  /* and put break label at the end of the body */
  whileBody = newNode (NULLOP,
                       whileBody, newNode (GOTO, newAst_VALUE (symbolVal (continueLabel)), createLabel (falseLabel, NULL)));

  /* put it all together */
  return newNode (NULLOP, whileTree, whileBody);
}

/*-----------------------------------------------------------------*/
/* isShiftRightLitVal _BitAndLitVal - helper function              */
/*-----------------------------------------------------------------*/
static ast *
isShiftRightLitVal_BitAndLitVal (ast * tree)
{
  /* if this is not a bit and */
  if (!IS_BITAND (tree))
    return NULL;

  /* will look for tree of the form
     ( expr >> litval2) & litval1 */
  if (!IS_AST_LIT_VALUE (tree->right))
    return NULL;

  if (!IS_RIGHT_OP (tree->left))
    return NULL;

  if (!IS_AST_LIT_VALUE (tree->left->right))
    return NULL;

  return tree->left->left;
}

/*-----------------------------------------------------------------*/
/* isBitAndPowOf2 - helper function                                */
/*-----------------------------------------------------------------*/
static int
isBitAndPow2 (ast * tree)
{
  /* if this is not a bit and */
  if (!IS_BITAND (tree))
    return -1;

  /* will look for tree of the form
     ( expr & (1 << litval) */
  if (!IS_AST_LIT_VALUE (tree->right))
    return -1;

  return powof2 (AST_ULONG_VALUE (tree->right));
}

/*-----------------------------------------------------------------*/
/* optimizeGetHbit - get highest order bit of the expression       */
/*-----------------------------------------------------------------*/
ast *
optimizeGetHbit (ast * tree, RESULT_TYPE resultType)
{
  unsigned int bit, msb;
  ast *expr;

  expr = isShiftRightLitVal_BitAndLitVal (tree);
  if (expr)
    {
      if ((AST_ULONG_VALUE (tree->right) != 1) ||
          ((bit = AST_ULONG_VALUE (tree->left->right)) != (msb = (bitsForType (TTYPE (expr)) - 1))))
        expr = NULL;
    }
  if (!expr && (resultType == RESULT_TYPE_BOOL))
    {
      int bit = isBitAndPow2 (tree);
      expr = tree->left;
      msb = bitsForType (TTYPE (expr)) - 1;
      if ((bit < 0) || (bit != (int) msb))
        expr = NULL;
    }
  if (!expr || IS_BOOLEAN (TTYPE (expr)))
    return tree;

  /* make sure the port supports GETHBIT */
  if (port->hasExtBitOp && !port->hasExtBitOp (GETHBIT, getSize (TTYPE (expr))))
    return tree;

  return decorateType (newNode (GETHBIT, expr, NULL), resultType);
}

/*-----------------------------------------------------------------*/
/* optimizeGetAbit - get a single bit of the expression            */
/*-----------------------------------------------------------------*/
ast *
optimizeGetAbit (ast * tree, RESULT_TYPE resultType)
{
  ast *expr;
  ast *count = NULL;

  expr = isShiftRightLitVal_BitAndLitVal (tree);
  if (expr)
    {
      if (AST_ULONG_VALUE (tree->right) != 1)
        expr = NULL;
      count = tree->left->right;
    }
  if (!expr && (resultType == RESULT_TYPE_BOOL))
    {
      int p2 = isBitAndPow2 (tree);
      if (p2 >= 0 && !IS_BOOLEAN (TTYPE (tree->left)))
        {
          expr = tree->left;
          count = newAst_VALUE (valueFromLit (p2));
        }
    }
  if (!expr)
    return tree;

  /* make sure the port supports GETABIT */
  if (port->hasExtBitOp && !port->hasExtBitOp (GETABIT, getSize (TTYPE (expr))))
    return tree;

  return decorateType (newNode (GETABIT, expr, count), resultType);
}

/*-----------------------------------------------------------------*/
/* optimizeGetByte - get a byte of the expression                  */
/*-----------------------------------------------------------------*/
ast *
optimizeGetByte (ast * tree, RESULT_TYPE resultType)
{
  unsigned int i = 1;
  unsigned int size;
  ast *expr;
  ast *count = NULL;

  expr = isShiftRightLitVal_BitAndLitVal (tree);
  if (expr)
    {
      i = AST_ULONG_VALUE (tree->left->right);
      count = tree->left->right;
      if (AST_ULONG_VALUE (tree->right) != 0xFF)
        expr = NULL;
    }
  if (!expr && resultType == RESULT_TYPE_CHAR)
    {
      /* if this is a right shift over a multiple of 8 */
      if (IS_RIGHT_OP (tree) && IS_AST_LIT_VALUE (tree->right))
        {
          i = AST_ULONG_VALUE (tree->right);
          count = tree->right;
          expr = tree->left;
        }
    }
  if (!expr || (i % 8))
    return tree;
  size = getSize (TTYPE (expr));
  if ((i >= size * 8) || (size <= 1))
    return tree;

  /* make sure the port supports GETBYTE */
  if (port->hasExtBitOp && !port->hasExtBitOp (GETBYTE, size))
    return tree;

  return decorateType (newNode (GETBYTE, expr, count), RESULT_TYPE_NONE);
}

/*-----------------------------------------------------------------*/
/* optimizeGetWord - get two bytes of the expression               */
/*-----------------------------------------------------------------*/
ast *
optimizeGetWord (ast *tree, RESULT_TYPE resultType)
{
  unsigned int i = 1;
  unsigned int size;
  ast *expr;
  ast *count = NULL;

  expr = isShiftRightLitVal_BitAndLitVal (tree);

  if (expr)
    {
      i = AST_ULONG_VALUE (tree->left->right);
      count = tree->left->right;
      if (AST_ULONG_VALUE (tree->right) != 0xFFFF)
        expr = NULL;
    }
  if (!expr && resultType == RESULT_TYPE_INT)
    {
      /* if this is a right shift over a multiple of 8 */
      if (IS_RIGHT_OP (tree) && IS_AST_LIT_VALUE (tree->right))
        {
          i = AST_ULONG_VALUE (tree->right);
          count = tree->right;
          expr = tree->left;
        }
    }
  if (!expr || (i % 8))
    return tree;
  size = getSize (TTYPE (expr));
  if ((i >= (size - 1) * 8) || (size <= 2))
    return tree;

  /* make sure the port supports GETWORD */
  if (port->hasExtBitOp && !port->hasExtBitOp (GETWORD, size))
    return tree;

  return decorateType (newNode (GETWORD, expr, count), RESULT_TYPE_NONE);
}

/*-----------------------------------------------------------------*/
/* optimizeRRCRLC :- optimize for Rotate Left/Right with carry     */
/*-----------------------------------------------------------------*/
ast *
optimizeRRCRLC (ast * root)
{
  /* will look for trees of the form
     (?expr << 1) | (?expr >> 7) or
     (?expr >> 7) | (?expr << 1) will make that
     into a RLC : operation ..
     Will also look for
     (?expr >> 1) | (?expr << 7) or
     (?expr << 7) | (?expr >> 1) will make that
     into a RRC operation
     note : by 7 I mean (number of bits required to hold the
     variable -1 ) */
  /* if the root operation is not a | operation then not */
  if (!IS_BITOR (root))
    return root;

  /* I have to think of a better way to match patterns this sucks */
  /* that aside let's start looking for the first case : I use a
     negative check a lot to improve the efficiency */
  /* (?expr << 1) | (?expr >> 7) */
  if (IS_LEFT_OP (root->left) && IS_RIGHT_OP (root->right))
    {

      if (!SPEC_USIGN (TETYPE (root->left->left)))
        return root;

      if (!IS_AST_LIT_VALUE (root->left->right) || !IS_AST_LIT_VALUE (root->right->right))
        goto tryNext0;

      /* make sure it is the same expression */
      if (!isAstEqual (root->left->left, root->right->left))
        goto tryNext0;

      if (AST_ULONG_VALUE (root->left->right) != 1)
        goto tryNext0;

      if (AST_ULONG_VALUE (root->right->right) != (getSize (TTYPE (root->left->left)) * 8 - 1))
        goto tryNext0;

      /* make sure the port supports RLC */
      if (port->hasExtBitOp && !port->hasExtBitOp (RLC, getSize (TTYPE (root->left->left))))
        return root;

      /* whew got the first case : create the AST */
      return newNode (RLC, root->left->left, NULL);
    }

tryNext0:
  /* check for second case */
  /* (?expr >> 7) | (?expr << 1) */
  if (IS_LEFT_OP (root->right) && IS_RIGHT_OP (root->left))
    {

      if (!SPEC_USIGN (TETYPE (root->left->left)))
        return root;

      if (!IS_AST_LIT_VALUE (root->left->right) || !IS_AST_LIT_VALUE (root->right->right))
        goto tryNext1;

      /* make sure it is the same symbol */
      if (!isAstEqual (root->left->left, root->right->left))
        goto tryNext1;

      if (AST_ULONG_VALUE (root->right->right) != 1)
        goto tryNext1;

      if (AST_ULONG_VALUE (root->left->right) != (getSize (TTYPE (root->left->left)) * 8 - 1))
        goto tryNext1;

      /* make sure the port supports RLC */
      if (port->hasExtBitOp && !port->hasExtBitOp (RLC, getSize (TTYPE (root->left->left))))
        return root;

      /* whew got the first case : create the AST */
      return newNode (RLC, root->left->left, NULL);

    }

tryNext1:
  /* third case for RRC */
  /*  (?symbol >> 1) | (?symbol << 7) */
  if (IS_LEFT_OP (root->right) && IS_RIGHT_OP (root->left))
    {

      if (!SPEC_USIGN (TETYPE (root->left->left)))
        return root;

      if (!IS_AST_LIT_VALUE (root->left->right) || !IS_AST_LIT_VALUE (root->right->right))
        goto tryNext2;

      /* make sure it is the same symbol */
      if (!isAstEqual (root->left->left, root->right->left))
        goto tryNext2;

      if (AST_ULONG_VALUE (root->left->right) != 1)
        goto tryNext2;

      if (AST_ULONG_VALUE (root->right->right) != (getSize (TTYPE (root->left->left)) * 8 - 1))
        goto tryNext2;

      /* make sure the port supports RRC */
      if (port->hasExtBitOp && !port->hasExtBitOp (RRC, getSize (TTYPE (root->left->left))))
        return root;

      /* whew got the first case : create the AST */
      return newNode (RRC, root->left->left, NULL);

    }
tryNext2:
  /* fourth and last case for now */
  /* (?symbol << 7) | (?symbol >> 1) */
  if (IS_RIGHT_OP (root->right) && IS_LEFT_OP (root->left))
    {
      if (!SPEC_USIGN (TETYPE (root->left->left)))
        return root;

      if (!IS_AST_LIT_VALUE (root->left->right) || !IS_AST_LIT_VALUE (root->right->right))
        return root;

      /* make sure it is the same symbol */
      if (!isAstEqual (root->left->left, root->right->left))
        return root;

      if (AST_ULONG_VALUE (root->right->right) != 1)
        return root;

      if (AST_ULONG_VALUE (root->left->right) != (getSize (TTYPE (root->left->left)) * 8 - 1))
        return root;

      /* make sure the port supports RRC */
      if (port->hasExtBitOp && !port->hasExtBitOp (RRC, getSize (TTYPE (root->left->left))))
        return root;

      /* whew got the first case : create the AST */
      return newNode (RRC, root->left->left, NULL);
    }

  /* not found return root */
  return root;
}

/*-----------------------------------------------------------------*/
/* optimizeSWAP :- optimize for nibble/byte/word swaps             */
/*-----------------------------------------------------------------*/
ast *
optimizeSWAP (ast * root)
{
  /* will look for trees of the form
     (?expr << 4) | (?expr >> 4) or
     (?expr >> 4) | (?expr << 4) will make that
     into a SWAP : operation ..
     note : by 4 I mean (number of bits required to hold the
     variable /2 ) */
  /* if the root operation is not a | operation then not */
  if (!IS_BITOR (root))
    return root;

  /* (?expr << 4) | (?expr >> 4) */
  if ((IS_LEFT_OP (root->left) && IS_RIGHT_OP (root->right)) || (IS_RIGHT_OP (root->left) && IS_LEFT_OP (root->right)))
    {

      if (!SPEC_USIGN (TETYPE (root->left->left)))
        return root;

      if (!IS_AST_LIT_VALUE (root->left->right) || !IS_AST_LIT_VALUE (root->right->right))
        return root;

      /* make sure it is the same expression */
      if (!isAstEqual (root->left->left, root->right->left))
        return root;

      if (AST_ULONG_VALUE (root->left->right) != (getSize (TTYPE (root->left->left)) * 4))
        return root;

      if (AST_ULONG_VALUE (root->right->right) != (getSize (TTYPE (root->left->left)) * 4))
        return root;

      /* make sure the port supports SWAP */
      if (port->hasExtBitOp && !port->hasExtBitOp (SWAP, getSize (TTYPE (root->left->left))))
        return root;

      /* found it : create the AST */
      return newNode (SWAP, root->left->left, NULL);
    }

  /* not found return root */
  return root;
}

/*-----------------------------------------------------------------*/
/* optimizeCompare - optimizes compares for bit variables          */
/*-----------------------------------------------------------------*/
static ast *
optimizeCompare (ast * root)
{
  ast *optExpr = NULL;
  value *vleft;
  value *vright;
  unsigned int litValue;

  /* if nothing then return nothing */
  if (!root)
    return NULL;

  /* if not a compare op then do leaves */
  if (!IS_COMPARE_OP (root))
    {
      root->left = optimizeCompare (root->left);
      root->right = optimizeCompare (root->right);
      return root;
    }

  /* if left & right are the same then depending
     of the operation do */
  if (isAstEqual (root->left, root->right) && !hasSEFcalls (root))
    {
      switch (root->opval.op)
        {
        case '>':
        case '<':
        case NE_OP:
          optExpr = newAst_VALUE (constBoolVal (0));
          break;
        case GE_OP:
        case LE_OP:
        case EQ_OP:
          optExpr = newAst_VALUE (constBoolVal (1));
          break;
        }

      return decorateType (optExpr, RESULT_TYPE_NONE);
    }

  vleft = (root->left->type == EX_VALUE ? root->left->opval.val : NULL);

  vright = (root->right->type == EX_VALUE ? root->right->opval.val : NULL);

  /* if left is a BITVAR in BITSPACE */
  /* and right is a LITERAL then     */
  /* optimize else do nothing        */
  if (vleft && vright && IS_BITVAR (vleft->etype) && IN_BITSPACE (SPEC_OCLS (vleft->etype)) && IS_LITERAL (vright->etype))
    {
      /* if right side > 1 then comparison may never succeed */
      if ((litValue = (int) ulFromVal (vright)) > 1)
        {
          werror (W_BAD_COMPARE);
          goto noOptimize;
        }

      if (litValue)
        {
          switch (root->opval.op)
            {
            case '>':          /* bit value greater than 1 cannot be */
              werror (W_BAD_COMPARE);
              goto noOptimize;
              break;

            case '<':          /* bit value < 1 means 0 */
            case NE_OP:
              optExpr = newNode ('!', newAst_VALUE (vleft), NULL);
              break;

            case LE_OP:        /* bit value <= 1 means no check */
              optExpr = newAst_VALUE (vright);
              break;

            case GE_OP:        /* bit value >= 1 means only check for = */
            case EQ_OP:
              optExpr = newAst_VALUE (vleft);
              break;
            }
        }
      else
        {                       /* literal is zero */
          switch (root->opval.op)
            {
            case '<':          /* bit value < 0 cannot be */
              werror (W_BAD_COMPARE);
              goto noOptimize;
              break;

            case '>':          /* bit value > 0 means 1 */
            case NE_OP:
              optExpr = newAst_VALUE (vleft);
              break;

            case LE_OP:        /* bit value <= 0 means no check */
            case GE_OP:        /* bit value >= 0 means no check */
              werror (W_BAD_COMPARE);
              goto noOptimize;
              break;

            case EQ_OP:        /* bit == 0 means ! of bit */
              optExpr = newNode ('!', newAst_VALUE (vleft), NULL);
              break;
            }
        }
      return decorateType (resolveSymbols (optExpr), RESULT_TYPE_NONE);
    }                           /* end-of-if of BITVAR */

noOptimize:
  return root;
}

/*-----------------------------------------------------------------*/
/* addSymToBlock : adds the symbol to the first block we find      */
/*-----------------------------------------------------------------*/
void
addSymToBlock (symbol * sym, ast * tree)
{
  /* reached end of tree or a leaf */
  if (!tree || IS_AST_LINK (tree) || IS_AST_VALUE (tree))
    return;

  /* found a block */
  if (IS_AST_OP (tree) && tree->opval.op == BLOCK)
    {
      symbol *lsym = copySymbol (sym);

      lsym->next = AST_VALUES (tree, sym);
      AST_VALUES (tree, sym) = lsym;
      return;
    }

  addSymToBlock (sym, tree->left);
  addSymToBlock (sym, tree->right);
}

/*-----------------------------------------------------------------*/
/* processRegParms - do processing for register parameters         */
/*-----------------------------------------------------------------*/
static void
processRegParms (value * args, ast * body)
{
  while (args)
    {
      if (IS_REGPARM (args->etype))
        addSymToBlock (args->sym, body);
      args = args->next;
    }
}

/*-----------------------------------------------------------------*/
/* resetParmKey - resets the operandkeys for the symbols           */
/*-----------------------------------------------------------------*/
DEFSETFUNC (resetParmKey)
{
  symbol *sym = item;

  sym->key = 0;
  sym->defs = NULL;
  sym->uses = NULL;
  sym->remat = 0;
  return 1;
}

/*------------------------------------------------------------------*/
/* fixupInlineLabel - change a label in an inlined function so that */
/*                    it is always unique no matter how many times  */
/*                    the function is inlined.                      */
/*------------------------------------------------------------------*/
static void
fixupInlineLabel (symbol * sym)
{
  struct dbuf_s dbuf;

  dbuf_init (&dbuf, 128);
  dbuf_printf (&dbuf, "%s_%d", sym->name, inlineState.count);
  strncpyz (sym->name, dbuf_c_str (&dbuf), SDCC_NAME_MAX);
  dbuf_destroy (&dbuf);
}

/*------------------------------------------------------------------*/
/* copyAstLoc - copy location information (file, line, block, etc.) */
/*              from one ast node to another                        */
/*------------------------------------------------------------------*/
static void
copyAstLoc (ast * dest, ast * src)
{
  dest->filename = src->filename;
  dest->lineno = src->lineno;
  dest->level = src->level;
  dest->block = src->block;
  dest->seqPoint = src->seqPoint;
}

static void fixupInline (ast * tree, long level);

/*-----------------------------------------------------------------*/
/* fixupInlineInDeclarators - recursively perform various fixups   */
/*                            on an inline function tree           */
/*-----------------------------------------------------------------*/
static void
fixupInlineInDeclarators (struct initList *ival, long level)
{
  for (; ival; ival = ival->next)
    {
      switch (ival->type)
        {
        case INIT_NODE:
          fixupInline (ival->init.node, level);
          break;
        case INIT_DEEP:
          fixupInlineInDeclarators (ival->init.deep, level);
          break;
        }
    }
}

/*-----------------------------------------------------------------*/
/* fixupInline - perform various fixups on an inline function tree */
/*               to take into account that it is no longer a       */
/*               stand-alone function.                             */
/*-----------------------------------------------------------------*/
static void
fixupInline (ast * tree, long level)
{
  int savedBlockno = currBlockno;

  if (IS_AST_OP (tree) && (tree->opval.op == BLOCK))
    {
      symbol *decls;

      int thisBlockBlockno;

      btree_add_child(currBlockno, ++blockNo);
      thisBlockBlockno = blockNo;

      level += LEVEL_UNIT;

      /* Add any declared variables back into the symbol table */
      for (decls = tree->values.sym; decls; decls = decls->next)
        {
          decls->level = level;
          decls->block = currBlockno = thisBlockBlockno;
          addSym (SymbolTab, decls, decls->name, decls->level, decls->block, 0);

          if (decls->ival)
            fixupInlineInDeclarators (decls->ival, level);
        }

      currBlockno = thisBlockBlockno;
    }

  tree->level = level;
  tree->block = currBlockno;

  /* Update symbols */
  if (IS_AST_VALUE (tree) && tree->opval.val->sym)
    {
      symbol *sym = tree->opval.val->sym;

      sym->level = level;
      sym->block = currBlockno;

      sym->reqv = NULL;
      SYM_SPIL_LOC (sym) = NULL;
      sym->key = 0;

      /* If the symbol is a label, we need to renumber it */
      if (sym->islbl)
        fixupInlineLabel (sym);
    }

  /* Update IFX target labels */
  if (tree->type == EX_OP && tree->opval.op == IFX)
    {
      if (tree->trueLabel)
        fixupInlineLabel (tree->trueLabel);
      if (tree->falseLabel)
        fixupInlineLabel (tree->falseLabel);
    }

  /* Replace RETURN with optional assignment and a GOTO to the end */
  /* of the inlined function */
  if (tree->type == EX_OP && tree->opval.op == RETURN)
    {
      ast *assignTree = NULL;
      ast *gotoTree;

      if (inlineState.retsym && tree->right)
        {
          assignTree = newNode ('=', newAst_VALUE (symbolVal (copySymbol (inlineState.retsym))), tree->right);
          copyAstLoc (assignTree, tree);
        }

      gotoTree = newNode (GOTO, newAst_VALUE (symbolVal (inlineState.retlab)), NULL);
      copyAstLoc (gotoTree, tree);

      tree->opval.op = NULLOP;
      tree->left = assignTree;
      tree->right = gotoTree;
    }

  /* Update any children */
  if (tree->left)
    fixupInline (tree->left, level);
  if (tree->right)
    fixupInline (tree->right, level);

  /* Update SWITCH branches */
  if (tree->type == EX_OP && tree->opval.op == SWITCH)
    {
      struct dbuf_s dbuf;
      const char *oldsuff = tree->values.switchVals.swSuffix;

      dbuf_init (&dbuf, 128);
      dbuf_printf (&dbuf, "%s_%d", oldsuff ? oldsuff : "", inlineState.count);
      tree->values.switchVals.swSuffix = dbuf_detach (&dbuf);
    }

  /* Update FOR expression */
  if (tree->type == EX_OP && tree->opval.op == FOR)
    {
      if (AST_FOR (tree, initExpr))
        fixupInline (AST_FOR (tree, initExpr), level);
      if (AST_FOR (tree, condExpr))
        fixupInline (AST_FOR (tree, condExpr), level);
      if (AST_FOR (tree, loopExpr))
        fixupInline (AST_FOR (tree, loopExpr), level);

      if (AST_FOR (tree, trueLabel))
        fixupInlineLabel (AST_FOR (tree, trueLabel));
      if (AST_FOR (tree, continueLabel))
        fixupInlineLabel (AST_FOR (tree, continueLabel));
      if (AST_FOR (tree, falseLabel))
        fixupInlineLabel (AST_FOR (tree, falseLabel));
      if (AST_FOR (tree, condLabel))
        fixupInlineLabel (AST_FOR (tree, condLabel));
    }

  if (IS_AST_OP (tree) && (tree->opval.op == LABEL))
    {
      symbol *label = tree->left->opval.val->sym;

      label->key = labelKey++;
      /* Add this label back into the symbol table */
      addSym (LabelTab, label, label->name, label->level, 0, 0);
    }

  if (IS_AST_OP (tree) && (tree->opval.op == BLOCK))
    {
      level -= LEVEL_UNIT;
      currBlockno = savedBlockno;
    }
}

/*-----------------------------------------------------------------*/
/* inlineAddDecl - add a variable declaration to an ast block. It  */
/*                 is also added to the symbol table if addSymTab  */
/*                 is nonzero.                                     */
/*-----------------------------------------------------------------*/
static void
inlineAddDecl (symbol * sym, ast * block, int addSymTab, int toFront)
{
  sym->reqv = NULL;
  SYM_SPIL_LOC (sym) = NULL;
  sym->key = 0;
  if (block != NULL)
    {
      symbol **decl = &(block->values.sym);

      sym->level = block->level;
      sym->block = block->block;

      while (*decl)
        {
          if (strcmp ((*decl)->name, sym->name) == 0)
            return;
          decl = &((*decl)->next);
        }

      if (toFront)
        {
          sym->next = block->values.sym;
          block->values.sym = sym;
        }
      else
        *decl = sym;

      if (addSymTab)
        addSym (SymbolTab, sym, sym->name, sym->level, sym->block, 0);
    }
}

/*-----------------------------------------------------------------*/
/* inlineTempVar - create a temporary variable for inlining        */
/*-----------------------------------------------------------------*/
static symbol *
inlineTempVar (sym_link * type, long level)
{
  symbol *sym;

  sym = newSymbol (genSymName (level), level);
  sym->type = copyLinkChain (type);
  sym->etype = getSpec (sym->type);
  SPEC_SCLS (sym->etype) = S_AUTO;
  SPEC_OCLS (sym->etype) = NULL;
  SPEC_EXTR (sym->etype) = 0;
  SPEC_STAT (sym->etype) = 0;
  if (IS_SPEC (sym->type))
    {
      SPEC_VOLATILE (sym->type) = 0;
      SPEC_ADDRSPACE (sym->type) = 0;
    }
  else
    {
      DCL_PTR_VOLATILE (sym->type) = 0;
      DCL_PTR_ADDRSPACE (sym->type) = 0;
    }
  SPEC_ABSA (sym->etype) = 0;

  return sym;
}

/*-----------------------------------------------------------------*/
/* inlineFindParm - search an ast tree of parameters to find one   */
/*                  at a particular index (0=first parameter).     */
/*                  Returns 0 if not found.                        */
/*-----------------------------------------------------------------*/
static ast *
inlineFindParm (ast *parms, int index)
{
  if (!parms)
    return 0;

  if (parms->type == EX_OP && parms->opval.op == PARAM)
  {
    ast *p;

    if (p = inlineFindParm (parms->left, index))
      return p;

    if (p = inlineFindParm (parms->right, index - 1))
      return p;
  }

  return (index ? 0 : parms);
}

static void expandInlineFuncs (ast * tree, ast * block);

/*-----------------------------------------------------------------*/
/* expandInlineFuncsInDeclarators - recursively replace calls to   */
/*                                  inline functions               */
/*-----------------------------------------------------------------*/
static void
expandInlineFuncsInDeclarators (struct initList *ival, ast * block)
{
  for (; ival; ival = ival->next)
    {
      switch (ival->type)
        {
        case INIT_NODE:
          expandInlineFuncs (ival->init.node, block);
          break;
        case INIT_DEEP:
          expandInlineFuncsInDeclarators (ival->init.deep, block);
          break;
        }
    }
}

/*-----------------------------------------------------------------*/
/* expandInlineFuncs - replace calls to inline functions with the  */
/*                     function itself                             */
/*-----------------------------------------------------------------*/
static void
expandInlineFuncs (ast * tree, ast * block)
{
  if (IS_AST_OP (tree) && (tree->opval.op == CALL) && tree->left && IS_AST_VALUE (tree->left) && tree->left->opval.val->sym)
    {
      int savedBlockno = currBlockno;
      symbol *func = tree->left->opval.val->sym;
      symbol *csym;

      currBlockno = tree->block;

      /* The symbol is probably not bound yet, so find the real one */
      csym = findSymWithLevel (SymbolTab, func);
      if (csym)
        func = csym;

      if ((inCriticalFunction || inCriticalBlock) && IFFUNC_ISCRITICAL (func->type))
        werrorfl (tree->left->filename, tree->left->lineno, E_INVALID_CRITICAL);

      /* Is this an inline function that we can inline? */
      if (IFFUNC_ISINLINE (func->type) && !IFFUNC_HASVARARGS(func->type) && func->funcTree)
        {
          symbol *retsym = NULL;
          symbol *retlab;
          ast *inlinetree;
          ast *inlinetree2;
          ast *temptree;
          value *args;
          int argIndex;

          /* Generate a label for the inlined function to branch to */
          /* in case it contains a return statement */
          retlab = newSymbol (genSymName (tree->level + LEVEL_UNIT), tree->level + LEVEL_UNIT);
          retlab->isitmp = 1;
          retlab->islbl = 1;
          inlineState.retlab = retlab;

          /* Build the subtree for the inlined function in the form: */
          /* { //inlinetree block                                    */
          /*   { //inlinetree2 block                                 */
          /*     inline_function_code;                               */
          /*     retlab:                                             */
          /*   }                                                     */
          /* }                                                       */
          temptree = newNode (LABEL, newAst_VALUE (symbolVal (retlab)), NULL);
          copyAstLoc (temptree, tree);
          temptree = newNode (NULLOP, copyAst (func->funcTree), temptree);
          copyAstLoc (temptree, tree);
          temptree = newNode (BLOCK, NULL, temptree);
          copyAstLoc (temptree, tree);
          inlinetree2 = temptree;
          inlinetree2->level += 2 * LEVEL_UNIT;
          inlinetree2->block = blockNo+2;

          /* Handle the return type */
          if (!IS_VOID (func->type->next))
            {
              /* Create a temporary symbol to hold the return value and   */
              /* join it with the inlined function using the comma        */
              /* operator. The fixupInline function will take care of     */
              /* changing return statements into assignments to retsym.   */
              /* (parameter passing and return label omitted for clarity) */
              /* rettype retsym;                                          */
              /* ...                                                      */
              /* {{inline_function_code}}, retsym                         */

              retsym = inlineTempVar (func->type->next, block->level);
              SPEC_SCLS (retsym->etype) = S_FIXED;
              inlineAddDecl (retsym, block, TRUE, TRUE);
            }

          inlinetree = newNode (BLOCK, NULL, inlinetree2);
          copyAstLoc (inlinetree, tree);
          inlinetree2->level += LEVEL_UNIT;
          inlinetree2->block = blockNo+1;

          /* To pass parameters to the inlined function, we need some  */
          /* intermediate variables. This avoids scoping problems      */
          /* when the parameter declaration names are used differently */
          /* during the function call. For example, a function         */
          /* declared as func(int x, int y) but called as func(y,x).   */
          /* { //inlinetree block                                      */
          /*   type1 temparg1 = argument1;                             */
          /*   ...                                                     */
          /*   typen tempargn = argumentn;                             */
          /*   { //inlinetree2 block                                   */
          /*     type1 param1 = temparg1;                              */
          /*     ...                                                   */
          /*     typen paramn = tempargn;                              */
          /*     inline_function_code;                                 */
          /*     retlab:                                               */
          /*   }                                                       */
          /* }                                                         */
          args = FUNC_ARGS (func->type);
          argIndex = 0;

          while (args)
            {
              symbol *temparg;
              ast *assigntree;
              symbol *parm;
              ast *passedarg = inlineFindParm (tree->right, argIndex);

              if (!passedarg)
                {
                  werror (E_TOO_FEW_PARMS);
                  break;
                }

              temparg = inlineTempVar (args->sym->type, tree->level + LEVEL_UNIT);
              inlineAddDecl (copySymbol (temparg), inlinetree, FALSE, FALSE);

              assigntree = newNode ('=', newAst_VALUE (symbolVal (temparg)), passedarg);
              assigntree->initMode = 1; // tell that assignment is initializer
              inlinetree->right = newNode (NULLOP, assigntree, inlinetree->right);

              parm = copySymbol (args->sym);
              inlineAddDecl (parm, inlinetree2, FALSE, FALSE);
              parm->_isparm = 0;

              assigntree = newNode ('=', newAst_VALUE (symbolVal (parm)), newAst_VALUE (symbolVal (temparg)));
              assigntree->initMode = 1; // tell that assignment is initializer
              inlinetree2->right = newNode (NULLOP, assigntree, inlinetree2->right);
              parm->onStack = 0; // stack usage will be recomputed later

              args = args->next;
              argIndex++;
            }

          if (inlineFindParm (tree->right, argIndex) && !IFFUNC_HASVARARGS (func->type))
            werror (E_TOO_MANY_PARMS);

          /* Handle the return type */
          if (!IS_VOID (func->type->next))
            {
              /* Generate return symbol coma statement;                   */
              /* ...                                                      */
              /* {{inline_function_code}}, retsym                         */

              tree->opval.op = ',';
              if (IFFUNC_ISCRITICAL (func->type))
                inlinetree = newNode (CRITICAL, inlinetree, NULL);
              tree->left = inlinetree;
              tree->right = newAst_VALUE (symbolVal (retsym));
            }
          else
            {
              tree->opval.op = NULLOP;
              if (IFFUNC_ISCRITICAL (func->type))
                inlinetree = newNode (CRITICAL, inlinetree, NULL);
              tree->left = NULL;
              tree->right = inlinetree;
            }
          inlineState.retsym = retsym;

          /* Renumber the various internal counters on the inlined   */
          /* function's tree nodes and symbols. Add the inlined      */
          /* function's local variables to the appropriate scope(s). */
          /* Convert inlined return statements to an assignment to   */
          /* retsym (if needed) and a goto retlab.                   */
          fixupInline (inlinetree, inlinetree->level);
          inlineState.count++;
        }
      currBlockno = savedBlockno;
    }

  /* Recursively continue to search for functions to inline. */
  if (IS_AST_OP (tree))
    {
      if (tree->opval.op == BLOCK)
        {
          struct symbol *decls;

          block = tree;

          for (decls = block->values.sym; decls; decls = decls->next)
            {
              if (decls->ival)
                expandInlineFuncsInDeclarators (decls->ival, block);
            }
        }

      if (tree->opval.op == FOR)
        {
          if (AST_FOR (tree, initExpr))
            expandInlineFuncs (AST_FOR (tree, initExpr), block);
          if (AST_FOR (tree, condExpr))
            expandInlineFuncs (AST_FOR (tree, condExpr), block);
          if (AST_FOR (tree, loopExpr))
            expandInlineFuncs (AST_FOR (tree, loopExpr), block);
        }

      if (tree->left)
        expandInlineFuncs (tree->left, block);
      if (tree->right)
        expandInlineFuncs (tree->right, block);
    }
}

/*------------------------------------------------------------*/
/* createFunctionDecl - Handle all of a function declaration  */
/*                      except for the function body.         */
/*------------------------------------------------------------*/
symbol *
createFunctionDecl (symbol *name)
{
  symbol *csym;
  value *args;
  sym_link *type;

  /* This change would be done by addSymChain() below anyway.
     But we need to do it here to avoid checkFunction() to report
     a mismatch with an earlier declaration (that already underwent the change).
     Fixed bug #2556. */
  changePointer (name->type);

  /* if check function return 0 then some problem */
  if (checkFunction (name, NULL) == 0)
    return NULL;

  /* Find the arguments in this declaration, if any */
  type = name->type;
  while (type && !IS_FUNC(type))
    type = type->next;
  assert (type);
  args = FUNC_ARGS (type);

  /* check if the function name already in the symbol table */
  if ((csym = findSym (SymbolTab, NULL, name->name)))
    {
      name = csym;
      /* special case for compiler defined functions
         we need to add the name to the publics list : this
         actually means we are now compiling the compiler
         support routine */
      if (name->cdef)
        {
          addSet (&publics, name);
        }
    }
  else
    {
      addSymChain (&name);
      allocVariables (name);
    }

  /* Now that the function name is in the symbol table, */
  /* add the names of the arguments */
  while (args)
    {
      if (args->sym)
        addSymChain (&args->sym);
      args = args->next;
    }

  return name;
}

/*-----------------------------------------------------------------*/
/* createFunction - This is the key node that calls the iCode for  */
/*                  generating the code for a function. Note code  */
/*                  is generated function by function, later when  */
/*                  add inter-procedural analysis this will change */
/*-----------------------------------------------------------------*/
ast *
createFunction (symbol * name, ast * body)
{
  ast *ex;
  int stack = 0;
  sym_link *fetype;
  iCode *piCode = NULL;

  if (!name)
    return NULL;

  if (getenv ("SDCC_DEBUG_FUNCTION_POINTERS"))
    fprintf (stderr, "SDCCast.c:createFunction(%s)\n", name->name);

  /* create a dummy block if none exists */
  if (!body)
    {
      body = newNode (BLOCK, NULL, NULL);
      body->block = ++blockNo;
    }

  noLineno++;

  name->lastLine = lexLineno;
  currFunc = name;

  /* set the stack pointer */
  stackPtr = -port->stack.direction * port->stack.call_overhead;
  xstackPtr = 0;

  if (IFFUNC_ISISR (name->type))
    stackPtr -= port->stack.direction * port->stack.isr_overhead;

  if (IFFUNC_ISREENT (name->type) || options.stackAuto)
    {
      if (options.useXstack)
        xstackPtr -= port->stack.direction * port->stack.reent_overhead;
      else
        stackPtr -= port->stack.direction * port->stack.reent_overhead;
    }

  if (IFFUNC_ISBANKEDCALL (name->type))
    stackPtr -= port->stack.direction * port->stack.banked_overhead;

  fetype = getSpec (name->type);        /* get the specifier for the function */
  /* if this is a reentrant function then */
  if (IFFUNC_ISREENT (name->type))
    reentrant++;

  if (FUNC_ISINLINE (name->type) && FUNC_ISNAKED (name->type))
    werrorfl (name->fileDef, name->lineDef, W_INLINE_NAKED, name->name);

  inlineState.count = 0;
  expandInlineFuncs (body, NULL);

  if (FUNC_ISINLINE (name->type))
    name->funcTree = copyAst (body);

  allocParms (FUNC_ARGS (name->type), IFFUNC_ISSMALLC (name->type));  /* allocate the parameters */

  /* do processing for parameters that are passed in registers */
  processRegParms (FUNC_ARGS (name->type), body);

  /* set the stack pointer */
  stackPtr = 0;
  xstackPtr = -1;

  gatherImplicitVariables (body, NULL); /* move implicit variables into blocks */

  /* allocate & autoinit the block variables */
  processBlockVars (body, &stack, ALLOCATE);

  /* name needs to be mangled */
  SNPRINTF (name->rname, sizeof (name->rname), "%s%s", port->fun_prefix, name->name);

  body = resolveSymbols (body); /* resolve the symbols */
  body = decorateType (body, RESULT_TYPE_NONE); /* propagateType & do semantic checks */

  /* save the stack information */
  if (options.useXstack)
    name->xstack = SPEC_STAK (fetype) = stack;
  else
    name->stack = SPEC_STAK (fetype) = stack;

  ex = newAst_VALUE (symbolVal (name)); /* create name */
  ex = newNode (FUNCTION, ex, body);
  ex->block = body->block;
  ex->values.args = FUNC_ARGS (name->type);
  ex->decorated = 1;
  if (options.dump_ast)
    PA (ex);
  if (fatalError)
    goto skipall;

  /* Do not generate code for inline functions unless extern also. */
#if 0
  if (FUNC_ISINLINE (name->type) && !IS_EXTERN (fetype))
    goto skipall;
#else
  /* Temporary hack: always generate code for static inline functions. */
  /* Ideally static inline functions should only be generated if needed. */
  if (FUNC_ISINLINE (name->type) && !IS_EXTERN (fetype) && !IS_STATIC (fetype))
    goto skipall;
#endif

  /* create the node & generate intermediate code */
  GcurMemmap = code;
  codeOutBuf = &code->oBuf;
  piCode = iCodeFromAst (ex);
  name->generated = 1;

  if (fatalError)
    goto skipall;

  eBBlockFromiCode (piCode);

  /* if there are any statics then do them */
  if (staticAutos)
    {
      GcurMemmap = statsg;
      codeOutBuf = &statsg->oBuf;
      eBBlockFromiCode (iCodeFromAst (decorateType (resolveSymbols (staticAutos), RESULT_TYPE_NONE)));
      staticAutos = NULL;
    }

skipall:
  /* dealloc the block variables */
  processBlockVars (body, &stack, DEALLOCATE);
  if (!fatalError)
    outputDebugStackSymbols ();
  /* deallocate paramaters */
  deallocParms (FUNC_ARGS (name->type));

  if (IFFUNC_ISREENT (name->type))
    reentrant--;

  /* we are done freeup memory & cleanup */
  noLineno--;
  if (port->reset_labelKey)
    labelKey = 1;
  name->key = 0;
  FUNC_HASBODY (name->type) = 1;
  addSet (&operKeyReset, name);
  applyToSet (operKeyReset, resetParmKey);

  if (options.debug)
    cdbStructBlock (1);

  cleanUpLevel (LabelTab, 0);
  cleanUpBlock (StructTab, 1);
  cleanUpBlock (TypedefTab, 1);
  cleanUpBlock (AddrspaceTab, 1);

  if (xstack)
    xstack->syms = NULL;
  istack->syms = NULL;
  currFunc = NULL;
  return NULL;
}

#define INDENT(x,f) do { fprintf (f, "%s:%d:", tree->filename, tree->lineno); fprintf (f, "%*s", (x) & 0xff, ""); } while (0)
/*-----------------------------------------------------------------*/
/* ast_print : prints the ast (for debugging purposes)             */
/*-----------------------------------------------------------------*/

void
ast_print (ast * tree, FILE * outfile, int indent)
{
  if (!tree)
    return;

  /* can print only decorated trees */
  if (!tree->decorated)
    {
      fprintf (outfile, "tree (%p) not decorated\n", tree);
      return;
    }

  /* if any child is an error | this one is an error do nothing */
  if (tree->isError || (tree->left && tree->left->isError) || (tree->right && tree->right->isError))
    {
      fprintf (outfile, "ERROR_NODE(%p)\n", tree);
    }

  if (tree->opval.op == FUNCTION)
    {
      int arg = 0;
      value *args;

      assert (tree->left != NULL);
      assert (tree->left->opval.val != NULL);

      args = FUNC_ARGS (tree->left->opval.val->type);
      fprintf (outfile, "FUNCTION (%s=%p) type (", tree->left->opval.val->name, tree);
      printTypeChain (tree->left->opval.val->type->next, outfile);
      fprintf (outfile, ") args (");
      do
        {
          if (arg)
            {
              fprintf (outfile, ", ");
            }
          printTypeChain (args ? args->type : NULL, outfile);
          arg++;
          args = args ? args->next : NULL;
        }
      while (args);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent);
      ast_print (tree->right, outfile, indent);
      return;
    }
  if (tree->opval.op == BLOCK)
    {
      symbol *decls = tree->values.sym;
      INDENT (indent, outfile);
      fprintf (outfile, "{ L%ld:%ld B%d\n", tree->level / LEVEL_UNIT, tree->level % LEVEL_UNIT, tree->block);
      while (decls)
        {
          INDENT (indent + 2, outfile);
          fprintf (outfile, "DECLARE SYMBOL (L%ld:%ld B%d %s=%p) type (",
                   decls->level / LEVEL_UNIT, decls->level % LEVEL_UNIT, decls->block, decls->name, decls);
          printTypeChain (decls->type, outfile);
          fprintf (outfile, ")");
          if (decls->ival)
            {
              struct initList *ival = decls->ival;
              fprintf (outfile, " = \n");
              for (; ival; ival = ival->next)
                {
                  if (ival->type == INIT_NODE)
                    {
                      ast_print (ival->init.node, outfile, indent + 4);
                    }
                  else
                    {
                      INDENT (indent + 4, outfile);
                      fprintf (outfile, "HUGE initializer skipped\n");
                    }
                }
            }
          else
            {
              fprintf (outfile, "\n");
            }

          decls = decls->next;
        }
      ast_print (tree->right, outfile, indent + 2);
      INDENT (indent, outfile);
      fprintf (outfile, "}\n");
      return;
    }
  if (tree->opval.op == NULLOP)
    {
      ast_print (tree->left, outfile, indent);
      ast_print (tree->right, outfile, indent);
      return;
    }
  INDENT (indent, outfile);

  /*------------------------------------------------------------------*/
  /*----------------------------*/
  /*   leaf has been reached    */
  /*----------------------------*/
  /* if this is of type value */
  /* just get the type        */
  if (tree->type == EX_VALUE)
    {
      if (IS_LITERAL (tree->opval.val->etype))
        {
          fprintf (outfile, "CONSTANT (%p) value = ", tree);
          if (SPEC_LONGLONG (tree->opval.val->etype))
            {
              unsigned long long ull = ullFromVal (tree->opval.val);

              if (SPEC_USIGN (tree->opval.val->etype))
                fprintf (outfile, "%llu, 0x%llx", ull, ull);
              else
                fprintf (outfile, "%lld, 0x%llx", (signed long long) ull, ull);
            }
          else
            {
              unsigned long ul = ulFromVal (tree->opval.val);

              if (SPEC_USIGN (tree->opval.val->etype))
                fprintf (outfile, "%lu", ul);
              else
                fprintf (outfile, "%ld", (signed long) ul);
              fprintf (outfile, ", 0x%lx, %f", ul, floatFromVal (tree->opval.val));
            }
        }
      else if (tree->opval.val->sym)
        {
          /* if the undefined flag is set then give error message */
          if (tree->opval.val->sym->undefined)
            {
              fprintf (outfile, "UNDEFINED SYMBOL ");
            }
          else
            {
              fprintf (outfile, "SYMBOL ");
            }
          fprintf (outfile, "(L%ld:%ld B%d %s=%p @ %p)",
                   tree->opval.val->sym->level / LEVEL_UNIT, tree->opval.val->sym->level % LEVEL_UNIT,
                   tree->opval.val->sym->block, tree->opval.val->sym->name, tree, tree->opval.val->sym);
        }
      if (tree->ftype)
        {
          fprintf (outfile, " type (");
          printTypeChain (tree->ftype, outfile);
          fprintf (outfile, ")\n");
        }
      else
        {
          fprintf (outfile, "\n");
        }
      return;
    }

  /* if type link for the case of cast */
  if (tree->type == EX_LINK)
    {
      fprintf (outfile, "TYPENODE (%p) type = (", tree);
      printTypeChain (tree->opval.lnk, outfile);
      fprintf (outfile, ")\n");
      return;
    }

  /* depending on type of operator do */
  switch (tree->opval.op)
    {
    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*        array node          */
    /*----------------------------*/
    case '[':
      fprintf (outfile, "ARRAY_OP (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*      struct/union          */
    /*----------------------------*/
    case '.':
      fprintf (outfile, "STRUCT_ACCESS (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*    struct/union pointer    */
    /*----------------------------*/
    case PTR_OP:
      fprintf (outfile, "PTR_ACCESS (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*  ++/-- operation           */
    /*----------------------------*/
    case INC_OP:
      if (tree->left)
        fprintf (outfile, "post-");
      else
        fprintf (outfile, "pre-");
      fprintf (outfile, "INC_OP (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);      /* postincrement case */
      ast_print (tree->right, outfile, indent + 2);     /* preincrement case */
      return;

    case DEC_OP:
      if (tree->left)
        fprintf (outfile, "post-");
      else
        fprintf (outfile, "pre-");
      fprintf (outfile, "DEC_OP (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);      /* postdecrement case */
      ast_print (tree->right, outfile, indent + 2);     /* predecrement case */
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*  bitwise and               */
    /*----------------------------*/
    case '&':
      if (tree->right)
        {
          fprintf (outfile, "& (%p) type (", tree);
          printTypeChain (tree->ftype, outfile);
          fprintf (outfile, ")\n");
          ast_print (tree->left, outfile, indent + 2);
          ast_print (tree->right, outfile, indent + 2);
        }
      else
        {
          fprintf (outfile, "ADDRESS_OF (%p) type (", tree);
          printTypeChain (tree->ftype, outfile);
          fprintf (outfile, ")\n");
          ast_print (tree->left, outfile, indent + 2);
          ast_print (tree->right, outfile, indent + 2);
        }
      return;

    /*----------------------------*/
    /*  bitwise or                */
    /*----------------------------*/
    case '|':
      fprintf (outfile, "OR (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*  bitwise xor               */
    /*----------------------------*/
    case '^':
      fprintf (outfile, "XOR (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*  division                  */
    /*----------------------------*/
    case '/':
      fprintf (outfile, "DIV (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*            modulus         */
    /*----------------------------*/
    case '%':
      fprintf (outfile, "MOD (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*  address dereference       */
    /*----------------------------*/
    case '*':                  /* can be unary  : if right is null then unary operation */
      if (!tree->right)
        {
          fprintf (outfile, "DEREF (%p) type (", tree);
          printTypeChain (tree->ftype, outfile);
          fprintf (outfile, ")\n");
          ast_print (tree->left, outfile, indent + 2);
          return;
        }
    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*      multiplication        */
    /*----------------------------*/
      fprintf (outfile, "MULT (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*    unary '+' operator      */
    /*----------------------------*/
    case '+':
      /* if unary plus */
      if (!tree->right)
        {
          fprintf (outfile, "UPLUS (%p) type (", tree);
          printTypeChain (tree->ftype, outfile);
          fprintf (outfile, ")\n");
          ast_print (tree->left, outfile, indent + 2);
        }
      else
        {
      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      addition              */
      /*----------------------------*/
          fprintf (outfile, "ADD (%p) type (", tree);
          printTypeChain (tree->ftype, outfile);
          fprintf (outfile, ")\n");
          ast_print (tree->left, outfile, indent + 2);
          ast_print (tree->right, outfile, indent + 2);
        }
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*      unary '-'             */
    /*----------------------------*/
    case '-':                  /* can be unary   */
      if (!tree->right)
        {
          fprintf (outfile, "UMINUS (%p) type (", tree);
          printTypeChain (tree->ftype, outfile);
          fprintf (outfile, ")\n");
          ast_print (tree->left, outfile, indent + 2);
        }
      else
        {
      /*------------------------------------------------------------------*/
      /*----------------------------*/
      /*      subtraction           */
      /*----------------------------*/
          fprintf (outfile, "SUB (%p) type (", tree);
          printTypeChain (tree->ftype, outfile);
          fprintf (outfile, ")\n");
          ast_print (tree->left, outfile, indent + 2);
          ast_print (tree->right, outfile, indent + 2);
        }
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*    complement              */
    /*----------------------------*/
    case '~':
      fprintf (outfile, "COMPL (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*           not              */
    /*----------------------------*/
    case '!':
      fprintf (outfile, "NOT (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*           shift            */
    /*----------------------------*/
    case RRC:
      fprintf (outfile, "RRC (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      return;

    case RLC:
      fprintf (outfile, "RLC (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      return;

    case SWAP:
      fprintf (outfile, "SWAP (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      return;

    case GETHBIT:
      fprintf (outfile, "GETHBIT (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      return;

    case GETABIT:
      fprintf (outfile, "GETABIT (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case GETBYTE:
      fprintf (outfile, "GETBYTE (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case GETWORD:
      fprintf (outfile, "GETWORD (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case LEFT_OP:
      fprintf (outfile, "LEFT_SHIFT (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case RIGHT_OP:
      fprintf (outfile, "RIGHT_SHIFT (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*         casting            */
    /*----------------------------*/
    case CAST:                 /* change the type   */
      fprintf (outfile, "CAST (%p) from type (", tree);
      assert (tree->right != NULL);
      printTypeChain (tree->right->ftype, outfile);
      fprintf (outfile, ") to type (");
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->right, outfile, indent + 2);
      return;

    case AND_OP:
      fprintf (outfile, "ANDAND (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case OR_OP:
      fprintf (outfile, "OROR (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*     comparison operators   */
    /*----------------------------*/
    case '>':
      fprintf (outfile, "GT(>) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case '<':
      fprintf (outfile, "LT(<) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case LE_OP:
      fprintf (outfile, "LE(<=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case GE_OP:
      fprintf (outfile, "GE(>=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case EQ_OP:
      fprintf (outfile, "EQ(==) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case NE_OP:
      fprintf (outfile, "NE(!=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*             sizeof         */
    /*----------------------------*/
    case SIZEOF:               /* evaluate wihout code generation */
      fprintf (outfile, "SIZEOF %d\n", (getSize (tree->right->ftype)));
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /* conditional operator  '?'  */
    /*----------------------------*/
    case '?':
      fprintf (outfile, "QUEST(?) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case ':':
      fprintf (outfile, "COLON(:) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*    assignment operators    */
    /*----------------------------*/
    case MUL_ASSIGN:
      fprintf (outfile, "MULASS(*=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case DIV_ASSIGN:
      fprintf (outfile, "DIVASS(/=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case AND_ASSIGN:
      fprintf (outfile, "ANDASS(&=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case OR_ASSIGN:
      fprintf (outfile, "ORASS(|=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case XOR_ASSIGN:
      fprintf (outfile, "XORASS(^=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case RIGHT_ASSIGN:
      fprintf (outfile, "RSHFTASS(>>=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case LEFT_ASSIGN:
      fprintf (outfile, "LSHFTASS(<<=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*    -= operator             */
    /*----------------------------*/
    case SUB_ASSIGN:
      fprintf (outfile, "SUBASS(-=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*          += operator       */
    /*----------------------------*/
    case ADD_ASSIGN:
      fprintf (outfile, "ADDASS(+=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*      straight assignemnt   */
    /*----------------------------*/
    case '=':
      fprintf (outfile, "ASSIGN(=) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*      comma operator        */
    /*----------------------------*/
    case ',':
      fprintf (outfile, "COMMA(,) (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*       function call        */
    /*----------------------------*/
    case CALL:
    case PCALL:
      fprintf (outfile, "CALL (%p) type (", tree);
      printTypeChain (tree->ftype, outfile);
      fprintf (outfile, ")\n");
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    case PARAM:
      fprintf (outfile, "PARMS\n");
      ast_print (tree->left, outfile, indent + 2);
      if (tree->right /*&& !IS_AST_PARAM(tree->right) */ )
        {
          ast_print (tree->right, outfile, indent + 2);
        }
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*     return statement       */
    /*----------------------------*/
    case RETURN:
      fprintf (outfile, "RETURN (%p) type (", tree);
      if (tree->right)
        {
          printTypeChain (tree->right->ftype, outfile);
        }
      fprintf (outfile, ")\n");
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*     label statement        */
    /*----------------------------*/
    case LABEL:
      fprintf (outfile, "LABEL (%p)\n", tree);
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /*     switch statement       */
    /*----------------------------*/
    case SWITCH:
      {
        value *val;
        fprintf (outfile, "SWITCH (%p) ", tree);
        ast_print (tree->left, outfile, 0);
        for (val = tree->values.switchVals.swVals; val; val = val->next)
          {
            INDENT (indent + 2, outfile);
            fprintf (outfile, "CASE 0x%x GOTO _case_%d_%d%s\n",
                     (int) ulFromVal (val),
                     tree->values.switchVals.swNum,
                     (int) ulFromVal (val), tree->values.switchVals.swSuffix ? tree->values.switchVals.swSuffix : "");
          }
        ast_print (tree->right, outfile, indent);
      }
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /* ifx Statement              */
    /*----------------------------*/
    case IFX:
      fprintf (outfile, "IF (%p) \n", tree);
      ast_print (tree->left, outfile, indent + 2);
      if (tree->trueLabel)
        {
          INDENT (indent + 2, outfile);
          fprintf (outfile, "NE(!=) 0 goto %s\n", tree->trueLabel->name);
        }
      if (tree->falseLabel)
        {
          INDENT (indent + 2, outfile);
          fprintf (outfile, "EQ(==) 0 goto %s\n", tree->falseLabel->name);
        }
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*----------------------------*/
    /* goto Statement              */
    /*----------------------------*/
    case GOTO:
      fprintf (outfile, "GOTO (%p) \n", tree);
      ast_print (tree->left, outfile, indent + 2);
      ast_print (tree->right, outfile, indent + 2);
      return;

    /*------------------------------------------------------------------*/
    /*----------------------------*/
    /* for Statement              */
    /*----------------------------*/
    case FOR:
      fprintf (outfile, "FOR (%p) \n", tree);
      if (AST_FOR (tree, initExpr))
        {
          INDENT (indent + 2, outfile);
          fprintf (outfile, "INIT EXPR ");
          ast_print (AST_FOR (tree, initExpr), outfile, indent + 2);
        }
      if (AST_FOR (tree, condExpr))
        {
          INDENT (indent + 2, outfile);
          fprintf (outfile, "COND EXPR ");
          ast_print (AST_FOR (tree, condExpr), outfile, indent + 2);
        }
      if (AST_FOR (tree, loopExpr))
        {
          INDENT (indent + 2, outfile);
          fprintf (outfile, "LOOP EXPR ");
          ast_print (AST_FOR (tree, loopExpr), outfile, indent + 2);
        }
      fprintf (outfile, "FOR LOOP BODY \n");
      ast_print (tree->left, outfile, indent + 2);
      return;

    case CRITICAL:
      fprintf (outfile, "CRITICAL (%p) \n", tree);
      ast_print (tree->left, outfile, indent + 2);
    default:
      return;
    }
}

void
PA (ast * t)
{
  ast_print (t, stdout, 0);
}

/*-----------------------------------------------------------------*/
/* astErrors : returns non-zero if errors present in tree          */
/*-----------------------------------------------------------------*/
int
astErrors (ast * t)
{
  int errors = 0;

  if (t)
    {
      if (t->isError)
        errors++;

      if (t->type == EX_VALUE && t->opval.val->sym && t->opval.val->sym->undefined)
        errors++;

      errors += astErrors (t->left);
      errors += astErrors (t->right);
    }

  return errors;
}

/*-----------------------------------------------------------------------------
 * verbatim copy from the gnu gcc-4.1 info page
 * >
 * >   info node:   (gcc-4.1)Offsetof
 * >
 * >      primary:
 * >        "__builtin_offsetof" "(" `typename' "," offsetof_member_designator ")"
 * >
 * >      offsetof_member_designator:
 * >          `identifier'
 * >        | offsetof_member_designator "." `identifier'
 * >        | offsetof_member_designator "[" `expr' "]"
 * >
 * >  This extension is sufficient such that
 * >
 * >     #define offsetof(TYPE, MEMBER)  __builtin_offsetof (TYPE, MEMBER)
 * >  ...
 *
 * Note:
 *   `expr',  here is indeed any expr valid, not only constants.
 *   `typename', gcc supports typeof(...) here, but the sdcc typeof has
 *                a completely different semantic, more sort of typeid
 *
 */

static ast *
offsetofOp_rec (sym_link *type, ast *snd, sym_link **result_type)
{
  /* make sure the type is complete and sane */
  checkTypeSanity (type, "(offsetofOp)");

  /* offsetof can only be applied to structs/unions */
  if (!IS_STRUCT (type) || !getSize (type))
    {
      werrorfl (snd->filename, snd->lineno, E_OFFSETOF_TYPE);
      *result_type = 0;
      return newAst_VALUE (valueFromLit (0));
    }

  /* offsetof(struct_type, symbol); */
  if (IS_AST_SYM_VALUE (snd))
    {
      structdef *structdef = SPEC_STRUCT (type);
      symbol *element = getStructElement (structdef, AST_SYMBOL (snd));
      *result_type = element->type;
      return newAst_VALUE (valueFromLit (element->offset));
    }

  /* offsetof(struct_type, a.something); */
  if (IS_AST_OP (snd) && snd->opval.op == '.')
    {
      sym_link *tmp;
      ast *o = offsetofOp_rec (type, snd->left, &tmp);
      return newNode ('+', o, offsetofOp_rec (tmp, snd->right, result_type));
    }

  /* offsetof(struct_type, a[expr]); */
  if (IS_ARRAY_OP (snd))
    {
      sym_link *tmp;
      ast *o = offsetofOp_rec (type, snd->left, &tmp);
      *result_type = tmp->next;
      return newNode ('+', o, newNode ('*', newAst_VALUE (valueFromLit (getSize (tmp->next))), snd->right));
    }

  wassertl (0, "this should never have happened");
  exit (1);
}

ast *
offsetofOp (sym_link *type, ast *snd)
{
  sym_link *result_type;

  return offsetofOp_rec (type, snd, &result_type);
}

