/*-------------------------------------------------------------------------
  genarith.c - source file for code generation - arithmetic

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Bug Fixes  -  Wojciech Stryjewski  wstryj1@tiger.lsu.edu (1999 v2.1.9a)
  PIC port:
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
/*
  Notes:
  000123 mlh    Moved aopLiteral to SDCCglue.c to help the split
                Made everything static
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SDCCglobl.h"
#include "newalloc.h"

#if defined(_MSC_VER) && (_MSC_VER < 1300)
#define __FUNCTION__            __FILE__
#endif

#include "common.h"
#include "SDCCpeeph.h"
#include "ralloc.h"
#include "pcode.h"
#include "gen.h"

#if 1
#define pic16_emitcode  DEBUGpic16_emitcode
#endif

extern void pic16_emitpLabelFORCE(int key);

#define BYTEofLONG(l,b) ( (l>> (b<<3)) & 0xff)

void DEBUGpic16_pic16_AopType(int line_no, operand *left, operand *right, operand *result);
void pic16_emitpcomment(char *, ...);
pCodeOp *pic16_popGet2p(pCodeOp *src, pCodeOp *dst);
const char *pic16_AopType(short type)
{
  switch(type) {
  case AOP_LIT:         return "AOP_LIT";
  case AOP_REG:         return "AOP_REG";
  case AOP_DIR:         return "AOP_DIR";
  case AOP_STK:         return "AOP_STK";
  case AOP_STR:         return "AOP_STR";
  case AOP_CRY:         return "AOP_CRY";
  case AOP_ACC:         return "AOP_ACC";
  case AOP_PCODE:       return "AOP_PCODE";
  case AOP_STA:         return "AOP_STA";
  }

  return "BAD TYPE";
}

const char *pic16_pCodeOpType(pCodeOp *pcop)
{

  if(pcop) {

    switch(pcop->type) {

    case PO_NONE:               return "PO_NONE";
    case PO_W:                  return  "PO_W";
    case PO_WREG:               return  "PO_WREG";
    case PO_STATUS:             return  "PO_STATUS";
    case PO_BSR:                return  "PO_BSR";
    case PO_FSR0:               return  "PO_FSR0";
    case PO_INDF0:              return  "PO_INDF0";
    case PO_INTCON:             return  "PO_INTCON";
    case PO_GPR_REGISTER:       return  "PO_GPR_REGISTER";
    case PO_GPR_BIT:            return  "PO_GPR_BIT";
    case PO_GPR_TEMP:           return  "PO_GPR_TEMP";
    case PO_SFR_REGISTER:       return  "PO_SFR_REGISTER";
    case PO_PCL:                return  "PO_PCL";
    case PO_PCLATH:             return  "PO_PCLATH";
    case PO_PCLATU:             return  "PO_PCLATU";
    case PO_PRODL:              return  "PO_PRODL";
    case PO_PRODH:              return  "PO_PRODH";
    case PO_LITERAL:            return  "PO_LITERAL";
    case PO_REL_ADDR:           return "PO_REL_ADDR";
    case PO_IMMEDIATE:          return  "PO_IMMEDIATE";
    case PO_DIR:                return  "PO_DIR";
    case PO_CRY:                return  "PO_CRY";
    case PO_BIT:                return  "PO_BIT";
    case PO_STR:                return  "PO_STR";
    case PO_LABEL:              return  "PO_LABEL";
    case PO_WILD:               return  "PO_WILD";
    case PO_TWO_OPS:            return  "PO_TWO_OPS";
    }
  }

  return "BAD PO_TYPE";
}

const char *pic16_pCodeOpSubType(pCodeOp *pcop)
{

  if(pcop && (pcop->type == PO_GPR_BIT)) {

    switch(PCORB(pcop)->subtype) {

    case PO_NONE:               return "PO_NONE";
    case PO_W:                  return  "PO_W";
    case PO_WREG:               return  "PO_WREG";
    case PO_STATUS:             return  "PO_STATUS";
    case PO_BSR:                return  "PO_BSR";
    case PO_FSR0:               return  "PO_FSR0";
    case PO_INDF0:              return  "PO_INDF0";
    case PO_INTCON:             return  "PO_INTCON";
    case PO_GPR_REGISTER:       return  "PO_GPR_REGISTER";
    case PO_GPR_BIT:            return  "PO_GPR_BIT";
    case PO_GPR_TEMP:           return  "PO_GPR_TEMP";
    case PO_SFR_REGISTER:       return  "PO_SFR_REGISTER";
    case PO_PCL:                return  "PO_PCL";
    case PO_PCLATH:             return  "PO_PCLATH";
    case PO_PCLATU:             return  "PO_PCLATU";
    case PO_PRODL:              return  "PO_PRODL";
    case PO_PRODH:              return  "PO_PRODH";
    case PO_LITERAL:            return  "PO_LITERAL";
    case PO_REL_ADDR:           return "PO_REL_ADDR";
    case PO_IMMEDIATE:          return  "PO_IMMEDIATE";
    case PO_DIR:                return  "PO_DIR";
    case PO_CRY:                return  "PO_CRY";
    case PO_BIT:                return  "PO_BIT";
    case PO_STR:                return  "PO_STR";
    case PO_LABEL:              return  "PO_LABEL";
    case PO_WILD:               return  "PO_WILD";
    case PO_TWO_OPS:            return  "PO_TWO_OPS";
    }
  }

  return "BAD PO_TYPE";
}

/*-----------------------------------------------------------------*/
/* pic16_genPlusIncr :- does addition with increment if possible         */
/*-----------------------------------------------------------------*/
bool pic16_genPlusIncr (iCode *ic)
{
  unsigned int icount;
  unsigned int size = pic16_getDataSize(IC_RESULT(ic));

    FENTRY;

    DEBUGpic16_emitcode ("; ","result %s, left %s, right %s",
                         pic16_AopType(AOP_TYPE(IC_RESULT(ic))),
                         pic16_AopType(AOP_TYPE(IC_LEFT(ic))),
                         pic16_AopType(AOP_TYPE(IC_RIGHT(ic))));

    /* will try to generate an increment */
    /* if the right side is not a literal
       we cannot */
    if (AOP_TYPE(IC_RIGHT(ic)) != AOP_LIT)
      {
        return FALSE;
      }

    DEBUGpic16_emitcode ("; ","%s  %d",__FUNCTION__,__LINE__);
    /* if the literal value of the right hand side
       is greater than 2 then it is faster to add */
    if ((icount = (unsigned int)ulFromVal(AOP(IC_RIGHT(ic))->aopu.aop_lit)) > 2)
      {
        return FALSE;
      }

    /* if increment 16 bits in register */
    if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) && icount == 1)
      {
        int offset = LSB;

        switch (size)
          {
          case 2:
            pic16_emitpcode(POC_INFSNZ, pic16_popGet(AOP(IC_RESULT(ic)), offset++));
          case 1:
            pic16_emitpcode(POC_INCF, pic16_popGet(AOP(IC_RESULT(ic)), offset));
            break;

          default:
            {
              symbol *inclbl = newiTempLabel(NULL);
              pCodeOp *pcop_lbl = pic16_popGetLabel(inclbl->key);
              int label_use = 0;

              size -= 2;
              if (size)
                {
                  label_use = 1;
                }

              while (size--)
                {
                  pic16_emitpcode(POC_INCF, pic16_popGet(AOP(IC_RESULT(ic)), offset++));
                  pic16_emitpcode(POC_BNC, pcop_lbl);
                }

              pic16_emitpcode(POC_INFSNZ, pic16_popGet(AOP(IC_RESULT(ic)), offset++));
              pic16_emitpcode(POC_INCF, pic16_popGet(AOP(IC_RESULT(ic)), offset));

              if (label_use)
                {
                  pic16_emitpLabelFORCE(inclbl->key);
                }
            }
        } // switch (size)

      return TRUE;
    }

//    DEBUGpic16_emitcode ("; ","%s  %d",__FUNCTION__,__LINE__);
    /* if left is in accumulator  - probably a bit operation*/                          // VR - why this is a bit operation?!
    if( (AOP_TYPE(IC_LEFT(ic)) == AOP_ACC) &&
        (AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) ) {

      pic16_emitpcode(POC_BCF, pic16_popGet(AOP(IC_RESULT(ic)),0));
      if(icount)
        pic16_emitpcode(POC_XORLW,pic16_popGetLit(1));
      //pic16_emitcode("xorlw","1");
      else
        pic16_emitpcode(POC_ANDLW,pic16_popGetLit(1));
      //pic16_emitcode("andlw","1");

      emitSKPZ;
      pic16_emitpcode(POC_BSF, pic16_popGet(AOP(IC_RESULT(ic)),0));

      return TRUE;
    }


    /* if the sizes are greater than 1 then we cannot */
    if (AOP_SIZE(IC_RESULT(ic)) > 1 ||
        AOP_SIZE(IC_LEFT(ic)) > 1   )
        return FALSE ;

    /* If we are incrementing the same register by two: */

    if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) ) {

      while (icount--)
        pic16_emitpcode(POC_INCF, pic16_popGet(AOP(IC_RESULT(ic)),0));
      //pic16_emitcode("incf","%s,f",pic16_aopGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));

      return TRUE ;
    }

    DEBUGpic16_emitcode ("; ","couldn't increment ");

    return FALSE ;
}

