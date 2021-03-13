/*-------------------------------------------------------------------------
  main.h - ds390 specific general functions

  Copyright (C) 2000, Kevin Vigor

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
/*
  Note that mlh prepended _ds390_ on the static functions.  Makes
  it easier to set a breakpoint using the debugger.
*/

#include "common.h"
#include "main.h"
#include "ralloc.h"
#include "gen.h"
#include "dbuf_string.h"

static char _defaultRules[] =
{
#include "peeph.rul"
};

#define OPTION_STACK_8BIT       "--stack-8bit"
#define OPTION_FLAT24_MODEL     "--model-flat24"
#define OPTION_STACK_SIZE       "--stack-size"

static OPTION _ds390_options[] =
  {
    { 0, OPTION_FLAT24_MODEL,   NULL, "use the flat24 model for the ds390 (default)" },
    { 0, OPTION_STACK_8BIT,     NULL, "use the 8bit stack for the ds390 (not supported yet)" },
    { 0, OPTION_STACK_SIZE,     &options.stack_size, "Tells the linker to allocate this space for stack", CLAT_INTEGER },
    { 0, "--stack-10bit",       &options.stack10bit, "use the 10bit stack for ds390 (default)" },
    { 0, "--use-accelerator",   &options.useAccelerator, "generate code for ds390 arithmetic accelerator"},
    { 0, "--protect-sp-update", &options.protect_sp_update, "will disable interrupts during ESP:SP updates"},
    { 0, "--parms-in-bank1",    &options.parms_in_bank1, "use Bank1 for parameter passing"},
    { 0, NULL }
  };


/* list of key words used by msc51 */
static char *_ds390_keywords[] =
{
  "at",
  "bit",
  "code",
  "critical",
  "data",
  "far",
  "idata",
  "interrupt",
  "near",
  "pdata",
  "reentrant",
  "sfr",
  "sfr16",
  "sfr32",
  "sbit",
  "using",
  "xdata",
  "_data",
  "_code",
  "_generic",
  "_near",
  "_xdata",
  "_pdata",
  "_idata",
  "_naked",
  NULL
};

static builtins __ds390_builtins[] = {
    { "__builtin_memcpy_x2x","v",3,{"cx*","cx*","i"}}, /* void __builtin_memcpy_x2x (xdata char *,xdata char *,int) */
    { "__builtin_memcpy_c2x","v",3,{"cx*","cp*","i"}}, /* void __builtin_memcpy_c2x (xdata char *,code  char *,int) */
    { "__builtin_memset_x","v",3,{"cx*","c","i"}},     /* void __builtin_memset     (xdata char *,char,int)         */
    /* __builtin_inp - used to read from a memory mapped port, increment first pointer */
    { "__builtin_inp","v",3,{"cx*","cx*","i"}},        /* void __builtin_inp        (xdata char *,xdata char *,int) */
    /* __builtin_inp - used to write to a memory mapped port, increment first pointer */
    { "__builtin_outp","v",3,{"cx*","cx*","i"}},       /* void __builtin_outp       (xdata char *,xdata char *,int) */
    { "__builtin_swapw","Us",1,{"Us"}},                /* unsigned short __builtin_swapw (unsigned short) */
    { "__builtin_memcmp_x2x","c",3,{"cx*","cx*","i"}}, /* void __builtin_memcmp_x2x (xdata char *,xdata char *,int) */
    { "__builtin_memcmp_c2x","c",3,{"cx*","cp*","i"}}, /* void __builtin_memcmp_c2x (xdata char *,code  char *,int) */
    { NULL , NULL,0, {NULL}}                           /* mark end of table */
};
void ds390_assignRegisters (ebbIndex * ebbi);

static int regParmFlg = 0;      /* determine if we can register a parameter */

static void
_ds390_init (void)
{
  asm_addTree (&asm_asxxxx_mapping);
}

static void
_ds390_reset_regparm (struct sym_link *funcType)
{
  regParmFlg = 0;
}

static int
_ds390_regparm (sym_link * l, bool reentrant)
{
    if (IS_SPEC(l) && (SPEC_NOUN(l) == V_BIT))
        return 0;
    if (options.parms_in_bank1 == 0) {
        /* simple can pass only the first parameter in a register */
        if (regParmFlg)
            return 0;

        regParmFlg = 1;
        return 1;
    } else {
        int size = getSize(l);
        int remain ;

        /* first one goes the usual way to DPTR */
        if (regParmFlg == 0) {
            regParmFlg += 4 ;
            return 1;
        }
        /* second one onwards goes to RB1_0 thru RB1_7 */
        remain = regParmFlg - 4;
        if (size > (8 - remain)) {
            regParmFlg = 12 ;
            return 0;
        }
        regParmFlg += size ;
        return regParmFlg - size + 1;
    }
}

static bool
_ds390_parseOptions (int *pargc, char **argv, int *i)
{
  /* TODO: allow port-specific command line options to specify
   * segment names here.
   */
  if (!strcmp (argv[*i], OPTION_STACK_8BIT))
    {
      options.stack10bit = 0;
      return TRUE;
    }
  else if (!strcmp (argv[*i], OPTION_FLAT24_MODEL))
    {
      options.model = MODEL_FLAT24;
      return TRUE;
    }
  return FALSE;
}

static void
_ds390_finaliseOptions (void)
{
  if (options.noXinitOpt)
    {
      port->genXINIT=0;
    }

  /* Hack-o-matic: if we are using the flat24 model,
   * adjust pointer sizes.
   */
  if (options.model != MODEL_FLAT24)
    {
      fprintf (stderr,
               "*** warning: ds390 port small and large model experimental.\n");
      if (options.model == MODEL_LARGE)
        {
          port->mem.default_local_map = xdata;
          port->mem.default_globl_map = xdata;
        }
      else
        {
          port->mem.default_local_map = data;
          port->mem.default_globl_map = data;
        }
    }
  else
    {
      port->s.far_ptr_size = 3;
      port->s.funcptr_size = 3;
      port->s.ptr_size = 4;

      port->stack.isr_overhead += 2;      /* Will save dpx on ISR entry. */

      port->stack.call_overhead += 2;     /* This acounts for the extra byte
                                           * of return addres on the stack.
                                           * but is ugly. There must be a
                                           * better way.
                                           */

      port->mem.default_local_map = xdata;
      port->mem.default_globl_map = xdata;

      if (!options.stack10bit)
        {
          fprintf (stderr,
                   "*** error: ds390 port only supports the 10 bit stack mode.\n");
        }
      else
        {
          if (!options.stack_loc) options.stack_loc = 0x400008;
        }

      /* Fixup the memory map for the stack; it is now in
       * far space and requires an FPOINTER to access it.
       */
      istack->fmap = 1;
      istack->ptrType = FPOINTER;

    }  /* MODEL_FLAT24 */
}

static void
_ds390_setDefaultOptions (void)
{
  options.model=MODEL_FLAT24;
  options.stack10bit=1;
}

static const char *
_ds390_getRegName (const struct reg_info *reg)
{
  if (reg)
    return reg->name;
  return "err";
}

extern char * iComments2;

