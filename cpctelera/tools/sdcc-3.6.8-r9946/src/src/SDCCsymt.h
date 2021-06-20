/*-------------------------------------------------------------------------
  SDCCsymt.h - Header file for Symbols table related structures and MACRO's.

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

#ifndef  SDCCSYMT_H
#define  SDCCSYMT_H

#define MAX_NEST_LEVEL  256
#define SDCC_SYMNAME_MAX 64
#define SDCC_NAME_MAX  3*SDCC_SYMNAME_MAX       // big enough for _<func>_<var>_etc
#include "SDCChasht.h"
#include "SDCCglobl.h"
#include "dbuf.h"

#define INTNO_MAX 255           /* maximum allowed interrupt number */
#define INTNO_TRAP INTNO_MAX
#define INTNO_UNSPEC (INTNO_MAX+1)      /* interrupt number unspecified */


#define BITVAR_PAD -1

enum
{
  TYPEOF_INT = 1,
  TYPEOF_SHORT,
  TYPEOF_BOOL,
  TYPEOF_CHAR,
  TYPEOF_LONG,
  TYPEOF_LONGLONG,
  TYPEOF_FLOAT,
  TYPEOF_FIXED16X16,
  TYPEOF_BIT,
  TYPEOF_BITFIELD,
  TYPEOF_SBIT,
  TYPEOF_SFR,
  TYPEOF_VOID,
  TYPEOF_STRUCT,
  TYPEOF_ARRAY,
  TYPEOF_FUNCTION,
  TYPEOF_POINTER,
  TYPEOF_FPOINTER,
  TYPEOF_CPOINTER,
  TYPEOF_GPOINTER,
  TYPEOF_PPOINTER,
  TYPEOF_IPOINTER,
  TYPEOF_EEPPOINTER
};

// values for first byte (or 3 most significant bits) of generic pointer.
#if 0
#define GPTYPE_FAR       0x00
#define GPTYPE_NEAR      0x40
#define GPTYPE_XSTACK    0x60
#define GPTYPE_CODE      0x80
#else
#define GPTYPE_FAR      (port->gp_tags.tag_far)
#define GPTYPE_NEAR     (port->gp_tags.tag_near)
#define GPTYPE_XSTACK   (port->gp_tags.tag_xstack)
#define GPTYPE_CODE     (port->gp_tags.tag_code)
#endif

#define HASHTAB_SIZE 256

/* hash table bucket */
typedef struct bucket
{
  void *sym;                    /* pointer to the object      */
  char name[SDCC_NAME_MAX + 1]; /* name of this symbol        */
  int level;                    /* nest level for this symbol */
  int block;                    /* belongs to which block     */
  struct bucket *prev;          /* ptr 2 previous bucket      */
  struct bucket *next;          /* ptr 2 next bucket          */
}
bucket;

typedef struct structdef
{
  char tag[SDCC_NAME_MAX + 1];  /* tag part of structure      */
  unsigned char level;          /* Nesting level              */
  int block;                    /* belongs to which block     */
  struct symbol *fields;        /* pointer to fields          */
  unsigned size;                /* sizeof the table in bytes  */
  int type;                     /* STRUCT or UNION            */
  bool b_flexArrayMember;       /* has got a flexible array member,
                                   only needed for syntax checks */
  struct symbol *tagsym;        /* tag symbol (NULL if no tag) */
}
structdef;

/* noun definitions */
typedef enum
{
  V_INT = 1,
  V_FLOAT,
  V_FIXED16X16,
  V_BOOL,
  V_CHAR,
  V_VOID,
  V_STRUCT,
  V_LABEL,
  V_BIT,
  V_BITFIELD,
  V_BBITFIELD,
  V_SBIT,
  V_DOUBLE
}
NOUN;

/* storage class    */
typedef enum
{
  S_FIXED = 0,
  S_AUTO,
  S_REGISTER,
  S_SFR,
  S_SBIT,
  S_CODE,
  S_XDATA,
  S_DATA,
  S_IDATA,
  S_PDATA,
  S_LITERAL,
  S_STACK,
  S_XSTACK,
  S_BIT,
  S_EEPROM
}
STORAGE_CLASS;

#define TYPE_TARGET_CHAR  TYPE_BYTE
#define TYPE_TARGET_INT   TYPE_WORD
#define TYPE_TARGET_LONG  TYPE_DWORD
#define TYPE_TARGET_UCHAR TYPE_UBYTE
#define TYPE_TARGET_UINT  TYPE_UWORD
#define TYPE_TARGET_ULONG TYPE_UDWORD
#define TYPE_TARGET_LONGLONG TYPE_QWORD
#define TYPE_TARGET_ULONGLONG TYPE_UQWORD

