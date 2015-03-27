/*-------------------------------------------------------------------------
  pcode.c - post code generation

  Copyright (C) 2000, Scott Dattalo scott@dattalo.com
  PIC16 port:
  Copyright (C) 2002, Martin Dubuc m.dubuc@rogers.com

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

#include <stdio.h>

#include "common.h"   // Include everything in the SDCC src directory
#include "newalloc.h"


#include "main.h"
#include "pcode.h"
#include "pcodeflow.h"
#include "ralloc.h"
#include "device.h"

extern char *pic16_aopGet (struct asmop *aop, int offset, bool bit16, bool dname);

#if defined(__BORLANDC__) || defined(_MSC_VER)
#define inline
#endif

#define DUMP_DF_GRAPHS 0

/****************************************************************/
/****************************************************************/

static peepCommand peepCommands[] = {

  {NOTBITSKIP, "_NOTBITSKIP_"},
  {BITSKIP, "_BITSKIP_"},
  {INVERTBITSKIP, "_INVERTBITSKIP_"},

  {-1, NULL}
};



// Eventually this will go into device dependent files:
pCodeOpReg pic16_pc_status    = {{PO_STATUS,  "STATUS"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_intcon    = {{PO_INTCON,  "INTCON"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_pcl       = {{PO_PCL,     "PCL"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_pclath    = {{PO_PCLATH,  "PCLATH"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_pclatu    = {{PO_PCLATU,  "PCLATU"}, -1, NULL,0,NULL}; // patch 14
pCodeOpReg pic16_pc_wreg      = {{PO_WREG,    "WREG"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_bsr       = {{PO_BSR,     "BSR"}, -1, NULL,0,NULL};

pCodeOpReg pic16_pc_tosl      = {{PO_SFR_REGISTER,   "TOSL"}, -1, NULL,0,NULL}; // patch 14
pCodeOpReg pic16_pc_tosh      = {{PO_SFR_REGISTER,   "TOSH"}, -1, NULL,0,NULL}; //
pCodeOpReg pic16_pc_tosu      = {{PO_SFR_REGISTER,   "TOSU"}, -1, NULL,0,NULL}; // patch 14

pCodeOpReg pic16_pc_tblptrl   = {{PO_SFR_REGISTER,   "TBLPTRL"}, -1, NULL,0,NULL}; // patch 15
pCodeOpReg pic16_pc_tblptrh   = {{PO_SFR_REGISTER,   "TBLPTRH"}, -1, NULL,0,NULL}; //
pCodeOpReg pic16_pc_tblptru   = {{PO_SFR_REGISTER,   "TBLPTRU"}, -1, NULL,0,NULL}; //
pCodeOpReg pic16_pc_tablat    = {{PO_SFR_REGISTER,   "TABLAT"}, -1, NULL,0,NULL};  // patch 15

//pCodeOpReg pic16_pc_fsr0      = {{PO_FSR0,    "FSR0"}, -1, NULL,0,NULL}; //deprecated !

pCodeOpReg pic16_pc_fsr0l       = {{PO_FSR0,    "FSR0L"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_fsr0h       = {{PO_FSR0,    "FSR0H"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_fsr1l       = {{PO_FSR0,    "FSR1L"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_fsr1h       = {{PO_FSR0,    "FSR1H"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_fsr2l       = {{PO_FSR0,    "FSR2L"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_fsr2h       = {{PO_FSR0,    "FSR2H"}, -1, NULL, 0, NULL};

pCodeOpReg pic16_pc_indf0       = {{PO_INDF0,   "INDF0"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_postinc0    = {{PO_INDF0,   "POSTINC0"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_postdec0    = {{PO_INDF0,   "POSTDEC0"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_preinc0     = {{PO_INDF0,   "PREINC0"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_plusw0      = {{PO_INDF0,   "PLUSW0"}, -1, NULL, 0, NULL};

pCodeOpReg pic16_pc_indf1       = {{PO_INDF0,   "INDF1"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_postinc1    = {{PO_INDF0,   "POSTINC1"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_postdec1    = {{PO_INDF0,   "POSTDEC1"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_preinc1     = {{PO_INDF0,   "PREINC1"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_plusw1      = {{PO_INDF0,   "PLUSW1"}, -1, NULL, 0, NULL};

pCodeOpReg pic16_pc_indf2       = {{PO_INDF0,   "INDF2"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_postinc2    = {{PO_INDF0,   "POSTINC2"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_postdec2    = {{PO_INDF0,   "POSTDEC2"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_preinc2     = {{PO_INDF0,   "PREINC2"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_plusw2      = {{PO_INDF0,   "PLUSW2"}, -1, NULL, 0, NULL};

pCodeOpReg pic16_pc_prodl       = {{PO_PRODL, "PRODL"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_prodh       = {{PO_PRODH, "PRODH"}, -1, NULL, 0, NULL};

/* EEPROM registers */
pCodeOpReg pic16_pc_eecon1      = {{PO_SFR_REGISTER, "EECON1"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_eecon2      = {{PO_SFR_REGISTER, "EECON2"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_eedata      = {{PO_SFR_REGISTER, "EEDATA"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_eeadr       = {{PO_SFR_REGISTER, "EEADR"}, -1, NULL, 0, NULL};

pCodeOpReg pic16_pc_kzero     = {{PO_GPR_REGISTER,  "KZ"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_wsave     = {{PO_GPR_REGISTER,  "WSAVE"}, -1, NULL,0,NULL};
pCodeOpReg pic16_pc_ssave     = {{PO_GPR_REGISTER,  "SSAVE"}, -1, NULL,0,NULL};

pCodeOpReg *pic16_stackpnt_lo;
pCodeOpReg *pic16_stackpnt_hi;
pCodeOpReg *pic16_stack_postinc;
pCodeOpReg *pic16_stack_postdec;
pCodeOpReg *pic16_stack_preinc;
pCodeOpReg *pic16_stack_plusw;

pCodeOpReg *pic16_framepnt_lo;
pCodeOpReg *pic16_framepnt_hi;
pCodeOpReg *pic16_frame_postinc;
pCodeOpReg *pic16_frame_postdec;
pCodeOpReg *pic16_frame_preinc;
pCodeOpReg *pic16_frame_plusw;

pCodeOpReg pic16_pc_gpsimio   = {{PO_GPR_REGISTER, "GPSIMIO"}, -1, NULL, 0, NULL};
pCodeOpReg pic16_pc_gpsimio2  = {{PO_GPR_REGISTER, "GPSIMIO2"}, -1, NULL, 0, NULL};

char *OPT_TYPE_STR[] = { "begin", "end", "jumptable_begin", "jumptable_end" };
char *LR_TYPE_STR[] = { "entry begin", "entry end", "exit begin", "exit end" };


static int mnemonics_initialized = 0;


static hTab *pic16MnemonicsHash = NULL;
static hTab *pic16pCodePeepCommandsHash = NULL;

static pFile *the_pFile = NULL;
static pBlock *pb_dead_pcodes = NULL;

/* Hardcoded flags to change the behavior of the PIC port */
static int peepOptimizing = 1;        /* run the peephole optimizer if nonzero */
static int functionInlining = 1;      /* inline functions if nonzero */
int pic16_debug_verbose = 0;                /* Set true to inundate .asm file */

int pic16_pcode_verbose = 0;

//static int GpCodeSequenceNumber = 1;
static int GpcFlowSeq = 1;

extern void pic16_RemoveUnusedRegisters(void);
extern void pic16_RegsUnMapLiveRanges(void);
extern void pic16_BuildFlowTree(pBlock *pb);
extern void pic16_pCodeRegOptimizeRegUsage(int level);

/****************************************************************/
/*                      Forward declarations                    */
/****************************************************************/

void pic16_unlinkpCode(pCode *pc);
#if 0
static void genericAnalyze(pCode *pc);
static void AnalyzeGOTO(pCode *pc);
static void AnalyzeSKIP(pCode *pc);
static void AnalyzeRETURN(pCode *pc);
#endif

static void genericDestruct(pCode *pc);
static void genericPrint(FILE *of,pCode *pc);

static void pCodePrintLabel(FILE *of, pCode *pc);
static void pCodePrintFunction(FILE *of, pCode *pc);
static void pCodeOpPrint(FILE *of, pCodeOp *pcop);
static char *pic16_get_op_from_instruction( pCodeInstruction *pcc);
char *pic16_get_op(pCodeOp *pcop,char *buff,size_t buf_size);
int pCodePeepMatchLine(pCodePeep *peepBlock, pCode *pcs, pCode *pcd);
int pic16_pCodePeepMatchRule(pCode *pc);
static void pBlockStats(FILE *of, pBlock *pb);
static pBlock *newpBlock(void);
extern void pic16_pCodeInsertAfter(pCode *pc1, pCode *pc2);
extern pCodeOp *pic16_popCopyReg(pCodeOpReg *pc);
pCodeOp *pic16_popCopyGPR2Bit(pCodeOp *pc, int bitval);
void pic16_pCodeRegMapLiveRanges(pBlock *pb);
void OptimizeLocalRegs(void);
pCodeOp *pic16_popGet2p(pCodeOp *src, pCodeOp *dst);

char *dumpPicOptype(PIC_OPTYPE type);

pCodeOp *pic16_popGetLit2(int, pCodeOp *);
pCodeOp *pic16_popGetLit(int);
pCodeOp *pic16_popGetWithString(char *);
int isBanksel(pCode *pc);
extern int inWparamList(char *s);

/** data flow optimization helpers **/
#if defined (DUMP_DF_GRAPHS) && DUMP_DF_GRAPHS > 0
static void pic16_vcg_dump (FILE *of, pBlock *pb);
static void pic16_vcg_dump_default (pBlock *pb);
#endif
static int pic16_pCodeIsAlive (pCode *pc);
static void pic16_df_stats ();
static void pic16_createDF (pBlock *pb);
static int pic16_removeUnusedRegistersDF ();
static void pic16_destructDF (pBlock *pb);
static void releaseStack ();

/****************************************************************/
/*                    PIC Instructions                          */
/****************************************************************/

pCodeInstruction pic16_pciADDWF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_ADDWF,
  "ADDWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_REGISTER | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciADDFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_ADDFW,
  "ADDWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_W | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciADDWFC = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_ADDWFC,
  "ADDWFC",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER | PCC_C),   // inCond
  (PCC_REGISTER | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciADDFWC = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_ADDFWC,
  "ADDWFC",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER | PCC_C),   // inCond
  (PCC_W | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciADDLW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_ADDLW,
  "ADDLW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_LITERAL),   // inCond
  (PCC_W | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciANDLW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_ANDLW,
  "ANDLW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_LITERAL),   // inCond
  (PCC_W | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciANDWF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_ANDWF,
  "ANDWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_REGISTER | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciANDFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_ANDFW,
  "ANDWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_W | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBC = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BC,
  "BC",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REL_ADDR | PCC_C),   // inCond
  PCC_NONE,    // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBCF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BCF,
  "BCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,1,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_BSF,
  (PCC_REGISTER | PCC_EXAMINE_PCOP),   // inCond
  PCC_REGISTER, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBN = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BN,
  "BN",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REL_ADDR | PCC_N),   // inCond
  PCC_NONE   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBNC = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BNC,
  "BNC",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REL_ADDR | PCC_C),   // inCond
  PCC_NONE   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBNN = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BNN,
  "BNN",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REL_ADDR | PCC_N),   // inCond
  PCC_NONE   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBNOV = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BNOV,
  "BNOV",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REL_ADDR | PCC_OV),   // inCond
  PCC_NONE   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBNZ = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BNZ,
  "BNZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REL_ADDR | PCC_Z),   // inCond
  PCC_NONE   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBOV = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BOV,
  "BOV",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REL_ADDR | PCC_OV),   // inCond
  PCC_NONE , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBRA = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BRA,
  "BRA",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REL_ADDR,   // inCond
  PCC_NONE   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBSF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BSF,
  "BSF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,1,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_BCF,
  (PCC_REGISTER | PCC_EXAMINE_PCOP),   // inCond
  (PCC_REGISTER | PCC_EXAMINE_PCOP), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBTFSC = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeSKIP,
   genericDestruct,
   genericPrint},
  POC_BTFSC,
  "BTFSC",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,1,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_BTFSS,
  (PCC_REGISTER | PCC_EXAMINE_PCOP),   // inCond
  PCC_EXAMINE_PCOP, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBTFSS = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeSKIP,
   genericDestruct,
   genericPrint},
  POC_BTFSS,
  "BTFSS",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,1,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_BTFSC,
  (PCC_REGISTER | PCC_EXAMINE_PCOP),   // inCond
  PCC_EXAMINE_PCOP, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBTG = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BTG,
  "BTG",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,1,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REGISTER | PCC_EXAMINE_PCOP),   // inCond
  (PCC_REGISTER | PCC_EXAMINE_PCOP), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciBZ = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_BZ,
  "BZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REL_ADDR | PCC_Z),   // inCond
  PCC_NONE, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciCALL = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_CALL,
  "CALL",
  4,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  1,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE, // inCond
  PCC_NONE, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciCOMF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_COMF,
  "COMF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,  // inCond
  (PCC_REGISTER | PCC_Z | PCC_N) , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciCOMFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_COMFW,
  "COMF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,  // inCond
  (PCC_W | PCC_Z | PCC_N) , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciCLRF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_CLRF,
  "CLRF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE, // inCond
  (PCC_REGISTER | PCC_Z), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciCLRWDT = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_CLRWDT,
  "CLRWDT",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE, // inCond
  PCC_NONE , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciCPFSEQ = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_CPFSEQ,
  "CPFSEQ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER), // inCond
  PCC_NONE , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciCPFSGT = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_CPFSGT,
  "CPFSGT",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER), // inCond
  PCC_NONE , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciCPFSLT = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_CPFSLT,
  "CPFSLT",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  1,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER), // inCond
  PCC_NONE , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciDAW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_DAW,
  "DAW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_W, // inCond
  (PCC_W | PCC_C), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciDCFSNZ = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_DCFSNZ,
  "DCFSNZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER, // inCond
  PCC_REGISTER , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciDCFSNZW = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_DCFSNZW,
  "DCFSNZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER, // inCond
  PCC_W , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciDECF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_DECF,
  "DECF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_REGISTER | PCC_STATUS)  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciDECFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_DECFW,
  "DECF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_W | PCC_STATUS)  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciDECFSZ = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeSKIP,
   genericDestruct,
   genericPrint},
  POC_DECFSZ,
  "DECFSZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  PCC_REGISTER   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciDECFSZW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeSKIP,
   genericDestruct,
   genericPrint},
  POC_DECFSZW,
  "DECFSZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  PCC_W          , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciGOTO = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeGOTO,
   genericDestruct,
   genericPrint},
  POC_GOTO,
  "GOTO",
  4,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REL_ADDR,   // inCond
  PCC_NONE   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciINCF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_INCF,
  "INCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_REGISTER | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciINCFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_INCFW,
  "INCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_W | PCC_STATUS)  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciINCFSZ = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeSKIP,
   genericDestruct,
   genericPrint},
  POC_INCFSZ,
  "INCFSZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_INFSNZ,
  PCC_REGISTER,   // inCond
  PCC_REGISTER   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciINCFSZW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeSKIP,
   genericDestruct,
   genericPrint},
  POC_INCFSZW,
  "INCFSZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_INFSNZW,
  PCC_REGISTER,   // inCond
  PCC_W          , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciINFSNZ = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeSKIP,
   genericDestruct,
   genericPrint},
  POC_INFSNZ,
  "INFSNZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_INCFSZ,
  PCC_REGISTER,   // inCond
  PCC_REGISTER   , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciINFSNZW = { // vrokas - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeSKIP,
   genericDestruct,
   genericPrint},
  POC_INFSNZW,
  "INFSNZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_INCFSZW,
  PCC_REGISTER,   // inCond
  PCC_W          , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciIORWF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_IORWF,
  "IORWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_REGISTER | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciIORFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_IORFW,
  "IORWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_W | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciIORLW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_IORLW,
  "IORLW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_LITERAL),   // inCond
  (PCC_W | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciLFSR = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_LFSR,
  "LFSR",
  4,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  1,    // second literal operand
  POC_NOP,
  PCC_LITERAL, // inCond
  PCC_NONE, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciMOVF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_MOVF,
  "MOVF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciMOVFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_MOVFW,
  "MOVF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_W | PCC_N | PCC_Z), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciMOVFF = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_MOVFF,
  "MOVFF",
  4,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  1,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  PCC_REGISTER2, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciMOVLB = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_MOVLB,
  "MOVLB",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_NONE | PCC_LITERAL),   // inCond
  PCC_REGISTER, // outCond - BSR
  PCI_MAGIC
};

pCodeInstruction pic16_pciMOVLW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_MOVLW,
  "MOVLW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_NONE | PCC_LITERAL),   // inCond
  PCC_W, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciMOVWF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_MOVWF,
  "MOVWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_W,   // inCond
  PCC_REGISTER, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciMULLW = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_MULLW,
  "MULLW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_LITERAL),   // inCond
  PCC_NONE, // outCond - PROD
  PCI_MAGIC
};

pCodeInstruction pic16_pciMULWF = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_MULWF,
  "MULWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  PCC_REGISTER, // outCond - PROD
  PCI_MAGIC
};

pCodeInstruction pic16_pciNEGF = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_NEGF,
  "NEGF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER, // inCond
  (PCC_REGISTER | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciNOP = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_NOP,
  "NOP",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,   // inCond
  PCC_NONE, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciPOP = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_POP,
  "POP",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciPUSH = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_PUSH,
  "PUSH",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciRCALL = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_RCALL,
  "RCALL",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REL_ADDR,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciRETFIE = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeRETURN,
   genericDestruct,
   genericPrint},
  POC_RETFIE,
  "RETFIE",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  1,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,   // inCond
  PCC_NONE,    // outCond (not true... affects the GIE bit too)
  PCI_MAGIC
};

pCodeInstruction pic16_pciRETLW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeRETURN,
   genericDestruct,
   genericPrint},
  POC_RETLW,
  "RETLW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_LITERAL,   // inCond
  PCC_W, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciRETURN = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   AnalyzeRETURN,
   genericDestruct,
   genericPrint},
  POC_RETURN,
  "RETURN",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  1,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  1,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,   // inCond
  PCC_NONE, // outCond
  PCI_MAGIC
};
pCodeInstruction pic16_pciRLCF = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_RLCF,
  "RLCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_C | PCC_REGISTER),   // inCond
  (PCC_REGISTER | PCC_C | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciRLCFW = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_RLCFW,
  "RLCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_C | PCC_REGISTER),   // inCond
  (PCC_W | PCC_C | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciRLNCF = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_RLNCF,
  "RLNCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_REGISTER | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};
pCodeInstruction pic16_pciRLNCFW = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_RLNCFW,
  "RLNCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_W | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};
pCodeInstruction pic16_pciRRCF = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_RRCF,
  "RRCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_C | PCC_REGISTER),   // inCond
  (PCC_REGISTER | PCC_C | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};
pCodeInstruction pic16_pciRRCFW = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_RRCFW,
  "RRCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_C | PCC_REGISTER),   // inCond
  (PCC_W | PCC_C | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};
pCodeInstruction pic16_pciRRNCF = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_RRNCF,
  "RRNCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_REGISTER | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciRRNCFW = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_RRNCFW,
  "RRNCF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  (PCC_W | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSETF = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SETF,
  "SETF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_REGISTER  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSUBLW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SUBLW,
  "SUBLW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_LITERAL),   // inCond
  (PCC_W | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSUBFWB = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SUBFWB,
  "SUBFWB",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER | PCC_C),   // inCond
  (PCC_W | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSUBWF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SUBWF,
  "SUBWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_REGISTER | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSUBFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SUBFW,
  "SUBWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_W | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSUBFWB_D1 = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SUBFWB_D1,
  "SUBFWB",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER | PCC_C),   // inCond
  (PCC_REGISTER | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSUBFWB_D0 = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SUBFWB_D0,
  "SUBFWB",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER | PCC_C),   // inCond
  (PCC_W | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSUBWFB_D1 = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SUBWFB_D1,
  "SUBWFB",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER | PCC_C),   // inCond
  (PCC_REGISTER | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSUBWFB_D0 = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SUBWFB_D0,
  "SUBWFB",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER | PCC_C),   // inCond
  (PCC_W | PCC_STATUS), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSWAPF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SWAPF,
  "SWAPF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REGISTER),   // inCond
  (PCC_REGISTER), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciSWAPFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_SWAPFW,
  "SWAPF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_REGISTER),   // inCond
  (PCC_W), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTBLRD = {     // patch 15
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_TBLRD,
  "TBLRD*",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTBLRD_POSTINC = {     // patch 15
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_TBLRD_POSTINC,
  "TBLRD*+",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTBLRD_POSTDEC = {     // patch 15
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_TBLRD_POSTDEC,
  "TBLRD*-",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTBLRD_PREINC = {      // patch 15
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_TBLRD_PREINC,
  "TBLRD+*",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTBLWT = {     // patch 15
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_TBLWT,
  "TBLWT*",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTBLWT_POSTINC = {     // patch 15
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_TBLWT_POSTINC,
  "TBLWT*+",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTBLWT_POSTDEC = {     // patch 15
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_TBLWT_POSTDEC,
  "TBLWT*-",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTBLWT_PREINC = {      // patch 15
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_TBLWT_PREINC,
  "TBLWT+*",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,  // inCond
  PCC_NONE  , // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciTSTFSZ = { // mdubuc - New
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_TSTFSZ,
  "TSTFSZ",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  2,    // num ops
  0,0,  // dest, bit instruction
  1,1,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_REGISTER,   // inCond
  PCC_NONE, // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciXORWF = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_XORWF,
  "XORWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  1,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_REGISTER | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciXORFW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_XORFW,
  "XORWF",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  3,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_REGISTER),   // inCond
  (PCC_W | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};

pCodeInstruction pic16_pciXORLW = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   //   genericAnalyze,
   genericDestruct,
   genericPrint},
  POC_XORLW,
  "XORLW",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  1,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  1,    // literal operand
  1,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  (PCC_W | PCC_LITERAL),   // inCond
  (PCC_W | PCC_Z | PCC_N), // outCond
  PCI_MAGIC
};


pCodeInstruction pic16_pciBANKSEL = {
  {PC_OPCODE, NULL, NULL, 0, NULL,
   genericDestruct,
   genericPrint},
  POC_BANKSEL,
  "BANKSEL",
  2,
  NULL, // from branch
  NULL, // to branch
  NULL, // label
  NULL, // operand
  NULL, // flow block
  NULL, // C source
  0,    // num ops
  0,0,  // dest, bit instruction
  0,0,  // branch, skip
  0,    // literal operand
  0,    // RAM access bit
  0,    // fast call/return mode select bit
  0,    // second memory operand
  0,    // second literal operand
  POC_NOP,
  PCC_NONE,   // inCond
  PCC_NONE, // outCond
  PCI_MAGIC
};


#define MAX_PIC16MNEMONICS 100
pCodeInstruction *pic16Mnemonics[MAX_PIC16MNEMONICS];

extern set *externs;
extern reg_info *pic16_allocProcessorRegister(int rIdx, char * name, short po_type, int alias);
extern reg_info *pic16_allocInternalRegister(int rIdx, char * name, short po_type, int alias);

void  pic16_pCodeInitRegisters(void)
{
  static int initialized=0;

        if(initialized)
                return;

        initialized = 1;

        pic16_pc_status.r = pic16_allocProcessorRegister(IDX_STATUS,"STATUS", PO_STATUS, 0x80);
        pic16_pc_pcl.r = pic16_allocProcessorRegister(IDX_PCL,"PCL", PO_PCL, 0x80);
        pic16_pc_pclath.r = pic16_allocProcessorRegister(IDX_PCLATH,"PCLATH", PO_PCLATH, 0x80);
        pic16_pc_pclatu.r = pic16_allocProcessorRegister(IDX_PCLATU,"PCLATU", PO_PCLATU, 0x80);
        pic16_pc_intcon.r = pic16_allocProcessorRegister(IDX_INTCON,"INTCON", PO_INTCON, 0x80);
        pic16_pc_wreg.r = pic16_allocProcessorRegister(IDX_WREG,"WREG", PO_WREG, 0x80);
        pic16_pc_bsr.r = pic16_allocProcessorRegister(IDX_BSR,"BSR", PO_BSR, 0x80);

        pic16_pc_tosl.r = pic16_allocProcessorRegister(IDX_TOSL,"TOSL", PO_SFR_REGISTER, 0x80);
        pic16_pc_tosh.r = pic16_allocProcessorRegister(IDX_TOSH,"TOSH", PO_SFR_REGISTER, 0x80);
        pic16_pc_tosu.r = pic16_allocProcessorRegister(IDX_TOSU,"TOSU", PO_SFR_REGISTER, 0x80);

        pic16_pc_tblptrl.r = pic16_allocProcessorRegister(IDX_TBLPTRL,"TBLPTRL", PO_SFR_REGISTER, 0x80);
        pic16_pc_tblptrh.r = pic16_allocProcessorRegister(IDX_TBLPTRH,"TBLPTRH", PO_SFR_REGISTER, 0x80);
        pic16_pc_tblptru.r = pic16_allocProcessorRegister(IDX_TBLPTRU,"TBLPTRU", PO_SFR_REGISTER, 0x80);
        pic16_pc_tablat.r = pic16_allocProcessorRegister(IDX_TABLAT,"TABLAT", PO_SFR_REGISTER, 0x80);

        pic16_pc_fsr0l.r = pic16_allocProcessorRegister(IDX_FSR0L, "FSR0L", PO_FSR0, 0x80);
        pic16_pc_fsr0h.r = pic16_allocProcessorRegister(IDX_FSR0H, "FSR0H", PO_FSR0, 0x80);
        pic16_pc_fsr1l.r = pic16_allocProcessorRegister(IDX_FSR1L, "FSR1L", PO_FSR0, 0x80);
        pic16_pc_fsr1h.r = pic16_allocProcessorRegister(IDX_FSR1H, "FSR1H", PO_FSR0, 0x80);
        pic16_pc_fsr2l.r = pic16_allocProcessorRegister(IDX_FSR2L, "FSR2L", PO_FSR0, 0x80);
        pic16_pc_fsr2h.r = pic16_allocProcessorRegister(IDX_FSR2H, "FSR2H", PO_FSR0, 0x80);

        pic16_stackpnt_lo = &pic16_pc_fsr1l;
        pic16_stackpnt_hi = &pic16_pc_fsr1h;
        pic16_stack_postdec = &pic16_pc_postdec1;
        pic16_stack_postinc = &pic16_pc_postinc1;
        pic16_stack_preinc = &pic16_pc_preinc1;
        pic16_stack_plusw = &pic16_pc_plusw1;

        pic16_framepnt_lo = &pic16_pc_fsr2l;
        pic16_framepnt_hi = &pic16_pc_fsr2h;
        pic16_frame_postdec = &pic16_pc_postdec2;
        pic16_frame_postinc = &pic16_pc_postinc2;
        pic16_frame_preinc = &pic16_pc_preinc2;
        pic16_frame_plusw = &pic16_pc_plusw2;

        pic16_pc_indf0.r = pic16_allocProcessorRegister(IDX_INDF0,"INDF0", PO_INDF0, 0x80);
        pic16_pc_postinc0.r = pic16_allocProcessorRegister(IDX_POSTINC0, "POSTINC0", PO_INDF0, 0x80);
        pic16_pc_postdec0.r = pic16_allocProcessorRegister(IDX_POSTDEC0, "POSTDEC0", PO_INDF0, 0x80);
        pic16_pc_preinc0.r = pic16_allocProcessorRegister(IDX_PREINC0, "PREINC0", PO_INDF0, 0x80);
        pic16_pc_plusw0.r = pic16_allocProcessorRegister(IDX_PLUSW0, "PLUSW0", PO_INDF0, 0x80);

        pic16_pc_indf1.r = pic16_allocProcessorRegister(IDX_INDF1,"INDF1", PO_INDF0, 0x80);
        pic16_pc_postinc1.r = pic16_allocProcessorRegister(IDX_POSTINC1, "POSTINC1", PO_INDF0, 0x80);
        pic16_pc_postdec1.r = pic16_allocProcessorRegister(IDX_POSTDEC1, "POSTDEC1", PO_INDF0, 0x80);
        pic16_pc_preinc1.r = pic16_allocProcessorRegister(IDX_PREINC1, "PREINC1", PO_INDF0, 0x80);
        pic16_pc_plusw1.r = pic16_allocProcessorRegister(IDX_PLUSW1, "PLUSW1", PO_INDF0, 0x80);

        pic16_pc_indf2.r = pic16_allocProcessorRegister(IDX_INDF2,"INDF2", PO_INDF0, 0x80);
        pic16_pc_postinc2.r = pic16_allocProcessorRegister(IDX_POSTINC2, "POSTINC2", PO_INDF0, 0x80);
        pic16_pc_postdec2.r = pic16_allocProcessorRegister(IDX_POSTDEC2, "POSTDEC2", PO_INDF0, 0x80);
        pic16_pc_preinc2.r = pic16_allocProcessorRegister(IDX_PREINC2, "PREINC2", PO_INDF0, 0x80);
        pic16_pc_plusw2.r = pic16_allocProcessorRegister(IDX_PLUSW2, "PLUSW2", PO_INDF0, 0x80);

        pic16_pc_prodl.r = pic16_allocProcessorRegister(IDX_PRODL, "PRODL", PO_PRODL, 0x80);
        pic16_pc_prodh.r = pic16_allocProcessorRegister(IDX_PRODH, "PRODH", PO_PRODH, 0x80);


        pic16_pc_eecon1.r = pic16_allocProcessorRegister(IDX_EECON1, "EECON1", PO_SFR_REGISTER, 0x80);
        pic16_pc_eecon2.r = pic16_allocProcessorRegister(IDX_EECON2, "EECON2", PO_SFR_REGISTER, 0x80);
        pic16_pc_eedata.r = pic16_allocProcessorRegister(IDX_EEDATA, "EEDATA", PO_SFR_REGISTER, 0x80);
        pic16_pc_eeadr.r = pic16_allocProcessorRegister(IDX_EEADR, "EEADR", PO_SFR_REGISTER, 0x80);


        pic16_pc_status.rIdx = IDX_STATUS;
        pic16_pc_intcon.rIdx = IDX_INTCON;
        pic16_pc_pcl.rIdx = IDX_PCL;
        pic16_pc_pclath.rIdx = IDX_PCLATH;
        pic16_pc_pclatu.rIdx = IDX_PCLATU;
        pic16_pc_wreg.rIdx = IDX_WREG;
        pic16_pc_bsr.rIdx = IDX_BSR;

        pic16_pc_tosl.rIdx = IDX_TOSL;
        pic16_pc_tosh.rIdx = IDX_TOSH;
        pic16_pc_tosu.rIdx = IDX_TOSU;

        pic16_pc_tblptrl.rIdx = IDX_TBLPTRL;
        pic16_pc_tblptrh.rIdx = IDX_TBLPTRH;
        pic16_pc_tblptru.rIdx = IDX_TBLPTRU;
        pic16_pc_tablat.rIdx = IDX_TABLAT;

        pic16_pc_fsr0l.rIdx = IDX_FSR0L;
        pic16_pc_fsr0h.rIdx = IDX_FSR0H;
        pic16_pc_fsr1l.rIdx = IDX_FSR1L;
        pic16_pc_fsr1h.rIdx = IDX_FSR1H;
        pic16_pc_fsr2l.rIdx = IDX_FSR2L;
        pic16_pc_fsr2h.rIdx = IDX_FSR2H;
        pic16_pc_indf0.rIdx = IDX_INDF0;
        pic16_pc_postinc0.rIdx = IDX_POSTINC0;
        pic16_pc_postdec0.rIdx = IDX_POSTDEC0;
        pic16_pc_preinc0.rIdx = IDX_PREINC0;
        pic16_pc_plusw0.rIdx = IDX_PLUSW0;
        pic16_pc_indf1.rIdx = IDX_INDF1;
        pic16_pc_postinc1.rIdx = IDX_POSTINC1;
        pic16_pc_postdec1.rIdx = IDX_POSTDEC1;
        pic16_pc_preinc1.rIdx = IDX_PREINC1;
        pic16_pc_plusw1.rIdx = IDX_PLUSW1;
        pic16_pc_indf2.rIdx = IDX_INDF2;
        pic16_pc_postinc2.rIdx = IDX_POSTINC2;
        pic16_pc_postdec2.rIdx = IDX_POSTDEC2;
        pic16_pc_preinc2.rIdx = IDX_PREINC2;
        pic16_pc_plusw2.rIdx = IDX_PLUSW2;
        pic16_pc_prodl.rIdx = IDX_PRODL;
        pic16_pc_prodh.rIdx = IDX_PRODH;

        pic16_pc_kzero.r = pic16_allocInternalRegister(IDX_KZ,"KZ",PO_GPR_REGISTER,0);
        pic16_pc_ssave.r = pic16_allocInternalRegister(IDX_SSAVE,"SSAVE", PO_GPR_REGISTER, 0);
        pic16_pc_wsave.r = pic16_allocInternalRegister(IDX_WSAVE,"WSAVE", PO_GPR_REGISTER, 0);

        pic16_pc_kzero.rIdx = IDX_KZ;
        pic16_pc_wsave.rIdx = IDX_WSAVE;
        pic16_pc_ssave.rIdx = IDX_SSAVE;

        pic16_pc_eecon1.rIdx = IDX_EECON1;
        pic16_pc_eecon2.rIdx = IDX_EECON2;
        pic16_pc_eedata.rIdx = IDX_EEDATA;
        pic16_pc_eeadr.rIdx = IDX_EEADR;


        pic16_pc_gpsimio.r = pic16_allocProcessorRegister(IDX_GPSIMIO, "GPSIMIO", PO_GPR_REGISTER, 0x80);
        pic16_pc_gpsimio2.r = pic16_allocProcessorRegister(IDX_GPSIMIO2, "GPSIMIO2", PO_GPR_REGISTER, 0x80);

        pic16_pc_gpsimio.rIdx = IDX_GPSIMIO;
        pic16_pc_gpsimio2.rIdx = IDX_GPSIMIO2;

        /* probably should put this in a separate initialization routine */
        pb_dead_pcodes = newpBlock();

}

/*-----------------------------------------------------------------*/
/*  mnem2key - convert a pic mnemonic into a hash key              */
/*   (BTW - this spreads the mnemonics quite well)                 */
/*                                                                 */
/*-----------------------------------------------------------------*/

static int mnem2key(unsigned char const *mnem)
{
  int key = 0;

  if(!mnem)
    return 0;

  while(*mnem) {

    key += toupper(*mnem++) +1;

  }

  return (key & 0x1f);

}

void pic16initMnemonics(void)
{
  int i = 0;
  int key;
  //  char *str;
  pCodeInstruction *pci;

  if(mnemonics_initialized)
    return;

  // NULL out the array before making the assignments
  // since we check the array contents below this initialization.

  for (i = 0; i < MAX_PIC16MNEMONICS; i++) {
    pic16Mnemonics[i] = NULL;
  }

  pic16Mnemonics[POC_ADDLW] = &pic16_pciADDLW;
  pic16Mnemonics[POC_ADDWF] = &pic16_pciADDWF;
  pic16Mnemonics[POC_ADDFW] = &pic16_pciADDFW;
  pic16Mnemonics[POC_ADDWFC] = &pic16_pciADDWFC;
  pic16Mnemonics[POC_ADDFWC] = &pic16_pciADDFWC;
  pic16Mnemonics[POC_ANDLW] = &pic16_pciANDLW;
  pic16Mnemonics[POC_ANDWF] = &pic16_pciANDWF;
  pic16Mnemonics[POC_ANDFW] = &pic16_pciANDFW;
  pic16Mnemonics[POC_BC] = &pic16_pciBC;
  pic16Mnemonics[POC_BCF] = &pic16_pciBCF;
  pic16Mnemonics[POC_BN] = &pic16_pciBN;
  pic16Mnemonics[POC_BNC] = &pic16_pciBNC;
  pic16Mnemonics[POC_BNN] = &pic16_pciBNN;
  pic16Mnemonics[POC_BNOV] = &pic16_pciBNOV;
  pic16Mnemonics[POC_BNZ] = &pic16_pciBNZ;
  pic16Mnemonics[POC_BOV] = &pic16_pciBOV;
  pic16Mnemonics[POC_BRA] = &pic16_pciBRA;
  pic16Mnemonics[POC_BSF] = &pic16_pciBSF;
  pic16Mnemonics[POC_BTFSC] = &pic16_pciBTFSC;
  pic16Mnemonics[POC_BTFSS] = &pic16_pciBTFSS;
  pic16Mnemonics[POC_BTG] = &pic16_pciBTG;
  pic16Mnemonics[POC_BZ] = &pic16_pciBZ;
  pic16Mnemonics[POC_CALL] = &pic16_pciCALL;
  pic16Mnemonics[POC_CLRF] = &pic16_pciCLRF;
  pic16Mnemonics[POC_CLRWDT] = &pic16_pciCLRWDT;
  pic16Mnemonics[POC_COMF] = &pic16_pciCOMF;
  pic16Mnemonics[POC_COMFW] = &pic16_pciCOMFW;
  pic16Mnemonics[POC_CPFSEQ] = &pic16_pciCPFSEQ;
  pic16Mnemonics[POC_CPFSGT] = &pic16_pciCPFSGT;
  pic16Mnemonics[POC_CPFSLT] = &pic16_pciCPFSLT;
  pic16Mnemonics[POC_DAW] = &pic16_pciDAW;
  pic16Mnemonics[POC_DCFSNZ] = &pic16_pciDCFSNZ;
  pic16Mnemonics[POC_DECF] = &pic16_pciDECF;
  pic16Mnemonics[POC_DECFW] = &pic16_pciDECFW;
  pic16Mnemonics[POC_DECFSZ] = &pic16_pciDECFSZ;
  pic16Mnemonics[POC_DECFSZW] = &pic16_pciDECFSZW;
  pic16Mnemonics[POC_GOTO] = &pic16_pciGOTO;
  pic16Mnemonics[POC_INCF] = &pic16_pciINCF;
  pic16Mnemonics[POC_INCFW] = &pic16_pciINCFW;
  pic16Mnemonics[POC_INCFSZ] = &pic16_pciINCFSZ;
  pic16Mnemonics[POC_INCFSZW] = &pic16_pciINCFSZW;
  pic16Mnemonics[POC_INFSNZ] = &pic16_pciINFSNZ;
  pic16Mnemonics[POC_INFSNZW] = &pic16_pciINFSNZW;
  pic16Mnemonics[POC_IORWF] = &pic16_pciIORWF;
  pic16Mnemonics[POC_IORFW] = &pic16_pciIORFW;
  pic16Mnemonics[POC_IORLW] = &pic16_pciIORLW;
  pic16Mnemonics[POC_LFSR] = &pic16_pciLFSR;
  pic16Mnemonics[POC_MOVF] = &pic16_pciMOVF;
  pic16Mnemonics[POC_MOVFW] = &pic16_pciMOVFW;
  pic16Mnemonics[POC_MOVFF] = &pic16_pciMOVFF;
  pic16Mnemonics[POC_MOVLB] = &pic16_pciMOVLB;
  pic16Mnemonics[POC_MOVLW] = &pic16_pciMOVLW;
  pic16Mnemonics[POC_MOVWF] = &pic16_pciMOVWF;
  pic16Mnemonics[POC_MULLW] = &pic16_pciMULLW;
  pic16Mnemonics[POC_MULWF] = &pic16_pciMULWF;
  pic16Mnemonics[POC_NEGF] = &pic16_pciNEGF;
  pic16Mnemonics[POC_NOP] = &pic16_pciNOP;
  pic16Mnemonics[POC_POP] = &pic16_pciPOP;
  pic16Mnemonics[POC_PUSH] = &pic16_pciPUSH;
  pic16Mnemonics[POC_RCALL] = &pic16_pciRCALL;
  pic16Mnemonics[POC_RETFIE] = &pic16_pciRETFIE;
  pic16Mnemonics[POC_RETLW] = &pic16_pciRETLW;
  pic16Mnemonics[POC_RETURN] = &pic16_pciRETURN;
  pic16Mnemonics[POC_RLCF] = &pic16_pciRLCF;
  pic16Mnemonics[POC_RLCFW] = &pic16_pciRLCFW;
  pic16Mnemonics[POC_RLNCF] = &pic16_pciRLNCF;
  pic16Mnemonics[POC_RLNCFW] = &pic16_pciRLNCFW;
  pic16Mnemonics[POC_RRCF] = &pic16_pciRRCF;
  pic16Mnemonics[POC_RRCFW] = &pic16_pciRRCFW;
  pic16Mnemonics[POC_RRNCF] = &pic16_pciRRNCF;
  pic16Mnemonics[POC_RRNCFW] = &pic16_pciRRNCFW;
  pic16Mnemonics[POC_SETF] = &pic16_pciSETF;
  pic16Mnemonics[POC_SUBLW] = &pic16_pciSUBLW;
  pic16Mnemonics[POC_SUBWF] = &pic16_pciSUBWF;
  pic16Mnemonics[POC_SUBFW] = &pic16_pciSUBFW;
  pic16Mnemonics[POC_SUBWFB_D0] = &pic16_pciSUBWFB_D0;
  pic16Mnemonics[POC_SUBWFB_D1] = &pic16_pciSUBWFB_D1;
  pic16Mnemonics[POC_SUBFWB_D0] = &pic16_pciSUBFWB_D0;
  pic16Mnemonics[POC_SUBFWB_D1] = &pic16_pciSUBFWB_D1;
  pic16Mnemonics[POC_SWAPF] = &pic16_pciSWAPF;
  pic16Mnemonics[POC_SWAPFW] = &pic16_pciSWAPFW;
  pic16Mnemonics[POC_TBLRD] = &pic16_pciTBLRD;
  pic16Mnemonics[POC_TBLRD_POSTINC] = &pic16_pciTBLRD_POSTINC;
  pic16Mnemonics[POC_TBLRD_POSTDEC] = &pic16_pciTBLRD_POSTDEC;
  pic16Mnemonics[POC_TBLRD_PREINC] = &pic16_pciTBLRD_PREINC;
  pic16Mnemonics[POC_TBLWT] = &pic16_pciTBLWT;
  pic16Mnemonics[POC_TBLWT_POSTINC] = &pic16_pciTBLWT_POSTINC;
  pic16Mnemonics[POC_TBLWT_POSTDEC] = &pic16_pciTBLWT_POSTDEC;
  pic16Mnemonics[POC_TBLWT_PREINC] = &pic16_pciTBLWT_PREINC;
  pic16Mnemonics[POC_TSTFSZ] = &pic16_pciTSTFSZ;
  pic16Mnemonics[POC_XORLW] = &pic16_pciXORLW;
  pic16Mnemonics[POC_XORWF] = &pic16_pciXORWF;
  pic16Mnemonics[POC_XORFW] = &pic16_pciXORFW;
  pic16Mnemonics[POC_BANKSEL] = &pic16_pciBANKSEL;

  for(i=0; i<MAX_PIC16MNEMONICS; i++)
    if(pic16Mnemonics[i])
      hTabAddItem(&pic16MnemonicsHash, mnem2key((const unsigned char *)pic16Mnemonics[i]->mnemonic), pic16Mnemonics[i]);
  pci = hTabFirstItem(pic16MnemonicsHash, &key);

  while(pci) {
    DFPRINTF((stderr, "element %d key %d, mnem %s\n",i++,key,pci->mnemonic));
    pci = hTabNextItem(pic16MnemonicsHash, &key);
  }

  mnemonics_initialized = 1;
}

int pic16_getpCodePeepCommand(char *cmd);

int pic16_getpCode(char *mnem,unsigned dest)
{

  pCodeInstruction *pci;
  int key = mnem2key((unsigned char *)mnem);

  if(!mnemonics_initialized)
    pic16initMnemonics();

  pci = hTabFirstItemWK(pic16MnemonicsHash, key);

  while(pci) {

    if(STRCASECMP(pci->mnemonic, mnem) == 0) {
      if((pci->num_ops <= 1)
        || (pci->isModReg == dest)
        || (pci->isBitInst)
        || (pci->num_ops <= 2 && pci->isAccess)
        || (pci->num_ops <= 2 && pci->isFastCall)
        || (pci->num_ops <= 2 && pci->is2MemOp)
        || (pci->num_ops <= 2 && pci->is2LitOp) )
        return(pci->op);
    }

    pci = hTabNextItemWK (pic16MnemonicsHash);

  }

  return -1;
}

/*-----------------------------------------------------------------*
 * pic16initpCodePeepCommands
 *
 *-----------------------------------------------------------------*/
void pic16initpCodePeepCommands(void)
{

  int key, i;
  peepCommand *pcmd;

  i = 0;
  do {
    hTabAddItem(&pic16pCodePeepCommandsHash,
                mnem2key((const unsigned char *)peepCommands[i].cmd), &peepCommands[i]);
    i++;
  } while (peepCommands[i].cmd);

  pcmd = hTabFirstItem(pic16pCodePeepCommandsHash, &key);

  while(pcmd) {
    //fprintf(stderr, "peep command %s  key %d\n",pcmd->cmd,pcmd->id);
    pcmd = hTabNextItem(pic16pCodePeepCommandsHash, &key);
  }

}

/*-----------------------------------------------------------------
 *
 *
 *-----------------------------------------------------------------*/

int pic16_getpCodePeepCommand(char *cmd)
{

  peepCommand *pcmd;
  int key = mnem2key((unsigned char *)cmd);


  pcmd = hTabFirstItemWK(pic16pCodePeepCommandsHash, key);

  while(pcmd) {
    // fprintf(stderr," comparing %s to %s\n",pcmd->cmd,cmd);
    if(STRCASECMP(pcmd->cmd, cmd) == 0) {
      return pcmd->id;
    }

    pcmd = hTabNextItemWK (pic16pCodePeepCommandsHash);

  }

  return -1;
}

static char getpBlock_dbName(pBlock *pb)
{
  if(!pb)
    return 0;

  if(pb->cmemmap)
    return pb->cmemmap->dbName;

  return pb->dbName;
}
void pic16_pBlockConvert2ISR(pBlock *pb)
{
        if(!pb)return;

        if(pb->cmemmap)pb->cmemmap = NULL;

        pb->dbName = 'I';

        if(pic16_pcode_verbose)
                fprintf(stderr, "%s:%d converting to 'I'interrupt pBlock\n", __FILE__, __LINE__);
}

void pic16_pBlockConvert2Absolute(pBlock *pb)
{
        if(!pb)return;
        if(pb->cmemmap)pb->cmemmap = NULL;

        pb->dbName = 'A';

        if(pic16_pcode_verbose)
                fprintf(stderr, "%s:%d converting to 'A'bsolute pBlock\n", __FILE__, __LINE__);
}

/*-----------------------------------------------------------------*/
/* pic16_movepBlock2Head - given the dbname of a pBlock, move all  */
/*                   instances to the front of the doubly linked   */
/*                   list of pBlocks                               */
/*-----------------------------------------------------------------*/

void pic16_movepBlock2Head(char dbName)
{
  pBlock *pb;


  /* this can happen in sources without code,
   * only variable definitions */
  if(!the_pFile)return;

  pb = the_pFile->pbHead;

  while(pb) {

    if(getpBlock_dbName(pb) == dbName) {
      pBlock *pbn = pb->next;
      pb->next = the_pFile->pbHead;
      the_pFile->pbHead->prev = pb;
      the_pFile->pbHead = pb;

      if(pb->prev)
        pb->prev->next = pbn;

      // If the pBlock that we just moved was the last
      // one in the link of all of the pBlocks, then we
      // need to point the tail to the block just before
      // the one we moved.
      // Note: if pb->next is NULL, then pb must have
      // been the last pBlock in the chain.

      if(pbn)
        pbn->prev = pb->prev;
      else
        the_pFile->pbTail = pb->prev;

      pb = pbn;

    } else
      pb = pb->next;

  }

}

void pic16_copypCode(FILE *of, char dbName)
{
  pBlock *pb;

        if(!of || !the_pFile)
                return;

        for(pb = the_pFile->pbHead; pb; pb = pb->next) {
                if(getpBlock_dbName(pb) == dbName) {
//                      fprintf(stderr, "%s:%d: output of pb= 0x%p\n", __FILE__, __LINE__, pb);
                        pBlockStats(of,pb);
                        pic16_printpBlock(of,pb);
                }
        }

}
void pic16_pcode_test(void)
{

  DFPRINTF((stderr,"pcode is alive!\n"));

  //initMnemonics();

  if (the_pFile) {
    pBlock *pb;
    FILE *pFile;
    char buffer[100];

    /* create the file name */
    strcpy(buffer,dstFileName);
    strcat(buffer,".p");

    if( !(pFile = fopen(buffer, "w" ))) {
      werror(E_FILE_OPEN_ERR,buffer);
      exit(1);
    }

    fprintf(pFile,"pcode dump\n\n");

    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
      fprintf(pFile,"\n\tNew pBlock\n\n");
      if(pb->cmemmap)
        fprintf(pFile,"%s",pb->cmemmap->sname);
      else
        fprintf(pFile,"internal pblock");

      fprintf(pFile,", dbName =%c\n",getpBlock_dbName(pb));
      pic16_printpBlock(pFile,pb);
    }
    fclose(pFile);
  }
}


unsigned long pic16_countInstructions(void)
{
  pBlock *pb;
  pCode *pc;
  unsigned long isize=0;

    if(!the_pFile)return -1;

    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
      for(pc = pb->pcHead; pc; pc = pc->next) {
        if(isPCI(pc) || isPCAD(pc))isize += PCI(pc)->isize;
      }
    }
  return (isize);
}


/*-----------------------------------------------------------------*/
/* int RegCond(pCodeOp *pcop) - if pcop points to the STATUS reg-  */
/*      ister, RegCond will return the bit being referenced.       */
/*                                                                 */
/* fixme - why not just OR in the pcop bit field                   */
/*-----------------------------------------------------------------*/

static int RegCond(pCodeOp *pcop)
{

  if(!pcop)
    return 0;

  if(!pcop->name)return 0;

  if(pcop->type == PO_GPR_BIT  && !strcmp(pcop->name, pic16_pc_status.pcop.name)) {
    switch(PCORB(pcop)->bit) {
    case PIC_C_BIT:
      return PCC_C;
    case PIC_DC_BIT:
        return PCC_DC;
    case PIC_Z_BIT:
      return PCC_Z;
    }

  }

  return 0;
}


/*-----------------------------------------------------------------*/
/* pic16_newpCode - create and return a newly initialized pCode          */
/*                                                                 */
/*  fixme - rename this                                            */
/*                                                                 */
/* The purpose of this routine is to create a new Instruction      */
/* pCode. This is called by gen.c while the assembly code is being */
/* generated.                                                      */
/*                                                                 */
/* Inouts:                                                         */
/*  PIC_OPCODE op - the assembly instruction we wish to create.    */
/*                  (note that the op is analogous to but not the  */
/*                  same thing as the opcode of the instruction.)  */
/*  pCdoeOp *pcop - pointer to the operand of the instruction.     */
/*                                                                 */
/* Outputs:                                                        */
/*  a pointer to the new malloc'd pCode is returned.               */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*-----------------------------------------------------------------*/
pCode *pic16_newpCode (PIC_OPCODE op, pCodeOp *pcop)
{
  pCodeInstruction *pci ;

  if(!mnemonics_initialized)
    pic16initMnemonics();

  pci = Safe_calloc(1, sizeof(pCodeInstruction));

  if((op>=0) && (op < MAX_PIC16MNEMONICS) && pic16Mnemonics[op]) {
    memcpy(pci, pic16Mnemonics[op], sizeof(pCodeInstruction));
    pci->pcop = pcop;

    if(pci->inCond & PCC_EXAMINE_PCOP)
      pci->inCond  |= RegCond(pcop);

    if(pci->outCond & PCC_EXAMINE_PCOP)
      pci->outCond  |= RegCond(pcop);

    pci->pc.prev = pci->pc.next = NULL;
    return (pCode *)pci;
  }

  fprintf(stderr, "pCode mnemonic error %s,%d\n",__FUNCTION__,__LINE__);
  exit(1);

  return NULL;
}

/*-----------------------------------------------------------------*/
/* pic16_newpCodeWild - create a "wild" as in wild card pCode            */
/*                                                                 */
/* Wild pcodes are used during the peep hole optimizer to serve    */
/* as place holders for any instruction. When a snippet of code is */
/* compared to a peep hole rule, the wild card opcode will match   */
/* any instruction. However, the optional operand and label are    */
/* additional qualifiers that must also be matched before the      */
/* line (of assembly code) is declared matched. Note that the      */
/* operand may be wild too.                                        */
/*                                                                 */
/*   Note, a wild instruction is specified just like a wild var:   */
/*      %4     ; A wild instruction,                               */
/*  See the peeph.def file for additional examples                 */
/*                                                                 */
/*-----------------------------------------------------------------*/

pCode *pic16_newpCodeWild(int pCodeID, pCodeOp *optional_operand, pCodeOp *optional_label)
{

  pCodeWild *pcw;

  pcw = Safe_calloc(1,sizeof(pCodeWild));

  pcw->pci.pc.type = PC_WILD;
  pcw->pci.pc.prev = pcw->pci.pc.next = NULL;
  pcw->pci.from = pcw->pci.to = pcw->pci.label = NULL;
  pcw->pci.pc.pb = NULL;

  //  pcw->pci.pc.analyze = genericAnalyze;
  pcw->pci.pc.destruct = genericDestruct;
  pcw->pci.pc.print = genericPrint;

  pcw->id = pCodeID;              // this is the 'n' in %n
  pcw->operand = optional_operand;
  pcw->label   = optional_label;

  pcw->mustBeBitSkipInst = 0;
  pcw->mustNotBeBitSkipInst = 0;
  pcw->invertBitSkipInst = 0;

  return ( (pCode *)pcw);

}

 /*-----------------------------------------------------------------*/
/* newPcodeInlineP - create a new pCode from a char string           */
/*-----------------------------------------------------------------*/


pCode *pic16_newpCodeInlineP(char *cP)
{

  pCodeComment *pcc ;

  pcc = Safe_calloc(1,sizeof(pCodeComment));

  pcc->pc.type = PC_INLINE;
  pcc->pc.prev = pcc->pc.next = NULL;
  //pcc->pc.from = pcc->pc.to = pcc->pc.label = NULL;
  pcc->pc.pb = NULL;

  //  pcc->pc.analyze = genericAnalyze;
  pcc->pc.destruct = genericDestruct;
  pcc->pc.print = genericPrint;

  if(cP)
    pcc->comment = Safe_strdup(cP);
  else
    pcc->comment = NULL;

  return ( (pCode *)pcc);

}

/*-----------------------------------------------------------------*/
/* newPcodeCharP - create a new pCode from a char string           */
/*-----------------------------------------------------------------*/

pCode *pic16_newpCodeCharP(char *cP)
{

  pCodeComment *pcc ;

  pcc = Safe_calloc(1,sizeof(pCodeComment));

  pcc->pc.type = PC_COMMENT;
  pcc->pc.prev = pcc->pc.next = NULL;
  //pcc->pc.from = pcc->pc.to = pcc->pc.label = NULL;
  pcc->pc.pb = NULL;

  //  pcc->pc.analyze = genericAnalyze;
  pcc->pc.destruct = genericDestruct;
  pcc->pc.print = genericPrint;

  if(cP)
    pcc->comment = Safe_strdup(cP);
  else
    pcc->comment = NULL;

  return ( (pCode *)pcc);

}

/*-----------------------------------------------------------------*/
/* pic16_newpCodeFunction -                                              */
/*-----------------------------------------------------------------*/


pCode *pic16_newpCodeFunction(const char *mod, const char *f)
{
  pCodeFunction *pcf;

  pcf = Safe_calloc(1,sizeof(pCodeFunction));

  pcf->pc.type = PC_FUNCTION;
  pcf->pc.prev = pcf->pc.next = NULL;
  //pcf->pc.from = pcf->pc.to = pcf->pc.label = NULL;
  pcf->pc.pb = NULL;

  //  pcf->pc.analyze = genericAnalyze;
  pcf->pc.destruct = genericDestruct;
  pcf->pc.print = pCodePrintFunction;

  pcf->ncalled = 0;
  pcf->absblock = 0;

  if(mod) {
    pcf->modname = Safe_calloc(1,strlen(mod)+1);
    strcpy(pcf->modname,mod);
  } else
    pcf->modname = NULL;

  if(f) {
    pcf->fname = Safe_calloc(1,strlen(f)+1);
    strcpy(pcf->fname,f);
  } else
    pcf->fname = NULL;

  pcf->stackusage = 0;

  return ( (pCode *)pcf);
}

/*-----------------------------------------------------------------*/
/* pic16_newpCodeFlow                                                    */
/*-----------------------------------------------------------------*/
static void destructpCodeFlow(pCode *pc)
{
  if(!pc || !isPCFL(pc))
    return;

/*
  if(PCFL(pc)->from)
  if(PCFL(pc)->to)
*/
  pic16_unlinkpCode(pc);

  deleteSet(&PCFL(pc)->registers);
  deleteSet(&PCFL(pc)->from);
  deleteSet(&PCFL(pc)->to);

  /* Instead of deleting the memory used by this pCode, mark
   * the object as bad so that if there's a pointer to this pCode
   * dangling around somewhere then (hopefully) when the type is
   * checked we'll catch it.
   */

  pc->type = PC_BAD;
  pic16_addpCode2pBlock(pb_dead_pcodes, pc);

//  Safe_free(pc);

}

pCode *pic16_newpCodeFlow(void )
{
  pCodeFlow *pcflow;

  //_ALLOC(pcflow,sizeof(pCodeFlow));
  pcflow = Safe_calloc(1,sizeof(pCodeFlow));

  pcflow->pc.type = PC_FLOW;
  pcflow->pc.prev = pcflow->pc.next = NULL;
  pcflow->pc.pb = NULL;

  //  pcflow->pc.analyze = genericAnalyze;
  pcflow->pc.destruct = destructpCodeFlow;
  pcflow->pc.print = genericPrint;

  pcflow->pc.seq = GpcFlowSeq++;

  pcflow->from = pcflow->to = NULL;

  pcflow->inCond = PCC_NONE;
  pcflow->outCond = PCC_NONE;

  pcflow->firstBank = -1;
  pcflow->lastBank = -1;

  pcflow->FromConflicts = 0;
  pcflow->ToConflicts = 0;

  pcflow->end = NULL;

  pcflow->registers = newSet();

  return ( (pCode *)pcflow);

}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeFlowLink *pic16_newpCodeFlowLink(pCodeFlow *pcflow)
{
  pCodeFlowLink *pcflowLink;

  pcflowLink = Safe_calloc(1,sizeof(pCodeFlowLink));

  pcflowLink->pcflow = pcflow;
  pcflowLink->bank_conflict = 0;

  return pcflowLink;
}

/*-----------------------------------------------------------------*/
/* pic16_newpCodeCSource - create a new pCode Source Symbol        */
/*-----------------------------------------------------------------*/

pCode *pic16_newpCodeCSource(int ln, const char *f, const char *l)
{

  pCodeCSource *pccs;

  pccs = Safe_calloc(1,sizeof(pCodeCSource));

  pccs->pc.type = PC_CSOURCE;
  pccs->pc.prev = pccs->pc.next = NULL;
  pccs->pc.pb = NULL;

  pccs->pc.destruct = genericDestruct;
  pccs->pc.print = genericPrint;

  pccs->line_number = ln;
  if(l)
    pccs->line = Safe_strdup(l);
  else
    pccs->line = NULL;

  if(f)
    pccs->file_name = Safe_strdup(f);
  else
    pccs->file_name = NULL;

  return ( (pCode *)pccs);

}


/*******************************************************************/
/* pic16_newpCodeAsmDir - create a new pCode Assembler Directive   */
/*                      added by VR 6-Jun-2003                     */
/*******************************************************************/

pCode *pic16_newpCodeAsmDir(char *asdir, char *argfmt, ...)
{
  pCodeAsmDir *pcad;
  va_list ap;
  char buffer[512];
  char *lbp=buffer;

        pcad = Safe_calloc(1, sizeof(pCodeAsmDir));
        pcad->pci.pc.type = PC_ASMDIR;
        pcad->pci.pc.prev = pcad->pci.pc.next = NULL;
        pcad->pci.pc.pb = NULL;
        pcad->pci.isize = 2;
        pcad->pci.pc.destruct = genericDestruct;
        pcad->pci.pc.print = genericPrint;

        if(asdir && *asdir) {

                while(isspace((unsigned char)*asdir))asdir++;   // strip any white space from the beginning

                pcad->directive = Safe_strdup( asdir );
        }

        va_start(ap, argfmt);

        memset(buffer, 0, sizeof(buffer));
        if(argfmt && *argfmt)
                vsprintf(buffer, argfmt, ap);

        va_end(ap);

        while(isspace((unsigned char)*lbp))lbp++;

        if(lbp && *lbp)
                pcad->arg = Safe_strdup( lbp );

  return ((pCode *)pcad);
}

/*-----------------------------------------------------------------*/
/* pCodeLabelDestruct - free memory used by a label.               */
/*-----------------------------------------------------------------*/
static void pCodeLabelDestruct(pCode *pc)
{

  if(!pc)
    return;

  pic16_unlinkpCode(pc);

//  if((pc->type == PC_LABEL) && PCL(pc)->label)
//    Safe_free(PCL(pc)->label);

  /* Instead of deleting the memory used by this pCode, mark
   * the object as bad so that if there's a pointer to this pCode
   * dangling around somewhere then (hopefully) when the type is
   * checked we'll catch it.
   */

  pc->type = PC_BAD;
  pic16_addpCode2pBlock(pb_dead_pcodes, pc);

//  Safe_free(pc);

}

pCode *pic16_newpCodeLabel(char *name, int key)
{

  char *s = buffer;
  pCodeLabel *pcl;

  pcl = Safe_calloc(1,sizeof(pCodeLabel) );

  pcl->pc.type = PC_LABEL;
  pcl->pc.prev = pcl->pc.next = NULL;
  //pcl->pc.from = pcl->pc.to = pcl->pc.label = NULL;
  pcl->pc.pb = NULL;

  //  pcl->pc.analyze = genericAnalyze;
  pcl->pc.destruct = pCodeLabelDestruct;
  pcl->pc.print = pCodePrintLabel;

  pcl->key = key;
  pcl->force = 0;

  pcl->label = NULL;
  if(key>0) {
    sprintf(s,"_%05d_DS_",key);
  } else
    s = name;

  if(s)
    pcl->label = Safe_strdup(s);

//  if(pic16_pcode_verbose)
//      fprintf(stderr, "%s:%d label name: %s\n", __FILE__, __LINE__, pcl->label);


  return ( (pCode *)pcl);

}

pCode *pic16_newpCodeLabelFORCE(char *name, int key)
{
  pCodeLabel *pcl = (pCodeLabel *)pic16_newpCodeLabel(name, key);

        pcl->force = 1;

  return ( (pCode *)pcl );
}

pCode *pic16_newpCodeInfo(INFO_TYPE type, pCodeOp *pcop)
{
  pCodeInfo *pci;

    pci = Safe_calloc(1, sizeof(pCodeInfo));
    pci->pci.pc.type = PC_INFO;
    pci->pci.pc.prev = pci->pci.pc.next = NULL;
    pci->pci.pc.pb = NULL;
    pci->pci.label = NULL;

    pci->pci.pc.destruct = genericDestruct;
    pci->pci.pc.print = genericPrint;

    pci->type = type;
    pci->oper1 = pcop;

  return ((pCode *)pci);
}


/*-----------------------------------------------------------------*/
/* newpBlock - create and return a pointer to a new pBlock         */
/*-----------------------------------------------------------------*/
static pBlock *newpBlock(void)
{

  pBlock *PpB;

  PpB = Safe_calloc(1,sizeof(pBlock) );
  PpB->next = PpB->prev = NULL;

  PpB->function_entries = PpB->function_exits = PpB->function_calls = NULL;
  PpB->tregisters = NULL;
  PpB->visited = 0;
  PpB->FlowTree = NULL;

  return PpB;

}

/*-----------------------------------------------------------------*/
/* pic16_newpCodeChain - create a new chain of pCodes                    */
/*-----------------------------------------------------------------*
 *
 *  This function will create a new pBlock and the pointer to the
 *  pCode that is passed in will be the first pCode in the block.
 *-----------------------------------------------------------------*/


pBlock *pic16_newpCodeChain(memmap *cm,char c, pCode *pc)
{

  pBlock *pB  = newpBlock();

  pB->pcHead  = pB->pcTail = pc;
  pB->cmemmap = cm;
  pB->dbName  = c;

  return pB;
}



/*-----------------------------------------------------------------*/
/* pic16_newpCodeOpLabel - Create a new label given the key              */
/*  Note, a negative key means that the label is part of wild card */
/*  (and hence a wild card label) used in the pCodePeep            */
/*   optimizations).                                               */
/*-----------------------------------------------------------------*/

pCodeOp *pic16_newpCodeOpLabel(char *name, int key)
{
  char *s=NULL;
  static int label_key=-1;

  pCodeOp *pcop;

  pcop = Safe_calloc(1,sizeof(pCodeOpLabel) );
  pcop->type = PO_LABEL;

  pcop->name = NULL;

  if(key>0)
    sprintf(s=buffer,"_%05d_DS_",key);
  else
    s = name, key = label_key--;

  if(s)
    pcop->name = Safe_strdup(s);

  ((pCodeOpLabel *)pcop)->key = key;

  //fprintf(stderr,"pic16_newpCodeOpLabel: key=%d, name=%s\n",key,((s)?s:""));
  return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *pic16_newpCodeOpLit(int lit)
{
  char *s = buffer;
  pCodeOp *pcop;


  pcop = Safe_calloc(1,sizeof(pCodeOpLit) );
  pcop->type = PO_LITERAL;

  pcop->name = NULL;
  //if(lit>=0)
    sprintf(s,"0x%02x", (unsigned char) lit);
  //else
  //  sprintf(s, "%i", lit);

  if(s)
    pcop->name = Safe_strdup(s);

  ((pCodeOpLit *)pcop)->lit = lit;

  return pcop;
}

/* Allow for 12 bit literals, required for LFSR */
pCodeOp *pic16_newpCodeOpLit12(int lit)
{
  char *s = buffer;
  pCodeOp *pcop;


  pcop = Safe_calloc(1,sizeof(pCodeOpLit) );
  pcop->type = PO_LITERAL;

  pcop->name = NULL;
  //if(lit>=0)
    sprintf(s,"0x%03x", ((unsigned int)lit) & 0x0fff);
  //else
  //  sprintf(s, "%i", lit);

  if(s)
    pcop->name = Safe_strdup(s);

  ((pCodeOpLit *)pcop)->lit = lit;

  return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *pic16_newpCodeOpLit2(int lit, pCodeOp *arg2)
{
  char *s = buffer, tbuf[256], *tb=tbuf;
  pCodeOp *pcop;


  tb = pic16_get_op(arg2, NULL, 0);
  pcop = Safe_calloc(1,sizeof(pCodeOpLit2) );
  pcop->type = PO_LITERAL;

  pcop->name = NULL;
  //if(lit>=0) {
    sprintf(s,"0x%02x, %s", (unsigned char)lit, tb);
    if(s)
      pcop->name = Safe_strdup(s);
  //}

  ((pCodeOpLit2 *)pcop)->lit = lit;
  ((pCodeOpLit2 *)pcop)->arg2 = arg2;

  return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *pic16_newpCodeOpImmd(char *name, int offset, int index, int code_space)
{
  pCodeOp *pcop;

        pcop = Safe_calloc(1,sizeof(pCodeOpImmd) );
        pcop->type = PO_IMMEDIATE;
        if(name) {
                reg_info *r = pic16_dirregWithName(name);
                pcop->name = Safe_strdup(name);
                PCOI(pcop)->r = r;

                if(r) {
//                      fprintf(stderr, "%s:%d %s reg %s exists (r: %p)\n",__FILE__, __LINE__, __FUNCTION__, name, r);
                        PCOI(pcop)->rIdx = r->rIdx;
                } else {
//                      fprintf(stderr, "%s:%d %s reg %s doesn't exist\n", __FILE__, __LINE__, __FUNCTION__, name);
                        PCOI(pcop)->rIdx = -1;
                }
//                      fprintf(stderr,"%s %s %d\n",__FUNCTION__,name,offset);
        } else {
                pcop->name = NULL;
                PCOI(pcop)->rIdx = -1;
        }

        PCOI(pcop)->index = index;
        PCOI(pcop)->offset = offset;
        PCOI(pcop)->_const = code_space;

  return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *pic16_newpCodeOpWild(int id, pCodeWildBlock *pcwb, pCodeOp *subtype)
{
  char *s = buffer;
  pCodeOp *pcop;


  if(!pcwb || !subtype) {
    fprintf(stderr, "Wild opcode declaration error: %s-%d\n",__FILE__,__LINE__);
    exit(1);
  }

  pcop = Safe_calloc(1,sizeof(pCodeOpWild));
  pcop->type = PO_WILD;
  sprintf(s,"%%%d",id);
  pcop->name = Safe_strdup(s);

  PCOW(pcop)->id = id;
  PCOW(pcop)->pcwb = pcwb;
  PCOW(pcop)->subtype = subtype;
  PCOW(pcop)->matched = NULL;

  PCOW(pcop)->pcop2 = NULL;

  return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *pic16_newpCodeOpWild2(int id, int id2, pCodeWildBlock *pcwb, pCodeOp *subtype, pCodeOp *subtype2)
{
  char *s = buffer;
  pCodeOp *pcop;


        if(!pcwb || !subtype || !subtype2) {
                fprintf(stderr, "Wild opcode declaration error: %s-%d\n",__FILE__,__LINE__);
                exit(1);
        }

        pcop = Safe_calloc(1,sizeof(pCodeOpWild));
        pcop->type = PO_WILD;
        sprintf(s,"%%%d",id);
        pcop->name = Safe_strdup(s);

        PCOW(pcop)->id = id;
        PCOW(pcop)->pcwb = pcwb;
        PCOW(pcop)->subtype = subtype;
        PCOW(pcop)->matched = NULL;

        PCOW(pcop)->pcop2 = Safe_calloc(1, sizeof(pCodeOpWild));

        if(!subtype2->name) {
                PCOW(pcop)->pcop2 = Safe_calloc(1, sizeof(pCodeOpWild));
                PCOW2(pcop)->pcop.type = PO_WILD;
                sprintf(s, "%%%d", id2);
                PCOW2(pcop)->pcop.name = Safe_strdup(s);
                PCOW2(pcop)->id = id2;
                PCOW2(pcop)->subtype = subtype2;

//              fprintf(stderr, "%s:%d %s [wild,wild] for name: %s (%d)\tname2: %s (%d)\n", __FILE__, __LINE__, __FUNCTION__,
//                              pcop->name, id, PCOW2(pcop)->pcop.name, id2);
        } else {
                PCOW2(pcop)->pcop2 = pic16_pCodeOpCopy( subtype2 );

//              fprintf(stderr, "%s:%d %s [wild,str] for name: %s (%d)\tname2: %s (%d)\n", __FILE__, __LINE__, __FUNCTION__,
//                              pcop->name, id, PCOW2(pcop)->pcop.name, id2);
        }



  return pcop;
}


/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *pic16_newpCodeOpBit(char *s, int bit, int inBitSpace, PIC_OPTYPE subt)
{
  pCodeOp *pcop;

  pcop = Safe_calloc(1,sizeof(pCodeOpRegBit) );
  pcop->type = PO_GPR_BIT;
  if(s)
    pcop->name = Safe_strdup(s);
  else
    pcop->name = NULL;

  PCORB(pcop)->bit = bit;
  PCORB(pcop)->inBitSpace = inBitSpace;
  PCORB(pcop)->subtype = subt;

  /* pCodeOpBit is derived from pCodeOpReg. We need to init this too */
  PCOR(pcop)->r = pic16_regWithName(s); //NULL;
//  fprintf(stderr, "%s:%d %s for reg: %s\treg= %p\n", __FILE__, __LINE__, __FUNCTION__, s, PCOR(pcop)->r);
//  PCOR(pcop)->rIdx = 0;
  return pcop;
}

pCodeOp *pic16_newpCodeOpBit_simple (struct asmop *op, int offs, int bit)
{
  return pic16_newpCodeOpBit (pic16_aopGet(op,offs,FALSE,FALSE),
                                bit, 0, PO_GPR_REGISTER);
}


/*-----------------------------------------------------------------*
 * pCodeOp *pic16_newpCodeOpReg(int rIdx) - allocate a new register
 *
 * If rIdx >=0 then a specific register from the set of registers
 * will be selected. If rIdx <0, then a new register will be searched
 * for.
 *-----------------------------------------------------------------*/

pCodeOp *pic16_newpCodeOpReg(int rIdx)
{
  pCodeOp *pcop;
  reg_info *r;

  pcop = Safe_calloc(1,sizeof(pCodeOpReg) );

  pcop->name = NULL;

  if(rIdx >= 0) {
        r = pic16_regWithIdx(rIdx);
        if(!r)
                r = pic16_allocWithIdx(rIdx);
  } else {
    r = pic16_findFreeReg(REG_GPR);

    if(!r) {
        fprintf(stderr, "%s:%d Could not find a free GPR register\n",
                __FUNCTION__, __LINE__);
        exit(EXIT_FAILURE);
    }
  }

  PCOR(pcop)->rIdx = rIdx;
  PCOR(pcop)->r = r;
  pcop->type = PCOR(pcop)->r->pc_type;

  return pcop;
}

pCodeOp *pic16_newpCodeOpRegNotVect(bitVect *bv)
{
  pCodeOp *pcop;
  reg_info *r;

    pcop = Safe_calloc(1, sizeof(pCodeOpReg));
    pcop->name = NULL;

    r = pic16_findFreeReg(REG_GPR);

    while(r) {
      if(!bitVectBitValue(bv, r->rIdx)) {
        PCOR(pcop)->r = r;
        PCOR(pcop)->rIdx = r->rIdx;
        pcop->type = r->pc_type;
        return (pcop);
      }

      r = pic16_findFreeRegNext(REG_GPR, r);
    }

  return NULL;
}



pCodeOp *pic16_newpCodeOpRegFromStr(char *name)
{
  pCodeOp *pcop;
  reg_info *r;

        pcop = Safe_calloc(1,sizeof(pCodeOpReg) );
        PCOR(pcop)->r = r = pic16_allocRegByName(name, 1, NULL);
        PCOR(pcop)->rIdx = PCOR(pcop)->r->rIdx;
        pcop->type = PCOR(pcop)->r->pc_type;
        pcop->name = PCOR(pcop)->r->name;

//      if(pic16_pcode_verbose) {
//              fprintf(stderr, "%s:%d %s allocates register %s rIdx:0x%02x\n",
//                      __FILE__, __LINE__, __FUNCTION__, r->name, r->rIdx);
//      }

  return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *pic16_newpCodeOpOpt(OPT_TYPE type, char *key)
{
  pCodeOpOpt *pcop;

        pcop = Safe_calloc(1, sizeof(pCodeOpOpt));

        pcop->type = type;
        pcop->key = Safe_strdup( key );

  return (PCOP(pcop));
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *pic16_newpCodeOpLocalRegs(LR_TYPE type)
{
  pCodeOpLocalReg *pcop;

        pcop = Safe_calloc(1, sizeof(pCodeOpLocalReg));

        pcop->type = type;

  return (PCOP(pcop));
}


/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

pCodeOp *pic16_newpCodeOp(char *name, PIC_OPTYPE type)
{
  pCodeOp *pcop;

  switch(type) {
  case PO_BIT:
  case PO_GPR_BIT:
    pcop = pic16_newpCodeOpBit(name, -1,0, type);
    break;

  case PO_LITERAL:
    pcop = pic16_newpCodeOpLit(-1);
    break;

  case PO_LABEL:
    pcop = pic16_newpCodeOpLabel(NULL,-1);
    break;
  case PO_GPR_TEMP:
    pcop = pic16_newpCodeOpReg(-1);
    break;

  case PO_GPR_REGISTER:
    if(name)
      pcop = pic16_newpCodeOpRegFromStr(name);
    else
      pcop = pic16_newpCodeOpReg(-1);
    break;

  case PO_TWO_OPS:
    assert( !"Cannot create PO_TWO_OPS from string!" );
    pcop = NULL;
    break;

  default:
    pcop = Safe_calloc(1,sizeof(pCodeOp) );
    pcop->type = type;
    if(name)
      pcop->name = Safe_strdup(name);
    else
      pcop->name = NULL;
  }

  return pcop;
}

pCodeOp *pic16_newpCodeOp2(pCodeOp *src, pCodeOp *dst)
{
  pCodeOp2 *pcop2 = Safe_calloc(1, sizeof(pCodeOp2));
  pcop2->pcop.type = PO_TWO_OPS;
  pcop2->pcopL = src;
  pcop2->pcopR = dst;
  return PCOP(pcop2);
}

/* This is a multiple of two as gpasm pads DB directives to even length,
 * thus the data would be interleaved with \0 bytes...
 * This is a multiple of three in order to have arrays of 3-byte pointers
 * continuously in memory (without 0-padding at the lines' end).
 * This is rather 12 than 6 in order not to split up 4-byte data types
 * in arrays right in the middle of a 4-byte word. */
#define DB_ITEMS_PER_LINE       12

typedef struct DBdata
  {
    int count;
    char buffer[512];
  } DBdata;

struct DBdata DBd;
static int DBd_init = -1;

/*-----------------------------------------------------------------*/
/*    Initialiase "DB" data buffer                                 */
/*-----------------------------------------------------------------*/
void pic16_initDB(void)
{
        DBd_init = -1;
}


/*-----------------------------------------------------------------*/
/*    Flush pending "DB" data to a pBlock                          */
/*                                                                 */
/* ptype - type of p pointer, 'f' file pointer, 'p' pBlock pointer */
/*-----------------------------------------------------------------*/
void pic16_flushDB(char ptype, void *p)
{
        if (DBd.count>0) {
                if(ptype == 'p')
                        pic16_addpCode2pBlock(((pBlock *)p),pic16_newpCodeAsmDir("DB", "%s", DBd.buffer));
                else
                if(ptype == 'f')
                        fprintf(((FILE *)p), "\tdb\t%s\n", DBd.buffer);
                else {
                        /* sanity check */
                        fprintf(stderr, "PIC16 port error: could not emit initial value data\n");
                }

                DBd.count = 0;
                DBd.buffer[0] = '\0';
        }
}


/*-----------------------------------------------------------------*/
/*    Add "DB" directives to a pBlock                              */
/*-----------------------------------------------------------------*/
void pic16_emitDB(int c, char ptype, void *p)
{
  int l;

        if (DBd_init<0) {
         // we need to initialize
                DBd_init = 0;
                DBd.count = 0;
                DBd.buffer[0] = '\0';
        }

        l = strlen(DBd.buffer);
        sprintf(DBd.buffer+l,"%s0x%02x", (DBd.count>0?", ":""), c & 0xff);

//      fprintf(stderr, "%s:%d DBbuffer: '%s'\n", __FILE__, __LINE__, DBd.buffer);

        DBd.count++;
        if (DBd.count>= DB_ITEMS_PER_LINE)
                pic16_flushDB(ptype, p);
}

void pic16_emitDS(char *s, char ptype, void *p)
{
  int l;

        if (DBd_init<0) {
         // we need to initialize
                DBd_init = 0;
                DBd.count = 0;
                DBd.buffer[0] = '\0';
        }

        l = strlen(DBd.buffer);
        sprintf(DBd.buffer+l,"%s%s", (DBd.count>0?", ":""), s);

//      fprintf(stderr, "%s:%d DBbuffer: '%s'\n", __FILE__, __LINE__, DBd.buffer);

        DBd.count++;    //=strlen(s);
        if (DBd.count>=DB_ITEMS_PER_LINE)
                pic16_flushDB(ptype, p);
}


/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
void pic16_pCodeConstString(char *name, const char *value, unsigned length)
{
  pBlock *pb;
  char *item;
  static set *emittedSymbols = NULL;

  if(!name || !value)
    return;

  /* keep track of emitted symbols to avoid multiple definition of str_<nr> */
  if (emittedSymbols) {
    /* scan set for name */
    for (item = setFirstItem (emittedSymbols); item; item = setNextItem (emittedSymbols))
    {
      if (!strcmp (item,name)) {
        //fprintf (stderr, "%s already emitted\n", name);
        return;
      } // if
    } // for
  } // if
  addSet (&emittedSymbols, Safe_strdup (name));

  //fprintf(stderr, " %s  %s  %s\n",__FUNCTION__,name,value);

  pb = pic16_newpCodeChain(NULL, 'P',pic16_newpCodeCharP("; Starting pCode block"));

  pic16_addpBlock(pb);

//  sprintf(buffer,"; %s = ", name);
//  strcat(buffer, value);
//  fputs(buffer, stderr);

//  pic16_addpCode2pBlock(pb,pic16_newpCodeCharP(buffer));
  pic16_addpCode2pBlock(pb,pic16_newpCodeLabel(name,-1));

  while (length--)
    pic16_emitDB(*value++, 'p', (void *)pb);

  pic16_flushDB('p', (void *)pb);
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
#if 0
static void pCodeReadCodeTable(void)
{
  pBlock *pb;

  fprintf(stderr, " %s\n",__FUNCTION__);

  pb = pic16_newpCodeChain(NULL, 'P',pic16_newpCodeCharP("; Starting pCode block"));

  pic16_addpBlock(pb);

  pic16_addpCode2pBlock(pb,pic16_newpCodeCharP("; ReadCodeTable - built in function"));
  pic16_addpCode2pBlock(pb,pic16_newpCodeCharP("; Inputs: temp1,temp2 = code pointer"));
  pic16_addpCode2pBlock(pb,pic16_newpCodeCharP("; Outpus: W (from RETLW at temp2:temp1)"));
  pic16_addpCode2pBlock(pb,pic16_newpCodeLabel("ReadCodeTable:",-1));

  pic16_addpCode2pBlock(pb,pic16_newpCode(POC_MOVFW,pic16_newpCodeOpRegFromStr("temp2")));
  pic16_addpCode2pBlock(pb,pic16_newpCode(POC_MOVWF,pic16_newpCodeOpRegFromStr("PCLATH")));
  pic16_addpCode2pBlock(pb,pic16_newpCode(POC_MOVFW,pic16_newpCodeOpRegFromStr("temp1")));
  pic16_addpCode2pBlock(pb,pic16_newpCode(POC_MOVWF,pic16_newpCodeOpRegFromStr("PCL")));


}
#endif
/*-----------------------------------------------------------------*/
/* pic16_addpCode2pBlock - place the pCode into the pBlock linked list   */
/*-----------------------------------------------------------------*/
void pic16_addpCode2pBlock(pBlock *pb, pCode *pc)
{

  if(!pc)
    return;

  if(!pb->pcHead) {
    /* If this is the first pcode to be added to a block that
     * was initialized with a NULL pcode, then go ahead and
     * make this pcode the head and tail */
    pb->pcHead  = pb->pcTail = pc;
  } else {
    //    if(pb->pcTail)
    pb->pcTail->next = pc;

    pc->prev = pb->pcTail;
    pc->pb = pb;

    pb->pcTail = pc;
  }
}

/*-----------------------------------------------------------------*/
/* pic16_addpBlock - place a pBlock into the pFile                 */
/*-----------------------------------------------------------------*/
void pic16_addpBlock(pBlock *pb)
{
  // fprintf(stderr," Adding pBlock: dbName =%c\n",getpBlock_dbName(pb));

  if(!the_pFile) {
    /* First time called, we'll pass through here. */
    //_ALLOC(the_pFile,sizeof(pFile));
    the_pFile = Safe_calloc(1,sizeof(pFile));
    the_pFile->pbHead = the_pFile->pbTail = pb;
    the_pFile->functions = NULL;
    return;
  }

  the_pFile->pbTail->next = pb;
  pb->prev = the_pFile->pbTail;
  pb->next = NULL;
  the_pFile->pbTail = pb;
}

/*-----------------------------------------------------------------*/
/* removepBlock - remove a pBlock from the pFile                   */
/*-----------------------------------------------------------------*/
static void removepBlock(pBlock *pb)
{
  pBlock *pbs;

  if(!the_pFile)
    return;


  //fprintf(stderr," Removing pBlock: dbName =%c\n",getpBlock_dbName(pb));

  for(pbs = the_pFile->pbHead; pbs; pbs = pbs->next) {
    if(pbs == pb) {

      if(pbs == the_pFile->pbHead)
        the_pFile->pbHead = pbs->next;

      if (pbs == the_pFile->pbTail)
        the_pFile->pbTail = pbs->prev;

      if(pbs->next)
        pbs->next->prev = pbs->prev;

      if(pbs->prev)
        pbs->prev->next = pbs->next;

      return;

    }
  }

  fprintf(stderr, "Warning: call to %s:%s didn't find pBlock\n",__FILE__,__FUNCTION__);

}

/*-----------------------------------------------------------------*/
/* printpCode - write the contents of a pCode to a file            */
/*-----------------------------------------------------------------*/
static void printpCode(FILE *of, pCode *pc)
{

  if(!pc || !of)
    return;

  if(pc->print) {
    pc->print(of,pc);
    return;
  }

  fprintf(of,"warning - unable to print pCode\n");
}

/*-----------------------------------------------------------------*/
/* pic16_printpBlock - write the contents of a pBlock to a file    */
/*-----------------------------------------------------------------*/
void pic16_printpBlock(FILE *of, pBlock *pb)
{
  pCode *pc;

        if(!pb)return;

        if(!of)of=stderr;

        for(pc = pb->pcHead; pc; pc = pc->next) {
                if(isPCF(pc) && PCF(pc)->fname) {
                        fprintf(of, "S_%s_%s\tcode", PCF(pc)->modname, PCF(pc)->fname);
                        if(pb->dbName == 'A') {
                          absSym *ab;
                                for(ab=setFirstItem(absSymSet); ab; ab=setNextItem(absSymSet)) {
//                                      fprintf(stderr, "%s:%d testing %s <-> %s\n", __FILE__, __LINE__, PCF(pc)->fname, ab->name);
                                        if(!strcmp(ab->name, PCF(pc)->fname)) {
//                                              fprintf(stderr, "%s:%d address = %x\n", __FILE__, __LINE__, ab->address);
                                                if(ab->address != -1)
                                                  fprintf(of, "\t0X%06X", ab->address);
                                                break;
                                        }
                                }
                        }
                        fprintf(of, "\n");
                }
                printpCode(of,pc);
        }
}

/*-----------------------------------------------------------------*/
/*                                                                 */
/*       pCode processing                                          */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*-----------------------------------------------------------------*/
pCode * pic16_findNextInstruction(pCode *pci);
pCode * pic16_findPrevInstruction(pCode *pci);

void pic16_unlinkpCode(pCode *pc)
{
  pCode *prev;

  if(pc) {
#ifdef PCODE_DEBUG
    fprintf(stderr,"Unlinking: ");
    printpCode(stderr, pc);
#endif
    if(pc->prev) {
      pc->prev->next = pc->next;
    } else if (pc->pb && (pc->pb->pcHead == pc)) {
        pc->pb->pcHead = pc->next;
    }
    if(pc->next) {
      pc->next->prev = pc->prev;
    } else if (pc->pb && (pc->pb->pcTail == pc)) {
        pc->pb->pcTail = pc->prev;
    }

    /* move C source line down (or up) */
    if (isPCI(pc) && PCI(pc)->cline) {
      prev = pic16_findNextInstruction (pc->next);
      if (prev && isPCI(prev) && !PCI(prev)->cline) {
        PCI(prev)->cline = PCI(pc)->cline;
      } else {
        prev = pic16_findPrevInstruction (pc->prev);
        if (prev && isPCI(prev) && !PCI(prev)->cline)
          PCI(prev)->cline = PCI(pc)->cline;
      }
    }
    pc->prev = pc->next = NULL;
  }
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

static void genericDestruct(pCode *pc)
{

  pic16_unlinkpCode(pc);

  if(isPCI(pc)) {
    /* For instructions, tell the register (if there's one used)
     * that it's no longer needed */
    reg_info *reg = pic16_getRegFromInstruction(pc);
    if(reg)
      deleteSetItem (&(reg->reglives.usedpCodes),pc);

        if(PCI(pc)->is2MemOp) {
                reg = pic16_getRegFromInstruction2(pc);
                if(reg)
                        deleteSetItem(&(reg->reglives.usedpCodes), pc);
        }
  }

  /* Instead of deleting the memory used by this pCode, mark
   * the object as bad so that if there's a pointer to this pCode
   * dangling around somewhere then (hopefully) when the type is
   * checked we'll catch it.
   */

  pc->type = PC_BAD;
  pic16_addpCode2pBlock(pb_dead_pcodes, pc);

  //Safe_free(pc);
}


void DEBUGpic16_emitcode (char *inst,char *fmt, ...);
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
/* modifiers for constant immediate */
const char *immdmod[3]={"LOW", "HIGH", "UPPER"};

char *pic16_get_op(pCodeOp *pcop,char *buffer, size_t size)
{
    reg_info *r;
    static char b[128];
    char *s;
    int use_buffer = 1;    // copy the string to the passed buffer pointer

    if(!buffer) {
        buffer = b;
        size = sizeof(b);
        use_buffer = 0;     // Don't bother copying the string to the buffer.
    }

    if(pcop) {

        switch(pcop->type) {
            case PO_W:
            case PO_WREG:
            case PO_PRODL:
            case PO_PRODH:
            case PO_INDF0:
            case PO_FSR0:
                if(use_buffer) {
                    SNPRINTF(buffer,size,"%s",PCOR(pcop)->r->name);
                    return (buffer);
                }
                return (PCOR(pcop)->r->name);
                break;
            case PO_GPR_TEMP:
                r = pic16_regWithIdx(PCOR(pcop)->r->rIdx);
                if(use_buffer) {
                    SNPRINTF(buffer,size,"%s",r->name);
                    return (buffer);
                }
                return (r->name);
                break;

            case PO_IMMEDIATE:
                s = buffer;
                if(PCOI(pcop)->offset && PCOI(pcop)->offset<4) {
                    if(PCOI(pcop)->index) {
                        SNPRINTF(s,size, "%s(%s + %d)",
                                immdmod[ PCOI(pcop)->offset ],
                                pcop->name,
                                PCOI(pcop)->index);
                    } else {
                        SNPRINTF(s,size,"%s(%s)",
                                immdmod[ PCOI(pcop)->offset ],
                                pcop->name);
                    }
                } else {
                    if(PCOI(pcop)->index) {
                        SNPRINTF(s,size, "%s(%s + %d)",
                                immdmod[ 0 ],
                                pcop->name,
                                PCOI(pcop)->index);
                    } else {
                        SNPRINTF(s,size, "%s(%s)",
                                immdmod[ 0 ],
                                pcop->name);
                    }
                }
                return (buffer);
                break;

            case PO_GPR_REGISTER:
            case PO_DIR:
                s = buffer;
                //size = sizeof(buffer);
                if( PCOR(pcop)->instance) {
                    SNPRINTF(s,size,"(%s + %d)",
                            pcop->name,
                            PCOR(pcop)->instance );
                } else {
                    SNPRINTF(s,size,"%s",pcop->name);
                }
                return (buffer);
                break;

            case PO_GPR_BIT:
                s = buffer;
                if(PCORB(pcop)->subtype == PO_GPR_TEMP) {
                    SNPRINTF(s, size, "%s", pcop->name);
                } else {
                    if(PCORB(pcop)->pcor.instance)
                        SNPRINTF(s, size, "(%s + %d)", pcop->name, PCORB(pcop)->pcor.instance);
                    else
                        SNPRINTF(s, size, "%s", pcop->name);
                }
                return (buffer);
                break;

            case PO_TWO_OPS:
                return (pic16_get_op( PCOP2(pcop)->pcopL, use_buffer ? buffer : NULL, size ));
                break;

            default:
                if(pcop->name) {
                    if(use_buffer) {
                        SNPRINTF(buffer,size,"%s",pcop->name);
                        return (buffer);
                    }
                    return (pcop->name);
                }

        }
        return ("unhandled type for op1");
    }

    return ("NO operand1");
}

/*-----------------------------------------------------------------*/
/* pic16_get_op2 - variant to support two memory operand commands  */
/*-----------------------------------------------------------------*/
char *pic16_get_op2(pCodeOp *pcop,char *buffer, size_t size)
{

  if(pcop && pcop->type == PO_TWO_OPS) {
    return pic16_get_op( PCOP2(pcop)->pcopR, buffer, size );
  }

  return "NO operand2";
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static char *pic16_get_op_from_instruction( pCodeInstruction *pcc)
{

  if(pcc )
    return pic16_get_op(pcc->pcop,NULL,0);

  /* gcc 3.2:  warning: concatenation of string literals with __FUNCTION__ is deprecated
   *   return ("ERROR Null: "__FUNCTION__);
   */
  return ("ERROR Null: pic16_get_op_from_instruction");

}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void pCodeOpPrint(FILE *of, pCodeOp *pcop)
{

  fprintf(of,"pcodeopprint- not implemented\n");
}

/*-----------------------------------------------------------------*/
/* pic16_pCode2str - convert a pCode instruction to string               */
/*-----------------------------------------------------------------*/
char *pic16_pCode2str(char *str, size_t size, pCode *pc)
{
    char *s = str;
    reg_info *r;

#if 0
    if(isPCI(pc) && (PCI(pc)->pci_magic != PCI_MAGIC)) {
        fprintf(stderr, "%s:%d: pCodeInstruction initialization error in instruction %s, magic is %x (defaut: %x)\n",
                __FILE__, __LINE__, PCI(pc)->mnemonic, PCI(pc)->pci_magic, PCI_MAGIC);
        //              exit(EXIT_FAILURE);
    }
#endif

    switch(pc->type) {

        case PC_OPCODE:
            SNPRINTF(s, size, "\t%s\t", PCI(pc)->mnemonic);
            size -= strlen(s);
            s += strlen(s);

            if( (PCI(pc)->num_ops >= 1) && (PCI(pc)->pcop)) {

                if (PCI(pc)->pcop->type == PO_TWO_OPS)
                {
                    /* split into two phases due to static buffer in pic16_get_op() */
                    SNPRINTF(s, size, "%s",
                            pic16_get_op((PCI(pc)->pcop), NULL, 0));
                    size -= strlen(s);
                    s += strlen(s);
                    SNPRINTF(s, size, ", %s",
                            pic16_get_op2((PCI(pc)->pcop), NULL, 0));
                    break;
                }

                if(PCI(pc)->is2LitOp) {
                    SNPRINTF(s,size, "%s", PCOP(PCI(pc)->pcop)->name);
                    break;
                }

                if(PCI(pc)->isBitInst) {
                    if(PCI(pc)->pcop->type != PO_GPR_BIT) {
                        if( (((pCodeOpRegBit *)(PCI(pc)->pcop))->inBitSpace) )
                            SNPRINTF(s,size,"(%s >> 3), (%s & 7)",
                                    PCI(pc)->pcop->name ,
                                    PCI(pc)->pcop->name );
                        else
                            SNPRINTF(s,size,"%s,%d", pic16_get_op_from_instruction(PCI(pc)),
                                    (((pCodeOpRegBit *)(PCI(pc)->pcop))->bit ));

                    } else if(PCI(pc)->pcop->type == PO_GPR_BIT) {
                        SNPRINTF(s,size,"%s, %d", pic16_get_op_from_instruction(PCI(pc)),PCORB(PCI(pc)->pcop)->bit);
                    } else
                        SNPRINTF(s,size,"%s,0 ; ?bug", pic16_get_op_from_instruction(PCI(pc)));
                } else {

                    if(PCI(pc)->pcop->type == PO_GPR_BIT) {
                        if( PCI(pc)->num_ops == 3)
                            SNPRINTF(s,size,"(%s >> 3),%c",pic16_get_op_from_instruction(PCI(pc)),((PCI(pc)->isModReg) ? 'F':'W'));
                        else
                            SNPRINTF(s,size,"(1 << (%s & 7))",pic16_get_op_from_instruction(PCI(pc)));
                    } else {
                        SNPRINTF(s,size,"%s", pic16_get_op_from_instruction(PCI(pc)));
                    }
                }

                if( PCI(pc)->num_ops == 3 || ((PCI(pc)->num_ops == 2) && (PCI(pc)->isAccess))) {
                    size -= strlen(s);
                    s += strlen(s);
                    if(PCI(pc)->num_ops == 3 && !PCI(pc)->isBitInst) {
                        SNPRINTF(s,size,", %c", ( (PCI(pc)->isModReg) ? 'F':'W'));
                        size -= strlen(s);
                        s += strlen(s);
                    }

                    r = pic16_getRegFromInstruction(pc);

                    if(PCI(pc)->isAccess) {
                        static char *bank_spec[2][2] = {
                            { "", ", ACCESS" },  /* gpasm uses access bank by default */
                            { ", B", ", BANKED" }/* MPASM (should) use BANKED by default */
                        };

                        SNPRINTF(s,size,"%s", bank_spec[(r && !isACCESS_BANK(r)) ? 1 : 0][pic16_mplab_comp ? 1 : 0]);
                    }
                }
            }
            break;

        case PC_COMMENT:
            /* assuming that comment ends with a \n */
            SNPRINTF(s,size,";%s", ((pCodeComment *)pc)->comment);
            break;

        case PC_INFO:
            SNPRINTF(s,size,"; info ==>");
            size -= strlen(s);
            s += strlen(s);
            switch( PCINF(pc)->type ) {
                case INF_OPTIMIZATION:
                    SNPRINTF(s,size, " [optimization] %s\n", OPT_TYPE_STR[ PCOO(PCINF(pc)->oper1)->type ]);
                    break;
                case INF_LOCALREGS:
                    SNPRINTF(s,size, " [localregs] %s\n", LR_TYPE_STR[ PCOLR(PCINF(pc)->oper1)->type ]);
                    break;
            }; break;

        case PC_INLINE:
            /* assuming that inline code ends with a \n */
            SNPRINTF(s,size,"%s", ((pCodeComment *)pc)->comment);
            break;

        case PC_LABEL:
            SNPRINTF(s,size,";label=%s, key=%d\n",PCL(pc)->label,PCL(pc)->key);
            break;
        case PC_FUNCTION:
            SNPRINTF(s,size,";modname=%s,function=%s: id=%d\n",PCF(pc)->modname,PCF(pc)->fname);
            break;
        case PC_WILD:
            SNPRINTF(s,size,";\tWild opcode: id=%d\n",PCW(pc)->id);
            break;
        case PC_FLOW:
            SNPRINTF(s,size,";\t--FLOW change\n");
            break;
        case PC_CSOURCE:
            SNPRINTF(s,size,"%s\t.line\t%d; %s\t%s\n", ((pic16_mplab_comp || !options.debug)?";":""),
                    PCCS(pc)->line_number, PCCS(pc)->file_name, PCCS(pc)->line);
            break;
        case PC_ASMDIR:
            if(PCAD(pc)->directive) {
                SNPRINTF(s,size,"\t%s%s%s\n", PCAD(pc)->directive, PCAD(pc)->arg?"\t":"", PCAD(pc)->arg?PCAD(pc)->arg:"");
            } else
                if(PCAD(pc)->arg) {
                    /* special case to handle inline labels without a tab */
                    SNPRINTF(s,size,"%s\n", PCAD(pc)->arg);
                }
            break;

        case PC_BAD:
            SNPRINTF(s,size,";A bad pCode is being used\n");
            break;
    }

    return str;
}

/*-----------------------------------------------------------------*/
/* genericPrint - the contents of a pCode to a file                */
/*-----------------------------------------------------------------*/
static void genericPrint(FILE *of, pCode *pc)
{

  if(!pc || !of)
    return;

  switch(pc->type) {
  case PC_COMMENT:
//    fputs(((pCodeComment *)pc)->comment, of);
    fprintf(of,"; %s\n", ((pCodeComment *)pc)->comment);
    break;

  case PC_INFO:
    {
      pBranch *pbl = PCI(pc)->label;
      while(pbl && pbl->pc) {
        if(pbl->pc->type == PC_LABEL)
          pCodePrintLabel(of, pbl->pc);
        pbl = pbl->next;
      }
    }

    if(pic16_pcode_verbose) {
      fprintf(of, "; info ==>");
      switch(((pCodeInfo *)pc)->type) {
        case INF_OPTIMIZATION:
              fprintf(of, " [optimization] %s\n", OPT_TYPE_STR[ PCOO(PCINF(pc)->oper1)->type ]);
              break;
        case INF_LOCALREGS:
              fprintf(of, " [localregs] %s\n", LR_TYPE_STR[ PCOLR(PCINF(pc)->oper1)->type ]);
              break;
        }
    };

    break;

  case PC_INLINE:
    fprintf(of,"%s\n", ((pCodeComment *)pc)->comment);
     break;

  case PC_OPCODE:
    // If the opcode has a label, print that first
    {
      pBranch *pbl = PCI(pc)->label;
      while(pbl && pbl->pc) {
        if(pbl->pc->type == PC_LABEL)
          pCodePrintLabel(of, pbl->pc);
        pbl = pbl->next;
      }
    }

    if(PCI(pc)->cline)
      genericPrint(of,PCODE(PCI(pc)->cline));

    {
      char str[256];

      pic16_pCode2str(str, 256, pc);

      fprintf(of,"%s",str);
      /* Debug */
      if(pic16_debug_verbose) {
        fprintf(of, "\t;key=%03x",pc->seq);
        if(PCI(pc)->pcflow)
          fprintf(of,", flow seq=%03x",PCI(pc)->pcflow->pc.seq);
      }
    }
    fprintf(of, "\n");
    break;

  case PC_WILD:
    fprintf(of,";\tWild opcode: id=%d\n",PCW(pc)->id);
    if(PCW(pc)->pci.label)
      pCodePrintLabel(of, PCW(pc)->pci.label->pc);

    if(PCW(pc)->operand) {
      fprintf(of,";\toperand  ");
      pCodeOpPrint(of,PCW(pc)->operand );
    }
    break;

  case PC_FLOW:
    if(pic16_debug_verbose) {
      fprintf(of,";<>Start of new flow, seq=0x%x",pc->seq);
      if(PCFL(pc)->ancestor)
        fprintf(of," ancestor = 0x%x", PCODE(PCFL(pc)->ancestor)->seq);
      fprintf(of,"\n");

    }
    break;

  case PC_CSOURCE:
//    fprintf(of,";#CSRC\t%s %d\t\t%s\n", PCCS(pc)->file_name, PCCS(pc)->line_number, PCCS(pc)->line);
    fprintf(of,"%s\t.line\t%d; %s\t%s\n", ((pic16_mplab_comp || !options.debug)?";":""),
        PCCS(pc)->line_number, PCCS(pc)->file_name, PCCS(pc)->line);

    break;

  case PC_ASMDIR:
        {
          pBranch *pbl = PCAD(pc)->pci.label;
                while(pbl && pbl->pc) {
                        if(pbl->pc->type == PC_LABEL)
                                pCodePrintLabel(of, pbl->pc);
                        pbl = pbl->next;
                }
        }
        if(PCAD(pc)->directive) {
                fprintf(of, "\t%s%s%s\n", PCAD(pc)->directive, PCAD(pc)->arg?"\t":"", PCAD(pc)->arg?PCAD(pc)->arg:"");
        } else
        if(PCAD(pc)->arg) {
                /* special case to handle inline labels without tab */
                fprintf(of, "%s\n", PCAD(pc)->arg);
        }
        break;

  case PC_LABEL:
  default:
    fprintf(of,"unknown pCode type %d\n",pc->type);
  }

}

/*-----------------------------------------------------------------*/
/* pCodePrintFunction - prints function begin/end                  */
/*-----------------------------------------------------------------*/

static void pCodePrintFunction(FILE *of, pCode *pc)
{

  if(!pc || !of)
    return;

#if 0
  if( ((pCodeFunction *)pc)->modname)
    fprintf(of,"F_%s",((pCodeFunction *)pc)->modname);
#endif

  if(!PCF(pc)->absblock) {
      if(PCF(pc)->fname) {
      pBranch *exits = PCF(pc)->to;
      int i=0;

      fprintf(of,"%s:", PCF(pc)->fname);

      if(pic16_pcode_verbose)
        fprintf(of, "\t;Function start");

      fprintf(of, "\n");

      while(exits) {
        i++;
        exits = exits->next;
      }
      //if(i) i--;

      if(pic16_pcode_verbose)
        fprintf(of,"; %d exit point%c\n",i, ((i==1) ? ' ':'s'));

    } else {
        if((PCF(pc)->from &&
                PCF(pc)->from->pc->type == PC_FUNCTION &&
                PCF(PCF(pc)->from->pc)->fname) ) {

                if(pic16_pcode_verbose)
                        fprintf(of,"; exit point of %s\n",PCF(PCF(pc)->from->pc)->fname);
        } else {
                if(pic16_pcode_verbose)
                        fprintf(of,"; exit point [can't find entry point]\n");
        }
        fprintf(of, "\n");
    }
  }
}
/*-----------------------------------------------------------------*/
/* pCodePrintLabel - prints label                                  */
/*-----------------------------------------------------------------*/

static void pCodePrintLabel(FILE *of, pCode *pc)
{

  if(!pc || !of)
    return;

  if(PCL(pc)->label)
    fprintf(of,"%s:\n",PCL(pc)->label);
  else if (PCL(pc)->key >=0)
    fprintf(of,"_%05d_DS_:\n",PCL(pc)->key);
  else
    fprintf(of,";wild card label: id=%d\n",-PCL(pc)->key);

}
/*-----------------------------------------------------------------*/
/* unlinkpCodeFromBranch - Search for a label in a pBranch and     */
/*                         remove it if it is found.               */
/*-----------------------------------------------------------------*/
static void unlinkpCodeFromBranch(pCode *pcl , pCode *pc)
{
  pBranch *b, *bprev;


  bprev = NULL;

  if(pcl->type == PC_OPCODE || pcl->type == PC_INLINE || pcl->type == PC_ASMDIR)
    b = PCI(pcl)->label;
  else {
    fprintf(stderr, "LINE %d. can't unlink from non opcode\n",__LINE__);
    exit(1);

  }

  //fprintf (stderr, "%s \n",__FUNCTION__);
  //pcl->print(stderr,pcl);
  //pc->print(stderr,pc);
  while(b) {
    if(b->pc == pc) {
      //fprintf (stderr, "found label\n");
      //pc->print(stderr, pc);

      /* Found a label */
      if(bprev) {
        bprev->next = b->next;  /* Not first pCode in chain */
//      Safe_free(b);
      } else {
        pc->destruct(pc);
        PCI(pcl)->label = b->next;   /* First pCode in chain */
//      Safe_free(b);
      }
      return;  /* A label can't occur more than once */
    }
    bprev = b;
    b = b->next;
  }

}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pBranch * pic16_pBranchAppend(pBranch *h, pBranch *n)
{
  pBranch *b;

  if(!h)
    return n;

  if(h == n)
    return n;

  b = h;
  while(b->next)
    b = b->next;

  b->next = n;

  return h;

}
/*-----------------------------------------------------------------*/
/* pBranchLink - given two pcodes, this function will link them    */
/*               together through their pBranches                  */
/*-----------------------------------------------------------------*/
static void pBranchLink(pCodeFunction *f, pCodeFunction *t)
{
  pBranch *b;

  // Declare a new branch object for the 'from' pCode.

  //_ALLOC(b,sizeof(pBranch));
  b = Safe_calloc(1,sizeof(pBranch));
  b->pc = PCODE(t);             // The link to the 'to' pCode.
  b->next = NULL;

  f->to = pic16_pBranchAppend(f->to,b);

  // Now do the same for the 'to' pCode.

  //_ALLOC(b,sizeof(pBranch));
  b = Safe_calloc(1,sizeof(pBranch));
  b->pc = PCODE(f);
  b->next = NULL;

  t->from = pic16_pBranchAppend(t->from,b);

}

#if 1
/*-----------------------------------------------------------------*/
/* pBranchFind - find the pBranch in a pBranch chain that contains */
/*               a pCode                                           */
/*-----------------------------------------------------------------*/
static pBranch *pBranchFind(pBranch *pb,pCode *pc)
{
  while(pb) {

    if(pb->pc == pc)
      return pb;

    pb = pb->next;
  }

  return NULL;
}

/*-----------------------------------------------------------------*/
/* pic16_pCodeUnlink - Unlink the given pCode from its pCode chain.      */
/*-----------------------------------------------------------------*/
void pic16_pCodeUnlink(pCode *pc)
{
  pBranch *pb1,*pb2;
  pCode *pc1;

  if (!pc) {
    return;
  }

  /* Remove the branches */

  pb1 = PCI(pc)->from;
  while(pb1) {
    pc1 = pb1->pc;    /* Get the pCode that branches to the
                       * one we're unlinking */

    /* search for the link back to this pCode (the one we're
     * unlinking) */
    if((pb2 = pBranchFind(PCI(pc1)->to,pc))) {
      pb2->pc = PCI(pc)->to->pc;  // make the replacement

      /* if the pCode we're unlinking contains multiple 'to'
       * branches (e.g. this a skip instruction) then we need
       * to copy these extra branches to the chain. */
      if(PCI(pc)->to->next)
        pic16_pBranchAppend(pb2, PCI(pc)->to->next);
    }

    pb1 = pb1->next;
  }

  pic16_unlinkpCode (pc);

}
#endif
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
#if 0
static void genericAnalyze(pCode *pc)
{
  switch(pc->type) {
  case PC_WILD:
  case PC_COMMENT:
    return;
  case PC_LABEL:
  case PC_FUNCTION:
  case PC_OPCODE:
    {
      // Go through the pCodes that are in pCode chain and link
      // them together through the pBranches. Note, the pCodes
      // are linked together as a contiguous stream like the
      // assembly source code lines. The linking here mimics this
      // except that comments are not linked in.
      //
      pCode *npc = pc->next;
      while(npc) {
        if(npc->type == PC_OPCODE || npc->type == PC_LABEL) {
          pBranchLink(pc,npc);
          return;
        } else
          npc = npc->next;
      }
      /* reached the end of the pcode chain without finding
       * an instruction we could link to. */
    }
    break;
  case PC_FLOW:
    fprintf(stderr,"analyze PC_FLOW\n");

    return;
  case PC_BAD:
    fprintf(stderr,,";A bad pCode is being used\n");

  }
}
#endif

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int compareLabel(pCode *pc, pCodeOpLabel *pcop_label)
{
  pBranch *pbr;

  if(pc->type == PC_LABEL) {
    if( ((pCodeLabel *)pc)->key ==  pcop_label->key)
      return TRUE;
  }
  if((pc->type == PC_OPCODE)
        || (pc->type == PC_ASMDIR)
        ) {
    pbr = PCI(pc)->label;
    while(pbr) {
      if(pbr->pc->type == PC_LABEL) {
        if( ((pCodeLabel *)(pbr->pc))->key ==  pcop_label->key)
          return TRUE;
      }
      pbr = pbr->next;
    }
  }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int checkLabel(pCode *pc)
{
  pBranch *pbr;

  if(pc && isPCI(pc)) {
    pbr = PCI(pc)->label;
    while(pbr) {
      if(isPCL(pbr->pc) && (PCL(pbr->pc)->key >= 0))
        return TRUE;

      pbr = pbr->next;
    }
  }

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* findLabelinpBlock - Search the pCode for a particular label     */
/*-----------------------------------------------------------------*/
static pCode * findLabelinpBlock(pBlock *pb,pCodeOpLabel *pcop_label)
{
  pCode  *pc;

  if(!pb)
    return NULL;

  for(pc = pb->pcHead; pc; pc = pc->next)
    if(compareLabel(pc,pcop_label))
      return pc;

  return NULL;
}
#if 0
/*-----------------------------------------------------------------*/
/* findLabel - Search the pCode for a particular label             */
/*-----------------------------------------------------------------*/
static pCode * findLabel(pCodeOpLabel *pcop_label)
{
  pBlock *pb;
  pCode  *pc;

  if(!the_pFile)
    return NULL;

  for(pb = the_pFile->pbHead; pb; pb = pb->next) {
    if( (pc = findLabelinpBlock(pb,pcop_label)) != NULL)
      return pc;
  }

  fprintf(stderr,"Couldn't find label %s", pcop_label->pcop.name);
  return NULL;
}
#endif
/*-----------------------------------------------------------------*/
/* pic16_findNextpCode - given a pCode, find the next of type 'pct'      */
/*                 in the linked list                              */
/*-----------------------------------------------------------------*/
pCode * pic16_findNextpCode(pCode *pc, PC_TYPE pct)
{

  while(pc) {
    if(pc->type == pct)
      return pc;

    pc = pc->next;
  }

  return NULL;
}

/*-----------------------------------------------------------------*/
/* findPrevpCode - given a pCode, find the previous of type 'pct'  */
/*                 in the linked list                              */
/*-----------------------------------------------------------------*/
static pCode * findPrevpCode(pCode *pc, PC_TYPE pct)
{

  while(pc) {
    if(pc->type == pct)
      return pc;

    pc = pc->prev;
  }

  return NULL;
}


//#define PCODE_DEBUG
/*-----------------------------------------------------------------*/
/* pic16_findNextInstruction - given a pCode, find the next instruction  */
/*                       in the linked list                        */
/*-----------------------------------------------------------------*/
pCode * pic16_findNextInstruction(pCode *pci)
{
  pCode *pc = pci;

  while(pc) {
    if((pc->type == PC_OPCODE)
        || (pc->type == PC_WILD)
        || (pc->type == PC_ASMDIR)
        )
      return pc;

#ifdef PCODE_DEBUG
    fprintf(stderr,"pic16_findNextInstruction:  ");
    printpCode(stderr, pc);
#endif
    pc = pc->next;
  }

  //fprintf(stderr,"Couldn't find instruction\n");
  return NULL;
}

/*-----------------------------------------------------------------*/
/* pic16_findPrevInstruction - given a pCode, find the next instruction  */
/*                       in the linked list                        */
/*-----------------------------------------------------------------*/
pCode * pic16_findPrevInstruction(pCode *pci)
{
  pCode *pc = pci;

  while(pc) {

    if((pc->type == PC_OPCODE)
        || (pc->type == PC_WILD)
        || (pc->type == PC_ASMDIR)
        )
      return pc;


#ifdef PCODE_DEBUG
    fprintf(stderr,"pic16_findPrevInstruction:  ");
    printpCode(stderr, pc);
#endif
    pc = pc->prev;
  }

  //fprintf(stderr,"Couldn't find instruction\n");
  return NULL;
}

#undef PCODE_DEBUG

#if 0
/*-----------------------------------------------------------------*/
/* findFunctionEnd - given a pCode find the end of the function    */
/*                   that contains it                              */
/*-----------------------------------------------------------------*/
static pCode * findFunctionEnd(pCode *pc)
{

  while(pc) {
    if(pc->type == PC_FUNCTION &&  !(PCF(pc)->fname))
      return pc;

    pc = pc->next;
  }

  fprintf(stderr,"Couldn't find function end\n");
  return NULL;
}
#endif
#if 0
/*-----------------------------------------------------------------*/
/* AnalyzeLabel - if the pCode is a label, then merge it with the  */
/*                instruction with which it is associated.         */
/*-----------------------------------------------------------------*/
static void AnalyzeLabel(pCode *pc)
{

  pic16_pCodeUnlink(pc);

}
#endif

#if 0
static void AnalyzeGOTO(pCode *pc)
{

  pBranchLink(pc,findLabel( (pCodeOpLabel *) (PCI(pc)->pcop) ));

}

static void AnalyzeSKIP(pCode *pc)
{

  pBranchLink(pc,pic16_findNextInstruction(pc->next));
  pBranchLink(pc,pic16_findNextInstruction(pc->next->next));

}

static void AnalyzeRETURN(pCode *pc)
{

  //  branch_link(pc,findFunctionEnd(pc->next));

}

#endif

/*-------------------------------------------------------------------*/
/* pic16_getRegFrompCodeOp - extract the register from a pCodeOp     */
/*                            if one is present. This is the common  */
/*                            part of pic16_getRegFromInstruction(2) */
/*-------------------------------------------------------------------*/

reg_info * pic16_getRegFrompCodeOp (pCodeOp *pcop) {
  if (!pcop) return NULL;

  switch(pcop->type) {
  case PO_PRODL:
  case PO_PRODH:
  case PO_INDF0:
  case PO_FSR0:
  case PO_W:
  case PO_WREG:
  case PO_STATUS:
  case PO_INTCON:
  case PO_PCL:
  case PO_PCLATH:
  case PO_PCLATU:
  case PO_BSR:
    return PCOR(pcop)->r;

  case PO_SFR_REGISTER:
    //fprintf (stderr, "%s - SFR\n", __FUNCTION__);
    return PCOR(pcop)->r;

  case PO_BIT:
  case PO_GPR_TEMP:
//      fprintf(stderr, "pic16_getRegFromInstruction - bit or temp\n");
    return PCOR(pcop)->r;

  case PO_IMMEDIATE:
//    return pic16_dirregWithName(PCOI(pcop)->r->name);

    if(PCOI(pcop)->r)
      return (PCOI(pcop)->r);
    else
      return NULL;

  case PO_GPR_BIT:
    return PCOR(pcop)->r;

  case PO_GPR_REGISTER:
  case PO_DIR:
//      fprintf(stderr, "pic16_getRegFromInstruction - dir\n");
    return PCOR(pcop)->r;

  case PO_LITERAL:
    //fprintf(stderr, "pic16_getRegFromInstruction - literal\n");
    break;

  case PO_REL_ADDR:
  case PO_LABEL:
    //fprintf (stderr, "%s - label or address: %d (%s)\n", __FUNCTION__, pcop->type, dumpPicOptype(pcop->type));
    break;

  case PO_CRY:
  case PO_STR:
    /* this should never turn up */
    //fprintf (stderr, "%s - unused pCodeOp->type: %d (%s)\n", __FUNCTION__, pcop->type, dumpPicOptype(pcop->type));
    break;

  case PO_WILD:
    break;

  case PO_TWO_OPS:
    return pic16_getRegFrompCodeOp( PCOP2(pcop)->pcopL );
    break;

  default:
        fprintf(stderr, "pic16_getRegFrompCodeOp - unknown reg type %d (%s)\n",pcop->type, dumpPicOptype (pcop->type));
//      assert( 0 );
        break;
  }

  return NULL;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
reg_info * pic16_getRegFromInstruction(pCode *pc)
{
  if(!pc                   ||
     !isPCI(pc)            ||
     !PCI(pc)->pcop        ||
     PCI(pc)->num_ops == 0 ||
     (PCI(pc)->num_ops == 1 && PCI(pc)->isFastCall))
    return NULL;

#if 0
  fprintf(stderr, "pic16_getRegFromInstruction - reg type %s (%d)\n",
        dumpPicOptype( PCI(pc)->pcop->type), PCI(pc)->pcop->type);
#endif

  return( pic16_getRegFrompCodeOp (PCI(pc)->pcop) );
}

/*-------------------------------------------------------------------------------*/
/* pic16_getRegFromInstruction2 - variant to support two memory operand commands */
/*-------------------------------------------------------------------------------*/
reg_info * pic16_getRegFromInstruction2(pCode *pc)
{

  if(!pc                   ||
     !isPCI(pc)            ||
     !PCI(pc)->pcop        ||
     PCI(pc)->num_ops == 0 ||
     (PCI(pc)->num_ops == 1))           // accept only 2 operand commands
    return NULL;

  if (PCI(pc)->pcop->type != PO_TWO_OPS)
    return NULL;

#if 0
  fprintf(stderr, "pic16_getRegFromInstruction2 - reg type %s (%d)\n",
        dumpPicOptype( PCI(pc)->pcop->type), PCI(pc)->pcop->type);
#endif

  return pic16_getRegFrompCodeOp (PCOP2(PCI(pc)->pcop)->pcopR);
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

static void AnalyzepBlock(pBlock *pb)
{
  pCode *pc;

  if(!pb)
    return;

  /* Find all of the registers used in this pBlock
   * by looking at each instruction and examining it's
   * operands
   */
  for(pc = pb->pcHead; pc; pc = pc->next) {

    /* Is this an instruction with operands? */
    if(pc->type == PC_OPCODE && PCI(pc)->pcop) {

      if(PCI(pc)->pcop->type == PO_GPR_TEMP) {

        /* Loop through all of the registers declared so far in
           this block and see if we find this one there */

        reg_info *r = setFirstItem(pb->tregisters);

        while(r) {
          if(r->rIdx == PCOR(PCI(pc)->pcop)->r->rIdx) {
            PCOR(PCI(pc)->pcop)->r = r;
            break;
          }
          r = setNextItem(pb->tregisters);
        }

        if(!r) {
          /* register wasn't found */
          //r = Safe_calloc(1, sizeof(regs));
          //memcpy(r,PCOR(PCI(pc)->pcop)->r, sizeof(regs));
          //addSet(&pb->tregisters, r);
          addSet(&pb->tregisters, PCOR(PCI(pc)->pcop)->r);
          //PCOR(PCI(pc)->pcop)->r = r;
          //fprintf(stderr,"added register to pblock: reg %d\n",r->rIdx);
        }/* else
          fprintf(stderr,"found register in pblock: reg %d\n",r->rIdx);
         */
      }
      if(PCI(pc)->pcop->type == PO_GPR_REGISTER) {
        if(PCOR(PCI(pc)->pcop)->r) {
          pic16_allocWithIdx(PCOR(PCI(pc)->pcop)->r->rIdx);                     /* FIXME! - VR */
          DFPRINTF((stderr,"found register in pblock: reg 0x%x\n",PCOR(PCI(pc)->pcop)->r->rIdx));
        } else {
          if(PCI(pc)->pcop->name)
            fprintf(stderr,"ERROR: %s is a NULL register\n",PCI(pc)->pcop->name );
          else
            fprintf(stderr,"ERROR: NULL register\n");
        }
      }
    }


  }
}

/*-----------------------------------------------------------------*/
/* */
/*-----------------------------------------------------------------*/
#define PCI_HAS_LABEL(x) ((x) && (PCI(x)->label != NULL))

static void InsertpFlow(pCode *pc, pCode **pflow)
{
  if(*pflow)
    PCFL(*pflow)->end = pc;

  if(!pc || !pc->next)
    return;

  *pflow = pic16_newpCodeFlow();
  pic16_pCodeInsertAfter(pc, *pflow);
}

/*-----------------------------------------------------------------*/
/* pic16_BuildFlow(pBlock *pb) - examine the code in a pBlock and build  */
/*                         the flow blocks.                        */
/*
 * pic16_BuildFlow inserts pCodeFlow objects into the pCode chain at each
 * point the instruction flow changes.
 */
/*-----------------------------------------------------------------*/
void pic16_BuildFlow(pBlock *pb)
{
  pCode *pc;
  pCode *last_pci=NULL;
  pCode *pflow=NULL;
  int seq = 0;

  if(!pb)
    return;

  //fprintf (stderr,"build flow start seq %d  ",GpcFlowSeq);
  /* Insert a pCodeFlow object at the beginning of a pBlock */

  InsertpFlow(pb->pcHead, &pflow);

  //pflow = pic16_newpCodeFlow();    /* Create a new Flow object */
  //pflow->next = pb->pcHead;  /* Make the current head the next object */
  //pb->pcHead->prev = pflow;  /* let the current head point back to the flow object */
  //pb->pcHead = pflow;        /* Make the Flow object the head */
  //pflow->pb = pb;

  for( pc = pic16_findNextInstruction(pb->pcHead);
       pc != NULL;
       pc=pic16_findNextInstruction(pc)) {

    pc->seq = seq++;
    PCI(pc)->pcflow = PCFL(pflow);

    //fprintf(stderr," build: ");
    //pflow->print(stderr,pflow);

    if (checkLabel(pc)) {

      /* This instruction marks the beginning of a
       * new flow segment */

      pc->seq = 0;
      seq = 1;

      /* If the previous pCode is not a flow object, then
       * insert a new flow object. (This check prevents
       * two consecutive flow objects from being insert in
       * the case where a skip instruction preceeds an
       * instruction containing a label.) */

      if(last_pci && (PCI(last_pci)->pcflow == PCFL(pflow)))
        InsertpFlow(pic16_findPrevInstruction(pc->prev), &pflow);

      PCI(pc)->pcflow = PCFL(pflow);

    }

    if( PCI(pc)->isSkip) {

      /* The two instructions immediately following this one
       * mark the beginning of a new flow segment */

      while(pc && PCI(pc)->isSkip) {

        PCI(pc)->pcflow = PCFL(pflow);
        pc->seq = seq-1;
        seq = 1;

        InsertpFlow(pc, &pflow);
        pc=pic16_findNextInstruction(pc->next);
      }

      seq = 0;

      if(!pc)
        break;

      PCI(pc)->pcflow = PCFL(pflow);
      pc->seq = 0;
      InsertpFlow(pc, &pflow);

    } else if ( PCI(pc)->isBranch && !checkLabel(pic16_findNextInstruction(pc->next)))  {

      InsertpFlow(pc, &pflow);
      seq = 0;

    }
    last_pci = pc;
    pc = pc->next;
  }

  //fprintf (stderr,",end seq %d",GpcFlowSeq);
  if(pflow)
    PCFL(pflow)->end = pb->pcTail;
}

/*-------------------------------------------------------------------*/
/* unBuildFlow(pBlock *pb) - examine the code in a pBlock and build  */
/*                           the flow blocks.                        */
/*
 * unBuildFlow removes pCodeFlow objects from a pCode chain
 */
/*-----------------------------------------------------------------*/
static void unBuildFlow(pBlock *pb)
{
  pCode *pc,*pcnext;

  if(!pb)
    return;

  pc = pb->pcHead;

  while(pc) {
    pcnext = pc->next;

    if(isPCI(pc)) {

      pc->seq = 0;
      if(PCI(pc)->pcflow) {
        //Safe_free(PCI(pc)->pcflow);
        PCI(pc)->pcflow = NULL;
      }

    } else if(isPCFL(pc) )
      pc->destruct(pc);

    pc = pcnext;
  }


}
#if 0
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void dumpCond(int cond)
{

  static char *pcc_str[] = {
    //"PCC_NONE",
    "PCC_REGISTER",
    "PCC_C",
    "PCC_Z",
    "PCC_DC",
    "PCC_OV",
    "PCC_N",
    "PCC_W",
    "PCC_EXAMINE_PCOP",
    "PCC_LITERAL",
    "PCC_REL_ADDR"
  };

  int ncond = sizeof(pcc_str) / sizeof(char *);
  int i,j;

  fprintf(stderr, "0x%04X\n",cond);

  for(i=0,j=1; i<ncond; i++, j<<=1)
    if(cond & j)
      fprintf(stderr, "  %s\n",pcc_str[i]);

}
#endif

#if 0
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void FlowStats(pCodeFlow *pcflow)
{

  pCode *pc;

  if(!isPCFL(pcflow))
    return;

  fprintf(stderr, " FlowStats - flow block (seq=%d)\n", pcflow->pc.seq);

  pc = pic16_findNextpCode(PCODE(pcflow), PC_OPCODE);

  if(!pc) {
    fprintf(stderr, " FlowStats - empty flow (seq=%d)\n", pcflow->pc.seq);
    return;
  }


  fprintf(stderr, "  FlowStats inCond: ");
  dumpCond(pcflow->inCond);
  fprintf(stderr, "  FlowStats outCond: ");
  dumpCond(pcflow->outCond);

}
#endif
/*-----------------------------------------------------------------*
 * int isBankInstruction(pCode *pc) - examine the pCode *pc to determine
 *    if it affects the banking bits.
 *
 * return: -1 == Banking bits are unaffected by this pCode.
 *
 * return: > 0 == Banking bits are affected.
 *
 *  If the banking bits are affected, then the returned value describes
 * which bits are affected and how they're affected. The lower half
 * of the integer maps to the bits that are affected, the upper half
 * to whether they're set or cleared.
 *
 *-----------------------------------------------------------------*/

static int isBankInstruction(pCode *pc)
{
  reg_info *reg;

  if(!isPCI(pc))
    return 0;

  if( PCI(pc)->op == POC_MOVLB ||
      (( (reg = pic16_getRegFromInstruction(pc)) != NULL) && isBSR_REG(reg))) {
  }

  return 1;
}


/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void FillFlow(pCodeFlow *pcflow)
{

  pCode *pc;

  if(!isPCFL(pcflow))
    return;

  //  fprintf(stderr, " FillFlow - flow block (seq=%d)\n", pcflow->pc.seq);

  pc = pic16_findNextpCode(PCODE(pcflow), PC_OPCODE);

  if(!pc) {
    //fprintf(stderr, " FillFlow - empty flow (seq=%d)\n", pcflow->pc.seq);
    return;
  }

  do {
    isBankInstruction(pc);
    pc = pc->next;
  } while (pc && (pc != pcflow->end) && !isPCFL(pc));

/*
  if(!pc ) {
    fprintf(stderr, "  FillFlow - Bad end of flow\n");
  } else {
    fprintf(stderr, "  FillFlow - Ending flow with\n  ");
    pc->print(stderr,pc);
  }

  fprintf(stderr, "  FillFlow inCond: ");
  dumpCond(pcflow->inCond);
  fprintf(stderr, "  FillFlow outCond: ");
  dumpCond(pcflow->outCond);
*/
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void LinkFlow_pCode(pCodeInstruction *from, pCodeInstruction *to)
{
  pCodeFlowLink *fromLink, *toLink;

  if(!from || !to || !to->pcflow || !from->pcflow)
    return;

  fromLink = pic16_newpCodeFlowLink(from->pcflow);
  toLink   = pic16_newpCodeFlowLink(to->pcflow);

  addSetIfnotP(&(from->pcflow->to), toLink);   //to->pcflow);
  addSetIfnotP(&(to->pcflow->from), fromLink); //from->pcflow);

}

pCode *pic16_getJumptabpCode (pCode *pc) {
  pCode *pcinf;

  //fprintf (stderr, "%s - start for %p in %p", __FUNCTION__, pc, isPCI(pc) ? PCI(pc)->pcflow : NULL);
  //pc->print (stderr, pc);
  pcinf = pc;
  while (pcinf) {
    if (isPCI(pcinf) && PCI(pcinf)->op != POC_GOTO) return NULL;
    if (pcinf->type == PC_INFO && PCINF(pcinf)->type == INF_OPTIMIZATION) {
      switch (PCOO(PCINF(pcinf)->oper1)->type) {
      case OPT_JUMPTABLE_BEGIN:
        /* leading begin of jump table -- in one */
        pcinf = pic16_findPrevInstruction (pcinf);
        return pcinf;
        break;

      case OPT_JUMPTABLE_END:
        /* leading end of jumptable -- not in one */
        return NULL;
        break;

      default:
        /* ignore all other PCInfos */
        break;
      }
    }
    pcinf = pcinf->prev;
  }

  /* no PCInfo found -- not in a jumptable */
  return NULL;
}

/*-----------------------------------------------------------------*
 * void LinkFlow(pBlock *pb)
 *
 * In pic16_BuildFlow, the PIC code has been partitioned into contiguous
 * non-branching segments. In LinkFlow, we determine the execution
 * order of these segments. For example, if one of the segments ends
 * with a skip, then we know that there are two possible flow segments
 * to which control may be passed.
 *-----------------------------------------------------------------*/
static void LinkFlow(pBlock *pb)
{
  pCode *pc=NULL;
  pCode *pcflow;
  pCode *pct;
  pCode *jumptab_pre = NULL;

  //fprintf(stderr,"linkflow \n");

  for( pcflow = pic16_findNextpCode(pb->pcHead, PC_FLOW);
       pcflow != NULL;
       pcflow = pic16_findNextpCode(pcflow->next, PC_FLOW) ) {

    if(!isPCFL(pcflow))
      fprintf(stderr, "LinkFlow - pcflow is not a flow object ");

    //fprintf(stderr," link: ");
    //pcflow->print(stderr,pcflow);

    //FillFlow(PCFL(pcflow));

    pc = PCFL(pcflow)->end;

    //fprintf(stderr, "LinkFlow - flow block (seq=%d) ", pcflow->seq);
    if(isPCI_SKIP(pc)) {
//      fprintf(stderr, "ends with skip\n");
//      pc->print(stderr,pc);

      pct=pic16_findNextInstruction(pc->next);
      LinkFlow_pCode(PCI(pc),PCI(pct));
      pct=pic16_findNextInstruction(pct->next);
      LinkFlow_pCode(PCI(pc),PCI(pct));
      continue;
    }

    if(isPCI_BRANCH(pc)) {
      pCodeOpLabel *pcol = PCOLAB(PCI(pc)->pcop);

      /* handle GOTOs in jumptables */
      if ((jumptab_pre = pic16_getJumptabpCode (pc)) != NULL) {
        /* link to previous flow */
        //fprintf (stderr, "linked jumptable GOTO to predecessor %p\n", PCI(jumptab_pre)->pcflow);
        LinkFlow_pCode (PCI(jumptab_pre), PCI(pc));
      }

      switch (PCI(pc)->op) {
      case POC_GOTO:
      case POC_BRA:
      case POC_RETURN:
      case POC_RETLW:
      case POC_RETFIE:
              /* unconditional branches -- do not link to next instruction */
              //fprintf (stderr, "%s: flow ended by unconditional branch\n", __FUNCTION__);
              break;

      case POC_CALL:
      case POC_RCALL:
              /* unconditional calls -- link to next instruction */
              //fprintf (stderr, "%s: flow ended by CALL\n", __FUNCTION__);
              LinkFlow_pCode(PCI(pc),PCI(pic16_findNextInstruction(pc->next)));
              break;

      case POC_BC:
      case POC_BN:
      case POC_BNC:
      case POC_BNN:
      case POC_BNOV:
      case POC_BNZ:
      case POC_BOV:
      case POC_BZ:
              /* conditional branches -- also link to next instruction */
              //fprintf (stderr, "%s: flow ended by conditional branch\n", __FUNCTION__);
              LinkFlow_pCode(PCI(pc),PCI(pic16_findNextInstruction(pc->next)));
              break;

      default:
              fprintf (stderr, "%s: unhandled op %u (%s)\n", __FUNCTION__, PCI(pc)->op , PCI(pc)->mnemonic);
              assert (0 && "unhandled branching instruction");
              break;
      }

      //fprintf(stderr, "ends with branch\n  ");
      //pc->print(stderr,pc);

      if(!(pcol && isPCOLAB(pcol))) {
        if((PCI(pc)->op != POC_RETLW)
                && (PCI(pc)->op != POC_RETURN) && (PCI(pc)->op != POC_CALL) && (PCI(pc)->op != POC_RCALL) && (PCI(pc)->op != POC_RETFIE) ) {

                /* continue if label is '$' which assembler knows how to parse */
                if(((PCI(pc)->pcop->type == PO_STR) && !strcmp(PCI(pc)->pcop->name, "$")))continue;

                if(pic16_pcode_verbose) {
                        pc->print(stderr,pc);
                        fprintf(stderr, "ERROR: %s, branch instruction doesn't have label\n",__FUNCTION__);
                }
        }
        continue;
      }

      if( (pct = findLabelinpBlock(pb,pcol)) != NULL)
        LinkFlow_pCode(PCI(pc),PCI(pic16_findNextInstruction(pct)));
      else
        fprintf(stderr, "ERROR: %s, couldn't find label. key=%d,lab=%s\n",
                __FUNCTION__,pcol->key,((PCOP(pcol)->name)?PCOP(pcol)->name:"-"));

//      fprintf(stderr,"pic16_newpCodeOpLabel: key=%d, name=%s\n",pcol->key,(PCOP(pcol)->name)?(PCOP(pcol)->name):"<unknown>");

      continue;
    }

    if(isPCI(pc)) {
      //fprintf(stderr, "ends with non-branching instruction:\n");
      //pc->print(stderr,pc);

      LinkFlow_pCode(PCI(pc),PCI(pic16_findNextInstruction(pc->next)));

      continue;
    }

    if(pc) {
      //fprintf(stderr, "ends with unknown\n");
      //pc->print(stderr,pc);
      continue;
    }

    //fprintf(stderr, "ends with nothing: ERROR\n");

  }
}
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
int pic16_isPCinFlow(pCode *pc, pCode *pcflow)
{

  if(!pc || !pcflow)
    return 0;

  if((!isPCI(pc) && !isPCAD(pc)) || !PCI(pc)->pcflow || !isPCFL(pcflow) )
    return 0;

  if( PCI(pc)->pcflow->pc.seq == pcflow->seq)
    return 1;

  return 0;
}





/*-----------------------------------------------------------------*/
/* insertBankSwitch - inserts a bank switch statement in the       */
/*                    assembly listing                             */
/*                                                                 */
/* position == 0: insert before                                    */
/* position == 1: insert after pc                                  */
/* position == 2: like 0 but previous was a skip instruction       */
/*-----------------------------------------------------------------*/
pCodeOp *pic16_popGetLabel(unsigned int key);
extern int pic16_labelOffset;

static void insertBankSwitch(unsigned char position, pCode *pc)
{
  pCode *new_pc;

        if(!pc)
                return;

        /* emit BANKSEL [symbol] */


        new_pc = pic16_newpCodeAsmDir("BANKSEL", "%s", pic16_get_op_from_instruction(PCI(pc)));

//      position = 0;           // position is always before (sanity check!)

#if 0
        fprintf(stderr, "%s:%d: inserting bank switch (pos: %d)\n", __FUNCTION__, __LINE__, position);
        pc->print(stderr, pc);
#endif

        switch(position) {
                case 1: {
                        /* insert the bank switch after this pc instruction */
                        pCode *pcnext = pic16_findNextInstruction(pc);

                                pic16_pCodeInsertAfter(pc, new_pc);
                                if(pcnext)pc = pcnext;
                }; break;

                case 0:
                        /* insert the bank switch BEFORE this pc instruction */
                        pic16_pCodeInsertAfter(pc->prev, new_pc);
                        break;

                case 2: {
                          symbol *tlbl;
                          pCode *pcnext, *pcprev, *npci, *ppc;
                          PIC_OPCODE ipci;
                          int ofs1=0, ofs2=0;

                        /* just like 0, but previous was a skip instruction,
                         * so some care should be taken */

                                pic16_labelOffset += 10000;
                                tlbl = newiTempLabel(NULL);

                                /* invert skip instruction */
                                pcprev = pic16_findPrevInstruction(pc->prev);
                                ipci = PCI(pcprev)->inverted_op;
                                npci = pic16_newpCode(ipci, PCI(pcprev)->pcop);

//                              fprintf(stderr, "%s:%d old OP: %d\tnew OP: %d\n", __FILE__, __LINE__, PCI(pcprev)->op, ipci);

                                /* copy info from old pCode */
                                ofs1 = ofs2 = sizeof( pCode ) + sizeof(PIC_OPCODE);
                                ofs1 += strlen( PCI(pcprev)->mnemonic) + 1;
                                ofs2 += strlen( PCI(npci)->mnemonic) + 1;
                                memcpy(&PCI(npci)->from, &PCI(pcprev)->from, (char *)(&(PCI(npci)->pci_magic)) - (char *)(&(PCI(npci)->from)));
                                PCI(npci)->op = PCI(pcprev)->inverted_op;

                                /* unlink old pCode */
                                ppc = pcprev->prev;
                                ppc->next = pcprev->next;
                                pcprev->next->prev = ppc;
                                pic16_pCodeInsertAfter(ppc, npci);

                                /* extra instructions to handle invertion */
                                pcnext = pic16_newpCode(POC_BRA, pic16_popGetLabel(tlbl->key));
                                pic16_pCodeInsertAfter(npci, pcnext);
                                pic16_pCodeInsertAfter(pc->prev, new_pc);

                                pcnext = pic16_newpCodeLabel(NULL,tlbl->key+100+pic16_labelOffset);
                                pic16_pCodeInsertAfter(pc, pcnext);
                        }; break;
        }


        /* Move the label, if there is one */
        if(PCI(pc)->label) {
//              fprintf(stderr, "%s:%d: moving label due to bank switch directive src= 0x%p dst= 0x%p\n",
//                      __FILE__, __LINE__, pc, new_pc);
                PCAD(new_pc)->pci.label = PCI(pc)->label;
                PCI(pc)->label = NULL;
        }
}


#if 0
/*-----------------------------------------------------------------*/
/*int compareBankFlow - compare the banking requirements between   */
/*  flow objects. */
/*-----------------------------------------------------------------*/
static int compareBankFlow(pCodeFlow *pcflow, pCodeFlowLink *pcflowLink, int toORfrom)
{

  if(!pcflow || !pcflowLink || !pcflowLink->pcflow)
    return 0;

  if(!isPCFL(pcflow) || !isPCFL(pcflowLink->pcflow))
    return 0;

  if(pcflow->firstBank == -1)
    return 0;


  if(pcflowLink->pcflow->firstBank == -1) {
    pCodeFlowLink *pctl = setFirstItem( toORfrom ?
                                        pcflowLink->pcflow->to :
                                        pcflowLink->pcflow->from);
    return compareBankFlow(pcflow, pctl, toORfrom);
  }

  if(toORfrom) {
    if(pcflow->lastBank == pcflowLink->pcflow->firstBank)
      return 0;

    pcflowLink->bank_conflict++;
    pcflowLink->pcflow->FromConflicts++;
    pcflow->ToConflicts++;
  } else {

    if(pcflow->firstBank == pcflowLink->pcflow->lastBank)
      return 0;

    pcflowLink->bank_conflict++;
    pcflowLink->pcflow->ToConflicts++;
    pcflow->FromConflicts++;

  }
  /*
  fprintf(stderr,"compare flow found conflict: seq %d from conflicts %d, to conflicts %d\n",
          pcflowLink->pcflow->pc.seq,
          pcflowLink->pcflow->FromConflicts,
          pcflowLink->pcflow->ToConflicts);
  */
  return 1;

}
#endif

#if 0
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void DumpFlow(pBlock *pb)
{
  pCode *pc=NULL;
  pCode *pcflow;
  pCodeFlowLink *pcfl;


  fprintf(stderr,"Dump flow \n");
  pb->pcHead->print(stderr, pb->pcHead);

  pcflow = pic16_findNextpCode(pb->pcHead, PC_FLOW);
  pcflow->print(stderr,pcflow);

  for( pcflow = pic16_findNextpCode(pb->pcHead, PC_FLOW);
       pcflow != NULL;
       pcflow = pic16_findNextpCode(pcflow->next, PC_FLOW) ) {

    if(!isPCFL(pcflow)) {
      fprintf(stderr, "DumpFlow - pcflow is not a flow object ");
      continue;
    }
    fprintf(stderr,"dumping: ");
    pcflow->print(stderr,pcflow);
    FlowStats(PCFL(pcflow));

    for(pcfl = setFirstItem(PCFL(pcflow)->to); pcfl; pcfl=setNextItem(PCFL(pcflow)->to)) {

      pc = PCODE(pcfl->pcflow);

      fprintf(stderr, "    from seq %d:\n",pc->seq);
      if(!isPCFL(pc)) {
        fprintf(stderr,"oops dumpflow - from is not a pcflow\n");
        pc->print(stderr,pc);
      }

    }

    for(pcfl = setFirstItem(PCFL(pcflow)->to); pcfl; pcfl=setNextItem(PCFL(pcflow)->to)) {

      pc = PCODE(pcfl->pcflow);

      fprintf(stderr, "    to seq %d:\n",pc->seq);
      if(!isPCFL(pc)) {
        fprintf(stderr,"oops dumpflow - to is not a pcflow\n");
        pc->print(stderr,pc);
      }

    }

  }

}
#endif
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int OptimizepBlock(pBlock *pb)
{
  pCode *pc, *pcprev;
  int matches =0;

  if(!pb || !peepOptimizing)
    return 0;

  DFPRINTF((stderr," Optimizing pBlock: %c\n",getpBlock_dbName(pb)));
/*
  for(pc = pb->pcHead; pc; pc = pc->next)
    matches += pic16_pCodePeepMatchRule(pc);
*/

  pc = pic16_findNextInstruction(pb->pcHead);
  if(!pc)
    return 0;

  pcprev = pc->prev;
  do {


    if(pic16_pCodePeepMatchRule(pc)) {

      matches++;

      if(pcprev)
        pc = pic16_findNextInstruction(pcprev->next);
      else
        pc = pic16_findNextInstruction(pb->pcHead);
    } else
      pc = pic16_findNextInstruction(pc->next);
  } while(pc);

  if(matches)
    DFPRINTF((stderr," Optimizing pBlock: %c - matches=%d\n",getpBlock_dbName(pb),matches));
  return matches;

}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static pCode * findInstructionUsingLabel(pCodeLabel *pcl, pCode *pcs)
{
  pCode *pc;

  for(pc = pcs; pc; pc = pc->next) {

    if(((pc->type == PC_OPCODE) || (pc->type == PC_INLINE) || (pc->type == PC_ASMDIR)) &&
       (PCI(pc)->pcop) &&
       (PCI(pc)->pcop->type == PO_LABEL) &&
       (PCOLAB(PCI(pc)->pcop)->key == pcl->key))
      return pc;
  }


  return NULL;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void exchangeLabels(pCodeLabel *pcl, pCode *pc)
{

  char *s=NULL;

  if(isPCI(pc) &&
     (PCI(pc)->pcop) &&
     (PCI(pc)->pcop->type == PO_LABEL)) {

    pCodeOpLabel *pcol = PCOLAB(PCI(pc)->pcop);

//      fprintf(stderr,"changing label key from %d to %d\n",pcol->key, pcl->key);
//    if(pcol->pcop.name)
//      Safe_free(pcol->pcop.name);

    /* If the key is negative, then we (probably) have a label to
     * a function and the name is already defined */

    if(pcl->key>0)
      sprintf(s=buffer,"_%05d_DS_",pcl->key);
    else
      s = pcl->label;

    //sprintf(buffer,"_%05d_DS_",pcl->key);
    if(!s) {
      fprintf(stderr, "ERROR %s:%d function label is null\n",__FUNCTION__,__LINE__);
    }
    pcol->pcop.name = Safe_strdup(s);
    pcol->key = pcl->key;
    //pc->print(stderr,pc);

  }


}

/*-----------------------------------------------------------------*/
/* pBlockRemoveUnusedLabels - remove the pCode labels from the     */
/*                            pCode chain if they're not used.     */
/*-----------------------------------------------------------------*/
static void pBlockRemoveUnusedLabels(pBlock *pb)
{
  pCode *pc; pCodeLabel *pcl;

  if(!pb || !pb->pcHead)
    return;

  for(pc = pb->pcHead; (pc=pic16_findNextInstruction(pc->next)) != NULL; ) {

    pBranch *pbr = PCI(pc)->label;
    if(pbr && pbr->next) {
      pCode *pcd = pb->pcHead;

//      fprintf(stderr, "multiple labels\n");
//      pc->print(stderr,pc);

      pbr = pbr->next;
      while(pbr) {

        while ( (pcd = findInstructionUsingLabel(PCL(PCI(pc)->label->pc), pcd)) != NULL) {
          //fprintf(stderr,"Used by:\n");
          //pcd->print(stderr,pcd);

          exchangeLabels(PCL(pbr->pc),pcd);

          pcd = pcd->next;
        }
        pbr = pbr->next;
      }
    }
  }

  for(pc = pb->pcHead; pc; pc = pc->next) {

    if(isPCL(pc)) // pc->type == PC_LABEL)
      pcl = PCL(pc);
    else if (isPCI(pc) && PCI(pc)->label) //((pc->type == PC_OPCODE) && PCI(pc)->label)
      pcl = PCL(PCI(pc)->label->pc);
    else continue;

//      fprintf(stderr," found  A LABEL !!! key = %d, %s\n", pcl->key,pcl->label);

    /* This pCode is a label, so search the pBlock to see if anyone
     * refers to it */

    if( (pcl->key>0) && (!findInstructionUsingLabel(pcl, pb->pcHead))
        && (!pcl->force)) {
    //if( !findInstructionUsingLabel(pcl, pb->pcHead)) {
      /* Couldn't find an instruction that refers to this label
       * So, unlink the pCode label from it's pCode chain
       * and destroy the label */
//      fprintf(stderr," removed  A LABEL !!! key = %d, %s\n", pcl->key,pcl->label);

      DFPRINTF((stderr," !!! REMOVED A LABEL !!! key = %d, %s\n", pcl->key,pcl->label));
      if(pc->type == PC_LABEL) {
        pic16_unlinkpCode(pc);
        pCodeLabelDestruct(pc);
      } else {
        unlinkpCodeFromBranch(pc, PCODE(pcl));
        /*if(pc->label->next == NULL && pc->label->pc == NULL) {
          Safe_free(pc->label);
        }*/
      }

    }
  }

}


/*-----------------------------------------------------------------*/
/* pic16_pBlockMergeLabels - remove the pCode labels from the pCode      */
/*                     chain and put them into pBranches that are  */
/*                     associated with the appropriate pCode       */
/*                     instructions.                               */
/*-----------------------------------------------------------------*/
void pic16_pBlockMergeLabels(pBlock *pb)
{
  pBranch *pbr;
  pCode *pc, *pcnext=NULL;

  if(!pb)
    return;

  /* First, Try to remove any unused labels */
  //pBlockRemoveUnusedLabels(pb);

  /* Now loop through the pBlock and merge the labels with the opcodes */

  pc = pb->pcHead;
  //  for(pc = pb->pcHead; pc; pc = pc->next) {

  while(pc) {
    pCode *pcn = pc->next;

    if(pc->type == PC_LABEL) {

//      fprintf(stderr," checking merging label %s\n",PCL(pc)->label);
//      fprintf(stderr,"Checking label key = %d\n",PCL(pc)->key);

      if((pcnext = pic16_findNextInstruction(pc) )) {

//              pcnext->print(stderr, pcnext);

        // Unlink the pCode label from it's pCode chain
        pic16_unlinkpCode(pc);

//      fprintf(stderr,"Merged label key = %d\n",PCL(pc)->key);
        // And link it into the instruction's pBranch labels. (Note, since
        // it's possible to have multiple labels associated with one instruction
        // we must provide a means to accomodate the additional labels. Thus
        // the labels are placed into the singly-linked list "label" as
        // opposed to being a single member of the pCodeInstruction.)

        //_ALLOC(pbr,sizeof(pBranch));
#if 1
        pbr = Safe_calloc(1,sizeof(pBranch));
        pbr->pc = pc;
        pbr->next = NULL;

        PCI(pcnext)->label = pic16_pBranchAppend(PCI(pcnext)->label,pbr);
#endif
      } else {
        if(pic16_pcode_verbose)
        fprintf(stderr, "WARNING: couldn't associate label %s with an instruction\n",PCL(pc)->label);
      }
    } else if(pc->type == PC_CSOURCE) {

      /* merge the source line symbolic info into the next instruction */
      if((pcnext = pic16_findNextInstruction(pc) )) {

        // Unlink the pCode label from it's pCode chain
        pic16_unlinkpCode(pc);
        PCI(pcnext)->cline = PCCS(pc);
        //fprintf(stderr, "merging CSRC\n");
        //genericPrint(stderr,pcnext);
      }

    }
    pc = pcn;
  }
  pBlockRemoveUnusedLabels(pb);

}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int OptimizepCode(char dbName)
{
#define MAX_PASSES 4

  int matches = 0;
  int passes = 0;
  pBlock *pb;

  if(!the_pFile)
    return 0;

  DFPRINTF((stderr," Optimizing pCode\n"));

  do {
    matches = 0;
    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
      if('*' == dbName || getpBlock_dbName(pb) == dbName)
        matches += OptimizepBlock(pb);
    }
  }
  while(matches && ++passes < MAX_PASSES);

  return matches;
}



const char *pic16_pCodeOpType(pCodeOp *pcop);
const char *pic16_pCodeOpSubType(pCodeOp *pcop);


/*-----------------------------------------------------------------*/
/* pic16_popCopyGPR2Bit - copy a pcode operator                          */
/*-----------------------------------------------------------------*/

pCodeOp *pic16_popCopyGPR2Bit(pCodeOp *pc, int bitval)
{
  pCodeOp *pcop=NULL;

//  fprintf(stderr, "%s:%d pc type: %s\tname: %s\n", __FILE__, __LINE__, pic16_pCodeOpType(pc), pc->name);

  if(pc->name) {
        pcop = pic16_newpCodeOpBit(pc->name, bitval, 0, pc->type);
  } else {
    if(PCOR(pc)->r)pcop = pic16_newpCodeOpBit(PCOR(pc)->r->name, bitval, 0, pc->type);
  }

  assert(pcop != NULL);

  if( !( (pcop->type == PO_LABEL) ||
         (pcop->type == PO_LITERAL) ||
         (pcop->type == PO_STR) ))
    PCOR(pcop)->r = PCOR(pc)->r;  /* This is dangerous... */
    PCOR(pcop)->r->wasUsed = 1;
    PCOR(pcop)->instance = PCOR(pc)->instance;

  return pcop;
}


/*----------------------------------------------------------------------*
 * pic16_areRegsSame - check to see if the names of two registers match *
 *----------------------------------------------------------------------*/
int pic16_areRegsSame(reg_info *r1, reg_info *r2)
{
        if(!strcmp(r1->name, r2->name))return 1;

  return 0;
}


/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void pic16_FixRegisterBanking(pBlock *pb)
{
  pCode *pc=NULL;
  pCode *pcprev=NULL;
  reg_info *reg, *prevreg;
  unsigned char flag=0;

        if(!pb)
                return;

        pc = pic16_findNextpCode(pb->pcHead, PC_OPCODE);
        if(!pc)return;

        /* loop through all of the flow blocks with in one pblock */

//      fprintf(stderr,"%s:%d: Register banking\n", __FUNCTION__, __LINE__);

        prevreg = NULL;
        do {
                /* at this point, pc should point to a PC_FLOW object */
                /* for each flow block, determine the register banking
                 * requirements */


                /* if label, then might come from other point, force banksel */
                if(isPCL(pc))prevreg = NULL;

                if(!isPCI(pc))goto loop;

                if(PCI(pc)->label)prevreg = NULL;

                if(PCI(pc)->is2MemOp)goto loop;

                /* if goto, then force banksel */
//              if(PCI(pc)->op == POC_GOTO)prevreg = NULL;

                reg = pic16_getRegFromInstruction(pc);

#if 0
                pc->print(stderr, pc);
                fprintf(stderr, "reg = %p\n", reg);

                if(reg) {
                        fprintf(stderr, "%s:%d:  %s  %d\n",__FUNCTION__, __LINE__, reg->name, reg->rIdx);
                        fprintf(stderr, "addr = 0x%03x, bit=%d\tfix=%d\n",
                                reg->address,reg->isBitField, reg->isFixed);
                }
#endif

                /* now make some tests to make sure that instruction needs bank switch */

                /* if no register exists, and if not a bit opcode goto loop */
                if(!reg) {
                        if(!(PCI(pc)->pcop && PCI(pc)->pcop->type == PO_GPR_BIT))goto loop;
                }

                if(isPCI_SKIP(pc)) {
//                      fprintf(stderr, "instruction is SKIP instruction\n");
//                prevreg = NULL;
                }
                if(reg && isACCESS_BANK(reg))goto loop;

                if(!isBankInstruction(pc))goto loop;

                if(isPCI_LIT(pc))goto loop;

                if(PCI(pc)->op == POC_CALL)goto loop;

                /* Examine the instruction before this one to make sure it is
                 * not a skip type instruction */
                pcprev = findPrevpCode(pc->prev, PC_OPCODE);

                flag = 0;               /* add before this instruction */

                /* if previous instruction is a skip one, then set flag
                 * to 2 and call insertBankSwitch */
                if(pcprev && isPCI_SKIP(pcprev)) {
                  flag=2;       //goto loop
//                prevreg = NULL;
                }

                if(pic16_options.opt_banksel>0) {
                  char op1[128], op2[128];

                    if(prevreg) {
                      strcpy(op1, pic16_get_op_from_instruction(PCI(pc)));
                      strcpy(op2, pic16_get_op_from_instruction(PCI(pcprev)));
                      if(!strcmp(op1, op2))goto loop;
                    }
                }
                prevreg = reg;
                insertBankSwitch(flag, pc);

//              fprintf(stderr, "BANK SWITCH inserted\n");

loop:
                pcprev = pc;
                pc = pc->next;
        } while (pc);
}

/** ADDITIONS BY RAPHAEL NEIDER, 2004-11-16: GOTO OPTIMIZATIONS **/

/* Returns the (maximum of the) number of bytes used by the specified pCode. */
int instrSize (pCode *pc)
{
  if (!pc) return 0;

  if (isPCAD(pc)) {
    if (!PCAD(pc)->directive || strlen (PCAD(pc)->directive) < 3) return 0;
    return 4; // assumes only regular instructions using <= 4 bytes
  }

  if (isPCI(pc)) return PCI(pc)->isize;

  return 0;
}

/* Returns 1 if pc is referenced by the given label (either
 * pc is the label itself or is an instruction with an attached
 * label).
 * Returns 0 if pc is not preceeded by the specified label.
 */
int isLabel (pCode *pc, char *label)
{
  if (!pc) return 0;

  // label attached to the pCode?
  if (isPCI(pc) || isPCAD(pc) || isPCW(pc) || pc->type == PC_INFO) {
    pBranch *lab = NULL;
    lab = PCI(pc)->label;

    while (lab) {
      if (isPCL(lab->pc) && strcmp(PCL(lab->pc)->label, label) == 0) {
        return 1;
      }
      lab = lab->next;
    } // while
  } // if

  // is inline assembly label?
  if (isPCAD(pc) && PCAD(pc)->directive == NULL && PCAD(pc)->arg) {
    // do not compare trailing ':'
    if (strncmp (PCAD(pc)->arg, label, strlen (label)) == 0) {
      return 1;
    }
  } // if

  // is pCodeLabel?
  if (isPCL(pc)) {
      if (strcmp(PCL(pc)->label,label) == 0) {
      return 1;
    }
  } // if

  // no label/no label attached/wrong label(s)
  return 0;
}

/* Returns the distance to the given label in terms of words.
 * Labels are searched only within -max .. max words from pc.
 * Returns max if the label could not be found or
 * its distance from pc in (-max..+max).
 */
int findpCodeLabel (pCode *pc, char *label, int max, pCode **target) {
  int dist = instrSize(pc);
  pCode *curr = pc;

  // search backwards
  while (dist < max && curr && !isLabel (curr, label)) {
    curr = curr->prev;
    dist += instrSize(curr); // sizeof (instruction)
  } // while
  if (curr && dist < max) {
    if (target != NULL) *target = curr;
    return -dist;
  }

  dist = 0;
  curr = pic16_findNextInstruction (pc->next);
  //search forwards
  while (dist < max && curr && !isLabel (curr, label)) {
    dist += instrSize(curr); // sizeof (instruction)
    curr = curr->next;
  } // while
  if (curr && dist < max) {
    if (target != NULL) *target = curr;
    return dist;
  }

  if (target != NULL) *target = NULL;
  return max;
}

/* Returns -1 if pc does NOT denote an instruction like
 * BTFS[SC] STATUS,i
 * Otherwise we return
 *   (a) 0x10 + i for BTFSS
 *   (b) 0x00 + i for BTFSC
 */
int isSkipOnStatus (pCode *pc)
{
  int res = -1;
  pCodeOp *pcop;
  if (!pc || !isPCI(pc)) return -1;
  if (PCI(pc)->op == POC_BTFSS) res = 0x10;
  else if (PCI(pc)->op == POC_BTFSC) res = 0x00;
  else return -1;

  pcop = PCI(pc)->pcop;

  if (pcop->type == PO_STATUS || (pcop->type == PO_GPR_BIT && strcmp(pcop->name, "STATUS") == 0)) {
    return res + ((pCodeOpRegBit *)pcop)->bit;
  }

  return -1;
}

/* Returns 1 if pc is one of BC, BZ, BOV, BN, BNC, BNZ, BNOV or BNN,
 * returns 0 otherwise. */
int isConditionalBranch (pCode *pc)
{
  if (!pc || !isPCI_BRANCH(pc)) return 0;

  switch (PCI(pc)->op) {
  case POC_BC:
  case POC_BZ:
  case POC_BOV:
  case POC_BN:
  case POC_BNC:
  case POC_BNZ:
  case POC_BNOV:
  case POC_BNN:
    return 1;

  default:
    break;
  } // switch

  return 0;
}

/* Returns 1 if pc has a label attached to it.
 * This can be either a label stored in the pCode itself (.label)
 * or a label making up its own pCode preceding this pc.
 * Returns 0 if pc cannot be reached directly via a label.
 */
int hasNoLabel (pCode *pc)
{
  pCode *prev;
  if (!pc) return 1;

  // are there any label pCodes between pc and the previous instruction?
  prev = pic16_findPrevInstruction (pc->prev);
  while (pc && pc != prev) {
    // pCode with attached label?
    if ((isPCI(pc) || isPCAD(pc) || isPCW(pc) || pc->type == PC_INFO)
        && PCI(pc)->label) {
      return 0;
    }
    // is inline assembly label?
    if (isPCAD(pc) && PCAD(pc)->directive == NULL) return 0;
    if (isPCW(pc) && PCW(pc)->label) return 0;

    // pCodeLabel?
    if (isPCL(pc)) return 0;

    pc = pc->prev;
  } // if

  // no label found
  return 1;
}

static void pic16_InsertCommentAfter (pCode *pc, const char *fmt, ...) {
  char buf[512];
  va_list va;

  va_start (va, fmt);
  vsprintf (buf, fmt, va);
  va_end (va);

  pic16_pCodeInsertAfter (pc, pic16_newpCodeCharP(buf));
}

/* Replaces the old pCode with the new one, moving the labels,
 * C source line and probably flow information to the new pCode.
 */
void pic16_pCodeReplace (pCode *oldPC, pCode *newPC) {
  if (!oldPC || !newPC || !isPCI(oldPC) || !isPCI(newPC))
    return;

  /* first move all labels from old to new */
  PCI(newPC)->label = pic16_pBranchAppend (PCI(oldPC)->label, PCI(newPC)->label);
  PCI(oldPC)->label = NULL;

#if 0
  /* move C source line (if possible) */
  if (PCI(oldPC)->cline && !PCI(newPC)->cline)
    PCI(newPC)->cline = PCI(oldPC)->cline;
#endif

  /* keep flow information intact */
  newPC->seq = oldPC->seq;
  PCI(newPC)->pcflow = PCI(oldPC)->pcflow;
  if (PCI(newPC)->pcflow && PCI(newPC)->pcflow->end == oldPC) {
    PCI(newPC)->pcflow->end = newPC;
  }

  /* insert a comment stating which pCode has been replaced */
#if 1
  if (pic16_pcode_verbose || pic16_debug_verbose) {
    char pc_str[256];
    pic16_pCode2str (pc_str, 256, oldPC);
    pic16_InsertCommentAfter (oldPC->prev, "%s: replaced %s", __FUNCTION__, pc_str);
  }
#endif

  /* insert new pCode into pBlock */
  pic16_pCodeInsertAfter (oldPC, newPC);
  pic16_unlinkpCode (oldPC);

  /* destruct replaced pCode */
  oldPC->destruct (oldPC);
}

/* Returns the inverted conditional branch (if any) or NULL.
 * pcop must be set to the new jump target.
 */
pCode *getNegatedBcc (pCode *bcc, pCodeOp *pcop)
{
  pCode *newBcc;

  if (!bcc || !isPCI(bcc)) return NULL;

  switch (PCI(bcc)->op) {
  case POC_BC:   newBcc = pic16_newpCode (POC_BNC , pcop); break;
  case POC_BZ:   newBcc = pic16_newpCode (POC_BNZ , pcop); break;
  case POC_BOV:  newBcc = pic16_newpCode (POC_BNOV, pcop); break;
  case POC_BN:   newBcc = pic16_newpCode (POC_BNN , pcop); break;
  case POC_BNC:  newBcc = pic16_newpCode (POC_BC  , pcop); break;
  case POC_BNZ:  newBcc = pic16_newpCode (POC_BZ  , pcop); break;
  case POC_BNOV: newBcc = pic16_newpCode (POC_BOV , pcop); break;
  case POC_BNN:  newBcc = pic16_newpCode (POC_BN  , pcop); break;
  default:
    newBcc = NULL;
  }
  return newBcc;
}

#define MAX_DIST_GOTO         0x7FFFFFFF
#define MAX_DIST_BRA                1020        // maximum offset (in bytes) possible with BRA
#define MAX_DIST_BCC                 120        // maximum offset (in bytes) possible with Bcc
#define MAX_JUMPCHAIN_DEPTH           16        // number of GOTOs to follow in resolveJumpChain() (to prevent endless loops)
#define IS_GOTO(arg) ((arg) && isPCI(arg) && (PCI(arg)->op == POC_GOTO || PCI(arg)->op == POC_BRA))

/* Follows GOTO/BRA instructions to their target instructions, stores the
 * final destination (not a GOTO or BRA instruction) in target and returns
 * the distance from the original pc to *target.
 */
int resolveJumpChain (pCode *pc, pCode **target, pCodeOp **pcop) {
        pCode *curr = pc;
        pCode *last = NULL;
        pCodeOp *lastPCOP = NULL;
        int dist = 0;
        int depth = 0;

        //fprintf (stderr, "%s:%d: -=-", __FUNCTION__, __LINE__);

        /* only follow unconditional branches, except for the initial pCode (which may be a conditional branch) */
        while (curr && (last != curr) && (depth++ < MAX_JUMPCHAIN_DEPTH) && isPCI(curr)
                        && (PCI(curr)->op == POC_GOTO || PCI(curr)->op == POC_BRA || (curr == pc && isConditionalBranch(curr)))) {
                last = curr;
                lastPCOP = PCI(curr)->pcop;
                dist = findpCodeLabel (pc, PCI(curr)->pcop->name, MAX_DIST_GOTO, &curr);
                //fprintf (stderr, "last:%p, curr:%p, label:%s\n", last, curr, PCI(last)->pcop->name);
        } // while

        if (target) *target = last;
        if (pcop) *pcop = lastPCOP;
        return dist;
}

/* Returns pc if it is not a OPT_JUMPTABLE_BEGIN INFO pCode.
 * Otherwise the first pCode after the jumptable (after
 * the OPT_JUMPTABLE_END tag) is returned.
 */
pCode *skipJumptables (pCode *pc, int *isJumptable)
{
  *isJumptable = 0;

  while (pc && pc->type == PC_INFO && PCINF(pc)->type == INF_OPTIMIZATION && PCOO(PCINF(pc)->oper1)->type == OPT_JUMPTABLE_BEGIN) {
    *isJumptable = 1;
    //fprintf (stderr, "SKIPPING jumptable\n");
    do {
      //pc->print(stderr, pc);
      pc = pc->next;
    } while (pc && (pc->type != PC_INFO || PCINF(pc)->type != INF_OPTIMIZATION
                    || PCOO(PCINF(pc)->oper1)->type != OPT_JUMPTABLE_END));
    //fprintf (stderr, "<<JUMPTAB:\n");
    // skip OPT_END as well
    if (pc) pc = pc->next;
  } // while

  return pc;
}

pCode *pic16_findNextInstructionSkipJumptables (pCode *pc, int *isJumptable)
{
  int isJumptab;
  *isJumptable = 0;
  while (pc && !isPCI(pc) && !isPCAD(pc) && !isPCW(pc)) {
    // set pc to the first pCode after a jumptable, leave pc untouched otherwise
    pc = skipJumptables (pc, &isJumptab);
    if (isJumptab) {
        // pc is the first pCode after the jumptable
        *isJumptable = 1;
    } else {
        // pc has not been changed by skipJumptables()
        pc = pc->next;
    }
  } // while

  return pc;
}

/* Turn GOTOs into BRAs if distance between GOTO and label
 * is less than 1024 bytes.
 *
 * This method is especially useful if GOTOs after BTFS[SC]
 * can be turned into BRAs as GOTO would cost another NOP
 * if skipped.
 */
void pic16_OptimizeJumps ()
{
  pCode *pc;
  pCode *pc_prev = NULL;
  pCode *pc_next = NULL;
  pBlock *pb;
  pCode *target;
  int change, iteration, isJumptab;
  int isHandled = 0;
  char *label;
  int opt=0, toofar=0, opt_cond = 0, cond_toofar=0, opt_reorder = 0, opt_gotonext = 0, opt_gotochain = 0;

  if (!the_pFile) return;

  //fprintf (stderr, "%s:%d: %s\n", __FILE__, __LINE__, __FUNCTION__);

  for (pb = the_pFile->pbHead; pb != NULL; pb = pb->next) {
    int matchedInvertRule = 1;
    iteration = 1;
    do {
      //fprintf (stderr, "%s:%d: iterating over pBlock %p\n", __FUNCTION__, __LINE__, pb);
      change = 0;
      pc = pic16_findNextInstruction (pb->pcHead);

      while (pc) {
        pc_next = pic16_findNextInstructionSkipJumptables (pc->next, &isJumptab);
        if (isJumptab) {
                // skip jumptable, i.e. start over with no pc_prev!
                pc_prev = NULL;
                pc = pc_next;
                continue;
        } // if

        /* (1) resolve chained jumps
         * Do not perform this until pattern (4) is no longer present! Otherwise we will
         * (a) leave dead code in and
         * (b) skip over the dead code with an (unneccessary) jump.
         */
        if (!matchedInvertRule && (IS_GOTO(pc) || isConditionalBranch(pc))) {
          pCodeOp *lastTargetOp = NULL;
          int newDist = resolveJumpChain (pc, &target, &lastTargetOp);
          int maxDist = MAX_DIST_BCC;
          if (PCI(pc)->op == POC_BRA) maxDist = MAX_DIST_BRA;
          if (PCI(pc)->op == POC_GOTO) maxDist = MAX_DIST_GOTO;

          /* be careful NOT to make the jump instruction longer (might break previously shortened jumps!) */
          if (lastTargetOp && newDist <= maxDist && lastTargetOp != PCI(pc)->pcop
              && strcmp (lastTargetOp->name, PCI(pc)->pcop->name) != 0) {
            //fprintf (stderr, "(1) ");pc->print(stderr, pc); fprintf (stderr, " --> %s\n", lastTargetOp->name);
            if (pic16_pcode_verbose) { pic16_pCodeInsertAfter (pc->prev, pic16_newpCodeCharP("(1) jump chain resolved")); }
            PCI(pc)->pcop->name = lastTargetOp->name;
            change++;
            opt_gotochain++;
          } // if
        } // if


        if (IS_GOTO(pc)) {
          int dist;
          int condBraType = isSkipOnStatus(pc_prev);
          label = PCI(pc)->pcop->name;
          dist = findpCodeLabel(pc, label, MAX_DIST_BRA, &target);
          if (dist < 0) dist = -dist;
          //fprintf (stderr, "distance: %d (", dist); pc->print(stderr, pc);fprintf (stderr, ")\n");
          isHandled = 0;


          /* (2) remove "GOTO label; label:" */
          if (isLabel (pc_next, label)) {
            //fprintf (stderr, "(2) GOTO next instruction: ");pc->print(stderr, pc);fprintf (stderr, " --> ");pc_next->print(stderr, pc_next); fprintf(stderr, "\n");
            // first remove all preceeding SKIP instructions
            while (pc_prev && isPCI_SKIP(pc_prev)) {
              // attach labels on this instruction to pc_next
              //fprintf (stderr, "(2) preceeding SKIP removed: ");pc_prev->print(stderr, pc_prev);fprintf(stderr, "\n");
              PCI(pc_next)->label = pic16_pBranchAppend (PCI(pc_prev)->label, PCI(pc_next)->label);
              PCI(pc_prev)->label = NULL;
              if (pic16_pcode_verbose) { pic16_pCodeInsertAfter (pc->prev, pic16_newpCodeCharP("(2) SKIP removed")); }
              pic16_unlinkpCode (pc_prev);
              pc_prev = pic16_findPrevInstruction (pc);
            } // while
            // now remove the redundant goto itself
            PCI(pc_next)->label = pic16_pBranchAppend (PCI(pc)->label, PCI(pc_next)->label);
            if (pic16_pcode_verbose) { pic16_pCodeInsertAfter (pc, pic16_newpCodeCharP("(2) GOTO next instruction removed")); }
            pic16_unlinkpCode (pc);
            pc = pic16_findPrevInstruction(pc_next->prev);
            isHandled = 1; // do not perform further optimizations
            opt_gotonext++;
            change++;
          } // if


          /* (3) turn BTFSx STATUS,i; GOTO label into Bcc label if possible */
          if (!isHandled && condBraType != -1 && hasNoLabel(pc)) {
            if (dist < MAX_DIST_BCC) {
              pCode *bcc = NULL;
              switch (condBraType) {
              case 0x00: bcc = pic16_newpCode (POC_BC, PCI(pc)->pcop);break;
                // no BDC on DIGIT CARRY available
              case 0x02: bcc = pic16_newpCode (POC_BZ, PCI(pc)->pcop);break;
              case 0x03: bcc = pic16_newpCode (POC_BOV, PCI(pc)->pcop);break;
              case 0x04: bcc = pic16_newpCode (POC_BN, PCI(pc)->pcop);break;
              case 0x10: bcc = pic16_newpCode (POC_BNC, PCI(pc)->pcop);break;
                // no BNDC on DIGIT CARRY available
              case 0x12: bcc = pic16_newpCode (POC_BNZ, PCI(pc)->pcop);break;
              case 0x13: bcc = pic16_newpCode (POC_BNOV, PCI(pc)->pcop);break;
              case 0x14: bcc = pic16_newpCode (POC_BNN, PCI(pc)->pcop);break;
              default:
                // no replacement possible
                bcc = NULL;
                break;
              } // switch
              if (bcc) {
                // ATTENTION: keep labels attached to BTFSx!
                // HINT: GOTO is label free (checked above)
                //fprintf (stderr, "%s:%d: (3) turning %s %s into %s %s\n", __FUNCTION__, __LINE__, PCI(pc)->mnemonic, label, PCI(bcc)->mnemonic, label);
                isHandled = 1; // do not perform further optimizations
                if (pic16_pcode_verbose) { pic16_pCodeInsertAfter(pc_prev->prev, pic16_newpCodeCharP("(3) conditional branch introduced")); }
                pic16_pCodeReplace (pc_prev, bcc);
                pc->destruct(pc);
                pc = bcc;
                opt_cond++;
                change++;
              } // if
            } else {
              //fprintf (stderr, "(%d, too far for Bcc)\n", dist);
              cond_toofar++;
            } // if
          } // if

          if (!isHandled) {
            // (4) eliminate the following (common) tripel:
            //           <pred.>;
            //  labels1: Bcc label2;
            //           GOTO somewhere;    ; <-- instruction referenced by pc
            //  label2:  <cont.>
            // and replace it by
            //  labels1: B#(cc) somewhere;  ; #(cc) is the negated condition cc
            //  label2:  <cont.>
            // ATTENTION: all labels pointing to "Bcc label2" must be attached
            //            to <cont.> instead
            // ATTENTION: This optimization is only valid if <pred.> is
            //            not a skip operation!
            // ATTENTION: somewhere must be within MAX_DIST_BCC bytes!
            // ATTENTION: no label may be attached to the GOTO instruction!
            if (isConditionalBranch(pc_prev)
                && (!isPCI_SKIP(pic16_findPrevInstruction(pc_prev->prev)))
                && (dist < MAX_DIST_BCC)
                && isLabel(pc_next,PCI(pc_prev)->pcop->name)
                && hasNoLabel(pc)) {
              pCode *newBcc = getNegatedBcc (pc_prev, PCI(pc)->pcop);

              if (newBcc) {
                //fprintf (stderr, "%s:%d: (4) turning %s %s into %s %s\n", __FUNCTION__, __LINE__, PCI(pc)->mnemonic, label, PCI(newBcc)->mnemonic, label);
                isHandled = 1; // do not perform further optimizations
                if (pic16_pcode_verbose) { pic16_pCodeInsertAfter(pc_prev->prev, pic16_newpCodeCharP("(4) conditional skipping branch inverted")); }
                pic16_pCodeReplace (pc_prev, newBcc);
                pc->destruct(pc);
                pc = newBcc;
                opt_reorder++;
                change++;
                matchedInvertRule++;
              }
            }
          }

          /* (5) now just turn GOTO into BRA */
          if (!isHandled && (PCI(pc)->op == POC_GOTO)) {
            if (dist < MAX_DIST_BRA) {
              pCode *newBra = pic16_newpCode (POC_BRA, PCI(pc)->pcop);
              //fprintf (stderr, "%s:%d: (5) turning %s %s into %s %s\n", __FUNCTION__, __LINE__, PCI(pc)->mnemonic, label, PCI(newBra)->mnemonic, label);
              if (pic16_pcode_verbose) { pic16_pCodeInsertAfter(pc->prev, pic16_newpCodeCharP("(5) GOTO replaced by BRA")); }
              pic16_pCodeReplace (pc, newBra);
              pc = newBra;
              opt++;
              change++;
            } else {
              //fprintf (stderr, "(%d, too far for BRA)\n", dist);
              toofar++;
            }
          } // if (!isHandled)
        } // if

        pc_prev = pc;
        pc = pc_next;
      } // while (pc)

      pBlockRemoveUnusedLabels (pb);

      // This line enables goto chain resolution!
      if (matchedInvertRule > 1) matchedInvertRule = 1; else matchedInvertRule = 0;

      iteration++;
    } while (change); /* fixpoint iteration per pBlock */
  } // for (pb)

  // emit some statistics concerning goto-optimization
#if 0
  if (pic16_debug_verbose || pic16_pcode_verbose) {
    fprintf (stderr, "optimize-goto:\n"
             "\t%5d GOTO->BRA; (%d GOTOs too far)\n"
             "\t%5d BTFSx, GOTO->Bcc (%d too far)\n"
             "\t%5d conditional \"skipping\" jumps inverted\n"
             "\t%5d GOTOs to next instruction removed\n"
             "\t%5d chained GOTOs resolved\n",
             opt, toofar, opt_cond, cond_toofar, opt_reorder, opt_gotonext, opt_gotochain);
  } // if
#endif
  //fprintf (stderr, "%s:%d: %s\n", __FILE__, __LINE__, __FUNCTION__);
}

#undef IS_GOTO
#undef MAX_JUMPCHAIN_DEPTH
#undef MAX_DIST_GOTO
#undef MAX_DIST_BRA
#undef MAX_DIST_BCC

/** END OF RAPHAEL NEIDER'S ADDITIONS **/

static void pBlockDestruct(pBlock *pb)
{

  if(!pb)
    return;


//  Safe_free(pb);

}

/*-----------------------------------------------------------------*/
/* void mergepBlocks(char dbName) - Search for all pBlocks with the*/
/*                                  name dbName and combine them   */
/*                                  into one block                 */
/*-----------------------------------------------------------------*/
static void mergepBlocks(char dbName)
{

  pBlock *pb, *pbmerged = NULL,*pbn;

  pb = the_pFile->pbHead;

  //fprintf(stderr," merging blocks named %c\n",dbName);
  while(pb) {

    pbn = pb->next;
    //fprintf(stderr,"looking at %c\n",getpBlock_dbName(pb));
    if( getpBlock_dbName(pb) == dbName) {

      //fprintf(stderr," merged block %c\n",dbName);

      if(!pbmerged) {
        pbmerged = pb;
      } else {
        pic16_addpCode2pBlock(pbmerged, pb->pcHead);
        /* pic16_addpCode2pBlock doesn't handle the tail: */
        pbmerged->pcTail = pb->pcTail;

        pb->prev->next = pbn;
        if(pbn)
          pbn->prev = pb->prev;


        pBlockDestruct(pb);
      }
      //pic16_printpBlock(stderr, pbmerged);
    }
    pb = pbn;
  }

}

/*-----------------------------------------------------------------*/
/* AnalyzeFlow - Examine the flow of the code and optimize         */
/*                                                                 */
/* level 0 == minimal optimization                                 */
/*   optimize registers that are used only by two instructions     */
/* level 1 == maximal optimization                                 */
/*   optimize by looking at pairs of instructions that use the     */
/*   register.                                                     */
/*-----------------------------------------------------------------*/

static void AnalyzeFlow(int level)
{
  static int times_called=0;
  pBlock *pb;

    if(!the_pFile) {
      /* remove unused allocated registers before exiting */
      pic16_RemoveUnusedRegisters();
      return;
    }


    /* if this is not the first time this function has been called,
     * then clean up old flow information */
    if(times_called++) {
      for(pb = the_pFile->pbHead; pb; pb = pb->next)
        unBuildFlow(pb);
        pic16_RegsUnMapLiveRanges();
    }
    GpcFlowSeq = 1;

    /* Phase 2 - Flow Analysis - Register Banking
     *
     * In this phase, the individual flow blocks are examined
     * and register banking is fixed.
     */

#if 0
    for(pb = the_pFile->pbHead; pb; pb = pb->next)
      pic16_FixRegisterBanking(pb);
#endif

    /* Phase 2 - Flow Analysis
     *
     * In this phase, the pCode is partition into pCodeFlow
     * blocks. The flow blocks mark the points where a continuous
     * stream of instructions changes flow (e.g. because of
     * a call or goto or whatever).
     */

    for(pb = the_pFile->pbHead; pb; pb = pb->next)
      pic16_BuildFlow(pb);


    /* Phase 2 - Flow Analysis - linking flow blocks
     *
     * In this phase, the individual flow blocks are examined
     * to determine their order of excution.
     */

    for(pb = the_pFile->pbHead; pb; pb = pb->next)
      LinkFlow(pb);

#if 1
        if (pic16_options.opt_flags & OF_OPTIMIZE_DF) {
                for(pb = the_pFile->pbHead; pb; pb = pb->next) {
                        pic16_createDF (pb);
#if defined (DUMP_DF_GRAPHS) && DUMP_DF_GRAPHS > 0
                        pic16_vcg_dump_default (pb);
#endif
                        //pic16_destructDF (pb);
                }

                pic16_df_stats ();
                if (0) releaseStack (); // releasing is costly...
        }
#endif

    /* Phase 3 - Flow Analysis - Flow Tree
     *
     * In this phase, the individual flow blocks are examined
     * to determine their order of execution.
     */

    for(pb = the_pFile->pbHead; pb; pb = pb->next)
      pic16_BuildFlowTree(pb);


    /* Phase x - Flow Analysis - Used Banks
     *
     * In this phase, the individual flow blocks are examined
     * to determine the Register Banks they use
     */

#if 0
    for(pb = the_pFile->pbHead; pb; pb = pb->next)
      FixBankFlow(pb);
#endif


    for(pb = the_pFile->pbHead; pb; pb = pb->next)
      pic16_pCodeRegMapLiveRanges(pb);

    pic16_RemoveUnusedRegisters();
    pic16_removeUnusedRegistersDF ();

  //  for(pb = the_pFile->pbHead; pb; pb = pb->next)
    pic16_pCodeRegOptimizeRegUsage(level);


#if 0
    if(!options.nopeep)
      OptimizepCode('*');
#endif

#if 0
    for(pb = the_pFile->pbHead; pb; pb = pb->next)
      DumpFlow(pb);
#endif

    /* debug stuff */
    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
      pCode *pcflow;

        for( pcflow = pic16_findNextpCode(pb->pcHead, PC_FLOW);
          (pcflow = pic16_findNextpCode(pcflow, PC_FLOW)) != NULL;
          pcflow = pcflow->next) {
            FillFlow(PCFL(pcflow));
        }
    }

#if 0
    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
      pCode *pcflow;

        for( pcflow = pic16_findNextpCode(pb->pcHead, PC_FLOW);
          (pcflow = pic16_findNextpCode(pcflow, PC_FLOW)) != NULL;
          pcflow = pcflow->next) {
            FlowStats(PCFL(pcflow));
        }
    }
#endif
}

/* VR -- no need to analyze banking in flow, but left here :
 *      1. because it may be used in the future for other purposes
 *      2. because if omitted we'll miss some optimization done here
 *
 * Perhaps I should rename it to something else
 */

/*-----------------------------------------------------------------*/
/* pic16_AnalyzeBanking - Called after the memory addresses have been    */
/*                  assigned to the registers.                     */
/*                                                                 */
/*-----------------------------------------------------------------*/

void pic16_AnalyzeBanking(void)
{
  pBlock  *pb;

    /* Phase x - Flow Analysis - Used Banks
     *
     * In this phase, the individual flow blocks are examined
     * to determine the Register Banks they use
     */

    AnalyzeFlow(0);
    AnalyzeFlow(1);

    if(!options.nopeep)
      OptimizepCode('*');


    if(!the_pFile)return;

    if(!pic16_options.no_banksel) {
      for(pb = the_pFile->pbHead; pb; pb = pb->next) {
//        fprintf(stderr, "%s:%d: Fix register banking in pb= 0x%p\n", __FILE__, __LINE__, pb);
        pic16_FixRegisterBanking(pb);
      }
    }
}

/*-----------------------------------------------------------------*/
/* buildCallTree - Look at the flow and extract all of the calls.  */
/*-----------------------------------------------------------------*/
#if 0
static set *register_usage(pBlock *pb);
#endif

static void buildCallTree(void    )
{
  pBranch *pbr;
  pBlock  *pb;
  pCode   *pc;
  reg_info *r;

  if(!the_pFile)
    return;



  /* Now build the call tree.
     First we examine all of the pCodes for functions.
     Keep in mind that the function boundaries coincide
     with pBlock boundaries.

     The algorithm goes something like this:
     We have two nested loops. The outer loop iterates
     through all of the pBlocks/functions. The inner
     loop iterates through all of the pCodes for
     a given pBlock. When we begin iterating through
     a pBlock, the variable pc_fstart, pCode of the start
     of a function, is cleared. We then search for pCodes
     of type PC_FUNCTION. When one is encountered, we
     initialize pc_fstart to this and at the same time
     associate a new pBranch object that signifies a
     branch entry. If a return is found, then this signifies
     a function exit point. We'll link the pCodes of these
     returns to the matching pc_fstart.

     When we're done, a doubly linked list of pBranches
     will exist. The head of this list is stored in
     `the_pFile', which is the meta structure for all
     of the pCode. Look at the pic16_printCallTree function
     on how the pBranches are linked together.

   */
  for(pb = the_pFile->pbHead; pb; pb = pb->next) {
    pCode *pc_fstart=NULL;
    for(pc = pb->pcHead; pc; pc = pc->next) {

        if(isPCI(pc) && pc_fstart) {
                if(PCI(pc)->is2MemOp) {
                        r = pic16_getRegFromInstruction2(pc);
                        if(r && !strcmp(r->name, "POSTDEC1"))
                                PCF(pc_fstart)->stackusage++;
                } else {
                        r = pic16_getRegFromInstruction(pc);
                        if(r && !strcmp(r->name, "PREINC1"))
                                PCF(pc_fstart)->stackusage--;
                }
        }

      if(isPCF(pc)) {
        if (PCF(pc)->fname) {
        char buf[16];

          sprintf(buf, "%smain", port->fun_prefix);
          if(STRCASECMP(PCF(pc)->fname, buf) == 0) {
            //fprintf(stderr," found main \n");
            pb->cmemmap = NULL;  /* FIXME do we need to free ? */
            pb->dbName = 'M';
          }

          pbr = Safe_calloc(1,sizeof(pBranch));
          pbr->pc = pc_fstart = pc;
          pbr->next = NULL;

          the_pFile->functions = pic16_pBranchAppend(the_pFile->functions,pbr);

          // Here's a better way of doing the same:
          addSet(&pb->function_entries, pc);

        } else {
          // Found an exit point in a function, e.g. return
          // (Note, there may be more than one return per function)
          if(pc_fstart)
            pBranchLink(PCF(pc_fstart), PCF(pc));

          addSet(&pb->function_exits, pc);
        }
      } else if(isCALL(pc)) {
        addSet(&pb->function_calls,pc);
      }
    }
  }


#if 0
  /* This is not needed because currently all register used
   * by a function are stored in stack -- VR */

  /* Re-allocate the registers so that there are no collisions
   * between local variables when one function call another */

  // this is weird...
  //  pic16_deallocateAllRegs();

  for(pb = the_pFile->pbHead; pb; pb = pb->next) {
    if(!pb->visited)
      register_usage(pb);
  }
#endif

}

/*-----------------------------------------------------------------*/
/* pic16_AnalyzepCode - parse the pCode that has been generated and form */
/*                all of the logical connections.                  */
/*                                                                 */
/* Essentially what's done here is that the pCode flow is          */
/* determined.                                                     */
/*-----------------------------------------------------------------*/

void pic16_AnalyzepCode(char dbName)
{
  pBlock *pb;
  int i,changes;

  if(!the_pFile)
    return;

  mergepBlocks('D');


  /* Phase 1 - Register allocation and peep hole optimization
   *
   * The first part of the analysis is to determine the registers
   * that are used in the pCode. Once that is done, the peep rules
   * are applied to the code. We continue to loop until no more
   * peep rule optimizations are found (or until we exceed the
   * MAX_PASSES threshold).
   *
   * When done, the required registers will be determined.
   *
   */
  i = 0;
  do {

    DFPRINTF((stderr," Analyzing pCode: PASS #%d\n",i+1));
    //fprintf(stderr," Analyzing pCode: PASS #%d\n",i+1);

    /* First, merge the labels with the instructions */
    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
      if('*' == dbName || getpBlock_dbName(pb) == dbName) {

        DFPRINTF((stderr," analyze and merging block %c\n",dbName));
        //fprintf(stderr," analyze and merging block %c\n",dbName);
        pic16_pBlockMergeLabels(pb);
        AnalyzepBlock(pb);
      } else {
        DFPRINTF((stderr," skipping block analysis dbName=%c blockname=%c\n",dbName,getpBlock_dbName));
      }
    }

        if(!options.nopeep)
                changes = OptimizepCode(dbName);
        else changes = 0;

  } while(changes && (i++ < MAX_PASSES));


  buildCallTree();
}


/* convert a series of movff's of local regs to stack, with a single call to
 * a support functions which does the same thing via loop */
static void pic16_convertLocalRegs2Support(pCode *pcstart, pCode *pcend, int count, reg_info *r, int entry)
{
  pBranch *pbr;
  pCode *pc, *pct;
  char *fname[]={"__lr_store", "__lr_restore"};

//    pc = pic16_newpCode(POC_CALL, pic16_popGetFromString( (entry?fname[0]:fname[1]) ));

    pct = pic16_findNextInstruction(pcstart->next);
    do {
      pc = pct;
      pct = pc->next;   //pic16_findNextInstruction(pc->next);
//      pc->print(stderr, pc);
      if(isPCI(pc) && PCI(pc)->label) {
        pbr = PCI(pc)->label;
        while(pbr && pbr->pc) {
          PCI(pcstart)->label = pic16_pBranchAppend(PCI(pcstart)->label, pbr);
          pbr = pbr->next;
        }

//        pc->print(stderr, pc);
        /* unlink pCode */
        pc->prev->next = pct;
        pct->prev = pc->prev;
//        pc->next = NULL;
//        pc->prev = NULL;
      }
    } while (pc != pcend);

    /* unlink movff instructions */
    pcstart->next = pcend;
    pcend->prev = pcstart;

    pc = pcstart;
//    if(!entry) {
//      pic16_pCodeInsertAfter(pc, pct = pic16_newpCode(POC_MOVFF, pic16_popGet2p(
//              pic16_popCopyReg(&pic16_pc_fsr0l), pic16_popCopyReg(pic16_framepnt_lo)))); pc = pct;
//    }

    pic16_pCodeInsertAfter(pc, pct=pic16_newpCode(POC_LFSR, pic16_popGetLit2(0, pic16_popGetWithString(r->name)))); pc = pct;
    pic16_pCodeInsertAfter(pc, pct=pic16_newpCode(POC_MOVLW, pic16_popGetLit( count ))); pc = pct;
    pic16_pCodeInsertAfter(pc, pct=pic16_newpCode(POC_CALL, pic16_popGetWithString( fname[ (entry==1?0:1) ] ))); pc = pct;

//    if(!entry) {
//      pic16_pCodeInsertAfter(pc, pct = pic16_newpCode(POC_MOVFF, pic16_popGet2p(
//              pic16_popCopyReg(pic16_framepnt_lo), pic16_popCopyReg(&pic16_pc_fsr0l)))); pc = pct;
//    }


    {
      symbol *sym;

        sym = newSymbol( fname[ entry?0:1 ], 0 );
        strcpy(sym->rname, fname[ entry?0:1 ]);
        checkAddSym(&externs, sym);

//        fprintf(stderr, "%s:%d adding extern symbol %s in externs\n", __FILE__, __LINE__, fname[ entry?0:1 ]);
    }

}

/*-----------------------------------------------------------------*/
/* OptimizeLocalRegs - turn sequence of MOVFF instructions for     */
/*    local registers to a support function call                   */
/*-----------------------------------------------------------------*/
void pic16_OptimizeLocalRegs(void)
{
  pBlock  *pb;
  pCode   *pc;
  pCodeInfo *pci;
  pCodeOpLocalReg *pclr;
  int regCount=0;
  int inRegCount=0;
  reg_info *r, *lastr=NULL, *firstr=NULL;
  pCode *pcstart=NULL, *pcend=NULL;
  int inEntry=0;
  char *curFunc=NULL;

        /* Overview:
         *   local_regs begin mark
         *      MOVFF r0x01, POSTDEC1
         *      MOVFF r0x02, POSTDEC1
         *      ...
         *      ...
         *      MOVFF r0x0n, POSTDEC1
         *   local_regs end mark
         *
         * convert the above to the below:
         *      MOVLW   starting_register_index
         *      MOVWF   PRODL
         *      MOVLW   register_count
         *      call    __save_registers_in_stack
         */

    if(!the_pFile)
      return;

    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
      inRegCount = regCount = 0;
      firstr = lastr = NULL;
      for(pc = pb->pcHead; pc; pc = pc->next) {

        /* hold current function name */
        if(pc && isPCF(pc))curFunc = PCF(pc)->fname;

        if(pc && (pc->type == PC_INFO)) {
          pci = PCINF(pc);

          if(pci->type == INF_LOCALREGS) {
            pclr = PCOLR(pci->oper1);

            if((pclr->type == LR_ENTRY_BEGIN)
              || (pclr->type == LR_ENTRY_END))inEntry = 1;
            else inEntry = 0;

            switch(pclr->type) {
              case LR_ENTRY_BEGIN:
              case LR_EXIT_BEGIN:
                        inRegCount = 1; regCount = 0;
                        pcstart = pc;   //pic16_findNextInstruction(pc->next);
                        firstr = lastr = NULL;
                        break;

              case LR_ENTRY_END:
              case LR_EXIT_END:
                        inRegCount = -1;
                        pcend = pc;     //pic16_findPrevInstruction(pc->prev);

#if 1
                        if(curFunc && inWparamList(curFunc+1)) {
                          fprintf(stderr, "sdcc: %s: warning: disabling lr-support for function %s\n",
                                        filename, curFunc);
                        } else {
                          if(regCount>2) {
                            pic16_convertLocalRegs2Support(pcstart, pcend, regCount,
                              firstr, inEntry);
                          }
                        }
#endif
                        firstr = lastr = NULL;
                        break;
            }

            if(inRegCount == -1) {
//              fprintf(stderr, "%s:%d registers used [%s] %d\n", __FILE__, __LINE__, inEntry?"entry":"exit", regCount);
              regCount = 0;
              inRegCount = 0;
            }
          }
        } else {
          if(isPCI(pc) && (PCI(pc)->op == POC_MOVFF) && (inRegCount == 1)) {
            if(inEntry)
              r = pic16_getRegFromInstruction(pc);
            else
              r = pic16_getRegFromInstruction2(pc);
            if(r && (r->type == REG_GPR) && (r->pc_type == PO_GPR_TEMP)) {
              if(!firstr)firstr = r;
              regCount++;
//              fprintf(stderr, "%s:%d\t%s\t%i\t%d/%d\n", __FILE__, __LINE__, r->name, r->rIdx);
            }
          }
        }
      }
    }
}





/*-----------------------------------------------------------------*/
/* ispCodeFunction - returns true if *pc is the pCode of a         */
/*                   function                                      */
/*-----------------------------------------------------------------*/
static bool ispCodeFunction(pCode *pc)
{

  if(pc && pc->type == PC_FUNCTION && PCF(pc)->fname)
    return 1;

  return 0;
}

/*-----------------------------------------------------------------*/
/* findFunction - Search for a function by name (given the name)   */
/*                in the set of all functions that are in a pBlock */
/* (note - I expect this to change because I'm planning to limit   */
/*  pBlock's to just one function declaration                      */
/*-----------------------------------------------------------------*/
static pCode *findFunction(char *fname)
{
  pBlock *pb;
  pCode *pc;
  if(!fname)
    return NULL;

  for(pb = the_pFile->pbHead; pb; pb = pb->next) {

    pc = setFirstItem(pb->function_entries);
    while(pc) {

      if((pc->type == PC_FUNCTION) &&
         (PCF(pc)->fname) &&
         (strcmp(fname, PCF(pc)->fname)==0))
        return pc;

      pc = setNextItem(pb->function_entries);

    }

  }
  return NULL;
}

#if 0
static void MarkUsedRegisters(set *regset)
{

  regs *r1,*r2;

  for(r1=setFirstItem(regset); r1; r1=setNextItem(regset)) {
//      fprintf(stderr, "marking register = %s\t", r1->name);
    r2 = pic16_regWithIdx(r1->rIdx);
//      fprintf(stderr, "to register = %s\n", r2->name);
    r2->isFree = 0;
    r2->wasUsed = 1;
  }
}
#endif

static void pBlockStats(FILE *of, pBlock *pb)
{

  pCode *pc;
  reg_info  *r;

        if(!pic16_pcode_verbose)return;

  fprintf(of,";***\n;  pBlock Stats: dbName = %c\n;***\n",getpBlock_dbName(pb));

  // for now just print the first element of each set
  pc = setFirstItem(pb->function_entries);
  if(pc) {
    fprintf(of,";entry:  ");
    pc->print(of,pc);
  }
  pc = setFirstItem(pb->function_exits);
  if(pc) {
    fprintf(of,";has an exit\n");
    //pc->print(of,pc);
  }

  pc = setFirstItem(pb->function_calls);
  if(pc) {
    fprintf(of,";functions called:\n");

    while(pc) {
      if(pc->type == PC_OPCODE && PCI(pc)->op == POC_CALL) {
        fprintf(of,";   %s\n",pic16_get_op_from_instruction(PCI(pc)));
      }
      pc = setNextItem(pb->function_calls);
    }
  }

  r = setFirstItem(pb->tregisters);
  if(r) {
    int n = elementsInSet(pb->tregisters);

    fprintf(of,";%d compiler assigned register%c:\n",n, ( (n!=1) ? 's' : ' '));

    while (r) {
      fprintf(of,   ";   %s\n",r->name);
      r = setNextItem(pb->tregisters);
    }
  }

  fprintf(of, "; uses %d bytes of stack\n", 1+ elementsInSet(pb->tregisters));
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
#if 0
static void sequencepCode(void)
{
  pBlock *pb;
  pCode *pc;


  for(pb = the_pFile->pbHead; pb; pb = pb->next) {

    pb->seq = GpCodeSequenceNumber+1;

    for( pc = pb->pcHead; pc; pc = pc->next)
      pc->seq = ++GpCodeSequenceNumber;
  }

}
#endif

#if 0
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static set *register_usage(pBlock *pb)
{
  pCode *pc,*pcn;
  set *registers=NULL;
  set *registersInCallPath = NULL;

  /* check recursion */

  pc = setFirstItem(pb->function_entries);

  if(!pc)
    return registers;

  pb->visited = 1;

  if(pc->type != PC_FUNCTION)
    fprintf(stderr,"%s, first pc is not a function???\n",__FUNCTION__);

  pc = setFirstItem(pb->function_calls);
  for( ; pc; pc = setNextItem(pb->function_calls)) {

    if(pc->type == PC_OPCODE && PCI(pc)->op == POC_CALL) {
      char *dest = pic16_get_op_from_instruction(PCI(pc));

      pcn = findFunction(dest);
      if(pcn)
        registersInCallPath = register_usage(pcn->pb);
    } else
      fprintf(stderr,"BUG? pCode isn't a POC_CALL %d\n",__LINE__);

  }

#ifdef PCODE_DEBUG
  pBlockStats(stderr,pb);  // debug
#endif

  // Mark the registers in this block as used.

  MarkUsedRegisters(pb->tregisters);
  if(registersInCallPath) {
    /* registers were used in the functions this pBlock has called */
    /* so now, we need to see if these collide with the ones we are */
    /* using here */

    regs *r1,*r2, *newreg;

    DFPRINTF((stderr,"comparing registers\n"));

    r1 = setFirstItem(registersInCallPath);
    while(r1) {

      r2 = setFirstItem(pb->tregisters);

      while(r2 && (r1->type != REG_STK)) {

        if(r2->rIdx == r1->rIdx) {
          newreg = pic16_findFreeReg(REG_GPR);


          if(!newreg) {
            DFPRINTF((stderr,"Bummer, no more registers.\n"));
            exit(1);
          }

          DFPRINTF((stderr,"Cool found register collision nIdx=%d moving to %d\n",
                  r1->rIdx, newreg->rIdx));
          r2->rIdx = newreg->rIdx;
          //if(r2->name) Safe_free(r2->name);
          if(newreg->name)
            r2->name = Safe_strdup(newreg->name);
          else
            r2->name = NULL;
          newreg->isFree = 0;
          newreg->wasUsed = 1;
        }
        r2 = setNextItem(pb->tregisters);
      }

      r1 = setNextItem(registersInCallPath);
    }

    /* Collisions have been resolved. Now free the registers in the call path */
    r1 = setFirstItem(registersInCallPath);
    while(r1) {
      if(r1->type != REG_STK) {
        newreg = pic16_regWithIdx(r1->rIdx);
        newreg->isFree = 1;
      }
      r1 = setNextItem(registersInCallPath);
    }

  }// else
  //    MarkUsedRegisters(pb->registers);

  registers = unionSets(pb->tregisters, registersInCallPath, THROW_NONE);
#ifdef PCODE_DEBUG
  if(registers)
    DFPRINTF((stderr,"returning regs\n"));
  else
    DFPRINTF((stderr,"not returning regs\n"));

  DFPRINTF((stderr,"pBlock after register optim.\n"));
  pBlockStats(stderr,pb);  // debug
#endif

  return registers;
}
#endif

/*-----------------------------------------------------------------*/
/* pct2 - writes the call tree to a file                           */
/*                                                                 */
/*-----------------------------------------------------------------*/
static void pct2(FILE *of,pBlock *pb,int indent,int usedstack)
{
  pCode *pc,*pcn;
  int i;
  //  set *registersInCallPath = NULL;

  if(!of)
    return;

  if(indent > 10) {
        fprintf(of, "recursive function\n");
    return; //recursion ?
  }

  pc = setFirstItem(pb->function_entries);

  if(!pc)
    return;

  pb->visited = 0;

  for(i=0;i<indent;i++)   // Indentation
        fputs("+   ", of);
  fputs("+- ", of);

  if(pc->type == PC_FUNCTION) {
    usedstack += PCF(pc)->stackusage;
    fprintf(of,"%s (stack: %i)\n",PCF(pc)->fname, usedstack);
  } else return;  // ???


  pc = setFirstItem(pb->function_calls);
  for( ; pc; pc = setNextItem(pb->function_calls)) {

    if(pc->type == PC_OPCODE && PCI(pc)->op == POC_CALL) {
      char *dest = pic16_get_op_from_instruction(PCI(pc));

      pcn = findFunction(dest);
      if(pcn)
        pct2(of,pcn->pb,indent+1, usedstack);   // + PCF(pcn)->stackusage);
    } else
      fprintf(of,"BUG? pCode isn't a POC_CALL %d\n",__LINE__);

  }


}


/*-----------------------------------------------------------------*/
/* pic16_printCallTree - writes the call tree to a file                  */
/*                                                                 */
/*-----------------------------------------------------------------*/

void pic16_printCallTree(FILE *of)
{
  pBranch *pbr;
  pBlock  *pb;
  pCode   *pc;

  if(!the_pFile)
    return;

  if(!of)
    of = stderr;

  fprintf(of, "\npBlock statistics\n");
  for(pb = the_pFile->pbHead; pb;  pb = pb->next )
    pBlockStats(of,pb);


  fprintf(of,"Call Tree\n");
  pbr = the_pFile->functions;
  while(pbr) {
    if(pbr->pc) {
      pc = pbr->pc;
      if(!ispCodeFunction(pc))
        fprintf(of,"bug in call tree");


      fprintf(of,"Function: %s\n", PCF(pc)->fname);

      while(pc->next && !ispCodeFunction(pc->next)) {
        pc = pc->next;
        if(pc->type == PC_OPCODE && PCI(pc)->op == POC_CALL)
          fprintf(of,"\t%s\n",pic16_get_op_from_instruction(PCI(pc)));
      }
    }

    pbr = pbr->next;
  }


  fprintf(of,"\n**************\n\na better call tree\n");
  for(pb = the_pFile->pbHead; pb; pb = pb->next) {
//    if(pb->visited)
      pct2(of,pb,0,0);
  }

  for(pb = the_pFile->pbHead; pb; pb = pb->next) {
    fprintf(of,"block dbname: %c\n", getpBlock_dbName(pb));
  }
}



/*-----------------------------------------------------------------*/
/*                                                                 */
/*-----------------------------------------------------------------*/

static void InlineFunction(pBlock *pb)
{
  pCode *pc;
  pCode *pc_call;

  if(!pb)
    return;

  pc = setFirstItem(pb->function_calls);

  for( ; pc; pc = setNextItem(pb->function_calls)) {

    if(isCALL(pc)) {
      pCode *pcn = findFunction(pic16_get_op_from_instruction(PCI(pc)));
      pCode *pct;
      pCode *pce;

      pBranch *pbr;

      if(pcn && isPCF(pcn) && (PCF(pcn)->ncalled == 0)) {               /* change 0 to 1 to enable inlining */

        //fprintf(stderr,"Cool can inline:\n");
        //pcn->print(stderr,pcn);

        //fprintf(stderr,"recursive call Inline\n");
        InlineFunction(pcn->pb);
        //fprintf(stderr,"return from recursive call Inline\n");

        /*
          At this point, *pc points to a CALL mnemonic, and
          *pcn points to the function that is being called.

          To in-line this call, we need to remove the CALL
          and RETURN(s), and link the function pCode in with
          the CALLee pCode.

        */


        /* Remove the CALL */
        pc_call = pc;
        pc = pc->prev;

        /* remove callee pBlock from the pBlock linked list */
        removepBlock(pcn->pb);

        pce = pcn;
        while(pce) {
          pce->pb = pb;
          pce = pce->next;
        }

        /* Remove the Function pCode */
        pct = pic16_findNextInstruction(pcn->next);

        /* Link the function with the callee */
        pc->next = pcn->next;
        pcn->next->prev = pc;

        /* Convert the function name into a label */

        pbr = Safe_calloc(1,sizeof(pBranch));
        pbr->pc = pic16_newpCodeLabel(PCF(pcn)->fname, -1);
        pbr->next = NULL;
        PCI(pct)->label = pic16_pBranchAppend(PCI(pct)->label,pbr);
        PCI(pct)->label = pic16_pBranchAppend(PCI(pct)->label,PCI(pc_call)->label);

        /* turn all of the return's except the last into goto's */
        /* check case for 2 instruction pBlocks */
        pce = pic16_findNextInstruction(pcn->next);
        while(pce) {
          pCode *pce_next = pic16_findNextInstruction(pce->next);

          if(pce_next == NULL) {
            /* found the last return */
            pCode *pc_call_next =  pic16_findNextInstruction(pc_call->next);

            //fprintf(stderr,"found last return\n");
            //pce->print(stderr,pce);
            pce->prev->next = pc_call->next;
            pc_call->next->prev = pce->prev;
            PCI(pc_call_next)->label = pic16_pBranchAppend(PCI(pc_call_next)->label,
                                                      PCI(pce)->label);
          }

          pce = pce_next;
        }


      }
    } else
      fprintf(stderr,"BUG? pCode isn't a POC_CALL %d\n",__LINE__);

  }

}

/*-----------------------------------------------------------------*/
/*                                                                 */
/*-----------------------------------------------------------------*/

void pic16_InlinepCode(void)
{

  pBlock  *pb;
  pCode   *pc;

  if(!the_pFile)
    return;

  if(!functionInlining)
    return;

  /* Loop through all of the function definitions and count the
   * number of times each one is called */
  //fprintf(stderr,"inlining %d\n",__LINE__);

  for(pb = the_pFile->pbHead; pb; pb = pb->next) {

    pc = setFirstItem(pb->function_calls);

    for( ; pc; pc = setNextItem(pb->function_calls)) {

      if(isCALL(pc)) {
        pCode *pcn = findFunction(pic16_get_op_from_instruction(PCI(pc)));
        if(pcn && isPCF(pcn)) {
          PCF(pcn)->ncalled++;
        }
      } else
        fprintf(stderr,"BUG? pCode isn't a POC_CALL %d\n",__LINE__);

    }
  }

  //fprintf(stderr,"inlining %d\n",__LINE__);

  /* Now, Loop through the function definitions again, but this
   * time inline those functions that have only been called once. */

  InlineFunction(the_pFile->pbHead);
  //fprintf(stderr,"inlining %d\n",__LINE__);

  for(pb = the_pFile->pbHead; pb; pb = pb->next)
    unBuildFlow(pb);

}

char *pic_optype_names[]={
        "PO_NONE",         // No operand e.g. NOP
        "PO_W",              // The working register (as a destination)
        "PO_WREG",           // The working register (as a file register)
        "PO_STATUS",         // The 'STATUS' register
        "PO_BSR",            // The 'BSR' register
        "PO_FSR0",           // The "file select register" (in PIC18 family it's one
                             // of three)
        "PO_INDF0",          // The Indirect register
        "PO_INTCON",         // Interrupt Control register
        "PO_GPR_REGISTER",   // A general purpose register
        "PO_GPR_BIT",        // A bit of a general purpose register
        "PO_GPR_TEMP",       // A general purpose temporary register
        "PO_SFR_REGISTER",   // A special function register (e.g. PORTA)
        "PO_PCL",            // Program counter Low register
        "PO_PCLATH",         // Program counter Latch high register
        "PO_PCLATU",         // Program counter Latch upper register
        "PO_PRODL",          // Product Register Low
        "PO_PRODH",          // Product Register High
        "PO_LITERAL",        // A constant
        "PO_REL_ADDR",       // A relative address
        "PO_IMMEDIATE",      //  (8051 legacy)
        "PO_DIR",            // Direct memory (8051 legacy)
        "PO_CRY",            // bit memory (8051 legacy)
        "PO_BIT",            // bit operand.
        "PO_STR",            //  (8051 legacy)
        "PO_LABEL",
        "PO_WILD",           // Wild card operand in peep optimizer
        "PO_TWO_OPS"         // combine two operands
};


char *dumpPicOptype(PIC_OPTYPE type)
{
        assert( type >= 0 && type < sizeof(pic_optype_names)/sizeof( char *) );
        return (pic_optype_names[ type ]);
}


/*** BEGIN of stuff belonging to the BANKSEL optimization ***/
#include "graph.h"

#define MAX_COMMON_BANK_SIZE    32
#define FIRST_PSEUDO_BANK_NR  1000

hTab *sym2bank = NULL; // <OPERAND NAME> --> <PSEUDO BANK NR>
hTab *bank2sym = NULL; // <PSEUDO BANK NR> --> <OPERAND NAME>
hTab *coerce = NULL;   // <PSEUDO BANK NR> --> <&PSEUDOBANK>
Graph *adj = NULL;

typedef enum { INVALID_BANK = -1, UNKNOWN_BANK = -2, FIXED_BANK = -3 } pseudoBankNr;

typedef struct {
  pseudoBankNr bank;  // number assigned to this pseudoBank
  unsigned int size;  // number of operands assigned to this bank
  unsigned int ref;   // number of symbols referring to this pseudoBank (for garbage collection)
} pseudoBank;

/*----------------------------------------------------------------------*/
/* hashSymbol - hash function used to map SYMBOLs (or operands) to ints */
/*----------------------------------------------------------------------*/
unsigned int hashSymbol (const char *str)
{
  unsigned int res = 0;
  if (!str) return 0;

  while (*str) {
    res ^= (*str);
    res = (res << 4) | (res >> (8 * sizeof(unsigned int) - 4));
    str++;
  } // while

  return res;
}

/*-----------------------------------------------------------------------*/
/* compareSymbol - return 1 iff sym1 equals sym2                         */
/*-----------------------------------------------------------------------*/
int compareSymbol (const void *sym1, const void *sym2)
{
  char *s1 = (char*) sym1;
  char *s2 = (char*) sym2;

  return (strcmp (s1,s2) == 0);
}

/*-----------------------------------------------------------------------*/
/* comparePre - return 1 iff p1 == p2                                    */
/*-----------------------------------------------------------------------*/
int comparePtr (const void *p1, const void *p2)
{
  return (p1 == p2);
}

/*----------------------------------------------------------*/
/* getSymbolFromOperand - return a pointer to the symbol in */
/*                        the given operand and its length  */
/*----------------------------------------------------------*/
char *getSymbolFromOperand (char *op, int *len)
{
  char *sym, *curr;
  *len = 0;

  if (!op) return NULL;

  // we recognize two forms of operands: SYMBOL and (SYMBOL + offset)
  sym = op;
  if (*sym == '(') sym++;

  curr = sym;
  while (((*curr >= 'A') && (*curr <= 'Z'))
         || ((*curr >= 'a') && (*curr <= 'z'))
         || ((curr != sym) && (*curr >= '0') && (*curr <= '9'))
         || (*curr == '_')) {
    // find end of symbol [A-Za-z_]?[A-Za-z0-9]*
    curr++;
    (*len)++;
  } // while

  return sym;
}

/*--------------------------------------------------------------------------*/
/* getSymFromBank - get (one) name of a symbol assigned to the given bank   */
/*--------------------------------------------------------------------------*/
char *getSymFromBank (pseudoBankNr bank)
{
  assert (bank2sym);

  if (bank < 0) return "<INVALID BANK NR>";
  return hTabFindByKey (bank2sym, bank % bank2sym->size, (void *) bank, &comparePtr);
}

/*-----------------------------------------------------------------------*/
/* getPseudoBsrFromOperand - maps a string to its corresponding pseudo   */
/*                           bank number (uses hTab sym2bank), if the    */
/*                           symbol is not yet assigned a pseudo bank it */
/*                           is assigned one here                        */
/*-----------------------------------------------------------------------*/
pseudoBankNr getPseudoBankNrFromOperand (const char *op)
{
  static pseudoBankNr next_bank = FIRST_PSEUDO_BANK_NR;
  pseudoBankNr bank;
  unsigned int hash;

  assert (sym2bank);

  hash = hashSymbol (op) % sym2bank->size;
  bank = (pseudoBankNr) hTabFindByKey (sym2bank, hash, op, &compareSymbol);
  if (bank == (pseudoBankNr)NULL) bank = UNKNOWN_BANK;

  if (bank == UNKNOWN_BANK) {
    // create a pseudo bank for the operand
    bank = next_bank++;
    hTabAddItemLong (&sym2bank, hash, (char *)op, (void *)bank);
    hTabAddItemLong (&bank2sym, bank, (void *) bank, (void *)op);
    getOrAddGNode (adj, NULL, bank); // adds the node if it does not exist yet
    //fprintf (stderr, "%s:%d: adding %s with hash %u in bank %u\n", __FUNCTION__, __LINE__, op, hash, bank);
  } else {
    //fprintf (stderr, "%s:%d: found %s with hash %u in bank %u\n", __FUNCTION__, __LINE__, op, hash, bank);
  } // if

  assert (bank >= 0);

  return bank;
}

/*--------------------------------------------------------------------*/
/* isBanksel - check whether the given pCode is a BANKSEL instruction */
/*--------------------------------------------------------------------*/
int isBanksel (pCode *pc)
{
  if (!pc) return 0;

  if (isPCI(pc) && (PCI(pc)->op == POC_BANKSEL || PCI(pc)->op == POC_MOVLB)) {
    // BANKSEL <variablename>  or  MOVLB <banknr>
    //fprintf (stderr, "%s:%d: BANKSEL found: %s %s\n", __FUNCTION__, __LINE__, PCAD(pc)->directive, PCAD(pc)->arg);
    return 1;
  }

  // check for inline assembler BANKSELs
  if (isPCAD(pc) && PCAD(pc)->directive && (STRCASECMP(PCAD(pc)->directive,"BANKSEL") == 0 ||
                                            STRCASECMP(PCAD(pc)->directive,"MOVLB") == 0)) {
    //fprintf (stderr, "%s:%d: BANKSEL found: %s %s\n", __FUNCTION__, __LINE__, PCAD(pc)->directive, PCAD(pc)->arg);
    return 1;
  }

  // assume pc is no BANKSEL instruction
  return 0;
}

/*---------------------------------------------------------------------------------*/
/* invalidatesBSR - check whether the pCodeInstruction passed in modifies the BSR  */
/*                  This method can not guarantee to find all modifications of the */
/*                  BSR (e.g. via INDirection registers) but covers all compiler   */
/*                  generated plus some cases.                                     */
/*---------------------------------------------------------------------------------*/
int invalidatesBSR(pCode *pc)
{
  // assembler directives invalidate BSR (well, they might, we don't know)
  if (isPCAD(pc)) return 1;

  // only ASMDIRs and pCodeInstructions can invalidate BSR
  if (!isPCI(pc)) return 0;

  // we have a pCodeInstruction

  // check for BSR modifying instructions
  switch (PCI(pc)->op) {
  case POC_CALL:
  case POC_RCALL:
  case POC_MOVLB:
  case POC_RETFIE:  // might be used as CALL replacement
  case POC_RETLW:   // might be used as CALL replacement
  case POC_RETURN:  // might be used as CALL replacement
  case POC_BANKSEL:
    return 1;
    break;

  default:          // other instruction do not change BSR unless BSR is an explicit operand!
    // TODO: check for BSR as an explicit operand (e.g. INCF BSR,F), which should be rather unlikely...!
    break;
  } // switch

  // no change of BSR possible/probable
  return 0;
}

/*------------------------------------------------------------*/
/* getBankFromBanksel - return the pseudo bank nr assigned to */
/*                      the symbol referenced in this BANKSEL */
/*------------------------------------------------------------*/
pseudoBankNr getBankFromBanksel (pCode *pc)
{
  char *sym;
  int data = 0;

  if (!pc) return INVALID_BANK;

  if (isPCAD(pc) && PCAD(pc)->directive) {
    if (STRCASECMP(PCAD(pc)->directive,"BANKSEL") == 0) {
      // get symbolname from PCAD(pc)->arg
      //fprintf (stderr, "%s:%d: BANKSEL found: %s %s\n", __FUNCTION__, __LINE__, PCAD(pc)->directive, PCAD(pc)->arg);
      sym = PCAD(pc)->arg;
      data = getPseudoBankNrFromOperand (sym);
      //fprintf (stderr, "symbol: %s, data=%i\n", sym, data);
    } else if (STRCASECMP(PCAD(pc)->directive,"MOVLB")) {
      // get (literal) bank number from PCAD(pc)->arg
      fprintf (stderr, "%s:%d: MOVLB found: %s %s\n", __FUNCTION__, __LINE__, PCAD(pc)->directive, PCAD(pc)->arg);
      assert (0 && "not yet implemented - turn off banksel optimization for now");
    }
  } else if (isPCI(pc)) {
    if (PCI(pc)->op == POC_BANKSEL) {
      // get symbolname from PCI(pc)->pcop->name (?)
      //fprintf (stderr, "%s:%d: BANKSEL found: %s %s\n", __FUNCTION__, __LINE__, PCI(pc)->mnemonic, PCI(pc)->pcop->name);
      sym = PCI(pc)->pcop->name;
      data = getPseudoBankNrFromOperand (sym);
      //fprintf (stderr, "symbol: %s, data=%i\n", sym, data);
    } else if (PCI(pc)->op == POC_MOVLB) {
      // get (literal) bank number from PCI(pc)->pcop->name
      fprintf (stderr, "%s:%d: MOVLB found: %s %s\n", __FUNCTION__, __LINE__, PCI(pc)->mnemonic, PCI(pc)->pcop->name);
      assert (0 && "not yet implemented - turn off banksel optimization for now");
    }
  }

  if (data == 0)
    // no assigned bank could be found
    return UNKNOWN_BANK;
  else
    return data;
}

/*------------------------------------------------------------------------------*/
/* getEffectiveBank - resolves the currently assigned effective pseudo bank nr  */
/*------------------------------------------------------------------------------*/
pseudoBankNr getEffectiveBank (pseudoBankNr bank)
{
  pseudoBank *data;

  if (bank < FIRST_PSEUDO_BANK_NR) return bank;

  do {
    //fprintf (stderr, "%s:%d: bank=%d\n", __FUNCTION__, __LINE__, bank);
    data = (pseudoBank *) hTabFindByKey (coerce, bank % coerce->size, (void *) bank, &comparePtr);
    if (data) {
      if (data->bank != bank)
        bank = data->bank;
      else
        data = NULL;
    }
  } while (data);

  //fprintf (stderr, "%s:%d: effective bank=%d\n", __FUNCTION__, __LINE__, bank);
  return bank;
}

/*------------------------------------------------------------------*/
/* attachBsrInfo2pBlock - create a look-up table as to which pseudo */
/*                        bank is selected at a given pCode         */
/*------------------------------------------------------------------*/

/* Create a graph with pseudo banks as its nodes and switches between
 * these as edges (with the edge weight representing the absolute
 * number of BANKSELs from one to the other).
 * Removes redundand BANKSELs instead iff mod == 1.
 * BANKSELs update the pseudo BSR, labels invalidate the current BSR
 * value (setting it to 0=UNNKOWN), (R)CALLs also invalidate the
 * pseudo BSR.
 * TODO: check ALL instructions operands if they modify BSR directly...
 *
 * pb - the pBlock to annotate
 * mod  - select either graph creation (0) or BANKSEL removal (1)
 */
unsigned int attachBsrInfo2pBlock (pBlock *pb, int mod)
{
  pCode *pc, *pc_next;
  unsigned int prevBSR = UNKNOWN_BANK, pseudoBSR = UNKNOWN_BANK;
  int isBankselect = 0;
  unsigned int banksels=0;

  if (!pb) return 0;

  pc = pic16_findNextInstruction(pb->pcHead);
  while (pc) {
    isBankselect = isBanksel (pc);
    pc_next = pic16_findNextInstruction (pc->next);

    if (!hasNoLabel (pc)) {
      // we don't know our predecessors -- assume different BSRs
      prevBSR = UNKNOWN_BANK;
      pseudoBSR = UNKNOWN_BANK;
      //fprintf (stderr, "invalidated by label at "); pc->print (stderr, pc);
    } // if

    // check if this is a BANKSEL instruction
    if (isBankselect) {
      pseudoBSR = getEffectiveBank (getBankFromBanksel(pc));
      //fprintf (stderr, "BANKSEL via "); pc->print (stderr, pc);
      if (mod) {
        if (prevBSR == pseudoBSR && pseudoBSR >= 0) {
          //fprintf (stderr, "removing redundant "); pc->print (stderr, pc);
          if (1 || pic16_pcode_verbose) pic16_pCodeInsertAfter (pc->prev, pic16_newpCodeCharP("removed redundant BANKSEL"));
          pic16_unlinkpCode (pc);
          banksels++;
        }
      } else {
        addGEdge2 (getOrAddGNode (adj, NULL, prevBSR), getOrAddGNode (adj, NULL, pseudoBSR), 1, 0);
        banksels++;
      }
    } // if

    if (!isBankselect && invalidatesBSR(pc)) {
      // check if this instruction invalidates the pseudoBSR
      pseudoBSR = UNKNOWN_BANK;
      //fprintf (stderr, "invalidated via "); pc->print (stderr, pc);
    } // if

    prevBSR = pseudoBSR;
    pc = pc_next;
  } // while

  return banksels;
}

/*------------------------------------------------------------------------------------*/
/* assignToSameBank - returns 0 on success or an error code                           */
/*  1 - common bank would be too large                                                */
/*  2 - assignment to fixed (absolute) bank not performed                             */
/*                                                                                    */
/* This functions assumes that unsplittable operands are already assigned to the same */
/* bank (e.g. all objects being referenced as (SYMBOL + offset) must be in the same   */
/* bank so that we can make sure the bytes are laid out sequentially in memory)       */
/* TODO: Symbols with an abslute address must be handled specially!                   */
/*------------------------------------------------------------------------------------*/
int assignToSameBank (int bank0, int bank1, int doAbs, int force)
{
  int eff0, eff1, dummy;
  pseudoBank *pbank0, *pbank1;
  hashtItem *hitem;

  eff0 = getEffectiveBank (bank0);
  eff1 = getEffectiveBank (bank1);

  //fprintf (stderr, "%s:%d: bank0=%d/%d, bank1=%d/%d, doAbs=%d\n", __FUNCTION__, __LINE__, bank0, eff0, bank1, eff1, doAbs);

  // nothing to do if already same bank
  if (eff0 == eff1) return 0;

  if (!doAbs && (eff0 < FIRST_PSEUDO_BANK_NR || eff1 < FIRST_PSEUDO_BANK_NR))
    return 2;

  // ensure eff0 < eff1
  if (eff0 > eff1) {
    // swap eff0 and eff1
    dummy = eff0;
    eff0 = eff1;
    eff1 = dummy;
    dummy = bank0;
    bank0 = bank1;
    bank1 = dummy;
  } // if

  // now assign bank eff1 to bank eff0
  pbank0 = (pseudoBank *) hTabFindByKey (coerce, eff0 % coerce->size, (void *)((char*)0+eff0), &comparePtr);
  if (!pbank0) {
    pbank0 = Safe_calloc (1, sizeof (pseudoBank));
    pbank0->bank = eff0;
    pbank0->size = 1;
    pbank0->ref = 1;
    hTabAddItemLong (&coerce, eff0 % coerce->size, (void *)((char*)0+eff0), (void *) pbank0);
  } // if

  pbank1 = NULL;
  hitem = hTabSearch (coerce, eff1 % coerce->size);
  while (hitem && hitem->pkey != (void *)((char*)0+eff1))
    hitem = hitem->next;

  if (hitem) pbank1 = (pseudoBank *) hitem->item;

#if 0
  fprintf (stderr, "bank #%d/%d & bank #%d/%d --> bank #%d: %u (%s & %s)\n", bank0, eff0, bank1, eff1,
           pbank0->bank, pbank0->size,
           getSymFromBank (eff0), getSymFromBank (eff1));
#endif

  if (pbank1) {
    if (!force && (pbank0->size + pbank1->size > MAX_COMMON_BANK_SIZE)) {
#if 0
      fprintf (stderr, "bank #%d: %u, bank #%d: %u --> bank #%d': %u > %u (%s,%s)\n",
               pbank0->bank, pbank0->size, pbank1->bank, pbank1->size,
               pbank0->bank, pbank0->size + pbank1->size, MAX_COMMON_BANK_SIZE,
               getSymFromBank (pbank0->bank), getSymFromBank (pbank1->bank));
#endif
      return 1;
    } // if
    pbank0->size += pbank1->size;
    pbank1->ref--;
    if (pbank1->ref == 0) Safe_free (pbank1);
  } else {
    pbank0->size++;
  } // if

  if (hitem)
    hitem->item = pbank0;
  else
    hTabAddItemLong (&coerce, eff1 % coerce->size, (void *)((char*)0+eff1), (void *) pbank0);
  pbank0->ref++;

  //fprintf (stderr, "%s:%d: leaving.\n", __FUNCTION__, __LINE__);

  return 0;
}

/*----------------------------------------------------------------*/
/* mergeGraphNodes - combines two nodes into one and modifies all */
/*                   edges to and from the nodes accordingly      */
/* This method needs complete backedges, i.e. if (A,B) is an edge */
/* then also (B,A) must be an edge (possibly with weight 0).      */
/*----------------------------------------------------------------*/
void mergeGraphNodes (GraphNode *node1, GraphNode *node2)
{
  GraphEdge *edge, *backedge, *nextedge;
  GraphNode *node;
  int backweight;

  assert (node1 && node2);
  assert (node1 != node2);

  // add all edges starting at node2 to node1
  edge = node2->edge;
  while (edge) {
    nextedge = edge->next;
    node = edge->node;
    backedge = getGEdge (node, node2);
    if (backedge)
      backweight = backedge->weight;
    else
      backweight = 0;
    // insert edges (node1,node) and (node,node1)
    addGEdge2 (node1, node, edge->weight, backweight);
    // remove edges (node, node2) and (node2, node)
    remGEdge (node2, node);
    remGEdge (node, node2);
    edge = nextedge;
  } // while

  // now node2 should not be referenced by any other GraphNode...
  //remGNode (adj, node2->data, node2->hash);
}

/*----------------------------------------------------------------*/
/* showGraph - dump the current BANKSEL graph as a node/edge list */
/*----------------------------------------------------------------*/
void showGraph (Graph *g)
{
  GraphNode *node;
  GraphEdge *edge;
  pseudoBankNr bankNr;
  pseudoBank *pbank;
  unsigned int size;

  node = g->node;
  while (node) {
    edge = node->edge;
    bankNr = getEffectiveBank (node->hash);
    assert (bankNr >= 0);
    pbank = (pseudoBank *) hTabFindByKey (coerce, bankNr % coerce->size, (void *) bankNr, &comparePtr);
    if (pbank) {
      bankNr = pbank->bank;
      size = pbank->size;
    } else {
      size = 1;
    }

    fprintf (stderr, "edges from %s (bank %u, size %u) to:\n", getSymFromBank (node->hash), bankNr, size);

    while (edge) {
      if (edge->weight > 0)
        fprintf (stderr, "  %4u x %s\n", edge->weight, getSymFromBank (edge->node->hash));
      edge = edge->next;
    } // while (edge)
    node = node->next;
  } // while (node)
}

/*---------------------------------------------------------------*/
/* pic16_OptimizeBanksel - remove redundant BANKSEL instructions */
/*---------------------------------------------------------------*/
void pic16_OptimizeBanksel ()
{
  GraphNode *node, *node1, *node1next;

#if 0
  // needed for more effective bank assignment (needs adjusted pic16_emit_usection())
  GraphEdge *edge, *backedge;
  GraphEdge *max;
  int maxWeight, weight, mergeMore, absMaxWeight;
  pseudoBankNr curr0, curr1;
#endif
  pseudoBank *pbank;
  pseudoBankNr bankNr;
  char *base_symbol0, *base_symbol1;
  int len0, len1;
  pBlock *pb;
  set *set;
  reg_info *reg;
  unsigned int bankselsTotal = 0, bankselsRemoved = 0;

  //fprintf (stderr, "%s:%s:%d: entered.\n", __FILE__, __FUNCTION__, __LINE__);

  if (!the_pFile || !the_pFile->pbHead) return;

  adj = newGraph (NULL);
  sym2bank = newHashTable ( 255 );
  bank2sym = newHashTable ( 255 );
  coerce = newHashTable ( 255 );

  // create graph of BANKSEL relationships (node = operands, edge (A,B) iff BANKSEL B follows BANKSEL A)
  for (pb = the_pFile->pbHead; pb; pb = pb->next) {
    bankselsTotal += attachBsrInfo2pBlock (pb, 0);
  } // for pb

#if 1
  // assign symbols with absolute addresses to their respective bank nrs
  set = pic16_fix_udata;
  for (reg = setFirstItem (set); reg; reg = setNextItem (set)) {
    bankNr = reg->address >> 8;
    node = getOrAddGNode (adj, NULL, bankNr);
    bankNr = (pseudoBankNr) getEffectiveBank (getPseudoBankNrFromOperand(reg->name));
    assignToSameBank (node->hash, bankNr, 1, 1);

    assert (bankNr >= 0);
    pbank = (pseudoBank *) hTabFindByKey (coerce, bankNr % coerce->size, (void *) bankNr, &comparePtr);
    if (!pbank) {
      pbank = Safe_calloc (1, sizeof (pseudoBank));
      pbank->bank = reg->address >> 8; //FIXED_BANK;
      pbank->size = 1;
      pbank->ref = 1;
      hTabAddItemLong (&coerce, bankNr % coerce->size, (void *) bankNr, pbank);
    } else {
      assert (pbank->bank == (reg->address >> 8));
      pbank->bank = reg->address >> 8; //FIXED_BANK;
      pbank->size++;
    }
    //fprintf (stderr, "ABS: %s (%d bytes) at %x in bank %u\n", reg->name, reg->size, reg->address, bankNr);
  } // for reg
#endif

#if 1
  // assign operands referring to the same symbol (which is not given an absolute address) to the same bank
  //fprintf (stderr, "assign operands with the same symbol to the same bank\n");
  node = adj->node;
  while (node) {
    if (node->hash < 0) { node = node->next; continue; }
    base_symbol0 = getSymbolFromOperand (getSymFromBank (getEffectiveBank(node->hash)), &len0);
    node1 = node->next;
    while (node1) {
      if (node1->hash < 0) { node1 = node1->next; continue; }
      node1next = node1->next;
      base_symbol1 = getSymbolFromOperand (getSymFromBank (getEffectiveBank (node1->hash)), &len1);
      if (len0 == len1 && len0 > 0 && strncmp (base_symbol0, base_symbol1, len0) == 0) {
        int res;
        // TODO: check for symbols with absolute addresses -- these might be placed across bank boundaries!
        //fprintf (stderr, "merging %s and %s\n", getSymFromBank (getEffectiveBank(node->hash)), getSymFromBank (getEffectiveBank(node1->hash)));
        if (0 != (res = assignToSameBank (node->hash, node1->hash, 0, 1))) {
          fprintf (stderr, "%s(%d) == %s(%d), res=%d\n", base_symbol0, len0, base_symbol1, len1, res);
          assert (0 && "Could not assign a symbol to a bank!");
        }
        mergeGraphNodes (node, node1);
        /*
        if (node->hash < node1->hash)
          mergeGraphNodes (node, node1);
        else
          mergeGraphNodes (node1, node); // this removes node so node->next will fail...
        */
      } // if
      node1 = node1next;
    } // while (node1)
    node = node->next;
  } // while (node)
#endif

#if 0
  // >>> THIS ALSO NEEDS AN UPDATED pic16_emit_usection() TO REFLECT THE BANK ASSIGNMENTS <<<
  // assign tightly coupled operands to the same (pseudo) bank
  //fprintf (stderr, "assign tightly coupled operands to the same bank\n");
  mergeMore = 1;
  absMaxWeight = 0;
  while (mergeMore) {
    node = adj->node;
    max = NULL;
    maxWeight = 0;
    while (node) {
      curr0 = getEffectiveBank (node->hash);
      if (curr0 < 0) { node = node->next; continue; }
      edge = node->edge;
      while (edge) {
        assert (edge->src == node);
        backedge = getGEdge (edge->node, edge->src);
        weight = edge->weight + (backedge ? backedge->weight : 0);
        curr1 = getEffectiveBank (edge->node->hash);
        if (curr1 < 0) { edge = edge->next; continue; }

        // merging is only useful if the items are not assigned to the same bank already...
        if (curr0 != curr1 && weight > maxWeight) {
          if (maxWeight > absMaxWeight) absMaxWeight = maxWeight;
          maxWeight = weight;
          max = edge;
        } // if
        edge = edge->next;
      } // while
      node = node->next;
    } // while

    if (maxWeight > 0) {
#if 0
      fprintf (stderr, "%s:%d: merging (%4u) %d(%s) and %d(%s)\n", __FUNCTION__, __LINE__, maxWeight,
               max->src->hash, getSymFromBank (max->src->hash),
               max->node->hash, getSymFromBank (max->node->hash));
#endif

      node = getGNode (adj, max->src->data, max->src->hash);
      node1 = getGNode (adj, max->node->data, max->node->hash);

      if (0 == assignToSameBank (max->src->hash, max->node->hash, 0, 0)) {
        if (max->src->hash < max->node->hash)
          mergeGraphNodes (node, node1);
        else
          mergeGraphNodes (node1, node);
      } else {
        remGEdge (node, node1);
        remGEdge (node1, node);
        //mergeMore = 0;
      }

    } else {
      mergeMore = 0;
    }
  } // while
#endif

#if 1
  // remove redundant BANKSELs
  //fprintf (stderr, "removing redundant BANKSELs\n");
  for (pb = the_pFile->pbHead; pb; pb = pb->next) {
    bankselsRemoved += attachBsrInfo2pBlock (pb, 1);
  } // for pb
#endif

#if 0
  fprintf (stderr, "display graph\n");
  showGraph ();
#endif

  deleteGraph (adj);
  //fprintf (stderr, "%s:%s:%d: leaving, %u/%u BANKSELs removed...\n", __FILE__, __FUNCTION__, __LINE__, bankselsRemoved, bankselsTotal);
}

/*** END of stuff belonging to the BANKSEL optimization ***/



/*** BEGIN of helpers for pCode dataflow optimizations ***/

typedef unsigned int symbol_t;
typedef unsigned int valnum_t;
//typedef unsigned int hash_t;

#ifndef INT_TO_PTR
#define INT_TO_PTR(x) (((char *) 0) + (x))
#endif

#ifndef PTR_TO_INT
#define PTR_TO_INT(x) (((char *)(x)) - ((char *) 0))
#endif

static int pic16_regIsLocal (reg_info *r);
static int pic16_safepCodeRemove (pCode *pc, char *comment);

/* statistics */
static unsigned int pic16_df_removed_pcodes = 0;
static unsigned int pic16_df_saved_bytes = 0;
static unsigned int df_findall_sameflow = 0;
static unsigned int df_findall_otherflow = 0;
static unsigned int df_findall_in_vals = 0;

static void pic16_df_stats () {
  return;
  if (pic16_debug_verbose || pic16_pcode_verbose) {
    fprintf (stderr, "PIC16: dataflow analysis removed %u instructions (%u bytes)\n", pic16_df_removed_pcodes, pic16_df_saved_bytes);
    fprintf (stderr, "findAll: same flow %u (%u in_vals), other flow %u\n", df_findall_sameflow, df_findall_in_vals, df_findall_otherflow);
    //pic16_df_removed_pcodes = pic16_df_saved_bytes = 0;
  }
}

/* Remove a pCode iff possible:
 * - previous pCode is no SKIP
 * - pc has no label
 * Returns 1 iff the pCode has been removed, 0 otherwise. */
static int pic16_safepCodeUnlink (pCode *pc, char *comment) {
  pCode *pcprev, *pcnext;
  char buf[256], *total=NULL;
  int len;

  if (!comment) comment = "=DF= pCode removed by pic16_safepCodeUnlink";

  pcprev = pic16_findPrevInstruction (pc->prev);
  pcnext = pic16_findNextInstruction (pc->next);

  /* move labels to next instruction (if possible) */
  if (PCI(pc)->label && !pcnext) return 0;

  /* if this is a SKIP with side-effects -- do not remove */
  /* XXX: might try to replace this one with the side-effect only version */
  if (isPCI_SKIP(pc)
        && ((PCI(pc)->outCond & (PCC_REGISTER | PCC_W)) != 0))
  {
    pCode *newpc;
    switch (PCI(pc)->op)
    {
    case POC_INCFSZ:
    case POC_INFSNZ:
      newpc = pic16_newpCode(POC_INCF, pic16_pCodeOpCopy( PCI(pc)->pcop ) );
      pic16_pCodeReplace( pc, newpc );
      return 1;
      break;
    case POC_INCFSZW:
      newpc = pic16_newpCode(POC_INCFW, pic16_pCodeOpCopy( PCI(pc)->pcop ) );
      pic16_pCodeReplace( pc, newpc );
      return 1;
      break;
    case POC_DECFSZ:
    case POC_DCFSNZ:
      newpc = pic16_newpCode(POC_INCF, pic16_pCodeOpCopy( PCI(pc)->pcop ) );
      pic16_pCodeReplace( pc, newpc );
      return 1;
      break;
    case POC_DECFSZW:
      newpc = pic16_newpCode(POC_INCF, pic16_pCodeOpCopy( PCI(pc)->pcop ) );
      pic16_pCodeReplace( pc, newpc );
      return 1;
      break;
    default:
      return 0;
    }
    return 0;
  }

  /* if previous instruction is a skip -- do not remove */
  if (pcprev && isPCI_SKIP(pcprev)) {
    if (!pic16_safepCodeUnlink (pcprev, "=DF= removed now unused SKIP")) {
      /* preceeding SKIP could not be removed -- keep this instruction! */
      return 0;
    }
  }

  if (PCI(pc)->label) {
    //fprintf (stderr, "%s: moving label(s)\n", __FUNCTION__);
    //pc->print (stderr, pc);
    PCI(pcnext)->label = pic16_pBranchAppend (PCI(pc)->label, PCI(pcnext)->label);
    PCI(pc)->label = NULL;
  }

  /* update statistics */
  pic16_df_removed_pcodes++;
  if (isPCI(pc)) pic16_df_saved_bytes += PCI(pc)->isize;

  /* remove the pCode */
  pic16_pCode2str (buf, 256, pc);
  //fprintf (stderr, "%s: removing pCode: %s\n", __FUNCTION__, buf);
  if (0 || pic16_debug_verbose || pic16_pcode_verbose) {
    len = strlen (buf) + strlen (comment) + 10;
    total = (char *) Safe_malloc (len);
    SNPRINTF (total, len, "%s: %s", comment, buf);
    pic16_pCodeInsertAfter (pc, pic16_newpCodeCharP(total));
    Safe_free (total);
  }

  /* actually unlink it from the pBlock -- also remove from to/from lists */
  pic16_pCodeUnlink (pc);

  /* remove the pCode -- release registers */
  pc->destruct (pc);

  /* report success */
  return 1;
}


/* ======================================================================== */
/* === SYMBOL HANDLING ==================================================== */
/* ======================================================================== */

static hTab *map_strToSym = NULL;               /** (char *) --> symbol_t */
static hTab *map_symToStr = NULL;               /** symbol_t -> (char *) */
static symbol_t nextSymbol = 0x2000;            /** next symbol_t assigned to the next generated symbol */

/** Calculate a hash for a given string.
 * If len == 0 the string is assumed to be NUL terminated. */
static hash_t symbolHash (const char *str, unsigned int len) {
  hash_t hash = 0;
  if (!len) {
    while (*str) {
      hash = (hash << 2) ^ *str;
      str++;
    } // while
  } else {
    while (len--) {
      hash = (hash << 2) ^ *str;
      str++;
    }
  }
  return hash;
}

/** Return 1 iff strings v1 and v2 are identical. */
static int symcmp (const void *v1, const void *v2) {
  return !strcmp ((const char *) v1, (const char *) v2);
}

/** Return 1 iff pointers v1 and v2 are identical. */
static int ptrcmp (const void *v1, const void *v2) {
  return (v1 == v2);
}

enum {  SPO_WREG=0x1000,
        SPO_STATUS,
        SPO_PRODL,
        SPO_PRODH,
        SPO_INDF0,
        SPO_POSTDEC0,
        SPO_POSTINC0,
        SPO_PREINC0,
        SPO_PLUSW0,
        SPO_INDF1,
        SPO_POSTDEC1,
        SPO_POSTINC1,
        SPO_PREINC1,
        SPO_PLUSW1,
        SPO_INDF2,
        SPO_POSTDEC2,
        SPO_POSTINC2,
        SPO_PREINC2,
        SPO_PLUSW2,
        SPO_STKPTR,
        SPO_TOSL,
        SPO_TOSH,
        SPO_TOSU,
        SPO_BSR,
        SPO_FSR0L,
        SPO_FSR0H,
        SPO_FSR1L,
        SPO_FSR1H,
        SPO_FSR2L,
        SPO_FSR2H,
        SPO_PCL,
        SPO_PCLATH,
        SPO_PCLATU,
        SPO_TABLAT,
        SPO_TBLPTRL,
        SPO_TBLPTRH,
        SPO_TBLPTRU,
        SPO_LAST
};

/* Return the unique symbol_t for the given string. */
static symbol_t symFromStr (const char *str) {
  hash_t hash;
  char *res;
  symbol_t sym;

  if (!map_symToStr) {
    int i;
    struct { char *name; symbol_t sym; } predefsyms[] = {
        {"WREG", SPO_WREG},
        {"STATUS", SPO_STATUS},
        {"PRODL", SPO_PRODL},
        {"PRODH", SPO_PRODH},
        {"INDF0", SPO_INDF0},
        {"POSTDEC0", SPO_POSTDEC0},
        {"POSTINC0", SPO_POSTINC0},
        {"PREINC0", SPO_PREINC0},
        {"PLUSW0", SPO_PLUSW0},
        {"INDF1", SPO_INDF1},
        {"POSTDEC1", SPO_POSTDEC1},
        {"POSTINC1", SPO_POSTINC1},
        {"PREINC1", SPO_PREINC1},
        {"PLUSW1", SPO_PLUSW1},
        {"INDF2", SPO_INDF2},
        {"POSTDEC2", SPO_POSTDEC2},
        {"POSTINC2", SPO_POSTINC2},
        {"PREINC2", SPO_PREINC2},
        {"PLUSW2", SPO_PLUSW2},
        {"STKPTR", SPO_STKPTR},
        {"TOSL", SPO_TOSL},
        {"TOSH", SPO_TOSH},
        {"TOSU", SPO_TOSU},
        {"BSR", SPO_BSR},
        {"FSR0L", SPO_FSR0L},
        {"FSR0H", SPO_FSR0H},
        {"FSR1L", SPO_FSR1L},
        {"FSR1H", SPO_FSR1H},
        {"FSR2L", SPO_FSR2L},
        {"FSR2H", SPO_FSR2H},
        {"PCL", SPO_PCL},
        {"PCLATH", SPO_PCLATH},
        {"PCLATU", SPO_PCLATU},
        {"TABLAT", SPO_TABLAT},
        {"TBLPTRL", SPO_TBLPTRL},
        {"TBLPTRH", SPO_TBLPTRH},
        {"TBLPTRU", SPO_TBLPTRU},
        {NULL, 0}
    };

    map_strToSym = newHashTable (128);
    map_symToStr = newHashTable (128);

    for (i=0; predefsyms[i].name; i++) {
      char *name;

      /* enter new symbol */
      sym = predefsyms[i].sym;
      name = predefsyms[i].name;
      res = Safe_strdup (name);
      hash = symbolHash (name, 0);

      hTabAddItemLong (&map_strToSym, hash, res, INT_TO_PTR(sym));
      hTabAddItemLong (&map_symToStr, sym % map_symToStr->size, INT_TO_PTR(sym), res);
    } // for i
  }

  hash = symbolHash (str, 0) % map_strToSym->size;

  /* find symbol in table */
  sym = PTR_TO_INT(hTabFindByKey (map_strToSym, hash, str, &symcmp));
  if (sym) {
    //fprintf (stderr, "found symbol %x for %s\n", sym, str);
    return sym;
  }

  /* enter new symbol */
  sym = nextSymbol++;
  res = Safe_strdup (str);

  hTabAddItemLong (&map_strToSym, hash, res, INT_TO_PTR(sym));
  hTabAddItemLong (&map_symToStr, sym % map_symToStr->size, INT_TO_PTR(sym), res);

  //fprintf (stderr, "created symbol %x for %s\n", sym, res);

  return sym;
}

#if 1
static const char *strFromSym (symbol_t sym) {
  return (const char *) hTabFindByKey (map_symToStr, sym % map_symToStr->size, INT_TO_PTR(sym), &ptrcmp);
}
#endif

/* ======================================================================== */
/* === DEFINITION MAP HANDLING ============================================ */
/* ======================================================================== */

/* A defmap provides information about which symbol is defined by which pCode.
 * The most recent definitions are prepended to the list, so that the most
 * recent definition can be found by forward scanning the list.
 * pc2: MOVFF r0x00, r0x01
 * pc1: INCF r0x01
 * head --> ("r0x01",pc1,42) --> ("STATUS",pc1,44) --> ("r0x01",pc2,28) --> NULL
 *
 * We attach one defmap to each flow object, and each pCode will occur at
 * least once in its flow's defmap (maybe defining the 0 symbol). This can be
 * used to find definitions for a pCode in its own defmap that precede pCode.
 */

typedef struct defmap_s {
  symbol_t sym;                 /** symbol this item refers to */
  union {
    struct {
      unsigned int in_mask:8;   /** mask leaving in accessed bits */
      unsigned int mask:8;      /** mask leaving in modified bits (if isWrite) */
      int isRead:1;             /** sym/mask is read */
      int isWrite:1;            /** sym/mask is written */
    } access;
    int accessmethod;
  } acc;
  pCode *pc;                    /** pCode this symbol is refrenced at */
  valnum_t in_val;              /** valnum_t of symbol's previous value (the one read at pc) */
  valnum_t val;                 /** new unique number for this value (if isWrite) */
  struct defmap_s *prev, *next; /** link to previous an next definition */
} defmap_t;

static defmap_t *defmap_free = NULL;            /** list of unused defmaps */
static int defmap_free_count = 0;               /** number of released defmap items */

/* Returns a defmap_t with the specified data; this will be the new list head.
 * next - pointer to the current list head */
static defmap_t *newDefmap (symbol_t sym, int in_mask, int mask, int isRead, int isWrite, pCode *pc, valnum_t val, defmap_t *next) {
  defmap_t *map;

  if (defmap_free) {
    map = defmap_free;
    defmap_free = map->next;
    --defmap_free_count;
  } else {
    map = (defmap_t *) Safe_calloc (1, sizeof (defmap_t));
  }
  map->sym = sym;
  map->acc.access.in_mask = (isRead ? (in_mask ? in_mask : 0xFF) : 0x00);
  map->acc.access.mask = (isWrite ? (mask ? mask : 0xFF) : 0x00);
  map->acc.access.isRead = (isRead != 0);
  map->acc.access.isWrite = (isWrite != 0);
  map->pc = pc;
  map->in_val = 0;
  map->val = (isWrite ? val : 0);
  map->prev = NULL;
  map->next = next;
  if (next) next->prev = map;

  return map;
}

/* Returns a copy of the single defmap item. */
static defmap_t *copyDefmap (defmap_t *map) {
  defmap_t *res = (defmap_t *) Safe_malloc (sizeof (defmap_t));
  memcpy (res, map, sizeof (defmap_t));
  res->next = NULL;
  res->prev = NULL;
  return res;
}

/* Insert a defmap item after the specified one. */
static int defmapInsertAfter (defmap_t *ref, defmap_t *newItem) {
  if (!ref || !newItem) return 1;

  newItem->next = ref->next;
  newItem->prev = ref;
  ref->next = newItem;
  if (newItem->next) newItem->next->prev = newItem;

  return 0;
}

/* Check whether item (or an identical one) is already in the chain and add it if neccessary.
 * item is copied before insertion into chain and therefore left untouched.
 * Returns 1 iff the item has been inserted into the list, 0 otherwise. */
static int defmapAddCopyIfNew (defmap_t **head, defmap_t *item) {
  defmap_t *dummy;
  dummy = *head;
  while (dummy && (dummy->sym != item->sym
                          || dummy->pc != item->pc
                          || dummy->acc.accessmethod != item->acc.accessmethod
                          || dummy->val != item->val
                          || dummy->in_val != item->in_val)) {
    dummy = dummy->next;
  } // while

  /* item already present? */
  if (dummy) return 0;

  /* otherwise: insert copy of item */
  dummy = copyDefmap (item);
  dummy->next = *head;
  if (*head) (*head)->prev = dummy;
  *head = dummy;

  return 1;
}

/* Releases a defmap. This also removes the map from its chain -- update the head manually! */
static void deleteDefmap (defmap_t *map) {
  if (!map) return;

  /* unlink from chain -- fails for the first item (head is not updated!) */
  if (map->next) map->next->prev = map->prev;
  if (map->prev) map->prev->next = map->next;

  /* clear map */
  memset (map, 0, sizeof (defmap_t));

  /* save for future use */
  map->next = defmap_free;
  defmap_free = map;
  ++defmap_free_count;
}

/* Release all defmaps referenced from map. */
static void deleteDefmapChain (defmap_t **_map) {
  defmap_t *map, *next;

  if (!_map) return;

  map = *_map;

  /* find list head */
  while (map && map->prev) map = map->prev;

  /* delete all items */
  while (map) {
    next = map->next;
    deleteDefmap (map);
    map = next;
  } // while

  *_map = NULL;
}

/* Free all defmap items. */
static void freeDefmap (defmap_t **_map) {
  defmap_t *next;
  defmap_t *map;

  if (!_map) return;

  map = (*_map);

  /* find list head */
  while (map->prev) map = map->prev;

  /* release all items */
  while (map) {
    next = map->next;
    Safe_free (map);
    map = next;
  }

  (*_map) = NULL;
}

/* Returns the most recent definition for the given symbol preceeding pc.
 * If no definition is found, NULL is returned.
 * If pc == NULL the whole list is scanned. */
static defmap_t *defmapFindDef (defmap_t *map, symbol_t sym, pCode *pc) {
  defmap_t *curr = map;

  if (pc) {
    /* skip all definitions up to pc */
    while (curr && (curr->pc != pc)) curr = curr->next;

    /* pc not in the list -- scan the whole list for definitions */
    if (!curr) {
      fprintf (stderr, "pc %p not found in defmap -- scanning whole list for symbol '%s'\n", pc, strFromSym (sym));
      curr = map;
    } else {
      /* skip all definitions performed by pc */
      while (curr && (curr->pc == pc)) curr = curr->next;
    }
  } // if (pc)

  /* find definition for sym */
  while (curr && (!curr->acc.access.isWrite || (curr->sym != sym))) {
    curr = curr->next;
  }

  return curr;
}

#if 0
/* Returns the first use (read) of the given symbol AFTER pc.
 * If no such use is found, NULL is returned.
 * If pc == NULL the whole list is scanned. */
static defmap_t *defmapFindUse (defmap_t *map, symbol_t sym, pCode *pc) {
  defmap_t *curr = map, *prev = NULL;

  if (pc) {
    /* skip all definitions up to pc */
    while (curr && (curr->pc != pc)) { prev = curr; curr = curr->next; }

    /* pc not in the list -- scan the whole list for definitions */
    if (!curr) {
      //fprintf (stderr, "pc %p not found in defmap -- scanning whole list for symbol '%s'\n", pc, strFromSym (sym));
      curr = prev;
    }
  } else {
    /* find end of list */
    while (curr && curr->next) curr = curr->next;
  } // if (pc)

  /* find use of sym (scan list backwards) */
  while (curr && (!curr->acc.access.isRead || (curr->sym != sym))) curr = curr->prev;

  return curr;
}
#endif

/* Return the defmap entry for sym AT pc.
 * If none is found, NULL is returned.
 * If more than one entry is found an assertion is triggered. */
static defmap_t *defmapCurr (defmap_t *map, symbol_t sym, pCode *pc) {
  defmap_t *res = NULL;

  /* find entries for pc */
  while (map && map->pc != pc) map = map->next;

  /* find first entry for sym @ pc */
  while (map && map->pc == pc && map->sym != sym) map = map->next;

  /* no entry found */
  if (!map) return NULL;

  /* check for more entries */
  res = map;
  map = map->next;
  while (map && map->pc == pc) {
    /* more than one entry for sym @ pc found? */
    assert (map->sym != sym);
    map = map->next;
  }

  /* return single entry for sym @ pc */
  return res;
}

/* Modifies the definition of sym at pCode to newval.
 * Returns 0 on success, 1 if no definition of sym in pc has been found.
 */
static int defmapUpdate (defmap_t *map, symbol_t sym, pCode *pc, valnum_t newval) {
  defmap_t *m  = map;

  /* find definitions of pc */
  while (m && m->pc != pc) m = m->next;

  /* find definition of sym at pc */
  while (m && m->pc == pc && (!m->acc.access.isWrite || (m->sym != sym))) m = m->next;

  /* no definition found */
  if (!m) return 1;

  /* redefine */
  m->val = newval;

  /* update following uses of sym */
  while (m && m->pc == pc) m = m->prev;
  while (m) {
    if (m->sym == sym) {
      m->in_val = newval;
      if (m->acc.access.isWrite) m = NULL;
    } // if
    if (m) m = m->prev;
  } // while

  return 0;
}

/* ======================================================================== */
/* === STACK ROUTINES ===================================================== */
/* ======================================================================== */

typedef struct stack_s {
  void *data;
  struct stack_s *next;
} stackitem_t;

typedef stackitem_t *dynstack_t;
static stackitem_t *free_stackitems = NULL;

/* Create a stack with one item. */
static dynstack_t *newStack () {
  dynstack_t *s = (dynstack_t *) Safe_malloc (sizeof (dynstack_t));
  *s = NULL;
  return s;
}

/* Remove a stack -- its items are only marked free. */
static void deleteStack (dynstack_t *s) {
  stackitem_t *i;

  while (*s) {
    i = *s;
    *s = (*s)->next;
    i->next = free_stackitems;
    free_stackitems = i;
  } // while
  Safe_free (s);
}

/* Release all stackitems. */
static void releaseStack () {
  stackitem_t *i;

  while (free_stackitems) {
    i = free_stackitems->next;
    Safe_free(free_stackitems);
    free_stackitems = i;
  } // while
}

static void stackPush (dynstack_t *stack, void *data) {
  stackitem_t *i;

  if (free_stackitems) {
    i = free_stackitems;
    free_stackitems = free_stackitems->next;
  } else {
    i = (stackitem_t *) Safe_calloc (1, sizeof (stackitem_t));
  }
  i->data = data;
  i->next = *stack;
  *stack = i;
}

static void *stackPop (dynstack_t *stack) {
  void *data;
  stackitem_t *i;

  if (stack && *stack) {
    data = (*stack)->data;
    i = *stack;
    *stack = (*stack)->next;
    i->next = free_stackitems;
    free_stackitems = i;
    return data;
  } else {
    return NULL;
  }
}

#if 0
static int stackContains (dynstack_t *s, void *data) {
  stackitem_t *i;
  if (!s) return 0;
  i = *s;
  while (i) {
    if (i->data == data) return 1;
    i = i->next;
  } // while

  /* not found */
  return 0;
}
#endif

static int stackIsEmpty (dynstack_t *s) {
  return (*s == NULL);
}


typedef struct {
  pCodeFlow *flow;
  defmap_t *lastdef;
} state_t;

static state_t *newState (pCodeFlow *flow, defmap_t *lastdef) {
  state_t *s = (state_t *) Safe_calloc (1, sizeof (state_t));
  s->flow = flow;
  s->lastdef = lastdef;
  return s;
}

static void deleteState (state_t *s) {
  Safe_free (s);
}

static int stateIsNew (state_t *state, dynstack_t *todo, dynstack_t *done) {
  stackitem_t *i;

  /* scan working list for state */
  if (todo) {
    i = *todo;
    while (i) {
      /* is i == state? -- state not new */
      if ((((state_t *) (i->data))->flow == state->flow) && (((state_t *) (i->data))->lastdef == state->lastdef)) return 0;
      i = i->next;
    } // while
  }

  if (done) {
    i = *done;
    while (i) {
      /* is i == state? -- state not new */
      if ((((state_t *) (i->data))->flow == state->flow) && (((state_t *) (i->data))->lastdef == state->lastdef)) return 0;
      i = i->next;
    } // while
  }

  /* not found -- state is new */
  return 1;
}

static inline valnum_t newValnum ();

const char *pic16_pBlockGetFunctionName (pBlock *pb) {
  pCode *pc;

  if (!pb) return "<unknown function>";

  pc = pic16_findNextpCode (pb->pcHead, PC_FUNCTION);
  if (pc && isPCF(pc)) return PCF(pc)->fname;
  else return "<unknown function>";
}

static defmap_t *pic16_pBlockAddInval (pBlock *pb, symbol_t sym) {
  defmap_t *map;
  pCodeFlow *pcfl;

  assert(pb);

  pcfl = PCI(pic16_findNextInstruction (pb->pcHead))->pcflow;

  /* find initial value (assigning pc == NULL) */
  map = PCFL(pcfl)->in_vals;
  while (map && map->sym != sym) map = map->next;

  /* initial value already present? */
  if (map) {
    //fprintf (stderr, "found init value for sym %s (%x): %u\n", strFromSym(sym), sym, map->val);
    return map;
  }

  /* create a new initial value */
  map = newDefmap (sym, 0x00, 0xff, 0, 1, NULL, newValnum(), PCFL(pcfl)->in_vals);
  PCFL(pcfl)->in_vals = map;
  //fprintf (stderr, "Created init value for sym %s (%x): %u\n", strFromSym(sym), sym, map->val);
  return map;

#if 0
  /* insert map as last item in pcfl's defmap */
  if (!prev) prev = PCFL(pcfl)->defmap;
  if (!prev) {
    PCFL(pcfl)->defmap = map;
  } else {
    while (prev->next) prev = prev->next;
    prev->next = map;
    map->prev = prev;
  }

  return map;
#endif
}

/* Find all reaching definitions for sym at pc.
 * A new (!) list of definitions is returned.
 * Returns the number of reaching definitions found.
 * The defining defmap entries are returned in *chain.
 */
static int defmapFindAll (symbol_t sym, pCode *pc, defmap_t **chain) {
  defmap_t *map;
  defmap_t *res;

  pCodeFlow *curr;
  pCodeFlowLink *succ;
  state_t *state;
  dynstack_t *todo;     /** stack of state_t */
  dynstack_t *done;     /** stack of state_t */

  int n_defs;

  assert (pc && isPCI(pc) && PCI(pc)->pcflow);
  assert (chain);

  /* initialize return list */
  *chain = NULL;

  /* wildcard symbol? */
  if (!sym) return 0;

  //fprintf (stderr, "Searching definition of sym %s(%x) @ pc %p(%p)\n", strFromSym(sym), sym, pc, pc->pb);

  map = PCI(pc)->pcflow->defmap;

  res = defmapFindDef (map, sym, pc);
  //if (res) fprintf (stderr, "found def in own flow @ pc %p\n", res->pc);

#define USE_PRECALCED_INVALS 1
#if USE_PRECALCED_INVALS
  if (!res && PCI(pc)->pcflow->in_vals) {
    res = defmapFindDef (PCI(pc)->pcflow->in_vals, sym, NULL);
    if (res) {
      //fprintf  (stderr, "found def in init values\n");
      df_findall_in_vals++;
    }
  }
#endif

  if (res) {
    // found a single definition (in pc's flow)
    //fprintf (stderr, "unique definition for %s @ %p found @ %p (val: %x)\n", strFromSym(sym), pc, res->pc, res->val);
    defmapAddCopyIfNew (chain, res);
    df_findall_sameflow++;
    return 1;
  }

#if USE_PRECALCED_INVALS
  else {
    defmapAddCopyIfNew (chain, pic16_pBlockAddInval (pc->pb, sym));
    return 1;
  }

#endif

#define FORWARD_FLOW_ANALYSIS 1
#if defined FORWARD_FLOW_ANALYSIS && FORWARD_FLOW_ANALYSIS
  /* no definition found in pc's flow preceeding pc */
  todo = newStack ();
  done = newStack ();
  n_defs = 0;
  stackPush (todo, newState (PCI(pic16_findNextInstruction(pc->pb->pcHead))->pcflow, res));

  while (!stackIsEmpty (todo)) {
    state = (state_t *) stackPop (todo);
    stackPush (done, state);
    curr = state->flow;
    res = state->lastdef;
    //fprintf (stderr, "searching def of sym %s in pcFlow %p (lastdef %x @ %p)\n", strFromSym(sym), curr, res ? res->val : 0, res ? res->pc : NULL);

    /* there are no definitions BEFORE pc in pc's flow (see above) */
    if (curr == PCI(pc)->pcflow) {
      if (!res) {
        //fprintf (stderr, "symbol %s(%x) might be used uninitialized at %p\n", strFromSym(sym), sym, pc);
        res = pic16_pBlockAddInval (pc->pb, sym);
        if (defmapAddCopyIfNew (chain, res)) n_defs++;
        res = NULL;
      } else {
        //fprintf (stderr, "reaching definition for %s @ %p found @ %p (val: %x)\n", strFromSym(sym), pc, res->pc, res->val);
        if (defmapAddCopyIfNew (chain, res)) n_defs++;
      }
    }

    /* save last definition of sym in this flow as initial def in successors */
    res = defmapFindDef (curr->defmap, sym, NULL);
    if (!res) res = state->lastdef;

    /* add successors to working list */
    state = newState (NULL, NULL);
    succ = (pCodeFlowLink *) setFirstItem (curr->to);
    while (succ) {
      //fprintf (stderr, "  %p --> %p with %x\n", curr, succ->pcflow, res ? res->val : 0);
      state->flow = succ->pcflow;
      state->lastdef = res;
      if (stateIsNew (state, todo, done)) {
        stackPush (todo, state);
        state = newState (NULL, NULL);
      } // if
      succ = (pCodeFlowLink *) setNextItem (curr->to);
    } // while
    deleteState (state);
  } // while

#else // !FORWARD_FLOW_ANALYSIS

    {
      int firstState = 1;

      /* no definition found in pc's flow preceeding pc */
      todo = newStack ();
      done = newStack ();
      n_defs = 0;
      stackPush (todo, newState (PCI(pc)->pcflow, res));

      while (!stackIsEmpty (todo)) {
          state = (state_t *) stackPop (todo);
          curr = state->flow;

          if (firstState) {
              firstState = 0;
              /* only check predecessor flows */
          } else {
              /* get (last) definition of sym in this flow */
              res = defmapFindDef (curr->defmap, sym, NULL);
          }

          if (res) {
              /* definition found */
              //fprintf (stderr, "reaching definition for %s @ %p found @ %p (val: %x)\n", strFromSym(sym), pc, res->pc, res->val);
              if (defmapAddCopyIfNew (chain, res)) n_defs++;
          } else {
              /* no definition found -- check predecessor flows */
              state = newState (NULL, NULL);
              succ = (pCodeFlowLink *) setFirstItem (curr->from);

              /* if no flow predecessor available -- sym might be uninitialized */
              if (!succ) {
                  //fprintf (stder, "sym %s might be used uninitialized at %p\n", strFromSym (sym), pc);
                  res = newDefmap (sym, 0xff, 0, 1, NULL, 0, *chain);
                  if (defmapAddCopyIfNew (chain, res)) n_defs++;
                  deleteDefmap (res); res = NULL;
              }

              while (succ) {
                  //fprintf (stderr, "  %p --> %p with %x\n", curr, succ->pcflow, res ? res->val : 0);
                  state->flow = succ->pcflow;
                  state->lastdef = res;
                  if (stateIsNew (state, todo, done)) {
                      stackPush (todo, state);
                      state = newState (NULL, NULL);
                  } // if
                  succ = (pCodeFlowLink *) setNextItem (curr->from);
              } // while
              deleteState (state);
          }
      } // while
    }

#endif

  /* clean up done stack */
  while (!stackIsEmpty(done)) {
    deleteState ((state_t *) stackPop (done));
  } // while
  deleteStack (done);

  /* return number of items in result set */
  if (n_defs == 0) {
    //fprintf (stderr, "sym %s might be used uninitialized at %p\n", strFromSym (sym), pc);
  } else if (n_defs == 1) {
    assert (*chain);
    //fprintf (stderr, "sym %s at %p always defined as %x @ %p\n", strFromSym(sym), pc, (*chain)->val, (*chain)->pc);
  } else if (n_defs > 0) {
    //fprintf (stderr, "%u definitions for sym %s at %p found:\n", n_defs, strFromSym(sym), pc);
#if 0
    res = *chain;
    while (res) {
      fprintf (stderr, "  as %4x @ %p\n", res->val, res->pc);
      res = res->next;
    } // while
#endif
  }
  //fprintf (stderr, "%u definitions for sym %s at %p found\n", n_defs, strFromSym(sym), pc);
  df_findall_otherflow++;
  return n_defs;
}

/* ======================================================================== */
/* === VALUE NUMBER HANDLING ============================================== */
/* ======================================================================== */

static valnum_t nextValnum = 0x1000;
static hTab *map_symToValnum = NULL;

/** Return a new value number. */
static inline valnum_t newValnum () {
  return (nextValnum += 4);
}

static valnum_t valnumFromStr (const char *str) {
  symbol_t sym;
  valnum_t val;
  void *res;

  sym = symFromStr (str);

  if (!map_symToValnum) {
    map_symToValnum = newHashTable (128);
  } // if

  /* literal already known? */
  res = hTabFindByKey (map_symToValnum, sym % map_symToValnum->size, INT_TO_PTR(sym), &ptrcmp);

  /* return existing valnum */
  if (res) return (valnum_t) PTR_TO_INT(res);

  /* create new valnum */
  val = newValnum();
  hTabAddItemLong (&map_symToValnum, sym % map_symToValnum->size, INT_TO_PTR(sym), INT_TO_PTR(val));
  //fprintf (stderr, "NEW VALNUM %x for symbol %s\n", val, str);
  return val;
}

/* Create a valnum for a literal. */
static valnum_t valnumFromLit (unsigned int lit) {
  return ((valnum_t) 0x100 + (lit & 0x0FF));
}

/* Return the (positive) literal value represented by val
 * or -1 iff val is no known literal's valnum. */
static int litFromValnum (valnum_t val) {
  if (val >= 0x100 && val < 0x200) {
    /* valnum is a (known) literal */
    return val & 0x00FF;
  } else {
    /* valnum is not a known literal */
    return -1;
  }
}

#if 0
/* Sanity check - all flows in a block must be reachable from initial flow. */
static int verifyAllFlowsReachable (pBlock *pb) {
  set *reached;
  set *flowInBlock;
  set *checked;
  pCode *pc;
  pCodeFlow *pcfl;
  pCodeFlowLink *succ;
  int res;

  //fprintf (stderr, "%s - started for %s.\n" ,__FUNCTION__, pic16_pBlockGetFunctionName (pb));

  reached = NULL;
  flowInBlock = NULL;
  checked = NULL;
  /* mark initial flow as reached (and "not needs to be reached") */
  pc = pic16_findNextpCode (pb->pcHead, PC_FLOW);
  assert (pc);
  addSetHead (&reached, pc);
  addSetHead (&checked, pc);

  /* mark all further flows in block as "need to be reached" */
  pc = pb->pcHead;
  do {
    if (isPCI(pc)) addSetIfnotP (&flowInBlock, PCI(pc)->pcflow);
    pc = pic16_findNextInstruction (pc->next);
  } while (pc);

  while (reached && (pcfl = (pCodeFlow *)indexSet (reached, 0)) != NULL) {
    /* mark as reached and "not need to be reached" */
    deleteSetItem (&reached, pcfl);
    //fprintf (stderr, "%s - checking %p\n" ,__FUNCTION__, pcfl);

    /* flow is no longer considered unreachable */
    deleteSetItem (&flowInBlock, pcfl);

    for (succ = setFirstItem (pcfl->to); succ; succ = setNextItem (pcfl->to)) {
      if (!isinSet (checked, succ->pcflow)) {
        /* flow has never been reached before */
        addSetHead (&reached, succ->pcflow);
        addSetHead (&checked, succ->pcflow);
      } // if
    } // for succ
  } // while

  //fprintf (stderr, "%s - finished\n", __FUNCTION__);

  /* by now every flow should have been reached
   * --> flowInBlock should be empty */
  res = (flowInBlock == NULL);

#if 1
  if (flowInBlock) {
          fprintf (stderr, "not all flows reached in %s:\n", pic16_pBlockGetFunctionName (pb));
    while (flowInBlock) {
      pcfl = indexSet (flowInBlock, 0);
      fprintf (stderr, "not reached: flow %p\n", pcfl);
      deleteSetItem (&flowInBlock, pcfl);
    } // while
  }
#endif

  /* clean up */
  deleteSet (&reached);
  deleteSet (&flowInBlock);
  deleteSet (&checked);

  /* if we reached every flow, succ is NULL by now... */
  //assert (res); // will fire on unreachable code...
  return (res);
}
#endif

/* Checks a flow for accesses to sym AFTER pc.
 *
 * Returns -1 if the symbol is read in this flow (before redefinition),
 * returns 0 if the symbol is redefined in this flow or
 * returns a mask [0x01 -- 0xFF] indicating the bits still alive after this flow.
 */
int pic16_isAliveInFlow (symbol_t sym, int mask, pCodeFlow *pcfl, pCode *pc) {
  defmap_t *map, *mappc;

  /* find pc or start of definitions */
  map = pcfl->defmap;
  while (map && (map->pc != pc) && map->next) map = map->next;
  /* if we found pc -- ignore it */
  while (map && map->pc == pc) map = map->prev;

  /* scan list backwards (first definition first) */
  while (map && mask) {
//    if (map->sym == sym) {
      //fprintf (stderr, "%s: accessing sym %s in pc %p/map %p\n", __FUNCTION__, strFromSym(sym), map->pc, map);
      mappc = map;
      /* scan list for reads at this pc first */
      while (map && map->pc == mappc->pc) {
        /* is the symbol (partially) read? */
        if ((map->sym == sym) && (map->acc.access.isRead && ((map->acc.access.in_mask & mask) != 0))) {
          //if (sym != SPO_STATUS) fprintf (stderr, "%s: symbol %s read at pc %p\n", __FUNCTION__, strFromSym (sym), map->pc);
          return -1;
        }
        map = map->prev;
      } // while
      map = mappc;

      while (map && map->pc == mappc->pc) {
        /* honor (partial) redefinitions of sym */
        if ((map->sym == sym) && (map->acc.access.isWrite)) {
          mask &= ~map->acc.access.mask;
          //if (sym != SPO_STATUS) fprintf (stderr, "%s: symbol %s redefined at pc %p, alive mask: %x\n", __FUNCTION__, strFromSym (sym), map->pc, mask);
        }
        map = map->prev;
      } // while
//    } // if
    /* map already points to the first defmap for the next pCode */
    //map = mappc->prev;
  } // while

  /* the symbol is not completely redefined in this flow and not accessed -- symbol
   * is still alive; return the appropriate mask of alive bits */
  return mask;
}

/* Check whether a symbol is alive (AFTER pc). */
static int pic16_isAlive (symbol_t sym, pCode *pc) {
  int mask, visit;
  dynstack_t *todo, *done;
  state_t *state;
  pCodeFlow *pcfl;
  pCodeFlowLink *succ;

  mask = 0x00ff;

  assert (isPCI(pc));
  pcfl = PCI(pc)->pcflow;

  todo = newStack ();
  done = newStack ();

  state = newState (pcfl, (defmap_t *) INT_TO_PTR(mask));
  stackPush (todo, state);
  visit = 0;

  while (!stackIsEmpty (todo)) {
    state = (state_t *) stackPop (todo);
    pcfl = state->flow;
    mask = PTR_TO_INT(state->lastdef);
    if (visit) stackPush (done, state); else deleteState(state);
    //fprintf (stderr, "%s: checking flow %p for symbol %s (%x)/%x\n", __FUNCTION__, pcfl, strFromSym(sym), sym, mask);
    // make sure flows like A(i1,i2,pc,i3,...) --> A with pc reading and writing sym are handled correctly!
    mask = pic16_isAliveInFlow (sym, mask, pcfl, visit == 0 ? pc : NULL);
    visit++;

    /* symbol is redefined in flow before use -- not alive in this flow (maybe in others?) */
    if (mask == 0) continue;

    /* symbol is (partially) read before redefinition in flow */
    if (mask == -1) break;

    /* symbol is neither read nor completely redefined -- check successor flows */
    for (succ = setFirstItem(pcfl->to); succ; succ = setNextItem (pcfl->to)) {
      state = newState (succ->pcflow, (defmap_t *) INT_TO_PTR(mask));
      if (stateIsNew (state, todo, done)) {
        stackPush (todo, state);
      } else {
        deleteState (state);
      }
    } // for
  } // while

  while (!stackIsEmpty (todo)) deleteState ((state_t *) stackPop (todo));
  while (!stackIsEmpty (done)) deleteState ((state_t *) stackPop (done));

  /* symbol is read in at least one flow -- is alive */
  if (mask == -1) return 1;

  /* symbol is read in no flow */
  return 0;
}

/* Returns whether access to the given symbol has side effects. */
static int pic16_symIsSpecial (symbol_t sym) {
  //fprintf (stderr, "%s: sym=%x\n", __FUNCTION__, sym);
  switch (sym) {
  case SPO_INDF0:
  case SPO_PLUSW0:
  case SPO_POSTINC0:
  case SPO_POSTDEC0:
  case SPO_PREINC0:
  case SPO_INDF1:
  case SPO_PLUSW1:
  case SPO_POSTINC1:
  case SPO_POSTDEC1:
  case SPO_PREINC1:
  case SPO_INDF2:
  case SPO_PLUSW2:
  case SPO_POSTINC2:
  case SPO_POSTDEC2:
  case SPO_PREINC2:
  case SPO_PCL:
          return 1;
  default:
          /* no special effects known */
          return 0;
  } // switch

  return 0;
}

/* Check whether a register should be considered local (to the current function) or not. */
static int pic16_regIsLocal (reg_info *r) {
  symbol_t sym;
  if (r) {
    if (r->type == REG_TMP) return 1;

    sym = symFromStr (r->name);
    switch (sym) {
    case SPO_WREG:
    case SPO_FSR0L: // used in ptrget/ptrput
    case SPO_FSR0H: // ... as well
    case SPO_FSR1L: // used as stack pointer... (so not really local but shared among function calls)
    case SPO_FSR1H: // ... as well
    case SPO_FSR2L: // used as frame pointer
    case SPO_FSR2H: // ... as well
    case SPO_PRODL: // used to return values from functions
    case SPO_PRODH: // ... as well
      /* these registers (and some more...) are considered local */
      return 1;
      break;
    default:
      /* for unknown regs: check is marked local, leave if not */
      if (r->isLocal) {
        return 1;
      } else {
        //fprintf (stderr, "%s: non-local reg used: %s\n", __FUNCTION__, r->name);
        return 0;
      }
    } // switch
  } // if

  /* if in doubt, assume non-local... */
  return 0;
}

/* Check all symbols touched by pc whether their newly assigned values are read.
 * Returns 0 if no symbol is used later on, 1 otherwise. */
static int pic16_pCodeIsAlive (pCode *pc) {
  pCodeInstruction *pci;
  defmap_t *map;
  reg_info *checkreg;

  /* we can only handle PCIs */
  if (!isPCI(pc)) return 1;

  //pc->print (stderr, pc);

  pci = PCI(pc);
  assert (pci && pci->pcflow && pci->pcflow->defmap);

  /* NEVER remove instructions with implicit side effects */
  switch (pci->op) {
  case POC_TBLRD:
  case POC_TBLRD_POSTINC:       /* modify TBLPTRx */
  case POC_TBLRD_POSTDEC:
  case POC_TBLRD_PREINC:
  case POC_TBLWT:               /* modify program memory */
  case POC_TBLWT_POSTINC:       /* modify TBLPTRx */
  case POC_TBLWT_POSTDEC:
  case POC_TBLWT_PREINC:
  case POC_CLRWDT:              /* clear watchdog timer */
  case POC_PUSH:                /* should be safe to remove though... */
  case POC_POP:                 /* should be safe to remove though... */
  case POC_CALL:
  case POC_RCALL:
  case POC_RETFIE:
  case POC_RETURN:
    //fprintf (stderr, "%s: instruction with implicit side effects not removed: %s\n", __FUNCTION__, pci->mnemonic);
    return 1;

  default:
    /* no special instruction */
    break;
  } // switch

  /* prevent us from removing assignments to non-local variables */
  checkreg = NULL;
  if (PCI(pc)->outCond & PCC_REGISTER) checkreg = pic16_getRegFromInstruction (pc);
  else if (PCI(pc)->outCond & PCC_REGISTER2) checkreg =  pic16_getRegFromInstruction2(pc);

  if ((PCI(pc)->outCond & (PCC_REGISTER | PCC_REGISTER2)) && !checkreg) {
    /* assignment to DIRECT operand like "BSF (_global + 1),6" */
    //fprintf (stderr, "%s: assignment to register detected, but register not available!\n", __FUNCTION__);
    //pc->print (stderr, pc);
    return 1;
  }
  if ((PCI(pc)->outCond & (PCC_REGISTER | PCC_REGISTER2)) && !pic16_regIsLocal (checkreg)) {
    //fprintf (stderr, "%s: dest-reg not local %s\n", __FUNCTION__, checkreg ? checkreg->name : "<unknown>");
    return 1;
  }

#if 1
  /* OVERKILL: prevent us from removing reads from non-local variables
   * THIS IS HERE TO AVOID PROBLEMS WITH VOLATILE OPERANDS ONLY!
   * Once registers get a "isVolatile" field this might be handled more efficiently... */
  checkreg = NULL;
  if (PCI(pc)->inCond & PCC_REGISTER) checkreg = pic16_getRegFromInstruction (pc);
  else if (PCI(pc)->inCond & PCC_REGISTER2) checkreg =  pic16_getRegFromInstruction2(pc);

  if ((PCI(pc)->inCond & (PCC_REGISTER | PCC_REGISTER2)) && !checkreg) {
    /* read from DIRECT operand like "BTFSS (_global + 1),6" -- might be volatile */
    //fprintf (stderr, "%s: read from register detected, but register not available!\n", __FUNCTION__);
    //pc->print (stderr, pc);
    return 1;
  }
  if ((PCI(pc)->inCond & (PCC_REGISTER | PCC_REGISTER2)) && !pic16_regIsLocal (checkreg)) {
    //fprintf (stderr, "%s: src-reg not local: %s\n", __FUNCTION__, checkreg ? checkreg->name : "<unknown>");
    return 1;
  }
#endif

  /* now check that the defined symbols are not used */
  map = pci->pcflow->defmap;

  /* find items for pc */
  while (map && map->pc != pc) map = map->next;

  /* no entries found? something is fishy with DF analysis... -- play safe */
  if (!map) {
    if (pic16_pcode_verbose) {
      fprintf (stderr, "%s: defmap not found\n", __FUNCTION__);
    }
    return 1;
  }

  /* check all symbols being modified by pc */
  while (map && map->pc == pc) {
    if (map->sym == 0) { map = map->next; continue; }

    /* keep pc if it references special symbols (like POSTDEC0) */
#if 0
    {
      char buf[256];
      pic16_pCode2str (buf, 256, pc);
      fprintf (stderr, "%s: checking for sym %x(%s) at pc %p (%s)\n", __FUNCTION__, map->sym, strFromSym (map->sym), pc, buf);
    }
#endif
    if (pic16_symIsSpecial (map->sym)) {
      //fprintf (stderr, "%s: special sym\n", __FUNCTION__);
      return 1;
    }
    if (map->acc.access.isWrite) {
      if (pic16_isAlive (map->sym, pc)) {
        //fprintf (stderr, "%s(%s): pCode is alive (sym %s still used)\n", __FUNCTION__, pic16_pBlockGetFunctionName (pc->pb),strFromSym (map->sym));
        return 1;
      }
    }
    map = map->next;
  } // while

  /* no use for any of the pc-assigned symbols found -- pCode is dead and can be removed */
#if 0
  {
    char buf[256];
    pic16_pCode2str (buf, 256, pc);
    fprintf (stderr, "%s: pCode %p (%s) is dead.\n", __FUNCTION__, pc, buf);
  }
#endif
  return 0;
}

/* Adds implied operands to the list.
 * sym - operand being accessed in the pCode
 * list - list to append the operand
 * isRead - set to 1 iff sym is read in pCode
 * listRead - set to 1 iff all operands being read are to be listed
 *
 * Returns 0 for "normal" operands, 1 for special operands.
 */
static int fixupSpecialOperands (symbol_t sym, int in_mask, int mask, pCode *pc, valnum_t val, defmap_t **list, int isRead, int isWrite) {
  /* check whether accessing REG accesses other REGs as well */
  switch (sym) {
  case SPO_INDF0:
    /* reads FSR0x */
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR0L, 0xff, 0xff, 1, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR0H, 0xff, 0xff, 1, 0, pc, 0, *list);
    break;

  case SPO_PLUSW0:
    /* reads FSR0x and WREG */
    *list = newDefmap (SPO_WREG, 0xff, 0x00, 1, 0, pc, 0, *list);
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR0L, 0xff, 0xff, 1, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR0H, 0xff, 0xff, 1, 0, pc, 0, *list);
    break;

  case SPO_POSTDEC0:
  case SPO_POSTINC0:
  case SPO_PREINC0:
    /* reads/modifies FSR0x */
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR0L, 0xff, 0xff, 1, 1, pc, newValnum (), *list);
    *list = newDefmap (SPO_FSR0H, 0xff, 0xff, 1, 1, pc, newValnum (), *list);
    break;

  case SPO_INDF1:
    /* reads FSR1x */
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR1L, 0xff, 0xff, 1, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR1H, 0xff, 0xff, 1, 0, pc, 0, *list);
    break;

  case SPO_PLUSW1:
    /* reads FSR1x and WREG */
    *list = newDefmap (SPO_WREG, 0xff, 0x00, 1, 0, pc, 0, *list);
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR1L, 0xff, 0xff, 1, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR1H, 0xff, 0xff, 1, 0, pc, 0, *list);
    break;

  case SPO_POSTDEC1:
  case SPO_POSTINC1:
  case SPO_PREINC1:
    /* reads/modifies FSR1x */
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR1L, 0xff, 0xff, 1, 1, pc, newValnum (), *list);
    *list = newDefmap (SPO_FSR1H, 0xff, 0xff, 1, 1, pc, newValnum (), *list);
    break;

  case SPO_INDF2:
    /* reads FSR2x */
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR2L, 0xff, 0xff, 1, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR2H, 0xff, 0xff, 1, 0, pc, 0, *list);
    break;

  case SPO_PLUSW2:
    /* reads FSR2x and WREG */
    *list = newDefmap (SPO_WREG, 0xff, 0x00, 1, 0, pc, 0, *list);
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR2L, 0xff, 0xff, 1, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR2H, 0xff, 0xff, 1, 0, pc, 0, *list);
    break;

  case SPO_POSTDEC2:
  case SPO_POSTINC2:
  case SPO_PREINC2:
    /* reads/modifies FSR2x */
    *list = newDefmap (sym, 0xff, 0xff, 0, 0, pc, 0, *list);
    *list = newDefmap (SPO_FSR2L, 0xff, 0xff, 1, 1, pc, newValnum (), *list);
    *list = newDefmap (SPO_FSR2H, 0xff, 0xff, 1, 1, pc, newValnum (), *list);
    break;

  case SPO_PCL:
    /* modifies PCLATH and PCLATU */
    *list = newDefmap (SPO_PCL, 0xff, 0xff, isRead, isWrite, pc, newValnum (), *list);
    if (isRead) {
      /* reading PCL updates PCLATx */
      *list = newDefmap (SPO_PCLATH, 0xff, 0xff, 0, 1, pc, newValnum (), *list);
      *list = newDefmap (SPO_PCLATU, 0xff, 0xff, 0, 1, pc, newValnum (), *list);
    }
    if (isWrite) {
      /* writing PCL implicitly reads PCLATx (computed GOTO) */
      *list = newDefmap (SPO_PCLATH, 0xff, 0xff, 1, 0, pc, 0, *list);
      *list = newDefmap (SPO_PCLATU, 0xff, 0xff, 1, 0, pc, 0, *list);
    }
    break;

  default:
    *list = newDefmap (sym, in_mask, mask, isRead, isWrite, pc, val, *list);
    /* nothing special */
    return 0;
    break;
  }

  /* has been a special operand */
  return 1;
}

static symbol_t pic16_fsrsym_idx[][2] = {
    {SPO_FSR0L, SPO_FSR0H},
    {SPO_FSR1L, SPO_FSR1H},
    {SPO_FSR2L, SPO_FSR2H}
};

/* Merge multiple defmap entries for the same symbol for list's pCode. */
static void mergeDefmapSymbols (defmap_t *list) {
  defmap_t *ref, *curr, *temp;

  /* now make sure that each symbol occurs at most once per pc */
  ref = list;
  while (ref && (ref->pc == list->pc)) {
    curr = ref->next;
    while (curr && (curr->pc == list->pc)) {
      if (curr->sym == ref->sym) {
        //fprintf (stderr, "Merging defmap entries for symbol %s\n", strFromSym (ref->sym));
        /* found a symbol occuring twice... merge the two */
        if (curr->acc.access.isRead) {
          //if (ref->acc.access.isRead) fprintf (stderr, "symbol %s was marked twice as read at pc %p\n", strFromSym (ref->sym), ref->pc);
          ref->acc.access.isRead = 1;
          ref->acc.access.in_mask |= curr->acc.access.in_mask;
        }
        if (curr->acc.access.isWrite) {
          //if (ref->acc.access.isWrite) fprintf (stderr, "symbol %s was marked twice as written at pc %p\n", strFromSym (ref->sym), ref->pc);
          ref->acc.access.isWrite = 1;
          ref->acc.access.mask |= curr->acc.access.mask;
        }
        temp = curr;
        curr = curr->next;
        deleteDefmap (temp);
        continue; // do not skip curr!
      } // if
      curr = curr->next;
    } // while
    ref = ref->next;
  } // while
}

/** Prepend list with the reads and definitions performed by pc. */
static defmap_t *createDefmap (pCode *pc, defmap_t *list) {
  pCodeInstruction *pci;
  int cond, inCond, outCond;
  int mask = 0xff, smask;
  symbol_t sym, sym2;
  char *name;

  if (isPCAD(pc)) {
    /* make sure there is at least one entry for each pc (needed by list traversal routines) */
    /* TODO: mark this defmap node as an ASMDIR -- any values might be read/modified */
    fprintf (stderr, "ASMDIRs not supported by data flow analysis!\n");
    list = newDefmap (0, 0xff, 0xff, 0, 0, pc, 0, list);
    return list;
  }
  assert (isPCI(pc));
  pci = PCI(pc);

  /* handle bit instructions */
  if (pci->isBitInst) {
    assert (pci->pcop->type == PO_GPR_BIT);
    mask = 1U << (PCORB(PCI(pc)->pcop)->bit);
  }

  /* handle (additional) implicit arguments */
  switch (pci->op) {
  case POC_LFSR:
    {
      int lit;
      valnum_t val;
      lit = PCOL(pci->pcop)->lit;
      assert (lit >= 0 && lit < 3);
      //fprintf (stderr, "LFSR: %s // %s\n", pci->pcop->name, pic16_get_op(((pCodeOpLit2 *)(pci->pcop))->arg2, NULL, 0));
      val = valnumFromStr (pic16_get_op(((pCodeOpLit2 *)(pci->pcop))->arg2, NULL, 0));
      //fprintf (stderr, "LFSR lit=%u, symval=%4x\n", lit, val);
      list = newDefmap (pic16_fsrsym_idx[lit][0], 0x00, 0xff, 0, 1, pc, val, list);
      list = newDefmap (pic16_fsrsym_idx[lit][1], 0x00, 0xff, 0, 1, pc, val+1, list); // val+1 is guaranteed not be used as a valnum...
    }
    break;

  case POC_MOVLB: // BSR
  case POC_BANKSEL: // BSR
    list = newDefmap (SPO_BSR, 0x00, 0xff, 0, 1, pc, valnumFromStr (pic16_get_op (((pCodeOpLit2 *)(pci->pcop))->arg2, NULL, 0)), list);
    break;

  case POC_MULWF: // PRODx
  case POC_MULLW: // PRODx
    list = newDefmap (SPO_PRODH, 0x00, 0xff, 0, 1, pc, newValnum (), list);
    list = newDefmap (SPO_PRODL, 0x00, 0xff, 0, 1, pc, newValnum (), list);
    break;

  case POC_POP: // TOS, STKPTR
    list = newDefmap (SPO_STKPTR, 0xff, 0xff, 1, 1, pc, newValnum (), list);
    list = newDefmap (SPO_TOSL, 0x00, 0xff, 0, 1, pc, newValnum (), list);
    list = newDefmap (SPO_TOSH, 0x00, 0xff, 0, 1, pc, newValnum (), list);
    list = newDefmap (SPO_TOSU, 0x00, 0xff, 0, 1, pc, newValnum (), list);
    break;

  case POC_PUSH: // STKPTR
    list = newDefmap (SPO_STKPTR, 0xff, 0xff, 1, 1, pc, newValnum (), list);
    list = newDefmap (SPO_TOSL, 0xff, 0xff, 0, 1, pc, newValnum (), list);
    list = newDefmap (SPO_TOSH, 0xff, 0xff, 0, 1, pc, newValnum (), list);
    list = newDefmap (SPO_TOSU, 0xff, 0xff, 0, 1, pc, newValnum (), list);
    break;

  case POC_CALL: // return values (and arguments?): WREG, PRODx, FSR0L
  case POC_RCALL: // return values (and arguments?): WREG, PRODx, FSR0L
    list = newDefmap (SPO_WREG, 0xff, 0xff, 1, 1, pc, newValnum (), list);
    list = newDefmap (SPO_PRODL, 0xff, 0xff, 1, 1, pc, newValnum (), list);
    list = newDefmap (SPO_PRODH, 0xff, 0xff, 1, 1, pc, newValnum (), list);
    list = newDefmap (SPO_FSR0L, 0xff, 0xff, 1, 1, pc, newValnum (), list);

    /* needs correctly set-up stack pointer */
    list = newDefmap (SPO_FSR1L, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR1H, 0xff, 0x00, 1, 0, pc, 0, list);
    break;

  case POC_RETLW: // return values: WREG, PRODx, FSR0L
    /* pseudo read on (possible) return values */
    // WREG is handled below via outCond
    list = newDefmap (SPO_PRODL, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_PRODH, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR0L, 0xff, 0x00, 1, 0, pc, 0, list);

    /* caller's stack pointers must be restored */
    list = newDefmap (SPO_FSR1L, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR1H, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR2L, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR2H, 0xff, 0x00, 1, 0, pc, 0, list);
    break;

  case POC_RETURN: // return values; WREG, PRODx, FSR0L
  case POC_RETFIE: // return value: WREG, PRODx, FSR0L
    /* pseudo read on (possible) return values */
    list = newDefmap (SPO_WREG, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_PRODL, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_PRODH, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR0L, 0xff, 0x00, 1, 0, pc, 0, list);

    /* caller's stack pointers must be restored */
    list = newDefmap (SPO_FSR1L, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR1H, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR2L, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_FSR2H, 0xff, 0x00, 1, 0, pc, 0, list);
    break;

  case POC_TBLRD:
    list = newDefmap (SPO_TBLPTRL, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_TBLPTRH, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_TBLPTRU, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_TABLAT, 0x00, 0xff, 0, 1, pc, newValnum(), list);
    break;

  case POC_TBLRD_POSTINC:
  case POC_TBLRD_POSTDEC:
  case POC_TBLRD_PREINC:
    list = newDefmap (SPO_TBLPTRL, 0xff, 0xff, 1, 1, pc, newValnum(), list);
    list = newDefmap (SPO_TBLPTRH, 0xff, 0xff, 1, 1, pc, newValnum(), list);
    list = newDefmap (SPO_TBLPTRU, 0xff, 0xff, 1, 1, pc, newValnum(), list);
    list = newDefmap (SPO_TABLAT, 0x00, 0xff, 0, 1, pc, newValnum(), list);
    break;

  case POC_TBLWT:
    list = newDefmap (SPO_TBLPTRL, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_TBLPTRH, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_TBLPTRU, 0xff, 0x00, 1, 0, pc, 0, list);
    list = newDefmap (SPO_TABLAT, 0xff, 0x00, 1, 0, pc, 0, list);
    break;

  case POC_TBLWT_POSTINC:
  case POC_TBLWT_POSTDEC:
  case POC_TBLWT_PREINC:
    list = newDefmap (SPO_TBLPTRL, 0xff, 0xff, 1, 1, pc, newValnum(), list);
    list = newDefmap (SPO_TBLPTRH, 0xff, 0xff, 1, 1, pc, newValnum(), list);
    list = newDefmap (SPO_TBLPTRU, 0xff, 0xff, 1, 1, pc, newValnum(), list);
    list = newDefmap (SPO_TABLAT, 0xff, 0x00, 1, 0, pc, 0, list);
    break;

  default:
    /* many instruction implicitly read BSR... -- THIS IS IGNORED! */
    break;
  } // switch

  /* handle explicit arguments */
  inCond = pci->inCond;
  outCond = pci->outCond;
  cond = inCond | outCond;
  if (cond & PCC_W) {
    list = newDefmap (symFromStr ("WREG"), mask, mask, inCond & PCC_W, outCond & PCC_W, pc, newValnum (), list);
  } // if

  /* keep STATUS read BEFORE STATUS write in the list (still neccessary?) */
  if (inCond & PCC_STATUS) {
    smask = 0;
    if (inCond & PCC_C) smask |= 1U << PIC_C_BIT;
    if (inCond & PCC_DC) smask |= 1U << PIC_DC_BIT;
    if (inCond & PCC_Z) smask |= 1U << PIC_Z_BIT;
    if (inCond & PCC_OV) smask |= 1U << PIC_OV_BIT;
    if (inCond & PCC_N) smask |= 1U << PIC_N_BIT;

    list = newDefmap (symFromStr ("STATUS"), smask, 0x00, 1, 0, pc, 0, list);
    //fprintf (stderr, "pc %p: def STATUS & %02x\n", pc, smask);
  } // if

  if (outCond & PCC_STATUS) {
    smask = 0;
    if (outCond & PCC_C) smask |= 1U << PIC_C_BIT;
    if (outCond & PCC_DC) smask |= 1U << PIC_DC_BIT;
    if (outCond & PCC_Z) smask |= 1U << PIC_Z_BIT;
    if (outCond & PCC_OV) smask |= 1U << PIC_OV_BIT;
    if (outCond & PCC_N) smask |= 1U << PIC_N_BIT;

    list = newDefmap (symFromStr ("STATUS"), 0x00, smask, 0, 1, pc, newValnum (), list);
    //fprintf (stderr, "pc %p: def STATUS & %02x\n", pc, smask);
  } // if

  sym = sym2 = 0;
  if (cond & PCC_REGISTER) {
    name = pic16_get_op (pci->pcop, NULL, 0);
    sym = symFromStr (name);
    fixupSpecialOperands (sym, mask, mask, pc, newValnum(), &list, inCond & PCC_REGISTER, outCond & PCC_REGISTER);
    //fprintf (stderr, "pc %p: def REG %s(%x) & %02x\n", pc, name, sym, mask);
  }

  if (cond & PCC_REGISTER2) {
    name = pic16_get_op2 (pci->pcop, NULL, 0);
    sym2 = symFromStr (name);
    fixupSpecialOperands (sym2, mask, mask, pc, newValnum(), &list, inCond & PCC_REGISTER2, outCond & PCC_REGISTER2);
    //fprintf (stderr, "pc %p: def REG2 %s(%x) & %02x\n", pc, name, sym2, mask);
  }


  /* make sure there is at least one entry for each pc (needed by list traversal routines) */
  list = newDefmap (0, 0x00, 0x00, 0, 0, pc, 0, list);

  mergeDefmapSymbols (list);

  return list;
}

#if 0
static void printDefmap (defmap_t *map) {
  defmap_t *curr;

  curr = map;
  fprintf (stderr, "defmap @ %p:\n", curr);
  while (curr) {
    fprintf (stderr, "%s%s: %4x|%4x / %02x|%02x, sym %s(%x) @ pc %p\n",
                    curr->acc.access.isRead ? "R" : " ",
                    curr->acc.access.isWrite ? "W": " ",
                    curr->in_val, curr->val,
                    curr->acc.access.in_mask, curr->acc.access.mask,
                    strFromSym(curr->sym), curr->sym,
                    curr->pc);
    curr = curr->next;
  } // while
  fprintf (stderr, "<EOL>\n");
}
#endif

/* Add "additional" definitions to uniq.
 * This can be used to merge the in_values and the flow's defmap to create an in_value-list for the flow's successors.
 * This can also be used to create a uniq (out)list from a flow's defmap by passing *uniq==NULL.
 *
 * If symbols defined in additional are not present in uniq, a definition is created.
 * Otherwise the present definition is altered to reflect the newer assignments.
 *
 * flow: <uniq> --> assign1 --> assign2 --> assign3 --> ... --> <uniq'>
 *       before     `------- noted in additional --------'      after
 *
 * I assume that each symbol occurs AT MOST ONCE in uniq.
 *
 */
static int defmapUpdateUniqueSym (defmap_t **uniq, defmap_t *additional) {
  defmap_t *curr;
  defmap_t *old;
  int change = 0;

  //fprintf (stderr, "%s: merging %p & %p\n", __FUNCTION__, *uniq, additional);
  /* find tail of additional list (holds the first assignment) */
  curr = additional;
  while (curr && curr->next) curr = curr->next;

  /* update uniq */
  do {
    /* find next assignment in additionals */
    while (curr && !curr->acc.access.isWrite) curr = curr->prev;

    if (!curr) break;

    /* find item in uniq */
    old = *uniq;
    //printDefmap (*uniq);
    while (old && (old->sym != curr->sym)) old = old->next;

    if (old) {
      /* definition found -- replace */
      if (old->val != curr->val) {
        old->val = curr->val;
        change++;
      } // if
    } else {
      /* new definition */
      *uniq = newDefmap (curr->sym, 0x00, 0xff, 0, 1, NULL, curr->val, *uniq);
      change++;
    }

    curr = curr->prev;
  } while (1);

  /* return 0 iff uniq remained unchanged */
  return change;
}

/* Creates the in_value list of a flow by (iteratively) merging the out_value
 * lists of its predecessor flows.
 * Initially *combined should be NULL, alt_in will be copied to combined.
 * If *combined != NULL, combined will be altered:
 * - for symbols defined in *combined but not in alt_in,
 *   *combined is altered to 0 (value unknown, either *combined or INIT).
 * - for symbols defined in alt_in but not in *combined,
 *   a 0 definition is created (value unknown, either INIT or alt).
 * - for symbols defined in both, *combined is:
 *   > left unchanged if *combined->val == alt_in->val or
 *   > modified to 0 otherwise (value unknown, either alt or *combined).
 *
 * I assume that each symbol occurs AT MOST ONCE in each list!
 */
static int defmapCombineFlows (defmap_t **combined, defmap_t *alt_in, pBlock *pb) {
  defmap_t *curr;
  defmap_t *old;
  int change = 0;
  valnum_t val;

  //fprintf (stderr, "%s: merging %p & %p\n", __FUNCTION__, *combined, alt_in);

  if (!(*combined)) {
    return defmapUpdateUniqueSym (combined, alt_in);
  } // if

  /* merge the two */
  curr = alt_in;
  while (curr) {
    /* find symbols definition in *combined */
    old = *combined;
    while (old && (old->sym != curr->sym)) old = old->next;

    if (old) {
      /* definition found */
      if (old->val && (old->val != curr->val)) {
        old->val = 0; /* value unknown */
        change++;
      }
    } else {
      /* no definition found -- can be either INIT or alt_in's value */
      val = pic16_pBlockAddInval (pb, curr->sym)->val;
      *combined = newDefmap (curr->sym, 0x00, 0xff, 0, 1, NULL, (val == curr->val) ? val : 0, *combined);
      if (val != curr->val) change++;
    }

    curr = curr->next;
  } // while (curr)

  /* update symbols from *combined that are NOT defined in alt_in -- can be either *combined's value or INIT */
  old = *combined;
  while (old) {
    if (old->val != 0) {
      /* find definition in alt_in */
      curr = alt_in;
      while (curr && curr->sym != old->sym) curr = curr->next;
      if (!curr) {
        /* symbol defined in *combined only -- can be either INIT or *combined */
        val = pic16_pBlockAddInval (pb, old->sym)->val;
        if (old->val != val) {
          old->val = 0;
          change++;
        }
      } // if
    } // if

    old = old->next;
  } // while

  return change;
}

static int defmapCompareUnique (defmap_t *map1, defmap_t *map2) {
  defmap_t *curr1, *curr2;
  symbol_t sym;

  /* identical maps are equal */
  if (map1 == map2) return 0;

  if (!map1) return -1;
  if (!map2) return 1;

  //fprintf (stderr, "%s: comparing %p & %p\n", __FUNCTION__, map1, map2);

  /* check length */
  curr1 = map1;
  curr2 = map2;
  while (curr1 && curr2) {
    curr1 = curr1->next;
    curr2 = curr2->next;
  } // while

  /* one of them longer? */
  if (curr1) return 1;
  if (curr2) return -1;

  /* both lists are of equal length -- compare (in O(n^2)) */
  curr1 = map1;
  while (curr1) {
    sym = curr1->sym;
    curr2 = map2;
    while (curr2 && curr2->sym != sym) curr2 = curr2->next;
    if (!curr2) return 1; // symbol not found in curr2
    if (curr2->val != curr1->val) return 1; // values differ

    /* compare next symbol */
    curr1 = curr1->next;
  } // while

  /* no difference found */
  return 0;
}


/* Prepare a list of all reaching definitions per flow.
 * This is done using a forward dataflow analysis.
 */
static void createReachingDefinitions (pBlock *pb) {
  defmap_t *out_vals, *in_vals;
  pCode *pc;
  pCodeFlow *pcfl;
  pCodeFlowLink *link;
  set *todo;
  set *blacklist;

  if (!pb) return;

  /* initialize out_vals to unique'fied defmaps per pCodeFlow */
  for (pc = pic16_findNextInstruction (pb->pcHead); pc; pc = pic16_findNextInstruction (pc->next)) {
    if (isPCFL(pc)) {
      deleteDefmapChain (&PCFL(pc)->in_vals);
      deleteDefmapChain (&PCFL(pc)->out_vals);
      defmapUpdateUniqueSym (&PCFL(pc)->out_vals, PCFL(pc)->defmap);
    } // if
  } // for

  pc = pic16_findNextInstruction (pb->pcHead);
  if (!pc) {
      // empty function, avoid NULL pointer dereference
      return;
  } // if
  todo = NULL; blacklist = NULL;
  addSetHead (&todo, PCI(pc)->pcflow);

  //fprintf (stderr, "%s: function %s()\n", __FUNCTION__, pic16_pBlockGetFunctionName (pb));
  while (elementsInSet (todo)) {
    //fprintf (stderr, "%u items in todo-set\n", elementsInSet (todo));
    pcfl = PCFL(indexSet (todo, 0));
    deleteSetItem (&todo, pcfl);
    //fprintf (stderr, "%s: checking %p\n", __FUNCTION__, pcfl);
    in_vals = NULL;
    out_vals = NULL;

    if (isinSet (blacklist, pcfl)) {
            fprintf (stderr, "ignoring blacklisted flow\n");
      continue;
    }

    /* create in_vals from predecessors out_vals */
    link = setFirstItem (pcfl->from);
    while (link) {
      defmapCombineFlows (&in_vals, link->pcflow->out_vals, pb);
      link = setNextItem (pcfl->from);
    } // while

    //printDefmap (in_vals);
    //printDefmap (pcfl->in_vals);

    if (!pcfl->in_vals || !pcfl->out_vals || defmapCompareUnique (in_vals, pcfl->in_vals)) {
      //fprintf (stderr, "in_vals changed\n");
      /* in_vals changed -- update out_vals */
      deleteDefmapChain (&pcfl->in_vals);
      pcfl->in_vals = in_vals;

      /* create out_val from in_val and defmap */
      out_vals = NULL;
      defmapUpdateUniqueSym (&out_vals, in_vals);
      defmapUpdateUniqueSym (&out_vals, pcfl->defmap);

      /* is out_vals different from pcfl->out_vals */
      if (!pcfl->out_vals || defmapCompareUnique (out_vals, pcfl->out_vals)) {
        //fprintf (stderr, "out_vals changed\n");
        deleteDefmapChain (&pcfl->out_vals);
        pcfl->out_vals = out_vals;

        if (pcfl->out_vals == NULL && pcfl->in_vals == NULL) {
          addSet (&blacklist, pcfl);
        } // if

        /* reschedule all successors */
        link = setFirstItem (pcfl->to);
        while (link) {
          //fprintf (stderr, "  %p --> %p\n", pcfl, link->pcflow);
          addSetIfnotP (&todo, link->pcflow);
          link = setNextItem (pcfl->to);
        } // while
      } else {
        deleteDefmapChain (&out_vals);
      }// if
    } else {
      deleteDefmapChain (&in_vals);
    } // if
  } // while
}

#if 0
static void showAllDefs (symbol_t sym, pCode *pc) {
  defmap_t *map;
  int count;

  assert (isPCI(pc));
  count = defmapFindAll (sym, pc, &map);

  fprintf (stderr, "sym %s(%x) @ %p defined as (val@pc): ", strFromSym(sym), sym, pc);
  while (map) {
#if 1
    fprintf (stderr, "(%x @ %p) ", map->val, map->pc);
#else
    { char buf[256];
    pic16_pCode2str (buf, 256, map->pc);
    fprintf (stderr, "\n    (%x @ %p(%s)) ", map->val, map->pc, buf);
#endif
    map = map->next;
  }
  deleteDefmapChain (&map);
}
#endif

/* safepCodeUnlink and remove pc from defmap. */
static int pic16_safepCodeRemove (pCode *pc, char *comment) {
  defmap_t *map, *next, **head;
  int res;

  map = isPCI(pc) ? PCI(pc)->pcflow->defmap : NULL;
  head = isPCI(pc) ? &PCI(pc)->pcflow->defmap : NULL;
  res = pic16_safepCodeUnlink (pc, comment);

  if (res && map) {
    /* remove pc from defmap */
    while (map) {
      next = map->next;
      if (map->pc == pc) {
        if (!map->prev && head) *head = map->next;
        deleteDefmap (map);
      } // if
      map = next;
    }
  }

  return res;
}

void pic16_fixDefmap (pCode *pc, pCode *newpc) {
  defmap_t *map;
  /* This breaks the defmap chain's references to pCodes... fix it! */
  map = PCI(pc)->pcflow->defmap;

  while (map && map->pc != pc) map = map->next;

  while (map && map->pc == pc) {
    map->pc = newpc;
    map = map->next;
  } // while
}

/* Replace a defmap entry for sym with newsym for read accesses (isRead == 1) or
 * write accesses (isRead == 0). */
void defmapReplaceSymRef (pCode *pc, symbol_t sym, symbol_t newsym, int isRead) {
  defmap_t *map, *map_start;
  defmap_t *copy;
  if (!isPCI(pc)) return;
  if (sym == newsym) return;

  map = PCI(pc)->pcflow->defmap;

  while (map && map->pc != pc) map = map->next;
  map_start = map;
  while (map && map->pc == pc) {
    if (map->sym == sym) {
      assert ((isRead && map->acc.access.isRead) || ((!isRead) && (map->acc.access.isWrite)));
      if (!(map->acc.access.isRead && map->acc.access.isWrite)) {
        /* only one kind of access handled... this is easy */
        map->sym = newsym;
      } else {
        /* must copy defmap entry before replacing symbol... */
        copy = copyDefmap (map);
        if (isRead) {
          map->acc.access.isRead = 0;
          copy->acc.access.isWrite = 0;
        } else {
          map->acc.access.isWrite = 0;
          copy->acc.access.isRead = 0;
        }
        copy->sym = newsym;
        /* insert copy into defmap chain */
        defmapInsertAfter (map, copy);
      }
    }
    map = map->next;
  } // while

  /* as this might introduce multiple defmap entries for newsym... */
  mergeDefmapSymbols (map_start);
}

/* Assign "better" valnums to results. */
static void assignValnums (pCode *pc) {
  pCodeInstruction *pci;
  pCode *newpc;
  symbol_t sym1, sym2;
  int cond, isSpecial1, isSpecial2, count, mask, lit;
  defmap_t *list, *val, *oldval, *dummy;
  reg_info *reg1 = NULL, *reg2 = NULL;
  valnum_t litnum;

  /* only works for pCodeInstructions... */
  if (!isPCI(pc)) return;

  pci = PCI(pc);
  cond = pci->inCond | pci->outCond;
  list = pci->pcflow->defmap;
  sym1 = sym2 = isSpecial1 = isSpecial2 = 0;

  if (cond & PCC_REGISTER) {
    sym1 = symFromStr (pic16_get_op (pci->pcop, NULL, 0));
    reg1 = pic16_getRegFromInstruction (pc);
    isSpecial1 = pic16_symIsSpecial (sym1);
  }
  if (cond & PCC_REGISTER2) {
    sym2 = symFromStr (pic16_get_op2 (pci->pcop, NULL, 0));
    reg2 = pic16_getRegFromInstruction (pc);
    isSpecial2 = pic16_symIsSpecial (sym2);
  }

  /* determine input values */
  val = list;
  while (val && val->pc != pc) val = val->next;
  //list = val; /* might save some time later... */
  while (val && val->pc == pc) {
    val->in_val = 0;
    if (val->sym != 0 && (1 || val->acc.access.isRead)) {
      /* get valnum for sym */
      count = defmapFindAll (val->sym, pc, &oldval);
      //fprintf (stderr, "%d defs for sym %s\n", count, strFromSym (val->sym));
      if (count == 1) {
        if ((val->acc.access.in_mask & oldval->acc.access.mask) == val->acc.access.in_mask) {
          val->in_val = oldval->val;
        } else {
          val->in_val = 0;
        }
      } else if (count == 0) {
        /* no definition found */
        val->in_val = 0;
      } else {
        /* multiple definition(s) found -- value not known (unless always the same valnum) */
        assert (oldval);
        dummy = oldval->next;
        mask = oldval->acc.access.mask;
        val->in_val = oldval->val;
        while (dummy && (dummy->val == val->in_val)) {
          mask &= dummy->acc.access.mask;
          dummy = dummy->next;
        } // while

        /* found other values or to restictive mask */
        if (dummy || ((mask & val->acc.access.in_mask) != val->acc.access.in_mask)) {
          val->in_val = 0;
        }
      }
      if (count > 0) deleteDefmapChain (&oldval);
    } // if
    val = val->next;
  }

  /* handle valnum assignment */
  switch (pci->op) {
  case POC_CLRF: /* modifies STATUS (Z) */
    if (!isSpecial1 && pic16_regIsLocal (reg1)) {
      oldval = defmapCurr (list, sym1, pc);
      if (oldval && (litFromValnum (oldval->in_val) == 0)) {
        //fprintf (stderr, "%s: REG (%s) already set up correctly (%x)\n", pci->mnemonic, strFromSym(sym1), oldval->in_val);
        if (!pic16_isAlive (SPO_STATUS, pc)) pic16_safepCodeRemove (pc, "=DF= redundant CLRF removed");
      }
      defmapUpdate (list, sym1, pc, valnumFromLit(0));
    }
    break;

  case POC_SETF: /* SETF does not touch STATUS */
    if (!isSpecial1 && pic16_regIsLocal (reg1)) {
      oldval = defmapCurr (list, sym1, pc);
      if (oldval && (litFromValnum (oldval->in_val) == 0x00FF)) {
        //fprintf (stderr, "%s: REG (%s) already set up correctly (%x)\n", pci->mnemonic, strFromSym(sym1), oldval->in_val);
        pic16_safepCodeRemove (pc, "=DF= redundant SETF removed");
      }
      defmapUpdate (list, sym1, pc, valnumFromLit (0x00FF));
    }
    break;

  case POC_MOVLW: /* does not touch STATUS */
    oldval = defmapCurr (list, SPO_WREG, pc);
    if (pci->pcop->type == PO_LITERAL) {
      //fprintf (stderr, "MOVLW: literal %u\n", PCOL(pci->pcop)->lit);
      litnum = valnumFromLit ((unsigned char)PCOL(pci->pcop)->lit);
    } else {
      //fprintf (stderr, "MOVLW: %s\n", pic16_get_op (pci->pcop, NULL, 0));
      litnum = valnumFromStr (pic16_get_op (pci->pcop, NULL, 0));
    }
    if (oldval && oldval->in_val == litnum) {
      //fprintf (stderr, "%s: W already set up correctly (%x)\n", PCI(pc)->mnemonic, oldval->in_val);
      pic16_safepCodeRemove (pc, "=DF= redundant MOVLW removed");
    }
    defmapUpdate (list, SPO_WREG, pc, litnum);
    break;

  case POC_ANDLW: /* modifies STATUS (Z,N) */
  case POC_IORLW: /* modifies STATUS (Z,N) */
  case POC_XORLW: /* modifies STATUS (Z,N) */
    /* can be optimized iff WREG contains a known literal (0x100 - 0x1FF) */
    if (pci->pcop->type == PO_LITERAL) {
      int vallit = -1;
      lit = (unsigned char) PCOL(pci->pcop)->lit;
      val = defmapCurr (list, SPO_WREG, pc);
      if (val) vallit = litFromValnum (val->in_val);
      if (vallit != -1) {
        /* xxxLW <literal>, WREG contains a known literal */
        //fprintf (stderr, "%s 0x%02x, WREG: 0x%x\n", pci->mnemonic, lit, vallit);
        if (pci->op == POC_ANDLW) {
          lit &= vallit;
        } else if (pci->op == POC_IORLW) {
          lit |= vallit;
        } else if (pci->op == POC_XORLW) {
          lit ^= vallit;
        } else {
          assert (0 && "invalid operation");
        }
        if (vallit == lit) {
          //fprintf (stderr, "%s: W already set up correctly (%x = val %x)\n", pci->mnemonic, vallit, val->in_val);
          if (!pic16_isAlive (SPO_STATUS, pc)) pic16_safepCodeRemove (pc, "=DF= redundant ANDLW/IORLW/XORLW removed");
        }
        defmapUpdate (list, SPO_WREG, pc, valnumFromLit (lit));
      } // if
    }
    break;

  case POC_LFSR:
    {
      /* check if old value matches new value */
      int lit;
      int ok = 1;
      assert (pci->pcop->type == PO_LITERAL);

      lit = PCOL(pci->pcop)->lit;

      val = defmapCurr (list, pic16_fsrsym_idx[lit][0], pc);

      if (val && (val->in_val != 0) && (val->in_val == val->val)) {
        //fprintf (stderr, "FSR%dL already set up correctly at %p (%x)\n", lit, pc, val->val);
      } else {
        /* cannot remove this LFSR */
        ok = 0;
      } // if

      val = defmapCurr (list, pic16_fsrsym_idx[lit][1], pc);
      if (val && (val->in_val != 0) && (val->in_val == val->val)) {
        //fprintf (stderr, "FSR%dH already set up correctly at %p (%x)\n", lit, pc, val->val);
      } else {
        ok = 0;
      } // if

      if (ok) {
        pic16_safepCodeRemove (pc, "=DF= redundant LFSR removed");
      }
    }
    break;

  case POC_MOVWF: /* does not touch flags */
    /* find value of WREG */
    val = defmapCurr (list, SPO_WREG, pc);
    oldval = defmapCurr (list, sym1, pc);
    if (val) lit = litFromValnum (val->in_val);
    else lit = -1;
    //fprintf (stderr, "MOVWF: lit: %i (%x, %x)\n", lit, lit, val->in_val);

    if ((lit == 0 || lit == 0x0ff) && !pic16_isAlive (SPO_STATUS, pc)) {
      /* might replace with CLRF/SETF (will possibly make previous MOVLW 0x00/0xff unneccessary --> dead code elimination) */
      //fprintf (stderr, "replacing MOVWF with CLRF/SETF\n");
      if (lit == 0) {
        newpc = pic16_newpCode (POC_CLRF, pic16_pCodeOpCopy (pci->pcop));
      } else {
        assert (lit == 0x0ff);
        newpc = pic16_newpCode (POC_SETF, pic16_pCodeOpCopy (pci->pcop));
      }
      if (pic16_debug_verbose || pic16_pcode_verbose) pic16_InsertCommentAfter (pc->prev, "=DF= MOVWF: replaced by CLRF/SETF");
      pic16_pCodeReplace (pc, newpc);
      defmapReplaceSymRef (pc, SPO_WREG, 0, 1);
      pic16_fixDefmap (pc, newpc);
      pc = newpc;

      /* This breaks the defmap chain's references to pCodes... fix it! */
      if (!val->prev) PCI(pc)->pcflow->defmap = val->next;
      if (!val->acc.access.isWrite) {
        deleteDefmap (val);     // delete reference to WREG as in value
        val = NULL;
      } else {
        val->acc.access.isRead = 0;     // delete reference to WREG as in value
      }
      oldval = PCI(pc)->pcflow->defmap;
      while (oldval) {
        if (oldval->pc == pc) oldval->pc = newpc;
          oldval = oldval->next;
      } // while
    } else if (!isSpecial1 && pic16_regIsLocal (reg1) && val && oldval && (val->in_val != 0) && (val->in_val == oldval->in_val)) {
      //fprintf (stderr, "MOVWF: F (%s) already set up correctly (%x) at %p\n", strFromSym (sym1), oldval->in_val, pc);
      pic16_safepCodeRemove (pc, "=DF= redundant MOVWF removed");
    }
    if (val) defmapUpdate (list, sym1, pc, val->in_val);
    break;

  case POC_MOVFW: /* modifies STATUS (Z,N) */
    /* find value of REG */
    if (!isSpecial1 && pic16_regIsLocal (reg1)) {
      val = defmapCurr (list, sym1, pc);
      oldval = defmapCurr (list, SPO_WREG, pc);
      if (val && oldval && (val->in_val != 0) && (val->in_val == oldval->in_val)) {
        //fprintf (stderr, "MOVFW: W already set up correctly (%x) at %p\n", oldval->in_val, pc);
        if (!pic16_isAlive (SPO_STATUS, pc)) pic16_safepCodeRemove (pc, "=DF= redundant MOVFW removed");
      } else {
          defmap_t *pred, *predpred;
          /* Optimize MOVLW immd; MOVWF reg1; [...]; MOVFW reg1
           * into MOVLW immd; MOVWF reg1; [...]; MOVLW immd
           * This might allow removal of the first two assignments. */
          pred = defmapFindDef (list, sym1, pc);
          predpred = pred ? defmapFindDef (list, SPO_WREG, pred->pc) : NULL;
          if (pred && predpred && (PCI(pred->pc)->op == POC_MOVWF) && (PCI(predpred->pc)->op == POC_MOVLW)
                && !pic16_isAlive (SPO_STATUS, pc))
          {
              newpc = pic16_newpCode (POC_MOVLW, pic16_pCodeOpCopy (PCI(predpred->pc)->pcop));

              if (pic16_debug_verbose || pic16_pcode_verbose) {
                  pic16_InsertCommentAfter (pc->prev, "=DF= MOVFW: replaced last of MOVLW;MOVWF;MOVFW by MOVLW");
              } // if
              pic16_pCodeReplace (pc, newpc);
              defmapReplaceSymRef (pc, sym1, 0, 1);
              pic16_fixDefmap (pc, newpc);
              pc = newpc;

              /* This breaks the defmap chain's references to pCodes... fix it! */
              if (!val->prev) PCI(pc)->pcflow->defmap = val->next;
              if (!val->acc.access.isWrite) {
                  deleteDefmap (val);   // delete reference to reg1 as in value
                  val = NULL;
              } else {
                  val->acc.access.isRead = 0;   // delete reference to reg1 as in value
              }
              oldval = PCI(pc)->pcflow->defmap;
              while (oldval) {
                  if (oldval->pc == pc) oldval->pc = newpc;
                  oldval = oldval->next;
              } // while
          } // if
      }
      if (val) defmapUpdate (list, SPO_WREG, pc, val->in_val);
    }
    break;

  case POC_MOVFF: /* does not touch STATUS */
    /* find value of REG */
    val = defmapCurr (list, sym1, pc);
    oldval = defmapCurr (list, sym2, pc);
    if (val) lit = litFromValnum (val->in_val);
    else lit = -1;
    newpc = NULL;
    if (!isSpecial1 && pic16_regIsLocal (reg1) && val && oldval && !pic16_isAlive (SPO_STATUS, pc)) {
      //pc->print (stderr, pc); fprintf (stderr, "lit: %d (%x, %x)\n", lit, lit, val->in_val);
      if (lit == 0) {
        newpc = pic16_newpCode (POC_CLRF, PCOP2(pci->pcop)->pcopR);
      } else if (lit == 0x00ff) {
        newpc = pic16_newpCode (POC_SETF, PCOP2(pci->pcop)->pcopR);
      } else {
        newpc = NULL;
      }
      if (newpc) {
        pic16_InsertCommentAfter (pc->prev, "=DF= MOVFF: replaced by CRLF/SETF");
        pic16_df_saved_bytes += PCI(pc)->isize - PCI(newpc)->isize;
        pic16_pCodeReplace (pc, newpc);
        defmapReplaceSymRef (pc, sym1, 0, 1);
        pic16_fixDefmap (pc, newpc);
        pc = newpc;
        break; // do not process instruction as MOVFF...
      }
    } else if (!isSpecial1 && !isSpecial2
                && pic16_regIsLocal (reg1) && pic16_regIsLocal (reg2)
                && val && oldval && (val->in_val != 0)) {
      if (val->in_val == oldval->in_val) {
        //fprintf (stderr, "MOVFF: F2 (%s) already set up correctly (%x) at %p\n", strFromSym (sym2), oldval->in_val, pc);
        pic16_safepCodeRemove (pc, "=DF= redundant MOVFF removed");
      } else {
        if (!pic16_isAlive (sym1, pc)) {
          defmap_t *copy = NULL;
          /* If there is another symbol S storing sym1's value we should assign from S thus shortening the liferange of sym1.
           * This should help eliminate
           *   MOVFF A,B
           *   <do something not changing A or using B>
           *   MOVFF B,C
           *   <B is not alive anymore>
           * and turn it into
           *   <do something not changing A or using B>
           *   MOVFF A,C
           */

          /* scan defmap for symbols storing sym1's value */
          while (oldval && (oldval->pc == pc || oldval->in_val != val->in_val)) oldval = oldval->next;
          if (oldval && (oldval->sym != sym1) && defmapFindAll (oldval->sym, pc, &copy) == 1) {
            /* unique reaching definition for sym found */
            if (copy->val && copy->val == val->in_val) {
              //fprintf (stderr, "found replacement symbol for %s (val %x) <-- %s (assigned %x @ %p)\n", strFromSym(sym1), val->in_val, strFromSym(copy->sym), copy->val, copy->pc);
              if (copy->sym == SPO_WREG) {
                newpc = pic16_newpCode (POC_MOVWF, pic16_pCodeOpCopy (PCOP2(pci->pcop)->pcopR));
              } else {
                pCodeOp *pcop = NULL;
                /* the code below fails if we try to replace
                 *   MOVFF PRODL, r0x03
                 *   MOVFF r0x03, PCLATU
                 * with
                 *   MOVFF PRODL, PCLATU
                 * as copy(PRODL) contains has pc==NULL, by name fails...
                 */
                if (!copy->pc || !PCI(copy->pc)->pcop) break;

                if (copy->pc && PCI(copy->pc)->pcop)
                  pcop = PCI(copy->pc)->pcop;
#if 0
                /* This code is broken--see above. */
                else
                {
                  const char *symname = strFromSym(copy->sym);

                  assert( symname );
                  pic16_InsertCommentAfter (pc->prev, "BUG-ME");
                  pic16_InsertCommentAfter (pc->prev, "=DF= MOVFF: newpCodeOpregFromStr(%s)", (char *)symname);
                  //pcop = pic16_newpCodeOpRegFromStr((char *)symname);
                }
#endif
                assert( pcop );
                newpc = pic16_newpCode(POC_MOVFF, pic16_popGet2p(
                        pcop,
                        pic16_pCodeOpCopy (PCOP2(pci->pcop)->pcopR)));
              }
              pic16_InsertCommentAfter (pc->prev, "=DF= MOVFF: SRC op %s replaced by %s", strFromSym(sym1), strFromSym(copy->sym));
              pic16_df_saved_bytes += PCI(pc)->isize - PCI(newpc)->isize;
              pic16_pCodeReplace (pc, newpc);
              assert (val->sym == sym1 && val->acc.access.isRead && !val->acc.access.isWrite);
              defmapReplaceSymRef (pc, sym1, copy->sym, 1);
              pic16_fixDefmap (pc, newpc);
              pc = newpc;
            }
          }
          deleteDefmapChain (&copy);
        }
      }
      if (val) defmapUpdate (list, sym2, pc, val->in_val);
    }
    break;

  default:
    /* cannot optimize */
    break;
  } // switch
}

static void pic16_destructDF (pBlock *pb) {
  pCode *pc, *next;

  if (!pb) return;

  /* remove old defmaps */
  pc = pic16_findNextInstruction (pb->pcHead);
  while (pc) {
    next = pic16_findNextInstruction (pc->next);

    assert (isPCI(pc) || isPCAD(pc));
    assert (PCI(pc)->pcflow);
    deleteDefmapChain (&PCI(pc)->pcflow->defmap);
    deleteDefmapChain (&PCI(pc)->pcflow->in_vals);
    deleteDefmapChain (&PCI(pc)->pcflow->out_vals);

    pc = next;
  } // while

  if (defmap_free || defmap_free_count) {
    //fprintf (stderr, "released defmaps: %u -- freeing up memory\n", defmap_free_count);
    freeDefmap (&defmap_free);
    defmap_free_count = 0;
  }
}

/* Checks whether a pBlock contains ASMDIRs. */
static int pic16_pBlockHasAsmdirs (pBlock *pb) {
  pCode *pc;

  if (!pb) return 0;

  pc = pic16_findNextInstruction (pb->pcHead);
  while (pc) {
    if (isPCAD(pc)) return 1;

    pc = pic16_findNextInstruction (pc->next);
  } // while

  /* no PCADs found */
  return 0;
}

#if 1
/* Remove MOVFF r0x??, POSTDEC1 and MOVFF PREINC1, r0x?? for otherwise unused registers. */
static int pic16_removeUnusedRegistersDF () {
  pCode *pc, *pc2;
  pBlock *pb;
  reg_info *reg1, *reg2, *reg3;
  set *seenRegs = NULL;
  int cond, i;
  int islocal, change = 0;

  /* no pBlocks? */
  if (!the_pFile || !the_pFile->pbHead) return 0;

  for (pb = the_pFile->pbHead; pb; pb = pb->next) {
    //fprintf (stderr, "%s: examining function %s\n", __FUNCTION__, pic16_pBlockGetFunctionName (pb));
#if 1
    /* find set of using pCodes per register */
    for (pc = pic16_findNextInstruction (pb->pcHead); pc;
                    pc = pic16_findNextInstruction(pc->next)) {

      cond = PCI(pc)->inCond | PCI(pc)->outCond;
      reg1 = reg2 = NULL;
      if (cond & PCC_REGISTER) reg1 = pic16_getRegFromInstruction (pc);
      if (cond & PCC_REGISTER2) reg2 = pic16_getRegFromInstruction2 (pc);

      if (reg1) {
        if (!isinSet (seenRegs, reg1)) reg1->reglives.usedpCodes = NULL;
        addSetIfnotP (&seenRegs, reg1);
        addSetIfnotP (&reg1->reglives.usedpCodes, pc);
      }
      if (reg2) {
        if (!isinSet (seenRegs, reg2)) reg2->reglives.usedpCodes = NULL;
        addSetIfnotP (&seenRegs, reg2);
        addSetIfnotP (&reg2->reglives.usedpCodes, pc);
      }
    } // for pc
#endif
    for (reg1 = setFirstItem (seenRegs); reg1; reg1 = setNextItem (seenRegs)) {
      /* may not use pic16_regIsLocal() here -- in interrupt routines
       * WREG, PRODx, FSR0x must be saved */
      islocal = (reg1->isLocal || reg1->rIdx == pic16_framepnt_lo->rIdx || reg1->rIdx == pic16_framepnt_hi->rIdx);
      if (islocal && elementsInSet (reg1->reglives.usedpCodes) == 2) {
        pc = pc2 = NULL;
        for (i=0; i < 2; i++) {
          pc = (pCode *) indexSet(reg1->reglives.usedpCodes, i);
          if (!pc2) pc2 = pc;
          if (!isPCI(pc) || !PCI(pc)->op == POC_MOVFF) continue;
          reg2 = pic16_getRegFromInstruction (pc);
          reg3 = pic16_getRegFromInstruction2 (pc);
          if (!reg2 || !reg3
              || (reg2->rIdx != pic16_stack_preinc->rIdx
                  && reg3->rIdx != pic16_stack_postdec->rIdx)) break;
          if (i == 1) {
            /* both pCodes are MOVFF R,POSTDEC1 / MOVFF PREINC1,R */
            //fprintf (stderr, "%s: removing local register %s from %s\n", __FUNCTION__, reg1->name, pic16_pBlockGetFunctionName (pb));
            pic16_safepCodeRemove (pc, "removed unused local reg IN");
            pic16_safepCodeRemove (pc2, "removed unused local reg OUT");
          }
        } // for
      } // if
      deleteSet (&reg1->reglives.usedpCodes);
    } // for reg1

    deleteSet (&seenRegs);
  } // for pb

  return change;
}
#endif

/* Set up pCodeFlow's defmap_ts.
 * Needs correctly set up to/from fields. */
static void pic16_createDF (pBlock *pb) {
  pCode *pc, *next;
  int change=0;

  if (!pb) return;

  //fprintf (stderr, "creating DF for pb %p (%s)\n", pb, pic16_pBlockGetFunctionName (pb));

  pic16_destructDF (pb);

  /* check pBlock: do not analyze pBlocks with ASMDIRs (for now...) */
  if (pic16_pBlockHasAsmdirs (pb)) {
    //fprintf (stderr, "%s: pBlock contains ASMDIRs -- data flow analysis not performed!\n", __FUNCTION__);
    return;
  }

  /* integrity check -- we need to reach all flows to guarantee
   * correct data flow analysis (reaching definitions, aliveness) */
#if 0
  if (!verifyAllFlowsReachable (pb)) {
    fprintf (stderr, "not all flows reachable -- aborting dataflow analysis for %s!\n", pic16_pBlockGetFunctionName (pb));
    return;
  }
#endif

  /* establish new defmaps */
  pc = pic16_findNextInstruction (pb->pcHead);
  while (pc) {
    next = pic16_findNextInstruction (pc->next);

    assert (PCI(pc)->pcflow);
    PCI(pc)->pcflow->defmap = createDefmap (pc, PCI(pc)->pcflow->defmap);

    pc = next;
  } // while

  //fprintf (stderr, "%s: creating reaching definitions...\n", __FUNCTION__);
  createReachingDefinitions (pb);

#if 1
  /* assign better valnums */
  //fprintf (stderr, "assigning valnums for pb %p\n", pb);
  pc = pic16_findNextInstruction (pb->pcHead);
  while (pc) {
    next = pic16_findNextInstruction (pc->next);

    assert (PCI(pc)->pcflow);
    assignValnums (pc);

    pc = next;
  } // while
#endif

#if 1
  /* remove dead pCodes */
  //fprintf (stderr, "removing dead pCodes in %p (%s)\n", pb, pic16_pBlockGetFunctionName (pb));
  do {
    change = 0;
    pc = pic16_findNextInstruction (pb->pcHead);
    while (pc) {
      next = pic16_findNextInstruction (pc->next);

      if (isPCI(pc) && !isPCI_BRANCH(pc) && !pic16_pCodeIsAlive (pc)) {
        change += pic16_safepCodeRemove (pc, "=DF= removed dead pCode");
      }

      pc = next;
    } // while
  } while (change);
#endif
}

/* ======================================================================== */
/* === VCG DUMPER ROUTINES ================================================ */
/* ======================================================================== */
#if defined (DUMP_DF_GRAPHS) && DUMP_DF_GRAPHS > 0
hTab *dumpedNodes = NULL;

/** Dump VCG header into of. */
static void pic16_vcg_init (FILE *of) {
  /* graph defaults */
  fprintf (of, "graph:{\n");
  fprintf (of, "title:\"graph1\"\n");
  fprintf (of, "label:\"graph1\"\n");
  fprintf (of, "color:white\n");
  fprintf (of, "textcolor:black\n");
  fprintf (of, "bordercolor:black\n");
  fprintf (of, "borderwidth:1\n");
  fprintf (of, "textmode:center\n");

  fprintf (of, "layoutalgorithm:dfs\n");
  fprintf (of, "late_edge_labels:yes\n");
  fprintf (of, "display_edge_labels:yes\n");
  fprintf (of, "dirty_edge_labels:yes\n");
  fprintf (of, "finetuning:yes\n");
  fprintf (of, "ignoresingles:no\n");
  fprintf (of, "straight_phase:yes\n");
  fprintf (of, "priority_phase:yes\n");
  fprintf (of, "manhattan_edges:yes\n");
  fprintf (of, "smanhattan_edges:no\n");
  fprintf (of, "nearedges:no\n");
  fprintf (of, "node_alignment:center\n"); // bottom|top|center
  fprintf (of, "port_sharing:no\n");
  fprintf (of, "arrowmode:free\n"); // fixed|free
  fprintf (of, "crossingphase2:yes\n");
  fprintf (of, "crossingoptimization:yes\n");
  fprintf (of, "edges:yes\n");
  fprintf (of, "nodes:yes\n");
  fprintf (of, "splines:no\n");

  /* node defaults */
  fprintf (of, "node.color:lightyellow\n");
  fprintf (of, "node.textcolor:black\n");
  fprintf (of, "node.textmode:center\n");
  fprintf (of, "node.shape:box\n");
  fprintf (of, "node.bordercolor:black\n");
  fprintf (of, "node.borderwidth:1\n");

  /* edge defaults */
  fprintf (of, "edge.textcolor:black\n");
  fprintf (of, "edge.color:black\n");
  fprintf (of, "edge.thickness:1\n");
  fprintf (of, "edge.arrowcolor:black\n");
  fprintf (of, "edge.backarrowcolor:black\n");
  fprintf (of, "edge.arrowsize:15\n");
  fprintf (of, "edge.backarrowsize:15\n");
  fprintf (of, "edge.arrowstyle:line\n"); // none|solid|line
  fprintf (of, "edge.backarrowstyle:none\n"); // none|solid|line
  fprintf (of, "edge.linestyle:continuous\n"); // continuous|solid|dotted|dashed|invisible

  fprintf (of, "\n");

  /* prepare data structures */
  if (dumpedNodes) {
    hTabDeleteAll (dumpedNodes);
    dumpedNodes = NULL;
  }
  dumpedNodes = newHashTable (128);
}

/** Dump VCG footer into of. */
static void pic16_vcg_close (FILE *of) {
  fprintf (of, "}\n");
}

#define BUF_SIZE 128
#define pcTitle(pc) (SNPRINTF (buf, BUF_SIZE, "n_%p, %p/%u", PCODE(pc), isPCI(pc) ? PCI(pc)->pcflow : NULL, PCODE(pc)->seq), &buf[0])

#if 0
static int ptrcmp (const void *p1, const void *p2) {
  return p1 == p2;
}
#endif

/** Dump a pCode node as VCG to of. */
static void pic16_vcg_dumpnode (pCode *pc, FILE *of) {
  char buf[BUF_SIZE];

  if (hTabFindByKey (dumpedNodes, (((char *) pc - (char *) 0)>>2) % 128, pc, ptrcmp)) {
    // dumped already
    return;
  }
  hTabAddItemLong (&dumpedNodes, (((char *) pc - (char *) 0)>>2) % 128, pc, pc);
  //fprintf (stderr, "dumping %p\n", pc);

  /* only dump pCodeInstructions and Flow nodes */
  if (!isPCI(pc) && !isPCAD(pc) && !isPCFL(pc)) return;

  /* emit node */
  fprintf (of, "node:{");
  fprintf (of, "title:\"%s\" ", pcTitle(pc));
  fprintf (of, "label:\"%s\n", pcTitle(pc));
  if (isPCFL(pc)) {
    fprintf (of, "<PCFLOW>");
  } else if (isPCI(pc) || isPCAD(pc)) {
    pc->print (of, pc);
  } else {
    fprintf (of, "<!PCI>");
  }
  fprintf (of, "\" ");
  fprintf (of, "}\n");

  if (1 && isPCFL(pc)) {
    defmap_t *map, *prev;
    unsigned int i;
    map = PCFL(pc)->defmap;
    i=0;
    while (map) {
      if (map->sym != 0) {
        i++;

        /* emit definition node */
        fprintf (of, "node:{title:\"%s_def%u\" ", pcTitle(pc), i);
        fprintf (of, "label:\"");

        prev = map;
        do {
          fprintf (of, "%s%c%c: val %4x|%4x & %02x|%02x, sym %s", (prev == map) ? "" : "\n", map->acc.access.isRead ? 'R' : ' ', map->acc.access.isWrite ? 'W' : ' ', map->in_val, map->val, map->acc.access.in_mask, map->acc.access.mask, strFromSym (map->sym));
          prev = map;
          map = map->next;
        } while (map && prev->pc == map->pc);
        map = prev;

        fprintf (of, "\" ");

        fprintf (of, "color:green ");
        fprintf (of, "}\n");

        /* emit edge to previous definition */
        fprintf (of, "edge:{sourcename:\"%s_def%u\" ", pcTitle(pc), i);
        if (i == 1) {
          fprintf (of, "targetname:\"%s\" ", pcTitle(pc));
        } else {
          fprintf (of, "targetname:\"%s_def%u\" ", pcTitle(pc), i-1);
        }
        fprintf (of, "color:green ");
        fprintf (of, "}\n");

        if (map->pc) {
          pic16_vcg_dumpnode (map->pc, of);
          fprintf (of, "edge:{sourcename:\"%s_def%u\" ", pcTitle(pc), i);
          fprintf (of, "targetname:\"%s\" linestyle:dashed color:lightgreen}\n", pcTitle(map->pc));
        }
      }
      map = map->next;
    } // while
  }

  /* emit additional nodes (e.g. operands) */
}

/** Dump a pCode's edges (control flow/data flow) as VCG to of. */
static void pic16_vcg_dumpedges (pCode *pc, FILE *of) {
  char buf[BUF_SIZE];
  pCodeInstruction *pci;
  pBranch *curr;
  int i;

  if (1 && isPCFL(pc)) {
    /* emit edges to flow successors */
    void *pcfl;
    //fprintf (stderr, "PCFLOWe @ %p\n", pc);
    pcfl = setFirstItem (PCFL(pc)->to);
    while (pcfl) {
      pcfl = ((pCodeFlowLink *) (pcfl))->pcflow;
      pic16_vcg_dumpnode (pc, of);
      pic16_vcg_dumpnode ((pCode *) pcfl, of);
      fprintf (of, "edge:{sourcename:\"%s\" ", pcTitle(pc));
      fprintf (of, "targetname:\"%s\" color:lightred linestyle:dashed}\n", pcTitle(pcfl));
      pcfl = setNextItem (PCFL(pc)->to);
    } // while
  } // if

  if (!isPCI(pc) && !isPCAD(pc)) return;

  pci = PCI(pc);

  /* emit control flow edges (forward only) */
  curr = pci->to;
  i=0;
  while (curr) {
    pic16_vcg_dumpnode (curr->pc, of);
    fprintf (of, "edge:{");
    fprintf (of, "sourcename:\"%s\" ", pcTitle(pc));
    fprintf (of, "targetname:\"%s\" ", pcTitle(curr->pc));
    fprintf (of, "color:red ");
    fprintf (of, "}\n");
    curr = curr->next;
  } // while

#if 1
  /* dump "flow" edge (link pCode according to pBlock order) */
  {
    pCode *pcnext;
    pcnext = pic16_findNextInstruction (pc->next);
    if (pcnext) {
      pic16_vcg_dumpnode (pcnext, of);
      fprintf (of, "edge:{sourcename:\"%s\" ", pcTitle(pc));
      fprintf (of, "targetname:\"%s\" color:red linestyle:solid}\n", pcTitle(pcnext));
    }
  }
#endif

#if 0
  /* emit flow */
  if (pci->pcflow) {
    pic16_vcg_dumpnode (&pci->pcflow->pc, of);
    fprintf (of, "edge:{sourcename:\"%s\" ", pcTitle(pc));
    fprintf (of, "targetname:\"%s\" color:lightblue linestyle:dashed}\n", pcTitle (pci->pcflow));
  }
#endif

  /* emit data flow edges (backward only) */
  /* TODO: gather data flow information... */
}

static void pic16_vcg_dump (FILE *of, pBlock *pb) {
  pCode *pc;

  if (!pb) return;

  /* check pBlock: do not analyze pBlocks with ASMDIRs (for now...) */
  if (pic16_pBlockHasAsmdirs (pb)) {
    //fprintf (stderr, "%s: pBlock contains ASMDIRs -- data flow analysis not performed!\n", __FUNCTION__);
    return;
  }

  for (pc=pb->pcHead; pc; pc = pc->next) {
    pic16_vcg_dumpnode (pc, of);
  } // for pc

  for (pc=pb->pcHead; pc; pc = pc->next) {
    pic16_vcg_dumpedges (pc, of);
  } // for pc
}

static void pic16_vcg_dump_default (pBlock *pb) {
  FILE *of;
  char buf[BUF_SIZE];
  pCode *pc;

  if (!pb) return;

  /* get function name */
  pc = pb->pcHead;
  while (pc && !isPCF(pc)) pc = pc->next;
  if (pc) {
    SNPRINTF (buf, BUF_SIZE, "%s_%s.vcg", PCF(pc)->modname, PCF(pc)->fname);
  } else {
    SNPRINTF (buf, BUF_SIZE, "pb_%p.vcg", pb);
  }

  //fprintf (stderr, "now dumping %s\n", buf);
  of = fopen (buf, "w");
  pic16_vcg_init (of);
  pic16_vcg_dump (of, pb);
  pic16_vcg_close (of);
  fclose (of);
}
#endif

/*** END of helpers for pCode dataflow optimizations ***/