static void
_ds390_genAssemblerPreamble (FILE * of)
{
  fputs (iComments2, of);
  fputs ("; CPU specific extensions\n",of);
  fputs (iComments2, of);

  fputs ("\t.DS80C390\n", of);

  if (options.model == MODEL_FLAT24)
    fputs ("\t.amode\t2\t; 24 bit flat addressing\n", of);

  fputs ("dpl\t=\t0x82\n", of);
  fputs ("dph\t=\t0x83\n", of);
  fputs ("dpl1\t=\t0x84\n", of);
  fputs ("dph1\t=\t0x85\n", of);
  fputs ("dps\t=\t0x86\n", of);
  fputs ("dpx\t=\t0x93\n", of);
  fputs ("dpx1\t=\t0x95\n", of);
  fputs ("esp\t=\t0x9B\n", of);
  fputs ("ap\t=\t0x9C\n", of);
  fputs ("acc1\t=\t0x9C\n", of);
  fputs ("mcnt0\t=\t0xD1\n", of);
  fputs ("mcnt1\t=\t0xD2\n", of);
  fputs ("ma\t=\t0xD3\n", of);
  fputs ("mb\t=\t0xD4\n", of);
  fputs ("mc\t=\t0xD5\n", of);
  fputs ("acon\t=\t0x9D\n", of);
  fputs ("mcon\t=\t0xC6\n", of);
  fputs ("F1\t=\t0xD1\t; user flag\n", of);
  if (options.parms_in_bank1)
    {
      int i ;
      for (i=0; i < 8 ; i++ )
          fprintf (of,"b1_%d\t=\t0x%02X\n",i,8+i);
    }
}

/* Generate interrupt vector table. */
static int
_ds390_genIVT (struct dbuf_s * oBuf, symbol ** interrupts, int maxInterrupts)
{
  int i;

  if (options.model != MODEL_FLAT24)
    {
      dbuf_printf (oBuf, "\tljmp\t__sdcc_gsinit_startup\n");

      /* now for the other interrupts */
      for (i = 0; i < maxInterrupts; i++)
        {
          if (interrupts[i])
            {
              dbuf_printf (oBuf, "\tljmp\t%s\n", interrupts[i]->rname);
              if ( i != maxInterrupts - 1 )
                dbuf_printf (oBuf, "\t.ds\t5\n");
            }
          else
            {
              dbuf_printf (oBuf, "\treti\n");
              if ( i != maxInterrupts - 1 )
                dbuf_printf (oBuf, "\t.ds\t7\n");
            }
        }
    }
  else
    {
      dbuf_printf (oBuf, "\t.amode\t0\t; 16 bit addressing\n");
      dbuf_printf (oBuf, "\tljmp\t__reset_vect\n");
      dbuf_printf (oBuf, "\t.amode\t2\t; 24 bit flat addressing\n");

      /* now for the other interrupts */
      for (i = 0; i < maxInterrupts; i++)
        {
          if (interrupts[i])
            {
              dbuf_printf (oBuf, "\tljmp\t%s\n\t.ds\t4\n", interrupts[i]->rname);
            }
          else
            {
              dbuf_printf (oBuf, "\treti\n\t.ds\t7\n");
            }
        }

      dbuf_printf (oBuf, "__reset_vect:\n");
      if (options.stack10bit)
        {
          dbuf_printf (oBuf, "\tmov _TA,#0xAA\n");
          dbuf_printf (oBuf, "\tmov _TA,#0x55\n");
          dbuf_printf (oBuf, "\tmov acon,#0x06\t;24 bit addresses, 10 bit stack\n");
          dbuf_printf (oBuf, "\tmov _TA,#0xAA\n");
          dbuf_printf (oBuf, "\tmov _TA,#0x55\n");
          dbuf_printf (oBuf, "\tmov mcon,#0x90\t;10 bit stack at 0x400000\n");
          dbuf_printf (oBuf, "\tmov _ESP,#0x00\t; reinitialize the stack\n");
          dbuf_printf (oBuf, "\tmov _SP,#0x00\n");
        }
      else
        {
          dbuf_printf (oBuf, "\tmov _TA,#0xAA\n");
          dbuf_printf (oBuf, "\tmov _TA,#0x55\n");
          dbuf_printf (oBuf, "\tmov acon,#0x02\t;24 bit addresses, default 8 bit stack\n");
        }
      dbuf_printf (oBuf, "\tljmp\t__sdcc_gsinit_startup\n");
    }
  return TRUE;
}

static void
_ds390_genInitStartup (FILE *of)
{
  fprintf (of, "__sdcc_gsinit_startup:\n");
  /* if external stack is specified then the
     higher order byte of the xdata location is
     going into P2 and the lower order going into
     spx */
  if (options.useXstack)
    {
      fprintf (of, "\tmov\tP2,#0x%02x\n",
               (((unsigned int) options.xdata_loc) >> 8) & 0xff);
      fprintf (of, "\tmov\t_spx,#0x%02x\n",
               (unsigned int) options.xdata_loc & 0xff);
    }

  // This should probably be a port option, but I'm being lazy.
  // on the 400, the firmware boot loader gives us a valid stack
  // (see '400 data sheet pg. 85 (TINI400 ROM Initialization code)
  if (!TARGET_IS_DS400 && !options.stack10bit)
    {
      /* initialise the stack pointer.  JCF: sdld takes care of the location */
      fprintf (of, "\tmov\tsp,#__start__stack - 1\n");     /* MOF */
    }

  fprintf (of, "\tlcall\t__sdcc_external_startup\n");
  fprintf (of, "\tmov\ta,dpl\n");
  fprintf (of, "\tjz\t__sdcc_init_data\n");
  fprintf (of, "\tljmp\t__sdcc_program_startup\n");
  fprintf (of, "__sdcc_init_data:\n");

  // if the port can copy the XINIT segment to XISEG
  if (port->genXINIT)
    {
      port->genXINIT(of);
    }
}

/* Generate code to copy XINIT to XISEG */
static void _ds390_genXINIT (FILE * of)
{
  fprintf (of, ";       _ds390_genXINIT() start\n");
  fprintf (of, "        mov     a,#l_XINIT\n");
  fprintf (of, "        orl     a,#l_XINIT>>8\n");
  fprintf (of, "        jz      00003$\n");
  fprintf (of, "        mov     a,#s_XINIT\n");
  fprintf (of, "        add     a,#l_XINIT\n");
  fprintf (of, "        mov     r1,a\n");
  fprintf (of, "        mov     a,#s_XINIT>>8\n");
  fprintf (of, "        addc    a,#l_XINIT>>8\n");
  fprintf (of, "        mov     r2,a\n");
  fprintf (of, "        mov     dptr,#s_XINIT\n");
  fprintf (of, "        mov     dps,#0x21\n");
  fprintf (of, "        mov     dptr,#s_XISEG\n");
  fprintf (of, "00001$: clr     a\n");
  fprintf (of, "        movc    a,@a+dptr\n");
  fprintf (of, "        movx    @dptr,a\n");
  fprintf (of, "        inc     dptr\n");
  fprintf (of, "        inc     dptr\n");
  fprintf (of, "00002$: mov     a,dpl\n");
  fprintf (of, "        cjne    a,ar1,00001$\n");
  fprintf (of, "        mov     a,dph\n");
  fprintf (of, "        cjne    a,ar2,00001$\n");
  fprintf (of, "        mov     dps,#0\n");
  fprintf (of, "00003$:\n");
  fprintf (of, ";       _ds390_genXINIT() end\n");

  fprintf (of, ";       _ds390_genXRAMCLEAR() start\n");
  fprintf (of, "        mov	r0,#l_PSEG\n");
  fprintf (of, "        mov	a,r0\n");
  fprintf (of, "        orl	a,#(l_PSEG >> 8)\n");
  fprintf (of, "        jz	00006$\n");
  fprintf (of, "        mov	r1,#s_PSEG\n");
  fprintf (of, "        mov	_P2,#(s_PSEG >> 8)\n");
  fprintf (of, "        clr     a\n");
  fprintf (of, "00005$:	movx	@r1,a\n");
  fprintf (of, "        inc	r1\n");
  fprintf (of, "        djnz	r0,00005$\n");
  fprintf (of, "00006$: mov	r0,#l_XSEG\n");
  fprintf (of, "        mov	a,r0\n");
  fprintf (of, "        orl	a,#(l_XSEG >> 8)\n");
  fprintf (of, "        jz	00008$\n");
  fprintf (of, "        mov	r1,#((l_XSEG + 255) >> 8)\n");
  fprintf (of, "        mov	dptr,#s_XSEG\n");
  fprintf (of, "        clr     a\n");
  fprintf (of, "00007$:	movx	@dptr,a\n");
  fprintf (of, "        inc	dptr\n");
  fprintf (of, "        djnz	r0,00007$\n");
  fprintf (of, "        djnz	r1,00007$\n");
  fprintf (of, "00008$:\n");
  fprintf (of, ";       _ds390_genXRAMCLEAR() end\n");
}

