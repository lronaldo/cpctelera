/*-------------------------------------------------------------------------
  SDCCmem.c - 8051 memory management routines
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


#include "common.h"
#include "dbuf_string.h"
#include "SDCCbtree.h"

/* memory segments */
memmap *xstack = NULL;          /* xternal stack data          */
memmap *istack = NULL;          /* internal stack              */
memmap *code = NULL;            /* code segment                */
memmap *data = NULL;            /* internal data upto 128      */
memmap *initialized = NULL;     /* initialized data, such as initialized, nonzero globals or local statics. */
memmap *initializer = NULL;     /* a copz of the values for the initialized data from initialized in code space */
memmap *pdata = NULL;           /* paged external data         */
memmap *xdata = NULL;           /* external data               */
memmap *xidata = NULL;          /* the initialized xdata       */
memmap *xinit = NULL;           /* the initializers for xidata */
memmap *idata = NULL;           /* internal data upto 256      */
memmap *bit = NULL;             /* bit addressable space       */
memmap *statsg = NULL;          /* the constant data segment   */
memmap *c_abs = NULL;           /* constant absolute data      */
memmap *x_abs = NULL;           /* absolute xdata/pdata        */
memmap *i_abs = NULL;           /* absolute idata upto 256     */
memmap *d_abs = NULL;           /* absolute data upto 128      */
memmap *sfr = NULL;             /* register space              */
memmap *reg = NULL;             /* register space              */
memmap *sfrbit = NULL;          /* sfr bit space               */
memmap *generic = NULL;         /* is a generic pointer        */
memmap *overlay = NULL;         /* overlay segment             */
memmap *eeprom = NULL;          /* eeprom location             */
memmap *home = NULL;            /* Unswitchable code bank      */
namedspacemap *namedspacemaps = 0; /* memory segments for named address spaces */

/* this is a set of sets each set containing
   symbols in a single overlay */
set *ovrSetSets = NULL;

int fatalError = 0;             /* fatal error flag            */

/*-----------------------------------------------------------------*/
/* allocMap - allocates a memory map                               */
/*-----------------------------------------------------------------*/
memmap *
allocMap (char rspace,          /* sfr space                   */
          char farmap,          /* far or near segment         */
          char paged,           /* can this segment be paged   */
          char direct,          /* directly addressable        */
          char bitaddr,         /* bit addressable space       */
          char codemap,         /* this is code space          */
          unsigned sloc,        /* starting location           */
          const char *name,     /* 8 character name            */
          char dbName,          /* debug name                  */
          int ptrType           /* pointer type for this space */
)
{
  memmap *map;

  if (!name)
    return NULL;

  if (!(map = Safe_alloc (sizeof (memmap))))
    {
      werror (E_OUT_OF_MEM, __FILE__, sizeof (memmap));
      exit (1);
    }

  memset (map, ZERO, sizeof (memmap));
  map->regsp = rspace;
  map->fmap = farmap;
  map->paged = paged;
  map->direct = direct;
  map->bitsp = bitaddr;
  map->codesp = codemap;
  map->sloc = sloc;
  map->sname = name;
  map->dbName = dbName;
  map->ptrType = ptrType;
  map->syms = NULL;

  dbuf_init(&map->oBuf, 4096);

  return map;
}

