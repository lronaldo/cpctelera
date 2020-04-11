/*-------------------------------------------------------------------------
 genutils.c - source file for code generation for pic16
 	code generation utility functions

	Created by Vangelis Rokas (vrokas@otenet.gr) [Nov-2003]

	Based on :

  Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)
         and -  Jean-Louis VERN.jlvern@writeme.com (1999)
  Bug Fixes  -  Wojciech Stryjewski  wstryj1@tiger.lsu.edu (1999 v2.1.9a)
  PIC port   -  Scott Dattalo scott@dattalo.com (2000)
  PIC16 port -  Martin Dubuc m.dubuc@rogers.com (2002)
  
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
  
  Notes:
  000123 mlh	Moved aopLiteral to SDCCglue.c to help the split
  		Made everything static
-------------------------------------------------------------------------*/

/**********************************************************
 * Here is a list with completed genXXXXX functions
 *
 * genNot
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SDCCglobl.h"
#include "newalloc.h"

#include "common.h"
#include "SDCCpeeph.h"
#include "ralloc.h"
#include "pcode.h"
#include "device.h"
#include "gen.h"

#include "genutils.h"

#if 1
#define pic16_emitcode	DEBUGpic16_emitcode
#endif

#if defined(GEN_Not)
/*-----------------------------------------------------------------*/
/* pic16_genNot - generate code for ! operation                    */
/*-----------------------------------------------------------------*/
void pic16_genNot (iCode *ic)
{
/*
 * result[AOP_CRY,AOP_REG]  = ! left[AOP_CRY, AOP_REG]
 */

    FENTRY;
   
    /* assign asmOps to operand & result */
    pic16_aopOp (IC_LEFT(ic),ic,FALSE);
    pic16_aopOp (IC_RESULT(ic),ic,TRUE);
    DEBUGpic16_pic16_AopType(__LINE__,IC_LEFT(ic),NULL,IC_RESULT(ic));

    /* if in bit space then a special case */
    if (AOP_TYPE(IC_LEFT(ic)) == AOP_CRY) {
      if (AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) {
        pic16_emitpcode(POC_MOVLW,pic16_popGet(AOP(IC_LEFT(ic)),0));
        pic16_emitpcode(POC_XORWF,pic16_popGet(AOP(IC_RESULT(ic)),0));
      } else {
        pic16_emitpcode(POC_CLRF,pic16_popGet(AOP(IC_RESULT(ic)),0));
        pic16_emitpcode(POC_BTFSS,pic16_popGet(AOP(IC_LEFT(ic)),0));
        pic16_emitpcode(POC_INCF,pic16_popGet(AOP(IC_RESULT(ic)),0));
      }
      goto release;
    }

#if 0
    if(AOP_SIZE(IC_LEFT(ic)) == 1) {
      pic16_emitpcode(POC_COMFW,pic16_popGet(AOP(IC_LEFT(ic)),0));
      pic16_emitpcode(POC_ANDLW,pic16_popGetLit(1));
      pic16_emitpcode(POC_MOVWF,pic16_popGet(AOP(IC_RESULT(ic)),0));
      goto release;
    }
#endif

    pic16_toBoolean( IC_LEFT(ic) );
    emitSETC;
    pic16_emitpcode(POC_TSTFSZ, pic16_popCopyReg( &pic16_pc_wreg ));
    emitCLRC;
    pic16_outBitC( IC_RESULT(ic) );

release:    
    /* release the aops */
    pic16_freeAsmop(IC_LEFT(ic),NULL,ic,(RESULTONSTACK(ic) ? 0 : 1));
    pic16_freeAsmop(IC_RESULT(ic),NULL,ic,TRUE);
}

#endif	/* defined(GEN_Not) */