/* Do CSE estimation */
static bool cseCostEstimation (iCode *ic, iCode *pdic)
{
    operand *result = IC_RESULT(ic);
    //operand *right  = IC_RIGHT(ic);
    //operand *left   = IC_LEFT(ic);
    sym_link *result_type = operandType(result);
    //sym_link *right_type  = (right ? operandType(right) : 0);
    //sym_link *left_type   = (left  ? operandType(left)  : 0);

    /* if it is a pointer then return ok for now */
    if (IC_RESULT(ic) && IS_PTR(result_type)) return 1;

    /* if bitwise | add & subtract then no since mcs51 is pretty good at it
       so we will cse only if they are local (i.e. both ic & pdic belong to
       the same basic block */
    if (IS_BITWISE_OP(ic) || ic->op == '+' || ic->op == '-') {
        /* then if they are the same Basic block then ok */
        if (ic->eBBlockNum == pdic->eBBlockNum) return 1;
        else return 0;
    }

    /* for others it is cheaper to do the cse */
    return 1;
}

bool _ds390_nativeMulCheck(iCode *ic, sym_link *left, sym_link *right)
{
    return
      getSize (left) == 1 && getSize (right) == 1 ||
      options.useAccelerator && getSize (left) == 2 && getSize (right) == 2;
}

/* Indicate which extended bit operations this port supports */
static bool
hasExtBitOp (int op, int size)
{
  if (op == RRC
      || op == RLC
      || op == GETHBIT
      || (op == SWAP && size <= 2)
     )
    return TRUE;
  else
    return FALSE;
}

/* Indicate the expense of an access to an output storage class */
static int
oclsExpense (struct memmap *oclass)
{
  if (IN_FARSPACE(oclass))
    return 1;

  return 0;
}

static int
instructionSize(char *inst, char *op1, char *op2)
{
  int isflat24 = (options.model == MODEL_FLAT24);

  #define ISINST(s) (strncmp(inst, (s), sizeof(s)-1) == 0)
  #define IS_A(s) (*(s) == 'a' && *(s+1) == '\0')
  #define IS_C(s) (*(s) == 'c' && *(s+1) == '\0')
  #define IS_Rn(s) (*(s) == 'r' && *(s+1) >= '0' && *(s+1) <= '7')
  #define IS_atRi(s) (*(s) == '@' && *(s+1) == 'r')

  /* Based on the current (2003-08-22) code generation for the
     small library, the top instruction probability is:

       57% mov/movx/movc
        6% push
        6% pop
        4% inc
        4% lcall
        4% add
        3% clr
        2% subb
  */
  /* mov, push, & pop are the 69% of the cases. Check them first! */
  if (ISINST ("mov"))
    {
      if (*(inst+3)=='x') return 1; /* movx */
      if (*(inst+3)=='c') return 1; /* movc */
      if (IS_C (op1) || IS_C (op2)) return 2;
      if (IS_A (op1))
        {
          if (IS_Rn (op2) || IS_atRi (op2)) return 1;
          return 2;
        }
      if (IS_Rn(op1) || IS_atRi(op1))
        {
          if (IS_A(op2)) return 1;
          return 2;
        }
      if (strcmp (op1, "dptr") == 0) return 3+isflat24;
      if (IS_A (op2) || IS_Rn (op2) || IS_atRi (op2)) return 2;
      return 3;
    }

  if (ISINST ("push")) return 2;
  if (ISINST ("pop")) return 2;

  if (ISINST ("lcall")) return 3+isflat24;
  if (ISINST ("ret")) return 1;
  if (ISINST ("ljmp")) return 3+isflat24;
  if (ISINST ("sjmp")) return 2;
  if (ISINST ("rlc")) return 1;
  if (ISINST ("rrc")) return 1;
  if (ISINST ("rl")) return 1;
  if (ISINST ("rr")) return 1;
  if (ISINST ("swap")) return 1;
  if (ISINST ("jc")) return 2;
  if (ISINST ("jnc")) return 2;
  if (ISINST ("jb")) return 3;
  if (ISINST ("jnb")) return 3;
  if (ISINST ("jbc")) return 3;
  if (ISINST ("jmp")) return 1; // always jmp @a+dptr
  if (ISINST ("jz")) return 2;
  if (ISINST ("jnz")) return 2;
  if (ISINST ("cjne")) return 3;
  if (ISINST ("mul")) return 1;
  if (ISINST ("div")) return 1;
  if (ISINST ("da")) return 1;
  if (ISINST ("xchd")) return 1;
  if (ISINST ("reti")) return 1;
  if (ISINST ("nop")) return 1;
  if (ISINST ("acall")) return 2+isflat24;
  if (ISINST ("ajmp")) return 2+isflat24;


  if (ISINST ("add") || ISINST ("addc") || ISINST ("subb") || ISINST ("xch"))
    {
      if (IS_Rn(op2) || IS_atRi(op2)) return 1;
      return 2;
    }
  if (ISINST ("inc") || ISINST ("dec"))
    {
      if (IS_A(op1) || IS_Rn(op1) || IS_atRi(op1)) return 1;
      if (strcmp(op1, "dptr") == 0) return 1;
      return 2;
    }
  if (ISINST ("anl") || ISINST ("orl") || ISINST ("xrl"))
    {
      if (IS_C(op1)) return 2;
      if (IS_A(op1))
        {
          if (IS_Rn(op2) || IS_atRi(op2)) return 1;
          return 2;
        }
      else
        {
          if (IS_A(op2)) return 2;
          return 3;
        }
    }
  if (ISINST ("clr") || ISINST ("setb") || ISINST ("cpl"))
    {
      if (IS_A(op1) || IS_C(op1)) return 1;
      return 2;
    }
  if (ISINST ("djnz"))
    {
      if (IS_Rn(op1)) return 2;
      return 3;
    }

  /* If the instruction is unrecognized, we shouldn't try to optimize. */
  /* Return a large value to discourage optimization.                  */
  return 999;
}

asmLineNode *
ds390newAsmLineNode (int currentDPS)
{
  asmLineNode *aln;

  aln = Safe_alloc ( sizeof (asmLineNode));
  aln->size = 0;
  aln->regsRead = NULL;
  aln->regsWritten = NULL;
  aln->initialized = 0;
  aln->currentDPS = currentDPS;

  return aln;
}