/*-----------------------------------------------------------------*/
/* initMem - allocates and initializes all the segments            */
/*-----------------------------------------------------------------*/
void
initMem ()
{
  /* allocate all the segments */
  /* xternal stack segment ;
     SFRSPACE       -   NO
     FAR-SPACE      -   YES
     PAGED          -   YES
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'A'
     POINTER-TYPE   -   FPOINTER
   */
  xstack = allocMap (0, 1, 1, 0, 0, 0, options.xstack_loc, XSTACK_NAME, 'A', PPOINTER);

  /* internal stack segment ;
     SFRSPACE       -   NO
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'B'
     POINTER-TYPE   -   POINTER
   */
  istack = allocMap (0, 0, 0, 0, 0, 0, options.stack_loc, ISTACK_NAME, 'B', POINTER);

  /* code  segment ;
     SFRSPACE       -   NO
     FAR-SPACE      -   YES
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   YES
     DEBUG-NAME     -   'C'
     POINTER-TYPE   -   CPOINTER
   */
  code = allocMap (0, 1, 0, 0, 0, 1, options.code_loc, CODE_NAME, 'C', CPOINTER);

  /* home  segment ;
     SFRSPACE       -   NO
     FAR-SPACE      -   YES
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   YES
     DEBUG-NAME     -   'C'
     POINTER-TYPE   -   CPOINTER
   */
  home = allocMap (0, 1, 0, 0, 0, 1, options.code_loc, HOME_NAME, 'C', CPOINTER);

  /* Static segment (code for variables );
     SFRSPACE       -   NO
     FAR-SPACE      -   YES
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   YES
     DEBUG-NAME     -   'D'
     POINTER-TYPE   -   CPOINTER
   */
  statsg = allocMap (0, 1, 0, 0, 0, 1, 0, STATIC_NAME, 'D', CPOINTER);

  /* Constant Absolute Data segment (for variables );
     SFRSPACE       -   NO
     FAR-SPACE      -   YES
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   YES
     DEBUG-NAME     -   'D'
     POINTER-TYPE   -   CPOINTER
   */
  c_abs = allocMap (0, 1, 0, 0, 0, 1, 0, CABS_NAME, 'D', CPOINTER);

  /* Data segment - internal storage segment ;
     SFRSPACE       -   NO
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   YES
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'E'
     POINTER-TYPE   -   POINTER
   */
  data = allocMap (0, 0, 0, 1, 0, 0, options.data_loc, DATA_NAME, 'E', POINTER);

  initialized = allocMap (0, 0, 0, 1, 0, 0, options.data_loc, INITIALIZED_NAME, 'E', POINTER);
  initializer = allocMap (0, 0, 0, 1, 0, 1, options.code_loc, INITIALIZER_NAME, 'C', CPOINTER);

  /* Absolute internal storage segment ;
     SFRSPACE       -   NO
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   YES
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'E'
     POINTER-TYPE   -   POINTER
   */
  d_abs = allocMap (0, 0, 0, 1, 0, 0, options.data_loc, IABS_NAME, 'E', POINTER);

  /* overlay segment - same as internal storage segment ;
     SFRSPACE       -   NO
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   YES
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'E'
     POINTER-TYPE   -   POINTER
   */
  if (OVERLAY_NAME)
      overlay = allocMap (0, 0, 0, 1, 0, 0, options.data_loc, DATA_NAME, 'E', POINTER);

  /* Xternal paged segment ;   
     SFRSPACE       -   NO
     FAR-SPACE      -   NO
     PAGED          -   YES
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO 
     DEBUG-NAME     -   'P'
     POINTER-TYPE   -   PPOINTER
   */
  pdata = allocMap (0, 0, 1, 0, 0, 0, options.xstack_loc, PDATA_NAME, 'P', PPOINTER);

  /* Xternal Data segment -
     SFRSPACE       -   NO
     FAR-SPACE      -   YES
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'F'
     POINTER-TYPE   -   FPOINTER
   */
  xdata = allocMap (0, 1, 0, 0, 0, 0, options.xdata_loc, XDATA_NAME, 'F', FPOINTER);
  xidata = allocMap (0, 1, 0, 0, 0, 0, 0, XIDATA_NAME, 'F', FPOINTER);
  xinit = allocMap (0, 1, 0, 0, 0, 1, 0, XINIT_NAME, 'C', CPOINTER);

  /* Absolute external storage segment ;
     SFRSPACE       -   NO
     FAR-SPACE      -   YES
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'F'
     POINTER-TYPE   -   FPOINTER
   */
  x_abs = allocMap (0, 1, 0, 0, 0, 0, options.xdata_loc, XABS_NAME, 'F', FPOINTER);

  /* Indirectly addressed internal data segment
     SFRSPACE       -   NO
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'G'
     POINTER-TYPE   -   IPOINTER
   */
  idata = allocMap (0, 0, 0, 0, 0, 0, options.idata_loc, IDATA_NAME, 'G', IPOINTER);

  /* Indirectly addressed absolute internal segment
     SFRSPACE       -   NO
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'E'
     POINTER-TYPE   -   IPOINTER
   */
  i_abs = allocMap (0, 0, 0, 0, 0, 0, options.data_loc, IABS_NAME, 'E', IPOINTER);

  /* Bit space ;
     SFRSPACE       -   NO
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   YES
     BIT-ACCESS     -   YES
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'H'
     POINTER-TYPE   -  _NONE_
   */
  bit = allocMap (0, 0, 0, 1, 1, 0, 0, BIT_NAME, 'H', 0);

  /* Special function register space :-
     SFRSPACE       -   YES
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   YES
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'I'
     POINTER-TYPE   -   _NONE_
   */
  sfr = allocMap (1, 0, 0, 1, 0, 0, 0, REG_NAME, 'I', 0);

  /* Register space ;
     SFRSPACE       -   YES
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   ' '
     POINTER-TYPE   -   _NONE_
   */
  reg = allocMap (1, 0, 0, 0, 0, 0, 0, REG_NAME, ' ', 0);

  /* SFR bit space
     SFRSPACE       -   YES
     FAR-SPACE      -   NO
     PAGED          -   NO
     DIRECT-ACCESS  -   YES
     BIT-ACCESS     -   YES
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'J'
     POINTER-TYPE   -   _NONE_
   */
  sfrbit = allocMap (1, 0, 0, 1, 1, 0, 0, REG_NAME, 'J', 0);

  /* EEPROM space
     SFRSPACE       -   NO
     FAR-SPACE      -   YES
     PAGED          -   NO
     DIRECT-ACCESS  -   NO
     BIT-ACCESS     -   NO
     CODE-ACCESS    -   NO
     DEBUG-NAME     -   'K'
     POINTER-TYPE   -   EEPPOINTER
   */
  eeprom = allocMap (0, 1, 0, 0, 0, 0, 0, REG_NAME, 'K', EEPPOINTER);

  /* the unknown map */
  generic = allocMap (1, 0, 0, 1, 1, 0, 0, REG_NAME, ' ', GPOINTER);

}