/*-----------------------------------------------------------------*/
/* pic16_outBitAcc - output a bit in acc                                 */
/*-----------------------------------------------------------------*/
void pic16_outBitAcc(operand *result)
{
    symbol *tlbl = newiTempLabel(NULL);
    /* if the result is a bit */
    DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);

    assert(0); // not implemented for PIC16?

    if (AOP_TYPE(result) == AOP_CRY){
        pic16_aopPut(AOP(result),"a",0);
    }
    else {
        pic16_emitcode("jz","%05d_DS_",tlbl->key+100);
        pic16_emitcode("mov","a,#01");
        pic16_emitcode("","%05d_DS_:",tlbl->key+100);
        pic16_outAcc(result);
    }
}

/*-----------------------------------------------------------------*/
/* pic16_genPlusBits - generates code for addition of two bits           */
/*-----------------------------------------------------------------*/
void pic16_genPlusBits (iCode *ic)
{
  FENTRY;

  DEBUGpic16_emitcode ("; ","result %s, left %s, right %s",
                       pic16_AopType(AOP_TYPE(IC_RESULT(ic))),
                       pic16_AopType(AOP_TYPE(IC_LEFT(ic))),
                       pic16_AopType(AOP_TYPE(IC_RIGHT(ic))));
  /*
    The following block of code will add two bits.
    Note that it'll even work if the destination is
    the carry (C in the status register).
    It won't work if the 'Z' bit is a source or destination.
  */

  /* If the result is stored in the accumulator (w) */
  //if(strcmp(pic16_aopGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE),"a") == 0 ) {
  switch(AOP_TYPE(IC_RESULT(ic))) {
  case AOP_ACC:
    pic16_emitpcode(POC_CLRF, pic16_popCopyReg(&pic16_pc_wreg));
    pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(IC_RIGHT(ic)),0));
    pic16_emitpcode(POC_XORLW, pic16_popGetLit(1));
    pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(IC_LEFT(ic)),0));
    pic16_emitpcode(POC_XORLW, pic16_popGetLit(1));
    break;
  case AOP_REG:
    pic16_emitpcode(POC_MOVLW, pic16_popGetLit(0));
    pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(IC_RIGHT(ic)),0));
    pic16_emitpcode(POC_XORLW, pic16_popGetLit(1));
    pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(IC_LEFT(ic)),0));
    pic16_emitpcode(POC_XORLW, pic16_popGetLit(1));
    pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(IC_RESULT(ic)),0));
    break;
  default:
    pic16_emitpcode(POC_MOVLW, pic16_popGet(AOP(IC_RESULT(ic)),0));
    pic16_emitpcode(POC_BCF,   pic16_popGet(AOP(IC_RESULT(ic)),0));
    pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(IC_RIGHT(ic)),0));
    pic16_emitpcode(POC_XORWF, pic16_popGet(AOP(IC_RESULT(ic)),0));
    pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(IC_LEFT(ic)),0));
    pic16_emitpcode(POC_XORWF, pic16_popGet(AOP(IC_RESULT(ic)),0));
    break;
  }

}

#if 0
/* This is the original version of this code.
 *
 * This is being kept around for reference,
 * because I am not entirely sure I got it right...
 */
static void adjustArithmeticResult(iCode *ic)
{
    if (AOP_SIZE(IC_RESULT(ic)) == 3 &&
        AOP_SIZE(IC_LEFT(ic)) == 3   &&
        !pic16_sameRegs(AOP(IC_RESULT(ic)),AOP(IC_LEFT(ic))))
        pic16_aopPut(AOP(IC_RESULT(ic)),
               pic16_aopGet(AOP(IC_LEFT(ic)),2,FALSE,FALSE),
               2);

    if (AOP_SIZE(IC_RESULT(ic)) == 3 &&
        AOP_SIZE(IC_RIGHT(ic)) == 3   &&
        !pic16_sameRegs(AOP(IC_RESULT(ic)),AOP(IC_RIGHT(ic))))
        pic16_aopPut(AOP(IC_RESULT(ic)),
               pic16_aopGet(AOP(IC_RIGHT(ic)),2,FALSE,FALSE),
               2);

    if (AOP_SIZE(IC_RESULT(ic)) == 3 &&
        AOP_SIZE(IC_LEFT(ic)) < 3    &&
        AOP_SIZE(IC_RIGHT(ic)) < 3   &&
        !pic16_sameRegs(AOP(IC_RESULT(ic)),AOP(IC_LEFT(ic))) &&
        !pic16_sameRegs(AOP(IC_RESULT(ic)),AOP(IC_RIGHT(ic)))) {
        char buffer[5];
        SNPRINTF(buffer, sizeof(buffer), "#%d", pointerCode(getSpec(operandType(IC_LEFT(ic)))));
        pic16_aopPut(AOP(IC_RESULT(ic)),buffer,2);
    }
}
//#else
/* This is the pure and virtuous version of this code.
 * I'm pretty certain it's right, but not enough to toss the old
 * code just yet...
 */
static void adjustArithmeticResult(iCode *ic)
{
    if (opIsGptr(IC_RESULT(ic)) &&
        opIsGptr(IC_LEFT(ic))   &&
        !pic16_sameRegs(AOP(IC_RESULT(ic)),AOP(IC_LEFT(ic))))
    {
        pic16_aopPut(AOP(IC_RESULT(ic)),
               pic16_aopGet(AOP(IC_LEFT(ic)), GPTRSIZE - 1,FALSE,FALSE),
               GPTRSIZE - 1);
    }

    if (opIsGptr(IC_RESULT(ic)) &&
        opIsGptr(IC_RIGHT(ic))   &&
        !pic16_sameRegs(AOP(IC_RESULT(ic)),AOP(IC_RIGHT(ic))))
    {
        pic16_aopPut(AOP(IC_RESULT(ic)),
               pic16_aopGet(AOP(IC_RIGHT(ic)),GPTRSIZE - 1,FALSE,FALSE),
               GPTRSIZE - 1);
    }

    if (opIsGptr(IC_RESULT(ic))            &&
        AOP_SIZE(IC_LEFT(ic)) < GPTRSIZE   &&
        AOP_SIZE(IC_RIGHT(ic)) < GPTRSIZE  &&
         !pic16_sameRegs(AOP(IC_RESULT(ic)),AOP(IC_LEFT(ic))) &&
         !pic16_sameRegs(AOP(IC_RESULT(ic)),AOP(IC_RIGHT(ic)))) {
         char buffer[5];

         SNPRINTF(buffer, sizeof(buffer), "#%d", pointerCode(getSpec(operandType(IC_LEFT(ic)))));
         pic16_aopPut(AOP(IC_RESULT(ic)),buffer,GPTRSIZE - 1);
     }
}
#endif

#if 1
/*-----------------------------------------------------------------*/
/* genAddlit - generates code for addition                         */
/*-----------------------------------------------------------------*/
static void genAddLit2byte (operand *result, int offr, int lit)
{
  FENTRY;

  switch(lit & 0xff)
    {
    case 0:
      break;

    case 1:
      pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),offr));
      break;

    case 0xff:
      pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),offr));
      break;

    default:
      {
        int resultIsWREG = (AOP_TYPE(result) != AOP_STA && ! strcmp(pic16_aopGet(AOP(result), 0, TRUE, FALSE), "WREG"));

        if (resultIsWREG)
          {
            pic16_emitpcode(POC_ADDLW, pic16_popGetLit(lit & 0xff));
          }
        else
          {
            pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lit & 0xff));
            pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),offr));
          }
      }
    }
}
#endif

static void emitMOVWF(operand *reg, int offset)
{
  if(!reg)
    return;

  if (AOP_TYPE(reg) == AOP_ACC) {
    DEBUGpic16_emitcode ("; ***","%s  %d ignoring mov into W",__FUNCTION__,__LINE__);
    return;
  }

  pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(reg),offset));

}


#if 1

static void genAddLit (iCode *ic, int lit)
{

  int size,same;
  int lo, offset;

  operand *result;
  operand *left;

  FENTRY;

  left = IC_LEFT(ic);
  result = IC_RESULT(ic);
  same = pic16_sameRegs(AOP(left), AOP(result));
  size = pic16_getDataSize(result);

  if ((AOP_PCODE == AOP_TYPE(left))
          && (PO_IMMEDIATE == AOP(left)->aopu.pcop->type))
  {
      /* see #1888004 for an example case for this */
      for (offset = 0; offset < size; offset++) {
          pic16_emitpcode(POC_MOVLW, pic16_newpCodeOpImmd(AOP(left)->aopu.pcop->name,
                  offset, PCOI(AOP(left)->aopu.pcop)->index + lit, 0));
          pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result), offset));
      } // for
      return;
  } // if

  if(same) {

    /* Handle special cases first */
    if(size == 1)
      genAddLit2byte (result, 0, lit);

    else if(size == 2) {
      int hi = 0xff & (lit >> 8);
      lo = lit & 0xff;

      switch(hi) {
      case 0:

        /* lit = 0x00LL */
        DEBUGpic16_emitcode ("; hi = 0","%s  %d",__FUNCTION__,__LINE__);
        switch(lo) {
        case 0:
          break;

        case 1:
          pic16_emitpcode(POC_INFSNZ, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          break;

        case 0xff:
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCFSZW, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));

          break;
        default:
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lit&0xff));
          pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),0));
          emitSKPNC;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
        }
        break;

      case 1:
        /* lit = 0x01LL */
        DEBUGpic16_emitcode ("; hi = 1","%s  %d",__FUNCTION__,__LINE__);
        switch(lo) {
        case 0:  /* 0x0100 */
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          break;
        case 1:  /* 0x0101  */
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),0));
          emitSKPNZ;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          break;
        case 0xff: /* 0x01ff */
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCFSZW, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          break;
        default: /* 0x01LL */
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
          pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),0));
          emitSKPNC;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
        }
        break;

      case 0xff:
        DEBUGpic16_emitcode ("; hi = ff","%s  %d",__FUNCTION__,__LINE__);
        /* lit = 0xffLL */
        switch(lo) {
        case 0:  /* 0xff00 */
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),MSB16));
          break;
        case 1:  /*0xff01 */
          pic16_emitpcode(POC_INCFSZ, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),MSB16));
          break;

        case 0xff: /* 0xffff */
          pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lo));
          pic16_emitpcode(POC_ADDWF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_ADDWFC, pic16_popGet(AOP(result),MSB16));
          break;

        default:
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
          pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),0));
          emitSKPC;
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),MSB16));

        }

        break;

      default:
        DEBUGpic16_emitcode ("; hi is generic","%d   %s  %d",hi,__FUNCTION__,__LINE__);

        /* lit = 0xHHLL */
        switch(lo) {
        case 0:  /* 0xHH00 */
          genAddLit2byte (result, MSB16, hi);
          break;
        case 1:  /* 0xHH01 */
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(hi));
          pic16_emitpcode(POC_ADDWFC,pic16_popGet(AOP(result),MSB16));
          break;
