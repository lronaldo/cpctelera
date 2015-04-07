/*
 * Simulator of microcontrollers (inst.cc)
 *
 * st7 code base from Vaclav Peroutka vaclavpe@users.sourceforge.net
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include "ddconfig.h"
#include "stdio.h"
#include <stdlib.h>

// local
#include "st7cl.h"
#include "regsst7.h"
#include "st7mac.h"

unsigned int
cl_st7::fetchea(t_mem code, unsigned char prefix)
{
   unsigned int resaddr;
   unsigned int ftc; //because of get2() MACRO!!!!!!
//   unsigned int ftc, gt;
//       ftc = fetch();
//       gt = get1(ftc);

//  printf("******************** fetchea() start PC= 0x%04x, prefix = 0x%02x, code = 0x%02x \n", PC, prefix, code);

  switch ((code >> 4) & 0x0f) {
	
    case 0xb:
	  if ( 0 == prefix) {              // short direct
		 resaddr = fetch();
     } else if ( 0x92 == prefix) {    // short indirect - pointer
       resaddr = get1(fetch());
	  } else {
	    resaddr = ( resHALT);
	  }
     break;
     
    case 0xc:
	  if ( 0 == prefix) {              // long direct
		resaddr = fetch2();
	  } else if ( 0x92 == prefix) {    // long indirect - pointer is under 0x100
       ftc = fetch();
       resaddr = get2(ftc);
	  } else {
	    resaddr = ( resHALT);
	  }
     break;

    case 0xd:
	  if ( 0 == prefix) {              // long offset with X reg
        resaddr = (fetch2()+regs.X);
	  } else if ( 0x90 == prefix) {    // long offset with Y reg
        resaddr = (fetch2()+regs.Y);
	  } else if ( 0x91 == prefix) {    // pointer to long offset with Y
      ftc = fetch();
	    resaddr = (get2(ftc)+regs.Y);
	  } else if ( 0x92 == prefix) {    // pointer to long offset with X
       ftc = fetch();
	    resaddr = (get2(ftc)+regs.X);
	  } else {
	    resaddr =( resHALT);
	  }
     break;

    case 0xe:
	  if ( 0 == prefix) {              // short offset with X
		resaddr = (fetch()+regs.X);
	  } else if ( 0x90 == prefix) {    // short offset with Y
	    resaddr = (fetch()+regs.Y);
	  } else if ( 0x91 == prefix) {    // pointer to short offset with Y
	    resaddr = (get1(fetch())+regs.Y);
	  } else if ( 0x92 == prefix) {    // pointer to short offset with X
	    resaddr = (get1(fetch())+regs.X);
	  } else {
	    resaddr =( resHALT);
	  }
     break;

    case 0xf:
	  if ( 0 == prefix) {
		resaddr = regs.X;  // X index
	  } else if ( 0x90 == prefix) {
	    resaddr = regs.Y;  // Y index
	  } else {
	    resaddr =( resHALT);
	  }
     break;

    default:
      resaddr =(resHALT);
      break;
  }
//  printf("******************** fetchea() end - resaddr=0x%04x, PC=0x%04x\n", resaddr, PC);
  return resaddr;
}


int
cl_st7::get_dest(t_mem code, unsigned char prefix)
{
   int resaddr;
//  printf("******************** get_dest() start PC= 0x%04x, prefix = 0x%02x, code = 0x%02x \n", PC, prefix, code);

  switch ((code >> 4) & 0x0f) {
	
    case 0x3:
	  if ( 0 == prefix) {              // short direct
		 resaddr = fetch();
     } else if ( 0x92 == prefix) {    // short indirect - pointer
       resaddr = get1(fetch());
	  } else {
	    resaddr = ( resHALT);
	  }
     break;
     
    case 0x6:
	  if ( 0 == prefix) {              // short offset with X
		resaddr = (fetch()+regs.X);
	  } else if ( 0x90 == prefix) {    // short offset with Y
	    resaddr = (fetch()+regs.Y);
	  } else if ( 0x91 == prefix) {    // pointer to short offset with Y
	    resaddr = (get1(fetch())+regs.Y);
	  } else if ( 0x92 == prefix) {    // pointer to short offset with X
	    resaddr = (get1(fetch())+regs.X);
	  } else {
	    resaddr =( resHALT);
	  }
     break;

    case 0x7:
	  if ( 0 == prefix) {              // X index
		resaddr = regs.X;
	  } else if ( 0x90 == prefix) {    // Y index
	    resaddr = regs.Y;
	  } else {
	    resaddr =( resHALT);
	  }
     break;

    default:
      resaddr =(resHALT);
      break;
  }
//  printf("******************** get_dest() end - resaddr=0x%04x, PC=0x%04x\n", resaddr, PC);
  return resaddr;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************
 */