/*-----------------------------------------------------------------*/
/* allocIntoSeg - puts a symbol into a memory segment              */
/*-----------------------------------------------------------------*/
void
allocIntoSeg (symbol *sym)
{
  memmap *segment;

  if (SPEC_ADDRSPACE (sym->etype))
    {
      namedspacemap *nm;
      for (nm = namedspacemaps; nm; nm = nm->next)
        if (!strcmp (nm->name, SPEC_ADDRSPACE (sym->etype)->name))
          break;

      if (!nm)
        {
          nm = Safe_alloc (sizeof (namedspacemap));
          nm->name = Safe_alloc (strlen(SPEC_ADDRSPACE (sym->etype)->name) + 1);
          strcpy (nm->name, SPEC_ADDRSPACE (sym->etype)->name);
          nm->is_const = (SPEC_ADDRSPACE (sym->etype)->type && SPEC_CONST (SPEC_ADDRSPACE (sym->etype)->type));
          nm->map = nm->is_const ?
            allocMap (0, 1, 0, 0, 0, 1, options.code_loc, SPEC_ADDRSPACE (sym->etype)->name, 'C', CPOINTER) :
            allocMap (0, 0, 0, 1, 0, 0, options.data_loc, SPEC_ADDRSPACE (sym->etype)->name, 'E', POINTER);
          nm->next = namedspacemaps;
          namedspacemaps = nm;
        }

      addSet (&nm->map->syms, sym);

      return;
    }
  segment = SPEC_OCLS (sym->etype);
  addSet (&segment->syms, sym);
  if (segment == pdata)
    sym->iaccess = 1;
}

/*-----------------------------------------------------------------*/
/* deleteFromSeg - deletes a symbol from segment used when a var   */
/*                 first declared as "extern" then no extern       */
/*-----------------------------------------------------------------*/
void deleteFromSeg(symbol *sym)
{
  if (SPEC_OCLS(sym->etype))
    {
      memmap *segment = SPEC_OCLS (sym->etype);
      deleteSetItem(&segment->syms, sym);
    }
}