/*      case 0xff: * 0xHHff *
          pic16_emitpcode(POC_MOVFW, pic16_popGet(AOP(result),0,FALSE,FALSE));
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),MSB16,FALSE,FALSE));
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(hi));
          pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),MSB16,FALSE,FALSE));
          break;
*/      default:  /* 0xHHLL */
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
          pic16_emitpcode(POC_ADDWF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(hi));
          pic16_emitpcode(POC_ADDWFC,pic16_popGet(AOP(result),MSB16));
          break;
        }

      }
    } else {
      int carry_info = 0;
      int prev_W = -1;
      /* size > 2 */
      DEBUGpic16_emitcode (";  add lit to long","%s  %d",__FUNCTION__,__LINE__);

      offset = 0;
      while (size--)
        {
        lo = BYTEofLONG(lit,0);

        if (carry_info)
          {
            if (prev_W != (lo & 0xFF))
              {
                pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lo));
                prev_W = lo & 0xFF;
              }

            pic16_emitpcode(POC_ADDWFC, pic16_popGet(AOP(result),offset));
          }
        else {
          /* no carry info from previous step */
          /* this means this is the first time to add */
          switch (lo)
            {
            case 0:
              break;

            case 1:
              pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),offset));
              carry_info = 1;
              break;

            default:
              {
                if (prev_W != (lo & 0xFF))
                  {
                    pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lo));
                    prev_W = lo & 0xFF;
                  }

                pic16_emitpcode(POC_ADDWF, pic16_popGet(AOP(result),offset));

                if (lit < 0x100)
                  carry_info = 3;  /* Were adding only one byte and propogating the carry. */
                else
                  carry_info = 2;
                break;
              }
            }
        }
        offset++;
        lit >>= 8;
      }

/*
      lo = BYTEofLONG(lit,0);

      if(lit < 0x100) {
        if(lo) {
          if(lo == 1) {
            pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),0,FALSE,FALSE));
            emitSKPNZ;
          } else {
            pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
            pic16_emitpcode(POC_ADDWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
            emitSKPNC;
          }
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),1,FALSE,FALSE));
          emitSKPNZ;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),2,FALSE,FALSE));
          emitSKPNZ;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),3,FALSE,FALSE));

        }
      }

*/
    }
  } else {
    int offset = 1;
    DEBUGpic16_emitcode (";  left and result aren't same","%s  %d",__FUNCTION__,__LINE__);

    if(size == 1) {

      if(AOP_TYPE(left) == AOP_ACC) {
        /* left addend is already in accumulator */
        switch(lit & 0xff) {
        case 0:
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
          break;
        default:
          pic16_emitpcode(POC_ADDLW, pic16_popGetLit(lit & 0xff));
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
        }
      } else {
        /* left addend is in a register */
        switch(lit & 0xff) {
        case 0:
          pic16_mov2w(AOP(left),0);
          emitMOVWF(result, 0);
          break;
        case 1:
          pic16_emitpcode(POC_INCFW, pic16_popGet(AOP(left),0));
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
          break;
        case 0xff:
          pic16_emitpcode(POC_DECFW, pic16_popGet(AOP(left),0));
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
          break;
        default:
          pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit & 0xff));
          pic16_emitpcode(POC_ADDFW, pic16_popGet(AOP(left),0));
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
        }
      }

//    } else if (pic16_isLitAop(AOP(left))) {
//      // adding two literals
//      assert ( !"adding two literals is not yet supported" );
    } else {
      int clear_carry=0;

      /* left is not the accumulator */
      if(lit & 0xff) {
        pic16_mov2w(AOP(left),0);
        pic16_emitpcode(POC_ADDLW, pic16_popGetLit(lit & 0xff));
      } else {
        pic16_mov2w(AOP(left),0);
        /* We don't know the state of the carry bit at this point */
        clear_carry = 1;
      }
      //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
      emitMOVWF(result,0);
      while(--size) {
        lit >>= 8;
        pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit & 0xff));
        if (offset < AOP_SIZE(left)) {
          pic16_emitpcode(clear_carry ? POC_ADDFW : POC_ADDFWC, pic16_popGet(AOP(left),offset));
          pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),offset));
        } else {
          pic16_emitpcode(POC_CLRF, pic16_popGet(AOP(result),offset));
          if (!SPEC_USIGN(operandType(IC_LEFT(ic)))) {
            /* sign-extend left (in result) */
            pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple(AOP(left),AOP_SIZE(left)-1,7));
            pic16_emitpcode(POC_SETF, pic16_popGet(AOP(result),offset));
          }
          pic16_emitpcode(clear_carry ? POC_ADDWF : POC_ADDWFC, pic16_popGet(AOP(result),offset));
        }
        clear_carry = 0;
        offset++;
      }
    }
  }
}

#else
    /* this fails when result is an SFR because value is written there
     * during addition and not at the end */

static void genAddLit (iCode *ic, int lit)
{

  int size,sizeL,same;
  int i, llit, prev_W;

  operand *result;
  operand *left;

    FENTRY;


  left = IC_LEFT(ic);
  result = IC_RESULT(ic);
  same = pic16_sameRegs(AOP(left), AOP(result));
  size = pic16_getDataSize(result);
  sizeL = pic16_getDataSize(left);
  llit = lit;

#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
  /* move left to result -- possibly sign extend */
  for (i=0; i < MIN(size, sizeL); i++) {
    pic16_mov2f (AOP(result), AOP(left), i);
  } // for i
#undef MIN

  /* extend to result size */
  pic16_addSign(result, sizeL, !IS_UNSIGNED(operandType(left)));

  /* special cases */
  if (lit == 0) {
    /* nothing to do */
  } else if (lit == 1) {
    switch (size) {
    case 1:
      /* handled below */
      break;
    case 2:
      pic16_emitpcode (POC_INFSNZ, pic16_popGet (AOP(result), 0));
      break;
    default:
      assert (size > 2);
      pic16_emitpcode (POC_INCF, pic16_popGet(AOP(result), 0));
      for (i=1; i < size-1; i++) {
        emitSKPNC; /* a jump here saves up to 2(size-2)cycles */
        pic16_emitpcode (POC_INCF, pic16_popGet(AOP(result), i));
      } // for i
      emitSKPNC;
      break;
    } // switch

    pic16_emitpcode (POC_INCF, pic16_popGet (AOP(result), size-1));
  } else {
    /* general case */

    /* add literal to result */
    prev_W = -1;
    for (i = 0; i < size; i++)
      {
        if (prev_W != (llit & 0xFF))
          {
            pic16_emitpcode(POC_MOVLW, pic16_popGetLit(llit));
            prev_W = llit & 0xFF;
          }

        llit >>= 8; /* FIXME: arithmetic right shift for signed literals? */
        pic16_emitpcode(i == 0 ? POC_ADDWF : POC_ADDWFC, pic16_popGet(AOP(result), i));
      }
  }

#if 0

  if(same) {

    /* Handle special cases first */
    if(size == 1)
      genAddLit2byte (result, 0, lit);

    else if(size == 2) {
      int hi = 0xff & (lit >> 8);
      lo = lit & 0xff;

      switch(hi) {
      case 0:

        /* lit = 0x00LL */
        DEBUGpic16_emitcode ("; hi = 0","%s  %d",__FUNCTION__,__LINE__);
        switch(lo) {
        case 0:
          break;
        case 1:
          pic16_emitpcode(POC_INFSNZ, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          break;
        case 0xff:
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCFSZW, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));

          break;
        default:
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lit&0xff));
          pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),0));
          emitSKPNC;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));


        }
        break;

      case 1:
        /* lit = 0x01LL */
        DEBUGpic16_emitcode ("; hi = 1","%s  %d",__FUNCTION__,__LINE__);
        switch(lo) {
        case 0:  /* 0x0100 */
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          break;
        case 1:  /* 0x0101  */
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),0));
          emitSKPNZ;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          break;
        case 0xff: /* 0x01ff */
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCFSZW, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          break;
        default: /* 0x01LL */
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
          pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),0));
          emitSKPNC;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16));
        }
        break;

      case 0xff:
        DEBUGpic16_emitcode ("; hi = ff","%s  %d",__FUNCTION__,__LINE__);
        /* lit = 0xffLL */
        switch(lo) {
        case 0:  /* 0xff00 */
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),MSB16));
          break;
        case 1:  /*0xff01 */
          pic16_emitpcode(POC_INCFSZ, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),MSB16));
          break;
