/*
 * Simulator of microcontrollers (inst.cc)
 *
 * stm8 code base from Vaclav Peroutka vaclavpe@users.sourceforge.net
 * and Valentin Dudouyt valentin.dudouyt@gmail.com
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
#include "stm8cl.h"
#include "regsstm8.h"
#include "stm8mac.h"



int
cl_stm8::fetchea(t_mem code, unsigned char prefix)
{
   int ftc;
   
   switch ((code >> 4) & 0x0f) {
   case 0x1: return (unsigned char)fetch()+regs.SP;  // SP indexed
	
   case 0xb: return fetch();           // direct short
	
   case 0xc:
      if ( 0 == prefix) {              // direct long
         return fetch2();  
      } else if ( 0x72 == prefix) {    // long pointer
         ftc = fetch2();
         return get2(ftc);
      } else if ( 0x92 == prefix) {    // short pointer
         ftc = fetch();
         return get2(ftc);
      } else {
         printf("************* bad prefix !!!!\n");
      }

   case 0xd:
      if ( 0 == prefix) {              // long offset with X reg
         return fetch2()+regs.X;
      } else if ( 0x72 == prefix) {    // long pointer to offset with X reg
         ftc = fetch2();
         return get2(ftc)+regs.X;
      } else if ( 0x90 == prefix) {    // long offset with Y reg
         return fetch2()+regs.Y;
      } else if ( 0x91 == prefix) {    // short pointer to offset with Y reg
         ftc = fetch();
         return get2(ftc)+regs.Y;
      } else if ( 0x92 == prefix) {    // short pointer to offset with X reg
         ftc = fetch();
         return get2(ftc)+regs.X;
      } else {
         return( resHALT);
      }

   case 0xe:
      if ( 0 == prefix) {               // short offset with X reg
         return fetch()+regs.X;  
      } else if ( 0x90 == prefix) {     // short offset with Y reg
         return fetch()+regs.Y;
      } else {
         return( resHALT);
      }

   case 0xf:
      if ( 0 == prefix) {               // X index
         return regs.X;
      } else if ( 0x90 == prefix) {     // Y index
         return regs.Y;
      } else {
         return( resHALT);
      }

   default:
      return(resHALT);
   }
}

int
cl_stm8::get_dest(t_mem code, unsigned char prefix)
{
   int resaddr, ftc;
//  printf("******************** get_dest() start PC= 0x%04x, prefix = 0x%02x, code = 0x%02x \n", PC, prefix, code);

  switch ((code >> 4) & 0x0f) {
    case 0x0:
	  if ( 0 == prefix) {              // short offset with SP
		 resaddr = fetch()+regs.SP;
	  } else {
	    resaddr = ( resHALT);
	  }
     break;
	
    case 0x3:
	  if ( 0 == prefix) {              // short direct
		 resaddr = fetch();
     } else if ( 0x72 == prefix) {    // long indirect - pointer
         ftc = fetch2();
       resaddr = get2(ftc);
     } else if ( 0x92 == prefix) {    // short indirect - pointer
         ftc = fetch2();
       resaddr = get2(ftc);
	  } else {
	    resaddr = ( resHALT);
	  }
     break;
     
    case 0x4:
	  if ( 0x72 == prefix) {           // long offset with X
         ftc = fetch2();
         resaddr = get2(ftc);
     } else if ( 0x90 == prefix) {    // long offset with Y
         ftc = fetch2();
         resaddr = get2(ftc);
	  } else {
         resaddr = ( resHALT);
	  }
     break;
     
    case 0x5:
	  if ( 0x72 == prefix) {           // long direct
         resaddr = fetch2();
	  } else {
         resaddr = ( resHALT);
	  }
     break;
     
    case 0x6:
	  if ( 0 == prefix) {              // short offset with X
		resaddr = (fetch()+regs.X);
	  } else if ( 0x72 == prefix) {    // long pointer to offset with X
         ftc = fetch2();
	    resaddr = (get2(ftc)+regs.X);
	  } else if ( 0x90 == prefix) {    // short offset with Y
	    resaddr = (fetch()+regs.Y);
	  } else if ( 0x91 == prefix) {    // short pointer to offset with Y
         ftc = fetch();
	    resaddr = (get1(ftc)+regs.Y);
	  } else if ( 0x92 == prefix) {    // short pointer to offset with X
         ftc = fetch();
	    resaddr = (get1(ftc)+regs.X);
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
cl_stm8::inst_adc(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;
  int carryin = (regs.CC & BIT_C)!=0;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 + operand2 + carryin;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_add(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 + operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::get2(unsigned int addr)
{
    return((ram->get((t_addr) (addr)) << 8) | ram->get((t_addr) (addr+1)));
}

int
cl_stm8::get3(unsigned int addr)
{
    return((ram->get((t_addr) (addr)) << 16) | (ram->get((t_addr) (addr+1)) << 8) |ram->get((t_addr) (addr+2)));
}

int
cl_stm8::inst_addw(t_mem code, unsigned char prefix)
{
  long int result, operand1, operand2, nibble_high, nibble_low;
  TYPE_UWORD *dest_ptr;

  nibble_high = (code >> 4) & 0x0f;
  nibble_low = code & 0x0f;
  dest_ptr = nibble_low == 0x09 || nibble_low == 0x02 ? &regs.Y : &regs.X;
  operand1 = *dest_ptr;

  switch(nibble_high)
  {
    case 0x1:
    case 0xa: operand2 = fetch2(); break; // Immediate
    case 0xb: operand2 = get2(fetch2()); break; // Long
    case 0xf: operand2 = get2(regs.SP + fetch()); break; // sp-indexed
    default: return(resHALT);
  }

  switch(nibble_low) {
    case 0x0:
    case 0xd: operand2 = -operand2;
    case 0xb:
    case 0xc: break;
    case 0x2: operand2 = -operand2;
    case 0x9: break;
    default: return(resHALT);
  }

  result = operand1 + operand2;

  FLAG_ASSIGN (BIT_V, 0x8000 & (operand1 ^ operand2 ^ result ^ (result >> 1)));
  FLAG_ASSIGN (BIT_H, 0x40 & (operand1 ^ operand2 ^ result));
  FLAG_ASSIGN (BIT_N, 0x8000 & result);
  FLAG_ASSIGN (BIT_Z, (result & 0xffff) == 0);
  FLAG_ASSIGN (BIT_C, 0x10000 & result);

  *dest_ptr = result & 0xffff;
  return(resGO);
}

int
cl_stm8::inst_and(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 & operand2;
  FLAG_NZ (result);

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_bccmbcpl(t_mem code, unsigned char prefix)
{
   int ea = fetch2();
   unsigned char dbyte = get1( ea);
  
   if (code & 0x01)  { // bccm  
      char pos = (code - 0x11) >> 1;
      dbyte = dbyte & (~(1<<pos));
      if (regs.CC & BIT_C) {
         dbyte |= (1<<pos);
      }	
   } else { // bcpl
      char pos = (code - 0x10) >> 1;
      dbyte = dbyte ^ (1<<pos);
	}
  
   store1(ea, dbyte);
   return(resGO);
}

int
cl_stm8::inst_bcp(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 & operand2;
  FLAG_NZ (result);

  return(resGO);
}

int
cl_stm8::inst_bresbset(t_mem code, unsigned char prefix)
{
  int ea = fetch2();
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
cl_stm8::inst_btjfbtjt(t_mem code, unsigned char prefix)
{
  int ea = fetch2();
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
cl_stm8::inst_call(t_mem code, unsigned char prefix)
{
  int newPC = fetchea(code, prefix);
  push2(PC);
  PC = newPC;

  return(resGO);
}

int
cl_stm8::inst_clr(t_mem code, unsigned char prefix)
{
  unsigned int opaddr = 0;

  FLAG_SET (BIT_Z);
  FLAG_CLEAR (BIT_N);
  
  switch(((code & 0xf0) | (prefix << 8)) >> 4) {
    /* clr */
    case 0x4: regs.A = 0; return(resGO);
    case 0x3: opaddr = fetch(); break;
    case 0x725: opaddr = fetch2(); break;
    case 0x7: opaddr = regs.X; break;
    case 0x6: opaddr = regs.X + fetch(); break;
    case 0x724: opaddr = regs.X + fetch2(); break;
    case 0x907: opaddr = regs.Y; break;
    case 0x906: opaddr = regs.Y + fetch(); break;
    case 0x904: opaddr = regs.Y + fetch2(); break;
    case 0x0: opaddr = regs.SP + fetch(); break;
    /* clrw */
    case 0x5: regs.X = 0; return(resGO);
    case 0x905: regs.Y = 0; return(resGO);
    default: return(resHALT);
  }

  store1(opaddr, 0);
  return(resGO);
}

