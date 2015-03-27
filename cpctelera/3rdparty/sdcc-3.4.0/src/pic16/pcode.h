/*-------------------------------------------------------------------------

   pcode.h - post code generation
   Written By -  Scott Dattalo scott@dattalo.com
   Ported to PIC16 By -  Martin Dubuc m.dubuc@rogers.com

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

//#include "ralloc.h"
struct reg_info;

/*
   Post code generation

   The post code generation is an assembler optimizer. The assembly code
   produced by all of the previous steps is fully functional. This step
   will attempt to analyze the flow of the assembly code and agressively
   optimize it. The peep hole optimizer attempts to do the same thing.
   As you may recall, the peep hole optimizer replaces blocks of assembly
   with more optimal blocks (e.g. removing redundant register loads).
   However, the peep hole optimizer has to be somewhat conservative since
   an assembly program has implicit state information that's unavailable
   when only a few instructions are examined.
     Consider this example:

   example1:
     movwf  t1
     movf   t1,w

   The movf seems redundant since we know that the W register already
   contains the same value of t1. So a peep hole optimizer is tempted to
   remove the "movf". However, this is dangerous since the movf affects
   the flags in the status register (specifically the Z flag) and subsequent
   code may depend upon this. Look at these two examples:

   example2:
     movwf  t1
     movf   t1,w     ; Can't remove this movf
     skpz
      return

   example3:
     movwf  t1
     movf   t1,w     ; This  movf can be removed
     xorwf  t2,w     ; since xorwf will over write Z
     skpz
      return

*/


#ifndef __PCODE_H__
#define __PCODE_H__

/***********************************************************************
 * debug stuff
 *
 * The DFPRINTF macro will call fprintf if PCODE_DEBUG is defined.
 * The macro is used like:
 *
 * DPRINTF(("%s #%d\n","test", 1));
 *
 * The double parenthesis (()) are necessary
 *
 ***********************************************************************/
//#define PCODE_DEBUG

#ifdef PCODE_DEBUG
#define DFPRINTF(args) (fprintf args)
#else
#define DFPRINTF(args) ;
#endif


#ifdef WORDS_BIGENDIAN
  #define _ENDIAN(x)  (3-x)
#else
  #define _ENDIAN(x)  (x)
#endif


#define BYTE_IN_LONG(x,b) ((x>>(8*_ENDIAN(b)))&0xff)


/***********************************************************************
 * Extended Instruction Set/Indexed Literal Offset Mode                *
 * Set this macro to enable code generation with the extended          *
 * instruction set and the new Indexed Literal Offset Mode             *
 ***********************************************************************/
#define XINST   1

/***********************************************************************
 *  PIC status bits - this will move into device dependent headers
 ***********************************************************************/
#define PIC_C_BIT    0
#define PIC_DC_BIT   1
#define PIC_Z_BIT    2
#define PIC_OV_BIT   3
#define PIC_N_BIT    4
#define PIC_IRP_BIT  7   /* Indirect register page select */

/***********************************************************************
 *  PIC INTCON bits - this will move into device dependent headers
 ***********************************************************************/
#define PIC_RBIF_BIT 0   /* Port B level has changed flag */
#define PIC_INTF_BIT 1   /* Port B bit 0 interrupt on edge flag */
#define PIC_T0IF_BIT 2   /* TMR0 has overflowed flag */
#define PIC_RBIE_BIT 3   /* Port B level has changed - Interrupt Enable */
#define PIC_INTE_BIT 4   /* Port B bit 0 interrupt on edge - Int Enable */
#define PIC_T0IE_BIT 5   /* TMR0 overflow Interrupt Enable */
#define PIC_PIE_BIT  6   /* Peripheral Interrupt Enable */
#define PIC_GIE_BIT  7   /* Global Interrupt Enable */

/***********************************************************************
 *  PIC bank definitions
 ***********************************************************************/
#define PIC_BANK_FIRST 0
#define PIC_BANK_LAST  0xf


/***********************************************************************
 *  Operand types
 ***********************************************************************/
#define POT_RESULT  0
#define POT_LEFT    1
#define POT_RIGHT   2


/***********************************************************************
 *
 *  PIC_OPTYPE - Operand types that are specific to the PIC architecture
 *
 *  If a PIC assembly instruction has an operand then here is where we
 *  associate a type to it. For example,
 *
 *     movf    reg,W
 *
 *  The movf has two operands: 'reg' and the W register. 'reg' is some
 *  arbitrary general purpose register, hence it has the type PO_GPR_REGISTER.
 *  The W register, which is the PIC's accumulator, has the type PO_W.
 *
 ***********************************************************************/