/*      case 0xff: * 0xffff *
          pic16_emitpcode(POC_INCFSZW, pic16_popGet(AOP(result),0,FALSE,FALSE));
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),MSB16,FALSE,FALSE));
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          break;
*/
        default:
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
          pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),0));
          emitSKPC;
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),MSB16));

        }

        break;

      default:
        DEBUGpic16_emitcode ("; hi is generic","%d   %s  %d",hi,__FUNCTION__,__LINE__);

        /* lit = 0xHHLL */
        switch(lo) {
        case 0:  /* 0xHH00 */
          genAddLit2byte (result, MSB16, hi);
          break;
        case 1:  /* 0xHH01 */
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(hi));
          pic16_emitpcode(POC_ADDWFC,pic16_popGet(AOP(result),MSB16));
          break;
/*      case 0xff: * 0xHHff *
          pic16_emitpcode(POC_MOVFW, pic16_popGet(AOP(result),0,FALSE,FALSE));
          pic16_emitpcode(POC_DECF, pic16_popGet(AOP(result),MSB16,FALSE,FALSE));
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(hi));
          pic16_emitpcode(POC_ADDWF,pic16_popGet(AOP(result),MSB16,FALSE,FALSE));
          break;
*/      default:  /* 0xHHLL */
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
          pic16_emitpcode(POC_ADDWF, pic16_popGet(AOP(result),0));
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(hi));
          pic16_emitpcode(POC_ADDWFC,pic16_popGet(AOP(result),MSB16));
          break;
        }

      }
    } else {
      int carry_info = 0;
      int offset = 0;
      /* size > 2 */
      DEBUGpic16_emitcode (";  add lit to long","%s  %d",__FUNCTION__,__LINE__);

      while(size--) {
        lo = BYTEofLONG(lit,0);

        if(carry_info) {
          pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
          pic16_emitpcode(POC_ADDWFC, pic16_popGet(AOP(result),offset));
        }else {
          /* no carry info from previous step */
          /* this means this is the first time to add */
          switch(lo) {
          case 0:
            break;
          case 1:
            pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),offset));
            carry_info=1;
            break;
          default:
            pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
            pic16_emitpcode(POC_ADDWF, pic16_popGet(AOP(result),offset));
            if(lit <0x100)
              carry_info = 3;  /* Were adding only one byte and propogating the carry */
            else
              carry_info = 2;
            break;
          }
        }
        offset++;
        lit >>= 8;
      }

/*
      lo = BYTEofLONG(lit,0);

      if(lit < 0x100) {
        if(lo) {
          if(lo == 1) {
            pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),0,FALSE,FALSE));
            emitSKPNZ;
          } else {
            pic16_emitpcode(POC_MOVLW,pic16_popGetLit(lo));
            pic16_emitpcode(POC_ADDWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
            emitSKPNC;
          }
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),1,FALSE,FALSE));
          emitSKPNZ;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),2,FALSE,FALSE));
          emitSKPNZ;
          pic16_emitpcode(POC_INCF, pic16_popGet(AOP(result),3,FALSE,FALSE));

        }
      }

*/
    }
  } else {
    int offset = 1;
    DEBUGpic16_emitcode (";  left and result aren't same","%s  %d",__FUNCTION__,__LINE__);

    if(size == 1) {

      if(AOP_TYPE(left) == AOP_ACC) {
        /* left addend is already in accumulator */
        switch(lit & 0xff) {
        case 0:
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
          break;
        default:
          pic16_emitpcode(POC_ADDLW, pic16_popGetLit(lit & 0xff));
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
        }
      } else {
        /* left addend is in a register */
        switch(lit & 0xff) {
        case 0:
          pic16_mov2w(AOP(left),0);
          emitMOVWF(result, 0);
          break;
        case 1:
          pic16_emitpcode(POC_INCFW, pic16_popGet(AOP(left),0));
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
          break;
        case 0xff:
          pic16_emitpcode(POC_DECFW, pic16_popGet(AOP(left),0));
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
          break;
        default:
          pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit & 0xff));
          pic16_emitpcode(POC_ADDFW, pic16_popGet(AOP(left),0));
          //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
          emitMOVWF(result,0);
        }
      }

    } else {
      int clear_carry=0;

      /* left is not the accumulator */
      if(lit & 0xff) {
        pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit & 0xff));
        pic16_emitpcode(POC_ADDFW, pic16_popGet(AOP(left),0));
      } else {
        pic16_mov2w(AOP(left),0);
        /* We don't know the state of the carry bit at this point */
        clear_carry = 1;
      }
      //pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),0,FALSE,FALSE));
      emitMOVWF(result,0);
      while(--size) {

        lit >>= 8;
        if(lit & 0xff) {
          if(clear_carry) {
            /* The ls byte of the lit must've been zero - that
               means we don't have to deal with carry */

            pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit & 0xff));
            pic16_emitpcode(POC_ADDFW,  pic16_popGet(AOP(left),offset));
            pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),offset));

            clear_carry = 0;

          } else {
            pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit & 0xff));
            pic16_emitpcode(POC_ADDFWC, pic16_popGet(AOP(left),offset));
            pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),offset));
          }

        } else {
          pic16_emitpcode(POC_CLRF,  pic16_popGet(AOP(result),offset));
          pic16_mov2w(AOP(left),offset);
          pic16_emitpcode(POC_ADDWFC, pic16_popGet(AOP(result),offset));
        }
        offset++;
      }
    }
  }
#endif
}

#endif