/*-----------------------------------------------------------------*/
/* defaultOClass - set the output segment based on SCLASS          */
/*-----------------------------------------------------------------*/
bool
defaultOClass (symbol *sym)
{
  switch (SPEC_SCLS (sym->etype))
    {
    case S_SFR:
      SPEC_OCLS (sym->etype) = sfr;
      break;
    case S_SBIT:
      SPEC_OCLS (sym->etype) = sfrbit;
      break;
    case S_CODE:
      if (sym->_isparm)
        return FALSE;
      /* if code change to constant */
      if (sym->ival && SPEC_ABSA (sym->etype))
        {
          SPEC_OCLS(sym->etype) = c_abs;
        }
      else
        {
          SPEC_OCLS (sym->etype) = statsg;
        }
      break;
    case S_XDATA:
      /* absolute initialized global */
      if (sym->ival && SPEC_ABSA (sym->etype))
        {
          SPEC_OCLS(sym->etype) = x_abs;
        }
      /* or should we move this to the initialized data segment? */
      else if (port->genXINIT && sym->ival && (sym->level==0))
        {
          SPEC_OCLS(sym->etype) = xidata;
        }
      else
        {
          SPEC_OCLS (sym->etype) = xdata;
        }
      break;
    case S_DATA:
      /* Absolute initialized global */
      if (sym->ival && SPEC_ABSA (sym->etype))
        {
          SPEC_OCLS (sym->etype) = d_abs;
        }
      /* Other initialized global */
      else if (sym->ival && port->mem.initialized_name && sym->level == 0)
        {
          SPEC_OCLS (sym->etype) = initialized;
        }
      else
        {
          SPEC_OCLS (sym->etype) = data;
        }
      break;
    case S_IDATA:
      /* absolute initialized global */
      if (sym->ival && SPEC_ABSA (sym->etype))
        {
          SPEC_OCLS(sym->etype) = i_abs;
        }
      else
        {
          SPEC_OCLS (sym->etype) = idata;
        }
      sym->iaccess = 1;
      break;
    case S_PDATA:
      SPEC_OCLS (sym->etype) = pdata;
      sym->iaccess = 1;
      break;
    case S_BIT:
      SPEC_OCLS (sym->etype) = bit;
      break;
    case S_EEPROM:
      SPEC_OCLS (sym->etype) = eeprom;
      break;
    default:
      return FALSE;
    }
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* allocDefault - assigns the output segment based on SCLASS       */
/*-----------------------------------------------------------------*/
bool
allocDefault (struct symbol * sym)
{
  if (defaultOClass (sym))
    {
      allocIntoSeg (sym);
      return TRUE;
    }
  return FALSE;
}

/*-----------------------------------------------------------------*/
/* allocGlobal - assigns the output segment to a global var        */
/*-----------------------------------------------------------------*/
void
allocGlobal (symbol * sym)
{
  /* symbol name is internal name  */
  if (!sym->level)              /* local statics can come here */
    SNPRINTF (sym->rname, sizeof(sym->rname),
              "%s%s", port->fun_prefix, sym->name);

  /* add it to the operandKey reset */
  if (!isinSet (operKeyReset, sym))
    {
      addSet(&operKeyReset, sym);
    }

  /* if this is a literal e.g. enumerated type */
  /* put it in the data segment & do nothing   */
  if (IS_LITERAL (sym->etype))
    {
      SPEC_OCLS (sym->etype) = data;
      return;
    }

  /* if this is a function then assign code space    */
  if (IS_FUNC (sym->type))
    {
      SPEC_OCLS (sym->etype) = code;
      /* if this is an interrupt service routine
         then put it in the interrupt service array */
      if (FUNC_ISISR (sym->type) && !options.noiv &&
          (FUNC_INTNO (sym->type) != INTNO_UNSPEC))
        {
          if (interrupts[FUNC_INTNO (sym->type)])
            werror (E_INT_DEFINED,
                    FUNC_INTNO (sym->type),
                    interrupts[FUNC_INTNO (sym->type)]->name);
          else
            interrupts[FUNC_INTNO (sym->type)] = sym;

          /* automagically extend the maximum interrupts */
          if (FUNC_INTNO (sym->type) >= maxInterrupts && FUNC_INTNO (sym->type)!=INTNO_TRAP)
            maxInterrupts = FUNC_INTNO (sym->type) + 1;
        }
      /* if it is not compiler defined */
      if (!sym->cdef)
        allocIntoSeg (sym);

      return;
    }

  /* if this is a bit variable and no storage class */
  if (bit && IS_SPEC(sym->type) && SPEC_NOUN (sym->type) == V_BIT)
    {
      SPEC_OCLS (sym->type) = bit;
      allocIntoSeg (sym);
      return;
    }

  if (!TARGET_IS_PIC16 || sym->level)
    /* register storage class ignored changed to FIXED */
    if (SPEC_SCLS (sym->etype) == S_REGISTER)
      SPEC_SCLS (sym->etype) = S_FIXED;

  /* if it is fixed, then allocate depending on the */
  /* current memory model, same for automatics      */
  if (SPEC_SCLS (sym->etype) == S_FIXED ||
      (TARGET_IS_PIC16 && (SPEC_SCLS (sym->etype) == S_REGISTER) && (sym->level == 0)) ||
      SPEC_SCLS (sym->etype) == S_AUTO)
    {
      if (port->mem.default_globl_map != xdata)
        {
          if (sym->ival && SPEC_ABSA (sym->etype))
            {
              /* absolute initialized global */
              SPEC_OCLS (sym->etype) = x_abs;
            }
          else if (sym->ival && sym->level == 0 && port->mem.initialized_name)
            {
              SPEC_OCLS (sym->etype) = initialized;
            }
          else
            {
              /* set the output class */
              SPEC_OCLS (sym->etype) = port->mem.default_globl_map;
            }
          /* generate the symbol  */
          allocIntoSeg (sym);
          return;
        }
      else
        {
          SPEC_SCLS (sym->etype) = S_XDATA;
        }
    }

  allocDefault (sym);
  return;
}

/*-----------------------------------------------------------------*/
/* allocParms - parameters are always passed on stack              */
/*-----------------------------------------------------------------*/
void
allocParms (value *val, bool smallc)
{
  value *lval;
  int pNum = 1;
  int stackParamSizeAdjust = 0;

  if (IFFUNC_ISSMALLC (currFunc->type))
    {
      for (lval = val; lval; lval = lval->next)
      {
        if (IS_REGPARM (lval->etype))
          continue;
        stackParamSizeAdjust += getSize (lval->type);
      }
    }
  stackPtr += stackParamSizeAdjust;

  for (lval = val; lval; lval = lval->next, pNum++)
    {
      /* check the declaration */
      checkDecl (lval->sym, 0);

      /* if this a register parm then allocate
         it as a local variable by adding it
         to the first block we see in the body */
      if (IS_REGPARM (lval->etype))
        continue;

      /* mark it as my parameter */
      lval->sym->ismyparm = 1;
      lval->sym->localof = currFunc;

      /* if automatic variables r 2b stacked */
      if (options.stackAuto || IFFUNC_ISREENT (currFunc->type))
        {
          if (lval->sym)
            lval->sym->onStack = 1;

          /* choose which stack 2 use   */
          /*  use xternal stack */
          if (options.useXstack)
            {
              /* PENDING: stack direction support */
              wassertl (!smallc, "SmallC calling convention not yet supported for xstack callee");
              SPEC_OCLS (lval->etype) = SPEC_OCLS (lval->sym->etype) = xstack;
              SPEC_STAK (lval->etype) = SPEC_STAK (lval->sym->etype) = lval->sym->stack =
                xstackPtr - getSize (lval->type);
              xstackPtr -= getSize (lval->type);
            }
          else
            {                   /* use internal stack   */
              SPEC_OCLS (lval->etype) = SPEC_OCLS (lval->sym->etype) = istack;
              if ((port->stack.direction > 0) ^(IFFUNC_ISSMALLC (currFunc->type)))
                {
                  SPEC_STAK (lval->etype) = SPEC_STAK (lval->sym->etype) = lval->sym->stack =
                    stackPtr - (FUNC_REGBANK (currFunc->type) ? port->stack.bank_overhead : 0) -
                    getSize (lval->type) -
                    (FUNC_ISISR (currFunc->type) ? port->stack.isr_overhead : 0);
                  stackPtr -= getSize (lval->type);
                }
              else
                {
                  /* This looks like the wrong order but it turns out OK... */
                  /* PENDING: isr, bank overhead, ... */
                  SPEC_STAK (lval->etype) = SPEC_STAK (lval->sym->etype) = lval->sym->stack =
                    stackPtr +
                    (FUNC_ISISR (currFunc->type) ? port->stack.isr_overhead : 0) +
                    0;
                  stackPtr += getSize (lval->type);
                }
            }
          allocIntoSeg (lval->sym);
        }
      else
        {
          /* Do not allocate for inline functions to avoid multiple definitions - see bug report #2591. */
          if(IFFUNC_ISINLINE (currFunc->type) && !IS_STATIC (currFunc->etype) && !IS_EXTERN (currFunc->etype))
            continue;

          /* allocate them in the automatic space */
          /* generate a unique name  */
          SNPRINTF (lval->sym->rname, sizeof(lval->sym->rname),
                    "%s%s_PARM_%d", port->fun_prefix, currFunc->name, pNum);
          strncpyz (lval->name, lval->sym->rname, sizeof(lval->name));

          /* if declared in specific storage */
          if (allocDefault (lval->sym))
            {
              SPEC_OCLS (lval->etype) = SPEC_OCLS (lval->sym->etype);
              continue;
            }

          /* otherwise depending on the memory model */
          SPEC_OCLS (lval->etype) = SPEC_OCLS (lval->sym->etype) =
              port->mem.default_local_map;
          if (options.model == MODEL_SMALL)
            {
              /* note here that we put it into the overlay segment
                 first, we will remove it from the overlay segment
                 after the overlay determination has been done */
              if (!options.noOverlay)
                {
                  SPEC_OCLS (lval->etype) = SPEC_OCLS (lval->sym->etype) =
                    overlay;
                }
            }
          else if (options.model == MODEL_MEDIUM)
            {
              SPEC_SCLS (lval->etype) = S_PDATA;
            }
          else
            {
              SPEC_SCLS (lval->etype) = S_XDATA;
            }
          allocIntoSeg (lval->sym);
        }
    }

  stackPtr -= stackParamSizeAdjust;

  return;
}

/*-----------------------------------------------------------------*/
/* deallocParms - parameters are always passed on stack            */
/*-----------------------------------------------------------------*/
void
deallocParms (value * val)
{
  value *lval;

  for (lval = val; lval; lval = lval->next)
    {
      /* unmark is myparm */
      lval->sym->ismyparm = 0;

      /* delete it from the symbol table  */
      deleteSym (SymbolTab, lval->sym, lval->sym->name);

      if (!lval->sym->isref)
        {
          lval->sym->allocreq = 0;
            werror (W_NO_REFERENCE,
                    currFunc ? currFunc->name : "(unknown)",
                    "function argument", lval->sym->name);
        }

      /* move the rname if any to the name for both val & sym */
      /* and leave a copy of it in the symbol table           */
      if (lval->sym->rname[0])
        {
          char buffer[SDCC_NAME_MAX];
          symbol * argsym = lval->sym;

          strncpyz (buffer, lval->sym->rname, sizeof(buffer));
          lval->sym = copySymbol (lval->sym);
          strncpyz (lval->sym->rname, buffer, sizeof(lval->sym->rname));

          strncpyz (lval->sym->name, buffer, sizeof(lval->sym->name));
          /* need to keep the original name for inlining to work */
          /*strncpyz (lval->name, buffer, sizeof(lval->name)); */

          addSym (SymbolTab, lval->sym, lval->sym->name,
                  lval->sym->level, lval->sym->block, 1);
          lval->sym->_isparm = 1;
          if (!isinSet (operKeyReset, lval->sym))
            {
              addSet(&operKeyReset, lval->sym);
            }

          /* restore the original symbol */
          lval->sym = argsym;
        }
    }
  return;
}

/*-----------------------------------------------------------------*/
/* allocLocal - allocate local variables                           */
/*-----------------------------------------------------------------*/
void
allocLocal (symbol * sym)
{
  /* generate an unique name */
  SNPRINTF (sym->rname, sizeof(sym->rname),
            "%s%s_%s_%d_%d",
            port->fun_prefix,
            currFunc->name, sym->name, sym->level, sym->block);

  sym->islocal = 1;
  sym->localof = currFunc;

  /* if this is a static variable */
  if (IS_STATIC (sym->etype))
    {
      allocGlobal (sym);
      sym->allocreq = 1;
      return;
    }

  /* if volatile then */
  if (IS_VOLATILE (sym->etype))
    sym->allocreq = 1;

  /* this is automatic           */

  /* if it's to be placed on the stack */
  if (options.stackAuto || reentrant)
    {
      sym->onStack = 1;
      if (options.useXstack)
        {
          /* PENDING: stack direction for xstack */
          SPEC_OCLS (sym->etype) = xstack;
          SPEC_STAK (sym->etype) = sym->stack = (xstackPtr + 1);
          xstackPtr += getSize (sym->type);
        }
      else
        {
          SPEC_OCLS (sym->etype) = istack;
          if (port->stack.direction > 0)
            {
              SPEC_STAK (sym->etype) = sym->stack = (stackPtr + 1);
              stackPtr += getSize (sym->type);
            }
          else
            {
              stackPtr -= getSize (sym->type);
              SPEC_STAK (sym->etype) = sym->stack = stackPtr;
            }
        }
      allocIntoSeg (sym);
      return;
    }

  /* else depending on the storage class specified */

  /* if this is a function then assign code space    */
  if (IS_FUNC (sym->type))
    {
      SPEC_OCLS (sym->etype) = code;
      return;
    }

  /* if this is a bit variable and no storage class */
  if (bit && IS_SPEC(sym->type) && SPEC_NOUN (sym->type) == V_BIT)
    {
      SPEC_SCLS (sym->type) = S_BIT;
      SPEC_OCLS (sym->type) = bit;
      allocIntoSeg (sym);
      return;
    }

  if ((SPEC_SCLS (sym->etype) == S_DATA) || (SPEC_SCLS (sym->etype) == S_REGISTER))
    {
      SPEC_OCLS (sym->etype) = (options.noOverlay ? data : overlay);
      allocIntoSeg (sym);
      return;
    }

  if (allocDefault (sym))
    {
      return;
    }

  /* again note that we have put it into the overlay segment
     will remove and put into the 'data' segment if required after
     overlay  analysis has been done */
  if (options.model == MODEL_SMALL)
    {
      SPEC_OCLS (sym->etype) =
        (options.noOverlay ? port->mem.default_local_map : overlay);
    }
  else
    {
      SPEC_OCLS (sym->etype) = port->mem.default_local_map;
    }
  allocIntoSeg (sym);
}

/*-----------------------------------------------------------------*/
/* deallocLocal - deallocates the local variables                  */
/*-----------------------------------------------------------------*/
void
deallocLocal (symbol * csym)
{
  symbol *sym;

  for (sym = csym; sym; sym = sym->next)
    {
      if (sym->_isparm)
        continue;

      /* if it is on the stack */
      if (sym->onStack)
        {
          if (options.useXstack)
            xstackPtr -= getSize (sym->type);
          else
            stackPtr -= getSize (sym->type);
        }
      /* if not used give a warning */
      if (!sym->isref && !IS_STATIC (sym->etype))
        werror (W_NO_REFERENCE,
                currFunc ? currFunc->name : "(unknown)",
                "local variable", sym->name);
      /* now delete it from the symbol table */
      deleteSym (SymbolTab, sym, sym->name);
    }
}

/*-----------------------------------------------------------------*/
/* overlay2data - moves declarations from the overlay seg to data  */
/*-----------------------------------------------------------------*/
void
overlay2data ()
{
  symbol *sym;

  for (sym = setFirstItem (overlay->syms); sym;
       sym = setNextItem (overlay->syms))
    {

      SPEC_OCLS (sym->etype) = data;
      allocIntoSeg (sym);
    }

  setToNull ((void *) &overlay->syms);
}

/*-----------------------------------------------------------------*/
/* overlay2Set - will add all symbols from the overlay segment to  */
/*               the set of sets containing the overlable symbols  */
/*-----------------------------------------------------------------*/
void
overlay2Set ()
{
  symbol *sym;
  set *oset = NULL;

  for (sym = setFirstItem (overlay->syms); sym;
       sym = setNextItem (overlay->syms))
    {

      addSet (&oset, sym);
    }

  setToNull ((void *) &overlay->syms);
  addSet (&ovrSetSets, oset);
}

/*-----------------------------------------------------------------*/
/* allocVariables - creates decl & assign storage class for a v    */
/*-----------------------------------------------------------------*/
int
allocVariables (symbol * symChain)
{
  symbol *sym;
  symbol *csym;
  int stack = 0;
  int saveLevel = 0;

  /* go thru the symbol chain   */
  for (sym = symChain; sym; sym = sym->next)
    {
      /* if this is a typedef then add it */
      /* to the typedef table             */
      if (IS_TYPEDEF (sym->etype))
        {
          /* check if the typedef already exists    */
          csym = findSym (TypedefTab, NULL, sym->name);
          if (csym && csym->level == sym->level)
            werror (E_DUPLICATE_TYPEDEF, sym->name);

          SPEC_EXTR (sym->etype) = 0;
          addSym (TypedefTab, sym, sym->name, sym->level, sym->block, 0);
          continue;             /* go to the next one */
        }
      /* make sure it already exists */
      csym = findSymWithLevel (SymbolTab, sym);
      if (!csym || (csym && csym->level != sym->level))
        csym = sym;

      /* check the declaration */
      checkDecl (csym, 0);

      /* if this is a function or a pointer to a */
      /* function then do args processing        */
      if (funcInChain (csym->type))
        {
          processFuncArgs (csym);
        }

      /* if this is an extern variable then change */
      /* the level to zero temporarily             */
      if (IS_EXTERN (csym->etype) || IS_FUNC (csym->type))
        {
          saveLevel = csym->level;
          csym->level = 0;
        }

      /* if this is a literal then it is an enumerated */
      /* type so need not allocate it space for it     */
      if (IS_LITERAL (sym->etype))
        continue;

      /* generate the actual declaration */
      if (csym->level)
        {
          allocLocal (csym);
          if (csym->onStack)
            stack += getSize (csym->type);
        }
      else
        allocGlobal (csym);

      /* restore the level */
      if (IS_EXTERN (csym->etype) || IS_FUNC (csym->type))
        csym->level = saveLevel;
    }

  return stack;
}

#define BTREE_STACK 1

/*-----------------------------------------------------------------*/
/* redoStackOffsets :- will reassign the values for stack offsets  */
/*-----------------------------------------------------------------*/
void
redoStackOffsets (void)
{
  symbol *sym;
  int sPtr = 0;
  int xsPtr = -1;

  /* after register allocation is complete we know
     which variables will need to be assigned space
     on the stack. We will eliminate those variables
     which do not have the allocReq flag thus reducing
     the stack space */
  for (sym = setFirstItem (istack->syms); sym; sym = setNextItem (istack->syms))
    {
      int size = getSize (sym->type);
      /* nothing to do with parameters so continue */
      if ((sym->_isparm && !IS_REGPARM (sym->etype)))
        continue;

      if (BTREE_STACK)
        {
          /* Remove them all, and let btree_alloc() below put them back in more efficiently. */
          currFunc->stack -= size;
          SPEC_STAK (currFunc->etype) -= size;
      
          if(IS_AGGREGATE (sym->type) || sym->allocreq)
            btree_add_symbol (sym);
        }
       /* Do it the old way - compared to the btree approach we waste space when allocating
          variables that had their address taken, unions and aggregates. */
       else
        {
          /* if allocation not required then subtract
             size from overall stack size & continue */
          if (!IS_AGGREGATE (sym->type) && !sym->allocreq)
            {
              currFunc->stack -= size;
              SPEC_STAK (currFunc->etype) -= size;
              continue;
            }

          if (port->stack.direction > 0)
            {
              SPEC_STAK (sym->etype) = sym->stack = (sPtr + 1);
              sPtr += size;
            }
          else
            {
              sPtr -= size;
              SPEC_STAK (sym->etype) = sym->stack = sPtr;
            }
        }
    }

  if (BTREE_STACK && elementsInSet (istack->syms))
    {
      btree_alloc ();
      btree_clear ();
    }

  /* do the same for the external stack */

  if (!xstack)
    return;

  for (sym = setFirstItem (xstack->syms); sym; sym = setNextItem (xstack->syms))
    {
      int size = getSize (sym->type);
      /* nothing to do with parameters so continue */
      if ((sym->_isparm && !IS_REGPARM (sym->etype)))
        continue;

      if (IS_AGGREGATE (sym->type))
        {
          SPEC_STAK (sym->etype) = sym->stack = (xsPtr + 1);
          xsPtr += size;
          continue;
        }

      /* if allocation not required then subtract
         size from overall stack size & continue */
      if (!sym->allocreq)
        {
          currFunc->xstack -= size;
          SPEC_STAK (currFunc->etype) -= size;
          continue;
        }

      SPEC_STAK (sym->etype) = sym->stack = (xsPtr + 1);
      xsPtr += size;
    }
}

#define SP_BP(sp, bp) (options.omitFramePtr ? sp : bp)
#define SYM_BP(sym)   (SPEC_OCLS (sym->etype)->paged ? SP_BP("_spx", "_bpx") : SP_BP("sp", "_bp"))

/*-----------------------------------------------------------------*/
/* printAllocInfoSeg- print the allocation for a given section     */
/*-----------------------------------------------------------------*/
static int
printAllocInfoSeg (memmap * map, symbol * func, struct dbuf_s *oBuf)
{
  symbol *sym;
  int flg = FALSE;

  if (!map || !map->syms)
    return 0;

  for (sym = setFirstItem (map->syms); sym;
       sym = setNextItem (map->syms))
    {
      if (sym->level == 0)
        continue;
      if (sym->localof != func)
        continue;

      dbuf_printf (oBuf, ";%-25s Allocated ", sym->name);
      flg = TRUE;

      /* if assigned to registers */
      if (!sym->allocreq && sym->reqv)
        {
          int i;

          sym = OP_SYMBOL (sym->reqv);
          if (!sym->isspilt || sym->remat)
            {
              dbuf_append_str (oBuf, "to registers ");
              for (i = 0; i < 4 && sym->regs[i]; i++)
                dbuf_printf (oBuf, "%s ", port->getRegName (sym->regs[i]));
              dbuf_append_char (oBuf, '\n');
              continue;
            }
          else
            {
              sym = sym->usl.spillLoc;
            }
        }

      /* if on stack */
      if (sym->onStack)
        {
          int stack_offset = 0;

          if (options.omitFramePtr)
            {
              if (SPEC_OCLS (sym->etype)->paged)
                stack_offset = func->xstack;
              else
                stack_offset = func->stack;
            }

          stack_offset += port->stack.offset; /* in case sp/bp points to the next location instead of last */

          if (port->stack.direction < 0)
            stack_offset = -stack_offset;

          dbuf_printf (oBuf, "to stack - %s %+d\n", SYM_BP (sym), sym->stack - stack_offset);
          continue;
        }

      /* otherwise give rname */
      dbuf_printf (oBuf, "with name '%s'\n", sym->rname);
    }

  return flg;
}

/*-----------------------------------------------------------------*/
/* canOverlayLocals - returns true if the local variables can overlayed */
/*-----------------------------------------------------------------*/
static bool
canOverlayLocals (eBBlock ** ebbs, int count)
{
  int i;
  /* if staticAuto is in effect or the current function
     being compiled is reentrant or the overlay segment
     is empty or no overlay option is in effect then */
  if (options.noOverlay ||
      options.stackAuto ||
      (currFunc &&
       (IFFUNC_ISREENT (currFunc->type) ||
        FUNC_ISISR (currFunc->type))) ||
      elementsInSet (overlay->syms) == 0)
    {
      return FALSE;
    }
  /* if this is a forces overlay */
  if (IFFUNC_ISOVERLAY(currFunc->type)) return TRUE;

  /* otherwise do thru the blocks and see if there
     any function calls if found then return false */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      for (ic = ebbs[i]->sch; ic; ic = ic->next)
          if (ic)
            {
              if (ic->op == CALL)
                {
                  sym_link *ftype = operandType(IC_LEFT(ic));
                  /* builtins only can use overlays */
                  if (!IFFUNC_ISBUILTIN(ftype)) return FALSE;
                }
              else if (ic->op == PCALL)
                {
                  return FALSE;
                }
          }
    }

  /* no function calls found return TRUE */
  return TRUE;
}