typedef enum
{
  PO_NONE=0,         // No operand e.g. NOP
  PO_W,              // The working register (as a destination)
  PO_WREG,           // The working register (as a file register)
  PO_STATUS,         // The 'STATUS' register
  PO_BSR,            // The 'BSR' register
  PO_FSR0,           // The "file select register" (in PIC18 family it's one
                     // of three)
  PO_INDF0,          // The Indirect register
  PO_INTCON,         // Interrupt Control register
  PO_GPR_REGISTER,   // A general purpose register
  PO_GPR_BIT,        // A bit of a general purpose register
  PO_GPR_TEMP,       // A general purpose temporary register
  PO_SFR_REGISTER,   // A special function register (e.g. PORTA)
  PO_PCL,            // Program counter Low register
  PO_PCLATH,         // Program counter Latch high register
  PO_PCLATU,         // Program counter Latch upper register
  PO_PRODL,          // Product Register Low
  PO_PRODH,          // Product Register High
  PO_LITERAL,        // A constant
  PO_REL_ADDR,       // A relative address
  PO_IMMEDIATE,      //  (8051 legacy)
  PO_DIR,            // Direct memory (8051 legacy)
  PO_CRY,            // bit memory (8051 legacy)
  PO_BIT,            // bit operand.
  PO_STR,            //  (8051 legacy)
  PO_LABEL,
  PO_WILD,           // Wild card operand in peep optimizer
  PO_TWO_OPS         // combine two operands
} PIC_OPTYPE;


/***********************************************************************
 *
 *  PIC_OPCODE
 *
 *  This is not a list of the PIC's opcodes per se, but instead
 *  an enumeration of all of the different types of pic opcodes.
 *
 ***********************************************************************/

typedef enum
{
  POC_WILD=-1,   /* Wild card - used in the pCode peep hole optimizer
                  * to represent ANY pic opcode */
  POC_ADDLW=0,
  POC_ADDWF,
  POC_ADDFW,
  POC_ADDFWC,
  POC_ADDWFC,
  POC_ANDLW,
  POC_ANDWF,
  POC_ANDFW,
  POC_BC,
  POC_BCF,
  POC_BN,
  POC_BNC,
  POC_BNN,
  POC_BNOV,
  POC_BNZ,
  POC_BOV,
  POC_BRA,
  POC_BSF,
  POC_BTFSC,
  POC_BTFSS,
  POC_BTG,
  POC_BZ,
  POC_CALL,
  POC_CLRF,
  POC_CLRWDT,
  POC_COMF,
  POC_COMFW,
  POC_CPFSEQ,
  POC_CPFSGT,
  POC_CPFSLT,
  POC_DAW,
  POC_DCFSNZ,
  POC_DCFSNZW,
  POC_DECF,
  POC_DECFW,
  POC_DECFSZ,
  POC_DECFSZW,
  POC_GOTO,
  POC_INCF,
  POC_INCFW,
  POC_INCFSZ,
  POC_INCFSZW,
  POC_INFSNZ,
  POC_INFSNZW,
  POC_IORWF,
  POC_IORFW,
  POC_IORLW,
  POC_LFSR,
  POC_MOVF,
  POC_MOVFW,
  POC_MOVFF,
  POC_MOVLB,
  POC_MOVLW,
  POC_MOVWF,
  POC_MULLW,
  POC_MULWF,
  POC_NEGF,
  POC_NOP,
  POC_POP,
  POC_PUSH,
  POC_RCALL,
  POC_RETFIE,
  POC_RETLW,
  POC_RETURN,
  POC_RLCF,
  POC_RLCFW,
  POC_RLNCF,
  POC_RLNCFW,
  POC_RRCF,
  POC_RRCFW,
  POC_RRNCF,
  POC_RRNCFW,
  POC_SETF,
  POC_SUBLW,
  POC_SUBFWB,
  POC_SUBWF,
  POC_SUBFW,
  POC_SUBWFB_D0,
  POC_SUBWFB_D1,
  POC_SUBFWB_D0,
  POC_SUBFWB_D1,
  POC_SWAPF,
  POC_SWAPFW,
  POC_TBLRD,
  POC_TBLRD_POSTINC,
  POC_TBLRD_POSTDEC,
  POC_TBLRD_PREINC,
  POC_TBLWT,
  POC_TBLWT_POSTINC,
  POC_TBLWT_POSTDEC,
  POC_TBLWT_PREINC,
  POC_TSTFSZ,
  POC_XORLW,
  POC_XORWF,
  POC_XORFW,

  POC_BANKSEL

  /* pseudo-instructions */
} PIC_OPCODE;


/***********************************************************************
 *  PC_TYPE  - pCode Types
 ***********************************************************************/