int
cl_st7::inst_adc(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;
  int carryin = (regs.CC & BIT_C)!=0;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 + operand2 + carryin;

//  printf("******************** adc() op1 = 0x%02x, op2=0x%02x , res=0x%02x \n", operand1, operand2, result);

  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_N, 0x80 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_st7::inst_add(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 + operand2;

//  printf("******************** adc() op1 = 0x%02x, op2=0x%02x , res=0x%02x \n", operand1, operand2, result);

  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_N, 0x80 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_st7::inst_and(t_mem code, unsigned char prefix)
{
  int operand2;

  operand2 = OPERAND(code, prefix);
  regs.A = regs.A & operand2;

  FLAG_ASSIGN (BIT_Z, (regs.A & 0xff) == 0);
  FLAG_ASSIGN (BIT_N, 0x80 & regs.A);

  return(resGO);
}

int
cl_st7::inst_bcp(t_mem code, unsigned char prefix)
{
  int operand2, opresult;

  operand2 = OPERAND(code, prefix);
  opresult = regs.A & operand2;

  FLAG_ASSIGN (BIT_Z, (opresult & 0xff) == 0);
  FLAG_ASSIGN (BIT_N, 0x80 & opresult);

  return(resGO);
}

int
cl_st7::inst_bresbset(t_mem code, unsigned char prefix)
{
  int ea = fetch();
  unsigned char dbyte = get1( ea);
  
  if (code & 0x01) { // bres  
	char pos = (code - 0x11) >> 1;
	dbyte = dbyte & (~(1<<pos));
	
  } else { // bset
 	char pos = (code - 0x10) >> 1;
	dbyte = dbyte | (1<<pos);
	
  }
  
  store1(ea, dbyte);
  return(resGO);
}

int
cl_st7::inst_btjfbtjt(t_mem code, unsigned char prefix)
{
  int ea = fetch();
  unsigned char dbyte = get1( ea);
  char reljump = fetch();
  
  if (code & 0x01) { // btjf  
	char pos = (code - 0x01) >> 1;
	if(!( dbyte & (1<<pos))) {
		PC += reljump;
	}
	
  } else { // btjt
 	char pos = (code - 0x00) >> 1;
	if ( dbyte & (1<<pos)) {
		PC += reljump;
	}
	
  }
  
  return(resGO);
}

int
cl_st7::inst_call(t_mem code, unsigned char prefix)
{
  int newPC = fetchea(code, prefix);
  push2(PC);
  PC = newPC;

  return(resGO);
}

int
cl_st7::inst_callr(t_mem code, unsigned char prefix)
{
   char newPC;

   if (prefix == 0x0) {
      newPC = fetch1();
   } else if (prefix == 0x92) {
      newPC = get1(fetch1());
   } else {
      return(resHALT);
   }
   
   push2(PC);
   PC += newPC;

   return(resGO);
}

int
cl_st7::inst_clr(t_mem code, unsigned char prefix)
{
  int operand;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
   }
   
   operand = 0;
  
   FLAG_SET (BIT_Z);
   FLAG_CLEAR (BIT_N);

   if ((code&0xf0)==0x40) {
      regs.A = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = operand;
   } else {
      //printf("******************** inst_neg() end - addr=0x%04x, val=0x%02x\n", opaddr, operand);
      store1(opaddr, operand);
   }
  
   return(resGO);
}

int
cl_st7::inst_cp(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2;

  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_N, 0x80 & result);

  return(resGO);
}

int
cl_st7::inst_cpxy(t_mem code, unsigned char prefix)
{
   int operand, result;

   operand = OPERAND(code, prefix);

   if ((prefix == 0x90) || (prefix == 0x91)) {
     result = regs.Y - operand;
   } else {
     result = regs.X - operand;
   }
   
   FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
   FLAG_ASSIGN (BIT_C, 0x100 & result);
   FLAG_ASSIGN (BIT_N, 0x80 & result);

   return(resGO);
}

int
cl_st7::inst_cpl(t_mem code, unsigned char prefix)
{
  int operand;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   operand = operand ^ 0xff;
  
   FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
   FLAG_ASSIGN (BIT_N, 0x80 & operand);
   FLAG_SET (BIT_C);

   if ((code&0xf0)==0x40) {
      regs.A = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = operand;
   } else {
      store1(opaddr, operand);
   }
  
   return(resGO);
}