/* specifier is the last in the type-chain */
typedef struct specifier
{
  NOUN noun;                        /* CHAR INT STRUCTURE LABEL   */
  STORAGE_CLASS sclass;             /* REGISTER,AUTO,FIX,CONSTANT */
  struct memmap *oclass;            /* output storage class       */
  unsigned b_long:1;                /* 1=long                     */
  unsigned b_longlong:1;            /* 1=long long                */
  unsigned b_short:1;               /* 1=short int                */
  unsigned b_unsigned:1;            /* 1=unsigned, 0=signed       */
  unsigned b_signed:1;              /* just for sanity checks only*/
  unsigned b_static:1;              /* 1=static keyword found     */
  unsigned b_extern:1;              /* 1=extern found             */
  unsigned b_inline:1;              /* inline function requested  */
  unsigned b_noreturn:1;            /* promised not to return     */
  unsigned b_alignas:1;             /* alignment                  */
  unsigned b_absadr:1;              /* absolute address specfied  */
  unsigned b_volatile:1;            /* is marked as volatile      */
  unsigned b_const:1;               /* is a constant              */
  unsigned b_restrict:1;            /* is restricted              */
  struct symbol *addrspace;         /* is in named address space  */
  unsigned b_typedef:1;             /* is typedefed               */
  unsigned b_isregparm:1;           /* is the first parameter     */
  unsigned b_isenum:1;              /* is an enumerated type      */
  unsigned b_bitUnnamed:1;          /* is an unnamed bit-field    */
  unsigned _bitStart;               /* bit start position         */
  unsigned _bitLength;              /* bit length                 */
  unsigned _addr;                   /* address of symbol          */
  unsigned _stack;                  /* stack offset for stacked v */
  int argreg;                       /* reg no for regparm         */
  union
  {                                   /* Values if constant or enum */
    TYPE_TARGET_INT v_int;            /* 2 bytes: int and char values            */
    const char *v_char;               /*          char character string          */
    const TYPE_TARGET_UINT *v_char16; /*          char16_t character string      */
    const TYPE_TARGET_ULONG *v_char32;/*          char32_t character string      */
    TYPE_TARGET_UINT v_uint;          /* 2 bytes: unsigned int const value       */
    TYPE_TARGET_LONG v_long;          /* 4 bytes: long constant value            */
    TYPE_TARGET_ULONG v_ulong;        /* 4 bytes: unsigned long constant value   */
    TYPE_TARGET_LONGLONG v_longlong;  /* 8 bytes: long long constant value       */
    TYPE_TARGET_ULONGLONG v_ulonglong;/* 8 bytes: unsigned long long const value */
    double v_float;                   /*          floating point constant value  */
    TYPE_TARGET_ULONG v_fixed16x16;   /* 4 bytes: fixed point constant value     */
    struct symbol *v_enum;            /* ptr to enum_list if enum==1             */
  }
  const_val;
  struct structdef *v_struct;       /* structure pointer      */
}
specifier;

/* types of declarators */
typedef enum
{
  UPOINTER = 0,                     /* unknown pointer used only when parsing */
  POINTER,                          /* pointer to near data  */
  IPOINTER,                         /* pointer to upper 128 bytes */
  PPOINTER,                         /* paged area pointer    */
  FPOINTER,                         /* pointer to far data   */
  CPOINTER,                         /* pointer to code space */
  GPOINTER,                         /* generic pointer       */
  EEPPOINTER,                       /* pointer to eeprom     */
  ARRAY,
  FUNCTION
}
DECLARATOR_TYPE;

typedef struct declarator
{
  DECLARATOR_TYPE dcl_type;         /* POINTER,ARRAY or FUNCTION  */
  size_t num_elem;                  /* # of elems if type==array, */
  /* always 0 for flexible arrays */
  unsigned ptr_const:1;             /* pointer is constant        */
  unsigned ptr_volatile:1;          /* pointer is volatile        */
  unsigned ptr_restrict:1;          /* pointer is resticted       */
  struct symbol *ptr_addrspace;     /* pointer is in named address space  */

  struct sym_link *tspec;           /* pointer type specifier     */
}
declarator;

typedef enum
{
  DECLARATOR = 1,
  SPECIFIER
} SYM_LINK_CLASS;
#define DECLSPEC2TXT(select) (select==DECLARATOR?"DECLARATOR":select==SPECIFIER?"SPECIFIER":"UNKNOWN")