typedef enum
{
  PC_COMMENT=0,   /* pCode is a comment     */
  PC_INLINE,      /* user's inline code     */
  PC_OPCODE,      /* PORT dependent opcode  */
  PC_LABEL,       /* assembly label         */
  PC_FLOW,        /* flow analysis          */
  PC_FUNCTION,    /* Function start or end  */
  PC_WILD,        /* wildcard - an opcode place holder used
                   * in the pCode peep hole optimizer */
  PC_CSOURCE,     /* C-Source Line  */
  PC_ASMDIR,      /* Assembler directive */
  PC_BAD,         /* Mark the pCode object as being bad */
  PC_INFO         /* pCode information node, used primarily in optimizing */
} PC_TYPE;


/***********************************************************************
 *  INFO_TYPE  - information node types
 ***********************************************************************/

typedef enum
{
  INF_OPTIMIZATION,      /* structure contains optimization information */
  INF_LOCALREGS          /* structure contains local register information */
} INFO_TYPE;



/***********************************************************************
 *  OPT_TYPE  - optimization node types
 ***********************************************************************/

typedef enum
{
  OPT_BEGIN,             /* mark beginning of optimization block */
  OPT_END,               /* mark ending of optimization block */
  OPT_JUMPTABLE_BEGIN,   /* mark beginning of a jumptable */
  OPT_JUMPTABLE_END      /* mark end of jumptable */
} OPT_TYPE;

/***********************************************************************
 *  LR_TYPE  - optimization node types
 ***********************************************************************/

typedef enum
{
  LR_ENTRY_BEGIN,             /* mark beginning of optimization block */
  LR_ENTRY_END,               /* mark ending of optimization block */
  LR_EXIT_BEGIN,
  LR_EXIT_END
} LR_TYPE;


/************************************************/
/***************  Structures ********************/
/************************************************/
/* These are here as forward references - the
 * full definition of these are below           */
struct pCode;
struct pCodeWildBlock;
struct pCodeRegLives;

/*************************************************
  pBranch

  The first step in optimizing pCode is determining
 the program flow. This information is stored in
 single-linked lists in the for of 'from' and 'to'
 objects with in a pcode. For example, most instructions
 don't involve any branching. So their from branch
 points to the pCode immediately preceding them and
 their 'to' branch points to the pcode immediately
 following them. A skip instruction is an example of
 a pcode that has multiple (in this case two) elements
 in the 'to' branch. A 'label' pcode is an where there
 may be multiple 'from' branches.
 *************************************************/

typedef struct pBranch
{
  struct pCode   *pc;    // Next pCode in a branch
  struct pBranch *next;  /* If more than one branch
                          * the next one is here */

} pBranch;

/*************************************************
  pCodeOp

  pCode Operand structure.
  For those assembly instructions that have arguments,
  the pCode will have a pCodeOp in which the argument
  can be stored. For example

    movf   some_register,w

  'some_register' will be stored/referenced in a pCodeOp

 *************************************************/

typedef struct pCodeOp
{
  PIC_OPTYPE type;
  char *name;

} pCodeOp;

#if 0
typedef struct pCodeOpBit
{
  pCodeOp pcop;
  int bit;
  unsigned int inBitSpace: 1; /* True if in bit space, else
                                 just a bit of a register */
} pCodeOpBit;
#endif

typedef struct pCodeOpLit
{
  pCodeOp pcop;
  int lit;
  pCodeOp *arg2;        /* needed as pCodeOpLit and pCodeOpLit2 are not separable via their type (PO_LITERAL) */
} pCodeOpLit;

typedef struct pCodeOpLit2
{
  pCodeOp pcop;
  int lit;
  pCodeOp *arg2;
} pCodeOpLit2;


typedef struct pCodeOpImmd
{
  pCodeOp pcop;
  int offset;           /* low,high or upper byte of immediate value */
  int index;            /* add this to the immediate value */
  unsigned _const:1;    /* is in code space    */

  int rIdx;             /* If this immd points to a register */
  struct reg_info *r;       /* then this is the reg. */

} pCodeOpImmd;

typedef struct pCodeOpLabel
{
  pCodeOp pcop;
  int key;
} pCodeOpLabel;

typedef struct pCodeOpReg
{
  pCodeOp pcop;    // Can be either GPR or SFR
  int rIdx;        // Index into the register table
  struct reg_info *r;
  int instance;    // byte # of Multi-byte registers
  struct pBlock *pb;
} pCodeOpReg;

typedef struct pCodeOp2
{
  pCodeOp pcop;         // describes this pCodeOp
  pCodeOp *pcopL;       // reference to left pCodeOp (src)
  pCodeOp *pcopR;       // reference to right pCodeOp (dest)
} pCodeOp2;

typedef struct pCodeOpRegBit
{
  pCodeOpReg  pcor;       // The Register containing this bit
  int bit;                // 0-7 bit number.
  PIC_OPTYPE subtype;     // The type of this register.
  unsigned int inBitSpace: 1; /* True if in bit space, else
                                 just a bit of a register */
} pCodeOpRegBit;