/*-----------------------------------------------------------------*/
/* pic16_genPlus - generates code for addition                     */
/*-----------------------------------------------------------------*/
void pic16_genPlus (iCode *ic)
{
  int i, size, offset = 0;
  operand *result, *left, *right;

    FENTRY;

    /* special cases :- */
        result = IC_RESULT(ic);
        left = IC_LEFT(ic);
        right = IC_RIGHT(ic);
        pic16_aopOp (left,ic,FALSE);
        pic16_aopOp (right,ic,FALSE);
        pic16_aopOp (result,ic,TRUE);
        DEBUGpic16_pic16_AopType(__LINE__,left, right, result);
        // pic16_DumpOp("(left)",left);

        /* if literal, literal on the right or
        if left requires ACC or right is already
        in ACC */

        if ( (AOP_TYPE(left) == AOP_LIT) || (pic16_sameRegs(AOP(right), AOP(result))) ) {
                operand *t = right;
                right = IC_RIGHT(ic) = left;
                left = IC_LEFT(ic) = t;
        }

        /* if both left & right are in bit space */
        if (AOP_TYPE(left) == AOP_CRY &&
                AOP_TYPE(right) == AOP_CRY) {
                pic16_genPlusBits (ic);
                goto release ;
        }

        /* if left in bit space & right literal */
        if (AOP_TYPE(left) == AOP_CRY &&
                AOP_TYPE(right) == AOP_LIT) {
                /* if result in bit space */
                if(AOP_TYPE(result) == AOP_CRY){
                        if(ulFromVal (AOP(right)->aopu.aop_lit) != 0L) {
                                pic16_emitpcode(POC_MOVLW, pic16_popGet(AOP(result),0));
                                if (!pic16_sameRegs(AOP(left), AOP(result)) )
                                        pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(left),0));
                                pic16_emitpcode(POC_XORWF, pic16_popGet(AOP(result),0));
                        }
                } else {
                        unsigned long lit = ulFromVal (AOP(right)->aopu.aop_lit);
                        size = pic16_getDataSize(result);
                        while (size--) {
                                pic16_emitpcode (POC_CLRF, pic16_popGet (AOP(result), offset));
                                pic16_emitpcode (POC_MOVLW, pic16_popGetLit ((lit >> (8*offset)) & 0xFF));
                                pic16_emitpcode (POC_ADDWFC, pic16_popGet(AOP(result), offset++));
                                //MOVA(pic16_aopGet(AOP(right),offset,FALSE,FALSE));
                                //pic16_emitcode("addc","a,#00  ;%d",__LINE__);
                                //pic16_aopPut(AOP(result),"a",offset++);
                        }
                }
        goto release ;
        } // left == CRY

        /* if I can do an increment instead
        of add then GOOD for ME */
        if (pic16_genPlusIncr (ic) == TRUE)
                goto release;

        size = pic16_getDataSize(result);

        if(AOP(right)->type == AOP_LIT) {
                /* Add a literal to something else */
                //bool know_W=0;
                unsigned lit = (unsigned) ulFromVal (AOP(right)->aopu.aop_lit);
                //unsigned l1=0;

                //offset = 0;
                DEBUGpic16_emitcode(";","adding lit to something. size %d",size);

                genAddLit (ic,  lit);
                goto release;

        } else if(AOP_TYPE(right) == AOP_CRY) {

                pic16_emitcode(";bitadd","right is bit: %s",pic16_aopGet(AOP(right),0,FALSE,FALSE));
                pic16_emitcode(";bitadd","left is bit: %s",pic16_aopGet(AOP(left),0,FALSE,FALSE));
                pic16_emitcode(";bitadd","result is bit: %s",pic16_aopGet(AOP(result),0,FALSE,FALSE));

                /* here we are adding a bit to a char or int */
                if(size == 1) {
                        if (pic16_sameRegs(AOP(left), AOP(result)) ) {

                                pic16_emitpcode(POC_BTFSC , pic16_popGet(AOP(right),0));
                                pic16_emitpcode(POC_INCF ,  pic16_popGet(AOP(result),0));
                        } else { // not same

                                if(AOP_TYPE(left) == AOP_ACC) {
                                        pic16_emitpcode(POC_BTFSC , pic16_popGet(AOP(right),0));
                                        pic16_emitpcode(POC_XORLW , pic16_popGetLit(1));
                                } else {
                                        pic16_mov2w(AOP(left),0);
                                        pic16_emitpcode(POC_BTFSC , pic16_popGet(AOP(right),0));
                                        pic16_emitpcode(POC_INCFW , pic16_popGet(AOP(left),0));
                                }

                                if(AOP_TYPE(result) != AOP_ACC) {

                                        if(AOP_TYPE(result) == AOP_CRY) {
                                                pic16_emitpcode(POC_ANDLW , pic16_popGetLit(1));
                                                pic16_emitpcode(POC_BCF ,   pic16_popGet(AOP(result),0));
                                                emitSKPZ;
                                                pic16_emitpcode(POC_BSF ,   pic16_popGet(AOP(result),0));
                                        } else {
                                                pic16_emitpcode(POC_MOVWF ,   pic16_popGet(AOP(result),0));
                                        }
                                }
                        }

                } else {
                        int offset = 1;
                        DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
                        if (pic16_sameRegs(AOP(left), AOP(result)) ) {
                                emitCLRZ;
                                pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(right),0));
                                pic16_emitpcode(POC_INCF,  pic16_popGet(AOP(result),0));
                        } else {
                                emitCLRZ; // needed here as well: INCFW is not always executed, Z is undefined then
                                pic16_mov2w(AOP(left),0);
                                pic16_emitpcode(POC_BTFSC, pic16_popGet(AOP(right),0));
                                pic16_emitpcode(POC_INCFW, pic16_popGet(AOP(left),0));
                                emitMOVWF(right,0);
                        }

                        while(--size){
                                emitSKPZ;
                                pic16_emitpcode(POC_INCF,  pic16_popGet(AOP(result),offset++));
                        }

                }

        } else {
                // add bytes

                // Note: the following is an example of WISC code, eg.
                // it's supposed to run on a Weird Instruction Set Computer :o)

                DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);

                if ( AOP_TYPE(left) == AOP_ACC) {
                        DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
                        pic16_emitpcode(POC_ADDFW, pic16_popGet(AOP(right),0));
                        if ( AOP_TYPE(result) != AOP_ACC)
                                pic16_emitpcode(POC_MOVWF,pic16_popGet(AOP(result),0));
                        goto release; // we're done, since WREG is 1 byte
                }


                DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);

                size = min( AOP_SIZE(result), AOP_SIZE(right) );
                size = min( size, AOP_SIZE(left) );
                offset = 0;

                if(pic16_debug_verbose) {
//                      fprintf(stderr, "%s:%d result: %d\tleft: %d\tright: %d\n", __FILE__, __LINE__,
//                              AOP_SIZE(result), AOP_SIZE(left), AOP_SIZE(right));
//                      fprintf(stderr, "%s:%d size of operands: %d\n", __FILE__, __LINE__, size);
                }



                if ((AOP_TYPE(left) == AOP_PCODE) && (
                                (AOP(left)->aopu.pcop->type == PO_LITERAL) ||
//                              (AOP(left)->aopu.pcop->type == PO_DIR) ||   // patch 9
                                (AOP(left)->aopu.pcop->type == PO_IMMEDIATE)))
                {
                        // add to literal operand

                        // add first bytes
                        for(i=0; i<size; i++) {
                                if (AOP_TYPE(right) == AOP_ACC) {
                                        pic16_emitpcode(POC_ADDLW, pic16_popGet(AOP(left),i));
                                } else {
                                        pic16_emitpcode(POC_MOVLW, pic16_popGet(AOP(left),i));
                                        if(i) { // add with carry
                                                pic16_emitpcode(POC_ADDFWC, pic16_popGet(AOP(right),i));
                                        } else { // add without
                                                pic16_emitpcode(POC_ADDFW, pic16_popGet(AOP(right),i));
                                        }
                                }
                                pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),i));
                        }

                        DEBUGpic16_pic16_AopTypeSign(__LINE__, NULL, right, NULL);

                        // add leftover bytes
                        if (SPEC_USIGN(getSpec(operandType(right)))) {
                                // right is unsigned
                                for(i=size; i< AOP_SIZE(result); i++) {
                                        pic16_emitpcode(POC_CLRF, pic16_popGet(AOP(result),i));
                                        pic16_emitpcode(POC_MOVLW, pic16_popGet(AOP(left),i));
                                        pic16_emitpcode(POC_ADDWFC, pic16_popGet(AOP(result),i));
                                }

                        } else {
                                // right is signed, oh dear ...
                                for(i=size; i< AOP_SIZE(result); i++) {
                                        pic16_emitpcode(POC_CLRF, pic16_popGet(AOP(result),i));
                                        pic16_emitpcode(POC_BTFSC, pic16_newpCodeOpBit(pic16_aopGet(AOP(right),size-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
                                        pic16_emitpcode(POC_SETF, pic16_popGet(AOP(result),i));
                                        pic16_emitpcode(POC_MOVLW, pic16_popGet(AOP(left),i));
                                        pic16_emitpcode(POC_ADDWFC, pic16_popGet(AOP(result),i));
                                }

                        }
                        goto release;

                } else {
                        // add regs

                        if (pic16_sameRegs(AOP(left), AOP(result))
                            && (AOP_SIZE(left) < AOP_SIZE(result)))
                        {
                            // extend left operand, sign-bit still intact
                            pic16_addSign (result, AOP_SIZE(left), !SPEC_USIGN(getSpec(operandType(left))));
                        }

                        // add first bytes
                        for(i=0; i<size; i++) {
                                if (AOP_TYPE(right) != AOP_ACC)
                                  pic16_mov2w(AOP(right),i);
                                if (pic16_sameRegs(AOP(left), AOP(result)))
                                {
                                        if(i) { // add with carry
                                                pic16_emitpcode(POC_ADDWFC, pic16_popGet(AOP(left),i));
                                        } else { // add without
                                                pic16_emitpcode(POC_ADDWF, pic16_popGet(AOP(left),i));
                                        }
                                } else { // not same
                                        if(i) { // add with carry
                                                pic16_emitpcode(POC_ADDFWC, pic16_popGet(AOP(left),i));
                                        } else { // add without
                                                pic16_emitpcode(POC_ADDFW, pic16_popGet(AOP(left),i));
                                        }
                                        pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),i));
                                }
                        }

                        // add leftover bytes
                        // either left or right is too short
                        for (i=size; i < AOP_SIZE(result); i++) {
                          // get right operand into WREG
                          if (i < AOP_SIZE(right)) {
                            pic16_mov2w (AOP(right), i);
                          } else {
                            // right is too short, not overwritten with result
                            pic16_emitpcode (POC_CLRF, pic16_popCopyReg (&pic16_pc_wreg));
                            if (!SPEC_USIGN(getSpec(operandType(right)))) {
                              // right operand is signed
                              // Make sure that right's sign is not yet overwritten
                              assert (!pic16_sameRegs (AOP(right), AOP(result)));
                              pic16_emitpcode(POC_BTFSC, pic16_newpCodeOpBit(pic16_aopGet(AOP(right),AOP_SIZE(right)-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
                              pic16_emitpcode(POC_SETF, pic16_popCopyReg (&pic16_pc_wreg));
                            }
                          }

                          // get left+WREG+CARRY into result
                          if (pic16_sameRegs (AOP(left), AOP(result))) {
                            // left might have been extended in result above
                            pic16_emitpcode (POC_ADDWFC, pic16_popGet (AOP(result), i));
                          } else if (i < AOP_SIZE(left)) {
                            pic16_emitpcode (POC_ADDFWC, pic16_popGet (AOP(left), i));
                            pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),i));
                          } else {
                            // left is too short, not overwritten with result
                            pic16_emitpcode (POC_CLRF, pic16_popGet (AOP(result), i));
                            if (!SPEC_USIGN(getSpec(operandType(left)))) {
                              // left operand is signed
                              pic16_emitpcode(POC_BTFSC, pic16_newpCodeOpBit(pic16_aopGet(AOP(left),AOP_SIZE(left)-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
                              pic16_emitpcode(POC_SETF, pic16_popGet (AOP(result), i));
                            }
                            pic16_emitpcode (POC_ADDWFC, pic16_popGet (AOP(result), i));
                          }
                        } // for i
                        goto release;
                }

        }

        assert( 0 );

release:
        pic16_freeAsmop(left,NULL,ic,(RESULTONSTACK(ic) ? FALSE : TRUE));
        pic16_freeAsmop(right,NULL,ic,(RESULTONSTACK(ic) ? FALSE : TRUE));
        pic16_freeAsmop(result,NULL,ic,TRUE);
}