int
cl_stm8::inst_cp(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  return(resGO);
}

int
cl_stm8::inst_cpw(t_mem code, unsigned char prefix)
{
  long int operand1, operand2, result;

  operand1 = prefix == 0x90 ? regs.Y : regs.X;
  operand2 = prefix == 0x90 ? regs.X : regs.Y;

  switch((code & 0xf0) >> 4)
  {
    case 0xa: operand2 = fetch2(); break; // Immediate
    case 0xb: operand2 = get2(fetch()); break; // Short
    case 0xc: operand2 = get2(fetch2()); break; // Long
    case 0x1: operand2 = get2(regs.SP + fetch()); break; // SP-indexed
    case 0xf: operand1 = get2(operand1); break; // cpw X|Y, (Y|X)
    case 0xe: operand1 = get2(operand1 + fetch()); break;
    case 0xd: operand1 = get2(operand1 + fetch2()); break;
    default: return(resHALT);
  }
   
  result = operand1 - operand2;

  FLAG_ASSIGN (BIT_V, 0x8000 & (operand1 ^ operand2 ^ result ^ (result >> 1)));
  FLAG_ASSIGN (BIT_Z, (result & 0xffff) == 0);
  FLAG_ASSIGN (BIT_C, 0x10000 & result);
  FLAG_ASSIGN (BIT_N, 0x8000 & result);

  return(resGO);
}