typedef struct sym_link
{
  SYM_LINK_CLASS xclass;            /* DECLARATOR or SPECIFIER     */
  unsigned tdef:1;                  /* current link created by     */
                                    /* typedef if this flag is set */
  union
  {
    specifier s;                    /* if CLASS == SPECIFIER      */
    declarator d;                   /* if CLASS == DECLARATOR     */
  } select;

  /* function attributes */
  struct
  {
    struct value *args;             /* the defined arguments                */
    unsigned hasVargs:1;            /* functions has varargs                */
    unsigned calleeSaves:1;         /* functions uses callee save           */
    unsigned hasbody:1;             /* function body defined                */
    unsigned hasFcall:1;            /* does it call other functions         */
    unsigned reent:1;               /* function is reentrant                */
    unsigned naked:1;               /* naked function                       */

    unsigned shadowregs:1;          /* function uses shadow registers (pic16 port) */
    unsigned wparam:1;              /* first byte of arguments is passed via WREG (pic16 port) */
    unsigned nonbanked:1;           /* function has the nonbanked attribute */
    unsigned banked:1;              /* function has the banked attribute    */
    unsigned critical:1;            /* critical function                    */
    unsigned intrtn:1;              /* this is an interrupt routine         */
    unsigned rbank:1;               /* seperate register bank               */
    unsigned inlinereq:1;           /* inlining requested                   */
    unsigned noreturn:1;            /* promised not to return               */
    unsigned smallc:1;              /* Parameters on stack are passed in reverse order */
    unsigned z88dk_fastcall:1;      /* For the z80-related ports: Function has a single paramter of at most 32 bits that is passed in dehl */
    unsigned z88dk_callee:1;        /* Stack pointer adjustment for parameters passed on the stack is done by the callee */
    unsigned intno;                 /* 1=Interrupt service routine          */
    short regbank;                  /* register bank 2b used                */
    unsigned builtin;               /* is a builtin function                */
    unsigned javaNative;            /* is a JavaNative Function (TININative ONLY) */
    unsigned overlay;               /* force parameters & locals into overlay segment */
    unsigned hasStackParms;         /* function has parameters on stack     */
    bool preserved_regs[9];         /* Registers preserved by the function - may be an underestimate */
  } funcAttrs;

  struct sym_link *next;            /* next element on the chain  */
}
sym_link;

typedef struct symbol
{
  char name[SDCC_SYMNAME_MAX + 1];  /* Input Variable Name     */
  char rname[SDCC_NAME_MAX + 1];    /* internal name           */

  short level;                      /* declaration lev,fld offset */
  short block;                      /* sequential block # of definition */
  int seqPoint;                     /* sequence point defined or, if unbound, used */
  int key;
  unsigned flexArrayLength;         /* if the symbol specifies a struct
                                       with a "flexible array member", then the additional length in bytes for
                                       the "fam" is stored here. Because the length can be different from symbol
                                       to symbol AND v_struct isn't copied in copyLinkChain(), it's located here
                                       in the symbol and not in v_struct or the declarator */
  unsigned implicit:1;              /* implicit flag                     */
  unsigned undefined:1;             /* undefined variable                */
  unsigned infertype:1;             /* type should be inferred from first assign */
  unsigned _isparm:1;               /* is a parameter          */
  unsigned ismyparm:1;              /* is parameter of the function being generated */
  unsigned isitmp:1;                /* is an intermediate temp */
  unsigned islbl:1;                 /* is a temporary label */
  unsigned isref:1;                 /* has been referenced  */
  unsigned isind:1;                 /* is an induction variable */
  unsigned isinvariant:1;           /* is a loop invariant  */
  unsigned cdef:1;                  /* compiler defined symbol */
  unsigned addrtaken:1;             /* address of the symbol was taken */
  unsigned isreqv:1;                /* is the register equivalent of a symbol */
  unsigned udChked:1;               /* use def checking has been already done */
  unsigned generated:1;             /* code generated (function symbols only) */
  unsigned isinscope:1;             /* is in scope */

  /* following flags are used by the backend
     for code generation and can be changed
     if a better scheme for backend is thought of */
  unsigned isLiveFcall:1;           /* is live at or across a function call */
  unsigned isspilt:1;               /* has to be spilt */
  unsigned spillA:1;                /* spilt be register allocator */
  unsigned remat:1;                 /* can be remateriazed */
  unsigned isptr:1;                 /* is a pointer */
  unsigned uptr:1;                  /* used as a pointer */
  unsigned isFree:1;                /* used by register allocator */
  unsigned islocal:1;               /* is a local variable        */
  unsigned blockSpil:1;             /* spilt at block level       */
  unsigned remainSpil:1;            /* spilt because not used in remainder */
  unsigned stackSpil:1;             /* has been spilt on temp stack location */
  unsigned onStack:1;               /* this symbol allocated on the stack */
  unsigned iaccess:1;               /* indirect access      */
  unsigned ruonly:1;                /* used in return statement only */
  unsigned spildir:1;               /* spilt in direct space */
  unsigned ptrreg:1;                /* this symbol assigned to a ptr reg */
  unsigned noSpilLoc:1;             /* cannot be assigned a spil location */
  unsigned div_flag_safe:1;         /* we know this function is safe to call with undocumented stm8 flag bit 6 set*/
  unsigned isstrlit;                /* is a string literal and it's usage count  */
  unsigned accuse;                  /* can be left in the accumulator
                                       On the Z80 accuse is divided into
                                       ACCUSE_A and ACCUSE_HL as the idea
                                       is quite similar.
                                     */
  unsigned dptr;                    /* 8051 variants with multiple DPTRS
                                       currently implemented in DS390 only
                                     */
  int allocreq;                     /* allocation is required for this variable */
  int stack;                        /* offset on stack      */
  int xstack;                       /* offset on xternal stack */
  short nRegs;                      /* number of registers required */
  short regType;                    /* type of register required    */

  struct reg_info *regs[8];         /* can have at the most 8 registers */
  struct asmop *aop;                /* asmoperand for this symbol */
  struct iCode *fuse;               /* furthest use */
  struct iCode *rematiCode;         /* rematerialise with which instruction */
  struct operand *reqv;             /* register equivalent of a local variable */
  struct symbol *prereqv;           /* symbol before register equiv. substitution */
  struct symbol *psbase;            /* if pseudo symbol, the symbol it is based on */
  union
  {
    struct symbol *spillLoc;        /* register spil location */
    struct set *itmpStack;          /* symbols spilt @ this stack location */
  }
  usl;
  signed char bitVar;               /* if bitVar != 0: this is a bit variable, bitVar is the size in bits */
  char bitUnnamed:1;                /* unnamed bit variable */
  unsigned offset;                  /* offset from top if struct */

  int lineDef;                      /* defined line number        */
  char *fileDef;                    /* defined filename           */
  int lastLine;                     /* for functions the last line */
  struct sym_link *type;            /* 1st link to declarator chain */
  struct sym_link *etype;           /* last link to declarator chain */
  struct symbol *next;              /* crosslink to next symbol   */
  struct symbol *localof;           /* local variable of which function */
  struct initList *ival;            /* ptr to initializer if any  */
  struct bitVect *defs;             /* bit vector for definitions */
  struct bitVect *uses;             /* bit vector for uses        */
  struct bitVect *regsUsed;         /* for functions registers used */
  int liveFrom;                     /* live from iCode sequence number */
  int liveTo;                       /* live to sequence number */
  int used;                         /* no. of times this was used */
  int recvSize;                     /* size of first argument  */
  struct bitVect *clashes;          /* overlaps with what other symbols */
  struct ast *funcTree;             /* function body ast if inlined */
  struct symbol *addressmod[2];     /* access functions for named address spaces */

  bool for_newralloc;
}
symbol;

