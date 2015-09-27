/*-------------------------------------------------------------------------
  genarith.c - source file for code generation - arithmetic 
  
  Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)
         and -  Jean-Louis VERN.jlvern@writeme.com (1999)
  Bug Fixes  -  Wojciech Stryjewski  wstryj1@tiger.lsu.edu (1999 v2.1.9a)
  PIC port   -  Scott Dattalo scott@dattalo.com (2000)
  
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
  000123 mlh  Moved aopLiteral to SDCCglue.c to help the split
      Made everything static
-------------------------------------------------------------------------*/

#if defined(_MSC_VER) && (_MSC_VER < 1300)
#define __FUNCTION__    __FILE__
#endif

#include "common.h"
#include "newalloc.h"
//#include "SDCCglobl.h"
//#include "SDCCpeeph.h"

#include "gen.h"
#include "pcode.h"
#include "ralloc.h"


#define BYTEofLONG(l,b) ( (l>> (b<<3)) & 0xff)

const char *AopType(short type)
{
        switch(type) {
        case AOP_LIT:
                return "AOP_LIT";
                break;
        case AOP_REG:
                return "AOP_REG";
                break;
        case AOP_DIR:
                return "AOP_DIR";
                break;
        case AOP_STK:
                return "AOP_STK";
                break;
        case AOP_IMMD:
                return "AOP_IMMD";
                break;
        case AOP_STR:
                return "AOP_STR";
                break;
        case AOP_CRY:
                return "AOP_CRY";
                break;
        case AOP_PCODE:
                return "AOP_PCODE";
                break;
        }
        
        return "BAD TYPE";
}

const char *pCodeOpType(pCodeOp *pcop)
{
        
        if(pcop) {
                
                switch(pcop->type) {
                        
                case  PO_NONE:
                        return "PO_NONE";
                case  PO_W:
                        return  "PO_W";
                case  PO_STATUS:
                        return  "PO_STATUS";
                case  PO_FSR:
                        return  "PO_FSR";
                case  PO_INDF:
                        return  "PO_INDF";
                case  PO_INTCON:
                        return  "PO_INTCON";
                case  PO_GPR_REGISTER:
                        return  "PO_GPR_REGISTER";
                case  PO_GPR_POINTER:
                        return  "PO_GPR_POINTER";
                case  PO_GPR_BIT:
                        return  "PO_GPR_BIT";
                case  PO_GPR_TEMP:
                        return  "PO_GPR_TEMP";
                case  PO_SFR_REGISTER:
                        return  "PO_SFR_REGISTER";
                case  PO_PCL:
                        return  "PO_PCL";
                case  PO_PCLATH:
                        return  "PO_PCLATH";
                case  PO_LITERAL:
                        return  "PO_LITERAL";
                case  PO_IMMEDIATE:
                        return  "PO_IMMEDIATE";
                case  PO_DIR:
                        return  "PO_DIR";
                case  PO_CRY:
                        return  "PO_CRY";
                case  PO_BIT:
                        return  "PO_BIT";
                case  PO_STR:
                        return  "PO_STR";
                case  PO_LABEL:
                        return  "PO_LABEL";
                case  PO_WILD:
                        return  "PO_WILD";
                }
        }
        
        return "BAD PO_TYPE";
}

/*-----------------------------------------------------------------*/
/* genPlusIncr :- does addition with increment if possible         */
/*-----------------------------------------------------------------*/
static bool genPlusIncr (iCode *ic)
{
        unsigned int icount ;
        unsigned int size = pic14_getDataSize(IC_RESULT(ic));
        FENTRY;
        
        DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
        DEBUGpic14_emitcode ("; ","result %s, left %s, right %s",
                AopType(AOP_TYPE(IC_RESULT(ic))),
                AopType(AOP_TYPE(IC_LEFT(ic))),
                AopType(AOP_TYPE(IC_RIGHT(ic))));
        
        /* will try to generate an increment */
        /* if the right side is not a literal 
        we cannot */
        if (AOP_TYPE(IC_RIGHT(ic)) != AOP_LIT)
                return FALSE ;
        
        DEBUGpic14_emitcode ("; ","%s  %d",__FUNCTION__,__LINE__);
        /* if the literal value of the right hand side
        is greater than 1 then it is faster to add */
        if ((icount = (unsigned int) ulFromVal (AOP(IC_RIGHT(ic))->aopu.aop_lit)) > 2)
                return FALSE ;
        
        /* if increment 16 bits in register */
        if (pic14_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) &&
                (icount == 1)) {
                
                int offset = MSB16;
                
                emitpcode(POC_INCF, popGet(AOP(IC_RESULT(ic)),LSB));
                //pic14_emitcode("incf","%s,f",aopGet(AOP(IC_RESULT(ic)),LSB,FALSE,FALSE));
                
                while(--size) {
                        emitSKPNZ;
                        emitpcode(POC_INCF, popGet(AOP(IC_RESULT(ic)),offset++));
                        //pic14_emitcode(" incf","%s,f",aopGet(AOP(IC_RESULT(ic)),offset++,FALSE,FALSE));
                }
                
                return TRUE;
        }
        
        DEBUGpic14_emitcode ("; ","%s  %d",__FUNCTION__,__LINE__);
        /* if left is in accumulator  - probably a bit operation*/
        if( strcmp(aopGet(AOP(IC_LEFT(ic)),0,FALSE,FALSE),"a")  &&
                (AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) ) {
                
                emitpcode(POC_BCF, popGet(AOP(IC_RESULT(ic)),0));
                pic14_emitcode("bcf","(%s >> 3), (%s & 7)",
                        AOP(IC_RESULT(ic))->aopu.aop_dir,
                        AOP(IC_RESULT(ic))->aopu.aop_dir);
                if(icount)
                        emitpcode(POC_XORLW,popGetLit(1));
                //pic14_emitcode("xorlw","1");
                else
                        emitpcode(POC_ANDLW,popGetLit(1));
                //pic14_emitcode("andlw","1");
                
                emitSKPZ;
                emitpcode(POC_BSF, popGet(AOP(IC_RESULT(ic)),0));
                pic14_emitcode("bsf","(%s >> 3), (%s & 7)",
                        AOP(IC_RESULT(ic))->aopu.aop_dir,
                        AOP(IC_RESULT(ic))->aopu.aop_dir);
                
                return TRUE;
        }
        
        
        
        /* if the sizes are greater than 1 then we cannot */
        if (AOP_SIZE(IC_RESULT(ic)) > 1 ||
                AOP_SIZE(IC_LEFT(ic)) > 1   )
                return FALSE ;
        
        /* If we are incrementing the same register by two: */
        
        if (pic14_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) ) {
                
                while (icount--) 
                        emitpcode(POC_INCF, popGet(AOP(IC_RESULT(ic)),0));
                //pic14_emitcode("incf","%s,f",aopGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));
                
                return TRUE ;
        }
        
        DEBUGpic14_emitcode ("; ","couldn't increment ");
        
        return FALSE ;
}

