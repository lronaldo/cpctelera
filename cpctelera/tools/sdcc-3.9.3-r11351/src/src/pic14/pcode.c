/*-------------------------------------------------------------------------

	pcode.c - post code generation
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

#include "device.h"
#include "gen.h"
#include "pcode.h"
#include "pcodeflow.h"
#include "ralloc.h"

/****************************************************************/
/****************************************************************/

// Eventually this will go into device dependent files:
pCodeOpReg pc_status    = {{PO_STATUS,  "STATUS"}, -1, NULL,0,NULL};
pCodeOpReg pc_fsr       = {{PO_FSR,     "FSR"}, -1, NULL,0,NULL};
pCodeOpReg pc_fsr0l     = {{PO_FSR,     "FSR0L"}, -1, NULL,0,NULL};
pCodeOpReg pc_fsr0h     = {{PO_FSR,     "FSR0H"}, -1, NULL,0,NULL};
pCodeOpReg pc_indf_     = {{PO_INDF,    "INDF"}, -1, NULL,0,NULL};
pCodeOpReg pc_indf0     = {{PO_INDF,    "INDF0"}, -1, NULL,0,NULL};
pCodeOpReg pc_intcon    = {{PO_INTCON,  "INTCON"}, -1, NULL,0,NULL};
pCodeOpReg pc_pcl       = {{PO_PCL,     "PCL"}, -1, NULL,0,NULL};
pCodeOpReg pc_pclath    = {{PO_PCLATH,  "PCLATH"}, -1, NULL,0,NULL};

pCodeOpReg *pc_indf     = &pc_indf_;

pCodeOpReg pc_wsave     = {{PO_GPR_REGISTER,  "WSAVE"}, -1, NULL,0,NULL};
pCodeOpReg pc_ssave     = {{PO_GPR_REGISTER,  "SSAVE"}, -1, NULL,0,NULL};
pCodeOpReg pc_psave     = {{PO_GPR_REGISTER,  "PSAVE"}, -1, NULL,0,NULL};

pFile *the_pFile = NULL;


#define SET_BANK_BIT (1 << 16)
#define CLR_BANK_BIT 0

static peepCommand peepCommands[] = {

	{NOTBITSKIP, "_NOTBITSKIP_"},
	{BITSKIP, "_BITSKIP_"},
	{INVERTBITSKIP, "_INVERTBITSKIP_"},

	{-1, NULL}
};

static int mnemonics_initialized = 0;

static hTab *pic14MnemonicsHash = NULL;
static hTab *pic14pCodePeepCommandsHash = NULL;

static pBlock *pb_dead_pcodes = NULL;

/* Hardcoded flags to change the behavior of the PIC port */
static int functionInlining = 1;      /* inline functions if nonzero */

// static int GpCodeSequenceNumber = 1;
static int GpcFlowSeq = 1;

/* statistics (code size estimation) */
static unsigned int pcode_insns = 0;
static unsigned int pcode_doubles = 0;

static unsigned peakIdx = 0; /* This keeps track of the peak register index for call tree register reuse */


/****************************************************************/
/*                      Forward declarations                    */
/****************************************************************/

static void genericDestruct(pCode *pc);
static void genericPrint(FILE *of,pCode *pc);

static void pBlockStats(FILE *of, pBlock *pb);
static pCode *findFunction(const char *fname);
static void pCodePrintLabel(FILE *of, pCode *pc);
static void pCodePrintFunction(FILE *of, pCode *pc);
static void pCodeOpPrint(FILE *of, pCodeOp *pcop);
static char *get_op_from_instruction( pCodeInstruction *pcc);
static pBlock *newpBlock(void);


/****************************************************************/
/*                    PIC Instructions                          */
/****************************************************************/

static pCodeInstruction pciADDWF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ADDWF,
		"ADDWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_REGISTER | PCC_C | PCC_DC | PCC_Z) // outCond
};

static pCodeInstruction pciADDFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ADDFW,
		"ADDWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_W | PCC_C | PCC_DC | PCC_Z) // outCond
};

static pCodeInstruction pciADDLW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ADDLW,
		"ADDLW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_LITERAL),   // inCond
		(PCC_W | PCC_Z | PCC_C | PCC_DC) // outCond
};

static pCodeInstruction pciANDLW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ANDLW,
		"ANDLW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_LITERAL),   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciANDWF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ANDWF,
		"ANDWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_REGISTER | PCC_Z) // outCond
};

static pCodeInstruction pciANDFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ANDFW,
		"ANDWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciBCF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_BCF,
		"BCF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		TRUE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_BSF,
		(PCC_REGISTER | PCC_EXAMINE_PCOP),	// inCond
		(PCC_REGISTER | PCC_EXAMINE_PCOP)	// outCond
};

static pCodeInstruction pciBSF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_BSF,
		"BSF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		TRUE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_BCF,
		(PCC_REGISTER | PCC_EXAMINE_PCOP),	// inCond
		(PCC_REGISTER | PCC_EXAMINE_PCOP)	// outCond
};

static pCodeInstruction pciBTFSC = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_BTFSC,
		"BTFSC",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		TRUE,  // bit instruction
		TRUE,  // branch
		TRUE,  // skip
		FALSE,    // literal operand
		POC_BTFSS,
		(PCC_REGISTER | PCC_EXAMINE_PCOP),	// inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciBTFSS = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_BTFSS,
		"BTFSS",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		TRUE,  // bit instruction
		TRUE,  // branch
		TRUE,  // skip
		FALSE,    // literal operand
		POC_BTFSC,
		(PCC_REGISTER | PCC_EXAMINE_PCOP),   // inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciCALL = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_CALL,
		"CALL",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_NONE | PCC_W), // inCond, reads argument from WREG
		(PCC_NONE | PCC_W | PCC_C | PCC_DC | PCC_Z)  // outCond, flags are destroyed by called function
};

static pCodeInstruction pciCOMF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_COMF,
		"COMF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_REGISTER | PCC_Z)  // outCond
};

static pCodeInstruction pciCOMFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_COMFW,
		"COMF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_W | PCC_Z)  // outCond
};

static pCodeInstruction pciCLRF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_CLRF,
		"CLRF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_REGISTER | PCC_Z // outCond
};

static pCodeInstruction pciCLRW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_CLRW,
		"CLRW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		0,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciCLRWDT = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_CLRWDT,
		"CLRWDT",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		0,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_NONE  // outCond
};

static pCodeInstruction pciDECF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_DECF,
		"DECF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_REGISTER | PCC_Z) // outCond
};

static pCodeInstruction pciDECFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_DECFW,
		"DECF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciDECFSZ = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_DECFSZ,
		"DECFSZ",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		TRUE,  // skip
		FALSE,    // literal operand
		POC_DECF,		// followed by BTFSC STATUS, Z --> also kills STATUS
		PCC_REGISTER,		// inCond
		(PCC_REGISTER | PCC_Z)	// outCond
};

static pCodeInstruction pciDECFSZW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_DECFSZW,
		"DECFSZ",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		TRUE,  // skip
		FALSE,    // literal operand
		POC_DECFW,	// followed by BTFSC STATUS, Z --> also kills STATUS
		PCC_REGISTER,   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciGOTO = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_GOTO,
		"GOTO",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE,   // inCond
		PCC_NONE    // outCond
};

static pCodeInstruction pciINCF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_INCF,
		"INCF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER,   // inCond
		(PCC_REGISTER | PCC_Z) // outCond
};

static pCodeInstruction pciINCFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_INCFW,
		"INCF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER,   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciINCFSZ = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_INCFSZ,
		"INCFSZ",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		TRUE,  // skip
		FALSE,    // literal operand
		POC_INCF,		// followed by BTFSC STATUS, Z --> also kills STATUS
		PCC_REGISTER,		// inCond
		(PCC_REGISTER | PCC_Z)  // outCond
};

static pCodeInstruction pciINCFSZW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_INCFSZW,
		"INCFSZ",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		TRUE,  // skip
		FALSE,    // literal operand
		POC_INCFW,	// followed by BTFSC STATUS, Z --> also kills STATUS
		PCC_REGISTER,   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciIORWF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_IORWF,
		"IORWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_REGISTER | PCC_Z) // outCond
};

static pCodeInstruction pciIORFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_IORFW,
		"IORWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciIORLW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_IORLW,
		"IORLW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_LITERAL),   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciMOVF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVF,
		"MOVF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER,   // inCond
		PCC_Z // outCond
};

static pCodeInstruction pciMOVFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVFW,
		"MOVF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER,   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciMOVWF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVWF,
		"MOVWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_W,   // inCond
		PCC_REGISTER // outCond
};

static pCodeInstruction pciMOVLW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVLW,
		"MOVLW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		(PCC_NONE | PCC_LITERAL),   // inCond
		PCC_W // outCond
};

static pCodeInstruction pciNOP = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_NOP,
		"NOP",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		0,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE,   // inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciRETFIE = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_RETFIE,
		"RETFIE",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		0,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE,   // inCond
		(PCC_NONE | PCC_C | PCC_DC | PCC_Z) // outCond (not true... affects the GIE bit too), STATUS bit are retored
};

static pCodeInstruction pciRETLW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_RETLW,
		"RETLW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		PCC_LITERAL,   // inCond
		(PCC_W| PCC_C | PCC_DC | PCC_Z) // outCond, STATUS bits are irrelevant after RETLW
};

static pCodeInstruction pciRETURN = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_RETURN,
		"RETURN",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		0,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_NONE | PCC_W), // inCond, return value is possibly present in W
		(PCC_NONE | PCC_C | PCC_DC | PCC_Z) // outCond, STATUS bits are irrelevant after RETURN
};

static pCodeInstruction pciRLF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_RLF,
		"RLF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_C | PCC_REGISTER), // inCond
		(PCC_REGISTER | PCC_C) // outCond
};

static pCodeInstruction pciRLFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_RLFW,
		"RLF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_C | PCC_REGISTER),   // inCond
		(PCC_W | PCC_C) // outCond
};

static pCodeInstruction pciRRF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_RRF,
		"RRF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_C | PCC_REGISTER),   // inCond
		(PCC_REGISTER | PCC_C) // outCond
};

static pCodeInstruction pciRRFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_RRFW,
		"RRF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_C | PCC_REGISTER),   // inCond
		(PCC_W | PCC_C) // outCond
};

static pCodeInstruction pciSUBWF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_SUBWF,
		"SUBWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_REGISTER | PCC_C | PCC_DC | PCC_Z) // outCond
};

static pCodeInstruction pciSUBFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_SUBFW,
		"SUBWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_W | PCC_C | PCC_DC | PCC_Z) // outCond
};

static pCodeInstruction pciSUBLW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_SUBLW,
		"SUBLW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_LITERAL),   // inCond
		(PCC_W | PCC_Z | PCC_C | PCC_DC) // outCond
};

static pCodeInstruction pciSWAPF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_SWAPF,
		"SWAPF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER,   // inCond
		PCC_REGISTER // outCond
};

static pCodeInstruction pciSWAPFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_SWAPFW,
		"SWAPF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER,   // inCond
		PCC_W // outCond
};

static pCodeInstruction pciTRIS = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_TRIS,
		"TRIS",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE,   // inCond /* FIXME: what's TRIS doing? */
		PCC_REGISTER // outCond	/* FIXME: what's TRIS doing */
};

static pCodeInstruction pciXORWF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_XORWF,
		"XORWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_REGISTER | PCC_Z) // outCond
};

static pCodeInstruction pciXORFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_XORFW,
		"XORWF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER),   // inCond
		(PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciXORLW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_XORLW,
		"XORLW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_LITERAL),   // inCond
		(PCC_W | PCC_Z) // outCond
};


static pCodeInstruction pciBANKSEL = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_BANKSEL,
		"BANKSEL",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_NONE  // outCond
};

static pCodeInstruction pciPAGESEL = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_PAGESEL,
		"PAGESEL",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_NONE  // outCond
};

/****************************************************************/
/*                    PIC Enhanced Instructions                 */
/****************************************************************/

static pCodeInstruction pciADDFSR = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ADDFSR,
		"ADDFSR",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_NONE  // outCond
};

static pCodeInstruction pciADDWFC = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ADDWFC,
		"ADDWFC",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
	        (PCC_W | PCC_REGISTER | PCC_C), // inCond
		(PCC_REGISTER | PCC_C | PCC_DC | PCC_Z) // outCond
};

static pCodeInstruction pciADDFWC = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ADDFWC,
		"ADDWFC",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER | PCC_C), // inCond
		(PCC_W | PCC_C | PCC_DC | PCC_Z) // outCond
};

static pCodeInstruction pciASRF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ASRF,
		"ASRF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_REGISTER | PCC_C | PCC_Z) // outCond
};

static pCodeInstruction pciASRFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_ASRFW,
		"ASRF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_W | PCC_C | PCC_Z) // outCond
};

static pCodeInstruction pciBRA = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_BRA,
		"BRA",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_NONE  // outCond
};

static pCodeInstruction pciBRW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_BRW,
		"BRW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		0,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_NONE | PCC_W), // inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciCALLW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_CALLW,
		"CALLW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		0,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		TRUE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_NONE | PCC_W), // inCond, reads lower bits of subroutine address from WREG
		(PCC_NONE | PCC_W | PCC_C | PCC_DC | PCC_Z)  // outCond, flags are destroyed by called function
};

static pCodeInstruction pciLSLF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_LSLF,
		"LSLF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_REGISTER | PCC_C | PCC_Z) // outCond
};

static pCodeInstruction pciLSLFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_LSLFW,
		"LSLF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_W | PCC_C | PCC_Z) // outCond
};

static pCodeInstruction pciLSRF = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_LSRF,
		"LSRF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_REGISTER | PCC_C | PCC_Z) // outCond
};

static pCodeInstruction pciLSRFW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_LSRFW,
		"LSRF",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_REGISTER, // inCond
		(PCC_W | PCC_C | PCC_Z) // outCond
};

static pCodeInstruction pciMOVIW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVIW,
		"MOVIW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		(PCC_NONE | PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciMOVIW_K = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVIW_K,
		"MOVIW",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		(PCC_NONE | PCC_W | PCC_Z) // outCond
};

static pCodeInstruction pciMOVLB = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVLB,
		"MOVLB",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciMOVLP = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVLP,
		"MOVLP",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciMOVWI = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVWI,
		"MOVWI",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		1,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		(PCC_NONE | PCC_W), // inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciMOVWI_K = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_MOVWI_K,
		"MOVWI",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		TRUE,    // literal operand
		POC_NOP,
		(PCC_NONE | PCC_W), // inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciRESET = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_RESET,
		"RESET",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		0,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		PCC_NONE, // inCond
		PCC_NONE // outCond
};

static pCodeInstruction pciSUBWFB = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_SUBWFB,
		"SUBWFB",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		TRUE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER | PCC_C), // inCond
		(PCC_REGISTER | PCC_C | PCC_DC | PCC_Z) // outCond
};