extern sym_link *validateLink (sym_link * l,
                               const char *macro, const char *args, const char select, const char *file, unsigned line);
/* Easy Access Macros */
#define IS_OP_RUONLY(x) (IS_SYMOP(x) && OP_SYMBOL(x) && OP_SYMBOL(x)->ruonly)
#define IS_OP_ACCUSE(x) (IS_SYMOP(x) && OP_SYMBOL(x) && OP_SYMBOL(x)->accuse)

#define DCL_TYPE(l)  validateLink(l, "DCL_TYPE", #l, DECLARATOR, __FILE__, __LINE__)->select.d.dcl_type
#define DCL_ELEM(l)  validateLink(l, "DCL_ELEM", #l, DECLARATOR, __FILE__, __LINE__)->select.d.num_elem
#define DCL_PTR_CONST(l) validateLink(l, "DCL_PTR_CONST", #l, DECLARATOR, __FILE__, __LINE__)->select.d.ptr_const
#define DCL_PTR_VOLATILE(l) validateLink(l, "DCL_PTR_VOLATILE", #l, DECLARATOR, __FILE__, __LINE__)->select.d.ptr_volatile
#define DCL_PTR_RESTRICT(l) validateLink(l, "DCL_PTR_RESTRICT", #l, DECLARATOR, __FILE__, __LINE__)->select.d.ptr_restrict
#define DCL_PTR_ADDRSPACE(l) validateLink(l, "DCL_PTR_ADDRSPACE", #l, DECLARATOR, __FILE__, __LINE__)->select.d.ptr_addrspace
#define DCL_TSPEC(l) validateLink(l, "DCL_TSPEC", #l, DECLARATOR, __FILE__, __LINE__)->select.d.tspec

#define FUNC_DEBUG              //assert(IS_FUNC(x));
#define FUNC_HASVARARGS(x) (x->funcAttrs.hasVargs)
#define IFFUNC_HASVARARGS(x) (IS_FUNC(x) && FUNC_HASVARARGS(x))
#define FUNC_ARGS(x) (x->funcAttrs.args)
#define IFFUNC_ARGS(x) (IS_FUNC(x) && FUNC_ARGS(x))
#define FUNC_HASFCALL(x) (x->funcAttrs.hasFcall)
#define IFFUNC_HASFCALL(x) (IS_FUNC(x) && FUNC_HASFCALL(x))
#define FUNC_HASBODY(x) (x->funcAttrs.hasbody)
#define IFFUNC_HASBODY(x) (IS_FUNC(x) && FUNC_HASBODY(x))
#define FUNC_CALLEESAVES(x) (x->funcAttrs.calleeSaves)
#define IFFUNC_CALLEESAVES(x) (IS_FUNC(x) && FUNC_CALLEESAVES(x))
#define FUNC_ISISR(x) (x->funcAttrs.intrtn)
#define IFFUNC_ISISR(x) (IS_FUNC(x) && FUNC_ISISR(x))
#define FUNC_INTNO(x) (x->funcAttrs.intno)
#define FUNC_REGBANK(x) (x->funcAttrs.regbank)
#define FUNC_HASSTACKPARM(x) (x->funcAttrs.hasStackParms)
#define FUNC_ISINLINE(x) (x->funcAttrs.inlinereq)
#define IFFUNC_ISINLINE(x) (IS_FUNC(x) && FUNC_ISINLINE(x))
#define FUNC_ISNORETURN(x) (x->funcAttrs.noreturn)
#define IFFUNC_ISNORETURN(x) (IS_FUNC(x) && FUNC_ISNORETURN(x))