/*-----------------------------------------------------------------*/
/* pic16_genMinusDec :- does subtraction with decrement if possible     */
/*-----------------------------------------------------------------*/
bool pic16_genMinusDec (iCode *ic)
{
    unsigned int icount ;
    unsigned int size = pic16_getDataSize(IC_RESULT(ic));

    FENTRY;
    /* will try to generate an increment */
    /* if the right side is not a literal
    we cannot */
    if ((AOP_TYPE(IC_RIGHT(ic)) != AOP_LIT) ||
        (AOP_TYPE(IC_LEFT(ic)) == AOP_CRY) ||
        (AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) )
        return FALSE ;

    DEBUGpic16_emitcode ("; lit val","%d",(unsigned int) ulFromVal (AOP(IC_RIGHT(ic))->aopu.aop_lit));

    /* if the literal value of the right hand side
    is greater than 4 then it is not worth it */
    if ((icount = (unsigned int) ulFromVal (AOP(IC_RIGHT(ic))->aopu.aop_lit)) > 2)
        return FALSE ;

    /* if decrement 16 bits in register */
    if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) &&
        (size > 1) &&
        (icount == 1)) {

      if(size == 2) {
        pic16_emitpcode(POC_DECF,    pic16_popGet(AOP(IC_RESULT(ic)),LSB));
        emitSKPC;
        pic16_emitpcode(POC_DECF,    pic16_popGet(AOP(IC_RESULT(ic)),MSB16));

        pic16_emitcode("decf","%s,f",pic16_aopGet(AOP(IC_RESULT(ic)),LSB,FALSE,FALSE));
        pic16_emitcode("incfsz","%s,w",pic16_aopGet(AOP(IC_RESULT(ic)),LSB,FALSE,FALSE));
        pic16_emitcode(" decf","%s,f",pic16_aopGet(AOP(IC_RESULT(ic)),MSB16,FALSE,FALSE));
      } else {
        /* size is 3 or 4 */
        pic16_emitpcode(POC_DECF,   pic16_popGet(AOP(IC_RESULT(ic)),LSB));
        pic16_emitpcode(POC_CLRF,   pic16_popCopyReg(&pic16_pc_wreg));
        pic16_emitpcode(POC_SUBWFB_D1,   pic16_popGet(AOP(IC_RESULT(ic)),MSB16));
        pic16_emitpcode(POC_SUBWFB_D1,   pic16_popGet(AOP(IC_RESULT(ic)),MSB24));

        pic16_emitcode("movlw","0xff");
        pic16_emitcode("addwf","%s,f",pic16_aopGet(AOP(IC_RESULT(ic)),LSB,FALSE,FALSE));

        //emitSKPNC;
        pic16_emitcode("addwf","%s,f",pic16_aopGet(AOP(IC_RESULT(ic)),MSB16,FALSE,FALSE));
        //emitSKPNC;
        pic16_emitcode("addwf","%s,f",pic16_aopGet(AOP(IC_RESULT(ic)),MSB24,FALSE,FALSE));

        if(size > 3) {
          pic16_emitpcode(POC_SUBWFB_D1,   pic16_popGet(AOP(IC_RESULT(ic)),MSB32));

          pic16_emitcode("skpnc","");
          //emitSKPNC;
          pic16_emitcode("addwf","%s,f",pic16_aopGet(AOP(IC_RESULT(ic)),MSB32,FALSE,FALSE));
        }

      }

      return TRUE;

    }

    /* if the sizes are greater than 1 then we cannot */
    if (AOP_SIZE(IC_RESULT(ic)) > 1 ||
        AOP_SIZE(IC_LEFT(ic)) > 1   )
        return FALSE ;

    /* we can if the aops of the left & result match or
    if they are in registers and the registers are the
    same */
    if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic)))) {

      while (icount--)
        pic16_emitpcode(POC_DECF, pic16_popGet(AOP(IC_RESULT(ic)),0));

        //pic16_emitcode ("decf","%s,f",pic16_aopGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));

        return TRUE ;
    }

    DEBUGpic16_emitcode ("; returning"," result=%s, left=%s",
                   pic16_aopGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE),
                   pic16_aopGet(AOP(IC_LEFT(ic)),0,FALSE,FALSE));
    if(size==1) {

      pic16_emitcode("decf","%s,w",pic16_aopGet(AOP(IC_LEFT(ic)),0,FALSE,FALSE));
      pic16_emitcode("movwf","%s",pic16_aopGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));

      pic16_emitpcode(POC_DECFW,  pic16_popGet(AOP(IC_LEFT(ic)),0));
      pic16_emitpcode(POC_MOVWF,  pic16_popGet(AOP(IC_RESULT(ic)),0));

      return TRUE;
    }

    return FALSE ;
}