/*-----------------------------------------------------------------*/
/* doOverlays - move the overlay segment to appropriate location   */
/*-----------------------------------------------------------------*/
void
doOverlays (eBBlock ** ebbs, int count)
{
  if (!overlay)
    return;

  /* check if the parameters and local variables
     of this function can be put in the overlay segment
     This check is essentially to see if the function
     calls any other functions if yes then we cannot
     overlay */
  if (canOverlayLocals (ebbs, count))
    /* if we can then put the parameters &
       local variables in the overlay set */
    overlay2Set ();
  else
    /* otherwise put them into data where
       they belong */
    overlay2data ();
}

/*-----------------------------------------------------------------*/
/* printAllocInfo - prints allocation information for a function   */
/*-----------------------------------------------------------------*/
void
printAllocInfo (symbol * func, struct dbuf_s * oBuf)
{
#define BREAKLINE ";------------------------------------------------------------\n"
  int cnt = 0;
  set *ovrset;
  set *tempOverlaySyms;

  if (!func)
    return;

  /* must be called after register allocation is complete */
  dbuf_append_str (oBuf, BREAKLINE);
  dbuf_printf (oBuf, ";Allocation info for local variables in function '%s'\n", func->name);
  dbuf_append_str (oBuf, BREAKLINE);

  cnt += printAllocInfoSeg (xstack, func, oBuf);
  cnt += printAllocInfoSeg (istack, func, oBuf);
  cnt += printAllocInfoSeg (code, func, oBuf);
  cnt += printAllocInfoSeg (data, func, oBuf);
  cnt += printAllocInfoSeg (xdata, func, oBuf);
  cnt += printAllocInfoSeg (idata, func, oBuf);
  cnt += printAllocInfoSeg (sfr, func, oBuf);
  cnt += printAllocInfoSeg (sfrbit, func, oBuf);

  tempOverlaySyms = overlay->syms;

  /* search the set of overlay sets for local variables/parameters */
  for (ovrset = setFirstItem (ovrSetSets); ovrset;
       ovrset = setNextItem (ovrSetSets))
    {
      overlay->syms = ovrset;
      cnt += printAllocInfoSeg (overlay, func, oBuf);
    }
  overlay->syms = tempOverlaySyms;

  if (cnt)
    dbuf_append_str (oBuf, BREAKLINE);
}