#define FUNC_ISREENT(x) (x->funcAttrs.reent)
#define IFFUNC_ISREENT(x) (IS_FUNC(x) && FUNC_ISREENT(x))
#define FUNC_ISSHADOWREGS(x) (x->funcAttrs.shadowregs)
#define IFFUNC_ISSHADOWREGS(x) (IS_FUNC(x) && FUNC_ISSHADOWREGS(x))
#define FUNC_ISWPARAM(x) (x->funcAttrs.wparam)
#define IFFUNC_ISWPARAM(x) (IS_FUNC(x) && FUNC_ISWPARAM(x))
#define FUNC_ISNAKED(x) (x->funcAttrs.naked)
#define IFFUNC_ISNAKED(x) (IS_FUNC(x) && FUNC_ISNAKED(x))
#define FUNC_NONBANKED(x) (x->funcAttrs.nonbanked)
#define IFFUNC_NONBANKED(x) (IS_FUNC(x) && FUNC_NONBANKED(x))
#define FUNC_BANKED(x) (x->funcAttrs.banked)
#define IFFUNC_BANKED(x) (IS_FUNC(x) && FUNC_BANKED(x))
#define FUNC_ISCRITICAL(x) (x->funcAttrs.critical)
#define IFFUNC_ISCRITICAL(x) (IS_FUNC(x) && FUNC_ISCRITICAL(x))
#define FUNC_ISBUILTIN(x) (x->funcAttrs.builtin)
#define IFFUNC_ISBUILTIN(x) (IS_FUNC(x) && FUNC_ISBUILTIN(x))
#define FUNC_ISJAVANATIVE(x) (x->funcAttrs.javaNative)
#define IFFUNC_ISJAVANATIVE(x) (IS_FUNC(x) && FUNC_ISJAVANATIVE(x))
#define FUNC_ISOVERLAY(x) (x->funcAttrs.overlay)
#define IFFUNC_ISOVERLAY(x) (IS_FUNC(x) && FUNC_ISOVERLAY(x))
#define FUNC_ISSMALLC(x) (x->funcAttrs.smallc)
#define IFFUNC_ISSMALLC(x) (IS_FUNC(x) && FUNC_ISSMALLC(x))
#define FUNC_ISZ88DK_FASTCALL(x) (x->funcAttrs.z88dk_fastcall)
#define IFFUNC_ISZ88DK_FASTCALL(x) (IS_FUNC(x) && FUNC_ISZ88DK_FASTCALL(x))
#define FUNC_ISZ88DK_CALLEE(x) (x->funcAttrs.z88dk_callee)
#define IFFUNC_ISZ88DK_CALLEE(x) (IS_FUNC(x) && FUNC_ISZ88DK_CALLEE(x))

#define BANKED_FUNCTIONS        ( options.model == MODEL_HUGE || \
                                  ( (options.model == MODEL_LARGE || options.model == MODEL_MEDIUM) && \
                                    TARGET_Z80_LIKE ) )
#define IFFUNC_ISBANKEDCALL(x)  ( IS_FUNC(x) && \
                                  ( FUNC_BANKED(x) || ( BANKED_FUNCTIONS && !FUNC_NONBANKED(x) ) ) )

#define SPEC_NOUN(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.noun
#define SPEC_LONG(x) validateLink(x, "SPEC_LONG", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_long
#define SPEC_LONGLONG(x) validateLink(x, "SPEC_LONGLONG", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_longlong
#define SPEC_SHORT(x) validateLink(x, "SPEC_LONG", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_short
#define SPEC_USIGN(x) validateLink(x, "SPEC_USIGN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_unsigned
#define SPEC_SIGN(x) validateLink(x, "SPEC_USIGN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_signed
#define SPEC_SCLS(x) validateLink(x, "SPEC_SCLS", #x, SPECIFIER, __FILE__, __LINE__)->select.s.sclass
#define SPEC_ENUM(x) validateLink(x, "SPEC_ENUM", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_isenum
#define SPEC_OCLS(x) validateLink(x, "SPEC_OCLS", #x, SPECIFIER, __FILE__, __LINE__)->select.s.oclass
#define SPEC_STAT(x) validateLink(x, "SPEC_STAT", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_static
#define SPEC_EXTR(x) validateLink(x, "SPEC_EXTR", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_extern
#define SPEC_CODE(x) validateLink(x, "SPEC_CODE", #x, SPECIFIER, __FILE__, __LINE__)->select.s._codesg
#define SPEC_ABSA(x) validateLink(x, "SPEC_ABSA", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_absadr
#define SPEC_BANK(x) validateLink(x, "SPEC_BANK", #x, SPECIFIER, __FILE__, __LINE__)->select.s._regbank
#define SPEC_ADDR(x) validateLink(x, "SPEC_ADDR", #x, SPECIFIER, __FILE__, __LINE__)->select.s._addr
#define SPEC_STAK(x) validateLink(x, "SPEC_STAK", #x, SPECIFIER, __FILE__, __LINE__)->select.s._stack
#define SPEC_CVAL(x) validateLink(x, "SPEC_CVAL", #x, SPECIFIER, __FILE__, __LINE__)->select.s.const_val
#define SPEC_BSTR(x) validateLink(x, "SPEC_BSTR", #x, SPECIFIER, __FILE__, __LINE__)->select.s._bitStart
#define SPEC_BLEN(x) validateLink(x, "SPEC_BLEN", #x, SPECIFIER, __FILE__, __LINE__)->select.s._bitLength
#define SPEC_BUNNAMED(x) validateLink(x, "SPEC_BLEN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_bitUnnamed

