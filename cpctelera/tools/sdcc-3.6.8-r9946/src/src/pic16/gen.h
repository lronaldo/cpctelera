/*-------------------------------------------------------------------------
  gen.h - header file for code generation for PIC16

             Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)
	     PIC port   - T. Scott Dattalo scott@dattalo.com (2000)
	     PIC16 port   - Martin Dubuc m.dubuc@rogers.com (2000)

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

#ifndef SDCCGENPIC16_H
#define SDCCGENPIC16_H

/* If you change these, you also have to update the library files
 * device/lib/pic16/libsdcc/gptr{get,put}{1,2,3,4}.c */
#define GPTR_TAG_DATA   0x80
#define GPTR_TAG_EEPROM 0x40
#define GPTR_TAG_CODE   0x00    /* must be 0 becaue of UPPER(sym)==0 */

struct pCodeOp;

enum
  {
    AOP_LIT = 1,
    AOP_REG,
    AOP_DIR,
    AOP_STK,
    AOP_STR,
    AOP_CRY,
    AOP_ACC,
    AOP_PCODE,
    AOP_STA		// asmop on stack
  };

/* type asmop : a homogenised type for 
   all the different spaces an operand can be
   in */
typedef struct asmop
  {

    short type;			/* can have values
				   AOP_LIT    -  operand is a literal value
				   AOP_REG    -  is in registers
				   AOP_DIR    -  direct just a name
				   AOP_STK    -  should be pushed on stack this
				   can happen only for the result
				   AOP_CRY    -  carry contains the value of this
				   AOP_STR    -  array of strings
				   AOP_ACC    -  result is in the acc:b pair
				 */
    short coff;			/* current offset */
    short size;			/* total size */
    unsigned code:1;		/* is in Code space */
    unsigned paged:1;		/* in paged memory  */
    unsigned freed:1;		/* already freed    */
    union
      {
	value *aop_lit;		/* if literal */
	reg_info *aop_reg[4];	/* array of registers */
	char *aop_dir;		/* if direct  */
	reg_info *aop_ptr;		/* either -> to r0 or r1 */
	int aop_stk;		/* stack offset when AOP_STK */
	char *aop_str[4];	/* just a string array containing the location */
/*	regs *aop_alloc_reg;     * points to a dynamically allocated register */
	pCodeOp *pcop;
	struct {
	  int stk;
	  pCodeOp *pop[4];
        } stk;
      }
    aopu;
  }
asmop;

void genpic16Code (iCode *);

extern unsigned pic16_fReturnSizePic;


#define AOP(op) op->aop
#define AOP_TYPE(op) AOP(op)->type
#define AOP_SIZE(op) AOP(op)->size

#define AOP_NEEDSACC(x) (AOP(x) && (AOP_TYPE(x) == AOP_CRY ||  \
                         AOP(x)->paged)) 

#define RESULTONSTACK(x) \
                         (IC_RESULT(x) && IC_RESULT(x)->aop && \
                         IC_RESULT(x)->aop->type == AOP_STK )
#define RESULTONSTA(x)	(IC_RESULT(x) && IC_RESULT(x)->aop && IC_RESULT(x)->aop->type == AOP_STA)


#define MOVA(x) if (strcmp(x,"a") && strcmp(x,"acc")) pic16_emitcode(";XXX mov","a,%s  %s,%d",x,__FILE__,__LINE__);
#define CLRC    pic16_emitcode(";XXX clr","c %s,%d",__FILE__,__LINE__);


#define BIT_NUMBER(x) (x & 7)
#define BIT_REGISTER(x) (x>>3)


#define LSB     0
#define MSB16   1
#define MSB24   2
#define MSB32   3


#define FUNCTION_LABEL_INC  40

/*-----------------------------------------------------------------*/
/* Macros for emitting skip instructions                           */
/*-----------------------------------------------------------------*/

#define emitSKPC    pic16_emitpcode(POC_BTFSS,pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_C_BIT))
#define emitSKPNC   pic16_emitpcode(POC_BTFSC,pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_C_BIT))
#define emitSKPZ    pic16_emitpcode(POC_BTFSS,pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_Z_BIT))
#define emitSKPNZ   pic16_emitpcode(POC_BTFSC,pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_Z_BIT))
#define emitSKPDC   pic16_emitpcode(POC_BTFSS,pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_DC_BIT))
#define emitSKPNDC  pic16_emitpcode(POC_BTFSC,pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_DC_BIT))
#define emitCLRZ    pic16_emitpcode(POC_BCF,  pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_Z_BIT))
#define emitCLRC    pic16_emitpcode(POC_BCF,  pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_C_BIT))
#define emitCLRDC   pic16_emitpcode(POC_BCF,  pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_DC_BIT))
#define emitSETZ    pic16_emitpcode(POC_BSF,  pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_Z_BIT))
#define emitSETC    pic16_emitpcode(POC_BSF,  pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_C_BIT))
#define emitSETDC   pic16_emitpcode(POC_BSF,  pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_DC_BIT))