int
cl_stm8::inst_cpl(t_mem code, unsigned char prefix)
{
  long int operand;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   operand ^= 0xffff;
  
   FLAG_SET (BIT_C);

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = operand&0xff;
      FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & operand);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = operand;
      FLAG_ASSIGN (BIT_Z, (operand & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & operand);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = operand;
      FLAG_ASSIGN (BIT_Z, (operand & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & operand);
   } else {
      store1(opaddr, operand &0xff);
      FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & operand);
   }
  
   return(resGO);
}

int
cl_stm8::inst_dec(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   resval = operand - 1;
  
   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_V, 0x80 & (operand ^ 0xff ^ resval ^ (resval >>1)));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_V, 0x8000 & (operand ^ 0xffff ^ resval ^ (resval >>1)));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_V, 0x8000 & (operand ^ 0xffff ^ resval ^ (resval >>1)));
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_V, 0x80 & (operand ^ 0xff ^ resval ^ (resval >>1)));
   }
  
   return(resGO);
}

int
cl_stm8::inst_div(t_mem code, unsigned char prefix)
{
   unsigned int quot, remi;

   FLAG_CLEAR(BIT_N);
   FLAG_CLEAR(BIT_H);
   FLAG_CLEAR(BIT_V);

   if( code == 0x65) { // divw
      if (regs.Y == 0x00) {
         FLAG_SET(BIT_C);
      } else {
         FLAG_CLEAR(BIT_C);
         quot = regs.X / regs.Y;
         remi = regs.X % regs.Y;
         regs.X = quot;
         regs.Y = remi;
         FLAG_ASSIGN (BIT_Z, (quot & 0xffff) == 0);
      }
   
   } else { //div
      if (regs.A == 0x00) {
         FLAG_SET(BIT_C);
      } else {
         FLAG_CLEAR(BIT_C);
         if (prefix == 0x00) {
            quot = regs.X / regs.A;
            remi = regs.X % regs.A;
            regs.X = quot;
            regs.A = remi;
         } else if (prefix == 0x90) {
            quot = regs.Y / regs.A;
            remi = regs.Y % regs.A;
            regs.Y = quot;
            regs.A = remi;
         
         } else {
            return (resHALT);
         }
         FLAG_ASSIGN (BIT_Z, (quot & 0xffff) == 0);
      }
   }
   
  return(resGO);
}

int
cl_stm8::inst_inc(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   resval = operand + 1;
  
   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_V, 0x80 & (operand ^ 0x01 ^ resval ^ (resval >>1)));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_V, 0x8000 & (operand ^ 0x01 ^ resval ^ (resval >>1)));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_V, 0x8000 & (operand ^ 0x01 ^ resval ^ (resval >>1)));
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_V, 0x80 & (operand ^ 0x01 ^ resval ^ (resval >>1)));
   }
  
   return(resGO);
}

int
cl_stm8::inst_jp(t_mem code, unsigned char prefix)
{
  int newPC = fetchea(code, prefix);

  PC = newPC;

  return(resGO);
}