/* Sleaze: SPEC_ISR_SAVED_BANKS is only used on
 * function type symbols, which obviously cannot
 * be of BIT type. Therefore, we recycle the
 * _bitStart field instead of defining a new field.
 */
#define SPEC_ISR_SAVED_BANKS(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s._bitStart
#define SPEC_VOLATILE(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_volatile
#define SPEC_CONST(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_const
#define SPEC_RESTRICT(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_restrict
#define SPEC_ADDRSPACE(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.addrspace
#define SPEC_STRUCT(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.v_struct
#define SPEC_TYPEDEF(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_typedef
#define SPEC_REGPARM(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_isregparm
#define SPEC_ARGREG(x) validateLink(x, "SPEC_NOUN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.argreg
#define SPEC_INLINE(x) validateLink(x, "SPEC_INLINE", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_inline
#define SPEC_NORETURN(x) validateLink(x, "SPEC_NORETURN", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_noreturn
#define SPEC_ALIGNAS(x) validateLink(x, "SPEC_ALIGNAS", #x, SPECIFIER, __FILE__, __LINE__)->select.s.b_alignas

/* type check macros */
#define IS_DECL(x)       ( x && x->xclass == DECLARATOR )
#define IS_SPEC(x)       ( x && x->xclass == SPECIFIER  )

#define IS_ARRAY(x)      (IS_DECL(x) && DCL_TYPE(x) == ARRAY)
#define IS_DATA_PTR(x)   (IS_DECL(x) && DCL_TYPE(x) == POINTER)
#define IS_SMALL_PTR(x)  (IS_DECL(x) && (DCL_TYPE(x) == POINTER    ||    \
                                         DCL_TYPE(x) == IPOINTER   ||    \
                                         DCL_TYPE(x) == PPOINTER  ))
#define IS_PTR(x)        (IS_DECL(x) && (DCL_TYPE(x) == POINTER    ||    \
                                         DCL_TYPE(x) == FPOINTER   ||    \
                                         DCL_TYPE(x) == GPOINTER   ||    \
                                         DCL_TYPE(x) == IPOINTER   ||    \
                                         DCL_TYPE(x) == PPOINTER   ||    \
                                         DCL_TYPE(x) == EEPPOINTER ||    \
                                         DCL_TYPE(x) == CPOINTER   ||    \
                                         DCL_TYPE(x) == UPOINTER  ))
#define IS_PTR_CONST(x)  (IS_PTR(x) && DCL_PTR_CONST(x))
#define IS_PTR_RESTRICT(x) (IS_PTR(x) && DCL_PTR_RESTRICT(x))
#define IS_FARPTR(x)     (IS_DECL(x) && DCL_TYPE(x) == FPOINTER)
#define IS_CODEPTR(x)    (IS_DECL(x) && DCL_TYPE(x) == CPOINTER)
#define IS_GENPTR(x)     (IS_DECL(x) && DCL_TYPE(x) == GPOINTER)
#define IS_FUNCPTR(x)    (IS_DECL(x) && (DCL_TYPE(x) == CPOINTER || DCL_TYPE(x) == GPOINTER) && IS_FUNC(x->next))
#define IS_FUNC(x)       (IS_DECL(x) && DCL_TYPE(x) == FUNCTION)
#define IS_LONG(x)       (IS_SPEC(x) && x->select.s.b_long)
#define IS_LONGLONG(x)   (IS_SPEC(x) && x->select.s.b_longlong)
#define IS_UNSIGNED(x)   (IS_SPEC(x) && x->select.s.b_unsigned)
#define IS_TYPEDEF(x)    (IS_SPEC(x) && x->select.s.b_typedef)
#define IS_CONSTANT(x)   (isConstant (x))
#define IS_RESTRICT(x)   (isRestrict (x))
#define IS_STRUCT(x)     (IS_SPEC(x) && x->select.s.noun == V_STRUCT)
#define IS_ABSOLUTE(x)   (IS_SPEC(x) && x->select.s.b_absadr )
#define IS_REGISTER(x)   (IS_SPEC(x) && SPEC_SCLS(x) == S_REGISTER)
#define IS_RENT(x)       (IS_SPEC(x) && x->select.s._reent )
#define IS_STATIC(x)     (IS_SPEC(x) && SPEC_STAT(x))
#define IS_INLINE(x)     (IS_SPEC(x) && SPEC_INLINE(x))
#define IS_NORETURN(x)   (IS_SPEC(x) && SPEC_NORETURN(x))
#define IS_INT(x)        (IS_SPEC(x) && x->select.s.noun == V_INT)
#define IS_VOID(x)       (IS_SPEC(x) && x->select.s.noun == V_VOID)
#define IS_BOOL(x)       (IS_SPEC(x) && x->select.s.noun == V_BOOL)
#define IS_CHAR(x)       (IS_SPEC(x) && x->select.s.noun == V_CHAR)
#define IS_EXTERN(x)     (IS_SPEC(x) && x->select.s.b_extern)
#define IS_VOLATILE(x)   (isVolatile (x))
#define IS_INTEGRAL(x)   (IS_SPEC(x) && (x->select.s.noun == V_INT       || \
                                         x->select.s.noun == V_BOOL      || \
                                         x->select.s.noun == V_CHAR      || \
                                         x->select.s.noun == V_BITFIELD  || \
                                         x->select.s.noun == V_BBITFIELD || \
                                         x->select.s.noun == V_BIT       || \
                                         x->select.s.noun == V_SBIT ))