/*-----------------------------------------------------------------*/
/* pic16_addSign - propogate sign bit to higher bytes                    */
/*-----------------------------------------------------------------*/
void pic16_addSign(operand *result, int offset, int sign)
{
  int size = (pic16_getDataSize(result) - offset);
  DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);

  if(size > 0){
    if(sign && offset) {

      if(size == 1) {
        pic16_emitpcode(POC_CLRF,pic16_popGet(AOP(result),offset));
        pic16_emitpcode(POC_BTFSC,pic16_newpCodeOpBit(pic16_aopGet(AOP(result),offset-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
        pic16_emitpcode(POC_SETF, pic16_popGet(AOP(result),offset));
      } else {

        pic16_emitpcode(POC_MOVLW, pic16_popGetLit(0));
        pic16_emitpcode(POC_BTFSC, pic16_newpCodeOpBit(pic16_aopGet(AOP(result),offset-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
        pic16_emitpcode(POC_MOVLW, pic16_popGetLit(0xff));
        while(size--)
          pic16_emitpcode(POC_MOVWF, pic16_popGet(AOP(result),offset+size));

      }
    } else
      while(size--)
        pic16_emitpcode(POC_CLRF,pic16_popGet(AOP(result),offset++));
  }
}

/*-----------------------------------------------------------------*/
/* pic16_genMinus - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
void pic16_genMinus (iCode *ic)
{
  int size, offset = 0, same=0;
  unsigned long lit = 0L;

    FENTRY;
  pic16_aopOp (IC_LEFT(ic),ic,FALSE);
  pic16_aopOp (IC_RIGHT(ic),ic,FALSE);
  pic16_aopOp (IC_RESULT(ic),ic,TRUE);

  if (AOP_TYPE(IC_RESULT(ic)) == AOP_CRY  &&
      AOP_TYPE(IC_RIGHT(ic)) == AOP_LIT) {
    operand *t = IC_RIGHT(ic);
    IC_RIGHT(ic) = IC_LEFT(ic);
    IC_LEFT(ic) = t;
  }

  DEBUGpic16_emitcode ("; ","result %s, left %s, right %s",
                   pic16_AopType(AOP_TYPE(IC_RESULT(ic))),
                   pic16_AopType(AOP_TYPE(IC_LEFT(ic))),
                   pic16_AopType(AOP_TYPE(IC_RIGHT(ic))));

  /* special cases :- */
  /* if both left & right are in bit space */
  if (AOP_TYPE(IC_LEFT(ic)) == AOP_CRY &&
      AOP_TYPE(IC_RIGHT(ic)) == AOP_CRY) {
    pic16_genPlusBits (ic);
    goto release ;
  }

  /* if I can do an decrement instead
     of subtract then GOOD for ME */
//  if (pic16_genMinusDec (ic) == TRUE)
//    goto release;

  size = pic16_getDataSize(IC_RESULT(ic));
  same = pic16_sameRegs(AOP(IC_RIGHT(ic)), AOP(IC_RESULT(ic)));

  if(AOP(IC_RIGHT(ic))->type == AOP_LIT) {
    /* Add a literal to something else */

    lit = ulFromVal (AOP(IC_RIGHT(ic))->aopu.aop_lit);
    lit = - (long)lit;

    genAddLit ( ic,  lit);
  } else if(AOP_TYPE(IC_RIGHT(ic)) == AOP_CRY) {
    // bit subtraction

    pic16_emitcode(";bitsub","right is bit: %s",pic16_aopGet(AOP(IC_RIGHT(ic)),0,FALSE,FALSE));
    pic16_emitcode(";bitsub","left is bit: %s",pic16_aopGet(AOP(IC_LEFT(ic)),0,FALSE,FALSE));
    pic16_emitcode(";bitsub","result is bit: %s",pic16_aopGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));

    /* here we are subtracting a bit from a char or int */
    if(size == 1) {
      if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) ) {

        pic16_emitpcode(POC_BTFSC , pic16_popGet(AOP(IC_RIGHT(ic)),0));
        pic16_emitpcode(POC_DECF ,  pic16_popGet(AOP(IC_RESULT(ic)),0));
      } else {

        if(AOP_TYPE(IC_LEFT(ic)) == AOP_ACC) {
          pic16_emitpcode(POC_BTFSC , pic16_popGet(AOP(IC_RIGHT(ic)),0));
          pic16_emitpcode(POC_XORLW , pic16_popGetLit(1));
        }else  if( (AOP_TYPE(IC_LEFT(ic)) == AOP_LIT) ) {

          lit = ulFromVal (AOP(IC_LEFT(ic))->aopu.aop_lit);

          if(AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) {
            if (pic16_sameRegs(AOP(IC_RIGHT(ic)), AOP(IC_RESULT(ic))) ) {
              if(lit & 1) {
                pic16_emitpcode(POC_MOVLW , pic16_popGetLit(1));
                pic16_emitpcode(POC_XORWF , pic16_popGet(AOP(IC_RIGHT(ic)),0));
              }
            }else{
              pic16_emitpcode(POC_BCF ,     pic16_popGet(AOP(IC_RESULT(ic)),0));
              if(lit & 1)
                pic16_emitpcode(POC_BTFSS , pic16_popGet(AOP(IC_RIGHT(ic)),0));
              else
                pic16_emitpcode(POC_BTFSC , pic16_popGet(AOP(IC_RIGHT(ic)),0));
              pic16_emitpcode(POC_BSF ,     pic16_popGet(AOP(IC_RESULT(ic)),0));
            }
            goto release;
          } else {
            pic16_emitpcode(POC_MOVLW , pic16_popGetLit(lit & 0xff));
            pic16_emitpcode(POC_BTFSC , pic16_popGet(AOP(IC_RIGHT(ic)),0));
            pic16_emitpcode(POC_MOVLW , pic16_popGetLit((lit-1) & 0xff));
            //pic16_emitpcode(POC_MOVWF , pic16_popGet(AOP(IC_RESULT(ic)),0));

          }

        } else {
          pic16_mov2w(AOP(IC_LEFT(ic)),0);
          pic16_emitpcode(POC_BTFSC , pic16_popGet(AOP(IC_RIGHT(ic)),0));
          pic16_emitpcode(POC_DECFW , pic16_popGet(AOP(IC_LEFT(ic)),0));
        }

        if(AOP_TYPE(IC_RESULT(ic)) != AOP_ACC) {

          pic16_emitpcode(POC_MOVWF ,   pic16_popGet(AOP(IC_RESULT(ic)),0));

        } else  {
          pic16_emitpcode(POC_ANDLW , pic16_popGetLit(1));
/*
          pic16_emitpcode(POC_BCF ,   pic16_popGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));
          emitSKPZ;
          pic16_emitpcode(POC_BSF ,   pic16_popGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));
*/
        }

      }

    }
  } else   if((AOP(IC_LEFT(ic))->type == AOP_LIT) &&
              (AOP_TYPE(IC_RIGHT(ic)) != AOP_ACC)) {

    lit = ulFromVal (AOP(IC_LEFT(ic))->aopu.aop_lit);
    DEBUGpic16_emitcode ("; left is lit","line %d result %s, left %s, right %s",__LINE__,
                   pic16_AopType(AOP_TYPE(IC_RESULT(ic))),
                   pic16_AopType(AOP_TYPE(IC_LEFT(ic))),
                   pic16_AopType(AOP_TYPE(IC_RIGHT(ic))));


    if( (size == 1) && ((lit & 0xff) == 0) ) {
      /* res = 0 - right */
      if (pic16_sameRegs(AOP(IC_RIGHT(ic)), AOP(IC_RESULT(ic))) ) {
        pic16_emitpcode(POC_NEGF,  pic16_popGet(AOP(IC_RIGHT(ic)),0));
      } else {
        pic16_emitpcode(POC_COMFW,  pic16_popGet(AOP(IC_RIGHT(ic)),0));
        pic16_emitpcode(POC_MOVWF,  pic16_popGet(AOP(IC_RESULT(ic)),0));
        pic16_emitpcode(POC_INCF,   pic16_popGet(AOP(IC_RESULT(ic)),0));
      }
      goto release;
    }

    pic16_mov2w(AOP(IC_RIGHT(ic)),0);
    pic16_emitpcode(POC_SUBLW, pic16_popGetLit(lit & 0xff));
    pic16_emitpcode(POC_MOVWF,pic16_popGet(AOP(IC_RESULT(ic)),0));


    offset = 0;
    while(--size) {
      lit >>= 8;
      offset++;
      if(same) {
        // here we have x = lit - x   for sizeof(x)>1
        pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit & 0xff));
        pic16_emitpcode(POC_SUBFWB_D1,  pic16_popGet(AOP(IC_RESULT(ic)),offset));
      } else {
        pic16_emitpcode(POC_MOVLW, pic16_popGetLit(lit & 0xff));
        pic16_emitpcode(POC_SUBFWB_D0,  pic16_popGet(AOP(IC_RIGHT(ic)),offset));
        pic16_emitpcode(POC_MOVWF,  pic16_popGet(AOP(IC_RESULT(ic)),offset));
      }
    }


  } else {

    DEBUGpic16_emitcode ("; ","line %d result %s, left %s, right %s",__LINE__,
                   pic16_AopType(AOP_TYPE(IC_RESULT(ic))),
                   pic16_AopType(AOP_TYPE(IC_LEFT(ic))),
                   pic16_AopType(AOP_TYPE(IC_RIGHT(ic))));

    if ((AOP_SIZE(IC_LEFT(ic)) < AOP_SIZE(IC_RESULT(ic)))
            && pic16_sameRegs (AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic)))) {
        // extend left in result
        pic16_addSign (IC_RESULT(ic), AOP_SIZE(IC_LEFT(ic)), !SPEC_USIGN(getSpec(operandType(IC_LEFT(ic)))));
    }

    if ((AOP_SIZE(IC_RIGHT(ic)) < AOP_SIZE(IC_RESULT(ic)))
            && pic16_sameRegs (AOP(IC_RIGHT(ic)), AOP(IC_RESULT(ic)))) {
        // extend right in result---fails if left resides in result as well...
        assert ((IC_LEFT(ic) == IC_RIGHT(ic)) || !pic16_sameRegs (AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))));
        pic16_addSign (IC_RESULT(ic), AOP_SIZE(IC_RIGHT(ic)), !SPEC_USIGN(getSpec(operandType(IC_RIGHT(ic)))));
    }

    if(AOP_TYPE(IC_LEFT(ic)) == AOP_ACC) {
      DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
      pic16_emitpcode(POC_SUBFW, pic16_popGet(AOP(IC_RIGHT(ic)),0));
      pic16_emitpcode(POC_SUBLW, pic16_popGetLit(0));
      if ( AOP_TYPE(IC_RESULT(ic)) != AOP_ACC)
        pic16_emitpcode(POC_MOVWF,pic16_popGet(AOP(IC_RESULT(ic)),0));
    } else {

        DEBUGpic16_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
        if(AOP_TYPE(IC_RIGHT(ic)) != AOP_ACC)
          pic16_mov2w(AOP(IC_RIGHT(ic)),0);

        if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) )
          pic16_emitpcode(POC_SUBWF, pic16_popGet(AOP(IC_LEFT(ic)),0));
        else {
          if( (AOP_TYPE(IC_LEFT(ic)) == AOP_LIT) ) {
            pic16_emitpcode(POC_SUBLW, pic16_popGet(AOP(IC_LEFT(ic)),0));
          } else {
            pic16_emitpcode(POC_SUBFW, pic16_popGet(AOP(IC_LEFT(ic)),0));
          }
          if ( AOP_TYPE(IC_RESULT(ic)) != AOP_ACC) {
            if ( AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) {
              pic16_emitpcode(POC_BCF ,   pic16_popGet(AOP(IC_RESULT(ic)),0));
              emitSKPZ;
              pic16_emitpcode(POC_BSF ,   pic16_popGet(AOP(IC_RESULT(ic)),0));
            }else
              pic16_emitpcode(POC_MOVWF,pic16_popGet(AOP(IC_RESULT(ic)),0));
          }
        }
    }

    /*
      pic16_emitpcode(POC_MOVFW,  pic16_popGet(AOP(IC_RIGHT(ic)),0,FALSE,FALSE));

      if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) ) {
      pic16_emitpcode(POC_SUBFW,  pic16_popGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));
      } else {
      pic16_emitpcode(POC_SUBFW,  pic16_popGet(AOP(IC_LEFT(ic)),0,FALSE,FALSE));
      pic16_emitpcode(POC_MOVWF,  pic16_popGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));
      }
    */
    offset = 1;
    size--;

    while (size--) {
      if (pic16_sameRegs (AOP(IC_RIGHT(ic)), AOP(IC_RESULT(ic)))) {
        pic16_mov2w (AOP(IC_RESULT(ic)), offset);
      } else if (offset < AOP_SIZE(IC_RIGHT(ic)))
        pic16_mov2w(AOP(IC_RIGHT(ic)),offset);
      else {
        // right operand is too short, not overwritten with result
        pic16_emitpcode (POC_CLRF, pic16_popCopyReg (&pic16_pc_wreg));
        if (!SPEC_USIGN(operandType(IC_RIGHT(ic)))) {
          // signed -- sign extend the right operand
          pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit(pic16_aopGet(AOP(IC_RIGHT(ic)),AOP_SIZE(IC_RIGHT(ic))-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
          pic16_emitpcode (POC_SETF, pic16_popCopyReg (&pic16_pc_wreg));
        }
      }
      if (pic16_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic)))) {
        pic16_emitpcode(POC_SUBWFB_D1, pic16_popGet(AOP(IC_RESULT(ic)),offset));
      } else if (offset < AOP_SIZE(IC_LEFT(ic))) {
        pic16_emitpcode(POC_SUBWFB_D0,  pic16_popGet(AOP(IC_LEFT(ic)),offset));
        pic16_emitpcode(POC_MOVWF,  pic16_popGet(AOP(IC_RESULT(ic)),offset));
      } else {
        // left operand is too short, not overwritten with result
        pic16_emitpcode (POC_CLRF, pic16_popGet(AOP(IC_RESULT(ic)), offset));
        if (!SPEC_USIGN(operandType(IC_LEFT(ic)))) {
          // signed -- sign extend the left operand
          pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit(pic16_aopGet(AOP(IC_LEFT(ic)),AOP_SIZE(IC_LEFT(ic))-1,FALSE,FALSE),7,0, PO_GPR_REGISTER));
          pic16_emitpcode (POC_SETF, pic16_popGet(AOP(IC_RESULT(ic)), offset)); // keep CARRY/#BORROW bit intact!
        }
        pic16_emitpcode(POC_SUBWFB_D1, pic16_popGet(AOP(IC_RESULT(ic)),offset));
      }
      offset++;
    }
  }

  //    adjustArithmeticResult(ic);

 release:
  pic16_freeAsmop(IC_LEFT(ic),NULL,ic,(RESULTONSTACK(ic) ? FALSE : TRUE));
  pic16_freeAsmop(IC_RIGHT(ic),NULL,ic,(RESULTONSTACK(ic) ? FALSE : TRUE));
  pic16_freeAsmop(IC_RESULT(ic),NULL,ic,TRUE);
}