int
cl_stm8::inst_jr(t_mem code, unsigned char prefix)
{
  bool taken;
  signed char ofs;
  unsigned char maskedP;
  
  if (prefix ==0x00) {
    switch ((code>>1) & 7) {
      case 0: // JRA/JRT
        taken = 1;
        break;
      case 1: // JRUGT
        taken = !(regs.CC & (BIT_C | BIT_Z));
        break;
      case 2: // JRUGE
        taken = !(regs.CC & BIT_C);
        break;
      case 3: // JRNE
        taken = !(regs.CC & BIT_Z);
        break;
      case 4: // JRNV
        taken = !(regs.CC & BIT_V);
        break;
      case 5: // JRPL
        taken = !(regs.CC & BIT_N);
        break;
      case 6: // JRSGT - Z or (N xor V) = 0
        maskedP = regs.CC & (BIT_N | BIT_V | BIT_Z);
        taken = !maskedP || (maskedP == (BIT_N | BIT_V));
        break;
      case 7: // JRSGE - N xor V = 0
        maskedP = regs.CC & (BIT_N | BIT_V);
        taken = !maskedP || (maskedP == (BIT_N | BIT_V));
        break;
      default:
        return(resHALT);
    } 
  }
  else if (prefix==0x90) {
    switch ((code>>1) & 7) {
      case 4: // JRNH
         taken = !(regs.CC & BIT_H);
       break;
      case 6: // JRNM - no int mask
        taken = !(regs.CC & (BIT_I1|BIT_I0));
        break;
      case 7: // JRIL - interrupt low - no means to test this ???
        taken = 0; 
      default:
        return(resHALT);
    }
  }
  else
    return(resHALT);
  
  if (code & 1)
    taken = ! taken;
  
  ofs = fetch();
  if (taken)
    PC += ofs;

  return(resGO);
}

int
cl_stm8::inst_lda(t_mem code, unsigned char prefix)
{
  int operand;

  operand = OPERAND(code, prefix);

  FLAG_NZ (operand);

  regs.A = operand;
   
  return(resGO);
}

int
cl_stm8::operandw(t_mem code, unsigned char prefix)
{
       if(EA_IMM(code)) {
               return(fetch2());
       } else {
               int addr = fetchea(code,prefix);
               int result = get2(addr);
               return(result);
       }
}

int
cl_stm8::inst_ldxy(t_mem code, unsigned char prefix)
{
  unsigned int operand;
  TYPE_UWORD *dest_ptr;
  dest_ptr = (prefix & 0x90) ? &regs.Y : &regs.X;
  if(code == 0x16) dest_ptr = &regs.Y; 

  switch((code & 0xf0) >> 4) {
     case 0xa: operand = fetch2(); break; // Immediate
     case 0xb: operand = get2(fetch()); break; // Short
     case 0xc: operand = get2(fetch2()); break; // Long
     case 0xf: operand = get2(*dest_ptr); break;
     case 0xe: operand = get2(*dest_ptr + fetch()); break;
     case 0xd: operand = get2(*dest_ptr + fetch2()); break;
     case 0x1: operand = get2(regs.SP + fetch()); break;
     case 0x9: // Special cases
     	switch(code | (prefix << 8)) {
		case 0x9093: dest_ptr = &regs.Y; operand = regs.X; break;
		case 0x0093: dest_ptr = &regs.X; operand = regs.Y; break;
		case 0x0096: dest_ptr = &regs.X; operand = regs.SP; break;
		case 0x0094: dest_ptr = &regs.SP; operand = regs.X; break;
		case 0x9096: dest_ptr = &regs.Y; operand = regs.SP; break;
		case 0x9094: dest_ptr = &regs.SP; operand = regs.Y; break;
		default: return(resHALT);
	}
     default:  return(resHALT);
  }

  if ((code & 0xf0) != 0x90)
    {
      FLAG_ASSIGN (BIT_Z, (operand & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & operand);
    }

  *dest_ptr = operand;
   
  return(resGO);
}

int
cl_stm8::inst_lddst(t_mem code, unsigned char prefix)
{
  unsigned int opaddr;

   opaddr = fetchea(code,prefix);

   FLAG_ASSIGN (BIT_Z, (regs.A & 0xff) == 0);
   FLAG_ASSIGN (BIT_N, 0x80 & regs.A);

   store1(opaddr, regs.A);
  
   return(resGO);
}

int
cl_stm8::inst_ldxydst(t_mem code, unsigned char prefix)
{
  /* ldw dst, REG */
  unsigned int opaddr, operand;
  bool inv = (code == 0xdf || code == 0xef || code == 0xff);
  operand = ((prefix == 0x90) ^ inv) ? regs.Y : regs.X;
  if(code == 0x17) operand = regs.Y;

  switch((code & 0xf0) >> 4) {
    case 0x1: opaddr = regs.SP + fetch(); break;
    case 0xb: opaddr = fetch(); break;
    case 0xc: opaddr = fetch2(); break;
    case 0xf: opaddr = (prefix == 0x90) ? regs.Y : regs.X; break;
    case 0xe: opaddr = ((prefix == 0x90) ? regs.Y : regs.X) + fetch(); break;
    case 0xd: opaddr = ((prefix == 0x90) ? regs.Y : regs.X)+ fetch2(); break;
    default: return(resHALT);
  }

  FLAG_ASSIGN (BIT_Z, (operand & 0xffff) == 0);
  FLAG_ASSIGN (BIT_N, 0x8000 & operand);

  store2(opaddr, operand);
   
  return(resGO);
}

int
cl_stm8::inst_neg(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   resval = 0 - operand;
  
   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_V, (0x80 == operand));
      FLAG_ASSIGN (BIT_C, 0x100 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_V, (0x8000 == operand));
      FLAG_ASSIGN (BIT_C, 0x10000 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_V, (0x8000 == operand));
      FLAG_ASSIGN (BIT_C, 0x10000 & resval);
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_V, (0x80 == operand));
      FLAG_ASSIGN (BIT_C, 0x100 & resval);
   }
  
   return(resGO);
}