/*-----------------------------------------------------------------*/
/* genAddlit - generates code for addition                         */
/*-----------------------------------------------------------------*/
static void genAddLit2byte (operand *result, int offr, int lit)
{
        FENTRY;
        
        switch(lit & 0xff) {
        case 0:
                break;
        case 1:
                emitpcode(POC_INCF, popGet(AOP(result),offr));
                break;
        case 0xff:
                emitpcode(POC_DECF, popGet(AOP(result),offr));
                break;
        default:
                emitpcode(POC_MOVLW,popGetLit(lit&0xff));
                emitpcode(POC_ADDWF,popGet(AOP(result),offr));
        }
        
}

static void emitMOVWF(operand *reg, int offset)
{
        FENTRY;
        if(!reg)
                return;
        
        emitpcode(POC_MOVWF, popGet(AOP(reg),offset));
        
}

static void genAddLit (iCode *ic, int lit)
{
  int size, same;
  int lo;

  operand *result;
  operand *left;

  FENTRY;
  DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);

  left = IC_LEFT(ic);
  result = IC_RESULT(ic);
  same = pic14_sameRegs(AOP(left), AOP(result));
  size = pic14_getDataSize(result);
  if (size > pic14_getDataSize(left))
    {
      size = pic14_getDataSize(left);
    }

  /*
   * Fix accessing libsdcc/<*>/idata.c:_cinit in __code space.
   */
  if (AOP_PCODE == AOP_TYPE(IC_LEFT(ic)))
    {
      int u;
      if (debug_verbose)
        {
          printf("%s:%u: CHECK: using address of '%s' instead of contents\n",
                 ic->filename, ic->lineno,
                 popGetAddr(AOP(IC_LEFT(ic)), 0, lit & 0xff)->name);
        } // if
      for (u = 0; u < size; ++u)
        {
          emitpcode(POC_MOVLW, popGetAddr(AOP(IC_LEFT(ic)), u, lit));
          emitpcode(POC_MOVWF, popGet(AOP(IC_RESULT(ic)), u));
        } // for

      if (size < pic14_getDataSize(result))
        {
          for (u = size; u < pic14_getDataSize(result); ++u)
            {
              /* XXX: Might fail for u >= size?!? */
              emitpcode(POC_MOVLW, popGetAddr(AOP(IC_LEFT(ic)), u, lit));
              emitpcode(POC_MOVWF, popGet(AOP(IC_RESULT(ic)), u));
            } // for
        } // if

      goto out;
    } // if

  if (same)
    {
      /* Handle special cases first */
      if (size == 1) 
        {
          genAddLit2byte (result, 0, lit);
        }
      else if (size == 2)
        {
          int hi = (lit >> 8) & 0xff;
          lo = lit & 0xff;

          switch (hi)
            {
              case 0: 
                  /* lit = 0x00LL */
                  DEBUGpic14_emitcode ("; hi = 0","%s  %d",__FUNCTION__,__LINE__);
                  switch (lo)
                    {
                      case 0:
                          break;
                      case 1:
                          emitpcode(POC_INCF, popGet(AOP(result),0));
                          emitSKPNZ;
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          break;
                      case 0xff:
                          emitpcode(POC_DECF, popGet(AOP(result),0));
                          emitpcode(POC_INCFSZW, popGet(AOP(result),0));
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          break;
                      default:
                          emitpcode(POC_MOVLW,popGetLit(lit&0xff));
                          emitpcode(POC_ADDWF,popGet(AOP(result),0));
                          emitSKPNC;
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          break;
                    } // switch
                  break;

              case 1:
                  /* lit = 0x01LL */
                  DEBUGpic14_emitcode ("; hi = 1","%s  %d",__FUNCTION__,__LINE__);
                  switch (lo)
                    {
                      case 0:  /* 0x0100 */
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          break;
                      case 1:  /* 0x0101  */
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          emitpcode(POC_INCF, popGet(AOP(result),0));
                          emitSKPNZ;
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          break;
                      case 0xff: /* 0x01ff */
                          emitpcode(POC_DECF, popGet(AOP(result),0));
                          emitpcode(POC_INCFSZW, popGet(AOP(result),0));
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          break;
                      default:
                          emitpcode(POC_MOVLW, popGetLit(lo));
                          emitpcode(POC_ADDWF, popGet(AOP(result),0));
                          emitSKPNC;
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          emitpcode(POC_INCF, popGet(AOP(result),MSB16));
                          break;
                    }    // switch
                  break;

              case 0xff:
                  DEBUGpic14_emitcode ("; hi = ff","%s  %d",__FUNCTION__,__LINE__);
                  /* lit = 0xffLL */
                  switch (lo)
                    {
                      case 0:  /* 0xff00 */
                          emitpcode(POC_DECF, popGet(AOP(result),MSB16));
                          break;
                      case 1:  /*0xff01 */
                          emitpcode(POC_INCFSZ, popGet(AOP(result),0));
                          emitpcode(POC_DECF, popGet(AOP(result),MSB16));
                          break;
                      default:
                          emitpcode(POC_MOVLW,popGetLit(lo));
                          emitpcode(POC_ADDWF,popGet(AOP(result),0));
                          emitSKPC;
                          emitpcode(POC_DECF, popGet(AOP(result),MSB16));
                          break;
                    } // switch
                  break;

              default:
                  DEBUGpic14_emitcode ("; hi is generic","%d   %s  %d",hi,__FUNCTION__,__LINE__);

                  /* lit = 0xHHLL */
                  switch (lo)
                    {
                      case 0:  /* 0xHH00 */
                          genAddLit2byte (result, MSB16, hi);
                          break;
                      case 1:  /* 0xHH01 */
                          emitpcode(POC_MOVLW,popGetLit((hi+1)&0xff));
                          emitpcode(POC_INCFSZ, popGet(AOP(result),0));
                          emitpcode(POC_MOVLW,popGetLit(hi));
                          emitpcode(POC_ADDWF,popGet(AOP(result),MSB16));
                          break;
                      default:  /* 0xHHLL */
                          emitpcode(POC_MOVLW,popGetLit(lo));
                          emitpcode(POC_ADDWF, popGet(AOP(result),0));
                          emitpcode(POC_MOVLW,popGetLit(hi));
                          emitSKPNC;
                          emitpcode(POC_MOVLW,popGetLit((hi+1) & 0xff));
                          emitpcode(POC_ADDWF,popGet(AOP(result),MSB16));
                          break;
                    } // switch
                  break;
            } // switch
        }
      else
        {
          int carry_info = 0;
          int offset = 0;
          /* size > 2 */
          DEBUGpic14_emitcode (";  add lit to long","%s  %d",__FUNCTION__,__LINE__);

          while (size--)
            {
              lo = BYTEofLONG(lit,0);

              if (carry_info)
                {
                  switch (lo)
                    {
                      case 0:
                          switch (carry_info)
                            {
                              case 1:
                                  emitSKPNZ;
                                  emitpcode(POC_INCF, popGet(AOP(result),offset));
                                  break;
                              case 2:
                                  emitpcode(POC_RLFW, popGet(AOP(result),offset));
                                  emitpcode(POC_ANDLW,popGetLit(1));
                                  emitpcode(POC_ADDWF, popGet(AOP(result),offset));
                                  break;
                              default: /* carry_info = 3  */
                                  emitSKPNC;
                                  emitpcode(POC_INCF, popGet(AOP(result),offset));
                                  carry_info = 1;
                                  break;
                            } // switch
                          break;
                      case 0xff:
                          emitpcode(POC_MOVLW,popGetLit(lo));
                          if (carry_info==1) 
                            emitSKPZ;
                          else
                            emitSKPC;
                          emitpcode(POC_ADDWF, popGet(AOP(result),offset));
                          break;
                      default:
                          emitpcode(POC_MOVLW,popGetLit(lo));
                          if (carry_info==1) 
                            emitSKPNZ;
                          else
                            emitSKPNC;
                          emitpcode(POC_MOVLW,popGetLit(lo+1));
                          emitpcode(POC_ADDWF, popGet(AOP(result),offset));
                          carry_info=2;
                          break;
                    } // switch
                }
              else
                {
                  /* no carry info from previous step */
                  /* this means this is the first time to add */
                  switch (lo)
                    {
                      case 0:
                          break;
                      case 1:
                          emitpcode(POC_INCF, popGet(AOP(result),offset));
                          carry_info=1;
                          break;
                      default:
                          emitpcode(POC_MOVLW,popGetLit(lo));
                          emitpcode(POC_ADDWF, popGet(AOP(result),offset));
                          if (lit <0x100) 
                            carry_info = 3;  /* Were adding only one byte and propogating the carry */
                          else
                            carry_info = 2;
                          break;
                    } // switch
                } // if
              offset++;
              lit >>= 8;
            } // while
        } // if
    }
  else
    {
      int offset = 1;
      DEBUGpic14_emitcode (";  left and result aren't same","%s  %d",__FUNCTION__,__LINE__);

      if (size == 1)
        {
          /* left addend is in a register */
          switch (lit & 0xff)
            {
              case 0:
                  emitpcode(POC_MOVFW, popGet(AOP(left),0));
                  emitMOVWF(result,0);
                  break;
              case 1:
                  emitpcode(POC_INCFW, popGet(AOP(left),0));
                  emitMOVWF(result,0);
                  break;
              case 0xff:
                  emitpcode(POC_DECFW, popGet(AOP(left),0));
                  emitMOVWF(result,0);
                  break;
              default:
                  emitpcode(POC_MOVLW, popGetLit(lit & 0xff));
                  emitpcode(POC_ADDFW, popGet(AOP(left),0));
                  emitMOVWF(result,0);
            } // switch
        }
      else
        {
          int clear_carry=0;

          /* left is not the accumulator */
          if (lit & 0xff)
            {
              emitpcode(POC_MOVLW, popGetLit(lit & 0xff));
              emitpcode(POC_ADDFW, popGet(AOP(left),0));
            }
          else
            {
              emitpcode(POC_MOVFW, popGet(AOP(left),0));
              /* We don't know the state of the carry bit at this point */
              clear_carry = 1;
            } // if
          emitMOVWF(result,0);
          while (--size)
            {
              lit >>= 8;
              if (lit & 0xff)
                {
                  if (clear_carry)
                    {
                      /* The ls byte of the lit must've been zero - that 
                         means we don't have to deal with carry */

                      emitpcode(POC_MOVLW, popGetLit(lit & 0xff));
                      emitpcode(POC_ADDFW,  popGet(AOP(left),offset));
                      emitpcode(POC_MOVWF, popGet(AOP(result),offset));

                      clear_carry = 0;
                    }
                  else
                    {
                      emitpcode(POC_MOVLW, popGetLit(lit & 0xff));
                      emitMOVWF(result,offset);
                      emitpcode(POC_MOVFW, popGet(AOP(left),offset));
                      emitSKPNC;
                      emitpcode(POC_INCFSZW,popGet(AOP(left),offset));
                      emitpcode(POC_ADDWF,  popGet(AOP(result),offset));
                    } // if
                }
              else
                {
                  emitpcode(POC_CLRF,  popGet(AOP(result),offset));
                  emitpcode(POC_RLF,   popGet(AOP(result),offset));
                  emitpcode(POC_MOVFW, popGet(AOP(left),offset));
                  emitpcode(POC_ADDWF, popGet(AOP(result),offset));
                } // if
              offset++;
            } // while
        } // if
    } // if