typedef struct ds390operanddata
  {
    char name[6];
    int regIdx1;
    int regIdx2;
  }
ds390operanddata;

static ds390operanddata ds390operandDataTable[] =
  {
    {"acc1",   AP_IDX,   -1},
    {"a",     A_IDX,    -1},
    {"ab",    A_IDX,    B_IDX},
    {"ac",    CND_IDX,  -1},
    {"ap",    AP_IDX,   -1},
    {"acc",   A_IDX,    -1},
    {"ar0",   R0_IDX,   -1},
    {"ar1",   R1_IDX,   -1},
    {"ar2",   R2_IDX,   -1},
    {"ar3",   R3_IDX,   -1},
    {"ar4",   R4_IDX,   -1},
    {"ar5",   R5_IDX,   -1},
    {"ar6",   R6_IDX,   -1},
    {"ar7",   R7_IDX,   -1},
    {"b",     B_IDX,    -1},
    {"c",     CND_IDX,  -1},
    {"cy",    CND_IDX,  -1},
    {"dph",   DPH_IDX,  -1},
    {"dph0",  DPH_IDX,  -1},
    {"dph1",  DPH1_IDX, -1},
    {"dpl",   DPL_IDX,  -1},
    {"dpl0",  DPL_IDX,  -1},
    {"dpl1",  DPL1_IDX, -1},
/*  {"dptr",  DPL_IDX,  DPH_IDX}, */ /* dptr is special, based on currentDPS */
    {"dps",   DPS_IDX,  -1},
    {"dpx",   DPX_IDX,  -1},
    {"dpx0",  DPX_IDX,  -1},
    {"dpx1",  DPX1_IDX, -1},
    {"f0",    CND_IDX,  -1},
    {"f1",    CND_IDX,  -1},
    {"ov",    CND_IDX,  -1},
    {"p",     CND_IDX,  -1},
    {"psw",   CND_IDX,  -1},
    {"r0",    R0_IDX,   -1},
    {"r1",    R1_IDX,   -1},
    {"r2",    R2_IDX,   -1},
    {"r3",    R3_IDX,   -1},
    {"r4",    R4_IDX,   -1},
    {"r5",    R5_IDX,   -1},
    {"r6",    R6_IDX,   -1},
    {"r7",    R7_IDX,   -1},
  };

static int
ds390operandCompare (const void *key, const void *member)
{
  return strcmp((const char *)key, ((ds390operanddata *)member)->name);
}

static void
updateOpRW (asmLineNode *aln, char *op, char *optype, int currentDPS)
{
  ds390operanddata *opdat;
  char *dot;
  int regIdx1 = -1;
  int regIdx2 = -1;
  int regIdx3 = -1;

  dot = strchr(op, '.');
  if (dot)
    *dot = '\0';

  opdat = bsearch (op, ds390operandDataTable,
                   sizeof(ds390operandDataTable)/sizeof(ds390operanddata),
                   sizeof(ds390operanddata), ds390operandCompare);

  if (opdat)
    {
      regIdx1 = opdat->regIdx1;
      regIdx2 = opdat->regIdx2;
    }
  if (!strcmp(op, "dptr"))
    {
      if (!currentDPS)
        {
          regIdx1 = DPL_IDX;
          regIdx2 = DPH_IDX;
          regIdx3 = DPX_IDX;
        }
      else
        {
          regIdx1 = DPL1_IDX;
          regIdx2 = DPH1_IDX;
          regIdx3 = DPX1_IDX;
        }
    }

  if (strchr(optype,'r'))
    {
      if (regIdx1 >= 0)
        aln->regsRead = bitVectSetBit (aln->regsRead, regIdx1);
      if (regIdx2 >= 0)
        aln->regsRead = bitVectSetBit (aln->regsRead, regIdx2);
      if (regIdx3 >= 0)
        aln->regsRead = bitVectSetBit (aln->regsRead, regIdx3);
    }
  if (strchr(optype,'w'))
    {
      if (regIdx1 >= 0)
        aln->regsWritten = bitVectSetBit (aln->regsWritten, regIdx1);
      if (regIdx2 >= 0)
        aln->regsWritten = bitVectSetBit (aln->regsWritten, regIdx2);
      if (regIdx3 >= 0)
        aln->regsWritten = bitVectSetBit (aln->regsWritten, regIdx3);
    }
  if (op[0] == '@')
    {
      if (!strcmp(op, "@r0"))
        aln->regsRead = bitVectSetBit (aln->regsRead, R0_IDX);
      if (!strcmp(op, "@r1"))
        aln->regsRead = bitVectSetBit (aln->regsRead, R1_IDX);
      if (strstr(op, "dptr"))
        {
          if (!currentDPS)
            {
              aln->regsRead = bitVectSetBit (aln->regsRead, DPL_IDX);
              aln->regsRead = bitVectSetBit (aln->regsRead, DPH_IDX);
              aln->regsRead = bitVectSetBit (aln->regsRead, DPX_IDX);
            }
          else
            {
              aln->regsRead = bitVectSetBit (aln->regsRead, DPL1_IDX);
              aln->regsRead = bitVectSetBit (aln->regsRead, DPH1_IDX);
              aln->regsRead = bitVectSetBit (aln->regsRead, DPX1_IDX);
            }
        }
      if (strstr(op, "a+"))
        aln->regsRead = bitVectSetBit (aln->regsRead, A_IDX);
    }
}

typedef struct ds390opcodedata
  {
    char name[6];
    char class[3];
    char pswtype[3];
    char op1type[3];
    char op2type[3];
  }
ds390opcodedata;

static ds390opcodedata ds390opcodeDataTable[] =
  {
    {"acall","j", "",   "",   ""},
    {"add",  "",  "w",  "rw", "r"},
    {"addc", "",  "rw", "rw", "r"},
    {"ajmp", "j", "",   "",   ""},
    {"anl",  "",  "",   "rw", "r"},
    {"cjne", "j", "w",  "r",  "r"},
    {"clr",  "",  "",   "w",  ""},
    {"cpl",  "",  "",   "rw", ""},
    {"da",   "",  "rw", "rw", ""},
    {"dec",  "",  "",   "rw", ""},
    {"div",  "",  "w",  "rw", ""},
    {"djnz", "j", "",  "rw",  ""},
    {"inc",  "",  "",   "rw", ""},
    {"jb",   "j", "",   "r",  ""},
    {"jbc",  "j", "",  "rw",  ""},
    {"jc",   "j", "",   "",   ""},
    {"jmp",  "j", "",  "",    ""},
    {"jnb",  "j", "",   "r",  ""},
    {"jnc",  "j", "",   "",   ""},
    {"jnz",  "j", "",  "",    ""},
    {"jz",   "j", "",  "",    ""},
    {"lcall","j", "",   "",   ""},
    {"ljmp", "j", "",   "",   ""},
    {"mov",  "",  "",   "w",  "r"},
    {"movc", "",  "",   "w",  "r"},
    {"movx", "",  "",   "w",  "r"},
    {"mul",  "",  "w",  "rw", ""},
    {"nop",  "",  "",   "",   ""},
    {"orl",  "",  "",   "rw", "r"},
    {"pop",  "",  "",   "w",  ""},
    {"push", "",  "",   "r",  ""},
    {"ret",  "j", "",   "",   ""},
    {"reti", "j", "",   "",   ""},
    {"rl",   "",  "",   "rw", ""},
    {"rlc",  "",  "rw", "rw", ""},
    {"rr",   "",  "",   "rw", ""},
    {"rrc",  "",  "rw", "rw", ""},
    {"setb", "",  "",   "w",  ""},
    {"sjmp", "j", "",   "",   ""},
    {"subb", "",  "rw", "rw", "r"},
    {"swap", "",  "",   "rw", ""},
    {"xch",  "",  "",   "rw", "rw"},
    {"xchd", "",  "",   "rw", "rw"},
    {"xrl",  "",  "",   "rw", "r"},
  };