#define emitTOGC    pic16_emitpcode(POC_BTG,  pic16_popCopyGPR2Bit(PCOP(&pic16_pc_status),PIC_C_BIT))

int pic16_getDataSize(operand *op);
void pic16_emitpcode_real(PIC_OPCODE poc, pCodeOp *pcop);
#define pic16_emitpcode(poc,pcop)	do { if (pic16_pcode_verbose) pic16_emitpcomment ("%s:%u(%s):", __FILE__, __LINE__, __FUNCTION__); pic16_emitpcode_real(poc,pcop); } while(0)
void pic16_emitpLabel(int key);
void pic16_emitcode (char *inst,char *fmt, ...);
void DEBUGpic16_emitcode (char *inst,char *fmt, ...);
void pic16_emitDebuggerSymbol (const char *);
bool pic16_sameRegs (asmop *aop1, asmop *aop2 );
char *pic16_aopGet (asmop *aop, int offset, bool bit16, bool dname);
void DEBUGpic16_pic16_AopType(int line_no, operand *left, operand *right, operand *result);
void DEBUGpic16_pic16_AopTypeSign(int line_no, operand *left, operand *right, operand *result);


bool pic16_genPlusIncr (iCode *ic);
void pic16_outBitAcc(operand *result);
void pic16_genPlusBits (iCode *ic);
void pic16_genPlus (iCode *ic);
bool pic16_genMinusDec (iCode *ic);
void pic16_addSign(operand *result, int offset, int sign);
void pic16_genMinusBits (iCode *ic);
void pic16_genMinus (iCode *ic);
void pic16_genLeftShiftLiteral (operand *left, operand *right, operand *result, iCode *ic);

pCodeOp *pic16_popGet2p(pCodeOp *src, pCodeOp *dst);
void pic16_emitpcomment (char *fmt, ...);

pCodeOp *pic16_popGetLabel(int key);
pCodeOp *pic16_popCopyReg(pCodeOpReg *pc);
pCodeOp *pic16_popCopyGPR2Bit(pCodeOp *pc, int bitval);
pCodeOp *pic16_popGetLit(int lit);
pCodeOp *pic16_popGetLit2(int lit, pCodeOp *arg2);
pCodeOp *popGetWithString(char *str);
pCodeOp *pic16_popGet (asmop *aop, int offset);//, bool bit16, bool dname);
pCodeOp *pic16_popGetTempReg(int lock);
pCodeOp *pic16_popGetTempRegCond(bitVect *, bitVect *, int lock);
void pic16_popReleaseTempReg(pCodeOp *pcop, int lock);

pCodeOp *pic16_popCombine2(pCodeOpReg *src, pCodeOpReg *dst, int noalloc);

void pic16_aopPut (asmop *aop, char *s, int offset);
void pic16_outAcc(operand *result);
void pic16_aopOp (operand *op, iCode *ic, bool result);
void pic16_outBitC(operand *result);
void pic16_toBoolean(operand *oper);
void pic16_freeAsmop (operand *op, asmop *aaop, iCode *ic, bool pop);
const char *pic16_pCodeOpType(  pCodeOp *pcop);
int pic16_my_powof2 (unsigned long num);

void pic16_mov2w (asmop *aop, int offset);
void pic16_mov2f(asmop *dst, asmop *src, int offset);

bool pic16_isLitOp(operand *op);
bool pic16_isLitAop(asmop *aop);

void dumpiCode(iCode *lic);

int inWparamList(char *s);

#include "device.h"

#define DUMP_FUNCTION_ENTRY	1
#define DUMP_FUNCTION_EXIT	0

#if DUMP_FUNCTION_ENTRY
#define FENTRY	if(pic16_options.debgen&2)pic16_emitpcomment("**{\t%d %s", __LINE__, __FUNCTION__)
#define FENTRY2 if(pic16_options.debgen&2)pic16_emitpcomment("**{\t%d %s", __LINE__, __FUNCTION__)
#else
#define FENTRY
#define FENTRY2
#endif

#if DUMP_FUNCTION_EXIT
#define FEXIT	if(pic16_options.debgen&2)pic16_emitpcomment("; **}", "%d %s", __LINE__, __FUNCTION__)
#define FEXIT2	if(pic16_options.debgen&2)pic16_emitpcomment("**{\t%d %s", __LINE__, __FUNCTION__)
#else
#define FEXIT
#define FEXIT2
#endif

#define ERROR	werror(W_POSSBUG2, __FILE__, __LINE__)
#endif