out:
  size = pic14_getDataSize(result);
  if (size > pic14_getDataSize(left))
    {
      size = pic14_getDataSize(left);
    } // if
  addSign(result, size, 0);
}

/*-----------------------------------------------------------------*/
/* genPlus - generates code for addition                           */
/*-----------------------------------------------------------------*/
void genPlus (iCode *ic)
{
        int size, offset = 0;
        
        /* special cases :- */
        DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
        FENTRY;
        
        aopOp (IC_LEFT(ic),ic,FALSE);
        aopOp (IC_RIGHT(ic),ic,FALSE);
        aopOp (IC_RESULT(ic),ic,TRUE);
        
        DEBUGpic14_AopType(__LINE__,IC_LEFT(ic),IC_RIGHT(ic),IC_RESULT(ic));
        
        /* if literal, literal on the right or
        if left requires ACC or right is already
        in ACC */
        
        if (AOP_TYPE(IC_LEFT(ic)) == AOP_LIT) {
                operand *t = IC_RIGHT(ic);
                IC_RIGHT(ic) = IC_LEFT(ic);
                IC_LEFT(ic) = t;
        }
        
        /* if left in bit space & right literal */
        if (AOP_TYPE(IC_LEFT(ic)) == AOP_CRY &&
                AOP_TYPE(IC_RIGHT(ic)) == AOP_LIT) {
                /* if result in bit space */
                if(AOP_TYPE(IC_RESULT(ic)) == AOP_CRY){
                        if(ulFromVal (AOP(IC_RIGHT(ic))->aopu.aop_lit) != 0L) {
                                emitpcode(POC_MOVLW, popGet(AOP(IC_RESULT(ic)),0));
                                if (!pic14_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) )
                                        emitpcode(POC_BTFSC, popGet(AOP(IC_LEFT(ic)),0));
                                emitpcode(POC_XORWF, popGet(AOP(IC_RESULT(ic)),0));
                        }
                } else {
                        size = pic14_getDataSize(IC_RESULT(ic));
                        while (size--) {
                                MOVA(aopGet(AOP(IC_RIGHT(ic)),offset,FALSE,FALSE));  
                                pic14_emitcode("addc","a,#00  ;%d",__LINE__);
                                aopPut(AOP(IC_RESULT(ic)),"a",offset++);
                        }
                }
                goto release ;
        }
        
        /* if I can do an increment instead
        of add then GOOD for ME */
        if (genPlusIncr (ic) == TRUE)
                goto release;   
        
        size = pic14_getDataSize(IC_RESULT(ic));
        
        if(AOP(IC_RIGHT(ic))->type == AOP_LIT) {
                /* Add a literal to something else */
                unsigned lit = (unsigned) ulFromVal (AOP(IC_RIGHT(ic))->aopu.aop_lit);
                DEBUGpic14_emitcode(";","adding lit to something. size %d",size);
                
                genAddLit (ic,  lit);
                goto release;
                
        } else if(AOP_TYPE(IC_RIGHT(ic)) == AOP_CRY) {
                
                pic14_emitcode(";bitadd","right is bit: %s",aopGet(AOP(IC_RIGHT(ic)),0,FALSE,FALSE));
                pic14_emitcode(";bitadd","left is bit: %s",aopGet(AOP(IC_LEFT(ic)),0,FALSE,FALSE));
                pic14_emitcode(";bitadd","result is bit: %s",aopGet(AOP(IC_RESULT(ic)),0,FALSE,FALSE));
                
                /* here we are adding a bit to a char or int */
                if(size == 1) {
                        if (pic14_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) ) {
                                emitpcode(POC_BTFSC , popGet(AOP(IC_RIGHT(ic)),0));
                                emitpcode(POC_INCF ,  popGet(AOP(IC_RESULT(ic)),0));
                        } else {
                                
                                emitpcode(POC_MOVFW , popGet(AOP(IC_LEFT(ic)),0));
                                emitpcode(POC_BTFSC , popGet(AOP(IC_RIGHT(ic)),0));
                                emitpcode(POC_INCFW , popGet(AOP(IC_LEFT(ic)),0));
                                
                                if(AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) {
                                        emitpcode(POC_ANDLW , popGetLit(1));
                                        emitpcode(POC_BCF ,   popGet(AOP(IC_RESULT(ic)),0));
                                        emitSKPZ;
                                        emitpcode(POC_BSF ,   popGet(AOP(IC_RESULT(ic)),0));
                                } else {
                                        emitpcode(POC_MOVWF ,   popGet(AOP(IC_RESULT(ic)),0));
                                }
                        }
                        
                } else {
                        int offset = 1;
                        DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
                        if (pic14_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) ) {
                                emitCLRZ;
                                emitpcode(POC_BTFSC, popGet(AOP(IC_RIGHT(ic)),0));
                                emitpcode(POC_INCF,  popGet(AOP(IC_RESULT(ic)),0));
                        } else {
                                
                                emitpcode(POC_MOVFW, popGet(AOP(IC_LEFT(ic)),0));
                                emitpcode(POC_BTFSC, popGet(AOP(IC_RIGHT(ic)),0));
                                emitpcode(POC_INCFW, popGet(AOP(IC_LEFT(ic)),0));
                                emitMOVWF(IC_RIGHT(ic),0);
                        }
                        
                        while(--size){
                                emitSKPZ;
                                emitpcode(POC_INCF,  popGet(AOP(IC_RESULT(ic)),offset++));
                        }
                        
                }
                
        } else {
                DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);    
                
                /* Add the first bytes */
                
                if(strcmp(aopGet(AOP(IC_LEFT(ic)),0,FALSE,FALSE),"a") == 0 ) {
                        emitpcode(POC_ADDFW, popGet(AOP(IC_RIGHT(ic)),0));
                        emitpcode(POC_MOVWF,popGet(AOP(IC_RESULT(ic)),0));
                } else {
                        
                        emitpcode(POC_MOVFW,popGet(AOP(IC_RIGHT(ic)),0));

                        if (pic14_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) )
                                emitpcode(POC_ADDWF, popGet(AOP(IC_LEFT(ic)),0));
                        else {
                                PIC_OPCODE poc = POC_ADDFW;
                                
                                if (op_isLitLike (IC_LEFT (ic)))
                                        poc = POC_ADDLW;
                                emitpcode(poc, popGetAddr(AOP(IC_LEFT(ic)),0,0));
                                emitpcode(POC_MOVWF,popGet(AOP(IC_RESULT(ic)),0));
                        }
                }
                
                size = min( AOP_SIZE(IC_RESULT(ic)), AOP_SIZE(IC_RIGHT(ic))) - 1;
                offset = 1;
                
                
                if(size){
                        if (pic14_sameRegs(AOP(IC_RIGHT(ic)), AOP(IC_RESULT(ic)))) {
                                if (op_isLitLike (IC_LEFT(ic)))
                                {
                                        while(size--){
                                                emitpcode(POC_MOVFW,   popGet(AOP(IC_RIGHT(ic)),offset));
                                                emitSKPNC;
                                                emitpcode(POC_INCFSZW, popGet(AOP(IC_RIGHT(ic)),offset));
                                                emitpcode(POC_ADDLW,   popGetAddr(AOP(IC_LEFT(ic)),offset,0));
                                                emitpcode(POC_MOVWF,   popGet(AOP(IC_RESULT(ic)),offset));
                                                offset++;
                                        }
                                } else {
                                        while(size--){
                                                emitpcode(POC_MOVFW,   popGet(AOP(IC_LEFT(ic)),offset));
                                                emitSKPNC;
                                                emitpcode(POC_INCFSZW, popGet(AOP(IC_LEFT(ic)),offset));
                                                emitpcode(POC_ADDWF,   popGet(AOP(IC_RESULT(ic)),offset));
                                                offset++;
                                        }
                                }
                        } else {
                                PIC_OPCODE poc = POC_MOVFW;
                                if (op_isLitLike (IC_LEFT(ic)))
                                        poc = POC_MOVLW;
                                while(size--){
                                        if (!pic14_sameRegs(AOP(IC_LEFT(ic)), AOP(IC_RESULT(ic))) ) {
                                                emitpcode(poc, popGetAddr(AOP(IC_LEFT(ic)),offset,0));
                                                emitpcode(POC_MOVWF, popGet(AOP(IC_RESULT(ic)),offset));
                                        }
                                        emitpcode(POC_MOVFW,   popGet(AOP(IC_RIGHT(ic)),offset));
                                        emitSKPNC;
                                        emitpcode(POC_INCFSZW, popGet(AOP(IC_RIGHT(ic)),offset));
                                        emitpcode(POC_ADDWF,   popGet(AOP(IC_RESULT(ic)),offset));
                                        offset++;
                                }
                        }
                }
        }
        
        if (AOP_SIZE(IC_RESULT(ic)) > AOP_SIZE(IC_RIGHT(ic))) {
                int sign =  !(SPEC_USIGN(getSpec(operandType(IC_LEFT(ic)))) |
                        SPEC_USIGN(getSpec(operandType(IC_RIGHT(ic)))) );
                
                
                /* Need to extend result to higher bytes */
                size = AOP_SIZE(IC_RESULT(ic)) - AOP_SIZE(IC_RIGHT(ic)) - 1;
                
                /* First grab the carry from the lower bytes */
                if (AOP_SIZE(IC_LEFT(ic)) > AOP_SIZE(IC_RIGHT(ic))) { 
                        int leftsize = AOP_SIZE(IC_LEFT(ic)) - AOP_SIZE(IC_RIGHT(ic));
                        PIC_OPCODE poc = POC_MOVFW;
                        if (op_isLitLike (IC_LEFT(ic)))
                                poc = POC_MOVLW;
                        while(leftsize-- > 0) {
                                emitpcode(poc, popGetAddr(AOP(IC_LEFT(ic)),offset,0));
                                emitSKPNC;
                                emitpcode(POC_ADDLW, popGetLit(0x01));
                                emitpcode(POC_MOVWF, popGet(AOP(IC_RESULT(ic)),offset));
                                //emitSKPNC;
                                //emitpcode(POC_INCF, popGet(AOP(IC_RESULT(ic)),offset)); /* INCF does not update Carry! */
                                offset++;
                                if (size)
                                        size--;
                                else
                                        break;
                        }
                } else {
                        emitpcode(POC_CLRF, popGet(AOP(IC_RESULT(ic)),offset));
                        emitpcode(POC_RLF,  popGet(AOP(IC_RESULT(ic)),offset));
                }
                
                
                if(sign && offset > 0 && offset < AOP_SIZE(IC_RESULT(ic))) {
                /* Now this is really horrid. Gotta check the sign of the addends and propogate
                        * to the result */
                        
                        emitpcode(POC_BTFSC, newpCodeOpBit(aopGet(AOP(IC_LEFT(ic)),offset-1,FALSE,FALSE),7,0));
                        emitpcode(POC_DECF,  popGet(AOP(IC_RESULT(ic)),offset));
                        emitpcode(POC_BTFSC, newpCodeOpBit(aopGet(AOP(IC_RIGHT(ic)),offset-1,FALSE,FALSE),7,0));
                        emitpcode(POC_DECF,  popGet(AOP(IC_RESULT(ic)),offset));
                        
                        /* if chars or ints or being signed extended to longs: */
                        if(size) {
                                emitpcode(POC_MOVLW, popGetLit(0));
                                emitpcode(POC_BTFSC, newpCodeOpBit(aopGet(AOP(IC_RESULT(ic)),offset,FALSE,FALSE),7,0));
                                emitpcode(POC_MOVLW, popGetLit(0xff));
                        }
                }
                
                offset++;
                while(size--) {
                        
                        if(sign)
                                emitpcode(POC_MOVWF, popGet(AOP(IC_RESULT(ic)),offset));
                        else
                                emitpcode(POC_CLRF,  popGet(AOP(IC_RESULT(ic)),offset));
                        
                        offset++;
                }
        }
        
        
        //adjustArithmeticResult(ic);
        