typedef struct pCodeOpWild
{
  pCodeOp pcop;

  struct pCodeWildBlock *pcwb;

  int id;                 /* index into an array of char *'s that will match
                           * the wild card. The array is in *pcp. */
  pCodeOp *subtype;       /* Pointer to the Operand type into which this wild
                           * card will be expanded */
  pCodeOp *matched;       /* When a wild matches, we'll store a pointer to the
                           * opcode we matched */

  pCodeOp *pcop2;         /* second operand if exists */

} pCodeOpWild;


typedef struct pCodeOpOpt
{
  pCodeOp pcop;

  OPT_TYPE type;          /* optimization node type */

  char *key;              /* key by which a block is identified */
} pCodeOpOpt;

typedef struct pCodeOpLocalReg
{
  pCodeOp pcop;

  LR_TYPE type;
} pCodeOpLocalReg;

/*************************************************
    pCode

    Here is the basic build block of a PIC instruction.
    Each pic instruction will get allocated a pCode.
    A linked list of pCodes makes a program.

**************************************************/

typedef struct pCode
{
  PC_TYPE    type;

  struct pCode *prev;  // The pCode objects are linked together
  struct pCode *next;  // in doubly linked lists.

  int seq;             // sequence number

  struct pBlock *pb;   // The pBlock that contains this pCode.

  /* "virtual functions"
   *  The pCode structure is like a base class
   * in C++. The subsequent structures that "inherit"
   * the pCode structure will initialize these function
   * pointers to something useful */
  //  void (*analyze) (struct pCode *_this);
  void (*destruct)(struct pCode *_this);
  void (*print)  (FILE *of,struct pCode *_this);

} pCode;


/*************************************************
    pCodeComment
**************************************************/

typedef struct pCodeComment
{

  pCode  pc;

  char *comment;

} pCodeComment;


/*************************************************
    pCodeCSource
**************************************************/

typedef struct pCodeCSource
{

  pCode  pc;

  int  line_number;
  char *line;
  char *file_name;

} pCodeCSource;


/*************************************************
    pCodeAsmDir
**************************************************/

/*************************************************
    pCodeFlow

  The Flow object is used as marker to separate
 the assembly code into contiguous chunks. In other
 words, everytime an instruction cause or potentially
 causes a branch, a Flow object will be inserted into
 the pCode chain to mark the beginning of the next
 contiguous chunk.

**************************************************/
struct defmap_s; // defined in pcode.c

typedef struct pCodeFlow
{

  pCode  pc;

  pCode *end;   /* Last pCode in this flow. Note that
                   the first pCode is pc.next */

  /*  set **uses;   * map the pCode instruction inCond and outCond conditions
                 * in this array of set's. The reason we allocate an
                 * array of pointers instead of declaring each type of
                 * usage is because there are port dependent usage definitions */
  //int nuses;    /* number of uses sets */

  set *from;    /* flow blocks that can send control to this flow block */
  set *to;      /* flow blocks to which this one can send control */
  struct pCodeFlow *ancestor; /* The most immediate "single" pCodeFlow object that
                               * executes prior to this one. In many cases, this
                               * will be just the previous */

  int inCond;   /* Input conditions - stuff assumed defined at entry */
  int outCond;  /* Output conditions - stuff modified by flow block */

  int firstBank; /* The first and last bank flags are the first and last */
  int lastBank;  /* register banks used within one flow object */

  int FromConflicts;
  int ToConflicts;

  set *registers;/* Registers used in this flow */

  struct defmap_s *defmap;      /* chronologically ordered list of definitions performed
                           in this flow (most recent at the front) */
  struct defmap_s *in_vals;     /* definitions of all symbols reaching this flow
                                 * symbols with multiple different definitions are stored
                                 * with an assigned value of 0. */
  struct defmap_s *out_vals;    /* definitions valid AFTER thie flow */

} pCodeFlow;

/*************************************************
  pCodeFlowLink

  The Flow Link object is used to record information
 about how consecutive excutive Flow objects are related.
 The pCodeFlow objects demarcate the pCodeInstructions
 into contiguous chunks. The FlowLink records conflicts
 in the discontinuities. For example, if one Flow object
 references a register in bank 0 and the next Flow object
 references a register in bank 1, then there is a discontinuity
 in the banking registers.

*/
typedef struct pCodeFlowLink
{
  pCodeFlow  *pcflow;   /* pointer to linked pCodeFlow object */

  int bank_conflict;    /* records bank conflicts */

} pCodeFlowLink;

/*************************************************
    pCodeInstruction

    Here we describe all the facets of a PIC instruction
    (expansion for the 18cxxx is also provided).

**************************************************/