int
cl_stm8::inst_or(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 | operand2;
  FLAG_NZ (result);

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_rlc(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) && (prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) && (prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) && (prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest (code, prefix);
      operand = get1 (opaddr);
   }
   
   resval = operand << 0x1;
   
   if (regs.CC & BIT_C) {
      resval++;
   }
  
   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x100));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x10000));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x10000));
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x100));
   }
  
   return (resGO);
}

int
cl_stm8::inst_rrc(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   resval = operand >> 1;
   
  
   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      if (regs.CC & BIT_C) {      resval |= 0x80;   }
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      if (regs.CC & BIT_C) {      resval |= 0x8000;   }
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      if (regs.CC & BIT_C) {      resval |= 0x8000;   }
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else {
      if (regs.CC & BIT_C) {      resval |= 0x80;   }
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   }
  
   return(resGO);
}

int
cl_stm8::inst_sbc(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;
  int carryin = (regs.CC & BIT_C)!=0;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2 - carryin;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_sll(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   resval = operand << 0x1;
  
   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x100));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x10000));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x10000));
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x100));
   }
  
   return(resGO);
}

int
cl_stm8::inst_sra(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   resval = operand >> 1;
   
  
   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      if (operand & 0x80) {      resval |= 0x80;   }
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      if (operand & 0x8000) {      resval |= 0x8000;   }
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      if (operand & 0x8000) {      resval |= 0x8000;   }
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else {
      if (operand & 0x80) {      resval |= 0x80;   }
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   }
  
   return(resGO);
}

int
cl_stm8::inst_srl(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }
   
   resval = operand >> 1;
   
  
   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      //if (operand & 0x80) {      resval |= 0x80;   }
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      //if (operand & 0x8000) {      resval |= 0x8000;   }
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      //if (operand & 0x8000) {      resval |= 0x8000;   }
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else {
      //if (operand & 0x80) {      resval |= 0x80;   }
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   }
  
   return(resGO);
}

int
cl_stm8::inst_sub(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);

  result = operand1 - operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_swap(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
      resval = (operand << 4) | (operand >> 4);
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
      resval = (operand << 8) | (operand >> 8);
      regs.X = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
      resval = (operand << 8) | (operand >> 8);
      regs.Y = resval;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
      resval = (operand << 4) | (operand >> 4);
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
   }
   
   return(resGO);
}

int
cl_stm8::inst_tnz(t_mem code, unsigned char prefix)
{
  unsigned int resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      resval = regs.A;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      resval = regs.X;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      resval = regs.Y;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
   } else {
      opaddr = get_dest(code,prefix);
      resval = get1(opaddr);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
   }
   
   return(resGO);
}

int
cl_stm8::inst_xor(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 ^ operand2;
  FLAG_NZ (result);

  regs.A = result & 0xff;
  return(resGO);
}

/* End of stm8.src/inst.cc */