release:
        freeAsmop(IC_LEFT(ic),NULL,ic,(RESULTONSTACK(ic) ? FALSE : TRUE));
        freeAsmop(IC_RIGHT(ic),NULL,ic,(RESULTONSTACK(ic) ? FALSE : TRUE));
        freeAsmop(IC_RESULT(ic),NULL,ic,TRUE);
}

/*-----------------------------------------------------------------*/
/* addSign - propogate sign bit to higher bytes                    */
/*-----------------------------------------------------------------*/
void addSign(operand *result, int offset, int sign)
{
        int size = (pic14_getDataSize(result) - offset);
        DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
        FENTRY;
        
        if(size > 0){
                if(sign && offset) {
                        
                        if(size == 1) {
                                emitpcode(POC_CLRF,popGet(AOP(result),offset));
                                emitpcode(POC_BTFSC,newpCodeOpBit(aopGet(AOP(result),offset-1,FALSE,FALSE),7,0));
                                emitpcode(POC_DECF, popGet(AOP(result),offset));
                        } else {
                                
                                emitpcode(POC_MOVLW, popGetLit(0));
                                emitpcode(POC_BTFSC, newpCodeOpBit(aopGet(AOP(result),offset-1,FALSE,FALSE),7,0));
                                emitpcode(POC_MOVLW, popGetLit(0xff));
                                while(size--)
                                        emitpcode(POC_MOVWF, popGet(AOP(result),offset+size));
                        }
                } else
                        while(size--)
                                emitpcode(POC_CLRF,popGet(AOP(result),offset++));
        }
}