static int
ds390opcodeCompare (const void *key, const void *member)
{
  return strcmp((const char *)key, ((ds390opcodedata *)member)->name);
}

static asmLineNode *
asmLineNodeFromLineNode (lineNode *ln, int currentDPS)
{
  asmLineNode *aln = ds390newAsmLineNode(currentDPS);
  char *op, op1[256], op2[256];
  int opsize;
  const char *p;
  char inst[8];
  ds390opcodedata *opdat;

  aln->initialized = 1;

  p = ln->line;

  while (*p && isspace(*p)) p++;
  for (op = inst, opsize=1; *p; p++)
    {
      if (isspace(*p) || *p == ';' || *p == ':' || *p == '=')
        break;
      else
        if (opsize < sizeof(inst))
          *op++ = tolower(*p), opsize++;
    }
  *op = '\0';

  if (*p == ';' || *p == ':' || *p == '=')
    return aln;

  while (*p && isspace(*p)) p++;
  if (*p == '=')
    return aln;

  for (op = op1, opsize=1; *p && *p != ','; p++)
    {
      if (!isspace(*p) && opsize < sizeof(op1))
        *op++ = tolower(*p), opsize++;
    }
  *op = '\0';

  if (*p == ',') p++;
  for (op = op2, opsize=1; *p && *p != ','; p++)
    {
      if (!isspace(*p) && opsize < sizeof(op2))
        *op++ = tolower(*p), opsize++;
    }
  *op = '\0';

  aln->size = instructionSize(inst, op1, op2);

  aln->regsRead = newBitVect (END_IDX);
  aln->regsWritten = newBitVect (END_IDX);

  opdat = bsearch (inst, ds390opcodeDataTable,
                   sizeof(ds390opcodeDataTable)/sizeof(ds390opcodedata),
                   sizeof(ds390opcodedata), ds390opcodeCompare);

  if (opdat)
    {
      updateOpRW (aln, op1, opdat->op1type, currentDPS);
      updateOpRW (aln, op2, opdat->op2type, currentDPS);
      if (strchr(opdat->pswtype,'r'))
        aln->regsRead = bitVectSetBit (aln->regsRead, CND_IDX);
      if (strchr(opdat->pswtype,'w'))
        aln->regsWritten = bitVectSetBit (aln->regsWritten, CND_IDX);
    }

  return aln;
}

static void
initializeAsmLineNode (lineNode *line)
{
  if (!line->aln)
    line->aln = (asmLineNodeBase *) asmLineNodeFromLineNode (line, 0);
  else if (line->aln && !((asmLineNode *)line->aln)->initialized)
    {
      int currentDPS = ((asmLineNode *)line->aln)->currentDPS;
      free(line->aln);
      line->aln = (asmLineNodeBase *) asmLineNodeFromLineNode (line, currentDPS);
    }
}

static int
getInstructionSize (lineNode *line)
{
  initializeAsmLineNode (line);
  return line->aln->size;
}

static bitVect *
getRegsRead (lineNode *line)
{
  initializeAsmLineNode (line);
  return line->aln->regsRead;
}

static bitVect *
getRegsWritten (lineNode *line)
{
  initializeAsmLineNode (line);
  return line->aln->regsWritten;
}

static const char *
get_model (void)
{
  switch (options.model)
    {
    case MODEL_SMALL:
      if (options.stackAuto)
        return "small-stack-auto";
      else
        return "small";

    case MODEL_LARGE:
      if (options.stackAuto)
        return "large-stack-auto";
      else
        return "large";

    case MODEL_FLAT24:
        return port->target;

    default:
      werror (W_UNKNOWN_MODEL, __FILE__, __LINE__);
      return "unknown";
    }
}