typedef struct pCodeInstruction
{

  pCode  pc;

  PIC_OPCODE op;        // The opcode of the instruction.

  char const * const mnemonic;       // Pointer to mnemonic string

  char isize;          // pCode instruction size

  pBranch *from;       // pCodes that execute before this one
  pBranch *to;         // pCodes that execute after
  pBranch *label;      // pCode instructions that have labels

  pCodeOp *pcop;               /* Operand, if this instruction has one */
  pCodeFlow *pcflow;           /* flow block to which this instruction belongs */
  pCodeCSource *cline;         /* C Source from which this instruction was derived */

  unsigned int num_ops;        /* Number of operands (0,1,2 for mid range pics) */
  unsigned int isModReg:  1;   /* If destination is W or F, then 1==F */
  unsigned int isBitInst: 1;   /* e.g. BCF */
  unsigned int isBranch:  1;   /* True if this is a branching instruction */
  unsigned int isSkip:    1;   /* True if this is a skip instruction */
  unsigned int isLit:     1;   /* True if this instruction has an literal operand */
  unsigned int isAccess:   1;   /* True if this instruction has an access RAM operand */
  unsigned int isFastCall: 1;   /* True if this instruction has a fast call/return mode select operand */
  unsigned int is2MemOp: 1;     /* True is second operand is a memory operand VR - support for MOVFF */
  unsigned int is2LitOp: 1;     /* True if instruction takes 2 literal operands VR - support for LFSR */

  PIC_OPCODE inverted_op;      /* Opcode of instruction that's the opposite of this one */
  unsigned int inCond;   // Input conditions for this instruction
  unsigned int outCond;  // Output conditions for this instruction

#define PCI_MAGIC       0x6e12
  unsigned int pci_magic;       // sanity check for pci initialization
} pCodeInstruction;



/*************************************************
    pCodeAsmDir
**************************************************/

typedef struct pCodeAsmDir
{
  pCodeInstruction pci;

  char *directive;
  char *arg;
} pCodeAsmDir;


/*************************************************
    pCodeLabel
**************************************************/

typedef struct pCodeLabel
{

  pCode  pc;

  char *label;
  int key;
  int force;            /* label cannot be optimized out */

} pCodeLabel;

/*************************************************
    pCodeFunction
**************************************************/

typedef struct pCodeFunction
{

  pCode  pc;

  char *modname;
  char *fname;     /* If NULL, then this is the end of
                      a function. Otherwise, it's the
                      start and the name is contained
                      here */

  pBranch *from;       // pCodes that execute before this one
  pBranch *to;         // pCodes that execute after
  pBranch *label;      // pCode instructions that have labels

  int  ncalled;    /* Number of times function is called */

  int absblock;    /* hack to emulate a block pCodes in absolute position
                      but not inside a function */
  int stackusage;  /* stack positions used in function */

} pCodeFunction;


/*************************************************
    pCodeWild
**************************************************/

typedef struct pCodeWild
{

  pCodeInstruction  pci;

  int    id;     /* Index into the wild card array of a peepBlock
                  * - this wild card will get expanded into that pCode
                  *   that is stored at this index */

  /* Conditions on wild pcode instruction */
  int    mustBeBitSkipInst:1;
  int    mustNotBeBitSkipInst:1;
  int    invertBitSkipInst:1;

  pCodeOp *operand;  // Optional operand
  pCodeOp *label;    // Optional label

} pCodeWild;


/*************************************************
    pInfo

    Here are stored generic informaton
*************************************************/
typedef struct pCodeInfo
{
  pCodeInstruction pci;

  INFO_TYPE type;       /* info node type */

  pCodeOp *oper1;       /* info node arguments */
} pCodeInfo;


/*************************************************
    pBlock

    Here are PIC program snippets. There's a strong
    correlation between the eBBlocks and pBlocks.
    SDCC subdivides a C program into managable chunks.
    Each chunk becomes a eBBlock and ultimately in the
    PIC port a pBlock.

**************************************************/

typedef struct pBlock
{
  memmap *cmemmap;   /* The snippet is from this memmap */
  char   dbName;     /* if cmemmap is NULL, then dbName will identify the block */
  pCode *pcHead;     /* A pointer to the first pCode in a link list of pCodes */
  pCode *pcTail;     /* A pointer to the last pCode in a link list of pCodes */

  struct pBlock *next;      /* The pBlocks will form a doubly linked list */
  struct pBlock *prev;

  set *function_entries;    /* dll of functions in this pblock */
  set *function_exits;
  set *function_calls;
  set *tregisters;

  set *FlowTree;
  unsigned visited:1;       /* set true if traversed in call tree */

  unsigned seq;             /* sequence number of this pBlock */

} pBlock;

/*************************************************
    pFile

    The collection of pBlock program snippets are
    placed into a linked list that is implemented
    in the pFile structure.

    The pcode optimizer will parse the pFile.

**************************************************/