#if defined(GEN_Cpl)
/*-----------------------------------------------------------------*/
/* pic16_genCpl - generate code for complement                     */
/*-----------------------------------------------------------------*/
void pic16_genCpl (iCode *ic)
{
  int offset = 0;
  int size ;

/*
 * result[CRY,REG] = ~left[CRY,REG]
 */
    FENTRY;

    DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
    /* assign asmOps to operand & result */
    pic16_aopOp (IC_LEFT(ic),ic,FALSE);
    pic16_aopOp (IC_RESULT(ic),ic,TRUE);
    DEBUGpic16_pic16_AopType(__LINE__,IC_LEFT(ic),NULL,IC_RESULT(ic));

    /* if both are in bit space then 
     * a special case */
    if (AOP_TYPE(IC_RESULT(ic)) == AOP_CRY
      && AOP_TYPE(IC_LEFT(ic)) == AOP_CRY ) { 

        /* FIXME */
        pic16_emitcode("mov","c,%s",IC_LEFT(ic)->aop->aopu.aop_dir); 
        pic16_emitcode("cpl","c"); 
        pic16_emitcode("mov","%s,c",IC_RESULT(ic)->aop->aopu.aop_dir); 
        goto release; 
    } 

    size = AOP_SIZE(IC_RESULT(ic));
    if (size >= AOP_SIZE(IC_LEFT(ic))) size = AOP_SIZE(IC_LEFT(ic));
    
    while (size--) {
      if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) ) {
        pic16_emitpcode(POC_COMF,  pic16_popGet(AOP(IC_LEFT(ic)), offset));
      } else {
        pic16_emitpcode(POC_COMFW, pic16_popGet(AOP(IC_LEFT(ic)),offset));
        pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(IC_RESULT(ic)),offset));
      }
      offset++;
    }

    /* handle implicit upcast */
    size = AOP_SIZE(IC_RESULT(ic));
    if (offset < size)
    {
      if (SPEC_USIGN(operandType(IC_LEFT(ic)))) {
	while (offset < size) {
	  pic16_emitpcode(POC_SETF, pic16_popGet(AOP(IC_RESULT(ic)), offset));
	  offset++;
	} // while
      } else {
	if ((offset + 1) == size) {
	  /* just one byte to fix */
	  pic16_emitpcode(POC_SETF, pic16_popGet(AOP(IC_RESULT(ic)), offset));
	  pic16_emitpcode(POC_BTFSC, pic16_newpCodeOpBit(pic16_aopGet(AOP(IC_RESULT(ic)),offset-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
	  pic16_emitpcode(POC_CLRF, pic16_popGet(AOP(IC_RESULT(ic)), offset));
	} else {
	  /* two or more byte to adjust */
	  pic16_emitpcode(POC_SETF, pic16_popCopyReg( &pic16_pc_wreg ));
	  pic16_emitpcode(POC_BTFSC, pic16_newpCodeOpBit(pic16_aopGet(AOP(IC_RESULT(ic)),offset-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
	  pic16_emitpcode(POC_CLRF, pic16_popCopyReg( &pic16_pc_wreg ));
	  while (offset < size) {
	    pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(IC_RESULT(ic)), offset));
	    offset++;
	  } // while
	} // if
      }
    } // if

release:
    /* release the aops */
    pic16_freeAsmop(IC_LEFT(ic),NULL,ic,(RESULTONSTACK(ic) ? 0 : 1));
    pic16_freeAsmop(IC_RESULT(ic),NULL,ic,TRUE);
}
#endif	/* defined(GEN_Cpl) */



/*-----------------------------------------------------------------*/
/* Helper function to dump operand into comment lines              */
/*-----------------------------------------------------------------*/

void pic16_DumpValue(char *prefix, value *val)
{
//	char s[INITIAL_INLINEASM];  
	if(!val) return;

	DEBUGpic16_emitcode (";", " %s Dump value",prefix);
	DEBUGpic16_emitcode (";", " %s name:%s",prefix,val->name);
}

void pic16_DumpPcodeOp(char *prefix, pCodeOp *pcop)
{
//	char s[INITIAL_INLINEASM];  
	if(!pcop) return;

	DEBUGpic16_emitcode (";", " %s Dump pCodeOp",prefix);
	DEBUGpic16_emitcode (";", " %s name:%s",prefix,pcop->name);
	if(pcop->type == PO_NONE) {
		DEBUGpic16_emitcode (";", " %s type:PO_NONE",prefix);
	}
	if(pcop->type == PO_W) {
		DEBUGpic16_emitcode (";", " %s type:PO_W",prefix);
	}
	if(pcop->type == PO_WREG) {
		DEBUGpic16_emitcode (";", " %s type:PO_WREG",prefix);
	}
	if(pcop->type == PO_STATUS) {
		DEBUGpic16_emitcode (";", " %s type:PO_STATUS",prefix);
	}
	if(pcop->type == PO_BSR) {
		DEBUGpic16_emitcode (";", " %s type:PO_BSR",prefix);
	}
	if(pcop->type == PO_FSR0) {
		DEBUGpic16_emitcode (";", " %s type:PO_FSR0",prefix);
	}
	if(pcop->type == PO_INDF0) {
		DEBUGpic16_emitcode (";", " %s type:PO_INDF0",prefix);
	}
	if(pcop->type == PO_INTCON) {
		DEBUGpic16_emitcode (";", " %s type:PO_INTCON",prefix);
	}
	if(pcop->type == PO_GPR_REGISTER) {
		DEBUGpic16_emitcode (";", " %s type:PO_GPR_REGISTER",prefix);
	}
	if(pcop->type == PO_GPR_BIT) {
		DEBUGpic16_emitcode (";", " %s type:PO_GPR_BIT",prefix);
	}
	if(pcop->type == PO_GPR_TEMP) {
		DEBUGpic16_emitcode (";", " %s type:PO_GPR_TEMP",prefix);
	}
	if(pcop->type == PO_SFR_REGISTER) {
		DEBUGpic16_emitcode (";", " %s type:PO_SFR_REGISTER",prefix);
	}
	if(pcop->type == PO_PCL) {
		DEBUGpic16_emitcode (";", " %s type:PO_PCL",prefix);
	}
	if(pcop->type == PO_PCLATH) {
		DEBUGpic16_emitcode (";", " %s type:PO_PCLATH",prefix);
	}
	if(pcop->type == PO_LITERAL) {
		DEBUGpic16_emitcode (";", " %s type:PO_LITERAL",prefix);
		DEBUGpic16_emitcode (";", " %s lit:%s",prefix,PCOL(pcop)->lit);
	}
	if(pcop->type == PO_REL_ADDR) {
		DEBUGpic16_emitcode (";", " %s type:PO_REL_ADDR",prefix);
	}
	if(pcop->type == PO_IMMEDIATE) {
		DEBUGpic16_emitcode (";", " %s type:PO_IMMEDIATE",prefix);
	}
	if(pcop->type == PO_DIR) {
		DEBUGpic16_emitcode (";", " %s type:PO_DIR",prefix);
	}
	if(pcop->type == PO_CRY) {
		DEBUGpic16_emitcode (";", " %s type:PO_CRY",prefix);
	}
	if(pcop->type == PO_BIT) {
		DEBUGpic16_emitcode (";", " %s type:PO_BIT",prefix);
	}
	if(pcop->type == PO_STR) {
		DEBUGpic16_emitcode (";", " %s type:PO_STR",prefix);
	}
	if(pcop->type == PO_LABEL) {
		DEBUGpic16_emitcode (";", " %s type:PO_LABEL",prefix);
	}
	if(pcop->type == PO_WILD) {
		DEBUGpic16_emitcode (";", " %s type:PO_WILD",prefix);
	}
}



void pic16_DumpAop(char *prefix, asmop *aop)
{
	char s[INITIAL_INLINEASM];  
	if(!aop) return;

	DEBUGpic16_emitcode (";", " %s Dump asmop",prefix);
	if (aop->type == AOP_LIT)
	{
		DEBUGpic16_emitcode (";", " %s type:AOP_LIT",prefix);
		SNPRINTF(s, sizeof(s), "%s (aopu.aop_lit)",prefix);
		pic16_DumpValue(s,aop->aopu.aop_lit);
	}
	if (aop->type == AOP_REG)
		DEBUGpic16_emitcode (";", " %s type:AOP_REG",prefix);
	if (aop->type == AOP_DIR)
	{
		DEBUGpic16_emitcode (";", " %s type:AOP_DIR",prefix);
		DEBUGpic16_emitcode (";", " %s aopu.aop_dir:%s",prefix,aop->aopu.aop_dir);
	}
	if (aop->type == AOP_STK)
		DEBUGpic16_emitcode (";", " %s type:AOP_STK",prefix);
	if (aop->type == AOP_STA)
		DEBUGpic16_emitcode (";", " %s type:AOP_STA",prefix);
	if (aop->type == AOP_STR)
	{
		DEBUGpic16_emitcode (";", " %s type:AOP_STR",prefix);
		DEBUGpic16_emitcode (";", " %s aopu.aop_str:%s/%s/%s/%s",prefix,aop->aopu.aop_str[0],
				aop->aopu.aop_str[1],aop->aopu.aop_str[2],aop->aopu.aop_str[3]);
	}
	if (aop->type == AOP_CRY)
		DEBUGpic16_emitcode (";", " %s type:AOP_CRY",prefix);
	if (aop->type == AOP_ACC)
		DEBUGpic16_emitcode (";", " %s type:AOP_ACC",prefix);
	if (aop->type == AOP_PCODE)
	{
		DEBUGpic16_emitcode (";", " %s type:AOP_PCODE",prefix);
		SNPRINTF(s, sizeof(s), "%s (aopu.pcop)",prefix);
		pic16_DumpPcodeOp(s,aop->aopu.pcop);
	}


	DEBUGpic16_emitcode (";", " %s coff:%d",prefix,aop->coff);
	DEBUGpic16_emitcode (";", " %s size:%d",prefix,aop->size);
	DEBUGpic16_emitcode (";", " %s code:%d",prefix,aop->code);
	DEBUGpic16_emitcode (";", " %s paged:%d",prefix,aop->paged);
	DEBUGpic16_emitcode (";", " %s freed:%d",prefix,aop->freed);

}

void pic16_DumpSymbol(char *prefix, symbol *sym)
{
	char s[INITIAL_INLINEASM];  
	if(!sym) return;

	DEBUGpic16_emitcode (";", " %s Dump symbol",prefix);
	DEBUGpic16_emitcode (";", " %s name:%s",prefix,sym->name);
	DEBUGpic16_emitcode (";", " %s rname:%s",prefix,sym->rname);
	DEBUGpic16_emitcode (";", " %s level:%d",prefix,sym->level);
	DEBUGpic16_emitcode (";", " %s block:%d",prefix,sym->block);
	DEBUGpic16_emitcode (";", " %s key:%d",prefix,sym->key);
	DEBUGpic16_emitcode (";", " %s implicit:%d",prefix,sym->implicit);
	DEBUGpic16_emitcode (";", " %s undefined:%d",prefix,sym->undefined);
	DEBUGpic16_emitcode (";", " %s _isparm:%d",prefix,sym->_isparm);
	DEBUGpic16_emitcode (";", " %s ismyparm:%d",prefix,sym->ismyparm);
	DEBUGpic16_emitcode (";", " %s isitmp:%d",prefix,sym->isitmp);
	DEBUGpic16_emitcode (";", " %s islbl:%d",prefix,sym->islbl);
	DEBUGpic16_emitcode (";", " %s isref:%d",prefix,sym->isref);
	DEBUGpic16_emitcode (";", " %s isind:%d",prefix,sym->isind);
	DEBUGpic16_emitcode (";", " %s isinvariant:%d",prefix,sym->isinvariant);
	DEBUGpic16_emitcode (";", " %s cdef:%d",prefix,sym->cdef);
	DEBUGpic16_emitcode (";", " %s addrtaken:%d",prefix,sym->addrtaken);
	DEBUGpic16_emitcode (";", " %s isreqv:%d",prefix,sym->isreqv);
	DEBUGpic16_emitcode (";", " %s udChked:%d",prefix,sym->udChked);
	DEBUGpic16_emitcode (";", " %s isLiveFcall:%d",prefix,sym->isLiveFcall);
	DEBUGpic16_emitcode (";", " %s isspilt:%d",prefix,sym->isspilt);
	DEBUGpic16_emitcode (";", " %s spillA:%d",prefix,sym->spillA);
	DEBUGpic16_emitcode (";", " %s remat:%d",prefix,sym->remat);
	DEBUGpic16_emitcode (";", " %s isptr:%d",prefix,sym->isptr);
	DEBUGpic16_emitcode (";", " %s uptr:%d",prefix,sym->uptr);
	DEBUGpic16_emitcode (";", " %s isFree:%d",prefix,sym->isFree);
	DEBUGpic16_emitcode (";", " %s islocal:%d",prefix,sym->islocal);
	DEBUGpic16_emitcode (";", " %s blockSpil:%d",prefix,sym->blockSpil);
	DEBUGpic16_emitcode (";", " %s remainSpil:%d",prefix,sym->remainSpil);
	DEBUGpic16_emitcode (";", " %s stackSpil:%d",prefix,sym->stackSpil);
	DEBUGpic16_emitcode (";", " %s onStack:%d",prefix,sym->onStack);
	DEBUGpic16_emitcode (";", " %s iaccess:%d",prefix,sym->iaccess);
	DEBUGpic16_emitcode (";", " %s ruonly:%d",prefix,sym->ruonly);
	DEBUGpic16_emitcode (";", " %s spildir:%d",prefix,sym->spildir);
	DEBUGpic16_emitcode (";", " %s ptrreg:%d",prefix,sym->ptrreg);
	DEBUGpic16_emitcode (";", " %s noSpilLoc:%d",prefix,sym->noSpilLoc);
	DEBUGpic16_emitcode (";", " %s isstrlit:%d",prefix,sym->isstrlit);
	DEBUGpic16_emitcode (";", " %s accuse:%d",prefix,sym->accuse);
	DEBUGpic16_emitcode (";", " %s dptr:%d",prefix,sym->dptr);
	DEBUGpic16_emitcode (";", " %s allocreq:%d",prefix,sym->allocreq);
	DEBUGpic16_emitcode (";", " %s stack:%d",prefix,sym->stack);
	DEBUGpic16_emitcode (";", " %s xstack:%d",prefix,sym->xstack);
	DEBUGpic16_emitcode (";", " %s nRegs:%d",prefix,sym->nRegs);
	DEBUGpic16_emitcode (";", " %s regType:%d",prefix,sym->regType);

	// struct regs !!!

	if(sym->aop)
	{
		SNPRINTF(s, sizeof(s), "%s (aop)",prefix);
		pic16_DumpAop(s,sym->aop);
	} else {
		DEBUGpic16_emitcode (";", " %s aop:NULL",prefix);
	}
}

void pic16_DumpOp(char *prefix, operand *op)
{
	char s[INITIAL_INLINEASM];  
	if(!op) return;

	DEBUGpic16_emitcode (";", " %s Dump operand",prefix);
	if(IS_SYMOP(op))
		DEBUGpic16_emitcode (";", " %s type: SYMBOL",prefix);
	if(IS_VALOP(op))
		DEBUGpic16_emitcode (";", " %s type: VALUE",prefix);
	if(IS_TYPOP(op))
		DEBUGpic16_emitcode (";", " %s type: TYPE",prefix);
	DEBUGpic16_emitcode (";", " %s isaddr:%d",prefix,op->isaddr);
	DEBUGpic16_emitcode (";", " %s isvolatile:%d",prefix,op->isvolatile);
	DEBUGpic16_emitcode (";" ," %s isGlobal:%d",prefix,op->isGlobal);
	DEBUGpic16_emitcode (";", " %s isPtr:%d",prefix,op->isPtr);
	DEBUGpic16_emitcode (";", " %s isGptr:%d",prefix,op->isGptr);
	DEBUGpic16_emitcode (";", " %s isParm:%d",prefix,op->isParm);
	DEBUGpic16_emitcode (";", " %s isLiteral:%d",prefix,op->isLiteral);
	DEBUGpic16_emitcode (";", " %s key:%d",prefix,op->key);
	if(IS_SYMOP(op)) {
		SNPRINTF(s, sizeof(s), "%s (symOperand)",prefix);
		pic16_DumpSymbol(s, OP_SYMBOL (op));
	}
}

void pic16_DumpOpX(FILE *fp, char *prefix, operand *op)
{
  if(!op)return;
    
  fprintf(fp, "%s [", prefix);
  fprintf(fp, "%s", IS_SYMOP(op)?"S":" ");
  fprintf(fp, "%s", IS_VALOP(op)?"V":" ");
  fprintf(fp, "%s", IS_TYPOP(op)?"T":" ");
  fprintf(fp, "] ");

  fprintf(fp, "isaddr:%d,", op->isaddr);
  fprintf(fp, "isvolatile:%d,", op->isvolatile);
  fprintf(fp, "isGlobal:%d,", op->isGlobal);
  fprintf(fp, "isPtr:%d,", op->isPtr);
  fprintf(fp, "isParm:%d,", op->isParm);
  fprintf(fp, "isLit:%d\n", op->isLiteral);
}  
    

void _debugf(char *f, int l, char *frm, ...)
{
  va_list ap;
  
    va_start(ap, frm);
    fprintf(stderr, "%s:%d ", f, l);
    vfprintf(stderr, frm, ap);
    va_end(ap);
}



void gpsimio2_pcop(pCodeOp *pcop)
{
  pic16_emitpcode(POC_MOVFF, pic16_popGet2p(pcop, pic16_popCopyReg(&pic16_pc_gpsimio2)));
}

void gpsimio2_lit(unsigned char lit)
{
  pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit));
  pic16_emitpcode(POC_MOVFF, pic16_popGet2p(pic16_popCopyReg(&pic16_pc_wreg), pic16_popCopyReg(&pic16_pc_gpsimio2)));
}

void gpsimio2_str(char *buf)
{
  while(*buf) {
    gpsimio2_lit(*buf);
    buf++;
  }
}

void gpsimDebug_StackDump(char *fname, int line, char *info)
{
  pic16_emitpcomment("; gpsim debug stack dump; %s @ %d\tinfo: ", fname, line, info);
  
  gpsimio2_str("&c[S:");
  gpsimio2_str(info);
  gpsimio2_str("] &h");
  
  pic16_emitpcode(POC_MOVFF, pic16_popGet2p(pic16_popCopyReg(&pic16_pc_fsr1h),
                pic16_popCopyReg(&pic16_pc_gpsimio2)));
  pic16_emitpcode(POC_MOVFF, pic16_popGet2p(pic16_popCopyReg(&pic16_pc_fsr1l),
                pic16_popCopyReg(&pic16_pc_gpsimio2)));

  gpsimio2_lit('\n');
}

const char *gptr_fns[4][2] = {
  { "_gptrget1", "_gptrput1" },
  { "_gptrget2", "_gptrput2" },
  { "_gptrget3", "_gptrput3" },
  { "_gptrget4", "_gptrput4" } };

extern set *externs;
  
/* generate a call to the generic pointer read/write functions */
void pic16_callGenericPointerRW(int rw, int size)
{
  char buf[32];
  symbol *sym;

    if(size>4) {
      werror(W_POSSBUG2, __FILE__, __LINE__);
      abort();
    }

    strcpy(buf, port->fun_prefix);
    strcat(buf, gptr_fns[size-1][rw]);
    
    pic16_emitpcode (POC_CALL, pic16_popGetWithString (buf));
    
    sym = newSymbol( buf, 0 );
    sym->used++;
    strcpy(sym->rname, buf);
    checkAddSym(&externs, sym);
}



/* check all condition and return appropriate instruction, POC_CPFSGT or POC_CPFFSLT */
static int selectCompareOp(resolvedIfx *rIfx, iCode *ifx,
        operand *result, int offset, int invert_op)
{
  /* add code here */
  
  /* check condition, > or < ?? */
  if(rIfx->condition != 0)invert_op ^= 1;
  
  if(ifx && IC_FALSE(ifx))invert_op ^= 1;

  if(!ifx)invert_op ^= 1;

  DEBUGpic16_emitcode("; +++", "%s:%d %s] rIfx->condition= %d, ifx&&IC_FALSE(ifx)= %d, invert_op = %d",
      __FILE__, __LINE__, __FUNCTION__, rIfx->condition, (ifx && IC_FALSE(ifx)), invert_op);
  
  /* do selection */
  if(!invert_op)return POC_CPFSGT;
  else return POC_CPFSLT;
}

/* return 1 if function handles compare, 0 otherwise */
/* this functions handles special cases like:
 * reg vs. zero
 * reg vs. one
 */
int pic16_genCmp_special(operand *left, operand *right, operand *result,
                    iCode *ifx, resolvedIfx *rIfx, int sign)
{
  int size;
  int offs=0;
  symbol *tmplbl;
  unsigned long lit;
  int op, cmp_op=0, cond_pre;

    FENTRY;
    
    if(!(pic16_options.opt_flags & OF_OPTIMIZE_CMP))return 0;

    size = max(AOP_SIZE(left), AOP_SIZE(right));

    cond_pre = rIfx->condition; // must restore old value on return with 0!!!
    
    if(!isAOP_REGlike(left)) {
      operand *dummy;

        dummy = left;
        left = right;
        right = dummy;
        
        /* invert comparing operand */
//        cmp_op ^= 1;
        rIfx->condition ^= 1;
    }
    
    
    if(isAOP_REGlike(left) && isAOP_LIT(right)) {
      /* comparing register vs. literal */
      lit = ulFromVal(AOP(right)->aopu.aop_lit);
      
      
      if(size == 1) {
        op = selectCompareOp(rIfx, ifx, result, offs, cmp_op);
        
        DEBUGpic16_emitcode("%%", "comparing operand %s, condition: %d", (op==POC_CPFSLT?"POC_CPFSLT":"POC_CPFSGT"), rIfx->condition);

        if(!sign) {
          /* unsigned compare */
          switch( lit ) {
            case 0:
              if(ifx && IC_FALSE(ifx)) {
                tmplbl = newiTempLabel( NULL );
                pic16_emitpcode(POC_TSTFSZ, pic16_popGet(AOP(left), 0));
                pic16_emitpcode(POC_BRA, pic16_popGetLabel(tmplbl->key));
                pic16_emitpcode(POC_GOTO, pic16_popGetLabel(rIfx->lbl->key));
                pic16_emitpLabel(tmplbl->key);

                ifx->generated = 1;
                return 1;
              }
              break;
          }	/* switch */

        }	/* if(!sign) */

      }		/* if(size==1) */

    }		/* */
      
  rIfx->condition = cond_pre;
  return 0;
}