static pCodeInstruction pciSUBWFBW = {
	{PC_OPCODE, NULL, NULL, 0, 0, NULL, 
		genericDestruct,
		genericPrint},
		POC_SUBWFBW,
		"SUBWFB",
		NULL, // from branch
		NULL, // to branch
		NULL, // label
		NULL, // operand
		NULL, // flow block
		NULL, // C source 
		2,    // num ops
		FALSE,  // dest
		FALSE,  // bit instruction
		FALSE,  // branch
		FALSE,  // skip
		FALSE,    // literal operand
		POC_NOP,
		(PCC_W | PCC_REGISTER | PCC_C), // inCond
		(PCC_W | PCC_C | PCC_DC | PCC_Z) // outCond
};

pCodeInstruction *pic14Mnemonics[MAX_PIC14MNEMONICS];


/*-----------------------------------------------------------------*/
/* return a unique ID number to assist pCodes debuging             */
/*-----------------------------------------------------------------*/
static unsigned PCodeID(void) {
	static unsigned int pcodeId = 1; /* unique ID number to be assigned to all pCodes */
	/*
	static unsigned int stop;
	if (pcodeId == 1448)
		stop++; // Place break point here
	*/
	return pcodeId++;
}

void  pCodeInitRegisters(void)
{
	static int initialized=0;
	int shareBankAddress, stkSize, haveShared;
	PIC_device *pic;
	
	if(initialized)
		return;
	initialized = 1;
	
	pic = init_pic(port->processor);
	haveShared = pic14_getSharedStack(NULL, &shareBankAddress, &stkSize);
	/* Set pseudo stack size to SHAREBANKSIZE - 3.
	 * On multi memory bank ICs this leaves room for WSAVE/SSAVE/PSAVE
	 * (used for interrupts) to fit into the shared portion of the
	 * memory bank. This is not needed on enhanced processors. */
	if (!pic->isEnhancedCore)
		stkSize = stkSize - 3;
	assert(stkSize >= 0);
	initStack(shareBankAddress, stkSize, haveShared);
	
	/* TODO: Read aliases for SFRs from regmap lines in device description. */
	pc_status.r = allocProcessorRegister(IDX_STATUS,"STATUS", PO_STATUS, 0xf80);
	pc_pcl.r = allocProcessorRegister(IDX_PCL,"PCL", PO_PCL, 0xf80);
	pc_pclath.r = allocProcessorRegister(IDX_PCLATH,"PCLATH", PO_PCLATH, 0xf80);
	pc_indf_.r = allocProcessorRegister(IDX_INDF,"INDF", PO_INDF, 0xf80);
	pc_indf0.r = allocProcessorRegister(IDX_INDF0,"INDF0", PO_INDF, 0xf80);
	pc_fsr.r = allocProcessorRegister(IDX_FSR,"FSR", PO_FSR, 0xf80);
	pc_fsr0l.r = allocProcessorRegister(IDX_FSR0L,"FSR0L", PO_FSR, 0xf80);
	pc_fsr0h.r = allocProcessorRegister(IDX_FSR0H,"FSR0H", PO_FSR, 0xf80);
	pc_intcon.r = allocProcessorRegister(IDX_INTCON,"INTCON", PO_INTCON, 0xf80);
	
	pc_status.rIdx = IDX_STATUS;
	pc_fsr.rIdx = IDX_FSR;
	pc_fsr0l.rIdx = IDX_FSR0L;
	pc_fsr0h.rIdx = IDX_FSR0H;
	pc_indf_.rIdx = IDX_INDF;
	pc_indf0.rIdx = IDX_INDF0;
	pc_intcon.rIdx = IDX_INTCON;
	pc_pcl.rIdx = IDX_PCL;
	pc_pclath.rIdx = IDX_PCLATH;

	if (!pic->isEnhancedCore) {
		/* Interrupt storage for working register - must be same address in all banks ie section SHAREBANK. */
		pc_wsave.r = allocInternalRegister(IDX_WSAVE,pc_wsave.pcop.name,pc_wsave.pcop.type, pic ? pic->bankMask : 0xf80);
		/* Interrupt storage for status register. */
		pc_ssave.r = allocInternalRegister(IDX_SSAVE,pc_ssave.pcop.name,pc_ssave.pcop.type, (pic && haveShared) ? pic->bankMask : 0);
		/* Interrupt storage for pclath register. */
		pc_psave.r = allocInternalRegister(IDX_PSAVE,pc_psave.pcop.name,pc_psave.pcop.type, (pic && haveShared) ? pic->bankMask : 0);
	
		pc_wsave.rIdx = pc_wsave.r->rIdx;
		pc_ssave.rIdx = pc_ssave.r->rIdx;
		pc_psave.rIdx = pc_psave.r->rIdx;
	
		pc_wsave.r->isFixed = 1; /* Some PIC ICs do not have a sharebank - this register needs to be reserved across all banks. */
		pc_wsave.r->address = shareBankAddress-stkSize;
		pc_ssave.r->isFixed = 1; /* This register must be in the first bank. */
		pc_ssave.r->address = shareBankAddress-stkSize-1;
		pc_psave.r->isFixed = 1; /* This register must be in the first bank. */
		pc_psave.r->address = shareBankAddress-stkSize-2;
	}

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

static void pic14initMnemonics(void)
{
	int i = 0;
	int key;
	//  char *str;
	pCodeInstruction *pci;
	
	if(mnemonics_initialized)
		return;
	
	//FIXME - probably should NULL out the array before making the assignments
	//since we check the array contents below this initialization.
	
	pic14Mnemonics[POC_ADDLW]   = &pciADDLW;
	pic14Mnemonics[POC_ADDWF]   = &pciADDWF;
	pic14Mnemonics[POC_ADDFW]   = &pciADDFW;
	pic14Mnemonics[POC_ANDLW]   = &pciANDLW;
	pic14Mnemonics[POC_ANDWF]   = &pciANDWF;
	pic14Mnemonics[POC_ANDFW]   = &pciANDFW;
	pic14Mnemonics[POC_BCF]     = &pciBCF;
	pic14Mnemonics[POC_BSF]     = &pciBSF;
	pic14Mnemonics[POC_BTFSC]   = &pciBTFSC;
	pic14Mnemonics[POC_BTFSS]   = &pciBTFSS;
	pic14Mnemonics[POC_CALL]    = &pciCALL;
	pic14Mnemonics[POC_COMF]    = &pciCOMF;
	pic14Mnemonics[POC_COMFW]   = &pciCOMFW;
	pic14Mnemonics[POC_CLRF]    = &pciCLRF;
	pic14Mnemonics[POC_CLRW]    = &pciCLRW;
	pic14Mnemonics[POC_CLRWDT]  = &pciCLRWDT;
	pic14Mnemonics[POC_DECF]    = &pciDECF;
	pic14Mnemonics[POC_DECFW]   = &pciDECFW;
	pic14Mnemonics[POC_DECFSZ]  = &pciDECFSZ;
	pic14Mnemonics[POC_DECFSZW] = &pciDECFSZW;
	pic14Mnemonics[POC_GOTO]    = &pciGOTO;
	pic14Mnemonics[POC_INCF]    = &pciINCF;
	pic14Mnemonics[POC_INCFW]   = &pciINCFW;
	pic14Mnemonics[POC_INCFSZ]  = &pciINCFSZ;
	pic14Mnemonics[POC_INCFSZW] = &pciINCFSZW;
	pic14Mnemonics[POC_IORLW]   = &pciIORLW;
	pic14Mnemonics[POC_IORWF]   = &pciIORWF;
	pic14Mnemonics[POC_IORFW]   = &pciIORFW;
	pic14Mnemonics[POC_MOVF]    = &pciMOVF;
	pic14Mnemonics[POC_MOVFW]   = &pciMOVFW;
	pic14Mnemonics[POC_MOVLW]   = &pciMOVLW;
	pic14Mnemonics[POC_MOVWF]   = &pciMOVWF;
	pic14Mnemonics[POC_NOP]     = &pciNOP;
	pic14Mnemonics[POC_RETFIE]  = &pciRETFIE;
	pic14Mnemonics[POC_RETLW]   = &pciRETLW;
	pic14Mnemonics[POC_RETURN]  = &pciRETURN;
	pic14Mnemonics[POC_RLF]     = &pciRLF;
	pic14Mnemonics[POC_RLFW]    = &pciRLFW;
	pic14Mnemonics[POC_RRF]     = &pciRRF;
	pic14Mnemonics[POC_RRFW]    = &pciRRFW;
	pic14Mnemonics[POC_SUBLW]   = &pciSUBLW;
	pic14Mnemonics[POC_SUBWF]   = &pciSUBWF;
	pic14Mnemonics[POC_SUBFW]   = &pciSUBFW;
	pic14Mnemonics[POC_SWAPF]   = &pciSWAPF;
	pic14Mnemonics[POC_SWAPFW]  = &pciSWAPFW;
	pic14Mnemonics[POC_TRIS]    = &pciTRIS;
	pic14Mnemonics[POC_XORLW]   = &pciXORLW;
	pic14Mnemonics[POC_XORWF]   = &pciXORWF;
	pic14Mnemonics[POC_XORFW]   = &pciXORFW;
	pic14Mnemonics[POC_BANKSEL] = &pciBANKSEL;
	pic14Mnemonics[POC_PAGESEL] = &pciPAGESEL;

	/* Enhanced instruction set. */

	pic14Mnemonics[POC_ADDFSR]    = &pciADDFSR;
	pic14Mnemonics[POC_ADDWFC]    = &pciADDWFC;
	pic14Mnemonics[POC_ADDFWC]    = &pciADDFWC;
	pic14Mnemonics[POC_ASRF]      = &pciASRF;
	pic14Mnemonics[POC_ASRFW]     = &pciASRFW;
	pic14Mnemonics[POC_BRA]       = &pciBRA;
	pic14Mnemonics[POC_BRW]       = &pciBRW;
	pic14Mnemonics[POC_CALLW]     = &pciCALLW;
	pic14Mnemonics[POC_LSLF]      = &pciLSLF;
	pic14Mnemonics[POC_LSLFW]     = &pciLSLFW;
	pic14Mnemonics[POC_LSRF]      = &pciLSRF;
	pic14Mnemonics[POC_LSRFW]     = &pciLSRFW;
	pic14Mnemonics[POC_MOVIW]     = &pciMOVIW;
	pic14Mnemonics[POC_MOVIW_K]   = &pciMOVIW_K;
	pic14Mnemonics[POC_MOVLB]     = &pciMOVLB;
	pic14Mnemonics[POC_MOVLP]     = &pciMOVLP;
	pic14Mnemonics[POC_MOVWI]     = &pciMOVWI;
	pic14Mnemonics[POC_MOVWI_K]   = &pciMOVWI_K;
	pic14Mnemonics[POC_RESET]     = &pciRESET;
	pic14Mnemonics[POC_SUBWFB]    = &pciSUBWFB;
	pic14Mnemonics[POC_SUBWFBW]   = &pciSUBWFBW;

	
	for(i=0; i<MAX_PIC14MNEMONICS; i++)
		if(pic14Mnemonics[i])
			hTabAddItem(&pic14MnemonicsHash, mnem2key((unsigned char *)pic14Mnemonics[i]->mnemonic), pic14Mnemonics[i]);
		pci = hTabFirstItem(pic14MnemonicsHash, &key);
		
		while(pci) {
			DFPRINTF((stderr, "element %d key %d, mnem %s\n",i++,key,pci->mnemonic));
			pci = hTabNextItem(pic14MnemonicsHash, &key);
		}
		
		mnemonics_initialized = 1;
}

int getpCode(const char *mnem, unsigned dest)
{
	
	pCodeInstruction *pci;
	int key = mnem2key((const unsigned char *)mnem);
	
	if(!mnemonics_initialized)
		pic14initMnemonics();
	
	pci = hTabFirstItemWK(pic14MnemonicsHash, key);
	
	while(pci) {
		
		if(STRCASECMP(pci->mnemonic, mnem) == 0) {
			if((pci->num_ops <= 1) || (pci->isModReg == dest) || (pci->isBitInst))
				return(pci->op);
		}
		
		pci = hTabNextItemWK (pic14MnemonicsHash);
		
	}
	
	return -1;
}

/*-----------------------------------------------------------------*
* pic14initpCodePeepCommands
*
*-----------------------------------------------------------------*/
void pic14initpCodePeepCommands(void)
{
	
	int key, i;
	peepCommand *pcmd;
	
	i = 0;
	do {
		hTabAddItem(&pic14pCodePeepCommandsHash, 
			mnem2key((const unsigned char *)peepCommands[i].cmd), &peepCommands[i]);
		i++;
	} while (peepCommands[i].cmd);
	
	pcmd = hTabFirstItem(pic14pCodePeepCommandsHash, &key);
	
	while(pcmd) {
		//fprintf(stderr, "peep command %s  key %d\n",pcmd->cmd,pcmd->id);
		pcmd = hTabNextItem(pic14pCodePeepCommandsHash, &key);
	}
	
}

/*-----------------------------------------------------------------
*
*
*-----------------------------------------------------------------*/

int getpCodePeepCommand(const char *cmd)
{
	
	peepCommand *pcmd;
	int key = mnem2key((const unsigned char *)cmd);
	
	
	pcmd = hTabFirstItemWK(pic14pCodePeepCommandsHash, key);
	
	while(pcmd) {
		// fprintf(stderr," comparing %s to %s\n",pcmd->cmd,cmd);
		if(STRCASECMP(pcmd->cmd, cmd) == 0) {
			return pcmd->id;
		}
		
		pcmd = hTabNextItemWK (pic14pCodePeepCommandsHash);
		
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

void pBlockConvert2ISR(pBlock *pb)
{
	if(!pb)
		return;

	if(pb->cmemmap)
		pb->cmemmap = NULL;

	pb->dbName = 'I';
}

/*-----------------------------------------------------------------*/
/* movepBlock2Head - given the dbname of a pBlock, move all        */
/*                   instances to the front of the doubly linked   */
/*                   list of pBlocks                               */
/*-----------------------------------------------------------------*/

void movepBlock2Head(char dbName)
{
	pBlock *pb;

	if (!the_pFile)
		return;

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

void copypCode(FILE *of, char dbName)
{
	pBlock *pb;

	if(!of || !the_pFile)
		return;

	for(pb = the_pFile->pbHead; pb; pb = pb->next) {
		if(getpBlock_dbName(pb) == dbName) {
			pBlockStats(of,pb);
			printpBlock(of,pb);
			fprintf (of, "\n");
		}
	}
}

void resetpCodeStatistics (void)
{
  pcode_insns = pcode_doubles = 0;
}

void dumppCodeStatistics (FILE *of)
{
	/* dump statistics */
	fprintf (of, "\n");
	fprintf (of, ";\tcode size estimation:\n");
	fprintf (of, ";\t%5u+%5u = %5u instructions (%5u byte)\n", pcode_insns, pcode_doubles, pcode_insns + pcode_doubles, 2*(pcode_insns + 2*pcode_doubles));
	fprintf (of, "\n");
}

void pcode_test(void)
{
	DFPRINTF((stderr,"pcode is alive!\n"));

	//initMnemonics();

	if(the_pFile) {

		pBlock *pb;
		FILE *pFile;
		char buffer[100];

		/* create the file name */
                SNPRINTF(buffer, sizeof(buffer), "%s.p", dstFileName);

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
			printpBlock(pFile,pb);
		}
	}
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

	if (pcop->type == PO_GPR_BIT) {
		const char *name = pcop->name;
		if (!name) 
			name = PCOR(pcop)->r->name;
		if (strcmp(name, pc_status.pcop.name) == 0)
		{
			switch(PCORB(pcop)->bit) {
			case PIC_C_BIT:
				return PCC_C;
			case PIC_DC_BIT:
				return PCC_DC;
			case PIC_Z_BIT:
				return PCC_Z;
			}
		}
	}

	return 0;
}

/*-----------------------------------------------------------------*/
/* newpCode - create and return a newly initialized pCode          */
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
pCode *newpCode (PIC_OPCODE op, pCodeOp *pcop)
{
	pCodeInstruction *pci ;

	if(!mnemonics_initialized)
		pic14initMnemonics();

	pci = Safe_alloc(sizeof(pCodeInstruction));

	if((op>=0) && (op < MAX_PIC14MNEMONICS) && pic14Mnemonics[op]) {
		memcpy(pci, pic14Mnemonics[op], sizeof(pCodeInstruction));
		pci->pc.id = PCodeID();
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
/* newpCodeWild - create a "wild" as in wild card pCode            */
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

pCode *newpCodeWild(int pCodeID, pCodeOp *optional_operand, pCodeOp *optional_label)
{
	pCodeWild *pcw;

	pcw = Safe_alloc(sizeof(pCodeWild));

	pcw->pci.pc.type = PC_WILD;
	pcw->pci.pc.prev = pcw->pci.pc.next = NULL;
	pcw->id = PCodeID();
	pcw->pci.from = pcw->pci.to = pcw->pci.label = NULL;
	pcw->pci.pc.pb = NULL;

	pcw->pci.pc.destruct = genericDestruct;
	pcw->pci.pc.print = genericPrint;

	pcw->id = pCodeID;              // this is the 'n' in %n
	pcw->operand = optional_operand;
	pcw->label   = optional_label;

	pcw->mustBeBitSkipInst = 0;
	pcw->mustNotBeBitSkipInst = 0;
	pcw->invertBitSkipInst = 0;

	return ((pCode *)pcw);
}

/*-----------------------------------------------------------------*/
/* newPcodeCharP - create a new pCode from a char string           */
/*-----------------------------------------------------------------*/

pCode *newpCodeCharP(const char *cP)
{
	pCodeComment *pcc;

	pcc = Safe_alloc(sizeof(pCodeComment));

	pcc->pc.type = PC_COMMENT;
	pcc->pc.prev = pcc->pc.next = NULL;
	pcc->pc.id = PCodeID();
	//pcc->pc.from = pcc->pc.to = pcc->pc.label = NULL;
	pcc->pc.pb = NULL;

	pcc->pc.destruct = genericDestruct;
	pcc->pc.print = genericPrint;

	if(cP)
		pcc->comment = Safe_strdup(cP);
	else
		pcc->comment = NULL;

	return ((pCode *)pcc);
}

/*-----------------------------------------------------------------*/
/* newpCodeFunction -                                              */
/*-----------------------------------------------------------------*/

pCode *newpCodeFunction(const char *mod, const char *f, int isPublic, int isInterrupt)
{
	pCodeFunction *pcf;

	pcf = Safe_alloc(sizeof(pCodeFunction));

	pcf->pc.type = PC_FUNCTION;
	pcf->pc.prev = pcf->pc.next = NULL;
	pcf->pc.id   = PCodeID();
	//pcf->pc.from = pcf->pc.to = pcf->pc.label = NULL;
	pcf->pc.pb   = NULL;

	pcf->pc.destruct = genericDestruct;
	pcf->pc.print    = pCodePrintFunction;

	pcf->ncalled = 0;

	pcf->modname = (mod != NULL) ? Safe_strdup(mod) : NULL;
	pcf->fname   = (f != NULL)   ? Safe_strdup(f)   : NULL;

	pcf->isPublic    = (unsigned int)isPublic;
	pcf->isInterrupt = (unsigned int)isInterrupt;

	return ((pCode *)pcf);
}

/*-----------------------------------------------------------------*/
/* newpCodeFlow                                                    */
/*-----------------------------------------------------------------*/
static void destructpCodeFlow(pCode *pc)
{
	if(!pc || !isPCFL(pc))
		return;

		/*
		if(PCFL(pc)->from)
		if(PCFL(pc)->to)
	*/
	unlinkpCode(pc);

	deleteSet(&PCFL(pc)->registers);
	deleteSet(&PCFL(pc)->from);
	deleteSet(&PCFL(pc)->to);
	free(pc);
}

static pCode *newpCodeFlow(void )
{
	pCodeFlow *pcflow;

	pcflow = Safe_alloc(sizeof(pCodeFlow));

	pcflow->pc.type = PC_FLOW;
	pcflow->pc.prev = pcflow->pc.next = NULL;
	pcflow->pc.pb = NULL;

	pcflow->pc.destruct = destructpCodeFlow;
	pcflow->pc.print = genericPrint;

	pcflow->pc.seq = GpcFlowSeq++;

	pcflow->from = pcflow->to = NULL;

	pcflow->inCond = PCC_NONE;
	pcflow->outCond = PCC_NONE;

	pcflow->firstBank = 'U'; /* Undetermined */
	pcflow->lastBank = 'U'; /* Undetermined */

	pcflow->FromConflicts = 0;
	pcflow->ToConflicts = 0;

	pcflow->end = NULL;

	pcflow->registers = newSet();

	return ((pCode *)pcflow);
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static pCodeFlowLink *newpCodeFlowLink(pCodeFlow *pcflow)
{
	pCodeFlowLink *pcflowLink;

	pcflowLink = Safe_alloc(sizeof(pCodeFlowLink));

	pcflowLink->pcflow = pcflow;
	pcflowLink->bank_conflict = 0;

	return pcflowLink;
}

/*-----------------------------------------------------------------*/
/* newpCodeCSource - create a new pCode Source Symbol              */
/*-----------------------------------------------------------------*/

pCode *newpCodeCSource(int ln, const char *f, const char *l)
{
	pCodeCSource *pccs;

	pccs = Safe_alloc(sizeof(pCodeCSource));

	pccs->pc.type = PC_CSOURCE;
	pccs->pc.prev = pccs->pc.next = NULL;
	pccs->pc.id = PCodeID();
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
/*                        added by VR 6-Jun-2003                   */
/*******************************************************************/

pCode *newpCodeAsmDir(const char *asdir, const char *argfmt, ...)
{
  pCodeAsmDir *pcad;
  va_list ap;
  char buffer[512];
  char *lbp=buffer;

  pcad = Safe_alloc(sizeof(pCodeAsmDir));
  pcad->pci.pc.type = PC_ASMDIR;
  pcad->pci.pc.prev = pcad->pci.pc.next = NULL;
  pcad->pci.pc.pb = NULL;
  pcad->pci.pc.destruct = genericDestruct;
  pcad->pci.pc.print = genericPrint;

  if(asdir && *asdir) {
    while(isspace((unsigned char)*asdir)) asdir++;	// strip any white space from the beginning

    pcad->directive = Safe_strdup(asdir);
  }

  va_start(ap, argfmt);

  memset(buffer, 0, sizeof(buffer));
  if(argfmt && *argfmt)
    vsprintf(buffer, argfmt, ap);

  va_end(ap);

  while(isspace((unsigned char)*lbp)) lbp++;

  if(lbp && *lbp)
    pcad->arg = Safe_strdup(lbp);

  return ((pCode *)pcad);
}

/*-----------------------------------------------------------------*/
/* pCodeLabelDestruct - free memory used by a label.               */
/*-----------------------------------------------------------------*/
static void pCodeLabelDestruct(pCode *pc)
{
	if(!pc)
		return;

	if((pc->type == PC_LABEL) && PCL(pc)->label)
		free(PCL(pc)->label);

	free(pc);
}

pCode *newpCodeLabel(const char *name, int key)
{
	const char *s;
	pCodeLabel *pcl;

	pcl = Safe_alloc(sizeof(pCodeLabel));

	pcl->pc.type = PC_LABEL;
	pcl->pc.prev = pcl->pc.next = NULL;
	pcl->pc.id = PCodeID();
	//pcl->pc.from = pcl->pc.to = pcl->pc.label = NULL;
	pcl->pc.pb = NULL;

	pcl->pc.destruct = pCodeLabelDestruct;
	pcl->pc.print = pCodePrintLabel;

	pcl->key = key;

	pcl->label = NULL;
	if(key>0) {
		SNPRINTF(buffer, sizeof(buffer), "_%05d_DS_", key);
		s = buffer;
	} else {
/*		SNPRINTF(buffer, sizeof(buffer), "%s:", name);
		s = buffer;*/
		s = name;
	}

	if(s)
		pcl->label = Safe_strdup(s);

	//fprintf(stderr,"newpCodeLabel: key=%d, name=%s\n",key, ((s)?s:""));
	return ((pCode *)pcl);
}


/*-----------------------------------------------------------------*/
/* newpBlock - create and return a pointer to a new pBlock         */
/*-----------------------------------------------------------------*/
static pBlock *newpBlock(void)
{
	pBlock *PpB;

	PpB = Safe_alloc(sizeof(pBlock));
	PpB->next = PpB->prev = NULL;

	PpB->function_entries = PpB->function_exits = PpB->function_calls = NULL;
	PpB->tregisters = NULL;
	PpB->visited = 0;
	PpB->FlowTree = NULL;

	return PpB;
}

/*-----------------------------------------------------------------*/
/* newpCodeChain - create a new chain of pCodes                    */
/*-----------------------------------------------------------------*
*
*  This function will create a new pBlock and the pointer to the
*  pCode that is passed in will be the first pCode in the block.
*-----------------------------------------------------------------*/


pBlock *newpCodeChain(memmap *cm,char c, pCode *pc)
{
	pBlock *pB  = newpBlock();

	pB->pcHead  = pB->pcTail = pc;
	pB->cmemmap = cm;
	pB->dbName  = c;

	return pB;
}

/*-----------------------------------------------------------------*/
/* newpCodeOpLabel - Create a new label given the key              */
/*  Note, a negative key means that the label is part of wild card */
/*  (and hence a wild card label) used in the pCodePeep            */
/*   optimizations).                                               */
/*-----------------------------------------------------------------*/

pCodeOp *newpCodeOpLabel(const char *name, int key)
{
	const char *s;
	static int label_key = -1;

	pCodeOp *pcop;

	pcop = Safe_alloc(sizeof(pCodeOpLabel));
	pcop->type = PO_LABEL;

	pcop->name = NULL;

	if(key>0) {
		SNPRINTF(buffer, sizeof(buffer), "_%05d_DS_", key);
		s = buffer;
	}
	else {
		s = name;
		key = label_key--;
	}

	PCOLAB(pcop)->offset = 0;
	if(s)
		pcop->name = Safe_strdup(s);

	((pCodeOpLabel *)pcop)->key = key;

	//fprintf(stderr,"newpCodeOpLabel: key=%d, name=%s\n",key,((s)?s:""));
	return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *newpCodeOpLit(int lit)
{
	pCodeOp *pcop;

	pcop = Safe_alloc(sizeof(pCodeOpLit));
	pcop->type = PO_LITERAL;

	pcop->name = NULL;
	if(lit>=0) {
		SNPRINTF(buffer, sizeof(buffer),"0x%02x", (unsigned char)lit);
		pcop->name = Safe_strdup(buffer);
	}

	((pCodeOpLit *)pcop)->lit = (unsigned char)lit;

	return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *newpCodeOpImmd(const char *name, int offset, int index, int code_space, int is_func)
{
	pCodeOp *pcop;

	pcop = Safe_alloc(sizeof(pCodeOpImmd));
	pcop->type = PO_IMMEDIATE;
	if(name) {
		reg_info *r = NULL;
		pcop->name = Safe_strdup(name);

		if(!is_func) 
			r = dirregWithName(name);

		PCOI(pcop)->r = r;
		if(r) {
			//fprintf(stderr, " newpCodeOpImmd reg %s exists\n",name);
			PCOI(pcop)->rIdx = r->rIdx;
		} else {
			//fprintf(stderr, " newpCodeOpImmd reg %s doesn't exist\n",name);
			PCOI(pcop)->rIdx = -1;
		}
		//fprintf(stderr,"%s %s %d\n",__FUNCTION__,name,offset);
	} else {
		pcop->name = NULL;
	}

	PCOI(pcop)->index = index;
	PCOI(pcop)->offset = offset;
	PCOI(pcop)->_const = code_space;
	PCOI(pcop)->_function = is_func;

	return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *newpCodeOpWild(int id, pCodeWildBlock *pcwb, pCodeOp *subtype)
{
	pCodeOp *pcop;

	if(!pcwb || !subtype) {
		fprintf(stderr, "Wild opcode declaration error: %s-%d\n",__FILE__,__LINE__);
		exit(1);
	}

	pcop = Safe_alloc(sizeof(pCodeOpWild));
	pcop->type = PO_WILD;
	SNPRINTF(buffer, sizeof(buffer), "%%%d", id);
	pcop->name = Safe_strdup(buffer);

	PCOW(pcop)->id = id;
	PCOW(pcop)->pcwb = pcwb;
	PCOW(pcop)->subtype = subtype;
	PCOW(pcop)->matched = NULL;

	return pcop;
}

/*-----------------------------------------------------------------*/
/* Find a symbol with matching name                                */
/*-----------------------------------------------------------------*/
static symbol *symFindWithName(memmap * map, const char *name)
{
	symbol *sym;
	
	for (sym = setFirstItem(map->syms); sym; sym = setNextItem (map->syms)) {
		if (sym->rname && (strcmp(sym->rname,name)==0))
			return sym;
	}
	return 0;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pCodeOp *newpCodeOpBit(const char *name, int ibit, int inBitSpace)
{
	pCodeOp *pcop;
	struct reg_info *r = 0;

	pcop = Safe_alloc(sizeof(pCodeOpRegBit));
	pcop->type = PO_GPR_BIT;

	PCORB(pcop)->bit = ibit;
	PCORB(pcop)->inBitSpace = inBitSpace;

        if (name) r = regFindWithName(name);
	if (name && !r) {
		// Register has not been allocated - check for symbol information
		symbol *sym;
		sym = symFindWithName(bit, name);
		if (!sym) sym = symFindWithName(sfrbit, name);
		if (!sym) sym = symFindWithName(sfr, name);
		if (!sym) sym = symFindWithName(reg, name);
		// Hack to fix accesses to _INTCON_bits (e.g. GIE=0), see #1579535.
		// XXX: This ignores nesting levels, but works for globals...
		if (!sym) sym = findSym(SymbolTab, NULL, name);
		if (!sym && name && name[0] == '_') sym = findSym(SymbolTab, NULL, &name[1]);
		if (sym) {
			r = allocNewDirReg(sym->etype,name);
		}
	}
	if (r) {
		pcop->name = NULL;
		PCOR(pcop)->r = r;
		PCOR(pcop)->rIdx = r->rIdx;
	} else if (name) {
		pcop->name = Safe_strdup(name);   
		PCOR(pcop)->r = NULL;
		PCOR(pcop)->rIdx = 0;
	} else {
                //fprintf(stderr, "Unnamed register duplicated for bit-access?!? Hope for the best ...\n");
        }
	return pcop;
}

/*-----------------------------------------------------------------*
* pCodeOp *newpCodeOpReg(int rIdx) - allocate a new register
*
* If rIdx >=0 then a specific register from the set of registers
* will be selected. If rIdx <0, then a new register will be searched
* for.
*-----------------------------------------------------------------*/

static pCodeOp *newpCodeOpReg(int rIdx)
{
	pCodeOp *pcop;
	
	pcop = Safe_alloc(sizeof(pCodeOpReg));
	
	pcop->name = NULL;
	
	if(rIdx >= 0) {
		PCOR(pcop)->rIdx = rIdx;
		PCOR(pcop)->r = pic14_regWithIdx(rIdx);
	} else {
		PCOR(pcop)->r = pic14_findFreeReg(REG_GPR);
		
		if(PCOR(pcop)->r)
			PCOR(pcop)->rIdx = PCOR(pcop)->r->rIdx;
	}
	
	if(PCOR(pcop)->r)
		pcop->type = PCOR(pcop)->r->pc_type;
	
	return pcop;
}

pCodeOp *newpCodeOpRegFromStr(const char *name)
{
	pCodeOp *pcop;
	
	pcop = Safe_alloc(sizeof(pCodeOpReg));
	PCOR(pcop)->r = allocRegByName(name, 1);
	PCOR(pcop)->rIdx = PCOR(pcop)->r->rIdx;
	pcop->type = PCOR(pcop)->r->pc_type;
	pcop->name = PCOR(pcop)->r->name;
	
	return pcop;
}

static pCodeOp *newpCodeOpStr(const char *name)
{
	pCodeOp *pcop;
	
	pcop = Safe_alloc(sizeof(pCodeOpStr));
	pcop->type = PO_STR;
	pcop->name = Safe_strdup(name);   
	
	PCOS(pcop)->isPublic = 0;
	
	return pcop;
}


/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

pCodeOp *newpCodeOp(const char *name, PIC_OPTYPE type)
{
	pCodeOp *pcop;
	
	switch(type) {
	case PO_BIT:
	case PO_GPR_BIT:
		pcop = newpCodeOpBit(name, -1, 0);
		break;
		
	case PO_LITERAL:
		pcop = newpCodeOpLit(-1);
		break;
		
	case PO_LABEL:
		pcop = newpCodeOpLabel(NULL,-1);
		break;
		
	case PO_GPR_TEMP:
		pcop = newpCodeOpReg(-1);
		break;
		
	case PO_GPR_POINTER:
	case PO_GPR_REGISTER:
		if(name)
			pcop = newpCodeOpRegFromStr(name);
		else
			pcop = newpCodeOpReg(-1);
		break;
		
	case PO_STR:
		pcop = newpCodeOpStr(name);
		break;
		
	default:
		pcop = Safe_alloc(sizeof(pCodeOp));
		pcop->type = type;
		pcop->name = (name != NULL) ? Safe_strdup(name) : NULL;
	}
	
	return pcop;
}

/*-----------------------------------------------------------------*/
/* addpCode2pBlock - place the pCode into the pBlock linked list   */
/*-----------------------------------------------------------------*/
void addpCode2pBlock(pBlock *pb, pCode *pc)
{
	
	if(!pb || !pc)
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
/* addpBlock - place a pBlock into the pFile                       */
/*-----------------------------------------------------------------*/
void addpBlock(pBlock *pb)
{
	// fprintf(stderr," Adding pBlock: dbName =%c\n",getpBlock_dbName(pb));

	if(!the_pFile) {
		/* First time called, we'll pass through here. */
		//_ALLOC(the_pFile,sizeof(pFile));
		the_pFile = Safe_alloc(sizeof(pFile));
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

void printpCode(FILE *of, pCode *pc)
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
/* printpBlock - write the contents of a pBlock to a file          */
/*-----------------------------------------------------------------*/

void printpBlock(FILE *of, pBlock *pb)
{
	pCode *pc;

	if(!pb)
		return;

	if(!of)
		of = stderr;

	for(pc = pb->pcHead; pc; pc = pc->next) {
                if(isPCF(pc) && PCF(pc)->fname && !PCF(pc)->isInterrupt) {
			fprintf(of, "S_%s_%s\tcode\n", PCF(pc)->modname, PCF(pc)->fname);
		}
		printpCode(of,pc);

		if (isPCI(pc))
		{
			if (isPCI(pc) && (PCI(pc)->op == POC_PAGESEL || PCI(pc)->op == POC_BANKSEL)) {
				pcode_doubles++;
			} else {
				pcode_insns++;
			}
		}
	} // for
}

/*-----------------------------------------------------------------*/
/*                                                                 */
/*       pCode processing                                          */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*-----------------------------------------------------------------*/

void unlinkpCode(pCode *pc)
{
	if(pc) {
#ifdef PCODE_DEBUG
		fprintf(stderr,"Unlinking: ");
		printpCode(stderr, pc);
#endif
		if(pc->prev) 
			pc->prev->next = pc->next;
		if(pc->next)
			pc->next->prev = pc->prev;

#if 0
		/* RN: I believe this should be right here, but this did not
		 *     cure the bug I was hunting... */
		/* must keep labels -- attach to following instruction */
		if (isPCI(pc) && PCI(pc)->label && pc->next)
		{
		  pCodeInstruction *pcnext = PCI(findNextInstruction (pc->next));
		  if (pcnext)
		  {
		    pBranchAppend (pcnext->label, PCI(pc)->label);
		  }
		}
#endif
		pc->prev = pc->next = NULL;
	}
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

static void genericDestruct(pCode *pc)
{
	unlinkpCode(pc);

	if(isPCI(pc)) {
	/* For instructions, tell the register (if there's one used)
		* that it's no longer needed */
		reg_info *reg = getRegFromInstruction(pc);
		if(reg)
			deleteSetItem (&(reg->reglives.usedpCodes),pc);
	}

	/* Instead of deleting the memory used by this pCode, mark
	* the object as bad so that if there's a pointer to this pCode
	* dangling around somewhere then (hopefully) when the type is
	* checked we'll catch it.
	*/

	pc->type = PC_BAD;

	addpCode2pBlock(pb_dead_pcodes, pc);

	//free(pc);
}


/*-----------------------------------------------------------------*/
/*  Copies the pCodeInstruction flow pointer from source pCode     */
/*-----------------------------------------------------------------*/
static void CopyFlow(pCodeInstruction *pcd, pCode *pcs) {
	pCode *p;
	pCodeFlow *pcflow = 0;
	for (p=pcs; p; p=p->prev) {
		if (isPCI(p)) {
			pcflow = PCI(p)->pcflow;
			break;
		}
		if (isPCF(p)) {
			pcflow = (pCodeFlow*)p;
			break;
		}
	}
	PCI(pcd)->pcflow = pcflow;
}

/*-----------------------------------------------------------------*/
/*  pCodeInsertAfter - splice in the pCode chain starting with pc2 */
/*                     into the pCode chain containing pc1         */
/*-----------------------------------------------------------------*/
void pCodeInsertAfter(pCode *pc1, pCode *pc2)
{
	if(!pc1 || !pc2)
		return;

	pc2->next = pc1->next;
	if(pc1->next)
		pc1->next->prev = pc2;

	pc2->pb = pc1->pb;
	pc2->prev = pc1;
	pc1->next = pc2;

	/* If this is an instrution type propogate the flow */
	if (isPCI(pc2))
		CopyFlow(PCI(pc2),pc1);
}

/*------------------------------------------------------------------*/
/*  pCodeInsertBefore - splice in the pCode chain starting with pc2 */
/*                      into the pCode chain containing pc1         */
/*------------------------------------------------------------------*/
void pCodeInsertBefore(pCode *pc1, pCode *pc2)
{
	if(!pc1 || !pc2)
		return;

	pc2->prev = pc1->prev;
	if(pc1->prev)
		pc1->prev->next = pc2;

	pc2->pb = pc1->pb;
	pc2->next = pc1;
	pc1->prev = pc2;

	/* If this is an instrution type propogate the flow */
	if (isPCI(pc2))
		CopyFlow(PCI(pc2),pc1);
}

/*-----------------------------------------------------------------*/
/* pCodeOpCopy - copy a pcode operator                             */
/*-----------------------------------------------------------------*/
pCodeOp *pCodeOpCopy(pCodeOp *pcop)
{
	pCodeOp *pcopnew=NULL;

	if(!pcop)
		return NULL;

	switch(pcop->type) { 
	case PO_NONE:
	case PO_STR:
		pcopnew = Safe_malloc(sizeof(pCodeOp));
		memcpy(pcopnew, pcop, sizeof(pCodeOp));
		break;

	case PO_W:
	case PO_STATUS:
	case PO_FSR:
	case PO_INDF:
	case PO_INTCON:
	case PO_GPR_REGISTER:
	case PO_GPR_TEMP:
	case PO_GPR_POINTER:
	case PO_SFR_REGISTER:
	case PO_PCL:
	case PO_PCLATH:
	case PO_DIR:
		//DFPRINTF((stderr,"pCodeOpCopy GPR register\n"));
		pcopnew = Safe_malloc(sizeof(pCodeOpReg));
		memcpy(pcopnew, pcop, sizeof(pCodeOpReg));
		DFPRINTF((stderr," register index %d\n", PCOR(pcop)->r->rIdx));
		break;

	case PO_LITERAL:
		//DFPRINTF((stderr,"pCodeOpCopy lit\n"));
		pcopnew = Safe_malloc(sizeof(pCodeOpLit));
		memcpy(pcopnew, pcop, sizeof(pCodeOpLit));
		break;

	case PO_IMMEDIATE:
		pcopnew = Safe_malloc(sizeof(pCodeOpImmd));
		memcpy(pcopnew, pcop, sizeof(pCodeOpImmd));
		break;

	case PO_GPR_BIT:
	case PO_CRY:
	case PO_BIT:
		//DFPRINTF((stderr,"pCodeOpCopy bit\n"));
		pcopnew = Safe_malloc(sizeof(pCodeOpRegBit));
		memcpy(pcopnew, pcop, sizeof(pCodeOpRegBit));
		break;

	case PO_LABEL:
		//DFPRINTF((stderr,"pCodeOpCopy label\n"));
		pcopnew = Safe_malloc(sizeof(pCodeOpLabel));
		memcpy(pcopnew, pcop, sizeof(pCodeOpLabel));
		break;

	case PO_WILD:
		/* Here we expand the wild card into the appropriate type: */
		/* By recursively calling pCodeOpCopy */
		//DFPRINTF((stderr,"pCodeOpCopy wild\n"));
		if(PCOW(pcop)->matched)
			pcopnew = pCodeOpCopy(PCOW(pcop)->matched);
		else {
			// Probably a label
			pcopnew = pCodeOpCopy(PCOW(pcop)->subtype);
			pcopnew->name = Safe_strdup(PCOW(pcop)->pcwb->vars[PCOW(pcop)->id]);
			//DFPRINTF((stderr,"copied a wild op named %s\n",pcopnew->name));
		}

		return pcopnew;
		break;

	default:
		assert ( !"unhandled pCodeOp type copied" );
		break;
	} // switch

	pcopnew->name = (pcop->name != NULL) ? Safe_strdup(pcop->name) : NULL;

	return pcopnew;
}

/*-----------------------------------------------------------------*/
/* popCopyReg - copy a pcode operator                              */
/*-----------------------------------------------------------------*/
pCodeOp *popCopyReg(pCodeOpReg *pc)
{
	pCodeOpReg *pcor;

	pcor = Safe_alloc(sizeof(pCodeOpReg));
	pcor->pcop.type = pc->pcop.type;
	if(pc->pcop.name) {
		if(!(pcor->pcop.name = Safe_strdup(pc->pcop.name)))
			fprintf(stderr,"oops %s %d",__FILE__,__LINE__);
	} else
		pcor->pcop.name = NULL;

	if (pcor->pcop.type == PO_IMMEDIATE){
		PCOL(pcor)->lit = PCOL(pc)->lit;
	} else {
		pcor->r = pc->r;
		pcor->rIdx = pc->rIdx;
		if (pcor->r)
			pcor->r->wasUsed=1;
	}
	//DEBUGpic14_emitcode ("; ***","%s  , copying %s, rIdx=%d",__FUNCTION__,pc->pcop.name,pc->rIdx);

	return PCOP(pcor);
}

/*-----------------------------------------------------------------*/
/* pCodeInstructionCopy - copy a pCodeInstructionCopy              */
/*-----------------------------------------------------------------*/
pCode *pCodeInstructionCopy(pCodeInstruction *pci,int invert)
{
	pCodeInstruction *new_pci;

	if(invert)
		new_pci = PCI(newpCode(pci->inverted_op,pci->pcop));
	else
		new_pci = PCI(newpCode(pci->op,pci->pcop));

	new_pci->pc.pb = pci->pc.pb;
	new_pci->from = pci->from;
	new_pci->to   = pci->to;
	new_pci->label = pci->label;
	new_pci->pcflow = pci->pcflow;

	return PCODE(new_pci);
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
void pCodeDeleteChain(pCode *f,pCode *t)
{
	pCode *pc;

	while(f && f!=t) {
		DFPRINTF((stderr,"delete pCode:\n"));
		pc = f->next;
		//f->print(stderr,f);
		//f->delete(f);  this dumps core...
		f = pc;
	}
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
char *get_op(pCodeOp *pcop,char *buffer, size_t size)
{
	reg_info *r;
	static char b[50];
	char *s;
	int use_buffer = 1;    // copy the string to the passed buffer pointer

	if(!buffer) {
		buffer = b;
		size = sizeof(b);
		use_buffer = 0;     // Don't bother copying the string to the buffer.
	} 

	if(pcop) {
		switch(pcop->type) {
		case PO_INDF:
		case PO_FSR:
			if(use_buffer) {
				SNPRINTF(buffer,size,"%s",PCOR(pcop)->r->name);
				return buffer;
			}
			return pcop->name;
			break;
		case PO_GPR_TEMP:
			if (PCOR(pcop)->r->type == REG_STK)
				r = typeRegWithIdx(PCOR(pcop)->r->rIdx,REG_STK,1);
			else
				r = pic14_regWithIdx(PCOR(pcop)->r->rIdx);

			if(use_buffer) {
				SNPRINTF(buffer,size,"%s",r->name);
				return buffer;
			}

			return r->name;
			break;

		case PO_IMMEDIATE:
			s = buffer;
			if(PCOI(pcop)->_const) {

				if( PCOI(pcop)->offset >= 0 && PCOI(pcop)->offset<4) {
					switch(PCOI(pcop)->offset) {
					case 0:
						SNPRINTF(s,size,"low (%s+%d)",pcop->name, PCOI(pcop)->index);
						break;
					case 1:
						SNPRINTF(s,size,"high (%s+%d)",pcop->name, PCOI(pcop)->index);
						break;
					case 2:
						SNPRINTF(s,size,"0x%02x",PCOI(pcop)->_const ? GPTRTAG_CODE : GPTRTAG_DATA);
						break;
					default:
						fprintf (stderr, "PO_IMMEDIATE/_const/offset=%d\n", PCOI(pcop)->offset);
						assert ( !"offset too large" );
						SNPRINTF(s,size,"(((%s+%d) >> %d)&0xff)",
							pcop->name,
							PCOI(pcop)->index,
							8 * PCOI(pcop)->offset );
					}
				} else
					SNPRINTF(s,size,"LOW (%s+%d)",pcop->name,PCOI(pcop)->index);
			} else {
				if( !PCOI(pcop)->offset) { // && PCOI(pcc->pcop)->offset<4) 
					SNPRINTF(s,size,"(%s + %d)",
						pcop->name,
						PCOI(pcop)->index);
				} else {
					switch(PCOI(pcop)->offset) {
					case 0:
						SNPRINTF(s,size,"(%s + %d)",pcop->name, PCOI(pcop)->index);
						break;
					case 1:
						SNPRINTF(s,size,"high (%s + %d)",pcop->name, PCOI(pcop)->index);
						break;
					case 2:
						SNPRINTF(s,size,"0x%02x",PCOI(pcop)->_const ? GPTRTAG_CODE : GPTRTAG_DATA);
						break;
					default:
						fprintf (stderr, "PO_IMMEDIATE/mutable/offset=%d\n", PCOI(pcop)->offset);
						assert ( !"offset too large" );
						SNPRINTF(s,size,"((%s + %d) >> %d)&0xff",pcop->name, PCOI(pcop)->index, 8*PCOI(pcop)->offset);
						break;
					}
				}
			}
			return buffer;
			break;

		case PO_DIR:
			s = buffer;
			if( PCOR(pcop)->instance) {
				SNPRINTF(s,size,"(%s + %d)",
					pcop->name,
					PCOR(pcop)->instance );
			} else
				SNPRINTF(s,size,"%s",pcop->name);
			return buffer;
			break;

		case PO_LABEL:
			s = buffer;
			if  (pcop->name) {
				if(PCOLAB(pcop)->offset == 1)
					SNPRINTF(s,size,"HIGH(%s)",pcop->name);
				else
					SNPRINTF(s,size,"%s",pcop->name);
			}
			return buffer;
			break;

		case PO_GPR_BIT:
			if(PCOR(pcop)->r) {
				if(use_buffer) {
					SNPRINTF(buffer,size,"%s",PCOR(pcop)->r->name);
					return buffer;
				}
				return PCOR(pcop)->r->name;
			}
			/* fall through to the default case */

		default:
			if(pcop->name) {
				if(use_buffer) {
					SNPRINTF(buffer,size,"%s",pcop->name);
					return buffer;
				}
				return pcop->name;
			}
		}
	}

	printf("PIC port internal warning: (%s:%d(%s)) %s not found\n",
	  __FILE__, __LINE__, __FUNCTION__,
	  pCodeOpType(pcop));

	return "NO operand";
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static char *get_op_from_instruction( pCodeInstruction *pcc)
{
	if(pcc)
		return get_op(pcc->pcop,NULL,0);

	return ("ERROR Null: get_op_from_instruction");
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void pCodeOpPrint(FILE *of, pCodeOp *pcop)
{
	fprintf(of,"pcodeopprint- not implemented\n");
}

/*-----------------------------------------------------------------*/
/* pCode2str - convert a pCode instruction to string               */
/*-----------------------------------------------------------------*/
char *pCode2str(char *str, size_t size, pCode *pc)
{
    char *s = str;

    switch(pc->type) {

        case PC_OPCODE:

            SNPRINTF(s,size, "\t%s\t", PCI(pc)->mnemonic);
            size -= strlen(s);
            s += strlen(s);

            if( (PCI(pc)->num_ops >= 1) && (PCI(pc)->pcop)) {
                if(PCI(pc)->isBitInst) {
                    if(PCI(pc)->pcop->type == PO_GPR_BIT) {
                        char *name = PCI(pc)->pcop->name;
                        if (!name)
                            name = PCOR(PCI(pc)->pcop)->r->name;
                        if( (((pCodeOpRegBit *)(PCI(pc)->pcop))->inBitSpace) )
                            SNPRINTF(s,size,"(%s >> 3), (%s & 7)", name, name);
                        else
                            SNPRINTF(s,size,"%s,%d", name, (((pCodeOpRegBit *)(PCI(pc)->pcop))->bit)&7);
                    } else if(PCI(pc)->pcop->type == PO_GPR_BIT) {
                        SNPRINTF(s,size,"%s,%d", get_op_from_instruction(PCI(pc)),PCORB(PCI(pc)->pcop)->bit);
                    } else
                        SNPRINTF(s,size,"%s,0 ; ?bug", get_op_from_instruction(PCI(pc)));
                } else {
                    if(PCI(pc)->pcop->type == PO_GPR_BIT) {
                        if( PCI(pc)->num_ops == 2)
                            SNPRINTF(s,size,"(%s >> 3),%c",get_op_from_instruction(PCI(pc)),((PCI(pc)->isModReg) ? 'F':'W'));
                        else
                            SNPRINTF(s,size,"(1 << (%s & 7))",get_op_from_instruction(PCI(pc)));
                    } else {
                        SNPRINTF(s,size,"%s",get_op_from_instruction(PCI(pc)));
                        size -= strlen(s);
                        s += strlen(s);
                        if( PCI(pc)->num_ops == 2)
                            SNPRINTF(s,size,",%c", ( (PCI(pc)->isModReg) ? 'F':'W'));
                    }
                }
            }
            break;

        case PC_COMMENT:
            /* assuming that comment ends with a \n */
            SNPRINTF(s,size,";%s", ((pCodeComment *)pc)->comment);
            break;

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
            SNPRINTF(s,size,"%s\t.line\t%d; \"%s\"\t%s\n",(options.debug?"":";"),PCCS(pc)->line_number, PCCS(pc)->file_name, PCCS(pc)->line);
            break;
        case PC_ASMDIR:
            if(PCAD(pc)->directive) {
                SNPRINTF(s,size,"\t%s%s%s\n", PCAD(pc)->directive, PCAD(pc)->arg?"\t":"", PCAD(pc)->arg?PCAD(pc)->arg:"");
            } else if(PCAD(pc)->arg) {
                /* special case to handle inline labels without a tab */
                SNPRINTF(s,size,"%s\n", PCAD(pc)->arg);
            }
            break;

        case PC_BAD:
            SNPRINTF(s,size,";A bad pCode is being used\n");
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
    fprintf(of,";%s\n", ((pCodeComment *)pc)->comment);
    break;

  case PC_INLINE:
    fprintf(of,"%s\n", ((pCodeComment *)pc)->comment);
    break;

  case PC_OPCODE:
    // If the opcode has a label, print that first
    {
      char str[256];
      pCodeInstruction *pci = PCI(pc);
      pBranch *pbl = pci->label;
      while(pbl && pbl->pc) {
        if(pbl->pc->type == PC_LABEL)
          pCodePrintLabel(of, pbl->pc);
        pbl = pbl->next;
      }

      if(pci->cline)
        genericPrint(of,PCODE(pci->cline));


      pCode2str(str, sizeof(str), pc);

      fprintf(of,"%s",str);

      /* Debug */
      if(debug_verbose) {
        pCodeOpReg *pcor = PCOR(pci->pcop);
        fprintf(of, "\t;id=%u,key=%03x,inCond:%x,outCond:%x",pc->id,pc->seq, pci->inCond, pci->outCond);
        if(pci->pcflow)
          fprintf(of,",flow seq=%03x",pci->pcflow->pc.seq);
        if (pcor && pcor->pcop.type==PO_GPR_TEMP && !pcor->r->isFixed)
          fprintf(of,",rIdx=r0x%X",pcor->rIdx);
      }
    }
    fprintf(of,"\n");
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
    if(debug_verbose) {
      fprintf(of,";<>Start of new flow, seq=0x%x",pc->seq);
      if(PCFL(pc)->ancestor)
        fprintf(of," ancestor = 0x%x", PCODE(PCFL(pc)->ancestor)->seq);
      fprintf(of,"\n");
      fprintf(of,";  from: ");
      {
        pCodeFlowLink *link;
        for (link = setFirstItem(PCFL(pc)->from); link; link = setNextItem (PCFL(pc)->from))
	{
	  fprintf(of,"%03x ",link->pcflow->pc.seq);
	}
      }
      fprintf(of,"; to: ");
      {
        pCodeFlowLink *link;
        for (link = setFirstItem(PCFL(pc)->to); link; link = setNextItem (PCFL(pc)->to))
	{
	  fprintf(of,"%03x ",link->pcflow->pc.seq);
	}
      }
      fprintf(of,"\n");
    }
    break;

  case PC_CSOURCE:
    fprintf(of,"%s\t.line\t%d; \"%s\"\t%s\n", (options.debug?"":";"), PCCS(pc)->line_number, PCCS(pc)->file_name, PCCS(pc)->line);
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
	if(PCF(pc)->fname) {
		pBranch *exits = PCF(pc)->to;
		int i=0;

		fprintf(of, "%s:", PCF(pc)->fname);

		if(options.verbose)
		    fprintf(of, "\t;Function start");

		fprintf(of, "\n");

		while(exits) {
			i++;
			exits = exits->next;
		}
		//if(i) i--;
		fprintf(of,"; %d exit point%c\n",i, ((i==1) ? ' ':'s'));

	}else {
		if((PCF(pc)->from && 
			PCF(pc)->from->pc->type == PC_FUNCTION &&
			PCF(PCF(pc)->from->pc)->fname) )
			fprintf(of,"; exit point of %s\n",PCF(PCF(pc)->from->pc)->fname);
		else
			fprintf(of,"; exit point [can't find entry point]\n");
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
		fprintf(of, "%s:\n", PCL(pc)->label);
	else if (PCL(pc)->key >= 0) 
		fprintf(of, "_%05d_DS_:\n", PCL(pc)->key);
	else
		fprintf(of, ";wild card label: id=%d\n", -PCL(pc)->key);
	
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
      
      /* Found a label */
      if(bprev) {
        bprev->next = b->next;  /* Not first pCode in chain */
        free(b);
      } else {
        pc->destruct(pc);
        PCI(pcl)->label = b->next;   /* First pCode in chain */
        free(b);
      }
      return;  /* A label can't occur more than once */
    }
    bprev = b;
    b = b->next;
  }
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
pBranch * pBranchAppend(pBranch *h, pBranch *n)
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
	b = Safe_alloc(sizeof(pBranch));
	b->pc = PCODE(t);             // The link to the 'to' pCode.
	b->next = NULL;
	
	f->to = pBranchAppend(f->to,b);
	
	// Now do the same for the 'to' pCode.
	
	//_ALLOC(b,sizeof(pBranch));
	b = Safe_alloc(sizeof(pBranch));
	b->pc = PCODE(f);
	b->next = NULL;
	
	t->from = pBranchAppend(t->from,b);
	
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int compareLabel(pCode *pc, pCodeOpLabel *pcop_label)
{
  pBranch *pbr;
  
  if(pc->type == PC_LABEL) {
    if( ((pCodeLabel *)pc)->key ==  pcop_label->key)
      return TRUE;
  }
  if(pc->type == PC_OPCODE || pc->type == PC_ASMDIR) {
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

/*-----------------------------------------------------------------*/
/* findNextpCode - given a pCode, find the next of type 'pct'      */
/*                 in the linked list                              */
/*-----------------------------------------------------------------*/
pCode * findNextpCode(pCode *pc, PC_TYPE pct)
{
	
	while(pc) {
		if(pc->type == pct)
			return pc;
		
		pc = pc->next;
	}
	
	return NULL;
}

#if 0
/*-----------------------------------------------------------------*/
/* findPrevpCode - given a pCode, find the previous of type 'pct'  */
/*                 in the linked list                              */
/*-----------------------------------------------------------------*/
static pCode * findPrevpCode(pCode *pc, PC_TYPE pct)
{
	
	while(pc) {
		if(pc->type == pct) {
			/*
			static unsigned int stop;
			if (pc->id == 524)
				stop++; // Place break point here
			*/
			return pc;
		}
		
		pc = pc->prev;
	}
	
	return NULL;
}
#endif

/*-----------------------------------------------------------------*/
/* findNextInstruction - given a pCode, find the next instruction  */
/*                       in the linked list                        */
/*-----------------------------------------------------------------*/
pCode * findNextInstruction(pCode *pci)
{
  pCode *pc = pci;

  while(pc) {
  if((pc->type == PC_OPCODE)
    || (pc->type == PC_WILD)
    || (pc->type == PC_ASMDIR))
      return pc;

#ifdef PCODE_DEBUG
    fprintf(stderr,"findNextInstruction:  ");
    printpCode(stderr, pc);
#endif
    pc = pc->next;
  }

  //fprintf(stderr,"Couldn't find instruction\n");
  return NULL;
}

/*-----------------------------------------------------------------*/
/* findNextInstruction - given a pCode, find the next instruction  */
/*                       in the linked list                        */
/*-----------------------------------------------------------------*/
pCode * findPrevInstruction(pCode *pci)
{
  pCode *pc = pci;

  while(pc) {

    if((pc->type == PC_OPCODE)
      || (pc->type == PC_WILD)
      || (pc->type == PC_ASMDIR))
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

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
reg_info * getRegFromInstruction(pCode *pc)
{
	reg_info *r;
	if(!pc                   || 
		!isPCI(pc)            ||
		!PCI(pc)->pcop        ||
		PCI(pc)->num_ops == 0 )
		return NULL;
	
	switch(PCI(pc)->pcop->type) {
	case PO_STATUS:
	case PO_FSR:
	case PO_INDF:
	case PO_INTCON:
	case PO_BIT:
	case PO_GPR_TEMP:
	case PO_SFR_REGISTER:
	case PO_PCL:
	case PO_PCLATH:
		return PCOR(PCI(pc)->pcop)->r;
	
	case PO_GPR_REGISTER:
	case PO_GPR_BIT:
	case PO_DIR:
		r = PCOR(PCI(pc)->pcop)->r;
		if (r)
			return r;
		return dirregWithName(PCI(pc)->pcop->name);
		
	case PO_LITERAL:
		break;
		
	case PO_IMMEDIATE:
		r = PCOI(PCI(pc)->pcop)->r;
		if (r)
			return r;
		return dirregWithName(PCI(pc)->pcop->name);
		
	default:
		break;
	}
	
	return NULL;
	
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
			
			if((PCI(pc)->pcop->type == PO_GPR_TEMP) 
				|| ((PCI(pc)->pcop->type == PO_GPR_BIT) && PCOR(PCI(pc)->pcop)->r && (PCOR(PCI(pc)->pcop)->r->pc_type == PO_GPR_TEMP))) {
				
				/* Loop through all of the registers declared so far in
				this block and see if we find this one there */
				
				reg_info *r = setFirstItem(pb->tregisters);
				
				while(r) {
					if((r->rIdx == PCOR(PCI(pc)->pcop)->r->rIdx) && (r->type == PCOR(PCI(pc)->pcop)->r->type)) {
						PCOR(PCI(pc)->pcop)->r = r;
						break;
					}
					r = setNextItem(pb->tregisters);
				}
				
				if(!r) {
					/* register wasn't found */
					//r = Safe_alloc(sizeof(regs));
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
					pic14_allocWithIdx (PCOR(PCI(pc)->pcop)->r->rIdx);
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
static void InsertpFlow(pCode *pc, pCode **pflow)
{
	if(*pflow)
		PCFL(*pflow)->end = pc;
	
	if(!pc || !pc->next)
		return;
	
	*pflow = newpCodeFlow();
	pCodeInsertAfter(pc, *pflow);
}

/*-----------------------------------------------------------------*/
/* BuildFlow(pBlock *pb) - examine the code in a pBlock and build  */
/*                         the flow blocks.                        */
/*
* BuildFlow inserts pCodeFlow objects into the pCode chain at each
* point the instruction flow changes. 
*/
/*-----------------------------------------------------------------*/
static void BuildFlow(pBlock *pb)
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
	
	//pflow = newpCodeFlow();    /* Create a new Flow object */
	//pflow->next = pb->pcHead;  /* Make the current head the next object */
	//pb->pcHead->prev = pflow;  /* let the current head point back to the flow object */
	//pb->pcHead = pflow;        /* Make the Flow object the head */
	//pflow->pb = pb;
	
	for( pc = findNextInstruction(pb->pcHead);
	pc != NULL;
	pc=findNextInstruction(pc)) { 
		
		pc->seq = seq++;
		PCI(pc)->pcflow = PCFL(pflow);
		
		//fprintf(stderr," build: ");
		//pc->print(stderr, pc);
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

			last_pci = findPrevInstruction (pc->prev);
			
			if(last_pci && (PCI(last_pci)->pcflow == PCFL(pflow)))
				InsertpFlow(last_pci, &pflow);
			
			PCI(pc)->pcflow = PCFL(pflow);
			
		}

		if(isPCI_SKIP(pc)) {
			
		/* The two instructions immediately following this one 
			* mark the beginning of a new flow segment */
			
			while(pc && isPCI_SKIP(pc)) {
				
				PCI(pc)->pcflow = PCFL(pflow);
				pc->seq = seq-1;
				seq = 1;
				
				InsertpFlow(pc, &pflow);
				pc=findNextInstruction(pc->next);
			}
			
			seq = 0;
			
			if(!pc)
				break;
			
			PCI(pc)->pcflow = PCFL(pflow);
			pc->seq = 0;
			InsertpFlow(pc, &pflow);
			
		} else if ( isPCI_BRANCH(pc) && !checkLabel(findNextInstruction(pc->next)))  {
			
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
				//free(PCI(pc)->pcflow);
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
			"PCC_W",
			"PCC_EXAMINE_PCOP",
			"PCC_REG_BANK0",
			"PCC_REG_BANK1",
			"PCC_REG_BANK2",
			"PCC_REG_BANK3"
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
	
	pc = findNextpCode(PCODE(pcflow), PC_OPCODE); 
	
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
/*
static int isBankInstruction(pCode *pc)
{
	regs *reg;
	int bank = -1;
	
	if(!isPCI(pc))
		return -1;
	
	if( ( (reg = getRegFromInstruction(pc)) != NULL) && isSTATUS_REG(reg)) {
		
		// Check to see if the register banks are changing
		if(PCI(pc)->isModReg) {
			
			pCodeOp *pcop = PCI(pc)->pcop;
			switch(PCI(pc)->op) {
				
			case POC_BSF:
				if(PCORB(pcop)->bit == PIC_RP0_BIT) {
					//fprintf(stderr, "  isBankInstruction - Set RP0\n");
					return  SET_BANK_BIT | PIC_RP0_BIT;
				}
				
				if(PCORB(pcop)->bit == PIC_RP1_BIT) {
					//fprintf(stderr, "  isBankInstruction - Set RP1\n");
					return  CLR_BANK_BIT | PIC_RP0_BIT;
				}
				break;
				
			case POC_BCF:
				if(PCORB(pcop)->bit == PIC_RP0_BIT) {
					//fprintf(stderr, "  isBankInstruction - Clr RP0\n");
					return  CLR_BANK_BIT | PIC_RP1_BIT;
				}
				if(PCORB(pcop)->bit == PIC_RP1_BIT) {
					//fprintf(stderr, "  isBankInstruction - Clr RP1\n");
					return  CLR_BANK_BIT | PIC_RP1_BIT;
				}
				break;
			default:
				//fprintf(stderr, "  isBankInstruction - Status register is getting Modified by:\n");
				//genericPrint(stderr, pc);
				;
			}
		}
		
				}
				
	return bank;
}
*/

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
/*
static void FillFlow(pCodeFlow *pcflow)
{
	pCode *pc;
	int cur_bank;
	
	if(!isPCFL(pcflow))
		return;
	
	//  fprintf(stderr, " FillFlow - flow block (seq=%d)\n", pcflow->pc.seq);
	
	pc = findNextpCode(PCODE(pcflow), PC_OPCODE); 
	
	if(!pc) {
		//fprintf(stderr, " FillFlow - empty flow (seq=%d)\n", pcflow->pc.seq);
		return;
	}
	
	cur_bank = -1;
	
	do {
		isBankInstruction(pc);
		pc = pc->next;
	} while (pc && (pc != pcflow->end) && !isPCFL(pc));
	/ *
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
		* /
}
*/

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void LinkFlow_pCode(pCodeInstruction *from, pCodeInstruction *to)
{
	pCodeFlowLink *fromLink, *toLink;
#if 0
	fprintf(stderr, "%s: linking ", __FUNCTION__ );
	if (from) from->pc.print(stderr, &from->pc);
	else fprintf(stderr, "(null)");
	fprintf(stderr, " -(%u)-> with -(%u)-> ",
		from && from->pcflow ? from->pcflow->pc.seq : 0,
		to && to->pcflow ? to->pcflow->pc.seq : 0);
	if (to) to->pc.print(stderr, &to->pc);
	else fprintf(stderr, "(null)");
#endif

	if(!from || !to || !to->pcflow || !from->pcflow)
		return;
	
	fromLink = newpCodeFlowLink(from->pcflow);
	toLink   = newpCodeFlowLink(to->pcflow);
	
	addSetIfnotP(&(from->pcflow->to), toLink);   //to->pcflow);
	addSetIfnotP(&(to->pcflow->from), fromLink); //from->pcflow);
	
}

/*-----------------------------------------------------------------*
* void LinkFlow(pBlock *pb)
*
* In BuildFlow, the PIC code has been partitioned into contiguous
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
	
	//fprintf(stderr,"linkflow \n");
	
	if (!pb) return;
	
	for( pcflow = findNextpCode(pb->pcHead, PC_FLOW); 
	pcflow != NULL;
	pcflow = findNextpCode(pcflow->next, PC_FLOW) ) {
		
		if(!isPCFL(pcflow))
			fprintf(stderr, "LinkFlow - pcflow is not a flow object ");
		
		//fprintf(stderr," link: ");
		//pcflow->print(stderr,pcflow);
		
		//FillFlow(PCFL(pcflow));
		
		/* find last instruction in flow */
		pc = findPrevInstruction (PCFL(pcflow)->end);
		if (!pc) {
			fprintf(stderr, "%s: flow without end (%u)?\n",
			__FUNCTION__, pcflow->seq );
			continue;
		}
		
		//fprintf(stderr, "LinkFlow - flow block (seq=%d) ", pcflow->seq);
		//pc->print(stderr, pc);
		if(isPCI_SKIP(pc)) {
			//fprintf(stderr, "ends with skip\n");
			//pc->print(stderr,pc);
			pct=findNextInstruction(pc->next);
			LinkFlow_pCode(PCI(pc),PCI(pct));
			pct=findNextInstruction(pct->next);
			LinkFlow_pCode(PCI(pc),PCI(pct));
			continue;
		}
		
		if(isPCI_BRANCH(pc)) {
			pCodeOpLabel *pcol = PCOLAB(PCI(pc)->pcop);
			
			//fprintf(stderr, "ends with branch\n  ");
			//pc->print(stderr,pc);

			if(!(pcol && isPCOLAB(pcol))) {
				if((PCI(pc)->op != POC_RETLW)
					&& (PCI(pc)->op != POC_RETURN)
					&& (PCI(pc)->op != POC_CALL)
					&& (PCI(pc)->op != POC_RETFIE) )
				{
					pc->print(stderr,pc);
					fprintf(stderr, "ERROR: %s, branch instruction doesn't have label\n",__FUNCTION__);
				}
			} else {
			
				if( (pct = findLabelinpBlock(pb,pcol)) != NULL)
					LinkFlow_pCode(PCI(pc),PCI(pct));
				else
					fprintf(stderr, "ERROR: %s, couldn't find label. key=%d,lab=%s\n",
					__FUNCTION__,pcol->key,((PCOP(pcol)->name)?PCOP(pcol)->name:"-"));
				//fprintf(stderr,"newpCodeOpLabel: key=%d, name=%s\n",key,((s)?s:""));
			}
			/* link CALLs to next instruction */
			if (PCI(pc)->op != POC_CALL) continue;
		}
		
		if(isPCI(pc)) {
			//fprintf(stderr, "ends with non-branching instruction:\n");
			//pc->print(stderr,pc);
			
			LinkFlow_pCode(PCI(pc),PCI(findNextInstruction(pc->next)));
			
			continue;
		}
		
		if(pc) {
			//fprintf(stderr, "ends with unknown\n");
			//pc->print(stderr,pc);
			continue;
		}
		
		fprintf(stderr, "ends with nothing: ERROR\n");
		
	}
}

static void pCodeReplace (pCode *old, pCode *new)
{
	pCodeInsertAfter (old, new);

	/* special handling for pCodeInstructions */
	if (isPCI(new) && isPCI(old))
	{
		//assert (!PCI(new)->from && !PCI(new)->to && !PCI(new)->label && /*!PCI(new)->pcflow && */!PCI(new)->cline);
		PCI(new)->from = PCI(old)->from;
		PCI(new)->to = PCI(old)->to;
		PCI(new)->label = PCI(old)->label;
		PCI(new)->pcflow = PCI(old)->pcflow;
		PCI(new)->cline = PCI(old)->cline;
	} // if

	old->destruct (old);
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void addpCodeComment(pCode *pc, const char *fmt, ...)
{
    va_list ap;
    char buffer[4096];
    pCode *newpc;

    va_start(ap, fmt);
    if (options.verbose || debug_verbose) {
	buffer[0] = ';';
	buffer[1] = ' ';
	vsprintf(&buffer[2], fmt, ap);

	newpc = newpCodeCharP(&buffer[0]); // strdup's the string
	pCodeInsertAfter(pc, newpc);
    }
    va_end(ap);
}

/*-----------------------------------------------------------------*/
/* Inserts a new pCodeInstruction before an existing one           */
/*-----------------------------------------------------------------*/
static void insertPCodeInstruction(pCodeInstruction *pci, pCodeInstruction *new_pci)
{
	pCode *pcprev;

	pcprev = findPrevInstruction(pci->pc.prev);
	
	pCodeInsertAfter(pci->pc.prev, &new_pci->pc);
	
	/* Move the label, if there is one */
	
	if(pci->label) {
		new_pci->label = pci->label;
		pci->label = NULL;
	}
	
	/* Move the C code comment, if there is one */
	
	if(pci->cline) {
		new_pci->cline = pci->cline;
		pci->cline = NULL;
	}
	
	/* The new instruction has the same pcflow block */
	new_pci->pcflow = pci->pcflow;

	/* Arrrrg: is pci's previous instruction is a skip, we need to
	 * change that into a jump (over pci and the new instruction) ... */
	if (pcprev && isPCI_SKIP(pcprev))
	{
		symbol *lbl = newiTempLabel (NULL);
		pCode *label = newpCodeLabel (NULL, lbl->key);
		pCode *jump = newpCode(POC_GOTO, newpCodeOpLabel(NULL, lbl->key));

		pCodeInsertAfter (pcprev, jump);

		// Yuck: Cannot simply replace INCFSZ/INCFSZW/DECFSZ/DECFSZW
		// We replace them with INCF/INCFW/DECF/DECFW followed by 'BTFSS STATUS, Z'
		switch (PCI(pcprev)->op) {
		case POC_INCFSZ:
		case POC_INCFSZW:
		case POC_DECFSZ:
		case POC_DECFSZW:
		    // These are turned into non-skipping instructions, so
		    // insert 'BTFSC STATUS, Z' after pcprev
		    pCodeInsertAfter (jump->prev, newpCode(POC_BTFSC, popCopyGPR2Bit(PCOP(&pc_status), PIC_Z_BIT)));
		    break;
		default:
		    // no special actions required
		    break;
		}
		pCodeReplace (pcprev, pCodeInstructionCopy (PCI(pcprev), 1));
		pcprev = NULL;
		pCodeInsertAfter((pCode*)pci, label);
		pBlockMergeLabels(pci->pc.pb);
	}
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int insertBankSel(pCodeInstruction  *pci, const char *name)
{
	pCode *new_pc;
	
	pCodeOp *pcop;

	// Never BANKSEL STATUS, this breaks all kinds of code (e.g., interrupt handlers).
	if (!strcmp("STATUS", name) || !strcmp("_STATUS", name)) return 0;
	
	pcop = popCopyReg(PCOR(pci->pcop));
	pcop->type = PO_GPR_REGISTER; // Sometimes the type is set to legacy 8051 - so override it
	if (pcop->name == 0)
		pcop->name = strdup(name);
	new_pc = newpCode(POC_BANKSEL, pcop);
	
	insertPCodeInstruction(pci, PCI(new_pc));
	return 1;
}

/*
 * isValidIdChar - check if c may be present in an identifier
 */
static int isValidIdChar (char c)
{
    if (c >= 'a' && c <= 'z') return 1;
    if (c >= 'A' && c <= 'Z') return 1;
    if (c >= '0' && c <= '9') return 1;
    if (c == '_') return 1;
    return 0;
}

/*
 * bankcompare - check if two operand string refer to the same register
 * This functions handles NAME and (NAME + x) in both operands.
 * Returns 1 on same register, 0 on different (or unknown) registers.
 */
static int bankCompare(const char *op1, const char *op2)
{
    int i;

    if (!op1 && !op2) return 0; // both unknown, might be different though!
    if (!op1 || !op2) return 0;

    // find start of operand name
    while (op1[0] == '(' || op1[0] == ' ') op1++;
    while (op2[0] == '(' || op2[0] == ' ') op2++;

    // compare till first non-identifier character
    for (i = 0; (op1[i] == op2[i]) && isValidIdChar(op1[i]); i++);
    if (!isValidIdChar(op1[i]) && !isValidIdChar(op2[i])) return 1;

    // play safe---assume different operands
    return 0;
}

/*
 * Interface to BANKSEL generation.
 * This function should return != 0 iff str1 and str2 denote operands that
 * are known to be allocated into the same bank. Consequently, there will
 * be no BANKSEL emitted if str2 is accessed while str1 has been used to
 * select the current bank just previously.
 *
 * If in doubt, return 0.
 */
static int
pic14_operandsAllocatedInSameBank(const char *str1, const char *str2) {
    // see glue.c(pic14printLocals)

    if (getenv("SDCC_PIC14_SPLIT_LOCALS")) {
        // no clustering applied, each register resides in its own bank
    } else {
        // check whether BOTH names are local registers
        // XXX: This is some kind of shortcut, should be safe...
        // In this model, all r0xXXXX are allocated into a single section
        // per file, so no BANKSEL required if accessing a r0xXXXX after a
        // (different) r0xXXXX. Works great for multi-byte operands.
        if (str1 && str2 && str1[0] == 'r' && str2[0] == 'r') return (1);
    } // if

    // assume operands in different banks
    return (0);
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int sameBank(reg_info *reg, reg_info *previous_reg, const char *new_bank, const char *cur_bank, unsigned max_mask)
{
    if (!cur_bank) return 0;

    if (previous_reg && reg && previous_reg->isFixed && reg->isFixed && ((previous_reg->address & max_mask) == (reg->address & max_mask)))	// only if exists 
      return 1;  // if we have address info, we use it for banksel optimization

    // regard '(regname + X)' and '(regname + Y)' as equal
    if (reg && reg->name && bankCompare(reg->name, cur_bank)) return 1;
    if (new_bank && bankCompare(new_bank, cur_bank)) return 1;

    // check allocation policy from glue.c
    if (reg && reg->name && pic14_operandsAllocatedInSameBank(reg->name, cur_bank)) return 1;
    if (new_bank && pic14_operandsAllocatedInSameBank(new_bank, cur_bank)) return 1;

    // seems to be a different operand--might be a different bank
    //printf ("BANKSEL from %s to %s/%s\n", cur_bank, reg->name, new_bank);
    return 0;
}
    
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void FixRegisterBanking(pBlock *pb)
{
    pCode *pc;
    pCodeInstruction *pci;
    reg_info *reg;
    reg_info *previous_reg;		// contains the previous variable access info
    const char *cur_bank, *new_bank;
    unsigned cur_mask, new_mask, max_mask;
    int allRAMmshared;
    
    if (!pb) return;

    max_mask = pic14_getPIC()->bankMask;
    cur_mask = max_mask;
    cur_bank = NULL;
    previous_reg = NULL;

    allRAMmshared = pic14_allRAMShared();

    for (pc = pb->pcHead; pc; pc = pc->next)
    {
	// this one has a label---might check bank at all jumps here...
	if (isPCI(pc) && (PCI(pc)->label || PCI(pc)->op == POC_CALL)) {
	    addpCodeComment(pc->prev, "BANKOPT3 drop assumptions: PCI with label or call found");
            previous_reg = NULL;
	    cur_bank = NULL; // start new flow
	    cur_mask = max_mask;
	}
	
	// this one is/might be a label or BANKSEL---assume nothing
	if (isPCL(pc) || isPCASMDIR(pc)) {
	    addpCodeComment(pc->prev, "BANKOPT4 drop assumptions: label or ASMDIR found");
            previous_reg = NULL;
	    cur_bank = NULL;
	    cur_mask = max_mask;
	}

	// this one modifies STATUS
	// XXX: this should be checked, but usually BANKSELs are not done this way in generated code
	
	if (isPCI(pc)) {
	    pci = PCI(pc);
	    if ((pci->inCond | pci->outCond) & PCC_REGISTER) {
		// might need a BANKSEL
		reg = getRegFromInstruction(pc);

		if (reg) {
		    new_bank = reg->name;
		    // reg->alias == 0: reg is in only one bank, we do not know which (may be any bank)
		    // reg->alias != 0: reg is in 2/4/8/2**N banks, we select one of them
		    new_mask = reg->alias;
		} else if (pci->pcop && pci->pcop->name) {
		    new_bank = pci->pcop->name;
		    new_mask = 0; // unknown, assume worst case
		} else {
		    assert(!"Could not get register from instruction.");
		    new_bank = "UNKNOWN";
		    new_mask = 0; // unknown, assume worst case
		}

		// optimizations...
		// XXX: add switch to disable these
		if (1) {
		    // reg present in all banks possibly selected?
		    if (new_mask == max_mask || (cur_mask && ((new_mask & cur_mask) == cur_mask))) {
			// no BANKSEL required
			addpCodeComment(pc->prev, "BANKOPT1 BANKSEL dropped; %s present in all of %s's banks", new_bank, cur_bank);
			continue;
		    }

		    // only one bank of memory and no SFR accessed?
		    // XXX: We can do better with fixed registers.
		    if (allRAMmshared && reg && (reg->type != REG_SFR) && (!reg->isFixed)) {
			// no BANKSEL required
			addpCodeComment(pc->prev, "BANKOPT1b BANKSEL dropped; %s present in all (of %s's) banks", new_bank, cur_bank);
			continue;
		    }

		    if (sameBank(reg, previous_reg, new_bank, cur_bank, max_mask)) {
			// no BANKSEL required
			addpCodeComment(pc->prev, "BANKOPT2 BANKSEL dropped; %s present in same bank as %s", new_bank, cur_bank);
			continue;
		    }
		} // if

		if (insertBankSel(pci, new_bank)) {
		    cur_mask = new_mask;
		    cur_bank = new_bank;
		    previous_reg = reg;
		} // if
	    } // if
	} // if
    } // for
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int OptimizepBlock(pBlock *pb)
{
	pCode *pc, *pcprev;
	int matches =0;

	if(!pb || options.nopeep)
		return 0;

	DFPRINTF((stderr," Optimizing pBlock: %c\n",getpBlock_dbName(pb)));
	/*
	for(pc = pb->pcHead; pc; pc = pc->next)
	matches += pCodePeepMatchRule(pc);
	*/

	pc = findNextInstruction(pb->pcHead);
	if(!pc)
		return 0;

	pcprev = pc->prev;
	do {
		if(pCodePeepMatchRule(pc)) {
			matches++;

			if(pcprev)
				pc = findNextInstruction(pcprev->next);
			else 
				pc = findNextInstruction(pb->pcHead);
		} else
			pc = findNextInstruction(pc->next);
	} while(pc);

	if(matches)
		DFPRINTF((stderr," Optimizing pBlock: %c - matches=%d\n",getpBlock_dbName(pb),matches));

	return matches;
}

/*-----------------------------------------------------------------*/
/* pBlockRemoveUnusedLabels - remove the pCode labels from the     */
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
	const char *s;

	if(isPCI(pc) && 
		(PCI(pc)->pcop) && 
		(PCI(pc)->pcop->type == PO_LABEL)) {

		pCodeOpLabel *pcol = PCOLAB(PCI(pc)->pcop);

		//fprintf(stderr,"changing label key from %d to %d\n",pcol->key, pcl->key);
		if(pcol->pcop.name)
			free(pcol->pcop.name);

		/* If the key is negative, then we (probably) have a label to
		* a function and the name is already defined */

		if(pcl->key>0) {
			SNPRINTF(buffer, sizeof(buffer), "_%05d_DS_", pcl->key);
			s = buffer;
		} else
			s = pcl->label;

		//SNPRINTF(buffer, sizeof(buffer), "_%05d_DS_", pcl->key);
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

	for(pc = pb->pcHead; (pc=findNextInstruction(pc->next)) != NULL; ) {
		pBranch *pbr = PCI(pc)->label;
		if(pbr && pbr->next) {
			pCode *pcd = pb->pcHead;

			//fprintf(stderr, "multiple labels\n");
			//pc->print(stderr,pc);

			pbr = pbr->next;
			while(pbr) {
				while ((pcd = findInstructionUsingLabel(PCL(PCI(pc)->label->pc), pcd)) != NULL) {
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
		if(isPCL(pc)) // Label pcode
			pcl = PCL(pc);
		else if (isPCI(pc) && PCI(pc)->label) // pcode instruction with a label
			pcl = PCL(PCI(pc)->label->pc);
		else continue;

		//fprintf(stderr," found  A LABEL !!! key = %d, %s\n", pcl->key,pcl->label);

		/* This pCode is a label, so search the pBlock to see if anyone
		* refers to it */

		if( (pcl->key>0) && (!findInstructionUsingLabel(pcl, pb->pcHead))) {
			/* Couldn't find an instruction that refers to this label
			* So, unlink the pCode label from it's pCode chain
			* and destroy the label */
			//fprintf(stderr," removed  A LABEL !!! key = %d, %s\n", pcl->key,pcl->label);

			DFPRINTF((stderr," !!! REMOVED A LABEL !!! key = %d, %s\n", pcl->key,pcl->label));
			if(pc->type == PC_LABEL) {
				unlinkpCode(pc);
				pCodeLabelDestruct(pc);
			} else {
				unlinkpCodeFromBranch(pc, PCODE(pcl));
				/*if(pc->label->next == NULL && pc->label->pc == NULL) {
				free(pc->label);
			}*/
			}
		}
	}
}

/*-----------------------------------------------------------------*/
/* pBlockMergeLabels - remove the pCode labels from the pCode      */
/*                     chain and put them into pBranches that are  */
/*                     associated with the appropriate pCode       */
/*                     instructions.                               */
/*-----------------------------------------------------------------*/
void pBlockMergeLabels(pBlock *pb)
{
	pBranch *pbr;
	pCode *pc, *pcnext=NULL;

	if(!pb)
		return;

	/* First, Try to remove any unused labels */
	//pBlockRemoveUnusedLabels(pb);

	/* Now loop through the pBlock and merge the labels with the opcodes */

	pc = pb->pcHead;

	while(pc) {
		pCode *pcn = pc->next;

		if(pc->type == PC_LABEL) {

			//fprintf(stderr," checking merging label %s\n",PCL(pc)->label);
			//fprintf(stderr,"Checking label key = %d\n",PCL(pc)->key);
			if((pcnext = findNextInstruction(pc) )) {

				// Unlink the pCode label from it's pCode chain
				unlinkpCode(pc);

				//fprintf(stderr,"Merged label key = %d\n",PCL(pc)->key);
				// And link it into the instruction's pBranch labels. (Note, since
				// it's possible to have multiple labels associated with one instruction
				// we must provide a means to accomodate the additional labels. Thus
				// the labels are placed into the singly-linked list "label" as 
				// opposed to being a single member of the pCodeInstruction.)

				//_ALLOC(pbr,sizeof(pBranch));
				pbr = Safe_alloc(sizeof(pBranch));
				pbr->pc = pc;
				pbr->next = NULL;

				PCI(pcnext)->label = pBranchAppend(PCI(pcnext)->label,pbr);
			} else {
				fprintf(stderr, "WARNING: couldn't associate label %s with an instruction\n",PCL(pc)->label);
			}
		} else if(pc->type == PC_CSOURCE) {

			/* merge the source line symbolic info into the next instruction */
			if((pcnext = findNextInstruction(pc) )) {

				// Unlink the pCode label from it's pCode chain
				unlinkpCode(pc);
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

/*-----------------------------------------------------------------*/
/* popCopyGPR2Bit - copy a pcode operator                          */
/*-----------------------------------------------------------------*/

pCodeOp *popCopyGPR2Bit(pCodeOp *pc, int bitval)
{
	pCodeOp *pcop;

	pcop = newpCodeOpBit(pc->name, bitval, 0);

	if( !( (pcop->type == PO_LABEL) ||
		(pcop->type == PO_LITERAL) ||
		(pcop->type == PO_STR) ))
		PCOR(pcop)->r = PCOR(pc)->r;  /* This is dangerous... */
	
	return pcop;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static void pBlockDestruct(pBlock *pb)
{
	
	if(!pb)
		return;
	
	
	free(pb);
	
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
				addpCode2pBlock(pbmerged, pb->pcHead);
				/* addpCode2pBlock doesn't handle the tail: */
				pbmerged->pcTail = pb->pcTail;
				
				pb->prev->next = pbn;
				if(pbn) 
					pbn->prev = pb->prev;
				
				
				pBlockDestruct(pb);
			}
			//printpBlock(stderr, pbmerged);
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

    if(!the_pFile)
        return;


    /* if this is not the first time this function has been called,
       then clean up old flow information */
    if(times_called++) {
        for(pb = the_pFile->pbHead; pb; pb = pb->next)
            unBuildFlow(pb);

        RegsUnMapLiveRanges();

    }

    GpcFlowSeq = 1;

    /* Phase 2 - Flow Analysis - Register Banking
     *
     * In this phase, the individual flow blocks are examined
     * and register banking is fixed.
     */

    //for(pb = the_pFile->pbHead; pb; pb = pb->next)
    //FixRegisterBanking(pb);

    /* Phase 2 - Flow Analysis
     *
     * In this phase, the pCode is partition into pCodeFlow
     * blocks. The flow blocks mark the points where a continuous
     * stream of instructions changes flow (e.g. because of
     * a call or goto or whatever).
     */

    for(pb = the_pFile->pbHead; pb; pb = pb->next)
        BuildFlow(pb);


    /* Phase 2 - Flow Analysis - linking flow blocks
     *
     * In this phase, the individual flow blocks are examined
     * to determine their order of excution.
     */

    for(pb = the_pFile->pbHead; pb; pb = pb->next)
        LinkFlow(pb);

    /* Phase 3 - Flow Analysis - Flow Tree
     *
     * In this phase, the individual flow blocks are examined
     * to determine their order of excution.
     */

    for(pb = the_pFile->pbHead; pb; pb = pb->next)
        BuildFlowTree(pb);


    /* Phase x - Flow Analysis - Used Banks
     *
     * In this phase, the individual flow blocks are examined
     * to determine the Register Banks they use
     */

//  for(pb = the_pFile->pbHead; pb; pb = pb->next)
//      FixBankFlow(pb);


    for(pb = the_pFile->pbHead; pb; pb = pb->next)
        pCodeRegMapLiveRanges(pb);

    RemoveUnusedRegisters();

//  for(pb = the_pFile->pbHead; pb; pb = pb->next)
    pCodeRegOptimizeRegUsage(level);

    OptimizepCode('*');

    /*
        for(pb = the_pFile->pbHead; pb; pb = pb->next)
        DumpFlow(pb);
     */
    /* debug stuff */
    /*
    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
        pCode *pcflow;
        for( pcflow = findNextpCode(pb->pcHead, PC_FLOW);
            (pcflow = findNextpCode(pcflow, PC_FLOW)) != NULL;
            pcflow = pcflow->next) {

                FillFlow(PCFL(pcflow));
            }
        }
     */
    /*
    for(pb = the_pFile->pbHead; pb; pb = pb->next) {
        pCode *pcflow;
        for( pcflow = findNextpCode(pb->pcHead, PC_FLOW);
            (pcflow = findNextpCode(pcflow, PC_FLOW)) != NULL;
            pcflow = pcflow->next) {

                FlowStats(PCFL(pcflow));
            }
        }
     */
}

/*-----------------------------------------------------------------*/
/* AnalyzeBanking - Called after the memory addresses have been    */
/*                  assigned to the registers.                     */
/*                                                                 */
/*-----------------------------------------------------------------*/

void AnalyzeBanking(void)
{
	pBlock  *pb;

	if(!picIsInitialized()) {
		werror(E_FILE_OPEN_ERR, "no memory size is known for this processor");
		exit(1);
	}
	
	if (!the_pFile) return;
	
	/* Phase x - Flow Analysis - Used Banks
	*
	* In this phase, the individual flow blocks are examined
	* to determine the Register Banks they use
	*/
	
	AnalyzeFlow(0);
	AnalyzeFlow(1);
	
	for(pb = the_pFile->pbHead; pb; pb = pb->next)
		FixRegisterBanking(pb);

	AnalyzeFlow(0);
	AnalyzeFlow(1);
	
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static DEFSETFUNC (resetrIdx)
{
	reg_info *r = (reg_info *)item;
	if (!r->isFixed) {
		r->rIdx = 0;
	}
	
	return 0;
}

/*-----------------------------------------------------------------*/
/* InitRegReuse - Initialises variables for code analyzer          */
/*-----------------------------------------------------------------*/
static void InitReuseReg(void)
{
	/* Find end of statically allocated variables for start idx */
	/* Start from begining of GPR. Note may not be 0x20 on some PICs */
	/* XXX: Avoid clashes with fixed registers, start late. */
	unsigned maxIdx = 0x1000;
	reg_info *r;
	for (r = setFirstItem(dynDirectRegs); r; r = setNextItem(dynDirectRegs)) {
		if (r->type != REG_SFR) {
			maxIdx += r->size; /* Increment for all statically allocated variables */
		}
	}
	peakIdx = maxIdx;
	applyToSet(dynAllocRegs,resetrIdx); /* Reset all rIdx to zero. */
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static unsigned
register_reassign(pBlock *pb, unsigned startIdx, unsigned level)
{
  pCode *pc;
  unsigned temp;
  unsigned idx = startIdx;

  /* check recursion */
  pc = setFirstItem(pb->function_entries);
  if (!pc)
    return idx;

  if (pb->visited)
    {
      set *regset;
      /* TODO: Recursion detection missing, should emit a warning as recursive code will fail. */

      //  Find the highest rIdx used by this function for return.
      regset = pb->tregisters;
      idx = 0;
      while (regset)
        {
          temp = ((reg_info *)regset->item)->rIdx;
          if (temp > idx)
            idx = temp;
          regset = regset->next;
        } // while
      DFPRINTF((stderr,
                "%*s(%u) function \"%s\" already visited: max idx = %04x\n",
                4 * level, "", level,PCF(pc)->fname, idx));
      return idx + 1;
    } // if

  /*
   * We now traverse the call tree depth first, assigning indices > startIdx
   * to the registers of all called functions before assigning indices to
   * the registers of the calling function, starting with one greater than
   * the max. index used by any child function.
   * This approach guarantees that, if f calls g, all registers of f have
   * greater indices than those of g (also holds transitively).
   *
   * XXX: If a function f calls a function g in a different module,
   *      we should handle the case that g could call a function h
   *      in f's module.
   *      The consequence of this is that even though f and h might
   *      share registers (they do not call each other locally) when
   *      looking only at f's module, they actually must not do so!
   *
   *      For a non-static function f, let ES(f) be the set of functions
   *      (including f) that can only be reached via f in the module-local
   *      call graph (ES(f) will hence be a subgraph).
   *      Let further REG(ES(f)) be the set of registers assigned to
   *      functions in ES(f).
   *      Then we should make sure that REG(ES(f)) and REG(ES(g)) are
   *      disjoint for all non-static functions f and g.
   *
   *      Unfortunately, determining the sets ES(f) is non-trivial,
   *      so we ignore this problem and declare all modules non-reentrant.
   *      This is a bug.
   */
  pb->visited = 1;

  DFPRINTF((stderr,
            "%*s(%u) reassigning registers for functions called by \"%s\":base idx = %04x\n",
            4 * level, "", level, PCF(pc)->fname, startIdx));

  for (pc = setFirstItem(pb->function_calls); pc; pc = setNextItem(pb->function_calls))
    {
      if (pc->type == PC_OPCODE && PCI(pc)->op == POC_CALL)
        {
          char *dest = get_op_from_instruction(PCI(pc));
          pCode *pcn = findFunction(dest);

          if (pcn)
            {
              /*
               * Reassign the registers of all called functions and record
               * the max. index I used by any child function --> I+1 will be
               * the first index available to this function.
               * (Problem shown with regression test src/regression/sub2.c)
               */
              unsigned childsMaxIdx;
              childsMaxIdx = register_reassign(pcn->pb,startIdx,level+1);
              if (childsMaxIdx > idx)
                idx = childsMaxIdx;
            } // if
        } // if
    } // for

  pc = setFirstItem(pb->function_entries);
  DFPRINTF((stderr,
            "%*s(%u) reassigning registers for function \"%s\":idx = %04x\n",
            4 * level, "", level, PCF(pc)->fname, idx));

  if (pb->tregisters)
    {
      reg_info *r;
      for (r = setFirstItem(pb->tregisters); r; r = setNextItem(pb->tregisters))
        {
          if ((r->type == REG_GPR) && (!r->isFixed) && (r->rIdx < (int)idx))
            {
              char s[20];
              set *regset;
              /*
               * Make sure, idx is not yet used in this routine ...
               * XXX: This should no longer be required, as all functions
               *      are reassigned at most once ...
               */
              do
                {
                  regset = pb->tregisters;
                  // do not touch s->curr ==> outer loop!
                  while (regset && ((reg_info *)regset->item)->rIdx != idx)
                    regset = regset->next;
                  if (regset)
                    idx++;
                }
              while (regset);
              r->rIdx = idx++;
              if (peakIdx < idx)
                peakIdx = idx;

              SNPRINTF(s, sizeof(s), "r0x%02X", r->rIdx);
              DFPRINTF((stderr,
                        "%*s(%u) reassigning register %p \"%s\" to \"%s\"\n",
                        4 * level, "", level, r, r->name, s));
              free(r->name);
              r->name = Safe_strdup(s);
            } // if
        } // for
    } // if

  /* return lowest index available for caller's registers */
  return idx;
}

/*------------------------------------------------------------------*/
/* ReuseReg were call tree permits                                  */
/*                                                                  */
/*  Re-allocate the GPR for optimum reuse for a given pblock        */ 
/*  eg  if a function m() calls function f1() and f2(), where f1    */
/*  allocates a local variable vf1 and f2 allocates a local         */
/*  variable vf2. Then providing f1 and f2 do not call each other   */
/*  they may share the same general purpose registers for vf1 and   */
/*  vf2.                                                            */
/*  This is done by first setting the the regs rIdx to start after  */
/*  all the global variables, then walking through the call tree    */
/*  renaming the registers to match their new idx and incrementng   */
/*  it as it goes. If a function has already been called it will    */
/*  only rename the registers if it has already used up those       */
/*  registers ie rIdx of the function's registers is lower than the */
/*  current rIdx. That way the register will not be reused while    */
/*  still being used by an eariler function call.                   */
/*                                                                  */
/*  Note for this to work the functions need to be declared static. */
/*                                                                  */
/*------------------------------------------------------------------*/
void
ReuseReg(void)
{
  pBlock *pb;

  if (options.noOverlay || !the_pFile)
    return;

  InitReuseReg();

  for(pb = the_pFile->pbHead; pb; pb = pb->next)
    {
      /* Non static functions can be called from other modules,
       * so their registers must reassign */
      if (pb->function_entries
          && (PCF(setFirstItem(pb->function_entries))->isPublic || !pb->visited))
        {
          register_reassign(pb,peakIdx,0);
        } // if
    } // for
}

/*-----------------------------------------------------------------*/
/* buildCallTree - look at the flow and extract all of the calls   */
/*                                                                 */
/*-----------------------------------------------------------------*/

static void buildCallTree(void)
{
	pBranch *pbr;
	pBlock  *pb;
	pCode   *pc;
	
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
           of the pCode. Look at the printCallTree function
           on how the pBranches are linked together.
         */

	for(pb = the_pFile->pbHead; pb; pb = pb->next) {
		pCode *pc_fstart=NULL;
		for(pc = pb->pcHead; pc; pc = pc->next) {
			if(isPCF(pc)) {
				pCodeFunction *pcf = PCF(pc);
				if (pcf->fname) {
					
					if(STRCASECMP(pcf->fname, "_main") == 0) {
						//fprintf(stderr," found main \n");
						pb->cmemmap = NULL;  /* FIXME do we need to free ? */
						pb->dbName = 'M';
					}
					
					pbr = Safe_alloc(sizeof(pBranch));
					pbr->pc = pc_fstart = pc;
					pbr->next = NULL;
					
					the_pFile->functions = pBranchAppend(the_pFile->functions,pbr);
					
					// Here's a better way of doing the same:
					addSet(&pb->function_entries, pc);
					
				} else {
					// Found an exit point in a function, e.g. return
					// (Note, there may be more than one return per function)
					if(pc_fstart)
						pBranchLink(PCF(pc_fstart), pcf);
					
					addSet(&pb->function_exits, pc);
				}
			} else if(isCALL(pc)) {
				addSet(&pb->function_calls,pc);
			}
		}
	}
}

/*-----------------------------------------------------------------*/
/* AnalyzepCode - parse the pCode that has been generated and form */
/*                all of the logical connections.                  */
/*                                                                 */
/* Essentially what's done here is that the pCode flow is          */
/* determined.                                                     */
/*-----------------------------------------------------------------*/

void AnalyzepCode(char dbName)
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
		
		/* First, merge the labels with the instructions */
		for(pb = the_pFile->pbHead; pb; pb = pb->next) {
			if('*' == dbName || getpBlock_dbName(pb) == dbName) {
				
				DFPRINTF((stderr," analyze and merging block %c\n",dbName));
				pBlockMergeLabels(pb);
				AnalyzepBlock(pb);
			} else {
				DFPRINTF((stderr," skipping block analysis dbName=%c blockname=%c\n",dbName,getpBlock_dbName(pb)));
			}
		}
		
		changes = OptimizepCode(dbName);
		
	} while(changes && (i++ < MAX_PASSES));
	
	buildCallTree();
}

/*-----------------------------------------------------------------*/
/* findFunction - Search for a function by name (given the name)   */
/*                in the set of all functions that are in a pBlock */
/* (note - I expect this to change because I'm planning to limit   */
/*  pBlock's to just one function declaration                      */
/*-----------------------------------------------------------------*/
static pCode *findFunction(const char *fname)
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

static void pBlockStats(FILE *of, pBlock *pb)
{
	
	pCode *pc;
	reg_info *r;
	
	fprintf(of,";***\n;  pBlock Stats: dbName = %c\n;***\n",getpBlock_dbName(pb));
	
	// for now just print the first element of each set
/*	pc = setFirstItem(pb->function_entries);
	if(pc) {
		fprintf(of,";entry:  ");
		pc->print(of,pc);
	}*/
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
				fprintf(of,";   %s\n",get_op_from_instruction(PCI(pc)));
			}
			pc = setNextItem(pb->function_calls);
		}
	}
	
	r = setFirstItem(pb->tregisters);
	if(r) {
		int n = elementsInSet(pb->tregisters);
		
		fprintf(of,";%d compiler assigned register%c:\n",n, ( (n!=1) ? 's' : ' '));
		
		while (r) {
			fprintf(of,";   %s\n",r->name);
			r = setNextItem(pb->tregisters);
		}
	}
}

#if 0
/*-----------------------------------------------------------------*/
/* printCallTree - writes the call tree to a file                  */
/*                                                                 */
/*-----------------------------------------------------------------*/
static void pct2(FILE *of,pBlock *pb,int indent)
{
	pCode *pc,*pcn;
	int i;
	//  set *registersInCallPath = NULL;
	
	if(!of)
		return;
	
	if(indent > 10)
		return; //recursion ?
	
	pc = setFirstItem(pb->function_entries);
	
	if(!pc)
		return;
	
	pb->visited = 0;
	
	for(i=0;i<indent;i++)   // Indentation
		fputc(' ',of);
	
	if(pc->type == PC_FUNCTION)
		fprintf(of,"%s\n",PCF(pc)->fname);
	else
		return;  // ???
	
	
	pc = setFirstItem(pb->function_calls);
	for( ; pc; pc = setNextItem(pb->function_calls)) {
		
		if(pc->type == PC_OPCODE && PCI(pc)->op == POC_CALL) {
			char *dest = get_op_from_instruction(PCI(pc));
			
			pcn = findFunction(dest);
			if(pcn) 
				pct2(of,pcn->pb,indent+1);
		} else
			fprintf(of,"BUG? pCode isn't a POC_CALL %d\n",__LINE__);
		
	}
	
	
}
#endif

#if 0
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
/* printCallTree - writes the call tree to a file                  */
/*                                                                 */
/*-----------------------------------------------------------------*/

static void printCallTree(FILE *of)
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
					fprintf(of,"\t%s\n",get_op_from_instruction(PCI(pc)));
			}
		}
		
		pbr = pbr->next;
	}
	
	
	fprintf(of,"\n**************\n\na better call tree\n");
	for(pb = the_pFile->pbHead; pb; pb = pb->next) {
		if(pb->visited)
			pct2(of,pb,0);
	}
	
	for(pb = the_pFile->pbHead; pb; pb = pb->next) {
		fprintf(of,"block dbname: %c\n", getpBlock_dbName(pb));
	}
}
#endif


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
			pCode *pcn = findFunction(get_op_from_instruction(PCI(pc)));
			pCode *pcp = pc->prev;
			pCode *pct;
			pCode *pce;
			
			pBranch *pbr;
			
			if(pcn && isPCF(pcn) && (PCF(pcn)->ncalled == 1) && !PCF(pcn)->isPublic && (pcp && (isPCI_BITSKIP(pcp)||!isPCI_SKIP(pcp)))) { /* Bit skips can be inverted other skips can not */
				
				InlineFunction(pcn->pb);
				
				/*
				At this point, *pc points to a CALL mnemonic, and
				*pcn points to the function that is being called.
				
				  To in-line this call, we need to remove the CALL
				  and RETURN(s), and link the function pCode in with
				  the CALLee pCode.
				  
				*/
				
				pc_call = pc;
				
				/* Check if previous instruction was a bit skip */
				if (isPCI_BITSKIP(pcp)) {
					pCodeLabel *pcl;
					/* Invert skip instruction and add a goto */
					PCI(pcp)->op = (PCI(pcp)->op == POC_BTFSS) ? POC_BTFSC : POC_BTFSS;
					
					if(isPCL(pc_call->next)) { // Label pcode
						pcl = PCL(pc_call->next);
					} else if (isPCI(pc_call->next) && PCI(pc_call->next)->label) { // pcode instruction with a label
						pcl = PCL(PCI(pc_call->next)->label->pc);
					} else {
						pcl = PCL(newpCodeLabel(NULL, newiTempLabel(NULL)->key+100));
						PCI(pc_call->next)->label->pc = (struct pCode*)pcl;
					}
					pCodeInsertAfter(pcp, newpCode(POC_GOTO, newpCodeOp(pcl->label,PO_STR)));
				}
				
				/* remove callee pBlock from the pBlock linked list */
				removepBlock(pcn->pb);
				
				pce = pcn;
				while(pce) {
					pce->pb = pb;
					pce = pce->next;
				}

				/* Remove the Function pCode */
				pct = findNextInstruction(pcn->next);

				/* Link the function with the callee */
				if (pcp) pcp->next = pcn->next;
				pcn->next->prev = pcp;

				/* Convert the function name into a label */

				pbr = Safe_alloc(sizeof(pBranch));
				pbr->pc = newpCodeLabel(PCF(pcn)->fname, -1);
				pbr->next = NULL;
				PCI(pct)->label = pBranchAppend(PCI(pct)->label,pbr);
				PCI(pct)->label = pBranchAppend(PCI(pct)->label,PCI(pc_call)->label);

				/* turn all of the return's except the last into goto's */
				/* check case for 2 instruction pBlocks */
				pce = findNextInstruction(pcn->next);
				while(pce) {
					pCode *pce_next = findNextInstruction(pce->next);

					if(pce_next == NULL) {
						/* found the last return */
						pCode *pc_call_next =  findNextInstruction(pc_call->next);

						//fprintf(stderr,"found last return\n");
						//pce->print(stderr,pce);
						pce->prev->next = pc_call->next;
						pc_call->next->prev = pce->prev;
						PCI(pc_call_next)->label = pBranchAppend(PCI(pc_call_next)->label,
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

void InlinepCode(void)
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
				pCode *pcn = findFunction(get_op_from_instruction(PCI(pc)));
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