int
cl_st7::inst_dec(t_mem code, unsigned char prefix)
{
  int operand;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   operand -= 1;
  
   FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
   FLAG_ASSIGN (BIT_N, 0x80 & operand);

   if ((code&0xf0)==0x40) {
      regs.A = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = operand;
   } else {
      store1(opaddr, operand);
   }
  
   return(resGO);
}

int
cl_st7::inst_inc(t_mem code, unsigned char prefix)
{
  int operand;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   operand += 1;
  
   FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
   FLAG_ASSIGN (BIT_N, 0x80 & operand);

   if ((code&0xf0)==0x40) {
      regs.A = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = operand;
   } else {
      store1(opaddr, operand);
   }
  
   return(resGO);
}

int
cl_st7::inst_jr(t_mem code, unsigned char prefix)
{
  bool taken;
  signed char ofs;
  
    switch ((code>>1) & 7) {
      case 0: // JRA / JRF
        taken = 1;
        break;
      case 1: // JRUGT / JRULE
        taken = !(regs.CC & (BIT_C | BIT_Z));
        break;
      case 2: // JRUGE / JRC
        taken = !(regs.CC & BIT_C);
        break;
      case 3: // JRNE / JREQ
        taken = !(regs.CC & BIT_Z);
        break;
      case 4: // JRNH / JRH
        taken = !(regs.CC & BIT_H);
        break;
      case 5: // JRPL / JRMI
        taken = !(regs.CC & BIT_N);
        break;
      case 6: // JRNM / JRM
        taken = !(regs.CC & BIT_I);
        break;
      case 7: // JRIL - interrupt low - no means to test this ???
        taken = 1; 
        
      default:
        return(resHALT);
    } 
  
  if (code & 1)
    taken = ! taken;
  
   if (prefix == 0x00) { // direct jump relative
      ofs = fetch();
   } else if (prefix == 0x92) { // pointer jump relative
      ofs = get1(fetch());
   } else {
      return (resHALT);
   }

  if (taken)
    PC += ofs;

  return(resGO);
}

int
cl_st7::inst_ldxy(t_mem code, unsigned char prefix)
{
  int operand;

  operand = OPERAND(code, prefix);

//  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
//  FLAG_ASSIGN (BIT_C, 0x100 & result);
//  FLAG_ASSIGN (BIT_N, 0x80 & result);
//  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

   if ((prefix == 0x90) || (prefix == 0x91)) {
     regs.Y = operand;
   } else {
     regs.X = operand;
   }
   
  return(resGO);
}

int
cl_st7::inst_lda(t_mem code, unsigned char prefix)
{
  int operand;

  operand = OPERAND(code, prefix);

//  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
//  FLAG_ASSIGN (BIT_C, 0x100 & result);
//  FLAG_ASSIGN (BIT_N, 0x80 & result);
//  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = operand;
   
  return(resGO);
}

int
cl_st7::inst_lddst(t_mem code, unsigned char prefix)
{
  unsigned int opaddr;

   opaddr = fetchea(code,prefix);

   FLAG_ASSIGN (BIT_Z, (regs.A & 0xff) == 0);
   FLAG_ASSIGN (BIT_N, 0x80 & regs.A);

   store1(opaddr, regs.A);
  
   return(resGO);
}