/** $1 is always the basename.
    $2 is always the output file.
    $3 varies
    $l is the list of extra options that should be there somewhere...
    MUST be terminated with a NULL.
*/
static const char *_linkCmd[] =
{
  "sdld", "-nf", "$1", NULL
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *_asmCmd[] =
{
  "sdas390", "$l", "$3", "$2", "$1.asm", NULL
};

static const char * const _libs_ds390[] = { STD_DS390_LIB, NULL, };

/* Globals */
PORT ds390_port =
{
  TARGET_ID_DS390,
  "ds390",
  "DS80C390",                   /* Target name */
  NULL,
  {
    glue,
    TRUE,                       /* Emit glue around main */
    MODEL_SMALL | MODEL_LARGE | MODEL_FLAT24,
    MODEL_SMALL,
    get_model,
  },
  {
    _asmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm",
    NULL                        /* no do_assemble function */
  },
  {                             /* Linker */
    _linkCmd,
    NULL,
    NULL,
    ".rel",
    1,
    NULL,                       /* crt */
    _libs_ds390,                /* libs */
  },
  {
    _defaultRules,
    getInstructionSize,
    getRegsRead,
    getRegsWritten,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, bit, float */
  { 1, 2, 2, 4, 8, 1, 2, 3, 2, 3, 1, 4 },

  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },           /* far, near, xstack, code */

  {
    "XSEG    (XDATA)",
    "STACK   (DATA)",
    "CSEG    (CODE)",
    "DSEG    (DATA)",
    "ISEG    (DATA)",
    "PSEG    (PAG,XDATA)",
    "XSEG    (XDATA)",
    "BSEG    (BIT)",
    "RSEG    (DATA)",
    "GSINIT  (CODE)",
    "OSEG    (OVR,DATA)",
    "GSFINAL (CODE)",
    "HOME    (CODE)",
    "XISEG   (XDATA)",          // initialized xdata
    "XINIT   (CODE)",           // a code copy of xiseg
    "CONST   (CODE)",           // const_name - const data (code or not)
    "CABS    (ABS,CODE)",       // cabs_name - const absolute data (code or not)
    "XABS    (ABS,XDATA)",      // xabs_name - absolute xdata/pdata
    "IABS    (ABS,DATA)",       // iabs_name - absolute idata/data
    NULL,                       // name of segment for initialized variables
    NULL,                       // name of segment for copies of initialized variables in code space
    NULL,
    NULL,
    1,
    1                           // No fancy alignments supported.
  },
  { NULL, NULL },
  { +1, 1, 4, 1, 1, 0, 0 },
  /* ds390 has an 16 bit mul & div */
  { -1, FALSE },
  { ds390_emitDebuggerSymbol },
  {
    255/4,      /* maxCount */
    4,          /* sizeofElement */
    {8,12,20},  /* sizeofMatchJump[] */
    {10,14,22}, /* sizeofRangeCompare[] */
    4,          /* sizeofSubtract */
    7,          /* sizeofDispatch */
  },
  "_",
  _ds390_init,
  _ds390_parseOptions,
  _ds390_options,
  NULL,
  _ds390_finaliseOptions,
  _ds390_setDefaultOptions,
  ds390_assignRegisters,
  _ds390_getRegName,
  0,
  NULL,
  _ds390_keywords,
  _ds390_genAssemblerPreamble,
  NULL,                         /* no genAssemblerEnd */
  _ds390_genIVT,
  _ds390_genXINIT,
  _ds390_genInitStartup,
  _ds390_reset_regparm,
  _ds390_regparm,
  NULL,
  NULL,
  _ds390_nativeMulCheck,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  FALSE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* No array initializer support. */
  cseCostEstimation,
  __ds390_builtins,             /* table of builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local static allowed */
  0,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

/*---------------------------------------------------------------------------------*/
/*                               TININative specific                               */
/*---------------------------------------------------------------------------------*/

#define OPTION_TINI_LIBID "--tini-libid"

static OPTION _tininative_options[] =
  {
    { 0, OPTION_FLAT24_MODEL,   NULL, "use the flat24 model for the ds390 (default)" },
    { 0, OPTION_STACK_8BIT,     NULL, "use the 8bit stack for the ds390 (not supported yet)" },
    { 0, OPTION_STACK_SIZE,     &options.stack_size, "Tells the linker to allocate this space for stack", CLAT_INTEGER },
    { 0, "--stack-10bit",       &options.stack10bit, "use the 10bit stack for ds390 (default)" },
    { 0, "--use-accelerator",   &options.useAccelerator, "generate code for ds390 arithmetic accelerator"},
    { 0, "--protect-sp-update", &options.protect_sp_update, "will disable interrupts during ESP:SP updates"},
    { 0, "--parms-in-bank1",    &options.parms_in_bank1, "use Bank1 for parameter passing"},
    { 0, OPTION_TINI_LIBID,     &options.tini_libid, "<nnnn> LibraryID used in -mTININative", CLAT_INTEGER },
    { 0, NULL }
  };

static void _tininative_init (void)
{
    asm_addTree (&asm_a390_mapping);
}

static void _tininative_setDefaultOptions (void)
{
    options.model=MODEL_FLAT24;
    options.stack10bit=1;
    options.stackAuto = 1;
}

static void _tininative_finaliseOptions (void)
{
    /* Hack-o-matic: if we are using the flat24 model,
     * adjust pointer sizes.
     */
    if (options.model != MODEL_FLAT24)  {
        options.model = MODEL_FLAT24 ;
        fprintf(stderr,"TININative supports only MODEL FLAT24\n");
    }
    port->s.far_ptr_size = 3;
    port->s.funcptr_size = 3;
    port->s.ptr_size = 4;

    port->stack.isr_overhead += 2;      /* Will save dpx on ISR entry. */

    port->stack.call_overhead += 2;     /* This acounts for the extra byte
                                         * of return address on the stack.
                                         * but is ugly. There must be a
                                         * better way.
                                         */

    port->mem.default_local_map = xdata;
    port->mem.default_globl_map = xdata;

    if (!options.stack10bit) {
        options.stack10bit = 1;
        fprintf(stderr,"TININative supports only stack10bit \n");
    }

    if (!options.stack_loc) options.stack_loc = 0x400008;

    /* Fixup the memory map for the stack; it is now in
     * far space and requires a FPOINTER to access it.
     */
    istack->fmap = 1;
    istack->ptrType = FPOINTER;
    options.cc_only =1;
}

static int _tininative_genIVT (struct dbuf_s * oBuf, symbol ** interrupts, int maxInterrupts)
{
    return TRUE;
}

static void _tininative_genAssemblerPreamble (FILE * of)
{
    fputs("$include(tini.inc)\n", of);
    fputs("$include(ds80c390.inc)\n", of);
    fputs("$include(tinimacro.inc)\n", of);
    fputs("$include(apiequ.inc)\n", of);
    fputs("_bpx EQU 01Eh \t\t; _bpx (frame pointer) mapped to R7_B3:R6_B3\n", of);
    fputs("acc1  EQU 01Dh \t\t; acc1 mapped to R5_B3\n", of);
    /* Must be first and return 0 */
    fputs("Lib_Native_Init:\n",of);
    fputs("\tclr\ta\n",of);
    fputs("\tret\n",of);
    fputs("LibraryID:\n",of);
    fputs("\tdb \"DS\"\n",of);
    if (options.tini_libid) {
        fprintf(of,"\tdb 0,0,0%02xh,0%02xh,0%02xh,0%02xh\n",
                (options.tini_libid>>24 & 0xff),
                (options.tini_libid>>16 & 0xff),
                (options.tini_libid>>8 & 0xff),
                (options.tini_libid  & 0xff));
    } else {
        fprintf(of,"\tdb 0,0,0,0,0,1\n");
    }

}
static void _tininative_genAssemblerEnd (FILE * of)
{
    fputs("\tend\n",of);
}
/* tininative assembler , calls "macro", if it succeeds calls "a390" */
static void _tininative_do_assemble (set *asmOptions)
{
    char *buf;
    static const char *macroCmd[] = {
        "macro","$1.a51",NULL
    };
    static const char *a390Cmd[] = {
        "a390","$1.mpp",NULL
    };

    buf = buildCmdLine(macroCmd, dstFileName, NULL, NULL, NULL);
    if (sdcc_system(buf)) {
        Safe_free (buf);
        exit(1);
    }
    Safe_free (buf);
    buf = buildCmdLine(a390Cmd, dstFileName, NULL, NULL, asmOptions);
    if (sdcc_system(buf)) {
        Safe_free (buf);
        exit(1);
    }
    Safe_free (buf);
}

/* list of key words used by TININative */
static char *_tininative_keywords[] =
{
  "at",
  "bit",
  "code",
  "critical",
  "data",
  "far",
  "idata",
  "interrupt",
  "near",
  "pdata",
  "reentrant",
  "sfr",
  "sbit",
  "using",
  "xdata",
  "_data",
  "_code",
  "_generic",
  "_near",
  "_xdata",
  "_pdata",
  "_idata",
  "_naked",
  "_JavaNative",
  NULL
};

static builtins __tininative_builtins[] = {
    { "__builtin_memcpy_x2x","v",3,{"cx*","cx*","i"}}, /* void __builtin_memcpy_x2x (xdata char *,xdata char *,int) */
    { "__builtin_memcpy_c2x","v",3,{"cx*","cp*","i"}}, /* void __builtin_memcpy_c2x (xdata char *,code  char *,int) */
    { "__builtin_memset_x","v",3,{"cx*","c","i"}},     /* void __builtin_memset     (xdata char *,char,int)         */
    /* TINI NatLib */
    { "NatLib_LoadByte","c",1,{"c"}},                  /* char  Natlib_LoadByte  (0 based parameter number)         */
    { "NatLib_LoadShort","s",1,{"c"}},                 /* short Natlib_LoadShort (0 based parameter number)         */
    { "NatLib_LoadInt","l",1,{"c"}},                   /* long  Natlib_LoadLong  (0 based parameter number)         */
    { "NatLib_LoadPointer","cx*",1,{"c"}},             /* long  Natlib_LoadPointer  (0 based parameter number)      */
    /* TINI StateBlock related */
    { "NatLib_InstallImmutableStateBlock","c",2,{"vx*","us"}},/* char NatLib_InstallImmutableStateBlock(state block *,int handle) */
    { "NatLib_InstallEphemeralStateBlock","c",2,{"vx*","us"}},/* char NatLib_InstallEphemeralStateBlock(state block *,int handle) */
    { "NatLib_RemoveImmutableStateBlock","v",0,{NULL}},/* void NatLib_RemoveImmutableStateBlock() */
    { "NatLib_RemoveEphemeralStateBlock","v",0,{NULL}},/* void NatLib_RemoveEphemeralStateBlock() */
    { "NatLib_GetImmutableStateBlock","i",0,{NULL}},   /* int  NatLib_GetImmutableStateBlock () */
    { "NatLib_GetEphemeralStateBlock","i",0,{NULL}},   /* int  NatLib_GetEphemeralStateBlock () */
    /* Memory manager */
    { "MM_XMalloc","i",1,{"l"}},                       /* int  MM_XMalloc (long)                */
    { "MM_Malloc","i",1,{"i"}},                        /* int  MM_Malloc  (int)                 */
    { "MM_ApplicationMalloc","i",1,{"i"}},             /* int  MM_ApplicationMalloc  (int)      */
    { "MM_Free","i",1,{"i"}},                          /* int  MM_Free  (int)                   */
    { "MM_Deref","cx*",1,{"i"}},                       /* char *MM_Free  (int)                  */
    { "MM_UnrestrictedPersist","c",1,{"i"}},           /* char  MM_UnrestrictedPersist  (int)   */
    /* System functions */
    { "System_ExecJavaProcess","c",2,{"cx*","i"}},     /* char System_ExecJavaProcess (char *,int) */
    { "System_GetRTCRegisters","v",1,{"cx*"}},         /* void System_GetRTCRegisters (char *) */
    { "System_SetRTCRegisters","v",1,{"cx*"}},         /* void System_SetRTCRegisters (char *) */
    { "System_ThreadSleep","v",2,{"l","c"}},           /* void System_ThreadSleep (long,char)  */
    { "System_ThreadSleep_ExitCriticalSection","v",2,{"l","c"}},/* void System_ThreadSleep_ExitCriticalSection (long,char)  */
    { "System_ProcessSleep","v",2,{"l","c"}},           /* void System_ProcessSleep (long,char)  */
    { "System_ProcessSleep_ExitCriticalSection","v",2,{"l","c"}},/* void System_ProcessSleep_ExitCriticalSection (long,char)  */
    { "System_ThreadResume","c",2,{"c","c"}},          /* char System_ThreadResume(char,char)  */
    { "System_SaveJavaThreadState","v",0,{NULL}},      /* void System_SaveJavaThreadState()    */
    { "System_RestoreJavaThreadState","v",0,{NULL}},   /* void System_RestoreJavaThreadState() */
    { "System_ProcessYield","v",0,{NULL}},             /* void System_ProcessYield() */
    { "System_ProcessSuspend","v",0,{NULL}},           /* void System_ProcessSuspend() */
    { "System_ProcessResume","v",1,{"c"}},             /* void System_ProcessResume(char) */
    { "System_RegisterPoll","c",1,{"vF*"}},            /* char System_RegisterPoll ((void *func pointer)()) */
    { "System_RemovePoll","c",1,{"vF*"}},              /* char System_RemovePoll ((void *func pointer)()) */
    { "System_GetCurrentProcessId","c",0,{NULL}},      /* char System_GetCurrentProcessId() */
    { "System_GetCurrentThreadId","c",0,{NULL}},       /* char System_GetCurrentThreadId() */
    { NULL , NULL,0, {NULL}}                       /* mark end of table */
};

static const char *_a390Cmd[] =
{
  "macro", "$l", "$3", "$1.a51", NULL
};

PORT tininative_port =
{
  TARGET_ID_DS390,
  "TININative",
  "DS80C390",                   /* Target name */
  NULL,                         /* processor */
  {
    glue,
    FALSE,                      /* Emit glue around main */
    MODEL_FLAT24,
    MODEL_FLAT24,
    get_model,
  },
  {
    _a390Cmd,
    NULL,
    "-l",                       /* Options with debug */
    "-l",                       /* Options without debug */
    0,
    ".a51",
    _tininative_do_assemble
  },
  {                             /* Linker */
    NULL,
    NULL,
    NULL,
    ".tlib",
    1,
    NULL,                       /* crt */
    _libs_ds390,                /* libs */
  },
  {
    _defaultRules,
    getInstructionSize,
    getRegsRead,
    getRegsWritten,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 1, 3, 3, 3, 3, 1, 4 },
  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },           /* far, near, xstack, code */

  {
    "XSEG    (XDATA)",
    "STACK   (DATA)",
    "CSEG    (CODE)",
    "DSEG    (DATA)",
    "ISEG    (DATA)",
    "PSEG    (PAG,XDATA)",
    "XSEG    (XDATA)",
    "BSEG    (BIT)",
    "RSEG    (DATA)",
    "GSINIT  (CODE)",
    "OSEG    (OVR,DATA)",
    "GSFINAL (CODE)",
    "HOME    (CODE)",
    NULL,
    NULL,
    "CONST   (CODE)",           // const_name - const data (code or not)
    "CABS    (ABS,CODE)",       // cabs_name - const absolute data (code or not)
    "XABS    (ABS,XDATA)",      // xabs_name - absolute xdata/pdata
    "IABS    (ABS,DATA)",       // iabs_name - absolute idata/data
    NULL,                       // name of segment for initialized variables
    NULL,                       // name of segment for copies of initialized variables in code space
    NULL,
    NULL,
    1,
    1                           // No fancy alignments supported.
  },
  { NULL, NULL },
  { +1, 1, 4, 1, 1, 0, 0 },
  /* ds390 has an 16 bit mul & div */
  { -1, FALSE },
  { ds390_emitDebuggerSymbol },
  {
    255/4,      /* maxCount */
    4,          /* sizeofElement */
    {8,12,20},  /* sizeofMatchJump[] */
    {10,14,22}, /* sizeofRangeCompare[] */
    4,          /* sizeofSubtract */
    7,          /* sizeofDispatch */
  },
  "",
  _tininative_init,
  _ds390_parseOptions,
  _tininative_options,
  NULL,
  _tininative_finaliseOptions,
  _tininative_setDefaultOptions,
  ds390_assignRegisters,
  _ds390_getRegName,
  0,
  NULL,
  _tininative_keywords,
  _tininative_genAssemblerPreamble,
  _tininative_genAssemblerEnd,
  _tininative_genIVT,
  NULL,
  _ds390_genInitStartup,
  _ds390_reset_regparm,
  _ds390_regparm,
  NULL,
  NULL,
  _ds390_nativeMulCheck,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  FALSE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* No array initializer support. */
  cseCostEstimation,
  __tininative_builtins,        /* table of builtin functions */
  FPOINTER,                     /* treat unqualified pointers as far pointers */
  0,                            /* DONOT reset labelKey */
  0,                            /* globals & local static NOT allowed */
  0,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

static int
_ds400_genIVT (struct dbuf_s * oBuf, symbol ** interrupts, int maxInterrupts)
{
  /* We can't generate a static IVT, since the boot rom creates one
   * for us in rom_init.
   *
   * we must patch it as part of the C startup.
   */
  dbuf_printf (oBuf, ";\tDS80C400 IVT must be generated at runtime.\n");
  dbuf_printf (oBuf, "\tsjmp\t__sdcc_400boot\n");
  dbuf_printf (oBuf, "\t.ascii\t'TINI'\t; required signature for 400 boot loader.\n");
  dbuf_printf (oBuf, "\t.db\t0\t; selected bank: zero *should* work...\n");
  dbuf_printf (oBuf, "\t__sdcc_400boot:\tljmp\t__sdcc_gsinit_startup\n");

  return TRUE;
}


/*---------------------------------------------------------------------------------*/
/*                               _ds400 specific                                   */
/*---------------------------------------------------------------------------------*/

static OPTION _ds400_options[] =
  {
    { 0, OPTION_FLAT24_MODEL,   NULL, "use the flat24 model for the ds400 (default)" },
    { 0, OPTION_STACK_8BIT,     NULL, "use the 8bit stack for the ds400 (not supported yet)" },
    { 0, OPTION_STACK_SIZE,     &options.stack_size, "Tells the linker to allocate this space for stack", CLAT_INTEGER },
    { 0, "--stack-10bit",       &options.stack10bit, "use the 10bit stack for ds400 (default)" },
    { 0, "--use-accelerator",   &options.useAccelerator, "generate code for ds400 arithmetic accelerator"},
    { 0, "--protect-sp-update", &options.protect_sp_update, "will disable interrupts during ESP:SP updates"},
    { 0, "--parms-in-bank1",    &options.parms_in_bank1, "use Bank1 for parameter passing"},
    { 0, NULL }
  };

static void
_ds400_finaliseOptions (void)
{
  if (options.noXinitOpt)
    {
      port->genXINIT=0;
    }

  // hackhack: we're a superset of the 390.
  addSet(&preArgvSet, Safe_strdup("-D__SDCC_ds390"));
  addSet(&preArgvSet, Safe_strdup("-D__ds390"));

  /* Hack-o-matic: if we are using the flat24 model,
   * adjust pointer sizes.
   */
  if (options.model != MODEL_FLAT24)
    {
      fprintf (stderr,
               "*** warning: ds400 port small and large model experimental.\n");
      if (options.model == MODEL_LARGE)
        {
          port->mem.default_local_map = xdata;
          port->mem.default_globl_map = xdata;
        }
      else
        {
          port->mem.default_local_map = data;
          port->mem.default_globl_map = data;
        }
    }
  else
    {
      port->s.far_ptr_size = 3;
      port->s.funcptr_size = 3;
      port->s.ptr_size = 4;

      port->stack.isr_overhead += 2;      /* Will save dpx on ISR entry. */

      port->stack.call_overhead += 2;     /* This acounts for the extra byte
                                           * of return addres on the stack.
                                           * but is ugly. There must be a
                                           * better way.
                                           */

      port->mem.default_local_map = xdata;
      port->mem.default_globl_map = xdata;

      if (!options.stack10bit)
        {
          fprintf (stderr,
                   "*** error: ds400 port only supports the 10 bit stack mode.\n");
        }
      else
        {
          if (!options.stack_loc)
            options.stack_loc = 0xffdc00;
          // assumes IDM1:0 = 1:0, CMA = 1.
        }

      /* Fixup the memory map for the stack; it is now in
       * far space and requires a FPOINTER to access it.
       */
      istack->fmap = 1;
      istack->ptrType = FPOINTER;

      // the DS400 rom calling interface uses register bank 3.
      RegBankUsed[3] = 1;

    }  /* MODEL_FLAT24 */
}

static void _ds400_generateRomDataArea(FILE *fp, bool isMain)
{
    /* Only do this for the file containing main() */
    if (isMain)
    {
        fprintf(fp, "%s", iComments2);
        fprintf(fp, "; the direct data area used by the DS80c400 ROM code.\n");
        fprintf(fp, "%s", iComments2);
        fprintf(fp, ".area ROMSEG (ABS,CON,DATA)\n\n");
        fprintf(fp, ".ds 24 ; 24 bytes of directs used starting at 0x68\n\n");
    }
}

static void _ds400_linkRomDataArea(FILE *fp)
{
    fprintf(fp, "-b ROMSEG = 0x0068\n");
}

static const char * const _libs_ds400[] = { STD_DS400_LIB, NULL, };

PORT ds400_port =
{
  TARGET_ID_DS400,
  "ds400",
  "DS80C400",                   /* Target name */
  NULL,
  {
    glue,
    TRUE,                       /* Emit glue around main */
    MODEL_SMALL | MODEL_LARGE | MODEL_FLAT24,
    MODEL_SMALL,
    get_model,
  },
  {
    _asmCmd,
    NULL,
    "-plosgffwy",               /* Options with debug */
    "-plosgffw",                /* Options without debug */
    0,
    ".asm",
    NULL                        /* no do_assemble function */
  },
  {                             /* Linker */
    _linkCmd,
    NULL,
    NULL,
    ".rel",
    1,
    NULL,                       /* crt */
    _libs_ds400,                /* libs */
  },
  {                             /* Peephole optimizer */
    _defaultRules,
    getInstructionSize,
    getRegsRead,
    getRegsWritten,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  },
  /* Sizes: char, short, int, long, long long, near ptr, far ptr, gptr, func ptr, banked func ptr, bit, float */
  { 1, 2, 2, 4, 8, 1, 2, 3, 2, 3, 1, 4 },

  /* tags for generic pointers */
  { 0x00, 0x40, 0x60, 0x80 },           /* far, near, xstack, code */

  {
    "XSEG    (XDATA)",
    "STACK   (DATA)",
    "CSEG    (CODE)",
    "DSEG    (DATA)",
    "ISEG    (DATA)",
    "PSEG    (PAG,XDATA)",
    "XSEG    (XDATA)",
    "BSEG    (BIT)",
    "RSEG    (DATA)",
    "GSINIT  (CODE)",
    "OSEG    (OVR,DATA)",
    "GSFINAL (CODE)",
    "HOME    (CODE)",
    "XISEG   (XDATA)",          // initialized xdata
    "XINIT   (CODE)",           // a code copy of xiseg
    "CONST   (CODE)",           // const_name - const data (code or not)
    "CABS    (ABS,CODE)",       // cabs_name - const absolute data (code or not)
    "XABS    (ABS,XDATA)",      // xabs_name - absolute xdata/pdata
    "IABS    (ABS,DATA)",       // iabs_name - absolute idata/data
    NULL,                       // name of segment for initialized variables
    NULL,                       // name of segment for copies of initialized variables in code space
    NULL,
    NULL,
    1
  },
  { _ds400_generateRomDataArea, _ds400_linkRomDataArea },
  { +1, 1, 4, 1, 1, 0, 0 },
  { -1, FALSE },
  { ds390_emitDebuggerSymbol },
  {
    255/4,      /* maxCount */
    4,          /* sizeofElement */
    {8,12,20},  /* sizeofMatchJump[] */
    {10,14,22}, /* sizeofRangeCompare[] */
    4,          /* sizeofSubtract */
    7,          /* sizeofDispatch */
  },
  "_",
  _ds390_init,
  _ds390_parseOptions,
  _ds400_options,
  NULL,
  _ds400_finaliseOptions,
  _ds390_setDefaultOptions,
  ds390_assignRegisters,
  _ds390_getRegName,
  0,
  NULL,
  _ds390_keywords,
  _ds390_genAssemblerPreamble,
  NULL,                         /* no genAssemblerEnd */
  _ds400_genIVT,
  _ds390_genXINIT,
  _ds390_genInitStartup,
  _ds390_reset_regparm,
  _ds390_regparm,
  NULL,
  NULL,
  _ds390_nativeMulCheck,
  hasExtBitOp,                  /* hasExtBitOp */
  oclsExpense,                  /* oclsExpense */
  FALSE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* No array initializer support. */
  cseCostEstimation,
  __ds390_builtins,             /* table of builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local static allowed */
  0,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};