typedef struct pFile
{
  pBlock *pbHead;     /* A pointer to the first pBlock */
  pBlock *pbTail;     /* A pointer to the last pBlock */

  pBranch *functions; /* A SLL of functions in this pFile */

} pFile;



/*************************************************
  pCodeWildBlock

  The pCodeWildBlock object keeps track of the wild
  variables, operands, and opcodes that exist in
  a pBlock.
**************************************************/
typedef struct pCodeWildBlock {
  pBlock    *pb;
  struct pCodePeep *pcp;    // pointer back to ... I don't like this...

  int       nvars;          // Number of wildcard registers in target.
  char    **vars;           // array of pointers to them

  int       nops;           // Number of wildcard operands in target.
  pCodeOp **wildpCodeOps;   // array of pointers to the pCodeOp's.

  int       nwildpCodes;    // Number of wildcard pCodes in target/replace
  pCode   **wildpCodes;     // array of pointers to the pCode's.

} pCodeWildBlock;

/*************************************************
  pCodePeep

  The pCodePeep object mimics the peep hole optimizer
  in the main SDCC src (e.g. SDCCpeeph.c). Essentially
  there is a target pCode chain and a replacement
  pCode chain. The target chain is compared to the
  pCode that is generated by gen.c. If a match is
  found then the pCode is replaced by the replacement
  pCode chain.
**************************************************/
typedef struct pCodePeep {
  pCodeWildBlock target;     // code we'd like to optimize
  pCodeWildBlock replace;    // and this is what we'll optimize it with.

  //pBlock *target;
  //pBlock replace;            // and this is what we'll optimize it with.



  /* (Note: a wildcard register is a place holder. Any register
   * can be replaced by the wildcard when the pcode is being
   * compared to the target. */

  /* Post Conditions. A post condition is a condition that
   * must be either true or false before the peep rule is
   * accepted. For example, a certain rule may be accepted
   * if and only if the Z-bit is not used as an input to
   * the subsequent instructions in a pCode chain.
   */
  unsigned int postFalseCond;
  unsigned int postTrueCond;

} pCodePeep;

/*************************************************

  pCode peep command definitions

 Here are some special commands that control the
way the peep hole optimizer behaves

**************************************************/

enum peepCommandTypes{
  NOTBITSKIP = 0,
  BITSKIP,
  INVERTBITSKIP,
  _LAST_PEEP_COMMAND_
};

/*************************************************
    peepCommand structure stores the peep commands.

**************************************************/

typedef struct peepCommand {
  int id;
  char *cmd;
} peepCommand;

/*************************************************
    pCode Macros

**************************************************/
#define PCODE(x)  ((pCode *)(x))
#define PCI(x)    ((pCodeInstruction *)(x))
#define PCL(x)    ((pCodeLabel *)(x))
#define PCF(x)    ((pCodeFunction *)(x))
#define PCFL(x)   ((pCodeFlow *)(x))
#define PCFLINK(x)((pCodeFlowLink *)(x))
#define PCW(x)    ((pCodeWild *)(x))
#define PCCS(x)   ((pCodeCSource *)(x))
#define PCAD(x)   ((pCodeAsmDir *)(x))
#define PCINF(x)  ((pCodeInfo *)(x))

#define PCOP(x)   ((pCodeOp *)(x))
#define PCOP2(x)  ((pCodeOp2 *)(x))
//#define PCOB(x)   ((pCodeOpBit *)(x))
#define PCOL(x)   ((pCodeOpLit *)(x))
#define PCOI(x)   ((pCodeOpImmd *)(x))
#define PCOLAB(x) ((pCodeOpLabel *)(x))
#define PCOR(x)   ((pCodeOpReg *)(x))
//#define PCOR2(x)  ((pCodeOpReg2 *)(x))
#define PCORB(x)  ((pCodeOpRegBit *)(x))
#define PCOO(x)   ((pCodeOpOpt *)(x))
#define PCOLR(x)  ((pCodeOpLocalReg *)(x))
#define PCOW(x)   ((pCodeOpWild *)(x))
#define PCOW2(x)  (PCOW(PCOW(x)->pcop2))
#define PBR(x)    ((pBranch *)(x))

#define PCWB(x)   ((pCodeWildBlock *)(x))