#define IS_BITFIELD(x)   (IS_SPEC(x) && (x->select.s.noun == V_BITFIELD  || \
                                         x->select.s.noun == V_BBITFIELD ))
#define IS_BITVAR(x)     (IS_SPEC(x) && (x->select.s.noun == V_BITFIELD  || \
                                         x->select.s.noun == V_BBITFIELD || \
                                         x->select.s.noun == V_BIT       || \
                                         x->select.s.noun == V_SBIT ))
#define IS_BIT(x)        (IS_SPEC(x) && (x->select.s.noun == V_BIT       || \
                                         x->select.s.noun == V_SBIT ))
#define IS_BOOLEAN(x)    (IS_SPEC(x) && (x->select.s.noun == V_BIT       || \
                                         x->select.s.noun == V_SBIT      || \
                                         x->select.s.noun == V_BBITFIELD || \
                                         x->select.s.noun == V_BOOL ))
#define IS_FLOAT(x)      (IS_SPEC(x) && x->select.s.noun == V_FLOAT)
#define IS_FIXED16X16(x) (IS_SPEC(x) && x->select.s.noun == V_FIXED16X16)
#define IS_FIXED(x)      (IS_FIXED16X16(x))
#define IS_ARITHMETIC(x) (IS_INTEGRAL(x) || IS_FLOAT(x) || IS_FIXED(x))
#define IS_AGGREGATE(x)  (IS_ARRAY(x) || IS_STRUCT(x))
#define IS_LITERAL(x)    (IS_SPEC(x) && x->select.s.sclass == S_LITERAL)
#define IS_CODE(x)       (IS_SPEC(x) && SPEC_SCLS(x) == S_CODE)
#define IS_REGPARM(x)    (IS_SPEC(x) && SPEC_REGPARM(x))

#define IS_VALID_PARAMETER_STORAGE_CLASS_SPEC(x)    (!SPEC_TYPEDEF(x) && !SPEC_EXTR(x) && !SPEC_STAT(x) && SPEC_SCLS(x) != S_AUTO)

/* symbol check macros */
#define IS_AUTO(x)       (x->level && !IS_STATIC(x->etype) && !IS_EXTERN(x->etype))

/* forward declaration for the global vars */
extern bucket *SymbolTab[];
extern bucket *StructTab[];
extern bucket *TypedefTab[];
extern bucket *LabelTab[];
extern bucket *enumTab[];
extern bucket *AddrspaceTab[];
extern symbol *fsadd;
extern symbol *fssub;
extern symbol *fsmul;
extern symbol *fsdiv;
extern symbol *fseq;
extern symbol *fsneq;
extern symbol *fslt;
extern symbol *fslteq;
extern symbol *fsgt;
extern symbol *fsgteq;

extern symbol *fps16x16_add;
extern symbol *fps16x16_sub;
extern symbol *fps16x16_mul;
extern symbol *fps16x16_div;
extern symbol *fps16x16_eq;
extern symbol *fps16x16_neq;
extern symbol *fps16x16_lt;
extern symbol *fps16x16_lteq;
extern symbol *fps16x16_gt;
extern symbol *fps16x16_gteq;