/*-----------------------------------------------------------------*
 * pic_genMult8XLit_n - multiplication of two 8-bit numbers.
 *
 *
 *-----------------------------------------------------------------*/
static void
pic16_genMult8XLit_n (operand *left, operand *right, operand *result)
{
  int lit;
  int size = AOP_SIZE (result);

  FENTRY;
  DEBUGpic16_pic16_AopType (__LINE__, left, right, result);

  if (AOP_TYPE (right) != AOP_LIT)
    {
      fprintf (stderr, "%s %d - right operand is not a literal\n", __FILE__, __LINE__);
      exit (1);
    }

  lit = (int) ulFromVal (AOP (right)->aopu.aop_lit);
  assert ((lit >= -128) && (lit < 256) );

  /* We need to adjust the high byte of the result (in PRODH) if
   * (a) literals < 0 or
   * (b) signed register operands < 0.
   *
   * If lit >= 0 and val is unsigned, MULLW will do the trick.
   *
   * If val is signed and 8 bit:
   * val = -2^7 * v_7 + SUM(2^i * v_i, i=0..6)
   *     = -2^7 * v_7 + -2^7 * v_7 + SUM(2^i * v_i, i=0..7)
   *     = -2^8 * v_7 + SUM(2^i * v_i, i=0..7)
   *     = -2^8 * v_7 + UNSIGNED(val)
   *
   * For lit >= 0:
   * lit * val = lit * UNSIGNED(val) - 2^8 * v_7 * lit
   *
   * For lit < 0, lit = l_7 .. l_0:
   *
   * lit * val = (-2^8 * l_7 + UNSIGNED(lit)) * (-2^8 * v_7 + UNSIGNED(val))
   *           = 2^8 * l_7 * 2^8 * v_7
   *             - 2^8 * l_7 * UNSIGNED(val)
   *             - 2^8 * v_7 * UNSIGNED(lit)
   *             + UNSIGNED(lit) * UNSIGNED(val)
   *           = 2^16 * l_7 * v_7
   *             - 2^8 * (l_7 * UNSIGNED(val) + v_7 * UNSIGNED(lit))
   *             + UNSIGNED(lit) * UNSIGNED(val)
   *
   * '2^16 * X' is only of interest if the result is larger than 16 * bit
   * ==> ignored.
   * '-2^8 * X' requires subtracting the original values of 'val'
   * respectively 'lit' from the high byte of the result.
   * UNSIGNED(lit) * UNSIGNED(val) can be computed via a MUL instruction.
   *
   * So, we do:
   * MOVF val, W
   * MULLW lit
   * # Now, PRODH:PRODL hold UNSIGNED(val) * UNSIGNED(lit)
   * if result > 8 bit:
   *     if lit < 0:
   *         SUBWF PRODH, F         # -2^8 * l_7 * UNSIGNED(val)
   *     MOVF PRODH, W
   *     BTFSC val, 7
   *     SUBLW lit                  # -2^8 * v_7 * UNSIGNED(lit)
   *     MOVWF result[1]            # this might overwrite val!
   * MOVFF PRODL, result[0]         # this might overwrite val!
   */

  pic16_mov2w (AOP (left), 0);
  pic16_emitpcode (POC_MULLW, pic16_popGetLit (lit & 0x00ff));

  if (size > 1)
    {
      /* Adjust PRODH for signed operands. */
      if (lit < 0)
        {
          /* left is still in WREG */
          pic16_emitpcode (POC_SUBWF, pic16_popCopyReg (&pic16_pc_prodh));
        } // if

      pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (&pic16_pc_prodh));
      if (!SPEC_USIGN (operandType (left)))
        {
          pic16_emitpcode (POC_BTFSC,
                           pic16_newpCodeOpBit (pic16_aopGet (AOP (left), 0, FALSE, FALSE),
                                                7, 0, PO_GPR_REGISTER));
          pic16_emitpcode (POC_SUBLW, pic16_popGetLit (lit & 0x00ff));
        } // if

      /* Assign result (high byte) -- this may overwrite val. */
      pic16_emitpcode (POC_MOVWF, pic16_popGet (AOP (result), 1));
    } // if

  /* Assign result (low byte) -- this may overwrite val. */
  pic16_emitpcode (POC_MOVFF,
                   pic16_popGet2p (pic16_popCopyReg (&pic16_pc_prodl),
                                   pic16_popGet (AOP (result), 0)));

  /* Must sign/zero-extend here if size > 2. */
  if (size > 2)
    {
      pic16_addSign (result, 2, !IS_UNSIGNED (operandType (left)));
    } // if
}

/*-----------------------------------------------------------------*
 * genMult8X8_n - multiplication of two 8-bit numbers.
 *
 *
 *-----------------------------------------------------------------*/
void
pic16_genMult8X8_n (operand *left, operand *right, operand *result)
{
  FENTRY;

  /* Special case: multiply by literal. */
  if (AOP_TYPE (right) == AOP_LIT)
    {
      pic16_genMult8XLit_n (left, right, result);
      return;
    }
  else if (AOP_TYPE (left) == AOP_LIT)
    {
      pic16_genMult8XLit_n (right, left, result);
      return;
    } // if

  /* cases:
     A = A x B  B = A x B
     A = B x C
     W = A x B
     W = W x B  W = B x W
   */
  /* if result == right then exchange left and right */
  if (pic16_sameRegs (AOP (result), AOP (right)))
    {
      operand *tmp;
      tmp = left;
      left = right;
      right = tmp;
    } // if

  if (AOP_TYPE (left) != AOP_ACC)
    {
      // left is not WREG
      if (AOP_TYPE (right) != AOP_ACC)
        {
          pic16_mov2w (AOP (left), 0);
          pic16_emitpcode (POC_MULWF, pic16_popGet (AOP (right), 0));
        }
      else
        {
          pic16_emitpcode (POC_MULWF, pic16_popGet (AOP (left), 0));
        } // if
    }
  else
    {
      pic16_emitpcode (POC_MULWF, pic16_popGet (AOP (right), 0));
    } // if

  /* result is in PRODL:PRODH */
  if (AOP_TYPE (result) != AOP_ACC)
    {
      int isPRODL = (AOP (result)->type != AOP_STA && ! strcmp (pic16_aopGet (AOP (result), 0, TRUE, FALSE), "_PRODL"));

      if (AOP_SIZE (result) > 1)
        {
          /* If s8 x s8 --> s16 multiplication was called for, fixup high byte.
           * (left=a1a0, right=b1b0, X1: high byte, X0: low byte)
           *
           *    a1a0 * b1b0
           * --------------
           *      a1b0 a0b0
           * a1b1 a0b1
           * ---------------
           *           a0b0  a1= 0, b1= 0 (both unsigned)
           *       -b0 a0b0  a1=-1, b1= 0 (a signed and < 0, b unsigned or >= 0)
           *       -a0 a0b0  a1= 0, b1=-1 (b signed and < 0, a unsigned or >= 0)
           *  -(a0+b0) a0b0  a1=-1, b1=-1 (a and b signed and < 0)
           *
           *  Currently, PRODH:PRODL holds a0b0 as 16 bit value; we need to
           *  subtract a0 and/or b0 from PRODH. */
          if (!IS_UNSIGNED (operandType (right)))
            {
              /* right operand (b1) signed and < 0, then subtract left op (a0) */
              pic16_mov2w (AOP (left), 0);
              pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (right), 0, 7));
              pic16_emitpcode (POC_SUBWF, pic16_popCopyReg (&pic16_pc_prodh));
            } // if

          if (!IS_UNSIGNED (getSpec (operandType (left))))
            {
              /* left operand (a1) signed and < 0, then subtract right op (b0) */
              pic16_mov2w (AOP (right), 0 );
              pic16_emitpcode (POC_BTFSC, pic16_newpCodeOpBit_simple (AOP (left), 0, 7));
              pic16_emitpcode (POC_SUBWF, pic16_popCopyReg (&pic16_pc_prodh));
            } // if

          /* Assing high byte of result -- this may overwrite one of the operands. */
          pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg(&pic16_pc_prodh), pic16_popGet (AOP (result), 1)));
        } // if

      /* Assign low byte of result -- this may overwrite one of the operands. */
      if (!isPRODL)
        {
          pic16_emitpcode (POC_MOVFF, pic16_popGet2p (pic16_popCopyReg (&pic16_pc_prodl), pic16_popGet (AOP (result), 0)));
        } // if

      /* Must sign/zero-extend here if size > 2. */
      if (AOP_SIZE (result) > 2)
        {
          pic16_addSign (result, 2, !IS_UNSIGNED (operandType (left)));
        } // if
    }
  else
    {
      pic16_emitpcode (POC_MOVFW, pic16_popCopyReg (&pic16_pc_prodl));
    }
}
