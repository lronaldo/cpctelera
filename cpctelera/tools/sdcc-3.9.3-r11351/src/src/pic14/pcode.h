/*-------------------------------------------------------------------------

   pcode.h - post code generation
   Written By -  Scott Dattalo scott@dattalo.com

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

#ifndef __PCODE_H__
#define __PCODE_H__

#include "common.h"

/* When changing these, you must also update the assembler template
 * in device/lib/libsdcc/macros.inc */
#define GPTRTAG_DATA	0x00
#define GPTRTAG_CODE	0x80

/* Cyclic dependency with ralloc.h: */
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
#define DFPRINTF(args) ((void)0)
#endif


/***********************************************************************
 *  PIC status bits - this will move into device dependent headers
 ***********************************************************************/
#define PIC_C_BIT    0
#define PIC_DC_BIT   1
#define PIC_Z_BIT    2
#define PIC_RP0_BIT  5   /* Register Bank select bits RP1:0 : */
#define PIC_RP1_BIT  6   /* 00 - bank 0, 01 - bank 1, 10 - bank 2, 11 - bank 3 */
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
	PO_W,              // The 'W' register
	PO_STATUS,         // The 'STATUS' register
	PO_FSR,            // The "file select register" (in 18c it's one of three)
	PO_INDF,           // The Indirect register
	PO_INTCON,         // Interrupt Control register
	PO_GPR_REGISTER,   // A general purpose register
	PO_GPR_BIT,        // A bit of a general purpose register
	PO_GPR_TEMP,       // A general purpose temporary register
	PO_GPR_POINTER,    // A general purpose pointer
	PO_SFR_REGISTER,   // A special function register (e.g. PORTA)
	PO_PCL,            // Program counter Low register
	PO_PCLATH,         // Program counter Latch high register
	PO_LITERAL,        // A constant
	PO_IMMEDIATE,      //  (8051 legacy)
	PO_DIR,            // Direct memory (8051 legacy)
	PO_CRY,            // bit memory (8051 legacy)
	PO_BIT,            // bit operand.
	PO_STR,            //  (8051 legacy)
	PO_LABEL,
	PO_WILD            // Wild card operand in peep optimizer
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
	POC_ANDLW,
	POC_ANDWF,
	POC_ANDFW,
	POC_BCF,
	POC_BSF,
	POC_BTFSC,
	POC_BTFSS,
	POC_CALL,
	POC_COMF,
	POC_COMFW,
	POC_CLRF,
	POC_CLRW,
	POC_CLRWDT,
	POC_DECF,
	POC_DECFW,
	POC_DECFSZ,
	POC_DECFSZW,
	POC_GOTO,
	POC_INCF,
	POC_INCFW,
	POC_INCFSZ,
	POC_INCFSZW,
	POC_IORLW,
	POC_IORWF,
	POC_IORFW,
	POC_MOVF,
	POC_MOVFW,
	POC_MOVLW,
	POC_MOVWF,
	POC_NOP,
	POC_RETLW,
	POC_RETURN,
	POC_RETFIE,
	POC_RLF,
	POC_RLFW,
	POC_RRF,
	POC_RRFW,
	POC_SUBLW,
	POC_SUBWF,
	POC_SUBFW,
	POC_SWAPF,
	POC_SWAPFW,
	POC_TRIS,
	POC_XORLW,
	POC_XORWF,
	POC_XORFW,
	POC_BANKSEL,
	POC_PAGESEL,

	/* Enhanced instruction set. */

	POC_ADDFSR,
	POC_ADDWFC,
	POC_ADDFWC,
	POC_ASRF,
	POC_ASRFW,
	POC_BRA,
	POC_BRW,
	POC_CALLW,
	POC_LSLF,
	POC_LSLFW,
	POC_LSRF,
	POC_LSRFW,
	POC_MOVIW,
	POC_MOVIW_K,
	POC_MOVLB,
	POC_MOVLP,
	POC_MOVWI,
	POC_MOVWI_K,
	POC_RESET,
	POC_SUBWFB,
	POC_SUBWFBW,

	MAX_PIC14MNEMONICS
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
	PC_BAD          /* Mark the pCode object as being bad */
} PC_TYPE;

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

typedef struct pCodeOpLit
{
	pCodeOp pcop;
	int lit;
} pCodeOpLit;

typedef struct pCodeOpImmd
{
	pCodeOp pcop;
	int offset;           /* low,med, or high byte of immediate value */
	int index;            /* add this to the immediate value */
	unsigned _const:1;    /* is in code space    */
	unsigned _function:1; /* is a (pointer to a) function */

	int rIdx;             /* If this immd points to a register */
	struct reg_info *r;       /* then this is the reg. */

} pCodeOpImmd;

typedef struct pCodeOpLabel
{
	pCodeOp pcop;
	int key;
	int offset;           /* low or high byte of label */
} pCodeOpLabel;