/*
  macros for checking pCode types
*/
#define isPCI(x)        ((PCODE(x)->type == PC_OPCODE))
#define isPCI_BRANCH(x) ((PCODE(x)->type == PC_OPCODE) &&  PCI(x)->isBranch)
#define isPCI_SKIP(x)   ((PCODE(x)->type == PC_OPCODE) &&  PCI(x)->isSkip)
#define isPCI_LIT(x)    ((PCODE(x)->type == PC_OPCODE) &&  PCI(x)->isLit)
#define isPCI_BITSKIP(x)((PCODE(x)->type == PC_OPCODE) &&  PCI(x)->isSkip && PCI(x)->isBitInst)
#define isPCFL(x)       ((PCODE(x)->type == PC_FLOW))
#define isPCF(x)        ((PCODE(x)->type == PC_FUNCTION))
#define isPCL(x)        ((PCODE(x)->type == PC_LABEL))
#define isPCW(x)        ((PCODE(x)->type == PC_WILD))
#define isPCCS(x)       ((PCODE(x)->type == PC_CSOURCE))
#define isPCAD(x)       ((PCODE(x)->type == PC_ASMDIR))
#define isPCINFO(x)     ((PCODE(x)->type == PC_INFO))

#define isCALL(x)       ((isPCI(x)) && (PCI(x)->op == POC_CALL))
#define isSTATUS_REG(r) ((r)->pc_type == PO_STATUS)
#define isBSR_REG(r)    ((r)->pc_type == PO_BSR)
#define isACCESS_BANK(r)        (r->accessBank)



#define isPCOLAB(x)     ((PCOP(x)->type) == PO_LABEL)

/*-----------------------------------------------------------------*
 * pCode functions.
 *-----------------------------------------------------------------*/

pCode *pic16_newpCode (PIC_OPCODE op, pCodeOp *pcop); // Create a new pCode given an operand
pCode *pic16_newpCodeCharP(char *cP);              // Create a new pCode given a char *
pCode *pic16_newpCodeInlineP(char *cP);            // Create a new pCode given a char *
pCode *pic16_newpCodeFunction(const char *g, const char *f); // Create a new function
pCode *pic16_newpCodeLabel(char *name,int key);    // Create a new label given a key
pCode *pic16_newpCodeLabelFORCE(char *name, int key); // Same as newpCodeLabel but label cannot be optimized out
pCode *pic16_newpCodeCSource(int ln, const char *f, const char *l); // Create a new symbol line
pBlock *pic16_newpCodeChain(memmap *cm,char c, pCode *pc); // Create a new pBlock
void pic16_printpBlock(FILE *of, pBlock *pb);      // Write a pBlock to a file
void pic16_addpCode2pBlock(pBlock *pb, pCode *pc); // Add a pCode to a pBlock
void pic16_addpBlock(pBlock *pb);                  // Add a pBlock to a pFile
void pic16_copypCode(FILE *of, char dbName);       // Write all pBlocks with dbName to *of
void pic16_movepBlock2Head(char dbName);           // move pBlocks around
void pic16_AnalyzepCode(char dbName);
void pic16_OptimizeLocalRegs(void);
void pic16_AssignRegBanks(void);
void pic16_printCallTree(FILE *of);
void pCodePeepInit(void);
void pic16_pBlockConvert2ISR(pBlock *pb);
void pic16_pBlockConvert2Absolute(pBlock *pb);
void pic16_initDB(void);
void pic16_emitDB(int c, char ptype, void *p);            // Add DB directives to a pBlock
void pic16_emitDS(char *s, char ptype, void *p);
void pic16_flushDB(char ptype, void *p);                          // Add pending DB data to a pBlock

pCode *pic16_newpCodeAsmDir(char *asdir, char *argfmt, ...);

pCodeOp *pic16_newpCodeOpLabel(char *name, int key);
pCodeOp *pic16_newpCodeOpImmd(char *name, int offset, int index, int code_space);
pCodeOp *pic16_newpCodeOpLit(int lit);
pCodeOp *pic16_newpCodeOpLit12(int lit);
pCodeOp *pic16_newpCodeOpLit2(int lit, pCodeOp *arg2);
pCodeOp *pic16_newpCodeOpBit(char *name, int bit,int inBitSpace, PIC_OPTYPE subt);
pCodeOp *pic16_newpCodeOpBit_simple (struct asmop *op, int offs, int bit);
pCodeOp *pic16_newpCodeOpRegFromStr(char *name);
pCodeOp *pic16_newpCodeOpReg(int rIdx);
pCodeOp *pic16_newpCodeOp(char *name, PIC_OPTYPE p);
pCodeOp *pic16_newpCodeOp2(pCodeOp *src, pCodeOp *dst);
pCodeOp *pic16_newpCodeOpRegNotVect(bitVect *bv);
pCodeOp *pic16_pCodeOpCopy(pCodeOp *pcop);

pCode *pic16_newpCodeInfo(INFO_TYPE type, pCodeOp *pcop);
pCodeOp *pic16_newpCodeOpOpt(OPT_TYPE type, char *key);
pCodeOp *pic16_newpCodeOpLocalRegs(LR_TYPE type);
pCodeOp *pic16_newpCodeOpReg(int rIdx);