/* Dims: mul/div/mod, BYTE/WORD/DWORD/QWORD, SIGNED/UNSIGNED/BOTH */
extern symbol *muldiv[3][4][4];
/* 16 x 16 -> 32 multiplication SIGNED/UNSIGNED */
extern symbol *muls16tos32[2];
/* Dims: BYTE/WORD/DWORD/QWORD SIGNED/UNSIGNED */
extern sym_link *multypes[4][2];
/* Dims: to/from float, BYTE/WORD/DWORD/QWORD, SIGNED/USIGNED */
extern symbol *conv[2][4][2];
/* Dims: to/from fixed16x16, BYTE/WORD/DWORD/QWORD/FLOAT, SIGNED/USIGNED */
extern symbol *fp16x16conv[2][5][2];
/* Dims: shift left/shift right, BYTE/WORD/DWORD/QWORD, SIGNED/UNSIGNED */
extern symbol *rlrr[2][4][2];

#define SCHARTYPE       multypes[0][0]
#define UCHARTYPE       multypes[0][1]
#define INTTYPE         multypes[1][0]
#define UINTTYPE        multypes[1][1]
#define LONGTYPE        multypes[2][0]
#define ULONGTYPE       multypes[2][1]
#define LONGLONGTYPE    multypes[3][0]
#define ULONGLONGTYPE   multypes[3][1]

extern sym_link *floatType;
extern sym_link *fixed16x16Type;

#include "SDCCval.h"

typedef enum
{
  RESULT_TYPE_NONE = 0,         /* operands will be promoted to int */
  RESULT_TYPE_BOOL,
  RESULT_TYPE_CHAR,
  RESULT_TYPE_INT,
  RESULT_TYPE_OTHER,            /* operands will be promoted to int */
  RESULT_TYPE_IFX,
  RESULT_TYPE_GPTR              /* operands will be promoted to generic ptr */
} RESULT_TYPE;

/* forward definitions for the symbol table related functions */
void initSymt ();
symbol *newSymbol (const char *, int);
sym_link *newLink (SYM_LINK_CLASS);
sym_link *newFloatLink ();
structdef *newStruct (const char *);
void addDecl (symbol *, int, sym_link *);
sym_link *finalizeSpec (sym_link *);
sym_link *mergeSpec (sym_link *, sym_link *, const char *name);
sym_link *mergeDeclSpec (sym_link *, sym_link *, const char *name);
symbol *reverseSyms (symbol *);
sym_link *reverseLink (sym_link *);
symbol *copySymbol (const symbol *);
symbol *copySymbolChain (const symbol *);
void printSymChain (symbol *, int);
void printStruct (structdef *, int);
char *genSymName (int);
sym_link *getSpec (sym_link *);
int compStructSize (int, structdef *);
sym_link *copyLinkChain (const sym_link *);
int checkDecl (symbol *, int);
void checkBasic (sym_link *, sym_link *);
value *checkPointerIval (sym_link *, value *);
value *checkStructIval (symbol *, value *);
value *checkArrayIval (sym_link *, value *);
value *checkIval (sym_link *, value *);
unsigned int getSize (sym_link *);
unsigned int bitsForType (sym_link *);
sym_link *newIntLink ();
sym_link *newCharLink ();
sym_link *newLongLink ();
sym_link *newBoolLink ();
sym_link *newVoidLink ();
int compareType (sym_link *, sym_link *);
int compareTypeExact (sym_link *, sym_link *, int);
int compareTypeInexact (sym_link *, sym_link *);
int checkFunction (symbol *, symbol *);
void cleanUpLevel (bucket **, int);
void cleanUpBlock (bucket **, int);
symbol *getAddrspace (sym_link *type);
int funcInChain (sym_link *);
void addSymChain (symbol **);
sym_link *structElemType (sym_link *, value *);
symbol *getStructElement (structdef *, symbol *);
sym_link *computeType (sym_link *, sym_link *, RESULT_TYPE, int);
void processFuncPtrArgs (sym_link *);
void processFuncArgs (symbol *);
int isSymbolEqual (symbol *, symbol *);
int powof2 (TYPE_TARGET_ULONG);
void dbuf_printTypeChain (sym_link *, struct dbuf_s *);
void printTypeChain (sym_link *, FILE *);
void printTypeChainRaw (sym_link *, FILE *);
void initCSupport ();
void initBuiltIns ();
void pointerTypes (sym_link *, sym_link *);
void cdbStructBlock (int);
void initHashT ();
bucket *newBucket ();
void addSym (bucket **, void *, char *, int, int, int checkType);
void deleteSym (bucket **, void *, const char *);
void *findSym (bucket **, void *, const char *);
void *findSymWithLevel (bucket **, struct symbol *);
void *findSymWithBlock (bucket **, struct symbol *, int, int);
void changePointer (sym_link * p);
void checkTypeSanity (sym_link * etype, const char *name);
sym_link *typeFromStr (const char *);
STORAGE_CLASS sclsFromPtr (sym_link * ptr);
sym_link *newEnumType (symbol *);
void promoteAnonStructs (int, structdef *);
int isConstant (sym_link * type);
int isVolatile (sym_link * type);
int isRestrict (sym_link * type);
value *aggregateToPointer (value *);


extern char *nounName (sym_link *);     /* noun strings */
extern void printFromToType (sym_link *, sym_link *);

#endif