typedef struct pCodeOpReg
{
	pCodeOp pcop;    // Can be either GPR or SFR
	int rIdx;        // Index into the register table
	struct reg_info *r;
	int instance;    // byte # of Multi-byte registers
	struct pBlock *pb;
} pCodeOpReg;

typedef struct pCodeOpRegBit
{
	pCodeOpReg  pcor;       // The Register containing this bit
	int bit;                // 0-7 bit number.
	PIC_OPTYPE subtype;     // The type of this register.
	unsigned int inBitSpace: 1; /* True if in bit space, else
	                            just a bit of a register */
} pCodeOpRegBit;

typedef struct pCodeOpStr /* Only used here for the name of fn being called or jumped to */
{
	pCodeOp  pcop;
	unsigned isPublic: 1; /* True if not static ie extern */
} pCodeOpStr;

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

} pCodeOpWild;


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

	unsigned id;         // unique ID number for all pCodes to assist in debugging
	int seq;             // sequence number

	struct pBlock *pb;   // The pBlock that contains this pCode.

	/* "virtual functions"
	 *  The pCode structure is like a base class
	 * in C++. The subsequent structures that "inherit"
	 * the pCode structure will initialize these function
	 * pointers to something useful */
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
    pCodeComment
**************************************************/

typedef struct pCodeCSource
{
	pCode  pc;

	int  line_number;
	char *line;
	char *file_name;

} pCodeCSource;


/*************************************************
    pCodeFlow

  The Flow object is used as marker to separate 
 the assembly code into contiguous chunks. In other
 words, everytime an instruction cause or potentially
 causes a branch, a Flow object will be inserted into
 the pCode chain to mark the beginning of the next
 contiguous chunk.

**************************************************/