pCode * pic16_findNextInstruction(pCode *pci);
pCode * pic16_findNextpCode(pCode *pc, PC_TYPE pct);
int pic16_isPCinFlow(pCode *pc, pCode *pcflow);
struct reg_info * pic16_getRegFromInstruction(pCode *pc);
struct reg_info * pic16_getRegFromInstruction2(pCode *pc);
char *pic16_get_op(pCodeOp *pcop,char *buffer, size_t size);
char *pic16_get_op2(pCodeOp *pcop,char *buffer, size_t size);
char *dumpPicOptype(PIC_OPTYPE type);

extern void pic16_pcode_test(void);
extern int pic16_debug_verbose;
extern int pic16_pcode_verbose;

extern char *LR_TYPE_STR[];


#ifndef debugf
//#define debugf(frm, rest...)       _debugf(__FILE__, __LINE__, frm, rest)
#define debugf(frm, rest)       _debugf(__FILE__, __LINE__, frm, rest)
#define debugf2(frm, arg1, arg2)        _debugf(__FILE__, __LINE__, frm, arg1, arg2)
#define debugf3(frm, arg1, arg2, arg3)  _debugf(__FILE__, __LINE__, frm, arg1, arg2, arg3)

#endif

extern void _debugf(char *f, int l, char *frm, ...);


/*-----------------------------------------------------------------*
 * pCode objects.
 *-----------------------------------------------------------------*/

extern pCodeOpReg pic16_pc_status;
extern pCodeOpReg pic16_pc_intcon;
extern pCodeOpReg pic16_pc_pcl;
extern pCodeOpReg pic16_pc_pclath;
extern pCodeOpReg pic16_pc_pclatu;
extern pCodeOpReg pic16_pc_wreg;
extern pCodeOpReg pic16_pc_tosl;
extern pCodeOpReg pic16_pc_tosh;
extern pCodeOpReg pic16_pc_tosu;
extern pCodeOpReg pic16_pc_tblptrl;
extern pCodeOpReg pic16_pc_tblptrh;
extern pCodeOpReg pic16_pc_tblptru;
extern pCodeOpReg pic16_pc_tablat;
extern pCodeOpReg pic16_pc_bsr;
extern pCodeOpReg pic16_pc_fsr0;
extern pCodeOpReg pic16_pc_fsr0l;
extern pCodeOpReg pic16_pc_fsr0h;
extern pCodeOpReg pic16_pc_fsr1l;
extern pCodeOpReg pic16_pc_fsr1h;
extern pCodeOpReg pic16_pc_fsr2l;
extern pCodeOpReg pic16_pc_fsr2h;
extern pCodeOpReg pic16_pc_indf0;
extern pCodeOpReg pic16_pc_postinc0;
extern pCodeOpReg pic16_pc_postdec0;
extern pCodeOpReg pic16_pc_preinc0;
extern pCodeOpReg pic16_pc_plusw0;
extern pCodeOpReg pic16_pc_indf1;
extern pCodeOpReg pic16_pc_postinc1;
extern pCodeOpReg pic16_pc_postdec1;
extern pCodeOpReg pic16_pc_preinc1;
extern pCodeOpReg pic16_pc_plusw1;
extern pCodeOpReg pic16_pc_indf2;
extern pCodeOpReg pic16_pc_postinc2;
extern pCodeOpReg pic16_pc_postdec2;
extern pCodeOpReg pic16_pc_preinc2;
extern pCodeOpReg pic16_pc_plusw2;
extern pCodeOpReg pic16_pc_prodl;
extern pCodeOpReg pic16_pc_prodh;

extern pCodeOpReg pic16_pc_eecon1;
extern pCodeOpReg pic16_pc_eecon2;
extern pCodeOpReg pic16_pc_eedata;
extern pCodeOpReg pic16_pc_eeadr;

extern pCodeOpReg pic16_pc_kzero;
extern pCodeOpReg pic16_pc_wsave;     /* wsave and ssave are used to save W and the Status */
extern pCodeOpReg pic16_pc_ssave;     /* registers during an interrupt */

extern pCodeOpReg *pic16_stackpnt_lo;
extern pCodeOpReg *pic16_stackpnt_hi;
extern pCodeOpReg *pic16_stack_postinc;
extern pCodeOpReg *pic16_stack_postdec;
extern pCodeOpReg *pic16_stack_preinc;
extern pCodeOpReg *pic16_stack_plusw;

extern pCodeOpReg *pic16_framepnt_lo;
extern pCodeOpReg *pic16_framepnt_hi;
extern pCodeOpReg *pic16_frame_postinc;
extern pCodeOpReg *pic16_frame_postdec;
extern pCodeOpReg *pic16_frame_preinc;
extern pCodeOpReg *pic16_frame_plusw;

extern pCodeOpReg pic16_pc_gpsimio;
extern pCodeOpReg pic16_pc_gpsimio2;

#endif // __PCODE_H__