int
cl_st7::inst_ldxydst(t_mem code, unsigned char prefix)
{
  unsigned int opaddr;

   opaddr = fetchea(code,prefix);

   if ((prefix == 0x90) || (prefix == 0x91)) {
      FLAG_ASSIGN (BIT_Z, (regs.Y & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & regs.Y);

      store1(opaddr, regs.Y);
   } else {
      FLAG_ASSIGN (BIT_Z, (regs.X & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & regs.X);

      store1(opaddr, regs.X);
   }
   
   return(resGO);
}



int
cl_st7::inst_neg(t_mem code, unsigned char prefix)
{
  int operand;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   operand ^= 0xff;
   operand++;
  
   FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
   FLAG_ASSIGN (BIT_C, operand);
   if (operand == 0x80) {
	   FLAG_SET (BIT_N);
   } else {
      FLAG_CLEAR (BIT_N);
   }

   if ((code&0xf0)==0x40) {
      regs.A = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = operand;
   } else {
      //printf("******************** inst_neg() end - addr=0x%04x, val=0x%02x\n", opaddr, operand);
      store1(opaddr, operand);
   }
  
   return(resGO);
}

int
cl_st7::inst_or(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 | operand2;

  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
  //FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_N, 0x80 & result);
  //FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_st7::inst_rlc(t_mem code, unsigned char prefix)
{
  int operand;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   operand <<= 0x1;
   
   if (regs.CC & BIT_C) {
      operand++;
   }
  
   FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
   FLAG_ASSIGN (BIT_C, (operand & 0x100));
   if (operand == 0x80) {
	   FLAG_SET (BIT_N);
   } else {
      FLAG_CLEAR (BIT_N);
   }

   if ((code&0xf0)==0x40) {
      regs.A = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = operand;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = operand;
   } else {
      store1(opaddr, operand);
   }
  
   return(resGO);
}

int
cl_st7::inst_rrc(t_mem code, unsigned char prefix)
{
  int operand, opres;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   opres = operand >> 1;
   
   if (regs.CC & BIT_C) {
      opres |= 0x80;
   }
  
   FLAG_ASSIGN (BIT_Z, (opres & 0xff) == 0);
   FLAG_ASSIGN (BIT_C, (operand & 0x1));
   if (opres == 0x80) {
	   FLAG_SET (BIT_N);
   } else {
      FLAG_CLEAR (BIT_N);
   }

   if ((code&0xf0)==0x40) {
      regs.A = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = opres;
   } else {
      store1(opaddr, opres);
   }
  
   return(resGO);
}

int
cl_st7::inst_sbc(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2;

   if (regs.CC & BIT_C) {
      result--;
   }

  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_N, 0x80 & result);
  //FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_st7::inst_sll(t_mem code, unsigned char prefix)
{
  int operand, opres;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   opres = operand << 1;
   
   FLAG_ASSIGN (BIT_Z, (opres & 0xff) == 0);
   FLAG_ASSIGN (BIT_C, (opres & 0x100));
   if (opres == 0x80) {
	   FLAG_SET (BIT_N);
   } else {
      FLAG_CLEAR (BIT_N);
   }

   if ((code&0xf0)==0x40) {
      regs.A = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = opres;
   } else {
      store1(opaddr, opres);
   }
  
   return(resGO);
}

int
cl_st7::inst_sra(t_mem code, unsigned char prefix)
{
  int operand, opres;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   opres = operand >> 1;
   
   if (opres > 0x3f) { // copy bit6 to bit7
      opres |= 0x80;
   }
  
   FLAG_ASSIGN (BIT_Z, (opres & 0xff) == 0);
   FLAG_ASSIGN (BIT_C, (operand & 0x1));
   if (opres == 0x80) {
	   FLAG_SET (BIT_N);
   } else {
      FLAG_CLEAR (BIT_N);
   }

   if ((code&0xf0)==0x40) {
      regs.A = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = opres;
   } else {
      store1(opaddr, opres);
   }
  
   return(resGO);
}

int
cl_st7::inst_srl(t_mem code, unsigned char prefix)
{
  int operand, opres;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   opres = operand >> 1;
   
   FLAG_ASSIGN (BIT_Z, (opres & 0xff) == 0);
   FLAG_ASSIGN (BIT_C, (operand & 0x1));
   if (opres == 0x80) {
	   FLAG_SET (BIT_N);
   } else {
      FLAG_CLEAR (BIT_N);
   }

   if ((code&0xf0)==0x40) {
      regs.A = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = opres;
   } else {
      store1(opaddr, opres);
   }
  
   return(resGO);
}

int
cl_st7::inst_sub(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2;

  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_N, 0x80 & result);
  //FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_st7::inst_swap(t_mem code, unsigned char prefix)
{
  int operand, opres;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   opres = operand << 4;
   opres |= (operand >> 4);
   
   
   FLAG_ASSIGN (BIT_Z, (opres & 0xff) == 0);
   if (opres == 0x80) {
	   FLAG_SET (BIT_N);
   } else {
      FLAG_CLEAR (BIT_N);
   }

   if ((code&0xf0)==0x40) {
      regs.A = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = opres;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = opres;
   } else {
      store1(opaddr, opres);
   }
  
   return(resGO);
}

int
cl_st7::inst_tnz(t_mem code, unsigned char prefix)
{
  int operand;
  unsigned int opaddr;// = 0xffff;

   if ((code&0xf0)==0x40) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
   if (operand == 0x80) {
	   FLAG_SET (BIT_N);
   } else {
      FLAG_CLEAR (BIT_N);
   }

   return(resGO);
}

int
cl_st7::inst_xor(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 ^ operand2;

  FLAG_ASSIGN (BIT_Z, (result & 0xff) == 0);
  FLAG_ASSIGN (BIT_N, 0x80 & result);

  regs.A = result & 0xff;
  return(resGO);
}

/* End of st7.src/inst.cc */