typedef struct pCodeFlow
{
	pCode  pc;

	pCode *end;   /* Last pCode in this flow. Note that
	                 the first pCode is pc.next */

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

	PIC_OPCODE inverted_op;      /* Opcode of instruction that's the opposite of this one */
	unsigned int inCond;   // Input conditions for this instruction
	unsigned int outCond;  // Output conditions for this instruction

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
	                    here. */

	pBranch *from;       // pCodes that execute before this one
	pBranch *to;         // pCodes that execute after
	pBranch *label;      // pCode instructions that have labels

	int  ncalled;        /* Number of times function is called. */
	unsigned isPublic:1; /* True if the fn is not static and can be called from another module (ie a another c or asm file). */
	unsigned isInterrupt:1; /* True if the fn is interrupt. */

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
typedef struct pCodeWildBlock
{
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
typedef struct pCodePeep
{
	pCodeWildBlock target;     // code we'd like to optimize
	pCodeWildBlock replace;    // and this is what we'll optimize it with.

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

enum peepCommandTypes
{
	NOTBITSKIP = 0,
	BITSKIP,
	INVERTBITSKIP,
	_LAST_PEEP_COMMAND_
};

/*************************************************
    peepCommand structure stores the peep commands.

**************************************************/

typedef struct peepCommand
{
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
#define PCAD(x)	  ((pCodeAsmDir *)(x))

#define PCOP(x)   ((pCodeOp *)(x))
#define PCOL(x)   ((pCodeOpLit *)(x))
#define PCOI(x)   ((pCodeOpImmd *)(x))
#define PCOLAB(x) ((pCodeOpLabel *)(x))
#define PCOR(x)   ((pCodeOpReg *)(x))
#define PCORB(x)  ((pCodeOpRegBit *)(x))
#define PCOS(x)   ((pCodeOpStr *)(x))
#define PCOW(x)   ((pCodeOpWild *)(x))

#define PBR(x)    ((pBranch *)(x))

#define PCWB(x)   ((pCodeWildBlock *)(x))

#define isPCOLAB(x)     ((PCOP(x)->type) == PO_LABEL)
#define isPCOS(x)       ((PCOP(x)->type) == PO_STR)


/*
  macros for checking pCode types
*/
#define isPCI(x)        ((PCODE(x)->type == PC_OPCODE))
#define isPCFL(x)       ((PCODE(x)->type == PC_FLOW))
#define isPCF(x)        ((PCODE(x)->type == PC_FUNCTION))
#define isPCL(x)        ((PCODE(x)->type == PC_LABEL))
#define isPCW(x)        ((PCODE(x)->type == PC_WILD))
#define isPCCS(x)       ((PCODE(x)->type == PC_CSOURCE))
#define isPCASMDIR(x)	((PCODE(x)->type == PC_ASMDIR))

/*
  macros for checking pCodeInstruction types
*/
#define isCALL(x)       (isPCI(x) && (PCI(x)->op == POC_CALL))
#define isPCI_BRANCH(x) (isPCI(x) &&  PCI(x)->isBranch)
#define isPCI_SKIP(x)   (isPCI(x) &&  PCI(x)->isSkip)
#define isPCI_LIT(x)    (isPCI(x) &&  PCI(x)->isLit)
#define isPCI_BITSKIP(x)(isPCI_SKIP(x) && PCI(x)->isBitInst)


#define isSTATUS_REG(r) ((r)->pc_type == PO_STATUS)

/*-----------------------------------------------------------------*
 * pCode functions.
 *-----------------------------------------------------------------*/

pCode *newpCode(PIC_OPCODE op, pCodeOp *pcop); // Create a new pCode given an operand
pCode *newpCodeCharP(const char *cP);              // Create a new pCode given a char *
pCode *newpCodeFunction(const char *g, const char *f, int, int); // Create a new function.
pCode *newpCodeLabel(const char *name,int key);    // Create a new label given a key
pCode *newpCodeCSource(int ln, const char *f, const char *l); // Create a new symbol line.
pCode *newpCodeWild(int pCodeID, pCodeOp *optional_operand, pCodeOp *optional_label);
pCode *findNextInstruction(pCode *pci);
pCode *findPrevInstruction(pCode *pci);
pCode *findNextpCode(pCode *pc, PC_TYPE pct);
pCode *pCodeInstructionCopy(pCodeInstruction *pci,int invert);

pBlock *newpCodeChain(memmap *cm,char c, pCode *pc); // Create a new pBlock
void printpBlock(FILE *of, pBlock *pb);      // Write a pBlock to a file
void printpCode(FILE *of, pCode *pc);        // Write a pCode to a file
void addpCode2pBlock(pBlock *pb, pCode *pc); // Add a pCode to a pBlock
void addpBlock(pBlock *pb);                  // Add a pBlock to a pFile
void unlinkpCode(pCode *pc);
void copypCode(FILE *of, char dbName);       // Write all pBlocks with dbName to *of
void movepBlock2Head(char dbName);           // move pBlocks around
void AnalyzeBanking(void);
void ReuseReg(void);
void AnalyzepCode(char dbName);
void InlinepCode(void);
void pCodeInitRegisters(void);
void pic14initpCodePeepCommands(void);
void pBlockConvert2ISR(pBlock *pb);
void pBlockMergeLabels(pBlock *pb);
void pCodeInsertAfter(pCode *pc1, pCode *pc2);
void pCodeInsertBefore(pCode *pc1, pCode *pc2);
void pCodeDeleteChain(pCode *f,pCode *t);

pCode *newpCodeAsmDir(const char *asdir, const char *argfmt, ...); 

pCodeOp *newpCodeOpLabel(const char *name, int key);
pCodeOp *newpCodeOpImmd(const char *name, int offset, int index, int code_space,int is_func);
pCodeOp *newpCodeOpLit(int lit);
pCodeOp *newpCodeOpBit(const char *name, int bit,int inBitSpace);
pCodeOp *newpCodeOpWild(int id, pCodeWildBlock *pcwb, pCodeOp *subtype);
pCodeOp *newpCodeOpRegFromStr(const char *name);
pCodeOp *newpCodeOp(const char *name, PIC_OPTYPE p);
pCodeOp *pCodeOpCopy(pCodeOp *pcop);
pCodeOp *popCopyGPR2Bit(pCodeOp *pc, int bitval);
pCodeOp *popCopyReg(pCodeOpReg *pc);

pBranch *pBranchAppend(pBranch *h, pBranch *n);

struct reg_info * getRegFromInstruction(pCode *pc);

char *get_op(pCodeOp *pcop, char *buff, size_t buf_size);
char *pCode2str(char *str, size_t size, pCode *pc);

int pCodePeepMatchRule(pCode *pc);

void pcode_test(void);
void resetpCodeStatistics (void);
void dumppCodeStatistics (FILE *of);

/*-----------------------------------------------------------------*
 * pCode objects.
 *-----------------------------------------------------------------*/

extern pCodeOpReg pc_status;
extern pCodeOpReg pc_intcon;
extern pCodeOpReg pc_fsr;
extern pCodeOpReg pc_fsr0l;
extern pCodeOpReg pc_fsr0h;
extern pCodeOpReg *pc_indf; /* pointer to either pc_indf_ or pc_indf0 */
extern pCodeOpReg pc_indf_;
extern pCodeOpReg pc_indf0;
extern pCodeOpReg pc_pcl;
extern pCodeOpReg pc_pclath;
extern pCodeOpReg pc_wsave;     /* wsave, ssave and psave are used to save W, the Status and PCLATH*/
extern pCodeOpReg pc_ssave;     /* registers during an interrupt */
extern pCodeOpReg pc_psave;     /* registers during an interrupt */

extern pFile *the_pFile;
extern pCodeInstruction *pic14Mnemonics[MAX_PIC14MNEMONICS];

/*
 * From pcodepeep.h:
 */
int getpCode(const char *mnem, unsigned dest);
int getpCodePeepCommand(const char *cmd);
int pCodeSearchCondition(pCode *pc, unsigned int cond, int contIfSkip);

#endif // __PCODE_H__