/*-----------------------------------------------------------------*/
/* genMinus - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
void genMinus (iCode *ic)
{
        int size, offset = 0, same=0;
        unsigned long lit = 0L;
        int isLit;
        symbol *lbl_comm, *lbl_next;
        asmop *left, *right, *result;

        FENTRY;
        DEBUGpic14_emitcode ("; ***","%s  %d",__FUNCTION__,__LINE__);
        aopOp (IC_LEFT(ic),ic,FALSE);
        aopOp (IC_RIGHT(ic),ic,FALSE);
        aopOp (IC_RESULT(ic),ic,TRUE);
        
        if (AOP_TYPE(IC_RESULT(ic)) == AOP_CRY  &&
                AOP_TYPE(IC_RIGHT(ic)) == AOP_LIT) {
                operand *t = IC_RIGHT(ic);
                IC_RIGHT(ic) = IC_LEFT(ic);
                IC_LEFT(ic) = t;
        }
        
        DEBUGpic14_emitcode ("; ","result %s, left %s, right %s",
                AopType(AOP_TYPE(IC_RESULT(ic))),
                AopType(AOP_TYPE(IC_LEFT(ic))),
                AopType(AOP_TYPE(IC_RIGHT(ic))));
        
        left = AOP(IC_LEFT(ic));
        right = AOP(IC_RIGHT(ic));
        result = AOP(IC_RESULT(ic));
        
        size = pic14_getDataSize(IC_RESULT(ic));
        same = pic14_sameRegs(right, result);

        if((AOP_TYPE(IC_LEFT(ic)) != AOP_LIT)
            && (pic14_getDataSize(IC_LEFT(ic)) < size))
        {
                fprintf(stderr, "%s:%d(%s):WARNING: left operand too short for result\n",
                        ic->filename, ic->lineno, __FUNCTION__);
        } // if
        if((AOP_TYPE(IC_RIGHT(ic)) != AOP_LIT)
            && (pic14_getDataSize(IC_RIGHT(ic)) < size))
        {
                fprintf(stderr, "%s:%d(%s):WARNING: right operand too short for result\n",
                        ic->filename, ic->lineno, __FUNCTION__ );
        } // if

        if(AOP_TYPE(IC_RIGHT(ic)) == AOP_LIT) {
                /* Add a literal to something else */
                
                lit = ulFromVal(right->aopu.aop_lit);
                lit = -(long)lit;
                
                genAddLit(ic, lit);
                
        } else if(AOP_TYPE(IC_RIGHT(ic)) == AOP_CRY) {
                // bit subtraction
                
                pic14_emitcode(";bitsub","right is bit: %s",aopGet(right,0,FALSE,FALSE));
                pic14_emitcode(";bitsub","left is bit: %s",aopGet(left,0,FALSE,FALSE));
                pic14_emitcode(";bitsub","result is bit: %s",aopGet(result,0,FALSE,FALSE));
                
                /* here we are subtracting a bit from a char or int */
                if(size == 1) {
                        if(pic14_sameRegs(left, result)) {
                                
                                emitpcode(POC_BTFSC, popGet(right, 0));
                                emitpcode(POC_DECF , popGet(result, 0));
                                
                        } else {
                                
                                if( (AOP_TYPE(IC_LEFT(ic)) == AOP_IMMD) ||
                                        (AOP_TYPE(IC_LEFT(ic)) == AOP_LIT) ) {
                                        /*
                                         * result = literal - bit
                                         *
                                         * XXX: probably fails for AOP_IMMDs!
                                         */
                                        
                                        lit = ulFromVal (left->aopu.aop_lit);
                                        
                                        if(AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) {
                                                if(pic14_sameRegs(right, result)) {
                                                        if(lit & 1) {
                                                                emitpcode(POC_MOVLW, popGet(right, 0));
                                                                emitpcode(POC_XORWF, popGet(result, 0));
                                                        }
                                                } else {
                                                        emitpcode(POC_BCF, popGet(result, 0));
                                                        if(lit & 1) 
                                                                emitpcode(POC_BTFSS, popGet(right, 0));
                                                        else
                                                                emitpcode(POC_BTFSC, popGet(right, 0));
                                                        emitpcode(POC_BSF, popGet(result, 0));
                                                }
                                                goto release;
                                        } else {
                                                emitpcode(POC_MOVLW, popGetLit(lit & 0xff));
                                                emitpcode(POC_BTFSC, popGet(right, 0));
                                                emitpcode(POC_MOVLW, popGetLit((lit-1) & 0xff));
                                                emitpcode(POC_MOVWF, popGet(result, 0));
                                        }
                                        
                                } else {
                                        // result = register - bit
                                        // XXX: Fails for lit-like left operands
                                        emitpcode(POC_MOVFW, popGet(left, 0));
                                        emitpcode(POC_BTFSC, popGet(right, 0));
                                        emitpcode(POC_DECFW, popGet(left, 0));
                                        emitpcode(POC_MOVWF, popGet(result, 0));
                                }
                        }
                } else {
                    fprintf(stderr, "WARNING: subtracting bit from multi-byte operands is incomplete.\n");
                    //exit(EXIT_FAILURE);
                } // if
        } else {
                /*
                 * RIGHT is not a literal and not a bit operand,
                 * LEFT is unknown (register, literal, bit, ...)
                 */
                lit = 0;
                isLit = 0;

                if(AOP_TYPE(IC_LEFT(ic)) == AOP_LIT) {
                        lit = ulFromVal (left->aopu.aop_lit);
                        isLit = 1;
                        DEBUGpic14_emitcode ("; left is lit","line %d result %s, left %s, right %s",__LINE__,
                                AopType(AOP_TYPE(IC_RESULT(ic))),
                                AopType(AOP_TYPE(IC_LEFT(ic))),
                                AopType(AOP_TYPE(IC_RIGHT(ic))));
                } // if


                /*
                 * First byte, no carry-in.
                 * Any optimizations that are longer than 2 instructions are
                 * useless.
                 */
                if(same && isLit && ((lit & 0xff) == 0xff)) {
                        // right === res = 0xFF - right = ~right
                        emitpcode(POC_COMF, popGet(right, 0));
                        if(size > 1) {
                                // setup CARRY/#BORROW
                                emitSETC;
                        } // if
                } else if((size == 1) && isLit && ((lit & 0xff) == 0xff)) {
                        // res = 0xFF - right = ~right
                        emitpcode(POC_COMFW, popGet(right, 0));
                        emitpcode(POC_MOVWF, popGet(result, 0));
                        // CARRY/#BORROW is not setup correctly
                } else if((size == 1) && same && isLit && ((lit & 0xff) == 0)) {
                        // right === res = 0 - right = ~right + 1
                        emitpcode(POC_COMF, popGet(right, 0));
                        emitpcode(POC_INCF, popGet(right, 0));
                        // CARRY/#BORROW is not setup correctly
                } else {
                        // general case, should always work
                        mov2w(right, 0);
                        if (pic14_sameRegs(left, result)) {
                                // result === left = left - right (in place)
                                emitpcode(POC_SUBWF, popGet(result, 0));
                        } else {
                                // works always: result = left - right
                                emitpcode(op_isLitLike(IC_LEFT(ic))
                                        ? POC_SUBLW : POC_SUBFW,
                                        popGetAddr(left, 0, 0));
                                emitpcode(POC_MOVWF, popGet(result, 0));
                        } // if
                } // if
                
                /*
                 * Now for the remaining bytes with carry-in (and carry-out).
                 */
                offset = 0;
                while(--size) {
                        lit >>= 8;
                        offset++;

                        /*
                         * The following code generation templates are ordered
                         * according to increasing length; the first template
                         * that matches will thus be the shortest and produce
                         * the best code possible with thees templates.
                         */

                        if(pic14_sameRegs(left, right)) {
                            /*
                             * This case should not occur; however, the
                             * template works if LEFT, RIGHT, and RESULT are
                             * register operands and LEFT and RIGHT are the
                             * same register(s) and at least as long as the
                             * result.
                             *
                             *   CLRF   result
                             *
                             * 1 cycle
                             */
                            emitpcode(POC_CLRF, popGet(result, offset));
                        } else if(pic14_sameRegs(left, result)) {
                            /*
                             * This template works if LEFT, RIGHT, and
                             * RESULT are register operands and LEFT and
                             * RESULT are the same register(s).
                             *
                             *   MOVF   right, W    ; W := right
                             *   BTFSS  STATUS, C
                             *   INCFSZ right, W    ; W := right + BORROW
                             *   SUBWF  result, F   ; res === left := left - (right + BORROW)
                             *
                             * The SUBWF *must* be skipped if we have a
                             * BORROW bit and right == 0xFF in order to
                             * keep the BORROW bit intact!
                             *
                             * 4 cycles
                             */
                            mov2w(right, offset);
                            emitSKPC;
                            emitpcode(POC_INCFSZW, popGet(right, offset));
                            emitpcode(POC_SUBWF, popGet(result, offset));
                        } else if((size == 1) && isLit && ((lit & 0xff) == 0xff)) {
                            /*
                             * This template works for the last byte (MSB) of
                             * the subtraction and ignores correct propagation
                             * of the outgoing BORROW bit. RIGHT and RESULT
                             * must be register operands, LEFT must be the
                             * literal 0xFF.
                             *
                             * (The case LEFT === RESULT is optimized above.)
                             *
                             * 0xFF - right - BORROW = ~right - BORROW
                             *
                             *   COMF   right, W    ; W := 0xff - right
                             *   BTFSS  STATUS, C
                             *   ADDLW  0xFF        ; W := 0xff - right - BORROW
                             *   MOVWF  result
                             *
                             * 4 cycles
                             */
                            emitpcode(POC_COMFW, popGet(right, offset));
                            emitSKPC;
                            emitpcode(POC_ADDLW, popGetLit(0xff));
                            emitpcode(POC_MOVWF, popGet(result, offset));
                        } else if(size == 1) {
                            /*
                             * This template works for the last byte (MSB) of
                             * the subtraction and ignores correct propagation
                             * of the outgoing BORROW bit. RIGHT and RESULT
                             * must be register operands, LEFT can be a
                             * register or a literal operand.
                             *
                             * (The case LEFT === RESULT is optimized above.)
                             *
                             *   MOVF   right, W    ; W := right
                             *   BTFSS  STATUS, C
                             *   INCF   right, W    ; W := right + BORROW
                             *   SUBxW  left, W     ; W := left - right - BORROW
                             *   MOVWF  result
                             *
                             * 5 cycles
                             */
                            mov2w(right, offset);
                            emitSKPC;
                            emitpcode(POC_INCFW, popGet(right, offset));
                            emitpcode(op_isLitLike(IC_LEFT(ic))
                                    ? POC_SUBLW : POC_SUBFW,
                                    popGetAddr(left, offset, 0));
                            emitpcode(POC_MOVWF, popGet(result, offset));
                        } else if(IS_ITEMP(IC_RESULT(ic))
                                && !pic14_sameRegs(right, result)) {
                            /*
                             * This code template works if RIGHT and RESULT
                             * are different register operands and RESULT
                             * is not volatile/an SFR (written twice).
                             *
                             * #########################################
                             * Since we cannot determine that for sure,
                             * we approximate via IS_ITEMP() for now.
                             * #########################################
                             *
                             *   MOVxW  left, W     ; copy left to result
                             *   MOVWF  result
                             *   MOVF   right, W    ; W := right
                             *   BTFSS  STATUS, C
                             *   INCFSZ right, W    ; W := right + BORROW
                             *   SUBWF  result, F   ; res === left := left - (right + BORROW)
                             *
                             * 6 cycles, but fails for SFR RESULT operands
                             */
                            mov2w(left, offset);
                            emitpcode(POC_MOVWF, popGet(result, offset));
                            mov2w(right, offset);
                            emitSKPC;
                            emitpcode(POC_INCFSZW, popGet(right, offset));
                            emitpcode(POC_SUBWF, popGet(result, offset));
                        } else if(!optimize.codeSize && isLit && ((lit & 0xff) != 0)) {
                            /*
                             * This template works if RIGHT and RESULT are
                             * register operands and LEFT is a literal
                             * operand != 0.
                             *
                             *   MOVxW  right, W
                             *   BTFSC  STATUS, C
                             *   GOTO   next
                             *   SUBLW  left-1
                             *   GOTO   common
                             * next:
                             *   SUBLW  left
                             * common:
                             *   MOVWF  result
                             *
                             * 6 cycles, 7 iff BORROW
                             * (9 instructions)
                             */
                            lbl_comm = newiTempLabel(NULL);
                            lbl_next = newiTempLabel(NULL);

                            mov2w(right, offset);
                            emitSKPNC;
                            emitpcode(POC_GOTO, popGetLabel(lbl_next->key));
                            emitpcode(POC_SUBLW, popGetLit((lit - 1) & 0xff));
                            emitpcode(POC_GOTO, popGetLabel(lbl_comm->key));
                            emitpLabel(lbl_next->key);
                            emitpcode(POC_SUBLW, popGetLit(lit & 0xff));
                            emitpLabel(lbl_comm->key);
                            emitpcode(POC_MOVWF, popGet(result, offset));
                        } else {
                            /*
                             * This code template works if RIGHT and RESULT
                             * are register operands.
                             *
                             *   MOVF   right, W    ; W := right
                             *   BTFSS  STATUS, C
                             *   INCFSZ right, W    ; W := right + BORROW
                             *   GOTO   common
                             *   MOVxW  left        ; if we subtract 0x100 = 0xFF + 1, ...
                             *   GOTO   next        ; res := left, but keep BORROW intact
                             * common:
                             *   SUBxW  left, W     ; W := left - (right + BORROW)
                             * next:
                             *   MOVW   result      ; res := left - (right + BORROW)
                             *
                             * 7 cycles, 8 iff BORROW and (right == 0xFF)
                             * (8 instructions)
                             *
                             *
                             *
                             * Alternative approach using -x = ~x + 1 ==> ~x = -x - 1 = -(x + 1)
                             *
                             *   COMFW  right, W    ; W := -right - (assumed BORROW)
                             *   BTFSC  STATUS, C   ; SKIP if we have a BORROW
                             *   ADDLW  1           ; W := -right (no BORROW)
                             *   BTFSC  STATUS, C   ; (***)
                             *   MOVLW  left        ; (***)
                             *   BTFSS  STATUS, C   ; (***)
                             *   ADDFW  left, W     ; W := left - right - BORROW (if any)
                             *   MOVWF  result      ; result := left - right - BORROW (if any)
                             *
                             * 8 cycles
                             *
                             * Case A: C=0 (BORROW)
                             * r=0x00, W=0xFF, W=left+0xFF, C iff left>0x00
                             * r=0x01, W=0xFE, W=left+0xFE, C iff left>0x01
                             * r=0xFE, W=0x01, W=left+0x01, C iff left>0xFE
                             * r=0xFF, W=0x00, W=left+0x00, C iff left>0xFF
                             *
                             * Case B: C=1 (no BORROW)
                             * r=0x00, W=0xFF, W=0x00/C=1, W=left+0x00, C iff left>=0x100 (***)
                             * r=0x01, W=0xFE, W=0xFF/C=0, W=left+0xFF, C iff left>=0x01
                             * r=0xFE, W=0x01, W=0x02/C=0, W=left+0x02, C iff left>=0xFE
                             * r=0xFF, W=0x00, W=0x01/C=0, W=left+0x01, C iff left>=0xFF
                             */
                            lbl_comm = newiTempLabel(NULL);
                            lbl_next = newiTempLabel(NULL);

                            mov2w(right, offset);
                            emitSKPC;
                            emitpcode(POC_INCFSZW, popGet(right, offset));
                            emitpcode(POC_GOTO, popGetLabel(lbl_comm->key));
                            mov2w(left, offset);
                            emitpcode(POC_GOTO, popGetLabel(lbl_next->key));
                            emitpLabel(lbl_comm->key);
                            emitpcode(op_isLitLike(IC_LEFT(ic))
                                    ? POC_SUBLW : POC_SUBFW,
                                    popGetAddr(left, offset, 0));
                            emitpLabel(lbl_next->key);
                            emitpcode(POC_MOVWF, popGet(result, offset));
                        } // if
                } // while
        } // if

        if(AOP_TYPE(IC_RESULT(ic)) == AOP_CRY) {
            fprintf(stderr, "WARNING: AOP_CRY (bit-) results are probably broken. Please report this with source code as a bug.\n");
            mov2w(result, 0); // load Z flag
            emitpcode(POC_BCF, popGet(result, 0));
            emitSKPZ;
            emitpcode(POC_BSF, popGet(result, 0));
        } // if
        
        //    adjustArithmeticResult(ic);
        
release:
        freeAsmop(IC_LEFT(ic),NULL,ic,(RESULTONSTACK(ic) ? FALSE : TRUE));
        freeAsmop(IC_RIGHT(ic),NULL,ic,(RESULTONSTACK(ic) ? FALSE : TRUE));
        freeAsmop(IC_RESULT(ic),NULL,ic,TRUE);
}

